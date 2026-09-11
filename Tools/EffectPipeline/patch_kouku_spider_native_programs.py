"""Complete the selected spider WorldOffset02 pixel ABI after native lowering.

The shared generator owns the material equations. This source-qualified patch
only supplies the engine prefix and world-position varying for program 2349.
The existing particle renderer must bind the emitter inverse to
g_ArtistSourceWorldToLocal, as it already does for DimensionMaster 341/361.
"""
import argparse
import hashlib
import json
import re
from pathlib import Path

PROGRAM = 2349
MATERIAL = "fx_m_mi_03.fx_mi.fx_m_pa_worldoffset_02_2_tr"
PIXEL_SHADER = "a0f7f5723e826143b3970e44f6a045f2"
VERTEX_SHADER = "1933037c8b482b47b9c7f17238734ad2"
MAP_KEY = "556a53205748259b14772cac15cc51ca4b99fc29a3e8ad2940bdffdafc2ff11e"
DECLARATION = "float4 g_ArtistSourceWorldToLocal[3];"
OLD_PREFIX = "    source[0].x=1.f; // Project engine opacity multiplier."
NEW_PREFIX = "\n".join((
    "    // CameraWorldPos is precomposed into the absolute source-centimetre varying.",
    "    // The engine opacity lane is W; WorldToLocal belongs to the emitter batch.",
    "    source[0]=float4(0.f,0.f,0.f,1.f);",
    "    source[1]=g_ArtistSourceWorldToLocal[0];",
    "    source[2]=g_ArtistSourceWorldToLocal[1];",
    "    source[3]=g_ArtistSourceWorldToLocal[2];",
))
OLD_POSITION = ("    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+"
    "float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW);"
    " // native texcoord5")
NEW_POSITION = ("    float4 v7 = float4(input.sourceWorldPosition,1.f);"
    " // native camera-relative world + CameraWorldPos")


def read(path):
    return json.loads(path.read_text(encoding="utf-8-sig"))


def function_blocks(text):
    return {int(number): block for block, number in re.findall(
        r"(float4 ArtistNative(\d+)\(ARTIST_NATIVE_INPUT input\)\n\{.*?\n\})",
        text, re.S)}


def patch(source_dir):
    contract_path = source_dir / "native_runtime_contract.json"
    contract = read(contract_path)
    row = next(item for item in contract["programs"] if item["program"] == PROGRAM)
    assert (row["sourceMaterial"], row["sourcePS"], row["sourceVS"], row["rendererShape"]) == (
        MATERIAL, PIXEL_SHADER, VERTEX_SHADER, "sprite"), "WorldOffset02 source selection changed."
    materials = read(source_dir / "native_material_inputs.json")["materials"]
    material = next(item for item in materials if item["sourceMaterial"] == MATERIAL)
    assert material["mapKey"] == MAP_KEY, "WorldOffset02 static permutation changed."
    counts = material["materialMap"]["uniformExpressionCounts"]
    assert all(counts[name] == 0 for name in (
        "vertexVectorExpressions", "vertexScalarExpressions", "vertexTexture2DExpressions")), (
        "WorldOffset02 now requires a vertex-material program.")
    pixel = read(source_dir / "full_programs" / (PIXEL_SHADER + ".json"))
    vertex = read(source_dir / "full_programs" / (VERTEX_SHADER + ".json"))
    assert pixel["bindings"]["constantBufferClosure"]["leadingUnownedConstantBuffer0Slots"] == [0, 1, 2, 3]
    assert pixel["disassembly"]["instructions"][0] == "add r0.xyz, v7.xyzx, cb0[0].xyzx"
    assert "mul o0.w, r0.x, cb0[0].w" in pixel["disassembly"]["instructions"]
    assert "mov o7.xyzw, r3.xyzw" in vertex["disassembly"]["instructions"]
    assert not vertex["disassembly"]["sampleInstructions"]

    shader_path = source_dir / "Shader_EffectArtistNative.hlsli"
    original = shader_path.read_text(encoding="utf8")
    before = function_blocks(original)
    block = before[PROGRAM]
    if NEW_PREFIX not in block:
        assert block.count(OLD_PREFIX) == 1
        block = block.replace(OLD_PREFIX, NEW_PREFIX)
    if NEW_POSITION not in block:
        assert block.count(OLD_POSITION) == 1
        block = block.replace(OLD_POSITION, NEW_POSITION)
    result = original.replace(before[PROGRAM], block, 1)
    if DECLARATION not in result:
        marker = "float g_ArtistSourceMaterialTime = 0.f;"
        assert result.count(marker) == 1
        result = result.replace(marker, marker + "\n" + DECLARATION, 1)
    after = function_blocks(result)
    assert before.keys() == after.keys()
    assert all(before[number] == after[number] for number in before if number != PROGRAM)
    assert OLD_PREFIX not in after[PROGRAM] and OLD_POSITION not in after[PROGRAM]
    if result != original:
        shader_path.write_text(result, encoding="utf8")
    row["enginePrefixAdapter"] = {
        "cameraWorldPosition": "precomposed absolute source-centimetre world varying; CB0[0].xyz = 0",
        "opacity": "CB0[0].w = 1; source particle alpha remains in TEXCOORD1.w",
        "worldToLocal": "CB0[1..3] = g_ArtistSourceWorldToLocal; inverse SourceEmitterWorld in source basis",
        "existingRuntimeReference": "DimensionMaster native 341/361 particle WorldToLocal binding",
        "requiresEmitterWorldToLocal": True,
    }
    contract_path.write_text(json.dumps(contract, indent=2) + "\n", encoding="utf8")
    receipt = {
        "program": PROGRAM,
        "sourcePS": PIXEL_SHADER,
        "sourceVS": VERTEX_SHADER,
        "materialMap": MAP_KEY,
        "preservedOtherProgramCount": len(before) - 1,
        "shaderSha256": hashlib.sha256(shader_path.read_bytes()).hexdigest(),
        "runtimeBindingRequired": DECLARATION,
        "runtimeBindingOwner": "particle emitter inverse; never the billboard instance inverse",
        "visualReview": "USER_REVIEW_PENDING",
    }
    (source_dir / "native_source_abi_patch.json").write_text(
        json.dumps(receipt, indent=2) + "\n", encoding="utf8")
    print("Kouku WorldOffset02 ABI patched; peer functions preserved:", len(before) - 1)
    return receipt


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-dir", type=Path, required=True)
    patch(parser.parse_args().source_dir.resolve())
