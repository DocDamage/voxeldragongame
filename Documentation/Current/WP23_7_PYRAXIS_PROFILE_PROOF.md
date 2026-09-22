# WP-23.7 Pyraxis explicit profile proof

**Result:** PASS for the bounded DRG-15 profile gate on September 21, 2026.
UE 5.8.2 editor-target build, all 76 native scaffold tests, and five focused
live-PIE groups passed. This does not accept Cinderreach regional gameplay.

## Implementation

`FWyrmDragonRigProfile` now explicitly recognizes `Pyraxis` and binds the
intake's Lava Dragon leader, shared skeleton, idle/flight animations, and all
34 named followers. Its TrueForm presentation scale is 0.040, matching the
corrected runtime-equivalent assembly fixture; Companion scale is 0.010.
Distinct capsule, movement, flight, mount, clearance, and landing values are
declared in the existing dragon owner. An unknown DragonId still fails closed.

No new character, combat, terrain, travel, fact, Echo, or save owner was added.
The existing GAS and dragon-save machinery remains authoritative.

## Verification

| Check | Observed result | Evidence |
|---|---|
| UE 5.8.2 `WYRMFALLEditor Win64 Development` | PASS in normal build mode | `Saved/Diagnostics/WP23_7_pyraxis_build_normal.log` |
| Native `WYRMFALL.Scaffold` | 76/76 PASS, including the extended rig-profile policy | `Saved/Automation/Scaffold/index.json` |
| PYR-01 | 34 follower components, living defeat then bond, 34×39 cm Companion capsule | `Saved/Diagnostics/WP23_7_pyraxis_profile_proof.json` |
| PYR-02 | Heartfold grow/shrink and low-ceiling rejection; 132×175 cm TrueForm capsule | same receipt |
| PYR-03 | Mount, takeoff, landing, and dismount | same receipt |
| PYR-04 | Direct control, GAS primary 24 damage, GAS secondary 18 damage, humanoid repossession | same receipt |
| PYR-05 | `Pyraxis` save-record identity and 34-follower restoration in the PIE fixture | same receipt |

The first diagnostic build used forced `-NoUBA -NoPCH` and failed on unrelated
existing missing-include errors in hovercar, Count Dripula, Corvyn/Counselor,
and Verdance arena translation units. The project's normal editor-target build
then passed; no unrelated source files were edited. See
`Saved/Diagnostics/WP23_7_pyraxis_build.log` for that failed diagnostic run.

## Boundary

The proof exercises a development-map PIE fixture, not a Cinderreach production
map or authored Pyraxis encounter. PYR-05 tests save-record build/restore in
that fixture, not disk persistence or a save-schema extension. Magnarok's
boss-animation breadth, the final animated Flayed Choir overseer and choir
staging, volcanic-country and ritual-arena composition, Pain Reprisal, regional
travel/facts/Echo, and completion remain open. Fire telegraph readability and
the throne-refusal trial have not been tested.
