"""Builds Data/UI/Customizing/CustomizingUI.json and cuts its PNG payload.

Sources: the retail charactercustomizing.gfx placement trace (CharCustomFrame_Main, authored
at 1920x1080) for every rect, its own atlas pages charactercustomizing_i1 / _i1b6 for the
screen art, and the shared shareimagev2 pages for the components it references externally
(V2btn / V2step1List / sliderTrack_V2_01 / sliderThumb_V2 / V2tabBtn2).

Rects are written scaled to the 1280x720 reference resolution every other lostark.ui-layout
document in this project uses. Run:

    python -X utf8 Tools/CharacterCustomizing/build_customizing_ui.py
"""
import json
import os

from PIL import Image

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
TEX = "D:/\ub85c\uc544 \ub9ac\uc18c\uc2a4/\ud14d\uc2a4\uccd0,\uba54\uc26c"
CC = TEX + "/EFUI_CHARACTERCUSTOMIZING/"
SH = TEX + "/EFUI_SHAREIMAGE/"
OUT_PNG = os.path.join(ROOT, "Client/Bin/Resources/UI/Customizing")
OUT_JSON = os.path.join(ROOT, "Data/UI/Customizing/CustomizingUI.json")
SCALE = 2.0 / 3.0

I1 = CC + "charactercustomizing_i1.dds"
I1B6 = CC + "charactercustomizing_i1b6.dds"
I10B = CC + "charactercustomizing_i10b.dds"
V2IE = SH + "shareimagev2_ie.png"
V2IB = SH + "shareimagev2_ib.png"
V2I2 = SH + "shareimagev2_i2.png"
V2I46 = SH + "shareimagev2_i46.png"
V2I6 = SH + "shareimagev2_i6.png"
# sliderTrack_V2_01 is the one component whose bitmap lives on the components atlas,
# not the shared shareimagev2 pages (see the componentsv2 trace).
CMP = TEX + "/EFUI_COMPONENTS/componentsv2_i1.png"

_pages = {}
_cuts = {}


def _page(path):
    if path not in _pages:
        _pages[path] = Image.open(path).convert("RGBA")
    return _pages[path]


def cut(name, path, x1, y1, x2, y2):
    """Crops the region once and returns its Resources-relative asset id."""
    if name not in _cuts:
        _page(path).crop((x1, y1, x2, y2)).save(os.path.join(OUT_PNG, name + ".png"))
        _cuts[name] = (x2 - x1, y2 - y1)
    return "UI/Customizing/" + name + ".png"


slots = []


def slot(sid, asset, x, y, w, h, hover=None):
    """x/y/w/h are gfx units at 1920x1080; stored at the 1280x720 reference resolution."""
    slots.append({
        "id": sid,
        "ownerClass": None,
        "type": 0,
        "rect": {"x": round(x * SCALE, 6), "y": round(y * SCALE, 6),
                 "width": round(w * SCALE, 6), "height": round(h * SCALE, 6)},
        "rotation": 0,
        "stages": {"baseFrom": 0, "shineFrom": 1},
        "layers": [{"path": asset, "hoverPath": hover, "tint": [1, 1, 1, 1],
                    "additive": False, "flipX": False}],
        "shine": {"texture": None, "additive": False},
        "animation": {"fps": 10, "scale": 1, "offset": {"x": 0, "y": 0},
                      "frames": [], "loop": True, "additive": False},
    })


# ---- shared components -------------------------------------------------------------------
BTN_N = cut("v2_btn_normal", V2I46, 643, 988, 746, 1024)
BTN_O = cut("v2_btn_over", V2I46, 328, 988, 431, 1024)
FOLD_N = cut("v2_fold_normal", V2I2, 618, 947, 827, 984)
FOLD_ARROW = cut("v2_fold_arrow", V2IB, 866, 973, 886, 989)
DIVISION = cut("v2_division", V2IB, 745, 992, 891, 997)
TRACK = cut("v2_slider_track", CMP, 352, 678, 503, 686)
THUMB_N = cut("v2_slider_thumb", V2IE, 286, 426, 303, 456)
THUMB_O = cut("v2_slider_thumb_over", V2IE, 267, 426, 284, 456)
TAB2_N = cut("v2_tab2_normal", V2IE, 785, 287, 867, 320)
TAB2_O = cut("v2_tab2_over", V2IE, 617, 287, 699, 320)

# ---- left panel: CharCustom_LeftPanel @ (0,0) ---------------------------------------------
slot("CC_LeftBg", cut("left_bg", I10B, 0, 0, 402, 1080), 0, 0, 402, 1080)
# leftResetButton sits on the panel head at (245,36) scaled 1.359 horizontally.
slot("CC_LeftResetBtn", BTN_N, 245, 36, 103 * 1.359, 36, hover=BTN_O)

# CharCustom_LeftRenderer is a 70x70 cell wearing the shared V2Slot_border frame. The three
# rolling lists sit at leftDressList (37,77), leftActionList (37,197) and, inside
# bottomPage (33,426), leftCustomizeList (2,151).
SLOT_BORDER = cut("v2_slot_border", V2I6, 863, 555, 933, 625)
CELL_PLATE = cut("left_cell", I1, 780, 458, 850, 528)


def cell(slot_id, x, y, size=70):
    """One CharCustom_LeftRenderer cell: plate, then the shared border over it."""
    slot(slot_id + "_Plate", CELL_PLATE, x, y, size, size)
    slot(slot_id, SLOT_BORDER, x, y, size, size)
for i in range(5):
    cell("CC_LeftDress%d" % i, 37 + 71 * i, 77)
for i in range(5):
    cell("CC_LeftAction%d" % i, 37 + 71 * i, 197)
BOTTOM_PAGE_Y = 296
for i in range(6):
    cell("CC_LeftSave%d" % i, 33 + 2 + 59 * i, BOTTOM_PAGE_Y + 151, 56)

# bottomPage @ (33,296) carries the save/load pair at (8,237)/(181,237), scaled 1.5922
# horizontally. The recommended-style row above them is account content this project has no
# source for, so it is left out rather than shown as empty frames.
BOTTOM_X, BOTTOM_Y = 33, BOTTOM_PAGE_Y
slot("CC_LeftSaveBtn", BTN_N, BOTTOM_X + 8, BOTTOM_Y + 237, 103 * 1.5922, 36, hover=BTN_O)
slot("CC_LeftLoadBtn", BTN_N, BOTTOM_X + 181, BOTTOM_Y + 237, 103 * 1.5922, 36, hover=BTN_O)

# ---- top panel: CharCustom_TopPanel @ (563,-1) --------------------------------------------
slot("CC_TopBannerA", cut("top_banner_a", I1, 404, 294, 968, 374), 563, -1, 564, 80)
slot("CC_TopBannerB", cut("top_banner_b", I1, 404, 376, 946, 456), 572, -1, 542, 80)
slot("CC_TopGuideBg", cut("top_guide_bg", I1, 404, 679, 970, 730), 563, 72, 566, 51)

# ---- right panel: CharCustom_RightPanel @ (1920,0), children at negative x -----------------
PANEL_X = 1920 - 437  # every tab page sits at (-437,192) inside the panel
slot("CC_RightBg", cut("right_bg", I1B6, 0, 0, 447, 1080), 1920 - 445, 0, 447, 1080)

# rightTabList @ (-433,73); CharCustom_Right_TabListRenderer cells are 61 wide, art 60x79.
TAB_BG = cut("tab_bg", I1, 739, 172, 799, 251)
TAB_BG_SELECTED = cut("tab_bg_selected", I1, 677, 172, 737, 251)
TAB_GLOW = cut("tab_glow", I1, 728, 608, 785, 674)
TAB_ICONS = [  # iconsMc frame order == tab order: base, face, hair, eye, skin, adorn, voice
    ("tab_icon_base", 593, 608, 654, 677, 0, 0),
    ("tab_icon_face", 530, 608, 591, 677, -1, -2),
    ("tab_icon_hair", 467, 608, 528, 677, 0, -1),
    ("tab_icon_eye", 404, 608, 465, 677, 0, 3),
    ("tab_icon_skin", 915, 458, 976, 527, 0, -2),
    ("tab_icon_adorn", 852, 458, 913, 527, -1, 2),
    ("tab_icon_voice", 978, 458, 1017, 506, 11, 6),
]
for i, (name, x1, y1, x2, y2, ox, oy) in enumerate(TAB_ICONS):
    tx = 1920 - 433 + 61 * i
    slot("CC_Tab%d_Bg" % i, TAB_BG, tx, 73, 60, 79, hover=TAB_BG_SELECTED)
    slot("CC_Tab%d_Glow" % i, TAB_GLOW, tx + 1, 73 + 11, 57, 66)
    slot("CC_Tab%d_Icon" % i, cut(name, I1, x1, y1, x2, y2),
         tx + ox, 73 + 12 + oy, x2 - x1, y2 - y1)

# ---- face tab: CharCustom_Right_TabFace @ (-437,192) --------------------------------------
# The tab is an accordion of two CharCustom_CategoryCheckBox sections. As authored,
# defaultElement sits at (0,0) with its header at (30,9) collapsed, and detailElement follows
# at (0,48) with its own header at (30,8) and its content expanded. Expanding the first pushes
# the second down by its content height, which the view does at runtime.
DEFAULT_FOLD_X = PANEL_X + 30
DEFAULT_FOLD_Y = 192 + 9
slot("CC_FoldDefault_Bg", FOLD_N, DEFAULT_FOLD_X - 1, DEFAULT_FOLD_Y - 1, 209 * 1.7799, 37)
slot("CC_FoldDefault_Arrow", FOLD_ARROW, DEFAULT_FOLD_X + 178 * 1.7799, DEFAULT_FOLD_Y + 10,
     20, 16)

# defaultElement > content @ (30,52): a divider at (5,25), the preset list at (7,59) on the
# same 70x71 renderer the left column uses, and the random/reset pair at y=423.
DEFAULT_CONTENT_X = PANEL_X + 30
DEFAULT_CONTENT_Y = 192 + 52
slot("CC_DefaultDivision", DIVISION, DEFAULT_CONTENT_X + 5, DEFAULT_CONTENT_Y + 25,
     146 * 2.4726, 5)
PRESET_COLUMNS = 5
PRESET_ROWS = 5
for i in range(PRESET_COLUMNS * PRESET_ROWS):
    cell("CC_FacePreset%d" % i,
         DEFAULT_CONTENT_X + 7 + 71 * (i % PRESET_COLUMNS),
         DEFAULT_CONTENT_Y + 59 + 72 * (i // PRESET_COLUMNS))

FOLD_X = PANEL_X + 30
FOLD_Y = 192 + 48 + 8
slot("CC_FoldDetail_Bg", FOLD_N, FOLD_X - 1, FOLD_Y - 1, 209 * 1.7799, 37)
slot("CC_FoldDetail_Arrow", FOLD_ARROW, FOLD_X + 178 * 1.7799, FOLD_Y + 10, 20, 16)

CONTENT_X = PANEL_X + 34
CONTENT_Y = 192 + 48 + 51
slot("CC_DetailDivision", DIVISION, CONTENT_X, CONTENT_Y + 25, 146 * 2.4726, 5)

# rightTabFaceDetailList @ (0,34): CharCustom_Right_TabFaceDetailRenderer is 123x33 (V2tabBtn2
# scaled 1.5), three per row inside the ~370 wide content column.
for i in range(6):
    sx = CONTENT_X + 123 * (i % 3)
    sy = CONTENT_Y + 34 + 33 * (i // 3)
    slot("CC_FaceSub%d_Bg" % i, TAB2_N, sx, sy, 82 * 1.5, 33, hover=TAB2_O)
    slot("CC_FaceSub%d_Selected" % i, TAB2_O, sx, sy, 82 * 1.5, 33)

# Each part sprite sits at (0,118) in the content; its sliders are ARKDefaultSlider_V2 at
# x=157 scaled (1.3576,1.1364) -- a 151x8 track drawn at y=7 of the box, with a 17x30 thumb
# scaled a further (0.647,0.733).
PART_X = CONTENT_X
PART_Y = CONTENT_Y + 118
SX, SY = 1.3576, 1.1364
SLIDER_ROWS = [  # (slider id, y inside the part sprite) in retail row order
    "eye_scaleh", "eye_scalev", "eye_width", "eye_height", "eye_angle",
    "eyebrow_height", "eyebrow_angle",
    "cheekbone_pull", "cheekbone_height",
    "jaw_scale", "jaw_height", "jaw_pull", "jaw_angle",
    "nose_scale", "nose_height", "nose_length",
    "mouthlip_height",
]
ROW_Y = {
    "eye_scaleh": 26, "eye_scalev": 56, "eye_width": 119, "eye_height": 149, "eye_angle": 211,
    "eyebrow_height": 26, "eyebrow_angle": 56,
    "cheekbone_pull": 26, "cheekbone_height": 56,
    "jaw_scale": 26, "jaw_height": 56, "jaw_pull": 119, "jaw_angle": 149,
    "nose_scale": 26, "nose_height": 56, "nose_length": 86,
    "mouthlip_height": 26,
}
for sid in SLIDER_ROWS:
    y = PART_Y + ROW_Y[sid]
    slot("CC_Slider_%s_Track" % sid, TRACK, PART_X + 157, y + 7 * SY, 151 * SX, 8 * SY)
    slot("CC_Slider_%s_Thumb" % sid, THUMB_N, PART_X + 157, y,
         17 * 0.647 * SX, 30 * 0.733 * SY, hover=THUMB_O)

# rightTabFaceDefault content @ (30,52): random/reset buttons at (18,423)/(189,423) scaled 1.5631.
BUTTON_X = PANEL_X + 30
BUTTON_Y = 192 + 52 + 423
slot("CC_FaceRandomBtn", BTN_N, BUTTON_X + 18, BUTTON_Y, 103 * 1.5631, 36, hover=BTN_O)
slot("CC_FaceResetBtn", BTN_N, BUTTON_X + 189, BUTTON_Y, 103 * 1.5631, 36, hover=BTN_O)

# ---- bottom panel: CharCustom_BottomPanel @ (0,1080), children at negative y ---------------
CREATE_X, CREATE_Y = 660, 1080 - 175  # createOutGamePage
slot("CC_BottomBg", cut("bottom_bg", I1, 404, 536, 1003, 606), CREATE_X, CREATE_Y, 599, 70)
slot("CC_CreateBtn", BTN_N, CREATE_X + 174, CREATE_Y + 85,
     103 * 2.476, 36 * 1.528, hover=BTN_O)
slot("CC_BackIcon", cut("back_icon", I1, 621, 937, 662, 978),
     28 + 20, 1080 - 99 + 6, 41, 41,
     hover=cut("back_icon_over", I1, 578, 937, 619, 978))
slot("CC_ResetAllIcon", cut("resetall_icon", I1, 790, 804, 854, 868),
     1364, 988, 64, 64,
     hover=cut("resetall_icon_over", I1, 922, 804, 986, 868))

# ---- left panel mouse guide: leftPanel > bottomPage (33,426) > guideMouseMc (11,291) -------
GUIDE_X, GUIDE_Y = 33 + 11, BOTTOM_PAGE_Y + 291
slot("CC_GuideRotateIcon", cut("guide_rotate", I1, 972, 68, 1008, 134),
     GUIDE_X + 11, GUIDE_Y + 10, 36, 66)
slot("CC_GuideEyeTrackIcon", cut("guide_rotate", I1, 972, 68, 1008, 134),
     GUIDE_X + 11, GUIDE_Y + 80, 36, 66)
slot("CC_GuideZoomIcon", cut("guide_zoom", I1, 972, 0, 1008, 66),
     GUIDE_X + 11, GUIDE_Y + 149 + 17, 36, 66)


# ---- the remaining right tabs -------------------------------------------------------------
# Every tab page is placed at (-437,192) inside CharCustom_RightPanel, which itself sits at
# (1920,0) -- PANEL_X already carries that 1483, so the pages only add their own y. Positions
# below are each element's own placement inside its tab sprite, straight out of the trace.
TAB_X = PANEL_X
TAB_Y = 192
COLOR_SWATCH = cut("v2_color_swatch", I1, 588, 260, 629, 281)


def color_picker(slot_id, x, y):
    """CharCustom_ColorPickerButton: a 39x19 colour chip under a 41x21 frame."""
    slot(slot_id + "_Chip", CELL_PLATE, x + 1, y + 1, 39, 19)
    slot(slot_id, COLOR_SWATCH, x, y, 41, 21)


def tab_slider(slot_id, x, y):
    """ARKDefaultSlider_V2, the same 151x8 track and 17x30 thumb the face rows use."""
    slot(slot_id + "_Track", TRACK, x, y + 7 * SY, 151 * SX, 8 * SY)
    slot(slot_id + "_Thumb", THUMB_N, x, y, 17 * 0.647 * SX, 30 * 0.733 * SY, hover=THUMB_O)


def tile_list(prefix, x, y, columns, rows, pitch=71):
    for i in range(columns * rows):
        cell("%s%d" % (prefix, i), x + pitch * (i % columns), y + (pitch + 1) * (i // columns))


# -- 머리카락: rightTabHairNewList (36,64), rightTabHairPresetList (36,170),
#    rightTabHairSubTabList (35,472), then the default page's colour picker and two sliders.
slot("CC_HairDivision", DIVISION, TAB_X + 35, TAB_Y + 25, 146 * 2.4726, 5)
tile_list("CC_HairNew", TAB_X + 36, TAB_Y + 64, 5, 1)
tile_list("CC_HairShape", TAB_X + 36, TAB_Y + 170, 5, 4)
for i, sid in enumerate(("CC_HairSubBasic", "CC_HairSubTwoTone")):
    slot(sid + "_Bg", TAB2_N, TAB_X + 35 + 123 * i, TAB_Y + 472, 82 * 1.5, 33, hover=TAB2_O)
    slot(sid + "_Selected", TAB2_O, TAB_X + 35 + 123 * i, TAB_Y + 472, 82 * 1.5, 33)
color_picker("CC_HairColor", TAB_X + 353, TAB_Y + 472 + 33 + 8)
tab_slider("CC_Slider_hair_strength", TAB_X + 190, TAB_Y + 472 + 33 + 40)
tab_slider("CC_Slider_hair_range", TAB_X + 190, TAB_Y + 472 + 33 + 73)

# -- 눈: subTabList (35,72.9), then defaultPage @ (0,190) with its preset grid at (36,128),
#    iris picker (353,32), eye colour picker (354,-1) and the definition/scale sliders.
slot("CC_EyeDivision", DIVISION, TAB_X + 35, TAB_Y + 25, 146 * 2.4726, 5)
for i, sid in enumerate(("CC_EyeSubIris", "CC_EyeSubOdd")):
    slot(sid + "_Bg", TAB2_N, TAB_X + 35 + 123 * i, TAB_Y + 73, 82 * 1.5, 33, hover=TAB2_O)
    slot(sid + "_Selected", TAB2_O, TAB_X + 35 + 123 * i, TAB_Y + 73, 82 * 1.5, 33)
tab_slider("CC_Slider_eye_scale", TAB_X + 191, TAB_Y + 37)
EYE_PAGE_Y = TAB_Y + 190
color_picker("CC_EyeColor", TAB_X + 354, EYE_PAGE_Y - 1)
color_picker("CC_EyeIrisColor", TAB_X + 353, EYE_PAGE_Y + 32)
tab_slider("CC_Slider_eye_definition", TAB_X + 191, EYE_PAGE_Y + 61)
tile_list("CC_EyeIris", TAB_X + 36, EYE_PAGE_Y + 128, 5, 3)

# -- 피부: three sliders and one colour picker, no lists.
slot("CC_SkinDivision", DIVISION, TAB_X + 35, TAB_Y + 25, 146 * 2.5, 5)
color_picker("CC_SkinColor", TAB_X + 354, TAB_Y + 38)
tab_slider("CC_Slider_skin_age", TAB_X + 191, TAB_Y + 69)
tab_slider("CC_Slider_skin_shine", TAB_X + 191, TAB_Y + 135)
tab_slider("CC_Slider_skin_freckles", TAB_X + 191, TAB_Y + 167)

# -- 꾸미기: CharCustom_Right_TabAdornMakeUp's four sub-tabs at (0,36), each page a list at
#    (1,31) with its own slider at (157,212) and picker at (319,186); the eye-line page adds a
#    second pair at (157,275)/(319,249).
slot("CC_AdornDivision", DIVISION, TAB_X + 35, TAB_Y + 25, 146 * 2.4726, 5)
ADORN_SUBS = ("Lip", "Touch", "EyeLine", "EyeBrow")
for i, name in enumerate(ADORN_SUBS):
    sx = TAB_X + 35 + 123 * (i % 3)
    sy = TAB_Y + 36 + 33 * (i // 3)
    slot("CC_AdornSub%s_Bg" % name, TAB2_N, sx, sy, 82 * 1.5, 33, hover=TAB2_O)
    slot("CC_AdornSub%s_Selected" % name, TAB2_O, sx, sy, 82 * 1.5, 33)
ADORN_PAGE_Y = TAB_Y + 36 + 66 + 31
tile_list("CC_AdornItem", TAB_X + 36, ADORN_PAGE_Y, 5, 2)
tab_slider("CC_Slider_adorn_strength", TAB_X + 157, ADORN_PAGE_Y + 181)
color_picker("CC_AdornColor", TAB_X + 319, ADORN_PAGE_Y + 155)
tab_slider("CC_Slider_adorn_shadow", TAB_X + 157, ADORN_PAGE_Y + 244)
color_picker("CC_AdornShadowColor", TAB_X + 319, ADORN_PAGE_Y + 218)

document = {
    "schema": "lostark.ui-layout",
    "formatVersion": 1,
    "resolution": {"width": 1280, "height": 720},
    "classes": ["Default"],
    "slots": slots,
}
os.makedirs(OUT_PNG, exist_ok=True)
os.makedirs(os.path.dirname(OUT_JSON), exist_ok=True)
with open(OUT_JSON, "w", encoding="utf-8", newline="\n") as stream:
    json.dump(document, stream, ensure_ascii=False, indent=2)
print("slots %d, images %d -> %s" % (len(slots), len(_cuts), OUT_JSON))
