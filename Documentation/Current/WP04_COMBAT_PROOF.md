# WP-04: First Real Combat Loop Proof

**Date**: 2026-09-15
**Engine**: Unreal Engine 5.8.2 (`C:\Program Files\UE_5.8`)
**Status**: PARTIAL (native and editor-world commandlet cases pass; complete WP-04 PIE NOT_RUN)
**Evidence Receipts**:
- `Saved/Diagnostics/WP04_combat_proof.json`
- `Saved/Automation/Scaffold/index.json` (14/14 native tests pass, including 4 combat foundation tests)

---

## 1. Scope and Authority

In accordance with [AGENTS.md](../../AGENTS.md), [Combat Rules](../DesignPack/docs/systems/COMBAT_AND_NIGHTMARE_ECHOES.md), and [Technical Architecture](../DesignPack/docs/TECHNICAL_ARCHITECTURE.md):
- **GAS is the sole combat authority**: All health, shield, focus, power, armor, damage calculations, costs, cooldowns, and status effects flow through Gameplay Ability System components (`UAbilitySystemComponent`) and attribute sets (`UWyrmAttributeSet`).
- **No secondary combat managers**: No external health or damage managers exist. Damage is applied via instant `UGameplayEffect` modifying `IncomingDamage`, resolved authoritatively in `PostGameplayEffectExecute`.
- **Single-player ARPG execution**: Abilities use `EGameplayAbilityNetExecutionPolicy::ServerOnly` for authoritative, prediction-error-free execution across both live gameplay and headless automation harnesses.

---

## 2. Implementation Overview

### 2.1 Attribute Set & Mitigation Formulas (`Source/WYRMFALL/Public/Combat/WyrmAttributeSet.h`)
- **Canonical Combat Formulas (`COM-01`)**:
  - Raw Damage: `RawDamage = WeaponBase + Power * PowerCoefficient`
  - Physical Mitigation: `MitigationFraction = Armor / (Armor + 50.0 + 10.0 * AttackerLevel)`
  - Mitigated Damage: `MitigatedDamage = RawDamage * (1.0 - MitigationFraction)`
  - Level Scaling: `MaxHealth = 100.0 + 8.0 * (Level - 1)`, `Power = 20.0 + 3.0 * (Level - 1)`
- **Bounds & Drain Enforcement (`COM-02`)**:
  - Shield absorbs damage first before health deduction.
  - Target with `State.Combat.Invulnerable` tag ignores incoming damage completely.
  - Defeated targets (`Health <= 0` or `State.Dead` tag) reject further damage (overkill prevention, clamped at 0.0).
  - Friendly fire check: Members of the same team (`Combat.Team.Player`) do not damage each other.

### 2.2 Melee Attack Abilities (`Source/WYRMFALL/Public/Combat/Abilities/WyrmMeleeAttackAbility.h`)
- **Base Attack (`UWyrmPrimaryMeleeAbility`)**:
  - Basic light attack: WeaponBase 10.0, PowerCoefficient 0.5, 0 Focus cost, 0s cooldown.
  - Hits target using forward sphere sweep (`AttackReach = 150 cm`, `AttackRadius = 50 cm`).
- **Heavy Attack (`UWyrmSecondaryMeleeAbility`) (`COM-04`)**:
  - Secondary attack: WeaponBase 10.0, PowerCoefficient 0.9, 20 Focus cost, 5.0s cooldown.
  - Uses modern UE 5.8 `UTargetTagsGameplayEffectComponent` on dynamic instant GE for cooldown tag application (`Cooldown.Melee.Secondary`).
  - Gated against insufficient Focus, active cooldown, stun, or death.

### 2.3 Authoritative Enemy Architecture (`Source/WYRMFALL/Public/Combat/WyrmEnemyCharacter.h`)
- **Contrasting Roles (`COM-05`)**:
  - `MeleeChaser` (Wolf profile): MaxHealth 60.0, Armor 10.0, Speed 550 cm/s.
  - `RangedSkirmisher` (Skirmisher profile): MaxHealth 50.0, Armor 5.0, Speed 400 cm/s.
- **Status Reactions & Combining (`COM-05`)**:
  - Slow status combines by maximum magnitude (not multiplicative stacking).
  - Boss resistance: Boss enemies (`bIsBoss = true`) resist hard crowd control (stun/root duration reduced to zero; speed remains unhalted).
  - Death handling: On lethal damage, broadcasts `OnEnemyDied`, applies `State.Dead` loose tag, disables movement, and clears capsule collision.

---

## 3. Verification Evidence

### 3.1 Editor-world Commandlet Suite (`tools/unreal/verify_wp04_combat_proof.py`)
Executed via:
```powershell
& "C:\Program Files\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "G:\assets\voxel project\WYRMFALL.uproject" -run=pythonscript -script="G:/assets/voxel project/tools/unreal/verify_wp04_combat_proof.py" -stdout -FullStdOutLogOutput -unattended -nopause -nosplash -nullrhi
```
Result: **Exit Code 0, Overall Commandlet Status PASS**. This invocation runs a
Python commandlet and obtains `get_editor_world()`; it does not start PIE.

| Case ID | Acceptance Target | Metric / Rule Tested | Result | Observed Evidence |
|:---|:---|:---|:---:|:---|
| **COM-01** | Canonical Damage Execution | Raw 20.0 vs Armor 10.0 at Level 1 | **PASS** | 17.143 damage dealt (14.3% mitigation observed, matches canonical formula). Enemy HP: 60.0 -> 42.857. |
| **COM-02** | Bounds & Drain Limits | Shield absorption, invulnerability immunity, dead clamp | **PASS** | 10.0 Shield absorbed completely; remaining 7.14 damage to HP. Invulnerable target took 0.0 damage. Lethal hit triggered `State.Dead` and clamped HP at 0.0 with overkill rejection. |
| **COM-03** | Two-Camera Combat | Mid-encounter perspective toggle | **PASS** | Primary attack dealt identical 17.14 damage in Third-Person and Top-Down modes without state or attack reset. |
| **COM-04** | Focus Cost & Cooldown Commit | Secondary attack resource and cooldown gating | **PASS** | 20.0 Focus deducted (100 -> 80). Immediate repeat activation blocked by cooldown. Activation blocked when Focus < 20. |
| **COM-05** | Status Effects & Enemy Roles | Slow combining, boss resistance, contrasting profiles | **PASS** | Distinct roles verified (Chaser: 60 HP / 10 Armor / 550 Spd; Skirmisher: 50 HP / 5 Armor / 400 Spd). Slow 0.3 -> 385 Spd, Slow 0.5 -> 275 Spd (combined by max 0.5). Boss resisted hard stun. |

### 3.2 Native Automation Tests (`Source/WYRMFALL/Private/Tests/WyrmScaffoldTests.cpp`)
All 14 source-declared automation tests pass:
1. `WYRMFALL.Scaffold.ActorClassesExist`
2. `WYRMFALL.Scaffold.AttributeSetAttributes`
3. `WYRMFALL.Scaffold.GameplayEffectClassesExist`
4. `WYRMFALL.Scaffold.TagRegistration`
5. `WYRMFALL.Scaffold.TerrainNumericBoundaries`
6. `WYRMFALL.Scaffold.TerrainRequestValidation`
7. `WYRMFALL.Scaffold.MutableTypesAndParameters`
8. `WYRMFALL.Scaffold.MutableDescriptorRoundtrip`
9. `WYRMFALL.Scaffold.EquipmentAttachmentSockets`
10. `WYRMFALL.Scaffold.SharedControlFoundation`
11. `WYRMFALL.Scaffold.CombatCanonicalDamage`
12. `WYRMFALL.Scaffold.CombatBoundsAndDrain`
13. `WYRMFALL.Scaffold.CombatCostAndCooldown`
14. `WYRMFALL.Scaffold.CombatEnemyRolesAndStatus`

---

## 4. Conclusion & Next Task

WP-04 source compiles and its native and editor-world cases pass. WP-06 adds
supporting real-PIE evidence for a live ranged hit, melee range gating, evade and
camera selection, but the complete COM-01 through COM-05 suite has not been
repeated in PIE. Keep the package PARTIAL until that focused gameplay proof runs.
