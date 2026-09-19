"""Run the bounded WP-23.5 Gloaming editor content fixture."""

import json
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
SCRIPT = (ROOT / "tools/unreal/verify_wp23_5_gloaming_content_fixture.py").as_posix()
REPORT = ROOT / "Saved/Diagnostics/WP23_5_gloaming_content_fixture.json"
EDITOR = r"C:\Program Files\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe"
COMMAND = [
    EDITOR, str(ROOT / "WYRMFALL.uproject"), "-unattended", "-nop4", "-nosplash", "-nosound",
    "-windowed", "-ResX=1280", "-ResY=720",
    f"-ExecCmds=py exec(open('{SCRIPT}').read())", "-stdout", "-FullStdOutLogOutput",
]

try:
    process = subprocess.run(COMMAND, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, timeout=300)
except subprocess.TimeoutExpired as exc:
    if exc.stdout:
        sys.stdout.write(exc.stdout if isinstance(exc.stdout, str) else exc.stdout.decode(errors="replace"))
    print("WP-23.5 Gloaming content fixture timed out after 300 seconds.", file=sys.stderr)
    sys.exit(1)

sys.stdout.write(process.stdout)
try:
    result = json.loads(REPORT.read_text(encoding="utf-8"))
except (OSError, json.JSONDecodeError) as exc:
    print(f"WP-23.5 Gloaming content fixture report missing or unreadable: {exc}", file=sys.stderr)
    sys.exit(1)

accepted = "PASS_AUTOMATED_REQUIRES_MANUAL_VISUAL_REVIEW"
sys.exit(0 if process.returncode == 0 and result.get("status") == accepted else 1)
