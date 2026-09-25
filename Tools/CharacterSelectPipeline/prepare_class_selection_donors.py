"""Stage exact source glTF/PSA class-movie donors; never install resources.

The original PSA can be large. Copy only requested complete sequence records,
including SCALEKEYS, before using the shared glTF/PSA intake and WModel writer.
The recipe records original-to-short clip names and source hashes. All outputs
must remain under the caller's repository out directory.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'Tools/ModelAssetConverter'))
import append_psa_clip_to_wmodel as psa_codec
import verify_dimensionmaster_summon_bind_pose as wm

CHUNK = struct.Struct('<20siii')


def digest(path):
    with Path(path).open('rb') as f:
        return hashlib.file_digest(f, 'sha256').hexdigest()


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + '\n', encoding='utf-8')


def stage_named_source_tracks(source_gltf, source_psa, output_gltf, report_path):
    """Join the original AnimSet tracks by FName; retain mesh-only ref bones.

    This is an explicit movie-recipe option, not a weaker shared cooker intake.
    Geometry, inverse binds and source joint indices are never remapped.
    """
    sys.path.insert(0, str(ROOT / 'Tools/ActorXAssetCooker'))
    import build_umodel_gltf_psa as stage
    document = json.loads(source_gltf.read_text(encoding='utf-8'))
    psa = stage.read_psa(source_psa)
    skin = document['skins'][0]['joints']
    names = [document['nodes'][node]['name'] for node in skin]
    # Run the unchanged strict geometry validation against its own palette.
    buffer_path, _ = stage.validate_gltf(document, source_gltf, {'bones': names})
    tracks = {name: index for index, name in enumerate(psa['bones'])}
    matched = [(name, node, tracks[name]) for name, node in zip(names, skin) if name in tracks]
    if not matched:
        raise ValueError('Source AnimSet and source mesh have no common bone names')
    matched_nodes = {node for _, node, _ in matched}
    for node in skin:
        for child in document['nodes'][node].get('children', []):
            if child in matched_nodes and node not in matched_nodes:
                raise ValueError('An animated bone has an unmatched mesh parent')
    original_count = len(psa['bones'])
    frames = len(psa['animation_keys']) // original_count
    selected = dict(psa)
    selected['bones'] = [name for name, _, _ in matched]
    for field in ('animation_keys', 'scale_keys'):
        if psa[field] is not None:
            selected[field] = [psa[field][frame * original_count + index]
                               for frame in range(frames) for _, _, index in matched]
    payload = bytearray(buffer_path.read_bytes())
    scaled = stage.scale_bind_geometry(document, payload, 100.)
    weight_accessors = stage.materialize_skin_weights(document, payload)
    clips = stage.build_animations(document, payload, selected, [node for _, node, _ in matched], 100.)
    stage.align4(payload)
    output_bin = output_gltf.with_suffix('.bin')
    document['buffers'][0].update(uri=output_bin.name, byteLength=len(payload))
    output_bin.write_bytes(payload)
    write(output_gltf, document)
    write(report_path, dict(schema='source-movie-exact-named-track-join',
        sourceGltf=str(source_gltf), sourceGltfSha256=digest(source_gltf),
        sourcePsa=str(source_psa), sourcePsaSha256=digest(source_psa),
        meshBoneCount=len(names), sourceTrackCount=original_count,
        matchedBoneNames=[name for name, _, _ in matched],
        meshReferencePoseBoneNames=[name for name in names if name not in tracks],
        absentMeshTrackNames=[name for name in psa['bones'] if name not in names],
        policy='Exact FName animation tracks; unmatched mesh bones retain original glTF reference TRS; no synthetic names or replacement tracks',
        copiedKeyCount=frames * len(matched), copiedKeyValueError=0,
        scaled=scaled, materializedWeightAccessors=weight_accessors, clips=clips))


def subset_psa(source, target, clip_map):
    chunks = {name: (size, count, offset) for name, size, count, offset in psa_codec.psa_chunks(source)}
    assert {'BONENAMES', 'ANIMINFO', 'ANIMKEYS'} <= chunks.keys()
    with source.open('rb') as f:
        def blob(name):
            size, count, offset = chunks[name]
            f.seek(offset)
            result = f.read(size * count)
            assert len(result) == size * count
            return result
        info = blob('ANIMINFO')
        assert chunks['ANIMINFO'][0] == 168
        records = {}
        for offset in range(0, len(info), 168):
            row = info[offset:offset+168]
            name = row[:64].split(b'\0')[0].decode('ascii')
            assert name not in records
            records[name] = row
        assert set(clip_map) <= records.keys(), sorted(set(clip_map) - records.keys())
        assert len(set(clip_map.values())) == len(clip_map)
        payloads = {'BONENAMES': blob('BONENAMES'), 'ANIMINFO': bytearray(), 'ANIMKEYS': bytearray()}
        if 'SCALEKEYS' in chunks:
            payloads['SCALEKEYS'] = bytearray()
        first = 0
        receipts = []
        for original, short in clip_map.items():
            assert short.isascii() and 0 < len(short.encode()) < 32
            row = bytearray(records[original])
            bones = struct.unpack_from('<i', row, 128)[0]
            old_first, frames = struct.unpack_from('<ii', row, 160)
            assert bones == chunks['BONENAMES'][1] and frames > 0
            row[:64] = short.encode().ljust(64, b'\0')
            struct.pack_into('<i', row, 160, first)
            payloads['ANIMINFO'].extend(row)
            proof = {}
            for name in ('ANIMKEYS', 'SCALEKEYS'):
                if name not in payloads:
                    continue
                size, count, offset = chunks[name]
                begin, span = old_first * bones, frames * bones
                assert begin + span <= count
                f.seek(offset + begin * size)
                data = f.read(span * size)
                assert len(data) == span * size
                payloads[name].extend(data)
                proof[name] = dict(bytes=len(data), sha256=hashlib.sha256(data).hexdigest())
            receipts.append(dict(sourceClip=original, stagedClip=short, firstSourceFrame=old_first,
                frameCount=frames, boneCount=bones, rate=struct.unpack_from('<f', row, 152)[0], keyPayloads=proof))
            first += frames
    target.parent.mkdir(parents=True, exist_ok=True)
    with target.open('wb') as f:
        f.write(CHUNK.pack(b'ANIMHEAD', 0, 0, 0))
        for name, data in payloads.items():
            size = chunks[name][0]
            assert size > 0 and len(data) % size == 0
            f.write(CHUNK.pack(name.encode(), 0, size, len(data)//size))
            f.write(data)
    return dict(source=str(source), sourceSha256=digest(source), output=str(target), outputSha256=digest(target), sequences=receipts)


def run(command, log):
    result = subprocess.run([str(x) for x in command], cwd=ROOT, stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT, text=True, encoding='utf-8', errors='replace',
        creationflags=subprocess.CREATE_NO_WINDOW if sys.platform=='win32' else 0)
    log.parent.mkdir(parents=True, exist_ok=True)
    log.write_text(result.stdout, encoding='utf-8')
    if result.returncode:
        raise RuntimeError(f'Command failed ({result.returncode}): {command}\n{result.stdout[-3000:]}')


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--recipe', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args=parser.parse_args()
    output=args.output.resolve()
    if not output.is_relative_to((ROOT/'out').resolve()):
        raise ValueError('Candidate output must remain under repository out')
    recipe=json.loads(args.recipe.read_text(encoding='utf-8-sig'))
    output.mkdir(parents=True,exist_ok=True)
    subset=output/'source-clips.psa'
    subset_receipt=subset_psa(Path(recipe['sourcePsa']).resolve(), subset, recipe['clipMap'])
    write(output/'source-clips.receipt.json',subset_receipt)
    actors=[]
    for actor in recipe['actors']:
        label=actor['actorId']
        if not label.replace('_','').isalnum():
            raise ValueError('Actor label must be a simple ASCII identifier')
        folder=(output/label).resolve()
        assert folder.is_relative_to(output)
        folder.mkdir(parents=True,exist_ok=True)
        gltf=folder/(label+'.gltf')
        command=[sys.executable, ROOT/'Tools/ActorXAssetCooker/build_umodel_gltf_psa.py',
            '--gltf',Path(actor['sourceGltf']).resolve(),'--psa',subset,'--output-gltf',gltf,
            '--output-bin',folder/(label+'.bin'),'--report',folder/'gltf.receipt.json',
            '--scale','100','--allow-bone-order-remap']
        if actor.get('sourceAnimSetNamedTrackJoin', False):
            stage_named_source_tracks(Path(actor['sourceGltf']).resolve(), subset, gltf, folder/'gltf.receipt.json')
        else:
            run(command,folder/'gltf.log')
        model=folder/(label+'.wmodel')
        # Native movie MIC descriptors are a separate mandatory stage. This
        # donor is offline geometry/animation input, never an installed asset.
        run([ROOT/'Tools/ModelAssetConverter/Bin/ModelAssetConverter.exe',gltf,'-o',model,
             '--no-auto-textures'],folder/'converter.log')
        check=wm.read_wmodel(model,include_geometry=False,animation_names=())
        assert len(check.animations)==len(recipe['clipMap'])
        mapped={source:next(a.name for a in check.animations if a.name==short or a.name.endswith('_'+short) or a.name.endswith('|'+short))
            for source,short in recipe['clipMap'].items()}
        for animation in check.animations:
            assert animation.ticks_per_second>0
        actors.append(dict(actorId=label,sourceGltf=actor['sourceGltf'],sourceGltfSha256=digest(Path(actor['sourceGltf'])),
            sourceActorExportIndices0=actor['sourceActorExportIndices0'],donorModelPath=str(model),geometryModelPath=str(model),
            donorSha256=digest(model),skeletonBones=len(check.skeleton_bones),clipMap=mapped,
            animations=[dict(name=a.name,durationTicks=a.duration_ticks,ticksPerSecond=a.ticks_per_second) for a in check.animations],
            status='OFFLINE_DONOR_ONLY_NATIVE_MATERIAL_PENDING'))
        print(label,len(check.skeleton_bones),mapped,flush=True)
    write(output/'donor-receipt.json',dict(classId=recipe['classId'],sourceClipReceipt=str(output/'source-clips.receipt.json'),actors=actors))


if __name__=='__main__':
    main()
