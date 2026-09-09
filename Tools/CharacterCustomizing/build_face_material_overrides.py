"""Resolve a retail face MaterialInstanceConstant into a CharacterCatalog material override.

The character-creation screen's skin and make-up controls are not a tint this project invents:
they are named parameters of the retail head material, and the native program that consumes them
is already compiled into ``Shader_SourceCharacterPrograms.hlsli`` (program 4, reached through the
``source.character.classic-head.v1`` family in ``SourceCharacterMaterialParameters.h``).  What was
missing is the per-class row that puts a class' face on that program with the retail values.

Measured, all four playable faces sit on the same master chain, so one family covers them:

    LanceMaster      pc_ft_face_mi        -> ft_head_opa_high -> pbr_base_opa
    Warlord          pc_wr_face_mi_high   -> wr_head_opa_high -> pbr_base_opa
    Artist           pc_sp_face_mi_high   -> sp_head_opa_high -> pbr_base_opa
    DimensionMaster  pc_sp_m_face_05_mi   -> sp_head_opa_high -> pbr_base_opa

Warlord is the one that needs saying out loud.  Its body mesh names ``pc_wr_face_mi``, whose chain
is the old ``pc_head_opa`` -- measured, that master declares only three texture parameters
(``var_headbase_meshtype_diffuse``/``_specular`` and ``var_headdeco_decaltexture_ui``) and no lip,
eye or cheek parameter at all, so no make-up control can reach it.  ``pc_wr_face_mi_high`` is the
same face -- it names the same ``pc_wr_00_face_d``/``_n``/``_s`` textures -- on the high master the
other three classes already use.

## Where a value comes from

A MaterialInstanceConstant states only its overrides; everything else comes from its parent, and
the defaults sit on the ``Material3`` at the end of the chain in its ``Collected*Parameters``.  So
each name is resolved child-first along the chain and the first statement of it wins.  A name the
chain never states is an error here rather than a zero, because ``Configure`` would silently treat
a missing parameter as an invalid row.

## Texture registers

The program addresses textures by register, not by name.  The mapping below is the one
DimensionMaster's existing row already uses, and it is confirmed independently by reading program
4: the register sampled next to ``var_makeup_lipcolor_ui`` is 4, next to the eyeliner and
eyeshadow colours is 5, next to ``var_makeup_cheekcolor_ui`` is 6, and next to
``var_headdeco_decalcolor_ui`` is 7 -- the same order as the four make-up texture variables in
``Data/UI/Customizing/CustomizingMeshTypes.json``.

usage:
  python build_face_material_overrides.py --export-root <umodel export dir> [--class-id Warlord]
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

# Cooked material slot -> retail MIC to resolve, per class.  The slot name is what the cooked
# model calls the submesh's material; the source is the retail instance whose values it takes.
HEAD_FAMILY = "source.character.classic-head.v1"
SKIN_FAMILY = "source.character.classic-skin.v1"
EYE_FAMILY = "source.character.eye.v1"
EYELASH_FAMILY = "source.character.eyelash.v1"

# Register -> the texture parameter the program reads there, per family.  See the module
# docstring for how the head map was read; the skin map is the one DimensionMaster's existing
# pc_sp_av_base_upper_mi row already uses, and the chain reproduces all seven of its entries.
TEXTURE_REGISTERS = {
    HEAD_FAMILY: [
        (0, "var_headbase_normaltexture_ui", "linear"),
        (1, "var_headbase_overlaynormaltexture_ui", "linear"),
        (2, "var_headbase_speculartexture_ui", "srgb"),
        (3, "var_headbase_diffusetexture_ui", "srgb"),
        (4, "var_makeup_liptexture_ui", "srgb"),
        (5, "var_makeup_eyetexture_ui", "srgb"),
        (6, "var_makeup_cheektexture_ui", "srgb"),
        (7, "var_headdeco_decaltexture_ui", "srgb"),
        (8, "texture_ao", "srgb"),
        (9, "texture_ibl_cube", "srgb"),
        (10, "texture_state_fx", "srgb"),
        (11, "texture_specular_power", "linear"),
    ],
    EYE_FAMILY: [
        (0, "texture_normal", "linear"),
        (1, "texture_tdspecular", "srgb"),
        (2, "var_eye_iristexture_ui", "srgb"),
        (3, "texture_diffuse_base", "srgb"),
        (4, "var_eye_iristextureleft_ui", "srgb"),
        (5, "texture_state_fx", "srgb"),
    ],
    EYELASH_FAMILY: [
        (0, "texture_diffuse", "srgb"),
    ],
    SKIN_FAMILY: [
        (0, "texture_normal", "linear"),
        (1, "texture_color_fx_skin", "srgb"),
        (2, "texture_specular", "srgb"),
        (3, "texture_diffuse", "srgb"),
        (4, "texture_ibl", "srgb"),
        (5, "texture_state_fx", "srgb"),
        (6, "texture_brdf", "linear"),
    ],
}

# Registers the material chain never names, because the engine binds them rather than the
# material.  These are the values the two existing DimensionMaster rows already carry, and this
# tool reproduces both rows exactly.
ENGINE_REGISTER_TEXTURES = {
    HEAD_FAMILY: {9: "hdr07_1", 10: "statefx_default", 11: "brdf_beckmann_spec"},
    SKIN_FAMILY: {5: "statefx_default"},
    EYE_FAMILY: {5: "statefx_default"},
    EYELASH_FAMILY: {},
}

TEXTURE_ROOTS = {
    "LanceMaster": "Character/LanceMaster/textures",
    "Warlord": "Character/Warlord/textures",
    "Artist": "Character/Artist/textures",
}

# One catalog row each: the cooked material slot, the retail MIC whose resolved chain fills it,
# and the family that packs it.  The face rows are explained in the module docstring; the skin
# rows are every material of these bodies that sits on a `*_parts_*_high` master, because that
# master is what declares `var_base_skincolor_ui` and its mask decides where skin shows -- so
# "which material is skin" is not a judgement this tool has to make.
#
# `pc_ft_01_arm_mi` is deliberately absent. It sits on `ft_parts_opa_high`, an opaque master,
# while every material this family is known to cover -- the three existing rows and every one
# below -- sits on a `*_parts_msk_high` masked master. Its 46 parameter names resolve, so the
# name set does not tell the two apart, and nothing establishes that the opaque master compiles
# to the same program. Adding it would be a guess about which shader draws an arm.
ROWS = [
    ("LanceMaster", "pc_ft_face_mi", "pc_ft_face_mi", HEAD_FAMILY),
    ("Warlord", "pc_wr_face_mi", "pc_wr_face_mi_high", HEAD_FAMILY),
    ("Artist", "pc_sp_face_mi_high", "pc_sp_face_mi_high", HEAD_FAMILY),

    ("LanceMaster", "pc_ft_01_upper_mi", "pc_ft_01_upper_mi", SKIN_FAMILY),
    ("LanceMaster", "pc_ft_01_lower_mi", "pc_ft_01_lower_mi", SKIN_FAMILY),
    ("Warlord", "pc_wr_00_arm_mi", "pc_wr_00_arm_mi", SKIN_FAMILY),
    ("Warlord", "pc_wr_00_upper_mi", "pc_wr_00_upper_mi", SKIN_FAMILY),
    ("Warlord", "pc_wr_00_lower_mi", "pc_wr_00_lower_mi", SKIN_FAMILY),
    ("Warlord", "pc_wr_base_upper_mi", "pc_wr_base_upper_mi", SKIN_FAMILY),
    ("Artist", "pc_sp_av_base_body_mi", "pc_sp_av_base_body_mi", SKIN_FAMILY),
    ("Artist", "pc_sp_01_upper_mi", "pc_sp_01_upper_mi", SKIN_FAMILY),
    ("Artist", "pc_sp_01-1_arm_mi", "pc_sp_01-1_arm_mi", SKIN_FAMILY),
    ("Artist", "pc_sp_01-1_lower_mi", "pc_sp_01-1_lower_mi", SKIN_FAMILY),

    # The eye family is program 5, and CModel rejects a program 5 override on a submesh
    # without native UV1 and UV2 (Model.cpp, "source character requires native extra UV
    # channels"). Those channels live in a WModel cooked at 1.3, which
    # Tools/ModelAssetConverter/cook_ocular_uv_channels.py appends from the retail body.
    ("LanceMaster", "pc_ft_eye_mi", "pc_ft_eye_mi", EYE_FAMILY),
    ("Warlord", "pc_wr_eye_mi", "pc_wr_eye_mi", EYE_FAMILY),
    ("Artist", "pc_sp_eye_mi", "pc_sp_eye_mi", EYE_FAMILY),
    ("LanceMaster", "pc_ft_eyelashes_mi", "pc_ft_eyelashes_mi", EYELASH_FAMILY),
    ("Artist", "pc_sp_eyeao_mi", "pc_sp_eyeao_mi", EYELASH_FAMILY),
]

MODEL_ASSETS = {
    "LanceMaster": "Character/LanceMaster/LanceMaster.wmodel",
    "Warlord": "Character/Warlord/Warlord.wmodel",
    "Artist": "Character/Artist/Artist.wmodel",
}

# Textures the master supplies rather than the class: they live once under SourceMaterials or in
# a class folder that already ships them, and every row points at the same copy.
SHARED_TEXTURES = {
    "null": "Character/SourceMaterials/efmaster_material_prologue/null.tga",
    "flat_white": "Character/SourceMaterials/efmaster_material_prologue/flat_white.tga",
    "flat_black": "Character/SourceMaterials/efmaster_material_prologue/flat_black.tga",
    "hdr07_1": "Character/SourceMaterials/efmaster_material_prologue/hdr07_1.tga",
    "statefx_default": "Character/SourceMaterials/efmaster_material_prologue/statefx_default.tga",
    "normal": "Character/LanceMaster/textures/normal.tga",
    "diffuse": "Character/SourceMaterials/efmaster_material_prologue/diffuse.dds",
    "lightbox_cube2_1": "Character/SourceMaterials/efmaster_material_prologue/lightbox_cube2_1.dds",
    "brdf_beckmann_spec": "Character/LanceMaster/textures/brdf_beckmann_spec.tga",
}

# Names the material declares but the engine writes per frame -- selection highlight, hit and
# buff flashes and the tool-preview tints.  The retail chain states no value for them because
# nothing authors one, so each is listed here with the neutral that means "not driven", rather
# than defaulted silently.
RUNTIME_INPUTS = {
    "selectioncolor": [0.0, 0.0, 0.0, 1.0],
}

PARENT = re.compile(r"^Parent = \w+'([^']+)'")
NAMED_BLOCK = re.compile(r"^\s*(\w+)\[\d+\] =\s*$")
COMPACT_BLOCK = re.compile(r"^\s*(\w+)\[\d+\] = \{(.*)\}\s*$")
FIELD = re.compile(r"^\s*(\w+) = (.*?)\s*$")
COLOUR = re.compile(r"\{ *R=([-\d.eE+]+), *G=([-\d.eE+]+), *B=([-\d.eE+]+), *A=([-\d.eE+]+) *\}")
TEXTURE = re.compile(r"Texture2D'(?:[\w.]*\.)?(\w+)'")


def read_props(path: Path):
    """Return (parentPath, {name: value}) for one exported .props.txt.

    Both shapes umodel writes are read: the expanded block and the one-line
    ``Collected...[2] = { Value=60, Name=specular_power, Group=parameters }``."""
    scalars, vectors, textures, parent = {}, {}, {}, None
    fields = {}
    for raw in path.read_text(encoding="utf-8", errors="replace").splitlines():
        match = PARENT.match(raw)
        if match:
            parent = match.group(1)
            continue
        compact = COMPACT_BLOCK.match(raw)
        if compact:
            pairs = {key.strip(): value.strip()
                     for key, value in (pair.split("=", 1)
                                        for pair in compact.group(2).split(", ") if "=" in pair)}
            _store(pairs, scalars, vectors, textures)
            continue
        if raw.strip() == "}":
            # An entry of a parameter list just closed; the outer list's own brace closes an
            # already-empty set, so no depth bookkeeping is needed.
            _store(fields, scalars, vectors, textures)
            fields = {}
            continue
        field = FIELD.match(raw)
        if field and field.group(2):
            fields[field.group(1)] = field.group(2)
    values = {}
    values.update({name: ("scalar", v) for name, v in scalars.items()})
    values.update({name: ("vector", v) for name, v in vectors.items()})
    values.update({name: ("texture", v) for name, v in textures.items()})
    return parent, values


def _store(fields, scalars, vectors, textures):
    name = fields.get("ParameterName") or fields.get("Name")
    if not name:
        return
    name = name.strip()
    raw = fields.get("ParameterValue")
    if raw is None:
        raw = fields.get("Value")
    if raw is None:
        raw = fields.get("Texture")
    if raw is None:
        return
    raw = raw.strip()
    colour = COLOUR.search(raw)
    texture = TEXTURE.search(raw)
    if colour:
        vectors.setdefault(name, [float(colour.group(i)) for i in range(1, 5)])
    elif texture:
        textures.setdefault(name, texture.group(1))
    else:
        try:
            scalars.setdefault(name, float(raw))
        except ValueError:
            pass


def resolve_chain(root: Path, leaf: str):
    """Walk a material's parents child-first and return {name: (kind, value)}."""
    # A parent is named by its group path, and the same leaf name appears twice in a chain: the
    # preset MaterialInstanceConstant `mastermaterial_ch_preset.pbr.pbr_base_opa` and the
    # Material3 `pbr.pbr_base_opa` it inherits from.  Keying on the name alone reads the first
    # and never reaches the master, where every default actually lives -- so the index keeps the
    # whole exported path and a parent is matched by its longest suffix.
    index = {}
    for path in root.rglob("*.props.txt"):
        dotted = "/".join(path.relative_to(root).parts)[:-len(".props.txt")].lower()
        index[dotted.replace("/", ".")] = path

    def find(reference: str):
        wanted = reference.lower().split(".")
        best = None
        for dotted, path in index.items():
            parts = dotted.split(".")
            if parts[-len(wanted):] == wanted and (best is None or len(parts) < best[0]):
                best = (len(parts), path)
        return best[1] if best else None

    resolved, seen, reference, chain = {}, set(), leaf, []
    while reference and reference.lower() not in seen:
        seen.add(reference.lower())
        path = find(reference)
        if path is None:
            raise SystemExit("%s: no export for %r under %s -- run umodel on its package first"
                             % (leaf, reference, root))
        chain.append(reference)
        parent, values = read_props(path)
        for key, value in values.items():
            resolved.setdefault(key, value)
        reference = parent
    return resolved, chain


def rule_document_textures(data_root: Path, class_id: str):
    """Texture defaults the CharacterCustomizingRule document states, by parameter name.

    The chain is silent about a texture the creation screen still sets -- LanceMaster's wrinkle
    overlay is the case in point -- and the rule document is where retail states it.  Its
    ``sourceTexture`` is a package object path, so only the object name is taken."""
    path = data_root / "UI" / "Customizing" / "CustomizingMeshTypes.json"
    classes = json.loads(path.read_text(encoding="utf-8"))["classes"]
    entry = classes.get(class_id)
    if entry is None:
        return {}
    defaults = {}
    for variable in entry.get("textureVariables", []):
        source = variable.get("sourceTexture")
        if source:
            defaults.setdefault(variable["variable"].lower(), source.rsplit(".", 1)[-1].lower())
    return defaults


def required_parameters(header: Path, family: str):
    """The parameter names Configure() reads for one family, from the header itself."""
    text = header.read_text(encoding="utf-8", errors="replace")
    start = text.index('family == "%s"' % family)
    end = text.find('if (family == "', start + 10)
    return sorted(set(re.findall(r'parameter\("([^"]+)"\)', text[start:end if end > 0 else len(text)])))


def main() -> int:
    repo = Path(__file__).resolve().parents[2]
    parser = argparse.ArgumentParser()
    parser.add_argument("--export-root", type=Path, required=True,
                        help="directory umodel exported the materials into")
    parser.add_argument("--class-id", action="append",
                        help="one class; repeat, or omit for every class in the table")
    parser.add_argument("--family", action="append",
                        help="one family; repeat, or omit for every family in the table")
    parser.add_argument("--header", type=Path,
                        default=repo / "Client" / "Public" / "SourceCharacterMaterialParameters.h")
    parser.add_argument("--resources", type=Path,
                        default=repo / "Client" / "Bin" / "Resources")
    parser.add_argument("--data", type=Path, default=repo / "Data")
    args = parser.parse_args()

    needed = {family: required_parameters(args.header, family)
              for family in TEXTURE_REGISTERS}
    rows, missing_files = [], []
    for class_id, material_name, source_material, family in ROWS:
        if (args.class_id and class_id not in args.class_id) or \
           (args.family and family not in args.family):
            continue
        resolved, chain = resolve_chain(args.export_root, source_material)
        print("%-22s %s" % (material_name, " -> ".join(chain[1:])), file=sys.stderr)

        parameters, absent = {}, []
        for name in needed[family]:
            # Configure() spells one parameter with a trailing space; the material does not.
            found = resolved.get(name) or resolved.get(name.strip())
            if found is not None:
                parameters[name] = found[1]
            elif name.strip() in RUNTIME_INPUTS:
                parameters[name] = RUNTIME_INPUTS[name.strip()]
            else:
                absent.append(name)
        if absent:
            raise SystemExit("%s: the chain never states %s, and they are not listed as engine "
                             "runtime inputs" % (material_name, absent))

        rule_textures = rule_document_textures(args.data, class_id)
        texture_root = TEXTURE_ROOTS[class_id]
        textures = []
        for index, name, colour_space in TEXTURE_REGISTERS[family]:
            found = resolved.get(name)
            texture = found[1] if found is not None and found[0] == "texture" else None
            if texture is None:
                texture = (rule_textures.get(name) or
                           ENGINE_REGISTER_TEXTURES[family].get(index))
            if texture is None:
                raise SystemExit("%s: register %d (%s) is stated by neither the material chain, "
                                 "the rule document nor the engine table"
                                 % (material_name, index, name))
            asset = SHARED_TEXTURES.get(texture)
            if asset is None:
                # Whichever container the texture was installed in: the older cook wrote TGA,
                # a BC5 normal map is kept as the DDS umodel exports rather than re-encoded.
                for suffix in (".tga", ".dds"):
                    candidate = "%s/%s%s" % (texture_root, texture, suffix)
                    if (args.resources / candidate).exists():
                        asset = candidate
                        break
                else:
                    asset = "%s/%s.tga" % (texture_root, texture)
            if not (args.resources / asset).exists():
                missing_files.append("%s (%s register %d)" % (asset, material_name, index))
            textures.append({"expressionIndex": index, "assetId": asset,
                             "colorSpace": colour_space})

        rows.append({"modelAssetId": MODEL_ASSETS[class_id],
                     "materialName": material_name,
                     "family": family,
                     "sourceMaterial": source_material,
                     "parameters": parameters,
                     "textures": textures})

    if missing_files:
        print("\nthese Resources files are not installed yet:", file=sys.stderr)
        for line in sorted(set(missing_files)):
            print("   " + line, file=sys.stderr)
    print(json.dumps(rows, indent=2, ensure_ascii=False))
    return 1 if missing_files else 0


if __name__ == "__main__":
    raise SystemExit(main())
