"""Regression tests for v0.2. Real Python child processes; no Unreal emulation."""
from __future__ import annotations
import contextlib
import io
import json
import os
from pathlib import Path
import sys
import tempfile
import time
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
import wyrm_support as s
import wyrm_process as p
import wyrm_onboarding as o


class TemporaryRoot(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="wyrm v02 spaces ")
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        (self.root / "Source").mkdir()
        (self.root / "WYRMFALL.uproject").write_text('{"fixture":"not an Unreal project"}')
        self.source = self.root / "Source/Fixture.cpp"
        self.source.write_text('IMPLEMENT_SIMPLE_AUTOMATION_TEST(F, "WYRMFALL.Scaffold.Fixture", flags)')


class TestDiscovery(TemporaryRoot):
    def test_actual_suite_contains_hardening_cases(self):
        required = {"WYRMFALL.Scaffold.AttributeBaseClamps", "WYRMFALL.Scaffold.TerrainNumericBoundaries"}
        self.assertTrue(required.issubset(s.discover_native_tests()))
    def test_new_source_case_becomes_required(self):
        self.source.write_text(self.source.read_text() + '\nIMPLEMENT_SIMPLE_AUTOMATION_TEST(G, "WYRMFALL.Scaffold.Added", flags)')
        names = s.discover_native_tests(self.root)
        self.assertEqual(len(names), 2)
        self.assertFalse(s.assess_automation({"tests": [{"fullTestPath": "WYRMFALL.Scaffold.Fixture", "state": "Success"}]}, names)[0])
    def test_commented_examples_not_required(self):
        self.source.write_text(self.source.read_text() + '\n// IMPLEMENT_SIMPLE_AUTOMATION_TEST(G, "WYRMFALL.Scaffold.Example", x)\n/* "WYRMFALL.Scaffold.Comment" */')
        self.assertEqual(len(s.discover_native_tests(self.root)), 1)
    def test_duplicate_native_names_rejected(self):
        self.source.write_text(self.source.read_text() * 2)
        with self.assertRaises(s.SetupError): s.discover_native_tests(self.root)
    def test_unknown_macro_not_silently_skipped(self):
        self.source.write_text('IMPLEMENT_COMPLEX_AUTOMATION_TEST(G, "WYRMFALL.Scaffold.Unrecognized", flags)')
        with self.assertRaises(s.SetupError): s.discover_native_tests(self.root)
    def test_empty_source_suite_rejected(self):
        self.source.write_text('// no tests')
        with self.assertRaises(s.SetupError): s.discover_native_tests(self.root)
    def test_boolean_summary_is_malformed(self):
        data = {"tests": [{"fullTestPath": "WYRMFALL.Scaffold.Fixture", "state": "Success"}], "failed": False}
        self.assertFalse(s.assess_automation(data, s.discover_native_tests(self.root))[0])
    def test_success_with_errors_rejected(self):
        data = {"tests": [{"fullTestPath": "WYRMFALL.Scaffold.Fixture", "state": "Success", "errors": 1}]}
        self.assertFalse(s.assess_automation(data, s.discover_native_tests(self.root))[0])
    def test_unexpected_scaffold_case_rejected(self):
        data = {"tests": [{"fullTestPath": "WYRMFALL.Scaffold." + name, "state": "Success"} for name in ("Fixture", "Extra")]}
        self.assertFalse(s.assess_automation(data, s.discover_native_tests(self.root))[0])
    def test_source_only_discovery_not_native_execution(self):
        self.assertFalse((self.root / "Saved").exists())
        s.discover_native_tests(self.root)
        self.assertFalse((self.root / "Saved").exists())


class ProcessTests(TemporaryRoot):
    def run_python(self, program, timeout=10):
        with contextlib.redirect_stdout(io.StringIO()):
            return p.execute_logged([sys.executable, "-c", program], self.root, "python-check", timeout)
    def test_real_child_success_and_hashed_log(self):
        result = self.run_python("print('real child output')")
        self.assertEqual((result['state'], result['exit_code']), ('EXITED', 0))
        log = self.root / result['log']
        self.assertIn('real child output', log.read_text())
        self.assertEqual(s.digest(log), result['log_sha256'])
        self.assertEqual(s.read_json(log.with_suffix('.json'))['state'], 'EXITED')
    def test_real_nonzero_is_not_success(self):
        result = self.run_python('raise SystemExit(7)')
        self.assertEqual(result['exit_code'], 7)
    def test_launch_failure_has_receipt(self):
        with contextlib.redirect_stdout(io.StringIO()):
            result = p.execute_logged([str(self.root / 'absent_program')], self.root, 'missing', 3)
        self.assertEqual(result['state'], 'START_FAILED')
        self.assertIsNone(result['exit_code'])
    def test_logs_never_collide(self):
        a = self.run_python('pass'); b = self.run_python('pass')
        self.assertNotEqual(a['log'], b['log'])
    def test_timeout_never_returns_exit_zero(self):
        result = self.run_python('import time; time.sleep(10)', timeout=1)
        self.assertEqual(result['state'], 'TIMED_OUT')
        self.assertIsNone(result['exit_code'])
        self.assertIn('cleanup', result)
    @unittest.skipIf(os.name == 'nt', 'POSIX process-group integration; Windows taskkill must be checked on Windows')
    def test_timeout_terminates_spawned_child(self):
        marker = self.root / 'descendant-survived.txt'
        child = f"import time; from pathlib import Path; time.sleep(2); Path({str(marker)!r}).write_text('bad')"
        parent = f'import subprocess,sys,time; subprocess.Popen([sys.executable,"-c",{child!r}]); time.sleep(20)'
        result = self.run_python(parent, timeout=1)
        time.sleep(1.3)
        self.assertEqual(result['cleanup'], 'PROCESS_GROUP_KILLED')
        self.assertFalse(marker.exists())
    def test_windows_batch_argv_is_quoted_without_escape_backslashes(self):
        command = [r'C:\Program Files\UE_5.8\Build.bat', r'-Project=C:\Games (Local)\WYRMFALL.uproject']
        actual = p.process_command(command, windows=True)
        self.assertIn('"-Project=C:\\Games (Local)\\WYRMFALL.uproject"', actual)
        self.assertNotIn('\\"', actual)
    def test_windows_batch_expansion_characters_rejected(self):
        for value in ('x%PATH%', 'x&whoami', 'x!PATH!', 'x|more', 'a\nb'):
            with self.subTest(value=value), self.assertRaises(s.SetupError):
                p.process_command(['Build.bat', value], windows=True)
    def test_executable_argv_does_not_use_shell(self):
        args = ['UnrealEditor.exe', 'path with spaces.uproject', 'literal%value']
        self.assertEqual(p.process_command(args, windows=True), args)
    def test_shell_script_runs_through_bash(self):
        self.assertEqual(p.process_command(['Build.sh', 'x'], windows=False), ['bash', 'Build.sh', 'x'])
    def test_invalid_timeout_rejected(self):
        with self.assertRaises(s.SetupError): p.execute_logged(['x'], self.root, 'x', 0)
    def test_log_path_traversal_rejected(self):
        with self.assertRaises(s.SetupError): p.execute_logged(['x'], self.root, '../x', 1)


class StrictMetadataTests(TemporaryRoot):
    def test_engine_version_does_not_truncate_fraction(self):
        path = self.root / 'Engine/Build/Build.version'
        s.write_json(path, {'MajorVersion': 5, 'MinorVersion': 8.9, 'PatchVersion': 2})
        with self.assertRaises(s.SetupError): s.engine_version(self.root)
    def test_engine_version_array_is_actionable(self):
        s.write_json(self.root / 'Engine/Build/Build.version', [])
        with self.assertRaises(s.SetupError): s.engine_version(self.root)
    def test_boolean_version_rejected(self):
        s.write_json(self.root / 'Engine/Build/Build.version', {'MajorVersion': True, 'MinorVersion': 8, 'PatchVersion': 2})
        with self.assertRaises(s.SetupError): s.engine_version(self.root)
    def test_manifest_non_object_rejected(self):
        s.write_json(self.root / 'SCAFFOLD_MANIFEST.json', [])
        self.assertTrue(s.manifest_errors(self.root))
    def test_manifest_duplicate_rejected(self):
        row = {'path': self.source.relative_to(self.root).as_posix(), 'bytes': self.source.stat().st_size, 'sha256': s.digest(self.source)}
        s.write_json(self.root / 'SCAFFOLD_MANIFEST.json', {'files': [row, row]})
        self.assertTrue(any('Duplicate' in e for e in s.manifest_errors(self.root)))
    def test_failed_json_write_does_not_destroy_old_data(self):
        path = self.root / 'state.json'; s.write_json(path, {'old': 1})
        with self.assertRaises(ValueError): s.write_json(path, {'value': float('nan')})
        self.assertEqual(s.read_json(path), {'old': 1})
        self.assertEqual(list(self.root.glob('state.json.*.tmp')), [])


class OnboardingTests(TemporaryRoot):
    def fake_process(self, command, root, label, timeout):
        # TEST FIXTURE only: no engine invocation or native claim is made here.
        log = root / 'Saved/ScaffoldLogs' / (label + '.log')
        log.parent.mkdir(parents=True, exist_ok=True); log.write_text('SYNTHETIC PROCESS RECEIPT FOR TOOLING TEST ONLY')
        for rel in o.ARTIFACTS.get(label, ()):
            path = root / rel; path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text('SYNTHETIC EVIDENCE IN TEMP DIRECTORY, NOT UNREAL CONTENT')
        return {'state': 'EXITED', 'exit_code': 0, 'log': log.relative_to(root).as_posix(), 'log_sha256': s.digest(log)}
    def onboard_fixture(self, native=False):
        with patch.object(o, 'execute_logged', side_effect=self.fake_process):
            return o.run_onboarding(self.root, native=native)
    def test_default_plan_never_runs_native(self):
        self.assertEqual([s['id'] for s in o.make_plan(self.root, False, None, 5)], list(o.OFFLINE))
    def test_explicit_native_plan_is_bounded(self):
        ids = [s['id'] for s in o.make_plan(self.root, True, None, 5)]
        self.assertEqual(ids, list(o.OFFLINE + o.NATIVE))
        self.assertNotIn('cook', ids); self.assertNotIn('open', ids)
    def test_plan_has_no_side_effects(self):
        o.make_plan(self.root, True, 'C:\\UE With Spaces', 5)
        self.assertFalse((self.root / 'Saved').exists())
    def test_engine_path_forwarded_intact(self):
        plan = o.make_plan(self.root, True, 'C:\\UE With Spaces', 5)
        self.assertIn('C:\\UE With Spaces', plan[-1]['command'])
    def test_offline_success_never_claims_native(self):
        code, path = self.onboard_fixture()
        self.assertEqual(code, 0)
        data = s.read_json(path)
        self.assertEqual(data['status'], 'OFFLINE_CHECKS_PASSED_NATIVE_NOT_RUN')
        self.assertEqual(data['pie'], 'NOT_RUN')
        self.assertEqual(o.inspect_latest(self.root)['status'], data['status'])
    def test_native_commands_still_do_not_pass_pie(self):
        code, path = self.onboard_fixture(native=True)
        self.assertEqual(s.read_json(path)['status'], 'NATIVE_COMMANDS_PASSED_PIE_NOT_RUN')
        self.assertEqual(o.inspect_latest(self.root)['pie'], 'NOT_RUN')
    def test_stops_at_doctor_failure(self):
        def failing(command, root, label, timeout):
            result = self.fake_process(command, root, label, timeout)
            if label == 'doctor': result['exit_code'] = 2
            return result
        with patch.object(o, 'execute_logged', side_effect=failing) as runner:
            code, path = o.run_onboarding(self.root, native=True)
        self.assertEqual(code, 2)
        self.assertEqual(runner.call_count, 4)
        data = s.read_json(path)
        self.assertTrue(all(step['status'] == 'NOT_RUN' for step in data['steps'][4:]))
        self.assertIn('Mutable', data['next_task'])
    def test_source_change_invalidates_report(self):
        self.onboard_fixture()
        self.source.write_text(self.source.read_text() + '\n// edit')
        self.assertEqual(o.inspect_latest(self.root)['status'], 'STALE_EVIDENCE')
    def test_log_change_invalidates_report(self):
        _, path = self.onboard_fixture()
        log = s.read_json(path)['steps'][0]['result']['log']
        (self.root / log).write_text('changed')
        self.assertEqual(o.inspect_latest(self.root)['status'], 'STALE_EVIDENCE')
    def test_source_change_during_run_rejected(self):
        def changing(command, root, label, timeout):
            self.source.write_text(self.source.read_text() + '\n// changed')
            return self.fake_process(command, root, label, timeout)
        with patch.object(o, 'execute_logged', side_effect=changing):
            code, path = o.run_onboarding(self.root)
        self.assertEqual(code, 1)
        self.assertEqual(s.read_json(path)['status'], 'SOURCE_CHANGED_RERUN_REQUIRED')
    def test_missing_evidence_cannot_pass(self):
        def missing(command, root, label, timeout):
            result = self.fake_process(command, root, label, timeout)
            if label == 'doctor': (root / o.ARTIFACTS['doctor'][0]).unlink()
            return result
        with patch.object(o, 'execute_logged', side_effect=missing):
            code, path = o.run_onboarding(self.root, native=True)
        self.assertEqual(code, 2)
        self.assertIn('required evidence', s.read_json(path)['steps'][3]['error'])
    def test_live_lock_is_not_deleted_or_stolen(self):
        lock = self.root / 'Saved/Onboarding/active.lock'; lock.parent.mkdir(parents=True); lock.write_text('live')
        with self.assertRaises(s.SetupError): self.onboard_fixture()
        self.assertEqual(lock.read_text(), 'live')
    def test_normal_run_releases_lock(self):
        self.onboard_fixture()
        self.assertFalse((self.root / 'Saved/Onboarding/active.lock').exists())
    def test_missing_latest_is_not_run(self):
        self.assertEqual(o.inspect_latest(self.root)['status'], 'NOT_RUN')
    def test_pointer_escape_rejected(self):
        s.write_json(self.root / 'Saved/Onboarding/latest.json', {'run': '../escape'})
        with self.assertRaises(s.SetupError): o.inspect_latest(self.root)
    def test_false_success_with_no_steps_rejected(self):
        _, path = self.onboard_fixture(); data = s.read_json(path); data['steps'] = []; s.write_json(path, data)
        with self.assertRaises(s.SetupError): o.inspect_latest(self.root)
    def test_false_success_missing_process_receipt_rejected(self):
        _, path = self.onboard_fixture(); data = s.read_json(path); data['steps'][0].pop('result'); s.write_json(path, data)
        with self.assertRaises(s.SetupError): o.inspect_latest(self.root)
    def test_readiness_notes_do_not_invalidate_source(self):
        a = o.source_snapshot(self.root)
        s.write_json(self.root / 'Config/IntegrationReadiness.json', {'note': 'human status, not runtime'})
        self.assertEqual(a['sha256'], o.source_snapshot(self.root)['sha256'])
    def test_config_ini_change_invalidates_source(self):
        a = o.source_snapshot(self.root)
        path = self.root / 'Config/DefaultEngine.ini'; path.parent.mkdir(); path.write_text('changed')
        self.assertNotEqual(a['sha256'], o.source_snapshot(self.root)['sha256'])



class BootstrapReceiptTests(TemporaryRoot):
    def setUp(self):
        super().setUp()
        self.map = self.root / 'map_presence_fixture.bin'
        self.map.write_bytes(b'TEST FIXTURE NOT UNREAL MAP' * 10)
        self.receipt = {'status': 'EDITOR_MAP_SAVED', 'map': s.MAP_PACKAGE, 'actor_types_checked': True,
                        'map_bytes': self.map.stat().st_size, 'map_sha256': s.digest(self.map), 'pie': 'NOT_RUN'}
    def test_receipt_identity_matches_bytes(self):
        s.validate_bootstrap_receipt(self.receipt, self.map)
    def test_changed_bytes_cannot_reuse_receipt(self):
        self.map.write_bytes(b'Changed same fixture!' * 20)
        with self.assertRaises(s.SetupError): s.validate_bootstrap_receipt(self.receipt, self.map)
    def test_legacy_untyped_receipt_rejected(self):
        self.receipt.pop('actor_types_checked')
        with self.assertRaises(s.SetupError): s.validate_bootstrap_receipt(self.receipt, self.map)
    def test_map_creation_cannot_claim_pie(self):
        self.receipt['pie'] = 'PASS'
        with self.assertRaises(s.SetupError): s.validate_bootstrap_receipt(self.receipt, self.map)
    def test_missing_map_rejected(self):
        self.map.unlink()
        with self.assertRaises(s.SetupError): s.validate_bootstrap_receipt(self.receipt, self.map)
    def test_invalid_local_roots_actionable(self):
        s.write_json(self.root / '.local/settings.json', {'asset_roots': 'not an array'})
        with self.assertRaises(s.SetupError): s.load_local(self.root)
    def test_invalid_configured_engine_type_actionable(self):
        s.write_json(self.root / '.local/settings.json', {'engine_root': 123})
        with self.assertRaises(s.SetupError): s.load_local(self.root)
    def test_configure_engine_preserves_existing_asset_roots(self):
        import wyrm
        engine = self.root / 'Local Engine'
        s.write_json(engine / 'Engine/Build/Build.version', {'MajorVersion': 5, 'MinorVersion': 8, 'PatchVersion': 2})
        assets = self.root / 'Asset Library'; assets.mkdir()
        settings = {'engine_root': str(engine), 'asset_roots': [str(assets)]}
        s.write_json(self.root / '.local/settings.json', settings)
        with patch.object(wyrm, 'ROOT', self.root), patch.object(wyrm, 'load_local', return_value=settings), contextlib.redirect_stdout(io.StringIO()):
            self.assertEqual(wyrm.main(['configure', '--engine-root', str(engine)]), 0)
        self.assertEqual(s.read_json(self.root / '.local/settings.json')['asset_roots'], [str(assets)])


if __name__ == '__main__': unittest.main()
