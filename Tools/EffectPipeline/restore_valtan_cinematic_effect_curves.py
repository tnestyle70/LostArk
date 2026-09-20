"""Close original Valtan moving emitters and PSC parameters in the V1 carrier.

Keep source Hermite keys/tangents and independent parameter factors. No sampled
trajectory, new runtime, or substituted particle/material asset is introduced.
"""
from pathlib import Path
import argparse
import copy
import json
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT/'Tools/EffectPipeline'))
import build_kouku_sequence_effect_groups as curves
import restore_valtan_cinematic_static_effects as world
import build_valtan_full_restore as writer

SOURCES = ROOT/'out/FullMapRestoration20260915/ValtanSequences'
MOVING = (
    ('SCENE06A', 54, 'trash', 165, 104, 667),
    ('SCENE06A', 54, 'trash', 167, 100, 667),
    ('SCENE06A', 54, 'trash', 157, 106, 667),
    ('SCENE06A', 55, 'roar', 179, 98, 4889),
    ('SCENE06A', 55, 'roar', 180, 99, 3843),
    ('SCENE02A', 52, 'phase2', 130, 75, 1549),
    ('SCENE02A', 52, 'phase2', 131, 76, 1539),
)


def occurrence(scene, matinee, group, actor):
    scene = 'LV_LUT_HEARTRB_ED_' + scene
    cache = json.loads((SOURCES/(scene+'.json')).read_bytes())
    rows = cache['rows']
    props = rows[str(actor)]['p']
    # UE stores the previous attachment bone even when Base is null. Only an
    # actual parent reference makes that name an active skeletal attachment.
    if not props.get('base'):
        props.pop('basebonename', None)
    component = rows[str(props['particlesystemcomponent'])]['p']
    data = next(link['linkedvariables'][0] for link in rows[str(matinee)]['p']['variablelinks']
                if link['linkdesc'] == 'Data')
    own = dict(sourceScene=scene, matineeExport=matinee, dataExport=data,
               groupExport=group, actorExport=actor, actorProperties=props,
               componentProperties=component,
               sourceOccurrenceId=f'{scene}.{matinee}.actor{actor}')
    system = cache['imports'][str(component['template'])].lower()
    return own, cache, 'effect.valtan.source.' + system


def scalar_keys(track):
    raw = copy.deepcopy(track['p']['floattrack'])
    for point in raw['points']:
        for field in ('outval', 'arrivetangent', 'leavetangent'):
            point[field] = dict(x=point.get(field, 0), y=0, z=0)
    return curves.curve_keys(raw)


def repair_seeded_lifetime_bounds(document):
    """Mirror the original constant seeded lifetime range in Detail's bounds."""
    repaired = []
    for element in document['elements']:
        particle = element['detail']['particle']
        if particle['lifeTimeSeconds'] != [0, 0]:
            continue
        modules = [m for m in element['sourceRecipe']['modules']
                   if m['className'] == 'particlemodulelifetime_seeded']
        assert len(modules) == 1
        distribution, = [d for d in modules[0]['distributions']
                         if d['propertyPath'] == 'lifetime']
        table = distribution['lookupTable']
        assert distribution['operation'] == 2 and distribution['componentCount'] == 1
        assert distribution['lookupTableTimeScale'] == 0 and not distribution['keys']
        assert len(table) == 6 and table[:2] == table[2:4] == table[4:6]
        assert 0 < table[0] <= table[1] <= 30
        particle['lifeTimeSeconds'] = table[:2]
        repaired.append(element['id'])
    return repaired


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--evidence-root', type=Path, required=True)
    parser.add_argument('--install', action='store_true')
    args = parser.parse_args()
    args.evidence_root.mkdir(parents=True, exist_ok=True)
    index = curves.action.restored_index(ROOT/'out/ValtanRestoration20260920/CinematicLibrary')
    documents, receipt, writes = {}, [], []

    def variant(base, identity):
        doc = json.loads((ROOT/'Data/Effects/Authored'/(base+'.effect.json')).read_bytes())
        repair_seeded_lifetime_bounds(doc)
        doc['effectAssetId'] = identity
        doc['displayName'] += ' / original cinematic occurrence'
        for element in doc['elements']:
            element['groupId'] = identity
        documents[identity] = doc
        return doc

    for scene, matinee, name, group, actor, start in MOVING:
        own, cache, base = occurrence(scene, matinee, group, actor)
        transform = curves.moving_transform(own, cache)
        identity = f'effect.valtan.cinematic.{name}.actor{actor}.at{start}'
        doc = variant(base, identity)
        for element in doc['elements']:
            assert not element.get('sourceTransformTrack')
            assert not element['actionCueAttachment']['enabled']
            element['sourceTransformTrack'] = dict(sourceOccurrenceId=own['sourceOccurrenceId'],
                sourceTimeOriginSeconds=start/1000, previewOriginUE3Cm=transform['sourcePositionUE3Cm'],
                nodes=transform['sourceTransformNodes'])
        receipt.append(dict(asset=identity, source=base, kind='EXACT_SOURCE_TRANSFORM',
            scene=own['sourceScene'], actor=actor, startMs=start,
            sourcePackageSha256=cache['sha256'], transform=transform))

    own, cache, base = occurrence('SCENE06A', 53, 121, 108)
    tracks = curves.source_tracks(own, cache)
    bindings = curves.parameters(own, tracks)
    assert bindings == [dict(name='color', type='vector', vectorValue=[0., 5., 12.])]
    eye_id = 'effect.valtan.cinematic.entrance.eyes.parameters'
    doc = variant(base, eye_id)
    evidence = []
    for element in doc['elements']:
        evidence += curves.action.project_parameters(index, element['sourceRecipe'], dict(parameterOverrides=bindings))
    second, second_cache, _ = occurrence('SCENE06A', 53, 122, 109)
    assert curves.parameters(second, curves.source_tracks(second, second_cache)) == bindings
    receipt.append(dict(asset=eye_id, source=base, kind='EXACT_SOURCE_CONSTANT_PARAMETER',
                        actors=[108, 109], parameters=bindings, projection=evidence,
                        sourcePackageSha256=cache['sha256']))

    own, cache, base = occurrence('SCENE04A', 24, 68, 35)
    tracks = curves.source_tracks(own, cache)
    factors = {t['p']['paramname']: scalar_keys(t) for t in tracks
               if t['cls'] == 'interptrackfloatparticleparam' and not t['p'].get('bdisabletrack', False)}
    assert set(factors) == {'alpha', 'alpha_1'}
    transform = curves.moving_transform(own, cache)
    finale_id = 'effect.valtan.cinematic.finale.actor35.parameters'
    doc = variant(base, finale_id)
    factor_owners = []
    for element in doc['elements']:
        names = []
        for module in element['sourceRecipe']['modules']:
            literals = {r['propertyPath']: r['value'] for r in module['literals']}
            if not literals.get('benabled', True):
                continue
            for dist in module['distributions']:
                obj = index.get_path(dist.get('sourceObjectPath'))
                name = curves.action.IMPORTED.prop(obj.properties, 'parametername', '').lower() if obj else ''
                if name not in factors:
                    continue
                assert module['className'] == 'particlemodulecolorscaleoverlife'
                assert dist['propertyPath'] == 'alphascaleoverlife'
                assert curves.action.IMPORTED.prop(obj.properties, 'parammode', '') == 'dpm_direct'
                names.append(name)
        assert names and len(names) <= 16
        curves.action.project_parameters(index, element['sourceRecipe'], dict(parameterOverrides=[
            dict(name=name, type='scalar', scalarValue=1.) for name in factors]))
        element['sourceTransformTrack'] = dict(sourceOccurrenceId=own['sourceOccurrenceId'],
            sourceTimeOriginSeconds=.720, previewOriginUE3Cm=transform['sourcePositionUE3Cm'],
            nodes=transform['sourceTransformNodes'], alphaScaleFactors=[factors[name] for name in names])
        factor_owners.append(dict(elementId=element['id'], factors=names))
    receipt.append(dict(asset=finale_id, source=base, kind='EXACT_SOURCE_ALPHA_FACTORS', actor=35,
                        factorCurves=factors, factorOwners=factor_owners,
                        sourcePackageSha256=cache['sha256'], transform=transform))

    for asset, doc in documents.items():
        path = ROOT/'Data/Effects/Authored'/(asset+'.effect.json')
        before = path.read_bytes() if path.exists() else None
        after = writer.encode_json(doc)
        previous = json.loads(before) if before is not None else None
        if previous is not None:
            repair_seeded_lifetime_bounds(previous)
        assert previous is None or previous == doc, 'Preserve edited curve occurrence ' + asset
        candidate = args.evidence_root/'candidate'/path.name
        candidate.parent.mkdir(parents=True, exist_ok=True)
        candidate.write_bytes(after)
        writes.append((path, before, after))
    # The shared source remains a valid standalone tool asset as well. Preserve
    # its exact recipe; only its stale zero Detail envelope is corrected.
    base_path = ROOT/'Data/Effects/Authored'/(base+'.effect.json')
    base_before = base_path.read_bytes()
    base_document = json.loads(base_before)
    if repair_seeded_lifetime_bounds(base_document):
        writes.append((base_path, base_before, writer.encode_json(base_document)))
    cat = ROOT/'Data/Effects/EffectCatalog.json'
    before = cat.read_bytes(); catalog = json.loads(before)
    by_id = {e['effectAssetId']: e for e in catalog['effects']}
    for asset in documents:
        row = dict(effectAssetId=asset, payloadKind='DIRECT_AUTHORED_DOCUMENT',
                   authoringPath=f'Effects/Authored/{asset}.effect.json')
        assert asset not in by_id or by_id[asset] == row
        if asset not in by_id:
            catalog['effects'].append(row)
    writes.append((cat, before, writer.encode_json(catalog)))
    writes += writer.project_writes(list(documents), False)
    for path, before, _ in writes:
        if before is not None:
            backup = args.evidence_root/'backup'/path.relative_to(ROOT)
            backup.parent.mkdir(parents=True, exist_ok=True)
            if not backup.exists():
                backup.write_bytes(before)
    (args.evidence_root/'receipt.json').write_bytes(writer.encode_json(dict(documents=receipt,
        installed=args.install, visualStatus='USER_PENDING')))
    if args.install:
        writer.commit_writes(writes)
    print(json.dumps(dict(variants=len(documents), installed=args.install)))


if __name__ == '__main__':
    main()
