"""chatting.gfx의 atlas를 잘라 CChatWindowView용 이미지와 layout JSON을 만든다.
원본 sprite 340의 1920×1080 좌표를 1280×720 기준으로 변환한다.
출력은 Resources/UI/Chat 이미지와 Data/UI/Chat/ChatWindow_Layout.json이다.
"""

from __future__ import annotations

# --repo와 입력 atlas --page를 받는다.
import argparse
# CUILayoutRuntime이 읽는 slot·rect·layer 문서를 저장한다.
import json
# 저장소 기준으로 이미지와 JSON 출력 경로를 만든다.
from pathlib import Path

# atlas 잘라내기와 배경 PNG 생성을 담당한다.
from PIL import Image

STAGE_TO_REF = 2.0 / 3.0
# 원본이 작게 보여 좌하단 anchor 기준으로 위치·크기를 함께 확대한다. 1이면 원본 크기다.
WIDGET_SCALE = 1.25
# 화면 하단에 놓인 원본 anchor. 자식의 local y는 위쪽으로 음수다.
WIDGET_STAGE = (1.0, 1080.0)
TAB_PITCH = 96.0

# 이미지 이름별 chatting_i1 atlas의 crop 범위. 원본 DefineSubImage 좌표를 사용한다.
CROPS = {
    "Chat_LogPanelArt": (0, 0, 378, 227),
    "Chat_InputBoxOuter": (554, 37, 849, 69),
    "Chat_InputBoxInner": (554, 71, 846, 101),
    "Chat_ComboBg": (851, 37, 944, 65),
    "Chat_ComboArrow": (458, 175, 483, 199),
    "Chat_LangKorean": (962, 175, 977, 189),
    "Chat_ScrollButton": (910, 175, 928, 191),
    "Chat_ScrollArrowUp": (396, 202, 410, 216),
    "Chat_ScrollArrowDown": (460, 202, 474, 216),
    "Chat_LockButton": (917, 146, 941, 171),
    "Chat_OptionButton": (749, 146, 775, 171),
    "Chat_MinimizeButton": (665, 146, 691, 171),
    "Chat_TabBg": (842, 0, 936, 32),
    "Chat_TabAddBg": (938, 0, 967, 32),
    "Chat_TabAddPlus": (872, 175, 889, 192),
}
# 패널 이미지 아래에 깔리는 원본 20% 검정 배경이다.
PANEL = ("Chat_LogPanel", (0, 0, 0, 51), 377, 85)

# stable slot ID별 local 위치·크기·이미지·반전 여부. 이미지 None은 텍스트/표시용 영역이다.
SLOTS = [
    ("Chat_LogPanel", 0.0, -150.0, 377.0, 85.0, "Chat_LogPanel", False),
    ("Chat_LogPanelArt", 0.0, -149.0, 378.0, 84.9, "Chat_LogPanelArt", False),
    ("Chat_LogTextBox", 24.0, -148.0, 352.0, 83.0, None, False),
    ("Chat_ScrollUp", 3.0, -147.9, 18.0, 16.0, "Chat_ScrollButton", False),
    ("Chat_ScrollUpArrow", 3.0, -144.9, 14.0, 14.0, "Chat_ScrollArrowUp", False),
    ("Chat_ScrollDown", 3.0, -82.8, 18.0, 16.0, "Chat_ScrollButton", True),
    ("Chat_ScrollDownArrow", 3.0, -79.8, 14.0, 14.0, "Chat_ScrollArrowDown", False),
    # 입력바 (0.2,-39)에 combobox local (0,-25)를 합산한다.
    ("Chat_ComboBg", 0.2, -64.0, 101.0, 32.0, "Chat_ComboBg", False),
    ("Chat_ComboArrow", 1.2, -60.0, 25.0, 24.0, "Chat_ComboArrow", False),
    ("Chat_ChannelTextBox", 28.2, -60.0, 60.0, 23.0, None, False),
    # 두 입력 배경에 부모 위치 (93,8)와 배율 (0.9595,0.9211)을 적용한 값이다.
    ("Chat_InputBoxOuter", 93.2, -66.9, 285.0, 35.0, "Chat_InputBoxOuter", False),
    ("Chat_InputBoxInner", 97.0, -62.3, 280.2, 27.6, "Chat_InputBoxInner", False),
    ("Chat_InputTextBox", 110.2, -59.0, 225.0, 23.0, None, False),
    ("Chat_LangIcon", 351.2, -55.0, 15.0, 14.0, "Chat_LangKorean", False),
    ("Chat_LockButton", 299.0, -178.0, 24.0, 25.0, "Chat_LockButton", False),
    ("Chat_OptionButton", 323.0, -178.0, 26.0, 25.0, "Chat_OptionButton", False),
    ("Chat_MinimizeButton", 351.0, -177.0, 26.0, 25.0, "Chat_MinimizeButton", False),
    ("Chat_Tab0", 0.0, -180.0, 94.0, 32.0, "Chat_TabBg", False),
    ("Chat_Tab0TextBox", 0.0, -173.0, 94.0, 25.0, None, False),
    ("Chat_TabAdd", TAB_PITCH, -180.0, 29.0, 32.0, "Chat_TabAddBg", False),
    ("Chat_TabAddPlus", TAB_PITCH + 6.0, -172.0, 17.0, 17.0, "Chat_TabAddPlus", False),
]


# CUILayoutRuntime이 찾는 ID·기준 해상도 rect·Resources 상대 이미지 경로를 묶는다.
def layout_slot(slot_id, x, y, w, h, path, flip_y=False):
    return {
        "id": slot_id, "ownerClass": None, "type": 0,
        "rect": {"x": round(x, 2), "y": round(y, 2), "width": round(w, 2), "height": round(h, 2)},
        # 아래쪽 스크롤 버튼은 같은 이미지를 180도 회전해 사용한다.
        "rotation": 180 if flip_y else 0,
        "stages": {"baseFrom": 0, "shineFrom": 1 if path else 0},
        "layers": ([{"path": "UI/Chat/" + path + ".png", "hoverPath": None, "tint": [1, 1, 1, 1],
                     "additive": False, "flipX": False}] if path else []),
        "shine": {"texture": None, "additive": False},
        "animation": {"fps": 10, "scale": 1, "offset": {"x": 0, "y": 0},
                      "frames": [], "loop": True, "additive": False},
    }


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--repo", type=Path, default=Path(__file__).resolve().parents[2])
    ap.add_argument("--page", type=Path, required=True)
    args = ap.parse_args()

    # atlas 조각이 완전히 투명하면 중단한다. 이미 저장한 앞쪽 PNG는 되돌리지 않는다.
    out_dir = args.repo / "Client/Bin/Resources/UI/Chat"
    out_dir.mkdir(parents=True, exist_ok=True)
    page = Image.open(args.page).convert("RGBA")
    for name, (x1, y1, x2, y2) in CROPS.items():
        crop = page.crop((x1, y1, x2, y2))
        if crop.getchannel("A").getbbox() is None:
            print("  ! empty crop", name)
            return 1
        crop.save(out_dir / (name + ".png"))
    name, rgba, w, h = PANEL
    Image.new("RGBA", (w, h), rgba).save(out_dir / (name + ".png"))

    # 원본 anchor와 local 배율을 합산한 뒤 layout 기준 해상도로 환산한다.
    slots = []
    for slot_id, x, y, w, h, path, flip_y in SLOTS:
        sx = (WIDGET_STAGE[0] + x * WIDGET_SCALE) * STAGE_TO_REF
        sy = (WIDGET_STAGE[1] + y * WIDGET_SCALE) * STAGE_TO_REF
        slots.append(layout_slot(slot_id, sx, sy, w * WIDGET_SCALE * STAGE_TO_REF,
                                 h * WIDGET_SCALE * STAGE_TO_REF, path, flip_y))
    doc = {"schema": "lostark.ui-layout", "formatVersion": 1,
           "resolution": {"width": 1280, "height": 720}, "classes": ["Default"], "slots": slots}
    # 모든 slot을 작성한 뒤 JSON을 저장한다. 실행 중 UI의 자동 Reload는 수행하지 않는다.
    layout = args.repo / "Data/UI/Chat/ChatWindow_Layout.json"
    layout.write_text(json.dumps(doc, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f"wrote {len(CROPS) + 1} png -> {out_dir}")
    print(f"wrote {layout} ({len(slots)} slots)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
