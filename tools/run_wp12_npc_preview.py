"""Run the scoped rendered WP-12 NPC developer fixture and verify its receipt."""

import argparse
import json
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
SCRIPT = (ROOT / "tools/unreal/preview_wp12_region01_npcs.py").as_posix()
NORMALIZED_SCRIPT = (ROOT / "tools/unreal/preview_wp12_region01_normalized_npcs.py").as_posix()
EDITOR = r"C:\Program Files\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe"
PROJECT = str(ROOT / "WYRMFALL.uproject")
EXPECTED_STATUS = "CAPTURED_REQUIRES_VISUAL_REVIEW"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--normalized",
        action="store_true",
        help="render the verified single-root normalized supplied FBX intake",
    )
    args = parser.parse_args()
    report = ROOT / "Saved/Diagnostics" / (
        "WP12Region01NormalizedNpcPreview/report.json"
        if args.normalized
        else "WP12Region01NpcPreview/report.json"
    )
    script = NORMALIZED_SCRIPT if args.normalized else SCRIPT
    # Keep the console invocation to the known-good single ``py exec`` form.
    # The normalized wrapper supplies its own Python globals after the editor
    # console has finished parsing this command.
    exec_cmd = "py exec(open('{}').read())".format(script)
    command = [
        EDITOR,
        PROJECT,
        "/Engine/Maps/Entry",
        "-unattended",
        "-nop4",
        "-nosplash",
        "-nosound",
        # Keep the unsaved diagnostic fixture isolated when the user already
        # has the project open in an editor window.
        "-NoMutex",
        "-windowed",
        "-ResX=1280",
        "-ResY=720",
        "-ExecCmds=" + exec_cmd,
        "-stdout",
        "-FullStdOutLogOutput",
    ]
    print("Launching WP-12 rendered {} NPC intake fixture: {}".format(
        "normalized" if args.normalized else "direct", script
    ))
    try:
        process = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, timeout=120)
    except subprocess.TimeoutExpired as exc:
        if exc.stdout:
            output = exc.stdout if isinstance(exc.stdout, str) else exc.stdout.decode(errors="replace")
            print(output[-12000:])
        print("WP-12 rendered NPC fixture timed out after 120 seconds.", file=sys.stderr)
        return 1

    try:
        result = json.loads(report.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        print("WP-12 rendered NPC fixture receipt missing or unreadable: {}".format(exc), file=sys.stderr)
        print(process.stdout[-12000:])
        return 1

    if process.returncode != 0 or result.get("status") != EXPECTED_STATUS:
        print(
            "WP-12 rendered NPC fixture failed: process={}, report={}".format(
                process.returncode, result.get("status")
            ),
            file=sys.stderr,
        )
        print(process.stdout[-12000:])
        return 1

    print("WP-12 rendered NPC fixture passed: {} captures.".format(len(result.get("captures", []))))
    return 0


if __name__ == "__main__":
    sys.exit(main())
