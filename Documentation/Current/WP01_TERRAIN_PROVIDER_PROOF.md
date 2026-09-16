# WP-01: one real terrain provider proof — GeoForge

**Date:** September 16, 2026
**Engine:** UE 5.8.2, CL 56702186
**Provider:** GeoForgeRuntime 5.0.0 through `AWyrmGeoForgeAdapter`
**Status:** **PASS — fresh compile, 22/22 full native suite, 7/7 real-PIE groups**

## Authority and completion contract

`AWyrmGeoForgeAdapter` is the single terrain provider selected by the project.
It implements `IWyrmTerrainProvider`; no second voxel owner was introduced.

An edit that changes geometry may return `QUEUED`. The adapter ticks until
GeoForge reports no queued generation/rebuild work, no mesh jobs, and no pending
chunk applies. It then submits navigation refresh and waits for the navigation
build to settle before broadcasting collision/navigation/edit completion. The
proof explicitly observed `QUEUED` for dig, refill and the persistence mutation.

## Finite yield semantics

For a remove edit, the adapter queries exact GeoForge cells before and after the
dig. Yield is based on the number of filled cells actually removed, multiplied
by resolved cell volume. Transaction IDs remain idempotent:

- the first proof dig removed 72 cells and yielded 72,000,000 cm³ of
  `Resource.Dirt`;
- reusing that action ID was rejected and returned a duplicate-prevented record
  with zero extraction;
- a different action ID over the exhausted volume completed with zero yield.

This closes the earlier requested-sphere estimate that could reward repeated
edits over empty terrain.

## Real PIE result

Command:

```powershell
py -3.12 tools/run_pie_proof.py
```

The runner starts `UnrealEditor.exe`; the script explicitly starts PIE and uses
the PIE game world. The runner now reads the receipt and returns failure when the
editor process exits successfully but the proof report does not say `PASS`.

Receipt: `Saved/Diagnostics/WP01_terrain_provider_proof.json`.

| Case | Observed result | Status |
|---|---|---|
| `WRLD-01` | Dig submit queued; completion confirmed with all GeoForge queues zero; floor dropped from Z 900 to Z 500 | PASS |
| `WRLD-02` | Refill submit queued; completion confirmed; floor restored to Z 1000 | PASS |
| `WRLD-03` | 72-cell finite yield; duplicate rejected; distinct exhausted-volume edit yielded zero | PASS |
| `WRLD-04` | New lower navigation surface projected at Z 560, 60 cm from the traced floor | PASS |
| `WRLD-05` | Buried navigation point at Z 500 no longer projected after refill | PASS |
| `WRLD-08` | Active pawn overlap detected through `ECC_Pawn`; fill rejected and crater preserved | PASS |
| `SAVE-01..04` | 57,286-byte GeoForge payload restored the saved floor at Z 1000 after a completed modification | PASS |

## Persistence boundary

`BuildSavePayload` and `ApplySavePayload` use GeoForge's terrain save data in a
binary Unreal save object. Capture and apply reject unbound terrain, empty input,
or an edit still pending. `UWyrmSaveSubsystem` now fails before mutating the
character when a supplied terrain owner/payload fails.

The PIE proof exercises the adapter payload directly. One combined
`UWyrmSaveSubsystem` roundtrip with bound terrain plus character and inventory is
still NOT_RUN and remains a WP-05 acceptance boundary.

## Implementation and evidence

- `Source/WYRMFALL/Public/Terrain/WyrmTerrainProvider.h`
- `Source/WYRMFALL/Public/Terrain/WyrmGeoForgeAdapter.h`
- `Source/WYRMFALL/Private/Terrain/WyrmGeoForgeAdapter.cpp`
- `Source/WYRMFALL/Private/Terrain/WyrmTerrainDiagnostics.cpp`
- `Source/WYRMFALL/Private/Save/WyrmSaveSubsystem.cpp`
- `tools/unreal/verify_wp01_terrain_proof.py`
- `tools/run_pie_proof.py`
- `Saved/Automation/Scaffold/index.json`
- `Saved/Diagnostics/WP01_terrain_provider_proof.json`

## Remaining boundary

- Multi-region streaming, biome generation, stress/performance and multiplayer
  behavior are outside this bounded proof.
- Production terrain material adaptation is not accepted by this proof.
- Full WP-00 RDY-02/03/04 readiness remains open.
- Cook and packaged-game validation are NOT_RUN.
