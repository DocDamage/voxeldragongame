# WP-06: ranged build and progression proof

**Date:** September 16, 2026
**Engine:** UE 5.8.2, CL 56702186
**Status:** **PASS — 22/22 native tests and 6/6 real-PIE proof groups**

## Scope

WP-06 adds a second functional combat kit on the shared humanoid:

- `EWyrmWeaponFamily` selects melee or ranged GAS abilities from the equipped
  main-hand item.
- `AWyrmProjectile` supplies physical ranged travel, hostile-team filtering,
  GAS damage and the supplied arrow mesh.
- Primary and secondary ranged abilities share the existing GAS cost/cooldown
  authority.
- `UWyrmEvadeAbility` supplies tactical movement.
- Level and XP live on `AWyrmCharacter`; Level-driven combat attributes remain
  in `UWyrmAttributeSet` under GAS.
- `UWyrmSaveSubsystem` records Level and current XP.

The fixture does not establish a separate ranger body or animation set. It
proves a ranged gameplay build on the shared created humanoid.

## Supplied assets

The following project assets were imported from the supplied ranger material and
loaded during the PIE proof:

| Asset | PIE result |
|---|---|
| `/Game/WYRMFALL/Items/Weapons/SM_Bow` | Loaded |
| `/Game/WYRMFALL/Items/Weapons/SM_Arrow` | Loaded and assigned to the live projectile |
| `/Game/WYRMFALL/Items/Weapons/T_Bow` | Loaded |
| `/Game/WYRMFALL/Items/Weapons/T_Arrow` | Loaded |

Import receipt: `Saved/Diagnostics/WP06_asset_import.json`.

## Fresh native evidence

Command:

```powershell
py -3.12 tools/wyrm.py ue-test --engine-root "C:/Program Files/UE_5.8" --timeout 1800
```

Result: **22 tests performed; 22 succeeded**. The WP-06 cases are:

- `WYRMFALL.Scaffold.ProgressionXpAndLevelUp`
- `WYRMFALL.Scaffold.ProgressionSaveRoundtrip`
- `WYRMFALL.Scaffold.RangedProjectileDamage`
- `WYRMFALL.Scaffold.WeaponFamilyGatingAndKitSwitch`

Native automation verifies the deterministic formulas and direct impact path; it
does not substitute for the PIE result below.

## Real PIE evidence

Command:

```powershell
py -3.12 tools/run_wp06_pie_proof.py
```

The runner starts `UnrealEditor.exe`, creates an unsaved fixture, explicitly
starts PIE, obtains the game world, and rejects a process/report mismatch.

Receipt: `Saved/Diagnostics/WP06_progression_proof.json`.

| Group | Observed result | Status |
|---|---|---|
| `ASSET.RangerWeapons` | Four supplied assets loaded; projectile component used `SM_Arrow` | PASS |
| `COM-08.Progression` | +60 XP did not level; +50 XP produced Level 2 with 10 XP, 150 required, 140 remaining, Power 23, MaxHealth 108 | PASS |
| `COM-08.KitSwitching` | Sword selected `Melee1H`; bow selected `RangedBow` | PASS |
| `COM-08.RangedCombat` | At 600 cm melee caused 0 damage; the spawned projectile caused 25.8125 damage | PASS |
| `COM-08.EvadeAndCamera` | Top-down and third-person modes selected; evade activated and moved 41.96 cm | PASS |
| `SAVE-01.Progression` | In-memory snapshot restored Level 2 and 10 XP | PASS |

## Implementation files

- `Source/WYRMFALL/Public/Combat/Abilities/WyrmRangedAttackAbility.h`
- `Source/WYRMFALL/Private/Combat/Abilities/WyrmRangedAttackAbility.cpp`
- `Source/WYRMFALL/Public/Combat/Abilities/WyrmEvadeAbility.h`
- `Source/WYRMFALL/Private/Combat/Abilities/WyrmEvadeAbility.cpp`
- `Source/WYRMFALL/Public/Combat/Projectiles/WyrmProjectile.h`
- `Source/WYRMFALL/Private/Combat/Projectiles/WyrmProjectile.cpp`
- `Source/WYRMFALL/Public/Player/WyrmCharacter.h`
- `Source/WYRMFALL/Private/Player/WyrmCharacter.cpp`
- `Source/WYRMFALL/Public/Inventory/WyrmInventoryTypes.h`
- `Source/WYRMFALL/Public/Save/WyrmSaveGame.h`
- `Source/WYRMFALL/Private/Save/WyrmSaveSubsystem.cpp`
- `tools/unreal/verify_wp06_progression_proof.py`
- `tools/run_wp06_pie_proof.py`

## Remaining boundary

- Ranger body/animation presentation and production targeting are NOT_RUN.
- The secondary ranged ability's live target impact was not separately exercised
  in PIE; inherited Focus/cooldown mechanics remain native-test evidence.
- Physical controller, cook, packaged game and production-map behavior are
  NOT_RUN.

The next bounded task remains the unfinished WP-00 readiness acceptance. There
is no current WP-07 task packet, so this report does not authorize a broad
integration-map implementation.
