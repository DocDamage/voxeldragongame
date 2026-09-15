# WYRMFALL starter v0.2 — focused hardening

September 15, 2026. Cumulative replacement for v0.1, not a patch that requires
layering ZIPs. No gameplay gates are promoted and no approved scope is changed.

## Concrete fixes

| Finding in v0.1 | v0.2 change | Evidence boundary |
|---|---|---|
| Native report runner expected four fixed names | Discovers simple native test declarations and requires exact, nonempty, duplicate-free coverage | Parser regressions passed; six UE tests authored, not executed |
| Successful-looking report could contain error counts | Rejects nonzero errors and malformed/nonzero summary counts | Offline parser tests |
| Engine-path reconfiguration cleared existing asset roots | Preserves them unless explicitly replaced | CLI regression with temporary metadata only |
| Integer conversion accepted fractional/bool engine-version fields | Strict integer metadata validation and actionable settings errors | Metadata regressions |
| Logs could collide within a second; timeout could leave descendants | Unique per-attempt logs/receipts, bounded waits and tree cleanup | Real Python subprocess/POSIX tests; Windows execution pending |
| No single bounded startup/recovery workflow | Added plan, onboard, report, source snapshot and hashed evidence checks | Real offline run; real missing-engine stop |
| Map receipt relied on file presence and actor labels | Requires typed/unique actors, floor mesh/collision-profile checks and map SHA-256 identity | Receipt parser tests; editor script not run |
| Repeated controller binding could duplicate callbacks | Clears only this controller's callbacks before rebinding | C++ source change, native/PIE pending |
| Click/jump paths lacked explicit pause/move-suppression guards | Added guards and cancel prior click motion before jump | C++ source change, native/PIE pending |
| GAS base-value route lacked shared clamps | One helper for current/base health/max-health input | Two new native test cases authored, not executed |

## Entry points

`python tools/wyrm.py plan --native` previews commands without executing them.
`python tools/wyrm.py onboard` runs offline source/tooling/design-integrity checks.
`python tools/wyrm.py onboard --native --engine-root "actual path"` adds the local
engine command sequence. `python tools/wyrm.py report` checks the recorded result
against the current workspace source and recorded evidence. `native-tests` lists
the source-declared suite; it does not run it.

Onboarding stops at the first failure and writes one recovery action. Success
never implies PIE, cooking, Mutable content, actual terrain or a gameplay gate.
Source fingerprints exclude engine/vendor binaries, global toolchains and most
production content; rerun when those change rather than treating the hash as a
complete environment attestation.

## Upgrade without losing local work

Extract into a fresh folder. This ZIP is a complete source snapshot. When v0.1
has already been edited locally, compare the old and new roots before merging;
do not overwrite local Content, Plugins, Source, .local settings or Saved evidence.
The release's machine-readable change list is under
[release changes](Documentation/Current/evidence/v02/changes_from_v01.json).
Setup.ps1 now uses `-Native` rather than the old `-Build -Bootstrap` switches.

All 36 original design-pack files remain byte-for-byte preserved. Mutable, GAS,
shared humanoid/cameras, pet-size dragons, horror power rewards, vehicles and the
colony retain their approved/proposed distinctions. No inventories, save owners,
new providers, production art or future-gameplay frameworks were added.

[Validation](VALIDATION.md) · [Current status](Documentation/Current/STATUS.md)
