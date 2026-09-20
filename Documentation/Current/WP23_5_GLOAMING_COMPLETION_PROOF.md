# WP-23.5 Gloaming regional-completion proof

**Result:** PASS in UE 5.8.2 live PIE on September 20, 2026.

## Accepted closure

`UWyrmGloamingSubsystem` now commits `gloaming.region_complete` with the
one-time `gloaming.region.completion_committed` receipt only when all required
horror resolutions are present, the supplied Dark Dragon is the validated and
bonded Nyxaroth identity, and `UWyrmWorldTravelSubsystem` reports the authored
Gloaming arrival plus the allowlisted Region01 return route. Sanguine Strike
and Second Turn remain optional and are not completion prerequisites.

The live PIE reproducer proved:

1. Closure rejects before the encounter ledger is complete and without a dragon.
2. A bonded validated non-Nyxaroth dragon cannot satisfy the identity gate.
3. All 20 required-horror outcomes plus bonded Nyxaroth commit the fact and
   receipt once; duplicate calls reject.
4. The final Pyre-Midhead anchor and authored return anchor project to
   navigation, with a complete four-point route between them.
5. Schema 7 restores the completion ledger and exactly one bonded Nyxaroth.
6. Region01 return and Gloaming re-entry remain allowlisted after completion.
7. Closure succeeds with zero optional local Echo unlocks.

Machine receipt:
`Saved/Diagnostics/WP23_5_gloaming_completion_proof.json`.

Rendered context:
`Saved/Diagnostics/WP23_5_GloamingCompletion/01_gloaming_closure.png`.

## Verification

```powershell
& "C:/Program Files/UE_5.8/Engine/Build/BatchFiles/Build.bat" WYRMFALLEditor Win64 Development "-Project=G:/assets/voxel project/WYRMFALL.uproject" -WaitMutex -NoHotReloadFromIDE -NoEngineChanges
py -3.12 tools/run_wp23_5_gloaming_completion.py
& "C:/Program Files/UE_5.8/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "G:/assets/voxel project/WYRMFALL.uproject" -unattended -nop4 -nosplash -nosound -nullrhi "-ExecCmds=Automation RunTests WYRMFALL.Scaffold" "-TestExit=Automation Test Queue Empty" "-ReportExportPath=G:/assets/voxel project/Saved/Automation/WP23_5_GloamingCompletion" -stdout -FullStdOutLogOutput
py -3.12 tools/wyrm.py verify
py -3.12 tools/wyrm.py test
git -c safe.directory="G:/assets/voxel project" diff --check
```

The editor target compiled cleanly, live PIE passed all seven closure checks,
and native automation passed 70/70 tests (58 clean and 12 carrying existing
logged warnings).

## Boundary

WP-23.5 is complete. This proof does not claim a new packaged build or an
interactive keyboard/gamepad walkthrough. Mutable remains creator authority,
GAS remains combat authority, and `UWyrmSaveSubsystem` remains the sole
persistence coordinator.
