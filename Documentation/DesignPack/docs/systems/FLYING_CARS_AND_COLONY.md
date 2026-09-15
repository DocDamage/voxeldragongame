# Flying Cars, Pet Travel and the Orbital Colony

**Documentation release:** v0.1 · September 14, 2026  
**Status:** Detailed design proposal; not an implementation report  
**Basis:** W01 Zenith/Mecha directions as retained in B03; B03 vehicles/colony; A01 shared ownership; see [source register](../SOURCES_AND_EVIDENCE.md).  
**Rule:** Existing requirements remain binding. New numbers and detailed behavior are draft baselines for a bounded prototype, not claims of user approval or runtime validation.

## 1. Preserve two separate promises

Zenith Spire contains visible flying traffic **and a genuinely pilotable player vehicle**. The orbital colony is a real playable destination, not only a sky prop. B03 restores both as full-world targets. Dragon-flight access through an authored transition is preferred; neither hovercar flight nor dragon flight automatically means seamless space simulation.

V1 proves the first civilian hovercar. Z1 proves actual city-to-colony travel, playable destination and return; the Broodmother dungeon/Echo then requires its own authored content. Neither feature is required to place a car in Chapter 01. Their shared control/save interfaces are specified now so later implementation does not replace core systems.

## 2. First vehicle design

One unarmed civilian hovercar, authorized ownership/use through a Zenith quest, assisted hover handling, one original humanoid driver, and one compact active dragon passenger where the real asset supports it. No fuel meter, battery-feeding chore, car-building economy, races or aerial dogfight system is implicitly added.

Actual car model, occupied interior/seat geometry, entry presentation, collision and animation assets are NOT INSPECTED. A floating mesh on a spline is not pilotable proof. Existing owned vehicle functionality may be reused if it satisfies these contracts without a duplicate inventory/save/interaction stack.

| Movement parameter | BASELINE first tuning |
|---|---|
| Cruise speed | 15m/s |
| Horizontal acceleration / braking | 6m/s² / 12m/s² |
| Vertical speed | 6m/s maximum |
| Low-speed landing mode | 3m/s horizontal, 2m/s vertical limits |
| Pilot inputs | Forward/back, lateral steering/strafe, turn/look, ascend, descend, brake/hover, interact/exit |
| Roll/pitch presentation | Assisted bank/tilt within tested comfort bounds; no unrestricted aircraft simulator demand |

Character/dragon inventory does not move into the car as copied records. First car has no cargo trunk unless a real inventory-container feature is separately integrated. Visible seats are not arbitrary new party slots.

## 3. State and action flow

States: Parked → Entering → OccupiedHover → Flying → Landing → Landed → Exiting; Disabled and Recovering are explicit interruption states. Transitions use the shared coordinator rather than a car-only possession architecture.

Enter validates permission, speed, ground/entry clearance, original humanoid form, no remote dragon/mount transition and a valid driver seat. Attach the **same** humanoid, preserving recipe/equipment visuals appropriately. Suspend its ordinary combat input; bind the vehicle context and relevant HUD. Any seat/pet staging failure rolls back to the original humanoid state without duplication.

Piloting is continuous player movement with acceleration/braking, real collision, altitude control and a usable chase/elevated view. Autopilot or a spline taxi can be separate optional transport, not a substitute. Both camera preferences persist across entering/exiting with vehicle-specific tuning.

Landing validates supported surface/pad, slope, full body clearance, occupants and safe exit candidates. Show valid/invalid touchdown information. Exit requires landed/stopped state and a clear supported humanoid envelope at an actual door/exit location. A blocked door or drop is rejected; remain safely occupied and explain it. An emergency recovery command uses a disclosed safe-depot procedure, not ordinary teleport exit anywhere.

## 4. Collision and disablement

The vehicle's real body/swept movement must not tunnel through towers or player structures at cruise speed. Ambient traffic and vehicle blockers are considered in collision/perception as appropriate. The first pilotable car need not simulate each cosmetic traffic vehicle physically, but visually intersecting lethal traffic cannot be ignored while claiming correct collision.

BASELINE car health is 250 for the isolated V1 fixture. Accepted heavy collision/environment damage can disable propulsion; ordinary gentle landings do not. Damage thresholds/impulse tuning require actual physics testing and are not settled by this HP value. No automatic explosive death or ejecting original humanoid through geometry.

On disablement, attempt a controlled descent only to a validated reachable surface. If no safe landing exists, pause the failure transition and offer explicit recovery to the last valid depot/landing anchor. Preserve car ID/ownership/damage and driver/pet identities. Depot recovery repairs through a visible bounded interaction, with no consumable/fuel grind in the first baseline. It is not a free second car alongside the wreck.

Hostile vehicle combat, theft/crime/police systems, crash ragdolls and vehicle weapon upgrades are later proposals only. Do not create them to explain an unarmed transport feature.

## 5. Heartfold passenger contract

Before boarding, the active dragon must be compact and safely grounded; normal transition rules apply. A real validated passenger anchor has enough visual/collision space, predictable idle presentation and a safe exit location. Attach the same dragon instance; suspend companion movement/attacks while aboard, retain health/effects/cooldowns, and show passenger status.

If the vehicle cannot fit that dragon's compact anatomy, the first baseline offers explicit safe staging at the boarding location. Confirm the actual dragon is safe there and show its map/status; no hidden disappearance, duplicate flying follower or magical pocket-storage interpretation. If safe staging cannot be established, block boarding with a clear reason until the situation is resolved.

On normal exit, release driver first to a valid spot, then place the same pet at its own valid position and resume AI. Partial exit cannot create two dragon instances. Disabled vehicle/region travel/reload uses the same staged/attached record. Test the smallest compact dragon and a longer-bodied variant later before generalizing passenger support.

## 6. Acquisition and Mecha Dragon role

A city quest can authorize the first car before the Mecha Dragon's regional resolution. Ownership is a stable record with a persistent parked location; borrowing uses a permission record, not silently converting every traffic car into player property.

The Mecha bond can unlock selected Wyrm-circuit upgrades, restricted infrastructure or advanced routes. Ordinary city cars must not all require the dragon to remain enslaved or crash when it is freed. The city remains technologically real; restoring freedom ends coercion without destroying necessary public systems.

First vehicle implementation is confined to a playable Zenith test extent. Cross-region permissions, transport through regional gates and eventual inventory cargo remain OPEN. Do not convert that implementation boundary into a permanent lore ban. A car is not automatically permitted into a narrow dungeon, and shrinking dragons does not imply shrinking vehicles.

## 7. Ambient traffic boundary

Ambient traffic can use authored route lanes and distant lightweight presentation. It needs sensible intersections with the playable space, density limits based on measured performance, no blocking of mandatory landing pads and deterministic enough staging for travel. It is not another autonomous world economy or a requirement to simulate every citizen's commute.

Separate traffic actors from the owned car record. A cosmetic car despawning at distance cannot delete the player's vehicle or driver. Conversely, owning one car should not force the entire distant skyline to load physics actors.

## 8. Colony travel contract

The preferred authored route is a suitably validated dragon flight to a designated transfer boundary. The game explicitly transitions to the colony destination; it does not pretend the car/dragon flew through a simulated atmosphere and orbit. Flight animation and control at each end must use real assets. If a required dragon rig cannot support the intended presentation, record the blocker; any alternate transit method requires an explicit design decision, not silent removal of dragon access.

Prerequisites are a known colony route/access event and a travel-capable dragon or explicitly approved transport. Do not require an optional horror Echo as the only means of reaching the colony. Before transfer, park/stage the city car and record its real location. The original humanoid and selected dragon transfer once with existing inventory, recipe, state and IDs.

Arrival has valid landing/dismount and compact companion access. The first playable colony segment uses normal gravity unless a separate zero-gravity mechanic is approved and proved. Include a recognizable arrival hub, at least one usable interior route/encounter objective, destination interaction and return transport. A distant platform screenshot or corridor inaccessible to the player does not pass Z1.

Broodmother Cipher's infestation dungeon involves investigate missing crew → reveal parasitic replacements → actual confrontation → one-time Brood Seed. Do not implement an unrestricted NPC-replacement simulation; authored changes suffice. Arrival/return must remain available after encounter completion and while choosing not to attempt it.

## 9. Persistence, recovery and tests

Store car ID/definition/permission, region/transform, damage, driver and pet/staging relations, last safe depot and travel transaction ID. Parked vehicles stay parked when the player visits the colony. Source→destination transfer publishes one commit; failed transfer restores source authority, completed retry resolves the same arrival.

V1 checks enter/pilot/ascend/descend/brake/real collision/land/exit, blocked exits, disability, pet boarding/staging and save/reload occupied and parked. Z1 checks playable destination, original identity/gear/dragon persistence, parked car not duplicated, failed travel recovery and return path. See VEH/COL procedures. All are NOT RUN; the specification is not a working vehicle build.
