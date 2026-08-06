#include "imgui.h"

#include "HUDRuntimeView.h"
#include "DataJson.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"

#include <fstream>
#include <cmath>

namespace
{
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

Client::CHUDRuntimeView::CHUDRuntimeView(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext,
	const wstring& strDocumentPath, const DRAW_TARGET eDrawTarget)
	: m_strDocumentPath{ strDocumentPath }
	, m_eDrawTarget{ eDrawTarget }
	, m_pDevice{ pDevice }
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
	const filesystem::path DataPath = CProjectDataRoot::Resolve(m_strDocumentPath);

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

		if (const DATA_JSON_VALUE* pAnimation = SlotValue.Find("animation"))
		{
			if (const DATA_JSON_VALUE* pFps = pAnimation->Find("fps"))
				if (pFps->Is_Number() && pFps->Get_Number() > 0.0)
					Slot.fAnimationFPS = static_cast<f32_t>(pFps->Get_Number());
			if (const DATA_JSON_VALUE* pFrames = pAnimation->Find("frames"))
			{
				if (pFrames->Is_Array())
				{
					for (const DATA_JSON_VALUE& FrameValue : pFrames->Get_Array())
						if (FrameValue.Is_String() && !FrameValue.Get_String().empty())
							Slot.AnimationFrames.push_back(FrameValue.Get_String());
				}
			}
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
	ImDrawList* pDrawList;
	switch (m_eDrawTarget)
	{
	case DRAW_TARGET::CURRENT_WINDOW:
		pDrawList = ImGui::GetWindowDrawList();
		break;
	case DRAW_TARGET::BACKGROUND:
		pDrawList = ImGui::GetBackgroundDrawList(pViewport);
		break;
	case DRAW_TARGET::FOREGROUND:
	default:
		pDrawList = ImGui::GetForegroundDrawList(pViewport);
		break;
	}

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

		/* A flipbook slot (login/lobby background frame sequences, ...) plays instead of
		drawing Layers -- ImGui::GetTime() is already the monotonic clock every other draw
		call this frame reads, so the whole HUD stays on one time source. Looping (modulo
		frame count) instead of clamping is deliberate: a background sequence is meant to
		keep playing for as long as this view is on screen. */
		if (!Slot.AnimationFrames.empty())
		{
			const double dElapsedSeconds = ImGui::GetTime();
			const size_t iFrameCount = Slot.AnimationFrames.size();
			const size_t iFrameIndex = static_cast<size_t>(
				dElapsedSeconds * static_cast<double>(Slot.fAnimationFPS)) % iFrameCount;

			ID3D11ShaderResourceView* pSRV =
				Get_Or_Load_Texture(Slot.AnimationFrames[iFrameIndex]);
			if (nullptr != pSRV)
				Draw_Image_Quad(pDrawList, pSRV, Corners, IM_COL32(255, 255, 255, 255), false, false);
			continue;
		}

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
