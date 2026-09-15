# Plugin Ownership and Compatibility Matrix

**Documentation release:** v0.1 · September 14, 2026  
**Status:** Detailed design proposal; not an implementation report  
**Basis:** H01 §§7–12,21–29; C01; A01; R1–R6; see [source register](./SOURCES_AND_EVIDENCE.md).  
**Rule:** Existing requirements remain binding. New numbers and detailed behavior are draft baselines for a bounded prototype, not claims of user approval or runtime validation.

## Selection rules

One authoritative implementation owns each responsibility. An adapter can translate a provider result into project intent; it must not become another inventory, health system, terrain engine, or universal plugin framework. Availability in a library is not sufficient reason to enable a plugin.

**Engine target:** H01 specifies Unreal Engine 5.8.x, preferably 5.8.2. **Installed engine build, project path, plugin versions and compatibility: NOT INSPECTED.** Do not replace that target with an assumed older/newer engine or claim any candidate works without G0/G1 evidence.

| Responsibility | Direction / candidate | Selection status | Exclusions / adapter and proof |
|---|---|---|---|
| Runtime customization | Mutable / Customizable Objects | REQUIRED backbone | No parallel non-Mutable creator. Recipe/appearance adapter only; G2 runtime and cooked proof. |
| Combat attributes/effects/abilities | GAS | H01 direction retained | Advanced Health/Shield/DBNO must not also own health. Map any retained visual helper to GAS data only. |
| Input routing | Enhanced Input | BASELINE native choice | One contextual route, no second player implementation; resolve real plugin/version names in G0. |
| UI navigation | Existing supplied UI + suitable Common UI use | Integration candidate | Common UI is not a second game-state owner. Inspect existing widget structure before choosing navigation layer. |
| Humanoid animation | Existing animation blueprint plus IK Rig/Retargeter | BASELINE | Source assets determine compatibility. Enable Control Rig only for a concrete need. |
| NPC appearance population | Same recipes; Mutable Population later | Later candidate | Not necessary for one-player G2 proof; no early crowd simulation requirement. |
| Smooth runtime terrain | One of shortlist below | VERIFY / unselected | Thin edit/completion/save/nav adapter after proof; never maintain all candidates. |
| Terrain material generation | Material Builder for Voxel Landscape if useful | Candidate | Material tool is not terrain authority. Confirm output works with chosen provider. |
| Inventory / stash | One owned inventory product, exact product unidentified | VERIFY / unselected | Stable instances, ownership transfers, overflow, serialization; select on real project evidence. |
| Equipment | Existing compatible equipment owner with project rules | VERIFY / unselected | Own slots and GAS grant handles; no duplicated stats in UI/creator. |
| Interaction | One owned interaction implementation, exact product unidentified | VERIFY / unselected | Intents for doors, mining, NPCs, mounts, crafting; no separate “car interaction” stack. |
| Constructed buildings | Easy Building System | Preferred candidate, not certified | One support/snapping owner; custom stability repo only after specific insufficiency. |
| Water | Waterline Pro 6 | H01 preferred candidate, not certified | One water-state authority; test swimming/fishing/underwater volumes independently of shader. |
| Water visuals supplement | Ultimate Water Shader | Optional candidate only | No second water simulation/state owner; use only a demonstrated visual need. |
| Footstep presentation | Auto Footstep Utility | Candidate | Consume authoritative surface/contact; no duplicate step events or hidden movement changes. |
| Audio/music/compass/ping | Existing supplied utilities | Unspecified candidates | Pure presentation of real events/state. No new quest owner or global soundtrack system assumed. |
| Enemy/companion AI | Small existing Unreal AI approach | BASELINE / inspect existing | One understandable actor decision flow; no Mass/crowd stack merely because available. |
| Dragon framework | Project gameplay over actual dragons | REQUIRED behavior, implementation new | Same identity across boss/ally, size/control/locomotion dimensions; no four duplicate frameworks. |
| Flying vehicles | One project hovercar implementation or suitable existing owned solution | VERIFY later | Share possession/save/interaction contracts. Do not assume wheel-vehicle physics fits a hovercar. |
| Persistence | One project coordinator + native storage + selected provider payload | BASELINE | SG_VoxelCharacters becomes data under this owner, not competing save authority. |

## Terrain shortlist: preserve options without integrating them all

| Candidate from H01 | Actual version/path | First question | Rule for proceeding |
|---|---|---|---|
| Voxel Pro 2 / Voxel Plugin candidate | NOT INSPECTED | Does the available build meet target engine, edit/collision/nav/save/material needs? | Bounded G1 proof; documented APIs alone do not pass. |
| GeoForgeRuntime | NOT INSPECTED | Are smooth runtime edit/add/persistence and required collision exposed in the real available source? | Consider if first candidate is unavailable or has a demonstrated blocker. |
| draquel/VoxelWorlds | NOT INSPECTED | Actual license/build/import and supported runtime operations? | Reference/candidate, not automatic dependency. |
| UnrealSandboxTerrain | NOT INSPECTED | Does the actual fork/build satisfy the complete finite-region proof? | Inspect scoped relevant implementation, not unrelated infrastructure. |
| MatterFlux-related technology | NOT INSPECTED | Which specific reusable capability solves an actual missing requirement? | No wholesale import based on project name. |
| Structural-stability experiments | NOT INSPECTED | Is a required support behavior missing from selected building/terrain solution? | Not a reason to add a second structural simulation. |

Choose the first available candidate with credible target-version support and complete needed APIs. Record why that one was tried first. Run the bounded proof, then stop comparing when it passes. A failed narrow test should produce a reproducible blocker and recovery choice, not five permanent branches of terrain code.

## Acceptance needed before selecting an owner

For every candidate record actual name/build, source path, target module dependency, runtime versus editor-only scope, author/license metadata, relevant existing implementation, API boundary, failure behavior, save compatibility, and measured proof result. “Selected provisionally” means exactly that; it does not mean imported or cooked successfully.

Inventory selection must demonstrate no duplicate equipment grants, preserved item rolls, one-time rewards and atomic-ish committed transfers. Building selection must demonstrate valid supports, blocked placement, demolition/refund and stored items. Water selection must demonstrate consistent wet/dry queries, swimming, fishing and bounded editable-bed behavior. These tests matter more than the number of product features advertised.

## Conflict-resolution order

Keep the approved product requirement. Reuse the chosen owner where it genuinely supports the behavior. Add a thin bounded project rule where necessary. Investigate or change the candidate if a real blocker remains. Any proposal to remove required gameplay, change engine target, purchase content, or manufacture replacement artwork must be explicit—not disguised as an integration shortcut.

Potential reference repositories named by H01 are not this game's authoritative repository. This documentation pack does not discover or alter any repository, branch, build configuration, plugin enable list, or vendor code.
