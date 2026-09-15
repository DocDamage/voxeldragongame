"""A bounded Codex/Windows bootstrap workflow, not a gameplay gate manager."""
from __future__ import annotations

from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import sys
import uuid

from wyrm_process import execute_logged
from wyrm_support import (SetupError, digest, discover_native_tests, inside,
                          read_json, source_files, write_json)

OFFLINE = ("verify", "test", "design-verify")
NATIVE = ("doctor", "generate", "build", "bootstrap", "ue-test")
ARTIFACTS = {
    "doctor": ("Saved/Diagnostics/doctor.json",),
    "bootstrap": ("Saved/Diagnostics/bootstrap.json",
                  "Content/WYRMFALL/Development/Maps/L_DEV_Bootstrap.umap"),
    "ue-test": ("Saved/Automation/Scaffold/index.json",),
}


def source_snapshot(root: Path) -> dict:
    """Fingerprint host source/config/tools, not an installed engine or all art.

    Generated maps are recorded separately after bootstrap. Engine binaries and
    vendor binary compatibility still require the real build, not this hash.
    """
    rows = []
    for name in ("Source", "Config", "tools", "tests", "Plugins"):
        for path in source_files(root / name):
            if path.suffix.lower() in {".md", ".txt"} or path.name == "IntegrationReadiness.json":
                continue
            rows.append({"path": path.relative_to(root).as_posix(), "sha256": digest(path)})
    project = root / "WYRMFALL.uproject"
    if not project.is_file():
        raise SetupError("No WYRMFALL.uproject in onboarding root")
    rows.append({"path": project.name, "sha256": digest(project)})
    rows.sort(key=lambda item: item["path"])
    encoded = json.dumps(rows, sort_keys=True, separators=(",", ":")).encode()
    return {"scope": "host_source_config_tools_project_plugin_sources_not_engine_binaries",
            "sha256": hashlib.sha256(encoded).hexdigest(), "files": rows}


def make_plan(root: Path, native: bool, engine_root: str | None, timeout: int) -> list[dict]:
    if type(timeout) is not int or timeout < 1:
        raise SetupError("Timeout must be a positive integer")
    steps = []
    for name in OFFLINE + (NATIVE if native else ()):
        if name == "design-verify":
            command = [sys.executable, str(root / "Documentation/DesignPack/tools/verify_package.py")]
        else:
            command = [sys.executable, str(root / "tools/wyrm.py"), name]
            if name in NATIVE and engine_root:
                command += ["--engine-root", engine_root]
            if name == "doctor":
                command += ["--write-report"]
            elif name in NATIVE:
                command += ["--timeout", str(timeout)]
        steps.append({"id": name, "kind": "native_command" if name in NATIVE else "offline_check",
                      "command": command, "timeout": timeout + 30})
    return steps


def recovery_for(step: str | None, native: bool) -> str:
    actions = {
        "verify": "Fix the reported source/config/link problem; rerun verify and the affected tooling tests.",
        "test": "Fix the failing Python regression; do not bypass or delete the test to continue.",
        "design-verify": "Restore the preserved DesignPack from the original archive; do not regenerate its checksums around edits.",
        "doctor": "Resolve the exact local engine/plugin/toolchain blocker in the log. Do not disable Mutable or change engine family.",
        "generate": "Inspect project-generation output and local permissions. Do not initialize a different project.",
        "build": "Fix the first UHT/UBT error against actual installed headers, then rerun the focused editor build.",
        "bootstrap": "Inspect the editor log and partial developer map. Preserve existing assets; repair explicitly in Unreal.",
        "ue-test": "Inspect the fresh native report and missing/failed source-declared cases; compile fixes before rerunning tests.",
    }
    if step:
        return actions.get(step, "Inspect the recorded failure before proceeding.")
    return ("BOOT-01 still needs focused PIE observations; then WP-00 inventory and one eligible WP-01 proof."
            if native else "Run onboard --native on the actual Unreal-equipped host, then perform focused PIE observations.")


def render_report(data: dict) -> str:
    lines = ["# WYRMFALL onboarding result", "", f"**Status: {data['status']}**", "",
             "These are command checks, not a playable build or a G0/G1 acceptance pass.", "",
             "| Step | Result | Exit | Log |", "|---|---|---|---|"]
    for step in data["steps"]:
        result = step.get("result", {})
        lines.append(f"| {step['id']} | {step['status']} | {result.get('exit_code', '—')} | `{result.get('log', '—')}` |")
    lines += ["", "## Next bounded task", "", data["next_task"], "",
              "## Still unverified", "", "PIE, controller hardware, asset import, terrain, Mutable content and cooking remain NOT_RUN here.",
              "No source snapshot or command exit code overrides those gates.", ""]
    return "\n".join(lines)


def run_onboarding(root: Path, native: bool = False, engine_root: str | None = None,
                   timeout: int = 1800) -> tuple[int, Path]:
    plan = make_plan(root, native, engine_root, timeout)
    snapshot = source_snapshot(root)
    expected = sorted(discover_native_tests(root))
    folder = root / "Saved/Onboarding"
    folder.mkdir(parents=True, exist_ok=True)
    lock = folder / "active.lock"
    try:
        handle = lock.open("x", encoding="utf-8")
    except FileExistsError as exc:
        raise SetupError("Another onboarding run or stale lock exists at Saved/Onboarding/active.lock. "
                         "Inspect its PID and processes; never delete a live run's lock.") from exc
    run_id = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ") + "_" + uuid.uuid4().hex[:12]
    target = folder / run_id / "run.json"
    data = {"schema": 1, "kind": "onboarding_command_evidence_not_gameplay_proof", "run_id": run_id,
            "workspace": str(root.resolve()), "native_requested": native, "status": "RUNNING",
            "source_snapshot": snapshot, "expected_native_tests": expected,
            "steps": [{**step, "status": "NOT_RUN"} for step in plan], "next_task": "Current run in progress.",
            "pie": "NOT_RUN", "cook": "NOT_RUN"}

    def save():
        write_json(target, data)
        target.with_suffix(".md").write_text(render_report(data), encoding="utf-8")
        write_json(folder / "latest.json", {"run": target.relative_to(root).as_posix()})

    with handle:
        handle.write(json.dumps({"pid": os.getpid(), "run_id": run_id})); handle.flush()
    code = 0
    try:
        save()
        for step in data["steps"]:
            step["status"] = "RUNNING"; save()
            try:
                result = execute_logged(step["command"], root, step["id"], step["timeout"])
                step["result"] = result
                if result["state"] != "EXITED" or result["exit_code"] != 0:
                    step["status"] = "BLOCKED" if result["exit_code"] == 2 or result["state"] != "EXITED" else "FAILED"
                    data["status"] = step["status"]
                    code = 2 if step["status"] == "BLOCKED" else 1
                else:
                    step["artifacts"] = []
                    for rel in ARTIFACTS.get(step["id"], ()):
                        path = inside(root, rel)
                        if not path.is_file():
                            raise SetupError(f"Successful command has no required evidence: {rel}")
                        step["artifacts"].append({"path": rel, "sha256": digest(path)})
                    step["status"] = "COMMAND_PASSED"
            except (SetupError, OSError) as exc:
                step["status"] = data["status"] = "BLOCKED"
                step["error"] = str(exc); code = 2
            if code:
                data["next_task"] = recovery_for(step["id"], native)
                save(); break
            save()
        after = source_snapshot(root)
        if snapshot["sha256"] != after["sha256"]:
            data["status"] = "SOURCE_CHANGED_RERUN_REQUIRED"
            data["next_task"] = "Source changed during the run. Inspect the changes and rerun the affected checks."
            code = 1
        elif not code:
            data["status"] = "NATIVE_COMMANDS_PASSED_PIE_NOT_RUN" if native else "OFFLINE_CHECKS_PASSED_NATIVE_NOT_RUN"
            data["next_task"] = recovery_for(None, native)
        data["finished_utc"] = datetime.now(timezone.utc).isoformat()
        save()
    finally:
        lock.unlink(missing_ok=True)
    return code, target


def inspect_latest(root: Path) -> dict:
    """Never promote an old success after source/evidence changes."""
    pointer = root / "Saved/Onboarding/latest.json"
    if not pointer.is_file():
        return {"status": "NOT_RUN", "problems": [], "next_task": "Run python tools/wyrm.py onboard."}
    value = read_json(pointer)
    if not isinstance(value, dict) or not isinstance(value.get("run"), str):
        raise SetupError("Malformed onboarding pointer")
    path = inside(root, value["run"])
    if not path.is_relative_to((root / "Saved/Onboarding").resolve()):
        raise SetupError("Onboarding pointer is outside its report directory")
    data = read_json(path)
    if (not isinstance(data, dict) or data.get("schema") != 1
            or data.get("kind") != "onboarding_command_evidence_not_gameplay_proof"
            or not isinstance(data.get("steps"), list) or not isinstance(data.get("source_snapshot"), dict)):
        raise SetupError("Malformed onboarding receipt")
    if type(data.get("native_requested")) is not bool:
        raise SetupError("Malformed onboarding native-request flag")
    wanted_steps = list(OFFLINE + (NATIVE if data["native_requested"] else ()))
    if [step.get("id") if isinstance(step, dict) else None for step in data["steps"]] != wanted_steps:
        raise SetupError("Missing, reordered or duplicated onboarding steps")
    success = data.get("status") in {"OFFLINE_CHECKS_PASSED_NATIVE_NOT_RUN", "NATIVE_COMMANDS_PASSED_PIE_NOT_RUN"}
    if success and any(step.get("status") != "COMMAND_PASSED" for step in data["steps"]):
        raise SetupError("Onboarding success contains unpassed steps")
    if data.get("status") == "NATIVE_COMMANDS_PASSED_PIE_NOT_RUN" and not data["native_requested"]:
        raise SetupError("Cannot claim native commands for an offline run")
    problems = []
    if data.get("workspace") != str(root.resolve()):
        problems.append("Receipt belongs to another workspace; run checks in this extracted folder.")
    if source_snapshot(root)["sha256"] != data["source_snapshot"].get("sha256"):
        problems.append("Host source/config/tool/plugin input fingerprint changed.")
    for step in data["steps"]:
        if not isinstance(step, dict):
            raise SetupError("Malformed onboarding step")
        result = step.get("result", {})
        artifacts = step.get("artifacts", [])
        if not isinstance(result, dict) or not isinstance(artifacts, list):
            raise SetupError("Malformed onboarding evidence")
        if step.get("status") == "COMMAND_PASSED":
            if (result.get("state") != "EXITED" or type(result.get("exit_code")) is not int
                    or result["exit_code"] != 0 or not result.get("log") or not result.get("log_sha256")):
                raise SetupError("Passed step lacks a successful process result and hashed log")
            required = set(ARTIFACTS.get(step["id"], ()))
            if {item.get("path") for item in artifacts if isinstance(item, dict)} != required:
                raise SetupError("Passed step lacks its exact required artifact evidence")
        evidence = list(artifacts)
        if result.get("log"):
            evidence.append({"path": result["log"], "sha256": result.get("log_sha256")})
        for entry in evidence:
            if not isinstance(entry, dict) or not isinstance(entry.get("path"), str):
                raise SetupError("Malformed evidence entry")
            file = inside(root, entry["path"])
            if not file.is_file() or digest(file) != entry.get("sha256"):
                problems.append("Missing or changed evidence: " + entry["path"])
    return {"status": "STALE_EVIDENCE" if problems else data["status"], "problems": problems,
            "recorded_status": data["status"], "report": path.relative_to(root).as_posix(),
            "next_task": "Rerun the affected checks; do not reuse this receipt as current proof." if problems else data["next_task"],
            "pie": "NOT_RUN", "cook": "NOT_RUN"}
