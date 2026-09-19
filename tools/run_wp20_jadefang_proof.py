import json
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
SCRIPT = (ROOT / "tools/unreal/verify_wp20_jadefang_proof.py").as_posix()
REPORT = ROOT / "Saved/Diagnostics/WP20_jadefang_proof.json"
EDITOR = r"C:\Program Files\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe"
PROJECT = str(ROOT / "WYRMFALL.uproject")
command = [
    EDITOR, PROJECT, "-unattended", "-nop4", "-nosplash", "-nosound",
    "-windowed", "-ResX=1280", "-ResY=720",
    f"-ExecCmds=py exec(open('{SCRIPT}').read())",
    "-stdout", "-FullStdOutLogOutput",
]

print(f"Launching WP-20 Jadefang PIE verification probe with script: {SCRIPT}")
try:
    process = subprocess.run(
        command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
        text=True, timeout=180)
except subprocess.TimeoutExpired as exc:
    if exc.stdout:
        sys.stdout.write(exc.stdout if isinstance(exc.stdout, str) else exc.stdout.decode(errors="replace"))
    print("WP-20 PIE proof timed out after 180 seconds.", file=sys.stderr)
    sys.exit(1)
sys.stdout.write(process.stdout)

try:
    result = json.loads(REPORT.read_text(encoding="utf-8"))
except (OSError, json.JSONDecodeError) as exc:
    print(f"WP-20 PIE report missing or unreadable: {exc}", file=sys.stderr)
    sys.exit(1)
if process.returncode != 0 or result.get("status") not in ("PASS", "ALL_PASSED"):
    print(
        f"WP-20 PIE proof failed: process={process.returncode}, "
        f"report={result.get('status')}", file=sys.stderr)
    sys.exit(1)
sys.exit(0)
