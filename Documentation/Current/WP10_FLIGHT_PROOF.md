# WP-10: Green Dragon Riding, Flight Locomotion, Obstacle Collision & Mounted Persistence Verification Proof

**Date**: 2026-09-17
**Engine**: Unreal Engine 5.8.2 (`C:\Program Files\UE_5.8`)
**Status**: **PASS in Real PIE & Native Automation**
**Evidence Receipts**:
- `Saved/Diagnostics/WP10_flight_proof.json` (5/5 PIE test groups PASS)
- `Saved/Automation/Scaffold/index.json` (41/41 native automation tests PASS)

---

## 1. Scope & Authority Boundaries

In strict compliance with [AGENTS.md](../../AGENTS.md) and single-owner architectural constraints:
1. **Dragon Riding Authority (`AWyrmDragonCharacter` / `DRG-05`)**:
   - The original created humanoid (`AWyrmCharacter`) attaches directly to the dragon mount socket offset (`FVector(0, 0, 160)`). No cosmetic duplicate or cloned rider actor is created.
   - Mounting in compact/companion form is rejected with clear feedback (`"Compact dragon cannot be mounted; True Form required"`).
   - In True Form, mounting locks the humanoid's movement, attaches the actor, transfers player controller possession to the dragon, and adjusts camera arm length to 1100cm.
   - Dismounting is rejected while airborne. On ground, dismounting places the humanoid safely on adjacent supported terrain, restores humanoid collision and locomotion, and re-possesses the humanoid.
2. **3D Flight Locomotion & Collision Authority (`AWyrmDragonCharacter` / `DRG-06`)**:
   - Manages takeoff via overhead box sweep verifying wing clearance (`WingSpanSweepRadius=320.f`, `TakeoffClearanceHeight=300.f`).
   - Transitions `CharacterMovement` to `MOVE_Flying` with `MaxFlySpeed=1600.f` and flight launch impulse.
   - Landing checks for supported ground beneath within `LandingSearchDistance=1200.f` and enforces maximum slope limits (`MaxLandingSlopeAngle=40.0 deg`).
   - Supports dual flight cameras: Third-person chase camera (arm length 1100cm) and top-down tactical flight camera (arm length 1800cm, -65 deg pitch).
3. **Mounted Defeat Emergency Ground Recovery (`AWyrmDragonCharacter` / `DRG-07`)**:
   - When a mounted dragon is defeated or incapacitated in flight, the system initiates emergency ground recovery:
     - Traces down to find the nearest valid ground anchor beneath the flight position.
     - Detaches the humanoid and places them safely on the ground anchor, preventing fallen/dropped riders stranded in midair.
     - Unlocks humanoid locomotion and restores player controller possession to the humanoid.
     - Transitions dragon role to `Recovering` and forces flight state to `Grounded`. Mounting a recovering dragon is rejected.
4. **Hub Companion Recovery (`AWyrmDragonCharacter` / `DRG-14`)**:
   - Named hub recovery interaction (`RecoverCompanion`) restores a defeated/recovering companion.
   - Restores dragon role to `AlliedCompanion` and flight state to `Grounded`.
   - Restores GAS attributes to full 420.0 Max Health and 100.0 Focus.
5. **Mounted Flight Persistence Authority (`UWyrmSaveSubsystem` / `SAVE-09`)**:
   - Serializes dragon flight state (`EWyrmDragonFlightState`), mounted rider status (`bIsRiderMounted`), world transform, and safe ground anchor.
   - When loading an airborne mounted snapshot, accurately restores dragon altitude, flight mode, rider attachment, and player possession without actor duplication.

---

## 2. Implemented Components

### 2.1 C++ Types & Interfaces
- `Source/WYRMFALL/Public/Dragon/WyrmDragonTypes.h`:
  - `EWyrmDragonFlightState`: `Grounded`, `TakingOff`, `Flying`, `Landing`.
  - Extended `FWyrmDragonSaveRecord`: Added `bIsRiderMounted`, `FlightState`, and `SafeGroundAnchor`.
- `Source/WYRMFALL/Public/Dragon/WyrmDragonCharacter.h` & `WyrmDragonCharacter.cpp`:
  - Mounting: `CanMount`, `MountHumanoid`, `CanDismount`, `DismountHumanoid`, `IsRiderMounted`, `GetMountedRider`.
  - Flight Locomotion: `CanTakeOff`, `TakeOff`, `CanLand`, `Land`, `IsInFlight`, `GetFlightState`, `GetSafeGroundAnchor`.
  - Camera & Defeat: `SetFlightCameraMode`, `HandleMountedDefeat`, `RecoverCompanion`.
  - Persistence: `BuildSaveRecord` and `RestoreFromSaveRecord` extended for riding and flight state.
- `Source/WYRMFALL/Public/Player/WyrmPlayerController.h` & `WyrmPlayerController.cpp`:
  - `MountDragon`, `DismountDragon`, `TakeOffDragon`, `LandDragon`.
  - `JumpPressed`: Initiates dragon takeoff when grounded, or landing when airborne.
  - `SwitchCamera`: Toggles dragon flight camera between third-person (1100cm) and top-down (1800cm, -65 deg).

---

## 3. Verification Scenarios & Results

All 5 scenarios passed in real PIE via `py -3.12 tools/run_wp10_pie_proof.py`:

| Test Identifier | Acceptance Requirement | Result | Verified Details |
|---|---|---|---|
| **DRG-05.MountAndDismount** | Compact mount rejected; True Form mount attaches humanoid to socket (0, 0, 160) without duplicate; ground dismount restores humanoid | **PASS** | Companion Form mount rejected; True Form mount attached original humanoid; PC possessed dragon; ground dismount detached humanoid and restored player controller |
| **DRG-06.FlightLocomotionAndCamera** | 3D flight (`MOVE_Flying`, max fly speed 1600); in-flight dismount rejected; dual camera toggle (1100cm / 1800cm); safe ground landing | **PASS** | Takeoff initiated 3D flight; in-flight dismount rejected; camera boom toggled between 1800cm and 1100cm; landing returned to `MOVE_Walking` and Grounded |
| **DRG-07.MountedDefeatEmergencyRecovery** | Defeat in midair safely recovers rider to ground anchor beneath; prevents fallen riders; sets role to `Recovering` | **PASS** | Dragon at Z=1500 defeated; rider recovered to safe ground at Z=90; humanoid unlocked; PC possessed humanoid; dragon role set to `Recovering` |
| **DRG-14.HubCompanionRecovery** | Hub recovery restores recovering companion to `AlliedCompanion`, resets flight state, and restores 420.0 HP / 100.0 Focus | **PASS** | `RecoverCompanion` restored role to `AlliedCompanion`, health to 420.0, focus to 100.0, and enabled mounting again |
| **SAVE-09.MountedFlightSaveAndRecovery** | Airborne mounted save roundtrip; restores dragon Z=800, flight state Flying, rider mounted, and controller possession | **PASS** | Snapshot saved at Z=800 with rider mounted in flight; applied snapshot; dragon restored to Z=800, flight state Flying, rider re-mounted, and PC possessed dragon |

---

## 4. Native Automation Suite (41/41 PASS)

Run via `py -3.12 tools/wyrm.py ue-test`:
- `WYRMFALL.Scaffold.DragonMountAndDismount`: Validates compact rejection, True Form mount, in-flight dismount rejection, and ground dismount.
- `WYRMFALL.Scaffold.DragonFlightTakeoffAndLanding`: Validates overhead clearance, 3D flight speed, ground slope verification, and landing.
- `WYRMFALL.Scaffold.DragonMountedDefeatAndHubRecovery`: Validates emergency ground recovery on defeat and hub companion recovery.
- `WYRMFALL.Scaffold.DragonMountedFlightSaveAndObstructedRecovery`: Validates airborne mounted serialization, state restore, and re-possession.

---

## 5. Verification Commands

```powershell
# Compile editor target
py -3.12 tools/wyrm.py build

# Run native automation test suite (41/41 tests)
py -3.12 tools/wyrm.py ue-test

# Run full WP-10 Play-In-Editor proof
py -3.12 tools/run_wp10_pie_proof.py
```
