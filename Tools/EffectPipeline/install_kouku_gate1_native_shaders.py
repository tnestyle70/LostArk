"""Install only the reviewed Kouku 2304 group into the existing native carrier.

Run build_kouku_gate1_native_shader_programs.py first.
Existing character program groups and project registrations remain untouched.
"""
import argparse
import json
import re
import uuid
import xml.etree.ElementTree as ET
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def update(path, transform):
    original = path.read_bytes()
    newline = "\r\n" if b"\r\n" in original else "\n"
    text = original.decode("utf8").replace("\r\n", "\n")
    result = transform(text).replace("\n", newline).encode("utf8")
    if result != original:
        path.write_bytes(result)


def section(text, name, body, marker):
    begin, end = f"// BEGIN {name}\n", f"// END {name}\n"
    text = re.sub(re.escape(begin) + r".*?" + re.escape(end), "", text, flags=re.S)
    assert text.count(marker) == 1, marker
    return text.replace(marker, begin + body + end + marker, 1)


def install(source_dir, append_source_dir=None):
    contract = json.loads((source_dir / "native_runtime_contract.json").read_bytes())
    rows = contract["programs"]
    assert not contract["deferredPrograms"], "Every selected source program must be recovered."
    source = (source_dir / "Shader_EffectArtistNative.hlsli").read_text(encoding="utf8")
    append_directories = ([] if append_source_dir is None else
                          [append_source_dir] if isinstance(append_source_dir, Path) else append_source_dir)
    for directory in append_directories:
        additional = json.loads((directory / "native_runtime_contract.json").read_bytes())
        assert not additional["deferredPrograms"], "Additional source programs must be complete."
        rows += additional["programs"]
        source += "\n" + (directory / "Shader_EffectArtistNative.hlsli").read_text(encoding="utf8")
    identifiers = {r["program"] for r in rows}
    assert len(identifiers) == len(rows) and set(range(2304, 2342)) <= identifiers <= set(range(2304, 2496))
    blocks = re.findall(r"(#ifndef ARTIST_NATIVE_MODEL_ONLY\n// [^\n]+\nfloat4 ArtistNative(\d+)\(ARTIST_NATIVE_INPUT input\)\n\{.*?\n\}\n#endif)", source, re.S)
    assert len(blocks) == len(rows) and {int(i) for _, i in blocks} == identifiers
    shaders = ROOT / "Client/Bin/ShaderFiles"
    installed_group = shaders / "Shader_EffectKoukuNativeGroup2304.hlsli"
    if installed_group.is_file():
        installed = {int(value) for value in re.findall(r"float4 ArtistNative(\d+)\(", installed_group.read_text(encoding="utf8"))}
        assert installed <= identifiers, ("Supply all installed Kouku groups; refusing to remove programs", sorted(installed - identifiers))
    def carrier_for(row):
        return {"mesh": "MESH", "decal": "DECAL", "animationTrail": "TRAIL", "ribbon": "TRAIL"}.get(row["rendererShape"], "PARTICLE")
    def carrier_guard(row):
        return " && ".join(f"!defined(EFFECT_NATIVE_{kind}_CARRIER)" for kind in
            ("MESH", "PARTICLE", "DECAL", "TRAIL", "SCREEN_POST") if kind != carrier_for(row))
    by_program = {row["program"]: row for row in rows}
    if 2360 in by_program:
        def bind_macro_uv(text):
            declaration = "float4 g_ArtistSourceMacroUV;"
            if declaration not in text:
                marker = "float g_ArtistSourceMaterialTime = 0.f;"
                assert text.count(marker) == 1
                text = text.replace(marker, marker + "\n" + declaration, 1)
            return text
        update(shaders / "Shader_EffectArtistNative.hlsli", bind_macro_uv)
    if 2349 in by_program:
        assert by_program[2349].get("enginePrefixAdapter", {}).get("requiresEmitterWorldToLocal")
        def bind_world_to_local(text):
            declaration = "float4 g_ArtistSourceWorldToLocal[3];"
            if declaration not in text:
                marker = "float g_ArtistSourceMaterialTime = 0.f;"
                assert text.count(marker) == 1
                text = text.replace(marker, marker + "\n" + declaration, 1)
            return text
        update(shaders / "Shader_EffectArtistNative.hlsli", bind_world_to_local)
    assert by_program[2310].get("distortionPass"), "Disto05 requires its original accumulation pass."
    extra = ""
    if by_program[2310].get("distortionPass"):
        extra = "\n#if " + carrier_guard(by_program[2310]) + "\n" + (source_dir / "KoukuDistortionProgram.hlsli").read_text(encoding="utf8") + "#endif\n"
    (shaders / "Shader_EffectKoukuNativeGroup2304.hlsli").write_text(
        "// Original Kouku material programs; native carrier group 2304.\n" +
        "\n\n".join("#if " + carrier_guard(by_program[int(identifier)]) + "\n" + block + "\n#endif"
            for block, identifier in blocks) + extra + "\n", encoding="utf8")
    includes = '#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2304\n#include "Shader_EffectKoukuNativeGroup2304.hlsli"\n#endif\n'
    cases = ""
    for row in rows:
        cases += "#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2304) && " + carrier_guard(row) + "\n"
        opaque = "true" if row["nativeBlend"] in ("blend_additive", "blend_masked", "blend_opaque") else "false"
        if row.get("distortionPass"):
            cases += f'''    case {row["program"]}u:
    {{
        nativeColor=ArtistNative{row["program"]}(input);
        const float4 accumulated=ArtistNative{row["program"]}Distortion(input);
        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,nativeColor.a);
        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);
        return output;
    }}
#endif
'''
        else:
            cases += f'    case {row["program"]}u: nativeColor=ArtistNative{row["program"]}(input); opaqueCoverage={opaque}; break;\n#endif\n'
    update(shaders / "Shader_EffectArtistNative.hlsli", lambda text: section(
        section(text, "KOUKU NATIVE GROUP", includes, "#ifndef ARTIST_NATIVE_MODEL_ONLY\nEFFECT_PS_OUT Shade_EffectArtistNative"),
        "KOUKU NATIVE CASES", cases, "    default: clip(-1.f); return output;"))
    for carrier in ("Mesh", "Particle"):
        (shaders / f"Shader_VtxEffect{carrier}Kouku2304.hlsl").write_text(
            '#define EFFECT_SHADER_FAMILY 7\n#define EFFECT_NATIVE_PROFILE_GROUP 2304\n' +
            f'#include "Shader_Effect{carrier}FamilyCarrier.hlsli"\n', encoding="utf8")

    # Descriptor admission alone is insufficient: the vertex and pixel carrier
    # dispatch gates must reach every newly installed program as well.
    for name, expected in (("Shader_EffectMeshFamilyCarrier.hlsli", 2),
                           ("Shader_EffectParticleFamilyCarrier.hlsli", 2),
                           ("Shader_VtxEffectDecal.hlsl", 1),
                           ("Shader_VtxEffectTrail.hlsl", 1)):
        def extend_dispatch(text):
            pattern = r'g_SourceMaterialProfile >= 2304u && g_SourceMaterialProfile <= \d+u'
            assert len(re.findall(pattern, text)) == expected, name
            return re.sub(pattern, 'g_SourceMaterialProfile >= 2304u && g_SourceMaterialProfile <= 2495u', text)
        update(shaders / name, extend_dispatch)

    def table(text):
        for carrier in ("MESH", "PARTICLE"):
            label = carrier.title()
            text = re.sub(rf'EFFECT_SHADER_PROGRAM_ROW\({carrier}, ARTIST, 2304u, \d+u, "Shader_VtxEffect{label}Kouku2304.hlsl"\)',
                          f'EFFECT_SHADER_PROGRAM_ROW({carrier}, ARTIST, 2304u, 2495u, "Shader_VtxEffect{label}Kouku2304.hlsl")', text)
            row = f'        EFFECT_SHADER_PROGRAM_ROW({carrier}, ARTIST, 2304u, 2495u, "Shader_VtxEffect{label}Kouku2304.hlsl"),\n'
            if row not in text:
                text = text.replace("    }};\n#undef EFFECT_SHADER_PROGRAM_ROW", row + "    }};\n#undef EFFECT_SHADER_PROGRAM_ROW", 1)
        count = len(re.findall(r"^        EFFECT_SHADER_PROGRAM_ROW\(", text, re.M))
        return re.sub(r"std::array<EFFECT_SHADER_PROGRAM_DESC, \d+u>", f"std::array<EFFECT_SHADER_PROGRAM_DESC, {count}u>", text, count=1)
    update(ROOT / "Client/Public/Effect_ShaderFamily.h", table)

    for suffix in ("", ".filters"):
        project = ROOT / ("Client/Default/Client.vcxproj" + suffix)
        def register(text):
            additions = []
            for carrier in ("Mesh", "Particle"):
                name = f"..\\Bin\\ShaderFiles\\Shader_VtxEffect{carrier}Kouku2304.hlsl"
                if f'Include="{name}"' in text:
                    continue
                if suffix:
                    additions.append(f'    <FxCompile Include="{name}"><Filter>97.ShaderFiles</Filter></FxCompile>')
                else:
                    additions.append(f'    <FxCompile Include="{name}">\n      <DisableOptimizations Condition="\'$(Platform)\'==\'x64\'">false</DisableOptimizations>\n      <AdditionalOptions Condition="\'$(Platform)\'==\'x64\'">/O1 %(AdditionalOptions)</AdditionalOptions>\n    </FxCompile>')
            name = "..\\Bin\\ShaderFiles\\Shader_EffectKoukuNativeGroup2304.hlsli"
            if f'Include="{name}"' not in text:
                additions.append(f'    <None Include="{name}"><Filter>97.ShaderFiles</Filter></None>' if suffix else f'    <None Include="{name}" />')
            if additions:
                prefix, marker, suffix_text = text.rpartition("</Project>")
                assert marker
                text = prefix + "  <ItemGroup>\n" + "\n".join(additions) + "\n  </ItemGroup>\n" + marker + suffix_text
            xml = ET.fromstring(text)
            ns = {"m": "http://schemas.microsoft.com/developer/msbuild/2003"}
            for metadata in xml.findall("./m:ItemGroup/m:ProjectReference/m:Project", ns):
                assert len(metadata) == 0
                uuid.UUID(metadata.text.strip())
            compile_items = {item.attrib["Include"] for item in xml.findall("./m:ItemGroup/m:FxCompile", ns)}
            assert all(f"..\\Bin\\ShaderFiles\\Shader_VtxEffect{carrier}Kouku2304.hlsl" in compile_items for carrier in ("Mesh", "Particle"))
            return text
        update(project, register)
    print(f"Installed {len(rows)} Kouku native programs and two existing-family shader carriers.")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument("--source-dir", type=Path, required=True)
    parser.add_argument("--append-source-dir", type=Path, action="append", default=[])
    args = parser.parse_args()
    install(args.source_dir.resolve(), [directory.resolve() for directory in args.append_source_dir])
