# GeoForge installation recovery — September 15, 2026

**Resolved:** GeoForge 5.0.0 is installed in the C: UE 5.8.2 engine. Actual
runtime, editor and ProceduralMeshComponent native loading passed. The earlier
failed attempts below remain history; this is not a WP-01 terrain proof.

## Observed failure

Epic's Download Manager showed **GeoForgeRuntime - Block-Based Cubic World**,
“Update failed at 100%,” and **IS-0009: Could not load installation information**.
The plugin's installation record still pointed to `D:\Unreal\UE_5.8`, including
its `.egstore` manifest and staging directories. Its host installation GUID was
the old D: engine. The working engine is `C:\Program Files\UE_5.8`, verified from
Build.version as 5.8.2 / CL 56702186 and from its complete Epic manifest.

ProceduralMeshComponent remains bundled in that engine and explicitly enabled
in WYRMFALL. Its earlier native load proof is in
[the dependency report](PROCEDURAL_MESH_DEPENDENCY.md).

Epic describes IS-0009 as a manifest-loading failure caused by an invalid or
corrupt file: [official support](https://www.epicgames.com/help/c-202300000001639/c-202300000001736/a202300000013120?lang=en-US).
The concrete local evidence here is the stale D: registration. The broad
game/library deletion procedure on that page was not performed.

## Bounded repair performed

- Inspected the launcher log, the actual failure UI, plugin/engine manifests,
  Build.version, engine plugin files, and installation index.
- Backed up GeoForge's `.item`, the C: engine `.item`, and LauncherInstalled.dat
  under `Saved/Diagnostics/GeoForgeInstallRepair_20260915/`.
- Stopped the verified EpicGamesLauncher process (no Unreal editor was running).
  An immediate process-exit check initially still saw it; a fresh process
  enumeration confirmed it had exited before any registration edit.
- Moved only `C19CF9B448337E23E304ECAB78C513A2.item` out of Epic's Manifests
  directory into the backup directory as `.item.disabled`.
- Removed only the matching GeoForge/D: entry from LauncherInstalled.dat.
  All eight other entries were preserved. No plugin or engine files were deleted.
- Restarted Epic. The launcher regenerated the C: UE 5.8 installation index
  entry, and the stale GeoForge `.item` remained absent.
  During restart Epic also regenerated its index for Fab/Bridge 5.8 and dropped
  six older D: engine/Fab/Bridge entries. A post-restart assertion that all eight
  unrelated index entries would remain identical therefore failed; the changes
  are retained explicitly in `repair.json`. Those entries were preserved by the
  repair edit, and no corresponding engine/plugin files were changed by it.
- Opened the owned GeoForge listing and prepared its **Install Plugin / 5.8**
  dialog. The user subsequently retried and reported another failure.

The restarted launcher also logged an unreadable old cache manifest at
`D:/VaultCache/FabLibrary/GeoForgeRuntime_-_Block-Based_Cubic_World-ac179c02/unreal-engine/manifest`.
On the subsequent retry, Fab showed a Download Failed toast with no new
GeoForge job in Download Manager and no GeoForge installation manifest/files
on C:. Logs identify `D:/VaultCache/FabLibrary/listings_v1.db` and failures to
commit local listings, download metadata and acquisitions. The stale engine
record removal was insufficient; the unreadable cache/database is the next
concrete recovery target. The exact causal link to the toast remains an
inference until a retry with a working cache succeeds.

Prepared `C:\EpicVaultCache` and verified a temporary file write/read there;
removed only that uniquely named probe file. Backed up GameUserSettings.ini.
The user explicitly authorized changing the cache and retrying installation.
The launcher folder picker appended `VaultCache`, producing the actual path
`C:\EpicVaultCache\VaultCache`. The UI showed the new value but the saved INI
still contained D:. With Epic stopped, changed only `VaultCacheDirectories` to
`C:/EpicVaultCache/VaultCache/` and restarted it. The default install directory
setting remains D:; this plugin install targets the existing C: engine explicitly.

Fab then opened `C:/EpicVaultCache/VaultCache/FabLibrary/listings_v1.db`, with no
recurrence of the database commit errors in the fresh session. Retried the owned
GeoForge listing with Install Plugin / 5.8. At 15:51:05 UTC Epic reported
`ErrorCode: OK` and committed a complete new installation manifest:
`0D0BD72340B76F5098954E82AA037942.item`, host GUID
`5D77D942AEF8AF9DF685629024CD644D`, install location `C:\Program Files\UE_5.8`.
The installed plugin is
`Engine/Plugins/Marketplace/GeoForge33217d52984fV2/GeoForgeRuntime.uplugin`:
version 5.0.0, EngineVersion 5.8.0, Win64, runtime and editor modules, dependency
ProceduralMeshComponent. Both native DLLs use the compatible BuildId 55116800.

Evidence: `cache_diagnosis.json` / `cache_errors.log` retain the failed state;
`cache_recovered.log`, configuration backups and `installation_verified.json`
record recovery under `Saved/Diagnostics/GeoForgeInstallRepair_20260915/`.

## Native installation verification

Ran the real C: editor against WYRMFALL with a command-line-only plugin enable:

```powershell
& 'C:\Program Files\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'G:\assets\voxel project\WYRMFALL.uproject' -EnablePlugins=GeoForgeRuntime -unattended -nop4 -nosplash -nosound -nullrhi '-ExecutePythonScript=G:/assets/voxel project/Saved/Diagnostics/probe_geoforge_load.py' -stdout -FullStdOutLogOutput
```

Executed through `wyrm_process.execute_logged`, label `geoforge-native-load`,
timeout 300 seconds. Process exit 0, report PASS. Loaded native class/default
objects for GeoForgeInfiniteTerrainActor, GeoForgeBlockDefinition, the editor's
GeoForgeTerrainBlockProfileFactory, and ProceduralMeshComponent. Instantiated
GeoForgeBlockDefinition and ProceduralMeshComponent. The log confirms all four
runtime/editor DLLs loaded from C:.

Reports: `Saved/Diagnostics/GeoForge_load_probe.json` and
`Saved/ScaffoldLogs/20260915T155218Z_15a09ff36fa3_geoforge-native-load.log/.json`.
The project descriptor was not changed to select a terrain authority. This
probe did not generate terrain. Editor-generated AndroidFileServer settings
were backed up and removed from DefaultEngine.ini to restore its prior contents.

## Validation boundary

| Check | Result |
|---|---|
| Actual C: UE version and complete installation manifest | PASS |
| Stale GeoForge registration backed up and removed | PASS |
| Launcher restart and C: engine registration | PASS |
| User retry after registration repair | FAILED — reported; Download Failed toast observed |
| Proposed C: cache write/read probe | PASS |
| Install retry after cache path change | PASS — complete C: manifest and plugin files |
| GeoForge runtime/editor/dependency native load | PASS |
| Build, automation, bootstrap, PIE after GeoForge installation | NOT_RUN |
| WP-01 terrain/provider/save/performance proofs | NOT_RUN |

No C++ or project descriptor changed during this repair. Earlier BOOT-01
results remain historical evidence, not proof of a future plugin installation.

**Next bounded task:** finish WP-00's exact vendor completion/collision/nav/save
and owner/content inspections, then one eligible WP-01 proof. Installation
availability is resolved; those implementation/gameplay gates remain open.
