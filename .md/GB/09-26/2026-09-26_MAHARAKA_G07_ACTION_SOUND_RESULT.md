# 2026-09-26 마하라카 G07 — action 4225601 cast/shot 사운드 복구 결과

설계서 `2026-09-26_MAHARAKA_SOURCE_RESTORATION_HANDOFF.md`의 G07을 수행했다.
G01(`2026-09-26_MAHARAKA_G01_ACTION4225601_RESULT.md`)이 확정한 event 4개와 DLChar 조건을
전제로 삼고 다시 조사하지 않았다.

## 1. 이번 G에서 실제 바꾼 것

| 대상 | 변경 | 보존 확인 |
|---|---|---|
| `Data/Sound/CharacterSoundCatalog.json` | `classes`에 신규 `Maharaka` 1개, event 4행 추가 (+1,368 bytes) | 기존 12 class 내용 **완전 동일**, event 총수 2,040 → 2,044, CRLF·무BOM 보존 |
| `Client/Bin/Resources/Sound/Maharaka/S_MOB_MOCOCOWATER1/` | 신규 `.wav` 12개 추가 | 기존 팀장 파일 덮어쓰기 0건(존재 시 건너뛰도록 구현, 실제 건너뜀 0) |

다른 파일은 바꾸지 않았다. C++·셰이더·게시본·`Data/Rendering`·`Client/Bin/ShaderFiles`를
건드리지 않았다. 다른 작업자의 미커밋 변경도 그대로다.

## 2. 원본 근거

### 2.1 패키지 — 설계서 명령이 틀렸다

설계서 G07-01의 `--filter MococoWater`는 **0개 패키지에 일치**한다(exit 1).
`S_Mob_MococoWater1`은 UE AkEvent 패키지 이름이고, `wwise_audio_package.py --filter`는
**Wwise `.pck`의 deobfuscated 이름**을 받는다. 두 이름 공간이 다르다.

`WwiseAudioPackage` 아래 `.pck` 104개의 이름을 전부 복호해 목록을 만들었고
(`out/.../G07/pck_names.txt`), event 4개는 모두 다음 한 곳에 있다.

| 항목 | 값 |
|---|---|
| event 보유 패키지 | **`SOUND_MOB4`** (물리 파일 1.1 MB, banks 101, streams 0) |
| 미디어 보유 패키지 | **`SOUND_MOB4_NONSTREAM`** (244.2 MB, streams 7,589) |
| 검색 범위 | `--filter MOB`에 일치하는 26개 패키지 전부 로드, merged HIRC 371,004 objects |

### 2.2 event ID와 resolve 결과

FNV-1 32bit(소문자) 해시를 직접 계산해 대조했다. FNV1a로 바꾸지 않았다.

| event | FNV1-32 | sources | unresolved | 실제 기록된 파일 |
|---|---|---|---|---|
| `MococoWater1_Attack01_Cast1` | 4025414652 | 3 | 0 | 3 |
| `MococoWater1_Attack01_Shot1` | 156663151 | 3 | 0 | 3 |
| `MococoWater2_Attack01_Cast1` | 3138669013 | 3 | 0 | 3 |
| `MococoWater2_Attack01_Shot1` | 4285122586 | 3 | 0 | 3 |

`EVENT NOT FOUND` 0건, `notStreamed` 0건, `failures` 0건
(`out/.../G07/resolve_report.json`).

### 2.3 도구의 함정 두 개를 실제로 확인했다

1. **exit 0 함정.** `main()`은 `KeyError`를 잡아 `EVENT NOT FOUND`를 출력하고 `continue`한 뒤
   마지막에 `return 0`한다(`wwise_audio_package.py:398~402`). exit code로 판정할 수 없다.
2. **조용한 미기록 함정.** 추출은 `package.stream_by_id(source)`가 `None`이면 아무 파일도 쓰지
   않고 넘어간다(`:410~418`). 따라서 `sources>0`인데 파일 0개가 정상 종료로 보일 수 있다.

그래서 exit code에 의존하지 않고 **event별로 resolve 여부 / sources>0 / 모든 source id가 실제
파일로 기록됨**을 개별 assert하는 스크립트를 작성했다(`g07_resolve_events.py`). 위 4행은 그
assert를 통과한 결과다. Stop-only event는 이 4개에 없다(3절 참조).

### 2.4 성능 — 공유 도구는 수정하지 않았다

`decrypt()`가 1.85 GB를 `bytes(a ^ b for a, b in zip(body, pad))`로 바이트 단위 순회한다
(`:101~106`). 처음 실행은 CPU 830초를 쓰고도 끝나지 않았다. 공유 도구를 고치는 대신
**내 스크립트 프로세스 안에서만** `int.from_bytes` 폭 XOR로 monkeypatch했고,
가장 작은 패키지에서 두 구현의 결과가 **바이트 동일**함을 먼저 검증한 뒤 사용했다.
같은 작업이 86초에 끝났다. `Tools/SoundPipeline/`은 한 글자도 바꾸지 않았다.

## 3. HIRC 구조 — Stop/loop/gain/attenuation

`Event → Action → container → Sound`를 직접 걸어 덤프했다(`event_graph.json`).

| event | action 수 | action type | target |
|---|---|---|---|
| 4개 전부 | 각 1개 | **Play** | `RanSeqCntr` + `Sound` 자식 3개 |

- **Stop/Pause/Resume/Seek action은 없다.** 4개 모두 단일 Play다. 따라서 stop-only event 해석은
  이 행동에 해당 사항이 없다.
- 컨테이너는 `RanSeqCntr` + 자식 3 = 기존 Valtan/Kouku와 같은 **동일 가중치 3 variant** 구조다.
  기존 `CNpc::Update_HitReactionSound`의 avoid-repeat 1 선택 로직을 그대로 재사용할 수 있다.
- **RTPC/attenuation/gain은 복원하지 않았다.** 엔진에 재생할 곳이 없다. 5절을 본다.
- loop 속성 바이트는 디코드하지 않았다. 다만 Play 단일 action이고 미디어 길이가 notify 창과
  맞아떨어지므로(4절) one-shot으로 다룬다. loop 플래그 자체는 **미확인**으로 남긴다.

### 3.1 Water1과 Water2는 같은 미디어를 쓴다 — 중요

| 비교 | 결과 |
|---|---|
| Cast 미디어 | Water1 `[884290698, 1068667522, 883323926]` = Water2 **완전 동일** |
| Shot 미디어 | Water1 `[170625403, 224393252, 605718293]`, Water2 `[224393252, 170625403, 605718293]` — **같은 집합, 순서만 다름** |
| Sound 노드 45 byte | 부모 컨테이너 id 4 byte(offset 21~24)만 다르고 **나머지 41 byte 동일** |
| Action 14 byte | target id(offset 2~5)만 다름 |
| 컨테이너 92 byte | 부모 id(offset 7~10)와 속성 영역 2 byte(offset 16: 0 vs 13, offset 18: 0 vs 4)가 다름 |
| 부모 ActorMixer | Water1 `342382564`(105 byte), Water2 `457115467`(110 byte). 각각 자식 `RanSeqCntr` 16개 |

즉 Water2는 **다른 ActorMixer 아래 별도로 저작된 사본**이고 실제 소리 파일은 같다.
현재 엔진은 Wwise 런타임 없이 디코드된 `.wav`를 평탄한 gain으로 재생하므로,
**DLChar 분기를 구현해도 들리는 소리는 양쪽이 같다.** 분기는 원본 계약이므로 구현하지만,
사용자가 모코모코와 워터캐논에서 다른 소리를 기대하면 안 된다.
offset 16/18 속성의 정확한 의미와 두 ActorMixer의 속성 차이는 Wwise 버전의 `AkPropID` 표가
필요해 **미해결**로 남긴다. 다음 조사 위치는 위 두 ActorMixer id다.

## 4. 시각과 미디어 길이 — 원본이 액션 끝에서 자른다

FMOD(프로젝트 자체 `fmod.dll`)로 디코드한 실측값이다. 6개 모두 **스테레오 44,100 Hz 16bit**.

| 용도 | 미디어 길이 | notify 시작 | notify duration | 시작+duration |
|---|---|---|---|---|
| Cast | **3.133 s** | 0.0 | 3.1333560943603516 | 3.133 |
| Shot | **4.500 s** | 2.2000000477 | 3.799999952316284 | **6.000** |

- Cast의 duration은 **미디어 길이와 일치**한다(3.133 ≒ 3.13336).
- Shot의 duration은 미디어보다 **짧다**(3.80 < 4.50). 그리고 `2.2000000477 + 3.799999952 = 6.0`,
  즉 clip `att_battle_1_01`의 6.000초(180틱/30fps)와 **정확히 같다**.
- 두 경우를 한 규칙으로 설명할 수 있다: **duration = min(미디어 길이, 액션 남은 시간)**.
  따라서 원본은 shot 소리를 액션이 끝나는 6.0초에서 **끊는다**. 4.5초 전체를 흘리면 안 된다.

이 규칙은 실측 두 점에서 유도한 것이다. 다른 notify로 재확인하면 더 단단해진다.

## 5. 엔진 한계 — 3D·감쇠는 구현하지 않았다

`Engine/Public/GameInstance.h:64~71`, `Engine/Public/Sound/Sound_Manager.h`의 공개 API 전체는
다음뿐이다.

```
Play_Sound(path, volume)
Play_SoundCue(path, volume, ageMs, paused, playbackRate) -> handle
Is_SoundCueActive / Pause_SoundCue / Set_SoundCuePlaybackRate / Seek_SoundCue / Stop_SoundCue
Play_LoopingSound / Stop_LoopingSound / Play_Music / Stop_Music
Apply_SoundCategoryVolume(category, volume)
```

**위치 인자, 리스너, 거리 감쇠, 3D 패닝이 없다.** 전부 2D다. 그래서 원본의 attenuation/RTPC를
재현할 방법이 현재 없다. 임의 전역 음량 증폭도 하지 않았다. 3D가 필요하면 Engine 사운드
계약 확장이 별도 수직 슬라이스로 필요하며, 이번 G에서 손대지 않았다.

## 6. G08이 소비할 계약

### 6.1 데이터 입력 (이미 배치 완료)

```
CSoundCueCatalog::Find_Variants("Maharaka", key) -> vector<string> (Resources 상대 경로 3개)

key = "s_mob_mococowater1.mococowater1_attack01_cast1"   MOKOMOKO   cast
      "s_mob_mococowater1.mococowater1_attack01_shot1"   MOKOMOKO   shot
      "s_mob_mococowater1.mococowater2_attack01_cast1"   WATERCANNON cast
      "s_mob_mococowater1.mococowater2_attack01_shot1"   WATERCANNON shot
```

archetype → 분기는 G01이 확정한 DLChar 조건 그대로다.
`NPC_MAHARAKA_MOKOMOKO`(`MN_ISMP_00`) → Water1, `NPC_MAHARAKA_WATERCANNON`(`MN_ISMP_00-1`) → Water2.
**둘을 동시에 재생하지 않는다.**

### 6.2 occurrence clock 소비 규칙

`Client/Private/WorldSequencePlayer_Objects.cpp:1708~1758`이 이미 검증된 같은 모양이다.
새 경로를 만들지 말고 이 규칙을 따른다.

| 규칙 | 값·방법 |
|---|---|
| 큐 시각 | cast `startMs = 0`, shot `startMs = 2200` |
| 큐 창 | cast `durationMs = 3133`, shot `durationMs = 3800` |
| 재생 조건 | `ageMs = localMs - startMs`가 `0 <= ageMs < durationMs`일 때만 |
| 늦은 입장 | `Play_SoundCue(path, 1.f, (uint32_t)ageMs, paused, rate)` — 지난 만큼 건너뛰고 시작 |
| 지나간 one-shot | `ageMs >= durationMs`면 **재생하지 않는다.** 몰아서 틀지 않는다 |
| 중복 방지 | occurrence별 stable key로 `wanted` 집합 대조. 같은 key면 재생하지 않는다 |
| 취소·퇴장·despawn | `wanted`에 없는 handle을 `Stop_SoundCue`하고 목록에서 제거 |
| seek | 기존 handle을 `Stop_SoundCue`한 뒤 새 `ageMs`로 다시 재생 |
| pause | `Play_SoundCue`의 `paused` 인자와 `Pause_SoundCue`로 clock과 함께 정지 |
| 실패 격리 | handle 0이면 그 큐만 상태 메시지로 격리. 매 프레임 파일 I/O를 다시 시도하지 않는다 |
| variant 선택 | 3개 동일 가중치, avoid-repeat 1. 기존 `Update_HitReactionSound`의 asset-id 비교 방식 재사용 |

권장 stable key 형태: `<netEntityId>:<actionId>:<occurrenceSequence>:<cueId>`.
포인터·vector index를 key로 쓰지 않는다.

### 6.3 ms 정책 — 결정과 근거

`Client/Private/NpcActionEffectCueDocument.cpp:54~64`의 `ReadUInt`는
`output = static_cast<std::uint32_t>(raw)`로 **절삭**하고 정수 여부를 검사하지 않는다.

| 원본 초 | ×1000 | 절삭 | 반올림 | 판정 |
|---|---|---|---|---|
| 0.0 | 0.0 | 0 | 0 | 같음 |
| 2.2000000477 | 2200.0000477 | 2200 | 2200 | 같음 |
| 3.1333560943603516 | 3133.356 | 3133 | 3133 | 같음 |
| 3.799999952316284 | 3799.99995 | **3799** | **3800** | **다름** |

shot의 창이 절삭에서 1 ms 짧아진다. 소리로는 무의미하지만 정책이 없으면 값마다 결과가
갈린다. 권장 정책은 **`std::llround(seconds * 1000.0)` 후 범위·유한성 검사**이고, 저작 JSON에는
이미 반올림된 정수 ms만 적는다. `ReadUInt` 자체에 정수 도메인 검사를 넣는 것은 공유 reader
변경이므로 v1 호환 회귀와 함께 G08에서 해야 한다. **이번 G에서는 ms를 정본에 저장하지 않았다.**

### 6.4 schema 공백 — 이번 G에서 메우지 않았다

`lostark.npc-action-effect-cues` version 1은 `cueId/effectAssetId/clip/startMs/durationMs/bone/followBone`만
읽는다. **사운드 필드가 없다.** 임의 필드를 적어도 실행되지 않으므로 적지 않았다.
사운드를 이 문서에 싣으려면 version 확장 + reader + validator + 소비자 + v1 호환 회귀 +
잘못된 version 거부 검사를 한 변경으로 구현해야 하며, 그것은 G08 범위다.
`CNpc::Arm_HitReactionSound`의 기존 5개 하드코딩 배열에는 덧붙이지 않았다.

## 7. 실행한 명령과 결과

| 명령 | 결과 |
|---|---|
| `wwise_audio_package.py --filter MococoWater --list` | **exit 1**, `no package matched` — 설계서 인자가 틀림 |
| `pck` 104개 이름 복호 | 목록 생성, `SOUND_MOB*` 26개 확인 |
| `g07_resolve_events.py` (자체 assert) | **exit 0**, 4 event × (sources 3 / unresolved 0 / written 3 / notStreamed 0), failures 0 |
| `g07_dump_event_graph.py` | exit 0, 4 event 모두 Play 단일 action, `RanSeqCntr`+Sound 3 |
| `g07_diff_water_props.py` | exit 0, Water1/Water2 차이는 id와 속성 2 byte뿐 |
| `g07_parent_mixers.py` | exit 0, 부모 ActorMixer 2개(105/110 byte, 각 자식 16) |
| `wwise_vorbis_to_ogg.py --wav` | **6/6 converted**, exit 0 |
| wav 헤더 실측 | 6개 전부 스테레오 44.1 kHz 16bit, Cast 3.133 s / Shot 4.500 s |
| `g07_deploy_catalog.py` | exit 0, wav 12개 추가, 카탈로그 +1,368 bytes, 미해결 경로 0 |
| 카탈로그 전후 비교 | 기존 12 class 완전 동일, 2,040 → 2,044 event |

**빌드는 하지 않았다.** Visual Studio(pid 30616)가 열려 있어 금지를 지켰다. C++도 바꾸지 않았다.

내가 띄운 python 3개가 중복 실행돼 11 GB를 쓰고 서로 경쟁하던 것을 PID로 종료했다
(14320, 18424, 28448). 다른 프로세스는 건드리지 않았다.

## 8. Drive 전달 목록 (Resources는 Git 비추적)

`Client/Bin/Resources/Sound/Maharaka/S_MOB_MOCOCOWATER1/` 아래 12개.

```
mococowater1_attack01_cast1__884290698.wav    552,768 bytes
mococowater1_attack01_cast1__1068667522.wav   552,768
mococowater1_attack01_cast1__883323926.wav    552,768
mococowater1_attack01_shot1__170625403.wav    793,848
mococowater1_attack01_shot1__224393252.wav    793,848
mococowater1_attack01_shot1__605718293.wav    793,848
mococowater2_attack01_cast1__884290698.wav    552,768
mococowater2_attack01_cast1__1068667522.wav   552,768
mococowater2_attack01_cast1__883323926.wav    552,768
mococowater2_attack01_shot1__224393252.wav    793,848
mococowater2_attack01_shot1__170625403.wav    793,848
mococowater2_attack01_shot1__605718293.wav    793,848
```

Water1과 Water2가 같은 미디어를 쓰므로 6개는 내용이 같은 사본이다. 기존 트리의
`<event>__<mediaId>.wav` 규칙을 그대로 지키려고 event 이름별로 두었다. 합계 약 8.1 MB.
`Sound/` 루트를 새로 만들지 않았고 기존 `Valtan`·`KoukuSaton` 옆에 그룹 폴더 하나를 더했다.

## 9. 상태 구분

| 구분 | 상태 |
|---|---|
| 확인한 원본 | event 4개, FNV1-32 ID, 보유 패키지, 미디어 6개, HIRC 구조, Water1/Water2 차이, 미디어 실측 길이 |
| 실제 구현 | 데이터만. 카탈로그 4행 + Resources wav 12개. **C++ 0줄** |
| live 설치 | Resources 12 파일 추가 완료(기존 파일 덮어쓰기 0) |
| 게시 | **없음.** 이 domain에 publisher가 없다(`Data/Sound`는 Level이 직접 읽는 정본) |
| 빌드/자동검사 | 빌드 안 함(VS 열림). 자체 assert 스크립트 4개 전부 통과, 변환 6/6, 카탈로그 전후 비교 통과 |
| 사용자 수동확인 | **전부 미확인.** 소리를 내가 듣지 않았다. 재생 연결이 G08이라 아직 게임에서 들리지 않는다 |
| 미완료 | 아래 10절 |

## 10. 미완료와 다음 조사 위치

1. **재생 연결 자체가 G08이다.** 지금은 데이터만 있다. `CNpc`의 occurrence clock에
   6.2절 규칙으로 붙이기 전까지 게임에서 이 소리는 나지 않는다.
2. **offset 16/18 속성과 두 ActorMixer의 차이** — Wwise 버전의 `AkPropID` 표 필요.
   다음 위치: object `342382564`, `457115467`의 속성 영역.
3. **loop 플래그 미확인** — Sound 노드 45 byte의 속성 영역을 디코드하지 않았다.
   Play 단일 action과 미디어 길이로 one-shot으로 판단했을 뿐이다.
4. **3D·거리 감쇠** — Engine 사운드 API에 위치·리스너가 없다. 별도 수직 슬라이스.
5. **`min(미디어, 액션 남은 시간)` 규칙** — 실측 두 점에서 유도했다.
   같은 LOA의 다른 sound notify로 재확인하면 확정된다.
6. **나머지 35 actions / 111 sound notify** — 이번 G는 4225601 하나만 했다.
   `MN_ISMP_00.action-effects.json`에 나머지가 있다. 111개가 서로 다른 소리라는 뜻은 아니다.
7. **`ReadUInt` 정수 검사** — 공유 reader 변경이라 v1 호환 회귀와 함께 G08에서.

## 11. 산출물

`out/MaharakaContinuation_20260926_193455/G07/`

```
g07_resolve_events.py        event별 assert 포함 resolve·추출
g07_dump_event_graph.py      Event→Action→container→Sound 덤프
g07_diff_water_props.py      Water1/Water2 바이트 diff
g07_parent_mixers.py         부모 ActorMixer 2개 기술
g07_deploy_catalog.py        Resources 추가 + 카탈로그 원자 패치
resolve_report.json          event별 sources/written/notStreamed
event_graph.json             action type·container·children
water_property_diff.json     차이 offset과 원시 hex
parent_mixers.json           ActorMixer 속성 hex
audio_format.json            채널·샘플레이트·길이 실측
deploy_report.json           추가 파일·카탈로그 행
CharacterSoundCatalog.json.before   패치 전 백업
pck_names.txt                pck 104개 복호 이름
MokoSoundWem/                원본 .wem 6개
MokoSoundAudio/              .ogg 6 + .wav 6
*.log                        각 단계 실행 로그
```
