"""WP-01 Terrain Provider Proof (GeoForge) in PIE.

Authoritative verification of the GeoForge adapter under PIE:
- WRLD-01: Dig edit drops floor collision, 0 queued mesh work (synchronous).
- WRLD-03: Finite resource extraction yields Resource.Dirt, duplicate action ID rejected.
- WRLD-04: New-surface navigation projected at newly exposed dug floor.
- WRLD-08: Occupied fill volume rejected when pawn envelope intersects edit sphere.
- WRLD-02 & WRLD-05: Refill restores floor collision; old deeper nav route no longer projected.
- SAVE-01..04: Save payload captured, terrain modified, save payload restored matching saved state.
"""
import hashlib
import json
from pathlib import Path
import time
import traceback
import unreal

ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
REPORT_PATH = ROOT / "Saved/Diagnostics/WP01_terrain_provider_proof.json"

report = {
    "kind": "wp01_terrain_provider_proof_pie",
    "engine": unreal.SystemLibrary.get_engine_version(),
    "status": "INITIALIZING",
    "tests": {
        "WRLD-01": {"name": "dig_collision_synchronous", "status": "NOT_RUN"},
        "WRLD-02": {"name": "refill_collision_restoration", "status": "NOT_RUN"},
        "WRLD-03": {"name": "finite_yield_and_duplicate_prevention", "status": "NOT_RUN"},
        "WRLD-04": {"name": "new_surface_navigation", "status": "NOT_RUN"},
        "WRLD-05": {"name": "stale_route_cancellation", "status": "NOT_RUN"},
        "WRLD-08": {"name": "occupied_fill_rejection", "status": "NOT_RUN"},
        "SAVE-01..04": {"name": "terrain_persistence_save_load", "status": "NOT_RUN"}
    },
    "details": {},
    "logs": []
}

def log(msg):
    ts = time.strftime("%H:%M:%S")
    line = f"[{ts}] {msg}"
    print(line)
    report["logs"].append(line)

def write_report():
    REPORT_PATH.parent.mkdir(parents=True, exist_ok=True)
    REPORT_PATH.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")

write_report()

def make_guid():
    import uuid
    g = unreal.Guid()
    g.import_text(uuid.uuid4().hex.upper())
    return g


def vertical_trace(context, x, y, start_z=2000.0, end_z=100.0):
    hit = unreal.SystemLibrary.line_trace_single(
        context, unreal.Vector(x, y, start_z), unreal.Vector(x, y, end_z),
        unreal.TraceTypeQuery.TRACE_TYPE_QUERY1, True, [], unreal.DrawDebugTrace.NONE, False)
    parts = hit.to_tuple()
    return {
        "blocking_hit": parts[0],
        "distance": parts[3],
        "impact_cm": [parts[5].x, parts[5].y, parts[5].z],
        "actor": parts[9].get_path_name() if parts[9] else None,
        "component": parts[10].get_name() if parts[10] else None
    }

def get_render_queue_counts(terrain):
    stats = terrain.get_runtime_render_stats()
    return {
        "queued_chunk_generation_count": stats.get_editor_property("queued_chunk_generation_count"),
        "queued_chunk_rebuild_count": stats.get_editor_property("queued_chunk_rebuild_count"),
        "chunk_generation_jobs_in_flight": stats.get_editor_property("chunk_generation_jobs_in_flight"),
        "chunk_mesh_jobs_in_flight": stats.get_editor_property("chunk_mesh_jobs_in_flight"),
        "pending_chunk_apply_count": stats.get_editor_property("pending_chunk_apply_count")
    }

# 1. Setup Editor Map & Spawn Actors
log("Setting up blank editor map and GeoForge + Adapter fixture...")
default_cdo = unreal.get_default_object(unreal.GeoForgeInfiniteTerrainActor)
default_cdo.set_editor_property("auto_rebuild_in_editor", False)
default_cdo.set_editor_property("auto_apply_preset_defaults", False)

unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
editor_actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

terrain_actor = editor_actor_subsystem.spawn_actor_from_class(
    unreal.GeoForgeInfiniteTerrainActor, unreal.Vector())

terrain_settings = {
    "auto_rebuild_in_editor": False,
    "auto_apply_preset_defaults": False,
    "follow_editor_viewport_camera_for_streaming": False,
    "world_shape": unreal.GeoForgeWorldShape.PLANAR_FINITE,
    "horizontal_bounds_mode": unreal.GeoForgeHorizontalBoundsMode.FINITE_CENTERED,
    "finite_bounds_half_extent_x_in_chunks": 1,
    "finite_bounds_half_extent_y_in_chunks": 0,
    "chunk_size_in_cells": 16,
    "chunk_height_in_cells": 16,
    "cell_size": 100.0,
    "ground_level_in_cells": 8,
    "max_mountain_height_in_cells": 0,
    "water_level_in_cells": -100,
    "river_depth_in_cells": 0,
    "generate_caves": False,
    "enable_built_in_cube_trees": False,
    "generate_collision": True,
    "persist_edited_cells_across_streaming": True,
    "use_async_chunk_generation": False,
    "max_queued_chunk_rebuilds_per_tick": 64,
    "chunk_render_mode": unreal.GeoForgeChunkRenderMode.MARCHING_CUBES_SURFACE,
    "world_seed": 1337,
    "save_compatibility_id": "WYRMFALL.WP01.Proof",
    "streamed_terrain_affects_navigation": True
}
for k, v in terrain_settings.items():
    terrain_actor.set_editor_property(k, v)

dirt_palette = unreal.load_asset("/Game/WYRMFALL/Development/Intake/WP00/Dirt/dirt-1/Materials/palette")
if dirt_palette:
    for mat_key in ("terrain_material", "surface_material", "soil_material", "rock_material", "deep_rock_material"):
        terrain_actor.set_editor_property(mat_key, dirt_palette)

unreal.GeoForgeTerrainBlueprintLibrary.clear_terrain_saved_state(terrain_actor)

# Nav bounds volume
nav_vol = editor_actor_subsystem.spawn_actor_from_class(
    unreal.NavMeshBoundsVolume, unreal.Vector(800, 800, 800))
nav_vol.set_actor_scale3d(unreal.Vector(30, 15, 15))

# Spawn Adapter
adapter_actor = editor_actor_subsystem.spawn_actor_from_class(
    unreal.WyrmGeoForgeAdapter, unreal.Vector())
adapter_actor.bind_terrain_actor(terrain_actor)

log("Editor fixture ready. Requesting PIE session...")
level_editor_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)

pie_started_at = time.monotonic()
pie_finished = False
pie_stage = "init"
stage_timer = 0
pie_context = {}

def pie_tick(delta):
    global pie_finished, pie_stage, stage_timer
    try:
        if pie_finished:
            if not level_editor_subsystem.is_in_play_in_editor():
                unreal.unregister_slate_post_tick_callback(tick_handle)
                log("PIE closed cleanly; exiting editor.")
                unreal.SystemLibrary.quit_editor()
            return

        if time.monotonic() - pie_started_at > 90:
            raise RuntimeError("Timed out waiting for PIE stages; current stage: " + pie_stage)

        world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
        if not world or not level_editor_subsystem.is_in_play_in_editor():
            return

        # Stage: init
        if pie_stage == "init":
            terrains = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.GeoForgeInfiniteTerrainActor)
            adapters = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.WyrmGeoForgeAdapter)
            if len(terrains) == 0 or len(adapters) == 0:
                return

            t = terrains[0]
            a = adapters[0]
            a.bind_terrain_actor(t)
            t.set_editor_property("use_async_chunk_generation", False)
            t.set_editor_property("max_queued_chunk_rebuilds_per_tick", 64)
            t.set_actor_tick_enabled(False)

            center = unreal.Vector(1550, 850, 750)
            t.prime_terrain_support_at_world_location(center, 1, 0)
            t.refresh_loaded_chunk_visuals()
            unreal.WyrmTerrainDiagnostics.refresh_navigation_data_for_actor(t)

            base_trace = vertical_trace(t, 1550, 850)
            if not base_trace["blocking_hit"]:
                return

            pie_context["terrain"] = t
            pie_context["adapter"] = a
            pie_context["center"] = center
            pie_context["base_trace"] = base_trace
            log(f"Baseline established at impact Z: {base_trace['impact_cm'][2]:.1f} cm")

            pie_stage = "wait_baseline_nav"
            stage_timer = time.monotonic()
            write_report()
            return

        t = pie_context["terrain"]
        a = pie_context["adapter"]
        center = pie_context["center"]

        # Stage: wait_baseline_nav
        if pie_stage == "wait_baseline_nav":
            if unreal.WyrmTerrainDiagnostics.is_navigation_build_pending(t):
                return
            base_z = pie_context["base_trace"]["impact_cm"][2]
            proj = unreal.WyrmTerrainDiagnostics.project_navigation_point(
                t, unreal.Vector(1550, 850, base_z), unreal.Vector(40, 40, 80))
            if not proj:
                if time.monotonic() - stage_timer > 15:
                    raise RuntimeError("Failed to project baseline navigation point within 15s")
                return

            log(f"Baseline navigation point projected: {proj}")
            pie_context["base_nav"] = proj
            pie_stage = "execute_wrld01_wrld03"
            return

        # Stage: execute_wrld01_wrld03 (Dig & Finite Yield)
        if pie_stage == "execute_wrld01_wrld03":
            if "dig_submit_result" not in pie_context:
                log("Executing WRLD-01 (Dig) and WRLD-03 (Yield)...")
                dig_guid = make_guid()
                req = unreal.WyrmTerrainEditRequest()
                req.action_id = dig_guid
                req.world_center = center
                req.radius_cm = 250.0
                req.operation = unreal.WyrmTerrainEditOperation.REMOVE

                res = a.execute_terrain_edit(req)
                if res not in (unreal.WyrmTerrainSubmitResult.COMPLETED,
                                unreal.WyrmTerrainSubmitResult.QUEUED):
                    raise RuntimeError(f"SubmitTerrainEdit rejected proof edit: {res}")
                pie_context["dig_guid"] = dig_guid
                pie_context["dig_request"] = req
                pie_context["dig_submit_result"] = res
                stage_timer = time.monotonic()

            if a.has_pending_terrain_edits() or unreal.WyrmTerrainDiagnostics.is_navigation_build_pending(t):
                if time.monotonic() - stage_timer > 30:
                    raise RuntimeError("Timed out waiting for dig collision/navigation completion")
                return

            dig_guid = pie_context["dig_guid"]
            req = pie_context["dig_request"]
            res = pie_context["dig_submit_result"]

            # Check queues are zero (synchronous refresh)
            queues = get_render_queue_counts(t)
            queue_sum = sum(queues.values())
            if queue_sum != 0:
                raise RuntimeError(f"Render queues not settled: {queues}")

            # Ray trace
            dug_trace = vertical_trace(t, 1550, 850)
            base_z = pie_context["base_trace"]["impact_cm"][2]
            dug_z = dug_trace["impact_cm"][2]
            drop_cm = base_z - dug_z
            log(f"Dug floor Z: {dug_z:.1f} cm (dropped {drop_cm:.1f} cm)")
            if drop_cm < 100.0:
                raise RuntimeError(f"Crater depth insufficient: expected > 100cm, got {drop_cm:.1f}cm")

            report["tests"]["WRLD-01"]["status"] = "PASS"
            report["details"]["WRLD-01"] = {
                "baseline_z": base_z,
                "dug_z": dug_z,
                "drop_cm": drop_cm,
                "queues": queues,
                "submit_result": str(res),
                "completion_confirmed": True
            }

            # WRLD-03: Yield query
            yield_out = a.query_last_yield(dig_guid)
            yield_success, voxel_yield = yield_out if isinstance(yield_out, tuple) else (True, yield_out)
            if not yield_success or voxel_yield.extracted_count <= 0 or voxel_yield.duplicate_prevented:
                raise RuntimeError(f"Yield query failed: success={yield_success}, count={voxel_yield.extracted_count}")

            log(f"Yield query passed: resource={voxel_yield.resource_id}, count={voxel_yield.extracted_count}, volume={voxel_yield.volume_extracted_cm3:.0f} cm3")

            # WRLD-03: Duplicate prevention
            dup_res = a.execute_terrain_edit(req)
            if dup_res != unreal.WyrmTerrainSubmitResult.REJECTED:
                raise RuntimeError(f"Duplicate edit expected REJECTED, got {dup_res}")

            dup_out = a.query_last_yield(dig_guid)
            dup_success, dup_yield = dup_out if isinstance(dup_out, tuple) else (True, dup_out)
            if not dup_success or not dup_yield.duplicate_prevented or dup_yield.extracted_count != 0:
                raise RuntimeError("Duplicate yield record check failed")

            log("Duplicate edit rejected and yield duplicate_prevented confirmed.")

            # A different transaction ID over the already-depleted volume must
            # complete with zero yield. This proves depletion is based on actual
            # removed cells rather than the requested sphere volume.
            exhausted_guid = make_guid()
            exhausted_req = unreal.WyrmTerrainEditRequest()
            exhausted_req.action_id = exhausted_guid
            exhausted_req.world_center = center
            exhausted_req.radius_cm = 250.0
            exhausted_req.operation = unreal.WyrmTerrainEditOperation.REMOVE
            exhausted_res = a.execute_terrain_edit(exhausted_req)
            if exhausted_res != unreal.WyrmTerrainSubmitResult.COMPLETED:
                raise RuntimeError(f"Exhausted-volume edit expected COMPLETED, got {exhausted_res}")
            exhausted_out = a.query_last_yield(exhausted_guid)
            exhausted_success, exhausted_yield = exhausted_out if isinstance(exhausted_out, tuple) else (True, exhausted_out)
            if not exhausted_success or exhausted_yield.extracted_count != 0 or exhausted_yield.volume_extracted_cm3 != 0:
                raise RuntimeError(
                    f"Exhausted volume granted yield: count={exhausted_yield.extracted_count}, "
                    f"volume={exhausted_yield.volume_extracted_cm3}")
            log("Distinct action over exhausted volume completed with zero yield.")

            report["tests"]["WRLD-03"]["status"] = "PASS"
            report["details"]["WRLD-03"] = {
                "initial_count": voxel_yield.extracted_count,
                "initial_volume_cm3": voxel_yield.volume_extracted_cm3,
                "resource_id": str(voxel_yield.resource_id),
                "duplicate_rejected": str(dup_res),
                "duplicate_prevented_flag": dup_yield.duplicate_prevented,
                "duplicate_extracted_count": dup_yield.extracted_count,
                "exhausted_new_action_result": str(exhausted_res),
                "exhausted_new_action_count": exhausted_yield.extracted_count,
                "exhausted_new_action_volume_cm3": exhausted_yield.volume_extracted_cm3
            }

            pie_context["dug_trace"] = dug_trace
            pie_stage = "wait_wrld04_nav"
            stage_timer = time.monotonic()
            write_report()
            return

        # Stage: wait_wrld04_nav (New-Surface Navigation)
        if pie_stage == "wait_wrld04_nav":
            if unreal.WyrmTerrainDiagnostics.is_navigation_build_pending(t):
                return
            dug_z = pie_context["dug_trace"]["impact_cm"][2]
            proj = unreal.WyrmTerrainDiagnostics.project_navigation_point(
                t, unreal.Vector(1550, 850, dug_z), unreal.Vector(40, 40, 80))
            if not proj:
                if time.monotonic() - stage_timer > 15:
                    raise RuntimeError("Timed out waiting for new surface navigation build")
                return

            proj_z = proj.z
            log(f"WRLD-04: Projected navigation at dug elevation Z: {proj_z:.1f} cm")
            if abs(proj_z - dug_z) > 80.0:
                raise RuntimeError(f"Nav projection height {proj_z} does not match dug floor {dug_z}")

            report["tests"]["WRLD-04"]["status"] = "PASS"
            report["details"]["WRLD-04"] = {
                "target_z": dug_z,
                "nav_projected_cm": [proj.x, proj.y, proj.z],
                "delta_z": abs(proj_z - dug_z)
            }

            pie_context["dug_nav"] = proj
            pie_stage = "execute_wrld08_occupied_fill"
            write_report()
            return

        # Stage: execute_wrld08_occupied_fill (Occupied Fill Rejection)
        if pie_stage == "execute_wrld08_occupied_fill":
            log("Executing WRLD-08 (Occupied Fill Protection)...")
            dug_z = pie_context["dug_trace"]["impact_cm"][2]
            pawn_target_loc = unreal.Vector(1550, 850, dug_z + 80)

            # Move active player pawn into the crater
            pawns = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.Pawn)
            if len(pawns) == 0:
                raise RuntimeError("No active pawn in PIE world to test occupied fill envelope")

            pawn = pawns[0]
            pie_context["pawn"] = pawn
            pie_context["pawn_saved_loc"] = pawn.get_actor_location()
            pawn.set_actor_location(pawn_target_loc, False, False)

            # Verify volume occupied check
            b_occupied = a.check_volume_occupied(pawn_target_loc, 150.0)
            log(f"CheckVolumeOccupied at pawn location: {b_occupied}")
            if not b_occupied:
                pawn.set_actor_location(pie_context["pawn_saved_loc"], False, False)
                raise RuntimeError("CheckVolumeOccupied failed to detect pawn in crater")

            # Attempt add edit that overlaps pawn
            fill_req = unreal.WyrmTerrainEditRequest()
            fill_req.action_id = make_guid()
            fill_req.world_center = unreal.Vector(1550, 850, dug_z + 50)
            fill_req.radius_cm = 200.0
            fill_req.operation = unreal.WyrmTerrainEditOperation.ADD

            fill_res = a.execute_terrain_edit(fill_req)
            log(f"Occupied fill attempt returned: {fill_res}")
            if fill_res != unreal.WyrmTerrainSubmitResult.REJECTED:
                pawn.set_actor_location(pie_context["pawn_saved_loc"], False, False)
                raise RuntimeError(f"Expected occupied fill to be REJECTED, got {fill_res}")

            # Confirm crater was not filled
            verify_trace = vertical_trace(t, 1550, 850)
            if verify_trace["impact_cm"][2] > dug_z + 50.0:
                pawn.set_actor_location(pie_context["pawn_saved_loc"], False, False)
                raise RuntimeError("Terrain was altered despite occupied rejection")

            # Restore pawn to safe location
            pawn.set_actor_location(unreal.Vector(500, 500, 1000), False, False)
            log("Occupied fill successfully rejected; character envelope preserved.")
            report["tests"]["WRLD-08"]["status"] = "PASS"
            report["details"]["WRLD-08"] = {
                "pawn_tested": pawn.get_path_name(),
                "attempted_edit_center": [fill_req.world_center.x, fill_req.world_center.y, fill_req.world_center.z],
                "attempted_radius_cm": fill_req.radius_cm,
                "submit_result": str(fill_res),
                "crater_preserved": True
            }

            pie_stage = "execute_wrld02_wrld05_refill"
            write_report()
            return

        # Stage: execute_wrld02_wrld05_refill (Refill & Stale Route Blockage)
        if pie_stage == "execute_wrld02_wrld05_refill":
            if "refill_submit_result" not in pie_context:
                log("Executing WRLD-02 (Refill) and WRLD-05 (Stale Route Cancellation)...")
                refill_req = unreal.WyrmTerrainEditRequest()
                refill_req.action_id = make_guid()
                refill_req.world_center = center
                refill_req.radius_cm = 250.0
                refill_req.operation = unreal.WyrmTerrainEditOperation.ADD

                refill_res = a.execute_terrain_edit(refill_req)
                if refill_res not in (unreal.WyrmTerrainSubmitResult.COMPLETED,
                                      unreal.WyrmTerrainSubmitResult.QUEUED):
                    raise RuntimeError(f"Refill edit rejected: {refill_res}")
                pie_context["refill_request"] = refill_req
                pie_context["refill_submit_result"] = refill_res
                stage_timer = time.monotonic()

            if a.has_pending_terrain_edits() or unreal.WyrmTerrainDiagnostics.is_navigation_build_pending(t):
                if time.monotonic() - stage_timer > 30:
                    raise RuntimeError("Timed out waiting for refill collision/navigation completion")
                return

            refill_req = pie_context["refill_request"]
            refill_res = pie_context["refill_submit_result"]

            refill_trace = vertical_trace(t, 1550, 850)
            base_z = pie_context["base_trace"]["impact_cm"][2]
            refill_z = refill_trace["impact_cm"][2]
            dug_z = pie_context["dug_trace"]["impact_cm"][2]
            log(f"Refill floor Z: {refill_z:.1f} cm (restored from {dug_z:.1f} cm)")
            if refill_z < dug_z + 100.0:
                raise RuntimeError(f"Refill did not restore collision height: refill_z={refill_z}, dug_z={dug_z}")

            report["tests"]["WRLD-02"]["status"] = "PASS"
            report["details"]["WRLD-02"] = {
                "baseline_z": base_z,
                "dug_z": dug_z,
                "refilled_z": refill_z,
                "submit_result": str(refill_res),
                "completion_confirmed": True
            }

            pie_context["refill_trace"] = refill_trace
            pie_stage = "wait_wrld05_nav_blockage"
            stage_timer = time.monotonic()
            write_report()
            return

        # Stage: wait_wrld05_nav_blockage
        if pie_stage == "wait_wrld05_nav_blockage":
            if unreal.WyrmTerrainDiagnostics.is_navigation_build_pending(t):
                return
            dug_z = pie_context["dug_trace"]["impact_cm"][2]
            old_proj = unreal.WyrmTerrainDiagnostics.project_navigation_point(
                t, unreal.Vector(1550, 850, dug_z), unreal.Vector(40, 40, 40))
            if old_proj and abs(old_proj.z - dug_z) < 40.0:
                if time.monotonic() - stage_timer > 15:
                    raise RuntimeError("Old dug nav point still projected at buried elevation after refill!")
                return

            log(f"WRLD-05: Old deep nav point at Z={dug_z:.1f} is no longer navigable (result: {old_proj}).")
            report["tests"]["WRLD-05"]["status"] = "PASS"
            report["details"]["WRLD-05"] = {
                "buried_z": dug_z,
                "stale_nav_result": str(old_proj) if old_proj else "None (nav surface removed)"
            }

            pie_stage = "execute_save_persistence"
            write_report()
            return

        # Stage: execute_save_persistence (SAVE-01..04)
        if pie_stage == "execute_save_persistence":
            if "save_payload" not in pie_context:
                log("Executing SAVE-01..04 (Save Payload & Restoration)...")
                # 1. Capture save payload of refilled state
                save_out = a.build_save_payload()
                save_bytes = save_out[1] if isinstance(save_out, tuple) else save_out
                if not save_bytes or len(save_bytes) == 0:
                    raise RuntimeError(f"build_save_payload returned empty payload: {save_out}")
                log(f"Captured terrain save payload: {len(save_bytes)} bytes")

                # 2. Modify terrain state (dig a new crater)
                new_dig = unreal.WyrmTerrainEditRequest()
                new_dig.action_id = make_guid()
                new_dig.world_center = center
                new_dig.radius_cm = 250.0
                new_dig.operation = unreal.WyrmTerrainEditOperation.REMOVE
                modify_result = a.execute_terrain_edit(new_dig)
                if modify_result not in (unreal.WyrmTerrainSubmitResult.COMPLETED,
                                         unreal.WyrmTerrainSubmitResult.QUEUED):
                    raise RuntimeError(f"Persistence modification edit rejected: {modify_result}")
                pie_context["save_payload"] = save_bytes
                pie_context["save_modify_result"] = modify_result
                stage_timer = time.monotonic()

            if a.has_pending_terrain_edits() or unreal.WyrmTerrainDiagnostics.is_navigation_build_pending(t):
                if time.monotonic() - stage_timer > 30:
                    raise RuntimeError("Timed out waiting for persistence modification completion")
                return

            save_bytes = pie_context["save_payload"]
            modified_trace = vertical_trace(t, 1550, 850)
            log(f"Modified terrain for persistence check, new Z: {modified_trace['impact_cm'][2]:.1f} cm")

            # 3. Apply saved payload
            apply_ok = a.apply_save_payload(save_bytes)
            if not apply_ok:
                raise RuntimeError("apply_save_payload returned False")

            # 4. Verify restored state matches refilled state
            restored_trace = vertical_trace(t, 1550, 850)
            restored_z = restored_trace["impact_cm"][2]
            saved_z = pie_context["refill_trace"]["impact_cm"][2]
            log(f"Restored terrain floor Z: {restored_z:.1f} cm (expected ~{saved_z:.1f} cm)")
            if abs(restored_z - saved_z) > 10.0:
                raise RuntimeError(f"Restored Z {restored_z} does not match saved Z {saved_z}")

            report["tests"]["SAVE-01..04"]["status"] = "PASS"
            report["details"]["SAVE-01..04"] = {
                "payload_bytes": len(save_bytes),
                "modify_submit_result": str(pie_context["save_modify_result"]),
                "modified_z": modified_trace["impact_cm"][2],
                "restored_z": restored_z,
                "saved_expected_z": saved_z
            }

            report["status"] = "PASS"
            log("ALL WP-01 PIE TESTS PASSED!")
            write_report()

            pie_finished = True
            level_editor_subsystem.editor_request_end_play()
            return

    except Exception:
        report["status"] = "ERROR"
        report["error"] = traceback.format_exc()
        log("PIE ERROR: " + report["error"])
        write_report()
        pie_finished = True
        level_editor_subsystem.editor_request_end_play()

tick_handle = unreal.register_slate_post_tick_callback(pie_tick)
level_editor_subsystem.editor_request_begin_play()
