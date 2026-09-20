# Ordered Implementation Backlog — Bounded Work Packages

**Documentation release:** v0.1 · September 14, 2026  
**Status:** Detailed design proposal; not an implementation report  
**Basis:** H01 development/model/gate rules; A01 G0–G5; B03 new feature gates; see [source register](../SOURCES_AND_EVIDENCE.md).  
**Rule:** Existing requirements remain binding. New numbers and detailed behavior are draft baselines for a bounded prototype, not claims of user approval or runtime validation.

## How to use this backlog

Select one eligible task. Inspect the actual project first, use the smallest competent model, and run only the relevant checks. Routing names follow H01 and are not claims that these models are available in every environment. Most tasks use one agent. Dependencies are product/integration ordering, not automatic Git branches or separate architecture stacks.

All tasks originally started **NOT STARTED** because this deliverable was documentation; current statuses are reconciled only from recorded evidence. Status may become IN PROGRESS, BLOCKED, IMPLEMENTED / NOT VERIFIED, or VERIFIED only with real evidence. A missing asset blocks its dependent work, not every unrelated design task. No CI, issues, commits, pushes or production artwork is authorized by this backlog itself.

## Work queue

| ID | Task | Dependency | Suggested routing | Status |
|---|---|---|---|---|
| WP-00 | G0 readiness and actual-owner inspection | None | Luna | NOT STARTED |
| WP-01 | G1 one terrain provider | WP-00 | Sol | NOT STARTED |
| WP-02 | G2 real Mutable recipe and runtime generation | WP-00 | Sol | NOT STARTED |
| WP-03 | Shared control/camera and input foundation | WP-01, WP-02 | Terra | NOT STARTED |
| WP-04 | G3 first real combat loop | WP-03 | Terra | NOT STARTED |
| WP-05 | Loot, equipment, inventory and coherent snapshot | WP-04 | Sol | NOT STARTED |
| WP-06 | Second build and progression fixture | WP-05 | Terra | NOT STARTED |
| WP-07 | Water, fishing, cooking and crafting | WP-05 | Terra | NOT STARTED |
| WP-08 | Supported camp and storage | WP-05, WP-01 | Terra | NOT STARTED |
| WP-09 | G4 Green Dragon locomotion/combat/direct control | WP-05 | Sol | NOT STARTED |
| WP-10 | G4 original rider and flight | WP-09 | Sol | NOT STARTED |
| WP-11 | G4-H Heartfold and compact behavior | WP-09 | Sol | NOT STARTED |
| WP-12 | Region 01 generated landmarks and quest facts | WP-06, WP-01 | Terra | NOT STARTED |
| WP-13 | Verdance authored boss, claim and bond | WP-09, WP-12 | Sol | NOT STARTED |
| WP-14 | Ally terrace, compact homecoming and cave | WP-10, WP-11, WP-13 | Terra | NOT STARTED |
| WP-15 | E1 Counselor and Relentless Advance | WP-06, WP-11, WP-14 | Terra | NOT STARTED |
| WP-16 | G5 connected slice and focused packaged verification | WP-07, WP-08, WP-15 | Sol | NOT STARTED |
| WP-17 | Creator content breadth milestone | WP-02, WP-16 | Terra | NOT STARTED |
| WP-18 | V1 pilotable Zenith hovercar | WP-16; actual car assets from G0 | Sol | NOT STARTED |
| WP-19 | Z1 playable colony travel and return | WP-18; actual city/colony and flight content | Sol | NOT STARTED |
| WP-20 | Next regional dragon and selected Echo content | WP-16; explicit next-region scope | Terra/Sol by integration risk | NOT STARTED |
| WP-21 | Moonbound transformation proof | WP-16; real Corvyn/beast assets | Sol | NOT STARTED |
| WP-22 | Jade Peaks production-region slice | WP-20; supplied Jadefang and palace content | Sol | VERIFIED (JP-01–06 + WP-22-QA1) |
| WP-23 | Rest-of-world production umbrella (WP-23.0–23.13) | WP-22 functional foundation; explicit September 19, 2026 user scope; child-specific real content | Sol | WP-23.0 + WP-23.1 + WP-23.2 COMPLETE / WP-23.5 SCARRIE PASS, COMPLETION GATED |

## Task packets

### WP-00 — G0 readiness and actual-owner inspection

**Dependencies:** None. **Routing:** Luna. **Read:** G0_G1_READINESS_AND_TERRAIN_PROOF.md; asset/plugin records.

**Work:** Locate authorized project, engine, candidate versions, actual content and existing owners. Populate evidence fields, not invented paths.

**Stop / evidence:** RDY-01–04; complete accurate readiness record or specific unavailable input.

**Boundary:** Do not expand into the next work package or unrelated refactors; record missing real inputs and preserve required scope.

### WP-01 — G1 one terrain provider

**Dependencies:** WP-00. **Routing:** Sol. **Read:** G0/G1 assignment; editable-world; save contract.

**Work:** Implement/prove one candidate with real materials, dig/add/cave/collision, finite yield, new-surface navigation and reload.

**Stop / evidence:** WRLD group; applicable SAVE cases; choose one owner or report reproduced blocker.

**Boundary:** Do not expand into the next work package or unrelated refactors; record missing real inputs and preserve required scope.

### WP-02 — G2 real Mutable recipe and runtime generation

**Dependencies:** WP-00. **Routing:** Sol. **Read:** Mutable integration; equipment boundary.

**Work:** One animated real recipe, part/color changes, revision-safe updates, attachment and saved reconstruction; focused cooked check.

**Stop / evidence:** CHAR-01–06; no alternate creator; document exact supported content.

**Boundary:** Do not expand into the next work package or unrelated refactors; record missing real inputs and preserve required scope.

### WP-03 — Shared control/camera and input foundation

**Dependencies:** WP-01, WP-02. **Routing:** Terra. **Read:** Technical architecture; UX; save/control.

**Work:** Same humanoid, both cameras, direct and optional click movement, mode context consumption and camera persistence.

**Stop / evidence:** UI-01–03; WRLD-04–06; no duplicated pawn progression.

**Boundary:** Do not expand into the next work package or unrelated refactors; record missing real inputs and preserve required scope.

### WP-04 — G3 first real combat loop

**Dependencies:** WP-03. **Routing:** Terra. **Read:** Game design; combat/Echo spec.

**Work:** One weapon kit, two real contrasting enemy roles, damage/status and readable attacks; no huge empty skill tree.

**Stop / evidence:** COM-01–05; first actual encounter playable in both cameras.

**Boundary:** Do not expand into the next work package or unrelated refactors; record missing real inputs and preserve required scope.

### WP-05 — Loot, equipment, inventory and coherent snapshot

**Dependencies:** WP-04. **Routing:** Sol. **Read:** Game design; Mutable; save contract.

**Work:** Stable rolled items, transfer/equip/visual commit, overflow and reward receipts; integrate terrain/character data in one save.

**Stop / evidence:** COM-06–08; SAVE-01–07; no duplicate stats/items or rerolls.

**Boundary:** Do not expand into the next work package or unrelated refactors; record missing real inputs and preserve required scope.

### WP-06 — Second build and progression fixture

**Dependencies:** WP-05. **Routing:** Terra. **Read:** Game design; combat rules.

**Work:** Materially different real ranged/melee kit, level fixture and compatible affixes/skills.

**Stop / evidence:** COM-01–08 relevant subset; demonstrate a meaningful gear/build decision.

**Boundary:** Do not expand into the next work package or unrelated refactors; record missing real inputs and preserve required scope.

### WP-07 — Water, fishing, cooking and crafting

**Dependencies:** WP-05. **Routing:** Terra. **Read:** Activities; editable-world; UX.

**Work:** Real wet/dry/swim/rod/catch/cook/use and small recipe transactions; ordinary safe fishing distinct from horror.

**Stop / evidence:** WRLD-10–11; ACT-01–05; full bag/cancel/save cases.

**Boundary:** Do not expand into the next work package or unrelated refactors; record missing real inputs and preserve required scope.

### WP-08 — Supported camp and storage

**Dependencies:** WP-05, WP-01. **Routing:** Terra. **Read:** Activities/building; editable-world.

**Work:** One real supported enclosure/storage/workpoint; placement, rejection, demolition/refund and contents persistence.

**Stop / evidence:** ACT-06–10; WRLD-08–09; no second structural solver.

**Boundary:** Do not expand into the next work package or unrelated refactors; record missing real inputs and preserve required scope.

### WP-09 — G4 Green Dragon locomotion/combat/direct control

**Dependencies:** WP-05. **Routing:** Sol. **Read:** Dragon spec; save/control.

**Work:** Real dragon AI/attacks and input transfer while original humanoid waits with documented risk; separate state ownership.

**Stop / evidence:** DRG-01–04; SAVE-08; no cloned default dragon or body.

**Boundary:** Do not expand into the next work package or unrelated refactors; record missing real inputs and preserve required scope.

### WP-10 — G4 original rider and flight

**Dependencies:** WP-09. **Routing:** Sol. **Read:** Dragon spec; UX; save/control.

**Work:** Real rider fit/mount/takeoff/collision/landing/dismount and both flight views; explicit recovery.

**Stop / evidence:** DRG-05–07, DRG-14; SAVE-09; unsupported rig reported, not replaced by cinematic pass.

**Boundary:** Do not expand into the next work package or unrelated refactors; record missing real inputs and preserve required scope.

### WP-11 — G4-H Heartfold and compact behavior

**Dependencies:** WP-09. **Routing:** Sol. **Read:** Dragon/Heartfold; editable-world; save/control.

**Work:** Pet form, correct clearance/nav/combat/direct control, safe town entry and interruptible valid/blocked transition.

**Stop / evidence:** DRG-08–13, DRG-15; SAVE-08; no reset of health/effects/timers.

**Boundary:** Do not expand into the next work package or unrelated refactors; record missing real inputs and preserve required scope.

### WP-12 — Region 01 generated landmarks and quest facts

**Dependencies:** WP-06, WP-01. **Routing:** Terra. **Read:** Region 01; story bible Chapter 01.

**Work:** Place valid landmark relationships with real content; fact-driven worker rescues, evidence, controls, Rusk and alternate approaches.

**Stop / evidence:** REG-01–05, REG-09–11; no story dependence on one trigger order.

**Boundary:** Do not expand into the next work package or unrelated refactors; record missing real inputs and preserve required scope.

### WP-13 — Verdance authored boss, claim and bond

**Dependencies:** WP-09, WP-12. **Routing:** Sol. **Read:** Region 01; dragon spec; reward/save contract.

**Work:** Real phases, optional auxiliary advantage, living defeat, claim release, consent and once-only ally role conversion.

**Stop / evidence:** REG-06–08; DRG-01; SAVE-10; no corpse/automatic ownership.

**Boundary:** Do not expand into the next work package or unrelated refactors; record missing real inputs and preserve required scope.

### WP-14 — Ally terrace, compact homecoming and cave

**Dependencies:** WP-10, WP-11, WP-13. **Routing:** Terra. **Read:** Region 01; UX; dragon spec.

**Work:** Actual relief fight, control/riding route, safe town shrink, truthful three-worker homecoming, compact optional cave.

**Stop / evidence:** REG-09–12; DRG relevant group; no grounded substitute marked flight verified.

**Boundary:** Do not expand into the next work package or unrelated refactors; record missing real inputs and preserve required scope.

### WP-15 — E1 Counselor and Relentless Advance

**Dependencies:** WP-06, WP-11, WP-14. **Routing:** Terra. **Read:** Region 01 Silent Landing; combat/Echo spec.

**Work:** Real optional horror encounter, demonstrated stance, guaranteed unlock, equip/use/practice and persistence.

**Stop / evidence:** ECHO-01–06; REG-13; standalone ability fixture is not completed encounter.

**Boundary:** Do not expand into the next work package or unrelated refactors; record missing real inputs and preserve required scope.

### WP-16 — G5 connected slice and focused packaged verification

**Dependencies:** WP-07, WP-08, WP-15. **Routing:** Sol. **Read:** Vertical slice; acceptance catalogue; current handoff.

**Work:** Run connected authored, alternate and skip paths with actual content and coherent quit/reload. Fix specific integration failures only.

**Stop / evidence:** All applicable G0–G5/Heartfold/E1 cases; no required blocker or unreported save loss.

**Boundary:** Do not expand into the next work package or unrelated refactors; record missing real inputs and preserve required scope.

### WP-17 — Creator content breadth milestone

**Dependencies:** WP-02, WP-16. **Routing:** Terra. **Read:** C01 preserved targets; Mutable integration.

**Work:** Expand validated categories/UI/randomization and animation coverage without a parallel creator; all original category targets tracked.

**Stop / evidence:** CHAR-07–08 plus prior recipe regressions; counts are real assets, not dummy entries.

**Boundary:** Do not expand into the next work package or unrelated refactors; record missing real inputs and preserve required scope.

### WP-18 — V1 pilotable Zenith hovercar

**Dependencies:** WP-16; actual car assets from G0. **Routing:** Sol. **Read:** Cars/colony; save/control; UX.

**Work:** Real car entry/pilot/collision/landing/exit/damage/pet/staging/ownership save in bounded city test.

**Stop / evidence:** VEH-01–09; not ambient spline traffic or generated substitute car.

**Boundary:** Do not expand into the next work package or unrelated refactors; record missing real inputs and preserve required scope.

### WP-19 — Z1 playable colony travel and return

**Dependencies:** WP-18; actual city/colony and flight content. **Routing:** Sol. **Read:** Cars/colony; campaign; save/control.

**Work:** Author city→colony transfer with same humanoid/dragon and parked car; real interior interaction and return.

**Stop / evidence:** COL-01–05; destination playable before claiming colony content.

**Boundary:** Do not expand into the next work package or unrelated refactors; record missing real inputs and preserve required scope.

### WP-20 — Next regional dragon and selected Echo content

**Dependencies:** WP-16; explicit next-region scope. **Routing:** Terra/Sol by integration risk. **Read:** Campaign map; system specs; chosen region bible.

**Work:** One new region/dragon proof and a bounded local encounter; per-rig compact/control/rider validation and per-power interactions.

**Stop / evidence:** Applicable DRG/ECHO tests; content-specific tests only for new behavior.

**Boundary:** Do not expand into the next work package or unrelated refactors; record missing real inputs and preserve required scope.

### WP-21 — Moonbound transformation proof

**Dependencies:** WP-16; real Corvyn/beast assets. **Routing:** Sol. **Read:** Combat/Echo transformation; Mutable; save/control.

**Work:** Actual beast kit and original-body restoration across interruption/load/equipment, with equivalent cure unlock.

**Stop / evidence:** ECHO-07–09; SAVE-11; glow-only buff is not transformation.

**Boundary:** Do not expand into the next work package or unrelated refactors; record missing real inputs and preserve required scope.

### WP-22 — Jade Peaks production-region slice

**Dependencies:** WP-20; supplied Jadefang and palace content. **Routing:** Sol. **Read:** Campaign map; combat/Echo specification; save/control; WP-22 current task packet.

**Work:** Author one bounded second production region around Jadefang: a playable Jade Peaks route, a palace conflict that preserves the duty/pact theme, one local disciple encounter, a validated Mirror Step interaction, and unified persistence. Reuse the proven Jadefang rig rather than creating another dragon owner.

**Stop / evidence:** JP-01–06; real PIE in the Jade Peaks map; Region 01, Verdance, Moonbound, and multi-dragon save regressions remain clean.

**Boundary:** This is not the full Jade Peaks campaign, all later regions, or the finale. Use supplied real art, preserve GAS as combat authority and the unified save subsystem as persistence coordinator, and do not invent missing full-world completion.

### WP-23 — Rest-of-world production umbrella

**Dependencies:** Verified WP-22 bounded foundation; explicit September 19, 2026 user authorization for WP-23 to cover the rest of the world; child-specific real content. **Routing:** Sol. **Read:** Campaign; B03 ending; save/control; current WP-23 packet.

**Work:** Execute the bounded child sequence in `Documentation/Current/tasks/WP-23.md`: readiness and asset ordering (WP-23.0); Verdant and Jade regional closure (WP-23.1–23.2); eight remaining dragon regions including the playable Zenith colony (WP-23.3–23.10); full-world continuity integration (WP-23.11); Judge/First Binder finale (WP-23.12); and continued-play plus release verification (WP-23.13).

**Stop / evidence:** Each child stops at its own region/integration acceptance boundary. The finale retains ECHO-10 and END-01–02; every regional child adds focused travel, route, dragon, Echo, and save-recovery evidence before it can feed WP-23.11.

**Boundary:** WP-23 is an umbrella, not one unbounded implementation pass. Work on one eligible child at a time. Missing art or rigs block only their dependent child; never substitute diagnostic primitives, create parallel owners, require optional Echoes, or claim the ten-region/finale state before integration evidence exists. WP-23.0, WP-23.1, and WP-23.2 are complete: Jade Peaks passed JC-01–08 and Verdant Reach passed VR-01–08 and remain readable under current Schema 7. WP-23.5 Nyxaroth passed NYX-01–05, the representative Cathedral/Michael Mire editor fixture passed, and `L_GloamingMarches` now passes its supplied foundation, Arrival-to-Ashgrave route, Count Malvaine parley/living submission, distinct Morrow/Mourn compassionate-release or paired GAS-submission encounter, optional Sanguine Strike, optional Second Turn, two-way Region01↔Gloaming Schema 7 recovery, the supplied 17-part Michael Mire GAS living-submission encounter, the supplied 17-part weapon-bearing Machete Mason GAS disarmed-submission encounter, the supplied 17-part sword-bearing Pleatherface GAS disarmed-submission encounter, the supplied 16-part 205 cm Wherewolf GAS calmed-submission encounter, the supplied 17-part 176 cm Annie Wails GAS disarmed-surrender encounter, and the supplied 16-part 174 cm Scarrie GAS living-submission encounter with one-time ledger restoration. Later Gloaming gameplay remains gated on the broader roster/completion boundary; every later new-dragon child remains rig/content gated.

## Reporting and scope changes

For each task state changed paths, exact inspected versions/assets, relevant test IDs and observed outcomes, commands/actions actually run, missing inputs and one next eligible task. Update only affected owner documents when behavior changes. Keep critical numeric values in their canonical system table. Never convert a BASELINE into approved production scope without recording the decision.

V1/Z1 and later creator/dragon/Echo content remain tracked after G5. Their dependency on a stable foundation is not a cancellation. Conversely, a complete planning file does not mean its associated task is implemented.
