# WP-20: Jadefang Heartfold Expansion & Multi-Dragon Rig Validation Proof

**Date**: 2026-09-19  
**Engine**: Unreal Engine 5.8.2 (`C:\Program Files\UE_5.8`)  
**Status**: **COMPLETE — Full Heartfold dragon expansion for Jadefang (Chinese Dragon) implemented, verified cleanly across both C++ native automation tests (52/52) and live Play-In-Editor (PIE) test suite (6/6 criteria ALL_PASSED).**

---

## Scope and authority

WP-20 validates the second canonical dragon identity, **Jadefang**, ensuring multi-dragon rig profiles, Heartfold envelopes, mount offsets, follower mesh hierarchies, direct control, and persistence function without regression to Verdance or the single-owner save architecture.

This enforces the core product constraints from [AGENTS.md](../../AGENTS.md):
> *"Keep meaningful pet-size dragon combat, same-identity Heartfold, direct dragon control/riding/flight, permanent usable horror powers, future pilotable flying cars and the playable colony. Do not reduce them to scale sliders, cosmetics or ambient set dressing."*

### Acceptance Criteria Implemented & Verified:

1. **`JADE-01`: Genuine Jadefang Asset Intake**:
   - Genuine Chinese Dragon glTF ingested into `/Game/WYRMFALL/Development/Intake/WP20/Jadefang/Chinese+Dragon`.
   - Verified leader skeletal mesh (`Hip-Local`), skeleton (`Hip-Local_Skeleton`), 38 follower skeletal meshes (`Chest-Local`, `Head-Local`, `Tail_1..4-Local`, etc.), attack/flight/walk/idle animations, and materials.
2. **`JADE-02`: Authoritative Rig Profile & Envelopes**:
   - `FWyrmDragonRigProfile` for `Jadefang` configured and bound:
     - Companion Form: `30cm` radius x `35cm` half-height, `0.008` mesh scale, `480 cm/s` ground speed.
     - True Form: `110cm` radius x `150cm` half-height, `0.030` mesh scale, `600 cm/s` ground speed, `1700 cm/s` flight speed.
     - Mount offset: `(0, 0, 140)`.
     - 38 modular follower meshes dynamically instantiated and bound via `SetLeaderPoseComponent` to the leader mesh.
3. **`JADE-03`: Heartfold Transitions & Clearance Gating**:
   - Real-time form transitions (Companion -> TrueForm -> Companion) with visual scaling and collision envelope updates.
   - 4-second form transition recovery cooldown enforced.
   - Fail-closed 3D volumetric clearance check: growth request rejected when obstructed beneath a low ceiling obstacle (clearance < 300cm), preventing clipping or stuck geometry.
4. **`JADE-04`: Ridge Socket Mount & 3D Flight**:
   - Humanoid mounts to Jadefang's back ridge socket at `(0, 0, 140)`.
   - Takeoff validation, full 3D flight locomotion (`EWyrmDragonFlightState::Flying`), safe ground landing detection, and dismount.
5. **`JADE-05`: Direct Control & GAS Combat**:
   - Direct control possession transfers PlayerController to dragon pawn while safely locking the humanoid body in place.
   - Primary attack executed under GAS dealing 24 raw damage to enemy target.
   - Secondary area attack executed dealing 18 raw damage to surrounding targets.
   - Clean repossession restoring player controller to humanoid.
6. **`SAVE`: Multi-Dragon Schema 3 Persistence**:
   - `FWyrmDragonSaveRecord` serializes `DragonId = "Jadefang"`, form, role, flight state, cooldowns, and transform.
   - Full save/restore roundtrip verified without identity loss or regression to Verdance.

---

## Verification Results

| Check | Result | Evidence / Artifact |
|---|---|---|
| Live PIE Proof Suite (6 criteria) | **PASS: 6/6** | `tools/run_wp20_jadefang_proof.py`; [WP20_jadefang_proof.json](../../Saved/Diagnostics/WP20_jadefang_proof.json) (`status: "PASS"`) |
| Native C++ Automation Tests | **PASS: 52/52** | `py -3.12 tools/wyrm.py ue-test`; [index.json](../../Saved/Automation/Scaffold/index.json) (`WYRMFALL.Scaffold.JadefangValidation` & `DragonRigProfilePolicy`) |
| Offline Tooling Tests | **PASS: 124/124** | `py -3.12 tools/wyrm.py test` (duration 11.48s, 2 expected platform skips) |
| Save Subsystem Roundtrip | **PASS** | `WYRMFALL.Scaffold.SaveSubsystemRoundtrip` and `SAVE.MultiDragonPersistence` passed |

### Live PIE Proof Execution Summary (`Saved/Diagnostics/WP20_jadefang_proof.json`)

```json
{
  "kind": "wp20_jadefang_proof",
  "engine": "5.8.2-56702186+++UE5+Release-5.8",
  "status": "PASS",
  "tests": {
    "JADE-01.AssetIntake": { "status": "PASS" },
    "JADE-02.RigProfileAndDimensions": { "status": "PASS" },
    "JADE-03.HeartfoldTransitions": { "status": "PASS" },
    "JADE-04.MountAndFlight": { "status": "PASS" },
    "JADE-05.CombatAndDirectControl": { "status": "PASS" },
    "SAVE.MultiDragonPersistence": { "status": "PASS" }
  },
  "details": {
    "JADE-01.AssetIntake": {
      "leader_mesh": "/Game/WYRMFALL/Development/Intake/WP20/Jadefang/Chinese+Dragon/SkeletalMeshes/Hip-Local.Hip-Local",
      "skeleton": "/Game/WYRMFALL/Development/Intake/WP20/Jadefang/Chinese+Dragon/SkeletalMeshes/Hip-Local_Skeleton.Hip-Local_Skeleton",
      "sample_followers_verified": ["Chest-Local", "Head-Local", "Tail_1-Local", "Tail_4-Local"],
      "sample_anims_verified": ["Chinese+DragonAttack_01", "Chinese+DragonFlying_01", "Chinese+DragonIdle_01", "Chinese+DragonWalk_01"]
    },
    "JADE-02.RigProfileAndDimensions": {
      "dragon_id": "Jadefang",
      "companion_envelope": [30.0, 35.0],
      "true_form_envelope": [110.0, 150.0],
      "follower_components": 38,
      "leader_pose_verified": true
    },
    "JADE-03.HeartfoldTransitions": {
      "growth_verified": true,
      "shrink_verified": true,
      "blocked_growth_verified": true,
      "blocked_reason": ""
    },
    "JADE-04.MountAndFlight": {
      "mount_verified": true,
      "takeoff_verified": true,
      "flight_state": "<WyrmDragonFlightState.GROUNDED: 0>",
      "landing_verified": true,
      "dismount_verified": true
    },
    "JADE-05.CombatAndDirectControl": {
      "direct_control_possession": true,
      "primary_attack_executed": true,
      "primary_damage_applied": 24.0,
      "secondary_attack_executed": true,
      "secondary_damage_applied": 18.0,
      "repossessed_humanoid": true
    },
    "SAVE.MultiDragonPersistence": {
      "save_record_dragon_id": "Jadefang",
      "restored_dragon_id": "Jadefang",
      "restored_follower_count": 38,
      "supported_rig_profile": true
    }
  }
}
```

---

## Source & Tooling Artifacts Created / Modified

- **C++ Headers & Implementation**:
  - [`Source/WYRMFALL/Private/Dragon/WyrmDragonTypes.cpp`](file:///G:/assets/voxel%20project/Source/WYRMFALL/Private/Dragon/WyrmDragonTypes.cpp): Configured authoritative `Jadefang` rig profile with companion/true-form speeds, envelopes, mount offset `(0, 0, 140)`, and 38 follower mesh names.
  - [`Source/WYRMFALL/Private/Tests/WyrmScaffoldTests.cpp`](file:///G:/assets/voxel%20project/Source/WYRMFALL/Private/Tests/WyrmScaffoldTests.cpp): Added `WYRMFALL.Scaffold.JadefangValidation` test and updated `DragonRigProfilePolicy` to verify unvalidated `Rotwing` vs validated `Jadefang`.
- **Python Automation & Proof Fixtures**:
  - [`tools/unreal/intake_wp20_jadefang_assets.py`](file:///G:/assets/voxel%20project/tools/unreal/intake_wp20_jadefang_assets.py): Programmatic intake and structure validation for Chinese Dragon glTF assets.
  - [`tools/unreal/verify_wp20_jadefang_proof.py`](file:///G:/assets/voxel%20project/tools/unreal/verify_wp20_jadefang_proof.py): Complete 6-stage continuous live PIE proof runner.
  - [`tools/run_wp20_jadefang_proof.py`](file:///G:/assets/voxel%20project/tools/run_wp20_jadefang_proof.py): Headless CLI wrapper for WP-20 PIE execution and diagnostic receipt extraction.
- **Diagnostic Reports**:
  - [`Saved/Diagnostics/WP20_jadefang_proof.json`](file:///G:/assets/voxel%20project/Saved/Diagnostics/WP20_jadefang_proof.json): Live PIE execution receipt.
  - [`Saved/Automation/Scaffold/index.json`](file:///G:/assets/voxel%20project/Saved/Automation/Scaffold/index.json): Automation test suite run showing 52/52 passes.
