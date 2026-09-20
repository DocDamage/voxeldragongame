"""Launch and validate the bounded WP-23.6 Cogspire Harbor foundation fixture."""

import json
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
SCRIPT = (ROOT / "tools/unreal/verify_wp23_6_harbor_foundation.py").as_posix()
REPORT = ROOT / "Saved/Diagnostics/WP23_6_harbor_foundation.json"
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
        text=True, timeout=300)
except subprocess.TimeoutExpired as exc:
    if exc.stdout:
        sys.stdout.write(exc.stdout if isinstance(exc.stdout, str) else exc.stdout.decode(errors="replace"))
    print("Cogspire Harbor foundation fixture timed out after 300 seconds", file=sys.stderr)
    raise SystemExit(1)
sys.stdout.write(process.stdout)
try:
    result = json.loads(REPORT.read_text(encoding="utf-8"))
except (OSError, json.JSONDecodeError) as exc:
    print(f"Cogspire Harbor foundation report missing or unreadable: {exc}", file=sys.stderr)
    raise SystemExit(1)

if process.returncode != 0 or result.get("status") != "PASS":
    print(
        f"Cogspire Harbor foundation failed: process={process.returncode}, "
        f"status={result.get('status')}, error={result.get('error')}",
        file=sys.stderr)
    raise SystemExit(1)
