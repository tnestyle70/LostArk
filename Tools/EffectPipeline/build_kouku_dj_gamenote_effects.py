"""Compose original GameNote art/font into the existing V2 screen-image carrier.

This is offline source extraction and text layout, not a screenshot/redraw.
The two Korean captions are original GameMsg rows; placement is project-tuned.
"""
from pathlib import Path
import copy
import hashlib
import json
import sqlite3
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'out/CardRain20260922'


def read(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')


def build():
    source = OUT / 'source'
    frame_path = source / 'ui-frame/EFUI_COMMONOBJECT/Texture2D/commonobject_i6a.dds'
    atlas_path = source / 'ui-atlas/EFUI_ICONATLAS_C/Texture2D/common_gamenote_npc_img_4.dds'
    font_path = source / 'YG760.ttf'
    frame_atlas = Image.open(frame_path).convert('RGBA')
    portrait = Image.open(atlas_path).convert('RGBA').crop((853, 569, 995, 711))
    # GameNoteFrame's source bounds: (-17,-24)..(654,121), 671x145.
    base = Image.new('RGBA', (671, 145))
    base.alpha_composite(frame_atlas.crop((814, 602, 986, 723)), (10, 23))
    base.alpha_composite(frame_atlas.crop((0, 317, 661, 462)), (10, 0))
    base.alpha_composite(portrait, (0, 2))
    manifest = read(OUT / 'candidate-manifest.json')
    notes = read(source / 'gamenote-kouku.json')
    connection = sqlite3.connect(ROOT / 'out/KoukuSceneAudioSubtitle20260920/source/EFGame_Extra/ClientData/TableData/EFTable_GameMsg.db')
    connection.row_factory = sqlite3.Row
    table = [dict(r) for r in connection.execute('SELECT * FROM GameMsg')]
    # Keep the original lookup keys in the receipt; do not infer localized text
    # from a material filename or from the supplied compressed screenshot.
    receipts = []
    for suffix, note_id, caption in [('cardrain', 110149, '쏟아져라, 카드 비!'),
                                    ('delivery', 110157, '특급 배송 출발!')]:
        note = next(r for r in notes if r['PrimaryKey'] == note_id)
        assert note['PortraitIndex'] == 231 and note['AutoShutDownTime'] == 5000
        matching = [r for r in table if note['SubTitle'].lower() in [str(v).lower() for v in r.values()]]
        assert len(matching) == 1 and caption in matching[0].values()
        banner = base.copy()
        draw = ImageDraw.Draw(banner)
        # $YG760 16px nameTF and 14px talkTF from original GameNoteFrame.
        # Text and speaker colour are baked; this is an ordinary ScreenPost PNG.
        for label, position, size, color in [('DJ쿠크', (151, 24), 16, (128, 242, 184, 255)),
                                             (caption, (151, 56), 14, (255, 255, 255, 255))]:
            font = ImageFont.truetype(str(font_path), size)
            draw.text((position[0]+1, position[1]+1), label, font=font,
                      fill=(0, 0, 0, 255), anchor='lt')
            draw.text(position, label, font=font, fill=color, anchor='lt')
        asset = f'UI/KoukuSaydon/GameNote/dj_kouku_{suffix}.png'
        texture = OUT / 'Resources' / asset
        texture.parent.mkdir(parents=True, exist_ok=True)
        banner.save(texture)
        doc = read(ROOT / 'Data/Effects/V2/Authored/boss.kouku.fear.face_1.effectv2.json')
        doc['effectId'] = f'boss.kouku.dj.{suffix}'
        doc['slots']['base'] = asset
        params = doc['params']
        params.update(lifetime=5, alphaInEnd=.03, alphaOutStart=.94,
                      scaleInEnd=0, scaleOutStart=1, bloomIntensity=0)
        params['scale'] = dict(start=[1, 1, 1], end=[1, 1, 1], lerp=False)
        screen = params['screenPost']
        screen.update(overlayPositionStart=[.5, .78], overlayPositionHold=[.5, .78],
                      overlayPositionEnd=[.5, .78], overlayScale=[671/1920, 145/1080],
                      overlayEnterEnd=0, overlayExitStart=1, displaySpace=True)
        path = OUT / 'candidate/Data/Effects/V2/Authored' / (doc['effectId'] + '.effectv2.json')
        write(path, doc)
        manifest['documents'].append(dict(effectAssetId=doc['effectId'], effectKind='V2',
            displayName='DJ쿠크_' + caption, candidatePath=path.relative_to(ROOT).as_posix(),
            destination='Data/Effects/V2/Authored/' + path.name, durationMs=5000,
            defaultAnchorKind='BOSS', followBoss=False, elementCount=1))
        manifest['resources'].append(dict(assetId=asset, candidatePath=texture.relative_to(ROOT).as_posix(),
            sha256=hashlib.sha256(texture.read_bytes()).hexdigest()))
        receipts.append(dict(effectId=doc['effectId'], gameNote=note, gameMsg=matching[0],
            portraitAtlas=atlas_path.relative_to(ROOT).as_posix(), portraitRect=[853, 569, 142, 142],
            frameAtlas=frame_path.relative_to(ROOT).as_posix(), frameRect=[0, 317, 661, 145],
            sourceFont='font.lpk/Binaries/Fonts/HANYoonGothic760.ttf',
            sourceFrame='EFUI_COMMONOBJECT/commonobject/GameNoteFrame',
            projectTuned=['Normalized viewport centre .5,.78', 'Speaker mint-green colour',
                          'Static Korean caption rasterization and short alpha fades']))
    write(source / 'dj-gamenote-provenance.json', receipts)
    write(OUT / 'candidate-manifest.json', manifest)
    print('DJ ScreenPost candidates=2 original-atlas+frame+font, resources=2')


if __name__ == '__main__':
    build()
