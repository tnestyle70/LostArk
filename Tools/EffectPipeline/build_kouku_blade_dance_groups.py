"""Build independent warning/impact groups from the RPCT05 weapon action.

Only authored effect documents and the evidence manifest are written. Catalog,
tree, Composition, project registration and runtime publication belong to the
integration step. Existing documents are compared before any write so authored
tuning is never silently replaced.
"""
import argparse
import base64
import copy
import hashlib
import json
import math
from pathlib import Path
import sqlite3
import struct

import build_kouku_showtime_warning_groups as warning

ROOT = warning.ROOT
SOURCE = warning.source.SOURCE
read = warning.source.read
write = warning.source.write


def source_contract():
    action_path = SOURCE / 'RemainingCharacterExtraction-20260829/ActionNameSources/MN_RPCT_05.action-effects.json'
    actions = {a['actionId']: a for a in read(action_path)['actions']}
    table_path = SOURCE / 'WorldObjectExtraction-20260907/EFTable_SkillEffect.db'
    database = sqlite3.connect('file:' + table_path.as_posix() + '?mode=ro', uri=True)
    database.row_factory = sqlite3.Row
    groups = []
    for shape, label, notify_index, skill, radius, inner in (
        ('circle', '원형', 6, 421980404, 3, 0),
        ('innerdonut', '도넛1', 11, 421980405, 4.5, 3),
        ('outerdonut', '도넛2', 16, 421980406, 6, 4.5)):
        damage = dict(database.execute('SELECT * FROM SkillEffect WHERE PrimaryKey=?', (skill,)).fetchone())
        assert (damage['AreaRange'], damage['AreaRemoveRange']) == (radius * 100, inner * 100)
        notifies = []
        for action_id in (4219804, 4219805):
            action = actions[action_id]
            stage = action['stages'][4]
            assert stage['animationClips'][0]['clipName'].lower() == 'att_battle_17_05'
            notify = next(n for n in stage['notifies'] if n['notifyId'].endswith(f'notify-{notify_index:03d}'))
            assert notify['sourceType'] == 'PlayParticleEffect'
            payload = base64.b64decode(notify['serializedPayload']['data'])
            assert payload[47] == 1, 'Source particle notify is disabled'
            effect = next(n for n in stage['notifies'] if n['sourceType'] == 'Effect'
                and struct.unpack_from('<I', base64.b64decode(n['serializedPayload']['data']), 90)[0] == skill)
            notifies.append(dict(actionId=action_id, notifyId=notify['notifyId'],
                sourceByteOffset=notify['sourceOffset'], particleTimeSeconds=notify['localTimeSeconds'],
                damageTimeSeconds=effect['localTimeSeconds'], damageDurationSeconds=effect['durationSeconds'],
                sourceParticleSystem=notify['assetReferences'][0]['objectPath'].lower(), enabled=True))
        assert notifies[0]['sourceParticleSystem'] == notifies[1]['sourceParticleSystem']
        groups.append(dict(shape=shape, label=label, radiusM=radius, innerRadiusM=inner,
            damageSkillEffectId=skill, sourceNotifies=notifies))
    decal_notify = next(n for n in actions[4219804]['stages'][3]['notifies'] if n['sourceType'] == 'PlayDecalEffect')
    raw = base64.b64decode(decal_notify['serializedPayload']['data'])
    assert struct.unpack_from('<I', raw, 91)[0] == 2122
    assert struct.unpack_from('<I', raw, 151)[0] == 421980404
    ground_name = '10_EF_PARTICLE_SOUND_DATA_GROUND_EFFECT_GR_Mon_Circle_cond_EX_02.loa'
    ground_path, ground_raw = warning.read_archive_entries('data3.lpk', [ground_name])[ground_name]
    material = next(row for row in warning.scan_length_prefixed_strings(ground_raw, 0, len(ground_raw))
        if row['value'].startswith("MaterialInstanceConstant'"))
    material_path = material['value'].split("'")[1].lower()
    assert material_path == 'fx_m_mi_o_00.fx_mi.fx_o_de_condcircle_02_01_tr'
    offset = material['sourceOffset'] + 4 + len(material['value']) + 1
    assert struct.unpack_from('<4f', ground_raw, offset) == (100, 100, -300, 300)
    color = list(struct.unpack_from('<4f', ground_raw, offset + 20))
    assert color == [4, 1.5, 0, 1]
    return dict(sourceActionPath=action_path.as_posix(), sourceSkillEffectTablePath=table_path.as_posix(),
        groups=groups, sourceCircleWarning=dict(skillDecalId=2122, sourceGroundEffect=ground_path,
            sourceGroundEffectSha256=hashlib.sha256(ground_raw).hexdigest(), sourceMaterial=material_path,
            color=color, notifyId=decal_notify['notifyId'], startSeconds=decal_notify['localTimeSeconds'],
            durationSeconds=decal_notify['durationSeconds']))


def impact_document(group, asset_id, name):
    """Retain decoded per-occurrence scales/parameters and current native bindings."""
    occurrence = group['sourceNotifies'][0]
    stage = read(ROOT / 'out/KoukuActionEffects20260912/candidate/effect.kouku.action.mn_rpct_05.4219804.stage004.effect.json')
    elements = [copy.deepcopy(e) for e in stage['elements']
                if e['sourceNode'].startswith(occurrence['notifyId'] + '|')]
    assert len(elements) == (17 if group['shape'] == 'circle' else 18)
    library_asset = 'effect.kouku.source.' + occurrence['sourceParticleSystem']
    library = read(ROOT / 'Data/Effects/Authored' / (library_asset + '.effect.json'))
    bindings = {e['sourceNode'].split('|')[-1]: e for e in library['elements']}
    assert len(bindings) == len(library['elements'])
    basis_yaw = elements[0]['actionCueAttachment'].get('snapshotRootSourceBasisYawDegrees', 0)
    for element in elements:
        installed = bindings[element['sourceNode'].split('|')[-1]]
        assert installed['material']['sourceMaterialPath'] == element['material']['sourceMaterialPath']
        assert installed['material']['sourceProfile']['enabled']
        element['material'] = copy.deepcopy(installed['material'])
        element['resources'] = copy.deepcopy(installed['resources'])
        timing = element['detail']['timing']
        timing['startDelaySeconds'] -= occurrence['particleTimeSeconds']
        assert abs(timing['startDelaySeconds']) < 1e-5
        timing['startDelaySeconds'] = 0
        # These root-snapshot occurrences all share the same caster-relative
        # center. An independent group exposes that center as its own pivot.
        assert element['detail']['transform']['position'] == [1.5, 0, 0]
        element['detail']['transform']['position'] = [0, 0, 0]
        attachment = element['actionCueAttachment']
        assert not attachment['follow']
        assert attachment.get('snapshotRootSourceBasisYawDegrees', 0) == basis_yaw
        attachment['enabled'] = False
        attachment['snapshotRootSourceBasisYawDegrees'] = 0
    document = dict(schema=stage['schema'], version=stage['version'],
        particleSystem=copy.deepcopy(stage['particleSystem']), modelCues=[], elements=elements)
    # Keep the original common source-basis rotation after removing root attachment.
    document['particleSystem']['yawOffsetDegrees'] += basis_yaw
    # Version 15 requires this root even when the selected particle occurrences
    # carry no baked edge histories from the surrounding Action stage.
    if document['version'] == 15:
        document['runtimeExtensions'] = dict(formatVersion=1, bakedEdgeHistories=[])
    return warning.independent_document(document, asset_id, name)


def build(evidence):
    contract = source_contract()
    showtime_evidence = ROOT / 'out/KoukuShowtimeWarnings20260912'
    showtime_records = warning.compose_independent(showtime_evidence)
    authored = ROOT / 'Data/Effects/Authored'
    documents, records = [], []
    for group in contract['groups']:
        for phase in ('warning', 'impact'):
            asset_id = 'effect.kouku.gate1.blade-dance.' + group['shape'] + '.' + phase
            name = '칼날댄스_' + group['label'] + ('_예고' if phase == 'warning' else '_폭발')
            if phase == 'warning':
                template_shape = 'circle' if group['shape'] == 'circle' else 'innerdonut'
                template = read(authored / ('effect.kouku.gate3.showtime.' + template_shape + '.warning.effect.json'))
                document = warning.independent_document(template, asset_id, name)
                # Source only explicitly warns the central circle. The two
                # ring warnings are authoring aids using its source yellow
                # and the exact ring damage radii, with an editable lead.
                lead = contract['sourceCircleWarning']['durationSeconds'] if group['shape'] == 'circle' else .5
                warning.resize_warning(document, group['radiusM'], group['innerRadiusM'], lead,
                    contract['sourceCircleWarning']['color'])
                element = document['elements'][0]
                element['displayName'] = name
                identity = 'project.blade-dance.warning.' + group['shape']
                element['sourceNode'] = identity + '|' + element['material']['sourceMaterialPath']
                element['sourceTransformTrack']['sourceOccurrenceId'] = identity
                element['sourceTransformTrack']['nodes'][0]['sourceObjectPath'] = identity
            else:
                document = impact_document(group, asset_id, name)
            documents.append(document)
            source_kind = ('ORIGINAL_CIRCLE_WARNING' if group['shape'] == 'circle' else 'PROJECT_AUTHORED_RING_WARNING') if phase == 'warning' else 'ORIGINAL_ENABLED_ACTION_PARTICLE'
            records.append(dict(effectAssetId=asset_id, displayName=name,
                authoringPath='Effects/Authored/' + asset_id + '.effect.json',
                parentPath=['KoukuSaydon', '1관문', '패턴', '세이튼', '칼날댄스', '원형·도넛'],
                durationMs=warning.document_duration_ms(document, phase == 'warning'), elementCount=len(document['elements']),
                phase=phase, radiusM=group['radiusM'], innerRadiusM=group['innerRadiusM'],
                sourceDamageSkillEffectId=group['damageSkillEffectId'], sourceKind=source_kind,
                sourceNotifies=group['sourceNotifies'], removedCasterRelativeOffsetM=[1.5, 0, 0],
                sourceCircleWarning=contract['sourceCircleWarning'],
                resourceId='kakulsaydon.effect.' + hashlib.sha256(asset_id.encode()).hexdigest()[:20]))
    warning.install_independent_documents(documents)
    write(evidence / 'source_contract.json', contract)
    manifest = dict(schema='lostark.independent-pattern-effect-groups', formatVersion=1,
        installed=True, groups=showtime_records + records, manualVisualValidation='USER_PENDING',
        legacyMergedDocuments='PRESERVED_UNCHANGED', catalogAndCompositionUpdated=False,
        warningLimitation='Blade inner/outer ring warnings are project authoring aids; original Action directly warns only the circle.')
    manifest['documents'] = [dict(effectAssetId=row['effectAssetId'], displayName=row['displayName'],
        path='Data/' + row['authoringPath'], categoryPath=row['parentPath'], durationMs=row['durationMs'],
        defaultAnchorKind='MAP') for row in manifest['groups']]
    write(evidence / 'installation_manifest.json', manifest)
    print(json.dumps(dict(groups=len(manifest['groups']), authoredWritten=True,
        manifest=(evidence / 'installation_manifest.json').as_posix()), ensure_ascii=False))
    return manifest


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--evidence-root', type=Path, default=ROOT / 'out/KoukuPatternGroups20260912/blade')
    args = parser.parse_args()
    build(args.evidence_root.resolve())
