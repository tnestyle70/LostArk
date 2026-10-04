"""Recover the six excluded Alt V debris occurrences into an out-only candidate.

Read the installed source packages and class defaults again. Reuse the already
restored, identical Class Select material and the reviewed source-sign geometry
candidate; do not generate shaders or replace any current authored element.
"""
from pathlib import Path
import argparse
import copy
import hashlib
import json
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "Tools/LevelPlacementExtractor"))
import build_imported_effect_documents as imported
import build_kouku_gate1_full_restore as source
from build_warlord_asvf_full_restore import norm, merge
from build_warlord_altv_radial_candidate import ASSET, SYSTEM, SLOT, source_rotators, require, encoded, sha

EMITTER = SYSTEM + ".particlespriteemitter_16"
MIC = "fx_m_mi_05.fx_mi.fx_e_me_ht_03_4_ma"
DONOR_ASSET = "effect.classselect.warlord.fx_pc_gst_00.par_k_gst_airstrike_exp_01"
PROFILE = "effect.ue3.kouku-5111-native.v1"
MESH_ASSET = "Effect/Warlord/FullRestore/Meshes/fm_a_stone_001.wmodel"


def acquire(game_root, umodel, default_locations):
    packages, records = {}, {}
    script_paths = {}
    for row in default_locations["records"]:
        logical = row["fullPath"].split(".")[0]
        script_paths[logical] = game_root / Path(row["sourcePackage"]).name

    def fetch(key):
        key = key.lower()
        if key in records:
            return records[key]
        logical, tail = key.split(".", 1)
        if logical not in packages:
            path = script_paths.get(logical)
            if path is None:
                path = source.ue3.resolve_physical_package(umodel, game_root / "Packages", logical, "kr")
            package = source.load_package(path, source.ue3.LOSTARK_KR_AES_KEY)
            exports = {source.ue3.package_ref_path(e.index + 1, package.imports, package.exports).lower(): e
                       for e in package.exports}
            packages[logical] = (package, exports)
        package, exports = packages[logical]
        record = source.record_from_export(package, logical, exports[tail])
        records[key] = record
        return record

    def parent_of(row):
        parent = row.get("archetypeFullPath")
        if parent is None and not row["fullPath"].split(".", 1)[1].startswith("default__"):
            package, name = row["classPath"].split(".", 1)
            parent = package + ".default__" + name
        return parent

    def close(key, stack=()):
        require(key not in stack, "Source archetype/reference cycle")
        row = fetch(key)
        parent = parent_of(row)
        if parent:
            close(parent, stack + (key,))
        for ref in row["references"]:
            if ref["property"].endswith("distribution"):
                close(ref["objectPath"], stack + (key,))

    emitter = fetch(EMITTER)
    lod_key = next(r["objectPath"] for r in emitter["references"] if r["property"] == "lodlevels")
    lod = fetch(lod_key)
    module_keys = [r["objectPath"] for r in lod["references"]
                   if r["property"] in ("requiredmodule", "modules", "typedatamodule", "spawnmodule")]
    require(len(module_keys) == 14, "Reviewed source module count changed")
    for key in [EMITTER, lod_key] + module_keys:
        close(key)
    effective = {}

    def resolve(key):
        if key not in effective:
            row = records[key]
            parent = parent_of(row)
            effective[key] = merge(resolve(parent) if parent else {}, norm(row["properties"]))
        return effective[key]

    index = imported.SourceIndex({}, {})
    for key, row in records.items():
        obj = imported.SourceObject(key, key, row["className"], key, resolve(key))
        for ref in row["references"]:
            pair = (ref["property"].lower(), ref["objectPath"].lower())
            obj.reference_paths.append(pair)
            if pair[1] in records:
                obj.references.append(pair)
        index.objects[key] = obj
        index.by_source_id[key] = obj
    objects = [index.objects[key] for key in module_keys]
    require(imported.prop(index.objects[lod_key].properties, "benabled", True), "Source LOD disabled")
    modules = []
    for obj in objects:
        literals, distributions = imported.flatten_source_properties(index, obj, include_source_contract_bindings=False)
        modules.append(dict(stableId="source.module." + sha(obj.key.encode())[:24],
                            className=obj.class_name, objectPath=obj.key,
                            literals=literals, distributions=distributions))
    required = next(obj for obj in objects if obj.class_name == "particlemodulerequired")
    typed = next(obj for obj in objects if obj.class_name == "particlemoduletypedatamesh")
    require(("material", MIC) in required.reference_paths, "Source MIC changed")
    require(("mesh", "fx_sm_00.fm_a_stone_001") in typed.reference_paths, "Source mesh changed")
    spawn = next(obj for obj in objects if obj.class_name == "particlemodulespawn")
    bursts = []
    for row in imported.prop(spawn.properties, "burstlist", []):
        count, low = imported.prop(row, "count"), imported.prop(row, "countlow", -1)
        if count > 0:
            bursts.append(dict(timeSeconds=imported.prop(row, "time"),
                               countMinimum=count if low < 0 else low, countMaximum=count))
    recipe = dict(enabled=True, rendererShape="mesh", modules=modules, bursts=bursts)
    for field, prop in (("emitterDurationSeconds", "emitterduration"),
                        ("emitterDelaySeconds", "emitterdelay"), ("emitterLoopCount", "emitterloops")):
        recipe[field] = imported.prop(required.properties, prop)
    require(recipe["bursts"] == [dict(timeSeconds=0.0, countMinimum=8, countMaximum=8)],
            "Source spawn count changed")
    package_receipt = {key: dict(path=str(p.path), sha256=sha(p.path.read_bytes()))
                       for key, (p, _) in packages.items()}
    return recipe, index, lod_key, objects, records, package_receipt


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--action", required=True, type=Path)
    parser.add_argument("--umodel", required=True, type=Path)
    parser.add_argument("--source-defaults", required=True, type=Path)
    parser.add_argument("--game-root", type=Path, default=Path("C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC"))
    parser.add_argument("--input", type=Path, default=ROOT / "Data/Effects/Authored" / (ASSET + ".effect.json"))
    parser.add_argument("--out", type=Path, default=ROOT / "out/WarlordAltVRockDirections20261004/Emitter16FinalCandidate")
    parser.add_argument("--mesh-candidate", type=Path,
                        help="Reviewed source-sign geometry candidate; omit after its Resources installation")
    parser.add_argument("--mesh-evidence", required=True, type=Path,
                        help="Source packed tangent-sign join and geometry cook receipt")
    args = parser.parse_args()
    output = args.out.resolve()
    require(output.is_relative_to((ROOT / "out").resolve()), "Candidate output must remain under out")
    before = args.input.read_bytes()
    document = json.loads(before.decode("utf-8-sig"))
    require(document["effectAssetId"] == ASSET, "Wrong destination asset")
    rotators, action_sha = source_rotators(args.action)
    default_bytes = args.source_defaults.read_bytes()
    recipe, index, lod_key, objects, records, packages = acquire(
        args.game_root, args.umodel, json.loads(default_bytes.decode("utf-8-sig")))
    unified_path = ROOT / "Data/Effects/Authored/effect.warlord.skill.17250.clip2.unified.effect.json"
    unified_bytes = unified_path.read_bytes()
    templates = [e for e in json.loads(unified_bytes.decode("utf-8-sig"))["elements"]
                 if "|element:" + EMITTER in e.get("sourceNode", "").lower()]
    require(len(templates) == 6, "Expected six source occurrence templates")
    donor_path = ROOT / "Data/Effects/Authored" / (DONOR_ASSET + ".effect.json")
    donor_bytes = donor_path.read_bytes()
    donor = next(e for e in json.loads(donor_bytes.decode("utf-8-sig"))["elements"]
                 if e["id"] == DONOR_ASSET + ".particlespriteemitter_3")
    require(donor["material"]["sourceMaterialPath"] == MIC and
            donor["material"]["sourceProfile"]["runtimeShaderProfileId"] == PROFILE,
            "Reviewed native material donor changed")
    require(donor["resources"] == [dict(slotId="meshModel", assetId="Effect/ClassSelect/Warlord/Meshes/fm_a_stone_001.wmodel")],
            "Reviewed native mesh donor changed")
    candidate = copy.deepcopy(document)
    existing = {e["id"]: e for e in document["elements"]}
    inserted = []
    for ordinal, (template, (cue_id, cue)) in enumerate(zip(templates, rotators.items())):
        require(template["sourcePresentation"]["sourceEventId"] == f"source-event-{40 + ordinal:03}",
                "Template occurrence order changed")
        origin = EMITTER + (f".event_source-event-{40 + ordinal:03}" if ordinal else "")
        element = copy.deepcopy(template)
        element_id = "authored.source-particle.full-warlord-alt_v." + sha(origin.encode())[:20]
        element.update(id=element_id, groupId="authored.source-particle.full-warlord-alt_v", visible=True,
                       sourceNode="authored-source-particle:effect.warlord.skill.17250.full.restore|source:effect.warlord.skill.17250.imported|element:" + origin)
        element["material"] = copy.deepcopy(donor["material"])
        element["resources"] = [dict(slotId="meshModel", assetId=MESH_ASSET)]
        detail, _, _ = imported.emitter_detail(index, index.objects[lod_key], objects,
            cue["sourceTimeSeconds"], 0.0, template["detail"]["particle"]["randomSeed"])
        detail["particle"].update(localSpace=False, authoringApproximate=False,
            sourceScale={key: 1 for key in ("count", "size", "lifeTime", "speed", "rotation", "alpha", "spawnDelay")})
        detail["transform"]["rotationDegrees"] = cue["rotationDegrees"]
        detail["mesh"].update(useModelMaterial=False, modelPreScale=0.01)
        element["detail"] = detail
        element["sourceRecipe"] = copy.deepcopy(recipe)
        element["sourcePresentation"].update(sourceActionCueId=cue_id,
            sourceTimeSeconds=cue["sourceTimeSeconds"], sourceOccurrenceIndex=ordinal)
        element["actionCueAttachment"] = dict(enabled=True, follow=True,
            sourceAnchorSlotId="b_effectroot", runtimeAnchorSlotId=SLOT, runtimeBoneName="b_effectroot",
            socketLocalTransform=dict(position=[0., 0., 0.], rotationDegrees=[90, 180, 0], scale=[1., 1., 1.]))
        element["transformInheritance"] = dict(enabled=False, masterElementId="")
        if element_id in existing:
            require(existing[element_id] == element, "Existing restored debris was edited; preserve it")
        else:
            candidate["elements"].append(element)
            inserted.append(dict(elementId=element_id, sourceActionCueId=cue_id,
                                 sourceElement=origin, sha256=sha(encoded(element)), element=element))
    require(not inserted or len(inserted) == 6, "Partial debris occurrence set")
    require(candidate["elements"][:len(document["elements"])] == document["elements"], "Existing elements changed")
    resources = {MESH_ASSET}
    resources.update(r["assetId"] for r in donor["material"]["sourceProfile"]["textures"])
    resource_receipt = []
    for asset in sorted(resources):
        installed = ROOT / "Client/Bin/Resources" / asset
        path = args.mesh_candidate if asset == MESH_ASSET and args.mesh_candidate else installed
        data = path.read_bytes()
        row = dict(assetId=asset, sha256=sha(data))
        if path != installed:
            row.update(candidatePath=str(path.resolve()),
                       installedBeforeSHA256=sha(installed.read_bytes()) if installed.exists() else None)
        resource_receipt.append(row)
    mesh_evidence = args.mesh_evidence.read_bytes()
    geometry = json.loads(mesh_evidence.decode("utf-8-sig"))
    mesh_row = next(row for row in resource_receipt if row["assetId"] == MESH_ASSET)
    require(geometry["resourcePath"] == MESH_ASSET and
            geometry["candidateSha256"] == mesh_row["sha256"] and
            geometry["officialStaticGeometryParser"] == "PASS" and
            geometry["nativeSignJoinCounts"] == 32 and
            geometry["onlyTangentWAndMetadataChanged"] is True and
            geometry["positionsNormalsTangentsUvsColorsIndicesBoundsAndMaterialBytesPreserved"] is True,
            "Geometry does not match the reviewed source-sign candidate")
    require(sha(Path(geometry["sourceSignProof"]).read_bytes()) == geometry["sourceSignProofSha256"],
            "Source-sign join proof changed")
    require(args.input.read_bytes() == before and donor_path.read_bytes() == donor_bytes
            and unified_path.read_bytes() == unified_bytes, "Input changed while preparing candidate")
    output.mkdir(parents=True, exist_ok=True)
    data = encoded(candidate)
    candidate_path = output / args.input.name
    (output / (args.input.name + ".before")).write_bytes(before)
    candidate_path.write_bytes(data)
    receipt = dict(schemaVersion=1, sourcePath=str(args.input.resolve()), beforeSHA256=sha(before),
        candidatePath=str(candidate_path), candidateSHA256=sha(data),
        sourceActionPath=str(args.action.resolve()), sourceActionSHA256=action_sha,
        donorPath=str(donor_path), donorSHA256=sha(donor_bytes),
        templatePath=str(unified_path), templateSHA256=sha(unified_bytes),
        oldElementCount=len(document["elements"]), elementCount=len(candidate["elements"]),
        insertedElementCount=len(inserted), inserted=inserted, resources=resource_receipt,
        originalEmitterCount=11, originalOccurrenceCount=66,
        nativeProfile=PROFILE, sourcePackages=packages,
        meshEvidencePath=str(args.mesh_evidence.resolve()), meshEvidenceSHA256=sha(mesh_evidence),
        validation=dict(existingElementsAndDocumentFieldsPreserved=True,
                        productInstallationPerformed=False, actualPlaybackAndUserScreenValidation="Separate receipts required"))
    for name, value in (("source-records.json", records), ("source-recipe.json", recipe), ("candidate-manifest.json", receipt)):
        (output / name).write_bytes(encoded(value))
    print(json.dumps({key: receipt[key] for key in ("candidatePath", "candidateSHA256", "insertedElementCount", "elementCount")}))


if __name__ == "__main__":
    main()
