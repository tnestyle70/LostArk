"""Convert an extracted Cascade recipe into a LostArk Effect Tool authoring file.

The recipe JSON produced by extract_effect_recipes.py keeps the original UE3
module graph. The Effect Tool reads a flat `.effect` text file. This script maps
one ParticleSystem from the first onto the second so an effect arrives in the
tool already assembled instead of being typed in by hand.

Usage:
    python convert_recipe_to_effect.py \
        --recipe <...>/fx_pc_flm_03.recipe.json \
        --object par_n_flm_dash_02 \
        --asset-id glaivier-dash-02 \
        --out <...>/Effect_Tool/Authored/glaivier-dash-02.effect \
        --texture "../Bin/Resources/.../fx_a_fragment_007.dds"

What is exact and what is not:
  * module parameters, material reference, SubUV layout, durations and loop
    counts come straight from the package and are exact.
  * a baked distribution stores its authored range in the first two entries;
    that was verified against 64,719 baked tables in the Glaivier recipes and
    holds for 97.6% of them. The remaining tables fall back to the range of the
    whole table.
  * over-life curves are emitted with evenly spaced time keys. The package does
    not state the time axis, so this is an assumption and the asset is marked
    DERIVED_CONVERSION rather than GAME_ORIGINAL.
  * schema-v5 material controls are inferred from case-insensitive UE3
    parameter names. The original material path is retained in `mat`, and each
    inferred mapping is printed as a note for review.
  * DynamicParameter keeps the source array order as X/Y/Z/W. Parameter names
    and unsupported Cascade evaluation flags are reported instead of discarded.
  * modules the tool cannot represent are skipped and listed on stderr. They are
    not silently replaced with defaults.
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path
from typing import Any

# UE3 authors in centimetres, the client works in metres.
DEFAULT_SCALE = 0.01

# Cascade module class -> Effect Tool module type.
MODULE_MAP = {
    "particlemodulerequired": "REQUIRED",
    "particlemodulespawn": "SPAWN",
    "particlemodulelifetime": "LIFETIME",
    "particlemodulelifetime_seeded": "LIFETIME",
    "particlemodulesize": "INITIAL_SIZE",
    "particlemodulevelocity": "INITIAL_VELOCITY",
    "particlemodulecolor": "INITIAL_COLOR",
    "particlemodulerotation": "INITIAL_ROTATION",
    "particlemodulerotationrate": "ROTATION_RATE",
    "particlemodulesizemultiplylife": "SIZE_OVER_LIFE",
    "particlemodulecolorscaleoverlife": "COLOR_OVER_LIFE",
    # Cascade separates "scale the colour" from "set the colour". The tool has
    # one multiplying curve, which matches either as long as Initial Color is
    # left at white; the note below flags the case where it is not.
    "particlemodulecoloroverlife": "COLOR_OVER_LIFE",
    "particlemodulesubuv": "SUB_UV",
    "particlemodulelocation": "INITIAL_LOCATION",
    "particlemodulelocationprimitivesphere": "INITIAL_LOCATION",
    "particlemodulelocationprimitivecylinder": "INITIAL_LOCATION",
    "particlemoduleparameterdynamic": "DYNAMIC_PARAMETER",
}

SCREEN_ALIGNMENT_MAP = {
    "psa_square": "SQUARE",
    "psa_rectangle": "RECTANGLE",
    "psa_velocity": "VELOCITY",
}

BLEND_HINTS = (("_tr", "ALPHA"), ("_ad", "ADDITIVE"))


def quote(value: str) -> str:
    escaped = value.replace("\\", "\\\\").replace('"', '\\"')
    return f'"{escaped}"'


def number(value: float) -> str:
    text = f"{value:.6g}"
    return "0" if text in ("-0", "-0.0") else text


class Range:
    """Authored min/max plus the baked samples that follow the header."""

    def __init__(self, low: float, high: float, samples: list[float]):
        self.low = low
        self.high = high
        self.samples = samples

    @property
    def is_constant(self) -> bool:
        return abs(self.high - self.low) < 1e-9


def read_distribution(node: Any) -> Range | None:
    if not isinstance(node, dict):
        return None

    kind = node.get("kind")
    if kind == "empty":
        return None

    if kind == "authored":
        # A parameter distribution is filled in by gameplay code at runtime.
        # Only its fallback constant exists in the package.
        constant = (node.get("values") or {}).get("constant")
        if isinstance(constant, dict):
            value = float(constant.get("x", 0.0))
            return Range(value, value, [value])
        if isinstance(constant, (int, float)):
            return Range(float(constant), float(constant), [float(constant)])
        return None

    table = node.get("lookup_table")
    if not isinstance(table, list) or not table:
        return None
    values = [float(v) for v in table]
    if len(values) < 3:
        return Range(min(values), max(values), values)

    head, tail = values[:2], values[2:]
    low, high = min(head), max(head)
    if all(low - 1e-4 <= v <= high + 1e-4 for v in tail):
        return Range(low, high, tail)
    # Vector tables occasionally carry a per-component header. Falling back to
    # the whole table keeps the value inside the authored bounds.
    return Range(min(values), max(values), values)


def vector_triples(samples: list[float]) -> list[tuple[float, float, float]]:
    count = len(samples) // 3
    return [tuple(samples[i * 3:i * 3 + 3]) for i in range(count)]


def curve_text(samples: list[float], stride: int = 1) -> str:
    if stride > 1:
        samples = [samples[i] for i in range(0, len(samples), stride)]
    if not samples:
        return ""
    if len(samples) == 1:
        return f"0:{number(samples[0])},1:{number(samples[0])}"
    step = 1.0 / (len(samples) - 1)
    return ",".join(
        f"{number(i * step)}:{number(v)}" for i, v in enumerate(samples)
    )


def scalar_distribution_fields(rng: Range | None, scale: float = 1.0) -> str:
    if rng is None:
        return ""
    low, high = rng.low * scale, rng.high * scale
    if rng.is_constant:
        return f" dist=CONSTANT c={number(low)} min={number(low)} max={number(high)}"
    return (
        f" dist=UNIFORM_RANGE c={number(low)}"
        f" min={number(low)} max={number(high)}"
    )


def vector_distribution_fields(rng: Range | None, scale: float = 1.0) -> str:
    if rng is None:
        return ""
    low, high = rng.low * scale, rng.high * scale
    lo = f"{number(low)},{number(low)},{number(low)}"
    hi = f"{number(high)},{number(high)},{number(high)}"
    kind = "CONSTANT" if rng.is_constant else "UNIFORM_RANGE"
    return f" dist={kind} c={lo} min={lo} max={hi}"


# Texture parameter names worth preferring when a material declares several.
# They are canonicalized before comparison, so UE3 names such as
# ``Texture(SubUV)`` and ``diffuse_tex`` work without case-sensitive aliases.
TEXTURE_PRIORITY = (
    "subuvtex",
    "texturesubuv",
    "diffuse",
    "diffusetex",
    "basetex",
    "base",
    "emissivetex",
    "texture",
    "masktex",
)

OPACITY_TEXTURE_NAMES = {
    "opacity",
    "opacitytex",
    "opacitytexture",
    "opacitymask",
    "opacitymasktex",
    "alphatex",
    "alphamask",
    "masktex",
}

DISSOLVE_TEXTURE_NAMES = {
    "dissolve",
    "dissolvetex",
    "dissolvetexture",
    "dissolvemask",
    "dissolvemasktex",
}

DISTORTION_TEXTURE_NAMES = {
    "distortion",
    "distortiontex",
    "distortiontexture",
    "distorttex",
    "flowmap",
    "flowmaptex",
    "normalmap",
    "normaltex",
}


def canonical_name(value: Any) -> str:
    """Case/punctuation-insensitive UE3 material parameter name."""
    return "".join(ch for ch in str(value or "").casefold() if ch.isalnum())


def material_key(path: str) -> str:
    """Join a recipe material reference to the package-independent lookup."""
    normalized = str(path or "").casefold()
    parts = normalized.split(".")
    return ".".join(parts[1:]) if len(parts) > 2 else normalized


def load_material_map(path: Path | None) -> dict[str, list[dict]]:
    if path is None:
        return {}
    document = json.loads(path.read_text(encoding="utf-8"))
    raw = document.get("materials", document)
    if not isinstance(raw, dict):
        return {}

    # Schema 1 maps only carried parent/textures. Schema 2 adds the original
    # path and scalar/vector parameters. Normalizing both here keeps old maps
    # and third-party hand-authored maps valid.
    normalized: dict[str, list[dict]] = {}
    for key, value in raw.items():
        entries = value if isinstance(value, list) else [value]
        normalized[str(key).casefold()] = [
            entry for entry in entries if isinstance(entry, dict)
        ]
    return normalized


class Converter:
    def __init__(self, scale: float, texture: str, verbose: bool,
                 material_map: dict[str, list[dict]] | None = None):
        self.scale = scale
        self.texture = texture
        self.verbose = verbose
        self.material_map = material_map or {}
        self.skipped: list[str] = []
        self.notes: list[str] = []
        self.unresolved_materials: list[str] = []
        self.next_id = 1
        self.pending_mesh = ""
        self.synthesized_spawn = 0
        self.synthesized_lifetime = 0
        self.mesh_root: Path | None = None
        self.mesh_index: set[str] = set()
        self.mesh_prefix = ""
        self.mesh_resolved = 0
        self.mesh_missing = 0
        self.empty_lod_emitters = 0
        self.unresolved_module_ref_emitters = 0
        self.unresolved_module_ref_occurrences = 0
        self._material_layers_cache: dict[str, list[dict]] = {}
        self._material_controls_cache: dict[str, dict[str, Any]] = {}

    def resolve_mesh(self, mesh_path: str) -> str:
        """Map a Cascade mesh reference onto a cooked .wmodel the tool loads."""
        if not mesh_path or not self.mesh_root:
            return ""
        # "fx_sm_00.fm_o_swing_02" -> "fm_o_swing_02.wmodel"
        name = mesh_path.split(".")[-1]
        if name in self.mesh_index:
            return f"{self.mesh_prefix}/{name}.wmodel"
        return ""

    def material_layers(self, material_path: str) -> list[dict]:
        """Return parent-first material entries, accepting schema 1 and 2 maps."""
        key = material_key(material_path)
        cached = self._material_layers_cache.get(key)
        if cached is not None:
            return cached

        layers: list[dict] = []
        visited: set[str] = set()

        def append_key(current_key: str, depth: int) -> None:
            if not current_key or current_key in visited:
                return
            if depth > 16:
                self.notes.append(
                    "material parent chain exceeded 16 levels; remaining "
                    f"parents below {current_key!r} were ignored"
                )
                return
            visited.add(current_key)
            entries = self.material_map.get(current_key, [])
            if len(entries) > 1:
                self.notes.append(
                    f"material lookup {current_key!r} has {len(entries)} "
                    "package candidates; parameters were merged in map order"
                )
            for entry in entries:
                parent = entry.get("parent")
                if parent:
                    append_key(material_key(str(parent)), depth + 1)
                layers.append(entry)

        append_key(key, 0)
        self._material_layers_cache[key] = layers
        return layers

    @staticmethod
    def _vector(value: Any) -> tuple[float, float, float, float] | None:
        if not isinstance(value, dict):
            return None
        try:
            return (
                float(value.get("x", value.get("r", 0.0))),
                float(value.get("y", value.get("g", 0.0))),
                float(value.get("z", value.get("b", 0.0))),
                float(value.get("w", value.get("a", 0.0))),
            )
        except (TypeError, ValueError):
            return None

    def material_parameters(
        self, material_path: str
    ) -> tuple[dict[str, dict], dict[str, tuple[str, float]],
               dict[str, tuple[str, tuple[float, float, float, float]]]]:
        """Merge inherited parameters with child values overriding parents."""
        textures: dict[str, dict] = {}
        scalars: dict[str, tuple[str, float]] = {}
        vectors: dict[
            str, tuple[str, tuple[float, float, float, float]]
        ] = {}
        for entry in self.material_layers(material_path):
            for item in entry.get("textures", []):
                if isinstance(item, dict) and item.get("dds_path"):
                    textures[canonical_name(item.get("name"))] = item
            for item in entry.get("scalars", []):
                if not isinstance(item, dict):
                    continue
                try:
                    value = float(item.get("value"))
                except (TypeError, ValueError):
                    continue
                name = str(item.get("name") or "")
                scalars[canonical_name(name)] = (name, value)
            for item in entry.get("vectors", []):
                if not isinstance(item, dict):
                    continue
                value = self._vector(item.get("value"))
                if value is None:
                    continue
                name = str(item.get("name") or "")
                vectors[canonical_name(name)] = (name, value)
        return textures, scalars, vectors

    def _mapping_note(self, source_name: str, target: str) -> None:
        self.notes.append(
            f"material parameter {source_name!r} inferred as {target} "
            "from its case-insensitive name"
        )

    def _role_texture(
        self,
        textures: dict[str, dict],
        exact_names: set[str],
        tokens: tuple[str, ...],
        target: str,
    ) -> str:
        for name, item in textures.items():
            if name in exact_names:
                self._mapping_note(str(item.get("name") or name), target)
                return str(item.get("dds_path") or "")
        for name, item in textures.items():
            if any(token in name for token in tokens):
                self._mapping_note(str(item.get("name") or name), target)
                return str(item.get("dds_path") or "")
        return ""

    def resolve_texture(self, material_path: str) -> str:
        """Map a Cascade material reference onto a DDS the tool can load."""
        if self.texture:
            return self.texture
        if not material_path or not self.material_map:
            return ""

        textures, _, _ = self.material_parameters(material_path)
        if not textures:
            self.unresolved_materials.append(material_path)
            return ""

        for wanted in TEXTURE_PRIORITY:
            if wanted in textures:
                return str(textures[wanted].get("dds_path") or "")
        return str(next(iter(textures.values())).get("dds_path") or "")

    def material_controls(self, material_path: str) -> dict[str, Any]:
        """Best-effort projection of UE3 parameters onto schema-v5 controls.

        The flat material map preserves every source parameter. This function
        maps only names whose intent is reasonably explicit; every inference
        is reported as a note instead of being silently presented as exact.
        """
        key = material_key(material_path)
        cached = self._material_controls_cache.get(key)
        if cached is not None:
            return cached

        controls: dict[str, Any] = {
            "opacitytex": "",
            "dissolvetex": "",
            "distortiontex": "",
            "uvtiling": (1.0, 1.0),
            "uvoffset": (0.0, 0.0),
            "uvpanner": (0.0, 0.0),
            "emissive": 1.0,
            "opacitythreshold": 0.0,
            "dissolveamount": 0.0,
            "dissolveedgewidth": 0.05,
            "dissolveedgecolor": (0.0, 0.0, 0.0, 0.0),
            "softdistance": 0.0,
            "distortionstrength": 0.0,
        }
        if not material_path or not self.material_map:
            self._material_controls_cache[key] = controls
            return controls

        textures, scalars, vectors = self.material_parameters(material_path)
        controls["opacitytex"] = self._role_texture(
            textures,
            OPACITY_TEXTURE_NAMES,
            ("opacity", "alphamask", "alphatex", "alphamap", "mainalpha",
             "texalpha"),
            "opacitytex",
        )
        controls["dissolvetex"] = self._role_texture(
            textures,
            DISSOLVE_TEXTURE_NAMES,
            ("dissolve", "disslove", "dissov"),
            "dissolvetex",
        )
        controls["distortiontex"] = self._role_texture(
            textures,
            DISTORTION_TEXTURE_NAMES,
            ("distort", "uvdistort", "flowmap", "flowtex", "normalmap",
             "normaltex"),
            "distortiontex",
        )

        def scalar(aliases: tuple[str, ...]) -> tuple[str, float] | None:
            for alias in aliases:
                found = scalars.get(canonical_name(alias))
                if found is not None:
                    return found
            # Numeric ordering prefixes ("05.distort_str") and texture
            # qualifiers ("maintex_tile_x") are common. Match a requested
            # semantic suffix only after exact names have failed.
            for alias in aliases:
                suffix = canonical_name(alias)
                if len(suffix) < 4:
                    continue
                for name, found in scalars.items():
                    if name.endswith(suffix):
                        return found
            return None

        def vector(
            aliases: tuple[str, ...]
        ) -> tuple[str, tuple[float, float, float, float]] | None:
            for alias in aliases:
                found = vectors.get(canonical_name(alias))
                if found is not None:
                    return found
            for alias in aliases:
                suffix = canonical_name(alias)
                if len(suffix) < 4:
                    continue
                for name, found in vectors.items():
                    if name.endswith(suffix):
                        return found
            return None

        def set_scalar(target: str, aliases: tuple[str, ...]) -> bool:
            found = scalar(aliases)
            if found is None:
                return False
            source, value = found
            controls[target] = value
            self._mapping_note(source, target)
            return True

        def set_pair(
            target: str,
            vector_aliases: tuple[str, ...],
            uniform_aliases: tuple[str, ...],
            x_aliases: tuple[str, ...],
            y_aliases: tuple[str, ...],
        ) -> None:
            vector_value = vector(vector_aliases)
            if vector_value is not None:
                source, value = vector_value
                controls[target] = (value[0], value[1])
                self._mapping_note(source, target)
                return
            uniform = scalar(uniform_aliases)
            if uniform is not None:
                source, value = uniform
                controls[target] = (value, value)
                self._mapping_note(source, target)
                return
            old = controls[target]
            x_value = scalar(x_aliases)
            y_value = scalar(y_aliases)
            if x_value is not None:
                controls[target] = (x_value[1], controls[target][1])
                self._mapping_note(x_value[0], f"{target}.x")
            if y_value is not None:
                controls[target] = (controls[target][0], y_value[1])
                self._mapping_note(y_value[0], f"{target}.y")
            if x_value is None and y_value is None:
                controls[target] = old

        set_pair(
            "uvtiling",
            ("uvtiling", "texturetiling", "emissivetiling", "opacitytiling"),
            ("tiling", "tile", "uvscale", "emissive_tiling"),
            (
                "maintex_tile_x", "maintex_tile_u", "tilingx", "tilex",
                "x_tile", "utile", "uvtilingx",
            ),
            (
                "maintex_tile_y", "maintex_tile_v", "tilingy", "tiley",
                "y_tile", "vtile", "uvtilingy",
            ),
        )
        set_pair(
            "uvoffset",
            ("uvoffset", "textureoffset", "emissiveoffset", "opacityoffset"),
            (),
            ("offsetx", "uoffset", "uvoffsetx"),
            ("offsety", "voffset", "uvoffsety"),
        )
        set_pair(
            "uvpanner",
            ("uvpanner", "panner", "pannerspeed", "uvspeed"),
            (),
            (
                "maintex_panspeed_x", "maintex_pan_u_time", "pannerx",
                "panspeedx", "uspeed", "emissive_pan_x",
            ),
            (
                "maintex_panspeed_y", "maintex_pan_v_time", "pannery",
                "panspeedy", "vspeed", "emissive_pan_y",
            ),
        )

        if not set_scalar(
            "emissive",
            (
                "emissive", "emissivestrength", "emissivepower",
                "emissivedensity", "emissivemultiplier", "emissivemultiply",
                "emissiveintensity", "emissivestr", "emissiionpower",
                "emissiionstr", "colorpower", "colordensity",
            ),
        ):
            color_density = vector(("colordensity", "emissivecolor"))
            if color_density is not None:
                source, value = color_density
                controls["emissive"] = max(value[0], value[1], value[2])
                self._mapping_note(source, "emissive (max RGB)")

        set_scalar(
            "opacitythreshold",
            (
                "opacitymaskthreshold", "opacitythreshold", "alphaclip",
                "clipvalue", "maskthreshold", "cutoff",
            ),
        )
        set_scalar(
            "dissolveamount",
            (
                "dissolveamount", "dissolvedensity", "dissolve_density",
                "alpha_dissolve", "dissolvevalue",
            ),
        )
        set_scalar(
            "dissolveedgewidth",
            ("dissolveedgewidth", "dissolverange", "edgerange"),
        )
        edge_color = vector(("dissolveedgecolor", "dissolvecolor", "edgecolor"))
        if edge_color is not None:
            source, value = edge_color
            controls["dissolveedgecolor"] = value
            self._mapping_note(source, "dissolveedgecolor")
        set_scalar(
            "softdistance",
            (
                "softparticledistance", "softdistance", "softparticlefade",
                "softfadedistance", "depthfade",
            ),
        )
        set_scalar(
            "distortionstrength",
            (
                "distortionstrength", "distortion_str", "distortstrength",
                "distort_str", "uvdistortionstr", "refractionstrength",
                "distortionpower", "distortionintensity",
            ),
        )

        self._material_controls_cache[key] = controls
        return controls

    def take_id(self) -> int:
        value = self.next_id
        self.next_id += 1
        return value

    def convert(self, asset: dict, asset_id: str) -> str:
        emitters = asset.get("emitters") or []
        lines = ["LOSTARK_EFFECT 5"]

        # The preview has to include the tail of the last particle, not only
        # the interval in which the emitter creates new particles.
        durations = [
            self.emitter_preview_duration(emitter)
            for emitter in emitters
        ]
        duration = max(durations) if durations else 1.0

        lines.append(
            "ASSET"
            f" id={quote(asset_id)}"
            f" name={quote(asset.get('object_name', asset_id))}"
            " provenance=DERIVED_CONVERSION"
            f" duration={number(duration)}"
            " warmup=0"
        )

        for index, emitter in enumerate(emitters):
            lines.extend(self.convert_emitter(emitter, index))
        return "\n".join(lines) + "\n"

    @staticmethod
    def lod_modules(emitter: dict) -> list[dict]:
        levels = emitter.get("lod_levels") or []
        if not levels:
            return []
        return levels[0].get("modules") or []

    def find_required(self, emitter: dict) -> dict | None:
        for module in self.lod_modules(emitter):
            if module.get("class") == "particlemodulerequired":
                return module.get("params") or {}
        return None

    def emitter_preview_duration(self, emitter: dict) -> float:
        required = self.find_required(emitter) or {}
        delay = float(required.get("emitterdelay") or 0.0)
        emitter_duration = float(
            required.get("emitterduration") or 1.0)
        raw_loop_count = required.get("emitterloops")
        loop_count = 1 if raw_loop_count is None else int(raw_loop_count)

        max_lifetime = 0.0
        for module in self.lod_modules(emitter):
            if module.get("class") not in (
                    "particlemodulelifetime",
                    "particlemodulelifetime_seeded"):
                continue
            lifetime = read_distribution(
                (module.get("params") or {}).get("lifetime"))
            if lifetime is not None:
                max_lifetime = max(
                    max_lifetime, lifetime.low, lifetime.high)

        # Loop count 0 means forever. The authoring preview still needs a
        # finite cycle, so use one emission cycle plus the longest particle.
        preview_loop_count = max(1, loop_count)
        return (
            max(0.0, delay) +
            max(0.0, emitter_duration) * preview_loop_count +
            max(0.0, max_lifetime)
        )

    def convert_emitter(self, emitter: dict, index: int) -> list[str]:
        lod_levels = emitter.get("lod_levels") or []
        has_lod = bool(lod_levels)
        if not has_lod:
            self.empty_lod_emitters += 1
        unresolved_refs = (
            lod_levels[0].get("unresolved_module_refs") or []
            if has_lod else []
        )
        has_unresolved_modules = bool(unresolved_refs)
        if has_unresolved_modules:
            self.unresolved_module_ref_emitters += 1
            self.unresolved_module_ref_occurrences += len(unresolved_refs)
        required = self.find_required(emitter) or {}
        material = (required.get("material") or {}).get("path", "")
        delay = float(required.get("emitterdelay") or 0.0)
        raw_loop_count = required.get("emitterloops")
        loop_count = 1 if raw_loop_count is None else int(raw_loop_count)

        blend = "ALPHA"
        for suffix, mode in BLEND_HINTS:
            if material.endswith(suffix):
                blend = mode

        # A mesh emitter draws geometry, not a billboard. The package names the
        # source mesh, and the cook step turns that name into a .wmodel that
        # CModel loads directly. An emitter whose mesh has not been cooked stays
        # disabled: drawing it as a sprite would look nothing like the original.
        self.pending_mesh = ""
        source_mesh = ""
        for module in self.lod_modules(emitter):
            if module.get("class") == "particlemoduletypedatamesh":
                source_mesh = (
                    (module.get("params") or {}).get("mesh") or {}
                ).get("path", "")
        is_mesh = bool(source_mesh)
        mesh_ready = False
        if is_mesh:
            self.pending_mesh = self.resolve_mesh(source_mesh)
            mesh_ready = bool(self.pending_mesh)
            if mesh_ready:
                self.mesh_resolved += 1
            else:
                self.pending_mesh = source_mesh
                self.mesh_missing += 1

        lines = [
            "",
            "EMITTER"
            f" id={self.take_id()}"
            f" name={quote(emitter.get('emitter_name') or f'emitter_{index}')}"
            f" type={'MESH' if is_mesh else 'SPRITE'}"
            f" enabled={0 if (not has_lod or has_unresolved_modules or (is_mesh and not mesh_ready)) else 1}"
            " space=LOCAL"
            f" blend={blend}"
            " sort=AGE_NEWEST_FIRST"
            f" delay={number(delay)}"
            f" duration={number(float(required.get('emitterduration') or 1.0))}"
            f" loops={loop_count}",
        ]

        emitted_subuv = False
        has_spawn = has_lifetime = False
        for module in self.lod_modules(emitter):
            klass = module.get("class", "")
            target = MODULE_MAP.get(klass)
            if target is None:
                self.skipped.append(klass)
                continue
            text = self.convert_module(klass, target, module, required)
            if text:
                lines.append(text)
                if target == "SUB_UV":
                    emitted_subuv = True
                elif target == "SPAWN":
                    has_spawn = True
                elif target == "LIFETIME":
                    has_lifetime = True

        if not emitted_subuv:
            columns = int(required.get("subimages_horizontal") or 1)
            rows = int(required.get("subimages_vertical") or 1)
            if columns > 1 or rows > 1:
                lines.append(self.subuv_line(required))

        # The simulator refuses to start without an enabled Spawn and Lifetime
        # module, so an emitter missing either would silently draw nothing.
        # Mesh emitters are already disabled, but a sprite emitter has to be
        # given something. These are invented values, counted and reported.
        if has_lod and not has_unresolved_modules and (not is_mesh or mesh_ready):
            duration = float(required.get("emitterduration") or 1.0)
            peak = int(emitter["lod_levels"][0]
                       .get("peak_active_particles") or 0)
            if not has_spawn:
                if is_mesh:
                    # A mesh emitter usually carries a single long lived
                    # particle that the mesh is drawn on, so PeakActive is the
                    # count rather than a per second rate.
                    count = max(1, peak)
                    rate = count / duration if duration > 0.0 else count
                else:
                    rate = (peak / duration) if (peak and duration > 0.0) \
                        else 20.0
                self.synthesized_spawn += 1
                lines.append(
                    f"  MODULE id={self.take_id()} name={quote('Spawn*')}"
                    f" type=SPAWN enabled=1 rate={number(rate)} burst=0"
                    f" max={self.estimate_max(rate, required)}"
                )
            if not has_lifetime:
                self.synthesized_lifetime += 1
                life = max(0.1, duration)
                lines.append(
                    f"  MODULE id={self.take_id()} name={quote('Lifetime*')}"
                    f" type=LIFETIME enabled=1 dist=CONSTANT"
                    f" c={number(life)} min={number(life)} max={number(life)}"
                )
        return lines

    def subuv_line(self, required: dict) -> str:
        columns = int(required.get("subimages_horizontal") or 1)
        rows = int(required.get("subimages_vertical") or 1)
        frames = max(1, columns * rows)
        duration = float(required.get("emitterduration") or 1.0)
        fps = frames / duration if duration > 0.0 else 30.0
        return (
            f"  MODULE id={self.take_id()} name={quote('SubUV')}"
            f" type=SUB_UV enabled=1 cols={columns} rows={rows}"
            f" fps={number(fps)} start=0 end={frames - 1} loop=1 reltime=1"
        )

    def convert_module(
        self, klass: str, target: str, module: dict, required: dict
    ) -> str:
        params = module.get("params") or {}
        head = (
            f"  MODULE id={self.take_id()}"
            f" name={quote(target.title().replace('_', ' '))}"
            f" type={target} enabled=1"
        )

        if target == "REQUIRED":
            align = SCREEN_ALIGNMENT_MAP.get(
                str(params.get("screenalignment") or "").lower(), "RECTANGLE"
            )
            material = (params.get("material") or {}).get("path", "")
            texture = self.resolve_texture(material)
            controls = self.material_controls(material)
            uv_tiling = controls["uvtiling"]
            uv_offset = controls["uvoffset"]
            uv_panner = controls["uvpanner"]
            edge_color = controls["dissolveedgecolor"]
            return (
                f"{head} tex={quote(texture)}"
                f" mesh={quote(self.pending_mesh)}"
                f" mat={quote(material)} align={align}"
                f" opacitytex={quote(controls['opacitytex'])}"
                f" dissolvetex={quote(controls['dissolvetex'])}"
                f" distortiontex={quote(controls['distortiontex'])}"
                f" uvtiling={number(uv_tiling[0])},{number(uv_tiling[1])}"
                f" uvoffset={number(uv_offset[0])},{number(uv_offset[1])}"
                f" uvpanner={number(uv_panner[0])},{number(uv_panner[1])}"
                f" emissive={number(controls['emissive'])}"
                f" opacitythreshold={number(controls['opacitythreshold'])}"
                f" dissolveamount={number(controls['dissolveamount'])}"
                f" dissolveedgewidth={number(controls['dissolveedgewidth'])}"
                " dissolveedgecolor="
                f"{number(edge_color[0])},{number(edge_color[1])},"
                f"{number(edge_color[2])},{number(edge_color[3])}"
                f" softdistance={number(controls['softdistance'])}"
                f" distortionstrength={number(controls['distortionstrength'])}"
            )

        if target == "SPAWN":
            rate = read_distribution(params.get("rate"))
            value = rate.low if rate else 10.0
            return (
                f"{head} rate={number(value)} burst=0"
                f" max={self.estimate_max(value, required)}"
            )

        if target == "LIFETIME":
            rng = read_distribution(params.get("lifetime"))
            return f"{head}{scalar_distribution_fields(rng)}" if rng else ""

        if target == "INITIAL_SIZE":
            rng = read_distribution(params.get("startsize"))
            return (
                f"{head}{vector_distribution_fields(rng, self.scale)}"
                if rng else ""
            )

        if target == "INITIAL_VELOCITY":
            rng = read_distribution(params.get("startvelocity"))
            return (
                f"{head}{vector_distribution_fields(rng, self.scale)}"
                if rng else ""
            )

        if target == "INITIAL_ROTATION":
            rng = read_distribution(params.get("startrotation"))
            if rng is None:
                return ""
            # Cascade stores rotation as turns, the tool uses degrees.
            return f"{head}{vector_distribution_fields(rng, 360.0)}"

        if target == "ROTATION_RATE":
            rng = read_distribution(params.get("startrotationrate"))
            if rng is None:
                return ""
            return f"{head}{vector_distribution_fields(rng, 360.0)}"

        if target == "INITIAL_COLOR":
            return self.color_module(head, params)

        if target == "INITIAL_LOCATION":
            return self.location_module(head, klass, params)

        if target == "SIZE_OVER_LIFE":
            rng = read_distribution(params.get("lifemultiplier"))
            if rng is None:
                return ""
            triples = vector_triples(rng.samples)
            values = [t[0] for t in triples] or rng.samples
            return f"{head} curve={curve_text(values)}"

        if target == "COLOR_OVER_LIFE":
            return self.color_over_life_module(head, params)

        if target == "SUB_UV":
            return self.subuv_line(required)

        if target == "DYNAMIC_PARAMETER":
            return self.dynamic_parameter_module(head, params)

        return ""

    def dynamic_parameter_module(self, head: str, params: dict) -> str:
        """Map Cascade's ordered DynamicParams array onto X/Y/Z/W curves.

        UE3 materials assign meaning to the four slots by parameter name. The
        runtime contract uses X/Y/Z/W, so slot order is the only lossless part
        we can preserve. Names and unsupported evaluation flags are reported
        as notes for author review.
        """
        source = params.get("dynamicparams")
        dynamic_params = source if isinstance(source, list) else []
        defaults = ([1.0], [1.0], [0.0], [1.0])
        channels = [curve_text(list(values)) for values in defaults]
        labels = "XYZW"

        if not dynamic_params:
            self.notes.append(
                "DynamicParameter had no decodable dynamicparams array; "
                "schema defaults X=1,Y=1,Z=0,W=1 were authored"
            )

        for index, entry in enumerate(dynamic_params[:4]):
            if not isinstance(entry, dict):
                self.notes.append(
                    f"DynamicParameter {labels[index]} entry was not an "
                    "object; the schema default was kept"
                )
                continue
            source_name = str(entry.get("paramname") or f"slot_{index}")
            distribution = entry.get("paramvalue")
            rng = read_distribution(distribution)
            if rng is None:
                self.notes.append(
                    f"DynamicParameter {labels[index]} ({source_name!r}) "
                    "could not be decoded; the schema default was kept"
                )
                continue

            samples = list(rng.samples) or [rng.low, rng.high]
            channels[index] = curve_text(samples)
            kind = distribution.get("kind") if isinstance(distribution, dict) else None
            if kind == "authored":
                self.notes.append(
                    f"DynamicParameter {labels[index]} ({source_name!r}) is "
                    "runtime-authored; its packaged fallback was preserved"
                )
            else:
                self.notes.append(
                    f"DynamicParameter slot {labels[index]} preserves source "
                    f"parameter {source_name!r}; verify the material channel "
                    "convention in the Effect Tool"
                )

            if entry.get("buseemittertime"):
                self.notes.append(
                    f"DynamicParameter {labels[index]} ({source_name!r}) used "
                    "emitter time; the tool evaluates its curve over particle "
                    "relative time"
                )
            if entry.get("bspawntimeonly"):
                self.notes.append(
                    f"DynamicParameter {labels[index]} ({source_name!r}) was "
                    "spawn-time-only; the tool currently evaluates it over life"
                )
            method = str(entry.get("valuemethod") or "").casefold()
            if method and method != "edpv_userset":
                self.notes.append(
                    f"DynamicParameter {labels[index]} ({source_name!r}) used "
                    f"{method}; only its decoded value curve was preserved"
                )
            if entry.get("bscalevelocitybyparamvalue"):
                self.notes.append(
                    f"DynamicParameter {labels[index]} ({source_name!r}) "
                    "scaled velocity in Cascade; that side effect is not "
                    "represented by the current schema"
                )

        if len(dynamic_params) > 4:
            omitted = [
                str(item.get("paramname") or f"slot_{i + 4}")
                if isinstance(item, dict) else f"slot_{i + 4}"
                for i, item in enumerate(dynamic_params[4:])
            ]
            self.notes.append(
                "DynamicParameter contains more than four source channels; "
                f"unrepresentable channels were not authored: {', '.join(omitted)}"
            )

        return (
            f"{head} x={channels[0]} y={channels[1]}"
            f" z={channels[2]} w={channels[3]}"
        )

    def estimate_max(self, rate: float, required: dict) -> int:
        duration = float(required.get("emitterduration") or 1.0)
        # Enough headroom for the spawn rate over one loop, clamped to the
        # runtime safety limit.
        return max(16, min(65536, int(rate * max(duration, 1.0) * 4) + 16))

    def color_module(self, head: str, params: dict) -> str:
        color = read_distribution(params.get("startcolor"))
        alpha = read_distribution(params.get("startalpha"))
        node = params.get("startcolor")
        rgb = (1.0, 1.0, 1.0)
        if isinstance(node, dict) and node.get("kind") == "authored":
            constant = (node.get("values") or {}).get("constant") or {}
            rgb = (
                float(constant.get("x", 1.0)),
                float(constant.get("y", 1.0)),
                float(constant.get("z", 1.0)),
            )
            self.notes.append(
                "startcolor is a runtime parameter "
                f"({(node.get('values') or {}).get('parametername')}); "
                "using its authored fallback"
            )
        elif color is not None:
            triples = vector_triples(color.samples)
            if triples:
                rgb = triples[0]

        a = alpha.low if alpha else 1.0
        value = f"{number(rgb[0])},{number(rgb[1])},{number(rgb[2])},{number(a)}"
        return f"{head} dist=CONSTANT c={value} min={value} max={value}"

    def color_over_life_module(self, head: str, params: dict) -> str:
        color = read_distribution(
            params.get("colorscaleoverlife") or params.get("coloroverlife"))
        alpha = read_distribution(
            params.get("alphascaleoverlife") or params.get("alphaoverlife"))
        if params.get("coloroverlife") is not None:
            self.notes.append(
                "ColorOverLife sets an absolute colour in Cascade but "
                "multiplies here; check emitters that also have Initial Color"
            )
        if color is None and alpha is None:
            return ""

        if color is not None:
            triples = vector_triples(color.samples)
            reds = [t[0] for t in triples] or color.samples
            greens = [t[1] for t in triples] or color.samples
            blues = [t[2] for t in triples] or color.samples
        else:
            reds = greens = blues = [1.0]

        alphas = alpha.samples if alpha is not None else [1.0]
        return (
            f"{head} r={curve_text(reds)} g={curve_text(greens)}"
            f" b={curve_text(blues)} a={curve_text(alphas)}"
        )

    def location_module(self, head: str, klass: str, params: dict) -> str:
        offset = read_distribution(params.get("startlocation"))
        offset_fields = vector_distribution_fields(offset, self.scale)
        if not offset_fields:
            offset_fields = " dist=CONSTANT c=0,0,0 min=0,0,0 max=0,0,0"

        if klass == "particlemodulelocationprimitivesphere":
            radius = read_distribution(params.get("startradius"))
            inner = read_distribution(params.get("startinnerradius"))
            return (
                f"{head}{offset_fields} shape=SPHERE"
                f" radius={number((radius.high if radius else 1.0) * self.scale)}"
                f" inner={number((inner.high if inner else 0.0) * self.scale)}"
                " height=0 surface=0"
            )

        if klass == "particlemodulelocationprimitivecylinder":
            radius = read_distribution(params.get("startradius"))
            height = read_distribution(params.get("startheight"))
            inner = read_distribution(params.get("startinnerradius"))
            return (
                f"{head}{offset_fields} shape=CYLINDER"
                f" radius={number((radius.high if radius else 1.0) * self.scale)}"
                f" inner={number((inner.high if inner else 0.0) * self.scale)}"
                f" height={number((height.high if height else 1.0) * self.scale)}"
                " surface=0"
            )

        rng = read_distribution(params.get("startlocation"))
        fields = vector_distribution_fields(rng, self.scale) or offset_fields
        return f"{head}{fields} shape=BOX radius=0 inner=0 height=0 surface=0"


def select_objects(catalog: Path | None, family: str | None,
                   contains: str | None) -> set[str] | None:
    """Object names for a skill family, taken from the skill grouping CSV/JSON."""
    if family is None and contains is None:
        return None
    if catalog is None:
        raise SystemExit("--family/--contains need --catalog")

    rows = json.loads(catalog.read_text(encoding="utf-8")).get("rows", [])
    selected = set()
    for row in rows:
        name = row.get("object_name", "")
        if family is not None and row.get("family") == family:
            selected.add(name)
        if contains is not None and contains.lower() in name.lower():
            selected.add(name)
    return selected


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--recipe", type=Path, nargs="+", required=True)
    parser.add_argument("--object", default=None,
                        help="single ParticleSystem object_name")
    parser.add_argument("--catalog", type=Path, default=None,
                        help="glaivier_skill_phase_1_3.json style grouping")
    parser.add_argument("--family", default=None,
                        help="convert every particle in this skill family")
    parser.add_argument("--contains", default=None,
                        help="convert every particle whose name contains this")
    parser.add_argument("--asset-id", default=None)
    parser.add_argument("--out", type=Path, default=None,
                        help="single file output")
    parser.add_argument("--out-dir", type=Path, default=None,
                        help="directory output, one .effect per particle")
    parser.add_argument("--material-map", type=Path, default=None,
                        help="material_texture_map.json for texture lookup")
    parser.add_argument("--mesh-root", type=Path, default=None,
                        help="folder holding cooked .wmodel meshes")
    parser.add_argument("--mesh-prefix", default="",
                        help="path the tool uses to reach --mesh-root")
    parser.add_argument("--texture", default="",
                        help="force this texture instead of the lookup")
    parser.add_argument("--scale", type=float, default=DEFAULT_SCALE,
                        help="UE3 centimetre to client metre factor")
    args = parser.parse_args(argv)

    wanted = select_objects(args.catalog, args.family, args.contains)
    if wanted is None and args.object is None:
        raise SystemExit("need --object, or --family/--contains with --catalog")
    if args.out is None and args.out_dir is None:
        raise SystemExit("need --out or --out-dir")

    assets: list[dict] = []
    for recipe in args.recipe:
        document = json.loads(recipe.read_text(encoding="utf-8"))
        for asset in document.get("assets", []):
            name = asset.get("object_name")
            if args.object is not None and name == args.object:
                assets.append(asset)
            elif wanted is not None and name in wanted:
                assets.append(asset)

    if not assets:
        print("no matching particle system in the given recipes",
              file=sys.stderr)
        return 1

    material_map = load_material_map(args.material_map)
    converter = Converter(args.scale, args.texture, True, material_map)
    if args.mesh_root is not None:
        converter.mesh_root = args.mesh_root
        converter.mesh_index = {
            path.stem for path in args.mesh_root.glob("*.wmodel")
        }
        converter.mesh_prefix = args.mesh_prefix or args.mesh_root.as_posix()

    written = with_texture = total_required = 0
    for asset in assets:
        name = asset["object_name"]
        asset_id = (
            args.asset_id if (args.asset_id and len(assets) == 1)
            else name.replace("_", "-")
        )
        text = converter.convert(asset, asset_id)
        target = (
            args.out if args.out is not None
            else args.out_dir / f"{asset_id}.effect"
        )
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(text, encoding="utf-8", newline="\n")
        written += 1
        # Count per emitter: one file can hold both textured and untextured
        # emitters, so a file level check would hide most of the coverage.
        for line in text.splitlines():
            if " type=REQUIRED " in line:
                total_required += 1
                # Schema v5 also has opacitytex/dissolvetex/distortiontex. The
                # leading space keeps those empty optional slots from being
                # mistaken for an empty primary ``tex`` field.
                if ' tex=""' not in line:
                    with_texture += 1

    print(f"wrote {written} effect file(s); "
          f"{with_texture}/{total_required} emitters got a texture")
    if converter.skipped:
        counts: dict[str, int] = {}
        for klass in converter.skipped:
            counts[klass] = counts.get(klass, 0) + 1
        summary = ", ".join(
            f"{k} x{v}" for k, v in sorted(counts.items(), key=lambda i: -i[1])
        )
        print(f"skipped modules the tool cannot represent: {summary}",
              file=sys.stderr)
    if converter.mesh_resolved or converter.mesh_missing:
        print(f"mesh emitters: {converter.mesh_resolved} matched a cooked "
              f".wmodel, {converter.mesh_missing} had no cooked mesh match",
              file=sys.stderr)
    if converter.synthesized_spawn or converter.synthesized_lifetime:
        print(f"invented Spawn for {converter.synthesized_spawn} and Lifetime "
              f"for {converter.synthesized_lifetime} sprite emitter(s); the "
              f"package had none and the simulator needs both (named with *)",
              file=sys.stderr)
    if converter.empty_lod_emitters:
        print(f"disabled {converter.empty_lod_emitters} emitter(s) with no LOD "
              "payload instead of inventing runnable modules",
              file=sys.stderr)
    if converter.unresolved_module_ref_emitters:
        print(
            f"disabled {converter.unresolved_module_ref_emitters} emitter(s) "
            f"with {converter.unresolved_module_ref_occurrences} external or "
            "missing module reference(s); dependency modules were not "
            "materialized into this local-export recipe",
            file=sys.stderr,
        )
    if converter.unresolved_materials:
        unique = sorted(set(converter.unresolved_materials))
        print(f"{len(unique)} material(s) had no texture in the lookup; "
              f"those emitters need Browse Texture in the tool",
              file=sys.stderr)
    for note in dict.fromkeys(converter.notes):
        print(f"note: {note}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
