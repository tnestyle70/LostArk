"""Audit the retired clip-aggregate Valtan Effect projection.

The join is the stage actionId. ValtanEncounter.json owns the stage list and
its hit shape, Valtan.patternbindings.json maps the stage actionId to a model
clip, and the imported source documents record which original Cascade systems
fired on that clip and what each of their emitters referenced.

One Element is one *emitter*, not one ParticleSystem. A Cascade system holds
many emitters and each emitter owns a single material, so counting textures
per system produced 21- and 42-texture totals that no slot layout can hold.
Per emitter the count is small, which is why the earlier system-level seeder
dropped 269 of 457 candidates and this one drops none: textures beyond the
template's slots are recorded on the Element as unboundResources so the
document still names every asset the original referenced.

This module is retained because the part-break audit and historical tests use
its parsing helpers.  It is no longer a canonical writer.  A clip name does
not select a source action branch, notify occurrence, emitter occurrence or
LOD/module occurrence, and writing its aggregate would overwrite hand tuning.

The canonical source inventory is built by
``build_valtan_source_occurrence_inventory.py``.  This command only reports
what the retired projection *would* have aggregated; it never writes Authored
documents, Product cues, the Effect catalog, or deletes files.
"""

import argparse
import collections
import copy
import json
import os
import re
import sys

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
DATA = os.path.join(REPO, "Data")
RESOURCES = os.path.join(REPO, "Client", "Bin", "Resources")
EFFECT_ROOT = os.path.join(RESOURCES, "Effect", "Valtan")
OUT_DIR = os.path.join(DATA, "Effects", "Authored")
SKELETON = os.path.join(OUT_DIR, "valtan.test.effect.json")
CUE_DOCUMENT = os.path.join(
    DATA, "Animation", "Authored", "Valtan",
    "Valtan.patterneffectcues.json")
EFFECT_CATALOG = os.path.join(DATA, "Effects", "EffectCatalog.json")

# The 420633 canary is referenced by the runtime catalog, the patterneffects
# binding and the Product canary, so the seeder never writes over it.
PROTECTED_ASSET_IDS = {"effect.valtan.pattern.420633.active"}

TEXTURE_SLOTS = ["base", "noise", "mask", "emissive", "dissolve",
                 "base2", "mask2", "noise2"]

# Seeded defaults. The previous skeleton used scale 0.01 and one particle,
# which renders as a dot; these are the smallest values that actually show a
# shape on screen so size and position can be tuned from something visible.
SEED_SCALE = 1.0
SEED_MAX_PARTICLES = 8
SEED_BURST_COUNT = 8
SEED_PARTICLE_LIFE = [0.6, 1.0]

# The original material name carries its blend mode in the last token: _tr is
# translucent, _ad is additive. The four player classes already author from that
# same token - 4,444 of 4,458 _tr rows are alpha and 2,966 of 2,976 _ad rows are
# additive - so Valtan uses the identical rule instead of flattening to alpha.
# One-sided versus two-sided is a Cascade module flag and the Valtan extraction
# holds no modules, so every row stays two-sided rather than guessing.
ALPHA_RENDER_PROFILE = "alpha_two_sided_depth_read"
ADDITIVE_RENDER_PROFILE = "additive_two_sided_depth_read"
ADDITIVE_MATERIAL_SUFFIXES = {"ad"}

# Cascade authored its mesh geometry in UE3 centimetres and this runtime is in
# metres, so a mesh carrier needs the same 0.01 factor the four classes apply.
# It lives on modelPreScale, not on transform.scale, because transform.scale is
# the value a person edits in Effect Detail.
MESH_MODEL_PRE_SCALE = 0.01

# A sprite particle is a flat quad, so the renderer only rebuilds it to face the
# camera when Detail.Particle.bBillboard is set; without it the quad keeps one
# world orientation and vanishes edge-on. A mesh carrier has real geometry and
# must not be billboarded. All 3,232 sprite and 1,284 mesh particle rows across
# the four classes follow that split with no exception.

SIX_DIRECTION_LINE_TEXTURE = (
    "Effect/Valtan/Textures/FX_TEX_00/fx_a_line_010_ycl.dds")
ARENA_WIPE_RING_TEXTURE = (
    "Effect/Valtan/Textures/FX_TEX_04/fx_f_ring_001.dds")

FLOOR_WIPE_STAGE_LABELS = {
    "WINDUP": "발탄 / 115줄 / 6방향 공격 예고",
    "FIRST_SMASH": "발탄 / 115줄 / 6방향 공격 충격",
    "INTERVAL": "발탄 / 115줄 / 전멸 공격 예고",
    "SECOND_SMASH": "발탄 / 115줄 / 전멸 공격 충격",
}

FLOOR_WIPE_BINDING_SUFFIXES = {
    "WINDUP": "six-direction-telegraph",
    "FIRST_SMASH": "six-direction-impact",
    "INTERVAL": "arena-wipe-telegraph",
    "SECOND_SMASH": "arena-wipe-impact",
}


def resolve_render_profile(material_path):
    """Blend mode from the original material name's trailing token.

    An empty path means a project-authored guide row rather than a recovered
    Cascade binding, and those stay alpha because nothing in the source names
    them.
    """
    if not material_path:
        return ALPHA_RENDER_PROFILE
    suffix = material_path.rsplit("_", 1)[-1].lower()
    if suffix in ADDITIVE_MATERIAL_SUFFIXES:
        return ADDITIVE_RENDER_PROFILE
    return ALPHA_RENDER_PROFILE


def element_carries_mesh(element):
    return any(binding.get("slotId") == "meshModel"
               for binding in element.get("resources") or [])


def read_json(path):
    with open(path, encoding="utf-8-sig") as handle:
        return json.load(handle)


def write_json_atomic(path, value):
    temporary = path + ".staging"
    with open(temporary, "w", encoding="utf-8", newline="\n") as handle:
        json.dump(value, handle, ensure_ascii=False, indent=2)
        handle.write("\n")
    os.replace(temporary, path)


def normalize_clip(name):
    lowered = name.lower()
    return lowered[5:] if lowered.startswith("mesh_") else lowered


def legacy_binding_clips(binding):
    """Flatten v1/v2 binding shapes for the report-only legacy audit."""
    raw = binding.get("clips")
    if raw is None:
        raw = binding.get("clip")
    if isinstance(raw, (str, dict)):
        raw = [raw]
    if not isinstance(raw, list):
        return []
    result = []
    for value in raw:
        clip = value if isinstance(value, str) else value.get("clip", "") \
            if isinstance(value, dict) else ""
        if clip:
            result.append(clip)
    return result


def index_resources():
    """Map a bare asset file name to its Resources-relative id."""
    index = {}
    for root, _dirs, files in os.walk(EFFECT_ROOT):
        for name in files:
            if not name.lower().endswith((".dds", ".wmodel")):
                continue
            relative = os.path.relpath(os.path.join(root, name), RESOURCES)
            index.setdefault(name.lower(), relative.replace("\\", "/"))
    return index


def resolve_slot(name):
    """Which slot a texture prefers, by the kind token in its file name."""
    lowered = name.lower()
    if "normal" in lowered or "_n." in lowered or "_n_" in lowered:
        return "noise"
    if "noise" in lowered or "flow" in lowered or "turbulence" in lowered:
        return "noise"
    if "mask" in lowered or "alpha" in lowered or "opacity" in lowered:
        return "mask"
    if "emis" in lowered or "glow" in lowered or "shine" in lowered:
        return "emissive"
    if "dissolve" in lowered:
        return "dissolve"
    if "decal" in lowered:
        return "base"
    if any(token in lowered for token in ("trail", "atypical", "aura", "line")):
        return "base"
    return None


def assign_slots(textures):
    """Place textures into slots; return (assigned, leftover).

    Preferred slot first, then the second layer of that same slot, then the
    remaining slots in declaration order. Whatever still does not fit is
    returned as leftover instead of being discarded.
    """
    second_layer = {"base": "base2", "mask": "mask2", "noise": "noise2"}
    assigned = {}
    deferred = []
    for name in textures:
        slot = resolve_slot(name)
        if slot is None:
            deferred.append(name)
            continue
        if slot not in assigned:
            assigned[slot] = name
            continue
        pair = second_layer.get(slot)
        if pair is not None and pair not in assigned:
            assigned[pair] = name
            continue
        deferred.append(name)
    leftover = []
    for name in deferred:
        for slot in TEXTURE_SLOTS:
            if slot not in assigned:
                assigned[slot] = name
                break
        else:
            leftover.append(name)
    # An Element with no base draws the shader's white fallback, which reads as
    # a white quad on screen. If the kind tokens sent every texture to another
    # slot, the first one by slot order becomes the base instead.
    if "base" not in assigned and assigned:
        for slot in TEXTURE_SLOTS:
            if slot in assigned:
                assigned["base"] = assigned.pop(slot)
                break
    return assigned, leftover


def stage_slug(pattern_action, stage):
    action = stage["actionId"]
    prefix = pattern_action + "."
    if action.startswith(prefix) and len(action) > len(prefix):
        return action[len(prefix):]
    return stage["stageId"].lower().replace("_", "-")


def pattern_slug(pattern_action):
    parts = pattern_action.split(".")
    # valtan.attack.whirlwind -> whirlwind ; the category is shown by the tree
    return ".".join(parts[2:]) if len(parts) > 2 else parts[-1]


def emitter_rows(system, materials, resources):
    """One row per material binding node, in the graph's own order.

    Returns (node_id, ordinal, textures, mesh_asset_id) tuples. The textures
    are Resources-relative ids that physically exist; the mesh is the WModel
    the same graph node referenced, or the system's single mesh when the
    emitter itself declared none.
    """
    graph = system.get("graph") or {}
    bindings = graph.get("resourceBindings") or []

    meshes_by_node = collections.defaultdict(list)
    system_meshes = []
    for binding in bindings:
        if binding.get("role") != "mesh":
            continue
        leaf = binding["objectPath"].rsplit(".", 1)[-1]
        asset = resources.get(leaf.lower() + ".wmodel")
        if asset is None:
            continue
        meshes_by_node[binding["sourceNodeId"]].append(asset)
        if asset not in system_meshes:
            system_meshes.append(asset)

    rows = []
    ordinal = 0
    for binding in bindings:
        if binding.get("role") != "material":
            continue
        material = materials.get(binding["objectPath"])
        names = []
        if material is not None:
            for entry in (material.get("instanceTextures") or []):
                value = entry.get("texture")
                if value and value not in names:
                    names.append(value)
            parent = material.get("parentDeclaration") or {}
            for entry in (parent.get("collectedTextureParameters") or []):
                value = entry.get("texture")
                if value and value not in names:
                    names.append(value)
        textures = []
        for name in names:
            asset = resources.get(name.rsplit(".", 1)[-1].lower() + ".dds")
            if asset is not None and asset not in textures:
                textures.append(asset)
        mesh = meshes_by_node.get(binding["sourceNodeId"]) or system_meshes
        rows.append((binding["sourceNodeId"], ordinal, textures,
                     mesh[0] if mesh else None, binding["objectPath"]))
        ordinal += 1
    return rows


def build_element(template, element_id, display_name, group_id,
                  textures, mesh, material_path, duration_ms):
    element = copy.deepcopy(template)
    element["id"] = element_id
    element["displayName"] = display_name
    element["groupId"] = group_id
    element["kind"] = "particle"
    element["visible"] = True

    slots = []
    if mesh is not None:
        slots.append({"slotId": "meshModel", "assetId": mesh})
    assigned, leftover = assign_slots(textures)
    for slot in TEXTURE_SLOTS:
        if slot in assigned:
            slots.append({"slotId": slot, "assetId": assigned[slot]})
    element["resources"] = slots
    if leftover:
        element["unboundResources"] = leftover

    material = element.setdefault("material", {})
    material["sourceMaterialPath"] = material_path
    material["renderProfile"] = resolve_render_profile(material_path)

    detail = element["detail"]
    detail["mesh"]["useModelMaterial"] = (
        mesh is not None and "base" not in assigned)
    if mesh is not None:
        detail["mesh"]["modelPreScale"] = MESH_MODEL_PRE_SCALE
    else:
        detail["mesh"].pop("modelPreScale", None)
    detail["particle"]["billboard"] = mesh is None
    detail["transform"]["scale"] = [SEED_SCALE, SEED_SCALE, SEED_SCALE]
    particle = detail["particle"]
    particle["maxParticles"] = SEED_MAX_PARTICLES
    particle["burstCount"] = SEED_BURST_COUNT
    particle["lifeTimeSeconds"] = list(SEED_PARTICLE_LIFE)
    # The encounter document states how long the stage runs, so the Element
    # lifetime is read from it rather than guessed.
    detail["timing"]["lifeTimeSeconds"] = round(max(duration_ms, 1) / 1000.0, 3)
    return element


def build_floor_lane_guides(template, stage_id, duration_ms):
    """Three centered decal axes are six radial lanes.

    These rows are project-authored tuning guides, not recovered Cascade
    evidence. Their dimensions come directly from the Server hit contract:
    half-length 14 and half-width 2.2 become a 28 x 4.4 strip.
    """
    if stage_id not in ("WINDUP", "FIRST_SMASH"):
        return []
    semantic = ("telegraph" if stage_id == "WINDUP" else "impact")
    guides = []
    for angle in (0, 60, 120):
        element = copy.deepcopy(template)
        element["id"] = "six-direction-{}.axis-{:03d}".format(
            semantic, angle)
        element["displayName"] = "6방향 {} / {}° 축".format(
            "예고" if semantic == "telegraph" else "충격", angle)
        element["groupId"] = "six_direction_{}".format(semantic)
        element["sourceNode"] = (
            "project-authored:valtan.floor-wipe-130.{}.axis-{:03d}".format(
                semantic, angle))
        element["visible"] = True
        element["kind"] = "decal"
        element["resources"] = [
            {"slotId": "base", "assetId": SIX_DIRECTION_LINE_TEXTURE}]
        element["material"]["templateId"] = "effect.standard"
        element["material"]["sourceMaterialPath"] = ""
        element["material"]["sourceProfile"] = {"enabled": False}
        detail = element["detail"]
        detail["transform"]["position"] = [0, 0.08, 0]
        detail["transform"]["rotationDegrees"] = [0, angle, 0]
        detail["transform"]["scale"] = [1, 1, 1]
        detail["color"]["multiply"] = (
            [1.0, 0.12, 0.02, 0.58] if semantic == "telegraph" else
            [1.0, 0.32, 0.04, 0.9])
        detail["color"]["emissiveIntensity"] = (
            1.5 if semantic == "telegraph" else 3.0)
        detail["decal"]["size"] = [4.4, 28.0]
        detail["decal"]["depth"] = 1.0
        detail["timing"]["startDelaySeconds"] = 0
        detail["timing"]["lifeTimeSeconds"] = round(
            max(duration_ms, 1) / 1000.0, 3)
        element["sourceRecipe"] = {
            "enabled": False,
            "rendererShape": "",
            "emitterDelaySeconds": 0,
            "emitterDurationSeconds": 0,
            "emitterLoopCount": 1,
            "bursts": [],
            "modules": [],
        }
        element["sourcePresentation"] = {"enabled": False}
        guides.append(element)
    return guides


def build_arena_wipe_guide(template, stage_id, duration_ms):
    if stage_id not in ("INTERVAL", "SECOND_SMASH"):
        return []
    semantic = "telegraph" if stage_id == "INTERVAL" else "impact"
    element = copy.deepcopy(template)
    element["id"] = "arena-wipe-{}.radius-100".format(semantic)
    element["displayName"] = "전멸 공격 {} / 반경 100".format(
        "예고" if semantic == "telegraph" else "충격")
    element["groupId"] = "arena_wipe_{}".format(semantic)
    element["sourceNode"] = (
        "project-authored:valtan.floor-wipe-130.{}.radius-100".format(
            semantic))
    element["visible"] = True
    element["kind"] = "decal"
    element["resources"] = [
        {"slotId": "base", "assetId": ARENA_WIPE_RING_TEXTURE}]
    element["material"]["templateId"] = "effect.standard"
    element["material"]["sourceMaterialPath"] = ""
    element["material"]["sourceProfile"] = {"enabled": False}
    detail = element["detail"]
    detail["transform"]["position"] = [0, 0.06, 0]
    detail["transform"]["rotationDegrees"] = [0, 0, 0]
    detail["transform"]["scale"] = [1, 1, 1]
    detail["color"]["multiply"] = (
        [0.9, 0.04, 0.02, 0.46] if semantic == "telegraph" else
        [1.0, 0.4, 0.08, 0.92])
    detail["color"]["emissiveIntensity"] = (
        1.25 if semantic == "telegraph" else 3.5)
    detail["decal"]["size"] = [200.0, 200.0]
    detail["decal"]["depth"] = 2.0
    detail["timing"]["startDelaySeconds"] = 0
    detail["timing"]["lifeTimeSeconds"] = round(
        max(duration_ms, 1) / 1000.0, 3)
    element["sourceRecipe"] = {
        "enabled": False,
        "rendererShape": "",
        "emitterDelaySeconds": 0,
        "emitterDurationSeconds": 0,
        "emitterLoopCount": 1,
        "bursts": [],
        "modules": [],
    }
    element["sourcePresentation"] = {"enabled": False}
    return [element]


def cue_binding_id(pattern_action, stage):
    if pattern_action == "valtan.mechanic.floor-wipe-130":
        suffix = FLOOR_WIPE_BINDING_SUFFIXES.get(stage["stageId"])
        if suffix:
            return "cue.valtan.floor-wipe-130." + suffix
    return "cue.valtan.{}.{}".format(
        pattern_slug(pattern_action), stage_slug(pattern_action, stage))


def build_cue_document(documents):
    cues = []
    for asset_id, pattern_id, stage_id, action_id, _clip, duration_ms, _doc in documents:
        pattern_action = action_id.rsplit(".", 1)[0]
        stage = {"stageId": stage_id, "actionId": action_id}
        cues.append({
            "bindingId": cue_binding_id(pattern_action, stage),
            "patternId": pattern_id,
            "stageId": stage_id,
            "actionId": action_id,
            "effectAssetId": asset_id,
            "anchorSlotId": "root",
            "followPolicy": "follow",
            "stopPolicy": "cue_end",
            "startMs": 0,
            "endMs": duration_ms,
            "localTransform": {
                "position": [0, 0, 0],
                "rotationDegrees": [0, 0, 0],
                "scale": [1, 1, 1],
            },
        })
    cues.sort(key=lambda cue: (
        cue["patternId"], cue["actionId"], cue["bindingId"]))
    return {
        "schema": "lostark.valtan-pattern-effect-cues",
        "formatVersion": 1,
        "ownerArchetypeId": "BOSS_VALTAN",
        "cues": cues,
    }


def sync_effect_catalog(catalog_document, documents):
    generated_ids = {row[0] for row in documents}
    preserved = []
    for entry in catalog_document["effects"]:
        asset_id = entry["effectAssetId"]
        if asset_id.startswith("effect.valtan.") and \
                asset_id not in PROTECTED_ASSET_IDS:
            continue
        preserved.append(entry)
    for asset_id in sorted(generated_ids):
        preserved.append({
            "effectAssetId": asset_id,
            "payloadKind": "DIRECT_AUTHORED_DOCUMENT_V13",
            "authoringPath": "Effects/Authored/{}.effect.json".format(
                asset_id),
        })
    preserved.sort(key=lambda entry: entry["effectAssetId"])
    staged = copy.deepcopy(catalog_document)
    staged["effects"] = preserved
    return staged


def migrate_existing(write):
    """Bring already-authored documents onto the render contract in place.

    A full reseed would discard whatever was tuned by hand in the Effect Tool,
    so this reads each existing document and replaces exactly two fields that
    the earlier seeder never wrote. Every other value, including a tuned
    transform or colour, is left byte-identical, and running it twice produces
    the same result as running it once.
    """
    documents = 0
    additive = 0
    alpha = 0
    meshes = 0
    billboards = 0
    changed_files = 0
    for name in sorted(os.listdir(OUT_DIR)):
        if not name.startswith("effect.valtan.") or \
                not name.endswith(".effect.json"):
            continue
        asset_id = name[:-len(".effect.json")]
        if asset_id in PROTECTED_ASSET_IDS:
            continue
        path = os.path.join(OUT_DIR, name)
        document = read_json(path)
        documents += 1
        touched = False
        for element in document.get("elements") or []:
            material = element.setdefault("material", {})
            profile = resolve_render_profile(
                material.get("sourceMaterialPath") or "")
            if material.get("renderProfile") != profile:
                material["renderProfile"] = profile
                touched = True
            if profile == ADDITIVE_RENDER_PROFILE:
                additive += 1
            else:
                alpha += 1

            detail = element.setdefault("detail", {})
            mesh = detail.setdefault("mesh", {})
            carries_mesh = element_carries_mesh(element)
            if carries_mesh:
                meshes += 1
                if mesh.get("modelPreScale") != MESH_MODEL_PRE_SCALE:
                    mesh["modelPreScale"] = MESH_MODEL_PRE_SCALE
                    touched = True
            elif "modelPreScale" in mesh:
                del mesh["modelPreScale"]
                touched = True

            if element.get("kind") == "particle":
                particle = detail.setdefault("particle", {})
                if particle.get("billboard") is not (not carries_mesh):
                    particle["billboard"] = not carries_mesh
                    touched = True
                    if not carries_mesh:
                        billboards += 1
        if touched:
            changed_files += 1
            if write:
                write_json_atomic(path, document)

    print(f"documents visited      {documents}")
    print(f"elements additive      {additive}")
    print(f"elements alpha         {alpha}")
    print(f"mesh carriers rescaled {meshes} at {MESH_MODEL_PRE_SCALE}")
    print(f"sprites billboarded    {billboards}")
    print(f"documents needing edit {changed_files}")
    if not write:
        print("\n(dry run; pass --write to emit)")
    else:
        print(f"\nrewrote {changed_files} documents in {OUT_DIR}")
    return 0


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--audit-legacy",
        action="store_true",
        help="explicitly select the report-only retired clip aggregate",
    )
    parser.parse_args()

    encounter = read_json(os.path.join(
        DATA, "Encounters", "Valtan", "ValtanEncounter.json"))
    bindings = read_json(os.path.join(
        DATA, "Animation", "Authored", "Valtan", "Valtan.patternbindings.json"))
    catalog = read_json(os.path.join(
        DATA, "Effects", "Imported", "Valtan",
        "Valtan.effect-resource-catalog.json"))
    evidence = read_json(os.path.join(
        DATA, "Effects", "Imported", "Valtan",
        "Valtan.source-material-evidence.json"))
    skeleton = read_json(SKELETON)
    element_template = skeleton["elements"][0]

    materials = {row["materialId"]: row for row in evidence["materials"]}
    clips_by_action = {
        b["actionId"]: legacy_binding_clips(b) for b in bindings["bindings"]
    }

    systems_by_clip = collections.defaultdict(list)
    for system in catalog["sourceSystems"]:
        for clip in (system.get("clipNames") or []):
            systems_by_clip[normalize_clip(clip)].append(system)

    resources = index_resources()

    documents = []
    stages_total = stages_without_clip = stages_without_element = 0
    unbound_total = mesh_elements = 0
    slot_use = collections.Counter()
    for pattern in encounter["patterns"]:
        pattern_action = pattern["actionId"]
        for stage in pattern["stages"]:
            stages_total += 1
            clips = clips_by_action.get(stage["actionId"], [])
            if not clips:
                stages_without_clip += 1
                continue
            elements = []
            seen = set()
            stage_systems = []
            for clip in clips:
                stage_systems.extend(
                    systems_by_clip.get(normalize_clip(clip), [])
                )
            for system in stage_systems:
                for node, ordinal, textures, mesh, path in emitter_rows(
                        system, materials, resources):
                    if not textures and mesh is None:
                        continue
                    key = (system["logicalPackage"], system["objectName"], node)
                    if key in seen:
                        continue
                    seen.add(key)
                    element_id = "{}.em{:02d}".format(
                        system["objectName"], ordinal)
                    if any(e["id"] == element_id for e in elements):
                        element_id = "{}.{}.em{:02d}".format(
                            system["logicalPackage"].lower(),
                            system["objectName"], ordinal)
                    element = build_element(
                        element_template, element_id,
                        "{} / emitter {:02d}".format(
                            system["objectName"], ordinal),
                        stage["stageId"].lower(), textures, mesh, path,
                        stage.get("durationMs", 0))
                    for binding in element["resources"]:
                        slot_use[binding["slotId"]] += 1
                    unbound_total += len(element.get("unboundResources", []))
                    if mesh is not None:
                        mesh_elements += 1
                    elements.append(element)

            if not elements:
                stages_without_element += 1
                continue
            asset_id = "effect.valtan.{}.{}".format(
                pattern_slug(pattern_action), stage_slug(pattern_action, stage))
            if asset_id in PROTECTED_ASSET_IDS:
                continue
            if pattern["patternId"] == "VALTAN_FLOOR_WIPE_130":
                elements.extend(build_floor_lane_guides(
                    element_template, stage["stageId"],
                    stage.get("durationMs", 0)))
                elements.extend(build_arena_wipe_guide(
                    element_template, stage["stageId"],
                    stage.get("durationMs", 0)))
            document = copy.deepcopy(skeleton)
            document["effectAssetId"] = asset_id
            document["displayName"] = FLOOR_WIPE_STAGE_LABELS.get(
                stage["stageId"], "{} / {}".format(
                    pattern["patternId"], stage["stageId"])) \
                if pattern["patternId"] == "VALTAN_FLOOR_WIPE_130" else \
                "{} / {}".format(pattern["patternId"], stage["stageId"])
            document["elements"] = elements
            documents.append((asset_id, pattern["patternId"],
                              stage["stageId"], stage["actionId"],
                              ",".join(clips),
                              stage.get("durationMs", 0), document))

    element_count = sum(len(d[6]["elements"]) for d in documents)
    print(f"stages                 {stages_total}")
    print(f"stages without a clip  {stages_without_clip}")
    print(f"stages without source  {stages_without_element}")
    print(f"documents              {len(documents)}")
    print(f"elements               {element_count}")
    print(f"  carrying a mesh      {mesh_elements}")
    print(f"unbound source refs    {unbound_total}")
    print("slot use               " + ", ".join(
        f"{slot}={slot_use[slot]}" for slot in
        ["meshModel"] + TEXTURE_SLOTS if slot_use[slot]))

    for asset_id, pat, st, _action, clip, _duration, doc in documents[:10]:
        print(f"  {asset_id:52} {pat:30} {st:16} "
              f"clips={clip} elements={len(doc['elements'])}")
    print("\nlegacy audit only; canonical writer is disabled")
    return 0


if __name__ == "__main__":
    sys.exit(main())
