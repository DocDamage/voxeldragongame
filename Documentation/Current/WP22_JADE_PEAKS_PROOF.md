# WP-22 Jade Peaks production-region proof

**Status:** VERIFIED — JP-01–06 passed in real PIE on Unreal Engine 5.8.2
(CL 56702186) on September 19, 2026, and WP-22-QA1 subsequently closed the
visual/editor findings with manual frame review plus complete navigation paths.

## Accepted slice

- `L_JadePeaks` is a distinct map with bounded finite GeoForge terrain, dynamic
  navigation configuration, a safe-arrival script case, six landmark receipts,
  and a return-route receipt. All six anchors project to navigation and the four
  ordered route legs return complete, non-partial paths.
- Supplied palace environment/prop assets, the normalized palace guard as the
  temple disciple, and the validated Jadefang Chinese Dragon rig load in PIE.
- Jadefang bonds to the same humanoid, completes a Companion-to-TrueForm
  Heartfold transition, mounts, takes off, lands, executes both GAS attacks
  under direct control (24/18 damage), and returns possession to the humanoid
  without introducing another dragon/control owner.
- The disciple supports a trust resolution and grants Mirror Step exactly once.
  Mirror Step commits through GAS for 20 Focus and a 10-second cooldown, moves
  250.2 cm in the valid case, and rejects sealed, occupied, water, over-400 cm,
  and terrain/nav-void destinations without spending Focus.
- Unified Schema 4 save/load restores the Jade Peaks fact/landmark record,
  Mirror Step cooldown, and both `Verdance` and `Jadefang` identities without a
  duplicate reward or a second persistence coordinator.

The production route uses a deterministic level GeoForge surface at Z=900 cm
for safe humanoid navigation and the short-displacement contract. Regional
height and identity come from the supplied palace, aerie placement, and open
true-form flight volume rather than collision-noisy procedural spawn peaks.

## Verification

| Check | Result | Evidence |
|---|---|---|
| Asset intake | PASS | `Saved/Diagnostics/WP22_jade_peaks_asset_intake.json` |
| Map composition | PASS | `Saved/Diagnostics/WP22_jade_peaks_map_composition.json` |
| Strict editor compile | PASS | `Saved/Diagnostics/WP22_editor_build.log`; `-NoUBA -DisableUnity`, 53 actions, 181.36 s |
| Native automation | PASS 54/54 | `Saved/ScaffoldLogs/20260919T124329Z_c76b79000863_ue-test.log` |
| JP-01–JP-06 live PIE | PASS 6/6 | `Saved/Diagnostics/WP22_jade_peaks_proof.json` |
| Visual/editor QA | PASS; interactive walkthrough NOT RUN | `Saved/Diagnostics/WP22_visual_qa.json`; [report](WP22_VISUAL_QA.md) |
| Region 01 regression | PASS 8/8 | `Saved/Diagnostics/WP12_production_pie_proof.json` |
| Jadefang regression | PASS 6/6 | `Saved/Diagnostics/WP20_jadefang_proof.json` |
| Moonbound regression | PASS 4/4 | `Saved/Diagnostics/WP21_moonbound_proof.json` |

Exact primary commands:

```powershell
& "C:/Program Files/UE_5.8/Engine/Build/BatchFiles/Build.bat" WYRMFALLEditor Win64 Development "-Project=G:/assets/voxel project/WYRMFALL.uproject" -WaitMutex -NoHotReloadFromIDE -NoEngineChanges -NoUBA -DisableUnity
py -3.12 tools/wyrm.py ue-test --engine-root "C:/Program Files/UE_5.8" --timeout 1800
py -3.12 tools/run_wp22_jade_peaks_proof.py
py -3.12 tools/run_wp22_visual_qa.py
py -3.12 tools/run_wp12_production_pie_proof.py
py -3.12 tools/run_wp20_jadefang_proof.py
py -3.12 tools/run_wp21_moonbound_proof.py
```

## Boundary

This proves the bounded JP-01–JP-06 slice only. It is not the full Jade Peaks
campaign, the missing later regions, the playable colony, or the full-world
finale. No cook/package rerun was required or performed for WP-22. The visual
QA and correction evidence is tracked in
[WP22_VISUAL_QA.md](WP22_VISUAL_QA.md); an interactive keyboard/gamepad
walkthrough remains NOT RUN and is not implied by the deterministic capture and
path queries. WP-23 is user-authorized as the rest-of-world umbrella, but its
regional children remain independently gated by real content and evidence.
