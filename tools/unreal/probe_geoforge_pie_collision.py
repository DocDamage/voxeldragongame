"""Disposable PIE collision probe. Fresh editor only; no production map is saved.

Launch with the GeoForge plugin and Sync/Boundary/Refresh probe flags enabled.
Uses the existing completion fixture, runs actual PIE, then exits the editor.
This does not grant WRLD/G1 acceptance or create a gameplay state owner.
"""
import json
from pathlib import Path
import runpy
import time
import traceback
import unreal

_geo_root = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
_geo_nav = '-WyrmGeoForgeNavProbe' in unreal.SystemLibrary.get_command_line()
_geo_explicit_nav_refresh = '-WyrmGeoForgeExplicitNavRefresh' in unreal.SystemLibrary.get_command_line()
_geo_out = _geo_root / ('Saved/Diagnostics/GeoForge_PIE_explicit_nav_probe.json' if _geo_explicit_nav_refresh
                        else 'Saved/Diagnostics/GeoForge_PIE_nav_probe.json' if _geo_nav
                        else 'Saved/Diagnostics/GeoForge_PIE_collision_probe.json')
_geo_result = {'kind': 'native_pie_collision_probe_not_G1', 'status': 'STARTING',
               'engine': unreal.SystemLibrary.get_engine_version(), 'snapshots': []}
_geo_out.write_text(json.dumps(_geo_result, indent=2) + '\n')
_geo_fixture = runpy.run_path(str(_geo_root / 'tools/unreal/probe_geoforge_completion.py'))
if _geo_fixture['result']['status'] != 'REFRESH_MESH_CHANGED_NO_QUEUED_WORK':
    _geo_result['status'] = 'ERROR_EDITOR_FIXTURE'
    _geo_out.write_text(json.dumps(_geo_result, indent=2) + '\n')
    raise RuntimeError('Editor fixture failed; inspect GeoForge_completion_refresh_probe.json')
unreal.GeoForgeTerrainBlueprintLibrary.clear_terrain_saved_state(_geo_fixture['actor'])
if _geo_nav:
    _geo_fixture['actor'].set_editor_property('streamed_terrain_affects_navigation', True)
    _geo_volume = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).spawn_actor_from_class(
        unreal.NavMeshBoundsVolume, unreal.Vector(800, 800, 800))
    _geo_volume.set_actor_scale3d(unreal.Vector(30, 15, 15))
    _geo_origin, _geo_extent = _geo_volume.get_actor_bounds(False)
    _geo_result['nav_bounds_extent_cm'] = [_geo_extent.x, _geo_extent.y, _geo_extent.z]
    if min(_geo_extent.x, _geo_extent.y, _geo_extent.z) <= 0:
        _geo_result['status'] = 'ERROR_NAV_VOLUME_HAS_NO_BOUNDS'
        _geo_out.write_text(json.dumps(_geo_result, indent=2) + '\n')
        unreal.SystemLibrary.quit_editor()
        raise RuntimeError('Native volume factory did not create a bounded brush')
_geo_level = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
_geo_started = time.monotonic()
_geo_finished = False
_geo_stage = 'prepare'
_geo_nav_point = None
_geo_nav_requested_at = None
_geo_lower_nav_point = None
_geo_filled_nav_point = None
_geo_last_wait_write = -1


def _geo_write():
    _geo_out.write_text(json.dumps(_geo_result, indent=2) + '\n', encoding='utf-8')


def _geo_tick(delta):
    global _geo_finished, _geo_stage, _geo_nav_point, _geo_nav_requested_at
    global _geo_lower_nav_point, _geo_filled_nav_point, _geo_last_wait_write
    try:
        if _geo_finished:
            if not _geo_level.is_in_play_in_editor():
                unreal.unregister_slate_post_tick_callback(_geo_handle)
                unreal.SystemLibrary.quit_editor()
            return
        if time.monotonic() - _geo_started > 90:
            raise RuntimeError('Timed out waiting for PIE/navigation stage: ' + _geo_stage)
        world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
        if not world or not _geo_level.is_in_play_in_editor():
            return
        if _geo_nav and _geo_stage in ('baseline_nav', 'dug_nav', 'filled_nav'):
            context = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.GeoForgeInfiniteTerrainActor)[0]
            projected = unreal.WyrmTerrainDiagnostics.project_navigation_point(
                context, _geo_nav_point, unreal.Vector(40, 40, 60))
            building = unreal.WyrmTerrainDiagnostics.is_navigation_build_pending(context)
            _geo_result['nav_last_query'] = {'stage': _geo_stage,
                'projected': str(projected), 'building_or_locked': building,
                'elapsed_seconds': time.monotonic() - _geo_nav_requested_at}
            if not projected or building:
                wait_bucket = int(time.monotonic() - _geo_nav_requested_at) // 5
                if wait_bucket != _geo_last_wait_write:
                    _geo_last_wait_write = wait_bucket
                    _geo_write()
                return
            if _geo_stage == 'filled_nav':
                obsolete = unreal.WyrmTerrainDiagnostics.project_navigation_point(
                    context, _geo_lower_nav_point, unreal.Vector(40, 40, 60))
                _geo_result['old_lower_nav_after_refill'] = str(obsolete)
                if obsolete:
                    return
            _geo_result.setdefault('nav_observations', []).append({
                'stage': _geo_stage, 'target_cm': [_geo_nav_point.x, _geo_nav_point.y, _geo_nav_point.z],
                'projected_cm': [projected.x, projected.y, projected.z],
                'elapsed_seconds': time.monotonic() - _geo_nav_requested_at})
            if _geo_stage == 'dug_nav':
                terrain = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.GeoForgeInfiniteTerrainActor)[0]
                terrain.add_sphere(unreal.Vector(1550, 850, 750), 250)
                terrain.refresh_loaded_chunk_visuals()
                if _geo_explicit_nav_refresh:
                    _geo_result['nav_refresh_submissions'].append(
                        unreal.WyrmTerrainDiagnostics.refresh_navigation_data_for_actor(terrain))
                _geo_lower_nav_point = _geo_nav_point
                _geo_nav_point = _geo_filled_nav_point
                _geo_stage = 'filled_nav'
                _geo_nav_requested_at = time.monotonic()
                _geo_write()
                return
            if _geo_stage == 'filled_nav':
                _geo_result['status'] = 'PASS_SCOPED_PIE_COLLISION_AND_NEW_SURFACE_NAV_PROJECTION'
                _geo_result['limitations'] += ' Navigation projection only; no route traversal or stale-path cancellation proof.'
                _geo_write()
                _geo_finished = True
                _geo_level.editor_request_end_play()
                return
            _geo_stage = 'collision'
        actors = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.GeoForgeInfiniteTerrainActor)
        if len(actors) != 1:
            raise RuntimeError('Expected exactly one PIE terrain actor, found ' + str(len(actors)))
        terrain = actors[0]
        _geo_result['world'] = world.get_path_name()
        _geo_result['actor'] = terrain.get_path_name()
        if _geo_nav:
            _geo_result['terrain_affects_navigation'] = terrain.get_editor_property('streamed_terrain_affects_navigation')
            _geo_result['nav_data'] = [{'path': data.get_path_name(),
                'runtime_generation': str(data.get_editor_property('runtime_generation'))}
                for data in unreal.GameplayStatics.get_all_actors_of_class(world, unreal.RecastNavMesh)]
        terrain.set_editor_property('use_async_chunk_generation', False)
        terrain.set_editor_property('max_queued_chunk_rebuilds_per_tick', 64)
        terrain.set_actor_tick_enabled(False)
        center = unreal.Vector(1550, 850, 750)
        terrain.prime_terrain_support_at_world_location(center, 1, 0)
        terrain.refresh_loaded_chunk_visuals()
        if _geo_explicit_nav_refresh:
            _geo_result.setdefault('nav_refresh_submissions', []).append(
                unreal.WyrmTerrainDiagnostics.refresh_navigation_data_for_actor(terrain))
        # Reuse instrumentation against the actual PIE object, not its editor copy.
        capture = _geo_fixture['snapshot']
        before = capture(terrain, 'pie_before_dig')
        if sum(before['queues'].values()) != 0:
            raise RuntimeError('PIE baseline did not settle')
        if _geo_nav and _geo_stage == 'prepare':
            _geo_nav_point = unreal.Vector(800, 850, before['vertical_trace']['impact_cm'][2])
            _geo_stage = 'baseline_nav'
            _geo_nav_requested_at = time.monotonic()
            _geo_result['status'] = 'WAITING_FOR_BASELINE_NAV'
            _geo_write()
            return
        terrain.dig_sphere(center, 250)
        terrain.prime_terrain_support_at_world_location(center, 1, 0)
        pending = capture(terrain, 'pie_dig_then_prime')
        terrain.refresh_loaded_chunk_visuals()
        dug = capture(terrain, 'pie_after_dig_refresh')
        terrain.add_sphere(center, 250)
        terrain.refresh_loaded_chunk_visuals()
        filled = capture(terrain, 'pie_after_fill_refresh')
        _geo_result['snapshots'] = [before, pending, dug, filled]
        valid = all(row['vertical_trace']['blocking_hit'] for row in (before, dug, filled))
        valid = valid and all(sum(row['queues'].values()) == 0 for row in (before, dug, filled))
        valid = valid and dug['vertical_trace']['impact_cm'][2] < before['vertical_trace']['impact_cm'][2] - 100
        valid = valid and filled['vertical_trace']['impact_cm'][2] > dug['vertical_trace']['impact_cm'][2] + 100
        for index in (1, 2):
            b, d, f = (row['vertical_traces'][index] for row in (before, dug, filled))
            valid = valid and b['blocking_hit'] and d['blocking_hit'] and f['blocking_hit']
            valid = valid and d['impact_cm'][2] < b['impact_cm'][2] - 100 and f['impact_cm'][2] > d['impact_cm'][2] + 100
        _geo_result['status'] = 'PASS_SCOPED_PIE_DIG_FILL_COLLISION' if valid else 'FAILED_COLLISION_ASSERTIONS'
        _geo_result['limitations'] = 'Fixed three-chunk synchronous fixture; tick disabled during same-frame operations. No nav, traversal, stress, inventory or game-save acceptance.'
        if valid and _geo_nav:
            terrain.dig_sphere(center, 250)
            terrain.refresh_loaded_chunk_visuals()
            if _geo_explicit_nav_refresh:
                _geo_result.setdefault('nav_refresh_submissions', []).append(
                    unreal.WyrmTerrainDiagnostics.refresh_navigation_data_for_actor(terrain))
            _geo_nav_point = unreal.Vector(*dug['vertical_trace']['impact_cm'])
            # Refill need not leave a walkable mound top. Check a surviving
            # ordinary floor and require the obsolete lower surface to vanish.
            _geo_filled_nav_point = unreal.Vector(800, 850, before['vertical_trace']['impact_cm'][2])
            _geo_stage = 'dug_nav'
            _geo_nav_requested_at = time.monotonic()
            _geo_result['status'] = 'WAITING_FOR_NEW_SURFACE_NAV'
            _geo_write()
            return
        _geo_write()
        _geo_finished = True
        _geo_level.editor_request_end_play()
    except Exception:
        _geo_result['status'] = 'ERROR'
        _geo_result['error'] = traceback.format_exc()
        _geo_write()
        _geo_finished = True
        _geo_level.editor_request_end_play()


_geo_handle = unreal.register_slate_post_tick_callback(_geo_tick)
_geo_level.editor_request_begin_play()
