#!/usr/bin/env python3
"""Build the item repair window's layout document and crop its art from the retail atlases.

Outputs
-------
  Client/Bin/Resources/UI/Repair/*.png
      the four pieces interactionrepair.gfx and its shared components chrome own. The slot
      frame and close button are not cut here: interactionrepair uses the same
      arkSlot_renew_basic_V2 / closeBtn symbols the inventory window already installed as
      UI/Inventory/Retail/inv_slot.png and inv_close_btn.png, so those are referenced.
  Data/UI/Repair/RepairUI.json
      lostark.ui-layout slots for CRepairWindowView, the authored interactionRepair placement
      scaled 2/3 onto the 1280x720 reference.

Every rect below is a region the gfx itself names through DefineSubImage or a setProp block --
nothing is eyeballed. The survey is in .md/TJ/09-27/2026-09-27_수리창_PLAN.md.

The atlas pages come from export_upk_texture.py:
  python export_upk_texture.py <Packages>/OVSG0AGFLO7WALGMFDE2YW.upk interactionrepair_i6 --out <pages>
  python export_upk_texture.py <Packages>/OVSG0AAM8TMFOFLED62YWW.upk components_i1 --out <pages>
  python export_upk_texture.py <Packages>/OVSG0AAM8TMFOFLED62YWW.upk components_i8 --out <pages>

Usage:
  python build_repair_ui.py --pages <dir with the exported atlas PNGs> [--repo <LostArk root>]
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

from PIL import Image

# Retail authors this UI on a 1920-wide stage; this project's layout reference is 1280x720.
STAGE_SCALE = 2.0 / 3.0

# Where the window's top-left sits on the reference screen. The authored document has no screen
# position of its own -- the host places the window -- so this centres it and Clamp_ToScreen
# keeps a dragged window on screen afterwards.
WINDOW_WIDTH = 495.0
WINDOW_HEIGHT = 530.0
WINDOW_X = (1280.0 - WINDOW_WIDTH * STAGE_SCALE) * 0.5
WINDOW_Y = (720.0 - WINDOW_HEIGHT * STAGE_SCALE) * 0.5

CROPS = [
    ('components_i1.png', (0, 558, 495, 962), 'repair_window_bg.png'),
    ('components_i8.png', (289, 360, 780, 384), 'repair_top_deco.png'),
    ('interactionrepair_i6.png', (0, 0, 252, 33), 'repair_cost_bar.png'),
    ('interactionrepair_i6.png', (254, 0, 269, 15), 'repair_discount_icon.png'),
]

PANEL_BG = 'UI/Repair/repair_window_bg.png'
TOP_DECO = 'UI/Repair/repair_top_deco.png'
COST_BAR = 'UI/Repair/repair_cost_bar.png'
DISCOUNT = 'UI/Repair/repair_discount_icon.png'
SLOT = 'UI/Inventory/Retail/inv_slot.png'
CLOSE_BUTTON = 'UI/Inventory/Retail/inv_close_btn.png'

# The repair buttons have no bitmap of their own: retail draws ARKButton with vector shapes.
# The cost bar art is the same flat plate, so the button reuses it and its caption is drawn by
# the view, which is what every other migrated window in this repo does for a text button.
BUTTON = COST_BAR

SLOT_SIZE = 46.0
SLOT_GAP = 2.0
SLOT_COLUMNS = 8
EQUIP_SLOT_COUNT = 16
# Retail scrolls 80 inventory slots inside the 441x155 viewport. Without a scrollbar this build
# places only the rows that fit, so no slot is authored where nothing can ever draw it.
INVENTORY_SLOT_COUNT = 24


def scaled(value: float) -> float:
    return round(value * STAGE_SCALE, 4)


def rect(x: float, y: float, width: float, height: float) -> dict:
    """Authored content-space box -> a reference-resolution slot rect."""
    return {
        'x': round(WINDOW_X + x * STAGE_SCALE, 4),
        'y': round(WINDOW_Y + y * STAGE_SCALE, 4),
        'width': scaled(width),
        'height': scaled(height),
    }


def slot(slot_id: str, box: dict, image: str) -> dict:
    return {
        'id': slot_id,
        'ownerClass': None,
        'type': 0,
        'rect': box,
        'rotation': 0.0,
        'layers': [{'path': image, 'tint': [1.0, 1.0, 1.0, 1.0],
                    'additive': False, 'flipX': False}],
    }


def build_slots() -> list:
    slots = [
        # The panel is authored 495x435 but stretched to the content's full height; the
        # document stores the drawn size, not the source bitmap's.
        slot('Repair_PanelBg', rect(0.0, 0.0, WINDOW_WIDTH, WINDOW_HEIGHT), PANEL_BG),
        slot('Repair_TopDeco', rect(3.0, 4.0, 491.0, 24.0), TOP_DECO),
        # Title bar and the two section labels are text anchors, not art: the view keeps them
        # hidden and only reads Get_SlotRect from them. Every slot needs a layer path, so they
        # carry a piece that is never drawn.
        slot('Repair_Title', rect(0.0, 0.0, WINDOW_WIDTH, 40.0), TOP_DECO),
        slot('Repair_CloseBtn', rect(465.0, 10.0, 20.0, 20.0), CLOSE_BUTTON),

        slot('Repair_EquipLabel', rect(12.0, 41.0, 200.0, 22.0), COST_BAR),
        slot('Repair_EquipCostBar', rect(13.0, 191.0, 252.0, 33.0), COST_BAR),
        slot('Repair_EquipDiscount', rect(250.0, 205.0, 15.0, 15.0), DISCOUNT),
        slot('Repair_EquipButton', rect(274.0, 190.0, 160.0, 34.0), BUTTON),

        slot('Repair_InventoryLabel', rect(11.0, 238.0, 200.0, 22.0), COST_BAR),
        slot('Repair_AllCostBar', rect(13.0, 444.0, 252.0, 33.0), COST_BAR),
        slot('Repair_AllDiscount', rect(250.0, 456.0, 15.0, 15.0), DISCOUNT),
        slot('Repair_AllButton', rect(274.0, 442.0, 160.0, 34.0), BUTTON),

        slot('Repair_MoneyBar', rect(13.0, 494.0, 452.0, 28.0), COST_BAR),
    ]

    for index in range(EQUIP_SLOT_COUNT):
        column = index % SLOT_COLUMNS
        row = index // SLOT_COLUMNS
        slots.append(slot(
            'Repair_EquipSlot_%d' % index,
            rect(16.0 + column * (SLOT_SIZE + SLOT_GAP),
                 80.0 + row * (SLOT_SIZE + SLOT_GAP), SLOT_SIZE, SLOT_SIZE),
            SLOT))

    for index in range(INVENTORY_SLOT_COUNT):
        column = index % SLOT_COLUMNS
        row = index // SLOT_COLUMNS
        slots.append(slot(
            'Repair_InventorySlot_%d' % index,
            rect(15.0 + column * (SLOT_SIZE + SLOT_GAP),
                 278.0 + row * (SLOT_SIZE + SLOT_GAP), SLOT_SIZE, SLOT_SIZE),
            SLOT))

    return slots


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--pages', type=Path, required=True,
                        help='directory holding the exported atlas PNGs')
    parser.add_argument('--repo', type=Path, default=Path('.'))
    args = parser.parse_args()

    resources = args.repo / 'Client' / 'Bin' / 'Resources' / 'UI' / 'Repair'
    resources.mkdir(parents=True, exist_ok=True)
    for source, box, destination in CROPS:
        image = Image.open(args.pages / source).crop(box)
        image.save(resources / destination)
        print('%-28s %dx%d' % (destination, image.width, image.height))

    document = {
        'schema': 'lostark.ui-layout',
        'formatVersion': 1,
        'resolution': {'width': 1280, 'height': 720},
        'classes': ['Default'],
        'slots': build_slots(),
    }
    data = args.repo / 'Data' / 'UI' / 'Repair'
    data.mkdir(parents=True, exist_ok=True)
    target = data / 'RepairUI.json'
    target.write_text(json.dumps(document, ensure_ascii=False, indent=2) + '\n',
                      encoding='utf-8')
    print('%s  %d slots' % (target, len(document['slots'])))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
