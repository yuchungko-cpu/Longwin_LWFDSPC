"""LWFDSPC 主機端診斷儀表板 (X2CScope / UART2)。

觀測 + 調校參數寫入。**不是**「從 PC 開車」的工具 —— 這顆控制器的命令來源是油門/VR、
煞車開關、排檔開關與 Modbus 儀表，韌體裡沒有給主機下命令的 apiData 介面。

用法:
    python motor_gui.py --demo                # 無硬體，驗版面
    python motor_gui.py --port COM12          # 接上車
    python motor_gui.py --port AUTO --read-only

先跑 check_link.py 再開這支：它把通訊問題與 GUI 問題隔開來。

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
        # 已呼叫 stop() 但執行緒還沒結束的舊 link，見 _watch_closing()。
        self._closing_link = None
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
        ttk.Label(header, textvariable=self.rate, style="Muted.TLabel",
                  font=("Consolas", 9)).pack(side="right")
        self.sentinel_label = ttk.Label(header, textvariable=self.sentinel,
                                        style="Muted.TLabel", font=("Consolas", 9))
        self.sentinel_label.pack(side="right", padx=16)
        # 生效的埠與 baud。掃描可能退回舊 baud，而不顯示的話完全看不出來連的是哪個
        # —— 而 baud 決定了頻寬上限，是判斷 FIFO 節流的前提。
        self.link_info_label = ttk.Label(header, textvariable=self.link_info,
                                         style="Muted.TLabel", font=("Consolas", 9))
        self.link_info_label.pack(side="right", padx=16)

        self.banner_label = tk.Label(self, textvariable=self.banner, anchor="w",
                                     justify="left", background=C["danger"],
                                     foreground=C["on_danger"], padx=12, pady=6,
                                     font=(self.font, 9, "bold"), wraplength=1400)

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
        self.link = factory(self.port_var.get() if not self.args.demo else "DEMO",
                            self.args.baud, self.elf, interval, self.events,
                            rare_slice=self.args.rare_slice,
                            allow_writes=not self.args.read_only, **kwargs)
        self.status.set("連線中 …")
        self.connect_btn.configure(text="斷線")
        self._first_data = self._data_count = 0
        self.link.start()

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
        self.rate.set("輪詢 -- Hz")
        self.link_info.set("")
        # 擷取狀態跟著這次連線作廢，否則斷線時若正在擷取，捲動圖會永遠凍住。
        self.on_scope_active(False)
        self._watch_closing()

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
        self._tick = self.after(UI_TICK_MS, self.drain)

    def _dispatch(self, kind, payload):
        if kind == "status":
            self.status.set(payload)
        elif kind == "error":
            self.status.set(payload.splitlines()[0])
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
        signature_text = "--" if signature is None else f"0x{int(signature):04X}"
        if info.get("signature_ok"):
            self.sentinel.set(f"哨兵 {signature_text} 對版")
            self.sentinel_label.configure(foreground=C["ok"])
        else:
            self.sentinel.set(f"哨兵 {signature_text} 不符")
            self.sentinel_label.configure(foreground=C["danger"])

        problems = []
        if not info.get("signature_ok"):
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
        legacy = "  (舊 baud)" if baud and baud != V.DEFAULT_BAUD else ""
        self.link_info.set(f"{info.get('port', '?')} @ {baud:,}{legacy}")
        self.link_info_label.configure(
            foreground=C["warn"] if legacy else C["muted"])
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

    def on_signature(self, payload):
        ok = bool(payload.get("ok"))
        signature = payload.get("signature")
        try:
            text = "--" if signature is None else f"0x{int(signature):04X}"
        except (TypeError, ValueError):
            # 失步時讀回來可能是任何東西；這是 Tk 回呼，拋例外只會噴 traceback。
            text = repr(signature)[:12]
        self.sentinel.set(f"哨兵 {text} {'對版' if ok else '不符'}")
        self.sentinel_label.configure(foreground=C["ok"] if ok else C["danger"])
        # 從連線時的授權重算，而不是 `writes_allowed and ok` —— 後者一旦被壓成 False
        # 就再也回不來，於是一次偶發失步之後就永遠不能寫入了。
        self.writes_allowed = self._writes_permitted and ok
        self._sync_write_state()
        if ok:
            self.show_banner(self._connect_banner)
            self.status.set("框架已重新對齊，數值恢復可信")
        else:
            self.show_banner(
                f"位址哨兵讀到 {text}（應為 0x{V.SIGNATURE_EXPECTED:04X}）。\n"
                "連線時哨兵是對的，所以這不是 ELF 版本問題，而是 LNet 框架失步 —— "
                "讀到的每個值都是錯位的別的變數。\n"
                "已在自動重新對齊；期間的資料一律丟棄不顯示，寫入暫時封鎖。"
                "若持續不恢復，請換一條 USB 線或換一個埠。")

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
        """捲動圖上的凍結說明。手動暫停優先於擷取中。"""
        note = "Scope 擷取中 · 繪圖凍結（擷取結束後恢復）" if self.scope_capturing else ""
        for chart, _traces in self.charts:
            chart.notice = note
            chart.draw()

    def on_scan_step(self, payload):
        port, desc, state = payload
        label = {"probing": "探測中 …", "found": "有回應 ✓",
                 "no-reply": "無回應"}.get(state, state)
        self.status.set(f"{port}  {desc}  —  {label}")

    def on_scan_done(self, found):
        self.scanning = False
        self.scan_btn.configure(state="normal")
        connect_next, self._connect_after_scan = self._connect_after_scan, False
        if not found:
            # 只列真正探測過的埠。把跳過的藍牙埠也算進「已試過」會讓人以為那邊
            # 已經排除掉了，於是漏掉「東西其實接在藍牙埠上」這條可能。
            tried = ", ".join(self._scan_candidates) or "(無)"
            skipped = [port for port, _desc in list_serial_ports()
                       if port not in self._scan_candidates]
            message = ("掃描完成，沒有任何序列埠回應 X2CScope 的 LNet 握手。\n"
                       f"已探測: {tried}\n")
            if skipped:
                message += (f"已跳過 (藍牙虛擬埠): {', '.join(skipped)}"
                            "  —— 需要一併試的話加 --include-bluetooth\n")
            message += ("檢查: 板子有供電、USB 線，以及 codeSw.h 的 "
                        "CODESW_X2C_SCOPE_ENABLE 仍為 1。"
                        "注意 X2CScope 走 UART2 (RB8/RB9)，不是 RS485 的 UART1。")
            self.show_banner(message)
            self.status.set("掃描完成 — 找不到裝置")
            return
        port, desc, _info, baud = found[0]
        self.port_var.set(port)
        self.show_banner("")
        if baud != self.args.baud:
            # 掃描退回到舊 baud 才連上 -> 板子燒的是 2026-08-25 之前的韌體。
            # 這能用，但值得講清楚，否則使用者不會知道自己少了一半頻寬。
            self.args.baud = baud
            self.status.set(f"找到 {port} ({desc}) @ {baud} —— 舊韌體 baud，"
                            f"重新建置燒錄可提升到 {V.DEFAULT_BAUD}")
        else:
            self.status.set(f"找到 {port} ({desc}) @ {baud}")
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
        self.status.set(f"讀取 {len(V.WRITE_VARS)} 個可寫參數 …")
        self.link.submit("read_writes")

    def on_write_values(self, values):
        for row in self.tuning_rows + self.danger_rows:
            row.refresh(values.get(row.name), self.scale)
        missing = [n for n, v in values.items() if v is None]
        if missing:
            self.status.set(f"已讀回 {len(values) - len(missing)} 項，"
                            f"{len(missing)} 項不在這個 build 裡")
        else:
            self.status.set(f"已讀回 {len(values)} 個可寫參數")

    def request_write(self, name, value):
        if self.link is None:
            messagebox.showwarning("未連線", "請先連線。")
            return
        if not self.writes_allowed:
            messagebox.showwarning("寫入已封鎖",
                                   "唯讀模式，或位址哨兵不符 (ELF 不對版)。")
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
    parser.add_argument("--baud", type=int, default=V.DEFAULT_BAUD)
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
    App(args).mainloop()


if __name__ == "__main__":
    main()
