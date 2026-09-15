# WYRMFALL onboarding result

**Status: OFFLINE_CHECKS_PASSED_NATIVE_NOT_RUN**

These are command checks, not a playable build or a G0/G1 acceptance pass.

| Step | Result | Exit | Log |
|---|---|---|---|
| verify | COMMAND_PASSED | 0 | `Saved/ScaffoldLogs/20260915T073537Z_508f9543b51d_verify.log` |
| test | COMMAND_PASSED | 0 | `Saved/ScaffoldLogs/20260915T073537Z_9cc6a2c85210_test.log` |
| design-verify | COMMAND_PASSED | 0 | `Saved/ScaffoldLogs/20260915T073544Z_a5db6f5d85f0_design-verify.log` |

## Next bounded task

Run onboard --native on the actual Unreal-equipped host, then perform focused PIE observations.

## Still unverified

PIE, controller hardware, asset import, terrain, Mutable content and cooking remain NOT_RUN here.
No source snapshot or command exit code overrides those gates.
