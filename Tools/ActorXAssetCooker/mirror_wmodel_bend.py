"""Mirror the bend direction of a skinned .wmodel clip.

Every rotation key in every WANM (animation) section is replaced by its
quaternion conjugate (x, y, z, w) -> (-x, -y, -z, w), i.e. each bone's local
rotation is reversed. For a paper-roll rig whose bones bend about one axis this
flips the unfold direction (rolls/unrolls the other way) while leaving the mesh,
the flat end pose, and the printed face untouched.

Only the 12 rotation-key floats are patched in place: position/scale keys, the
channel table, section headers, and every byte size stay identical, so the file
remains byte-compatible with the decoder. Writes atomically (temp + os.replace)
after a SHA-256 backup and re-reads to confirm |q| is preserved and nothing but
the rotation vectors moved.

Usage: python mirror_wmodel_bend.py <path-to-.wmodel>
"""
import struct, sys, os, hashlib, io

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding="utf-8", errors="replace")

WANM = b"WANM"
CHAN_FMT = "<QIIIIIIiI"   # boneHash, posCount, posOff, rotCount, rotOff, sclCount, sclOff, cbi, reserved
CHAN_SIZE = 40
KEY_SIZE = 20             # QUATERNION_KEY: time,x,y,z,w


def _rot_key_offsets(data):
    """Yield absolute byte offset of every rotation QUATERNION_KEY."""
    off = 0
    while True:
        w = data.find(WANM, off)
        if w < 0:
            break
        off = w + 4
        channel_count = struct.unpack_from("<I", data, w + 4)[0]
        chan_base = w + 32
        pool_base = chan_base + channel_count * CHAN_SIZE
        for c in range(channel_count):
            f = struct.unpack_from(CHAN_FMT, data, chan_base + c * CHAN_SIZE)
            rot_count, rot_off = f[3], f[4]
            for k in range(rot_count):
                yield pool_base + rot_off + k * KEY_SIZE


def mirror(path):
    data = bytearray(open(path, "rb").read())
    sha = hashlib.sha256(data).hexdigest()
    backup = path + ".prebend." + sha[:12] + ".bak"
    if not os.path.exists(backup):
        with open(backup, "wb") as fh:
            fh.write(data)

    offsets = list(_rot_key_offsets(data))
    if not offsets:
        raise SystemExit("no WANM rotation keys found")

    for o in offsets:
        t, x, y, z, wq = struct.unpack_from("<5f", data, o)
        struct.pack_into("<5f", data, o, t, -x, -y, -z, wq)  # conjugate: reverse the rotation

    tmp = path + ".tmp." + os.urandom(6).hex()
    with open(tmp, "wb") as fh:
        fh.write(data)
    os.replace(tmp, path)
    return offsets, backup, sha


def verify(path, backup, offsets):
    new = open(path, "rb").read()
    old = open(backup, "rb").read()
    assert len(new) == len(old), "file size changed"
    # every differing byte must lie inside a rotation-key's x/y/z span (bytes 4..15 of the 20)
    allowed = set()
    for o in offsets:
        for b in range(4, 16):   # x,y,z floats; time(0..3) and w(16..19) must stay put
            allowed.add(o + b)
    diffs = [i for i in range(len(new)) if new[i] != old[i]]
    outside = [i for i in diffs if i not in allowed]
    assert not outside, f"{len(outside)} bytes changed outside rotation x/y/z"
    # unit-length preserved on a sample
    import math
    ok = 0
    for o in offsets:
        t, x, y, z, wq = struct.unpack_from("<5f", new, o)
        n = math.sqrt(x * x + y * y + z * z + wq * wq)
        assert abs(n - 1.0) < 1e-3, f"non-unit quaternion |q|={n}"
        ok += 1
    return len(diffs), ok


if __name__ == "__main__":
    target = sys.argv[1] if len(sys.argv) > 1 else None
    if not target or not os.path.isfile(target):
        raise SystemExit("usage: python mirror_wmodel_bend.py <path-to-.wmodel>")
    offsets, backup, sha = mirror(target)
    diff_bytes, keys = verify(target, backup, offsets)
    print(f"MIRROR_OK keys={keys} changed_bytes={diff_bytes} backup={os.path.basename(backup)}")
    print(f"  source sha256 {sha}")
