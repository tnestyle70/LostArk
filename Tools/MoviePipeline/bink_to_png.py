# -*- coding: utf-8 -*-
"""Decode a decrypted Bink 1/2 movie (.bik/.bk2) to an RGBA PNG sequence with the
game's own bink2w64.dll (FFmpeg has no Bink 2 decoder; 'KB2j' needs this path).

  python bink_to_png.py <movie.bk2> <out_dir> [--every N] [--frames a-b]

Frames are written as <out_dir>/<stem>_<frame:04d>.png, straight alpha.
"""
from __future__ import annotations

import ctypes
import os
import struct
import sys

from PIL import Image

BINK_DLL = r"D:\Games\LOSTARK\Binaries\Win64\bink2w64.dll"
BINKALPHA = 0x00100000
BINKSURFACE32A = 5          # BGRA, alpha channel copied
BINKCOPYALL = 0x80000000


class BinkDecoder:
    def __init__(self, path: str):
        self.dll = ctypes.WinDLL(BINK_DLL)
        d = self.dll
        d.BinkOpen.restype = ctypes.c_void_p
        d.BinkOpen.argtypes = [ctypes.c_char_p, ctypes.c_uint32]
        d.BinkDoFrame.restype = ctypes.c_int32
        d.BinkDoFrame.argtypes = [ctypes.c_void_p]
        d.BinkNextFrame.restype = None
        d.BinkNextFrame.argtypes = [ctypes.c_void_p]
        d.BinkCopyToBuffer.restype = ctypes.c_int32
        d.BinkCopyToBuffer.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_int32,
                                       ctypes.c_uint32, ctypes.c_uint32, ctypes.c_uint32, ctypes.c_uint32]
        d.BinkClose.restype = None
        d.BinkClose.argtypes = [ctypes.c_void_p]
        d.BinkGetError.restype = ctypes.c_char_p
        d.BinkGetError.argtypes = []
        self.h = d.BinkOpen(path.encode("mbcs"), BINKALPHA)
        if not self.h:
            raise RuntimeError(f"BinkOpen failed: {d.BinkGetError()!r}")
        # BINK struct begins: U32 Width, Height, Frames, FrameNum, LastFrameNum
        hdr = ctypes.string_at(self.h, 20)
        self.width, self.height, self.frames = struct.unpack_from("<III", hdr, 0)
        self.pitch = self.width * 4
        self.buf = ctypes.create_string_buffer(self.pitch * self.height)

    def frame_rgba(self) -> Image.Image:
        d = self.dll
        d.BinkDoFrame(self.h)
        d.BinkCopyToBuffer(self.h, self.buf, self.pitch, self.height, 0, 0, BINKSURFACE32A | BINKCOPYALL)
        img = Image.frombuffer("RGBA", (self.width, self.height), self.buf.raw, "raw", "BGRA", 0, 1)
        d.BinkNextFrame(self.h)
        return img

    def close(self):
        if self.h:
            self.dll.BinkClose(self.h)
            self.h = None


def main(argv: list[str]) -> int:
    if len(argv) < 2:
        print(__doc__)
        return 2
    src, out_dir = argv[0], argv[1]
    every = 1
    lo, hi = 0, None
    if "--every" in argv:
        every = int(argv[argv.index("--every") + 1])
    if "--frames" in argv:
        a, b = argv[argv.index("--frames") + 1].split("-")
        lo, hi = int(a), int(b)
    os.makedirs(out_dir, exist_ok=True)
    stem = os.path.splitext(os.path.basename(src))[0]
    dec = BinkDecoder(src)
    print(f"{stem}: {dec.width}x{dec.height} frames={dec.frames}")
    last = dec.frames - 1 if hi is None else min(hi, dec.frames - 1)
    written = 0
    for i in range(last + 1):
        img = dec.frame_rgba()
        if i < lo or (i - lo) % every:
            continue
        img.save(os.path.join(out_dir, f"{stem}_{i:04d}.png"))
        written += 1
    dec.close()
    print(f"wrote {written} png -> {out_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
