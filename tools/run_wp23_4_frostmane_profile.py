"""Launch and validate the focused WP-23.4 Frostmane profile PIE proof."""

import json
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
SCRIPT = (ROOT / "tools/unreal/verify_wp23_4_frostmane_profile.py").as_posix()
REPORT = ROOT / "Saved/Diagnostics/WP23_4_frostmane_profile_proof.json"
EDITOR = r"C:\Program Files\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe"
COMMAND = [
    EDITOR, str(ROOT / "WYRMFALL.uproject"), "-unattended", "-nop4",
    "-nosplash", "-nosound", "-windowed", "-ResX=1280", "-ResY=720",
    f"-ExecCmds=py exec(open('{SCRIPT}').read())",
    "-stdout", "-FullStdOutLogOutput",
]

try:
    process = subprocess.run(
        COMMAND, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
        text=True, timeout=180)
except subprocess.TimeoutExpired as exc:
    if exc.stdout:
        sys.stdout.write(exc.stdout if isinstance(exc.stdout, str) else exc.stdout.decode(errors="replace"))
    print("Frostmane PIE proof timed out after 180 seconds", file=sys.stderr)
    raise SystemExit(1)

sys.stdout.write(process.stdout)
try:
    result = json.loads(REPORT.read_text(encoding="utf-8"))
except (OSError, json.JSONDecodeError) as exc:
    print(f"Frostmane PIE report missing or unreadable: {exc}", file=sys.stderr)
    raise SystemExit(1)

if process.returncode != 0 or result.get("status") != "PASS":
    print(
        f"Frostmane PIE proof failed: process={process.returncode}, "
        f"report={result.get('status')}", file=sys.stderr)
    raise SystemExit(1)
