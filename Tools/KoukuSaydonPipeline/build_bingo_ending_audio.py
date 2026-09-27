"""Prepare P9/P75 final-ending audio on the common source Slomo clock.

This writes candidate WAV/field-patch/receipt files only below repository out/.
It never launches Client, plays audio, publishes, or changes Data/Resources.
Source SCENE01B, the original extracted bank and its wwiser XML, plus an existing
sound proof receipt are mandatory. The proof must contain sourceScene.sha256,
sourceBank.sha256, bgm.sourceSha256, bgm.oldRenderedSha256 and voice.sha256.
These hashes prevent a previously faded or retimed file becoming the raw input.

Example (bank/XML are previously extracted from SOUND_BGM_COMMANDERRAID):
  python Tools/KoukuSaydonPipeline/build_bingo_ending_audio.py \
    --source-scene out/KoukuOriginalWorldRebuild20260926/SCENE01B.json \
    --source-bank out/KoukuEndingAudioClock20260926/792876931.bnk \
    --bank-xml out/KoukuEndingAudioClock20260926/792876931.bnk.xml \
    --proof-receipt out/KoukuEndingAudioClock20260926/audio-source-clock-receipt.json \
    --output out/KoukuEndingAudioClockRebuild20260926

The BGM retains one-pass media, sample rate and existing wwiser TXTP P fade
approximation. Only the Stop event moves to wall time; this is not a Wwise
bit-exact render. The original voice WAV is reused unchanged.
"""
from __future__ import annotations

from pathlib import Path
import argparse
import struct
import hashlib
import json
import math
import sys
import xml.etree.ElementTree as ET

import numpy as np
from scipy.io import wavfile

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'Tools/KoukuSaydonPipeline'))
from source_scene_clock import SourceSceneClock


def read(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def write(out, name, value):
    (out / name).write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')


def bank_object(root, identity):
    return next(n for n in root.iter('object')
                if any(f.get('name') == 'ulID' and f.get('value') == str(identity)
                       for f in n.findall('field')))


def prop(node, name):
    return next(int(n.find("field[@name='pValue']").get('value'))
                for n in node.iter('object')
                if (f := n.find("field[@name='pID']")) is not None
                and f.get('valuefmt', '').endswith('[' + name + ']'))


def label(path):
    path = Path(path).resolve()
    return path.relative_to(ROOT).as_posix() if path.is_relative_to(ROOT) else str(path)


def build_candidate(source_scene: Path, source_bank: Path, bank_xml: Path,
                    proof_receipt: Path, raw_bgm: Path, baseline_bgm: Path,
                    voice_wav: Path, output_dir: Path, action_source: Path,
                    sequence_source: Path, sound_catalog: Path) -> dict:
    inputs = [Path(p).resolve() for p in (source_scene, source_bank, bank_xml,
              proof_receipt, raw_bgm, baseline_bgm, voice_wav, action_source,
              sequence_source, sound_catalog)]
    (source, source_bank, bank_xml, proof_receipt, raw_path, old_path, voice_path,
     action_source, sequence_source, catalog_path) = inputs
    OUT = Path(output_dir).resolve()
    if not OUT.is_relative_to((ROOT / 'out').resolve()):
        raise ValueError('Output must stay under repository out; this tool cannot install')
    output_names = ['audio-field-patch.json', 'audio-source-clock-receipt.json',
        'candidate-resources/Sound/KoukuSaton/Events/bgm_midnightc_ed_m20_scene_finish.source-clock-stop.wav']
    if any(OUT / name in inputs for name in output_names):
        raise ValueError('Candidate output cannot overwrite a source or proof input')
    proof = read(proof_receipt)
    expected = [(source, proof['sourceScene']['sha256']),
        (source_bank, proof['sourceBank']['sha256']),
        (raw_path, proof['bgm']['sourceSha256']),
        (old_path, proof['bgm']['oldRenderedSha256']),
        (voice_path, proof['voice']['sha256'])]
    for path, expected_hash in expected:
        if sha(path) != expected_hash:
            raise ValueError(f'Source hash differs from the sound proof receipt: {path}')
    OUT.mkdir(parents=True, exist_ok=True)
    scene = read(source)
    rows = {int(k): v for k, v in scene['rows'].items()}
    clock = SourceSceneClock.from_scene_rows(rows, 45)
    bank = ET.parse(bank_xml).getroot()
    play = bank_object(bank, 783980885)
    stop = bank_object(bank, 299433481)
    container = bank_object(bank, 393268365)
    delay_ms = prop(play, 'DelayTime')
    fade_ms = prop(stop, 'TransitionTime')
    curve = int(next(f.get('value') for f in stop.iter('field') if f.get('name') == 'eFadeCurve'))
    loops = [int(f.get('value')) for f in container.iter('field') if f.get('name') == 'Loop']
    assert (delay_ms, fade_ms, curve, loops) == (10, 2000, 7, [1, 1])
    # Verify the relevant XML values against their actual original-bank bytes.
    # These offsets are provided by wwiser, not inferred from a hierarchy layout.
    bank_bytes = source_bank.read_bytes()
    for node, wanted in [(play, {'pValue', 'idExt'}), (stop, {'pValue', 'idExt'}),
                         (container, {'Loop'})]:
        for field in node.iter('field'):
            if field.get('name') not in wanted:
                continue
            offset = int(field.get('offset'))
            format_ = '<h' if field.get('name') == 'Loop' else '<I'
            if struct.unpack_from(format_, bank_bytes, offset)[0] != int(field.get('value')):
                raise ValueError('Bank XML value differs from its proof-matched binary')
    fade_bits = stop.find(".//object[@name='ActiveActionParams']/field[@name='byBitVector']")
    if bank_bytes[int(fade_bits.get('offset'))] & 0x1f != curve:
        raise ValueError('Bank XML fade curve differs from original binary')

    # Source 70's Stop key is a Matinee scene-clock event. The Play action delay
    # and the audio frames themselves are wall time, applied exactly once.
    stop_source_ms = next(e['time'] * 1000.0 for e in rows[70]['p']['akevents'] if e['event'] == -2)
    voice_source_ms = next(e['time'] * 1000.0 for e in rows[69]['p']['akevents'] if e['event'] == -3)
    assert next(e['time'] for e in rows[70]['p']['akevents'] if e['event'] == -1) == 0
    stop_wall_ms = clock.to_elapsed_ms(stop_source_ms)
    fade_local_ms = stop_wall_ms - delay_ms
    sr, pcm = wavfile.read(raw_path)
    assert sr == 48000 and pcm.dtype == np.int16 and pcm.ndim == 2 and pcm.shape[1] == 2
    old_sr, old = wavfile.read(old_path)
    voice_sr, voice = wavfile.read(voice_path)
    assert old_sr == sr
    # Keep the existing SoundPipeline int16->float convention, media order, rate,
    # and channels. The existing wwiser SineRecip fade-out uses TXTP 'P', an
    # inverted parabola approximation, not Wwise bit-exact output.
    raw = pcm.astype(np.float32) / 32767.0
    time_ms = np.arange(len(raw), dtype=np.float64) * 1000.0 / sr
    u = np.clip((time_ms - fade_local_ms) / fade_ms, 0.0, 1.0)
    gain = 1.0 - u * u
    candidate = (raw * gain[:, None]).astype(np.float32)
    asset = 'Sound/KoukuSaton/Events/bgm_midnightc_ed_m20_scene_finish.source-clock-stop.wav'
    output = OUT / 'candidate-resources' / asset
    output.parent.mkdir(parents=True, exist_ok=True)
    wavfile.write(output, sr, candidate)
    native_duration_ms = len(candidate) * 1000.0 / sr
    duration_ms = math.ceil(native_duration_ms)
    voice_duration_ms = math.ceil(len(voice) * 1000.0 / voice_sr)
    voice_start_ms = round(clock.to_elapsed_ms(voice_source_ms))
    required_end_ms = max(delay_ms + duration_ms, voice_start_ms + voice_duration_ms)

    # A baseline reconstruction demonstrates that this changes the fade clock,
    # rather than loading an unrelated raw media or introducing a pitch/rate edit.
    old_u = np.clip((time_ms[:len(old)] - (stop_source_ms - delay_ms)) / fade_ms, 0, 1)
    old_reconstructed = (raw[:len(old)] * (1.0 - old_u * old_u)[:, None]).astype(np.float32)
    old_error = float(np.max(np.abs(old_reconstructed - old)))
    assert old_error < 0.000005
    prefix = time_ms < fade_local_ms
    assert np.array_equal(candidate[prefix], raw[prefix])
    assert np.isfinite(candidate).all() and len(candidate) == len(raw)
    assert np.all(gain[1:] <= gain[:-1])
    loaded_sr, loaded = wavfile.read(output)
    assert loaded_sr == sr and np.array_equal(loaded, candidate)

    bgm_id = 'sound.kouku.bd6539c4598c9be3626e'
    voice_id = 'sound.kouku.107e53f0c73d5a11c2c8'
    event = 's_bgm_commanderraid.bgm_midnightc_ed_m20_scene_finish'
    documents = []
    for source_path, ordinal in [(action_source, 75), (sequence_source, 9)]:
        path = label(source_path)
        document = read(source_path)
        pattern_id = f'KAKULSAYDON_G1_PATTERN_{ordinal}'
        pattern = next(p for p in document['patterns'] if p['patternId'] == pattern_id)
        occurrences = []
        for resource_id, values in [(bgm_id, dict(startMs=delay_ms, durationMs=duration_ms, soundSourceStartMs=0)),
                                    (voice_id, dict(startMs=voice_start_ms, durationMs=voice_duration_ms, soundSourceStartMs=0))]:
            matches = [o for o in pattern['presentationOccurrences'] if o['resourceId'] == resource_id]
            assert len(matches) == 1
            occurrence = matches[0]
            occurrences.append(dict(occurrenceId=occurrence['occurrenceId'], resourceId=resource_id,
                                    before={k: occurrence[k] for k in values}, fields=values))
        resource = next(r for r in document['presentationResources'] if r['resourceId'] == bgm_id)
        documents.append(dict(path=path, baseSha256=sha(source_path), patternId=pattern_id,
            presentationOccurrenceChanges=occurrences,
            presentationResourceChanges=[dict(resourceId=bgm_id,
                before={k: resource[k] for k in ['assetId', 'durationMs']},
                fields=dict(assetId=asset, durationMs=duration_ms))],
            minimumPatternDurationMs=required_end_ms,
            durationPolicy='Take max(latest pattern duration, minimumPatternDurationMs); preserve final visual hold. Do not retime the audio frames.'))
    catalog = read(catalog_path)
    catalog_patch = dict(path=label(catalog_path), baseSha256=sha(catalog_path),
        characterClass='KoukuSaydon', soundEventId=event,
        before=catalog['classes']['KoukuSaydon']['s_bgm_commanderraid.bgm_midnightc_ed_m20_scene_finish'],
        variants=[asset])
    write(OUT, 'audio-field-patch.json', dict(schema='kouku.ending.audio.source-clock.field-patch.v1',
        scope='P9/P75 ending SOUND only; candidate, not installed',
        documents=documents, catalogChange=catalog_patch,
        requiredMinimumPatternDurationMs=required_end_ms,
        sourceSceneTerminalMs=round(clock.elapsed_duration_ms),
        mergePolicy='Re-read latest documents, compare targeted before fields, preserve all unrelated fields. Catalog variant is the real consumer when soundEvent is nonempty.'))

    write(OUT, 'audio-source-clock-receipt.json', dict(
        status='offline candidate generated and numerically checked; not installed or auditioned',
        generationInputs=dict(generator=label(Path(__file__)), proofReceipt=label(proof_receipt), proofReceiptSha256=sha(proof_receipt), bankXml=label(bank_xml), bankXmlSha256=sha(bank_xml), sourceClock=label(ROOT/'Tools/KoukuSaydonPipeline/source_scene_clock.py'), sourceClockSha256=sha(ROOT/'Tools/KoukuSaydonPipeline/source_scene_clock.py')),
        sourceScene=dict(path=label(source), sha256=sha(source), matinee=32, interpData=45, akVoiceTrack=69, akBgmTrack=70),
        sourceBank=dict(path=label(source_bank), sha256=sha(source_bank),
            originalPackage='SOUND_BGM_COMMANDERRAID / HPVIK3I65B3ODPBBZIKRAAZJK.pck', bankId=792876931,
            playEventId=627223628, playActionId=783980885, skipEndEventId=2514073161, stopActionId=299433481,
            targetMusicContainer=393268365, sourceMediaId=702586732, playDelayMs=delay_ms,
            stopTransitionMs=fade_ms, stopCurve='7 SineRecip', finitePlaylistLoopCounts=loops,
            musicSegmentSourceDurationMs=53733.67346938775,
            parser='wwiser v20260808 XML; no client/audio-device launch'),
        clock=dict(sourceStopMs=stop_source_ms, wallStopMs=stop_wall_ms, sampleLocalFadeStartMs=fade_local_ms,
            theoreticalFadeEndWallMs=stop_wall_ms+fade_ms, naturalMediaEndWallMs=delay_ms+native_duration_ms,
            sourceSceneTerminalMs=round(clock.elapsed_duration_ms), requiredPatternEndMs=required_end_ms),
        bgm=dict(sourcePath=label(raw_path), sourceSha256=sha(raw_path), oldRenderedSha256=sha(old_path),
            candidatePath=label(output), runtimeAssetId=asset, sha256=sha(output), sampleRate=sr,
            channels=2, frames=len(candidate), exactDurationMs=native_duration_ms, integerDurationMs=duration_ms,
            playbackRate=1, loop=False, fadeApproximation='wwiser TXTP P inverted parabola (1 - u*u), as the prior installed source-stop render; not Wwise bit-exact',
            naturalEndPolicy='Original finite one-pass media ends before the 2-second stop fade would finish. No invented samples, padding, looping, or time-stretch.'),
        voice=dict(path=label(voice_path), sha256=sha(voice_path), startMs=voice_start_ms,
            frames=len(voice), sampleRate=voice_sr, durationMs=voice_duration_ms, modification='Occurrence start only; existing two-layer WAV bytes preserved'),
        validation=dict(originalSourceStopReconstructionMaxFloatError=old_error,
            candidatePrefixBeforeStopIdentical=True, exactRawFrameCountPreserved=True, finiteSamples=True,
            monotoneGain=True, candidateWavRoundTripExact=True,
            noLiveDataOrResourcesWrites=True),
        consumer=dict(file='Client/Private/KoukuSaydonPresentationPlayer.cpp', behavior='SOUND chooses m_SoundEventVariants for nonempty soundEvent, then wall age plus soundSourceStartMs; no pitch/time-stretch',
            catalog='Data/Sound/CharacterSoundCatalog.json classes.KoukuSaydon event variants; explicit admission reload required',
            integrationRequirement='Install candidate WAV, resource fields AND catalog variant together; retain scene visual hold through minimumPatternDurationMs to avoid truncating BGM.'),
        limitations=['Reproduction requires the provided original-bank XML and sound proof receipt; this generator does not extract banks or substitute missing evidence.',
                     'Preserves existing decoded 702586732 source and existing wwiser approximate curve route; no new claim of Wwise bus, decoder or bit-exact audio parity.',
                     'Numerical checks do not establish listening quality or in-client playback.']))
    result = dict(candidate=str(output), sha256=sha(output), stopWallMs=stop_wall_ms,
                  durationMs=duration_ms, requiredEndMs=required_end_ms, baselineMaxError=old_error)
    return result


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--source-scene', type=Path, required=True)
    parser.add_argument('--source-bank', type=Path, required=True, help='Original extracted bank 792876931.bnk (read-only).')
    parser.add_argument('--bank-xml', type=Path, required=True, help='wwiser XML dump of that same bank (read-only).')
    parser.add_argument('--proof-receipt', type=Path, required=True, help='Existing source/decoded-audio SHA256 proof; required, never inferred.')
    parser.add_argument('--output', type=Path, required=True, help='Candidate directory strictly below repository out/.')
    parser.add_argument('--raw-bgm', type=Path, default=ROOT/'Client/Bin/Resources/Sound/KoukuSaton/S_BGM_COMMANDERRAID/bgm_midnightc_ed_m20_scene_finish__702586732.wav')
    parser.add_argument('--baseline-bgm', type=Path, default=ROOT/'Client/Bin/Resources/Sound/KoukuSaton/Events/bgm_midnightc_ed_m20_scene_finish.source-stop.wav')
    parser.add_argument('--voice', type=Path, default=ROOT/'Client/Bin/Resources/Sound/KoukuSaton/Events/event.2fd2795661e9a5534224.wav')
    parser.add_argument('--action-source', type=Path, default=ROOT/'Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json')
    parser.add_argument('--sequence-source', type=Path, default=ROOT/'Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json')
    parser.add_argument('--sound-catalog', type=Path, default=ROOT/'Data/Sound/CharacterSoundCatalog.json')
    args = parser.parse_args()
    result = build_candidate(args.source_scene, args.source_bank, args.bank_xml,
        args.proof_receipt, args.raw_bgm, args.baseline_bgm, args.voice, args.output,
        args.action_source, args.sequence_source, args.sound_catalog)
    print(json.dumps(result, ensure_ascii=False))


if __name__ == '__main__':
    main()
