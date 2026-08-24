# 2026-08-24 Effect Element 재사용·Transform Motion/Hold·Sky Axe 시간 확장 계획

branch: `codex/effect-element-reuse-motion-hold`

base: `origin/main@26147430`

## G01. Element Life와 Transform Motion 시간 분리

### 목표

`Element Life`는 Element의 전체 가시 수명만 소유하고, 새 optional
`transformMotionDurationSeconds`는 위치·회전·scale·velocity·revolution이 진행되는 시간만 소유한다.

```json
"timing": {
  "startDelaySeconds": 0.96,
  "lifeTimeSeconds": 3.24,
  "transformMotionDurationSeconds": 0.24,
  "afterImageSeconds": 0,
  "dissolveStartNormalized": 1
}
```

- `0`: 기존 문서와 완전히 같은 legacy 동작. motion duration은 Life로 평가한다.
- 명시값: `0 < Motion Duration <= Life`만 허용한다.
- transform clock은 `min(localTime, Motion Duration)`에서 멈춘다.
- visibility, color, dissolve, UV는 계속 전체 Life clock을 사용한다.
- standalone Mesh와 Mesh Particle의 Element root transform에 같은 계약을 적용한다.
- 개별 Particle의 Initial Velocity/Acceleration은 이 계약의 대상이 아니다.

### 수정 파일과 계약

- `Client/Public/Effect_AuthoringDocument.h`
  - `EFFECT_TIMING_DESC`에 `fTransformMotionDurationSeconds = 0.f`를 추가한다.
- `Client/Private/Effect_DocumentCodec.cpp`
  - optional parse, nonzero-only serialize, finite/range validation을 추가한다.
  - generic authored Element copy가 새 timing 값을 그대로 보존하는지 검증한다.
- `Client/Private/Effect_Playback.cpp`
  - `Evaluate_ElementWorld`의 transform 평가에만 clamped motion clock을 사용한다.
- `Client/Private/Effect_Tool.cpp`
  - `Separate Transform Motion From Life`, `Motion Duration`, `Hold After Motion`, `Motion End`를
    Effect Detail에 추가한다.
  - 최초 분리 시 Motion Duration을 현재 Life로 복사해 보이는 속도를 보존한다.
  - Hold 편집은 `Life = Motion + Hold`로 stage한 뒤 기존 Apply 흐름으로만 commit한다.
- `Tools/EffectPipeline/Publish-Effects.ps1`
  - source/runtime 양쪽 optional field를 같은 범위로 검증한다.
- `Tools/EffectRenderContractHarness/Private/EffectRenderContractHarness.cpp`
  - legacy omission round-trip, invalid range, motion 종료 뒤 matrix 고정, hold 중 color clock 진행을
    계약 테스트로 추가한다.

## G02. Data Files의 Effect → Family → Element 트리와 단일 Element 복제

### 목표

`Data Files > Saved Skill Effects`에서 저장 Effect 전체를 여는 기존 경로는 유지한다. 각 Effect 아래에
현재 Effect Tool과 같은 Family/Element 트리를 추가하고, 선택한 Element 하나만 현재 편집 Effect에
새 unsaved Element로 append한다.

```text
Saved Skill Effects
└─ VALTAN_TRIPLE_SLASH ...
   ├─ Mesh Particle
   │  ├─ Mesh Particle 01
   │  ├─ Mesh Particle 02
   │  └─ Mesh Particle 03
   └─ Sprite Particle
      ├─ fx_h_hit_01.dds
      └─ fx_a_hit_007.dds

[Load Saved Element for Editing]
```

### 불변식

- 버튼은 source Effect를 현재 Effect로 교체하지 않는다.
- 대상은 NEW/AUTHORED current Effect이고 unapplied detail draft가 없어야 한다.
- 클릭 시 source 파일을 다시 parse해 선택한 stable Element ID가 정확히 하나인지 검증한다.
- `CEffectDocumentCodec::Build_GenericAuthoredElementStartingCopy`를 재사용한다.
- source recipe, attachment, transform inheritance, presentation/source ownership은 제거한다.
- WModel/DDS/material/detail/timing 값은 보존한다.
- Effect-level ParticleSystem multiplier는 Element에 숨겨 복사하지 않는다. source와 target이 다르면
  fail-close한다.
- 새 ID는 `authored.copy.<source-id>.<N>`으로 발급하고 현재 문서 안에서 유일해야 한다.
- append는 staged document에서 수행하고 `Try_CommitDocument` 성공 뒤에만 current document를 교체한다.
- 복제 뒤 새 Element를 선택하고 dirty만 표시하며 자동 저장·publish는 하지 않는다.

### 수정 파일과 계약

- `Client/Public/Effect_Tool.h`
  - `EFFECT_DATA_FILE_ENTRY`에 parse된 read-only document와 parse 상태를 보관한다.
  - 선택한 Data File Element ID와 append helper를 추가한다.
- `Client/Private/Effect_Tool.cpp`
  - `Refresh_DataFiles`에서 문서를 한 번 parse해 stage/commit한다.
  - Saved Skill Effects를 Effect → Family → Element tree로 그린다.
  - `Try_AppendSavedElementToActiveDocument`가 단일 Element 복제 transaction을 소유한다.
- `Tools/EffectPipeline/test_effect_tool_saved_element_clone.py`
  - tree projection, exact source selection, duplicate ID allocation, foreign ParticleSystem 거부,
    current Effect 불변·dirty append를 검증한다.

## G03. Valtan High Jump WORLD authoring 시간과 서버 생존시간

### 현재 실측

```text
TAKEOFF   1933 ms
AIRBORNE  2400 ms
LAND      3200 ms
RECOVERY   400 ms

target axe hit  1200 ms
target axe life 1900 ms
```

### 적용 방향

- TAKEOFF와 LAND는 원본 animation 길이를 유지한다.
- AIRBORNE만 `8000 ms`로 늘려 6–8초 중앙 원 성장 연출을 넣을 authoring 구간을 만든다.
- LAND는 현재 end clip이 loop이므로 8초로 늘리지 않는다. 중앙 원은 AIRBORNE Product/WORLD와
  별도 owner를 갖게 한다.
- `[WORLD]` preview는 `max(effect end, owner AIRBORNE duration)`을 timeline으로 사용한다.
- target axe Server owner life를 AIRBORNE와 같은 `8000 ms`로 늘려 `[WORLD]` Effect가
  stage 도중 조기 제거되지 않게 한다. 도끼 Mesh 자체의 planted/소멸 시점은 Element의
  Motion Duration과 Life가 소유한다.
- player별 WORLD axe Effect에 arena-center 원을 넣으면 인원수만큼 중복되므로 중앙 원 Effect는 이번
  Element 복제/시간 계약과 분리한다.
- AIRBORNE ENTER는 플레이어별 axe를 한 번씩만 생성한다. 반복 투척 schedule은 이번 변경에 넣지 않는다.

### 수정 파일과 계약

- `Data/Encounters/Valtan/ValtanEncounter.json`: HIGH_JUMP AIRBORNE `8000 ms`.
- `Data/Encounters/Valtan/ValtanCombatObjects.json`: target axe owner `lifeMs = 8000`.
- `Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json`: 바뀐 두 field를
  `PROJECT_TUNED` 결과와 동기화한다.
- `Client/Public/Effect_Tool.h`, `Client/Private/Effect_Tool.cpp`: WORLD owner stage duration을 preview에 전달한다.
- publisher 생성 Server bootstrap과 focused Effect/Server contract test를 함께 갱신한다.

## 구현 순서

1. G01 timing schema/codec/playback/UI/harness를 구현한다.
2. G02 Data Files tree와 transactional single-Element append를 구현한다.
3. G03 AIRBORNE/WORLD/target-axe life 계약을 구현한다.
4. Effect publisher Validate/Publish, GameplayBalance publisher Validate, JSON/XML parse를 실행한다.
5. Effect focused unittest, EffectRenderContractHarness, Server Debug contract, Client Debug build를 실행한다.
6. RESULT에 자동 검증과 사용자 수동 화면 검증을 분리해 기록한다.

## 사용자 수동 검증

- Data Files에서 3연격 Effect의 두 바닥 Element가 Family 아래에 보이는지 확인한다.
- 하나를 선택해 `Load Saved Element for Editing`을 누르면 현재 Effect에 복제되고 source Effect는
  열리지 않는지 확인한다.
- Sky Axe Mesh에서 Motion Duration을 고정한 채 Hold를 늘려도 낙하 속도가 변하지 않는지 확인한다.
- Pattern 행 `Play Server Pattern`으로 HIGH_JUMP를 재생해 WORLD owner가 8초를 유지하고,
  axe Mesh는 손튜닝한 Motion/Hold 시점에 맞게 정지·소멸하는지 확인한다.
- AIRBORNE WORLD timeline이 8초까지 열리는지 확인한다.

에이전트는 Client를 실행하거나 visual PASS를 대신 기록하지 않는다.
