"""Bounded subprocess execution with per-attempt logs and honest failure states."""
from __future__ import annotations

from collections import deque
from datetime import datetime, timezone
import os
from pathlib import Path
import signal
import subprocess
import time
import uuid

from wyrm_support import SetupError, digest, write_json


def process_command(command: list[str], windows: bool | None = None):
    """Return an executable argv (or explicit cmd command line on Windows).

    Batch files have a shell boundary. Reject expansion/metacharacters rather
    than interpolating an unsafe path. Every batch argument is quoted, including
    paths containing spaces and parentheses. No engine commands come from logs.
    """
    if not command or any(not isinstance(a, str) or not a for a in command):
        raise SetupError("Process command must contain nonempty string arguments")
    windows = os.name == "nt" if windows is None else windows
    suffix = Path(command[0]).suffix.lower()
    if windows and suffix in {".bat", ".cmd"}:
        if any(any(c in arg for c in '%!^&|<>\r\n"') for arg in command):
            raise SetupError("Windows batch arguments contain shell expansion characters; use a safe local path")
        shell = os.environ.get("COMSPEC", "cmd.exe")
        if '"' in shell or '\n' in shell:
            raise SetupError("Invalid COMSPEC")
        quoted = " ".join('"' + arg + '"' for arg in command)
        return f'"{shell}" /d /s /c "{quoted}"'
    if not windows and suffix == ".sh":
        return ["bash", *command]
    return command


def terminate_tree(proc: subprocess.Popen) -> str:
    """Do not leave UBT/editor grandchildren running after a timed-out wrapper."""
    if os.name == "nt":
        try:
            result = subprocess.run(["taskkill", "/PID", str(proc.pid), "/T", "/F"],
                                    capture_output=True, timeout=15, check=False)
            note = "TASKKILL_TREE_REQUESTED" if result.returncode == 0 else "TASKKILL_FAILED_INSPECT_PROCESSES"
        except (OSError, subprocess.TimeoutExpired):
            note = "TASKKILL_FAILED_INSPECT_PROCESSES"
        if proc.poll() is None:
            proc.kill()
    else:
        try:
            os.killpg(proc.pid, signal.SIGKILL)
            note = "PROCESS_GROUP_KILLED"
        except ProcessLookupError:
            note = "PROCESS_GROUP_ALREADY_EXITED"
    try:
        proc.wait(timeout=15)
    except subprocess.TimeoutExpired:
        return note + "; ROOT_PROCESS_STILL_RUNNING"
    return note


def execute_logged(command: list[str], root: Path, label: str, timeout: int) -> dict:
    """Record an attempt; exit 0 means only this command exited successfully."""
    if type(timeout) is not int or timeout < 1:
        raise SetupError("Timeout must be a positive integer")
    if not label or any(c not in "abcdefghijklmnopqrstuvwxyz0123456789-_" for c in label):
        raise SetupError("Unsafe log label")
    actual = process_command(command)
    run_id = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ") + "_" + uuid.uuid4().hex[:12]
    folder = root / "Saved/ScaffoldLogs"
    folder.mkdir(parents=True, exist_ok=True)
    log = folder / f"{run_id}_{label}.log"
    result = {"kind": "process_attempt_not_gameplay_proof", "run_id": run_id, "label": label,
              "command": command, "state": "NOT_STARTED", "exit_code": None,
              "log": log.relative_to(root).as_posix(), "started_utc": datetime.now(timezone.utc).isoformat()}
    started = time.monotonic()
    print("Executing:", subprocess.list2cmdline(command), flush=True)
    print("Log:", log, flush=True)
    proc = None
    try:
        with log.open("x", encoding="utf-8") as stream:
            stream.write(subprocess.list2cmdline(command) + "\n")
            stream.flush()
            proc = subprocess.Popen(actual, cwd=root, stdout=stream, stderr=subprocess.STDOUT,
                                    start_new_session=os.name != "nt")
            result["state"] = "RUNNING"
            try:
                result["exit_code"] = proc.wait(timeout=timeout)
                result["state"] = "EXITED"
            except subprocess.TimeoutExpired:
                result["state"] = "TIMED_OUT"
                result["cleanup"] = terminate_tree(proc)
            except KeyboardInterrupt:
                result["state"] = "INTERRUPTED"
                result["cleanup"] = terminate_tree(proc)
    except OSError as exc:
        result["state"] = "START_FAILED"
        result["error"] = str(exc)
    finally:
        result["duration_seconds"] = round(time.monotonic() - started, 3)
        result["finished_utc"] = datetime.now(timezone.utc).isoformat()
        if log.is_file():
            result["log_sha256"] = digest(log)
            # Stream the tail instead of loading a potentially huge compiler log.
            with log.open(encoding="utf-8", errors="replace") as stream:
                print("".join(deque(stream, maxlen=30)), end="")
        write_json(log.with_suffix(".json"), result)
    print(f"Process state: {result['state']}; exit: {result['exit_code']}", flush=True)
    return result
