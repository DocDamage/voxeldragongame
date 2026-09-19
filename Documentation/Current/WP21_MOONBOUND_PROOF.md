# WP-21: Moonbound Transformation Proof (ECHO-07..09, SAVE-11)

**Date**: 2026-09-19  
**Engine**: Unreal Engine 5.8.2 (`C:\Program Files\UE_5.8`)  
**Status**: **COMPLETE — Full Moonbound Form implementation verified cleanly across C++ native automation tests (53/53), offline verification tests (124/124), live PIE proof suite (4/4 criteria ALL_PASSED), and full regression across WP-15, WP-16, and WP-20.**

---

## Scope and authority

WP-21 implements and validates the second canonical Horror Echo power, **Moonbound Form**, and the associated encounter with **Ser Corvyn** at Silent Landing / Region 01.

This enforces the core product constraints from [AGENTS.md](../../AGENTS.md):
> *"Keep meaningful pet-size dragon combat, same-identity Heartfold, direct dragon control/riding/flight, permanent usable horror powers, future pilotable flying cars and the playable colony. Do not reduce them to scale sliders, cosmetics or ambient set dressing."*
> *"Mutable remains the creator. GAS remains combat authority. ... Do not introduce a second health, inventory, equipment, interaction, terrain, water, building or save owner."*

### Acceptance Criteria Implemented & Verified:

1. **`ECHO-07.MercyParity` — Ser Corvyn Resolution Parity**:
   - Authored encounter profile for Ser Corvyn (`AWyrmCorvynCharacter`): 750 HP, 12 Armor, 22 Power, with beast strike (15 dmg) and beast pounce (25 dmg).
   - Dual resolution pathways:
     - **Hostile Defeat**: Combat reduction to 0 HP records `corvyn.defeated_hostile`, grants `Unlock.Echo.MoonboundForm`, equips the echo, and delivers `Item_CorvynRelic`.
     - **Mercy / Cure Path**: Trust/cure mechanism records `corvyn.cured`, grants identical `Unlock.Echo.MoonboundForm`, equips the echo, and delivers `Item_CorvynRelic` with zero penalty for mercy.
   - Strict idempotency and full-bag safety: if inventory is full, loot is preserved in Corvyn's preserved storage (`GetPreservedLoot()`) for later retrieval without loss.
2. **`ECHO-08.ActualBeast` — Genuine Beast Form & Presentation**:
   - Transforms player character into a genuine wolf/beast mesh (`/Game/WYRMFALL/Development/Intake/WP00/Review/Wolf/wolf1`).
   - Humanoid mesh stowed/hidden, beast mesh instantiated and visible.
   - 700 cm/s beast walk speed (up from 450 cm/s base).
   - Passive stats (health, armor, power) fully retained; active humanoid weapon attacks suppressed.
   - Authoritative beast combat kit under GAS:
     - **Beast Claw**: Primary basic attack dealing 25 raw damage.
     - **Beast Pounce**: Secondary heavy attack (15 Focus cost, 4s cooldown) dealing 35 raw damage with obstacle collision sweep.
   - Clean expiry/deactivation restores the original created humanoid without appearance mutation or loss of Mutable state.
3. **`ECHO-09.BeastReturnBlockage` — Low-Ceiling Obstacle Clearance Gating**:
   - Fail-closed 3D volumetric clearance check before restoring humanoid form (evaluating humanoid capsule envelope with floor clearance tolerance).
   - When obstructed under low ceiling (<192 cm clearance), humanoid return is blocked and character enters `bMoonboundReturnPending` state.
   - Beast attacks are suppressed during return-pending state to prevent infinite combat extensions.
   - Safe resolution via `ResolveMoonboundReturnBlockage()` teleports player to `LastSafeHumanoidLocation` and safely restores the humanoid form.
4. **`SAVE-11.BeastFormSave` — Schema 3 Persistence**:
   - `UWyrmSaveGame` Schema 3 serializes `bMoonboundActive`, `MoonboundRemainingDuration`, `MoonboundRemainingCooldown`, `bMoonboundReturnPending`, and `LastSafeHumanoidLocation`.
   - Full save and load roundtrip restores active beast presentation, remaining timers, cooldowns, and return blockage state without identity distortion.

---

## Verification Results

| Check | Result | Evidence / Artifact |
|---|---|---|
| Live PIE Proof Suite (4 criteria) | **PASS: 4/4** | `tools/run_wp21_moonbound_proof.py`; [WP21_moonbound_proof.json](../../Saved/Diagnostics/WP21_moonbound_proof.json) (`status: "ALL_PASSED"`) |
| Native C++ Automation Tests | **PASS: 53/53** | `py -3.12 tools/wyrm.py ue-test`; [index.json](../../Saved/Automation/Scaffold/index.json) (`WYRMFALL.Scaffold.MoonboundValidation`) |
| Offline Tooling Tests | **PASS: 124/124** | `py -3.12 tools/wyrm.py test` |
| WP-15 Echo Regression | **PASS: 7/7** | `tools/run_wp15_echo_proof.py` (all 7 acceptance criteria passed) |
| WP-20 Jadefang Regression | **PASS: 6/6** | `tools/run_wp20_jadefang_proof.py` (all criteria passed) |
| WP-16 Connected Slice Regression | **PASS: 8/8** | `tools/run_wp16_connected_slice_proof.py` (all 8 connected slice criteria passed) |

### Live PIE Proof Execution Summary (`Saved/Diagnostics/WP21_moonbound_proof.json`)

```json
{
  "kind": "wp21_moonbound_proof",
  "engine": "5.8.2-56702186+++UE5+Release-5.8",
  "map": "/Game/WYRMFALL/World/Regions/L_Region01",
  "status": "ALL_PASSED",
  "tests": {
    "ECHO-07.MercyParity": { "status": "PASS" },
    "ECHO-08.ActualBeast": { "status": "PASS" },
    "ECHO-09.BeastReturnBlockage": { "status": "PASS" },
    "SAVE-11.BeastFormSave": { "status": "PASS" }
  },
  "details": {
    "ECHO-07.MercyParity": {
      "hostile": {
        "fact": "corvyn.defeated_hostile",
        "echo_unlocked": true,
        "loot_delivered": true,
        "idempotent": true
      },
      "mercy": {
        "fact": "corvyn.cured",
        "echo_unlocked": true,
        "loot_delivered": true,
        "parity_verified": true
      },
      "full_bag_safe": true,
      "parity_confirmed": true
    },
    "ECHO-08.ActualBeast": {
      "beast_mesh_visible": true,
      "human_mesh_stowed": true,
      "beast_speed": 700.0,
      "claw_damage": 25.0,
      "pounce_damage": 35.0,
      "humanoid_restored": true
    },
    "ECHO-09.BeastReturnBlockage": {
      "blocked_clearance_detected": true,
      "return_pending_active": true,
      "combat_extension_suppressed": true,
      "safe_return_resolved": true
    },
    "SAVE-11.BeastFormSave": {
      "save_succeeded": true,
      "load_succeeded": true,
      "duration_preserved": 8.5,
      "cooldown_preserved": 35.0,
      "presentation_restored": true,
      "clean_expiry_return": true
    }
  }
}
```

---

## Changed Files & Integration Points

- `Config/DefaultGameplayTags.ini`: Added `Ability.Echo.MoonboundForm`, `Cooldown.Echo.MoonboundForm`, `State.Combat.MoonboundForm`, `Unlock.Echo.MoonboundForm`, `Ability.Combat.BeastClaw`, `Ability.Combat.BeastPounce`, `Cooldown.Combat.BeastPounce`.
- `Source/WYRMFALL/Public/Combat/Abilities/WyrmMoonboundFormAbility.h` & `.cpp`: GAS ability with 40 Focus cost, 12s duration, 35s cooldown, and precondition verification.
- `Source/WYRMFALL/Public/Combat/Abilities/WyrmBeastAttackAbilities.h` & `.cpp`: `UWyrmBeastClawAbility` (25 raw dmg) and `UWyrmBeastPounceAbility` (15 Focus, 4s CD, 35 raw dmg).
- `Source/WYRMFALL/Public/Combat/WyrmCorvynCharacter.h` & `.cpp`: Ser Corvyn boss actor with dual hostile/mercy resolution paths, beast strike/pounce attacks, and preserved loot safety.
- `Source/WYRMFALL/Public/Player/WyrmCharacter.h` & `.cpp`: `BeastMeshComponent`, `ActivateMoonboundForm`, `DeactivateMoonboundForm` with 3D volumetric clearance check, `ResolveMoonboundReturnBlockage`, beast attack routing, 700 cm/s speed.
- `Source/WYRMFALL/Public/Save/WyrmSaveGame.h` & `Source/WYRMFALL/Private/Save/WyrmSaveSubsystem.cpp`: Schema 3 persistence fields for Moonbound Form state.
- `Source/WYRMFALL/Public/Region/WyrmRegion01Subsystem.h` & `.cpp`: Tracking for `corvyn.defeated_hostile`, `corvyn.cured`, and `echo.moonbound_form`.
- `Source/WYRMFALL/Private/Tests/WyrmScaffoldTests.cpp`: `FWyrmMoonboundScaffoldTest` verifying tags, stats, and ability specs.
- `tools/unreal/verify_wp21_moonbound_proof.py` & `tools/run_wp21_moonbound_proof.py`: Automated live PIE test suite for WP-21.
