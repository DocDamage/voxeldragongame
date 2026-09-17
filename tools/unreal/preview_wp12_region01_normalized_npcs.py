"""Launch the normalized WP-12 developer fixture through one console command.

Unreal's ``ExecCmds`` parser splits semicolons and commas before it forwards a
``py`` expression. This wrapper retains the known-good ``py exec(open(...))``
startup shape and supplies the normalized mode only after Python is running.
"""

from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
SOURCE = ROOT / "tools" / "unreal" / "preview_wp12_region01_npcs.py"
exec(compile(SOURCE.read_text(encoding="utf-8"), SOURCE.as_posix(), "exec"), {"WP12_USE_NORMALIZED": True})
