# WP-23.5 Gloaming content-integration fixture

**Result:** PASS on September 19, 2026 for the bounded editor fixture.
**Region status:** GATED. No production Gloaming map, route, encounter, Echo,
travel, navigation, or save acceptance is claimed.

## Scope proved

The unsaved UE 5.8.2 editor fixture uses supplied art only. It stages four
Cathedral roles and one representative required horror actor:

| Fixture identity | Supplied presentation | Animation strategy | Measured height |
|---|---|---|---:|
| Ashgrave | Crusader skeletal mesh and palette | supplied Idle `AnimSequence` | 180 cm |
| Count Malvaine | Priest skeletal mesh and palette | supplied Preach `AnimSequence` | 180 cm |
| Hollow Twin Morrow | Nun skeletal mesh and palette | supplied Praying `AnimSequence` | 180 cm |
| Hollow Twin Mourn | separately staged Nun instance and palette | supplied Praying clip at a distinct phase | 180 cm |
| Michael Mire | 17 supplied `MicahelMeyers` static body parts | rigid modular procedural-root idle; skeletal locomotion is not claimed | 190 cm |

Every Cathedral mesh retains its imported skeleton and physics asset. The
fixture imports each source FBX into an isolated namespace to prevent repeated
`palette_001` names from silently sharing the wrong material. It then binds the
matching supplied character PNG through a fixture-only material. No generated
replacement art is used.

Michael Mire uses one shared transform and scale for all 17 visual parts. The
visual parts use `NoCollision`; a 76 cm-wide `BlockAll` body proxy supplies the
bounded collision strategy. The same body-proxy pattern is used for the four
Cathedral roles while retaining their physics assets for later gameplay actors.

## Native editor evidence

Command:

```text
py -3.12 tools/run_wp23_5_gloaming_content_fixture.py
```

The final run exited 0 and wrote
`Saved/Diagnostics/WP23_5_gloaming_content_fixture.json` with status
`PASS_AUTOMATED_REQUIRES_MANUAL_VISUAL_REVIEW`. Five vertical traces hit the
expected Ashgrave, Count Malvaine, two Hollow Twin, and Michael Mire collision
proxies. All material slots were non-null, all role animations targeted their
own supplied skeleton, and both captures were written.

## Manual capture review

Final captures:

- `Saved/Diagnostics/WP23_5_GloamingContentFixture/01_cathedral_roles_and_horror.png`
- `Saved/Diagnostics/WP23_5_GloamingContentFixture/02_michael_mire_assembly.png`

Manual review passed the scoped visual gate:

- the four Cathedral actors are grounded, consistently human-scaled, visually
  distinct, and show their supplied dark/blue/white/yellow palettes;
- Michael Mire is a coherent humanoid rather than scattered parts, with a
  readable face, clothing, blood accents, silhouette, and grounded feet;
- the 180 cm roles and 190 cm horror actor read as compatible human-scale
  content; and
- fixture lighting exposes the materials without hiding the dark silhouettes
  or clipping the supplied colors.

The first dark capture and two overexposed diagnostic attempts were rejected,
not counted as passes. They exposed and led to corrections for physical-light
intensity, palette namespace collisions, explicit external PNG binding, and
collision-proxy visibility.

## Remaining boundary

This representative proof closes the content-integration strategy gate, not
the regional production gate. The modular cemetery/church OBJ subset still
needs selective import and the eventual map needs environment scale/material/
collision, navigation, lighting, and route-readability proof. Additional
required horror identities can reuse the proven assembly strategy but remain
unaccepted until authored and reviewed. Permission/provenance is still open.
