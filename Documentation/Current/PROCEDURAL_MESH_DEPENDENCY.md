# GeoForge prerequisite — ProceduralMeshComponent

**September 15, 2026:** the plugin is already bundled with the verified
`C:\Program Files\UE_5.8` installation. No separate download is required for
this engine. Version 1.0; runtime and editor DLLs have BuildId 55116800.

Actual descriptor:
`Engine/Plugins/Runtime/ProceduralMeshComponent/ProceduralMeshComponent.uplugin`.
It already declares `EnabledByDefault: true`. Its `Installed: false` field means
it was not installed on top of the engine; the installed `PluginDescriptor.h`
documents that meaning. It does not mean the bundled files are missing.

The previously inspected GeoForge 5.0.0 descriptor explicitly depends on
ProceduralMeshComponent. Added an explicit enabled entry to `WYRMFALL.uproject`.
No engine descriptor, global setting, C++ source or GeoForge installation was changed.

Validation:

- `py -3.12 tools/wyrm.py verify`: PASS source/config check.
- Actual C: `UnrealEditor-Cmd.exe` loaded both ProceduralMeshComponent DLLs and
  instantiated `/Script/ProceduralMeshComponent.ProceduralMeshComponent`: PASS.
  Invocation: project path, `-unattended -nop4 -nosplash -nosound -nullrhi`,
  `-ExecutePythonScript=G:/assets/voxel project/Saved/Diagnostics/probe_procedural_mesh.py`,
  `-stdout -FullStdOutLogOutput`; executed via `wyrm_process.execute_logged`.
- Evidence: `Saved/Diagnostics/ProceduralMeshComponent_probe.json`,
  `Saved/Diagnostics/ProceduralMeshComponent_files.json`, and
  `Saved/ScaffoldLogs/20260915T143318Z_7e93b9b7839b_procedural-mesh-load.log`.
- C++ rebuild, automation suite, PIE, cook and GeoForge/terrain validation for
  this descriptor-only change: **NOT_RUN**. Earlier BOOT-01 results remain
  historical evidence of their recorded configuration.

Subsequent recovery: GeoForge 5.0.0 is now installed in the C: UE 5.8 engine,
and a separate native probe loaded its runtime/editor modules together with
ProceduralMeshComponent. See [installation recovery](GEOFORGE_INSTALL_RECOVERY.md).
Terrain/gameplay validation remains NOT_RUN.
