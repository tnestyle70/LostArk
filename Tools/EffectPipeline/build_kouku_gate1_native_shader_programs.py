"""Lower both original color and distortion passes of the two Gate1 effects."""
import argparse
import copy
import json
import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
DISTORTION_PS = "56fd4335bcbc2843a7577f32fab7249e"
DISTORTION_VS = "68f9a531df3afb44abdebb9954c4ff42"


def generate(directory, first):
    subprocess.run([sys.executable, str(ROOT / "Tools/EffectPipeline/generate_artist_native_runtime_shader.py"),
        "--source-dir", str(directory), "--program-start", str(first), "--profile-domain", "kouku"], check=True)


def build(directory):
    shader = directory / "full_programs" / (DISTORTION_PS + ".json")
    assert shader.is_file(), "Original Disto05 accumulation shader must be recovered."
    generate(directory, 2304)
    selections = json.loads((directory / "selected_runtime_material_programs.json").read_bytes())
    selected = copy.deepcopy(selections["programs"][6])
    assert selected["resolvedMaterial"] == "fx_mastermaterial.fx_mi.fx_c_pa_dist_05_ad"
    selected.update(sourcePS=DISTORTION_PS, sourceVS=DISTORTION_VS)
    extra = directory / "Distortion"
    (extra / "full_programs").mkdir(parents=True, exist_ok=True)
    for name in ("native_material_inputs.json", "texture_asset_map.json"):
        shutil.copyfile(directory / name, extra / name)
    shutil.copyfile(shader, extra / "full_programs" / shader.name)
    (extra / "selected_runtime_material_programs.json").write_text(json.dumps({"programs": [selected]}), encoding="utf8")
    generate(extra, 2310)
    extra_contract = json.loads((extra / "native_runtime_contract.json").read_bytes())
    assert not extra_contract["deferredPrograms"] and len(extra_contract["programs"]) == 1
    source = (extra / "Shader_EffectArtistNative.hlsli").read_text(encoding="utf8")
    block = re.search(r"#ifndef ARTIST_NATIVE_MODEL_ONLY\n// [^\n]+\nfloat4 ArtistNative2310\(ARTIST_NATIVE_INPUT input\)\n\{.*?\n\}\n#endif", source, re.S)
    assert block
    (directory / "KoukuDistortionProgram.hlsli").write_text(block[0].replace("ArtistNative2310(", "ArtistNative2310Distortion(") + "\n", encoding="utf8")
    contract_path = directory / "native_runtime_contract.json"
    contract = json.loads(contract_path.read_bytes())
    row = next(r for r in contract["programs"] if r["program"] == 2310)
    distortion = extra_contract["programs"][0]
    assert row["parameters"] == distortion["parameters"] and row["textures"] == distortion["textures"]
    row["requiresDepthSample"] |= distortion["requiresDepthSample"]
    row["distortionPass"] = dict(sourcePS=DISTORTION_PS, sourceVS=DISTORTION_VS,
        depthAdapter=distortion["depthAdapter"],
        outputAdapter="source positive XY minus negative ZW into existing signed UV distortion target")
    contract_path.write_text(json.dumps(contract, indent=2) + "\n", encoding="utf8")
    print("Recovered 38 source color programs plus the original Disto05 accumulation pass.")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument("--source-dir", type=Path, required=True)
    build(parser.parse_args().source_dir.resolve())
