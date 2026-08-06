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
```

팀장이 전달한 runtime 리소스를 `Client/Bin/Resources/{Fonts,Character,Deploy,Effect,Map,UI}` 물리 폴더에 둔다. 별도 asset-pack lock, ZIP hash, publish/hydrate/verify 절차는 사용하지 않는다.

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
| 프로젝트 데이터 정본 | `Data/`의 catalog, imported, authoring, reference JSON/문서 | Git 일반 추적; 대용량 map 문서는 Git LFS |
| 필수 바이너리 입력 | `Engine/ThirdPartyLib/` | **Git LFS** (`.gitattributes` 패턴) |
| 실행 데이터 생성물 | `Client/Bin/DataFiles/`, `Server/Bin/DataFiles/` | `Data/`에서 publisher가 생성; 직접 편집 금지 |
| 런타임 리소스 · 쿠킹 산출물 | `Client/Bin/Resources/{Fonts,Character,Deploy,Effect,Map,UI}` | 팀장 관리 물리 폴더 |

- clone 시 `git lfs install` 후 clone하거나, 이미 받았다면 `git lfs pull`을 실행해야 lib/DLL/DDS가 포인터가 아닌 실물이 된다.
- `Client/Bin/Resources/` 최상위에는 위 여섯 폴더만 허용한다. `Resources/LostArk`, `Models`, `Textures`, `SourceData`, `Sound` 래퍼를 다시 만들지 않는다.
- raw 추출물과 SourceData는 runtime Resources에 넣지 않는다. 팀장이 채택한 쿠킹 결과만 물리 폴더에 둔다.
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

MapTool의 현재 지원 범위인 player spawn/NPC/boss/triggerBox/collisionBox 배치는 `Data/Worlds/<AreaId>/Gameplay.world.json`에 stable placement ID로 저장한다. Valtan monster anchor/wave/group은 같은 Area의 `SpawnGroups.world.json`에 분리하며 triggerBox는 stable group ID만 참조한다. `Tools/WorldPipeline/Publish-WorldGameplay.ps1`이 actor/encounter/shape/spawn 참조를 검증한 뒤 `Server/Bin/DataFiles/World/*.worldbootstrap`과 optional `*.spawngroupsbootstrap`을 한 transaction으로 생성하며 Server pre-build가 이 publish를 강제한다. 수업용 `CMonster` 경로는 이 계약에 포함하지 않는다.

Server는 fixed 30 Hz에서 world entity의 transform/action/pattern state를 소유하고 Shared protocol v6 snapshot으로 보낸다. Client의 `CClientReplication`과 `CValtan`은 표현만 담당한다. UI·MapTool·Client GameObject가 제품 보스 판정을 직접 결정하지 않는다.

### 최소 수련장 Area

`dev.training.ground`는 새 Engine Level이 아니라 기존 `LEVEL::DEVELOPMENT`를 사용하는 Debug Map Editor Test 진입이다. 제품 캐릭터 테스트는 `LEVEL::CHARACTER_SELECT -> LV_LOBBY_CLASSSELECT_SL00 -> Lobby-approved WORLD_ID::CHARACTER_SELECT_ARENA -> LEVEL::CHARACTER_SELECT`를 사용한다. 최초 Character Select는 socket 없이 여섯 class의 3D preview를 제공한다. preview에서 class를 고르고 `Server Play`를 선택하면 tokenized TEST command가 Lobby로 넘어가며, Lobby가 port `7777`의 `S2C_ENTER_ACCEPTED` 전체 payload를 검증한 뒤 기존 socket을 one-shot handoff한다. Server Arena에서 `Preview`를 선택하면 현재 socket과 replication을 정리하고 tokenized CHARACTER_SELECT를 거쳐 socket 없는 Preview로 재진입한다. 재진입한 Character Select는 직접 connect/send하지 않고 queued snapshot을 `CClientReplication`으로 소비해 HUD, 우클릭 이동, class quick-slot 스킬을 Server snapshot으로 반영한다. Client host는 process-local `LOSTARK_SERVER_HOST`를 읽으며 값이 없거나 `0.0.0.0`이면 `127.0.0.1`을 사용한다. 연결 실패·거부·5초 승인 timeout은 Lobby에 남고, 진입 후 disconnect는 Lobby로 복귀하며 자동 local gameplay fallback은 없다. `Summon Valtan (Lazy)`는 Client local spawn이 아니라 Server의 disabled placement template 승인을 거치며 presentation asset은 Engine batch prototype commit으로 지연 준비한다. Bern/Valtan map 진입도 Lobby Server 승인이 필수다.

같은 PC 검증은 `Framework.slnLaunch`의 `Server + Client` profile을 선택해 Server와 Client를 함께 실행하고 기본 `127.0.0.1:7777`을 사용한다. LAN 검증은 Git에서 제외되는 `Server.vcxproj.user`에 `--bind-address 0.0.0.0`, `Client.vcxproj.user`에 `LOSTARK_SERVER_HOST=<host 사설 IPv4>`를 설정한다. `0.0.0.0`은 Server bind 주소이지 Client 접속 주소가 아니다. endpoint 입력 UI와 자동 LAN discovery는 아직 제공하지 않으며 개인 IP를 소스·JSON·공유 project 설정에 커밋하지 않는다.

- visual admission: `LV_DEV_TRAINING_GROUND.mapassets`의 RCArena 10종만 로드
- visual placement: authoring 18개를 publisher가 runtime placement로 승격
- gameplay: 클래스 중립 `playerSpawn` 4개만 저장하며 `archetypeId`는 `null`
- navigation: `Data/Navigation/LV_DEV_TRAINING_GROUND.navgrid.json`에서 32×32 runtime grid를 결정적으로 생성
- runtime: `CClientReplication -> CPlayerController -> IPlayerCommandSink`와 `CCombatHUDViewModel`을 사용
- automated contracts: `NetworkProtocolHarness`, `Server.exe --contract-test`, `ProjectAudit`을 실행
- runtime validation: `Framework.slnLaunch`로 실제 Server와 Client를 함께 실행해 Lobby → Character Select Preview → Server Play 선택 → Lobby 승인 → 같은 visual map Server gameplay 재진입, 우클릭 이동·스킬 snapshot, F6 follow/free와 free-camera command 차단, Preview 복귀, Valtan lazy summon, disconnect 시 Lobby 복귀, Bern/Valtan 진입을 확인

`playerSpawn`은 자리와 transform만 소유한다. 실제 character class는 Lobby/session 선택과 `C2S_ENTER_WORLD`가 소유하며 MapTool/world JSON이 특정 클래스를 고정하지 않는다.

Lobby에는 Lance Master, Gunslinger, Slayer, Artist, DimensionMaster, Warlord 여섯 slot이 보이며 여섯 class 모두 Client Loader/Spec과 Server player profile까지 연결되어 Bern/Valtan/Training 입장 계약을 사용한다. 실제 runtime payload는 팀장이 관리하는 `Client/Bin/Resources` 물리 폴더를 사용한다. DimensionMaster는 combined body `.wmodel`과 `WP_WSWP_M_06` L/S/P/E 네 정적 기본 무기 파츠를 사용하고, Warlord는 body가 얼굴과 눈만 그려 머리카락이 별도 equipment 파츠이고 총창과 방패 두 무기를 함께 든다. 나머지 네 class는 body/equipment/weapon 형식이다. 여섯 class의 quick slot과 LMB 평타는 `Data/Balance/PlayerSkills.json`의 Server 계약으로 연결된다. ACTIVE 슬롯은 Lance Master `Q W E R A S T V ALT_V`, Gunslinger `Q W E R A S D F T V ALT_V`, Slayer `Q W E R A S D F V ALT_V`, Artist `Q W E R A S V ALT_V`, DimensionMaster `Q W E R A S D F T V`, Warlord `Q W E R A S D F T X V ALT_V`이며, DimensionMaster에는 `ALT_V`가 없다. `X`와 `Z` 방어 태세 전환은 Warlord만 사용한다. LMB COMBO skillId는 각각 `34010/38000/45000/31000/2050010/17000`이다. 입력과 HUD는 실제 class 정의만 노출하고 누락 class를 Lance Master로 대체하지 않는다. 제품 Character presentation은 `Data/Animation/Authored/<Asset>/<Asset>.skillbindings.json`의 skillId → ordered model clips를 사용한다. `Data/Animation/Reference`의 clip/notify/chain/timing 문서와 `.skilltiming/.clipmap/.animnotify/.clipseq`는 저작 참고용 read-only이며 runtime 정본이 아니다.

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

F1의 `Effect Tool`은 `Data/Balance/PlayerSkills.json`의 class/input slot/effectId와
`Data/Effects/Authored/<effectId>.effect.json`을 결합해 All Effects 트리를 만든다. 저작 문서는
`lostark.effect-authoring` v6으로 저장하며 Element display/group/source/visible, Material Template ID,
stable resource slot ID를 소유한다. v5 문서는 호환 load 뒤 다음 Save에서 v6으로 승격한다. 현재 등록된
Template은 실제 Effect HLSL에 대응하는 `effect.standard` 하나이고 Base/Noise/Mask/Emissive/Dissolve
다섯 input을 제공한다. 원본 추출 근거와 HLSL 구현이 없는 custom Template/slot은 등록하지 않는다.
All Effects의 스킬 행과 Data Files의 Authored 행은 같은 완성 Effect Document를 여는 두 진입점이며,
스킬 행은 현재 문서와 같아도 Complete Effect를 0초부터 다시 재생한다. 여러 시각 파츠의 조합 단위는
별도 Effect 중첩이 아니라 한 Document 안의 Mesh/Sprite/Particle/Decal/Trail Element다. Effect Detail의
연속 수치는 drag 중 local draft만 world preview에 live-stage하고 Apply에서만 active Document에 commit한다.
저작 UI는 runtime Published 목록을 편집하지 않으며, 제품 재생은 계속 `CEffectCatalog ->
CEffectPresentationService -> CEffectObject` 경로를 사용한다. publish는
`Tools/EffectPipeline/Publish-Effects.ps1`만 수행한다.

Debug Lobby의 `Test`는 기존 Server 승인을 받은 뒤 새 제품 Level을 추가하지 않고 `LEVEL::DEVELOPMENT`를 격리된 Map Editor workspace로 연다. F1은 모든 Level에서 Developer Tools 표시만 토글하고 Map Tool 버튼도 Level을 전환하지 않는다. editor 모드에서는 수련장 런타임, 캐릭터, 네트워크 복제를 올리지 않으며 Character Select, Bern, Valtan, 원본 Training Map(`LV_SHS_RCARENA_D`)을 `Data/Maps/MapCatalog.json`의 정확한 source 경로로 stage 후 commit한다. 저장 대상은 `Data` authoring 문서뿐이고 `Client/Bin/DataFiles` 런타임 문서는 publisher만 교체한다. Area별 저장 정책과 맵 담당자 절차는 `.md/TEAM/AREA_DATA_LAYER_GUIDE.md`를 따른다.

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
   연결한다. Git에는 이미지 payload가 아니라 `Data/UI` JSON과 resource pack lock/manifest만
   둔다.
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
- 전투 수치: `Data/Balance/PlayerProfiles.json`, `PlayerSkills.json`, `DamageProfiles.json`, `BossProfiles.json`이 정본이다. Server pre-build의 `Publish-GameplayBalance.ps1`만 runtime bootstrap을 생성한다.
- Git 관리 대상 `Data` 원본은 `Client.vcxproj`에서 `96.DataFiles`의 `None` 항목으로 보인다. 이는 탐색용 링크이며 runtime 복사나 두 번째 정본이 아니다.
- 현재 밸런스 검증은 JSON publish 후 Server 재기동과 `dev.training.ground` smoke로 수행한다. 무중단 Hot Reload는 아직 활성화하지 않으며 revision과 Server tick-boundary commit 없이 Client만 재읽지 않는다. 상세 계약은 `.md/TEAM/BALANCE_TUNING_AND_HOT_RELOAD_CONTRACT.md`를 따른다.
- 서버 길찾기: `Data/Navigation`이 정본이다. MapTool bake Area는 `<AreaId>.navsource/.navpaint/.navblockers`, 단순 uniform Area는 `<AreaId>.navgrid.json`을 사용하며 `Publish-ServerNavigation.ps1`이 Client/Server runtime `.navgrid`를 결정적으로 생성한다. gameplay spawn/boss의 walkable cell·높이 정합성도 같은 publish에서 검사한다.
- 런타임 리소스: `CRuntimeAssetRoot::Resolve("Character/..."|"Map/..."|...)`를 사용한다.
- 애니메이션 작성 데이터: `Data/Animation/Authored/<AssetId>/`
- 애니메이션 추출 참조: `Data/Animation/Reference/<AssetId>/`; 0-row 컨테이너는 Tool 경로/파서 계약일 뿐 추출 완료 증거가 아니다.
- 맵 추출 기준본: `Data/Maps/Imported/<AreaId>/`; `.mapassets`, shard `.mapset`과 baseline placement를 소유한다.
- MapTool 작성본: `Data/Maps/Authoring/<AreaId>/`; publish 후에만 `Client/Bin/DataFiles/Map/` 런타임 입력이 된다.
- shard-set을 포함한 visual publish는 `Tools/MapPipeline/Publish-MapAuthoring.ps1`만 수행한다. `Imported` catalog와 `Authoring` placement를 읽어 catalog, 여러 shard placement, mapset, optional deploy pair를 한 트랜잭션으로 교체하고 중간 실패 시 전부 rollback한다.

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
- 현재 World Gameplay 제품 kind는 `playerSpawn`, `npc`, `boss`, 단일 `movePlayer`/`changeLevel`/`activateSpawnGroup`/`activateEncounter` action의 `triggerBox`, 정적 `collisionBox`다. NPC presentation은 `NPC_BEDA`, monster presentation은 Valtan 4 archetype을 지원한다. 수업용 Monster 구현은 포함하지 않는다.
- Area별 레이어 보유 현황과 생략 규칙은 `.md/TEAM/AREA_DATA_LAYER_GUIDE.md`가 정본이다. Debug Development MapTool에서 `triggerBox`, `collisionBox`, Valtan spawn anchor/group/wave를 저작하면 publisher가 bootstrap v5와 optional spawn bootstrap v1으로 변환하고 Server가 player OBB 진입, swept 이동 차단, monster wave/AI/combat/despawn을 판정한다. `destroyable`, 파티 대기, 컷신, Area별 balance override는 아직 지원하지 않는다.

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
