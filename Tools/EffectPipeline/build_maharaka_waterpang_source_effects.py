"""Restore the Maharaka Waterpang attack presentation from its source FX.

Two MN_ISMP_00 actions drive the match hazards the Server already applies
(Shared/Public/Gameplay/MaharakaWaterpangContract.h):

* 4225612 "arena water deluge" of the big mokoko (NPC 570941), the Waterpang
  variant of 4225601 (same clip, Arena_Attk01 water / ground systems, decal
  SkillDecal 1106): one 6 s stage whose Face / water / finish notifies become
  one full-restore document on the held mokoko. Its ground notify is a root
  document of its own: the held mokoko root stands 1.6 m above the deck
  (project frame contact adapter), so the attack sequence places the splash on
  the deck itself, at the mokoko and at five more stations around the arena,
  because the project waterfall hits the whole deck.
* 4225615 rotating cannon (NPC 570911, MN_ISMP_00-1): the start / loop / end
  stages become three documents. FX_Turn01..04 are notify names of
  root-snapshot bursts, not sockets. The jet itself is the projectile
  422560315 particle (CEFSequenceSummonsProjectileFixArea, identity local
  transform), restored as its own root document with its source snapshot
  basis; the loop sequences carry it as a V1 track that the Client turns
  with the bursts on the Server clock.

Acquisition, native material recovery, installation and projection reuse the
Inanna/vehicle cohort drivers; this module owns its evidence root, the shared
program range of both actions, the jet notify and the authored identities.
The ground telegraphs (SkillDecal 1103 rectangle, 1106 donut) are composed on
the existing LocalDecal contract from their GroundEffect records.
"""
from pathlib import Path
import argparse
import base64
import collections
import copy
import json
import math
import shutil
import sys

ROOT = Path(__file__).resolve().parents[2]
for extra in ('Tools/EffectPipeline', 'Tools/LevelPlacementExtractor',
              'Tools/VehiclePipeline', 'Tools/ModelAssetConverter'):
    sys.path.insert(0, str(ROOT / extra))
import build_esther_inanna_source_effects as drv
import build_kouku_gate1_full_restore as source
import build_vehicle_skill_effects as vehicle

EVIDENCE = ROOT / 'out/MaharakaWaterpangFX20260928'
UMODEL = Path('C:/Users/USER/OneDrive/바탕 화면/UModel/umodel_win32/umodel_lostark_v7.exe')
ACTION_LOA = ROOT / 'out/MaharakaReaudit20260926/MN_ISMP_00.Action.loa'
PROJECTILE = Path('C:/Users/USER/.claude/jobs/46aea322/tmp/watercannon_research/proj/'
                  'Common_Extra/XMLData/Projectile/422560315.loa')
JET_SYSTEM = 'FX_MN_ISMP_00.Par_G_ISMP_Attk03_WaterBomb_Loop_800_01'
JET_TEMPLATE_SYSTEM = 'FX_MN_ISMP_00.Par_G_ISMP_Attk03_WaterBomb_800_01_01'
# The longest authored fire window is 20 s; the projectile lives 24-25 s.
JET_SECONDS = 24.0
TEXTURE_ROOT = 'Effect/Maharaka/Waterpang/Textures'
MESH_ROOTS = ('Effect/Maharaka/Waterpang/Meshes', 'Effect/Esther/Inanna/Meshes', 'Effect/Esther/Thirain/Meshes',
              'Effect/Esther/Wei/Meshes', 'Effect/Esther/Ninave/Meshes', 'Effect/KoukuSaydon/FullRestore/Meshes')
# 3621..3679 is free: the tail of the installed 3584 bucket and the head of
# the 3648 bucket (3680..3688 are taken). Both buckets already own their
# Mesh/Particle wrappers and runtime rows, so no shader file or project entry
# is added. The cohort needs 34 programs.
NATIVE_FIRST, NATIVE_LAST = 3621, 3679

PROFILES = {
    'mokomoko': dict(
        archetype='NPC_MAHARAKA_MOKOMOKO', prefix='effect.maharaka.waterpang.mokomoko.',
        action=4225612, model='Character/NPC/Maharaka/MN_ISMP_00/MN_ISMP_00.wmodel',
        mesh=('mn_ismp_00', 'mesh.mn_ismp_00_sk'), display='Waterpang mokoko ',
        clips={'att_battle_1_01': (0, 'Att_Battle_1_01')}),
    'cannon': dict(
        archetype='NPC_MAHARAKA_WATERCANNON', prefix='effect.maharaka.waterpang.cannon.',
        action=4225615, model='Character/NPC/Maharaka/MN_ISMP_00-1/MN_ISMP_00-1.wmodel',
        mesh=('mn_ismp_00', 'mesh.mn_ismp_00-1_sk'), display='Waterpang cannon ',
        clips={'att_battle_3_01': (0, 'Att_Battle_3_01'), 'att_battle_3_04': (2, 'Att_Battle_3_04')}),
    'jet': dict(
        archetype='NPC_MAHARAKA_WATERCANNON', prefix='effect.maharaka.waterpang.cannon.',
        action=4225615, model='Character/NPC/Maharaka/MN_ISMP_00-1/MN_ISMP_00-1.wmodel',
        mesh=('mn_ismp_00', 'mesh.mn_ismp_00-1_sk'), display='Waterpang cannon ',
        clips={'jet': (1, 'Projectile 422560315 WaterBomb_Loop_800_01')}),
}
# The 24 s loop stage bursts WaterBomb_800_01 every 0.5 s under the notify
# names FX_Turn01..04 (one per 6 s quarter). One document for the whole stage
# exceeds the 8192-particle codec budget, so each quarter is its own document
# keeping the source times; all four start with the loop clip.
LOOP_PARTS = 4
LOOP_PART_SECONDS = 6.0
for part in range(LOOP_PARTS):
    PROFILES['loop%d' % part] = dict(PROFILES['cannon'], clips={
        'att_battle_3_02.part%d' % part: (1, 'Att_Battle_3_02 %g-%gs' % (part * LOOP_PART_SECONDS, (part + 1) * LOOP_PART_SECONDS))})
# The arena ground splash notify of 4225612 alone (source time kept), placed as
# a root document on the deck by the mokomoko attack sequence.
GROUND_SYSTEM = 'FX_MN_ISMP_00.Par_G_ISMP_Arena_Attk01_WaterGround_01'
PROFILES['ground'] = dict(PROFILES['mokomoko'], prefix='effect.maharaka.waterpang.mokomoko.ground.',
                          display='Waterpang mokoko ground splash ')


def configure(name, evidence_root=EVIDENCE):
    profile = PROFILES[name]
    vehicle.UMODEL = UMODEL
    drv.PROFILE = 'chain'
    drv.ARCHETYPE = profile['archetype']
    drv.ASSET_PREFIX = profile['prefix']
    drv.ACTION_ID = profile['action']
    drv.ACTION_PROFILE = 'MN_ISMP_00'
    drv.ACTION_LOA = ACTION_LOA
    drv.CLIPS = profile['clips']
    drv.SOURCE_MESH = profile['mesh']
    drv.NPC_MODEL = profile['model']
    drv.TEXTURE_ROOT = TEXTURE_ROOT
    drv.MESH_ROOTS = MESH_ROOTS
    drv.zone_actions = jet_actions if name == 'jet' else ground_actions if name == 'ground' else \
        mokomoko_actions if name == 'mokomoko' else \
        (lambda evidence, part=int(name[4:]): loop_actions(evidence, part)) if name.startswith('loop') else plain_actions
    keep_byte47_notifies()
    return evidence_root / name


def keep_byte47_notifies():
    """PlayParticleEffect byte 47 is read as 'enabled' by the shared decoder.
    Across MN_ISMP_00 it is 0 on ~1,400 of 1,500 notifies, independent of the
    socket (G01), and 4225601's water/ground systems it marks 0 are visible in
    the source video. This cohort keeps those notifies and records the byte."""
    original = source.decode_typed_payload
    if getattr(original, 'keepsByte47Notifies', False):
        return

    def decode(source_type, payload, contract, references, labels):
        cue = original(source_type, payload, contract, references, labels)
        if isinstance(cue, dict) and cue.get('schema') == 'lostark.cef-play-particle-effect-header' \
                and cue.get('sourceByteOffset') == 47 and not cue.get('enabled'):
            cue.update(enabled=True, sourceByte47Semantic='UNRESOLVED_NOT_DISABLE')
        return cue
    decode.keepsByte47Notifies = True
    decode.resolvesBoneAnchors = getattr(original, 'resolvesBoneAnchors', False)
    source.decode_typed_payload = decode


def plain_actions(evidence):
    """The extracted action file as-is: these stages add no synthetic notifies."""
    actions = evidence / 'actions' / f'{drv.ACTION_PROFILE}.action-effects.json'
    if not actions.is_file():
        actions = drv.extract_actions(evidence)
    return actions


def jet_actions(evidence):
    """Stage 1 of 4225615 holding only the projectile jet: the root 'FX'
    notify of the same stage is the payload template, its ParticleSystem
    reference swapped for the projectile's (identity transform, scale 1)."""
    import struct
    raw_projectile = PROJECTILE.read_bytes()
    reference = f"ParticleSystem'{JET_SYSTEM}'".encode('ascii')
    at = raw_projectile.find(reference)
    assert at > 0, 'projectile jet reference missing'
    block = raw_projectile[at + len(reference) + 1:]
    assert struct.unpack_from('<3f', block, 76) == (0.0, 0.0, 0.0), 'jet has a local offset'
    assert struct.unpack_from('<3f', block, 136) == (1.0, 1.0, 1.0), 'jet has a local scale'
    actions = plain_actions(evidence)
    document = source.read(actions)
    action = next(a for a in document['actions'] if int(a['actionId']) == drv.ACTION_ID)
    stage = next(s for s in action['stages'] if s['stageIndex'] == 1)
    template = next(n for n in stage['notifies'] if n['sourceType'] == 'PlayParticleEffect'
                    and n['assetReferences'][0]['objectPath'] == JET_TEMPLATE_SYSTEM)
    raw = drv.replace_particle_reference(base64.b64decode(template['serializedPayload']['data']),
                                         JET_TEMPLATE_SYSTEM, JET_SYSTEM)
    row = copy.deepcopy(template)
    row.update(notifyId=template['notifyId'] + '-projectile-jet', localTimeSeconds=0.0,
        sourceEndSeconds=JET_SECONDS, durationSeconds=JET_SECONDS,
        assetReferences=[dict(className='ParticleSystem', objectPath=JET_SYSTEM)],
        serializedLabels=['jet', 'FX', 'CEFParticleData', f"ParticleSystem'{JET_SYSTEM}'"],
        synthetic=dict(kind='CEFSequenceSummonsProjectileFixArea CEFProjectileParticleData',
            projectile=PROJECTILE.name, clonedFrom=template['notifyId'], system=JET_SYSTEM))
    row['serializedPayload'] = dict(template['serializedPayload'], data=base64.b64encode(raw).decode('ascii'),
        byteSize=len(raw), sha256='synthetic')
    stage['notifies'] = [row]
    patched = actions.with_name(f'{drv.ACTION_PROFILE}.action-effects.jet.json')
    source.write(patched, document)
    return patched


def mokomoko_actions(evidence):
    """Stage 0 of 4225612 without its ground splash notify (placed separately)."""
    actions = plain_actions(evidence)
    document = source.read(actions)
    action = next(a for a in document['actions'] if int(a['actionId']) == drv.ACTION_ID)
    stage = next(s for s in action['stages'] if s['stageIndex'] == 0)
    before = len(stage['notifies'])
    stage['notifies'] = [n for n in stage['notifies'] if not (n['sourceType'] == 'PlayParticleEffect'
                         and n['assetReferences'][0]['objectPath'] == GROUND_SYSTEM)]
    assert len(stage['notifies']) == before - 1, 'arena ground notify missing'
    patched = actions.with_name(f'{drv.ACTION_PROFILE}.action-effects.mokomoko.json')
    source.write(patched, document)
    return patched


def ground_actions(evidence):
    """Stage 0 of 4225612 holding only its arena ground splash notify."""
    actions = plain_actions(evidence)
    document = source.read(actions)
    action = next(a for a in document['actions'] if int(a['actionId']) == drv.ACTION_ID)
    stage = next(s for s in action['stages'] if s['stageIndex'] == 0)
    stage['notifies'] = [n for n in stage['notifies'] if n['sourceType'] == 'PlayParticleEffect'
                         and n['assetReferences'][0]['objectPath'] == GROUND_SYSTEM]
    assert len(stage['notifies']) == 1, 'arena ground notify missing'
    patched = actions.with_name(f'{drv.ACTION_PROFILE}.action-effects.ground.json')
    source.write(patched, document)
    return patched


# GroundEffect GR_Mon_Donut_behit_01 (SkillDecal 1006) has its own parent
# material, so it is lowered with this cohort through the LocalDecal adapter
# the Kouku GroundEffect warnings use; it is not a Cascade emitter.
DONUT_MATERIAL = 'fx_m_mi_o_00.fx_mi.fx_o_de_behitmondonut_02_01_tr'
DONUT_ELEMENT = 'maharaka.waterpang.mokomoko.donut.warning'
DONUT_ADAPTER = 'project.groundeffect.adapter.maharaka.donut'
REQUIRED_MODULE = 'engine.default__particlemodulerequired'


def loop_actions(evidence, part):
    """Stage 1 of 4225615 holding one 6 s quarter of its particle notifies; the
    last quarter also keeps the final burst at exactly 24 s."""
    actions = plain_actions(evidence)
    document = source.read(actions)
    action = next(a for a in document['actions'] if int(a['actionId']) == drv.ACTION_ID)
    stage = next(s for s in action['stages'] if s['stageIndex'] == 1)
    start, end = part * LOOP_PART_SECONDS, (part + 1) * LOOP_PART_SECONDS
    last = part == LOOP_PARTS - 1
    stage['notifies'] = [n for n in stage['notifies'] if n['sourceType'] == 'PlayParticleEffect' and
        (start <= n['localTimeSeconds'] < end or (last and n['localTimeSeconds'] >= end))]
    patched = actions.with_name(f'{drv.ACTION_PROFILE}.action-effects.loop{part}.json')
    source.write(patched, document)
    return patched


def native_environment():
    """The vehicle texture hook predates the resource_root keyword; the
    installed Resources root stays the destination. The engine noise input of
    the GroundEffect material resolves to the source white texture, exactly as
    build_kouku_showtime_warning_groups does."""
    vehicle.TEXTURE_ROOT = TEXTURE_ROOT
    pattern = vehicle.native_environment()
    hook = pattern.prepare_textures
    pattern.prepare_textures = lambda evidence, out, resource_root=None: hook(evidence, out)
    original_object = pattern.obj
    if not getattr(original_object, 'maharakaGroundNoise', False):
        def resolve_ground_texture(path):
            if path == 'engineresources.defaulttexture':
                return original_object('fx_tex_00.fx_a_blankwhite_01')
            return original_object(path)
        resolve_ground_texture.maharakaGroundNoise = True
        pattern.obj = resolve_ground_texture
    return pattern


def native(first, last):
    """One cohort for every profile: no earlier reviewed contract exists on
    this machine, so each source material x vertex factory permutation is
    lowered once inside this cohort's range and shared by all documents."""
    configure('mokomoko')
    pattern = native_environment()
    root = EVIDENCE / 'material'
    root.mkdir(parents=True, exist_ok=True)
    drv.exact_texture_reuse(pattern, root / 'native')
    excluded, occurrences, records, defaults, seen = [], [], {}, {}, set()
    for name, profile in PROFILES.items():
        for clip in profile['clips']:
            folder = EVIDENCE / name / 'stages' / clip
            module_inputs = source.read(folder / 'source_module_inputs.json')['records']
            for occurrence in source.read(folder / 'source_occurrences.json'):
                if occurrence['elementId'] in seen:
                    continue
                seen.add(occurrence['elementId'])
                reason = drv.native_occurrence_excluded(occurrence, module_inputs)
                if reason:
                    excluded.append(dict(elementId=occurrence['elementId'], sourceEmitter=occurrence['sourceEmitter'],
                        rendererShape=occurrence['rendererShape'], reason=reason))
                    continue
                occurrences.append(occurrence)
            records.update(module_inputs)
            for row in source.read(folder / 'source_class_defaults.json')['records']:
                defaults[row['fullPath']] = row
    records[REQUIRED_MODULE] = dict(fullPath=REQUIRED_MODULE, classPath='engine.particlemodulerequired',
                                    archetypeFullPath=None, properties={})
    records[DONUT_ADAPTER] = dict(fullPath=DONUT_ADAPTER, classPath='project.GroundEffectLocalDecalAdapter', properties={})
    occurrences.append(dict(elementId=DONUT_ELEMENT, sourceEmitter=DONUT_ADAPTER, moduleOrder=[REQUIRED_MODULE],
        sourceMaterial=DONUT_MATERIAL, rendererShape='decal', sourceMesh='',
        sourceKind='ORIGINAL_GROUND_EFFECT_MATERIAL_WITH_PROJECT_DECAL_CARRIER_ADAPTER'))
    source.write(root / 'source_occurrences.json', occurrences)
    source.write(root / 'native_input_exclusions.json', excluded)
    source.write(root / 'source_module_inputs.json', dict(records=records))
    source.write(root / 'source_class_defaults.json', dict(records=[defaults[k] for k in sorted(defaults)]))
    reviewed = root / 'reviewed'
    vehicle.native_materials_prepare(pattern, root, first, last,
        [reviewed] if (reviewed / 'native_runtime_contract.json').is_file() else [])
    contract = source.read(root / 'native' / 'native_runtime_contract.json')
    failures = source.read(root / 'native' / 'source_material_failures.json')
    summary = dict(programs=len(contract['programs']), deferred=len(contract['deferredPrograms']),
        sourceFailures=len(failures), excludedOccurrences=len(excluded), first=first, last=last,
        shapes=dict(collections.Counter(p['rendererShape'] for p in contract['programs'])),
        blends=dict(collections.Counter(p['nativeBlend'] for p in contract['programs'])))
    source.write(root / 'native_summary.json', summary)
    print(json.dumps(summary, ensure_ascii=False))


def install_native(first, last):
    configure('mokomoko')
    native_environment()
    vehicle.VEHICLE_NATIVE_FIRST, vehicle.VEHICLE_NATIVE_LAST = first, last
    vehicle.install_native(EVIDENCE)
    route_receiver_isolated_distortion(first, last)


DISTORTION_ORDINARY = 'output.Distortion=float4(accumulated.xy-accumulated.zw,0.f,0.f);'
DISTORTION_ISOLATED = 'output.Distortion=float4(0.f,0.f,accumulated.xy-accumulated.zw);'


def route_receiver_isolated_distortion(first, last):
    """The jets and the head orb stack their distortion around the mokoko; on the
    ordinary RG offset the saturated sum pulled the mokoko head into a second
    image. The receiver-isolated BA offset (install_kouku_gate1_native_shaders)
    keeps the source expressions and only moves offsets on one depth plane."""
    import re
    shaders = ROOT / 'Client/Bin/ShaderFiles'
    for name in ('Shader_EffectArtistNativeDispatchKoukuNativeCases3584Part2.hlsli',
                 'Shader_EffectArtistNativeDispatchKoukuNativeCases3648.hlsli'):
        path = shaders / name
        text = path.read_bytes().decode('ascii')

        def route(match):
            block = match.group(0)
            program = int(re.search(r'case (\d+)u:', block)[1])
            if not first <= program <= last or DISTORTION_ORDINARY not in block:
                return block
            assert block.count(DISTORTION_ORDINARY) == 1, (name, program)
            return block.replace(DISTORTION_ORDINARY, DISTORTION_ISOLATED)
        routed = re.sub(r'#if [^\r\n]*\r\n    case \d+u:.*?#endif', route, text, flags=re.S)
        if routed != text:
            path.write_bytes(routed.encode('ascii'))


JET_ID = 'effect.maharaka.waterpang.cannon.jet.full.restore'


LOOP_PART_IDS = {f'effect.maharaka.waterpang.cannon.att_battle_3_02.part{index}.full.restore' for index in range(4)}
SPRAY_SYSTEM = 'par_g_ismp_attk03_waterbomb_800_01_01'


def pin_spray_to_stream_axis(document):
    """WaterBomb_800_01_01 turns its spray by a StartVelocity curve (-181 degrees
    per 6 s emitter loop about source Z) that ignores the Server turn, and the
    CCW loop reuses these CW documents, so it drew a second line turning at
    its own rate and sense. The loop instance already turns every burst onto
    the Server line: pin each sample onto the stream's source Y axis on the
    side it starts from. The two emitters keep their opposite branches and
    sampled speeds; repeating this is a no-op."""
    pinned = 0
    for element in document['elements']:
        if SPRAY_SYSTEM not in element['sourceNode']:
            continue
        for module in element['sourceRecipe']['modules']:
            if module['className'] != 'particlemodulevelocity':
                continue
            for distribution in module['distributions']:
                table = distribution.get('lookupTable') or []
                # Only the time curve turns; a uniform range (time scale 0) is jitter.
                if (distribution['propertyPath'] != 'startvelocity' or distribution.get('componentCount') != 3
                        or not distribution.get('lookupTableTimeScale') or len(table) <= 8 or (len(table) - 2) % 3):
                    continue
                samples = [table[offset:offset + 3] for offset in range(2, len(table), 3)]
                start = round(-distribution['lookupTableStartTime'] * distribution['lookupTableTimeScale'])
                side = 1.0 if samples[max(0, min(len(samples) - 1, start))][1] >= 0 else -1.0
                flat = [value for sample in samples for value in (0.0, side * math.hypot(*sample), 0.0)]
                distribution['lookupTable'] = [min(flat), max(flat)] + flat
                speed = sum(math.hypot(*sample) for sample in samples) / len(samples) / 100.0
                # Portable summary in Client metres: source (x, y, z) -> (x, z, -y).
                element['detail']['particle'].update(initialVelocityMin=[0.0, 0.0, -side * speed],
                                                     initialVelocityMax=[0.0, 0.0, -side * speed])
                pinned += 1
    return pinned


MOKOMOKO_ID = 'effect.maharaka.waterpang.mokomoko.att_battle_1_01.full.restore'
GROUND_ID = 'effect.maharaka.waterpang.mokomoko.ground.att_battle_1_01.full.restore'
# (head followers, root snapshots) of 4225612 and of its ground notify alone.
MOKOMOKO_RIG = (22, 0)
GROUND_RIG = (0, 17)
# MN_ISMP_00 and MN_ISMP_00-1 are centimetre WModels without the retail x100
# root node; the WorldSequence object builds them with PreTransform
# Scale(modelPreScale 0.01), so every b_ismp_root combined matrix carries 0.01
# (Model.cpp combined order, WorldSequencePlayer_Objects
# Sample_ObjectEffectAttachments; measured 0.01000 on both models through
# att_battle_1_01 / 3_01 / 3_02 / 3_04). The metre socket and element offsets
# are undone here, as the vehicle wing bones are.
OBJECT_RIG_BASIS_INVERSE = 100.0
# (followers, root snapshots) each cannon document holds; its root snapshots keep
# the source -90 basis the loop bursts are turned on (Server line contract).
CANNON_RIG = {
    'effect.maharaka.waterpang.cannon.att_battle_3_01.full.restore': (9, 0),
    'effect.maharaka.waterpang.cannon.att_battle_3_02.part0.full.restore': (11, 338),
    'effect.maharaka.waterpang.cannon.att_battle_3_02.part1.full.restore': (0, 338),
    'effect.maharaka.waterpang.cannon.att_battle_3_02.part2.full.restore': (0, 338),
    'effect.maharaka.waterpang.cannon.att_battle_3_02.part3.full.restore': (0, 366),
    'effect.maharaka.waterpang.cannon.att_battle_3_04.full.restore': (17, 0),
}


# The held mokoko (project frame contact adapter) carries its FX_01 socket 1.10 m
# higher above the deck than the source composition: at the 2.29 s water spawn
# the three tornado columns' bases stood at y 23.55 / 23.55 / 23.75 over the
# 22.43 deck top (arena_sim). The socket's local +Y is world up at that pose
# (b_ismp_root keeps its yaw only), so one shared local drop keeps the source
# column spacing and puts the two lower columns on the deck.
ARENA_COLUMN_SYSTEM = 'par_g_ismp_arena_attk01_water_01'
ARENA_COLUMN_EMITTERS = ('particlespriteemitter_22', 'particlespriteemitter_7', 'particlespriteemitter_23')
ARENA_COLUMN_DROP_M = 1.10


def seat_arena_columns(document):
    """Lower the 4225612 tornado water columns by ARENA_COLUMN_DROP_M along the
    socket up axis; returns the column count, repeating it is a no-op."""
    seated = 0
    for element in document['elements']:
        node = element['sourceNode'].split('|')[-1]
        if ARENA_COLUMN_SYSTEM not in node or node.rsplit('.', 1)[-1] not in ARENA_COLUMN_EMITTERS:
            continue
        position = element['detail']['transform']['position']
        assert abs(position[0]) < 1e-6 and abs(position[2]) < 1e-6 and position[1] in (0.0, -0.0, -ARENA_COLUMN_DROP_M), position
        position[1] = -ARENA_COLUMN_DROP_M
        seated += 1
    return seated


def bind_object_rig(document, root_basis=None):
    """Place a MN_ISMP_00 action on its held WorldSequence object.

    * Head followers (the FX_01 socket or the b_ismp_root bone itself): scale and
      position by the 0.01 bone basis inverse, and the source socket Roll (UE X
      axis, -16384 = -90 deg) as a rotation about the Y-mirrored bone X axis (+90).
      The shared socket contract stored it as a rotation about Z, which turned the
      mokoko water 90 degrees off its facing. The bone itself has no rotation.
    * Root snapshots: root_basis replaces the snapshot basis when given. The
      mokoko object's +X already is the source actor forward (0); the cannon keeps
      the source -90 its bursts are turned on (None).
    Returns (followers, root snapshots); repeating it is a no-op."""
    followers = roots = 0
    for element in document['elements']:
        attachment = element['actionCueAttachment']
        if not attachment['enabled']:
            continue
        if attachment['follow']:
            socket = attachment['socketLocalTransform']
            assert attachment['runtimeBoneName'] == 'b_ismp_root' and \
                attachment['runtimeAnchorSlotId'] in ('FX_01', 'B_ISMP_Root'), attachment
            if socket['scale'] == [1.0, 1.0, 1.0]:
                pitch, yaw, roll = socket['rotationDegrees']
                # Only the single-axis source rotator is mapped; FX_01 has no pitch/yaw.
                assert pitch == 0 and yaw == 0, ('head socket gained pitch/yaw', socket)
                socket.update(position=[v * OBJECT_RIG_BASIS_INVERSE for v in socket['position']],
                              rotationDegrees=[roll, 0.0, 0.0], scale=[OBJECT_RIG_BASIS_INVERSE] * 3)
            assert socket['scale'] == [OBJECT_RIG_BASIS_INVERSE] * 3 and socket['rotationDegrees'][1:] == [0.0, 0.0]
            followers += 1
        else:
            if root_basis is not None:
                attachment['snapshotRootSourceBasisYawDegrees'] = root_basis
            roots += 1
    return followers, roots


AUTHORED = ROOT / 'Data/Effects/Authored'
CATALOG = ROOT / 'Data/Effects/EffectCatalog.json'
RESOURCES = ROOT / 'Client/Bin/Resources'
MESH_ROOT = 'Effect/Maharaka/Waterpang/Meshes'
ASSET_PREFIX = 'effect.maharaka.waterpang.'
RECTANGLE_ID = ASSET_PREFIX + 'cannon.telegraph'
DONUT_ID = ASSET_PREFIX + 'mokomoko.telegraph'
RECTANGLE_TEMPLATE = AUTHORED / 'effect.kouku.gate3.showtime.rectangle.warning.effect.json'
DONUT_TEMPLATE = AUTHORED / 'effect.kouku.common.circus.innerdonut.warning.effect.json'
# 10_EF_PARTICLE_SOUND_DATA_GROUND_EFFECT_GR_Mon_{Rectangle_cond,Donut_behit}_01.loa
# (data3.lpk) active colors, and the PlayDecalEffect tail fields read in the
# Kouku named order (Time, Duration, BlendIn, Scale, Fill, BlendOut).
RECTANGLE_COLOR = [0.009999999776482582, 0.10000000149011612, 1.0, 3.0]
# GR_Mon_Donut_cond_01 (SkillDecal 1106 of 4225612), MaterialInstanceConstant
# FX_O_De_CondMonDonut_02_01_Tr: the same parent the donut template carries.
DONUT_COLOR = [0.009999999776482582, 0.10000000149011612, 1.0, 3.0]
BLEND_IN, BLEND_OUT = 0.1, 0.2


def mesh_asset(source_mesh):
    """Installed full-restore meshes live flat (Kouku) or under the package folder (Esther)."""
    package_name, relative = source_mesh.split('.', 1)
    name = relative.rsplit('.', 1)[-1]
    for mesh_root in MESH_ROOTS:
        for asset in (f'{mesh_root}/{package_name.upper()}/{name}.wmodel', f'{mesh_root}/{name}.wmodel'):
            if (RESOURCES / asset).is_file():
                return asset
    raise AssertionError(('Waterpang source mesh is not installed', source_mesh))


def ensure_meshes(evidence):
    """Cook source meshes no earlier cohort installed, through the vehicle geometry contract."""
    vehicle.MESH_ROOT = MESH_ROOT
    for clip in drv.CLIPS:
        folder = evidence / 'stages' / clip
        missing = []
        for occurrence in source.read(folder / 'source_occurrences.json'):
            if not occurrence['sourceMesh']:
                continue
            try:
                mesh_asset(occurrence['sourceMesh'])
            except AssertionError:
                missing.append(occurrence)
        if missing:
            vehicle.prepare_geometry(folder, missing)


def key(time, values):
    return dict(timeSeconds=time, value=values, arriveTangent=[0] * len(values),
                leaveTangent=[0] * len(values), interpolation='linear')


def set_scalars(element, overrides):
    profile = element['material']['sourceProfile']
    names = {p['name'] for p in profile['scalars']}
    assert set(overrides) <= names, ('decal material lacks parameters', sorted(set(overrides) - names))
    for param in profile['scalars']:
        if param['name'] in overrides:
            param['value'] = overrides[param['name']]


def ground_decal(template_path, asset_id, display, element_id, width, length, window, fill, color,
                 scalars, fill_from, material=None):
    """One LocalDecal on an installed GroundEffect warning: the source window,
    fill and blend times, size and active color replace the template's."""
    document = source.read(template_path)
    assert len(document['elements']) == 1 and not document.get('modelCues')
    document.update(effectAssetId=asset_id, displayName=display)
    document.pop('sourceModelPreview', None)
    element = document['elements'][0]
    element.update(id=element_id, groupId=asset_id, displayName=display)
    if material is not None:
        element['material'] = copy.deepcopy(material)
    element['sourceNode'] = 'project.groundeffect.adapter.' + element_id + '|' + element['material']['sourceMaterialPath']
    detail, recipe = element['detail'], element['sourceRecipe']
    detail['timing'].update(startDelaySeconds=0, lifeTimeSeconds=window)
    detail['color']['multiply'] = color
    detail['decal'].update(size=[width, length])
    detail['particle'].update(lifeTimeSeconds=[window] * 2, startSize=[width, length], endSize=[width, length])
    recipe.update(emitterDelaySeconds=0, emitterDurationSeconds=window, emitterLoopCount=1)
    for module in recipe['modules']:
        for literal in module['literals']:
            if literal['propertyPath'] == 'emitterduration':
                literal['value'] = window
        for distribution in module['distributions']:
            if distribution['propertyPath'] == 'lifetime':
                distribution.update(defaultMinimum=[window, 0, 0, 0], defaultMaximum=[window, 0, 0, 0], keys=[])
            elif distribution['propertyPath'] == 'startsize':
                # LocalDecal Size.x/Size.y are the X/Z projector axes (cm).
                value = [width * 100, length * 100, width * 100, 0]
                distribution.update(defaultMinimum=value, defaultMaximum=value, keys=[])
    track = element['sourceTransformTrack']
    track['alphaScaleKeys'] = [key(0, [0] * 3), key(BLEND_IN, [1] * 3), key(window - BLEND_OUT, [1] * 3), key(window, [0] * 3)]
    track['materialParameterTracks'] = [dict(name='inner', kind='SCALAR', keys=[key(0, [fill_from]), key(fill, [1])])]
    set_scalars(element, scalars)
    return document


def telegraph_documents():
    # SkillEffect 422560333/334: two 750 x 90 cm boxes at 0 and 180 degrees are
    # one 15 m x 0.9 m line through the cannon; window 1.5 s, fill 1.3 s.
    rectangle = ground_decal(RECTANGLE_TEMPLATE, RECTANGLE_ID, 'Waterpang cannon telegraph (SkillDecal 1103)',
        RECTANGLE_ID + '.decal', 0.9, 15.0, 1.5, 1.3, RECTANGLE_COLOR,
        dict(inner=0, decal_drawscale_x=0.9, decal_drawscale_y=15.0), 0)
    # SkillDecal 1106 (4225612 PlayDecalEffect, window 2.3 s, fill 2.0 s) on the
    # project hit volume: the whole deck, one full disc (no remove range, 360
    # degrees) of the contract deck radius at the cannon centre.
    radius = contract_constant('MAHARAKA_WATERPANG_DECK_RADIUS_M')
    donut = ground_decal(DONUT_TEMPLATE, DONUT_ID, 'Waterpang mokoko telegraph (SkillDecal 1106, whole deck)',
        DONUT_ID + '.decal', 2 * radius, 2 * radius, 2.3, 2.0, DONUT_COLOR,
        dict(thickness=0.0, inner=0.0, decal_drawscale=2 * radius, angle=1.0), 0.0)
    assert donut['elements'][0]['material']['sourceMaterialPath'] == DONUT_COND_MATERIAL
    return [rectangle, donut]


DONUT_COND_MATERIAL = 'fx_m_mi_o_00.fx_mi.fx_o_de_condmondonut_02_01_tr'


def project(install):
    reviewed = EVIDENCE / 'material' / 'reviewed'
    deferred = {identity: row for row in source.read(reviewed / 'native_deferred_programs.json')
                for identity in row.get('occurrences', [])}
    installed = drv.installed_native_materials()
    documents, rows = [], []
    for name, profile in PROFILES.items():
        evidence = configure(name)
        drv.mesh_asset = mesh_asset
        shutil.copytree(reviewed, evidence / 'material' / 'reviewed', dirs_exist_ok=True)
        ensure_meshes(evidence)
        for clip in profile['clips']:
            document, row = drv.project_clip(evidence, clip, installed, deferred)
            document['displayName'] = profile['display'] + clip
            if document['effectAssetId'] in LOOP_PART_IDS:
                assert pin_spray_to_stream_axis(document) == 2, 'loop part lost its spray pair'
            if document['effectAssetId'] == MOKOMOKO_ID:
                assert bind_object_rig(document, root_basis=0) == MOKOMOKO_RIG, 'mokoko attachment inventory changed'
                assert seat_arena_columns(document) == 3, 'arena water columns changed'
            if document['effectAssetId'] == GROUND_ID:
                assert bind_object_rig(document, root_basis=0) == GROUND_RIG, 'ground attachment inventory changed'
            if document['effectAssetId'] in CANNON_RIG:
                assert bind_object_rig(document) == CANNON_RIG[document['effectAssetId']], \
                    ('cannon attachment inventory changed', document['effectAssetId'])
            documents.append(document)
            rows.append(row)
    documents += telegraph_documents()
    writes = [(AUTHORED / (d['effectAssetId'] + '.effect.json'),
               (json.dumps(d, ensure_ascii=False, indent=2, allow_nan=False) + '\n').encode('utf8')) for d in documents]
    catalog = source.read(CATALOG)
    catalog['effects'] = [row for row in catalog['effects'] if not row['effectAssetId'].startswith(ASSET_PREFIX)]
    catalog['effects'] += [dict(effectAssetId=d['effectAssetId'], payloadKind='DIRECT_AUTHORED_DOCUMENT',
                                authoringPath='Effects/Authored/' + d['effectAssetId'] + '.effect.json') for d in documents]
    writes.append((CATALOG, (json.dumps(catalog, ensure_ascii=False, indent=2) + '\n').encode('utf8')))
    keep = {d['effectAssetId'] + '.effect.json' for d in documents}
    stale = [p for p in AUTHORED.glob(ASSET_PREFIX + '*.effect.json') if p.name not in keep]
    changed = []
    for path, payload in writes:
        if not path.exists() or path.read_bytes() != payload:
            changed.append(path.relative_to(ROOT).as_posix())
            if install:
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(payload)
    if install:
        for path in stale:
            path.unlink()
    source.write(EVIDENCE / 'projection_installation.json', dict(installed=install, changedPaths=changed,
        removedPaths=[p.relative_to(ROOT).as_posix() for p in stale], documents=rows))
    for row in rows:
        print('%-40s elements=%-4d deferred=%-3d duration=%dms programs=%d attachments=%s' % (
            row['effectAssetId'], row['elementCount'], len(row['deferredEmitters']), row['durationMs'],
            len(row['nativePrograms']), row['attachments']))
    print(('installed' if install else 'candidate') + ' files changed', len(changed))


SEQUENCES = ROOT / 'Data/Maps/Authoring/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.worldsequences.json'
CAMERA_SHOTS = ROOT / 'Data/Maps/Authoring/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.camerashots.json'
CONTRACT = ROOT / 'Shared/Public/Gameplay/MaharakaWaterpangContract.h'
# 4225601 AKEvents Cast1 (0 s) / Shot1 (2.2 s) for MN_ISMP_00 (Water1). The
# source RanSeqCntr picks one of three equal variants; a World sound track holds
# one asset, so the first catalog variant is authored and the editor may swap it.
MOKOMOKO_SOUNDS = (
    ('sound.mokomoko.cast1', 'Sound/Maharaka/S_MOB_MOCOCOWATER1/mococowater1_attack01_cast1__884290698.wav', 0, 3133),
    ('sound.mokomoko.shot1', 'Sound/Maharaka/S_MOB_MOCOCOWATER1/mococowater1_attack01_shot1__170625403.wav', 2200, 3800),
)
HOLDS = {
    'mokomoko': ('sequence.maharaka.waterpang.source.intro15.mokomoko', 'world.object.maharaka.waterpang.source.intro15.mokomoko',
                 'npc.maharaka.source57009.actor100'),
    'cannon': ('sequence.maharaka.waterpang.source.intro15.cannon', 'world.object.maharaka.waterpang.source.intro15.cannon',
               'npc.maharaka.source57009.actor188'),
}
# (instance suffix, actor, clip, loop, duration ms, effect clip): the Client
# plays these on the Server hazard clock (MaharakaWaterpangPresentation).
ATTACKS = (
    ('mokomoko', 'mokomoko', 'att_battle_1_01', False, 6000, 'att_battle_1_01'),
    ('cannon.start', 'cannon', 'att_battle_3_01', False, 1500, 'att_battle_3_01'),
    ('cannon.loop.cw', 'cannon', 'att_battle_3_02', True, 24000, 'att_battle_3_02.part*'),
    ('cannon.loop.ccw', 'cannon', 'att_battle_3_03', True, 24000, 'att_battle_3_02.part*'),
    ('cannon.end', 'cannon', 'att_battle_3_04', False, 1000, 'att_battle_3_04'),
)


# The ground splash sprites sit 0.35 m below their root; the deck top is 22.43 m
# (WaterpangEntry navigation), so the root is raised to keep them 2 cm above it.
GROUND_ROOT_Y = 22.43 + 0.35 + 0.02
GROUND_STATIONS = 6


def contract_constant(name):
    """A float of the shared Server/Client hazard contract, so the authored
    telegraph and jet placement start on exactly the Server geometry."""
    import re
    match = re.search(r'\b%s = (-?[0-9.]+)f;' % re.escape(name), CONTRACT.read_text(encoding='ascii'))
    assert match, ('contract constant missing', name)
    return float(match[1])


def hazard_rows(suffix, templates):
    """Tracks that used to be placed by MaharakaWaterpangPresentation code, now
    authored on the attack sequences (World Object V1 tracks and sound tracks):

    * cannon.start: SkillDecal 1103 rectangle, 0-1.5 s, on the Server line at the
      telegraph (cannon yaw + 90, no turn yet), world yaw (inheritObjectRotation
      off) at the cannon centre on the deck.
    * cannon.loop.cw/ccw: the projectile jet, 0-24 s looping its native emitters,
      on the held cannon like the bursts; its source -90 snapshot basis plus this
      180 degree track yaw equals the former Ground_Root(S - 180), and the effect
      turn the Client applies to the loop instances carries it with the bursts.
    * mokomoko: SkillDecal 1106 as one whole-deck disc, 0-2.3 s, at the cannon
      centre on the deck (the Server waterfall hits the whole deck); the arena
      ground splash at GROUND_STATIONS stations spaced evenly from the mokoko's
      own bearing, each at the mokoko's distance facing the centre like the
      source cast, its root raised so the lowest splash sprite rests on the deck
      top; Cast1/Shot1.
    """
    cannon_x, cannon_z = contract_constant('MAHARAKA_WATERPANG_CANNON_X'), contract_constant('MAHARAKA_WATERPANG_CANNON_Z')
    cannon_yaw = contract_constant('MAHARAKA_WATERPANG_CANNON_YAW_DEGREES')
    mokoko_x, mokoko_z = contract_constant('MAHARAKA_WATERPANG_MOKOMOKO_X'), contract_constant('MAHARAKA_WATERPANG_MOKOMOKO_Z')
    cannon_pose = templates[HOLDS['cannon'][0]]['tracks'][0]['keys'][-1]['positionOffset']
    mokoko_pose = templates[HOLDS['mokomoko'][0]]['tracks'][0]['keys'][-1]['positionOffset']
    deck_y = cannon_pose[1]
    common = dict(slotId='actor', resourceKind='V1_EFFECT', timing='TIME', startMs=0, bone='', scale=[1.0, 1.0, 1.0])
    effects, sounds = [], []
    # Six decimals keep the editor fields readable; well under a millimetre / millidegree.
    tidy = lambda values: [round(value, 6) + 0.0 for value in values]
    if suffix == 'cannon.start':
        effects.append(dict(common, effectTrackId='fx.cannon.start.telegraph', resourceId=RECTANGLE_ID, durationMs=1500,
            followObject=False, inheritObjectRotation=False,
            positionOffset=tidy([cannon_x - cannon_pose[0], 0.0, cannon_z - cannon_pose[2]]),
            rotationDegrees=tidy([0.0, cannon_yaw + 90.0, 0.0])))
    elif suffix.startswith('cannon.loop.'):
        effects.append(dict(common, effectTrackId='fx.' + suffix + '.jet', resourceId=JET_ID, durationMs=24000,
            followObject=True, inheritObjectRotation=True, loopEffectToDuration=True,
            positionOffset=[0.0, 0.0, 0.0], rotationDegrees=[0.0, 180.0, 0.0]))
    elif suffix == 'mokomoko':
        effects.append(dict(common, effectTrackId='fx.mokomoko.telegraph', resourceId=DONUT_ID, durationMs=2300,
            followObject=False, inheritObjectRotation=False,
            positionOffset=tidy([cannon_x - mokoko_pose[0], deck_y - mokoko_pose[1], cannon_z - mokoko_pose[2]]),
            rotationDegrees=[0.0, 0.0, 0.0]))
        bearing = math.degrees(math.atan2(mokoko_x - cannon_x, mokoko_z - cannon_z))
        distance = math.hypot(mokoko_x - cannon_x, mokoko_z - cannon_z)
        for station in range(GROUND_STATIONS):
            angle = bearing + station * 360.0 / GROUND_STATIONS
            x = cannon_x + distance * math.sin(math.radians(angle))
            z = cannon_z + distance * math.cos(math.radians(angle))
            # Root +X (heading 90 + yaw) faces the centre: yaw = bearing + 90.
            effects.append(dict(common, effectTrackId='fx.mokomoko.ground.%d' % station, resourceId=GROUND_ID,
                durationMs=6000, followObject=False, inheritObjectRotation=False,
                positionOffset=tidy([x - mokoko_pose[0], GROUND_ROOT_Y - mokoko_pose[1], z - mokoko_pose[2]]),
                rotationDegrees=tidy([0.0, (angle + 90.0) % 360.0, 0.0])))
        sounds += [dict(soundTrackId=track, assetId=asset, startMs=start, durationMs=window, volume=1.0)
                   for track, asset, start, window in MOKOMOKO_SOUNDS]
    return effects, sounds


def attack_rows(document):
    templates = {t['sequenceId']: t for t in document['templates']}
    new_templates, new_instances = [], []
    for suffix, actor, clip, loop, duration, effect_clip in ATTACKS:
        hold_id, object_id, npc = HOLDS[actor]
        hold = templates[hold_id]
        pose = hold['tracks'][0]['keys'][-1]
        assert hold['tracks'][0]['slotId'] == 'actor' and len(hold['tracks']) == 1
        keys = [dict(pose, timeMs=0), dict(pose, timeMs=duration)]
        effect_clips = [effect_clip.replace('*', str(part)) for part in range(LOOP_PARTS)]             if effect_clip.endswith('*') else [effect_clip]
        sequence_id = 'sequence.maharaka.waterpang.attack.' + suffix
        hazard_effects, hazard_sounds = hazard_rows(suffix, templates)
        template = dict(sequenceId=sequence_id,
            displayName='Waterpang attack / ' + suffix + ' (' + clip + ')', category='World', durationMs=duration,
            interpolation='LINEAR', tracks=[dict(slotId='actor', keys=keys)],
            animationTracks=[dict(slotId='actor', clipName=clip, startMs=0, playbackRate=1, loop=loop, holdLastFrame=True)],
            effectTracks=[dict(effectTrackId='fx.' + suffix + ('.part%d' % index if len(effect_clips) > 1 else ''),
                slotId='actor', resourceKind='V1_EFFECT', resourceId=PROFILES[actor]['prefix'] + clip_name + '.full.restore',
                timing='TIME', startMs=0, durationMs=duration, followObject=True, inheritObjectRotation=True, bone='',
                positionOffset=[0.0, 0.0, 0.0], rotationDegrees=[0.0, 0.0, 0.0], scale=[1.0, 1.0, 1.0])
                for index, clip_name in enumerate(effect_clips)] + hazard_effects)
        if hazard_sounds:
            template['soundTracks'] = hazard_sounds
        new_templates.append(template)
        new_instances.append(dict(instanceId='world.sequence.instance.maharaka.waterpang.attack.' + suffix,
            templateId=sequence_id, enabled=True, startDelayMs=0, playbackSpeed=1, anchorKind='WORLD',
            position=[0.0, 0.0, 0.0], motionEnd='HOLD',
            bindings=[dict(slotId='actor', targetKind='OBJECT_RESOURCE', targetId=object_id, previewNpcPlacementId=npc)]))
    return new_templates, new_instances


def seed_rows(existing, generated, key):
    """Add generated rows whose stable ID is absent; an existing row is the
    editor's (MapTool) and is never rewritten. Returns the rows added."""
    present = {row[key] for row in existing}
    added = [row for row in generated if row[key] not in present]
    existing.extend(added)
    return added


def replace_atomic(path, before, document, candidate_name, install, label):
    """CAS on the bytes read at the start, backup, temporary file, os.replace."""
    import hashlib
    import os
    payload = json.dumps(document, ensure_ascii=False, indent=2).encode('utf8') + b'\n'
    source.write(EVIDENCE / candidate_name, document)
    if not install:
        print(label, 'candidate written, revision', document['revision'])
        return True
    (EVIDENCE / candidate_name.replace('.candidate.', '.before.')).write_bytes(before)
    temporary = path.with_suffix('.json.tmp')
    temporary.write_bytes(payload)
    assert hashlib.sha256(path.read_bytes()).digest() == hashlib.sha256(before).digest(), 'concurrent save'
    os.replace(temporary, path)
    print(label, 'saved, revision', document['revision'])
    return True


def author_sequences(install):
    """Seed the attack rows into the latest saved document. The builder only adds
    templates, instances and tracks whose stable ID is missing: once seeded, the
    rows are the effect artist's (MapTool Camera -> cutscene -> World actor) and a
    re-run never overwrites their edits. The file is re-read and compared right
    before the atomic replace."""
    before = SEQUENCES.read_bytes()
    document = json.loads(before)
    # MapTool saves in its own serializer layout: any valid JSON is accepted and
    # the file is rewritten only when a missing row has to be added.
    templates, instances = attack_rows(document)
    changed = []
    existing = {t['sequenceId']: t for t in document['templates']}
    for template in templates:
        current = existing.get(template['sequenceId'])
        if current is None:
            document['templates'].append(template)
            changed.append(template['sequenceId'])
            continue
        for field, key in (('effectTracks', 'effectTrackId'), ('soundTracks', 'soundTrackId')):
            if template.get(field):
                rows = current.setdefault(field, [])
                changed += [template['sequenceId'] + ':' + row[key] for row in seed_rows(rows, template[field], key)]
    changed += [row['instanceId'] for row in seed_rows(document['instances'], instances, 'instanceId')]
    if not changed:
        print('World sequences already seeded')
        return False
    document['revision'] = int(document['revision']) + 1
    print('World sequence rows added:', ', '.join(changed))
    return replace_atomic(SEQUENCES, before, document, 'worldsequences.candidate.json', install, 'World sequences')


# Camera-less preview cutscenes, like the existing floor motion previews: they
# expose each attack instance in MapTool Camera so its World actor, clip,
# effect and sound rows can be played, scrubbed, edited and saved there.
ATTACK_CUTSCENES = (
    ('mokomoko', '\uc6cc\ud130\ud321 / \uacf5\uaca9 \u00b7 \ud070 \ubaa8\ucf54\ubaa8\ucf54 \ubb3c\ubcbc\ub77d (\uacf5\uaca9 \uc5f0\ucd9c \ubbf8\ub9ac\ubcf4\uae30)', 6000),
    ('cannon.start', '\uc6cc\ud130\ud321 / \uacf5\uaca9 \u00b7 \uc6cc\ud130\uce90\ub17c \uc2dc\uc791\uacfc \uc608\uace0 (\uacf5\uaca9 \uc5f0\ucd9c \ubbf8\ub9ac\ubcf4\uae30)', 1500),
    ('cannon.loop.cw', '\uc6cc\ud130\ud321 / \uacf5\uaca9 \u00b7 \uc6cc\ud130\uce90\ub17c \ud68c\uc804 \uc2dc\uacc4 (\uacf5\uaca9 \uc5f0\ucd9c \ubbf8\ub9ac\ubcf4\uae30)', 24000),
    ('cannon.loop.ccw', '\uc6cc\ud130\ud321 / \uacf5\uaca9 \u00b7 \uc6cc\ud130\uce90\ub17c \ud68c\uc804 \ubc18\uc2dc\uacc4 (\uacf5\uaca9 \uc5f0\ucd9c \ubbf8\ub9ac\ubcf4\uae30)', 24000),
    ('cannon.end', '\uc6cc\ud130\ud321 / \uacf5\uaca9 \u00b7 \uc6cc\ud130\uce90\ub17c \ub05d (\uacf5\uaca9 \uc5f0\ucd9c \ubbf8\ub9ac\ubcf4\uae30)', 1000),
)


def author_cutscenes(install):
    """Seed the attack preview cutscenes by stable ID into the saved camera
    document; existing cutscenes (and their editor changes) are kept."""
    before = CAMERA_SHOTS.read_bytes()
    document = json.loads(before)
    generated = [dict(cutsceneId='cutscene.maharaka.waterpang.attack.' + suffix, displayName=name, durationMs=duration,
                      cameraCuts=[], worldInstanceIds=['world.sequence.instance.maharaka.waterpang.attack.' + suffix])
                 for suffix, name, duration in ATTACK_CUTSCENES]
    added = seed_rows(document.setdefault('cutscenes', []), generated, 'cutsceneId')
    if not added:
        print('Attack preview cutscenes already seeded')
        return False
    document['revision'] = int(document['revision']) + 1
    print('Attack preview cutscenes added:', ', '.join(row['cutsceneId'] for row in added))
    return replace_atomic(CAMERA_SHOTS, before, document, 'camerashots.candidate.json', install, 'Camera shots')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--profile', choices=sorted(PROFILES))
    parser.add_argument('--acquire', action='store_true')
    parser.add_argument('--native', action='store_true')
    parser.add_argument('--install-native', action='store_true')
    parser.add_argument('--native-first', type=int, default=NATIVE_FIRST)
    parser.add_argument('--native-last', type=int, default=NATIVE_LAST)
    parser.add_argument('--project', action='store_true')
    parser.add_argument('--sequences', action='store_true')
    parser.add_argument('--install', action='store_true')
    args = parser.parse_args()
    if args.acquire:
        drv.acquire(configure(args.profile))
    if args.native:
        native(args.native_first, args.native_last)
    if args.install_native:
        install_native(args.native_first, args.native_last)
    if args.project:
        project(args.install)
    if args.sequences:
        author_sequences(args.install)
        author_cutscenes(args.install)


if __name__ == '__main__':
    main()
