"""X2CScope 連線自我測試 (無 GUI)。

**先跑這支，再開 GUI。** 它把通訊問題 (COM port 錯、板子沒供電、ELF 不對版)
與 GUI 問題隔開來，免得在圖形介面裡瞎猜。

用法:
    python gui/check_link.py                 # 自動掃描 port，再跑完整檢查
    python gui/check_link.py --port COM12
    python gui/check_link.py --list          # 只列出序列埠 (★ = 會被掃描)
    python gui/check_link.py --scan          # 只掃描，找出 X2CScope 在哪個埠

Component: HOST TOOLING
"""

import argparse
import logging
import sys
import time
from pathlib import Path

import serial.tools.list_ports

from x2c_link import (PORT_BUSY, candidate_ports, flush_target,
                      force_utf8_console, probe_port)
from x2c_vars import (
    ALL_VARS,
    BATTERY_STATUS,
    DEFAULT_BAUD,
    KNOWN_BAUDS,
    DEFAULT_ELF,
    DEFAULT_SCALE,
    EMB_LOCK_REASON,
    GUI_VERSION,
    EMB_STATE,
    IABC_COUNTS_PER_AMP,
    REQUIRED_VARS,
    SIGNATURE_EXPECTED,
    SPEED_SOURCE,
    TEMP_ZONE,
    V_SIGNATURE,
    decode,
    decode_alarms,
    load_last_baud,
    save_last_baud,
    newest_source_mtime,
    q15_to_amp,
)


def list_ports(include_bluetooth=False):
    ports = list(serial.tools.list_ports.comports())
    if not ports:
        print("找不到任何序列埠。")
        return
    candidates = [device for device, _desc in candidate_ports(include_bluetooth)]
    print(f"{len(ports)} 個序列埠 (★ = 會被自動掃描，依掃描順序):")
    ordered = candidate_ports(include_bluetooth) + [
        (d, s) for d, s in ((p.device, p.description) for p in ports)
        if d not in candidates]
    for device, desc in ordered:
        mark = "★" if device in candidates else "  "
        note = "" if device in candidates else "   (跳過: 藍牙虛擬埠)"
        print(f"  {mark} {device:8s}  {desc}{note}")


def scan_ports(baud, include_bluetooth=False):
    """逐一探測序列埠，找出跑著 X2CScope 的那一個。

    握手成功是目標端有沒有在跑 X2CScope 的決定性證據 —— 只看埠的描述字串猜不出來，
    同一條 USB 轉序列線插到任何板子上描述都一樣。
    """
    ports = candidate_ports(include_bluetooth)
    if not ports:
        print("沒有候選序列埠 (藍牙虛擬埠預設排除，用 --include-bluetooth 納入)。")
        return None
    # 依序試 KNOWN_BAUDS 的每一個值。兩個 baud 都是韌體支援的正常設定 (板子燒哪個
    # 由 X2C_BAUD_TARGET 決定)，而 baud 不符的症狀是「完全沒回應」—— 與沒接線、
    # 沒供電長得一模一樣。逐一試過再報告，使用者才分辨得出來。
    # 順序有講究，不是單純「每個 baud 都試一遍」:
    #   1. 主 baud 直接探測
    #   2. 主 baud + 先沖洗目標端 (它可能停在半截 LNet 框架上)
    #   3. 其他 baud —— **這一輪會弄壞跑其他 baud 的目標**，所以排最後
    #   4. 主 baud + 沖洗，收拾第 3 輪造成的汙染
    # 少了第 2、4 輪，「掃描一次就再也連不上、只能重置控制器電源」會變成常態，
    # 而元凶其實是掃描自己。詳見 x2c_link.flush_target()。
    others = [r for r in KNOWN_BAUDS if r != baud]
    rounds = [(baud, "", False), (baud, " (先沖洗目標端)", True)]
    rounds += [(r, "", False) for r in others]
    if others:
        rounds.append((baud, " (清除其他 baud 造成的汙染)", True))
    print(f"掃描 {len(ports)} 個埠 x {len(rounds)} 輪，每次最多約 2 秒 …\n")
    busy = []
    for rate, note, flush in rounds:
        print(f"  --- @{rate} baud{note} ---")
        for device, desc in ports:
            print(f"  {device:8s} {desc:42s} ", end="", flush=True)
            if flush:
                flush_target(device, rate)
            info = probe_port(device, rate)
            if info is PORT_BUSY:
                # 被占用**不等於**排除掉了 —— 裝置可能就在這個埠上，只是現在開不起來。
                # (PORT_BUSY 是個物件，truthy，所以一定要在 `if info` 之前擋掉。)
                print("開不起來 — 被其他程式占用")
                busy.append(device)
                continue
            if info:
                print("有回應 ✓")
                print(f"\n[ OK ] 找到 X2CScope 裝置: {device} @ {rate} baud")
                print(f"       裝置資訊: {info}")
                # 記下來，GUI 與下一次 check_link 就會優先用這個值 ——
                # 這支工具的用法本來就是「先跑它再開 GUI」，順手把結果傳過去。
                save_last_baud(rate)
                if rate != baud:
                    # 只陳述事實，不建議改成另一個值。KNOWN_BAUDS 裡的每個值都是
                    # 韌體支援的正常設定，而實測 230400 在某些板子上會中途停止回應、
                    # 115200 穩定 —— 叫人「升級」等於推薦他回到會掉線的設定。
                    print(f"\n[INFO] 這塊板子燒的是 {rate} (不是你指定的 {baud})。")
                    print(f"       韌體端的值在 diagnostics_x2cscope.c 的 "
                          f"X2C_BAUD_TARGET。")
                    print(f"       之後用 --baud {rate} 執行本工具，"
                          "GUI 則在下方 baud 選單選這個值 —— ")
                    print("       用錯的 baud 探測會把目標端的 LNet 解析器弄卡，"
                          "選對就完全不會發生。")
                return device
            print("無回應")
    if busy:
        # 排在最前面：這件事可以馬上動手解決，而下面那些檢查項目全是白費工。
        uniq = sorted(set(busy))
        print(f"\n[FAIL] 這些序列埠開不起來，被其他程式占用: {', '.join(uniq)}")
        print("       **它們沒有被排除** —— 裝置可能就在其中一個上面。先關掉占用者:")
        print("       * motor_gui.py 的視窗還開著")
        print("       * 另一個 check_link.py 還在跑")
        print("       * 終端機軟體 (PuTTY / 序列埠監控) 開著同一個埠")
        return None
    print("\n[FAIL] 所有埠 x 所有輪次都沒有回應 X2CScope 的 LNet 握手。")
    print("       baud 已經排除，卡住的半截框架也沖洗過了，所以問題在別處。檢查:")
    print("       * 板子有供電、USB 線接好")
    print("       * codeSw.h 的 CODESW_X2C_SCOPE_ENABLE 仍為 1")
    print("       * X2CScope 走 UART2 (RB8/RB9)，不是 RS485 的 UART1")
    print("       * 目標端主迴圈是否卡死 (X2CScope_Update 沒被呼叫) -> 重置控制器電源")
    return None


def main():
    # 要在 argparse 之前 —— --help 會印中文的 docstring，在 cp1252 主控台下
    # 光那一步就會當掉。
    force_utf8_console()
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--version", action="version",
                    version=f"LWFDSPC 主機端工具 {GUI_VERSION}")
    ap.add_argument("--port", default="AUTO", help='COM port，或 "AUTO" (預設)')
    # 與 GUI 一致：沒指定就用上次成功的值，那比一律套 DEFAULT_BAUD 好 ——
    # 用錯的 baud 探測會把目標端的解析器弄卡 (見 x2c_link.flush_target)。
    ap.add_argument("--baud", type=int, default=None, choices=KNOWN_BAUDS,
                    help=f"UART2 baud（預設：上次成功的值，或 {DEFAULT_BAUD}）")
    ap.add_argument("--elf", default=str(DEFAULT_ELF))
    ap.add_argument("--list", action="store_true", help="列出序列埠後結束")
    ap.add_argument("--scan", action="store_true",
                    help="逐一探測序列埠找出 X2CScope 裝置後結束")
    ap.add_argument("--include-bluetooth", action="store_true",
                    help="掃描時也試藍牙虛擬 COM 埠 (預設跳過)")
    ap.add_argument("--verbose", action="store_true")
    args = ap.parse_args()
    if args.baud is None:
        args.baud = load_last_baud() or DEFAULT_BAUD

    if args.list:
        list_ports(args.include_bluetooth)
        return 0

    logging.disable(logging.WARNING if not args.verbose else logging.NOTSET)

    if args.scan:
        return 0 if scan_ports(args.baud, args.include_bluetooth) else 3

    # AUTO 先用自己的掃描解析成具體的埠，而不是直接把 "AUTO" 丟給 pyx2cscope。
    # 兩者都做真正的 LNet 握手，但自己掃可以印出進度、可以跳過藍牙虛擬埠
    # (開一個沒連線的藍牙埠可能阻塞數秒)，而且失敗時講得出試過哪些埠。
    if args.port.upper() == "AUTO":
        print("埠設為 AUTO —— 先掃描。\n")
        found = scan_ports(args.baud, args.include_bluetooth)
        if found is None:
            return 3
        args.port = found
        print()

    print("ELF :", args.elf)
    print("Port:", args.port, "@", args.baud)
    print("刻度:", DEFAULT_SCALE.describe())

    # build 比原始碼舊是最容易誤導人的失效模式：每個變數都還解析得出來，所以在
    # 寫入落到錯誤位址、或某個新欄位其實不存在之前，什麼異狀都看不出來。
    elf_path = Path(args.elf)
    if elf_path.is_file():
        elf_time = elf_path.stat().st_mtime
        src_time = newest_source_mtime()
        stamp = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(elf_time))
        print(f"      建置於 {stamp}")
        if src_time > elf_time:
            newer = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(src_time))
            print(f"\n[WARN] 過期的 BUILD: 有原始檔在 {newer} 被修改過，"
                  f"\n       比這個 ELF 還新。請重新建置並燒錄，或用 --elf 指定"
                  f"\n       你真正燒進板子的那一份。")
    else:
        print(f"\n[FAIL] 找不到 ELF: {args.elf}")
        print("       先在 MPLAB X 建置專案，或用 --elf <path> 指定。")
        return 2

    # 步驟 1 — 離線解析 ELF。這一步不需要硬體，所以失敗純粹是建置/路徑問題。
    from pyx2cscope.parser.generic_parser import GenericParser

    try:
        parser = GenericParser(args.elf)
    except Exception as exc:  # noqa: BLE001 - 報告後直接結束
        print(f"\n[FAIL] 無法解析 ELF: {exc}")
        return 2

    print(f"\n[ OK ] ELF 解析完成，{len(parser.get_var_list())} 個符號")

    missing_required = [n for n in REQUIRED_VARS if parser.get_var_info(n) is None]
    if missing_required:
        print(f"[FAIL] {len(missing_required)} 個**必要**變數不在這個 ELF 裡:")
        for name in missing_required:
            print("        ", name)
        print("       這個 ELF 幾乎確定是別的韌體版本。")
        return 2
    print(f"[ OK ] {len(REQUIRED_VARS)} 個必要變數全部解析成功")

    # 選用變數缺席是合法的 (例如某個 CODESW 關掉時整個模組沒進 build)，
    # 但要講清楚，否則 GUI 上那一格空白會被當成 GUI 壞了。
    absent = [n for n in ALL_VARS if parser.get_var_info(n) is None]
    if absent:
        print(f"[WARN] {len(absent)} 個選用變數不在這個 ELF 裡 "
              f"-> GUI 對應欄位會顯示 '--':")
        for name in absent:
            print("          缺少:", name)
    else:
        print(f"[ OK ] 全部 {len(ALL_VARS)} 個變數都解析成功")

    # 步驟 2 — 開啟連線。這一步**需要**硬體。
    from pyx2cscope.x2cscope import X2CScope

    try:
        scope = X2CScope(port=args.port, baud_rate=args.baud, elf_file=args.elf)
    except Exception as exc:  # noqa: BLE001 - 報告後直接結束
        print(f"\n[FAIL] 無法開啟 X2CScope 連線: {exc}")
        print("       檢查: 板子有供電、USB 線、COM port 正確，")
        print("       以及 codeSw.h 的 CODESW_X2C_SCOPE_ENABLE 仍為 1。")
        print("       注意 X2CScope 走 UART2 (RB8/RB9)，不是 RS485 的 UART1。")
        return 3

    print("[ OK ] 連線建立")
    try:
        print("       裝置資訊:", scope.get_device_info())
    except Exception as exc:  # noqa: BLE001 - 非致命
        print("       (裝置資訊讀取失敗:", exc, ")")

    # 步驟 3 — 位址正確性哨兵。這是所有其他數字可信與否的前提，所以先驗它。
    print("\n--- 位址哨兵 (ELF 與韌體是否對版) ---")
    # 「讀不回來」與「讀到的值不對」是兩件事，結論相反：前者位址好得很，是對方沒
    # 講話 (baud 不符 / 解析器卡住)；後者位址真的位移了，要換 ELF。混成一句
    # 「哨兵不符 => 請用對版的 ELF 重跑」會讓人白花時間在一個本來就對的檔案上。
    signature_ok = False
    signature_read = False
    signature = None
    try:
        # get_value() 失敗時 pyx2cscope 是**吞掉例外回傳 None**，不是拋例外 ——
        # 所以「讀不到」必須靠 None 判斷，只包 try/except 會漏掉。
        signature = scope.get_variable(V_SIGNATURE).get_value()
    except Exception as exc:  # noqa: BLE001
        print("  (哨兵讀取拋出例外:", exc, ")")
    if signature is None:
        print(f"  {V_SIGNATURE} = 讀不回來 (期望 0x{SIGNATURE_EXPECTED:04X})")
        print("  [FAIL] 目標端沒有回應這一顆 => 這**不是** ELF 版本問題，位址沒有位移。")
        print("         最可能是 baud 不符，或目標端的 LNet 解析器卡在半截框架上。")
        print("         跑 --scan：它會逐一試 baud，並沖洗目標端卡住的解析器。")
    else:
        signature_read = True
        print(f"  {V_SIGNATURE} = 0x{_fmt(signature, '04X')} "
              f"(期望 0x{SIGNATURE_EXPECTED:04X})")
        try:
            signature_ok = int(signature) == SIGNATURE_EXPECTED
        except (TypeError, ValueError):
            signature_ok = False
        if signature_ok:
            print("  [ OK ] ELF 與韌體對版，以下數值可信。")
        else:
            print("  [FAIL] 哨兵值不符 => 這個 ELF 是舊的 / 別的 build。")
            print("         全域變數位址已位移，以下讀到的每一個數字都是錯位的"
                  "別的變數，數值全部作廢。")
            print("         請改用這次燒錄所對應的 ELF。")

    # 步驟 4 — 把每個變數讀一次。
    print("\n--- 即時數值 ---")
    failures = 0
    values = {}
    for name in ALL_VARS:
        # get_variable 對未知符號回 None，不會拋例外。
        handle = scope.get_variable(name)
        if handle is None:
            print(f"  {name:52s} 不在 ELF 中")
            continue
        try:
            value = handle.get_value()
        except Exception as exc:  # noqa: BLE001 - 繼續跑，最後統計
            print(f"  {name:52s} 讀取錯誤: {exc}")
            failures += 1
            continue
        values[name] = value
        text = f"{value:.4f}" if isinstance(value, float) else str(value)
        print(f"  {name:52s} {text}")

    # 步驟 5 — 換算成物理量。原始 Q15 沒辦法用眼睛判斷合不合理，換算過才有意義。
    print("\n--- 換算後的物理量 ---")
    scale = DEFAULT_SCALE

    def get(name):
        return values.get(name)

    speed_q15 = get("Speed")
    print(f"  量測速度        {speed_q15} Q15"
          f"  ->  {_fmt(scale.q15_to_motor_rpm(speed_q15), '.0f')} 馬達RPM"
          f"  ->  {_fmt(scale.q15_to_kmh(speed_q15), '.2f')} km/h")
    ref = get("ReferenceRAWSet")
    print(f"  速度環命令      {ref} Q15"
          f"  ->  {_fmt(scale.q15_to_kmh(ref), '.2f')} km/h")
    target = get("g_i16ScopeCmdTarget")
    print(f"  油門目標(未斜坡) {target} Q15"
          f"  ->  {_fmt(scale.q15_to_kmh(target), '.2f')} km/h")
    print(f"  韌體上報車速    {_fmt_x10(get('g_stSystemData.sSharedData.u16CurrentSpeedKmh_x10'))} km/h")
    print(f"  Hall 週期       {get('HallPeriod')} tick"
          f"  ->  {_fmt(scale.hall_period_to_kmh(get('HallPeriod')), '.2f')} km/h")

    ibus = get("IbusAmpX10")
    print(f"  DC bus 電流     {_fmt_x10(ibus)} A  (韌體換算，回充為負)")
    print(f"  Iq              {_fmt(q15_to_amp(get('idq.q')), '+.2f')} A"
          f"     Id {_fmt(q15_to_amp(get('idq.d')), '+.2f')} A"
          f"   (主機換算 @{IABC_COUNTS_PER_AMP:.1f} counts/A)")
    print(f"  電池電壓        {_fmt_x100(get('g_stSystemData.u16BatteryVoltage'))} V"
          f"   電量 {get('g_stSystemData.u16BatteryPercent')} %")
    print(f"  控制器溫度      {_fmt_x10(get('g_stSystemData.u16ControllerTemp'))} °C"
          f"   馬達溫度 {_fmt_x10(get('g_stSystemData.u16MotorTemp'))} °C")
    print(f"  油門輸入        {get('g_stSystemData.u16ThrottleVRMv')} mV"
          f"   IEMB {get('g_stSystemData.u16IEMBMv')} mV")

    # 步驟 6 — 狀態機與故障，讓人可以人工複核。
    print("\n--- 狀態解碼 ---")
    print(f"  RunMotor              {get('uGF.RunMotor')}"
          f"   Fault {get('uGF.Fault')}"
          f"   UVWLock {get('uGF.UVWLock')}"
          f"   ReGenFlag {get('uGF.ReGenFlag')}")
    print(f"  電磁煞車狀態          {decode(EMB_STATE, get('s_eCurrentState'))}")
    print(f"  最後夾煞原因          {decode(EMB_LOCK_REASON, get('g_u8EmbLockReason'))}")
    print(f"  溫控區間              {decode(TEMP_ZONE, get('s_temp_currentZone'))}")
    print(f"  電池狀態              {decode(BATTERY_STATUS, get('sst_currentBatteryInfo.eSystemStatus'))}")
    print(f"  速度來源              {decode(SPEED_SOURCE, get('s_currentMotorConfig.eSpeedSource'))}")
    print(f"  作用中警報            {decode_alarms(get('su32_err_activeAlarmsBitmask'))}")
    print(f"  故障旗標              stall={get('FaultFlags.MotorStall')}"
          f" uv={get('FaultFlags.Undervoltage')}"
          f" ov={get('FaultFlags.Overvoltage')}"
          f" mos={get('FaultFlags.MOSOverHeat')}"
          f" mcu={get('FaultFlags.MCUOverHeat')}")

    # 步驟 7 — EMB 夾煞路徑。正常停車應該只有 UvwDelay 在累加。
    print("\n--- 電磁煞車夾煞路徑計數 (韌體註解: 正常停車只有 UVW_DELAY 會累加) ---")
    emb_counters = (
        ("1 UVW_DELAY 正常停車", "g_u16EmbLockCntUvwDelay", False),
        ("2 FAILSAFE 逾時帶速夾", "g_u16EmbLockCntFailsafe", True),
        ("3 DOWNHILL 下坡滑動", "g_u16EmbLockCntDownhill", True),
        ("4 ROLLBACK 有動力倒溜", "g_u16EmbLockCntRollback", False),
        ("其餘 (PlanB/IBKS/故障)", "g_u16EmbLockCntOther", False),
    )
    for label, name, suspicious in emb_counters:
        count = get(name)
        flag = ""
        if suspicious and count:
            flag = "   <== 平路不該出現，停車鏈可能有路徑失效"
        print(f"  {label:26s} {count}{flag}")
    print(f"  夾煞當下車速  pulses={get('g_i16EmbLockPulses')}"
          f" period={get('g_u16EmbLockPeriod')}"
          f"  ->  {_fmt(scale.hall_period_to_kmh(get('g_u16EmbLockPeriod')), '.2f')} km/h")

    # 步驟 8 — 鏈路健康度。這決定了 GUI 的輪詢與 Scope 擷取撐不撐得住。
    print("\n--- 主迴圈 / X2CScope 鏈路健康度 ---")
    loop_hz = get("g_u32MainLoopHz")
    print(f"  主迴圈頻率            {loop_hz} Hz  (Modbus 停用時實測約 42835Hz)")
    print(f"  RX 錯誤總數           {get('g_u16X2cRxErrCount')}"
          f"  (OERR {get('g_u16X2cRxOerrCount')}"
          f" / FERR {get('g_u16X2cRxFerrCount')})")
    print(f"  RX 軟體 FIFO 丟棄     {get('g_u16X2cRxFifoDropCount')}"
          f"   最大用量 {get('g_u16X2cRxFifoMaxUsed')} / 127")
    print(f"  TX 軟體 FIFO 丟棄     {get('g_u16X2cTxFifoDropCount')}"
          f"   最大用量 {get('g_u16X2cTxFifoMaxUsed')} / 511")
    print(f"  Update 被跳過次數     {get('g_u16X2cUpdateSkipCount')}"
          f"   單圈 Communicate 最多 {get('g_u16X2cCommunicatePassMax')} 次")
    drops = (get("g_u16X2cRxFifoDropCount") or 0) + (get("g_u16X2cTxFifoDropCount") or 0)
    if drops:
        print("  [WARN] 有 FIFO 丟棄 => 主迴圈曾出現長停頓。Scope 擷取可能會 timeout。")

    scope.disconnect()

    print()
    if not signature_read:
        print("[FAIL] 位址哨兵讀不回來 —— 目標端沒有回應，上面的數值全是 '--'。")
        print("       這不是 ELF 版本問題。先跑 --scan 確認 baud 與埠。")
        return 2
    if not signature_ok:
        print("[FAIL] 位址哨兵不符，上面所有數值都不可信。請用對版的 ELF 重跑。")
        return 2
    if failures:
        print(f"[FAIL] {failures} 個讀取錯誤。連線已關閉。")
        return 1
    print("[ OK ] 全部通過。連線已關閉。")
    return 0


def _fmt(value, spec):
    """容許 None 的格式化 —— 讀取失敗時 pyx2cscope 回 None 而不是拋例外。"""
    if value is None:
        return "--"
    try:
        return format(value, spec)
    except (TypeError, ValueError):
        return str(value)


def _fmt_x10(value):
    return "--" if value is None else f"{value / 10.0:.1f}"


def _fmt_x100(value):
    return "--" if value is None else f"{value / 100.0:.2f}"


if __name__ == "__main__":
    sys.exit(main())
