# H01 — Retrieved original handoff summary

**Source:** `Pasted markdown.md`, title `Voxel ARPG — Project Handoff`, file-library upload September 14, 2026 at 15:57:55 UTC; referenced file ID `file_00000000544081f598edb3bedb78274d`.

**Evidence form:** Source-derived summary of retrieved passages, not a byte-for-byte copy of the original attachment. The original source text was available through file search, not as a mounted local Markdown file. This note intentionally does not manufacture missing sections or report source code inspection.

## Preserved original decisions

Sections 1–7: Single-player Unreal voxel ARPG; engine target 5.8.x, preferably 5.8.2; direct third-person and top-down movement on the same character; optional top-down click-to-move; finite regions; smooth terrain, mining, digging, filling, caves, persistent edits. Procedural generation is allowed for geography and placement using real assets, not replacement artwork.

Section 7 shortlist: Voxel Pro 2; GeoForgeRuntime; `draquel/VoxelWorlds`; UnrealSandboxTerrain; MatterFlux-related technology; structural-stability experiments. Evaluate one small real-content provider proof at a time and stop after a candidate satisfies the requirements. No simultaneous production terrain providers.

Sections 8–9: Mutable is authoritative; Beta/Experimental acceptance is explicit; no duplicate non-Mutable creator. Save appearance recipes. Creator scope includes body/head/face/hair/facial hair/clothing/armor/shoes/gloves/accessories/colors, presets, locks, undo/redo, naming, randomization, and mouse/keyboard/gamepad. Animation priority: owned, owned Fab, appropriate Unreal/Manny, Mixamo, then ask for the specific missing asset.

Sections 10–17: Diablo-style itemization; one inventory/equipment authority; GAS current combat/attribute direction; do not make the Advanced Health/Shield/DBNO asset a parallel owner. Separate constructed buildings from voxel edits. Easy Building System is a candidate. Building is useful but not the main game. Fishing, cooking, gathering and harvesting support adventure. No survival maintenance. Ten themed dragons become region bosses, allies, directly controlled pawns, and supported mounts. Initially assume one active dragon. Green Dragon/Verdant Wilds is the starting recommendation.

Sections 18–24: High-level supplied asset inventory includes humanoids, horror characters, animals, armory, medieval/dungeon/tavern/church/cemetery/farm content, foliage, beach/water/terrain content, UI/icons, VFX, SFX/foley/footsteps and dragons. Named packs: Aurelith Forge Fantasy RPG UI; Premium Armory; Candle Light Horror Interaction SFX; Cooking Minigame SFX; Fishing Minigame SFX; Adventure Foley; Auto Footstep Utility; seven Beat 'em Up Pixel Art VFX packs; dragon collection. Soundtrack exists but was not uploaded because of size. Waterline Pro 6 is the likely water owner; Ultimate Water Shader only supplements it for a demonstrated visual need. Other supplied plugins require overlap evaluation.

Reference repositories named in section 24: `DocDamage/epic-voxel-structural-stability`, `DocDamage/MatterFlux`, `draquel/VoxelCharacterPlugin`, `DocDamage/Boids_Project`, `DocDamage/UnrealSandboxTerrain`. These are potential sources, not approved dependencies and not the game's repository.

Sections 25–29: One-person development; lowest suitable model; Luna reconnaissance/mechanical, Terra everyday implementation, Sol integration/difficult bugs, Astra exceptional unresolved problems; Astra not default orchestrator. Single-agent first, generally no more than two concurrent agents. No speculative scope, unnecessary abstraction, replacement art, unnecessary tests/builds/cooks/CI, or automatic Git bureaucracy. Do not push/merge without authorization. Smallest useful verification; profile before optimizing; reuse marketplace functionality; stop at acceptance.

Sections 30–34: Pre-production, no completed production implementation claimed. Requested core docs include AGENTS, GAME_DESIGN, TECHNICAL_ARCHITECTURE, ASSET_REGISTRY, PLUGIN_MATRIX, REGION_01, VERTICAL_SLICE, plus UX and art-direction work. First slice runs from creator through real procedural terrain, both cameras, combat/loot/equipment, mining, fishing/cooking/camp/cave, Green Dragon victory/bond, companion/control/riding/flight, then quit/reload with preserved state. Do not build the other nine regions before this slice is stable.

## Scope of this summary

Product requirements are carried into the active documents. Compatibility, filenames inside an Unreal project, skeleton support, asset import, runtime behavior, performance, a project repository, and a specific installed patch have not been verified by retrieving this handoff.
