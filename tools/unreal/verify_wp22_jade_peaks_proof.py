"""Focused live-PIE acceptance proof for the bounded WP-22 Jade Peaks slice."""

import json
import time
import traceback
from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
REPORT_PATH = ROOT / "Saved/Diagnostics/WP22_jade_peaks_proof.json"
MAP_PACKAGE = "/Game/WYRMFALL/World/Regions/L_JadePeaks"
SLOT = "WP22_JadePeaks_Proof"
TEST_NAMES = (
    "JP-01.RegionRuntime", "JP-02.RealPresentation", "JP-03.LandmarkRoute",
    "JP-04.JadefangContinuity", "JP-05.MirrorStep", "JP-06.UnifiedPersistence",
)
report = {
    "kind": "wp22_jade_peaks_proof",
    "engine": unreal.SystemLibrary.get_engine_version(),
    "map": MAP_PACKAGE,
    "status": "INITIALIZING",
    "tests": {name: {"status": "NOT_RUN"} for name in TEST_NAMES},
    "details": {}, "logs": [],
}


def log(message):
    line = f"[{time.strftime('%H:%M:%S')}] {message}"
    print(line)
    report["logs"].append(line)


def write_report():
    REPORT_PATH.parent.mkdir(parents=True, exist_ok=True)
    REPORT_PATH.write_text(json.dumps(report, indent=2, default=str) + "\n", encoding="utf-8")


def finish(name, details):
    report["tests"][name]["status"] = "PASS"
    report["details"][name] = details
    log(f"Test {name}: PASS")


def actors(world, cls):
    return unreal.GameplayStatics.get_all_actors_of_class(world, cls)


def first(world, cls):
    found = actors(world, cls)
    return found[0] if found else None


def surface_hit(world, x, y, ignored=None):
    hit = unreal.SystemLibrary.line_trace_single(
        world, unreal.Vector(x, y, 3000), unreal.Vector(x, y, -1000),
        unreal.TraceTypeQuery.TRACE_TYPE_QUERY1, True, ignored or [],
        unreal.DrawDebugTrace.NONE, False)
    if hit is None:
        return None
    parts = hit.to_tuple()
    if not parts[0]:
        return None
    return {
        "location": parts[5],
        "actor": parts[9].get_path_name() if parts[9] else None,
        "component": parts[10].get_name() if parts[10] else None,
    }


def make_editor_blocker(editor_actors, location, scale, label):
    actor = editor_actors.spawn_actor_from_class(unreal.StaticMeshActor, location, unreal.Rotator(0, 0, 0))
    actor.static_mesh_component.set_static_mesh(unreal.load_asset("/Engine/BasicShapes/Cube.Cube"))
    actor.static_mesh_component.set_collision_profile_name("BlockAll")
    actor.set_actor_scale3d(scale)
    actor.set_actor_label(label)
    return actor


write_report()
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
editor = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
current = editor.get_editor_world()
if not current or MAP_PACKAGE not in current.get_path_name():
    if not levels.load_level(MAP_PACKAGE):
        raise RuntimeError(f"Could not load {MAP_PACKAGE}")

# Simulate PIE does not ask GameMode to create a default pawn. Older fixtures
# already contain one; this production map intentionally uses PlayerStart, so a
# transient editor-world pawn is duplicated into PIE for the focused proof.
editor_actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
editor_nav = unreal.NavigationSystemV1.get_navigation_system(editor.get_editor_world())
nav_bounds = [actor for actor in editor_actors.get_all_level_actors()
              if isinstance(actor, unreal.NavMeshBoundsVolume)]
assert editor_nav and nav_bounds, "editor world must have navigation system and bounds"
# A session-local bounds registration makes Simulate PIE create/duplicate the
# dynamic Recast actor. The saved production map still owns its real bounds.
proof_nav_bound = editor_actors.spawn_actor_from_class(
    unreal.NavMeshBoundsVolume, unreal.Vector(0, 0, 900), unreal.Rotator(0, 0, 0))
proof_nav_bound.set_actor_scale3d(unreal.Vector(100, 100, 30))
proof_nav_bound.set_actor_label("DIAG_WP22_NavRegistration")
nav_bounds.append(proof_nav_bound)
for nav_bound in nav_bounds:
    editor_nav.on_navigation_bounds_updated(nav_bound)
if not editor_actors.get_all_level_actors() or not any(
        isinstance(actor, unreal.WyrmCharacter) for actor in editor_actors.get_all_level_actors()):
    proof_player = editor_actors.spawn_actor_from_class(
        unreal.WyrmCharacter, unreal.Vector(-4000, -2500, 996), unreal.Rotator(0, 0, 0))
    proof_player.set_actor_label("DIAG_WP22_ProofPlayer")
make_editor_blocker(editor_actors, unreal.Vector(-3850, -2500, 1100), unreal.Vector(0.1, 2.0, 2.0), "DIAG_WP22_SealedWall")
make_editor_blocker(editor_actors, unreal.Vector(-4000, -2250, 920), unreal.Vector(0.7, 0.7, 0.2), "DIAG_WP22_OccupiedLow")
proof_water = editor_actors.spawn_actor_from_class(
    unreal.WyrmWaterVolume, unreal.Vector(-4250, -2500, 900), unreal.Rotator(0, 0, 0))
proof_water.set_actor_scale3d(unreal.Vector(0.1, 0.1, 0.1))
proof_water.surface_elevation = 950.0
proof_water.set_actor_label("DIAG_WP22_Water")

started = time.monotonic()
finished = False
stage = "init"
wait_frames = 0
data = {}


def pie_tick(_delta):
    global finished, stage, wait_frames
    try:
        if finished:
            if not levels.is_in_play_in_editor():
                unreal.unregister_slate_post_tick_callback(tick_handle)
                unreal.SystemLibrary.quit_editor()
            return
        if time.monotonic() - started > 180:
            raise RuntimeError(f"WP-22 proof timed out in stage {stage}")
        world = editor.get_game_world()
        if not world or not levels.is_in_play_in_editor():
            return
        if wait_frames:
            wait_frames -= 1
            return
        pc = data.get("wyrm_pc") or unreal.GameplayStatics.get_player_controller(world, 0)
        player = pc.get_controlled_pawn() if pc else None
        player = unreal.WyrmCharacter.cast(player) if player else None
        if not player:
            candidates = actors(world, unreal.WyrmCharacter)
            if not candidates:
                return
            player = candidates[0]
            if pc:
                pc.possess(player)
        region = unreal.WyrmJadePeaksSubsystem.get_jade_peaks_subsystem(world)
        adapter = first(world, unreal.WyrmGeoForgeAdapter)
        if not region or not adapter:
            return

        if stage == "init":
            if not isinstance(pc, unreal.WyrmPlayerController):
                if pc:
                    pc.un_possess()
                pc = unreal.WyrmPlayerController.spawn_wyrm_player_controller(world, unreal.Transform())
                assert pc, "focused proof must create the production controller"
                pc.possess(player)
                data["wyrm_pc"] = pc
            else:
                data["wyrm_pc"] = pc
            player.grant_combat_abilities()
            attrs = player.get_attributes()
            attrs.set_current_health(100.0)
            attrs.set_current_focus(100.0)
            samples = {}
            sample_debug = {}
            trace_ignored = [player]
            trace_ignored.extend(
                actor for actor in actors(world, unreal.StaticMeshActor)
                if actor.get_actor_label().startswith("DIAG_WP22_"))
            trace_ignored.extend(actors(world, unreal.WyrmWaterVolume))
            for label, x, y in (
                    ("arrival", -4000, -2500), ("source", -4000, -2800),
                    ("valid", -3750, -2800), ("sealed", -3700, -2500),
                    ("occupied", -4000, -2250), ("water", -4250, -2500)):
                hit = surface_hit(world, x, y, trace_ignored)
                sample_debug[label] = None if not hit else {
                    "z": hit["location"].z, "actor": hit["actor"], "component": hit["component"]}
                if not hit or "GeoForge" not in str(hit["actor"]):
                    data["init_wait_ticks"] = data.get("init_wait_ticks", 0) + 1
                    if data["init_wait_ticks"] % 300 == 1:
                        terrain = first(world, unreal.GeoForgeInfiniteTerrainActor)
                        stats = terrain.get_runtime_render_stats() if terrain else None
                        log(f"Waiting for Jade Peaks terrain collision: {sample_debug}; stats={stats}")
                    return
                samples[label] = hit
            data["surfaces"] = samples
            half_height = player.capsule_component.get_scaled_capsule_half_height()
            arrival_z = samples["arrival"]["location"].z
            player.set_actor_location(unreal.Vector(-4000, -2500, arrival_z + half_height), False, False)

            for blocker in actors(world, unreal.StaticMeshActor):
                label = blocker.get_actor_label()
                if label == "DIAG_WP22_SealedWall":
                    blocker.set_actor_location(unreal.Vector(-3850, -2500, samples["sealed"]["location"].z + 200), False, False)
                elif label == "DIAG_WP22_OccupiedLow":
                    blocker.set_actor_location(unreal.Vector(-4000, -2250, samples["occupied"]["location"].z + 20), False, False)
            waters = actors(world, unreal.WyrmWaterVolume)
            if waters:
                waters[0].set_actor_location(unreal.Vector(-4250, -2500, samples["water"]["location"].z), False, False)
                waters[0].surface_elevation = samples["water"]["location"].z + 150.0
            region.reset_jade_peaks_state()
            nav = unreal.NavigationSystemV1.get_navigation_system(world)
            assert nav, "navigation system must exist"
            finish("JP-01.RegionRuntime", {
                "map": world.get_path_name(), "terrain_adapter": adapter.get_name(),
                "navigation_system": nav.get_class().get_name(),
                "safe_arrival": [-4000, -2500, arrival_z + half_height],
                "terrain_surface_samples": {
                    key: [value["location"].x, value["location"].y, value["location"].z]
                    for key, value in samples.items()
                },
            })
            stage = "presentation"
            wait_frames = 20
            return

        if stage == "presentation":
            required = [
                "/Game/WYRMFALL/Development/Intake/WP22/JadePeaks/Palace/Environment/TVS_VoxelPalace_Palace.TVS_VoxelPalace_Palace",
                "/Game/WYRMFALL/Development/Intake/WP22/JadePeaks/Palace/Characters/TVS_VoxelPalace_Guard.TVS_VoxelPalace_Guard",
                "/Game/WYRMFALL/Development/Intake/WP22/JadePeaks/Palace/Materials/M_TVS_VoxelPalace_Guard.M_TVS_VoxelPalace_Guard",
                "/Game/WYRMFALL/Development/Intake/WP20/Jadefang/Chinese+Dragon/SkeletalMeshes/Hip-Local.Hip-Local",
            ]
            loaded = [unreal.load_asset(path) for path in required]
            assert all(loaded), "all supplied presentation assets must load"
            disciple = first(world, unreal.WyrmJadeDiscipleCharacter)
            assert disciple and disciple.mesh.skeletal_mesh, "disciple must present supplied skeletal mesh"
            assert disciple.mesh.get_material(0), "disciple must present supplied guard material"
            finish("JP-02.RealPresentation", {"assets": required, "disciple": disciple.get_name()})
            data["disciple"] = disciple
            stage = "route"
            return

        if stage == "route":
            landmark_ids = [
                "LM-JADE-ARRIVAL", "LM-JADE-PALACE", "LM-JADE-STORMCOURT",
                "LM-JADE-MIRRORWELL", "LM-JADE-AERIE", "LM-JADE-RETURN",
            ]
            assert region.has_valid_landmark_graph(), "landmark graph must validate"
            assert len(actors(world, unreal.WyrmJadePeaksLandmark)) == 6, "map must contain six landmark actors"
            assert all(region.visit_landmark(item) for item in landmark_ids), "each landmark must commit once"
            assert not region.visit_landmark(landmark_ids[0]), "arrival revisit must be idempotent"
            assert region.record_jadefang_arrival(), "Jadefang arrival fact must commit"
            finish("JP-03.LandmarkRoute", {"ordered_route": landmark_ids, "duplicate_rejected": True})
            stage = "jadefang"
            return

        if stage == "jadefang":
            jadefangs = [d for d in actors(world, unreal.WyrmDragonCharacter) if str(d.dragon_id) == "Jadefang"]
            assert len(jadefangs) == 1, "map must contain exactly one Jadefang"
            jadefang = jadefangs[0]
            assert jadefang.has_supported_rig_profile(), "Jadefang profile must be supported"
            assert jadefang.bond_with_humanoid(player), "Jadefang must bond with the same humanoid"
            assert jadefang.request_form_change(unreal.WyrmDragonForm.TRUE_FORM), "Heartfold growth request must start"
            data["jadefang"] = jadefang
            stage = "jadefang_wait"
            wait_frames = 10
            return

        if stage == "jadefang_wait":
            jadefang = data["jadefang"]
            if jadefang.is_transitioning_form():
                return
            assert jadefang.get_dragon_form() == unreal.WyrmDragonForm.TRUE_FORM, "Jadefang must reach true form"
            # Exercise the established shared dragon systems in this production
            # destination, not only in the older WP-20 fixture.
            jadefang.set_actor_location(unreal.Vector(3000, 2000, 1050), False, False)
            player.set_actor_location(unreal.Vector(2850, 2000, 988), False, False)
            assert jadefang.mount_humanoid(player), "Jade Peaks mounting must succeed"
            assert jadefang.take_off(), "Jade Peaks takeoff must succeed"
            flight_observed = jadefang.is_in_flight()
            assert flight_observed, "Jade Peaks flight state must become active"
            assert jadefang.land(), "Jade Peaks landing must succeed"
            assert jadefang.dismount_humanoid(), "Jade Peaks dismount must succeed"

            wyrm_pc = data["wyrm_pc"]
            assert jadefang.start_direct_control(wyrm_pc, player), "direct control must possess Jadefang"
            dummy = unreal.WyrmEnemyCharacter.spawn_wyrm_enemy(
                world, unreal.WyrmEnemyRole.MELEE_CHASER,
                unreal.Transform(location=unreal.Vector(3150, 2000, 1050)))
            assert dummy, "Jade Peaks combat target must spawn"
            dummy.attributes.set_current_max_health(100.0)
            dummy.attributes.set_current_health(100.0)
            dummy.attributes.set_current_armor(0.0)
            assert jadefang.perform_primary_attack(dummy), "Jadefang primary GAS attack must execute"
            primary_damage = 100.0 - dummy.attributes.get_current_health()
            dummy.attributes.set_current_health(100.0)
            assert jadefang.perform_secondary_attack(dummy), "Jadefang secondary GAS attack must execute"
            secondary_damage = 100.0 - dummy.attributes.get_current_health()
            dummy.destroy_actor()
            assert jadefang.end_direct_control(wyrm_pc), "direct control must return to the humanoid"
            assert wyrm_pc.get_controlled_pawn() == player, "humanoid possession must be restored"
            jadefang.set_dragon_form(unreal.WyrmDragonForm.COMPANION_FORM)
            player.set_actor_location(unreal.Vector(-4000, -2500, 988), False, False)
            finish("JP-04.JadefangContinuity", {
                "dragon_id": str(jadefang.dragon_id), "bonded": True,
                "heartfold_true_form_observed": True, "mount_and_flight": flight_observed,
                "primary_damage": primary_damage, "secondary_damage": secondary_damage,
                "direct_control_and_humanoid_return": True,
            })
            stage = "mirror_nav_wait"
            data["nav_wait_ticks"] = 0
            return

        if stage == "mirror_nav_wait":
            if not data.get("mirror_unlocked"):
                disciple = data["disciple"]
                assert disciple.resolve_with_trust(player), "trust path must resolve"
                assert player.is_echo_unlocked("MirrorStep"), "Mirror Step must unlock"
                assert player.equip_echo("MirrorStep"), "Mirror Step must equip"
                data["mirror_unlocked"] = True
            surfaces = data["surfaces"]
            half_height = player.capsule_component.get_scaled_capsule_half_height()
            player.set_actor_location(unreal.Vector(
                -4000, -2800, surfaces["source"]["location"].z + half_height), False, False)
            nav_reason = player.get_mirror_step_failure_reason(surfaces["valid"]["location"])
            if nav_reason == "UnreachableOrVoid":
                data["nav_wait_ticks"] += 1
                if data["nav_wait_ticks"] % 120 == 1:
                    terrain = first(world, unreal.GeoForgeInfiniteTerrainActor)
                    if terrain:
                        unreal.WyrmTerrainDiagnostics.refresh_navigation_data_for_actor(terrain)
                    log(f"Waiting for Jade Peaks navigation projection ({data['nav_wait_ticks']} checks)")
                return
            assert not nav_reason, f"arrival-area navigation readiness rejected: {nav_reason}"
            stage = "mirror_setup"
            return

        if stage == "mirror_setup":
            disciple = data["disciple"]
            assert not disciple.resolve_with_trust(player), "disciple reward must not duplicate"
            assert region.has_fact("disciple.trusted") and region.has_fact("echo.mirror_step")
            assert region.record_return_route_ready(), "return route fact must commit after unlock"
            attrs = player.get_attributes()
            initial_focus = attrs.get_current_focus()
            surfaces = data["surfaces"]
            half_height = player.capsule_component.get_scaled_capsule_half_height()
            start = unreal.Vector(-4000, -2500, surfaces["arrival"]["location"].z + half_height)
            player.set_actor_location(start, False, False)

            # Diagnostic-only blockers prove the explicit fail-closed branches;
            # the production map presentation remains supplied art.
            sealed_reason = player.get_mirror_step_failure_reason(surfaces["sealed"]["location"])
            sealed_ok = not sealed_reason
            assert not sealed_ok and sealed_reason == "SealedBoundary", sealed_reason

            occupied_reason = player.get_mirror_step_failure_reason(surfaces["occupied"]["location"])
            occupied_ok = not occupied_reason
            assert not occupied_ok and occupied_reason == "DestinationOccupied", occupied_reason

            water_reason = player.get_mirror_step_failure_reason(surfaces["water"]["location"])
            water_ok = not water_reason
            assert not water_ok and water_reason == "WaterDestination", water_reason

            range_reason = player.get_mirror_step_failure_reason(unreal.Vector(
                -3500, -2500, surfaces["arrival"]["location"].z))
            range_ok = not range_reason
            assert not range_ok and range_reason == "OutOfRange", range_reason

            player.set_actor_location(unreal.Vector(-7000, -2500, surfaces["arrival"]["location"].z + half_height), False, False)
            void_reason = player.get_mirror_step_failure_reason(unreal.Vector(-7350, -2500, surfaces["arrival"]["location"].z))
            void_ok = not void_reason
            assert not void_ok and void_reason == "UnreachableOrVoid", void_reason
            assert abs(attrs.get_current_focus() - initial_focus) < 0.01, "rejections must not spend Focus"

            player.set_actor_location(unreal.Vector(
                -4000, -2800, surfaces["source"]["location"].z + half_height), False, False)
            valid_target = surfaces["valid"]["location"]
            valid_reason = player.get_mirror_step_failure_reason(valid_target)
            valid_ok = not valid_reason
            assert valid_ok, f"valid Mirror Step rejected: {valid_reason}"
            before = player.get_actor_location()
            assert player.activate_mirror_step(valid_target), "valid Mirror Step activation must commit through GAS"
            after = player.get_actor_location()
            assert (after - before).length() > 100.0, "Mirror Step must move the player"
            assert abs(attrs.get_current_focus() - (initial_focus - 20.0)) < 0.1, "valid use must spend 20 Focus"
            assert player.get_mirror_step_remaining_cooldown() > 0.0, "valid use must start cooldown"
            finish("JP-05.MirrorStep", {
                "rejections": [sealed_reason, occupied_reason, water_reason, range_reason, void_reason],
                "focus_spent_on_rejections": 0, "valid_distance_cm": (after - before).length(),
                "valid_focus_cost": 20.0, "cooldown_started": player.get_mirror_step_remaining_cooldown(),
                "trust_resolution": True, "duplicate_reward_rejected": True,
            })
            stage = "save"
            return

        if stage == "save":
            verdance = unreal.WyrmDragonCharacter.spawn_wyrm_dragon(
                world, unreal.WyrmDragonRole.ALLIED_COMPANION,
                unreal.Transform(location=unreal.Vector(-3200, -500, 1000)))
            verdance.set_dragon_id("Verdance")
            data["verdance"] = verdance
            snapshot = unreal.WyrmSaveSubsystem.create_snapshot_object(SLOT, player, adapter, world)
            assert snapshot and snapshot.schema_version == 5
            regional_ids = sorted(str(item.region_id) for item in snapshot.regional_world_records)
            assert "JadePeaks" in regional_ids, regional_ids
            assert unreal.WyrmSaveSubsystem.is_schema_version_supported(4), "schema 4 backward read must remain supported"
            ids = sorted(str(item.dragon_id) for item in snapshot.dragon_records)
            assert "Jadefang" in ids and "Verdance" in ids, ids
            assert unreal.WyrmSaveSubsystem.save_snapshot_to_slot(SLOT, player, adapter, world)
            saved_cooldown = player.get_mirror_step_remaining_cooldown()
            region.reset_jade_peaks_state()
            player.restore_mirror_step_state(0.0)
            assert unreal.WyrmSaveSubsystem.load_snapshot_from_slot(SLOT, player, adapter, world)
            assert region.has_fact("echo.mirror_step") and region.has_visited_landmark("LM-JADE-ARRIVAL")
            assert player.get_mirror_step_remaining_cooldown() > 0.0
            restored_ids = sorted(str(d.dragon_id) for d in actors(world, unreal.WyrmDragonCharacter))
            assert "Jadefang" in restored_ids and "Verdance" in restored_ids
            finish("JP-06.UnifiedPersistence", {
                "schema": snapshot.schema_version, "snapshot_dragons": ids,
                "regional_records": regional_ids, "schema4_backward_read": True,
                "restored_dragons": restored_ids, "region_facts_restored": True,
                "landmark_route_restored": True, "saved_cooldown": saved_cooldown,
                "restored_cooldown": player.get_mirror_step_remaining_cooldown(),
            })
            report["status"] = "PASS"
            write_report()
            finished = True
            levels.editor_request_end_play()
            return
    except Exception as exc:
        report["status"] = "FAIL"
        report["failed_stage"] = stage
        report["error"] = repr(exc)
        report["traceback"] = traceback.format_exc()
        log(f"FAIL in {stage}: {exc!r}")
        write_report()
        finished = True
        if levels.is_in_play_in_editor():
            levels.editor_request_end_play()


tick_handle = unreal.register_slate_post_tick_callback(pie_tick)
levels.editor_play_simulate()
log("WP-22 focused PIE proof started")
