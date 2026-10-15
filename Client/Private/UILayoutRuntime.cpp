// 저작 UI JSON을 slot 값과 Layer에 등록된 sprite로 연결한다.
#include "UILayoutRuntime.h"
// 기존 애니메이션 공통 include를 유지한다. 아래 KEYFRAME_ANIM_*는 이 클래스가 선언한 문서 타입이다.
#include "Engine_AnimationTypes.h"

// GameInstance로 원형·Layer를 연결하고 DataJson으로 JSON을 읽는다.
#include "GameInstance.h"
#include "DataJson.h"
// 저작 문서와 Resources 상대 이미지 경로를 각각의 루트에서 찾는다.
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"
// 실제 sprite를 만들고 Texture 원형·실행 중 SRV 캐시를 준비한다.
#include "UI_Sprite.h"
#include "Texture.h"
#include "UITextureCache.h"

// 파일 읽기, 슬롯·frame 범위 계산, 시간·글꼴 수치의 유한값 검사에 사용한다.
#include <fstream>
#include <algorithm>
#include <cmath>

// MainApp·각 view가 생성한다. 장치 참조와 경로를 보관하고 Texture 캐시를 만든 뒤 문서를 읽는다.
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

// 멤버의 참조만 자동 해제된다. Layer에도 등록된 sprite는 별도 Release_Sprites로 제거한다.
Client::CUILayoutRuntime::~CUILayoutRuntime()
{
}

// 화면 소유자가 조기 종료할 때 호출한다. 자신이 만든 sprite만 숨겨 Layer에서 빼고 슬롯을 비운다.
void Client::CUILayoutRuntime::Release_Sprites()
{
	const auto release = [this](shared_ptr<CUI_Sprite>& sprite)
	{
		if (!sprite)
			return;
		// 이번 프레임 Renderer에도 참조가 남을 수 있어 먼저 숨긴다.
		sprite->Set_Visible(false);
		(void)CGameInstance::Get().Remove_GameObject_from_Layer(
			m_iGameObjectLevelIndex, m_strLayerTag, sprite);
		sprite.reset();
	};
	for (RUNTIME_SLOT& slot : m_Slots)
	{
		release(slot.pSprite);
		for (auto& sprite : slot.ExtraLayerSprites)
			release(sprite);
		for (auto& sprite : slot.KeyframeSprites)
			release(sprite);
	}
	m_Slots.clear();
	m_SlotIndices.clear();
}

// Load·동적 슬롯 추가가 좌상단 rect를 중심 rect로 바꿔 UI 원형을 복제하고 Layer에 등록한다.
shared_ptr<Client::CUI_Sprite> Client::CUILayoutRuntime::Create_Sprite(
	f32_t fRectX, f32_t fRectY, f32_t fRectWidth, f32_t fRectHeight,
	const string& strTexturePath)
{
	const wstring_t strWidePath(strTexturePath.begin(), strTexturePath.end());
	Ensure_TexturePrototype(strWidePath);

	CUI_Sprite::UI_SPRITE_DESC Desc{};
	Desc.fX = (fRectX + fRectWidth * 0.5f);
	Desc.fY = (fRectY + fRectHeight * 0.5f);
	Desc.fSizeX = fRectWidth;
	Desc.fSizeY = fRectHeight;
	Desc.strTextureTag = strWidePath;
	Desc.referenceResolution = {m_fResolutionWidth, m_fResolutionHeight};

	shared_ptr<CGameObject> pObject;
	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		ETOUI(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Sprite"),
		m_iGameObjectLevelIndex, m_strLayerTag, &Desc, &pObject)))
	{
		// Layer 추가가 실패하면 sprite만 비워 둔다. 호출자는 슬롯 ID와 사각형을 계속 등록한다.
		return nullptr;
	}
	const shared_ptr<CUI_Sprite> pSprite = static_pointer_cast<CUI_Sprite>(pObject);
	// 나중에 생성되는 애니메이션 이미지도 문서의 정렬 layer를 따른다.
	pSprite->Set_UISortLayer(m_iUISortLayer);
	return pSprite;
}

// view가 문서 전체의 앞뒤 layer를 바꾸면 현재 sprite에도 같은 값을 전파한다.
void Client::CUILayoutRuntime::Set_UISortLayer(const int32_t iLayer)
{
	m_iUISortLayer = iLayer;
	for (RUNTIME_SLOT& Slot : m_Slots)
	{
		if (nullptr != Slot.pSprite)
			Slot.pSprite->Set_UISortLayer(iLayer);
		for (const shared_ptr<CUI_Sprite>& pExtra : Slot.ExtraLayerSprites)
			if (nullptr != pExtra)
				pExtra->Set_UISortLayer(iLayer);
		for (const shared_ptr<CUI_Sprite>& pKeyframe : Slot.KeyframeSprites)
			if (nullptr != pKeyframe)
				pKeyframe->Set_UISortLayer(iLayer);
	}
}

// 생성자가 JSON을 읽어 슬롯을 순서대로 등록한다. 파일·파싱 실패는 S_OK, 누락된 필수 슬롯 필드는 continue로 넘긴다.
HRESULT Client::CUILayoutRuntime::Load()
{
	// Data/UI의 저작 JSON을 직접 읽는다. Resources 이미지 경로나 Bin/DataFiles 복사본과 구분한다.
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

	// 문서 좌표를 유지하고 실제 viewport 배율은 CUI_Sprite에서 적용한다.

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

		// 좌상단·폭·높이는 문서 기준 단위로 보관해 Get_SlotRect와 입력 검사가 공유한다.
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

		// 이미지 없는 슬롯도 텍스트 위치용으로 등록한다.
		// frames가 있으면 정적 layers 대신 첫 frame의 sprite를 만들고 Update에서 교체한다.
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
			// keyframe 레이어마다 sprite를 미리 만들어 순서를 맞춘다.
			// asset 없는 레이어는 nullptr 자리를 남기고, 유효한 sprite도 재생 전에는 숨긴다.
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
			// 슬롯의 layers를 순서대로 겹친다. 첫 이미지는 기본 sprite, 나머지는 추가 sprite다.
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

		// 이 행의 생성 결과를 즉시 보관한다. 뒤 행의 누락·실패 때문에 앞서 만든 sprite를 되돌리지 않는다.
		Append_Slot(move(Slot));
	}

	return S_OK;
}

// 경로를 한 번 기록하고 STATIC Texture 원형 등록을 시도한다. 실패 경로도 이 인스턴스에서는 재시도하지 않는다.
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

	// 중복 tag·로드 실패의 반환값은 여기서 처리하지 않는다. 기존 원형이 있으면 그 원형이 유지된다.
	CGameInstance::Get().Add_Prototype(
		ETOUI(LEVEL::STATIC), strPath,
		CTexture::Create(m_pDevice, m_pContext, ResolvedPath.c_str(), 1));
}

// Load·재생 함수가 요청한 문서를 캐시한다. 실패도 기록해 매 프레임 같은 파일을 다시 읽지 않는다.
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
							if (const DATA_JSON_VALUE* pTint = KeyValue.Find("tint"))
							{
								if (pTint->Is_Array() && 3 <= pTint->Get_Array().size())
								{
									const auto& Tint = pTint->Get_Array();
									if (Tint[0].Is_Number())
										Key.fTintR = static_cast<f32_t>(Tint[0].Get_Number());
									if (Tint[1].Is_Number())
										Key.fTintG = static_cast<f32_t>(Tint[1].Get_Number());
									if (Tint[2].Is_Number())
										Key.fTintB = static_cast<f32_t>(Tint[2].Get_Number());
								}
							}
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

	// isLoaded가 false인 결과도 보관한다. 반환 포인터는 이 캐시 객체의 수명 안에서만 차용한다.
	auto InsertedIter = m_KeyframeAnimationCache.emplace(strPath, move(Document)).first;
	return InsertedIter->second.isLoaded ? &InsertedIter->second : nullptr;
}

// 슬롯 본문은 vector로 옮기고 같은 ID의 검색 목록에 새 index를 추가한다.
void Client::CUILayoutRuntime::Append_Slot(RUNTIME_SLOT Slot)
{
	m_Slots.push_back(move(Slot));
	m_SlotIndices[m_Slots.back().strId].push_back(m_Slots.size() - 1u);
}

// 수정 가능한 조회도 const 조회의 선택 규칙을 재사용한다. 반환 주소는 vector 재배치 전에만 유효하다.
Client::CUILayoutRuntime::RUNTIME_SLOT* Client::CUILayoutRuntime::Find_Slot(
	const string& strId, const SLOT_REQUIREMENT eRequirement)
{
	return const_cast<RUNTIME_SLOT*>(
		static_cast<const CUILayoutRuntime&>(*this).Find_Slot(strId, eRequirement));
}

// ID 후보를 등록 순서대로 검사해 요구한 애니메이션 종류의 첫 슬롯을 차용한다.
const Client::CUILayoutRuntime::RUNTIME_SLOT* Client::CUILayoutRuntime::Find_Slot(
	const string& strId, const SLOT_REQUIREMENT eRequirement) const
{
	const auto Found = m_SlotIndices.find(strId);
	if (Found == m_SlotIndices.end())
		return nullptr;
	if (eRequirement == SLOT_REQUIREMENT::ANY)
		return &m_Slots[Found->second.front()];
	for (const size_t iSlot : Found->second)
	{
		const RUNTIME_SLOT& Slot = m_Slots[iSlot];
		if (eRequirement == SLOT_REQUIREMENT::FLIPBOOK && Slot.AnimationFramePaths.empty())
			continue;
		if (eRequirement == SLOT_REQUIREMENT::KEYFRAME && Slot.strKeyframeAnimationPath.empty())
			continue;
		return &Slot;
	}
	return nullptr;
}

// view와 입력·드래그가 같은 문서 좌표를 읽는다. ID가 없으면 출력값을 바꾸지 않는다.
bool_t Client::CUILayoutRuntime::Get_SlotRect(
	const string& strId, f32_t& fX, f32_t& fY, f32_t& fWidth, f32_t& fHeight) const
{
	const RUNTIME_SLOT* pSlot = Find_Slot(strId);
	if (nullptr == pSlot)
		return false;
	const RUNTIME_SLOT& Slot = *pSlot;
	fX = Slot.fX;
	fY = Slot.fY;
	fWidth = Slot.fSizeX;
	fHeight = Slot.fSizeY;
	return true;
}

// view가 ID의 표시 상태를 바꾼다. keyframe은 숨김만 즉시 전파하고 표시는 다음 갱신에 맡긴다.
void Client::CUILayoutRuntime::Set_SlotVisible(const string& strId, bool_t bVisible)
{
	RUNTIME_SLOT* pSlot = Find_Slot(strId);
	if (nullptr == pSlot)
		return;
	RUNTIME_SLOT& Slot = *pSlot;
	Slot.bVisible = bVisible;
	if (nullptr != Slot.pSprite)
		Slot.pSprite->Set_Visible(bVisible);
	for (const shared_ptr<CUI_Sprite>& pExtra : Slot.ExtraLayerSprites)
		if (nullptr != pExtra)
			pExtra->Set_Visible(bVisible);
	// 여기서는 keyframe을 숨기기만 한다. 다시 보일지는 Update가 현재 key로 결정한다.
	if (!bVisible)
	{
		for (const shared_ptr<CUI_Sprite>& pKeySprite : Slot.KeyframeSprites)
			if (nullptr != pKeySprite)
				pKeySprite->Set_Visible(false);
	}
}

// 연출 view가 빈 위치 슬롯에 caption을 넣는다. 검증·Layer 등록 성공 뒤 slot의 sprite 참조를 바꾼다.
bool_t Client::CUILayoutRuntime::Set_SlotCaption(const string& strId,
    const wstring_t& caption, const wstring_t& font)
{
    auto* slot = Find_Slot(strId);
    if (!slot || slot->pSprite || !slot->KeyframeSprites.empty() || caption.empty() || font.empty()) return false;
    const auto measured = CGameInstance::Get().Measure_Text(font, caption.c_str());
    if (!std::isfinite(measured.y) || measured.y <= 0.f ||
        m_fResolutionWidth <= 0.f || m_fResolutionHeight <= 0.f) return false;
    CUI_Sprite::UI_SPRITE_DESC desc{};
    desc.fX = slot->fX + slot->fSizeX * 0.5f;
    desc.fY = slot->fY + slot->fSizeY * 0.5f;
    desc.fSizeX = slot->fSizeX; desc.fSizeY = slot->fSizeY;
    desc.referenceResolution = {m_fResolutionWidth, m_fResolutionHeight};
    desc.caption = caption; desc.captionFont = font;
    shared_ptr<CGameObject> object;
    if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(ETOUI(LEVEL::STATIC),
        TEXT("Prototype_GameObject_UI_Sprite"), m_iGameObjectLevelIndex, m_strLayerTag, &desc, &object))) return false;
    slot->pSprite = static_pointer_cast<CUI_Sprite>(object);
    slot->pSprite->Set_UISortLayer(m_iUISortLayer);
    slot->pSprite->Set_Visible(slot->bVisible);
    return true;
}

// 슬롯의 모든 sprite가 Late_Update에서 선택할 렌더 그룹을 바꾼다.
void Client::CUILayoutRuntime::Set_SlotScenePresentation(const string& strId, const bool_t enabled)
{
	RUNTIME_SLOT* slot = Find_Slot(strId);
	if (!slot) return;
	if (slot->pSprite) slot->pSprite->Set_ScenePresentation(enabled);
	for (const auto& sprite : slot->ExtraLayerSprites) if (sprite) sprite->Set_ScenePresentation(enabled);
	for (const auto& sprite : slot->KeyframeSprites) if (sprite) sprite->Set_ScenePresentation(enabled);
}

// 연출 호출자가 준 label과 초 위치로 keyframe 상태를 즉시 계산한다.
bool_t Client::CUILayoutRuntime::Sample_KeyframeAnimation(const string& strId, const string& label, const f32_t seconds)
{
	if (!std::isfinite(seconds) || seconds < 0.f || !Play_KeyframeAnimation(strId, label)) return false;
	auto* slot = Find_Slot(strId);
	slot->fKeyframeElapsedSeconds = seconds;
	Update_KeyframeSlot(*slot, 0.f);
	return true;
}

// 컷신 소유자가 지정한 슬롯만 일반 HUD 숨김 중에도 표시하도록 한다.
void Client::CUILayoutRuntime::Set_SlotCinematicOverlay(const string& strId, const bool_t overlay)
{
	RUNTIME_SLOT* slot = Find_Slot(strId);
	if (!slot) return;
	if (slot->pSprite) slot->pSprite->Set_CinematicOverlay(overlay);
	for (const auto& sprite : slot->ExtraLayerSprites)
		if (sprite) sprite->Set_CinematicOverlay(overlay);
	for (const auto& sprite : slot->KeyframeSprites)
		if (sprite) sprite->Set_CinematicOverlay(overlay);
}

// 화면 소유자가 선택 class의 슬롯을 표시한다. ownerClass 없는 공용 슬롯은 유지한다.
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

// view 전체를 켜고 끈다. keyframe의 실제 재표시는 현재 key 계산을 거친다.
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

// 기본·추가 정적 이미지의 색을 교체한다. keyframe의 개별 tint는 건드리지 않는다.
void Client::CUILayoutRuntime::Set_SlotTint(const string& strId, const float4_t& vTint)
{
	RUNTIME_SLOT* pSlot = Find_Slot(strId);
	if (nullptr == pSlot)
		return;
	RUNTIME_SLOT& Slot = *pSlot;
	if (nullptr != Slot.pSprite)
		Slot.pSprite->Set_Tint(vTint);
	for (const shared_ptr<CUI_Sprite>& pExtra : Slot.ExtraLayerSprites)
		if (nullptr != pExtra)
			pExtra->Set_Tint(vTint);
}

// 정적·keyframe 이미지의 원래 색은 보존하고 공통 표현 배율만 지정한다.
void Client::CUILayoutRuntime::Set_SlotTintMultiplier(
	const string& strId,
	const float4_t& vTintMultiplier)
{
	RUNTIME_SLOT* pSlot = Find_Slot(strId);
	if (nullptr == pSlot)
		return;
	RUNTIME_SLOT& Slot = *pSlot;
	if (nullptr != Slot.pSprite)
		Slot.pSprite->Set_TintMultiplier(vTintMultiplier);
	for (const shared_ptr<CUI_Sprite>& pExtra : Slot.ExtraLayerSprites)
		if (nullptr != pExtra)
			pExtra->Set_TintMultiplier(vTintMultiplier);
	for (const shared_ptr<CUI_Sprite>& pKeySprite : Slot.KeyframeSprites)
		if (nullptr != pKeySprite)
			pKeySprite->Set_TintMultiplier(vTintMultiplier);
}

// 정적 이미지의 RGB를 흰색으로 바꾸고 요청 alpha를 함께 적용한다.
void Client::CUILayoutRuntime::Set_SlotAlpha(const string& strId, f32_t fAlpha)
{
	Set_SlotTint(strId, float4_t(1.f, 1.f, 1.f, fAlpha));
}

// 기본 이미지에 캐시 SRV를 지정한다. 빈 경로·로드 실패의 nullptr는 원래 텍스처 사용으로 이어진다.
void Client::CUILayoutRuntime::Set_SlotTexture(const string& strId, const string& strAssetPath)
{
	RUNTIME_SLOT* pSlot = Find_Slot(strId);
	if (nullptr == pSlot)
		return;
	RUNTIME_SLOT& Slot = *pSlot;
	if (nullptr == Slot.pSprite)
		return;
	if (strAssetPath.empty())
	{
		Slot.pSprite->Set_Texture(nullptr);
		return;
	}
	Slot.pSprite->Set_Texture(m_pTextureCache->Get_Or_Load(strAssetPath));
}

// 초상화 view 등이 만든 SRV를 기본 sprite가 ComPtr로 보유하게 한다.
void Client::CUILayoutRuntime::Set_SlotTextureSRV(
	const string& strId, ComPtr<ID3D11ShaderResourceView> pSRV)
{
	RUNTIME_SLOT* pSlot = Find_Slot(strId);
	if (nullptr == pSlot)
		return;
	RUNTIME_SLOT& Slot = *pSlot;
	if (nullptr != Slot.pSprite)
		Slot.pSprite->Set_Texture(pSRV);
}

// 창 전체 이동·표시에 쓸 ID 목록을 추가 순서대로 복사한다.
vector<string> Client::CUILayoutRuntime::Get_SlotIds() const
{
	vector<string> Ids;
	Ids.reserve(m_Slots.size());
	for (const RUNTIME_SLOT& Slot : m_Slots)
		Ids.push_back(Slot.strId);
	return Ids;
}

// 드래그·메뉴 view가 문서 좌상단을 바꾼다. 정적 이미지는 즉시, keyframe 위치는 다음 갱신에 반영된다.
void Client::CUILayoutRuntime::Set_SlotPosition(const string& strId, f32_t fX, f32_t fY)
{
	RUNTIME_SLOT* pSlot = Find_Slot(strId);
	if (nullptr == pSlot)
		return;
	RUNTIME_SLOT& Slot = *pSlot;
	Slot.fX = fX;
	Slot.fY = fY;
	const f32_t fCenterX = (fX + Slot.fSizeX * 0.5f);
	const f32_t fCenterY = (fY + Slot.fSizeY * 0.5f);
	const f32_t fSizeX = Slot.fSizeX;
	const f32_t fSizeY = Slot.fSizeY;
	if (nullptr != Slot.pSprite)
		Slot.pSprite->Set_Rect(fCenterX, fCenterY, fSizeX, fSizeY);
	for (const shared_ptr<CUI_Sprite>& pExtra : Slot.ExtraLayerSprites)
		if (nullptr != pExtra)
			pExtra->Set_Rect(fCenterX, fCenterY, fSizeX, fSizeY);
}

// 위치·크기를 보관하고 기본·추가 정적 이미지에 중심 좌표로 전달한다.
void Client::CUILayoutRuntime::Set_SlotRect(
	const string& strId, f32_t fX, f32_t fY, f32_t fWidth, f32_t fHeight)
{
	RUNTIME_SLOT* pSlot = Find_Slot(strId);
	if (nullptr == pSlot)
		return;
	RUNTIME_SLOT& Slot = *pSlot;
	Slot.fX = fX;
	Slot.fY = fY;
	Slot.fSizeX = fWidth;
	Slot.fSizeY = fHeight;
	const f32_t fCenterX = (fX + fWidth * 0.5f);
	const f32_t fCenterY = (fY + fHeight * 0.5f);
	if (nullptr != Slot.pSprite)
		Slot.pSprite->Set_Rect(fCenterX, fCenterY, fWidth, fHeight);
	for (const shared_ptr<CUI_Sprite>& pExtra : Slot.ExtraLayerSprites)
		if (nullptr != pExtra)
			pExtra->Set_Rect(fCenterX, fCenterY, fWidth, fHeight);
}

// 게이지 view가 기본 이미지의 U축 표시 비율을 지정한다.
void Client::CUILayoutRuntime::Set_SlotFillRatio(const string& strId, f32_t fFillRatio)
{
	RUNTIME_SLOT* pSlot = Find_Slot(strId);
	if (nullptr == pSlot)
		return;
	RUNTIME_SLOT& Slot = *pSlot;
	if (nullptr != Slot.pSprite)
		Slot.pSprite->Set_FillRatio(fFillRatio);
}

// 쿨타임 view가 기본 이미지의 부채꼴 표시 비율을 지정한다.
void Client::CUILayoutRuntime::Set_SlotArcRatio(const string& strId, f32_t fArcRatio)
{
	RUNTIME_SLOT* pSlot = Find_Slot(strId);
	if (nullptr == pSlot)
		return;
	RUNTIME_SLOT& Slot = *pSlot;
	if (nullptr != Slot.pSprite)
		Slot.pSprite->Set_ArcRatio(fArcRatio);
}

// 미니맵 등의 기본 이미지에서 샘플링할 영역만 바꾼다.
void Client::CUILayoutRuntime::Set_SlotUVWindow(const string& strId, f32_t fOffsetU,
	f32_t fOffsetV, f32_t fScaleU, f32_t fScaleV)
{
	RUNTIME_SLOT* pSlot = Find_Slot(strId);
	if (nullptr == pSlot)
		return;
	RUNTIME_SLOT& Slot = *pSlot;
	if (nullptr != Slot.pSprite)
		Slot.pSprite->Set_UVWindow(float2_t(fOffsetU, fOffsetV), float2_t(fScaleU, fScaleV));
}

// 미니맵 등의 UV 영역을 radian과 월드 종횡비로 회전한다.
void Client::CUILayoutRuntime::Set_SlotUVRotation(const string& strId, f32_t fRadians, f32_t fAspect)
{
	RUNTIME_SLOT* pSlot = Find_Slot(strId);
	if (nullptr == pSlot)
		return;
	if (nullptr != pSlot->pSprite)
		pSlot->pSprite->Set_UVRotation(fRadians, fAspect);
}

// 정적 이미지들을 시계 방향 degree로 회전한다. keyframe 회전은 key가 별도로 결정한다.
void Client::CUILayoutRuntime::Set_SlotRotation(const string& strId, f32_t fDegrees)
{
	RUNTIME_SLOT* pSlot = Find_Slot(strId);
	if (nullptr == pSlot)
		return;
	RUNTIME_SLOT& Slot = *pSlot;
	if (nullptr != Slot.pSprite)
		Slot.pSprite->Set_Rotation(fDegrees);
	for (const shared_ptr<CUI_Sprite>& pExtra : Slot.ExtraLayerSprites)
		if (nullptr != pExtra)
			pExtra->Set_Rotation(fDegrees);
}

// 누락된 ID만 보충한다. sprite 생성에 실패해도 위치용 슬롯은 등록한다.
void Client::CUILayoutRuntime::Ensure_RuntimeSlot(const string& strId, f32_t fX, f32_t fY,
	f32_t fWidth, f32_t fHeight, const string& strTexturePath)
{
	if (nullptr != Find_Slot(strId))
		return;

	RUNTIME_SLOT Slot{};
	Slot.strId = strId;
	Slot.fX = fX;
	Slot.fY = fY;
	Slot.fSizeX = fWidth;
	Slot.fSizeY = fHeight;
	Slot.pSprite = Create_Sprite(fX, fY, fWidth, fHeight, strTexturePath);
	Append_Slot(move(Slot));
}

// 이벤트를 받은 view가 label 재생을 요청한다. 문서·label 확인 뒤 재생 구간과 시간을 교체한다.
bool_t Client::CUILayoutRuntime::Play_KeyframeAnimation(const string& strId, const string& strLabel)
{
	RUNTIME_SLOT* pSlot = Find_Slot(strId, SLOT_REQUIREMENT::KEYFRAME);
	if (nullptr == pSlot)
		return false;
	RUNTIME_SLOT& Slot = *pSlot;

	const KEYFRAME_ANIM_DOCUMENT* pDocument =
		Get_Or_Load_KeyframeAnimation(Slot.strKeyframeAnimationPath);
	if (nullptr == pDocument)
		return false;

	const auto LabelIter = pDocument->Labels.find(strLabel);
	if (pDocument->Labels.end() == LabelIter)
		return false;

	// 재생 구간은 현재 label 이상, 다음 label 미만이다. 다음 label이 없으면 문서 끝까지다.
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

// Update·외부 시각 샘플링이 현재 frame의 마지막 key를 골라 레이어 sprite에 적용한다.
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

	// 반복 문서는 구간 안으로 되돌리고 단발 문서는 마지막 frame에 머무른다.
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

		// 문서 key가 frame 순서로 저장됐다는 전제로 현재 frame 이하의 마지막 key를 선택한다.
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

		// key 위치는 슬롯 좌상단의 상대값이다. 이미지 원래 픽셀 크기에 key 배율과 슬롯 배율을 곱한다.
		const f32_t fLocalScale = Slot.fKeyframeAnimationScale;
		const f32_t fKeyX = Slot.fX + pActiveKey->fX * fLocalScale;
		const f32_t fKeyY = Slot.fY + pActiveKey->fY * fLocalScale;
		const f32_t fKeyWidth = fTexWidth * pActiveKey->fScaleX * fLocalScale;
		const f32_t fKeyHeight = fTexHeight * pActiveKey->fScaleY * fLocalScale;

		pKeySprite->Set_Rect(
			(fKeyX + fKeyWidth * 0.5f),
			(fKeyY + fKeyHeight * 0.5f),
			fKeyWidth, fKeyHeight);
		pKeySprite->Set_Rotation(pActiveKey->fRotationDeg);
		pKeySprite->Set_Texture(pKeySRV);
		pKeySprite->Set_Tint(float4_t(
			pActiveKey->fTintR, pActiveKey->fTintG, pActiveKey->fTintB, pActiveKey->fAlpha));
		pKeySprite->Set_Additive(pActiveKey->bAdditive);
		pKeySprite->Set_FlipX(pActiveKey->bFlipX);
		pKeySprite->Set_Visible(true);
	}
}

// 화면 소유자가 준 초 단위 delta로 진행한다. keyframe과 flipbook은 각자의 종료 규칙을 사용한다.
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
			// 단발 flipbook은 끝나면 숨긴다. keyframe의 마지막 frame 유지와 다른 규칙이다.
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

// 이벤트에 맞춰 flipbook 시간과 첫 이미지로 돌아간다. 표시 여부는 다음 Update가 반영한다.
void Client::CUILayoutRuntime::Restart_Animation(const string& strId)
{
	RUNTIME_SLOT* pSlot = Find_Slot(strId, SLOT_REQUIREMENT::FLIPBOOK);
	if (nullptr == pSlot)
		return;
	RUNTIME_SLOT& Slot = *pSlot;
	Slot.fAnimationElapsed = 0.f;
	Slot.iAnimationFrame = 0;
	if (nullptr != Slot.pSprite)
	{
		Slot.pSprite->Set_Texture(
			m_pTextureCache->Get_Or_Load(Slot.AnimationFramePaths[0]));
	}
}

// 게이지 등의 수치로 frame을 지정한다. 범위를 제한하고 시간도 해당 frame 위치로 맞춘다.
void Client::CUILayoutRuntime::Set_Animation_Frame(const string& strId, int32_t iFrame)
{
	RUNTIME_SLOT* pSlot = Find_Slot(strId, SLOT_REQUIREMENT::FLIPBOOK);
	if (nullptr == pSlot)
		return;
	RUNTIME_SLOT& Slot = *pSlot;
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
}

// 레이드 입장 탭 등이 frame 목록을 교체한다. 양수 fps만 갱신하고 재생을 처음으로 돌린다.
void Client::CUILayoutRuntime::Set_SlotAnimation(const string& strId,
	const vector<string>& Frames, f32_t fFps, bool_t bLoop)
{
	RUNTIME_SLOT* pSlot = Find_Slot(strId);
	if (nullptr == pSlot)
		return;
	RUNTIME_SLOT& Slot = *pSlot;
	Slot.AnimationFramePaths = Frames;
	if (fFps > 0.f)
		Slot.fAnimationFPS = fFps;
	Slot.bAnimationLoop = bLoop;
	Slot.fAnimationElapsed = 0.f;
	Slot.iAnimationFrame = 0;
	// 목록 교체 즉시 첫 이미지를 적용한다. 빈 목록이면 현재 이미지 참조를 바꾸지 않는다.
	if (nullptr != Slot.pSprite && !Frames.empty())
		Slot.pSprite->Set_Texture(m_pTextureCache->Get_Or_Load(Frames[0]));
}

// 연출 준비 단계에서 로드한 SRV의 참조를 경로별 캐시로 넘긴다.
void Client::CUILayoutRuntime::Adopt_Texture(const string& strPath,
	ComPtr<ID3D11ShaderResourceView> pSRV)
{
	m_pTextureCache->Adopt(strPath, std::move(pSRV));
}
