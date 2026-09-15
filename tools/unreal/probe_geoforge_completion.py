"""Disposable native WP-00 contract probe, not a G1 or PIE acceptance test.

Run with UnrealEditor-Cmd -EnablePlugins=GeoForgeRuntime -ExecutePythonScript.
Creates an unsaved map; never run inside an editor with unsaved user work.
Only project-owned instrumentation is tracked; no vendor source is copied.
"""
import hashlib
import json
from pathlib import Path
import traceback
import unreal

ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
SYNC_CONTROL = '-WyrmGeoForgeSyncProbe' in unreal.SystemLibrary.get_command_line()
BOUNDARY_PROBE = '-WyrmGeoForgeBoundaryProbe' in unreal.SystemLibrary.get_command_line()
REPORT = ROOT / ('Saved/Diagnostics/GeoForge_completion_boundary_probe.json' if BOUNDARY_PROBE
                 else 'Saved/Diagnostics/GeoForge_completion_sync_control.json' if SYNC_CONTROL
                 else 'Saved/Diagnostics/GeoForge_completion_probe.json')
result = {'kind': 'native_editor_contract_probe_not_gameplay_acceptance',
          'engine': unreal.SystemLibrary.get_engine_version(), 'status': 'ERROR',
          'pie': 'NOT_RUN', 'sync_control': SYNC_CONTROL,
          'boundary_probe': BOUNDARY_PROBE, 'snapshots': []}


def snapshot(actor, label):
    stats = actor.get_runtime_render_stats()
    counts = {key: stats.get_editor_property(key) for key in (
        'queued_chunk_generation_count', 'queued_chunk_rebuild_count',
        'chunk_generation_jobs_in_flight', 'chunk_mesh_jobs_in_flight',
        'pending_chunk_apply_count')}
    meshes = []
    for component in actor.get_components_by_class(unreal.ProceduralMeshComponent):
        sections = []
        for index in range(component.get_num_sections()):
            vertices, triangles, normals, uvs, tangents = unreal.ProceduralMeshLibrary.get_section_from_procedural_mesh(component, index)
            sections.append({'vertices': [[v.x, v.y, v.z] for v in vertices],
                             'triangles': list(triangles)})
        meshes.append({'name': component.get_name(),
                       'async_cooking': component.get_editor_property('use_async_cooking'),
                       'sections': sections})
    meshes.sort(key=lambda m: m['name'])
    data = unreal.GeoForgeTerrainBlueprintLibrary.build_terrain_save_data(actor)
    row = {'label': label, 'queues': counts, 'saved_cells': len(data.saved_cells),
           'mesh_sha256': hashlib.sha256(json.dumps(meshes, sort_keys=True).encode()).hexdigest(),
           'vertices': sum(len(s['vertices']) for m in meshes for s in m['sections']),
           'mesh_components': len(meshes),
           'async_components': sum(m['async_cooking'] for m in meshes)}
    result['snapshots'].append(row)
    return row


try:
    # Avoid the vendor default constructor's large editor preview. These are
    # process-local CDO settings, never saved to the plugin or project.
    default = unreal.get_default_object(unreal.GeoForgeInfiniteTerrainActor)
    default.set_editor_property('auto_rebuild_in_editor', False)
    default.set_editor_property('auto_apply_preset_defaults', False)
    unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
    actor = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).spawn_actor_from_class(
        unreal.GeoForgeInfiniteTerrainActor, unreal.Vector())
    settings = {
        'auto_rebuild_in_editor': False, 'auto_apply_preset_defaults': False,
        'follow_editor_viewport_camera_for_streaming': False,
        'world_shape': unreal.GeoForgeWorldShape.PLANAR_FINITE,
        'horizontal_bounds_mode': unreal.GeoForgeHorizontalBoundsMode.FINITE_CENTERED,
        'finite_bounds_half_extent_x_in_chunks': 1 if BOUNDARY_PROBE else 0,
        'finite_bounds_half_extent_y_in_chunks': 0,
        'chunk_size_in_cells': 16, 'chunk_height_in_cells': 16, 'cell_size': 100.0,
        'ground_level_in_cells': 8, 'max_mountain_height_in_cells': 0,
        'water_level_in_cells': -100, 'river_depth_in_cells': 0,
        'generate_caves': False, 'enable_built_in_cube_trees': False,
        'generate_collision': True, 'persist_edited_cells_across_streaming': True,
        'use_async_chunk_generation': not SYNC_CONTROL,
        'max_queued_chunk_rebuilds_per_tick': 64,
        'chunk_render_mode': unreal.GeoForgeChunkRenderMode.MARCHING_CUBES_SURFACE,
        'world_seed': 1337, 'save_compatibility_id': 'WYRMFALL.WP00.ContractProbe',
    }
    for key, value in settings.items():
        actor.set_editor_property(key, value)
    result['settings'] = {key: str(value) for key, value in settings.items()}
    material = unreal.load_asset('/Game/WYRMFALL/Development/Intake/WP00/Dirt/dirt-1/Materials/palette')
    if not material:
        raise RuntimeError('Required existing real dirt sample missing')
    for key in ('terrain_material', 'surface_material', 'soil_material', 'rock_material', 'deep_rock_material'):
        actor.set_editor_property(key, material)
    result['material'] = material.get_path_name()
    center = unreal.Vector(1550 if BOUNDARY_PROBE else 850, 850, 750)
    prime_radius = 1 if BOUNDARY_PROBE else 0
    result['edit_center_cm'] = [center.x, center.y, center.z]
    result['edit_radius_cm'] = 250
    actor.prime_terrain_support_at_world_location(center, prime_radius, 0)
    if BOUNDARY_PROBE and SYNC_CONTROL:
        # Finish initial neighbor seam work so it cannot explain edit queues.
        actor.refresh_loaded_chunk_visuals()
    before = snapshot(actor, 'before_dig')
    if sum(before['queues'].values()) != 0:
        raise RuntimeError('Baseline not quiescent; cannot attribute pending work to the edit')
    actor.dig_sphere(center, 250)
    edited = snapshot(actor, 'after_dig_before_prime')
    actor.prime_terrain_support_at_world_location(center, prime_radius, 0)
    primed = snapshot(actor, 'after_dig_and_prime_same_frame')
    reproduced = (before['vertices'] > 0 and edited['saved_cells'] > before['saved_cells']
                  and primed['mesh_sha256'] == before['mesh_sha256']
                  and sum(primed['queues'].values()) > 0)
    result['status'] = 'REPRODUCED_PRIME_IS_NOT_EDIT_COMPLETION_BARRIER' if reproduced else 'NOT_REPRODUCED'
    if BOUNDARY_PROBE and (before['vertices'] > 0 and edited['saved_cells'] > before['saved_cells']
                           and primed['mesh_sha256'] == edited['mesh_sha256']
                           and sum(primed['queues'].values()) > 0):
        result['status'] = 'REPRODUCED_PRIME_RETURNS_WITH_PENDING_EDIT_MESH_WORK'
    if SYNC_CONTROL and (before['vertices'] > 0 and edited['saved_cells'] > before['saved_cells']
                         and primed['mesh_sha256'] != before['mesh_sha256']
                         and sum(primed['queues'].values()) == 0):
        result['status'] = 'SYNC_CONTROL_MESH_CHANGED_NO_QUEUED_WORK'
    result['limitation'] = 'Same-frame native editor observation; no claim that queued work never completes, or that terrain gameplay/collision/nav passed.'
except Exception:
    result['error'] = traceback.format_exc()
finally:
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
    unreal.log('WYRMFALL GeoForge completion probe: ' + json.dumps(result))
