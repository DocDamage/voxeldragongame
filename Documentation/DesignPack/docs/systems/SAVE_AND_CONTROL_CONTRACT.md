# Save, Identity, Control and Recovery Contract

**Documentation release:** v0.1 · September 14, 2026  
**Status:** Detailed design proposal; not an implementation report  
**Basis:** A01 §§6–9; B03 integration/regression rules; H01; R2–R3; see [source register](../SOURCES_AND_EVIDENCE.md).  
**Rule:** Existing requirements remain binding. New numbers and detailed behavior are draft baselines for a bounded prototype, not claims of user approval or runtime validation.

## 1. Invariants

The same created humanoid, dragons, items, structures and vehicles survive camera switches, possession, transformation, regional travel and reload. A transient actor can be reconstructed; its logical identity and committed state cannot be replaced with a fresh default. One active controller, one active dragon record, one authoritative item location and one complete save generation are the baseline.

No ordinary load, form change, equipment preview, summoning or possession is a free heal, resource refill, cooldown reset, cleanse, re-roll or duplicate unlock. Explicit recovery actions are named exceptions and have a saved event/outcome, not accidental side effects.

## 2. Logical save schema

Field names are proposed contracts, not engine reflection declarations. Native SaveGame supports custom storage [R3]; the project must author consistent capture and reconstruction.

| Record | Required content |
|---|---|
| Header | Schema/content versions, save generation ID, slot/character/world IDs, timestamp, required provider/module versions, payload inventory and integrity metadata. |
| Character | Identity/name, validated appearance recipe/version, level/XP, current attributes, learned skills/Echoes, equipped skill IDs, effect and cooldown remaining durations, temporary form state. |
| Inventory/equipment | Exact item instance records and rolled values, container ownership, equipped item references, currency/material balances. Runtime GAS handles are rebuilt, not blindly deserialized. |
| Region | Region definition and instance ID, seed, generator/version/parameters, resolved critical landmarks, provider baseline/edit payload references, world revision. |
| Resources/objects | Deposit IDs and depletion, harvested/removal state, persistent loot IDs/contents/locations, reward ledger, placed structures/support/content records. |
| Quests | Stable facts, outcome variants, one-time dialogue/reward markers, unresolved encounter attempt identity and checkpoint. |
| Dragon | Stable ID/definition, relationship/unlock, active/inactive location, form, health/resource/effects/cooldowns, movement state, controller relation, rider reference, bond reward receipt. |
| Vehicle | Stable ID/definition, ownership, region/transform, damage/disabled state, original occupant reference, pet attachment/staging references, recovery/transit status. |
| Control | Active target ID/type, original humanoid location, camera preference, rider/occupant relation, last safe arrival/return positions; no raw controller pointers. |
| Travel/ending | Transfer ID and source/destination arrival commitment; preserved pre-departure save branch for home epilogue. |
| Settings | Difficulty/assists as applicable, input profile and camera/accessibility preferences; distinguish user-wide settings from world state. |

Do not create a general serializer for arbitrary running ability tasks. Save stable logical durations, targets and supported persistent effects. Definitions may declare that an effect is transient and ends at a safe save/attempt boundary; that behavior must be documented and not erase earned unlocks.

## 3. Coherent capture and disk commit

Serialize at a consistent gameplay boundary. Stop accepting a conflicting inventory/edit/form/occupancy transaction during capture, finish or safely cancel its pending operation, then freeze/copy the stable records. Initial maximum deferral before showing “save waiting for operation” is 3 seconds; do not claim a completed save while waiting. A cancelable failed operation rolls back before capture. A stuck provider operation is an explicit error, retaining the previous save.

Use one save payload where practical. When terrain/provider data requires multiple files, every piece uses the same generation ID; write new payloads, validate their required integrity/size metadata, and publish the generation as complete last. Keep the last complete generation until the new one is durable under the implemented storage strategy. Engine asynchronous storage alone does not provide cross-file atomicity [R3].

Do not gather mutable actor state from arbitrary worker threads. Copy stable data first, then serialize/write through supported mechanisms. Record separate capture, provider serialization, write and completion timings when measuring hitches. A save icon reflects actual completion, not the start of a request.

## 4. Load order

1. Read/validate header and required content/provider formats without modifying the old slot.
2. Restore compatible region baseline and terrain edits, resources/removals, structures and persistent pickups.
3. Make collision, water-state queries and relevant navigation ready around each required arrival/return location.
4. Reconstruct logical humanoid/inventory/progression, regenerate Mutable appearance from the recipe and apply equipment grants once.
5. Restore dragon/vehicle identities and supported effects/timers; resolve safe positions before attachments.
6. Restore temporary humanoid form, rider/occupant/pet relationships and the single active control target.
7. Bind UI/input/camera to authoritative owners, then allow simulation.

Missing required content or incompatible provider data stops load with a specific non-destructive error. Do not “repair” it by silently loading a fresh world, a different dragon or a default character. A backup restore is an explicit user action and preserves the failing slot for diagnosis.

## 5. Timer and attempt policy

Saved timers are remaining gameplay time. They do not tick while the application is closed or paused. Switching control leaves statuses on their logical owner, ticking when gameplay runs; UI may show the waiting humanoid's remaining protection. Unequipping a skill does not remove its cooldown record.

Ordinary safe saves preserve health/resources/effects. During an unresolved authored boss attempt, a manual save records committed world progress and the player's current logical state but labels the attempt **restart on load**: the player resumes at its validated approach checkpoint, and the unresolved boss resets. The UI warns of this behavior before saving. Loading does not refill the player's health/Focus; explicit retry/rest at that checkpoint can perform the normal disclosed recovery action. Unfinished attack tasks/projectiles are not resumed as arbitrary serialized execution.

A defeated-alive dragon, broken claim, bonded ally or resolved Counselor is a committed outcome, not an unresolved attempt. Their outcome must not reset. Do not use the restart policy to undo terrain edits, consumed items, rescued workers, learned powers or previously awarded loot.

## 6. Control transition matrix

| Request | Required conditions | Commit and failure behavior |
|---|---|---|
| Humanoid → remote dragon | Active bonded healthy dragon, same region, in tether range, humanoid original form, no rider/vehicle or other transition | Original body stays visible/damageable; stop companion movement and bind dragon input. On failure remain humanoid. |
| Remote dragon → humanoid | Manual request, body damaged, tether limit, dragon defeat or supported interruption | Return to same body, resume appropriate dragon AI; never copy health between them. |
| Mount dragon | True form, grounded, valid rider socket/clearance, original humanoid available | Attach original humanoid, route player control to dragon; no cosmetic duplicate. |
| Dismount | Grounded valid nearby supported space for actual humanoid | Detach/re-enable original body there; reject blocked/airborne dismount rather than drop it. |
| Heartfold | Living bonded dragon, grounded, no rider/passenger conflict, target envelope fits | Preserve state; one transition at a time; interrupted operation returns last stable form. |
| Enter car | Authorized vehicle, safe stationary entry, original humanoid, valid seat/pet policy | Attach original occupant, route input; if pet cannot safely stage, explain and reject or request explicit staging. |
| Exit car | Safely landed/stopped, validated supported exit | Return same body; blocked exit remains occupied, with clear error. |
| Moonbound entry/return | Valid original/beast return envelopes; no rider/vehicle/remote dragon transition | Preserve recipe/inventory/logical combat owner; never instantiate another character profile. |
| Region/colony travel | Stable source state and a valid destination/arrival bundle | Publish one transfer ID; commit source parking/staging and destination arrival once. Retry does not duplicate actors. |

## 7. Remote body rules

BASELINE: the original humanoid waits where control was transferred and remains normally damageable. No autonomous following/fighting AI is implied. Remote control is same-region with a 150m maximum straight-line tether and warning at 120m, supplemented by region/streaming validity. At the limit, stop further outward input and return control rather than teleport the humanoid through obstacles.

Any accepted damaging hit on the waiting body requests immediate safe return after that damage resolves. If lethal, use humanoid defeat recovery rather than possessing a dead body. Hard-control state survives the return; the player is not cleansed. Dragon defeat similarly returns to the living body. Mounted control has no remote tether because the original body is present on the dragon.

Remote interaction is dragon-capability limited. It may perform authored dragon interactions, not open every humanoid merchant UI or carry the whole inventory to the far end of a tunnel. Quests cannot require the humanoid to teleport through a pet opening.

## 8. Explicit recovery actions

**Humanoid defeat:** retry at last valid checkpoint, disclosed health/Focus recovery and transient effect termination, retained gear/XP/world edits/outcomes. Outstanding skill cooldowns persist through ordinary loading; a full checkpoint retry may clear them as part of the named recovery event, saved once. Do not treat merely opening a save slot as retry.

**Companion defeat:** same dragon becomes recovering and unavailable for control/mount. At a safe hub, a Recover Companion interaction restores it to its allied baseline health/resource and clears transient combat effects as an explicit event. No permanent loss, cloned baby or consumable-feeding requirement.

**Bonding:** Verdance's hostile profile is converted once to its allied profile, then the first bond explicitly recovers it to at least 50% allied maximum health. Record that receipt once. See dragon specification; no subsequent fold/reload may rerun it.

**Invalid placement:** select a validated same-region/same-side safe anchor appropriate to the body. Priority is saved position → nearby clearance search without crossing a sealed boundary → last safe anchor → explicit checkpoint recovery with explanation. Do not reshape the player's terrain to make space. If none is available, preserve state and report a recoverable load failure.

**Vehicle disabled:** retain vehicle identity and inventory/ownership. Safe depot recovery is explicit, not a fresh free car spawn. Pet/driver are safely staged by the vehicle rules; no explosion/death presentation unless supported and specified.

## 9. Multi-entity and travel failure cases

On mounted airborne restore, validate dragon flight capability, unobstructed swept envelope and rider attachment before resuming. Unsupported location falls back to a validated ground landing anchor with disclosed recovery; health/ability state persists. Do not claim flight saving works until that case is actually tested.

Car passengers, detached pets, inactive dragons and staged colony vehicles remain represented exactly once. If a colony transfer fails before destination commit, source state remains authoritative. After commit, destination owns arrival; retries resolve the same transfer rather than issuing a second vehicle. A parked car stays in Zenith during dragon-to-colony travel unless later vehicle transport is separately designed.

For the home ending, create/retain a named pre-departure continuation snapshot. The epilogue does not overwrite the only playable world. Continue Adventure explicitly loads that branch; it is not canonically pretending the departed character never left.

## 10. Required tests and evidence

Inspect persisted records as well as visible state for item rolls, unique IDs, current HP, remaining timers and one-time receipts. Include duplicate callback, mid-transaction save request, interrupted write, missing-content load, unsafe terrain arrival, compact remote control, mounted flight, vehicle occupancy and temporary beast restoration. No Unreal test has been run for this package; [acceptance tests](../production/ACCEPTANCE_TESTS.md) specify the procedures.
