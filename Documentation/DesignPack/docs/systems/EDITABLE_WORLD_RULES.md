# Editable World — Terrain, Water, Navigation and Quest Safety

**Documentation release:** v0.1 · September 14, 2026  
**Status:** Detailed design proposal; not an implementation report  
**Basis:** H01 §§4–7,13,23; A01 §§7–8; B03 world/quest boundaries; R4–R5; see [source register](../SOURCES_AND_EVIDENCE.md).  
**Rule:** Existing requirements remain binding. New numbers and detailed behavior are draft baselines for a bounded prototype, not claims of user approval or runtime validation.

## 1. Core requirements and scope

Smooth editable terrain, digging, mining, addition/filling, tunnels and persistent changes are REQUIRED. Generated terrain shape/caves/resources/foliage use real approved assets. Constructed buildings remain a separate owned system. The first provider proof must demonstrate gameplay consequences, not just a sculpt cursor. [H01; A01]

The following localized restrictions are **BASELINE proposals for testing**, not newly approved permanent limits on the whole game. Validate/review them before relying on them for production layouts. Do not turn the convenient first proof into a universal no-dig world.

## 2. Region construction contract

A finite region is defined by seed, generator version/parameters, selected asset tables and resolved placements. Reserve authored landmark relationships and viable entrances/arena supports, then vary surrounding ground/caves/foliage/ordinary encounters/resources. Save the resolved critical placements so a later generator update cannot move Tidecross underneath a saved camp.

Generation order: define traversable region extent → place protected landmark envelopes and water bounds → generate terrain/caves respecting them → validate arrival and main/alternative routes → distribute resource IDs and foliage → place ordinary encounters → validate start/landmark/recovery clearance. Save the actual chosen baseline/versions. Failed validation regenerates a new candidate **before a new world is accepted**, never after the player has edited it.

A valid first region has at least one ordinary humanoid route between required locations and meaningful optional excavation bypasses. A pet-only route can provide utility/cache access but cannot be the sole way a stranded humanoid must travel. Full dragon flight can bypass patrols/walls without satisfying missing story facts by itself.

## 3. What can be edited

| Zone / object | Dig / fill / build baseline | Reason and player-facing handling |
|---|---|---|
| Ordinary hills, soil, rock, wilderness approaches | Dig/fill allowed; construction where supported | Core editable adventure, no blanket protection. |
| Ore/resource deposit | Dig through provider; finite budget tracked separately | No yield from merely repeating an input or re-mining filled volume. |
| Player-filled soil/stone | Dig allowed with bounded material return | Reclaim only its remaining deposited material budget, not new ore. |
| Landmark foundation/support envelope | Critical authored structure itself and essential base protected | Clear material/interaction feedback; narrow envelope only, approaches remain editable. |
| Quest device, claim assembly, chest/NPC | Not terrain; explicit interaction/destructibility rule | Cannot mine a quest object by treating it as generic voxel material. |
| Boss arena structural platform/control access | Fixed support/clearance envelope protected during authored encounter | Ordinary peripheral terrain editable outside it; no world-wide boss no-dig rule. |
| Player camp supports | Reject support-removing edit in first baseline | Prevent accidental floating/destruction without inventing a structural physics solver. |
| Inside an occupied actor envelope | Filling/building rejected | Cannot entomb the player/dragon/NPC or push them through walls. |
| Lake/river water volume | Not mineable; bed rules below | Water shader is not a resource solid or full fluid simulation. |
| Safe arrival/critical interaction working space | Reject obstructing placement, small documented envelope | Keep interactable reachable; no invisible broad region wall. |

Protected authored structures should be legible through material, ownership, support shape and a precise message. Log their dimensions and purpose in Region 01. Terrain next to them stays useful for alternative approaches. Story cannot assume a particular tunnel remains blocked after approved digging.

## 4. Edit transaction and resources

Validate requested shape/extent, tool, reach, allowed material, budgets and protected intersections. Provider accepts/rejects an operation ID. Await its supported completion and collision readiness. Then commit resource depletion and reward availability consistently, record world revision, invalidate affected paths, and publish presentation.

Each deposit has a stable ID, resource type, initial/remaining yield and bounded extraction units. A successful edit intersects eligible still-unharvested deposit portions; it consumes finite units once. Exact volume accounting depends on provider capability; a discrete deposit-cell/portion ledger is acceptable when a trustworthy removed-volume query is unavailable. Never infer reward quantity solely from elapsed tool animation. Provider-specific limitations such as those noted in R5 must be tested.

Ordinary terrain is not infinite saleable ore. For fill/add, consume a measured construction material budget corresponding to the accepted operation; rejected/partial operations refund only their uncommitted portion. Reclaiming player-filled ground can return at most the unclaimed deposited budget. Depositing over ore, removing a building and editing the same cell must not recreate original resource yields.

Full inventory creates a persistent output bundle or leaves accepted loot available through the same inventory owner. Geometry/depletion may already be committed; the reward must remain represented by one transaction/instance set rather than rerunning the edit. Save the resulting ledger and provider payload in one complete generation.

Initial slice ore does not respawn. Ordinary enemies may respawn by a separate encounter rule. A new session, region travel, chapter resolution or camp rest is not permission to refill deposits or rerun harvested foliage placement.

## 5. Collision and navigation readiness

An edit is not gameplay-ready until the required collision change is usable. Never let the character step into a hole whose old collision remains invisibly in place while the UI reports a completed tunnel. Where asynchronous readiness is visible, show a short pending state and block only the affected action/route rather than freezing the entire region unnecessarily.

Rebuild/invalidate affected nav areas using a supported runtime solution. Dynamic Modifiers Only does not prove new-surface generation [R4]. Existing paths crossing removed ground become invalid; filled passages cannot retain routes through solid material. AI and optional click-to-move use the same world revision/geometry truth. A blocked path stops with feedback; no teleport fallback through a wall.

Targeting distinguishes stacked cave floors using ray hit and reachability. A top-down click on a ceiling must not route the player to the roof while showing a floor marker below it. Form growth and vehicle exit use swept body-volume checks, not only a point trace to a destination.

## 6. Bounded water baseline

One water owner defines authored lake/river/coast volumes and their surface/wet/dry queries. The first region does not simulate conservation, river rerouting or pressure-driven cave flooding after every voxel edit. State this limitation in the prototype and do not imply that Waterline Pro 6's visual adoption solves it.

Within an existing water volume, excavating the bed makes additional space wet according to that volume's defined vertical extent. No new water spreads beyond the authored bounds. Dry quarry/compact-cave areas are placed outside the bounds with deliberate separation; natural presentation must not show a floating lake above a newly opened dry void. Reject edits that would expose an invalid boundary edge in the first controlled water area, with a narrow documented bed/separation envelope.

Swimming and underwater presentation consult the same wet query. Fishing casts validate approved water/catch area and sensible line/rod conditions; do not fish invisible water after an incompatible edit. Actor recovery finds a safe bank/landing point after illegal underwater arrival. No thirst, cold-survival bar or required underwater breathing management is introduced; deeper swimming/hazard limits are a separate declared design decision.

The proof must include a bed edit inside the allowed wet volume, a rejected boundary-breaking edit, underwater transition and a nearby dry cave. A cosmetic surface plane with no gameplay water state does not pass.

## 7. Construction support and obstruction

One building owner handles support/snapping. Initial camp pieces require validated foundation support and contact tolerance. Reject placement in occupied volume, no-build landmark envelope, invalid support, unsafe slope or blocking essential interaction/recovery points. Placement preview names the reason and spends nothing.

Digging that would remove required support from a player structure is rejected in the first baseline. The player can demolish/reposition the affected structure first, recovering contents through normal transactions. This is explicitly a prototype policy; do not claim realistic collapse or import the structural-stability reference repository without demonstrated need.

Noncritical paths may be blocked by player buildings. Click movement/AI adapts; the player can remove their own obstruction. Critical interaction working space remains protected locally. Do not silently demolish or relocate player-built storage on a quest completion, nav rebuild or save recovery.

## 8. Quest and extreme-route behavior

Reaching the quarry early records location discovery and current evidence. Rescuing Sella before Pell/Iven records Sella's actual rescue, not all three. Defeating Verdance before optional evidence is collected records the valid boss outcome, then exposes the same claim-break interaction. Optional relief/worker follow-ups remain truthful. The [Region 01](../REGION_01.md) fact graph owns exact completion predicates.

If the player excavates around a patrol, do not spawn the same patrol invisibly in the new tunnel simply to force the original fight. If a boss arena is reached from above, initialize its encounter from a legal position and current facts. A protected structure should not be erased by an unapproved mining exploit; when the legal route changes, dialogue and objectives derive from facts rather than “walked through trigger 3.”

On reload, never regenerate missing ground merely to place a dragon/car. Validate its appropriate body and recover to a same-side safe anchor with state preserved. Growth, teleport Echoes and vehicle exits all obey these same clearance/boundary queries.

## 9. Proof and limits

G1 must test repeated dig/add, finite resource payout, fill/re-mine exploit resistance, collision, new tunnel navigation in both cameras, save/reload, water adjacency, real materials and a representative streaming extent. Record edit latency, nav readiness, memory and save size/time on declared hardware; do not invent an FPS guarantee from design targets.

The reference hardware baseline is to be recorded from the actual test machine. The user's known RTX 3060 12GB is a planning target, not measured evidence in this session. Region size/streaming distance/performance budgets follow measurement before production lock.
