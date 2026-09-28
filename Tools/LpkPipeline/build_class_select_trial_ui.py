#!/usr/bin/env python3
"""Cut the class-selection preview/trial buttons and the 체험 모드 banner from the retail atlases,
and append their slots to Data/UI/ClassSelect/ClassSelect_Layout.json.

Source: retail createcharacter.gfx, sprite 219 CreateCharacterCenterFrame (a full-screen
1920x1080 overlay, so its children's coordinates are absolute stage px):
  preview_btn  AnimatedButton_renew_V2VerticalAlign  (799,948) 160x40  "미리보기"  pccreate.preview_btn
  try_btn      AnimatedButton_renew_V2VerticalAlign  (961,948) 160x40  "체험 하기" pccreate.trymod_btn
  select_btn   AnimatedButton_CenterFrame_CreateBtn  (786,992) 348x55  "캐릭터 생성" pccreate.complete_btn
  exam_title_bgMc   band shape 216 (createCharacter_ID7 0,0,1920,220) at x -540 scaled 1.5625,
                    plate shape 198 (createCharacter_I1 0,661,689,152) at (616,0)
  exam_title_lb     (660,14) 600x50, "체험 모드" pccreate.trymode_title, YoonGasiIIM 26 #FFF6E2
V2 button cells are shareImageV2 V2btn_normal/over (103x36); the create button is plate 202 (normal)
or 202 + glow 207 (over) under the ornament frame 204. The 1280x720 layout is the stage x 2/3.

Pages:
  --createcharacter  dir holding createcharacter_i1.png, createcharacter_id7.png
                     (D:/ClaudeWork/Extracted/CustomizeGfx_Extracted/png)
  --shareimage       dir holding shareimagev2_i46.png
                     (D:/ClaudeWork/Extracted/ShareImageGfx_Extracted/tex/EFUI_SHAREIMAGE)

Usage:
  python build_class_select_trial_ui.py --createcharacter <dir> --shareimage <dir> [--repo <root>]

The layout document is edited by the HUD layout tool, so the slots are appended as text and a slot
that already exists is left as it is.
"""
from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image

STAGE_SCALE = 2.0 / 3.0
OUT_DIR = 'UI/ClassSelect/Common'


def crop(page: Image.Image, x: int, y: int, w: int, h: int) -> Image.Image:
    return page.crop((x, y, x + w, y + h))


def build_art(createcharacter: Path, shareimage: Path, resources: Path) -> None:
    cc = Image.open(createcharacter / 'createcharacter_i1.png').convert('RGBA')
    cc7 = Image.open(createcharacter / 'createcharacter_id7.png').convert('RGBA')
    sv = Image.open(shareimage / 'shareimagev2_i46.png').convert('RGBA')

    v2_normal = crop(sv, 643, 988, 103, 36)
    v2_over = v2_normal.copy()
    v2_over.alpha_composite(crop(sv, 328, 988, 103, 36))

    frame = crop(cc, 0, 897, 348, 55)
    plate = crop(cc, 461, 897, 310, 44)
    glow = crop(cc, 0, 954, 310, 44)

    def create_button(with_glow: bool) -> Image.Image:
        image = Image.new('RGBA', (348, 55))
        image.alpha_composite(plate, (19, 6))
        if with_glow:
            image.alpha_composite(glow, (19, 6))
        image.alpha_composite(frame, (0, 0))
        return image

    # The band is placed at x -540 and stretched 1.5625x, so the part on screen is raw x
    # 540/1.5625 .. (540+1920)/1.5625; the slot stretches it back across the full width.
    band = crop(cc7, 346, 0, 1228, 220)

    out = resources / OUT_DIR
    out.mkdir(parents=True, exist_ok=True)
    pieces = {
        'TrialV2Button.png': v2_normal,
        'TrialV2ButtonHover.png': v2_over,
        'CreateCharacterCenterButton.png': create_button(False),
        'CreateCharacterCenterButtonHover.png': create_button(True),
        'TrialModeBand.png': band,
        'TrialModePlate.png': crop(cc, 0, 661, 689, 152),
    }
    for name, image in pieces.items():
        image.save(out / name)
        print('%-40s %dx%d' % (name, image.width, image.height))


def slot_text(slot_id: str, x: float, y: float, w: float, h: float,
              path: str, hover: str | None) -> str:
    def f(value: float) -> str:
        return ('%.4f' % value).rstrip('0').rstrip('.')
    hover_text = 'null' if hover is None else '"%s"' % hover
    return (
        '    {\n'
        '      "id": "%s",\n'
        '      "ownerClass": null,\n'
        '      "type": 0,\n'
        '      "rect": { "x": %s, "y": %s, "width": %s, "height": %s },\n'
        '      "rotation": 0,\n'
        '      "stages": { "baseFrom": 0, "shineFrom": 1 },\n'
        '      "layers": [\n'
        '        { "path": "%s", "hoverPath": %s, "tint": [1, 1, 1, 1], "additive": false, "flipX": false }\n'
        '      ],\n'
        '      "shine": { "texture": null, "additive": false },\n'
        '      "animation": { "fps": 10, "scale": 1, "offset": { "x": 0, "y": 0 }, "frames": [], "loop": true, "additive": false }\n'
        '    }') % (slot_id, f(x), f(y), f(w), f(h), path, hover_text)


def stage(value: float) -> float:
    return value * STAGE_SCALE


SLOTS = [
    # (id, stage x, y, w, h, art, hover art)
    ('TrialModeBand', 0.0, 0.0, 1920.0, 220.0, 'TrialModeBand.png', None),
    ('TrialModePlate', 616.0, 0.0, 689.0, 152.0, 'TrialModePlate.png', None),
    ('PreviewButton', 799.0, 948.0, 160.0, 40.0, 'TrialV2Button.png', 'TrialV2ButtonHover.png'),
    ('TrialButton', 961.0, 948.0, 160.0, 40.0, 'TrialV2Button.png', 'TrialV2ButtonHover.png'),
    ('CreateCharacterCenterButton', 786.0, 992.0, 348.0, 55.0,
     'CreateCharacterCenterButton.png', 'CreateCharacterCenterButtonHover.png'),
]


def append_slots(layout: Path) -> None:
    raw = layout.read_bytes().decode('utf-8')
    newline = '\r\n' if '\r\n' in raw else '\n'
    text = raw.replace('\r\n', '\n')
    added = []
    for slot_id, x, y, w, h, art, hover in SLOTS:
        if '"id": "%s"' % slot_id in text:
            continue
        added.append(slot_text(slot_id, stage(x), stage(y), stage(w), stage(h),
                               '%s/%s' % (OUT_DIR, art),
                               None if hover is None else '%s/%s' % (OUT_DIR, hover)))
    if not added:
        print('layout already has every slot')
        return
    tail = '\n  ]\n}'
    end = text.rstrip().rfind('\n  ]')
    if end < 0 or not text.rstrip().endswith(tail.strip('\n')[-3:].strip() or '}'):
        raise SystemExit('unexpected layout tail')
    body = text[:end]
    text = body + ',\n' + ',\n'.join(added) + tail + '\n'
    layout.write_bytes(text.replace('\n', newline).encode('utf-8'))
    print('%s  +%d slots' % (layout, len(added)))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--createcharacter', type=Path, required=True)
    parser.add_argument('--shareimage', type=Path, required=True)
    parser.add_argument('--repo', type=Path, default=Path('.'))
    args = parser.parse_args()
    build_art(args.createcharacter, args.shareimage,
              args.repo / 'Client' / 'Bin' / 'Resources')
    append_slots(args.repo / 'Data' / 'UI' / 'ClassSelect' / 'ClassSelect_Layout.json')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
