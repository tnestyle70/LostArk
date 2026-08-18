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
		if (const DATA_JSON_VALUE* pId = SlotValue.Find("id"))
			if (pId->Is_String())
				Slot.strId = pId->Get_String();

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
			if (const DATA_JSON_VALUE* pLoop = pAnimation->Find("loop"))
				if (pLoop->Is_Boolean())
					Slot.bAnimationLoop = pLoop->Get_Boolean();
		}

		if (const DATA_JSON_VALUE* pKeyframePath = SlotValue.Find("keyframeAnimationPath"))
			if (pKeyframePath->Is_String())
				Slot.strKeyframeAnimationPath = pKeyframePath->Get_String();
		if (const DATA_JSON_VALUE* pKeyframeScale = SlotValue.Find("keyframeAnimationScale"))
			if (pKeyframeScale->Is_Number() && pKeyframeScale->Get_Number() > 0.0)
				Slot.fKeyframeAnimationScale = static_cast<f32_t>(pKeyframeScale->Get_Number());

		m_Slots.push_back(move(Slot));
	}

	return S_OK;
}

ID3D11ShaderResourceView* Client::CHUDRuntimeView::Load_Texture(const string& strPath)
{
	return Get_Or_Load_Texture(strPath);
}

bool_t Client::CHUDRuntimeView::Get_SlotRect(const string& strId, f32_t& fX, f32_t& fY, f32_t& fWidth, f32_t& fHeight) const
{
	for (const HUD_SLOT& Slot : m_Slots)
	{
		if (Slot.strId != strId)
			continue;

		fX = Slot.fX;
		fY = Slot.fY;
		fWidth = Slot.fSizeX;
		fHeight = Slot.fSizeY;
		return true;
	}

	return false;
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

	TEXTURE_SIZE Size{};
	if (SUCCEEDED(hr) && nullptr != pSRV)
	{
		ComPtr<ID3D11Resource> pResource;
		pSRV->GetResource(&pResource);
		ComPtr<ID3D11Texture2D> pTexture2D;
		if (SUCCEEDED(pResource.As(&pTexture2D)))
		{
			D3D11_TEXTURE2D_DESC Desc{};
			pTexture2D->GetDesc(&Desc);
			Size.fWidth = static_cast<f32_t>(Desc.Width);
			Size.fHeight = static_cast<f32_t>(Desc.Height);
		}
	}
	m_TextureSizeCache[strPath] = Size;

	return FAILED(hr) ? nullptr : pSRV.Get();
}

Client::CHUDRuntimeView::TEXTURE_SIZE Client::CHUDRuntimeView::Get_Texture_Size(const string& strPath)
{
	Get_Or_Load_Texture(strPath);
	const auto Iter = m_TextureSizeCache.find(strPath);
	return m_TextureSizeCache.end() != Iter ? Iter->second : TEXTURE_SIZE{};
}

const Client::CHUDRuntimeView::KEYFRAME_ANIM_DOCUMENT*
Client::CHUDRuntimeView::Get_Or_Load_KeyframeAnimation(const string& strPath)
{
	if (strPath.empty())
		return nullptr;

	const auto Iter = m_KeyframeAnimationCache.find(strPath);
	if (m_KeyframeAnimationCache.end() != Iter)
		return Iter->second.isLoaded ? &Iter->second : nullptr;

	KEYFRAME_ANIM_DOCUMENT Document{};

	const u8string Utf8Path(strPath.begin(), strPath.end());
	const filesystem::path DataPath = CProjectDataRoot::Resolve(filesystem::path(Utf8Path));
	ifstream Stream(DataPath, ios::binary);
	if (Stream.is_open())
	{
		const string Text(
			(istreambuf_iterator<char>(Stream)),
			istreambuf_iterator<char>());

		DATA_JSON_VALUE Root;
		string Error;
		if (CDataJson::Parse(Text, Root, Error) && Root.Is_Object())
		{
			if (const DATA_JSON_VALUE* pFrameRate = Root.Find("frameRate"))
				if (pFrameRate->Is_Number() && pFrameRate->Get_Number() > 0.0)
					Document.fFrameRate = static_cast<f32_t>(pFrameRate->Get_Number());
			if (const DATA_JSON_VALUE* pFrameCount = Root.Find("frameCount"))
				if (pFrameCount->Is_Number())
					Document.iFrameCount = static_cast<int32_t>(pFrameCount->Get_Number());
			if (const DATA_JSON_VALUE* pLoop = Root.Find("loop"))
				if (pLoop->Is_Boolean())
					Document.bLoop = pLoop->Get_Boolean();

			if (const DATA_JSON_VALUE* pLabels = Root.Find("labels"))
			{
				if (pLabels->Is_Object())
				{
					for (const auto& Pair : pLabels->Get_Object())
						if (Pair.second.Is_Number())
							Document.Labels[Pair.first] = static_cast<int32_t>(Pair.second.Get_Number());
				}
			}

			if (const DATA_JSON_VALUE* pLayers = Root.Find("layers"))
			{
				if (pLayers->Is_Array())
				{
					for (const DATA_JSON_VALUE& LayerValue : pLayers->Get_Array())
					{
						const DATA_JSON_VALUE* pKeyframes = LayerValue.Find("keyframes");
						if (nullptr == pKeyframes || !pKeyframes->Is_Array())
							continue;

						KEYFRAME_ANIM_LAYER Layer{};
						for (const DATA_JSON_VALUE& KeyValue : pKeyframes->Get_Array())
						{
							KEYFRAME_ANIM_KEY Key{};
							if (const DATA_JSON_VALUE* pFrame = KeyValue.Find("frame"))
								if (pFrame->Is_Number())
									Key.iFrame = static_cast<int32_t>(pFrame->Get_Number());
							if (const DATA_JSON_VALUE* pAsset = KeyValue.Find("asset"))
								if (pAsset->Is_String())
									Key.strAsset = pAsset->Get_String();
							if (const DATA_JSON_VALUE* pX = KeyValue.Find("x"))
								if (pX->Is_Number())
									Key.fX = static_cast<f32_t>(pX->Get_Number());
							if (const DATA_JSON_VALUE* pY = KeyValue.Find("y"))
								if (pY->Is_Number())
									Key.fY = static_cast<f32_t>(pY->Get_Number());
							if (const DATA_JSON_VALUE* pScaleX = KeyValue.Find("scaleX"))
								if (pScaleX->Is_Number())
									Key.fScaleX = static_cast<f32_t>(pScaleX->Get_Number());
							if (const DATA_JSON_VALUE* pScaleY = KeyValue.Find("scaleY"))
								if (pScaleY->Is_Number())
									Key.fScaleY = static_cast<f32_t>(pScaleY->Get_Number());
							if (const DATA_JSON_VALUE* pRotation = KeyValue.Find("rotationDeg"))
								if (pRotation->Is_Number())
									Key.fRotationDeg = static_cast<f32_t>(pRotation->Get_Number());
							if (const DATA_JSON_VALUE* pAlpha = KeyValue.Find("alpha"))
								if (pAlpha->Is_Number())
									Key.fAlpha = static_cast<f32_t>(pAlpha->Get_Number());
							if (const DATA_JSON_VALUE* pAdditive = KeyValue.Find("additive"))
								if (pAdditive->Is_Boolean())
									Key.bAdditive = pAdditive->Get_Boolean();
							if (const DATA_JSON_VALUE* pFlipX = KeyValue.Find("flipX"))
								if (pFlipX->Is_Boolean())
									Key.bFlipX = pFlipX->Get_Boolean();
							Layer.Keys.push_back(move(Key));
						}
						Document.Layers.push_back(move(Layer));
					}
				}
			}

			Document.isLoaded = !Document.Layers.empty();
		}
	}

	auto InsertedIter = m_KeyframeAnimationCache.emplace(strPath, move(Document)).first;
	return InsertedIter->second.isLoaded ? &InsertedIter->second : nullptr;
}

bool_t Client::CHUDRuntimeView::Play_KeyframeAnimation(const string& strSlotId, const string& strLabel)
{
	for (HUD_SLOT& Slot : m_Slots)
	{
		if (Slot.strId != strSlotId || Slot.strKeyframeAnimationPath.empty())
			continue;

		const KEYFRAME_ANIM_DOCUMENT* pDocument =
			Get_Or_Load_KeyframeAnimation(Slot.strKeyframeAnimationPath);
		if (nullptr == pDocument)
			return false;

		const auto LabelIter = pDocument->Labels.find(strLabel);
		if (pDocument->Labels.end() == LabelIter)
			return false;

		const int32_t iStartFrame = LabelIter->second;
		int32_t iEndFrame = pDocument->iFrameCount;
		for (const auto& Pair : pDocument->Labels)
		{
			if (Pair.second > iStartFrame && Pair.second < iEndFrame)
				iEndFrame = Pair.second;
		}

		Slot.iKeyframeAnimWindowStart = iStartFrame;
		Slot.iKeyframeAnimWindowEnd = iEndFrame;
		Slot.dKeyframeAnimStartSeconds = ImGui::GetTime();

		/* TEMP DIAGNOSTIC (2026-08-14): confirms which window this call actually resolved to --
		in particular, whether a later label meant to cap a window early (e.g. LanceMaster's
		focus_settled) is really being found as the nearest-greater label, or whether iEndFrame is
		falling back to something else. Remove once the LanceMaster stance icon is confirmed
		stable. */
		{
			char pDebugLine[256];
			sprintf_s(pDebugLine,
				"[Play_KeyframeAnimation] slot=%s label=%s window=[%d,%d) frameCount=%d\n",
				strSlotId.c_str(), strLabel.c_str(), iStartFrame, iEndFrame, pDocument->iFrameCount);
			OutputDebugStringA(pDebugLine);
		}
		return true;
	}

	return false;
}

bool_t Client::CHUDRuntimeView::Set_SlotRotation(const string& strSlotId, f32_t fDegrees)
{
	for (HUD_SLOT& Slot : m_Slots)
	{
		if (Slot.strId != strSlotId)
			continue;

		Slot.fRotation = fDegrees;
		return true;
	}

	return false;
}

bool_t Client::CHUDRuntimeView::Set_SlotPosition(const string& strSlotId, f32_t fX, f32_t fY)
{
	for (HUD_SLOT& Slot : m_Slots)
	{
		if (Slot.strId != strSlotId)
			continue;

		Slot.fX = fX;
		Slot.fY = fY;
		return true;
	}

	return false;
}

bool_t Client::CHUDRuntimeView::Set_SlotVisible(const string& strSlotId, bool_t bVisible)
{
	for (HUD_SLOT& Slot : m_Slots)
	{
		if (Slot.strId != strSlotId)
			continue;

		Slot.bForceHidden = !bVisible;
		return true;
	}

	return false;
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

	for (HUD_SLOT& Slot : m_Slots)
	{
		if (!Slot.strOwnerClass.empty() && Slot.strOwnerClass != strOwnerClass)
			continue;
		if (iStage < Slot.iBaseFromStage)
			continue;
		if (Slot.bForceHidden)
			continue;

		const ImVec2 vTopLeft(
			pViewport->WorkPos.x + Slot.fX * fScaleX,
			pViewport->WorkPos.y + Slot.fY * fScaleY);
		const ImVec2 vBotRight(
			vTopLeft.x + Slot.fSizeX * fScaleX,
			vTopLeft.y + Slot.fSizeY * fScaleY);

		ImVec2 Corners[4];
		Get_Rotated_Rect_Corners(vTopLeft, vBotRight, Slot.fRotation, Corners);

		// TEMP DIAGNOSTIC: dump resolved slot rect for the LanceMaster gauge slots once/sec.
		if (Slot.strId.rfind("Lance_Id_Gauge", 0) == 0)
		{
			static double s_dLastGaugeLog = -1.0;
			const double dNow = ImGui::GetTime();
			if (dNow - s_dLastGaugeLog > 1.0)
			{
				char szBuf[256];
				sprintf_s(szBuf, "[GAUGE-DIAG] %s slotX=%.2f slotY=%.2f sizeX=%.2f sizeY=%.2f rot=%.2f -> topLeft=(%.1f,%.1f) botRight=(%.1f,%.1f)\n",
					Slot.strId.c_str(), Slot.fX, Slot.fY, Slot.fSizeX, Slot.fSizeY, Slot.fRotation,
					vTopLeft.x, vTopLeft.y, vBotRight.x, vBotRight.y);
				OutputDebugStringA(szBuf);
			}
		}

		/* A flipbook slot (login/lobby background frame sequences, ...) plays instead of
		drawing Layers. Playback is timed from this slot's own first Render() call (stamped
		into dAnimationStartSeconds below) rather than raw ImGui::GetTime(), so engine/asset
		init time before this view ever draws can't eat into or skip past a non-looping slot.
		Looping (modulo frame count) is for a background meant to keep playing for as long as
		this view is on screen; a non-looping slot (e.g. a boot logo intro) instead stops
		drawing itself once its last frame's duration has elapsed, which reveals whatever draws
		beneath it -- Lobby_Layout.json relies on this to uncover the title background once the
		intro finishes, so slot order there puts the intro after the background. */
		if (!Slot.AnimationFrames.empty())
		{
			if (Slot.dAnimationStartSeconds < 0.0)
				Slot.dAnimationStartSeconds = ImGui::GetTime();

			const double dElapsedSeconds = ImGui::GetTime() - Slot.dAnimationStartSeconds;
			const size_t iFrameCount = Slot.AnimationFrames.size();
			const double dFrameProgress = dElapsedSeconds * static_cast<double>(Slot.fAnimationFPS);

			size_t iFrameIndex = 0;
			if (Slot.bAnimationLoop)
			{
				iFrameIndex = static_cast<size_t>(dFrameProgress) % iFrameCount;
			}
			else
			{
				if (dFrameProgress >= static_cast<double>(iFrameCount))
					continue;
				iFrameIndex = static_cast<size_t>(dFrameProgress);
			}

			ID3D11ShaderResourceView* pSRV =
				Get_Or_Load_Texture(Slot.AnimationFrames[iFrameIndex]);
			if (nullptr != pSRV)
				Draw_Image_Quad(pDrawList, pSRV, Corners, IM_COL32(255, 255, 255, 255), false, false);
			continue;
		}

		/* A slot with a keyframe animation document (an extracted Scaleform identity-HUD asset,
		e.g. LanceMaster's stance-switch icon) draws that instead of Layers, once something has
		called Play_KeyframeAnimation() on it -- until then it draws nothing (Layers is
		intentionally empty for such a slot; see HUD_Layout.json). */
		if (!Slot.strKeyframeAnimationPath.empty())
		{
			if (Slot.dKeyframeAnimStartSeconds < 0.0)
				continue;

			const KEYFRAME_ANIM_DOCUMENT* pDocument =
				Get_Or_Load_KeyframeAnimation(Slot.strKeyframeAnimationPath);
			if (nullptr == pDocument)
				continue;

			const double dElapsedSeconds = ImGui::GetTime() - Slot.dKeyframeAnimStartSeconds;
			const int32_t iWindowSpan = Slot.iKeyframeAnimWindowEnd - Slot.iKeyframeAnimWindowStart;
			int32_t iCurrentFrame = Slot.iKeyframeAnimWindowStart +
				static_cast<int32_t>(dElapsedSeconds * static_cast<double>(pDocument->fFrameRate));
			if (pDocument->bLoop && iWindowSpan > 0)
			{
				iCurrentFrame = Slot.iKeyframeAnimWindowStart +
					((iCurrentFrame - Slot.iKeyframeAnimWindowStart) % iWindowSpan + iWindowSpan) % iWindowSpan;
			}
			else
			{
				if (iCurrentFrame >= Slot.iKeyframeAnimWindowEnd)
					iCurrentFrame = Slot.iKeyframeAnimWindowEnd - 1;
				if (iCurrentFrame < Slot.iKeyframeAnimWindowStart)
					iCurrentFrame = Slot.iKeyframeAnimWindowStart;
			}

			for (const KEYFRAME_ANIM_LAYER& Layer : pDocument->Layers)
			{
				const KEYFRAME_ANIM_KEY* pActiveKey = nullptr;
				for (const KEYFRAME_ANIM_KEY& Key : Layer.Keys)
				{
					if (Key.iFrame > iCurrentFrame)
						break;
					pActiveKey = &Key;
				}
				if (nullptr == pActiveKey || pActiveKey->strAsset.empty() || pActiveKey->fAlpha <= 0.f)
					continue;

				ID3D11ShaderResourceView* pKeySRV = Get_Or_Load_Texture(pActiveKey->strAsset);
				if (nullptr == pKeySRV)
					continue;
				const TEXTURE_SIZE KeySize = Get_Texture_Size(pActiveKey->strAsset);
				if (KeySize.fWidth <= 0.f || KeySize.fHeight <= 0.f)
					continue;

				const f32_t fLocalScale = Slot.fKeyframeAnimationScale;
				const ImVec2 vKeyTopLeft(
					vTopLeft.x + pActiveKey->fX * fLocalScale * fScaleX,
					vTopLeft.y + pActiveKey->fY * fLocalScale * fScaleY);
				const ImVec2 vKeyBotRight(
					vKeyTopLeft.x + KeySize.fWidth * pActiveKey->fScaleX * fLocalScale * fScaleX,
					vKeyTopLeft.y + KeySize.fHeight * pActiveKey->fScaleY * fLocalScale * fScaleY);

				ImVec2 KeyCorners[4];
				Get_Rotated_Rect_Corners(vKeyTopLeft, vKeyBotRight, pActiveKey->fRotationDeg, KeyCorners);

				// TEMP DIAGNOSTIC: dump the resolved key rect for the LanceMaster gauge slots once/sec.
				if (Slot.strId.rfind("Lance_Id_Gauge", 0) == 0)
				{
					static double s_dLastGaugeKeyLog = -1.0;
					const double dNow2 = ImGui::GetTime();
					if (dNow2 - s_dLastGaugeKeyLog > 1.0)
					{
						s_dLastGaugeKeyLog = dNow2;
						char szBuf[320];
						sprintf_s(szBuf, "[GAUGE-KEY-DIAG] %s asset=%s keyX=%.2f keyY=%.2f keyScaleX=%.2f keyRot=%.2f localScale=%.2f texSize=(%.0f,%.0f) -> keyTopLeft=(%.1f,%.1f) keyBotRight=(%.1f,%.1f)\n",
							Slot.strId.c_str(), pActiveKey->strAsset.c_str(), pActiveKey->fX, pActiveKey->fY, pActiveKey->fScaleX, pActiveKey->fRotationDeg, fLocalScale,
							KeySize.fWidth, KeySize.fHeight, vKeyTopLeft.x, vKeyTopLeft.y, vKeyBotRight.x, vKeyBotRight.y);
						OutputDebugStringA(szBuf);
					}
				}

				const ImU32 iKeyTint = ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 1.f, 1.f, pActiveKey->fAlpha));
				Draw_Image_Quad(pDrawList, pKeySRV, KeyCorners, iKeyTint, pActiveKey->bAdditive, pActiveKey->bFlipX);
			}
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
