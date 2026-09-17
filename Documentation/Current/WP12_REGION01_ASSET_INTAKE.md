# WP-12: Region 01 asset intake

**Date**: 2026-09-17

**Engine checked**: Unreal Engine 5.8.2 (`C:\\Program Files\\UE_5.8`)

**Status**: **PARTIAL — environment-package discovery/load, supplied NPC
mesh/texture/clip metadata, authored-palette inspection, and normalized
mesh/animation skeleton compatibility PASS; visual playback, production
material, and production placement NOT_RUN**

## Intake record

Two existing entries from the user's installed Epic/Fab library cache were
copied one-way into the project `Content` directory. The copy preserved their
native Unreal package roots, so vendor references continue to resolve under
`/Game/BanditCamp` and `/Game/DarkHalls`. No package was renamed, saved, or
added to source control.

| Content | Intended Region 01 use | Native mount root | Installed files | Source identity |
|---|---|---:|---:|---|
| Modular Medieval Village | Tidecross buildings, cart/claim dressing, settlement props | `/Game/BanditCamp` | 724 | Fab listing `76716e34-0fe4-403c-8b77-cdd7935c668e` |
| Modular Dungeon | Crowncut buried underworks and compact-cave interior geometry | `/Game/DarkHalls` | 269 | Fab listing `0ff28c1f-790d-4a1b-92be-04617439270b` |

The corresponding local sources were already present in the user's installed
library cache. A restartable, non-destructive copy was followed by a `robocopy
/L` manifest comparison: both destinations had zero files to copy, mismatches,
failures, or extras. The local vendor directories are deliberately covered by
the root `.gitignore`; the public repository records only the intake method and
proof script, never proprietary `.uasset` binaries.

## Native UE verification

`UnrealEditor-Cmd.exe` ran
`tools/unreal/verify_wp12_region01_asset_intake.py` under UE 5.8.2. It
confirmed discovery and successful object loading for all four representative
assets:

| Role | Object path | Class | Result |
|---|---|---|---|
| Tidecross cart | `/Game/BanditCamp/Blueprints/BP_Cart_01.BP_Cart_01` | Blueprint | PASS |
| Tidecross cooking pot | `/Game/BanditCamp/Blueprints/BP_Cooking_Pot.BP_Cooking_Pot` | Blueprint | PASS |
| Underworks corridor | `/Game/DarkHalls/BluePrints/BP_CorridorStraight_01.BP_CorridorStraight_01` | Blueprint | PASS |
| Underworks wall | `/Game/DarkHalls/StaticMeshes/Architecture/SM_CorridorWall_01.SM_CorridorWall_01` | StaticMesh | PASS |

The generated receipt is
`Saved/Diagnostics/WP12_region01_asset_intake.json`. This check calls no save
operation and does not prove visual composition, collision in a production
map, navigation, quest interactions, or any `REG` acceptance case.

## Supplied NPC candidate intake

The user-supplied `assets and old docs/voxel/characters` archives contain the
approved human source collection registered as `ART-HUM-01`. Six distinct
role-fit candidates were selected from that supplied art, selectively extracted
under `Saved/AssetIntake/WP12/NPC`, and imported only into the ignored
`/Game/WYRMFALL/Development/Intake/WP12/NPC` mount. The import did not modify
the source archives or add proprietary `.uasset` files to Git.

| Region 01 role | Supplied source candidate | Native mesh/texture result |
|---|---|---|
| Tamsin | Voxel Rangers / Captain | PASS — SkeletalMesh, matching texture, 15 bones, 1 material slot |
| Mara | Voxel Village / MarketWoman | PASS — SkeletalMesh, matching texture, 15 bones, 1 material slot |
| Sella | Voxel Farm / FarmersDaughter | PASS — SkeletalMesh, matching texture, 15 bones, 1 material slot |
| Pell | Voxel Farm / FarmHand | PASS — SkeletalMesh, matching texture, 15 bones, 1 material slot |
| Iven | Voxel Village / YoungMan | PASS — SkeletalMesh, matching texture, 15 bones, 1 material slot |
| Rusk | Voxel Knights / Commander | PASS — SkeletalMesh, matching texture, 15 bones, 1 material slot |

`UnrealEditor-Cmd.exe` ran
`tools/unreal/inspect_wp12_region01_npc_intake.py` under UE 5.8.2 and wrote
`Saved/Diagnostics/WP12_region01_npc_intake.json`. The receipt records exact
source paths and SHA-256 values, imported-object paths, skeletons, bone counts,
bounds, and material-slot counts. The FBX source importer reported non-fatal
bind-pose and smoothing-group warnings for the selected meshes; the native
imports and measurements nevertheless completed successfully.

The initial UE 5.8 Interchange animation test classified an idle sample as a
StaticMesh. A subsequent legacy-FBX clip import did create the six matching
`AnimSequence` metadata pairs (60 keys / 2.458 s idle and 41 keys / 1.667 s
walk), but that alone is not visual acceptance: its directly imported mesh
retained only 15 bones while the supplied source armature has 25. The direct
developer-fixture walk sample at a known non-zero frame visibly deformed, so
that route remains rejected for playback despite matching object metadata.

## Normalized supplied-animation compatibility

Read-only Blender inspection found that the supplied mesh and walk clip use
the same 25 named bones, but the source armature has three roots. The local
normalization tool preserves every supplied source FBX, adds one identity
`Wyrmfall_NormalizedRoot`, and exports mesh/idle/walk derivatives only under
ignored `Saved/AssetIntake/WP12/NPC/Normalized`.

`UnrealEditor-Cmd.exe` then ran
`tools/unreal/verify_wp12_normalized_npc_animation_intake.py`. Its receipt,
`Saved/Diagnostics/WP12_region01_normalized_npc_intake.json`, records all six
roles as 26-bone `SkeletalMesh` assets with their matching 26-bone `Skeleton`
and `AnimSequence` assets. Every normalized idle is 60 keys / 2.458 s and
every walk is 41 keys / 1.667 s. This is a native asset and skeleton
compatibility pass only; the derivative assets live in the ignored developer
intake mount and are not production content or a committed vendor conversion.

## Palette/material diagnosis and developer fixture

The source-material probe found UE's generated source material instance has
no texture parameter. Read-only Blender inspection instead confirms the
authored palette mapping: the selected Captain mesh uses a single UV row at
`V = 0.5`, the supplied source has a 256-by-1 palette texture, and that image
is connected in the authored Blender material. The direct fixture therefore
uses a separately factory-imported palette texture and a diagnostic emissive
material solely to make the real voxel colours legible during inspection.

The direct intake fixture rendered six supplied meshes in a blank unsaved map,
recorded their bounds at its `0.5` actor scale, and captured visible idle
colours. Its non-zero direct walk capture is the visual evidence that the
15-bone route is unsuitable. A normalized pose capture was not completed in
this run because the editor console split the first mode-passing invocation;
the launcher now uses a dedicated wrapper for the next attempt, but no
normalized visual receipt exists yet. None of this establishes a production
material, lighting choice, collision, navigation, interaction, dialogue, or
world placement.

## Remaining gate

The asset-to-game integration gate remains open. First use the normalized
26-bone intake to capture and inspect all six candidates at a non-zero walk
frame, then select and validate a production material/scale path. Only then
compose the actual Region 01 map, place the landmarks and role actors, wire
their real interactions to the fact ledger, and run production `REG-01`
through `REG-05` and `REG-09` through `REG-11`. All of those production cases
remain **NOT_RUN**.
