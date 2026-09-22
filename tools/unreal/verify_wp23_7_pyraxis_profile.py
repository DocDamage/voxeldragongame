"""Run the focused dragon-profile PIE proof against Pyraxis's Lava Dragon rig."""

from pathlib import Path

import unreal


root = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
template_path = root / "tools/unreal/verify_wp23_6_cogfang_profile.py"
source = template_path.read_text(encoding="utf-8")
replacements = (
    ("WP-23.6 Cogfang", "WP-23.7 Pyraxis"),
    ("WP23_6_cogfang_profile_proof", "WP23_7_pyraxis_profile_proof"),
    ("wp23_6_cogfang_profile_proof", "wp23_7_pyraxis_profile_proof"),
    ("WP23_6/Cogfang/Steampunk_Dragon", "WP23_7/Pyraxis/Lava_Dragon"),
    ("Steampunk_Dragon", "Lava_Dragon"),
    ("Cogfang", "Pyraxis"),
    ("COG-", "PYR-"),
    ("(32.0, 38.0)", "(34.0, 39.0)"),
    ("(125.0, 165.0)", "(132.0, 175.0)"),
    ('"mount_offset": [0.0, 0.0, 170.0], "flight_speed": 1500.0',
     '"mount_offset": [0.0, 0.0, 175.0], "flight_speed": 1550.0'),
)
for old, new in replacements:
    if old not in source:
        raise RuntimeError(f"Pyraxis proof template marker missing: {old}")
    source = source.replace(old, new)

exec(compile(source, str(template_path), "exec"), globals(), globals())
