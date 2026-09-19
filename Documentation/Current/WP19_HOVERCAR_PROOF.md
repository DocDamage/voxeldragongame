# WP-19: Pilotable Zenith Civilian Hovercar Proof (VEH-01..09)

**Date**: 2026-09-19  
**Engine**: Unreal Engine 5.8.2 (`C:\Program Files\UE_5.8`)  
**Status**: **COMPLETE — Full pilotable civilian hovercar gameplay slice implemented, integrated into Player Controller and Save Subsystem Schema 3, and verified cleanly across both C++ native automation tests (51/51) and live Play-In-Editor (PIE) test suite (9/9 criteria ALL_PASSED).**

---

## Scope and authority

WP-19 resolves the civilian flight vehicle milestone specified in [HANDOFF.md](HANDOFF.md) and enforces the core product constraint from [AGENTS.md](../../AGENTS.md):
> *"Keep meaningful pet-size dragon combat, same-identity Heartfold, direct dragon control/riding/flight, permanent usable horror powers, future pilotable flying cars and the playable colony. Do not reduce them to scale sliders, cosmetics or ambient set dressing."*

### Acceptance Criteria Implemented & Verified:
1. **`VEH-01`: Real Civilian Hovercar Entry**:
   - `AWyrmHovercar` spawned in the world with real voxel art (`SM_ZenithHovercar`).
   - Nearby humanoid player interacts via `CanEnter` (distance threshold <= 400cm, unblocked, not disabled).
   - `AWyrmPlayerController::EnterHovercar` transfers control to hovercar pawn, hides humanoid mesh, and locks humanoid movement while retaining driver identity.
2. **`VEH-02`: 3D Flight Locomotion & Camera Modes**:
   - 3D flight physics with forward/backward thrust, yaw steering with banking roll assist, altitude hold, and vertical climb/dive.
   - Dual-camera toggle support: Third-person (650cm arm, pitch -15°) and Top-down (1200cm arm, pitch -60°).
3. **`VEH-03`: Swept Obstacle Collision & Landing Validation**:
   - Root collision box extents calibrated to 200cm x 80cm x 70cm (vehicle size: 400cm x 160cm x 140cm).
   - Dynamic swept collision against terrain and world static obstacles prevents clipping and wall penetration.
   - `CanLand` validates landing zone clearance and surface slope.
4. **`VEH-04`: Blocked & In-Flight Exit Rejection**:
   - `CanExit` rejects dismount while airborne (`Altitude > 120cm` or state is `Cruising`) with `"Cannot exit hovercar while airborne; land vehicle first."`.
   - `CanExit` validates left and right door clearances against obstacles.
   - Upon safe landing, `ExitHovercar` restores player controller possession to humanoid, unlocks movement, and places character safely beside the vehicle.
5. **`VEH-05`: Companion Passenger Heartfold Boarding**:
   - Companion dragon boarding respects form constraints:
     - `TrueForm` dragon is rejected with `"Dragon too large to fit passenger seat; Heartfold to compact form first."`.
     - `CompanionForm` dragon boards passenger seat, attaches to hovercar hierarchy, and unboards safely to the ground upon dismount.
6. **`VEH-06`: Disablement & Depot Recovery**:
   - Hovercar has 250 HP base health and receives damage via `ApplyDamage`.
   - At 0 HP, vehicle transitions to `Disabled` state, disabling thrusters and rejecting entry.
   - `RecoverToDepot` repairs vehicle to 250 HP, resets state to `Parked`, relocates to depot coordinates, and stages companion safely at depot.
7. **`VEH-07`: Schema 3 Save Persistence**:
   - Save system bumped to `CurrentSchemaVersion = 3`.
   - `FWyrmHovercarSaveRecord` serializes hovercar transform, health, mecha upgrade state, driver occupancy, and companion boarding.
   - `UWyrmSaveSubsystem` roundtrips hovercar snapshot to disk and restores state cleanly.
8. **`VEH-08`: Traffic Layer Separation**:
   - Cruising altitude and vertical speed separation (600 cm/s vertical rate, 1500 cm/s base cruise speed) separates civilian flight layer from pedestrian ground movement and dragon flight corridors.
9. **`VEH-09`: Mecha Progression Boost Hook**:
   - `bMechaCircuitUnlocked` provides a +500 cm/s cruise speed boost (1500 -> 2000 cm/s), unlocking advanced propulsion via colony engineering progression.

---

## Real Supplied Art Ingestion

Per project constraints, no diagnostic primitives or AI-generated meshes are used in production:
- Source asset: `assets and old docs/voxel/tiny cars/Tiny_Cars.zip` (`car1.vox` / `car1.obj`).
- Extracted and normalized into:
  - `Saved/Staging/WP19/SM_ZenithHovercar.obj`
  - `Saved/Staging/WP19/T_ZenithHovercar.png`
- Unreal Engine Assets created and assigned:
  - Static Mesh: `/Game/WYRMFALL/Vehicles/SM_ZenithHovercar`
  - Texture: `/Game/WYRMFALL/Vehicles/T_ZenithHovercar`
  - Material: `/Game/WYRMFALL/Vehicles/M_ZenithHovercar`
- Intake receipt written to `Saved/Diagnostics/WP19_hovercar_asset_intake.json` (`status: "SUCCESS"`).

---

## Verification Results

| Check | Result | Evidence / Artifact |
|---|---|---|
| Live PIE Proof Suite (9 criteria) | **PASS: 9/9** | `tools/run_wp19_hovercar_proof.py`; [WP19_hovercar_proof.json](../../Saved/Diagnostics/WP19_hovercar_proof.json) (`status: "ALL_PASSED"`) |
| Native C++ Automation Tests | **PASS: 51/51** | `py -3.12 tools/wyrm.py ue-test`; [index.json](../../Saved/Automation/Scaffold/index.json) |
| Offline Tooling Tests | **PASS: 124/124** | `py -3.12 tools/wyrm.py test` (duration 9.36s, 2 expected platform skips) |
| Save Subsystem Schema 3 | **PASS** | `UWyrmSaveSubsystem` roundtrip verified in both native test and live PIE |

### Live PIE Proof Execution Summary (`Saved/Diagnostics/WP19_hovercar_proof.json`)
```json
{
  "kind": "wp19_hovercar_proof",
  "engine": "5.8.2-56702186+++UE5+Release-5.8",
  "map": "/Game/WYRMFALL/World/Regions/L_Region01",
  "status": "ALL_PASSED",
  "tests": {
    "VEH-01.Entry": { "status": "PASS" },
    "VEH-02.FlightLocomotion": { "status": "PASS" },
    "VEH-03.SweptCollision": { "status": "PASS" },
    "VEH-04.ExitValidation": { "status": "PASS" },
    "VEH-05.CompanionBoarding": { "status": "PASS" },
    "VEH-06.DamageAndDepot": { "status": "PASS" },
    "VEH-07.Persistence": { "status": "PASS" },
    "VEH-08.TrafficSeparation": { "status": "PASS" },
    "VEH-09.MechaProgression": { "status": "PASS" }
  }
}
```

---

## Source & Tooling Artifacts Created / Modified

- **C++ Headers & Implementation**:
  - [`Source/WYRMFALL/Public/Vehicles/WyrmVehicleTypes.h`](file:///G:/assets/voxel%20project/Source/WYRMFALL/Public/Vehicles/WyrmVehicleTypes.h): `EWyrmHovercarState`, `FWyrmHovercarSaveRecord`.
  - [`Source/WYRMFALL/Public/Vehicles/WyrmHovercar.h`](file:///G:/assets/voxel%20project/Source/WYRMFALL/Public/Vehicles/WyrmHovercar.h): Vehicle pawn with flight physics, occupancy, companion boarding, camera modes, damage, and depot recovery.
  - [`Source/WYRMFALL/Private/Vehicles/WyrmHovercar.cpp`](file:///G:/assets/voxel%20project/Source/WYRMFALL/Private/Vehicles/WyrmHovercar.cpp): Vehicle implementation.
  - [`Source/WYRMFALL/Public/Player/WyrmPlayerController.h`](file:///G:/assets/voxel%20project/Source/WYRMFALL/Public/Player/WyrmPlayerController.h): Added `EnterHovercar`, `ExitHovercar`.
  - [`Source/WYRMFALL/Private/Player/WyrmPlayerController.cpp`](file:///G:/assets/voxel%20project/Source/WYRMFALL/Private/Player/WyrmPlayerController.cpp): Input routing and possession for hovercar.
  - [`Source/WYRMFALL/Public/Save/WyrmSaveGame.h`](file:///G:/assets/voxel%20project/Source/WYRMFALL/Public/Save/WyrmSaveGame.h): Schema 3 bump and `HovercarRecord`.
  - [`Source/WYRMFALL/Private/Save/WyrmSaveSubsystem.cpp`](file:///G:/assets/voxel%20project/Source/WYRMFALL/Private/Save/WyrmSaveSubsystem.cpp): Capture and restore of active hovercar state.
  - [`Source/WYRMFALL/Private/Tests/WyrmScaffoldTests.cpp`](file:///G:/assets/voxel%20project/Source/WYRMFALL/Private/Tests/WyrmScaffoldTests.cpp): 4 new native automation tests (`HovercarLocomotion`, `HovercarOccupancyAndCompanion`, `HovercarDamageAndDepot`, `HovercarSaveSchema3`).
- **Python Verification & Asset Intake**:
  - `tools/unreal/import_wp19_hovercar_assets.py`: Asset intake automation for hovercar voxel model.
  - `tools/unreal/verify_wp19_hovercar_proof.py`: 9-stage live PIE verification suite.
  - `tools/run_wp19_hovercar_proof.py`: Headless/unattended runner for live PIE proof.
