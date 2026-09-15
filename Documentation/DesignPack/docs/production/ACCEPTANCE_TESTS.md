# Acceptance Procedures and Regression Catalogue

**Documentation release:** v0.1 · September 14, 2026  
**Status:** Detailed design proposal; not an implementation report  
**Basis:** H01 risk-based testing; A01 G0–G5; B03 critical regression list; this pack specifications; see [source register](../SOURCES_AND_EVIDENCE.md).  
**Rule:** Existing requirements remain binding. New numbers and detailed behavior are draft baselines for a bounded prototype, not claims of user approval or runtime validation.

## Evidence status and execution rules

**Every one of the 118 cases below is NOT RUN.** These are authored procedures, not test results or an implemented automated suite. They can be grouped into focused playthroughs and small deterministic fixtures; do not create one large infrastructure component per row or run the entire catalogue after every edit.

Use explicit disposable test slots/worlds and real approved assets. Record project/revision, engine/plugin versions, hardware/settings, test data, actual commands or player actions, observed outcome and evidence path. Mark PASS, FAIL or BLOCKED only after execution. A blocked asset/runtime prerequisite is not a pass; an untested later feature is not a claim of absence from the intended game.

Deterministic/fault-injection cases protect important corruption/duplication boundaries. Visual/movement/animation cases need actual PIE/runtime observation. Cooked checks occur at risky integration or connected milestone, not after every change. Existing suitable tests should be reused. Testing does not authorize CI, branches, pushes, new production art or user-data deletion.

Values referenced by a case come from the canonical owner specification. Do not hard-code a duplicate cooldown table into this document. Setup may use an explicitly labeled test fixture/save for a later mechanic; that fixture proves its rule, not the finished story encounter.

## Readiness / G0

| ID | Case / setup and action | Expected result | Method | Requirement | Result |
|---|---|---|---|---|---|
| RDY-01 | **Actual environment:** Inspect authorized .uproject, engine association/build and working-tree state. | Record real paths/versions without changing another project or assuming a branch. | Inspection | REQ-20 | NOT RUN |
| RDY-02 | **Real assets:** Open the selected humanoid, terrain materials, enemy/weapon and Green Dragon assets; inventory clips/bounds. | Reported names are separated from located/imported/verified capabilities; missing inputs explicit. | Inspection | REQ-05 | NOT RUN |
| RDY-03 | **Owner conflicts:** Inspect inventory, health, interaction, building, water and save implementations/plugins. | Exactly one intended authority each; incompatible or unselected candidates recorded. | Inspection | REQ-06,REQ-18 | NOT RUN |
| RDY-04 | **Baseline provenance:** Compare current requirements, v0.3 and active specs; inspect source/licensing metadata of selected assets. | No superseded town/car rules or manufactured artwork; provenance is recorded without assuming redistribution rights. | Inspection | REQ-05,REQ-20 | NOT RUN |
## Editable world / G1

| ID | Case / setup and action | Expected result | Method | Requirement | Result |
|---|---|---|---|---|---|
| WRLD-01 | **Useful dig:** In a new fixed-seed test region excavate an actual blocked passage, then walk through it. | Visible geometry and collision change together at documented completion; useful route remains. | PIE | REQ-02 | NOT RUN |
| WRLD-02 | **Add and fill:** Fill part of that passage using finite construction material, then collide against it. | Accepted fill consumes only committed budget and restores solid collision; no phantom wall or free material. | PIE + deterministic | REQ-02 | NOT RUN |
| WRLD-03 | **Finite ore:** Harvest a deposit, fill and mine the same volume, then repeat completion callback. | Original deposit yield does not return; each accepted portion pays once. | Deterministic + PIE | REQ-02,REQ-07 | NOT RUN |
| WRLD-04 | **New-surface nav:** Request AI and top-down click movement through a newly excavated traversable route. | Route uses actual new navigation surface after readiness, not teleport/stale original nav. | PIE | REQ-01,REQ-02 | NOT RUN |
| WRLD-05 | **Stale route:** Start movement through a passage, refill its route and requery. | Affected route invalidates/stops with feedback; no actor moves through solid terrain. | PIE | REQ-01,REQ-02 | NOT RUN |
| WRLD-06 | **Stacked cave floor:** Click reachable lower floor beneath an upper cave surface; then direct-move to cancel. | Correct destination layer or honest rejection; direct movement cancels immediately. | PIE | REQ-01 | NOT RUN |
| WRLD-07 | **Narrow protection:** Attempt editing a critical device support and ordinary adjacent ground. | Only declared local envelope rejects; useful adjacent digging remains legal. | PIE | REQ-02,REQ-21 | NOT RUN |
| WRLD-08 | **Occupied fill:** Try fill/build intersecting humanoid, compact dragon and current interaction working space. | Reject without spending, burying, pushing through geometry or deleting actors. | PIE | REQ-02,REQ-13 | NOT RUN |
| WRLD-09 | **Camp support:** Place supported camp piece, then dig away its required support. | First-baseline edit rejects clearly; demolish/reposition path remains available. | PIE | REQ-08 | NOT RUN |
| WRLD-10 | **Bounded wet bed:** Edit allowed bed inside water volume, swim across surface and enter adjacent dry cave. | Consistent wet/dry/swim state from one owner, real underwater cues, dry cave remains coherent. | PIE | REQ-08 | NOT RUN |
| WRLD-11 | **Water boundary:** Try excavation exposing a prohibited outer water edge/bottom separation. | Narrow documented rejection, no floating invisible lake or falsely claimed fluid flow. | PIE | REQ-02 | NOT RUN |
| WRLD-12 | **Representative edit stress:** Run declared repeated dig/add sequence and stream across representative test extent; record machine/settings. | No observed lost edits/collision errors; report actual latency/memory/save measurements, not invented budgets. | PIE + profiling | REQ-02,REQ-03 | NOT RUN |
| WRLD-13 | **Region generation:** Generate several explicit test seeds and inspect all required connections/arrival/arena/return anchors. | Each accepted world has valid landmark graph and ordinary humanoid routes; reject invalid new-world seed before play. | Deterministic + PIE | REQ-03 | NOT RUN |
## Mutable character / G2 and breadth

| ID | Case / setup and action | Expected result | Method | Requirement | Result |
|---|---|---|---|---|---|
| CHAR-01 | **Real recipe:** Generate an actual supported body/part/color recipe and play its locomotion. | Mutable result is valid and animated; original created identity persists. | PIE | REQ-04 | NOT RUN |
| CHAR-02 | **Async revision:** Request rapid part/color revisions, including completion arriving out of order. | Only latest intended valid revision commits; old result cannot overwrite it. | Focused deterministic + PIE | REQ-04 | NOT RUN |
| CHAR-03 | **Generation failure:** Force an invalid/pending appearance result and attempt Finish/equip. | Last committed appearance/stats remain coherent; specific error and no invisible/default reset. | PIE | REQ-04,REQ-07 | NOT RUN |
| CHAR-04 | **Equipment attachment:** Equip actual weapon and armor, animate and unequip/re-equip repeatedly. | Correct sockets/appearance and one set of effects; no body-baked free inventory. | PIE + deterministic | REQ-04,REQ-07 | NOT RUN |
| CHAR-05 | **Recipe restart:** Save recipe/equipment, quit process and load. | Same resolved parts/colors/name and exact item references; no rerandomization. | Restart + record comparison | REQ-04,REQ-18 | NOT RUN |
| CHAR-06 | **Cooked runtime:** Run focused packaged/runtime Mutable creation and part-change with real assets. | Works outside editor preview; record actual build/environment or blocker. | Cooked gameplay | REQ-04 | NOT RUN |
| CHAR-07 | **Creator breadth:** Compare real accessible validated part/color counts to C01 production milestone; test shortest/tallest/broadest. | Counts represent real content; animation/rider clipping unsupported presets are reported, not dummy entries. | Inspection + PIE | REQ-04 | NOT RUN |
| CHAR-08 | **Creator UX/randomization:** Use category locks, undo/redo, presets and randomize on mouse, keyboard and controller. | Compatible recipes and preserved locks; final intended preview/save match. Broader random-case test is milestone-only. | PIE + focused deterministic | REQ-04,REQ-19 | NOT RUN |
## Core combat / G3

| ID | Case / setup and action | Expected result | Method | Requirement | Result |
|---|---|---|---|---|---|
| COM-01 | **Damage fixture:** Execute the canonical L1 damage example and type-split cases through the authoritative combat owner. | Expected result from combat formula; armor/resistance applied once, finite nonnegative outputs. | Deterministic | REQ-06 | NOT RUN |
| COM-02 | **Critical and drain bounds:** Apply overkill, shield/invulnerable/friendly/dead targets and repeated secondary damage. | Only eligible actual damage feeds permitted effects; no overkill drain or recursion. | Deterministic | REQ-06,REQ-14 | NOT RUN |
| COM-03 | **Two-camera combat:** Fight the same enemy with same loadout while switching third-person/top-down mid-encounter. | Identical actor/state/damage; readable targeting/telegraphs and no attack reset. | PIE | REQ-01,REQ-06 | NOT RUN |
| COM-04 | **Cost/cooldown commit:** Try ability with missing Focus, invalid target, cooldown; then duplicate a valid activation request. | Invalid requests spend nothing; valid action commits once; repeated request does not double spend or cast. | Deterministic + PIE | REQ-06 | NOT RUN |
| COM-05 | **Status interaction:** Apply slow/stagger/root/stun and the defined boss resistance alternative. | Declared stacking/suppression/resistance feedback; no undocumented universal immunity. | Deterministic + PIE | REQ-06 | NOT RUN |
| COM-06 | **Item rolls/equip:** Receive randomized drop, record instance/rolls, equip, save and restore repeatedly. | Rolls and ID unchanged; grants/removals exactly once, not cumulative bonuses. | Deterministic + restart | REQ-07 | NOT RUN |
| COM-07 | **Transfers/full bag:** Move exact item between bag/stash/drop while filling the bag and retrying a failed pickup. | One owner/location; source persists on failure; no lost or duplicate item. | Deterministic + PIE | REQ-07 | NOT RUN |
| COM-08 | **Second build:** Complete comparable encounter with materially distinct real melee and ranged kits. | Both usable from either camera; legal equipment/ability requirements, not identical UI-only choice. | PIE | REQ-06,REQ-07 | NOT RUN |
## Coherent saves and control

| ID | Case / setup and action | Expected result | Method | Requirement | Result |
|---|---|---|---|---|---|
| SAVE-01 | **Snapshot roundtrip:** Capture recipe/items/equipment/HP/timers/terrain/depletion/storage/quests in a stable test slot; restart. | All committed records match; same IDs, rolled values and world edits. | Deterministic + restart | REQ-18 | NOT RUN |
| SAVE-02 | **Compound edit save:** Request save during an accepted mining operation before reward/depletion completion. | Capture waits for stable operation; saved geometry/depletion/output agree with one action receipt. | Fault injection | REQ-02,REQ-18 | NOT RUN |
| SAVE-03 | **Failed disk generation:** Interrupt/fail a required provider or state payload write before new-generation publication. | Previous complete generation remains loadable; no partial new state claimed saved. | Fault injection | REQ-18 | NOT RUN |
| SAVE-04 | **Missing/incompatible content:** Load test save with absent required part/dragon/provider/vehicle definition or mismatched schema. | Specific preserved-save error; no new world/character/dragon silently replaces it. | Fault injection | REQ-18 | NOT RUN |
| SAVE-05 | **Timer continuity:** Save with active effect and cooldown, close game, reload and unequip/re-equip. | Remaining gameplay durations persist without offline tick or loadout reset. | Deterministic + restart | REQ-14,REQ-18 | NOT RUN |
| SAVE-06 | **Grant duplication:** Replay a completed quest/reward/equip callback and load twice. | Reward receipts/equipment grants/unlocks remain one set. | Deterministic | REQ-07,REQ-14,REQ-18 | NOT RUN |
| SAVE-07 | **Load geometry first:** Restore edited arrival area with delayed terrain collision readiness. | Actor input/physics are not enabled inside unedited ground; no fall before readiness. | Fault injection + PIE | REQ-02,REQ-18 | NOT RUN |
| SAVE-08 | **Remote compact save:** Save while compact Verdance is remotely controlled with original humanoid waiting; restart. | Same two identities/locations, form, health, timers and control/tether context restore safely. | Restart | REQ-11,REQ-13,REQ-18 | NOT RUN |
| SAVE-09 | **Mounted flight save:** Save supported airborne rider/dragon state; reload with valid space, then test obstructed saved position. | Original rider/dragon restore or disclosed safe ground recovery; no duplicate body/heal/terrain erasure. | Restart + PIE | REQ-12,REQ-18 | NOT RUN |
| SAVE-10 | **Living-defeat boundaries:** Save after boss defeated alive, after claim broken and after bond separately. | Correct next interaction each time, no corpse, second full fight, skipped consent or duplicate bond recovery. | Restart | REQ-10,REQ-18 | NOT RUN |
| SAVE-11 | **Beast form save:** Save in actual Moonbound form, restart, expire/return near changed clearance. | Original recipe/equipment/logical combat identity restored; remaining duration respected; no permanent beast or fresh human. | Restart + PIE | REQ-04,REQ-14,REQ-18 | NOT RUN |
| SAVE-12 | **Occupied/parked car save:** Save driving with attached pet and separately parked with staged pet; restart. | One driver/car/dragon record; correct control, locations, damage and attachments. | Restart | REQ-16,REQ-18 | NOT RUN |
| SAVE-13 | **Attempt/retry distinction:** Save mid unresolved boss attempt, load, then explicitly choose checkpoint retry. | Load restarts attempt with saved player state; only disclosed retry grants recovery. Committed edits/rewards/rescues persist. | Restart + PIE | REQ-18,REQ-21 | NOT RUN |
## Dragon and Heartfold / G4, G4-H

| ID | Case / setup and action | Expected result | Method | Requirement | Result |
|---|---|---|---|---|---|
| DRG-01 | **Living outcome/role conversion:** Defeat Verdance with large hit plus lingering DOT, free/bond, then replay completion. | Living terminal state and once-only allied profile/recovery/unlock; no corpse or cloned dragon. | PIE + deterministic | REQ-10 | NOT RUN |
| DRG-02 | **Companion combat:** Fight relief/ordinary enemy alongside same bonded dragon, issue Attack and Return. | Actual autonomous attacks and valid command behavior with persistent own health/cooldowns. | PIE | REQ-11 | NOT RUN |
| DRG-03 | **Direct control/body:** Possess dragon, look back at original created humanoid, move/attack then return. | Actual input-controlled dragon; original visible body/inventory/health preserved. | PIE | REQ-11 | NOT RUN |
| DRG-04 | **Remote risk/tether:** Damage waiting body, approach warning/limit and defeat controlled dragon in separate cases. | Documented return/recovery, no invulnerable body, teleport or free cleanse. | PIE | REQ-11 | NOT RUN |
| DRG-05 | **Original mount:** Mount true grounded dragon with representative body/equipment presets. | Original humanoid attached correctly, not default cosmetic duplicate; compact mount rejected. | PIE | REQ-12 | NOT RUN |
| DRG-06 | **Flight and landing:** Take off, steer/attack where supported, collide/check obstacles, land/dismount in both camera preferences. | Real movement/collision and supported safe landing; invalid landing rejected, not scripted taxi proof. | PIE | REQ-12 | NOT RUN |
| DRG-07 | **Mounted entry/failure:** Approach town airborne/mounted; test blocked dismount and dragon defeat. | Land/dismount before folding; no dropped rider or lost identity; disclosed recovery. | PIE | REQ-12,REQ-13 | NOT RUN |
| DRG-08 | **Compact size/collision:** Fold then follow/control through measured ordinary door and narrow cave. | Visible mesh and collision/nav/attack envelope actually compact; no invisible boss collider. | PIE | REQ-13 | NOT RUN |
| DRG-09 | **Compact fight:** Fight a real cave enemy using AI and direct compact control. | Small dragon contributes accepted damage/control and can be targeted fairly. | PIE | REQ-13 | NOT RUN |
| DRG-10 | **Form state conservation:** Injure dragon, apply status and cooldown, repeatedly fold/unfold. | Same ID/current/max allied HP, resource/effects/cooldowns; no reapplication of bond recovery. | Deterministic + PIE | REQ-13,REQ-18 | NOT RUN |
| DRG-11 | **Blocked growth:** Request growth below a low cave or newly built ceiling. | Clear rejection leaves valid compact dragon; no clipping, push-through or demolished player building. | PIE | REQ-13 | NOT RUN |
| DRG-12 | **Interrupted form:** Begin valid transition then hit dragon/change target clearance before commit. | Roll back last stable form while retaining damage and accepted cooldown; no duplicate state. | PIE + deterministic | REQ-13 | NOT RUN |
| DRG-13 | **Town behavior:** Visit merchant/homecoming with compact companion. | Usable following/interactions, no autonomous citizen attacks, trampling or total UI occlusion. | PIE | REQ-13,REQ-21 | NOT RUN |
| DRG-14 | **Recovery/inactive identity:** Defeat ally then use explicit hub recovery; switch away/back when supported. | Nonpermanent same dragon; only named recovery heals, inactive state preserved. | PIE + record comparison | REQ-11,REQ-18 | NOT RUN |
| DRG-15 | **Different rig policy:** Inspect next dragon, especially long-bodied Jadefang, against Green mappings/dimensions. | Unsupported clips/shape/rider assumptions blocked and recorded; first rig not treated as universal proof. | Inspection + later PIE | REQ-12,REQ-13,REQ-22 | NOT RUN |
## Nightmare Echoes / E1 and later powers

| ID | Case / setup and action | Expected result | Method | Requirement | Result |
|---|---|---|---|---|---|
| ECHO-01 | **Real Counselor reward:** Resolve actual Silent Landing fight, observe enemy stance and first unlock. | Relentless Advance is permanently learned and usable, with separate ordinary loot, not icon-only reward. | PIE | REQ-14,REQ-15 | NOT RUN |
| ECHO-02 | **Relentless statuses:** Activate against existing/new slow, stagger, damage, hard stun and root in separate cases. | Only declared slow/light-medium stagger suppression; still damaged, hard control not universally removed. | Deterministic + PIE | REQ-14 | NOT RUN |
| ECHO-03 | **Relentless failure/time:** Attempt invalid activation then valid cast; cancel/unequip/reload during cooldown/effect. | Correct one-time cost, duration and retained cooldown; no refund/reset/health gain. | Deterministic + restart | REQ-14,REQ-18 | NOT RUN |
| ECHO-04 | **Full bag/double outcome:** Fill bag, resolve Counselor, replay outcome callback and return to scene. | Unlock once outside bag, ordinary loot preserved once, no repeated stat/reward gains. | Deterministic + PIE | REQ-14,REQ-18 | NOT RUN |
| ECHO-05 | **Optional skip/dragon support:** Skip hunt and travel onward; separately attempt hunt with bonded dragon in legal form. | No campaign gate or arbitrary anti-dragon ban; ordinary fishing remains safe. | PIE | REQ-11,REQ-15 | NOT RUN |
| ECHO-06 | **Proc cap:** Combine authored repeat/drain/storage/helper effects in controlled fixtures. | Secondary/reflected/repeated effects cannot recursively trigger or exceed documented caps. | Deterministic | REQ-14 | NOT RUN |
| ECHO-07 | **Mercy parity:** Resolve Corvyn or disciple via actual authored cure/trust path; compare hostile valid resolution. | Equivalent signature capability and distinct correct narrative facts, no reward penalty for mercy. | PIE + deterministic | REQ-14,REQ-15 | NOT RUN |
| ECHO-08 | **Actual beast:** Activate real Moonbound Form, use unique claw/pounce then return. | Real form/kit rather than glow buff; original recipe, item ownership and health preserved. | PIE | REQ-04,REQ-14 | NOT RUN |
| ECHO-09 | **Beast return blockage:** Block humanoid return volume during form; expire/interruption/save-load. | Bounded same-side safe return/error policy without infinite combat extension, teleport exploit or duplicate human. | Fault injection + PIE | REQ-14,REQ-18 | NOT RUN |
| ECHO-10 | **Final Judge unlock:** Repel earlier authored Judge, then complete final permanent defeat in separate fixture. | Only final outcome grants Break the Verdict once; recurring fights are not repeatable permanent rewards. | Deterministic + PIE | REQ-14,REQ-22 | NOT RUN |
## Region 01 story and sequence breaks / G5

| ID | Case / setup and action | Expected result | Method | Requirement | Result |
|---|---|---|---|---|---|
| REG-01 | **Arrival excavation:** Create character, escape by real dig and reach Tamsin/Tidecross. | Immediate objective/personal mystery clear; exit edits remain and no prescribed Earth identity. | PIE | REQ-04,REQ-21 | NOT RUN |
| REG-02 | **Worker facts:** Secure Pell/Iven/Sella individually and inspect journal/staging after each. | Only actual rescued workers counted, no invisible escort failure or real-time death timer. | PIE + deterministic | REQ-21 | NOT RUN |
| REG-03 | **Sella first:** Reach Sella before notice/Tamsin or other workers. | Witness/auxiliary available and later dialogue reflects known facts without reset to step one. | PIE | REQ-21 | NOT RUN |
| REG-04 | **Evidence redundancy:** Miss one optional record while observing machinery/meeting witness. | Conflict remains understandable; missing single note does not make boss impossible. | PIE | REQ-21 | NOT RUN |
| REG-05 | **Rusk bypass:** Dig legal flank to arena, then return to unresolved Rusk after bond. | No forced out-of-order cutscene; actual later custody branch, queen responsibility retained. | PIE | REQ-02,REQ-21 | NOT RUN |
| REG-06 | **Auxiliary impact:** Fight with and without auxiliary shutdown in matched test states. | One identifiable interference pattern removed, both valid routes winnable, real boss remains. | PIE | REQ-10,REQ-21 | NOT RUN |
| REG-07 | **Consent sequence:** Win boss, inspect release then bond actions. | Defeat alone grants no control; claim broken before voluntary bond; no enslavement campaign falsely offered. | PIE + deterministic | REQ-10 | NOT RUN |
| REG-08 | **Relief use:** Complete actual postbond crown encounter with companion. | Dragon reward used in gameplay; neither cutscene-only attacks nor second dragon unlock. | PIE | REQ-11 | NOT RUN |
| REG-09 | **Late rescue:** Bond before securing all workers, return to town, then finish rescue. | Pet/control available immediately; partial debrief true; full homecoming waits until all three secured. | PIE + deterministic | REQ-21 | NOT RUN |
| REG-10 | **Persistent local recovery:** End extraction/homecoming after mining/building and optional wage recovery. | Selected hazards/dialogue change; no world regeneration, ore refill, lost camp or repeated reward. | Restart + PIE | REQ-02,REQ-18,REQ-21 | NOT RUN |
| REG-11 | **Skip preparation:** Complete main story without fishing/cooking/camp/Counselor/cave. | No hidden activity/skill gate, hunger or compulsory chores; onward lead remains. | PIE | REQ-08,REQ-09,REQ-15 | NOT RUN |
| REG-12 | **Compact cave:** Complete A Smaller Kind of Strength using compact combat/control, then valid and invalid growth. | Humanoid remains safely reachable; no required teleport through pet hole or second bond. | PIE | REQ-13,REQ-21 | NOT RUN |
| REG-13 | **Horror separation:** Fish at Quiet Water and independently enter Silent Landing. | Safe lesson stays ordinary; deliberate horror entry/retreat/resolution, no random stalker elsewhere. | PIE | REQ-15 | NOT RUN |
## Activities and construction

| ID | Case / setup and action | Expected result | Method | Requirement | Result |
|---|---|---|---|---|---|
| ACT-01 | **Catch transaction:** Cast with actual rod at valid water, respond/reel and accept catch. | Real catch enters authoritative inventory once; sounds/UI alone not completion. | PIE + deterministic | REQ-08 | NOT RUN |
| ACT-02 | **Cook/use:** Cook actual catch and consume result; compare input/output/health. | Correct finite ingredients, usable benefit, no hunger requirement or duplicated outputs. | PIE + deterministic | REQ-08,REQ-09 | NOT RUN |
| ACT-03 | **Cancel/damage/full bag:** Cancel or take damage during fishing/cooking; repeat with full bag at reward. | Settles safely, no free catch/input loss; committed output remains represented once. | PIE + deterministic | REQ-08,REQ-18 | NOT RUN |
| ACT-04 | **Buff replacement:** Consume same then different preparation buff, change max Focus gear and reload. | Declared replace/refresh/duration behavior, no unlimited stacks or free Focus refill. | Deterministic + restart | REQ-08 | NOT RUN |
| ACT-05 | **Craft failure:** Attempt missing ingredients, invalid station and capacity failure, then valid recipe. | Failed actions spend nothing; successful action consumes/produces once. | Deterministic | REQ-07,REQ-08 | NOT RUN |
| ACT-06 | **Useful camp:** Place real supported foundation/walls/door/storage/workpoint and use them. | Functional coherent camp, valid snap/rotation/entry and real supplied content. | PIE | REQ-08 | NOT RUN |
| ACT-07 | **Placement rejection:** Preview unsupported/overlapping/occupied/protected locations and cancel. | Clear reason and no consumed materials/hidden actor. | PIE + deterministic | REQ-08 | NOT RUN |
| ACT-08 | **Storage identity:** Move rolled item bag→storage→bag; save while transfer requested. | Exactly one owner and identical roll/ID after reload. | Deterministic + restart | REQ-07,REQ-18 | NOT RUN |
| ACT-09 | **Demolition overflow:** Demolish storage containing items while bag full; retry refund callback. | Contents/materials persist in accessible recovery bundle once; no loss/duplication. | Deterministic + PIE | REQ-08,REQ-18 | NOT RUN |
| ACT-10 | **Camp persistence/pet:** Save camp, finish chapter, return with pet, try growing below roof. | Same camp/storage/terrain; compact access works and blocked growth respects roof. | Restart + PIE | REQ-08,REQ-13,REQ-18 | NOT RUN |
## Input, UI and camera accessibility

| ID | Case / setup and action | Expected result | Method | Requirement | Result |
|---|---|---|---|---|---|
| UI-01 | **Device-only routes:** Complete core creator/HUD/inventory/quest/skill/dragon flows separately controller-only and mouse/keyboard. | Equivalent actions, valid glyph/focus and no inaccessible confirmation. | PIE | REQ-19 | NOT RUN |
| UI-02 | **Context/chords:** Use LB skill chord then release, open/close menu with attack held, enter/exit car or dragon. | One action/context consumes input; no accidental attack/jump/interact or stuck held action. | PIE | REQ-01,REQ-19 | NOT RUN |
| UI-03 | **Camera continuity:** Switch cameras in combat, cave, compact/full dragon and later vehicle. | Same actors/state; usable framing and correct floor targeting, no progression reset. | PIE | REQ-01,REQ-19 | NOT RUN |
| UI-04 | **Blocked feedback:** Attempt growth, mount, landing, dismount, vehicle exit, save and equip in invalid states. | Specific current cause and consistent pending/failed/committed state, not silent button failure. | PIE | REQ-13,REQ-16,REQ-19 | NOT RUN |
| UI-05 | **Text/tooltips:** Use long names/affixes/quest lines and increased UI/text scale. | Readable nonclipped layout, exact rolled comparison values and consistent focus. | PIE | REQ-07,REQ-19 | NOT RUN |
| UI-06 | **Non-audio/non-color cues:** Disable audio/reduce flashes and inspect fishing bite, boss tell, Counselor stance and rarity. | Gameplay-critical cues remain understandable without sound or color alone. | PIE | REQ-15,REQ-19 | NOT RUN |
| UI-07 | **Pause/remapping:** Pause/background game, change bindings and trigger deliberate conflicts. | Gameplay time stops as declared; binding conflicts visible and action semantics preserved. | PIE | REQ-19 | NOT RUN |
| UI-08 | **Actual art adaptation:** Inspect real supplied UI/props/VFX in both cameras and around compact companion. | Not unauthorized replacements; legible enemies/telegraphs and no permanent interaction occlusion. | Visual review | REQ-05,REQ-19 | NOT RUN |
## Flying vehicle / V1 later

| ID | Case / setup and action | Expected result | Method | Requirement | Result |
|---|---|---|---|---|---|
| VEH-01 | **Original occupant:** Enter real authorized parked hovercar with created/equipped humanoid. | Same original driver attached; correct recipe/gear and one control owner. | PIE | REQ-16 | NOT RUN |
| VEH-02 | **Actual piloting:** Drive freely, ascend/descend/brake/turn under player input in both views. | Continuous control and actual movement, not spline taxi or ambient traffic only. | PIE | REQ-16 | NOT RUN |
| VEH-03 | **Collision and landing:** Approach tower/structure at cruise speed and land on valid/invalid surfaces. | Swept collision prevents tunneling; clearance/ground validation and honest rejection. | PIE | REQ-16 | NOT RUN |
| VEH-04 | **Blocked/air exit:** Request exit in flight, while moving too fast, beside wall and over void. | Driver remains safely occupied; clear reason and valid landing/recovery option, no dropped duplicate. | PIE | REQ-16 | NOT RUN |
| VEH-05 | **Pet passenger:** Board/exit with validated compact dragon anchor and repeat where anatomy cannot fit. | Same pet attached/staged explicitly; no invisible duplicate follower, lost state or healing. | PIE | REQ-13,REQ-16 | NOT RUN |
| VEH-06 | **Disable/recover:** Disable propulsion via declared collision/hazard and recover at depot. | Safe driver/pet staging, same car ID/ownership/damage lifecycle; no second free car. | PIE + record comparison | REQ-16,REQ-18 | NOT RUN |
| VEH-07 | **Occupied/parked persistence:** Save/reload occupied and parked/staged cases, change nearby terrain where legal. | Correct IDs/locations and disclosed safe recovery without erasing terrain. | Restart | REQ-16,REQ-18 | NOT RUN |
| VEH-08 | **Traffic separation:** Stream/despawn ambient cars while keeping owned car parked/occupied. | Cosmetic traffic lifecycle cannot delete player vehicle or block essential landing forever. | PIE | REQ-16 | NOT RUN |
| VEH-09 | **Mecha progression:** Use ordinary car before Mecha bond, then resolve bond/circuit access. | Ordinary transport still operates, advanced benefit meaningful; city traffic does not all crash. | PIE + quest inspection | REQ-16,REQ-22 | NOT RUN |
## Playable colony / Z1 later

| ID | Case / setup and action | Expected result | Method | Requirement | Result |
|---|---|---|---|---|---|
| COL-01 | **Actual destination:** Travel to colony, dismount/fold, walk/use an interior interaction and return. | Real playable destination and return route, not backdrop or cutscene-only landing. | PIE | REQ-17 | NOT RUN |
| COL-02 | **Transfer identity:** Transfer via validated authored route with recorded recipe/items/dragon and parked city car. | Same identities/state once at destination; car still parked in Zenith, not spontaneously a spacecraft. | Restart + record comparison | REQ-17,REQ-18 | NOT RUN |
| COL-03 | **Transfer failure:** Fail destination load before commit, then retry before/after committed arrival. | Source or destination authority consistent; no duplicate arrival/driver/pet/car. | Fault injection | REQ-17,REQ-18 | NOT RUN |
| COL-04 | **Colony encounter reward:** Complete actual Broodmother dungeon and separately leave without attempting it. | Real encounter/one-time Brood Seed when resolved; skipped encounter does not strand player. | PIE | REQ-14,REQ-17 | NOT RUN |
| COL-05 | **Gravity/flight scope:** Inspect route and gameplay claims against actual movement capabilities. | No seamless spaceflight/zero-gravity claim without proof; preferred dragon transition openly identified. | Inspection + PIE | REQ-12,REQ-17 | NOT RUN |
## Full-world ending later

| ID | Case / setup and action | Expected result | Method | Requirement | Result |
|---|---|---|---|---|---|
| END-01 | **Home continuation:** Choose home with camps/items/dragons/Echoes/car owned; inspect continuation options. | Named pre-departure adventure branch preserved and loadable; only playable world not overwritten. | Restart + record comparison | REQ-18,REQ-22 | NOT RUN |
| END-02 | **Stay/world identity:** Complete willing reconnection and stay; revisit altered regions/camp. | Distinct regions/history and saved edits/assets/ownership remain; no forced full-world regeneration. | PIE + restart | REQ-18,REQ-22 | NOT RUN |

## Requirement coverage and release use

All REQ-01…REQ-22 are referenced by at least one case. V1/Z1, transformation and full-ending rows are future milestones, not G5 completion requirements. G5 uses applicable readiness/world/creator/combat/save/dragon/region/activity/UI rows and the first six Echo cases; full creator breadth and later-rig cases are tracked separately.

For a task report, list only executed case IDs, actual result, evidence and remaining risks. A single captured playthrough can satisfy several cases when each behavior is visible and individually assessed. Known progression blockers or repeatable save/item/reward/depletion loss cannot be waived by reporting a large count of unrelated passed checks.
