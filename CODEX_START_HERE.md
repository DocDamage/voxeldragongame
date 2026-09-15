# Codex entrypoint

Open the folder containing this file as the **primary local project**. Do not open only the preserved documentation. Prefer native Windows for the Unreal stage so the agent can use the engine/assets on the development machine. A cloud/Linux Codex environment may run portable source/tooling checks, but it does not acquire your Windows engine, plugins or licensed local assets automatically. [Official references](Documentation/Current/TECHNICAL_REFERENCES.md).

For this existing Windows checkout, use [the current handoff](Documentation/Current/HANDOFF.md)
after STATUS. BOOT-01 already has scoped native evidence; do not restart onboarding
or GeoForge installation troubleshooting without a new failure.

## Initial read budget

Read root [AGENTS.md](AGENTS.md), [STATUS](Documentation/Current/STATUS.md) and [BOOT-01](Documentation/Current/tasks/BOOT-01.md). Preview with `python tools/wyrm.py plan --native`. Run `python tools/wyrm.py onboard --native` on the actual Unreal host; use `onboard` without `--native` for offline-only work. Read `python tools/wyrm.py report` after the run. Read source only as needed for compile failures. Do not read every reference file or repeat a full asset search each turn.

Use [CODEX_FIRST_TASK.txt](CODEX_FIRST_TASK.txt) as the session instruction. Once the host is actually built and observed, advance to [WP-00](Documentation/Current/tasks/WP-00.md), and then one [WP-01](Documentation/Current/tasks/WP-01.md) proof with sufficient inputs. Do not start the entire production backlog. Detailed acceptance procedures are already in [the preserved catalogue](Documentation/DesignPack/docs/production/ACCEPTANCE_TESTS.md).

## Client configuration

`.codex/config.toml` supplies workspace-write / on-request defaults and moderate reasoning effort. It intentionally does not choose an account-dependent model. Codex loads project configuration only for a trusted project. Root AGENTS instructions apply to the workspace; narrower source/content instructions give task-specific details. Verify effective settings in the actual client because organization/user policy and command-line overrides can affect them. This archive does not modify `~/.codex`.

Keep one agent by default. Use the least expensive capable available model for inventory/mechanical work; escalate difficult Unreal/GAS/Mutable/voxel integration only when needed. No claim is made that H01's model nicknames exist in the current account. No speculative parallel agents or mandatory all-history prompts.

UE build tooling may need writes to machine-local caches outside the project. Approve the necessary command or narrowly scoped location; do not disable the sandbox globally. Do not ask for an API key for this workflow.

## One bounded startup run

The onboarding workflow stops at the first failed command; it does not install
missing software, skip tests, open PIE, cook, or advance the production backlog.
Its `Saved/Onboarding/<run>/run.md` report names the failed step and one recovery
action. Use the individual `doctor`, `generate`, `build`, `bootstrap` and `ue-test`
commands while fixing a concrete error; do not repeatedly rebuild everything.
A completed native command run still needs the manual checks in BOOT-01.

`report` checks the receipt's workspace, host-source fingerprint and recorded log/
map/report hashes. Changed or missing evidence is stale. Source fingerprints are
not full engine/vendor-binary attestations; rerun when those installations change.
After a fresh extraction, no local onboarding receipt exists until you run it.

## Evidence and continuity

A tool test pass is not an Unreal pass. A native automation pass is not PIE. A debug capsule is not Mutable. A map-save receipt is not a terrain proof. Update [STATUS](Documentation/Current/STATUS.md), [readiness](Config/IntegrationReadiness.json), and a focused report using [SESSION_REPORT_TEMPLATE](Documentation/Current/SESSION_REPORT_TEMPLATE.md). Keep actual logs under Saved and copy only concise relevant evidence into tracked documentation when appropriate.

Stop on missing engine/plugin/real-art inputs for that dependent step, list precisely what could not be located, and continue only independent authorized work. Never invent paths or silently downgrade engine/features. Do not commit/push or initialize a repository without separate authorization.
