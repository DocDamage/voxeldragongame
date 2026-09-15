# Combat and Nightmare Echoes — Prototype Specification

**Documentation release:** v0.1 · September 14, 2026  
**Status:** Detailed design proposal; not an implementation report  
**Basis:** H01 §§3,10–12; A01; B03 Nightmare Echoes; R1; see [source register](../SOURCES_AND_EVIDENCE.md).  
**Rule:** Existing requirements remain binding. New numbers and detailed behavior are draft baselines for a bounded prototype, not claims of user approval or runtime validation.

## 1. Ownership and terminology

GAS is the single authority for attributes, damage, status effects, ability costs and cooldowns. Inventory/equipment grants effects through one equipment owner; UI shows committed results. Separate combatants retain separate state. This is project design built on GAS's documented framework [R1], not a claim that GAS supplies this balance.

`Power` below means the humanoid's attack-power attribute after valid equipment grants. `WeaponBase` means the instance's saved base damage roll. `DamageEventID` prevents applying one impact twice. `AbilityID` identifies unlock, loadout, cooldown and persistence. A `CombatantID` is not the currently possessed actor reference.

## 2. Canonical numeric baseline — humanoid only

All values in this section are untested initial tuning. This is the only source for these numbers; other documents reference it.

| Parameter | Initial value / rule |
|---|---|
| Maximum health at level L | `100 + 8*(L-1)` before equipment |
| Maximum Focus | 100 before equipment |
| Power at level L | `20 + 3*(L-1)` before equipment |
| Focus recovery | 10 per gameplay second after one second without a Focus cost; no health regeneration implied |
| Base critical chance / multiplier | 5% / 1.5; only eligible direct hits, not every DOT tick |
| Physical mitigation | `min(0.70, Armor / (Armor + 50 + 10*AttackerLevel))` for nonnegative Armor |
| Elemental resistance | Clamp each applicable resistance to -0.50…0.75; negative means vulnerability |
| Basic damage | `WeaponBase + 0.50*Power`, coefficient adjusted only by the weapon-family definition |
| Weapon secondary fixture | `WeaponBase + 0.90*Power`; 20 Focus; 5-second cooldown |
| Dodge fixture | 0.50-second action; 1.25-second recovery; no Focus cost initially |
| Dodge evasion window | Gameplay seconds 0.10–0.25 of the action against eligible attacks; not all environmental hazards |
| Melee/ranged enemy fixture HP at L1 | 60 / 50; real encounter tuning may differ |
| Fixture XP | ordinary 15, elite 45; named quest/boss rewards use distinct authored records |

Damage pipeline: validate target/team and hit acceptance → compute nonnegative base → apply explicitly additive bonuses within a group, then declared multipliers → eligible critical → split by damage type → apply eligible temporary damage reduction within its declared cap → mitigate each component once → absorb eligible shield damage → subtract remaining damage from health → evaluate stagger/status and outcome. Armor is not applied again to elemental portions. No integer rounding until presentation; zero remains zero. Bound all inputs and reject non-finite values.

Example, not a benchmark: at L1, WeaponBase 10 and Power 20 gives a basic raw 20. Against Armor 20, mitigation is 20/(20+60)=0.25, so a noncritical purely physical hit deals 15. This arithmetic is a deterministic regression fixture.

Healing clamps to missing health. Health drain uses actual eligible damage dealt, excluding overkill, invulnerable hits, friendly targets and already-dead targets. Self-damage never feeds drain, damage storage, or on-hit loops unless separately authored; no such exception exists in the slice.

## 3. Two starting loadouts

Melee: real one-handed weapon, basic attack, committed secondary and dodge. Ranged: real ranged weapon/appropriate animation, projectile basic, a deliberate stronger secondary and dodge. Both draw from the same Power/Focus rules; a later magic kit can use a different damage type without a second stats implementation.

Do not invent a complete combo/skill tree before these work. Melee attacks need correct facing, range and recovery. Projectiles need spawn clearance, team filtering, collision and one-hit accounting. Camera mode changes targeting presentation, never the accepted damage rules. Tests include top-down ground-floor targeting beneath an upper surface.

## 4. Status and targeting contract

Statuses identify movement slow, light/medium stagger, hard stun, root, knockback, fear, DOT, temporary shield and control break. A full taxonomy is not required before a real mechanic uses it. Slow effects combine by strongest magnitude, not multiplicative stacking; remaining durations continue while temporarily suppressed. DOT instances have one source/ability family and an explicit refresh rule. Reapplication does not add unlimited independent stacks.

Bosses have declared resistance profiles. A resisted fear/grab is communicated and converts to the ability's specified damage/stagger-pressure alternative; it does not become an unannounced ineffective unlock. A hard story seal, noneditable structure or interactable is not an enemy status to cleanse. Friendly town NPCs cannot be farmed for on-hit effects.

Default active abilities block while dead, transitioning avatar/form, in an occupied vehicle, or in a non-gameplay modal interaction. Weapon skills also validate equipped family. A single pending action owns cancellation/commit. Cost and cooldown commit on activation acceptance, not UI press; an invalid target/destination costs nothing. After successful activation, interruption normally keeps spent cost/cooldown unless the ability explicitly refunds a pre-impact failure.

## 5. Permanent Echo acquisition

Designated encounter outcome + valid resolution predicate → unique resolution transaction → permanent `EchoUnlockRecord` + story facts + ordinary reward record → presentation. The ability unlock does not consume a bag slot and is not dependent on picking up a mesh. Repeated callbacks, alternative resolution entry, reloaded actor, or duplicate quest message grant it once.

Every Echo is stored permanently, but only equipped abilities are active. The four-slot BASELINE is shared with ordinary skills. Cooldowns belong to AbilityID and continue while unequipped; changing loadout or controlling a dragon does not erase them. Respec never revokes a learned Echo. Upgrades can modify an ability later, but its base playable function cannot require a rare item.

Resolve sympathetic encounters by an authored mercy/cure/trust alternative when B03 provides one. The Hollow Twins are released from their haunting, not a mandatory child-killing boss. Corvyn's cure can teach the same beast power. The disciple's ally route teaches Unseen Hand. These paths save different narrative outcomes with equivalent signature capability.

## 6. All twenty Echoes — authored player versions

Costs/durations/cooldowns here are **BASELINE tuning**, not approved final balance. Each is an active ability in the initial design; no passive operates automatically just because it was collected. `P` is current Power. Each named source remains as written in B03; unnamed characters are not silently given new canon names.

| ID / source → power | Cost; duration; cooldown | Playable effect and main bound |
|---|---|---|
| ECHO-01 Count Malvaine → **Sanguine Strike** | 25 Focus; next strike within 4s; 12s | Next eligible weapon hit gains 0.5P damage and heals 25% of actual damage, capped at 12% max HP. No drain from reflected/secondary effects. |
| ECHO-02 Hollow Twins → **Second Turn** | 25; repeat after 0.6s; 14s | Repeat the next eligible basic strike at 50% of its snapshotted pre-mitigation base against the still-valid target, then apply target mitigation once. Repeat is noncritical, cannot repeat itself or trigger drain/other Echo procs. |
| ECHO-03 Broodmother Cipher → **Brood Seed** | 35; 4s; 16s | One target takes total 0.8P over duration then a 0.8P burst in a small authored radius. One active seed per caster; no living NPC replacement or propagation. |
| ECHO-04 House Mark → **Deathmark** | 20; 6s; 12s | Mark one hostile; next eligible direct hit consumes mark for +1.0P damage. Mark cannot move through walls to unseen targets. |
| ECHO-05 Chef Aurelio Vane → **Carver’s Precision** | 25; next strike within 4s; 12s | Melee strike ignores 30% of target Armor for that hit and applies 0.6P wound over 3s. Not armor destruction stacking or cannibalism. |
| ECHO-06 Flayed Choir overseer → **Pain Reprisal** | 30; store 4s; 18s | Stores 30% of actual hostile health damage up to 20% max HP; expires into one targeted counter burst. Excludes self/reflected/stored damage and shields. |
| ECHO-07 Counselor → **Relentless Advance** | 30; 6s; 18s | Suppress movement slows and resist light/medium stagger. Still takes damage; no hard-control immunity or heal. Full behavior below. |
| ECHO-08 Canopy hunter → **Hunter’s Veil** | 25; up to 5s; 16s | Reduce normal detection and break targeting on eligible ordinary enemies. Attacking or taking direct damage ends veil. Not universal invisibility to bosses. |
| ECHO-09 Ser Corvyn → **Moonbound Form** | 40; 12s; 35s | Actual beast form, claw basic and one pounce; no equipped weapon actions in form. Same identity/health; restore original recipe/equipment. Contract below. |
| ECHO-10 Prisoners’ nightmare → **Waking Terror** | 30; 2s; 16s | Disrupt eligible nearby enemies; boss alternative 0.75P psychic damage plus authored stagger pressure, not a mandatory stun. |
| ECHO-11 Doctor Hollowmend → **Patchwork Guard** | 25; 5s; 16s | Shield equal to 15% max HP. One shield in this family; recast replaces only after cooldown, never adds permanent max HP. |
| ECHO-12 Bunker false rescuer → **False Refuge** | 35; field 5s; 20s | One fixed field grants player/active dragon 20% incoming direct-damage reduction while inside. No stacking multiple fields or safe-save sanctuary. |
| ECHO-13 Skinning Man → **Rending Grip** | 25; 0.8s control; 14s | Grab eligible nearby light enemy and deal 1.0P. Oversized/boss alternative a close hit plus wound, not lifting a dragon. |
| ECHO-14 Wrapped guardian → **Sepulcher Ward** | 25; up to 5s; 18s | Reduce next eligible heavy direct hit by 60%, then consume. No protection from void/death recovery or every tomb trap. |
| ECHO-15 Well-Bound → **Mirror Step** | 20; instant, afterimage 1s; 10s | Displace at most 4m to a validated clear reachable-volume destination. Cannot cross authored sealed boundaries or water/void incompatibilities. |
| ECHO-16 Temple disciple → **Unseen Hand** | 25; one impulse; 12s | Push a light eligible enemy or explicitly tagged object within 8m. Boss alternative 0.75P pressure hit. Does not move arbitrary homes or reshape terrain. |
| ECHO-17 Hollow Harvestman → **Dread Presence** | 30; 3s; 18s | Local fear/disruption for ordinary enemies. Boss alternative reduced aggression window where authored plus 0.5P impact; no guaranteed boss retreat. |
| ECHO-18 Carnival presence → **Dread Reflection** | 30; 5s; 20s | One temporary decoy attracts eligible attacks. Boss perception may reject it with readable feedback; cannot interact, loot, cast or reproduce. |
| ECHO-19 Unfinished puppet → **Borrowed Motion** | 35; 6s; 24s | One short-lived helper with bounded basic attacks and no ability grants. Not a second permanent dragon/party slot or worker economy. |
| ECHO-20 Rustbound Judge → **Break the Verdict** | 35; instant + 2s counter window; 24s | Remove eligible root/stun/restraint then one retaliation. Explicitly usable under those statuses; cannot undo narrative bindings. Unlock only at final permanent defeat. |

Area dimensions and presentation must be authored against actual assets before each later encounter enters production. Content that lacks a required beast, decoy, helper or effect asset is a real dependency, not a completed icon. The later catalogue is not a demand to implement all twenty skills before E1.

## 7. Relentless Advance — first complete ability contract

**Acquisition:** Silent Landing's resolved Counselor outcome (`echo.relentless_advance`), not a rare weapon drop. The enemy visibly demonstrates the advancing stance before the player learns it. Equipping is optional; tutorial interaction provides a safe practice target and can be skipped without losing the power.

**Activation:** Living humanoid, learned and equipped, enough Focus, cooldown ready, not hard-stunned, not form/control transitioning and not occupying a vehicle. Rooted activation is permitted but does not remove the root. A valid activation commits cost, duration and cooldown together. A pending camera/animation presentation does not delay its gameplay effect indefinitely.

**During effect:** Existing/new movement-slow modifiers are suppressed, not deleted; their timers continue. Light/medium stagger reactions are resisted. Hard stun, root, knockback, grabs and damage still work. Movement/attack speed is not increased. Terrain clearance, mining, swimming and flight restrictions are unchanged. The effect has no offensive damage of its own and no invulnerability frames.

**End:** Remaining unsuppressed slows resume only for their remaining time. Death ends the effect; control changes leave its remaining duration on the waiting humanoid. Temporary beast transformation is disallowed while a transition is pending, but active Relentless may continue as an identity-owned status if its tags apply to that form. The cooldown remains regardless of unequipping. UI displays status and remaining cooldown separately.

**Cancellation/failure:** Insufficient Focus, cooldown, dead or transition state produces a specific non-spending error. Once successfully active, manual cancellation does not refund Focus or reset cooldown. Duplicate activation request has one activation ID. Save during the effect stores its remaining duration and cooldown; reload resumes those values without an offline tick.

## 8. Temporary transformation — Moonbound contract

This is a real alternate playable form, not a stat buff with glowing eyes. A validated beast mesh/rig/animations supply movement, claws, pounce, hit and defeat presentation. Humanoid character name, identity, inventory, learned Echoes, base stats and permanent recipe remain unchanged. Save `formID`, remaining duration, and a reference to the original recipe/equipment, not a fresh generated character.

The first implementation should preserve the humanoid's logical GAS/inventory ownership and use a bounded alternate-form presentation/movement adapter. If the real rig requires another physical avatar, explicitly rebind that same logical state and never create a second inventory/health pool. This is not a second character creator or automatic Mutable creature-customization requirement.

Entry requires grounded clearance, no rider/vehicle occupancy, no remote dragon control, and a valid return area. Equipment visuals stow; their passive stat grants remain exactly once, but weapon-family active attacks are unavailable. Beast claws use P, not free extra weapon grant copies. Pounce validates path and destination; blocked movement consumes only under the documented accepted-activation rule, not arbitrary teleportation.

Return tests the original humanoid's actual body envelope. If a newly blocked position cannot fit, preserve the form/identity in a controlled return-pending state and select the last validated same-side return point. Do not gain health, cross a sealed barrier, discard gear or spawn a duplicate human. A finite recovery prompt handles missing safe points; no endless timer extension usable for combat. Damage/recovery rules remain explicit during that resolution.

Supported saves in beast form restore the remaining form duration after world/collision/recipe readiness. If assets are missing, stop with a preserved-save error rather than substituting another character. Hard-load compatibility failure is distinct from an ordinary blocked return position. A successful cure/trust route for Corvyn grants the exact same playable form record.

## 9. Proc and interaction limits

Every damage event carries source/parent IDs and a generation class: primary, periodic, reflected, repeated or summoned. Only declared primary events trigger on-hit Echo bonuses. Reflections/repeats/decoys/helpers do not trigger themselves, other recursive Echoes, item-based echo repetition or life drain. A helper does not count as another permanent companion and cannot receive a humanoid loadout.

Damage-reduction effects compose with an explicit cap of 75% from temporary ability effects before the relevant typed mitigation; this cap does not convert Relentless into damage reduction. Shields consume before health and do not count as health damage for Pain Reprisal. These are prototype design bounds and must be verified with representative gear combinations.

## 10. Definition of done

E1 requires an actual Counselor encounter, readable demonstration of the stance, valid persistent resolution, permanent usable ability, equip/use/save/reload, duplicate-reward protection, inventory-full behavior and optional-story skip path. A unit ability fixture alone proves a rule, not the encounter. Later powers each add their asset/interaction-specific cases rather than claiming E1 validates the whole catalogue.

See [acceptance procedures](../production/ACCEPTANCE_TESTS.md) for COM/ECHO cases. All runtime checks are NOT RUN in this documentation release.
