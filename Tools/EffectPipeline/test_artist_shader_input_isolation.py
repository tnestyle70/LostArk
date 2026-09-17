"""An Artist install must change only the shader inputs that consume it."""
from pathlib import Path
import re
import shutil
import tempfile
import unittest

from native_shader_dispatch import (
    artist_program_selection_name, expand_dispatch_includes,
    partition_artist_runtime_source, write_artist_runtime_source,
    write_partitioned_dispatch,
)

SHADERS = Path(__file__).resolve().parents[2] / 'Client/Bin/ShaderFiles'


def copy_installed(target):
    for pattern in ('Shader_EffectArtistNative*.hlsli', 'Shader_VtxEffect*.hlsl'):
        for path in SHADERS.glob(pattern):
            shutil.copyfile(path, target / path.name)
    return target / 'Shader_EffectArtistNative.hlsli'


def snapshot(target):
    return {path.name: (path.read_bytes(), path.stat().st_mtime_ns)
            for path in target.iterdir() if path.is_file()}


def guarded_cases(text):
    guards, cases = [], {}
    for line in text.splitlines():
        stripped = line.strip()
        if stripped.startswith(('#if ', '#ifdef ', '#ifndef ')):
            guards.append(stripped)
        elif stripped == '#endif':
            guards.pop()
        else:
            match = re.match(r'case (\d+)u:', stripped)
            if match:
                identifier = int(match[1])
                if identifier in cases:
                    raise AssertionError(f'duplicate case {identifier}')
                cases[identifier] = tuple(guards)
    if guards:
        raise AssertionError('unclosed case guard')
    return cases


class ArtistShaderInputIsolation(unittest.TestCase):
    def test_installed_roundtrip_and_noop_keep_every_input(self):
        with tempfile.TemporaryDirectory() as directory:
            target = Path(directory)
            path = copy_installed(target)
            authored = expand_dispatch_includes(path.read_text(encoding='utf8'), target)
            before = snapshot(target)
            write_partitioned_dispatch(path)
            self.assertEqual(before, snapshot(target))
            self.assertEqual(authored, expand_dispatch_includes(path.read_text(encoding='utf8'), target))

    def test_new_group_does_not_edit_existing_group_or_common_inputs(self):
        with tempfile.TemporaryDirectory() as directory:
            target = Path(directory)
            path = copy_installed(target)
            source = expand_dispatch_includes(path.read_text(encoding='utf8'), target)
            marker = '// END KOUKU NATIVE GROUP\n'
            source = source.replace(marker,
                '#if !defined(ARTIST_NATIVE_MODEL_ONLY) && (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3840)\n'
                '#include "Shader_EffectKoukuNativeGroup3840.hlsli"\n#endif\n' + marker, 1)
            marker = '// END KOUKU NATIVE CASES\n'
            source = source.replace(marker,
                '#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 3840) && !defined(EFFECT_NATIVE_PARTICLE_CARRIER)\n'
                '    case 3840u: nativeColor=ArtistNative3840(input); opaqueCoverage=true; break;\n#endif\n' + marker, 1)
            before = snapshot(target)
            write_partitioned_dispatch(path, source)
            after = snapshot(target)
            changed = {name for name in before if before[name] != after[name]}
            self.assertEqual({'Shader_EffectArtistNativePrograms.hlsli'}, changed)
            self.assertIn(artist_program_selection_name(3840), after)
            self.assertEqual(source, expand_dispatch_includes(path.read_text(encoding='utf8'), target))

    def test_case_math_edit_keeps_facade_and_selection_inputs(self):
        with tempfile.TemporaryDirectory() as directory:
            target = Path(directory)
            path = copy_installed(target)
            source = expand_dispatch_includes(path.read_text(encoding='utf8'), target)
            original = 'case 2304u:'
            self.assertEqual(1, source.count(original))
            source = source.replace(original, 'case 2304u: /* local material edit */', 1)
            before = snapshot(target)
            write_partitioned_dispatch(path, source)
            after = snapshot(target)
            changed = {name for name in before if before[name] != after[name]}
            self.assertEqual({'Shader_EffectArtistNativeDispatchKoukuNativeCases2304.hlsli'}, changed)

    def test_predicate_keeps_exact_case_ids_and_carrier_guards(self):
        for path in SHADERS.glob('Shader_EffectArtistNativeSelectedGroup*.hlsli'):
            source = path.read_text(encoding='utf8')
            dispatch = source[source.index('EFFECT_PS_OUT Shade_EffectArtistNative'):]
            dispatch = dispatch[:dispatch.index('    default: clip(-1.f); return output;')]
            dispatch = dispatch[dispatch.index('    switch(profile)\n    {') + len('    switch(profile)\n    {'):]
            dispatch = re.sub(r'#include "(Shader_EffectArtistNativeDispatch\w+\.hlsli)"',
                lambda match: (SHADERS / match[1]).read_text(encoding='utf8'), dispatch)
            predicate = source[source.index('bool Has_EffectArtistNativeProfile'):]
            self.assertEqual(guarded_cases(dispatch), guarded_cases(predicate), path.name)
            self.assertIn('default: return false;', predicate)

    def test_legacy_ungrouped_reset_replaces_selected_corpus_too(self):
        source = ('#ifndef EFFECT_ARTIST_NATIVE_HLSLI\n#define EFFECT_ARTIST_NATIVE_HLSLI\n'
            'float4 g_ArtistSourceMaterialParameters[32];\nfloat g_ArtistSourceMaterialTime = 0.f;\n'
            'float4 ArtistNativeAppend(float4 a, float4 b, uint n) { return a; }\n'
            'float4 ArtistNativeSample0(float2 uv, float lod, bool explicitLod) { return 0.f; }\n'
            'float4 ArtistNative500(ARTIST_NATIVE_INPUT input) { return 1.f; }\n'
            '#ifndef ARTIST_NATIVE_MODEL_ONLY\n'
            'EFFECT_PS_OUT Shade_EffectArtistNative(uint profile, ARTIST_NATIVE_INPUT input)\n{\n'
            '    EFFECT_PS_OUT output=(EFFECT_PS_OUT)0;\n    float4 nativeColor=0.f;\n'
            '    switch(profile)\n    {\n#if !defined(EFFECT_NATIVE_PARTICLE_CARRIER)\n'
            '    case 500u: nativeColor=ArtistNative500(input); break;\n#endif\n'
            '    default: clip(-1.f); return output;\n    }\n    return output;\n}\n#endif\n#endif\n')
        with tempfile.TemporaryDirectory() as directory:
            target = Path(directory)
            path = copy_installed(target)
            write_artist_runtime_source(path, source)
            self.assertEqual(source, expand_dispatch_includes(path.read_text(encoding='utf8'), target))
            for selected in target.glob('Shader_EffectArtistNativeSelectedGroup*.hlsli'):
                text = selected.read_text(encoding='utf8')
                self.assertIn('case 500u:', text)
                self.assertNotIn('case 2304u:', text)

    def test_invalid_selector_fails_before_any_write(self):
        with tempfile.TemporaryDirectory() as directory:
            target = Path(directory)
            path = copy_installed(target)
            source = expand_dispatch_includes(path.read_text(encoding='utf8'), target)
            source = source.replace('EFFECT_NATIVE_PROFILE_GROUP == 448',
                                    'EFFECT_NATIVE_PROFILE_GROUP == UNKNOWN_GROUP', 1)
            before = snapshot(target)
            with self.assertRaisesRegex(ValueError, 'Unsupported Artist group selector'):
                partition_artist_runtime_source(source, target)
            self.assertEqual(before, snapshot(target))


if __name__ == '__main__':
    unittest.main()
