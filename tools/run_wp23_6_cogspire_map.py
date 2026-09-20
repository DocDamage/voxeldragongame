"""Compose and verify the saved WP-23.6 Cogspire Harbor map foundation."""

import json
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
ENGINE = Path(r"C:\Program Files\UE_5.8\Engine\Binaries\Win64")
PROJECT = ROOT / "WYRMFALL.uproject"
COMMANDLET = ENGINE / "UnrealEditor-Cmd.exe"
EDITOR = ENGINE / "UnrealEditor.exe"
REPORT = ROOT / "Saved/Diagnostics/WP23_6_cogspire_map_proof.json"


def run(executable, script, windowed=False):
    command = [str(executable), str(PROJECT), "-unattended", "-nop4", "-nosplash", "-nosound"]
    if windowed:
        command.extend(["-windowed", "-ResX=1280", "-ResY=720"])
        command.append(f"-ExecCmds=py exec(open('{script.as_posix()}').read())")
    else:
        command.append(f"-ExecutePythonScript={script.as_posix()}")
    command.extend(["-stdout", "-FullStdOutLogOutput"])
    result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, timeout=300)
    sys.stdout.write(result.stdout)
    if result.returncode or "LogPython: Error:" in result.stdout or "Python script executed with errors" in result.stdout:
        raise RuntimeError(f"Unreal step failed ({result.returncode}): {script.name}")


try:
    run(COMMANDLET, ROOT / "tools/unreal/compose_wp23_6_cogspire_harbor.py")
    run(EDITOR, ROOT / "tools/unreal/verify_wp23_6_cogspire_map.py", windowed=True)
    proof = json.loads(REPORT.read_text(encoding="utf-8"))
except (OSError, RuntimeError, subprocess.TimeoutExpired, json.JSONDecodeError) as exc:
    print(f"WP-23.6 Cogspire map foundation failed: {exc}", file=sys.stderr)
    raise SystemExit(1)
if proof.get("status") != "PASS":
    print(f"WP-23.6 Cogspire map proof status: {proof.get('status')}", file=sys.stderr)
    raise SystemExit(1)
