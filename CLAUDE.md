# CLAUDE.md

이 파일은 Claude Code(claude.ai/code)가 이 저장소에서 작업할 때 참고하는 LostArk 코드베이스 설명이다.
공통 에이전트 행동 규칙의 정본은 `AGENTS.md`이며, 작업 시작 시 먼저 읽는다.
계획서·설계서 요청은 `AGENTS.md`의 규칙 파일 탐색 순서를 따르고 `.md/GB/<MM-DD>/`에 PLAN/RESULT 문서를 작성한다.
LostArk 맵 에셋 검색·추출·`.wmodel` 변환·MapTool 적용 작업은 `.md/GB/07-29/2026-07-29_LOSTARK_MAP_ASSET_EXTRACTION_RUNTIME_RESULT.md`를 먼저 읽는다.

@AGENTS.md

모든 세션은 작업 전에 `AGENTS.md`, 이 문서, `.md/GB/gotchas.md`, 있으면
`.md/GB/gotchas.local.md`, `.md/TEAM/README.md`, 대응 PLAN/RESULT를 읽는다. Artist F와
Client Effect 결과의 화면 조작·판정은 사용자 전용이며 아래 경계를 따른다.

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
```

팀장이 전달한 runtime 리소스를 `Client/Bin/Resources/{Fonts,Character,Deploy,Effect,Map,UI}` 물리 폴더에 둔다. Git pull 재현이 명시된 V1 Effect는 Product 문서가 실제 참조하는 선별 dependency closure를 Git/LFS로 함께 받는다. 별도 asset-pack lock, ZIP hash, publish/hydrate/verify 절차는 사용하지 않는다.

세팅 후 Debug 정본 회귀를 한 번 실행한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 `
  -Configuration Debug
```

팀 문서의 단일 입구는 `.md/TEAM/README.md`다. 역할별 시작 파일과 금지 경계는 그 폴더의 `TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`, Area별 optional layer와 MapTool 지원 범위는 `AREA_DATA_LAYER_GUIDE.md`를 따른다.

## 빌드

**Visual Studio 2022**에서 `Framework.sln`을 연다. 제품과 계약 검증에 사용하는 프로젝트는 다음과 같다.

- `Shared\Default\Shared.vcxproj` — Client/Server 공용 protocol 계약
- `Engine\Default\Engine.vcxproj` — `Engine.dll` + `Engine.lib` 생성
- `Client\Default\Client.vcxproj` — 게임 EXE 생성, `Engine.lib`에 링크
- `Server\Default\Server.vcxproj` — 서버 권위 world/room 실행 파일
- `Tools\NetworkProtocolHarness\Default\NetworkProtocolHarness.vcxproj` — protocol 회귀 하네스
- `Tools\EffectRenderContractHarness\Default\EffectRenderContractHarness.vcxproj` — Effect stage/commit, compiled shader, WARP 계약 하네스
- `Tools\PointLightFalloffContractHarness\Default\PointLightFalloffContractHarness.vcxproj` — Engine Deferred compiled shader 소비 계약 하네스

(`Engine\External\imgui\examples\*`의 vcxproj들은 ImGui 원본에 딸려온 샘플이며 솔루션에 포함되지 않는다. 건드리지 않는다.)

### 빌드 순서 — 반드시 지킬 것

```
1) Engine 빌드
2) UpdateLib.bat [Debug|Release]   ← 인자 생략 시 Debug
3) Shared + NetworkProtocolHarness 빌드/실행
4) Server 빌드
5) Client 빌드
6) EffectRenderContractHarness + PointLightFalloffContractHarness 빌드/실행
7) compiled shader closure/hash 검사
8) Lobby/Bern/Valtan smoke + 변경 domain publisher validation
```

Debug와 Release 바이너리는 서로 덮어쓰지 않도록 구성별 폴더에 생성한다.

- Engine: `Engine\Bin\Debug\`, `Engine\Bin\Release\`
- Client: `Client\Bin\Debug\`, `Client\Bin\Release\`
- Engine import library: `EngineSDK\lib\Debug\`, `EngineSDK\lib\Release\`

`UpdateLib.bat`은 선택한 구성의 Engine 산출물로 `EngineSDK\`와 Client runtime 폴더를 갱신한다.

- `Engine\Public\*.*` → `EngineSDK\inc\`
- `Engine\Bin\<Configuration>\*.lib` → `EngineSDK\lib\<Configuration>\`
- `Engine\ThirdPartyLib\*.lib` → `EngineSDK\lib\`
- `Engine.dll`, `fmod.dll`, 구성에 맞는 `assimp-vc143-mt(d).dll`, PhysX/PhysXCommon/
  PhysXFoundation 구성별 DLL → `Client\Bin\<Configuration>\`
- `Engine\Bin\<Configuration>\Shader_Cell.cso`, `Shader_Deferred.cso` →
  `Client\Bin\<Configuration>\`
- `Engine\Bin\ShaderFiles\*` → `EngineSDK\hlsl\` → `Client\Bin\ShaderFiles\`

Client 빌드는 Client가 소유하는 `Shader_*.hlsl`을 같은 구성의 module-adjacent `Shader_*.cso`로
생성한다. 제품 런타임은 이 CSO만 `D3DX11CreateEffectFromMemory`로 열며 HLSL source compile로
fallback하지 않는다. 누락·빈 파일·손상 bytecode·technique/pass/input-layout 불일치는 즉시 실패한다.
`Tools/Build/Test-CompiledShaderClosure.ps1 -Configuration <Debug|Release>`는 Engine/Client의 23개
producer와 Client/Effect/PointLight 소비 복사본의 존재 및 SHA-256 일치를 검사한다. CSO는 빌드
산출물이므로 Git에 커밋하지 않는다.

`EngineSDK\`는 `.gitignore` 대상이다. **clean clone 직후에는 존재하지 않으므로 Client부터 빌드하면 반드시 실패한다.** 병렬 빌드(`/m`)로 한 번에 돌릴 때도 Engine → UpdateLib → Client 순서가 보장되지 않으면 race가 난다.

### 정리 스크립트

- `CleanBuild.bat` — `.vs`, `EngineSDK`, Engine/Client의 Debug·Release 산출물과 각 프로젝트의 `x64` 중간 산출물 삭제
- `CleanBuildV2.bat` — 위와 동일하며 이전 공용 출력 구조가 남긴 root exe/dll/pdb도 정리한다. `Resources`, `DataFiles`, `ShaderFiles`는 보존한다.

정본 자동화는 `Tools/Build/Invoke-BuildAndRegression.ps1`이다. Client 작업 디렉터리를 반드시 `Client/Default`로 고정하고 셰이더·리소스 전제조건을 먼저 검사한다. 이 스크립트는 Client 뒤 두 shader 하네스를 빌드하고 compiled shader closure를 검사한 뒤 하네스를 실행한다. Effect 하네스는 Client에 비링크 ProjectReference를 두므로 clean/병렬 solution 빌드에서도 Client의 전체 CSO 생성이 끝난 뒤 복사한다. 개별 MSBuild를 직접 실행할 때도 위 순서를 그대로 따른다.

같은 working tree에서 Visual Studio와 자동화 빌드/publisher를 겹쳐 실행하지 않는다. 선언과 정의가 일치하는데
`LNK2019`가 발생하면 broad clean보다 먼저 선택한 `Configuration|Platform`의 evaluated `IntDir/OutDir`와 provider
`.obj`의 정의 심볼을 확인하고, 해당 translation unit만 강제 재컴파일한다. `LNK1104`, `MSB3021`, `MSB3027`의
대상이 EXE/DLL이면 실행 중 출력물 잠금이므로 compile 성공과 최종 link 실패, 실행 중인 이전 바이너리를 분리해
보고한다. Client 최종 link는 실행 중인 `Client.exe`를 사용자가 종료한 뒤 한 번만 수행한다.

Loader worker에서 호출되는 shader/model/navigation/camera/character/part/Valtan factory는 modal dialog를 띄우지 않고 실패를 반환한다. 종료 시 cooperative cancellation과 `CancelSynchronousIo`를 순서대로 시도한다. 그래도 10초를 넘기면 `TerminateThread`로 손상된 process를 계속 실행하지 않고 `ERROR_TIMEOUT`으로 process fail-fast한다. smoke harness는 조기 종료나 report 누락을 실패로 판정한다.

## 팀 협업 규칙

### 저장소 · 리소스 배포 정책

리소스가 세 갈래로 나뉘어 있다. 어디에 속하는지 먼저 확인하고 파일을 추가할 것.

| 종류 | 위치 | 배포 경로 |
|---|---|---|
| 소스 · 프로젝트 파일 | `Engine/`, `Client/`, `*.sln/.vcxproj/.filters` | Git 일반 추적 |
| 프로젝트 데이터 정본 | `Data/`의 catalog, imported, authoring, reference JSON/문서 | Git 일반 추적; 대용량 map 문서는 Git LFS |
| 필수 바이너리 입력 | `Engine/ThirdPartyLib/` | **Git LFS** (`.gitattributes` 패턴) |
| 실행 데이터 생성물 | `Client/Bin/DataFiles/`, `Server/Bin/DataFiles/` | `Data/`에서 publisher가 생성; 직접 편집 금지 |
| 런타임 리소스 · 쿠킹 산출물 | `Client/Bin/Resources/{Fonts,Character,Deploy,Effect,Map,UI}` | 기본은 팀장 관리 물리 폴더; 명시된 feature의 exact dependency closure만 Git/LFS |

- clone 시 `git lfs install` 후 clone하거나, 이미 받았다면 `git lfs pull`을 실행해야 lib/DLL/DDS가 포인터가 아닌 실물이 된다.
- `Client/Bin/Resources/` 최상위에는 위 여섯 폴더만 허용한다. `Resources/LostArk`, `Models`, `Textures`, `SourceData`, `Sound` 래퍼를 다시 만들지 않는다.
- raw 추출물과 SourceData는 runtime Resources에 넣지 않는다. 팀장이 채택한 쿠킹 결과만 물리 폴더에 둔다.
- UI와 gameplay 설정 정본은 JSON이다. `.cfg`를 새로 만들거나 Resources에서 직접 읽지 않는다.
- 빌드 산출물(`exe/dll/lib/pdb/cso`), `.vs`, `EngineSDK`, `_work`, `out`, `imgui.ini`는 전부 ignore 대상이다. **커밋에 섞여 들어가지 않게 할 것.**
- 새 바이너리 자산을 추가할 때는 LFS 대상인지 Drive 팩 대상인지 먼저 판단한다. pull-only 재현 feature는 현재 Product가 참조하는 최소 closure만 포함하고 전체 pack이나 미참조 자산을 커밋하지 않는다.

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
| 물리 | `CPhysics_Manager` | PhysX scene, generation actor handle, fixed-step과 post-physics pose 동기화 |

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
                 CMapStaticBatchObject, CCamera_Free
```

### 프로토타입 패턴

- **`Create()`** → `unique_ptr<T>`. `CPrototype_Manager`에 보관되는 원본.
- **`Clone()`** → `shared_ptr<CPrototype>`. 레이어에 배치되는 실제 인스턴스.
- `Initialize_Prototype()`은 원본에서, `Initialize(void* pArg)`는 각 사본에서 호출된다.
- 프로토타입은 레벨 인덱스로 구분되며, `LEVEL::STATIC`(0)은 모든 레벨이 공유한다.

### 레벨 · 레이어

레벨은 `Client_Defines.h`의 enum이다.

```cpp
enum class LEVEL { STATIC, LOADING, LOBBY, CHARACTER_SELECT, BERN, VALTAN_ARENA, DEVELOPMENT, END };
```

시작 Level은 항상 `LOBBY`다. Lobby는 `Test`, `Character Select`, `Valtan`, `Bern` 네 명령을 제공하고 `CLevelRegistry`가 각 `LEVEL`의 생성 함수, Loader 함수, map area와 load scope를 연결한다. 별도 실행 시나리오 catalog, 문자열 기반 Level 분기, direct `Change_Level` 호출을 추가하지 않는다.

`LEVEL::STATIC`은 전환 시에도 살아남는 영구 레벨이고, 나머지는 `Change_Level`에서 정리된다. 각 레벨 인덱스는 `map<wstring_t, shared_ptr<CLayer>>`를 가지며, `CLayer`는 `list<shared_ptr<CGameObject>>`를 들고 매 프레임 `Priority_Update → Update → Late_Update`를 구동한다.

### 레벨 전환 흐름

Level 전환 요청은 `CLevelTransitionService`에 제출한다. `CMainApp`은 현재 Level update가 끝난 뒤 `LOADING` 진입과 목표 Level activation을 수행하는 유일한 `Change_Level` 호출자다. `CLevel_Loading`은 Loader 성공 후 activation 요청만 제출한다. 로드는 `parse -> validate -> stage -> commit`이며 실패/취소 시 staging을 rollback한다.

`CLevelRegistry` descriptor의 `MAP_LOAD_SCOPE`가 제품 맵 로딩 범위의 런타임 정본이다. Bern과 Valtan 제품 Level은 자신의 진입/전투 범위와 배경만 로드한다. Loader와 `CMapPlacementRuntime`은 같은 `MAP_LOAD_SCOPE`를 소비해야 하며 한쪽만 필터링하면 안 된다. 로더 작업 스레드의 실패는 상태와 HRESULT로 반환하고 `MessageBox`로 대기시키지 않는다.

### 서버 권위 월드 파이프라인

MapTool의 현재 지원 범위인 player spawn/NPC/boss/triggerBox/collisionBox 배치는 `Data/Worlds/<AreaId>/Gameplay.world.json`에 stable placement ID로 저장한다. Valtan monster anchor/wave/group은 같은 Area의 `SpawnGroups.world.json`에 분리하며 triggerBox는 stable group ID만 참조한다. `Tools/WorldPipeline/Publish-WorldGameplay.ps1`이 actor/encounter/shape/spawn 참조와 `MonsterProfiles.json` formatVersion 2의 추적 유지 거리·회전·가속·감속·도착 감속 반경을 검증한 뒤 `Server/Bin/DataFiles/World/*.worldbootstrap`과 spawn-group bootstrap v4를 한 transaction으로 생성하며 Server pre-build가 이 publish를 강제한다. 제품 일반 몬스터는 Server에서 타깃 hysteresis, 공격 중 대상/방향 고정, navigation 경로 단축, 제한 회전과 가감속, 기존 원형 body sweep/slide를 사용하고 Client에서 2-tick transform 보간, occurrence 기반 결정적 공격 clip pool, 비공격 중 transient hit clip을 사용한다. presentation clip과 playback rate는 `MonsterCatalog.json` formatVersion 2가 소유하며 Server timing을 바꾸지 않는다. 수업용 `CMonster` 경로는 이 계약에 포함하지 않는다.

Server는 fixed 30 Hz에서 world entity의 transform/action/pattern state를 소유하고 Shared protocol v40 snapshot으로 보낸다. Client의 `CClientReplication`과 `CValtan`은 표현만 담당한다. UI·MapTool·Client GameObject가 제품 보스 판정을 직접 결정하지 않는다.

### 최소 수련장 Area

`dev.training.ground`는 새 Engine Level이 아니라 기존 `LEVEL::DEVELOPMENT`를 사용하는 Debug Map Editor Test 진입이다. 제품 캐릭터 테스트는 `Lobby-approved WORLD_ID::CHARACTER_SELECT_ARENA -> LEVEL::CHARACTER_SELECT -> LV_LOBBY_CLASSSELECT_SL00`을 사용한다. Lobby가 port `7777`의 `S2C_ENTER_ACCEPTED` 전체 payload를 검증한 뒤에만 기존 socket을 one-shot handoff하며 offline Preview와 `Preview / Server Play` 분기는 없다. Character Select는 직접 connect/send하지 않고 queued snapshot을 `CClientReplication`으로 소비해 HUD, 우클릭 이동, class quick-slot 스킬을 Server snapshot으로 반영한다. class thumbnail 선택은 target asset을 admission한 뒤 typed class-change command를 즉시 제출한다. Server는 identity와 살아 있는 위치를 유지하고 새 profile로 전투 상태를 초기화하며, 사망 상태면 원래 spawn을 navigation projection한 위치에서 부활시킨다. Client는 snapshot class 변경을 보고 같은 entity presentation을 transactionally 교체하고 Controller sequence를 보존해 새 class skill을 계속 제출한다. Client host는 process-local `LOSTARK_SERVER_HOST`를 우선하며 값이 없거나 `0.0.0.0`이면 현재 팀 endpoint `10.207.18.151`을 사용한다. 연결 실패·거부·5초 승인 timeout은 Lobby에 남고, 진입 후 disconnect는 Lobby로 복귀하며 자동 local gameplay fallback은 없다. Debug ImGui의 `Monster / Mid Boss (Lugaru) / Valtan` 선택과 `Spawn Selected`는 stable ID만 Server에 보내며, Server가 Character Select의 SpawnGroups 또는 disabled Valtan placement를 검증·활성화한다. Client local spawn은 없고 Valtan presentation asset만 Engine batch prototype commit으로 지연 준비한다. `Show Combat Colliders`는 Server가 복제한 radius의 Debug wire만 토글하며 damage에는 관여하지 않는다. Bern/Valtan map 진입도 마지막 Server 승인 class로 Lobby Server 승인이 필수다.

Server는 `CHARACTER_SELECT_ARENA` 진입 session마다 독립된 `CGameRoom` simulation을 만든다. 따라서 class 변경, 몬스터 소환, collider 판정과 damage는 모두 Server에서 실행되지만 다른 Character Select session과 player/entity/HP/damage snapshot을 공유하지 않는다. session 퇴장 시 queued `LEAVE`를 room tick이 소비하고 private simulation을 폐기한다. `BERN`, `VALTAN_ARENA`, `TRAINING_GROUND`는 world별 shared simulation을 유지한다.

2026-09-30 23:59 KST까지 공유 LAN Server는 `Framework.slnLaunch`의 `Server + Client` profile로 `0.0.0.0:7777`에 수신하고, 같은 팀 LAN의 Client는 `10.207.18.151:7777`에 접속한다. `Tools/Network/TeamLanEndpoint.json`이 endpoint와 만료일 정본이며 모든 에이전트는 pull 후 `Tools/Network/Sync-TeamLanEndpoint.ps1`을 실행해 Git 제외 debugger 설정을 동기화한다. 공유 x64 debugger 설정과 코드 기본값도 같은 endpoint를 사용하며, 실제 `Ctrl+F5` 시작은 사용자가 수행한다. Visual Studio가 이전 값을 캐시하면 project Reload 또는 IDE 재시작이 필요하다. `0.0.0.0`은 Server bind 주소이지 Client 접속 주소가 아니다. 세부 설정, 동일 revision/build/resource 준비와 `10049` 진단은 `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`의 `서로 다른 장소에서 Server와 Client 연결`을 따른다.

- visual admission: `LV_DEV_TRAINING_GROUND.mapassets`의 RCArena 10종만 로드
- visual placement: authoring 18개를 publisher가 runtime placement로 승격
- gameplay: 클래스 중립 `playerSpawn` 4개만 저장하며 `archetypeId`는 `null`
- navigation: `Data/Navigation/LV_DEV_TRAINING_GROUND.navgrid.json`에서 32×32 runtime grid를 결정적으로 생성
- runtime: `CClientReplication -> CPlayerController -> IPlayerCommandSink`와 `CCombatHUDViewModel`을 사용
- automated contracts: `NetworkProtocolHarness`, `Server.exe --contract-test`, 변경 domain publisher/check와 focused 실행 검증을 실행
- runtime validation: `Framework.slnLaunch`로 실제 Server와 Client를 함께 실행해 Lobby → Server 승인 Character Select 진입, class 연속 변경과 각 class 스킬 snapshot, 우클릭 이동, F6 follow/free와 free-camera command 차단, disconnect 시 Lobby 복귀, Bern/Valtan 진입을 확인

`playerSpawn`은 자리와 transform만 소유한다. 실제 character class는 Lobby/session 선택과 `C2S_ENTER_WORLD`가 소유하며 MapTool/world JSON이 특정 클래스를 고정하지 않는다.

Lobby에는 Lance Master, Gunslinger, Slayer, Artist, DimensionMaster, Warlord 여섯 slot이 보이며 여섯 class 모두 Client Loader/Spec과 Server player profile까지 연결되어 Bern/Valtan/Training 입장 계약을 사용한다. 실제 runtime payload는 팀장이 관리하는 `Client/Bin/Resources` 물리 폴더를 사용한다. DimensionMaster는 combined body `.wmodel`과 `WP_WSWP_M_06` L/S/P/E 네 정적 기본 무기 파츠를 사용하고, Warlord는 body가 얼굴과 눈만 그려 머리카락이 별도 equipment 파츠이고 총창과 방패 두 무기를 함께 든다. 나머지 네 class는 body/equipment/weapon 형식이다. 여섯 class의 quick slot과 LMB 평타는 `Data/Balance/PlayerSkills.json`의 Server 계약으로 연결된다. ACTIVE 슬롯은 Lance Master `Q W E R A S T V ALT_V`, Gunslinger `Q W E R A S D F T V ALT_V`, Slayer `Q W E R A S D F V ALT_V`, Artist `Q W E R A S T V Z ALT_V`, DimensionMaster `Q W E R A S D F T V ALT_V`, Warlord `Q W E R A S D F T X V ALT_V`다. Artist는 `Z` 저무는 달을 사용하고, Warlord는 `X` 전장의 방패와 `Z` 방어 태세 전환을 사용한다. LMB COMBO skillId는 각각 `34010/38000/45000/31000/2050010/17000`이다. 입력과 HUD는 실제 class 정의만 노출하고 누락 class를 Lance Master로 대체하지 않는다. 제품 Character presentation은 `Data/Animation/Authored/<Asset>/<Asset>.skillbindings.json`의 skillId → ordered model clips를 사용한다. `Data/Animation/Reference`의 clip/notify/chain/timing 문서와 `.skilltiming/.clipmap/.animnotify/.clipseq`는 저작 참고용 read-only이며 runtime 정본이 아니다.

Area Loader는 여섯 class binary를 전부 선로드하지 않는다. `CPlayableCharacterAssetService`가 선택 class를 먼저 admission하고 `CClientReplication`이 다른 class의 최초 spawn을 받을 때 같은 경로로 한 번만 추가한다. 이 경계를 우회하는 두 번째 model loader나 silent fallback을 만들지 않는다.

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

`_DEBUG`에서 `CMainApp`이 전역 Developer Tools 허브를 소유하고 F1로 토글한다. F6는 gameplay camera의 follow/free mode를 전환한다. Free camera는 WASD 이동, Tab mouse-look 전환을 사용하며 그동안 `CPlayerController`는 물리 key/mouse edge만 동기화하고 gameplay command는 제출하지 않는다. follow 복귀 뒤 새 press부터 제출한다. F2~F5와 F7~F12를 레벨/도구 전환에 사용하지 않는다. ImGui가 입력을 가져갈 때는 `CGameInstance::SetInputBlocked()`로 DirectInput 폴링을 막되 Character Select Server gameplay는 text input이 아닐 때만 명시적 keyboard passthrough를 사용한다. Client 실행 인자와 `CMainApp` 내부 runtime harness를 검증 경로로 다시 만들지 않는다.
F1 허브의 Diagnostics는 profiler 활성화와 무관하게 smoothed FPS와 최근 frame time을 항상 표시하며,
Profiler 체크박스는 별도의 CPU/GPU 상세 overlay와 capture를 활성화한다.

F1의 `Balance Tool`은 five-class/boss selector, stats·movement·skill/combo·pattern authoring과 Server
snapshot/damage-event 진단을 제공한다. Save는 `Data/Balance`/`Data/Encounters` 원본만 교체하고 변경
field의 provenance를 `PROJECT_TUNED`로 동기화한 뒤 Validate한다. `Publish Server Data` 뒤 Server를
재시작해야 적용된다. Tool이 실행 중 Server 구조체나 Client HUD 값만 덮어쓰는 hot reload는 없다.

`Data/Valtan/Valtan.pattern.json`은 admission된 발탄 1페이즈 pattern의 Server stage, ordered body-animation
occurrence, Product/independent Effect 사용 위치를 함께 저작하는 정본이다. 전용 publisher가 이를 기존
Encounter, patternbindings, patterneffectcues와 combat-object typed 제품 문서로 투영하며 Server/Arena는
master를 두 번째 런타임으로 직접 읽지 않는다. Animation Tool의 `Valtan Pattern Master`는 이 정본의 7개
pattern 전체 body timeline을 재생·seek한다. 각 stage는 `EXACT`, `HOLD_LAST_POSE`,
`LOOP_TO_STAGE_END` 중 하나의 명시적 animation 종료 정책을 가지며, Tool은 master branch graph와
presentation source를 그대로 소비한다. 160~109 normal 선택은 master가 소유하는 정확한 5-pattern
`WEIGHTED_POOL`을 gameplay rotation product로 투영하며 health-bar mechanic queue가 우선한다.
`counterReactionLayers`는 기존 Product의 counterable stage와 animation action을 reference-only로 join해
Animation/Effect Tool에 노출하되 7-pattern admission에는 추가하지 않는다. 기존 1~67
patternpreview/clipseq 화면은 source reference다.

Debug F1의 `Effect Tool`과 `All Effects`는 direct-authored Player Product cue와 Valtan pattern cue를
같은 unified Effect 저작 tree로 연다. Player의 skill과 Valtan의 pattern은 같은 최상위 저작 단위다.
Valtan pattern을 열면 master가 가리키는 Product cue와 stage-authored reference를 중복 없이 나열하고,
combat-object/도넛 같은 재사용 asset은 최상위 `INDEPENDENT EFFECT` tree에 한 번만 노출한다. 그 아래에는
semantic stage와 ordered clip occurrence를 표시한다. Open은 Valtan 모델과 해당 animation을 함께
stage하고, `Play Authoring Timeline`은 선택한 pattern 경로의 모든 stage body animation과 Effect clock을
같은 시작점에서 재생한다. 이때 선택 Effect의 Product cue가 가진 global stage offset, anchor/follow/local
transform과 stop window를 보존한다. `SERVER_PATTERN_STAGE` 독립 Effect는 이 cue 경로를,
`SERVER_COMBAT_OBJECT` 독립 Effect는 replicated world root 경로를 사용한다. saved 문서 decode는
Open/Play 전까지 지연한다.
phase band는 Server encounter 메타데이터이며 All Effects의 반복 tree나 stage 숨김 filter로 사용하지
않는다. 두 owner는 같은 Mesh, Sprite, Mesh Particle, Sprite Particle, Local Decal, Trail/Ribbon family를
사용한다.
Save는 선택된 direct-authored Effect 하나를 원자 저장한 뒤 같은 catalog revision의 prepared target을
stage/validate/commit한다. 실행 중 occurrence는 이전 immutable document를 끝까지 유지하고 다음 spawn부터
새 document를 사용한다. 준비나 renderer commit 실패 시에는 compare-and-swap으로 저장 파일도 이전 bytes로
복원하고 이전 Product pointer와 cache를 유지한다.
`Data/Effects/EffectCatalog.json`과 `Data/Effects/Authored/*.effect.json`이 제품 Effect의 단일 입력이며,
Editor Save 직후의 다음 재생과 다음 Client 실행이 같은 authored 파일을 소비한다. schema·catalog·source batch는
`Tools/EffectPipeline/Validate-EffectSources.ps1`로 검증하고 다른 폴더로 복사하거나 publish하지 않는다.
v15의 고급 trail/light projection도 같은 authored 문서의 typed `runtimeCarrier`에 inline 저장한다. runtime 재생은
별도 Tool renderer 없이 `CEffectCatalog -> CEffectPresentationService -> CEffectObject` 한 경로만 사용한다.
V1 Product 문서가 참조하는 DDS/WModel dependency closure는 `Client/Bin/Resources`의 같은 상대 asset ID로
선별 추적되므로 팀원은 clone/pull 뒤 `git lfs pull`만 수행하면 같은 Effect payload를 받는다. 이 예외는 전체
Resources pack이나 미참조 자산을 Git 정본으로 승격하지 않는다.

#### Artist F와 Effect 화면 검증은 사용자 전용

- 에이전트는 Client UI를 자율적으로 실행·조작하지 않고 화면을 직접 캡처하거나 스크린샷을 만들지 않으며, visual fidelity를 대신 판정하지 않는다.
- 사용자가 대화에 첨부한 스크린샷이나 이미지를 분석해 달라고 요청하면 에이전트는 반드시 열람·분석해 형태·색·타이밍·밀도·궤적의 관찰 결과와 가능한 occurrence 진단을 보고한다.
- 에이전트가 수행하는 자동 검증은 빌드, 구조화된 로그, draw/resource/shader 수치 진단까지다. 최종 화면 판정은 사용자가 실제 Client에서 직접 수행한다.
- 에이전트는 실행 준비 후 Server CMD/Client 상태와 사용자가 직접 확인할 class/skill 경로를 전달하고 멈춘다.
- 사용자의 서면 판정이 없으면 `manual first pixel`, `eye smoke`, `visual PASS`, occurrence 승인을 완료로 기록하지 않는다.
- 일반적인 완성·복원 요청은 Client/UI 자율 실행·조작이나 화면 캡처를 허가하지 않는다. 요청받은 사용자 첨부 이미지 분석은 진단 입력이며 최종 visual PASS나 단독 admission 증거가 아니다.

연속 clip을 가진 스킬의 Product Effect는 stage 첫 clip 하나에 합치지 않는다. 각 시각 clip의
`effectref=asset` cue가 clip-local Authored 문서를 가리키며 Character가 이미 적용하는 `playMs`,
`playRate`, loop와 late snapshot catch-up을 그대로 사용한다. Product target 정본은 publisher가 네
class animevent에서 선택해 runtime catalog에 실은 `effectref=asset` ID membership이다. authoring-only
문서는 runtime catalog와 Character 준비 queue에는 들어오지 않는다.

Character 초기화는 cue/anchor/HIT metadata를 검증·commit하고 target ID만 process-global queue에
등록한다. 등록 frame은 resource 작업을 양보하고, 이후 `CMainApp` main-thread frame seam이 target을
최대 하나씩 JSON parse/drawable validation/budget/GPU prepared-cache commit한다. 실패 target만 같은
catalog revision에서 fail-closed하며 이미 준비한 target과 다음 target은 보존한다. 전투 중 Product
Spawn은 cache-only catalog lookup과 준비된 bundle만 사용하고 shader compile, model/DDS/vector-field
로드를 수행하지 않는다. prepared miss는 동기 fallback 없이 해당 cue만 거부한다.
Character Select는 Loader 시작과 함께 선택 class cue를 priority queue에 등록해 map/model load와 Effect
CPU 준비를 겹친다. activation은 현재 catalog revision의 선택 target만 terminal인지 확인하며 무관한
background pending은 진입을 막지 않는다. Product prepared record는 catalog document를 immutable shared
ownership으로 유지하며 Playback/Renderer attach는 revision/document identity를 재사용한다.

animevent v6 Effect cue는 위치 추적 `follow=follow|snapshot`과 방향 권위
`orientation=anchor|action_facing`을 분리한다. `action_facing`은 root anchor의 ACTIVE 스킬에서만
Server snapshot의 스킬 edge yaw/actionStartTick을 한 번 캡아 사용하며, HOLD나 world-root cue에는
자동 적용하지 않는다. v5 문서의 orientation 누락은 `anchor`로 읽히고 unknown token,
non-finite yaw, mirrored action-facing basis는 해당 cue만 fail-closed한다.

Character Select 내부의 Server 승인 class 변경도 local snapshot을 stable generation으로 stage하고 새 class
Product cue target이 settle된 뒤에만 기존 character replacement transaction을 commit한다. 준비 중
gameplay와 class/stage/create 입력은 차단하며 replacement 실패는 입력 정지 대신 Lobby 복귀로 격리한다.

Debug Lobby의 `Test`는 기존 Server 승인을 받은 뒤 새 제품 Level을 추가하지 않고 `LEVEL::DEVELOPMENT`를 격리된 Map Editor workspace로 연다. F1은 모든 Level에서 Developer Tools 표시만 토글하고 Map Tool 버튼도 Level을 전환하지 않는다. editor 모드에서는 수련장 런타임, 캐릭터, 네트워크 복제를 올리지 않으며 Character Select, Bern, Valtan, 원본 Training Map(`LV_SHS_RCARENA_D`)을 `Data/Maps/MapCatalog.json`의 정확한 source 경로로 stage 후 commit한다. 저장 대상은 `Data` authoring 문서뿐이고 `Client/Bin/DataFiles` 런타임 문서는 publisher만 교체한다. Area별 저장 정책과 맵 담당자 절차는 `.md/TEAM/AREA_DATA_LAYER_GUIDE.md`를 따른다.

MapCatalog의 optional `sourceLights`/`lights` pair는 Area별 point-light presentation 계약이다.
source는 `Data/Maps/Authoring/<AreaId>/<AreaId>.maplights.json`, runtime은
`Client/Bin/DataFiles/Map/<AreaId>.maplights.json`이며 둘 중 하나만 선언할 수 없다.
`Publish-MapAuthoring.ps1`이 visual placement와 같은 transaction으로 strict validate/publish하고,
MapTool은 source를, 제품 Level은 runtime을 기존 `CPresentation_Manager` transient light 경로로 제출한다.
Valtan은 이 pair를 필수로 선언하며 누락·손상 시 이전 editor Area 보존 또는 제품 Level 진입 실패로
fail-closed한다. 이 레이어는 Client 표현 전용이고 Server gameplay light/collision 계약이 아니다.
현재 Valtan 문서는 SL04 source-instance-exact PointLight 22개를 소유한다. 높이 기준 상단 5개와 중·하단
17개로 구분하지만, 이를 특정 철탑 station에 묶는 것은 geometry와 위치를 대조한 inference이지
source가 직접 제공한 parent/slot 관계가 아니다. 위치·색·반경·밝기는 source instance 값이고,
falloff `2`는 source 행에 직렬화되지 않아 current-revision class default에서 가져온 inference다.
PointLight는 주변 조명만 만들며 원작 화면의 visible
fire/sprite와 발광 slot surface는 별도 미복구 표현이다.

Valtan의 `World Destruction` 모드에는 제품 Server와 분리된 `Destruction Model View`가 있다.
`Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.destructionsimulation.json` format v2의
stable debris element를 기존 `CDeployPropRuntime -> CDeployPropObject -> CModel` world instance에
연결한다. element의 source placement 하나만 Wall Mesh Emitter이고 runtime이 stable fragment 12개를
파생해 각각 CModel proxy와 PhysX actor로 재생한다. 같은 벽을 중복 표시하는 placement는
`suppressionAliasPlacementIds`에 stable ID로 저장하며 debris를 추가 생성하지 않는다. All Fragments/Solo
Emitter/Solo Fragment, play/pause/restart, 1/60 single-step과 reset 후 고정-step seek를 제공한다.
direction과 speed는 초기 linear velocity로 변환되고 gravity scale/lifetime/trigger는 authoring policy다.
activation부터 fragment lifetime 만료 뒤까지 source와 suppression alias는 숨겨 두고 Reset/Clear에서
이전 상태를 복원한다. fragment model/state/life/pose/velocity는 read-only runtime sample이다. format v1은
자동 추측 변환 없이 fail-closed하며 MapTool에서 v2로 다시 authoring해야 한다.
이 파일은 제품 publisher 입력이 아니며 `Gameplay.world.json kind=destroyable`의 Server admission,
동적 navigation/collision과 Shared replication은 계속 fail-closed다.
맵 담당자의 실제 실행·확장 절차는 `.md/TEAM/MAP_DESTRUCTION_PHYSX_HANDOFF.md`를 따른다.

### UI 레이아웃 authoring과 제품 런타임 전환

`CHUDLayoutTool`은 F1 Developer Tools에서 사용하는 ImGui authoring 도구다. ImGui 화면을
제품 UI 이미지로 캡처하는 도구가 아니라, 실제 UI 이미지의 배치 계약을 작성하는 도구다.

| 문서 | asset domain | 용도 |
|---|---|---|
| `Data/UI/HUD/HUD_Layout.json` | `UI/HUD/` | class별 전투 HUD |
| `Data/UI/ScreenUI/ScreenUI.json` | `UI/ScreenUI/` | 공용 화면 UI |

현재 JSON은 `lostark.ui-layout` format version 1이며 reference resolution은 1280×720이다.
`slot.id`, owner class, type, rect, rotation, layer 순서, normal/hover image, tint, additive,
flip, shine, frame animation을 저장한다. `CHUDLayoutTool`은 `Resources/UI`를 palette로 읽어
drag/drop, 이동, 크기, 회전, layer/z-order, hover preview와 JSON save/load를 제공한다.
asset path는 반드시 `UI/...` Resources-relative ID이며 `CRuntimeAssetRoot::Resolve()`로
해석한다.

제품 전환 규칙은 다음과 같다.

1. UI 담당자는 이미지를 `Resources/UI/<Domain>/...`에 준비하고 ImGui tool에서 slot/layer에
   연결한다. Git에는 `Data/UI` JSON을 두고, 실제 이미지 payload인 `Client/Bin/Resources`는
   팀장이 관리하는 runtime 입력으로 유지한다. 별도 immutable pack/lock/manifest를 완료 조건으로 만들지 않는다.
2. `slot.id`가 저장·runtime widget identity다. pointer, vector index, ImGui label은 ID가 아니다.
3. 제품 runtime loader는 JSON을 한 번 `parse -> validate -> stage -> commit`하고 `CUIObject`
   계열 image widget을 만든다. 매 프레임 JSON이나 이미지를 다시 읽지 않는다.
4. 제품 UI click은 world `CPicking`이 아니라 reference resolution으로 보정한 mouse 좌표와
   transformed slot rect의 screen-space hit test다. 앞쪽 draw order의 visible/enabled widget
   하나만 소비하고 그 프레임의 gameplay mouse command를 차단한다.
5. display-only HUD의 기본 hit test는 `none`이다. 버튼이 필요하면 layout schema version과
   함께 explicit interaction/command binding을 추가하고 stable command ID를 기존 typed
   command service에 매핑한다. JSON 문자열로 함수 이름을 실행하거나 UI에서 socket을 직접
   호출하지 않는다.
6. HP/resource/cooldown/boss 상태는 `CCombatHUDViewModel`에서 읽는다. UI가 Server 수치나
   판정을 자체 생성하지 않는다.

현재 완료된 범위는 ImGui authoring, asset preview, layout JSON save/load와 runtime HUD
ViewModel/임시 overlay다. layout JSON으로 최종 image widget을 생성하는 runtime factory,
2D UI input router, interaction schema/command binding은 아직 별도 수직 슬라이스이며,
이 세 항목을 구현하기 전에는 “ImGui UI가 제품 이미지 UI로 자동 교체됐다”고 판단하지 않는다.

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
- 전투 수치: `Data/Balance/PlayerProfiles.json`, `PlayerSkills.json`, `DamageProfiles.json`, `BossProfiles.json`이 정본이다. two-step ground target의 optional 입력·preview 계약은 `PlayerSkillTargeting.json`이며 기존 skillId/maximumRange와 exact join한다. Server pre-build의 `Publish-GameplayBalance.ps1`이 수치와 `SKILLTARGET` admission runtime bootstrap을 생성한다. texture ID와 tint는 Client-only이며 Server bootstrap에 넣지 않는다.
- 아이템: `Data/Items/ItemCatalog.json`이 정본이다. Server pre-build의 `Publish-ItemCatalog.ps1`이 `Server/Bin/DataFiles/Items/Items.bootstrap`을 생성하고 `CItemCatalog`이 이를 필수 로드한다. `Server/Bin` 생성물을 커밋하거나 Server가 authoring JSON을 직접 읽게 하지 않는다.
- Git 관리 대상 `Data` 원본은 `Client.vcxproj`에서 `96.DataFiles`의 `None` 항목으로 보인다. 이는 탐색용 링크이며 runtime 복사나 두 번째 정본이 아니다.
- 현재 밸런스 검증은 JSON publish 후 Server 재기동과 `dev.training.ground` smoke로 수행한다. 무중단 Hot Reload는 아직 활성화하지 않으며 revision과 Server tick-boundary commit 없이 Client만 재읽지 않는다. 상세 계약은 `.md/TEAM/BALANCE_TUNING_AND_HOT_RELOAD_CONTRACT.md`를 따른다.
- 서버 길찾기: `Data/Navigation`이 정본이다. MapTool bake Area는 `<AreaId>.navsource/.navpaint/.navblockers`, 단순 uniform Area는 `<AreaId>.navgrid.json`을 사용하며 `Publish-ServerNavigation.ps1`이 Client/Server runtime `.navgrid`와 Area별 최대 인접 높이차를 가진 `.navpolicy`를 결정적으로 생성한다. gameplay spawn/boss의 walkable cell·높이 정합성도 같은 publish에서 검사한다. `.navpaint` version 3의 optional height override는 resolved surface의 다층 bake 오선택을 교정하며 Server A*와 이동 적용 직전 guard가 `.navpolicy`를 소비한다.
- 런타임 리소스: `CRuntimeAssetRoot::Resolve("Character/..."|"Map/..."|...)`를 사용한다.
- 애니메이션 작성 데이터: `Data/Animation/Authored/<AssetId>/`
- 플레이어 스킬 히트 셰이프: `Data/Animation/HitShapes/<AssetId>.hitshapes.json`이 Server 판정 정본이다. `Tools/CharacterAnimationIntake/build_hitshapes.py`가 `.animevents` HIT 행과 skillbindings 체인에서 생성하고 `Publish-GameplayBalance.ps1`이 `SKILLHIT/SKILLSTAGEHIT` 행으로 publish한다. `areaType`은 원본 SkillEffect 의미 그대로 1=원/링, 2=전방 박스(원본 `AreaAngle`이 폭 cm → `width` m), 3=부채꼴(`angle` 도)이다. Server는 스킬당 damage rate를 sub-hit 수로 분할해 셰이프 안의 대상 전부에 적용하며, 셰이프가 없는 스킬만 `maximumRange` 원형 단일 판정을 유지한다. notify HIT가 없는 스킬은 `fill_animevents_hit_shapes.py`가 `PlayerSkills.json hitTimeMs` 위치에 skilltiming caster 셰이프 한 행을 합성한다. 원작이 투사체/장판으로 때리는 스킬은 `Data/Animation/Reference/<AssetId>/<AssetId>.projectiles`(원본 `XMLData/Projectile/<PK>.loa` 추출) → `Tools/CharacterAnimationIntake/fill_projectiles.py` → `Data/Animation/Authored/<AssetId>/<AssetId>.projectiles.json` → `build_hitshapes.py`의 skill/stage `projectiles[]`(v3) → `SKILLPROJ/SKILLSTAGEPROJ` 행으로 이어지며, Server `CPlayerSkillSystem`이 spawn 시각에 MISSILE(조준 방향 직진, 거리·수명 소멸, 접촉 히트는 대상당 1회)·FIXAREA(조준 지점, 최대 거리 clamp, 예약 시각 히트) 오브젝트를 만들어 caster 히트와 같은 damage rate를 sub-hit로 나눠 적용한다. Client는 Debug 와이어 예측만 그리고 판정하지 않는다.
- 애니메이션 추출 참조: `Data/Animation/Reference/<AssetId>/`; 0-row 컨테이너는 Tool 경로/파서 계약일 뿐 추출 완료 증거가 아니다.
- 맵 추출 기준본: `Data/Maps/Imported/<AreaId>/`; `.mapassets`, shard `.mapset`과 baseline placement를 소유한다.
- MapTool 작성본: `Data/Maps/Authoring/<AreaId>/`; visual placement와 optional point-light presentation source를 소유하며 publish 후에만 `Client/Bin/DataFiles/Map/` 런타임 입력이 된다.
- shard-set을 포함한 visual publish는 `Tools/MapPipeline/Publish-MapAuthoring.ps1`만 수행한다. `Imported` catalog와 `Authoring` placement를 읽어 catalog, 여러 shard placement, mapset, optional deploy pair, optional point-light presentation을 한 트랜잭션으로 교체하고 중간 실패 시 전부 rollback한다.

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
- 현재 World Gameplay 제품 kind는 `playerSpawn`, `npc`, `boss`, 단일 `movePlayer`/`changeLevel`/`activateSpawnGroup`/`activateEncounter` action의 `triggerBox`, 정적 `collisionBox`다. NPC presentation은 `Data/Actors/NpcCatalog.json`의 `runtimeStatus=supported` archetype 70종, monster presentation은 Valtan 4 archetype을 지원한다. 수업용 Monster 구현은 포함하지 않는다.
- Area별 레이어 보유 현황과 생략 규칙은 `.md/TEAM/AREA_DATA_LAYER_GUIDE.md`가 정본이다. Debug Development MapTool에서 `triggerBox`, `collisionBox`, Valtan spawn anchor/group/wave를 저작하면 publisher가 bootstrap v5와 optional spawn bootstrap v2로 변환하고 Server가 player OBB 진입, swept 이동 차단, monster wave/AI/combat/despawn을 판정한다. `destroyable`, 파티 대기, 컷신, Area별 balance override는 아직 지원하지 않는다.

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
