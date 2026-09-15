# WYRMFALL — active workspace instructions

## Read little; verify honestly

Start with the current user task, [STATUS](Documentation/Current/STATUS.md), and [Codex entrypoint](CODEX_START_HERE.md). Then read only the assigned task packet and relevant source files. Do not ingest the complete 138 KB bible or entire backlog every turn. This is a new source scaffold, not a previously compiled game. The preserved DesignPack has historical documentation-only status text; it does not override the active status.

Source precedence: current explicit user request → required directions in the preserved design pack → v0.3 story treatment → labeled draft baselines. [Design decisions](Documentation/DesignPack/docs/DECISIONS_AND_SCOPE.md) and [source register](Documentation/DesignPack/docs/SOURCES_AND_EVIDENCE.md) retain context. All earlier documents are preserved, not silently rewritten.

## Product constraints

Unreal **5.8.x, preferably 5.8.2**, Windows first, single-player ARPG. Never silently migrate engine. One created humanoid, shared third-person/top-down systems, direct movement plus optional click-to-move. Finite procedural smooth editable terrain must support actual digging/addition, collision, resource depletion, navigation and persistence.

**Mutable remains the creator. GAS remains combat authority.** Experimental/beta status is not a reason to replace Mutable. The plugin descriptor is `Mutable`; the runtime module is `CustomizableObject`. Inspect installed headers/manifests before binding APIs. Do not introduce a second health, inventory, equipment, interaction, terrain, water, building or save owner.

Use supplied real art. Diagnostic engine primitives are allowed in the developer fixture, never a substitute for actual character/dragon/horror/vehicle assets. No replacement art/audio/VFX generation, purchases or paid asset redistribution. Use animation sources in the approved order stated in the design pack.

Keep meaningful pet-size dragon combat, same-identity Heartfold, direct dragon control/riding/flight, permanent usable horror powers, future pilotable flying cars and the playable colony. Do not reduce them to scale sliders, cosmetics or ambient set dressing. No hunger/thirst/sleep meters, mandatory care or maintenance treadmills. Building remains robust but secondary.

## Scope and implementation

The current scaffold contains a diagnostic humanoid/control/GAS host and a fail-closed terrain interface. It has no terrain provider, playable Mutable recipe, game save, economy, dragon, Echo, vehicle or production map. Folder notes are integration reservations, not finished features. Do not create whole empty frameworks for every future feature.

Use `plan --native` to preview and `onboard --native` for the bounded initial command run; `onboard` alone is offline. Check `report` for a recovery action and stale evidence. Begin with BOOT-01 local compile/editor checks, then WP-00 readiness, then ONE eligible WP-01 provider proof. The full shared-control feature WP-03 is still gated; a source shell does not clear it. Respect the original backlog dependencies. Missing one asset blocks only its dependent step.

Use one agent by default and the lowest capable available model/effort. No assumed model aliases or child-agent cascade. H01 routing labels are guidance, not account capabilities. A task gets objective, paths, constraints, tests, stop condition. Do not invent arbitrary file-size/coverage targets or import workflows from another project. No routine full-engine rebuild, automatic cooking or CI.

## Commands and evidence

`python tools/wyrm.py verify` and `python tools/wyrm.py test` are portable offline checks, **not Unreal compilation**. Run native commands on the Windows machine that has the actual UE installation and approved content. `doctor` inspects version/plugin metadata; even its success is not a build. Failed/missing commands, empty or incomplete source-declared native test selections and stale receipts are not passes. Native test discovery is not execution. Onboarding never clears PIE or gameplay gates; its snapshot does not attest engine/vendor binaries.

For modified C++: compile the editor target against actual headers, run the relevant native tests, then use focused PIE for visible behavior. Use the installed plugin/engine source and official primary docs where needed. Do not emulate UE headers and call that an Unreal build. Keep game acceptance cases NOT_RUN until actually exercised. Write current facts under Documentation/Current and logs under Saved; preserved originals remain unchanged.

## Permissions and finish

Inspect existing working-tree state before edits when a repository exists. This workspace is a Git checkout on `main` with origin `https://github.com/DocDamage/voxeldragongame.git`. Use `git -c safe.directory="G:/assets/voxel project" ...` per command for its ownership mismatch; do not change global Git settings. Do not initialize, commit, branch, push, tag, open issues/PRs, buy/install assets or alter global machine/Codex settings without relevant authorization. Preserve unrelated local work; never infer another DocDamage repo is WYRMFALL.

Report changed files, exact commands and results, evidence paths, what was not run, blockers, and one next bounded task. Stop at the task's acceptance boundary. Do not inflate status from source presence to runtime proof.
