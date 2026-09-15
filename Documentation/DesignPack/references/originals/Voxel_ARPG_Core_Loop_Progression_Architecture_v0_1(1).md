# Voxel ARPG — Core Loop, Progression & System Architecture

**Planning Pass 1 · Proposal v0.1 · September 14, 2026**

**Project stage:** Pre-production.  
**Source of approved direction:** `Pasted markdown.md`, titled *Voxel ARPG — Project Handoff* (H01).  
**Engine target carried forward:** Unreal Engine 5.8.x, preferably 5.8.2.  
**Purpose:** Turn the handoff’s next planning step into a concrete proposal that can lead into Region 01 and the Green Dragon vertical-slice specification.

**Authority rule:** The uploaded handoff remains the source of approved product decisions. New rules, quantities, implementation choices, and scope boundaries in this document are **proposals**, not additional user approvals. Technical documentation establishes available APIs or documented limitations; it does not prove that the user’s assets or plugins work together.

**Work actually performed for this document:** Read the supplied handoff and consult the primary technical references listed at the end. No Unreal project, plugin binary, asset archive, skeleton, animation, packaged build, or running gameplay was inspected or validated in this pass. No repository changes were made.

---

## 1. What carries forward unchanged

The game remains a **single-player action RPG**, not a survival game. It uses finite procedural regions, smooth editable voxel terrain, a shared humanoid gameplay implementation, switchable third-person and top-down cameras, direct movement, and optional top-down click-to-move. Mining, excavation, terrain addition, and persistent terrain edits remain requirements. [H01 §§1–7]

**Mutable remains the authoritative character-customization backbone.** Its experimental/beta status is accepted. This plan does not introduce a parallel non-Mutable creator. Appearance is stored as a recipe; real approved source assets provide the visual parts. Weapons and tools remain separate attachments where appropriate. [H01 §§8–11]

The handoff establishes Diablo-style itemization and identifies GAS as the current architectural direction for combat attributes and abilities. One system must own inventory, equipment, interactions, and each other gameplay responsibility. GAS is not a reason to add multiplayer architecture. [H01 §§10–12, 21, 34]

Building is robust but secondary. Constructed buildings are distinct from voxel terrain edits. Fishing, cooking, gathering, and crafting support adventuring; hunger, thirst, mandatory sleep, and survival-meter maintenance stay excluded. [H01 §§12–15]

Dragons remain regional bosses that can become companions and directly controllable creatures. Riding and flight remain goals for suitable supplied rigs/assets, subject to actual feasibility. The Green Dragon and Verdant Wilds are the handoff’s recommended starting point, not a finalized regional design. Ten supplied dragons do not automatically mean ten launch regions. [H01 §§16–17, 31]

Real supplied art is required. Procedural terrain and placement are authorized; manufacturing replacement production artwork is not. Missing assets must be reported. Model routing, focused verification, single-agent-first work, and no unauthorized Git writes remain unchanged. [H01 §§5, 18–29, 34]

---

## 2. Proposed game pillars

These are concise design tests derived from the approved direction, not new feature commitments.

### Pillar A — Action first, from either camera

Movement, attacks, dodges, targeting, and feedback should feel responsive before content volume increases. Switching cameras changes presentation and target acquisition, not character identity, damage rules, inventory, or progression.

**Design test:** The same encounter is understandable and playable in both camera modes without maintaining two combat implementations.

### Pillar B — The terrain is part of the adventure

Digging should expose resources, open routes, and change how the player approaches exploration. Terrain placement should support access and camps. Editable terrain must interact with collision, resource depletion, navigation, and persistence rather than exist as a disconnected sculpting demonstration.

**Design test:** Dig a useful passage, cross it, collect its resource, leave, and reload; the route remains and the resource does not duplicate.

### Pillar C — Loot changes decisions, not just numbers

An item should offer understandable tradeoffs through base properties, affixes, and eventually build-defining effects. Randomization applies to gameplay data backed by real authored visuals. The player should not need a separate model for every roll.

**Design test:** A player can explain why they equipped a drop and see the correct appearance and gameplay effect.

### Pillar D — Defeating a dragon expands what the player can do

A dragon is more than a trophy. Its boss encounter introduces recognizable capabilities that later matter in companion and controlled play. The player earns a new combat and traversal option, while humanoid combat and dungeon exploration remain useful.

**Design test:** After bonding, the dragon participates in a real fight and can be directly controlled; a menu unlock alone is insufficient.

### Pillar E — Preparation is useful, never compulsory maintenance

Camps, food, fishing, and crafting provide convenience, specialization, and rewards. They should not repeatedly interrupt the adventure with mandatory chores.

**Design test:** A combat-focused player can progress without routinely fishing, cooking, or rebuilding a camp, while someone who enjoys those activities gains worthwhile benefits.

---

## 3. Proposed core gameplay loop

**Choose an objective → explore → fight or overcome terrain → obtain loot/resources → improve a build or prepare → pursue a harder objective.**

At the regional scale: **establish a foothold → learn the region → complete its major challenge → defeat/bond its dragon → use the dragon in subsequent adventures.** The narrative meaning and exact order of those milestones still need Region 01 design.

### An illustrative 30-minute session

This is a player-experience example, not a mandatory checklist or a development-time estimate.

| Session segment | Example player activity | Payoff |
|---|---|---|
| Minutes 0–3 | Choose a cave, quest, resource opportunity, or unexplored landmark; adjust equipment if needed. | A clear objective without compulsory preparation. |
| Minutes 3–12 | Travel through the region, fight small groups, discover a side route, or mine a shortcut. | Exploration progress and immediate rewards. |
| Minutes 12–21 | Complete a cave encounter, fight an elite, or reach a contested resource deposit. | A meaningful challenge and a reason to assess the build. |
| Minutes 21–26 | Compare loot, finish the objective, and decide whether to continue or return. | An upgrade, useful resources, or persistent progress. |
| Minutes 26–30 | Optionally craft, cook, fish, improve a camp, or set the next objective. | A satisfying stopping point with a reliable save. |

Not every session includes every secondary activity. A boss attempt can replace the exploration segment. After the first dragon unlock, dragon travel or companion combat can change the route without forcing every session into dragon gameplay.

### Progress should be visible on three timescales

A normal encounter provides immediate feedback and a useful reward opportunity. A session advances an objective, build, recipe collection, or regional milestone. A regional climax unlocks a substantial new capability through its dragon.

Do not guarantee an equipment upgrade every fight. Do ensure that important encounters have an understandable reward purpose.

---

## 4. Proposed player progression

The handoff leaves classes, attributes, skill slots, respecs, and level caps open. Everything in this section is a design proposal. [H01 §31C]

### 4.1 Classless builds with starting archetypes

**Recommendation:** Use starting archetypes rather than permanent classes. Offer a melee-oriented, ranged-oriented, and magic-oriented starting loadout once their real assets and animations are available. Names remain provisional.

A starting archetype chooses initial equipment and abilities, not a permanent restriction. Later builds combine compatible weapons, unlocked abilities, passive choices, and item effects. A player who starts with a sword can later specialize in a bow or magic without remaking their appearance or restarting the character.

Weapon-specific abilities still require the appropriate weapon. Classless does not mean a sword animation can fire a bow ability or that every combination needs bespoke support.

**Slice boundary:** Prove one complete loadout first, then a second materially different loadout before calling build variation demonstrated. The third is a content target, not permission to substitute unapproved art or animations.

### 4.2 Keep appearance independent from combat power

Character-recipe choices should not silently assign combat bonuses. Body proportions, hair, face, and colors belong to appearance. Any later race traits or body-size gameplay differences require explicit design approval and collision/animation validation.

Equipment appearance is derived from equipped items and the character recipe. Do not let creator clothing selection grant inventory items, item affixes, or abilities for free.

### 4.3 Separate progression tracks without creating separate mandatory grinds

| Track | Proposed purpose | Boundary |
|---|---|---|
| Character level and experience | Establish broad progression and unlock access to choices. | Full-game level cap remains open. |
| Abilities and passives | Change combat options and specialization. | Use a small initial catalog; avoid a huge empty skill tree. |
| Items | Provide numerical improvement and later build-changing effects. | One item-instance model, independent of visual-mesh count. |
| Regional milestones | Record discoveries, quest steps, major encounters, and dragon access. | Do not equate every milestone with a new currency. |
| Dragon progression | Record unlock, abilities, and later bond/level advancement. | No compulsory feeding or survival upkeep. |
| Activity recipes | Reward discovery and optional crafting/fishing/cooking. | No required profession grind for the main story in the slice. |

For an initial balancing environment, **levels 1–10** are a useful proposed test range. This is not a launch level cap, an estimate of campaign length, or a promise about Region 01’s final level range.

### 4.4 Initial combat attributes

Propose health, an ability resource, attack power, armor, elemental resistances, and critical-hit properties as the first useful set. Add a separate dodge/block resource only when combat testing demonstrates a need; rejecting survival meters does not prohibit a combat resource.

Do not invent six primary attributes before specifying how they create distinct builds. The choice between attributes such as strength/dexterity/intelligence and a smaller derived-stat model remains open for the combat specification.

GAS provides the framework for abilities, attributes, and Gameplay Effects; the project must still define the actual calculations, balance, damage types, and stacking rules. [R1]

### 4.5 Ability controls

A proposed controller-conscious target is a basic attack, one weapon-specific secondary action, up to four equipped active abilities, dodge/defense, and a quick-use consumable. Dragon commands belong in their own context rather than permanently consuming every humanoid ability slot.

Do not ship empty skill slots merely to match this proposed count. Start with the smallest complete combat kit, then validate keyboard/mouse and gamepad bindings before expanding it. Heavy attacks, blocking, parrying, and ultimates are not all implicitly approved by listing a secondary action.

### 4.6 Respec and difficulty

Propose free respecs during development and an out-of-combat respec at a safe location for the slice. That encourages trying item and ability combinations. Final costs and restrictions remain open.

Propose regional difficulty bands rather than automatically scaling every enemy to the player at all times. Exact bands, elite modifiers, accessibility assists, death penalties, and boss retry rules belong in the combat/region design. Do not introduce gear loss, XP loss, or hardcore deletion by default.

---

## 5. Proposed loot and inventory rules

### 5.1 Separate item definitions from item instances

An **item definition** describes the authored base item: stable identifier, display name, icon, mesh or Mutable appearance mapping, slot, weapon family, and allowed affix pools. An **item instance** stores that specific drop’s identifier, item level, rarity, rolled values, and any later upgrade state.

Equipping, saving, or loading an instance must not reroll it. Store the actual rolled results; a random seed can be diagnostic metadata, not the sole record of the player’s item after balance tables change.

Propose Common, Magic, and Rare for the first loot proof. A later slice encounter can add one real, authored special effect if supported by the asset and ability catalog. The complete rarity hierarchy remains unapproved; do not erase the handoff’s later Legendary/Unique possibilities.

### 5.2 Inventory presentation

**Recommendation:** A fixed-slot grid with stackable materials, sorting, filtering, and a stash; no weight meter in the slice. Reserve quest-critical records so a full ordinary bag cannot make a quest impossible. Exact slot counts and stack limits depend on the supplied UI and inventory system.

One inventory model backs all interfaces. An equipment screen, stash screen, and crafting screen are views of authoritative item ownership, not independent collections of copied items.

Propose familiar equipment categories—main-hand/off-hand, head, torso, legs, gloves, feet, and limited accessories—but finalize exact slots only after checking which supplied modular items can be worn and represented correctly. A two-handed weapon must reserve both weapon hands.

### 5.3 Reward transaction

A pickup or crafting reward succeeds only when the selected inventory owner accepts it. Otherwise the source stays available or follows a clearly defined overflow policy. It must not disappear while a UI says the inventory is full.

Identify one-time rewards and harvested resources persistently. Loading a save or receiving the same completion callback twice must not award the same reward twice.

---

## 6. Proposed system architecture and ownership

This table separates product direction from integration decisions. It does not claim that any third-party product has passed an import or runtime test.

| Responsibility | Owner/direction | Status and boundary |
|---|---|---|
| Character customization | Mutable / Customizable Objects | Approved. Recipe maps to supplied visual parts; no duplicate creator. |
| Combat attributes and abilities | GAS | Handoff architectural direction. Separate combatants can have their own Ability System Components within the same system. |
| Player input and possession | One player-controller flow with mode-aware input routing | Proposed. Control changes do not transfer item ownership. |
| Humanoid camera modes | One camera policy for the same humanoid | Shared character approved; exact implementation proposed. |
| Inventory | One selected owned inventory implementation | Selection open. Must support stable item instances and persistence. |
| Equipment | One equipment authority associated with the inventory | Exact implementation open. Owns slot validation and GAS grants. |
| Loot generation | A small project rule layer using selected inventory item instances | Proposed; reuse existing plugin functionality where suitable. |
| Terrain generation, edits, and collision | One selected voxel provider | Open pending the provider proof. |
| Resource depletion and rewards | One project resource-state authority | Proposed. Uses terrain outcomes; does not become a second terrain engine. |
| Interaction | One selected interaction implementation | Open. Same interaction intent can invoke gathering, crafting, doors, or mounting. |
| Constructed buildings | Easy Building System candidate | Not yet locked by this document. Separate from terrain edits. |
| Water state and water presentation | Waterline Pro 6 candidate | Handoff preference, not a verified selection. Swimming/fishing need explicit integration. |
| Enemy and companion decisions | One modest AI approach for the initial actors | Final choice open. Do not start with Mass merely because it exists. |
| Dragon gameplay | One shared dragon framework with data-driven variations | Approved concept; locomotion and rig compatibility need proof. |
| Persistence | One save coordinator using engine saving plus provider payloads | Proposed. One coherent snapshot, not unrelated subsystem saves. |
| UI | Supplied fantasy UI assets over authoritative gameplay data | Approved visual direction; interaction/navigation implementation open. |
| Audio and VFX | Supplied approved assets, selected utility integrations | Do not add alternate gameplay state owners through audio/VFX plugins. |

### 6.1 Minimal runtime organization

Use a small persistent run-level owner for session identity, current-region identity, and references to the authoritative inventory/progression owners. Keep the save coordinator available across regional transitions.

The original humanoid retains its identity while a dragon is controlled. The dragon has its own identity and combat state. Enemy actors also own their own combat state. The player controller directs the currently controlled creature; it does not become the owner of every creature’s health.

Do not build duplicate mutable inventory collections in the run owner, humanoid, dragon, and UI. If the selected inventory plugin expects a component on the humanoid, retain that humanoid and route access to it during possession; do not rewrite the plugin simply to satisfy a diagram. Its exact persistence across region travel must then be validated.

A provider-specific call can sit behind a thin project-facing function where necessary. No universal plugin framework, generic event bus, multiplayer scaffolding, or automatic one-subsystem-per-feature policy is required.

### 6.2 Stable data identities

Use stable identifiers for character recipes, item definitions, item instances, placed buildings, resource deposits, region instances, and dragon unlocks. Definition data describes content; runtime state records what happened to a particular instance. Save payloads carry schema/content versions so mismatches can be detected rather than silently interpreted as a fresh world.

These are logical records. They need not each become a separate plugin, service, subsystem, or database.

---

## 7. Critical integration contracts

### 7.1 Character recipe → Mutable → animated character

The recipe supplies validated appearance choices. Mutable generates the visual result from the approved source assets. Animation and attached equipment must remain correct after an appearance update. [H01 §§8–9]

Epic documents runtime generation of skeletal meshes/materials/textures and asynchronous skeletal-mesh update APIs. Those capabilities support the chosen approach but do not certify a particular modular asset collection. [R2, R3]

**Proposed behavior:** Keep the last valid appearance visible while a requested update is pending. Coalesce repeated preview changes and track request revisions so an older result cannot replace the latest intended appearance. Validate result status using the installed version’s API. Save the validated recipe, not the transient generated mesh.

**Proof:** Change a real visible part, recolor a supported material, animate the result, attach a weapon, save the recipe, restart, and reconstruct the same character. Test in a cooked build at the integration gate, not only in an editor preview.

### 7.2 Equip item → stats/abilities → appearance

The equipment authority first checks item ownership, slot compatibility, restrictions, and the availability of required content. On success it replaces the previous slot assignment, removes that slot’s previous effects/grants, applies the new effects/grants, and requests the corresponding appearance update.

Track the actual grants/effects by equipped item or slot. Do not recompute them by repeatedly adding bonuses to a current total. Re-equipping or loading must not accumulate permanent bonus copies.

For the slice, propose a visible pending state and no final equipment commit until the required visual result is ready. On failure, retain the prior valid equipped state and report the problem. Do not silently show one armor item while granting another item’s bonuses.

Attached weapons/tools stay distinct from generated body/clothing geometry. A later full creator may expose cosmetic overrides, but this pass does not approve a transmog system.

### 7.3 Mining → deformation → depletion → reward

Validate reach, tool/ability, protected-area rules, and the target resource before accepting an edit. Commit resource consumption/reward only when the edit has succeeded under the selected provider’s completion semantics. Treat a request as pending until collision and other required consequences are ready.

Keep resource yield independent from a mere button press. Voxel Plugin’s current 2.0p8 documentation specifically notes that extracting the amount of terrain removed by a sculpt operation is not straightforward; documented sculpting support therefore does not automatically supply a mining economy. This is evidence about that provider, not an assertion about every candidate. [R4]

**Proposed initial rule:** Give each authored/procedurally placed deposit a stable identity and a finite remaining yield. Track depletion once. General terrain digging changes geometry but does not generate unlimited ore. Filling and re-mining the same volume must not regenerate the original deposit. Any yield from player-placed material must preserve an explicit material budget.

Save the edit state and deposit depletion in the same coherent save generation. A geometry-only save is insufficient.

### 7.4 Terrain/buildings → navigation → click-to-move and AI

A new tunnel is a new traversable surface only after relevant collision and navigation updates. A filled tunnel or placed wall can invalidate an existing route. This must affect enemies and optional player click-to-move consistently.

Epic distinguishes Dynamic navigation generation from Dynamic Modifiers Only: the latter changes existing navigation but does not generate new surfaces. Consequently, using only modifiers is not enough to prove navigation through newly excavated routes. [R5]

**Proposed behavior:** Invalidate affected paths, rebuild relevant local navigation where the chosen provider supports it, and report a blocked/pending route rather than moving through stale geometry. Manual movement cancels click-to-move immediately. Never teleport through a blocked path as an implementation shortcut.

Top-down targeting must distinguish stacked cave floors using a reachable, appropriate surface; a hit on the wrong vertical layer is not an acceptable destination. Dragon flight uses its own aerial movement/obstacle rules, not an assumption that a ground navigation mesh supports flight.

### 7.5 Activities/building → inventory → save

Recipes and construction request a single inventory transaction. Inputs and outputs must succeed together, or leave the prior state intact. Previewing placement does not consume materials. Failed placement does not leave a hidden actor or charge the player.

Placed buildings have persistent IDs, selected asset/definition IDs, transforms, and any state genuinely implemented, such as contents or damage. Storage refers to the same item-instance model as the player bag.

### 7.6 Gameplay → UI/audio/VFX

UI displays committed state and explicit pending/error states. It requests actions; it does not directly award items, change health, complete quests, or unlock dragons.

Audio and VFX respond to confirmed gameplay events and approved anticipation cues. A hit effect does not independently apply damage. A reward animation does not determine whether the reward was saved.

---

## 8. Procedural regions and persistence

### 8.1 Proposed region construction

Use a finite region definition with a saved seed, generator version, parameters, approved asset tables, and explicit placements for critical landmarks. Generate terrain/caves first, then place content against usable geometry.

Reserve critical arrival points, necessary quest locations, and the dragon arena before distributing ordinary resources and encounters. This is a proposed generation rule, not a finalized map. Whether particular landmarks are protected from terrain edits remains a product decision.

For persistent procedural objects, do not use a temporary foliage-array index as identity. Record a stable identity or resolved placement so harvesting and removal survive streaming and reload.

A seed is not a complete save. Changing the generator, assets, parameters, or provider may change the result. Pin the generation contract for an existing save and store the state required by the chosen provider. Do not promise cross-version regeneration without a real compatibility test.

### 8.2 World persistence proposal

Player terrain edits and placed buildings persist until intentionally changed. Critical quest/boss state also persists. Ordinary enemy respawns can use a separate encounter rule, but entering a region must not reset player construction or mined terrain.

Resource respawn policy remains open. The initial persistence proof should use non-respawning test deposits so it can clearly detect accidental duplication. That is a test rule, not a final economy decision.

Water requires a separate design choice where terrain can be removed underneath a river or lake. Do not imply that selecting a water shader automatically creates excavation-aware fluid flow. For the initial region, propose explicitly bounded water volumes and documented underwater/excavation behavior; any no-dig zone or non-flooding cave rule requires approval before becoming a permanent restriction.

### 8.3 One coherent save snapshot

The save coordinator gathers character recipe, progression, current health/resources, inventory, equipment references, ability choices, supported persistent effects and remaining cooldowns, current region, terrain payload, resource depletion, structures/storage, quests, boss state, dragon records, and any implemented activity progress.

Unreal’s SaveGame system supplies serialization/storage mechanisms and asynchronous save/load entry points. It does not automatically discover and preserve this project’s gameplay state. [R6]

**Proposed slice approach:** Prefer one coherent snapshot containing the provider payload when its API and size permit. If the provider requires separate files, associate all pieces with one save-generation ID and only mark that generation complete when every required piece succeeds. Retain the last complete save on failure. This is project behavior to implement, not an atomicity guarantee provided by a save function.

Capture stable data before writing. Do not read changing actor/component state from arbitrary worker threads. Do not capture an item after it was granted but a deposit before it was depleted. Initially, delay a save until the current equipment/mining/build transaction reaches a stable boundary rather than inventing a general transaction platform.

For the slice, propose preserving remaining gameplay-time buff/cooldown durations without advancing them while the game is closed. Do not serialize arbitrary running ability tasks; reach a defined stable boundary instead. Reloading must not act as a free heal or cooldown reset. Final offline-time behavior remains a product decision.

An asynchronous disk write does not prove that payload gathering, terrain serialization, or completion processing is hitch-free. Measure those stages independently when a runnable project is available.

### 8.4 Proposed load order

1. Validate save integrity, required content/provider versions, and region identity.
2. Restore the compatible region baseline and its required terrain payload.
3. Restore depletion, removed/placed objects, buildings, and storage.
4. Make required collision/water/navigation state ready for the arrival area.
5. Restore the humanoid recipe and logical inventory/progression/equipment state; rebuild gameplay grants once.
6. Restore relevant dragon state, possession relationship, and rider attachment.
7. Rebind UI/input, check valid placement, and enable play.

The exact ordering can be adjusted for provider requirements, but the invariant is fixed: the player must not briefly spawn into unedited terrain, receive duplicate equipment bonuses, or lose a dragon because its controller was not reconstructed.

For missing content or incompatible terrain payloads, preserve the save and give a specific error. Do not silently overwrite it with a new character/world.

---

## 9. Dragon control architecture

### 9.1 One framework, separate state dimensions

Boss, companion, direct-control, and mounted behavior are roles of the dragon framework, not four unrelated implementations. However, they should not be forced into one flat enum: a dragon can be bonded, player-controlled, airborne, and carrying the rider at the same time.

Propose separate records for relationship/unlock state, current controller, rider attachment, locomotion state, and combat state. These are a few explicit fields, not a general-purpose state-framework project.

Use per-dragon data for mesh/skeleton, animation mappings, scale/collision, rider socket, abilities, region identity, and locomotion tuning. Reuse where compatible, but do not assume ten supplied dragons share a skeleton or that a long-bodied dragon can use the Green Dragon’s animations unchanged.

### 9.2 Combat ownership during possession

Keep the humanoid’s GAS state with the humanoid and the dragon’s GAS state with the dragon. The controller’s active ability-input target changes with possession. Merely changing control must not heal the dragon, reset cooldowns, copy humanoid equipment bonuses, or delete the humanoid’s inventory.

Epic’s Ability System Component distinguishes logical owner and physical avatar. That supports explicit ownership design, but it does not select the correct project-specific possession policy automatically. [R7]

**Proposed transition:** Validate eligibility; safely end/cancel applicable actions; stop AI movement; transfer control; change input/camera context; rebind ability input/UI; preserve both creatures’ identities. Returning reverses the process and resumes the appropriate companion policy.

### 9.3 What happens to the unmounted humanoid?

This is unresolved in the handoff and must not be ignored.

**Proposal for the first direct-control proof:** The original humanoid remains where control was transferred, stays visible, and retains normal collision/damage behavior. Do not grant automatic invulnerability. Block starting remote control during an unsafe transition; define a clear return/death response if the waiting humanoid takes damage or is defeated. This is a provisional design, not a final remote-control mechanic.

Autonomous humanoid following or fighting while the player controls a dragon would be an additional companion-AI requirement. Do not silently add it to the first slice.

### 9.4 Riding and flight

Validate rider position against real dragon and humanoid rigs. Mounting must attach the original humanoid rather than spawn an unrelated cosmetic duplicate. Mounted control directs the dragon, with explicit rider collision and damage rules.

Takeoff requires clearance. Landing requires a valid supported destination. Dismount requires a safe reachable location and must reject attempts that would place the rider inside terrain or over an unsupported drop. Dragon defeat while mounted needs a deliberate rider recovery/failure rule.

Retain both camera options; tune a usable dragon/flight presentation for each rather than silently deleting top-down control. Camera tuning may differ by controlled creature while the humanoid remains one implementation.

### 9.5 Unlock and recovery

Defeat/bonding must create one persistent unlock record and one set of one-time rewards. Boss abilities and companion abilities may share definitions but need distinct balance data where appropriate.

The initial assumption remains one active dragon. Switching dragons must retain their respective identities, progression, and recovery state. Propose non-permanent companion defeat for the slice, with a clearly communicated recovery rule; permanent loss is not approved.

The completed slice should save and restore companion, directly controlled, mounted-grounded, and mounted-flight states. Loading after terrain has made a saved position unsafe requires a documented recovery location while preserving progression and creature identity, not a fresh dragon spawn with reset state.

---

## 10. Optional activities and camps: useful minimums

**Fishing proposal:** Use an accessible cast/bite/reel interaction backed by a real rod, water location, appropriate animations, and supplied sounds. The first successful catch enters the same inventory and is usable in a real recipe. Final minigame complexity, regional species, and rod progression remain open.

**Cooking proposal:** Produce useful healing or temporary bonuses, not hunger relief. Begin with one clearly defined buff category and a replacement/refresh rule rather than stacking unlimited food bonuses. Exact durations and recipe counts are tuning decisions.

**Crafting proposal:** Start with a small set of useful recipes and one clear station interaction. Consume inputs and add results through the inventory transaction. Station content can include equipment, tools, or consumables according to verified assets; not all crafting professions are required at once.

**Camp proposal:** Prove a functional small camp with placement preview, snapping where applicable, a floor/foundation, enclosure/entry pieces, storage, and a usable crafting/cooking point from supplied assets. Removal, persistence, and invalid-placement feedback matter more than a large decorative catalog.

Digging under a constructed building must have an explicit result. Either use the selected building system’s suitable support behavior or define an approved restriction. Do not leave structures accidentally floating and do not add a second structural simulation without a demonstrated need. Full cave-collapse simulation is a separate unapproved feature.

---

## 11. Technical proof sequence before regional production

The handoff’s full create-character-to-dragon-to-reload chain stays the destination. The following earlier gates reduce the risk of building a region around an unproven dependency. They are sequencing proposals, not removal of approved features.

| Gate | Small, real-content demonstration | Required evidence |
|---|---|---|
| G0 — Content and version readiness | Locate the real engine/project, relevant plugins, one humanoid kit, terrain materials, one enemy/weapon set, UI, and Green Dragon assets. | Exact paths, versions, import status, and a short missing-content list. No assumed asset compatibility. |
| G1 — Editable-world proof | One provider creates smooth terrain and a cave; the player digs/adds terrain; collision, a resource deposit, navigation, and save/reload respond correctly. | Actual gameplay plus persistence results and measured behavior for a representative edit sequence. |
| G2 — Mutable-character proof | One supplied customizable humanoid animates, changes a real part, wears/equips a real item, and reconstructs from its recipe. | PIE result plus a focused cooked-build check for runtime customization. |
| G3 — Playable ARPG core | Both cameras, direct movement, one enemy, a usable attack/defense kit, one randomized drop, equipment, mining, and a coherent save form a loop. | Playable end-to-end behavior using supplied content, without duplication or state resets. |
| G4 — Dragon mechanics proof | Green Dragon moves, attacks, runs as AI, accepts direct control, and demonstrates mounting/flight where the rig permits. | Actual transfer/return, rider placement, collision, landing, and saved-state results. Not a boss-balance claim. |
| G5 — Connected vertical slice | Region 01 connects exploration, loot, cave content, useful activities/camp, a designed dragon boss, bonding, companion combat, control/riding, and save/reload. | A real playable sequence and a recorded pass/fail list for the slice acceptance criteria. |

Run gates sequentially by default. G1 and G2 can be investigated independently, but do not create separate production stacks that must later be merged. The dragon mechanics proof should occur before polishing a large boss encounter or producing nine additional regions.

One focused cooked/package check is justified for a risky runtime plugin integration and at the complete slice gate. That does not authorize cooking after every feature or rebuilding the engine after every edit.

### What makes a provider pass?

It must prove the handoff’s terrain requirements using actual project materials and collision. The proof must include repeated edits, addition/filling, cave/tunnel access, persistence, relevant navigation, and representative region/streaming behavior—not just a small editor sculpt demo. Measure edit latency, collision readiness, frame behavior, save size/time, and memory on the declared test hardware before setting production budgets.

The shortlist remains the handoff’s shortlist. No provider is selected by this document. Stop comparing alternatives once one satisfies the agreed requirements; do not maintain multiple integrations.

---

## 12. Proposed Region 01 slice envelope

**Working region:** Verdant Wilds. **Working boss:** Green Dragon. Both remain recommendations carried forward from H01.

Propose one compact but complete adventure area with a recognizable arrival/foothold, wilderness route, water/fishing location, editable resource area, cave/dungeon encounter, camp opportunity, and a dragon arena. These can be adjacent parts of one region rather than separate expensive content zones.

Start with a small set of real enemy roles: a melee threat and one contrasting ranged or creature threat; add an elite only once those work. Use the approved inventory of real assets to select them. Do not assume that a listed creature has attack, hit, and death animations until inspected.

The region design must answer why the player explores, what prepares or grants access to the dragon encounter, why defeat leads to bonding, and what useful activity becomes possible after unlocking the dragon. Those story and encounter choices are deliberately not invented as canon here.

### Completed-slice acceptance

A player creates and saves an appearance, enters the real procedural region, switches cameras, fights a real animated enemy, obtains a randomized item, equips it correctly, mines terrain/resources, uses fishing/cooking, constructs a useful small camp, and completes cave/dungeon content. The player then defeats/bonds the Green Dragon, uses it as a companion, directly controls it, and demonstrates supported riding/flight. [H01 §33]

After quitting and reopening the game, appearance, item rolls, equipment, progression, terrain edits, depleted resources, camp/storage, quests, boss defeat, dragon unlock/state, and implemented activity progress remain consistent. Reloading must not reroll gear, duplicate rewards, refill the mined deposit, or create a second dragon.

Optional click-to-move must have a demonstrated route/cancel/blocked-path behavior before that option is presented as working. No empty menu entry counts as implementation.

Full creator breadth, all item tiers, every activity upgrade, every enemy family, and all ten dragons are not required to prove the shared foundation. They remain later content/design work rather than silently canceled features.

---

## 13. Decisions still open after this pass

| Decision | Proposed direction here | What remains unresolved |
|---|---|---|
| Player builds | Classless with starting archetypes | Approval; detailed ability/passive catalog and attribute math. |
| Initial progression | Small 1–10 test range | Final level cap, regional bands, respec economy, death penalties. |
| Inventory | Fixed-slot grid, stackable materials, no weight | Exact owned plugin, slots, capacity, stash and controller UX. |
| Terrain provider | One real-content proof, then select one | Candidate version compatibility, performance, persistence format. |
| Terrain boundaries | Allow useful edits; identify critical-location rules | Whether and where no-dig/no-build protection is acceptable. |
| World persistence | Persistent edits/buildings; separately defined respawns | Resource and enemy respawn rules, region resets, migration policy. |
| Water | One water owner with explicit excavation behavior | Flooding/non-flooding rule, swimming state, underwater camera. |
| Dragon remote control | Preserve the original humanoid and explicit risk | Waiting/following behavior, range, damage and death response. |
| Dragon riding | Real rider on suitable rigs, safe land/dismount | Asset feasibility, both flight cameras, recovery and balance. |
| Building | Small functional camp using one building owner | Support rules, permitted locations, fast travel, upgrade extent. |
| Regional identity | Verdant Wilds / Green Dragon first | Story premise, exact map/quests, encounter and reward design. |
| Content scale | One stable slice before expansion | Launch region count, campaign length, enemy/item/dragon quantities. |

This document does not close those decisions by calling a recommendation “approved.”

---

## 14. Bounded next work package

The next detailed design deliverable is **Region 01 / Green Dragon Vertical Slice Design**, using the proposed loop and system boundaries above while preserving open decisions.

The first implementation work package, once the minimum specifications and actual project files are available, should be **G0 plus one G1 provider proof**, not the entire game. Its assignment should identify the exact project path and engine build, the chosen first candidate, the supplied test assets, the digging/addition/save/navigation acceptance checks, and what constitutes a blocker.

Use Luna for inventory/reconnaissance, Terra for ordinary contained work, Sol where voxel/Mutable/GAS integration complexity warrants it, and Astra only for exceptional unresolved problems. These are the handoff’s routing labels; this document does not assert their availability in a particular coding environment. [H01 §26]

The agent report should state what changed, what was actually run, what passed, what is still missing, and the next single bounded task. Do not fabricate a successful import, screenshot, benchmark, or package result. Do not create CI, branches, PRs, pushes, or replacement art merely to make the report appear complete.

**Planning outcome:** The project now has a proposed normal-session loop, player-progression model, system ownership map, integration contracts, persistence requirements, and a staged path to the full Green Dragon slice. Implementation and content compatibility remain unproven.

---

## Sources and evidence boundaries

### H01 — User-supplied project baseline

`Pasted markdown.md` — *Voxel ARPG — Project Handoff*, uploaded in this conversation. Section references identify the relevant portions of that document. Its approved decisions take precedence over new proposals in this file. Asset names and ownership claims here are carried from H01, not independently audited.

### Primary technical references checked September 14, 2026

**R1 — Epic Games: Gameplay Ability System.** Documents abilities, attributes, Ability System Components, and Gameplay Effects; does not supply this game’s balance rules.  
`https://dev.epicgames.com/documentation/unreal-engine/gameplay-ability-system-for-unreal-engine`

**R2 — Epic Games: The Mutable Sample Project is now available.** Documents runtime skeletal mesh/material/texture customization; does not certify the supplied voxel character parts.  
`https://www.unrealengine.com/news/the-mutable-sample-project-is-now-available`

**R3 — Epic Games: UCustomizableObjectInstance API.** Documents parameter access, update delegates, and asynchronous skeletal-mesh generation/update functions. Confirm exact signatures in the installed engine.  
`https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Plugins/CustomizableObject/UCustomizableObjectInstance`

**R4 — Voxel Plugin: Runtime Edits & Sculpting, documentation branch labeled 2.0p8 (latest).** Documents sculpt operations, save/load entry points, and limitations including extracting terrain-removal quantities. This is not an evaluation of other voxel candidates or of the user’s installed build.  
`https://docs.voxelplugin.com/knowledgebase/blueprints/runtime-edits-and-sculpting`

**R5 — Epic Games: Modifying the Navigation Mesh.** Distinguishes Dynamic generation from Dynamic Modifiers Only, including the latter’s inability to create new navigation surfaces.  
`https://dev.epicgames.com/documentation/unreal-engine/overview-of-how-to-modify-the-navigation-mesh-in-unreal-engine?lang=en-US`

**R6 — Epic Games: Saving and Loading Your Game.** Documents custom SaveGame objects and synchronous/asynchronous storage APIs; coherent game-state capture and multi-payload consistency remain project responsibilities.  
`https://dev.epicgames.com/documentation/en-us/unreal-engine/saving-and-loading-your-game-in-unreal-engine`

**R7 — Epic Games: UAbilitySystemComponent API.** Documents owner/avatar concepts and component APIs; creature/control policy in this plan is a project-specific proposal.  
`https://dev.epicgames.com/documentation/unreal-engine/API/Plugins/GameplayAbilities/UAbilitySystemComponent?lang=en-US`
