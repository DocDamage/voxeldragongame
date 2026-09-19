# WP-23.5 Arrival-to-Ashgrave gameplay slice

**Result:** PASS in UE 5.8.2 live PIE on September 19, 2026.

## Accepted boundary

`L_GloamingMarches` now contains an invisible authored arrival trigger and an
Ashgrave extraction-seal interaction volume positioned against the supplied
crypt scene. `UWyrmGloamingSubsystem` is the narrow regional fact/receipt
owner; it does not own combat, inventory, terrain, travel, or save slots.

The focused slice proved:

1. Arrival and Ashgrave anchors both project to live navigation.
2. Arrival → Ashgrave produces a valid, complete, non-partial path (4 points).
3. The seal rejects interaction before the Arrival fact exists.
4. Entering the Arrival trigger commits `gloaming.arrival` once.
5. Interacting at Ashgrave commits
   `gloaming.ashgrave_extraction_seal_resolved` and receipt
   `gloaming.ashgrave_extraction_seal.resolved` once; repeats are rejected.
6. No Count Malvaine, Hollow Twins, Gloaming Echo, or regional-completion fact
   is granted by this bounded route.

## Commands and evidence

```text
C:\Program Files\UE_5.8\Engine\Build\BatchFiles\Build.bat
  WYRMFALLEditor Win64 Development
  -Project=G:/assets/voxel project/WYRMFALL.uproject
  -WaitMutex -NoHotReloadFromIDE -NoEngineChanges

py -3.12 tools/run_wp23_5_ashgrave_slice.py --pie-only
py -3.12 tools/wyrm.py ue-test --engine-root C:/Program Files/UE_5.8 --timeout 1800
```

- Editor target build: PASS against the installed UE 5.8.2 headers.
- Focused live PIE: PASS.
- Native automation: 56/56 source-declared `WYRMFALL.Scaffold` tests passed.
- Machine receipt: `Saved/Diagnostics/WP23_5_ashgrave_slice_proof.json`.
- Native report: `Saved/Automation/Scaffold/index.json`.

The reproducible wrapper runs the supplied environment intake and map composer
before PIE when invoked without `--pie-only`.

## Explicitly not claimed

- Count Malvaine or Hollow Twins encounter logic.
- Sanguine Strike, Second Turn, or any other Gloaming Echo outcome.
- Nyxaroth regional encounter/closure.
- Gloaming travel, return, unified save recovery, or regional completion.
- A new manual keyboard/gamepad collision walkthrough. The prior foundation's
  rendered lighting/scale/collision-configuration/route-readability review
  remains the visual evidence for this map.

## Next bounded task

Completed by [Count Malvaine encounter proof](WP23_5_MALVAINE_SLICE_PROOF.md).
The next bounded task is a focused Hollow Twins encounter using two distinct
authored identities; keep Second Turn, travel/save recovery, and regional
completion outside that slice.
