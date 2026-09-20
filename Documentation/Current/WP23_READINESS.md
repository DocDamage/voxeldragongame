# WP-23.0 rest-of-world readiness ledger

**Result:** PASS for planning/readiness on September 19, 2026. Archive presence
is not Unreal import, rig validation, or production acceptance.

## Evidence

`py -3.12 tools/wp23_readiness_inventory.py` hashes the supplied candidate
files and verifies each named dragon GLTF inside
`assets and old docs/Voxel+Dragons+Pack+Upload.zip`. The receipt is
`Saved/Diagnostics/WP23_readiness_inventory.json`.

All ten source dragons are present: Green, Chinese, Wooden, White, Dark,
Steampunk, Lava, Zombie, Skull, and Mecha. Only **Verdance/Green** and
**Jadefang/Chinese** have project rig profiles plus live-PIE Heartfold, control,
combat, flight, and persistence evidence. The remaining eight models are
source candidates and stay fail-closed under DRG-15.

## Readiness and production order

| Order | Child | Evidence-backed state | Exact gate |
|---:|---|---|---|
| 1 | WP-23.2 Jade Peaks closure | **COMPLETE** | JC-01..08 passed and were rerun under current Schema 7 with supplied King art, pact/disciple route parity, Unseen Hand, and two-route travel. |
| 2 | WP-23.1 Verdant Reach closure | **COMPLETE** | VR-01..08 passed and were rerun under current Schema 7 with supplied Ranger art, Meridess route parity, optional canopy hunter, Hunter's Veil, live navigation, and rendered QA. |
| 3 | WP-23.5 Gloaming Marches | **COUNT DRIPULA PASS / COMPLETION GATED** | Verified through the supplied 16-part, 184 cm Count Dripula GAS bloodless-surrender encounter with Schema 7 restoration and no Echo/completion leakage. Further required-horror identities and completion remain gated. |
| 4 | WP-23.8 Ashen Wastes | BLOCKED | Validate Rotwing/Zombie Dragon rig; establish Rotking, laboratory, and bunker content fit. |
| 5 | WP-23.9 Bonelands | BLOCKED | Validate Ossuroth/Skull Dragon rig; establish Kael, tomb, and guardian content fit. |
| 6 | WP-23.3 Hallowwood | BLOCKED | Validate Grovemaw/Wooden Dragon rig; establish ruler and three distinct horror encounter fits. |
| 7 | WP-23.6 Cogspire Harbor | BLOCKED | Validate Cogfang/Steampunk Dragon rig; establish harbor, city-engine, ruler, and urban encounter content fit. |
| 8 | WP-23.4 Frosthold | BLOCKED | Validate Frostmane/White Dragon rig and locate suitable snow/ice region and ruler content. |
| 9 | WP-23.7 Cinderreach | BLOCKED | Validate Pyraxis/Lava Dragon rig and locate suitable volcanic trial/forge/ruler content. |
| 10 | WP-23.10 Zenith Spire | BLOCKED | Validate the unnamed Mecha Dragon rig and locate a playable city/colony plus ruler content. Robot and tiny-car archives alone do not satisfy Z1. |

This order prioritizes validated identities and the strongest supplied regional
fits. It is not a requirement to build blocked regions in parallel, and later
ordering may change when a blocker is cleared by real evidence.

## Travel/save contract for regional children

WP-23.2 introduced the bounded `UWyrmWorldTravelSubsystem` owner with only the
Region01↔JadePeaks and Region01↔GloamingMarches routes. `UWyrmSaveSubsystem`
Schema 7 remains the sole save
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
6. Full-world persistence uses the Schema 7 record, retaining the Schema 6
   region-keyed terrain/camp array, current region and arrival/return identifiers,
   and earlier Echo state while adding Gloaming facts/receipts plus Sanguine
   Strike and Second Turn cooldowns. Schemas 1–6 remain accepted;
   later regions must extend this contract without another save owner.
7. Inactive dragons remain saved by stable `DragonId`; destination restoration
   may spawn only a known validated profile. DRG-15 remains fail-closed.

WP-23.2 implemented only the minimal route and schema increment exercised in
real PIE. Later children extend the same owners; do not build a generic empty
framework.

## Host note

The earlier C: derived-data-cache shortage is resolved; the drive reported
approximately 107 GB free during this inventory. The project-local DDC override
remains harmless and reproducible but is no longer a blocker.

## Next bounded task

Inspect and author one additional supplied required-horror identity using the
same established assembly and authorization context. Keep any new Echo and
regional completion outside that slice; see
[WP-23.5](tasks/WP-23.5.md).
