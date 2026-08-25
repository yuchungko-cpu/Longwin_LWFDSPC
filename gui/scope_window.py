"""X2CScope 擷取視窗 —— 真波形，不是輪詢遙測。

與儀表板的捲動圖根本不同：目標端在 ADC ISR 裡 (20kHz) 把通道寫進自己的 5000 byte
緩衝區，擷取完成後才整批傳回主機。所以這裡看到的是 50µs 解析度的真波形，而輪詢遙測
的解析度只有幾十毫秒 —— 斜率限制器的暫態、換相抖動這類東西只有在這裡看得到。

本視窗**不自己開連線**：一個 COM 埠只能開一次，所以一律走 motor_gui 共用的
x2c_link.Link，透過它的 command queue 下配置與啟動。

Component: HOST TOOLING
"""

import csv
import json
import tkinter as tk
from tkinter import filedialog, messagebox, ttk

import matplotlib

matplotlib.use("TkAgg")
# matplotlib 預設的 DejaVu Sans 沒有中文字，軸標籤與圖例會變成一排方框並噴一堆
# findfont 警告。unicode_minus 也要關掉 —— 中文字型多半沒有 U+2212 那個負號字符，
# 負的 Y 軸刻度會顯示成方框，而電流回充與速度反轉正好都是負值。
matplotlib.rcParams["font.sans-serif"] = [
    "Microsoft JhengHei", "Microsoft YaHei", "Noto Sans TC", "DejaVu Sans"]
matplotlib.rcParams["axes.unicode_minus"] = False

from matplotlib.backends.backend_tkagg import (FigureCanvasTkAgg,  # noqa: E402
                                               NavigationToolbar2Tk)
from matplotlib.figure import Figure  # noqa: E402

import x2c_vars as V  # noqa: E402
from x2c_link import load_elf_scalars_async  # noqa: E402

# 與 motor_gui 同一組配色。這裡重複一份而不是 import，是為了讓 scope_window 可以
# 被單獨 import 做測試而不牽動整個 GUI。
C = {
    "bg": "#eef1f4", "panel": "#ffffff", "row": "#e3e7ec", "edge": "#c3c9d1",
    "text": "#1a1f26", "muted": "#5a6672", "accent": "#1668b3",
    "ok": "#1a7a3e", "warn": "#b45309", "danger": "#c0392b",
    "disabled": "#a3abb4",
    "plot_bg": "#ffffff", "grid": "#dde1e6", "trigger": "#c2410c",
}

# Y 軸換算模式。刻意做成使用者明選而不是自動判斷：一次擷取可以同時包含速度域與
# 電流域的通道，自動換算一定會把其中一組畫錯，而畫錯的圖看起來跟畫對的一樣。
AXIS_MODES = (
    ("原始 count", "raw", "count"),
    ("Q15 → km/h", "q15_kmh", "km/h"),
    ("Q15 → A", "q15_amp", "A"),
    ("Q15 → %滿刻度", "q15_pct", "%FS"),
)

EDGES = (("上升緣", 1), ("下降緣", 0))
MODES = (("Triggered 等觸發", 1), ("Auto 自由跑", 0))


class ScopeWindow(tk.Toplevel):
    def __init__(self, app):
        super().__init__(app)
        self.app = app
        self.scale = app.scale
        self.font = app.font
        self.alive = True
        self.title(f"X2CScope 擷取 {V.GUI_VERSION} — LWFDSPC")
        self.geometry("1320x820")
        self.minsize(1080, 640)
        self.configure(background=C["bg"])

        self.selected = list(V.SCOPE_DEFAULT)
        self.custom = self._load_custom()
        self.elf_scalars = []
        self.last_capture = None
        # 由 _make_hover_artists() 在每次 redraw 後建立 (axes.clear 會移除它們)。
        self._hover_line = None
        self._hover_bg = None      # blit 用的背景快照
        self._hover_index = None   # 目前顯示的取樣索引，用來跳過無變化的事件
        self._capturing_bg = False  # 見 _invalidate_hover_bg 的說明
        self._redraw_pending = False  # 見 _request_redraw 的說明
        self._hover_px_last = None  # 上一次十字線的像素 x，blit 區域要涵蓋它
        self.cursor_time = tk.StringVar(value="")

        self.state_text = tk.StringVar(value="尚未配置")
        self.estimate = tk.StringVar(value="")
        self.axis_mode = tk.StringVar(value=AXIS_MODES[0][0])
        self.dashed = tk.BooleanVar(value=False)
        self.factor_var = tk.StringVar(value="50")     # ScopeConfig.cfg 的 stf=50
        self.trigger_on = tk.BooleanVar(value=True)
        self.trigger_src = tk.StringVar(value="ReferenceRAWSet")
        self.trigger_level = tk.StringVar(value="5000")   # ScopeConfig.cfg 的實用值
        self.trigger_edge = tk.StringVar(value=EDGES[0][0])
        self.trigger_delay = tk.StringVar(value="30")
        self.trigger_mode = tk.StringVar(value=MODES[0][0])
        self.level_hint = tk.StringVar(value="")

        self._build()
        self.protocol("WM_DELETE_WINDOW", self.on_close)
        self._refresh_selected()
        # ELF 符號表**不在開窗時載入**。那是 1.5 秒的 DWARF 解析，而大多數時候使用者
        # 只是要用左邊那 50 條精選通道，根本不會碰「ELF 全域」。改成第一次真的切到
        # 那個分頁時才載 —— 開窗因此是純 UI 動作，不做任何重活。
        self.scalars_requested = False

    # -- 版面 --------------------------------------------------------------
    def _build(self):
        root = ttk.Frame(self, padding=8)
        root.pack(fill="both", expand=True)
        left = ttk.Frame(root, width=400)
        left.pack(side="left", fill="y", padx=(0, 8))
        left.pack_propagate(False)
        right = ttk.Frame(root)
        right.pack(side="left", fill="both", expand=True)

        self._build_channels(left)
        self._build_trigger(left)
        self._build_actions(left)
        self._build_plot(right)

    def _build_channels(self, parent):
        book = ttk.Notebook(parent)
        book.pack(fill="both", expand=True)
        self.channel_book = book

        # -- 精選通道 --
        curated = _Scroller(book)
        book.add(curated, text=f"精選 {len(V.SCOPE_CHANNELS)}")
        self.channel_vars = {}
        for index, (label, name) in enumerate(V.SCOPE_CHANNELS):
            var = tk.BooleanVar(value=name in self.selected)
            self.channel_vars[name] = var
            ttk.Checkbutton(curated.body, text=f"{label}   {name}", variable=var,
                            style="Chan.TCheckbutton",
                            command=lambda n=name: self._toggle(n)).grid(
                row=index, column=0, sticky="w", padx=6, pady=1)

        # -- 從 ELF 自由挑選 --
        free = ttk.Frame(book, style="Panel.TFrame", padding=6)
        book.add(free, text="ELF 全域")
        ttk.Label(free, style="Doc.TLabel", font=(self.font, 8), wraplength=350,
                  justify="left",
                  text="build 裡任何純量全域都能擷取。加入的通道會存下來，下次開啟仍在。"
                  ).pack(anchor="w", pady=(0, 6))
        self.filter_var = tk.StringVar()
        entry = ttk.Entry(free, textvariable=self.filter_var, font=("Consolas", 9))
        entry.pack(fill="x")
        entry.bind("<KeyRelease>", lambda _e: self._refill_free())
        self.free_list = tk.Listbox(free, background=C["panel"], foreground=C["text"],
                                    selectbackground=C["accent"],
                                    selectforeground="#ffffff", borderwidth=0,
                                    highlightthickness=1,
                                    highlightbackground=C["edge"],
                                    font=("Consolas", 8), activestyle="none")
        self.free_list.pack(fill="both", expand=True, pady=6)
        self.free_list.bind("<Double-Button-1>", lambda _e: self._add_free())
        ttk.Button(free, text="加入所選通道", command=self._add_free).pack(fill="x")
        self.free_note = tk.StringVar(value="切到這個分頁時才會載入符號表 (約 1.5 秒)")
        ttk.Label(free, textvariable=self.free_note, style="Doc.TLabel",
                  font=(self.font, 8)).pack(anchor="w", pady=(4, 0))
        self.free_tab = free
        book.bind("<<NotebookTabChanged>>", lambda _e: self._on_channel_tab())

        # -- 已選 --
        chosen = ttk.Frame(parent, style="Panel.TFrame", padding=6)
        chosen.pack(fill="x", pady=(6, 0))
        self.chosen_title = tk.StringVar()
        ttk.Label(chosen, textvariable=self.chosen_title, style="Row.TLabel",
                  font=(self.font, 9, "bold")).pack(anchor="w")
        self.chosen_body = ttk.Frame(chosen, style="Panel.TFrame")
        self.chosen_body.pack(fill="x", pady=(4, 0))
        ttk.Label(chosen, textvariable=self.estimate, style="Doc.TLabel",
                  font=("Consolas", 8), justify="left").pack(anchor="w", pady=(6, 0))

    def _build_trigger(self, parent):
        box = ttk.Frame(parent, style="Panel.TFrame", padding=6)
        box.pack(fill="x", pady=(6, 0))
        ttk.Label(box, text="取樣與觸發", style="Row.TLabel",
                  font=(self.font, 9, "bold")).grid(row=0, column=0, columnspan=3,
                                                    sticky="w", pady=(0, 6))

        ttk.Label(box, text="取樣分頻", style="Row.TLabel").grid(row=1, column=0, sticky="w")
        factor = ttk.Entry(box, textvariable=self.factor_var, width=7, justify="right",
                           font=("Consolas", 9))
        factor.grid(row=1, column=1, sticky="w", padx=6)
        factor.bind("<KeyRelease>", lambda _e: self._refresh_estimate())
        ttk.Label(box, text="1 = 每個 ISR 都取", style="Doc.TLabel",
                  font=(self.font, 8)).grid(row=1, column=2, sticky="w")

        ttk.Checkbutton(box, text="啟用觸發", variable=self.trigger_on,
                        style="Chan.TCheckbutton", command=self._sync_trigger).grid(
            row=2, column=0, columnspan=3, sticky="w", pady=(8, 2))

        self.trigger_widgets = []

        def row(index, label, widget):
            ttk.Label(box, text=label, style="Row.TLabel").grid(
                row=index, column=0, sticky="w", pady=2)
            widget.grid(row=index, column=1, columnspan=2, sticky="w", padx=6, pady=2)
            self.trigger_widgets.append(widget)

        self.src_box = ttk.Combobox(box, textvariable=self.trigger_src, width=26,
                                    state="readonly", font=("Consolas", 8))
        row(3, "來源", self.src_box)
        level = ttk.Entry(box, textvariable=self.trigger_level, width=10,
                          justify="right", font=("Consolas", 9))
        level.bind("<KeyRelease>", lambda _e: self._refresh_level_hint())
        row(4, "位準", level)
        ttk.Label(box, textvariable=self.level_hint, style="Doc.TLabel",
                  font=("Consolas", 8)).grid(row=5, column=1, columnspan=2, sticky="w")
        row(6, "邊緣", ttk.Combobox(box, textvariable=self.trigger_edge, width=10,
                                    state="readonly",
                                    values=[label for label, _v in EDGES]))
        row(7, "延遲 %", ttk.Entry(box, textvariable=self.trigger_delay, width=10,
                                   justify="right", font=("Consolas", 9)))
        row(8, "模式", ttk.Combobox(box, textvariable=self.trigger_mode, width=18,
                                    state="readonly",
                                    values=[label for label, _v in MODES]))
        self._refresh_level_hint()

    def _build_actions(self, parent):
        box = ttk.Frame(parent, padding=(0, 8, 0, 0))
        box.pack(fill="x")
        ttk.Button(box, text="配置", command=self.configure_scope).pack(side="left")
        ttk.Button(box, text="單次擷取", command=lambda: self.start(False)).pack(side="left", padx=4)
        ttk.Button(box, text="連續", command=lambda: self.start(True)).pack(side="left")
        ttk.Button(box, text="中止", command=self.abort).pack(side="left", padx=4)
        ttk.Label(parent, textvariable=self.state_text, style="Muted.TLabel",
                  wraplength=380, justify="left").pack(anchor="w", pady=(6, 0))

    def _build_plot(self, parent):
        top = ttk.Frame(parent)
        top.pack(fill="x")
        ttk.Label(top, text="Y 軸", style="Muted.TLabel").pack(side="left")
        combo = ttk.Combobox(top, textvariable=self.axis_mode, width=16, state="readonly",
                             values=[label for label, _k, _u in AXIS_MODES])
        combo.pack(side="left", padx=6)
        combo.bind("<<ComboboxSelected>>", lambda _e: self.redraw())
        # 預設關掉：虛線在密集/雜訊大的軌跡上會糊成一團。只有在軌跡真的重疊到
        # 分不出來時才需要它。
        ttk.Checkbutton(top, text="虛線區分重疊軌跡", variable=self.dashed,
                        command=self.redraw).pack(side="left", padx=12)
        ttk.Label(top, textvariable=self.cursor_time, style="Muted.TLabel",
                  font=("Consolas", 9)).pack(side="left", padx=16)
        ttk.Button(top, text="匯出 CSV", command=self.export_csv).pack(side="right")

        self.figure = Figure(figsize=(8, 5.6), dpi=100, facecolor=C["panel"])
        self.axes = self.figure.add_subplot(111, facecolor=C["plot_bg"])
        self.canvas = FigureCanvasTkAgg(self.figure, master=parent)
        self.canvas.get_tk_widget().pack(fill="both", expand=True, pady=(6, 0))
        self.canvas.mpl_connect("motion_notify_event", self._on_hover)
        self.canvas.mpl_connect("axes_leave_event", self._on_hover_leave)
        # 縮放/平移/改視窗大小之後 blit 的背景快照就失效了。
        self.canvas.mpl_connect("draw_event", self._invalidate_hover_bg)
        self.canvas.mpl_connect("resize_event", self._invalidate_hover_bg)
        bar = NavigationToolbar2Tk(self.canvas, parent, pack_toolbar=False)
        _style_toolbar(bar)
        bar.pack(fill="x")

        self.stats = ttk.Treeview(parent, columns=("cursor", "min", "max", "mean", "pp"),
                                  show="tree headings", height=5, selectmode="none")
        self.stats.heading("#0", text="通道", anchor="w")
        self.stats.column("#0", width=230, anchor="w")
        # 游標讀值放這裡，不畫在圖上。原本用 matplotlib 的 text artist 顯示，
        # 實測 draw_artist(text) 要 10.7ms/次 (5 行文字 + 圓角框 + alpha 合成，
        # 每次都重做排版) —— 滑鼠每秒上百個事件時那是追不上的。
        # Treeview.set() 是微秒級，而且位置固定、不會遮住波形，
        # 與旁邊的 min/max/平均並排也更好對照。
        for column, title, width in (("cursor", "游標值", 110), ("min", "最小", 95),
                                     ("max", "最大", 95), ("mean", "平均", 95),
                                     ("pp", "峰對峰", 95)):
            self.stats.heading(column, text=title, anchor="e")
            self.stats.column(column, width=width, anchor="e")
        self.stats.pack(fill="x", pady=(6, 0))
        self.redraw()

    # -- 通道選擇 ----------------------------------------------------------
    def _toggle(self, name):
        if self.channel_vars[name].get():
            if len(self.selected) >= V.MAX_SCOPE_CHANNELS:
                self.channel_vars[name].set(False)
                messagebox.showinfo(
                    "通道已滿",
                    f"一次最多 {V.MAX_SCOPE_CHANNELS} 個通道。\n"
                    "這是 X2CScope 傳輸協定 (mchplnet) 的硬限制，不是本工具的設定。")
                return
            if name not in self.selected:
                self.selected.append(name)
        elif name in self.selected:
            self.selected.remove(name)
        self._refresh_selected()

    def _refresh_selected(self):
        self.chosen_title.set(f"已選 {len(self.selected)} / {V.MAX_SCOPE_CHANNELS}")
        for child in self.chosen_body.winfo_children():
            child.destroy()
        for index, name in enumerate(self.selected):
            colour = V.SCOPE_COLOURS[index % len(V.SCOPE_COLOURS)]
            row = ttk.Frame(self.chosen_body, style="Panel.TFrame")
            row.pack(fill="x", pady=1)
            tk.Frame(row, background=colour, width=14, height=10).pack(side="left", padx=(0, 6))
            ttk.Label(row, text=V.custom_channel_label(name), style="Row.TLabel",
                      font=("Consolas", 8)).pack(side="left")
        self.src_box.configure(values=list(self.selected))
        if self.selected and self.trigger_src.get() not in self.selected:
            self.trigger_src.set(self.selected[0])
        self._refresh_estimate()
        self._sync_trigger()

    def _refresh_estimate(self):
        try:
            factor = max(1, int(float(self.factor_var.get())))
        except ValueError:
            factor = 1
        # 用 ELF 給的真實寬度算，不要一律當成 2 byte：混進一顆 4 byte 的通道
        # (例如 FilteredSpeed 或 s_temp_controller_currentTempC) 會讓實際點數
        # 比估算少一大截，那個落差看起來就像傳輸掉了資料。
        sizes = {name: size for name, _type, size in self.elf_scalars}
        widths = [sizes.get(n, V.SCOPE_DEFAULT_SAMPLE_BYTES) or
                  V.SCOPE_DEFAULT_SAMPLE_BYTES for n in self.selected]
        dataset = sum(widths) or V.SCOPE_DEFAULT_SAMPLE_BYTES
        exact = bool(self.elf_scalars) and all(n in sizes for n in self.selected)
        points = V.X2C_BUFFER_BYTES // dataset
        dt_us = 1_000_000.0 / V.ISR_FREQ_HZ * factor
        window_ms = points * dt_us / 1000.0
        self.estimate.set(
            f"{'預計' if exact else '預估'} ~{points} 點/通道"
            f"   取樣間隔 {dt_us:.0f} µs\n"
            f"視窗長度 ~{window_ms:.1f} ms"
            f"   (緩衝區 {V.X2C_BUFFER_BYTES} byte / 每組 {dataset} byte)")

    def _refresh_level_hint(self):
        try:
            level = float(self.trigger_level.get())
        except ValueError:
            self.level_hint.set("")
            return
        kmh = self.scale.q15_to_kmh(level)
        self.level_hint.set(f"若為 Q15 速度域 ≈ {kmh:+.2f} km/h")

    def _sync_trigger(self):
        state = "normal" if (self.trigger_on.get() and self.selected) else "disabled"
        for widget in self.trigger_widgets:
            try:
                widget.configure(state="readonly" if isinstance(widget, ttk.Combobox)
                                 and state == "normal" else state)
            except tk.TclError:
                pass

    # -- ELF 自由挑選 ------------------------------------------------------
    def _on_channel_tab(self):
        """第一次切到「ELF 全域」才去解析符號表。"""
        try:
            current = self.channel_book.select()
        except tk.TclError:
            return
        if current != str(self.free_tab) or self.scalars_requested:
            return
        self.scalars_requested = True
        self.free_note.set("解析 ELF 符號表 … (約 1.5 秒)")
        # 不能在 Tk 執行緒跑 (畫面會凍住)，也不該排進 link worker (遙測會停)。
        load_elf_scalars_async(self.app.elf, self.app.events)

    def _refill_free(self):
        needle = self.filter_var.get().strip().lower()
        self.free_list.delete(0, "end")
        shown = 0
        for name, type_name, size in self.elf_scalars:
            if needle and needle not in name.lower():
                continue
            self.free_list.insert("end", f"{name}   [{type_name} {size}B]")
            shown += 1
            if shown >= 400:      # Listbox 塞滿 595 筆會變鈍，且沒人會滾到底
                break
        self.free_note.set(f"{shown} / {len(self.elf_scalars)} 個符號"
                           + ("（已截斷，請縮小搜尋）" if shown >= 400 else ""))

    def _add_free(self):
        indices = self.free_list.curselection()
        if not indices:
            return
        name = self.free_list.get(indices[0]).split("   [")[0]
        if name in self.channel_vars:
            self.channel_vars[name].set(True)
            self._toggle(name)
            return
        if len(self.selected) >= V.MAX_SCOPE_CHANNELS:
            messagebox.showinfo("通道已滿", f"一次最多 {V.MAX_SCOPE_CHANNELS} 個通道。")
            return
        self.selected.append(name)
        if name not in self.custom:
            self.custom.append(name)
            self._save_custom()
        self._refresh_selected()

    def _load_custom(self):
        try:
            return list(json.loads(V.SCOPE_CUSTOM_STORE.read_text("utf-8")))
        except (OSError, ValueError):
            return []

    def _save_custom(self):
        try:
            V.SCOPE_CUSTOM_STORE.write_text(
                json.dumps(self.custom, ensure_ascii=False, indent=2), "utf-8")
        except OSError as exc:
            self.state_text.set(f"無法保存自訂通道: {exc}")

    # -- 擷取 --------------------------------------------------------------
    def configure_scope(self):
        if self.app.link is None:
            messagebox.showwarning("未連線", "請先在主視窗連線。")
            return False
        if not self.selected:
            messagebox.showwarning("沒有通道", "請至少選一個通道。")
            return False
        try:
            factor = max(1, int(float(self.factor_var.get())))
            delay = int(float(self.trigger_delay.get()))
            level = float(self.trigger_level.get())
        except ValueError:
            messagebox.showerror("設定無效", "取樣分頻 / 位準 / 延遲必須是數字。")
            return False
        trigger = None
        if self.trigger_on.get():
            trigger = {
                "variable": self.trigger_src.get(),
                "level": level,
                "edge": dict(EDGES)[self.trigger_edge.get()],
                "delay": delay,
                "mode": dict(MODES)[self.trigger_mode.get()],
            }
        self.app.link.submit("scope_config", {
            "channels": list(self.selected),
            "sample_factor": factor,
            "trigger": trigger,
        })
        return True

    def start(self, continuous):
        if not self.configure_scope():
            return
        self.app.link.submit("scope_start", continuous)

    def abort(self):
        if self.app.link is not None:
            self.app.link.submit("scope_abort")

    # -- 事件 --------------------------------------------------------------
    def on_event(self, kind, payload):
        if kind == "scope_state":
            self.state_text.set(payload)
        elif kind == "scope_data":
            self.last_capture = payload
            self._request_redraw()
        elif kind == "elf_scalars":
            self.elf_scalars = payload or []
            self._refill_free()
            self._refresh_estimate()      # 現在知道真實寬度了，重算一次
        elif kind == "disconnected":
            self.state_text.set("主視窗已斷線")

    # -- 繪圖 --------------------------------------------------------------
    def _request_redraw(self):
        """把積壓的多筆擷取合併成一次重繪。

        一次 redraw() 要 ~27 ms (大部分花在 tight_layout() 的文字排版量測)。連續模式
        下 link 端可能在畫完前就送來下一筆 —— 原本是每來一筆畫一次，於是進料快過重繪
        時 event queue 會無上限累積，畫面永遠落後而且愈落愈多。

        只有最後一筆看得到，中間那些畫了也是白畫，所以積壓時直接跳過。
        """
        if self._redraw_pending:
            return
        self._redraw_pending = True
        self.after_idle(self._flush_redraw)

    def _flush_redraw(self):
        self._redraw_pending = False
        if self.alive:
            self.redraw()

    def redraw(self):
        kind, unit = next((k, u) for label, k, u in AXIS_MODES
                          if label == self.axis_mode.get())
        self.axes.clear()
        self.axes.set_facecolor(C["plot_bg"])
        for spine in self.axes.spines.values():
            spine.set_color(C["edge"])
        self.axes.tick_params(colors=C["muted"], labelsize=8)
        # 格線比邊框更淡：它是背景刻度，不該和軌跡爭視覺重量。
        self.axes.grid(True, color=C["grid"], linewidth=0.6)
        self.axes.set_xlabel("時間 (ms)", color=C["muted"], fontsize=8)
        self.axes.set_ylabel(unit, color=C["muted"], fontsize=8)

        for item in self.stats.get_children():
            self.stats.delete(item)
        self._clear_cursor_readout()

        if not self.last_capture:
            self.axes.text(0.5, 0.5, "尚未擷取", ha="center", va="center",
                           color=C["muted"], transform=self.axes.transAxes)
            self._make_hover_artists()
            self.canvas.draw_idle()
            return

        capture = self.last_capture
        dt_ms = capture["dt_us"] / 1000.0
        channels = capture["channels"]
        for index, (name, series) in enumerate(channels.items()):
            if not series:
                continue
            values = [V.chart_value(kind, v, self.scale) for v in series]
            times = [i * dt_ms for i in range(len(values))]
            colour = V.SCOPE_COLOURS[index % len(V.SCOPE_COLOURS)]
            # 疊圖時最後畫的那條會吃掉每一個像素，量測疊在命令上就會把命令整條藏起來
            # —— 而那正是最想看的情況。解法用**遞減線寬**而不是虛線：重疊時較粗的
            # 底層會從較細的上層兩側露出來，效果一樣，但在密集/雜訊大的軌跡上不會糊掉。
            # (虛線碰上 625 點的高頻振盪時，虛線間隔會與訊號互相干擾成一團。)
            # 真的需要虛線區分時可以勾「虛線」。
            kwargs = {"color": colour,
                      "linewidth": max(0.8, 1.9 - 0.32 * index),
                      "alpha": 0.9,
                      "label": V.custom_channel_label(name)}
            if self.dashed.get() and index > 0:
                kwargs["dashes"] = V.SCOPE_OVERLAY_DASHES[
                    (index - 1) % len(V.SCOPE_OVERLAY_DASHES)]
            self.axes.plot(times, values, **kwargs)

            finite = [v for v in values if v is not None]
            if finite:
                lo, hi = min(finite), max(finite)
                # iid 用變數名，這樣 _on_hover 可以直接 stats.set(name, "cursor", ...)
                # 而不必每次去搜尋是哪一列。
                self.stats.insert("", "end", iid=name,
                                  text=V.custom_channel_label(name),
                                  values=("", f"{lo:,.2f}", f"{hi:,.2f}",
                                          f"{sum(finite) / len(finite):,.2f}",
                                          f"{hi - lo:,.2f}"))

        trigger_index = capture.get("trigger_index")
        if trigger_index:
            self.axes.axvline(trigger_index * dt_ms, color=C["trigger"], linewidth=1.2,
                              dashes=(4, 3), label=f"觸發 @{trigger_index}")
        # loc="best" 而不是固定角落：波形的走向每次都不一樣，固定在右上角時
        # 一段上升的軌跡會整條被圖例蓋住 —— 而那通常就是要看的那一段。
        self.axes.legend(loc="best", fontsize=7, facecolor=C["panel"],
                         edgecolor=C["edge"], labelcolor=C["text"], framealpha=0.88)
        points = max((len(s) for s in channels.values()), default=0)
        # 把「拿到幾點 / 預期幾點」寫在標題上。兩者不一致代表傳輸掉了區塊，
        # 而掉了區塊的波形是錯位的 —— 這件事必須跟著圖一起被看到。
        # 不用 ⚠ 之類的符號：中文字型 (Microsoft JhengHei) 沒有那個字符，而 matplotlib
        # 不像 Tk 會逐字 fallback，會直接畫成一個方框。純文字 + 紅色標題一樣清楚。
        expected = capture.get("expected")
        short = (f"   [資料不完整 — 預期 {expected} 點]"
                 if expected and points < expected else "")
        self.axes.set_title(
            f"{len(channels)} 通道 × {points} 點{short}   "
            f"取樣 {capture['dt_us']:.0f} µs (分頻 {capture['sample_factor']})   "
            f"視窗 {points * dt_ms:.2f} ms",
            color=C["danger"] if short else C["text"], fontsize=9)
        # 必須在 axes.clear() 之後重建，否則游標讀值在第一次重畫後就再也不出現。
        self._make_hover_artists()
        self.figure.tight_layout()
        self.canvas.draw_idle()

    # -- 游標讀值 ----------------------------------------------------------
    def _invalidate_hover_bg(self, _event=None):
        """背景失效：圖被重畫或視窗被縮放之後，舊的背景快照就不能用了。

        _blit_hover() 自己也會呼叫 canvas.draw() 去拍背景，而那同樣會發出 draw_event
        —— 少了 _capturing_bg 這個旗標，剛拍好的背景會被自己的事件立刻作廢，
        於是每次 hover 都重拍一次 (等於 blitting 完全沒生效)。
        """
        if self._capturing_bg:
            return
        self._hover_bg = None
        self._hover_index = None
        self._hover_px_last = None

    def _make_hover_artists(self):
        """建立 (或在 axes.clear() 之後重建) 游標的十字線與文字框。

        必須在每次 redraw 之後重建：axes.clear() 會把所有 artist 移除，
        沿用舊物件的話它們已經脫離 axes，游標讀值就再也不會顯示。
        只建這兩個、之後只改資料 —— 每次 motion 事件都新增 artist 的話，
        滑一陣子圖上會累積幾百個殘留物件。
        """
        self._hover_line = self.axes.axvline(0, color=C["muted"], linewidth=0.8,
                                             visible=False, zorder=5)
        self._invalidate_hover_bg()

    def _clear_cursor_readout(self):
        self.cursor_time.set("")
        for item in self.stats.get_children():
            self.stats.set(item, "cursor", "")

    def _on_hover_leave(self, _event=None):
        # 已經是隱藏狀態就什麼都不做 —— 少了這個判斷，滑鼠在座標軸外的邊界移動會
        # 對每一個事件觸發一次重畫，純粹是白燒 CPU。
        if self._hover_line is None or not self._hover_line.get_visible():
            self._hover_index = None
            return
        self._hover_line.set_visible(False)
        self._hover_index = None
        self._hover_px_last = None
        self._clear_cursor_readout()
        self.canvas.draw_idle()
        self._hover_bg = None

    def _blit_hover(self):
        """只重畫十字線，不重畫整張圖。

        canvas.draw_idle() 在 4 通道 x 625 點下要 **60~70 ms**，而滑鼠移動每秒會產生
        上百個事件 —— 事件佇列會塞爆、Tk 的 drain() 被排擠、畫面反而停止更新。
        blitting 把背景存成點陣圖，之後每次只還原背景 + 畫那一條線。

        只 blit 十字線周圍的窄長條，不是整個 axes：TkAgg 的 blit 要把 ARGB 緩衝逐像素
        複製進 Tk PhotoImage，實測整個 axes (817x495 = 404k 像素) 要 3.0 ms，
        而 160px 寬的長條只要 0.85 ms。
        """
        if self._hover_bg is None:
            # 第一次 (或重畫/縮放之後) 需要一張乾淨的背景。artist 先隱藏才不會被
            # 拍進背景裡，否則十字線會殘留成鬼影。
            visible = self._hover_line.get_visible()
            self._hover_line.set_visible(False)
            self._capturing_bg = True
            try:
                self.canvas.draw()
                self._hover_bg = self.canvas.copy_from_bbox(self.axes.bbox)
            finally:
                self._capturing_bg = False
            self._hover_line.set_visible(visible)
        self.canvas.restore_region(self._hover_bg)
        self.axes.draw_artist(self._hover_line)
        self.canvas.blit(self._blit_region())

    def _blit_region(self):
        """只複製十字線移動過的那一小塊，而不是整個 axes。

        TkAgg 的 blit 把 ARGB 緩衝逐像素複製進 Tk PhotoImage —— 實測整個 axes
        (817x495 = 404k 像素) 要 3.0 ms，160px 寬的長條只要 0.85 ms。

        必須涵蓋**舊位置與新位置的聯集**：只複製新位置的話舊那條線不會被背景蓋掉，
        會在圖上留下一排殘影。滑鼠跳很遠時聯集會退化成接近整個 axes 的寬度，
        那時的成本就跟不最佳化一樣 —— 可以接受，因為那不是常見情況。
        """
        from matplotlib.transforms import Bbox

        bb = self.axes.bbox
        try:
            x_new = self.axes.transData.transform(
                (self._hover_line.get_xdata()[0], 0))[0]
        except Exception:  # noqa: BLE001 - 座標轉換失敗就退回整個 axes
            return bb
        pad = 1.5 * max(1.0, self._hover_line.get_linewidth()) + 4.0
        x_old = self._hover_px_last if self._hover_px_last is not None else x_new
        self._hover_px_last = x_new
        return Bbox.from_extents(max(bb.x0, min(x_old, x_new) - pad), bb.y0,
                                 min(bb.x1, max(x_old, x_new) + pad), bb.y1)

    def _on_hover(self, event):
        """在游標時間點畫十字線，並列出每個通道在那一點的值。

        取最接近的**取樣點**而不是游標的原始 x：擷取資料是離散的，顯示內插值會讓人
        以為那是量到的東西。索引由 dt 直接算出來，不用逐點搜尋。
        """
        if (self._hover_line is None or event.inaxes is not self.axes
                or not self.last_capture or event.xdata is None):
            self._on_hover_leave()
            return
        capture = self.last_capture
        dt_ms = capture["dt_us"] / 1000.0
        channels = capture["channels"]
        length = max((len(s) for s in channels.values()), default=0)
        if length < 1 or dt_ms <= 0:
            return
        index = max(0, min(length - 1, int(round(event.xdata / dt_ms))))
        # 索引沒變就不用重畫。滑鼠在同一個取樣點內移動幾個像素是最常見的情況。
        if index == self._hover_index and self._hover_line.get_visible():
            return
        self._hover_index = index
        kind, unit = next((k, u) for label, k, u in AXIS_MODES
                          if label == self.axis_mode.get())

        self.cursor_time.set(f"游標 t = {index * dt_ms:.3f} ms  (#{index})")
        for name, series in channels.items():
            if not self.stats.exists(name):
                continue
            if index >= len(series):
                self.stats.set(name, "cursor", "--")
                continue
            value = V.chart_value(kind, series[index], self.scale)
            self.stats.set(name, "cursor",
                           "--" if value is None else f"{value:+,.2f}")

        self._hover_line.set_xdata([index * dt_ms, index * dt_ms])
        self._hover_line.set_visible(True)
        self._blit_hover()

    # -- 匯出 --------------------------------------------------------------
    def export_csv(self):
        if not self.last_capture:
            messagebox.showinfo("沒有資料", "請先擷取一次。")
            return
        path = filedialog.asksaveasfilename(
            title="匯出擷取資料", defaultextension=".csv",
            filetypes=[("CSV", "*.csv")])
        if not path:
            return
        capture = self.last_capture
        channels = capture["channels"]
        names = list(channels)
        length = max((len(channels[n]) for n in names), default=0)
        dt_ms = capture["dt_us"] / 1000.0
        try:
            with open(path, "w", newline="", encoding="utf-8-sig") as handle:
                writer = csv.writer(handle)
                # 註解行讓 CSV 自己說明取樣條件 —— 沒有它，幾天後沒人記得這份
                # 資料是哪個分頻抓的，時間軸就無從還原。
                writer.writerow([f"# X2CScope 擷取  取樣間隔 {capture['dt_us']:.1f} us"
                                 f"  分頻 {capture['sample_factor']}"
                                 f"  觸發索引 {capture.get('trigger_index')}"])
                writer.writerow(["time_ms"] + names)
                for i in range(length):
                    writer.writerow([f"{i * dt_ms:.4f}"]
                                    + [channels[n][i] if i < len(channels[n]) else ""
                                       for n in names])
        except OSError as exc:
            messagebox.showerror("匯出失敗", str(exc))
            return
        self.state_text.set(f"已匯出 {length} 點 × {len(names)} 通道 → {path}")

    def on_close(self):
        self.alive = False
        # 關窗一定要中止擷取。連續模式會自己重新武裝，視窗沒了就沒人按得到「中止」——
        # 那會變成一個沒有 UI 的背景擷取一直吃頻寬，而主頁只看得到遙測變慢。
        #
        # 也是主頁捲動圖恢復的途徑：link 端擷取結束後會發 scope_active=False。
        try:
            self.abort()
        except Exception:  # noqa: BLE001 - 關窗流程不該因為通知失敗而卡住
            pass
        self.destroy()


def _style_toolbar(bar):
    """把 matplotlib 的 Tk 工具列配色對齊本視窗，而不是留在 OS 預設灰。

    matplotlib 會依按鈕背景的亮度決定圖示要用原色還是反白版
    (_backend_tk 的 _recolor_icon)，而判斷發生在按鈕**建立的那一刻**。
    白底主題下原本的黑色圖示剛好就是對的，所以這裡改完背景再請它重載一次圖示，
    純粹是為了讓兩者的判斷保持一致 —— 日後若改回深色主題，這一步就會自動讓
    圖示反白，不用再回來改這個函式。

    用到一個私有方法 (_set_image_for_button)，所以整段包在 try 裡：換 matplotlib
    版本時最壞的情況是圖示配色沒跟上，而不是視窗開不起來。
    """
    bar.configure(background=C["bg"], borderwidth=0)
    for child in bar.winfo_children():
        # foreground 必須在 _set_image_for_button 之前設好：反白後的圖示是用
        # 按鈕的 foreground 顏色重新著色的。
        try:
            child.configure(background=C["bg"], foreground=C["text"],
                            highlightbackground=C["bg"])
        except tk.TclError:
            pass          # 分隔線之類的 widget 不吃這些選項
        try:
            child.configure(disabledforeground=C["disabled"])
        except tk.TclError:
            pass          # Label 沒有這個選項
    for button in getattr(bar, "_buttons", {}).values():
        try:
            button.configure(background=C["bg"], activebackground=C["row"],
                             highlightbackground=C["bg"], borderwidth=0)
            bar._set_image_for_button(button)
        except Exception:  # noqa: BLE001 - 見上面的說明，退化成黑圖示可以接受
            pass
    bar.update()


class _Scroller(ttk.Frame):
    """精選通道有 45 條，裝不進一個畫面。"""

    def __init__(self, parent):
        super().__init__(parent)
        self.canvas = tk.Canvas(self, background=C["panel"], highlightthickness=0)
        bar = ttk.Scrollbar(self, orient="vertical", command=self.canvas.yview)
        self.body = ttk.Frame(self.canvas, style="Panel.TFrame")
        window = self.canvas.create_window((0, 0), window=self.body, anchor="nw")
        self.canvas.configure(yscrollcommand=bar.set)
        self.canvas.pack(side="left", fill="both", expand=True)
        bar.pack(side="right", fill="y")
        self.body.bind("<Configure>",
                       lambda _e: self.canvas.configure(scrollregion=self.canvas.bbox("all")))
        self.canvas.bind("<Configure>",
                         lambda e: self.canvas.itemconfigure(window, width=e.width))
        self.canvas.bind("<Enter>", lambda _e: self.canvas.bind_all(
            "<MouseWheel>", lambda e: self.canvas.yview_scroll(int(-e.delta / 120), "units")))
        self.canvas.bind("<Leave>", lambda _e: self.canvas.unbind_all("<MouseWheel>"))
