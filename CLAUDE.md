# CLAUDE.md

이 파일은 Claude Code(claude.ai/code)가 이 저장소에서 작업할 때 참고하는 LostArk 코드베이스 설명이다.
공통 에이전트 행동 규칙의 정본은 `AGENTS.md`이며, 작업 시작 시 먼저 읽는다.
계획서·설계서 요청은 `.md/계획서작성규칙.md`를 추가로 읽고 `.md/GB/<MM-DD>/`에 PLAN/RESULT 문서를 작성한다. 계획서는 C1~C8, 문제 해결 ①~⑤, 자료구조·알고리즘, 파일 목록, 전체 구현 코드 순서로 작성한다.
LostArk 맵 에셋 검색·추출·`.wmodel` 변환·MapTool 적용 작업은 `.md/GB/07-29/2026-07-29_LOSTARK_MAP_ASSET_EXTRACTION_RUNTIME_RESULT.md`를 먼저 읽는다.

@AGENTS.md

## 프로젝트 개요

**LostArk** — C++로 직접 작성한 DirectX 11 3D 게임 엔진 프레임워크와 그 위에서 동작하는 클라이언트. **여러 명이 함께 작업하는 팀 프로젝트**이며, 엔진은 DLL로 빌드되어 클라이언트 EXE가 이를 링크해 사용한다.

혼자 쓰는 저장소가 아니다. 아래 "팀 협업 규칙"을 빌드/아키텍처만큼 중요하게 다룰 것.

## 빌드

**Visual Studio 2022**에서 `Framework.sln`을 연다. 실제로 빌드하는 프로젝트는 두 개다.

- `Engine\Default\Engine.vcxproj` — `Engine.dll` + `Engine.lib` 생성
- `Client\Default\Client.vcxproj` — 게임 EXE 생성, `Engine.lib`에 링크

(`Engine\External\imgui\examples\*`의 vcxproj들은 ImGui 원본에 딸려온 샘플이며 솔루션에 포함되지 않는다. 건드리지 않는다.)

### 빌드 순서 — 반드시 지킬 것

```
1) Engine 빌드
2) UpdateLib.bat [Debug|Release]   ← 인자 생략 시 Debug
3) Client 빌드
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

커맨드라인 빌드 스크립트(`BuildDebug.bat` 등)는 아직 없다. MSBuild로 직접 돌릴 경우에도 위 3단계를 그대로 따른다.

## 팀 협업 규칙

### 저장소 · 리소스 배포 정책

리소스가 세 갈래로 나뉘어 있다. 어디에 속하는지 먼저 확인하고 파일을 추가할 것.

| 종류 | 위치 | 배포 경로 |
|---|---|---|
| 소스 · 프로젝트 파일 | `Engine/`, `Client/`, `*.sln/.vcxproj/.filters` | Git 일반 추적 |
| 필수 바이너리 입력 | `Engine/ThirdPartyLib/`, `Client/Bin/DataFiles/`, `Client/Bin/*.dds` | **Git LFS** (`.gitattributes` 패턴) |
| 런타임 리소스 · 쿠킹 산출물 | `Client/Bin/Resources/`, `*.wmesh/.wmat/.wskel/.wanim` | **공유 Drive 팩** (Git 미추적, `.gitignore`) |

- clone 시 `git lfs install` 후 clone하거나, 이미 받았다면 `git lfs pull`을 실행해야 lib/DLL/DDS가 포인터가 아닌 실물이 된다.
- `Client/Bin/Resources/`는 clone만으로 채워지지 않는다. Drive 팩을 별도로 받아 배치해야 실행된다.
- 빌드 산출물(`exe/dll/lib/pdb/cso`), `.vs`, `EngineSDK`, `_work`, `out`, `imgui.ini`는 전부 ignore 대상이다. **커밋에 섞여 들어가지 않게 할 것.**
- 새 바이너리 자산을 추가할 때는 LFS 대상인지 Drive 팩 대상인지 먼저 판단하고, 애매하면 커밋하지 말고 물어본다.

### 브랜치 · PR

`main`이 정본이고, 작업은 `feature/<주제>` 브랜치에서 한 뒤 PR로 합친다. `main`에 직접 커밋하지 않는다.

### 프로젝트 파일(.vcxproj / .filters) 취급

- `.vcxproj`의 **설정(PropertyGroup, ItemDefinitionGroup, Import)과 고정 item은 사람이 관리하는 정본**이다.
- 소스 목록(`ClCompile`/`ClInclude`)과 `.filters`는 **물리 폴더 구조가 정본**이다. `.filters`는 Git으로 추적하되 사람이 손으로 편집하지 않는다.
- 파일 추가/이동은 물리 경로를 먼저 정하고, 그에 맞춰 프로젝트에 등록한다. 남의 필터 구조를 임의로 재배치하지 않는다.

### 소스 파일 인코딩 — 중요

`Engine/`, `Client/`의 **C++ 소스는 CP949(ANSI)로 저장되어 있고, 주석이 한국어다.** 편집할 때 기존 인코딩을 유지할 것. 파일 전체를 UTF-8로 다시 저장하면 한 줄만 고쳐도 전체가 변경된 것으로 보여 팀원의 diff/merge를 망가뜨린다.

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
│   │   ├── CVIBuffer_Rect / _Cube / _Terrain / _Cell
│   │   └── CVIBuffer_Instance ├─ _Rect / _Point (파티클 인스턴싱)
│   ├── CTexture              — SRV 배열, DDS/WIC 로딩
│   ├── CShader               — FX11 이펙트 래퍼
│   ├── CModel / CMesh / CMaterial / CBone / CAnimation / CChannel
│   ├── CCollider (+ CBounding_Sphere / _AABB / _OBB)
│   └── CNavigation / CCell
└── CGameObject (추상)
    ├── CContainerObject / CPartObject   — 파츠 조립형 오브젝트
    ├── CUIObject, CCamera
    └── [Client] CPlayer, CBody_Player, CWeapon, CMonster, CTerrain,
                 CSky, CSnow, CEffect, CExplosion, CForkLift,
                 CBackGround, CCamera_Free, CBinaryAssetObject
```

### 프로토타입 패턴

- **`Create()`** → `unique_ptr<T>`. `CPrototype_Manager`에 보관되는 원본.
- **`Clone()`** → `shared_ptr<CPrototype>`. 레이어에 배치되는 실제 인스턴스.
- `Initialize_Prototype()`은 원본에서, `Initialize(void* pArg)`는 각 사본에서 호출된다.
- 프로토타입은 레벨 인덱스로 구분되며, `LEVEL::STATIC`(0)은 모든 레벨이 공유한다.

### 레벨 · 레이어

레벨은 `Client_Defines.h`의 enum이다.

```cpp
enum class LEVEL { STATIC, LOADING, LOGO, GAMEPLAY, ASSET_TEST, END };
```

> **레벨 구성은 앞으로 계속 바뀐다.** 이 목록을 기억해서 쓰지 말고, 작업 시점에 `Client_Defines.h`를 직접 확인할 것. 레벨을 추가/제거할 때는 `CLoader::Ready_For_Level_*`, `CLevel_Loading`, `CMainApp::Start_Level`의 분기까지 함께 손봐야 하고, 다른 팀원의 레벨 작업과 충돌하기 쉬우므로 변경 사실을 공유한다.

`LEVEL::STATIC`은 전환 시에도 살아남는 영구 레벨이고, 나머지는 `Change_Level`에서 정리된다. 각 레벨 인덱스는 `map<wstring_t, shared_ptr<CLayer>>`를 가지며, `CLayer`는 `list<shared_ptr<CGameObject>>`를 들고 매 프레임 `Priority_Update → Update → Late_Update`를 구동한다.

### 레벨 전환 흐름

모든 전환은 `CLevel_Loading`을 거친다. `_beginthreadex`로 워커 스레드를 띄우고, `CLoader`가 그 안에서 프로토타입을 등록한다. 공유 상태는 `CRITICAL_SECTION`으로 보호한다. `m_isFinished == true`가 되면 목표 레벨로 전환한다.

### 바이너리 에셋 파이프라인

런타임 모델 입구는 `CModel` 하나로 통합한다. `CModel::Create()`는 FBX를 Assimp로 읽고, `.wmodel`은 `CWModelDecoder`로 읽은 뒤 모두 기존 `CMesh / CMaterial / CBone / CAnimation`으로 변환한다.

- **`CCookedModel`과 `CBinaryAssetObject`는 레거시 검증 경로다. 신규 기능을 추가하거나 MapTool·GameObject가 이 경로를 사용하게 하지 않는다.**
- `Engine/Public/BinaryAsset/`의 `CBinaryReader`, `IModelDecoder`, `CWModelDecoder` 등 decode 기반 코드는 `CModel` 내부의 `.wmodel` 입력을 지원한다.
- 신규 맵·캐릭터·보스 모델은 `CLoader -> CModel Prototype -> GameObject의 CModel Component` 계약을 사용한다.
- `.wmodel` 머티리얼에 diffuse와 emissive가 모두 없으면 `CMaterial`이 1×1 회색 diffuse를 만들어 형상 확인을 보장한다. 이는 안전망일 뿐이며 최종 에셋은 실제 텍스처 경로를 가져야 한다.
- 추출·스케일·텍스처 복구의 상세 주의사항은 `.md/GB/07-29/gotchas.md`를 따른다.

### 런타임 에셋 루트

`CRuntimeAssetRoot::Get()`이 Drive 공유 팩의 루트를 해석한다.

1. 환경 변수 `LOSTARK_SHARED_ASSET_ROOT`가 있으면 그 경로
2. 없으면 `<exe 폴더>/Resources/LostArk`

팀원마다 팩 위치가 다를 수 있으므로 **공유 에셋 경로를 코드에 하드코딩하지 말고 `CRuntimeAssetRoot::Resolve()`를 쓴다.**

### 디버그 툴 (ImGui / MapTool)

`_DEBUG` 빌드에서만 존재한다. `CMainApp`이 `Engine::CImGuiLayer`와 `Client::CMapTool`을 소유하고, **F1**으로 토글한다. ImGui가 입력을 가져갈 때는 `CGameInstance::SetInputBlocked()`로 DirectInput 폴링을 막는다(엔진은 WndProc이 아니라 폴링 기반이므로 이 게이트가 필요하다). Release 빌드 경로에 디버그 툴 의존성을 넣지 말 것.

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
- 데이터: `../Bin/DataFiles/*.dat` (내비게이션 등, LFS 추적)
- 로컬 리소스: `../Bin/Resources/...` — 텍스처 경로는 배열용 printf 번호 표기를 지원한다. 예: `TEXT("../Bin/Resources/Textures/Default%d.jpg")` + 개수 인자
- 공유 Drive 팩 에셋: `CRuntimeAssetRoot::Resolve()` 사용

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
- **다른 팀원이 담당하는 영역(다른 레벨, 다른 오브젝트, 공용 헤더)을 필요 없이 손대지 않는다.** 불가피하면 왜 손댔는지 명시한다.
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
- 소스 주석은 한국어이고 파일은 CP949다. 주변 주석의 언어와 파일 인코딩을 그대로 맞춘다.
- 빌드가 깨진 채로 커밋하지 않는다. `Engine/Public/` 변경 후에는 `UpdateLib.bat` → Client 빌드까지 확인한다.
