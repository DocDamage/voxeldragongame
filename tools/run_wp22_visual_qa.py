import json
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
SCRIPT = (ROOT / "tools/unreal/capture_wp22_visual_qa.py").as_posix()
REPORT = ROOT / "Saved/Diagnostics/WP22_visual_qa.json"
EDITOR = r"C:\Program Files\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe"
command = [
    EDITOR, str(ROOT / "WYRMFALL.uproject"), "-unattended", "-nop4", "-nosplash", "-nosound",
    f"-LocalDataCachePath={(ROOT / 'Saved/DerivedDataCache').as_posix()}",
    "-windowed", "-ResX=1280", "-ResY=720",
    f"-ExecCmds=py exec(open('{SCRIPT}').read())", "-stdout", "-FullStdOutLogOutput",
]
process = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, timeout=240)
sys.stdout.write(process.stdout)
try:
    result = json.loads(REPORT.read_text(encoding="utf-8"))
except (OSError, json.JSONDecodeError) as exc:
    print(f"WP-22 visual QA report missing or unreadable: {exc}", file=sys.stderr)
    sys.exit(1)
if process.returncode != 0 or result.get("capture_status") != "PASS":
    print(
        f"WP-22 visual QA capture failed: process={process.returncode}, "
        f"capture={result.get('capture_status')}",
        file=sys.stderr,
    )
    sys.exit(1)
sys.exit(0)
