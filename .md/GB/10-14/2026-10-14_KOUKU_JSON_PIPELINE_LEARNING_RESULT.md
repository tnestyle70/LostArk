# Python 파싱·투영과 전체 패턴을 읽는 방법

읽는 순서: [00 처음부터 읽기](C:/Users/tnest/Desktop/LostArk/.md/GB/10-14/2026-10-14_KOUKU_FULL_PATTERN_WALKTHROUGH_LEARNING_RESULT.md) → [01 Python 파싱·투영](C:/Users/tnest/Desktop/LostArk/.md/GB/10-14/2026-10-14_KOUKU_JSON_PIPELINE_LEARNING_RESULT.md) → [02 Server 구조체·판정](C:/Users/tnest/Desktop/LostArk/.md/GB/10-14/2026-10-14_KOUKU_SERVER_RUNTIME_LEARNING_RESULT.md) → [03 Client 저작·애니메이션](C:/Users/tnest/Desktop/LostArk/.md/GB/10-14/2026-10-14_KOUKU_CLIENT_ANIMATION_LEARNING_RESULT.md) → [04 Collider·좌표·피해](C:/Users/tnest/Desktop/LostArk/.md/GB/10-14/2026-10-14_KOUKU_COLLIDER_RUNTIME_LEARNING_RESULT.md) → [05 전체 패턴 지도](C:/Users/tnest/Desktop/LostArk/.md/GB/10-14/2026-10-14_KOUKU_PATTERN_ATLAS_LEARNING_RESULT.md)

2026-10-14 현재 저장본을 기준으로 작성했다. 제품 코드·데이터·PPT를 수정하거나 빌드/게임/publisher를 실행하지 않았다.
Python은 C++ 실행 프로세스의 구조체 메모리를 직접 채우지 않는다. Python이 JSON을 읽어 실행용 JSON 두 개를 만들고, Gameplay publisher가 bootstrap 행을 만든다. Server C++의 GameplayCatalog가 그 행을 파싱해 typed 정의를 구성한다.
PPT의 “Python → KoukuSaydon.cpp 구조체”는 이 중간 단계를 생략한 설명이다. Brain은 JSON 파서가 아니며 Catalog의 typed 정의와 현재 서버 상태를 사용한다.

## 1. 현재 파일과 실제 개수

| 역할 | 경로 | revision/개수 |
|---|---|---|
| 저작 정본 | [Composition](C:/Users/tnest/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json) | revision 2500, patterns 121 |
| 서버용 투영 JSON | [Encounter](C:/Users/tnest/Desktop/LostArk/Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json) | sourceRevision 2500, patterns 114 |
| Client 애니메이션·표현 투영 | [patternbindings](C:/Users/tnest/Desktop/LostArk/Data/Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json) | sourceRevision 2500, bindings 584, patterns 114 |

경로 이름 Gate1과 ID의 `_G1_`은 저장 ID 계열이다. 관문은 각 패턴의 실제 `gateId`로 읽는다. 이 한 Composition에 GATE1/GATE2/GATE3/BINGO가 모두 있다.
저장/투영 개수는 GATE1 22/21, GATE2 29/28, GATE3 53/48, BINGO 17/17이다. 121개 전체와 개별 단계·논리·의존은 [pattern_atlas.md](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/pattern_atlas.md), 정확한 파라미터는 [pattern_inventory.json](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/pattern_inventory.json)에 있다.
Composition에는 573 stage가 저장되어 있고 Encounter에는 584 stage가 있다. Parent 확장 및 trailing pose 처리 때문에 저장 stage 수를 runtime 수로 그대로 설명하면 안 된다.

### Composition 루트의 Python 타입

| 키 | 타입 | 값/개수 |
|---|---|---|
| `schema` | `str` | lostark.kouku-saydon-composition |
| `formatVersion` | `int` | 3 |
| `revision` | `int` | 2500 |
| `compositionId` | `str` | boss.composition.koukusaydon.gate1 |
| `encounterId` | `str` | ENCOUNTER_KOUKUSAYDON_G1 |
| `bossArchetypeId` | `str` | BOSS_KOUKUSAYDON_G1_KOUKU |
| `bossPlacementId` | `str` | boss.koukusaydon.g1.kouku |
| `areaId` | `str` | LV_LUT_MIDNIGHTC_ED |
| `fixedTickHz` | `int` | 30 |
| `nextPatternOrdinal` | `int` | 130 |
| `nextFolderOrdinal` | `int` | 27 |
| `nextBundleOrdinal` | `int` | 12 |
| `nextLogicOrdinal` | `int` | 560 |
| `nextSummonOrdinal` | `int` | 10 |
| `nextWorldOrdinal` | `int` | 51 |
| `nextSceneProfileOrdinal` | `int` | 3 |
| `madnessPolicy` | `dict` | 2 |
| `playAllPatternIds` | `list` | 7 |
| `logics` | `list` | 210 |
| `summons` | `list` | 9 |
| `worlds` | `list` | 81 |
| `sceneProfiles` | `list` | 2 |
| `nextPresentationResourceOrdinal` | `int` | 518 |
| `presentationResources` | `list` | 1139 |
| `patterns` | `list` | 121 |
| `folders` | `list` | 26 |
| `bundles` | `list` | 11 |
| `patternFlows` | `list` | 4 |

### Encounter 루트의 Python 타입

| 키 | 타입 | 값/개수 |
|---|---|---|
| `schema` | `str` | lostark.encounter-profile |
| `formatVersion` | `int` | 4 |
| `encounterId` | `str` | ENCOUNTER_KOUKUSAYDON_G1 |
| `bossArchetypeId` | `str` | BOSS_KOUKUSAYDON_G1_KOUKU |
| `authority` | `str` | server |
| `fixedTickHz` | `int` | 30 |
| `sourceRevision` | `int` | 2500 |
| `madnessPolicy` | `dict` | 2 |
| `playAllPatternIds` | `list` | 114 |
| `patterns` | `list` | 114 |
| `folders` | `list` | 22 |
| `bundles` | `list` | 9 |
| `patternInventory` | `dict` | 3 |
| `raidGates` | `list` | 4 |

## 2. JSON 텍스트가 dict/list가 되는 첫 지점

제품 진입 순서는 `main → run → _run → load_json → prepare_publication → projected_outputs`다. `run`은 validation certificate를 기록할 수 있으므로 이번 읽기 조사에서 호출하지 않았다.
[projector.py:6589](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:6589)

```python
def _run(root: Path, mode: str) -> dict[str, Any]:
    source = load_json(root / SOURCE_PATH)
    document, inventory = prepare_publication(source, root)
    outputs = projected_outputs(document, root, inventory)
    session = _PUBLICATION_INPUTS.get()
    if mode == "publish":
        publish_outputs(root, outputs)
    else:
        validate_outputs(root, outputs)
```

[projector.py:392](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:392)

```python
def load_json(path: Path, *, use_cache: bool = True) -> dict[str, Any]:
    session = _PUBLICATION_INPUTS.get() if use_cache else None
    if session is not None:
        path = session.observe(path)
        if path in session.json:
            return session.json[path]
    try:
        text = (session.read_content(path).decode("utf-8") if session is not None
                else path.read_text(encoding="utf-8"))
    except (OSError, UnicodeError) as error:
        raise CompositionError(f"cannot read UTF-8 JSON {path}: {error}") from error
    try:
        value = json.loads(text, object_pairs_hook=_reject_duplicate_pairs)
    except (json.JSONDecodeError, CompositionError) as error:
        raise CompositionError(f"invalid JSON {path}: {error}") from error
    if not isinstance(value, dict):
        raise CompositionError(f"JSON root must be an object: {path}")
```

`json.loads`가 텍스트를 Python 값으로 파싱한다. JSON object는 dict, array는 list, 숫자는 int/float, true/false는 bool, null은 None이 된다. dict가 곧 C++ 구조체라는 뜻은 아니다.
`object_pairs_hook=_reject_duplicate_pairs`는 같은 object 안의 중복 키를 거절한다. 루트가 object인지도 검사한다. 파일 읽기 오류와 JSON 문법 오류는 `CompositionError`로 올린다.
파싱 성공은 게임 데이터의 유효성 성공이 아니다. 이후 `_validate_document`는 schema·version·고정 식별자·30Hz·ID·필드·범위·참조·시간 관계를 검증한다.
[projector.py:2049](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:2049)

```python
def _validate_document(document: dict[str, Any], root: Path = REPOSITORY_ROOT) -> None:
    """Check persisted identities and timing; reference/oracle metadata is advisory."""

    _keys(document, ROOT_KEYS, ROOT_OPTIONAL_KEYS, "composition")
    validate_pattern_flows(document)
    version = _integer(document["formatVersion"], "composition formatVersion", 1, FORMAT_VERSION)
    exact_values = {
        "schema": SCHEMA,
        "compositionId": COMPOSITION_ID,
        "encounterId": ENCOUNTER_ID,
        "bossArchetypeId": BOSS_ARCHETYPE_ID,
        "bossPlacementId": BOSS_PLACEMENT_ID,
        "areaId": AREA_ID,
        "fixedTickHz": FIXED_TICK_HZ,
    }
    for field, expected in exact_values.items():
        if document[field] != expected:
            raise CompositionError(
                f"composition {field} must be {expected!r}, got {document[field]!r}"
```


## 3. 정의와 occurrence를 나누는 이유

| 정의 | 배치/발생 | 구분 |
|---|---|---|
| logics[210]의 logicId | pattern.logicOccurrences의 occurrenceId+logicId | 무엇을 판정할지 vs 이 패턴의 언제 판정할지 |
| RESULT logic의 outcomeKind | onSuccessLogicIds/onFailLogicIds/onTimeoutLogicIds | 결과 정의를 참조하며 순서대로 투영한다. |
| presentationResources[1139] | presentationOccurrences | 재사용 shape/effect/sound 정의 vs 시간·TRS·anchor·bone 배치 |
| worlds[81] | worldOccurrences | WorldObject/sequence 참조 정의 vs 해당 패턴의 배치·수명 |
| summons[9], sceneProfiles[2] | summonOccurrences, sceneProfileOccurrences | 소환/렌더링 정의 vs 시간·개별 설정 |
| patterns[121] | patternOccurrences / bundle.members / flow.entries | 개별 실행 정의 vs 자식 배치·동시 구성·레이드 순서 |

`displayName`은 UI 표시용 한글이다. 조회는 stable ID, 분기는 logicType/judgementKind/triggerKind/outcomeKind로 한다. vector/list 위치와 사람용 이름은 저장 ID가 아니다.
아래 dictionary comprehension은 ID→정의 조회표다. 원본 list의 순서를 재생 순서로 읽는 곳과 ID로 찾는 곳을 구분해야 한다.
[projector.py:5005](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:5005)

```python
    document = _expand_parent_patterns(document)
    arena_archetypes_by_profile = arena_boss_archetypes_by_profile(root)
    logics = {logic["logicId"]: logic for logic in document.get("logics", [])}
    worlds = {world["worldId"]: world for world in document.get("worlds", [])}
    scene_profiles = {row["sceneProfileId"]: row for row in document.get("sceneProfiles", [])}
    world_sequences: dict[str, Any] | None = None
    session = _PUBLICATION_INPUTS.get()
    bone_cache: dict[Any, Any] = session.actors if session is not None else {}
    patterns: list[dict[str, Any]] = []
    for source in document["patterns"]:
        if source["authoringStatus"] != "PRODUCT":
            continue
        root_motion = _project_pattern_root_motion(document, source, root, bone_cache)
        boss_archetype_ids = list(
            arena_archetypes_by_profile.get(_pattern_actor_profile_id(source), [])
        )
        source_action_ids = list(
            dict.fromkeys(
                occurrence["sourceActionId"]
                for stage in source["stages"]
                for occurrence in stage["animationOccurrences"]
                if not (occurrence["sourceActionId"] == 0 and occurrence["sourceStageId"] == "RAW")
            )
```


## 4. 121개 저장과 114개 투영이 다른 이유

현재 저장본의 DRAFT 상태를 곧 게시 불가로 해석하면 틀린다. `_publication_candidate`는 원본을 deep copy하고 검증할 패턴/Bundle만 선택한 뒤 그 복사본의 status만 PRODUCT로 바꾼다. 디스크 원본의 status는 바꾸지 않는다.
[projector.py:2731](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:2731)

```python
def _publication_candidate(source: dict[str, Any], pattern_ids: set[str],
                           bundle_ids: set[str] | None = None) -> dict[str, Any]:
    # Status is a legacy editing field. Only the private validation copy changes;
    # category, timings, lanes, and the persisted document remain authored values.
    candidate = copy.deepcopy({**source,
        "patterns": [p for p in source["patterns"] if p["patternId"] in pattern_ids],
        "bundles": [b for b in source.get("bundles", []) if b["bundleId"] in (bundle_ids or set())]})
    # The Client audition service resolves the saved Flow against this admitted
    # inventory and submits existing pattern/bundle requests after Server completion.
    candidate.pop("patternFlows", None)
    for folder in candidate.get("folders", []):
        if folder.get("timelinePatternId") not in pattern_ids: folder.pop("timelinePatternId", None)
    for row in [*candidate["patterns"], *candidate["bundles"]]:
        row["authoringStatus"] = "PRODUCT"
    candidate["playAllPatternIds"] = [p["patternId"] for p in candidate["patterns"]]
    return candidate
```

`_pattern_dependencies`(2836)는 결과 followup, clone/direction IDs, 자식 patternOccurrences, summon.patternSpawns를 모은다. 비활성 occurrence의 결과 참조도 해석 가능해야 한다.
`_prepare_publication`(2876)은 dependency closure를 만든 뒤 validate_document → validate_publishable → projected_outputs를 수행한다. 실패한 항목은 patternInventory.unavailableReason에 남기며, 나머지 완성된 묶음을 게시 후보로 만든다.
Bundle은 모든 member가 admitted 상태여야 하고, 관문 Flow도 모든 entry가 admitted여야 raid plan으로 투영된다.
[projector.py:2882](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:2882)

```python
    for row in inventory["patterns"]:
        identity = row["patternId"]
        try:
            closure: set[str] = set()
            pending = [identity]
            while pending:
                current = pending.pop()
                if current in closure:
                    continue
                if current not in patterns:
                    raise CompositionError("required Pattern is missing: " + current)
                closure.add(current)
                pending.extend(_pattern_dependencies(source, patterns[current]) - closure)
            candidate = _publication_candidate(source, closure)
            validate_document(candidate, root)
            validate_publishable(candidate, root)
            projected_outputs(candidate, root)
            ready.add(identity)
        except CompositionError as error:
            reasons[identity] = str(error)
        row["unavailableReason"] = reasons.get(identity, "")
    # Every admitted closure was validated, so its dependencies are also admitted.
    product = _publication_candidate(source, ready)
    validate_document(product, root)
    validate_publishable(product, root)
    projected_outputs(product, root)
```


| 미투영 패턴 | 현재 저장된 unavailableReason |
|---|---|
| KOUKUSAYDON_G1_PATTERN_3 / 세이튼_무력화실패 | PRODUCT pattern has no stages: KOUKUSAYDON_G1_PATTERN_3 |
| KOUKUSAYDON_G1_PATTERN_20 / 대형세이튼_레이저 | PRODUCT pattern has no stages: KOUKUSAYDON_G1_PATTERN_20 |
| KOUKUSAYDON_G1_PATTERN_45 / 쿠크세이튼_분신소환_십자화염폭발 | PRODUCT pattern has no stages: KOUKUSAYDON_G1_PATTERN_45 |
| KOUKUSAYDON_G1_PATTERN_51 / 쿠크세이튼_분신소환_기분나빠_전방 | PRODUCT pattern has no stages: KOUKUSAYDON_G1_PATTERN_51 |
| KOUKUSAYDON_G1_PATTERN_65 / 쿠크세이튼_분신소환_십자화염폭발 | Cross direction child needs its clone end Stage before the full ending and must fit the owning window duration |
| KOUKUSAYDON_G1_PATTERN_110 / 앵콜세이튼_백스텝후화염링 | PRODUCT pattern has no stages: KOUKUSAYDON_G1_PATTERN_110 |
| KOUKUSAYDON_G1_PATTERN_111 / 앵콜세이튼_십자화염폭발 | PRODUCT pattern has no stages: KOUKUSAYDON_G1_PATTERN_111 |

Bundle은 11개 저장/9개 투영이다. bundle.8은 P20, bundle.11은 P51이 미투영이므로 제외된다. atlas는 11개 전부의 member, 시작 offset, 제외 사유를 보존한다.

## 5. P1 첫 Stage로 세 종류의 시간을 구분한다

실제 P1 첫 Stage 원문:
```json
{
  "stageId": "STAGE_43",
  "actionId": "KOUKUSAYDON_G1_PATTERN_1.stage.43",
  "stageKind": "ACTIVE",
  "durationMs": 2267,
  "animationOccurrences": [
    {
      "occurrenceId": "KOUKUSAYDON_G1_PATTERN_1.animation.43",
      "profileId": "MN_RPCT_07",
      "sourceActionId": 4219945,
      "sourceStageId": "stage-004",
      "sourceSlotId": "animation-000",
      "referenceRevision": "6c04775d1325013e0ae83ba1dd33edcda5da3e765ff9698c538617a30cd7c98c",
      "runtimeClip": "rpct00_att_battle_6_04",
      "startOffsetMs": 0,
      "sourceStartMs": 0,
      "playMs": 2267,
      "playRate": 1,
      "endPolicy": "EXACT"
    }
  ]
}
```

첫 stage의 action ID는 Client animation binding의 조회 키로도 투영된다. `sourceActionId=4219945`는 원본 animation reference의 동작 출처이고, `sourceStageId=stage-004`/`sourceSlotId=animation-000`은 그 출처 안의 선택이다.
`runtimeClip`은 실제 재생 clip 이름이다. `startOffsetMs=0`은 stage 내부 재생 시작, `sourceStartMs=0`은 원본 clip을 어디부터 샘플링할지, `playMs=2267`은 이 occurrence가 차지하는 재생 시간이다.

P1의 STAGE_43/51/53/54/55/52 길이는 2267/2667/2667/2667/2667/2200ms다. 합은 **15135ms**, 별도 authored durationMs는 **15156ms**다.
현재 Encounter도 6 stage 합15135ms를 유지하면서 timelineDurationMs15156과 fixedTimeline=true를 보관한다. `_pattern_duration`은 effect 끝을 max로 계산하지 않는다.
[projector.py:1132](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:1132)

```python
def _pattern_duration(pattern: dict[str, Any]) -> int:
    return pattern.get("durationMs", 0) or sum(stage["durationMs"] for stage in pattern["stages"])
```

P1 presentation 마지막 끝은15135ms, Logic 마지막 끝은12947ms다. 따라서 21ms 차이가 effect 꼬리나 반올림 때문에 생겼다고 단정할 근거는 없다. 서로 다른 저장 필드로 구분한다.
leaf는 긴 sound/effect/World/Logic 수명 때문에 가짜 pose wait stage를 자동으로 추가하지 않는다. 빈 animation stage 보완과 Parent 확장은 별도 경로다.
[projector.py:1506](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:1506)

```python
    if not parent.get("patternOccurrences"):
        if not parent.get("durationMs"):
            return document
        stages = parent["stages"]
        # A leaf's Stage clock advances the boss independently of row tails.
        # Keep explicitly authored hold stages, but never synthesize a pose wait
        # from a longer sound/effect/World/Logic lifetime.
        if stages:
            if all(stage["animationOccurrences"] for stage in stages) or _trailing_pose_hold_source(parent) is not None:
                return document
```

tick 변환은 다시 별도다. fixed timeline의 bone bake는 누적 ms를 사용하며, legacy 경로는 stage별 ceil tick과 entry tick 보정을 사용한다.
[projector.py:4374](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:4374)

```python
def _bone_bake_stage_origins(pattern, fixed_timeline=False):
    elapsed_ticks = 0
    elapsed_ms = 0
    for index, stage in enumerate(pattern["stages"]):
        # Fixed-timeline patterns share the authored Pattern clock with Logic
        # blends. Preserve the legacy Brain entry-tick schedule for other rows.
        yield stage, (elapsed_ms if pattern.get("durationMs") or fixed_timeline else max(0, elapsed_ticks - (1 if index else 0)) * 1000 / FIXED_TICK_HZ)
        elapsed_ticks += math.ceil(stage["durationMs"] * FIXED_TICK_HZ / 1000)
        elapsed_ms += stage["durationMs"]
```

순수 산술 예: ceil(15135×30/1000)=455, ceil(15156×30/1000)=455 tick이다. 이 산술만으로 모든 Brain stage 진입 시각을 증명하지는 않는다.

P1 Logic definition과 occurrence도 분리해서 읽는다:
```json
{
  "definition": {
    "logicId": "koukusaydon.g1.logic.1",
    "displayName": "세이튼로직_방패무력화",
    "logicType": "DURATION",
    "judgementKind": "STAGGER_WINDOW",
    "threshold": 1150000,
    "shieldArcDegrees": 71.737272,
    "endsPatternOnSuccess": true,
    "normalYawOffsetDegrees": 90
  },
  "occurrence": {
    "occurrenceId": "KOUKUSAYDON_G1_PATTERN_1.logic.1",
    "logicId": "koukusaydon.g1.logic.1",
    "startMs": 2263,
    "durationMs": 10684,
    "enabled": true,
    "onSuccessLogicIds": [
      "koukusaydon.g1.logic.7"
    ],
    "onFailLogicIds": [],
    "onTimeoutLogicIds": [
      "koukusaydon.g1.logic.2"
    ]
  },
  "results": {
    "koukusaydon.g1.logic.7": {
      "logicId": "koukusaydon.g1.logic.7",
      "displayName": "세이튼로직_무력화성공",
      "logicType": "RESULT",
      "outcomeKind": "FOLLOWUP_PATTERN",
      "percent": 0,
      "durationMs": 0,
      "followupPatternId": "KOUKUSAYDON_G1_PATTERN_4"
    },
    "koukusaydon.g1.logic.2": {
      "logicId": "koukusaydon.g1.logic.2",
      "displayName": "세이튼로직_전원전멸",
      "logicType": "RESULT",
      "outcomeKind": "INSTANT_DEATH",
      "percent": 0,
      "durationMs": 0,
      "followupPatternId": ""
    }
  }
}
```

definition은 STAGGER_WINDOW·threshold1150000·shieldArc71.737272·normalYawOffset90을 정한다. occurrence는 start2263+duration10684=12947ms 구간과 성공/시간초과 결과 연결을 정한다.

## 6. P13 한 번의 카드 접촉을 실제 값으로 투영한다

P13은 GATE2의 대형세이튼_조커찾기, owner `boss.koukusaydon.g2.big-saydon`, profile `MN_RPCT_06`, duration32118ms다.
첫 카드 뒤집기 occurrence는 P13.logic.2이다. 이 예시의 순서는 정의 참조 → 기본 창 생성 → Collider 영역 추가 → 카드 대상 추가다.

### 6-1. 실제 입력

```json
{
  "logicOccurrence": {
    "occurrenceId": "KOUKUSAYDON_G1_PATTERN_13.logic.2",
    "logicId": "koukusaydon.g1.logic.22",
    "startMs": 4858,
    "durationMs": 1044,
    "enabled": true,
    "onSuccessLogicIds": [
      "koukusaydon.g1.logic.24",
      "koukusaydon.g1.logic.26"
    ],
    "onFailLogicIds": [],
    "onTimeoutLogicIds": []
  },
  "logicDefinition": {
    "logicId": "koukusaydon.g1.logic.22",
    "displayName": "대형세이튼로직_카드뒤집기",
    "logicType": "TRIGGER",
    "triggerKind": "OBJECT_CONTACT",
    "targetWorldOccurrenceIds": [
      "KOUKUSAYDON_G1_PATTERN_13.world.5",
      "KOUKUSAYDON_G1_PATTERN_13.world.6",
      "KOUKUSAYDON_G1_PATTERN_13.world.7",
      "KOUKUSAYDON_G1_PATTERN_13.world.8",
      "KOUKUSAYDON_G1_PATTERN_13.world.9",
      "KOUKUSAYDON_G1_PATTERN_13.world.10",
      "KOUKUSAYDON_G1_PATTERN_13.world.11"
    ],
    "targetRadiusM": 1.25,
    "contactPriority": 200,
    "contactGroupId": "joker.hammer.contact"
  },
  "colliderOccurrence": {
    "occurrenceId": "KOUKUSAYDON_G1_PATTERN_13.presentation.4",
    "resourceId": "koukusaydon.g1.presentation.29",
    "startMs": 4858,
    "durationMs": 1044,
    "positionOffset": [
      0,
      0,
      0
    ],
    "rotationDegrees": [
      0,
      0,
      0
    ],
    "scale": [
      1,
      1.100000023841858,
      0.6499999761581421
    ],
    "followBoss": true,
    "bone": "b_rpct_01",
    "boneTarget": "WEAPON",
    "anchorKind": "BOSS",
    "logicOccurrenceId": "KOUKUSAYDON_G1_PATTERN_13.logic.2"
  },
  "colliderResource": {
    "resourceId": "koukusaydon.g1.presentation.29",
    "displayName": "콜라이더_2관문_조커찾기_중앙뿅망치",
    "defaultAnchorKind": "BOSS",
    "kind": "COLLIDER",
    "assetId": "",
    "soundEvent": "",
    "resourceKind": "",
    "elementId": "",
    "durationMs": 1000,
    "shape": "BOX",
    "colliderKind": "GEOMETRY",
    "halfExtents": [
      1,
      1,
      1
    ],
    "radiusM": 3,
    "halfAngleDegrees": 45
  }
}
```

`logics[box["logicId"]]`는 여기서 L22를 고른다. `enabled=false`면 이 루프에서 건너뛴다. 모든 TRIGGER를 logicWindows로 만드는 것은 아니며 mechanic trigger로 가는 종류를 분리한다.
[projector.py:5054](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:5054)

```python
        logic_windows: list[dict[str, Any]] = []
        for box in source.get("logicOccurrences", []):
            if not box.get("enabled", True):
                continue
            logic = logics[box["logicId"]]
            kind = logic.get("judgementKind", logic.get("triggerKind"))
            if kind in {"SHOWTIME_PLAYER_TARGETS", "BOSS_TRACK_TARGET", "BOSS_RANDOM_TARGET", "CROSS_DIRECTION_CLONES", "PURSUIT_PROJECTILES", "BINGO_BOARD"}:
                continue
            if logic["logicType"] != "DURATION" and kind not in {"ENTER_AREA", "OBJECT_CONTACT"}:
                continue
            window = _project_logic_window(box, logic, logics, 0.0)
            linked_shields = kind == "STAGGER_WINDOW" and any(
                row.get("logicOccurrenceId") == box["occurrenceId"]
                for row in source.get("presentationOccurrences", []))
            if kind in {"ROULETTE_CARD_MATCH", "AREA_OVERLAP", "ENTER_AREA", "OBJECT_OVERLAP", "OBJECT_CONTACT", "INVULNERABILITY_ZONE"} or linked_shields:
                if world_sequences is None:
                    world_sequences = load_world_sequences(root, document["areaId"])
                window["cardRegions"] = _project_collider_regions(document, source, box, logic, world_sequences, root,
                    bone_cache, bool(root_motion))
                if kind in {"AREA_OVERLAP", "ENTER_AREA", "OBJECT_CONTACT", "OBJECT_OVERLAP"}:
                    owner_world = _contact_world_body_owner(document, source, box, world_sequences)
                    if owner_world: window["ownerWorldOccurrenceId"] = owner_world
            if kind == "OBJECT_OVERLAP":
                window.update(_project_object_overlap_target(document, source, box, logic, logics, world_sequences))
            elif kind == "OBJECT_CONTACT":
                window.update(_project_object_contact_targets(document, source, box, logic, logics, world_sequences))
            logic_windows.append(window)
```

### 6-2. 기본 window와 결과를 만든다

`_project_logic_window`는 occurrence ID를 windowId로, occurrence의 시작·길이를 창 시간으로, L22.triggerKind를 kind로 사용한다. 결과는 L24와 L26을 ID로 찾아 onSuccess 배열로 내린다.
[projector.py:4870](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:4870)

```python
def _project_logic_window(
    box: dict[str, Any],
    logic: dict[str, Any],
    logics: dict[str, dict[str, Any]],
    stop_yaw_degrees: float,
) -> dict[str, Any]:
    kind = logic.get("judgementKind", logic.get("triggerKind"))
    return {
        "windowId": box["occurrenceId"],
        **({"patternIds": list(logic["patternIds"]), "completionCount": logic["completionCount"]} if kind == "PATTERN_COMPLETION_COUNT" else {}),
        **({"cancelAtEnd": True} if box.get("cancelAtEnd", False) else {}),
        **({"holdLogicOccurrenceId": box["holdLogicOccurrenceId"]} if box.get("holdLogicOccurrenceId") else {}),
        **({"bossChargeDistanceM": logic["bossChargeDistanceM"]} if logic.get("bossChargeDistanceM", 0) else {}),
        **({"chargeYawOffsetDegrees": logic["chargeYawOffsetDegrees"]} if logic.get("chargeYawOffsetDegrees", 0) else {}),
        **({"rearmOnExit": True} if logic.get("rearmOnExit", False) else {}),
        **({"repeatAfterKnockback": True} if logic.get("repeatAfterKnockback", False) else {}),
        **({"repeatIntervalMs": logic["repeatIntervalMs"]} if logic.get("repeatIntervalMs", 0) else {}),
        "kind": kind,
        **({"gazeDuringWindow": True} if logic.get("gazeDuringWindow", False) else {}),
        "startMs": box["startMs"],
        "durationMs": box["durationMs"],
```

[projector.py:4903](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:4903)

```python
        "threshold": int(logic.get("threshold", 3 if kind == "BINGO_COMPLETED_LINES" else 0)) if kind in {"STAGGER_WINDOW", "BINGO_COMPLETED_LINES", "INVULNERABILITY_ZONE"} else 0,
        "shieldArcDegrees": float(logic.get("shieldArcDegrees", 0.0)) if kind == "STAGGER_WINDOW" else 0.0,
        "endsPatternOnSuccess": bool(logic.get("endsPatternOnSuccess", False)) if kind in {"STAGGER_WINDOW", "COUNTER_WINDOW", "EXTERNAL_SIGNAL"} else False,
        "onSuccess": _project_outcomes(logics, outcome_logic_ids(box, "Success")),
        "onFail": _project_outcomes(logics, outcome_logic_ids(box, "Fail")),
        "onTimeout": _project_outcomes(logics, outcome_logic_ids(box, "Timeout")),
    }
```

[projector.py:4792](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:4792)

```python
def _project_outcomes(logics: dict[str, dict[str, Any]], targets: list[str]) -> list[dict[str, Any]]:
    projected: list[dict[str, Any]] = []
    for target in targets:
        logic = logics[target]
        projected.append({
            "kind": logic["outcomeKind"],
            "percent": int(logic.get("percent", 0)),
            "durationMs": int(logic.get("durationMs", 0)),
            "patternId": logic.get("followupPatternId", ""),
        })
        if logic["outcomeKind"] == "FIXED_DAMAGE":
            projected[-1]["damageAmount"] = int(logic["damageAmount"])
```

[projector.py:4820](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:4820)

```python
        elif logic["outcomeKind"] == "PLAY_CONTACT_WORLD_OBJECT_MOTION":
            projected[-1]["contactMotions"] = [dict(row) for row in logic["contactMotions"]]
        elif logic["outcomeKind"] == "COMPLETE_LOGIC_WINDOW":
            projected[-1]["targetLogicOccurrenceId"] = logic["targetLogicOccurrenceId"]
            projected[-1]["contactTargetWorldOccurrenceId"] = logic.get("contactTargetWorldOccurrenceId", "")
        elif logic["outcomeKind"] == "MARIO_ENTER" and int(logic.get("marioStage", 0)):
            # Omitted at 0 so existing product rows stay byte-identical.
            projected[-1]["marioStage"] = int(logic["marioStage"])
    return projected
```

이 세 함수의 **현재 원문만 AST로 추출해** P13 실제 입력에 실행했다. 파일 로드/본 bake/게시 entrypoint는 호출하지 않았다.
기본 창의 cardRegions를 제외한 21개 필드가 현재 Encounter와 정확히 일치했다. [p13_projection_step.json](C:/Users/tnest/Desktop/LostArk/out/KoukuFullLearning20261014/p13_projection_step.json)에 함수 원문 hash·비교 필드·결과를 남겼다.
이는 이 dict 변환 단계의 검증이다. Collider 본 샘플과 WorldObject 배치를 이번 조사에서 새로 bake·게시했다는 뜻은 아니다.

### 6-3. Collider와 카드 대상을 붙인 현재 Encounter 결과

```json
{
  "windowId": "KOUKUSAYDON_G1_PATTERN_13.logic.2",
  "kind": "OBJECT_CONTACT",
  "startMs": 4858,
  "durationMs": 1044,
  "contactGroupId": "joker.hammer.contact",
  "contactPriority": 200,
  "cardRegion": {
    "regionId": "KOUKUSAYDON_G1_PATTERN_13.presentation.4",
    "shape": "BOX",
    "anchorKind": "BOSS_CURRENT",
    "center": [
      0,
      0,
      0
    ],
    "yawDegrees": 0,
    "halfExtents": [
      1,
      1.100000023841858,
      0.6499999761581421
    ],
    "radiusM": 3,
    "halfAngleDegrees": 45,
    "cardSymbol": "NONE",
    "cardColor": "NONE",
    "worldTrackSummary": {
      "startMs": 4858,
      "startDelayMs": 0,
      "durationMs": 1044,
      "playbackSpeed": 1.0,
      "interpolation": "LINEAR",
      "baselinePosition": [
        0.0,
        0.0,
        0.0
      ],
      "baselineYawDegrees": 0.0,
      "baselineScale": [
        1.0,
        1.0,
        1.0
      ],
      "keyCount": 57,
      "firstKey": {
        "timeMs": 0,
        "positionOffset": [
          9.770304424,
          -0.29156416,
          0.402464307
        ],
        "rotationY": 0.0,
        "rotationW": 1.0,
        "scaleMultiplier": [
          1.0,
          1.0,
          1.0
        ],
        "visible": true
      },
      "lastKey": {
        "timeMs": 1044,
        "positionOffset": [
          4.227649913,
          3.077926892,
          -6.421468872
        ],
        "rotationY": 0.0,
        "rotationW": 1.0,
        "scaleMultiplier": [
          1.0,
          1.0,
          1.0
        ],
        "visible": true
      }
    }
  },
  "onSuccess": [
    {
      "kind": "PLAY_CONTACT_WORLD_OBJECT_MOTION",
      "percent": 0,
      "durationMs": 0,
      "patternId": "",
      "contactMotions": [
        {
          "targetWorldOccurrenceId": "KOUKUSAYDON_G1_PATTERN_13.world.5",
          "motionInstanceId": "world.object.instance.kouku.card_flip"
        },
        {
          "targetWorldOccurrenceId": "KOUKUSAYDON_G1_PATTERN_13.world.6",
          "motionInstanceId": "world.object.instance.kouku.card_flip"
        },
        {
          "targetWorldOccurrenceId": "KOUKUSAYDON_G1_PATTERN_13.world.7",
          "motionInstanceId": "world.object.instance.kouku.card_flip"
        },
        {
          "targetWorldOccurrenceId": "KOUKUSAYDON_G1_PATTERN_13.world.8",
          "motionInstanceId": "world.object.instance.kouku.card_flip"
        },
        {
          "targetWorldOccurrenceId": "KOUKUSAYDON_G1_PATTERN_13.world.9",
          "motionInstanceId": "world.object.instance.kouku.card_flip"
        },
        {
          "targetWorldOccurrenceId": "KOUKUSAYDON_G1_PATTERN_13.world.10",
          "motionInstanceId": "world.object.instance.kouku.card_flip"
        },
        {
          "targetWorldOccurrenceId": "KOUKUSAYDON_G1_PATTERN_13.world.11",
          "motionInstanceId": "world.object.instance.kouku.joker_card_flip"
        }
      ]
    },
    {
      "kind": "COMPLETE_LOGIC_WINDOW",
      "percent": 0,
      "durationMs": 0,
      "patternId": "",
      "targetLogicOccurrenceId": "KOUKUSAYDON_G1_PATTERN_13.logic.6",
      "contactTargetWorldOccurrenceId": "KOUKUSAYDON_G1_PATTERN_13.world.11"
    }
  ]
}
```

저작 anchor BOSS+follow=true가 BOSS_CURRENT로 투영되었다. WEAPON `b_rpct_01`의 motion은 숫자 worldTrack으로 저장되며, Server가 Client CModel의 bone 포인터를 읽는 구조가 아니다.
shape BOX의 halfExtents [1,1,1]과 scale [1,1.100000023841858,0.6499999761581421]가 결합되어 위 영역 크기가 된다. boneRotation이 저장되지 않은 이 occurrence는 기본값 경로를 사용한다.
`_project_object_contact_targets`는 같은 패턴의 world occurrence ID를 찾고 고정 target 위치·반경을 만든다. contactMotions는 모든 후보를 정확히 한 번 매핑해야 한다.
[projector.py:3523](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:3523)

```python
def _project_object_contact_targets(document, pattern, box, logic, logics, sequences):
    worlds = {row["worldId"]: row for row in document.get("worlds", [])}
    cues = {row["occurrenceId"]: row for row in pattern.get("worldOccurrences", [])}
    targets = []
    for occurrence_id in logic["targetWorldOccurrenceIds"]:
        cue = cues.get(occurrence_id)
        if cue is None:
            raise CompositionError("OBJECT_CONTACT target must name a same-pattern WORLD occurrence")
        world = worlds[cue["worldId"]]
        target, _, _ = _project_fixed_object_target(cue, world, box, logic["targetRadiusM"], sequences, "OBJECT_CONTACT")
        targets.append({"targetWorldOccurrenceId": occurrence_id, **target})
    by_id = {row["targetWorldOccurrenceId"]: row for row in targets}
    for result_id in outcome_logic_ids(box, "Success"):
        result = logics[result_id]
        if result.get("outcomeKind") != "PLAY_CONTACT_WORLD_OBJECT_MOTION":
            continue
        if {row["targetWorldOccurrenceId"] for row in result["contactMotions"]} != set(by_id):
            raise CompositionError("OBJECT_CONTACT contactMotions must map every candidate exactly once")
        for mapping in result["contactMotions"]:
            # Contact is consumed once per card occurrence; its response may flip,
            # fly away or disappear without changing an unhit card's fixed target.
            _validate_world_object_motion_result({
                "targetWorldInstanceId": by_id[mapping["targetWorldOccurrenceId"]]["targetWorldInstanceId"],
                "motionInstanceId": mapping["motionInstanceId"]}, sequences)
    return {"contactTargets": targets, "contactGroupId": logic.get("contactGroupId", ""),
            "contactPriority": logic.get("contactPriority", 0)}
```


| target occurrence | instance | X / Z (m) | 반경 m |
|---|---|---|---:|
| KOUKUSAYDON_G1_PATTERN_13.world.5 | world.object.instance.kouku.card | 4.2870001792907715 / 323.2749938964844 | 1.25 |
| KOUKUSAYDON_G1_PATTERN_13.world.6 | world.object.instance.kouku.card | 13.024999618530273 / 325.96600341796875 | 1.25 |
| KOUKUSAYDON_G1_PATTERN_13.world.7 | world.object.instance.kouku.card | 9.954999923706055 / 325.5010070800781 | 1.25 |
| KOUKUSAYDON_G1_PATTERN_13.world.8 | world.object.instance.kouku.card | 6.894000053405762 / 324.59698486328125 | 1.25 |
| KOUKUSAYDON_G1_PATTERN_13.world.9 | world.object.instance.kouku.card | 1.343999981880188 / 314.8970031738281 | 1.25 |
| KOUKUSAYDON_G1_PATTERN_13.world.10 | world.object.instance.kouku.card | 2.944000005722046 / 320.74700927734375 | 1.25 |
| KOUKUSAYDON_G1_PATTERN_13.world.11 | world.object.instance.kouku.joker_card | 2.1440000534057617 / 317.74700927734375 | 1.25 |

L22의 priority200과 L23의 priority100은 같은 contactGroupId `joker.hammer.contact`를 공유한다. L24는 접촉한 카드 flip, L25는 hop 모션을 매핑한다.
L26 COMPLETE_LOGIC_WINDOW는 접촉 대상이 world.11일 때 P13.logic.6을 완료하도록 지정한다. P13.logic.6의 EXTERNAL_SIGNAL 성공 결과 L21이 P14로 이어진다. stable world.11이 조커 의미를 가진다.
실제 접촉 우선순위와 한 번 소비 여부는 C++ LogicRuntime의 실행 ledger가 책임진다. Python은 그 규칙의 입력 데이터를 구성하는 단계다.

## 7. Client 표현 timeline은 Encounter와 별도 결과다

project_presentation은 stage/action/clip/time binding과 pattern presentation occurrence를 만든다. Collider 숫자 샘플을 서버에 전달하는 데이터와 화면용 animation/effect/sound의 소비자는 다르다.
[projector.py:6247](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:6247)

```python
                bindings.append(
                    {
                        "actionId": stage["actionId"],
                        "occurrenceId": occurrence["occurrenceId"],
                        "clip": occurrence["runtimeClip"],
                        **transition,
                        **({"animationBlendWindows": copy.deepcopy(stage_blends)} if stage_blends else {}),
                        "startOffsetMs": occurrence["startOffsetMs"],
                        "sourceStartMs": occurrence["sourceStartMs"],
                        **({"sourceEndMs": occurrence["sourceEndMs"]} if occurrence.get("sourceEndMs", 0) else {}),
                        "playMs": occurrence["playMs"],
                        "playRate": occurrence["playRate"],
                        "endPolicy": occurrence["endPolicy"],
                        **({"holdAtWindowEnd": True} if _animation_holds_window_end(stage, occurrence) else {}),
                        **({"unblendedBoneContact": True} if unblended_bone_contact else {}),
                        **({"animationRootVerticalScale": 0.0} if suppress_root_motion else
                           ({"animationRootVerticalScale": pattern["animationRootVerticalScale"]} if pattern.get("animationRootVerticalScale", 1.0) != 1.0 else {})),
                    }
                )
            stage_start += stage["durationMs"]
```

`_project_pattern_presentation`(6032)은 resourceId로 resource를 찾아 occurrence의 시간·TRS와 합친다. controlled showtime/airborne/bomb visual은 해당 제어 경로로 분리한다.
현재 patternbindings에는 bindings584, patterns114, fearPresentations3, attachmentGrips1, targetedCombatVisuals25가 있다. P13에는 presentationOccurrences39와 sourceAnchorAnimations21, randomTargetWindows3가 있다.
현재 JSON의 실제 필드명은 `presentationOccurrences`와 `bindings`이다. “cue timeline”은 이러한 시간 배치의 역할을 부르는 말이며 별도 cueTimelines 루트 키를 가정하면 안 된다.

## 8. 투영 JSON이 서버 입력으로 게시되는 연결

[projector.py:6290](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:6290)

```python
def projected_outputs(document: dict[str, Any], root: Path = REPOSITORY_ROOT,
                      pattern_inventory: dict[str, Any] | None = None) -> dict[Path, bytes]:
    key = (str(root.resolve()), _memo_digest(document))
    encounter, presentation = _publication_memo("projection", key,
        lambda: (project_encounter(document, root), project_presentation(document, root)))
    if pattern_inventory is not None:
        encounter = {**encounter, "patternInventory": pattern_inventory}
    sequence_path = root / "Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json"
    if _input_exists(sequence_path):
        from raid_flow_projection import project_raid_gates
        try:
            sequence = load_json(sequence_path)
            # The Client loads the entire Sequence catalog before playing a raid.
            # Reuse its existing World identity rule even for unreferenced actors.
            _validate_catalog(sequence, "nextWorldOrdinal", "worlds", "worldId",
                GENERATED_WORLD_RE, WORLD_KEYS, MAX_WORLDS, "koukusaydon.g1.world",
                optional_keys={"positionOffset", "anchorKind", "anchorPosition", "companionEffectResourceId", "objectResourceId"})
            encounter = {**encounter, "raidGates": project_raid_gates(document, sequence)}
        except (ValueError, KeyError, TypeError) as error:
            raise CompositionError(f"Complete Play raid plan: {error}") from error
    encounter_bytes = serialize_encounter_json(encounter)
    return {
        ENCOUNTER_PATH: encounter_bytes,
        PRESENTATION_PATH: serialize_json(presentation),
    }
```

`projected_outputs`는 Encounter와 patternbindings 두 파일을 반환한다. SequenceComposition이 있으면 raid_flow_projection.project_raid_gates로 4관문의 raidGates를 Encounter에 결합한다.
serialize_json(6284)은 ensure_ascii=False/allow_nan=False의 UTF-8 JSON을 만든다. 한글 표시 이름을 보존하며 NaN 저장을 허용하지 않는다.
publish_outputs(6359)는 임시 파일 작성→재파싱→백업→입력 freshness 확인→교체 순서를 사용한다. validate_outputs(6344)는 실제 저장 출력과 예상 출력의 개행 정규화 byte 비교 후 재파싱한다.

| 이후 단계 | 실제 위치 |
|---|---|
| Gameplay publisher가 projector validate 선행 | [Publish-GameplayBalance.ps1:33](C:/Users/tnest/Desktop/LostArk/Tools/GameplayPipeline/Publish-GameplayBalance.ps1:33) |
| Encounter를 읽어 Add-KoukuBootstrapRows 호출 | [Publish-GameplayBalance.ps1:3565](C:/Users/tnest/Desktop/LostArk/Tools/GameplayPipeline/Publish-GameplayBalance.ps1:3565) |
| PATTERNLOGICREGION/CAPTURE/MOTION/WORLD, OUTCOME/PUSH 행 생성 | [KoukuBootstrapRows.ps1:1509](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/KoukuBootstrapRows.ps1:1509) / [1771](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/KoukuBootstrapRows.ps1:1771) |
| 기본 Server bootstrap 경로 선택 | [GameplayCatalog::Load:1419](C:/Users/tnest/Desktop/LostArk/Server/Private/GameplayCatalog.cpp:1419) |
| 행→C++ typed 정의 파싱 | [Load_BootstrapBytes:1773](C:/Users/tnest/Desktop/LostArk/Server/Private/GameplayCatalog.cpp:1773) |
| typed pattern으로 시작 | [Brain::Begin_Pattern:1258](C:/Users/tnest/Desktop/LostArk/Server/Private/KoukuSaydonBrain.cpp:1258) |
| 실행 ledger와 tick으로 논리 처리 | [LogicRuntime::Build:129](C:/Users/tnest/Desktop/LostArk/Server/Private/KoukuSaydonLogicRuntime.cpp:129) / [Update:1750](C:/Users/tnest/Desktop/LostArk/Server/Private/KoukuSaydonLogicRuntime.cpp:1750) |

기본 설치 bootstrap은 `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap`이며 LOSTARK_SERVER_DATA_ROOT로 루트가 달라질 수 있다. 저장된 JSON, 게시된 bootstrap, 실행 중 Server가 핀한 Catalog revision은 별도로 확인한다.

## 9. 전체 읽기 순서와 이번 확인의 범위

먼저 atlas의 관문 Flow entries와 HP group 경계를 읽고 원하는 entry의 pattern/bundle로 이동한다. 패턴 안에서는 owner→stage/clip→Logic occurrence→결과→Collider/World→의존 패턴 순으로 읽는다.
Flow 전체는358entry, entryGroups37개다. GATE1 166entry/13group, GATE2 85/13, GATE3 72/11, BINGO35/0이며 BINGO는 loopStartEntryId와 bingoSpecialPatternId를 별도로 사용한다.
210 Logic 정의(TRIGGER83/DURATION50/RESULT77)는 atlas 부록에서 ID·한글 이름·kind·저장 필드·참조 occurrence를 분류했다. 미참조 정의도 누락하지 않았다.
검증: 세 JSON 문법 파싱, 전체121 ID의 중복/Encounter114 ID 부분집합 확인, 입력 hash 기록 및 추출 중 저장 변경 없음 확인, P13 순수 dict 변환 필드 비교. 전체 projector 실행·본 bake·게시·빌드·화면 검증은 하지 않았다.
좌표 의미와 C++ 피해/넉백의 이어지는 설명은 [패턴 흐름 메모](C:/Users/tnest/Desktop/LostArk/out/BossSimulationRefactor20261014/PATTERN_FLOW_NOTES.md)를 참고한다.
PPT의 서술을 현재 구조로 고쳐 설명하는 자료이며, 조사로 확인한 현재 구조를 사용자의 과거 개인 기여로 자동 귀속하지 않는다.
