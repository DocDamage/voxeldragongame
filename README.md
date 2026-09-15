# WYRMFALL — Codex-ready source starter

**v0.2 · September 15, 2026 · New Unreal source scaffold**  
**Target:** UE 5.8.x, preferably 5.8.2; Windows first.  
**Status:** C++ source and local tooling authored. Unreal compile, editor, PIE and cooked build **NOT RUN** here.

This is the working project shell, not another documentation-only ZIP and not a playable game. Start with [CODEX_START_HERE](CODEX_START_HERE.md), [current status](Documentation/Current/STATUS.md), and [validation](VALIDATION.md). The complete uploaded documentation pack is preserved under [Documentation/DesignPack](Documentation/DesignPack/README.md).

## Start on your Windows development machine

Extract into a new writable local folder such as `D:\Projects\WYRMFALL`. Do not overwrite a different Unreal project. Open that extracted folder as the **primary local project** in Codex. The folder containing `WYRMFALL.uproject` and `AGENTS.md` is the root, not Documentation/DesignPack.

Paste [CODEX_FIRST_TASK.txt](CODEX_FIRST_TASK.txt) into Codex. It directs the agent to inspect the existing environment, compile the host, create the diagnostic map and then inventory the approved content. No repository is required for these local commands; no Git operation is performed by setup.

For a single bounded setup run, use these commands from the extracted root:

```powershell
py -3 tools/wyrm.py plan --native
py -3 tools/wyrm.py onboard --native --engine-root "C:\Program Files\Epic Games\UE_5.8"
py -3 tools/wyrm.py report
```

The example engine path is not a claim about your installation. `plan` only prints
commands. `onboard` without `--native` runs the offline source, regression and
documentation-integrity checks. With `--native`, it additionally runs doctor →
project generation → editor build → typed/hash-checked diagnostic map creation →
the source-declared native test suite. It stops on the first failure and writes a
readable recovery report under `Saved/Onboarding/`. It never opens PIE or cooks.

`Setup.ps1 -Native -EngineRoot "actual path"` wraps the same workflow. Its default
is offline checks; `-PlanOnly` has no execution side effects. The old v0.1
`-Build -Bootstrap` switches are replaced by `-Native`. Python commands remain
available when PowerShell script execution is restricted. No execution policy,
software installation, global Codex setting or Git history is changed.

Python **3.11+** is required by the standalone tools; Unreal's embedded Python is
separate. To remember local paths, use `configure --engine-root "actual path"`;
this preserves already configured asset roots. Full prerequisites and individual
recovery commands: [WINDOWS_SETUP](Documentation/Current/WINDOWS_SETUP.md).

See [v0.2 changes](CHANGELOG.md) and [validation](VALIDATION.md). Engine/marketplace
binaries are not fingerprinted by the host-source receipt; rerun native checks
when those change. This package does not claim an unchanged engine from a source
hash alone.

## What exists

| Layer | Included | Truth boundary |
|---|---|---|
| Unreal host | `.uproject`, game/editor targets, single runtime module and INI defaults | Source written; not UHT/UBT verified |
| Input/cameras | Enhanced Input, movement, mouse/gamepad look, jump, C/R3 camera switching, optional diagnostic nav click route | No real-asset/control-completion claim |
| Combat foundation | One humanoid GAS component, health attribute set, current/base health clamps | No actual damage/abilities/death loop |
| Terrain seam | Capability/request types and unsupported-by-default interface | No provider, collision/nav callback or terrain save integration |
| Developer map | Explicit Unreal Python map builder; engine floor/light/player-start only | No `.umap` fabricated outside Unreal; no production art |
| Setup and checks | Configure, doctor, scoped inventory, generate/build/bootstrap/open, native test runner, tooling tests | Stops on missing inputs, hashes evidence, rejects incomplete native suites |
| Codex | Root instructions, project-local config, current status, bounded task packets, VS Code tasks | No API key/model subscription/global settings changed |
| Future integrations | Ownership/content folders, readiness record, preserved full design/backlog/tests | Not empty classes masquerading as completed systems |

## Controls in the diagnostic map

WASD / left stick move, mouse / right stick look, Space / gamepad A jump, C / right-stick click switch cameras. F6 toggles the **development-only** click-to-move test; top-down left-click requests a complete navigable route. The bootstrap map has **no NavMesh**, so a click correctly reports a blocked destination until an actual navigation fixture is added in G1. There is no combat input or final UI yet. The visible wireframe capsule is test instrumentation, not a replacement Mutable character.

## Deliberate exclusions

No Unreal installation, marketplace plugins, licensed assets, generated `.uasset` placeholders, engine binaries, cooked game, CI, credentials or Git history. No made-up save system or parallel inventory. Dragon/Heartfold/Echo/vehicle/colony requirements remain in the design pack and future task routing rather than being declared implemented.

[Architecture and ownership](Documentation/Current/SCAFFOLD_ARCHITECTURE.md) · [Content intake](Documentation/Current/ASSET_INTAKE.md) · [First session](Documentation/Current/tasks/BOOT-01.md) · [G0 readiness](Documentation/Current/tasks/WP-00.md) · [G1 provider proof](Documentation/Current/tasks/WP-01.md)
