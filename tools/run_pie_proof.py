import subprocess
import sys
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SCRIPT = (ROOT / "tools/unreal/verify_wp01_terrain_proof.py").as_posix()
EDITOR = r"C:\Program Files\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe"
PROJECT = str(ROOT / "WYRMFALL.uproject")

cmd = [
    EDITOR,
    PROJECT,
    "-unattended",
    "-nop4",
    "-nosplash",
    "-nosound",
    "-windowed",
    "-ResX=1280",
    "-ResY=720",
    f"-ExecCmds=py exec(open('{SCRIPT}').read())",
    "-stdout",
    "-FullStdOutLogOutput"
]

print(f"Launching PIE verification probe with script: {SCRIPT}")
proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, bufsize=1)
for line in proc.stdout:
    sys.stdout.write(line)
proc.wait()
report_path = ROOT / "Saved/Diagnostics/WP01_terrain_provider_proof.json"
try:
    report = json.loads(report_path.read_text(encoding="utf-8"))
except (OSError, json.JSONDecodeError) as exc:
    print(f"PIE proof report missing or unreadable: {exc}", file=sys.stderr)
    sys.exit(1)

if proc.returncode != 0 or report.get("status") != "PASS":
    print(f"PIE proof failed: process={proc.returncode}, report={report.get('status')}", file=sys.stderr)
    sys.exit(1)
sys.exit(0)
