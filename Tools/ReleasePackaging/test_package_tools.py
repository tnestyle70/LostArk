from __future__ import annotations

import copy
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import time
import unittest

import build_portable as builder


class PortableCheckTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.base = builder.OUTPUT_ROOT / ('tests-' + str(time.time_ns()))
        cls.base.mkdir(parents=True)
        cls.launcher = builder.compile_launcher(cls.base / 'compiler', 120)

    def setUp(self):
        self.root = self.base / ('한글 공백 bundle ' + self._testMethodName)
        self.root.mkdir()
        shutil.copy2(self.launcher, self.root / 'LostArk.exe')
        self.resources = self.base / ('external-' + self._testMethodName)
        for name in ('Fonts', 'Character', 'Deploy', 'Effect', 'Map', 'Sound', 'UI'):
            (self.resources / name).mkdir(parents=True)
        for name in ('Client/Default', 'Server/Default'):
            (self.root / name).mkdir(parents=True)
        files = {
            'Client/Bin/Release/Client.exe': b'fixture-not-executable',
            'Client/Bin/Release/Engine.dll': b'fixture-not-library',
            'Server/Bin/Release/Server.exe': b'fixture-not-executable',
            'Client/Bin/Release/vcruntime140.dll': b'fixture-not-library',
            'Server/Bin/Release/vcruntime140.dll': b'fixture-not-library',
            'Data/Effects/EffectCatalog.json': b'{}',
            'Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json': b'{"revision":7}',
            'Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json': b'{"sourceRevision":7}',
            'Data/Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json': b'{"sourceRevision":7}',
            'Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json': b'{"revision":8}',
            'Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap': (
                'KOUKUSAYDONPRODUCTREVISION\tENCOUNTER_KAKULSAYDON_G1\tBOSS\t7\n' +
                ''.join('RAIDGATE\tENCOUNTER_KAKULSAYDON_G1\t%s\tx\tx\t8\n' % g for g in ('GATE1','GATE2','GATE3','BINGO'))).encode(),
        }
        rows = []
        for relative, content in files.items():
            path = self.root / relative; path.parent.mkdir(parents=True, exist_ok=True); path.write_bytes(content)
            rows.append(dict(path=relative, bytes=len(content), sha256=builder.digest(path)))
        self.manifest = dict(schema='lostark.portable-runtime-bundle', formatVersion=1, configuration='Release', protocol=120,
                             serverEndpoint='192.168.200.139:7777', resourcePolicy='external-only-no-install',
                             dataRevisions=dict(sourceRevision=7, sequenceRevision=8), files=rows)

    def check(self, expected, resources=None, package_only=False, environment=None):
        builder.write(self.root / 'bundle-manifest.json', self.manifest)
        receipt = self.base / (self._testMethodName + '.json')
        command = [str(self.root / 'LostArk.exe')]
        command += ['--check-package', str(receipt)] if package_only else ['--check', str(resources or self.resources), str(receipt)]
        result = subprocess.run(command, timeout=30, env=environment)
        self.assertEqual(result.returncode, 0 if expected == 'PASS' else 1)
        observed = builder.read(receipt)
        self.assertEqual(observed['status'], expected)
        self.assertFalse(observed['clientStarted']); self.assertFalse(observed['serverStarted'])
        return observed

    def test_valid_nonlaunch_unicode_portable_without_repository(self):
        result = self.check('PASS')
        self.assertEqual(Path(result['projectDataRoot']), self.root / 'Data')
        self.assertEqual(Path(result['resourceRoot']), self.resources)

    def test_inherited_server_data_override_is_replaced_by_bundle_root(self):
        environment = os.environ.copy()
        environment['LOSTARK_SERVER_DATA_ROOT'] = str(self.base / 'unrelated-old-server')
        result = self.check('PASS', package_only=True, environment=environment)
        self.assertEqual(Path(result['serverDataRoot']), self.root / 'Server/Bin/DataFiles')
        self.assertEqual(Path(result['projectDataRoot']), self.root / 'Data')

    def test_server_preflight_needs_no_resources(self):
        self.check('PASS', package_only=True)

    def test_hash_tampering_rejected(self):
        (self.root / 'Client/Bin/Release/Client.exe').write_bytes(b'altered')
        self.check('FAIL')

    def test_wrong_protocol_rejected(self):
        self.manifest['protocol'] = 106
        self.check('FAIL')

    def test_wrong_endpoint_rejected(self):
        self.manifest['serverEndpoint'] = '192.168.200.142:7777'
        self.check('FAIL')

    def test_missing_resource_folder_rejected(self):
        self.check('FAIL', resources=self.base / 'missing')

    def test_mixed_product_revision_rejected(self):
        self.manifest['dataRevisions']['sourceRevision'] = 9
        self.check('FAIL')

    def test_path_escape_rejected(self):
        self.manifest['files'][0]['path'] = '../escaped.exe'
        self.check('FAIL')

    def test_duplicate_path_rejected(self):
        self.manifest['files'].append(copy.deepcopy(self.manifest['files'][0]))
        self.check('FAIL')

    def test_required_manifest_file_cannot_be_omitted(self):
        self.manifest['files'] = [r for r in self.manifest['files'] if r['path'] != 'Client/Bin/Release/Engine.dll']
        self.check('FAIL')

    def test_published_sequence_mismatch_even_with_updated_hash_is_rejected(self):
        path = self.root / 'Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap'
        path.write_text(path.read_text().replace('\tx\tx\t8', '\tx\tx\t9'), encoding='utf-8')
        row = next(r for r in self.manifest['files'] if r['path'].endswith('Gameplay.bootstrap'))
        row.update(bytes=path.stat().st_size, sha256=builder.digest(path))
        self.check('FAIL')

    def numeric_save(self):
        source = self.root / 'Data/Balance/PlayerProfiles.json'
        source.parent.mkdir(parents=True, exist_ok=True); builder.write(source, {'hp':100})
        self.manifest['files'].append(dict(path='Data/Balance/PlayerProfiles.json',bytes=source.stat().st_size,sha256=builder.digest(source)))
        builder.write(source, {'hp':250})
        bootstrap = self.root / 'Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap'
        bootstrap.write_text(bootstrap.read_text()+'PLAYER\tLANCE_MASTER\t250\n',encoding='utf-8')
        active = bootstrap.parent / 'NumericBalance.active.json'
        builder.write(active,dict(schema='lostark.numeric-balance-active',parentGameplayRevision='a'*64,
            bootstrapContentSha256=builder.digest(bootstrap),numericRevision='b'*64,nonNumericRowsSha256='c'*64))
        saved=dict(schema='lostark.numeric-balance-save',numericRevision='b'*64,bootstrapContentSha256=builder.digest(bootstrap),
            files=[dict(path=p.relative_to(self.root).as_posix(),sha256=builder.digest(p)) for p in (source,bootstrap,active)])
        receipt=bootstrap.parent/'BalanceNumeric.save.receipt.json';builder.write(receipt,saved)
        return source,active,receipt,saved

    def test_native_numeric_save_validates_without_reinstall(self):
        self.numeric_save();self.check('PASS')

    def test_numeric_receipt_cannot_override_executable(self):
        source,active,receipt,saved=self.numeric_save()
        binary=self.root/'Client/Bin/Release/Client.exe';binary.write_bytes(b'altered')
        saved['files'].append(dict(path='Client/Bin/Release/Client.exe',sha256=builder.digest(binary)))
        builder.write(receipt,saved);self.check('FAIL')

    def test_numeric_saved_source_tampering_rejected(self):
        source,active,receipt,saved=self.numeric_save();builder.write(source,{'hp':999});self.check('FAIL')

    def test_numeric_generation_mismatch_rejected(self):
        source,active,receipt,saved=self.numeric_save();doc=builder.read(active);doc['numericRevision']='d'*64
        builder.write(active,doc)
        next(r for r in saved['files'] if r['path'].endswith('NumericBalance.active.json'))['sha256']=builder.digest(active)
        builder.write(receipt,saved);self.check('FAIL')

    def test_numeric_receipt_missing_active_generation_rejected(self):
        source,active,receipt,saved=self.numeric_save();saved['files']=saved['files'][:1]
        builder.write(receipt,saved);self.check('FAIL')

    def test_changed_authoring_without_numeric_receipt_rejected(self):
        source,active,receipt,saved=self.numeric_save();receipt.rename(receipt.with_suffix('.fixture-backup'));self.check('FAIL')


class ClosureTests(unittest.TestCase):
    def valtan_generation_fixture(self, root, artifacts=None):
        gameplay = root / 'Server/Bin/DataFiles/Gameplay'
        directory = gameplay / 'ValtanPresentationGenerations'
        directory.mkdir(parents=True)
        payload = (json.dumps(dict(schema='lostark.valtan-presentation-generation',
                                   formatVersion=1, artifacts=artifacts or [])) + '\n').encode()
        generation_id = hashlib.sha256(payload).hexdigest()
        active = directory / (generation_id + '.json')
        active.write_bytes(payload)
        bootstrap = gameplay / 'Gameplay.bootstrap'
        bootstrap.write_text('LOSTARK_GAMEPLAY_BOOTSTRAP\t38\t1\n'
                             'PATTERNPRESENTATIONGENERATION\tENCOUNTER_VALTAN\t'
                             + generation_id + '\n', encoding='utf-8')
        return bootstrap, active

    def test_only_active_valtan_generation_and_its_references_are_collected(self):
        root = builder.OUTPUT_ROOT / ('valtan-closure-' + str(time.time_ns()))
        for relative in [*('Client/Bin/Release/' + n for n in builder.MODULES), 'Server/Bin/Release/Server.exe', 'Client/Bin/Release/Shader.cso']:
            path = root / relative; path.parent.mkdir(parents=True, exist_ok=True); path.write_bytes(b'fixture')
        docs = {
            'Data/Effects/EffectCatalog.json': {},
            'Data/Animation/Reference/active-only.json': {},
            'Data/Animation/Reference/inactive-only.json': {},
            'Server/Bin/DataFiles/Gameplay/other-runtime.json': {},
        }
        for relative, document in docs.items():
            path = root / relative; path.parent.mkdir(parents=True, exist_ok=True); builder.write(path, document)
        bootstrap, active = self.valtan_generation_fixture(root, [dict(path='Data/Animation/Reference/active-only.json')])
        inactive = active.parent / ('a' * 64 + '.json')
        builder.write(inactive, dict(artifacts=[dict(path='Data/Animation/Reference/inactive-only.json')]))
        broken = active.parent / ('b' * 64 + '.json')
        broken.write_bytes(b'inactive invalid JSON must not be parsed')
        before = {path: path.read_bytes() for path in (bootstrap, active, inactive, broken)}
        files, _ = builder.collect(root)
        generations = [path for path in files.values() if path.parent == active.parent]
        self.assertEqual(generations, [active])
        self.assertIn(bootstrap.relative_to(root).as_posix(), files)
        self.assertIn('Server/Bin/DataFiles/Gameplay/other-runtime.json', files)
        self.assertIn('Data/Animation/Reference/active-only.json', files)
        self.assertNotIn('Data/Animation/Reference/inactive-only.json', files)
        self.assertEqual(before, {path: path.read_bytes() for path in before})

    def test_valtan_generation_rejects_invalid_missing_or_mismatched_active_input(self):
        root = builder.OUTPUT_ROOT / ('valtan-admission-' + str(time.time_ns()))
        bootstrap, active = self.valtan_generation_fixture(root)
        valid = bootstrap.read_text(encoding='utf-8')
        row = valid.splitlines()[1]
        for payload in ('LOSTARK_GAMEPLAY_BOOTSTRAP\t38\t0\n',
                        valid + row + '\n',
                        valid.replace('ENCOUNTER_VALTAN', 'ENCOUNTER_OTHER'),
                        valid.replace(active.stem, '0' * 64),
                        valid.replace(active.stem, '../escape'),
                        valid.replace(active.stem, 'c' * 64)):
            with self.subTest(payload=payload):
                bootstrap.write_text(payload, encoding='utf-8')
                with self.assertRaisesRegex(AssertionError, '[Vv]altan presentation generation'):
                    builder.collect(root)
        bootstrap.write_text(valid.replace(active.stem, active.stem.upper()), encoding='utf-8')
        self.assertEqual(builder.active_valtan_presentation_generation(root), active)
        active.write_bytes(b'changed descriptor')
        with self.assertRaisesRegex(AssertionError, 'generation hash mismatch'):
            builder.collect(root)

    def test_packaged_effect_element_names_use_native_byte_limit(self):
        root = builder.OUTPUT_ROOT / ('effect-labels-' + str(time.time_ns()))
        for relative in [*('Client/Bin/Release/' + n for n in builder.MODULES), 'Server/Bin/Release/Server.exe', 'Client/Bin/Release/Shader.cso']:
            path = root / relative; path.parent.mkdir(parents=True, exist_ok=True); path.write_bytes(b'fixture')
        self.valtan_generation_fixture(root)
        effect = root / 'Data/Effects/Authored/effect.test.labels.effect.json'
        effect.parent.mkdir(parents=True)
        builder.write(root / 'Data/Effects/EffectCatalog.json', {'effects': [{'authoringPath': 'Effects/Authored/' + effect.name}]})
        document = dict(schema='lostark.effect-authoring', version=13,
                        displayName='Top-level label is permitted to exceed the element label byte limit.',
                        elements=[dict(id='label.one', displayName='가' * 21 + 'a')])
        builder.write(effect, document)
        self.assertIn(effect.relative_to(root).as_posix(), builder.collect(root)[0])
        document['elements'][0]['displayName'] += 'b'
        builder.write(effect, document)
        with self.assertRaisesRegex(RuntimeError, r'Element.*65 bytes'):
            builder.collect(root)

    def test_current_domain_new_source_literal_and_recursive_reference(self):
        root = builder.OUTPUT_ROOT / ('closure-' + str(time.time_ns()))
        for relative in [*('Client/Bin/Release/' + n for n in builder.MODULES), 'Server/Bin/Release/Server.exe', 'Client/Bin/Release/Shader.cso']:
            path = root / relative; path.parent.mkdir(parents=True, exist_ok=True); path.write_bytes(b'fixture')
        docs = {
            'Data/Effects/EffectCatalog.json': {'effects': [{'authoringPath': 'Data/Effects/Authored/new.json'}]},
            'Data/Effects/Authored/new.json': {'linked': 'Data/Animation/Reference/linked.json'},
            'Data/Animation/Reference/linked.json': {},
            'Data/Animation/Reference/unconsumed.json': {},
            'Data/UI/ServerSelect/new.json': {},
            'Data/Maps/Authoring/AREA/new.camerashots.json': {},
        }
        for relative, document in docs.items():
            path = root / relative; path.parent.mkdir(parents=True, exist_ok=True); builder.write(path, document)
        self.valtan_generation_fixture(root)
        source = root / 'Client/Private/new.cpp'; source.parent.mkdir(parents=True)
        source.write_text('Load("Maps/Authoring/AREA/new.camerashots.json");', encoding='utf-8')
        files, reasons = builder.collect(root)
        self.assertIn('Data/UI/ServerSelect/new.json', files)
        self.assertIn('Data/Animation/Reference/linked.json', files)
        self.assertIn('Data/Maps/Authoring/AREA/new.camerashots.json', files)
        self.assertNotIn('Data/Animation/Reference/unconsumed.json', files)


if __name__ == '__main__':
    unittest.main(verbosity=2)
