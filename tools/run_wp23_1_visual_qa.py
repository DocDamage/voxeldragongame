import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SCRIPT = (ROOT / "tools/unreal/capture_wp23_1_visual_qa.py").as_posix()
REPORT = ROOT / "Saved/Diagnostics/WP23_1_visual_qa.json"
EDITOR = r"C:\Program Files\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe"
command = [EDITOR, str(ROOT / "WYRMFALL.uproject"), "-unattended", "-nop4", "-nosplash", "-nosound",
           "-windowed", "-ResX=1280", "-ResY=720",
           f"-ExecCmds=py exec(open('{SCRIPT}').read())", "-stdout", "-FullStdOutLogOutput"]
try:
    process = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, timeout=300)
except subprocess.TimeoutExpired as exc:
    if exc.stdout:
        sys.stdout.write(exc.stdout if isinstance(exc.stdout, str) else exc.stdout.decode(errors="replace"))
    print("WP-23.1 visual capture timed out after 300 seconds.", file=sys.stderr)
    sys.exit(1)
sys.stdout.write(process.stdout)
try:
    result = json.loads(REPORT.read_text(encoding="utf-8"))
except (OSError, json.JSONDecodeError) as exc:
    print(f"WP-23.1 visual report missing or unreadable: {exc}", file=sys.stderr)
    sys.exit(1)
sys.exit(0 if process.returncode == 0 and result.get("capture_status") == "PASS" else 1)
