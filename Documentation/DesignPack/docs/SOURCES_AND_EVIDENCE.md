# Sources, Provenance, and Evidence Boundaries

**Documentation release:** v0.1 · September 14, 2026  
**Status:** Source register and documentation-only evidence  
**Basis:** H01; C01; B03; B02; C02; A01; W01; R1–R6; see [source register](./SOURCES_AND_EVIDENCE.md).  
**Rule:** Existing requirements remain binding. New numbers and detailed behavior are draft baselines for a bounded prototype, not claims of user approval or runtime validation.

## Source hierarchy

The latest explicit user requirements and H01's approved foundation govern scope. B03 is the current narrative/gameplay treatment. This pack expands B03 and A01 with labeled prototype baselines; it is not a new declaration that all proposed numbers have been approved. An old source is retained for traceability, not allowed to reverse a later direction.

| ID | Source | Local representation | What it establishes |
|---|---|---|---|
| B03 | WYRMFALL Project-Aligned Story Bible v0.3 | [Unchanged source](../references/originals/WYRMFALL_Project_Aligned_Story_Bible_v0_3.md) | Current story, complete Chapter 01, Heartfold, 20 Echoes, pilotable cars, colony, revised gates. |
| H01 | Voxel ARPG — Project Handoff; uploaded as Pasted markdown.md on September 14, 2026, 15:57:55 UTC | [Retrieved summary](../references/retrieved/H01_PROJECT_HANDOFF_RETRIEVED_SUMMARY.md) | Explicit product/development rules, reported asset categories, candidate plugins, proof sequence. File reference: file_00000000544081f598edb3bedb78274d. |
| C01 | Unreal 5.8 Mutable Voxel Character Creator Implementation Plan; uploaded as Pasted markdown.md on September 14, 2026, 15:41:16 UTC | [Retrieved summary](../references/retrieved/C01_MUTABLE_PLAN_RETRIEVED_SUMMARY.md) | Creator recipe, visual pipeline, categories, animation and future expansion targets. File reference: file_00000000c44881f5b2cd35a8ef4de9a9. |
| A01 | Core Loop, Progression & System Architecture v0.1 | [Unchanged source](../references/originals/Voxel_ARPG_Core_Loop_Progression_Architecture_v0_1(1).md) | Earlier proposed builds, item/save contracts, system ownership, G0–G5. |
| B02 | Story Bible v0.2 | [Unchanged source](../references/originals/WYRMFALL_Project_Aligned_Story_Bible_v0_2(1).md) | Historical story revision; superseded where B03 differs. |
| C02 | Chapter 01 — The First Unbound, earlier expansion | [Unchanged source](../references/originals/WYRMFALL_Chapter_01_The_First_Unbound_Expanded.md) | Historical expanded opening; B03 controls current town/homecoming/feature treatment. |
| W01 | Original wyrmfall-world-bible | [Unchanged source](../references/originals/wyrmfall-world-bible(1).md) | Original roster/regions; not authority for superseded invincible Judge or survival-oriented rewards. |

Five source Markdown files were available as actual local bytes and are included unchanged. H01 and C01 were retrieved as searchable text from the user's File Library; their two package files are expressly summaries, not claimed byte-for-byte copies. No model archives, Unreal project files, licensed plugin binaries, or runtime logs were inspected in this documentation pass.

## Technical references consulted

Checked September 14, 2026. These are limited API/documentation observations, not compatibility certification for Unreal 5.8.2 or any supplied asset. Installed headers, plugin manifests, and actual runtime tests remain authoritative for implementation. URLs are provided for future verification; the package works offline without them.

| ID | Primary source and URL | Limited observation supported |
|---|---|---|
| R1 | Epic — Gameplay Ability System. `https://dev.epicgames.com/documentation/en-us/unreal-engine/gameplay-ability-system-for-unreal-engine` | GAS provides abilities, attributes, effects, and related gameplay infrastructure. The combat math in this pack is our proposal, not supplied by Epic. |
| R2 | Epic — UCustomizableObjectInstance. `https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Plugins/CustomizableObject/UCustomizableObjectInstance` | Runtime parameter/update APIs exist. Exact signatures and deprecations must be checked in the installed Mutable build; generated visuals are not the save recipe. |
| R3 | Epic — Saving and Loading Your Game. `https://dev.epicgames.com/documentation/en-us/unreal-engine/saving-and-loading-your-game-in-unreal-engine` | SaveGame supports custom save data and asynchronous storage operations. It does not implement our coherent multi-system transaction automatically. |
| R4 | Epic — Modifying the Navigation Mesh. `https://dev.epicgames.com/documentation/unreal-engine/overview-of-how-to-modify-the-navigation-mesh-in-unreal-engine` | Dynamic Modifiers Only cannot generate new navigation surfaces. Newly dug passages require a real compatible navigation solution, not just a modifier. |
| R5 | Voxel Plugin — Runtime Edits & Sculpting. `https://docs.voxelplugin.com/knowledgebase/blueprints/runtime-edits-and-sculpting` | Runtime edit/save concepts and provider-specific limitations are documented. Terrain-removal volume/yield cannot simply be assumed; validate the actual chosen version. |
| R6 | Epic — Enhanced Input. `https://dev.epicgames.com/documentation/en-us/unreal-engine/enhanced-input-in-unreal-engine` | Input actions and mapping contexts support contextual routing. The proposed keys, controller layout, and priority rules are project design. |

The engine target is carried from H01, not inferred from these websites. No current engine release, supported plugin patch, asset price, license entitlement, or benchmark is asserted on the basis of a generic documentation page.

## Evidence vocabulary

**REPORTED:** A supplied planning source names content or ownership. **LOCATED:** Real bytes/path inspected. **IMPORTED:** Real engine import completed. **RUNTIME VERIFIED:** Required behavior observed with the real content. **COOKED VERIFIED:** Relevant packaged behavior observed. These are separate fields, not one escalating adjective used without logs.

A requirement marked REQUIRED can still be unimplemented. A test marked NOT RUN is not a failure and not a pass. A missing input should be marked BLOCKED when it actually prevents a bounded task. This package's readiness tables use NOT INSPECTED where the source cannot establish absence or presence.

## What was done for this deliverable

Read the local story/architecture sources, retrieve the original handoff/creator directions, author the new specifications and draft rules, preserve source files, and check the documentation package. Package validation details are recorded separately. No game project was built, no content imported, no Unreal gameplay test run, no repository modified, and no art or audio manufactured.
