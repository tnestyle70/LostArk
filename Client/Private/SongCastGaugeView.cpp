#include <WinSock2.h>

#include "SongCastGaugeView.h"

#include "DataJson.h"
#include "GameInstance.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"
#include "UILabelFont.h"
#include "UILayoutRuntime.h"
#include "Network/PacketMessages.h"

#include <algorithm>
#include <cmath>
#include <fstream>

namespace
{
	/* CommonActionTimingBar nameTF: $YG760 13 px on the 1920 stage, centred over the track. */
	constexpr f32_t CAPTION_PX = 13.f * 2.f / 3.f;
	const wstring_t FONT_YG760 = TEXT("Font_YG760");
	constexpr float4_t COLOR_CAPTION{ 1.f, 1.f, 1.f, 1.f };
	/* Retail plays the instrument of the caster's class family, not one shared clip: the six
	families each have their own recording of every square-hole song (Wwise events
	ui_pc_inst_<family>_<song> in the SOUND_INSTRUMENT package). These are the "return"
	song -- the one the square holes play -- rendered per family into Resources by
	Tools/SoundPipeline/render_events.py. A class with no file falls back to the warrior
	take rather than going silent. */
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
		default:
			return L"Sound/UI/SquareHole/squarehole_song_warrior.wav";
		}
	}

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
}

Client::CSongCastGaugeView::~CSongCastGaugeView() = default;

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

void Client::CSongCastGaugeView::Update(const f32_t fTimeDelta, const HUD_PLAYER_STATE& Player)
{
	(void)fTimeDelta;
	using LostArk::Shared::PLAYER_ACTION_STATE;
	if (!Player.isValid || Player.isPreview ||
		PLAYER_ACTION_STATE::SQUAREHOLE_SONG != Player.eAction || 0u == Player.iActionStartTick)
	{
		m_iShownActionStartTick = 0u;
		Hide();
		return;
	}
	/* Age in Server ticks (30 Hz) over the Shared lock length; the Server releases the action at
	the same count, so the fill reaches the end as the lock ends. */
	constexpr f32_t TICK_HZ = 30.f;
	const f32_t fDurationTicks = static_cast<f32_t>(
		LostArk::Shared::SQUAREHOLE_SONG_DURATION_MS) * TICK_HZ / 1000.f;
	const int32_t iAge = static_cast<int32_t>(Player.iServerTick - Player.iActionStartTick);
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

void Client::CSongCastGaugeView::Render_Text()
{
	if (!m_bVisible || m_strCaption.empty())
		return;
	/* The caption sits inside the bar, centred on the fill track (retail nameTF). */
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
	gameInstance.Draw_Text(strFont, m_strCaption.c_str(),
		float2_t(vPosition.x + 1.f, vPosition.y + 1.f),
		XMVectorSet(0.f, 0.f, 0.f, 0.8f), 0.f, float2_t(0.f, 0.f), fScale);
	gameInstance.Draw_Text(strFont, m_strCaption.c_str(), vPosition,
		XMLoadFloat4(&COLOR_CAPTION), 0.f, float2_t(0.f, 0.f), fScale);
}
