# 플레이어 히트 셰이프 원작 복원 계획

창술사 T 적룡필살이 돌진하면서 아무것도 못 맞히는 증상에서 출발해, 원본
`EFTable_SkillEffect`와 대조해 확인한 인테이크 누락 세 곳을 고친다.

## 1. 현재 반영 상태와 이번 작업 경계

### 1.1 적룡필살(34650)의 원본과 현재

원본 DB `SourceData/LPK/data2/EFGame_Extra/ClientData/TableData/EFTable_SkillEffect.db`에서
`PrimaryKey 346500~346506, SecondaryKey=1`을 직접 조회한 결과다. 이 스킬은 트라이포드 변형이
없고 기본 변형 하나가 효과 일곱 개를 전부 소유한다.

| 효과 PK | 정체 | 원본 값 |
|---|---|---|
| 346500 | 미사일 스포너 | `AreaType=0 AreaOrigin=1`, 클립 `flm_sk_dragoncleave_02` 0.27초 |
| 346501 | 미사일 1 접촉 판정 | `AreaType=2 AreaRange=200 AreaAngle=200`, push 110/50 |
| 346502 | 미사일 스포너 | 같은 클립 0.55초 |
| 346503 | 미사일 2 접촉 판정 | `AreaType=2 AreaRange=600 AreaAngle=160`, push 110/50 |
| 346504 | 시전자 타격 | `HitTypeTimeMin=1240 AreaType=2 AreaRange=260 AreaAngle=300 AreaOffsetX=-150` |
| 346505 | 시전자 타격 | 346504와 모든 필드 동일 |
| 346506 | 시전자 타격 | `HitTypeTimeMin=1240 AreaType=2 AreaRange=350 AreaAngle=300 AreaOffsetX=-250` |

현재 게시본은 이 중 346504 하나만 싣는다. 전방 피해를 담당하는 346503의 6m 박스가 통째로
없어서, 루트모션이 4.9m를 전진하는 동안 판정이 캐릭터 뒤쪽 2.6m 박스 하나뿐이다.

루트모션 실측값은 `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap`의
`SKILLROOTMOTION 34650` 74샘플이다. 0ms에서 0m, 900ms에서 -3.02m까지 후퇴하고 1133ms부터
전진해 2433ms에 +1.89m로 끝난다. 타격이 터지는 1240ms의 캐릭터 위치는 -2.21m이므로,
`offset -1.5`에 길이 2.6m인 전방 박스는 -3.71m부터 -1.11m까지만 덮는다.

### 1.2 누락 경로 세 곳

| G | 파일 | 현재 동작 | 원인 |
|---|---|---|---|
| G01 | `fill_animevents_hit_shapes.py` | 한 타격 시각의 도형 중 **첫 개만** 채택 | `reference_shape`가 `(base or pool)[0]` 반환 |
| G02 | `fill_projectiles.py` | 기본 변형이 소유한 투사체를 트라이포드로 오판해 제거 | 판별식이 clipseq 최저 그룹 하나뿐 |
| G03 | `build_hitshapes.py` | v4 문서를 만나면 재생성 자체를 거부 | Action Workbench 소유 보호 장치 |

G02의 전제 오류가 핵심이다. 스크립트 주석은 "높은 clipseq 그룹은 같은 클립에 트라이포드
스폰만 추가한 것"이라고 적고 있지만, 적룡필살의 clipseq는 seq 2/3/4가 모두 존재하고 seq=4만
`flm_sk_dragoncleave_02`라는 **다른 클립**을 쓴다. 그런데 그 seq=4에 달린 미사일의 효과 PK
346500과 346502는 10으로 나누면 34650, 즉 기본 변형 소유다. clipseq 그룹은 이 스킬에서
트라이포드가 아니라 연출 변형을 뜻한다.

### 1.3 이번 범위 밖

- **notify가 있는 클립의 순번 매칭.** `build_rows`가 `ref['hits'][min(ordinal, len-1)]`로
  notify 하나당 도형 하나를 집는 구조는 그대로 둔다. 바꾸면 notify를 가진 모든 스킬의 판정
  개수가 한꺼번에 늘어 회귀 범위를 통제할 수 없다. 별도 G로 분리한다.
- **중복 효과 복원.** 346504와 346505는 원본에서 서로 다른 두 번의 피해지만 `.skilltiming`
  추출 단계에서 이미 한 행으로 합쳐져 있다. 되살리려면 추출기를 다시 돌려야 하므로 제외한다.
  서버가 총 피해를 서브히트 수로 나누는 구조라 총 피해량은 달라지지 않고 표시되는 피해 숫자
  개수만 달라진다.
- **미추출 도형 컬럼.** `AreaAffectDir`(0이 아닌 행 약 38000), `AreaOffsetAngle`(약 8000),
  `AreaOrigin`(2804), `AreaOverlapFirst`(5289)를 인테이크가 읽지 않는다. 적룡필살은 이 값이
  전부 기본값이라 이번 증상과 무관하다. 판정 방향이 어긋나는 다른 스킬을 다룰 때 착수한다.
- **`areaType` 해석 변경 없음.** `.md/JS/2026-07-31_LOSTARK_LANCEMASTER_SKILLTIMING_V2_EXTRACT_RESULT.md`
  5절에 "1=박스, 2=부채꼴, 3=원"으로 적힌 대목이 있으나 실측과 어긋난다. 추출된 874개 히트 행
  기준으로 area=1은 289개 중 282개가 `aa=0`이고, area=2는 `aa`가 60~650으로 35개가 360을
  넘으며, area=3은 40~360을 벗어나지 않는다. 각도로 읽을 수 없는 area=2가 폭(cm)이고 현재
  코드가 맞다. 그 문서를 근거로 매핑을 뒤집지 않는다.

## 2. 파일 목록

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 수정 | C:/Users/95jus/Desktop/TeamProject/LostArk/Tools/CharacterAnimationIntake/fill_animevents_hit_shapes.py | 한 타격 시각의 시전자 도형을 전부 기록 |
| 수정 | C:/Users/95jus/Desktop/TeamProject/LostArk/Tools/CharacterAnimationIntake/fill_projectiles.py | 기본 변형 소유 투사체 채택과 클립 재배치 |
| 수정 | C:/Users/95jus/Desktop/TeamProject/LostArk/Tools/CharacterAnimationIntake/build_hitshapes.py | v4 재생성 경로 개방 |
| 재생성 | C:/Users/95jus/Desktop/TeamProject/LostArk/Data/Animation/Authored/Asset/Asset.animevents | G01 출력, 5개 에셋 변경 |
| 재생성 | C:/Users/95jus/Desktop/TeamProject/LostArk/Data/Animation/Authored/LanceMaster/LanceMaster.projectiles.json | G02 출력, 투사체 0에서 4 |
| 재생성 | C:/Users/95jus/Desktop/TeamProject/LostArk/Data/Animation/HitShapes/Asset.hitshapes.json | G03 출력, 5개 에셋 변경 |
| 재생성 | C:/Users/95jus/Desktop/TeamProject/LostArk/Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap | Publish-GameplayBalance 출력 |

새 C++ 파일이 없으므로 `.vcxproj`와 `.vcxproj.filters` 등록 변경은 없다. 세 스크립트는 모두
기존 ASCII 인코딩과 LF 줄바꿈을 유지한다.

## 3. G01 - 한 타격 시각의 시전자 도형 전부 기록

### 3.1 파일 역할

`Data/Animation/Reference/` 아래의 `.animnotify`, `.skilltiming`, `.clipseq`, `.projectiles`를
읽어 `Data/Animation/Authored/` 아래 `.animevents`의 `HIT` 행을 다시 쓴다. 경로는 두 가지다.

```text
notify 경로    클립에 kind=HIT 노티파이가 있으면 그 t/d를 시각으로 삼고
               .skilltiming 행의 도형을 순번으로 붙인다  (이번 G에서 건드리지 않음)
합성 경로      노티파이가 없으면 PlayerSkills.json의 hitTimeMs 위치를 찾아
               .skilltiming 도형을 찍는다                (이번 G의 대상)
```

적룡필살의 세 클립 `flm_sk_dragoncleave_01/_02/_03`은 `.animnotify`에 `kind=HIT` 행이 하나도
없다. 그래서 합성 경로를 탄다.

### 3.2 변경되는 선언

| 대상 | 작업 | 내용 |
|---|---|---|
| `reference_shape` | 이름과 반환형 교체 | `reference_shapes`로 바꾸고 도형 하나 대신 목록을 반환 |
| `synthesize_timed_rows` | 함수 내부 교체 | 호출부 변수명과 빈 목록 판정, 도형마다 행을 찍는 루프 |

`reference_shapes`의 반환 계약은 `(도형 목록, 실패 사유)`다. 목록이 비면 사유가 채워지고,
차면 사유는 `None`이다. 실패 시 해당 스킬만 건너뛰고 나머지 스킬의 행은 보존한다.

### 3.3 함수 한 줄 책임과 흐름

`reference_shapes`는 한 `.skilltiming` 행에서 시전자가 직접 적용하는 도형 전부를 고른다.

```text
호출자        synthesize_timed_rows
→ 입력 검증   area>0, key가 CASTER_HIT_KEYS(1,2), 투사체가 적용하는 도형이 아닐 것
→ 읽는 상태   row['hits']와 호출자가 넘긴 projectile_shapes
→ 분기        timed 행들의 t가 둘 이상이면 다단 타임라인이므로 목록 없이 사유 반환
→ 선택        timed 우선, 그중 g==0인 기본 그룹 우선
→ 성공 출력   선택된 도형 목록 전체
→ 실패 정리   빈 목록과 사유. 호출자가 그 스킬만 건너뛴다
```

기존과 달라지는 지점은 마지막 한 줄뿐이다. `(base or pool)[0]` 대신 `list(base or pool)`을
돌려준다. 선택 규칙 자체는 바꾸지 않으므로 도형이 원래 하나뿐인 스킬의 출력은 그대로다.

`synthesize_timed_rows`는 클립을 한 번 찾고 그 안에서 도형 수만큼 `HIT` 행을 찍는다.
`locate_clip` 호출을 루프 밖에 두어 같은 시각의 도형들이 같은 클립과 같은 `startms`를 공유하게
한다. 행 내용이 완전히 같으면 추가하지 않는 기존 중복 제거는 유지한다.

### 3.4 실측 효과

`--check`로 측정한 `.animevents` 행 수 변화다.

```text
LanceMaster      3136 -> 3140   34650(1->2), 34630(1->4)
Artist           1047 -> 1049   31200(1->3), 31470(1->2)
DimensionMaster  1542 -> 1545   2050160(1->3), 2050230(1->2)
GuardianKnight   2249 -> 2250   49330(1->2)
GunSlinger       2326 -> 2329   38050(1->2), 38140(1->2)
Warlord          1725 -> 1725   변화 없음
Slayer           2442 -> 2442   변화 없음
```

### 3.5 전체 코드

**파일:** `C:/Users/95jus/Desktop/TeamProject/LostArk/Tools/CharacterAnimationIntake/fill_animevents_hit_shapes.py`

**변경 종류:** 전체 교체

```python
# -*- coding: utf-8 -*-
"""usage:
  <blender-python> fill_animevents_hit_shapes.py <Asset> [<Asset> ...] [--check]
"""
import io, json, os, re, struct, sys

from build_hitshapes import read_clip_ticks, TICK_RATE

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
REF = os.path.join(REPO, 'Data', 'Animation', 'Reference')
AUTH = os.path.join(REPO, 'Data', 'Animation', 'Authored')


def f32(x):
    return struct.unpack('f', struct.pack('f', x))[0]


def to_ms(seconds):
    return int(f32(f32(f32(seconds) * f32(1000.0)) + f32(0.5)))


def read_lines(path):
    with io.open(path, 'rb') as f:
        data = f.read()
    return data.decode('latin-1').split('\n')


def parse_pairs(text):
    out = {}
    for m in re.finditer(r'(\w+)=("([^"]*)"|\S+)', text):
        out[m.group(1)] = m.group(3) if m.group(3) is not None else m.group(2)
    return out


def load_notify(asset):
    clips = {}
    order = []
    cur = None
    for line in read_lines(os.path.join(REF, asset, asset + '.animnotify'))[1:]:
        m = re.match(r'^"([^"]+)"', line)
        if m:
            cur = m.group(1)
            clips.setdefault(cur, [])
            order.append(cur)
            continue
        if cur is None or not line.startswith('  n '):
            continue
        p = parse_pairs(line[4:])
        if p.get('kind') != 'HIT':
            continue
        clips[cur].append((float(p['t']), float(p['d']), p.get('label', '')))
    return clips, order


def load_clipmap(asset):
    out = {}
    for line in read_lines(os.path.join(REF, asset, asset + '.clipmap'))[1:]:
        m = re.match(r'^"([^"]+)"(.*)$', line)
        if m:
            out[m.group(1)] = int(parse_pairs(m.group(2)).get('skill', '0'))
    return out


def load_clipseq(asset):
    chains = []
    for line in read_lines(os.path.join(REF, asset, asset + '.clipseq'))[1:]:
        m = re.match(r'^(\d+) "[^"]*"(.*)$', line)
        if not m:
            continue
        clips = [c for c in parse_pairs(m.group(2)).get('clips', '').split(',') if c]
        if clips:
            chains.append(clips)
    return chains


SHAPE_KEYS = ('rep', 'repms', 'fz', 'fzin', 'fzout', 'push', 'pushr',
              'area', 'ar', 'aa', 'ah', 'ax', 'arem', 'maxt')


def zero_shape():
    s = {k: 0 for k in SHAPE_KEYS}
    s['rep'] = 1
    return s


def load_skilltiming(asset):
    rows = []
    for line in read_lines(os.path.join(REF, asset, asset + '.skilltiming'))[1:]:
        m = re.match(r'^(\d+) "[^"]*"(.*)$', line)
        if m:
            p = parse_pairs(m.group(2))
            hits = []
            for span in p.get('hits', '').split(','):
                if '-' in span:
                    hits.append(zero_shape())
            rows.append({'id': int(m.group(1)), 'base': int(p.get('base', '0')),
                         'hits': hits, 'detail': False})
            continue
        m = re.match(r'^  (hit|shape) (.*)$', line)
        if m and rows:
            row = rows[-1]
            if not row['detail']:
                row['detail'] = True
                row['hits'] = []
            p = parse_pairs(m.group(2))
            s = zero_shape()
            for k in SHAPE_KEYS:
                if k in p:
                    s[k] = int(p[k])
            if s['rep'] < 1:
                s['rep'] = 1
            s['timed'] = int(p.get('timed', '0'))
            s['t'] = int(p.get('t', '0'))
            s['g'] = int(p.get('g', '0'))
            s['w'] = int(p.get('w', '0'))
            s['key'] = int(p.get('key', '0'))
            row['hits'].append(s)
    return rows


def find_reference_row(rows, skill_id):
    variant = None
    for row in rows:
        if not row['hits']:
            continue
        if row['id'] == skill_id:
            return row
        if variant is None and row['base'] == skill_id:
            variant = row
    return variant


def distinct_hits(hits):
    seen = []
    n = 0
    for h in hits:
        if h in seen:
            continue
        seen.append(h)
        n += 1
    return n


def preceding_chain_hits(chains, notify, clip):
    for chain in chains:
        count = 0
        for c in chain:
            if c == clip:
                return count
            count += distinct_hits(notify.get(c, []))
    return 0


def hit_row(clip, start, end, s):
    return ('"%s" HIT startms=%d endms=%d rep=%d repms=%d fz=%d fzin=%d fzout=%d '
            'push=%d pushr=%d area=%d ar=%d aa=%d ah=%d ax=%d arem=%d maxt=%d src=orig'
            % (clip, start, end, s['rep'], s['repms'], s['fz'], s['fzin'], s['fzout'],
               s['push'], s['pushr'], s['area'], s['ar'], s['aa'], s['ah'], s['ax'],
               s['arem'], s['maxt']))


def load_json(*parts):
    return json.load(io.open(os.path.join(REPO, *parts), encoding='utf-8'))


CASTER_HIT_KEYS = (1, 2)
SHAPE_IDENTITY = ('area', 'ar', 'aa', 'ah', 'ax', 'arem', 'rep', 'repms', 'push', 'pushr')


def shape_identity(h):
    return tuple(max(1, int(h.get(k, 0))) if k == 'rep' else int(h.get(k, 0)) for k in SHAPE_IDENTITY)


def load_projectile_shapes(asset):
    """Shapes a spawned object (missile, fixed area...) applies itself, per skill,
    from the reference .projectiles; the caster must not stamp those again."""
    out = {}
    path = os.path.join(REF, asset, asset + '.projectiles')
    if not os.path.exists(path):
        return out
    # only the tripod-free chain (lowest clipseq group) spawns for the product
    base_seq = {}
    for line in read_lines(os.path.join(REF, asset, asset + '.clipseq'))[1:]:
        m = re.match(r'^(\d+) "[^"]*" seq=(\d+)', line)
        if m:
            skill, seq = int(m.group(1)), int(m.group(2))
            base_seq[skill] = min(base_seq.get(skill, seq), seq)
    skill = None
    for line in read_lines(path)[1:]:
        m = re.match(r'^(\d+) "[^"]*" pk=\d+ kind=\w+ seq=(-?\d+)', line)
        if m:
            skill = int(m.group(1))
            if int(m.group(2)) != base_seq.get(skill, int(m.group(2))):
                skill = None
            continue
        m = re.match(r'^  e (.*)$', line.rstrip('\r'))
        if m and skill is not None:
            p = parse_pairs(m.group(1))
            if int(p.get('area', '0')) > 0 and int(p.get('dmg', '0')) > 0:
                out.setdefault(skill, set()).add(shape_identity(p))
    return out


def reference_shapes(row, projectile_shapes):
    """Every caster-keyed shape the skill applies at that one moment. The source
    stacks boxes of different reach on a single hit time - a lunge carries both a
    short box around the caster and a long one along the travel - so returning
    only the first shape drops the reach that covers the movement."""
    hits = [h for h in row['hits'] if h['area'] > 0 and h['key'] in CASTER_HIT_KEYS and
            shape_identity(h) not in projectile_shapes]
    if not hits:
        return [], 'no caster-keyed shaped skilltiming row' + (
            ' (projectile applies the shape)' if projectile_shapes else '')
    timed = [h for h in hits if h['timed']]
    if len(set(h['t'] for h in timed)) > 1:
        return [], 'timed rows form a %d-step timeline' % len(set(h['t'] for h in timed))
    pool = timed or hits
    base = [h for h in pool if h['g'] == 0]
    return list(base or pool), None


def locate_clip(entries, clip_ticks, time_ms, label):
    elapsed = 0.0
    last = None
    for entry in entries:
        name = entry if isinstance(entry, str) else entry['clip']
        play_ms = 0 if isinstance(entry, str) else int(entry.get('playMs', 0))
        rate = 1.0 if isinstance(entry, str) else float(entry.get('playRate', 1.0))
        if name not in clip_ticks:
            raise SystemExit('%s: clip %s is not in the body model' % (label, name))
        source_ms = clip_ticks[name] / TICK_RATE * 1000.0
        if play_ms:
            source_ms = min(source_ms, float(play_ms))
        duration = source_ms / rate
        if elapsed <= time_ms < elapsed + duration:
            return name, int(round((time_ms - elapsed) * rate))
        last = (name, int(round(source_ms)))
        elapsed += duration
    print('%s: hitTimeMs %d is past the %d ms chain, clamped to the end of %s' % (
        label, time_ms, int(round(elapsed)), last[0]))
    return last


def synthesize_timed_rows(asset, notify, timing):
    catalog = load_json('Data', 'Actors', 'CharacterCatalog.json')
    entry = next(c for c in catalog['characters'] if c['assetId'] == asset)
    clip_ticks = read_clip_ticks(os.path.join(REPO, 'Client', 'Bin', 'Resources', *entry['bodyModel'].split('/')))
    bindings = load_json('Data', 'Animation', 'Authored', asset, asset + '.skillbindings.json')
    balance = load_json('Data', 'Balance', 'PlayerSkills.json')
    skills = {int(s['skillId']): s for s in balance['skills'] if s['characterClass'] == bindings['characterClass']}
    projectile_shapes = load_projectile_shapes(asset)
    generated = {}
    stamped = 0
    for binding in sorted(bindings['bindings'], key=lambda b: int(b['skillId'])):
        skill_id = int(binding['skillId'])
        skill = skills.get(skill_id)
        if skill is None or not skill.get('serverDamageProfileId'):
            continue
        entries = binding['clips']
        stages = [list(e) for e in entries] if entries and isinstance(entries[0], list) else [list(entries)]
        names = [e if isinstance(e, str) else e['clip'] for stage in stages for e in stage]
        if any(notify.get(n) for n in names):
            continue
        ref = find_reference_row(timing, skill_id)
        shapes, reason = (reference_shapes(ref, projectile_shapes.get(skill_id, set()))
                          if ref else ([], 'no skilltiming row'))
        if not shapes:
            print('%s %d: no HIT notify, left as range circle: %s' % (asset, skill_id, reason))
            continue
        if skill['skillKind'] in ('COMBO', 'HOLD', 'COUNTER'):
            targets = [(stages[i], int(st['hitTimeMs']), '%s %d stage %d' % (asset, skill_id, i))
                       for i, st in enumerate(skill.get('comboStages') or []) if i < len(stages) and int(st['hitTimeMs']) > 0]
        else:
            targets = [(stages[0], int(skill['hitTimeMs']), '%s %d' % (asset, skill_id))]
        for group, hit_ms, label in targets:
            clip, start = locate_clip(group, clip_ticks, hit_ms, label)
            rows = generated.setdefault(clip, [])
            for shape in shapes:
                row = hit_row(clip, start, start + shape['w'], shape)
                if row not in rows:
                    rows.append(row)
                    stamped += 1
    return generated, stamped


def build_rows(asset):
    notify, order = load_notify(asset)
    clipmap = load_clipmap(asset)
    chains = load_clipseq(asset)
    timing = load_skilltiming(asset)
    generated = {}
    shaped = 0
    for clip in order:
        rows = notify.get(clip, [])
        if not rows:
            continue
        ref = find_reference_row(timing, clipmap.get(clip, 0)) if clip in clipmap else None
        ordinal = preceding_chain_hits(chains, notify, clip) if ref else 0
        added = []
        out = []
        for t, d, label in rows:
            start = to_ms(t)
            end = start + to_ms(d)
            key = (start, end, label)
            if key in added:
                continue
            added.append(key)
            s = zero_shape()
            if ref:
                src = ref['hits'][min(ordinal, len(ref['hits']) - 1)]
                ordinal += 1
                if src['area'] > 0:
                    s = dict(src)
                    shaped += 1
            out.append(hit_row(clip, start, end, s))
        generated[clip] = out
    synthesized, stamped = synthesize_timed_rows(asset, notify, timing)
    for clip, rows in synthesized.items():
        generated.setdefault(clip, []).extend(rows)
    return generated, shaped, stamped


def rewrite(asset, generated, check):
    path = os.path.join(AUTH, asset, asset + '.animevents')
    lines = read_lines(path)
    eol = '\r\n' if lines[0].endswith('\r') else '\n'
    lines = [l.rstrip('\r') for l in lines]
    if lines and lines[-1] == '':
        lines.pop()
    header = lines[0]
    m = re.match(r'^(LOSTARK_ANIM_EVENTS \d+ "[^"]+" )(\d+)$', header)
    if not m:
        raise SystemExit('%s: unexpected header %r' % (asset, header))
    body = lines[1:]
    orig_pat = re.compile(r'^"([^"]+)" HIT .* src=orig$')
    first_index = {}
    kept = []
    for i, line in enumerate(body):
        mm = orig_pat.match(line)
        if mm and mm.group(1) in generated:
            first_index.setdefault(mm.group(1), len(kept))
            kept.append(None)
        elif mm:
            # every src=orig HIT row comes from this script or the tool's import
            # of the same notifies; a clip that no longer generates any is stale
            continue
        else:
            kept.append(line)
    out = []
    emitted = set()
    for i, line in enumerate(kept):
        if line is None:
            for clip, idx in first_index.items():
                if idx == i and clip not in emitted:
                    out.extend(generated[clip])
                    emitted.add(clip)
            continue
        out.append(line)
    for clip in generated:
        if clip not in emitted:
            out.extend(generated[clip])
            emitted.add(clip)
    new_header = '%s%d' % (m.group(1), len(out))
    text = eol.join([new_header] + out) + eol
    old_text = eol.join([header] + body) + eol
    changed = text != old_text
    print('%s: rows %s -> %d, HIT rows %d, %s' % (
        asset, m.group(2), len(out), sum(len(v) for v in generated.values()),
        'changed' if changed else 'unchanged'))
    if changed and not check:
        with io.open(path, 'wb') as f:
            f.write(text.encode('latin-1'))
    return changed


def main(argv):
    check = '--check' in argv
    assets = [a for a in argv if not a.startswith('--')]
    if not assets:
        raise SystemExit(__doc__)
    for asset in assets:
        generated, shaped, stamped = build_rows(asset)
        print('%s: %d clips with HIT rows, %d notify rows shaped from skilltiming, %d rows stamped at hitTimeMs' % (
            asset, len(generated), shaped, stamped))
        rewrite(asset, generated, check)


if __name__ == '__main__':
    main(sys.argv[1:])
```

## 4. G02 - 기본 변형이 소유한 투사체 채택과 클립 재배치

### 4.1 파일 역할

`Data/Animation/Reference/` 아래 `.projectiles`의 스폰 객체를 `Data/Animation/Authored/` 아래
`.projectiles.json`으로 승격한다. 현재 창술사 출력은 `"projectiles": []`이고, 원본에는 로스터
스킬 일곱 개에 투사체가 달려 있다.

### 4.2 변경되는 선언

| 대상 | 작업 | 내용 |
|---|---|---|
| 모듈 docstring | 교체 | 새 채택 규칙과 재배치 규칙을 기술 |
| `from build_hitshapes import ...` | 추가 | `read_clip_ticks`, `TICK_RATE`. 클립 길이가 있어야 시각을 옮길 수 있다 |
| `load_seq_chains` | 새 함수 | `load_reference` 바로 위. (스킬, clipseq 그룹)에서 클립 목록 |
| `clip_source_ms` | 새 함수 | 클립 틱을 ms로 환산 |
| `action_time_ms` | 새 함수 | 클립 로컬 스폰 시각을 액션 로컬 시각으로 환산 |
| `locate_in_chain` | 새 함수 | 액션 로컬 시각을 덮는 바운드 클립과 그 안의 오프셋 |
| `build` | 블록 교체 | `candidates` 수집을 리스트 컴프리헨션에서 루프로 |

새 함수 넷은 모두 `load_reference` 정의 바로 위, `load_base_seq` 정의 바로 아래에 놓는다.

### 4.3 채택 판별식

```text
기존   seq == clipseq 최저 그룹
변경   seq == clipseq 최저 그룹  또는  효과 PK // 10 == 스킬 ID
```

PK 규칙을 단독으로 쓰면 안 된다. 실측하면 워로드 17110은 seq 규칙이 3건을 통과시키는데 PK
규칙은 0건이고, 차원술사 2050160은 20건 대 0건이다. 공용 효과가 스킬 ID와 무관한 PK를 갖기
때문이다. 그래서 합집합으로만 쓴다.

합집합의 실측 증분은 창술사 두 스킬뿐이다.

```text
LanceMaster 34140   +2  clips=flm_sk_chestdestruction_01, _04   둘 다 바운드 체인 안
LanceMaster 34650   +2  clip=flm_sk_dragoncleave_02             바운드 체인 밖, 재배치 필요
Warlord / Artist / DimensionMaster   추가 0건
```

추가된 행이 기존 채택 행과 `(clip, t, kind)`가 겹치는 경우도 0건이라 중복 스폰이 생기지 않는다.

### 4.4 클립 재배치

바운드 체인에 없는 클립에 달린 기본 변형 객체만 옮긴다.

```text
원본 seq=4 체인   flm_sk_dragoncleave_02(667ms) + flm_sk_dragoncleave_03(1700ms)
바운드 체인       flm_sk_dragoncleave_01(733ms) + flm_sk_dragoncleave_03(1700ms)

스폰 270ms (클립 로컬)
→ action_time_ms: _02가 원본 체인의 첫 클립이므로 액션 로컬도 270ms
→ locate_in_chain: 바운드 체인에서 270ms는 _01 안의 270ms
→ 결과 clip=flm_sk_dragoncleave_01, t=0.270
```

사용자 확인 사항이 여기 반영돼 있다. 우리 프로젝트의 적룡필살 연출은 돌진뿐이고 창을 던지는
모션이 없으므로, 바인딩을 `_02` 체인으로 바꾸지 않고 현재 `_01` 체인의 같은 액션 시각에
미사일을 붙인다.

재배치가 불가능한 경우는 그 객체만 제외하고 사유를 출력한다.

```text
스테이지가 둘 이상인 스킬        액션 로컬 시각이 스테이지 경계에서 모호하므로 제외
원본 clipseq 그룹을 못 찾음      제외
바운드 체인이 그 시각을 안 덮음  제외
```

### 4.5 함수 한 줄 책임과 흐름

`action_time_ms`는 스폰 시각의 기준을 클립에서 액션으로 옮긴다. 원본 체인을 앞에서부터 훑어
대상 클립 이전 클립들의 길이를 더하고 거기에 클립 로컬 시각을 더한다. 대상 클립을 만나기 전에
모델에 없는 클립이 나오면 `None`을 돌려 호출자가 그 객체를 제외하게 한다.

`locate_in_chain`은 반대 방향이다. 바운드 체인의 각 항목에서 `playMs` 상한과 `playRate`를
적용한 실제 재생 길이를 누적하고, 주어진 액션 로컬 시각을 포함하는 구간을 찾으면 그 클립
이름과 구간 내 오프셋을 돌려준다. 오프셋에는 `playRate`를 곱해 클립 원본 시간축으로 되돌린다.
어느 구간도 덮지 않으면 `None`이다. 체인 끝을 넘긴 시각을 마지막 클립으로 당겨 붙이지 않는데,
이는 `locate_clip`이 타격 시각에 쓰는 클램프와 달리 스폰은 조용히 앞당기면 안 되기 때문이다.

`build`의 수집 루프는 객체마다 다음 순서로 판정한다.

```text
스킬·종류·layout 확인
→ base_owned = (pk // 10 == skill_id)
→ base_owned 아니고 seq도 최저 그룹이 아니면 제외
→ 클립이 바운드 체인 안이면 그대로 채택
→ base_owned 아니면 제외
→ 스테이지가 둘 이상이면 사유 출력 후 제외
→ action_time_ms 다음 locate_in_chain 으로 재배치, 실패하면 사유 출력 후 제외
→ clip과 t를 옮긴 사본을 채택하고 이동 내역 출력
```

원본 `row`를 직접 수정하지 않고 `dict(row)` 사본을 만든다. `reference`는 스킬 루프 밖에서 한 번
읽은 공유 목록이라, 제자리에서 고치면 다른 스킬의 판정에 영향을 준다.

### 4.6 전체 코드

**파일:** `C:/Users/95jus/Desktop/TeamProject/LostArk/Tools/CharacterAnimationIntake/fill_projectiles.py`

**변경 종류:** 전체 교체

```python
# -*- coding: utf-8 -*-
"""usage:
  <blender-python> fill_projectiles.py <Asset> [<Asset> ...] [--check]

Promotes the reference <Asset>.projectiles (objects a skill spawns: missiles,
fixed areas, grenades, traces, with the SkillEffect hits the object itself
applies) into Data/Animation/Authored/<Asset>/<Asset>.projectiles.json for the
damage skills of the class. A spawn counts when it belongs to the skill's lowest
clipseq group (the tripod-free chain) or when its effect PK names the base variant
(pk // 10 == skillId), which is how an awakening skill owns its objects outright
while its clipseq groups only describe alternate presentations. A base-variant
object authored on a clip the roster does not play keeps its action-local time and
moves to the bound clip that covers it. The lowest PK of a same-time spawn is the
base object, and an object without a damaging shaped hit (dmg > 0 against enemies,
area > 0) is skipped.
"""
import io, json, os, re, sys

from build_hitshapes import read_clip_ticks, TICK_RATE

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
REF = os.path.join(REPO, 'Data', 'Animation', 'Reference')
AUTH = os.path.join(REPO, 'Data', 'Animation', 'Authored')
UNITS_TO_METERS = 0.01

KINDS = ('MISSILE', 'FIXAREA', 'GRENADE', 'TRACE')
ORIGINS = {0: 'CASTER', 1: 'AIM'}


def read_lines(path):
    with io.open(path, 'rb') as f:
        return f.read().decode('utf-8').split('\n')


def parse_pairs(text):
    out = {}
    for m in re.finditer(r'(\w+)=("([^"]*)"|\S+)', text):
        out[m.group(1)] = m.group(3) if m.group(3) is not None else m.group(2)
    return out


def load_base_seq(asset):
    """The tripod-free chain of a skill is its lowest clipseq group; a higher
    group re-uses the same clips with the tripod's own spawns added."""
    base = {}
    for line in read_lines(os.path.join(REF, asset, asset + '.clipseq'))[1:]:
        m = re.match(r'^(\d+) "[^"]*" seq=(\d+)', line)
        if m:
            skill, seq = int(m.group(1)), int(m.group(2))
            base[skill] = min(base.get(skill, seq), seq)
    return base


def load_seq_chains(asset):
    """Clips of each (skill, clipseq group). A spawn time is local to its own clip,
    so the group's clip order is what turns it into an action-local time."""
    chains = {}
    for line in read_lines(os.path.join(REF, asset, asset + '.clipseq'))[1:]:
        m = re.match(r'^(\d+) "[^"]*" seq=(\d+)(.*)$', line.rstrip('\r'))
        if m:
            clips = [c for c in parse_pairs(m.group(3)).get('clips', '').split(',') if c]
            if clips:
                chains[(int(m.group(1)), int(m.group(2)))] = clips
    return chains


def clip_source_ms(clip_ticks, name):
    return clip_ticks[name] / TICK_RATE * 1000.0


def action_time_ms(chain, clip_ticks, clip, local_ms):
    """Spawn time measured from the start of the action instead of its own clip."""
    elapsed = 0.0
    for name in chain:
        if name == clip:
            return elapsed + local_ms
        if name not in clip_ticks:
            return None
        elapsed += clip_source_ms(clip_ticks, name)
    return None


def locate_in_chain(entries, clip_ticks, time_ms):
    """The bound clip covering an action-local time, and the offset inside it."""
    elapsed = 0.0
    for entry in entries:
        name = entry if isinstance(entry, str) else entry['clip']
        play_ms = 0 if isinstance(entry, str) else int(entry.get('playMs', 0))
        rate = 1.0 if isinstance(entry, str) else float(entry.get('playRate', 1.0))
        if name not in clip_ticks:
            return None
        source_ms = clip_source_ms(clip_ticks, name)
        if play_ms:
            source_ms = min(source_ms, float(play_ms))
        duration = source_ms / rate
        if elapsed <= time_ms < elapsed + duration:
            return name, (time_ms - elapsed) * rate
        elapsed += duration
    return None


def load_reference(asset):
    rows = []
    for line in read_lines(os.path.join(REF, asset, asset + '.projectiles'))[1:]:
        line = line.rstrip('\r')
        m = re.match(r'^(\d+) "[^"]*"(.*)$', line)
        if m:
            p = parse_pairs(m.group(2))
            rows.append({'skill': int(m.group(1)), 'pk': int(p['pk']), 'kind': p['kind'],
                         'seq': int(p['seq']), 'clip': p['clip'], 't': float(p['t']),
                         'origin': int(p.get('origin', '0')), 'ox': int(p.get('ox', '0')),
                         'oy': int(p.get('oy', '0')),
                         'radius': int(p['radius']), 'mindist': int(p['mindist']),
                         'maxdist': int(p['maxdist']), 'life': float(p['life']),
                         'speed': int(p['speed']), 'layout': p['layout'], 'hits': []})
            continue
        m = re.match(r'^  e (.*)$', line)
        if m and rows:
            rows[-1]['hits'].append(parse_pairs(m.group(1)))
    return rows


def hit_of(e):
    if 'missing' in e or int(e.get('area', '0')) <= 0 or int(e.get('dmg', '0')) <= 0:
        return None
    area = int(e['area'])
    aa = int(e.get('aa', '0'))
    contact = e.get('contact') == '1'
    rep = max(1, int(e.get('rep', '0')))
    push = int(e.get('push', '0'))
    return {
        'sourcePk': int(e['pk']),
        'trigger': 'CONTACT' if contact else 'TIMED',
        'atMs': 0 if contact else int(round(float(e['at']) * 1000.0)),
        'count': rep if contact else max(1, int(e['count'])),
        'everyMs': int(e.get('repms', '0')) if contact else int(round(float(e['every']) * 1000.0)),
        'areaType': area,
        'range': round(int(e['ar']) * UNITS_TO_METERS, 2),
        'angle': min(max(aa, 0), 360) if area == 3 else 0,
        'width': round(aa * UNITS_TO_METERS, 2) if area == 2 else 0.0,
        'height': round(int(e.get('ah', '0')) * UNITS_TO_METERS, 2),
        'offset': round(int(e.get('ax', '0')) * UNITS_TO_METERS, 2),
        'inner': round(int(e.get('arem', '0')) * UNITS_TO_METERS, 2),
        'maxTargets': int(e.get('maxt', '0')),
        'pushMs': max(0, push),
        'pushRange': round(int(e.get('pushr', '0')) * UNITS_TO_METERS, 2) if push > 0 else 0.0,
    }


def build(asset):
    bindings = json.load(io.open(os.path.join(AUTH, asset, asset + '.skillbindings.json'), encoding='utf-8'))
    balance = json.load(io.open(os.path.join(REPO, 'Data', 'Balance', 'PlayerSkills.json'), encoding='utf-8'))
    skills = {int(s['skillId']): s for s in balance['skills'] if s['characterClass'] == bindings['characterClass']}
    reference = load_reference(asset)
    base_seq = load_base_seq(asset)
    seq_chains = load_seq_chains(asset)
    catalog = json.load(io.open(os.path.join(REPO, 'Data', 'Actors', 'CharacterCatalog.json'), encoding='utf-8'))
    body = next(c for c in catalog['characters'] if c['assetId'] == asset)['bodyModel']
    clip_ticks = read_clip_ticks(os.path.join(REPO, 'Client', 'Bin', 'Resources', *body.split('/')))
    out = []
    skipped = 0
    for binding in sorted(bindings['bindings'], key=lambda b: int(b['skillId'])):
        skill_id = int(binding['skillId'])
        skill = skills.get(skill_id)
        if skill is None or not skill.get('serverDamageProfileId'):
            continue
        entries = binding['clips']
        stages = [list(e) for e in entries] if entries and isinstance(entries[0], list) else [list(entries)]
        chain = {e if isinstance(e, str) else e['clip'] for stage in stages for e in stage}
        candidates = []
        for row in reference:
            if row['skill'] != skill_id or row['kind'] not in KINDS or row['layout'] == 'none':
                continue
            base_owned = row['pk'] // 10 == skill_id
            if not base_owned and row['seq'] != base_seq.get(skill_id, row['seq']):
                continue
            if row['clip'] in chain:
                candidates.append(row)
                continue
            if not base_owned:
                continue
            if len(stages) != 1:
                print('%s %d: base object %d sits on unbound clip %s of a staged skill, left out' % (
                    asset, skill_id, row['pk'], row['clip']))
                skipped += 1
                continue
            source_chain = seq_chains.get((skill_id, row['seq']))
            absolute = (action_time_ms(source_chain, clip_ticks, row['clip'], row['t'] * 1000.0)
                        if source_chain else None)
            located = locate_in_chain(stages[0], clip_ticks, absolute) if absolute is not None else None
            if located is None:
                print('%s %d: base object %d on unbound clip %s has no place in the bound chain, left out' % (
                    asset, skill_id, row['pk'], row['clip']))
                skipped += 1
                continue
            moved = dict(row)
            moved['clip'], moved['t'] = located[0], located[1] / 1000.0
            print('%s %d: base object %d moved from %s %dms to %s %dms' % (
                asset, skill_id, row['pk'], row['clip'], int(round(row['t'] * 1000.0)),
                moved['clip'], int(round(located[1]))))
            candidates.append(moved)
        groups = {}
        for row in candidates:
            key = (row['clip'], round(row['t'], 3), row['kind'])
            best = groups.get(key)
            if best is None or row['pk'] < best['pk']:
                groups[key] = row
        for key in sorted(groups, key=lambda k: (k[0], k[1], k[2])):
            row = groups[key]
            hits = [h for h in (hit_of(e) for e in row['hits']) if h is not None]
            if not hits:
                skipped += 1
                continue
            # AreaOrigin 0 spawns on the caster (offset forward/right), 1 at the
            # aim point; the rarer anchor codes are not modelled and stay out.
            if row['origin'] not in ORIGINS:
                print('%s %d: projectile %d uses AreaOrigin %d, left out' % (
                    asset, skill_id, row['pk'], row['origin']))
                skipped += 1
                continue
            hits.sort(key=lambda h: (h['trigger'] != 'CONTACT', h['atMs'], h['sourcePk']))
            out.append({
                'skillId': skill_id,
                'sourcePk': row['pk'],
                'clip': row['clip'],
                'startMs': int(round(row['t'] * 1000.0)),
                'kind': row['kind'],
                'origin': ORIGINS[row['origin']],
                'offsetForward': round(row['ox'] * UNITS_TO_METERS, 2),
                'offsetRight': round(row['oy'] * UNITS_TO_METERS, 2),
                'speed': round(row['speed'] * UNITS_TO_METERS, 2),
                'minDistance': round(row['mindist'] * UNITS_TO_METERS, 2),
                'maxDistance': round(row['maxdist'] * UNITS_TO_METERS, 2),
                'lifeMs': int(round(row['life'] * 1000.0)),
                'radius': round(row['radius'] * UNITS_TO_METERS, 2),
                'hits': hits,
            })
    return {
        'schema': 'lostark.animation-projectiles',
        'formatVersion': 1,
        'animationAssetId': asset,
        'characterClass': bindings['characterClass'],
        'projectiles': out,
    }, skipped


def main(argv):
    check = '--check' in argv
    assets = [a for a in argv if not a.startswith('--')]
    if not assets:
        raise SystemExit(__doc__)
    for asset in assets:
        document, skipped = build(asset)
        text = json.dumps(document, indent=2, ensure_ascii=False) + '\n'
        path = os.path.join(AUTH, asset, asset + '.projectiles.json')
        old = io.open(path, encoding='utf-8').read() if os.path.exists(path) else None
        print('%s: %d projectiles for %d skills (%d visual-only skipped), %s' % (
            asset, len(document['projectiles']), len({p['skillId'] for p in document['projectiles']}),
            skipped, 'unchanged' if old == text else 'changed'))
        if old != text and not check:
            io.open(path, 'w', encoding='utf-8', newline='\n').write(text)


if __name__ == '__main__':
    main(sys.argv[1:])
```

## 5. G03 - v4 재생성 경로 개방

### 5.1 현재 막혀 있는 이유

`build_hitshapes.py`는 `formatVersion 3`을 쓰고, 디스크의 일곱 파일은 전부 v4다. `main`이 v4를
발견하면 "Action Workbench 소유"라며 중단한다. 그래서 G01과 G02를 고쳐도 출력에 반영할 수 없다.

v3과 v4의 차이는 `migrate_player_hit_results.py`가 소유한다. 이 스크립트는 두 가지를 한다.

```text
split_hits    도형 하나를 DAMAGE / COUNTER / STAGGER 세 채널로 복제하고
              skill<id>.stage<n>.caster.hit<n>.<kind>.collider 형태의 ID를 붙인다
fallback      도형이 하나도 없는 피해 스킬에 maximumRange 원형 판정 한 개를 만들고
              sourceBasis = EXISTING_SERVER_MAXIMUM_RANGE로 표시한다
```

실측하면 현재 v4 파일의 모든 기하 도형이 예외 없이 `DAMAGE+COUNTER+STAGGER` 세 벌을 갖는다.
ID 규칙도 결정적이다. 즉 v4는 v3에서 기계적으로 재생성할 수 있다.

`fallback`이 만든 스킬은 여덟 개다.

```text
DimensionMaster  2050010 기본 공격/LMB
GunSlinger       38120 유탄/E, 38180 나선의 추적자/S, 38260 피스키퍼/F, 38290 프리즌 불릿/T
Slayer           45820 레이지 슬래셔/ALT_V, 45000 슬레이어 평타/LMB
Warlord          17240 풀배럴 캐넌/T
```

`build_hitshapes`만 다시 돌리고 `migrate`를 건너뛰면 이 여덟 개가 사라져 publisher의 커버리지
검사가 깨진다. 그래서 거부를 없애는 대신 `migrate`를 파이프라인에 넣는다.

### 5.2 변경되는 선언

| 대상 | 작업 | 내용 |
|---|---|---|
| `from migrate_player_hit_results import migrate` | 추가 | `import io, json, os, re, struct, sys` 바로 아래 |
| `balance` 로드 | 추가 | `main`의 `for asset in assets:` 바로 위 |
| `document = build(asset)` | 교체 | `migrate(build(asset), balance['skills'])` |
| v4 거부 두 줄 | 삭제 | `if old and json.loads(old).get('formatVersion') == 4:`와 그 `raise SystemExit` |

`build`는 기하 도형을, `migrate`는 결과 채널과 maximumRange 보충을 소유한다. 둘을 이어 붙이면
재생성이 기존 v4 파일을 그대로 재현하면서 G01과 G02의 변경분만 반영한다.

`build` 함수 자체와 `formatVersion 3` 반환값은 그대로 둔다. `migrate`가 정확히 v3만 받기
때문이고, 두 소유권을 섞지 않는 편이 나중에 Action Workbench 쪽 규칙이 바뀔 때 추적하기 쉽다.

### 5.3 반복 횟수 클램프

`stage_hits`에 반복 클램프를 함께 넣는다. 원본이 저작한 반복이 우리가 바인딩한 액션보다 길 때
게시 전체가 막히는 것을 막는다.

건슬링어 38180 나선의 추적자가 이 경우다. 원본은 1000ms에 300ms 간격 3연타인데
`PlayerSkills.json`의 `actionDurationMs`가 1000이라 마지막 발동이 1600ms로 밖으로 나간다.
publisher는 `Hit shape repeat exceeds its action/stage duration: 38180`으로 throw한다.

```text
fire_ms   = min(클립 누적 시각, limit_ms)
repeat    = min(원본 rep, (limit_ms - fire_ms) // repeat_ms + 1)
repeat==1 이면 repeat_ms 는 0
```

38180은 이 규칙으로 3연타가 단타가 되지만 도형은 원본 1.7m 원을 유지한다. 3연타까지 살리려면
그 스킬의 `actionDurationMs`를 늘려야 하고, 이는 건슬링어 담당 판단이다. 스킬 하나의 꼬리를
잃는 쪽이 게시 전체가 막히는 것보다 낫다는 판단으로 클램프를 택했다.

### 5.4 재현성 증거

수정본으로 일곱 에셋을 실제 재생성한 결과다.

```text
LanceMaster      19 skills, 192 hits,  4 projectiles, changed
Artist           12 skills,  51 hits,  7 projectiles, changed
DimensionMaster  12 skills,  48 hits,  6 projectiles, changed
GuardianKnight   16 skills, 111 hits,  0 projectiles, changed
GunSlinger       12 skills,  48 hits,  0 projectiles, changed
Warlord          10 skills,  36 hits,  0 projectiles, unchanged
Slayer           11 skills,  99 hits,  0 projectiles, unchanged
```

워로드와 슬레이어가 `unchanged`라는 것이 핵심이다. G01이 이 두 에셋의 `.animevents`를 바꾸지
않았고, 재생성된 v4 문서가 디스크의 기존 파일과 바이트 단위로 같다. v4 승격이 손실 없이
재현된다는 뜻이다.

재생성 후 적룡필살은 다음과 같다.

```text
시전자 t=1240  박스 2.6m x 3.0m offset -1.5
시전자 t=1240  박스 3.5m x 3.0m offset -2.5
투사체 t=270   MISSILE speed=7.0 dist=16.0 life=1000ms
        접촉판정 CONTACT  박스 2.0m x 2.0m  넉백 110ms/0.5m
투사체 t=550   MISSILE speed=7.0 dist=16.0 life=1000ms
        접촉판정 CONTACT  박스 6.0m x 1.6m  넉백 110ms/0.5m
```

1.1절의 원본 표와 일치한다. 중복 효과 346505만 1.3절 사유로 빠져 있다.

### 5.5 전체 코드

**파일:** `C:/Users/95jus/Desktop/TeamProject/LostArk/Tools/CharacterAnimationIntake/build_hitshapes.py`

**변경 종류:** 전체 교체

```python
# -*- coding: utf-8 -*-
"""usage:
  <blender-python> build_hitshapes.py <Asset> [<Asset> ...] [--check]
"""
import io, json, os, re, struct, sys

from migrate_player_hit_results import migrate

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
RESOURCES = os.path.join(REPO, 'Client', 'Bin', 'Resources')
TICK_RATE = 30.0
UNITS_TO_METERS = 0.01
MAX_SUB_HITS = 64
AREA_CIRCLE = 1
AREA_BOX = 2
AREA_FAN = 3


def read_clip_ticks(wmodel_path):
    data = open(wmodel_path, 'rb').read()
    cb = 16
    _m, section_count, _a, _f = struct.unpack_from('<4sIII', data, cb)
    ticks = {}
    for i in range(section_count):
        o = cb + 32 + i * 64
        kind, _idx, off, _size = struct.unpack_from('<IIQQ', data, o)
        name = data[o + 24:o + 64].split(b'\x00')[0].decode('ascii', 'replace')
        if kind != 4:
            continue
        b = cb + off
        _mg, _channels, duration_ticks, _tps, _k, _e, _l = struct.unpack_from('<4sIffIIB', data, b + 16)
        ticks[name] = duration_ticks
    return ticks


def read_hit_rows(asset):
    path = os.path.join(REPO, 'Data', 'Animation', 'Authored', asset, asset + '.animevents')
    hits = {}
    for line in io.open(path, 'rb').read().decode('latin-1').split('\n'):
        m = re.match(r'^"([^"]+)" HIT (.*)$', line.rstrip('\r'))
        if not m:
            continue
        fields = dict(kv.split('=', 1) for kv in m.group(2).split() if '=' in kv)
        area = int(fields.get('area', '0'))
        if area <= 0:
            continue
        hits.setdefault(m.group(1), []).append({
            'startMs': int(fields['startms']),
            'rep': max(1, int(fields.get('rep', '1'))),
            'repMs': int(fields.get('repms', '0')),
            'area': area,
            'ar': int(fields.get('ar', '0')),
            'aa': int(fields.get('aa', '0')),
            'ah': int(fields.get('ah', '0')),
            'ax': int(fields.get('ax', '0')),
            'arem': int(fields.get('arem', '0')),
            'maxt': int(fields.get('maxt', '0')),
            'push': int(fields.get('push', '0')),
            'pushr': int(fields.get('pushr', '0')),
        })
    for rows in hits.values():
        rows.sort(key=lambda h: h['startMs'])
    return hits


def read_projectile_rows(asset):
    path = os.path.join(REPO, 'Data', 'Animation', 'Authored', asset, asset + '.projectiles.json')
    if not os.path.exists(path):
        return {}
    document = json.load(io.open(path, encoding='utf-8'))
    if document.get('schema') != 'lostark.animation-projectiles' or document.get('formatVersion') != 1:
        raise SystemExit('%s: unexpected projectiles document header' % asset)
    rows = {}
    for entry in document['projectiles']:
        rows.setdefault((int(entry['skillId']), entry['clip']), []).append(entry)
    return rows


MAX_PROJECTILES = 8


def stage_projectiles(skill_id, entries, clip_ticks, projectile_rows, limit_ms, label):
    """Every object the stage's clips spawn, in stage-local time; the object's own
    hits keep their spawn-relative schedule and are not rescaled by playRate."""
    out = []
    elapsed_ms = 0.0
    for entry in entries:
        name = entry if isinstance(entry, str) else entry['clip']
        play_ms = 0 if isinstance(entry, str) else int(entry.get('playMs', 0))
        rate = 1.0 if isinstance(entry, str) else float(entry.get('playRate', 1.0))
        source_ms = clip_ticks[name] / TICK_RATE * 1000.0
        if play_ms:
            source_ms = min(source_ms, float(play_ms))
        for spawn in projectile_rows.get((skill_id, name), []):
            spawn_ms = elapsed_ms + spawn['startMs'] / rate
            out.append({
                'timeMs': min(int(round(spawn_ms)), limit_ms),
                'kind': spawn['kind'],
                'origin': spawn['origin'],
                'offsetForward': spawn['offsetForward'],
                'offsetRight': spawn['offsetRight'],
                'speed': spawn['speed'],
                'minDistance': spawn['minDistance'],
                'maxDistance': spawn['maxDistance'],
                'lifeMs': spawn['lifeMs'],
                'radius': spawn['radius'],
                'hits': [{k: v for k, v in h.items() if k != 'sourcePk'} for h in spawn['hits']],
            })
        elapsed_ms += source_ms / rate
    out.sort(key=lambda p: p['timeMs'])
    if len(out) > MAX_PROJECTILES:
        raise SystemExit('%s: more than %d projectiles' % (label, MAX_PROJECTILES))
    return out


def stage_hits(entries, clip_ticks, clip_hits, limit_ms, label):
    out = []
    elapsed_ms = 0.0
    for entry in entries:
        name = entry if isinstance(entry, str) else entry['clip']
        play_ms = 0 if isinstance(entry, str) else int(entry.get('playMs', 0))
        rate = 1.0 if isinstance(entry, str) else float(entry.get('playRate', 1.0))
        if name not in clip_ticks:
            raise SystemExit('%s: clip %s is not in the body model' % (label, name))
        source_ms = clip_ticks[name] / TICK_RATE * 1000.0
        if play_ms:
            source_ms = min(source_ms, float(play_ms))
        for h in clip_hits.get(name, []):
            fire_ms = min(int(round(elapsed_ms + h['startMs'] / rate)), limit_ms)
            repeat_ms = int(round(h['repMs'] / rate))
            repeat = h['rep']
            # The source can author more repeats than the bound action is long.
            # Keep the ones that still fire inside it: dropping the overhang costs
            # the tail of one skill, rejecting the row costs the whole publish.
            if repeat > 1 and repeat_ms > 0:
                repeat = max(1, min(repeat, (limit_ms - fire_ms) // repeat_ms + 1))
            if repeat == 1:
                repeat_ms = 0
            # Official AreaType: 1 circle/ring, 2 forward box whose AreaAngle
            # is the width in cm, 3 fan whose AreaAngle is the sweep in degrees.
            out.append({
                'timeMs': fire_ms,
                'repeatCount': repeat,
                'repeatMs': repeat_ms,
                'areaType': h['area'],
                'range': round(h['ar'] * UNITS_TO_METERS, 2),
                'angle': min(max(h['aa'], 0), 360) if h['area'] == AREA_FAN else 0,
                'width': round(h['aa'] * UNITS_TO_METERS, 2) if h['area'] == AREA_BOX else 0.0,
                'height': round(h['ah'] * UNITS_TO_METERS, 2),
                'offset': round(h['ax'] * UNITS_TO_METERS, 2),
                'inner': round(h['arem'] * UNITS_TO_METERS, 2),
                'maxTargets': h['maxt'],
                # Official push: duration in ms (0 = no push) and signed range,
                # negative pulling the target toward the caster.
                'pushMs': max(0, h['push']),
                'pushRange': round(h['pushr'] * UNITS_TO_METERS, 2) if h['push'] > 0 else 0.0,
            })
        elapsed_ms += source_ms / rate
    out.sort(key=lambda h: h['timeMs'])
    if sum(h['repeatCount'] for h in out) > MAX_SUB_HITS:
        raise SystemExit('%s: more than %d sub-hits' % (label, MAX_SUB_HITS))
    return out


def build(asset):
    catalog = json.load(io.open(os.path.join(REPO, 'Data', 'Actors', 'CharacterCatalog.json'), encoding='utf-8'))
    entry = next(c for c in catalog['characters'] if c['assetId'] == asset)
    clip_ticks = read_clip_ticks(os.path.join(RESOURCES, *entry['bodyModel'].split('/')))
    bindings = json.load(io.open(os.path.join(REPO, 'Data', 'Animation', 'Authored', asset, asset + '.skillbindings.json'), encoding='utf-8'))
    balance = json.load(io.open(os.path.join(REPO, 'Data', 'Balance', 'PlayerSkills.json'), encoding='utf-8'))
    skills = {int(s['skillId']): s for s in balance['skills'] if s['characterClass'] == bindings['characterClass']}
    clip_hits = read_hit_rows(asset)
    projectile_rows = read_projectile_rows(asset)
    out = []
    for binding in sorted(bindings['bindings'], key=lambda b: int(b['skillId'])):
        skill_id = int(binding['skillId'])
        skill = skills.get(skill_id)
        if skill is None or not skill.get('serverDamageProfileId'):
            continue
        entries = binding['clips']
        stages = [list(e) for e in entries] if entries and isinstance(entries[0], list) else [list(entries)]
        combo_stages = list(skill.get('comboStages') or [])
        if skill['skillKind'] in ('COMBO', 'HOLD', 'COUNTER'):
            if len(stages) != len(combo_stages):
                raise SystemExit('%s %d: stage count mismatch' % (asset, skill_id))
            rows = []
            for index, group in enumerate(stages):
                label = '%s %d stage %d' % (asset, skill_id, index)
                limit_ms = int(combo_stages[index]['actionDurationMs'])
                hits = stage_hits(group, clip_ticks, clip_hits, limit_ms, label)
                projectiles = stage_projectiles(skill_id, group, clip_ticks, projectile_rows, limit_ms, label)
                if hits or projectiles:
                    row = {'stageIndex': index, 'hits': hits}
                    if projectiles:
                        row['projectiles'] = projectiles
                    rows.append(row)
            if rows:
                out.append({'skillId': skill_id, 'stages': rows})
            continue
        label = '%s %d' % (asset, skill_id)
        limit_ms = int(skill['actionDurationMs'])
        hits = stage_hits(stages[0], clip_ticks, clip_hits, limit_ms, label)
        projectiles = stage_projectiles(skill_id, stages[0], clip_ticks, projectile_rows, limit_ms, label)
        if hits or projectiles:
            row = {'skillId': skill_id, 'hits': hits}
            if projectiles:
                row['projectiles'] = projectiles
            out.append(row)
    return {
        'schema': 'lostark.animation-hit-shapes',
        'formatVersion': 3,
        'animationAssetId': asset,
        'characterClass': bindings['characterClass'],
        'skills': out,
    }


def main(argv):
    check = '--check' in argv
    assets = [a for a in argv if not a.startswith('--')]
    if not assets:
        raise SystemExit(__doc__)
    out_dir = os.path.join(REPO, 'Data', 'Animation', 'HitShapes')
    os.makedirs(out_dir, exist_ok=True)
    balance = json.load(io.open(os.path.join(REPO, 'Data', 'Balance', 'PlayerSkills.json'), encoding='utf-8'))
    for asset in assets:
        # build() owns the geometry; migrate() owns the v4 result channels and the
        # maximumRange import for a damage skill the animation gives no shape, so
        # regenerating keeps both instead of dropping one of them.
        document = migrate(build(asset), balance['skills'])
        text = json.dumps(document, indent=2, ensure_ascii=False) + '\n'
        path = os.path.join(out_dir, asset + '.hitshapes.json')
        old = io.open(path, encoding='utf-8').read() if os.path.exists(path) else None
        skill_count = len(document['skills'])
        hit_count = sum(len(s.get('hits', [])) + sum(len(st['hits']) for st in s.get('stages', [])) for s in document['skills'])
        projectile_count = sum(len(s.get('projectiles', [])) + sum(len(st.get('projectiles', [])) for st in s.get('stages', []))
                               for s in document['skills'])
        print('%s: %d skills, %d hits, %d projectiles, %s' % (
            asset, skill_count, hit_count, projectile_count, 'unchanged' if old == text else 'changed'))
        if old != text and not check:
            io.open(path, 'w', encoding='utf-8', newline='\n').write(text)


if __name__ == '__main__':
    main(sys.argv[1:])
```

## 6. 적용 순서와 검증

### 6.1 파일 교체 순서

1. `Tools/CharacterAnimationIntake/fill_animevents_hit_shapes.py`
2. `Tools/CharacterAnimationIntake/fill_projectiles.py`
3. `Tools/CharacterAnimationIntake/build_hitshapes.py`

세 파일 모두 ASCII와 LF를 유지한다. `migrate_player_hit_results.py`는 수정하지 않는다.

### 6.2 재생성 명령

시스템 Python이 없으므로 Blender 번들 인터프리터를 쓴다. 작업 디렉터리는 저장소 루트다.

```powershell
$py = "C:\Program Files\Blender Foundation\Blender 5.0\5.0\python\bin\python.exe"
$all = "LanceMaster","Warlord","Artist","DimensionMaster","GuardianKnight","GunSlinger","Slayer"

& $py Tools/CharacterAnimationIntake/fill_animevents_hit_shapes.py @all --check
& $py Tools/CharacterAnimationIntake/fill_projectiles.py LanceMaster Warlord Artist DimensionMaster --check
```

`--check`로 변경 목록을 먼저 확인한 뒤 같은 명령을 `--check` 없이 실행한다. 그다음
히트 셰이프를 재생성한다.

```powershell
& $py Tools/CharacterAnimationIntake/build_hitshapes.py @all
```

`fill_projectiles.py`에는 일곱 에셋을 한 번에 넘기지 않는다. 가디언나이트, 건슬링어,
슬레이어는 `Data/Animation/Reference/` 아래 `.projectiles`가 없어 `FileNotFoundError`로 멈춘다.
이는 이번 변경과 무관한 기존 동작이다.

### 6.3 게시

```powershell
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Publish
```

`SKILLHIT`, `SKILLSTAGEHIT`, `SKILLPROJ` 행이 갱신된다. 적룡필살은 `SKILLHIT 34650`의
`hitCount`가 3에서 6으로 늘고(기하 도형 2개 곱하기 채널 3개), `SKILLPROJ 34650` 두 행이 새로
생긴다.

### 6.4 성공 판정

```text
자동으로 확인 가능
  세 스크립트의 --check 출력이 1.2절과 4.3절의 변경 목록과 일치
  build_hitshapes 출력에서 Warlord와 Slayer가 unchanged
  Publish-GameplayBalance가 PASS
  Gameplay.bootstrap에 SKILLPROJ 34650 두 행 존재
  Tools/CharacterAnimationIntake/test_player_hitshape_coverage_contract.py 통과
  git diff --check 클린

사용자 화면 확인 (에이전트가 대신 판정하지 않음)
  창술사로 아레나 진입 후 T 적룡필살 사용
  돌진 궤적을 따라 미사일 두 발의 접촉 판정이 몬스터에 적중하는지
  6m 박스가 전방 대상을 밀어내는지 (넉백 110ms, 0.5m)
  기존에 맞던 근접 판정이 사라지지 않았는지
```

### 6.5 실패 시 보존 확인

- `reference_shapes`가 빈 목록을 돌려주면 그 스킬만 건너뛰고 다른 스킬의 `HIT` 행은 유지된다.
- `fill_projectiles`의 재배치 실패는 해당 객체만 제외하고 사유를 출력하며 나머지 객체는 채택된다.
- 세 스크립트 모두 `--check`에서는 디스크를 건드리지 않고, 출력이 기존과 같으면 `unchanged`로
  파일을 다시 쓰지 않는다.
- 되돌리려면 `git checkout -- Data/Animation/Authored Data/Animation/HitShapes` 후
  `Publish-GameplayBalance.ps1 -Mode Publish`를 다시 실행한다.

### 6.6 이 계획을 세우며 실제로 실행한 것

```text
원본 DB 조회      EFTable_SkillEffect.db PK 346500~346506, AreaType/AreaOrigin 분포
데이터 실측       .skilltiming 874개 히트 행의 area별 aa 분포, clipseq와 projectiles의 seq 대조
수정본 실행       out/hitshape-plan/ 에서 세 스크립트 --check 및 실제 재생성
복구              git checkout -- Data/Animation/Authored Data/Animation/HitShapes 로 원상복구 확인

아직 실행하지 않음
  Publish-GameplayBalance.ps1
  test_player_hitshape_coverage_contract.py
  Server/Client 빌드와 실행
  사용자의 아레나 화면 확인
```
