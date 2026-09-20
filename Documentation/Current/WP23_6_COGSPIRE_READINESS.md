# WP-23.6 Cogspire Harbor readiness evidence

**Result:** PASS for bounded readiness on September 20, 2026. Cogfang, the
ruler, harbor, authored city-engine construction set, and both urban encounters
have exact supplied candidates. The user explicitly confirms ownership and
project authorization for all supplied assets. No gameplay was authored by this
readiness pass.

## Command and evidence

Executed against UE 5.8.2 CL 56702186:

```text
UnrealEditor-Cmd.exe WYRMFALL.uproject -unattended -nop4 -nosplash -nosound
-nullrhi -ExecutePythonScript=tools/unreal/inspect_wp23_6_cogspire_readiness.py
-stdout -FullStdOutLogOutput
```

The command exited 0 and wrote
`Saved/Diagnostics/WP23_6_cogspire_readiness.json` with status
`PASS_READINESS`. The tracked inspector recreates ignored
diagnostic intake under `/Game/WYRMFALL/Development/Intake/WP23_6`.

## Cogfang finding

`Voxel+Dragons+Pack+Upload.zip::GLTF/Steampunk Dragon.gltf` contains 194 nodes,
35 meshes, 20 animations, 35 materials, and 70 embedded images. UE imported a
`Hip-Local` leader, 34 follower skeletal meshes, one shared skeleton, 20
animation sequences, and 35 material instances. Named coverage includes idle,
walk, run, flying, takeoff, landing, hit, airborne hit, and four attacks.

The imported leader bounds are approximately 3000 x 2800 x 2800 source units,
so final Companion/TrueForm scale cannot be inherited from Verdance or
Jadefang. The asset is an import-feasible **rig candidate**, not a validated
profile. DRG-15 remains fail-closed; form dimensions, compact readability,
living defeat, mount socket, flight, direct control, GAS attacks, and save
identity remain NOT_RUN.

## Exact selected content

| Need | Exact supplied candidate | Native finding | Suitability boundary |
|---|---|---|---|
| Baron Feist Cogwell | `knights.zip::TVS_VoxelKnights_Captain.fbx` | Skeletal mesh, skeleton, physics asset, and one populated material slot; imported bounds about 340 x 131 x 400 | Conditional. Requires authored industrial-noble presentation, production scaling, compatible command/idle/walk proof, and visual approval. |
| Harbor | `pirates and ships.zip::{Boat, Rowboat, Bridge_2, Building_1, Lantern_1, Chest}.fbx` plus Aquatic Pack `jetty.fbx` | All seven imported with populated materials and collision/physics scaffolding. Boat is about 410 x 925 x 755; rowboat 210 x 370 x 150; bridge 120 x 300 x 140; building 285 x 270 x 330. | Concrete source set. Layout scale, walkable collision, waterline, navigation, and route readability remain NOT_RUN. The six pirate FBXs imported as skeletal meshes because they contain animation tracks; final static/skeletal use must be chosen deliberately. |
| House Mark / Deathmark investigation | `knights.zip::TVS_VoxelKnights_Champion.fbx` in `Voxel Casino - Free Sample.fbx` | Champion imported with skeleton, physics, and material; the casino produced eleven static meshes including poker/roulette tables, slot machine, stools, barriers, and three materials. | Conditional. This is exact presentation/location art, not an authored investigation, resolution, or GAS Echo proof. |
| Chef Aurelio Vane / Carver's Precision | `villagers.zip::TVS_VoxelVillage_Chef.fbx` with Tavern `knife`, `bar`, `shelf_meat`, `table`, and `fireplace` OBJ pieces | Chef imported with skeleton, physics, and material. All five props imported with material and body setup. | Conditional. The OBJ props arrive at roughly 0.1–1.7 source units and need an explicit unit/scale correction. Animation compatibility and encounter behavior remain NOT_RUN. |
| City engine | Aquatic Pack `water_pump.fbx`, `steam_leak/idle/frame_001.fbx`, `modular robots.zip::Robot_structures.vox`, and its modular robot parts | Pump and one steam frame imported with material/body setup; the supplied VOX structure and modular archive are located. | Pass for source fit. Cogspire's engine is an authored assembly from these real supplied parts; no monolithic mesh is required. VOX conversion, assembly, collision, material, scale, and public-machinery continuity remain later production acceptance work. |

Source animation files exist for the selected humanoids: Chef has idle/walk;
Captain and Champion have idle/walk/command/slash candidates. This pass did not
retarget or play those clips on the selected meshes, so animation suitability is
recorded as available-but-NOT_RUN rather than passed.

## Materials, collision, scale, and provenance

The receipt contains numeric bounds and material/collision records for 58
imported meshes. Every measured selected mesh has at least one non-null material
and either a physics asset or static body setup. Those generated/imported
objects are inspection evidence, not accepted production collision.

The user explicitly confirmed on September 20, 2026 that every supplied asset
is owned and authorized for WYRMFALL. That statement is the project-use
provenance authority for this gate. The Aquatic Pack additionally contains an
inspected project-use statement. Windows origin metadata links the dragon
archive to CGTrader and the Village/Knights archives to The Voxel Store on
itch.io. No paid source archive or standalone asset pack is redistributed.

## Decision and stop boundary

WP-23.6 is ready for its next bounded packet. Start with a focused Cogfang
profile proof—not the whole region—covering form dimensions, compact
readability, living defeat, mount socket, flight, direct control, GAS attacks,
and stable save identity. Mutable remains the creator, GAS remains combat
authority, existing dragon/travel owners remain authoritative, and
`UWyrmSaveSubsystem` remains the sole persistence coordinator.
