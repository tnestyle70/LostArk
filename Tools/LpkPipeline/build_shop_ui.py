#!/usr/bin/env python3
"""Build the NPC shop window's layout document and crop its art from the retail atlases.

Outputs
-------
  Client/Bin/Resources/UI/Shop/*.png
      the list-cell plate and the cost coin interactionmarket.gfx names through DefineSubImage.
      The window chrome, slot frame, close button and flat button plate are not cut here: they
      are the same components pieces the repair window already installed, so those are
      referenced.
  Client/Bin/Resources/UI/Items/Common/battle_*.png
      the 파괴 폭탄 / 회오리 수류탄 icons, EFTable_Item Icon Battle_Item_01 index 21 / 53 ->
      IconInfo -> Battle_Item_0 page at (512,64) / (512,192), 64x64; 성스러운 부적 (101191,
      index 51 -> the same page at (384,192)).
  Client/Bin/Resources/UI/Common/Money_Silver.png, Money_Gold.png
      the purse coins: Shared_MoneyLabel_Elem_MoneyMC frame 2 (icon 1, 실링) and frame 11
      (icon 10, 골드), both on components_i8 -- the glyphs the inventory's costMc / currencyMc
      and every other ARKMoneyLabel draw.
  Data/UI/Shop/ShopUI.json
      lostark.ui-layout slots for CShopWindowView: the authored MarketWndContent placement
      scaled 2/3 onto the 1280x720 reference.

Every position below is MarketWndContent / MarketListItem's own (ffdec dump of
interactionmarket.gfx); the survey is in .md/TJ/09-28/2026-09-28_상점창_PLAN.md.

The atlas pages come from export_upk_texture.py / UModel:
  python export_upk_texture.py <Packages>/<interactionmarket package>.upk interactionmarket_i36 --out <pages>
  EFUI_ICONATLAS_B Texture2D battle_item_0 (UModel tga export)
  python export_upk_texture.py <Packages>/OVSG0AAM8TMFOFLED62YWW.upk components_i8 --out <pages>

Usage:
  python build_shop_ui.py --pages <dir with interactionmarket_i36.png>
                          --battle-atlas <battle_item_0.tga> [--repo <LostArk root>]
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

from PIL import Image

STAGE_SCALE = 2.0 / 3.0

# MarketWndContent's widest child is the 715-wide list viewport at x 12, and the lowest is the
# second money line ending at y 665. The host window adds no padding (contentPadding top 0), so
# the drawn panel is the content box plus the same 12 px margin on the right and bottom.
# The retail tabs (구매 / 재구매 / 판매, y 39..73) and the category row (y 85) are not built:
# only buying exists. Everything from the list down moves up by CONTENT_SHIFT so the list sits
# just under the title bar, and the panel is that much shorter.
CONTENT_SHIFT = 80.0
WINDOW_WIDTH = 739.0
WINDOW_HEIGHT = 680.0 - CONTENT_SHIFT
WINDOW_X = (1280.0 - WINDOW_WIDTH * STAGE_SCALE) * 0.5
WINDOW_Y = (720.0 - WINDOW_HEIGHT * STAGE_SCALE) * 0.5

PANEL_BG = 'UI/Repair/repair_window_bg.png'
TOP_DECO = 'UI/Repair/repair_top_deco.png'
PLATE = 'UI/Repair/repair_cost_bar.png'
SLOT = 'UI/Inventory/Retail/inv_slot.png'
CLOSE_BUTTON = 'UI/Inventory/Retail/inv_close_btn.png'
CELL = 'UI/Shop/shop_cell.png'
COIN = 'UI/Shop/shop_coin.png'

MARKET_CROPS = [
    ((0, 0, 342, 78), 'shop_cell.png'),
    ((630, 0, 655, 24), 'shop_coin.png'),
]
# (atlas, box on that page, file under UI/Items/Common)
ICON_CROPS = [
    ('battle', (512, 64, 576, 128), 'battle_destruction_bomb.png'),
    ('battle', (512, 192, 576, 256), 'battle_whirlwind_grenade.png'),
    ('battle', (384, 192, 448, 256), 'battle_holy_charm.png'),
]
# (box on components_i8, file under UI/Common)
COIN_CROPS = [
    ((869, 360, 894, 384), 'Money_Silver.png'),
    ((413, 332, 437, 358), 'Money_Gold.png'),
]

# Grid: two columns at x 12 / 357, five rows from y 124 (before the shift) on an 80 px pitch,
# each cell 342x78.
CELL_COLUMNS = (12.0, 357.0)
CELL_ROW_Y = 124.0 - CONTENT_SHIFT
CELL_PITCH_Y = 80.0
CELL_ROWS = 5
CELL_WIDTH = 342.0
CELL_HEIGHT = 78.0
# MarketListItem: the item slot sits at the cell origin at scale 0.806 of the 70 px arkSlot,
# the cost coin at (316,54).
CELL_SLOT_SIZE = 56.0
CELL_COIN = (316.0, 54.0, 25.0, 24.0)

# Item icons are the shop's stock, which CShopWindowView reads from ItemCatalog.json "shops"
# and pushes with Set_SlotTexture; the document only places them. Until then they carry a
# plain placeholder the view hides whenever a cell or basket slot is empty.
ICON_PLACEHOLDER = 'UI/Common/White1x1.png'

# basketSlotList at (13,553): ten slots from x 63 on a 57 px pitch, arkSlot at scale 0.657.
BASKET_X = 13.0 + 50.0
BASKET_Y = 553.0 - CONTENT_SHIFT
BASKET_PITCH = 57.0
BASKET_COUNT = 10
BASKET_SIZE = 46.0


def scaled(value: float) -> float:
    return round(value * STAGE_SCALE, 4)


def rect(x: float, y: float, width: float, height: float) -> dict:
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
        slot('Shop_PanelBg', rect(0.0, 0.0, WINDOW_WIDTH, WINDOW_HEIGHT), PANEL_BG),
        slot('Shop_TopDeco', rect(3.0, 4.0, WINDOW_WIDTH - 4.0, 24.0), TOP_DECO),
        # Text anchor only (the view hides it): the title bar is also the drag handle.
        slot('Shop_Title', rect(0.0, 0.0, WINDOW_WIDTH, 38.0), TOP_DECO),
        slot('Shop_CloseBtn', rect(WINDOW_WIDTH - 30.0, 10.0, 20.0, 20.0), CLOSE_BUTTON),
    ]

    for index in range(len(CELL_COLUMNS) * CELL_ROWS):
        column = index % len(CELL_COLUMNS)
        row = index // len(CELL_COLUMNS)
        x = CELL_COLUMNS[column]
        y = CELL_ROW_Y + row * CELL_PITCH_Y
        slots.append(slot('Shop_Cell_%d' % index, rect(x, y, CELL_WIDTH, CELL_HEIGHT), CELL))
        slots.append(slot('Shop_CellSlot_%d' % index,
                          rect(x, y, CELL_SLOT_SIZE, CELL_SLOT_SIZE), SLOT))
        inset = 4.0
        slots.append(slot('Shop_CellIcon_%d' % index,
                          rect(x + inset, y + inset, CELL_SLOT_SIZE - inset * 2.0,
                               CELL_SLOT_SIZE - inset * 2.0), ICON_PLACEHOLDER))
        cx, cy, cw, ch = CELL_COIN
        slots.append(slot('Shop_CellCoin_%d' % index, rect(x + cx, y + cy, cw, ch), COIN))

    # pagebg: the same plate twice across the bottom band, with the basket on top of it.
    slots.append(slot('Shop_PageBg_0', rect(12.0, 524.0 - CONTENT_SHIFT, CELL_WIDTH, CELL_HEIGHT), CELL))
    slots.append(slot('Shop_PageBg_1', rect(357.0, 524.0 - CONTENT_SHIFT, CELL_WIDTH, CELL_HEIGHT), CELL))
    for index in range(BASKET_COUNT):
        x = BASKET_X + index * BASKET_PITCH
        slots.append(slot('Shop_BasketSlot_%d' % index,
                          rect(x, BASKET_Y, BASKET_SIZE, BASKET_SIZE), SLOT))
        inset = 3.0
        slots.append(slot('Shop_BasketIcon_%d' % index,
                          rect(x + inset, BASKET_Y + inset, BASKET_SIZE - inset * 2.0,
                               BASKET_SIZE - inset * 2.0), ICON_PLACEHOLDER))

    # financialMc: 구매 금액 (y 617) and 구매 후 잔액 (y 641), coin at the right end of each.
    slots.append(slot('Shop_BuyAmount', rect(20.0, 617.0 - CONTENT_SHIFT, 268.0, 22.0), PLATE))
    slots.append(slot('Shop_BuyAmountCoin', rect(290.0, 616.0 - CONTENT_SHIFT, 25.0, 24.0), COIN))
    slots.append(slot('Shop_Balance', rect(20.0, 641.0 - CONTENT_SHIFT, 268.0, 22.0), PLATE))
    slots.append(slot('Shop_BalanceCoin', rect(290.0, 640.0 - CONTENT_SHIFT, 25.0, 24.0), COIN))

    # oddmentBtn / purchaseBtn / emptyBtn along y 610, ARKButton at 0.951 x 1.686.
    slots.append(slot('Shop_JunkBtn', rect(396.0, 610.0 - CONTENT_SHIFT, 98.0, 40.0), PLATE))
    slots.append(slot('Shop_BuyBtn', rect(498.0, 610.0 - CONTENT_SHIFT, 98.0, 40.0), PLATE))
    slots.append(slot('Shop_EmptyBtn', rect(600.0, 610.0 - CONTENT_SHIFT, 98.0, 40.0), PLATE))
    return slots


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--pages', type=Path, required=True)
    parser.add_argument('--battle-atlas', type=Path, required=True)
    parser.add_argument('--repo', type=Path, default=Path('.'))
    args = parser.parse_args()

    resources = args.repo / 'Client' / 'Bin' / 'Resources' / 'UI'
    (resources / 'Shop').mkdir(parents=True, exist_ok=True)
    page = Image.open(args.pages / 'interactionmarket_i36.png')
    for box, name in MARKET_CROPS:
        piece = page.crop(box)
        piece.save(resources / 'Shop' / name)
        print('%-30s %dx%d' % (name, piece.width, piece.height))
    atlases = {'battle': Image.open(args.battle_atlas).convert('RGBA')}
    for atlas, box, name in ICON_CROPS:
        piece = atlases[atlas].crop(box)
        piece.save(resources / 'Items' / 'Common' / name)
        print('%-30s %dx%d' % (name, piece.width, piece.height))
    components = Image.open(args.pages / 'components_i8.png').convert('RGBA')
    for box, name in COIN_CROPS:
        piece = components.crop(box)
        piece.save(resources / 'Common' / name)
        print('%-30s %dx%d' % (name, piece.width, piece.height))

    document = {
        'schema': 'lostark.ui-layout',
        'formatVersion': 1,
        'resolution': {'width': 1280, 'height': 720},
        'classes': ['Default'],
        'slots': build_slots(),
    }
    data = args.repo / 'Data' / 'UI' / 'Shop'
    data.mkdir(parents=True, exist_ok=True)
    target = data / 'ShopUI.json'
    target.write_text(json.dumps(document, ensure_ascii=False, indent=2) + '\n',
                      encoding='utf-8')
    print('%s  %d slots' % (target, len(document['slots'])))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
