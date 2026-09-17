# WP-09: Green Dragon Locomotion, Combat & Direct Control Verification Proof

**Date**: 2026-09-17  
**Engine**: Unreal Engine 5.8.2 (`C:\Program Files\UE_5.8`)  
**Status**: **PASS in Real PIE & Native Automation**  
**Evidence Receipts**:
- `Saved/Diagnostics/WP09_dragon_proof.json` (5/5 PIE test groups PASS)
- `Saved/Automation/Scaffold/index.json` (37/37 native automation tests PASS)

---

## 1. Scope & Authority Boundaries

In strict compliance with [AGENTS.md](../../AGENTS.md) and single-owner architectural constraints:
1. **Dragon Companion / Actor Authority (`AWyrmDragonCharacter`)**:
   - Represents the living dragon entity in the world using genuine modular assets (Leader pose mesh `Hip-Local`, skeleton, 4 animations, and modular skeletal parts).
   - Single persistent dragon identity: no duplicate, ghost, or cloned dragon actors are created upon bond or possession.
2. **Combat Authority (`UAbilitySystemComponent` & `UWyrmAttributeSet`)**:
   - All dragon combat attributes (Health, MaxHealth, Focus, MaxFocus, AttackPower, DefensePower) and effects are authoritative through GAS.
   - Damage is delivered strictly via `UWyrmMeleeAttackAbility::ApplyDamageEffect`.
   - Hostile boss begins with 1800 Max HP / 1800 HP.
   - Living defeat transitions the dragon to `DefeatedAlive` state (0 HP, incapacitated, not a corpse).
   - One-way bonding converts the boss into an allied companion with 420 Max HP and heals to 50% (210 HP) on initial bond only. Re-bonding attempts are rejected.
3. **Possession & Control Authority (`AWyrmPlayerController`)**:
   - Manages direct control possession transfers between the created humanoid (`AWyrmCharacter`) and dragon companion (`AWyrmDragonCharacter`).
   - During dragon control, the created humanoid remains physically present and visible in the world with movement locked, while retaining active collision, GAS attributes, and inventory.
   - Input actions (`Move`, `Look`, `Jump`, `PrimaryAttack`, `SecondaryAttack`) route directly to the active pawn.
4. **Remote Risk & Tether Authority (`AWyrmDragonCharacter` / `AWyrmPlayerController`)**:
   - Enforces a 150m (15,000 cm) maximum tether radius from the waiting humanoid, with a warning threshold at 120m (12,000 cm).
   - Reaching the 150m boundary immediately halts dragon velocity and returns control safely to the humanoid.
   - Any damage dealt to the waiting humanoid immediately forces control return to humanoid to protect the vulnerable body.
   - Dragon defeat/incapacitation during direct control forces control return to the humanoid.
5. **Save Authority (`UWyrmSaveSubsystem` & `UWyrmSaveGame`)**:
   - Coordinates unified persistence across the game.
   - Schema version 5 incorporates `FWyrmDragonSaveRecord DragonRecord`, storing dragon transform, role, current/max health and focus, active order, cooldowns, bond receipt, and direct control possession status.
   - Restoring a snapshot taken during dragon direct control accurately restores dragon state and re-possesses the dragon while immobilizing the humanoid.

---

## 2. Implemented Components

### 2.1 Genuine Green Dragon Art Assets (`Content/WYRMFALL/Development/Intake/WP00/GreenDragon/`)
- Leader Skeletal Mesh: `/Game/WYRMFALL/Development/Intake/WP00/GreenDragon/Green_Dragon/SkeletalMeshes/Hip-Local`
- Skeleton: `/Game/WYRMFALL/Development/Intake/WP00/GreenDragon/Green_Dragon/SkeletalMeshes/Hip-Local_Skeleton`
- Verified Modular Parts: 7 key body parts (`Jaw`, `Neck`, `Spine1`, `Tail1`, `Wing_L`, `Wing_R`, `Head`) assembled with `LeaderPoseComponent`
- Verified Animations: 4 core animations (`Attack_01`, `Flight_Cycle`, `Locomotion_Cycle`, `Roar`)

### 2.2 Core C++ Classes (`Source/WYRMFALL/`)
- `Source/WYRMFALL/Public/Dragon/WyrmDragonTypes.h`:
  - `EWyrmDragonRole`: `WildBoss`, `DefeatedAlive`, `AlliedCompanion`.
  - `EWyrmCompanionOrder`: `Follow`, `Hold`, `AttackTarget`, `Return`.
  - `EWyrmDragonForm`: `Ground`, `Heartfold`, `Flight`.
  - `EWyrmTetherStatus`: `WithinTether`, `Warning`, `LimitReached`.
  - `FWyrmDragonSaveRecord`: Comprehensive serialization struct for dragon identity and state.
- `Source/WYRMFALL/Public/Dragon/WyrmDragonCharacter.h` & `WyrmDragonCharacter.cpp`:
  - Modular skeletal mesh assembly using `LeaderPoseComponent`.
  - GAS initialization with dedicated attribute set (`Health`, `MaxHealth`, `Focus`, `MaxFocus`).
  - Living defeat logic: prevents death/ragdoll, sets role to `DefeatedAlive`.
  - `AttemptBond()`: One-way conversion to `AlliedCompanion` with 420 Max HP and 210 initial HP. Rejects repeated attempts.
  - Companion orders & autonomous behavior (`Follow`, `Hold`, `AttackTarget`, `Return`).
  - Combat abilities: Primary claw strike (24 raw damage) and secondary area sweep (18 raw damage, 6s cooldown).
  - Tether tracking & tick monitoring against waiting humanoid.
  - Waiting humanoid damage reaction listener.
- `Source/WYRMFALL/Public/Player/WyrmPlayerController.h` & `WyrmPlayerController.cpp`:
  - `TransferControlToDragon()`: Moves player controller to dragon, anchors humanoid body, switches input.
  - `ReturnControlToHumanoid()`: Restores player controller to humanoid, reactivates humanoid movement.
- `Source/WYRMFALL/Public/Player/WyrmCharacter.h` & `WyrmCharacter.cpp`:
  - `FWyrmCharacterDamagedDelegate OnCharacterDamaged`: Broadcasts damage events to notify waiting body watchers.
- `Source/WYRMFALL/Public/Save/WyrmSaveGame.h` & `WyrmSaveSubsystem.cpp`:
  - Save schema version incremented to 5.
  - `FWyrmDragonSaveRecord DragonRecord` roundtrip serialization in `CreateSnapshotObject` and `ApplySnapshotObject`.

---

## 3. Verification Scenarios & Results

All 5 scenarios passed in real PIE via `py -3.12 tools/run_wp09_pie_proof.py`:

| Test Identifier | Acceptance Requirement | Result | Verified Details |
|---|---|---|---|
| **ASSET.DragonAssets** | Authentic Green Dragon skeletal meshes, skeleton, animations, and modular attachments loaded | **PASS** | Leader mesh, skeleton, 4 animations, and 7 modular parts verified in package registry |
| **DRG-01.LivingDefeatAndBond** | Boss 1800 HP $\rightarrow$ `DefeatedAlive` $\rightarrow$ one-way bond with 420 Max HP, 210 initial HP, no repeat bonding | **PASS** | Initial 1800 HP reduced to 0; transitioned to `DefeatedAlive`; bonded to `AlliedCompanion` with 210/420 HP; repeat bond call returned `false` |
| **DRG-02.CompanionCombat** | Companion autonomous orders (`Hold`, `Follow`) and GAS combat (primary 24 dmg, secondary 18 dmg, 6s cooldown) | **PASS** | Dummy health reduced from 60 to 36 (24 dmg) by primary strike; reduced from 36 to 18 (18 dmg) by area sweep; 6s cooldown blocked immediate reuse |
| **DRG-03_DRG-04.DirectControlAndTether** | Direct control possession, humanoid immobilized, 120m warning, 150m tether limit return, waiting body damage return | **PASS** | Controller possessed dragon; humanoid movement locked; 120m warning triggered; 150m tether returned control to humanoid; damaging humanoid body immediately returned control |
| **SAVE-08.DragonSaveRoundtrip** | Unified save schema roundtrip of dragon state, health, position, and direct control possession | **PASS** | Saved at 310 HP under direct control at `(400, 500, 100)`; applied snapshot; dragon restored to exact location, 310 HP, and direct control possession restored |

---

## 4. Native Automation Suite (37/37 PASS)

Run via `py -3.12 tools/wyrm.py ue-test`:
- `WYRMFALL.Scaffold.DragonLivingDefeatAndOneWayBond`: Validates living defeat, state transition, and one-way bonding receipt.
- `WYRMFALL.Scaffold.DragonCompanionOrdersAndCombat`: Validates orders (`Follow`, `Hold`, `AttackTarget`, `Return`) and GAS cooldowns.
- `WYRMFALL.Scaffold.DragonDirectControlPossession`: Validates possession transfer, humanoid immobilization, and return of control.
- `WYRMFALL.Scaffold.DragonRemoteTetherAndBodyDamageReaction`: Validates 120m warning, 150m tether limit return, and waiting body damage reaction.
- `WYRMFALL.Scaffold.DragonSaveLoadRemoteRoundtrip`: Validates dragon snapshot creation and roundtrip restore.

---

## 5. Verification Commands

```powershell
# Compile editor target
& "C:/Program Files/UE_5.8/Engine/Build/BatchFiles/Build.bat" WYRMFALLEditor Win64 Development "-Project=G:/assets/voxel project/WYRMFALL.uproject" -WaitMutex -NoHotReloadFromIDE -NoEngineChanges

# Run native automation test suite (37/37 tests)
py -3.12 tools/wyrm.py ue-test

# Run full WP-09 Play-In-Editor proof
py -3.12 tools/run_wp09_pie_proof.py
```
