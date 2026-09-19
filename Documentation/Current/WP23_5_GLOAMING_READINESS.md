# WP-23.5 Gloaming Marches readiness evidence

**Result:** PASS for bounded import/content readiness on September 19, 2026.
**Production status:** GATED. No Gloaming map, Nyxaroth rig profile, or gameplay
acceptance is claimed.

## Command and receipt

Executed with UE 5.8.2 CL 56702186:

```text
UnrealEditor-Cmd.exe WYRMFALL.uproject -unattended -nop4 -nosplash -nosound
-nullrhi -ExecutePythonScript=tools/unreal/inspect_wp23_5_gloaming_readiness.py
-stdout -FullStdOutLogOutput
```

The command exited 0 and wrote
`Saved/Diagnostics/WP23_5_gloaming_readiness.json` with status `PASS`.
Imported intake is diagnostic, ignored content under
`/Game/WYRMFALL/Development/Intake/WP23_5`; the tracked script recreates it
from the supplied archives.

## Nyxaroth finding

`Voxel+Dragons+Pack+Upload.zip::GLTF/Dark Dragon.gltf` contains 194 nodes, 33
meshes, 20 animations, 33 materials, and 66 embedded images. UE imported one
`Hip-Local` leader, 32 follower skeletal meshes, one skeleton, 20 animation
sequences, and 33 material instances. Named motion coverage includes idle,
walk, flying, takeoff, landing, and attacks.

This is an import-feasible modular candidate, structurally close to the proven
Jadefang source. It is not a validated production rig: the receipt deliberately
records `profile_validated: false`.

## Gloaming content fit

`Voxel Cathedral.zip` supplies four character FBXs, six environment FBXs,
eight humanoid animation FBXs, and 16 textures. The focused import succeeded
for three skeletal characters (Crusader, Priest, Nun) and five static region
meshes (Cathedral, CrossGrave, Grave, Statue, Tree). Proposed roles are
Ashgrave/Crusader, Count Malvaine/Priest, and two separately authored Hollow
Twins based on Nun.

`cemetary and church voxel set.zip` contains a nested 338-entry modular set
with 84 OBJ meshes and 168 PNG textures, but no FBX. It is a region-detail
candidate whose material, collision, scale, and route readability still need
visual/editor proof.

`horror characters.fbx` is retained as required Gloaming content. UE imported
331 separate StaticMesh body parts rather than ready-to-play skeletal
characters, and the source names identify its intended horror roster. This
clears source availability but makes production fit conditional on an authored
modular assembly/animation approach, collision and scale proof, material review,
and documented permission/provenance.

Production-facing actor names are original WYRMFALL puns while source asset
names remain intake-only metadata: Ail-Yen, Annie Wails, Scarrie, Chuckles,
Count Dripula, Frank N. Shrine, Dready Freddie, Roastface, the Gravy Daughters,
Canniball, Bellraiser, Knit, Machete Mason, Pleatherface, Michael Mire,
Dreadator, Pyre-Midhead, Sad Echo, Mum's the Wyrd, and Wherewolf. These aliases
reduce direct naming overlap but do not by themselves establish permission.

## Unrun production gates

- No fail-closed Nyxaroth `FWyrmDragonRigProfile` was implemented or compiled.
- No Companion/TrueForm dimensions, Heartfold, mount, flight, direct control,
  GAS combat, living bond, or save behavior was run in PIE.
- No Gloaming region map, navigation route, ruler encounter, Hollow Twin
  encounter, optional Echo outcome, travel/return, or recovery case exists.
- Selected Cathedral and cemetery content has not received interactive visual
  QA for scale, materials, collision, lighting, or route readability.
- The required horror roster has not yet been assembled into authored actors or
  proven for animation, collision, scale, materials, or permission/provenance.

## Decision

The readiness blocker is narrowed from unknown source suitability to explicit
runtime authoring work. Begin with a bounded Nyxaroth profile proof. Do not
start regional composition until it passes.
