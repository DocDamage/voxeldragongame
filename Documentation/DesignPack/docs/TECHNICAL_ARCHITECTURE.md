# Technical Architecture — One Game, One Set of Owners

**Documentation release:** v0.1 · September 14, 2026  
**Status:** Detailed design proposal; not an implementation report  
**Basis:** H01; B03; A01; see [source register](./SOURCES_AND_EVIDENCE.md).  
**Rule:** Existing requirements remain binding. New numbers and detailed behavior are draft baselines for a bounded prototype, not claims of user approval or runtime validation.

## 1. Architecture boundary

This is a proposed integration architecture for the actual project to be inspected in G0. Named records and functions are logical contracts, not existing C++ classes, Blueprint assets or verified APIs. Preserve existing suitable marketplace implementations instead of recreating them to match a diagram. Unreal/module versions remain unverified.

```text
Input actions → Player control coordinator → active controllable actor
                       │                      │
                 camera/UI context      actor movement + combat
                       │                      │
Interaction intents → authoritative gameplay owners → committed events
                       │
      inventory/equipment   quests/rewards   terrain/resources
      Mutable recipe        dragons/Echoes   buildings/water
                       │
             one coherent save coordinator
```

This separation does not authorize a generic message bus or one subsystem per box. A handful of components/records in the existing project can own these responsibilities. GAS offers abilities/attributes/effects [R1]; Enhanced Input offers action/context machinery [R6]; the game still defines all ownership and transitions.

## 2. Ownership map

| State or responsibility | Proposed authority | Key invariant |
|---|---|---|
| Character identity/progression | Persistent player record plus its live humanoid | Possession does not create a new character. |
| Character appearance | Validated recipe → Mutable appearance component | Save recipe, not transient generated meshes. |
| Humanoid combat | Humanoid logical GAS owner | Retains health/status/cooldowns while waiting or transforming. |
| Dragon combat | Each dragon's logical GAS owner | Form/control changes do not copy player stats or heal it. |
| Inventory/stash | One selected inventory implementation | Item instance belongs to exactly one container/location. |
| Equipment | One equipment authority | Owns slots and tracked grant handles; no independent UI bonuses. |
| Input/possession | One player control coordinator | Exactly one controllable target/context wins at a time. |
| Terrain shape, collision and serialized edits | One selected provider | Provider edit completion is real, not inferred from button press. |
| Resource depletion/yield | Project resource records alongside provider edits | Yield tied to committed edit/deposit, not regenerated geometry. |
| Constructed buildings/support | One building owner | Separate from terrain and from story landmark state. |
| Wet/dry/swimming queries | One water-state owner | Shader appearance alone does not define water gameplay. |
| Quests and one-time rewards | Small quest/outcome rule layer | Facts and reward IDs committed consistently. |
| Dragon bonds / Echo unlocks / vehicle ownership | Typed records under run progression | Not three new currencies or duplicate inventory databases. |
| Vehicle movement/occupancy | Vehicle actor and control coordinator | Real original humanoid occupant; saved once. |
| UI/audio/VFX | Presentation over committed state | A reward animation cannot grant rewards itself. |
| Save capture/storage/load | One coordinator; provider payloads as needed | One complete generation, previous valid one retained. |

Inventory may physically live on the humanoid if that is the owned product's model. Keep/reconstruct that owner deliberately during region transitions; do not duplicate it on the controller, dragon and save subsystem. Records identify the owner; a thin access adapter can route UI actions.

## 3. Data identities and definitions

Definitions describe authored content (`ItemDefinition`, `DragonDefinition`, `EchoDefinition`, `RegionDefinition`, `VehicleDefinition`, `RecipeDefinition`). Instance records describe the user's state (`ItemInstanceID`, `DragonID`, `VehicleID`, `RegionInstanceID`, `ResourceDepositID`, `StructureID`, `CharacterID`). Definitions have content versions; saves have schema and generation versions.

Examples such as `dragon.verdance` and `echo.relentless_advance` are stable design IDs, not real asset references. Map them to verified assets in G0/content integration. Do not use actor memory addresses, foliage array indices, UI order or localized display names as persistence keys.

A region baseline includes generator version, seed, parameters, resolved critical landmark placements and provider format. Seed alone is insufficient after generator/content changes. Store enough baseline/edits for the actual selected provider; compatibility policy must be tested rather than promised.

## 4. Control dimensions

Do not encode every combination in one flat “mode.” Track independently:

| Dimension | Representative values |
|---|---|
| Controlled target | Humanoid, dragon, occupied vehicle |
| Humanoid presentation | Original recipe, temporary beast form |
| Dragon relationship | Hostile, defeated alive, freed, bonded, recovering |
| Dragon physical form | True, companion, transition pending |
| Dragon decision owner | AI, direct player input |
| Rider/occupant relation | None, humanoid riding dragon, humanoid occupying vehicle |
| Locomotion | Grounded, swimming, airborne, landing, disabled |
| Camera preference | Third-person, top-down; tuned per active target |

A dragon can be bonded, compact and directly controlled at once. A mounted dragon is true-form/player-controlled with the original humanoid attached. A vehicle passenger pet is the same active compact dragon with AI/combat suspended by occupancy, not a second pet actor.

Validate combinations centrally through small explicit transitions. Mounting a compact dragon, remotely controlling a dragon from a car, or entering a car as a beast is invalid in the first baseline. These are documented interaction limits, not accidental missing code.

## 5. Critical transaction contracts

**Equipment:** validate owned item → prepare required appearance/attachment → commit slot and tracked GAS grants once → notify UI. Failure retains previous committed state. Coalesce preview requests using revision IDs; old Mutable completions cannot overwrite a newer intended appearance [R2]. Cosmetic preview does not grant equipment.

**Mining:** validate reach/tool/material/edit envelope → request provider edit → observe required geometry/collision completion → commit deposit/material-budget consumption plus inventory reward or persistent overflow → publish success. Record the accepted action ID so repeated callbacks cannot pay again. Defer save capture during an incomplete compound operation.

**Building/crafting:** validate materials/space/station → reserve a single transaction → commit structure/output and consume inputs together → notify. Failed preview, overlap or asset load is non-spending. Demolition/storage recovery must preserve exact contained item IDs, not manufacture replacement stacks.

**Quest/bond/Echo:** evaluate fact predicate → apply outcome + unlock + one-time reward ID in one committed state → show dialogue/animation. No dialogue widget or level trigger directly grants a second reward. Save an intermediate defeated-alive dragon correctly before the later claim/bond action.

**Control/form:** validate target state and space → lock conflicting transitions → safely end applicable actions → change attachment/collision/input/avatar references → rebind ability UI → release lock. Preserve identities, resources and remaining timers. Roll back to last stable valid control state on failure; no duplicate spawn used as rollback.

These contracts require bounded state coordination, not a distributed transaction platform. Critical save snapshots wait for a stable boundary; failures surface clearly and retain last complete disk generation.

## 6. Terrain, navigation and water

New terrain can change ground surfaces. Navigation must be rebuilt or updated with the actual provider's supported runtime behavior; Dynamic Modifiers Only is insufficient for new surfaces [R4]. Queries distinguish separate cave floors and validate routes before action. Pending updates invalidate affected paths. Manual movement cancels click-to-move.

Different bodies need appropriate navigation/clearance: humanoid, compact dragon, full dragon. One giant capsule or invisible full-size collider behind a pet is unacceptable. Aerial motion uses obstacle/landing checks, not presumed ground-nav support. Vehicles use movement collision and clearance, not a city traffic spline for player piloting.

The water-state owner supplies `IsWet`, surface/depth, safe swimming state and relevant hazard queries. The first region uses explicitly bounded water, not a full excavation-driven fluid solver. Fishing and underwater effects consume the same water query. Detailed protected-site and support policies live in [editable-world rules](systems/EDITABLE_WORLD_RULES.md).

## 7. World updates and presentation

Quest recovery changes selected actor states and props. It never regenerates a region or re-runs ore/foliage placement over player changes. Critical landmarks have stable IDs and local generation constraints; ordinary approaches vary. Real actor locations may be recovered safely after edits without clearing those edits.

UI subscribes to stable target/owner references after possession. Separate display bindings show waiting humanoid risk and controlled dragon health. Damage effects consume confirmed hits; anticipation cues can precede impact but do not apply gameplay independently. Animation notifies can request validated impacts, but duplicate/end callbacks require identity checks.

## 8. Integration order and tests

G0 locates versions/content; G1 proves terrain with material/collision/resource/nav/save; G2 proves Mutable including a focused cooked test; G3 connects the ARPG loop; G4 proves original-rider/direct/flight behavior; G4-H adds compact form; E1 proves a real horror ability; G5 connects the authored opening and both optional/skip paths. V1 and Z1 follow as bounded later capabilities.

Use the smallest useful developer map and focused deterministic tests. Do not build every map, system and infrastructure layer before the first real-content loop. Per-feature “implemented” status needs evidence in the workspace's `Documentation/Current/HANDOFF.md`; the [DesignPack v0.1 handoff](production/CURRENT_HANDOFF.md) is historical and this document alone establishes no runtime result.
