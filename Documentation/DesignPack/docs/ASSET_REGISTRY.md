# Asset Registry and Real-Content Readiness

**Documentation release:** v0.1 · September 14, 2026  
**Status:** Detailed design proposal; not an implementation report  
**Basis:** H01 §§5,8–9,16,18–24; C01; B03; see [source register](./SOURCES_AND_EVIDENCE.md).  
**Rule:** Existing requirements remain binding. New numbers and detailed behavior are draft baselines for a bounded prototype, not claims of user approval or runtime validation.

## Purpose and current inspection boundary

This is a populated planning inventory, not a fabricated filesystem audit. H01 reports the packs/categories below. The current session contains planning Markdown, not their model, animation, texture, audio, or plugin bytes. **Every production asset path, skeleton, import result, and cooked result below is NOT INSPECTED.** Populate them in G0, then promote individual capabilities only with evidence.

Use `AssetID`, source pack/product, source archive path, production object path, source file checksum where useful, skeleton/rig, orientation/units, material slots, LODs, collision, animation inventory, sockets, provenance/redistribution notes, tested engine/plugin build, evidence, and blocker. A product title is not a production object path. Do not fill `/Game/...` with invented assets just to remove empty fields.

## Reported source inventory

| Registry ID | Reported source/category | Intended use | First meaningful check |
|---|---|---|---|
| ART-UI-01 | Aurelith Forge Fantasy RPG UI | HUD, inventory, tooltip, menu and creator presentation | Open real widgets/textures/icons; check editable text, scalable panels, controller focus states and licensing. |
| ART-GEAR-01 | Premium Armory; other fantasy weapons/armor | Real visual bases for procedural item instances | Validate at least one melee and one ranged weapon, pivots, sockets, armor compatibility. |
| ART-HUM-01 | Voxel humanoid collection: villagers, knights, wizards, farmers, rangers, crusaders, religious characters | Player parts and local NPC/enemy recipes | Inspect part separation, bones, materials, actual supported proportions; avoid claiming existing modularity. |
| ART-HOR-01 | Horror characters, high-level category reported | Counselor first; later named encounter mappings | Match actual model to each role; idle/move/attack/hit/defeat clips, silhouette and weapon handling. |
| ART-CRE-01 | Horses, wolves, bats, cobras, wasps, slimes, golem, owls and other animals | Wilderness enemies and ambience | Pick two contrasting animated roles; do not infer every animal can attack. |
| ART-ENV-01 | Medieval, dungeon, tavern, church/cemetery, farm structures | Tidecross, quarry services, underworks | Dimensions, collisions, modular seams, readable silhouettes from both cameras. |
| ART-NAT-01 | Trees, grass, mushrooms, beaches, aquatic assets, terrain materials/pieces | Actual procedural terrain materials and foliage | Shader mapping to selected voxel provider, scale, instancing, resource visual variants. |
| ART-DRG-01 | Ten-dragon collection | Regional bosses, companions, controlled dragons, mounts | Inspect each rig independently; detailed ledger below. |
| AUD-HOR-01 | Candle Light Horror Interaction SFX | Authored horror cues and interactions | Listen to actual clips; cue coverage, peaks, loop points, subtitles/visual alternatives. |
| AUD-COOK-01 | Cooking Minigame SFX | Cooking confirmations, station activity | Map clips to actual committed gameplay, not automatic minigame scope. |
| AUD-FISH-01 | Fishing Minigame SFX | Cast/bite/reel/catch feedback | Bite cue must also have visual/controller equivalent. |
| AUD-FOLEY-01 | Adventure Foley | Humanoid actions and material responses | Match to animation timing and surfaces. |
| AUD-STEP-01 | Auto Footstep Utility | Footstep event/presentation candidate | Do not duplicate movement or contact ownership; test terrain material lookup. |
| VFX-PIX-01 | Seven Beat ’em Up Pixel Art VFX packs | Candidate impacts/slashes/magic | Test flipbook/billboard/Niagara presentation in both cameras. Reject pasted-on results, not the real-art rule. |
| AUD-MUS-01 | Soundtrack reportedly exists; intentionally not uploaded due size | Adventure/town/boss/horror music | Actual tracks/loop licenses unavailable in this pass; not proof of missing ownership. |
| ART-FUT-01 | Futuristic environments/cars/colony implied by story, exact production source unassigned | Zenith and V1/Z1 | Locate actual owned suitable assets; story prose is not an ownership claim. |

No pack in this table is newly purchased, downloaded, or certified. For missing animation: owned → owned Fab → suitable Unreal/Manny → Mixamo for humanoid gaps → specific user asset request. Dragon animation gaps do not automatically become solvable by humanoid retargeting.

## Ten-dragon capability ledger

All rows: real object path, rig, clips, rider socket, compact/true collision, import and packaged results **NOT INSPECTED**. Names describe the role, not a claimed filename.

| ID | Source type / character | Required form and locomotion concern | Initial proof priority |
|---|---|---|---|
| DRG-GREEN | Green / Verdance | Ground kit, turning, wing clearance, pet attack reach, real humanoid rider | G4, G4-H, G5 first |
| DRG-WOOD | Wooden / Grovemaw | Bulky protector clearance, compact guard presentation | Later per-dragon proof |
| DRG-LAVA | Lava / Pyraxis | Fire cues, attack muzzle/volume, legible compact VFX | Later per-dragon proof |
| DRG-WHITE | White / Frostmane | Frost cues and supported ice locomotion | Later per-dragon proof |
| DRG-DARK | Dark / Nyxaroth | UE 5.8.2 profile proof passed NYX-01..05: `Hip-Local` + 32 followers, one skeleton, distinct 28x34 / 115x155 envelopes, Heartfold, mount/flight, GAS combat/control, living bond, and save identity. The Cathedral/Michael Mire fixture, `L_GloamingMarches` foundation, Arrival-to-Ashgrave route, supplied-Priest Count Malvaine encounter, distinct supplied-Nun Hollow Twins encounter, optional GAS Sanguine Strike/Second Turn, two-way Schema 7 recovery, the 17-part Michael Mire encounter, and the 17-part weapon-bearing Machete Mason encounter pass | Further required-horror gameplay assemblies before regional completion |
| DRG-ZOMBIE | Zombie / Rotwing | Undead visual preserved after bond; deformed anatomy collision | Later per-dragon proof |
| DRG-SKULL | Skull / Ossuroth | Bone silhouette, heavy-impact reach, rider position | Later per-dragon proof |
| DRG-JADE | Chinese Dragon asset / Jadefang | Long body, turn radius and aerial path; never assume Green skeleton | Later per-dragon proof |
| DRG-MECHA | Mecha / personal name unassigned | Mechanical locomotion, weapon sockets, advanced circuits | Before Zenith dragon production |
| DRG-STEAM | Steampunk / Cogfang | Heavy movement and steam cues, compact readability | Before harbor dragon production |

Inventory animation names from actual files, then map them to semantic actions: Idle, Walk/Run, Turn, Attack.Front, Attack.Area, Hit, DefeatedAlive, Recover, Takeoff, Flight, Land. A semantic name is a required behavior, not proof of a clip. If no suitable living-defeat animation exists, flag a presentation blocker; do not use a corpse and call it surrender.

## Chapter 01 role-to-content map

| Role / object | Source to inspect first | Required content / blocker criterion |
|---|---|---|
| Player | ART-HUM-01 + approved animation sources | Recipe-generated animated humanoid; actual weapon/tool, combat, mine, interaction, swim and mount coverage as relevant. |
| Tamsin | Ranger humanoid parts | Adult ranger silhouette; dialogue/interact, no permanent follower requirement. |
| Mara | Villager/merchant parts | Supplies/storage context, clear dialogue focus; no invented voice recording. |
| Sella, Pell, Iven | Worker/farmer/villager parts | Distinct readable silhouettes/outfits; injured/resting pose only if supported. |
| Rusk and relief forces | Guard/knight parts | Rusk as elite variant; custody state, not kill-and-resurrect presentation. |
| First wilderness threat | ART-CRE-01 | Two useful enemy behaviors from actual animated assets; exact species selected by G0. |
| Warm heart and fossil channel | ART-ENV-01 / real bone/rock assets if supplied | Convincing existing geometry/material assembly; custom required heart/bone artwork reported if absent. |
| Extraction / restraint / claim device | Existing industrial/dungeon assets if suitable | Distinct interaction points and readable active/inactive states; no assumed mesh. |
| Fishing/cooking/camp | Tools, environment, supplied SFX/UI | Rod/catch presentation, working storage, real pieces and usable station. |
| Counselor | ART-HOR-01 + real weapon/animations | Masked silent lake threat with readable attack/advance; cannot pass E1 with a capsule enemy alone. |
| Moonbound beast | Suitable existing creature asset, not yet identified | Playable beast locomotion/attacks/hit/defeat and safe restore; not a glow-only transformation. |
| First car / seats / pet anchor | ART-FUT-01, exact source open | Exterior, supported occupied presentation, collision and real landing geometry; pilotable pawn proof. |

Use ordinary collision/debug primitives only as explicit diagnostic helpers in a technical test, not substitute production artwork or claimed final screenshots. Missing art does not authorize generation in this task.

## G0 capture procedure

Record actual `.uproject`, engine build and content roots. Search the scoped project/approved archive locations first. Extract/read product manifests and licensing metadata without republishing paid packs. Inspect candidate assets in the engine, not just thumbnails. Record animation names, bounds at import scale, material coverage, and missing dependencies. Preserve source imports; normalize project-facing variants without editing unrelated vendor originals.

For each capability, attach a short evidence record: environment, object path, action performed, observation, screenshot/log location, result and unresolved issue. One video can demonstrate multiple behaviors only when the record identifies each actual observation.

## Readiness decisions

G1 needs actual terrain materials, one collision-capable player test body and a real resource visual. G2 needs genuine modular character parts and compatible animations. G3 needs a real enemy/weapon/UI path. G4 needs Green Dragon and rider content. E1 needs Counselor presentation plus real ability cues. V1/Z1 need their actual car and destination assets; no early narrative document can prove these.

An unlocated asset blocks only work that depends on it. Continue independent documentation/data-rule work when authorized, but do not mark the blocked feature complete. Future requests should name the missing role and exact required capabilities, not ask for “more assets” indiscriminately.
