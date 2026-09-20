"""Run the existing focused dragon-profile proof against Grovemaw's distinct rig."""

from pathlib import Path

import unreal


root = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
template_path = root / "tools/unreal/verify_wp23_6_cogfang_profile.py"
source = template_path.read_text(encoding="utf-8")
replacements = (
    ("WP-23.6 Cogfang", "WP-23.3 Grovemaw"),
    ("WP23_6_cogfang_profile_proof", "WP23_3_grovemaw_profile_proof"),
    ("WP23_6/Cogfang/Steampunk_Dragon", "WP23_3/Grovemaw/Wooden_Dragon"),
    ("Steampunk_Dragon", "Wooden_Dragon"),
    ("Cogfang", "Grovemaw"),
    ("COG-", "GROVE-"),
    ("(32.0, 38.0)", "(29.0, 35.0)"),
    ("(125.0, 165.0)", "(118.0, 158.0)"),
    ('"mount_offset": [0.0, 0.0, 170.0], "flight_speed": 1500.0',
     '"mount_offset": [0.0, 0.0, 155.0], "flight_speed": 1550.0'),
)
for old, new in replacements:
    if old not in source:
        raise RuntimeError(f"Grovemaw proof template marker missing: {old}")
    source = source.replace(old, new)

exec(compile(source, str(template_path), "exec"), globals(), globals())
