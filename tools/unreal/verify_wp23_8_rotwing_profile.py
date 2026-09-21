"""Run the focused dragon-profile proof against Rotwing's distinct undead rig."""

from pathlib import Path

import unreal


root = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
template_path = root / "tools/unreal/verify_wp23_6_cogfang_profile.py"
source = template_path.read_text(encoding="utf-8")
replacements = (
    ("WP-23.6 Cogfang", "WP-23.8 Rotwing"),
    ("WP23_6_cogfang_profile_proof", "WP23_8_rotwing_profile_proof"),
    ("WP23_6/Cogfang/Steampunk_Dragon", "WP23_8/Rotwing/Zombie_Dragon"),
    ("Steampunk_Dragon", "Zombie_Dragon"),
    ("Cogfang", "Rotwing"),
    ("COG-", "ROT-"),
    ("(32.0, 38.0)", "(31.0, 37.0)"),
    ("(125.0, 165.0)", "(122.0, 162.0)"),
    ('"mount_offset": [0.0, 0.0, 170.0], "flight_speed": 1500.0',
     '"mount_offset": [0.0, 0.0, 165.0], "flight_speed": 1450.0'),
)
for old, new in replacements:
    if old not in source:
        raise RuntimeError(f"Rotwing proof template marker missing: {old}")
    source = source.replace(old, new)

exec(compile(source, str(template_path), "exec"), globals(), globals())
