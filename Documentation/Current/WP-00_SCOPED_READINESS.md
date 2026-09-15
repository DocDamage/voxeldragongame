# WP-00 scoped readiness — September 15, 2026

**Boundary:** scoped archive/descriptor and native sample inspection recorded.
The replacement C: UE 5.8.2 build, six native tests, bootstrap and focused
keyboard/mouse PIE checks passed. Full readiness acceptance and WP-01 remain
NOT_RUN. GeoForge 5.0.0 has now been restored to C: with native runtime/editor
and dependency load PASS; see [installation recovery](GEOFORGE_INSTALL_RECOVERY.md).
Terrain suitability and the remaining content/owner inspections are still open.
Earlier D: findings below remain historical evidence. Latest [owner inspection](WP00_OWNER_IMPLEMENTATION_INSPECTION.md)
and [native real-asset review](WP00_REAL_ASSET_REVIEW.md) supersede earlier
filename-only/static-assembly NOT_RUN statements within their stated scopes.
Inventory source on D: remains unreadable. See [handoff](HANDOFF.md).

**Subsequent source/native inspection:** [GeoForge completion report](GEOFORGE_COMPLETION_INSPECTION.md)
records actual edit/save/mesh ordering, engine collision and navigation boundaries,
and vendor drop/water/save ownership conflicts. Native probes reproduced support
priming returning before edited geometry, even for synchronous cross-chunk edits.
The single-chunk synchronous control updated its mesh. This supersedes the old
source-read failure below, but does not clear WP-00 or any full WRLD/SAVE case.

**Further bounded progress:** the [synchronous bridge probe](GEOFORGE_SYNC_BRIDGE_PROBE.md)
compiled against C: headers, passed all six scaffold regressions with GeoForge
loaded, and passed actual PIE collision and nav-projection checks. Explicit visual
refresh completed both sides of a dig; native nav-data submission enabled new lower
navigation and removed it after refill. This is not general async/streaming
completion, traversable-cave/path-cancellation proof or completed owner readiness.

## Actual environment

- Project: `G:\assets\voxel project\WYRMFALL.uproject`; association 5.8;
  runtime module WYRMFALL. Git origin is DocDamage/voxeldragongame, branch main;
  initial clean commit `32407491f6cd9bb34848ffb6fb59a2d82be14904`.
- Current inspected engine: `C:\Program Files\UE_5.8`, UE 5.8.2 CL 56702186,
  compatible CL 55116800. The earlier D: copy failed with device I/O errors.
- Tooling interpreter: installed Python 3.12.10. Default `python` is 3.10.
- UBT selected VS 2022 MSVC 14.44.35227 and SDK 10.0.26100.0.
- CPU: Intel Core i5-14600K, 14 cores / 20 logical processors.
  UBT reported 47.77 GB physical memory. GPU inspection found NVIDIA RTX 3060
  (driver 32.0.15.9649, current 1920x1080 output) and Intel UHD 770.
  Game resolution, graphics settings, frame/edit latency, VRAM usage and payload
  performance are NOT_RUN / NOT_MEASURED.
- `Saved/Diagnostics/doctor.json` records the required EnhancedInput 1.0,
  GameplayAbilities 1.0, Mutable 1.8.0, PythonScriptPlugin 1.0 and
  EditorScriptingUtilities 1.0 descriptors, hashes, modules and paths.
  Mutable's runtime module remains `CustomizableObject`; no creator substitution.

## Scoped supplied content

Root inspected: `G:\assets\voxel project\assets and old docs`.
The bounded inventory recorded 1,461 matching files without truncation. The tool's
extension filter does not inventory RAR contents; three relevant RARs were listed
separately with installed 7-Zip. Evidence is under `Saved/Diagnostics`:
`G0Inventory.json`, `G0ScopedMetadata.json`, and the `*_metadata.txt` records.

| Requirement | Actual located bytes / metadata | Remaining proof |
|---|---|---|
| Terrain materials | `voxel/terrain and paths/PathAndTerrain-gltf.zip`: 373 glTF, 373 binary buffers, 29 PNGs | Dirt/stone samples imported with texture/material and bounds; smooth provider adaptation NOT_RUN |
| Humanoid | `voxel/characters/knights.zip`: 50 FBX files; knight imported through current UE importer with 15 bones | Scale/material completion, animation compatibility and Mutable recipe NOT_RUN; legacy importer failed with multiple roots |
| Horror | `voxel/characters/horror characters.fbx` located | Model-role match and usable animation coverage NOT_RUN |
| Gear | `voxel/Premium_Armory.zip` located | Weapon import, pivots/sockets and animation compatibility NOT_RUN |
| Green Dragon | `Voxel+Dragons+Pack+Upload.zip`, entry `GLTF/Green Dragon.gltf`: 44 meshes, 44 materials, 88 embedded images, 20 animations, zero source glTF skins | Unreal imported 44 skeletal parts, one 195-bone hierarchy and 20 AnimSequences; assembly, scale, visual clip usability, riding and compact combat NOT_RUN |
| Building | `EasyBuildingSystemV10/EasyBuildingSystemv10.uproject` associates with 5.8 and includes real `.uasset` content | Blueprint behavior/compatibility NOT_RUN |
| UI | `ShowcaseMainMenuandNotifi/ShowcaseMainMenuandNotifi.uproject` associates with 5.6; Aurelith Forge archive also located | Actual widgets, controller focus and 5.8 import NOT_RUN |
| Water | `UE_WaterlinePRO6_DownloadPiratecom.rar` and `UWS_DownloadPirate.com.rar` located and manifest listings recorded | Modules, single water-state authority and runtime import NOT_RUN |

Green Dragon animation names read from glTF: Idle 01, Death Pose 01, Eat 01,
Flying 01, Run 01, Take off 01, Detection 01, Chase 01, Death 01, Combo Attack 03,
Combo Attack 02, Falling 01, Combo Attack 01, Get Hit (Air) 01, Attack 01,
Idle Float 01, Landing 01, T Pose, Get Hit 01, Walk 01. Names do not establish
clip usability or a living-defeat presentation.

Provenance is recorded as user-supplied local archives and installed plugin
descriptors. The inspected selected ZIPs contained no filename matching the
license/licence/readme/credits search. Purchase receipts and redistribution rights
were not inspected or inferred. No paid content was redistributed or replacement
art generated. Archive filenames do not establish licensing either way.

## One terrain candidate to investigate first

**First candidate: GeoForgeRuntime 5.0.0**, now restored at
`C:\Program Files\UE_5.8\Engine\Plugins\Marketplace\GeoForge33217d52984fV2`.
The fresh Epic manifest is complete, and its runtime/editor modules and
ProceduralMeshComponent load in the actual C: 5.8.2 editor. It is **not selected
as terrain authority or integrated**; terrain behavior remains NOT_RUN.

Why first: its actual installed descriptor targets 5.8.0 and Win64 and provides
`GeoForgeRuntime` (Runtime) and `GeoForgeRuntimeEditor` (Editor) modules. Its
`UnrealEditor.modules` BuildId is 55116800. The runtime build file depends on
Core, CoreUObject, Engine, NavigationSystem and ProceduralMeshComponent. The
descriptor names CelestiaDominance and links the vendor documentation and Fab
listing. These are metadata facts, not binary/source compatibility proof.

In comparison, `VoxelPro2_DownloadPirate.com.rar` contains 5.6 and 5.7 builds.
The inspected 5.7 descriptor is Voxel `dev-17314`, EngineVersion 5.7.0, with
VoxelCore/VoxelGraph/Voxel and related modules. It is not a verified 5.8 build.
No alternate stacks were enabled.

Inspected `GeoForgeInfiniteTerrainActor.h` exposes:

| Concern | Actual header evidence | Unresolved boundary |
|---|---|---|
| Smooth surface | `EGeoForgeChunkRenderMode::MarchingCubesSurface`, iso level and sharpness settings | Runtime geometry and collision agreement NOT_RUN |
| Removal/addition | `DigSphere`, `AddSphere`, `AddSphereWithBlock`, single-cell/generation operations | A void return or edit count cannot attest collision/nav completion |
| Collision | `bGenerateCollision`; planet-specific readiness queries | General planar collision completion contract not established |
| Navigation | `bStreamedTerrainAffectsNavigation`, navigation-invoker settings | Newly exposed surface readiness and stale-route cancellation NOT_RUN |
| Request result | `FGeoForgeTerrainEditRequestCompleted` carries requester, request ID, operation, applied flag and edit count | Its implementation timing was not verified; it must not be treated as geometry/collision/nav completion |
| Persistence | `BuildTerrainSaveData`, `ApplyTerrainSaveData`, compatibility ID and generation fingerprint | Full edit/depletion/inventory save transaction NOT_RUN |
| Materials | Block specification, material layers and refresh APIs | Real source-material import/adaptation NOT_RUN |

The historical D: private-source search failed with OS error 433, “A device which
does not exist was specified.” Required implementation/callback semantics therefore
remained unresolved at that time. The C: source is now readable and the exact
implementation was inspected in [the current report](GEOFORGE_COMPLETION_INSPECTION.md).
Do not set `HasMinimumG1APIs` flags or award rewards from header names or acceptance
notifications; a validated edit-completion bridge remains missing.

## Owners and conflicts

GAS is the intended combat authority; the authored humanoid owns its ASC and
attributes. Enhanced Input routes this single humanoid/controller. Mutable is the
required creator. The terrain seam still fails closed with no provider.

Inventory, equipment, interaction, water, building and persistence owners remain
unselected/unimplemented in the project. The supplied Easy Building System has
`BP_EBS_BuildingComponent`, `BP_EBS_InteractionComponent`,
`BP_EBS_ResourcesComponent`, `BP_EBS_SaveGame`, and `BP_EBS_SaveLibrary` packages.
Their **filenames only** were inspected; do not activate their resource/save owners
alongside new project owners without examining the actual Blueprints. No second
health/inventory/save implementation was added.

## Acceptance state and next bounded task

- RDY-01 environment inspection: PASS for recorded facts, **not G0 or BOOT-01**.
- RDY-02 complete selected asset opening/import/bounds: NOT_RUN; native samples
  below are partial evidence; enemy/weapon and assembled visual checks remain.
- RDY-03 full owner implementation inspection: NOT_RUN; source/filename findings
  above are partial evidence.
- RDY-04 complete baseline/provenance review: NOT_RUN; metadata findings above
  do not establish the full case.
- WP-00 is incomplete at the provider/remaining inspection boundary. WP-01 and all WRLD/SAVE
  gameplay checks remain NOT_RUN. No G0/G1 acceptance pass is claimed.

BOOT-01 was exercised on C:. GeoForge availability and native loading are now
verified. Inspect the exact private implementation, open the
selected real terrain/resource assets, and resolve owner conflicts. Proceed to
one WP-01 proof only when those inputs and completion contracts are established.

## Native sample intake on C: — current evidence

Source archives remained unchanged. Exact selected entries, extracted paths,
byte counts and hashes are in `Saved/AssetIntake/WP00/selection.json`. Native
packages were written only under `/Game/WYRMFALL/Development/Intake/WP00`.
No asset was assigned to the diagnostic player or made a gameplay owner.

| Sample | Actual native observation | Remaining limit |
|---|---|---|
| `gltf/Dirt/dirt-1.gltf` | StaticMesh + MaterialInstanceConstant + Texture2D; bounds approximately X/Y -160..160 cm, Z 0..20 cm | Material adaptation to smooth editable terrain and visible provider behavior NOT_RUN |
| `gltf/Stones/stone-1.gltf` | StaticMesh + material instance + texture; X -20..30 cm, Y -20..20 cm, Z 0..20 cm | Possible supplied resource visual only; finite deposit/state/yield owner NOT_IMPLEMENTED |
| `TVS_VoxelKnights_Knight.fbx` | Legacy FbxFactory failed with multiple bone roots. Current importer succeeded with one SkeletalMesh, Skeleton, PhysicsAsset and material; 15 bones. Imported bounds origin about (0.605,-28.768,180), extent (170,55,180) cm | Approximate 360 cm height is not the accepted character scale. Texture completion, selected idle clip import, retargeting, collision and Mutable usability NOT_RUN |
| `GLTF/Green Dragon.gltf` | 241 packages: 88 textures, 44 material instances, 44 skeletal meshes, 44 physics assets, one skeleton and 20 animation sequences. Components expose 195 imported bones. For example Chest-Local bounds origin roughly (0,-857.532,4994.357), extent (1400,2000,1900) cm | Source node animation became a skeletal hierarchy on import despite no source glTF skins. Large raw units require deliberate scale/assembly inspection. A usable assembled dragon, clip playback, rider/compact combat and persistent identity remain NOT_RUN |

The 20 imported Green Dragon clip lengths (seconds) were read in Unreal:
Attack 1.033; Chase 0.567; Combo Attack 01/02/03 1.033/1.133/2.300;
Death 1.267; Death Pose 0.033; Detection 0.867; Eat 10.433; Falling 1.000;
Flying 1.000; Get Hit 1.333; Get Hit Air 1.000; Idle 10.000; Idle Float 1.000;
Landing 1.633; Run 0.567; T Pose 0.033; Take off 1.267; Walk 1.767.
Metadata duration is not observed animation/gameplay suitability.

Evidence: `Saved/Diagnostics/WP00_native_samples.json` and
`WP00_native_rigs.json`. The former retains the legacy knight import failure;
the latter records its successful current-importer retry and complete numeric
rig/bounds inspection. Initial Python report serialization/API errors were
corrected; earlier logs remain. Existing successful imports were loaded for
inspection, not overwritten on retry.

Exact native invocation used the C: `UnrealEditor-Cmd.exe` with the project,
`-unattended -nop4 -nosplash -nosound -nullrhi`,
`-ExecutePythonScript=G:/assets/voxel project/Saved/Diagnostics/<script>.py`,
`-stdout -FullStdOutLogOutput`, through `wyrm_process.execute_logged`:

- `import_wp00_samples.py`: `20260915T141309Z_a1cf1bf4ec74_wp00-import-samples.log`
  (report serialization failed), then
  `20260915T141356Z_e5e00004d227_wp00-import-resume.log` (three imports succeeded;
  legacy knight import failed).
- `inspect_wp00_rigs.py`: `20260915T141539Z_5ba041b44689_wp00-inspect-rigs.log`
  (current knight importer succeeded; bounds API corrected afterward), then
  `20260915T141658Z_f093791c0068_wp00-rig-bounds.log`
  (all 45 imported skeletal parts inspected, no inspection errors).

These logs are in `Saved/ScaffoldLogs`. A process exit 0 alone was not treated
as import success; returned assets, saved packages and inspection reports were read.

### Precise missing-input boundary

ProceduralMeshComponent is already bundled with the C: engine and has now been
explicitly enabled in WYRMFALL. An actual Unreal load/instantiation probe passed;
see [dependency evidence](PROCEDURAL_MESH_DEPENDENCY.md). GeoForge installation
and native loading are now verified in [the recovery report](GEOFORGE_INSTALL_RECOVERY.md).

1. RESOLVED: the approved GeoForge UE 5.8 copy is installed on C: and its native
   runtime/editor/dependency loading passed. The user authorized cache recovery
   and the installation retry. The former D: source read failure is historical.
2. Exact edit/save implementation and geometry/collision/nav boundaries have now
   been inspected. Native probes disprove `DigSphere` plus immediate support
   prime as a completion barrier, including across chunks with async disabled.
   See [source references, control case and reproduction](GEOFORGE_COMPLETION_INSPECTION.md).
   A [small synchronous diagnostic bridge](GEOFORGE_SYNC_BRIDGE_PROBE.md) now has
   compile/native-regression/PIE collision/nav-projection evidence. General
   streaming/action-revision completion remains unverified; no capability flag
   was granted. Full terrain gameplay/save tests remain NOT_RUN.
3. Selected enemy/weapon opening, completed material/scale/assembled-asset review,
   full building/water/inventory/save owner inspection and complete provenance
   acceptance remain NOT_RUN. Their existing bytes/partial findings remain
   recorded above. Missing later vehicle/colony content does not block terrain.

The bounded WP-00 discovery now has an actionable missing-input list. It does
not clear the full readiness cases. WP-01 implementation and every unexecuted
WRLD/SAVE/performance validation remain **NOT_RUN**.

### Later owner inspection - September 15, 2026

[Native graph inspection](WP00_OWNER_IMPLEMENTATION_INSPECTION.md) now covers eight
EBS and five Waterline Blueprints, with source hashes unchanged. EBS resource,
interaction, building and slot-save implementations were traced. Waterline has
readable visual and physical paths; neither package is integrated or selected.
Advanced Inventory System source on D: is unreadable (device error 433), with no
matching C: engine copy located. This supersedes earlier filename-only owner
inspection statements, not their runtime NOT_RUN status. Full RDY-02/03/04 and
WP-01 remain NOT_RUN. Next: independent real-content review while preserving the
inventory source blocker and single-owner constraints.

### Later native real-asset review - September 15, 2026

[Real-asset review](WP00_REAL_ASSET_REVIEW.md) supersedes earlier NOT_RUN statements
for selected enemy/weapon opening, static dragon assembly and bounded palette/
scale inspection only. Actual native wolf/sword imports and two inspected renders
include the knight, all 44 Green Dragon parts, wolf, sword, dirt and stone.
Preview scales are explicit; animation, collision, Mutable usability, production
scale/material acceptance and full RDY-02/03/04 remain unpassed. Inventory source
on D: remains unreadable; another local source/archive path has been requested.
No provider/owner was selected and WP-01 remains NOT_RUN.
