# WP-23.5 Wherewolf encounter slice proof

**Result:** PASS in UE 5.8.2 live PIE on September 20, 2026. This is one
bounded required-horror gameplay assembly; it does not claim another identity,
a new Echo, or Gloaming regional completion.

## Implemented contract

- `AWyrmWherewolfCharacter` assembles all 16 supplied `Werewolf` source meshes
  under the original production-facing identity Wherewolf. It uses the same
  user-supplied-art authorization context as the other accepted project assets;
  source filenames remain intake-only metadata.
- The broad presentation is normalized to 205 cm. Visual parts use
  `NoCollision`, the existing character capsule owns collision, and a bounded
  breathing/guard sway provides motion without claiming skeletal locomotion.
- Encounter activation rejects until `gloaming.pleatherface_resolved` exists.
  Six proof strikes use the existing GAS damage path, taking Wherewolf from 640
  health to the living calmed-submission threshold at 160 health.
- Calmed submission commits `gloaming.wherewolf_resolved` and the one-time
  `gloaming.wherewolf.calmed_submission` receipt through
  `UWyrmGloamingSubsystem`; duplicate resolution is rejected.
- Existing Schema 7 ledger serialization restores the fact and receipt after an
  in-memory reset/apply roundtrip. No combat, health, travel, or save owner was
  introduced.

## Live PIE evidence

Receipt: `Saved/Diagnostics/WP23_5_wherewolf_slice_proof.json`

Screenshot: `Saved/Diagnostics/WP23_5_Wherewolf/01_wherewolf_encounter.png`

The live proof passed all eight assertions:

1. Pleatherface and Wherewolf route anchors project to navigation.
2. The ordered route is valid, complete, and non-partial (two path points).
3. All 16 supplied meshes and their materials are present in the 205 cm runtime
   presentation.
4. Encounter activation rejects before Pleatherface resolves.
5. Six GAS damage applications reach exactly 160/640 health without killing
   Wherewolf.
6. Calmed submission commits once and duplicate resolution is rejected.
7. Schema 7 restores the Wherewolf fact and receipt after reset.
8. Wherewolf Echo and `gloaming.region_complete` state remain absent.

Manual review accepted the corrected final frame for the wolf head and extended
claw silhouette, ground contact and shadow, nearby tree/character scale, and
route-context readability. An earlier capture was rejected because a foreground
tree obscured the identity; only the proof camera was corrected.

## Verification

```powershell
& "C:/Program Files/UE_5.8/Engine/Build/BatchFiles/Build.bat" WYRMFALLEditor Win64 Development "-Project=G:/assets/voxel project/WYRMFALL.uproject" -WaitMutex -NoHotReloadFromIDE -NoEngineChanges
py -3.12 tools/run_wp23_5_wherewolf_slice.py
& "C:/Program Files/UE_5.8/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "G:/assets/voxel project/WYRMFALL.uproject" -unattended -nop4 -nosplash -nosound -nullrhi "-ExecCmds=Automation RunTests WYRMFALL.Scaffold" "-TestExit=Automation Test Queue Empty" "-ReportExportPath=G:/assets/voxel project/Saved/Automation/WP23_5_Wherewolf" -stdout -FullStdOutLogOutput
py -3.12 tools/wyrm.py verify
py -3.12 tools/wyrm.py test
git -c safe.directory="G:/assets/voxel project" diff --check
```

Results: editor target compiled cleanly; live PIE passed without Python errors
or navigation ensures; all 63 source-declared native tests passed (51 clean and
12 with pre-existing logged warnings). Portable verification, 124 tooling tests
with two expected platform/privilege skips, and diff check passed.

## Explicitly not claimed

- Another required-horror identity or encounter.
- A Wherewolf Echo reward.
- Gloaming regional completion.
- An interactive keyboard/gamepad walkthrough.
- A new packaged build for this slice.

## Next bounded task

Inspect and author one additional supplied required-horror identity using the
same established assembly and authorization context. Keep any new Echo and
regional completion outside that slice.
