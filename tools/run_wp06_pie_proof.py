import json
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
SCRIPT = (ROOT / "tools/unreal/verify_wp06_progression_proof.py").as_posix()
REPORT = ROOT / "Saved/Diagnostics/WP06_progression_proof.json"
EDITOR = r"C:\Program Files\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe"
PROJECT = str(ROOT / "WYRMFALL.uproject")
command = [
    EDITOR, PROJECT, "-unattended", "-nop4", "-nosplash", "-nosound",
    "-windowed", "-ResX=1280", "-ResY=720",
    f"-ExecCmds=py exec(open('{SCRIPT}').read())",
    "-stdout", "-FullStdOutLogOutput",
]

print(f"Launching WP-06 PIE verification probe with script: {SCRIPT}")
process = subprocess.Popen(
    command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
    text=True, bufsize=1)
for line in process.stdout:
    sys.stdout.write(line)
process.wait()

try:
    result = json.loads(REPORT.read_text(encoding="utf-8"))
except (OSError, json.JSONDecodeError) as exc:
    print(f"WP-06 PIE report missing or unreadable: {exc}", file=sys.stderr)
    sys.exit(1)
if process.returncode != 0 or result.get("status") != "PASS":
    print(
        f"WP-06 PIE proof failed: process={process.returncode}, "
        f"report={result.get('status')}", file=sys.stderr)
    sys.exit(1)
sys.exit(0)
