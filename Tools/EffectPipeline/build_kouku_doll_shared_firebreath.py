"""Stage the existing Odd Doll motion with the authored common firebreath.

The live authoring files are never overwritten. Keep the two original mouth
sockets, preparation/end effects and model cue, replacing only the main flame.
"""
import argparse
import copy
import hashlib
import json
from pathlib import Path

from build_kouku_shared_firebreath import copy_shared_firebreath_elements, SHARED_ASSET

ROOT = Path(__file__).resolve().parents[2]
SOURCE_ASSET = 'effect.kouku.gate3.doll.flame.object-sustain15'
TARGET_ASSET = 'effect.kouku.gate3.doll.flame.shared'


def build(document, shared):
    assert document['effectAssetId'] == SOURCE_ASSET
    assert document['particleSystem'] == shared['particleSystem']
    selected = [e for e in document['elements'] if 'notify-007|' in e['sourceNode']]
    assert len(selected) == 28
    groups = {}
    for element in selected:
        attachment = element['actionCueAttachment']
        assert attachment['enabled'] and attachment['follow']
        groups.setdefault(attachment['runtimeBoneName'], []).append(element)
    assert set(groups) == {'b_mouth_f', 'b_mouth_b'}
    result = copy.deepcopy(document)
    result.update(effectAssetId=TARGET_ASSET, displayName='괴기스러운 인형_회전 화염 뿜기_세이튼 공통 불뿜기')
    ids = {e['id'] for e in selected}
    result['elements'] = [e for e in result['elements'] if e['id'] not in ids]
    proof = []
    for bone, originals in sorted(groups.items()):
        first = originals[0]
        placement = first['detail']['transform']
        timing = first['detail']['timing']
        attachment = first['actionCueAttachment']
        assert len(originals) == 14
        assert placement['position'] == placement['rotationDegrees'] == [0, 0, 0]
        assert placement['scale'] == [100, 100, 100]
        assert all(e['actionCueAttachment'] == attachment and e['detail']['transform'] == placement
                   and e['detail']['timing'] == timing for e in originals)
        group = TARGET_ASSET + '.' + bone
        elements = copy_shared_firebreath_elements(shared, group)
        for element in elements:
            element['actionCueAttachment'] = copy.deepcopy(attachment)
            detail = element['detail']
            transform = detail['transform']
            # Source Doll projects along socket +X. Common breath uses +Z.
            # Socket/bone coordinates are centimetres and the actual CModel
            # applies .01, so compensate here once, retaining authored size.
            transform['rotationDegrees'][1] += 90
            transform['scale'] = [v * 100 for v in transform['scale']]
            detail['timing']['startDelaySeconds'] += timing['startDelaySeconds']
            detail['timing']['lifeTimeSeconds'] = timing['lifeTimeSeconds']
            # Sustain source emitters while the existing 15-second flame beat
            # runs, preserving their spawn/distribution/material recipes.
            element['sourceRecipe']['emitterLoopCount'] = 0
        result['elements'].extend(elements)
        proof.append(dict(bone=bone, attachment=attachment, sourceForward=[1, 0, 0],
                          sharedForward=[0, 0, 1], localYawDegrees=90,
                          centimetreBasisCompensation=100,
                          startSeconds=timing['startDelaySeconds'],
                          lifetimeSeconds=timing['lifeTimeSeconds'], elements=len(elements)))
    assert len(result['elements']) == 58
    assert result['elements'][:8] == [e for e in document['elements'] if e['id'] not in ids]
    return result, proof


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    output = args.output.resolve()
    if not output.is_relative_to(ROOT / 'out'):
        raise ValueError('Stage output must stay under repository out/.')
    paths = [ROOT / 'Data/Effects/Authored' / (name + '.effect.json')
             for name in (SOURCE_ASSET, SHARED_ASSET)]
    data = [path.read_bytes() for path in paths]
    result, branches = build(*(json.loads(value) for value in data))
    output.mkdir(parents=True, exist_ok=True)
    candidate = output / (TARGET_ASSET + '.effect.json')
    candidate.write_text(json.dumps(result, ensure_ascii=False, indent=2, allow_nan=False) + '\n', encoding='utf-8')
    receipt = dict(candidate=str(candidate), candidateSha256=hashlib.sha256(candidate.read_bytes()).hexdigest(),
                   installed=False, inputs=[dict(path=str(path), sha256=hashlib.sha256(value).hexdigest())
                                            for path, value in zip(paths, data)],
                   retainedPreparationAndEndElements=8, replacementElements=50, branches=branches,
                   modelCuesPreserved=result.get('modelCues') == json.loads(data[0]).get('modelCues'),
                   visualValidation='USER_PENDING')
    (output / 'doll-fire-stage.json').write_text(json.dumps(receipt, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(dict(elements=58, candidate=str(candidate), installed=False)))


if __name__ == '__main__':
    main()
