# First Implementation Assignment — G0 Readiness and One G1 Terrain Proof

**Documentation release:** v0.1 · September 14, 2026  
**Status:** Detailed design proposal; not an implementation report  
**Basis:** H01 §§7,25–29,32–33; A01 G0/G1; B03 updated gates; see [source register](../SOURCES_AND_EVIDENCE.md).  
**Rule:** Existing requirements remain binding. New numbers and detailed behavior are draft baselines for a bounded prototype, not claims of user approval or runtime validation.

## 1. Current boundary

This is a ready-to-use future implementation assignment. The present documentation task has not located an Unreal project, enabled a plugin, imported content or run G0/G1. Actual project/repository/branch, installed engine patch, plugin builds and asset paths remain VERIFY. Do not borrow paths from the user's other projects.

G0's useful outcome is an accurate environment/content/owner inventory. G1's useful outcome is one real-content editable-world proof or one reproducible candidate blocker. It is not an entire game, all terrain integrations, a new engine fork or production of Region 01.

## 2. Inputs and discovery

| Input | Current state | G0 action |
|---|---|---|
| Game `.uproject` and root | NOT INSPECTED | Locate within the explicitly authorized workspace; read modules/maps and working-tree status. |
| Engine build | H01 target UE5.8.x, preferably5.8.2; actual unknown | Read real build/version association; do not silently migrate. |
| Candidate terrain source/binary | Shortlist documented; actual unknown | Inspect available versions and required operations, choose one credible first candidate. |
| Terrain materials | Reported source categories | Locate actual approved material assets and test adaptation, not substitute generated art. |
| Humanoid collision/movement actor | Actual unknown | Reuse existing appropriate project actor/content for proof; no second final player architecture. |
| Resource visual and finite-yield record | Design specified; content unverified | Choose real source mesh/material and stable ID; data fixture may be authored. |
| Inventory/state owners | Exact existing implementation unknown | Inspect ownership/serialization, do not create parallel temporary economy later forgotten. |
| Build/test hardware | Planning target includes RTX3060 12GB, actual unmeasured | Record CPU/GPU/RAM/resolution/graphics settings and build type; no assumed benchmark. |

If a required input truly cannot be found through scoped discovery, report it precisely and stop that dependent step. Do not inspect unrelated personal folders, import every library pack or ask the user to repeat an already supplied path. Source-library ownership claims remain distinct from accessible bytes.

## 3. Bounded proof scene

Use a single proposed development map such as `L_DEV_VoxelWorld` only if it matches the actual project's conventions. It needs a hill/cave, diggable wall, add/fill area, one finite resource deposit, one ordinary and one newly dug navigable route, a humanoid arrival point, real material assignment and a compact water-adjacency fixture. Diagnostic overlays/collision primitives are permitted only as test instrumentation, not production artwork substitutions.

Define a fixed seed/test world, actual baseline provider version and a representative extent/streaming configuration. Start small enough to isolate correctness, then exercise the agreed representative region/streaming extent before claiming provider suitability. A tiny editor sculpt can establish an API call works, not that the project terrain requirement is met.

## 4. Required sequence

Run baseline collision/traversal; remove ground to open a real tunnel; wait for authoritative completion; walk through it; add/fill a section and verify it blocks again. Use both cameras and test direct movement plus a nav-controlled actor/destination through the new surface. Cancel stale routes after refill.

Extract finite deposit units, inspect remaining budget, fill/re-mine the same area and confirm no original ore returns. Force full inventory and verify accepted reward remains represented once. Save/quit/reload; inspect terrain, collision, depletion and item IDs. Repeat the same accepted callback/action ID and confirm no additional payout.

Test protected structural envelope/occupied-fill rejection, allowed adjacent excavation, bounded wet-bed edit and rejected water-boundary break. Do not implement global fluid simulation or a second structural-stability solver just to prove the initial water/support rules.

Exercise a declared repeat-edit sequence (proposed 100 accepted dig/add operations), multiple nearby deposits and representative streaming travel. Record every relevant failure. This is one provider stress fixture, not a performance pass achieved by averaging away collision errors.

## 5. Measurements and pass decision

Record operation count/shape/extent, average and tail edit/collision readiness latency, navigation readiness, frame behavior during edits, peak memory/VRAM where accessible, terrain payload size, capture/write/load time and machine settings. These measurements are currently **NOT RUN**. Set production budgets after the first observations and user target constraints; no invented 60FPS claim.

Pass requires actual core behavior and no observed state-loss/duplication in the scoped cases. The provider can be provisionally selected only with clear remaining limitations and their impact. If a mandatory capability fails, capture reproduction, environment, logs and what was ruled out; fix the narrow issue or evaluate the next candidate. Stop comparisons once one meets the requirements.

G1 does not pass by disabling terrain addition, limiting edits to an irrelevant box, using prebuilt static caves instead of exposing a cave, disabling AI navigation, dropping water gameplay or failing to save mined resources. Each partial success must be reported separately.

## 6. Small verification set

Run relevant RDY and WRLD cases plus SAVE-01/02/03/04/06/07 in the [acceptance catalogue](ACCEPTANCE_TESTS.md). Add focused deterministic checks around deposit/reward/serialization if not already present. Use PIE for visible collision/navigation/material behavior. A cooked check is justified for a specific risky runtime dependency; do not rebuild the engine or cook after each edit.

Do not create CI, Git issues, automatic branches or PRs for this assignment. Preserve unrelated local work. The user's next code authorization governs permitted files/Git actions, not this document alone.

## 7. Ready-to-paste assignment

> Work only on WYRMFALL G0 readiness and one bounded G1 terrain-provider proof. Read AGENTS.md, the current handoff, plugin/asset records, editable-world rules and shared save contract. Locate the actual authorized project and engine build before changing code. Keep Mutable/GAS and one-owner rules; use real approved materials/content. Choose one available terrain candidate, prove smooth dig/add/cave collision, finite resource yield, new-surface navigation, both camera implications and coherent save/quit/reload. Include the documented protected-site and bounded-water edge cases. Record actual measurements and results. Do not create multiple terrain stacks, production regions, replacement art, CI or unauthorized Git writes. Stop when the bounded checks pass or a reproducible blocker is documented. Report exact files changed, commands/actions run, results, missing inputs and one next task.

## 8. Required report

A useful report has actual project/version/asset paths, chosen candidate/reason, changed files, commands/PIE actions, relevant test IDs with results, measurements or “not measured,” screenshot/log locations, unresolved blockers and next task. No large framework or completion percentage is required. Import success does not imply cooked/runtime/save success.
