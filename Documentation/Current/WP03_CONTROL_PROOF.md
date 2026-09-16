# WP-03: Shared Humanoid Control, Camera & Input Proof

**Date**: 2026-09-15
**Engine**: Unreal Engine 5.8.2 (`C:\Program Files\UE_5.8`)
**Status**: PARTIAL (native and editor-world commandlet checks pass; current full WP-03 PIE NOT_RUN)
**Evidence Receipts**:
- `Saved/Diagnostics/WP03_control_proof.json`
- `Saved/Automation/Scaffold/index.json` (10/10 native tests pass, including `WYRMFALL.Scaffold.SharedControlFoundation`)

---

## 1. Scope and Authority

In accordance with [AGENTS.md](../../AGENTS.md), [Save and Control Contract](../DesignPack/docs/systems/SAVE_AND_CONTROL_CONTRACT.md), and [Technical Architecture](../DesignPack/docs/TECHNICAL_ARCHITECTURE.md):
- Single created humanoid actor (`AWyrmCharacter`) shared between Third-Person follow view and Top-Down tactical/isometric view.
- Single player controller (`AWyrmPlayerController`) authoritative over input routing and movement commands.
- No secondary or duplicate pawn progression instances.
- Immediate route cancellation upon direct movement input (`WRLD-06`).
- Movement, look, and pathfinding respect input gating and movement lock states (`UI-02`, `UI-07`).
- Camera mode and control preferences captured and restored for persistent save integration (`SAVE-05` / `UI-03`).

---

## 2. Implementation Overview

### 2.1 Control Types and Persistence (`Source/WYRMFALL/Public/Player/WyrmControlTypes.h`)
- `EWyrmCameraMode` enum:
  - `ThirdPerson`: Follow camera behind character (pitch -18 deg, arm length 450 cm, FOV 80, control rotation active).
  - `TopDown`: Tactical camera overhead (pitch -60 deg, arm length 1100 cm, FOV 55, control rotation disabled, mouse cursor visible).
- `FWyrmControlState` struct:
  - Stores `CameraMode`, `bClickMoveEnabled`, `bInvertY`, and `bMovementLocked`.
  - Provides JSON serialization and deserialization helpers for save coordination.

### 2.2 Shared Character Architecture (`AWyrmCharacter`)
- **Camera Perspective Controls**:
  - `SetCameraMode(EWyrmCameraMode NewMode)` and `ToggleCamera()`.
  - `ApplyCamera()` dynamically updates `CameraBoom->TargetArmLength`, `CameraBoom->bUsePawnControlRotation`, `CameraBoom->SetWorldRotation`, and `Camera->FieldOfView`.
  - Preserves character attributes, GAS ability system info, equipped items, and velocity across mode switches.
- **Movement Lock Gating**:
  - `SetMovementLocked(bool bLocked)` and `IsMovementLocked()`.
  - Halts active pathing and jump state immediately when locked.
- **Control State Capture/Restore**:
  - `CaptureControlState(FWyrmControlState& OutState)`
  - `RestoreControlState(const FWyrmControlState& InState)`

### 2.3 Player Controller Architecture (`AWyrmPlayerController`)
- **Dual Movement Modes**:
  - Direct movement (WASD / Left stick): Calls `StopMovement()` immediately to cancel any active click-to-move path (`WRLD-06`). Routes movement input relative to control rotation yaw in ThirdPerson, and relative to fixed world axes in TopDown.
  - Top-Down click-to-move: Enabled by default in TopDown mode. Traces cursor against geometry, verifies reachability via `UNavigationSystemV1::ProjectPointToNavigation`, validates full path synchronously via `UNavigationPath`, and routes character with `UAIBlueprintHelperLibrary::SimpleMoveToLocation`.
  - Invalid destination clicks reject without setting bogus paths, stopping any stale route.
- **Input Gating & Chord Cleansing**:
  - All movement, look, jump, and click actions verify `!IsMovementLocked()`, `!IsMoveInputIgnored()`, and `!UGameplayStatics::IsGamePaused()`.
  - `OnUnPossess()` cleanses held actions and calls `StopMovement()` and `JumpReleased()`.
- **Static Factory Helper**:
  - `SpawnWyrmPlayerController(UObject* WorldContextObject, const FTransform& SpawnTransform)` for headless automation test and programmatic controller instantiation.

---

## 3. Verification Evidence

### 3.1 Native Automation Test Suite (`ue-test`)
- Test: `WYRMFALL.Scaffold.SharedControlFoundation` in `Source/WYRMFALL/Private/Tests/WyrmScaffoldTests.cpp`
- Results in `Saved/Automation/Scaffold/index.json`:
  - `WYRMFALL.Scaffold.SharedControlFoundation`: **Success** (Duration: 0.021s)
  - Full suite: **10/10 succeeded** (0 failed, 0 not run).

### 3.2 Editor-world Commandlet Suite (`verify_wp03_control_proof.py`)
Results recorded in `Saved/Diagnostics/WP03_control_proof.json`:

| Check ID | Requirement | Result | Verified Details |
|---|---|---|---|
| `CTRL-00` | Actor Spawn | **PASS** | `WyrmCharacter_0` and `WyrmPlayerController_0` instantiated and possessed cleanly. |
| `CTRL-01` | Third-Person Defaults | **PASS** | Mode: `ThirdPerson`, TargetArmLength: 450.0, FOV: 80.0, PawnControlRotation: True, MouseCursor: False. |
| `CTRL-02` | Top-Down Switch | **PASS** | Mode: `TopDown`, TargetArmLength: 1100.0, FOV: 55.0, PawnControlRotation: False, MouseCursor: True. |
| `CTRL-03` | Click-to-Move Navigation | **PASS** | Valid ground point queries navigation; unreachable destination `[99999, 99999, 99999]` rejected cleanly with zero spurious pathing. |
| `CTRL-04` | Direct Move Interruption | **PASS** | `StopMovement()` explicitly invoked on direct movement input and camera switches (`WRLD-06`). |
| `CTRL-05` | Movement Lock Gating | **PASS** | Both controller and character lock states verified; movement requests rejected during lock; state restored upon unlock (`UI-02`, `UI-07`). |
| `CTRL-06` | Control State Persistence | **PASS** | `FWyrmControlState` captured from controller, modified, restored, and verified matching across controller and character (`SAVE-05`). |
| `CTRL-07` | Third-Person Return | **PASS** | Mode: `ThirdPerson`, TargetArmLength: 450.0, FOV: 80.0, PawnControlRotation: True, MouseCursor: False. |
| `Actor_Cleanup` | Teardown | **PASS** | Test actors cleanly unpossessed and destroyed. |

Commandlet result: **PASS**. The script did not request a PIE session, so this
does not by itself clear the WP-03 gameplay gate. Historical BOOT-01 focused
keyboard/mouse PIE checks remain narrower supporting evidence.
