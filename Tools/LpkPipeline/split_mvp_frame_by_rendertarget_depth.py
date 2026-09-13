#!/usr/bin/env python3
"""Split the MVP frame keyframe document at the 3D render targets' depths.

Why
---
The award page stages four characters and they do NOT share one depth. The
winner is `mvpGFxRenderTarget` at depth 13 of the MvpResultFrame sprite, but
each party column's character is a `renderTex` inside its `otherStatItemN`
instance, and those instances sit at depths 27 / 66 / 105. So the frame art
splits into three bands, not two:

  depth < 13    backdrop the winner stands in front of
  13 .. 26      over the winner, under the columns -- this is where
                `MvpFrame_d21.png` lives, a 321x1080 fully opaque column panel
  depth >= 27   over everything, including `MvpFrame_d27.png` (the column's own
                lower gradient, char 215 at depth 8 inside otherStatItem) and
                `MvpFrame_d170.png`, the 1x996 strip that sinks the winner's
                legs into the dark lower half of the page

Cutting once at 13 puts the opaque d21 panels above the column characters and
hides them completely, which is exactly what happened the first time.

The extracted keyframe document is one flat layer list, so the runtime draws
every layer under the character and the legs stay fully lit. The authored depth
survives in each layer's asset name (`MvpFrame_d<depth>.png`), so the document
can be split back into a below-character and an above-character half without
re-running the original gfx extraction.

Layers whose asset carries no `_d<depth>` name are the nested effect flipbooks
(Shine, ParticleBoomGold, Beam). Their depth is inside their own sprite, so it
cannot be compared with the frame's and --effects decides which half they join.
Use `below`: ParticleBoomGold runs f32-f116 centred on the MVP panel and the
retail capture's settled frame shows that golden burst *behind* the winner, so
the character emerges out of the burst rather than being washed out by it. The
default is `above` only because that is where the unsplit document drew them --
pass it deliberately, never leave it to the default.

Usage:
  python split_mvp_frame_by_rendertarget_depth.py \
      --document Data/UI/MVP/MvpResult_Intro.keyframes.json \
      --depth 13 [--effects above|below]
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

DEPTH_IN_ASSET = re.compile(r"_d(\d+)\.png$", re.IGNORECASE)


def layer_depth(layer):
    """Authored depth of a layer, or None when its asset does not carry one."""
    for key in layer.get("keyframes", ()):
        match = DEPTH_IN_ASSET.search(key.get("asset", "") or "")
        if match:
            return int(match.group(1))
    return None


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--document", type=Path, required=True)
    parser.add_argument("--depth", type=int, default=13,
                        help="the winner render target's authored depth")
    parser.add_argument("--column-depth", type=int, default=27,
                        help="the depth of otherStatItem0, which owns the first "
                             "column's character")
    parser.add_argument("--effects", choices=("above", "below"), default="above")
    args = parser.parse_args()

    document = json.loads(args.document.read_text(encoding="utf-8"))
    bands = {"below": [], "mid": [], "above": []}
    report = []
    for index, layer in enumerate(document["layers"]):
        depth = layer_depth(layer)
        if depth is None:
            side = args.effects
        elif depth < args.depth:
            side = "below"
        elif depth < args.column_depth:
            side = "mid"
        else:
            side = "above"
        bands[side].append(layer)
        first = next((k.get("asset") for k in layer["keyframes"] if k.get("asset")), "")
        report.append((index, depth, side, Path(first).name))

    empty = [name for name, layers in bands.items() if not layers]
    if empty:
        raise SystemExit("empty band(s): %s -- check --depth/--column-depth"
                         % ", ".join(empty))

    stem = args.document.name.replace(".keyframes.json", "")
    for side, layers in (("Below", bands["below"]), ("Mid", bands["mid"]),
                         ("Above", bands["above"])):
        out = dict(document)
        out["layers"] = layers
        path = args.document.with_name("%s_%s.keyframes.json" % (stem, side))
        path.write_text(json.dumps(out, ensure_ascii=False, indent=1),
                        encoding="utf-8")
        print("%s  %d layers" % (path.name, len(layers)))

    print("\nidx depth  side   asset")
    for index, depth, side, asset in report:
        print("%3d %-6s %-6s %s"
              % (index, "-" if depth is None else "d%d" % depth, side, asset))
    return 0


if __name__ == "__main__":
    sys.exit(main())
