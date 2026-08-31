#include "UILayoutRuntime.h"

#include "GameInstance.h"
#include "DataJson.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"
#include "UI_Sprite.h"
#include "Texture.h"
#include "UITextureCache.h"

#include <fstream>
#include <algorithm>

Client::CUILayoutRuntime::CUILayoutRuntime(
	ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext,
	uint32_t iGameObjectLevelIndex, const wstring_t& strLayerTag,
	const wstring_t& strDocumentPath)
	: m_pDevice(pDevice)
	, m_pContext(pContext)
	, m_iGameObjectLevelIndex(iGameObjectLevelIndex)
	, m_strLayerTag(strLayerTag)
	, m_strDocumentPath(strDocumentPath)
	, m_pTextureCache(make_unique<CUITextureCache>(pDevice))
{
	Load();
}

Client::CUILayoutRuntime::~CUILayoutRuntime()
{
}

HRESULT Client::CUILayoutRuntime::Load()
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

	/* Reference-resolution -> current viewport pixels, computed once here (not re-applied on
	later resize) -- the same simplification CMainApp's own LoadingLayout chrome already
	accepts (its JSON coordinates are consumed as direct viewport pixels with no per-frame
	rescale either). Stored on the instance (not just a Load()-local) so Set_SlotPosition can
	apply the same conversion later. */
	const float2_t vViewportSize = CGameInstance::Get().Get_ViewportSize();
	m_fScaleX = (m_fResolutionWidth > 0.f) ? vViewportSize.x / m_fResolutionWidth : 1.f;
	m_fScaleY = (m_fResolutionHeight > 0.f) ? vViewportSize.y / m_fResolutionHeight : 1.f;

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

		string strId;
		if (const DATA_JSON_VALUE* pId = SlotValue.Find("id"))
			if (pId->Is_String())
				strId = pId->Get_String();
		if (strId.empty())
			continue;

		/* Raw reference-resolution units -- matches CHUDRuntimeView::Get_SlotRect's own
		contract, which RUNTIME_SLOT/Get_SlotRect below preserve. Scaled to viewport pixels
		only where a CUI_Sprite is actually positioned, just below. */
		const f32_t fRectX = static_cast<f32_t>(pX->Get_Number());
		const f32_t fRectY = static_cast<f32_t>(pY->Get_Number());
		const f32_t fRectWidth = static_cast<f32_t>(pWidth->Get_Number());
		const f32_t fRectHeight = static_cast<f32_t>(pHeight->Get_Number());

		/* A slot with no layers and no animation frames is a position-only marker -- e.g.
		ItemAnnounce_TextBox, whose rect a caller reads via Get_SlotRect to place text
		CGameInstance::Draw_Text draws separately, with nothing for this class itself to render.
		Its id/rect still needs to be queryable, so this only skips creating a CUI_Sprite, not
		registering the RUNTIME_SLOT below -- CHUDRuntimeView's own HUD_SLOT tracked marker
		slots the same way (Layers simply stays empty).

		A non-empty "animation.frames" plays as a flipbook instead of Layers, same convention
		CHUDRuntimeView's own HUD_SLOT uses (Layers stays empty for such a slot in the source
		JSON) -- frame 0 becomes this CUI_Sprite's initial texture; Update() swaps it from there. */
		vector<string> AnimationFramePaths;
		f32_t fAnimationFPS = 10.f;
		bool_t bAnimationLoop = true;
		bool_t bAnimationAdditive = false;
		if (const DATA_JSON_VALUE* pAnimation = SlotValue.Find("animation"))
		{
			if (const DATA_JSON_VALUE* pFrames = pAnimation->Find("frames"))
			{
				if (pFrames->Is_Array())
				{
					for (const DATA_JSON_VALUE& FrameValue : pFrames->Get_Array())
						if (FrameValue.Is_String() && !FrameValue.Get_String().empty())
							AnimationFramePaths.push_back(FrameValue.Get_String());
				}
			}
			if (const DATA_JSON_VALUE* pFps = pAnimation->Find("fps"))
				if (pFps->Is_Number() && pFps->Get_Number() > 0.0)
					fAnimationFPS = static_cast<f32_t>(pFps->Get_Number());
			if (const DATA_JSON_VALUE* pLoop = pAnimation->Find("loop"))
				if (pLoop->Is_Boolean())
					bAnimationLoop = pLoop->Get_Boolean();
			if (const DATA_JSON_VALUE* pAdditive = pAnimation->Find("additive"))
				if (pAdditive->Is_Boolean())
					bAnimationAdditive = pAdditive->Get_Boolean();
		}

		shared_ptr<CUI_Sprite> pSprite;
		const DATA_JSON_VALUE* pLayers = SlotValue.Find("layers");
		const DATA_JSON_VALUE* pFirstLayer = (nullptr != pLayers && pLayers->Is_Array() &&
			!pLayers->Get_Array().empty()) ? &pLayers->Get_Array()[0] : nullptr;
		const DATA_JSON_VALUE* pPath = (nullptr != pFirstLayer) ? pFirstLayer->Find("path") : nullptr;
		const bool_t hasStaticLayer = nullptr != pPath && pPath->Is_String() && !pPath->Get_String().empty();
		const bool_t hasAnimation = !AnimationFramePaths.empty();

		if (hasStaticLayer || hasAnimation)
		{
			const string& strNarrowPath = hasStaticLayer ? pPath->Get_String() : AnimationFramePaths[0];
			const wstring_t strWidePath(strNarrowPath.begin(), strNarrowPath.end());
			Ensure_TexturePrototype(strWidePath);

			CUI_Sprite::UI_SPRITE_DESC Desc{};
			Desc.fX = (fRectX + fRectWidth * 0.5f) * m_fScaleX;
			Desc.fY = (fRectY + fRectHeight * 0.5f) * m_fScaleY;
			Desc.fSizeX = fRectWidth * m_fScaleX;
			Desc.fSizeY = fRectHeight * m_fScaleY;
			Desc.strTextureTag = strWidePath;

			shared_ptr<CGameObject> pObject;
			if (SUCCEEDED(CGameInstance::Get().Add_GameObject_to_Layer(
				ETOUI(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Sprite"),
				m_iGameObjectLevelIndex, m_strLayerTag, &Desc, &pObject)))
			{
				pSprite = static_pointer_cast<CUI_Sprite>(pObject);

				if (hasStaticLayer)
				{
					float4_t vTint(1.f, 1.f, 1.f, 1.f);
					if (const DATA_JSON_VALUE* pTint = pFirstLayer->Find("tint"))
					{
						if (pTint->Is_Array() && 4u == pTint->Get_Array().size())
						{
							vTint.x = static_cast<f32_t>(pTint->Get_Array()[0].Get_Number());
							vTint.y = static_cast<f32_t>(pTint->Get_Array()[1].Get_Number());
							vTint.z = static_cast<f32_t>(pTint->Get_Array()[2].Get_Number());
							vTint.w = static_cast<f32_t>(pTint->Get_Array()[3].Get_Number());
						}
					}
					pSprite->Set_Tint(vTint);

					if (const DATA_JSON_VALUE* pAdditive = pFirstLayer->Find("additive"))
						if (pAdditive->Is_Boolean())
							pSprite->Set_Additive(pAdditive->Get_Boolean());
					if (const DATA_JSON_VALUE* pFlipX = pFirstLayer->Find("flipX"))
						if (pFlipX->Is_Boolean())
							pSprite->Set_FlipX(pFlipX->Get_Boolean());
				}
				else
				{
					pSprite->Set_Additive(bAnimationAdditive);
				}
			}
			/* One missing/renamed slot texture should not take the whole screen down -- same
			tolerance Level_Loading.cpp's own chrome loop already accepts. pSprite just stays
			null; the RUNTIME_SLOT (and its rect) is still registered below. */
		}

		RUNTIME_SLOT Slot{};
		Slot.strId = strId;
		Slot.fX = fRectX;
		Slot.fY = fRectY;
		Slot.fSizeX = fRectWidth;
		Slot.fSizeY = fRectHeight;
		Slot.AnimationFramePaths = move(AnimationFramePaths);
		Slot.fAnimationFPS = fAnimationFPS;
		Slot.bAnimationLoop = bAnimationLoop;
		Slot.pSprite = pSprite;
		m_Slots.push_back(move(Slot));
	}

	return S_OK;
}

void Client::CUILayoutRuntime::Ensure_TexturePrototype(const wstring_t& strPath)
{
	if (m_RegisteredTexturePrototypes.end() !=
		find(m_RegisteredTexturePrototypes.begin(), m_RegisteredTexturePrototypes.end(), strPath))
	{
		return;
	}
	m_RegisteredTexturePrototypes.push_back(strPath);

	const filesystem::path ResolvedPath = CRuntimeAssetRoot::Resolve(strPath);
	if (ResolvedPath.empty())
		return;

	/* Fails silently (and safely -- CGameInstance::Add_Prototype no-ops on a null prototype or
	a tag some earlier CUILayoutRuntime instance already registered) if strPath is already a
	registered prototype or CTexture::Create couldn't load it; the Add_GameObject_to_Layer call
	right after this in Load() then fails cleanly for that one slot instead. */
	CGameInstance::Get().Add_Prototype(
		ETOUI(LEVEL::STATIC), strPath,
		CTexture::Create(m_pDevice, m_pContext, ResolvedPath.c_str(), 1));
}

bool_t Client::CUILayoutRuntime::Get_SlotRect(
	const string& strId, f32_t& fX, f32_t& fY, f32_t& fWidth, f32_t& fHeight) const
{
	for (const RUNTIME_SLOT& Slot : m_Slots)
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

void Client::CUILayoutRuntime::Set_SlotVisible(const string& strId, bool_t bVisible)
{
	for (RUNTIME_SLOT& Slot : m_Slots)
	{
		if (Slot.strId != strId)
			continue;
		if (nullptr != Slot.pSprite)
			Slot.pSprite->Set_Tint(bVisible ?
				float4_t(1.f, 1.f, 1.f, 1.f) : float4_t(1.f, 1.f, 1.f, 0.f));
		return;
	}
}

void Client::CUILayoutRuntime::Set_SlotTint(const string& strId, const float4_t& vTint)
{
	for (RUNTIME_SLOT& Slot : m_Slots)
	{
		if (Slot.strId != strId)
			continue;
		if (nullptr != Slot.pSprite)
			Slot.pSprite->Set_Tint(vTint);
		return;
	}
}

void Client::CUILayoutRuntime::Set_SlotAlpha(const string& strId, f32_t fAlpha)
{
	Set_SlotTint(strId, float4_t(1.f, 1.f, 1.f, fAlpha));
}

void Client::CUILayoutRuntime::Set_SlotTexture(const string& strId, const string& strAssetPath)
{
	for (RUNTIME_SLOT& Slot : m_Slots)
	{
		if (Slot.strId != strId)
			continue;
		if (nullptr == Slot.pSprite)
			return;
		if (strAssetPath.empty())
		{
			Slot.pSprite->Set_Texture(nullptr);
			return;
		}
		Slot.pSprite->Set_Texture(m_pTextureCache->Get_Or_Load(strAssetPath));
		return;
	}
}

void Client::CUILayoutRuntime::Set_SlotPosition(const string& strId, f32_t fX, f32_t fY)
{
	for (RUNTIME_SLOT& Slot : m_Slots)
	{
		if (Slot.strId != strId)
			continue;
		Slot.fX = fX;
		Slot.fY = fY;
		if (nullptr != Slot.pSprite)
		{
			Slot.pSprite->Set_Rect(
				(fX + Slot.fSizeX * 0.5f) * m_fScaleX,
				(fY + Slot.fSizeY * 0.5f) * m_fScaleY,
				Slot.fSizeX * m_fScaleX, Slot.fSizeY * m_fScaleY);
		}
		return;
	}
}

void Client::CUILayoutRuntime::Update(f32_t fTimeDelta)
{
	for (RUNTIME_SLOT& Slot : m_Slots)
	{
		if (Slot.AnimationFramePaths.empty() || nullptr == Slot.pSprite)
			continue;

		const int32_t iFrameCount = static_cast<int32_t>(Slot.AnimationFramePaths.size());
		Slot.fAnimationElapsed += fTimeDelta;
		int32_t iFrame = static_cast<int32_t>(Slot.fAnimationElapsed * Slot.fAnimationFPS);
		if (Slot.bAnimationLoop)
			iFrame %= iFrameCount;
		else if (iFrame >= iFrameCount)
			iFrame = iFrameCount - 1;

		if (iFrame == Slot.iAnimationFrame)
			continue;
		Slot.iAnimationFrame = iFrame;
		Slot.pSprite->Set_Texture(m_pTextureCache->Get_Or_Load(Slot.AnimationFramePaths[iFrame]));
	}
}
