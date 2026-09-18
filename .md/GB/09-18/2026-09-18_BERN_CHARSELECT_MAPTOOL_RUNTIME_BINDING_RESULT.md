# 2026-09-18 Bern / Character Select Map Tool 런타임 바인딩 RESULT

## 요청

Bern(`LEVEL::BERN` / `LV_BER_BERNCASTLE`)과 Character Select(`LEVEL::CHARACTER_SELECT` /
`LV_LOBBY_CLASSSELECT_SL00`)에 실제로 진입한 상태에서 F1 → Map Tool을 열면 그 Level이
로드한 맵이 편집기에 연결되어 Area가 잡히고 Navigation 탭이 활성화되게 한다. 이번 세션에
`LEVEL::VALTAN_ARENA`에 넣은 것과 같은 방식의 런타임 연결이며, Development workspace를
경유하는 우회 경로는 사용하지 않는다.

## 변경 전 동작 (코드 근거)

`Is_MapAuthoringLevel()`(`Client/Private/MapTool_Area.cpp`)은 DEVELOPMENT workspace이거나
`Runtime_AuthoringTargets().pPlacements != nullptr`일 때만 true다. 변경 전
`Runtime_AuthoringTargets()`는 `KAKULSAYDON_ARENA`와 `VALTAN_ARENA`만 채웠으므로 두 Level에서는
빈 `TARGET_SET{}`이 반환되었다. 그 결과 `Handle_LevelTransition`이
`m_iAuthoringLevelIndex = ETOUI(LEVEL::END)`(=8)로 두고 `m_EditorAreas.clear()`를 수행해
`Get_ActiveEditorArea()`가 nullptr가 되고, 화면에는

- `Level: 8 | Area: NO MAP AREA | Catalog: NOT READY` (`MapTool.cpp` Map Assets 패널)
- `Waiting for the isolated Development editor shell.` (`MapTool.cpp` workspace bar)
- `Navigation authoring is disabled for this Area.` (`MapTool_NavigationPanels.cpp`)

가 표시되었다. 두 Area의 `navigationPolicy`는 이전부터 `SOURCE_PAINT`로 선언되어 있었으므로
정책 문제가 아니라 Area 자체가 연결되지 않은 것이다.

`git log -S"LEVEL::BERN" -- Client/Private/MapTool_Area.cpp Client/Private/MapTool.cpp`에
이력이 없어 두 Level의 런타임 연결은 과거에도 존재한 적이 없다. 회귀가 아니라 미지원이었다.

## 변경 내용

### `Client/Public/Level_Bern.h` (+14줄)

- `#include "DeployPropRuntime.h"` 추가.
- 기존 `#ifdef _DEBUG` 블록(`Get_DebugCamera` 옆)에 `Get_MapAuthoringRuntime()`,
  `Get_MapAuthoringDeploy()`, `Get_MapAuthoringDevice()`, `Get_MapAuthoringContext()` 추가.
  `CLevel_ValtanArena`의 같은 이름 접근자와 시그니처가 동일하다.
- `m_MapRuntime` 옆에 `#ifdef _DEBUG` 가드된 `CDeployPropRuntime m_MapAuthoringDeploy;` 멤버 추가.

### `Client/Public/Level_CharacterSelect.h` (+14줄)

같은 include / 접근자 4개 / 멤버 1개를 동일한 형태로 추가. 접근자는 기존
`Debug_ReloadFloorSwapOptions` 앞의 `#ifdef _DEBUG` 블록 안에 넣었다.

### `Client/Private/MapTool_Area.cpp` (+약 60줄, 그 외는 기존 미커밋 발탄 변경)

- `#include "Level_Bern.h"`, `#include "Level_CharacterSelect.h"` 추가.
- `Runtime_AuthoringTargets()`에 `LEVEL::BERN`과 `LEVEL::CHARACTER_SELECT` 분기 추가. 각각
  `Get_MapAuthoringRuntime().Get_Catalog()` / `.Get_MutablePlacements()` /
  `Get_MapAuthoringDeploy()` / device / context를 채운다. 발탄 분기와 같은 형태다.
- `Authoring_Batches()`에 두 Level 분기 추가
  (`Get_MapAuthoringRuntime().Get_AuthoringBatches()`). 이것이 없으면 runtime attach 상태에서
  배치 컨테이너만 tool 소유 빈 벡터를 가리켜 batch 기반 placement와 어긋난다.
- `Can_ReplaceRuntimeAuthoringTargets()`의 반환을 `VALTAN_ARENA || BERN || CHARACTER_SELECT`로
  확장. 세 Level 모두 정지시킬 arena 소유 World Sequence / Composition 재생이 없다.
- placement ID 불일치 메시지의 `re-enter Kouku` → `re-enter this level`.

`Apply_RuntimeAuthoringActive()`와 `Rebase_RuntimeMotions()`는 쿠크 전용 early-return / 게이트를
그대로 두었다. 맵 self motion은 쿠크 arena만 구동하므로 두 Level에 해당 사항이 없다.

### `Client/Private/MapTool.cpp` (1줄)

workspace bar의 `"Editing the current Kouku runtime map. ..."` → `"Editing the current level's
runtime map. ..."`. 이 파일의 나머지 변경(`Update_EditorCutscene` 호출 이동, 쿠크 게이트,
`Apply_RuntimeAuthoringActive` 리팩터)은 이번 작업 이전의 미커밋 변경이며 건드리지 않았다.

### `.md/TEAM/AREA_DATA_LAYER_GUIDE.md` (+12줄)

`다른 제품 Level과 Release에는 이 편집 경로를 열지 않는다` 문장을 걷어내고, 네 Level이 같은
런타임 연결을 사용한다는 사실과 두 신규 Level의 제약(Deploy pair 미선언, arena 재생 없음,
Debug 전용)을 기술했다.

## 쟁점 판정

### (1) Deploy runtime 소유자 — Level 소유로 결정

`CWorldSequencePlayer::TARGET_SET::Is_Complete()`(`Client/Public/WorldSequencePlayer.h`)가
`pCatalog && pPlacements && pDeployRuntime` 세 포인터를 모두 요구하고,
`Switch_EditorArea`의 `runtimeAttach` 판정도 같은 세 포인터를 본다. 따라서 `pDeployRuntime`을
nullptr로 두면 `runtimeAttach == false`가 되어 tool이 live 컨테이너가 아닌 자기 사본을 편집한다.
이는 요청한 "실제 런타임 연결"이 아니다.

두 Area는 MapCatalog에 `sourceDeployCatalog` / `sourceDeployPlacements` 키가 없다(실측:
`LV_BER_BERNCASTLE`, `LV_LOBBY_CLASSSELECT_SL00` 모두 해당 키 없음. 반면 `LV_LUT_HEARTRB_ED`와
`LV_LUT_MIDNIGHTC_ED`는 있음). `Stage_DeployProps`는 두 경로가 모두 비면 `outRuntime`을 건드리지
않고 즉시 `true`를 반환하므로 빈 채로 유지된다.

후보는 두 가지였다.

- **Level 소유(채택)**: 발탄과 같은 형태. `Runtime_AuthoringTargets()`가 "live Level이 소유한
  deploy runtime"을 넘긴다는 의미가 유지된다. 나중에 두 Area가 Deploy pair를 선언하면 Level이
  소유한 컨테이너에 staging되어 정상 동작한다.
- **Tool 소유(기각)**: `Runtime_AuthoringTargets()`가 const 멤버라 `m_DeployRuntime`을 넘기려면
  `const_cast`가 필요하고, 그렇게 하면 `Authoring_Deploy()`의 attach/non-attach 두 분기가 같은
  객체를 가리켜 두 상태가 구분되지 않는다. 또 나중에 Deploy pair가 선언되면 staging 결과가 tool
  안에 갇혀 live 맵에 표시되지 않는 조용한 결함이 된다.

멤버와 접근자를 `#ifdef _DEBUG`로 가드해 Release에는 들어가지 않는다.

안전성 확인: `runtimeAttach` 경로는 `runtimeTargets.pDeployRuntime->Get_Catalog().Get_Assets()`를
순회한다. `Get_Catalog()`는 값 멤버 참조를 돌려주고 미로드 상태의 `Get_Assets()`는 빈 벡터이므로
루프가 0회 실행된다. `PreserveLiveDeployPresentation(live, staged)`도 `staged.Get_Entries()`를
순회하므로 빈 staged에서 즉시 `true`다.

### (2) placement ID 검사 — 통과 판정

`Switch_EditorArea`는 runtime attach일 때 live placement의 `(placementId, assetId)` 집합이
source 문서 전체와 일치하지 않으면 거부한다. Bern은 `MakeBernMapScope()`, Character Select는
`MakeFullMapScope()`를 사용한다(`Client/Private/LevelRegistry.cpp`). `MakeFullMapScope`는
bounds가 ±FLT_MAX이고 `includeBackground = true`, `excludedAssetGroupId`가 비어 있어
`MAP_LOAD_SCOPE::Contains()`가 유한 좌표 전부에 true를 돌려준다. 즉 scope 필터가 아무 행도
제거하지 않는다. `MakeBernMapScope`가 추가로 설정하는 `frustumCulling`은
`m_FrustumCulling`으로 저장되어 렌더 시점 정책으로만 쓰이며 placement 레코드를 제거하지 않는다.

또 `CMapPlacementRuntime`의 적재 루프는 마지막에
`return outPlacements.size() == records.size();`로 1:1 대응을 보장하고, 개별
`Create_Placement` 실패 시 전체 로드가 실패한다. 따라서 `Load_Area`가 성공한 Level에서는
live 집합과 scope 통과 레코드 집합의 크기가 항상 같다.

단, 이 검사는 **authoring source와 published runtime이 어긋나면** 설계대로 거부한다. Map Tool은
`Data/Maps/Authoring/...`을 읽고 Level은 자신이 로드한 맵을 들고 있으므로, 미게시 저작 변경이
쌓여 있으면 `Runtime/source placement IDs differ` 메시지가 나온다. 이는 쿠크/발탄과 동일한
기존 계약이며 이번 변경으로 완화하지 않았다.

**미확인**: 위는 코드 판정이며 실제 Bern 진입 시 통과 여부는 실행으로 확인하지 않았다.

## 검증

| 항목 | 결과 |
|---|---|
| 격리 구문 검사 `MapTool_Area.cpp` | EXITCODE=0, 오류 0건 |
| 격리 구문 검사 `MapTool.cpp` | EXITCODE=0, 오류 0건 |
| 격리 구문 검사 `Level_Bern.cpp` | EXITCODE=0, 오류 0건 |
| 격리 구문 검사 `Level_CharacterSelect.cpp` | EXITCODE=0, 오류 0건 |
| `git diff --check` | 출력 없음 |
| 인코딩 / 줄끝 | 4개 소스 모두 UTF-8 noBOM · CRLF 유지, lone LF 0 |
| `#ifdef`/`#endif` 균형 | 3개 파일 모두 일치 |

격리 컴파일 명령(저장소 IntDir/OutDir에 쓰지 않음, `/Zs` 구문 검사만):

```
cl /c /nologo /EHsc /std:c++20 /utf-8 /bigobj /Y- /MDd /Zs
   /D WIN32 /D _DEBUG /D _WINDOWS /D _UNICODE /D UNICODE
   /I "../Public/" /I "../../EngineSDK/Inc/" /I "../../Shared/Public/" <source>
```

`/Zs`는 코드 생성을 하지 않는다. 링크와 최종 산출물은 사용자의 Visual Studio 빌드로 확인한다.

## 사용자 확인 절차

1. Visual Studio에서 `Client` 프로젝트 `Debug|x64` **Build**(증분). 헤더 2개가 바뀌었으므로
   두 Level과 Map Tool 계열 TU가 재컴파일된다. `Rebuild`/`Clean`은 사용하지 않는다.
   실행 중인 `Client.exe`가 있으면 먼저 종료한다.
2. Server를 띄운 상태에서 Lobby → `Bern`(또는 `Character Select`) 진입.
3. F1 → Map Tool. 정상이면 `Level: 8 | Area: NO MAP AREA` 대신 실제 Level 인덱스와 Area ID가
   표시되고, workspace bar에 `Editing the current level's runtime map.`이 나타난다.
   `Waiting for the isolated Development editor shell.` 문구는 사라진다.
4. Navigation 탭이 활성화되면 Bake → Save. 저장 대상은 `Data/Navigation/<AreaId>.navsource`,
   `.navpaint`다.
5. Server 반영은 별도 실행이다.

```
powershell -ExecutionPolicy Bypass -File Tools/NavigationPipeline/Publish-ServerNavigation.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/NavigationPipeline/Publish-ServerNavigation.ps1 -Mode Publish
```

이후 Server 재시작. (스크립트의 실제 파라미터 이름은 실행 전 `-?`로 확인할 것. 이번 작업에서
publisher를 실행하지 않았다.)

## 두 Level에서 열리는 탭과 위험

| 탭 | Bern | Character Select | 비고 |
|---|---|---|---|
| Map Assets | 열림 | 열림 | visual placement 편집 / Save는 `Data/Maps/Authoring` |
| World Gameplay | Area의 `gameplayPolicy`에 따름 | 동일 | **위험**: Save가 `Gameplay.world.json`을 재작성한다 |
| World Destruction | 미선언이라 비활성 | 미선언이라 비활성 | |
| World Sequence | 문서 없음 | 문서 없음 | |
| Navigation | 열림 | 열림 | 이번 작업의 목표 |
| Camera | Area 선언에 따름 | Area 선언에 따름 | |

**가장 큰 위험은 World Gameplay 탭의 Save다.** 두 Area 모두 `gameplayDocument`를 선언하고 있어
탭이 열리며, Save는 `Data/Worlds/<AreaId>/Gameplay.world.json`을 전부 다시 쓴다. 과거 이 경로로
float32 정밀도 오염이 발생한 이력이 있고, Bern의 `Gameplay.world.json`은 현재 다른 작업(NPC 배치)이
편집 중이다. Navigation만 저작할 때는 이 탭의 Save를 누르지 않는다. 이번 변경은 이 위험을 새로
만들지 않았지만(탭 자체는 Area 선언으로 이미 결정됨) 두 Level에서 처음으로 접근 가능해졌으므로
경계로 남긴다.

Map Tool은 Server gameplay / collision / navigation runtime을 직접 바꾸지 않는다. Character
Select는 Server 승인 세션이 살아 있는 제품 Level이지만 이번 연결은 visual placement와 navigation
저작만 추가하며 replication 경로를 건드리지 않는다.

## 미확인 항목

- 실제 Client 실행으로 두 Level에서 Area가 붙는지, placement ID 검사가 통과하는지 확인하지 않았다
  (에이전트는 Client를 실행하지 않는다).
- Bern은 placement가 약 5만 건이라 연결 시 staging 비용이 든다. 실제 소요 시간 미측정.
- Navigation bake 후 publish와 Server 재시작을 실행하지 않았다.
- 링크 단계는 검증하지 않았다(`/Zs` 구문 검사만 수행).

## 이번 작업에서 내가 한 실수

1. 격리 컴파일 첫 시도에서 heredoc으로 작성한 `.bat`이 LF 줄끝이라 `call` 문이 깨졌다. CRLF로
   다시 썼다.
2. 두 번째 시도에서 Python 문자열에 `C:\Users\...`를 그대로 넣어 `\U` 이스케이프로 SyntaxError가
   났다. 세션 gotcha에 이미 있던 항목인데 반복했다. Write 도구로 raw string을 써서 해결했다.
3. 세 번째 시도에서 `_UNICODE`/`UNICODE` 정의를 빠뜨려 기존 코드의 `TEXT()` 매크로가 `char`로
   전개되며 오류가 났다. 내 변경과 무관한 오류였는데 원인 파악에 한 번 더 실행이 필들었다.
   vcxproj의 CharacterSet 기본값을 먼저 확인했어야 했다.
