# 2026-09-18 발탄 Map Tool 컷신 배우 재생 — 적대적 재검증 RESULT

앞선 fork 의 "고쳤다"는 주장을 근거로 쓰지 않고, 코드와 데이터를 직접 읽어 판정했다.

## 최종 판정

앞선 수정의 **진단과 구현은 유효하다.** 다만 같은 Level 안에서 툴이 Area 를 놓았다가
다시 잡는 경우에 **중복 등록으로 실패하는 결함**이 남아 있었고, 이번에 고쳤다.

## C1 프로토타입 미등록 — 성립

`CWorldSequenceObject::PROTOTYPE_TAG` 를 `Add_Prototype` 으로 등록하는 곳은 저장소 전체에서
두 곳뿐이다.

- `Client/Private/Level_KakulSaydonArena.cpp:1214` — 쿠크 레벨
- `Client/Private/MapTool_Cutscenes.cpp:65` — 이번에 추가된 헬퍼

`Client/Private/Level_ValtanArena.cpp` 의 `Add_Prototype` 호출은 **0건**이다. `Loader.cpp`,
`MainApp.cpp` 에도 이 태그 등록은 없다. 발탄 레벨에 프로토타입이 없었다는 진단은 사실이다.

## C2 실행 경로 — 성립

- 호출 지점 2곳: `MapTool_Cutscenes.cpp:147`(`Prepare_EditorCutsceneWorld`, `Load_Area`/`Play`
  **이전**), `:1301`(`Play_CardMiroMarch`).
- `:133` `CMapTool::Update` 가 매 프레임 `Handle_LevelTransition(currentLevelIndex, ...)` 호출.
- `MapTool_Area.cpp:1311` 에서 `targetLevelIndex == m_iAuthoringLevelIndex` 면 early return,
  즉 레벨이 바뀔 때만 리셋 본문이 돈다.
- `MapTool_Area.cpp:1384` `m_bWorldObjectPrototypeReady = false;` 가
  `:1387` `m_iAuthoringLevelIndex = targetLevelIndex;` **직전**에 있다. 순서 정확.
- 쿠크 건너뛰기 조건은 `ETOUI(LEVEL::KAKULSAYDON_ARENA) == m_iAuthoringLevelIndex` 이며
  발탄 인덱스(`VALTAN_ARENA`)에서는 거짓이다. 잘못 참이 되지 않는다.
- Stop → 재Play: 플래그는 레벨 전환에서만 리셋되고 헬퍼는 멱등하므로 성립.

## C3 등록 인자 — 성립

등록은 `m_iAuthoringLevelIndex`, 클론은 `WorldSequencePlayer_Objects.cpp:912-914` 의
`targets.levelIndex` 를 프로토타입 레벨과 레이어 레벨 양쪽에 쓴다.
`Build_CutsceneTargets` 가 `outTargets.levelIndex = m_iAuthoringLevelIndex` 로 채우므로 동일
인덱스다. device/context 도 같은 `m_pDevice/m_pContext` 를 쓴다.

## C4 나머지 의존성 — 충족

`CWorldSequenceObject::Initialize` 가 요구하는 것은 모델과 셰이더다
(`WorldSequenceObject.cpp:29-31`): skinned 이면 `Prototype_Component_Shader_VtxAnimMeshBinary`,
아니면 `..._VtxMeshBinary` 를 `desc.levelIndex` 아래에서 찾는다.

`Loader.cpp:608` `Ready_For_ValtanArena` → `:642` `Ready_Character_Rendering(VALTAN_ARENA, ...)`
→ `:1276` `Ready_AnimatedMeshShader(iLevelIndex)` → `:1188`
`Prototype_Component_Shader_VtxAnimMeshBinary` 를 **VALTAN_ARENA 인덱스에 등록**한다. 충족.

모델 파일은 `Client/Bin/Resources` 에 전부 실재한다(바이트 확인).

| asset | 크기 |
|---|---|
| `Character/Valtan/MN_RPBF_01.wmodel` | 11M |
| `Character/Valtan/AnimSets/MN_RPBF_01_AnimSet.wmodel` | 45M |
| `Character/Valtan/Ghost/MN_RPBF_02.wmodel` | 41M |
| `Character/Valtan/Ghost/MN_RPBF_02_AnimSet.wmodel` | 45M |
| `Character/Monster/NPC_480005_MN_RPRS_02/NPC_480005_MN_RPRS_02.wmodel` | 21M |

**확인 못 한 것**: `Attach_AnimationSet` 의 본 수 일치는 런타임 판정이라 정적으로 확인하지 못했다.

## C5 갱신·렌더 — 성립

배우는 `Add_GameObject_to_Layer` 로 `targets.levelIndex` 레이어에 들어가므로 Object_Manager 가
매 프레임 구동하고, `WorldSequenceObject.cpp:123` 에서 스스로
`Add_RenderObject(RENDERGROUP::NONBLEND, ...)` 한다. 별도 소유자가 필요 없다.
세션 시계는 `MapTool.cpp:193` `Update_EditorCutscene` → `MapTool_Cutscenes.cpp:297`
`m_fCutsceneSessionMs += ...` → `:308` `Seek_EditorCutsceneWorld()` → `Seek_AllToMs` 로 연결된다.

## C6 데이터 — 성립

`LV_LUT_HEARTRB_ED.worldsequences.json`: objectResources 3, templates 7, instances 7.
저작본과 런타임본(`Client/Bin/DataFiles/Map/`)의 SHA-256 앞자리가 `1b8d0501…` 로 동일하다.

컷신 5개의 `worldInstanceIds` 가 가리키는 instance 7개가 **전부 문서에 실재**한다.

| 컷신 | durationMs | worldInstanceIds |
|---|---|---|
| gate1-entrance | 13000 | gate1-entrance, gate1-entrance.black-wolf |
| entrance | 24708 | entrance, entrance.colorless |
| finale | 23000 | finale |
| trash | 6374 | trash |
| roar | 7003 | roar |

## C7 컴파일 — 통과

`out/IsolatedCompile20260918c`, `cl /c /Zs /std:c++20 /EHsc /D_DEBUG /D_WINDOWS /D_UNICODE
/DUNICODE`, 저장소 IntDir/OutDir 미사용.

| TU | 결과 |
|---|---|
| `MapTool_Cutscenes.cpp` (수정 대상) | EXIT=0 |
| `MainApp_WorldLevel.cpp` (`MapTool.h` 소비) | EXIT=0 |
| `MainApp_SequenceViewer.cpp` (`MapTool.h` 소비) | `/utf-8` 추가 시 EXIT=0 |

`MainApp_SequenceViewer.cpp` 는 BOM 없는 UTF-8 이라 기본 코드페이지(949)로는 C2001 이 난다.
소스 인코딩 문제이며 이번 변경과 무관하다.

## 발견한 결함과 수정

`CPrototype_Manager::Add_Prototype` 은 **같은 태그가 이미 있으면 `E_FAIL`** 을 반환한다
(`Engine/Private/Prototype_Manager.cpp:28`). 그리고 프로토타입은 `Change_Level` 의
`Clear(iClearLevelID)`(`Engine/Private/GameInstance.cpp:250`)에서만 지워진다.

재진입은 가설이 아니라 코드에 있는 경로다. `m_iAuthoringLevelIndex` 를 `LEVEL::END` 로
강제하는 지점이 두 곳 있고, 둘 다 Change_Level 이 아니다.

- `Client/Private/MapTool.cpp:93-94` — `SetOpen(true)` 시 런타임 attach 대상이 있으면 END 로
  되돌려 다음 프레임에 재바인딩을 유도한다. 즉 **F1 로 Map Tool 을 닫았다 열면** 발생한다.
- `Client/Private/MapTool.cpp:420-421` — `Retry runtime map binding` 버튼. 사용자가 Area 가
  안 잡힐 때 누르는 바로 그 버튼이다.

두 경우 모두 다음 프레임 `Handle_LevelTransition` 이 END → 원래 Level 로 전환하며 플래그를
리셋하지만, 엔진 프로토타입은 같은 Level 인덱스에 그대로 남아 있다.

따라서 툴이 **Change_Level 없이** 같은 Level 에서 Area 를 놓았다가 다시 잡으면
(`Is_MapAuthoringLevel()` 이 false → true 로 오갈 때) `m_bWorldObjectPrototypeReady` 만 리셋되고
엔진 프로토타입은 남아 있어, 재등록이 `E_FAIL` 로 떨어지고
`"World Object prototype registration failed for this Level."` 로 배우가 다시 사라진다.

`Client/Private/MapTool_Cutscenes.cpp:64-76` 수정:

- `ETOUI(LEVEL::END) <= m_iAuthoringLevelIndex` 인 경우만 실패로 처리(레벨 인덱스 범위 초과는
  `Add_Prototype` 이 거부하는 진짜 오류다)
- 그 외에는 `Add_Prototype` 결과를 `(void)` 로 흘리고 준비됨으로 표시. 이미 등록된 Level 은
  실제로 사용 가능한 상태이고, **진짜로 프로토타입이 없으면** `Apply_Objects` 의
  `"World Object clone/shader creation failed: ..."` 로 정확히 드러난다.

## 남은 실패 시나리오와 확인 위치

| 시나리오 | 화면 문구 |
|---|---|
| 프로토타입이 진짜 없음 | `World Object clone/shader creation failed: <objectId>` |
| World 문서 로드 실패 | `World Sequence load failed: ...` |
| 인스턴스 시작 실패 | `World instance could not start: <instanceId>` |
| 모델/애님셋 admission 실패 | `World actors stopped: ...` |
| Area 미확보 | `World actors need a loaded Area.` / `No active Area for the World actors.` |

## 검증 기록

- `git diff --check` 종료코드 0 (경고 2건은 다른 작업의 CLASSSELECT 런타임 JSON)
- `MapTool_Cutscenes.cpp` BOM 없음 · CRLF 1670 · lone LF 0 유지
- 커밋/푸시 없음, Client 실행 없음, 제품 빌드 없음
