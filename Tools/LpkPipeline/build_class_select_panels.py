"""Cut the character-select side panels out of the retail document and write their
lostark.ui-layout document.

Source: EFUI_CREATECHARACTER (createcharacter.gfx), a 1920x1080 stage that places
  createCharacterSelectedInfoFrame  CreateCharacterClassInfoFrame  at (0, 0)
  createCharacterSelectFrame        CreateCharacterSelectFrame     at (1240, 0)
so the left info panel and the right class list are the two panels this builds.

Nothing here is a hand-placed number.  `walk` follows the sprite tree from those two
roots, accumulating each PlaceObject matrix, and every shape it reaches contributes one
layer per bitmap fill at that fill's own matrix offset, sized by the DefineSubImage rect
it samples.  The stage is exactly 1.5x the 1280x720 reference every runtime UI uses, so
the only conversion applied is that divide.

The per-class art is not in this document.  The panels leave holes their host fills:
  classImg          ARKIcon_Class_Thumbnail   the 396x374 portrait, IconInfo ClassSelectImg_Big_<PrimaryKey>
  classSymbol_mc    ClassSymbol_CharInfo      one 83x83 rect per class on the shareImageV2 atlas
  identitySymbolMc  PlayingClassIdentity      one emblem per class on the same atlas
  tagTileList       one CreateCharacterClassInfoTagItem per ClassTag, Tag_Icon atlas cell
componentsv2.gfx binds an export name to each class frame of the symbol and emblem
sprites, so those bindings are read rather than guessed; EFTable_PCPreview supplies the
tag ids, difficulty and the message keys the strings come from.

Text is left to the host: the edit fields' rect, font, size and alignment are written to
the sidecar so CUILabelFont draws them, because baking a string into a picture would
freeze the language.

Inputs (already extracted on this machine)
  D:/ClaudeWork/Extracted/CustomizeGfx_Extracted/createcharacter/createcharacter.xml
  D:/ClaudeWork/Extracted/CustomizeGfx_Extracted/png/createcharacter_i*.png
  D:/ClaudeWork/Extracted/ShareImageGfx_Extracted/  (shareimagev2 structure + pages)
  D:/ClaudeWork/Extracted/ClassSelectPanels/atlas_C, atlas_T  (portrait + tag atlases)
  D:/ClaudeWork/Extracted/LpkTables/**/EFTable_PCPreview.db, EFTable_GameMsg.db
"""

from __future__ import annotations

import argparse
import json
import sqlite3
import xml.etree.ElementTree as ET
from pathlib import Path

from PIL import Image

EXTRACTED = Path(r"D:/ClaudeWork/Extracted")
GFX = EXTRACTED / "CustomizeGfx_Extracted/createcharacter/createcharacter.xml"
GFX_PAGES = EXTRACTED / "CustomizeGfx_Extracted/png"
SHARE = EXTRACTED / "ShareImageGfx_Extracted"
SHARE_PAGES = SHARE / "tex_v2/EFUI_SHAREIMAGE/Texture2D"
COMPONENTS = EXTRACTED / "SweepXml/componentsv2.xml"
PORTRAIT_PAGES = EXTRACTED / "ClassSelectPanels/atlas_C/EFUI_ICONATLAS_C/Texture2D"
TAG_PAGE = EXTRACTED / "ClassSelectPanels/atlas_T/EFUI_ICONATLAS_T/Texture2D/tag_icon_0.png"
TABLES = EXTRACTED / "LpkTables/PC/EFGame_Extra/ClientData/TableData"
GAMEMSG = EXTRACTED / "LpkTables/EFGame_Extra/ClientData/TableData/EFTable_GameMsg.db"

TWIPS = 20.0
STAGE_TO_REF = 1.0 / 1.5
ASSETS = "UI/ClassSelect/Source/"

# Root-timeline placements of the two panels, in stage px.
PANELS = [("Left", "CreateCharacterClassInfoFrame", 0.0, 0.0),
          ("Right", "CreateCharacterSelectFrame", 1240.0, 0.0)]

# EFTable_PC PrimaryKey -> the name this project knows the class by.  Gunlancer is the
# retail row name for 워로드 and Berserker_Female for 슬레이어, so the row cannot name these.
CLASSES = [(104, "Warlord"), (112, "Slayer"), (305, "LanceMaster"), (512, "Gunslinger"),
           (602, "Artist"), (612, "DimensionMaster"), (702, "GuardianKnight")]

# Which class each frame of the componentsv2 symbol/emblem sprites stands for.  The sprites
# share one class_<n> frame space; these are the seven this project plays.
CLASS_FRAME = {104: 9, 112: 32, 305: 25, 512: 30, 602: 23, 612: 17, 702: 38}
SYMBOL_SPRITE, IDENTITY_SPRITE = "1246", "942"

PORTRAIT_W, PORTRAIT_H, PORTRAIT_COLUMNS = 396, 374, 2
TAG_CELL, TAG_COLUMNS = 50, 20

# The panel's edit fields ship with placeholder text ("textField", an empty <p>), because the
# host fills them from EFTable_GameMsg at runtime.  These are the keys it fills them from.
# Each section is a header, a V2Line_HDivision under it, then the section's content, which is
# what ties a key to a field: title_difficulty sits above diffGraph, and button_identity is
# inside the identity sprite.  The tag section's header has no such tie -- no GameMsg row
# holds a bare "특징" -- so title_base_info is the closest candidate and is marked as one.
FIELD_LABELS = {
    "Left_classInfoMc_d8": ("sys.pccreate.title_difficulty", True),
    "Left_classInfoMc_d9": ("sys.pccreate.title_base_info", False),
    "Left_classInfoMc_identity_d2": ("sys.pccreate.button_identity", True),
    "Left_prev_btn_textField": ("sys.pccreate.button_back", True),
    "Left_setting_btn_textField": ("sys.pccreate.btn_class_preview", False),
    "Left_story_btn_textField": ("sys.pccreate.btn_class_story", True),
}
# The header colour is in the field's own authored HTML: <font color="#f3e4bc">.
HEADER_COLOUR = "#f3e4bc"

# The name the panel shows.  EFTable_PC's own Name column is the internal row name
# (Gunlancer, Berserker_Female), so the displayed one comes from the enum message instead.
DISPLAY_NAME_KEYS = {
    104: "tip.name.enum_playerclass_warlord",
    112: "tip.name.enum_playerclass_berserker_female",
    305: "tip.name.enum_playerclass_lance_master",
    512: "tip.name.enum_playerclass_devil_hunter_female",
    602: "tip.name.enum_playerclass_yinyangshi",
    612: "tip.name.enum_playerclass_dimension_master",
    702: "tip.name.enum_playerclass_dragon_knight",
}

# Where the left panel leaves its per-class holes, in that panel's own stage px.
# classImg carries no size of its own; its gradeIcon placeholder is a 26x26 square scaled
# by (15.231, 14.385), which is exactly the 396x374 the portrait atlas cell measures.
PORTRAIT_AT = (0.0, 23.0)
SYMBOL_AT, SYMBOL_SCALE = (19.0, 341.0), 0.831
# identity sits at (11, 696) and holds identitySymbolMc at (17, 49).
IDENTITY_AT = (28.0, 745.0)
# tagTileList at (29, 531) lays out CreateCharacterClassInfoTagItem, a 57x50 cell whose
# iconSlot draws a Tag_Icon atlas cell at scale 0.781.
TAG_AT, TAG_PITCH, TAG_DRAW = (29.0, 531.0), 57.0, TAG_CELL * 0.781

# The class list. bindClassList sits at (324, 93) inside the right panel's root, which the
# root timeline places at stage x 1240, and CreateCharacterSelectListItem measures 330x53
# with its symbol at (9, 5) scaled 0.618 and its label at (67, 18).
LIST_AT = (1240.0 + 324.0, 93.0)
ROW_SIZE = (330.0, 53.0)
ROW_SYMBOL_AT, ROW_SYMBOL_SCALE = (9.0, 5.0), 0.618
ROW_LABEL_AT = (67.0, 18.0)
# CreateCharacterDetailListItem's own thumbnail: a 26x26 gradeIcon placeholder scaled
# (5.231, 3.077), and a 129x38 name plate under it.
THUMB_SIZE, THUMB_PLATE = (136.0, 80.0), (129.0, 38.0)
# How many rows this project's list has. Retail groups its own class roster differently, so
# this count is ours; every measurement above is the source's.
LIST_ROWS = 6

# The six categories the list shows, as ClassSymbol_CharInfo ids on the shareImageV2 atlas.
# Specialist(F) was identified by matching the panel's own shipped CategorySymbol_*.png
# against all forty source crops; the same comparison put Warrior and Specialist(M) on the
# ids already confirmed by eye, which is what makes the Specialist(F) answer trustworthy.
CATEGORY_SYMBOLS = [("WarriorMale", 1), ("WarriorFemale", 33), ("MartialFemale", 3),
                    ("HunterFemale", 30), ("SpecialistFemale", 6), ("SpecialistMale", 40)]

# Row and thumbnail art, as sprite 9 and sprite 38 draw it, by the id `walk` gives each piece.
ROW_ART = {
    "Row_d2": "Row/Normal.png",
    "Row_d3_f11": "Row/Over.png",
    "Row_d3_f61": "Row/Selected.png",
    "Thumb_d7": "Row/ThumbPlate.png",
    "Thumb_d8_f11_d1": "Row/ThumbOver.png",
    "Thumb_d8_f61_d1": "Row/ThumbSelected.png",
}


# ---------------------------------------------------------------- gfx reading


class Movie:
    """The tags of one .gfx, indexed the four ways this builder looks things up."""

    def __init__(self, path: Path):
        root = ET.parse(path).getroot()
        self.subimages, self.images, self.shapes, self.sprites = {}, {}, {}, {}
        self.texts, self.by_name, self.grids = {}, {}, {}
        for tag in root.iter():
            kind = tag.get("type") or ""
            if kind == "DefineScalingGridTag":
                rect = next((child for child in tag if child.get("type") == "RECT"), None)
                if rect is not None:
                    self.grids[tag.get("characterId")] = tuple(
                        int(rect.get(key)) / TWIPS
                        for key in ("Xmin", "Ymin", "Xmax", "Ymax"))
                continue
            if kind == "DefineSubImage":
                self.subimages[tag.get("characterID")] = tag
            elif kind == "DefineExternalImage2":
                self.images[tag.get("imageID")] = tag.get("fileName")
            elif kind.startswith("DefineShape"):
                self.shapes[tag.get("shapeId")] = tag
            elif kind == "DefineSpriteTag":
                self.sprites[tag.get("spriteId")] = tag
            elif kind == "DefineEditTextTag":
                self.texts[tag.get("characterID")] = tag
            elif kind == "SymbolClassTag":
                ids = [item.text for item in tag.find("tags")]
                names = [item.text for item in tag.find("names")]
                self.by_name.update({name: cid for cid, name in zip(ids, names)})

    def rect(self, bitmap):
        """The atlas page and rect a bitmap id samples, or None if it is not an image."""
        sub = self.subimages.get(bitmap)
        if sub is None:
            return None
        page = self.images.get(sub.get("imageId"))
        if page is None:
            return None
        box = tuple(int(sub.get(key)) for key in ("x1", "y1", "x2", "y2"))
        return Path(page).stem.lower(), box


def offset(matrix):
    if matrix is None:
        return 0.0, 0.0
    return (int(matrix.get("translateX", "0")) / TWIPS,
            int(matrix.get("translateY", "0")) / TWIPS)


def scale_of(matrix):
    if matrix is None or matrix.get("hasScale") != "true":
        return 1.0, 1.0
    return float(matrix.get("scaleX", "1")), float(matrix.get("scaleY", "1"))


def walk(movie: Movie, character, x, y, sx, sy, label, out, texts, depth=0, grid=None):
    """Collect every bitmap and edit field a character draws, in stage coordinates."""
    if depth > 6:
        return
    shape = movie.shapes.get(character)
    if shape is not None:
        index = 0
        for fill in shape.iter():
            bitmap = fill.get("bitmapId")
            if not bitmap or bitmap == "65535":
                continue
            found = movie.rect(bitmap)
            if found is None:
                continue
            page, (x1, y1, x2, y2) = found
            fx, fy = offset(fill.find("bitmapMatrix"))
            index += 1
            out.append({"id": label if index == 1 else "%s_%d" % (label, index),
                        "page": page, "box": (x1, y1, x2, y2),
                        "x": x + fx * sx, "y": y + fy * sy,
                        "w": (x2 - x1) * sx, "h": (y2 - y1) * sy,
                        # The enclosing sprite's DefineScalingGrid, if it has one: the art is
                        # meant to be stretched with its corners held, not scaled whole.
                        "grid": grid if index == 1 else None})
        return
    text = movie.texts.get(character)
    if text is not None:
        bounds = text.find("bounds")
        width = height = 0.0
        if bounds is not None:
            width = (int(bounds.get("Xmax")) - int(bounds.get("Xmin"))) / TWIPS
            height = (int(bounds.get("Ymax")) - int(bounds.get("Ymin"))) / TWIPS
        colour = text.find("textColor")
        texts.append({"id": label, "x": x, "y": y, "width": width * sx, "height": height * sy,
                      "font": text.get("fontClass") or text.get("fontId"),
                      "sizePx": int(text.get("fontHeight", "0")) // 20,
                      "align": int(text.get("align") or 0),
                      "colour": [int(colour.get(channel)) for channel in
                                 ("red", "green", "blue", "alpha")] if colour is not None
                      else [255, 255, 255, 255],
                      "wordWrap": text.get("wordWrap") == "true",
                      "multiline": text.get("multiline") == "true",
                      # Extra space between lines, in stage px; negative tightens them.
                      "leading": int(text.get("leading") or 0) / TWIPS})
        return
    sprite = movie.sprites.get(character)
    if sprite is None:
        return
    frame = 1
    for child in sprite.iter():
        kind = child.get("type")
        if kind == "ShowFrameTag":
            frame += 1
            continue
        if kind not in ("PlaceObject2Tag", "PlaceObject3Tag"):
            continue
        inner = child.get("characterId")
        if not inner or inner == "0":
            continue
        matrix = child.find("matrix")
        dx, dy = offset(matrix)
        csx, csy = scale_of(matrix)
        name = child.get("name") or "d%s" % child.get("depth")
        if frame > 1:
            name = "%s_f%d" % (name, frame)
        walk(movie, inner, x + dx * sx, y + dy * sy, sx * csx, sy * csy,
             "%s_%s" % (label, name), out, texts, depth + 1,
             movie.grids.get(inner, grid))


def class_bindings(sprite_id):
    """frame index -> the export name componentsv2 places on that frame."""
    root = ET.parse(COMPONENTS).getroot()
    for tag in root.iter():
        if tag.get("type") != "DefineSpriteTag" or tag.get("spriteId") != sprite_id:
            continue
        bindings, frame, label = {}, 1, None
        for child in tag.iter():
            kind = child.get("type")
            if kind == "FrameLabelTag":
                label = child.get("name")
            elif kind in ("PlaceObject2Tag", "PlaceObject3Tag") and child.get("className"):
                if label and label.startswith("class_"):
                    bindings[int(label.split("_")[1])] = child.get("className")
            elif kind == "ShowFrameTag":
                frame += 1
        return bindings
    return {}


# ---------------------------------------------------------------- atlases


def share_exports():
    """Exported name -> character id in shareImageV2."""
    exports = {}
    for line in (SHARE / "symbols_v2.txt").read_text(encoding="utf-8", errors="replace").splitlines():
        parts = line.split()
        if len(parts) >= 2 and parts[0].isdigit():
            exports[parts[1]] = parts[0]
    return exports


def share_rect(movie: Movie, character, depth=0):
    """Follow a shareImageV2 export down to the DefineSubImage it draws."""
    if character in movie.subimages:
        return movie.rect(character)
    sprite = movie.sprites.get(character)
    if sprite is None or depth > 3:
        return None
    for child in sprite.iter():
        if child.get("type") in ("PlaceObject2Tag", "PlaceObject3Tag"):
            inner = child.get("characterId")
            if inner and inner != "0":
                found = share_rect(movie, inner, depth + 1)
                if found:
                    return found
    return None


def portrait_cells():
    """PrimaryKey -> (page number, cell index) from IconInfo's ClassSelectImg_Big rows."""
    lines = (EXTRACTED / "ClassSelectPanels/iconinfo.txt").read_text(
        encoding="utf-8", errors="replace").splitlines()
    pages = {}
    for index, line in enumerate(lines):
        name = line.strip()
        if not (name.startswith("ClassSelectImg_Big_") and name.endswith(".png")):
            continue
        key = int(name[len("ClassSelectImg_Big_"):-len(".png")])
        for probe in range(index + 1, min(index + 4, len(lines))):
            page = lines[probe].strip()
            if page.startswith("ClassSelectImg_Big_"):
                pages.setdefault(int(page.rsplit("_", 1)[1]), []).append(key)
                break
    cells = {}
    for page, keys in pages.items():
        for cell, key in enumerate(sorted(keys)):
            cells[key] = (page, cell)
    return cells


# ---------------------------------------------------------------- tables


def class_rows():
    preview = sqlite3.connect(TABLES / "EFTable_PCPreview.db")
    messages = sqlite3.connect(GAMEMSG)
    messages.text_factory = bytes

    def msg(key):
        if not key:
            return ""
        row = messages.execute("select MSG from GameMsg where lower(KEY)=lower(?)",
                               (key,)).fetchone()
        return row[0].decode("utf-8", "replace") if row else ""

    labels = {}
    for field, (key, certain) in FIELD_LABELS.items():
        labels[field] = {"messageKey": key, "text": msg(key), "colour": HEADER_COLOUR,
                         "fromSource": certain}

    rows = {}
    for key, name in CLASSES:
        found = preview.execute(
            "select ClassDesc, IdentityDesc1, WeaponDesc, ClassDifficulty,"
            " ClassTag1, ClassTag2, ClassTag3, ClassTag4, ClassTag5"
            " from PCPreview where PrimaryKey=?", (key,)).fetchone()
        rows[name] = {
            "primaryKey": key,
            "displayName": msg(DISPLAY_NAME_KEYS[key]),
            "classDescription": msg(found[0]),
            "identityDescription": msg(found[1]),
            "weaponDescription": msg(found[2]),
            "difficulty": found[3],
            "tags": [tag for tag in found[4:] if tag],
        }
    return rows, labels


def tag_rows():
    table = sqlite3.connect(TABLES / "EFTable_PCPreviewClassTagDescription.db")
    messages = sqlite3.connect(GAMEMSG)
    messages.text_factory = bytes
    rows = {}
    for key, index, tooltip in table.execute(
            "select PrimaryKey, IconIndex, ClassTagTooltip from PCPreviewClassTagDescription"):
        found = messages.execute("select MSG from GameMsg where lower(KEY)=lower(?)",
                                 (tooltip,)).fetchone()
        text = found[0].decode("utf-8", "replace") if found else ""
        # The row is one <img> tag, then "<name><br><sentence>".
        body = text.split("</img>")[-1]
        name, _, detail = body.partition("<br>")
        rows[key] = {"iconIndex": index, "name": name.strip(), "detail": detail.strip()}
    return rows


# ---------------------------------------------------------------- layout


def layout_slot(slot_id, x, y, w, h, path, owner=None):
    return {
        "id": slot_id, "ownerClass": owner, "type": 0,
        "rect": {"x": round(x, 3), "y": round(y, 3),
                 "width": round(w, 3), "height": round(h, 3)},
        "rotation": 0, "stages": {"baseFrom": 0, "shineFrom": 1},
        "layers": [{"path": path, "hoverPath": None, "tint": [1, 1, 1, 1],
                    "additive": False, "flipX": False}],
        "shine": {"texture": None, "additive": False},
        "animation": {"fps": 10, "scale": 1, "offset": {"x": 0, "y": 0},
                      "frames": [], "loop": True, "additive": False},
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo", type=Path, default=Path(__file__).resolve().parents[2])
    args = parser.parse_args()
    art = args.repo / "Client/Bin/Resources/UI/ClassSelect/Source"
    data = args.repo / "Data/UI/ClassSelect"
    for folder in ("Panel", "Portrait", "Symbol", "Identity", "Tag", "Row"):
        (art / folder).mkdir(parents=True, exist_ok=True)
    data.mkdir(parents=True, exist_ok=True)

    movie = Movie(GFX)
    pages, written = {}, 0

    def save(page_stem, box, target, source_dir=GFX_PAGES):
        nonlocal written
        path = source_dir / (page_stem + ".png")
        if path not in pages:
            pages[path] = Image.open(path).convert("RGBA")
        crop = pages[path].crop(box)
        if crop.getchannel("A").getbbox() is None:
            print("  ! 투명한 조각:", target)
            return False
        crop.save(art / target)
        written += 1
        return True

    # --- panel chrome, straight off the two root placements
    def ref(piece_x, piece_y, piece_w, piece_h, slot_id, path, owner=None):
        return layout_slot(slot_id, piece_x * STAGE_TO_REF, piece_y * STAGE_TO_REF,
                           piece_w * STAGE_TO_REF, piece_h * STAGE_TO_REF, path, owner)

    slots, texts, difficulty_art = [], [], {}
    for label, export, px, py in PANELS:
        character = movie.by_name.get(export)
        if character is None:
            print("  ! %s 를 찾지 못함" % export)
            return 1
        drawn = []
        walk(movie, character, px, py, 1.0, 1.0, label, drawn, texts)
        for piece in drawn:
            # A piece with a scaling grid is art the host resizes: the retail panel grows to
            # whatever the class list needs. There is no nine-slice in this engine, so the
            # band the grid marks as stretchable is cut out as its own slot and the caps
            # above and below it become slots of their own -- three ordinary sprites the
            # view repositions, rather than one picture scaled out of shape.
            if piece["grid"] is not None:
                _, top, _, bottom = piece["grid"]
                x1, y1, x2, y2 = piece["box"]
                height = y2 - y1
                if 0.0 < top < bottom < height:
                    bands = (("Top", 0.0, top), ("Middle", top, bottom),
                             ("Bottom", bottom, height))
                    for band, band_top, band_bottom in bands:
                        name = "Panel/%s_%s.png" % (piece["id"], band)
                        box = (x1, int(y1 + band_top), x2, int(y1 + band_bottom))
                        if not save(piece["page"], box, name):
                            continue
                        slots.append(ref(piece["x"], piece["y"] + band_top,
                                         piece["w"], band_bottom - band_top,
                                         "%s_%s" % (piece["id"], band), ASSETS + name))
                    continue
            name = "Panel/%s.png" % piece["id"]
            if not save(piece["page"], piece["box"], name):
                continue
            # The difficulty bar keeps one fill per level on frames 2..6; the buttons keep
            # their over/down/disabled art on later frames.  Neither is a standing slot.
            if "diffGraph_d2_f" in piece["id"]:
                difficulty_art[int(piece["id"].rsplit("_f", 1)[1]) - 1] = (piece, name)
                continue
            if any(part[:1] == "f" and part[1:].isdigit() for part in piece["id"].split("_")):
                continue
            slots.append(ref(piece["x"], piece["y"], piece["w"], piece["h"],
                             piece["id"], ASSETS + name))
    print("패널 조각 %d개 (난이도 채움 %d단계 별도)" % (len(slots), len(difficulty_art)))

    # --- per-class art the panels leave to their host
    share = Movie(SHARE / "shareimagev2_structure.xml")
    exports = share_exports()
    symbols = class_bindings(SYMBOL_SPRITE)
    emblems = class_bindings(IDENTITY_SPRITE)
    cells = portrait_cells()
    rows, labels = class_rows()
    tags = tag_rows()

    for key, name in CLASSES:
        frame = CLASS_FRAME[key]
        for kind, bindings in (("Symbol", symbols), ("Identity", emblems)):
            export = bindings.get(frame)
            if export is None:
                print("  ! %s %s: class_%d 바인딩 없음" % (name, kind, frame))
                continue
            found = share_rect(share, exports.get(export))
            if found is None:
                print("  ! %s %s: %s 해결 실패" % (name, kind, export))
                continue
            page, box = found
            asset = "%s/%s.png" % (kind, name)
            save(page, box, asset, SHARE_PAGES)
            rows[name].setdefault("sources", {})[kind.lower()] = export
            width, height = box[2] - box[0], box[3] - box[1]
            if kind == "Symbol":
                slots.append(ref(SYMBOL_AT[0], SYMBOL_AT[1],
                                 width * SYMBOL_SCALE, height * SYMBOL_SCALE,
                                 "%s_Symbol" % name, ASSETS + asset, name))
            else:
                slots.append(ref(IDENTITY_AT[0], IDENTITY_AT[1], width, height,
                                 "%s_Identity" % name, ASSETS + asset, name))

        page, cell = cells[key]
        left = (cell % PORTRAIT_COLUMNS) * PORTRAIT_W
        top = (cell // PORTRAIT_COLUMNS) * PORTRAIT_H
        save("classselectimg_big_%d" % page,
             (left, top, left + PORTRAIT_W, top + PORTRAIT_H),
             "Portrait/%s.png" % name, PORTRAIT_PAGES)
        slots.append(ref(PORTRAIT_AT[0], PORTRAIT_AT[1], PORTRAIT_W, PORTRAIT_H,
                         "%s_Portrait" % name, ASSETS + "Portrait/%s.png" % name, name))

        level = rows[name]["difficulty"]
        if level in difficulty_art:
            piece, asset = difficulty_art[level]
            slots.append(ref(piece["x"], piece["y"], piece["w"], piece["h"],
                             "%s_Difficulty" % name, ASSETS + asset, name))
        else:
            print("  ! %s: 난이도 %s 채움 이미지 없음" % (name, level))
        for index, tag in enumerate(rows[name]["tags"]):
            slots.append(ref(TAG_AT[0] + index * TAG_PITCH, TAG_AT[1], TAG_DRAW, TAG_DRAW,
                             "%s_Tag_%d" % (name, index + 1),
                             ASSETS + "Tag/Tag_%02d.png" % tag, name))

    # --- the class list: the six category symbols, the row art, and a slot per row
    for label, symbol_id in CATEGORY_SYMBOLS:
        export = "ClassSymbol_CharInfo_%d" % symbol_id
        found = share_rect(share, exports.get(export))
        if found is None:
            print("  ! 카테고리 심볼 %s(%d) 해결 실패" % (label, symbol_id))
            continue
        page, box = found
        save(page, box, "Symbol/Category_%s.png" % label, SHARE_PAGES)

    row_art, thumb_art = {}, {}
    for sprite_id, prefix, store in (("9", "Row", row_art), ("38", "Thumb", thumb_art)):
        pieces = []
        walk(movie, sprite_id, 0.0, 0.0, 1.0, 1.0, prefix, pieces, [])
        for piece in pieces:
            target = ROW_ART.get(piece["id"])
            if target is None:
                continue
            if save(piece["page"], piece["box"], target):
                store[target] = (piece["w"], piece["h"])

    row_width, row_height = (value * STAGE_TO_REF for value in ROW_SIZE)
    for index in range(LIST_ROWS):
        # Only the first row's authored position is meaningful: the list is an accordion, so
        # Level_CharacterSelect moves each row every frame. These are its starting rects.
        top = LIST_AT[1] * STAGE_TO_REF + row_height * index
        slots.append(layout_slot("ClassRow%d" % index, LIST_AT[0] * STAGE_TO_REF, top,
                                 row_width, row_height, ASSETS + "Row/Normal.png"))
        symbol = ROW_SYMBOL_AT[0] * STAGE_TO_REF, ROW_SYMBOL_AT[1] * STAGE_TO_REF
        side = TAG_CELL * ROW_SYMBOL_SCALE * STAGE_TO_REF * (83.0 / TAG_CELL)
        slots.append(layout_slot(
            "ClassRowSymbol%d" % index, LIST_AT[0] * STAGE_TO_REF + symbol[0],
            top + symbol[1], side, side,
            ASSETS + "Symbol/Category_%s.png" % CATEGORY_SYMBOLS[0][0]))
    thumb_width, thumb_height = (value * STAGE_TO_REF for value in THUMB_SIZE)
    slots.append(layout_slot("ClassThumbFrame", LIST_AT[0] * STAGE_TO_REF,
                             LIST_AT[1] * STAGE_TO_REF, thumb_width, thumb_height,
                             ASSETS + "Row/ThumbSelected.png"))
    slots.append(layout_slot("ClassThumb", LIST_AT[0] * STAGE_TO_REF,
                             LIST_AT[1] * STAGE_TO_REF, thumb_width, thumb_height,
                             ASSETS + "Portrait/%s.png" % CLASSES[0][1]))
    slots.append(layout_slot(
        "ClassThumbPlate", LIST_AT[0] * STAGE_TO_REF, LIST_AT[1] * STAGE_TO_REF,
        THUMB_PLATE[0] * STAGE_TO_REF, THUMB_PLATE[1] * STAGE_TO_REF,
        ASSETS + "Row/ThumbPlate.png"))

    tag_page = Image.open(TAG_PAGE).convert("RGBA")
    for key, row in sorted(tags.items()):
        left = (row["iconIndex"] % TAG_COLUMNS) * TAG_CELL
        top = (row["iconIndex"] // TAG_COLUMNS) * TAG_CELL
        tag_page.crop((left, top, left + TAG_CELL, top + TAG_CELL)).save(
            art / ("Tag/Tag_%02d.png" % key))
        written += 1

    print("png %d개 -> %s" % (written, art))

    document = {"schema": "lostark.ui-layout", "formatVersion": 1,
                "resolution": {"width": 1280, "height": 720},
                "classes": ["Default"] + [name for _, name in CLASSES],
                "slots": slots}
    (data / "ClassSelectPanels_Layout.json").write_text(
        json.dumps(document, indent=1, ensure_ascii=False) + "\n", encoding="utf-8")

    sidecar = {"schema": "lostark.class-select-panels", "formatVersion": 1,
               "resolution": {"width": 1280, "height": 720},
               "stageScale": STAGE_TO_REF,
               "textFields": [dict(field,
                                   x=round(field["x"] * STAGE_TO_REF, 3),
                                   y=round(field["y"] * STAGE_TO_REF, 3),
                                   width=round(field["width"] * STAGE_TO_REF, 3),
                                   height=round(field["height"] * STAGE_TO_REF, 3),
                                   sizePx=round(field["sizePx"] * STAGE_TO_REF, 2),
                                   leading=round(field["leading"] * STAGE_TO_REF, 2))
                              for field in texts],
               "labels": labels, "tags": tags, "classes": rows}
    (data / "ClassSelectPanels_Content.json").write_text(
        json.dumps(sidecar, indent=1, ensure_ascii=False) + "\n", encoding="utf-8")
    print("레이아웃 %d슬롯, 텍스트 %d칸, 태그 %d종, 직업 %d종"
          % (len(slots), len(texts), len(tags), len(rows)))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
