#!/usr/bin/env python3
"""Cut the filled character card and the rename / option icon buttons from the retail
characterselect.gfx atlases, and append their slots to
Data/UI/CharacterSelect/CharacterSelectWindow_Layout.json.

Source (retail characterselect.gfx, stage 1920x1080):
  CharacterSelectListRenderer (char 289), card body 228x60:
    bgMc frame 2 (filled)   CharacterSelect_IA (695,521,228,60) at (0,0)
    over glow               CharacterSelect_IA (756,647,227,58) at (1,1)
    selected frame          CharacterSelect_IA (229,707,227,58) at (1,1)
    playerDeco class icon   ClassIcon_gold, (6,3) at 0.5588 of 68 px = 38 px
  CharacterSelectBottomGroup buttons, 110x80 each, caption $YG760 14 at +(10,49) 100x50:
    changeName_btn  icon at +(40,12)  up CharacterSelect_ID (984,218,37,40)  over ID (984,176,37,40)
    option_btn      icon at +(38,11)  up CharacterSelect_ID (978,689,43,43)  over ID (933,920,43,43)
  Retail lines them up as 외형 변경 1459.5, 캐릭터명 변경 1559.5, 캐릭터 삭제 1659.5, 환경설정 1759.5
  (y 1000.5). Only rename and options are built, so rename takes the slot next to options.
The card slots already exist (CharSel_CardSlot_0..5 = the 228x60 card at 2/3); the class icon of
each filled card is a slot of its own, textured at runtime from the roster's class.

Pages: --pages holds characterselect_ia.png and characterselect_id.png
(D:/ClaudeWork/Extracted/CharSelectCards/pages, converted from EFUI_LOBBY's CharacterSelect_IA/ID).

Usage:
  python build_character_roster_ui.py --pages <dir> [--repo <root>]

The layout document is edited by the HUD layout tool, so slots are appended as text and a slot that
already exists is left alone.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

from PIL import Image

STAGE_SCALE = 2.0 / 3.0
OUT_DIR = 'UI/CharacterSelect'
LAYOUT = 'Data/UI/CharacterSelect/CharacterSelectWindow_Layout.json'


def crop(page: Image.Image, x: int, y: int, w: int, h: int) -> Image.Image:
    return page.crop((x, y, x + w, y + h))


def build_art(pages: Path, resources: Path) -> None:
    ia = Image.open(pages / 'characterselect_ia.png').convert('RGBA')
    idp = Image.open(pages / 'characterselect_id.png').convert('RGBA')

    body = crop(ia, 695, 521, 228, 60)

    def card(overlay: Image.Image | None) -> Image.Image:
        image = body.copy()
        if overlay is not None:
            image.alpha_composite(overlay, (1, 1))
        return image

    pieces = {
        'charsel_card_filled.png': card(None),
        'charsel_card_filled_hover.png': card(crop(ia, 756, 647, 227, 58)),
        'charsel_card_filled_selected.png': card(crop(ia, 229, 707, 227, 58)),
        'charsel_icon_rename.png': crop(idp, 984, 218, 37, 40),
        'charsel_icon_rename_hover.png': crop(idp, 984, 176, 37, 40),
        'charsel_icon_option.png': crop(idp, 978, 689, 43, 43),
        'charsel_icon_option_hover.png': crop(idp, 933, 920, 43, 43),
    }
    out = resources / OUT_DIR
    out.mkdir(parents=True, exist_ok=True)
    for name, image in pieces.items():
        image.save(out / name)
        print('%-36s %dx%d' % (name, image.width, image.height))


def stage_rect(x: float, y: float, w: float, h: float) -> tuple:
    return (x * STAGE_SCALE, y * STAGE_SCALE, w * STAGE_SCALE, h * STAGE_SCALE)


def slot_text(slot_id: str, rect: tuple, path: str) -> str:
    def f(value: float) -> str:
        return ('%.4f' % value).rstrip('0').rstrip('.')
    x, y, w, h = rect
    return (
        '    {\n'
        '      "id": "%s",\n'
        '      "ownerClass": null,\n'
        '      "type": 0,\n'
        '      "rect": { "x": %s, "y": %s, "width": %s, "height": %s },\n'
        '      "rotation": 0,\n'
        '      "stages": { "baseFrom": 0, "shineFrom": 1 },\n'
        '      "layers": [\n'
        '        { "path": "%s", "hoverPath": null, "tint": [1, 1, 1, 1], "additive": false, "flipX": false }\n'
        '      ],\n'
        '      "shine": { "texture": null, "additive": false },\n'
        '      "animation": { "fps": 10, "scale": 1, "offset": { "x": 0, "y": 0 }, "frames": [], "loop": true, "additive": false }\n'
        '    }') % (slot_id, f(x), f(y), f(w), f(h), path)


def build_slots(layout_path: Path) -> list:
    document = json.loads(layout_path.read_text(encoding='utf-8'))
    cards = {slot['id']: slot['rect'] for slot in document['slots']}
    slots = []
    # Class icon of each card: (6,3) 38x38 in card stage px, the card slot being the card at 2/3.
    for index in range(6):
        card = cards['CharSel_CardSlot_%d' % index]
        slots.append(('CharSel_CardIcon_%d' % index,
                      (card['x'] + 6 * STAGE_SCALE, card['y'] + 3 * STAGE_SCALE,
                       38 * STAGE_SCALE, 38 * STAGE_SCALE),
                      'UI/Common/White1x1.png'))
    # Icon buttons: button origins 1659.5 (rename) and 1759.5 (options) at y 1000.5.
    slots.append(('CharSel_RenameIcon', stage_rect(1659.5 + 40, 1000.5 + 12, 37, 40),
                  '%s/charsel_icon_rename.png' % OUT_DIR))
    slots.append(('CharSel_RenameLabelBox', stage_rect(1659.5 + 10, 1000.5 + 49, 100, 20),
                  'UI/Common/White1x1.png'))
    slots.append(('CharSel_OptionIcon', stage_rect(1759.5 + 38, 1000.5 + 11, 43, 43),
                  '%s/charsel_icon_option.png' % OUT_DIR))
    slots.append(('CharSel_OptionLabelBox', stage_rect(1759.5 + 10, 1000.5 + 49, 100, 20),
                  'UI/Common/White1x1.png'))
    # Rename dialog: the same art and rects as ClassSelect's CreateCharacterModal.
    slots.append(('CharSel_RenamePanel', (354.5, 250.0, 571.0, 131.0),
                  'UI/ClassSelect/Common/CreateCharacterModalPanel.png'))
    slots.append(('CharSel_RenameTextBox', (562.0, 310.0, 156.0, 28.0),
                  'UI/ClassSelect/Common/CreateCharacterModalTextBox.png'))
    slots.append(('CharSel_RenameConfirm', (530.0, 365.0, 100.0, 34.0),
                  'UI/ClassSelect/Common/NormalButton.png'))
    slots.append(('CharSel_RenameCancel', (650.0, 365.0, 100.0, 34.0),
                  'UI/ClassSelect/Common/NormalButton.png'))
    return slots


def append_slots(layout_path: Path) -> None:
    raw = layout_path.read_bytes().decode('utf-8')
    newline = '\r\n' if '\r\n' in raw else '\n'
    text = raw.replace('\r\n', '\n')
    added = [slot_text(slot_id, rect, path) for slot_id, rect, path in build_slots(layout_path)
             if '"id": "%s"' % slot_id not in text]
    if not added:
        print('layout already has every slot')
        return
    end = text.rstrip().rfind('\n  ]')
    if end < 0:
        raise SystemExit('unexpected layout tail')
    text = text[:end] + ',\n' + ',\n'.join(added) + '\n  ]\n}\n'
    json.loads(text)
    layout_path.write_bytes(text.replace('\n', newline).encode('utf-8'))
    print('%s  +%d slots' % (layout_path, len(added)))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--pages', type=Path, required=True)
    parser.add_argument('--repo', type=Path, default=Path('.'))
    args = parser.parse_args()
    build_art(args.pages, args.repo / 'Client' / 'Bin' / 'Resources')
    append_slots(args.repo / LAYOUT)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
