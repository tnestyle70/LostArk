# 2026-08-05 UI Display Colorspace Fix PLAN

## G00. UI display-space texture load correction

### 1. C1~C8 관점

- **C1 기준계:** sRGB 파일 값, 선형 shader sample, UNORM back buffer 사이의 색공간 변환을 한 번만 수행한다.
- C2 이동>계산: UI는 lighting 계산 대상이 아니므로 runtime shader에서 tone mapping하지 않는다.
- C3 공유는 비싸다: 새 texture manager나 두 번째 UI runtime을 만들지 않는다.
- C4 수명은 선언된다: 기존 `CTexture` Prototype과 `CHUDRuntimeView` cache 수명을 유지한다.
- C5 이산화와 오차: 8-bit UI 색을 display-encoded 값으로 보존한다.
- C6 가지치기: 현재 실제 UI consumer 세 경로만 수정한다.
- **C7 권위와 정합성:** UI PNG/DDS의 display-space 픽셀과 `Data/UI`의 tint가 presentation 정본이다.
- **C8 검증이 병목:** 두 loader의 SRV format 선택, Engine/Client build, frontend smoke와 ProjectAudit로 종료한다.

### 2. 문제 해결 ①~⑤

① 문제·제약: UI PNG는 sRGB/gAMA metadata를 가지지만 UI는 tone mapping 이후 UNORM back buffer에 그려져, 자동 sRGB decode 뒤 encode가 빠져 어둡게 보인다. 기존 Prototype/Clone, HUD cache, blend 순서는 유지한다.

② 단순 해법의 문제: UI shader에 무조건 `pow(1/2.2)`를 넣으면 non-sRGB DDS와 ImGui font/widget까지 이중 보정될 수 있고, swap-chain 전체를 sRGB RTV로 바꾸면 scene final gamma 계약까지 흔든다.

③ 해결 방식: display-space UI를 읽는 기존 `CTexture`, `CHUDRuntimeView`, `CHUDLayoutTool` loader가 DDS/WIC metadata의 sRGB view 승격을 무시하고 UNORM SRV를 만들게 한다. shader와 blend는 그대로 둔다.

④ 비교: 월드 `CMaterial`은 sRGB texture를 선형 lighting에 사용하므로 기존 force-sRGB 계약을 유지한다. 이번 G는 lighting을 거치지 않는 UI presentation 경로만 다룬다.

⑤ 대가: UI loader 세 곳이 같은 load flag를 명시한다. UI texture service를 새로 만드는 비용보다 작으며, 향후 `CTexture`에 비-UI consumer가 생기면 명시적 color-space enum으로 승격한다.

### 3. 자료구조·알고리즘 핵심

- 입력: `Resources/UI/...`의 PNG/DDS와 기존 `Data/UI` Resources-relative asset ID.
- 런타임 객체: loading chrome은 `CTexture` Prototype → `CUI_Sprite` Clone, combat HUD는 `CHUDRuntimeView::m_TextureCache`가 SRV를 보유한다.
- 불변식: UI texture는 display-encoded byte를 그대로 sample하며 scene final gamma를 다시 통과하지 않는다.
- 실패 처리: 기존 missing/corrupt asset 격리 동작을 유지하고 색공간 flag만 바꾼다.
- 복잡도: load와 cache 횟수, draw call 수, memory 사용량은 기존과 동일하다.

### 4. 파일 목록

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 수정 | C:/Users/user/Desktop/LostArk/Engine/Private/Texture.cpp | loading `CUI_Sprite`가 소비하는 texture Prototype을 display-space UNORM SRV로 로드 |
| 수정 | C:/Users/user/Desktop/LostArk/Client/Private/HUDRuntimeView.cpp | ImGui foreground HUD texture cache를 display-space UNORM SRV로 로드 |
| 수정 | C:/Users/user/Desktop/LostArk/Client/Private/HUDLayoutTool.cpp | authoring canvas와 Debug runtime preview texture cache를 제품 HUD와 같은 display-space UNORM SRV로 로드 |
| 추가 | C:/Users/user/Desktop/LostArk/.md/GB/08-05/2026-08-05_UI_DISPLAY_COLORSPACE_FIX_RESULT.md | 실제 diff, 자동 검증, 수동 검증과 남은 경계 기록 |

새 C++ 파일과 public header 변경이 없으므로 `.vcxproj`와 `.vcxproj.filters` 변경은 없다. 기존 등록은 각각 `Engine/Default/Engine.vcxproj`, `Client/Default/Client.vcxproj`와 대응 filters에서 확인한다.

### 5-1. C:/Users/user/Desktop/LostArk/Engine/Private/Texture.cpp

변경 종류: 전체 교체

적용 위치: `CTexture::Initialize_Prototype`의 DDS/WIC load 분기. 그 외 코드는 현재 파일과 동일하게 유지한다.

```cpp
#include "Texture.h"

#include "GameInstance.h"

CTexture::CTexture(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CComponent { pDevice, pContext }
{
}


CTexture::~CTexture()
{
}

HRESULT CTexture::Initialize_Prototype(const tchar_t* pTextureFilePath, uint32_t iNumTextures)
{
	m_iNumTextures = iNumTextures;

	m_Textures.reserve(iNumTextures);

	for (size_t i = 0; i < m_iNumTextures; i++)
	{

		ComPtr<ID3D11ShaderResourceView>		pSRV = { nullptr };

		tchar_t			szTextureFilePath[MAX_PATH] = {};

		wsprintf(szTextureFilePath, pTextureFilePath, i);

		tchar_t			szEXT[MAX_PATH] = {};

		_wsplitpath_s(szTextureFilePath, nullptr, 0, nullptr, 0, nullptr, 0, szEXT, MAX_PATH);

		HRESULT		hr = {};

		/* CTexture currently backs display-space UI sprites.  Preserve the authored
		   bytes because these sprites render after scene gamma into an UNORM back buffer. */
		if (false == lstrcmp(szEXT, TEXT(".dds")))
		{
			hr = CreateDDSTextureFromFileEx(
				m_pDevice.Get(), szTextureFilePath, 0,
				D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE,
				0, 0, DDS_LOADER_IGNORE_SRGB, nullptr, &pSRV);
		}
		else if (false == lstrcmp(szEXT, TEXT(".tga")))
		{
			hr = E_FAIL;
		}
		else
		{
			hr = CreateWICTextureFromFileEx(
				m_pDevice.Get(), szTextureFilePath, 0,
				D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE,
				0, 0, WIC_LOADER_IGNORE_SRGB, nullptr, &pSRV);
		}

		m_Textures.push_back(move(pSRV));
	}

	m_SRVs.resize(m_iNumTextures);

	return S_OK;
}

HRESULT CTexture::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CTexture::Bind_ShaderResource(shared_ptr<class CShader> pShader, const char_t* pConstantName, uint32_t iTextureIndex)
{
	if (iTextureIndex >= m_iNumTextures)
		return E_FAIL;

	return pShader->Bind_Texture(pConstantName, m_Textures[iTextureIndex]);
}

HRESULT CTexture::Bind_ShaderResources(shared_ptr<class CShader> pShader, const char_t* pConstantName)
{
	uint32_t		iNumTextures = {};

	for (auto& pTexture : m_Textures)
		m_SRVs[iNumTextures++] = pTexture.Get();

	return pShader->Bind_Textures(pConstantName, &m_SRVs.front(), m_iNumTextures);
}


unique_ptr<CTexture> CTexture::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const tchar_t* pTextureFilePath, uint32_t iNumTextures)
{
	auto pInstance = unique_ptr<CTexture>(new CTexture(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype(pTextureFilePath, iNumTextures)))
	{
		MSG_BOX("Failed to Created : CTexture");
		return nullptr;
	}

	return pInstance;
}

shared_ptr<CPrototype> CTexture::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CTexture>(new CTexture(*this));

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CTexture");
		return nullptr;
	}

	return pInstance;
}
```

### 5-2. C:/Users/user/Desktop/LostArk/Client/Private/HUDRuntimeView.cpp

변경 종류: 전체 교체

적용 위치: `CHUDRuntimeView::Get_Or_Load_Texture`의 DDS/WIC load 분기. 그 외 코드는 현재 파일과 동일하게 유지한다.

```cpp
#include "imgui.h"

#include "HUDRuntimeView.h"
#include "DataJson.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"

#include <fstream>
#include <cmath>

namespace
{
	/* Reference resolution the layout was authored at (matches Data/UI/HUD/HUD_Layout.json). */
	constexpr const wchar_t* HUD_LAYOUT_RELATIVE_PATH = L"UI/HUD/HUD_Layout.json";

	void Get_Rotated_Rect_Corners(const ImVec2& vTopLeft, const ImVec2& vBotRight, float fDegrees, ImVec2 outCorners[4])
	{
		outCorners[0] = vTopLeft;
		outCorners[1] = ImVec2(vBotRight.x, vTopLeft.y);
		outCorners[2] = vBotRight;
		outCorners[3] = ImVec2(vTopLeft.x, vBotRight.y);

		if (0.f == fDegrees)
			return;

		const ImVec2 vCenter((vTopLeft.x + vBotRight.x) * 0.5f, (vTopLeft.y + vBotRight.y) * 0.5f);
		const float fRadians = fDegrees * (3.14159265f / 180.f);
		const float fCos = cosf(fRadians);
		const float fSin = sinf(fRadians);

		for (int32_t i = 0; i < 4; ++i)
		{
			const float fDX = outCorners[i].x - vCenter.x;
			const float fDY = outCorners[i].y - vCenter.y;
			outCorners[i].x = vCenter.x + fDX * fCos - fDY * fSin;
			outCorners[i].y = vCenter.y + fDX * fSin + fDY * fCos;
		}
	}
}

Client::CHUDRuntimeView::CHUDRuntimeView(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
{
	/* Same additive blend state as CHUDLayoutTool, so glow/particle layers (bAdditive) read
	identically in the runtime view as they do in the editor preview. */
	D3D11_BLEND_DESC BlendDesc{};
	BlendDesc.RenderTarget[0].BlendEnable = TRUE;
	BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	m_pDevice->CreateBlendState(&BlendDesc, &m_pAdditiveBlendState);

	Load();
}

Client::CHUDRuntimeView::~CHUDRuntimeView()
{
}

HRESULT Client::CHUDRuntimeView::Load()
{
	const filesystem::path DataPath = CProjectDataRoot::Resolve(HUD_LAYOUT_RELATIVE_PATH);

	ifstream Stream(DataPath, ios::binary);
	if (!Stream.is_open())
		return S_OK;

	const string Text(
		(istreambuf_iterator<char>(Stream)),
		istreambuf_iterator<char>());

	DATA_JSON_VALUE Root;
	string Error;
	if (!CDataJson::Parse(Text, Root, Error) || !Root.Is_Object())
		return S_OK;

	if (const DATA_JSON_VALUE* pResolution = Root.Find("resolution"))
	{
		if (const DATA_JSON_VALUE* pWidth = pResolution->Find("width"))
			if (pWidth->Is_Number())
				m_fResolutionWidth = static_cast<f32_t>(pWidth->Get_Number());
		if (const DATA_JSON_VALUE* pHeight = pResolution->Find("height"))
			if (pHeight->Is_Number())
				m_fResolutionHeight = static_cast<f32_t>(pHeight->Get_Number());
	}

	const DATA_JSON_VALUE* pSlots = Root.Find("slots");
	if (nullptr == pSlots || !pSlots->Is_Array())
		return S_OK;

	for (const DATA_JSON_VALUE& SlotValue : pSlots->Get_Array())
	{
		const DATA_JSON_VALUE* pRect = SlotValue.Find("rect");
		if (nullptr == pRect || !pRect->Is_Object())
			continue;

		const DATA_JSON_VALUE* pX = pRect->Find("x");
		const DATA_JSON_VALUE* pY = pRect->Find("y");
		const DATA_JSON_VALUE* pWidth = pRect->Find("width");
		const DATA_JSON_VALUE* pHeight = pRect->Find("height");
		if (nullptr == pX || nullptr == pY || nullptr == pWidth || nullptr == pHeight)
			continue;

		HUD_SLOT Slot{};
		Slot.fX = static_cast<f32_t>(pX->Get_Number());
		Slot.fY = static_cast<f32_t>(pY->Get_Number());
		Slot.fSizeX = static_cast<f32_t>(pWidth->Get_Number());
		Slot.fSizeY = static_cast<f32_t>(pHeight->Get_Number());

		if (const DATA_JSON_VALUE* pOwnerClass = SlotValue.Find("ownerClass"))
			if (pOwnerClass->Is_String())
				Slot.strOwnerClass = pOwnerClass->Get_String();

		if (const DATA_JSON_VALUE* pRotation = SlotValue.Find("rotation"))
			if (pRotation->Is_Number())
				Slot.fRotation = static_cast<f32_t>(pRotation->Get_Number());

		if (const DATA_JSON_VALUE* pStages = SlotValue.Find("stages"))
		{
			if (const DATA_JSON_VALUE* pBaseFrom = pStages->Find("baseFrom"))
				if (pBaseFrom->Is_Number())
					Slot.iBaseFromStage = static_cast<int32_t>(pBaseFrom->Get_Number());
			if (const DATA_JSON_VALUE* pShineFrom = pStages->Find("shineFrom"))
				if (pShineFrom->Is_Number())
					Slot.iShineFromStage = static_cast<int32_t>(pShineFrom->Get_Number());
		}

		if (const DATA_JSON_VALUE* pLayers = SlotValue.Find("layers"))
		{
			if (pLayers->Is_Array())
			{
				for (const DATA_JSON_VALUE& LayerValue : pLayers->Get_Array())
				{
					const DATA_JSON_VALUE* pPath = LayerValue.Find("path");
					if (nullptr == pPath || !pPath->Is_String() || pPath->Get_String().empty())
						continue;

					TEXTURE_LAYER Layer{};
					Layer.strPath = pPath->Get_String();

					if (const DATA_JSON_VALUE* pTint = LayerValue.Find("tint"))
					{
						if (pTint->Is_Array() && 4u == pTint->Get_Array().size())
						{
							for (size_t i = 0; i < 4; ++i)
								Layer.vTint[i] = static_cast<f32_t>(pTint->Get_Array()[i].Get_Number());
						}
					}
					if (const DATA_JSON_VALUE* pAdditive = LayerValue.Find("additive"))
						if (pAdditive->Is_Boolean())
							Layer.bAdditive = pAdditive->Get_Boolean();
					if (const DATA_JSON_VALUE* pFlipX = LayerValue.Find("flipX"))
						if (pFlipX->Is_Boolean())
							Layer.bFlipX = pFlipX->Get_Boolean();

					Slot.Layers.push_back(move(Layer));
				}
			}
		}

		if (const DATA_JSON_VALUE* pShine = SlotValue.Find("shine"))
		{
			if (const DATA_JSON_VALUE* pTexture = pShine->Find("texture"))
				if (pTexture->Is_String())
					Slot.strShineTexture = pTexture->Get_String();
			if (const DATA_JSON_VALUE* pAdditive = pShine->Find("additive"))
				if (pAdditive->Is_Boolean())
					Slot.bShineAdditive = pAdditive->Get_Boolean();
		}

		m_Slots.push_back(move(Slot));
	}

	return S_OK;
}

ID3D11ShaderResourceView* Client::CHUDRuntimeView::Get_Or_Load_Texture(const string& strPath)
{
	if (strPath.empty())
		return nullptr;

	const auto Iter = m_TextureCache.find(strPath);
	if (m_TextureCache.end() != Iter)
		return Iter->second.Get();

	const u8string Utf8Path(strPath.begin(), strPath.end());
	const filesystem::path ResolvedPath = CRuntimeAssetRoot::Resolve(filesystem::path(Utf8Path));
	if (ResolvedPath.empty())
		return nullptr;

	ComPtr<ID3D11ShaderResourceView> pSRV = { nullptr };

	HRESULT hr = {};
	/* ImGui writes this HUD after scene gamma into an UNORM back buffer.  Keep the
	   authored display values instead of letting the loaders decode them to linear. */
	if (0 == _wcsicmp(ResolvedPath.extension().c_str(), L".dds"))
	{
		hr = CreateDDSTextureFromFileEx(
			m_pDevice.Get(), ResolvedPath.c_str(), 0,
			D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE,
			0, 0, DDS_LOADER_IGNORE_SRGB, nullptr, &pSRV);
	}
	else
	{
		hr = CreateWICTextureFromFileEx(
			m_pDevice.Get(), ResolvedPath.c_str(), 0,
			D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE,
			0, 0, WIC_LOADER_IGNORE_SRGB, nullptr, &pSRV);
	}

	m_TextureCache[strPath] = pSRV;

	return FAILED(hr) ? nullptr : pSRV.Get();
}

void Client::CHUDRuntimeView::Enable_Additive_Blend(const ImDrawList* pParentList, const ImDrawCmd* pCmd)
{
	auto pView = static_cast<CHUDRuntimeView*>(pCmd->UserCallbackData);
	pView->m_pContext->OMSetBlendState(pView->m_pAdditiveBlendState.Get(), nullptr, 0xffffffff);
}

void Client::CHUDRuntimeView::Draw_Image_Quad(ImDrawList* pDrawList, ID3D11ShaderResourceView* pSRV,
	const ImVec2 Corners[4], uint32_t iTint, bool_t bAdditive, bool_t bFlipX)
{
	if (bAdditive)
		pDrawList->AddCallback(&CHUDRuntimeView::Enable_Additive_Blend, this);

	if (bFlipX)
		pDrawList->AddImageQuad(pSRV, Corners[0], Corners[1], Corners[2], Corners[3],
			ImVec2(1, 0), ImVec2(0, 0), ImVec2(0, 1), ImVec2(1, 1), iTint);
	else
		pDrawList->AddImageQuad(pSRV, Corners[0], Corners[1], Corners[2], Corners[3],
			ImVec2(0, 0), ImVec2(1, 0), ImVec2(1, 1), ImVec2(0, 1), iTint);

	if (bAdditive)
		pDrawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
}

void Client::CHUDRuntimeView::Render(const string& strOwnerClass, int32_t iStage)
{
	if (m_Slots.empty())
		return;

	ImGuiViewport* pViewport = ImGui::GetMainViewport();
	ImDrawList* pDrawList = ImGui::GetForegroundDrawList(pViewport);

	const float fScaleX = pViewport->WorkSize.x / m_fResolutionWidth;
	const float fScaleY = pViewport->WorkSize.y / m_fResolutionHeight;

	for (const HUD_SLOT& Slot : m_Slots)
	{
		if (!Slot.strOwnerClass.empty() && Slot.strOwnerClass != strOwnerClass)
			continue;
		if (iStage < Slot.iBaseFromStage)
			continue;

		const ImVec2 vTopLeft(
			pViewport->WorkPos.x + Slot.fX * fScaleX,
			pViewport->WorkPos.y + Slot.fY * fScaleY);
		const ImVec2 vBotRight(
			vTopLeft.x + Slot.fSizeX * fScaleX,
			vTopLeft.y + Slot.fSizeY * fScaleY);

		ImVec2 Corners[4];
		Get_Rotated_Rect_Corners(vTopLeft, vBotRight, Slot.fRotation, Corners);

		for (const TEXTURE_LAYER& Layer : Slot.Layers)
		{
			ID3D11ShaderResourceView* pSRV = Get_Or_Load_Texture(Layer.strPath);
			if (nullptr == pSRV)
				continue;

			const ImU32 iTint = ImGui::ColorConvertFloat4ToU32(
				ImVec4(Layer.vTint[0], Layer.vTint[1], Layer.vTint[2], Layer.vTint[3]));

			Draw_Image_Quad(pDrawList, pSRV, Corners, iTint, Layer.bAdditive, Layer.bFlipX);
		}

		if (iStage >= Slot.iShineFromStage && !Slot.strShineTexture.empty())
		{
			ID3D11ShaderResourceView* pShineSRV = Get_Or_Load_Texture(Slot.strShineTexture);
			if (nullptr != pShineSRV)
				Draw_Image_Quad(pDrawList, pShineSRV, Corners, IM_COL32(255, 255, 255, 255), Slot.bShineAdditive, false);
		}
	}
}
```

### 5-3. C:/Users/user/Desktop/LostArk/Client/Private/HUDLayoutTool.cpp

변경 종류: 함수 교체

적용 위치: `CHUDLayoutTool::Get_Or_Load_Texture` 함수 전체를 교체한다. 이 함수의 caller인 palette, canvas, inspector preview와 Debug runtime preview는 변경하지 않는다.

```cpp
ID3D11ShaderResourceView* Client::CHUDLayoutTool::Get_Or_Load_Texture(const string& strPath)
{
	if (strPath.empty())
		return nullptr;

	auto Iter = m_TextureCache.find(strPath);
	if (m_TextureCache.end() != Iter)
		return Iter->second.Get();

	const u8string utf8Path(strPath.begin(), strPath.end());
	const filesystem::path resolvedPath =
		CRuntimeAssetRoot::Resolve(filesystem::path(utf8Path));
	if (resolvedPath.empty())
		return nullptr;
	const filesystem::path Ext = resolvedPath.extension();

	ComPtr<ID3D11ShaderResourceView> pSRV = { nullptr };

	HRESULT hr = {};
	/* The authoring canvas and Debug runtime preview share the product HUD's
	   display-space contract, so their cached SRVs must preserve authored bytes. */
	if (0 == _wcsicmp(Ext.c_str(), L".dds"))
	{
		hr = CreateDDSTextureFromFileEx(
			m_pDevice.Get(), resolvedPath.c_str(), 0,
			D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE,
			0, 0, DDS_LOADER_IGNORE_SRGB, nullptr, &pSRV);
	}
	else
	{
		hr = CreateWICTextureFromFileEx(
			m_pDevice.Get(), resolvedPath.c_str(), 0,
			D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE,
			0, 0, WIC_LOADER_IGNORE_SRGB, nullptr, &pSRV);
	}

	m_TextureCache[strPath] = pSRV;

	return FAILED(hr) ? nullptr : pSRV.Get();
}
```

### 6. 프로젝트 등록

- `Engine/Private/Texture.cpp`, `Client/Private/HUDRuntimeView.cpp`, `Client/Private/HUDLayoutTool.cpp`는 기존 `.vcxproj` 및 `.vcxproj.filters`에 등록되어 있다.
- 새 C++/shader/data 파일이 없으므로 프로젝트 XML은 수정하지 않는다.

### 7. 적용 순서와 검증

1. `Engine/Private/Texture.cpp`의 DDS/WIC standard loader를 Ex loader + `*_LOADER_IGNORE_SRGB`로 교체한다.
2. `Client/Private/HUDRuntimeView.cpp`의 cache loader를 같은 display-space 계약으로 교체한다.
3. `Client/Private/HUDLayoutTool.cpp`의 cache loader를 같은 display-space 계약으로 교체한다.
4. `Engine x64 Debug/Release`를 빌드하고 각 구성에서 `UpdateLib.bat`를 실행한다.
5. `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug`와 `-Configuration Release`를 실행한다.
6. `Tools/ProjectAudit/Invoke-ProjectAudit.ps1`와 `git diff --check`를 실행한다.
7. Client를 `Client/Default` working directory로 실행해 loading background/chrome, combat HUD, HUD Layout Tool canvas가 원본 PNG 밝기로 보이고, world scene tone mapping이 변하지 않았는지 확인한다.
8. 반투명 가장자리만 남아 어두우면 별도 premultiplied-alpha 진단으로 분리하고 이번 색공간 G에는 섞지 않는다.
