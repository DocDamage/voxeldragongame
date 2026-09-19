import json
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
SCRIPT = (ROOT / "tools/unreal/verify_wp22_jade_peaks_proof.py").as_posix()
REPORT = ROOT / "Saved/Diagnostics/WP22_jade_peaks_proof.json"
EDITOR = r"C:\Program Files\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe"
command = [
    EDITOR, str(ROOT / "WYRMFALL.uproject"), "-unattended", "-nop4", "-nosplash", "-nosound",
    "-windowed", "-ResX=1280", "-ResY=720",
    f"-ExecCmds=py exec(open('{SCRIPT}').read())", "-stdout", "-FullStdOutLogOutput",
]
print(f"Launching WP-22 Jade Peaks PIE verification probe with script: {SCRIPT}")
try:
    process = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, timeout=300)
except subprocess.TimeoutExpired as exc:
    if exc.stdout:
        sys.stdout.write(exc.stdout if isinstance(exc.stdout, str) else exc.stdout.decode(errors="replace"))
    print("WP-22 PIE proof timed out after 300 seconds.", file=sys.stderr)
    sys.exit(1)
sys.stdout.write(process.stdout)
try:
    result = json.loads(REPORT.read_text(encoding="utf-8"))
except (OSError, json.JSONDecodeError) as exc:
    print(f"WP-22 PIE report missing or unreadable: {exc}", file=sys.stderr)
    sys.exit(1)
if process.returncode != 0 or result.get("status") != "PASS":
    print(f"WP-22 PIE proof failed: process={process.returncode}, report={result.get('status')}", file=sys.stderr)
    sys.exit(1)
sys.exit(0)
