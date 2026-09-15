# Optional Activities, Crafting and Robust Camps

**Documentation release:** v0.1 · September 14, 2026  
**Status:** Detailed design proposal; not an implementation report  
**Basis:** H01 §§13–15,18,23; A01 §10; B03 optional content; see [source register](../SOURCES_AND_EVIDENCE.md).  
**Rule:** Existing requirements remain binding. New numbers and detailed behavior are draft baselines for a bounded prototype, not claims of user approval or runtime validation.

## 1. Required usefulness without maintenance

Fishing, cooking, gathering, resource harvesting/crafting and useful camps support ARPG adventuring. Main quests remain completable without them. Their usefulness comes from items, buffs, convenience and personal space—not hunger, thirst, compulsory sleep, spoilage, temperature babysitting, dragon care or recurring fuel chores.

Use the same inventory, equipment, interaction, water, building and save owners as the rest of the game. A new activity UI does not own its own fish/item collection. Real rods, tools, pieces, sounds and appropriate animations are required before its proof passes.

## 2. Gathering and crafting

A resource interaction validates reach, equipped tool where needed, depleted state and actual accepted action. Terrain deposits follow the edit/depletion transaction; trees/plants use stable object IDs and explicit harvest state. No regeneration caused by camera switching, region re-entry, chapter completion or rest. The slice's finite-resource rule is separate from any eventual respawn design.

Recipe definition: stable ID, station/capability, input quantities, output definition/quantity, unlock predicate and presentation. Craft instance: selected recipe, input reservation/validation, committed output instance IDs and transaction receipt. Preview costs nothing. Confirm either consumes inputs and produces results consistently or leaves the original inventory unchanged.

Do not discard materials because the output bag is full. Prevalidate output space or deliver the output into a persistently represented station/pickup container using the same item IDs. Repeated completion animation does not repeat the craft reward.

## 3. First useful recipe set

These are draft recipe roles/names to map to real item definitions, not invented existing asset paths.

| Recipe | Inputs / result baseline | Adventure purpose |
|---|---|---|
| Simple cooked fish | 1 valid raw fish → 1 cooked fish | Consumable restores 25 health, clamped to missing HP; not hunger relief. |
| Field broth | 1 eligible fish/material food + 1 gathered herb → 1 broth | One preparation buff: +10% maximum Focus for 5 gameplay minutes, no instant Focus refill. |
| Basic field remedy | 2 appropriate gathered ingredients → 1 remedy | Existing healing consumable family; uses same quick-use inventory rules. |
| Camp utility piece | Real building definition's material cost | Useful floor/storage/work point, not compulsory quest key. |
| Simple equipment improvement | Existing eligible item + finite crafting material | Small authored upgrade preserving item ID/rolls; implement only after actual inventory supports it. |

Health/damage balance for combat stays in the combat spec. These recipe quantities are activity baselines. No final loot/economy claim is made without actual item definitions. Food-buff category allows one preparation buff: consuming same refreshes duration; consuming a different one replaces by explicit confirmation. No unlimited food stacking. Saved remaining time follows gameplay time, not application-closed time.

## 4. Fishing interaction

States: Ready → Aiming/Casting → Waiting → BiteWindow → Reeling → CatchPending → Caught/Cancelled. Validate a real rod/attachment, accessible approved water and line/position constraints. A visible catch and inventory transaction complete the loop; hearing a splash alone does not count.

BASELINE bite interaction uses a forgiving prompted input followed by a short hold-to-reel, with toggle/assist alternatives. No mandatory rapid mashing. A missed bite returns to an understandable waiting/recast state without erasing unrelated inventory. Catch species/weights can use curated random tables per water location, but first proof only needs one genuine usable catch and a contrasting possible result if supplied.

Combat/damage or movement beyond the station condition cancels the activity safely. Cancellation before catch commit awards nothing and consumes no hidden survival resource. Save requests settle/cancel the fishing action to a stable boundary; they cannot duplicate a pending catch. A full bag retains caught-item availability through the declared overflow policy.

Quiet Water is the normal safe lesson. Silent Landing is a separate optional horror site; enabling fishing never grants a global slasher spawn system. Darkness in scenery does not require the player to wait for a real-time night cycle to complete the horror story.

## 5. Cooking/station UX

Interact with the real cooking station → inspect recipes and exact costs/results → choose quantity → validate/reserve → short supported activity presentation → commit outputs → show actual inventory result. Batch crafting can wait until single-item transaction reliability is proved. No cooking rhythm game is implied by owning a Cooking Minigame SFX pack.

Stations outside a player-built camp are available so optional building cannot block crafting/cooking. Consumables show health/buff effect, duration, replacement rule and eligibility. Full health use warns/rejects a purely healing item rather than silently wasting it; intentional override is a later UX choice. Food never spoils in the slice.

## 6. Camp piece and function catalogue

Minimum useful camp: foundation/floor, wall, doorway/door, roof or equivalent shelter piece where supplied, stairs where useful, storage container, and cooking/crafting work point. Show valid placement, snapping/free placement where the owner supports it, rotation, support feedback, cost, repair if actual damage is implemented, demolition and persistence.

The first proof can be a small coherent enclosure, not an enormous prefab settlement. Robustness means the operations work correctly: occupied-space rejection, overlapping pieces, invalid support, removal/refund, storage identity, saved transformation and edited terrain. Decorative breadth comes after these.

Building owner performs preview/support/snap. Inventory owner performs material/contained-item transactions. Terrain provider performs ground edits. No second support solver runs beside Easy Building System merely because a reference repo exists. Missing essential building functionality is a documented candidate blocker.

## 7. Support, removal and storage

Initial supports follow [editable-world rules](EDITABLE_WORLD_RULES.md): reject unsupported placement and edits removing necessary support; do not let a foundation float accidentally. Permit deliberate demolition/reposition as the way to change the supported layout. Full collapse/physics simulation is not implicitly approved.

Demolishing an empty piece refunds a BASELINE 100% of its recorded construction materials in the slice, subject to persistent overflow rather than item loss. Damaged-item refund economics remain OPEN. Demolishing storage first transfers/retains every exact contained item instance in an accessible recovery bundle; do not delete it because the bag is full. The structure's consumed-material receipt prevents repeated refund calls.

Storage item ownership is authoritative whether UI is open or closed. Moving an item bag→chest→bag is one transfer per action, never copying records. Save during a transfer captures before or after, not both. Undo-like building preview cancellation is not an inventory refund after an already committed placement.

## 8. Camps, dragons and world progression

A compact dragon can follow into appropriately sized camp spaces, hold nearby and avoid blocking doors/storage interactions. Camps do not require feeding/sleeping it. A newly built ceiling may prevent growth; the form request is rejected clearly without demolishing that ceiling.

No main quest asks the player to build a house before rescuing workers. Camps persist when extraction stops or the region's local dialogue changes. Fast travel from arbitrary camps is not part of the first baseline; do not add it as an assumed building benefit. Safe service/checkpoint recovery uses explicitly defined locations.

## 9. Acceptance

ACT cases prove gather→craft, cast→catch→cook→use, buff replacement/time persistence, full bag handling, cancel/damage interaction, camp supported placement and invalid rejection, exact storage transfers and demolition/refund. G5 requires a successful optional-activity route and a separate playthrough skipping these without blocking the main story. All tests are NOT RUN.
