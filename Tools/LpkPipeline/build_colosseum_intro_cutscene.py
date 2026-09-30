"""Build the Colosseum match-intro cutscene data from the retail Matinee.

Source: LV_PVP_COLOSSEUM_SCENE01A (package 756RZ5Z6VGS7SKKUYE6RKGULU4B2.upk), InterpData
`interpdata_3` -- the 8 s 3v3 "scene_pvp_*_start" sequence: fade in, an aerial shot of the arena
(camera c2, 0-3 s), a cut to the lineup shot (camera c1, 3-8 s) while the six players stand in
two rows of three, VS and the name rows pop in, fade to black.

Input : the JSON written by extract_scene_matinee.py (this folder) for SCENE01A.
Output: Data/Camera/ColosseumIntro.cutscene.json          (camera, fade, lineup slots, timings)
        Data/UI/Colosseum/IntroCutscene_Layout.json       (VS art from the match-loading layout,
                                                           letterbox bars and the fade plate)

Coordinates. The Matinee tracks are absolute in the arena's frame in X/Z (the octagon centre is
x -0.65, z 0.25 in both), but the scene was authored with the floor at y 0.2 while the arena
floor of the imported map is y 12.64, so every scene Y gets CAMERA_Y_OFFSET. Players keep the
Server-projected floor height FLOOR_Y. Lineup camera c1 is a child of the boom group `interpgroup_11`
(yaw 90 deg in UE, i.e. looking down -Z): its own move track is a local offset, so the eye is the
boom position plus that offset turned by the boom yaw.

Usage: python build_colosseum_intro_cutscene.py --raw scene01a.raw.json
"""
import argparse
import copy
import json
import math
import pathlib

ROOT = pathlib.Path(__file__).resolve().parents[2]
CAMERA_Y_OFFSET = 12.45
FLOOR_Y = 12.64
ARENA_CENTER_X = -0.65
# UE FOVAngle 75 is a horizontal angle for a 16:9 view. The retail intro is letterboxed to 2.35:1 and
# the wider band keeps the VERTICAL angle (Hor+), so the visible horizontal angle is 90.8 deg, not 75.
# Measured on the retail capture: lineup shot 96 deg (six feet vs their slots), aerial shot 84-86 deg
# (six players and the octagon vs the navgrid); a fixed 75 deg framed the lineup about 1.4x too tight.
SOURCE_FOV_X_DEGREES = 75.0
SOURCE_ASPECT = 16.0 / 9.0
LETTERBOX_ASPECT = 2.35
FOV_X_DEGREES = round(math.degrees(2.0 * math.atan(math.tan(math.radians(SOURCE_FOV_X_DEGREES) / 2.0)
                                                   / SOURCE_ASPECT * LETTERBOX_ASPECT)), 3)
# Slot order per team = the order players are assigned; the first player takes the middle slot.
TEAM_SLOT_GROUPS = {"A": ["interpgroup_6", "interpgroup_5", "interpgroup_7"],
                    "B": ["interpgroup_2", "interpgroup_3", "interpgroup_4"]}


def forward(pitch_degrees):
    # UE yaw 90 == runtime -Z. Pitch is negative when looking down.
    p = math.radians(pitch_degrees)
    return [0.0, round(math.sin(p), 6), round(-math.cos(p), 6)]


def track_of(seq, group, cls):
    for g in seq["groups"]:
        if g["group"] == group:
            for t in g["tracks"]:
                if t["class"] == cls:
                    return t
    raise KeyError(group)


def build_cutscene(raw):
    seq = next(s for s in raw["sequences"] if s["data"] == "interpdata_3")
    cuts = seq["liveDirectorTrack"]["cuts"]
    assert [c["camera"] for c in cuts] == ["c2", "c1"], cuts
    director = next(g for g in seq["groups"] if g["group"] == "interpgroupdirector_0")
    fade = next(t for t in director["tracks"] if t["class"] == "interptrackfade")
    events = next(t for t in director["tracks"] if t["class"] == "interptrackevent")
    ev = {k["eventname"]: k["timeMs"] for k in events["keys"]}

    aerial = track_of(seq, "interpgroup_8", "interptrackmove")
    a_pitch = aerial["euler"][0]["rollPitchYawDegrees"][1]
    aerial_keys = [{"timeMs": k["timeMs"],
                    "eye": [k["value"][0], round(k["value"][1] + CAMERA_Y_OFFSET, 6), k["value"][2]]}
                   for k in aerial["position"]]

    boom = track_of(seq, "interpgroup_11", "interptrackmove")
    cam = track_of(seq, "interpgroup_0", "interptrackmove")
    off = cam["position"][0]["value"]          # runtime (x, up, z) local offset, forward is -X in UE
    l_pitch = cam["euler"][0]["rollPitchYawDegrees"][1]
    lineup_keys = [{"timeMs": k["timeMs"],
                    "eye": [round(k["value"][0], 6),
                            round(k["value"][1] + off[1] + CAMERA_Y_OFFSET, 6),
                            round(k["value"][2] - off[0], 6)]}
                   for k in boom["position"]]

    def slot(group):
        m = track_of(seq, group, "interptrackmove")
        p = m["position"][0]["value"]
        th = math.radians(m["euler"][0]["rollPitchYawDegrees"][2])
        yaw = math.degrees(math.atan2(math.cos(th), -math.sin(th)))
        return {"x": round(p[0], 6), "z": round(p[2], 6), "yawDegrees": round(yaw, 3)}

    teams = {}
    for team, groups in TEAM_SLOT_GROUPS.items():
        slots = [slot(g) for g in groups]
        order = sorted(range(len(slots)), key=lambda i: slots[i]["z"])   # far (small z) -> near
        for row, i in enumerate(order):
            slots[i]["row"] = row
        teams[team] = slots

    duration = fade["keys"][-1]["timeMs"]
    return {
        "schema": "lostark.colosseum-intro",
        "formatVersion": 1,
        "source": {"package": "LV_PVP_COLOSSEUM_SCENE01A", "interpData": "interpdata_3",
                   "note": "retail 3v3 match intro; see .md/GB/09-30/2026-09-30_COLOSSEUM_INTRO_CUTSCENE_RESULT.md"},
        "durationMs": duration,
        "returnFadeMs": 600,
        "waitForCharacterTimeoutMs": 6000,
        "floorY": FLOOR_Y,
        "arenaCenterX": ARENA_CENTER_X,
        "fovXDegrees": FOV_X_DEGREES,
        "letterboxAspect": LETTERBOX_ASPECT,
        "fade": [{"timeMs": k["timeMs"], "value": k["value"]} for k in fade["keys"]],
        "shots": [
            {"startMs": cuts[0]["timeMs"], "endMs": cuts[1]["timeMs"],
             "keys": aerial_keys, "forward": forward(a_pitch)},
            {"startMs": cuts[1]["timeMs"], "endMs": duration,
             "keys": lineup_keys, "forward": forward(l_pitch)},
        ],
        "vs": {"showMs": ev["showsequence3"], "hideMs": ev["hidesequence3"]},
        "rows": [{"showMs": ev["showsequence%d" % i], "hideMs": ev["hidesequence%d" % i]}
                 for i in range(3)],
        "teams": teams,
    }


def build_layout(match_layout):
    """VS art (loading-screen slots, re-centred and enlarged) + letterbox bars + fade plate."""
    slots = {s["id"]: s for s in match_layout["slots"]}
    vs_ids = ["MatchLoading_VS_Glow", "MatchLoading_VS_Beam", "MatchLoading_VS_SparksA",
              "MatchLoading_VS_SparksB", "MatchLoading_VS_Flare", "MatchLoading_VS_V",
              "MatchLoading_VS_S"]
    v, s = slots["MatchLoading_VS_V"]["rect"], slots["MatchLoading_VS_S"]["rect"]
    old_cx = (v["x"] + s["x"] + s["width"]) / 2.0
    old_cy = v["y"] + v["height"] / 2.0
    scale, new_cx, new_cy = 1.25, 640.0, 326.0
    out = []
    for sid in vs_ids:
        slot = copy.deepcopy(slots[sid])
        r = slot["rect"]
        cx, cy = r["x"] + r["width"] / 2.0, r["y"] + r["height"] / 2.0
        w, h = r["width"] * scale, r["height"] * scale
        slot["id"] = sid.replace("MatchLoading_VS_", "Intro_VS_")
        slot["rect"] = {"x": round(new_cx + (cx - old_cx) * scale - w / 2.0, 3),
                        "y": round(new_cy + (cy - old_cy) * scale - h / 2.0, 3),
                        "width": round(w, 3), "height": round(h, 3)}
        out.append(slot)
    black = slots["MatchLoading_Black"]
    for sid, rect in (("Intro_BarTop", {"x": 0.0, "y": 0.0, "width": 1280.0, "height": 88.0}),
                      ("Intro_BarBottom", {"x": 0.0, "y": 632.0, "width": 1280.0, "height": 88.0}),
                      ("Intro_Fade", {"x": 0.0, "y": 0.0, "width": 1280.0, "height": 720.0})):
        slot = copy.deepcopy(black)
        slot["id"], slot["rect"] = sid, rect
        out.append(slot)
    doc = copy.deepcopy(match_layout)
    doc["slots"] = out
    return doc


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--raw", required=True, help="extract_scene_matinee.py output for SCENE01A")
    args = ap.parse_args()
    raw = json.loads(pathlib.Path(args.raw).read_text(encoding="utf-8"))
    cutscene = build_cutscene(raw)
    camera_path = ROOT / "Data" / "Camera" / "ColosseumIntro.cutscene.json"
    camera_path.write_text(json.dumps(cutscene, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    match = json.loads((ROOT / "Data" / "UI" / "Colosseum" / "MatchLoading_Layout.json")
                       .read_text(encoding="utf-8"))
    layout_path = ROOT / "Data" / "UI" / "Colosseum" / "IntroCutscene_Layout.json"
    layout_path.write_text(json.dumps(build_layout(match), indent=2, ensure_ascii=False) + "\n",
                           encoding="utf-8")
    print("wrote", camera_path)
    print("wrote", layout_path)


if __name__ == "__main__":
    main()
