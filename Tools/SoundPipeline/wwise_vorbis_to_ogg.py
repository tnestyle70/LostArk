#!/usr/bin/env python3
"""Convert a Wwise RIFF/RIFX Vorbis .wem into a standard Ogg Vorbis file.

The game's streamed audio is Wwise Vorbis: a RIFF whose fmt codec id is 0xFFFF,
whose packets carry a 2- or 6-byte Wwise header instead of Ogg framing, and
whose setup packet has been stripped -- the codebooks are not in the file at
all, only 10-bit indices into the fixed aoTuV 6.03 codebook library that the
Wwise runtime has compiled in. Nothing can read that without the library, which
is why this pipeline used to shell out to vgmstream or ww2ogg.

This is a port of ww2ogg (Adam Gashlin / Xiph.org, BSD 3-clause; see
`Tools/SoundPipeline/WW2OGG_COPYING`), so the repo can go from .pck to playable
audio with nothing but Python. The library itself is ww2ogg's
`packed_codebooks_aoTuV_603.bin`, kept here as
`wwise_vorbis_codebooks_aoTuV_603.dat` (the `*.dat` Git LFS rule).

Ogg Vorbis is what comes out: FMOD plays it directly, so `CSound_Manager` needs
no extra step, and every desktop player opens it. `--wav` decodes each result to
16-bit PCM as well, through the project's own FMOD (see `fmod_decode.py`), which
is the format the rest of `Client/Bin/Resources/Sound` uses.

  python wwise_vorbis_to_ogg.py <in.wem> [-o out.ogg] [--wav]
  python wwise_vorbis_to_ogg.py <dir> --out-dir <dir>     # every .wem below it

Covered: little- and big-endian RIFF, fmt 0x42 with the vorb fields inline,
separate vorb chunks of 0x2A/0x32/0x34 (2-byte headers, no granule) and
0x28/0x2C (the old 8-byte headers are rejected, see below), stripped setup with
external codebooks, modified packets, smpl loop points. Not covered: the
pre-2011 header-triad layout (vorb 0x28/0x2C), which none of the game's banks
use; it raises rather than guessing.
"""
from __future__ import annotations

import argparse
import struct
import sys
from pathlib import Path

CODEBOOK_LIBRARY = Path(__file__).resolve().parent / "wwise_vorbis_codebooks_aoTuV_603.dat"
VENDOR = b"converted from Audiokinetic Wwise by LostArk SoundPipeline"


class WemError(Exception):
    pass


def ilog(v: int) -> int:
    """Tremor's ilog: how many bits the value occupies (ilog(0) == 0)."""
    ret = 0
    while v:
        ret += 1
        v >>= 1
    return ret


def book_maptype1_quantvals(entries: int, dimensions: int) -> int:
    """Tremor's _book_maptype1_quantvals, needed to size a lookup-1 table."""
    bits = ilog(entries)
    vals = entries >> ((bits - 1) * (dimensions - 1) // dimensions)
    while True:
        acc = 1
        acc1 = 1
        for _ in range(dimensions):
            acc *= vals
            acc1 *= vals + 1
        if acc <= entries and acc1 > entries:
            return vals
        if acc > entries:
            vals -= 1
        else:
            vals += 1


def _crc_table():
    table = []
    for i in range(256):
        r = i << 24
        for _ in range(8):
            r = ((r << 1) ^ 0x04C11DB7) & 0xFFFFFFFF if (r & 0x80000000) else (r << 1) & 0xFFFFFFFF
        table.append(r)
    return table


CRC_LOOKUP = _crc_table()


def ogg_checksum(data: bytes) -> int:
    """Ogg's CRC32: polynomial 0x04c11db7, not reflected, no initial or final xor."""
    crc = 0
    for byte in data:
        crc = ((crc << 8) & 0xFFFFFFFF) ^ CRC_LOOKUP[((crc >> 24) & 0xFF) ^ byte]
    return crc


class BitReader:
    """Reads single bits least-significant-first out of a byte string."""

    def __init__(self, data: bytes, start: int = 0):
        self._data = data
        self._pos = start
        self._buffer = 0
        self._bits_left = 0
        self.total_bits_read = 0

    def bit(self) -> int:
        if 0 == self._bits_left:
            if self._pos >= len(self._data):
                raise WemError("ran out of bits")
            self._buffer = self._data[self._pos]
            self._pos += 1
            self._bits_left = 8
        self.total_bits_read += 1
        self._bits_left -= 1
        return 1 if (self._buffer & (0x80 >> self._bits_left)) else 0

    def uint(self, size: int) -> int:
        value = 0
        for i in range(size):
            if self.bit():
                value |= 1 << i
        return value


class OggWriter:
    """Packs bits least-significant-first into Vorbis packets and pages them.

    Packets are laced into pages the way the Ogg mapping asks for -- up to 255
    lacing segments per page, a page's granule position being the end of the
    last packet that finishes on it -- rather than ww2ogg's one page per packet,
    which is why ww2ogg tells you to run its output through revorb. Doing it
    here keeps the whole conversion in one step.
    """

    HEADER_BYTES = 27
    MAX_SEGMENTS = 255
    SEGMENT_SIZE = 255

    def __init__(self, serial: int = 1):
        self._out = bytearray()
        self._bit_buffer = 0
        self._bits_stored = 0
        self._packet = bytearray()
        self._serial = serial
        self._seqno = 0
        self._first = True
        # current page under construction
        self._lacing = []
        self._payload = bytearray()
        self._continued = False      # this page starts mid-packet
        self._granule = -1           # -1 == no packet ends on this page
        self._last_granule = 0

    def bytes(self) -> bytes:
        return bytes(self._out)

    def put_bit(self, bit: int) -> None:
        if bit:
            self._bit_buffer |= 1 << self._bits_stored
        self._bits_stored += 1
        if 8 == self._bits_stored:
            self.flush_bits()

    def uint(self, size: int, value: int) -> None:
        if size and (value >> (size - 1)) > 1:
            raise WemError("value %d does not fit in %d bits" % (value, size))
        for i in range(size):
            self.put_bit((value >> i) & 1)

    def copy_uint(self, reader: BitReader, size: int) -> int:
        value = reader.uint(size)
        self.uint(size, value)
        return value

    def flush_bits(self) -> None:
        if self._bits_stored:
            self._packet.append(self._bit_buffer)
            self._bits_stored = 0
            self._bit_buffer = 0

    def end_packet(self, granule: int, flush: bool = False, last: bool = False) -> None:
        """Close the packet being written and lace it into the current page."""
        self.flush_bits()
        packet = bytes(self._packet)
        self._packet = bytearray()
        self._last_granule = granule

        lacing = [self.SEGMENT_SIZE] * (len(packet) // self.SEGMENT_SIZE)
        lacing.append(len(packet) % self.SEGMENT_SIZE)   # always < 255, may be 0
        written = 0
        for index, value in enumerate(lacing):
            if len(self._lacing) == self.MAX_SEGMENTS:
                self._emit_page(False)
                # The new page only continues a packet when this one was already
                # part way out; a page that starts on a packet boundary must not
                # set the flag, or a decoder throws the packet away.
                self._continued = index > 0
            self._lacing.append(value)
            self._payload += packet[written:written + value]
            written += value
            if index == len(lacing) - 1:
                self._granule = granule
        if flush or last:
            self._emit_page(last)

    def _emit_page(self, last: bool) -> None:
        if not self._lacing:
            return
        segments = len(self._lacing)
        header = bytearray(self.HEADER_BYTES + segments)
        header[0:4] = b"OggS"
        header[4] = 0
        header[5] = (1 if self._continued else 0) | (2 if self._first else 0) | (4 if last else 0)
        granule = self._granule if self._granule >= 0 else 0xFFFFFFFFFFFFFFFF
        struct.pack_into("<Q", header, 6, granule)
        struct.pack_into("<I", header, 14, self._serial)
        struct.pack_into("<I", header, 18, self._seqno)
        struct.pack_into("<I", header, 22, 0)
        header[26] = segments
        header[27:27 + segments] = bytes(self._lacing)

        page = bytes(header) + bytes(self._payload)
        struct.pack_into("<I", header, 22, ogg_checksum(page))
        self._out += bytes(header) + bytes(self._payload)

        self._seqno += 1
        self._first = False
        self._lacing = []
        self._payload = bytearray()
        self._granule = -1
        self._continued = False

    def finish(self) -> None:
        if self._lacing:
            self._emit_page(True)


class CodebookLibrary:
    """ww2ogg's packed codebook library: bodies, then a table of offsets."""

    def __init__(self, path: Path):
        data = path.read_bytes()
        offset_offset = struct.unpack_from("<I", data, len(data) - 4)[0]
        count = (len(data) - offset_offset) // 4
        self._data = data[:offset_offset]
        self._offsets = [struct.unpack_from("<I", data, offset_offset + 4 * i)[0]
                         for i in range(count)]

    def codebook(self, index: int):
        if index < 0 or index >= len(self._offsets) - 1:
            raise WemError("invalid codebook id %d" % index)
        return self._data[self._offsets[index]:self._offsets[index + 1]]

    def rebuild(self, index: int, out: OggWriter) -> None:
        """Expand one packed codebook into a standard Vorbis setup codebook."""
        body = self.codebook(index)
        reader = BitReader(body)

        dimensions = reader.uint(4)
        entries = reader.uint(14)

        out.uint(24, 0x564342)  # "BCV"
        out.uint(16, dimensions)
        out.uint(24, entries)

        ordered = out.copy_uint(reader, 1)
        if ordered:
            out.copy_uint(reader, 5)  # initial length
            current_entry = 0
            while current_entry < entries:
                number = out.copy_uint(reader, ilog(entries - current_entry))
                current_entry += number
            if current_entry > entries:
                raise WemError("codebook current_entry out of range")
        else:
            codeword_length_length = reader.uint(3)
            sparse = reader.uint(1)
            if 0 == codeword_length_length or codeword_length_length > 5:
                raise WemError("nonsense codeword length")
            out.uint(1, sparse)
            for _ in range(entries):
                present = True
                if sparse:
                    present = bool(out.copy_uint(reader, 1))
                if present:
                    out.uint(5, reader.uint(codeword_length_length))

        lookup_type = reader.uint(1)
        out.uint(4, lookup_type)
        if 1 == lookup_type:
            out.copy_uint(reader, 32)  # minimum
            out.copy_uint(reader, 32)  # maximum
            value_length = out.copy_uint(reader, 4)
            out.copy_uint(reader, 1)   # sequence flag
            for _ in range(book_maptype1_quantvals(entries, dimensions)):
                out.copy_uint(reader, value_length + 1)
        elif 0 != lookup_type:
            raise WemError("unsupported codebook lookup type %d" % lookup_type)

        # Every packed codebook ends inside its last byte, so one partial byte
        # is expected; anything else means the bit widths above went wrong.
        if reader.total_bits_read // 8 + 1 != len(body):
            raise WemError("codebook %d size mismatch: read %d of %d bytes"
                           % (index, reader.total_bits_read // 8 + 1, len(body)))


class WwiseRiffVorbis:
    def __init__(self, path: Path, codebooks: Path = CODEBOOK_LIBRARY):
        self.path = path
        self._data = path.read_bytes()
        self._codebooks_path = codebooks
        self._parse()

    # -- parsing -----------------------------------------------------------
    def _u16(self, offset: int) -> int:
        return struct.unpack_from(self._e + "H", self._data, offset)[0]

    def _u32(self, offset: int) -> int:
        return struct.unpack_from(self._e + "I", self._data, offset)[0]

    def _parse(self) -> None:
        data = self._data
        if data[:4] == b"RIFX":
            self._e = ">"
        elif data[:4] == b"RIFF":
            self._e = "<"
        else:
            raise WemError("missing RIFF")
        riff_size = self._u32(4) + 8
        if riff_size > len(data):
            raise WemError("RIFF truncated")
        if data[8:12] != b"WAVE":
            raise WemError("missing WAVE")

        chunks = {}
        offset = 12
        while offset < riff_size:
            if offset + 8 > riff_size:
                raise WemError("chunk header truncated")
            name = data[offset:offset + 4]
            size = self._u32(offset + 4)
            chunks[name] = (offset + 8, size)
            offset += 8 + size
        if offset > riff_size:
            raise WemError("chunk truncated")

        if b"fmt " not in chunks or b"data" not in chunks:
            raise WemError("expected fmt and data chunks")
        fmt_offset, fmt_size = chunks[b"fmt "]
        self._data_offset, self._data_size = chunks[b"data"]

        vorb = chunks.get(b"vorb")
        if vorb is None:
            if 0x42 != fmt_size:
                raise WemError("expected a 0x42 fmt when there is no vorb chunk")
            vorb_offset, vorb_size = fmt_offset + 0x18, -1
        else:
            vorb_offset, vorb_size = vorb
            if fmt_size not in (0x28, 0x18, 0x12):
                raise WemError("bad fmt size 0x%X" % fmt_size)

        if 0xFFFF != self._u16(fmt_offset):
            raise WemError("not Wwise Vorbis (codec id 0x%X)" % self._u16(fmt_offset))
        self.channels = self._u16(fmt_offset + 2)
        self.sample_rate = self._u32(fmt_offset + 4)
        self.avg_bytes_per_second = self._u32(fmt_offset + 8)
        if 0 != self._u16(fmt_offset + 12):
            raise WemError("bad block align")
        if 0 != self._u16(fmt_offset + 14):
            raise WemError("expected 0 bits per sample")
        if fmt_size - 0x12 != self._u16(fmt_offset + 16):
            raise WemError("bad extra fmt length")

        self.loop_count = 0
        self.loop_start = 0
        self.loop_end = 0
        smpl = chunks.get(b"smpl")
        if smpl is not None:
            self.loop_count = self._u32(smpl[0] + 0x1C)
            if 1 != self.loop_count:
                raise WemError("expected exactly one loop")
            self.loop_start = self._u32(smpl[0] + 0x2C)
            self.loop_end = self._u32(smpl[0] + 0x30)

        if vorb_size not in (-1, 0x28, 0x2A, 0x2C, 0x32, 0x34):
            raise WemError("bad vorb size 0x%X" % vorb_size)

        self.sample_count = self._u32(vorb_offset)
        self.no_granule = False
        self.mod_packets = False
        self.header_triad_present = False
        self.old_packet_headers = False

        if vorb_size in (-1, 0x2A):
            self.no_granule = True
            mod_signal = self._u32(vorb_offset + 0x04)
            # 0x4A/0x4B/0x69/0x70 are the values seen on standard packets;
            # everything else (0xD9, 0xCB, 0xBC, 0xB2, ...) means modified.
            if mod_signal not in (0x4A, 0x4B, 0x69, 0x70):
                self.mod_packets = True
            cursor = vorb_offset + 0x10
        else:
            cursor = vorb_offset + 0x18

        self.setup_packet_offset = self._u32(cursor)
        self.first_audio_packet_offset = self._u32(cursor + 4)

        self.blocksize_0_pow = 0
        self.blocksize_1_pow = 0
        if vorb_size in (0x28, 0x2C):
            self.header_triad_present = True
            self.old_packet_headers = True
        else:
            tail = vorb_offset + (0x24 if vorb_size in (-1, 0x2A) else 0x2C)
            self.uid = self._u32(tail)
            self.blocksize_0_pow = self._data[tail + 4]
            self.blocksize_1_pow = self._data[tail + 5]

        if self.loop_count:
            self.loop_end = self.sample_count if 0 == self.loop_end else self.loop_end + 1
            if (self.loop_start >= self.sample_count or self.loop_end > self.sample_count
                    or self.loop_start > self.loop_end):
                raise WemError("loop points out of range")

    # -- packets -----------------------------------------------------------
    def _packet(self, offset: int):
        """(payload offset, size, granule, next offset) of a Wwise packet."""
        if self.old_packet_headers:
            size = self._u32(offset)
            granule = self._u32(offset + 4)
            header = 8
        elif self.no_granule:
            size = self._u16(offset)
            granule = 0
            header = 2
        else:
            size = self._u16(offset)
            granule = self._u32(offset + 2)
            header = 6
        return offset + header, size, granule, offset + header + size

    # -- generation --------------------------------------------------------
    def to_ogg(self) -> bytes:
        if self.header_triad_present:
            raise WemError("the pre-2011 header-triad layout is not supported")
        out = OggWriter()
        mode_blockflag, mode_bits = self._write_headers(out)
        self._write_audio(out, mode_blockflag, mode_bits)
        out.finish()
        return out.bytes()

    @staticmethod
    def _packet_header(out: OggWriter, kind: int) -> None:
        out.uint(8, kind)
        for c in b"vorbis":
            out.uint(8, c)

    def _write_headers(self, out: OggWriter):
        # identification
        self._packet_header(out, 1)
        out.uint(32, 0)                                   # vorbis version
        out.uint(8, self.channels)
        out.uint(32, self.sample_rate)
        out.uint(32, 0)                                   # bitrate maximum
        out.uint(32, self.avg_bytes_per_second * 8)       # bitrate nominal
        out.uint(32, 0)                                   # bitrate minimum
        out.uint(4, self.blocksize_0_pow)
        out.uint(4, self.blocksize_1_pow)
        out.uint(1, 1)                                    # framing
        out.end_packet(0, flush=True)

        # comment
        self._packet_header(out, 3)
        out.uint(32, len(VENDOR))
        for c in VENDOR:
            out.uint(8, c)
        if self.loop_count:
            comments = [b"LoopStart=%d" % self.loop_start, b"LoopEnd=%d" % self.loop_end]
        else:
            comments = []
        out.uint(32, len(comments))
        for comment in comments:
            out.uint(32, len(comment))
            for c in comment:
                out.uint(8, c)
        out.uint(1, 1)
        out.end_packet(0, flush=True)

        # setup
        self._packet_header(out, 5)
        payload_offset, packet_size, granule, next_offset = self._packet(
            self._data_offset + self.setup_packet_offset)
        if granule:
            raise WemError("setup packet granule != 0")
        reader = BitReader(self._data, payload_offset)

        codebook_count = out.copy_uint(reader, 8) + 1
        library = CodebookLibrary(self._codebooks_path)
        for _ in range(codebook_count):
            library.rebuild(reader.uint(10), out)

        out.uint(6, 0)      # time domain transform count - 1
        out.uint(16, 0)     # the one placeholder transform

        floor_count = out.copy_uint(reader, 6) + 1
        for _ in range(floor_count):
            out.uint(16, 1)  # Wwise only ever uses floor type 1
            partitions = out.copy_uint(reader, 5)
            partition_classes = [out.copy_uint(reader, 4) for _ in range(partitions)]
            maximum_class = max(partition_classes) if partition_classes else 0
            class_dimensions = []
            for _ in range(maximum_class + 1):
                class_dimensions.append(out.copy_uint(reader, 3) + 1)
                subclasses = out.copy_uint(reader, 2)
                if subclasses:
                    masterbook = out.copy_uint(reader, 8)
                    if masterbook >= codebook_count:
                        raise WemError("invalid floor1 masterbook")
                for _ in range(1 << subclasses):
                    subclass_book = out.copy_uint(reader, 8) - 1
                    if subclass_book >= codebook_count:
                        raise WemError("invalid floor1 subclass book")
            out.copy_uint(reader, 2)                 # multiplier - 1
            rangebits = out.copy_uint(reader, 4)
            for partition_class in partition_classes:
                for _ in range(class_dimensions[partition_class]):
                    out.copy_uint(reader, rangebits)

        residue_count = out.copy_uint(reader, 6) + 1
        for _ in range(residue_count):
            residue_type = reader.uint(2)
            out.uint(16, residue_type)
            if residue_type > 2:
                raise WemError("invalid residue type")
            out.copy_uint(reader, 24)                # begin
            out.copy_uint(reader, 24)                # end
            out.copy_uint(reader, 24)                # partition size - 1
            classifications = out.copy_uint(reader, 6) + 1
            classbook = out.copy_uint(reader, 8)
            if classbook >= codebook_count:
                raise WemError("invalid residue classbook")
            cascade = []
            for _ in range(classifications):
                low_bits = out.copy_uint(reader, 3)
                high_bits = 0
                if out.copy_uint(reader, 1):
                    high_bits = out.copy_uint(reader, 5)
                cascade.append(high_bits * 8 + low_bits)
            for value in cascade:
                for k in range(8):
                    if value & (1 << k):
                        if out.copy_uint(reader, 8) >= codebook_count:
                            raise WemError("invalid residue book")

        mapping_count = out.copy_uint(reader, 6) + 1
        for _ in range(mapping_count):
            out.uint(16, 0)                          # mapping type 0, the only one
            submaps = 1
            if out.copy_uint(reader, 1):
                submaps = out.copy_uint(reader, 4) + 1
            if out.copy_uint(reader, 1):             # square polar
                coupling_steps = out.copy_uint(reader, 8) + 1
                for _ in range(coupling_steps):
                    magnitude = out.copy_uint(reader, ilog(self.channels - 1))
                    angle = out.copy_uint(reader, ilog(self.channels - 1))
                    if angle == magnitude or magnitude >= self.channels or angle >= self.channels:
                        raise WemError("invalid coupling")
            if out.copy_uint(reader, 2):             # a reserved field Wwise keeps
                raise WemError("mapping reserved field nonzero")
            if submaps > 1:
                for _ in range(self.channels):
                    if out.copy_uint(reader, 4) >= submaps:
                        raise WemError("mapping mux >= submaps")
            for _ in range(submaps):
                out.copy_uint(reader, 8)             # unused time configuration
                if out.copy_uint(reader, 8) >= floor_count:
                    raise WemError("invalid floor mapping")
                if out.copy_uint(reader, 8) >= residue_count:
                    raise WemError("invalid residue mapping")

        mode_count = out.copy_uint(reader, 6) + 1
        mode_blockflag = []
        mode_bits = ilog(mode_count - 1)
        for _ in range(mode_count):
            mode_blockflag.append(bool(out.copy_uint(reader, 1)))
            out.uint(16, 0)                          # window type
            out.uint(16, 0)                          # transform type
            if out.copy_uint(reader, 8) >= mapping_count:
                raise WemError("invalid mode mapping")
        out.uint(1, 1)                               # framing
        out.end_packet(0, flush=True)

        if (reader.total_bits_read + 7) // 8 != packet_size:
            raise WemError("setup packet not consumed exactly (%d of %d bytes)"
                           % ((reader.total_bits_read + 7) // 8, packet_size))
        if next_offset != self._data_offset + self.first_audio_packet_offset:
            raise WemError("first audio packet does not follow the setup packet")
        return mode_blockflag, mode_bits

    def _write_audio(self, out: OggWriter, mode_blockflag, mode_bits) -> None:
        end = self._data_offset + self._data_size
        block_size = (1 << self.blocksize_0_pow, 1 << self.blocksize_1_pow)
        packets = self._audio_packets(end, mode_blockflag, mode_bits)

        # Wwise's 2-byte packet headers carry no granule at all, so the page
        # positions have to be derived: a Vorbis packet outputs
        # (previous blocksize + this blocksize) / 4 samples, and the first one
        # only primes the overlap. Without this every page says 0 and a decoder
        # reads the file as empty.
        granules = []
        running = 0
        for index, packet in enumerate(packets):
            if index:
                previous = block_size[1 if packets[index - 1][4] else 0]
                current = block_size[1 if packet[4] else 0]
                running += (previous + current) // 4
            granules.append(running)
        if packets and 0 < self.sample_count <= running:
            granules[-1] = self.sample_count

        prev_blockflag = False
        for index, (payload_offset, size, source_granule, _next, blockflag, mode_number) in                 enumerate(packets):
            granule = granules[index] if self.no_granule else (
                1 if 0xFFFFFFFF == source_granule else source_granule)

            if self.mod_packets:
                # Wwise drops the packet type and the window flags; rebuild them.
                out.uint(1, 0)                       # packet type: audio
                reader = BitReader(self._data, payload_offset)
                out.copy_uint(reader, mode_bits)
                remainder = reader.uint(8 - mode_bits)
                if blockflag:
                    next_blockflag = packets[index + 1][4] if index + 1 < len(packets) else False
                    out.uint(1, 1 if prev_blockflag else 0)
                    out.uint(1, 1 if next_blockflag else 0)
                prev_blockflag = blockflag
                out.uint(8 - mode_bits, remainder)
            else:
                out.uint(8, self._data[payload_offset])

            for i in range(payload_offset + 1, payload_offset + size):
                out.uint(8, self._data[i])
            out.end_packet(granule, last=(index == len(packets) - 1))

    def _audio_packets(self, end: int, mode_blockflag, mode_bits):
        """Every audio packet as (payload offset, size, granule, next, blockflag, mode)."""
        packets = []
        offset = self._data_offset + self.first_audio_packet_offset
        while offset < end:
            payload_offset, size, granule, next_offset = self._packet(offset)
            if payload_offset > end:
                raise WemError("packet header truncated")
            if next_offset > end:
                raise WemError("audio data truncated")
            mode_number = 0
            blockflag = False
            if size > 0:
                reader = BitReader(self._data, payload_offset)
                if not self.mod_packets:
                    reader.bit()                     # packet type, always 0 here
                mode_number = reader.uint(mode_bits)
                if mode_number >= len(mode_blockflag):
                    raise WemError("packet mode %d out of range" % mode_number)
                blockflag = mode_blockflag[mode_number]
            packets.append((payload_offset, size, granule, next_offset, blockflag, mode_number))
            offset = next_offset
        return packets


def convert(path: Path, out_path: Path, codebooks: Path = CODEBOOK_LIBRARY) -> int:
    wem = WwiseRiffVorbis(path, codebooks)
    ogg = wem.to_ogg()
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_bytes(ogg)
    return len(ogg)


def open_wav_decoder():
    """An FMOD decoder for --wav, or None with the reason printed."""
    import fmod_decode
    try:
        return fmod_decode.FmodDecoder()
    except fmod_decode.FmodError as error:
        print("--wav needs FMOD: %s" % error, file=sys.stderr)
        return None


def write_wav(decoder, ogg_path: Path) -> bool:
    import fmod_decode
    try:
        decoder.to_wav(ogg_path, ogg_path.with_suffix(".wav"))
    except fmod_decode.FmodError as error:
        print("%s: %s" % (ogg_path.name, error), file=sys.stderr)
        return False
    return True


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("input", type=Path, help=".wem file, or a directory to walk")
    ap.add_argument("-o", "--out", type=Path, help="output .ogg for a single input")
    ap.add_argument("--out-dir", type=Path, help="output directory for a directory input")
    ap.add_argument("--codebooks", type=Path, default=CODEBOOK_LIBRARY)
    ap.add_argument("--wav", action="store_true",
                    help="also decode each result to 16-bit PCM wav through FMOD")
    a = ap.parse_args()

    decoder = open_wav_decoder() if a.wav else None
    if a.wav and decoder is None:
        return 2

    if a.input.is_dir():
        out_dir = a.out_dir or a.input
        failures = 0
        sources = sorted(a.input.rglob("*.wem"))
        for source in sources:
            target = (out_dir / source.relative_to(a.input)).with_suffix(".ogg")
            try:
                size = convert(source, target, a.codebooks)
            except WemError as error:
                print("%s: %s" % (source.name, error), file=sys.stderr)
                failures += 1
                continue
            if decoder is not None and not write_wav(decoder, target):
                failures += 1
            print("%s -> %s (%d bytes)" % (source.name, target.name, size))
        if decoder is not None:
            decoder.close()
        print("%d/%d converted" % (len(sources) - failures, len(sources)))
        return 1 if failures else 0

    target = a.out or a.input.with_suffix(".ogg")
    try:
        size = convert(a.input, target, a.codebooks)
    except WemError as error:
        print("%s: %s" % (a.input.name, error), file=sys.stderr)
        return 1
    failed = decoder is not None and not write_wav(decoder, target)
    if decoder is not None:
        decoder.close()
    print("%s -> %s (%d bytes)" % (a.input.name, target, size))
    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
