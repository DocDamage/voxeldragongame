"""WYRMFALL local tooling. Standard library only; never fetches or installs assets."""
from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path
import platform
import re
import shutil
import subprocess
import sys
import time
import tempfile
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PROJECT_NAME = "WYRMFALL"
MAP_PACKAGE = "/Game/WYRMFALL/Development/Maps/L_DEV_Bootstrap"
REQUIRED_PLUGINS = ("EnhancedInput", "GameplayAbilities", "Mutable", "PythonScriptPlugin", "EditorScriptingUtilities")
EXCLUDED = {".git", ".local", "Binaries", "Intermediate", "Saved", "DerivedDataCache", "__pycache__", ".pytest_cache", "dist"}


class SetupError(RuntimeError):
    """Actionable failure, not a successful or silently downgraded setup."""


def read_json(path: Path) -> Any:
    try:
        return json.loads(path.read_text(encoding="utf-8-sig"))
    except (OSError, ValueError, UnicodeError) as exc:
        raise SetupError(f"Cannot read JSON {path}: {exc}") from exc


def write_json(path: Path, data: Any) -> None:
    """Same-filesystem atomic replacement; caller chooses a permitted output."""
    path.parent.mkdir(parents=True, exist_ok=True)
    # A unique sibling avoids two local processes trampling the same .tmp.
    tmp = None
    try:
        with tempfile.NamedTemporaryFile(mode="w", encoding="utf-8", dir=path.parent,
                                         prefix=path.name + ".", suffix=".tmp", delete=False) as stream:
            tmp = Path(stream.name)
            json.dump(data, stream, indent=2, ensure_ascii=False, allow_nan=False)
            stream.write("\n")
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(tmp, path)
    finally:
        if tmp is not None:
            tmp.unlink(missing_ok=True)


def digest(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def inside(root: Path, relative: str) -> Path:
    """Prevent file traversal, absolute paths and symlink escapes in manifests."""
    if "\\" in relative or Path(relative).is_absolute() or ":" in relative:
        raise SetupError(f"Not a safe relative path: {relative}")
    result = (root / relative).resolve()
    if not result.is_relative_to(root.resolve()) or result == root.resolve():
        raise SetupError(f"Path escapes file root: {relative}")
    return result


def source_files(root: Path):
    for current, dirs, files in os.walk(root, followlinks=False):
        dirs[:] = sorted(d for d in dirs if d not in EXCLUDED and not (Path(current) / d).is_symlink())
        for name in sorted(files):
            p = Path(current) / name
            if p.is_symlink() or p.suffix == ".pyc":
                continue
            yield p


def load_local(root: Path = ROOT) -> dict:
    path = root / ".local/settings.json"
    if not path.exists():
        return {}
    value = read_json(path)
    if not isinstance(value, dict):
        raise SetupError(".local/settings.json must contain a JSON object")
    if "engine_root" in value and (not isinstance(value["engine_root"], str) or not value["engine_root"].strip()):
        raise SetupError("Configured engine_root must be a nonempty path string")
    roots = value.get("asset_roots", [])
    if not isinstance(roots, list) or any(not isinstance(p, str) or not p.strip() for p in roots):
        raise SetupError("Configured asset_roots must be a list of nonempty path strings")
    return value


def normalize_engine(path: Path) -> Path:
    """Accept the UE installation root or its Engine child, never migrate it."""
    path = path.expanduser().resolve()
    if path.name.lower() == "engine" and (path / "Build/Build.version").is_file():
        return path.parent
    return path


def engine_candidates(root: Path, explicit: str | None) -> list[Path]:
    # Explicit and configured values are authoritative, including invalid ones.
    # Do not hide a typo by silently choosing another engine.
    if explicit:
        return [normalize_engine(Path(explicit))]
    configured = os.environ.get("UE_ROOT") or load_local(root).get("engine_root")
    if configured:
        return [normalize_engine(Path(configured))]
    candidates: list[Path] = []
    if platform.system() == "Windows":
        base = Path(os.environ.get("ProgramFiles", "C:/Program Files")) / "Epic Games"
        if base.is_dir():
            candidates.extend(sorted(base.glob("UE_5.8*")))
        try:
            import winreg
            with winreg.OpenKey(winreg.HKEY_CURRENT_USER, r"SOFTWARE\Epic Games\Unreal Engine\Builds") as key:
                for i in range(winreg.QueryInfoKey(key)[1]):
                    _, value, _ = winreg.EnumValue(key, i)
                    candidates.append(Path(value))
        except (ImportError, OSError):
            pass
    # No scanning entire drives or another game's workspace.
    return list(dict.fromkeys(normalize_engine(p) for p in candidates))


def engine_version(engine: Path) -> tuple[int, int, int]:
    data = read_json(engine / "Engine/Build/Build.version")
    keys = ("MajorVersion", "MinorVersion", "PatchVersion")
    if not isinstance(data, dict) or any(type(data.get(k)) is not int for k in keys):
        raise SetupError("Engine Build.version needs integer major/minor/patch fields")
    result = tuple(data[k] for k in keys)
    if min(result) < 0:
        raise SetupError("Invalid negative engine version")
    return result


def choose_engine(root: Path = ROOT, explicit: str | None = None) -> Path:
    candidates = engine_candidates(root, explicit)
    matching = []
    for p in candidates:
        try:
            if engine_version(p)[:2] == (5, 8):
                matching.append(p)
        except SetupError:
            if len(candidates) == 1:
                raise
    if len(matching) > 1:
        raise SetupError("Multiple UE 5.8 installations found. Use configure --engine-root with the intended build: " + ", ".join(map(str, matching)))
    if matching:
        return matching[0]
    if candidates:
        raise SetupError("Configured/discovered engine is not UE 5.8.x. No migration performed. " + ", ".join(map(str, candidates)))
    raise SetupError("No UE 5.8 installation located. Run configure --engine-root with your actual installation directory, or set UE_ROOT. Unreal is not bundled.")


def plugin_locations(engine: Path, root: Path = ROOT) -> dict[str, list[Path]]:
    result: dict[str, list[Path]] = {}
    for base in (engine / "Engine/Plugins", root / "Plugins"):
        if not base.is_dir():
            continue
        for p in source_files(base):
            if p.suffix == ".uplugin":
                result.setdefault(p.stem, []).append(p)
    return result


def ue_tools(engine: Path, system: str | None = None) -> dict[str, Path]:
    system = system or platform.system()
    if system == "Windows":
        return {
            "build": engine / "Engine/Build/BatchFiles/Build.bat",
            "editor": engine / "Engine/Binaries/Win64/UnrealEditor.exe",
            "editor_cmd": engine / "Engine/Binaries/Win64/UnrealEditor-Cmd.exe",
            "uat": engine / "Engine/Build/BatchFiles/RunUAT.bat",
        }
    if system == "Linux":
        return {
            "build": engine / "Engine/Build/BatchFiles/Linux/Build.sh",
            "editor": engine / "Engine/Binaries/Linux/UnrealEditor",
            "editor_cmd": engine / "Engine/Binaries/Linux/UnrealEditor-Cmd",
            "uat": engine / "Engine/Build/BatchFiles/RunUAT.sh",
        }
    raise SetupError("This scaffold's runner supports Windows first, with Linux source tooling. Other hosts need explicit validation.")


def toolchain_inventory(system: str | None = None) -> dict:
    result: dict[str, Any] = {"status": "NOT_BUILD_VERIFIED"}
    if (system or platform.system()) != "Windows":
        result["compiler"] = shutil.which("clang++")
        result["note"] = "Not a Win64 toolchain proof; cross-compiling Win64 is not configured."
        return result
    vswhere = Path(os.environ.get("ProgramFiles(x86)", "C:/Program Files (x86)")) / "Microsoft Visual Studio/Installer/vswhere.exe"
    result["vswhere"] = str(vswhere) if vswhere.is_file() else None
    if vswhere.is_file():
        try:
            proc = subprocess.run([str(vswhere), "-products", "*", "-requires", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64", "-format", "json"], capture_output=True, text=True, timeout=30, check=False)
            result["visual_studio"] = json.loads(proc.stdout) if proc.returncode == 0 else []
        except (OSError, subprocess.TimeoutExpired, ValueError):
            result["visual_studio"] = []
    sdk = Path(os.environ.get("ProgramFiles(x86)", "C:/Program Files (x86)")) / "Windows Kits/10/Include"
    result["sdk_include_versions"] = sorted(p.name for p in sdk.glob("10.*") if p.is_dir()) if sdk.is_dir() else []
    return result


def doctor(root: Path = ROOT, explicit: str | None = None) -> dict:
    report: dict[str, Any] = {"kind": "local_environment_inspection_not_runtime_validation", "project": str(root / "WYRMFALL.uproject"),
        "platform": platform.platform(), "python": sys.version.split()[0], "engine": None, "plugins": {}, "errors": [], "warnings": [],
        "gameplay_status": "NOT_RUN", "toolchain": toolchain_inventory()}
    try:
        engine = choose_engine(root, explicit)
        version = engine_version(engine)
        report["engine"] = {"path": str(engine), "version": ".".join(map(str, version)), "preferred_patch": "5.8.2", "status": "VERSION_FILE_INSPECTED"}
        if version != (5, 8, 2):
            report["warnings"].append("Target family matches 5.8.x, but preferred patch is 5.8.2. No engine change was made.")
        locations = plugin_locations(engine, root)
        for name in REQUIRED_PLUGINS:
            found = locations.get(name, [])
            row: dict[str, Any] = {"paths": [str(p) for p in found], "status": "MISSING"}
            if len(found) == 1:
                data = read_json(found[0])
                if not isinstance(data, dict) or not isinstance(data.get("Modules", []), list):
                    raise SetupError(f"Malformed plugin descriptor: {found[0]}")
                row.update({"status": "DESCRIPTOR_FOUND_NOT_VALIDATED", "version": data.get("VersionName"),
                            "modules": data.get("Modules", []), "sha256": digest(found[0])})
            elif len(found) > 1:
                row["status"] = "AMBIGUOUS_DUPLICATES"
                report["errors"].append(f"Duplicate {name} descriptors; inspect intended plugin ownership.")
            else:
                report["errors"].append(f"Required plugin descriptor missing: {name}")
            report["plugins"][name] = row
        report["executables"] = {k: {"path": str(v), "exists": v.is_file()} for k,v in ue_tools(engine).items()}
        for key in ("build", "editor", "editor_cmd"):
            if not report["executables"][key]["exists"]:
                report["errors"].append(f"Missing engine tool: {report['executables'][key]['path']}")
        if platform.system() == "Windows":
            if not report["toolchain"].get("visual_studio"):
                report["warnings"].append("No C++ Visual Studio installation confirmed by vswhere. UBT's build is the authority; inspect compiler installation.")
            if not report["toolchain"].get("sdk_include_versions"):
                report["warnings"].append("No Windows SDK include directory found in the standard location.")
    except SetupError as exc:
        report["errors"].append(str(exc))
    report["status"] = "BLOCKED" if report["errors"] else "INSPECTED_NOT_BUILD_VERIFIED"
    return report


def require_engine(root: Path, explicit: str | None = None) -> Path:
    report = doctor(root, explicit)
    if report["errors"]:
        raise SetupError("\n".join(report["errors"]))
    return Path(report["engine"]["path"])


def build_command(root: Path, engine: Path, action: str, system: str | None = None) -> list[str]:
    """Command construction is pure and tested with paths containing spaces."""
    tools = ue_tools(engine, system)
    proj = str(root / "WYRMFALL.uproject")
    if action == "generate":
        return [str(tools["build"]), "-projectfiles", f"-project={proj}", "-game", "-engine", "-progress"]
    if action == "build":
        target = "Win64" if (system or platform.system()) == "Windows" else "Linux"
        return [str(tools["build"]), "WYRMFALLEditor", target, "Development", f"-Project={proj}", "-WaitMutex", "-NoHotReloadFromIDE", "-NoEngineChanges"]
    if action == "bootstrap":
        # The editor Python command parser interprets backslash escapes (e.g.
        # the Windows path segment \tools becomes a tab). Use forward slashes.
        script = (root / "tools/unreal/create_bootstrap_map.py").as_posix()
        return [str(tools["editor_cmd"]), proj, "-unattended", "-nop4", "-nosplash", "-nosound", "-nullrhi", f"-ExecutePythonScript={script}", "-stdout", "-FullStdOutLogOutput"]
    if action == "open":
        cmd = [str(tools["editor"]), proj]
        if (root / "Content/WYRMFALL/Development/Maps/L_DEV_Bootstrap.umap").is_file():
            cmd.append(MAP_PACKAGE)
        return cmd + ["-log"]
    if action == "ue-test":
        return [str(tools["editor_cmd"]), proj, "-unattended", "-nop4", "-nosplash", "-nosound", "-nullrhi", "-ExecCmds=Automation RunTests WYRMFALL.Scaffold", "-TestExit=Automation Test Queue Empty", f"-ReportExportPath={root / 'Saved/Automation/Scaffold'}", "-stdout", "-FullStdOutLogOutput"]
    raise SetupError(f"Unsupported engine command: {action}")


def run_logged(command: list[str], root: Path, label: str, timeout: int) -> int:
    # The process runner is shared with onboarding; it records every attempt.
    from wyrm_process import execute_logged
    result = execute_logged(command, root, label, timeout)
    if result["state"] != "EXITED":
        raise SetupError(f"{label}: {result['state']}. {result.get('error', '')} Inspect {result['log']}")
    return result["exit_code"]


def discover_native_tests(root: Path = ROOT) -> set[str]:
    """Read literal test IDs, not C++ behavior. This is NOT compiling the tests.

    Keep the scaffold suite to IMPLEMENT_SIMPLE_AUTOMATION_TEST macros. A new
    parameterized/spec test needs an explicitly reviewed discovery extension.
    """
    names: list[str] = []
    for path in source_files(root / "Source"):
        if path.suffix != ".cpp":
            continue
        text = path.read_text(encoding="utf-8")
        # Preserve strings while removing comments so example IDs are not tests.
        pattern = r'"(?:\\.|[^"\\])*"|//[^\n]*|/\*.*?\*/'
        text = re.sub(pattern, lambda m: m.group() if m.group().startswith('"') else ' ', text, flags=re.S)
        declared = re.findall(r'IMPLEMENT_SIMPLE_AUTOMATION_TEST\s*\(\s*\w+\s*,\s*"(WYRMFALL\.Scaffold\.[^"]+)"', text)
        literals = set(re.findall(r'"(WYRMFALL\.Scaffold\.[^"]+)"', text))
        if literals - set(declared):
            raise SetupError(f"Unrecognized scaffold test declaration in {path}; extend discovery explicitly")
        names.extend(declared)
    if not names or len(names) != len(set(names)):
        raise SetupError("Native source test catalogue is empty or has duplicate test IDs")
    return set(names)


def assess_automation(data: dict, expected: set[str] | None = None) -> tuple[bool, str]:
    """Require the complete source-declared suite, not a hard-coded case count."""
    if expected is None:
        expected = discover_native_tests()
    if not expected or any(not isinstance(n, str) or not n.startswith("WYRMFALL.Scaffold.") for n in expected):
        return False, "Invalid/empty expected native test catalogue."
    if not isinstance(data, dict) or not isinstance(data.get("tests"), list):
        return False, "Malformed automation report."
    tests = data["tests"]
    if any(not isinstance(t, dict) or not isinstance(t.get("fullTestPath"), str) for t in tests):
        return False, "Malformed test record."
    selected = [t for t in tests if t["fullTestPath"].startswith("WYRMFALL.Scaffold.")]
    names = [t["fullTestPath"] for t in selected]
    if set(names) != expected or len(names) != len(set(names)):
        return False, ("Native test coverage mismatch. Missing: " + str(sorted(expected - set(names)))
                       + "; unexpected: " + str(sorted(set(names) - expected)) + "; duplicates forbidden.")
    if any(t.get("state") != "Success" for t in selected):
        return False, "At least one scaffold automation case did not report Success."
    for key in ("failed", "notRun", "inProcess"):
        value = data.get(key, 0)
        if type(value) is not int or value != 0:
            return False, f"Automation summary {key} is nonzero or malformed."
    for test in selected:
        count = test.get("errors", 0)
        if type(count) is not int or count != 0:
            return False, "Successful-looking native test contains errors."
    return True, f"All {len(expected)} source-declared native tests succeeded; not a gameplay gate pass."


def validate_bootstrap_receipt(data: dict, map_file: Path) -> None:
    """Check editor output identity, not collision/PIE or production content."""
    if (not isinstance(data, dict) or data.get("status") != "EDITOR_MAP_SAVED"
            or data.get("map") != MAP_PACKAGE or data.get("actor_types_checked") is not True):
        raise SetupError("Malformed/missing typed bootstrap receipt; run the current editor script")
    if not map_file.is_file() or map_file.stat().st_size < 64:
        raise SetupError("Bootstrap map file is missing or implausibly small")
    if (type(data.get("map_bytes")) is not int or data["map_bytes"] != map_file.stat().st_size
            or data.get("map_sha256") != digest(map_file)):
        raise SetupError("Bootstrap map does not match the fresh editor receipt")
    if data.get("pie") != "NOT_RUN":
        raise SetupError("Map creation must not claim a PIE pass")


def markdown_link_errors(root: Path) -> list[str]:
    errors = []
    # Source originals include informal/example links; their own preserved verifier
    # owns those. New Markdown uses ordinary relative file links, not autolinks.
    for p in source_files(root):
        if p.suffix != ".md" or "DesignPack" in p.parts:
            continue
        text = re.sub(r"```.*?```", "", p.read_text(encoding="utf-8"), flags=re.S)
        for value in re.findall(r"\[[^\]]*\]\(([^\s]+)\)", text):
            if "://" in value or value.startswith(("#", "mailto:")):
                continue
            value = value.split("#", 1)[0]
            if value and not (p.parent / value).exists():
                errors.append(f"Broken Markdown file link: {p.relative_to(root)} -> {value}")
    return errors


def verify_structure(root: Path = ROOT) -> list[str]:
    errors: list[str] = []
    needed = ["WYRMFALL.uproject", "Source/WYRMFALL.Target.cs", "Source/WYRMFALLEditor.Target.cs", "Source/WYRMFALL/WYRMFALL.Build.cs", "AGENTS.md", "CODEX_START_HERE.md", "CODEX_FIRST_TASK.txt", "Documentation/Current/STATUS.md", ".codex/config.toml", "Config/IntegrationReadiness.json"]
    for rel in needed:
        if not (root / rel).is_file():
            errors.append(f"Missing required scaffold file: {rel}")
    try:
        p = read_json(root / "WYRMFALL.uproject")
        if p.get("EngineAssociation") != "5.8":
            errors.append("Engine association must retain the approved 5.8 family")
        if [m.get("Name") for m in p.get("Modules", [])] != ["WYRMFALL"]:
            errors.append("Expected one WYRMFALL runtime module")
        plugins = {r["Name"]:r for r in p.get("Plugins", [])}
        for name in REQUIRED_PLUGINS:
            if not plugins.get(name, {}).get("Enabled"):
                errors.append(f"Required plugin must remain enabled: {name}")
        for name in ("PythonScriptPlugin", "EditorScriptingUtilities"):
            if plugins.get(name, {}).get("TargetAllowList") != ["Editor"]:
                errors.append(f"Editor-only plugin not restricted to Editor: {name}")
    except SetupError as exc:
        errors.append(str(exc))
    for p in source_files(root):
        if p.suffix == ".json":
            try: read_json(p)
            except SetupError as exc: errors.append(str(exc))
        if p.suffix == ".h" and "Source" in p.parts:
            text = p.read_text(encoding="utf-8")
            includes = re.findall(r'^#include\s+"([^"]+)"', text, re.M)
            if any(x in text for x in ("UCLASS(", "USTRUCT(", "UINTERFACE(")):
                if not includes or not includes[-1].endswith(".generated.h"):
                    errors.append(f"Generated header must be the last include: {p.relative_to(root)}")
        if p.suffix in {".uasset", ".umap"} and p.stat().st_size < 64:
            errors.append(f"Suspicious placeholder Unreal binary: {p.relative_to(root)}")
    try:
        import tomllib
        with (root / ".codex/config.toml").open("rb") as stream:
            config = tomllib.load(stream)
        if "model" in config or "model_provider" in config:
            errors.append("Do not pin an unverified account model/provider")
        if config.get("sandbox_mode") != "workspace-write":
            errors.append("Scaffold uses workspace-write, not full filesystem access")
    except (OSError, ValueError, ImportError) as exc:
        errors.append(f"Codex TOML/Python 3.11 requirement: {exc}")
    try:
        discover_native_tests(root)
    except (SetupError, OSError) as exc:
        errors.append(str(exc))
    errors.extend(markdown_link_errors(root))
    return errors


def manifest_errors(root: Path = ROOT) -> list[str]:
    data = read_json(root / "SCAFFOLD_MANIFEST.json")
    if not isinstance(data, dict) or not isinstance(data.get("files"), list) or not data["files"]:
        return ["Manifest must contain a nonempty files array"]
    errors: list[str] = []
    seen: set[str] = set()
    for entry in data["files"]:
        try:
            if not isinstance(entry, dict) or not isinstance(entry.get("path"), str):
                raise SetupError("Malformed manifest record")
            if entry["path"] in seen:
                raise SetupError(f"Duplicate manifest entry: {entry['path']}")
            seen.add(entry["path"])
            if type(entry.get("bytes")) is not int or entry["bytes"] < 0:
                raise SetupError(f"Invalid manifest byte count: {entry['path']}")
            if not isinstance(entry.get("sha256"), str) or not re.fullmatch(r"[0-9a-f]{64}", entry["sha256"]):
                raise SetupError(f"Invalid SHA-256: {entry['path']}")
            path = inside(root, entry["path"])
            if not path.is_file():
                errors.append(f"Manifest missing: {entry['path']}")
            elif path.stat().st_size != entry["bytes"] or digest(path) != entry["sha256"]:
                errors.append(f"Manifest changed: {entry['path']}")
        except (SetupError, OSError) as exc:
            errors.append(str(exc))
    return errors
