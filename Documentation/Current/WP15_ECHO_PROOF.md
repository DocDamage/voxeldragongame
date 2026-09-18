# WP-15: Echo Power Manifestation / Relentless Advance and Combat Evolution Proof

**Date**: 2026-09-18  
**Engine**: Unreal Engine 5.8.2 (`C:\Program Files\UE_5.8`)  
**Status**: **COMPLETE — First permanent usable horror power ("Echo: Relentless Advance"), Counselor encounter at Silent Landing, GAS combat authority integration, stance status resistances, cost/cooldown commits, full-bag safety, optional skip, dragon support, 75% ability damage reduction cap, and Quiet Water horror separation verified in live Play-In-Editor (PIE) under `L_Region01` (PASS: `ECHO-01`, `ECHO-02`, `ECHO-03`, `ECHO-04`, `ECHO-05`, `ECHO-06`, `REG-13`)**

## Scope and authority

WP-15 realizes the first permanent usable horror power (**"Echo: Relentless Advance"**) unlocked from the Counselor encounter at Silent Landing (`LM-SILENTLANDING`), integrating authoritatively with Gameplay Ability System (GAS) without creating parallel combat, resource, or inventory systems:

1. **Sole Combat Authority**: GAS remains the sole authority for combat attributes, abilities, damage mitigation, and cooldowns.
   - Tag hierarchy: `Ability.Echo.RelentlessAdvance`, `Cooldown.Echo.RelentlessAdvance`, `State.Combat.RelentlessAdvance`, `Unlock.Echo.RelentlessAdvance`.
   - `UWyrmGameplayAbility` and `UWyrmRelentlessAdvanceAbility` enforce authoritative commit of 30 Focus cost, 6.0s duration, and 18.0s cooldown through GAS.
   - Stance status profile: Suppresses movement slows to base walk speed (450 cm/s) without giving a speed buff; resists light and medium stagger; takes incoming damage normally (no iframes, no immunity, no healing); hard stuns still stop movement; rooted activation is permitted without cleansing the root.
2. **Sole Attribute & Damage Mitigation Authority**: `UWyrmAttributeSet` governs all combat formulas and damage mitigation:
   - Added `GetMaxAbilityDamageReductionPercent()` enforcing an explicit 75% ceiling on ability damage reduction.
   - `CalculateMitigatedDamageWithAbilityReduction` clamps requested ability reduction to 75% before armor calculations.
   - Relentless Advance has zero offensive damage procs; incoming damage and secondary procs cannot recursively trigger Echo effects.
3. **Sole Inventory Authority & Full-Bag Safety**: `UWyrmInventoryComponent` remains the sole inventory manager.
   - The Echo power is unlocked as a permanent ability on `AWyrmCharacter` and recorded as a fact on `UWyrmRegion01Subsystem` (`echo.relentless_advance`), never as a physical bag item. A completely full inventory does not block unlocking the Echo.
   - Ordinary loot (`Item_CounselorShard`) dropped by Counselor is safely preserved in `AWyrmCounselorCharacter::PreservedLoot` if the bag is full and can be claimed later once inventory space is freed.
   - Idempotency: Replaying encounter resolution callbacks strictly rejects duplicate rewards.
4. **Horror Character Art & Separation**:
   - Dedicated horror character artwork is preserved as a future integration reservation; diagnostic actor definitions (`AWyrmCounselorCharacter`, `AWyrmPracticeTarget`) are used without introducing generated or paid replacement art.
   - Horror separation: Ordinary fishing lessons at Quiet Water remain completely peaceful; Counselor is staged exclusively at Silent Landing (> 3800 units away), requiring deliberate traversal to encounter.

## Verification performed

| Check | Result | Evidence / boundary |
|---|---|---|
| UE 5.8.2 editor compile | PASS | `WYRMFALLEditor Win64 Development`; clean compile and link |
| Production Region 01 WP-15 PIE proof | PASS: 7/7 | `py -3.12 tools/run_wp15_echo_proof.py`; [receipt](../../Saved/Diagnostics/WP15_echo_proof.json) |
| Production Region 01 WP-14 PIE proof | PASS: 6/6 | `py -3.12 tools/run_wp14_terrace_cave_proof.py`; clean regression check |
| Source-declared native automation tests | PASS: 46/46 | `py -3.12 tools/wyrm.py ue-test`; all 46 tests pass |
| Portable source/config check | PASS | `py -3.12 tools/wyrm.py verify`; clean source compliance |
| Portable tooling tests | PASS: 124, 2 expected skips | `py -3.12 tools/wyrm.py test` under local Windows permissions |

## Detailed Production Acceptance Evidence

All seven assigned acceptance cases were verified in live Play-In-Editor (PIE) in `L_Region01`:

### 1. `ECHO-01.RealCounselorReward` (PASS)
- **Stance Demonstration**: Observed Counselor entering Relentless Advance stance (`bInRelentlessStance = True`, `State.Combat.RelentlessAdvance` tag present).
- **Stance Resistances**: Verified Counselor resists movement slow (speed maintained at 450 cm/s) while still taking full damage when struck by the player.
- **Normal Strike Combat**: Counselor performed normal strike dealing 14.0 authoritative physical damage to the humanoid player.
- **Living Defeat**: Defeated Counselor to 0 HP; living defeat recognized without corpse/ragdoll destruction.
- **Echo & Loot Grant**: Defeat resolution permanently unlocked `Echo: Relentless Advance` on the character, granted `Unlock.Echo.RelentlessAdvance` tag, equipped the Echo, recorded `echo.relentless_advance` fact in `UWyrmRegion01Subsystem`, and delivered ordinary loot (`Item_CounselorShard`) to player inventory.

### 2. `ECHO-02.RelentlessStatuses` (PASS)
- **Slow Suppression**: Player slowed to 50% speed (225 cm/s); activating Relentless Advance restored speed to base 450 cm/s while the slow timer continued running. No speed boost was granted above 450 cm/s.
- **Stagger Resistance**: Applied light/medium stagger (`State.Combat.Stagger`); stagger reaction was resisted and ignored while in stance.
- **Damage Taken Normally**: Counselor struck player during Relentless Advance; damage was taken normally (no invulnerability frames, no healing).
- **Hard Stun Functioning**: Applied hard stun (`State.Combat.Stun`); player movement was completely disabled (speed = 0 cm/s) even during active Relentless Advance.
- **Rooted Activation**: Applied root (`State.Combat.Root`); movement stopped (speed = 0 cm/s). Activating Relentless Advance while rooted was permitted by contract, but did not cleanse the root or restore speed until root expired.

### 3. `ECHO-03.RelentlessFailureAndTime` (PASS)
- **Insufficient Focus Rejection**: With Focus < 30 (15.0 Focus), activation was rejected with zero spend (Focus remained 15.0).
- **Control Transition Rejection**: Activation while in control transition state (`State.Control.Transition`) was rejected.
- **Authoritative Commit**: Valid activation committed 30 Focus cost (100.0 -> 70.0), 6.0s duration, and 18.0s cooldown together via GAS.
- **Cooldown Gating**: Subsequent activation attempt while on cooldown was rejected (`OnCooldown`).
- **Cooldown Retention on Unequip**: Unequipping the Echo during cooldown retained the active cooldown without reset or Focus refund.

### 4. `ECHO-04.FullBagAndDoubleOutcome` (PASS)
- **Full-Bag Outcome**: Player inventory filled completely to maximum bag capacity (10/10 slots). Encounter resolution with full bags successfully granted the Echo unlock outside bag storage.
- **Preserved Loot**: Ordinary loot (`Item_CounselorShard`) was safely held in `counselor.preserved_loot`.
- **Strict Idempotency**: Replaying the resolution callback returned `False` and rejected duplicate grants.
- **Preserved Loot Claim**: Freeing 1 bag slot enabled `claim_preserved_loot`, cleanly delivering the shard without loss.

### 5. `ECHO-05.OptionalSkipAndDragonSupport` (PASS)
- **Optional Skip**: Reset Region 01 state and verified that full homecoming readiness requires zero Echo/Counselor facts (`homecoming_ready = True` with `echo.relentless_advance` absent).
- **Dragon Combat Support**: Bonded dragon Verdance moved to Silent Landing and performed primary attack dealing 24.0 damage to Counselor.
- **Quiet Water Distance**: Verified Counselor location (`3000, -1900, 750`) is 3842.4 units away from Mara at Quiet Water (`0, 50, 888`), ensuring zero cross-encounter interference.

### 6. `ECHO-06.ProcCap` (PASS)
- **Damage Reduction Cap**: Verified `UWyrmAttributeSet::GetMaxAbilityDamageReductionPercent()` returns 0.75 (75% explicit cap).
- **Formula Clamping**: Tested reduction formula with 90% requested reduction on 100 raw damage; output was clamped to 75% reduction (25.0 mitigated damage).
- **Zero Offensive Procs**: Verified Relentless Advance has no offensive damage procs, and incoming damage triggers no secondary recursive Echo effects.

### 7. `REG-13.HorrorSeparation` (PASS)
- **Quiet Water Safety**: Player stood at Quiet Water and successfully engaged in peaceful fishing without horror encounters or combat triggers.
- **Deliberate Landmark Entry**: Traversed to Silent Landing; completed homecoming and recorded deliberate visit to `LM-SILENTLANDING`.
- **Practice Target**: Activated practice target at Silent Landing, which applied practice slow and allowed safe player testing.
- **Safe Retreat**: Traversed back to town entry safely with normal movement restored.

## Diagnostic Receipts

- [WP15 Echo Proof Receipt](../../Saved/Diagnostics/WP15_echo_proof.json)
- [WP14 Terrace and Cave Proof Receipt](../../Saved/Diagnostics/WP14_terrace_cave_proof.json)
- [Composed Region 01 Map](../../Content/WYRMFALL/World/Regions/L_Region01.umap)
