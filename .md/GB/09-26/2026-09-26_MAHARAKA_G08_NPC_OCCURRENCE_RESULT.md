# 2026-09-26 마하라카 복원 G08 — CNpc occurrence·clock·handle 연결 결과

설계서 `.md/GB/09-26/2026-09-26_MAHARAKA_SOURCE_RESTORATION_HANDOFF.md` G08을 수행한 기록이다.
배관(plumbing)을 실제로 닫았고 정본 Product 빌드와 실제 reader를 링크한 계약 검사로 확인했다.
화면·소리 판정은 하지 않았다. **G08은 배관까지 완료이고 마하라카 연출 재생은 아직 관찰할 수 없다.**

## 1. 실제 바꾼 파일

| 파일 | 크기 변화 | 인코딩 |
|---|---|---|
| `Client/Public/NpcActionEffectCueDocument.h` | 1,537 → 2,691 (+1,154) | ASCII·CRLF·무BOM 유지 |
| `Client/Private/NpcActionEffectCueDocument.cpp` | 5,225 → 8,789 (+3,564) | 동일 |
| `Client/Public/Npc.h` | 19,972 → 20,559 (+587) | 동일 |
| `Client/Private/Npc.cpp` | 54,492 → 59,341 (+4,849) | 동일 |

백업은 `out/MaharakaContinuation_20260926_193455/G08/backup/*.before` 4개다.
새 C++ 주석은 전부 영문이다. 이 저장소는 UTF-8 소스를 CP949로 컴파일하므로 줄 끝 한글 주석이
다음 줄을 삼킨다.

`.vcxproj`/`.filters`는 수정하지 않았다. 새 파일을 제품에 추가하지 않고 기존 4파일 안에서 해결했다.

### 보존 확인

`Data/Rendering`, `Client/Bin/DataFiles/Rendering`, `UI_Sprite.cpp`, `Renderer.*`,
`Client.vcxproj`, `Client.vcxproj.filters` diff가 전부 빈 출력이다.
`Publish-RenderingProfiles.ps1`을 실행하지 않았다. `Client/Bin/Resources`에 쓰지 않았다.
`install_terrain.py`를 재실행하지 않았다.

**내가 바꾸지 않았는데 이 세션 중에 바뀐 파일 2개가 있다.**
`Tools/LevelPlacementExtractor/extract_source_map_component_lighting.py`(21:25:32)와
`test_extract_source_map_component_lighting.py`(21:34:23)다. 내 편집은 21:07~21:16이고
`Tools/`를 건드린 적이 없다. 다른 세션의 작업이므로 보존했다.

## 2. 원본 근거 (G01/G06/G07에서 확정된 것을 소비)

이번 G에서 원본을 새로 조사하지 않았다. 앞선 결과를 입력으로 썼다.

- duration 규칙(G06): 양수 = 소유자가 정지, 0 = 자체 종료. WaterFinish만 `emitterloops` 14행
  명시적 1이고 자체 종료 1.800초, 나머지 3종은 미직렬화 → UE3 기본 0 = 무한.
- 사운드 조건(G07): 기본 Water1, `DLChar == EFDLChar_MN_ISMP_00-1`이면 Water2.
- 사운드 duration 규칙(G07): min(미디어 길이, 액션 남은 시간). shot은 `2.2000000477 + 3.799999952
  = 6.000`으로 액션 끝에서 끊는다.
- socket(G01): `fx_01` → parent bone `b_ismp_root`, `(0, −50, 90)` cm, Roll −16384 = −90.0°.

## 3. 구현 내용

### 3.1 엔진 계약과 원본 계약이 이미 일치함을 확인

`Client/Private/Effect_Playback.cpp`의

```
bBoundedSourceLoop = m_fSourceLoopEndSeconds > 0.f &&
    Element.SourceRecipe.bEnabled && Element.SourceRecipe.iEmitterLoopCount == 0u
```

이 G06이 원본에서 유도한 규칙과 정확히 같다. 양수면 무한 source loop를 끊고 0이면 저작 타이밍을
유지한다. 따라서 **수동 정지 타이머를 만들지 않았고 새 renderer도 만들지 않았다.**
`cue.iDurationMs`를 `bOwnerSustainedSourceLoops` + `fSourceLoopEndSeconds`로 넘기면 끝이다.

### 3.2 `Npc.cpp` spawn 경로

이전에는 `cue.iDurationMs`가 파싱만 되고 spawn에서 쓰이지 않았고, 반환
`EFFECT_WORLD_ROOT_HANDLE`이 지역변수로 버려졌다. 두 가지를 모두 연결했다.

- `0u != cue.iDurationMs`면 `desc.bOwnerSustainedSourceLoops = true`,
  `desc.fSourceLoopEndSeconds = durationMs * 0.001f`. 0이면 둘 다 기본값으로 두어 저작 타이밍을 보존한다.
- 반환 handle을 `NPC_ACTION_EFFECT_LIVE_CUE`에 담아 `m_NpcActionEffectState.LiveCues`가 소유한다.
- `Release_ActionEffectCues()`가 `Stop_WorldRoot`와 `Stop_SoundCue`로 정리한다.
  호출처는 **새 action edge(`Arm_ActionEffectCues` 맨 앞)와 소멸자** 두 곳이다.
- `m_NpcActionEffectState.Reset()` 호출처는 저장소 전체에서 1곳이고 바로 앞에 Release가 있다.
  Grep으로 확인했으므로 handle 누출 경로가 없다.
- 무언가를 실제로 시작한 cue만 `LiveCues`에 넣는다. 정리가 소유하지 않은 handle을 끄지 않는다.
- 정상 tail(엔진이 `fSourceLoopEndSeconds`로 경계)과 명시 kill(재무장·teardown)을 구분한다.

### 3.3 clock

`Update_ActionEffectCues`는 cursor가 소진돼 `bActive`가 false가 된 뒤에도 **사운드 정지 시각이
남아 있으면 시계를 계속 돌린다.** 클립의 마지막 notify가 자기 정지 시각보다 먼저 시작하므로,
이게 없으면 마지막 소리가 영원히 끊기지 않는다. 정지가 모두 끝나면 조기 반환으로 되돌아간다.

frame overshoot는 기존대로 `fInitialSampleTimeSeconds = fElapsedSeconds - fDue`로 저작 위상에서
시작한다. 사운드도 같은 age를 `Play_SoundCue(path, volume, ageMs)`에 넘겨 늦은 프레임이 one-shot을
처음부터 다시 틀지 않게 한다.

### 3.4 사운드 연결

`Play_ActionEffectCueSound`가 같은 occurrence clock에서 소비한다.

- 모델 태그가 `alternateModelTagSuffix`로 끝날 때만 alternate event를 쓴다. **둘 중 하나만 재생한다.**
- `CSoundCueCatalog::Find_Variants` → equal weight, avoid-repeat-1. 이전 asset은 per-instance
  멤버 `m_strLastActionEffectSoundAsset`이 들고 있어 NPC 두 마리가 서로 오염되지 않는다.
- `Play_SoundCue`가 handle을 돌려주므로 `fSoundStopAtSeconds = fDue + durationMs*0.001f`에
  `Stop_SoundCue`로 끊는다. 이것이 min(미디어, 남은시간) 규칙의 실행 형태다.
- `Arm_HitReactionSound`의 기존 5개 하드코딩 배열은 건드리지 않았다.

### 3.5 스키마 version 2

`lostark.npc-action-effect-cues` formatVersion 2를 추가했다. reader·검증·소비자·v1 호환·
잘못된 version 거부를 같은 변경에 넣었다.

- 행의 optional `sound` 블록: `soundClass`, `event`, optional `alternateEvent` +
  `alternateModelTagSuffix`. **둘 중 하나만 있으면 거부**한다. 조건을 평가할 수 없기 때문이다.
- v1 문서에 `sound`가 있으면 세대 불일치로 거부한다. 무시하지 않는다.
- v2에서만 `effectAssetId`가 빈 문자열일 수 있다. 원본 cast(0초)·shot(2.2초)은 파티클이 없는
  시각이라 사운드 전용 행이 없으면 아예 표현할 수 없었다. 빈 asset은 catalog 대조를 건너뛰고,
  visual도 sound도 없는 행은 거부한다.

### 3.6 ms 정책

`ReadUInt`가 `static_cast<uint32_t>`로 절삭하던 것을 `std::llround` 후 정수성 검사로 바꿨다.
허용 오차는 0.001 ms다. 원본 2299.9999523은 **2300**이 된다(이전에는 2299). 진짜 비정수인
1500.5 같은 값은 거부한다. 기존 v1 문서 5개에 비정수 ms가 하나도 없어 값이 바뀌지 않는다.

### 3.7 실패 보존

`Load`가 파싱 전에 빈 cache 항목을 넣던 것을 고쳤다. 이전에는 malformed 문서가 실패한 뒤
다음 호출에서 `g_Cues.contains()`에 걸려 **빈 성공**으로 바뀌었다. 이제 문서가 없을 때만 빈 항목을
commit하고, 거부는 이유와 함께 `g_Failures`에 남아 다음 호출에서 같은 실패를 그대로 돌려준다.
중복 cue ID도 거부한다(placement id가 모호해져 handle 장부가 깨진다).

## 4. 하지 않은 것과 그 이유

- **재질 파라미터 cue를 넣지 않았다.** G06이 `PawnMaterialParam`의 값·곡선·대상 slot 의미를
  풀지 못했다. 소비 매핑이 불명한 필드를 스키마에 넣으면 소비자 없는 인터페이스가 된다.
- **socket local transform 필드를 넣지 않았다.** 확인해보니 bone anchor는 **effect 문서가 소유**한다.
  `EFFECT_LEVEL_PLACEMENT_SPAWN_DESC`에 bone 이름 필드 자체가 없고, `pAnchorOwner`는 모델만
  공급하며 `Effect.SourceAnchorRequests`가 effect 문서에서 온다
  (`Effect_PresentationService.cpp:6241`). 따라서 cue에 socket transform을 넣어도 소비자가 없다.
  기존 `bone`/`followBone` 두 필드도 같은 이유로 소비자가 없는 상태 그대로 뒀다.
- **마하라카 cue 문서를 만들지 않았다.** 파티클 asset이 없고(G06 재질 15종 추출 차단),
  두 NPC는 idle만 재생하므로 `att_battle_1_01`을 트리거할 경로가 아직 없다.
- **F1 Debug 재생 명령을 만들지 않았다.** 설계서 G08-03 1번 항목이지만 `MainApp.cpp`는 오늘
  CP949 사고가 난 8,000줄 파일이고 다른 세션이 편집 중이다. 배관 우선 지시에 따라 분리했다.

## 5. 실행한 검사

### 5.1 정본 Product 빌드

```
Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
RUNNER_EXIT=0
```

- `error C####` / `fatal error` / `error LNK` **0건**. 로그의 LNK4099는 기존 DirectXTK PDB 경고다.
- receipt `out/BuildPipeline/runs/20260926T121841504Z-debug-product.json`: `result = PASS`,
  `missingRuntimeInputs = []`, `invalidRuntimeInputs = []`, `runtimeDataChecks` 38개 전부 PASS.
- OBJ 20개 재컴파일, CSO 0개(셰이더 무관), `Client.exe` 21:18:36 갱신.
  `Engine.dll`은 18:12:23 그대로 — Client만 바꿨으므로 정상이다.
- 빌드 전 `devenv.exe` 없음을 확인했고 잠금을 잡았다가 해제했다.

### 5.2 실제 reader 계약 검사 — 14 PASS / 0 FAIL

`out/MaharakaContinuation_20260926_193455/G08/g08_reader_test.cpp`는 **실제
`NpcActionEffectCueDocument.cpp`와 실제 `DataJson.cpp`를 링크**한다. reader가 소유하지 않는
두 협력자(`CEffectCatalog::Contains`, `CProjectDataRoot::Resolve`)만 스텁이고, 데이터 루트는
임시 폴더로 돌려 각 케이스가 자기 문서를 쓴다. vcxproj를 추가하지 않고 `cl`로 직접 빌드하며
중간 산출물과 exe가 전부 G08 폴더 안에 있다.

```
[PASS] A version 1 document still reads its rows unchanged
[PASS] An unsupported formatVersion is rejected
[PASS] A rejected document stays rejected on the next call
[PASS] A genuinely non-integral millisecond is rejected
[PASS] The source millisecond 2299.9999523 reads as 2300, not 2299
[PASS] A millisecond within rounding of a whole value reads as that value
[PASS] A repeated cue id is rejected
[PASS] A version 2 sound-only row is admitted without an effect asset
[PASS] A sound block in a version 1 document is rejected
[PASS] An alternate event without its condition is rejected
[PASS] A row with no effect and no sound is rejected
[PASS] A cue whose asset is absent from the catalog is dropped alone
[PASS] An archetype without a document loads as empty
[PASS] Rows are sorted by their source start time

npc action cue reader: 14 passed, 0 failed   EXIT=0
```

첫 실행은 13개 중 1개가 FAIL이었다. 원인은 코드가 아니라 **내 케이스 기대가 틀린 것**이었다.
2299.9999523은 반올림 오차 안이라 2300으로 받는 것이 의도된 동작인데 거부를 기대했다.
케이스를 진짜 비정수(1500.5)로 바꾸고, 원본 값이 2300이 되는지를 별도 케이스로 분리했다.

### 5.3 기존 v1 문서 실제 데이터 회귀

`Data/Effects/NpcActionCues/` 5개(`NPC_58700`, `NPC_59030`, `NPC_59060`, `NPC_59504`,
`NPC_59620`) 전부 새 규칙에 걸리지 않는다. 중복 ID 0, 비정수 ms 0, v1 sound 블록 0,
빈 effectAssetId 0. **새 규칙 때문에 거부되는 기존 문서는 없다.**

### 5.4 `git diff --check`

내 4개 파일에 공백 오류가 없다. 보고되는 5건은 전부 `Client/Private/MainApp.cpp`
7962·7972·7982·8026·8043행이며, 오늘 CP949 줄바꿈 삼킴을 고치려고 의도적으로 넣은 줄 끝 공백이다.
내 변경과 무관하므로 정리하지 않았다.

## 6. 실행하지 않은 것

- 게임·Client·UI를 실행·조작·캡처하지 않았다. 소리를 듣지 않았다.
- `Server.exe` contract test를 돌리지 않았다. Shared/Server를 바꾸지 않았으므로 대상이 아니다.
- Release 구성을 빌드하지 않았다. 제품 계약(Shared packet 등)을 바꾸지 않았다.
- 게시(publish)를 하지 않았다. 저작 데이터를 만들지 않았으므로 게시할 domain이 없다.

## 7. 사용자가 확인할 수 있는 것과 없는 것

**지금 화면에서 확인할 수 있는 마하라카 변화는 없다.** 마하라카 cue 문서가 없고 두 NPC는 idle만
재생하므로 새 경로가 실행되지 않는다. 이번 변경은 다른 NPC의 기존 v1 cue 경로에는 영향이 있다.

기존 경로의 회귀 여부만 확인하려면 이미 v1 문서를 가진 NPC 5종(`NPC_58700`, `NPC_59030`,
`NPC_59060`, `NPC_59504`, `NPC_59620`)이 나오는 기존 위치에서 그 NPC의 해당 클립이
전과 똑같이 보이는지 보면 된다. 새 F1 메뉴는 만들지 않았으므로 **없는 메뉴를 안내하지 않는다.**

## 8. 미완료와 다음 조사 위치

| 항목 | 막힌 이유 | 다음 위치 |
|---|---|---|
| 마하라카 cue 문서 | 파티클 asset 없음 + 트리거 없음 | G06 재질 15종 추출이 열린 뒤 |
| 재질 파라미터 cue | `PawnMaterialParam` 값·곡선·대상 slot 의미 미확정 | `CEFActionNotify_PawnMaterialParam` 클래스 레이아웃 |
| socket local transform | cue에 소비자 없음. effect 문서가 anchor 소유 | effect 문서의 `SourceAnchorRequests` 생성 경로 |
| Ground 축 | 1,500건에 2성분 사례 없음 | 다른 LOA의 2성분 notify |
| F1 Debug 재생 명령 | `MainApp.cpp` CP949 위험 + 타 세션 편집 중 | 설계서 G08-03 1번 |
| 서버 권위 occurrence | Server 이벤트가 아직 없음 | 설계서 G08-02 데이터 흐름 |
| 재질 slot fragment 함정 | 미해결로 남김 | `mn_ismp_00`이 3 slot 전부의 부분문자열이므로 `_mi` 포함 필요 |

재질 15종과 RNM 729 component가 **같은 뿌리 하나**에 막혀 있다. 이 Area의
`sourceMaterialBuild`가 `null`이고 source material compiler가 한 번도 돌지 않았다.

## 9. 다음 G를 시작해도 되는 근거

G08 배관은 닫혔고 빌드·계약 검사·실데이터 회귀로 확인했다. G09(무대)는 G02가 후보 0개라
독립적으로 막혀 있다. G06 재질과 G04 RNM은 같은 compiler 문제로 수렴하므로 그쪽이
다음 우선순위다. 마하라카 cue 문서 작성은 G06이 열린 뒤에 G08 배관 위에서 바로 가능하다.
