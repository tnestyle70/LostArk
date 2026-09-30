"""Cut the 30 mokoko avatar shop icons (UI/Items/Avatar/mokoko_036<var>_<head|outfit>.png).

Chain: EFTable_Item Model EFDLItem_PC_WR_AV_036<VAR>_<Head|Dress> -> Icon/IconIndex
       -> IconInfo.loa `<Icon>_<IconIndex>.png` (page, x, y, w, h) -> icon atlas page.
The warrior model rows are used; every class shares one icon per variant (classVariants template).
Pass --dry-run to print the rects without writing.
"""
import argparse, glob, sqlite3, struct
from pathlib import Path
from PIL import Image

EXTRACTED = Path(r"D:/ClaudeWork/Extracted")
ITEM_DB = EXTRACTED / "AllTables/EFGame_Extra/ClientData/TableData/DEV/EFTable_Item.db"
ICONINFO = EXTRACTED / "Data3/EFGame_Extra/ClientData/XmlData/IconInfo.loa"
PAGE_ROOTS = [EXTRACTED / "IconAtlas_Extracted", EXTRACTED / "Vehicle/icons"]
VARIANTS = ["036", "036-1", "036-2", "036-3", "036-4", "036-5", "036a", "036a-1", "036a-2",
            "036b", "036b-1", "036b-2", "036c1", "036c2", "036c3"]


def iconinfo_lookup(data, name):
    needle = (name + ".png").encode()
    i = data.lower().find(needle.lower())
    if i < 0:
        return None
    n = struct.unpack_from("<i", data, i - 4)[0]
    j = i + n
    plen = struct.unpack_from("<i", data, j)[0]
    page = data[j + 4:j + 4 + plen].split(b"\0")[0].decode()
    return (page,) + struct.unpack_from("<4i", data, j + 4 + plen)


def open_page(stem):
    for root in PAGE_ROOTS:
        hits = [h for h in glob.glob(str(root / "**" / (stem + ".*")), recursive=True)
                if h.lower().endswith((".dds", ".tga", ".png")) and "_sheet" not in h.lower()]
        if hits:
            return Image.open(hits[0]).convert("RGBA")
    raise SystemExit("icon page %s not extracted" % stem)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--repo", type=Path, default=Path(__file__).resolve().parents[2])
    ap.add_argument("--dry-run", action="store_true")
    args = ap.parse_args()
    out_dir = args.repo / "Client/Bin/Resources/UI/Items/Avatar"
    con = sqlite3.connect(str(ITEM_DB))
    info = ICONINFO.read_bytes()
    for var in VARIANTS:
        for part, model_part in (("head", "Head"), ("outfit", "Dress")):
            model = "EFDLItem_PC_WR_AV_%s_%s.PC_WR_AV_%s_%s" % (var.upper().replace("036A", "036A"), model_part, var.upper(), model_part)
            rows = con.execute("select distinct Icon, IconIndex from Item where lower(Model)=lower(?)", (model,)).fetchall()
            icons = {(r[0].decode() if isinstance(r[0], bytes) else r[0], r[1]) for r in rows}
            if len(icons) != 1:
                raise SystemExit("%s: expected one icon, got %s" % (model, icons))
            icon, index = next(iter(icons))
            found = iconinfo_lookup(info, "%s_%d" % (icon, index))
            if found is None:
                raise SystemExit("IconInfo has no %s_%d" % (icon, index))
            page, x, y, w, h = found
            name = "mokoko_%s_%s.png" % (var, part)
            print(name, icon, index, page, (x, y, w, h))
            if not args.dry_run:
                out_dir.mkdir(parents=True, exist_ok=True)
                open_page(page).crop((x, y, x + w, y + h)).save(out_dir / name)


main()
