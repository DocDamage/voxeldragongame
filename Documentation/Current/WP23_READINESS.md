# WP-23.0 rest-of-world readiness ledger

**Result:** PASS for planning/readiness, refreshed September 20, 2026. Archive presence
is not Unreal import, rig validation, or production acceptance.

## Evidence

`py -3.12 tools/wp23_readiness_inventory.py` hashes the supplied candidate
files and verifies each named dragon GLTF inside
`assets and old docs/Voxel+Dragons+Pack+Upload.zip`. The receipt is
`Saved/Diagnostics/WP23_readiness_inventory.json`.

All ten source dragons are present: Green, Chinese, Wooden, White, Dark,
Steampunk, Lava, Zombie, Skull, and Mecha. **Verdance/Green, Jadefang/Chinese,
Nyxaroth/Dark, Cogfang/Steampunk, Grovemaw/Wooden, and Rotwing/Zombie** have
project rig profiles plus live-PIE Heartfold, control, combat, flight, and
persistence evidence. The remaining four models are source candidates and stay
fail-closed under DRG-15.

## Readiness and production order

| Order | Child | Evidence-backed state | Exact gate |
|---:|---|---|---|
| 1 | WP-23.2 Jade Peaks closure | **COMPLETE** | JC-01..08 passed and were rerun under current Schema 7 with supplied King art, pact/disciple route parity, Unseen Hand, and two-route travel. |
| 2 | WP-23.1 Verdant Reach closure | **COMPLETE** | VR-01..08 passed and were rerun under current Schema 7 with supplied Ranger art, Meridess route parity, optional canopy hunter, Hunter's Veil, live navigation, and rendered QA. |
| 3 | WP-23.5 Gloaming Marches | **COMPLETE** | All 20 supplied identities, optional Echo paths, validated bonded Nyxaroth, navigation, two-way travel, and Schema 7 recovery passed. Completion commits once after the full roster and safe-return context, without taxing either optional Echo. |
| 4 | WP-23.6 Cogspire Harbor | **COMPLETE** | Mainline completion remains stable; both optional supplied-art investigations and permanent usable GAS Echoes pass and restore through Schema 8 without becoming completion taxes; [Chef proof](WP23_6_CHEF_AURELIO_CARVERS_PRECISION_PROOF.md), [House Mark proof](WP23_6_HOUSE_MARK_DEATHMARK_PROOF.md), [closure proof](WP23_6_COGSPIRE_COMPLETION_PROOF.md). |
| 5 | WP-23.3 Hallowwood | **GROVEMAW COMPLETE; AUDIT COMPLETE; REGION BLOCKED** | The distinct 35-part Grovemaw profile passed all five live-PIE groups and stable-identity recovery. Master Wizard and forest passed intake. An exhaustive 64-ZIP/10-RAR audit found supporting trace props but no exact authored carnival kit or valid non-franchise named-horror fits. See [profile proof](WP23_3_GROVEMAW_PROFILE_PROOF.md) and [focused readiness](WP23_3_HALLOWWOOD_READINESS.md). |
| 6 | WP-23.8 Ashen Wastes | **ROTWING/AUDIT COMPLETE; REGION BLOCKED** | The distinct 35-part Rotwing profile passed all five live-PIE groups and stable-identity recovery. Zombie is the selected Rotking base; ash/ruin and bunker props passed focused intake. An exhaustive 64-ZIP/10-RAR audit found no exact laboratory, Doctor Hollowmend, or false-rescuer model-name fit. See [profile proof](WP23_8_ROTWING_PROFILE_PROOF.md) and [focused readiness](WP23_8_ASHEN_WASTES_READINESS.md). |
| 7 | WP-23.9 Bonelands | BLOCKED | Validate Ossuroth/Skull Dragon rig; establish Kael, tomb, and guardian content fit. |
| 8 | WP-23.4 Frosthold | BLOCKED | Validate Frostmane/White Dragon rig and locate suitable snow/ice region and ruler content. |
| 9 | WP-23.7 Cinderreach | BLOCKED | Validate Pyraxis/Lava Dragon rig and locate suitable volcanic trial/forge/ruler content. |
| 10 | WP-23.10 Zenith Spire | BLOCKED | Validate the unnamed Mecha Dragon rig and locate a playable city/colony plus ruler content. Robot and tiny-car archives alone do not satisfy Z1. |

Completed children are listed first. Hallowwood's bounded source-fit audit is
finished and does not clear its regional entry gate. WP-23.8's bounded intake
and exhaustive audit are also finished without clearing its regional entry
gate. WP-23.9 Bonelands is the next selected readiness target.
Blocked regions are not built in parallel, and later ordering may change when
a blocker is cleared by real evidence.

## Travel/save contract for regional children

WP-23.2 introduced the bounded `UWyrmWorldTravelSubsystem` owner. It now
allowlists only Region01↔JadePeaks, Region01↔GloamingMarches, and
Region01↔CogspireHarbor. `UWyrmSaveSubsystem`
Schema 8 remains the sole save
coordinator and now owns region-keyed terrain/camp records, current travel
region/arrival data, the existing character/inventory/dragon/vehicle records,
and both regional fact records. Later work extends these owners rather than
adding parallel travel or save coordinators.

Each regional child that introduces cross-map travel must prove this bounded
contract:

1. A route is an allowlisted source region/landmark → destination map/arrival
   landmark pair with an explicit humanoid return pair. Arbitrary map strings
   are rejected.
2. Travel preflight rejects unsafe transient states such as active Heartfold,
   unsupported airborne dismount, unresolved terrain mutation, or missing safe
   arrival; rejection does not spend rewards or mutate regional facts.
3. The unified save coordinator writes a recoverable pre-travel snapshot before
   loading the destination. A failed load or arrival validation restores the
   source map and source safe anchor.
4. Destination arrival waits for terrain collision and navigation, resolves the
   authored arrival anchor, then restores character, inventory, active dragon
   identity, Echo cooldowns, and permitted vehicle state through existing owners.
5. Regional facts and one-time rewards commit only after successful arrival or
   the authored interaction—not when travel is requested. Duplicate travel and
   reload cannot duplicate a dragon, reward, camp piece, or hovercar.
6. Full-world persistence uses the Schema 8 record, retaining the Schema 6
   region-keyed terrain/camp array, current region and arrival/return identifiers,
   and earlier Echo state while adding Gloaming facts/receipts plus Sanguine
   Strike and Second Turn cooldowns plus Cogspire facts and selective-engine
   state. Schemas 1–7 remain accepted;
   later regions must extend this contract without another save owner.
7. Inactive dragons remain saved by stable `DragonId`; destination restoration
   may spawn only a known validated profile. DRG-15 remains fail-closed.

Each accepted regional slice implements only its exercised routes. Cogspire
extends the same owner through Schema 8 without a generic empty framework.

## Host note

The earlier C: derived-data-cache shortage is resolved; the drive reported
approximately 107 GB free during this inventory. The project-local DDC override
remains harmless and reproducible but is no longer a blocker.

## Next bounded task

Run the WP-23.9 Bonelands source-fit audit for Ossuroth/Skull Dragon, Kael,
tomb, and guardian content. Do not start a production map or regional gameplay
from archive presence or generic substitutes.
