"""Applies the desktop tuning sheet to the KoukuSaton pop-up book cutscene.

The sheet lives at "바탕 화면/쿠크컷신_조절값.txt" so the numbers can be edited
without touching a script. Every value it carries is rebuilt from scratch here,
so re-running never accumulates offsets.

Group timing is read off 책2.mp4, a slow-motion capture of the original, as
plain video seconds with the moment the cover starts opening as zero. Every
placement in a group shares one start and one end, so a group moves as a single
piece the way the paper fans do in the reference instead of 421 placements
trickling in one after another.

Motion follows the original frame by frame (연출 1.mp4 10.6s-12.3s):
  - curtains start bunched toward the stage like two closed hand fans and sweep
    around the arena centre to their own angle while standing up,
  - stands and every other upright piece lie flat and hinge up in place,
  - the centre stage disc comes last, growing up out of the page.

CWorldSequencePlayer composes a key in the placement's own frame: the offset is
rotated by the baseline rotation and the key quaternion is applied before it
(XMQuaternionMultiply(key, baseline)). A world-space orbit around the arena
centre therefore becomes key = B^-1 * R * B * F and offset = B^-1 * (R(p-c) - (p-c)).
The script re-composes every curtain key with the same algebra and refuses to
write a document whose bunched pose does not land on the spine direction.

    python Tools/KakulSaydonPipeline/apply_cutscene_tuning.py
"""

import io
import json
import math
import os
import re
import sys

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding="utf-8", errors="replace")

REPOSITORY_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
os.chdir(REPOSITORY_ROOT)

SHEET = os.path.expanduser("~/OneDrive/바탕 화면/쿠크컷신_조절값.txt")
AREA = "LV_LUT_MIDNIGHTC_ED"
CENTER = (-0.3, 737.5)
RADIUS = 80.0
MAX_TRACKS = 32
# CMapTool plays exactly this many arena_rise instances
# (KAKUL_ARENA_RISE_INSTANCE_COUNT in MapTool.cpp).
INSTANCE_COUNT = 14
BOOK_PLACEMENT_ID = 6
BOOK_CLIP = "bg_rad_koukusaton_book.ao_evt2_book02"
BOOK_CLIP_MS = 2370
# WorldSequenceDocument MIN_RUNTIME_SCALE_DETERMINANT.
MIN_DETERMINANT = 1e-6
# Keys sampled along each unfold. The player eases per segment when a template
# is SMOOTH_STEP, which pulses across many keys, so templates are LINEAR and
# the easing is baked into the key spacing instead.
SUBKEYS = 10
# Floor pieces above this height are the stand tiers, not the stage disc.
STAND_FLOOR_MIN_Y = 5.0
SEQ_PREFIX = f"sequence.{AREA}.arena_rise_"
INST_PREFIX = "world.sequence.instance.arena_rise_"
CATEGORIES = ("커튼", "벽", "게이트", "관람석", "장식", "조명", "바닥")

DEFAULTS = {
    "SLOW": 2.0, "BOOK_SCALE": 1.4, "BOOK_Y": 0.2, "BOOK_SPEED": 0.5,
    "BOOK_YAW": 45.0, "FOLDED_SCALE": 0.12, "OVERSHOOT": 1.06, "RISE_FROM": 7.0,
    "FOLD_ANGLE": -90.0, "FOLD_AXIS": 2.0,
    "FAN_SWEEP": 1.0, "SPINE_DEG": 135.0,
    "BOOK2_OPEN_START": 12.3, "BOOK2_SLOWMO": 2.0,
}


def read_sheet():
    values = dict(DEFAULTS)
    groups = {}
    if not os.path.isfile(SHEET):
        raise SystemExit(f"조절값 시트가 없습니다: {SHEET}")
    for line in open(SHEET, encoding="utf-8").read().splitlines():
        line = line.split("#")[0].strip()
        m = re.match(r"^GROUP_(\S+)\s*=\s*(-?[0-9.]+)\s*,\s*(-?[0-9.]+)", line)
        if m:
            groups[m.group(1)] = (float(m.group(2)), float(m.group(3)))
            continue
        m = re.match(r"^([A-Z0-9_]+)\s*=\s*(-?[0-9.]+)", line)
        if m and m.group(1) in values:
            values[m.group(1)] = float(m.group(2))
    missing = [c for c in CATEGORIES if c not in groups]
    if missing:
        raise SystemExit(f"GROUP 값이 빠졌습니다: {missing}")
    return values, groups


def category(mesh, y):
    if "curtain" in mesh:
        return "커튼"
    if "wall" in mesh:
        return "벽"
    if "floor" in mesh:
        # The raised floor tiers carry the seats; they are the green stands
        # that hinge up in the original, not the stage disc that comes last.
        return "관람석" if y > STAND_FLOOR_MIN_Y else "바닥"
    if "lighting" in mesh:
        return "조명"
    if "chair" in mesh or "fence" in mesh:
        return "관람석"
    if "gate" in mesh:
        return "게이트"
    return "장식"


def load_arena():
    catalog = {}
    path = f"Data/Maps/Imported/{AREA}/{AREA}.mapassets"
    for line in open(path, encoding="utf-8").read().splitlines()[1:]:
        q = line.split('"')
        if len(q) > 3:
            catalog[q[1]] = q[3]
    items = []
    path = f"Data/Maps/Authoring/{AREA}/{AREA}.mapplacements"
    for line in open(path, encoding="utf-8").read().splitlines()[1:]:
        t = line.split()
        # Field 15 is MAP_PLACEMENT_RECORD::visible. Culling boxes and the
        # roulette wheel are authored invisible and must stay that way.
        if len(t) != 16 or t[15] != "1":
            continue
        x, y, z = float(t[5]), float(t[6]), float(t[7])
        if math.hypot(x - CENTER[0], z - CENTER[1]) > RADIUS:
            continue
        det = abs(float(t[12]) * float(t[13]) * float(t[14]))
        items.append({
            "id": int(t[0]),
            "cat": category(catalog.get(t[4].strip('"'), ""), y),
            "det": det,
            "pos": (x, y, z),
            # Fields 8..11 are rotationQuaternion x y z w.
            "rot": tuple(float(v) for v in t[8:12]),
        })
    return items


def write_book_placement(values):
    path = f"Data/Maps/Authoring/{AREA}/{AREA}.deployplacements"
    raw = open(path, "rb").read().decode("utf-8")
    sep = "\r\n" if "\r\n" in raw else "\n"
    lines = [l for l in raw.split(sep) if l != ""]
    for index, line in enumerate(lines[1:], 1):
        t = line.split()
        if len(t) == 17 and t[0] == str(BOOK_PLACEMENT_ID):
            t[6] = str(values["BOOK_Y"])
            # The arena sits 45 degrees off the world axes, so the book needs a
            # yaw of its own or its spine cuts across it diagonally.
            half = math.radians(values["BOOK_YAW"]) * 0.5
            t[8] = "0"
            t[9] = f"{math.sin(half):.9f}"
            t[10] = "0"
            t[11] = f"{math.cos(half):.9f}"
            # Only field 12 is uniformScale; 13..15 are flags.
            t[12] = str(values["BOOK_SCALE"])
            lines[index] = " ".join(t)
    open(path, "wb").write((sep.join(lines) + sep).encode("utf-8"))


# ---- quaternion algebra, Hamilton product, (x, y, z, w) like XMFLOAT4 --------

def qmul(a, b):
    ax, ay, az, aw = a
    bx, by, bz, bw = b
    return (aw * bx + ax * bw + ay * bz - az * by,
            aw * by - ax * bz + ay * bw + az * bx,
            aw * bz + ax * by - ay * bx + az * bw,
            aw * bw - ax * bx - ay * by - az * bz)


def qconj(q):
    return (-q[0], -q[1], -q[2], q[3])


def qnorm(q):
    n = math.sqrt(sum(v * v for v in q)) or 1.0
    q = tuple(v / n for v in q)
    return tuple(-v for v in q) if q[3] < 0 else q


def qrot(v, q):
    """XMVector3Rotate: q v q*."""
    r = qmul(qmul(q, (v[0], v[1], v[2], 0.0)), qconj(q))
    return (r[0], r[1], r[2])


def qaxis(axis, angle):
    h = angle * 0.5
    q = [0.0, 0.0, 0.0, math.cos(h)]
    q[axis] = math.sin(h)
    return tuple(q)


def smoothstep(p):
    p = min(1.0, max(0.0, p))
    return p * p * (3.0 - 2.0 * p)


def schedule(values, groups, items):
    """책2.mp4 초 -> 우리 타임라인 ms. 표지가 열리기 시작하는 순간이 0.

    원작은 표지가 열리는 도중에 이미 접힌 맵이 보이지만, 사용자 요청으로 책이
    완전히 펼쳐진 뒤에 맵이 나온다. 그룹 사이의 순서와 겹침은 책2.mp4 실측
    그대로 두고, 가장 이른 그룹이 책 종료 시각에 오도록 전체를 통째로 민다.
    """
    open_start = values["BOOK2_OPEN_START"]
    scale = values["SLOW"] / values["BOOK2_SLOWMO"] * 1000.0

    def ms(t):
        return int(round((t - open_start) * scale))

    for cat in CATEGORIES:
        a, b = groups[cat]
        if a < open_start or b <= a:
            raise SystemExit(
                f"GROUP_{cat} = {a}, {b} : 시작은 BOOK2_OPEN_START({open_start}) "
                "이상, 끝은 시작보다 커야 합니다")

    book_ms = int(round(BOOK_CLIP_MS / values["BOOK_SPEED"]))
    shift = max(0, book_ms - min(ms(groups[c][0]) for c in CATEGORIES))
    show = book_ms
    windows = {}
    for cat in CATEGORIES:
        a, b = groups[cat]
        start = max(ms(a) + shift, show + 2)
        end = max(ms(b) + shift, start + SUBKEYS * 5)
        windows[cat] = (start, end)

    # 회전으로 펴는 그룹은 크기가 그대로다. 바닥만 크기로 펼치므로 바닥 배치의
    # 행렬식 * FOLDED_SCALE^3 이 엔진 하한을 넘어야 문서가 admission 된다.
    folded = values["FOLDED_SCALE"]
    bad = [it["id"] for it in items
           if it["cat"] == "바닥" and it["det"] * folded ** 3 <= MIN_DETERMINANT * 1.05]
    if bad:
        raise SystemExit(
            f"FOLDED_SCALE {folded} 는 바닥 배치 {bad} 의 행렬식을 엔진 하한 "
            f"{MIN_DETERMINANT} 아래로 떨어뜨립니다. 값을 올리세요.")
    return show, windows, shift


def build(values, show, windows, items):
    path = f"Data/Maps/Authoring/{AREA}/{AREA}.worldsequences.json"
    doc = json.load(open(path, encoding="utf-8"))
    doc["templates"] = [t for t in doc["templates"]
                        if not t["sequenceId"].startswith(SEQ_PREFIX)]
    doc["instances"] = [i for i in doc["instances"]
                        if not i["instanceId"].startswith(INST_PREFIX)]

    folded = values["FOLDED_SCALE"]
    over = values["OVERSHOOT"]
    rise = values["RISE_FROM"]
    fold = math.radians(values["FOLD_ANGLE"])
    axis = int(values["FOLD_AXIS"])
    sweep_on = values["FAN_SWEEP"] >= 0.5
    spine = math.radians(values["SPINE_DEG"])
    # 0 = +Z, 90 = +X, same convention as the sheet.
    u = (math.sin(spine), 0.0, math.cos(spine))
    identity = (0.0, 0.0, 0.0, 1.0)

    def key(ms, offset, rot, scale, visible):
        return {"timeMs": int(ms), "positionOffset": [round(v, 5) for v in offset],
                "rotationQuaternion": [round(v, 7) for v in qnorm(rot)],
                "scaleMultiplier": [round(scale, 6)] * 3, "visible": visible}

    def sweep_start(item):
        """부채가 뭉쳐 있을 때 이 커튼이 서 있어야 할 세계 yaw. 없으면 0."""
        if not sweep_on or item["cat"] != "커튼":
            return 0.0
        x, _, z = item["pos"]
        d = (x - CENTER[0], 0.0, z - CENTER[1])
        if math.hypot(d[0], d[2]) < 1e-3:
            return 0.0
        phi = math.atan2(u[0] * d[2] - u[2] * d[0], u[0] * d[0] + u[2] * d[2])
        # Whichever sign of phi lands the placement on the spine direction is
        # the sweep; the quaternion handedness decides, not a guessed formula.
        best = None
        for theta in (phi, -phi):
            r = qrot(d, qaxis(1, theta))
            err = abs(math.atan2(u[0] * r[2] - u[2] * r[0],
                                 u[0] * r[0] + u[2] * r[2]))
            if best is None or err < best[0]:
                best = (err, theta)
        return best[1]

    def pose(item, e):
        """진행도 e(0=접힘, 1=완료)에서의 (offset, rot, scale)."""
        cat = item["cat"]
        if cat == "바닥":
            # 원판은 페이지 아래에서 작게 솟아 살짝 넘겼다가 자리를 잡는다.
            if e <= 0.72:
                k = smoothstep(e / 0.72)
                return ((0.0, -rise + (rise + 0.6) * k, 0.0), identity,
                        folded + (over - folded) * k)
            k = smoothstep((e - 0.72) / 0.28)
            return ((0.0, 0.6 * (1.0 - k), 0.0), identity, over + (1.0 - over) * k)
        k = smoothstep(e)
        f = qaxis(axis, fold * (1.0 - k))
        theta0 = item["theta0"]
        if theta0 == 0.0:
            return ((0.0, 0.0, 0.0), f, 1.0)
        b = item["rot"]
        r = qaxis(1, theta0 * (1.0 - k))
        x, _, z = item["pos"]
        d = (x - CENTER[0], 0.0, z - CENTER[1])
        rd = qrot(d, r)
        world_offset = (rd[0] - d[0], rd[1] - d[1], rd[2] - d[2])
        offset = qrot(world_offset, qconj(b))
        rot = qmul(qmul(qmul(qconj(b), r), b), f)
        return (offset, rot, 1.0)

    def track_keys(item):
        start, end = windows[item["cat"]]
        o0, r0, s0 = pose(item, 0.0)
        keys = []
        if show > 0:
            keys += [key(0, o0, r0, s0, False), key(show, o0, r0, s0, False),
                     key(show + 1, o0, r0, s0, True)]
        else:
            keys.append(key(0, o0, r0, s0, True))
        for j in range(SUBKEYS + 1):
            e = j / SUBKEYS
            t = start + (end - start) * e
            o, r, s = pose(item, e)
            keys.append(key(round(t), o, r, s, True))
        keys[-1] = key(end, (0.0, 0.0, 0.0), identity, 1.0, True)
        return keys

    # 자체 검증: 플레이어와 같은 합성으로 뭉친 자세가 책등 방향에 놓이는지.
    for item in items:
        item["theta0"] = sweep_start(item)
    worst = 0.0
    for item in [it for it in items if it["theta0"] != 0.0]:
        o, r, _ = pose(item, 0.0)
        b = item["rot"]
        moved = qrot(o, b)
        x, _, z = item["pos"]
        wx, wz = x + moved[0] - CENTER[0], z + moved[2] - CENTER[1]
        err = abs(math.degrees(math.atan2(u[0] * wz - u[2] * wx,
                                          u[0] * wx + u[2] * wz)))
        worst = max(worst, err)
        combined = qnorm(qmul(b, r))
        expect = qnorm(qmul(qmul(qaxis(1, item["theta0"]), b), qaxis(axis, fold)))
        if max(abs(p - q) for p, q in zip(combined, expect)) > 1e-6:
            raise SystemExit(f"커튼 {item['id']} 회전 합성이 예상과 다릅니다")
    if worst > 0.5:
        raise SystemExit(f"부채 시작 자세가 책등 방향에서 {worst:.2f}도 벗어납니다")

    sched = sorted(items, key=lambda it: (windows[it["cat"]][0], it["id"]))
    chunks = [sched[i:i + MAX_TRACKS] for i in range(0, len(sched), MAX_TRACKS)]
    if len(chunks) != INSTANCE_COUNT:
        raise SystemExit(
            f"묶음 {len(chunks)}개 != CMapTool 인스턴스 {INSTANCE_COUNT}개. "
            "배치 수가 바뀌었으면 MapTool.cpp 의 KAKUL_ARENA_RISE_INSTANCE_COUNT 를 맞추세요.")
    for index, chunk in enumerate(chunks):
        tracks, bindings = [], []
        for slot, item in enumerate(chunk, 1):
            sid = f"obj{slot:02d}"
            tracks.append({"slotId": sid, "keys": track_keys(item)})
            bindings.append({"slotId": sid, "targetKind": "MAP_PLACEMENT",
                             "targetId": str(item["id"])})
        duration = max(tr["keys"][-1]["timeMs"] for tr in tracks)
        for tr in tracks:
            if tr["keys"][-1]["timeMs"] < duration:
                tr["keys"].append(key(duration, (0.0, 0.0, 0.0), identity, 1.0, True))
        doc["templates"].append({
            "sequenceId": f"{SEQ_PREFIX}{index:02d}",
            "displayName": f"arena_rise_{index:02d}",
            "category": "World", "durationMs": duration,
            "interpolation": "LINEAR",
            "tracks": tracks, "animationTracks": []})
        doc["instances"].append({
            "instanceId": f"{INST_PREFIX}{index:02d}",
            "templateId": f"{SEQ_PREFIX}{index:02d}",
            "enabled": True, "startDelayMs": 0,
            "playbackSpeed": 1, "bindings": bindings})

    seq = f"sequence.{AREA}.book_open"
    inst = "world.sequence.instance.book_open"
    doc["templates"] = [t for t in doc["templates"] if t["sequenceId"] != seq]
    doc["instances"] = [i for i in doc["instances"] if i["instanceId"] != inst]
    doc["templates"].append({
        "sequenceId": seq, "displayName": "book_open", "category": "Animation",
        "durationMs": BOOK_CLIP_MS, "interpolation": "LINEAR", "tracks": [],
        "animationTracks": [{"slotId": "animated.prop", "clipName": BOOK_CLIP,
                             "playbackRate": 1.0, "loop": False,
                             "holdLastFrame": True}]})
    doc["instances"].append({
        "instanceId": inst, "templateId": seq, "enabled": True,
        "startDelayMs": 0, "playbackSpeed": values["BOOK_SPEED"],
        "bindings": [{"slotId": "animated.prop",
                      "targetKind": "DEPLOY_PLACEMENT",
                      "targetId": str(BOOK_PLACEMENT_ID)}]})

    doc["revision"] = doc.get("revision", 1) + 1
    json.dump(doc, open(path, "w", encoding="utf-8"),
              ensure_ascii=False, indent=1)
    return doc, chunks, worst


def main():
    values, groups = read_sheet()
    items = load_arena()
    show, windows, shift = schedule(values, groups, items)
    write_book_placement(values)
    doc, chunks, worst = build(values, show, windows, items)

    print("조절값")
    for k in ("SLOW", "BOOK_SCALE", "BOOK_Y", "BOOK_YAW", "BOOK_SPEED",
              "FOLDED_SCALE", "OVERSHOOT", "RISE_FROM", "FOLD_ANGLE",
              "FOLD_AXIS", "FAN_SWEEP", "SPINE_DEG", "BOOK2_OPEN_START",
              "BOOK2_SLOWMO"):
        print(f"  {k:<18} {values[k]}")
    print()
    print(f"배치 {len(items)}개 / 인스턴스 {len(chunks)}개 / "
          f"revision {doc['revision']}")
    print(f"책 펼침 완료 = 맵 등장   {show:>6} ms  (그룹 전체를 {shift} ms 뒤로 밈)")
    swept = sum(1 for it in items if it["theta0"] != 0.0)
    print(f"부채 쓸기 커튼 {swept}개, 뭉친 자세의 책등 방향 오차 최대 {worst:.3f}도")
    print()
    print("그룹          개수   책2.mp4 초        우리 ms (시작 ~ 완료)")
    counts = {}
    for it in items:
        counts[it["cat"]] = counts.get(it["cat"], 0) + 1
    for cat in CATEGORIES:
        a, b = groups[cat]
        s, e = windows[cat]
        print(f"  {cat:<6} {counts.get(cat, 0):>6}   {a:5.1f} ~ {b:5.1f}    "
              f"{s:>6} ~ {e:>6}")
    book = int(BOOK_CLIP_MS / values["BOOK_SPEED"])
    end = max(e for _, e in windows.values())
    print(f"\n  책 펼침 완료 {book} ms / 아레나 완료 {end} ms")
    print("  책은 아레나 완료 뒤 MapTool 이 Hold 시간만큼 두었다가 치웁니다.")
    print("\n발행하려면:")
    print("  powershell -ExecutionPolicy Bypass -File "
          "Tools/MapPipeline/Publish-MapAuthoring.ps1 "
          f"-AreaId {AREA} -Mode Publish")


if __name__ == "__main__":
    main()
