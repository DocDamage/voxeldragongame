# WP-07 scoped activities proof

**Status: PARTIAL — scoped native and real-PIE proof passed; no WP-07 task packet exists in the current repository.**

## Implemented owners

- `AWyrmWaterVolume`: bounded water queries, wet/swim transitions, and protected basin edit validation.
- `UWyrmFishingComponent`: fishing state machine, movement/damage cancellation, and inventory catch commit.
- `UWyrmCraftingSubsystem`: recipe authority with ingredient/capacity preflight and rollback.
- `UWyrmSaveSubsystem`: remains the single save coordinator and now persists the active food preparation buff.
- GAS attributes remain the combat/stat authority.

The supplied campfire, water props, fish, potato, fishing-tool proxy, and fishing/cooking audio were imported. The current `SM_FishingRod` is sourced from the supplied ranger spear and is a fixture proxy, not accepted production fishing-rod art.

## Fresh evidence

- Editor target: PASS with UE 5.8.2, `-NoUBA -NoPCH`.
- Native automation: PASS, 27/27 source-declared tests.
- Real PIE: PASS, 8/8 scoped groups in `Saved/Diagnostics/WP07_activities_proof.json`.

The PIE proof loaded ten real assets and exercised water entry/exit, basin-lip rejection and interior excavation, fishing catch/interrupt/full-bag behavior, campfire fish-stew crafting, ingredient-slot capacity, and food-buff refresh/replacement/in-memory save restore.

Native tests additionally cover station and missing-ingredient rejection, exact output-capacity rejection without input loss, food-buff expiry, Power/MaxFocus removal, and disk save/load.

## Corrections made during verification

- Food buffs now remove modifiers after natural expiry, apply and remove Power once, save base attributes, preserve the display name, and restore Focus after rebuilding the active maximum.
- Crafting aggregates duplicate ingredient requirements, simulates post-consumption capacity, and restores inventory snapshots on unexpected removal/output failure.
- Fishing clears the reel timer on commit and ignores zero damage cancellation.
- Terrain adapters discover water volumes regardless of actor BeginPlay order.
- The crafting enum was renamed to avoid an Unreal Python reflection collision with `AWyrmCraftingStation`.

## Boundary

This proves a bounded fixture, not full WP-07 acceptance. Production interaction/UI, animations, audio playback wiring, water rendering, final fishing-rod art, a production map, combined GeoForge water-save behavior, cook, and packaged-game validation remain open. The current backlog has no WP-07 packet defining complete acceptance.
