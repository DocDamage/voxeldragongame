# WYRMFALL — Coding Agent Rules

**Baseline:** Original Voxel ARPG Project Handoff (H01), supplemented by the current v0.3 story/gameplay direction. Read [source register](docs/SOURCES_AND_EVIDENCE.md) and [document index](docs/DOCUMENT_INDEX.md). This pack is pre-production, not evidence of an existing implementation.

## Before changing anything

Read the current user task, this file, the [current handoff](docs/production/CURRENT_HANDOFF.md), and only the specifications relevant to that task. Inspect actual project paths, installed versions, existing owners, and working-tree state. Do not infer a repository or branch from another DocDamage project. Preserve unrelated work.

Treat REQUIRED directions as constraints. Treat BASELINE details as proposals for the specified prototype. Do not silently turn a local test restriction into a permanent game restriction. Resolve real requirement conflicts explicitly; record a narrow blocker rather than replacing the game.

## Product constraints

Single-player ARPG; one created humanoid; third-person and top-down cameras share movement, combat, inventory, and progression. Direct movement is default; optional top-down click-to-move remains required. Smooth, persistent editable voxel terrain and finite procedural regions are core, not optional decoration.

Mutable is the authoritative creator. Experimental status is accepted. Do not build a parallel non-Mutable creator. GAS owns attributes, damage, effects, and abilities; do not enable a separate authoritative health system. One owner per inventory, equipment, interaction, terrain, building, water, and save responsibility.

Use real supplied artwork. Procedural terrain/placement is authorized; replacement character art, meshes, textures, UI/icons, audio, animation, or VFX is not. Approved humanoid animation priority is owned animations, owned Fab content, suitable Unreal/Manny animations, Mixamo, then a specific asset request. Validate retargeting rather than assuming it works.

Building is robust but secondary. Fishing, cooking, gathering, and crafting support adventuring. No hunger, thirst, compulsory sleep, spoilage treadmill, survival-bar maintenance, dragon-care chores, or fuel-maintenance treadmill introduced by inference.

Dragons must become real companions and directly controllable creatures. Heartfold means the same dragon at two sizes with meaningful small-form combat. A pet mesh is not the feature. Horror victories grant usable permanent powers, not trophies. Zenith's pilotable cars and playable colony remain future deliverables, not ambient-only substitutes. No feature is reported as working without its actual proof.

## Work economically

Use one agent first and the lowest capable model. H01's routing labels are Luna for reconnaissance/mechanical work, Terra for ordinary contained implementation, Sol for difficult Unreal/GAS/Mutable/voxel/save integration, and Astra for exceptional unresolved problems. These labels are not claims that a given coding environment provides those models. Astra is not the default orchestrator. Normal concurrency is at most two agents and only when it reduces total work.

Give a task a concrete objective, affected paths, constraints, acceptance checks, and stop condition. Reuse existing suitable marketplace functionality. No speculative systems, incidental refactors, generic framework, multiplayer scaffolding, second state owner, or new subsystem per data record. No arbitrary line-count or coverage mandate is imported from unrelated projects.

## Verification and Git

Run the smallest verification that protects the changed behavior. Deterministic regression tests are appropriate for saves, item identity, recipes, rewards, terrain persistence, and progression. Use focused PIE gameplay checks for presentation and control. A cooked build is justified at risky runtime integration and connected-slice gates, not after every edit. No automatic full-engine rebuild, test-farm expansion, or CI setup.

Do not create issues, branches, pull requests, commits, pushes, merges, tags, or workflows automatically. Use existing history and local diffs for inspection. Coherent checkpoints and risky experiments may justify Git actions only within actual user authorization. No Git action is authorized by this documentation task.

## Completion report

State the exact task and files changed; what was run, with result and evidence location; what was not run; blockers and missing assets; and one next bounded task. Distinguish documentation complete, implementation present, PIE verified, and packaged-build verified. A scripted camera shot is not riding; a scale slider is not Heartfold; a debug ability is not the finished Counselor quest.

Stop when the assignment's acceptance criteria pass. Do not add work to make the report look larger.
