# WP-23.5 required-horror roster completion proof

**Result:** PASS in UE 5.8.2 live PIE on September 20, 2026. All 20 supplied
required-horror source identities now have accepted runtime encounters. This
proof closes the remaining eleven-identity roster boundary; it does not claim
new Echo rewards or Gloaming regional completion.

## Completed roster

The shared `AWyrmRequiredHorrorCharacter` rigid-assembly host preserves the
existing ownership model while eleven concrete classes provide distinct source
parts, scale, stats, idle cadence, prerequisite, and one-time resolution:

| Identity | Source | Parts | Height | Living threshold |
|---|---|---:|---:|---:|
| Ail-Yen | `Alien` | 21 | 214 cm | 220/880 |
| Bellraiser | `Hellraiser` | 16 | 186 cm | 230/920 |
| Sad Echo | `Sadoko` | 16 | 168 cm | 240/960 |
| Dreadator | `Predator` | 16 | 208 cm | 250/1000 |
| Roastface | `Ghostface` | 17 | 182 cm | 260/1040 |
| The Gravy Daughters | `GradyDaughter` | 16 | 148 cm | 270/1080 |
| Knit | `It` | 17 | 178 cm | 280/1120 |
| Canniball | `Hannibal` | 13 | 180 cm | 290/1160 |
| Mum's the Wyrd | `TheMummy` | 18 | 196 cm | 300/1200 |
| Dready Freddie | `Freddy` | 16 | 184 cm | 310/1240 |
| Pyre-Midhead | `PyramidHead` | 17 | 220 cm | 320/1280 |

All 183 unique supplied parts are loaded with their materials. Visual parts use
`NoCollision`; the existing character capsule owns collision. The Gravy
Daughters are separately staged as two complete 16-part instances while their
pair-level encounter commits one receipt.

Each encounter rejects before its preceding identity resolves. Six proof
strikes use the existing GAS damage path and leave exactly 25% positive health.
`UWyrmGloamingSubsystem` commits one identity fact and receipt, rejects the
duplicate, and the existing Schema 7 ledger restores every new fact/receipt
after an in-memory reset/apply roundtrip. No combat, health, travel, or save
owner was introduced.

## Live PIE evidence

Receipt: `Saved/Diagnostics/WP23_5_required_horror_roster_proof.json`

Screenshots:

- `Saved/Diagnostics/WP23_5_RequiredHorrorRoster/01_north_roster.png`
- `Saved/Diagnostics/WP23_5_RequiredHorrorRoster/02_east_roster.png`
- `Saved/Diagnostics/WP23_5_RequiredHorrorRoster/03_south_roster.png`

The live proof passed all eight aggregate assertions:

1. All eleven new route anchors project to navigation.
2. All eleven ordered route legs are complete and non-partial (two points each).
3. Every supplied assembly has the expected source identity, part count,
   materials, and distinct authored height.
4. Two complete Gravy Daughter instances are visibly staged.
5. Ail-Yen rejects before Frank N. Shrine resolves.
6. All eleven GAS living submissions commit once and reject duplicates.
7. Schema 7 restores all eleven facts and receipts after reset.
8. New Echo facts/unlocks and `gloaming.region_complete` remain absent.

Manual review accepted the three final frames for the supplied silhouettes,
scale separation, special parts (Ail-Yen tail, Roastface knife, Knit balloon,
Mum's bandages, Pyre-Midhead sword), paired Daughters staging, grounding,
shadows, and outer cemetery-route continuity.

## Verification

```powershell
& "C:/Program Files/UE_5.8/Engine/Build/BatchFiles/Build.bat" WYRMFALLEditor Win64 Development "-Project=G:/assets/voxel project/WYRMFALL.uproject" -WaitMutex -NoHotReloadFromIDE -NoEngineChanges
py -3.12 tools/run_wp23_5_required_horror_roster.py
& "C:/Program Files/UE_5.8/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "G:/assets/voxel project/WYRMFALL.uproject" -unattended -nop4 -nosplash -nosound -nullrhi "-ExecCmds=Automation RunTests WYRMFALL.Scaffold" "-TestExit=Automation Test Queue Empty" "-ReportExportPath=G:/assets/voxel project/Saved/Automation/WP23_5_RequiredHorrorRoster" -stdout -FullStdOutLogOutput
py -3.12 tools/wyrm.py verify
py -3.12 tools/wyrm.py test
git -c safe.directory="G:/assets/voxel project" diff --check
```

Results: editor target compiled cleanly; live PIE passed without Python errors;
all 69 source-declared native tests passed (57 clean and 12 with pre-existing
logged warnings). Portable verification, tooling tests, and diff check are
recorded with the final checkpoint.

## Explicitly not claimed

- A new Echo reward from any of the eleven identities.
- Gloaming regional completion.
- An interactive keyboard/gamepad walkthrough.
- A new packaged build.

## Next bounded task

With the supplied required-horror roster complete, the next bounded task is the
separate Gloaming regional-completion closure and its full prerequisite,
Nyxaroth, navigation, travel, and save-recovery proof.
