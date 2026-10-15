from pathlib import Path
import importlib.util
import sys
import tempfile
import unittest

from cpp_source_domains import cpp_domain_paths, cpp_domain_header_paths, cpp_function_body, cpp_function_definition, read_cpp_domain, read_source_text

ROOT = Path(__file__).resolve().parents[2]


class CppSourceDomainTests(unittest.TestCase):
    def setUp(self):
        output = ROOT / 'out'
        output.mkdir(exist_ok=True)
        self.temporary = tempfile.TemporaryDirectory(prefix='source-domain-fixture-', dir=output)
        self.root = Path(self.temporary.name)
        self.project_root = self.root / 'Client'
        self.private = self.project_root / 'Private'
        self.private.mkdir(parents=True)
        (self.project_root / 'Default').mkdir()
        self.project = self.project_root / 'Default/Client.vcxproj'

    def tearDown(self):
        self.temporary.cleanup()

    def project_with(self, entries):
        self.project.write_text(
            '<Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003"><ItemGroup>' +
            ''.join(f'<{kind} Include="..\\Private\\{name}" />' for kind, name in entries) +
            '</ItemGroup></Project>', encoding='utf-8')

    def test_registration_limits_source_scope_and_keeps_private_constants(self):
        for name, text in (
            ('Effect_Tool.cpp', 'void Core() {}'),
            ('Effect_Tool_Catalog.cpp', 'void Catalog() {} // 새 UTF-8 구현'),
            ('Effect_Tool_Unregistered.cpp', 'do_not_read_unregistered();'),
            ('Effect_Tool_V2.cpp', 'do_not_read_another_owner();'),
            ('Effect_Tool_Internal.h', 'constexpr int PROFILE = 7;'),
        ):
            (self.private / name).write_text(text, encoding='utf-8')
        self.project_with([('ClCompile', 'Effect_Tool.cpp'), ('ClCompile', 'Effect_Tool_Catalog.cpp'),
                           ('ClCompile', 'Effect_Tool_V2.cpp'), ('ClInclude', 'Effect_Tool_Internal.h')])
        source = read_cpp_domain(self.project_root, 'Effect_Tool')
        self.assertIn('void Core()', source)
        self.assertIn('새 UTF-8 구현', source)
        self.assertIn('PROFILE = 7', source)
        self.assertNotIn('do_not_read', source)
        self.assertEqual(len(cpp_domain_paths(self.project_root, 'Effect_Tool')), 2)
        self.assertEqual(len(cpp_domain_header_paths(self.project_root, 'Effect_Tool')), 1)

    def test_missing_and_duplicate_registered_sources_are_errors(self):
        (self.private / 'MapTool.cpp').write_text('void Core() {}', encoding='utf-8')
        self.project_with([('ClCompile', 'MapTool.cpp'), ('ClCompile', 'MapTool_Missing.cpp')])
        with self.assertRaises(FileNotFoundError):
            read_cpp_domain(self.project_root, 'MapTool')
        self.project_with([('ClCompile', 'MapTool.cpp'), ('ClCompile', 'MapTool.cpp')])
        with self.assertRaises(ValueError):
            read_cpp_domain(self.project_root, 'MapTool')

    def test_mixed_source_encoding_and_physical_metadata_read(self):
        (self.private / 'Animation_Tool.cpp').write_bytes('// 기존 CP949\r\n'.encode('cp949'))
        (self.private / 'Animation_Tool_Catalog.cpp').write_text('// 새 UTF-8\n', encoding='utf-8')
        self.project_with([('ClCompile', 'Animation_Tool.cpp'), ('ClCompile', 'Animation_Tool_Catalog.cpp')])
        source = read_source_text(self.private / 'Animation_Tool.cpp', encoding='utf-8')
        self.assertIn('기존 CP949', source)
        self.assertIn('새 UTF-8', source)
        metadata = self.project_root / 'Public.h'
        metadata.write_bytes(b'caf\xe9')
        self.assertEqual(read_source_text(metadata, encoding='latin-1'), 'café')
        self.assertEqual(read_source_text(self.project, encoding='utf-8'), self.project.read_text(encoding='utf-8'))

    def test_body_parser_ignores_string_and_comment_braces(self):
        body = '{ auto s = R"tag({quoted})tag"; /* } */ return std::string("{"); }'
        source = 'std::string Wanted();\nvoid Later() {}\nstd::string Wanted() ' + body + '\nvoid Earlier() {}'
        self.assertEqual(cpp_function_body(source, 'std::string Wanted()'), body)
        self.assertEqual(cpp_function_definition(source, 'std::string Wanted()'), 'std::string Wanted() ' + body)

    def test_active_effect_guard_detects_forbidden_input_in_split_unit(self):
        (self.private / 'Effect_Tool.cpp').write_text('void Core() {}', encoding='utf-8')
        helper = self.private / 'Effect_Tool_Catalog.cpp'
        helper.write_text('const char* path = "EffectCatalog.runtime.json";', encoding='utf-8')
        (self.private / 'Effect_Catalog.cpp').write_text(
            'bool_t Client::CEffectCatalog::Load() {}\n'
            'bool_t Client::CEffectCatalog::Stage_DebugDirectAuthoredReplacement() {}', encoding='utf-8')
        self.project_with([('ClCompile', 'Effect_Tool.cpp'), ('ClCompile', 'Effect_Tool_Catalog.cpp')])
        pipeline = ROOT / 'Tools/EffectPipeline'
        sys.path.insert(0, str(pipeline))
        try:
            spec = importlib.util.spec_from_file_location('source_domain_effect_guard', pipeline / 'validate_effect_sources.py')
            module = importlib.util.module_from_spec(spec)
            sys.modules[spec.name] = module
            spec.loader.exec_module(module)
            with self.assertRaisesRegex(module.ContractError, 'retired runtime input'):
                module._validate_active_consumer_guard(self.root)
            helper.write_text('void LoadCurrentAuthoring() {}', encoding='utf-8')
            module._validate_active_consumer_guard(self.root)
        finally:
            sys.path.remove(str(pipeline))
            sys.modules.pop('source_domain_effect_guard', None)


class CppFunctionBoundsTests(unittest.TestCase):
    def test_template_return_type_comma_precedes_the_parameter_list(self):
        signature = 'std::unordered_map<std::string, f32_t>\n\tCollectModelClipSourceDurationSeconds('
        body = '{ return durations; }'
        source = signature + 'const Model& model);\n' + signature + 'const Model& model) ' + body
        self.assertEqual(cpp_function_body(source, signature), body)

    def test_empty_braced_default_is_not_a_definition_body(self):
        signature = 'ResolveEffectV2Binding('
        declaration = 'const Binding* ' + signature + 'const Snapshot&, std::string_view id = {});'
        body = '{ return BuildEffectV2BindingStableId(snapshot, id); }'
        definition = 'const Binding* ' + signature + 'const Snapshot& snapshot, std::string_view id = {}) ' + body
        source = declaration + '\nvoid Other() {}\n' + definition
        self.assertEqual(cpp_function_body(source, signature), body)
        self.assertEqual(cpp_function_definition(source, signature), definition[definition.index(signature):])

    def test_nested_braced_arguments_and_lambda_defaults_stay_in_signature(self):
        signature = 'void Wanted('
        parameters = '(Rows rows = {{1, 2}, {3, 4}}, Factory factory = []() { return Item{4}; })'
        body = '{ Consume(rows, factory()); }'
        source = 'void Wanted' + parameters + ';\nvoid Wanted' + parameters + ' ' + body
        self.assertEqual(cpp_function_body(source, signature), body)
        self.assertEqual(cpp_function_definition(source, signature), 'void Wanted' + parameters + ' ' + body)

    def test_signature_text_in_comments_or_literals_is_not_a_definition(self):
        signature = 'Wanted('
        body = '{ return value; }'
        source = '''// Wanted() { wrong(); }
/* Wanted() { wrong(); } */
const char* quoted = "Wanted() { wrong(); }";
const char* raw = u8R"tag(Wanted() { wrong(); })tag";
int Wanted(int value = 3) ''' + body
        self.assertEqual(cpp_function_body(source, signature), body)

    def test_signature_literals_comments_and_noexcept_do_not_supply_body_brace(self):
        signature = 'auto Wanted('
        parameters = '''const char* text = "} ; {", char brace = '{',
    const char* raw = R"limit( ) ; {} Wanted( )limit",
    int count = 4 /* ) { ; */) const noexcept(noexcept(Probe{})) -> Result'''
        body = '{ return Result{text, brace, count}; }'
        source = signature + parameters + ' // { ; }\n' + body
        self.assertEqual(cpp_function_body(source, signature), body)

    def test_raw_literal_prefixes_keep_internal_quotes_and_signature_text_masked(self):
        body = '{ return actual; }'
        for prefix in ('R', 'u8R', 'uR', 'UR', 'LR'):
            with self.subTest(prefix=prefix):
                source = ('auto raw = ' + prefix + '"tag(a " } " Wanted() {)tag";\n'
                          'int Wanted() ' + body)
                self.assertEqual(cpp_function_body(source, 'Wanted('), body)

    def test_calls_and_declarations_are_skipped_before_definition(self):
        signature = 'Wanted('
        body = '{ return ready; }'
        source = '''int Wanted(Rows rows = {});
void Caller() {
    Wanted({});
    if (Wanted({}) && ready) { wrong(); }
    Consume(Wanted({}), [] { wrong(); });
    const auto result = Wanted({}) ? Item{1} : Item{2};
}
int Wanted(Rows ready) ''' + body
        self.assertEqual(cpp_function_body(source, signature), body)

    def test_body_offsets_preserve_escaped_literals_comments_and_digit_separators(self):
        signature = 'void Wanted()'
        body = r'''{ auto quote = "escaped \" }"; char c = '\'';
    auto count = 1'000 + 0xFF'00;
    /* } */ // }
    auto raw = R"tag({ })tag"; }'''
        source = signature + ' ' + body + '\nvoid Later() {}'
        self.assertEqual(cpp_function_body(source, signature), body)
        self.assertEqual(cpp_function_definition(source, signature), signature + ' ' + body)

    def test_missing_definition_and_unterminated_body_fail(self):
        for source in ('void Wanted(Rows rows = {});\nvoid Other() {}',
                       'void Caller() { Wanted({}); }', 'void Wanted() {'):
            with self.subTest(source=source), self.assertRaises(ValueError):
                cpp_function_body(source, 'Wanted(')


if __name__ == '__main__':
    unittest.main()
