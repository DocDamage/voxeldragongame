# WP-23.5 Pleatherface encounter slice proof

**Result:** PASS in UE 5.8.2 live PIE on September 20, 2026. This is one
bounded required-horror gameplay assembly; it does not claim another identity,
a new Echo, or Gloaming regional completion.

## Implemented contract

- `AWyrmPleatherfaceCharacter` assembles all 17 supplied `Leatherface` source
  meshes, including `Sword_Leatherface`, under the original production-facing
  identity Pleatherface. It uses the same user-supplied-art authorization
  context as the other accepted project assets; source filenames remain
  intake-only metadata.
- The presentation is normalized to 192 cm. Visual parts use `NoCollision`, the
  existing character capsule owns collision, and a bounded procedural-root sway
  provides motion without claiming skeletal locomotion.
- Encounter activation rejects until `gloaming.machete_mason_resolved` exists.
  Six proof strikes use the existing GAS damage path, taking Pleatherface from
  600 health to the living disarmed-submission threshold at 150 health.
- Disarmed submission commits `gloaming.pleatherface_resolved` and the one-time
  `gloaming.pleatherface.disarmed_submission` receipt through
  `UWyrmGloamingSubsystem`; duplicate resolution is rejected.
- Existing Schema 7 ledger serialization restores the fact and receipt after an
  in-memory reset/apply roundtrip. No combat, health, travel, or save owner was
  introduced.

## Live PIE evidence

Receipt: `Saved/Diagnostics/WP23_5_pleatherface_slice_proof.json`

Screenshot: `Saved/Diagnostics/WP23_5_Pleatherface/01_pleatherface_encounter.png`

The live proof passed all eight assertions:

1. Machete Mason and Pleatherface route anchors project to navigation.
2. The ordered route is valid, complete, and non-partial (four path points).
3. All 17 supplied meshes, their materials, and the sword are present in the
   192 cm runtime presentation.
4. Encounter activation rejects before Machete Mason resolves.
5. Six GAS damage applications reach exactly 150/600 health without killing
   Pleatherface.
6. Disarmed submission commits once and duplicate resolution is rejected.
7. Schema 7 restores the Pleatherface fact and receipt after reset.
8. Pleatherface Echo and `gloaming.region_complete` state remain absent.

Manual review accepted the final frame for face, clothing and sword silhouette,
ground contact and shadow, nearby character/tree scale, and route-context
readability. The bright local fill is intentional encounter lighting; it does
not obscure the presentation or its terrain contact.

## Verification

```powershell
& "C:/Program Files/UE_5.8/Engine/Build/BatchFiles/Build.bat" WYRMFALLEditor Win64 Development "-Project=G:/assets/voxel project/WYRMFALL.uproject" -WaitMutex -NoHotReloadFromIDE -NoEngineChanges
py -3.12 tools/run_wp23_5_pleatherface_slice.py
& "C:/Program Files/UE_5.8/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "G:/assets/voxel project/WYRMFALL.uproject" -unattended -nop4 -nosplash -nosound -nullrhi "-ExecCmds=Automation RunTests WYRMFALL.Scaffold" "-TestExit=Automation Test Queue Empty" "-ReportExportPath=G:/assets/voxel project/Saved/Automation/WP23_5_Pleatherface" -stdout -FullStdOutLogOutput
py -3.12 tools/wyrm.py verify
py -3.12 tools/wyrm.py test
git -c safe.directory="G:/assets/voxel project" diff --check
```

Results: editor target compiled cleanly; live PIE passed without Python errors
or navigation ensures; all 62 source-declared native tests passed (50 clean and
12 with pre-existing logged warnings). Portable verification, tooling tests,
and diff check are recorded after documentation reconciliation.

## Explicitly not claimed

- Another required-horror identity or encounter.
- A Pleatherface Echo reward.
- Gloaming regional completion.
- An interactive keyboard/gamepad walkthrough.
- A new packaged build for this slice.

## Next bounded task

Inspect and author one additional supplied required-horror identity using the
same established assembly and authorization context. Keep any new Echo and
regional completion outside that slice.
