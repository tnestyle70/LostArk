#!/usr/bin/env python3
"""characterselect.gfx atlas에서 캐릭터 카드·이름 변경·설정 이미지를 만든다.
1920×1080 원본 좌표를 2/3로 변환하고 CharacterSelectWindow_Layout.json에 없는 slot을 추가한다.
--pages의 characterselect_ia.png/id.png를 사용하며 기존 slot의 배치는 유지한다.
"""
from __future__ import annotations

# atlas 폴더와 저장소 경로를 받으며 이 모듈 설명을 CLI 도움말로 표시한다.
import argparse
# 기존 layout을 읽고 추가할 JSON의 문법을 검사한다.
import json
# 이미지와 layout의 저장소 상대 경로를 만든다.
from pathlib import Path

# 카드 원본을 자르고 hover·selected 테두리를 합성한다.
from PIL import Image

STAGE_SCALE = 2.0 / 3.0
OUT_DIR = 'UI/CharacterSelect'
LAYOUT = 'Data/UI/CharacterSelect/CharacterSelectWindow_Layout.json'


def crop(page: Image.Image, x: int, y: int, w: int, h: int) -> Image.Image:
    return page.crop((x, y, x + w, y + h))


# 원본 카드와 테두리를 조합해 상태별 PNG와 하단 버튼 이미지를 저장한다.
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


# 원본 stage 좌표를 1280×720 layout 기준으로 환산한다.
def stage_rect(x: float, y: float, w: float, h: float) -> tuple:
    return (x * STAGE_SCALE, y * STAGE_SCALE, w * STAGE_SCALE, h * STAGE_SCALE)


# 기존 layout의 텍스트 형식에 맞춰 stable ID·rect·이미지 경로를 작성한다.
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
    # 현재 카드 rect에 원본 local (6,3), 크기 38×38의 2/3 값을 더한다.
    for index in range(6):
        card = cards['CharSel_CardSlot_%d' % index]
        slots.append(('CharSel_CardIcon_%d' % index,
                      (card['x'] + 6 * STAGE_SCALE, card['y'] + 3 * STAGE_SCALE,
                       38 * STAGE_SCALE, 38 * STAGE_SCALE),
                      'UI/Common/White1x1.png'))
    # 이름 변경은 설정 옆 x=1659.5로 옮기고 설정은 원본 x=1759.5를 유지한다. 공통 y는 1000.5다.
    slots.append(('CharSel_RenameIcon', stage_rect(1659.5 + 40, 1000.5 + 12, 37, 40),
                  '%s/charsel_icon_rename.png' % OUT_DIR))
    slots.append(('CharSel_RenameLabelBox', stage_rect(1659.5 + 10, 1000.5 + 49, 100, 20),
                  'UI/Common/White1x1.png'))
    slots.append(('CharSel_OptionIcon', stage_rect(1759.5 + 38, 1000.5 + 11, 43, 43),
                  '%s/charsel_icon_option.png' % OUT_DIR))
    slots.append(('CharSel_OptionLabelBox', stage_rect(1759.5 + 10, 1000.5 + 49, 100, 20),
                  'UI/Common/White1x1.png'))
    # 이름 변경 창은 ClassSelect의 캐릭터 생성 창 이미지와 rect를 재사용한다.
    slots.append(('CharSel_RenamePanel', (354.5, 250.0, 571.0, 131.0),
                  'UI/ClassSelect/Common/CreateCharacterModalPanel.png'))
    slots.append(('CharSel_RenameTextBox', (562.0, 310.0, 156.0, 28.0),
                  'UI/ClassSelect/Common/CreateCharacterModalTextBox.png'))
    slots.append(('CharSel_RenameConfirm', (530.0, 365.0, 100.0, 34.0),
                  'UI/ClassSelect/Common/NormalButton.png'))
    slots.append(('CharSel_RenameCancel', (650.0, 365.0, 100.0, 34.0),
                  'UI/ClassSelect/Common/NormalButton.png'))
    return slots


# 이미 있는 ID는 건드리지 않고 새 slot만 배열 끝에 넣는다. 파싱 성공 후 원래 행끝으로 저장한다.
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
    # 이미지 저장이 먼저다. 뒤의 layout 추가 실패가 앞서 저장한 PNG를 되돌리지는 않는다.
    build_art(args.pages, args.repo / 'Client' / 'Bin' / 'Resources')
    append_slots(args.repo / LAYOUT)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
