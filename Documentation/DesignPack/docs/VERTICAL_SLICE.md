# Chapter 01 / Green Dragon Vertical Slice

**Documentation release:** v0.1 · September 14, 2026  
**Status:** Detailed design proposal; not an implementation report  
**Basis:** H01; B03; A01; see [source register](./SOURCES_AND_EVIDENCE.md).  
**Rule:** Existing requirements remain binding. New numbers and detailed behavior are draft baselines for a bounded prototype, not claims of user approval or runtime validation.

## 1. Definition of the first playable

A connected real-content adventure in Verdant Wilds demonstrates the actual game: create a character, use both cameras/direct and optional click controls, explore/edit terrain, fight for randomized equipment, rescue the workers, defeat/free/bond Verdance, fight with and control the same dragon, use supported riding/flight, enter Tidecross in compact form, complete a compact cave and optionally earn/use Relentless Advance. Save/quit/reload preserves the same world and identities.

Fishing, cooking, crafting and a useful camp are optional to the player's story, but working versions are part of the development proof. A second playthrough deliberately skips them and the Counselor, confirming onward progress is not gated. Cars/colony remain later milestones, not required forest content or canceled features.

## 2. Exact content boundary

| Included for connected G5 | Later, still tracked |
|---|---|
| One finite generated region with authored relationship constraints and real materials | Full production of the other nine regions, final campaign tuning/release count |
| Same customizable humanoid and two materially different combat loadouts | Full original creator content-breadth milestone and large NPC population |
| Limited actual wilderness/crown roles and one Rusk elite variant | Every enemy species, crowd/army systems, full faction war |
| Complete first dragon boss/bond/AI/direct/compact/validated mount-flight chain | Nine additional independently validated dragon kits/forms |
| First real horror encounter and playable persistent Relentless Advance | Nineteen later Echo encounters/powers, real Moonbound beast integration |
| Useful catch/cook/craft/supported camp/storage path | Huge building catalogue, advanced professions, extra minigames |
| Both cameras, controller/mouse/keyboard, usable supplied UI | Final visual polish/localization breadth/voice production |
| Coherent saves, finite depletion, protected-zone policy and bounded water | Advanced fluid simulation/structural collapse if separately approved |

G5 cannot be called fully passed while a required subproof is blocked. A grounded story fallback can make a partial development build navigable; it does not pass the flight acceptance line. Report each incomplete capability specifically, not as complete with an obscure footnote.

## 3. Gate sequence and evidence

| Gate | Real demonstration | Exit evidence / not a pass |
|---|---|---|
| G0 | Locate actual project/engine/content/plugin versions and candidate owners | Scoped records with real paths and missing inputs; a library title is not imported content. |
| G1 | Smooth dig/add/cave/collision/resource/nav/save with real materials and bounded water implications | Repeated edits and reload, measured behavior and finite yield; not editor-only sculpt. |
| G2 | Actual Mutable part/color/animation/attachment/recipe reconstruction | PIE and focused cooked runtime result; not a static character preview. |
| G3 | Same humanoid in both cameras, combat/loot/equip/mining/save loop and second build | Real visual and logical state; not disconnected demos with duplicated inventory. |
| G4 | Verdance AI combat, direct control/original body, real rider and supported flight/landing | Actual transfer/rider/collision/save results; not scripted travel shot. |
| G4-H | Compact/true transitions, town doorway, cave combat/control, valid/blocked growth | Same HP/ID/cooldowns and real compact collision; not scale slider. |
| E1 | Actual Counselor story encounter → permanent Relentless Advance → equip/use/save | Duplicate/full-bag/skip cases; a skill fixture alone is not authored encounter. |
| G5 | Full Region 01 sequence with alternate orders, optional/skip paths and persistent world | Focused packaged end-to-end result plus case list; not all text authored only. |
| V1 (later) | Real hovercar piloting/occupancy/landing/pet/save | Separate player vehicle proof, not ambient traffic. |
| Z1 (later) | Playable city and colony segment/transfer/return, then dungeon integration | Destination actually playable; not sky prop or cutscene. |

Run sequentially by default. Investigating G1/G2 risk independently can be useful but does not authorize parallel production architectures. Integrate dragon mechanics before polishing an expensive boss encounter. Keep one proof map per actual need rather than creating every conceivable test level immediately.

## 4. Connected player walkthrough

**Start/arrival:** create and save a valid appearance; escape the warm heart through a useful persistent dig. Meet Tamsin, fight a real enemy, enter Tidecross, read the workers' names and access real services.

**Exploration/preparation route:** switch cameras, use optional click movement then cancel with direct input, take a randomized drop, compare/equip it with correct appearance/stats, mine a finite deposit and open a shortcut. Optionally catch/cook/use a fish-derived item and construct/store items in a small supported camp.

**Quarry story:** secure actual workers, learn the extraction, disable the optional auxiliary, resolve Rusk, fight true-form Verdance, win the living-defeat opening, break the claim and accept the willing bond. Exact ordering follows the fact-driven Region 01 rules.

**Earned dragon use:** complete the relief encounter with actual companion attacks; control the dragon while the original body remains; mount and fly/land/dismount where validated; compact entry to Tidecross with all-three homecoming when true.

**Extended features:** optional cave demonstrates compact combat/direct control and legal/blocked growth. Optional Silent Landing grants the real Echo; equip, activate and observe its bounded protection. Do not require the player to finish these before seeing the onward Hallowwood lead.

**Persistence:** save, quit process, relaunch and compare IDs/recipe/item rolls/equipment/HP/timers/terrain/deposits/camp/storage/rescues/claim/bond/form/Echo outcomes. Merely reloading a level in the editor is not the full quit/reload case.

## 5. Alternate playthroughs

Route A is the authored rescue-before-boss route with optional content completed. Route B bypasses optional activities and some conversations, reaches the quarry through legal edits, bonds first where valid, then completes late rescue; it must not fake missing workers' homecoming. Route C targets persistence boundaries: save before/after defeat, before/after claim-breaking, before/after bond, during supported compact control and during active Echo.

These are focused QA playthroughs, not a mandatory player progression loop. Original user files and personal saves must never be used as disposable test data; create explicit test slots/worlds.

## 6. Completion criteria by requirement

All REQ-01 through REQ-15 and REQ-18 through REQ-21 have slice-level demonstrations. REQ-16/17 remain specified and tracked for V1/Z1; REQ-22 is preserved narrative coverage, not implemented ten-region evidence. Asset breadth outside the slice remains separately accounted for.

A passed slice has no known progression blocker in the required/alternate paths, no known repeatable inventory/reward/deposit duplication, no lost created identity after control changes, and no cleared player terrain/building state after chapter resolution. Performance is measured on declared hardware; the provisional region extent is adjusted from evidence, not hidden by deleting required interaction.

## 7. Evidence packet and stop rule

For a milestone record the project/revision/engine/plugin build, actual assets/maps used, test IDs run, commands/PIE actions, pass/fail/blocked outcomes, exact limitations and short evidence paths. Keep this small and relevant. Visual cases use screenshots/video and observation; deterministic corruption/duplication risks merit automated regression tests. Do not create an enormous framework or cook after each small edit.

All gates are **NOT RUN** in this documentation pack. The next actual work is G0 then one G1 candidate proof. Stop the bounded task when its acceptance passes or a reproducible blocker is documented; do not start nine regions “while here.”
