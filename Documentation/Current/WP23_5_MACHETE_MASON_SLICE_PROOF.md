# WP-23.5 Machete Mason encounter slice proof

**Result:** PASS in UE 5.8.2 live PIE on September 20, 2026. This is one
bounded required-horror gameplay assembly; it does not claim another identity,
a new Echo, or Gloaming regional completion.

## Implemented contract

- `AWyrmMacheteMasonCharacter` assembles all 17 supplied `Jason` source meshes,
  including the weapon, under the original production-facing identity Machete
  Mason. It uses the same user-supplied-art authorization context as the other
  accepted project assets; source filenames remain intake-only metadata.
- The presentation is normalized to 188 cm. Visual parts use `NoCollision`, the
  existing character capsule owns collision, and a bounded procedural-root
  guard sway provides motion without claiming skeletal locomotion.
- Encounter activation rejects until `gloaming.michael_mire_resolved` exists.
  Six proof strikes use the existing GAS damage path, taking Mason from 560
  health to the living disarmed-submission threshold at 140 health.
- Disarmed submission commits `gloaming.machete_mason_resolved` and the one-time
  `gloaming.machete_mason.disarmed_submission` receipt through
  `UWyrmGloamingSubsystem`; duplicate resolution is rejected.
- Existing Schema 7 ledger serialization restores the fact and receipt after an
  in-memory reset/apply roundtrip. No combat, health, travel, or save owner was
  introduced.

## Live PIE evidence

Receipt: `Saved/Diagnostics/WP23_5_machete_mason_slice_proof.json`

Screenshot: `Saved/Diagnostics/WP23_5_MacheteMason/01_machete_mason_encounter.png`

The live proof passed all eight assertions:

1. Michael Mire and Machete Mason route anchors project to navigation.
2. The ordered route is valid, complete, and non-partial (two path points).
3. All 17 supplied meshes, their materials, and the weapon are present in the
   188 cm runtime presentation.
4. Encounter activation rejects before Michael Mire resolves.
5. Six GAS damage applications reach exactly 140/560 health without killing
   Mason.
6. Disarmed submission commits once and duplicate resolution is rejected.
7. Schema 7 restores the Machete Mason fact and receipt after reset.
8. Machete Mason Echo and `gloaming.region_complete` state remain absent.

Manual review accepted the corrected final frame for mask, clothing and weapon
silhouette, ground contact and shadow, nearby grave/tree scale, visible Michael
continuity, and cathedral landmark readability. The first bright, empty frame
was rejected and replaced.

## Verification

```powershell
& "C:/Program Files/UE_5.8/Engine/Build/BatchFiles/Build.bat" WYRMFALLEditor Win64 Development "-Project=G:/assets/voxel project/WYRMFALL.uproject" -WaitMutex -NoHotReloadFromIDE -NoEngineChanges
py -3.12 tools/run_wp23_5_machete_mason_slice.py
py -3.12 tools/wyrm.py ue-test
py -3.12 tools/wyrm.py verify
py -3.12 tools/wyrm.py test
git -c safe.directory="G:/assets/voxel project" diff --check
```

Results: editor target compiled cleanly; live PIE passed without Python errors
or navigation ensures; all 61 source-declared native tests passed. Portable
verification, tooling tests, and diff check are recorded after documentation
reconciliation.

## Explicitly not claimed

- Another required-horror identity or encounter.
- A Machete Mason Echo reward.
- Gloaming regional completion.
- An interactive keyboard/gamepad walkthrough.
- A new packaged build for this slice.

## Next bounded task

Inspect and author one additional supplied required-horror identity using the
same established assembly and authorization context. Keep any new Echo and
regional completion outside that slice.
