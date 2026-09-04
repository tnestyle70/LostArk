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

shared_ptr<Client::CUI_Sprite> Client::CUILayoutRuntime::Create_Sprite(
	f32_t fRectX, f32_t fRectY, f32_t fRectWidth, f32_t fRectHeight,
	const string& strTexturePath)
{
	const wstring_t strWidePath(strTexturePath.begin(), strTexturePath.end());
	Ensure_TexturePrototype(strWidePath);

	CUI_Sprite::UI_SPRITE_DESC Desc{};
	Desc.fX = (fRectX + fRectWidth * 0.5f) * m_fScaleX;
	Desc.fY = (fRectY + fRectHeight * 0.5f) * m_fScaleY;
	Desc.fSizeX = fRectWidth * m_fScaleX;
	Desc.fSizeY = fRectHeight * m_fScaleY;
	Desc.strTextureTag = strWidePath;

	shared_ptr<CGameObject> pObject;
	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		ETOUI(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Sprite"),
		m_iGameObjectLevelIndex, m_strLayerTag, &Desc, &pObject)))
	{
		/* One missing/renamed texture should not take the whole screen down -- same tolerance
		Level_Loading.cpp's own chrome loop already accepts. The RUNTIME_SLOT (and its rect) is
		still registered by the caller. */
		return nullptr;
	}
	return static_pointer_cast<CUI_Sprite>(pObject);
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
	rescale either). Stored on the instance (not just a Load()-local) so Set_SlotPosition and
	the per-frame keyframe evaluation can apply the same conversion later. */
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
		only where a CUI_Sprite is actually positioned. */
		const f32_t fRectX = static_cast<f32_t>(pX->Get_Number());
		const f32_t fRectY = static_cast<f32_t>(pY->Get_Number());
		const f32_t fRectWidth = static_cast<f32_t>(pWidth->Get_Number());
		const f32_t fRectHeight = static_cast<f32_t>(pHeight->Get_Number());

		string strOwnerClass;
		if (const DATA_JSON_VALUE* pOwnerClass = SlotValue.Find("ownerClass"))
			if (pOwnerClass->Is_String())
				strOwnerClass = pOwnerClass->Get_String();

		f32_t fRotation = 0.f;
		if (const DATA_JSON_VALUE* pRotation = SlotValue.Find("rotation"))
			if (pRotation->Is_Number())
				fRotation = static_cast<f32_t>(pRotation->Get_Number());

		string strKeyframePath;
		if (const DATA_JSON_VALUE* pKeyframePath = SlotValue.Find("keyframeAnimationPath"))
			if (pKeyframePath->Is_String())
				strKeyframePath = pKeyframePath->Get_String();
		f32_t fKeyframeScale = 1.f;
		if (const DATA_JSON_VALUE* pKeyframeScale = SlotValue.Find("keyframeAnimationScale"))
			if (pKeyframeScale->Is_Number() && pKeyframeScale->Get_Number() > 0.0)
				fKeyframeScale = static_cast<f32_t>(pKeyframeScale->Get_Number());

		/* A slot with no layers, no animation frames, and no keyframe document is a
		position-only marker -- e.g. ItemAnnounce_TextBox, whose rect a caller reads via
		Get_SlotRect to place text CGameInstance::Draw_Text draws separately, with nothing for
		this class itself to render. Its id/rect still needs to be queryable, so this only skips
		creating sprites, not registering the RUNTIME_SLOT below.

		A non-empty "animation.frames" plays as a flipbook instead of Layers, same convention
		CHUDRuntimeView's own HUD_SLOT uses -- frame 0 becomes this CUI_Sprite's initial
		texture; Update() swaps it from there. */
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

		RUNTIME_SLOT Slot{};
		Slot.strId = strId;
		Slot.strOwnerClass = strOwnerClass;
		Slot.fX = fRectX;
		Slot.fY = fRectY;
		Slot.fSizeX = fRectWidth;
		Slot.fSizeY = fRectHeight;
		Slot.fAnimationFPS = fAnimationFPS;
		Slot.bAnimationLoop = bAnimationLoop;
		Slot.strKeyframeAnimationPath = strKeyframePath;
		Slot.fKeyframeAnimationScale = fKeyframeScale;

		const bool_t hasAnimation = !AnimationFramePaths.empty();
		const bool_t hasKeyframe = !strKeyframePath.empty();

		if (hasKeyframe)
		{
			/* One sprite per document layer, created eagerly (documents are known at load time)
			so global draw order stays exactly the JSON's slot order. Prototype texture: the
			layer's first real key asset; a layer with no asset at all can never draw and gets a
			null placeholder to keep layer indices aligned. All start hidden -- a keyframe slot
			draws nothing until Play_KeyframeAnimation, matching CHUDRuntimeView. */
			if (const KEYFRAME_ANIM_DOCUMENT* pDocument =
				Get_Or_Load_KeyframeAnimation(strKeyframePath))
			{
				for (const KEYFRAME_ANIM_LAYER& Layer : pDocument->Layers)
				{
					const string* pFirstAsset = nullptr;
					for (const KEYFRAME_ANIM_KEY& Key : Layer.Keys)
					{
						if (!Key.strAsset.empty())
						{
							pFirstAsset = &Key.strAsset;
							break;
						}
					}
					shared_ptr<CUI_Sprite> pLayerSprite;
					if (nullptr != pFirstAsset)
					{
						pLayerSprite = Create_Sprite(
							fRectX, fRectY, fRectWidth, fRectHeight, *pFirstAsset);
						if (nullptr != pLayerSprite)
							pLayerSprite->Set_Visible(false);
					}
					Slot.KeyframeSprites.push_back(pLayerSprite);
				}
			}
		}
		else if (hasAnimation)
		{
			Slot.AnimationFramePaths = move(AnimationFramePaths);
			Slot.pSprite = Create_Sprite(
				fRectX, fRectY, fRectWidth, fRectHeight, Slot.AnimationFramePaths[0]);
			if (nullptr != Slot.pSprite)
			{
				Slot.pSprite->Set_Additive(bAnimationAdditive);
				if (0.f != fRotation)
					Slot.pSprite->Set_Rotation(fRotation);
			}
		}
		else if (const DATA_JSON_VALUE* pLayers = SlotValue.Find("layers"))
		{
			/* Every authored layer becomes its own sprite (stacked in order) -- the skill/item
			quick slots author a background + frame pair in one slot. */
			if (pLayers->Is_Array())
			{
				for (const DATA_JSON_VALUE& LayerValue : pLayers->Get_Array())
				{
					const DATA_JSON_VALUE* pPath = LayerValue.Find("path");
					if (nullptr == pPath || !pPath->Is_String() || pPath->Get_String().empty())
						continue;

					shared_ptr<CUI_Sprite> pLayerSprite = Create_Sprite(
						fRectX, fRectY, fRectWidth, fRectHeight, pPath->Get_String());
					if (nullptr == pLayerSprite)
						continue;

					float4_t vTint(1.f, 1.f, 1.f, 1.f);
					if (const DATA_JSON_VALUE* pTint = LayerValue.Find("tint"))
					{
						if (pTint->Is_Array() && 4u == pTint->Get_Array().size())
						{
							vTint.x = static_cast<f32_t>(pTint->Get_Array()[0].Get_Number());
							vTint.y = static_cast<f32_t>(pTint->Get_Array()[1].Get_Number());
							vTint.z = static_cast<f32_t>(pTint->Get_Array()[2].Get_Number());
							vTint.w = static_cast<f32_t>(pTint->Get_Array()[3].Get_Number());
						}
					}
					pLayerSprite->Set_Tint(vTint);
					if (const DATA_JSON_VALUE* pAdditive = LayerValue.Find("additive"))
						if (pAdditive->Is_Boolean())
							pLayerSprite->Set_Additive(pAdditive->Get_Boolean());
					if (const DATA_JSON_VALUE* pFlipX = LayerValue.Find("flipX"))
						if (pFlipX->Is_Boolean())
							pLayerSprite->Set_FlipX(pFlipX->Get_Boolean());
					if (0.f != fRotation)
						pLayerSprite->Set_Rotation(fRotation);

					if (nullptr == Slot.pSprite)
						Slot.pSprite = pLayerSprite;
					else
						Slot.ExtraLayerSprites.push_back(pLayerSprite);
				}
			}
		}

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
	right after this in Create_Sprite then fails cleanly for that one sprite instead. */
	CGameInstance::Get().Add_Prototype(
		ETOUI(LEVEL::STATIC), strPath,
		CTexture::Create(m_pDevice, m_pContext, ResolvedPath.c_str(), 1));
}

const Client::CUILayoutRuntime::KEYFRAME_ANIM_DOCUMENT*
Client::CUILayoutRuntime::Get_Or_Load_KeyframeAnimation(const string& strPath)
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
							if (const DATA_JSON_VALUE* pKeyX = KeyValue.Find("x"))
								if (pKeyX->Is_Number())
									Key.fX = static_cast<f32_t>(pKeyX->Get_Number());
							if (const DATA_JSON_VALUE* pKeyY = KeyValue.Find("y"))
								if (pKeyY->Is_Number())
									Key.fY = static_cast<f32_t>(pKeyY->Get_Number());
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
		Slot.bVisible = bVisible;
		if (nullptr != Slot.pSprite)
			Slot.pSprite->Set_Visible(bVisible);
		for (const shared_ptr<CUI_Sprite>& pExtra : Slot.ExtraLayerSprites)
			if (nullptr != pExtra)
				pExtra->Set_Visible(bVisible);
		/* Keyframe layer sprites only ever hide here -- showing is Update()'s per-key decision
		(each layer's active key decides whether that layer draws at all this frame). */
		if (!bVisible)
		{
			for (const shared_ptr<CUI_Sprite>& pKeySprite : Slot.KeyframeSprites)
				if (nullptr != pKeySprite)
					pKeySprite->Set_Visible(false);
		}
		return;
	}
}

void Client::CUILayoutRuntime::Set_ActiveOwnerClass(const string& strOwnerClass)
{
	for (RUNTIME_SLOT& Slot : m_Slots)
	{
		if (Slot.strOwnerClass.empty())
			continue;
		const bool_t bVisible = Slot.strOwnerClass == strOwnerClass;
		Slot.bVisible = bVisible;
		if (nullptr != Slot.pSprite)
			Slot.pSprite->Set_Visible(bVisible);
		for (const shared_ptr<CUI_Sprite>& pExtra : Slot.ExtraLayerSprites)
			if (nullptr != pExtra)
				pExtra->Set_Visible(bVisible);
		if (!bVisible)
		{
			for (const shared_ptr<CUI_Sprite>& pKeySprite : Slot.KeyframeSprites)
				if (nullptr != pKeySprite)
					pKeySprite->Set_Visible(false);
		}
	}
}

void Client::CUILayoutRuntime::Set_AllSlotsVisible(bool_t bVisible)
{
	for (RUNTIME_SLOT& Slot : m_Slots)
	{
		Slot.bVisible = bVisible;
		if (nullptr != Slot.pSprite)
			Slot.pSprite->Set_Visible(bVisible);
		for (const shared_ptr<CUI_Sprite>& pExtra : Slot.ExtraLayerSprites)
			if (nullptr != pExtra)
				pExtra->Set_Visible(bVisible);
		if (!bVisible)
		{
			for (const shared_ptr<CUI_Sprite>& pKeySprite : Slot.KeyframeSprites)
				if (nullptr != pKeySprite)
					pKeySprite->Set_Visible(false);
		}
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
		for (const shared_ptr<CUI_Sprite>& pExtra : Slot.ExtraLayerSprites)
			if (nullptr != pExtra)
				pExtra->Set_Tint(vTint);
		return;
	}
}

void Client::CUILayoutRuntime::Set_SlotTintMultiplier(
	const string& strId,
	const float4_t& vTintMultiplier)
{
	for (RUNTIME_SLOT& Slot : m_Slots)
	{
		if (Slot.strId != strId)
			continue;
		if (nullptr != Slot.pSprite)
			Slot.pSprite->Set_TintMultiplier(vTintMultiplier);
		for (const shared_ptr<CUI_Sprite>& pExtra : Slot.ExtraLayerSprites)
			if (nullptr != pExtra)
				pExtra->Set_TintMultiplier(vTintMultiplier);
		for (const shared_ptr<CUI_Sprite>& pKeySprite : Slot.KeyframeSprites)
			if (nullptr != pKeySprite)
				pKeySprite->Set_TintMultiplier(vTintMultiplier);
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
		const f32_t fCenterX = (fX + Slot.fSizeX * 0.5f) * m_fScaleX;
		const f32_t fCenterY = (fY + Slot.fSizeY * 0.5f) * m_fScaleY;
		const f32_t fSizeX = Slot.fSizeX * m_fScaleX;
		const f32_t fSizeY = Slot.fSizeY * m_fScaleY;
		if (nullptr != Slot.pSprite)
			Slot.pSprite->Set_Rect(fCenterX, fCenterY, fSizeX, fSizeY);
		for (const shared_ptr<CUI_Sprite>& pExtra : Slot.ExtraLayerSprites)
			if (nullptr != pExtra)
				pExtra->Set_Rect(fCenterX, fCenterY, fSizeX, fSizeY);
		return;
	}
}

void Client::CUILayoutRuntime::Set_SlotRect(
	const string& strId, f32_t fX, f32_t fY, f32_t fWidth, f32_t fHeight)
{
	for (RUNTIME_SLOT& Slot : m_Slots)
	{
		if (Slot.strId != strId)
			continue;
		Slot.fX = fX;
		Slot.fY = fY;
		Slot.fSizeX = fWidth;
		Slot.fSizeY = fHeight;
		const f32_t fCenterX = (fX + fWidth * 0.5f) * m_fScaleX;
		const f32_t fCenterY = (fY + fHeight * 0.5f) * m_fScaleY;
		if (nullptr != Slot.pSprite)
			Slot.pSprite->Set_Rect(fCenterX, fCenterY, fWidth * m_fScaleX, fHeight * m_fScaleY);
		for (const shared_ptr<CUI_Sprite>& pExtra : Slot.ExtraLayerSprites)
			if (nullptr != pExtra)
				pExtra->Set_Rect(fCenterX, fCenterY, fWidth * m_fScaleX, fHeight * m_fScaleY);
		return;
	}
}

void Client::CUILayoutRuntime::Set_SlotFillRatio(const string& strId, f32_t fFillRatio)
{
	for (RUNTIME_SLOT& Slot : m_Slots)
	{
		if (Slot.strId != strId)
			continue;
		if (nullptr != Slot.pSprite)
			Slot.pSprite->Set_FillRatio(fFillRatio);
		return;
	}
}

void Client::CUILayoutRuntime::Set_SlotArcRatio(const string& strId, f32_t fArcRatio)
{
	for (RUNTIME_SLOT& Slot : m_Slots)
	{
		if (Slot.strId != strId)
			continue;
		if (nullptr != Slot.pSprite)
			Slot.pSprite->Set_ArcRatio(fArcRatio);
		return;
	}
}

void Client::CUILayoutRuntime::Set_SlotRotation(const string& strId, f32_t fDegrees)
{
	for (RUNTIME_SLOT& Slot : m_Slots)
	{
		if (Slot.strId != strId)
			continue;
		if (nullptr != Slot.pSprite)
			Slot.pSprite->Set_Rotation(fDegrees);
		for (const shared_ptr<CUI_Sprite>& pExtra : Slot.ExtraLayerSprites)
			if (nullptr != pExtra)
				pExtra->Set_Rotation(fDegrees);
		return;
	}
}

void Client::CUILayoutRuntime::Ensure_RuntimeSlot(const string& strId, f32_t fX, f32_t fY,
	f32_t fWidth, f32_t fHeight, const string& strTexturePath)
{
	for (const RUNTIME_SLOT& Slot : m_Slots)
	{
		if (Slot.strId == strId)
			return;
	}

	RUNTIME_SLOT Slot{};
	Slot.strId = strId;
	Slot.fX = fX;
	Slot.fY = fY;
	Slot.fSizeX = fWidth;
	Slot.fSizeY = fHeight;
	Slot.pSprite = Create_Sprite(fX, fY, fWidth, fHeight, strTexturePath);
	m_Slots.push_back(move(Slot));
}

bool_t Client::CUILayoutRuntime::Play_KeyframeAnimation(const string& strId, const string& strLabel)
{
	for (RUNTIME_SLOT& Slot : m_Slots)
	{
		if (Slot.strId != strId || Slot.strKeyframeAnimationPath.empty())
			continue;

		const KEYFRAME_ANIM_DOCUMENT* pDocument =
			Get_Or_Load_KeyframeAnimation(Slot.strKeyframeAnimationPath);
		if (nullptr == pDocument)
			return false;

		const auto LabelIter = pDocument->Labels.find(strLabel);
		if (pDocument->Labels.end() == LabelIter)
			return false;

		/* Window = [this label, the next label after it) -- or the document end when this is
		the last label. Same contract as CHUDRuntimeView::Play_KeyframeAnimation. */
		const int32_t iStartFrame = LabelIter->second;
		int32_t iEndFrame = pDocument->iFrameCount;
		for (const auto& Pair : pDocument->Labels)
		{
			if (Pair.second > iStartFrame && Pair.second < iEndFrame)
				iEndFrame = Pair.second;
		}

		Slot.iKeyframeWindowStart = iStartFrame;
		Slot.iKeyframeWindowEnd = iEndFrame;
		Slot.fKeyframeElapsedSeconds = 0.0;
		return true;
	}
	return false;
}

void Client::CUILayoutRuntime::Update_KeyframeSlot(RUNTIME_SLOT& Slot, f32_t fTimeDelta)
{
	if (Slot.fKeyframeElapsedSeconds >= 0.0)
		Slot.fKeyframeElapsedSeconds += static_cast<f64_t>(fTimeDelta);

	if (!Slot.bVisible || Slot.fKeyframeElapsedSeconds < 0.0)
	{
		for (const shared_ptr<CUI_Sprite>& pKeySprite : Slot.KeyframeSprites)
			if (nullptr != pKeySprite)
				pKeySprite->Set_Visible(false);
		return;
	}

	const KEYFRAME_ANIM_DOCUMENT* pDocument =
		Get_Or_Load_KeyframeAnimation(Slot.strKeyframeAnimationPath);
	if (nullptr == pDocument)
		return;

	/* Same frame/window math as CHUDRuntimeView::Render's keyframe branch: a looping document
	wraps within the played window; a one-shot holds its window's last frame (the persistent
	end state baked into the extraction -- e.g. Artist's bubble glow). */
	const int32_t iWindowSpan = Slot.iKeyframeWindowEnd - Slot.iKeyframeWindowStart;
	int32_t iCurrentFrame = Slot.iKeyframeWindowStart + static_cast<int32_t>(
		Slot.fKeyframeElapsedSeconds * static_cast<f64_t>(pDocument->fFrameRate));
	if (pDocument->bLoop && iWindowSpan > 0)
	{
		iCurrentFrame = Slot.iKeyframeWindowStart +
			((iCurrentFrame - Slot.iKeyframeWindowStart) % iWindowSpan + iWindowSpan) % iWindowSpan;
	}
	else
	{
		if (iCurrentFrame >= Slot.iKeyframeWindowEnd)
			iCurrentFrame = Slot.iKeyframeWindowEnd - 1;
		if (iCurrentFrame < Slot.iKeyframeWindowStart)
			iCurrentFrame = Slot.iKeyframeWindowStart;
	}

	const size_t iLayerCount =
		(std::min)(pDocument->Layers.size(), Slot.KeyframeSprites.size());
	for (size_t iLayer = 0; iLayer < iLayerCount; ++iLayer)
	{
		const shared_ptr<CUI_Sprite>& pKeySprite = Slot.KeyframeSprites[iLayer];
		if (nullptr == pKeySprite)
			continue;

		const KEYFRAME_ANIM_KEY* pActiveKey = nullptr;
		for (const KEYFRAME_ANIM_KEY& Key : pDocument->Layers[iLayer].Keys)
		{
			if (Key.iFrame > iCurrentFrame)
				break;
			pActiveKey = &Key;
		}
		if (nullptr == pActiveKey || pActiveKey->strAsset.empty() || pActiveKey->fAlpha <= 0.f)
		{
			pKeySprite->Set_Visible(false);
			continue;
		}

		ID3D11ShaderResourceView* pKeySRV = m_pTextureCache->Get_Or_Load(pActiveKey->strAsset);
		f32_t fTexWidth = 0.f, fTexHeight = 0.f;
		if (nullptr == pKeySRV ||
			!m_pTextureCache->Get_Texture_Size(pActiveKey->strAsset, fTexWidth, fTexHeight))
		{
			pKeySprite->Set_Visible(false);
			continue;
		}

		/* Key x/y are the piece's reference-resolution offset from the slot's own top-left; the
		piece's size is its art's native pixel size times the key's scale -- identical numbers to
		the old ImGui draw, with the viewport scale applied by Set_Rect here instead of at the
		draw call. */
		const f32_t fLocalScale = Slot.fKeyframeAnimationScale;
		const f32_t fKeyX = Slot.fX + pActiveKey->fX * fLocalScale;
		const f32_t fKeyY = Slot.fY + pActiveKey->fY * fLocalScale;
		const f32_t fKeyWidth = fTexWidth * pActiveKey->fScaleX * fLocalScale;
		const f32_t fKeyHeight = fTexHeight * pActiveKey->fScaleY * fLocalScale;

		pKeySprite->Set_Rect(
			(fKeyX + fKeyWidth * 0.5f) * m_fScaleX,
			(fKeyY + fKeyHeight * 0.5f) * m_fScaleY,
			fKeyWidth * m_fScaleX, fKeyHeight * m_fScaleY);
		pKeySprite->Set_Rotation(pActiveKey->fRotationDeg);
		pKeySprite->Set_Texture(pKeySRV);
		pKeySprite->Set_Tint(float4_t(1.f, 1.f, 1.f, pActiveKey->fAlpha));
		pKeySprite->Set_Additive(pActiveKey->bAdditive);
		pKeySprite->Set_FlipX(pActiveKey->bFlipX);
		pKeySprite->Set_Visible(true);
	}
}

void Client::CUILayoutRuntime::Update(f32_t fTimeDelta)
{
	for (RUNTIME_SLOT& Slot : m_Slots)
	{
		if (!Slot.strKeyframeAnimationPath.empty())
		{
			Update_KeyframeSlot(Slot, fTimeDelta);
			continue;
		}

		if (Slot.AnimationFramePaths.empty() || nullptr == Slot.pSprite)
			continue;

		const int32_t iFrameCount = static_cast<int32_t>(Slot.AnimationFramePaths.size());
		Slot.fAnimationElapsed += fTimeDelta;
		int32_t iFrame = static_cast<int32_t>(Slot.fAnimationElapsed * Slot.fAnimationFPS);
		bool_t bPastEnd = false;
		if (Slot.bAnimationLoop)
		{
			iFrame %= iFrameCount;
		}
		else if (iFrame >= iFrameCount)
		{
			/* A finished one-shot stops drawing entirely (CHUDRuntimeView's own AnimationFrames
			contract -- ItemUpgrade's CoreFlash/Shockwave/result bursts rely on vanishing at
			their last frame, not freezing on it) until Restart_Animation rewinds it. Re-applied
			every tick so a consumer's own Set_SlotVisible(true) on a finished slot doesn't
			resurrect a stale frame. */
			iFrame = iFrameCount - 1;
			bPastEnd = true;
		}
		Slot.pSprite->Set_Visible(Slot.bVisible && !bPastEnd);

		if (iFrame == Slot.iAnimationFrame)
			continue;
		Slot.iAnimationFrame = iFrame;
		Slot.pSprite->Set_Texture(m_pTextureCache->Get_Or_Load(Slot.AnimationFramePaths[iFrame]));
	}
}

void Client::CUILayoutRuntime::Restart_Animation(const string& strId)
{
	for (RUNTIME_SLOT& Slot : m_Slots)
	{
		if (Slot.strId != strId || Slot.AnimationFramePaths.empty())
			continue;
		Slot.fAnimationElapsed = 0.f;
		Slot.iAnimationFrame = 0;
		if (nullptr != Slot.pSprite)
		{
			Slot.pSprite->Set_Texture(
				m_pTextureCache->Get_Or_Load(Slot.AnimationFramePaths[0]));
		}
		return;
	}
}

void Client::CUILayoutRuntime::Set_Animation_Frame(const string& strId, int32_t iFrame)
{
	for (RUNTIME_SLOT& Slot : m_Slots)
	{
		if (Slot.strId != strId || Slot.AnimationFramePaths.empty())
			continue;
		const int32_t iFrameCount = static_cast<int32_t>(Slot.AnimationFramePaths.size());
		const int32_t iClamped = (std::max)(0, (std::min)(iFrame, iFrameCount - 1));
		Slot.fAnimationElapsed = (Slot.fAnimationFPS > 0.f) ?
			static_cast<f32_t>(iClamped) / Slot.fAnimationFPS : 0.f;
		if (iClamped != Slot.iAnimationFrame && nullptr != Slot.pSprite)
		{
			Slot.iAnimationFrame = iClamped;
			Slot.pSprite->Set_Texture(
				m_pTextureCache->Get_Or_Load(Slot.AnimationFramePaths[iClamped]));
		}
		return;
	}
}

void Client::CUILayoutRuntime::Set_SlotAnimation(const string& strId,
	const vector<string>& Frames, f32_t fFps, bool_t bLoop)
{
	for (RUNTIME_SLOT& Slot : m_Slots)
	{
		if (Slot.strId != strId)
			continue;
		Slot.AnimationFramePaths = Frames;
		if (fFps > 0.f)
			Slot.fAnimationFPS = fFps;
		Slot.bAnimationLoop = bLoop;
		Slot.fAnimationElapsed = 0.f;
		Slot.iAnimationFrame = 0;
		/* Show frame 0 immediately so the swap doesn't flash the previous boss's last frame for
		one tick before Update() advances. A cleared list leaves the static texture untouched. */
		if (nullptr != Slot.pSprite && !Frames.empty())
			Slot.pSprite->Set_Texture(m_pTextureCache->Get_Or_Load(Frames[0]));
		return;
	}
}
