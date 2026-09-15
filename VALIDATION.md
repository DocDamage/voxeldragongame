# WYRMFALL validation record

## Current Windows execution — September 15, 2026

UE 5.8.2 at `C:\Program Files\UE_5.8`: editor build PASS (`-NoUBA -NoPCH`),
bootstrap creation/unchanged rerun PASS, all six native scaffold tests Success,
and focused BOOT-01 keyboard/mouse PIE checks exercised successfully. Physical
controller behavior is NOT_RUN (none connected). Portable suite: 124 tests,
OK with two skips. WP-00 sample imports/bounds/bones/clips are recorded; complete
readiness and WP-01 remain NOT_RUN at the missing-provider boundary.

See [exact commands, limitations and evidence](Documentation/Current/BOOT-01_C_RECOVERY.md).
The historical D: onboarding receipt is STALE_EVIDENCE and is not reused as a pass.

## Original v0.2 authoring record (historical)

**September 15, 2026. Source hardening and portable tooling, not a playable build.**
The authoring host is Linux, not the user's Windows Unreal workstation. No Unreal
installation was located. All native compilation/editor/PIE/cook results remain
**NOT_RUN**.

## Executed checks

| Check | Result | What it proves |
|---|---|---|
| Tooling regression suite | **123 passed, zero failures** | Standard-library tooling behavior; includes real Python child processes |
| Offline onboarding | **PASS** | Source/config checks, tooling tests and original design-pack integrity run in sequence |
| Missing-engine native onboarding | **Expected BLOCKED at doctor, exit 2** | No generation/build/bootstrap/native-test step ran after the blocker |
| Source/config/link verification | **PASS** | JSON/TOML, source structure, generated-header ordering and current local Markdown links |
| Python syntax | **All eight Python files parsed** | AST syntax only; editor script was not imported into a fake Unreal module |
| Original pack | **36/36 exact byte matches** | All uploaded design-pack files preserved, including its manifest |
| Original pack verifier | **PASS** | 35 payload checksums, 89 local links, 118 NOT_RUN procedures, 22 requirements |
| Native suite discovery | **Six source-declared cases** | Declaration discovery, NOT execution of C++ tests |

The 123 tests include new/omitted/duplicate native-case detection, malformed/error
reports, real child exit/failure/timeout handling, POSIX descendant cleanup,
Windows command-line construction, onboarding stop/recovery behavior, changed
source/log evidence, missing map receipts, strict version/settings/manifest data,
and asset-root preservation. Synthetic metadata and native-report fixtures are
explicitly labeled and created only in temporary test directories. They do not
stand in for an Unreal installation or gameplay proof.

The Windows batch command-line and PowerShell wrapper are authored, not executed
on Windows here. The POSIX process-tree cleanup was exercised; Windows taskkill
cleanup requires its own local check. No test attempts to compile against fake UE
headers or substitutes another game engine.

## Native work still required

Compile with the actual UE 5.8.x installation and approved plugins, then run the
bootstrap script and all six source-declared native tests. The updated controller,
GAS base/current clamps, typed map checks, collision/navigation and visible input
behavior need actual engine execution. Run focused PIE afterward. A map receipt
or successful command sequence does not clear any terrain/Mutable/production gate.

No map, terrain provider, Mutable recipe, attacks, items, save system, dragon,
Heartfold transition, horror reward, flying car or colony was implemented by this
hardening pass. Existing source boundaries and all full-game design requirements
remain intact. No Git operation, CI, asset installation or paid redistribution
occurred.

## Evidence and reproducibility

[Regression output](Documentation/Current/evidence/v02/tooling_unittest.txt) ·
[Offline startup output](Documentation/Current/evidence/v02/offline_onboarding.log) ·
[Missing-engine startup output](Documentation/Current/evidence/v02/native_blocked_onboarding.log) ·
[Source/syntax checks](Documentation/Current/evidence/v02/source_checks.json) ·
[Original preservation](Documentation/Current/evidence/source_preservation.json) ·
[Changes from v0.1](Documentation/Current/evidence/v02/changes_from_v01.json)

Evidence copied into Documentation records this authoring run. Paths inside those
logs identify that run, not files already present in a new user's Saved folder.
The shipped source intentionally excludes Saved, local settings and caches; a
fresh extraction reports NOT_RUN until its own onboarding execution.

```powershell
py -3 tools/wyrm.py verify --manifest
py -3 tools/wyrm.py onboard
py -3 tools/wyrm.py plan --native
py -3 tools/wyrm.py onboard --native --engine-root "actual local engine root"
py -3 tools/wyrm.py report
```

`--manifest` checks the pristine release only. Intentional edits change release
hashes; use plain `verify` during development. The onboarding fingerprint checks
host source/config/tools and recorded evidence, not the entire engine installation
or third-party binaries. Changes to those require a fresh native validation run.
