# WP-11: Green Dragon Heartfold and Compact Behavior Verification Proof

**Date**: 2026-09-17
**Engine**: Unreal Engine 5.8.2 (`C:\Program Files\UE_5.8`)
**Status**: **PASS for the Green Dragon/Verdance rig in real PIE and native automation**

## Scope and authority

This proof covers the bounded WP-11 Heartfold work package, not a general
multi-rig dragon implementation. `AWyrmDragonCharacter` owns form geometry,
transition state, companion behavior, and rig-profile gating. GAS remains the
combat authority, `AWyrmPlayerController` routes/suppresses player actions,
and `UWyrmSaveSubsystem` remains the sole save coordinator.

Normal form changes take one second, stop movement, suppress attacks and
controller movement/jump input, and apply a four-second recovery cooldown.
The target form is checked again immediately before collision and mesh changes
are committed. Health, Focus, attack cooldowns, role, and save ownership are
not duplicated or reset by Heartfold.

## Real PIE results

`py -3.12 tools/run_wp11_pie_proof.py` completed all seven live groups.
The evidence receipt is `Saved/Diagnostics/WP11_heartfold_proof.json`.

| Acceptance | Result | Observed evidence |
|---|---|---|
| DRG-08 compact size and doorway | PASS | Compact capsule is 30 cm radius / 35 cm half-height and fits the 100 cm × 210 cm fixture; True Form (120 / 160) is blocked. |
| DRG-09 compact combat and direct control | PASS | Direct possession returned cleanly; compact primary dealt 9 and secondary dealt 6, with the shared six-second area cooldown. |
| DRG-10 form-state conservation | PASS | A live one-second fold and unfold preserved 175 HP, 420 max HP, and 80 Focus with no free healing. |
| DRG-11 blocked growth | PASS | A 200 cm ceiling rejected True Form growth and leaving the compact dragon unchanged; open space accepted growth. |
| DRG-12 interrupted transition | PASS | Damage interrupted the transition, retained compact form, committed the four-second recovery cooldown, and conserved resulting 390 HP. |
| DRG-13 town behavior | PASS | Town mode suppressed the attack order to Follow and cleared the combat target. |
| SAVE-08 compact save roundtrip | PASS | Compact form, 390 HP, town mode, and compact capsule dimensions restored through the existing save owner. |

## Native automation and build

- `py -3.12 tools/wyrm.py build --timeout 1800` passed the actual UE 5.8.2
  editor target. Receipt:
  `Saved/ScaffoldLogs/20260917T161434Z_878dcfbc906f_build.json`.
- `py -3.12 tools/wyrm.py ue-test --timeout 1800` completed 46/46 native
  scaffold cases: 42 Success, 4 SuccessWithWarnings, and 0 failures. Receipt:
  `Saved/ScaffoldLogs/20260917T161552Z_a3b2dd7233fa_ue-test.json` and
  `Saved/Automation/Scaffold/index.json`.
- Heartfold-native coverage includes compact collision/door fit, combat and
  parity, blocked growth with late clearance revalidation, interruption
  rollback, and the separate `DragonRigProfilePolicy` test.

## DRG-15 rig-profile policy

The supplied Chinese Dragon/Jadefang source asset was inspected as source
metadata only (a glTF with its own skin and animation set). It has not been
imported or proven in Unreal, so it is not reported as playable. The runtime
therefore recognizes only the validated `Verdance` profile. Any other
`DragonId` is rejected before Heartfold, mesh setup, mounting, or flight can
reuse Green Dragon settings. This passes the required policy inspection while
leaving a real Jadefang profile and proof as future work.

## Not run

- Imported Jadefang mesh/skeleton, animation, collision, and its individual
  Heartfold proof.
- Physical-controller validation, cook, packaged-game validation, production
  UI, production maps, and final animation presentation.

## Next bounded task

Read and scope the WP-12 Region 01 landmarks and quest-facts packet. WP-13
remains gated by WP-12.
