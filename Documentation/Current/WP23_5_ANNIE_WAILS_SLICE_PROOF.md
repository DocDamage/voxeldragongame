# WP-23.5 Annie Wails encounter slice proof

**Result:** PASS in UE 5.8.2 live PIE on September 20, 2026. This is one
bounded required-horror gameplay assembly; it does not claim another identity,
a new Echo, or Gloaming regional completion.

## Implemented contract

- `AWyrmAnnieWailsCharacter` assembles all 17 supplied `AnnieWilkes` source
  meshes, including the source-labelled `Sword_AnnieWilkes` weapon, under the
  original production-facing identity Annie Wails. Source filenames remain
  intake-only metadata under the established user-supplied-art authorization.
- The presentation is normalized to 176 cm. Visual parts use `NoCollision`, the
  existing character capsule owns collision, and a bounded vigilant sway
  provides motion without claiming skeletal locomotion.
- Encounter activation rejects until `gloaming.wherewolf_resolved` exists. Six
  proof strikes use the existing GAS damage path, taking Annie Wails from 680
  health to the living disarmed-surrender threshold at 170 health.
- Surrender commits `gloaming.annie_wails_resolved` and the one-time
  `gloaming.annie_wails.disarmed_surrender` receipt through
  `UWyrmGloamingSubsystem`; duplicate resolution is rejected.
- Existing Schema 7 ledger serialization restores the fact and receipt after an
  in-memory reset/apply roundtrip. No combat, health, travel, or save owner was
  introduced.

## Live PIE evidence

Receipt: `Saved/Diagnostics/WP23_5_annie_wails_slice_proof.json`

Screenshot: `Saved/Diagnostics/WP23_5_AnnieWails/01_annie_wails_encounter.png`

The live proof passed all eight assertions:

1. Wherewolf and Annie Wails route anchors project to navigation.
2. The ordered route is valid, complete, and non-partial (four path points).
3. All 17 supplied meshes, their materials, and the source-labelled weapon are
   present in the 176 cm runtime presentation.
4. Encounter activation rejects before Wherewolf resolves.
5. Six GAS damage applications reach exactly 170/680 health without killing
   Annie Wails.
6. Disarmed surrender commits once and duplicate resolution is rejected.
7. Schema 7 restores the Annie Wails fact and receipt after reset.
8. Annie Wails Echo and `gloaming.region_complete` state remain absent.

Manual review accepted the corrected final frame for body and weapon silhouette,
ground contact and shadow, nearby character scale, and cathedral route context.
The first capture was rejected for a rear-biased view; only the proof camera was
corrected.

## Verification

```powershell
& "C:/Program Files/UE_5.8/Engine/Build/BatchFiles/Build.bat" WYRMFALLEditor Win64 Development "-Project=G:/assets/voxel project/WYRMFALL.uproject" -WaitMutex -NoHotReloadFromIDE -NoEngineChanges
py -3.12 tools/run_wp23_5_annie_wails_slice.py
& "C:/Program Files/UE_5.8/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "G:/assets/voxel project/WYRMFALL.uproject" -unattended -nop4 -nosplash -nosound -nullrhi "-ExecCmds=Automation RunTests WYRMFALL.Scaffold" "-TestExit=Automation Test Queue Empty" "-ReportExportPath=G:/assets/voxel project/Saved/Automation/WP23_5_AnnieWails" -stdout -FullStdOutLogOutput
py -3.12 tools/wyrm.py verify
py -3.12 tools/wyrm.py test
git -c safe.directory="G:/assets/voxel project" diff --check
```

Results: editor target compiled cleanly; live PIE passed without Python errors
or navigation ensures; all 64 source-declared native tests passed (52 clean and
12 with pre-existing logged warnings). Portable verification, 124 tooling tests
with two expected platform/privilege skips, and diff check passed.

## Explicitly not claimed

- Another required-horror identity or encounter.
- An Annie Wails Echo reward.
- Gloaming regional completion.
- An interactive keyboard/gamepad walkthrough.
- A new packaged build for this slice.

## Next bounded task

Inspect and author one additional supplied required-horror identity using the
same established assembly and authorization context. Keep any new Echo and
regional completion outside that slice.
