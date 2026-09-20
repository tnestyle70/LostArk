"""Cut the retail interaction key prompt (EFUI_MASTERKEY) and write its ui-layout document for
CInteractKeyPromptView.

Retail source: EFUI_MASTERKEY (OVSG0A8WELO7UOKDY2YWI6.upk, masterkey.gfx), class
MasterKeyComponent (sprite 178), which the host shows over an interactable spot:

  iconType_mc (sprite 135) at (-25,-25) scale 0.7429 x 0.7536: one 70x69 action icon per frame
      label (godown, climb, singleLine, jump, check, npc, ... 62 labels), i.e. a ~52 px icon
      centred on the component origin.
  descriptionTF (LabelEx_YG760_2) at (-109,29): YG760 14 px, white, centred, black blur 2/3.
      The host fills it with GameMsg sys.tip.masterkey_string_combination
      "<P ALIGN='CENTER'><FONT SIZE='14'>{0} {1}</FONT></P>": {0} the action name
      (tip.name.interactionkey_<type>, e.g. godown = "down"), {1} the key as an inline
      emoticon image (EFUI_SHAREIMAGE Shared_GlobalInputDeviceKey_G).
  effectMc (sprite 177): "show" plays shapes 138/141/144/147/150 on frames 3..7 at 40 fps.
      Each shape is two bitmap fills centred on the origin: a 300x300 glow (136/139/...) at
      scale 1 and a 100x100 spark (137/140/...) at scale 1.5884 (translate -81.35,-81.45).
  key_lb / masterShortBGMc stay hidden in the single-key form (setProp visible=false, alpha 0).

Inputs (exported with umodel_lostark_v7 -export -dds -game=lostark -kr):
  --masterkey-pages  folder holding masterkey_i4.dds / masterkey_i4d.dds
  --shareicon-page   shareiconimage_i6 page (tga/dds/png) holding Shared_GlobalInputDeviceKey_G
                     (DefineSubImage 675: image 5 (990,580)-(1024,615))
Writes Client/Bin/Resources/UI/Interact/*.png and Data/UI/Interact/InteractKey_Layout.json.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from PIL import Image

# action -> (masterkey page, x1, y1, x2, y2) from the DefineSubImage of that icon frame
ICONS = {
    "godown": (792, 850, 862, 919),
    "climb": (144, 850, 214, 919),
    "singleLine": (216, 850, 286, 919),
    "check": None,  # resolved below from shape 118 / bitmap 117
}
# effect "show" frames: (glow sub-rect, spark sub-rect), both on masterkey_i4
SHOW_FRAMES = [
    ((302, 0, 602, 300), (510, 604, 610, 704)),
    ((604, 0, 904, 300), (604, 474, 704, 574)),
]
SPARK_SCALE = 31.767578 / 20.0
SPARK_OFFSET = (150.0 - 1627 / 20.0, 150.0 - 1629 / 20.0)  # spark top-left inside the 300 box
KEY_G = (990, 580, 1024, 615)

STAGE_TO_REF = 2.0 / 3.0
ICON_W = 70 * 0.7428589
ICON_H = 69 * 0.7536011


def layout_slot(slot_id, w, h, path):
    return {
        "id": slot_id, "ownerClass": None, "type": 0,
        "rect": {"x": 0, "y": 0, "width": w, "height": h},
        "rotation": 0, "stages": {"baseFrom": 0, "shineFrom": 1},
        "layers": [{"path": path, "hoverPath": None, "tint": [1, 1, 1, 1],
                    "additive": False, "flipX": False}],
        "shine": {"texture": None, "additive": False},
        "animation": {"fps": 10, "scale": 1, "offset": {"x": 0, "y": 0},
                      "frames": [], "loop": True, "additive": False},
    }


def find_page(folder: Path, stem: str) -> Image.Image:
    for ext in (".dds", ".png", ".tga"):
        p = folder / (stem + ext)
        if p.exists():
            return Image.open(p).convert("RGBA")
    raise FileNotFoundError(folder / stem)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--repo", type=Path, default=Path(__file__).resolve().parents[2])
    ap.add_argument("--masterkey-pages", type=Path, required=True)
    ap.add_argument("--masterkey-xml", type=Path, required=True,
                    help="ffdec -swf2xml of masterkey.gfx (resolves the remaining frames)")
    ap.add_argument("--shareicon-page", type=Path, required=True)
    args = ap.parse_args()

    import xml.etree.ElementTree as ET
    tags = ET.parse(args.masterkey_xml).getroot().find("tags")
    subs = {t.get("characterID"): (int(t.get("x1")), int(t.get("y1")), int(t.get("x2")), int(t.get("y2")))
            for t in tags if t.get("type") == "DefineSubImage" and t.get("imageId") == "0"}
    ICONS["check"] = subs["117"]
    show = []
    for glow_id, spark_id in (("136", "137"), ("139", "140"), ("142", "143"), ("145", "146"), ("148", "149")):
        show.append((subs[glow_id], subs[spark_id]))

    page = find_page(args.masterkey_pages, "masterkey_i4")
    art_out = args.repo / "Client/Bin/Resources/UI/Interact"
    data_out = args.repo / "Data/UI/Interact"
    art_out.mkdir(parents=True, exist_ok=True)
    data_out.mkdir(parents=True, exist_ok=True)

    for name, box in ICONS.items():
        page.crop(box).save(art_out / ("Icon_%s.png" % name))
    for i, (glow_box, spark_box) in enumerate(show):
        frame = Image.new("RGBA", (300, 300), (0, 0, 0, 0))
        frame.alpha_composite(page.crop(glow_box).resize((300, 300), Image.LANCZOS))
        spark = page.crop(spark_box)
        size = round(spark.size[0] * SPARK_SCALE)
        spark = spark.resize((size, size), Image.LANCZOS)
        frame.alpha_composite(spark, (round(SPARK_OFFSET[0]), round(SPARK_OFFSET[1])))
        frame.save(art_out / ("ShowFx_%d.png" % i))
    Image.open(args.shareicon_page).convert("RGBA").crop(KEY_G).save(art_out / "Key_G.png")

    s = STAGE_TO_REF
    a = "UI/Interact/"
    doc = {"schema": "lostark.ui-layout", "formatVersion": 1,
           "resolution": {"width": 1280, "height": 720}, "classes": ["Default"],
           "slots": [
               layout_slot("IKP_Fx", 300 * s, 300 * s, a + "ShowFx_0.png"),
               layout_slot("IKP_Icon", ICON_W * s, ICON_H * s, a + "Icon_godown.png"),
               layout_slot("IKP_Key", 18 * s, 18 * 35 / 34 * s, a + "Key_G.png"),
           ]}
    (data_out / "InteractKey_Layout.json").write_text(
        json.dumps(doc, indent=1, ensure_ascii=False) + "\n", encoding="utf-8")
    print("wrote", len(ICONS) + len(show) + 1, "png and InteractKey_Layout.json")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
