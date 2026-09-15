# Current implementation status

**September 15, 2026 · starter v0.2 · C: native build/tests and focused PIE exercised**

This status supersedes only the old documentation pack's statement that no scaffold exists. It does not change approved design or grant broader Git/art permissions. The uploaded pack's 36 files remain byte-for-byte preserved under Documentation/DesignPack.

| Area | Actual status | Evidence / next proof |
|---|---|---|
| Source project | AUTHORED | WYRMFALL.uproject, targets, runtime module, Config |
| Portable tooling | Test results recorded in root VALIDATION.md | Standard-library tests; no engine emulation |
| Engine on this Windows host | UE 5.8.2 VERIFIED on C: | `C:\Program Files\UE_5.8`, CL 56702186; old D: failure retained as history |
| Windows UE installation | Five required descriptors INSPECTED | Mutable 1.8.0, EnhancedInput, GAS, Python and EditorScriptingUtilities; doctor is metadata evidence |
| UHT / UBT compile | PASS | Actual headers; final project build `-NoUBA -NoPCH` succeeded in 18.40 seconds |
| Native automation | 6/6 Success | Fresh `Saved/Automation/Scaffold/index.json`, no warnings/failures/not-run cases |
| Diagnostic map | SAVED and unchanged on bootstrap rerun | Real 17,817-byte `.umap` and validated receipt |
| PIE, input, cameras, HUD | Focused keyboard/mouse checks PASS; controller NOT_RUN (absent) | Rebinding, pause/ignore guards, click rejection and editor relaunch exercised; fixture lighting warning remains; no WP-03 pass |
| Mutable | REQUIRED; plugin enabled in descriptor, runtime content NOT INTEGRATED | WP-02 real recipe/runtime/cooked proof |
| Terrain | Provider NOT SELECTED; GeoForgeRuntime 5.0.0 installed on C: | Complete Epic manifest; native runtime/editor load PASS; completion/save semantics and terrain behavior NOT_RUN; [repair](GEOFORGE_INSTALL_RECOVERY.md); WP-01 NOT_RUN |
| GeoForge prerequisite | ProceduralMeshComponent bundled and native load PASS | Explicitly enabled in the project and loaded successfully alongside GeoForge; [dependency evidence](PROCEDURAL_MESH_DEPENDENCY.md) |
| Real sample assets | Native dirt/stone/knight/Green Dragon imports inspected | Bounds/bones/clips recorded; scale/material/assembly/gameplay suitability not established |
| GAS | Native attribute tests PASS; PIE HUD 100/100, no combat loop | WP-04/05 remain gated |
| Saves/inventory/equipment | NOT IMPLEMENTED / owners unresolved | Preserve one-owner requirements |
| Dragon, Heartfold, Echoes | Requirements and integration locations preserved; NOT IMPLEMENTED | Original WP-09 onward |
| Vehicles/colony | Future required deliverables; NOT IMPLEMENTED | Original WP-18/19 |
| Git/remote/branch | Git initialized; origin configured; main | [DocDamage/voxeldragongame](https://github.com/DocDamage/voxeldragongame); existing remote history retained; generated data and supplied/imported vendor assets excluded |

## v0.2 changes

Added bounded onboarding/recovery reports and source/evidence freshness checks;
made the native runner require the source-declared suite; added process-tree
cleanup and unique hashed attempt logs; preserved asset roots during engine-path
updates; strengthened metadata/map receipts. The existing C++ shell now guards
input rebinding/paused movement and clamps GAS base health as well as current
health. These native changes and the typed map checks have now been exercised
on the C: UE 5.8.2 installation, as scoped in the recovery report.

The first-run wrapper is offline by default; `--native` explicitly adds local
engine steps. No new gameplay owner, terrain provider or speculative content
system was introduced. See [changes](../../CHANGELOG.md).

## What Codex should do next

Read [the current C: recovery report](BOOT-01_C_RECOVERY.md). Finish
[WP-00](tasks/WP-00.md) at the missing-provider/remaining readiness boundary;
then one eligible [WP-01](tasks/WP-01.md) proof. Do not jump to the entire first
region. GeoForge's stale D: registration and failing D: cache were recovered with
user approval. Fab now uses `C:\EpicVaultCache\VaultCache`; GeoForge is installed
in the C: engine and native loading passed. Finish the remaining WP-00 inspections.

## Unverified boundaries to keep visible

Input mappings are transient developer defaults rather than final remappable input assets. The top-down fixture has no roof/occlusion, menus, target selection or edit-aware nav invalidation. Camera preference is not saved. The humanoid is intentionally unmeshed. GAS state is owned by the humanoid while it exists; destruction/region transfer reconstruction is not implemented. The terrain seam intentionally omits speculative vendor completion/save APIs. The map builder never creates production materials, rigs, animations or UI.

Later sessions must replace these specific statuses with observed results, not delete the limitations wholesale. Use [the report template](SESSION_REPORT_TEMPLATE.md).

## Current Windows evidence and remaining boundary

[Current C: recovery, commands and PIE evidence](BOOT-01_C_RECOVERY.md) ·
[WP-00 scoped readiness](WP-00_SCOPED_READINESS.md) ·
[Historical D: failure](BOOT-01_WINDOWS_SESSION.md).

Use installed `py -3.12`; default `python` is 3.10. The final C: editor build,
real bootstrap map, six native tests and focused keyboard/mouse PIE checks passed.
The portable suite ran 124 tests with two platform/privilege skips. The old
onboarding receipt correctly reports STALE_EVIDENCE after the engine/source
changes; current individual execution evidence is listed in the recovery report.

WP-00 native sample intake found real dirt/stone meshes and materials, a
15-bone knight through the current FBX importer, and 44 Green Dragon skeletal
parts sharing a 195-bone imported hierarchy plus 20 clips. Legacy knight import
failed with multiple roots; the source FBX was preserved. Scale, material
completion, assembled dragon preview, selected enemy/weapon opening, full owner
implementation inspection and complete readiness acceptance remain unverified.

The C: engine now has GeoForgeRuntime 5.0.0 with native runtime/editor/dependency
loading verified. Its completion, collision, navigation and save semantics still
need readiness inspection. The supplied Voxel archive has only 5.6/5.7 builds.
All WP-01 terrain/gameplay/save/performance checks,
full RDY-02/03/04, connected-controller behavior and cooked validation remain
**NOT_RUN**. No G0/G1 or production feature pass is claimed.

**Next bounded task:** inspect the restored GeoForge UE 5.8 private
completion/collision/nav/save semantics, finish the
remaining WP-00 inputs, then implement one eligible WP-01 proof.
