"""Fill in the class-select art the panel has no folder for.

Four of the seven playable classes ship UI/ClassSelect/<Class>/; Slayer, Gunslinger and
Guardian Knight do not, so their side panel draws nothing. This cuts the same five pieces the
existing folders hold out of the retail atlases, matching what each shipped file is:

  Illustration.png       ClassSelectImg_Big_<PrimaryKey>   the 396x374 portrait cell
  IllustrationSmall.png  ClassSelectImg_<PrimaryKey>       the 136x80 cell; the three newest
                                                           classes live on the _once page
  IdentitySymbol.png     ClassSymbol_CharInfo_<n>          the class crest on shareImageV2
  NameID.png             <code>_identity                   the identity emblem on the same
  SeparateBar.png        copied -- byte-identical in every existing folder
  Tag_01..28.png         copied -- byte-identical in every existing folder

componentsv2.gfx binds the crest and the emblem to each class frame itself, so neither is
guessed. It also writes the two category crests the class list still draws as nullptr.

Inputs (already extracted on this machine)
  D:/ClaudeWork/Extracted/ClassSelectPanels/atlas_C/...  ClassSelectImg pages
  D:/ClaudeWork/Extracted/ShareImageGfx_Extracted/       shareimagev2 structure + pages
"""

from __future__ import annotations

import argparse
import collections
import re
import shutil
import xml.etree.ElementTree as ET
from pathlib import Path

from PIL import Image

EXTRACTED = Path(r"D:/ClaudeWork/Extracted")
ICON_PAGES = EXTRACTED / "ClassSelectPanels/atlas_C/EFUI_ICONATLAS_C/Texture2D"
ICON_INFO = EXTRACTED / "ClassSelectPanels/iconinfo.txt"
SHARE = EXTRACTED / "ShareImageGfx_Extracted"
SHARE_PAGES = SHARE / "tex_v2/EFUI_SHAREIMAGE/Texture2D"
COMPONENTS = EXTRACTED / "SweepXml/componentsv2.xml"

# EFTable_PC PrimaryKey, the folder name, and the class_<n> frame componentsv2 binds its
# crest and emblem to.
MISSING = [(112, "Slayer", 32), (512, "Gunslinger", 30), (702, "GuardianKnight", 38)]
# The class list still draws these two rows without a crest.
CATEGORY_CRESTS = [("HunterFemale", 30), ("DragonKnight", 38)]
# EFTable_PC PrimaryKey -> folder, for the classes IconInfo puts on ClassSelectImg_once_0
# (512 Devilhunter_Female, 612 DimensionMaster, 702 DragonKnight).
ONCE_FOLDERS = {512: "Gunslinger", 612: "DimensionMaster", 702: "GuardianKnight"}

BIG_CELL = (396, 374)
BIG_COLUMNS = 2
# The small class image's cell is NOT a constant. ClassSelectImg_0 packs 27 icons at a 136px
# pitch; ClassSelectImg_once_0, which holds only the three newest classes, packs three at 318.
# Assuming the first pitch for both cut each of those three off at 43% of its width, so the
# pitch is measured off the page's own alpha instead of being written down here.
SMALL_HEIGHT = 80
SYMBOL_SPRITE, IDENTITY_SPRITE = "1246", "942"
# Every existing folder holds these byte-identical, so a new folder copies them.
SHARED_FILES = ["SeparateBar.png"] + ["Tag_%02d.png" % n for n in range(1, 29)]


def icon_cells(prefix):
    """PrimaryKey -> (page name, cell index), from IconInfo's own listing order."""
    lines = ICON_INFO.read_text(encoding="utf-8", errors="replace").splitlines()
    pages = collections.defaultdict(list)
    for index, line in enumerate(lines):
        found = re.fullmatch(prefix + r"_(\d+)\.png", line.strip())
        if not found:
            continue
        for probe in range(index + 1, min(index + 4, len(lines))):
            page = lines[probe].strip()
            if page.startswith(prefix):
                pages[page].append(int(found.group(1)))
                break
    cells = {}
    for page, keys in pages.items():
        for cell, key in enumerate(sorted(keys)):
            cells[key] = (page.lower(), cell)
    return cells


def icon_columns(image):
    """The x spans the page's own alpha separates its icons into, left to right.

    An atlas page packs its icons at one pitch, but that pitch differs per page, so it is read
    off the page rather than assumed: a fully transparent column is a gap between icons.
    """
    width, height = image.size
    pixels = image.load()
    spans, start = [], None
    for x in range(width + 1):
        filled = x < width and any(pixels[x, y][3] > 4 for y in range(height))
        if filled and start is None:
            start = x
        elif not filled and start is not None:
            spans.append((start, x))
            start = None
    return spans


def cut_measured(page_name, cell, target):
    """Cut one icon out of a page whose columns are measured, not assumed."""
    path = ICON_PAGES / (page_name + ".png")
    if not path.exists():
        print("   페이지 없음: %s" % path.name)
        return False
    image = Image.open(path).convert("RGBA")
    spans = icon_columns(image)
    row, column = divmod(cell, len(spans)) if spans else (0, 0)
    if not spans or column >= len(spans):
        print("   칸을 잴 수 없음: %s cell %d" % (page_name, cell))
        return False
    left, right = spans[column]
    top = row * SMALL_HEIGHT
    if top + SMALL_HEIGHT > image.height:
        print("   칸이 페이지 밖: %s cell %d" % (page_name, cell))
        return False
    crop = image.crop((left, top, right, top + SMALL_HEIGHT))
    if crop.getchannel("A").getbbox() is None:
        print("   빈 칸: %s cell %d" % (page_name, cell))
        return False
    crop = narrow_to_slot(crop)
    crop.save(target)
    return True


def standard_slot():
    """The list thumbnail's own geometry, measured off ClassSelectImg_0 rather than written
    down: the opaque span of its first column by the opaque span of its rows."""
    path = ICON_PAGES / "ClassSelectImg_0.png"
    image = Image.open(path).convert("RGBA")
    left, right = icon_columns(image)[0]
    box = image.crop((left, 0, right, SMALL_HEIGHT)).getchannel("A").getbbox()
    return right - left, box[3] - box[1]


def narrow_to_slot(crop):
    """The three newest classes (512/612/702) have no 136-wide thumbnail at all -- IconInfo
    gives them only ClassSelectImg_once_<PK>, a 318-wide banner. Both kinds carry the SAME
    trapezoid: the shear is an absolute 15 px across the 78 px height on either width, not a
    proportion of it. So a slot-wide cut keeps retail's exact shape as long as it carries the
    banner's own slanted ends -- what is ours to choose is only which window of the banner
    shows, and that is its centre, the same rule for all three. A cell already at or under the
    slot width is returned untouched. A cell that only overshoots in height -- the page row is
    80 tall but the art inside it is 78 -- is trimmed the same way."""
    width, height = standard_slot()
    if crop.width <= width and crop.height <= height:
        return crop
    width = min(width, crop.width)
    top = max(0, (crop.height - height) // 2)
    narrowed = crop.crop(((crop.width - width) // 2, top,
                          (crop.width - width) // 2 + width, top + height)).copy()
    # Carry enough of each end to include the whole slant plus its soft edge.
    edge = 24
    alpha = Image.new("L", (width, height), 255)
    alpha.paste(crop.crop((0, top, edge, top + height)).getchannel("A"), (0, 0))
    alpha.paste(crop.crop((crop.width - edge, top, crop.width, top + height)).getchannel("A"),
                (width - edge, 0))
    narrowed.putalpha(alpha)
    return narrowed


def cut(page_name, cell, size, columns, target):
    path = ICON_PAGES / (page_name + ".png")
    if not path.exists():
        print("   페이지 없음: %s" % path.name)
        return False
    image = Image.open(path).convert("RGBA")
    left, top = (cell % columns) * size[0], (cell // columns) * size[1]
    if left + size[0] > image.width or top + size[1] > image.height:
        print("   칸이 페이지 밖: %s cell %d" % (page_name, cell))
        return False
    crop = image.crop((left, top, left + size[0], top + size[1]))
    if crop.getchannel("A").getbbox() is None:
        print("   빈 칸: %s cell %d" % (page_name, cell))
        return False
    crop.save(target)
    return True


class Share:
    """shareImageV2's exports, and the rect each one draws."""

    def __init__(self):
        root = ET.parse(SHARE / "shareimagev2_structure.xml").getroot()
        self.subimages, self.images, self.sprites = {}, {}, {}
        for tag in root.iter():
            kind = tag.get("type") or ""
            if kind == "DefineSubImage":
                self.subimages[tag.get("characterID")] = tag
            elif kind == "DefineExternalImage2":
                self.images[tag.get("imageID")] = tag.get("fileName")
            elif kind == "DefineSpriteTag":
                self.sprites[tag.get("spriteId")] = tag
        self.exports = {}
        for line in (SHARE / "symbols_v2.txt").read_text(
                encoding="utf-8", errors="replace").splitlines():
            parts = line.split()
            if len(parts) >= 2 and parts[0].isdigit():
                self.exports[parts[1]] = parts[0]

    def rect(self, character, depth=0):
        sub = self.subimages.get(character)
        if sub is not None:
            page = self.images.get(sub.get("imageId"))
            if page is None:
                return None
            box = tuple(int(sub.get(key)) for key in ("x1", "y1", "x2", "y2"))
            return Path(page).stem.lower(), box
        sprite = self.sprites.get(character)
        if sprite is None or depth > 3:
            return None
        for child in sprite.iter():
            if child.get("type") in ("PlaceObject2Tag", "PlaceObject3Tag"):
                inner = child.get("characterId")
                if inner and inner != "0":
                    found = self.rect(inner, depth + 1)
                    if found:
                        return found
        return None

    def save(self, export, target):
        found = self.rect(self.exports.get(export))
        if found is None:
            print("   해결 실패: %s" % export)
            return False
        page, box = found
        path = SHARE_PAGES / (page + ".png")
        if not path.exists():
            print("   페이지 없음: %s" % path.name)
            return False
        Image.open(path).convert("RGBA").crop(box).save(target)
        return True


def class_bindings(sprite_id):
    """class_<n> frame -> the export componentsv2 places on it."""
    root = ET.parse(COMPONENTS).getroot()
    for tag in root.iter():
        if tag.get("type") != "DefineSpriteTag" or tag.get("spriteId") != sprite_id:
            continue
        bindings, label = {}, None
        for child in tag.iter():
            kind = child.get("type")
            if kind == "FrameLabelTag":
                label = child.get("name")
            elif kind in ("PlaceObject2Tag", "PlaceObject3Tag") and child.get("className"):
                if label and label.startswith("class_"):
                    bindings[int(label.split("_")[1])] = child.get("className")
        return bindings
    return {}


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo", type=Path, default=Path(__file__).resolve().parents[2])
    args = parser.parse_args()
    panel = args.repo / "Client/Bin/Resources/UI/ClassSelect"
    donor = panel / "Warlord"
    if not donor.is_dir():
        print("기준 폴더가 없습니다: %s" % donor)
        return 1

    share = Share()
    crests, emblems = class_bindings(SYMBOL_SPRITE), class_bindings(IDENTITY_SPRITE)
    big, small = icon_cells("ClassSelectImg_Big"), icon_cells("ClassSelectImg")
    small.update(icon_cells("ClassSelectImg_once"))

    for key, name, frame in MISSING:
        folder = panel / name
        folder.mkdir(parents=True, exist_ok=True)
        print("== %s" % name)
        if key in big:
            page, cell = big[key]
            cut(page, cell, BIG_CELL, BIG_COLUMNS, folder / "Illustration.png")
        else:
            print("   큰 초상화 칸 없음 (PrimaryKey %d)" % key)
        if key in small:
            page, cell = small[key]
            cut_measured(page, cell, folder / "IllustrationSmall.png")
        else:
            print("   작은 초상화 칸 없음 (PrimaryKey %d)" % key)
        crest = crests.get(frame)
        if crest:
            share.save(crest, folder / "IdentitySymbol.png")
        else:
            print("   class_%d 에 crest 바인딩 없음" % frame)
        emblem = emblems.get(frame)
        if emblem:
            share.save(emblem, folder / "NameID.png")
        else:
            print("   class_%d 에 emblem 바인딩 없음" % frame)
        for shared in SHARED_FILES:
            source = donor / shared
            if source.exists():
                shutil.copyfile(source, folder / shared)
        print("   파일 %d개" % len(list(folder.iterdir())))

    # Every class whose thumbnail comes off the _once page is re-cut here, folder already
    # present or not: DimensionMaster shipped from that page before this narrowing existed and
    # so carries a 316-wide image in a 134-wide slot, which is what made it overflow its
    # hover frame. Which classes those are is read from IconInfo, not listed by hand.
    for key, (page, cell) in sorted(icon_cells("ClassSelectImg_once").items()):
        name = ONCE_FOLDERS.get(key)
        if not name:
            print("_once 칸의 폴더 이름을 모름: PrimaryKey %d" % key)
            continue
        folder = panel / name
        if not folder.is_dir():
            continue
        if cut_measured(page, cell, folder / "IllustrationSmall.png"):
            print("작은 초상화 재단: %s" % name)

    for label, frame in CATEGORY_CRESTS:
        crest = crests.get(frame)
        if not crest:
            print("카테고리 crest 바인딩 없음: %s (class_%d)" % (label, frame))
            continue
        share.save(crest, panel / "Common" / ("CategorySymbol_%s.png" % label))
        print("카테고리 crest: CategorySymbol_%s.png" % label)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
