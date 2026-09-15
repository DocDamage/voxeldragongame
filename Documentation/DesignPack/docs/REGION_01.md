# Region 01 — The First Unbound: Level and Quest Specification

**Documentation release:** v0.1 · September 14, 2026  
**Status:** Detailed design proposal; not an implementation report  
**Basis:** B03 complete Chapter 01 and acceptance notes; H01; A01; see [source register](./SOURCES_AND_EVIDENCE.md).  
**Rule:** Existing requirements remain binding. New numbers and detailed behavior are draft baselines for a bounded prototype, not claims of user approval or runtime validation.

## 1. Identity and scope

**Region:** Verdant Wilds, the mainland opening within the wider Verdant Reach. **Hub:** Tidecross. **Major site:** Crowncut Quarry and its buried underworks. **Dragon:** Verdance. **Ruler:** Queen Meridess Tidecaller remains in power beyond this local chapter.

**Immediate objective:** find Sella, Pell and Iven and end the quarry danger. **Personal objective:** understand the warm heart and whether the dragon can help the outsider return home. The local resolution is a rescue, stopped exploitation and willing bond—not conquest of Meridess's realm.

B03 contains the full current prose/dialogue treatment. This document specifies a playable layout, facts, encounters and alternate order. Proposed dimensions/rewards/branch staging below are implementation baselines. Existing names, motivations and key consent moments are retained.

## 2. Landmark graph and generation constraints

```text
Heart-chamber → daylight/Tamsin → Tidecross ← safe fishing + optional camp
                                      │
                               quarry approach
                               /             \
                  survivors' cutting      editable flank
                               \             /
                            underworks / Sella
                                      │
                             Rusk / claim works
                                      │
                             Verdance chamber
                                      │
                           ally terrace / overlook
                                      │
                         safe town-entry buffer → hub

Post-bond optional: compact service cave; separate Silent Landing horror site.
Onward: Hallowwood correspondence/travel lead, not a fixed full campaign order.
```

All edges are logical connections, not mandatory corridor triggers. A new seed can move the landmarks while preserving arrival, humanoid passage and grounded return. Proposed first region footprint is a compact 1km-class test extent; exact scale follows provider/hardware measurement. Do not claim a final map size or playable-minute promise from this sketch.

| Landmark ID | Required function / contents | Geometry and state constraint |
|---|---|---|
| LM-HEART | Created character arrival, warm fossil material, real dig obstruction | Valid spawn/exit, narrow protected heart structure only; obstruction edits persist. |
| LM-TAMSIN | Broken cart, local dialogue, first threat | Reachable from exit; two-camera readability; no long forced escort. |
| LM-TIDECROSS | Mara/services, notice/ledger, workers' homecoming | Safe compact-dragon corridors/interiors and visible interaction space; local no-build protection. |
| LM-QUIETWATER | Ordinary fishing and optional cooking lead | Real authored water, safe approach, physically distinct from Counselor site. |
| LM-CAMP | Optional supported clearing | Outside quest/protected envelopes; terrain edits, floor/wall/door/storage/work point usable. |
| LM-CUTTING | Pell/Iven rescue behind collapse | Diggable access, valid survivor staging; not a randomized long escort route. |
| LM-SELLA | Sella, repair records and auxiliary machinery | Safe interaction pocket with alternate access; no required profession minigame. |
| LM-CONTROL | Rusk, records and claim assembly | Device/support envelope protected narrowly; editable flanking approach remains. |
| LM-ARENA | True-form dragon and binding hazard | Supported floor/claim access; safe encounter initialization from legal alternate entries. |
| LM-TERRACE | Relief encounter; separate calm control tutorial | Combat area then quiet demonstration space; valid humanoid/dragon return anchor. |
| LM-TOWNENTRY | Safe mount/Heartfold transition | Room for true dragon landing/dismount; no forced airborne shrinking. |
| LM-COMPACTCAVE | Optional service cache and compact play | Pet-size actual clearance, safe waiting body; wider growth test chamber only if envelope fits. |
| LM-SILENTLANDING | Optional Counselor hunt and Echo demonstration | Disused lake site, walkable combat/retreat route, no compulsory helpless swim. |

Protected objects are limited to authored structures/essential working volumes, not the full quarry hillside or whole forest. Mark exact extents in the level data and test adjacent digging. Water/bed and building-support policies come from the shared editable-world spec, not a special “story magic” workaround.

## 3. Cast and services

Tamsin is a ranger who recommended quarry employment and wants the workers returned; she knows local routes, not the whole cosmology. Mara keeps supplies and asks what people will live on when extraction ends. Sella understands the machinery from maintaining it, not from omniscient lore. Pell and Iven are recognizable named workers with lightweight interaction needs. Rusk knowingly sealed people below and defends it as public safety; he is not a rogue excuse that absolves the queen.

Tamsin and worker relocations use authored safe staging after encounters. No permanent companion-follow AI is needed for them. Inventory/crafting services can be offered by these named NPCs or a simple real-content station; do not add dialogue-heavy NPCs just to fill menu categories. Rusk's nonlethal resolution needs a disarmed/custody state, not a corpse renamed as a prisoner.

## 4. Persistent facts and predicates

Use booleans/enumerated outcomes with stable names and unique event receipts, not a single quest-step integer. Facts below are design IDs; authoring can use the existing project's equivalent naming.

| Fact | Set by actual event | Never inferred merely from |
|---|---|---|
| `heart.exit_reached` | Player reaches valid outside space | Character creation completed |
| `tidecross.visited` / `notice.read` | Actual visit/read interaction | Walking near a distant trigger |
| `worker.pell.secured`, `worker.iven.secured`, `worker.sella.secured` | Individual rescue and valid safe staging | One rescue, boss defeat or diary discovery |
| `evidence.machine_seen` / `evidence.records` / `evidence.sella_account` | Machine event, read record, witness conversation | Quest acceptance alone |
| `quarry.aux_disabled` | Auxiliary shutdown committed | Sella found but controls untouched |
| `rusk.outcome` | `unresolved`, `defeated_custody`, or proposed `surrendered_custody` | Player bypassed his position |
| `quarry.extraction_stopped` | Control shutdown or primary claim destruction | Merely disabling one auxiliary device |
| `verdance.defeated_alive` | Genuine encounter terminal outcome | Entering the arena or reading the truth |
| `verdance.claim_broken` | Release interaction after valid defeat | Selecting a dialogue tone |
| `verdance.bond_accepted` | Voluntary bond interaction outcome | Dragon HP reaches zero |
| `dragon.heartfold_available` | Bond committed | A tutorial popup viewed |
| `relief.resolved` | Actual allied encounter or authored safe resolution | A cinematic cut |
| `homecoming.complete` | All workers secured + extraction stopped + bond + local Rusk/relief resolution | First arrival in town with a pet |
| `compactcave.complete` | Cache/service objective reached | Dragon folded once |
| `counselor.resolved` / `echo.relentless_advance` | Valid hunt outcome and reward transaction | Player saw the distant silhouette |
| `lead.hallowwood_known` | Recovered correspondence or factual debrief | Final fixed campaign order |

Main bond readiness depends on real dragon defeat and claim-breaking, **not every optional clue, worker or Rusk conversation**. The full homecoming waits for all-three rescue; the player can use Verdance, enter town and pursue remaining rescue tasks before that celebration. No hidden worker-death timer punishes fishing/exploration.

## 5. Main quest packages

| Quest | Activation and objective | Completion / reward purpose |
|---|---|---|
| Q01 Warm Stone | New character in heart chamber; clear real blockage and escape | Exit fact, early excavation learned through play; ordinary finite resource pickup. |
| Q02 Three Names | Tamsin, notice, survivor discovery or related evidence can establish the search | Each worker secured independently; one rescue reward transaction per authored package. |
| Q03 Beneath Root and Crown | Discover quarry/underworks; find exploitation evidence and safe control route | Stop extraction; optional auxiliary improvement; records retained even if NPC moved. |
| Q04 The Cost of Safety | Encounter Rusk or later return to unresolved control works | Rusk in actual custody, records available; optional flanking route not forced main-door fight. |
| Q05 Verdance, Bound | Legal arena entry starts genuine boss from current apparatus facts | DefeatedAlive; primary claim becomes accessible. No corpse/automatic taming. |
| Q06 No More Chains | Valid defeated dragon; player chooses Break the Claim | Claim broken → freely offered bond accepted; one dragon unlock/role conversion/recovery receipt. |
| Q07 What Freedom Can Do | Bond committed | Allied relief encounter, real control demonstration, supported mount/flight proof; tutorial flags separate from story lock. |
| Q08 Three Names Returned | All homecoming predicate facts true | Actual workers gather, small Verdance accompanies player, Warden recognition and Hallowwood lead. |

Draft quest rewards: Q02 grants a practical supply/equipment reward once after all three secure; Q03 records one normal rare-eligible loot bundle; Q06 guarantees the dragon and one separate boss loot roll; Q08 recognizes the outcome without granting a second dragon. Numeric currency/item definitions are selected from the actual item catalogue in G3; a missing chosen item is a content blocker, not permission to grant a nonexistent placeholder. Critical rewards never rely on bag capacity.

## 6. Encounters and evidence delivery

EN01 approach: one supported wildlife/creature role introduces readable attack/dodge and real loot. EN02 cutting: a small contrast role guards/occupies the collapse route. EN03 underworks: crown guard group uses room/cover geometry without a new combat stack. EN04 Rusk: elite variation plus limited guards; disarm/custody end state. EN05 Verdance: full authored boss. EN06 relief: a short reused crown group allowing meaningful allied attacks. EN07 compact cave: ordinary enemies with real small-dragon targeting. EN08 Counselor: distinct optional horror boss.

Use three evidence channels: physical damage aimed at machinery, Sella's testimony, and records of extraction after known danger. The chapter remains understandable if one optional note is missed. A brief visual cue at unusual arena entry supplies the necessary machine/dragon conflict without teleporting Rusk in for an out-of-order speech.

All encounter numbers are small content-role targets, not animation promises. Real assets determine exact creature species and attacks; the registry records those selections before assembly.

## 7. Verdance boss authoring

Phase A emphasizes keep-away: front committed strike and supported turn/sweep with visible recovery. The dragon repeatedly looks/strikes toward the apparatus. Phase B activates the remaining binding interference; tell, pulse and compelled response must be visibly causally connected. `quarry.aux_disabled` removes exactly one identifiable hazard source, not the whole fight. Phase C exhaustion creates the terminal opening.

Precise phase thresholds are a BASELINE: first escalation around 65% HP, final pressure around 30%; use health/state triggers once, not per-frame repeated cutscenes. All HP/damage/cooldowns come from the dragon spec. Do not pad phases with unexplained invulnerability. Readable protected machinery may prevent destroying the primary claim early, but the encounter remains damageable and winnable through every valid route.

At terminal defeat, cancel attacks/DOT target acceptance, preserve living actor and expose release. Break the claim stops the primary extraction even when Rusk was bypassed. Bond is a separate willing interaction and shared sensory moment. Save/reload at all three points continues without replaying a full fight or skipping consent.

## 8. Alternate-order handling

| Player route | Required response |
|---|---|
| Finds a worker before Tamsin/notice | Record that actual rescue and remaining names; later dialogue acknowledges it. No “return to start trigger” demand. |
| Finds Sella before Pell/Iven | Her machinery account/auxiliary action becomes available; other two remain rescuable. |
| Digs around road patrol or Rusk | Respect bypass. Evidence/arena introduction uses current facts; no forced teleport/cutscene. |
| Defeats/bonds before all workers | Ally and Heartfold work immediately; rescue objectives remain open; no false three-person homecoming. |
| Bypasses Rusk until after bond | New proposed branch: return to control works; now-disarmed operation supports Rusk's surrender into custody, or an actual short nonlethal encounter if he resists. Record actual branch, never pretend earlier victory. |
| Stops primary extraction before auxiliary | Auxiliary becomes safely inactive; no inaccessible required switch; optional advantage no longer needed. |
| Returns to town early with bonded Verdance | Safe compact entry and partial debrief. Celebration waits for real predicate. |
| Skips controls/tutorial prompts | Learned capabilities persist; story does not require clicking every tutorial. Development evidence still must demonstrate actual features. |
| Skips all activities/Counselor/cave | Main chapter and onward lead remain available. No Echo, fishing or camp gate secretly added. |

Rusk's post-bond custody branch is new staging to make the source's alternate-order requirement concrete. It does not rewrite him as innocent or the queen as unaware. Flag final dialogue polish separately from the mechanics.

## 9. Conditional dialogue and homecoming

| Speaker/state | Line purpose / proposed short text |
|---|---|
| Tamsin first exit | “That path was buried yesterday.” Establish useful stranger, not foretold savior. |
| Mara before rescue | “Presumed lost is what they write when they want the wages to stop before the searching does.” Establish names/accountability. |
| Sella first control | “Stopped, yes. Dumped through this passage, no.” Teach extraction versus safe shutdown. |
| Rusk confronted | “I kept it from reaching three hundred.” Defend coercion without denying the workers. |
| Player before bond | Tone choice such as “I'm not taking their place.” No enslavement campaign option implied. |
| Tamsin partial return | Name actual missing workers; no “Three” line while someone remains below. |
| Tamsin full homecoming | “Three.” / “You already counted.” / “I know.” Only when all secured. |
| Mara afterward | “We still need stone. We still need work.” Upper quarry livelihood can continue without extraction. |
| Warden recognition | Earned through protecting rather than owning; not a coronation or player kingdom system. |

Player-created name/gender/appearance are respected. No fixed Earth trauma/death, forced romance or hidden alignment check. Dialogue uses localization keys and condition priorities; repeated ambient barks never regrant rewards. Unvoiced text is valid if voice assets are absent, but not claimed as recorded voice production.

## 10. Optional content, required development proof

**QO01 Quiet Water:** Mara offers ordinary fishing; catch enters inventory and can be cooked at an accessible station. No Counselor spawn in this safe lesson. **QO02 A Place Above the Road:** a supported clearing with useful floor/enclosure/storage/work point. No mandatory home before rescue. **QO03 Unpaid Shift:** recovered wage/supply record before or after boss; limited restitution, no economic simulation.

**QO04 A Smaller Kind of Strength:** postbond service-cache cave. The player can send/control compact Verdance through a low route, use compact combat and return to the same safely staged humanoid. Cache interaction can unlock a humanoid-accessible service route or simply reveal reachable treasure; it must not require carrying the humanoid through the pet hole. A wide chamber explicitly tests legal growth; growth remains optional to completion.

**QO05 Silent Landing:** available after local quarry/homecoming resolution in the baseline. Distant earlier warning/sighting is optional and nonattacking. At the separate disused landing, investigate signs, deliberately enter the encounter and confront the Counselor on walkable ground. No real-time night waiting or mandatory swimming helplessness.

Counselor fixture has a readable normal attack, a Relentless Advance stance during which damage still works but light stagger/slows do not, and a punishable recovery. Draft HP 600; draft normal strike 14 damage. The ability table owns stance duration/cooldown, keeping enemy demonstration recognizable without handing over arbitrary immortality. Verdance may help in a legal form; no anti-dragon invisible ban. A valid defeat/resolution grants `echo.relentless_advance` once plus ordinary loot. A safe practice target demonstrates resisting a slow/stagger; skipping practice never revokes the power.

## 11. Checkpoints, persistence and closure

Checkpoints: heart exit, Tidecross arrival, secured underworks approach, boss approach, terrace after relief, homecoming. Use actual safe anchors validated against edited ground. Unresolved boss attempts restart according to the save contract; worker facts, apparatus state, claims, bonds, Echoes, terrain, depleted resources and camps do not reset.

Local closure means actual workers home, extraction ended, Rusk/relief resolved, bond earned and usable, and Tidecross reacting truthfully. Hallowwood correspondence and a reference to the Judge provide an onward lead; the First Binder stays unnamed here. No world regeneration, ore refill, camp erasure or forced region conquest.

All content/results are NOT RUN. Region acceptance is the QA `REG` group plus the complete G5 evidence sequence, not merely authored quest text.
