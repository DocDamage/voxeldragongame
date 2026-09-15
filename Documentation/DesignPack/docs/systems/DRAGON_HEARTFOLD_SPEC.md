# Dragons and Heartfold — Verdance First

**Documentation release:** v0.1 · September 14, 2026  
**Status:** Detailed design proposal; not an implementation report  
**Basis:** H01 §16; A01 §9; B03 Heartfold/Chapter 01; see [source register](../SOURCES_AND_EVIDENCE.md).  
**Rule:** Existing requirements remain binding. New numbers and detailed behavior are draft baselines for a bounded prototype, not claims of user approval or runtime validation.

## 1. Feature promise

A bonded dragon is a real creature with its own identity, behavior and relationship. Heartfold is its voluntary compact form, not imprisonment, baby replacement, pet-care progression or a second creature inventory. It follows into towns and ordinary dungeon passages, fights meaningfully, and can be directly controlled. True Form supports its larger kit and validated mounting/flight.

Use actual supplied dragon models/animations. This document's semantic actions and dimensions are proposed targets, not discovered asset capabilities. One dragon proving a behavior does not prove all ten rigs. See [asset registry](../ASSET_REGISTRY.md).

## 2. Definition and instance data

Dragon definition: stable type/character ID, real model/rig references, animation mappings, true/compact form profiles, collision and navigation envelopes, locomotion capability flags, ability definitions, rider socket, presentation cues, and regional story role.

Dragon instance: stable dragon ID, relationship state, current form, current health/Focus and remaining effects/cooldowns, AI/player control relation, rider reference, progression/unlock, region/transform, inactive/recovery state, and one-time bond receipt. Per-form profiles cannot each keep their own health or independent cooldown copy.

A single framework covers Hostile → DefeatedAlive → Freed → Bonded → Recovering. Form, control and locomotion remain separate dimensions. Before consent, Heartfold commands and allied UI are unavailable; defeat alone is not ownership.

## 3. Verdance dimensional and movement prototype

These numbers are an **initial test envelope**, not measurements of the imported Green Dragon. G0 must measure actual animated bounds and choose a uniform mesh scale that fits the intended compact silhouette without distorting anatomy. Maximum wing/tail sweep requires separate clearance from the ground movement core.

| Profile field | Companion Form target | True Form target |
|---|---|---|
| Approximate visible nose-to-tail length | 0.9m | 8m |
| Standing body height | Source proportions at uniform compact scale; aim at or below 0.6m where anatomy permits | Source proportions at the selected true scale, measured during G0 |
| Movement core radius / half-height | 0.30m / 0.35m initial proxy | 1.2m / 1.6m initial proxy |
| Locomotion speed | 4.5m/s follow; 6m/s catch-up on valid route | 7m/s ground; 12m/s supported flight cruise |
| Attacking footprint | Short measured head/claw sweeps | Measured front and area attack sweeps |
| Aerial behavior in first proof | Ground-follow/direct-ground control; decorative wing motion grants no flight collision bypass | Validated takeoff/flight/landing with swept wing/body clearance |
| Rider | Never | Original humanoid on verified attachment |

The collision proxies must contain the relevant torso and correctly prevent tunneling; use additional sweeps for head/tail/wing attacks. Do not claim a pet is collision-correct merely because its visible mesh fits a doorway. Exact locomotion/turn radius/nav agent dimensions are finalized from real assets, then stored in one data profile.

Long-bodied Jadefang gets its own silhouette/turning solution, not nonuniform squash to meet Verdance's proportions. All compact forms should read as the same adult characters. Do not require ten new baby models.

## 4. Combat profiles and one-way bonding conversion

Canonical Verdance fixture: hostile boss max HP 1800 at the intended first balancing encounter; allied max HP 420 and Focus 100. These are BASELINE values, not final difficulty. Hostile and allied damage use separate data so a boss-scale hit is not automatically inherited by a tiny companion.

The boss transitions to DefeatedAlive at its terminal defeat threshold (HP reaches zero in its **defeat logic**, not a kill/death handler). Lethal overflow, lingering DOT and repeated hits cannot spawn a corpse or repeatedly issue rewards. Make it target-ineligible after the terminal outcome, cancel its offensive actions, and preserve the living defeated presentation.

On first bond only: convert hostile remaining-health fraction to allied maximum, then explicitly recover to **at least 50% of allied maximum**. At a zero-health defeated outcome this means 210 HP. Commit the allied role, ability grants and recovery receipt once. Later form changes retain the same allied maximum/current health; no repeated 210-HP recovery through summon/save/fold.

| Semantic action | Boss behavior | Allied True Form | Allied Companion Form |
|---|---|---|---|
| Front strike | Readable committed bite/claw attack; draft 22 damage | Close strike, draft 24 damage | Short precise strike, draft 9 damage |
| Area pressure | Supported sweep/turn attack; draft 16 damage | Limited area strike, draft 18 damage; 6s cooldown | One-target interruption against susceptible ordinary foe, draft 6 damage; shared family cooldown 6s |
| Wilderness ability | Only if actual animation/effect support exists | Focused nature/pressure ability, authored after first core kit | Small localized support version; never full arena-filling effect from tiny body |
| Binding interference | Apparatus hazard; draft 12 damage on accepted hit | Not part of allied kit | Not inherited as a pet ability |

A front strike and one contrasting supported attack are the minimum functional combat kit. If a named animation is absent, choose a supported attack that preserves its telegraph/recovery role and revise its mapping visibly; do not fabricate an asset. The third thematic skill is an expansion, not a prerequisite to truthful two-attack proof.

Both forms remain useful: compact attacks offer precision/interrupt contribution; full attacks offer reach/area. Health parity prevents transformation healing exploits but is not immunity. Enemies can target and hit compact Verdance using its actual hurt volume. No invisibly huge boss hitboxes around the pet, and no impossible-to-target tiny proxy.

## 5. Companion AI and commands

Commands: Follow, Hold, Attack Target, Return/Regroup, direct-control request, and form request. One active dragon initially. Follow preserves a comfortable offset, avoids blocking interaction focus/doors, and uses a valid navigation route. Town mode suppresses autonomous hostility and damaging collision with citizens while retaining the same dragon state.

Attack Target validates hostility, perception/range and reachable attack position. The companion engages with its own kit and cooldowns; Return cancels a valid pursuit and resumes navigation. Hold does not grant invulnerability. Do not require a broad behavior-tree framework if the existing modest AI solution supports these states.

For a temporarily blocked follower path, show its location and allow a valid alternative route/explicit safe regroup. A recovery reposition must not bypass a sealed quest boundary, transport loot, retrieve an unearned boss, or silently heal. Never auto-grow to force passage through a door. Following/staging policy must also account for boats/vehicles only when their actual integration exists.

Inactive bonded dragons retain their records. Selecting another active dragon happens out of combat at a safe location in the first baseline; current health/recovery state persists. No simultaneous ten-dragon army, kennel maintenance or population simulation.

## 6. Heartfold transition protocol

Manual form request is available on the dragon command interface; approaching an authored town/interior entry may request automatic compact form. A town label does not excuse dropping a mounted rider or forcing growth in an undersized room.

BASELINE transition duration is 1 second with a shared 4-second recovery timer committed on accepted transition. Require living bonded dragon, grounded stable locomotion, no rider/vehicle occupant attachment, no control/form transition already pending, and a clear target envelope. No shrink in flight in the first baseline; land/dismount first. A directly controlled compact dragon can request growth in a suitable chamber while preserving the waiting humanoid tether.

Preflight validates current and target collision/animated bounds against terrain, structures, actors and protected travel boundaries. Reserve a valid transformation volume. During presentation, use conservative nonpenetrating collision and suppress attacks/movement that would invalidate the operation. Accepted damage can interrupt before commit; return to the previous form and retain damage/cooldown, not a fresh state. Revalidate immediately before commit because the world can change.

On success, update mesh scale, collision profile, navigation dimensions, movement tuning, camera framing, attack traces/effect scale and form-specific action mapping together. Corresponding ability families retain cooldowns even when their action mapping changes. If presentation fails, keep the last valid form, report it and do not grant another health pool.

On insufficient space: “Not enough room for True Form.” On mounting conflict: “Land and dismount first.” On combat transition cooldown: show remaining time. Feedback must be specific and use actual current state, not a generic unavailable button.

## 7. Direct control, mounting and flight

Direct control actually routes input to the dragon and its abilities. Compact direct control is not a destination order. The same original humanoid waits in the world with normal damage/collision. Range, damage response, save and restore follow the [shared control contract](SAVE_AND_CONTROL_CONTRACT.md); do not duplicate the constants here.

Mount attaches the original humanoid at the actual verified socket; animated body proportions must fit without creating a different cosmetic rider. Rider collision/damage policy BASELINE: dragon receives normal external combat targeting while mounted; explicitly authored rider hazards can still affect the humanoid. Do not grant general humanoid invulnerability while it is detached/waiting. Dismount requires supported clear space and restores the original combat target.

Takeoff checks body/wing sweep and allowable destination volume. Flight uses obstacle avoidance/collision, attack restrictions and sufficient landing room. Both camera preferences need usable tuned dragon views. A top-down dragon camera may pull back and alter pitch without becoming a second game or deleting that mode.

Landing finds a valid supported surface, considers steep slope/obstacles, and checks rider/dragon clearance. A requested dismount in flight is rejected; explicit emergency recovery is defined, not a fall caused by a town trigger. Mounted dragon defeat invokes the shared recovery path, safely placing the rider or triggering disclosed checkpoint recovery rather than losing its inventory.

## 8. Chapter 01 demonstrations

After breaking the claim, a short crown relief encounter demonstrates actual allied combat. A safe terrace demonstrates direct control and returning to the same visible body. A validated mounting/flight route demonstrates riding, with a grounded narrative route retained only to prevent a story blocker—not counted as a flight pass.

At Tidecross, approach the entry buffer, land/dismount if required, fold once, and follow the player to the homecoming. Small Verdance is present during ordinary merchant/quest interactions without trampling citizens or obscuring the screen.

A Smaller Kind of Strength provides the compact cave proof: actual low route, real compact combat, safe waiting body, control transfer, blocked growth check, and a wider chamber where full size works only if its envelope genuinely fits. The cave and Counselor are optional to the player's onward story, but their cases must be implemented and demonstrated before the expanded slice is called complete.

## 9. Acceptance boundary

G4 proves real dragon control and rig-dependent mount/flight. G4-H independently proves compact collision, combat, direct control, transition/rejection and persistence. Save/quit/reload in AI companion, compact direct-control, true grounded mounted and supported airborne mounted states. Inspect identities, current HP and cooldowns across each path.

All results are NOT RUN. A scale slider, scripted ride, invisible rider duplicate, pet icon or boss-only model is insufficient. Additional dragons each require their own form/animation/collision and riding evidence before being called playable.
