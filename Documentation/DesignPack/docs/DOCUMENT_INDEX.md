# Document Index and Reading Routes

**Documentation release:** v0.1 · September 14, 2026  
**Status:** Detailed design proposal; not an implementation report  
**Basis:** H01; B03; A01; see [source register](./SOURCES_AND_EVIDENCE.md).  
**Rule:** Existing requirements remain binding. New numbers and detailed behavior are draft baselines for a bounded prototype, not claims of user approval or runtime validation.

## Current authority by topic

| File | Owns | Read when |
|---|---|---|
| [AGENTS](../AGENTS.md) | Development boundaries and reporting | Every coding session |
| [Decisions and scope](DECISIONS_AND_SCOPE.md) | REQUIRED / BASELINE / VERIFY / OPEN; requirement IDs | A rule or scope decision is unclear |
| [Game design](GAME_DESIGN.md) | Normal loop, progression, items, recovery | Building gameplay |
| [Technical architecture](TECHNICAL_ARCHITECTURE.md) | Runtime ownership and integration seams | Connecting systems |
| [Asset registry](ASSET_REGISTRY.md) | Reported content, capability checks, actual availability | Selecting or importing content |
| [Plugin matrix](PLUGIN_MATRIX.md) | Candidate ownership, exclusions, proof required | Enabling dependencies |
| [Region 01](REGION_01.md) | Locations, quest facts, dialogue/rewards, alternate order | Building the first region |
| [Vertical slice](VERTICAL_SLICE.md) | Connected feature boundary and gate exit | Claiming first-playable completion |
| [UI/UX/controls](UI_UX_CONTROLS.md) | Input contexts, screens, camera behavior, accessibility | Building player-facing interactions |
| [Art direction](ART_DIRECTION.md) | Cross-pack scale, materials, regional/horror presentation | Assembling real content |
| [Dragon/Heartfold](systems/DRAGON_HEARTFOLD_SPEC.md) | Verdance profiles, compact kit, boss and mount behavior | G4 / G4-H / first boss |
| [Combat/Echoes](systems/COMBAT_AND_NIGHTMARE_ECHOES.md) | Combat calculations, 20 powers, Relentless Advance, transformation | Combat and E1 |
| [Cars/colony](systems/FLYING_CARS_AND_COLONY.md) | Pilotable vehicle, pet, travel and colony contracts | V1 / Z1 |
| [Mutable integration](systems/MUTABLE_CHARACTER_INTEGRATION.md) | Recipe/appearance/animation/equipment bridge | G2 and equipment |
| [Editable-world rules](systems/EDITABLE_WORLD_RULES.md) | Terrain, resources, water, navigation and protected sites | G1 / region / building |
| [Save/control contract](systems/SAVE_AND_CONTROL_CONTRACT.md) | IDs, snapshot/load order, transitions, failure/recovery | Every persistence or possession change |
| [Activities/building](systems/ACTIVITIES_AND_BUILDING.md) | Recipes, fishing/cooking, camp operations | Secondary gameplay |
| [Campaign progression](production/CAMPAIGN_PROGRESSION.md) | Ten-region capabilities, horror placement, reveal order | Future regional planning |
| [Implementation backlog](production/IMPLEMENTATION_BACKLOG.md) | Ordered bounded work packages and dependencies | Selecting one next task |
| [G0/G1 assignment](production/G0_G1_READINESS_AND_TERRAIN_PROOF.md) | First actual inspection and provider proof | Starting implementation |
| [Acceptance tests](production/ACCEPTANCE_TESTS.md) | Focused procedures, expected outcomes, current NOT RUN state | Verifying a changed feature |
| [Current handoff](production/CURRENT_HANDOFF.md) | Actual status and next bounded work | Starting or ending a session |
| [Sources/evidence](SOURCES_AND_EVIDENCE.md) | Local source provenance and limited technical references | Checking a factual claim |

## Narrative authority

[Story/gameplay bible v0.3](../references/originals/WYRMFALL_Project_Aligned_Story_Bible_v0_3.md) includes the full revised opening. Region 01 translates that treatment into game facts; it does not change the characters' identities or make all draft dialogue immutable.

[Original world bible](../references/originals/wyrmfall-world-bible(1).md), [story bible v0.2](../references/originals/WYRMFALL_Project_Aligned_Story_Bible_v0_2(1).md), and [earlier Chapter 01 expansion](../references/originals/WYRMFALL_Chapter_01_The_First_Unbound_Expanded.md) are historical. Do not revive the invincible Judge, town-excluded dragon, or ambient-only flying-car interpretation from those versions.

[Architecture v0.1](../references/originals/Voxel_ARPG_Core_Loop_Progression_Architecture_v0_1(1).md) supplies earlier proposals; this pack expands affected details. [Retrieved H01 summary](../references/retrieved/H01_PROJECT_HANDOFF_RETRIEVED_SUMMARY.md) and [retrieved creator-plan summary](../references/retrieved/C01_MUTABLE_PLAN_RETRIEVED_SUMMARY.md) preserve decisions from source texts whose raw file bytes were not mounted in this session.

## Minimal task packets

Terrain: AGENTS → G0/G1 → plugin/asset records → editable-world → relevant save/test sections. Creator: AGENTS → Mutable integration → equipment contract → G2 tests. Dragon: AGENTS → Dragon/Heartfold → save/control → G4 tests. Chapter: AGENTS → Region 01 → relevant bible scenes → slice/tests. Vehicle: AGENTS → cars/colony → save/control → V1/Z1 tests.

Read the packet, not every historical file, for a bounded change. A source search hit is not a reason to import unrelated project constraints.

## Package checks

[Package validation](../PACKAGE_VALIDATION.md) records documentation integrity checks. [Verifier](../tools/verify_package.py) rechecks payload hashes and active links; it does not run gameplay tests.
