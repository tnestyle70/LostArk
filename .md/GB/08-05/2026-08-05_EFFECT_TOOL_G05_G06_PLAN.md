# Effect Tool G05-G06 전체 반영 PLAN

기준일: 2026-08-05

기준 브랜치: `codex/effect-tool-reboot`

기준 코드: G04의 `Effect_AuthoringDocument.h`, `Effect_Tool.h`,
`Effect_Tool.cpp`가 반영된 상태

이 문서는 구현 결과가 아니다. 사용자가 G05와 G06을 직접 반영할 때 비교할
코드 정본과 검증 순서를 고정한다. 다른 세션에서 조사 중인 Client 시작 직후
`abort() has been called` 문제는 이 계획의 범위가 아니며, 이 계획 때문에
`Client.exe`를 자동 실행하지 않는다.

## 0. 먼저 바로잡을 구조

질문에 적힌 모든 값이 Mesh, Particle, Decal, Texture마다 각각 복제되는 것은 아니다.
본질은 아래 세 층이다.

```text
선택한 Effect Element
├─ Common Detail
│  ├─ Position / Rotation / Revolution / Scaling
│  ├─ ColorOffset / ColorMultiply / ColorClip
│  ├─ UV Start / Speed / Wave / Sequence / Loop / Tile
│  └─ Start Delay / Life Time / After Image / Dissolve Start
├─ Material
│  ├─ Base / Noise / Mask / Emissive / Dissolve
│  └─ Render Profile(저장 이름) -> 실제 Technique 0 Pass 번호
└─ Type Detail
   ├─ Mesh: CModel과 model material 사용 여부
   ├─ Sprite(Texture): Billboard와 사각형 표시
   ├─ Decal: 표면 카드 크기와 깊이
   ├─ Particle: G08에서 emitter/module을 추가
   └─ Trail: G08에서 segment/after-image module을 추가
```

이번 범위는 G06에서 닫는다. G05 저장 계약과 G06 정지 시점 미리보기를 각각
검증해 실패 지점을 구분한다. Timeline/Curve는 이 문서에 넣지 않고 다음 G07
문서에서 시작한다.

`Pass Name`은 `0`, `1`, `2` 같은 숫자를 저장하지 않는다. JSON에는
`opaque_back_depth_write`, `alpha_two_sided_depth_read`,
`additive_two_sided_depth_read` 중 하나를 저장하고, Preview가 그 이름을 실제 셰이더
Pass 번호로 번역한다. 셰이더의 Pass 순서가 바뀌어도 문서 의미를 보존하기 위해서다.

## 1. 폴더와 소유권

| 위치 | 소유하는 것 | 이유 |
|---|---|---|
| `Client/Public`, `Client/Private`의 Effect 파일 | F1 개발 도구의 데이터, UI, Preview | ImGui Effect Tool은 Client 개발 도구이기 때문이다. |
| `Client/Bin/ShaderFiles` | Preview 전용 HLSL | GPU 표현만 담당하고 JSON 의미를 소유하지 않는다. |
| `Data/Effects/Authored` | 저장한 `.effect.json` 원본 | 팀이 편집하는 authoring 정본이다. |
| `Client/Bin/Resources/Effect` | `.wmodel`, `.dds` payload | JSON에는 이 루트 기준 `Effect/...` ID만 저장한다. |
| `Tools/ProjectAudit` | 구조가 다시 섞이지 않았는지 검사 | 빌드만으로 잡히지 않는 stable ID와 project 등록 경계를 확인한다. |

모든 기능이 물리적인 `Tools/` 디렉터리 안에서 실행되는 것은 아니다. 화면과 Preview는
Client의 `03.Tools\02.Effect` 필터에 속하고, 자동 검증 스크립트만 저장소의
`Tools/ProjectAudit`에 있다. 제품 런타임과 별도의 모델 시스템은 만들지 않고 Mesh는
기존 `CModel -> CMaterial`, Texture는 D3D11 SRV를 그대로 사용한다.

## 2. 본질만 남기는 고정 결정

- 현재 문서는 아직 팀 배포 정본이 아니므로 G05에서 format v3으로 한 번 정리한다.
  v2 실험 파일은 자동 호환 코드를 늘리지 않고 Effect Tool에서 다시 저장한다.
- JSON 검사는 version, 필수 field type, stable ID, Element/slot 중복, 안전한
  `Effect/...` 경로, 유한한 수치와 범위까지만 둔다.
- 미지의 enum, 절대 경로, `..`, 중복 ID를 정상값으로 바꾸는 fallback은 두지 않는다.
- G06 Preview는 저장 Document를 바꾸지 않는다. Tool session의 sample time을 입력받아
  같은 Document와 같은 시간이면 같은 한 장을 만든다.
- G06에서 Distortion 값은 저장하고 UI에 표시하지만 화면 복사본을 흔드는 실제
  distortion composite는 하지 않는다. 그 렌더 타깃 연동은 G08이다.
- G06에서 After Image 값은 저장하지만 Trail segment를 만들지 않는다. 소비는 G08이다.
- G06 Preview 대상은 Mesh, Sprite, Decal이다. Particle과 Trail은 `G08 required`를
  표시하며 가짜 quad/particle을 생성하지 않는다.
- G07 Timeline/Curve는 이번 변경에 미리 struct나 placeholder를 만들지 않는다.

## 3. 전체 파일 진행표

| G | 파일 | 변경 이유 | 연결 |
|---|---|---|---|
| G05 | `Effect_AuthoringDocument.h` | 다중 material slot과 stable render profile 계약 | JSON, Tool, Preview 공통 |
| G05 | `Effect_AuthoringDocument.cpp` 신규 | 검증·parse·serialize를 UI 파일에서 분리 | `CDataJson`, `CProjectDataRoot` |
| G05 | `Effect_Tool.h/.cpp` | slot 선택, bind/clear, profile 적용 UI | Active Document와 resource catalog |
| G05 | `Client.vcxproj/.filters` | 신규 CPP를 빌드와 Effect 필터에 등록 | Visual Studio 빌드 |
| G05 | `Invoke-ProjectAudit.ps1` | v3 material/profile/save 경계 검사 | ProjectAudit |
| G06 | `Effect_AuthoringDocument.h/.cpp` | Common/Type Detail을 v4 JSON에 저장 | Preview 입력 |
| G06 | `Effect_Preview.h/.cpp` 신규 | CModel/CMaterial과 texture quad를 off-screen 렌더 | D3D11, CModel, CShader |
| G06 | `Shader_VtxEffectMeshPreview.hlsl` 신규 | Mesh material preview | `VTXMESH` |
| G06 | `Shader_VtxEffectRectPreview.hlsl` 신규 | Sprite/Decal card preview | `VTXTEX` |
| G06 | `Effect_Tool.h/.cpp` | Detail 편집, sample slider, Preview 이미지 | `CEffectPreview` |
| G06 | `MainApp.cpp` | Effect Tool 생성 시 device/context 전달 | Tool Preview 초기화 |
| G06 | project/filter/audit | 신규 H/CPP/HLSL 등록과 경계 검사 | 빌드/Audit |

## G05 — Material Input와 stable Render Profile

### G05-1. 목표와 종료 증거

선택한 Element 하나가 Mesh source와 Base/Noise/Mask/Emissive/Dissolve texture를
서로 다른 stable slot으로 저장하고, 숫자 Pass 대신 stable render profile을 저장하면
끝이다. 이 단계에는 GPU Preview가 없다.

```text
Mesh Element에 Mesh Model + Base + Dissolve를 동시에 bind할 수 있다.
같은 slot을 다시 bind하면 새 값 하나로 교체되고 중복 slot은 남지 않는다.
Sprite/Decal/Particle/Trail에 Mesh Model을 bind할 수 없다.
Model slot에는 .wmodel, texture slot에는 .dds만 들어간다.
Render Profile은 세 stable 이름 중 하나만 저장된다.
Save 후 Load하면 resources와 renderProfile이 동일하다.
실패한 Apply/Load는 Active Document를 바꾸지 않는다.
```

### G05-2. 추가되는 항목과 한 줄 연결

- `<filesystem>`: EffectAssetId를 `Data/Effects/Authored` 경로로 계산하는 I/O 선언에
  필요하다.
- `EFFECT_RESOURCE_SLOT`: vector index 대신 model/base/noise/mask/emissive/dissolve의
  뜻을 저장한다.
- `EFFECT_RENDER_PROFILE`: JSON의 stable 이름을 G06 Preview pass와 연결한다.
- `EFFECT_MATERIAL_DESC`: 렌더 프로필을 resource 경로와 분리한다.
- `ResourceBindings`: Element 하나가 model과 여러 texture를 동시에 소유한다.
- `Effect_AuthoringDocument.cpp`: 검증·parse·serialize를 ImGui UI에서 분리한다.
- `EffectDocumentDetail` named namespace: 반복 JSON 문법만 모으고 Tool session에는
  접근하지 않는다.
- `m_eSelectedResourceSlot`: 지금 편집 중인 slot만 기억하는 Tool session 값이다.
- `Try_BindSelectedResource`: 같은 slot을 교체한 후보 Document를 검증하고 commit한다.
- `Try_ClearSelectedResource`: 선택 slot만 제거한 후보를 검증하고 commit한다.
- `Try_ApplyRenderProfile`: profile만 바꾼 후보를 검증하고 commit한다.

### G05-3. 실제 함수 흐름

```text
Bind Selected
  Render_MaterialPanel
  -> Active/Element/slot/catalog 선택 검사
  -> Active Document 전체를 StagedDocument로 복사
  -> 같은 slot 제거 후 새 binding 추가
  -> Validate_EffectDocument
  -> 성공: m_ActiveDocument 한 번 교체
  -> 실패: 기존 Document 보존

Save
  Try_SaveDocument
  -> Validate -> Serialize -> .tmp write -> atomic replace
  -> 중간 실패: 기존 파일 보존

Load
  Try_LoadDocument
  -> 파일 읽기 -> Try_ParseEffectDocumentJson(StagedDocument)
  -> 파일 이름 ID와 내부 ID 일치 검사
  -> 성공 commit / 실패 기존 상태 보존
```

### G05-4. `Client/Public/Effect_AuthoringDocument.h` 전체 코드

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

NS_BEGIN(Client)

inline constexpr uint32_t EFFECT_AUTHORING_FORMAT_VERSION = 3u;
inline constexpr uint32_t EFFECT_AUTHORING_MIN_SUPPORTED_VERSION = 3u;

enum class EFFECT_ELEMENT_KIND : uint8_t
{
	MESH,
	SPRITE,
	PARTICLE,
	DECAL,
	TRAIL,
	END
};

enum class EFFECT_RESOURCE_SLOT : uint8_t
{
	MESH_MODEL,
	BASE_TEXTURE,
	NOISE_TEXTURE,
	MASK_TEXTURE,
	EMISSIVE_TEXTURE,
	DISSOLVE_TEXTURE,
	END
};

enum class EFFECT_RESOURCE_FILE_KIND : uint8_t
{
	MODEL,
	TEXTURE,
	END
};

enum class EFFECT_RENDER_PROFILE : uint8_t
{
	OPAQUE_BACK_DEPTH_WRITE,
	ALPHA_TWO_SIDED_DEPTH_READ,
	ADDITIVE_TWO_SIDED_DEPTH_READ,
	END
};

struct EFFECT_RESOURCE_BINDING_DESC final
{
	EFFECT_RESOURCE_SLOT eSlot = EFFECT_RESOURCE_SLOT::END;
	std::string strAssetId;
};

struct EFFECT_MATERIAL_DESC final
{
	EFFECT_RENDER_PROFILE eRenderProfile =
		EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ;
};

struct EFFECT_ELEMENT_DESC final
{
	std::string strElementId;
	EFFECT_ELEMENT_KIND eKind = EFFECT_ELEMENT_KIND::END;
	std::vector<EFFECT_RESOURCE_BINDING_DESC> ResourceBindings;
	EFFECT_MATERIAL_DESC Material;
};

struct EFFECT_DOCUMENT_DESC final
{
	uint32_t iFormatVersion = EFFECT_AUTHORING_FORMAT_VERSION;
	std::string strEffectAssetId;
	std::string strDisplayName;
	std::vector<EFFECT_ELEMENT_DESC> Elements;
};

const char_t* To_EffectElementKindLabel(EFFECT_ELEMENT_KIND eKind);
const char_t* To_EffectResourceSlotLabel(EFFECT_RESOURCE_SLOT eSlot);
const char_t* To_EffectRenderProfileLabel(EFFECT_RENDER_PROFILE eProfile);
EFFECT_RESOURCE_FILE_KIND Get_EffectResourceFileKind(
	EFFECT_RESOURCE_SLOT eSlot);
bool_t Is_EffectResourceSlotAllowed(
	EFFECT_ELEMENT_KIND eKind,
	EFFECT_RESOURCE_SLOT eSlot);

EFFECT_RESOURCE_BINDING_DESC* Find_EffectResourceBinding(
	EFFECT_ELEMENT_DESC& Element,
	EFFECT_RESOURCE_SLOT eSlot);
const EFFECT_RESOURCE_BINDING_DESC* Find_EffectResourceBinding(
	const EFFECT_ELEMENT_DESC& Element,
	EFFECT_RESOURCE_SLOT eSlot);

bool_t Validate_EffectDocument(
	const EFFECT_DOCUMENT_DESC& Document,
	std::string& strOutError);
std::filesystem::path Resolve_EffectDocumentPath(
	const std::string& strEffectAssetId);
bool_t Try_ParseEffectDocumentJson(
	const std::string& strJson,
	EFFECT_DOCUMENT_DESC& OutDocument,
	std::string& strOutError);
std::string Serialize_EffectDocumentJson(
	const EFFECT_DOCUMENT_DESC& Document);

NS_END
```

### G05-5. `Client/Public/Effect_Tool.h` 전체 코드

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Effect_AuthoringDocument.h"

#include <array>
#include <optional>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CEffect_Tool final
{
private:
	struct EFFECT_RESOURCE_CATALOG_ENTRY final
	{
		std::string strAssetId;
		EFFECT_RESOURCE_FILE_KIND eFileKind =
			EFFECT_RESOURCE_FILE_KIND::END;
	};

public:
	CEffect_Tool() = default;
	~CEffect_Tool() = default;

	void Render();

private:
	void Render_NewDocumentPanel();
	void Render_ActiveDocumentPanel();
	void Render_EffectTypeSelector();
	void Render_AddElementPanel();
	void Render_ElementList();
	void Render_MaterialPanel();

	bool_t Try_CreateDocument();
	bool_t Try_AddElement();
	bool_t Try_SaveDocument();
	bool_t Try_LoadDocument();
	bool_t Refresh_ResourceCatalog();
	bool_t Try_BindSelectedResource();
	bool_t Try_ClearSelectedResource();
	bool_t Try_ApplyRenderProfile(EFFECT_RENDER_PROFILE eProfile);
	void Discard_ActiveDocument();

private:
	EFFECT_ELEMENT_KIND m_eSelectedEffectType =
		EFFECT_ELEMENT_KIND::MESH;
	EFFECT_RESOURCE_SLOT m_eSelectedResourceSlot =
		EFFECT_RESOURCE_SLOT::MESH_MODEL;
	std::array<char_t, 129> m_NewAssetId{};
	std::array<char_t, 65> m_NewDisplayName{};
	std::array<char_t, 129> m_NewElementId{};
	std::optional<EFFECT_DOCUMENT_DESC> m_ActiveDocument;
	std::string m_strDocumentStatus;
	std::string m_strElementStatus;

	std::vector<EFFECT_RESOURCE_CATALOG_ENTRY> m_ResourceCatalog;
	std::string m_strSelectedElementId;
	std::string m_strSelectedResourceAssetId;
	std::array<char_t, 129> m_ResourceFilter{};
	std::string m_strResourceStatus;
	bool_t m_bResourceCatalogRefreshAttempted = false;
};

NS_END
```
### G05-6. `Client/Private/Effect_AuthoringDocument.cpp` 전체 코드

신규 파일이다. JSON과 Document 의미만 소유하며 ImGui, device, Preview에는 접근하지
않는다.

```cpp
#include "Effect_AuthoringDocument.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <algorithm>
#include <cctype>
#include <cwctype>
#include <cmath>
#include <sstream>
#include <unordered_set>
#include <utility>

namespace Client::EffectDocumentDetail
{
	bool_t Is_StableAuthoringId(const std::string& strValue)
	{
		if (strValue.empty() || strValue.size() > 128u)
			return false;

		return std::all_of(strValue.begin(), strValue.end(),
			[](const char_t value)
			{
				return (value >= 'A' && value <= 'Z') ||
					(value >= 'a' && value <= 'z') ||
					(value >= '0' && value <= '9') ||
					value == '_' || value == '.' || value == '-';
			});
	}

	bool_t Has_VisibleCharacter(const std::string& strValue)
	{
		return std::any_of(strValue.begin(), strValue.end(),
			[](const char_t value)
			{
				return 0 == std::isspace(
					static_cast<unsigned char>(value));
			});
	}

	std::string Lower_Extension(const std::string& strAssetId)
	{
		std::string strExtension =
			std::filesystem::path(strAssetId).extension().string();
		std::transform(strExtension.begin(), strExtension.end(),
			strExtension.begin(),
			[](const char_t value)
			{
				return static_cast<char_t>(std::tolower(
					static_cast<unsigned char>(value)));
			});
		return strExtension;
	}

	bool_t Is_SafeEffectAssetId(
		const std::string& strAssetId,
		const EFFECT_RESOURCE_FILE_KIND eFileKind)
	{
		if (strAssetId.empty() || strAssetId.size() > 260u ||
			0u != strAssetId.rfind("Effect/", 0u) ||
			std::string::npos != strAssetId.find('\\') ||
			std::string::npos != strAssetId.find(':'))
			return false;

		const std::filesystem::path AssetPath(strAssetId);
		if (AssetPath.is_absolute() || AssetPath.has_root_name())
			return false;
		for (const std::filesystem::path& Part : AssetPath)
		{
			if (Part == ".." || Part == ".")
				return false;
		}

		const std::string strExtension = Lower_Extension(strAssetId);
		return (EFFECT_RESOURCE_FILE_KIND::MODEL == eFileKind &&
			strExtension == ".wmodel") ||
			(EFFECT_RESOURCE_FILE_KIND::TEXTURE == eFileKind &&
			strExtension == ".dds");
	}

	const DATA_JSON_VALUE* Find_Field(
		const DATA_JSON_VALUE& Object,
		const char_t* pName,
		const DATA_JSON_TYPE eType,
		std::string& strOutError)
	{
		const DATA_JSON_VALUE* pValue = Object.Find(pName);
		if (nullptr == pValue || pValue->Get_Type() != eType)
		{
			strOutError = std::string("Missing or invalid field: ") + pName;
			return nullptr;
		}
		return pValue;
	}

	const char_t* To_ElementToken(const EFFECT_ELEMENT_KIND eKind)
	{
		switch (eKind)
		{
		case EFFECT_ELEMENT_KIND::MESH: return "mesh";
		case EFFECT_ELEMENT_KIND::SPRITE: return "sprite";
		case EFFECT_ELEMENT_KIND::PARTICLE: return "particle";
		case EFFECT_ELEMENT_KIND::DECAL: return "decal";
		case EFFECT_ELEMENT_KIND::TRAIL: return "trail";
		case EFFECT_ELEMENT_KIND::END:
		default: return nullptr;
		}
	}

	bool_t Try_ParseElementToken(
		const std::string& strToken,
		EFFECT_ELEMENT_KIND& OutKind)
	{
		if (strToken == "mesh") OutKind = EFFECT_ELEMENT_KIND::MESH;
		else if (strToken == "sprite") OutKind = EFFECT_ELEMENT_KIND::SPRITE;
		else if (strToken == "particle") OutKind = EFFECT_ELEMENT_KIND::PARTICLE;
		else if (strToken == "decal") OutKind = EFFECT_ELEMENT_KIND::DECAL;
		else if (strToken == "trail") OutKind = EFFECT_ELEMENT_KIND::TRAIL;
		else return false;
		return true;
	}

	const char_t* To_SlotToken(const EFFECT_RESOURCE_SLOT eSlot)
	{
		switch (eSlot)
		{
		case EFFECT_RESOURCE_SLOT::MESH_MODEL: return "meshModel";
		case EFFECT_RESOURCE_SLOT::BASE_TEXTURE: return "base";
		case EFFECT_RESOURCE_SLOT::NOISE_TEXTURE: return "noise";
		case EFFECT_RESOURCE_SLOT::MASK_TEXTURE: return "mask";
		case EFFECT_RESOURCE_SLOT::EMISSIVE_TEXTURE: return "emissive";
		case EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE: return "dissolve";
		case EFFECT_RESOURCE_SLOT::END:
		default: return nullptr;
		}
	}

	bool_t Try_ParseSlotToken(
		const std::string& strToken,
		EFFECT_RESOURCE_SLOT& OutSlot)
	{
		if (strToken == "meshModel") OutSlot = EFFECT_RESOURCE_SLOT::MESH_MODEL;
		else if (strToken == "base") OutSlot = EFFECT_RESOURCE_SLOT::BASE_TEXTURE;
		else if (strToken == "noise") OutSlot = EFFECT_RESOURCE_SLOT::NOISE_TEXTURE;
		else if (strToken == "mask") OutSlot = EFFECT_RESOURCE_SLOT::MASK_TEXTURE;
		else if (strToken == "emissive") OutSlot = EFFECT_RESOURCE_SLOT::EMISSIVE_TEXTURE;
		else if (strToken == "dissolve") OutSlot = EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE;
		else return false;
		return true;
	}

	const char_t* To_ProfileToken(const EFFECT_RENDER_PROFILE eProfile)
	{
		switch (eProfile)
		{
		case EFFECT_RENDER_PROFILE::OPAQUE_BACK_DEPTH_WRITE:
			return "opaque_back_depth_write";
		case EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ:
			return "alpha_two_sided_depth_read";
		case EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ:
			return "additive_two_sided_depth_read";
		case EFFECT_RENDER_PROFILE::END:
		default:
			return nullptr;
		}
	}

	bool_t Try_ParseProfileToken(
		const std::string& strToken,
		EFFECT_RENDER_PROFILE& OutProfile)
	{
		if (strToken == "opaque_back_depth_write")
			OutProfile = EFFECT_RENDER_PROFILE::OPAQUE_BACK_DEPTH_WRITE;
		else if (strToken == "alpha_two_sided_depth_read")
			OutProfile = EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ;
		else if (strToken == "additive_two_sided_depth_read")
			OutProfile = EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ;
		else
			return false;
		return true;
	}
}

const char_t* Client::To_EffectElementKindLabel(
	const EFFECT_ELEMENT_KIND eKind)
{
	switch (eKind)
	{
	case EFFECT_ELEMENT_KIND::MESH: return "Mesh";
	case EFFECT_ELEMENT_KIND::SPRITE: return "Sprite";
	case EFFECT_ELEMENT_KIND::PARTICLE: return "Particle";
	case EFFECT_ELEMENT_KIND::DECAL: return "Decal";
	case EFFECT_ELEMENT_KIND::TRAIL: return "Trail";
	case EFFECT_ELEMENT_KIND::END:
	default: return "Invalid";
	}
}

const char_t* Client::To_EffectResourceSlotLabel(
	const EFFECT_RESOURCE_SLOT eSlot)
{
	switch (eSlot)
	{
	case EFFECT_RESOURCE_SLOT::MESH_MODEL: return "Mesh Model";
	case EFFECT_RESOURCE_SLOT::BASE_TEXTURE: return "Base";
	case EFFECT_RESOURCE_SLOT::NOISE_TEXTURE: return "Noise";
	case EFFECT_RESOURCE_SLOT::MASK_TEXTURE: return "Mask";
	case EFFECT_RESOURCE_SLOT::EMISSIVE_TEXTURE: return "Emissive";
	case EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE: return "Dissolve";
	case EFFECT_RESOURCE_SLOT::END:
	default: return "Invalid";
	}
}

const char_t* Client::To_EffectRenderProfileLabel(
	const EFFECT_RENDER_PROFILE eProfile)
{
	switch (eProfile)
	{
	case EFFECT_RENDER_PROFILE::OPAQUE_BACK_DEPTH_WRITE:
		return "Opaque / Cull Back / Depth Write";
	case EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ:
		return "Alpha / Two Sided / Depth Read";
	case EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ:
		return "Additive / Two Sided / Depth Read";
	case EFFECT_RENDER_PROFILE::END:
	default:
		return "Invalid";
	}
}

Client::EFFECT_RESOURCE_FILE_KIND Client::Get_EffectResourceFileKind(
	const EFFECT_RESOURCE_SLOT eSlot)
{
	if (EFFECT_RESOURCE_SLOT::MESH_MODEL == eSlot)
		return EFFECT_RESOURCE_FILE_KIND::MODEL;
	if (eSlot >= EFFECT_RESOURCE_SLOT::BASE_TEXTURE &&
		eSlot <= EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE)
		return EFFECT_RESOURCE_FILE_KIND::TEXTURE;
	return EFFECT_RESOURCE_FILE_KIND::END;
}

bool_t Client::Is_EffectResourceSlotAllowed(
	const EFFECT_ELEMENT_KIND eKind,
	const EFFECT_RESOURCE_SLOT eSlot)
{
	if (EFFECT_ELEMENT_KIND::END == eKind ||
		EFFECT_RESOURCE_SLOT::END == eSlot)
		return false;
	if (EFFECT_RESOURCE_SLOT::MESH_MODEL == eSlot)
		return EFFECT_ELEMENT_KIND::MESH == eKind;
	return true;
}

Client::EFFECT_RESOURCE_BINDING_DESC* Client::Find_EffectResourceBinding(
	EFFECT_ELEMENT_DESC& Element,
	const EFFECT_RESOURCE_SLOT eSlot)
{
	const auto Iterator = std::find_if(
		Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
		[eSlot](const EFFECT_RESOURCE_BINDING_DESC& Binding)
		{
			return Binding.eSlot == eSlot;
		});
	return Iterator == Element.ResourceBindings.end() ? nullptr : &*Iterator;
}

const Client::EFFECT_RESOURCE_BINDING_DESC*
Client::Find_EffectResourceBinding(
	const EFFECT_ELEMENT_DESC& Element,
	const EFFECT_RESOURCE_SLOT eSlot)
{
	const auto Iterator = std::find_if(
		Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
		[eSlot](const EFFECT_RESOURCE_BINDING_DESC& Binding)
		{
			return Binding.eSlot == eSlot;
		});
	return Iterator == Element.ResourceBindings.end() ? nullptr : &*Iterator;
}

bool_t Client::Validate_EffectDocument(
	const EFFECT_DOCUMENT_DESC& Document,
	std::string& strOutError)
{
	using namespace EffectDocumentDetail;
	if (EFFECT_AUTHORING_FORMAT_VERSION != Document.iFormatVersion)
	{
		strOutError = "Unsupported Effect document version.";
		return false;
	}
	if (!Is_StableAuthoringId(Document.strEffectAssetId))
	{
		strOutError = "Effect Asset ID is invalid.";
		return false;
	}
	if (!Has_VisibleCharacter(Document.strDisplayName) ||
		Document.strDisplayName.size() > 64u)
	{
		strOutError = "Display Name is invalid.";
		return false;
	}

	std::unordered_set<std::string> ElementIds;
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		if (!Is_StableAuthoringId(Element.strElementId) ||
			EFFECT_ELEMENT_KIND::END == Element.eKind ||
			!ElementIds.insert(Element.strElementId).second)
		{
			strOutError = "Element ID or kind is invalid or duplicated.";
			return false;
		}
		if (EFFECT_RENDER_PROFILE::END == Element.Material.eRenderProfile)
		{
			strOutError = "Render Profile is invalid.";
			return false;
		}

		std::unordered_set<uint32_t> Slots;
		for (const EFFECT_RESOURCE_BINDING_DESC& Binding :
			Element.ResourceBindings)
		{
			const uint32_t iSlot = static_cast<uint32_t>(Binding.eSlot);
			const EFFECT_RESOURCE_FILE_KIND eFileKind =
				Get_EffectResourceFileKind(Binding.eSlot);
			if (!Is_EffectResourceSlotAllowed(Element.eKind, Binding.eSlot) ||
				EFFECT_RESOURCE_FILE_KIND::END == eFileKind ||
				!Slots.insert(iSlot).second ||
				!Is_SafeEffectAssetId(Binding.strAssetId, eFileKind))
			{
				strOutError = "Effect resource slot, type, path, or duplicate is invalid.";
				return false;
			}
		}
	}

	strOutError.clear();
	return true;
}

std::filesystem::path Client::Resolve_EffectDocumentPath(
	const std::string& strEffectAssetId)
{
	if (!EffectDocumentDetail::Is_StableAuthoringId(strEffectAssetId))
		return {};
	return CProjectDataRoot::Resolve(
		std::filesystem::path(L"Effects") / L"Authored" /
		std::filesystem::path(strEffectAssetId + ".effect.json"));
}

bool_t Client::Try_ParseEffectDocumentJson(
	const std::string& strJson,
	EFFECT_DOCUMENT_DESC& OutDocument,
	std::string& strOutError)
{
	using namespace EffectDocumentDetail;
	DATA_JSON_VALUE Root;
	if (!CDataJson::Parse(strJson, Root, strOutError) || !Root.Is_Object())
	{
		if (strOutError.empty()) strOutError = "Effect document root must be an object.";
		return false;
	}

	const DATA_JSON_VALUE* pVersion =
		Find_Field(Root, "version", DATA_JSON_TYPE::NUMBER, strOutError);
	const DATA_JSON_VALUE* pAssetId =
		Find_Field(Root, "effectAssetId", DATA_JSON_TYPE::STRING, strOutError);
	const DATA_JSON_VALUE* pDisplayName =
		Find_Field(Root, "displayName", DATA_JSON_TYPE::STRING, strOutError);
	const DATA_JSON_VALUE* pElements =
		Find_Field(Root, "elements", DATA_JSON_TYPE::ARRAY, strOutError);
	if (nullptr == pVersion || nullptr == pAssetId ||
		nullptr == pDisplayName || nullptr == pElements)
		return false;

	const double fVersion = pVersion->Get_Number();
	if (!std::isfinite(fVersion) ||
		fVersion != static_cast<double>(EFFECT_AUTHORING_FORMAT_VERSION))
	{
		strOutError = "Effect document version is not supported.";
		return false;
	}

	EFFECT_DOCUMENT_DESC StagedDocument;
	StagedDocument.iFormatVersion = static_cast<uint32_t>(fVersion);
	StagedDocument.strEffectAssetId = pAssetId->Get_String();
	StagedDocument.strDisplayName = pDisplayName->Get_String();

	for (const DATA_JSON_VALUE& ElementValue : pElements->Get_Array())
	{
		if (!ElementValue.Is_Object())
		{
			strOutError = "Effect Element must be an object.";
			return false;
		}
		const DATA_JSON_VALUE* pId =
			Find_Field(ElementValue, "id", DATA_JSON_TYPE::STRING, strOutError);
		const DATA_JSON_VALUE* pKind =
			Find_Field(ElementValue, "kind", DATA_JSON_TYPE::STRING, strOutError);
		const DATA_JSON_VALUE* pResources =
			Find_Field(ElementValue, "resources", DATA_JSON_TYPE::ARRAY, strOutError);
		const DATA_JSON_VALUE* pMaterial =
			Find_Field(ElementValue, "material", DATA_JSON_TYPE::OBJECT, strOutError);
		if (nullptr == pId || nullptr == pKind ||
			nullptr == pResources || nullptr == pMaterial)
			return false;

		EFFECT_ELEMENT_DESC StagedElement;
		StagedElement.strElementId = pId->Get_String();
		if (!Try_ParseElementToken(pKind->Get_String(), StagedElement.eKind))
		{
			strOutError = "Effect Element kind is invalid.";
			return false;
		}

		for (const DATA_JSON_VALUE& ResourceValue : pResources->Get_Array())
		{
			if (!ResourceValue.Is_Object())
			{
				strOutError = "Effect resource must be an object.";
				return false;
			}
			const DATA_JSON_VALUE* pSlot =
				Find_Field(ResourceValue, "slot", DATA_JSON_TYPE::STRING, strOutError);
			const DATA_JSON_VALUE* pResourceAssetId =
				Find_Field(ResourceValue, "assetId", DATA_JSON_TYPE::STRING, strOutError);
			if (nullptr == pSlot || nullptr == pResourceAssetId)
				return false;

			EFFECT_RESOURCE_BINDING_DESC Binding;
			if (!Try_ParseSlotToken(pSlot->Get_String(), Binding.eSlot))
			{
				strOutError = "Effect resource slot is invalid.";
				return false;
			}
			Binding.strAssetId = pResourceAssetId->Get_String();
			StagedElement.ResourceBindings.push_back(std::move(Binding));
		}

		const DATA_JSON_VALUE* pProfile = Find_Field(
			*pMaterial, "renderProfile", DATA_JSON_TYPE::STRING, strOutError);
		if (nullptr == pProfile || !Try_ParseProfileToken(
			pProfile->Get_String(), StagedElement.Material.eRenderProfile))
		{
			strOutError = "Effect renderProfile is invalid.";
			return false;
		}
		StagedDocument.Elements.push_back(std::move(StagedElement));
	}

	if (!Validate_EffectDocument(StagedDocument, strOutError))
		return false;
	OutDocument = std::move(StagedDocument);
	return true;
}

std::string Client::Serialize_EffectDocumentJson(
	const EFFECT_DOCUMENT_DESC& Document)
{
	using namespace EffectDocumentDetail;
	std::ostringstream Stream;
	Stream << "{\n"
		<< "  \"version\": " << Document.iFormatVersion << ",\n"
		<< "  \"effectAssetId\": \""
		<< CDataJson::Escape(Document.strEffectAssetId) << "\",\n"
		<< "  \"displayName\": \""
		<< CDataJson::Escape(Document.strDisplayName) << "\",\n"
		<< "  \"elements\": [\n";

	for (size_t iElement = 0; iElement < Document.Elements.size(); ++iElement)
	{
		const EFFECT_ELEMENT_DESC& Element = Document.Elements[iElement];
		Stream << "    {\n"
			<< "      \"id\": \""
			<< CDataJson::Escape(Element.strElementId) << "\",\n"
			<< "      \"kind\": \"" << To_ElementToken(Element.eKind) << "\",\n"
			<< "      \"resources\": [\n";
		for (size_t iResource = 0;
			iResource < Element.ResourceBindings.size(); ++iResource)
		{
			const EFFECT_RESOURCE_BINDING_DESC& Binding =
				Element.ResourceBindings[iResource];
			Stream << "        { \"slot\": \"" << To_SlotToken(Binding.eSlot)
				<< "\", \"assetId\": \""
				<< CDataJson::Escape(Binding.strAssetId) << "\" }";
			if (iResource + 1u != Element.ResourceBindings.size()) Stream << ',';
			Stream << '\n';
		}
		Stream << "      ],\n"
			<< "      \"material\": { \"renderProfile\": \""
			<< To_ProfileToken(Element.Material.eRenderProfile) << "\" }\n"
			<< "    }";
		if (iElement + 1u != Document.Elements.size()) Stream << ',';
		Stream << '\n';
	}
	Stream << "  ]\n}\n";
	return Stream.str();
}
```

### G05-7. `Client/Private/Effect_Tool.cpp` 전체 코드

```cpp
#include "imgui.h"

#include "Effect_Tool.h"

#include "RuntimeAssetRoot.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <system_error>
#include <utility>

namespace Client::EffectToolDetail
{
	bool_t Contains_NoCase(
		const std::string& strValue,
		const std::string& strFilter)
	{
		if (strFilter.empty())
			return true;
		return std::search(strValue.begin(), strValue.end(),
			strFilter.begin(), strFilter.end(),
			[](const char_t left, const char_t right)
			{
				return std::tolower(static_cast<unsigned char>(left)) ==
					std::tolower(static_cast<unsigned char>(right));
			}) != strValue.end();
	}

	EFFECT_ELEMENT_DESC* Find_Element(
		EFFECT_DOCUMENT_DESC& Document,
		const std::string& strElementId)
	{
		const auto Iterator = std::find_if(
			Document.Elements.begin(), Document.Elements.end(),
			[&strElementId](const EFFECT_ELEMENT_DESC& Element)
			{
				return Element.strElementId == strElementId;
			});
		return Iterator == Document.Elements.end() ? nullptr : &*Iterator;
	}

	const EFFECT_ELEMENT_DESC* Find_Element(
		const EFFECT_DOCUMENT_DESC& Document,
		const std::string& strElementId)
	{
		const auto Iterator = std::find_if(
			Document.Elements.begin(), Document.Elements.end(),
			[&strElementId](const EFFECT_ELEMENT_DESC& Element)
			{
				return Element.strElementId == strElementId;
			});
		return Iterator == Document.Elements.end() ? nullptr : &*Iterator;
	}

	EFFECT_RESOURCE_FILE_KIND Classify_ResourceFile(
		const std::filesystem::path& FilePath)
	{
		std::wstring Extension = FilePath.extension().wstring();
		std::transform(Extension.begin(), Extension.end(), Extension.begin(),
			[](const wchar_t value)
			{
				return static_cast<wchar_t>(std::towlower(value));
			});
		if (Extension == L".wmodel")
			return EFFECT_RESOURCE_FILE_KIND::MODEL;
		if (Extension == L".dds")
			return EFFECT_RESOURCE_FILE_KIND::TEXTURE;
		return EFFECT_RESOURCE_FILE_KIND::END;
	}
}

void Client::CEffect_Tool::Render()
{
	ImGui::SetNextWindowSize(ImVec2(1040.f, 760.f),
		ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("LostArk Effect Tool###LostArkEffectToolG5"))
	{
		ImGui::End();
		return;
	}

	ImGui::TextWrapped(
		"G5: bind model/Base/Noise/Mask/Emissive/Dissolve inputs and "
		"save a stable Render Profile. GPU preview starts in G6.");

	if (m_ActiveDocument.has_value())
		Render_ActiveDocumentPanel();
	else
		Render_NewDocumentPanel();

	ImGui::Separator();
	Render_EffectTypeSelector();
	if (m_ActiveDocument.has_value())
	{
		Render_AddElementPanel();
		Render_ElementList();
		Render_MaterialPanel();
	}
	else
	{
		ImGui::TextDisabled(
			"Create or load a document before adding an Element.");
	}
	ImGui::End();
}

void Client::CEffect_Tool::Render_NewDocumentPanel()
{
	ImGui::SeparatorText("Data File");
	ImGui::InputText("Effect Asset ID", m_NewAssetId.data(),
		m_NewAssetId.size());
	ImGui::InputText("Display Name", m_NewDisplayName.data(),
		m_NewDisplayName.size());
	if (ImGui::Button("Create Document"))
		Try_CreateDocument();
	ImGui::SameLine();
	if (ImGui::Button("Load Document"))
		Try_LoadDocument();
	if (!m_strDocumentStatus.empty())
		ImGui::TextWrapped("%s", m_strDocumentStatus.c_str());
}

void Client::CEffect_Tool::Render_ActiveDocumentPanel()
{
	ImGui::SeparatorText("Active Document");
	const EFFECT_DOCUMENT_DESC& Document = *m_ActiveDocument;
	ImGui::Text("Format Version: %u", Document.iFormatVersion);
	ImGui::Text("Effect Asset ID: %s", Document.strEffectAssetId.c_str());
	ImGui::Text("Display Name: %s", Document.strDisplayName.c_str());
	ImGui::Text("Elements: %zu", Document.Elements.size());
	if (ImGui::Button("Save Active Document"))
		Try_SaveDocument();
	ImGui::SameLine();
	if (ImGui::Button("Discard Active Document"))
		Discard_ActiveDocument();
	if (!m_strDocumentStatus.empty())
		ImGui::TextWrapped("%s", m_strDocumentStatus.c_str());
}

void Client::CEffect_Tool::Render_EffectTypeSelector()
{
	ImGui::TextUnformatted("Next Element Type");
	for (int32_t iType = 0;
		iType < static_cast<int32_t>(EFFECT_ELEMENT_KIND::END); ++iType)
	{
		ImGui::SameLine();
		const EFFECT_ELEMENT_KIND eType =
			static_cast<EFFECT_ELEMENT_KIND>(iType);
		if (ImGui::RadioButton(To_EffectElementKindLabel(eType),
			m_eSelectedEffectType == eType))
		{
			m_eSelectedEffectType = eType;
		}
	}
}

void Client::CEffect_Tool::Render_AddElementPanel()
{
	ImGui::SeparatorText("Add Element");
	ImGui::Text("Selected Type: %s",
		To_EffectElementKindLabel(m_eSelectedEffectType));
	ImGui::InputText("Element ID", m_NewElementId.data(),
		m_NewElementId.size());
	if (ImGui::Button("Add Element"))
		Try_AddElement();
	if (!m_strElementStatus.empty())
		ImGui::TextWrapped("%s", m_strElementStatus.c_str());
}

void Client::CEffect_Tool::Render_ElementList()
{
	using EffectToolDetail::Find_Element;
	ImGui::SeparatorText("Current Elements");
	if (!m_ActiveDocument.has_value() ||
		m_ActiveDocument->Elements.empty())
	{
		ImGui::TextDisabled("No Elements have been added.");
		m_strSelectedElementId.clear();
		m_strSelectedResourceAssetId.clear();
		return;
	}

	if (!m_strSelectedElementId.empty() &&
		nullptr == Find_Element(*m_ActiveDocument, m_strSelectedElementId))
	{
		m_strSelectedElementId.clear();
		m_strSelectedResourceAssetId.clear();
	}

	constexpr ImGuiTableFlags TABLE_FLAGS =
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
		ImGuiTableFlags_SizingStretchProp;
	if (!ImGui::BeginTable("EffectElementList", 4, TABLE_FLAGS))
		return;
	ImGui::TableSetupColumn("Element ID");
	ImGui::TableSetupColumn("Kind");
	ImGui::TableSetupColumn("Bindings");
	ImGui::TableSetupColumn("Render Profile");
	ImGui::TableHeadersRow();

	for (const EFFECT_ELEMENT_DESC& Element : m_ActiveDocument->Elements)
	{
		ImGui::PushID(Element.strElementId.c_str());
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		const bool_t bSelected =
			m_strSelectedElementId == Element.strElementId;
		if (ImGui::Selectable(Element.strElementId.c_str(), bSelected,
			ImGuiSelectableFlags_SpanAllColumns))
		{
			m_strSelectedElementId = Element.strElementId;
			m_strSelectedResourceAssetId.clear();
			m_strResourceStatus.clear();
			m_eSelectedResourceSlot =
				EFFECT_ELEMENT_KIND::MESH == Element.eKind ?
				EFFECT_RESOURCE_SLOT::MESH_MODEL :
				EFFECT_RESOURCE_SLOT::BASE_TEXTURE;
		}
		ImGui::TableSetColumnIndex(1);
		ImGui::TextUnformatted(To_EffectElementKindLabel(Element.eKind));
		ImGui::TableSetColumnIndex(2);
		ImGui::Text("%zu", Element.ResourceBindings.size());
		ImGui::TableSetColumnIndex(3);
		ImGui::TextUnformatted(
			To_EffectRenderProfileLabel(Element.Material.eRenderProfile));
		ImGui::PopID();
	}
	ImGui::EndTable();
}

void Client::CEffect_Tool::Render_MaterialPanel()
{
	using EffectToolDetail::Contains_NoCase;
	using EffectToolDetail::Find_Element;
	ImGui::SeparatorText("Material Inputs and Render Profile");
	if (!m_ActiveDocument.has_value() || m_strSelectedElementId.empty())
	{
		ImGui::TextDisabled("Select an Element first.");
		return;
	}

	EFFECT_ELEMENT_DESC* pElement =
		Find_Element(*m_ActiveDocument, m_strSelectedElementId);
	if (nullptr == pElement)
	{
		m_strSelectedElementId.clear();
		m_strSelectedResourceAssetId.clear();
		ImGui::TextDisabled("Selected Element no longer exists.");
		return;
	}

	ImGui::Text("Selected Element: %s", pElement->strElementId.c_str());
	if (ImGui::BeginCombo("Render Profile",
		To_EffectRenderProfileLabel(pElement->Material.eRenderProfile)))
	{
		for (int32_t iProfile = 0;
			iProfile < static_cast<int32_t>(EFFECT_RENDER_PROFILE::END);
			++iProfile)
		{
			const EFFECT_RENDER_PROFILE eProfile =
				static_cast<EFFECT_RENDER_PROFILE>(iProfile);
			if (ImGui::Selectable(To_EffectRenderProfileLabel(eProfile),
				pElement->Material.eRenderProfile == eProfile))
			{
				Try_ApplyRenderProfile(eProfile);
			}
		}
		ImGui::EndCombo();
	}

	if (ImGui::BeginCombo("Input Slot",
		To_EffectResourceSlotLabel(m_eSelectedResourceSlot)))
	{
		for (int32_t iSlot = 0;
			iSlot < static_cast<int32_t>(EFFECT_RESOURCE_SLOT::END);
			++iSlot)
		{
			const EFFECT_RESOURCE_SLOT eSlot =
				static_cast<EFFECT_RESOURCE_SLOT>(iSlot);
			if (!Is_EffectResourceSlotAllowed(pElement->eKind, eSlot))
				continue;
			if (ImGui::Selectable(To_EffectResourceSlotLabel(eSlot),
				m_eSelectedResourceSlot == eSlot))
			{
				m_eSelectedResourceSlot = eSlot;
				m_strSelectedResourceAssetId.clear();
				m_strResourceStatus.clear();
			}
		}
		ImGui::EndCombo();
	}

	const EFFECT_RESOURCE_BINDING_DESC* pCurrentBinding =
		Find_EffectResourceBinding(*pElement, m_eSelectedResourceSlot);
	ImGui::Text("Current: %s", nullptr == pCurrentBinding ?
		"Unbound" : pCurrentBinding->strAssetId.c_str());

	if (!m_bResourceCatalogRefreshAttempted)
		Refresh_ResourceCatalog();
	if (ImGui::Button("Refresh Resources/Effect"))
		Refresh_ResourceCatalog();
	ImGui::InputText("Resource Filter", m_ResourceFilter.data(),
		m_ResourceFilter.size());

	const EFFECT_RESOURCE_FILE_KIND eExpectedFileKind =
		Get_EffectResourceFileKind(m_eSelectedResourceSlot);
	const std::string strFilter = m_ResourceFilter.data();
	if (ImGui::BeginChild("EffectResourceCatalog", ImVec2(0.f, 180.f), true))
	{
		for (const EFFECT_RESOURCE_CATALOG_ENTRY& Entry : m_ResourceCatalog)
		{
			if (Entry.eFileKind != eExpectedFileKind ||
				!Contains_NoCase(Entry.strAssetId, strFilter))
				continue;
			const bool_t bSelected =
				m_strSelectedResourceAssetId == Entry.strAssetId;
			if (ImGui::Selectable(Entry.strAssetId.c_str(), bSelected))
			{
				m_strSelectedResourceAssetId = Entry.strAssetId;
				m_strResourceStatus.clear();
			}
		}
	}
	ImGui::EndChild();

	ImGui::BeginDisabled(m_strSelectedResourceAssetId.empty());
	if (ImGui::Button("Bind Selected Resource"))
		Try_BindSelectedResource();
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(nullptr == pCurrentBinding);
	if (ImGui::Button("Clear Selected Slot"))
		Try_ClearSelectedResource();
	ImGui::EndDisabled();

	if (!m_strResourceStatus.empty())
		ImGui::TextWrapped("%s", m_strResourceStatus.c_str());
}

bool_t Client::CEffect_Tool::Try_CreateDocument()
{
	if (m_ActiveDocument.has_value())
	{
		m_strDocumentStatus =
			"Discard the active document before creating another one.";
		return false;
	}

	EFFECT_DOCUMENT_DESC StagedDocument;
	StagedDocument.strEffectAssetId = m_NewAssetId.data();
	StagedDocument.strDisplayName = m_NewDisplayName.data();
	std::string strError;
	if (!Validate_EffectDocument(StagedDocument, strError))
	{
		m_strDocumentStatus = strError;
		return false;
	}

	m_ActiveDocument = std::move(StagedDocument);
	m_NewAssetId.fill('\0');
	m_NewDisplayName.fill('\0');
	m_NewElementId.fill('\0');
	m_strSelectedElementId.clear();
	m_strSelectedResourceAssetId.clear();
	m_strElementStatus.clear();
	m_strResourceStatus.clear();
	m_strDocumentStatus =
		"Created in memory. This document has not been saved.";
	return true;
}

bool_t Client::CEffect_Tool::Try_AddElement()
{
	if (!m_ActiveDocument.has_value())
	{
		m_strElementStatus =
			"Create or load a document before adding an Element.";
		return false;
	}

	const std::string strElementId = m_NewElementId.data();
	EFFECT_DOCUMENT_DESC StagedDocument = *m_ActiveDocument;
	EFFECT_ELEMENT_DESC StagedElement;
	StagedElement.strElementId = strElementId;
	StagedElement.eKind = m_eSelectedEffectType;
	StagedDocument.Elements.push_back(std::move(StagedElement));
	std::string strError;
	if (!Validate_EffectDocument(StagedDocument, strError))
	{
		m_strElementStatus = strError;
		return false;
	}

	m_ActiveDocument = std::move(StagedDocument);
	m_NewElementId.fill('\0');
	m_strSelectedElementId = strElementId;
	m_eSelectedResourceSlot =
		EFFECT_ELEMENT_KIND::MESH == m_eSelectedEffectType ?
		EFFECT_RESOURCE_SLOT::MESH_MODEL :
		EFFECT_RESOURCE_SLOT::BASE_TEXTURE;
	m_strSelectedResourceAssetId.clear();
	m_strResourceStatus.clear();
	m_strElementStatus =
		"Added to memory. Save to persist this Element.";
	return true;
}

bool_t Client::CEffect_Tool::Try_SaveDocument()
{
	if (!m_ActiveDocument.has_value())
	{
		m_strDocumentStatus = "There is no active document to save.";
		return false;
	}

	std::string strError;
	if (!Validate_EffectDocument(*m_ActiveDocument, strError))
	{
		m_strDocumentStatus = "Save rejected: " + strError;
		return false;
	}

	const std::filesystem::path DestinationPath =
		Resolve_EffectDocumentPath(m_ActiveDocument->strEffectAssetId);
	if (DestinationPath.empty())
	{
		m_strDocumentStatus = "Save path escaped the project Data root.";
		return false;
	}
	std::error_code DirectoryError;
	std::filesystem::create_directories(
		DestinationPath.parent_path(), DirectoryError);
	if (DirectoryError)
	{
		m_strDocumentStatus = "Failed to create Data/Effects/Authored.";
		return false;
	}

	std::filesystem::path TemporaryPath = DestinationPath;
	TemporaryPath += L".tmp";
	const std::string strSerialized =
		Serialize_EffectDocumentJson(*m_ActiveDocument);
	std::ofstream Output(TemporaryPath,
		std::ios::binary | std::ios::trunc);
	if (!Output)
	{
		m_strDocumentStatus = "Failed to create temporary Effect document.";
		return false;
	}
	Output.write(strSerialized.data(),
		static_cast<std::streamsize>(strSerialized.size()));
	Output.close();
	if (!Output)
	{
		std::error_code IgnoreError;
		std::filesystem::remove(TemporaryPath, IgnoreError);
		m_strDocumentStatus =
			"Temporary write failed; previous file preserved.";
		return false;
	}

	if (!MoveFileExW(TemporaryPath.c_str(), DestinationPath.c_str(),
		MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
	{
		std::error_code IgnoreError;
		std::filesystem::remove(TemporaryPath, IgnoreError);
		m_strDocumentStatus =
			"Atomic replace failed; previous file preserved.";
		return false;
	}

	m_strDocumentStatus = "Saved " +
		m_ActiveDocument->strEffectAssetId + ".effect.json as v3.";
	return true;
}

bool_t Client::CEffect_Tool::Try_LoadDocument()
{
	if (m_ActiveDocument.has_value())
	{
		m_strDocumentStatus =
			"Discard the active document before loading another one.";
		return false;
	}

	const std::string strRequestedAssetId = m_NewAssetId.data();
	const std::filesystem::path SourcePath =
		Resolve_EffectDocumentPath(strRequestedAssetId);
	if (SourcePath.empty())
	{
		m_strDocumentStatus = "Enter a valid Effect Asset ID before loading.";
		return false;
	}
	std::ifstream Input(SourcePath, std::ios::binary);
	if (!Input)
	{
		m_strDocumentStatus =
			"Effect document does not exist or cannot be read.";
		return false;
	}
	std::ostringstream Buffer;
	Buffer << Input.rdbuf();
	if (Input.bad())
	{
		m_strDocumentStatus = "Failed while reading Effect document.";
		return false;
	}

	std::string strError;
	EFFECT_DOCUMENT_DESC StagedDocument;
	if (!Try_ParseEffectDocumentJson(
		Buffer.str(), StagedDocument, strError))
	{
		m_strDocumentStatus =
			"Load rejected; current state preserved: " + strError;
		return false;
	}
	if (StagedDocument.strEffectAssetId != strRequestedAssetId)
	{
		m_strDocumentStatus =
			"Load rejected: file name ID and document ID do not match.";
		return false;
	}

	m_ActiveDocument = std::move(StagedDocument);
	m_NewAssetId.fill('\0');
	m_NewDisplayName.fill('\0');
	m_NewElementId.fill('\0');
	m_strSelectedElementId.clear();
	m_strSelectedResourceAssetId.clear();
	m_strElementStatus.clear();
	m_strResourceStatus.clear();
	m_strDocumentStatus = "Loaded and committed the v3 Effect document.";
	return true;
}

bool_t Client::CEffect_Tool::Refresh_ResourceCatalog()
{
	using EffectToolDetail::Classify_ResourceFile;
	m_bResourceCatalogRefreshAttempted = true;
	const std::filesystem::path EffectRoot =
		CRuntimeAssetRoot::Resolve(std::filesystem::path(L"Effect"));
	std::error_code ErrorCode;
	if (EffectRoot.empty() ||
		!std::filesystem::is_directory(EffectRoot, ErrorCode) || ErrorCode)
	{
		m_strResourceStatus =
			"Catalog refresh failed; previous catalog preserved.";
		return false;
	}

	std::vector<EFFECT_RESOURCE_CATALOG_ENTRY> StagedCatalog;
	std::filesystem::recursive_directory_iterator Iterator(
		EffectRoot, std::filesystem::directory_options::none, ErrorCode);
	const std::filesystem::recursive_directory_iterator EndIterator;
	for (; !ErrorCode && Iterator != EndIterator; Iterator.increment(ErrorCode))
	{
		std::error_code FileError;
		if (!Iterator->is_regular_file(FileError))
		{
			if (FileError) ErrorCode = FileError;
			continue;
		}
		const EFFECT_RESOURCE_FILE_KIND eFileKind =
			Classify_ResourceFile(Iterator->path());
		if (EFFECT_RESOURCE_FILE_KIND::END == eFileKind)
			continue;

		const std::filesystem::path RelativePath =
			Iterator->path().lexically_relative(EffectRoot);
		if (RelativePath.empty())
		{
			ErrorCode = std::make_error_code(std::errc::invalid_argument);
			break;
		}
		EFFECT_RESOURCE_CATALOG_ENTRY Entry;
		Entry.strAssetId =
			(std::filesystem::path("Effect") / RelativePath).generic_string();
		Entry.eFileKind = eFileKind;
		StagedCatalog.push_back(std::move(Entry));
	}
	if (ErrorCode)
	{
		m_strResourceStatus =
			"Catalog refresh failed; previous catalog preserved.";
		return false;
	}

	std::sort(StagedCatalog.begin(), StagedCatalog.end(),
		[](const EFFECT_RESOURCE_CATALOG_ENTRY& Left,
			const EFFECT_RESOURCE_CATALOG_ENTRY& Right)
		{
			return Left.strAssetId < Right.strAssetId;
		});
	const auto Duplicate = std::adjacent_find(
		StagedCatalog.begin(), StagedCatalog.end(),
		[](const EFFECT_RESOURCE_CATALOG_ENTRY& Left,
			const EFFECT_RESOURCE_CATALOG_ENTRY& Right)
		{
			return Left.strAssetId == Right.strAssetId;
		});
	if (Duplicate != StagedCatalog.end())
	{
		m_strResourceStatus =
			"Catalog refresh failed: duplicate stable asset ID.";
		return false;
	}

	m_ResourceCatalog = std::move(StagedCatalog);
	if (!m_strSelectedResourceAssetId.empty())
	{
		const bool_t bStillExists = std::any_of(
			m_ResourceCatalog.begin(), m_ResourceCatalog.end(),
			[this](const EFFECT_RESOURCE_CATALOG_ENTRY& Entry)
			{
				return Entry.strAssetId == m_strSelectedResourceAssetId;
			});
		if (!bStillExists) m_strSelectedResourceAssetId.clear();
	}
	m_strResourceStatus = "Catalog refreshed: " +
		std::to_string(m_ResourceCatalog.size()) + " supported files.";
	return true;
}

bool_t Client::CEffect_Tool::Try_BindSelectedResource()
{
	using EffectToolDetail::Find_Element;
	if (!m_ActiveDocument.has_value() ||
		m_strSelectedElementId.empty() ||
		m_strSelectedResourceAssetId.empty())
	{
		m_strResourceStatus = "Select an Element and resource first.";
		return false;
	}
	const EFFECT_ELEMENT_DESC* pCurrentElement =
		Find_Element(*m_ActiveDocument, m_strSelectedElementId);
	if (nullptr == pCurrentElement ||
		!Is_EffectResourceSlotAllowed(
			pCurrentElement->eKind, m_eSelectedResourceSlot))
	{
		m_strResourceStatus = "Selected slot is not valid for this Element.";
		return false;
	}

	const EFFECT_RESOURCE_FILE_KIND eExpectedFileKind =
		Get_EffectResourceFileKind(m_eSelectedResourceSlot);
	const auto CatalogIterator = std::find_if(
		m_ResourceCatalog.begin(), m_ResourceCatalog.end(),
		[this](const EFFECT_RESOURCE_CATALOG_ENTRY& Entry)
		{
			return Entry.strAssetId == m_strSelectedResourceAssetId;
		});
	if (CatalogIterator == m_ResourceCatalog.end() ||
		CatalogIterator->eFileKind != eExpectedFileKind)
	{
		m_strResourceStatus =
			"Selected resource is stale, missing, or incompatible.";
		return false;
	}

	EFFECT_DOCUMENT_DESC StagedDocument = *m_ActiveDocument;
	EFFECT_ELEMENT_DESC* pStagedElement =
		Find_Element(StagedDocument, m_strSelectedElementId);
	if (nullptr == pStagedElement)
	{
		m_strResourceStatus = "Selected Element disappeared while staging.";
		return false;
	}
	pStagedElement->ResourceBindings.erase(
		std::remove_if(pStagedElement->ResourceBindings.begin(),
			pStagedElement->ResourceBindings.end(),
			[this](const EFFECT_RESOURCE_BINDING_DESC& Binding)
			{
				return Binding.eSlot == m_eSelectedResourceSlot;
			}),
		pStagedElement->ResourceBindings.end());
	pStagedElement->ResourceBindings.push_back(
		{ m_eSelectedResourceSlot, CatalogIterator->strAssetId });

	std::string strError;
	if (!Validate_EffectDocument(StagedDocument, strError))
	{
		m_strResourceStatus =
			"Bind rejected; current document preserved: " + strError;
		return false;
	}
	m_ActiveDocument = std::move(StagedDocument);
	m_strResourceStatus = "Resource binding committed in memory.";
	return true;
}

bool_t Client::CEffect_Tool::Try_ClearSelectedResource()
{
	using EffectToolDetail::Find_Element;
	if (!m_ActiveDocument.has_value() || m_strSelectedElementId.empty())
	{
		m_strResourceStatus = "Select an Element before clearing.";
		return false;
	}
	EFFECT_DOCUMENT_DESC StagedDocument = *m_ActiveDocument;
	EFFECT_ELEMENT_DESC* pStagedElement =
		Find_Element(StagedDocument, m_strSelectedElementId);
	if (nullptr == pStagedElement)
	{
		m_strResourceStatus = "Selected Element no longer exists.";
		return false;
	}
	pStagedElement->ResourceBindings.erase(
		std::remove_if(pStagedElement->ResourceBindings.begin(),
			pStagedElement->ResourceBindings.end(),
			[this](const EFFECT_RESOURCE_BINDING_DESC& Binding)
			{
				return Binding.eSlot == m_eSelectedResourceSlot;
			}),
		pStagedElement->ResourceBindings.end());

	std::string strError;
	if (!Validate_EffectDocument(StagedDocument, strError))
	{
		m_strResourceStatus =
			"Clear rejected; current document preserved: " + strError;
		return false;
	}
	m_ActiveDocument = std::move(StagedDocument);
	m_strSelectedResourceAssetId.clear();
	m_strResourceStatus = "Selected slot cleared in memory.";
	return true;
}

bool_t Client::CEffect_Tool::Try_ApplyRenderProfile(
	const EFFECT_RENDER_PROFILE eProfile)
{
	using EffectToolDetail::Find_Element;
	if (!m_ActiveDocument.has_value() ||
		m_strSelectedElementId.empty() ||
		EFFECT_RENDER_PROFILE::END == eProfile)
	{
		m_strResourceStatus = "Select an Element and valid profile first.";
		return false;
	}
	EFFECT_DOCUMENT_DESC StagedDocument = *m_ActiveDocument;
	EFFECT_ELEMENT_DESC* pStagedElement =
		Find_Element(StagedDocument, m_strSelectedElementId);
	if (nullptr == pStagedElement)
	{
		m_strResourceStatus = "Selected Element no longer exists.";
		return false;
	}
	pStagedElement->Material.eRenderProfile = eProfile;

	std::string strError;
	if (!Validate_EffectDocument(StagedDocument, strError))
	{
		m_strResourceStatus =
			"Profile rejected; current document preserved: " + strError;
		return false;
	}
	m_ActiveDocument = std::move(StagedDocument);
	m_strResourceStatus = "Render Profile committed in memory.";
	return true;
}

void Client::CEffect_Tool::Discard_ActiveDocument()
{
	m_ActiveDocument.reset();
	m_NewElementId.fill('\0');
	m_strSelectedElementId.clear();
	m_strSelectedResourceAssetId.clear();
	m_strElementStatus.clear();
	m_strResourceStatus.clear();
	m_strDocumentStatus =
		"Discarded the in-memory document. No file was changed.";
}
```

### G05-8. project/filter/Audit 반영 코드

`Client/Default/Client.vcxproj`의 기존 Effect 항목 옆에 아래 한 줄을 추가한다.

```xml
<ClCompile Include="..\Private\Effect_AuthoringDocument.cpp" />
```

`Client/Default/Client.vcxproj.filters`의 `03. Tools\02. Effect` 항목에 아래 블록을
추가한다.

```xml
<ClCompile Include="..\Private\Effect_AuthoringDocument.cpp">
  <Filter>03. Tools\02. Effect</Filter>
</ClCompile>
```

`Tools/ProjectAudit/Invoke-ProjectAudit.ps1`의 기존 Effect check에서 새 CPP를 읽고 G05
shape를 검사한다.

```powershell
	$effectDocumentSource = Get-Content -LiteralPath 'Client\Private\Effect_AuthoringDocument.cpp' -Raw
	$effectG5MaterialShape =
		$effectDocumentHeader -match 'EFFECT_AUTHORING_FORMAT_VERSION\s*=\s*3u' -and
		$effectDocumentHeader -match 'enum class EFFECT_RESOURCE_SLOT[\s\S]*MESH_MODEL[\s\S]*BASE_TEXTURE[\s\S]*NOISE_TEXTURE[\s\S]*MASK_TEXTURE[\s\S]*EMISSIVE_TEXTURE[\s\S]*DISSOLVE_TEXTURE[\s\S]*END' -and
		$effectDocumentHeader -match 'enum class EFFECT_RENDER_PROFILE[\s\S]*OPAQUE_BACK_DEPTH_WRITE[\s\S]*ALPHA_TWO_SIDED_DEPTH_READ[\s\S]*ADDITIVE_TWO_SIDED_DEPTH_READ[\s\S]*END' -and
		$effectDocumentHeader -match 'vector<EFFECT_RESOURCE_BINDING_DESC>\s+ResourceBindings' -and
		$effectDocumentSource -match 'Validate_EffectDocument' -and
		$effectDocumentSource -match 'Try_ParseEffectDocumentJson' -and
		$effectDocumentSource -match 'Serialize_EffectDocumentJson' -and
		$effectToolSource -match 'Try_ApplyRenderProfile' -and
		$effectToolSource -match 'Bind Selected Resource' -and
		$clientProjectSource -match 'Effect_AuthoringDocument\.cpp'
	Add-Check 'effect.g5-material-profile-boundary' $effectG5MaterialShape "material=$effectG5MaterialShape"
```

### G05-9. 사용자가 직접 작성할 순서

1. `Effect_AuthoringDocument.h`에서 v3 enum과 `ResourceBindings`, `Material`을 먼저 쓴다.
2. 새 `Effect_AuthoringDocument.cpp`에 label/token, validation, parse, serialize를 쓴다.
3. `.vcxproj`와 `.filters`에 새 CPP를 등록한다.
4. `Effect_Tool.h`에서 선택 slot과 Material UI 함수 선언을 쓴다.
5. `Effect_Tool.cpp`에서 G04 JSON helper를 제거하고 새 Document 함수를 호출한다.
6. Element list의 단일 `ResourceBinding` 출력을 binding 개수로 바꾼다.
7. Material panel, bind/clear/profile stage-validate-commit을 쓴다.
8. 저장한 v3 JSON을 다시 Load해 값이 같은지 확인한다.

### G05-10. 검증

```powershell
$project = [xml](Get-Content -LiteralPath 'Client/Default/Client.vcxproj' -Raw)
$filters = [xml](Get-Content -LiteralPath 'Client/Default/Client.vcxproj.filters' -Raw)
& MSBuild.exe 'Client/Default/Client.vcxproj' /t:Build /p:Configuration=Debug /p:Platform=x64
& 'Tools/ProjectAudit/Invoke-ProjectAudit.ps1'
git diff --check
```

수동 smoke는 다른 세션의 시작 abort가 해결된 뒤에만 한다.

```text
F1 -> Effect -> Mesh Element 생성
Mesh Model, Base, Dissolve 각각 bind
Alpha / Two Sided / Depth Read 선택
Save -> Discard -> 같은 ID Load
세 binding과 profile이 동일한지 확인
Particle 선택 시 Mesh Model slot이 목록에 나타나지 않는지 확인
```

## G06 — Common/Type Detail과 Stateless Preview

### G06-1. 목표와 종료 증거

선택 Element에 공통 Transform, Color, UV, Timing과 최소 type detail을 저장하고,
Mesh/Sprite/Decal을 Tool 내부 render target에 한 장으로 표시하면 끝이다. Preview 시간은
Tool의 slider가 소유하며 Preview 객체 자체는 시간을 증가시키지 않는다.

```text
Position/Rotation/Revolution/Scale 변경이 Preview 한 장에 반영된다.
ColorOffset/ColorMultiply/ColorClip/Emissive/Dissolve가 반영된다.
UV Start/Speed/Wave/Sequence/Loop/Tile 값이 저장된다.
Start Delay/Lifetime/After Image/Dissolve Start 값이 저장된다.
Mesh는 기존 CModel -> CMaterial로 그려진다.
Sprite와 Decal은 같은 rect geometry를 쓰되 type detail이 다르다.
Particle/Trail 선택 시 G08 required가 표시되고 가짜 Preview는 없다.
Preview resource load 실패 시 이전에 성공한 Preview와 Active Document를 보존한다.
```

### G06-2. 추가되는 파일·헤더·변수·함수와 연결

#### `Effect_AuthoringDocument.h`

- `EFFECT_TRANSFORM_DESC`: 위치·자전·공전·크기를 한 공통 계약으로 묶는다.
- `EFFECT_COLOR_DESC`: 색 가감/곱, clip, emissive, distortion 값을 묶는다.
- `EFFECT_UV_DESC`: 시작 offset, 속도, wave, sequence, loop, tile을 묶는다.
- `EFFECT_TIMING_DESC`: delay, lifetime, after-image, dissolve 시작점을 묶는다.
- `EFFECT_MESH_DETAIL_DESC`: Mesh만 쓰는 model material 선택이다.
- `EFFECT_SPRITE_DETAIL_DESC`: Sprite의 billboard 선택이다.
- `EFFECT_DECAL_DETAIL_DESC`: Decal surface card의 크기와 깊이다.
- `EFFECT_DETAIL_DESC`: 공통 값과 type payload를 Element 하나에 연결한다.

Document 헤더는 enum/struct 데이터만 소유한다. 새 `Effect_AuthoringDocument.cpp`와
Document public helper 함수는 만들지 않는다. Detail 검증·JSON 읽기·쓰기는 G04부터
존재하는 `Effect_Tool.cpp`의 기존 함수에 이어 붙인다.

#### `Effect_Preview.h/.cpp`

- `ID3D11Device/Context`: Preview 전용 render target, shader, texture를 만든다.
- `CModel`: Mesh `.wmodel`을 기존 통합 model/material 경로로 읽고 그린다.
- `CVIBuffer_Rect`: Sprite/Decal이 공유하는 사각형 geometry다.
- `CShader`: stable render profile을 실제 HLSL pass로 시작한다.
- `Ensure_Resources`: 선택 Element의 asset ID가 바뀔 때만 후보 GPU resource를 만들고
  전부 성공한 뒤 교체한다.
- `Render`: 현재 render target/viewport를 보존하고 Preview target에 한 장을 그린 뒤
  원래 상태를 복구한다.
- `Select_Pass`: profile enum을 Mesh/Rect shader의 동일한 0,1,2 pass로 번역한다.
- `Get_TextureView`: ImGui `Image`가 off-screen 결과를 읽는 입구다.

#### `Effect_Tool.h/.cpp`

- 생성자의 device/context: `CEffectPreview`가 D3D resource를 만들 수 있게 MainApp이
  이미 소유한 장치를 전달한다.
- `m_pPreview`: Document가 아닌 Tool session GPU cache다.
- `m_fPreviewTimeSeconds`: slider가 소유하는 명시적인 sample time이다.
- `Render_DetailPanel`: 선택 Element의 후보 detail을 편집하고 검증 후 commit한다.
- `Render_PreviewPanel`: sample time을 넘겨 렌더하고 SRV를 ImGui에 표시한다.

### G06-3. 실제 함수 흐름

```text
Detail 편집
  Render_DetailPanel
  -> 선택 Element를 StagedElement로 복사
  -> ImGui가 StagedElement만 편집
  -> 변화가 있으면 Active Document 전체를 StagedDocument로 복사
  -> 해당 Element의 Detail만 교체
  -> Effect_Tool.cpp의 기존 Validate_EffectDocument
  -> 성공 commit / 실패 기존 Document 보존

Preview
  Render_PreviewPanel
  -> 선택 Element + explicit sample time 전달
  -> CEffectPreview::Render
  -> Ensure_Resources가 asset ID signature 비교
  -> 달라졌으면 shader/model/texture/rect 후보 생성
  -> 후보 전부 성공 시 GPU cache 교체
  -> render target/viewport 저장
  -> Transform/Color/UV/Timing 상수 bind
  -> Render Profile을 pass 0/1/2로 mapping
  -> CModel::Render 또는 CVIBuffer_Rect::Render
  -> 원래 target/viewport 복구
  -> ImGui::Image(Get_TextureView)
```

### G06-4. `Client/Public/Effect_AuthoringDocument.h` 전체 코드

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <string>
#include <vector>

NS_BEGIN(Client)

inline constexpr uint32_t EFFECT_AUTHORING_FORMAT_VERSION = 4u;
inline constexpr uint32_t EFFECT_AUTHORING_MIN_SUPPORTED_VERSION = 3u;

enum class EFFECT_ELEMENT_KIND : uint8_t
{
	MESH,
	SPRITE,
	PARTICLE,
	DECAL,
	TRAIL,
	END
};

enum class EFFECT_RESOURCE_SLOT : uint8_t
{
	MESH_MODEL,
	BASE_TEXTURE,
	NOISE_TEXTURE,
	MASK_TEXTURE,
	EMISSIVE_TEXTURE,
	DISSOLVE_TEXTURE,
	END
};

enum class EFFECT_RESOURCE_FILE_KIND : uint8_t
{
	MODEL,
	TEXTURE,
	END
};

enum class EFFECT_RENDER_PROFILE : uint8_t
{
	OPAQUE_BACK_DEPTH_WRITE,
	ALPHA_TWO_SIDED_DEPTH_READ,
	ADDITIVE_TWO_SIDED_DEPTH_READ,
	END
};

struct EFFECT_RESOURCE_BINDING_DESC final
{
	EFFECT_RESOURCE_SLOT eSlot = EFFECT_RESOURCE_SLOT::END;
	std::string strAssetId;
};

struct EFFECT_MATERIAL_DESC final
{
	EFFECT_RENDER_PROFILE eRenderProfile =
		EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ;
};

struct EFFECT_TRANSFORM_DESC final
{
	float3_t vPosition = { 0.f, 0.f, 0.f };
	float3_t vRotationDegrees = { 0.f, 0.f, 0.f };
	float3_t vRevolutionDegreesPerSecond = { 0.f, 0.f, 0.f };
	float3_t vScale = { 1.f, 1.f, 1.f };
};

struct EFFECT_COLOR_DESC final
{
	float4_t vColorOffset = { 0.f, 0.f, 0.f, 0.f };
	float4_t vColorMultiply = { 1.f, 1.f, 1.f, 1.f };
	f32_t fColorClip = 0.f;
	f32_t fEmissiveIntensity = 1.f;
	f32_t fDistortionIntensity = 0.f;
	bool_t bDistortionOnBaseMaterial = false;
	f32_t fRadialTime = 0.f;
	f32_t fRadialIntensity = 0.f;
};

struct EFFECT_UV_DESC final
{
	float2_t vStart = { 0.f, 0.f };
	float2_t vSpeed = { 0.f, 0.f };
	bool_t bWave = false;
	float2_t vWaveAmplitude = { 0.f, 0.f };
	f32_t fWaveFrequency = 1.f;
	bool_t bSequence = false;
	bool_t bLoop = false;
	f32_t fSequenceTerm = 0.1f;
	int32_t iTileColumns = 1;
	int32_t iTileRows = 1;
	int32_t iTileIndex = 0;
};

struct EFFECT_TIMING_DESC final
{
	f32_t fStartDelaySeconds = 0.f;
	f32_t fLifeTimeSeconds = 1.f;
	f32_t fAfterImageSeconds = 0.f;
	f32_t fDissolveStartNormalized = 1.f;
};

struct EFFECT_MESH_DETAIL_DESC final
{
	bool_t bUseModelMaterial = true;
};

struct EFFECT_SPRITE_DETAIL_DESC final
{
	bool_t bBillboard = true;
};

struct EFFECT_DECAL_DETAIL_DESC final
{
	float2_t vSize = { 1.f, 1.f };
	f32_t fDepth = 0.25f;
};

struct EFFECT_DETAIL_DESC final
{
	EFFECT_TRANSFORM_DESC Transform;
	EFFECT_COLOR_DESC Color;
	EFFECT_UV_DESC UV;
	EFFECT_TIMING_DESC Timing;
	EFFECT_MESH_DETAIL_DESC Mesh;
	EFFECT_SPRITE_DETAIL_DESC Sprite;
	EFFECT_DECAL_DETAIL_DESC Decal;
};

struct EFFECT_ELEMENT_DESC final
{
	std::string strElementId;
	EFFECT_ELEMENT_KIND eKind = EFFECT_ELEMENT_KIND::END;
	std::vector<EFFECT_RESOURCE_BINDING_DESC> ResourceBindings;
	EFFECT_MATERIAL_DESC Material;
	EFFECT_DETAIL_DESC Detail;
};

struct EFFECT_DOCUMENT_DESC final
{
	uint32_t iFormatVersion = EFFECT_AUTHORING_FORMAT_VERSION;
	std::string strEffectAssetId;
	std::string strDisplayName;
	std::vector<EFFECT_ELEMENT_DESC> Elements;
};

NS_END
```




### G06-5. 기존 `Client/Private/Effect_Tool.cpp`의 Document 처리 확장

새 `Effect_AuthoringDocument.cpp`를 만들지 않는다. G04/G05부터 `Effect_Tool.cpp`에
있던 `Validate_EffectDocument`, `Parse_EffectDocumentJson`,
`Serialize_EffectDocumentJson`의 책임만 Detail까지 넓힌다. 전체 반영 코드는
G06-11의 `Effect_Tool.cpp`에 합쳐 둔다.

정확한 수정 위치와 흐름은 다음과 같다.

```text
Validate_EffectDocument
  기존 Element resource/material 검사 직후
  -> Transform/Color/UV/Timing/Type Detail 범위 검사

Parse_EffectDocumentJson
  resource/material을 StagedElement에 채운 직후
  -> detail이 있으면 StagedElement.Detail에 읽기
  -> v3에 detail이 없으면 struct 기본값 유지
  -> 모든 Element가 성공한 뒤 OutDocument 교체

Serialize_EffectDocumentJson
  resources/material 출력 직후
  -> Element.Detail을 detail object로 출력
```

추가 public 함수는 만들지 않는다. 숫자 배열 읽기·쓰기처럼 반복되는 작은 처리는
같은 `Effect_Tool.cpp`의 `EffectDocumentDetail` 내부 helper로만 두며, Document
헤더나 다른 파일의 계약으로 공개하지 않는다.









### G06-6. `Client/Public/Effect_Preview.h` 전체 코드

신규 파일이다.

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Effect_AuthoringDocument.h"

#include <array>
#include <string>

NS_BEGIN(Engine)
class CModel;
class CShader;
class CVIBuffer_Rect;
NS_END

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

	ID3D11ShaderResourceView* Get_TextureView() const
	{
		return m_pColorSRV.Get();
	}
	const std::string& Get_Status() const { return m_strStatus; }

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
	std::string m_strStatus = "Select a Mesh, Sprite, or Decal Element.";
};

NS_END
```

### G06-7. `Client/Private/Effect_Preview.cpp` 전체 코드

```cpp
#include "Effect_Preview.h"

#include "BinaryAsset/ModelAssetData.h"
#include "DirectXTK/DDSTextureLoader.h"
#include "Model.h"
#include "RuntimeAssetRoot.h"
#include "Shader.h"
#include "VIBuffer_Rect.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <sstream>
#include <utility>

namespace Client::EffectPreviewDetail
{
	constexpr uint32_t MIN_TARGET_SIZE = 256u;
	constexpr uint32_t MAX_TARGET_SIZE = 1024u;
	constexpr uint32_t TARGET_GRANULARITY = 64u;

	uint32_t Quantize_TargetSize(uint32_t iValue)
	{
		iValue = (std::max)(MIN_TARGET_SIZE,
			(std::min)(MAX_TARGET_SIZE, iValue));
		iValue = (iValue + TARGET_GRANULARITY - 1u) /
			TARGET_GRANULARITY * TARGET_GRANULARITY;
		return (std::min)(MAX_TARGET_SIZE, iValue);
	}

	size_t Texture_Index(const EFFECT_RESOURCE_SLOT eSlot)
	{
		return static_cast<size_t>(eSlot) -
			static_cast<size_t>(EFFECT_RESOURCE_SLOT::BASE_TEXTURE);
	}

	ComPtr<ID3D11ShaderResourceView> Find_Texture(
		const std::array<ComPtr<ID3D11ShaderResourceView>, 5>& Textures,
		const EFFECT_RESOURCE_SLOT eSlot)
	{
		return Textures[Texture_Index(eSlot)];
	}

	const EFFECT_RESOURCE_BINDING_DESC* Find_Binding(
		const EFFECT_ELEMENT_DESC& Element,
		const EFFECT_RESOURCE_SLOT eSlot)
	{
		const auto Iterator = std::find_if(
			Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
			[eSlot](const EFFECT_RESOURCE_BINDING_DESC& Binding)
			{
				return Binding.eSlot == eSlot;
			});
		return Iterator == Element.ResourceBindings.end() ?
			nullptr : &*Iterator;
	}
}

Client::CEffectPreview::CEffectPreview(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice(std::move(pDevice)),
	  m_pContext(std::move(pContext))
{
}

HRESULT Client::CEffectPreview::Initialize()
{
	if (nullptr == m_pDevice || nullptr == m_pContext)
		return E_INVALIDARG;

	unique_ptr<CShader> MeshShader = CShader::Create(
		m_pDevice,
		m_pContext,
		TEXT("../Bin/ShaderFiles/Shader_VtxEffectMeshPreview.hlsl"),
		VTXMESH::Elements,
		VTXMESH::iNumElements);
	unique_ptr<CShader> RectShader = CShader::Create(
		m_pDevice,
		m_pContext,
		TEXT("../Bin/ShaderFiles/Shader_VtxEffectRectPreview.hlsl"),
		VTXTEX::Elements,
		VTXTEX::iNumElements);
	unique_ptr<CVIBuffer_Rect> Rect =
		CVIBuffer_Rect::Create(m_pDevice, m_pContext);
	if (nullptr == MeshShader || nullptr == RectShader || nullptr == Rect ||
		FAILED(Create_FallbackTextures()))
	{
		m_strStatus = "Effect Preview core resource creation failed.";
		return E_FAIL;
	}

	m_pMeshShader = std::move(MeshShader);
	m_pRectShader = std::move(RectShader);
	m_pRect = std::move(Rect);
	m_strStatus = "Select a Mesh, Sprite, or Decal Element.";
	return S_OK;
}

void Client::CEffectPreview::Clear()
{
	m_pModel.reset();
	for (ComPtr<ID3D11ShaderResourceView>& Texture : m_InputTextures)
		Texture.Reset();
	m_strResourceSignature.clear();
	m_strStatus = "Select a Mesh, Sprite, or Decal Element.";
}

HRESULT Client::CEffectPreview::Create_FallbackTextures()
{
	const auto CreateSolid = [this](
		const uint32_t iRGBA,
		ComPtr<ID3D11ShaderResourceView>& OutSRV) -> HRESULT
	{
		D3D11_TEXTURE2D_DESC Desc{};
		Desc.Width = 1u;
		Desc.Height = 1u;
		Desc.MipLevels = 1u;
		Desc.ArraySize = 1u;
		Desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		Desc.SampleDesc.Count = 1u;
		Desc.Usage = D3D11_USAGE_IMMUTABLE;
		Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		D3D11_SUBRESOURCE_DATA Data{};
		Data.pSysMem = &iRGBA;
		Data.SysMemPitch = sizeof(iRGBA);
		ComPtr<ID3D11Texture2D> Texture;
		if (FAILED(m_pDevice->CreateTexture2D(&Desc, &Data, &Texture)))
			return E_FAIL;
		return m_pDevice->CreateShaderResourceView(
			Texture.Get(), nullptr, &OutSRV);
	};

	return FAILED(CreateSolid(0xffffffffu, m_pWhiteTexture)) ||
		FAILED(CreateSolid(0xff000000u, m_pBlackTexture)) ? E_FAIL : S_OK;
}

HRESULT Client::CEffectPreview::Ensure_RenderTarget(
	uint32_t iWidth,
	uint32_t iHeight)
{
	using EffectPreviewDetail::Quantize_TargetSize;
	iWidth = Quantize_TargetSize(iWidth);
	iHeight = Quantize_TargetSize(iHeight);
	if (iWidth == m_iTargetWidth && iHeight == m_iTargetHeight &&
		nullptr != m_pColorSRV && nullptr != m_pDepthDSV)
		return S_OK;

	D3D11_TEXTURE2D_DESC ColorDesc{};
	ColorDesc.Width = iWidth;
	ColorDesc.Height = iHeight;
	ColorDesc.MipLevels = 1u;
	ColorDesc.ArraySize = 1u;
	ColorDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	ColorDesc.SampleDesc.Count = 1u;
	ColorDesc.Usage = D3D11_USAGE_DEFAULT;
	ColorDesc.BindFlags =
		D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	ComPtr<ID3D11Texture2D> ColorTexture;
	ComPtr<ID3D11RenderTargetView> ColorRTV;
	ComPtr<ID3D11ShaderResourceView> ColorSRV;
	if (FAILED(m_pDevice->CreateTexture2D(
			&ColorDesc, nullptr, &ColorTexture)) ||
		FAILED(m_pDevice->CreateRenderTargetView(
			ColorTexture.Get(), nullptr, &ColorRTV)) ||
		FAILED(m_pDevice->CreateShaderResourceView(
			ColorTexture.Get(), nullptr, &ColorSRV)))
		return E_FAIL;

	D3D11_TEXTURE2D_DESC DepthDesc{};
	DepthDesc.Width = iWidth;
	DepthDesc.Height = iHeight;
	DepthDesc.MipLevels = 1u;
	DepthDesc.ArraySize = 1u;
	DepthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DepthDesc.SampleDesc.Count = 1u;
	DepthDesc.Usage = D3D11_USAGE_DEFAULT;
	DepthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	ComPtr<ID3D11Texture2D> DepthTexture;
	ComPtr<ID3D11DepthStencilView> DepthDSV;
	if (FAILED(m_pDevice->CreateTexture2D(
			&DepthDesc, nullptr, &DepthTexture)) ||
		FAILED(m_pDevice->CreateDepthStencilView(
			DepthTexture.Get(), nullptr, &DepthDSV)))
		return E_FAIL;

	m_pColorTexture = std::move(ColorTexture);
	m_pColorRTV = std::move(ColorRTV);
	m_pColorSRV = std::move(ColorSRV);
	m_pDepthTexture = std::move(DepthTexture);
	m_pDepthDSV = std::move(DepthDSV);
	m_iTargetWidth = iWidth;
	m_iTargetHeight = iHeight;
	return S_OK;
}

std::string Client::CEffectPreview::Build_ResourceSignature(
	const EFFECT_ELEMENT_DESC& Element) const
{
	std::ostringstream Stream;
	Stream << static_cast<uint32_t>(Element.eKind) << '|';
	for (const EFFECT_RESOURCE_BINDING_DESC& Binding :
		Element.ResourceBindings)
	{
		Stream << static_cast<uint32_t>(Binding.eSlot)
			<< '=' << Binding.strAssetId << '|';
	}
	return Stream.str();
}

HRESULT Client::CEffectPreview::Load_Texture(
	const std::string& strAssetId,
	ComPtr<ID3D11ShaderResourceView>& OutSRV) const
{
	const std::filesystem::path Path =
		CRuntimeAssetRoot::Resolve(std::filesystem::path(strAssetId));
	if (Path.empty() || !std::filesystem::is_regular_file(Path))
		return E_FAIL;
	return DirectX::CreateDDSTextureFromFile(
		m_pDevice.Get(), Path.c_str(), nullptr, &OutSRV);
}

HRESULT Client::CEffectPreview::Ensure_Resources(
	const EFFECT_ELEMENT_DESC& Element)
{
	const std::string strSignature = Build_ResourceSignature(Element);
	if (strSignature == m_strResourceSignature)
		return S_OK;
	if (EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind ||
		EFFECT_ELEMENT_KIND::TRAIL == Element.eKind)
	{
		m_strStatus = "Particle/Trail Preview requires G08 modules.";
		return S_FALSE;
	}

	shared_ptr<CModel> StagedModel;
	if (EFFECT_ELEMENT_KIND::MESH == Element.eKind)
	{
		const EFFECT_RESOURCE_BINDING_DESC* pModelBinding =
			EffectPreviewDetail::Find_Binding(
				Element, EFFECT_RESOURCE_SLOT::MESH_MODEL);
		if (nullptr == pModelBinding)
		{
			m_strStatus = "Mesh Preview requires a Mesh Model binding.";
			return E_FAIL;
		}
		const std::filesystem::path ModelPath = CRuntimeAssetRoot::Resolve(
			std::filesystem::path(pModelBinding->strAssetId));
		if (ModelPath.empty())
			return E_FAIL;
		unique_ptr<CModel> Model = CModel::Create(
			m_pDevice, m_pContext, MODEL::NONANIM,
			ModelPath.string().c_str(), XMMatrixIdentity());
		if (nullptr == Model || !Model->Has_LocalBounds())
		{
			m_strStatus = "Mesh CModel load or bounds validation failed.";
			return E_FAIL;
		}
		StagedModel = std::move(Model);
	}

	std::array<ComPtr<ID3D11ShaderResourceView>, 5> StagedTextures;
	for (const EFFECT_RESOURCE_BINDING_DESC& Binding :
		Element.ResourceBindings)
	{
		if (Binding.eSlot < EFFECT_RESOURCE_SLOT::BASE_TEXTURE ||
			Binding.eSlot > EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE)
			continue;
		ComPtr<ID3D11ShaderResourceView> Texture;
		if (FAILED(Load_Texture(Binding.strAssetId, Texture)))
		{
			m_strStatus = "Texture load failed; previous Preview preserved: " +
				Binding.strAssetId;
			return E_FAIL;
		}
		StagedTextures[
			EffectPreviewDetail::Texture_Index(Binding.eSlot)] =
			std::move(Texture);
	}
	if ((EFFECT_ELEMENT_KIND::SPRITE == Element.eKind ||
		EFFECT_ELEMENT_KIND::DECAL == Element.eKind) &&
		nullptr == EffectPreviewDetail::Find_Texture(
			StagedTextures, EFFECT_RESOURCE_SLOT::BASE_TEXTURE))
	{
		m_strStatus = "Sprite/Decal Preview requires a Base texture.";
		return E_FAIL;
	}

	m_pModel = std::move(StagedModel);
	m_InputTextures = std::move(StagedTextures);
	m_strResourceSignature = strSignature;
	m_strStatus = "Preview resources committed.";
	return S_OK;
}

uint32_t Client::CEffectPreview::Select_Pass(
	const EFFECT_RENDER_PROFILE eProfile) const
{
	switch (eProfile)
	{
	case EFFECT_RENDER_PROFILE::OPAQUE_BACK_DEPTH_WRITE: return 0u;
	case EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ: return 1u;
	case EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ: return 2u;
	case EFFECT_RENDER_PROFILE::END:
	default: return UINT32_MAX;
	}
}

HRESULT Client::CEffectPreview::Bind_CommonShaderValues(
	const shared_ptr<CShader>& pShader,
	const EFFECT_ELEMENT_DESC& Element,
	const f32_t fSampleTimeSeconds,
	const float4x4_t& World,
	const float4x4_t& View,
	const float4x4_t& Projection)
{
	if (nullptr == pShader)
		return E_INVALIDARG;
	const EFFECT_DETAIL_DESC& Detail = Element.Detail;
	const f32_t fLocalTime = (std::max)(
		0.f, fSampleTimeSeconds - Detail.Timing.fStartDelaySeconds);
	const f32_t fNormalizedTime = (std::min)(
		1.f, fLocalTime / Detail.Timing.fLifeTimeSeconds);
	float2_t UVOffset(
		Detail.UV.vStart.x + Detail.UV.vSpeed.x * fLocalTime,
		Detail.UV.vStart.y + Detail.UV.vSpeed.y * fLocalTime);
	if (Detail.UV.bWave)
	{
		const f32_t fWave = std::sin(
			XM_2PI * Detail.UV.fWaveFrequency * fLocalTime);
		UVOffset.x += Detail.UV.vWaveAmplitude.x * fWave;
		UVOffset.y += Detail.UV.vWaveAmplitude.y * fWave;
	}
	int32_t iTileIndex = Detail.UV.iTileIndex;
	if (Detail.UV.bSequence)
	{
		const int32_t iTileCount =
			Detail.UV.iTileColumns * Detail.UV.iTileRows;
		const int32_t iFrame =
			static_cast<int32_t>(fLocalTime / Detail.UV.fSequenceTerm);
		iTileIndex = Detail.UV.bLoop ?
			iFrame % iTileCount : (std::min)(iFrame, iTileCount - 1);
	}
	const float2_t UVScale(
		1.f / static_cast<f32_t>(Detail.UV.iTileColumns),
		1.f / static_cast<f32_t>(Detail.UV.iTileRows));
	UVOffset.x += static_cast<f32_t>(
		iTileIndex % Detail.UV.iTileColumns) * UVScale.x;
	UVOffset.y += static_cast<f32_t>(
		iTileIndex / Detail.UV.iTileColumns) * UVScale.y;
	const f32_t fDissolve = Detail.Timing.fDissolveStartNormalized >= 1.f ?
		0.f : (std::max)(0.f,
			(fNormalizedTime - Detail.Timing.fDissolveStartNormalized) /
			(1.f - Detail.Timing.fDissolveStartNormalized));
	const uint32_t iHasNoise = nullptr != EffectPreviewDetail::Find_Texture(
		m_InputTextures, EFFECT_RESOURCE_SLOT::NOISE_TEXTURE) ? 1u : 0u;
	const uint32_t iHasMask = nullptr != EffectPreviewDetail::Find_Texture(
		m_InputTextures, EFFECT_RESOURCE_SLOT::MASK_TEXTURE) ? 1u : 0u;
	const uint32_t iHasEmissive = nullptr != EffectPreviewDetail::Find_Texture(
		m_InputTextures, EFFECT_RESOURCE_SLOT::EMISSIVE_TEXTURE) ? 1u : 0u;
	const uint32_t iHasDissolve = nullptr != EffectPreviewDetail::Find_Texture(
		m_InputTextures, EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE) ? 1u : 0u;

	return FAILED(pShader->Bind_Matrix("g_WorldMatrix", &World)) ||
		FAILED(pShader->Bind_Matrix("g_ViewMatrix", &View)) ||
		FAILED(pShader->Bind_Matrix("g_ProjMatrix", &Projection)) ||
		FAILED(pShader->Bind_RawValue("g_UVScale", &UVScale, sizeof(UVScale))) ||
		FAILED(pShader->Bind_RawValue("g_UVOffset", &UVOffset, sizeof(UVOffset))) ||
		FAILED(pShader->Bind_RawValue("g_ColorOffset",
			&Detail.Color.vColorOffset, sizeof(Detail.Color.vColorOffset))) ||
		FAILED(pShader->Bind_RawValue("g_ColorMultiply",
			&Detail.Color.vColorMultiply, sizeof(Detail.Color.vColorMultiply))) ||
		FAILED(pShader->Bind_RawValue("g_ColorClip",
			&Detail.Color.fColorClip, sizeof(Detail.Color.fColorClip))) ||
		FAILED(pShader->Bind_RawValue("g_EmissiveIntensity",
			&Detail.Color.fEmissiveIntensity,
			sizeof(Detail.Color.fEmissiveIntensity))) ||
		FAILED(pShader->Bind_RawValue("g_DissolveAmount",
			&fDissolve, sizeof(fDissolve))) ||
		FAILED(pShader->Bind_RawValue("g_HasNoise", &iHasNoise, sizeof(iHasNoise))) ||
		FAILED(pShader->Bind_RawValue("g_HasMask", &iHasMask, sizeof(iHasMask))) ||
		FAILED(pShader->Bind_RawValue("g_HasEmissive", &iHasEmissive,
			sizeof(iHasEmissive))) ||
		FAILED(pShader->Bind_RawValue("g_HasDissolve", &iHasDissolve,
			sizeof(iHasDissolve))) ||
		FAILED(pShader->Bind_Texture("g_NoiseTexture", iHasNoise ?
			EffectPreviewDetail::Find_Texture(m_InputTextures,
				EFFECT_RESOURCE_SLOT::NOISE_TEXTURE) : m_pBlackTexture)) ||
		FAILED(pShader->Bind_Texture("g_MaskTexture", iHasMask ?
			EffectPreviewDetail::Find_Texture(m_InputTextures,
				EFFECT_RESOURCE_SLOT::MASK_TEXTURE) : m_pWhiteTexture)) ||
		FAILED(pShader->Bind_Texture("g_EmissiveTexture", iHasEmissive ?
			EffectPreviewDetail::Find_Texture(m_InputTextures,
				EFFECT_RESOURCE_SLOT::EMISSIVE_TEXTURE) : m_pBlackTexture)) ||
		FAILED(pShader->Bind_Texture("g_DissolveTexture", iHasDissolve ?
			EffectPreviewDetail::Find_Texture(m_InputTextures,
				EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE) : m_pBlackTexture)) ?
		E_FAIL : S_OK;
}

HRESULT Client::CEffectPreview::Render_Mesh(
	const EFFECT_ELEMENT_DESC& Element,
	const f32_t fSampleTimeSeconds)
{
	if (nullptr == m_pModel || nullptr == m_pMeshShader)
		return E_FAIL;
	const float3_t Min = m_pModel->Get_LocalBoundsMin();
	const float3_t Max = m_pModel->Get_LocalBoundsMax();
	const float3_t Center(
		(Min.x + Max.x) * 0.5f,
		(Min.y + Max.y) * 0.5f,
		(Min.z + Max.z) * 0.5f);
	const float3_t Half(
		(Max.x - Min.x) * 0.5f,
		(Max.y - Min.y) * 0.5f,
		(Max.z - Min.z) * 0.5f);
	const f32_t fRadius = (std::max)(0.1f, std::sqrt(
		Half.x * Half.x + Half.y * Half.y + Half.z * Half.z));
	const EFFECT_TRANSFORM_DESC& Transform = Element.Detail.Transform;
	const vector_t Scale = XMLoadFloat3(&Transform.vScale);
	const vector_t Rotation = XMVectorSet(
		XMConvertToRadians(Transform.vRotationDegrees.x +
			Transform.vRevolutionDegreesPerSecond.x * fSampleTimeSeconds),
		XMConvertToRadians(Transform.vRotationDegrees.y +
			Transform.vRevolutionDegreesPerSecond.y * fSampleTimeSeconds),
		XMConvertToRadians(Transform.vRotationDegrees.z +
			Transform.vRevolutionDegreesPerSecond.z * fSampleTimeSeconds), 0.f);
	const matrix_t WorldMatrix =
		XMMatrixTranslation(-Center.x, -Center.y, -Center.z) *
		XMMatrixScalingFromVector(Scale) *
		XMMatrixRotationRollPitchYawFromVector(Rotation) *
		XMMatrixTranslationFromVector(XMLoadFloat3(&Transform.vPosition));
	const matrix_t ViewMatrix = XMMatrixLookAtLH(
		XMVectorSet(0.f, 0.f, -fRadius * 3.f, 1.f),
		XMVectorZero(), XMVectorSet(0.f, 1.f, 0.f, 0.f));
	const matrix_t ProjectionMatrix = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(45.f),
		static_cast<f32_t>(m_iTargetWidth) /
			static_cast<f32_t>(m_iTargetHeight),
		0.01f, fRadius * 10.f);
	float4x4_t World{}, View{}, Projection{};
	XMStoreFloat4x4(&World, WorldMatrix);
	XMStoreFloat4x4(&View, ViewMatrix);
	XMStoreFloat4x4(&Projection, ProjectionMatrix);
	if (FAILED(Bind_CommonShaderValues(
		m_pMeshShader, Element, fSampleTimeSeconds,
		World, View, Projection)))
		return E_FAIL;

	const ComPtr<ID3D11ShaderResourceView> BaseOverride =
		Element.Detail.Mesh.bUseModelMaterial ? nullptr :
		EffectPreviewDetail::Find_Texture(
			m_InputTextures, EFFECT_RESOURCE_SLOT::BASE_TEXTURE);
	const uint32_t iUseBaseOverride = nullptr != BaseOverride ? 1u : 0u;
	if (FAILED(m_pMeshShader->Bind_RawValue(
		"g_UseBaseOverride", &iUseBaseOverride, sizeof(iUseBaseOverride))))
		return E_FAIL;
	for (uint32_t iMesh = 0; iMesh < m_pModel->Get_NumMeshes(); ++iMesh)
	{
		if ((0u != iUseBaseOverride && FAILED(m_pMeshShader->Bind_Texture(
				"g_BaseTexture", BaseOverride))) ||
			(0u == iUseBaseOverride && FAILED(m_pModel->Bind_Material(
				m_pMeshShader, "g_BaseTexture", iMesh,
				aiTextureType_DIFFUSE))) ||
			FAILED(m_pMeshShader->Begin(
				Select_Pass(Element.Material.eRenderProfile))) ||
			FAILED(m_pModel->Render(iMesh)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT Client::CEffectPreview::Render_Rect(
	const EFFECT_ELEMENT_DESC& Element,
	const f32_t fSampleTimeSeconds)
{
	if (nullptr == m_pRect || nullptr == m_pRectShader)
		return E_FAIL;
	const EFFECT_TRANSFORM_DESC& Transform = Element.Detail.Transform;
	float3_t Scale = Transform.vScale;
	if (EFFECT_ELEMENT_KIND::DECAL == Element.eKind)
	{
		Scale.x *= Element.Detail.Decal.vSize.x;
		Scale.y *= Element.Detail.Decal.vSize.y;
	}
	const matrix_t WorldMatrix =
		XMMatrixScaling(Scale.x, Scale.y, Scale.z) *
		XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(Transform.vRotationDegrees.x +
				Transform.vRevolutionDegreesPerSecond.x * fSampleTimeSeconds),
			XMConvertToRadians(Transform.vRotationDegrees.y +
				Transform.vRevolutionDegreesPerSecond.y * fSampleTimeSeconds),
			XMConvertToRadians(Transform.vRotationDegrees.z +
				Transform.vRevolutionDegreesPerSecond.z * fSampleTimeSeconds)) *
		XMMatrixTranslation(
			Transform.vPosition.x,
			Transform.vPosition.y,
			Transform.vPosition.z);
	const matrix_t ViewMatrix = XMMatrixIdentity();
	const matrix_t ProjectionMatrix = XMMatrixOrthographicLH(2.f, 2.f, 0.f, 1.f);
	float4x4_t World{}, View{}, Projection{};
	XMStoreFloat4x4(&World, WorldMatrix);
	XMStoreFloat4x4(&View, ViewMatrix);
	XMStoreFloat4x4(&Projection, ProjectionMatrix);
	const ComPtr<ID3D11ShaderResourceView> Base =
		EffectPreviewDetail::Find_Texture(
			m_InputTextures, EFFECT_RESOURCE_SLOT::BASE_TEXTURE);
	return FAILED(Bind_CommonShaderValues(
			m_pRectShader, Element, fSampleTimeSeconds,
			World, View, Projection)) ||
		FAILED(m_pRectShader->Bind_Texture("g_BaseTexture", Base)) ||
		FAILED(m_pRectShader->Begin(
			Select_Pass(Element.Material.eRenderProfile))) ||
		FAILED(m_pRect->Render()) ? E_FAIL : S_OK;
}

HRESULT Client::CEffectPreview::Render(
	const EFFECT_ELEMENT_DESC& Element,
	const f32_t fSampleTimeSeconds,
	const uint32_t iWidth,
	const uint32_t iHeight)
{
	const HRESULT ResourceResult = Ensure_Resources(Element);
	if (S_FALSE == ResourceResult)
		return S_FALSE;
	if (FAILED(ResourceResult) ||
		FAILED(Ensure_RenderTarget(iWidth, iHeight)))
		return E_FAIL;

	ComPtr<ID3D11RenderTargetView> PreviousRTV;
	ComPtr<ID3D11DepthStencilView> PreviousDSV;
	m_pContext->OMGetRenderTargets(1u, &PreviousRTV, &PreviousDSV);
	std::array<D3D11_VIEWPORT,
		D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE> PreviousViewports{};
	uint32_t iPreviousViewportCount =
		static_cast<uint32_t>(PreviousViewports.size());
	m_pContext->RSGetViewports(
		&iPreviousViewportCount, PreviousViewports.data());
	ID3D11ShaderResourceView* pNullSRV = nullptr;
	m_pContext->PSSetShaderResources(0u, 1u, &pNullSRV);
	ID3D11RenderTargetView* pPreviewRTV = m_pColorRTV.Get();
	m_pContext->OMSetRenderTargets(1u, &pPreviewRTV, m_pDepthDSV.Get());
	const float ClearColor[4] = { 0.025f, 0.035f, 0.045f, 0.f };
	m_pContext->ClearRenderTargetView(m_pColorRTV.Get(), ClearColor);
	m_pContext->ClearDepthStencilView(
		m_pDepthDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0u);
	D3D11_VIEWPORT Viewport{};
	Viewport.Width = static_cast<f32_t>(m_iTargetWidth);
	Viewport.Height = static_cast<f32_t>(m_iTargetHeight);
	Viewport.MaxDepth = 1.f;
	m_pContext->RSSetViewports(1u, &Viewport);

	HRESULT RenderResult = S_OK;
	if (fSampleTimeSeconds >= Element.Detail.Timing.fStartDelaySeconds)
	{
		RenderResult = EFFECT_ELEMENT_KIND::MESH == Element.eKind ?
			Render_Mesh(Element, fSampleTimeSeconds) :
			Render_Rect(Element, fSampleTimeSeconds);
	}

	ID3D11RenderTargetView* pRestoredRTV = PreviousRTV.Get();
	m_pContext->OMSetRenderTargets(
		1u, &pRestoredRTV, PreviousDSV.Get());
	if (0u != iPreviousViewportCount)
		m_pContext->RSSetViewports(
			iPreviousViewportCount, PreviousViewports.data());
	if (FAILED(RenderResult))
	{
		m_strStatus = "Effect Preview rendering failed.";
		return E_FAIL;
	}
	m_strStatus = "Stateless Preview ready.";
	return S_OK;
}
```

### G06-8. `Client/Bin/ShaderFiles/Shader_VtxEffectMeshPreview.hlsl` 전체 코드

신규 파일이다. 세 Pass의 순서는 `Select_Pass()`와 고정 계약이다.

```hlsl
#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix;
float4x4 g_ViewMatrix;
float4x4 g_ProjMatrix;

Texture2D g_BaseTexture;
Texture2D g_NoiseTexture;
Texture2D g_MaskTexture;
Texture2D g_EmissiveTexture;
Texture2D g_DissolveTexture;

float2 g_UVScale = float2(1.f, 1.f);
float2 g_UVOffset = float2(0.f, 0.f);
float4 g_ColorOffset = float4(0.f, 0.f, 0.f, 0.f);
float4 g_ColorMultiply = float4(1.f, 1.f, 1.f, 1.f);
float g_ColorClip = 0.f;
float g_EmissiveIntensity = 1.f;
float g_DissolveAmount = 0.f;
uint g_UseBaseOverride = 0;
uint g_HasNoise = 0;
uint g_HasMask = 0;
uint g_HasEmissive = 0;
uint g_HasDissolve = 0;

struct VS_IN
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float2 uv : TEXCOORD0;
};

struct VS_OUT
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

VS_OUT VS_MAIN(VS_IN input)
{
    VS_OUT output;
    output.position = mul(float4(input.position, 1.f),
        mul(mul(g_WorldMatrix, g_ViewMatrix), g_ProjMatrix));
    output.normal = normalize(mul(
        float4(input.normal, 0.f), g_WorldMatrix).xyz);
    output.uv = input.uv * g_UVScale + g_UVOffset;
    return output;
}

float4 Shade_Effect(float2 uv, float3 normal)
{
    float4 color = g_BaseTexture.Sample(LinearSampler, uv);
    const float noise = 0 != g_HasNoise ?
        g_NoiseTexture.Sample(LinearSampler, uv).r : 0.f;
    const float mask = 0 != g_HasMask ?
        g_MaskTexture.Sample(LinearSampler, uv).r : 1.f;
    if (0 != g_HasDissolve)
    {
        const float dissolve =
            g_DissolveTexture.Sample(LinearSampler, uv).r + noise * 0.1f;
        clip(dissolve - g_DissolveAmount);
    }
    color = color * g_ColorMultiply + g_ColorOffset;
    color.a *= mask;
    clip(color.a - g_ColorClip);
    const float light = 0.35f +
        saturate(dot(normalize(normal), normalize(float3(0.4f, 0.8f, -0.3f)))) * 0.65f;
    color.rgb *= light;
    if (0 != g_HasEmissive)
        color.rgb += g_EmissiveTexture.Sample(
            LinearSampler, uv).rgb * g_EmissiveIntensity;
    return saturate(color);
}

float4 PS_MAIN(VS_OUT input) : SV_TARGET0
{
    return Shade_Effect(input.uv, input.normal);
}

technique11 DefaultTechnique
{
    pass OpaqueBackDepthWrite
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
    pass AlphaTwoSidedDepthRead
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
    pass AdditiveTwoSidedDepthRead
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
}
```

### G06-9. `Client/Bin/ShaderFiles/Shader_VtxEffectRectPreview.hlsl` 전체 코드

신규 파일이다.

```hlsl
#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix;
float4x4 g_ViewMatrix;
float4x4 g_ProjMatrix;

Texture2D g_BaseTexture;
Texture2D g_NoiseTexture;
Texture2D g_MaskTexture;
Texture2D g_EmissiveTexture;
Texture2D g_DissolveTexture;

float2 g_UVScale = float2(1.f, 1.f);
float2 g_UVOffset = float2(0.f, 0.f);
float4 g_ColorOffset = float4(0.f, 0.f, 0.f, 0.f);
float4 g_ColorMultiply = float4(1.f, 1.f, 1.f, 1.f);
float g_ColorClip = 0.f;
float g_EmissiveIntensity = 1.f;
float g_DissolveAmount = 0.f;
uint g_HasNoise = 0;
uint g_HasMask = 0;
uint g_HasEmissive = 0;
uint g_HasDissolve = 0;

struct VS_IN
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
};

struct VS_OUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

VS_OUT VS_MAIN(VS_IN input)
{
    VS_OUT output;
    output.position = mul(float4(input.position, 1.f),
        mul(mul(g_WorldMatrix, g_ViewMatrix), g_ProjMatrix));
    output.uv = input.uv * g_UVScale + g_UVOffset;
    return output;
}

float4 PS_MAIN(VS_OUT input) : SV_TARGET0
{
    float4 color = g_BaseTexture.Sample(LinearSampler, input.uv);
    const float noise = 0 != g_HasNoise ?
        g_NoiseTexture.Sample(LinearSampler, input.uv).r : 0.f;
    const float mask = 0 != g_HasMask ?
        g_MaskTexture.Sample(LinearSampler, input.uv).r : 1.f;
    if (0 != g_HasDissolve)
    {
        const float dissolve =
            g_DissolveTexture.Sample(LinearSampler, input.uv).r + noise * 0.1f;
        clip(dissolve - g_DissolveAmount);
    }
    color = color * g_ColorMultiply + g_ColorOffset;
    color.a *= mask;
    clip(color.a - g_ColorClip);
    if (0 != g_HasEmissive)
        color.rgb += g_EmissiveTexture.Sample(
           LinearSampler, input.uv).rgb * g_EmissiveIntensity;
    return saturate(color);
}

technique11 DefaultTechnique
{
    pass OpaqueBackDepthWrite
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
    pass AlphaTwoSidedDepthRead
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
    pass AdditiveTwoSidedDepthRead
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
}
```




### G06-10. `Client/Public/Effect_Tool.h` 전체 코드

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Effect_AuthoringDocument.h"

#include <array>
#include <memory>
#include <optional>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CEffectPreview;

class CEffect_Tool final
{
private:
	struct EFFECT_RESOURCE_CATALOG_ENTRY final
	{
		std::string strAssetId;
		EFFECT_RESOURCE_FILE_KIND eFileKind =
			EFFECT_RESOURCE_FILE_KIND::END;
	};

public:
	CEffect_Tool(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	~CEffect_Tool();

	void Render();

private:
	void Render_NewDocumentPanel();
	void Render_ActiveDocumentPanel();
	void Render_EffectTypeSelector();
	void Render_AddElementPanel();
	void Render_ElementList();
	void Render_MaterialPanel();
	void Render_DetailPanel();
	void Render_PreviewPanel();

	bool_t Try_CreateDocument();
	bool_t Try_AddElement();
	bool_t Try_SaveDocument();
	bool_t Try_LoadDocument();
	bool_t Refresh_ResourceCatalog();
	bool_t Try_BindSelectedResource();
	bool_t Try_ClearSelectedResource();
	bool_t Try_ApplyRenderProfile(EFFECT_RENDER_PROFILE eProfile);
	bool_t Try_ApplyDetail(const EFFECT_DETAIL_DESC& Detail);
	void Discard_ActiveDocument();

private:
	EFFECT_ELEMENT_KIND m_eSelectedEffectType =
		EFFECT_ELEMENT_KIND::MESH;
	EFFECT_RESOURCE_SLOT m_eSelectedResourceSlot =
		EFFECT_RESOURCE_SLOT::MESH_MODEL;
	std::array<char_t, 129> m_NewAssetId{};
	std::array<char_t, 65> m_NewDisplayName{};
	std::array<char_t, 129> m_NewElementId{};
	std::optional<EFFECT_DOCUMENT_DESC> m_ActiveDocument;
	std::string m_strDocumentStatus;
	std::string m_strElementStatus;

	std::vector<EFFECT_RESOURCE_CATALOG_ENTRY> m_ResourceCatalog;
	std::string m_strSelectedElementId;
	std::string m_strSelectedResourceAssetId;
	std::array<char_t, 129> m_ResourceFilter{};
	std::string m_strResourceStatus;
	bool_t m_bResourceCatalogRefreshAttempted = false;

	unique_ptr<CEffectPreview> m_pPreview;
	f32_t m_fPreviewTimeSeconds = 0.f;
	std::string m_strDetailStatus;
};

NS_END
```

### G06-11. `Client/Private/Effect_Tool.cpp` 전체 코드

G05의 Document 생성·저장·로드·catalog·bind 함수는 의미가 바뀌지 않는다. 아래 전체
코드는 그 함수도 생략하지 않고 포함한다.

```cpp
#include "imgui.h"

#include "Effect_Tool.h"

#include "DataJson.h"
#include "Effect_Preview.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cwctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <system_error>
#include <unordered_set>
#include <utility>

// Document 데이터의 검증과 JSON 입출력은 Tool 구현 안에서만 사용한다.
namespace Client
{
	const char_t* To_EffectElementKindLabel(EFFECT_ELEMENT_KIND eKind);
	const char_t* To_EffectResourceSlotLabel(EFFECT_RESOURCE_SLOT eSlot);
	const char_t* To_EffectRenderProfileLabel(EFFECT_RENDER_PROFILE eProfile);
	EFFECT_RESOURCE_FILE_KIND Get_EffectResourceFileKind(
		EFFECT_RESOURCE_SLOT eSlot);
	bool_t Is_EffectResourceSlotAllowed(
		EFFECT_ELEMENT_KIND eKind,
		EFFECT_RESOURCE_SLOT eSlot);
	EFFECT_RESOURCE_BINDING_DESC* Find_EffectResourceBinding(
		EFFECT_ELEMENT_DESC& Element,
		EFFECT_RESOURCE_SLOT eSlot);
	const EFFECT_RESOURCE_BINDING_DESC* Find_EffectResourceBinding(
		const EFFECT_ELEMENT_DESC& Element,
		EFFECT_RESOURCE_SLOT eSlot);
	bool_t Validate_EffectDocument(
		const EFFECT_DOCUMENT_DESC& Document,
		std::string& strOutError);
	std::filesystem::path Resolve_EffectDocumentPath(
		const std::string& strEffectAssetId);
	bool_t Try_ParseEffectDocumentJson(
		const std::string& strJson,
		EFFECT_DOCUMENT_DESC& OutDocument,
		std::string& strOutError);
	std::string Serialize_EffectDocumentJson(
		const EFFECT_DOCUMENT_DESC& Document);
}

namespace Client::EffectDocumentDetail
{
	bool_t Is_StableAuthoringId(const std::string& strValue)
	{
		if (strValue.empty() || strValue.size() > 128u)
			return false;
		return std::all_of(strValue.begin(), strValue.end(),
			[](const char_t value)
			{
				return (value >= 'A' && value <= 'Z') ||
					(value >= 'a' && value <= 'z') ||
					(value >= '0' && value <= '9') ||
					value == '_' || value == '.' || value == '-';
			});
	}

	bool_t Has_VisibleCharacter(const std::string& strValue)
	{
		return std::any_of(strValue.begin(), strValue.end(),
			[](const char_t value)
			{
				return 0 == std::isspace(
					static_cast<unsigned char>(value));
			});
	}

	std::string Lower_Extension(const std::string& strAssetId)
	{
		std::string strExtension =
			std::filesystem::path(strAssetId).extension().string();
		std::transform(strExtension.begin(), strExtension.end(),
			strExtension.begin(),
			[](const char_t value)
			{
				return static_cast<char_t>(std::tolower(
					static_cast<unsigned char>(value)));
			});
		return strExtension;
	}

	bool_t Is_SafeEffectAssetId(
		const std::string& strAssetId,
		const EFFECT_RESOURCE_FILE_KIND eFileKind)
	{
		if (strAssetId.empty() || strAssetId.size() > 260u ||
			0u != strAssetId.rfind("Effect/", 0u) ||
			std::string::npos != strAssetId.find('\\') ||
			std::string::npos != strAssetId.find(':'))
			return false;
		const std::filesystem::path AssetPath(strAssetId);
		if (AssetPath.is_absolute() || AssetPath.has_root_name())
			return false;
		for (const std::filesystem::path& Part : AssetPath)
		{
			if (Part == ".." || Part == ".")
				return false;
		}
		const std::string strExtension = Lower_Extension(strAssetId);
		return (EFFECT_RESOURCE_FILE_KIND::MODEL == eFileKind &&
			strExtension == ".wmodel") ||
			(EFFECT_RESOURCE_FILE_KIND::TEXTURE == eFileKind &&
			strExtension == ".dds");
	}

	const DATA_JSON_VALUE* Find_Field(
		const DATA_JSON_VALUE& Object,
		const char_t* pName,
		const DATA_JSON_TYPE eType,
		std::string& strOutError)
	{
		const DATA_JSON_VALUE* pValue = Object.Find(pName);
		if (nullptr == pValue || pValue->Get_Type() != eType)
		{
			strOutError = std::string("Missing or invalid field: ") + pName;
			return nullptr;
		}
		return pValue;
	}

	bool_t Try_ReadFloat(
		const DATA_JSON_VALUE& Object,
		const char_t* pName,
		f32_t& OutValue,
		std::string& strOutError)
	{
		const DATA_JSON_VALUE* pValue =
			Find_Field(Object, pName, DATA_JSON_TYPE::NUMBER, strOutError);
		if (nullptr == pValue || !std::isfinite(pValue->Get_Number()))
			return false;
		OutValue = static_cast<f32_t>(pValue->Get_Number());
		return std::isfinite(OutValue);
	}

	bool_t Try_ReadInt(
		const DATA_JSON_VALUE& Object,
		const char_t* pName,
		int32_t& OutValue,
		std::string& strOutError)
	{
		f32_t fValue = 0.f;
		if (!Try_ReadFloat(Object, pName, fValue, strOutError) ||
			std::floor(fValue) != fValue)
			return false;
		OutValue = static_cast<int32_t>(fValue);
		return true;
	}

	bool_t Try_ReadBool(
		const DATA_JSON_VALUE& Object,
		const char_t* pName,
		bool_t& OutValue,
		std::string& strOutError)
	{
		const DATA_JSON_VALUE* pValue =
			Find_Field(Object, pName, DATA_JSON_TYPE::BOOLEAN, strOutError);
		if (nullptr == pValue)
			return false;
		OutValue = pValue->Get_Boolean();
		return true;
	}

	bool_t Try_ReadFloatArray(
		const DATA_JSON_VALUE& Object,
		const char_t* pName,
		f32_t* pOutValues,
		const size_t iCount,
		std::string& strOutError)
	{
		const DATA_JSON_VALUE* pValue =
			Find_Field(Object, pName, DATA_JSON_TYPE::ARRAY, strOutError);
		if (nullptr == pValue || pValue->Get_Array().size() != iCount)
			return false;
		for (size_t i = 0; i < iCount; ++i)
		{
			const DATA_JSON_VALUE& Item = pValue->Get_Array()[i];
			if (!Item.Is_Number() || !std::isfinite(Item.Get_Number()))
				return false;
			pOutValues[i] = static_cast<f32_t>(Item.Get_Number());
			if (!std::isfinite(pOutValues[i]))
				return false;
		}
		return true;
	}

	bool_t Is_Finite(const float2_t& Value)
	{
		return std::isfinite(Value.x) && std::isfinite(Value.y);
	}

	bool_t Is_Finite(const float3_t& Value)
	{
		return std::isfinite(Value.x) && std::isfinite(Value.y) &&
			std::isfinite(Value.z);
	}

	bool_t Is_Finite(const float4_t& Value)
	{
		return std::isfinite(Value.x) && std::isfinite(Value.y) &&
			std::isfinite(Value.z) && std::isfinite(Value.w);
	}

	const char_t* To_ElementToken(const EFFECT_ELEMENT_KIND eKind)
	{
		switch (eKind)
		{
		case EFFECT_ELEMENT_KIND::MESH: return "mesh";
		case EFFECT_ELEMENT_KIND::SPRITE: return "sprite";
		case EFFECT_ELEMENT_KIND::PARTICLE: return "particle";
		case EFFECT_ELEMENT_KIND::DECAL: return "decal";
		case EFFECT_ELEMENT_KIND::TRAIL: return "trail";
		case EFFECT_ELEMENT_KIND::END:
		default: return nullptr;
		}
	}

	bool_t Try_ParseElementToken(
		const std::string& strToken,
		EFFECT_ELEMENT_KIND& OutKind)
	{
		if (strToken == "mesh") OutKind = EFFECT_ELEMENT_KIND::MESH;
		else if (strToken == "sprite") OutKind = EFFECT_ELEMENT_KIND::SPRITE;
		else if (strToken == "particle") OutKind = EFFECT_ELEMENT_KIND::PARTICLE;
		else if (strToken == "decal") OutKind = EFFECT_ELEMENT_KIND::DECAL;
		else if (strToken == "trail") OutKind = EFFECT_ELEMENT_KIND::TRAIL;
		else return false;
		return true;
	}

	const char_t* To_SlotToken(const EFFECT_RESOURCE_SLOT eSlot)
	{
		switch (eSlot)
		{
		case EFFECT_RESOURCE_SLOT::MESH_MODEL: return "meshModel";
		case EFFECT_RESOURCE_SLOT::BASE_TEXTURE: return "base";
		case EFFECT_RESOURCE_SLOT::NOISE_TEXTURE: return "noise";
		case EFFECT_RESOURCE_SLOT::MASK_TEXTURE: return "mask";
		case EFFECT_RESOURCE_SLOT::EMISSIVE_TEXTURE: return "emissive";
		case EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE: return "dissolve";
		case EFFECT_RESOURCE_SLOT::END:
		default: return nullptr;
		}
	}

	bool_t Try_ParseSlotToken(
		const std::string& strToken,
		EFFECT_RESOURCE_SLOT& OutSlot)
	{
		if (strToken == "meshModel") OutSlot = EFFECT_RESOURCE_SLOT::MESH_MODEL;
		else if (strToken == "base") OutSlot = EFFECT_RESOURCE_SLOT::BASE_TEXTURE;
		else if (strToken == "noise") OutSlot = EFFECT_RESOURCE_SLOT::NOISE_TEXTURE;
		else if (strToken == "mask") OutSlot = EFFECT_RESOURCE_SLOT::MASK_TEXTURE;
		else if (strToken == "emissive") OutSlot = EFFECT_RESOURCE_SLOT::EMISSIVE_TEXTURE;
		else if (strToken == "dissolve") OutSlot = EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE;
		else return false;
		return true;
	}

	const char_t* To_ProfileToken(const EFFECT_RENDER_PROFILE eProfile)
	{
		switch (eProfile)
		{
		case EFFECT_RENDER_PROFILE::OPAQUE_BACK_DEPTH_WRITE:
			return "opaque_back_depth_write";
		case EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ:
			return "alpha_two_sided_depth_read";
		case EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ:
			return "additive_two_sided_depth_read";
		case EFFECT_RENDER_PROFILE::END:
		default: return nullptr;
		}
	}

	bool_t Try_ParseProfileToken(
		const std::string& strToken,
		EFFECT_RENDER_PROFILE& OutProfile)
	{
		if (strToken == "opaque_back_depth_write")
			OutProfile = EFFECT_RENDER_PROFILE::OPAQUE_BACK_DEPTH_WRITE;
		else if (strToken == "alpha_two_sided_depth_read")
			OutProfile = EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ;
		else if (strToken == "additive_two_sided_depth_read")
			OutProfile = EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ;
		else return false;
		return true;
	}

	bool_t Try_ReadDetail(
		const DATA_JSON_VALUE& Value,
		EFFECT_DETAIL_DESC& OutDetail,
		std::string& strOutError)
	{
		const DATA_JSON_VALUE* pTransform =
			Find_Field(Value, "transform", DATA_JSON_TYPE::OBJECT, strOutError);
		const DATA_JSON_VALUE* pColor =
			Find_Field(Value, "color", DATA_JSON_TYPE::OBJECT, strOutError);
		const DATA_JSON_VALUE* pUV =
			Find_Field(Value, "uv", DATA_JSON_TYPE::OBJECT, strOutError);
		const DATA_JSON_VALUE* pTiming =
			Find_Field(Value, "timing", DATA_JSON_TYPE::OBJECT, strOutError);
		const DATA_JSON_VALUE* pMesh =
			Find_Field(Value, "mesh", DATA_JSON_TYPE::OBJECT, strOutError);
		const DATA_JSON_VALUE* pSprite =
			Find_Field(Value, "sprite", DATA_JSON_TYPE::OBJECT, strOutError);
		const DATA_JSON_VALUE* pDecal =
			Find_Field(Value, "decal", DATA_JSON_TYPE::OBJECT, strOutError);
		if (nullptr == pTransform || nullptr == pColor || nullptr == pUV ||
			nullptr == pTiming || nullptr == pMesh || nullptr == pSprite ||
			nullptr == pDecal)
			return false;

		return Try_ReadFloatArray(*pTransform, "position",
			&OutDetail.Transform.vPosition.x, 3u, strOutError) &&
			Try_ReadFloatArray(*pTransform, "rotationDegrees",
				&OutDetail.Transform.vRotationDegrees.x, 3u, strOutError) &&
			Try_ReadFloatArray(*pTransform, "revolutionDegreesPerSecond",
				&OutDetail.Transform.vRevolutionDegreesPerSecond.x, 3u,
				strOutError) &&
			Try_ReadFloatArray(*pTransform, "scale",
				&OutDetail.Transform.vScale.x, 3u, strOutError) &&
			Try_ReadFloatArray(*pColor, "offset",
				&OutDetail.Color.vColorOffset.x, 4u, strOutError) &&
			Try_ReadFloatArray(*pColor, "multiply",
				&OutDetail.Color.vColorMultiply.x, 4u, strOutError) &&
			Try_ReadFloat(*pColor, "clip", OutDetail.Color.fColorClip,
				strOutError) &&
			Try_ReadFloat(*pColor, "emissiveIntensity",
				OutDetail.Color.fEmissiveIntensity, strOutError) &&
			Try_ReadFloat(*pColor, "distortionIntensity",
				OutDetail.Color.fDistortionIntensity, strOutError) &&
			Try_ReadBool(*pColor, "distortionOnBaseMaterial",
				OutDetail.Color.bDistortionOnBaseMaterial, strOutError) &&
			Try_ReadFloat(*pColor, "radialTime",
				OutDetail.Color.fRadialTime, strOutError) &&
			Try_ReadFloat(*pColor, "radialIntensity",
				OutDetail.Color.fRadialIntensity, strOutError) &&
			Try_ReadFloatArray(*pUV, "start",
				&OutDetail.UV.vStart.x, 2u, strOutError) &&
			Try_ReadFloatArray(*pUV, "speed",
				&OutDetail.UV.vSpeed.x, 2u, strOutError) &&
			Try_ReadBool(*pUV, "wave", OutDetail.UV.bWave, strOutError) &&
			Try_ReadFloatArray(*pUV, "waveAmplitude",
				&OutDetail.UV.vWaveAmplitude.x, 2u, strOutError) &&
			Try_ReadFloat(*pUV, "waveFrequency",
				OutDetail.UV.fWaveFrequency, strOutError) &&
			Try_ReadBool(*pUV, "sequence",
				OutDetail.UV.bSequence, strOutError) &&
			Try_ReadBool(*pUV, "loop", OutDetail.UV.bLoop, strOutError) &&
			Try_ReadFloat(*pUV, "sequenceTerm",
				OutDetail.UV.fSequenceTerm, strOutError) &&
			Try_ReadInt(*pUV, "tileColumns",
				OutDetail.UV.iTileColumns, strOutError) &&
			Try_ReadInt(*pUV, "tileRows",
				OutDetail.UV.iTileRows, strOutError) &&
			Try_ReadInt(*pUV, "tileIndex",
				OutDetail.UV.iTileIndex, strOutError) &&
			Try_ReadFloat(*pTiming, "startDelaySeconds",
				OutDetail.Timing.fStartDelaySeconds, strOutError) &&
			Try_ReadFloat(*pTiming, "lifeTimeSeconds",
				OutDetail.Timing.fLifeTimeSeconds, strOutError) &&
			Try_ReadFloat(*pTiming, "afterImageSeconds",
				OutDetail.Timing.fAfterImageSeconds, strOutError) &&
			Try_ReadFloat(*pTiming, "dissolveStartNormalized",
				OutDetail.Timing.fDissolveStartNormalized, strOutError) &&
			Try_ReadBool(*pMesh, "useModelMaterial",
				OutDetail.Mesh.bUseModelMaterial, strOutError) &&
			Try_ReadBool(*pSprite, "billboard",
				OutDetail.Sprite.bBillboard, strOutError) &&
			Try_ReadFloatArray(*pDecal, "size",
				&OutDetail.Decal.vSize.x, 2u, strOutError) &&
			Try_ReadFloat(*pDecal, "depth",
				OutDetail.Decal.fDepth, strOutError);
	}

	void Write_Float2(std::ostringstream& Stream, const float2_t& Value)
	{
		Stream << '[' << Value.x << ", " << Value.y << ']';
	}

	void Write_Float3(std::ostringstream& Stream, const float3_t& Value)
	{
		Stream << '[' << Value.x << ", " << Value.y << ", " << Value.z << ']';
	}

	void Write_Float4(std::ostringstream& Stream, const float4_t& Value)
	{
		Stream << '[' << Value.x << ", " << Value.y << ", "
			<< Value.z << ", " << Value.w << ']';
	}

	void Write_Detail(
		std::ostringstream& Stream,
		const EFFECT_DETAIL_DESC& Detail)
	{
		Stream << "      \"detail\": {\n"
			<< "        \"transform\": { \"position\": ";
		Write_Float3(Stream, Detail.Transform.vPosition);
		Stream << ", \"rotationDegrees\": ";
		Write_Float3(Stream, Detail.Transform.vRotationDegrees);
		Stream << ", \"revolutionDegreesPerSecond\": ";
		Write_Float3(Stream, Detail.Transform.vRevolutionDegreesPerSecond);
		Stream << ", \"scale\": ";
		Write_Float3(Stream, Detail.Transform.vScale);
		Stream << " },\n        \"color\": { \"offset\": ";
		Write_Float4(Stream, Detail.Color.vColorOffset);
		Stream << ", \"multiply\": ";
		Write_Float4(Stream, Detail.Color.vColorMultiply);
		Stream << ", \"clip\": " << Detail.Color.fColorClip
			<< ", \"emissiveIntensity\": " << Detail.Color.fEmissiveIntensity
			<< ", \"distortionIntensity\": " << Detail.Color.fDistortionIntensity
			<< ", \"distortionOnBaseMaterial\": "
			<< (Detail.Color.bDistortionOnBaseMaterial ? "true" : "false")
			<< ", \"radialTime\": " << Detail.Color.fRadialTime
			<< ", \"radialIntensity\": " << Detail.Color.fRadialIntensity
			<< " },\n        \"uv\": { \"start\": ";
		Write_Float2(Stream, Detail.UV.vStart);
		Stream << ", \"speed\": ";
		Write_Float2(Stream, Detail.UV.vSpeed);
		Stream << ", \"wave\": " << (Detail.UV.bWave ? "true" : "false")
			<< ", \"waveAmplitude\": ";
		Write_Float2(Stream, Detail.UV.vWaveAmplitude);
		Stream << ", \"waveFrequency\": " << Detail.UV.fWaveFrequency
			<< ", \"sequence\": " << (Detail.UV.bSequence ? "true" : "false")
			<< ", \"loop\": " << (Detail.UV.bLoop ? "true" : "false")
			<< ", \"sequenceTerm\": " << Detail.UV.fSequenceTerm
			<< ", \"tileColumns\": " << Detail.UV.iTileColumns
			<< ", \"tileRows\": " << Detail.UV.iTileRows
			<< ", \"tileIndex\": " << Detail.UV.iTileIndex
			<< " },\n        \"timing\": { \"startDelaySeconds\": "
			<< Detail.Timing.fStartDelaySeconds
			<< ", \"lifeTimeSeconds\": " << Detail.Timing.fLifeTimeSeconds
			<< ", \"afterImageSeconds\": " << Detail.Timing.fAfterImageSeconds
			<< ", \"dissolveStartNormalized\": "
			<< Detail.Timing.fDissolveStartNormalized
			<< " },\n        \"mesh\": { \"useModelMaterial\": "
			<< (Detail.Mesh.bUseModelMaterial ? "true" : "false")
			<< " },\n        \"sprite\": { \"billboard\": "
			<< (Detail.Sprite.bBillboard ? "true" : "false")
			<< " },\n        \"decal\": { \"size\": ";
		Write_Float2(Stream, Detail.Decal.vSize);
		Stream << ", \"depth\": " << Detail.Decal.fDepth
			<< " }\n      }\n";
	}
}

const char_t* Client::To_EffectElementKindLabel(
	const EFFECT_ELEMENT_KIND eKind)
{
	switch (eKind)
	{
	case EFFECT_ELEMENT_KIND::MESH: return "Mesh";
	case EFFECT_ELEMENT_KIND::SPRITE: return "Sprite";
	case EFFECT_ELEMENT_KIND::PARTICLE: return "Particle";
	case EFFECT_ELEMENT_KIND::DECAL: return "Decal";
	case EFFECT_ELEMENT_KIND::TRAIL: return "Trail";
	case EFFECT_ELEMENT_KIND::END:
	default: return "Invalid";
	}
}

const char_t* Client::To_EffectResourceSlotLabel(
	const EFFECT_RESOURCE_SLOT eSlot)
{
	switch (eSlot)
	{
	case EFFECT_RESOURCE_SLOT::MESH_MODEL: return "Mesh Model";
	case EFFECT_RESOURCE_SLOT::BASE_TEXTURE: return "Base";
	case EFFECT_RESOURCE_SLOT::NOISE_TEXTURE: return "Noise";
	case EFFECT_RESOURCE_SLOT::MASK_TEXTURE: return "Mask";
	case EFFECT_RESOURCE_SLOT::EMISSIVE_TEXTURE: return "Emissive";
	case EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE: return "Dissolve";
	case EFFECT_RESOURCE_SLOT::END:
	default: return "Invalid";
	}
}

const char_t* Client::To_EffectRenderProfileLabel(
	const EFFECT_RENDER_PROFILE eProfile)
{
	switch (eProfile)
	{
	case EFFECT_RENDER_PROFILE::OPAQUE_BACK_DEPTH_WRITE:
		return "Opaque / Cull Back / Depth Write";
	case EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ:
		return "Alpha / Two Sided / Depth Read";
	case EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ:
		return "Additive / Two Sided / Depth Read";
	case EFFECT_RENDER_PROFILE::END:
	default: return "Invalid";
	}
}

Client::EFFECT_RESOURCE_FILE_KIND Client::Get_EffectResourceFileKind(
	const EFFECT_RESOURCE_SLOT eSlot)
{
	if (EFFECT_RESOURCE_SLOT::MESH_MODEL == eSlot)
		return EFFECT_RESOURCE_FILE_KIND::MODEL;
	if (eSlot >= EFFECT_RESOURCE_SLOT::BASE_TEXTURE &&
		eSlot <= EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE)
		return EFFECT_RESOURCE_FILE_KIND::TEXTURE;
	return EFFECT_RESOURCE_FILE_KIND::END;
}

bool_t Client::Is_EffectResourceSlotAllowed(
	const EFFECT_ELEMENT_KIND eKind,
	const EFFECT_RESOURCE_SLOT eSlot)
{
	if (EFFECT_ELEMENT_KIND::END == eKind ||
		EFFECT_RESOURCE_SLOT::END == eSlot)
		return false;
	if (EFFECT_RESOURCE_SLOT::MESH_MODEL == eSlot)
		return EFFECT_ELEMENT_KIND::MESH == eKind;
	return true;
}

Client::EFFECT_RESOURCE_BINDING_DESC* Client::Find_EffectResourceBinding(
	EFFECT_ELEMENT_DESC& Element,
	const EFFECT_RESOURCE_SLOT eSlot)
{
	const auto Iterator = std::find_if(
		Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
		[eSlot](const EFFECT_RESOURCE_BINDING_DESC& Binding)
		{
			return Binding.eSlot == eSlot;
		});
	return Iterator == Element.ResourceBindings.end() ? nullptr : &*Iterator;
}

const Client::EFFECT_RESOURCE_BINDING_DESC*
Client::Find_EffectResourceBinding(
	const EFFECT_ELEMENT_DESC& Element,
	const EFFECT_RESOURCE_SLOT eSlot)
{
	const auto Iterator = std::find_if(
		Element.ResourceBindings.begin(), Element.ResourceBindings.end(),
		[eSlot](const EFFECT_RESOURCE_BINDING_DESC& Binding)
		{
			return Binding.eSlot == eSlot;
		});
	return Iterator == Element.ResourceBindings.end() ? nullptr : &*Iterator;
}

bool_t Client::Validate_EffectDocument(
	const EFFECT_DOCUMENT_DESC& Document,
	std::string& strOutError)
{
	using namespace EffectDocumentDetail;
	if (EFFECT_AUTHORING_FORMAT_VERSION != Document.iFormatVersion)
	{
		strOutError = "Unsupported Effect document version.";
		return false;
	}
	if (!Is_StableAuthoringId(Document.strEffectAssetId) ||
		!Has_VisibleCharacter(Document.strDisplayName) ||
		Document.strDisplayName.size() > 64u)
	{
		strOutError = "Effect Asset ID or Display Name is invalid.";
		return false;
	}

	std::unordered_set<std::string> ElementIds;
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		if (!Is_StableAuthoringId(Element.strElementId) ||
			EFFECT_ELEMENT_KIND::END == Element.eKind ||
			!ElementIds.insert(Element.strElementId).second ||
			EFFECT_RENDER_PROFILE::END == Element.Material.eRenderProfile)
		{
			strOutError = "Element identity, kind, duplicate, or profile is invalid.";
			return false;
		}

		std::unordered_set<uint32_t> Slots;
		for (const EFFECT_RESOURCE_BINDING_DESC& Binding :
			Element.ResourceBindings)
		{
			const EFFECT_RESOURCE_FILE_KIND eFileKind =
				Get_EffectResourceFileKind(Binding.eSlot);
			if (!Is_EffectResourceSlotAllowed(Element.eKind, Binding.eSlot) ||
				EFFECT_RESOURCE_FILE_KIND::END == eFileKind ||
				!Slots.insert(static_cast<uint32_t>(Binding.eSlot)).second ||
				!Is_SafeEffectAssetId(Binding.strAssetId, eFileKind))
			{
				strOutError = "Effect resource slot, type, path, or duplicate is invalid.";
				return false;
			}
		}

		const EFFECT_DETAIL_DESC& Detail = Element.Detail;
		if (!Is_Finite(Detail.Transform.vPosition) ||
			!Is_Finite(Detail.Transform.vRotationDegrees) ||
			!Is_Finite(Detail.Transform.vRevolutionDegreesPerSecond) ||
			!Is_Finite(Detail.Transform.vScale) ||
			Detail.Transform.vScale.x <= 0.f ||
			Detail.Transform.vScale.y <= 0.f ||
			Detail.Transform.vScale.z <= 0.f ||
			!Is_Finite(Detail.Color.vColorOffset) ||
			!Is_Finite(Detail.Color.vColorMultiply) ||
			!std::isfinite(Detail.Color.fColorClip) ||
			Detail.Color.fColorClip < 0.f || Detail.Color.fColorClip > 1.f ||
			!std::isfinite(Detail.Color.fEmissiveIntensity) ||
			Detail.Color.fEmissiveIntensity < 0.f ||
			!std::isfinite(Detail.Color.fDistortionIntensity) ||
			Detail.Color.fDistortionIntensity < 0.f ||
			!std::isfinite(Detail.Color.fRadialTime) ||
			!std::isfinite(Detail.Color.fRadialIntensity) ||
			!Is_Finite(Detail.UV.vStart) || !Is_Finite(Detail.UV.vSpeed) ||
			!Is_Finite(Detail.UV.vWaveAmplitude) ||
			!std::isfinite(Detail.UV.fWaveFrequency) ||
			Detail.UV.fWaveFrequency < 0.f ||
			!std::isfinite(Detail.UV.fSequenceTerm) ||
			Detail.UV.fSequenceTerm <= 0.f ||
			Detail.UV.iTileColumns <= 0 || Detail.UV.iTileRows <= 0 ||
			Detail.UV.iTileIndex < 0 ||
			Detail.UV.iTileIndex >=
				Detail.UV.iTileColumns * Detail.UV.iTileRows ||
			!std::isfinite(Detail.Timing.fStartDelaySeconds) ||
			Detail.Timing.fStartDelaySeconds < 0.f ||
			!std::isfinite(Detail.Timing.fLifeTimeSeconds) ||
			Detail.Timing.fLifeTimeSeconds <= 0.f ||
			!std::isfinite(Detail.Timing.fAfterImageSeconds) ||
			Detail.Timing.fAfterImageSeconds < 0.f ||
			!std::isfinite(Detail.Timing.fDissolveStartNormalized) ||
			Detail.Timing.fDissolveStartNormalized < 0.f ||
			Detail.Timing.fDissolveStartNormalized > 1.f ||
			!Is_Finite(Detail.Decal.vSize) ||
			Detail.Decal.vSize.x <= 0.f || Detail.Decal.vSize.y <= 0.f ||
			!std::isfinite(Detail.Decal.fDepth) || Detail.Decal.fDepth <= 0.f)
		{
			strOutError = "Effect Detail contains an invalid number or range.";
			return false;
		}
	}
	strOutError.clear();
	return true;
}

std::filesystem::path Client::Resolve_EffectDocumentPath(
	const std::string& strEffectAssetId)
{
	if (!EffectDocumentDetail::Is_StableAuthoringId(strEffectAssetId))
		return {};
	return CProjectDataRoot::Resolve(
		std::filesystem::path(L"Effects") / L"Authored" /
		std::filesystem::path(strEffectAssetId + ".effect.json"));
}

bool_t Client::Try_ParseEffectDocumentJson(
	const std::string& strJson,
	EFFECT_DOCUMENT_DESC& OutDocument,
	std::string& strOutError)
{
	using namespace EffectDocumentDetail;
	DATA_JSON_VALUE Root;
	if (!CDataJson::Parse(strJson, Root, strOutError) || !Root.Is_Object())
	{
		if (strOutError.empty()) strOutError = "Effect root must be an object.";
		return false;
	}
	const DATA_JSON_VALUE* pVersion =
		Find_Field(Root, "version", DATA_JSON_TYPE::NUMBER, strOutError);
	const DATA_JSON_VALUE* pAssetId =
		Find_Field(Root, "effectAssetId", DATA_JSON_TYPE::STRING, strOutError);
	const DATA_JSON_VALUE* pDisplayName =
		Find_Field(Root, "displayName", DATA_JSON_TYPE::STRING, strOutError);
	const DATA_JSON_VALUE* pElements =
		Find_Field(Root, "elements", DATA_JSON_TYPE::ARRAY, strOutError);
	if (nullptr == pVersion || nullptr == pAssetId ||
		nullptr == pDisplayName || nullptr == pElements ||
		!std::isfinite(pVersion->Get_Number()))
	{
		strOutError = "Effect document fields or version are invalid.";
		return false;
	}
	const uint32_t iSourceVersion =
		static_cast<uint32_t>(pVersion->Get_Number());
	if (pVersion->Get_Number() != static_cast<double>(iSourceVersion) ||
		iSourceVersion < EFFECT_AUTHORING_MIN_SUPPORTED_VERSION ||
		iSourceVersion > EFFECT_AUTHORING_FORMAT_VERSION)
	{
		strOutError = "Effect document version is not supported.";
		return false;
	}

	EFFECT_DOCUMENT_DESC StagedDocument;
	StagedDocument.strEffectAssetId = pAssetId->Get_String();
	StagedDocument.strDisplayName = pDisplayName->Get_String();
	for (const DATA_JSON_VALUE& ElementValue : pElements->Get_Array())
	{
		if (!ElementValue.Is_Object())
		{
			strOutError = "Effect Element must be an object.";
			return false;
		}
		const DATA_JSON_VALUE* pId =
			Find_Field(ElementValue, "id", DATA_JSON_TYPE::STRING, strOutError);
		const DATA_JSON_VALUE* pKind =
			Find_Field(ElementValue, "kind", DATA_JSON_TYPE::STRING, strOutError);
		const DATA_JSON_VALUE* pResources =
			Find_Field(ElementValue, "resources", DATA_JSON_TYPE::ARRAY, strOutError);
		const DATA_JSON_VALUE* pMaterial =
			Find_Field(ElementValue, "material", DATA_JSON_TYPE::OBJECT, strOutError);
		const DATA_JSON_VALUE* pDetail = iSourceVersion >= 4u ?
			Find_Field(ElementValue, "detail", DATA_JSON_TYPE::OBJECT, strOutError) :
			nullptr;
		if (nullptr == pId || nullptr == pKind || nullptr == pResources ||
			nullptr == pMaterial || (iSourceVersion >= 4u && nullptr == pDetail))
			return false;

		EFFECT_ELEMENT_DESC StagedElement;
		StagedElement.strElementId = pId->Get_String();
		if (!Try_ParseElementToken(pKind->Get_String(), StagedElement.eKind))
		{
			strOutError = "Effect Element kind is invalid.";
			return false;
		}
		for (const DATA_JSON_VALUE& ResourceValue : pResources->Get_Array())
		{
			if (!ResourceValue.Is_Object())
			{
				strOutError = "Effect resource must be an object.";
				return false;
			}
			const DATA_JSON_VALUE* pSlot =
				Find_Field(ResourceValue, "slot", DATA_JSON_TYPE::STRING, strOutError);
			const DATA_JSON_VALUE* pResourceAssetId =
				Find_Field(ResourceValue, "assetId", DATA_JSON_TYPE::STRING, strOutError);
			EFFECT_RESOURCE_BINDING_DESC Binding;
			if (nullptr == pSlot || nullptr == pResourceAssetId ||
				!Try_ParseSlotToken(pSlot->Get_String(), Binding.eSlot))
			{
				strOutError = "Effect resource binding is invalid.";
				return false;
			}
			Binding.strAssetId = pResourceAssetId->Get_String();
			StagedElement.ResourceBindings.push_back(std::move(Binding));
		}
		const DATA_JSON_VALUE* pProfile = Find_Field(
			*pMaterial, "renderProfile", DATA_JSON_TYPE::STRING, strOutError);
		if (nullptr == pProfile || !Try_ParseProfileToken(
			pProfile->Get_String(), StagedElement.Material.eRenderProfile) ||
			(iSourceVersion >= 4u &&
				!Try_ReadDetail(*pDetail, StagedElement.Detail, strOutError)))
			return false;
		StagedDocument.Elements.push_back(std::move(StagedElement));
	}
	if (!Validate_EffectDocument(StagedDocument, strOutError))
		return false;
	OutDocument = std::move(StagedDocument);
	return true;
}

std::string Client::Serialize_EffectDocumentJson(
	const EFFECT_DOCUMENT_DESC& Document)
{
	using namespace EffectDocumentDetail;
	std::ostringstream Stream;
	Stream << "{\n  \"version\": " << Document.iFormatVersion
		<< ",\n  \"effectAssetId\": \""
		<< CDataJson::Escape(Document.strEffectAssetId)
		<< "\",\n  \"displayName\": \""
		<< CDataJson::Escape(Document.strDisplayName)
		<< "\",\n  \"elements\": [\n";
	for (size_t iElement = 0; iElement < Document.Elements.size(); ++iElement)
	{
		const EFFECT_ELEMENT_DESC& Element = Document.Elements[iElement];
		Stream << "    {\n      \"id\": \""
			<< CDataJson::Escape(Element.strElementId)
			<< "\",\n      \"kind\": \"" << To_ElementToken(Element.eKind)
			<< "\",\n      \"resources\": [\n";
		for (size_t iResource = 0;
			iResource < Element.ResourceBindings.size(); ++iResource)
		{
			const EFFECT_RESOURCE_BINDING_DESC& Binding =
				Element.ResourceBindings[iResource];
			Stream << "        { \"slot\": \"" << To_SlotToken(Binding.eSlot)
				<< "\", \"assetId\": \""
				<< CDataJson::Escape(Binding.strAssetId) << "\" }";
			if (iResource + 1u != Element.ResourceBindings.size()) Stream << ',';
			Stream << '\n';
		}
		Stream << "      ],\n      \"material\": { \"renderProfile\": \""
			<< To_ProfileToken(Element.Material.eRenderProfile) << "\" },\n";
		Write_Detail(Stream, Element.Detail);
		Stream << "    }";
		if (iElement + 1u != Document.Elements.size()) Stream << ',';
		Stream << '\n';
	}
	Stream << "  ]\n}\n";
	return Stream.str();
}

namespace Client::EffectToolDetail
{
	bool_t Contains_NoCase(
		const std::string& strValue,
		const std::string& strFilter)
	{
		if (strFilter.empty()) return true;
		return std::search(strValue.begin(), strValue.end(),
			strFilter.begin(), strFilter.end(),
			[](const char_t left, const char_t right)
			{
				return std::tolower(static_cast<unsigned char>(left)) ==
					std::tolower(static_cast<unsigned char>(right));
			}) != strValue.end();
	}

	EFFECT_ELEMENT_DESC* Find_Element(
		EFFECT_DOCUMENT_DESC& Document,
		const std::string& strElementId)
	{
		const auto Iterator = std::find_if(
			Document.Elements.begin(), Document.Elements.end(),
			[&strElementId](const EFFECT_ELEMENT_DESC& Element)
			{
				return Element.strElementId == strElementId;
			});
		return Iterator == Document.Elements.end() ? nullptr : &*Iterator;
	}

	const EFFECT_ELEMENT_DESC* Find_Element(
		const EFFECT_DOCUMENT_DESC& Document,
		const std::string& strElementId)
	{
		const auto Iterator = std::find_if(
			Document.Elements.begin(), Document.Elements.end(),
			[&strElementId](const EFFECT_ELEMENT_DESC& Element)
			{
				return Element.strElementId == strElementId;
			});
		return Iterator == Document.Elements.end() ? nullptr : &*Iterator;
	}

	EFFECT_RESOURCE_FILE_KIND Classify_ResourceFile(
		const std::filesystem::path& FilePath)
	{
		std::wstring Extension = FilePath.extension().wstring();
		std::transform(Extension.begin(), Extension.end(), Extension.begin(),
			[](const wchar_t value)
			{
				return static_cast<wchar_t>(std::towlower(value));
			});
		if (Extension == L".wmodel") return EFFECT_RESOURCE_FILE_KIND::MODEL;
		if (Extension == L".dds") return EFFECT_RESOURCE_FILE_KIND::TEXTURE;
		return EFFECT_RESOURCE_FILE_KIND::END;
	}
}

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

void Client::CEffect_Tool::Render()
{
	ImGui::SetNextWindowSize(ImVec2(1240.f, 820.f),
		ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("LostArk Effect Tool###LostArkEffectToolG6"))
	{
		ImGui::End();
		return;
	}
	ImGui::TextWrapped(
		"G6: common/type Detail and explicit-time Mesh/Sprite/Decal Preview. "
		"Particle/Trail modules and distortion composite start in G8.");

	if (m_ActiveDocument.has_value()) Render_ActiveDocumentPanel();
	else Render_NewDocumentPanel();
	ImGui::Separator();
	Render_EffectTypeSelector();
	if (m_ActiveDocument.has_value())
	{
		Render_AddElementPanel();
		Render_ElementList();
		if (ImGui::BeginTable("EffectToolWorkArea", 2,
			ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerV))
		{
			ImGui::TableNextColumn();
			Render_MaterialPanel();
			Render_DetailPanel();
			ImGui::TableNextColumn();
			Render_PreviewPanel();
			ImGui::EndTable();
		}
	}
	else
		ImGui::TextDisabled("Create or load a document first.");
	ImGui::End();
}

void Client::CEffect_Tool::Render_NewDocumentPanel()
{
	ImGui::SeparatorText("Data File");
	ImGui::InputText("Effect Asset ID", m_NewAssetId.data(),
		m_NewAssetId.size());
	ImGui::InputText("Display Name", m_NewDisplayName.data(),
		m_NewDisplayName.size());
	if (ImGui::Button("Create Document")) Try_CreateDocument();
	ImGui::SameLine();
	if (ImGui::Button("Load Document")) Try_LoadDocument();
	if (!m_strDocumentStatus.empty())
		ImGui::TextWrapped("%s", m_strDocumentStatus.c_str());
}

void Client::CEffect_Tool::Render_ActiveDocumentPanel()
{
	ImGui::SeparatorText("Active Document");
	const EFFECT_DOCUMENT_DESC& Document = *m_ActiveDocument;
	ImGui::Text("v%u | %s | %s | Elements %zu",
		Document.iFormatVersion, Document.strEffectAssetId.c_str(),
		Document.strDisplayName.c_str(), Document.Elements.size());
	if (ImGui::Button("Save Active Document")) Try_SaveDocument();
	ImGui::SameLine();
	if (ImGui::Button("Discard Active Document"))
		Discard_ActiveDocument();
	if (!m_strDocumentStatus.empty())
		ImGui::TextWrapped("%s", m_strDocumentStatus.c_str());
}

void Client::CEffect_Tool::Render_EffectTypeSelector()
{
	ImGui::TextUnformatted("Next Element Type");
	for (int32_t i = 0;
		i < static_cast<int32_t>(EFFECT_ELEMENT_KIND::END); ++i)
	{
		ImGui::SameLine();
		const EFFECT_ELEMENT_KIND eKind =
			static_cast<EFFECT_ELEMENT_KIND>(i);
		if (ImGui::RadioButton(To_EffectElementKindLabel(eKind),
			m_eSelectedEffectType == eKind))
			m_eSelectedEffectType = eKind;
	}
}

void Client::CEffect_Tool::Render_AddElementPanel()
{
	ImGui::SeparatorText("Add Element");
	ImGui::InputText("Element ID", m_NewElementId.data(),
		m_NewElementId.size());
	if (ImGui::Button("Add Element")) Try_AddElement();
	if (!m_strElementStatus.empty())
		ImGui::TextWrapped("%s", m_strElementStatus.c_str());
}

void Client::CEffect_Tool::Render_ElementList()
{
	using EffectToolDetail::Find_Element;
	ImGui::SeparatorText("Current Elements");
	if (m_ActiveDocument->Elements.empty())
	{
		ImGui::TextDisabled("No Elements have been added.");
		m_strSelectedElementId.clear();
		return;
	}
	if (!m_strSelectedElementId.empty() &&
		nullptr == Find_Element(*m_ActiveDocument, m_strSelectedElementId))
		m_strSelectedElementId.clear();

	if (!ImGui::BeginTable("EffectElementList", 4,
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
		ImGuiTableFlags_SizingStretchProp))
		return;
	ImGui::TableSetupColumn("Element ID");
	ImGui::TableSetupColumn("Kind");
	ImGui::TableSetupColumn("Bindings");
	ImGui::TableSetupColumn("Render Profile");
	ImGui::TableHeadersRow();
	for (const EFFECT_ELEMENT_DESC& Element : m_ActiveDocument->Elements)
	{
		ImGui::PushID(Element.strElementId.c_str());
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		if (ImGui::Selectable(Element.strElementId.c_str(),
			m_strSelectedElementId == Element.strElementId,
			ImGuiSelectableFlags_SpanAllColumns))
		{
			m_strSelectedElementId = Element.strElementId;
			m_strSelectedResourceAssetId.clear();
			m_eSelectedResourceSlot =
				EFFECT_ELEMENT_KIND::MESH == Element.eKind ?
				EFFECT_RESOURCE_SLOT::MESH_MODEL :
				EFFECT_RESOURCE_SLOT::BASE_TEXTURE;
			m_fPreviewTimeSeconds = 0.f;
			m_pPreview->Clear();
		}
		ImGui::TableSetColumnIndex(1);
		ImGui::TextUnformatted(To_EffectElementKindLabel(Element.eKind));
		ImGui::TableSetColumnIndex(2);
		ImGui::Text("%zu", Element.ResourceBindings.size());
		ImGui::TableSetColumnIndex(3);
		ImGui::TextUnformatted(
			To_EffectRenderProfileLabel(Element.Material.eRenderProfile));
		ImGui::PopID();
	}
	ImGui::EndTable();
}

void Client::CEffect_Tool::Render_MaterialPanel()
{
	using EffectToolDetail::Contains_NoCase;
	using EffectToolDetail::Find_Element;
	ImGui::SeparatorText("Material");
	EFFECT_ELEMENT_DESC* pElement = m_strSelectedElementId.empty() ?
		nullptr : Find_Element(*m_ActiveDocument, m_strSelectedElementId);
	if (nullptr == pElement)
	{
		ImGui::TextDisabled("Select an Element first.");
		return;
	}

	if (ImGui::BeginCombo("Render Profile",
		To_EffectRenderProfileLabel(pElement->Material.eRenderProfile)))
	{
		for (int32_t i = 0;
			i < static_cast<int32_t>(EFFECT_RENDER_PROFILE::END); ++i)
		{
			const EFFECT_RENDER_PROFILE eProfile =
				static_cast<EFFECT_RENDER_PROFILE>(i);
			if (ImGui::Selectable(To_EffectRenderProfileLabel(eProfile),
				pElement->Material.eRenderProfile == eProfile))
				Try_ApplyRenderProfile(eProfile);
		}
		ImGui::EndCombo();
	}
	if (ImGui::BeginCombo("Input Slot",
		To_EffectResourceSlotLabel(m_eSelectedResourceSlot)))
	{
		for (int32_t i = 0;
			i < static_cast<int32_t>(EFFECT_RESOURCE_SLOT::END); ++i)
		{
			const EFFECT_RESOURCE_SLOT eSlot =
				static_cast<EFFECT_RESOURCE_SLOT>(i);
			if (!Is_EffectResourceSlotAllowed(pElement->eKind, eSlot)) continue;
			if (ImGui::Selectable(To_EffectResourceSlotLabel(eSlot),
				m_eSelectedResourceSlot == eSlot))
			{
				m_eSelectedResourceSlot = eSlot;
				m_strSelectedResourceAssetId.clear();
			}
		}
		ImGui::EndCombo();
	}
	const EFFECT_RESOURCE_BINDING_DESC* pBinding =
		Find_EffectResourceBinding(*pElement, m_eSelectedResourceSlot);
	ImGui::Text("Current: %s", nullptr == pBinding ?
		"Unbound" : pBinding->strAssetId.c_str());
	if (!m_bResourceCatalogRefreshAttempted) Refresh_ResourceCatalog();
	if (ImGui::Button("Refresh Resources/Effect")) Refresh_ResourceCatalog();
	ImGui::InputText("Resource Filter", m_ResourceFilter.data(),
		m_ResourceFilter.size());
	const EFFECT_RESOURCE_FILE_KIND eExpected =
		Get_EffectResourceFileKind(m_eSelectedResourceSlot);
	const std::string strFilter = m_ResourceFilter.data();
	if (ImGui::BeginChild("EffectResourceCatalog", ImVec2(0.f, 140.f), true))
	{
		for (const EFFECT_RESOURCE_CATALOG_ENTRY& Entry : m_ResourceCatalog)
		{
			if (Entry.eFileKind != eExpected ||
				!Contains_NoCase(Entry.strAssetId, strFilter)) continue;
			if (ImGui::Selectable(Entry.strAssetId.c_str(),
				m_strSelectedResourceAssetId == Entry.strAssetId))
				m_strSelectedResourceAssetId = Entry.strAssetId;
		}
	}
	ImGui::EndChild();
	ImGui::BeginDisabled(m_strSelectedResourceAssetId.empty());
	if (ImGui::Button("Bind Selected Resource")) Try_BindSelectedResource();
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(nullptr == pBinding);
	if (ImGui::Button("Clear Selected Slot")) Try_ClearSelectedResource();
	ImGui::EndDisabled();
	if (!m_strResourceStatus.empty())
		ImGui::TextWrapped("%s", m_strResourceStatus.c_str());
}

void Client::CEffect_Tool::Render_DetailPanel()
{
	using EffectToolDetail::Find_Element;
	ImGui::SeparatorText("Effect Detail");
	const EFFECT_ELEMENT_DESC* pElement = m_strSelectedElementId.empty() ?
		nullptr : Find_Element(*m_ActiveDocument, m_strSelectedElementId);
	if (nullptr == pElement)
	{
		ImGui::TextDisabled("Select an Element first.");
		return;
	}
	EFFECT_DETAIL_DESC Staged = pElement->Detail;
	bool_t bChanged = false;
	if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		bChanged |= ImGui::DragFloat3("Position", &Staged.Transform.vPosition.x, 0.01f);
		bChanged |= ImGui::DragFloat3("Rotation Degrees",
			&Staged.Transform.vRotationDegrees.x, 0.5f);
		bChanged |= ImGui::DragFloat3("Revolution Deg/Sec",
			&Staged.Transform.vRevolutionDegreesPerSecond.x, 0.5f);
		bChanged |= ImGui::DragFloat3("Scale", &Staged.Transform.vScale.x, 0.01f);
		ImGui::TreePop();
	}
	if (ImGui::TreeNodeEx("Color", ImGuiTreeNodeFlags_DefaultOpen))
	{
		bChanged |= ImGui::ColorEdit4("Color Offset", &Staged.Color.vColorOffset.x,
			ImGuiColorEditFlags_Float);
		bChanged |= ImGui::ColorEdit4("Color Multiply",
			&Staged.Color.vColorMultiply.x, ImGuiColorEditFlags_Float);
		bChanged |= ImGui::SliderFloat("Color Clip", &Staged.Color.fColorClip, 0.f, 1.f);
		bChanged |= ImGui::DragFloat("Emissive Intensity",
			&Staged.Color.fEmissiveIntensity, 0.05f, 0.f, 100.f);
		bChanged |= ImGui::DragFloat("Distortion Intensity",
			&Staged.Color.fDistortionIntensity, 0.01f, 0.f, 10.f);
		bChanged |= ImGui::Checkbox("Distortion On Base Material",
			&Staged.Color.bDistortionOnBaseMaterial);
		bChanged |= ImGui::DragFloat("Radial Time", &Staged.Color.fRadialTime, 0.01f);
		bChanged |= ImGui::DragFloat("Radial Intensity",
			&Staged.Color.fRadialIntensity, 0.01f);
		ImGui::TextDisabled("Distortion values are authored now; composite is G08.");
		ImGui::TreePop();
	}
	if (ImGui::TreeNodeEx("UV", ImGuiTreeNodeFlags_DefaultOpen))
	{
		bChanged |= ImGui::DragFloat2("UV Start", &Staged.UV.vStart.x, 0.01f);
		bChanged |= ImGui::DragFloat2("UV Speed", &Staged.UV.vSpeed.x, 0.01f);
		bChanged |= ImGui::Checkbox("UV Wave", &Staged.UV.bWave);
		bChanged |= ImGui::DragFloat2("Wave Amplitude",
			&Staged.UV.vWaveAmplitude.x, 0.01f);
		bChanged |= ImGui::DragFloat("Wave Frequency",
			&Staged.UV.fWaveFrequency, 0.01f, 0.f, 100.f);
		bChanged |= ImGui::Checkbox("Is Sequence", &Staged.UV.bSequence);
		bChanged |= ImGui::Checkbox("Is Loop", &Staged.UV.bLoop);
		bChanged |= ImGui::DragFloat("Sequence Term",
			&Staged.UV.fSequenceTerm, 0.01f, 0.001f, 10.f);
		bChanged |= ImGui::DragInt("UV Tile Columns", &Staged.UV.iTileColumns, 1.f, 1, 64);
		bChanged |= ImGui::DragInt("UV Tile Rows", &Staged.UV.iTileRows, 1.f, 1, 64);
		bChanged |= ImGui::DragInt("UV Tile Index", &Staged.UV.iTileIndex, 1.f, 0, 4095);
		ImGui::TreePop();
	}
	if (ImGui::TreeNodeEx("Timing", ImGuiTreeNodeFlags_DefaultOpen))
	{
		bChanged |= ImGui::DragFloat("Start Delay",
			&Staged.Timing.fStartDelaySeconds, 0.01f, 0.f, 60.f);
		bChanged |= ImGui::DragFloat("Life Time",
			&Staged.Timing.fLifeTimeSeconds, 0.01f, 0.01f, 600.f);
		bChanged |= ImGui::DragFloat("After Image Time",
			&Staged.Timing.fAfterImageSeconds, 0.01f, 0.f, 60.f);
		bChanged |= ImGui::SliderFloat("Dissolve Start",
			&Staged.Timing.fDissolveStartNormalized, 0.f, 1.f);
		ImGui::TextDisabled("After Image is stored; Trail consumes it in G8.");
		ImGui::TreePop();
	}
	if (EFFECT_ELEMENT_KIND::MESH == pElement->eKind)
		bChanged |= ImGui::Checkbox("Use Model Material",
			&Staged.Mesh.bUseModelMaterial);
	else if (EFFECT_ELEMENT_KIND::SPRITE == pElement->eKind)
		bChanged |= ImGui::Checkbox("Billboard", &Staged.Sprite.bBillboard);
	else if (EFFECT_ELEMENT_KIND::DECAL == pElement->eKind)
	{
		bChanged |= ImGui::DragFloat2("Decal Size", &Staged.Decal.vSize.x, 0.01f);
		bChanged |= ImGui::DragFloat("Decal Depth", &Staged.Decal.fDepth, 0.01f);
	}
	if (bChanged) Try_ApplyDetail(Staged);
	if (!m_strDetailStatus.empty())
		ImGui::TextWrapped("%s", m_strDetailStatus.c_str());
}

void Client::CEffect_Tool::Render_PreviewPanel()
{
	using EffectToolDetail::Find_Element;
	ImGui::SeparatorText("Stateless Preview");
	const EFFECT_ELEMENT_DESC* pElement = m_strSelectedElementId.empty() ?
		nullptr : Find_Element(*m_ActiveDocument, m_strSelectedElementId);
	if (nullptr == pElement)
	{
		ImGui::TextDisabled("Select an Element first.");
		return;
	}
	const f32_t fMaxTime = pElement->Detail.Timing.fStartDelaySeconds +
		pElement->Detail.Timing.fLifeTimeSeconds +
		pElement->Detail.Timing.fAfterImageSeconds;
	ImGui::SliderFloat("Sample Time", &m_fPreviewTimeSeconds,
		0.f, (std::max)(0.01f, fMaxTime));
	const ImVec2 Available = ImGui::GetContentRegionAvail();
	const uint32_t iSize = static_cast<uint32_t>(
		(std::max)(256.f, (std::min)(Available.x, 640.f)));
	const HRESULT Result = m_pPreview->Render(
		*pElement, m_fPreviewTimeSeconds, iSize, iSize);
	if (SUCCEEDED(Result) && S_FALSE != Result &&
		nullptr != m_pPreview->Get_TextureView())
	{
		ImGui::Image(
			reinterpret_cast<ImTextureID>(m_pPreview->Get_TextureView()),
			ImVec2(static_cast<f32_t>(iSize), static_cast<f32_t>(iSize)));
	}
	ImGui::TextWrapped("%s", m_pPreview->Get_Status().c_str());
}

bool_t Client::CEffect_Tool::Try_CreateDocument()
{
	if (m_ActiveDocument.has_value())
	{
		m_strDocumentStatus = "Discard the active document first.";
		return false;
	}
	EFFECT_DOCUMENT_DESC Staged;
	Staged.strEffectAssetId = m_NewAssetId.data();
	Staged.strDisplayName = m_NewDisplayName.data();
	std::string Error;
	if (!Validate_EffectDocument(Staged, Error))
	{
		m_strDocumentStatus = Error;
		return false;
	}
	m_ActiveDocument = std::move(Staged);
	m_NewAssetId.fill('\0');
	m_NewDisplayName.fill('\0');
	m_NewElementId.fill('\0');
	m_strSelectedElementId.clear();
	m_pPreview->Clear();
	m_strDocumentStatus = "Created in memory; not saved.";
	return true;
}

bool_t Client::CEffect_Tool::Try_AddElement()
{
	if (!m_ActiveDocument.has_value()) return false;
	const std::string Id = m_NewElementId.data();
	EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
	EFFECT_ELEMENT_DESC Element;
	Element.strElementId = Id;
	Element.eKind = m_eSelectedEffectType;
	Staged.Elements.push_back(std::move(Element));
	std::string Error;
	if (!Validate_EffectDocument(Staged, Error))
	{
		m_strElementStatus = Error;
		return false;
	}
	m_ActiveDocument = std::move(Staged);
	m_NewElementId.fill('\0');
	m_strSelectedElementId = Id;
	m_eSelectedResourceSlot = EFFECT_ELEMENT_KIND::MESH == m_eSelectedEffectType ?
		EFFECT_RESOURCE_SLOT::MESH_MODEL : EFFECT_RESOURCE_SLOT::BASE_TEXTURE;
	m_fPreviewTimeSeconds = 0.f;
	m_pPreview->Clear();
	m_strElementStatus = "Element committed in memory.";
	return true;
}

bool_t Client::CEffect_Tool::Try_SaveDocument()
{
	if (!m_ActiveDocument.has_value()) return false;
	std::string Error;
	if (!Validate_EffectDocument(*m_ActiveDocument, Error))
	{
		m_strDocumentStatus = "Save rejected: " + Error;
		return false;
	}
	const std::filesystem::path Destination =
		Resolve_EffectDocumentPath(m_ActiveDocument->strEffectAssetId);
	if (Destination.empty()) return false;
	std::error_code DirectoryError;
	std::filesystem::create_directories(
		Destination.parent_path(), DirectoryError);
	if (DirectoryError) return false;
	std::filesystem::path Temporary = Destination;
	Temporary += L".tmp";
	const std::string Json =
		Serialize_EffectDocumentJson(*m_ActiveDocument);
	std::ofstream Output(Temporary, std::ios::binary | std::ios::trunc);
	if (!Output) return false;
	Output.write(Json.data(), static_cast<std::streamsize>(Json.size()));
	Output.close();
	if (!Output || !MoveFileExW(Temporary.c_str(), Destination.c_str(),
		MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
	{
		std::error_code Ignore;
		std::filesystem::remove(Temporary, Ignore);
		m_strDocumentStatus = "Save failed; previous file preserved.";
		return false;
	}
	m_strDocumentStatus = "Saved v4 Effect document.";
	return true;
}

bool_t Client::CEffect_Tool::Try_LoadDocument()
{
	if (m_ActiveDocument.has_value()) return false;
	const std::string RequestedId = m_NewAssetId.data();
	const std::filesystem::path Source =
		Resolve_EffectDocumentPath(RequestedId);
	std::ifstream Input(Source, std::ios::binary);
	if (Source.empty() || !Input)
	{
		m_strDocumentStatus = "Effect document cannot be opened.";
		return false;
	}
	std::ostringstream Buffer;
	Buffer << Input.rdbuf();
	EFFECT_DOCUMENT_DESC Staged;
	std::string Error;
	if (Input.bad() || !Try_ParseEffectDocumentJson(
		Buffer.str(), Staged, Error) || Staged.strEffectAssetId != RequestedId)
	{
		m_strDocumentStatus = "Load rejected; current state preserved: " + Error;
		return false;
	}
	m_ActiveDocument = std::move(Staged);
	m_NewAssetId.fill('\0');
	m_NewDisplayName.fill('\0');
	m_NewElementId.fill('\0');
	m_strSelectedElementId.clear();
	m_fPreviewTimeSeconds = 0.f;
	m_pPreview->Clear();
	m_strDocumentStatus = "Loaded and committed v4 Effect document.";
	return true;
}

bool_t Client::CEffect_Tool::Refresh_ResourceCatalog()
{
	using EffectToolDetail::Classify_ResourceFile;
	m_bResourceCatalogRefreshAttempted = true;
	const std::filesystem::path Root =
		CRuntimeAssetRoot::Resolve(std::filesystem::path(L"Effect"));
	std::error_code Error;
	if (Root.empty() || !std::filesystem::is_directory(Root, Error) || Error)
		return false;
	std::vector<EFFECT_RESOURCE_CATALOG_ENTRY> Staged;
	std::filesystem::recursive_directory_iterator It(
		Root, std::filesystem::directory_options::none, Error);
	const std::filesystem::recursive_directory_iterator End;
	for (; !Error && It != End; It.increment(Error))
	{
		std::error_code FileError;
		if (!It->is_regular_file(FileError))
		{
			if (FileError) Error = FileError;
			continue;
		}
		const EFFECT_RESOURCE_FILE_KIND Kind = Classify_ResourceFile(It->path());
		if (EFFECT_RESOURCE_FILE_KIND::END == Kind) continue;
		const std::filesystem::path Relative =
			It->path().lexically_relative(Root);
		if (Relative.empty())
		{
			Error = std::make_error_code(std::errc::invalid_argument);
			break;
		}
		Staged.push_back({
			(std::filesystem::path("Effect") / Relative).generic_string(), Kind });
	}
	if (Error) return false;
	std::sort(Staged.begin(), Staged.end(),
		[](const auto& Left, const auto& Right)
		{
			return Left.strAssetId < Right.strAssetId;
		});
	if (std::adjacent_find(Staged.begin(), Staged.end(),
		[](const auto& Left, const auto& Right)
		{
			return Left.strAssetId == Right.strAssetId;
		}) != Staged.end()) return false;
	m_ResourceCatalog = std::move(Staged);
	m_strResourceStatus = "Catalog refreshed: " +
		std::to_string(m_ResourceCatalog.size()) + " files.";
	return true;
}

bool_t Client::CEffect_Tool::Try_BindSelectedResource()
{
	using EffectToolDetail::Find_Element;
	if (!m_ActiveDocument.has_value() || m_strSelectedElementId.empty() ||
		m_strSelectedResourceAssetId.empty()) return false;
	const auto Catalog = std::find_if(
		m_ResourceCatalog.begin(), m_ResourceCatalog.end(),
		[this](const EFFECT_RESOURCE_CATALOG_ENTRY& Entry)
		{
			return Entry.strAssetId == m_strSelectedResourceAssetId;
		});
	if (Catalog == m_ResourceCatalog.end() || Catalog->eFileKind !=
		Get_EffectResourceFileKind(m_eSelectedResourceSlot)) return false;
	EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
	EFFECT_ELEMENT_DESC* Element = Find_Element(Staged, m_strSelectedElementId);
	if (nullptr == Element ||
		!Is_EffectResourceSlotAllowed(Element->eKind, m_eSelectedResourceSlot))
		return false;
	Element->ResourceBindings.erase(std::remove_if(
		Element->ResourceBindings.begin(), Element->ResourceBindings.end(),
		[this](const EFFECT_RESOURCE_BINDING_DESC& Binding)
		{
			return Binding.eSlot == m_eSelectedResourceSlot;
		}), Element->ResourceBindings.end());
	Element->ResourceBindings.push_back(
		{ m_eSelectedResourceSlot, Catalog->strAssetId });
	std::string Error;
	if (!Validate_EffectDocument(Staged, Error))
	{
		m_strResourceStatus = Error;
		return false;
	}
	m_ActiveDocument = std::move(Staged);
	m_pPreview->Clear();
	m_strResourceStatus = "Binding committed in memory.";
	return true;
}

bool_t Client::CEffect_Tool::Try_ClearSelectedResource()
{
	using EffectToolDetail::Find_Element;
	if (!m_ActiveDocument.has_value()) return false;
	EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
	EFFECT_ELEMENT_DESC* Element = Find_Element(Staged, m_strSelectedElementId);
	if (nullptr == Element) return false;
	Element->ResourceBindings.erase(std::remove_if(
		Element->ResourceBindings.begin(), Element->ResourceBindings.end(),
		[this](const EFFECT_RESOURCE_BINDING_DESC& Binding)
		{
			return Binding.eSlot == m_eSelectedResourceSlot;
		}), Element->ResourceBindings.end());
	std::string Error;
	if (!Validate_EffectDocument(Staged, Error)) return false;
	m_ActiveDocument = std::move(Staged);
	m_pPreview->Clear();
	m_strResourceStatus = "Slot cleared in memory.";
	return true;
}

bool_t Client::CEffect_Tool::Try_ApplyRenderProfile(
	const EFFECT_RENDER_PROFILE eProfile)
{
	using EffectToolDetail::Find_Element;
	EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
	EFFECT_ELEMENT_DESC* Element = Find_Element(Staged, m_strSelectedElementId);
	if (nullptr == Element || EFFECT_RENDER_PROFILE::END == eProfile) return false;
	Element->Material.eRenderProfile = eProfile;
	std::string Error;
	if (!Validate_EffectDocument(Staged, Error)) return false;
	m_ActiveDocument = std::move(Staged);
	return true;
}

bool_t Client::CEffect_Tool::Try_ApplyDetail(
	const EFFECT_DETAIL_DESC& Detail)
{
	using EffectToolDetail::Find_Element;
	EFFECT_DOCUMENT_DESC Staged = *m_ActiveDocument;
	EFFECT_ELEMENT_DESC* Element = Find_Element(Staged, m_strSelectedElementId);
	if (nullptr == Element) return false;
	Element->Detail = Detail;
	std::string Error;
	if (!Validate_EffectDocument(Staged, Error))
	{
		m_strDetailStatus = "Detail rejected; current values preserved: " + Error;
		return false;
	}
	m_ActiveDocument = std::move(Staged);
	m_strDetailStatus = "Detail committed in memory.";
	return true;
}

void Client::CEffect_Tool::Discard_ActiveDocument()
{
	m_ActiveDocument.reset();
	m_NewElementId.fill('\0');
	m_strSelectedElementId.clear();
	m_strSelectedResourceAssetId.clear();
	m_strElementStatus.clear();
	m_strResourceStatus.clear();
	m_strDetailStatus.clear();
	m_fPreviewTimeSeconds = 0.f;
	m_pPreview->Clear();
	m_strDocumentStatus = "Discarded memory Document; no file changed.";
}
```


### G06-12. `Client/Private/MainApp.cpp` 변경 함수 전체 코드

`MainApp.cpp`의 다른 함수는 바뀌지 않는다. `EnsureDebugTool()` 한 함수만 아래 전체
코드로 교체한다. Effect Tool 생성 시 MainApp이 이미 소유한 device/context를 넘기는
연결 한 줄이 본질이다.

```cpp
HRESULT CMainApp::EnsureDebugTool(const DEBUG_TOOL eTool)
{
	if (nullptr != m_pMapTool && DEBUG_TOOL::MAP != eTool)
		m_pMapTool->SetOpen(false);

	switch (eTool)
	{
	case DEBUG_TOOL::MAP:
		if (nullptr == m_pMapTool)
		{
			auto mapTool = make_unique<CMapTool>();
			if (FAILED(mapTool->Initialize(m_pDevice, m_pContext)))
				return E_FAIL;
			m_pMapTool = move(mapTool);
		}
		m_pMapTool->SetOpen(true);
		break;
	case DEBUG_TOOL::ANIMATION:
		if (nullptr == m_pAnimationTool)
			m_pAnimationTool = make_unique<CAnimation_Tool>();
		break;
	case DEBUG_TOOL::EFFECT:
		if (nullptr == m_pEffectTool)
			m_pEffectTool = make_unique<CEffect_Tool>(
				m_pDevice, m_pContext);
		break;
	case DEBUG_TOOL::UI:
		if (nullptr == m_pHUDLayoutTool)
			m_pHUDLayoutTool =
				make_unique<CHUDLayoutTool>(m_pDevice, m_pContext);
		break;
	default:
		return E_INVALIDARG;
	}

	m_eActiveDebugTool = eTool;
	return S_OK;
}
```

### G06-13. project/filter 전체 추가 항목

`Client/Default/Client.vcxproj`에서 기존 Effect 항목 옆에 다음 항목을 추가한다.

```xml
<ClInclude Include="..\Public\Effect_Preview.h" />
<ClCompile Include="..\Private\Effect_Preview.cpp" />
<FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshPreview.hlsl" />
<FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectRectPreview.hlsl" />
```

`Effect_AuthoringDocument.cpp`는 만들지 않으므로 project에도 등록하지 않는다.

`Client/Default/Client.vcxproj.filters`에 다음 블록을 추가한다.

```xml
<ClInclude Include="..\Public\Effect_Preview.h">
  <Filter>03. Tools\02. Effect</Filter>
</ClInclude>
<ClCompile Include="..\Private\Effect_Preview.cpp">
  <Filter>03. Tools\02. Effect</Filter>
</ClCompile>
<FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectMeshPreview.hlsl">
  <Filter>97.ShaderFiles</Filter>
</FxCompile>
<FxCompile Include="..\Bin\ShaderFiles\Shader_VtxEffectRectPreview.hlsl">
  <Filter>97.ShaderFiles</Filter>
</FxCompile>
```

### G06-14. ProjectAudit 추가 코드

기존 Effect audit block에서 새 파일을 읽는다.

```powershell
	$effectPreviewHeader = Get-Content -LiteralPath 'Client\Public\Effect_Preview.h' -Raw
	$effectPreviewSource = Get-Content -LiteralPath 'Client\Private\Effect_Preview.cpp' -Raw
	$effectMeshPreviewShader = Get-Content -LiteralPath 'Client\Bin\ShaderFiles\Shader_VtxEffectMeshPreview.hlsl' -Raw
	$effectRectPreviewShader = Get-Content -LiteralPath 'Client\Bin\ShaderFiles\Shader_VtxEffectRectPreview.hlsl' -Raw
	$mainAppSource = Get-Content -LiteralPath 'Client\Private\MainApp.cpp' -Raw
```

같은 block 끝에 G06 check를 추가한다.

```powershell
	$effectG6DetailPreviewShape =
		$effectDocumentHeader -match 'EFFECT_AUTHORING_FORMAT_VERSION\s*=\s*4u' -and
		$effectDocumentHeader -match 'EFFECT_AUTHORING_MIN_SUPPORTED_VERSION\s*=\s*3u' -and
		$effectDocumentHeader -match 'struct EFFECT_TRANSFORM_DESC' -and
		$effectDocumentHeader -match 'struct EFFECT_COLOR_DESC' -and
		$effectDocumentHeader -match 'struct EFFECT_UV_DESC' -and
		$effectDocumentHeader -match 'struct EFFECT_TIMING_DESC' -and
		$effectDocumentHeader -match 'struct EFFECT_DETAIL_DESC' -and
		$effectPreviewHeader -match 'class CEffectPreview' -and
		$effectPreviewSource -match 'CModel::Create' -and
		$effectPreviewSource -match 'Ensure_Resources' -and
		$effectPreviewSource -match 'Ensure_RenderTarget' -and
		$effectPreviewSource -match 'Select_Pass' -and
		$effectToolSource -match 'Validate_EffectDocument' -and
		$effectToolSource -match 'Try_ParseEffectDocumentJson' -and
		$effectToolSource -match 'Serialize_EffectDocumentJson' -and
		$effectToolSource -match 'Render_DetailPanel' -and
		$effectToolSource -match 'Render_PreviewPanel' -and
		$mainAppSource -match 'make_unique<CEffect_Tool>\(\s*m_pDevice,\s*m_pContext\s*\)' -and
		$effectMeshPreviewShader -match 'pass OpaqueBackDepthWrite[\s\S]*pass AlphaTwoSidedDepthRead[\s\S]*pass AdditiveTwoSidedDepthRead' -and
		$effectRectPreviewShader -match 'pass OpaqueBackDepthWrite[\s\S]*pass AlphaTwoSidedDepthRead[\s\S]*pass AdditiveTwoSidedDepthRead' -and
		$clientProjectSource -match 'Effect_Preview\.cpp' -and
		$clientProjectSource -notmatch 'Effect_AuthoringDocument\.cpp' -and
		$clientProjectSource -match 'Shader_VtxEffectMeshPreview\.hlsl' -and
		$clientProjectSource -match 'Shader_VtxEffectRectPreview\.hlsl'
	Add-Check 'effect.g6-detail-stateless-preview-boundary' `
		$effectG6DetailPreviewShape "detail=$effectG6DetailPreviewShape"
```

### G06-15. 사용자가 직접 작성할 순서

1. G05 build와 save/load가 끝난 상태에서 G06을 시작한다.
2. `Effect_AuthoringDocument.h`에 Detail struct를 추가하고 format을 v4로 올린다.
3. 기존 `Effect_Tool.cpp`의 validation, parse, serialize에 Detail을 연결한다.
4. v3 파일은 기본 Detail을 채운 v4 후보로 stage하고, 저장할 때 v4로 교체한다.
5. 두 Preview HLSL을 쓰고 Pass 순서를 0=Opaque, 1=Alpha, 2=Additive로 맞춘다.
6. `Effect_Preview.h/.cpp`를 쓰고 project/filter에 즉시 등록한다.
7. `Effect_Tool.h` 생성자와 Preview session 변수를 쓴다.
8. `MainApp::EnsureDebugTool()`에서 device/context를 전달한다.
9. `Effect_Tool.cpp`의 Detail panel과 Preview panel을 연결한다.
10. 마지막에 audit check를 추가하고 XML parse, build, audit를 순서대로 실행한다.

### G06-16. 자동 검증

소스 반영 전인 지금 PLAN 작성 단계에서는 아래 명령을 실행 완료로 기록하지 않는다.
구현 후 다음 순서로 실행한다.

```powershell
$project = [xml](Get-Content -LiteralPath 'Client/Default/Client.vcxproj' -Raw)
$filters = [xml](Get-Content -LiteralPath 'Client/Default/Client.vcxproj.filters' -Raw)
& 'Tools/Build/Invoke-BuildAndRegression.ps1' -Configuration Debug
& 'Tools/ProjectAudit/Invoke-ProjectAudit.ps1'
git diff --check
```

Effect 파일만 빠르게 확인한 뒤 정본 regression으로 넘어갈 때의 최소 선행 조건은
다음이다.

```text
Effect_AuthoringDocument.cpp가 Client project에 등록되지 않음
Effect_Preview.h/.cpp가 Effect filter에 등록됨
두 HLSL이 FxCompile과 ShaderFiles filter에 등록됨
G05 audit PASS
G06 audit PASS
Client x64 Debug compile/link PASS
```

### G06-17. F1 Runtime smoke에서 보여야 하는 결과

현재 첨부 화면은 정상 G04다. G06 반영 후에는 다음 차이가 보여야 한다.

```text
F1 -> Developer Tools -> Effect
상단 안내가 G6 문구로 바뀜
Document 생성/Load, Element 종류 선택은 그대로 유지
Element를 선택하면 왼쪽에 Material과 Effect Detail이 나타남
Material에는 Mesh Model, Base, Noise, Mask, Emissive, Dissolve slot이 나타남
Effect Detail에는 Transform, Color, UV, Timing 접이식 패널이 나타남
오른쪽에는 Sample Time slider와 정사각형 Preview가 나타남
Mesh + Mesh Model bind: CModel 모습 표시
Sprite + Base bind: texture quad 표시
Decal + Base bind: surface card 표시
Particle/Trail: "requires G08 modules" 표시, 가짜 Preview 없음
Start Delay보다 이른 Sample Time: 빈 Preview
잘못된 resource: 오류 문구, 직전 성공 Preview 보존
Save -> Discard -> Load: Detail 값과 resource/profile 동일
```

Client가 실행 직후 `abort() has been called`로 닫히면 위 smoke를 PASS로 기록하지 않는다.
그 경우 이 기능의 UI 문제가 아니라 별도 시작 경로 문제로 분리하고, Debug build와
ProjectAudit 결과만 사실대로 남긴다.

### G06-18. G07 이후 북극성 전환

G06 실행 화면과 `C:/Users/user/Desktop/툴`의 Winters Effect Tool 참고 화면을 다시 비교한 결과,
다음 단계의 우선 목표를 기능 목록 추가가 아니라 **참고 화면과 같은 저작 흐름 복원**으로 바꾼다.

참고 화면의 본질은 탭 세 개가 아니다. 다음 다섯 창을 동시에 보면서 하나의 결과를 편집하는
도킹 작업 공간이다.

```text
왼쪽       Effect Tool    타입 선택, Add Element, resource slot, thumbnail catalog
가운데     Model View     현재 월드, 합성 Effect, 이후 Character/Animation/Anchor
오른쪽     Effect Detail  선택한 Element 하나의 Transform/Color/UV/Timing
오른쪽 아래 All Effects  현재 Effect를 구성하는 Element stack
오른쪽 최하 Data Files   완성 Effect document의 이름, Save/Load 목록
```

`Effect Tool`, `Model View`, `Effect Detail`을 서로 가리는 상위 탭으로 만들지 않는다. 리소스를
누르면서 월드 결과와 Detail 수치를 동시에 봐야 하기 때문이다. 탭은 왼쪽 Resource 영역의
`Base / Noise / Mask / Emissive / Dissolve`에만 사용한다. 작은 화면을 위한 compact tab 배치는
선택 모드로만 남기고 기본 배치는 참고 화면을 따른다.

### G07-1. 현재 이름과 참고 화면 역할 매핑

```text
현재 Active Document
  -> Data Files로 이동
  -> Format Version은 일반 화면에서 숨기고 Diagnostics에만 표시

현재 Next Element Type + Add Element
  -> 왼쪽 Effect Tool 상단 유지
  -> UI에서는 SPRITE를 참고 화면처럼 Texture로 표시하되 저장 token은 sprite 유지

현재 Current Elements
  -> 오른쪽 아래 All Effects
  -> 한 행이 합성 Effect를 구성하는 한 Element layer
  -> Select / Delete / Clear All / Time Reset All 제공

현재 Material Inputs and Render Profile
  -> 왼쪽 Effect Tool의 resource slot과 thumbnail catalog
  -> 문자열 목록을 주 UI로 사용하지 않고 search/category 보조 정보로만 사용

현재 Effect Detail
  -> 독립 오른쪽 창
  -> 선택 Element만 편집

현재 360x360 Preview
  -> 독립 주 화면에서 제거
  -> 가운데 실제 월드 Model View가 주 Preview
  -> 기존 off-screen Preview는 thumbnail 생성과 고립 진단용으로 재사용
```

### G07-2. 저장 데이터의 본질

```text
EFFECT_DOCUMENT_DESC 하나
  = 저장되는 완성 Effect asset 하나

Document.Elements
  = 그 Effect를 동시에 구성하는 Mesh/Texture/Particle/Decal/Trail layer 목록

Add Element
  = 새 완성 Effect를 만드는 명령이 아니라 현재 Effect에 layer 하나를 추가하는 명령

All Effects
  = 현재 Document.Elements를 저작자가 이해하기 쉬운 stack으로 보여주는 창

Data Files Save
  = stack 전체를 하나의 <EffectAssetId>.effect.json으로 저장
```

따라서 `Add Element`와 `Current Elements` 기능은 삭제하지 않는다. `Current Elements`라는 개발자
표현과 상단의 큰 Document 블록만 없애고, 참고 화면의 `All Effects`와 `Data Files` 역할로 옮긴다.

### G07-3. Thumbnail catalog

```text
DDS
  -> asset ID를 key로 SRV를 lazy load
  -> ImGui::ImageButton으로 실제 thumbnail 표시

WMODEL
  -> catalog에서는 이름/placeholder card를 먼저 표시
  -> 선택한 model은 가운데 Model View에서 실제 CModel로 확인
  -> 필요할 때만 off-screen render 결과를 thumbnail cache에 저장

cache
  -> 매 frame 파일 scan/load 금지
  -> Refresh Resources에서 revision 교체
  -> 화면 밖 thumbnail은 만들지 않음
```

위쪽 다섯 slot card에는 현재 선택 Element의 `Base / Noise / Mask / Emissive / Dissolve` 연결 결과를
항상 표시한다. 아래 thumbnail grid에서 누른 resource는 현재 활성 slot에 stage/validate/commit한다.

### G07-4. 실제 월드 합성 Preview와 Timeline

가운데 Model View는 선택 Element 하나가 아니라 Document의 모든 Element를 같은 Effect root 아래
동시에 평가한다.

```text
Document.Elements 전체
-> 같은 Sample Time으로 평가
-> Element local Transform 적용
-> world preview root 적용
-> 현재 Development world에 합성 결과 표시
```
Timeline의 `Play / Stop / Loop / Time Reset All`은 Tool session 상태다. Sample Time과 재생 상태는
Effect JSON에 저장하지 않는다. Curve/key 데이터가 실제로 추가될 때만 별도 저장 계약을 연다.

### G07-5. Model View Character/Animation/Anchor 연결

Character 선택과 preview body 생성은 새 경로를 만들지 않고 기존 `CCharacterPreviewPanel`을 재사용한다.
Effect Tool은 level/layer/part/vector index로 Character를 찾지 않고 `CAnimationTargetService`만 읽는다.

```text
CCharacterPreviewPanel::Render_Selector
-> CAnimationTargetService::Resolve_Model
-> 실제 model clip 목록에서 Animation 선택/재생
-> Resolve_RootTransform 또는 Resolve_AnchorTransform(anchorSlotId)
-> Element local matrix * 선택 anchor world matrix
-> animation을 따라 Effect preview 이동
```

Character class, 선택 clip, playback frame, preview anchor는 Tool session 값이다. Effect asset은 캐릭터나
animation 이름을 저장하지 않고 effect-local 모양과 수명만 소유한다. 제품 Animation cue가 나중에
`EffectAssetId + attachment slot`을 소유한다.

### G07-6. 구현 순서와 종료 증거

```text
G07-A  창 재배치
       Effect Tool / Model View / Effect Detail / All Effects / Data Files 동시 표시
       기존 v4 JSON 변경 없음

G07-B  thumbnail resource browser
       DDS thumbnail 클릭 -> 활성 slot commit -> slot card와 Preview 즉시 갱신

G07-C  composite world Preview와 Timeline
       Document의 Mesh/Texture/Decal layer가 실제 월드에서 함께 재생

G07-D  Character/Animation/Anchor
       Preview character 선택 -> clip 재생 -> root/weapon/bone anchor를 따라 Effect 이동

G08    Particle/Trail/AfterImage와 Distortion/HDR/Bloom 실제 소비

G09    EffectAssetId admission과 Animation cue/runtime 연결
       이후 DimensionMaster 모든 skill의 composite Effect를 저장하고 skill/animation에 연결
```

G07은 UI만 비슷하게 보이는 것으로 끝내지 않는다. 최종 종료 증거는 thumbnail 선택, layer 합성,
Save/Load, world playback, Character animation anchor가 모두 같은 Document와 Preview evaluator를
소비하는 것이다. G07용 enum, key struct, timeline session 변수는 각 세부 단계의 실제 소비자가
연결될 때만 추가한다.

### G07~G09 상세 계획 정본

- [G07 레퍼런스 워크스페이스와 월드 Preview](2026-08-05_EFFECT_TOOL_G07_REFERENCE_WORKSPACE_WORLD_PREVIEW_PLAN.md)
- [G08 Particle·Trail·AfterImage와 실제 렌더링](2026-08-05_EFFECT_TOOL_G08_PARTICLE_TRAIL_RENDERING_PLAN.md)
- [G09 Effect admission·runtime·DimensionMaster 스킬](2026-08-05_EFFECT_TOOL_G09_RUNTIME_DIMENSIONMASTER_PLAN.md)
