# Effect Tool v2 G01 — Effect Type 선택 · Mesh/Base/Noise/Mask/Emissive/Dissolve 슬롯 · 리소스 선택 · 미리보기

작성 2026-08-22. 브랜치 `feature/effect-tool-v2`. 기존 `CEffect_Tool`(팀장 소유)과 완전히 분리된
새 F1 툴이다. UE3 Cascade를 참고하는 툴의 첫 슬라이스로, 저장/문서/런타임 재생은 아직 없다.

## G01 범위 (사용자 확인 완료 2026-08-22)

- 슬롯별 라이브러리 필터: `Data/Effects/V2/TextureSlotUsage.v1.json`(`Tools/EffectToolV2/build_texture_slot_usage.py`가
  Imported/Authored 392개 문서에서 생성, 텍스처별 base/noise/mask/emissive/dissolve 사용 횟수 + 원본 파라미터명)을
  로드해 선택 슬롯으로 쓰인 적 있는 텍스처만 횟수순 표시. 여러 슬롯에 쓰인 텍스처는 각 슬롯에 모두 노출.
  `Show all`로 전체. 타일 뱃지 `B.. N.. M.. E.. D..`, 툴팁에 원본 파라미터명.

- F1 Developer Tools `Effect Tool v2` 버튼 → `Effect Tool v2` 창.
- Effect Type 라디오 `Mesh / Texture / Particle / Decal / Trail`.
- 슬롯 카드 `Mesh`(Mesh/Particle 타입에서만) + `Base / Noise / Mask / Emissive / Dissolve`. Type별 바인딩 독립.
- Resource Library: `Resources/Effect/**` DDS(텍스처 슬롯) / WModel(Mesh 슬롯) 스캔. 도메인은
  `Meshes/Textures/Models/Animations/VectorFields` 폴더 앞까지의 경로(`Esther/Wei` 등으로 세분화).
- 카드 클릭 → 슬롯 선택, 라이브러리 썸네일 클릭 → 바인드, 카드와 Preview 패널에 표시. `Clear`로 해제.
- DDS 미리보기: `DirectXTK CreateDDSTextureFromFile`(최대 512px), 프레임당 2개.
- WModel 썸네일: Engine `CModel`을 `NONANIM`→실패 시 `ANIM`으로 로드, 128px RT에 렌더.
  스태틱은 `Shader_VtxMeshPreview.hlsl`, 스킨은 v2 소유 `Shader_VtxAnimMeshPreview_V2.hlsl` +
  `Bind_BoneMatrices`. 스킨 모델은 Engine이 local bounds를 계산하지 않으므로(`Model.cpp:914`)
  `CModelDecoderRegistry`로 다시 디코드해 `정점 × inverseBind × combined`로 bounds를 구한다
  (원시 정점 bounds는 본 오프셋 공간이라 카메라가 빗나갔던 문제의 수정).
- Preview 패널에 모델 진단(skinned/static, 메시·본·애니·머티리얼 수, bounds, 메시별 diffuse 파일명) 표시.


## G03 범위 — Create Effect 월드 preview + 실시간 튜닝 (사용자 확인 완료 2026-08-22)

- `CEffectPreviewV2`(`Effect_Preview_V2.h/.cpp`, `CGameObject` 파생): `Prototype_GameObject_EffectPreviewV2`를
  `LEVEL::STATIC`에 1회 등록하고 현재 레벨의 `Layer_EffectPreviewV2`에 Clone. 레벨 전환 시 자동 정리(툴은 `weak_ptr`).
- Shape: Mesh 타입 → WModel(NONANIM, 실패 시 ANIM + `Bind_BoneMatrices`), Texture 타입 → `CVIBuffer_Rect` 빌보드.
- v2 소유 셰이더: `Shader_EffectV2_Common.hlsli`(공용 PS: Base(UV scale/pan) + Noise 왜곡 × Mask.r × Dissolve
  smoothstep × Tint + Emissive×Intensity, 패스 4개 = Alpha/Additive × DepthTest on/off),
  `Shader_EffectMeshV2.hlsl`, `Shader_EffectAnimMeshV2.hlsl`, `Shader_EffectRectV2.hlsl`. BLEND 그룹 제출.
- 툴: `Create Effect`(Base 필수, Mesh/Particle 타입은 Mesh 필수; Particle/Decal/Trail은 비활성) → 카메라 전방 3 m.
  Tuning: Restart/Visible/Bring To Camera, Position/Rotation/Scale, Tint/Blend/DepthTest/Billboard, UV Scale·Base Pan,
  Noise Strength/Scale/Pan, Emissive Intensity, Dissolve Amount/Softness, Lifetime/Loop/Play Rate. 실패 사유는
  `CEffectPreviewV2::Last_Error()`로 상태줄에 노출.
- 카메라 없는 Lobby에서는 안 보임(Character Select/Bern/Valtan/Test에서 사용).
- AGENTS '단일 Effect 런타임 경로' 규칙 대비: 이 경로는 Debug 툴 preview 전용이며 제품 재생 경로가 아니다.

## 파일

| 파일 | 역할 |
|---|---|
| `Client/Public/Effect_Tool_V2.h` (새) | `CEffect_Tool_V2` 선언 |
| `Client/Private/Effect_Tool_V2.cpp` (새) | 스캔/미리보기/썸네일/ImGui |
| `Client/Bin/ShaderFiles/Shader_VtxAnimMeshPreview_V2.hlsl` (새) | 스킨 메시 썸네일 셰이더 |
| `Client/Public/Effect_Preview_V2.h`, `Client/Private/Effect_Preview_V2.cpp` (새) | 월드 preview GameObject |
| `Client/Bin/ShaderFiles/Shader_EffectV2_Common.hlsli`, `Shader_EffectMeshV2.hlsl`, `Shader_EffectAnimMeshV2.hlsl`, `Shader_EffectRectV2.hlsl` (새) | preview 셰이더 |
| `Tools/EffectToolV2/build_texture_slot_usage.py` (새) | 슬롯 사용 사이드카 생성기 |
| `Data/Effects/V2/TextureSlotUsage.v1.json` (생성물, Git 추적) | 텍스처→슬롯 사용 횟수 |
| `Client/Public/MainApp.h`, `Client/Private/MainApp.cpp` | `DEBUG_TOOL::EFFECT_V2`, 멤버, 버튼/생성/Render/Free |
| `Client/Default/Client.vcxproj`, `.filters` | 등록, 필터 `03. Tools\06. Effect V2` |

의존: `imgui.h`, `RuntimeAssetRoot.h`, `Model.h`, `Shader.h`, `BinaryAsset/ModelDecoderRegistry.h`,
`DirectXTK/DDSTextureLoader.h`. 기존 Effect 코드(`Effect_AuthoringDocument.h`, `CEffectThumbnailCache`,
`CEffectCatalog`)는 include하지 않는다.

## 검증

- Client Debug x64 빌드 성공, `Effect_Tool_V2.cpp` 경고 0, `git diff --check` 통과.
- 사용자 확인(2026-08-22): Type/슬롯/라이브러리/미리보기 동작, `Esther/Balthorr·Thirain·Wei` 도메인 분리,
  `Esther/Wei/Models/Dochul/FX_CUDC_00_SK.wmodel` 스킨 썸네일 표시.

## 전체 코드

### Client/Public/Effect_Tool_V2.h

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

NS_BEGIN(Engine)
class CModel;
class CShader;
NS_END

NS_BEGIN(Client)

class CEffectPreviewV2;

class CEffect_Tool_V2 final
{
public:
	enum class EFFECT_TYPE : int32_t
	{
		MESH,
		TEXTURE,
		PARTICLE,
		DECAL,
		TRAIL,
		END
	};

	enum class RESOURCE_SLOT : int32_t
	{
		MESH,
		BASE,
		NOISE,
		MASK,
		EMISSIVE,
		DISSOLVE,
		END
	};

	enum class RESOURCE_KIND : int32_t
	{
		TEXTURE,
		MODEL,
		END
	};

private:
	struct TEXTURE_USAGE final
	{
		std::array<uint32_t, static_cast<size_t>(RESOURCE_SLOT::END)> Counts{};
		std::vector<std::string> Params;
	};

	struct RESOURCE_ENTRY final
	{
		std::string strAssetId;
		std::string strDomain;
		std::string strFileName;
		RESOURCE_KIND eKind = RESOURCE_KIND::TEXTURE;
		const TEXTURE_USAGE* pUsage = nullptr;
	};

	struct PREVIEW_ENTRY final
	{
		ComPtr<ID3D11ShaderResourceView> pTextureView;
		std::string strError;
		std::string strInfo;
		uint32_t iWidth = 0u;
		uint32_t iHeight = 0u;
	};

	using SLOT_BINDINGS =
		std::array<std::string, static_cast<size_t>(RESOURCE_SLOT::END)>;

public:
	CEffect_Tool_V2(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	~CEffect_Tool_V2();

	void Render();

private:
	void Scan_Resources();
	void Load_TextureUsage();
	void Rebuild_VisibleResources();
	const PREVIEW_ENTRY* Request_Preview(
		const std::string& strAssetId, RESOURCE_KIND eKind);
	bool_t Create_ModelThumbnail(
		const std::filesystem::path& Path,
		ComPtr<ID3D11ShaderResourceView>& OutTextureView,
		std::string& strOutError,
		std::string& strOutInfo);
	static bool_t Compute_SkinnedBounds(
		const std::filesystem::path& Path,
		const Engine::CModel& Model,
		float3_t& OutMinimum,
		float3_t& OutMaximum,
		std::string& strOutInfo);

	void Render_TypeSelector();
	void Render_SlotCards();
	void Render_ResourceBrowser();
	void Render_PreviewPanel();
	void Render_CreatePanel();
	void Render_TuningPanel();
	bool_t Try_CreatePreview();

	SLOT_BINDINGS& Current_Bindings();
	std::string& Current_SlotAssetId();
	bool_t Slot_VisibleForType(RESOURCE_SLOT eSlot) const;

	static RESOURCE_KIND Slot_Kind(RESOURCE_SLOT eSlot);
	static std::string Domain_FromRelativePath(
		const std::filesystem::path& EffectRelative);
	static const char* Type_Label(EFFECT_TYPE eType);
	static const char* Slot_Label(RESOURCE_SLOT eSlot);
	static const char* Slot_Description(RESOURCE_SLOT eSlot);

private:
	ComPtr<ID3D11Device> m_pDevice;
	ComPtr<ID3D11DeviceContext> m_pContext;
	shared_ptr<Engine::CShader> m_pModelShader;
	shared_ptr<Engine::CShader> m_pAnimModelShader;

	EFFECT_TYPE m_eType = EFFECT_TYPE::MESH;
	RESOURCE_SLOT m_eSelectedSlot = RESOURCE_SLOT::BASE;
	std::array<SLOT_BINDINGS, static_cast<size_t>(EFFECT_TYPE::END)> m_Bindings;

	bool_t m_bScanned = false;
	std::vector<RESOURCE_ENTRY> m_Resources;
	std::vector<std::string> m_Domains;
	std::vector<size_t> m_VisibleResources;
	RESOURCE_KIND m_eVisibleKind = RESOURCE_KIND::END;
	std::string m_strDomainFilter;
	char m_szNameFilter[128] = {};
	bool_t m_bVisibleDirty = true;
	bool_t m_bShowAllTextures = false;
	std::unordered_map<std::string, TEXTURE_USAGE> m_TextureUsage;
	std::string m_strUsageStatus;

	std::unordered_map<std::string, PREVIEW_ENTRY> m_Previews;
	uint32_t m_iLoadsThisFrame = 0u;

	std::weak_ptr<CEffectPreviewV2> m_pPreview;
	bool_t m_bPreviewPrototypeRegistered = false;
	std::string m_strPreviewStatus;

	std::string m_strStatus;
};

NS_END
```

### Client/Private/Effect_Tool_V2.cpp

```cpp
#include "imgui.h"

#include "Effect_Tool_V2.h"
#include "BinaryAsset/ModelAssetData.h"
#include "BinaryAsset/ModelDecoderRegistry.h"
#include "DataJson.h"
#include "Effect_Preview_V2.h"
#include "GameInstance.h"
#include "Model.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"
#include "Shader.h"

#include "DirectXTK/DDSTextureLoader.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <limits>
#include <set>

namespace
{
	constexpr uint32_t MAX_PREVIEW_LOADS_PER_FRAME = 2u;
	constexpr size_t MAX_PREVIEW_DIMENSION = 512u;
	constexpr uint32_t MODEL_THUMBNAIL_SIZE = 128u;
	constexpr float SLOT_CARD_SIZE = 96.f;
	constexpr float BROWSER_TILE_SIZE = 80.f;
	constexpr float PREVIEW_PANEL_SIZE = 256.f;

	constexpr const char* ASSET_KIND_FOLDERS[] = {
		"meshes", "textures", "models", "animations", "vectorfields"
	};

	std::string To_Lower(std::string Value)
	{
		std::transform(Value.begin(), Value.end(), Value.begin(),
			[](const char Character)
			{
				return static_cast<char>(std::tolower(
					static_cast<unsigned char>(Character)));
			});
		return Value;
	}

	bool_t Is_AssetKindFolder(const std::string& strName)
	{
		const std::string Lower = To_Lower(strName);
		for (const char* pFolder : ASSET_KIND_FOLDERS)
		{
			if (Lower == pFolder)
				return true;
		}
		return false;
	}
}

Client::CEffect_Tool_V2::CEffect_Tool_V2(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice(std::move(pDevice)),
	  m_pContext(std::move(pContext))
{
}

Client::CEffect_Tool_V2::~CEffect_Tool_V2() = default;

void Client::CEffect_Tool_V2::Render()
{
	m_iLoadsThisFrame = 0u;
	if (!m_bScanned)
		Scan_Resources();
	if (!Slot_VisibleForType(m_eSelectedSlot))
		m_eSelectedSlot = RESOURCE_SLOT::BASE;

	if (!ImGui::Begin("Effect Tool v2"))
	{
		ImGui::End();
		return;
	}

	Render_TypeSelector();
	ImGui::Separator();
	Render_SlotCards();
	ImGui::Separator();

	if (ImGui::BeginTable("EffectToolV2Body", 2,
		ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV))
	{
		ImGui::TableSetupColumn("Resources",
			ImGuiTableColumnFlags_WidthStretch, 0.62f);
		ImGui::TableSetupColumn("Preview",
			ImGuiTableColumnFlags_WidthStretch, 0.38f);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		Render_ResourceBrowser();
		ImGui::TableSetColumnIndex(1);
		Render_PreviewPanel();
		ImGui::Separator();
		Render_CreatePanel();
		Render_TuningPanel();
		ImGui::EndTable();
	}

	if (!m_strStatus.empty())
		ImGui::TextWrapped("%s", m_strStatus.c_str());
	ImGui::End();
}

void Client::CEffect_Tool_V2::Scan_Resources()
{
	m_bScanned = true;
	m_Resources.clear();
	m_Domains.clear();
	m_bVisibleDirty = true;

	std::error_code Error;
	const std::filesystem::path Root = CRuntimeAssetRoot::Get();
	const std::filesystem::path EffectRoot = Root / "Effect";
	if (!std::filesystem::is_directory(EffectRoot, Error) || Error)
	{
		m_strStatus = "Resources/Effect is missing: " + EffectRoot.string();
		return;
	}

	std::set<std::string> Domains;
	size_t iTextures = 0u;
	size_t iModels = 0u;
	for (std::filesystem::recursive_directory_iterator Iterator(
		EffectRoot,
		std::filesystem::directory_options::skip_permission_denied,
		Error), End; Iterator != End; Iterator.increment(Error))
	{
		if (Error)
		{
			Error.clear();
			continue;
		}
		if (!Iterator->is_regular_file())
			continue;
		const std::string Extension =
			To_Lower(Iterator->path().extension().string());
		RESOURCE_KIND eKind = RESOURCE_KIND::END;
		if (".dds" == Extension)
			eKind = RESOURCE_KIND::TEXTURE;
		else if (".wmodel" == Extension)
			eKind = RESOURCE_KIND::MODEL;
		else
			continue;
		const std::filesystem::path EffectRelative =
			Iterator->path().lexically_relative(EffectRoot);
		if (EffectRelative.empty() || !EffectRelative.has_parent_path())
			continue;
		const std::string Domain = Domain_FromRelativePath(EffectRelative);
		if (Domain.empty())
			continue;
		Domains.insert(Domain);
		(RESOURCE_KIND::TEXTURE == eKind ? iTextures : iModels)++;
		m_Resources.push_back({
			Iterator->path().lexically_relative(Root).generic_string(),
			Domain,
			Iterator->path().filename().string(),
			eKind });
	}
	std::sort(m_Resources.begin(), m_Resources.end(),
		[](const RESOURCE_ENTRY& Left, const RESOURCE_ENTRY& Right)
		{
			return Left.strAssetId < Right.strAssetId;
		});
	m_Domains.assign(Domains.begin(), Domains.end());
	Load_TextureUsage();
	size_t iWithUsage = 0u;
	for (RESOURCE_ENTRY& Entry : m_Resources)
	{
		if (RESOURCE_KIND::TEXTURE != Entry.eKind)
			continue;
		const auto Iterator = m_TextureUsage.find(To_Lower(Entry.strFileName));
		Entry.pUsage = m_TextureUsage.end() == Iterator ? nullptr : &Iterator->second;
		iWithUsage += nullptr != Entry.pUsage ? 1u : 0u;
	}
	m_strStatus = "Scanned " + std::to_string(iTextures) + " DDS (" +
		std::to_string(iWithUsage) + " with slot usage), " +
		std::to_string(iModels) + " WModel in " +
		std::to_string(m_Domains.size()) + " domains. " + m_strUsageStatus;
}

void Client::CEffect_Tool_V2::Load_TextureUsage()
{
	m_TextureUsage.clear();
	const std::filesystem::path Path =
		CProjectDataRoot::Resolve(L"Effects/V2/TextureSlotUsage.v1.json");
	std::ifstream File(Path, std::ios::binary);
	if (!File)
	{
		m_strUsageStatus = "TextureSlotUsage.v1.json not found.";
		return;
	}
	const std::string Text{
		std::istreambuf_iterator<char>(File), std::istreambuf_iterator<char>() };
	DATA_JSON_VALUE Root;
	std::string Error;
	if (!CDataJson::Parse(Text, Root, Error) || !Root.Is_Object())
	{
		m_strUsageStatus = "TextureSlotUsage parse failed: " + Error;
		return;
	}
	const DATA_JSON_VALUE* pVersion = Root.Find("formatVersion");
	const DATA_JSON_VALUE* pTextures = Root.Find("textures");
	if (nullptr == pVersion || !pVersion->Is_Number() ||
		1 != static_cast<int32_t>(pVersion->Get_Number()) ||
		nullptr == pTextures || !pTextures->Is_Object())
	{
		m_strUsageStatus = "TextureSlotUsage has an unsupported format.";
		return;
	}
	for (const std::string& Name : pTextures->Get_ObjectInsertionOrder())
	{
		const DATA_JSON_VALUE* pEntry = pTextures->Find(Name);
		if (nullptr == pEntry || !pEntry->Is_Object())
			continue;
		TEXTURE_USAGE Usage;
		for (int32_t iSlot = static_cast<int32_t>(RESOURCE_SLOT::BASE);
			iSlot < static_cast<int32_t>(RESOURCE_SLOT::END); ++iSlot)
		{
			const DATA_JSON_VALUE* pCount =
				pEntry->Find(To_Lower(Slot_Label(static_cast<RESOURCE_SLOT>(iSlot))));
			if (nullptr != pCount && pCount->Is_Number())
				Usage.Counts[static_cast<size_t>(iSlot)] =
					static_cast<uint32_t>((std::max)(0.0, pCount->Get_Number()));
		}
		if (const DATA_JSON_VALUE* pParams = pEntry->Find("params");
			nullptr != pParams && pParams->Is_Array())
		{
			for (const DATA_JSON_VALUE& Param : pParams->Get_Array())
			{
				if (Param.Is_String())
					Usage.Params.push_back(Param.Get_String());
			}
		}
		m_TextureUsage.emplace(To_Lower(Name), std::move(Usage));
	}
	m_strUsageStatus = "Slot usage: " + std::to_string(m_TextureUsage.size()) +
		" textures.";
}

std::string Client::CEffect_Tool_V2::Domain_FromRelativePath(
	const std::filesystem::path& EffectRelative)
{
	std::string Domain;
	const std::filesystem::path Parent = EffectRelative.parent_path();
	for (const std::filesystem::path& Component : Parent)
	{
		const std::string Name = Component.string();
		if (Is_AssetKindFolder(Name))
			break;
		if (!Domain.empty())
			Domain += '/';
		Domain += Name;
	}
	if (Domain.empty() && Parent.begin() != Parent.end())
		Domain = Parent.begin()->string();
	return Domain;
}

void Client::CEffect_Tool_V2::Rebuild_VisibleResources()
{
	m_bVisibleDirty = false;
	m_VisibleResources.clear();
	const RESOURCE_KIND eKind = Slot_Kind(m_eSelectedSlot);
	m_eVisibleKind = eKind;
	const std::string Filter = To_Lower(m_szNameFilter);
	const size_t iSlot = static_cast<size_t>(m_eSelectedSlot);
	const bool_t bUsageFilter =
		RESOURCE_KIND::TEXTURE == eKind && !m_bShowAllTextures;
	for (size_t iEntry = 0u; iEntry < m_Resources.size(); ++iEntry)
	{
		const RESOURCE_ENTRY& Entry = m_Resources[iEntry];
		if (Entry.eKind != eKind)
			continue;
		if (bUsageFilter &&
			(nullptr == Entry.pUsage || 0u == Entry.pUsage->Counts[iSlot]))
			continue;
		if (!m_strDomainFilter.empty() && Entry.strDomain != m_strDomainFilter)
			continue;
		if (!Filter.empty() &&
			std::string::npos == To_Lower(Entry.strFileName).find(Filter))
			continue;
		m_VisibleResources.push_back(iEntry);
	}
	if (RESOURCE_KIND::TEXTURE == eKind)
	{
		std::stable_sort(m_VisibleResources.begin(), m_VisibleResources.end(),
			[this, iSlot](const size_t iLeft, const size_t iRight)
			{
				const TEXTURE_USAGE* pLeft = m_Resources[iLeft].pUsage;
				const TEXTURE_USAGE* pRight = m_Resources[iRight].pUsage;
				const uint32_t iLeftCount = nullptr == pLeft ? 0u : pLeft->Counts[iSlot];
				const uint32_t iRightCount = nullptr == pRight ? 0u : pRight->Counts[iSlot];
				return iLeftCount > iRightCount;
			});
	}
}

const Client::CEffect_Tool_V2::PREVIEW_ENTRY*
Client::CEffect_Tool_V2::Request_Preview(
	const std::string& strAssetId, const RESOURCE_KIND eKind)
{
	if (strAssetId.empty())
		return nullptr;
	auto Iterator = m_Previews.find(strAssetId);
	if (Iterator != m_Previews.end())
		return &Iterator->second;
	if (m_iLoadsThisFrame >= MAX_PREVIEW_LOADS_PER_FRAME)
		return nullptr;
	m_iLoadsThisFrame += RESOURCE_KIND::MODEL == eKind ?
		MAX_PREVIEW_LOADS_PER_FRAME : 1u;

	PREVIEW_ENTRY Staged;
	const std::filesystem::path Path =
		CRuntimeAssetRoot::Resolve(std::filesystem::path(strAssetId));
	std::error_code Error;
	if (Path.empty() || !std::filesystem::is_regular_file(Path, Error))
	{
		Staged.strError = "Missing: " + strAssetId;
	}
	else if (RESOURCE_KIND::MODEL == eKind)
	{
		if (Create_ModelThumbnail(Path, Staged.pTextureView, Staged.strError,
			Staged.strInfo))
		{
			Staged.iWidth = MODEL_THUMBNAIL_SIZE;
			Staged.iHeight = MODEL_THUMBNAIL_SIZE;
		}
	}
	else
	{
		ComPtr<ID3D11Resource> pResource;
		if (FAILED(DirectX::CreateDDSTextureFromFile(
			m_pDevice.Get(), Path.c_str(), &pResource,
			&Staged.pTextureView, MAX_PREVIEW_DIMENSION)))
		{
			Staged.strError = "DDS load failed: " + strAssetId;
		}
		else
		{
			ComPtr<ID3D11Texture2D> pTexture;
			if (SUCCEEDED(pResource.As(&pTexture)))
			{
				D3D11_TEXTURE2D_DESC Desc{};
				pTexture->GetDesc(&Desc);
				Staged.iWidth = Desc.Width;
				Staged.iHeight = Desc.Height;
			}
		}
	}
	return &m_Previews.emplace(strAssetId, std::move(Staged)).first->second;
}

bool_t Client::CEffect_Tool_V2::Create_ModelThumbnail(
	const std::filesystem::path& Path,
	ComPtr<ID3D11ShaderResourceView>& OutTextureView,
	std::string& strOutError,
	std::string& strOutInfo)
{
	unique_ptr<Engine::CModel> Model = Engine::CModel::Create(
		m_pDevice, m_pContext, MODEL::NONANIM,
		Path.string().c_str(), XMMatrixIdentity());
	bool_t bSkinned = false;
	if (nullptr == Model)
	{
		Model = Engine::CModel::Create(
			m_pDevice, m_pContext, MODEL::ANIM,
			Path.string().c_str(), XMMatrixIdentity());
		bSkinned = nullptr != Model;
	}
	if (nullptr == Model)
	{
		strOutError = "WModel load failed: " + Path.string();
		return false;
	}
	float3_t Minimum{};
	float3_t Maximum{};
	if (Model->Has_LocalBounds())
	{
		Minimum = Model->Get_LocalBoundsMin();
		Maximum = Model->Get_LocalBoundsMax();
	}
	else if (!Compute_SkinnedBounds(Path, *Model, Minimum, Maximum, strOutInfo))
	{
		strOutError = "WModel bounds failed: " + Path.string();
		return false;
	}
	{
		char szBounds[192] = {};
		std::snprintf(szBounds, sizeof(szBounds),
			"%s | meshes=%u | min(%.2f %.2f %.2f) max(%.2f %.2f %.2f)",
			bSkinned ? "skinned" : "static", Model->Get_NumMeshes(),
			Minimum.x, Minimum.y, Minimum.z, Maximum.x, Maximum.y, Maximum.z);
		strOutInfo = strOutInfo.empty() ? szBounds : szBounds + ("\n" + strOutInfo);
	}

	shared_ptr<Engine::CShader>& pShader =
		bSkinned ? m_pAnimModelShader : m_pModelShader;
	if (nullptr == pShader)
	{
		unique_ptr<Engine::CShader> Shader = bSkinned ?
			Engine::CShader::Create(m_pDevice, m_pContext,
				TEXT("../Bin/ShaderFiles/Shader_VtxAnimMeshPreview_V2.hlsl"),
				VTXANIMMESH::Elements, VTXANIMMESH::iNumElements) :
			Engine::CShader::Create(m_pDevice, m_pContext,
				TEXT("../Bin/ShaderFiles/Shader_VtxMeshPreview.hlsl"),
				VTXMESH::Elements, VTXMESH::iNumElements);
		if (nullptr == Shader)
		{
			strOutError = bSkinned ?
				"Skinned mesh preview shader creation failed." :
				"Mesh preview shader creation failed.";
			return false;
		}
		pShader = std::move(Shader);
	}

	D3D11_TEXTURE2D_DESC ColorDesc{};
	ColorDesc.Width = MODEL_THUMBNAIL_SIZE;
	ColorDesc.Height = MODEL_THUMBNAIL_SIZE;
	ColorDesc.MipLevels = 1u;
	ColorDesc.ArraySize = 1u;
	ColorDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	ColorDesc.SampleDesc.Count = 1u;
	ColorDesc.Usage = D3D11_USAGE_DEFAULT;
	ColorDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	ComPtr<ID3D11Texture2D> ColorTexture;
	ComPtr<ID3D11RenderTargetView> ColorRTV;
	ComPtr<ID3D11ShaderResourceView> ColorSRV;
	if (FAILED(m_pDevice->CreateTexture2D(&ColorDesc, nullptr, &ColorTexture)) ||
		FAILED(m_pDevice->CreateRenderTargetView(
			ColorTexture.Get(), nullptr, &ColorRTV)) ||
		FAILED(m_pDevice->CreateShaderResourceView(
			ColorTexture.Get(), nullptr, &ColorSRV)))
	{
		strOutError = "Mesh preview color target creation failed.";
		return false;
	}

	D3D11_TEXTURE2D_DESC DepthDesc = ColorDesc;
	DepthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DepthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	ComPtr<ID3D11Texture2D> DepthTexture;
	ComPtr<ID3D11DepthStencilView> DepthDSV;
	if (FAILED(m_pDevice->CreateTexture2D(&DepthDesc, nullptr, &DepthTexture)) ||
		FAILED(m_pDevice->CreateDepthStencilView(
			DepthTexture.Get(), nullptr, &DepthDSV)))
	{
		strOutError = "Mesh preview depth target creation failed.";
		return false;
	}

	const uint32_t WhitePixel = 0xffffffffu;
	D3D11_TEXTURE2D_DESC WhiteDesc{};
	WhiteDesc.Width = 1u;
	WhiteDesc.Height = 1u;
	WhiteDesc.MipLevels = 1u;
	WhiteDesc.ArraySize = 1u;
	WhiteDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	WhiteDesc.SampleDesc.Count = 1u;
	WhiteDesc.Usage = D3D11_USAGE_IMMUTABLE;
	WhiteDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	D3D11_SUBRESOURCE_DATA WhiteData{};
	WhiteData.pSysMem = &WhitePixel;
	WhiteData.SysMemPitch = sizeof(WhitePixel);
	ComPtr<ID3D11Texture2D> WhiteTexture;
	ComPtr<ID3D11ShaderResourceView> WhiteSRV;
	if (FAILED(m_pDevice->CreateTexture2D(&WhiteDesc, &WhiteData, &WhiteTexture)) ||
		FAILED(m_pDevice->CreateShaderResourceView(
			WhiteTexture.Get(), nullptr, &WhiteSRV)))
	{
		strOutError = "Mesh preview fallback texture creation failed.";
		return false;
	}

	const float3_t Center(
		(Minimum.x + Maximum.x) * 0.5f,
		(Minimum.y + Maximum.y) * 0.5f,
		(Minimum.z + Maximum.z) * 0.5f);
	const float3_t HalfExtent(
		(Maximum.x - Minimum.x) * 0.5f,
		(Maximum.y - Minimum.y) * 0.5f,
		(Maximum.z - Minimum.z) * 0.5f);
	const f32_t Radius = std::sqrt(
		HalfExtent.x * HalfExtent.x +
		HalfExtent.y * HalfExtent.y +
		HalfExtent.z * HalfExtent.z);
	if (!std::isfinite(Radius) || Radius <= 0.0001f)
	{
		strOutError = "Mesh preview bounds are invalid.";
		return false;
	}

	ComPtr<ID3D11RenderTargetView> PreviousRTV;
	ComPtr<ID3D11DepthStencilView> PreviousDSV;
	m_pContext->OMGetRenderTargets(1u, &PreviousRTV, &PreviousDSV);
	std::array<D3D11_VIEWPORT,
		D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE>
		PreviousViewports{};
	uint32_t iPreviousViewportCount =
		static_cast<uint32_t>(PreviousViewports.size());
	m_pContext->RSGetViewports(&iPreviousViewportCount, PreviousViewports.data());

	ID3D11RenderTargetView* pTarget = ColorRTV.Get();
	m_pContext->OMSetRenderTargets(1u, &pTarget, DepthDSV.Get());
	const float ClearColor[4] = { 0.035f, 0.045f, 0.06f, 1.f };
	m_pContext->ClearRenderTargetView(ColorRTV.Get(), ClearColor);
	m_pContext->ClearDepthStencilView(
		DepthDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0u);
	D3D11_VIEWPORT Viewport{};
	Viewport.Width = static_cast<f32_t>(MODEL_THUMBNAIL_SIZE);
	Viewport.Height = static_cast<f32_t>(MODEL_THUMBNAIL_SIZE);
	Viewport.MinDepth = 0.f;
	Viewport.MaxDepth = 1.f;
	m_pContext->RSSetViewports(1u, &Viewport);

	const f32_t Distance = Radius / std::sin(XMConvertToRadians(22.5f));
	const matrix_t WorldMatrix =
		XMMatrixTranslation(-Center.x, -Center.y, -Center.z) *
		XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(18.f), XMConvertToRadians(-32.f), 0.f);
	const vector_t Eye = XMVectorSet(0.f, 0.f, -Distance, 1.f);
	const matrix_t ViewMatrix = XMMatrixLookAtLH(
		Eye, XMVectorZero(), XMVectorSet(0.f, 1.f, 0.f, 0.f));
	const matrix_t ProjectionMatrix = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(45.f), 1.f,
		(std::max)(0.001f, Distance - Radius * 1.25f),
		Distance + Radius * 3.f);
	float4x4_t World{};
	float4x4_t View{};
	float4x4_t Projection{};
	XMStoreFloat4x4(&World, WorldMatrix);
	XMStoreFloat4x4(&View, ViewMatrix);
	XMStoreFloat4x4(&Projection, ProjectionMatrix);
	float3_t CameraPosition{};
	XMStoreFloat3(&CameraPosition, Eye);
	const float3_t LightDirection(-0.45f, -0.75f, 0.35f);

	bool_t bRendered =
		SUCCEEDED(pShader->Bind_Matrix("g_WorldMatrix", &World)) &&
		SUCCEEDED(pShader->Bind_Matrix("g_ViewMatrix", &View)) &&
		SUCCEEDED(pShader->Bind_Matrix("g_ProjMatrix", &Projection)) &&
		SUCCEEDED(pShader->Bind_RawValue(
			"g_CameraPosition", &CameraPosition, sizeof(CameraPosition))) &&
		SUCCEEDED(pShader->Bind_RawValue(
			"g_LightDirection", &LightDirection, sizeof(LightDirection)));
	for (uint32_t iMesh = 0u; bRendered && iMesh < Model->Get_NumMeshes(); ++iMesh)
	{
		const uint32_t iHasNormal =
			Model->Has_MaterialTexture(iMesh, aiTextureType_NORMALS) ? 1u : 0u;
		bRendered = SUCCEEDED(pShader->Bind_RawValue(
			"g_HasNormalTexture", &iHasNormal, sizeof(iHasNormal)));
		if (bRendered && bSkinned)
			bRendered = SUCCEEDED(Model->Bind_BoneMatrices(
				pShader, "g_BoneMatrices", iMesh));
		if (bRendered && Model->Has_MaterialTexture(iMesh, aiTextureType_DIFFUSE))
			bRendered = SUCCEEDED(Model->Bind_Material(
				pShader, "g_DiffuseTexture", iMesh, aiTextureType_DIFFUSE));
		else if (bRendered)
			bRendered = SUCCEEDED(pShader->Bind_Texture(
				"g_DiffuseTexture", WhiteSRV));
		if (bRendered && 0u != iHasNormal)
			bRendered = SUCCEEDED(Model->Bind_Material(
				pShader, "g_NormalTexture", iMesh, aiTextureType_NORMALS));
		bRendered = bRendered &&
			SUCCEEDED(pShader->Begin(0u)) &&
			SUCCEEDED(Model->Render(iMesh));
	}

	ID3D11RenderTargetView* pPreviousTarget = PreviousRTV.Get();
	m_pContext->OMSetRenderTargets(1u, &pPreviousTarget, PreviousDSV.Get());
	if (0u != iPreviousViewportCount)
		m_pContext->RSSetViewports(iPreviousViewportCount, PreviousViewports.data());
	if (!bRendered)
	{
		strOutError = "Mesh preview render failed: " + Path.string();
		return false;
	}
	OutTextureView = std::move(ColorSRV);
	strOutError.clear();
	return true;
}

bool_t Client::CEffect_Tool_V2::Compute_SkinnedBounds(
	const std::filesystem::path& Path,
	const Engine::CModel& Model,
	float3_t& OutMinimum,
	float3_t& OutMaximum,
	std::string& strOutInfo)
{
	MODEL_ASSET_LOAD_DESC Desc{};
	Desc.meshPath = Path.lexically_normal();
	std::error_code Error;
	const std::filesystem::path Absolute =
		std::filesystem::absolute(Desc.meshPath, Error).lexically_normal();
	for (std::filesystem::path Current = Absolute.parent_path();
		!Current.empty() && Current != Current.parent_path();
		Current = Current.parent_path())
	{
		if (L"Resources" == Current.filename())
		{
			Desc.assetRoot = Current;
			break;
		}
	}
	MODEL_ASSET_DATA Asset{};
	if (!CModelDecoderRegistry::Get().Decode(Desc, Asset) || Asset.meshes.empty())
		return false;

	const f32_t fMax = (std::numeric_limits<f32_t>::max)();
	float3_t Minimum(fMax, fMax, fMax);
	float3_t Maximum(-fMax, -fMax, -fMax);
	bool_t bAny = false;
	const auto Include = [&](const float3_t& Position)
	{
		if (!std::isfinite(Position.x) || !std::isfinite(Position.y) ||
			!std::isfinite(Position.z))
			return;
		Minimum.x = (std::min)(Minimum.x, Position.x);
		Minimum.y = (std::min)(Minimum.y, Position.y);
		Minimum.z = (std::min)(Minimum.z, Position.z);
		Maximum.x = (std::max)(Maximum.x, Position.x);
		Maximum.y = (std::max)(Maximum.y, Position.y);
		Maximum.z = (std::max)(Maximum.z, Position.z);
		bAny = true;
	};
	const size_t iBoneCount = Asset.skeleton.bones.size();
	std::vector<matrix_t> SkinMatrices(iBoneCount, XMMatrixIdentity());
	for (size_t iBone = 0u; iBone < iBoneCount; ++iBone)
	{
		matrix_t Combined = XMMatrixIdentity();
		if (!Model.Get_BoneCombinedMatrix(static_cast<uint32_t>(iBone), Combined))
			return false;
		SkinMatrices[iBone] =
			XMLoadFloat4x4(&Asset.skeleton.bones[iBone].inverseBind) * Combined;
	}
	for (const MODEL_MESH_DATA& Mesh : Asset.meshes)
	{
		for (const VTXANIMMESH& Vertex : Mesh.skinnedVertices)
		{
			const uint32_t Indices[4] = {
				Vertex.vBlendIndices.x, Vertex.vBlendIndices.y,
				Vertex.vBlendIndices.z, Vertex.vBlendIndices.w };
			const f32_t Weights[4] = {
				Vertex.vBlendWeights.x, Vertex.vBlendWeights.y,
				Vertex.vBlendWeights.z, Vertex.vBlendWeights.w };
			matrix_t Skin = XMMatrixSet(
				0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
				0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
			f32_t fTotalWeight = 0.f;
			for (uint32_t iInfluence = 0u; iInfluence < 4u; ++iInfluence)
			{
				if (Weights[iInfluence] <= 0.f || Indices[iInfluence] >= iBoneCount)
					continue;
				Skin += SkinMatrices[Indices[iInfluence]] * Weights[iInfluence];
				fTotalWeight += Weights[iInfluence];
			}
			if (fTotalWeight <= 0.f)
			{
				Include(Vertex.vPosition);
				continue;
			}
			float3_t Position{};
			XMStoreFloat3(&Position, XMVector3TransformCoord(
				XMLoadFloat3(&Vertex.vPosition), Skin));
			Include(Position);
		}
		for (const VTXMESH& Vertex : Mesh.vertices)
			Include(Vertex.vPosition);
	}
	if (!bAny)
		return false;
	OutMinimum = Minimum;
	OutMaximum = Maximum;

	strOutInfo = "bones=" + std::to_string(Asset.skeleton.bones.size()) +
		" anims=" + std::to_string(Asset.animations.size()) +
		" materials=" + std::to_string(Asset.materials.size());
	for (size_t iMesh = 0u; iMesh < Asset.meshes.size(); ++iMesh)
	{
		const MODEL_MESH_DATA& Mesh = Asset.meshes[iMesh];
		strOutInfo += "\n[" + std::to_string(iMesh) + "] " + Mesh.name +
			" v=" + std::to_string(Mesh.skinnedVertices.size() + Mesh.vertices.size()) +
			" mat=" + std::to_string(Mesh.materialIndex);
		if (Mesh.materialIndex < Asset.materials.size())
		{
			strOutInfo += " diffuse=" +
				Asset.materials[Mesh.materialIndex].diffusePath.filename().string();
		}
	}
	return true;
}

void Client::CEffect_Tool_V2::Render_TypeSelector()
{
	ImGui::TextUnformatted("Effect Type");
	for (int32_t iType = 0; iType < static_cast<int32_t>(EFFECT_TYPE::END);
		++iType)
	{
		if (0 != iType)
			ImGui::SameLine();
		const EFFECT_TYPE eType = static_cast<EFFECT_TYPE>(iType);
		if (ImGui::RadioButton(Type_Label(eType), m_eType == eType))
		{
			m_eType = eType;
			m_bVisibleDirty = true;
		}
	}
}

void Client::CEffect_Tool_V2::Render_SlotCards()
{
	ImGui::Text("%s Slots", Type_Label(m_eType));
	SLOT_BINDINGS& Bindings = Current_Bindings();
	bool_t bFirst = true;
	for (int32_t iSlot = 0; iSlot < static_cast<int32_t>(RESOURCE_SLOT::END);
		++iSlot)
	{
		const RESOURCE_SLOT eSlot = static_cast<RESOURCE_SLOT>(iSlot);
		if (!Slot_VisibleForType(eSlot))
			continue;
		std::string& strAssetId = Bindings[static_cast<size_t>(iSlot)];
		if (!bFirst)
			ImGui::SameLine();
		bFirst = false;
		ImGui::PushID(iSlot);
		ImGui::BeginGroup();

		const PREVIEW_ENTRY* pPreview =
			Request_Preview(strAssetId, Slot_Kind(eSlot));
		bool_t bClicked = false;
		if (nullptr != pPreview && nullptr != pPreview->pTextureView)
		{
			bClicked = ImGui::ImageButton("slot",
				pPreview->pTextureView.Get(),
				ImVec2(SLOT_CARD_SIZE, SLOT_CARD_SIZE));
		}
		else
		{
			const char* pLabel = strAssetId.empty() ? "Empty" :
				(nullptr == pPreview ? "Loading" : "Error");
			bClicked = ImGui::Button(pLabel,
				ImVec2(SLOT_CARD_SIZE + 8.f, SLOT_CARD_SIZE + 8.f));
			if (nullptr != pPreview && !pPreview->strError.empty() &&
				ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("%s", pPreview->strError.c_str());
			}
		}
		if (m_eSelectedSlot == eSlot)
		{
			ImGui::GetWindowDrawList()->AddRect(
				ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
				ImGui::GetColorU32(ImGuiCol_HeaderActive), 2.f, 0, 3.f);
		}
		if (bClicked && m_eSelectedSlot != eSlot)
		{
			m_eSelectedSlot = eSlot;
			m_bVisibleDirty = true;
		}

		ImGui::TextUnformatted(Slot_Label(eSlot));
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s", Slot_Description(eSlot));
		if (strAssetId.empty())
		{
			ImGui::TextDisabled("(none)");
		}
		else
		{
			std::string Name =
				std::filesystem::path(strAssetId).filename().string();
			if (Name.size() > 12u)
				Name = Name.substr(0u, 10u) + "..";
			ImGui::TextDisabled("%s", Name.c_str());
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%s", strAssetId.c_str());
			if (ImGui::SmallButton("Clear"))
				strAssetId.clear();
		}
		ImGui::EndGroup();
		ImGui::PopID();
	}
}

void Client::CEffect_Tool_V2::Render_ResourceBrowser()
{
	const RESOURCE_KIND eKind = Slot_Kind(m_eSelectedSlot);
	ImGui::Text("Resource Library (%s) -> %s / %s",
		RESOURCE_KIND::MODEL == eKind ? "WModel" : "DDS",
		Type_Label(m_eType), Slot_Label(m_eSelectedSlot));

	if (ImGui::Button("Rescan"))
		Scan_Resources();
	ImGui::SameLine();
	ImGui::SetNextItemWidth(180.f);
	if (ImGui::BeginCombo("Domain",
		m_strDomainFilter.empty() ? "All" : m_strDomainFilter.c_str()))
	{
		if (ImGui::Selectable("All", m_strDomainFilter.empty()))
		{
			m_strDomainFilter.clear();
			m_bVisibleDirty = true;
		}
		for (const std::string& Domain : m_Domains)
		{
			if (ImGui::Selectable(Domain.c_str(), m_strDomainFilter == Domain))
			{
				m_strDomainFilter = Domain;
				m_bVisibleDirty = true;
			}
		}
		ImGui::EndCombo();
	}
	ImGui::SameLine();
	ImGui::SetNextItemWidth(200.f);
	if (ImGui::InputTextWithHint("##NameFilter", "filename filter",
		m_szNameFilter, sizeof(m_szNameFilter)))
	{
		m_bVisibleDirty = true;
	}

	if (RESOURCE_KIND::TEXTURE == eKind)
	{
		ImGui::SameLine();
		if (ImGui::Checkbox("Show all", &m_bShowAllTextures))
			m_bVisibleDirty = true;
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip(
				"Off: only textures that the imported/authored source documents bound to this slot at least once.\nOn: every DDS.");
		}
	}

	if (m_bVisibleDirty || m_eVisibleKind != eKind)
		Rebuild_VisibleResources();
	ImGui::TextDisabled("%zu shown", m_VisibleResources.size());

	if (!ImGui::BeginChild("ResourceGrid", ImVec2(0.f, 0.f),
		ImGuiChildFlags_Borders))
	{
		ImGui::EndChild();
		return;
	}
	const float fTileWidth = BROWSER_TILE_SIZE + 12.f;
	const int32_t iColumns = (std::max)(1,
		static_cast<int32_t>(ImGui::GetContentRegionAvail().x / fTileWidth));
	const int32_t iRows = static_cast<int32_t>(
		(m_VisibleResources.size() + iColumns - 1u) / iColumns);
	std::string& strBoundAssetId = Current_SlotAssetId();

	ImGuiListClipper Clipper;
	Clipper.Begin(iRows, BROWSER_TILE_SIZE + 56.f);
	while (Clipper.Step())
	{
		for (int32_t iRow = Clipper.DisplayStart; iRow < Clipper.DisplayEnd;
			++iRow)
		{
			for (int32_t iColumn = 0; iColumn < iColumns; ++iColumn)
			{
				const size_t iVisible = static_cast<size_t>(
					iRow * iColumns + iColumn);
				if (iVisible >= m_VisibleResources.size())
					break;
				const RESOURCE_ENTRY& Entry =
					m_Resources[m_VisibleResources[iVisible]];
				if (0 != iColumn)
					ImGui::SameLine();
				ImGui::PushID(Entry.strAssetId.c_str());
				ImGui::BeginGroup();

				const PREVIEW_ENTRY* pPreview =
					Request_Preview(Entry.strAssetId, Entry.eKind);
				bool_t bClicked = false;
				if (nullptr != pPreview && nullptr != pPreview->pTextureView)
				{
					bClicked = ImGui::ImageButton("tile",
						pPreview->pTextureView.Get(),
						ImVec2(BROWSER_TILE_SIZE, BROWSER_TILE_SIZE));
				}
				else
				{
					bClicked = ImGui::Button(
						nullptr == pPreview ? "..." :
							(RESOURCE_KIND::MODEL == Entry.eKind ? "Mesh" : "DDS"),
						ImVec2(BROWSER_TILE_SIZE + 8.f, BROWSER_TILE_SIZE + 8.f));
					if (nullptr != pPreview && !pPreview->strError.empty() &&
						ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("%s", pPreview->strError.c_str());
					}
				}
				if (Entry.strAssetId == strBoundAssetId)
				{
					ImGui::GetWindowDrawList()->AddRect(
						ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
						ImGui::GetColorU32(ImGuiCol_HeaderActive), 2.f, 0, 3.f);
				}
				std::string Name = Entry.strFileName;
				if (Name.size() > 12u)
					Name = Name.substr(0u, 10u) + "..";
				ImGui::TextUnformatted(Name.c_str());
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("%s", Entry.strAssetId.c_str());
				if (nullptr != Entry.pUsage)
				{
					std::string Badge;
					for (int32_t iSlot = static_cast<int32_t>(RESOURCE_SLOT::BASE);
						iSlot < static_cast<int32_t>(RESOURCE_SLOT::END); ++iSlot)
					{
						const uint32_t iCount =
							Entry.pUsage->Counts[static_cast<size_t>(iSlot)];
						if (0u == iCount)
							continue;
						if (!Badge.empty())
							Badge += ' ';
						Badge += Slot_Label(static_cast<RESOURCE_SLOT>(iSlot))[0];
						Badge += std::to_string(iCount);
					}
					ImGui::TextDisabled("%s", Badge.c_str());
					if (ImGui::IsItemHovered())
					{
						std::string Tip = "Source usage (B=Base N=Noise M=Mask E=Emissive D=Dissolve)";
						for (const std::string& Param : Entry.pUsage->Params)
							Tip += "\n  " + Param;
						ImGui::SetTooltip("%s", Tip.c_str());
					}
				}
				ImGui::EndGroup();
				if (bClicked)
				{
					strBoundAssetId = Entry.strAssetId;
					m_strStatus = std::string("Bound ") + Slot_Label(m_eSelectedSlot) +
						" <- " + Entry.strAssetId;
				}
				ImGui::PopID();
			}
		}
	}
	ImGui::EndChild();
}

void Client::CEffect_Tool_V2::Render_PreviewPanel()
{
	ImGui::Text("Preview: %s / %s",
		Type_Label(m_eType), Slot_Label(m_eSelectedSlot));
	const std::string& strAssetId = Current_SlotAssetId();
	if (strAssetId.empty())
	{
		ImGui::TextDisabled("Select a slot card, then click a resource in the library.");
		return;
	}
	const PREVIEW_ENTRY* pPreview =
		Request_Preview(strAssetId, Slot_Kind(m_eSelectedSlot));
	if (nullptr == pPreview)
	{
		ImGui::TextDisabled("Loading...");
		return;
	}
	if (nullptr == pPreview->pTextureView)
	{
		ImGui::TextColored(ImVec4(1.f, 0.4f, 0.4f, 1.f), "%s",
			pPreview->strError.c_str());
		return;
	}
	const float fAvail = (std::max)(64.f,
		(std::min)(PREVIEW_PANEL_SIZE, ImGui::GetContentRegionAvail().x));
	float fWidth = fAvail;
	float fHeight = fAvail;
	if (pPreview->iWidth > 0u && pPreview->iHeight > 0u)
	{
		const float fAspect = static_cast<float>(pPreview->iWidth) /
			static_cast<float>(pPreview->iHeight);
		if (fAspect >= 1.f)
			fHeight = fAvail / fAspect;
		else
			fWidth = fAvail * fAspect;
	}
	ImGui::Image(pPreview->pTextureView.Get(), ImVec2(fWidth, fHeight));
	ImGui::TextWrapped("%s", strAssetId.c_str());
	if (RESOURCE_KIND::MODEL == Slot_Kind(m_eSelectedSlot))
		ImGui::TextDisabled("WModel thumbnail %u px", pPreview->iWidth);
	else
		ImGui::TextDisabled("%u x %u", pPreview->iWidth, pPreview->iHeight);
	if (!pPreview->strInfo.empty())
		ImGui::TextWrapped("%s", pPreview->strInfo.c_str());
}

void Client::CEffect_Tool_V2::Render_CreatePanel()
{
	ImGui::TextUnformatted("World Preview");
	const SLOT_BINDINGS& Bindings = Current_Bindings();
	const bool_t bMeshType =
		EFFECT_TYPE::MESH == m_eType || EFFECT_TYPE::PARTICLE == m_eType;
	const bool_t bSupportedType =
		EFFECT_TYPE::MESH == m_eType || EFFECT_TYPE::TEXTURE == m_eType;
	const bool_t bHasBase =
		!Bindings[static_cast<size_t>(RESOURCE_SLOT::BASE)].empty();
	const bool_t bHasMesh =
		!Bindings[static_cast<size_t>(RESOURCE_SLOT::MESH)].empty();
	const bool_t bCanCreate =
		bSupportedType && bHasBase && (!bMeshType || bHasMesh);
	ImGui::BeginDisabled(!bCanCreate);
	if (ImGui::Button("Create Effect"))
		Try_CreatePreview();
	ImGui::EndDisabled();
	if (!bSupportedType)
		ImGui::TextDisabled("Only Mesh and Texture types can be previewed yet.");
	else if (!bHasBase)
		ImGui::TextDisabled("Bind a Base texture first.");
	else if (bMeshType && !bHasMesh)
		ImGui::TextDisabled("Bind a Mesh first.");
	if (!m_strPreviewStatus.empty())
		ImGui::TextWrapped("%s", m_strPreviewStatus.c_str());
}

bool_t Client::CEffect_Tool_V2::Try_CreatePreview()
{
	CGameInstance& GameInstance = CGameInstance::Get();
	if (!m_bPreviewPrototypeRegistered)
	{
		unique_ptr<CEffectPreviewV2> pPrototype =
			CEffectPreviewV2::Create(m_pDevice, m_pContext);
		if (nullptr == pPrototype)
		{
			m_strPreviewStatus = "Preview prototype creation failed.";
			return false;
		}
		const HRESULT hResult = GameInstance.Add_Prototype(
			ETOUI(LEVEL::STATIC), L"Prototype_GameObject_EffectPreviewV2",
			std::move(pPrototype));
		if (FAILED(hResult))
		{
			m_strPreviewStatus =
				"Prototype registration returned failure (already registered or STATIC level unavailable); continuing.";
		}
		m_bPreviewPrototypeRegistered = true;
	}
	if (const std::shared_ptr<CEffectPreviewV2> pPrevious = m_pPreview.lock())
		pPrevious->Set_Hidden(true);

	const SLOT_BINDINGS& Bindings = Current_Bindings();
	CEffectPreviewV2::DESC Desc{};
	Desc.eShape = EFFECT_TYPE::MESH == m_eType ?
		CEffectPreviewV2::SHAPE::MESH : CEffectPreviewV2::SHAPE::SPRITE;
	Desc.strMeshAssetId = Bindings[static_cast<size_t>(RESOURCE_SLOT::MESH)];
	for (int32_t iSlot = static_cast<int32_t>(RESOURCE_SLOT::BASE);
		iSlot < static_cast<int32_t>(RESOURCE_SLOT::END); ++iSlot)
	{
		Desc.TextureAssetIds[static_cast<size_t>(
			iSlot - static_cast<int32_t>(RESOURCE_SLOT::BASE))] =
			Bindings[static_cast<size_t>(iSlot)];
	}

	const float4_t* pCameraPosition = GameInstance.Get_CamPosition();
	const float4x4_t* pCameraWorld = GameInstance.Get_InverseTransform(D3DTS::VIEW);
	if (nullptr == pCameraPosition || nullptr == pCameraWorld)
	{
		m_strPreviewStatus = "Camera is not available in this level.";
		return false;
	}
	const vector_t Look = XMVector3Normalize(XMLoadFloat4x4(pCameraWorld).r[2]);
	const vector_t Spawn = XMLoadFloat4(pCameraPosition) + Look * 3.f;
	XMStoreFloat3(&Desc.vPosition, Spawn);
	Desc.vScale = { 1.f, 1.f, 1.f };

	std::shared_ptr<CGameObject> pGameObject;
	if (FAILED(GameInstance.Add_GameObject_to_Layer(
		ETOUI(LEVEL::STATIC), L"Prototype_GameObject_EffectPreviewV2",
		GameInstance.Get_CurrentLevelID(), L"Layer_EffectPreviewV2",
		&Desc, &pGameObject)))
	{
		m_strPreviewStatus = "Create failed: " +
			(CEffectPreviewV2::Last_Error().empty() ?
				std::string("prototype clone or layer add failed.") :
				CEffectPreviewV2::Last_Error());
		return false;
	}
	const std::shared_ptr<CEffectPreviewV2> pPreview =
		std::dynamic_pointer_cast<CEffectPreviewV2>(pGameObject);
	if (nullptr == pPreview)
	{
		m_strPreviewStatus = "Create failed: unexpected object type.";
		return false;
	}
	m_pPreview = pPreview;
	m_strPreviewStatus = "Spawned at camera forward 3 m.";
	return true;
}

void Client::CEffect_Tool_V2::Render_TuningPanel()
{
	const std::shared_ptr<CEffectPreviewV2> pPreview = m_pPreview.lock();
	if (nullptr == pPreview)
	{
		ImGui::TextDisabled("No live preview. Create Effect to spawn one.");
		return;
	}
	ImGui::SeparatorText("Tuning");
	ImGui::Text("%s | %.2fs | %s",
		CEffectPreviewV2::SHAPE::MESH == pPreview->Shape() ? "Mesh" : "Sprite",
		pPreview->Time(), pPreview->Status().c_str());
	if (ImGui::Button("Restart"))
		pPreview->Restart();
	ImGui::SameLine();
	bool_t bVisible = !pPreview->Is_Hidden();
	if (ImGui::Checkbox("Visible", &bVisible))
		pPreview->Set_Hidden(!bVisible);
	ImGui::SameLine();
	if (ImGui::Button("Bring To Camera"))
	{
		CGameInstance& GameInstance = CGameInstance::Get();
		const float4_t* pCameraPosition = GameInstance.Get_CamPosition();
		const float4x4_t* pCameraWorld = GameInstance.Get_InverseTransform(D3DTS::VIEW);
		if (nullptr != pCameraPosition && nullptr != pCameraWorld)
		{
			const vector_t Look = XMVector3Normalize(XMLoadFloat4x4(pCameraWorld).r[2]);
			XMStoreFloat3(&pPreview->Position(),
				XMLoadFloat4(pCameraPosition) + Look * 3.f);
		}
	}

	ImGui::SeparatorText("Transform");
	ImGui::DragFloat3("Position", &pPreview->Position().x, 0.05f);
	ImGui::DragFloat3("Rotation (deg)", &pPreview->RotationDegrees().x, 1.f);
	ImGui::DragFloat3("Scale", &pPreview->Scale().x, 0.01f, 0.001f, 1000.f);

	CEffectPreviewV2::PARAMS& P = pPreview->Params();
	ImGui::SeparatorText("Material");
	ImGui::ColorEdit4("Tint", &P.vTint.x, ImGuiColorEditFlags_Float);
	int32_t iBlend = static_cast<int32_t>(P.eBlend);
	if (ImGui::Combo("Blend", &iBlend, "Alpha\0Additive\0"))
		P.eBlend = static_cast<CEffectPreviewV2::BLEND_MODE>(iBlend);
	ImGui::Checkbox("Depth Test", &P.bDepthTest);
	if (CEffectPreviewV2::SHAPE::SPRITE == pPreview->Shape())
	{
		ImGui::SameLine();
		ImGui::Checkbox("Billboard", &P.bBillboard);
	}
	ImGui::DragFloat2("UV Scale", &P.vUVScale.x, 0.01f, 0.01f, 64.f);
	ImGui::DragFloat2("Base Pan (uv/s)", &P.vBasePan.x, 0.01f, -10.f, 10.f);
	ImGui::DragFloat("Noise Strength", &P.fNoiseStrength, 0.005f, 0.f, 2.f);
	ImGui::DragFloat("Noise Scale", &P.fNoiseScale, 0.01f, 0.01f, 64.f);
	ImGui::DragFloat2("Noise Pan (uv/s)", &P.vNoisePan.x, 0.01f, -10.f, 10.f);
	ImGui::DragFloat("Emissive Intensity", &P.fEmissiveIntensity, 0.05f, 0.f, 32.f);
	ImGui::SliderFloat("Dissolve Amount", &P.fDissolveAmount, 0.f, 1.f);
	ImGui::SliderFloat("Dissolve Softness", &P.fDissolveSoftness, 0.f, 0.5f);

	ImGui::SeparatorText("Playback");
	ImGui::DragFloat("Lifetime (s, 0 = infinite)", &P.fLifetime, 0.05f, 0.f, 600.f);
	ImGui::Checkbox("Loop", &P.bLoop);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(120.f);
	ImGui::DragFloat("Play Rate", &P.fPlayRate, 0.01f, 0.f, 16.f);
	if (pPreview->Is_Finished())
		ImGui::TextDisabled("Finished (lifetime reached). Restart to replay.");
}

Client::CEffect_Tool_V2::SLOT_BINDINGS& Client::CEffect_Tool_V2::Current_Bindings()
{
	return m_Bindings[static_cast<size_t>(m_eType)];
}

std::string& Client::CEffect_Tool_V2::Current_SlotAssetId()
{
	return Current_Bindings()[static_cast<size_t>(m_eSelectedSlot)];
}

bool_t Client::CEffect_Tool_V2::Slot_VisibleForType(const RESOURCE_SLOT eSlot) const
{
	if (RESOURCE_SLOT::MESH != eSlot)
		return true;
	return EFFECT_TYPE::MESH == m_eType || EFFECT_TYPE::PARTICLE == m_eType;
}

Client::CEffect_Tool_V2::RESOURCE_KIND Client::CEffect_Tool_V2::Slot_Kind(
	const RESOURCE_SLOT eSlot)
{
	return RESOURCE_SLOT::MESH == eSlot ?
		RESOURCE_KIND::MODEL : RESOURCE_KIND::TEXTURE;
}

const char* Client::CEffect_Tool_V2::Type_Label(const EFFECT_TYPE eType)
{
	switch (eType)
	{
	case EFFECT_TYPE::MESH: return "Mesh";
	case EFFECT_TYPE::TEXTURE: return "Texture";
	case EFFECT_TYPE::PARTICLE: return "Particle";
	case EFFECT_TYPE::DECAL: return "Decal";
	case EFFECT_TYPE::TRAIL: return "Trail";
	default: return "Unknown";
	}
}

const char* Client::CEffect_Tool_V2::Slot_Label(const RESOURCE_SLOT eSlot)
{
	switch (eSlot)
	{
	case RESOURCE_SLOT::MESH: return "Mesh";
	case RESOURCE_SLOT::BASE: return "Base";
	case RESOURCE_SLOT::NOISE: return "Noise";
	case RESOURCE_SLOT::MASK: return "Mask";
	case RESOURCE_SLOT::EMISSIVE: return "Emissive";
	case RESOURCE_SLOT::DISSOLVE: return "Dissolve";
	default: return "Unknown";
	}
}

const char* Client::CEffect_Tool_V2::Slot_Description(const RESOURCE_SLOT eSlot)
{
	switch (eSlot)
	{
	case RESOURCE_SLOT::MESH: return "Mesh: one WModel carrier shape.";
	case RESOURCE_SLOT::BASE: return "Base: RGB color, A opacity.";
	case RESOURCE_SLOT::NOISE: return "Noise: UV distortion source.";
	case RESOURCE_SLOT::MASK: return "Mask: R channel multiplies opacity.";
	case RESOURCE_SLOT::EMISSIVE: return "Emissive: RGB added as glow.";
	case RESOURCE_SLOT::DISSOLVE: return "Dissolve: R channel threshold over lifetime.";
	default: return "";
	}
}
```

### Client/Public/Effect_Preview_V2.h

```cpp
#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

#include <array>
#include <cstdint>
#include <string>

NS_BEGIN(Engine)
class CModel;
class CShader;
class CVIBuffer_Rect;
NS_END

NS_BEGIN(Client)

class CEffectPreviewV2 final : public CGameObject
{
public:
	enum class SHAPE : int32_t
	{
		MESH,
		SPRITE,
		END
	};

	enum class BLEND_MODE : int32_t
	{
		ALPHA,
		ADDITIVE,
		END
	};

	enum class TEXTURE_INPUT : int32_t
	{
		BASE,
		NOISE,
		MASK,
		EMISSIVE,
		DISSOLVE,
		END
	};

	struct PARAMS final
	{
		float4_t vTint = { 1.f, 1.f, 1.f, 1.f };
		BLEND_MODE eBlend = BLEND_MODE::ADDITIVE;
		bool_t bBillboard = true;
		bool_t bDepthTest = true;
		float2_t vUVScale = { 1.f, 1.f };
		float2_t vBasePan = { 0.f, 0.f };
		f32_t fNoiseStrength = 0.f;
		f32_t fNoiseScale = 1.f;
		float2_t vNoisePan = { 0.f, 0.f };
		f32_t fEmissiveIntensity = 1.f;
		f32_t fDissolveAmount = 0.f;
		f32_t fDissolveSoftness = 0.1f;
		f32_t fLifetime = 0.f;
		bool_t bLoop = true;
		f32_t fPlayRate = 1.f;
	};

	struct DESC final : public GAMEOBJECT_DESC
	{
		SHAPE eShape = SHAPE::SPRITE;
		std::string strMeshAssetId;
		std::array<std::string, static_cast<size_t>(TEXTURE_INPUT::END)> TextureAssetIds;
		float3_t vPosition = { 0.f, 0.f, 0.f };
		float3_t vRotationDegrees = { 0.f, 0.f, 0.f };
		float3_t vScale = { 1.f, 1.f, 1.f };
		PARAMS Params;
	};

private:
	CEffectPreviewV2(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CEffectPreviewV2();

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

	PARAMS& Params() { return m_Params; }
	float3_t& Position() { return m_vPosition; }
	float3_t& RotationDegrees() { return m_vRotationDegrees; }
	float3_t& Scale() { return m_vScale; }
	const std::string& Status() const { return m_strStatus; }
	SHAPE Shape() const { return m_eShape; }
	f32_t Time() const { return m_fTime; }
	bool_t Is_Finished() const { return m_bFinished; }
	bool_t Is_Hidden() const { return m_bHidden; }
	void Set_Hidden(const bool_t bHidden) { m_bHidden = bHidden; }
	void Restart();
	static const std::string& Last_Error() { return s_strLastError; }

private:
	HRESULT Load_Texture(
		const std::string& strAssetId, ComPtr<ID3D11ShaderResourceView>& OutView);
	void Apply_Transform();
	HRESULT Bind_Common(const shared_ptr<Engine::CShader>& pShader);

private:
	SHAPE m_eShape = SHAPE::SPRITE;
	PARAMS m_Params;
	float3_t m_vPosition = { 0.f, 0.f, 0.f };
	float3_t m_vRotationDegrees = { 0.f, 0.f, 0.f };
	float3_t m_vScale = { 1.f, 1.f, 1.f };
	f32_t m_fTime = 0.f;
	bool_t m_bFinished = false;
	bool_t m_bHidden = false;
	bool_t m_bSkinned = false;
	std::string m_strStatus;

	shared_ptr<Engine::CShader> m_pShader;
	shared_ptr<Engine::CModel> m_pModel;
	shared_ptr<Engine::CVIBuffer_Rect> m_pRect;
	std::array<ComPtr<ID3D11ShaderResourceView>,
		static_cast<size_t>(TEXTURE_INPUT::END)> m_Textures;
	static std::string s_strLastError;

public:
	static unique_ptr<CEffectPreviewV2> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
```

### Client/Private/Effect_Preview_V2.cpp

```cpp
#include "Effect_Preview_V2.h"
#include "GameInstance.h"
#include "Model.h"
#include "RuntimeAssetRoot.h"
#include "Shader.h"
#include "VIBuffer_Rect.h"

#include "DirectXTK/DDSTextureLoader.h"

#include <cmath>
#include <filesystem>

namespace
{
	constexpr const char* TEXTURE_CONSTANTS[] = {
		"g_BaseTexture", "g_NoiseTexture", "g_MaskTexture",
		"g_EmissiveTexture", "g_DissolveTexture"
	};
	constexpr const char* TEXTURE_FLAG_CONSTANTS[] = {
		"g_HasBase", "g_HasNoise", "g_HasMask", "g_HasEmissive", "g_HasDissolve"
	};
}

Client::CEffectPreviewV2::CEffectPreviewV2(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject(std::move(pDevice), std::move(pContext))
{
}

Client::CEffectPreviewV2::~CEffectPreviewV2() = default;

HRESULT Client::CEffectPreviewV2::Initialize_Prototype()
{
	return S_OK;
}

std::string Client::CEffectPreviewV2::s_strLastError;

HRESULT Client::CEffectPreviewV2::Initialize(void* pArg)
{
	const auto Fail = [this](std::string strReason)
	{
		m_strStatus = std::move(strReason);
		s_strLastError = m_strStatus;
		return E_FAIL;
	};
	s_strLastError.clear();
	if (nullptr == pArg)
		return Fail("Preview desc is null.");
	if (FAILED(__super::Initialize(pArg)))
		return Fail("Transform component creation failed.");
	const DESC& Desc = *static_cast<const DESC*>(pArg);
	m_eShape = Desc.eShape;
	m_Params = Desc.Params;
	m_vPosition = Desc.vPosition;
	m_vRotationDegrees = Desc.vRotationDegrees;
	m_vScale = Desc.vScale;

	if (SHAPE::MESH == m_eShape)
	{
		const std::filesystem::path MeshPath =
			CRuntimeAssetRoot::Resolve(std::filesystem::path(Desc.strMeshAssetId));
		if (MeshPath.empty() || !std::filesystem::is_regular_file(MeshPath))
			return Fail("Mesh asset is missing: " + Desc.strMeshAssetId);
		unique_ptr<Engine::CModel> Model = Engine::CModel::Create(
			m_pDevice, m_pContext, MODEL::NONANIM,
			MeshPath.string().c_str(), XMMatrixIdentity());
		m_bSkinned = false;
		if (nullptr == Model)
		{
			Model = Engine::CModel::Create(
				m_pDevice, m_pContext, MODEL::ANIM,
				MeshPath.string().c_str(), XMMatrixIdentity());
			m_bSkinned = nullptr != Model;
		}
		if (nullptr == Model)
			return Fail("Mesh load failed: " + Desc.strMeshAssetId);
		m_pModel = std::move(Model);
		unique_ptr<Engine::CShader> Shader = m_bSkinned ?
			Engine::CShader::Create(m_pDevice, m_pContext,
				TEXT("../Bin/ShaderFiles/Shader_EffectAnimMeshV2.hlsl"),
				VTXANIMMESH::Elements, VTXANIMMESH::iNumElements) :
			Engine::CShader::Create(m_pDevice, m_pContext,
				TEXT("../Bin/ShaderFiles/Shader_EffectMeshV2.hlsl"),
				VTXMESH::Elements, VTXMESH::iNumElements);
		if (nullptr == Shader)
		{
			return Fail(m_bSkinned ?
				"Shader_EffectAnimMeshV2.hlsl compile failed." :
				"Shader_EffectMeshV2.hlsl compile failed.");
		}
		m_pShader = std::move(Shader);
	}
	else
	{
		unique_ptr<Engine::CVIBuffer_Rect> Rect =
			Engine::CVIBuffer_Rect::Create(m_pDevice, m_pContext);
		if (nullptr == Rect)
			return Fail("Rect buffer creation failed.");
		m_pRect = std::move(Rect);
		unique_ptr<Engine::CShader> Shader = Engine::CShader::Create(
			m_pDevice, m_pContext,
			TEXT("../Bin/ShaderFiles/Shader_EffectRectV2.hlsl"),
			VTXTEX::Elements, VTXTEX::iNumElements);
		if (nullptr == Shader)
			return Fail("Shader_EffectRectV2.hlsl compile failed.");
		m_pShader = std::move(Shader);
	}

	for (size_t iInput = 0u; iInput < m_Textures.size(); ++iInput)
	{
		const std::string& strAssetId = Desc.TextureAssetIds[iInput];
		if (strAssetId.empty())
			continue;
		if (FAILED(Load_Texture(strAssetId, m_Textures[iInput])))
			return Fail("Texture load failed: " + strAssetId);
	}
	m_strStatus = "Ready";
	Apply_Transform();
	return S_OK;
}

HRESULT Client::CEffectPreviewV2::Load_Texture(
	const std::string& strAssetId,
	ComPtr<ID3D11ShaderResourceView>& OutView)
{
	const std::filesystem::path Path =
		CRuntimeAssetRoot::Resolve(std::filesystem::path(strAssetId));
	if (Path.empty() || !std::filesystem::is_regular_file(Path))
		return E_FAIL;
	return DirectX::CreateDDSTextureFromFile(
		m_pDevice.Get(), Path.c_str(), nullptr, &OutView);
}

void Client::CEffectPreviewV2::Restart()
{
	m_fTime = 0.f;
	m_bFinished = false;
}

void Client::CEffectPreviewV2::Update(const f32_t fTimeDelta)
{
	if (!m_bFinished)
	{
		m_fTime += fTimeDelta * m_Params.fPlayRate;
		if (m_Params.fLifetime > 0.f && m_fTime >= m_Params.fLifetime)
		{
			if (m_Params.bLoop)
				m_fTime = std::fmod(m_fTime, m_Params.fLifetime);
			else
				m_bFinished = true;
		}
	}
	Apply_Transform();
}

void Client::CEffectPreviewV2::Apply_Transform()
{
	const matrix_t Scale =
		XMMatrixScaling(m_vScale.x, m_vScale.y, m_vScale.z);
	matrix_t Rotation = XMMatrixRotationRollPitchYaw(
		XMConvertToRadians(m_vRotationDegrees.x),
		XMConvertToRadians(m_vRotationDegrees.y),
		XMConvertToRadians(m_vRotationDegrees.z));
	if (SHAPE::SPRITE == m_eShape && m_Params.bBillboard)
	{
		const float4x4_t* pCameraWorld =
			CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW);
		if (nullptr != pCameraWorld)
		{
			matrix_t CameraWorld = XMLoadFloat4x4(pCameraWorld);
			CameraWorld.r[0] = XMVector3Normalize(CameraWorld.r[0]);
			CameraWorld.r[1] = XMVector3Normalize(CameraWorld.r[1]);
			CameraWorld.r[2] = XMVector3Normalize(CameraWorld.r[2]);
			CameraWorld.r[3] = XMVectorSet(0.f, 0.f, 0.f, 1.f);
			Rotation = Rotation * CameraWorld;
		}
	}
	const matrix_t World = Scale * Rotation *
		XMMatrixTranslation(m_vPosition.x, m_vPosition.y, m_vPosition.z);
	m_pTransformCom->Set_State(STATE::RIGHT, World.r[0]);
	m_pTransformCom->Set_State(STATE::UP, World.r[1]);
	m_pTransformCom->Set_State(STATE::LOOK, World.r[2]);
	m_pTransformCom->Set_State(STATE::POSITION, World.r[3]);
}

void Client::CEffectPreviewV2::Late_Update(const f32_t fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);
	if (m_bHidden || m_bFinished || nullptr == m_pShader)
		return;
	CGameInstance::Get().Add_RenderObject(
		RENDERGROUP::BLEND,
		static_pointer_cast<CGameObject>(shared_from_this()));
}

HRESULT Client::CEffectPreviewV2::Bind_Common(
	const shared_ptr<Engine::CShader>& pShader)
{
	CGameInstance& GameInstance = CGameInstance::Get();
	if (FAILED(m_pTransformCom->Bind_ShaderResource(pShader, "g_WorldMatrix")) ||
		FAILED(GameInstance.Bind_Transform(pShader, "g_ViewMatrix", D3DTS::VIEW)) ||
		FAILED(GameInstance.Bind_Transform(pShader, "g_ProjMatrix", D3DTS::PROJ)))
	{
		return E_FAIL;
	}
	const PARAMS& P = m_Params;
	if (FAILED(pShader->Bind_RawValue("g_Time", &m_fTime, sizeof(m_fTime))) ||
		FAILED(pShader->Bind_RawValue("g_Tint", &P.vTint, sizeof(P.vTint))) ||
		FAILED(pShader->Bind_RawValue("g_UVScale", &P.vUVScale, sizeof(P.vUVScale))) ||
		FAILED(pShader->Bind_RawValue("g_BasePan", &P.vBasePan, sizeof(P.vBasePan))) ||
		FAILED(pShader->Bind_RawValue("g_NoiseStrength", &P.fNoiseStrength, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_NoiseScale", &P.fNoiseScale, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_NoisePan", &P.vNoisePan, sizeof(P.vNoisePan))) ||
		FAILED(pShader->Bind_RawValue("g_EmissiveIntensity", &P.fEmissiveIntensity, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_DissolveAmount", &P.fDissolveAmount, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_DissolveSoftness", &P.fDissolveSoftness, sizeof(f32_t))))
	{
		return E_FAIL;
	}
	for (size_t iInput = 0u; iInput < m_Textures.size(); ++iInput)
	{
		const uint32_t iHas = nullptr != m_Textures[iInput] ? 1u : 0u;
		if (FAILED(pShader->Bind_RawValue(
			TEXTURE_FLAG_CONSTANTS[iInput], &iHas, sizeof(iHas))))
			return E_FAIL;
		if (0u != iHas &&
			FAILED(pShader->Bind_Texture(TEXTURE_CONSTANTS[iInput], m_Textures[iInput])))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT Client::CEffectPreviewV2::Render()
{
	if (FAILED(Bind_Common(m_pShader)))
	{
		m_strStatus = "Shader bind failed.";
		return E_FAIL;
	}
	const uint32_t iPass =
		static_cast<uint32_t>(m_Params.eBlend) + (m_Params.bDepthTest ? 0u : 2u);
	if (SHAPE::MESH == m_eShape)
	{
		for (uint32_t iMesh = 0u; iMesh < m_pModel->Get_NumMeshes(); ++iMesh)
		{
			if (m_bSkinned && FAILED(m_pModel->Bind_BoneMatrices(
				m_pShader, "g_BoneMatrices", iMesh)))
			{
				m_strStatus = "Bone matrix bind failed.";
				return E_FAIL;
			}
			if (FAILED(m_pShader->Begin(iPass)) || FAILED(m_pModel->Render(iMesh)))
			{
				m_strStatus = "Mesh draw failed.";
				return E_FAIL;
			}
		}
		return S_OK;
	}
	if (FAILED(m_pShader->Begin(iPass)) || FAILED(m_pRect->Render()))
	{
		m_strStatus = "Sprite draw failed.";
		return E_FAIL;
	}
	return S_OK;
}

unique_ptr<Client::CEffectPreviewV2> Client::CEffectPreviewV2::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	unique_ptr<CEffectPreviewV2> Instance(new CEffectPreviewV2(
		std::move(pDevice), std::move(pContext)));
	if (FAILED(Instance->Initialize_Prototype()))
		return nullptr;
	return Instance;
}

shared_ptr<CPrototype> Client::CEffectPreviewV2::Clone(void* pArg)
{
	shared_ptr<CEffectPreviewV2> Instance(new CEffectPreviewV2(m_pDevice, m_pContext));
	if (FAILED(Instance->Initialize(pArg)))
		return nullptr;
	return Instance;
}
```

### Client/Bin/ShaderFiles/Shader_EffectV2_Common.hlsli

```hlsl
#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix;
float4x4 g_ViewMatrix;
float4x4 g_ProjMatrix;

float g_Time;
float4 g_Tint = float4(1.f, 1.f, 1.f, 1.f);
float2 g_UVScale = float2(1.f, 1.f);
float2 g_BasePan = float2(0.f, 0.f);
float g_NoiseStrength = 0.f;
float g_NoiseScale = 1.f;
float2 g_NoisePan = float2(0.f, 0.f);
float g_EmissiveIntensity = 1.f;
float g_DissolveAmount = 0.f;
float g_DissolveSoftness = 0.1f;

Texture2D g_BaseTexture;
Texture2D g_NoiseTexture;
Texture2D g_MaskTexture;
Texture2D g_EmissiveTexture;
Texture2D g_DissolveTexture;
uint g_HasBase = 0;
uint g_HasNoise = 0;
uint g_HasMask = 0;
uint g_HasEmissive = 0;
uint g_HasDissolve = 0;

RasterizerState RS_EffectV2
{
	FillMode = Solid;
	CullMode = None;
	FrontCounterClockwise = false;
};

struct PS_EFFECT_IN
{
	float4 vPosition : SV_POSITION;
	float2 vTexcoord : TEXCOORD0;
};

float4 PS_EFFECT_V2(PS_EFFECT_IN input) : SV_TARGET0
{
	const float2 uv = input.vTexcoord * g_UVScale + g_BasePan * g_Time;

	float2 distortion = float2(0.f, 0.f);
	if (0 != g_HasNoise)
	{
		const float2 noiseUV = uv * g_NoiseScale + g_NoisePan * g_Time;
		distortion = (g_NoiseTexture.Sample(LinearSampler, noiseUV).rg * 2.f - 1.f) *
			g_NoiseStrength;
	}
	const float2 baseUV = uv + distortion;

	float4 base = float4(1.f, 1.f, 1.f, 1.f);
	if (0 != g_HasBase)
		base = g_BaseTexture.Sample(LinearSampler, baseUV);

	float mask = 1.f;
	if (0 != g_HasMask)
		mask = g_MaskTexture.Sample(LinearSampler, uv).r;

	float3 emissive = float3(0.f, 0.f, 0.f);
	if (0 != g_HasEmissive)
		emissive = g_EmissiveTexture.Sample(LinearSampler, baseUV).rgb * g_EmissiveIntensity;

	float dissolve = 1.f;
	if (0 != g_HasDissolve)
	{
		const float threshold = g_DissolveTexture.Sample(LinearSampler, uv).r;
		dissolve = smoothstep(
			g_DissolveAmount - g_DissolveSoftness,
			g_DissolveAmount + g_DissolveSoftness,
			threshold);
	}

	const float alpha = base.a * mask * dissolve * g_Tint.a;
	if (alpha <= 0.001f)
		discard;
	const float3 color = base.rgb * g_Tint.rgb + emissive;
	return float4(color, alpha);
}

#define EFFECT_V2_PASSES(VS_FUNC) \
	pass AlphaDepth \
	{ \
		SetRasterizerState(RS_EffectV2); \
		SetDepthStencilState(DSS_ReadOnly, 0); \
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff); \
		VertexShader = compile vs_5_0 VS_FUNC(); \
		GeometryShader = NULL; \
		PixelShader = compile ps_5_0 PS_EFFECT_V2(); \
	} \
	pass AdditiveDepth \
	{ \
		SetRasterizerState(RS_EffectV2); \
		SetDepthStencilState(DSS_ReadOnly, 0); \
		SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff); \
		VertexShader = compile vs_5_0 VS_FUNC(); \
		GeometryShader = NULL; \
		PixelShader = compile ps_5_0 PS_EFFECT_V2(); \
	} \
	pass AlphaNoDepth \
	{ \
		SetRasterizerState(RS_EffectV2); \
		SetDepthStencilState(DSS_ZNone, 0); \
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff); \
		VertexShader = compile vs_5_0 VS_FUNC(); \
		GeometryShader = NULL; \
		PixelShader = compile ps_5_0 PS_EFFECT_V2(); \
	} \
	pass AdditiveNoDepth \
	{ \
		SetRasterizerState(RS_EffectV2); \
		SetDepthStencilState(DSS_ZNone, 0); \
		SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff); \
		VertexShader = compile vs_5_0 VS_FUNC(); \
		GeometryShader = NULL; \
		PixelShader = compile ps_5_0 PS_EFFECT_V2(); \
	}
```

### Client/Bin/ShaderFiles/Shader_EffectMeshV2.hlsl

```hlsl
#include "Shader_EffectV2_Common.hlsli"

struct VS_IN
{
	float3 vPosition : POSITION;
	float3 vNormal : NORMAL;
	float3 vTangent : TANGENT;
	float3 vBinormal : BINORMAL;
	float2 vTexcoord : TEXCOORD0;
};

PS_EFFECT_IN VS_MAIN(VS_IN input)
{
	PS_EFFECT_IN output;
	const matrix worldViewProjection =
		mul(mul(g_WorldMatrix, g_ViewMatrix), g_ProjMatrix);
	output.vPosition = mul(float4(input.vPosition, 1.f), worldViewProjection);
	output.vTexcoord = input.vTexcoord;
	return output;
}

technique11 DefaultTechnique
{
	EFFECT_V2_PASSES(VS_MAIN)
}
```

### Client/Bin/ShaderFiles/Shader_EffectAnimMeshV2.hlsl

```hlsl
#include "Shader_EffectV2_Common.hlsli"

matrix g_BoneMatrices[512];

struct VS_IN
{
	float3 vPosition : POSITION;
	float3 vNormal : NORMAL;
	float3 vTangent : TANGENT;
	float3 vBinormal : BINORMAL;
	float2 vTexcoord : TEXCOORD0;
	uint4 vBlendIndices : BLENDINDEX;
	float4 vBlendWeights : BLENDWEIGHT;
};

PS_EFFECT_IN VS_MAIN(VS_IN input)
{
	PS_EFFECT_IN output;
	const matrix boneMatrix =
		g_BoneMatrices[input.vBlendIndices.x] * input.vBlendWeights.x +
		g_BoneMatrices[input.vBlendIndices.y] * input.vBlendWeights.y +
		g_BoneMatrices[input.vBlendIndices.z] * input.vBlendWeights.z +
		g_BoneMatrices[input.vBlendIndices.w] * input.vBlendWeights.w;
	const float4 skinnedPosition = mul(float4(input.vPosition, 1.f), boneMatrix);
	const matrix worldViewProjection =
		mul(mul(g_WorldMatrix, g_ViewMatrix), g_ProjMatrix);
	output.vPosition = mul(skinnedPosition, worldViewProjection);
	output.vTexcoord = input.vTexcoord;
	return output;
}

technique11 DefaultTechnique
{
	EFFECT_V2_PASSES(VS_MAIN)
}
```

### Client/Bin/ShaderFiles/Shader_EffectRectV2.hlsl

```hlsl
#include "Shader_EffectV2_Common.hlsli"

struct VS_IN
{
	float3 vPosition : POSITION;
	float2 vTexcoord : TEXCOORD0;
};

PS_EFFECT_IN VS_MAIN(VS_IN input)
{
	PS_EFFECT_IN output;
	const matrix worldViewProjection =
		mul(mul(g_WorldMatrix, g_ViewMatrix), g_ProjMatrix);
	output.vPosition = mul(float4(input.vPosition, 1.f), worldViewProjection);
	output.vTexcoord = input.vTexcoord;
	return output;
}

technique11 DefaultTechnique
{
	EFFECT_V2_PASSES(VS_MAIN)
}
```

### Client/Bin/ShaderFiles/Shader_VtxAnimMeshPreview_V2.hlsl

```hlsl
#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix;
float4x4 g_ViewMatrix;
float4x4 g_ProjMatrix;
matrix g_BoneMatrices[512];

float3 g_CameraPosition;
float3 g_LightDirection;

Texture2D g_DiffuseTexture;
Texture2D g_NormalTexture;
uint g_HasNormalTexture = 0;

RasterizerState RS_Preview
{
	FillMode = Solid;
	CullMode = None;
	FrontCounterClockwise = false;
};

struct VS_IN
{
	float3 vPosition : POSITION;
	float3 vNormal : NORMAL;
	float3 vTangent : TANGENT;
	float3 vBinormal : BINORMAL;
	float2 vTexcoord : TEXCOORD0;
	uint4 vBlendIndices : BLENDINDEX;
	float4 vBlendWeights : BLENDWEIGHT;
};

struct VS_OUT
{
	float4 vPosition : SV_POSITION;
	float3 vNormal : NORMAL;
	float3 vTangent : TANGENT;
	float3 vBinormal : BINORMAL;
	float2 vTexcoord : TEXCOORD0;
	float3 vWorldPosition : TEXCOORD1;
};

VS_OUT VS_MAIN(VS_IN input)
{
	VS_OUT output;
	const matrix boneMatrix =
		g_BoneMatrices[input.vBlendIndices.x] * input.vBlendWeights.x +
		g_BoneMatrices[input.vBlendIndices.y] * input.vBlendWeights.y +
		g_BoneMatrices[input.vBlendIndices.z] * input.vBlendWeights.z +
		g_BoneMatrices[input.vBlendIndices.w] * input.vBlendWeights.w;
	const float4 skinnedPosition =
		mul(float4(input.vPosition, 1.f), boneMatrix);
	const float3 skinnedNormal =
		mul(float4(input.vNormal, 0.f), boneMatrix).xyz;
	const float3 skinnedTangent =
		mul(float4(input.vTangent, 0.f), boneMatrix).xyz;
	const float3 skinnedBinormal =
		mul(float4(input.vBinormal, 0.f), boneMatrix).xyz;

	const matrix worldView = mul(g_WorldMatrix, g_ViewMatrix);
	const matrix worldViewProjection = mul(worldView, g_ProjMatrix);

	output.vPosition = mul(skinnedPosition, worldViewProjection);
	output.vNormal = normalize(mul(float4(skinnedNormal, 0.f), g_WorldMatrix).xyz);
	output.vTangent = normalize(mul(float4(skinnedTangent, 0.f), g_WorldMatrix).xyz);
	output.vBinormal = normalize(mul(float4(skinnedBinormal, 0.f), g_WorldMatrix).xyz);
	output.vTexcoord = input.vTexcoord;
	output.vWorldPosition = mul(skinnedPosition, g_WorldMatrix).xyz;
	return output;
}

float4 PS_MAIN(VS_OUT input) : SV_TARGET0
{
	const float4 albedo = g_DiffuseTexture.Sample(LinearSampler, input.vTexcoord);
	if (albedo.a < 0.3f)
		discard;

	float3 normal = normalize(input.vNormal);
	if (0 != g_HasNormalTexture)
	{
		const float3 tangentNormal =
			g_NormalTexture.Sample(LinearSampler, input.vTexcoord).xyz * 2.f - 1.f;
		const float3x3 tangentToWorld = float3x3(
			normalize(input.vTangent),
			normalize(input.vBinormal) * -1.f,
			normal);
		normal = normalize(mul(tangentNormal, tangentToWorld));
	}

	const float3 light = normalize(-g_LightDirection);
	const float diffuseLight = saturate(dot(normal, light));
	const float hemisphere = 0.38f + saturate(normal.y) * 0.18f;
	const float3 viewDirection = normalize(g_CameraPosition - input.vWorldPosition);
	const float rim = pow(1.f - saturate(dot(normal, viewDirection)), 3.f) * 0.12f;

	const float3 color = albedo.rgb * (hemisphere + diffuseLight * 0.72f) + rim;
	return float4(color, albedo.a);
}

technique11 DefaultTechnique
{
	pass PreviewPass
	{
		SetRasterizerState(RS_Preview);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN();
	}
}
```

### Tools/EffectToolV2/build_texture_slot_usage.py

```python
"""Build Data/Effects/V2/TextureSlotUsage.v1.json for Effect Tool v2.

Scans every imported/authored Effect document and records, per texture file name,
how many times it was bound to each material slot (base/noise/mask/emissive/dissolve)
and which original material parameter names referenced it.

Usage (Blender bundled python is fine):
    python Tools/EffectToolV2/build_texture_slot_usage.py
"""
import collections
import glob
import json
import os
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
OUTPUT = os.path.join(ROOT, "Data", "Effects", "V2", "TextureSlotUsage.v1.json")
SOURCES = [
    "Data/Effects/Imported/*/Converted/*.imported.effect.json",
    "Data/Effects/Authored/*.json",
]
SLOT_ALIASES = {
    "base": "base", "base2": "base",
    "noise": "noise", "noise2": "noise",
    "mask": "mask", "mask2": "mask",
    "emissive": "emissive",
    "dissolve": "dissolve",
}


def walk(node, slots, params):
    if isinstance(node, dict):
        asset = node.get("assetId")
        if isinstance(asset, str) and asset.lower().endswith(".dds"):
            name = os.path.basename(asset).lower()
            slot = SLOT_ALIASES.get(str(node.get("slotId", "")).lower())
            if slot:
                slots[name][slot] += 1
            param = node.get("name")
            if isinstance(param, str) and "sourceObjectPath" in node:
                params[name][param.lower()] += 1
        for value in node.values():
            walk(value, slots, params)
    elif isinstance(node, list):
        for value in node:
            walk(value, slots, params)


def main():
    slots = collections.defaultdict(collections.Counter)
    params = collections.defaultdict(collections.Counter)
    files = []
    for pattern in SOURCES:
        files.extend(sorted(glob.glob(os.path.join(ROOT, pattern))))
    parsed = 0
    for path in files:
        try:
            with open(path, encoding="utf-8") as handle:
                walk(json.load(handle), slots, params)
            parsed += 1
        except (OSError, ValueError) as error:
            print(f"skip {path}: {error}", file=sys.stderr)

    names = sorted(set(slots) | set(params))
    textures = {}
    for name in names:
        entry = {slot: count for slot, count in sorted(slots[name].items())}
        if params[name]:
            entry["params"] = [p for p, _ in params[name].most_common()]
        textures[name] = entry

    document = {
        "schema": "lostark.effect-tool-v2.texture-slot-usage",
        "formatVersion": 1,
        "sourcePatterns": SOURCES,
        "sourceDocumentCount": parsed,
        "slots": ["base", "noise", "mask", "emissive", "dissolve"],
        "textures": textures,
    }
    os.makedirs(os.path.dirname(OUTPUT), exist_ok=True)
    with open(OUTPUT, "w", encoding="utf-8", newline="\n") as handle:
        json.dump(document, handle, ensure_ascii=False, indent=1)
        handle.write("\n")
    print(f"wrote {OUTPUT}: {len(textures)} textures from {parsed} documents")


if __name__ == "__main__":
    main()
```

### 기존 파일 변경 (MainApp.h / MainApp.cpp / Client.vcxproj / Client.vcxproj.filters, 누적 diff vs HEAD)

```diff
diff --git a/Client/Default/Client.vcxproj b/Client/Default/Client.vcxproj
index 8a692e5c..b0b09706 100644
--- a/Client/Default/Client.vcxproj
+++ b/Client/Default/Client.vcxproj
@@ -121,6 +121,7 @@
     <ClInclude Include="..\Public\Client_Defines.h" />
     <ClInclude Include="..\public\Effect_Tool.h" />
     <ClInclude Include="..\Public\Effect_Tool_V2.h" />
+    <ClInclude Include="..\Public\Effect_Preview_V2.h" />
     <ClInclude Include="..\Public\HUDLayoutTool.h" />
     <ClInclude Include="..\Public\Level_Loading.h" />
     <ClInclude Include="..\public\Loader.h" />
@@ -250,6 +251,7 @@
       <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
     </ClCompile>
     <ClCompile Include="..\Private\Effect_Tool_V2.cpp" />
+    <ClCompile Include="..\Private\Effect_Preview_V2.cpp" />
     <ClCompile Include="..\Private\HUDLayoutTool.cpp" />
     <ClCompile Include="..\Private\Level_Loading.cpp" />
     <ClCompile Include="..\private\Loader.cpp" />
diff --git a/Client/Default/Client.vcxproj.filters b/Client/Default/Client.vcxproj.filters
index d40493c0..a01b058e 100644
--- a/Client/Default/Client.vcxproj.filters
+++ b/Client/Default/Client.vcxproj.filters
@@ -381,6 +381,9 @@
     <ClCompile Include="..\Private\Effect_Tool_V2.cpp">
       <Filter>03. Tools\06. Effect V2</Filter>
     </ClCompile>
+    <ClCompile Include="..\Private\Effect_Preview_V2.cpp">
+      <Filter>03. Tools\06. Effect V2</Filter>
+    </ClCompile>
     <ClCompile Include="..\Private\Valtan.cpp">
       <Filter>02.GameObjects\01. Boss</Filter>
     </ClCompile>
@@ -715,6 +718,9 @@
     <ClInclude Include="..\Public\Effect_Tool_V2.h">
       <Filter>03. Tools\06. Effect V2</Filter>
     </ClInclude>
+    <ClInclude Include="..\Public\Effect_Preview_V2.h">
+      <Filter>03. Tools\06. Effect V2</Filter>
+    </ClInclude>
     <ClInclude Include="..\Public\Valtan.h">
       <Filter>02.GameObjects\01. Boss</Filter>
     </ClInclude>
```
