"""LWFDSPC 主機端診斷儀表板 (X2CScope / UART2)。

觀測 + 調校參數寫入。**不是**「從 PC 開車」的工具 —— 這顆控制器的命令來源是油門/VR、
煞車開關、排檔開關與 Modbus 儀表，韌體裡沒有給主機下命令的 apiData 介面。

用法:
    python motor_gui.py --demo                # 無硬體，驗版面
    python motor_gui.py --port COM12          # 接上車
    python motor_gui.py --port COM12 --baud 115200
    python motor_gui.py --port AUTO --read-only

baud 由使用者決定：底部有選單，`--baud` 可覆寫，沒指定就用上次成功的值。
不知道板子燒的是哪個就按「掃描」，它會把 KNOWN_BAUDS 都試過。

先跑 check_link.py 再開這支：它把通訊問題與 GUI 問題隔開來，而且它掃到的 baud
會被記下來成為這支的預設值。

Component: HOST TOOLING
"""

import argparse
import queue
import time
import tkinter as tk
import tkinter.font as tkfont
from collections import deque
from tkinter import filedialog, messagebox, ttk

import x2c_vars as V
from x2c_link import (DemoLink, Link, force_utf8_console, list_serial_ports,
                      scan_ports_async)

UI_TICK_MS = 40          # Tk 抽取 event queue 的間隔
TRACE_POINTS = 300       # 捲動圖保留的點數

# 多久沒收到 data 事件就判定「遙測已停止」。取 max(這個值, 4 × 輪詢間隔)。
#
# 為什麼需要這個看門狗：哨兵閘門判定資料不可信時，worker 那一輪**完全不發 data**
# (見 x2c_link 的 _poll_loop)。於是輪詢率、捲動圖與圖例上的即時值全部停在最後一筆
# 好資料上 —— 而畫面看起來跟「一切正常」一模一樣。在診斷工具上這比畫面空白糟得多。
#
# 下限訂 2.5 秒是為了不誤報偶發失步：_recover_sync 走完整條復原階梯 (含重開序列埠
# 與 250ms 穩定等待) 約需 1 秒，期間本來就不會有 data。
STALE_MIN_S = 2.5
STALE_INTERVAL_MULT = 4

# 關閉程式時等 worker 放掉序列埠的上限。涵蓋最壞情況：一次 Scope 區塊讀取的
# SCOPE_READ_TIMEOUT_S(3.0) 加上收尾。超過就照關 —— 讓視窗永遠關不掉更糟。
SHUTDOWN_WAIT_S = 4.0

# 淺色主題。文字與強調色都對白底挑過對比 (前景 ≥4.5:1、圖形 ≥3:1)，
# 所以同一組顏色在 UI 邊框、表格文字與繪圖軌跡上都讀得清楚。
C = {
    "bg": "#eef1f4",        # 視窗底：比卡片略深，卡片才浮得出來
    "panel": "#ffffff",     # 分頁 / 卡片
    "row": "#e3e7ec",       # 輸入框、未選中的分頁標籤
    "edge": "#c3c9d1",
    "text": "#1a1f26",
    "muted": "#5a6672",
    "accent": "#1668b3",
    "ok": "#1a7a3e",
    "warn": "#b45309",
    "danger": "#c0392b",
    "disabled": "#a3abb4",
    "on_danger": "#ffffff",  # 紅色橫幅上的字
}

# 繪圖區專用。與 C[] 分開一組是因為這幾個顏色的取捨不同：格線要淡到不搶軌跡，
# 零線要比格線深但不能被當成資料，暫停標記要一眼看到。
PLOT = {
    "bg": "#ffffff",
    "border": "#c3c9d1",
    "grid": "#dde1e6",
    "zero": "#9aa5b1",      # 零線：比格線深，但不能搶過軌跡
    "title": "#1a1f26",
    "label": "#5a6672",
    "paused": "#c2410c",
}


def pick_font(*candidates):
    """回傳第一個系統裝得到的字型名稱。

    中文字型缺席時 Tk 會用方框代替字，整個介面會變成豆腐。這裡挑得到就挑，
    挑不到退回 Tk 的預設，至少還讀得出英數。
    """
    available = set(tkfont.families())
    for name in candidates:
        if name in available:
            return name
    return "TkDefaultFont"


class ScrollFrame(ttk.Frame):
    """可垂直捲動的容器。調校分頁有 24 列、每列兩行，一定裝不進一個畫面。"""

    def __init__(self, parent, **kw):
        super().__init__(parent, **kw)
        self.canvas = tk.Canvas(self, background=C["panel"], highlightthickness=0)
        bar = ttk.Scrollbar(self, orient="vertical", command=self.canvas.yview)
        self.body = ttk.Frame(self.canvas, style="Panel.TFrame")
        self._window = self.canvas.create_window((0, 0), window=self.body, anchor="nw")
        self.canvas.configure(yscrollcommand=bar.set)
        self.canvas.pack(side="left", fill="both", expand=True)
        bar.pack(side="right", fill="y")
        self.body.bind("<Configure>", self._on_body)
        self.canvas.bind("<Configure>", self._on_canvas)
        # 滑鼠滾輪只在指標進入時綁定 —— 綁在全域會把 Notebook 其他分頁的滾輪也吃掉。
        self.canvas.bind("<Enter>", lambda _e: self.canvas.bind_all("<MouseWheel>", self._wheel))
        self.canvas.bind("<Leave>", lambda _e: self.canvas.unbind_all("<MouseWheel>"))

    def _on_body(self, _event):
        self.canvas.configure(scrollregion=self.canvas.bbox("all"))

    def _on_canvas(self, event):
        self.canvas.itemconfigure(self._window, width=event.width)

    def _wheel(self, event):
        self.canvas.yview_scroll(int(-event.delta / 120), "units")


class Chart(tk.Canvas):
    """捲動式多軌跡圖。自繪而不用 matplotlib —— 這裡要的是高更新率，不是可量測性。

    (需要縮放/游標/匯出的是 Scope 擷取視窗，那邊才用 matplotlib。)
    """

    def __init__(self, parent, title, unit, traces, font, **kw):
        super().__init__(parent, height=190, background=PLOT["bg"],
                         highlightthickness=1, highlightbackground=PLOT["border"], **kw)
        self.title, self.unit, self.traces, self.font = title, unit, traces, font
        self.points = {name: deque(maxlen=TRACE_POINTS)
                       for _l, name, _c, _k in traces}
        self.span_s = 0.0
        self.paused = False
        self.notice = ""      # 非暫停原因的凍結說明 (例如 Scope 擷取中)
        # 真的量字寬，不用「字數 × 常數」估算：中文字元大約是英數的兩倍寬，
        # 用估算值排出來的圖例會直接疊到標題和彼此身上。
        self.legend_font = tkfont.Font(family=font, size=8)
        # 游標讀值。存的是像素 x，而不是取樣索引 —— 圖會因為新資料進來而重畫，
        # 存索引會讓十字線隨著資料捲動而漂移，存像素才是「我指的那個位置」。
        self._hover_px = None
        self.bind("<Configure>", lambda _e: self.draw())
        self.bind("<Motion>", self._on_motion)
        self.bind("<Leave>", self._on_leave)

    def _on_motion(self, event):
        self._hover_px = event.x
        self.draw()

    def _on_leave(self, _event):
        self._hover_px = None
        self.draw()

    def append(self, converted, span_s):
        """converted = {變數名: 已換算成物理量的浮點數或 None}。"""
        self.span_s = span_s
        for _label, name, _colour, _kind in self.traces:
            value = converted.get(name)
            self.points[name].append(float("nan") if value is None else float(value))
        self.draw()

    def latest(self, name):
        seq = self.points[name]
        for value in reversed(seq):
            if value == value:      # 非 NaN
                return value
        return None

    def draw(self):
        self.delete("all")
        width = max(1, self.winfo_width())
        height = max(1, self.winfo_height())
        right = width - 8

        self.create_text(8, 7, text=f"{self.title}   [{self.unit}]", anchor="nw",
                         fill=PLOT["title"], font=(self.font, 10, "bold"))
        # 曲線停住的原因一定要在圖上講明白。凍住的曲線與「鏈路掛了」長得一模一樣，
        # 沒有這個標記的話下一個看畫面的人只能猜。手動暫停優先於其他原因。
        # 純文字不加符號：⏸ 不在 Microsoft JhengHei 裡，會變成一個方框。
        notice = "已暫停" if self.paused else self.notice
        if notice:
            self.create_text(right, 7, text=notice, anchor="ne",
                             fill=PLOT["paused"], font=(self.font, 9, "bold"))

        # 圖例帶目前值 —— 看波形的人同時要知道「現在是多少」，不然還要去翻分頁。
        # 從第二行開始排並自動換行：四條中文標籤在窄視窗下一行絕對排不完。
        x, legend_y = 8, 31
        for label, name, colour, _kind in self.traces:
            value = self.latest(name)
            text = f"{label} {value:+.1f}" if value is not None else f"{label} --"
            item = 17 + self.legend_font.measure(text) + 14
            if x > 8 and x + item > right:
                x, legend_y = 8, legend_y + 14
            self.create_rectangle(x, legend_y - 4, x + 10, legend_y + 5,
                                  fill=colour, outline="")
            self.create_text(x + 15, legend_y, text=text, anchor="w",
                             fill=PLOT["label"], font=self.legend_font)
            x += item
        left, top, bottom = 58, legend_y + 16, height - 20

        samples = [v for seq in self.points.values() for v in seq if v == v]
        if not samples:
            self.create_text(width / 2, height / 2, text="等待取樣 …",
                             fill=PLOT["label"], font=(self.font, 9))
            return

        lo, hi = min(samples), max(samples)
        if hi - lo < 1e-6:
            lo, hi = lo - 1.0, hi + 1.0
        pad = (hi - lo) * 0.12
        lo, hi = lo - pad, hi + pad

        for i in range(5):
            y = top + (bottom - top) * i / 4
            value = hi - (hi - lo) * i / 4
            self.create_line(left, y, right, y, fill=PLOT["grid"])
            self.create_text(left - 6, y, text=f"{value:.3g}", anchor="e",
                             fill=PLOT["label"], font=(self.font, 8))
        # 零線要看得出來：電流回充與速度反轉都是靠「跨過零」判斷的。
        if lo < 0 < hi:
            y0 = bottom - (0 - lo) / (hi - lo) * (bottom - top)
            self.create_line(left, y0, right, y0, fill=PLOT["zero"], dash=(3, 3))
        if self.span_s:
            self.create_text(right, bottom + 10, text=f"← {self.span_s:.1f} s",
                             anchor="e", fill=PLOT["label"], font=(self.font, 8))

        for _label, name, colour, _kind in self.traces:
            seq = list(self.points[name])
            if len(seq) < 2:
                continue
            # NaN 要把線斷開，不能內插 —— 讀取失敗的那一段本來就沒有資料，
            # 用直線連過去會憑空生出一段看起來很正常的波形。
            run, runs = [], []
            for i, value in enumerate(seq):
                if value != value:
                    if len(run) >= 4:
                        runs.append(run)
                    run = []
                    continue
                x = left + (right - left) * i / max(1, TRACE_POINTS - 1)
                y = bottom - (value - lo) / (hi - lo) * (bottom - top)
                run.extend((x, y))
            if len(run) >= 4:
                runs.append(run)
            for coords in runs:
                self.create_line(*coords, fill=colour, width=2)

        self._draw_hover(left, right, top, bottom, lo, hi)

    def _draw_hover(self, left, right, top, bottom, lo, hi):
        """游標所在位置的十字線與各軌跡讀值。

        畫在 draw() 最後，所以新資料進來重畫時讀值會跟著更新 —— 指著同一個位置看
        數字變化，比追著曲線用眼睛估好用得多。
        """
        px = self._hover_px
        if px is None or not (left <= px <= right):
            return
        length = max(len(seq) for seq in self.points.values())
        if length < 2:
            return

        # 像素 -> 取樣索引。用 TRACE_POINTS-1 當分母，與畫線時同一套映射。
        span_px = max(1, right - left)
        index = int(round((px - left) * (TRACE_POINTS - 1) / span_px))
        index = max(0, min(length - 1, index))
        snap_x = left + span_px * index / max(1, TRACE_POINTS - 1)

        self.create_line(snap_x, top, snap_x, bottom, fill=PLOT["zero"])

        # 時間標籤：最右邊那點是「現在」，往左是多久以前。
        per_point = self.span_s / max(1, min(length, TRACE_POINTS) - 1) if self.span_s else 0.0
        ago = (length - 1 - index) * per_point
        lines = [(f"-{ago:.1f} s" if ago else "現在", PLOT["title"])]
        for label, name, colour, _kind in self.traces:
            seq = self.points[name]
            value = seq[index] if index < len(seq) else float("nan")
            shown = "--" if value != value else f"{value:+.3g}"
            lines.append((f"{label} {shown}", colour))

        pad, line_h = 5, 13
        width_px = max(self.legend_font.measure(t) for t, _c in lines) + pad * 2 + 10
        height_px = line_h * len(lines) + pad * 2
        # 靠右邊時把方塊翻到游標左側，否則會被畫布邊緣裁掉。
        box_x = snap_x + 10 if snap_x + 10 + width_px <= right else snap_x - 10 - width_px
        box_y = top
        self.create_rectangle(box_x, box_y, box_x + width_px, box_y + height_px,
                              fill=PLOT["bg"], outline=PLOT["border"])
        for row, (text, colour) in enumerate(lines):
            y = box_y + pad + line_h * row + line_h / 2
            if row:
                self.create_rectangle(box_x + pad, y - 4, box_x + pad + 7, y + 3,
                                      fill=colour, outline="")
            self.create_text(box_x + pad + (10 if row else 0), y, text=text,
                             anchor="w", fill=colour if row else PLOT["title"],
                             font=self.legend_font)


def write_table_header(parent, font):
    """調校/危險分頁的表頭。少了它，輸入框左邊那個數字是「目前值」並不明顯。"""
    for column, text, anchor in ((0, "參數", "w"), (1, "目前值", "e"),
                                 (2, "寫入新值", "e"), (4, "寫入後是否留住", "w")):
        ttk.Label(parent, text=text, style="Doc.TLabel", font=(font, 8)).grid(
            row=0, column=column, sticky=anchor, padx=(8, 6), pady=(0, 2))


class WriteRow:
    """調校分頁的一列：目前值 + 輸入框 + 寫入鈕 + persist 徽章 + 說明。"""

    def __init__(self, parent, spec, row, app, font):
        self.name, self.label, self.kind, self.persist, self.doc = spec
        self.app = app
        self.value = tk.StringVar(value="--")
        self.entry_var = tk.StringVar()

        # 每一列佔兩個 grid row (主列 + 說明)，並整體下移一列讓 grid row 0 留給表頭。
        main_row = row * 2 + 1
        doc_row = main_row + 1

        ttk.Label(parent, text=self.label, style="Row.TLabel",
                  font=(font, 9, "bold")).grid(row=main_row, column=0, sticky="w",
                                               padx=(8, 10), pady=(6, 0))
        ttk.Label(parent, textvariable=self.value, style="Value.TLabel",
                  font=("Consolas", 9)).grid(row=main_row, column=1, sticky="e",
                                             padx=6, pady=(6, 0))
        self.entry = ttk.Entry(parent, textvariable=self.entry_var, width=11,
                               font=("Consolas", 9), justify="right")
        self.entry.grid(row=main_row, column=2, padx=4, pady=(6, 0))
        self.entry.bind("<Return>", lambda _e: self.write())
        self.button = ttk.Button(parent, text="寫入", width=6, command=self.write)
        self.button.grid(row=main_row, column=3, padx=(2, 8), pady=(6, 0))

        # persist == "loop" 代表主迴圈每圈都重寫這一格，從主機寫進去只會存活幾毫秒。
        # 這件事必須在按下按鈕**之前**就看得到，否則使用者會以為 GUI 壞了。
        badge = "主迴圈會覆寫" if self.persist == "loop" else "開機值"
        style = "Loop.TLabel" if self.persist == "loop" else "Boot.TLabel"
        ttk.Label(parent, text=badge, style=style,
                  font=(font, 8)).grid(row=main_row, column=4, sticky="w",
                                       padx=(0, 8), pady=(6, 0))

        if self.doc:
            ttk.Label(parent, text=self.doc, style="Doc.TLabel", font=(font, 8),
                      wraplength=560, justify="left").grid(
                row=doc_row, column=0, columnspan=5, sticky="w",
                padx=(8, 8), pady=(0, 4))

    def refresh(self, raw, scale):
        """更新「目前值」。q15 型別額外顯示物理量，免得要手算 Q15。"""
        if raw is None:
            self.value.set("--")
            return
        text = f"{int(raw):,}"
        if self.kind == "q15":
            # 這一組 q15 變數有的是速度域有的是電流域，兩個都印出來讓人自己取用 ——
            # 猜錯域比多印一個數字糟得多。
            kmh = scale.q15_to_kmh(raw)
            amp = V.q15_to_amp(raw)
            text += f"   ({kmh:+.2f} km/h  |  {amp:+.2f} A)"
        self.value.set(text)

    def write(self):
        text = self.entry_var.get().strip()
        if not text:
            return
        try:
            value = int(float(text))
        except ValueError:
            messagebox.showerror("輸入無效", f"{text!r} 不是整數")
            return
        self.app.request_write(self.name, value)

    def set_enabled(self, enabled):
        state = "normal" if enabled else "disabled"
        self.entry.configure(state=state)
        self.button.configure(state=state)


class App(tk.Tk):
    def __init__(self, args):
        super().__init__()
        self.args = args
        self.title(f"LWFDSPC 主機端診斷 {V.GUI_VERSION} — X2CScope")
        self.geometry("1500x940")
        self.minsize(1180, 720)
        self.configure(background=C["bg"])

        self.font = pick_font("Microsoft JhengHei UI", "Microsoft JhengHei",
                              "Noto Sans TC", "Segoe UI")
        self.events = queue.Queue()
        self.link = None
        self.values = {}
        self.scale = V.Scale()
        self.scope_win = None
        self.connected = False
        self.writes_allowed = False
        self.elf = str(args.elf)
        self._last_data = 0.0
        self._first_data = 0.0
        self._data_count = 0
        self._tick = None
        self.scanning = False
        self.paused = False
        self.scope_capturing = False
        # 遙測看門狗的狀態，見 _check_stale()。
        self._data_stale = False
        # 哨兵三態 "ok"/"desync"/"noreply"，決定「寫入被封鎖」時該講哪個原因。
        self._signature_state = "noreply"
        # 已呼叫 stop() 但執行緒還沒結束的舊 link，見 _watch_closing()。
        self._closing_link = None
        # 最後一次鏈路錯誤，跨連線保留 —— 掉線原因不能被下一張橫幅蓋掉。
        self._last_link_error = ""
        # 連線時發現的問題（ELF 不對版、build 過期、缺變數）。原本只在 on_connected
        # 裡才建立，於是任何在首次連線**之前**想還原這張橫幅的路徑都會 AttributeError。
        self._connect_banner = ""
        self._writes_permitted = False
        self._counter_base = {}       # 手動基準 (「計數器歸零」按鈕)
        self._connect_base = {}       # 自動基準，連線後第一輪掃描完成時記下
        self._await_connect_base = False
        self._connect_after_scan = False
        self._scan_candidates = []

        self.status = tk.StringVar(value="未連線")
        self.rate = tk.StringVar(value="輪詢 -- Hz")
        self.sentinel = tk.StringVar(value="哨兵 --")
        self.link_info = tk.StringVar(value="")
        self.banner = tk.StringVar(value="")

        self._init_style()
        self._build()
        self.protocol("WM_DELETE_WINDOW", self.on_close)
        self._tick = self.after(UI_TICK_MS, self.drain)
        if args.demo or args.port:
            self.connect()

    # -- 樣式 --------------------------------------------------------------
    def _init_style(self):
        style = ttk.Style(self)
        # clam 會吃自訂顏色，Windows 的 vista 主題不會 —— 深色配色必須用 clam。
        style.theme_use("clam")
        style.configure(".", background=C["bg"], foreground=C["text"],
                        fieldbackground=C["row"], font=(self.font, 9))
        style.configure("TFrame", background=C["bg"])
        style.configure("Panel.TFrame", background=C["panel"])
        style.configure("TLabel", background=C["bg"], foreground=C["text"])
        style.configure("Row.TLabel", background=C["panel"], foreground=C["text"])
        style.configure("Value.TLabel", background=C["panel"], foreground=C["accent"])
        style.configure("Doc.TLabel", background=C["panel"], foreground=C["muted"])
        style.configure("Loop.TLabel", background=C["panel"], foreground=C["warn"])
        style.configure("Boot.TLabel", background=C["panel"], foreground=C["muted"])
        style.configure("Muted.TLabel", background=C["bg"], foreground=C["muted"])
        style.configure("Title.TLabel", background=C["bg"], foreground=C["text"],
                        font=(self.font, 15, "bold"))
        style.configure("TNotebook", background=C["bg"], borderwidth=0)
        # 十三個分頁要塞進右半邊，padding 給多了標籤文字就會被截掉。
        style.configure("TNotebook.Tab", background=C["row"], foreground=C["muted"],
                        padding=(7, 5))
        style.map("TNotebook.Tab",
                  background=[("selected", C["panel"])],
                  foreground=[("selected", C["text"])])
        style.configure("Treeview", background=C["panel"], fieldbackground=C["panel"],
                        foreground=C["text"], borderwidth=0, rowheight=21)
        style.configure("Treeview.Heading", background=C["row"], foreground=C["muted"],
                        relief="flat")
        style.map("Treeview", background=[("selected", C["edge"])])
        style.configure("TButton", background=C["row"], foreground=C["text"],
                        borderwidth=1, focusthickness=0, padding=(8, 3))
        style.map("TButton", background=[("active", C["edge"]),
                                         ("disabled", C["bg"])],
                  foreground=[("disabled", C["disabled"])])
        style.configure("TEntry", fieldbackground=C["row"], foreground=C["text"],
                        insertcolor=C["text"], borderwidth=1)
        style.configure("TCombobox", fieldbackground=C["row"], background=C["row"],
                        foreground=C["text"], arrowcolor=C["muted"])
        # state="readonly" 的 Combobox 把自己的文字當成「已選取」來畫，所以只設
        # foreground 不夠 —— 不覆寫 select* 的話文字會被畫成選取色，整格看不見。
        style.map("TCombobox",
                  fieldbackground=[("readonly", C["row"]), ("disabled", C["bg"])],
                  foreground=[("readonly", C["text"]), ("disabled", C["disabled"])],
                  selectbackground=[("readonly", C["row"])],
                  selectforeground=[("readonly", C["text"])],
                  arrowcolor=[("disabled", C["disabled"])])
        # 下拉開來的清單是 Tk 的 Listbox，不吃 ttk 樣式，只能走 option database。
        self.option_add("*TCombobox*Listbox.background", C["panel"])
        self.option_add("*TCombobox*Listbox.foreground", C["text"])
        self.option_add("*TCombobox*Listbox.selectBackground", C["accent"])
        self.option_add("*TCombobox*Listbox.selectForeground", "#ffffff")
        style.configure("TCheckbutton", background=C["bg"], foreground=C["text"])
        style.configure("Danger.TCheckbutton", background=C["panel"],
                        foreground=C["danger"])
        # Scope 視窗的通道勾選也用這套樣式 —— Toplevel 共用同一個 ttk.Style。
        style.configure("Chan.TCheckbutton", background=C["panel"],
                        foreground=C["text"])
        style.map("Chan.TCheckbutton", background=[("active", C["row"])])
        style.configure("Banner.TLabel", foreground=C["on_danger"],
                        font=(self.font, 9, "bold"))
        style.configure("TPanedwindow", background=C["bg"])

    # -- 版面 --------------------------------------------------------------
    def _build(self):
        header = ttk.Frame(self, padding=(14, 10, 14, 6))
        header.pack(fill="x")
        ttk.Label(header, text="LWFDSPC 主機端診斷", style="Title.TLabel").pack(side="left")
        # 版本要在**畫面內**，不能只放視窗標題列：回報問題附的截圖常常把標題列裁掉，
        # 而「你跑的是哪一版」是支援時第一件要問的事。
        ttk.Label(header, text=V.GUI_VERSION, style="Muted.TLabel",
                  font=("Consolas", 9)).pack(side="left", padx=(6, 0))
        ttk.Label(header, textvariable=self.status, style="Muted.TLabel").pack(side="left", padx=16)
        self.rate_label = ttk.Label(header, textvariable=self.rate,
                                    style="Muted.TLabel", font=("Consolas", 9))
        self.rate_label.pack(side="right")
        self.sentinel_label = ttk.Label(header, textvariable=self.sentinel,
                                        style="Muted.TLabel", font=("Consolas", 9))
        self.sentinel_label.pack(side="right", padx=16)
        # 生效的埠與 baud。掃描可能改用另一個 baud 才連上，不顯示就完全看不出連的是哪個
        # —— 而 baud 決定了頻寬上限，是判斷 FIFO 節流的前提。
        self.link_info_label = ttk.Label(header, textvariable=self.link_info,
                                         style="Muted.TLabel", font=("Consolas", 9))
        self.link_info_label.pack(side="right", padx=16)

        self.banner_label = tk.Label(self, textvariable=self.banner, anchor="w",
                                     justify="left", background=C["danger"],
                                     foreground=C["on_danger"], padx=12, pady=6,
                                     font=(self.font, 9, "bold"), wraplength=1400)
        # wraplength 必須跟著視窗寬度走。寫死 1400 而 minsize 只有 1180 時，縮小視窗
        # (或在 1280 寬的筆電上) 這幾行紅字右邊會被裁掉約 200px —— 而橫幅往往是
        # 唯一的線索來源，被裁掉的通常正是「該怎麼做」那一段。
        self.bind("<Configure>", self._on_resize)

        body = ttk.PanedWindow(self, orient="horizontal")
        body.pack(fill="both", expand=True, padx=12, pady=(4, 6))
        left = ttk.Frame(body)
        right = ttk.Frame(body)
        body.add(left, weight=5)
        body.add(right, weight=6)
        # weight 只影響「之後」的縮放，不決定初始位置。這個比例是兩邊的下限決定的：
        # 左邊要放得下四條中文圖例 (否則換行吃掉一整列繪圖區)，右邊要放得下
        # 十三個分頁標籤 (否則文字被截掉)。
        self.after(80, lambda: self._place_sash(body))

        # 捲動圖上方的控制列。放在圖旁邊而不是底部工具列，是為了讓「暫停」與它
        # 影響的東西在視覺上連在一起 —— 暫停只凍結繪圖，遙測與分頁數值照常更新。
        chart_bar = ttk.Frame(left)
        chart_bar.pack(fill="x", pady=(0, 4))
        self.pause_btn = ttk.Button(chart_bar, text="暫停繪圖", width=12,
                                    command=self.toggle_pause)
        self.pause_btn.pack(side="left")
        ttk.Button(chart_bar, text="清除", width=6,
                   command=self.clear_charts).pack(side="left", padx=4)
        # 單調計數器的基準線。做「改一個設定，看它有沒有停止增加」這種實驗時，
        # 累計值本身沒有用 —— 分不出「早就發生完了」與「正在持續發生」。
        self.baseline_btn = ttk.Button(chart_bar, text="計數器歸零", width=11,
                                       command=self.zero_counters)
        self.baseline_btn.pack(side="left", padx=(4, 0))
        self.pause_note = tk.StringVar(value="")
        ttk.Label(chart_bar, textvariable=self.pause_note,
                  style="Muted.TLabel").pack(side="left", padx=8)

        self.charts = []
        for title, unit, traces in V.CHARTS:
            chart = Chart(left, title, unit, traces, self.font)
            chart.pack(fill="both", expand=True, pady=(0, 6))
            self.charts.append((chart, traces))

        self.notebook = ttk.Notebook(right)
        self.notebook.pack(fill="both", expand=True)
        self.tables = {}
        # 分頁 widget 名稱 -> PANELS 的 key。刻意不靠 notebook.tab(..., "text") 反查：
        # 十三個分頁的標籤本來就快擠不下，一旦為了排版縮短顯示文字，用文字反查
        # 就會查不到對應的資料 —— 而那種壞法是靜默的 (分頁變成永遠不更新)。
        self._tab_pages = {}
        for page, rows in V.PANELS.items():
            frame = ttk.Frame(self.notebook, style="Panel.TFrame", padding=6)
            self.notebook.add(frame, text=page)
            self._tab_pages[str(frame)] = page
            table = ttk.Treeview(frame, columns=("value", "delta"),
                                 show="tree headings", selectmode="none")
            table.heading("#0", text="欄位", anchor="w")
            table.heading("value", text="數值", anchor="w")
            table.heading("delta", text="增量", anchor="w")
            # 標籤欄固定寬、數值欄靠左：數值靠右對齊時會被推到視窗最邊緣，
            # 跟它自己的標籤隔著一大片空白，眼睛得橫掃整列才對得起來。
            # 固定起點靠左排一樣是對齊的，而且緊跟在標籤後面。
            table.column("#0", width=250, minwidth=180, stretch=False)
            table.column("value", width=300, anchor="w", stretch=True)
            # 增量給獨立欄位，不塞在數值字串尾巴 —— 塞進去時長的說明文字會把它
            # 擠出可見範圍 (實測 [自連線 +22,5… 被截斷)，而增量正是最該看到的東西。
            table.column("delta", width=120, minwidth=90, anchor="w", stretch=False)
            for index, (label, _name, _kind) in enumerate(rows):
                table.insert("", "end", iid=str(index), text=label, values=("--", ""))
            table.pack(fill="both", expand=True)
            self.tables[page] = table
        self.notebook.bind("<<NotebookTabChanged>>", lambda _e: self.refresh_table())

        self._build_tuning()
        self._build_danger()
        self._build_scale()
        self._build_log()
        self._build_footer()

    def _place_sash(self, body, tries=15):
        # 視窗還沒 map 時 winfo_width() 回 1，直接照它算會把分隔線推到最左邊。
        width = body.winfo_width()
        if width <= 1 and tries > 0:
            self.after(80, lambda: self._place_sash(body, tries - 1))
            return
        try:
            body.sashpos(0, int(width * 0.44))
        except tk.TclError:
            pass

    def _build_tuning(self):
        frame = ScrollFrame(self.notebook)
        self.notebook.add(frame, text="調校")
        ttk.Label(frame.body, style="Doc.TLabel", font=(self.font, 8), wraplength=620,
                  justify="left",
                  text="按「讀取目前值」把韌體現在的值抓回來。這一組不做週期輪詢 —— "
                       "它們絕大多數只在開機初始化時被寫，一直重讀只是白花頻寬。\n"
                       "寫入後會自動回讀。回讀值與寫入值不同不是錯誤 —— "
                       "標著「主迴圈會覆寫」的變數本來就會被韌體在幾毫秒內蓋回去，"
                       "回讀值就是唯一能證明這件事的證據。"
                  ).grid(row=0, column=0, columnspan=5, sticky="w", padx=8, pady=(6, 6))
        self._read_btn_tuning = ttk.Button(frame.body, text="讀取目前值",
                                          command=self.read_write_values)
        self._read_btn_tuning.grid(row=1, column=0, sticky="w", padx=8, pady=(0, 8))
        holder = ttk.Frame(frame.body, style="Panel.TFrame")
        holder.grid(row=2, column=0, columnspan=5, sticky="nsew")
        holder.columnconfigure(0, weight=1)
        write_table_header(holder, self.font)
        self.tuning_rows = [WriteRow(holder, spec, i, self, self.font)
                            for i, spec in enumerate(V.WRITE_TUNING)]

    def _build_danger(self):
        frame = ScrollFrame(self.notebook)
        self.notebook.add(frame, text="⚠ 危險")
        self.danger_unlocked = tk.BooleanVar(value=False)
        ttk.Label(frame.body, style="Doc.TLabel", font=(self.font, 8), wraplength=620,
                  justify="left",
                  text="這一組會直接動到馬達輸出或安全鏈。多數會被主迴圈立刻覆寫，"
                       "但那不代表安全 —— 只代表效果是一次性的脈衝，"
                       "在實車上仍可能造成一次抽動。"
                  ).grid(row=0, column=0, columnspan=5, sticky="w", padx=8, pady=(6, 4))
        ttk.Checkbutton(frame.body, style="Danger.TCheckbutton",
                        variable=self.danger_unlocked,
                        command=self._sync_write_state,
                        text="我知道這會動到馬達輸出與安全鏈，解除鎖定"
                        ).grid(row=1, column=0, columnspan=5, sticky="w", padx=8, pady=(0, 4))
        # 讀取不需要解鎖 —— 看現在的值本身沒有風險，而看不到現值就動手才有風險。
        self._read_btn_danger = ttk.Button(frame.body, text="讀取目前值",
                                          command=self.read_write_values)
        self._read_btn_danger.grid(row=2, column=0, sticky="w", padx=8, pady=(0, 8))
        holder = ttk.Frame(frame.body, style="Panel.TFrame")
        holder.grid(row=3, column=0, columnspan=5, sticky="nsew")
        holder.columnconfigure(0, weight=1)
        write_table_header(holder, self.font)
        self.danger_rows = [WriteRow(holder, spec, i, self, self.font)
                            for i, spec in enumerate(V.WRITE_DANGER)]

    def _build_scale(self):
        frame = ttk.Frame(self.notebook, style="Panel.TFrame", padding=10)
        self.notebook.add(frame, text="刻度")
        ttk.Label(frame, style="Doc.TLabel", font=(self.font, 8), wraplength=620,
                  justify="left",
                  text="這四個實體參數決定畫面上每一個速度數字的意義。改這裡只影響"
                       "主機端換算，**不會**寫進韌體 —— 用途是驗證「顯示 7.2 km/h 到底對不對」："
                       "把參數改成你量到的實際值，看數字有沒有對上。"
                  ).grid(row=0, column=0, columnspan=3, sticky="w", pady=(0, 10))

        self.scale_vars = {}
        fields = (("極對數", "pole_pairs"), ("齒比 ×100", "gear_ratio_x100"),
                  ("輪徑 吋×10", "wheel_inch_x10"), ("Q15 滿刻度 RPM", "speed_fs_rpm"))
        for i, (label, attr) in enumerate(fields):
            ttk.Label(frame, text=label, style="Row.TLabel").grid(
                row=i + 1, column=0, sticky="w", pady=3)
            var = tk.StringVar(value=str(getattr(self.scale, attr)))
            self.scale_vars[attr] = var
            entry = ttk.Entry(frame, textvariable=var, width=10, justify="right",
                              font=("Consolas", 9))
            entry.grid(row=i + 1, column=1, sticky="w", padx=8, pady=3)
            entry.bind("<Return>", lambda _e: self.apply_scale())
        ttk.Button(frame, text="套用", command=self.apply_scale).grid(
            row=len(fields) + 1, column=1, sticky="w", padx=8, pady=(8, 4))
        ttk.Button(frame, text="回復預設", command=self.reset_scale).grid(
            row=len(fields) + 1, column=2, sticky="w", pady=(8, 4))

        self.scale_desc = tk.StringVar(value=self.scale.describe())
        ttk.Label(frame, textvariable=self.scale_desc, style="Value.TLabel",
                  font=("Consolas", 8), wraplength=620, justify="left").grid(
            row=len(fields) + 2, column=0, columnspan=3, sticky="w", pady=(10, 0))

        # 主機假設 vs 韌體實際設定的對照。兩邊不一致就是速度顯示對不上的直接原因。
        ttk.Label(frame, text="韌體回報的組態 (執行期可被 LCD 改)", style="Row.TLabel",
                  font=(self.font, 9, "bold")).grid(
            row=len(fields) + 3, column=0, columnspan=3, sticky="w", pady=(16, 4))
        self.fw_config = tk.StringVar(value="--")
        ttk.Label(frame, textvariable=self.fw_config, style="Doc.TLabel",
                  font=("Consolas", 8), justify="left").grid(
            row=len(fields) + 4, column=0, columnspan=3, sticky="w")

    def _build_log(self):
        frame = ttk.Frame(self.notebook, style="Panel.TFrame", padding=6)
        self.notebook.add(frame, text="寫入紀錄")
        columns = ("time", "var", "want", "got", "note")
        self.log = ttk.Treeview(frame, columns=columns, show="headings", selectmode="none")
        for column, title, width, anchor in (
                ("time", "時間", 78, "w"), ("var", "變數", 250, "w"),
                ("want", "寫入", 80, "e"), ("got", "回讀", 80, "e"),
                ("note", "結果", 190, "w")):
            self.log.heading(column, text=title, anchor=anchor)
            self.log.column(column, width=width, anchor=anchor)
        self.log.tag_configure("ok", foreground=C["ok"])
        self.log.tag_configure("overwritten", foreground=C["warn"])
        self.log.tag_configure("fail", foreground=C["danger"])
        self.log.pack(fill="both", expand=True)

    def _build_footer(self):
        bar = ttk.Frame(self, padding=(14, 6, 14, 10))
        bar.pack(fill="x")

        ttk.Label(bar, text="埠", style="Muted.TLabel").pack(side="left")
        # 預設 AUTO：連線時會先掃描並回報進度，比要使用者自己從五個埠裡猜哪個是
        # 除錯線可靠得多。
        self.port_var = tk.StringVar(value=self.args.port or "AUTO")
        self.port_box = ttk.Combobox(bar, textvariable=self.port_var, width=9,
                                     state="readonly")
        self.port_box.pack(side="left", padx=(4, 4))
        self.refresh_ports()
        self.scan_btn = ttk.Button(bar, text="掃描", width=5, command=self.scan_ports)
        self.scan_btn.pack(side="left", padx=(0, 12))

        # baud 讓使用者選，不由工具猜。
        #
        # 兩個值都是韌體支援的正常設定 (X2C_BAUD_TARGET)，工具不推薦任何一個 ——
        # 實測 230400 在某些板子上會中途停止回應而 115200 穩定，但那是板子的特性，
        # 不是「舊/新」。刻意不做自動退回：明確選了 115200 卻偷偷連上 230400 只會讓
        # 之後的排查更難。真的不知道燒的是哪個時按「掃描」，它兩個都會試。
        ttk.Label(bar, text="baud", style="Muted.TLabel").pack(side="left")
        self.baud_var = tk.StringVar(value=str(self.args.baud))
        self.baud_box = ttk.Combobox(
            bar, textvariable=self.baud_var, width=8, state="readonly",
            font=("Consolas", 9),
            values=[str(b) for b in V.KNOWN_BAUDS])
        self.baud_box.pack(side="left", padx=(4, 12))
        self.baud_box.bind("<<ComboboxSelected>>", self._on_baud_pick)

        ttk.Label(bar, text="間隔 ms", style="Muted.TLabel").pack(side="left")
        self.interval_var = tk.StringVar(value=str(self.args.interval))
        interval = ttk.Entry(bar, textvariable=self.interval_var, width=6,
                             justify="right", font=("Consolas", 9))
        interval.pack(side="left", padx=(4, 4))
        interval.bind("<Return>", lambda _e: self.apply_interval())
        ttk.Button(bar, text="套用", command=self.apply_interval, width=5).pack(side="left", padx=(0, 12))

        self.connect_btn = ttk.Button(bar, text="連線", command=self.toggle_connection)
        self.connect_btn.pack(side="left", padx=(0, 8))
        self.scope_btn = ttk.Button(bar, text="開啟 Scope 視窗", command=self.open_scope,
                                    state="disabled")
        self.scope_btn.pack(side="left")

        self.elf_var = tk.StringVar(value=self.elf)
        ttk.Button(bar, text="選 ELF …", command=self.choose_elf).pack(side="right")
        ttk.Label(bar, textvariable=self.elf_var, style="Muted.TLabel",
                  font=("Consolas", 8)).pack(side="right", padx=8)

    # -- 連線 --------------------------------------------------------------
    def toggle_connection(self):
        if self.link is not None:
            self.disconnect()
        else:
            self.connect()

    def _selected_baud(self):
        """下拉選單裡的 baud。壞值退回目前生效的值，不讓一個爛字串弄掉連線。"""
        try:
            return int(self.baud_var.get())
        except (TypeError, ValueError):
            self.baud_var.set(str(self.args.baud))
            return self.args.baud

    def _on_baud_pick(self, _event=None):
        baud = self._selected_baud()
        if baud == self.args.baud:
            return
        self.args.baud = baud
        # 記下來當下次開程式的預設。選了就算採用 —— 不等連線成功才記，因為連不上
        # 的時候使用者最需要的就是「我上次選的還在」而不是被打回預設值。
        V.save_last_baud(baud)
        # 選了非預設值就當場警告，不要等它掉線才說。實機驗證 230400 掉線後自動與
        # 手動重連都救不回來（只能重置控制器電源），所以這不是「可能比較慢」等級的
        # 取捨 —— 使用者有權在按下連線之前就知道。
        if baud != V.DEFAULT_BAUD:
            self.show_banner(
                f"已選 {baud:,} baud。**實機驗證這個 baud 會在連線幾十秒後完全停止"
                f"回應，而且自動與手動重連都救不回來，只能重置控制器電源。**\n"
                f"穩定的是 {V.DEFAULT_BAUD:,}（本工具預設）。"
                f"{baud:,} 的好處只有頻寬加倍（Scope 擷取較快）——"
                "讀不到資料時那沒有意義。\n"
                "板子燒的 X2C_BAUD_TARGET 必須與這裡一致，否則連不上。")
        else:
            self.show_banner(self._connect_banner)
        if self.link is not None:
            self.status.set(f"baud 改為 {baud:,} —— 要斷線再連線才生效")
        else:
            self.status.set(f"baud 設為 {baud:,}"
                            f"（韌體端是 diagnostics_x2cscope.c 的 X2C_BAUD_TARGET）")

    def _sync_baud_state(self):
        """連線中不給改 baud —— 改了也要重連才生效，開著只會讓人以為能即時切換。"""
        self.baud_box.configure(
            state="disabled" if (self.link is not None or self.scanning)
            else "readonly")

    def refresh_ports(self):
        """重新列舉序列埠。拔插 USB 線之後不用重開程式。"""
        ports = ["AUTO"] + [device for device, _desc in list_serial_ports()]
        self.port_box.configure(values=ports)
        if self.port_var.get() not in ports:
            self.port_var.set("AUTO")

    def scan_ports(self):
        """逐一探測序列埠找出跑著 X2CScope 的那一個。"""
        if self.scanning:
            return
        self.refresh_ports()
        self.scanning = True
        self.scan_btn.configure(state="disabled")
        # 掃描從選單裡選的那個 baud 開始，失敗才試另一個 —— 所以選對的話完全不會
        # 發生「用錯的 baud 探測」，而那正是把目標端解析器弄卡的主因。
        self.args.baud = self._selected_baud()
        self._sync_baud_state()
        scan_ports_async(self.events, self.args.baud,
                         include_bluetooth=self.args.include_bluetooth)

    def connect(self):
        if self.link is not None or self.scanning:
            return
        # 上一條 link 還在收尾就不能開新的 —— 埠還在它手上。按鈕平常是鎖住的，
        # 這裡再擋一次是因為 Enter 鍵或指令列都繞得過按鈕狀態。
        if self._closing_link is not None and self._closing_link.is_alive():
            self.status.set("上一次連線還在關閉序列埠，請稍候 …")
            return
        self._closing_link = None
        # AUTO 走自己的掃描而不是交給 pyx2cscope 的 port="AUTO"：兩者都做真正的
        # LNet 握手，但 pyx2cscope 那條路徑沒有任何進度回報，最壞情況會讓畫面
        # 靜靜卡十幾秒；而且它不會跳過藍牙埠，開一個沒連線的藍牙埠可能卡更久。
        if self.port_var.get() == "AUTO" and not self.args.demo:
            self._connect_after_scan = True
            self.scan_ports()
            return
        self._start_link()

    def _start_link(self):
        interval = self._interval_ms()
        factory = DemoLink if self.args.demo else Link
        kwargs = {}
        if self.args.demo:
            kwargs["bad_signature"] = self.args.bad_signature
        # 一律用選單裡的值。原本直接吃 self.args.baud，而那只有掃描那條路徑會被
        # 更新 —— 於是「從下拉選單挑 COM15 再按連線」永遠用指令列的預設 baud，
        # 韌體換成另一個值之後那條路徑就直接連不上，而且畫面上完全看不出原因。
        self.args.baud = self._selected_baud()
        self.link = factory(self.port_var.get() if not self.args.demo else "DEMO",
                            self.args.baud, self.elf, interval, self.events,
                            rare_slice=self.args.rare_slice,
                            allow_writes=not self.args.read_only, **kwargs)
        self.status.set(f"連線中 … @ {self.args.baud:,}")
        self.connect_btn.configure(text="斷線")
        self._first_data = self._data_count = 0
        self.link.start()
        self._sync_baud_state()

    def disconnect(self):
        if self.link is None:
            return
        self.link.stop()
        # stop() 只設旗標。worker 要跑到 finally 才會 X2CScope.disconnect() 關掉序列埠，
        # 而它可能正卡在一次讀取裡。埠沒關之前重連一定失敗 (Windows: Access is denied)，
        # 所以要等執行緒真的結束 —— 而不是讓使用者按下「連線」再吃一個看不懂的錯誤。
        self._closing_link = self.link
        self.link = None
        self.connected = False
        self.writes_allowed = self._writes_permitted = False
        # 基準跟著這一次連線作廢 —— 下次連線 (或期間電源重置過) 計數器可能已歸零，
        # 拿舊基準去減會算出負的增量。
        self._connect_base = {}
        self._counter_base = {}
        self._await_connect_base = False
        self.baseline_btn.configure(text="計數器歸零")
        self._sync_write_state()
        self.connect_btn.configure(text="連線")
        self.scope_btn.configure(state="disabled")
        self._clear_live_indicators()
        self.link_info.set("")
        # 擷取狀態跟著這次連線作廢，否則斷線時若正在擷取，捲動圖會永遠凍住。
        self.on_scope_active(False)
        self._watch_closing()

    def _clear_live_indicators(self):
        """把「還活著」的指示燈全部收回中性狀態。斷線的兩條路徑都要走這裡。

        少了這個，斷線後標頭會留著最後一筆輪詢率與哨兵狀態，看起來像還連著；
        而看門狗的「遙測停止 N s」更糟 —— 已經按了斷線，那個秒數只是雜訊。
        """
        self._data_stale = False
        self._last_data = 0.0
        self.rate_label.configure(foreground=C["muted"])
        self.rate.set("輪詢 -- Hz")
        self._signature_state = "noreply"
        self.sentinel.set("哨兵 --")
        self.sentinel_label.configure(foreground=C["muted"])
        self._sync_baud_state()     # 斷線了就可以改 baud 了
        self._update_chart_notice()

    def _watch_closing(self):
        """等 worker 結束、序列埠真的釋放，期間把「連線」鈕鎖住並說明在等什麼。"""
        link = self._closing_link
        if link is None:
            return
        if link.is_alive():
            self.connect_btn.configure(state="disabled")
            self.status.set("關閉序列埠中 …")
            self.after(60, self._watch_closing)
            return
        self._closing_link = None
        self.connect_btn.configure(state="normal")
        self.status.set("已斷線")

    def _interval_ms(self):
        try:
            return max(50, int(float(self.interval_var.get())))
        except ValueError:
            return self.args.interval

    def apply_interval(self):
        ms = self._interval_ms()
        self.interval_var.set(str(ms))
        if self.link is not None:
            self.link.submit("interval", ms)

    def choose_elf(self):
        path = filedialog.askopenfilename(
            title="選擇燒進板子的 ELF", initialdir=str(V.FIRMWARE_DIR),
            filetypes=[("ELF", "*.elf"), ("所有檔案", "*.*")])
        if not path:
            return
        self.elf = path
        self.elf_var.set(path)
        if self.link is not None:
            messagebox.showinfo("需要重新連線", "ELF 已更換，請斷線再連線以重新解析。")

    # -- 事件抽取 ----------------------------------------------------------
    def drain(self):
        try:
            while True:
                kind, payload, token = self.events.get_nowait()
                # 丟掉已被取代的 link 發出的事件。斷線後 worker 可能還卡在一次序列埠
                # 讀取裡 (pyserial 逐位元組 timeout=1s)，之後才發出 "disconnected"
                # ——那時使用者可能已經重新連線了，照收會把新連線的狀態清掉。
                # token 為 None 的是埠掃描與 ELF 解析，不屬於任何 link，一律接受。
                if token is not None and token is not getattr(self.link, "token", None):
                    continue
                self._dispatch(kind, payload)
        except queue.Empty:
            pass
        # 看門狗掛在這裡而不是自己一條 after 迴圈：它要在剛抽完事件之後判斷，
        # 而且兩者的節奏本來就一樣。多一條 after 只是多一個會忘記取消的計時器。
        self._check_stale()
        self._tick = self.after(UI_TICK_MS, self.drain)

    def _dispatch(self, kind, payload):
        if kind == "status":
            self.status.set(payload)
        elif kind == "error":
            self._last_link_error = str(payload).splitlines()[0]
            self.status.set(self._last_link_error)
            self.show_banner(payload)
        elif kind == "connected":
            self.on_connected(payload)
        elif kind == "disconnected":
            self.connected = False
            # 斷線時 worker 直接結束，scope_active=False 永遠不會發 —— 不強制解除的話
            # 捲動圖會永久凍在一張舊快照上，圖上還寫著「擷取中」，兩個都是假的。
            self.on_scope_active(False)
            self.writes_allowed = self._writes_permitted = False
            self._sync_write_state()
            self.connect_btn.configure(text="連線")
            self.scope_btn.configure(state="disabled")
            self.link = None
            self._clear_live_indicators()
        elif kind == "data":
            self.on_data(payload)
        elif kind == "signature":
            self.on_signature(payload)
        elif kind == "scope_active":
            self.on_scope_active(payload)
        elif kind == "write_done":
            self.on_write_done(payload)
        elif kind == "write_values":
            self.on_write_values(payload)
        elif kind == "scan_begin":
            self._scan_candidates = [port for port, _desc in payload]
            names = ", ".join(self._scan_candidates) or "(無候選埠)"
            self.status.set(f"掃描 {len(payload)} 個埠: {names}")
        elif kind == "scan_step":
            self.on_scan_step(payload)
        elif kind == "scan_done":
            self.on_scan_done(payload)
        if self.scope_win is not None and self.scope_win.alive:
            self.scope_win.on_event(kind, payload)

    def on_connected(self, info):
        self.connected = True
        # 連線當下的授權。之後哨兵若因失步而短暫不符，寫入會被暫時關掉；等框架
        # 重新對齊就從這個值恢復 —— 所以要另外記著，不能只靠 writes_allowed。
        self._writes_permitted = bool(info.get("writes_allowed"))
        self.writes_allowed = self._writes_permitted
        signature = info.get("signature")
        self._signature_state = info.get("signature_state") or (
            "ok" if info.get("signature_ok") else "desync")
        signature_text = self._show_sentinel(signature, self._signature_state)

        problems = []
        if self._signature_state == "noreply":
            # 握手成功但哨兵讀不回來。這**不是**版本問題 —— 位址沒有位移，是對方
            # 沒回話。指去重新建置 ELF 會讓人白花時間在對的檔案上。
            problems.append(
                "握手成功，但位址哨兵讀不回任何值 —— 目標端沒有回應。"
                "這不是 ELF 版本問題。最可能是 baud 不符或目標端的 LNet 解析器卡住；"
                "先按「斷線」再按「連線」，仍然不行就跑 check_link.py --scan。"
                "寫入已封鎖。")
        elif not info.get("signature_ok"):
            problems.append(
                f"位址哨兵 {signature_text} ≠ 0x{V.SIGNATURE_EXPECTED:04X} — "
                "這個 ELF 是舊的或別的 build。全域變數位址已位移，"
                "畫面上每一個數字都是錯位的別的變數，全部作廢。寫入已封鎖。")
        if info.get("stale"):
            problems.append(
                "過期的 BUILD：有韌體原始檔比這個 ELF 還新。請重新建置並燒錄，"
                "或用「選 ELF …」指定你真正燒進板子的那一份。")
        absent = info.get("absent") or []
        if absent:
            problems.append(f"{len(absent)} 個選用變數不在這個 build 裡，"
                            f"對應欄位會顯示 '--'：{', '.join(absent[:6])}"
                            f"{' …' if len(absent) > 6 else ''}")
        # 記著連線時的問題。之後失步會蓋上另一條橫幅，重新對齊後要還原成這一條 ——
        # 直接清空會把「build 過期」這種仍然成立的警告一起擦掉。
        self._connect_banner = "\n".join(problems)
        self.show_banner(self._connect_banner)

        baud = info.get("baud") or 0
        # 只陳述生效的 baud，不加價值判斷。原本會把非 DEFAULT_BAUD 的值標成橙色的
        # 「(舊 baud)」—— 但 115200 與 230400 都是韌體支援的正常設定，而且實測
        # 230400 在某些板子上不穩、115200 穩定。把使用者實測可用的那個標成「舊」
        # 並建議升級，等於推薦他回到會掉線的設定。
        self.link_info.set(f"{info.get('port', '?')} @ {baud:,}")
        self.link_info_label.configure(foreground=C["muted"])
        # 選單要跟著實際連上的值走 (掃描可能退回另一個 baud 才連上)，
        # 否則畫面上會出現選單寫 230400、標題列寫 115,200 的矛盾。
        if baud in V.KNOWN_BAUDS and baud != self._selected_baud():
            self.args.baud = baud
            self.baud_var.set(str(baud))
            V.save_last_baud(baud)
        self.status.set(f"已連線 — {info.get('device', '')}"[:110])
        self.elf_var.set(info.get("elf", self.elf))
        self.scope_btn.configure(state="normal")
        self.connect_btn.configure(text="斷線")
        self.values.update(info.get("info") or {})
        # 自動記下連線基準。這些計數器是**韌體開機後**累計的，所以裡面混著韌體
        # 啟動期的一次性事件 (實測 RX FIFO 丟棄固定是 27、最大用量固定填滿 127 ——
        # 跨電源重置都一樣，是連線握手時灌進來的，不是持續發生的故障)。
        # 把連線那一刻當基準，畫面上才分得出「韌體開機就有」與「這次 session 才發生」。
        self._connect_base = {}
        self._await_connect_base = True
        self._sync_write_state()
        self.refresh_table()
        self.refresh_fw_config()

    def _show_sentinel(self, signature, state):
        """更新標頭的哨兵指示燈。回傳顯示用的哨兵值字串。

        三態各有自己的字與顏色，不能共用「不符」：

        * ``對版``   綠 —— 位址正確，數值可信。
        * ``不符``   紅 —— 位址位移，畫面上每個數字都是別的變數。要換 ELF。
        * ``無回應`` 橘 —— 對方沒講話。位址沒有問題，數字只是舊的。要重連或重置目標端。

        紅與橘的分工是刻意的：紅色在這支工具裡專門表示「數值是錯的」，橘色表示
        「沒有數值」。把後者也印成紅色的「不符」會把使用者推去重新建置 ELF ——
        那是完全錯的方向，而這正是修掉的那個 bug。
        """
        try:
            text = "--" if signature is None else f"0x{int(signature):04X}"
        except (TypeError, ValueError):
            # 失步時讀回來可能是任何東西；這是 Tk 回呼，拋例外只會噴 traceback。
            text = repr(signature)[:12]
        word, colour = {"ok": ("對版", C["ok"]),
                        "noreply": ("無回應", C["warn"])}.get(
                            state, ("不符", C["danger"]))
        self.sentinel.set(f"哨兵 {text} {word}")
        self.sentinel_label.configure(foreground=colour)
        return text

    def on_signature(self, payload):
        ok = bool(payload.get("ok"))
        # 舊版的 payload 沒有 state；沒有它就退回原本的二分法，行為不變。
        state = payload.get("state") or ("ok" if ok else "desync")
        was = self._signature_state
        self._signature_state = state
        text = self._show_sentinel(payload.get("signature"), state)
        # 從連線時的授權重算，而不是 `writes_allowed and ok` —— 後者一旦被壓成 False
        # 就再也回不來，於是一次偶發失步之後就永遠不能寫入了。
        self.writes_allowed = self._writes_permitted and ok
        self._sync_write_state()
        if ok:
            self.show_banner(self._connect_banner)
            self.status.set("目標端恢復回應，數值恢復可信" if was == "noreply"
                            else "框架已重新對齊，數值恢復可信")
        elif state == "desync":
            self.show_banner(
                f"位址哨兵讀到 {text}（應為 0x{V.SIGNATURE_EXPECTED:04X}）。\n"
                "連線時哨兵是對的，所以這不是 ELF 版本問題，而是 LNet 框架失步 —— "
                "讀到的每個值都是錯位的別的變數。\n"
                "已在自動重新對齊；期間的資料一律丟棄不顯示，寫入暫時封鎖。"
                "若持續不恢復，請換一條 USB 線或換一個埠。")
        # state == "noreply" 時**刻意不動橫幅**：worker 緊接著會發一張講得更清楚的
        # (是 baud 不符還是曾經讀得到後來卡死，以及該怎麼做)。在這裡先貼一張
        # 「框架失步、已在自動重新對齊」只是先閃一下錯的訊息再被蓋掉 —— 而那句話
        # 在 noreply 路徑上根本是假的，那條路徑刻意不做重新對齊。

    def on_scope_active(self, active):
        """Scope 擷取開始/結束時凍結或恢復主頁捲動圖。

        判斷依據是「**是否正在擷取**」，不是「Scope 視窗是否開啟」—— 視窗開著但擷取
        已終止時，主頁沒有理由不畫。

        為什麼整段凍結而不是「有資料就畫」：擷取與遙測是交錯進行的，資料一陣一陣來，
        直接畫出來就是一下停一下動的斷續更新，在捲動圖上看起來像壞掉。凍結成一張
        「開始擷取前」的快照，本身也是有用的參考。

        ⚠ 只凍結繪圖，**不停遙測輪詢**。哨兵閘門、失步偵測與鏈路健康全靠輪詢，
        那也正是原本連續模式的致命問題 (擷取期間完全不輪詢 -> 出事時什麼都看不到)。
        """
        active = bool(active)
        if active == self.scope_capturing:
            return
        self.scope_capturing = active
        if not active:
            # 擷取結束後清掉軌跡。凍結期間跳過的取樣在時間軸上是個缺口，接著畫下去
            # 會把擷取前與擷取後接成連續的 —— 那是對時間軸說謊。清掉重新開始才誠實。
            self.clear_charts()
        self._update_chart_notice()

    def _update_chart_notice(self):
        """捲動圖上的凍結說明。手動暫停 (Chart 自己畫) 優先於這裡的原因。

        Scope 擷取排在遙測停止之前：擷取期間本來就不發遙測，那時的「停止」是預期
        行為，講成鏈路問題會誤導人。
        """
        if self.scope_capturing:
            note = "Scope 擷取中 · 繪圖凍結（擷取結束後恢復）"
        elif self._data_stale:
            # 曲線停住與鏈路掛掉長得一模一樣，所以一定要在圖上寫明白這是舊資料。
            note = "遙測已停止 · 以下是最後一筆資料"
        else:
            note = ""
        for chart, _traces in self.charts:
            chart.notice = note
            chart.draw()

    def on_scan_step(self, payload):
        port, desc, state = payload
        label = {"probing": "探測中 …", "found": "有回應 ✓",
                 "no-reply": "無回應",
                 "busy": "開不起來 (被其他程式占用)"}.get(state, state)
        self.status.set(f"{port}  {desc}  —  {label}")

    def on_scan_done(self, payload):
        self.scanning = False
        self.scan_btn.configure(state="normal")
        self._sync_baud_state()
        connect_next, self._connect_after_scan = self._connect_after_scan, False
        found = payload.get("found") or []
        busy = payload.get("busy") or []
        if not found:
            # 只列真正探測過的埠。把跳過的藍牙埠也算進「已試過」會讓人以為那邊
            # 已經排除掉了，於是漏掉「東西其實接在藍牙埠上」這條可能。
            # 被占用的埠同理 —— 它根本沒被探測到，不能算在「已排除」裡。
            tried = ", ".join(p for p in self._scan_candidates
                              if p not in busy) or "(無)"
            skipped = [port for port, _desc in list_serial_ports()
                       if port not in self._scan_candidates]
            if busy:
                # 這一段要排在最前面：它是可以馬上動手解決的，而下面那些檢查項目
                # (供電、USB 線、CODESW) 在這種情況下全都是白費工。
                message = (f"以下序列埠開不起來，被其他程式占用: {', '.join(busy)}\n"
                           "**這些埠沒有被排除** —— 裝置可能就在其中一個上面。\n"
                           "先關掉占用它的程式: 這支 GUI 的另一個視窗、還在跑的 "
                           "check_link.py、或終端機軟體 (PuTTY / 序列埠監控)。\n")
            else:
                message = "掃描完成，沒有任何序列埠回應 X2CScope 的 LNet 握手。\n"
            message += f"已探測: {tried}\n"
            if skipped:
                message += (f"已跳過 (藍牙虛擬埠): {', '.join(skipped)}"
                            "  —— 需要一併試的話加 --include-bluetooth\n")
            message += ("已試過 "
                        + " 與 ".join(f"{b:,}" for b in V.KNOWN_BAUDS)
                        + " baud，並沖洗過目標端卡住的半截 LNet 框架。\n")
            # 掉線的原因不能被掃描結果蓋掉。那句話往往是唯一的線索，而使用者按下
            # 「連線」就會把它換成這張橫幅 —— 於是每次都只剩「找不到裝置」可看。
            if self._last_link_error:
                message += f"上次掉線的原因: {self._last_link_error}\n"
            if not busy:
                message += ("檢查: 板子有供電、USB 線，以及 codeSw.h 的 "
                            "CODESW_X2C_SCOPE_ENABLE 仍為 1。"
                            "注意 X2CScope 走 UART2 (RB8/RB9)，不是 RS485 的 UART1。")
            self.show_banner(message)
            self.status.set("掃描完成 — 埠被占用" if busy else "掃描完成 — 找不到裝置")
            return
        port, desc, _info, baud = found[0]
        self.port_var.set(port)
        self.show_banner("")
        if baud != self.args.baud:
            # 掃描退回到另一個 baud 才連上 -> 板子燒的是那個值。把選單同步過去並
            # 記下來，下次就直接用對的，不會再發生錯 baud 探測。
            # 刻意不建議「重新建置燒錄改成 DEFAULT_BAUD」—— 兩個值都是正常設定，
            # 而實測 230400 在某些板子上會中途停止回應。
            self.args.baud = baud
            self.baud_var.set(str(baud))
            V.save_last_baud(baud)
            self.status.set(f"找到 {port} ({desc}) @ {baud:,}"
                            f" —— 選單已同步（韌體端的 X2C_BAUD_TARGET 就是這個值）")
        else:
            self.status.set(f"找到 {port} ({desc}) @ {baud:,}")
        if connect_next:
            self._start_link()

    def on_data(self, payload):
        values, rate, when = payload
        self.values.update(values)
        # 連線基準要等第一輪 RARE 掃完 (計數器在 RARE_VARS 裡，約 1 秒) 才湊齊。
        if self._await_connect_base and all(
                self.values.get(n) is not None for n in V.MONOTONIC_COUNTERS):
            self._connect_base = {n: self.values[n] for n in V.MONOTONIC_COUNTERS}
            self._await_connect_base = False
        self.rate.set(f"輪詢 {rate:4.1f} Hz")
        self._last_data = when
        if not self._first_data:
            self._first_data = when
        self._data_count += 1

        # 捲動圖的時間跨距 = 已取樣點數 × 實際週期。用實測而不是設定值，因為讀取
        # 本身要花時間，兩者會差上幾成。
        elapsed = when - self._first_data
        per_point = (elapsed / self._data_count) if self._data_count else 0.0
        span = per_point * min(self._data_count, TRACE_POINTS)

        # 暫停時完全不 append —— 不是畫了再蓋掉。這樣恢復後畫面上接的是「暫停前
        # 那一刻」的後續，中間沒有一段憑空補出來的資料。
        # Scope 擷取進行中同樣不 append，見 on_scope_active()。
        if not self.paused and not self.scope_capturing:
            converted = {}
            for name, fn in V.SYNTHETIC_TRACES.items():
                converted[name] = fn(self.values)
            for chart, traces in self.charts:
                frame = {}
                for _label, name, _colour, kind in traces:
                    raw = converted[name] if name in converted else self.values.get(name)
                    frame[name] = V.chart_value(kind, raw, self.scale)
                chart.append(frame, span)

        self.refresh_table()
        for row in self.tuning_rows + self.danger_rows:
            if row.name in values:
                row.refresh(values[row.name], self.scale)
        self.refresh_fw_config()

    def on_write_done(self, payload):
        name, want, got, ok, note = payload
        # 回讀值直接更新那一列的「目前值」。這是最重要的一步：對 persist == "loop"
        # 的變數，這一格會當場顯示韌體蓋回去的值，不必去翻紀錄分頁比對。
        if got is not None:
            for row in self.tuning_rows + self.danger_rows:
                if row.name == name:
                    row.refresh(got, self.scale)
        if not ok:
            tag, result = "fail", note or "失敗"
        elif note:
            tag, result = "overwritten", note
        else:
            tag, result = "ok", "已生效"
        self.log.insert("", 0, values=(time.strftime("%H:%M:%S"), name,
                                       "--" if want is None else f"{want:,}",
                                       "--" if got is None else f"{int(got):,}",
                                       result), tags=(tag,))
        self.status.set(f"{name}: {result}")

    # -- 更新 --------------------------------------------------------------
    def refresh_table(self):
        """只更新目前看得到的分頁。

        九個分頁全刷是 102 次 Treeview.set()，每個輪詢週期做一次會讓 Tk 明顯變鈍，
        而看不見的分頁刷了也沒人看 —— 切換分頁時會重刷。
        """
        try:
            page = self._tab_pages.get(self.notebook.select())
        except tk.TclError:
            return
        table = self.tables.get(page)
        if table is None:
            return          # 目前在調校 / 刻度 / 紀錄之類的非資料分頁
        for index, (_label, name, kind) in enumerate(V.PANELS[page]):
            table.set(str(index), "value",
                      V.format_row(kind, self.values.get(name), self.scale))
            table.set(str(index), "delta", self._delta_text(name))

    def _delta_text(self, name):
        """計數器的增量字串；非計數器或還沒有基準時回空字串。

        手動基準優先於自動的連線基準 —— 按下按鈕就是明確表示「我要從現在起算」。
        +0 也要顯示：「確認沒有增加」與「不知道有沒有增加」是兩件事，
        而後者正是這個功能要消滅的。
        """
        current = self.values.get(name)
        if current is None:
            return ""
        for base_map, label in ((self._counter_base, "自歸零"),
                                (self._connect_base, "自連線")):
            base = base_map.get(name)
            if base is None:
                continue
            # 一定要用模數算術。這些計數器全是 uint16_t，直接相減在迴繞後會得到負值
            # —— 而負值看起來像「韌體重置了」。實測被這件事誤導過：
            # 58,538 -> 6,588 直接相減是 -51,950，其實是 +13,586 的迴繞。
            delta = V.counter_delta(base, current)
            return "" if delta is None else f"{label} +{delta:,}"
        return ""

    def refresh_fw_config(self):
        pole = self.values.get("s_currentMotorConfig.u8PolePairs")
        wheel = self.values.get("s_currentMotorConfig.u16WheelDimensionInches")
        hall_ppr = self.values.get("s_currentMotorConfig.u16HallPPR")
        source = self.values.get("s_currentMotorConfig.eSpeedSource")
        min_period = self.values.get("HallMinPeriod")
        lines = [
            f"極對數        {pole if pole is not None else '--'}"
            f"      (主機假設 {self.scale.pole_pairs})",
            f"輪徑          {V.format_row('inch_x10', wheel)}"
            f"      (主機假設 {self.scale.wheel_inch_x10 / 10:.1f} 吋)",
            f"Hall PPR      {hall_ppr if hall_ppr is not None else '--'}",
            f"速度來源      {V.decode(V.SPEED_SOURCE, source)}",
            f"HallMinPeriod {min_period if min_period is not None else '--'}"
            f"      (主機算出 {self.scale.hall_min_period})",
        ]
        self.fw_config.set("\n".join(lines))

    def show_banner(self, text):
        """對版/缺變數警告的紅色橫幅。沒有問題時完全不佔版面。"""
        if text:
            self.banner.set(text)
            if not self.banner_label.winfo_ismapped():
                body = self._body_widget()
                if body is not None:
                    self.banner_label.pack(fill="x", padx=12, pady=(0, 4), before=body)
                else:
                    self.banner_label.pack(fill="x", padx=12, pady=(0, 4))
        else:
            self.banner.set("")
            if self.banner_label.winfo_ismapped():
                self.banner_label.pack_forget()

    def _body_widget(self):
        """橫幅要插在標題列與主體之間，所以需要主體那個 widget。"""
        for child in self.winfo_children():
            if isinstance(child, ttk.PanedWindow):
                return child
        return None

    def _on_resize(self, event):
        """讓橫幅的換行寬度跟著視窗走。

        ⚠ 這個回呼會收到**所有子 widget** 的 <Configure>：Tk 把 toplevel 放在每個
        後代的 bindtags 裡。所以一定要先擋掉不是自己的事件，否則捲動圖每次重畫都會
        跑進來一次。
        只在數值真的改變時才設 —— 設 wraplength 會觸發重新排版，無條件設會變成
        Configure -> 排版 -> Configure 的迴圈。
        """
        if event.widget is not self:
            return
        wrap = max(400, event.width - 40)      # 40 = 左右 padx(12) + 標籤內 padx(12)
        if wrap != self.banner_label.cget("wraplength"):
            self.banner_label.configure(wraplength=wrap)

    def _stale_limit_s(self):
        return max(STALE_MIN_S, STALE_INTERVAL_MULT * self._interval_ms() / 1000.0)

    def _check_stale(self):
        """遙測看門狗：偵測「還連著，但已經收不到資料」這個狀態。

        哨兵閘門判定資料不可信時 worker 那一輪不發 data，而且**不會斷線** (刻意的
        —— 斷線會把畫面上最後一筆證據一起清掉)。少了這個看門狗，標頭上的輪詢率、
        捲動圖與圖例的即時值就全部凍在最後一筆好資料上，看起來跟正常運轉一樣。

        Scope 擷取期間不算停止：那時遙測是**被刻意停掉**的 (見 on_scope_active)，
        不是鏈路有問題。順手把心跳往前推，擷取結束的瞬間才不會閃一下紅字。
        """
        was = self._data_stale
        now = time.monotonic()
        if self.scope_capturing:
            self._last_data = now
        gap = 0.0
        if not self.connected or not self._last_data:
            self._data_stale = False
        else:
            gap = now - self._last_data
            self._data_stale = gap > self._stale_limit_s()
        if self._data_stale:
            # 秒數每個 tick 都更新 —— 「掛了 3 秒」與「掛了 20 分鐘」的處置不同，
            # 而 worker 的橫幅只能每 10 秒重發一次，解析度不夠。
            self.rate.set(f"遙測停止 {gap:6.1f} s")
        if self._data_stale == was:
            return
        self.rate_label.configure(
            foreground=C["danger"] if self._data_stale else C["muted"])
        if not self._data_stale:
            # 恢復時先寫回一個中性值。下一個 data 事件 (最多一個輪詢週期後) 會蓋上
            # 真的頻率；空著會讓「遙測停止 12.3 s」留在畫面上假裝還沒好。
            self.rate.set("輪詢 -- Hz")
        self._update_chart_notice()

    def _sync_write_state(self):
        for row in self.tuning_rows:
            row.set_enabled(self.writes_allowed)
        unlocked = self.writes_allowed and bool(self.danger_unlocked.get())
        for row in self.danger_rows:
            row.set_enabled(unlocked)
        # 讀取只需要連線。哨兵不符時讀回來的是錯位位址上的別的變數，但那件事由紅色
        # 橫幅負責講清楚 —— 唯讀分頁在同樣情況下也照常顯示數值，這裡保持一致。
        for button in (getattr(self, "_read_btn_tuning", None),
                       getattr(self, "_read_btn_danger", None)):
            if button is not None:
                button.configure(state="normal" if self.connected else "disabled")

    # -- 動作 --------------------------------------------------------------
    def toggle_pause(self):
        """凍結/恢復捲動圖。只影響繪圖 —— 遙測輪詢與唯讀分頁照常更新。

        刻意不停掉輪詢：暫停的用途是「把剛剛那一段抓在畫面上看清楚」，而不是
        中斷與車子的連線；停掉輪詢還會讓鏈路健康計數器與故障旗標一起停止更新。
        """
        self.paused = not self.paused
        for chart, _traces in self.charts:
            chart.paused = self.paused
            chart.draw()
        self.pause_btn.configure(text="繼續繪圖" if self.paused else "暫停繪圖")
        self.pause_note.set("繪圖已凍結，遙測仍在更新" if self.paused else "")


    def zero_counters(self):
        """把目前的單調計數器值記成基準；之後各列會多顯示自基準以來的增量。

        只影響主機端顯示，不寫韌體的計數器 —— 那些是 volatile 全域，技術上寫得下去，
        但清掉別人的診斷資料不是這個工具該做的事。再按一次可取消基準。
        """
        if self._counter_base:
            self._counter_base = {}
            self.baseline_btn.configure(text="計數器歸零")
            self.status.set("已取消計數器基準，回到顯示累計值")
        else:
            self._counter_base = {name: self.values.get(name)
                                  for name in V.MONOTONIC_COUNTERS
                                  if self.values.get(name) is not None}
            self.baseline_btn.configure(text="取消基準")
            self.status.set(f"已記下 {len(self._counter_base)} 個計數器基準，"
                            "之後顯示自此的增量")
        self.refresh_table()

    def clear_charts(self):
        """清掉所有軌跡。要看下一次動作的乾淨波形時比等舊資料滾出去快。"""
        for chart, _traces in self.charts:
            for seq in chart.points.values():
                seq.clear()
            chart.span_s = 0.0
            chart.draw()

    def read_write_values(self):
        """把「調校」與「危險」兩頁的目前值從韌體讀回來。"""
        if self.link is None:
            messagebox.showwarning("未連線", "請先連線。")
            return
        note = ""
        if self._data_stale:
            # 目標端沒回應時每顆變數都會卡滿 pyserial 的 1 秒逾時，40 顆就是 40 秒。
            # 不先講清楚的話，使用者會以為程式當掉了。按鈕刻意保持可按 —— 目標端
            # 可能已經恢復，而重試一次是判斷它有沒有恢復最直接的方法。
            note = "（遙測已停止，每顆都要等逾時，最久約 40 秒）"
        self.status.set(f"讀取 {len(V.WRITE_VARS)} 個可寫參數 …{note}")
        self.link.submit("read_writes")

    def on_write_values(self, payload):
        """payload = (values, 讀取失敗的變數名)。

        失敗名單一定要分開拿。原本只看「值是不是 None」，於是鏈路掛掉時整排 None
        被講成「40 項不在這個 build 裡」—— 那是完全錯的診斷，而且正好把人推去
        懷疑韌體少編了東西。「不在 build 裡」與「讀不回來」兩件事的處置完全不同。
        """
        values, failed = payload if isinstance(payload, tuple) else (payload, [])
        failed = set(failed or ())
        for row in self.tuning_rows + self.danger_rows:
            row.refresh(values.get(row.name), self.scale)
        absent = [n for n, v in values.items() if v is None and n not in failed]
        got = len(values) - len(failed) - len(absent)
        parts = [f"已讀回 {got} 項"]
        if failed:
            parts.append(f"{len(failed)} 項讀取失敗（目標端沒有回應）")
        if absent:
            parts.append(f"{len(absent)} 項不在這個 build 裡")
        self.status.set("，".join(parts))

    def request_write(self, name, value):
        if self.link is None:
            messagebox.showwarning("未連線", "請先連線。")
            return
        if not self.writes_allowed:
            # 原因要照哨兵的三態分流。一句「位址哨兵不符 (ELF 不對版)」在目標端
            # 只是沒回話的時候是錯的診斷 —— 使用者會跑去重新建置一個本來就對的 ELF。
            reason = {
                "noreply": "目標端沒有回應，寫不進去。先按「斷線」再按「連線」試一次。",
                "desync": "位址哨兵不符：ELF 不對版或 LNet 框架失步，"
                          "寫入會落在錯誤的位址上。",
            }.get(self._signature_state, "唯讀模式（--read-only）。")
            messagebox.showwarning("寫入已封鎖", reason)
            return
        self.link.submit("write", (name, value))

    def apply_scale(self):
        try:
            self.scale = V.Scale(**{attr: int(float(var.get()))
                                    for attr, var in self.scale_vars.items()})
        except (ValueError, ZeroDivisionError) as exc:
            messagebox.showerror("刻度無效", f"參數不是有效整數: {exc}")
            return
        self.scale_desc.set(self.scale.describe())
        self.refresh_table()
        self.refresh_fw_config()
        for row in self.tuning_rows + self.danger_rows:
            row.refresh(self.values.get(row.name), self.scale)
        if self.scope_win is not None and self.scope_win.alive:
            self.scope_win.scale = self.scale

    def reset_scale(self):
        defaults = V.Scale()
        for attr, var in self.scale_vars.items():
            var.set(str(getattr(defaults, attr)))
        self.apply_scale()

    def open_scope(self):
        if self.scope_win is not None and self.scope_win.alive:
            self.scope_win.lift()
            return
        from scope_window import ScopeWindow
        self.scope_win = ScopeWindow(self)

    def on_close(self):
        # 先取消排定的抽取回呼。不取消的話它會在 destroy() 之後觸發，
        # Tk 會噴 'invalid command name ...drain'。
        if self._tick is not None:
            self.after_cancel(self._tick)
            self._tick = None
        # 兩條都要停。_closing_link 是還在收尾的舊 link。
        for link in (self.link, self._closing_link):
            if link is not None:
                link.stop()
        self._shutdown_deadline = time.monotonic() + SHUTDOWN_WAIT_S
        self._finish_close()

    def _finish_close(self):
        """等 worker 放掉序列埠再真的關掉視窗。

        為什麼不能直接 destroy(): worker 是 daemon 執行緒，而 CPython 在解譯器結束時
        「殺掉」daemon 執行緒的方式是等它下一次要取得 GIL 時才讓它死。卡在 ReadFile
        裡的執行緒 (GIL 已釋放) 會先等滿 pyserial 的 timeout 才輪到那一步 —— 這段時間
        行程還沒真正結束，序列埠還在它手上。使用者這時重開程式就會開不了埠。

        所以視窗要留著、狀態列要說明在等什麼，而不是看起來關掉了卻在背景抓著埠。
        """
        alive = [link for link in (self.link, self._closing_link)
                 if link is not None and link.is_alive()]
        if alive and time.monotonic() < self._shutdown_deadline:
            self.status.set("關閉序列埠中 …")
            self.after(60, self._finish_close)
            return
        self.destroy()


def main():
    # --help 會印中文的 docstring，在 cp1252 主控台下那一步就會 UnicodeEncodeError。
    force_utf8_console()
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    # 支援問題回報時第一件要問的事：「你跑的是哪一版？」
    parser.add_argument("--version", action="version",
                        version=f"LWFDSPC 主機端工具 {V.GUI_VERSION}")
    parser.add_argument("--port", default=None, help='COM port，或 "AUTO"')
    # default=None 才分得出「使用者明確指定」與「沒給」。沒給時用上次成功的 baud，
    # 那比一律套 DEFAULT_BAUD 好：韌體的 X2C_BAUD_TARGET 一旦改成另一個值，
    # 每次開程式都要重新選一遍。
    parser.add_argument("--baud", type=int, default=None,
                        choices=V.KNOWN_BAUDS,
                        help=f"UART2 baud（預設：上次成功的值，或 {V.DEFAULT_BAUD}）")
    parser.add_argument("--elf", default=str(V.DEFAULT_ELF))
    parser.add_argument("--interval", type=int, default=125, help="輪詢間隔 ms")
    parser.add_argument("--rare-slice", type=int, default=8,
                        help="每個週期讀幾顆 RARE 變數 (切片大小)")
    parser.add_argument("--read-only", action="store_true", help="完全停用寫入")
    parser.add_argument("--include-bluetooth", action="store_true",
                        help="掃描時也試藍牙虛擬 COM 埠 (預設跳過：開啟未連線的"
                             "藍牙埠可能阻塞數秒，且它幾乎不可能是除錯 UART)")
    parser.add_argument("--demo", action="store_true",
                        help="模擬模式，不開序列埠 (驗版面用)")
    parser.add_argument("--bad-signature", action="store_true",
                        help="--demo 時強制哨兵不符，驗封鎖路徑")
    args = parser.parse_args()
    if args.baud is None:
        args.baud = V.load_last_baud() or V.DEFAULT_BAUD
    App(args).mainloop()


if __name__ == "__main__":
    main()
