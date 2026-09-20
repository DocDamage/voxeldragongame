"""Reproduce the bounded WP-23.6 Cogspire arrival observation proof."""

import json
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
REPORT = ROOT / "Saved/Diagnostics/WP23_6_cogspire_arrival_proof.json"
SCRIPT = ROOT / "tools/unreal/verify_wp23_6_cogspire_arrival.py"
command = [
    r"C:\Program Files\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe",
    str(ROOT / "WYRMFALL.uproject"), "-unattended", "-nop4", "-nosplash", "-nosound",
    "-windowed", "-ResX=1280", "-ResY=720",
    f"-ExecCmds=py exec(open('{SCRIPT.as_posix()}').read())",
    "-stdout", "-FullStdOutLogOutput",
]
try:
    result = subprocess.run(
        command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, timeout=300)
    sys.stdout.write(result.stdout)
    if result.returncode or "LogPython: Error:" in result.stdout or "Python script executed with errors" in result.stdout:
        raise RuntimeError(f"Unreal PIE step failed ({result.returncode})")
    proof = json.loads(REPORT.read_text(encoding="utf-8"))
except (OSError, RuntimeError, subprocess.TimeoutExpired, json.JSONDecodeError) as exc:
    print(f"WP-23.6 Cogspire arrival slice failed: {exc}", file=sys.stderr)
    raise SystemExit(1)
if proof.get("status") != "PASS":
    print(f"WP-23.6 Cogspire arrival proof status: {proof.get('status')}", file=sys.stderr)
    raise SystemExit(1)
