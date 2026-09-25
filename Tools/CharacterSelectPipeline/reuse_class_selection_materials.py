"""Reuse admitted source-exact S/D materials in staged selection Effects.

OneLayer's existing S/D carrier also owns its opaque SceneColor replacement
blend. A newly generated additive descriptor alone is not that full contract.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
from pathlib import Path

from project_guardian_selection import read, write

ROOT = Path(__file__).resolve().parents[2]
REVIEWED = {
    "fx_mastermaterial.fx_mi.fx_mm_onelayerdistortion_02_01_ad": (
        "5825675b4ffbc840ad691ec56973cf7e", "eb2bcd5c8f3c6c49805ab689687b14f6", "effect.ue3.sd-374-native.v1"),
    "fx_m_mi_j_00.fx_mi.fx_j_pa_hologram_01_01_tr": (
        "c7afe261ee6b2342b612cebb098b1933", "e0b5fe98a973904985c18256b8eb8f8b", "effect.ue3.sd-375-native.v1"),
}


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def semantic(material, resources):
    value = copy.deepcopy(material)
    profile = value["sourceProfile"]
    profile.pop("runtimeShaderProfileId")
    for texture in profile["textures"]:
        asset = texture.pop("assetId")
        paths = [root / asset for root in resources if (root / asset).is_file()]
        if not paths or len({digest(path) for path in paths}) != 1:
            raise ValueError((asset, "Texture missing or differs across candidate/runtime roots"))
        texture["bytesSha256"] = digest(paths[0])
    for name in ("textures", "scalars", "vectors", "staticSwitches"):
        profile[name] = sorted(profile[name], key=lambda row: row["name"])
    return value


def reuse(candidate, source_effect, native_contract, resources, receipt):
    donor_hash = digest(source_effect)
    contract = read(native_contract)
    if contract.get("deferredPrograms"):
        raise ValueError("Source-native preparation must have no unresolved programs")
    donors = {}
    for element in read(source_effect)["elements"]:
        material = element.get("material", {})
        source = material.get("sourceMaterialPath")
        if source not in REVIEWED:
            continue
        if material["sourceProfile"]["runtimeShaderProfileId"] != REVIEWED[source][2]:
            raise ValueError((source, "Unexpected admitted source carrier"))
        if source in donors and material != donors[source]:
            raise ValueError((source, "Ambiguous admitted material"))
        donors[source] = material
    qualified = {}
    for source, (vs, ps, runtime) in REVIEWED.items():
        matches = [p for p in contract["programs"] if p["sourceMaterial"] == source]
        if len(matches) != 1 or (matches[0]["sourceVS"], matches[0]["sourcePS"]) != (vs, ps):
            raise ValueError((source, "Source shader pair differs from admitted carrier"))
        qualified[source] = matches[0]
    changed = []
    for path in sorted(candidate.glob("*.effect.json")):
        document = read(path)
        replacements = []
        for element in document["elements"]:
            current = element.get("material", {})
            source = current.get("sourceMaterialPath")
            if source not in qualified:
                continue
            donor = donors[source]
            if semantic(current, resources) != semantic(donor, resources):
                raise ValueError((path, element["id"], "Source MIC parameters or texture bytes differ"))
            # Preserve current exact texture IDs; only the already admitted
            # runtime descriptor changes after complete semantic equivalence.
            old = current["sourceProfile"]["runtimeShaderProfileId"]
            current["sourceProfile"]["runtimeShaderProfileId"] = REVIEWED[source][2]
            replacements.append(dict(elementId=element["id"], previousRuntimeProfile=old,
                                     runtimeProfile=REVIEWED[source][2], sourceMaterial=source))
        if replacements:
            before = digest(path)
            write(path, document)
            changed.append(dict(path=str(path), before=before, after=digest(path), replacements=replacements))
    if digest(source_effect) != donor_hash:
        raise ValueError("Admitted donor changed during source verification")
    write(receipt, dict(installed=False, donor=str(source_effect), donorSha256=donor_hash,
        sourceNativeContract=str(native_contract), changes=changed,
        oneLayerContract="Existing SDNative374 opacity/clip-Z/neutral scene attenuation and opaque SceneColor replacement blend"))
    return sum(len(row["replacements"]) for row in changed)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--candidate", type=Path, required=True)
    parser.add_argument("--source-effect", type=Path, required=True)
    parser.add_argument("--native-contract", type=Path, required=True)
    parser.add_argument("--resource-root", type=Path, action="append", default=[])
    parser.add_argument("--receipt", type=Path, required=True)
    args = parser.parse_args()
    print("Reused source-exact material occurrences:", reuse(args.candidate, args.source_effect,
        args.native_contract, args.resource_root+[ROOT / "Client/Bin/Resources"], args.receipt))
