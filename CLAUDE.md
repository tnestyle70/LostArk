# CLAUDE.md

이 파일은 Claude Code(claude.ai/code)가 이 저장소에서 작업할 때 참고하는 LostArk 코드베이스 설명이다.
공통 에이전트 행동 규칙의 정본은 `AGENTS.md`이며, 작업 시작 시 먼저 읽는다.
계획서·설계서 요청은 `AGENTS.md`의 규칙 파일 탐색 순서를 따르고 `.md/GB/<MM-DD>/`에 PLAN/RESULT 문서를 작성한다.
LostArk 맵 에셋 검색·추출·`.wmodel` 변환·MapTool 적용 작업은 `.md/GB/07-29/2026-07-29_LOSTARK_MAP_ASSET_EXTRACTION_RUNTIME_RESULT.md`를 먼저 읽는다.

@AGENTS.md

## 프로젝트 개요

**LostArk** — C++로 직접 작성한 DirectX 11 3D 게임 엔진 프레임워크와 그 위에서 동작하는 클라이언트. **여러 명이 함께 작업하는 팀 프로젝트**이며, 엔진은 DLL로 빌드되어 클라이언트 EXE가 이를 링크해 사용한다.

혼자 쓰는 저장소가 아니다. 아래 "팀 협업 규칙"을 빌드/아키텍처만큼 중요하게 다룰 것.

## 팀원 최초 세팅

새 팀원이나 새 PC는 다음 순서로 시작한다.

```powershell
git lfs install
git clone <repository-url>
Set-Location LostArk
git lfs pull

# 팀 Drive에서 lock과 같은 버전의 ZIP을 받은 뒤 외부 pack root에 압축 해제
tar.exe -xf <lostark-resources-version.zip> -C <external-pack-root>

powershell -ExecutionPolicy Bypass -File Tools/AssetPipeline/Manage-ResourcePack.ps1 `
  -Mode Hydrate -PackRoot <external-pack-root>
powershell -ExecutionPolicy Bypass -File Tools/AssetPipeline/Manage-ResourcePack.ps1 `
  -Mode Verify
```

ZIP은 `<external-pack-root>/lostark-resources/<version>/{READY,manifest.json,payload}` 구조로 풀려야 한다. `Data/AssetPacks.lock.json`의 version, manifest hash, content hash와 다르면 사용하지 않는다. ZIP을 저장소 안에 풀거나 `Client/Bin/Resources`에 직접 덮어쓰지 않는다.

세팅 후 Debug 정본 회귀를 한 번 실행한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 `
  -Configuration Debug -DeepAssetHash
```

팀 문서의 단일 입구는 `.md/TEAM/README.md`다. 역할별 시작 파일과 금지 경계는 그 폴더의 `TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`, Area별 optional layer와 MapTool 지원 범위는 `AREA_DATA_LAYER_GUIDE.md`를 따른다.

## 빌드

**Visual Studio 2022**에서 `Framework.sln`을 연다. 제품과 계약 검증에 사용하는 프로젝트는 다음과 같다.

- `Shared\Default\Shared.vcxproj` — Client/Server 공용 protocol 계약
- `Engine\Default\Engine.vcxproj` — `Engine.dll` + `Engine.lib` 생성
- `Client\Default\Client.vcxproj` — 게임 EXE 생성, `Engine.lib`에 링크
- `Server\Default\Server.vcxproj` — 서버 권위 world/room 실행 파일
- `Tools\NetworkProtocolHarness\Default\NetworkProtocolHarness.vcxproj` — protocol 회귀 하네스

(`Engine\External\imgui\examples\*`의 vcxproj들은 ImGui 원본에 딸려온 샘플이며 솔루션에 포함되지 않는다. 건드리지 않는다.)

### 빌드 순서 — 반드시 지킬 것

```
1) Engine 빌드
2) UpdateLib.bat [Debug|Release]   ← 인자 생략 시 Debug
3) Shared + NetworkProtocolHarness 빌드/실행
4) Server 빌드
5) Client 빌드
6) Lobby/Bern/Valtan smoke + ProjectAudit
```

Debug와 Release 바이너리는 서로 덮어쓰지 않도록 구성별 폴더에 생성한다.

- Engine: `Engine\Bin\Debug\`, `Engine\Bin\Release\`
- Client: `Client\Bin\Debug\`, `Client\Bin\Release\`
- Engine import library: `EngineSDK\lib\Debug\`, `EngineSDK\lib\Release\`

`UpdateLib.bat`은 선택한 구성의 Engine 산출물로 `EngineSDK\`와 Client runtime 폴더를 갱신한다.

- `Engine\Public\*.*` → `EngineSDK\inc\`
- `Engine\Bin\<Configuration>\*.lib` → `EngineSDK\lib\<Configuration>\`
- `Engine\ThirdPartyLib\*.lib` → `EngineSDK\lib\`
- `Engine.dll`, `fmod.dll`, 구성에 맞는 `assimp-vc143-mt(d).dll` → `Client\Bin\<Configuration>\`
- `Engine\Bin\ShaderFiles\*` → `EngineSDK\hlsl\` → `Client\Bin\ShaderFiles\`

`EngineSDK\`는 `.gitignore` 대상이다. **clean clone 직후에는 존재하지 않으므로 Client부터 빌드하면 반드시 실패한다.** 병렬 빌드(`/m`)로 한 번에 돌릴 때도 Engine → UpdateLib → Client 순서가 보장되지 않으면 race가 난다.

### 정리 스크립트

- `CleanBuild.bat` — `.vs`, `EngineSDK`, Engine/Client의 Debug·Release 산출물과 각 프로젝트의 `x64` 중간 산출물 삭제
- `CleanBuildV2.bat` — 위와 동일하며 이전 공용 출력 구조가 남긴 root exe/dll/pdb도 정리한다. `Resources`, `DataFiles`, `ShaderFiles`는 보존한다.

정본 자동화는 `Tools/Build/Invoke-BuildAndRegression.ps1`이다. Client 작업 디렉터리를 반드시 `Client/Default`로 고정하고 셰이더·리소스 전제조건을 먼저 검사한다. 개별 MSBuild를 직접 실행할 때도 위 순서를 그대로 따른다.

Loader worker에서 호출되는 shader/model/navigation/camera/character/part/Valtan factory는 modal dialog를 띄우지 않고 실패를 반환한다. 종료 시 cooperative cancellation과 `CancelSynchronousIo`를 순서대로 시도한다. 그래도 10초를 넘기면 `TerminateThread`로 손상된 process를 계속 실행하지 않고 `ERROR_TIMEOUT`으로 process fail-fast한다. smoke harness는 조기 종료나 report 누락을 실패로 판정한다.

## 팀 협업 규칙

### 저장소 · 리소스 배포 정책

리소스가 세 갈래로 나뉘어 있다. 어디에 속하는지 먼저 확인하고 파일을 추가할 것.

| 종류 | 위치 | 배포 경로 |
|---|---|---|
| 소스 · 프로젝트 파일 | `Engine/`, `Client/`, `*.sln/.vcxproj/.filters` | Git 일반 추적 |
| 프로젝트 데이터 정본 | `Data/`의 catalog, authoring, reference JSON/문서 | Git 일반 추적 |
| 필수 바이너리 입력 | `Engine/ThirdPartyLib/`, 승인된 `Client/Bin/DataFiles/` | **Git LFS** (`.gitattributes` 패턴) |
| 런타임 리소스 · 쿠킹 산출물 | `Client/Bin/Resources/{Fonts,Character,Deploy,Effect,Map,UI}` | 버전 고정 외부 팩 (`Data/AssetPacks.lock.json`) |

- clone 시 `git lfs install` 후 clone하거나, 이미 받았다면 `git lfs pull`을 실행해야 lib/DLL/DDS가 포인터가 아닌 실물이 된다.
- `Client/Bin/Resources/` 최상위에는 위 여섯 폴더만 허용한다. `Resources/LostArk`, `Models`, `Textures`, `SourceData`, `Sound` 래퍼를 다시 만들지 않는다.
- raw 추출물과 SourceData는 런타임 팩에 넣지 않는다. `Tools/AssetPipeline/Manage-ResourcePack.ps1`로 publish/hydrate/verify한다.
- 리소스 팩의 Snapshot은 manifest와 lock을 한 트랜잭션으로 취급하며 lock commit 전 실패한 orphan manifest를 제거한다. 상세 운영 순서는 `Tools/AssetPipeline/README.md`를 따른다.
- UI와 gameplay 설정 정본은 JSON이다. `.cfg`를 새로 만들거나 Resources에서 직접 읽지 않는다.
- 빌드 산출물(`exe/dll/lib/pdb/cso`), `.vs`, `EngineSDK`, `_work`, `out`, `imgui.ini`는 전부 ignore 대상이다. **커밋에 섞여 들어가지 않게 할 것.**
- 새 바이너리 자산을 추가할 때는 LFS 대상인지 Drive 팩 대상인지 먼저 판단하고, 애매하면 커밋하지 말고 물어본다.

### 브랜치 · PR

`main`이 정본이고, 작업은 별도 브랜치와 PR로 합친다. 사람 작업은 팀의 `feature/<주제>` 관례를 따르고 Codex 작업은 `codex/<주제>`를 사용한다. `main`에 직접 커밋하지 않는다.

### 프로젝트 파일(.vcxproj / .filters) 취급

- `.vcxproj`의 **설정(PropertyGroup, ItemDefinitionGroup, Import)과 고정 item은 사람이 관리하는 정본**이다.
- 소스 목록(`ClCompile`/`ClInclude`)과 `.filters`는 **물리 폴더 구조가 정본**이다. 새 파일에 필요한 항목만 등록하고 기존 필터를 재배치하지 않는다.
- 파일 추가/이동은 물리 경로를 먼저 정하고, 그에 맞춰 프로젝트에 등록한다. 남의 필터 구조를 임의로 재배치하지 않는다.

### 소스 파일 인코딩 — 중요

`Engine/`, `Client/` C++ 소스는 CP949, UTF-8(BOM 포함/미포함), ASCII가 혼재한다. 편집 전에 파일별 기존 인코딩을 감지하고 그대로 보존한다. 인코딩을 확신할 수 없는 기존 파일은 ASCII 구간만 최소 수정하며 파일 전체를 임의 변환하지 않는다. 새 C++ 파일은 UTF-8(BOM 없음)과 영문 주석을 기본으로 한다. 인코딩 일괄 변환은 별도 합의 작업으로만 수행한다.

반대로 `.md/GB/**/*.md` 문서와 이 파일은 UTF-8이다.

### 작업 문서

`.md/GB/<MM-DD>/`에 `YYYY-MM-DD_주제_PLAN.md` / `_RESULT.md` 형식으로 작업 계획과 결과를 남긴다. 규모 있는 작업을 할 때는 기존 문서를 먼저 읽고, 같은 형식으로 남긴다.

## 아키텍처

### 서브시스템 소유권

`CGameInstance`는 `DECLARE_SINGLETON` 기반의 **싱글턴 파사드**로, 모든 엔진 서브시스템을 `unique_ptr` 멤버로 소유하고 외부 호출을 각 서브시스템으로 중계한다.

| 영역 | 클래스 | 역할 |
|---|---|---|
| 그래픽 | `CGraphic_Device` | DX11 device/context, 스왑체인, RTV/DSV |
| 입력 | `CInput_Device` | DirectInput 키보드/마우스 폴링, 입력 차단 게이트 |
| 사운드 | `CSound_Manager` | FMOD 재생 (`_WIN64` 한정) |
| 타이머 | `CTimer_Manager` / `CTimer` | 태그별 타이머, 프레임 델타 |
| 레벨 | `CLevel_Manager` | 현재 레벨 갱신/렌더, 레벨 전환 |
| 원형 | `CPrototype_Manager` | 레벨 인덱스별 프로토타입 등록소 |
| 오브젝트 | `CObject_Manager` | 레벨별 레이어 맵, 프레임 갱신 구동 |
| 렌더링 | `CRenderer` | 렌더 그룹 큐 |
| 파이프라인 | `CPipeLine` | View/Proj 및 역행렬, 카메라 위치 |
| 조명 | `CLight_Manager` | 광원 등록, 디퍼드 라이팅 렌더 |
| 폰트 | `CFont_Manager` | SpriteFont 텍스트 출력 |
| 렌더 타깃 | `CTarget_Manager` | RenderTarget/MRT 관리, 디버그 출력 |
| 피킹 | `CPicking` | 마우스 레이 피킹 |
| 그림자 | `CShadow` | 그림자 광원 행렬 바인딩 |
| 절두체 | `CFrustum` | 월드/로컬 공간 컬링 판정 |

렌더 그룹 순서(`RENDERGROUP`): `PRIORITY → SHADOW → NONBLEND → NONLIGHT → BLEND → UI`

### 오브젝트 · 컴포넌트 계층

```
CPrototype (추상, enable_shared_from_this)
├── CComponent (추상)
│   ├── CTransform            — 월드 행렬, 이동/회전 헬퍼
│   ├── CVIBuffer (추상)      — 정점/인덱스 버퍼 기반 클래스
│   │   ├── CVIBuffer_Rect / CVIBuffer_Cell
│   │   └── CVIBuffer_Instance (공통 인스턴싱 기반)
│   ├── CTexture              — SRV 배열, DDS/WIC 로딩
│   ├── CShader               — FX11 이펙트 래퍼
│   ├── CModel / CMesh / CMaterial / CBone / CAnimation / CChannel
│   ├── CCollider (+ CBounding_Sphere / _AABB / _OBB)
│   └── CNavigation / CCell
└── CGameObject (추상)
    ├── CContainerObject / CPartObject   — 파츠 조립형 오브젝트
    ├── CUIObject, CCamera
    └── [Client] CCharacter, CPart_Body, CPart_Equipment, CNpc,
                 CValtan, CDeployPropObject, CMapAssetObject,
                 CMapStaticBatchObject, CEffect_Runtime, CCamera_Free
```

### 프로토타입 패턴

- **`Create()`** → `unique_ptr<T>`. `CPrototype_Manager`에 보관되는 원본.
- **`Clone()`** → `shared_ptr<CPrototype>`. 레이어에 배치되는 실제 인스턴스.
- `Initialize_Prototype()`은 원본에서, `Initialize(void* pArg)`는 각 사본에서 호출된다.
- 프로토타입은 레벨 인덱스로 구분되며, `LEVEL::STATIC`(0)은 모든 레벨이 공유한다.

### 레벨 · 레이어

레벨은 `Client_Defines.h`의 enum이다.

```cpp
enum class LEVEL { STATIC, LOADING, LOBBY, BERN, VALTAN_ARENA, DEVELOPMENT, END };
```

시작 레벨은 항상 `LOBBY`다. 제품/개발 시나리오 정본은 `Data/Levels/LevelCatalog.json`이며 `CLevelCatalog`와 `CLevelRegistry`를 통해 해석한다. 임의의 Level enum, 문자열 분기, direct `Change_Level` 호출을 추가하지 않는다.

`LEVEL::STATIC`은 전환 시에도 살아남는 영구 레벨이고, 나머지는 `Change_Level`에서 정리된다. 각 레벨 인덱스는 `map<wstring_t, shared_ptr<CLayer>>`를 가지며, `CLayer`는 `list<shared_ptr<CGameObject>>`를 들고 매 프레임 `Priority_Update → Update → Late_Update`를 구동한다.

### 레벨 전환 흐름

제품/개발 전환 요청은 `CSceneTransitionService`에 제출하고 `CMainApp`만 `LOADING` 진입을 수행한다. `CLevel_Loading`만 로드 완료 후 목표 레벨로 commit한다. 로드는 `parse -> validate -> stage -> commit`이며 실패/취소 시 staging을 rollback한다.

`Data/Levels/LevelCatalog.json`의 `mapLoadBounds`가 제품 맵 로딩 범위의 정본이다. Bern과 Valtan 제품 레벨은 자신의 진입/전투 범위와 배경만 로드하고, `dev.map.active`만 전체 맵을 연다. Loader와 `CMapPlacementRuntime`은 같은 `MAP_LOAD_SCOPE`를 소비해야 하며 한쪽만 필터링하면 안 된다. 로더 작업 스레드의 실패는 상태와 HRESULT로 반환하고 `MessageBox`로 대기시키지 않는다.

### 서버 권위 월드 파이프라인

MapTool의 현재 지원 범위인 player spawn/NPC/boss 배치는 `Data/Worlds/<AreaId>/Gameplay.world.json`에 stable placement ID로 저장한다. `Tools/WorldPipeline/Publish-WorldGameplay.ps1`이 actor/encounter 참조를 검증한 뒤 `Server/Bin/DataFiles/World/*.worldbootstrap`을 원자적으로 생성하며 Server pre-build가 이 publish를 강제한다. 수업용 Monster 경로와 빈 미래용 Monster catalog/schema는 이 계약에 포함하지 않는다.

Server는 fixed 30 Hz에서 world entity의 transform/action/pattern state를 소유하고 Shared protocol v5 snapshot으로 보낸다. Client의 `CClientReplication`과 `CValtan`은 표현만 담당한다. UI·MapTool·Client GameObject가 제품 보스 판정을 직접 결정하지 않는다.

### 최소 수련장 Area

`dev.training.ground`는 새 Engine Level이 아니라 기존 `LEVEL::DEVELOPMENT`를 사용하는 시나리오다. map area는 `LV_DEV_TRAINING_GROUND`, world ID는 `TRAINING_GROUND`다. Lobby의 Bern/Valtan/Training은 ImGui에서 `Local Preview`와 `Multiplayer`를 명시적으로 고르며 기본 선택은 Local이다. Local은 socket을 열지 않고 `Data/Worlds/<AreaId>/Gameplay.world.json`의 placement ID 정렬상 첫 enabled `playerSpawn`에 선택 class의 presentation-only `CCharacter` 하나를 만든다. 이 Character는 `CNetObjectRegistry`와 server ID를 사용하지 않으며 command sink, 이동·스킬·damage·boss authority도 없다. Multiplayer는 입력한 IPv4 또는 `localhost`와 port에 연결하고 서버 승인을 받은 뒤에만 전환한다. 연결 실패·거부 또는 5초 이내 승인 부재는 Lobby에 남고, 진입 후 연결이 끊기면 replicated state를 제거한 뒤 Lobby로 복귀하며 Local로 자동 전환하지 않는다.

같은 PC의 Server는 기본 실행 후 Client에 `127.0.0.1:7777`을 입력한다. 같은 LAN의 다른 PC를 받으려면 Server PC에서 `Server.exe --bind-address 0.0.0.0`으로 실행하고, 다른 Client는 그 PC의 사설 IPv4와 `7777`을 입력한다. 특정 interface만 열려면 `0.0.0.0` 대신 해당 사설 IPv4를 사용한다. Windows Firewall inbound 허용은 별도로 필요하다. 자동 LAN 서버 탐색은 현재 계약이 아니며, 필요하면 UDP advertisement/discovery를 별도 protocol과 harness로 추가한다.

- visual admission: `LV_DEV_TRAINING_GROUND.mapassets`의 RCArena 10종만 로드
- visual placement: authoring 18개를 publisher가 runtime placement로 승격
- gameplay: 클래스 중립 `playerSpawn` 4개만 저장하며 `archetypeId`는 `null`
- navigation: `Data/Navigation/LV_DEV_TRAINING_GROUND.navgrid.json`에서 32×32 runtime grid를 결정적으로 생성
- runtime: `CClientReplication -> CPlayerController -> IPlayerCommandSink`와 `CCombatHUDViewModel`을 사용
- online smoke: player spawn, Q command, Server action 승인, cooldown snapshot과 HUD 반영까지 확인
- offline smoke: `Tools/Build/Invoke-OfflineClientSmoke.ps1`이 7777 listener 부재, local Character/class/placement, camera follow, network command sink 0개, Valtan network entity 부재를 확인
- LAN/disconnect smoke: `Tools/Build/Invoke-NetworkEndpointSmoke.ps1`이 Server PID의 정확한 `0.0.0.0:7777` 소유, 현재 LAN IPv4 실접속, Server 종료 후 Lobby 복귀와 online sink 정리를 확인

`playerSpawn`은 자리와 transform만 소유한다. 실제 character class는 Lobby/session 선택과 `C2S_ENTER_WORLD`가 소유하며 MapTool/world JSON이 특정 클래스를 고정하지 않는다.

Lobby에는 Lance Master, Gunslinger, Slayer, Artist 네 slot이 보이며 네 class 모두 immutable resource pack, Client Loader/Spec, Server player profile까지 연결되어 Bern/Valtan/Training에 진입할 수 있다. class별 스킬은 `PlayerSkills.json`에 실제 Server 계약이 있는 항목만 HUD와 입력에 노출하며, 누락 class를 Lance Master로 대체하지 않는다.

Area Loader는 네 class binary를 전부 선로드하지 않는다. `CPlayableCharacterAssetService`가 선택 class를 먼저 admission하고 `CClientReplication`이 다른 class의 최초 spawn을 받을 때 같은 경로로 한 번만 추가한다. 이 경계를 우회하는 두 번째 model loader나 silent fallback을 만들지 않는다.

### 바이너리 에셋 파이프라인

런타임 모델 입구는 `CModel` 하나로 통합한다. `CModel::Create()`는 FBX를 Assimp로 읽고, `.wmodel`은 `CWModelDecoder`로 읽은 뒤 모두 기존 `CMesh / CMaterial / CBone / CAnimation`으로 변환한다.

- `CCookedModel`과 `CBinaryAssetObject` 경로는 제거됐다. 동등한 두 번째 런타임 모델 경로를 다시 만들지 않는다.
- `Engine/Public/BinaryAsset/`의 `CBinaryReader`, `IModelDecoder`, `CWModelDecoder` 등 decode 기반 코드는 `CModel` 내부의 `.wmodel` 입력을 지원한다.
- 신규 맵·캐릭터·보스 모델은 `CLoader -> CModel Prototype -> GameObject의 CModel Component` 계약을 사용한다.
- `.wmodel` 머티리얼에 diffuse와 emissive가 모두 없으면 `CMaterial`이 1×1 회색 diffuse를 만들어 형상 확인을 보장한다. 이는 안전망일 뿐이며 최종 에셋은 실제 텍스처 경로를 가져야 한다.
- 추출·스케일·텍스처 복구의 상세 주의사항은 `.md/GB/07-29/gotchas.md`를 따른다.

### 런타임 에셋 루트

`CRuntimeAssetRoot::Get()`이 평탄화된 런타임 Resources 루트를 해석한다.

1. 환경 변수 `LOSTARK_RESOURCE_ROOT`가 있으면 그 경로
2. `LOSTARK_SHARED_ASSET_ROOT`가 있으면 그 경로 자체. 구형 `.../LostArk` 래퍼를 자동 보정하지 않는다.
3. 없으면 `<exe 폴더>/Resources` 또는 그 부모의 `Resources`

팀원마다 팩 위치가 다를 수 있으므로 **경로를 코드에 하드코딩하지 말고 `CRuntimeAssetRoot::Resolve()`를 쓴다.** asset ID는 Resources 상대 경로이며 절대 경로와 `..` 탈출은 거부한다.

### 디버그 툴 (ImGui / MapTool)

`_DEBUG`에서 `CMainApp`이 전역 Developer Tools 허브를 소유하고 F1로 토글한다. F6는 gameplay camera의 follow/free mode를 전환하며 free mode에서는 gameplay command를 보내지 않는다. Free camera는 WASD 이동, Tab mouse-look 전환을 사용한다. F2~F5와 F7~F12를 레벨/도구 전환에 사용하지 않는다. ImGui가 입력을 가져갈 때는 `CGameInstance::SetInputBlocked()`로 DirectInput 폴링을 막는다. 자동 검증은 `--smoke --scenario=<stable-id> --timeout-ms=<ms> --report=<path>`만 사용한다. 구형 별칭이나 추가 F키 이동을 다시 만들지 않는다.

네 class binary 진입 검증은 smoke 전용 `--character-class=lance-master|gunslinger|slayer|artist`를 사용한다. 정식 사용자는 Lobby UI에서 선택하며 이 옵션을 gameplay shortcut으로 사용하지 않는다.

### 새 GameObject 추가

1. `Client/Public`, `Client/Private`에 `CGameObject` 파생 클래스를 만든다.
2. `Initialize_Prototype`, `Initialize(void*)`, 갱신/렌더 가상 함수, `Clone`을 구현한다.
3. `CLoader::Ready_For_Level_*`에서 `CGameInstance::Get().Add_Prototype(레벨인덱스, TAG, T::Create(...))` 호출.
4. 대상 `CLevel::Initialize`에서 `Add_GameObject_to_Layer(원형레벨인덱스, TAG, 레이어레벨인덱스, 레이어태그)` 호출.
5. `Render()`에서 `Add_RenderObject(RENDERGROUP::NONBLEND, shared_from_this())` 호출.

### 새 Component 추가

1. Engine 쪽에 `CComponent`(또는 `CVIBuffer`) 파생 클래스를 만든다.
2. `Create()`는 `unique_ptr`, `Clone()`은 `shared_ptr<CPrototype>`를 반환한다.
3. `Add_Prototype`으로 등록하고, `CGameObject`의 템플릿 `Add_Component<T>()`로 붙인다.
4. **공개 헤더(`Engine/Public/`)를 건드렸다면 `UpdateLib.bat`을 다시 돌려야 Client가 새 헤더를 본다.** 공개 API 변경은 팀 전체에 영향을 주므로 반드시 공유한다.

## 주요 매크로 · 타입

| 매크로 / 타입 | 의미 |
|---|---|
| `NS_BEGIN(Engine)` / `NS_END` | `namespace Engine` 열기/닫기 |
| `DECLARE_SINGLETON(T)` | 복사 금지 + `static T& Get()` 정의 |
| `ENGINE_DLL` | `__declspec(dllexport/import)` 전환 |
| `FAILED_CHECK(hr)` | 실패 시 MessageBox 후 `E_FAIL` 반환 |
| `NULL_CHECK(_ptr)` 계열 | 널 검사 후 반환(메시지 버전 포함) |
| `MSG_BOX("msg")` | `MessageBox` 래퍼 |
| `ETOI` / `ETOUI` | enum → `int32_t` / `uint32_t` 캐스팅 |
| `f32_t` / `f64_t` / `bool_t` | `float` / `double` / `bool` |
| `char_t` / `tchar_t` / `wstring_t` | `char` / `wchar_t` / `wstring` |
| `float2_t` ~ `float4_t`, `float4x4_t` | `XMFLOAT2`~`XMFLOAT4`, `XMFLOAT4X4` |
| `vector_t` / `fvector_t` | `XMVECTOR` / `FXMVECTOR` |
| `matrix_t` / `fmatrix_t` | `XMMATRIX` / `FXMMATRIX` |

## 명명 규칙

- 클래스: `C` 접두사 — `CGameObject`, `CTransform`
- 멤버: `m_` 접두사 — `m_pDevice`, `m_Components`
- 프로토타입 태그: `Add_Prototype` / `Clone_Prototype`에 넘기는 와이드 문자열 — 예 `TEXT("Prototype_GameObject_BackGround")`
- 컴포넌트 태그: `m_Components`의 키 — 예 `TEXT("Com_Transform")`
- 레이어 태그: 레이어 맵의 키 — 예 `TEXT("Layer_BackGround")`

## 리소스 경로 규칙

클라이언트 실행 파일 기준 상대 경로를 사용한다.

- 셰이더: `../Bin/ShaderFiles/Shader_*.hlsl`
- 프로젝트 데이터: `CProjectDataRoot::Resolve()`로 `Data/` 정본을 해석한다.
- 전투 수치: `Data/Balance/PlayerProfiles.json`, `PlayerSkills.json`, `DamageProfiles.json`, `BossProfiles.json`이 정본이다. Server pre-build의 `Publish-GameplayBalance.ps1`만 runtime bootstrap을 생성한다.
- Git 관리 대상 `Data` 원본은 `Client.vcxproj`에서 `96.DataFiles`의 `None` 항목으로 보인다. 이는 탐색용 링크이며 runtime 복사나 두 번째 정본이 아니다.
- 현재 밸런스 검증은 JSON publish 후 Server 재기동과 `dev.training.ground` smoke로 수행한다. 무중단 Hot Reload는 아직 활성화하지 않으며 revision과 Server tick-boundary commit 없이 Client만 재읽지 않는다. 상세 계약은 `.md/TEAM/BALANCE_TUNING_AND_HOT_RELOAD_CONTRACT.md`를 따른다.
- 서버 길찾기: 신규 Area는 `Data/Navigation/<AreaId>.navgrid.json` authoring이 정본이며 `Publish-ServerNavigation.ps1`이 runtime navgrid를 결정적으로 생성한다. 기존 Valtan binary source도 같은 publisher가 검증하며 gameplay spawn/boss의 walkable cell·높이 정합성까지 확인한다.
- 런타임 리소스: `CRuntimeAssetRoot::Resolve("Character/..."|"Map/..."|...)`를 사용한다.
- 애니메이션 작성 데이터: `Data/Animation/Authored/<AssetId>/`
- 애니메이션 추출 참조: `Data/Animation/Reference/<AssetId>/`
- MapTool 작성본: `Data/Maps/Authoring/<AreaId>/`; publish 후에만 `Client/Bin/DataFiles/Map/` 런타임 입력이 된다.
- shard-set을 포함한 visual placement publish는 `Tools/MapPipeline/Publish-MapAuthoring.ps1`만 수행한다. 여러 shard placement와 mapset은 한 트랜잭션으로 교체되고 중간 실패 시 전부 rollback한다.

## 팀 작업 인터페이스

<!-- team-contract: vertical-slice-feature-owner; roles-are-not-file-permissions -->

역할 이름은 시작점과 권위 경계를 나타내며 파일 수정 권한을 제한하지 않는다. 기능 담당자는
요청된 동작이 실제로 실행되도록 필요한 Data, Shared, Server, Client, UI와 harness를 한 수직
슬라이스로 연결한다. 예를 들어 Player/Input 담당자가 서버 권위 스킬을 추가한다면 Server 판정과
snapshot까지 직접 구현해야 하며, Server 담당 파일이라는 이유로 Client command만 남기지 않는다.
표의 금지 경계는 다른 폴더를 수정하지 말라는 뜻이 아니라 계층을 우회하지 말라는 뜻이다.

| 담당 | 읽는 계약 | 쓰는 계약 | 금지 경계 |
|---|---|---|---|
| UI | layout JSON, `CCombatHUDViewModel` | lobby/scene/gameplay command service | packet/snapshot 파싱, Character 직접 변경 |
| Character/Animation | transform·locomotion·action 의미, `CHARACTER_SPEC` | visual/presentation 결과 | DirectInput, socket, damage·cooldown 결정 |
| Map/Encounter | asset·actor catalog, navigation 계약 | visual placement, player spawn/NPC/boss placement | runtime NetEntityId, HP, phase, prototype tag 저장 |
| Server/Boss | world bootstrap, player truth, encounter profile, fixed tick | server state, snapshot, semantic action | model·clip·texture·camera·ImGui 참조 |
| Transport | byte/frame | `RoomCommand` enqueue, frame send | `GameRoom` 상태 직접 변경 |

- 로컬 입력의 현재 제품 경계는 `CPlayerController -> IPlayerCommandSink`다. Controller는 transport를 모르고 Character는 입력을 읽지 않는다.
- `ICharacterLogic::Update_Presentation`은 표현 전용이다. `Logic_*`에서 `Play_Skill`을 직접 호출하지 않는다.
- quick slot → skill ID는 `Data/Balance/PlayerSkills.json`의 `inputSlot`이 정본이고 `CPlayerSkillCatalog`가 파싱해 `CPlayerController`와 `CCombatHUDViewModel`이 함께 읽는다. Controller는 슬롯 이름과 물리 키만 알고 skill ID를 하드코딩하지 않으므로, 다른 class의 스킬을 JSON에 추가하면 코드 변경 없이 바인딩된다. 제출 경로는 `C2S_USE_SKILL -> GameRoom -> CPlayerSkillSystem -> S2C_WORLD_SNAPSHOT`이다. 새 스킬은 balance 정의, Shared/Server 계약, presentation, harness를 함께 추가할 때만 활성화하고 로컬 우회 재생하지 않는다.
- UI는 `CCombatHUDViewModel`에서 server tick, HP/resource, action, cooldown end tick, boss HP/phase/action을 읽는다. UI가 cooldown이나 damage를 자체 판정하지 않는다.
- 현재 World Gameplay kind는 `playerSpawn`, `npc`, `boss`뿐이다. 수업용 Monster 구현과 빈 미래용 Monster 계약은 포함하지 않는다.
- Area별 레이어 보유 현황과 생략 규칙은 `.md/TEAM/AREA_DATA_LAYER_GUIDE.md`가 정본이다. 현재 일반 Monster, wave/증분 spawn, trigger, Area별 balance override, 제품 NPC presentation은 구현되지 않았다.

## 작업 방식 지침

### 1. 코딩 전에 생각한다
추측하지 않는다. 헷갈리는 부분을 숨기지 않는다. 트레이드오프를 드러낸다.

구현 전에:
- 전제를 명시한다. 확신이 없으면 묻는다.
- 해석이 여러 가지면 전부 제시한다. 혼자 골라 넘어가지 않는다.
- 더 단순한 방법이 있으면 말한다. 필요하면 반대 의견을 낸다.
- 불분명하면 멈춘다. 무엇이 불분명한지 이름 붙여 묻는다.

### 2. 단순함이 먼저다
문제를 푸는 최소한의 코드만 쓴다. 추측성 코드는 쓰지 않는다.

- 요청받지 않은 기능은 만들지 않는다.
- 한 번만 쓰는 코드에 추상화를 두지 않는다.
- 요청받지 않은 "유연성", "설정 가능성"을 넣지 않는다.
- 일어날 수 없는 상황에 대한 예외 처리를 넣지 않는다.
- 200줄을 썼는데 50줄로 될 일이면 다시 쓴다.
- "선임 개발자가 이걸 과하다고 할까?" 그렇다면 단순화한다.

### 3. 수술하듯 최소 변경 — 팀 작업에서 특히 중요
꼭 필요한 곳만 건드린다. 내가 만든 것만 치운다.

기존 코드를 수정할 때:
- 주변 코드, 주석, 서식을 임의로 "개선"하지 않는다.
- 고장나지 않은 것을 리팩터링하지 않는다.
- 내 취향과 달라도 주변 스타일에 맞춘다.
- 관련 없는 죽은 코드를 발견하면 알리기만 하고 지우지 않는다.
- **기능과 무관한 다른 레벨, 오브젝트, 공용 헤더는 손대지 않는다.** 반대로 기능 완성에 필요한
  Shared/Server/Client/Data 교차 수정은 생략하지 않으며, 변경 이유와 실행 검증을 같은 RESULT에 남긴다.
- 대량 변경(전체 서식 정리, 인코딩 변환, 파일 이동)은 merge 충돌을 만든다. 요청받은 경우에만 한다.

변경으로 고아가 생겼을 때:
- **내 변경 때문에** 안 쓰이게 된 include/변수/함수는 제거한다.
- 원래 있던 죽은 코드는 요청 없이 제거하지 않는다.
- 기준: 바뀐 모든 줄이 요청과 직접 연결되어야 한다.

### 4. 목표 기반 실행
성공 기준을 정의하고, 검증될 때까지 반복한다.

작업을 검증 가능한 목표로 바꾼다:
- "검증 추가" → "잘못된 입력에 대한 확인 절차를 만들고 통과시킨다"
- "버그 수정" → "재현 절차를 먼저 만들고, 그다음 고친다"
- "X 리팩터링" → "전후 동작이 같음을 확인한다"

다단계 작업은 짧은 계획을 먼저 밝힌다.
1. [단계] → 검증: [확인 방법]
2. [단계] → 검증: [확인 방법]

이 프로젝트에는 자동화된 테스트 스위트가 없다. 검증은 대개 **빌드 성공 + 실제 실행 확인**이므로, 무엇을 어떻게 확인했는지(빌드 구성, 실행한 레벨, 눈으로 본 결과) 명시한다.

## 프로젝트 고유 규칙

- 실시간 게임 엔진이다. 성능이 중요하다 — 갱신/렌더 루프에서 힙 할당을 피한다.
- `new`/`delete` 직접 사용 금지. 스마트 포인터, `ComPtr`, `Safe_Delete`/`Safe_Release`를 쓴다.
- 서브시스템 소유권이 헷갈리면 위 "서브시스템 소유권" 표를 본다.
- 기존 소스는 주변 주석의 언어와 파일별 인코딩을 그대로 맞춘다. 저장소 전체가 하나의 인코딩이라는 가정을 금지한다.
- 빌드가 깨진 채로 커밋하지 않는다. `Engine/Public/` 변경 후에는 `UpdateLib.bat` → Client 빌드까지 확인한다.
