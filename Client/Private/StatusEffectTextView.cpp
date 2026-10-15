#include "StatusEffectTextView.h"

// 살아 있는 캐릭터 anchor에서 Transform을 얻는다.
#include "Character.h"
// 상태 메시지 표시 옵션을 등록 시 확인한다.
#include "UserSettingsDocument.h"
// 카메라·viewport와 폰트 측정·글자 출력을 사용한다.
#include "GameInstance.h"
// 캐릭터의 월드 바닥 위치를 읽는다.
#include "Transform.h"
// 월드 anchor를 화면에 투영하는 helper를 재사용한다.
#include "WorldPlayerNameplateView.h"

// 보간 비율 제한과 만료 항목 삭제를 처리한다.
#include <algorithm>
// delta의 유한성 검사와 Sine 보간에 사용한다.
#include <cmath>

namespace
{
	/* AdvancedDamageText.ani0:

	     alpha = initAlpha;
	     Tween(alphaTime, this, {alpha: endAlpha},
	           {delay: effectTime * 0.4, ease: Cubic.easeIn});
	     Tween(effectTime * 0.55, textGroup,
	           {scaleX: endScale, scaleY: endScale},
	           {ease: customDamageTextEase1});
	     Tween(effectTime * 0.45, textGroup,
	           {x: originX + xTo, y: originY + yTo},
	           {delay: effectTime * 0.05, ease: Sine.easeIn});
	*/
	// AdvancedDamageText.ani0 방식으로 크기를 줄이고 지연 후 투명하게 만든다.
	// 현재 값은 1.6→1배, 이동 0, alpha 지연 0.4초+소거 1초로 총 표시 수명 1.4초다.
	constexpr f32_t EFFECT_TIME_SECONDS = 1.f;
	constexpr f32_t ALPHA_TIME_SECONDS = 1.f;
	constexpr f32_t ALPHA_DELAY_SECONDS = EFFECT_TIME_SECONDS * 0.4f;
	constexpr f32_t SCALE_TIME_SECONDS = EFFECT_TIME_SECONDS * 0.55f;
	constexpr f32_t DRIFT_DELAY_SECONDS = EFFECT_TIME_SECONDS * 0.05f;
	constexpr f32_t DRIFT_TIME_SECONDS = EFFECT_TIME_SECONDS * 0.45f;
	constexpr f32_t INIT_ALPHA = 1.f;
	constexpr f32_t END_ALPHA = 0.f;
	// 원본 문구의 100% 크기는 32픽셀이다.
	constexpr f32_t ORIGINAL_TEXT_SIZE_PX = 32.f;
	// 원본 크기에 대한 시작·종료 배율이다.
	constexpr f32_t INIT_SCALE = 1.6f;
	constexpr f32_t END_SCALE = 1.f;
	constexpr f32_t DRIFT_X_PX = 0.f;
	constexpr f32_t DRIFT_Y_PX = 0.f;
	constexpr f32_t LIFETIME_SECONDS =
		(EFFECT_TIME_SECONDS > ALPHA_DELAY_SECONDS + ALPHA_TIME_SECONDS) ?
		EFFECT_TIME_SECONDS : ALPHA_DELAY_SECONDS + ALPHA_TIME_SECONDS;
	// viewport 높이에 따라 확대할 기준 화면 높이다.
	constexpr f32_t STAGE_HEIGHT_PX = 1080.f;
	// 캐릭터 바닥 위치에서 문구 anchor까지의 프로젝트 월드 높이다.
	constexpr f32_t HEAD_OFFSET_METERS = 1.6f;
	// 발생마다 한 번 적용하는 프로젝트의 무작위 화면 오프셋 범위다.
	constexpr f32_t SCATTER_X_PX = 26.f;
	constexpr f32_t SCATTER_Y_PX = 18.f;
	// SpriteFont의 검은색 네 방향 재출력으로 흐린 외곽선을 근사한다.
	constexpr f32_t SHADOW_OFFSET_PX = 1.5f;
	constexpr f32_t SHADOW_ALPHA = 0.75f;
	// 이 view가 동시에 보관할 최대 문구 수다. 초과하면 가장 먼저 등록한 항목을 지운다.
	constexpr size_t MAX_OCCURRENCES = 12u;

	struct EASE_SEGMENT
	{
		f32_t s;
		f32_t cp;
		f32_t e;
	};

	// AdvancedDamageText의 customDamageTextEase1 구간 제어값이다.
	constexpr EASE_SEGMENT CUSTOM_DAMAGE_TEXT_EASE_1[] =
	{
		{ 0.f,		0.608f,		0.686f },
		{ 0.686f,	0.764f,		1.f },
	};
	constexpr int32_t EASE_SEGMENT_COUNT = static_cast<int32_t>(
		sizeof(CUSTOM_DAMAGE_TEXT_EASE_1) / sizeof(CUSTOM_DAMAGE_TEXT_EASE_1[0]));

	// 0..1 시간 비율을 구간으로 나누고 각 구간의 2차 곡선값을 구한다.
	f32_t Ease_CustomDamageText1(const f32_t fRatio)
	{
		const f32_t fClamped = (std::clamp)(fRatio, 0.f, 1.f);
		int32_t iSegment = static_cast<int32_t>(EASE_SEGMENT_COUNT * fClamped);
		iSegment = (std::min)(iSegment, EASE_SEGMENT_COUNT - 1);
		const f32_t fLocal =
			(fClamped - static_cast<f32_t>(iSegment) / EASE_SEGMENT_COUNT) * EASE_SEGMENT_COUNT;
		const EASE_SEGMENT& Segment = CUSTOM_DAMAGE_TEXT_EASE_1[iSegment];
		return Segment.s + fLocal *
			(2.f * (1.f - fLocal) * (Segment.cp - Segment.s) + fLocal * (Segment.e - Segment.s));
	}

	// 시간 비율을 제한한 뒤 Sine ease-in 값을 구한다.
	f32_t Ease_SineIn(const f32_t fRatio)
	{
		const f32_t fClamped = (std::clamp)(fRatio, 0.f, 1.f);
		return 1.f - std::cos(fClamped * 3.14159265f * 0.5f);
	}

	// 시간 비율을 제한한 뒤 Cubic ease-in 값을 구한다.
	f32_t Ease_CubicIn(const f32_t fRatio)
	{
		const f32_t fClamped = (std::clamp)(fRatio, 0.f, 1.f);
		return fClamped * fClamped * fClamped;
	}
}

// Level의 표시 요청을 검사해 중복 key를 기록하고 발생 항목을 추가한다. 캐릭터는 계속 weak 참조로 둔다.
void Client::CStatusEffectTextView::Submit(const REQUEST& Request)
{
	const shared_ptr<CCharacter> pAnchor = Request.pAnchor.lock();
	if (Request.strWord.empty() || nullptr == pAnchor)
		return;
	// 환경설정의 상태 메시지 표시가 꺼져 있으면 새 발생을 등록하지 않는다.
	if (!CUserSettings::Get().Is_ConditionMessageShown())
		return;

	const auto ownerWord = std::make_pair(Request.iOwnerEntityId, Request.strWord);
	const auto Spawned = m_SpawnedKeyByOwner.find(ownerWord);
	if (m_SpawnedKeyByOwner.end() != Spawned && Spawned->second == Request.iOccurrenceKey)
		return;
	m_SpawnedKeyByOwner[ownerWord] = Request.iOccurrenceKey;

	// 같은 머리에 여러 문구를 허용한다. 여기서는 anchor가 사라진 기존 항목만 제거한다.
	m_Occurrences.erase(
		std::remove_if(m_Occurrences.begin(), m_Occurrences.end(),
			[](const OCCURRENCE& Occurrence) { return Occurrence.pAnchor.expired(); }),
		m_Occurrences.end());

	std::uniform_real_distribution<f32_t> ScatterX(-SCATTER_X_PX, SCATTER_X_PX);
	std::uniform_real_distribution<f32_t> ScatterY(-SCATTER_Y_PX, SCATTER_Y_PX);
	OCCURRENCE Occurrence{};
	Occurrence.iOwnerEntityId = Request.iOwnerEntityId;
	Occurrence.strWord = Request.strWord;
	Occurrence.iColorRgb = Request.iColorRgb;
	Occurrence.pAnchor = Request.pAnchor;
	Occurrence.vScatterPx = float2_t(ScatterX(m_Scatter), ScatterY(m_Scatter));
	m_Occurrences.push_back(std::move(Occurrence));
	if (m_Occurrences.size() > MAX_OCCURRENCES)
		m_Occurrences.erase(m_Occurrences.begin());
}

// 표시 시간과 anchor 수명만 검사한다. Server의 상태 해제를 직접 검사하지 않는다.
void Client::CStatusEffectTextView::Update(const f32_t fTimeDelta)
{
	if (!std::isfinite(fTimeDelta) || fTimeDelta < 0.f)
		return;
	for (OCCURRENCE& Occurrence : m_Occurrences)
		Occurrence.fAgeSeconds += fTimeDelta;
	m_Occurrences.erase(
		std::remove_if(m_Occurrences.begin(), m_Occurrences.end(),
			[](const OCCURRENCE& Occurrence)
			{
				return Occurrence.fAgeSeconds >= LIFETIME_SECONDS || Occurrence.pAnchor.expired();
			}),
		m_Occurrences.end());
}

// 각 anchor를 잠시 lock하고 머리 높이를 화면에 투영해 크기·alpha·색을 적용한다.
void Client::CStatusEffectTextView::Render(const std::uint32_t onlyOwnerEntityId) const
{
	if (m_Occurrences.empty())
		return;

	CGameInstance& GameInstance = CGameInstance::Get();
	const float4x4_t* const pView = GameInstance.Get_Transform(D3DTS::VIEW);
	const float4x4_t* const pProj = GameInstance.Get_Transform(D3DTS::PROJ);
	const float2_t vViewport = GameInstance.Get_ViewportSize();
	if (nullptr == pView || nullptr == pProj || vViewport.x <= 0.f || vViewport.y <= 0.f)
		return;
	const f32_t fStageScale = vViewport.y / STAGE_HEIGHT_PX;

	for (const OCCURRENCE& Occurrence : m_Occurrences)
	{
		if (onlyOwnerEntityId && Occurrence.iOwnerEntityId != onlyOwnerEntityId) continue;
		const shared_ptr<CCharacter> pAnchor = Occurrence.pAnchor.lock();
		if (nullptr == pAnchor)
			continue;
		const shared_ptr<CTransform> pTransform = pAnchor->Get_Transform();
		if (nullptr == pTransform)
			continue;

		float3_t vHead{};
		XMStoreFloat3(&vHead, pTransform->Get_State(STATE::POSITION));
		vHead.y += HEAD_OFFSET_METERS;
		float2_t vScreen{};
		if (!CWorldPlayerNameplateView::Try_ProjectWorldPosition(
			vHead, *pView, *pProj, vViewport, vScreen))
		{
			continue;
		}

		// 크기와 위치는 별도의 기간·지연으로 보간한다. 현재 이동량 상수는 0이다.
		const f32_t fScaleFactor = INIT_SCALE + (END_SCALE - INIT_SCALE) *
			Ease_CustomDamageText1(Occurrence.fAgeSeconds / SCALE_TIME_SECONDS);
		const f32_t fDrift = Occurrence.fAgeSeconds <= DRIFT_DELAY_SECONDS ? 0.f :
			Ease_SineIn((Occurrence.fAgeSeconds - DRIFT_DELAY_SECONDS) / DRIFT_TIME_SECONDS);
		if (fScaleFactor <= 0.f)
			continue;

		f32_t fAlpha = INIT_ALPHA;
		if (Occurrence.fAgeSeconds > ALPHA_DELAY_SECONDS)
		{
			const f32_t fEased = Ease_CubicIn(
				(Occurrence.fAgeSeconds - ALPHA_DELAY_SECONDS) / ALPHA_TIME_SECONDS);
			fAlpha = INIT_ALPHA + (END_ALPHA - INIT_ALPHA) * fEased;
		}
		if (fAlpha <= 0.f)
			continue;

		const float2_t vMeasured =
			GameInstance.Measure_Text(TEXT("Font_YG760"), Occurrence.strWord.c_str());
		if (vMeasured.y <= 0.f)
			continue;
		// 측정한 글꼴 높이를 현재 동작 배율의 32픽셀 기준 높이에 맞춘다.
		const f32_t fDrawScale =
			(ORIGINAL_TEXT_SIZE_PX * fScaleFactor * fStageScale) / vMeasured.y;
		// Draw_Text의 origin은 픽셀이 아닌 글자 크기에 대한 0..1 비율이다. 중앙 정렬에 0.5를 쓴다.
		constexpr float2_t CENTER_ORIGIN(0.5f, 0.5f);
		const float2_t vPosition(
			vScreen.x + (Occurrence.vScatterPx.x + DRIFT_X_PX * fDrift) * fStageScale,
			vScreen.y + (Occurrence.vScatterPx.y + DRIFT_Y_PX * fDrift) * fStageScale);

		const f32_t fShadow = SHADOW_OFFSET_PX * fStageScale;
		const fvector_t vShadowColor = XMVectorSet(0.f, 0.f, 0.f, fAlpha * SHADOW_ALPHA);
		const float2_t ShadowOffsets[] =
		{
			float2_t(-fShadow, 0.f), float2_t(fShadow, 0.f),
			float2_t(0.f, -fShadow), float2_t(0.f, fShadow),
		};
		for (const float2_t& vOffset : ShadowOffsets)
		{
			GameInstance.Draw_Text(TEXT("Font_YG760"), Occurrence.strWord.c_str(),
				float2_t(vPosition.x + vOffset.x, vPosition.y + vOffset.y),
				vShadowColor, 0.f, CENTER_ORIGIN, fDrawScale);
		}

		const fvector_t vColor = XMVectorSet(
			static_cast<f32_t>((Occurrence.iColorRgb >> 16) & 0xFFu) / 255.f,
			static_cast<f32_t>((Occurrence.iColorRgb >> 8) & 0xFFu) / 255.f,
			static_cast<f32_t>(Occurrence.iColorRgb & 0xFFu) / 255.f,
			fAlpha);
		GameInstance.Draw_Text(TEXT("Font_YG760"), Occurrence.strWord.c_str(),
			vPosition, vColor, 0.f, CENTER_ORIGIN, fDrawScale);
	}
}
