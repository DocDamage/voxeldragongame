# Windows local setup

## Prerequisites to inspect, not assume

Use the approved UE **5.8.x** installation, preferably **5.8.2**. The engine is not included. The standalone runner needs Python **3.11+** available as `py -3` or `python`. The embedded editor Python is separate and needs no pip packages for this map script. The existing Unreal project enables EnhancedInput, GameplayAbilities, Mutable, PythonScriptPlugin and EditorScriptingUtilities. The last two are Editor-only.

Epic's current UE 5.8 setup documentation lists Visual Studio 2022 17.14+ or Visual Studio 2026 18.0+; it recommends VS 2026 for general development. Install/modify the C++ game-development components and an appropriate Windows SDK using the existing Visual Studio Installer only when needed and authorized. The locally installed UBT diagnostics, not this text, settle the exact compiler/SDK compatibility. [Dated primary references](TECHNICAL_REFERENCES.md).

## One-command bootstrap

```powershell
py -3 tools/wyrm.py plan --native
py -3 tools/wyrm.py onboard --native --engine-root "C:\Program Files\Epic Games\UE_5.8"
py -3 tools/wyrm.py report
```

Use the actual local engine path, not the example blindly. `plan` only previews.
`onboard` without `--native` runs offline checks only. The native sequence does
not run PIE, cook, install software or perform Git operations. It stops on a
failure; later steps remain NOT_RUN in its recovery report. Fix a concrete
failure using the individual commands below. Close existing editor instances
before the native build.

The PowerShell equivalent is `.\Setup.ps1 -Native -EngineRoot "actual path"`.
The v0.1 `-Build -Bootstrap` switches are replaced by `-Native`. `-PlanOnly`
previews without running commands or changing local paths.

## Local procedure

Open the extracted root in a Windows terminal. First verify without an engine:

```powershell
py -3 tools/wyrm.py verify --manifest
py -3 tools/wyrm.py test
```

`--manifest` verifies the pristine release, so after editing source use `verify` without that flag. No packages/network are required by these checks.

Set a real engine location and optionally approved asset locations:

```powershell
py -3 tools/wyrm.py configure --engine-root "C:\Program Files\Epic Games\UE_5.8" --asset-root "D:\WYRMFALL_Assets"
py -3 tools/wyrm.py doctor --write-report
py -3 tools/wyrm.py inventory
```

Those are example paths; use actual locations already known in the session. The runner checks UE_ROOT first, then `.local/settings.json`, then a bounded Windows launcher/registered-build discovery. An explicit wrong path is an error, not permission to silently select another engine. Multiple matching engines need a concrete choice. Source-built installations are supported by their explicit path and Build.version; `.uproject` association is not silently rewritten.

Close any existing editor instance before a normal source build to avoid Live Coding/module-lock confusion. Then:

```powershell
py -3 tools/wyrm.py generate
py -3 tools/wyrm.py build
py -3 tools/wyrm.py bootstrap
py -3 tools/wyrm.py ue-test
py -3 tools/wyrm.py open
```

The explicit editor launch supplies the diagnostic map if generated, while default INI startup remains the real engine Entry map. The bootstrap script runs via full-editor `-ExecutePythonScript`, not runtime Python or an early ExecCmd. It loads an existing map unchanged rather than overwriting it. Native reports are archived before reruns so stale success cannot count.

## Expected evidence locations

`Saved/Onboarding/<run>/run.md` and `run.json`, `Saved/Onboarding/latest.json`, `Saved/Diagnostics/doctor.json`, `Saved/Diagnostics/G0Inventory.json`, `Saved/Diagnostics/bootstrap.json`, `Saved/ScaffoldLogs/`, `Saved/Automation/Scaffold/index.json`. All are local/generated, not prefilled fake evidence. For a handoff, summarize relevant results under Documentation/Current and reference the actual log paths.

## Troubleshooting

**Python command missing:** use an installed 3.11+ interpreter. Do not change global PATH or install packages without need. `py -3 --version` confirms what the launcher selected.

**PowerShell execution policy prevents Setup.ps1:** run the Python commands directly. Do not globally disable execution policy.

**Engine or plugin missing:** inspect the precise reported path/plugin. `Mutable` is the `.uplugin` name; `CustomizableObject` is a module, not the enable-list name. Do not disable Mutable to force a green build.

**Doctor succeeds but build fails:** metadata discovery is not compiler/SDK/plugin certification. Read the complete UBT log; resolve the specific dependency/version error. No automatic full-engine rebuild is configured.

**Build settings or include-order warning:** targets use the installed engine's `Latest` settings within the explicitly constrained 5.8 family. Verify that installed engine/target match; don't downgrade or rewrite vendor source indiscriminately.

**Build blocked by Codex sandbox:** Unreal may write to external machine-local caches. Approve the necessary command/location under the current client policy rather than grant blanket machine/network access.

**Bootstrap exits but no map receipt:** failure. Inspect Python/Unreal logs; no receipt is manufactured. No native test report / zero selected tests is likewise failure.

**Input seems absent in PIE:** focus the viewport; inspect `showdebug enhancedinput`. Ensure the intended native game mode is active and editor build is current. The bootstrap capsule is a deliberate wireframe; no animated character is supplied yet.

**Click-to-move is blocked:** expected in the bootstrap map, which has no NavMesh. Add/prove actual navigation in WP-01; static floor movement is not a dynamic voxel proof.

**Using cloud/WSL:** portable tooling is supported, but Windows engine commands must run on the real Windows host. No Win64 cross-toolchain or automatic asset sync is included. Do not treat cloud tests as local engine verification.

**Timeout/interrupt:** the runner attempts process-tree cleanup and never records a pass. Windows taskkill execution remains unverified in the authoring environment. Check whether Unreal/UBT child processes remain active before retrying, preserve their logs, and close only the processes belonging to this task.


**Stale report:** `report` rejects changed host source/config/tool inputs, missing
logs, and altered recorded map/report bytes. The fingerprint deliberately does
not claim complete engine/vendor-binary attestation. Rerun relevant native checks
after changing an engine installation, plugin binaries, compiler or runtime content.

**Active onboarding lock:** inspect `Saved/Onboarding/active.lock` and its PID.
Do not delete a live run's lock. After a crash, confirm its processes have exited
before removing only that stale lock and rerunning.

**Batch-path rejection:** Windows batch commands reject shell expansion characters
such as `%`, `!`, `&` and embedded quotes rather than interpolate them. Ordinary
paths with spaces or parentheses are quoted. Use a normal local workspace path;
Windows command construction was tested, but actual Windows execution is pending.
