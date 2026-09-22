"""Run the focused dragon-profile proof against Frostmane's White Dragon rig."""

from pathlib import Path

import unreal


root = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
template_path = root / "tools/unreal/verify_wp23_6_cogfang_profile.py"
source = template_path.read_text(encoding="utf-8")
replacements = (
    ("WP-23.6 Cogfang", "WP-23.4 Frostmane"),
    ("WP23_6_cogfang_profile_proof", "WP23_4_frostmane_profile_proof"),
    ("wp23_6_cogfang_profile_proof", "wp23_4_frostmane_profile_proof"),
    ("WP23_6/Cogfang/Steampunk_Dragon", "WP23_4/Frostmane/White_Dragon"),
    ("Steampunk_Dragon", "White_Dragon"),
    ("Cogfang", "Frostmane"),
    ("COG-", "FRM-"),
    ("!= 34", "!= 36"),
    ("Expected 34 followers", "Expected 36 followers"),
    ("(32.0, 38.0)", "(30.0, 36.0)"),
    ("(125.0, 165.0)", "(120.0, 160.0)"),
    ('"mount_offset": [0.0, 0.0, 170.0], "flight_speed": 1500.0',
     '"mount_offset": [0.0, 0.0, 160.0], "flight_speed": 1500.0'),
)
for old, new in replacements:
    if old not in source:
        raise RuntimeError(f"Frostmane proof template marker missing: {old}")
    source = source.replace(old, new)

exec(compile(source, str(template_path), "exec"), globals(), globals())
