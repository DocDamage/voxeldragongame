"""Portable tooling tests only. Fake engine directories are metadata fixtures,
NOT an Unreal installation, import, compiler or gameplay simulation."""
from __future__ import annotations
import json
import os
from pathlib import Path
import shutil
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
import wyrm_support as s


class FixtureTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix="wyrm tooling ")
        self.addCleanup(self.tmp.cleanup)
        self.base = Path(self.tmp.name)
        self.root = self.base / "Project With Spaces"; self.root.mkdir()
        self.engine = self.base / "Engine With Spaces"; self.engine.mkdir()
        self.env = patch.dict(os.environ, {}, clear=True); self.env.start(); self.addCleanup(self.env.stop)
        self.toolchain = patch.object(s, "toolchain_inventory", return_value={"status": "FIXTURE_METADATA_ONLY"})
        self.toolchain.start(); self.addCleanup(self.toolchain.stop)

    def engine_fixture(self, version=(5, 8, 2), tools=True, plugins=True):
        s.write_json(self.engine / "Engine/Build/Build.version", dict(zip(("MajorVersion", "MinorVersion", "PatchVersion"), version)))
        if tools:
            for p in s.ue_tools(self.engine).values():
                p.parent.mkdir(parents=True, exist_ok=True); p.write_text("METADATA FIXTURE - NOT EXECUTABLE")
        if plugins:
            for name in s.REQUIRED_PLUGINS:
                s.write_json(self.engine / f"Engine/Plugins/{name}/{name}.uplugin", {"FileVersion":3, "VersionName":"fixture", "Modules":[]})
        return self.engine


class JsonPathTests(FixtureTest):
    def test_json_roundtrip_unicode(self):
        p=self.root/'state.json'; s.write_json(p, {'name':'Verdance — 龙'}); self.assertEqual(s.read_json(p)['name'],'Verdance — 龙')
    def test_atomic_json_leaves_no_tmp(self):
        p=self.root/'state.json'; s.write_json(p,{'a':1}); self.assertFalse(p.with_name('state.json.tmp').exists())
    def test_atomic_replace_preserves_new_value(self):
        p=self.root/'state.json'; s.write_json(p,{'a':1}); s.write_json(p,{'a':2}); self.assertEqual(s.read_json(p),{'a':2})
    def test_utf8_bom_json(self):
        p=self.root/'bom.json';p.write_text('{"x":1}',encoding='utf-8-sig');self.assertEqual(s.read_json(p),{'x':1})
    def test_bad_json_actionable(self):
        p=self.root/'broken.json';p.write_text('{');self.assertRaises(s.SetupError,s.read_json,p)
    def test_missing_json_actionable(self): self.assertRaises(s.SetupError,s.read_json,self.root/'missing.json')
    def test_safe_relative_path(self): self.assertEqual(s.inside(self.root,'nested/file.txt'),self.root/'nested/file.txt')
    def test_traversal_rejected(self): self.assertRaises(s.SetupError,s.inside,self.root,'../outside')
    def test_absolute_rejected(self): self.assertRaises(s.SetupError,s.inside,self.root,str(self.base/'outside'))
    def test_windows_path_rejected(self): self.assertRaises(s.SetupError,s.inside,self.root,'C:\\outside.txt')
    def test_root_not_file_rejected(self): self.assertRaises(s.SetupError,s.inside,self.root,'.')
    def test_symlink_escape_rejected(self):
        out=self.base/'out';out.mkdir()
        try:
            (self.root/'escape').symlink_to(out,target_is_directory=True)
        except (OSError, NotImplementedError) as exc:
            self.skipTest(f'This host cannot create test symlinks: {exc}')
        self.assertRaises(s.SetupError,s.inside,self.root,'escape/file.txt')
    def test_digest_changes(self):
        p=self.root/'x';p.write_bytes(b'a');a=s.digest(p);p.write_bytes(b'b');self.assertNotEqual(a,s.digest(p))
    def test_missing_local_settings_is_empty(self): self.assertEqual(s.load_local(self.root),{})
    def test_invalid_local_settings_rejected(self):
        s.write_json(self.root/'.local/settings.json',[]);self.assertRaises(s.SetupError,s.load_local,self.root)


class EngineTests(FixtureTest):
    def test_normalize_engine_child(self):
        self.engine_fixture();self.assertEqual(s.normalize_engine(self.engine/'Engine'),self.engine)
    def test_read_version(self): self.assertEqual(s.engine_version(self.engine_fixture()),(5,8,2))
    def test_missing_version_rejected(self): self.assertRaises(s.SetupError,s.engine_version,self.engine)
    def test_malformed_version_rejected(self):
        s.write_json(self.engine/'Engine/Build/Build.version',{'MajorVersion':5});self.assertRaises(s.SetupError,s.engine_version,self.engine)
    def test_negative_version_rejected(self):
        self.engine_fixture((5,8,-1));self.assertRaises(s.SetupError,s.engine_version,self.engine)
    def test_explicit_engine_selected(self):
        self.engine_fixture();self.assertEqual(s.choose_engine(self.root,str(self.engine)),self.engine)
    def test_wrong_family_rejected(self):
        self.engine_fixture((5,7,4));self.assertRaises(s.SetupError,s.choose_engine,self.root,str(self.engine))
    def test_env_path_selected(self):
        self.engine_fixture();os.environ['UE_ROOT']=str(self.engine);self.assertEqual(s.choose_engine(self.root),self.engine)
    def test_configured_path_selected(self):
        self.engine_fixture();s.write_json(self.root/'.local/settings.json',{'engine_root':str(self.engine)});self.assertEqual(s.choose_engine(self.root),self.engine)
    def test_invalid_explicit_not_hidden(self):
        self.engine_fixture();os.environ['UE_ROOT']=str(self.engine);self.assertRaises(s.SetupError,s.choose_engine,self.root,str(self.base/'missing'))
    def test_invalid_env_not_hidden(self):
        self.engine_fixture();s.write_json(self.root/'.local/settings.json',{'engine_root':str(self.engine)});os.environ['UE_ROOT']=str(self.base/'missing');self.assertRaises(s.SetupError,s.choose_engine,self.root)
    def test_multiple_engines_require_selection(self):
        self.engine_fixture();other=self.base/'Other';shutil.copytree(self.engine,other)
        with patch.object(s,'engine_candidates',return_value=[self.engine,other]):self.assertRaises(s.SetupError,s.choose_engine,self.root)
    def test_no_engine_actionable(self):
        with patch.object(s,'engine_candidates',return_value=[]):self.assertRaises(s.SetupError,s.choose_engine,self.root)
    def test_doctor_metadata_not_build_pass(self):
        self.engine_fixture();r=s.doctor(self.root,str(self.engine));self.assertEqual(r['status'],'INSPECTED_NOT_BUILD_VERIFIED');self.assertEqual(r['gameplay_status'],'NOT_RUN')
    def test_doctor_missing_plugin_blocked(self):
        self.engine_fixture();(self.engine/'Engine/Plugins/Mutable/Mutable.uplugin').unlink();r=s.doctor(self.root,str(self.engine));self.assertEqual(r['status'],'BLOCKED');self.assertTrue(any('Mutable' in e for e in r['errors']))
    def test_doctor_duplicate_plugin_blocked(self):
        self.engine_fixture();s.write_json(self.root/'Plugins/Mutable/Mutable.uplugin',{});r=s.doctor(self.root,str(self.engine));self.assertEqual(r['plugins']['Mutable']['status'],'AMBIGUOUS_DUPLICATES')
    def test_doctor_missing_tools_blocked(self):
        self.engine_fixture(tools=False);self.assertEqual(s.doctor(self.root,str(self.engine))['status'],'BLOCKED')
    def test_other_patch_warns_not_migrates(self):
        self.engine_fixture((5,8,1));r=s.doctor(self.root,str(self.engine));self.assertTrue(r['warnings']);self.assertEqual(r['engine']['version'],'5.8.1')
    def test_require_engine_cannot_bypass_plugins(self):
        self.engine_fixture(plugins=False);self.assertRaises(s.SetupError,s.require_engine,self.root,str(self.engine))


class CommandTests(FixtureTest):
    def test_windows_build_paths_with_spaces(self):
        c=s.build_command(self.root,self.engine,'build','Windows');self.assertEqual(c[1:4],['WYRMFALLEditor','Win64','Development']);self.assertIn(f'-Project={self.root / "WYRMFALL.uproject"}',c)
    def test_windows_no_engine_changes_flag(self): self.assertIn('-NoEngineChanges',s.build_command(self.root,self.engine,'build','Windows'))
    def test_linux_not_claimed_win64(self): self.assertEqual(s.build_command(self.root,self.engine,'build','Linux')[2],'Linux')
    def test_generate_ubt_flag(self): self.assertIn('-projectfiles',s.build_command(self.root,self.engine,'generate','Windows'))
    def test_bootstrap_uses_full_editor_python(self):
        c=s.build_command(self.root,self.engine,'bootstrap','Windows');self.assertTrue(any(a.startswith('-ExecutePythonScript=') for a in c));self.assertFalse(any(a.startswith('-run=') for a in c))
    def test_bootstrap_windows_python_path_has_no_escape_sequences(self):
        from pathlib import PureWindowsPath
        c = s.build_command(PureWindowsPath('G:/assets/voxel project'),
                            PureWindowsPath('C:/Program Files/UE_5.8'), 'bootstrap', 'Windows')
        self.assertIn('-ExecutePythonScript=G:/assets/voxel project/tools/unreal/create_bootstrap_map.py', c)
    def test_open_without_map_does_not_invent_map(self): self.assertNotIn(s.MAP_PACKAGE,s.build_command(self.root,self.engine,'open','Windows'))
    def test_open_existing_map_passes_explicit_map(self):
        p=self.root/'Content/WYRMFALL/Development/Maps/L_DEV_Bootstrap.umap';p.parent.mkdir(parents=True);p.write_text('FILE-PRESENCE FIXTURE ONLY');self.assertIn(s.MAP_PACKAGE,s.build_command(self.root,self.engine,'open','Windows'))
    def test_native_tests_no_premature_quit(self):
        c=s.build_command(self.root,self.engine,'ue-test','Windows');self.assertIn('-TestExit=Automation Test Queue Empty',c);self.assertFalse(any(';Quit' in a for a in c))
    def test_native_tests_use_narrow_filter(self):self.assertIn('-ExecCmds=Automation RunTests WYRMFALL.Scaffold',s.build_command(self.root,self.engine,'ue-test','Windows'))
    def test_unsupported_host_rejected(self):self.assertRaises(s.SetupError,s.ue_tools,self.engine,'Darwin')
    def test_unsupported_action_rejected(self):self.assertRaises(s.SetupError,s.build_command,self.root,self.engine,'cook-all','Windows')


class AutomationReportTests(unittest.TestCase):
    def valid(self):return {'tests':[{'fullTestPath':n,'state':'Success'} for n in sorted(s.discover_native_tests())],'failed':0,'notRun':0,'inProcess':0}
    def test_valid_native_report(self):self.assertTrue(s.assess_automation(self.valid())[0])
    def test_empty_report_not_pass(self):self.assertFalse(s.assess_automation({'tests':[]})[0])
    def test_unrelated_tests_not_pass(self):self.assertFalse(s.assess_automation({'tests':[{'fullTestPath':'Other.Test','state':'Success'}]*4})[0])
    def test_failed_case_not_pass(self):
        d=self.valid();d['tests'][0]['state']='Fail';self.assertFalse(s.assess_automation(d)[0])
    def test_notrun_case_not_pass(self):
        d=self.valid();d['tests'][0]['state']='NotRun';self.assertFalse(s.assess_automation(d)[0])
    def test_missing_case_not_pass(self):
        d=self.valid();d['tests'].pop();self.assertFalse(s.assess_automation(d)[0])
    def test_duplicate_cases_not_pass(self):
        d=self.valid();d['tests'].append(d['tests'][0]);self.assertFalse(s.assess_automation(d)[0])
    def test_fake_names_not_pass(self):
        d=self.valid();d['tests'][0]['fullTestPath']='WYRMFALL.Scaffold.Fake';self.assertFalse(s.assess_automation(d)[0])
    def test_failed_summary_not_pass(self):
        d=self.valid();d['failed']=1;self.assertFalse(s.assess_automation(d)[0])
    def test_inprocess_summary_not_pass(self):
        d=self.valid();d['inProcess']=1;self.assertFalse(s.assess_automation(d)[0])
    def test_malformed_report_not_pass(self):self.assertFalse(s.assess_automation([])[0])
    def test_malformed_row_not_pass(self):self.assertFalse(s.assess_automation({'tests':['bad']})[0])


class SourceValidationTests(FixtureTest):
    def test_walker_excludes_generated_and_local(self):
        for folder in ['Saved','.local','Binaries','Source']:
            p=self.root/folder/'file.txt';p.parent.mkdir();p.write_text('x')
        self.assertEqual([p.relative_to(self.root).as_posix() for p in s.source_files(self.root)],['Source/file.txt'])
    def test_markdown_missing_link_reported(self):
        (self.root/'README.md').write_text('[bad](missing.md)');self.assertEqual(len(s.markdown_link_errors(self.root)),1)
    def test_markdown_existing_link_passes(self):
        (self.root/'README.md').write_text('[good](target.md)');(self.root/'target.md').write_text('# test');self.assertEqual(s.markdown_link_errors(self.root),[])
    def test_markdown_code_block_not_link_checked(self):
        (self.root/'README.md').write_text('```text\n[example](missing.md)\n```');self.assertEqual(s.markdown_link_errors(self.root),[])
    def test_manifest_integrity_pass(self):
        p=self.root/'data.txt';p.write_text('x');s.write_json(self.root/'SCAFFOLD_MANIFEST.json',{'files':[{'path':'data.txt','bytes':1,'sha256':s.digest(p)}]});self.assertEqual(s.manifest_errors(self.root),[])
    def test_manifest_modified_detected(self):
        p=self.root/'data.txt';p.write_text('x');s.write_json(self.root/'SCAFFOLD_MANIFEST.json',{'files':[{'path':'data.txt','bytes':1,'sha256':s.digest(p)}]});p.write_text('y');self.assertTrue(s.manifest_errors(self.root))
    def test_manifest_escape_detected(self):
        s.write_json(self.root/'SCAFFOLD_MANIFEST.json',{'files':[{'path':'../escape','bytes':1,'sha256':'x'}]});self.assertTrue(s.manifest_errors(self.root))
    def test_manifest_empty_not_pass(self):
        s.write_json(self.root/'SCAFFOLD_MANIFEST.json',{'files':[]});self.assertTrue(s.manifest_errors(self.root))
    def test_actual_scaffold_structure(self):self.assertEqual(s.verify_structure(s.ROOT),[])
    def test_preserved_acceptance_not_falsely_passed(self):
        d=s.read_json(s.ROOT/'Documentation/DesignPack/docs/production/acceptance_catalogue.json');self.assertTrue(d['cases']);self.assertTrue(all(c['status']=='NOT_RUN' for c in d['cases']))
    def test_readiness_uses_explicit_validation_states(self):
        d=s.read_json(s.ROOT/'Config/IntegrationReadiness.json')
        allowed={'NOT_RUN','PASS','FAIL','BLOCKED','IMPLEMENTED_NOT_VERIFIED'}
        self.assertTrue(d['native_results'])
        self.assertTrue(all(v in allowed for v in d['native_results'].values()))

if __name__=='__main__':unittest.main()
