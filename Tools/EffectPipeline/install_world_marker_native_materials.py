"""Install World marker source materials through the existing native carriers.

Only programs 2351..2359 belong to this installer. Four identical source
materials retain their existing native programs. Source shader extraction and
World Resources installation must already have completed; no placeholder shader
or texture is generated here.
"""
import argparse
import copy
import json
import re
import shutil
import subprocess
import sys
from pathlib import Path

import install_kouku_gate1_native_materials as tables

ROOT = Path(__file__).resolve().parents[2]
SHADERS = ROOT / "Client/Bin/ShaderFiles"
REUSE = {
    "fx_m_mi_03.fx_mi.fx_d_pa_ring_11_09_ts_tr": (511, "artist", "effect.artist.skill.31910.full.restore.effect.json", "Shader_EffectArtistNativeGroup448.hlsli"),
    "bfx_m_mi_00.bfx_m.bfx_i_pa_backglow_cl_02_tr": (850, "artist", "effect.artist.skill.31930.full.restore.effect.json", "Shader_EffectArtistNativeGroup832.hlsli"),
    "fx_m_mi_05.fx_mi.fx_e_pa_gl_07_1_ad": (862, "artist", "effect.artist.skill.31930.full.restore.effect.json", "Shader_EffectArtistNativeGroup832.hlsli"),
    "bfx_m_mi_00.bfx_m.bfx_i_pa_glow_01_ad": (2343, "kouku", None, "Shader_EffectKoukuNativeGroup2304.hlsli"),
}
OWNED = {
    "fx_m_mi_05.fx_m.fx_b_me_pickingarrow_01_ad": 2351,
    "fx_m_mi_04.fx_mi.fx_e_de_ri_12_1_ad": 2352,
    "fx_m_mi_04.fx_mi.fx_e_de_ri_11_1_ad": 2353,
    "fx_m_mi_00.fx_mi.fx_a_pa_gl_01_1_tr": 2354,
    "fx_m_mi_05.fx_m.fx_e_pa_mask_01_ad": 2355,
    "fx_m_mi_05.fx_m.fx_b_me_quest_01_op": 2356,
    "bfx_m_mi_00.bfx_mi.bfx_i_pa_glow_03_ad": 2357,
    "bfx_m_mi_00.bfx_mi.bfx_f_pa_ht_02_2_ad": 2358,
    "fx_m_mi_03.fx_mi.fx_d_pa_ring_07_20_ad": 2359,
}
DISTORTION_MATERIAL = "bfx_m_mi_00.bfx_mi.bfx_f_pa_ht_02_2_ad"
DISTORTION_PS = "e49c3f5d391ee74a8d37d9251055e1e6"
DISTORTION_VS = "68f9a531df3afb44abdebb9954c4ff42"


def walk(value):
    if isinstance(value, dict):
        yield value
        for child in value.values():
            yield from walk(child)
    elif isinstance(value, list):
        for child in value:
            yield from walk(child)


def edit_preserving(path, transform):
    raw = path.read_bytes()
    bom = b"\xef\xbb\xbf" if raw.startswith(b"\xef\xbb\xbf") else b""
    text = raw[len(bom):].decode("utf-8")
    newline = "\r\n" if "\r\n" in text else "\n"
    updated = transform(text.replace("\r\n", "\n")).replace("\n", newline)
    assert path.read_bytes() == raw, f"Concurrent edit: {path}"
    result = bom + updated.encode("utf-8")
    if result != raw:
        path.write_bytes(result)


def replace_section(text, label, body, anchor):
    begin, end = f"// BEGIN {label}\n", f"// END {label}\n"
    block = begin + body.rstrip() + "\n" + end
    if begin in text:
        assert text.count(begin) == text.count(end) == 1
        return re.sub(re.escape(begin) + ".*?" + re.escape(end), lambda _: block, text, flags=re.S)
    assert text.count(anchor) == 1
    return text.replace(anchor, anchor + block)


def carrier_guard(shape):
    kind = {"mesh": "MESH", "decal": "DECAL", "sprite": "PARTICLE"}[shape]
    return " && ".join(f"!defined(EFFECT_NATIVE_{other}_CARRIER)"
                       for other in ("MESH", "PARTICLE", "DECAL", "TRAIL", "SCREEN_POST") if other != kind)


def install_shaders(stage, programs):
    generated = (stage / "Shader_EffectArtistNative.hlsli").read_text(encoding="utf-8")
    blocks, cases = [], []
    for row in programs:
        number = row["program"]
        pattern = rf"#ifndef ARTIST_NATIVE_MODEL_ONLY\n// [^\n]+\nfloat4 ArtistNative{number}\(ARTIST_NATIVE_INPUT input\)\n\{{.*?\n\}}\n#endif"
        found = re.findall(pattern, generated, re.S)
        assert len(found) == 1, number
        guard = carrier_guard(row["rendererShape"])
        blocks.append(f"#if {guard}\n{found[0]}\n#endif\n")
        coverage = str(row["nativeBlend"] in ("blend_additive", "blend_opaque", "blend_masked")).lower()
        cases.append(f"#if (!defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2304) && {guard}\n")
        if row.get("distortionPass"):
            distortion = (stage / "WorldDistortionProgram.hlsli").read_text(encoding="utf-8")
            blocks.append(f"#if {guard}\n{distortion}\n#endif\n")
            cases.append(f"    case {number}u:\n    {{\n"
                         f"        nativeColor=ArtistNative{number}(input);\n"
                         f"        const float4 accumulated=ArtistNative{number}Distortion(input);\n"
                         f"        output.SceneColor=float4(nativeColor.rgb*g_EmissiveIntensity,{coverage} ? 1.f : nativeColor.a);\n"
                         "        output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);\n"
                         "        if(g_ColorClip>0.f) clip(output.SceneColor.a-g_ColorClip);\n"
                         "        return output;\n    }\n")
        else:
            cases.append(f"    case {number}u: nativeColor=ArtistNative{number}(input); opaqueCoverage={coverage}; break;\n")
        cases.append("#endif\n")
    destination = SHADERS / "Shader_EffectWorldNative.hlsli"
    destination.write_text("// World marker RT0 programs lowered from the original material shader maps.\n"
                           "// Particle color alpha is a signed material input for picking and decal waves.\n"
                           + "\n".join(blocks), encoding="utf-8", newline="\n")
    def include(text):
        text = replace_section(text, "WORLD NATIVE GROUP",
            '#if !defined(EFFECT_NATIVE_PROFILE_GROUP) || EFFECT_NATIVE_PROFILE_GROUP == 2304\n'
            '#include "Shader_EffectWorldNative.hlsli"\n#endif\n', "// END KOUKU NATIVE GROUP\n")
        return replace_section(text, "WORLD NATIVE CASES", "".join(cases), "// END KOUKU NATIVE CASES\n")
    edit_preserving(SHADERS / "Shader_EffectArtistNative.hlsli", include)


def reuse_material(selection, native, header):
    material_id = selection["resolvedMaterial"]
    number, domain, document_name, shader_name = REUSE[material_id]
    runtime = f"effect.ue3.{domain}-{number}-native.v1"
    # The installed function must be the exact selected PS, not a similarly
    # named parent material. Source child identity is checked independently.
    shader = (SHADERS / shader_name).read_text(encoding="utf-8")
    assert re.search(r"// [^\n]*" + re.escape(selection["sourcePS"]) +
                     rf"[^\n]*\nfloat4 ArtistNative{number}\(", shader)
    entry = re.search(rf'^    \{{{number}u,.*$', header, re.M)
    assert entry and f'"{runtime}","{material_id}","{native["parentMaterial"]}"' in entry[0]
    documents = [ROOT / "Data/Effects/Authored" / document_name] if document_name else sorted((ROOT / "Data/Effects/Authored").glob("effect.kouku*.effect.json"))
    candidates = []
    for document in documents:
        for value in walk(tables.read(document)):
            if value.get("sourceMaterialPath") == material_id and value.get("sourceProfile", {}).get("runtimeShaderProfileId") == runtime:
                candidates.append(value)
        if candidates:
            break
    assert candidates, f"Existing material document missing: {material_id}"
    material = copy.deepcopy(candidates[0])
    profile = material["sourceProfile"]
    assert profile["parentMaterialPath"] == native["parentMaterial"]
    assert not native["effectiveTextures"] and not profile["textures"]
    defaults = {"scalars": {}, "vectors": {}}
    for node in walk(native["materialMap"]["uniformExpressionSet"]):
        group = {"fmaterialuniformexpressionscalarparameter": "scalars",
                 "fmaterialuniformexpressionvectorparameter": "vectors"}.get(node.get("typeName"))
        if group:
            name = node["parameterName"]
            assert node.get("parameterNameNumber", 0) == 0
            assert name not in defaults[group] or defaults[group][name] == node["defaultValue"]
            defaults[group][name] = node["defaultValue"]
    for group in defaults:
        assert set(defaults[group]) == {p["name"] for p in profile[group]}
        for value in profile[group]:
            value["value"] = native["effectiveNumericOverrides"][group].get(value["name"], {}).get("value", defaults[group][value["name"]])
    switches = {s["parameterName"]: s["value"] for s in native.get("mic", {}).get("staticParameterSet", {}).get("staticSwitchParameters", [])}
    assert switches == {s["name"]: s["value"] for s in profile["staticSwitches"]}
    return dict(program=number, sourceMaterial=material_id, rendererShape=selection["rendererShape"],
                occurrences=selection["occurrences"], material=material, reuse="exact-source-material-and-selected-pixel-shader")


def prepare_distortion(evidence, stage, selected, contract):
    selection = copy.deepcopy(next(p for p in selected if p["resolvedMaterial"] == DISTORTION_MATERIAL))
    selection.update(sourcePS=DISTORTION_PS, sourceVS=DISTORTION_VS)
    folder = stage / "distortion"
    (folder / "full_programs").mkdir(parents=True, exist_ok=True)
    for name in ("native_material_inputs.json", "texture_asset_map.json"):
        shutil.copyfile(evidence / name, folder / name)
    for shader_id in (DISTORTION_PS, DISTORTION_VS):
        shutil.copyfile(evidence / "full_programs" / (shader_id + ".json"), folder / "full_programs" / (shader_id + ".json"))
    tables.write(folder / "selected_runtime_material_programs.json", {"programs": [selection]})
    subprocess.run([sys.executable, str(ROOT / "Tools/EffectPipeline/generate_artist_native_runtime_shader.py"),
                    "--source-dir", str(folder), "--program-start", "2358", "--profile-domain", "kouku"], check=True)
    extra = tables.read(folder / "native_runtime_contract.json")
    assert not extra["deferredPrograms"] and len(extra["programs"]) == 1, extra["deferredPrograms"]
    distortion = extra["programs"][0]
    color = next(row for row in contract["programs"] if row["program"] == 2358)
    assert color["parameters"] == distortion["parameters"] and color["textures"] == distortion["textures"]
    color["requiresDepthSample"] |= distortion["requiresDepthSample"]
    color["requiresSceneColor"] |= distortion["requiresSceneColor"]
    color["distortionPass"] = dict(sourcePS=DISTORTION_PS, sourceVS=DISTORTION_VS,
        depthAdapter=distortion["depthAdapter"],
        outputAdapter="Original positive XY minus negative ZW into the existing signed distortion target; source-pass discard becomes zero accumulation, preserving the separate color result.")
    generated = (folder / "Shader_EffectArtistNative.hlsli").read_text(encoding="utf-8")
    block = re.search(r"#ifndef ARTIST_NATIVE_MODEL_ONLY\n// [^\n]+\nfloat4 ArtistNative2358\(ARTIST_NATIVE_INPUT input\)\n\{.*?\n\}\n#endif", generated, re.S)
    assert block
    # The source rendered color and distortion in separate passes. A discard
    # from the distortion pass must not discard the color MRT in our combined
    # carrier. Zero is the identity of the additive signed distortion target.
    body = block[0].replace("ArtistNative2358(", "ArtistNative2358Distortion(")
    assert body.count("clip(-1.f);") == 1
    body = body.replace("clip(-1.f);", "return 0.f;")
    (stage / "WorldDistortionProgram.hlsli").write_text(body + "\n", encoding="utf-8")


def install(evidence, generate_only=False):
    selections = tables.read(evidence / "selected_runtime_material_programs.json")["programs"]
    assert {p["resolvedMaterial"] for p in selections} == set(OWNED) | set(REUSE)
    selected = sorted((p for p in selections if p["resolvedMaterial"] in OWNED), key=lambda p: OWNED[p["resolvedMaterial"]])
    stage = evidence / "world_programs"
    (stage / "full_programs").mkdir(parents=True, exist_ok=True)
    for name in ("native_material_inputs.json", "texture_asset_map.json"):
        shutil.copyfile(evidence / name, stage / name)
    for row in selected:
        for shader_id in (row["sourcePS"], row["sourceVS"]):
            shutil.copyfile(evidence / "full_programs" / (shader_id + ".json"), stage / "full_programs" / (shader_id + ".json"))
    tables.write(stage / "selected_runtime_material_programs.json", {"programs": selected})
    subprocess.run([sys.executable, str(ROOT / "Tools/EffectPipeline/generate_artist_native_runtime_shader.py"),
                    "--source-dir", str(stage), "--program-start", "2351", "--profile-domain", "kouku"], check=True)
    contract_path = stage / "native_runtime_contract.json"
    contract = tables.read(contract_path)
    assert not contract["deferredPrograms"], contract["deferredPrograms"]
    assert {p["sourceMaterial"]: p["program"] for p in contract["programs"]} == OWNED
    prepare_distortion(evidence, stage, selected, contract)
    tables.write(contract_path, contract)
    if generate_only:
        print("Prepared World color and distortion programs; product files were not changed.")
        return
    header_path = ROOT / "Client/Public/Effect_ArtistMaterial.h"
    header = header_path.read_text(encoding="utf-8-sig")
    for material, number in OWNED.items():
        old = re.search(rf'^    \{{{number}u,.*$', header, re.M)
        assert old is None or f'"{material}"' in old[0], f"Native slot is owned by another material: {number}"
    native = {row["sourceMaterial"]: row for row in tables.read(evidence / "native_material_inputs.json")["materials"]}
    reused = [reuse_material(row, native[row["resolvedMaterial"]], header) for row in selections if row["resolvedMaterial"] in REUSE]
    tables.install(contract_path, stage, header_path)
    install_shaders(stage, contract["programs"])
    materials = tables.read(stage / "native_material_patch.json")["programs"] + reused
    assert len(materials) == 13 and sum(len(row["occurrences"]) for row in materials) == 16
    tables.write(evidence / "native_material_patch.json", {"programs": materials})
    tables.write(evidence / "world_native_installation.json", {
        "newPrograms": sorted(OWNED.values()), "reusedPrograms": sorted(row["program"] for row in reused),
        "occurrences": 16, "sourcePixelPrograms": [{"program": p["program"], "sourcePS": p["sourcePS"],
            "sourceVS": p["sourceVS"], "rt0InstructionCount": p["nativeRT0InstructionCount"]} for p in contract["programs"]],
        "vertexAdapter": "Existing local/decal/particle vertex factory carriers; selected World mesh VS programs contain no WPO.",
        "pickingAlpha": "Signed mesh particle color alpha drives source UV.y minus alpha; it is not opacity.",
        "questPrefix": "Opaque mesh source CB0[0] is particle color; CB0[1] is the material selectioncolor vector.",
        "questOutputAdapter": "Original emissive RT0 in the existing forward mesh carrier; native GBuffer-only outputs are not submitted.",
        "distortionPass": next(p["distortionPass"] for p in contract["programs"] if p["program"] == 2358),
        "validation": "Source/descriptor installation only; compile and numeric runtime checks are separate. User visual validation pending."})
    print("World native materials: 9 new programs, 4 exact reuses, 16 occurrences.")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-dir", type=Path, default=ROOT / "out/WorldMarkers20260911/native")
    parser.add_argument("--generate-only", action="store_true", help="Prepare only out artifacts while a product build is running.")
    arguments = parser.parse_args()
    install(arguments.source_dir.resolve(), arguments.generate_only)
