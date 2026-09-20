"""Reproduce the bounded WP-23.5 Hollow Twins encounter proof."""

import json
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
ENGINE = Path(r"C:\Program Files\UE_5.8\Engine\Binaries\Win64")
PROJECT = ROOT / "WYRMFALL.uproject"
COMMANDLET = ENGINE / "UnrealEditor-Cmd.exe"
EDITOR = ENGINE / "UnrealEditor.exe"
REPORT = ROOT / "Saved/Diagnostics/WP23_5_hollow_twins_slice_proof.json"


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
    if "--pie-only" not in sys.argv:
        run(COMMANDLET, ROOT / "tools/unreal/import_wp23_5_gloaming_environment.py")
        run(COMMANDLET, ROOT / "tools/unreal/import_wp23_5_malvaine.py")
        run(COMMANDLET, ROOT / "tools/unreal/import_wp23_5_hollow_twins.py")
        run(COMMANDLET, ROOT / "tools/unreal/compose_wp23_5_gloaming_foundation.py")
    run(EDITOR, ROOT / "tools/unreal/verify_wp23_5_hollow_twins_slice.py", windowed=True)
    proof = json.loads(REPORT.read_text(encoding="utf-8"))
except (OSError, RuntimeError, subprocess.TimeoutExpired, json.JSONDecodeError) as exc:
    print(f"WP-23.5 Hollow Twins slice failed: {exc}", file=sys.stderr)
    sys.exit(1)
if proof.get("status") != "PASS_AUTOMATED_REQUIRES_MANUAL_VISUAL_REVIEW":
    print(f"WP-23.5 Hollow Twins proof status: {proof.get('status')}", file=sys.stderr)
    sys.exit(1)
sys.exit(0)
