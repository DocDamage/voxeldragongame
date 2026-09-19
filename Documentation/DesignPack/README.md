# WYRMFALL — Production Documentation Pack
## v0.1 · Based on the story/gameplay bible v0.3

**Date:** September 14, 2026  
**Stage:** Pre-production  
**Deliverable:** Editable Markdown specifications, an ordered implementation backlog, and acceptance procedures. This is not an Unreal project or a playable build.

## Start here

Read [AGENTS.md](AGENTS.md), then the [document index](docs/DOCUMENT_INDEX.md), the [decisions and scope register](docs/DECISIONS_AND_SCOPE.md), and the [v0.1 release handoff](docs/production/CURRENT_HANDOFF.md). Current runtime status lives in `Documentation/Current/STATUS.md` and `Documentation/Current/HANDOFF.md` in the project workspace. For the first playable game, continue with [Region 01](docs/REGION_01.md) and the [vertical slice](docs/VERTICAL_SLICE.md).

The complete, unchanged [story/gameplay bible v0.3](references/originals/WYRMFALL_Project_Aligned_Story_Bible_v0_3.md) is included. Its Chapter 01 is the current narrative treatment. Earlier source files are archived for provenance, not competing instructions.

## What this pack contains

| Area | Practical result |
|---|---|
| Development rules | A small AGENTS file, source precedence, bounded tasks, and risk-based verification. |
| Asset and plugin readiness | Named source inventory, capability requirements, owner assignments, and explicit unknowns rather than fabricated imports. |
| Playable rules | Combat math, builds, loot, activities, death/recovery, editable-world policies, and control/save contracts. |
| Dragons | Verdance's two proposed form profiles, meaningful compact combat, direct control, mounted flight, town entry, and failure handling. |
| Horror rewards | All 20 named/source-described encounters mapped to their v0.3 Echoes; detailed Relentless Advance and transformation contracts. |
| Vehicles | Real hovercar piloting, vehicle occupancy, pet travel, landing, recovery, ownership, and city/colony transfer. |
| Region 01 | Landmarks, quest facts, dialogues by condition, boss phases, optional cave, Counselor hunt, and alternate-order handling. |
| Presentation | Both camera modes, controller and mouse/keyboard contexts, interface flows, accessibility, and art/audio direction. |
| Production | G0–G5 plus Heartfold/Echo/vehicle/colony gates, dependencies, focused acceptance cases, and a campaign capability map. |

## How to use the specifications

**REQUIRED** preserves a user direction or explicit original handoff rule. **BASELINE** is a newly specified proposal that makes a prototype buildable. **VERIFY** needs real project/asset inspection. **OPEN** means a production or release choice genuinely remains undecided. These labels are not synonyms for implemented.

A working name, cooldown, region distance, component name, folder, or proposed map name is not proof that an Unreal asset exists. Real-content evidence starts with G0. Asset paths and engine/plugin versions cannot be truthfully populated from the story files alone.

There are no artificial completion percentages. Every gameplay acceptance case starts **NOT RUN**. The package checks verify only documentation links, coverage, preserved source bytes, checksums, and archive integrity.

## First implementation assignment

Use the ready-to-paste bounded assignment in [G0/G1 readiness and terrain proof](docs/production/G0_G1_READINESS_AND_TERRAIN_PROOF.md). Begin by locating the actual project and approved assets. Do not start all regions, replace Mutable, create parallel inventories, or enable every owned plugin.

The user's request for this ZIP does not authorize Git writes, asset purchases, new production artwork, or implementation outside a separately authorized task.

## Extraction and integrity

Extract the ZIP with the folder structure intact; relative document links then work in a Markdown viewer. `MANIFEST.json` records files, sizes, and SHA-256 hashes. `tools/verify_package.py` checks the manifest using Python's standard library. It does not install anything, access the network, run Unreal, or modify the game.

All original local Markdown sources are preserved byte-for-byte. Two additional retrieved-source notes summarize the original handoff and Mutable plan; those are clearly identified as summaries, not falsely reconstructed original attachments.

See [package validation](PACKAGE_VALIDATION.md) for the documentation-only checks and [the included verifier](tools/verify_package.py) for a read-only recheck after extraction.
