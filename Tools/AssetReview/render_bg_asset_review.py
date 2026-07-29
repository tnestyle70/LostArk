from __future__ import annotations

import argparse
import csv
import html
import json
import math
import sys
import time
from pathlib import Path

import bpy
from mathutils import Vector


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Render a resumable Lost Ark BG glTF review gallery in Blender."
    )
    parser.add_argument("--inventory", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument(
        "--prefix",
        default="bg_rad_",
        help="Object-name prefix to include; use '*' to include every row.",
    )
    parser.add_argument("--size", type=int, default=256)
    parser.add_argument("--limit", type=int, default=0)
    parser.add_argument("--start", type=int, default=0)
    parser.add_argument("--force", action="store_true")
    return parser.parse_args(sys.argv[sys.argv.index("--") + 1 :])


def clear_scene() -> None:
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for material in list(bpy.data.materials):
        bpy.data.materials.remove(material)
    for mesh in list(bpy.data.meshes):
        if 0 == mesh.users:
            bpy.data.meshes.remove(mesh)


def mesh_bounds(meshes: list[bpy.types.Object]) -> tuple[Vector, Vector]:
    points = [
        obj.matrix_world @ Vector(corner)
        for obj in meshes
        for corner in obj.bound_box
    ]
    if not points:
        raise RuntimeError("imported asset contains no mesh bounds")
    low = Vector(tuple(min(point[axis] for point in points) for axis in range(3)))
    high = Vector(tuple(max(point[axis] for point in points) for axis in range(3)))
    return low, high


def configure_scene(meshes: list[bpy.types.Object], low: Vector, high: Vector, size: int) -> None:
    material = bpy.data.materials.new("ReviewGray")
    material.diffuse_color = (0.34, 0.38, 0.43, 1.0)
    material.roughness = 0.72
    for obj in meshes:
        obj.data.materials.clear()
        obj.data.materials.append(material)

    center = (low + high) * 0.5
    extent = high - low
    radius = max(extent.length * 0.5, 0.01)

    world = bpy.context.scene.world or bpy.data.worlds.new("ReviewWorld")
    bpy.context.scene.world = world
    world.color = (0.012, 0.018, 0.028)

    bpy.ops.object.light_add(
        type="AREA", location=center + Vector((radius, -radius, radius * 1.7))
    )
    key = bpy.context.object
    key.data.energy = 850.0
    key.data.shape = "DISK"
    key.data.size = radius * 1.6

    bpy.ops.object.light_add(
        type="AREA", location=center + Vector((-radius, radius * 0.4, radius * 0.7))
    )
    fill = bpy.context.object
    fill.data.energy = 500.0
    fill.data.size = radius * 1.2

    bpy.ops.object.camera_add()
    camera = bpy.context.object
    direction = Vector((1.35, -1.75, 1.1)).normalized()
    camera.location = center + direction * radius * 3.0
    camera.rotation_euler = (center - camera.location).to_track_quat("-Z", "Y").to_euler()
    camera.data.type = "ORTHO"
    camera.data.ortho_scale = max(extent.x, extent.y, extent.z, 0.01) * 1.35
    bpy.context.scene.camera = camera

    scene = bpy.context.scene
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.resolution_x = size
    scene.render.resolution_y = size
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.image_settings.color_mode = "RGB"
    scene.render.film_transparent = False
    scene.render.use_file_extension = True
    scene.render.image_settings.color_depth = "8"
    scene.render.image_settings.compression = 55
    scene.render.resolution_percentage = 100
    scene.render.use_file_extension = True
    scene.render.use_overwrite = True
    scene.render.use_placeholder = False
    scene.render.engine = "BLENDER_EEVEE"
    scene.view_settings.look = "AgX - Medium High Contrast"


def render_row(row: dict[str, str], output: Path, size: int) -> dict[str, object]:
    clear_scene()
    source = Path(row["sourceGltf"])
    if not source.is_file():
        raise FileNotFoundError(source)
    bpy.ops.import_scene.gltf(filepath=str(source))
    meshes = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    low, high = mesh_bounds(meshes)
    configure_scene(meshes, low, high, size)
    thumbnail = output / "thumbs" / f"{row['assetId']}.png"
    bpy.context.scene.render.filepath = str(thumbnail)
    bpy.ops.render.render(write_still=True)
    return {
        "assetId": row["assetId"],
        "logicalPackage": row["logicalPackage"],
        "objectName": row["objectName"],
        "category": row["category"],
        "dimensions": json.loads(row["dimensions"]),
        "sourceGltf": str(source),
        "thumbnail": f"thumbs/{thumbnail.name}",
        "status": "rendered",
    }


def write_index(output: Path, rows: list[dict[str, object]], total: int) -> None:
    manifest = {
        "schemaVersion": 1,
        "scope": "BG_RAD",
        "total": total,
        "rendered": sum(row.get("status") == "rendered" for row in rows),
        "failed": sum(row.get("status") == "failed" for row in rows),
        "assets": rows,
    }
    (output / "review_manifest.json").write_text(
        json.dumps(manifest, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )
    cards = []
    for row in rows:
        search = " ".join(
            str(row.get(key, ""))
            for key in ("assetId", "logicalPackage", "objectName", "category")
        ).casefold()
        thumbnail = html.escape(str(row.get("thumbnail", "")))
        label = html.escape(str(row.get("objectName", row.get("assetId", ""))))
        package = html.escape(str(row.get("logicalPackage", "")))
        asset_id = html.escape(str(row.get("assetId", "")))
        cards.append(
            f'<article class="card" data-search="{html.escape(search)}" data-id="{asset_id}">'
            f'<img loading="lazy" src="{thumbnail}" alt="{label}">'
            f'<label><input type="checkbox" class="pick"> {label}</label>'
            f'<small>{package}</small><code>{asset_id}</code></article>'
        )
    document = """<!doctype html>
<meta charset="utf-8"><title>Lost Ark BG_RAD visual review</title>
<style>
body{margin:0;background:#10151d;color:#e7edf5;font:14px system-ui}header{position:sticky;top:0;z-index:2;background:#18202b;padding:12px;display:flex;gap:8px;align-items:center}input[type=search]{flex:1;padding:9px}.grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(210px,1fr));gap:10px;padding:10px}.card{background:#1b2532;padding:8px;display:flex;flex-direction:column;gap:5px}.card img{width:100%;aspect-ratio:1;object-fit:cover;background:#0b0f15}.card small,.card code{overflow-wrap:anywhere;color:#9fb0c2}.card:has(.pick:checked){outline:3px solid #5ed0ff}.hidden{display:none}</style>
<header><b>BG_RAD asset review</b><input id="q" type="search" placeholder="name / package / category / asset id"><span id="count"></span><button id="export">Export picks</button></header>
<main class="grid">__CARDS__</main>
<script>
const cards=[...document.querySelectorAll('.card')],q=document.querySelector('#q'),count=document.querySelector('#count');
const saved=new Set(JSON.parse(localStorage.getItem('bgRadPicks')||'[]'));for(const c of cards)c.querySelector('.pick').checked=saved.has(c.dataset.id);
function update(){const s=q.value.toLowerCase();let n=0;for(const c of cards){const show=c.dataset.search.includes(s);c.classList.toggle('hidden',!show);n+=show}count.textContent=`${n} / ${cards.length}`}
q.oninput=update;document.onchange=()=>{const ids=cards.filter(c=>c.querySelector('.pick').checked).map(c=>c.dataset.id);localStorage.setItem('bgRadPicks',JSON.stringify(ids))};
document.querySelector('#export').onclick=()=>{const ids=cards.filter(c=>c.querySelector('.pick').checked).map(c=>c.dataset.id);const a=document.createElement('a');a.href=URL.createObjectURL(new Blob([JSON.stringify({schemaVersion:1,assetIds:ids},null,2)],{type:'application/json'}));a.download='bg_rad_selected_assets.json';a.click()};update();
</script>""".replace("__CARDS__", "\n".join(cards))
    (output / "index.html").write_text(document, encoding="utf-8")


def main() -> None:
    args = parse_args()
    if not args.inventory.is_file():
        raise FileNotFoundError(args.inventory)
    args.output.mkdir(parents=True, exist_ok=True)
    (args.output / "thumbs").mkdir(parents=True, exist_ok=True)
    with args.inventory.open("r", encoding="utf-8-sig", newline="") as handle:
        candidates = [
            row
            for row in csv.DictReader(handle)
            if args.prefix == "*"
            or row["objectName"].casefold().startswith(args.prefix.casefold())
        ]
    candidates.sort(key=lambda row: (row["logicalPackage"].casefold(), row["objectName"].casefold()))
    total = len(candidates)
    selected = candidates[args.start :]
    if 0 < args.limit:
        selected = selected[: args.limit]

    manifest_path = args.output / "review_manifest.json"
    prior: dict[str, dict[str, object]] = {}
    if manifest_path.is_file():
        document = json.loads(manifest_path.read_text(encoding="utf-8"))
        prior = {str(row["assetId"]): row for row in document.get("assets", [])}

    started = time.time()
    for index, row in enumerate(selected, 1):
        asset_id = row["assetId"]
        thumbnail = args.output / "thumbs" / f"{asset_id}.png"
        if not args.force and thumbnail.is_file() and thumbnail.stat().st_size > 256:
            prior[asset_id] = {
                "assetId": asset_id,
                "logicalPackage": row["logicalPackage"],
                "objectName": row["objectName"],
                "category": row["category"],
                "dimensions": json.loads(row["dimensions"]),
                "sourceGltf": row["sourceGltf"],
                "thumbnail": f"thumbs/{thumbnail.name}",
                "status": "rendered",
            }
        else:
            try:
                prior[asset_id] = render_row(row, args.output, args.size)
            except Exception as exception:
                prior[asset_id] = {
                    "assetId": asset_id,
                    "logicalPackage": row["logicalPackage"],
                    "objectName": row["objectName"],
                    "category": row["category"],
                    "sourceGltf": row["sourceGltf"],
                    "thumbnail": "",
                    "status": "failed",
                    "error": str(exception),
                }
        if 0 == index % 20 or index == len(selected):
            rows = sorted(prior.values(), key=lambda item: (str(item.get("logicalPackage", "")), str(item.get("objectName", ""))))
            write_index(args.output, rows, total)
            elapsed = max(time.time() - started, 0.001)
            print(
                f"review {index}/{len(selected)} total={total} rate={index / elapsed:.2f}/s",
                flush=True,
            )


if __name__ == "__main__":
    main()
