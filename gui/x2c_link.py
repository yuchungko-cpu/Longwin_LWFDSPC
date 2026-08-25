"""共用的 X2CScope 連線工作執行緒。

**為什麼要獨立成一支：** 一個 COM 埠只能開一次，而 Scope 擷取視窗是從儀表板開出來的
—— 兩邊必須共用同一個 X2CScope session，不能各開一條。所以連線由單一 worker thread
獨佔，Tk 執行緒永遠不碰 serial，只透過兩條 queue 溝通：

    events   (worker -> Tk)   狀態、遙測、寫入結果、Scope 資料
    commands (Tk -> worker)   寫入變數、配置/啟動 Scope、載入 ELF 符號表

DemoLink 是同介面的模擬實作，不開序列埠，用合成訊號餵同一條 events queue。
用途是無車驗證版面與格式器 —— 而不是拿來假裝有量到東西。

Component: HOST TOOLING
"""

import contextlib
import logging
import queue
import threading
import time

import x2c_vars as V

# pyx2cscope 在 INFO 等級會把每一次 LNet 來回都寫進 pyx2cscope.x2cscope.log，
# 一秒幾十筆；輪詢迴圈跑起來那個檔案會長到幾百 MB。
#
# 必須 disable 到 ERROR 而不是 WARNING：logging.disable(WARNING) 只擋 WARNING 及以下，
# **ERROR 照樣輸出**。而 pyx2cscope 對每一次讀取失敗都會 logging.error 兩行
# ("ord() expected a character..." + "cannot convert 'NoneType' object to bytes")，
# 讀取失敗時那是每秒數百行的洗頻，會把我們自己的診斷訊息完全埋掉。
#
# 抑制它們不會遺失資訊：這些例外**已經**被 _read_one() 捕捉，訊息存進 self._last_error
# 並顯示在 GUI 的斷線/失步說明裡 —— 那裡才是使用者會看的地方。
logging.disable(logging.ERROR)

# 連續讀取失敗到這個數目才判定鏈路真的斷了。這是**最後的保險**，不是主要偵測手段
# —— 判斷鏈路死活是哨兵閘門的工作 (它有復原階梯，也講得出是失步、目標端卡死還是
# baud 不符)。
#
# 這個門檻刻意訂得很高。原本是 30，而一個輪詢週期最壞要讀 55 顆變數 —— 也就是
# 「68ms 內全失敗就斷線」，太敏感了。更關鍵的是它在 _read_one 修好之前**從來不會
# 觸發** (get_value() 吞掉例外回傳 None，_err_streak 永遠是 0)，修好之後它突然活了，
# 於是「偶發一陣失敗但會自己恢復」這種以前撐得過去的狀況變成直接斷線。
# 200 次 ≈ 連續 4 個週期完全讀不到，那時哨兵早就先報告了。
ERROR_STREAK_LIMIT = 200

# Scope 取樣的時間基準：目標端在 ADC ISR 裡寫入自己的緩衝區。
SCOPE_BASE_DT_US = 1_000_000.0 / V.ISR_FREQ_HZ   # 50 µs @ 20 kHz

# --demo 連續模式下兩次擷取的間隔。取 1.2 s 是照實機的量測值：一次滿緩衝擷取
# (請求 -> 等觸發 -> 分塊讀回 4KB) 大約就是這個時間。demo 若送得比這快，畫出來的
# 更新率會給人「連續擷取很即時」的錯誤印象，而且會把 GUI 餵到飽和。
DEMO_SCOPE_PERIOD_S = 1.2

# 連續失步幾輪之後才升級到「完整復原階梯」(含重開序列埠)。之前的輪次只做一次
# 便宜的 flush + 複驗。完整階梯約需 1 秒，每輪都跑會把輪詢率從 8Hz 拖到 1.2Hz。
DESYNC_ESCALATE = 3

# 每次偵測到框架失步時，重新對齊的嘗試次數。
RESYNC_ATTEMPTS = 3

# 重開序列埠的最短間隔 (秒)。重開本身會讓接下來幾筆交易失敗，所以無節制地重開會
# 變成自我維持的故障：每個週期重開一次 -> 下一輪必定失敗 -> 再重開，永遠出不來。
REOPEN_MIN_INTERVAL = 8.0

# 連續多少輪「目標端完全不回應」才判定成目標端卡死。125ms 間隔下 24 輪約 3 秒。
# 目標端卡死時主機端做什麼都沒用 (清緩衝區、重開埠都碰不到目標端的狀態機)，
# 所以要停止無謂的重試，直接告訴使用者需要重置控制器電源。
NOREPLY_WEDGE_LIMIT = 24

# 連續多少輪都無法讓框架重新對齊，才判定成「ELF 不對版 / 鏈路失效」並封鎖寫入。
# 給幾輪緩衝的理由：偶發的失步靠清緩衝區就能修好，不該每次都跳紅色橫幅嚇人。
INVALID_CYCLES_LIMIT = 5

# Scope 讀回期間的逐位元組讀取逾時 (秒)。LNetSerial 預設 1 秒，而 bulk 傳輸時目標端
# 受 TX FIFO 節流影響會斷斷續續地吐 —— 一個還在正常進行的回應會被誤判成失敗。
# 只在讀回期間放寬；遙測維持 1 秒，才不會讓真正的斷線變得遲鈍。
SCOPE_READ_TIMEOUT_S = 3.0

# 讀回緩衝區時，單一個 253 byte 區塊的重試次數。重試同一個區塊而不是跳過它 ——
# 跳過會讓後面所有通道的解交錯錯位 (見 _read_scope_data 的說明)。
SCOPE_CHUNK_RETRY = 2

# 擷取失敗自動重試的次數。失敗多半是傳輸框架錯位，清掉收接緩衝區重來就好，
# 不該要使用者自己按第二次。
SCOPE_MAX_RETRY = 2

# 讀回的點數至少要達到預期的這個比例才算有效。pyx2cscope 會靜默回傳短少的緩衝區，
# 而短少的資料解交錯之後是整體錯位的垃圾 —— 寧可報失敗，不要畫一張會騙人的圖。
SCOPE_MIN_COMPLETENESS = 0.98


class Link(threading.Thread):
    """獨佔 X2CScope session 的背景執行緒。"""

    def __init__(self, port, baud, elf, interval_ms, events,
                 rare_slice=8, allow_writes=True):
        super().__init__(daemon=True, name="x2c-link")
        self.port, self.baud, self.elf = port, baud, str(elf)
        self.interval = max(0.05, interval_ms / 1000.0)
        self.rare_slice = max(1, rare_slice)
        self.allow_writes = allow_writes
        self.events = events
        self.commands = queue.Queue()
        self.stop_event = threading.Event()

        self._scope = None
        self._handles = {}
        self._signature_ok = False
        self._err_streak = 0
        self._last_error = ""
        # 「從連線到現在有沒有成功讀過任何一顆變數」。用來分辨兩種完全不同的故障：
        # 從頭到尾讀不到 = baud/設定不符；曾經讀得到後來不行 = 目標端卡死。
        # 給的建議剛好相反，弄錯只會讓人白白重置電源。
        self._ever_read_ok = False
        self._invalid_cycles = 0
        self._last_signature = None
        self._flush_broken = False
        self._last_reopen = 0.0
        self._noreply_cycles = 0
        self._wedge_reported = False
        self._last_drained = 0
        # Scope 狀態機：None -> "sampling" -> "reading" -> None (或回 sampling)
        self._scope_mode = None
        self._scope_continuous = False
        self._scope_rearm = False
        self._scope_active_reported = False
        self._scope_channels = ()
        self._scope_factor = 1
        self._scope_deadline = 0.0
        self._scope_retry = 0

    # -- Tk 端呼叫 ---------------------------------------------------------
    def submit(self, kind, payload=None):
        """從 Tk 執行緒送一個命令給 worker。不阻塞。"""
        self.commands.put((kind, payload))

    def stop(self):
        self.stop_event.set()

    def set_interval(self, interval_ms):
        self.interval = max(0.05, interval_ms / 1000.0)

    # -- worker 內部 -------------------------------------------------------
    def post(self, kind, payload=None):
        self.events.put((kind, payload))

    def run(self):
        try:
            if not self._connect():
                return
            self._poll_loop()
        except Exception as exc:  # noqa: BLE001 - 一律回報到 GUI，不留未捕捉的 worker 例外
            if not self.stop_event.is_set():
                self.post("error", f"鏈路中斷: {exc}")
        finally:
            if self._scope is not None:
                try:
                    self._scope.disconnect()
                except Exception:  # noqa: BLE001
                    pass
            self.post("disconnected")

    # -- 連線 --------------------------------------------------------------
    def _connect(self):
        """照 check_link.py 已經證明過的順序：先離線驗 ELF，再開連線，最後驗哨兵。"""
        from pathlib import Path

        from pyx2cscope.parser.generic_parser import GenericParser

        elf_path = Path(self.elf)
        if not elf_path.is_file():
            self.post("error", f"找不到 ELF: {self.elf}")
            return False

        # 步驟 1 — 離線解析。這一步不需要硬體，所以失敗純粹是建置/路徑問題，
        # 先擋掉可以省下「以為是接線問題」的時間。
        self.post("status", "解析 ELF …")
        try:
            parser = GenericParser(self.elf)
        except Exception as exc:  # noqa: BLE001
            self.post("error", f"無法解析 ELF: {exc}")
            return False

        missing_required = [n for n in V.REQUIRED_VARS if parser.get_var_info(n) is None]
        if missing_required:
            self.post("error",
                      f"這個 ELF 缺 {len(missing_required)} 個必要變數 "
                      f"({', '.join(missing_required[:4])}…)，"
                      "幾乎確定是別的韌體版本。重試沒有意義，請換對版的 ELF。")
            return False
        absent = [n for n in V.ALL_VARS if parser.get_var_info(n) is None]

        # 步驟 2 — build 比原始碼舊是最容易誤導人的失效模式：每個變數都還解析得出來，
        # 所以在寫入落到錯誤位址之前什麼異狀都看不出來。
        elf_mtime = elf_path.stat().st_mtime
        src_mtime = V.newest_source_mtime()
        stale = src_mtime > elf_mtime

        # 步驟 3 — 開連線。這一步需要硬體。
        self.post("status", f"開啟 {self.port} @ {self.baud} …")
        try:
            from pyx2cscope.x2cscope import X2CScope
            self._scope = X2CScope(port=self.port, baud_rate=self.baud,
                                   elf_file=self.elf)
        except Exception as exc:  # noqa: BLE001
            self.post("error",
                      f"無法開啟連線: {exc}\n"
                      "檢查: 板子有供電、USB 線、COM port 正確，"
                      "以及 codeSw.h 的 CODESW_X2C_SCOPE_ENABLE 仍為 1。"
                      "注意 X2CScope 走 UART2 (RB8/RB9)，不是 RS485 的 UART1。")
            return False

        try:
            device = str(self._scope.get_device_info())
        except Exception as exc:  # noqa: BLE001 - 非致命
            device = f"(讀取失敗: {exc})"

        # 步驟 4 — 位址正確性哨兵。這是所有其他數字可信與否的前提，所以先驗它。
        signature = self._read_one(V.V_SIGNATURE)
        self._signature_ok = (signature is not None
                              and int(signature) == V.SIGNATURE_EXPECTED)

        info = {name: self._read_one(name) for name in V.INFO_VARS}

        self.post("connected", {
            "port": self.port, "baud": self.baud, "elf": self.elf,
            "elf_mtime": elf_mtime, "stale": stale, "device": device,
            "signature": signature, "signature_ok": self._signature_ok,
            "absent": absent, "info": info,
            "writes_allowed": self.allow_writes and self._signature_ok,
        })
        return True

    # -- 變數存取 ----------------------------------------------------------
    def _handle(self, name):
        """惰性解析並快取變數 handle。未知符號回 None (get_variable 不拋例外)。"""
        if name not in self._handles:
            try:
                self._handles[name] = self._scope.get_variable(name)
            except Exception:  # noqa: BLE001
                self._handles[name] = None
        return self._handles[name]

    def _read_one(self, name):
        """讀一顆變數。失敗回 None，並做失步復原。

        ⚠ **不能只靠 try/except。** pyx2cscope 的 Variable.get_value() 是
        `except Exception: logging.error(e); return None` —— 它把例外吞掉、回傳 None。
        所以讀取失敗在這裡看起來是「拿到 None」而不是「拋例外」。
        原本只處理例外的版本因此從來沒有走到失敗路徑：_err_streak 不增加、
        _last_error 是空的 (實機橫幅上顯示「最後一次的錯誤: (無)」就是這個 bug)、
        而且**清收接緩衝區完全沒有執行**，失步永遠修不好。
        """
        handle = self._handle(name)
        if handle is None:
            return None            # 這顆不在 build 裡 —— 不是讀取失敗，不計入
        error = None
        try:
            value = handle.get_value()
            if value is None:
                error = "get_value() 回傳 None (pyx2cscope 已吞掉底層例外)"
        except Exception as exc:  # noqa: BLE001
            value, error = None, "{}: {}".format(type(exc).__name__, exc)
        if error is not None:
            self._err_streak += 1
            self._last_error = error
            # 讀取失敗代表這個框架已經被放棄，而它剩下的位元組還躺在收接緩衝區裡。
            # mchplnet 的 LNetSerial.read() 不驗 SYN、盲目相信長度位元組，所以那些
            # 殘留位元組會讓**後續每一個**框架連鎖錯位 —— 一次逾時就變成永久失步。
            # 它自己沒有重新同步的機制，所以主機端必須在這裡清掉，
            # 把「永久失步」降級成「掉一個取樣」。
            self._flush_input()
            return None
        self._err_streak = 0
        self._ever_read_ok = True
        return value

    def _read_many(self, names):
        return {name: self._read_one(name) for name in names}

    # -- 框架對齊 ----------------------------------------------------------
    def _sentinel_state(self):
        """讀一次位址哨兵，回傳 "ok" / "desync" / "noreply"。

        **這三種狀態必須分開處理**，把後兩者混為一談是會出事的：

        * ``desync``  讀到值，但不是 0xA5C3 -> 框架真的錯位了，需要重新對齊。
        * ``noreply`` 讀取逾時或拋例外 -> 可能只是一次暫時的逾時 (主迴圈瞬間長停頓
          就會這樣)。這**不代表**框架錯位，拿它去觸發重開序列埠只會把一次暫時失誤
          放大成「每個週期都重開埠」的自我維持故障 —— 而每次重開又會讓接下來幾筆
          交易失敗，於是永遠出不來。
        """
        value = self._read_one(V.V_SIGNATURE)
        self._last_signature = value
        if value is None:
            return "noreply"
        try:
            return "ok" if int(value) == V.SIGNATURE_EXPECTED else "desync"
        except (TypeError, ValueError):
            return "desync"

    def _sentinel_ok(self):
        return self._sentinel_state() == "ok"

    def _drain_until_quiet(self, quiet_s=0.03, max_s=0.6):
        """讀掉並丟棄所有還在來的位元組，直到線上安靜 quiet_s 為止。回傳丟掉幾個。

        **這是 reset_input_buffer() 做不到的事，也是實機失敗的直接原因。**
        清緩衝區只丟掉**已經到達**的位元組。Scope 讀回一次要 253 byte，在 230400 baud
        下是 11 ms 的線上時間 —— 讀到一半逾時的話，主機清完緩衝區只花微秒，而目標端
        還在傳剩下的，那些會在清完之後陸續抵達。於是重試的請求與殘留尾巴混在一起，
        得到 "Received data size is invalid."，再重試也一樣 (實機出現過
        「第 2/20 個區塊重試 2 次仍失敗」)。

        回傳值本身就是診斷資訊：丟掉的位元組數告訴我們目標端當時到底還欠多少。
        """
        try:
            ser = self._scope.interface.serial
        except Exception:  # noqa: BLE001
            return 0
        drained = 0
        deadline = time.monotonic() + max_s
        last_data = time.monotonic()
        while time.monotonic() < deadline:
            try:
                pending = ser.in_waiting
            except Exception:  # noqa: BLE001 - 埠可能剛被關掉
                break
            if pending:
                try:
                    drained += len(ser.read(pending))
                except Exception:  # noqa: BLE001
                    break
                last_data = time.monotonic()
                continue
            if time.monotonic() - last_data >= quiet_s:
                break
            self.stop_event.wait(0.005)
        return drained

    def _recover_sync(self):
        """讓 LNet 框架重新對齊。成功回 True。

        兩階段，由弱到強：

        1. **清收接緩衝區**（清兩次，中間留時間）。只清一次會漏掉「還在路上」的
           位元組，它們清完之後才抵達，於是下一個框架照樣錯位。
        2. **關掉再重開序列埠**。清緩衝區只能丟掉**已經到達**的位元組；擷取失敗時
           目標端可能還在傳一個沒被讀完的陣列回應 (一次 253 byte)，那些位元組清了
           又來。重開埠是唯一能把驅動層緩衝與在途資料一起丟乾淨的方法。
        """
        # ⚠ 這裡**刻意只用便宜的 reset_input_buffer()**，不用 _drain_until_quiet()。
        #
        # 曾經改成 drain 過，結果讓情況變糟：沒開 Scope 也會在兩秒內斷線。原因是
        # drain 會**主動從序列埠讀取並丟棄** —— 若偵測到失步的那一刻線上其實有一個
        # 正常的回應在傳，drain 會把它整個吃掉，下一次讀取必定失敗 -> 又判定失步 ->
        # 又 drain，形成自我強化的迴圈。而且每次至少 30ms、最多 600ms。
        #
        # 253 byte 的殘留問題只存在於 Scope 讀回 (見 _read_scope_data)，那裡才需要
        # drain。遙測的回應只有幾個位元組，清緩衝區就夠 —— 這也是 115200 與 230400
        # 兩次實機驗證通過的版本。
        for _ in range(RESYNC_ATTEMPTS):
            self._flush_input()
            self.stop_event.wait(0.05)
            self._flush_input()
            if self._sentinel_ok():
                return True
        self._last_drained = 0
        # 重開埠有最短間隔限制，見 REOPEN_MIN_INTERVAL 的說明。
        now = time.monotonic()
        if now - self._last_reopen < REOPEN_MIN_INTERVAL:
            return False
        self._last_reopen = now
        if not self._reopen_port():
            return False
        # 重開之後給鏈路時間穩定再判斷。剛開好的埠前幾筆交易失敗是正常的，
        # 立刻判定失敗只會讓我們再重開一次。
        self.stop_event.wait(0.25)
        for _ in range(RESYNC_ATTEMPTS):
            self._flush_input()
            if self._sentinel_ok():
                return True
            self.stop_event.wait(0.1)
        return False

    def _reopen_port(self):
        """關掉再重開序列埠，強制得到一條乾淨的串流。"""
        try:
            interface = self._scope.interface
            interface.stop()
            self.stop_event.wait(0.15)      # 讓 Windows 真的放掉這個埠
            interface.start()
            self.stop_event.wait(0.05)
        except Exception as exc:  # noqa: BLE001
            self.post("status", f"重開序列埠失敗: {exc}")
            return False
        self.post("status", "序列埠已重開，正在確認框架對齊 …")
        return True

    def _handle_bad_sentinel(self, state):
        """哨兵不 ok 時的分流。這一輪的資料一律不發出去。

        "desync" 與 "noreply" 要走完全不同的路：前者是框架錯位，主機端清緩衝區
        或重開埠有機會修好；後者是目標端根本沒回話，主機端做什麼都沒用 ——
        對它套重開埠只會製造自我維持的故障 (重開讓下幾筆交易失敗 -> 又重開)。
        """
        if state == "desync":
            self._invalid_cycles += 1
            self._noreply_cycles = 0
            # 先用最便宜的手段：清一次緩衝區再驗一次 (~4ms)。
            #
            # 實機踩過：一偵測到失步就走完整條復原階梯 (3 次 flush+50ms 等待、
            # 重開序列埠、250ms 穩定、再 3 次驗證) 要花約 1 秒，而失步是每隔幾輪
            # 就會發生一次的 —— 結果輪詢率從 8Hz 掉到 1.2Hz，**復原動作本身變成了
            # 最大的效能問題**，而且狀態列一直卡在「序列埠已重開」。
            # 偶發失步用一次 flush 就修得好，貴的手段留給真的修不動的情況。
            if self._invalid_cycles < DESYNC_ESCALATE:
                self._flush_input()
                if self._sentinel_ok():
                    self._invalid_cycles = 0
                    self._set_signature(V.SIGNATURE_EXPECTED, True)
                return
            if self._recover_sync():
                self.post("status",
                          f"框架失步已修正 (丟棄 {self._invalid_cycles} 輪資料)")
                self._invalid_cycles = 0
                self._set_signature(V.SIGNATURE_EXPECTED, True)
            elif self._invalid_cycles >= INVALID_CYCLES_LIMIT:
                self._set_signature(self._last_signature, False)
            return

        # state == "noreply"
        self._noreply_cycles += 1
        if self._noreply_cycles < NOREPLY_WEDGE_LIMIT or self._wedge_reported:
            return
        # 目標端連續幾秒完全不回話。主機端已經沒有牌可以打了 —— 清緩衝區與重開埠
        # 都只動到主機這一側，碰不到目標端 X2CScope 的狀態機。誠實講出來，
        # 而不是無限重試裝作還在努力。
        self._wedge_reported = True
        self._set_signature(None, False)
        head = (f"目標端沒有回應 X2CScope（連續 {self._noreply_cycles} 輪讀不到任何值）。\n"
                f"最後一次的錯誤: {self._last_error or '(無)'}\n")
        if not self._ever_read_ok:
            # 從連線到現在一顆都沒讀成功 -> 幾乎確定是設定不符，不是目標端卡死。
            # 這裡叫人重置電源只會浪費時間。
            self.post("error", head +
                      "**從連線到現在沒有成功讀取過任何一顆變數**，所以這不是目標端卡死，"
                      f"而是設定不符 —— 最可能是 baud rate。\n"
                      f"主機目前用 {self.baud}；韌體的值在 diagnostics_x2cscope.c 的 "
                      f"X2C_BAUD_TARGET。\n"
                      f"韌體在 2026-08-25 由 115200 改成 {V.DEFAULT_BAUD}，"
                      "若板子還沒重新燒錄就會是這個症狀。\n"
                      "請先跑 check_link.py --scan：它會逐一試 baud 並告訴你哪一個通。")
            return
        self.post("error", head +
                  "曾經讀得到、後來停止回應 -> 目標端卡死。\n"
                  "主機端無法自行復原 —— 清收接緩衝區與重開序列埠都只動到 PC 這一側，"
                  "碰不到目標端 X2CScope 的狀態機。\n"
                  "**請重置控制器電源。** 之後按「連線」重新連上即可。\n"
                  "已知觸發條件: Scope 讀回在中途失敗 (pyx2cscope 會吞掉失敗的傳輸區塊"
                  "並繼續，目標端可能因此卡在「還要再送 N 個位元組」的狀態)。\n"
                  "降低發生機率: 取樣分頻調大、通道數減少、或先停用 Modbus 儀表。")

    def _set_signature(self, value, ok):
        """哨兵狀態變了才通知 GUI，避免每個週期都發同一個事件。"""
        if ok == self._signature_ok:
            return
        self._signature_ok = ok
        self.post("signature", {"signature": value, "ok": ok,
                                "at_connect": False})

    # -- 輪詢主迴圈 --------------------------------------------------------
    def _poll_loop(self):
        self.post("status", "已連線")
        cycle, rare_pos = 0, 0
        rate_mark, rate_count, rate = time.monotonic(), 0, 0.0

        while not self.stop_event.is_set():
            started = time.monotonic()
            self._drain_commands()
            if self.stop_event.is_set():
                break

            # 「是否有一個擷取工作在進行」—— 從狀態推導，而不是在每個進入/離開點各發
            # 一次事件 (那種寫法遲早會漏掉某條退出路徑，然後主頁繪圖永遠凍結)。
            #
            # 連續模式在兩次擷取之間會把 _scope_mode 放回 None (讓遙測跑一輪)，所以
            # 光看 _scope_mode 會快速跳動 -> 主頁繪圖跟著一下停一下動。把 _scope_continuous
            # 與 _scope_rearm 也算進來，整個連續擷取期間才會維持成一個穩定的「進行中」。
            active = (self._scope_mode is not None
                      or self._scope_continuous or self._scope_rearm)
            if active != self._scope_active_reported:
                self._scope_active_reported = active
                self.post("scope_active", active)

            # Scope 擷取進行中就完全不讀遙測。
            #
            # 原本是「照常讀，讓儀表板不要凍住」，那是錯的：擷取期間目標端同時在跑
            # 20kHz 的 ADC ISR 寫自己的緩衝區、主迴圈、以及回應主機每秒約 50 次的
            # LNet 來回。而 mchplnet 的 LNetSerial.read() 沒有重新同步機制 —— 它盲目
            # 相信長度位元組、不驗 SYN，所以只要有一個回應被截斷，殘留位元組就會讓
            # 後續每一個框架連鎖錯位。更糟的是 pyx2cscope 的 _read_array_chunks 會把
            # 每個 chunk 的讀取失敗**吞掉**再回傳短少的緩衝區，於是通道解交錯全部錯位、
            # 畫出一張看起來像真波形的垃圾圖。
            #
            # 停 1~2 秒的遙測，換不會說謊的波形。
            if self._scope_mode is not None:
                self._safe_service_scope()
                self.stop_event.wait(0.05)
                continue

            # ---- 哨兵當成每個週期的資料有效性閘門 ----------------------------
            # 這是本迴圈最重要的一段。串流失步**不會拋例外**：LNetSerial.read() 盲目
            # 相信長度位元組、不驗 SYN，錯位時它照樣讀出一個框架形狀的位元組塊並回傳
            # —— 只是讀到的是別的變數。所以「靠例外偵測」完全抓不到，畫面上會出現
            # Iq 變成速度、DC bus 變成 1300A 這種看起來像真資料的垃圾。
            #
            # g_u16X2cDiagSignature 是固定常數 0xA5C3，所以它是現成的框架對齊校驗碼。
            # 在讀值的**前後各驗一次**，兩次都對才把這一輪的資料發出去 —— 相當於給
            # 整個週期加一個 checksum。成本是每週期多 4ms (125ms 預算的 3%)。
            state = self._sentinel_state()
            if state != "ok":
                self._handle_bad_sentinel(state)
                cycle += 1
                self.stop_event.wait(max(0.0, self.interval
                                         - (time.monotonic() - started)))
                continue
            self._noreply_cycles = 0
            if self._wedge_reported:
                self._wedge_reported = False
                self.post("status", "目標端恢復回應")

            values = self._read_many(V.FAST_VARS)
            if cycle % V.SLOW_DIVIDER == 0:
                values.update(self._read_many(V.SLOW_VARS))

            # RARE_VARS 有 57 顆，每顆是一次獨立 LNet 來回 (~2ms)，一口氣讀完是 114ms
            # —— 捲動圖會每 20 個週期卡一下。切片輪流讀，完整掃完一輪的時間與
            # RARE_DIVIDER 的原意相同，但單一週期不再爆預算。
            slice_names = V.RARE_VARS[rare_pos:rare_pos + self.rare_slice]
            values.update(self._read_many(slice_names))
            rare_pos += self.rare_slice
            if rare_pos >= len(V.RARE_VARS):
                rare_pos = 0

            # 後驗。失步若發生在這一輪的讀取途中，前驗抓不到，只有後驗抓得到。
            post_state = self._sentinel_state()
            if post_state != "ok":
                self._handle_bad_sentinel(post_state)
                cycle += 1
                self.stop_event.wait(max(0.0, self.interval
                                         - (time.monotonic() - started)))
                continue        # 這一輪的值全部丟掉，不畫、不更新分頁
            self._invalid_cycles = 0
            if not self._signature_ok:
                self._set_signature(V.SIGNATURE_EXPECTED, True)
            values[V.V_SIGNATURE] = V.SIGNATURE_EXPECTED

            if self._err_streak >= ERROR_STREAK_LIMIT:
                self.post("error",
                          f"連續 {self._err_streak} 次讀取失敗，判定鏈路中斷。\n"
                          f"最後一次的錯誤: {self._last_error or '(無)'}\n"
                          f"目標端: {self._link_health()}\n"
                          "板子斷電/USB 被拔掉會是這樣；若板子還在跑，"
                          "多半是主迴圈長停頓讓回應逾時。")
                return

            rate_count += 1
            now = time.monotonic()
            if now - rate_mark >= 1.0:
                rate = rate_count / (now - rate_mark)
                rate_count, rate_mark = 0, now
            self.post("data", (values, rate, now))

            # 連續模式的重新武裝點：遙測這一輪已經跑完 (含哨兵前後驗)，所以哨兵閘門
            # 與捲動圖在兩次擷取之間都有機會更新。見 _service_scope 的說明。
            if self._scope_rearm and not self.stop_event.is_set():
                self._scope_rearm = False
                self._safe_arm_scope()

            self._safe_service_scope()

            cycle += 1
            # 已經花掉的時間從間隔裡扣掉，否則實際週期會是「間隔 + 讀取時間」，
            # 畫面上的 Hz 就會跟設定值對不上。
            self.stop_event.wait(max(0.0, self.interval - (time.monotonic() - started)))

    # -- 命令處理 ----------------------------------------------------------
    def _drain_commands(self):
        while True:
            try:
                kind, payload = self.commands.get_nowait()
            except queue.Empty:
                return
            try:
                self._handle_command(kind, payload)
            except Exception as exc:  # noqa: BLE001 - 單一命令失敗不該拖垮輪詢
                self.post("error", f"命令 {kind} 失敗: {exc}")

    def _handle_command(self, kind, payload):
        if kind == "write":
            self._do_write(*payload)
        elif kind == "scope_config":
            self._do_scope_config(payload)
        elif kind == "scope_start":
            self._do_scope_start(bool(payload))
        elif kind == "scope_abort":
            self._scope_mode = None
            self._scope_continuous = False
            self._scope_rearm = False
            self.post("scope_state", "已中止")
        elif kind == "read_writes":
            # 40 顆 x 約 2ms = 一次約 80ms，這一個輪詢週期會變長。可以接受：這是
            # 使用者明確按下按鈕才發生的一次性動作，不是背景輪詢。
            # 刻意不把可寫參數放進週期輪詢 —— 它們絕大多數只在開機初始化時被寫，
            # 一直重讀只是白花頻寬，捲動圖的時間解析度才是頻寬該花的地方。
            self.post("write_values", self._read_many(V.WRITE_VARS))
        elif kind == "interval":
            self.set_interval(payload)

    # -- 寫入 --------------------------------------------------------------
    def _do_write(self, name, value):
        """寫入後立刻回讀。

        回讀不是形式 —— persist == "loop" 的變數會被主迴圈在幾毫秒內蓋回去，
        回讀值就是「這個寫入到底有沒有留下來」的唯一證據。
        """
        if not self.allow_writes:
            self.post("write_done", (name, value, None, False, "此 session 為唯讀模式"))
            return
        if not self._signature_ok:
            # 位址哨兵不符時全域變數位址已經位移，寫入會落在錯誤的位址上。
            # 在實車上那不只是顯示錯，是真的會動到別的東西。
            self.post("write_done", (name, value, None, False,
                                     "位址哨兵不符，寫入已封鎖 (ELF 不對版)"))
            return
        handle = self._handle(name)
        if handle is None:
            self.post("write_done", (name, value, None, False, "變數不在這個 build 裡"))
            return
        try:
            handle.set_value(value)
        except Exception as exc:  # noqa: BLE001
            self.post("write_done", (name, value, None, False, f"寫入失敗: {exc}"))
            return
        actual = self._read_one(name)
        matched = actual is not None and abs(float(actual) - float(value)) < 1e-6
        note = "" if matched else "已被韌體覆寫"
        self.post("write_done", (name, value, actual, True, note))

    # -- Scope 擷取 --------------------------------------------------------
    def _do_scope_config(self, cfg):
        """套用通道 / 觸發 / 取樣分頻。cfg 由 scope_window.py 組出來。"""
        channels = tuple(cfg.get("channels") or ())
        if not channels:
            self.post("error", "Scope 至少要選一個通道")
            return
        if len(channels) > V.MAX_SCOPE_CHANNELS:
            self.post("error", f"Scope 最多 {V.MAX_SCOPE_CHANNELS} 個通道")
            return

        self._scope.clear_all_scope_channel()
        accepted = []
        for name in channels:
            handle = self._handle(name)
            if handle is None:
                self.post("error", f"通道 {name} 不在這個 build 裡，已略過")
                continue
            self._scope.add_scope_channel(handle)
            accepted.append(name)
        if not accepted:
            self.post("error", "沒有任何通道加入成功")
            return

        factor = max(1, int(cfg.get("sample_factor", 1)))
        self._scope.set_sample_time(factor)

        trigger = cfg.get("trigger")
        if trigger and trigger.get("variable") in accepted:
            from pyx2cscope.x2cscope import TriggerConfig
            handle = self._handle(trigger["variable"])
            self._scope.set_scope_trigger(TriggerConfig(
                variable=handle,
                trigger_level=float(trigger.get("level", 0)),
                trigger_mode=int(trigger.get("mode", 1)),
                trigger_delay=int(trigger.get("delay", 0)),
                trigger_edge=int(trigger.get("edge", 1)),
            ))
        else:
            self._scope.clear_trigger()

        self._scope_channels = tuple(accepted)
        self._scope_factor = factor
        self.post("scope_state", f"已配置 {len(accepted)} 通道，分頻 {factor}")

    def _do_scope_start(self, continuous):
        if not self._scope_channels:
            self.post("error", "請先配置 Scope 通道")
            return
        # 框架失步時擷取回來的一定是垃圾 (通道解交錯會整體錯位)，所以先確認對齊。
        if not self._sentinel_ok() and not self._recover_sync():
            self.post("scope_state",
                      "框架失步，無法擷取 —— 擷取回來的波形會是錯位的別的變數。"
                      "等狀態列顯示重新對齊後再試。")
            return
        self._scope_continuous = continuous
        self._scope_retry = 0
        self._arm_scope()

    def _flush_input(self):
        """丟掉序列埠收接緩衝區裡的殘留位元組。

        mchplnet 的 LNetSerial.read() 不驗 SYN、盲目相信長度位元組，所以一次被截斷的
        回應會留下尾巴，讓**後續每一個**框架都錯位。它自己沒有重新同步的機制。
        """
        try:
            self._scope.interface.serial.reset_input_buffer()
        except Exception as exc:  # noqa: BLE001
            # 只回報一次。靜默失敗最糟 —— 會變成「以為清了其實沒清」，
            # 然後花很久找為什麼失步一直修不好。
            if not self._flush_broken:
                self._flush_broken = True
                self.post("status", f"⚠ 無法清序列埠緩衝區 ({exc})，失步將無法自動修復")

    def _link_health(self):
        """讀回目標端的 FIFO 丟棄計數。擷取失敗時這是「是不是主迴圈卡住」的直接證據。"""
        names = ("g_u16X2cRxFifoDropCount", "g_u16X2cTxFifoDropCount",
                 "g_u16X2cUpdateSkipCount", "g_u16X2cRxErrCount")
        got = {n: self._read_one(n) for n in names}
        if all(v is None for v in got.values()):
            # 連計數器都讀不到，就不要假裝有量到 —— 那本身就是最重要的資訊。
            return "讀不到 (目標端沒有回應)"
        show = lambda n: "?" if got[n] is None else got[n]  # noqa: E731
        return (f"RX丟棄 {show('g_u16X2cRxFifoDropCount')} / "
                f"TX丟棄 {show('g_u16X2cTxFifoDropCount')} / "
                f"Update跳過 {show('g_u16X2cUpdateSkipCount')} / "
                f"RX錯誤 {show('g_u16X2cRxErrCount')}")

    def _safe_arm_scope(self):
        """從輪詢迴圈重新武裝。包一層是因為迴圈本體沒有 try —— Scope 的問題不准
        弄掉整條鏈路 (同 _safe_service_scope 的理由)。"""
        try:
            self._arm_scope()
        except Exception as exc:  # noqa: BLE001
            self._scope_mode = None
            self._scope_continuous = False
            self._flush_input()
            self.post("scope_state", f"連續擷取中止: {exc}（連線維持）")

    def _safe_service_scope(self):
        """Scope 的任何問題都不准弄掉整條鏈路。

        _service_scope 會走到 request_scope_data() 這種真的碰序列埠的呼叫，那些會拋
        例外。少了這一層，一次擷取失敗 -> 重試 -> 重試時 arm 又失敗，例外就會一路穿出
        _poll_loop、殺掉 worker，使用者看到的是「斷線」而不是「這次擷取失敗」。
        遙測與 Scope 是兩件獨立的事，不該互相拖累。
        """
        try:
            self._service_scope()
        except Exception as exc:  # noqa: BLE001
            self._scope_mode = None
            self._scope_continuous = False
            self._scope_rearm = False
            self._flush_input()
            self.post("scope_state", f"擷取中止: {exc}（連線維持）")

    def _arm_scope(self):
        self._flush_input()
        try:
            self._scope.request_scope_data()
        except Exception as exc:  # noqa: BLE001
            # 這是真的序列 I/O，會失敗。失敗就結束這次擷取，不要往上拋。
            self._scope_mode = None
            self._scope_continuous = False
            self.post("scope_state", f"無法啟動擷取: {exc}（連線維持）")
            return
        self._scope_mode = "sampling"
        # 逾時保護：觸發條件可能永遠不成立 (例如在停車狀態下等速度上升緣)。
        # 沒有這個上限，「等待觸發」會看起來像當掉。
        self._scope_deadline = time.monotonic() + 30.0
        self.post("scope_state", "等待觸發 / 取樣中 …（遙測暫停）")

    def _scope_failed(self, message):
        """擷取失敗的統一出口。

        **先把框架修好，再做其他事。** 順序很重要：擷取失敗通常伴隨串流失步，
        在失步狀態下讀健康計數只會得到「讀不到」(先前就是這樣)，而且更糟的是
        —— 把一條失步的鏈路直接交還給輪詢迴圈，主頁的每個數值都會變成錯位的
        別的變數。修好之後計數器才讀得到，主頁也才拿得到可信的值。
        """
        recovered = self._recover_sync()
        if self._scope_retry < SCOPE_MAX_RETRY and recovered:
            self._scope_retry += 1
            self.post("scope_state",
                      f"{message} — 已重新對齊，重試 {self._scope_retry}/{SCOPE_MAX_RETRY}")
            self._arm_scope()
            return
        self._scope_mode = None
        self._scope_continuous = False
        self._scope_rearm = False
        if recovered:
            self._set_signature(V.SIGNATURE_EXPECTED, True)
            tail = ("目標端鏈路健康: {}\n".format(self._link_health())
                    + "丟棄計數在增加就代表主迴圈曾長停頓；可試著把取樣分頻調大、"
                      "減少通道數，或先停用 Modbus 儀表以降低主迴圈負擔。\n"
                      "連線與主頁數值不受影響。")
        else:
            tail = (f"框架仍未對齊（排空 {self._last_drained} byte 殘留、"
                    "並重開序列埠）。主頁數值會暫時停止更新，"
                    "直到重新對齊 —— 停止更新是刻意的，顯示錯位的值比不顯示更糟。\n"
                    "若一直不恢復，請換一條 USB 線或換一個埠。")
        self.post("scope_state",
                  f"{message}（已重試 {self._scope_retry} 次）\n{tail}")

    def _service_scope(self):
        """檢查擷取進度。擷取期間輪詢迴圈不讀遙測，所以這裡獨佔傳輸。"""
        if self._scope_mode != "sampling":
            return
        try:
            ready = self._scope.is_scope_data_ready()
        except Exception as exc:  # noqa: BLE001
            self._scope_failed(f"擷取失敗: {exc}")
            return
        if not ready:
            if time.monotonic() > self._scope_deadline:
                self._scope_mode = None
                self._scope_continuous = False
                self.post("scope_state", "逾時 — 觸發條件可能沒有成立")
            return

        self._scope_mode = "reading"
        self.post("scope_state", "讀回緩衝區 …")
        try:
            data = self._read_scope_data()
            trigger_index = self._scope.get_trigger_position()
        except Exception as exc:  # noqa: BLE001
            self._scope_failed(f"讀回失敗: {exc}")
            return

        # 完整性檢查。pyx2cscope 的 _read_array_chunks 會靜默吞掉單一 chunk 的讀取
        # 失敗 (每次少 253 byte) 再回傳短少的緩衝區 —— 少的位元組讓通道解交錯整體
        # 錯位，畫出來是一張看起來很像真波形的垃圾。所以短少就當失敗，不畫。
        expected = self._expected_points()
        actual = min((len(series) for series in data.values()), default=0)
        if expected and actual < expected * SCOPE_MIN_COMPLETENESS:
            self._scope_failed(
                f"讀回不完整: 只拿到 {actual} / 預期 {expected} 點"
                f" (少了約 {(expected - actual) * self._dataset_size() // 253} 個傳輸區塊)")
            return

        # 讀完 20 個連續區塊之後再驗一次框架對齊。位元組數對得上並不代表內容對得上
        # —— 失步時每個區塊都「讀得到」，只是讀到偏移過的位址。哨兵是唯一能分辨
        # 「完整而正確」與「完整但整體錯位」的東西。
        if not self._sentinel_ok():
            self._scope_failed("讀回後框架失步，本次波形作廢 (內容會是錯位的別的變數)")
            return

        self.post("scope_data", {
            "channels": data,
            "trigger_index": trigger_index,
            "sample_factor": self._scope_factor,
            "dt_us": SCOPE_BASE_DT_US * self._scope_factor,
            "points": actual,
            "expected": expected,
        })
        self._scope_retry = 0
        self._scope_mode = None
        if self._scope_continuous and not self.stop_event.is_set():
            # **不要**在這裡直接 _arm_scope()。連續模式若一擷取完就立刻重新武裝，
            # _scope_mode 就永遠不是 None，輪詢迴圈那個 `continue` 會讓遙測**完全**
            # 不執行 —— 不只捲動圖凍結，連哨兵閘門也停擺，於是失步偵測、資料有效性
            # 檢查、鏈路健康全部看不到。出問題時最需要看到的東西恰好全沒了。
            # 改成先放手一輪讓遙測跑完，再由輪詢迴圈重新武裝。
            self._scope_rearm = True
            self.post("scope_state", f"完成 — {actual} 點/通道（連續：等遙測一輪）")
        else:
            self.post("scope_state", f"完成 — {actual} 點/通道")

    def _read_scope_data(self):
        """讀回 Scope 緩衝區，取代 pyx2cscope 的 get_scope_channel_data()。

        **為什麼不用它內建的：** pyx2cscope 的 _read_array_chunks() 對每一個 253 byte
        區塊是 `except Exception: logging.error(...)` 然後**繼續下一個區塊** ——
        失敗的那 253 byte 就這樣消失，回傳一個短少的緩衝區，於是通道解交錯整體錯位，
        畫出一張看起來像真波形的垃圾 (實機出現過 347/625 點、Speed 讀到 -18,944)。
        更糟的是它一路讀到底，目標端可能被留在「我還要再送 N 個位元組」的狀態。

        這裡改成**失敗就重試同一個區塊**，重試用盡才整批放棄 —— 要嘛拿到完整正確的
        資料，要嘛乾脆失敗，不會回傳一半。解交錯與觸發旋轉仍沿用 pyx2cscope 的實作
        (私有方法，但那兩段邏輯沒問題，重寫只會多一份要維護的東西)。
        """
        scope = self._scope
        # 讀回期間放寬逐位元組逾時。目標端在 bulk 傳輸時是「慢但仍在進展」——
        # 函式庫受 TX FIFO 節流影響會斷斷續續地吐，而 LNetSerial 每個位元組只給 1 秒，
        # 於是一個還在正常進行的回應會被誤判成失敗 (實機出現過
        # 「第 15/20 個區塊: ord() expected a character, but string of length 0 found」)。
        # 只在讀回期間放寬 —— 遙測仍然維持 1 秒，才不會讓真正的斷線變得遲鈍。
        with self._read_timeout(SCOPE_READ_TIMEOUT_S):
            return self._read_scope_chunks(scope)

    @contextlib.contextmanager
    def _read_timeout(self, seconds):
        """暫時改大 pyserial 的讀取逾時，離開時還原。"""
        ser = None
        original = None
        try:
            ser = self._scope.interface.serial
            original = ser.timeout
            ser.timeout = seconds
        except Exception:  # noqa: BLE001 - 私有路徑；改不到就照原設定跑
            ser = None
        try:
            yield
        finally:
            if ser is not None:
                try:
                    ser.timeout = original
                except Exception:  # noqa: BLE001
                    pass

    def _read_scope_chunks(self, scope):
        used = int(scope._calc_sda_used_length())
        base = scope.lnet.scope_data.data_array_address
        chunk = 253                    # 扣掉 Service-ID / Error-ID 後的滿載長度
        data = bytearray()
        offset = 0
        drained_total = 0
        while offset < used:
            size = min(chunk, used - offset)
            for attempt in range(SCOPE_CHUNK_RETRY + 1):
                try:
                    part = scope.lnet.get_ram_array(base + offset, size, 1)
                    if part is None or len(part) != size:
                        raise ValueError(
                            f"區塊長度不符: 收到 {0 if part is None else len(part)}"
                            f" / 預期 {size}")
                    data.extend(part)
                    break
                except Exception as exc:  # noqa: BLE001
                    if attempt >= SCOPE_CHUNK_RETRY:
                        raise RuntimeError(
                            f"第 {offset // chunk + 1}/{-(-used // chunk)} 個區塊"
                            f"重試 {SCOPE_CHUNK_RETRY} 次仍失敗: {exc}"
                            f"（每次重試前排空了 {drained_total} byte 的殘留）"
                        ) from exc
                    # 失敗的區塊留下未讀完的殘留位元組。**必須排空到線上安靜**，
                    # 只清一次緩衝區來不及 —— 目標端還在傳的那 253 byte 會在清完
                    # 之後才到，讓重試同樣得到 "Received data size is invalid."。
                    drained_total += self._drain_until_quiet()
            offset += size
        channels = scope._sort_channel_data(data)
        return scope._filter_channels(channels)

    def _dataset_size(self):
        """一組取樣 (所有通道各一點) 的位元組數。"""
        try:
            return max(1, int(self._scope.scope_setup.get_dataset_size()))
        except Exception:  # noqa: BLE001
            return max(1, len(self._scope_channels) * V.SCOPE_DEFAULT_SAMPLE_BYTES)

    def _expected_points(self):
        """目標端緩衝區裝得下的每通道點數。用目標回報的大小，不用主機的估算常數。"""
        try:
            size = int(self._scope.lnet.scope_data.data_array_size)
        except Exception:  # noqa: BLE001
            size = V.X2C_BUFFER_BYTES
        return size // self._dataset_size()


# ---------------------------------------------------------------------------
# 模擬鏈路
# ---------------------------------------------------------------------------
class DemoLink(threading.Thread):
    """不開序列埠的模擬鏈路，介面與 Link 相同。

    存在的理由是「無車也要能驗版面與格式器」，不是「假裝有量到東西」—— 所以
    connected 事件裡的 device 字串就寫明是模擬，GUI 會把它顯示在標題列上。

    合成的情境是一次完整的起步→巡航→放油門→停車→夾煞，因為那是這台控制器最常
    被追問的一段 (斜率限制、UVW 鎖定、EMB 夾煞原因都在裡面)。

    註：情境假設**有接 Modbus 儀表**，所以 sSharedData 的欄位 (輪徑/每轉脈衝數/
    電池型別/控制模式/助力段位) 都是非零。實車沒接儀表時那些欄位會全是 0 ——
    儀表板上這些欄位標了「(儀表)」就是為了讓那個 0 讀起來是「沒有資料」而不是
    「控制器設定成 0」。
    """

    CYCLE_S = 24.0   # 一輪情境的長度

    # 可寫參數的模擬值，取自韌體實際的初始值 (main.c 的宣告與 userparms.h)，
    # 這樣 demo 模式的「調校」分頁看起來就是真的那組數字，不是一排零。
    # Q15(x) = round(x * 32768)。
    WRITE_DEFAULTS = {
        # 速度環 PI —— main.c:2854 的 SPEEDCNTR_*TERM
        "piInputOmega.piState.kp": 5000,
        "piInputOmega.piState.ki": 20,
        "piInputOmega.piState.kc": 32735,      # Q15(0.999)
        "piInputOmega.piState.outMax": 19000,  # 溫控任務每個 tick 重算
        # 電流環 PI —— main.c:2845/2836 的 Q15(0.02) / Q15(0.001)
        "piInputIq.piState.kp": 655,
        "piInputIq.piState.ki": 33,
        "piInputId.piState.kp": 655,
        "piInputId.piState.ki": 33,
        # 加減速 —— userparms.h ACC_SET / DE_ACC_SET / SPEED_SLOP_CNTR_SET
        "AccSet": 50,
        "DeAccSet": 50,
        "SpeedSlopCntrSet": 20,
        "ReferenceRAWSetStep": 10,             # main.c:348
        # 電流 / 轉矩上限
        "TorqMode_IqMax": 19005,               # main.c:366 Q15(0.58)
        "IqSquare.RatedIq": 9830,
        "IqSquare.OverCurrent": 22937,
        "IqSquare.Limit": 30000,
        # 保護門檻
        "MOSFET_OverTemp": 220,                # main.c:387 OVERTEMP_MOSFET_90
        "UVWLockSpeed": 1966,                  # main.c:361 Q15(0.06)
        "MotorAlignLockTime": 2000,
        "SpeedCtrlLimit": 12670,               # Q15_MAXSPEED_CtrlMode_2
        "SpeedModeCtrlLimit": 21845,           # Q15_MAXSPEED_REF_LIMIT
        # 再生制動 —— main.c:352/353
        "ReGenTorq": 3277,                     # Q15(0.1)
        "ReGenSpeed": 0,
        # 起步 / 煞車門檻
        "MotorStartSpeed": 328,                # main.c:371 Q15(0.01)
        "MotorStartSpeedPulses": 3,            # main.c:386
        "BrakeStartSpeed": 1200,
        "BrakeStopSpeed": 400,
        # 換相 —— main.c:231
        "HallOffset": 0,
        # 危險組
        "uGF.ReGenEnable": 0,
        "uGF.CtrlMode": 0,
        "uGF.DriveMode": 1,
        "uGF.ReGenMode": 0,
    }

    # 可寫但不在儀表板欄位裡的「活」變數：從情境的其他軌跡導出，這樣讀回來的值
    # 與畫面上的波形一致。用靜態預設值會讓它在車子明明在跑的時候顯示 0。
    DERIVED_WRITES = {
        # ReferenceRAW 是限斜率**前**的速度環命令源，情境裡與 cmd_out 同一條軌跡。
        "ReferenceRAW": lambda live: live.get("g_i16ScopeCmdOut"),
    }

    def __init__(self, port, baud, elf, interval_ms, events,
                 rare_slice=8, allow_writes=True, bad_signature=False):
        super().__init__(daemon=True, name="x2c-demo")
        self.interval = max(0.05, interval_ms / 1000.0)
        self.events = events
        self.commands = queue.Queue()
        self.stop_event = threading.Event()
        self.allow_writes = allow_writes
        self.bad_signature = bad_signature
        self.elf = str(elf)
        self._written = {}
        self._last_scenario = {}
        self._scope_channels = ()
        self._scope_factor = 1
        self._scope_pending = False
        self._scope_continuous = False
        self._scope_active_reported = False
        self._scope_next_at = 0.0

    submit = Link.submit
    stop = Link.stop
    post = Link.post
    set_interval = Link.set_interval

    def run(self):
        signature = 0x0000 if self.bad_signature else V.SIGNATURE_EXPECTED
        # 先填一次，免得使用者在第一個輪詢週期完成前就按下「讀取目前值」時拿到空的。
        self._last_scenario = self._scenario(0.0)
        self.post("connected", {
            "port": "DEMO", "baud": 0, "elf": self.elf,
            "elf_mtime": time.time(), "stale": False,
            "device": "模擬模式 — 沒有連上任何硬體",
            "signature": signature,
            "signature_ok": not self.bad_signature,
            "absent": [], "info": self._scenario(0.0),
            "writes_allowed": self.allow_writes and not self.bad_signature,
        })
        self.post("status", "模擬模式")
        start = time.monotonic()
        rate_mark, rate_count, rate = start, 0, 0.0
        while not self.stop_event.is_set():
            began = time.monotonic()
            self._drain_commands()
            if self.stop_event.is_set():
                break
            # 與真實 Link 一樣從狀態推導，讓 --demo 真的驗得到主頁的凍結/恢復。
            active = self._scope_pending or self._scope_continuous
            if active != self._scope_active_reported:
                self._scope_active_reported = active
                self.post("scope_active", active)
            values = self._scenario(began - start)
            self._last_scenario = values
            rate_count += 1
            now = time.monotonic()
            if now - rate_mark >= 1.0:
                rate = rate_count / (now - rate_mark)
                rate_count, rate_mark = 0, now
            self.post("data", (values, rate, now))
            if self._scope_pending and began >= self._scope_next_at:
                # 實機一次滿緩衝擷取要 1~2 秒 (請求 -> 等觸發 -> 分塊讀回)。原本 demo
                # 是**每個輪詢週期**就送一筆 = 12.5 Hz，比實機快兩個數量級：既不像實機，
                # 又因為每筆都要重繪 (~27ms) 而把 Tk 餵到飽和 —— 實測連續模式下單次
                # Tk.update() 被卡住 51 秒。
                self._scope_next_at = began + DEMO_SCOPE_PERIOD_S
                self._emit_scope(began - start)
            self.stop_event.wait(max(0.0, self.interval - (time.monotonic() - began)))
        self.post("disconnected")

    def _drain_commands(self):
        while True:
            try:
                kind, payload = self.commands.get_nowait()
            except queue.Empty:
                return
            if kind == "write":
                name, value = payload
                if not self.allow_writes or self.bad_signature:
                    self.post("write_done", (name, value, None, False,
                                             "位址哨兵不符，寫入已封鎖 (ELF 不對版)"
                                             if self.bad_signature else "唯讀模式"))
                    continue
                # persist == "loop" 的變數在模擬裡也會被「韌體」蓋回去，這樣那條
                # 警告在 demo 模式下就看得到，不用接上車才發現。
                persist = dict((item[0], item[3])
                               for item in V.WRITE_TUNING + V.WRITE_DANGER).get(name)
                if persist == "loop":
                    self.post("write_done", (name, value, 0, True, "已被韌體覆寫"))
                else:
                    self._written[name] = value
                    self.post("write_done", (name, value, value, True, ""))
            elif kind == "scope_config":
                self._scope_channels = tuple(payload.get("channels") or ())
                self._scope_factor = max(1, int(payload.get("sample_factor", 1)))
                self.post("scope_state",
                          f"已配置 {len(self._scope_channels)} 通道 (模擬)")
            elif kind == "scope_start":
                self._scope_continuous = bool(payload)
                self._scope_pending = True
                self._scope_next_at = 0.0      # 第一筆不等，按下就出波形
                self.post("scope_state", "取樣中 (模擬) …")
            elif kind == "scope_abort":
                self._scope_pending = self._scope_continuous = False
                self.post("scope_state", "已中止")
            elif kind == "read_writes":
                # 有些可寫變數也在遙測裡 (outMax、AssistLevel、uGF 旗標…)，那些要拿
                # 情境**當下**的值。用 _last_scenario 而不是重算：重算會用到不同的
                # 相位，讀回來的值就會跟畫面上的波形對不起來。
                live = self._last_scenario
                values = {}
                for name in V.WRITE_VARS:
                    if name in live:
                        values[name] = live[name]
                    elif name in self.DERIVED_WRITES:
                        values[name] = self.DERIVED_WRITES[name](live)
                    else:
                        values[name] = self.WRITE_DEFAULTS.get(name)
                values.update({k: v for k, v in self._written.items()
                               if k in V.WRITE_VARS})
                self.post("write_values", values)
            elif kind == "interval":
                self.set_interval(payload)

    # -- 合成訊號 ----------------------------------------------------------
    def _scenario(self, t):
        """一輪 24 秒的起步→巡航→放油門→停車→夾煞。"""
        import math

        phase = (t % self.CYCLE_S) / self.CYCLE_S
        # 油門：0-15% 死區, 15-35% 拉起, 35-60% 全開, 60-70% 放掉, 之後歸零
        if phase < 0.15:
            throttle = 0.0
        elif phase < 0.35:
            throttle = (phase - 0.15) / 0.20
        elif phase < 0.60:
            throttle = 1.0
        elif phase < 0.70:
            throttle = 1.0 - (phase - 0.60) / 0.10
        else:
            throttle = 0.0

        target = throttle * 18_000
        # 斜率限制：命令追不上階躍，這正是 ①→③ 要看的差距
        lag = 0.55 if throttle > 0 else 0.30
        cmd_out = target * lag + 2_500 * throttle
        # 量測再落後一點，並加上量測雜訊
        speed = cmd_out * 0.93 + 180 * math.sin(t * 7.0)
        if throttle == 0 and phase > 0.72:
            speed = max(0.0, speed * 0.0)

        moving = speed > 400
        stopped_and_locked = (not moving) and phase > 0.75
        throttle_mv = int(V.THROTTLE_MIN_MV
                          + throttle * (V.THROTTLE_MAX_MV - V.THROTTLE_MIN_MV))
        iq = 9_000 * throttle + 1_200 * math.sin(t * 3.0)
        ibus_a_x10 = int(iq / V.IABC_COUNTS_PER_AMP * 10 * 0.8)
        kmh = abs(V.DEFAULT_SCALE.q15_to_kmh(speed) or 0.0)
        hall_period = int(V.DEFAULT_SCALE.hall_min_period * 32768 / speed) if speed > 400 else 0
        laps = int(t / self.CYCLE_S)

        values = {
            # -- FAST --
            "Speed": int(speed),
            # 進 PI 的命令：斜坡後、夾限後。比 cmd_out 再落後一點，這樣「命令 − 量測」
            # 的誤差圖上看得到速度環在追。
            "piInputOmega.inReference": int(cmd_out * 0.97),
            "g_i16ScopeCmdTarget": int(target),
            "g_i16ScopeCmdOut": int(cmd_out),
            "IbusAmpX10": ibus_a_x10,
            "idq.q": int(iq),
            # -- 執行狀態 --
            "uGF.RunMotor": 1 if moving or throttle > 0 else 0,
            "uGF.Fault": 0,
            "uGF.UVWLock": 1 if stopped_and_locked else 0,
            "uGF.ReGenFlag": 0,
            "uGF.Direction": 0,
            "uGF.DirectionDefault": 0,
            "uGF.BrakeSWOn": 0,
            "uGF.CtrlMode": 0,
            "g_stSystemData.bMotorStop": 0 if moving else 1,
            "g_stSystemData.bMotorDirection": 0,
            "g_u8ELockActive": 0,
            # -- 命令鏈 --
            "g_i16ScopeCmdRateLim": int(cmd_out),
            "ReferenceRAWADC": int(target),
            "ctrlParm.qVelRef": int(cmd_out * 0.97),
            # 速度模式下韌體從來不寫它 -> 恆為 0。demo 如實反映，否則這個「一直是 0」
            # 的事實就會被模擬資料蓋掉。
            "ReferenceRAWSet": 0,
            "g_stSystemData.i16TargetRpm": int(target),
            "g_stSystemData.i16CurrentRpm": int(speed),
            # 一階低通落後於 Speed，所以刻意乘一個小於 1 的係數，讓兩條軌跡分得開。
            "FilteredSpeed": int(speed * 0.90),
            "g_stSystemData.i16SpeedFiltered": abs(int(speed * 0.90)),
            "g_stSystemData.sSharedData.u16CurrentSpeedKmh_x10": int(kmh * 10),
            "piInputOmega.piState.outMax": 19_000,
            "piOutputOmega.out": int(iq),
            # -- 電源 / 溫度 --
            "g_stSystemData.u16BatteryVoltage": int(4_820 - 90 * throttle),
            # BMS 回報值 (0.1V) 與 ADC 換算值 (0.01V) 刻意給同一個電壓的兩種刻度，
            # 這樣「兩條路徑對不對得上」在 demo 模式下就看得出來。
            "g_stSystemData.sBatteryData.u16Voltage_x10": int(482 - 9 * throttle),
            "g_stSystemData.u16BatteryPercent": 78,
            "sst_currentBatteryInfo.u8StateOfChargePercent": 78,
            "sst_currentBatteryInfo.eSystemStatus": 0,
            "IbusMeanQ15": int(iq * 0.8),
            "g_stSystemData.u16ControllerTemp": int(312 + 90 * throttle),
            "s_temp_controller_currentTempC": int(3_120 + 900 * throttle),
            "g_stSystemData.u16MotorTemp": int(295 + 60 * throttle),
            "s_temp_currentZone": 0,
            "seCurrentMotorTempStatus": 0,
            "g_stSystemData.bControllerIsOverTemp": 0,
            "g_stSystemData.bControllerIsOverLoad": 0,
            "s_bMotorStallCurrentLimited": 0,
            "g_stSystemData.bBatteryShouldProhibit": 0,
            "g_stSystemData.bBatteryVoltageValid": 1,
            "g_stSystemData.bControllerTempValid": 1,
            "g_stSystemData.bMotorTempValid": 1,
            # -- 輸入 --
            "g_stSystemData.u16ThrottleVRMv": throttle_mv,
            "g_stSystemData.u16ThrottleVRRaw": int(throttle_mv * V.ADC_FULL_SCALE
                                                   / V.THROTTLE_FS_MV),
            "g_stSystemData.u16ThrottleVR": int(throttle * 32_767),
            "g_stSystemData.bThrottleVRValid": 1,
            "g_stSystemData.bThrottleVrRelease": 1 if throttle == 0 else 0,
            "g_stSystemData.u16IEMBMv": 4_100 if stopped_and_locked else 1_050,
            "g_stSystemData.sSharedData.u8AssistLevel": 3,
            "g_stSystemData.sSharedData.eControlMode": 1,
            "g_stSystemData.sSharedData.eAccelCurve": 2,
            # -- 電磁煞車 --
            "s_eCurrentState": 1 if stopped_and_locked else 2,
            "g_u8EmbLockReason": 1 if stopped_and_locked else 0,
            "s_eLastLockReason": 1,
            "s_u16EmbPwmDuty": 0 if stopped_and_locked else V.EMB_PWM_PERIOD,
            "s_u16EmbPwmTicksLeft": 0 if stopped_and_locked else 120,
            "g_u16EmbLockCntUvwDelay": laps,
            "g_u16EmbLockCntFailsafe": 0,
            "g_u16EmbLockCntDownhill": 0,
            "g_u16EmbLockCntRollback": 0,
            "g_u16EmbLockCntOther": 0,
            "g_i16EmbLockPulses": 0,
            "g_u16EmbLockPeriod": 0,
            "g_bEmbRollbackArmed": 0,
            "g_u8EmbNoDecelCnt": 0,
            "g_u8EmbRevEdgeCnt": 0,
            "g_i16EmbZeroCmdEdgeCnt": laps,
            # -- 感測 / 換相 --
            "HallPeriod": hall_period,
            "HallPulsesLatch": int(V.DEFAULT_SCALE.q15_to_motor_rpm(speed)
                                   * V.HALL_EDGES_PER_REV / 600.0),
            "HallState": 1 + int(t * 13) % 6,
            "thetaElectrical": int((t * 9_000) % 65_536) - 32_768,
            # -- 故障 / 警報 --
            "su32_err_activeAlarmsBitmask": 0,
            "FaultFlags.MotorStall": 0,
            "FaultFlags.Undervoltage": 0,
            "FaultFlags.Overvoltage": 0,
            "FaultFlags.MOSOverHeat": 0,
            "FaultFlags.MCUOverHeat": 0,
            # -- 鏈路健康 --
            V.V_SIGNATURE: 0x0000 if self.bad_signature else V.SIGNATURE_EXPECTED,
            "g_u32MainLoopHz": 42_835,
            "g_u16X2cRxErrCount": 0,
            "g_u16X2cRxOerrCount": 0,
            "g_u16X2cRxFerrCount": 0,
            "g_u16X2cRxFifoDropCount": 0,
            "g_u16X2cRxFifoMaxUsed": 12,
            "g_u16X2cTxFifoDropCount": 0,
            "g_u16X2cTxFifoMaxUsed": 96,
            "g_u16X2cUpdateSkipCount": 0,
            "g_u16X2cCommunicatePassMax": 3,
            "g_u16ModbusState": 2,
            "g_u16ModbusParseFailCount": 0,
            "g_u16ModbusGuardErrorCount": 0,
            "g_u16ModbusLastRxLen": 21,
            # -- 組態 --
            # 控制器自己的組態 —— 用 s_logic_motor.h 的實際編譯期預設值。
            "s_currentMotorConfig.u8PolePairs": V.MOTOR_POLE_PAIRS,
            "s_currentMotorConfig.u16HallPPR": 610,   # LOGIC_MOTOR_DEFAULT_HALL_PPR (PPR x10)
            "s_currentMotorConfig.u16WheelDimensionInches": V.WHEEL_DIAMETER_INCH_X10,
            "s_currentMotorConfig.u8ExternalSensorPPR": 6,   # 1~6 Pulse/R
            "s_currentMotorConfig.eSpeedSource": 0,
            "g_stSystemData.sSharedData.eBatteryType": 2,
            # 控制器對外回報的版本，跟著 s_modbus_decode.h 的 FW_VER_* 走：
            # 0/2/5 -> (0<<8)|(2<<4)|5 = 0x0025 = V0.25。
            # 改韌體版號時這裡要一起改，否則 demo 會顯示與實機不同的版本。
            "g_stModbusAllData.uPcGuiData.u16Regs[21]": 0x0025,
            "g_stSystemData.sSharedData.u8PulsePerRev": 1,
            "g_stSystemData.sSharedData.u8WheelDiameterInches": 8,
            "HallMinPeriod": V.DEFAULT_SCALE.hall_min_period,
            "pwmPeriod": 4_999,          # 100MHz/(4999+1) = 20kHz
            "MOSFET_OverTemp": 220,      # OVERTEMP_MOSFET_90 (ADC raw，NTC 故數值越小越熱)
        }
        values.update(self._written)
        return values

    def _emit_scope(self, t):
        """合成一次擷取。取樣率是 ISR 速率，所以看得到輪詢遙測看不到的細節。"""
        import math

        points = max(64, V.X2C_BUFFER_BYTES
                     // (max(1, len(self._scope_channels))
                         * V.SCOPE_DEFAULT_SAMPLE_BYTES))
        dt_us = SCOPE_BASE_DT_US * self._scope_factor
        channels = {}
        for index, name in enumerate(self._scope_channels):
            series = []
            for i in range(points):
                # 一次油門階躍的響應：命令是階躍，實際值是一階上升 + 漣波。
                # 每個通道給不同的時間常數與相位，這樣疊圖看起來像真的多通道量測。
                x = i / points
                step = 0.0 if x < 0.25 else 1.0
                tau = 0.06 + 0.05 * index
                risen = step * (1.0 - math.exp(-max(0.0, x - 0.25) / tau))
                ripple = 0.02 * math.sin(2 * math.pi * (i / 11.0 + index))
                series.append(int((risen + ripple) * (12_000 - 900 * index)))
            channels[name] = series
        self.post("scope_data", {
            "channels": channels,
            "trigger_index": int(points * 0.25),
            "sample_factor": self._scope_factor,
            "dt_us": dt_us,
            "points": points,
            "expected": points,
        })
        if not self._scope_continuous:
            self._scope_pending = False
            self.post("scope_state", "完成 (模擬)")


def force_utf8_console():
    """把 stdout/stderr 切成 UTF-8。

    本專案的主機端工具輸出幾乎全是中文，而 Windows 主控台的 Python 預設編碼是
    cp950 或 cp1252 —— 實測在 cp1252 下光是印一行說明就會 UnicodeEncodeError 當掉，
    使用者看到的是一串 codecs traceback，看起來像工具壞了而不是編碼問題。
    連 argparse 的 --help 都會炸，因為 docstring 也是中文。
    """
    import sys

    for stream in (sys.stdout, sys.stderr):
        try:
            stream.reconfigure(encoding="utf-8")
        except (AttributeError, OSError, ValueError):
            pass          # 被重導向到不支援 reconfigure 的物件時就算了


def load_elf_scalars_async(elf, events):
    """在自己的短命執行緒解析 ELF 符號表，結果以 ("elf_scalars", [...]) 送回。

    刻意**不**走 Link 的 command queue。這是純離線的 DWARF 解析 —— 實測 1.4 秒、
    7600 個符號、完全不碰序列埠 —— 排進 worker 會讓遙測輪詢連同後續的 Scope 命令
    一起停住一秒半，開一次 Scope 視窗整個儀表板就凍一下。
    """
    def work():
        try:
            events.put(("elf_scalars", V.elf_scalar_variables(elf)))
        except Exception as exc:  # noqa: BLE001 - 回報到 GUI
            events.put(("error", f"無法解析 ELF 符號表: {exc}"))

    threading.Thread(target=work, daemon=True, name="elf-scalars").start()


def list_serial_ports():
    """[(device, description)]，找不到時回空清單。"""
    try:
        import serial.tools.list_ports
    except ImportError:
        return []
    return [(p.device, p.description) for p in serial.tools.list_ports.comports()]


# ---------------------------------------------------------------------------
# 序列埠自動掃描
# ---------------------------------------------------------------------------
# 藍牙虛擬 COM 埠幾乎不可能是除錯 UART，而**開啟**一個沒有連線的藍牙埠可能阻塞好幾秒
# —— serial.Serial() 的 timeout 參數只管讀寫，管不到 open 本身。所以預設把它們排除。
BLUETOOTH_HINTS = ("bluetooth", "rfcomm", "藍牙")

# 描述字串裡有這些字的優先試。除錯線是 USB 轉序列，這樣通常第一個就中。
PREFERRED_HINTS = ("usb serial", "usb-serial", "cp210", "ch340", "ft232", "ftdi",
                   "silicon labs", "prolific", "usb serial device")


def candidate_ports(include_bluetooth=False):
    """回傳排序過的候選埠 [(device, description)]。

    排序是為了讓掃描早點命中：每個沒有回應的埠要付一次 LNet 握手逾時 (約 1-2 秒)，
    所以順序直接決定使用者要等多久。
    """
    ports = list_serial_ports()
    if not include_bluetooth:
        ports = [(d, s) for d, s in ports
                 if not any(h in s.lower() for h in BLUETOOTH_HINTS)]

    def rank(item):
        _device, desc = item
        return 0 if any(h in desc.lower() for h in PREFERRED_HINTS) else 1

    return sorted(ports, key=rank)


def probe_port(port, baud=V.DEFAULT_BAUD):
    """對單一埠做一次 LNet 握手。成功回裝置資訊字串，失敗回 None。

    刻意在**不帶 ELF** 的 LNet 層做，不建 X2CScope —— X2CScope 的建構子會解析 ELF
    (實測 1.4 秒)，每個候選埠都付一次的話光掃描就要十幾秒。
    握手本身 (get_device_info) 是目標端有沒有跑 X2CScope 的決定性證據。
    """
    from mchplnet.interfaces.factory import InterfaceFactory, InterfaceType
    from mchplnet.lnet import LNet

    interface = None
    try:
        interface = InterfaceFactory.get_interface(
            InterfaceType.SERIAL, port=port, baud_rate=baud)
        lnet = LNet(interface)
        info = lnet.get_device_info()
        return str(info) if info is not None else None
    except Exception:  # noqa: BLE001 - 掃描時每一種失敗都只代表「不是這個埠」
        return None
    finally:
        if interface is not None:
            try:
                interface.stop()
            except Exception:  # noqa: BLE001
                pass


def scan_ports_async(events, baud=V.DEFAULT_BAUD, include_bluetooth=False):
    """在背景執行緒逐一探測序列埠，過程回報給 GUI。

    事件序列：
        ("scan_begin", [(port, desc), ...])
        ("scan_step",  (port, desc, "probing" | "found" | "no-reply"))
        ("scan_done",  [(port, desc, device_info, baud), ...])

    一顆一顆回報而不是掃完才說話，是因為最壞情況要等十幾秒 —— 沒有進度的話
    使用者只會看到畫面卡住，然後以為工具壞了。

    主 baud 全數失敗時會自動用 LEGACY_BAUD (115200) 再掃一輪。韌體在 2026-08-25 從
    115200 改成 230400，但燒舊韌體的板子還在 —— 少了這層退回，使用者只會看到
    「找不到裝置」，完全猜不到是 baud 不符。
    """
    def work():
        ports = candidate_ports(include_bluetooth)
        events.put(("scan_begin", ports))
        rates = [baud] + ([V.LEGACY_BAUD] if baud != V.LEGACY_BAUD else [])
        for rate in rates:
            legacy = rate != baud
            for port, desc in ports:
                label = f"{desc}  @{rate}" + ("  (退回舊 baud)" if legacy else "")
                events.put(("scan_step", (port, label, "probing")))
                info = probe_port(port, rate)
                events.put(("scan_step", (port, label, "found" if info else "no-reply")))
                if info:
                    # 找到就停：接著要用它連線，繼續掃只是讓人多等。
                    events.put(("scan_done", [(port, desc, info, rate)]))
                    return
        events.put(("scan_done", []))

    threading.Thread(target=work, daemon=True, name="port-scan").start()
