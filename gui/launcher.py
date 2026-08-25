"""LWFDSPC 主機端 GUI 啟動器 —— 先檢查環境，再開 GUI。

給不熟 Python 的使用者用：run_gui.bat 會呼叫這一支。它把「環境不對」與「工具有 bug」
分開來，免得使用者看到一串 traceback 卻不知道其實只是少裝了一個套件。

本檔刻意寫得比其他檔案保守：
  * 不用 f-string，改用 .format()  —— 這樣連 Python 3.5 都還能**解析**它，
    才有機會印出「你的版本太舊」而不是丟一個 SyntaxError。
  * 檢查相依套件**之前**不匯入任何第三方模組。
  * 只匯入標準庫。

Component: HOST TOOLING
"""

import os
import subprocess
import sys

# pyx2cscope / mchplnet 的 metadata 寫死 Requires-Python: >=3.10,<3.15。
# 本工具自己的程式碼只用到 3.7 等級的語法，所以這個範圍完全來自相依套件。
MIN_VERSION = (3, 10)
MAX_EXCLUSIVE = (3, 15)

# (import 名稱, pip 名稱)。serial 由 pyx2cscope 帶進來，但分開檢查才能指出是哪一個缺。
REQUIRED = (
    ("pyx2cscope", "pyx2cscope"),
    ("mchplnet", "pyx2cscope"),
    ("matplotlib", "matplotlib"),
    ("serial", "pyserial"),
)

HERE = os.path.dirname(os.path.abspath(__file__))
DOWNLOAD_URL = "https://www.python.org/downloads/windows/"


def force_utf8_console():
    """Windows 主控台預設 cp950/cp1252，本檔輸出全是中文，不轉會直接當掉。"""
    for stream in (sys.stdout, sys.stderr):
        try:
            stream.reconfigure(encoding="utf-8")
        except Exception:      # noqa: BLE001 - 舊版沒有 reconfigure，就只能算了
            pass


def fail(*lines):
    print("")
    print("  [無法啟動]")
    for line in lines:
        print("  " + line)
    print("")
    return 1


def check_version():
    current = sys.version_info[:3]
    text = "{}.{}.{}".format(*current)
    if current[:2] < MIN_VERSION:
        return fail(
            "Python 版本太舊: {} (需要 {}.{} 以上)".format(
                text, MIN_VERSION[0], MIN_VERSION[1]),
            "",
            "這是 pyx2cscope 的硬限制 (Requires-Python: >=3.10,<3.15)，",
            "不是本工具的限制 —— pip 在舊版上會直接拒絕安裝。",
            "",
            "請安裝 Python 3.13 或 3.14 (64-bit):",
            "  " + DOWNLOAD_URL,
            "安裝時務必勾選 tcl/tk and IDLE。")
    if current[:2] >= MAX_EXCLUSIVE:
        return fail(
            "Python 版本太新: {} (需要低於 {}.{})".format(
                text, MAX_EXCLUSIVE[0], MAX_EXCLUSIVE[1]),
            "",
            "pyx2cscope 目前宣告 Requires-Python: >=3.10,<3.15，",
            "在這個版本上 pip 會拒絕安裝它。",
            "",
            "請另外安裝 Python 3.13 或 3.14 (可以與現有版本並存):",
            "  " + DOWNLOAD_URL)
    return 0


def check_tkinter():
    try:
        import tkinter                      # noqa: F401
    except Exception as exc:                # noqa: BLE001
        return fail(
            "這個 Python 沒有 tkinter: {}".format(exc),
            "",
            "GUI 需要 tkinter。沒有它的散布版有:",
            "  * python.org 安裝時未勾選 tcl/tk and IDLE",
            "  * embeddable zip 版",
            "  * 部分 conda / 精簡環境",
            "",
            "請用 python.org 的安裝檔重裝並勾選 tcl/tk and IDLE:",
            "  " + DOWNLOAD_URL)
    return 0


def missing_packages():
    missing = []
    for module, package in REQUIRED:
        try:
            __import__(module)
        except ImportError:
                # 只在這裡收集，不即刻報錯 —— 一次列出全部要裝的比一個一個試好。
            if package not in missing:
                missing.append(package)
    return missing


def install(packages):
    requirements = os.path.join(HERE, "requirements.txt")
    if os.path.isfile(requirements):
        command = [sys.executable, "-m", "pip", "install", "-r", requirements]
    else:
        command = [sys.executable, "-m", "pip", "install"] + list(packages)
    print("")
    print("  執行: " + " ".join(command))
    print("")
    return subprocess.call(command)


def ensure_packages():
    missing = missing_packages()
    if not missing:
        return 0
    print("")
    print("  缺少 {} 個套件: {}".format(len(missing), ", ".join(missing)))
    if not sys.stdin or not sys.stdin.isatty():
        # 非互動環境 (排程 / CI) 不要卡在 input() 等一個永遠不會來的回答。
        return fail("非互動模式，不自動安裝。請先執行:",
                    "  {} -m pip install -r gui/requirements.txt".format(sys.executable))
    try:
        answer = input("  現在自動安裝? [Y/n] ").strip().lower()
    except (EOFError, KeyboardInterrupt):
        print("")
        return 1
    if answer not in ("", "y", "yes"):
        return fail("已取消。手動安裝指令:",
                    "  {} -m pip install -r gui/requirements.txt".format(sys.executable))
    if install(missing) != 0:
        return fail("pip 安裝失敗。常見原因: 沒有網路、或公司 proxy 擋住 PyPI。",
                    "可改用離線安裝: pip install --no-index --find-links <資料夾> ...")
    still = missing_packages()
    if still:
        return fail("安裝完成但仍匯入不到: {}".format(", ".join(still)),
                    "可能裝到了另一個 Python。目前這一支是:",
                    "  " + sys.executable)
    print("  安裝完成。")
    return 0


def main():
    force_utf8_console()
    print("")
    print("  LWFDSPC 主機端診斷 GUI")
    print("  Python {}.{}.{}  ({})".format(
        sys.version_info[0], sys.version_info[1], sys.version_info[2], sys.executable))

    for step in (check_version, check_tkinter, ensure_packages):
        code = step()
        if code:
            return code

    # 到這裡環境確定沒問題，再匯入 GUI。放在檢查之後才匯入，是為了讓「少裝套件」
    # 顯示成一句話，而不是一串 ImportError traceback。
    if HERE not in sys.path:
        sys.path.insert(0, HERE)
    import motor_gui

    # 讓 argparse 的 usage / 錯誤訊息印出使用者**實際打的**那個名字。
    # 不改的話參數打錯會顯示 "usage: launcher.py ..."，而使用者從來沒打過這個字。
    invoked_as = os.environ.get("LWFDSPC_INVOKED_AS")
    if invoked_as:
        sys.argv[0] = invoked_as

    # --help 只是印說明就結束，前面加一句「啟動 GUI」純粹是噪音。
    if not ({"-h", "--help"} & set(sys.argv[1:])):
        print("  啟動 GUI …")
    print("")
    motor_gui.main()
    return 0


if __name__ == "__main__":
    sys.exit(main())
