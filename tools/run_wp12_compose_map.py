import json
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parent.parent
SCRIPT = (ROOT / "tools/unreal/compose_wp12_region01_map.py").as_posix()
RECEIPT = ROOT / "Saved/Diagnostics/WP12_region01_map_composition.json"
EDITOR = r"C:\Program Files\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe"
PROJECT = str(ROOT / "WYRMFALL.uproject")

command = [
    EDITOR, PROJECT, "-unattended", "-nop4", "-nosplash", "-nosound",
    "-windowed", "-ResX=1280", "-ResY=720",
    f"-ExecCmds=py exec(open('{SCRIPT}').read()); quit",
    "-stdout", "-FullStdOutLogOutput",
]

print(f"Launching Region 01 map composition with script: {SCRIPT}")
try:
    process = subprocess.run(
        command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
        text=True, timeout=180)
except subprocess.TimeoutExpired as exc:
    if exc.stdout:
        sys.stdout.write(exc.stdout if isinstance(exc.stdout, str) else exc.stdout.decode(errors="replace"))
    print("Region 01 map composition timed out after 180 seconds.", file=sys.stderr)
    sys.exit(1)

sys.stdout.write(process.stdout)

try:
    result = json.loads(RECEIPT.read_text(encoding="utf-8"))
except (OSError, json.JSONDecodeError) as exc:
    print(f"Region 01 map composition receipt missing or unreadable: {exc}", file=sys.stderr)
    sys.exit(1)

if process.returncode != 0 or result.get("status") != "PASS_REGION01_MAP_COMPOSED":
    print(
        f"Region 01 map composition failed: process={process.returncode}, "
        f"status={result.get('status')}", file=sys.stderr)
    sys.exit(1)

print("Region 01 map composition succeeded.")
sys.exit(0)
