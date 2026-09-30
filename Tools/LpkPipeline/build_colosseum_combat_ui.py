"""Extract exact retail HP/kill-row bitmap shapes; author a two-player team layout.

Uses the same GFX decoder as result extraction. Dynamic clipping is performed by
CUI_Sprite fill ratio, not by baking a fake full-HP image or a screenshot.
"""
import argparse
import json
from pathlib import Path

from build_colosseum_match_result_ui import Extractor, slot, write


SHAPES = (273, 276, 280, 282, 12, 15, 18, 29, 31)


def validate(repo):
    path = repo / 'Data/UI/Colosseum/CombatHUD_Layout.json'
    doc = json.loads(path.read_text(encoding='utf-8'))
    assert doc['schema'] == 'lostark.ui-layout' and doc['formatVersion'] == 1
    slots = {s['id']: s for s in doc['slots']}
    assert len(slots) == len(doc['slots'])
    required = {f'{side}{i}_{part}' for side in ('Left', 'Right') for i in range(2)
                for part in ('BG', 'HP', 'Frame', 'Name', 'Health', 'Kills', 'Number')}
    required |= {f'Feed{i}_{part}' for i in range(3)
                 for part in ('BG', 'Red', 'Blue', 'RedIcon', 'BlueIcon', 'Killer', 'Victim')}
    assert required <= slots.keys()
    assets = set()
    for s in slots.values():
        r = s['rect']
        assert 0 <= r['x'] < 1280 and 0 <= r['y'] < 720
        assert r['width'] > 0 and r['height'] > 0
        for layer in s['layers']:
            asset = layer['path']
            assert asset.startswith('UI/Colosseum/Combat/') and '..' not in asset and ':' not in asset
            assert (repo / 'Client/Bin/Resources' / asset).is_file(), asset
            assets.add(asset)
    print(f'Colosseum combat UI: {len(slots)} slots, {len(assets)} exact source images validated')


def build(repo, movie, textures):
    target = repo / 'Client/Bin/Resources/UI/Colosseum/Combat'
    ex = Extractor(movie, textures, target)
    bounds = {}
    for cid in SHAPES:
        image, x, y = ex.shape(cid)
        image.save(target / f'shape_{cid}.png')
        bounds[cid] = (x, y, *image.size)
    slots = []

    def image_slot(name, cid, x, y, scale):
        bx, by, w, h = bounds[cid]
        item = slot(name, x + bx * scale, y + by * scale, w * scale, h * scale,
                    f'UI/Colosseum/Combat/shape_{cid}.png')
        slots.append(item)
        return item

    # Source sprite 327 -> hpBarFrame_mc 294: backing, clip target at (9,10),
    # track 286 -> bg 285 (three team states), frame 276. No clip-mask geometry.
    for side, x, fill in [('Left', 8, 280), ('Right', 1088, 282)]:
        for i in range(2):
            prefix = f'{side}{i}_'
            y, scale = 316 + i * 52, .8
            image_slot(prefix + 'BG', 273, x, y, scale)
            image_slot(prefix + 'HP', fill, x + 9 * scale, y + 10 * scale, scale)
            image_slot(prefix + 'Frame', 276, x, y, scale)
            slots += [slot(prefix + 'Number', x + 4, y + 14, 20, 18),
                      slot(prefix + 'Name', x + 29, y + 7, 108, 16),
                      slot(prefix + 'Health', x + 29, y + 24, 108, 14),
                      slot(prefix + 'Kills', x + 141, y + 15, 34, 18)]
    # Sprite 35: background, team ribbon, arrow/cross icon, fromUser/toUser.
    for i in range(3):
        prefix, x, y, scale = f'Feed{i}_', 980, 94 + i * 35, .65
        image_slot(prefix + 'BG', 12, x, y, scale)
        image_slot(prefix + 'Red', 15, x, y, scale)
        image_slot(prefix + 'Blue', 18, x, y, scale)
        image_slot(prefix + 'RedIcon', 29, x + 172 * scale, y + 5 * scale, scale)
        image_slot(prefix + 'BlueIcon', 31, x + 172 * scale, y + 5 * scale, scale)
        slots += [slot(prefix + 'Killer', x + 7, y + 9, 100, 18),
                  slot(prefix + 'Victim', x + 172, y + 9, 100, 18)]
    write(repo / 'Data/UI/Colosseum/CombatHUD_Layout.json',
          {'schema': 'lostark.ui-layout', 'formatVersion': 1,
           'resolution': {'width': 1280, 'height': 720},
           'classes': ['Default'], 'slots': slots})
    validate(repo)


if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('--repo', type=Path, default=Path(__file__).resolve().parents[2])
    ap.add_argument('--movie', type=Path)
    ap.add_argument('--tex', type=Path)
    ap.add_argument('--validate', action='store_true')
    args = ap.parse_args()
    if args.validate:
        validate(args.repo)
    else:
        if not args.movie or not args.tex:
            ap.error('--movie and --tex are required')
        build(args.repo, args.movie, args.tex)
