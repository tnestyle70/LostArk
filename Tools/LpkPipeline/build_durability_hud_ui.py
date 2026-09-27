#!/usr/bin/env python3
"""Build the durability HUD's layout document and crop its art from the retail atlas.

Outputs
-------
  Client/Bin/Resources/UI/Durability/*.png
      the armour silhouette (two variants, by whether the account has bracers enabled) and
      the damaged/destroyed overlay for each of the eight parts, cut at the DefineSubImage
      regions durability.gfx resolves to.
  Data/UI/Durability/DurabilityUI.json
      lostark.ui-layout slots for CDurabilityHudView: the authored DurabilityFrame placement
      scaled 2/3 onto the 1280x720 reference, parked under the minimap on the right edge.

The overlay contract is DurabilityFrame.as `set setDurabilityPart`: per part, -1 hides the
overlay, 0 shows the "demaged" frame and >=1 shows "destroy". The survey is in
.md/TJ/09-27/2026-09-27_수리창_PLAN.md.

The atlas page comes from export_upk_texture.py:
  python export_upk_texture.py <Packages>/OVSG0AHS7W3G1GLKD62YW9.upk durability_i7 --out <pages>

Usage:
  python build_durability_hud_ui.py --pages <dir with durability_i7.png> [--repo <root>]
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

from PIL import Image

STAGE_SCALE = 2.0 / 3.0
# durability.gfx carries no screen position: the host places the frame, and no other .gfx of
# the 661 in the sweep contains DurabilityFrame (commonobject's LifeDurabilityFrame is the
# life-tool tooltip row and epicgatedurability is the epic-gate mount). UserOption.xml stores
# no HUD coordinates either, so this is measured off a retail raid capture instead: the icon
# sits against the top edge, about 79% across. Converting that frame's game area onto this
# reference puts its centre near x 1014 with the top within ten pixels of the screen edge.
# A compressed stream frame is worth maybe +/-10 px, so treat these as the measured values,
# not exact authored ones. The minimap owns x 1063..1281 y 25..198
# (Data/UI/Minimap/Minimap_Layout.json), which this clears.
HUD_X = 1007.0
HUD_Y = 12.0

PAGE = 'durability_i7.png'

# name -> (atlas x, y, width, height). The two silhouette variants first, then each part's
# damaged and destroyed frame.
PIECES = {
    'base_bracer': (0, 0, 71, 96),
    'base_plain': (73, 0, 71, 96),
    'weapon_damaged': (230, 0, 14, 55),
    'weapon_destroyed': (213, 0, 15, 55),
    'helmet_damaged': (562, 59, 15, 16),
    'helmet_destroyed': (528, 59, 16, 17),
    'top_damaged': (288, 0, 40, 52),
    'top_destroyed': (246, 0, 40, 52),
    'gloves_damaged': (975, 0, 48, 28),
    'gloves_destroyed': (146, 59, 48, 28),
    'gloves_plain_damaged': (196, 59, 48, 28),
    'gloves_plain_destroyed': (867, 0, 49, 29),
    'bottoms_damaged': (179, 0, 32, 56),
    'bottoms_destroyed': (146, 0, 31, 57),
    'shoulder_damaged': (483, 59, 43, 17),
    'shoulder_destroyed': (340, 59, 45, 19),
    'life_damaged': (665, 0, 31, 40),
    'life_destroyed': (698, 0, 31, 40),
    'bracer_damaged': (435, 59, 46, 18),
    'bracer_destroyed': (387, 59, 46, 18),
}

# slot id -> (authored x, y inside repairGroup, the piece whose size the slot starts at).
# The view swaps texture and rect together when a part changes state, so the authored entry is
# just the damaged frame -- the state the slot is first shown in.
PARTS = [
    ('Durability_Weapon', 9.0, 0.0, 'weapon_damaged'),
    ('Durability_Helmet', 41.0, 1.0, 'helmet_damaged'),
    ('Durability_Top', 27.8, 7.2, 'top_damaged'),
    ('Durability_Gloves', 24.0, 27.0, 'gloves_damaged'),
    ('Durability_Bottoms', 32.0, 39.0, 'bottoms_damaged'),
    ('Durability_Shoulder', 27.0, 10.0, 'shoulder_damaged'),
    ('Durability_Life', 1.0, 56.0, 'life_damaged'),
    ('Durability_Bracer', 25.0, 24.0, 'bracer_damaged'),
]


def scaled(value: float) -> float:
    return round(value * STAGE_SCALE, 4)


def slot(slot_id: str, x: float, y: float, width: float, height: float, image: str) -> dict:
    return {
        'id': slot_id,
        'ownerClass': None,
        'type': 0,
        'rect': {
            'x': round(HUD_X + x * STAGE_SCALE, 4),
            'y': round(HUD_Y + y * STAGE_SCALE, 4),
            'width': scaled(width),
            'height': scaled(height),
        },
        'rotation': 0.0,
        'layers': [{'path': 'UI/Durability/%s.png' % image,
                    'tint': [1.0, 1.0, 1.0, 1.0], 'additive': False, 'flipX': False}],
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--pages', type=Path, required=True)
    parser.add_argument('--repo', type=Path, default=Path('.'))
    args = parser.parse_args()

    page = Image.open(args.pages / PAGE)
    resources = args.repo / 'Client' / 'Bin' / 'Resources' / 'UI' / 'Durability'
    resources.mkdir(parents=True, exist_ok=True)
    for name, (x, y, width, height) in PIECES.items():
        piece = page.crop((x, y, x + width, y + height))
        piece.save(resources / ('%s.png' % name))
        print('%-26s %dx%d' % (name, piece.width, piece.height))

    slots = [slot('Durability_Base', 1.0, 0.0, 71.0, 96.0, 'base_bracer')]
    for slot_id, x, y, piece_name in PARTS:
        _, _, width, height = PIECES[piece_name]
        slots.append(slot(slot_id, x, y, float(width), float(height), piece_name))

    document = {
        'schema': 'lostark.ui-layout',
        'formatVersion': 1,
        'resolution': {'width': 1280, 'height': 720},
        'classes': ['Default'],
        'slots': slots,
    }
    data = args.repo / 'Data' / 'UI' / 'Durability'
    data.mkdir(parents=True, exist_ok=True)
    target = data / 'DurabilityUI.json'
    target.write_text(json.dumps(document, ensure_ascii=False, indent=2) + '\n',
                      encoding='utf-8')
    print('%s  %d slots' % (target, len(slots)))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
