"""Render deterministic skeleton-pose contact sheets from a runtime WModel.

This is a read-only diagnostic companion to ``render_action_preview.py``.  It
samples the exact WANM keys consumed by CModel, draws the major body hierarchy
from two projections, and writes PNG contact sheets without launching Client or
Animation Tool.  The output is evidence for candidate comparison only; the
user still owns the final visual-fidelity decision in Animation Tool.
"""

from __future__ import annotations

import argparse
import math
from pathlib import Path
import re
import runpy
from typing import Any

from PIL import Image, ImageDraw, ImageFont


SAMPLE_NORMALIZED_TIMES = (0.0, 0.16, 0.33, 0.5, 0.67, 0.84, 1.0)

MAJOR_CHAINS = (
    ("bip001-pelvis", "bip001-spine", "bip001-spine1", "bip001-spine2", "bip001-neck", "bip001-head"),
    ("bip001-spine2", "bip001-l-clavicle", "bip001-l-upperarm", "bip001-l-forearm", "bip001-l-hand"),
    ("bip001-spine2", "bip001-r-clavicle", "bip001-r-upperarm", "bip001-r-forearm", "bip001-r-hand", "b_wp_r_01"),
    ("bip001-pelvis", "bip001-l-thigh", "bip001-l-calf", "bip001-l-foot", "bip001-l-toe0"),
    ("bip001-pelvis", "bip001-r-thigh", "bip001-r-calf", "bip001-r-foot", "bip001-r-toe0"),
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--wmodel", required=True)
    parser.add_argument("--output-dir", required=True)
    parser.add_argument(
        "--reader",
        default="Tools/ModelAssetConverter/verify_dimensionmaster_summon_bind_pose.py",
        help="Repository WModel reader containing read_wmodel/sample helpers",
    )
    parser.add_argument("--name-regex", default=".*")
    parser.add_argument("--clips-per-sheet", type=int, default=8)
    parser.add_argument("--overwrite", action="store_true")
    return parser.parse_args()


def require(condition: bool, message: str) -> None:
    if not condition:
        raise RuntimeError(message)


def load_reader(path: Path) -> dict[str, Any]:
    require(path.is_file(), f"WModel reader does not exist: {path}")
    return runpy.run_path(str(path), run_name="wmodel_catalog_reader")


def sample_bones(module: dict[str, Any], model: Any, animation: Any, time_ticks: float) -> dict[str, tuple[float, float, float]]:
    local = [list(bone.transform) for bone in model.skeleton_bones]
    for channel in animation.channels:
        local[channel.bone_index] = module["affine_matrix"](
            module["sample_vector"](channel.scale_keys, time_ticks, (1.0, 1.0, 1.0)),
            module["sample_quaternion"](channel.rotation_keys, time_ticks),
            module["sample_vector"](channel.position_keys, time_ticks, (0.0, 0.0, 0.0)),
        )
    combined = module["combined_transforms"](model.skeleton_bones, local)
    return {
        bone.name: (matrix[12], matrix[13], matrix[14])
        for bone, matrix in zip(model.skeleton_bones, combined)
    }


def centered_pose(pose: dict[str, tuple[float, float, float]]) -> tuple[dict[str, tuple[float, float, float]], tuple[float, float, float]]:
    anchor = pose.get("bip001-pelvis") or pose.get("b_root")
    require(anchor is not None, "Valtan skeleton has no pelvis/root anchor")
    return (
        {
            name: (point[0] - anchor[0], point[1] - anchor[1], point[2] - anchor[2])
            for name, point in pose.items()
        },
        anchor,
    )


def projected_extent(poses: list[dict[str, tuple[float, float, float]]]) -> float:
    names = {name for chain in MAJOR_CHAINS for name in chain}
    maximum = 0.0
    for pose in poses:
        for name in names:
            if name not in pose:
                continue
            x, y, z = pose[name]
            maximum = max(maximum, abs(x), abs(y), abs(z))
    return max(maximum, 1.0)


def map_point(point: tuple[float, float, float], view: str, center: tuple[float, float], scale: float) -> tuple[float, float]:
    x, y, z = point
    if view == "front":
        horizontal, vertical = x, z
    else:
        horizontal, vertical = x, y
    return center[0] + horizontal * scale, center[1] - vertical * scale


def draw_pose(draw: ImageDraw.ImageDraw, pose: dict[str, tuple[float, float, float]], center: tuple[float, float], scale: float, view: str) -> None:
    for chain in MAJOR_CHAINS:
        for first, second in zip(chain, chain[1:]):
            if first not in pose or second not in pose:
                continue
            color = (255, 192, 72) if second == "b_wp_r_01" else (120, 220, 255)
            draw.line(
                (map_point(pose[first], view, center, scale), map_point(pose[second], view, center, scale)),
                fill=color,
                width=3,
            )
    joint_names = {name for chain in MAJOR_CHAINS for name in chain}
    for name in joint_names:
        if name not in pose:
            continue
        px, py = map_point(pose[name], view, center, scale)
        radius = 4 if name.endswith(("hand", "foot")) else 3
        draw.ellipse((px - radius, py - radius, px + radius, py + radius), fill=(238, 246, 255))


def default_font() -> ImageFont.ImageFont:
    return ImageFont.load_default()


def render_animation(animation: Any, samples: list[dict[str, tuple[float, float, float]]], roots: list[tuple[float, float, float]]) -> Image.Image:
    width = 1540
    height = 390
    image = Image.new("RGB", (width, height), (15, 19, 27))
    draw = ImageDraw.Draw(image)
    font = default_font()
    draw.text((16, 12), animation.name, fill=(255, 255, 255), font=font)
    draw.text(
        (16, 30),
        f"duration={animation.duration_ticks:.3f} ticks  rate={animation.ticks_per_second:.3f}  seconds={animation.duration_ticks / animation.ticks_per_second:.3f}",
        fill=(165, 184, 206),
        font=font,
    )
    extent = projected_extent(samples)
    cell_width = width / len(samples)
    scale = min((cell_width * 0.39) / extent, 125.0 / extent)
    for index, (normalized, pose, root) in enumerate(zip(SAMPLE_NORMALIZED_TIMES, samples, roots)):
        left = index * cell_width
        draw.line((left, 54, left, height - 8), fill=(45, 54, 69), width=1)
        center_x = left + cell_width * 0.5
        draw_pose(draw, pose, (center_x, 165), scale, "front")
        draw_pose(draw, pose, (center_x, 310), scale, "top")
        draw.text((left + 7, 60), f"t={normalized:.2f}", fill=(210, 220, 235), font=font)
        draw.text((left + 7, 76), f"root=({root[0]:.1f},{root[1]:.1f},{root[2]:.1f})", fill=(126, 145, 169), font=font)
        draw.text((left + 7, 200), "front X/Z", fill=(112, 132, 157), font=font)
        draw.text((left + 7, 345), "top X/Y", fill=(112, 132, 157), font=font)
    return image


def safe_name(value: str) -> str:
    return re.sub(r"[^A-Za-z0-9._-]+", "_", value).strip("._") or "clip"


def compose_sheet(rows: list[tuple[str, Image.Image]], sheet_index: int) -> Image.Image:
    require(rows, "Cannot compose an empty sheet")
    margin = 16
    title_height = 34
    width = rows[0][1].width + margin * 2
    height = title_height + margin + sum(image.height + margin for _, image in rows)
    sheet = Image.new("RGB", (width, height), (8, 11, 16))
    draw = ImageDraw.Draw(sheet)
    draw.text((margin, 10), f"Valtan runtime skeleton catalog sheet {sheet_index:02d}", fill=(255, 255, 255), font=default_font())
    y = title_height + margin
    for _, image in rows:
        sheet.paste(image, (margin, y))
        y += image.height + margin
    return sheet


def main() -> None:
    args = parse_args()
    wmodel = Path(args.wmodel).resolve()
    reader = Path(args.reader).resolve()
    output_dir = Path(args.output_dir).resolve()
    require(wmodel.is_file(), f"WModel does not exist: {wmodel}")
    require(args.clips_per_sheet > 0, "--clips-per-sheet must be positive")
    output_dir.mkdir(parents=True, exist_ok=True)
    module = load_reader(reader)
    model = module["read_wmodel"](wmodel)
    matcher = re.compile(args.name_regex)
    animations = [animation for animation in model.animations if matcher.search(animation.name)]
    require(animations, "No WModel animation matched --name-regex")

    rendered: list[tuple[str, Image.Image]] = []
    for animation in animations:
        poses: list[dict[str, tuple[float, float, float]]] = []
        roots: list[tuple[float, float, float]] = []
        for normalized in SAMPLE_NORMALIZED_TIMES:
            raw = sample_bones(module, model, animation, animation.duration_ticks * normalized)
            pose, root = centered_pose(raw)
            poses.append(pose)
            roots.append(root)
        image = render_animation(animation, poses, roots)
        individual = output_dir / f"{safe_name(animation.name)}.png"
        if individual.exists() and not args.overwrite:
            raise RuntimeError(f"Output exists; pass --overwrite: {individual}")
        image.save(individual)
        rendered.append((animation.name, image))

    for sheet_offset in range(0, len(rendered), args.clips_per_sheet):
        sheet_number = sheet_offset // args.clips_per_sheet + 1
        path = output_dir / f"catalog_{sheet_number:02d}.png"
        if path.exists() and not args.overwrite:
            raise RuntimeError(f"Output exists; pass --overwrite: {path}")
        compose_sheet(rendered[sheet_offset:sheet_offset + args.clips_per_sheet], sheet_number).save(path)

    print(f"wmodel={wmodel}")
    print(f"animations={len(animations)}")
    print(f"sheets={math.ceil(len(animations) / args.clips_per_sheet)}")
    print(f"output={output_dir}")


if __name__ == "__main__":
    main()
