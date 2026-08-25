"""Longwin LWFDSPC 韌體的 X2CScope 變數定義、刻度換算與列舉解碼。

本檔是主機端工具 (check_link.py / motor_gui.py / scope_window.py) 的唯一資料來源。
所有變數名稱
都已對 LWFDSPC.X/dist/default/production/LWFDSPC.X.production.elf 用 pyx2cscope
的 GenericParser (DWARF) 驗證過，可直接在目標上解析。

【與 mclv48v300w 參考專案的根本差異】
參考專案的韌體有一整組 `apiData.*` 供主機下命令；本專案**沒有**。這顆控制器的
命令來源是油門/VR、煞車開關、排檔開關與 Modbus 儀表，韌體裡唯一叫「Scope」的
三顆變數 (g_i16ScopeCmdTarget / RateLim / Out) 是**純觀測**用的取樣點，不是命令
入口。因此本工具的定位是「觀測 + 調校參數寫入」，不是「從 PC 開車」。

Component: HOST TOOLING
"""

from pathlib import Path

# 主機端工具自己的版本，與韌體版本 (s_modbus_decode.h 的 FW_VER_*) 各走各的 ——
# 兩者本來就會各自改版，混成一個號碼只會讓「這支 GUI 配哪版韌體」更難講清楚。
# 韌體版本由 GUI 從目標端讀回來顯示 (Modbus reg[21])，不寫死在這裡。
#
# 出版時更新這個字串，並在 git 打對應 tag (ex: gui-v1.0)。
GUI_VERSION = "V1.0"

# 專案根目錄 = 本 gui/ 資料夾的上一層。
REPO_ROOT = Path(__file__).resolve().parent.parent

# 韌體原始碼所在，用來偵測「build 比原始碼舊」。
FIRMWARE_DIR = REPO_ROOT / "LWFDSPC.X"


def newest_elf(root: Path = REPO_ROOT):
    """回傳 root 底下 mtime 最新的 .elf，沒有則 None。

    MPLAB X、它的 VS Code 擴充與 CMake 匯出各自寫到不同的輸出目錄
    (LWFDSPC.X/dist/、LWFDSPC.X/out/、LWFDSPC.X/_build/)，本專案四個地方都有
    .elf 且版本不同。硬編路徑很容易讀到舊的那份，取最新的再把選中的路徑顯示
    出來，遠比猜路徑可靠 —— 猜錯時使用者看得到，可以自己改。
    """
    candidates = []
    for path in root.rglob("*.elf"):
        try:
            candidates.append((path.stat().st_mtime, path))
        except OSError:  # pragma: no cover - 掃描途中檔案消失
            continue
    return max(candidates)[1] if candidates else None


def newest_source_mtime(root: Path = FIRMWARE_DIR):
    """韌體原始碼中最新的 mtime，用來抓出過期的 build。

    只掃 .c/.h/.s —— 組語檔也會進 build，漏掉它就會把「改了 meascurr.s 卻沒重編」
    誤判成對版。
    """
    newest = 0.0
    for pattern in ("*.c", "*.h", "*.s"):
        for path in root.rglob(pattern):
            # 建置產物本身不算原始碼，否則永遠比 elf 新，警告就沒意義了。
            if any(part in ("_build", "out", "dist", "build", "debug",
                            "mcc_generated_files") for part in path.parts):
                continue
            try:
                newest = max(newest, path.stat().st_mtime)
            except OSError:  # pragma: no cover
                continue
    return newest


DEFAULT_ELF = (newest_elf()
               or (FIRMWARE_DIR / "dist" / "default" / "production"
                   / "LWFDSPC.X.production.elf"))

# X2CScope 走專屬的 UART2 (RB8=U2TX / RB9=U2RX)，與 RS485/Modbus 的 UART1 分開。
# diagnostics_x2cscope.c 的 X2C_BAUDRATE_DIVIDER = 26：
#   FCY 100MHz / 16 / (1+26) = 231,481 baud，對 230400 誤差 +0.47%，在容差內。
#
# 【2026-08-25 由 115200 提升到 230400】UART2 頻寬是本工具的瓶頸：實測目標端的
# g_u16X2cTxFifoMaxUsed 長期頂在 416，正好是 TX FIFO 的節流點 (512-96)，代表
# X2CScope_Communicate() 每次都因 TX 快滿而中斷。baud 加倍讓線上時間減半。
#
# ⚠ 必須與韌體一致，否則連不上 (握手就失敗，不是資料錯)。
DEFAULT_BAUD = 230400

# 燒舊韌體 (divider 53) 的板子仍然存在，所以埠掃描在主 baud 全數失敗後會自動退回
# 這個值再掃一輪 —— 不然使用者只會看到「找不到裝置」，卻猜不到是 baud 不符。
LEGACY_BAUD = 115200


# ---------------------------------------------------------------------------
# 位址正確性哨兵
# ---------------------------------------------------------------------------
# diagnostics_x2cscope.c 的 g_u16X2cDiagSignature 是固定常數 0xA5C3。
#
# 每次重新建置後全域變數位址都會位移 (linker 用 --no-gc-sections，位址依連結順序
# 變動)。若主機還套用舊的 elf 去讀新韌體，讀到的會是錯位的別的變數 —— 數字看起來
# 很合理但完全沒有意義。韌體註解裡就記著曾被這件事誤導過。
# 所以連上線的第一件事就是驗這一格。
V_SIGNATURE = "g_u16X2cDiagSignature"
SIGNATURE_EXPECTED = 0xA5C3  # = 42435


# ---------------------------------------------------------------------------
# 刻度換算 (與 LWFDSPC.X/src/motor_scale.h 對應)
# ---------------------------------------------------------------------------
# 這裡的預設值必須與 motor_scale.h 的實體參數一致。改韌體那邊時要一起改，
# 否則主機顯示的 RPM / km/h 會與韌體上報的對不上。
FCY_HZ = 100_000_000.0        # hal/clock.h
ISR_FREQ_HZ = 20_000.0        # userparms.h PWMFREQUENCY_HZ，Scope 的取樣基準
TIMER_PRESCALER = 64          # Timer1 前除器 (Hall 週期量測)

MOTOR_POLE_PAIRS = 3          # 6 極馬達 = 3 極對
GEAR_RATIO_X100 = 2030        # 減速齒比 x100，實測 20.30:1
WHEEL_DIAMETER_INCH_X10 = 80  # 8.0 吋
SPEED_FS_RPM = 12000          # Q15 速度滿刻度 (Speed == 32768 代表的馬達機械 RPM)
MOTOR_MAX_RPM = 4060          # 僅供 sanity check

HALL_EDGES_PER_REV = 6 * MOTOR_POLE_PAIRS          # 18
HALL_TIMER_HZ = FCY_HZ / TIMER_PRESCALER           # 1,562,500
HALL_MIN_PERIOD = 434         # motor_scale.h 的編譯期護欄鎖住這個值
HALL_PULSE_WINDOW_MS = 100    # HallPulsesLatch 的統計窗

Q15_MAX = 32768.0

# userparms.h：counts/A = (R_SHUNT x GAIN / VREF) x ADC滿刻度 x (2 x KCURRA)
#            = (0.002 x 7.89 / 3.3) x 65520 x 1.0 = 313.3   (1 count = 3.2 mA)
# Q15 滿刻度 32768 <-> 104.6 A，正好等於感測器物理極限，兩者一致是刻度正確的自我驗證。
IABC_COUNTS_PER_AMP = 313.30

# 速度模式的轉速上限 (userparms.h)。
MAXSPEED_REF_LIMIT_RPM = 2000
MAXSPEED_CTRLMODE_2_RPM = 1160


class Scale:
    """Q15 <-> 馬達 RPM <-> 輪速 <-> km/h 的換算器。

    做成物件而不是模組級函式，是因為這幾個實體參數 (極對數 / 齒比 / 輪徑 /
    Q15 滿刻度) 決定了畫面上每一個速度數字的意義，而它們在實機上是會被懷疑的
    —— 「顯示 7.2 km/h 到底對不對」正是要靠改一組參數重算來驗。GUI 上可以臨時
    改這裡的值重新換算，**不會**寫進韌體 (韌體的編譯期常數改不了，執行期輪徑
    另有 logic_motor_setWheelDimension，那是 LCD 的路徑，本工具不碰)。
    """

    def __init__(self, pole_pairs=MOTOR_POLE_PAIRS,
                 gear_ratio_x100=GEAR_RATIO_X100,
                 wheel_inch_x10=WHEEL_DIAMETER_INCH_X10,
                 speed_fs_rpm=SPEED_FS_RPM):
        self.pole_pairs = int(pole_pairs)
        self.gear_ratio_x100 = int(gear_ratio_x100)
        self.wheel_inch_x10 = int(wheel_inch_x10)
        self.speed_fs_rpm = int(speed_fs_rpm)

    # -- 推導值 ------------------------------------------------------------
    @property
    def hall_edges_per_rev(self):
        return 6 * self.pole_pairs

    @property
    def wheel_circum_mm(self):
        """車輪周長 (mm)。直徑(吋x10) x 79796 / 10000，與韌體同式同截尾。"""
        return (self.wheel_inch_x10 * 79796) // 10000

    @property
    def hall_min_period(self):
        """Q15 滿刻度對應的 Hall 週期 (Timer1 ticks)。標準參數下 = 434。"""
        return int((HALL_TIMER_HZ * 60.0)
                   / (self.speed_fs_rpm * self.hall_edges_per_rev))

    @property
    def cmd_per_kmh(self):
        """1 km/h 對應多少 Q15 命令 count。標準參數下 = 1447.9。"""
        return self.kmh_to_cmd(1.0)

    # -- 換算 --------------------------------------------------------------
    def q15_to_motor_rpm(self, q15):
        """Q15 速度 -> 馬達機械 RPM。對應 scale_speedToMotorRpm()。"""
        if q15 is None:
            return None
        return q15 * self.speed_fs_rpm / Q15_MAX

    def q15_to_wheel_rpm(self, q15):
        """Q15 速度 -> 車輪 RPM (含齒比)。"""
        rpm = self.q15_to_motor_rpm(q15)
        return None if rpm is None else rpm * 100.0 / self.gear_ratio_x100

    def q15_to_kmh(self, q15):
        """Q15 速度 -> 車速 km/h。負值代表反轉，這裡保留正負號。

        韌體的 u16CurrentSpeedKmh_x10 取絕對值，所以與本函式在反轉時會差一個
        負號 —— 那是刻意的：主機端要能看出方向。
        """
        wheel_rpm = self.q15_to_wheel_rpm(q15)
        if wheel_rpm is None:
            return None
        return wheel_rpm * self.wheel_circum_mm * 60.0 / 1_000_000.0

    def kmh_to_cmd(self, kmh):
        """車速 km/h -> Q15 命令 count。對應 KMHX100_TO_CMD()。"""
        wheel_rpm = kmh * 1_000_000.0 / (60.0 * self.wheel_circum_mm)
        motor_rpm = wheel_rpm * self.gear_ratio_x100 / 100.0
        return motor_rpm * Q15_MAX / self.speed_fs_rpm

    def hall_period_to_motor_rpm(self, period):
        """Hall 週期 (Timer1 ticks) -> 馬達機械 RPM。

        直式換算，不繞 Q15：RPM = 60 x HALL_TIMER_HZ / (period x 邊緣數)。
        標準參數下 period=434 -> 12000 RPM，與 Q15 滿刻度吻合。
        period 為 0 代表「還沒量到任何邊緣」(車停)，回 None 而不是除零。
        """
        if not period:
            return None
        return 60.0 * HALL_TIMER_HZ / (period * self.hall_edges_per_rev)

    def hall_period_to_kmh(self, period):
        """Hall 週期 -> 車速 km/h。EMB 診斷的 g_u16EmbLockPeriod 就用這個看。"""
        rpm = self.hall_period_to_motor_rpm(period)
        if rpm is None:
            return None
        wheel_rpm = rpm * 100.0 / self.gear_ratio_x100
        return wheel_rpm * self.wheel_circum_mm * 60.0 / 1_000_000.0

    def pulses_to_motor_rpm(self, pulses):
        """Hall 邊緣數/100ms -> 馬達機械 RPM。對應 scale_pulsesToMotorRpm()。"""
        if pulses is None:
            return None
        return abs(pulses) * (60_000.0 / HALL_PULSE_WINDOW_MS) / self.hall_edges_per_rev

    def pulses_to_kmh(self, pulses):
        rpm = self.pulses_to_motor_rpm(pulses)
        if rpm is None:
            return None
        wheel_rpm = rpm * 100.0 / self.gear_ratio_x100
        return wheel_rpm * self.wheel_circum_mm * 60.0 / 1_000_000.0

    def describe(self):
        return (f"極對數 {self.pole_pairs}  齒比 {self.gear_ratio_x100 / 100:.2f}  "
                f"輪徑 {self.wheel_inch_x10 / 10:.1f}\"  周長 {self.wheel_circum_mm}mm  "
                f"Q15滿刻度 {self.speed_fs_rpm} RPM  "
                f"1km/h={self.cmd_per_kmh:.1f}count  "
                f"HALL_MIN_PERIOD={self.hall_min_period}")


DEFAULT_SCALE = Scale()


def q15_to_amp(q15):
    """Q15 相電流 -> 安培 (帶正負號，回充為負)。"""
    if q15 is None:
        return None
    return q15 / IABC_COUNTS_PER_AMP


def q15_to_pct(q15):
    """Q15 -> 滿刻度百分比。PI 輸出、佔空比之類的無單位量用這個看。"""
    if q15 is None:
        return None
    return q15 * 100.0 / Q15_MAX


# ---------------------------------------------------------------------------
# 油門電壓刻度 (s_logic_throttle.h + s_logic_convert.c)
# ---------------------------------------------------------------------------
# u16ThrottleVRMv 是**分壓前**的電壓：ADC 讀 12 位元 / VREF 3300mV，再乘
# (R1+R2)/R2 = 151/100 還原成油門線上的實際電壓。所以滿刻度是 4983mV 而不是 3300。
ADC_VREF_MV = 3300
ADC_FULL_SCALE = (1 << 12) - 1
THROTTLE_DIVIDER_R1 = 51_000
THROTTLE_DIVIDER_R2 = 100_000
THROTTLE_FS_MV = int(ADC_VREF_MV
                     * (THROTTLE_DIVIDER_R1 + THROTTLE_DIVIDER_R2)
                     / THROTTLE_DIVIDER_R2)          # 4983

# 韌體的有效油門區間。低於 MIN 是死區 (刻意加大以抑制低端誤觸)，高於 MAX 一律全開。
THROTTLE_MIN_MV = 1200        # LOGIC_THROTTLE_FWD/REV_VOLTAGE_MIN_MV
THROTTLE_MAX_MV = 4800        # LOGIC_THROTTLE_FWD/REV_VOLTAGE_MAX_MV


def throttle_pct(mv):
    """油門電壓 (mV) -> 有效行程百分比。

    刻意不用 mv/滿刻度：那個比例在死區裡就已經是 24%，看起來像「油門有踩」，
    而韌體其實還沒開始出力。這裡用韌體真正的 MIN..MAX 區間換算，0% 就是
    「剛要離開死區」，與車輛行為一致。回傳值不夾在 0..100，超出區間要看得見。
    """
    if mv is None:
        return None
    span = THROTTLE_MAX_MV - THROTTLE_MIN_MV
    return (mv - THROTTLE_MIN_MV) * 100.0 / span


# ---------------------------------------------------------------------------
# 電磁煞車 PWM 刻度 (hal/pwm.h)
# ---------------------------------------------------------------------------
# CCP2 週期 = 100MHz/(4999+1) = 20kHz。duty 的極性與直覺相反：
#   0 = 硬鎖 (夾煞)   EMB_PWM_PERIOD = 全開 (完全釋放)
# 所以顯示成「釋放度」而不是「煞車力道」，免得看反。
EMB_PWM_PERIOD = 4999


def emb_release_pct(duty):
    """s_u16EmbPwmDuty -> 釋放度百分比 (0% = 夾死, 100% = 完全放開)。"""
    if duty is None:
        return None
    return duty * 100.0 / EMB_PWM_PERIOD


# 換相角的刻度是 65536 / 電氣圈，不是 Q15。證據在 main.c 的 Hall 扇區表：
#   FindHallAngle.Pos[5] = 10922 + HallOffset   而 65536/6 = 10922.67
# 這個值宣告成 int16 (main.c: volatile int16_t thetaElectrical)，所以超過半圈之後
# 會變成負數 —— 那不是錯誤，是 65536 域在 16 位元容器裡的自然回捲。因此要先當成
# 無號 16 位元看待再換算，直接拿負值去乘會得到負的角度。
THETA_FULL_TURN = 65536.0


def theta_to_deg(theta):
    """換相角 (65536/電氣圈, 存成 int16) -> 電氣角度 0..360。"""
    if theta is None:
        return None
    try:
        return (int(theta) & 0xFFFF) * 360.0 / THETA_FULL_TURN
    except (TypeError, ValueError):
        return None


# ---------------------------------------------------------------------------
# 遙測：每個輪詢週期都讀，餵捲動圖
# ---------------------------------------------------------------------------
# 每一顆變數都是一次獨立的 LNet 來回，115200 baud 下約 2ms。所以這一組必須小 ——
# 它決定了捲動圖的時間解析度。
FAST_VARS = (
    "Speed",                  # Q15 量測速度 (Hall 週期換算)
    # 真正進速度 PI 的命令 (main.c:2369)：ctrlParm.qVelRef 經 ±SpeedModeCtrlLimit 夾限後。
    # 這是速度環誤差唯一正確的被減數 —— 不是 ReferenceRAWSet，那顆在速度模式下恆為 0
    # (見下方 SLOW_VARS 的說明)。
    "piInputOmega.inReference",
    "g_i16ScopeCmdTarget",    # 油門算出的目標，未經任何斜坡 -> 轉油門時是階躍
    "g_i16ScopeCmdOut",       # 實際送往速度環的命令 (= i16ActiveRpm)
    "IbusAmpX10",             # DC bus 電流 0.1A，帶正負號 (回充為負)
    "idq.q",                  # Q15 轉矩電流
)

SLOW_DIVIDER = 4

# ---------------------------------------------------------------------------
# 遙測：每 SLOW_DIVIDER 個週期讀一次，餵儀表板主要欄位
# ---------------------------------------------------------------------------
SLOW_VARS = (
    # -- 執行狀態 --
    "uGF.RunMotor",
    "uGF.Fault",
    "uGF.UVWLock",
    "uGF.ReGenFlag",
    "uGF.Direction",
    "uGF.DirectionDefault",
    "uGF.BrakeSWOn",
    "uGF.CtrlMode",
    "g_stSystemData.bMotorStop",
    "g_stSystemData.bMotorDirection",
    # -- 命令鏈 (油門 -> 目標 -> 限斜率 -> 速度環) --
    "g_stSystemData.i16TargetRpm",
    "g_stSystemData.i16CurrentRpm",
    "g_i16ScopeCmdRateLim",
    "ReferenceRAWADC",
    # 斜坡後、夾限前的速度命令 (main.c:2361-2365，每 SpeedSlopCntrSet 次走 ±AccSet/DeAccSet)。
    "ctrlParm.qVelRef",
    # ⚠ ReferenceRAWSet 不在速度命令鏈上，也不在 km/h 域。它只在 uGF.CtrlMode == 2
    #   (「speed control, torque limit」) 的分支裡被用來**限制速度環輸出的斜率**：
    #     if (ReferenceRAWSet < ReferenceRAW) ReferenceRAWSet += ReferenceRAWSetStep;
    #     if (piOutputOmega.out > ReferenceRAWSet) piOutputOmega.out = ReferenceRAWSet;
    #   所以它是電流域的上限。CtrlMode 1 還會把它歸零 (main.c:2426 "Reset slop control")。
    #   本車跑 CtrlMode 0，那段程式碼永遠走不到 -> 這一格會恆為 0。那個 0 本身就是
    #   「目前不在 CtrlMode 2」的證據，所以留著顯示，但放在慢速輪詢。
    "ReferenceRAWSet",
    "g_stSystemData.sSharedData.u16CurrentSpeedKmh_x10",
    # Speed 的一階低通 (係數 0.01，main.c:1910)。帶正負號，所以看得出方向；
    # i16SpeedFiltered 是它取絕對值後的版本，方向資訊在那裡就沒了。
    "FilteredSpeed",
    "g_stSystemData.i16SpeedFiltered",
    # -- 電源 / 溫度 --
    "g_stSystemData.u16BatteryVoltage",
    "g_stSystemData.u16BatteryPercent",
    "g_stSystemData.u16ControllerTemp",
    "g_stSystemData.u16MotorTemp",
    "IbusMeanQ15",
    # -- 油門 / 電磁煞車輸入 --
    "g_stSystemData.u16ThrottleVRMv",
    "g_stSystemData.u16ThrottleVRRaw",
    "g_stSystemData.u16ThrottleVR",
    "g_stSystemData.u16IEMBMv",
    "g_stSystemData.sSharedData.u8AssistLevel",
    # -- 電磁煞車狀態 --
    "s_eCurrentState",
    "g_u8EmbLockReason",
    "s_u16EmbPwmDuty",
    "s_u16EmbPwmTicksLeft",
    # -- 感測 --
    "HallPeriod",
    "HallPulsesLatch",
    "HallState",
    "thetaElectrical",
    # -- 速度環 --
    "piInputOmega.piState.outMax",
    "piOutputOmega.out",
)

RARE_DIVIDER = 20

# ---------------------------------------------------------------------------
# 遙測：每 RARE_DIVIDER 個週期讀一次
# ---------------------------------------------------------------------------
# 計數器與慢速旗標。放在自己一組是頻寬考量：全部塞進 SLOW_VARS 的話一次慢週期
# 要跑 75 次來回 (約 150ms)，捲動圖會被卡住。這些值本來就是「事後看累計」用的，
# 一秒兩次綽綽有餘。
RARE_VARS = (
    # -- 位址哨兵，每次都驗 --
    V_SIGNATURE,
    # -- 故障旗標 --
    "FaultFlags.MotorStall",
    "FaultFlags.Undervoltage",
    "FaultFlags.Overvoltage",
    "FaultFlags.MOSOverHeat",
    "FaultFlags.MCUOverHeat",
    "su32_err_activeAlarmsBitmask",
    # -- 電池 / 溫控 / 保護 --
    "g_stSystemData.bBatteryShouldProhibit",
    "g_stSystemData.bBatteryVoltageValid",
    "g_stSystemData.bControllerIsOverTemp",
    "g_stSystemData.bControllerIsOverLoad",
    "g_stSystemData.bControllerTempValid",
    "g_stSystemData.bMotorTempValid",
    "g_stSystemData.bThrottleVRValid",
    "g_stSystemData.bThrottleVrRelease",
    "s_temp_currentZone",
    "s_temp_controller_currentTempC",
    "seCurrentMotorTempStatus",
    "sst_currentBatteryInfo.eSystemStatus",
    "sst_currentBatteryInfo.u8StateOfChargePercent",
    # BMS 經 Modbus 回報的電壓 (0.1V)。與 ADC 換算的 u16BatteryVoltage (0.01V) 是
    # 兩條獨立的路徑，兩邊對不上就代表其中一條的刻度或接線有問題。
    "g_stSystemData.sBatteryData.u16Voltage_x10",
    "s_bMotorStallCurrentLimited",
    # -- EMB 夾煞原因計數器 --
    # 正常停車應該只有 UvwDelay 在累加。Failsafe > 0 表示停車鏈有路徑失效；
    # Downhill > 0 在平路不該出現；Rollback > 0 表示倒溜偵測動作過。
    "g_u16EmbLockCntUvwDelay",
    "g_u16EmbLockCntFailsafe",
    "g_u16EmbLockCntDownhill",
    "g_u16EmbLockCntRollback",
    "g_u16EmbLockCntOther",
    "g_i16EmbLockPulses",
    "g_u16EmbLockPeriod",
    "g_u8ELockActive",
    "g_bEmbRollbackArmed",
    "g_u8EmbNoDecelCnt",
    "g_u8EmbRevEdgeCnt",
    "g_i16EmbZeroCmdEdgeCnt",
    "s_eLastLockReason",
    # -- 主迴圈 / X2CScope 鏈路健康 --
    "g_u32MainLoopHz",
    "g_u16X2cRxErrCount",
    "g_u16X2cRxOerrCount",
    "g_u16X2cRxFerrCount",
    "g_u16X2cRxFifoDropCount",
    "g_u16X2cRxFifoMaxUsed",
    "g_u16X2cTxFifoDropCount",
    "g_u16X2cTxFifoMaxUsed",
    "g_u16X2cUpdateSkipCount",
    "g_u16X2cCommunicatePassMax",
    # -- Modbus --
    "g_u16ModbusState",
    "g_u16ModbusParseFailCount",
    "g_u16ModbusGuardErrorCount",
    "g_u16ModbusLastRxLen",
    # -- 組態 (執行期可被 LCD 改，所以要讀不能假設) --
    "s_currentMotorConfig.u8PolePairs",
    "s_currentMotorConfig.u16HallPPR",
    "s_currentMotorConfig.u16WheelDimensionInches",
    "s_currentMotorConfig.eSpeedSource",
    "g_stSystemData.sSharedData.eControlMode",
    "g_stSystemData.sSharedData.eAccelCurve",
    "g_stSystemData.sSharedData.eBatteryType",
    # 控制器對外回報的韌體版本 (Modbus 暫存器 0x0015 = 索引 21)。
    # 不讀 sSharedData.u8Fw* —— 那三個欄位韌體從未賦值，永遠是 0。
    "g_stModbusAllData.uPcGuiData.u16Regs[21]",
)


# ---------------------------------------------------------------------------
# 連線時讀一次的常數
# ---------------------------------------------------------------------------
# 這些是開機時從 userparms.h 初始化的，讀回來就知道**燒進板子的**韌體是什麼設定
# —— 這是純看 ELF 辦不到的事。
INFO_VARS = (
    V_SIGNATURE,
    "s_currentMotorConfig.u8PolePairs",
    "s_currentMotorConfig.u16HallPPR",
    "s_currentMotorConfig.u16WheelDimensionInches",
    "s_currentMotorConfig.u8ExternalSensorPPR",
    "s_currentMotorConfig.eSpeedSource",
    "g_stModbusAllData.uPcGuiData.u16Regs[21]",
    "g_stSystemData.sSharedData.u8PulsePerRev",
    "g_stSystemData.sSharedData.u8WheelDiameterInches",
    "HallMinPeriod",
    "pwmPeriod",
    "MOSFET_OverTemp",
)


# ---------------------------------------------------------------------------
# 可寫入的變數
# ---------------------------------------------------------------------------
# X2CScope 技術上可以寫任何全域變數，所以「能不能寫」從來不是問題，「寫了會不會
# 被覆寫」才是。下面每一項都標了 persist：
#
#   "boot"  只在開機 / 馬達重新初始化時被寫 -> 改了會一直有效，直到下次 stop→start。
#   "loop"  主迴圈或某個任務每次都重寫 -> 從主機寫進去只會存活幾毫秒，幾乎無用，
#           但仍列出來，因為「寫了沒反應」本身就是需要被看見的事實。
#
# 欄位：(變數名, 顯示標籤, 型別, persist, 說明)
#   型別 "int" 直接送整數；"q15" 讓 GUI 提供 Q15 <-> 實數的輔助。

WRITE_TUNING = (
    # -- 速度環 PI --
    ("piInputOmega.piState.kp", "速度環 Kp", "int", "boot",
     "userparms.h SPEEDCNTR_PTERM = 5000。只在 InitControlParameters() 被寫，"
     "所以改了會一直有效，直到下一次 stop→start 重新初始化。"),
    ("piInputOmega.piState.ki", "速度環 Ki", "int", "boot",
     "SPEEDCNTR_ITERM = 20。同上，stop→start 會回到編譯值。"),
    ("piInputOmega.piState.kc", "速度環 Kc", "int", "boot",
     "SPEEDCNTR_CTERM = Q15(0.999)，積分抗飽和係數。"),
    ("piInputOmega.piState.outMax", "速度環 outMax", "q15", "loop",
     "⚠ 溫控任務每個 tick 都重寫這一格 (main.c 依溫度算出的電流上限)，"
     "從主機寫進去會立刻被蓋掉。要改電流上限請改溫控門檻或 TorqMode_IqMax。"),
    # -- 電流環 PI --
    ("piInputIq.piState.kp", "Iq 環 Kp", "int", "boot",
     "Q_CURRCNTR_PTERM。"),
    ("piInputIq.piState.ki", "Iq 環 Ki", "int", "boot",
     "Q_CURRCNTR_ITERM。"),
    ("piInputId.piState.kp", "Id 環 Kp", "int", "boot",
     "D_CURRCNTR_PTERM。"),
    ("piInputId.piState.ki", "Id 環 Ki", "int", "boot",
     "D_CURRCNTR_ITERM。"),
    # -- 加減速 --
    ("AccSet", "加速率 AccSet", "int", "boot",
     "userparms.h ACC_SET = 50。1 = 每秒 0.3% 滿刻度速度。"),
    ("DeAccSet", "減速率 DeAccSet", "int", "boot",
     "DE_ACC_SET = 50。"),
    ("SpeedSlopCntrSet", "斜坡分頻 SlopCntr", "int", "boot",
     "SPEED_SLOP_CNTR_SET = 20，即 2ms 速度環下每 10ms 走一階。"),
    ("ReferenceRAWSetStep", "斜坡步階", "int", "boot",
     "每一階的 Q15 增量，預設 10。"),
    # -- 電流 / 轉矩上限 --
    ("TorqMode_IqMax", "轉矩上限 IqMax", "q15", "boot",
     "main.c:366 Q15(0.58) 起始值。CTRLMODE 1/2 的轉矩命令上限。"
     "註：DoControl() 在**轉矩控制**分支裡會依 uGF.DriveMode (1/2/3) 把它改成 "
     "Q15(0.6)/0.65/0.75 (main.c:2504-2515)。本車跑速度模式 (CtrlMode 0) 走不到那條"
     "路徑，所以寫入會留住；切到轉矩模式就會被蓋掉。"),
    ("IqSquare.RatedIq", "IqSquare RatedIq", "q15", "boot",
     "I²t 過載保護的額定值。"),
    ("IqSquare.OverCurrent", "IqSquare 過流門檻", "q15", "boot", ""),
    ("IqSquare.Limit", "IqSquare 積分上限", "int", "boot", ""),
    # -- 保護門檻 --
    ("MOSFET_OverTemp", "MOSFET 過溫門檻", "int", "boot",
     "OVERTEMP_MOSFET_90 起始值，ADC raw。"),
    ("UVWLockSpeed", "UVW 鎖定速度", "q15", "boot",
     "註：僅被 main.c 的 #if 0 死碼區使用，live 路徑走 "
     "UVW_LOCK_STOP_PULSES / UVW_LOCK_RELEASE_REF。改了看不到效果是正常的。"),
    ("MotorAlignLockTime", "對位鎖定時間", "int", "boot", ""),
    ("SpeedCtrlLimit", "SpeedCtrlLimit", "q15", "boot",
     "Q15_MAXSPEED_CtrlMode_2。同樣位於 #if 0 死碼區的消費者。"),
    ("SpeedModeCtrlLimit", "SpeedModeCtrlLimit", "q15", "boot", ""),
    # -- 再生制動 --
    ("ReGenTorq", "再生制動力道", "q15", "boot",
     "Q15(0.1) 起始值，再生制動的佔空比。"),
    ("ReGenSpeed", "再生制動速度下限", "q15", "boot", ""),
    # -- 起步 / 煞車速度門檻 --
    ("MotorStartSpeed", "起步速度", "q15", "boot", ""),
    ("MotorStartSpeedPulses", "起步脈衝數", "int", "boot", ""),
    ("BrakeStartSpeed", "煞車起始速度", "q15", "boot", ""),
    ("BrakeStopSpeed", "煞車停止速度", "q15", "boot", ""),
    # -- 換相 --
    ("HallOffset", "Hall 角度偏移", "int", "boot",
     "換相角補償。改這格會直接影響換相對位，先小步試。"),
    # -- 段位 --
    ("g_stSystemData.sSharedData.u8AssistLevel", "助力段位", "int", "loop",
     "0 = e-lock 電子鎖車 (油門歸零且 EMB 不放開)。"
     "⚠ Modbus 儀表若在線，會週期性把它改回儀表的設定值。"),
)

WRITE_DANGER = (
    # 這一組會直接動到馬達輸出或安全鏈。多數會被主迴圈立刻覆寫 —— 這不代表安全，
    # 只代表「效果是一次性的脈衝」，在實車上仍可能造成一次抽動。
    ("uGF.RunMotor", "uGF.RunMotor", "int", "loop",
     "⚠⚠ FOC 執行旗標。狀態機每圈都重算，寫入多為暫態脈衝。"),
    ("uGF.UVWLock", "uGF.UVWLock", "int", "loop",
     "⚠⚠ 1 = 三相下橋全開 (UVW 短路)。低速時 EMB/停車鏈會重寫。"),
    ("uGF.Fault", "uGF.Fault", "int", "loop",
     "⚠⚠ 寫 0 相當於強制清故障，會繞過保護。"),
    ("uGF.ReGenEnable", "uGF.ReGenEnable", "int", "loop", "⚠⚠"),
    ("uGF.CtrlMode", "uGF.CtrlMode", "int", "boot",
     "⚠⚠ 0=speed 1=torque 2=speed limit+torque。切換控制律本身。"),
    ("uGF.DriveMode", "uGF.DriveMode", "int", "boot", "⚠⚠"),
    ("uGF.ReGenMode", "uGF.ReGenMode", "int", "boot", "⚠⚠"),
    ("uGF.Direction", "uGF.Direction", "int", "loop",
     "⚠⚠ 行進方向。排檔開關每圈重寫。"),
    ("ReferenceRAW", "ReferenceRAW", "q15", "loop",
     "⚠⚠ 速度環命令源。main.c 每毫秒從 i16ActiveRpm 重寫，"
     "所以寫入只會存活一個週期 —— 但那一個週期是真的會出力的。"),
    ("ReferenceRAWSet", "ReferenceRAWSet", "q15", "loop",
     "⚠⚠ **不是**速度命令，也不在 km/h 域。它是 uGF.CtrlMode == 2 分支裡用來限制"
     "速度環輸出 (piOutputOmega.out，電流域) 斜率的上限，見 main.c:2449-2455。"
     "CtrlMode 1 會把它歸零 (main.c:2426)。本車跑 CtrlMode 0，那段程式碼走不到，"
     "所以這一格恆為 0，寫進去也不會有任何效果。"),
    ("g_stSystemData.i16TargetRpm", "i16TargetRpm", "q15", "loop",
     "⚠⚠ 油門算出的目標。油門任務每毫秒重寫。"),
)

# 全部可寫變數，供連線時一次解析。
WRITE_VARS = tuple(item[0] for item in WRITE_TUNING + WRITE_DANGER)


# ---------------------------------------------------------------------------
# 列舉解碼 (值取自韌體 header)
# ---------------------------------------------------------------------------
# src/longwin/s_logic_embraker.h  E_EMBRAKER_STATE
EMB_STATE = {
    0: "FAULT 故障",
    1: "LOCKED 鎖定中",
    2: "RELEASED 已釋放",
    3: "WAITING_TO_LOCK 等待鎖定",
}

# src/longwin/s_logic_embraker.h  E_EMBRAKER_LOCK_REASON
EMB_LOCK_REASON = {
    0: "0 NONE 尚未夾過",
    1: "1 UVW_DELAY 正常停車",
    2: "2 FAILSAFE 逾時帶速夾 (異常)",
    3: "3 DOWNHILL 下坡滑動",
    4: "4 ROLLBACK 有動力倒溜",
    5: "5 REVERSE_EDGE Plan B 反向邊緣",
    6: "6 IBKS 手剎車/充電中",
    7: "7 ROLLBACK_HOLD 倒溜閂鎖保持",
    8: "8 FAULT A04 IEMB 故障",
    9: "9 PRERUN_FAIL 運轉前檢查不合格",
}

# src/longwin/s_logic_temp_controller.h  E_LOGIC_TEMP_CONTROLLER_ZONE_T
TEMP_ZONE = {
    0: "NORMAL 正常",
    1: "LEVEL_1 一級限流",
    2: "LEVEL_2 二級限流",
    3: "LEVEL_3 三級限流",
    4: "LEVEL_4 四級限流",
    5: "LEVEL_5 五級限流",
    6: "OVERTEMP 過溫跛行 (限流+限速，不停車)",
}

# src/longwin/s_logic_temp_motor.h  E_LOGIC_MOTOR_TEMP_STATUS_T
MOTOR_TEMP_STATUS = {
    0: "NORMAL 正常",
    1: "OVERHEAT_PENDING 高溫待確認",
    2: "OVERHEAT_ACTIVE 高溫保護中",
    3: "RECOVERY_PENDING 恢復待確認",
}

# src/longwin/s_logic_battery.h  E_LOGIC_BATTERY_STATUS_T
BATTERY_STATUS = {
    0: "OK 正常",
    1: "LOW_WARNING 低電量警告",
    2: "LOW_CUTOFF 低壓切斷 (A01)",
    3: "OVER_VOLTAGE 過壓 (A15)",
    4: "AWAITING_RECOVERY 等待恢復電壓",
}

# src/longwin/s_modbus_decode.h  E_CONTROL_MODE
CONTROL_MODE_LCD = {
    0: "0 ASSIST 助力",
    1: "1 ELECTRIC 電動",
    2: "2 AUTO 自動",
    3: "3 WALK_ASSIST 助推",
    4: "4 TORQUE_SENSOR 扭力感測",
    5: "5 VR_DIRECTION VR 方向",
    6: "6 CRUISE 定速",
}

# src/longwin/s_modbus_decode.h  E_ACCEL_CURVE
ACCEL_CURVE = {
    0: "A0 外部 (LCD/APP)",
    1: "A1 內建 1",
    2: "A2 內建 2",
    3: "A3 內建 3",
    4: "A4 內建 4",
    5: "A5 內建 5",
}

# src/longwin/s_modbus_decode.h  E_BATTERY_TYPE
BATTERY_TYPE = {
    0: "鋰電 24V", 1: "鋰電 36V", 2: "鋰電 48V",
    5: "鉛酸 24V", 6: "鉛酸 36V", 7: "鉛酸 48V",
}

# src/longwin/s_logic_motor.h  E_LOGIC_MOTOR_SPEED_SOURCE_T
SPEED_SOURCE = {0: "HALL 馬達霍爾 (IHU)", 1: "EXTERNAL 外部輪速 (ILSN)"}

# src/longwin/s_modbus_master.c  E_SVC_STATE
MODBUS_STATE = {
    0: "READY_TO_SEND 待發送",
    1: "WAITING_FOR_RESPONSE 等回應",
    2: "DELAY 間隔中",
}

# userparms.h CTRLMODE
UGF_CTRL_MODE = {0: "0 speed 速度", 1: "1 torque 轉矩", 2: "2 speed limit + torque"}

# src/longwin/s_logic_error_handler.h  E_LOGIC_ALARM_CODE_T
# 位元索引 = 列舉值 (logic_errorHandler_setAlarmStatus 用 1UL << eAlarmCode)。
ALARM_BITS = {
    0: "NONE",
    1: "A03 油門異常",
    2: "A02 煞車開關異常",
    3: "A04 電磁煞車感測異常",
    4: "A19 馬達霍爾異常",
    5: "A05 馬達過電流",
    6: "A06 馬達堵轉",
    7: "A07 馬達短路",
    8: "A08 馬達缺相",
    9: "A20 馬達過溫",
    10: "A01 電池低壓",
    11: "A15 電池過壓",
    12: "A09 控制器過溫",
    13: "A10 儀表通信超時",
    14: "A11 控制器參數錯誤",
    15: "A12 控制器硬體故障",
    16: "A14 LSN 感測異常",
    17: "A13 啟動自檢失敗",
    18: "A16 EEPROM 故障",
    19: "A17 低壓禁止輸出",
}


def decode(table, value):
    """把列舉值轉成可讀標籤；未知值回 '?<value>'，None 回 '--'。"""
    if value is None:
        return "--"
    try:
        key = int(value)
    except (TypeError, ValueError):
        return str(value)
    return table.get(key, f"?{key}")


def decode_alarms(bitmask):
    """把 su32_err_activeAlarmsBitmask 展開成中文警報清單。"""
    if bitmask is None:
        return "--"
    try:
        mask = int(bitmask)
    except (TypeError, ValueError):
        return str(bitmask)
    if mask == 0:
        return "無警報"
    active = [ALARM_BITS.get(bit, f"bit{bit}")
              for bit in range(32) if mask & (1 << bit)]
    return ", ".join(active) if active else "無警報"


def hall_bits(hall_state):
    """HallState -> 'HA HB HC' 字串。

    estim/HallScan 把 (HA<<2)|(HB<<1)|HC 打包成 sector 索引，HA 是 MSB。
    寫成三個位元顯示是為了能直接對照示波器/邏輯分析儀，而不是心算。
    """
    if hall_state is None:
        return "--"
    try:
        value = int(hall_state) & 0x7
    except (TypeError, ValueError):
        return str(hall_state)
    return f"{(value >> 2) & 1} {(value >> 1) & 1} {value & 1}   (0b{value:03b})"


# ---------------------------------------------------------------------------
# 儀表板欄位格式化
# ---------------------------------------------------------------------------
# 原始讀值幾乎都沒辦法用眼睛判斷合不合理 (Q15 的 12670 是幾 km/h?)，但換算後的值
# 又沒辦法拿去跟 MPLAB X 的 watch 視窗或示波器對照。所以多數種類**兩個都顯示**：
# 原始值是憑據，物理量才是人看得懂的。

_ENUM_TABLES = {
    "EMB_STATE": EMB_STATE,
    "EMB_LOCK_REASON": EMB_LOCK_REASON,
    "TEMP_ZONE": TEMP_ZONE,
    "MOTOR_TEMP_STATUS": MOTOR_TEMP_STATUS,
    "BATTERY_STATUS": BATTERY_STATUS,
    "CONTROL_MODE_LCD": CONTROL_MODE_LCD,
    "ACCEL_CURVE": ACCEL_CURVE,
    "BATTERY_TYPE": BATTERY_TYPE,
    "SPEED_SOURCE": SPEED_SOURCE,
    "MODBUS_STATE": MODBUS_STATE,
    "UGF_CTRL_MODE": UGF_CTRL_MODE,
}


def _pair(raw, converted):
    """'原始值 -> 物理量' 的統一排版。"""
    return f"{raw}  ->  {converted}"


def counter_delta(old, new):
    """單調 16 位元計數器的增量，正確處理迴繞。回 None 表示算不出來。

    模數算術的代價是「無法分辨 +13,586 與 +79,122」—— 一旦兩次取樣之間繞超過一圈就
    看不出來。這是 16 位元容器的固有限制，不是這裡能解決的；取樣間隔遠小於迴繞週期
    時它是對的，而 GUI 每 125ms 取樣、最快的計數器繞一圈要幾秒，所以夠用。
    """
    try:
        return (int(new) - int(old)) % COUNTER_MODULUS
    except (TypeError, ValueError):
        return None


def _fifo_text(value, size, throttle=None):
    """FIFO 最大用量 -> '用量 / 容量 (百分比)' 加上狀態註記。

    環形緩衝區的可用量是 size-1，所以用量等於 size-1 就是**填滿**。填滿代表已經在
    丟位元組，那是整條鏈路失效的領先指標，一定要講出來而不是只印一個數字。
    """
    used = int(value)
    usable = size - 1
    text = f"{used:,} / {usable:,}   ({used * 100.0 / usable:.0f}%)"
    if used >= usable:
        return text + "   << 已填滿，正在丟位元組"
    if throttle is not None and used >= throttle:
        return text + "   << 已達節流點"
    if used >= usable * FIFO_WARN_RATIO:
        return text + "   < 接近上限"
    return text


def _sign(value, spec):
    """帶正負號的格式化。方向資訊在停車/倒溜診斷裡是重點，不能被絕對值吃掉。"""
    return format(value, spec)


_ROW_FORMATTERS = {
    # -- 無單位 --
    "int": lambda v, s: f"{int(v):,}",
    "hex16": lambda v, s: f"0x{int(v) & 0xFFFF:04X}   ({int(v)})",
    "flag": lambda v, s: "1  ON" if int(v) else "0  off",
    "pct": lambda v, s: f"{int(v)} %",
    "hz": lambda v, s: f"{int(v):,} Hz",
    "tick": lambda v, s: f"{int(v):,} tick",
    "alarms": lambda v, s: decode_alarms(v),
    "hall_bits": lambda v, s: hall_bits(v),
    # -- Q15 --
    "q15_kmh": lambda v, s: _pair(int(v), f"{_sign(s.q15_to_kmh(v), '+.2f')} km/h"),
    "q15_rpm": lambda v, s: _pair(int(v), f"{_sign(s.q15_to_motor_rpm(v), '+.0f')} RPM"),
    "q15_amp": lambda v, s: _pair(int(v), f"{_sign(q15_to_amp(v), '+.2f')} A"),
    "q15_pct": lambda v, s: _pair(int(v), f"{_sign(q15_to_pct(v), '+.1f')} %FS"),
    # -- 韌體已經換算過的定點值。x10/x100 的倍率寫在 main.c 的結構註解裡 --
    "volt_x100": lambda v, s: f"{int(v) / 100.0:.2f} V",
    "volt_x10": lambda v, s: f"{int(v) / 10.0:.1f} V",
    "ppr_x10": lambda v, s: f"{int(v) / 10.0:.1f} PPR",
    # FIFO 用量：把「填滿」講出來，而不是留一個要人自己去比對容量的數字。
    "fifo_rx": lambda v, s: _fifo_text(v, X2C_RX_FIFO_SIZE),
    "fifo_tx": lambda v, s: _fifo_text(v, X2C_TX_FIFO_SIZE,
                                       X2C_TX_FIFO_SIZE - X2C_TX_FIFO_READY_MARGIN),
    # 丟棄計數：0 是正常，非 0 一律標出來 —— 丟掉的是請求框架的一部分。
    "drops": lambda v, s: (f"{int(v):,}" if not int(v)
                           else f"{int(v):,}   << 丟位元組 -> 框架失步"),
    # 這是 uint16_t 且在 ADC ISR 裡以最高 20kHz 遞增 -> 約 3.3 秒就繞一圈 65,535。
    # 所以**絕對值幾乎沒有意義**，只有「增量」那一欄有意義。
    #
    # 刻意不再把它換算成「累計秒數」：那個換算建立在「值是從開機累計」的假設上，
    # 而迴繞讓假設不成立，算出來的秒數是錯的 (曾經印出過錯的秒數)。
    # 說明文字也刻意保持簡短 —— 長字串會把右邊的增量擠出可見範圍，
    # 而增量正是最該看到的東西。詳細解釋放 README。
    "isr_skips": lambda v, s: f"{int(v):,} / {COUNTER_MODULUS - 1:,}   (會迴繞)",
    # LongWin 版本封裝 (s_modbus_decode.h)：(類別<<8)|(主版<<4)|次版，
    # 顯示成「類別.主版次版」兩位小數。ex: 0x0475 -> V4.75
    "fw_version": lambda v, s: (f"V{(int(v) >> 8) & 0xFF}."
                                f"{(int(v) >> 4) & 0xF}{int(v) & 0xF}"
                                f"   (0x{int(v) & 0xFFFF:04X})"),
    "temp_x10": lambda v, s: f"{int(v) / 10.0:.1f} °C",
    "temp_x100": lambda v, s: f"{int(v) / 100.0:.2f} °C",
    "kmh_x10": lambda v, s: f"{int(v) / 10.0:.1f} km/h",
    "amp_x10": lambda v, s: f"{_sign(int(v) / 10.0, '+.1f')} A",
    "inch_x10": lambda v, s: f"{int(v) / 10.0:.1f} 吋",
    "inch": lambda v, s: f"{int(v)} 吋",
    # -- 感測器原始域 --
    "mv": lambda v, s: f"{int(v):,} mV",
    "throttle_pct": lambda v, s: f"{throttle_pct(v):+.1f} % 行程",
    "emb_release": lambda v, s: _pair(int(v), f"釋放 {emb_release_pct(v):.0f}%"),
    "theta": lambda v, s: _pair(int(v), f"{theta_to_deg(v):.1f}°"),
    # Hall 週期為 0 代表「還沒量到任何邊緣」(車停)，換算會回 None -> 顯示「停止」，
    # 而不是 0 km/h：兩者意思不同，前者是沒資料，後者是量到了而且是零。
    "hall_period": lambda v, s: _pair(
        f"{int(v):,} tick",
        "停止 (無邊緣)" if not int(v) else f"{s.hall_period_to_kmh(v):.2f} km/h"),
    "hall_pulses": lambda v, s: _pair(
        f"{int(v):+,} 邊緣/100ms", f"{s.pulses_to_kmh(v):.2f} km/h"),
}


def format_row(kind, value, scale=None):
    """把一顆原始讀值格式化成儀表板要顯示的字串。

    value 為 None (讀取失敗，或這顆變數不在目前的 build 裡) 一律回 '--'，所以呼叫端
    不需要自己先檢查 —— pyx2cscope 讀取失敗時回 None 而不是拋例外。

    scale 省略時用 DEFAULT_SCALE。GUI 會把使用者臨時改過的 Scale 傳進來，這樣在畫面上
    改輪徑或齒比之後，整張表會立刻用新刻度重算 —— 那正是驗證「顯示 7.2 km/h 到底對不對」
    的方法。
    """
    if value is None:
        return "--"
    if scale is None:
        scale = DEFAULT_SCALE
    if kind.startswith("enum:"):
        return decode(_ENUM_TABLES[kind[len("enum:"):]], value)
    formatter = _ROW_FORMATTERS.get(kind)
    if formatter is None:
        return str(value)
    try:
        return formatter(value, scale)
    except (TypeError, ValueError, ZeroDivisionError, OverflowError):
        # 換過 build 之後某個欄位從純量變成結構是會發生的。那一格顯示得醜，總比
        # 一個例外把整個分頁的更新打斷、讓其他 86 個欄位一起凍住要好。
        return str(value)


# ---------------------------------------------------------------------------
# 儀表板分頁 (motor_gui.py 的唯讀欄位)
# ---------------------------------------------------------------------------
# 每列 = (顯示標籤, 變數名, 格式種類)。分頁沿用韌體自己的模組切分，這樣畫面上的一頁
# 對應原始碼裡的一個檔案，追問題時不必在心裡做對照。
#
# 同一顆變數可以出現在多列 —— 油門電壓要同時看 mV 與有效行程 %，那是兩件不同的事：
# 前者判斷線路與 ADC，後者判斷韌體有沒有開始出力 (死區裡 mV 已經是滿刻度的 24%)。
#
# 這裡覆蓋 SLOW_VARS + RARE_VARS + INFO_VARS 全部，所以每一顆讀回來的變數都看得到。
PANELS = {
    "執行狀態": (
        ("FOC 執行中 RunMotor", "uGF.RunMotor", "flag"),
        ("故障 Fault", "uGF.Fault", "flag"),
        ("UVW 三相短路鎖定", "uGF.UVWLock", "flag"),
        ("再生制動中 ReGenFlag", "uGF.ReGenFlag", "flag"),
        ("行進方向 Direction", "uGF.Direction", "flag"),
        ("預設方向 DirectionDefault", "uGF.DirectionDefault", "flag"),
        ("煞車開關 BrakeSWOn", "uGF.BrakeSWOn", "flag"),
        ("控制律 CtrlMode", "uGF.CtrlMode", "enum:UGF_CTRL_MODE"),
        ("馬達停止 bMotorStop", "g_stSystemData.bMotorStop", "flag"),
        ("馬達方向 bMotorDirection", "g_stSystemData.bMotorDirection", "flag"),
        ("電子鎖車 e-lock", "g_u8ELockActive", "flag"),
    ),
    # 這一頁是速度控制診斷的核心：①→④ 是同一個命令被逐步變形的四個取樣點，
    # 轉油門時 ① 是階躍而 ③ 是斜坡，兩者的差距就是斜率限制器在做的事。
    # 註：i16TargetRpm / i16CurrentRpm 名字叫 Rpm，實際上是 Q15 命令域
    # (main.c:2030 把它夾在 32767，main.c:2037 直接餵給 g_i16ScopeCmdTarget)。
    "命令鏈": (
        ("① 油門目標 (未斜坡)", "g_i16ScopeCmdTarget", "q15_kmh"),
        ("② 限斜率後", "g_i16ScopeCmdRateLim", "q15_kmh"),
        ("③ 送往速度環", "g_i16ScopeCmdOut", "q15_kmh"),
        ("④ 斜坡後 qVelRef", "ctrlParm.qVelRef", "q15_kmh"),
        ("⑤ 進 PI 的命令 (夾限後)", "piInputOmega.inReference", "q15_kmh"),
        ("ReferenceRAWADC", "ReferenceRAWADC", "q15_kmh"),
        ("量測速度 Speed", "Speed", "q15_kmh"),
        ("量測速度 (馬達)", "Speed", "q15_rpm"),
        ("濾波速度 (帶號)", "FilteredSpeed", "q15_kmh"),
        ("濾波速度 (取絕對值)", "g_stSystemData.i16SpeedFiltered", "q15_kmh"),
        ("i16TargetRpm (Q15)", "g_stSystemData.i16TargetRpm", "q15_kmh"),
        ("i16CurrentRpm (Q15)", "g_stSystemData.i16CurrentRpm", "q15_kmh"),
        ("韌體上報車速", "g_stSystemData.sSharedData.u16CurrentSpeedKmh_x10", "kmh_x10"),
        ("速度環電流上限 outMax", "piInputOmega.piState.outMax", "q15_amp"),
        ("速度環輸出", "piOutputOmega.out", "q15_amp"),
        # 電流域的斜率上限，只在 CtrlMode 2 作用；CtrlMode 0/1 下恆為 0。
        # 顯示成安培而不是 km/h —— 它夾的是 piOutputOmega.out，不是速度。
        ("ReferenceRAWSet (僅 CtrlMode 2)", "ReferenceRAWSet", "q15_amp"),
    ),
    "電源溫度": (
        ("電池電壓 (ADC 換算)", "g_stSystemData.u16BatteryVoltage", "volt_x100"),
        ("電池電壓 (BMS 回報)", "g_stSystemData.sBatteryData.u16Voltage_x10", "volt_x10"),
        ("電池電量", "g_stSystemData.u16BatteryPercent", "pct"),
        ("BMS 回報電量", "sst_currentBatteryInfo.u8StateOfChargePercent", "pct"),
        ("電池狀態", "sst_currentBatteryInfo.eSystemStatus", "enum:BATTERY_STATUS"),
        ("DC bus 電流 (回充為負)", "IbusAmpX10", "amp_x10"),
        ("DC bus 電流均值", "IbusMeanQ15", "q15_amp"),
        ("轉矩電流 Iq", "idq.q", "q15_amp"),
        ("控制器溫度", "g_stSystemData.u16ControllerTemp", "temp_x10"),
        ("控制器溫度 (溫控模組)", "s_temp_controller_currentTempC", "temp_x100"),
        ("馬達溫度", "g_stSystemData.u16MotorTemp", "temp_x10"),
        ("溫控區間", "s_temp_currentZone", "enum:TEMP_ZONE"),
        ("馬達溫度狀態", "seCurrentMotorTempStatus", "enum:MOTOR_TEMP_STATUS"),
        ("控制器過溫", "g_stSystemData.bControllerIsOverTemp", "flag"),
        ("控制器過載", "g_stSystemData.bControllerIsOverLoad", "flag"),
        ("堵轉限流中", "s_bMotorStallCurrentLimited", "flag"),
        ("電池禁止輸出", "g_stSystemData.bBatteryShouldProhibit", "flag"),
        ("電池電壓有效", "g_stSystemData.bBatteryVoltageValid", "flag"),
        ("控制器溫度有效", "g_stSystemData.bControllerTempValid", "flag"),
        ("馬達溫度有效", "g_stSystemData.bMotorTempValid", "flag"),
    ),
    "輸入": (
        ("油門電壓", "g_stSystemData.u16ThrottleVRMv", "mv"),
        ("油門有效行程", "g_stSystemData.u16ThrottleVRMv", "throttle_pct"),
        ("油門 ADC 原始值", "g_stSystemData.u16ThrottleVRRaw", "int"),
        ("油門 Q15", "g_stSystemData.u16ThrottleVR", "q15_pct"),
        ("油門有效", "g_stSystemData.bThrottleVRValid", "flag"),
        ("油門已放開", "g_stSystemData.bThrottleVrRelease", "flag"),
        ("IEMB 電磁煞車回饋", "g_stSystemData.u16IEMBMv", "mv"),
        ("助力段位 (0 = e-lock)", "g_stSystemData.sSharedData.u8AssistLevel", "int"),
        ("儀表控制模式", "g_stSystemData.sSharedData.eControlMode", "enum:CONTROL_MODE_LCD"),
        ("加速曲線", "g_stSystemData.sSharedData.eAccelCurve", "enum:ACCEL_CURVE"),
    ),
    # 韌體註解說得很直接：正常停車應該只有 UVW_DELAY 在累加。其餘四個計數器不為零
    # 就代表停車鏈走了非預期的路徑，這一頁就是為了讓那件事一眼看得出來。
    "電磁煞車": (
        ("目前狀態", "s_eCurrentState", "enum:EMB_STATE"),
        ("本次夾煞原因", "g_u8EmbLockReason", "enum:EMB_LOCK_REASON"),
        ("上次夾煞原因", "s_eLastLockReason", "enum:EMB_LOCK_REASON"),
        ("EMB PWM (0 = 夾死)", "s_u16EmbPwmDuty", "emb_release"),
        ("EMB PWM 剩餘 ticks", "s_u16EmbPwmTicksLeft", "int"),
        ("① UVW_DELAY 正常停車", "g_u16EmbLockCntUvwDelay", "int"),
        ("② FAILSAFE 逾時帶速夾 ⚠", "g_u16EmbLockCntFailsafe", "int"),
        ("③ DOWNHILL 下坡滑動 ⚠", "g_u16EmbLockCntDownhill", "int"),
        ("④ ROLLBACK 有動力倒溜", "g_u16EmbLockCntRollback", "int"),
        ("其餘 (PlanB/IBKS/故障)", "g_u16EmbLockCntOther", "int"),
        ("夾煞當下脈衝", "g_i16EmbLockPulses", "hall_pulses"),
        ("夾煞當下週期", "g_u16EmbLockPeriod", "hall_period"),
        ("倒溜偵測已解除保險", "g_bEmbRollbackArmed", "flag"),
        ("未減速計數 noDecelCnt", "g_u8EmbNoDecelCnt", "int"),
        ("反向邊緣計數 revEdgeCnt", "g_u8EmbRevEdgeCnt", "int"),
        ("零命令邊緣計數", "g_i16EmbZeroCmdEdgeCnt", "int"),
    ),
    "感測換相": (
        ("Hall 週期", "HallPeriod", "hall_period"),
        ("Hall 邊緣數 (閂鎖)", "HallPulsesLatch", "hall_pulses"),
        ("Hall 狀態 HA HB HC", "HallState", "hall_bits"),
        ("換相角 thetaElectrical", "thetaElectrical", "theta"),
    ),
    "故障警報": (
        ("作用中警報", "su32_err_activeAlarmsBitmask", "alarms"),
        ("馬達堵轉 MotorStall", "FaultFlags.MotorStall", "flag"),
        ("低壓 Undervoltage", "FaultFlags.Undervoltage", "flag"),
        ("過壓 Overvoltage", "FaultFlags.Overvoltage", "flag"),
        ("MOSFET 過溫", "FaultFlags.MOSOverHeat", "flag"),
        ("MCU 過溫", "FaultFlags.MCUOverHeat", "flag"),
    ),
    # 這一頁決定了上面每個數字可不可信，也決定 Scope 擷取撐不撐得住：
    # 有 FIFO 丟棄就代表主迴圈曾經長停頓，那時 Scope 擷取會 timeout。
    "鏈路健康": (
        ("位址哨兵 (應為 0xA5C3)", V_SIGNATURE, "hex16"),
        ("主迴圈頻率", "g_u32MainLoopHz", "hz"),
        ("X2C RX 錯誤總數", "g_u16X2cRxErrCount", "int"),
        ("X2C RX 溢位 OERR", "g_u16X2cRxOerrCount", "int"),
        ("X2C RX 框架錯 FERR", "g_u16X2cRxFerrCount", "int"),
        ("X2C RX FIFO 丟棄", "g_u16X2cRxFifoDropCount", "drops"),
        ("X2C RX FIFO 最大用量", "g_u16X2cRxFifoMaxUsed", "fifo_rx"),
        ("X2C TX FIFO 丟棄", "g_u16X2cTxFifoDropCount", "drops"),
        ("X2C TX FIFO 最大用量", "g_u16X2cTxFifoMaxUsed", "fifo_tx"),
        # 這不是頻寬指標，是 DiagnosticsStepIsr() 的重入保護：ADC ISR 在主迴圈正跑
        # X2CScope_Communicate() 時觸發就跳過 X2CScope_Update()。除以 ISR 頻率
        # (20kHz) 就是累計耗在 Communicate 裡面的秒數，而那段時間 Scope 取樣有缺口。
        ("X2C Update 被跳過", "g_u16X2cUpdateSkipCount", "isr_skips"),
        ("X2C 單圈 Communicate 最多", "g_u16X2cCommunicatePassMax", "int"),
        ("Modbus 狀態", "g_u16ModbusState", "enum:MODBUS_STATE"),
        ("Modbus 解析失敗", "g_u16ModbusParseFailCount", "int"),
        ("Modbus 護衛錯誤", "g_u16ModbusGuardErrorCount", "int"),
        ("Modbus 上次收到長度", "g_u16ModbusLastRxLen", "int"),
    ),
    # 執行期可被 LCD 改，所以一定要讀不能假設。這一頁看到的是**燒進板子的**設定，
    # 那是純看 ELF 辦不到的事。
    # 「(儀表)」的欄位是從 Modbus 收到的 LCD 儀表設定 (s_modbus_decode.c 的解碼路徑)。
    # 沒接儀表時它們一直是 0 —— 那不是控制器的設定，標明來源才不會被當成真的組態。
    "組態": (
        # 控制器對外回報的版本，取自 Modbus 暫存器 0x0015。
        # 刻意**不**讀 sSharedData.u8FwMajor/Minor/Category：那三個欄位在整份韌體裡
        # 只有宣告、從來沒有被賦值 (s_modbus_decode.c:423 的註解自己就寫明
        # 「不依賴未賦值的來源欄位」)，讀它們永遠是 0。
        ("韌體版本 (Modbus 回報)", "g_stModbusAllData.uPcGuiData.u16Regs[21]", "fw_version"),
        ("極對數", "s_currentMotorConfig.u8PolePairs", "int"),
        # u16HallPPR 的單位是 PPR x10，而且已廢棄：舊版用它隱含代表減速齒比
        # (610/120 x 4 = 20.33)，齒比現在由 motor_scale.h 的 GEAR_RATIO_X100 明確定義，
        # 這個欄位已無 live 消費者 (s_logic_motor.h:18-21)。
        ("Hall PPR [已廢棄]", "s_currentMotorConfig.u16HallPPR", "ppr_x10"),
        ("輪徑 (馬達模組)", "s_currentMotorConfig.u16WheelDimensionInches", "inch_x10"),
        ("輪徑 (儀表)", "g_stSystemData.sSharedData.u8WheelDiameterInches", "inch"),
        ("外部輪速感測 PPR", "s_currentMotorConfig.u8ExternalSensorPPR", "int"),
        ("每轉脈衝數 (儀表)", "g_stSystemData.sSharedData.u8PulsePerRev", "int"),
        ("速度來源", "s_currentMotorConfig.eSpeedSource", "enum:SPEED_SOURCE"),
        ("電池型別 (儀表)", "g_stSystemData.sSharedData.eBatteryType", "enum:BATTERY_TYPE"),
        ("HallMinPeriod (Q15 滿刻度)", "HallMinPeriod", "tick"),
        ("PWM 週期", "pwmPeriod", "tick"),
        ("MOSFET 過溫門檻 (ADC raw)", "MOSFET_OverTemp", "int"),
    ),
}


# ---------------------------------------------------------------------------
# 即時捲動圖 (motor_gui.py 左側)
# ---------------------------------------------------------------------------
# 只畫 FAST_VARS —— 它們每個輪詢週期都讀，所以是唯一有足夠時間解析度的一組。
# 每張圖 = (標題, Y 軸單位, ((標籤, 變數名, 顏色, 換算種類), ...))。
#
# 換算掛在**每條軌跡**上而不是整張圖，因為同一張圖裡的軌跡刻度可以不同：
# 電流圖上 IbusAmpX10 是韌體換算過的 0.1A 定點值 (÷10)，而 idq.q 是 Q15 相電流
# (÷313.3)。用單一的每圖換算會把其中一條畫錯 30 倍。
# 軌跡顏色是為**白底**挑的：每一個對白色都有足夠對比 (≥3:1)，且彼此在色相上分得開。
# 原本那組 (#ffd24d / #66dd88 / #4de1d3) 是深底用的淺色，放到白底上幾乎看不見 ——
# 換底色一定要連顏色一起換，否則圖會變成一片空白。
CHARTS = (
    ("命令鏈", "km/h", (
        ("① 油門目標", "g_i16ScopeCmdTarget", "#a06a00", "q15_kmh"),
        ("③ 送往速度環", "g_i16ScopeCmdOut", "#d2600a", "q15_kmh"),
        ("④ 進 PI 的命令", "piInputOmega.inReference", "#7b3fa0", "q15_kmh"),
        ("量測 Speed", "Speed", "#1668b3", "q15_kmh"),
    )),
    ("速度環誤差 (命令 − 量測)", "km/h", (
        ("誤差", "__speed_error", "#1a7a3e", "q15_kmh"),
    )),
    ("電流", "A", (
        ("DC bus (回充為負)", "IbusAmpX10", "#0d7a74", "amp_x10"),
        ("轉矩 Iq", "idq.q", "#c0392b", "q15_amp"),
    )),
)

# CHARTS 裡的合成軌跡：韌體沒有這顆變數，由主機從別的讀值算出來。
# 「速度環追不追得上」看誤差比看兩條幾乎重疊的曲線清楚得多 —— 兩條重疊時後畫的那條
# 會吃掉每一個像素，而差距只有幾百個 count 的時候根本分不出來。
# 用 piInputOmega.inReference 而不是 ReferenceRAWSet：後者在速度模式下恆為 0，
# 相減出來的「誤差」其實只是 -Speed —— 一張看起來很合理、實際上毫無意義的圖。
SYNTHETIC_TRACES = {
    "__speed_error": lambda values: (
        None if values.get("piInputOmega.inReference") is None
        or values.get("Speed") is None
        else values["piInputOmega.inReference"] - values["Speed"]),
}

_CHART_CONVERTERS = {
    "q15_kmh": lambda v, s: s.q15_to_kmh(v),
    "q15_rpm": lambda v, s: s.q15_to_motor_rpm(v),
    "q15_amp": lambda v, s: q15_to_amp(v),
    "q15_pct": lambda v, s: q15_to_pct(v),
    "amp_x10": lambda v, s: v / 10.0,
    "kmh_x10": lambda v, s: v / 10.0,
    "raw": lambda v, s: float(v),
}


def chart_value(kind, value, scale=None):
    """把一條軌跡的原始讀值換算成捲動圖 Y 軸的物理量；None 進 None 出。"""
    if value is None:
        return None
    if scale is None:
        scale = DEFAULT_SCALE
    converter = _CHART_CONVERTERS.get(kind, _CHART_CONVERTERS["raw"])
    try:
        return converter(value, scale)
    except (TypeError, ValueError, ZeroDivisionError, OverflowError):
        return None


# 捲動圖需要的變數 = FAST_VARS 加上合成軌跡的來源。目前合成軌跡只用 FAST_VARS，
# 這個斷言把「以後有人拿 SLOW 變數做合成軌跡」擋在測試而不是實車上。
CHART_VARS = tuple(name for _t, _u, traces in CHARTS for _l, name, _c, _k in traces
                   if not name.startswith("__"))
assert set(CHART_VARS) <= set(FAST_VARS), "捲動圖只能用 FAST_VARS，否則時間解析度不足"


# ---------------------------------------------------------------------------
# Scope 擷取通道
# ---------------------------------------------------------------------------
# 目標端以 ISR 速率 (20kHz) 把這些寫進自己的緩衝區再整批傳回，所以跟上面的輪詢
# 遙測不同，這裡看到的是真波形。一次最多 8 通道 (mchplnet MAX_SCOPE_CHANNELS)。
SCOPE_CHANNELS = (
    # -- 速度命令鏈。四個取樣點合起來就是「油門到速度環」的完整變形過程 --
    ("cmd target [Q15]", "g_i16ScopeCmdTarget"),
    ("cmd rateLim[Q15]", "g_i16ScopeCmdRateLim"),
    ("cmd out    [Q15]", "g_i16ScopeCmdOut"),
    ("qVelRef 斜坡後", "ctrlParm.qVelRef"),
    ("refSet (僅CtrlMode2)", "ReferenceRAWSet"),
    ("ref raw    [Q15]", "ReferenceRAW"),
    ("ref rawADC [Q15]", "ReferenceRAWADC"),
    # -- 速度量測 --
    ("Speed      [Q15]", "Speed"),
    ("Speed filt [Q15]", "FilteredSpeed"),
    ("Hall period[tick]", "HallPeriod"),
    ("Hall periodF", "HallPeriodFiltered"),
    ("Hall pulses", "HallPulses"),
    # -- 電流 --
    ("Iq         [Q15]", "idq.q"),
    ("Id         [Q15]", "idq.d"),
    ("Ia         [Q15]", "iabc.a"),
    ("Ib         [Q15]", "iabc.b"),
    ("Ibus       [Q15]", "Ibus"),
    ("Ibus AVG   [Q15]", "IbusAVG"),
    ("Ibus mean  [Q15]", "IbusMeanQ15"),
    ("Ibus       [0.1A]", "IbusAmpX10"),
    # -- 電壓 --
    ("Vq         [Q15]", "vdq.q"),
    ("Vd         [Q15]", "vdq.d"),
    # -- PI 內部。integrator 是判斷「飽和」與「積分風up」的唯一直接證據 --
    ("omega ref", "piInputOmega.inReference"),
    ("omega meas", "piInputOmega.inMeasure"),
    ("omega integ", "piInputOmega.piState.integrator"),
    ("omega out", "piOutputOmega.out"),
    ("omega outMax", "piInputOmega.piState.outMax"),
    ("Iq integ", "piInputIq.piState.integrator"),
    ("Iq out", "piOutputIq.out"),
    # -- 換相 --
    ("theta_e", "thetaElectrical"),
    ("Hall angle", "HallAngle"),
    ("Hall angleF", "HallAngleFltr"),
    ("Hall state", "HallState"),
    # -- PWM 佔空比 (ADC ISR 內取樣，見 main.c 的觀測變數說明) --
    ("PG1 duty", "X2CPG1Duty"),
    ("PG2 duty", "X2CPG2Duty"),
    ("PG3 duty", "X2CPG3Duty"),
    # -- 旗標。狀態機的邊緣是所有暫態分析的時間基準 --
    ("RunMotor", "uGF.RunMotor"),
    ("UVWLock", "uGF.UVWLock"),
    ("ReGenFlag", "uGF.ReGenFlag"),
    ("Fault", "uGF.Fault"),
    # -- 電磁煞車 --
    ("EMB state", "s_eCurrentState"),
    ("EMB pwm duty", "s_u16EmbPwmDuty"),
    ("EMB pwm ticks", "s_u16EmbPwmTicksLeft"),
    ("EMB lockReason", "g_u8EmbLockReason"),
    ("EMB noDecelCnt", "g_u8EmbNoDecelCnt"),
    ("EMB revEdgeCnt", "g_u8EmbRevEdgeCnt"),
    # -- 輸入 --
    ("throttle [mV]", "g_stSystemData.u16ThrottleVRMv"),
    ("throttle [ADC]", "g_stSystemData.u16ThrottleVRRaw"),
    ("throttle [Q15]", "g_stSystemData.u16ThrottleVR"),
    ("IEMB raw", "g_stSystemData.u16IEMBRaw"),
    ("activeRpm [Q15]", "g_stSystemData.i16ActiveRpm"),
)

# 預設勾選的通道：速度命令鏈的頭尾 + 量測 + 轉矩電流，剛好是調速度環最常看的四條。
SCOPE_DEFAULT = (
    "g_i16ScopeCmdTarget",
    "piInputOmega.inReference",
    "Speed",
    "idq.q",
)

MAX_SCOPE_CHANNELS = 8

# 上面是精選清單。build 裡任何純量全域都能擷取，所以 Scope 視窗另外提供從 ELF
# 自由挑選的介面，使用者加的通道記在這裡跨 session 保留。
SCOPE_CUSTOM_STORE = Path(__file__).resolve().parent / "scope_custom_channels.json"

# 真實 sample 寬度未知時用來預估擷取視窗長度。目標端才決定真正的佈局，這只影響估算。
SCOPE_DEFAULT_SAMPLE_BYTES = 2  # 本韌體絕大多數觀測量是 int16

# ---------------------------------------------------------------------------
# X2CScope 軟體 FIFO 容量 (diagnostics_x2cscope.c)
# ---------------------------------------------------------------------------
# 環形緩衝區的可用量是 SIZE-1，所以「最大用量」看到 127 / 511 就是**填滿**。
#
# RX 填滿是整條鏈路失效的**領先指標**，值得單獨標警告：韌體註解寫明 128 byte
# 約只能緩衝 11ms 的連續資料，一旦主迴圈停頓超過那個長度就會丟位元組 —— 而丟掉的
# 是主機請求框架的一部分，於是目標端不回應 (讀不到) 或回錯 (框架失步)。
#
# TX 的 416 不是巧合：X2C_TX_FIFO_READY_MARGIN = 96，Communicate 在剩餘空間
# 少於 96 時就跳出迴圈，也就是用量到 512-96 = 416 就觸發節流。看到 416 代表
# TX 正持續頂在節流點上。
#
# ⚠ 必須與 diagnostics_x2cscope.c 的 #define 一致。不一致時畫面上的「x / 容量」
#   與百分比會是錯的，而且**看不出來** —— 曾經韌體已改成 2048 而這裡還是 512，
#   於是畫面顯示「36 / 511」，完全無法分辨板子燒的是哪一版。
X2C_RX_FIFO_SIZE = 128
X2C_TX_FIFO_SIZE = 2048
X2C_TX_FIFO_READY_MARGIN = 96

# 用量超過容量的這個比例就標警告。
FIFO_WARN_RATIO = 0.75


# ---------------------------------------------------------------------------
# 單調計數器 —— 需要「基準線」才能拿來做實驗
# ---------------------------------------------------------------------------
# 這些值全部是**開機後累計**，最大用量還是歷史高水位、永遠不會下降。所以單看數字
# 無法回答實驗最想問的那件事：「我剛才改的設定，讓它停止增加了嗎？」
#
# 例如 RX FIFO 丟棄 = 27 可能是這個 session 剛開機那兩秒就發生完的，也可能正在
# 持續增加 —— 兩者的意義完全相反，但畫面上長得一模一樣。
#
# GUI 因此提供「計數器歸零」：按下時把當前值記成基準，之後每一列額外顯示
# 自基準以來的增量。歸零只影響主機端的顯示，不會去寫韌體的計數器
# (那些是 volatile 全域，寫得下去，但清掉別人的診斷資料不是這個工具該做的事)。
#
# ⚠ 這些計數器**全部是 uint16_t**（已對 ELF 逐一驗證），上限 65,535，會迴繞。
#   所以增量一定要用模數算術，直接相減會得到負值並被誤讀成「韌體重置了」。
#   實測踩過：g_u16X2cUpdateSkipCount 在 ADC ISR 裡以最高 20kHz 遞增，幾秒就繞一圈，
#   58,538 -> 6,588 直接相減是 -51,950，迴繞後其實是 +13,586。
#
#   也因此**無法**用這些計數器偵測韌體重啟：迴繞與歸零在 16 位元容器裡長得一樣。
#   要判斷重啟得靠韌體記錄 RCON，那是這裡拿不到的東西。
COUNTER_BITS = 16
COUNTER_MODULUS = 1 << COUNTER_BITS

MONOTONIC_COUNTERS = frozenset((
    # X2CScope 鏈路
    "g_u16X2cRxErrCount", "g_u16X2cRxOerrCount", "g_u16X2cRxFerrCount",
    "g_u16X2cRxFifoDropCount", "g_u16X2cRxFifoMaxUsed",
    "g_u16X2cTxFifoDropCount", "g_u16X2cTxFifoMaxUsed",
    "g_u16X2cUpdateSkipCount", "g_u16X2cCommunicatePassMax",
    # Modbus
    "g_u16ModbusParseFailCount", "g_u16ModbusGuardErrorCount",
    # 電磁煞車夾煞路徑 —— 同樣的問題：「這趟試車有沒有再夾一次」才是重點，
    # 而不是開機以來總共夾過幾次。
    "g_u16EmbLockCntUvwDelay", "g_u16EmbLockCntFailsafe",
    "g_u16EmbLockCntDownhill", "g_u16EmbLockCntRollback",
    "g_u16EmbLockCntOther", "g_i16EmbZeroCmdEdgeCnt",
))

# diagnostics/X2CScope 的 ScopeArray[5000]，用來預估視窗長度：
#   每通道點數 ~= 緩衝區 / (通道數 x 每點位元組)
X2C_BUFFER_BYTES = 5000

# 區分度高的繪圖顏色，依選取順序對應。與 CHARTS 同一組色系 —— 兩邊都是**白底**，
# 所以每個顏色對白色都有 ≥3:1 的對比。換回深色主題時這一組要一起換。
SCOPE_COLOURS = (
    "#1668b3", "#c0392b", "#1a7a3e", "#a06a00",
    "#7b3fa0", "#0d7a74", "#d2600a", "#4a5568",
)

# 疊圖時第二條起套用的虛線樣式。只靠顏色不夠：兩條軌跡重合時最後畫的那條會吃掉
# 每一個像素，量測疊在命令上就會把命令整條藏起來 —— 而那正是最想看的情況。
SCOPE_OVERLAY_DASHES = ((6, 3), (2, 3), (9, 3, 2, 3), (4, 2, 1, 2))


def custom_channel_label(name):
    """使用者自加通道的顯示標籤。

    去掉冗長的 `g_stSystemData.` 前綴 —— 它只吃寬度；其餘一字不改，因為自由挑選
    的通道，精確的變數名就是重點本身。
    """
    for prefix in ("g_stSystemData.sSharedData.", "g_stSystemData."):
        if name.startswith(prefix):
            return name[len(prefix):]
    return name


# ---------------------------------------------------------------------------
# 連線時一次解析的全部變數
# ---------------------------------------------------------------------------
# dict.fromkeys 去重且保留順序 —— V_SIGNATURE 同時出現在 RARE_VARS 與 INFO_VARS。
ALL_VARS = tuple(dict.fromkeys(FAST_VARS + SLOW_VARS + RARE_VARS + INFO_VARS))

# 這些是「必須存在」的核心，缺一個就代表 ELF 與韌體不對版，重試沒有意義。
REQUIRED_VARS = FAST_VARS + (V_SIGNATURE, "uGF.RunMotor", "uGF.Fault")


def elf_scalar_variables(elf_path):
    """[(name, type, byte_size)]，ELF 裡每一個純量全域，已排序。

    陣列**元素**會被剔除。GenericParser 把每個緩衝區的每個元素都展開成獨立條目，
    所以原始清單裡絕大多數是 `ScopeArray[0]` 這種單一位元組 —— 過濾掉才剩下真正
    有用的純量。技術上擷取一個元素是可行的，但擷取 Scope 自己緩衝區的某一個位元組
    從來不會是任何人的意思。

    這個呼叫要跑 DWARF 解析，約一秒，所以呼叫端必須放在 Tk 執行緒之外。
    """
    from pyx2cscope.parser.generic_parser import GenericParser

    parser = GenericParser(str(elf_path))
    out = []
    for name in parser.get_var_list():
        if "[" in name:
            continue
        info = parser.get_var_info(name)
        if info is None:
            continue
        out.append((name, info.type, int(info.byte_size or 0)))
    out.sort(key=lambda item: item[0].lower())
    return out
