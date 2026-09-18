#!/usr/bin/env python3
"""Decode audio to a PCM `.wav` using the FMOD the project already ships.

`wwise_vorbis_to_ogg` produces Ogg Vorbis, and turning that into WAV needs a
Vorbis decoder. Rather than pull in another third-party binary, this drives
`Engine/ThirdPartyLib/FMOD/Bin/fmod.dll` through its C API with `ctypes`: the
same decoder the client plays the file with, so the WAV is exactly the samples
the game would hear. It is already a tracked project dependency, so nothing new
has to be installed or committed.

Output is chosen by FMOD, which decodes Vorbis to 16-bit PCM -- the format the
other 4,145 WAVs under `Client/Bin/Resources/Sound` already use.

The system is opened on the `NOSOUND` output, so this never takes the audio
device and is safe to run while the game is up.

  python fmod_decode.py <in.ogg> [-o out.wav]
  python fmod_decode.py <dir> --out-dir <dir>      # every .ogg below it
"""
from __future__ import annotations

import argparse
import ctypes
import struct
import sys
from ctypes import byref, c_float, c_int, c_uint, c_void_p
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
DLL_CANDIDATES = (
    REPO_ROOT / "Engine/ThirdPartyLib/FMOD/Bin/fmod.dll",
    REPO_ROOT / "Client/Bin/Debug/fmod.dll",
    REPO_ROOT / "Client/Bin/Release/fmod.dll",
)

FMOD_VERSION = 0x00020312               # matches Engine/ThirdPartyLib/FMOD/Inc/fmod_common.h
FMOD_OUTPUTTYPE_NOSOUND = 2
FMOD_DEFAULT = 0
FMOD_INIT_NORMAL = 0
FMOD_TIMEUNIT_PCMBYTES = 0x00000004
FMOD_SOUND_FORMAT_PCM16 = 2


class FmodError(Exception):
    pass


def find_dll() -> Path:
    for candidate in DLL_CANDIDATES:
        if candidate.is_file():
            return candidate
    raise FmodError("fmod.dll not found; looked in %s"
                    % ", ".join(str(c) for c in DLL_CANDIDATES))


class FmodDecoder:
    """One FMOD system reused across a batch of files."""

    def __init__(self, dll_path: Path | None = None):
        self._dll = ctypes.CDLL(str(dll_path or find_dll()))
        self._system = c_void_p()
        self._check(self._dll.FMOD_System_Create(byref(self._system), c_uint(FMOD_VERSION)),
                    "System::create")
        self._check(self._dll.FMOD_System_SetOutput(self._system, c_int(FMOD_OUTPUTTYPE_NOSOUND)),
                    "System::setOutput")
        self._check(self._dll.FMOD_System_Init(self._system, c_int(8), c_uint(FMOD_INIT_NORMAL),
                                               None), "System::init")

    @staticmethod
    def _check(result: int, what: str) -> None:
        if 0 != result:
            raise FmodError("%s failed, FMOD_RESULT %d" % (what, result))

    def close(self) -> None:
        if self._system:
            self._dll.FMOD_System_Release(self._system)
            self._system = c_void_p()

    def __enter__(self):
        return self

    def __exit__(self, *_):
        self.close()

    def to_wav(self, source: Path, target: Path) -> int:
        """Decode `source` and write a PCM wav; returns the frame count."""
        sound = c_void_p()
        self._check(self._dll.FMOD_System_CreateSound(
            self._system, str(source).encode("utf-8"), c_uint(FMOD_DEFAULT), None, byref(sound)),
            "System::createSound(%s)" % source.name)
        try:
            pcm_bytes = c_uint()
            self._check(self._dll.FMOD_Sound_GetLength(
                sound, byref(pcm_bytes), c_uint(FMOD_TIMEUNIT_PCMBYTES)), "Sound::getLength")
            sound_type, sound_format = c_int(), c_int()
            channels, bits = c_int(), c_int()
            self._check(self._dll.FMOD_Sound_GetFormat(
                sound, byref(sound_type), byref(sound_format), byref(channels), byref(bits)),
                "Sound::getFormat")
            if FMOD_SOUND_FORMAT_PCM16 != sound_format.value:
                raise FmodError("%s decodes to FMOD sound format %d, expected PCM16"
                                % (source.name, sound_format.value))
            rate = c_float()
            self._check(self._dll.FMOD_Sound_GetDefaults(sound, byref(rate), None),
                        "Sound::getDefaults")

            first, second = c_void_p(), c_void_p()
            first_len, second_len = c_uint(), c_uint()
            self._check(self._dll.FMOD_Sound_Lock(
                sound, c_uint(0), pcm_bytes, byref(first), byref(second),
                byref(first_len), byref(second_len)), "Sound::lock")
            try:
                pcm = ctypes.string_at(first, first_len.value)
                if second and second_len.value:
                    pcm += ctypes.string_at(second, second_len.value)
            finally:
                self._dll.FMOD_Sound_Unlock(sound, first, second, first_len, second_len)
        finally:
            self._dll.FMOD_Sound_Release(sound)

        sample_rate = int(round(rate.value))
        block_align = channels.value * bits.value // 8
        header = (b"RIFF" + struct.pack("<I", 36 + len(pcm)) + b"WAVEfmt "
                  + struct.pack("<IHHIIHH", 16, 1, channels.value, sample_rate,
                                sample_rate * block_align, block_align, bits.value)
                  + b"data" + struct.pack("<I", len(pcm)))
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(header + pcm)
        return len(pcm) // block_align if block_align else 0


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("input", type=Path, help="audio file FMOD can open, or a directory to walk")
    ap.add_argument("-o", "--out", type=Path, help="output .wav for a single input")
    ap.add_argument("--out-dir", type=Path, help="output directory for a directory input")
    ap.add_argument("--pattern", default="*.ogg", help="glob used when input is a directory")
    a = ap.parse_args()

    try:
        decoder = FmodDecoder()
    except FmodError as error:
        print(error, file=sys.stderr)
        return 2

    with decoder:
        if a.input.is_dir():
            out_dir = a.out_dir or a.input
            sources = sorted(a.input.rglob(a.pattern))
            failures = 0
            for source in sources:
                target = (out_dir / source.relative_to(a.input)).with_suffix(".wav")
                try:
                    frames = decoder.to_wav(source, target)
                except FmodError as error:
                    print("%s: %s" % (source.name, error), file=sys.stderr)
                    failures += 1
                    continue
                print("%s -> %s (%d frames)" % (source.name, target.name, frames))
            print("%d/%d decoded" % (len(sources) - failures, len(sources)))
            return 1 if failures else 0

        target = a.out or a.input.with_suffix(".wav")
        try:
            frames = decoder.to_wav(a.input, target)
        except FmodError as error:
            print("%s: %s" % (a.input.name, error), file=sys.stderr)
            return 1
        print("%s -> %s (%d frames)" % (a.input.name, target, frames))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
