# WP-23.5 Count Malvaine encounter slice

**Result: PASS in UE 5.8.2 live PIE; focused rendered review PASS**

`L_GloamingMarches` now contains `AWyrmCountMalvaineCharacter` at the verified
cathedral route anchor. The encounter reuses the supplied Voxel Cathedral
Priest skeletal mesh, palette material, and preaching animation through a
dedicated presentation component; the capsule remains collision authority and
GAS remains combat authority. `UWyrmGloamingSubsystem` remains the sole owner
of regional facts and receipts.

## Proved acceptance

1. Ashgrave and Malvaine anchors project to live navigation and their path is
   valid, complete, and non-partial.
2. Malvaine rejects encounter start before the Ashgrave extraction seal.
3. Ordinary parley commits its route and shared resolution exactly once.
4. Six GAS damage applications reduce Malvaine from 700 to a living 100 HP;
   the nonlethal submission route commits its distinct route and shared
   resolution exactly once.
5. Neither route unlocks Sanguine Strike or commits Hollow Twins / regional
   completion state.
6. The supplied Priest is visible, grounded, animating, and measured at
   179.34 cm. A pre-encounter gravity hold prevents falling before GeoForge
   collision settles.

## Evidence

- Reproducer: `py -3.12 tools/run_wp23_5_malvaine_slice.py`
- Machine receipt: `Saved/Diagnostics/WP23_5_malvaine_slice_proof.json`
- Intake receipt: `Saved/Diagnostics/WP23_5_malvaine_intake.json`
- Reviewed frame: `Saved/Diagnostics/WP23_5_Malvaine/01_malvaine_cathedral.png`
- Editor target, native automation, and portable checks: PASS.

The rendered frame was manually reviewed for lighting, grounded character and
prop scale, capsule/terrain alignment, cathedral landmark readability, and
approach context. No diagnostic primitive substitutes for Malvaine.

## Explicitly not claimed

- Sanguine Strike manifestation or persistence.
- Hollow Twins encounter or Second Turn.
- Gloaming travel/return, save recovery, or regional completion.

## Next bounded task

Build one bounded Hollow Twins encounter slice using two distinct authored
identities and the existing Gloaming fact owner. Keep Second Turn,
travel/save recovery, and regional completion outside that slice.
