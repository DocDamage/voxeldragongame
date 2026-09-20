# Decisions, Scope, and Source Precedence

**Documentation release:** v0.1 · September 14, 2026  
**Status:** Detailed design proposal; not an implementation report  
**Basis:** H01; B03; A01; see [source register](./SOURCES_AND_EVIDENCE.md).  
**Rule:** Existing requirements remain binding. New numbers and detailed behavior are draft baselines for a bounded prototype, not claims of user approval or runtime validation.

## 1. Authority, without accidental approvals

The latest explicit user instruction takes precedence. H01 supplies approved product and development constraints. B03 is the current consolidated story/gameplay draft and supersedes the contradictory town-exclusion, decoration-only vehicle, and old horror-reserve wording in B02/C02. A01 remains the prior architecture proposal; this pack supplies its next detailed pass. C01 supplies the creator plan, reconciled with H01's later one-owner rules.

Within this pack, the named owner document in the index controls a particular rule. The story bible controls narrative identity; it does not certify a plugin. A test record controls whether a behavior passed; it does not change product scope. Do not resolve conflicts by whichever file happens to be newest on disk.

**REQUIRED:** existing direction. **BASELINE:** a concrete new draft to evaluate in a bounded prototype. **VERIFY:** factual information to obtain from actual content. **OPEN:** production decision not silently made here. Unnamed source characters and the Mecha Dragon's personal name remain unnamed.

## 2. Requirement register

| ID | Direction | Basis | Owner |
|---|---|---|---|
| REQ-01 | One humanoid, shared third-person/top-down combat; direct movement and optional click-to-move | H01 §§2–3 | UI_UX_CONTROLS |
| REQ-02 | Smooth terrain, digging, tunneling, filling and persistent edits | H01 §§5–7 | EDITABLE_WORLD_RULES |
| REQ-03 | Finite generated regions with reliable authored landmarks | H01 §4; B03 Chapter 01 | REGION_01 |
| REQ-04 | Runtime Mutable creator, recipe saves, no parallel creator | H01 §§8–9; C01 | MUTABLE_CHARACTER_INTEGRATION |
| REQ-05 | Real supplied production art and approved animation sourcing | H01 §§5,9,18–20 | ASSET_REGISTRY |
| REQ-06 | One GAS attribute/ability/damage owner | H01 §12 | COMBAT_AND_NIGHTMARE_ECHOES |
| REQ-07 | Diablo-style loot, one inventory/equipment authority, stable rolls | H01 §§10–11 | GAME_DESIGN |
| REQ-08 | Useful optional fishing/cooking/gathering/crafting and robust camps | H01 §§13,15 | ACTIVITIES_AND_BUILDING |
| REQ-09 | No survival upkeep, dragon-care or compulsory activity loop | H01 §14; B03 | GAME_DESIGN |
| REQ-10 | Real dragon boss, defeated alive, claim ended, voluntary bond | H01 §16; B03 | DRAGON_HEARTFOLD_SPEC |
| REQ-11 | Companion AI and actual direct dragon control; original body preserved | H01 §16; B03 | SAVE_AND_CONTROL_CONTRACT |
| REQ-12 | Real riding/flight where supported; a cutscene is not a pass | H01 §16; B03 | DRAGON_HEARTFOLD_SPEC |
| REQ-13 | Pet-sized dragons in towns/dungeons, useful compact combat and safe forms | User Heartfold request; B03 | DRAGON_HEARTFOLD_SPEC |
| REQ-14 | Guaranteed permanent usable horror powers, integrated into normal loadout | User power request; B03 | COMBAT_AND_NIGHTMARE_ECHOES |
| REQ-15 | Actual horror stories; authored mercy parity; no random fishing stalker | B03 | REGION_01 / CAMPAIGN_PROGRESSION |
| REQ-16 | Pilotable Zenith flying car, separate from ambient traffic | B03 | FLYING_CARS_AND_COLONY |
| REQ-17 | Playable colony, preferred dragon-flight transition, no assumed seamless orbit | B03 | FLYING_CARS_AND_COLONY |
| REQ-18 | Coherent saves and stable identities across control, form, equipment and travel | A01; B03 | SAVE_AND_CONTROL_CONTRACT |
| REQ-19 | Mouse/keyboard/gamepad and readable context-specific interfaces | H01; C01 | UI_UX_CONTROLS |
| REQ-20 | Single-agent-first, need-based plugins, smallest useful tests, no unauthorized Git | H01 §§21–29 | AGENTS |
| REQ-21 | Outcome-driven Chapter 01, late rescue allowed, persistent bounded aftermath | B03 Chapter 01 | REGION_01 |
| REQ-22 | Preserve ten regions, rulers, dragons, First Binder and eventual Judge payoff | B03 | CAMPAIGN_PROGRESSION |

This is a traceability list, not twenty-two new implementations. B03's drafted dialogue and named power details remain revisable narrative/design material.

## 3. Concrete draft decisions added in this pack

| ID | Proposed baseline | Why / reconsideration trigger |
|---|---|---|
| D-01 | Classless starting loadouts, one melee proof then ranged; four active slots as an initial UI capacity | Allows Echo/weapon combinations; change after real controller/build tests. |
| D-02 | Health + Focus, no extra survival or combat meter initially; formulas in combat spec | Keeps first tuning understandable; add a defensive resource only for a demonstrated combat need. |
| D-03 | Fixed-slot bag, stackable resources, reserved quest records, no weight | Existing inventory candidate must support this; capacity remains data-driven. |
| D-04 | Local protected structural envelopes only; ordinary soil/routes editable | Prevents device/worker softlocks without locking whole regions. This restriction needs review before production. |
| D-05 | Bounded non-flowing authored water volumes for the first region; explicit wet/dry edits | Proves swimming/fishing without claiming a fluid solver. Validate player comprehension and provider behavior. |
| D-06 | No ore respawn in the slice; ordinary encounter resets separated from resource state | Makes duplication testing decisive. Full-game replenishment policy remains open. |
| D-07 | Reject edits that remove required supports in the first camp proof; no second structural solver | Safe bounded starting behavior; review against robust building goals. |
| D-08 | Remote dragon radius 150 m, warning at 120 m, same-region only, damage returns control | Concrete proof baseline; mounted travel is not tethered to an abandoned body. |
| D-09 | Grounded Heartfold transitions, one second, shared four-second recovery, interruptible | Avoids an instant invulnerability exploit while proving the actual forms. |
| D-10 | Boss-to-bond stat conversion is one-way; explicit once-only bond recovery to 50% allied maximum | Supports reclamation combat; never tied to folding or possession. |
| D-11 | No permanent companion death; defeated dragon can recover at a safe hub without consumable grind | Recovery is an explicit event, not a hidden spawn reset. |
| D-12 | Humanoid defeat returns to checkpoint with explicit recovery; no item/XP deletion | Boss retries reset only unresolved encounter state, not world edits or unlocked content. |
| D-13 | Remaining gameplay-time durations persist; the closed game does not tick them | Stops reload exploits and offline maintenance. |
| D-14 | One unarmed hovercar first, assisted hover flight, no fuel upkeep | Vehicle combat/races/construction are not implied by piloting. |
| D-15 | Compact dragon rides as the same actor on a validated passenger anchor; safe staging if no fit | Never leave a duplicate following the car. Missing passenger support remains visible. |
| D-16 | First vehicle operated within Zenith; cross-region availability is deferred, not permanently banned | Region compatibility needs actual geometry and travel decisions. |
| D-17 | Colony is a separate destination using an authored transition and return route | Preserves preferred dragon arrival without requiring a spaceflight simulator. |
| D-18 | Epilogue preserves an explicit pre-departure adventure save branch | Does not delete the character for choosing home; final UI choice remains a production decision. |
| D-19 | Prototype numeric targets live in one canonical section per system, not repeated balance tables | Change data and affected tests together; never present initial values as measured balance. |
| D-20 | WP-23 is the rest-of-world umbrella, split into readiness, ten regional closure/production children, continuity integration, finale, and continued-play/release verification | September 19, 2026 user direction; prevents a single unbounded finale task or parallel region build. Child work remains asset- and evidence-gated. |
| D-21 | Cross-region state extends one bounded travel owner and `UWyrmSaveSubsystem`; Schema 5 uses region-keyed terrain/camp records and accepts Schemas 1–4 | WP-23.2 real-PIE evidence; prevents parallel travel/save authorities while later regions are added. |
| D-22 | Verdant Reach remains an extension of `L_Region01`; Meridess survives both closure routes, the canopy hunter/Hunter's Veil is optional, and Schema 6 adds only Hunter's Veil state while accepting Schemas 1–5 | WP-23.1 VR-01..08 real-PIE evidence; preserves Verdance identity, regional/save ownership, and optional-Echo progression. |
| D-23 | Nyxaroth/Dark Dragon uses its own validated profile; Cathedral/cemetery candidates and the required modular horror roster are retained, with original WYRMFALL pun-name aliases for production-facing identities | WP-23.5 NYX-01..05 validates the dragon. The editor fixture validates four Cathedral roles plus Michael Mire; `L_GloamingMarches` validates the supplied foundation, Arrival-to-Ashgrave route, Count Malvaine parley/living submission, distinct Morrow/Mourn compassionate-release or paired GAS-submission encounter, optional Sanguine Strike/Second Turn, two-way travel/save recovery, Michael Mire as a 17-part 190 cm GAS living-submission encounter, Machete Mason as a 17-part 188 cm weapon-bearing GAS disarmed-submission encounter, and Pleatherface as a 17-part 192 cm sword-bearing GAS disarmed-submission encounter. All three one-time receipts restore through Schema 7 without granting a new Echo or completion. The remaining roster and completion stay gated. |
| D-24 | Schema 7 extends the sole save coordinator with Gloaming facts/receipts plus Sanguine Strike and Second Turn cooldowns; the bounded travel owner adds only Region01↔GloamingMarches and rejects unsafe preflight state | WP-23.5 focused native and live-PIE travel/recovery evidence; Schemas 1–6 remain readable and repeated recovery restores one stable Nyxaroth identity. |

These decisions are sufficiently specified to test. They are not disguised approvals of limits the user never selected.

## 4. Scope tiers

**Shared proofs:** G0 readiness, G1 editable world, G2 Mutable, G3 ARPG/save, G4 dragon control, G4-H forms, E1 reward system/first hunt, G5 connected Chapter 01. Development must prove optional activities and the optional cave/Echo branch, but the player may omit them and still complete the main chapter.

**Authorized rest-of-world target:** WP-23 covers remaining dragon regions and both forms; remaining horror stories/Echoes; the playable Zenith city/colony connection; cross-region continuity; the First Binder/Judge finale; and continued play. It is split into bounded child packets, executed one eligible region at a time, and must reuse the shared state/control/save contracts. Authorization does not clear missing real-content gates.

**Not implicitly in scope:** multiplayer, infinite world, automatic global terrain regeneration, ten active dragons, hunger/sanity/fuel chores, universal destructibility of every authored mesh, seamless orbital physics, vehicle construction, aerial dogfighting, a separate farming economy, player-run settlement administration, procedural personalized horror, arbitrary skeletal compatibility, or replacement art production.

## 5. Facts that cannot be supplied by this writing task

VERIFY actual `.uproject` location, repository/default branch, installed engine patch, plugin builds, asset paths, skeletons, animation clips, legal distribution provenance, cooked-build behavior, and hardware measurements. A high-level ownership claim is not a discovered file. The registry contains requirements and source names with honest statuses, not fabricated content paths.

OPEN release date, total campaign duration, final level cap, final numerical balance, names for unnamed characters, exact regional traversal order, expanded building support, and final performance budgets. Full-world scope is assigned to WP-23; the delivery sequence and asset readiness of its children remain evidence-driven.

## 6. Change rule

A changed requirement needs a dated rationale and user direction. A changed BASELINE needs a reason and affected contracts/tests. A found asset updates its evidence fields. A passing test updates only its actual result. Keep the index and current handoff current; do not create another competing master bible for ordinary corrections.
