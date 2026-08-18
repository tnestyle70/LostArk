"""Build the Valtan arena floor collapse contract for LV_LUT_HEARTRB_ED.

The arena floor is four concentric layers of map placements. This tool moves the
six collapsing ring placements into the deploy prop layer, where the existing
world destruction runtime already owns INTACT/BREAKING/DESPAWNED state, and
derives the navigation blocker regions from the real floor geometry instead of
from hand-typed coordinates.

Cell ownership rule: a navgrid cell belongs to the last floor layer that still
supports it. The safe core therefore never appears in a collapse region, and
stage A and stage B are disjoint by construction.

Apply writes five authoring documents in one transaction. Every document is
staged first and the originals are restored if any replacement fails, so a
partial contract can never reach a publisher.
"""

from __future__ import annotations

import argparse
import collections
import io
import math
import os
import re
import shlex
import struct
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO_ROOT / "Tools" / "ModelAssetConverter"))

import cook_wmodel_geometry_contract as contract  # noqa: E402

AREA_ID = "LV_LUT_HEARTRB_ED"
ARENA_CENTRE = (156.03, -122.06)
FLOOR_Y_RANGE = (21.0, 24.5)
UP_FACING_MINIMUM = 0.70
ASSET_PRETRANSFORM = 0.01
CORE_RADIUS = 8.60
ARENA_RADIUS = 16.6
BREAKING_DURATION_MS = 250

MAP_ASSETS = REPO_ROOT / "Data/Maps/Imported" / AREA_ID / f"{AREA_ID}.mapassets"
MAP_PLACEMENTS = REPO_ROOT / "Data/Maps/Authoring" / AREA_ID / f"{AREA_ID}.mapplacements"
DEPLOY_ASSETS = REPO_ROOT / "Data/Maps/Imported" / AREA_ID / f"{AREA_ID}.deployassets"
DEPLOY_PLACEMENTS = REPO_ROOT / "Data/Maps/Authoring" / AREA_ID / f"{AREA_ID}.deployplacements"
WORLD_EVENTS = REPO_ROOT / "Data/Encounters/Valtan/ValtanWorldEvents.json"
NAV_BLOCKERS = REPO_ROOT / "Data/Navigation" / f"{AREA_ID}.navblockers"
NAV_GRID = REPO_ROOT / "Client/Bin/DataFiles/Navigation" / f"{AREA_ID}.navgrid"
RESOURCE_ROOT = REPO_ROOT / "Client/Bin/Resources"

# layer name, collapse order (0 = never collapses)
FLOOR_LAYERS = {
    "MAP_FBC80A02F72E_BG_LUT_WAGLOY_CIRCLEFLOOR01_SM_JJY": ("CORE", 0),
    "MAP_4A6CF4B84315_LV_LUT_HEARTRB_FLOOR01_SM": ("CORE", 0),
    "BG_RAD_VALTAN_FLOOR01A_SM": ("STAGE_B", 1),
    "BG_RAD_VALTAN_FLOOR01B_SM": ("STAGE_B", 1),
    "BG_RAD_VALTAN_FLOOR01_SM": ("STAGE_A", 2),
}

DEPLOY_ASSET_OF_MAP_ASSET = {
    "BG_RAD_VALTAN_FLOOR01_SM": "VALTAN_FLOOR_RAIL",
    "BG_RAD_VALTAN_FLOOR01A_SM": "VALTAN_FLOOR_BRICK_A",
    "BG_RAD_VALTAN_FLOOR01B_SM": "VALTAN_FLOOR_BRICK_B",
}

DEPLOY_ASSET_LABEL = {
    "VALTAN_FLOOR_RAIL": "Valtan Arena Outer Rail Floor",
    "VALTAN_FLOOR_BRICK_A": "Valtan Arena Brick Floor A",
    "VALTAN_FLOOR_BRICK_B": "Valtan Arena Brick Floor B",
}

DEPLOY_ASSET_EVIDENCE = (
    "PROJECT_AUTHORED collapse unit; geometry is the BG_RAD_VALTAN_A intact-arena "
    "overlay export migrated out of the map placement layer; no fractured mesh "
    "because the authored mutation ends at DESPAWNED"
)

STAGE_KEY = {"STAGE_A": "floor84", "STAGE_B": "floor30"}
STAGE_SLUG = {"STAGE_A": "rail", "STAGE_B": "brick"}
STAGE_PATTERN = {
    "STAGE_A": ("VALTAN_ARENA_BREAK_84", "IMPACT"),
    "STAGE_B": ("VALTAN_ARENA_BREAK_33", "LANDING"),
}

# Project-authored deploy identities. The runtime refuses a zero actor or
# definition id, and the 109 rows already reserve the 109xxxxxx range, so the
# floor sectors take their own fixed block instead of reusing DeployData values.
DEPLOY_ACTOR_ID = {
    "7000000000000000001": 84000001,
    "7000000000000000005": 84000002,
    "7000000000000000002": 30000001,
    "7000000000000000003": 30000002,
    "7000000000000000006": 30000003,
    "7000000000000000007": 30000004,
}
DEPLOY_DEFINITION_ID = {
    "7000000000000000001": 84000101,
    "7000000000000000005": 84000102,
    "7000000000000000002": 30000101,
    "7000000000000000003": 30000102,
    "7000000000000000006": 30000103,
    "7000000000000000007": 30000104,
}

EXPECTED_SECTOR_CELLS = {
    "7000000000000000001": 347,
    "7000000000000000005": 325,
    "7000000000000000002": 365,
    "7000000000000000003": 427,
    "7000000000000000006": 360,
    "7000000000000000007": 425,
}


class BuildError(RuntimeError):
    pass


def read_document(path: Path) -> tuple[str, str]:
    """Return (text, newline) so a rewrite keeps the document's line endings."""
    raw = path.read_bytes()
    newline = "\r\n" if b"\r\n" in raw else "\n"
    return raw.decode("utf-8"), newline


def split_row(line: str) -> list[str]:
    return shlex.split(line)


def load_map_assets() -> dict[str, str]:
    text, _ = read_document(MAP_ASSETS)
    assets: dict[str, str] = {}
    for line in text.splitlines()[1:]:
        if not line.strip():
            continue
        tokens = split_row(line)
        if len(tokens) >= 4:
            assets[tokens[0]] = tokens[2]
    return assets


def load_floor_placements() -> list[dict]:
    text, _ = read_document(MAP_PLACEMENTS)
    rows: list[dict] = []
    for line in text.splitlines()[1:]:
        if not line.strip():
            continue
        tokens = split_row(line)
        if len(tokens) < 16 or tokens[4] not in FLOOR_LAYERS:
            continue
        x, y, z = float(tokens[5]), float(tokens[6]), float(tokens[7])
        if math.hypot(x - ARENA_CENTRE[0], z - ARENA_CENTRE[1]) > 22.0:
            continue
        rows.append(
            {
                "placementId": tokens[0],
                "sourcePlacementId": tokens[1],
                "assetId": tokens[4],
                "position": (x, y, z),
                "rotation": (
                    float(tokens[8]),
                    float(tokens[9]),
                    float(tokens[10]),
                    float(tokens[11]),
                ),
                "scale": (float(tokens[12]), float(tokens[13]), float(tokens[14])),
                "line": line,
            }
        )
    return rows


_MESH_CACHE: dict[str, list] = {}


def load_triangles(asset_id: str, asset_paths: dict[str, str]) -> list:
    if asset_id in _MESH_CACHE:
        return _MESH_CACHE[asset_id]
    relative = asset_paths.get(asset_id)
    if relative is None:
        raise BuildError(f"Floor asset is missing from the map catalog: {asset_id}")
    resolved = RESOURCE_ROOT / Path(relative)
    if not resolved.is_file():
        raise BuildError(f"Floor asset payload is missing from Resources: {relative}")
    _header, _sections, submeshes = contract.parse_legacy_wmodel(resolved.read_bytes())
    triangles = []
    for submesh in submeshes:
        vertices, indices = submesh.vertices, submesh.indices
        for base in range(0, len(indices) - 2, 3):
            a = vertices[indices[base]]
            b = vertices[indices[base + 1]]
            c = vertices[indices[base + 2]]
            triangles.append(((a[0], a[1], a[2]), (b[0], b[1], b[2]), (c[0], c[1], c[2])))
    _MESH_CACHE[asset_id] = triangles
    return triangles


def rotation_rows(qx: float, qy: float, qz: float, qw: float):
    length = math.sqrt(qx * qx + qy * qy + qz * qz + qw * qw)
    if length <= 0.0:
        raise BuildError("Placement rotation quaternion is degenerate.")
    qx, qy, qz, qw = qx / length, qy / length, qz / length, qw / length
    return (
        (1 - 2 * (qy * qy + qz * qz), 2 * (qx * qy - qz * qw), 2 * (qx * qz + qy * qw)),
        (2 * (qx * qy + qz * qw), 1 - 2 * (qx * qx + qz * qz), 2 * (qy * qz - qx * qw)),
        (2 * (qx * qz - qy * qw), 2 * (qy * qz + qx * qw), 1 - 2 * (qx * qx + qy * qy)),
    )


class NavGrid:
    def __init__(self, path: Path) -> None:
        data = path.read_bytes()
        if len(data) < 20:
            raise BuildError("Navigation grid header is truncated.")
        self.width, self.height = struct.unpack_from("<II", data, 0)
        self.cell_size, self.origin_x, self.origin_z = struct.unpack_from("<fff", data, 8)
        if len(data) != 20 + self.width * self.height * 5:
            raise BuildError("Navigation grid size does not match its header.")
        self.walkable = data[20 : 20 + self.width * self.height]

    def index(self, cx: int, cz: int) -> int:
        return cz * self.width + cx

    def is_walkable(self, cx: int, cz: int) -> bool:
        if cx < 0 or cz < 0 or cx >= self.width or cz >= self.height:
            return False
        return self.walkable[self.index(cx, cz)] == 1

    def centre(self, cx: int, cz: int) -> tuple[float, float]:
        return (
            self.origin_x + (cx + 0.5) * self.cell_size,
            self.origin_z + (cz + 0.5) * self.cell_size,
        )

    def radius(self, cx: int, cz: int) -> float:
        wx, wz = self.centre(cx, cz)
        return math.hypot(wx - ARENA_CENTRE[0], wz - ARENA_CENTRE[1])


def rasterise_coverage(placements, asset_paths, grid: NavGrid):
    """cell index -> set of placement ids whose up-facing floor covers it."""
    coverage: dict[int, set[str]] = collections.defaultdict(set)
    for placement in placements:
        rows = rotation_rows(*placement["rotation"])
        sx, sy, sz = (value * ASSET_PRETRANSFORM for value in placement["scale"])
        px, py, pz = placement["position"]
        for triangle in load_triangles(placement["assetId"], asset_paths):
            world = []
            for vertex in triangle:
                lx, ly, lz = vertex[0] * sx, vertex[1] * sy, vertex[2] * sz
                world.append(
                    (
                        rows[0][0] * lx + rows[0][1] * ly + rows[0][2] * lz + px,
                        rows[1][0] * lx + rows[1][1] * ly + rows[1][2] * lz + py,
                        rows[2][0] * lx + rows[2][1] * ly + rows[2][2] * lz + pz,
                    )
                )
            a, b, c = world
            if not FLOOR_Y_RANGE[0] <= (a[1] + b[1] + c[1]) / 3.0 <= FLOOR_Y_RANGE[1]:
                continue
            ux, uy, uz = b[0] - a[0], b[1] - a[1], b[2] - a[2]
            vx, vy, vz = c[0] - a[0], c[1] - a[1], c[2] - a[2]
            nx = uy * vz - uz * vy
            ny = uz * vx - ux * vz
            nz = ux * vy - uy * vx
            norm = math.sqrt(nx * nx + ny * ny + nz * nz)
            if norm <= 1e-12 or abs(ny) / norm < UP_FACING_MINIMUM:
                continue
            ax, az = a[0], a[2]
            bx, bz = b[0], b[2]
            cx, cz = c[0], c[2]
            denominator = (bz - cz) * (ax - cx) + (cx - bx) * (az - cz)
            if abs(denominator) < 1e-12:
                continue
            i0 = max(0, int((min(ax, bx, cx) - grid.origin_x) / grid.cell_size))
            i1 = min(grid.width - 1, int((max(ax, bx, cx) - grid.origin_x) / grid.cell_size))
            j0 = max(0, int((min(az, bz, cz) - grid.origin_z) / grid.cell_size))
            j1 = min(grid.height - 1, int((max(az, bz, cz) - grid.origin_z) / grid.cell_size))
            for j in range(j0, j1 + 1):
                for i in range(i0, i1 + 1):
                    wx, wz = grid.centre(i, j)
                    l1 = ((bz - cz) * (wx - cx) + (cx - bx) * (wz - cz)) / denominator
                    l2 = ((cz - az) * (wx - cx) + (ax - cx) * (wz - cz)) / denominator
                    if l1 < -1e-9 or l2 < -1e-9 or 1.0 - l1 - l2 < -1e-9:
                        continue
                    coverage[grid.index(i, j)].add(placement["placementId"])
    return coverage


def assign_owners(coverage, placements, grid: NavGrid):
    layer_of = {row["placementId"]: FLOOR_LAYERS[row["assetId"]] for row in placements}
    owner: dict[int, str] = {}
    for cell, placement_ids in coverage.items():
        best = min(placement_ids, key=lambda pid: (layer_of[pid][1], pid))
        if layer_of[best][1] == 0 or grid.walkable[cell] != 1:
            continue
        owner[cell] = best
    return owner, layer_of


def close_seams(owner, grid: NavGrid) -> int:
    """Absorb walkable cells that would float once their neighbours collapse."""
    absorbed = 0
    while True:
        changed = 0
        for cell in range(grid.width * grid.height):
            if grid.walkable[cell] != 1 or cell in owner:
                continue
            cx, cz = cell % grid.width, cell // grid.width
            radius = grid.radius(cx, cz)
            if radius < CORE_RADIUS or radius > ARENA_RADIUS:
                continue
            neighbours = []
            escapes = False
            for dx, dz in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                nx, nz = cx + dx, cz + dz
                if not grid.is_walkable(nx, nz):
                    continue
                neighbour = grid.index(nx, nz)
                if neighbour in owner:
                    neighbours.append(owner[neighbour])
                    continue
                nradius = grid.radius(nx, nz)
                if nradius < CORE_RADIUS or nradius > ARENA_RADIUS:
                    escapes = True
            if escapes or not neighbours:
                continue
            owner[cell] = collections.Counter(neighbours).most_common(1)[0][0]
            changed += 1
        absorbed += changed
        if changed == 0:
            return absorbed


def stable_ids(placement_id: str, layer_name: str) -> dict[str, str]:
    stage = STAGE_KEY[layer_name]
    slug = STAGE_SLUG[layer_name]
    return {
        "groupId": f"destroyable.group.valtan.{stage}.{slug}.{placement_id}",
        "mutationId": f"mutation.valtan.{stage}.{slug}.{placement_id}.collapse",
        "bindingId": f"binding.valtan.{stage}.{slug}.{placement_id}",
        "navRegionId": f"navregion.valtan.{stage}.{slug}.{placement_id}",
        "conditionId": f"condition.valtan.{stage}.{slug}.{placement_id}.collapsed",
    }


def sector_order(owner, layer_of) -> list[str]:
    return sorted({owner[cell] for cell in owner}, key=lambda pid: (layer_of[pid][1], pid))


def build_deploy_asset_rows(asset_paths: dict[str, str]) -> list[str]:
    rows = []
    for map_asset_id in sorted(DEPLOY_ASSET_OF_MAP_ASSET):
        deploy_asset_id = DEPLOY_ASSET_OF_MAP_ASSET[map_asset_id]
        relative = asset_paths[map_asset_id]
        rows.append(
            f'"{deploy_asset_id}" STATIC "BG_RAD_VALTAN_A.{map_asset_id}" '
            f'"{relative}" "Prototype_Component_Model_{deploy_asset_id}_INTACT" '
            f'"" "" "{DEPLOY_ASSET_EVIDENCE}"'
        )
    return rows


def format_number(value: float) -> str:
    text = repr(float(value))
    return text[:-2] if text.endswith(".0") else text


def build_deploy_placement_rows(placements, layer_of) -> list[str]:
    rows = []
    for placement in sorted(placements, key=lambda row: row["placementId"]):
        placement_id = placement["placementId"]
        if layer_of[placement_id][1] == 0:
            continue
        scale = placement["scale"]
        if max(abs(scale[0] - scale[1]), abs(scale[1] - scale[2])) > 1e-6:
            raise BuildError(f"Floor placement is not uniformly scaled: {placement_id}")
        if placement_id not in DEPLOY_ACTOR_ID:
            raise BuildError(f"Floor placement has no authored deploy identity: {placement_id}")
        px, py, pz = placement["position"]
        qx, qy, qz, qw = placement["rotation"]
        rows.append(
            f"{placement_id} {DEPLOY_ACTOR_ID[placement_id]} "
            f"{DEPLOY_DEFINITION_ID[placement_id]} "
            f'"{placement["sourcePlacementId"]}" '
            f'"{DEPLOY_ASSET_OF_MAP_ASSET[placement["assetId"]]}" '
            f"{format_number(px)} {format_number(py)} {format_number(pz)} "
            f"{format_number(qx)} {format_number(qy)} {format_number(qz)} "
            f"{format_number(qw)} {format_number(scale[0])} 1 0 0"
        )
    return rows


def build_nav_region_blocks(owner, layer_of, grid: NavGrid) -> list[list[str]]:
    by_placement: dict[str, list[int]] = collections.defaultdict(list)
    for cell, placement_id in owner.items():
        by_placement[placement_id].append(cell)
    blocks = []
    for placement_id in sector_order(owner, layer_of):
        ids = stable_ids(placement_id, layer_of[placement_id][0])
        cells = sorted(
            (cell % grid.width, cell // grid.width) for cell in by_placement[placement_id]
        )
        lines = [f'REGION "{ids["navRegionId"]}" "{ids["conditionId"]}" 1 {len(cells)}']
        lines.extend(f"{cx} {cz}" for cx, cz in cells)
        blocks.append(lines)
    return blocks


def json_entries(owner, layer_of):
    groups, mutations, bindings = [], [], []
    for placement_id in sector_order(owner, layer_of):
        layer_name = layer_of[placement_id][0]
        ids = stable_ids(placement_id, layer_name)
        pattern_id, stage_id = STAGE_PATTERN[layer_name]
        groups.append(
            [
                ('"groupId":  "%s",' % ids["groupId"]),
                '"memberPlacementIds":  [',
                '@@LIST@@"%s"' % placement_id,
                "@@ENDLIST@@],",
                '"navigationRegionIds":  [',
                '@@LIST@@"%s"' % ids["navRegionId"],
                "@@ENDLIST@@],",
                '"navPolarity":  "BLOCK_WHILE_FRACTURED",',
                '"initialState":  "INTACT"',
            ]
        )
        mutations.append(
            [
                '"mutationId":  "%s",' % ids["mutationId"],
                '"groupId":  "%s",' % ids["groupId"],
                '"targetState":  "DESPAWNED",',
                '"breakingDurationMs":  %d' % BREAKING_DURATION_MS,
            ]
        )
        bindings.append(
            [
                '"bindingId":  "%s",' % ids["bindingId"],
                '"mutationId":  "%s",' % ids["mutationId"],
                '"patternId":  "%s",' % pattern_id,
                '"stageId":  "%s",' % stage_id,
                '"triggerKind":  "STAGE_ENTER",',
                '"offsetMs":  0,',
                '"receiverCollisionId":  "",',
                '"enabled":  true',
            ]
        )
    return groups, mutations, bindings


def insert_json_entries(text: str, newline: str, section: str, entries) -> str:
    """Append objects to one top-level array while preserving the existing style."""
    lines = text.split(newline)
    start = None
    for index, line in enumerate(lines):
        if line.strip().startswith('"%s":' % section) and line.rstrip().endswith("["):
            start = index
            break
    if start is None:
        raise BuildError(f"World events document has no '{section}' array.")
    # Nested arrays such as memberPlacementIds close with the same token, so the
    # end of this array is found by bracket depth rather than by line shape.
    end = None
    depth = 0
    in_string = False
    escaped = False
    for index in range(start, len(lines)):
        for character in lines[index]:
            if in_string:
                if escaped:
                    escaped = False
                elif character == "\\":
                    escaped = True
                elif character == '"':
                    in_string = False
                continue
            if character == '"':
                in_string = True
            elif character in "[{":
                depth += 1
            elif character in "]}":
                depth -= 1
                if depth == 0 and index > start:
                    end = index
                    break
        if end is not None:
            break
    if end is None:
        raise BuildError(f"World events '{section}' array is not closed.")

    brace_indent = len(lines[start + 1]) - len(lines[start + 1].lstrip())
    field_indent = len(lines[start + 2]) - len(lines[start + 2].lstrip())
    list_indent = field_indent + 4
    if lines[end - 1].strip() != "}":
        raise BuildError(f"World events '{section}' array does not end with an object.")
    lines[end - 1] = lines[end - 1] + ","

    rendered: list[str] = []
    for position, entry in enumerate(entries):
        rendered.append(" " * brace_indent + "{")
        for field in entry:
            if field.startswith("@@LIST@@"):
                rendered.append(" " * (list_indent + 22) + field[len("@@LIST@@") :])
            elif field.startswith("@@ENDLIST@@"):
                rendered.append(" " * (list_indent + 18) + field[len("@@ENDLIST@@") :])
            else:
                rendered.append(" " * field_indent + field)
        rendered.append(" " * brace_indent + "}" + ("," if position + 1 < len(entries) else ""))
    return newline.join(lines[:end] + rendered + lines[end:])


def replace_header_count(text: str, newline: str, token_index: int, value: int) -> str:
    lines = text.split(newline)
    tokens = lines[0].split(" ")
    tokens[token_index] = str(value)
    lines[0] = " ".join(tokens)
    return newline.join(lines)


def strip_trailing_blank(lines: list[str]) -> tuple[list[str], bool]:
    trailing = bool(lines) and lines[-1] == ""
    return (lines[:-1] if trailing else lines), trailing


def build_documents(owner, layer_of, placements, asset_paths, grid: NavGrid):
    """Return {path: new text} for every document this contract touches."""
    documents: dict[Path, str] = {}
    collapse_ids = {
        row["placementId"] for row in placements if layer_of[row["placementId"]][1] != 0
    }

    text, newline = read_document(DEPLOY_ASSETS)
    lines, trailing = strip_trailing_blank(text.split(newline))
    count = int(split_row(lines[0])[3])
    new_rows = build_deploy_asset_rows(asset_paths)
    for row in new_rows:
        if any(split_row(existing)[0] == split_row(row)[0] for existing in lines[1:]):
            raise BuildError(f"Deploy asset already exists: {split_row(row)[0]}")
    lines = lines[:1] + lines[1:] + new_rows
    lines[0] = f'LOSTARK_DEPLOY_PROP_CATALOG 1 "{AREA_ID}" {count + len(new_rows)}'
    documents[DEPLOY_ASSETS] = newline.join(lines + ([""] if trailing else []))

    text, newline = read_document(DEPLOY_PLACEMENTS)
    lines, trailing = strip_trailing_blank(text.split(newline))
    count = int(split_row(lines[0])[3])
    existing_ids = {split_row(line)[0] for line in lines[1:] if line.strip()}
    new_rows = build_deploy_placement_rows(placements, layer_of)
    for row in new_rows:
        if split_row(row)[0] in existing_ids:
            raise BuildError(f"Deploy placement id already exists: {split_row(row)[0]}")
    lines = lines + new_rows
    lines[0] = f'LOSTARK_DEPLOY_PROP_PLACEMENTS 1 "{AREA_ID}" {count + len(new_rows)}'
    documents[DEPLOY_PLACEMENTS] = newline.join(lines + ([""] if trailing else []))

    text, newline = read_document(MAP_PLACEMENTS)
    lines, trailing = strip_trailing_blank(text.split(newline))
    count = int(split_row(lines[0])[3])
    kept = [lines[0]]
    removed = 0
    for line in lines[1:]:
        if line.strip() and split_row(line)[0] in collapse_ids:
            removed += 1
            continue
        kept.append(line)
    if removed != len(collapse_ids):
        raise BuildError(
            f"Expected to remove {len(collapse_ids)} map placements, removed {removed}."
        )
    kept[0] = f'LOSTARK_MAP_PLACEMENTS 2 "{AREA_ID}" {count - removed}'
    documents[MAP_PLACEMENTS] = newline.join(kept + ([""] if trailing else []))

    text, newline = read_document(WORLD_EVENTS)
    groups, mutations, bindings = json_entries(owner, layer_of)
    text = insert_json_entries(text, newline, "groups", groups)
    text = insert_json_entries(text, newline, "mutations", mutations)
    text = insert_json_entries(text, newline, "bindings", bindings)
    documents[WORLD_EVENTS] = text

    text, newline = read_document(NAV_BLOCKERS)
    lines, trailing = strip_trailing_blank(text.split(newline))
    header = split_row(lines[0])
    region_count = int(header[8])
    blocks = build_nav_region_blocks(owner, layer_of, grid)
    for block in blocks:
        region_id = split_row(block[0])[1]
        if any(line.startswith("REGION ") and split_row(line)[1] == region_id for line in lines):
            raise BuildError(f"Navigation region already exists: {region_id}")
        lines.extend(block)
    header[8] = str(region_count + len(blocks))
    lines[0] = " ".join(
        [header[0], header[1], f'"{header[2]}"'] + header[3:]
    )
    documents[NAV_BLOCKERS] = newline.join(lines + ([""] if trailing else []))

    return documents


def commit(documents: dict[Path, str]) -> None:
    """Stage every document, then swap them with a full restore on any failure."""
    originals = {path: path.read_bytes() for path in documents}
    staged: dict[Path, Path] = {}
    try:
        for path, text in documents.items():
            staging = path.with_suffix(path.suffix + ".staging")
            staging.write_bytes(text.encode("utf-8"))
            staged[path] = staging
        replaced: list[Path] = []
        try:
            for path, staging in staged.items():
                os.replace(staging, path)
                replaced.append(path)
        except Exception:
            for path in replaced:
                path.write_bytes(originals[path])
            raise
    finally:
        for staging in staged.values():
            if staging.exists():
                staging.unlink()


def report(owner, layer_of, absorbed: int) -> None:
    by_placement = collections.Counter(owner.values())
    print(f"seam cells absorbed: {absorbed}")
    print(f"{'runtimePlacementId':<22}{'stage':<10}{'cells':>7}")
    total = 0
    for placement_id in sector_order(owner, layer_of):
        count = by_placement[placement_id]
        total += count
        print(f"{placement_id:<22}{layer_of[placement_id][0]:<10}{count:>7}")
        expected = EXPECTED_SECTOR_CELLS.get(placement_id)
        if expected is None:
            raise BuildError(f"Sector {placement_id} is not an approved collapse sector.")
        if expected != count:
            raise BuildError(
                f"Sector cell count changed for {placement_id}: expected {expected}, "
                f"measured {count}. Re-approve the sector boundary before publishing."
            )
    print(f"{'TOTAL':<22}{'':<10}{total:>7}")
    if len(by_placement) != len(EXPECTED_SECTOR_CELLS):
        raise BuildError(
            f"Expected {len(EXPECTED_SECTOR_CELLS)} collapse sectors, found {len(by_placement)}."
        )
    stage_cells = collections.defaultdict(set)
    for cell, placement_id in owner.items():
        stage_cells[layer_of[placement_id][0]].add(cell)
    overlap = stage_cells["STAGE_A"] & stage_cells["STAGE_B"]
    if overlap:
        raise BuildError(f"Floor stages share {len(overlap)} navigation cells.")
    print(
        f"stage A={len(stage_cells['STAGE_A'])} "
        f"stage B={len(stage_cells['STAGE_B'])} overlap=0"
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--mode", choices=("Validate", "Apply"), default="Validate")
    arguments = parser.parse_args()

    asset_paths = load_map_assets()
    placements = load_floor_placements()
    grid = NavGrid(NAV_GRID)

    coverage = rasterise_coverage(placements, asset_paths, grid)
    owner, layer_of = assign_owners(coverage, placements, grid)
    absorbed = close_seams(owner, grid)
    for cell in owner:
        if grid.walkable[cell] != 1:
            raise BuildError("A collapse region claimed a base non-walkable cell.")

    report(owner, layer_of, absorbed)
    documents = build_documents(owner, layer_of, placements, asset_paths, grid)
    print(
        "prepared %d documents: %s"
        % (len(documents), ", ".join(path.name for path in documents))
    )

    if arguments.mode == "Validate":
        print("Validate only: no file was written.")
        return 0

    commit(documents)
    print("Apply succeeded: five authoring documents were replaced in one transaction.")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except BuildError as error:
        print(f"build_valtan_floor_collapse: {error}", file=sys.stderr)
        sys.exit(1)
