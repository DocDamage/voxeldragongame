"""Run the focused dragon-profile proof against Ossuroth's distinct skull rig."""

from pathlib import Path

import unreal


root = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
template_path = root / "tools/unreal/verify_wp23_6_cogfang_profile.py"
source = template_path.read_text(encoding="utf-8")
replacements = (
    ("WP-23.6 Cogfang", "WP-23.9 Ossuroth"),
    ("WP23_6_cogfang_profile_proof", "WP23_9_ossuroth_profile_proof"),
    ("wp23_6_cogfang_profile_proof", "wp23_9_ossuroth_profile_proof"),
    ("WP23_6/Cogfang/Steampunk_Dragon", "WP23_9/Ossuroth/Skull_Dragon"),
    ("Steampunk_Dragon", "Skull_Dragon"),
    ("Cogfang", "Ossuroth"),
    ("COG-", "OSS-"),
    ("!= 34", "!= 38"),
    ("Expected 34 followers", "Expected 38 followers"),
    ("(32.0, 38.0)", "(33.0, 40.0)"),
    ("(125.0, 165.0)", "(128.0, 170.0)"),
    ('"mount_offset": [0.0, 0.0, 170.0], "flight_speed": 1500.0',
     '"mount_offset": [0.0, 0.0, 170.0], "flight_speed": 1400.0'),
)
for old, new in replacements:
    if old not in source:
        raise RuntimeError(f"Ossuroth proof template marker missing: {old}")
    source = source.replace(old, new)

exec(compile(source, str(template_path), "exec"), globals(), globals())
