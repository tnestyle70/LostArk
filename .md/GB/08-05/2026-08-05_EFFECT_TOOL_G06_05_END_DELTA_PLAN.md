# Effect Tool G06-05~끝 DELTA PLAN

기준일: 2026-08-05

기준 브랜치: `codex/effect-tool-reboot`

이 문서는 기존 파일 전문을 다시 싣지 않는다. 현재 파일에서 **추가·교체·삭제되는
부분만** 적는다. 함수 본문 전체가 필요한 경우에도 먼저 선언 위치와 한 줄 책임,
호출 흐름을 적고, 변경되지 않는 기존 함수는 출력하지 않는다.

## 0. 현재 실측과 먼저 고칠 G06-4 세 줄

현재 `Effect_AuthoringDocument.h`에는 Detail struct가 들어갔지만 다음 세 항목이 남았다.
이 세 줄을 고친 뒤 G06-05를 시작한다.

### 0-1. 사용하지 않는 include 삭제

```text
파일: Client/Public/Effect_AuthoringDocument.h
작업: 삭제
기준점: #include <cstdint>와 #include <string> 사이
삭제 대상: #include <filesystem>
이유: Document 헤더는 enum/struct 데이터만 소유한다.
연결: 파일 경로 처리는 Effect_Tool.cpp가 소유한다.
```

### 0-2. 저장 형식 버전 교체

```text
파일: Client/Public/Effect_AuthoringDocument.h
작업: 교체
기준점: EFFECT_AUTHORING_FORMAT_VERSION
```

```cpp
inline constexpr uint32_t EFFECT_AUTHORING_FORMAT_VERSION = 4u;
inline constexpr uint32_t EFFECT_AUTHORING_MIN_SUPPORTED_VERSION = 3u;
```

### 0-3. UV 멤버 오타 교체

```text
파일: Client/Public/Effect_AuthoringDocument.h
작업: 이름 교체
기준점: struct EFFECT_DETAIL_DESC final
기존 -> 변경: EFFECT_UV_DESC VU; -> EFFECT_UV_DESC UV;
연결: Render_DetailPanel, JSON, Preview가 모두 Element.Detail.UV를 읽는다.
```

## 1. 이번 DELTA에서 추가·수정되는 파일

| 파일 | 작업 | 직접 의존 관계 |
|---|---|---|
| `Effect_AuthoringDocument.h` | 위 세 줄 교정만 | Tool과 Preview가 읽는 순수 데이터 |
| `Effect_Tool.h` | 선언·세션 변수 추가 | ImGui 입력을 Document와 Preview에 전달 |
| `Effect_Tool.cpp` | G05 잔여 단일 binding 교체, Detail JSON/UI 연결 | `CDataJson`, `CProjectDataRoot`, `CEffectPreview` |
| `Effect_Preview.h` | 현재 `#pragma once` stub 전체 교체 | Element 입력과 Preview GPU cache 계약 |
| `Effect_Preview.cpp` | 현재 0바이트 stub에 구현 추가 | `CModel`, `CShader`, `CVIBuffer_Rect`, DirectXTK DDS |
| Mesh/Rect Preview HLSL 2개 | 신규 | Preview CPP의 shader 상수와 pass 번호 |
| `MainApp.cpp` | 생성 한 줄 교체 | Device/Context를 Tool에 전달 |
| `.vcxproj/.filters` | HLSL 2개만 추가 | Preview H/CPP 등록은 이미 완료 |
| `ProjectAudit` | 기존 G4 Effect check 교체 | G06 구조와 project 등록 검사 |

새 외부 라이브러리는 추가하지 않는다. DirectXTK, D3D11, Engine의 `CModel`, `CShader`,
`CVIBuffer_Rect`는 이미 프로젝트가 사용하는 의존성이다.

## G06-05. 기존 `Effect_Tool.cpp`의 Document 처리 확장

새 `Effect_AuthoringDocument.cpp`를 만들지 않는다. 현재 `Effect_Tool.cpp`의 기존
anonymous namespace 안에서만 저장·로드 helper를 확장한다.

### G06-05-1. include 추가

```text
파일: Client/Private/Effect_Tool.cpp
작업: 추가
기준점: #include <cctype> 바로 아래
```

```cpp
#include <cmath>
#include <unordered_set>
```

- `<cmath>`: Detail float의 finite 검사와 UV wave 계산에 필요하다.
- `<unordered_set>`: Element ID와 resource slot 중복 검사에 필요하다.

### G06-05-2. resource token 배열 교체와 profile token 추가

```text
파일: Client/Private/Effect_Tool.cpp
작업: 기존 EFFECT_RESOURCE_SLOT_TOKENS 블록 교체
기준점: EFFECT_KIND_TOKENS 바로 아래
```

```cpp
constexpr const char_t* EFFECT_RESOURCE_SLOT_TOKENS[] = {
	"meshModel", "base", "noise", "mask", "emissive", "dissolve"
};

constexpr const char_t* EFFECT_RENDER_PROFILE_TOKENS[] = {
	"opaque_back_depth_write",
	"alpha_two_sided_depth_read",
	"additive_two_sided_depth_read"
};
```

- slot token: JSON의 `resources[].slot`과 `EFFECT_RESOURCE_SLOT`을 연결한다.
- profile token: JSON의 저장 이름을 shader pass 번호와 분리한다.

### G06-05-3. 기존 helper 교체·추가 목록

현재 anonymous namespace에서 다음 함수만 교체하거나 추가한다.

| 작업 | 함수 | 한 줄 책임 |
|---|---|---|
| 교체 | `To_EffectResourceSlotLabel` | 여섯 Material slot을 ImGui 이름으로 변환 |
| 교체 | `Try_ParseEffectResourceSlot` | 여섯 JSON slot token을 enum으로 변환 |
| 삭제 | `Try_GetResourceContract` | Element당 slot 하나만 허용하므로 더 이상 사용하지 않음 |
| 추가 | `Get_EffectResourceFileKind` | 선택 slot이 model인지 texture인지 결정 |
| 추가 | `Is_EffectResourceSlotAllowed` | Mesh Model은 Mesh만, texture slot은 모든 지원 Element에 허용 |
| 추가 | `To_EffectRenderProfileLabel` | profile enum을 ImGui 표시 이름으로 변환 |
| 추가 | `Try_ParseEffectRenderProfile` | JSON profile token을 enum으로 변환 |
| 추가 2개 | `Find_EffectResourceBinding` | Element의 binding vector에서 선택 slot 검색 |
| 추가 | `Try_ReadDetail` | JSON `detail` 객체를 `EFFECT_DETAIL_DESC` 후보에 입력 |
| 추가 | `Write_Detail` | `EFFECT_DETAIL_DESC`를 JSON `detail` 객체로 출력 |

기존 `Try_GetResourceFileKind(FilePath, OutKind)`는 실제 파일 확장자를 분류하므로
유지한다. 새 `Get_EffectResourceFileKind(Slot)`은 선택 slot이 요구하는 파일 종류를
알려주는 별도 역할이다.

추가 함수 선언 모양은 다음과 같다. 모두 `Effect_Tool.cpp` 내부 전용이며 헤더에
선언하지 않는다.

```cpp
Client::EFFECT_RESOURCE_FILE_KIND Get_EffectResourceFileKind(
	Client::EFFECT_RESOURCE_SLOT eSlot);

bool_t Is_EffectResourceSlotAllowed(
	Client::EFFECT_ELEMENT_KIND eKind,
	Client::EFFECT_RESOURCE_SLOT eSlot);

const char_t* To_EffectRenderProfileLabel(
	Client::EFFECT_RENDER_PROFILE eProfile);

bool_t Try_ParseEffectRenderProfile(
	const std::string& strValue,
	Client::EFFECT_RENDER_PROFILE& eOutProfile);

Client::EFFECT_RESOURCE_BINDING_DESC* Find_EffectResourceBinding(
	Client::EFFECT_ELEMENT_DESC& Element,
	Client::EFFECT_RESOURCE_SLOT eSlot);

const Client::EFFECT_RESOURCE_BINDING_DESC* Find_EffectResourceBinding(
	const Client::EFFECT_ELEMENT_DESC& Element,
	Client::EFFECT_RESOURCE_SLOT eSlot);

bool_t Try_ReadDetail(
	const Client::DATA_JSON_VALUE& Value,
	Client::EFFECT_DETAIL_DESC& OutDetail,
	std::string& strOutError);

void Write_Detail(
	std::ostringstream& Output,
	const Client::EFFECT_DETAIL_DESC& Detail);
```

`Try_ReadDetail` 내부 흐름:

```text
detail의 transform/color/uv/timing/mesh/sprite/decal object 읽기
→ 숫자·bool·2/3/4개 배열을 Detail 후보에 입력
→ 필드가 없거나 타입이 다르면 false
→ Active Document는 변경하지 않음
```

`Write_Detail` 내부 흐름:

```text
Element.Detail 읽기
→ transform/color/uv/timing/type detail 순서로 JSON 출력
→ 파일 교체는 하지 않음
→ 실제 파일 commit은 Try_SaveDocument가 담당
```

### G06-05-4. `Validate_EffectDocument` 변경 부분

```text
파일: Client/Private/Effect_Tool.cpp
작업: 함수 내부의 단일 ResourceBinding 검사 블록 교체
기준점: const EFFECT_RESOURCE_BINDING_DESC& Binding = Element.ResourceBinding;
교체 끝: 해당 binding 검사 뒤의 continue/실패 블록 전체
```

새 흐름:

```text
Element ID/kind 중복 검사
→ Material.eRenderProfile이 END인지 검사
→ ResourceBindings를 순회
→ slot 중복 검사
→ Element kind가 slot을 받을 수 있는지 검사
→ slot과 확장자(.wmodel/.dds)가 맞는지 검사
→ Detail 수치 범위 검사
→ 하나라도 실패하면 false, 기존 Document 유지
```

Detail에서 추가 검사하는 값만 적는다.

```text
Transform: 모든 값 finite, Scale 각 축 > 0
Color: 모든 값 finite, clip 0~1, emissive/distortion >= 0
UV: 모든 값 finite, sequenceTerm > 0, tile row/column >= 1,
    tileIndex < row * column
Timing: delay/afterImage >= 0, lifetime > 0, dissolveStart 0~1
Decal: size 각 축 > 0, depth > 0
```

### G06-05-5. `Parse_EffectDocumentJson` 변경 부분

```text
파일: Client/Private/Effect_Tool.cpp
작업: 기존 함수 내부 version/Element resource 처리 교체
기준점: const double fVersion = pVersion->Get_Number();
기존 지원: v1/v2, resource 단일 object
변경 지원: v3/v4, resources 배열 + material + 선택적 detail
```

새 흐름:

```text
Root parse
→ version이 3 또는 4인지 확인
→ StagedDocument 생성
→ Element id/kind 읽기
→ resources 배열을 ResourceBindings에 입력
→ material.renderProfile 입력
→ v4면 detail 필수로 읽기
→ v3면 기본 생성된 Element.Detail 유지
→ Validate_EffectDocument
→ 전부 성공하면 OutDocument 한 번 교체
```

여기서 `detail`은 ImGui를 그리기 위한 파싱이 아니다. Save한 값을 다시 Load하기 위한
파일 경계에서만 읽는다.

### G06-05-6. `Serialize_EffectDocumentJson` 변경 부분

```text
파일: Client/Private/Effect_Tool.cpp
작업: Element의 단일 "resource" 출력 블록 교체
기준점: "resource": 를 출력하는 기존 block
```

새 출력 순서:

```text
id
kind
resources[]: slot + assetId
material: renderProfile
detail: transform + color + uv + timing + mesh + sprite + decal
```

### G06-05-7. Save/Load 상태 문구만 교체

`Try_SaveDocument()` 마지막 성공 문구:

```cpp
m_strDocumentStatus = "Saved v4 Effect document.";
```

`Try_LoadDocument()` 성공 분기:

```text
source v3 → 기본 Detail을 가진 v4 메모리 Document로 load
source v4 → Detail까지 load
두 경우 모두 parse/validate 성공 뒤 m_ActiveDocument 교체
```

## G06-06. `Effect_Preview.h` stub 교체

```text
파일: Client/Public/Effect_Preview.h
작업: 기존 #pragma once 한 줄 stub 전체 교체
소유: Preview render target과 GPU resource cache
소유하지 않음: Active Document, 자동 증가 시간, 제품 Runtime Effect
직접 호출자: CEffect_Tool
```

### 추가 include

```cpp
#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Effect_AuthoringDocument.h"

#include <array>
#include <string>
```

### Engine 전방 선언

```cpp
NS_BEGIN(Engine)
class CModel;
class CShader;
class CVIBuffer_Rect;
NS_END
```

### 추가할 class와 함수 헤더

```cpp
NS_BEGIN(Client)

class CEffectPreview final
{
public:
	CEffectPreview(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	~CEffectPreview() = default;

	HRESULT Initialize();
	HRESULT Render(
		const EFFECT_ELEMENT_DESC& Element,
		f32_t fSampleTimeSeconds,
		uint32_t iWidth,
		uint32_t iHeight);
	void Clear();

	ID3D11ShaderResourceView* Get_TextureView() const;
	const std::string& Get_Status() const;

private:
	HRESULT Ensure_RenderTarget(uint32_t iWidth, uint32_t iHeight);
	HRESULT Ensure_Resources(const EFFECT_ELEMENT_DESC& Element);
	HRESULT Create_FallbackTextures();
	HRESULT Load_Texture(
		const std::string& strAssetId,
		ComPtr<ID3D11ShaderResourceView>& OutSRV) const;
	HRESULT Bind_CommonShaderValues(
		const shared_ptr<Engine::CShader>& pShader,
		const EFFECT_ELEMENT_DESC& Element,
		f32_t fSampleTimeSeconds,
		const float4x4_t& World,
		const float4x4_t& View,
		const float4x4_t& Projection);
	HRESULT Render_Mesh(
		const EFFECT_ELEMENT_DESC& Element,
		f32_t fSampleTimeSeconds);
	HRESULT Render_Rect(
		const EFFECT_ELEMENT_DESC& Element,
		f32_t fSampleTimeSeconds);
	std::string Build_ResourceSignature(
		const EFFECT_ELEMENT_DESC& Element) const;
	uint32_t Select_Pass(EFFECT_RENDER_PROFILE eProfile) const;

private:
	ComPtr<ID3D11Device> m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext> m_pContext = { nullptr };

	ComPtr<ID3D11Texture2D> m_pColorTexture = { nullptr };
	ComPtr<ID3D11RenderTargetView> m_pColorRTV = { nullptr };
	ComPtr<ID3D11ShaderResourceView> m_pColorSRV = { nullptr };
	ComPtr<ID3D11Texture2D> m_pDepthTexture = { nullptr };
	ComPtr<ID3D11DepthStencilView> m_pDepthDSV = { nullptr };
	uint32_t m_iTargetWidth = 0u;
	uint32_t m_iTargetHeight = 0u;

	shared_ptr<Engine::CShader> m_pMeshShader;
	shared_ptr<Engine::CShader> m_pRectShader;
	shared_ptr<Engine::CModel> m_pModel;
	shared_ptr<Engine::CVIBuffer_Rect> m_pRect;

	std::array<ComPtr<ID3D11ShaderResourceView>, 5> m_InputTextures;
	ComPtr<ID3D11ShaderResourceView> m_pWhiteTexture;
	ComPtr<ID3D11ShaderResourceView> m_pBlackTexture;
	std::string m_strResourceSignature;
	std::string m_strStatus =
		"Select a Mesh, Sprite, or Decal Element.";
};

NS_END
```

### 추가할 멤버 변수

위 class의 마지막 `private:` 구역에 다음 상태를 둔다.

| 멤버 | 한 줄 의미 |
|---|---|
| `m_pDevice`, `m_pContext` | MainApp에서 받은 D3D 장치 참조 |
| `m_pColorTexture/RTV/SRV` | ImGui에 표시할 off-screen 색 결과 |
| `m_pDepthTexture/DSV` | Preview 전용 depth |
| `m_iTargetWidth/Height` | 현재 target 크기 |
| `m_pMeshShader`, `m_pRectShader` | Mesh와 Rect shader cache |
| `m_pModel`, `m_pRect` | 현재 model과 공용 rect geometry |
| `m_InputTextures[5]` | Base/Noise/Mask/Emissive/Dissolve SRV |
| `m_pWhiteTexture`, `m_pBlackTexture` | 미지정 입력 fallback |
| `m_strResourceSignature` | binding 변경 여부 비교값 |
| `m_strStatus` | Preview 성공·실패 문구 |

## G06-07. `Effect_Preview.cpp` stub 구현

```text
파일: Client/Private/Effect_Preview.cpp
작업: 현재 0바이트 파일에 추가
직접 입력: EFFECT_ELEMENT_DESC와 명시적인 Sample Time
직접 출력: Preview SRV와 상태 문자열
```

### 추가 include와 이유

| include | 이유 |
|---|---|
| `Effect_Preview.h` | class 정의 |
| `BinaryAsset/ModelAssetData.h` | Mesh shader input layout |
| `DirectXTK/DDSTextureLoader.h` | DDS를 SRV로 로드 |
| `Model.h` | `.wmodel` 생성·렌더 |
| `RuntimeAssetRoot.h` | `Effect/`로 시작하는 Resources-relative ID 해석 |
| `Shader.h` | 상수·texture bind와 pass 시작 |
| `VIBuffer_Rect.h` | Sprite/Decal 사각형 |
| `<algorithm>` | clamp와 binding 검색 |
| `<cmath>` | sin, sqrt |
| `<filesystem>` | asset ID를 path로 전달 |
| `<sstream>` | resource signature 생성 |
| `<utility>` | GPU 후보 commit 시 move |

### 파일 내부 helper

| 함수 | 한 줄 책임 |
|---|---|
| `Quantize_TargetSize` | target을 256~1024, 64단위로 맞춤 |
| `Texture_Index` | Base~Dissolve slot을 0~4 index로 변환 |
| `Find_Texture` | slot에 대응하는 cached SRV 반환 |
| `Find_Binding` | Element.ResourceBindings에서 slot 검색 |

이 helper들은 `EffectPreviewDetail` namespace에만 두고 다른 파일에 공개하지 않는다.

### class 함수 한 줄 책임과 흐름

| 함수 | 한 줄 책임 | 내부 흐름 |
|---|---|---|
| 생성자 | Device/Context 보관 | MainApp → Tool → Preview |
| `Initialize` | shader, rect, fallback 생성 | 전부 후보 생성 → 성공 후 멤버 commit |
| `Clear` | 선택 Element GPU cache 제거 | model/texture/signature reset |
| `Ensure_RenderTarget` | 크기가 바뀔 때 target 재생성 | color/depth 후보 생성 → 전부 성공 후 교체 |
| `Build_ResourceSignature` | kind와 binding을 비교 문자열로 생성 | cache reload 여부 판단 |
| `Load_Texture` | Resources-relative DDS를 SRV로 로드 | RuntimeAssetRoot → DDSTextureLoader |
| `Ensure_Resources` | model/texture cache stage 후 commit | 하나라도 실패하면 이전 cache 유지 |
| `Select_Pass` | profile을 0/1/2로 변환 | 알 수 없는 값은 유효 pass로 대체하지 않음 |
| `Bind_CommonShaderValues` | 시간에 따른 UV/dissolve와 색 상수 bind | Detail 읽기 → local 계산 → shader bind |
| `Render_Mesh` | CModel을 Mesh shader로 그림 | bounds camera → world → material/base → meshes |
| `Render_Rect` | Sprite/Decal rect를 그림 | type scale → world → base texture → rect render |
| `Render` | target과 viewport를 보존하며 한 장 렌더 | resource → target → draw → 원래 상태 복구 |
| `Get_TextureView` | ImGui가 읽을 SRV 반환 | 상태 변경 없음 |
| `Get_Status` | 마지막 상태 문구 반환 | 상태 변경 없음 |

`Render`의 새 연결 흐름:

```text
Render_PreviewPanel
→ CEffectPreview::Render(Element, SampleTime, Width, Height)
→ Ensure_Resources
→ Ensure_RenderTarget
→ 이전 RTV/DSV/Viewport 저장
→ Preview target clear
→ Start Delay가 지났으면 Mesh 또는 Rect render
→ 이전 RTV/DSV/Viewport 복구
→ ImGui::Image(Get_TextureView)
```

Particle/Trail은 G08 module이 없으므로 `S_FALSE`와 상태 문구만 반환한다.

## G06-08. Mesh Preview HLSL 신규

```text
파일: Client/Bin/ShaderFiles/Shader_VtxEffectMeshPreview.hlsl
작업: 새 파일
직접 호출자: CEffectPreview::Initialize / Render_Mesh
```

추가되는 shader 입력:

```text
World/View/Projection
Base/Noise/Mask/Emissive/Dissolve Texture2D
UVScale/UVOffset
ColorOffset/ColorMultiply/ColorClip
EmissiveIntensity/DissolveAmount
UseBaseOverride와 texture 존재 flag
```

추가 함수:

- `VS_MAIN`: Mesh vertex와 UV를 변환한다.
- `Shade_Effect`: texture, mask, dissolve, color, lighting, emissive를 계산한다.
- `PS_MAIN`: `Shade_Effect` 결과를 출력한다.

Technique pass 순서는 고정한다.

```text
0 OpaqueBackDepthWrite
1 AlphaTwoSidedDepthRead
2 AdditiveTwoSidedDepthRead
```

## G06-09. Rect Preview HLSL 신규

```text
파일: Client/Bin/ShaderFiles/Shader_VtxEffectRectPreview.hlsl
작업: 새 파일
직접 호출자: CEffectPreview::Initialize / Render_Rect
```

- 입력 상수와 texture는 Mesh shader와 동일하다.
- `VS_MAIN`: Rect 위치와 UV를 변환한다.
- `PS_MAIN`: lighting 없이 texture, mask, dissolve, color, emissive를 계산한다.
- pass 0/1/2 순서는 Mesh shader와 반드시 동일하다.

## G06-10. `Effect_Tool.h` 선언 delta

### G06-10-1. 잘못된 전방 선언 교체

```text
파일: Client/Public/Effect_Tool.h
작업: 이름 교체
기준점: NS_BEGIN(Client) 바로 아래
기존 -> 변경: class CClientPreview; -> class CEffectPreview;
```

`<memory>` include는 이미 반영됐으므로 다시 추가하지 않는다.

### G06-10-2. 생성자·소멸자 교체

```text
기준점: public의 CEffect_Tool()와 ~CEffect_Tool()
작업: 기존 두 선언 교체
```

```cpp
CEffect_Tool(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext);
~CEffect_Tool();
```

소멸자를 CPP에서 정의하는 이유는 `unique_ptr<CEffectPreview>`가 전방 선언 타입이기 때문이다.

### G06-10-3. Panel 함수 선언 교체·추가

```text
기준점: void Render_ElementList(); 바로 아래
작업: Render_ResourceCatalogPanel 이름 교체 후 두 선언 추가
```

```cpp
void Render_MaterialPanel();
void Render_DetailPanel();
void Render_PreviewPanel();
```

- `Render_MaterialPanel`: slot/resource/profile 편집.
- `Render_DetailPanel`: 선택 Element의 Detail 후보 편집.
- `Render_PreviewPanel`: sample time과 Preview SRV 표시.

### G06-10-4. Apply 함수 선언 추가

```text
기준점: bool_t Try_ClearSelectedResource(); 바로 아래
```

```cpp
bool_t Try_ApplyRenderProfile(EFFECT_RENDER_PROFILE eProfile);
bool_t Try_ApplyDetail(const EFFECT_DETAIL_DESC& Detail);
```

### G06-10-5. Tool 세션 멤버 추가

`m_eSelectedEffectType` 바로 아래:

```cpp
EFFECT_RESOURCE_SLOT m_eSelectedResourceSlot =
	EFFECT_RESOURCE_SLOT::MESH_MODEL;
```

`m_bResourceCatalogRefreshAttempted` 바로 아래:

```cpp
unique_ptr<CEffectPreview> m_pPreview;
f32_t m_fPreviewTimeSeconds = 0.f;
std::string m_strDetailStatus;
```

- 선택 slot, preview time, status는 저장 Document가 아니라 Tool session 상태다.

## G06-11. `Effect_Tool.cpp` UI와 Preview 연결 delta

### G06-11-1. include 추가

```text
기준점: #include "Effect_Tool.h" 바로 아래
```

```cpp
#include "Effect_Preview.h"
```

### G06-11-2. 생성자·소멸자 정의 추가

```text
위치: anonymous namespace 닫는 } 바로 아래, Render() 바로 위
```

```cpp
Client::CEffect_Tool::CEffect_Tool(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: m_pPreview(std::make_unique<CEffectPreview>(
		std::move(pDevice), std::move(pContext)))
{
	if (FAILED(m_pPreview->Initialize()))
		m_strDetailStatus = m_pPreview->Get_Status();
}

Client::CEffect_Tool::~CEffect_Tool() = default;
```

생성자는 device/context로 Preview를 만들고 초기화 실패 문구만 보존한다.

### G06-11-3. `Render` 변경 부분

```text
창 ID: LostArkEffectToolG4 -> LostArkEffectToolG6
창 기본 크기: 920x680 -> 1240x820
안내 문구: G4 resource-only -> G6 Detail + Preview
```

Active Document가 있을 때의 마지막 UI 연결만 다음 순서로 교체한다.

```cpp
Render_AddElementPanel();
Render_ElementList();

if (ImGui::BeginTable("EffectToolWorkArea", 2,
	ImGuiTableFlags_SizingStretchProp |
	ImGuiTableFlags_BordersInnerV))
{
	ImGui::TableNextColumn();
	Render_MaterialPanel();
	Render_DetailPanel();
	ImGui::TableNextColumn();
	Render_PreviewPanel();
	ImGui::EndTable();
}
```

### G06-11-4. `Render_ElementList` 변경 부분

```text
Table column: Resource 한 칸 -> Bindings 개수 + Render Profile
Element 선택 시 추가 상태 변경:
  m_eSelectedResourceSlot을 Mesh Model 또는 Base로 설정
  m_fPreviewTimeSeconds = 0
  m_pPreview->Clear()
```

기존 `Element.ResourceBinding` 출력은 삭제하고 다음 값만 출력한다.

```cpp
Element.ResourceBindings.size()
To_EffectRenderProfileLabel(Element.Material.eRenderProfile)
```

### G06-11-5. `Render_ResourceCatalogPanel` 전체 이름·책임 교체

```text
기존 함수: Render_ResourceCatalogPanel
변경 함수: Render_MaterialPanel
```

새 흐름:

```text
선택 Element 검색
→ Render Profile combo
→ Element kind에 허용된 Input Slot combo
→ 현재 slot binding 표시
→ Resource catalog에서 slot의 file kind만 표시
→ Bind 또는 Clear 명령 제출
```

### G06-11-6. `Render_DetailPanel` 추가

```text
위치: Render_MaterialPanel 정의 바로 아래
```

흐름:

```text
선택 Element의 Detail을 local Staged에 복사
→ Transform/Color/UV/Timing ImGui가 Staged만 변경
→ Mesh/Sprite/Decal type detail 변경
→ 값이 바뀐 프레임에 Try_ApplyDetail(Staged)
```

ImGui에 직접 연결되는 필드는 다음뿐이다.

```text
Transform: Position, Rotation, Revolution, Scale
Color: Offset, Multiply, Clip, Emissive, Distortion, Radial
UV: Start, Speed, Wave, Sequence, Loop, Tile
Timing: Start Delay, Lifetime, After Image, Dissolve Start
Mesh: Use Model Material
Sprite: Billboard
Decal: Size, Depth
```

### G06-11-7. `Render_PreviewPanel` 추가

```text
위치: Render_DetailPanel 정의 바로 아래
```

흐름:

```text
선택 Element 검색
→ max sample time = delay + lifetime + afterImage
→ Sample Time slider
→ CEffectPreview::Render
→ 성공 SRV만 ImGui::Image로 표시
→ Preview status 표시
```

### G06-11-8. 기존 명령 함수에 추가되는 reset만

다음 함수의 성공 commit 뒤에 필요한 경우만 추가한다.

| 함수 | 추가 상태 변경 |
|---|---|
| `Try_CreateDocument` | selected ID 비우기, sample time 0, Preview Clear |
| `Try_AddElement` | 새 Element 선택, 기본 slot 설정, sample time 0, Preview Clear |
| `Try_LoadDocument` | selected ID 비우기, sample time 0, Preview Clear |
| `Try_BindSelectedResource` | Preview Clear |
| `Try_ClearSelectedResource` | Preview Clear |
| `Discard_ActiveDocument` | detail status 비우기, sample time 0, Preview Clear |

### G06-11-9. `Try_BindSelectedResource` 변경 흐름

기존 `Try_GetResourceContract`와 단일 `ResourceBinding` 대입을 삭제한다.

```text
선택 slot의 expected file kind 계산
→ catalog 선택 asset의 file kind 확인
→ Document 전체 stage
→ 선택 Element 검색
→ 같은 slot의 기존 binding 삭제
→ { selected slot, asset ID } push_back
→ Validate
→ 성공 시 Active Document와 Preview cache 교체
```

### G06-11-10. `Try_ClearSelectedResource` 변경 흐름

기존 `pStagedElement->ResourceBinding = {};` 한 줄을 삭제한다.

```text
Document 전체 stage
→ 선택 Element의 ResourceBindings에서 selected slot만 erase
→ Validate
→ 성공 commit
→ Preview Clear
```

### G06-11-11. 새 Apply 함수

`Try_ClearSelectedResource` 정의 바로 아래에 추가한다.

- `Try_ApplyRenderProfile`: Document stage → 선택 Element profile 교체 → validate → commit.
- `Try_ApplyDetail`: Document stage → 선택 Element Detail 교체 → validate → commit, 실패 시 기존 값 유지.

## G06-12. `MainApp.cpp` 한 줄 교체

```text
파일: Client/Private/MainApp.cpp
함수: CMainApp::EnsureDebugTool
기준점: case DEBUG_TOOL::EFFECT
```

```text
기존: make_unique<CEffect_Tool>()
변경: make_unique<CEffect_Tool>(m_pDevice, m_pContext)
```

## G06-13. project/filter delta

현재 다음 항목은 이미 등록 완료다. 다시 추가하지 않는다.

```text
Effect_Preview.h: ClInclude + Effect filter
Effect_Preview.cpp: ClCompile + Effect filter
```

`Client.vcxproj`의 기존 `FxCompile` ItemGroup에 두 줄만 추가한다.

```xml
<FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshPreview.hlsl" />
<FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectRectPreview.hlsl" />
```

`Client.vcxproj.filters`의 `97.ShaderFiles` ItemGroup에 두 블록만 추가한다.

```xml
<FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshPreview.hlsl">
  <Filter>97.ShaderFiles</Filter>
</FxCompile>
<FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectRectPreview.hlsl">
  <Filter>97.ShaderFiles</Filter>
</FxCompile>
```

`Effect_AuthoringDocument.cpp`는 생성하거나 등록하지 않는다.

## G06-14. ProjectAudit delta

```text
파일: Tools/ProjectAudit/Invoke-ProjectAudit.ps1
작업: 기존 effect.g4-typed-resource-boundary block 교체
기준점: $effectG4TypedResourceShape 선언부터 해당 Add-Check 끝까지
```

기존 Effect 파일 읽기 세 줄 바로 아래에 다음 읽기만 추가한다.

```powershell
$effectPreviewHeader = Get-Content -LiteralPath 'Client\Public\Effect_Preview.h' -Raw
$effectPreviewSource = Get-Content -LiteralPath 'Client\Private\Effect_Preview.cpp' -Raw
$effectMeshShader = Get-Content -LiteralPath 'Client\Bin\ShaderFiles\Shader_VtxEffectMeshPreview.hlsl' -Raw
$effectRectShader = Get-Content -LiteralPath 'Client\Bin\ShaderFiles\Shader_VtxEffectRectPreview.hlsl' -Raw
$mainAppSource = Get-Content -LiteralPath 'Client\Private\MainApp.cpp' -Raw
```

새 check가 확인할 항목:

```text
Document format 4, minimum 3
ResourceBindings vector와 Material/Detail struct
Document header에 filesystem/public parse 함수 없음
Tool CPP가 validation/parse/serialize와 Detail/Preview panel 소유
Preview가 CModel, Ensure_Resources, Ensure_RenderTarget, Select_Pass 사용
MainApp이 device/context 전달
두 shader pass 순서 0/1/2
Preview H/CPP와 두 HLSL project 등록
Effect_AuthoringDocument.cpp project 등록 없음
```

교체할 실제 shape block은 다음과 같다.

```powershell
$effectG6Shape =
	$effectDocumentHeader -match 'EFFECT_AUTHORING_FORMAT_VERSION\s*=\s*4u' -and
	$effectDocumentHeader -match 'EFFECT_AUTHORING_MIN_SUPPORTED_VERSION\s*=\s*3u' -and
	$effectDocumentHeader -match 'vector<EFFECT_RESOURCE_BINDING_DESC>\s+ResourceBindings' -and
	$effectDocumentHeader -match 'struct EFFECT_MATERIAL_DESC' -and
	$effectDocumentHeader -match 'struct EFFECT_DETAIL_DESC' -and
	$effectDocumentHeader -match 'EFFECT_UV_DESC\s+UV' -and
	$effectDocumentHeader -notmatch '#include\s*<filesystem>' -and
	$effectDocumentHeader -notmatch 'ParseEffectDocument|Serialize_EffectDocument' -and
	$effectToolSource -match 'Validate_EffectDocument' -and
	$effectToolSource -match 'Parse_EffectDocumentJson' -and
	$effectToolSource -match 'Serialize_EffectDocumentJson' -and
	$effectToolSource -match 'Render_MaterialPanel' -and
	$effectToolSource -match 'Render_DetailPanel' -and
	$effectToolSource -match 'Render_PreviewPanel' -and
	$effectPreviewHeader -match 'class CEffectPreview' -and
	$effectPreviewSource -match 'CModel::Create' -and
	$effectPreviewSource -match 'Ensure_Resources' -and
	$effectPreviewSource -match 'Ensure_RenderTarget' -and
	$effectPreviewSource -match 'Select_Pass' -and
	$mainAppSource -match 'make_unique<CEffect_Tool>\(\s*m_pDevice,\s*m_pContext\s*\)' -and
	$effectMeshShader -match 'pass OpaqueBackDepthWrite[\s\S]*pass AlphaTwoSidedDepthRead[\s\S]*pass AdditiveTwoSidedDepthRead' -and
	$effectRectShader -match 'pass OpaqueBackDepthWrite[\s\S]*pass AlphaTwoSidedDepthRead[\s\S]*pass AdditiveTwoSidedDepthRead' -and
	$clientProjectSource -match 'Effect_Preview\.cpp' -and
	$clientProjectSource -match 'Shader_VtxEffectMeshPreview\.hlsl' -and
	$clientProjectSource -match 'Shader_VtxEffectRectPreview\.hlsl' -and
	$clientProjectSource -notmatch 'Effect_AuthoringDocument\.cpp'

Add-Check 'effect.g6-detail-stateless-preview-boundary' (
	$removedEffectPathHits.Count -eq 0 -and
	$unexpectedAuthoredEffectFiles.Count -eq 0 -and
	$effectIntakeFiles.Count -eq 0 -and
	$legacyEffectSymbolHits.Count -eq 0 -and
	$legacyEffectProjectHits.Count -eq 0 -and
	-not $legacyEffectEntry -and
	$effectG6Shape) "effectG6=$effectG6Shape"
```

## G06-15. 직접 반영 순서

1. `Effect_AuthoringDocument.h`의 version, filesystem, `VU -> UV` 세 줄을 고친다.
2. `Effect_Tool.cpp`에서 단일 binding JSON/validation을 vector/material/detail로 바꾼다.
3. `Effect_Preview.h` stub에 class 선언과 멤버를 쓴다.
4. Mesh/Rect HLSL을 추가하고 pass 순서를 맞춘다.
5. `Effect_Preview.cpp`를 함수 하나씩 구현한다.
6. Tool H의 잘못된 forward declaration과 생성자를 고친다.
7. Tool CPP에 Material/Detail/Preview panel을 연결한다.
8. MainApp에서 device/context를 전달한다.
9. HLSL 두 개를 project/filter에 등록한다.
10. ProjectAudit Effect block을 G06 형태로 교체한다.

## G06-16. 자동 검증

계획서 작성 시점에는 실행 완료로 기록하지 않는다. 구현 후 실행한다.

```powershell
$project = [xml](Get-Content -LiteralPath 'Client/Default/Client.vcxproj' -Raw)
$filters = [xml](Get-Content -LiteralPath 'Client/Default/Client.vcxproj.filters' -Raw)
& 'Tools/Build/Invoke-BuildAndRegression.ps1' -Configuration Debug
& 'Tools/ProjectAudit/Invoke-ProjectAudit.ps1'
git diff --check
```

빠른 소스 점검:

```powershell
rg -n "ResourceBinding\b|\bVU\b|CClientPreview|LostArkEffectToolG4" `
  Client/Public/Effect_AuthoringDocument.h `
  Client/Public/Effect_Tool.h `
  Client/Private/Effect_Tool.cpp
```

기대 결과는 이전 단일 멤버 `ResourceBinding`, 오타 `VU`, 잘못된 class 이름
`CClientPreview`, G4 window ID가 0건이다. `ResourceBindings`는 정상이다.

## G06-17. F1 Runtime smoke

Client 시작 abort가 다른 세션에서 해결된 뒤 수동 확인한다.

```text
F1 → Effect Tool
Document Create 또는 Load
Element 추가·선택
왼쪽: Material + Transform/Color/UV/Timing
오른쪽: Sample Time + 정사각형 Preview
Mesh + model: CModel 표시
Sprite/Decal + Base: texture rect 표시
Particle/Trail: G08 required 문구만 표시
잘못된 asset: 오류 문구와 직전 성공 Preview 유지
Save → Discard → Load: binding/profile/detail 동일
```

## G06-18. 여기서 끝나는 경계

```text
G06: 정지 Sample Time의 Mesh/Sprite/Decal Preview까지
G07: play/stop/loop와 curve
G08: Particle/Trail module과 After Image 실제 소비
G08: Particle/Trail과 함께 Distortion composite 실제 소비
```
