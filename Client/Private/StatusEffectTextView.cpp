#include "StatusEffectTextView.h"

#include "Character.h"
#include "GameInstance.h"
#include "Transform.h"
#include "WorldPlayerNameplateView.h"

#include <algorithm>
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

	   The word starts at initScale (initTextPositionAndScale sets it before the
	   motion runs) and eases down to endScale. Nothing bounces and nothing moves
	   unless the host passes a non-zero xTo/yTo.

	   Which ani a status word uses is NOT settled by shipped data. EFTable_SkillBuff
	   carries a per-buff FontShow of 0..5 that lines up with ani0..ani5, but whether
	   it is the type itself or the type plus one cannot be told from the table: the
	   0-based reading explains ani5 being reachable, the 1-based reading explains
	   FontShow 0 meaning "no word". The mainstream status words (burn, bleed, stun,
	   fear) are FontShow 1, and watching the real client settles it -- they neither
	   bounce (ani1) nor grow from nothing (ani2/ani3), so ani0 is what they use.

	   effectTime, alphaTime, initTextSize, endTextSize, xTo and yTo are host
	   parameters of DamageTextFactory.make() and no shipped table carries them. The
	   only per-buff size data is EFTable_SkillBuff.BuffFontScale, a percentage that
	   is 0 ("host default") on 33523 of its 33527 rows, so the size it calls 100% is
	   the authored 32 pt -- that is what the word ends at here. The rest below is
	   this project's, matched against the real client:
	     * effectTime and alphaTime keep make()'s own 1000 ms defaults.
	     * the word enters 1.6x oversized and eases down to the authored size.
	     * xTo/yTo are 0: retail does not drift the word away at the end. */
	constexpr f32_t EFFECT_TIME_SECONDS = 1.f;
	constexpr f32_t ALPHA_TIME_SECONDS = 1.f;
	constexpr f32_t ALPHA_DELAY_SECONDS = EFFECT_TIME_SECONDS * 0.4f;
	constexpr f32_t SCALE_TIME_SECONDS = EFFECT_TIME_SECONDS * 0.55f;
	constexpr f32_t DRIFT_DELAY_SECONDS = EFFECT_TIME_SECONDS * 0.05f;
	constexpr f32_t DRIFT_TIME_SECONDS = EFFECT_TIME_SECONDS * 0.45f;
	constexpr f32_t INIT_ALPHA = 1.f;
	constexpr f32_t END_ALPHA = 0.f;
	/* ORIGINAL_TEXT_SIZE in the movie, and the size BuffFontScale calls 100%. */
	constexpr f32_t ORIGINAL_TEXT_SIZE_PX = 32.f;
	/* initTextSize / ORIGINAL_TEXT_SIZE and endTextSize / ORIGINAL_TEXT_SIZE. */
	constexpr f32_t INIT_SCALE = 1.6f;
	constexpr f32_t END_SCALE = 1.f;
	constexpr f32_t DRIFT_X_PX = 0.f;
	constexpr f32_t DRIFT_Y_PX = 0.f;
	constexpr f32_t LIFETIME_SECONDS =
		(EFFECT_TIME_SECONDS > ALPHA_DELAY_SECONDS + ALPHA_TIME_SECONDS) ?
		EFFECT_TIME_SECONDS : ALPHA_DELAY_SECONDS + ALPHA_TIME_SECONDS;
	/* Stage height these tweens are authored against; the same 1080 reference the
	damage numbers already scale with. */
	constexpr f32_t STAGE_HEIGHT_PX = 1080.f;
	/* Head-to-neck height, not the nameplate point: CWorldPlayerNameplateView
	anchors at 2.2 m, which sits clear above the head, and the word belongs lower
	than that. Retail's own anchor is a native value like the drift, so this is the
	project's, picked from watching the real client. */
	constexpr f32_t HEAD_OFFSET_METERS = 1.6f;
	/* Two words landing together must not sit on top of each other, and the same
	word twice must not land in the same place. Drawn once per occurrence. Nothing
	in the movie randomises this -- the AS3 has no Math.random and make() takes an
	absolute stage position -- so the range is the project's. */
	constexpr f32_t SCATTER_X_PX = 26.f;
	constexpr f32_t SCATTER_Y_PX = 18.f;
	/* DropShadow angle 45 deg, distance 0, blur 6.0, strength 1.5, opaque black.
	A zero-distance blurred black shadow reads as a soft outline, approximated with
	four offset black passes because SpriteFont cannot blur. */
	constexpr f32_t SHADOW_OFFSET_PX = 1.5f;
	constexpr f32_t SHADOW_ALPHA = 0.75f;
	/* Words stack, so this is the whole arena's cap. The movie's own canvas cap is
	DamageTextTween.DAMAGE_ANI_LIMIT = 20, shared with the damage numbers. */
	constexpr size_t MAX_OCCURRENCES = 12u;

	struct EASE_SEGMENT
	{
		f32_t s;
		f32_t cp;
		f32_t e;
	};

	/* CustomEase.create("customDamageTextEase1", ...) in AdvancedDamageText. */
	constexpr EASE_SEGMENT CUSTOM_DAMAGE_TEXT_EASE_1[] =
	{
		{ 0.f,		0.608f,		0.686f },
		{ 0.686f,	0.764f,		1.f },
	};
	constexpr int32_t EASE_SEGMENT_COUNT = static_cast<int32_t>(
		sizeof(CUSTOM_DAMAGE_TEXT_EASE_1) / sizeof(CUSTOM_DAMAGE_TEXT_EASE_1[0]));

	/* CustomEase.ease(t, b, c, d) with b = 0, c = 1, d = 1: pick the segment the
	ratio falls in, then evaluate that segment's quadratic. */
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

	/* fl.motion.easing.Sine.easeIn with b = 0, c = 1, d = 1. */
	f32_t Ease_SineIn(const f32_t fRatio)
	{
		const f32_t fClamped = (std::clamp)(fRatio, 0.f, 1.f);
		return 1.f - std::cos(fClamped * 3.14159265f * 0.5f);
	}

	/* fl.motion.easing.Cubic.easeIn with b = 0, c = 1, d = 1. */
	f32_t Ease_CubicIn(const f32_t fRatio)
	{
		const f32_t fClamped = (std::clamp)(fRatio, 0.f, 1.f);
		return fClamped * fClamped * fClamped;
	}
}

void Client::CStatusEffectTextView::Submit(const REQUEST& Request)
{
	const shared_ptr<CCharacter> pAnchor = Request.pAnchor.lock();
	if (Request.strWord.empty() || nullptr == pAnchor)
		return;

	const auto Spawned = m_SpawnedKeyByOwner.find(Request.iOwnerEntityId);
	if (m_SpawnedKeyByOwner.end() != Spawned && Spawned->second == Request.iOccurrenceKey)
		return;
	m_SpawnedKeyByOwner[Request.iOwnerEntityId] = Request.iOccurrenceKey;

	/* Words stack on one head, so only anchors that are already gone leave here. */
	m_Occurrences.erase(
		std::remove_if(m_Occurrences.begin(), m_Occurrences.end(),
			[](const OCCURRENCE& Occurrence) { return Occurrence.pAnchor.expired(); }),
		m_Occurrences.end());

	std::uniform_real_distribution<f32_t> ScatterX(-SCATTER_X_PX, SCATTER_X_PX);
	std::uniform_real_distribution<f32_t> ScatterY(-SCATTER_Y_PX, SCATTER_Y_PX);
	OCCURRENCE Occurrence{};
	Occurrence.strWord = Request.strWord;
	Occurrence.iColorRgb = Request.iColorRgb;
	Occurrence.pAnchor = Request.pAnchor;
	Occurrence.vScatterPx = float2_t(ScatterX(m_Scatter), ScatterY(m_Scatter));
	m_Occurrences.push_back(std::move(Occurrence));
	if (m_Occurrences.size() > MAX_OCCURRENCES)
		m_Occurrences.erase(m_Occurrences.begin());
}

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

void Client::CStatusEffectTextView::Render() const
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

		/* Scale and position are two independent tweens with their own lengths and
		their own delays, so neither waits on the other. */
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
		/* The sprite font is baked at its own pixel height, so scale it until one
		line is ORIGINAL_TEXT_SIZE stage px at the current motion scale. */
		const f32_t fDrawScale =
			(ORIGINAL_TEXT_SIZE_PX * fScaleFactor * fStageScale) / vMeasured.y;
		/* CCustomFont::Draw takes the origin as a 0..1 anchor fraction and multiplies
		it by the string's own measured extent, so this is "centered", not pixels. */
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
