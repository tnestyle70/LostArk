"""Append the five source smoke occurrences as reviewable candidates only.

Existing authored elements are copied from the latest disk documents unchanged.
The install receipt retains baseline hashes for the caller's final CAS install.
"""
from pathlib import Path
import copy
import hashlib

import build_maharaka_waterpang_source_effects as hazard
from build_kouku_action_effect_groups import restored_index

OUT = hazard.ROOT / 'out/WaterpangEffects20260930/smoke'
MATERIAL = 'bfx_m_mi_00.bfx_mi.bfx_d_pa_smoke_01_03_tr'


def main():
    source = hazard.source
    patches = source.read(hazard.EVIDENCE / 'missing_smoke/native/native_material_patch.json')['programs']
    natives = {key: row['material'] for row in patches for key in row['occurrences']}
    receipt = []
    for path in sorted(hazard.EVIDENCE.glob('*/stages/*/source_occurrences.json')):
        occurrences = [row for row in source.read(path) if row['sourceMaterial'] == MATERIAL]
        if not occurrences:
            continue
        stage, clip, profile_name = path.parent, path.parent.name, path.parents[2].name
        profile = hazard.PROFILES[profile_name]
        hazard.configure(profile_name)
        source.SELECTED = {profile['action']: ([], clip)}
        index = restored_index(stage)
        destination = OUT / 'raw' / profile_name / clip
        source.project(stage, index, source.read(stage / 'source_notifies.json'), occurrences, {}, destination)
        subset = source.read(destination / ('effect.kouku.gate1.%s.full.restore.effect.json' % profile['action']))
        asset_id = profile['prefix'] + clip + '.full.restore'
        by_id = {row['elementId']: row for row in occurrences}
        for element in subset['elements']:
            identity = element['id'] if element['id'] in by_id else element['id'].rsplit('.', 1)[0]
            element['groupId'] = asset_id
            element['material'] = copy.deepcopy(natives[identity])
            element['detail']['uv'].update(start=[0, 0], speed=[0, 0], wave=False, sequence=False)
            element['detail']['color']['emissiveIntensity'] = 1
        hazard.drv.bind_providers(subset, index, by_id)
        hazard.bind_object_rig(subset, root_basis=0 if profile_name == 'mokomoko' else None)
        authored = hazard.AUTHORED / (asset_id + '.effect.json')
        baseline = authored.read_bytes()
        document = source.read(authored)
        existing = {element['id'] for element in document['elements']}
        added = [element for element in subset['elements'] if element['id'] not in existing]
        assert len(added) == len(occurrences), 'Source smoke already present or anchor inventory changed'
        document['elements'].extend(added)
        hazard.drv.audit_document(document)
        source.write(OUT / 'candidate' / authored.name, document)
        receipt.append(dict(effectAssetId=asset_id, baselineSha256=hashlib.sha256(baseline).hexdigest(),
            addedElements=[element['id'] for element in added], elementCount=len(document['elements'])))
    assert sum(len(row['addedElements']) for row in receipt) == 5
    source.write(OUT / 'candidate_summary.json', receipt)
    print(receipt)


if __name__ == '__main__':
    main()
