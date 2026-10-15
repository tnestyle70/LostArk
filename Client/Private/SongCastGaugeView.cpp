// Windows 헤더보다 WinSock2를 먼저 포함하는 Client 공통 순서를 유지한다.
#include <WinSock2.h>

// 복제 action을 게이지·소리·암전 표시로 바꾼다.
#include "SongCastGaugeView.h"

// caption 문서의 JSON 객체와 문자열 필드를 읽는다.
#include "DataJson.h"
// 소리를 재생하고 viewport·폰트 측정과 글자 출력을 사용한다.
#include "GameInstance.h"
// caption JSON을 Data 상대 경로에서 찾는다.
#include "ProjectDataRoot.h"
// 연주 소리를 Resources 상대 경로에서 찾는다.
#include "RuntimeAssetRoot.h"
// 화면의 글자 크기에 맞는 등록 폰트 tag와 배율을 선택한다.
#include "UILabelFont.h"
// 게이지·암전 sprite를 만들고 표시·채움·색을 바꾼다.
#include "UILayoutRuntime.h"
// Shared의 연주·암전·Bern 이동 기간 상수를 사용한다.
#include "Network/PacketMessages.h"

// 진행 비율과 alpha를 clamp하고 글자 배율의 min을 구한다.
#include <algorithm>
// caption의 출력 위치를 round로 픽셀에 맞춘다.
#include <cmath>
// WorldMapPanels.json을 파일 스트림으로 읽는다.
#include <fstream>

namespace
{
	// 1920 기준 원본 13픽셀 글자 크기를 1280 기준으로 환산한다.
	constexpr f32_t CAPTION_PX = 13.f * 2.f / 3.f;
	const wstring_t FONT_YG760 = TEXT("Font_YG760");
	// 기존 KoukuFade 문서의 전체 화면 검정 슬롯 ID를 재사용한다.
	const string FADE_SLOT = "KoukuFade_Screen";
	constexpr f32_t FADE_IN_SECONDS = 0.5f;
	constexpr float4_t COLOR_CAPTION{ 1.f, 1.f, 1.f, 1.f };
	// 직업 계열별 스퀘어홀 소리 asset을 고른다. 별도 분기가 없는 직업은 warrior 경로를 쓴다.
	const wchar_t* Song_Sound_For(LostArk::Shared::CHARACTER_CLASS_ID eClass)
	{
		switch (eClass)
		{
		case LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER:
			return L"Sound/UI/SquareHole/squarehole_song_fighter.wav";
		case LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER:
			return L"Sound/UI/SquareHole/squarehole_song_gunner.wav";
		case LostArk::Shared::CHARACTER_CLASS_ID::ARTIST:
			return L"Sound/UI/SquareHole/squarehole_song_specialist.wav";
		case LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER:
			return L"Sound/UI/SquareHole/squarehole_song_assassin.wav";
		case LostArk::Shared::CHARACTER_CLASS_ID::SLAYER:
		case LostArk::Shared::CHARACTER_CLASS_ID::DESTROYER:
		case LostArk::Shared::CHARACTER_CLASS_ID::WARLORD:
		case LostArk::Shared::CHARACTER_CLASS_ID::GUARDIANKNIGHT:
		default:
			return L"Sound/UI/SquareHole/squarehole_song_warrior.wav";
		}
	}

	// caption의 UTF-8 바이트를 Windows wide 문자열로 바꾼다. 빈 문자열은 출력을 비운다.
	bool_t Convert_Utf8(const string& strUtf8, wstring_t& outWide)
	{
		if (strUtf8.empty())
		{
			outWide.clear();
			return true;
		}
		const int32_t iLength = MultiByteToWideChar(
			CP_UTF8, 0, strUtf8.c_str(), static_cast<int32_t>(strUtf8.size()), nullptr, 0);
		if (iLength <= 0)
			return false;
		outWide.resize(static_cast<size_t>(iLength));
		return 0 != MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(),
			static_cast<int32_t>(strUtf8.size()), outWide.data(), iLength);
	}
}

Client::CSongCastGaugeView::CSongCastGaugeView(
	ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pView{ std::make_unique<CUILayoutRuntime>(
		pDevice, pContext, ETOUI(LEVEL::STATIC), TEXT("Layer_UI"),
		L"UI/WorldMap/SongCastGauge_Layout.json") }
{
	Load_Strings();
	Hide();
	// 같은 정렬 layer에서 뒤에 만든 암전 sprite가 게이지 위에 그려지도록 순서를 잡는다.
	m_pFadeView = std::make_unique<CUILayoutRuntime>(
		pDevice, pContext, ETOUI(LEVEL::STATIC), TEXT("Layer_UI"),
		L"UI/KoukuFade/KoukuFadeUI.json");
	m_pFadeView->Set_SlotVisible(FADE_SLOT, false);
	m_pFadeView->Set_SlotCinematicOverlay(FADE_SLOT, true);
}

Client::CSongCastGaugeView::~CSongCastGaugeView() = default;

// 파일·JSON·필드 확인에 실패하면 현재 caption을 그대로 둔다.
void Client::CSongCastGaugeView::Load_Strings()
{
	const filesystem::path DataPath = CProjectDataRoot::Resolve(L"UI/WorldMap/WorldMapPanels.json");
	ifstream Stream(DataPath, ios::binary);
	if (!Stream.is_open())
		return;
	const string Text((istreambuf_iterator<char>(Stream)), istreambuf_iterator<char>());
	DATA_JSON_VALUE Root;
	string Error;
	if (!CDataJson::Parse(Text, Root, Error) || !Root.Is_Object())
		return;
	const DATA_JSON_VALUE* pStrings = Root.Find("strings");
	const DATA_JSON_VALUE* pCaption = (nullptr != pStrings && pStrings->Is_Object()) ?
		pStrings->Find("songCasting") : nullptr;
	if (nullptr != pCaption && pCaption->Is_String())
		(void)Convert_Utf8(pCaption->Get_String(), m_strCaption);
}

void Client::CSongCastGaugeView::Hide()
{
	m_bVisible = false;
	m_pView->Set_AllSlotsVisible(false);
}

// MainApp::Update_WorldMap이 호출한다. 연주 action 시작 tick별로 소리를 한 번 재생한다.
void Client::CSongCastGaugeView::Update(
	const f32_t fTimeDelta, const HUD_PLAYER_STATE& Player, const bool_t bTravelFadeLevel)
{
	using LostArk::Shared::PLAYER_ACTION_STATE;
	const bool_t bInSong = Player.isValid && !Player.isPreview &&
		PLAYER_ACTION_STATE::SQUAREHOLE_SONG == Player.eAction && 0u != Player.iActionStartTick;
	const int32_t iAge = bInSong ?
		static_cast<int32_t>(Player.iServerTick - Player.iActionStartTick) : 0;
	// Bern에서만 요청한 trigger 이동 암전이다. 자체 암전이 있는 쿠크에는 중복 적용하지 않는다.
	const bool_t bInTravel = bTravelFadeLevel && Player.isValid && !Player.isPreview &&
		PLAYER_ACTION_STATE::TRIGGER_MOVE == Player.eAction && 0u != Player.iActionStartTick;
	Update_Fade(fTimeDelta, bInSong, iAge, bInTravel);
	if (!bInSong)
	{
		m_iShownActionStartTick = 0u;
		Hide();
		return;
	}
	// 30Hz 서버 tick과 Shared의 밀리초 기간을 같은 tick 단위로 맞춰 채움 비율을 구한다.
	constexpr f32_t TICK_HZ = 30.f;
	const f32_t fDurationTicks = static_cast<f32_t>(
		LostArk::Shared::SQUAREHOLE_SONG_DURATION_MS) * TICK_HZ / 1000.f;
	m_fFill = std::clamp(static_cast<f32_t>((std::max)(iAge, 0)) / (std::max)(fDurationTicks, 1.f), 0.f, 1.f);

	if (m_iShownActionStartTick != Player.iActionStartTick)
	{
		m_iShownActionStartTick = Player.iActionStartTick;
		const filesystem::path SoundPath =
			CRuntimeAssetRoot::Resolve(Song_Sound_For(Player.eCharacterClass));
		if (!SoundPath.empty() && filesystem::exists(SoundPath))
			CGameInstance::Get().Play_Sound(SoundPath.wstring(), 1.f);
	}
	m_bVisible = true;
	m_pView->Set_SlotVisible("SC_Plate", true);
	m_pView->Set_SlotVisible("SC_Fill", true);
	m_pView->Set_SlotFillRatio("SC_Fill", m_fFill);
}

// action 상태에서 정한 방향으로 alpha만 갱신한다. 서버 이동이나 action 상태는 수정하지 않는다.
void Client::CSongCastGaugeView::Update_Fade(
	const f32_t fTimeDelta, const bool_t bInSong, const int32_t iAgeTicks, const bool_t bInTravel)
{
	// 서버 tick이 연주 끝 직전 구간에 들어오면 암전을 시작하고 연주 action이 끝나면 해제한다.
	// 일단 시작한 alpha 변화는 프레임 delta로 진행하므로 snapshot 간격과 분리된다.
	constexpr f32_t TICK_HZ = 30.f;
	constexpr f32_t FADE_OUT_SECONDS =
		static_cast<f32_t>(LostArk::Shared::SQUAREHOLE_BLACKOUT_FADE_MS) / 1000.f;
	constexpr f32_t FADE_START_TICKS =
		static_cast<f32_t>(LostArk::Shared::SQUAREHOLE_SONG_DURATION_MS -
			LostArk::Shared::SQUAREHOLE_BLACKOUT_FADE_MS) * TICK_HZ / 1000.f;
	if (!bInSong)
		m_bFadingOut = false;
	else if (static_cast<f32_t>(iAgeTicks) >= FADE_START_TICKS)
		m_bFadingOut = true;
	// Bern 이동은 별도 암전 시간으로 진행한다. 연주 여부와 합쳐 현재 어두워질 방향을 정한다.
	constexpr f32_t TRAVEL_FADE_OUT_SECONDS =
		static_cast<f32_t>(LostArk::Shared::BERN_TRAVEL_FADE_OUT_MS) / 1000.f;
	const bool_t bDarken = m_bFadingOut || bInTravel;
	const f32_t fFadeOutSeconds = m_bFadingOut ? FADE_OUT_SECONDS : TRAVEL_FADE_OUT_SECONDS;
	const f32_t fStep = bDarken ?
		fTimeDelta / fFadeOutSeconds : -fTimeDelta / FADE_IN_SECONDS;
	m_fFadeAlpha = std::clamp(m_fFadeAlpha + fStep, 0.f, 1.f);
	if (nullptr == m_pFadeView)
		return;
	const bool_t bShow = m_fFadeAlpha > 0.f;
	if (bShow != m_bFadeShown)
	{
		m_bFadeShown = bShow;
		m_pFadeView->Set_SlotVisible(FADE_SLOT, bShow);
	}
	// Set_SlotAlpha는 RGB를 흰색으로 바꾸므로 검정 RGB와 alpha를 함께 전달한다.
	if (bShow)
		m_pFadeView->Set_SlotTint(FADE_SLOT, float4_t(0.f, 0.f, 0.f, m_fFadeAlpha));
}

void Client::CSongCastGaugeView::Render_Text()
{
	// 화면이 절반 이상 어두워지면 caption도 생략한다.
	if (!m_bVisible || m_strCaption.empty() || m_fFadeAlpha >= 0.5f)
		return;
	// SC_Fill의 문서 사각형 중심을 viewport 픽셀로 바꿔 caption을 배치한다.
	f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
	if (!m_pView->Get_SlotRect("SC_Fill", fX, fY, fW, fH))
		return;
	CGameInstance& gameInstance = CGameInstance::Get();
	const float2_t vViewport = gameInstance.Get_ViewportSize();
	if (vViewport.x <= 0.f || vViewport.y <= 0.f)
		return;
	const f32_t fScaleX = vViewport.x / m_pView->Get_ResolutionWidth();
	const f32_t fScaleY = vViewport.y / m_pView->Get_ResolutionHeight();
	const f32_t fUiScale = (std::min)(fScaleX, fScaleY);
	f32_t fScale = 1.f;
	const wstring_t strFont = UILabelFont::Resolve(FONT_YG760, CAPTION_PX * fUiScale, fScale);
	const float2_t vMeasured = gameInstance.Measure_Text(strFont, m_strCaption.c_str());
	const float2_t vPosition(
		std::round((fX + fW * 0.5f) * fScaleX - vMeasured.x * fScale * 0.5f),
		std::round((fY + fH * 0.5f) * fScaleY - vMeasured.y * fScale * 0.5f));
	// 1픽셀 어긋난 검정 그림자를 먼저 그리고 같은 문자열의 흰 본문을 겹친다.
	gameInstance.Draw_Text(strFont, m_strCaption.c_str(),
		float2_t(vPosition.x + 1.f, vPosition.y + 1.f),
		XMVectorSet(0.f, 0.f, 0.f, 0.8f), 0.f, float2_t(0.f, 0.f), fScale);
	gameInstance.Draw_Text(strFont, m_strCaption.c_str(), vPosition,
		XMLoadFloat4(&COLOR_CAPTION), 0.f, float2_t(0.f, 0.f), fScale);
}
