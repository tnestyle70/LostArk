#include "UserSettingsDocument.h"

#include "GameInstance.h"
#include "RuntimeAssetRoot.h"
#include "Sound/Sound_Manager.h"

#include <algorithm>
#include <cmath>
#include <iterator>

namespace
{
	constexpr f32_t VOLUME_MAXIMUM = 100.f;
	constexpr f32_t BRIGHTNESS_NEUTRAL = 50.f;
	/* Brightness scales the profile's own tone-map gamma rather than replacing it, so a
	Level that authored a darker or brighter look keeps its relative mood. 0 -> x0.75,
	50 -> x1.0, 100 -> x1.25; CRenderingProfileService validates the 1..3 range after. */
	constexpr f32_t BRIGHTNESS_GAMMA_SPAN = 0.5f;

	/* Retail cursor size choices in order (normal / large / full / xlarge / xxlarge) ->
	the EFUI_CURSOR size folder. Pixel order, not name order: `large` (64) is smaller than
	`full` (96). */
	const char* CURSOR_SIZE_FOLDERS[] = { "normal", "large", "full", "xlarge", "xxlarge" };
	/* Retail cursor outline choices (none / blue / black / white) -> export name suffix. */
	const char* CURSOR_OUTLINE_SUFFIXES[] = { "", "_blue", "_black", "_white" };

	/* The one cursor this client ever shows is the arrow; the class cursor is replaced in
	place. Loaded cursors are owned here so a later choice can destroy the previous one. */
	HCURSOR s_hUserCursor = nullptr;
}

f32_t Client::USER_SETTINGS::Get(const string& strRowId, const f32_t fFallback) const
{
	const auto found = Values.find(strRowId);
	return Values.end() == found ? fFallback : found->second;
}

bool_t Client::USER_SETTINGS::Get_Flag(const string& strRowId, const bool_t bFallback) const
{
	const auto found = Values.find(strRowId);
	return Values.end() == found ? bFallback : found->second != 0.f;
}

bool_t Client::USER_SETTINGS::Has_SameValues(const USER_SETTINGS& Other) const
{
	return Values == Other.Values;
}

Client::CUserSettings& Client::CUserSettings::Get()
{
	static CUserSettings s_Instance;
	return s_Instance;
}

Client::CUserSettings::~CUserSettings()
{
	if (m_bCursorClipped)
		ClipCursor(nullptr);
	if (m_bPointerChanged)
	{
		SystemParametersInfoW(SPI_SETMOUSESPEED, 0,
			reinterpret_cast<PVOID>(static_cast<INT_PTR>(m_iBaselineMouseSpeed)), SPIF_SENDCHANGE);
		SystemParametersInfoW(SPI_SETMOUSE, 0, m_BaselineMouseAcceleration, SPIF_SENDCHANGE);
	}
}

void Client::CUserSettings::Set_Default(const string& strRowId, const f32_t fDefault)
{
	if (!std::isfinite(fDefault))
		return;
	m_Defaults[strRowId] = fDefault;
	if (m_Settings.Values.end() == m_Settings.Values.find(strRowId))
		m_Settings.Values[strRowId] = fDefault;
}

void Client::CUserSettings::Initialize()
{
	Apply_Audio();
	Apply_Cursor();
}

bool_t Client::CUserSettings::Commit(
	const USER_SETTINGS& Staged, string& strOutStatus)
{
	for (const auto& [strRowId, fValue] : Staged.Values)
	{
		if (!std::isfinite(fValue))
		{
			strOutStatus = "User settings value '" + strRowId + "' is not finite.";
			return false;
		}
	}
	const auto Changed = [this, &Staged](const char* pRow)
		{ return m_Settings.Get(pRow, 0.f) != Staged.Get(pRow, 0.f); };
	const bool_t bCursorChanged =
		Changed(SystemOptionRowId::CURSOR_PRESET) ||
		Changed(SystemOptionRowId::CURSOR_PRESET_SIZE) ||
		Changed(SystemOptionRowId::CURSOR_PRESET_OUTLINE);
	const bool_t bPointerChanged =
		Changed(SystemOptionRowId::POINTER_SPEED) ||
		Changed(SystemOptionRowId::POINTER_ACCELERATE);
	m_Settings = Staged;
	Apply_Audio();
	if (bCursorChanged)
		Apply_Cursor();
	if (bPointerChanged)
		Apply_PointerSpeed();
	strOutStatus = "User settings committed.";
	return true;
}

void Client::CUserSettings::Update_CursorLock(const bool_t bWindowFocused)
{
	const bool_t bWantClip = bWindowFocused && m_Settings.Get_Flag(SystemOptionRowId::MOUSE_LOCK, false);
	if (!bWantClip)
	{
		if (m_bCursorClipped)
			ClipCursor(nullptr);
		m_bCursorClipped = false;
		return;
	}
	/* Re-applied every frame so a moved or resized window keeps the cursor inside. */
	RECT rcClient{};
	if (nullptr == g_hWnd || !GetClientRect(g_hWnd, &rcClient))
		return;
	::POINT ptTopLeft{ rcClient.left, rcClient.top };
	::POINT ptBottomRight{ rcClient.right, rcClient.bottom };
	ClientToScreen(g_hWnd, &ptTopLeft);
	ClientToScreen(g_hWnd, &ptBottomRight);
	const RECT rcScreen{ ptTopLeft.x, ptTopLeft.y, ptBottomRight.x, ptBottomRight.y };
	m_bCursorClipped = 0 != ClipCursor(&rcScreen);
}

void Client::CUserSettings::Apply_PointerSpeed()
{
	const f32_t fSpeed = m_Settings.Get(SystemOptionRowId::POINTER_SPEED, 50.f);
	const f32_t fAccelerate = m_Settings.Get(SystemOptionRowId::POINTER_ACCELERATE, 50.f);
	if (!m_bPointerBaselineTaken)
	{
		/* Untouched at 50 / 50 -- the retail defaults mean "leave Windows alone". */
		if (50.f == fSpeed && 50.f == fAccelerate)
			return;
		INT_PTR iSpeed = 10;
		if (SystemParametersInfoW(SPI_GETMOUSESPEED, 0, &iSpeed, 0))
			m_iBaselineMouseSpeed = static_cast<int32_t>(iSpeed);
		int32_t Acceleration[3] = { 6, 10, 1 };
		if (SystemParametersInfoW(SPI_GETMOUSE, 0, Acceleration, 0))
			std::copy(std::begin(Acceleration), std::end(Acceleration), std::begin(m_BaselineMouseAcceleration));
		m_bPointerBaselineTaken = true;
	}
	/* Windows' own scale: speed 1..20 (10 is the control panel default), acceleration off /
	one threshold / two thresholds -- the slider thirds map onto those three levels. */
	const int32_t iSpeed = (std::clamp)(1 + static_cast<int32_t>(std::lround(fSpeed * 19.f / 100.f)), 1, 20);
	int32_t Acceleration[3] = { 6, 10, fAccelerate < 34.f ? 0 : (fAccelerate < 67.f ? 1 : 2) };
	SystemParametersInfoW(SPI_SETMOUSESPEED, 0,
		reinterpret_cast<PVOID>(static_cast<INT_PTR>(iSpeed)), SPIF_SENDCHANGE);
	SystemParametersInfoW(SPI_SETMOUSE, 0, Acceleration, SPIF_SENDCHANGE);
	m_bPointerChanged = true;
}

f32_t Client::CUserSettings::Get_DamageFontScale() const
{
	static constexpr f32_t SCALES[] = { 0.75f, 1.f, 1.5f, 2.f, 3.f };
	const int32_t iChoice = static_cast<int32_t>(m_Settings.Get(SystemOptionRowId::BATTLE_FONT_SIZE, 1.f));
	return iChoice >= 0 && iChoice < static_cast<int32_t>(std::size(SCALES)) ? SCALES[iChoice] : 1.f;
}

bool_t Client::CUserSettings::Is_DamageNumberShown() const
{
	return m_Settings.Get_Flag(SystemOptionRowId::SHOW_DAMAGE, true);
}

bool_t Client::CUserSettings::Is_ConditionMessageShown() const
{
	return m_Settings.Get_Flag(SystemOptionRowId::SHOW_CONDITION_MESSAGE, true);
}

bool_t Client::CUserSettings::Is_SkillCameraShakeOn() const
{
	return m_Settings.Get_Flag(SystemOptionRowId::SKILL_CAMERA_SHAKE, true);
}

bool_t Client::CUserSettings::Is_MouseButtonSwapped() const
{
	return m_Settings.Get_Flag(SystemOptionRowId::MOUSE_BUTTON_SWAP, false);
}

int32_t Client::CUserSettings::Get_FpsDisplayMode() const
{
	return static_cast<int32_t>(m_Settings.Get(SystemOptionRowId::FPS_DISPLAY, 1.f));
}

int32_t Client::CUserSettings::Get_FrameLimit(const bool_t bForeground) const
{
	const char* pOn = bForeground ? SystemOptionRowId::FPS_LIMIT_FOREGROUND_ON : SystemOptionRowId::FPS_LIMIT_BACKGROUND_ON;
	const char* pValue = bForeground ? SystemOptionRowId::FPS_LIMIT_FOREGROUND : SystemOptionRowId::FPS_LIMIT_BACKGROUND;
	if (!m_Settings.Get_Flag(pOn, false))
		return 0;
	const int32_t iLimit = static_cast<int32_t>(m_Settings.Get(pValue, 0.f));
	return iLimit > 0 ? iLimit : 0;
}

bool_t Client::CUserSettings::Is_NametagShown(const PLAYER_RELATION eRelation, const bool_t bHonorTitle) const
{
	switch (eRelation)
	{
	case PLAYER_RELATION::LOCAL:
		return m_Settings.Get_Flag(bHonorTitle ? SystemOptionRowId::PLAYER_NAMETAG_TITLE : SystemOptionRowId::PLAYER_NAMETAG_NAME, true);
	case PLAYER_RELATION::PARTY:
		return m_Settings.Get_Flag(bHonorTitle ? SystemOptionRowId::PARTY_NAMETAG_TITLE : SystemOptionRowId::PARTY_NAMETAG_NAME, true);
	default:
		return m_Settings.Get_Flag(bHonorTitle ? SystemOptionRowId::OTHER_NAMETAG_TITLE : SystemOptionRowId::OTHER_NAMETAG_NAME, true);
	}
}

bool_t Client::CUserSettings::Is_ChatBubbleShown(const PLAYER_RELATION eRelation) const
{
	switch (eRelation)
	{
	case PLAYER_RELATION::LOCAL: return m_Settings.Get_Flag(SystemOptionRowId::PLAYER_BALLOON, true);
	case PLAYER_RELATION::PARTY: return m_Settings.Get_Flag(SystemOptionRowId::PARTY_BALLOON, true);
	default: return m_Settings.Get_Flag(SystemOptionRowId::OTHER_BALLOON, true);
	}
}

void Client::CUserSettings::Apply_Audio() const
{
	const auto BusVolume = [this](const char* pOnRow, const char* pVolumeRow)
		{
			const f32_t fDefault = m_Defaults.count(pVolumeRow) ? m_Defaults.at(pVolumeRow) : 100.f;
			if (!m_Settings.Get_Flag(pOnRow, true))
				return 0.f;
			const f32_t fVolume = m_Settings.Get(pVolumeRow, fDefault);
			return (fVolume < 0.f ? 0.f : (fVolume > VOLUME_MAXIMUM ? VOLUME_MAXIMUM : fVolume)) / VOLUME_MAXIMUM;
		};
	CGameInstance& Instance = CGameInstance::Get();
	Instance.Apply_SoundCategoryVolume(SOUND_CATEGORY::MASTER,
		BusVolume(SystemOptionRowId::MASTER_ON, SystemOptionRowId::MASTER_VOLUME));
	Instance.Apply_SoundCategoryVolume(SOUND_CATEGORY::MUSIC,
		BusVolume(SystemOptionRowId::MUSIC_ON, SystemOptionRowId::MUSIC_VOLUME));
	Instance.Apply_SoundCategoryVolume(SOUND_CATEGORY::EFFECT,
		BusVolume(SystemOptionRowId::EFFECT_ON, SystemOptionRowId::EFFECT_VOLUME));
	Instance.Apply_SoundCategoryVolume(SOUND_CATEGORY::INTERFACE,
		BusVolume(SystemOptionRowId::UI_ON, SystemOptionRowId::UI_VOLUME));
	/* sound-in-background: whether losing focus mutes -- the check the mixer already
	performs every frame, now with the user's answer. */
	Instance.Set_SoundMuteOnFocusLoss(!m_Settings.Get_Flag(SystemOptionRowId::BACKGROUND_SOUND, true));
}

void Client::CUserSettings::Apply_Video(RENDER_QUALITY_SETTINGS& Quality) const
{
	const f32_t fBrightness = m_Settings.Get(SystemOptionRowId::BRIGHTNESS, BRIGHTNESS_NEUTRAL);
	const f32_t fBrightnessScale = 1.f + BRIGHTNESS_GAMMA_SPAN *
		(fBrightness - BRIGHTNESS_NEUTRAL) / VOLUME_MAXIMUM;
	const f32_t fGamma = Quality.fGamma * fBrightnessScale;
	Quality.fGamma = fGamma < 1.f ? 1.f : (fGamma > 3.f ? 3.f : fGamma);

	if (!m_Settings.Get_Flag(SystemOptionRowId::BLOOM, true))
		Quality.bBloomEnabled = false;
	/* Retail's anti-aliasing / SSAO combos: 0 high, 1 low, 2 off. The renderer has FXAA and
	SSAO as on/off; the third choice turns them off, the other two leave the profile alone. */
	if (2 == static_cast<int32_t>(m_Settings.Get(SystemOptionRowId::ANTIALIASING, 0.f)))
		Quality.bFXAAEnabled = false;
	if (2 == static_cast<int32_t>(m_Settings.Get(SystemOptionRowId::SSAO, 0.f)))
		Quality.bSSAOEnabled = false;

	/* Accessibility colour-vision filter: the type combo picks the deficiency, the slider
	how far the corrected colour replaces the original (50 = half way). */
	const int32_t iFilter = static_cast<int32_t>(m_Settings.Get(SystemOptionRowId::COLOR_FILTER_TYPE, 0.f));
	const f32_t fStrength = m_Settings.Get(SystemOptionRowId::COLOR_FILTER_VALUE, 50.f) / VOLUME_MAXIMUM;
	Quality.iColorFilterType = iFilter >= 0 && iFilter <= 3 ? iFilter : 0;
	Quality.fColorFilterStrength = fStrength < 0.f ? 0.f : (fStrength > 1.f ? 1.f : fStrength);
}

void Client::CUserSettings::Apply_Cursor() const
{
	const int32_t iPreset = static_cast<int32_t>(m_Settings.Get(SystemOptionRowId::CURSOR_PRESET, 0.f));
	/* The retail table defaults the size row to -1 ("pick by resolution"); at this client's
	1280x720 that is the normal 48 px art, so a negative size reads as the first choice
	instead of aborting the whole apply. */
	int32_t iSize = static_cast<int32_t>(m_Settings.Get(SystemOptionRowId::CURSOR_PRESET_SIZE, 0.f));
	if (iSize < 0)
		iSize = 0;
	const int32_t iOutline = static_cast<int32_t>(m_Settings.Get(SystemOptionRowId::CURSOR_PRESET_OUTLINE, 0.f));
	if (iSize >= static_cast<int32_t>(std::size(CURSOR_SIZE_FOLDERS)) ||
		iOutline < 0 || iOutline >= static_cast<int32_t>(std::size(CURSOR_OUTLINE_SUFFIXES)) ||
		iPreset < 0 || iPreset > 6)
	{
		return;
	}

	/* <preset>/<size>/normal<outline>.{ani,cur}: the manifest lists what exists; a missing
	combination keeps the current cursor rather than blanking it. Outline variants have no
	preset6 (mokoko) art in retail either, so the outline is ignored there. */
	const string strPreset = 0 == iPreset ? "default" : "preset" + std::to_string(iPreset);
	const string strName = string("normal") + (6 == iPreset ? "" : CURSOR_OUTLINE_SUFFIXES[iOutline]);
	const string strBase = "UI/Cursors/" + strPreset + "/" + CURSOR_SIZE_FOLDERS[iSize] + "/" + strName;
	filesystem::path path;
	for (const char* pExtension : { ".ani", ".cur" })
	{
		const filesystem::path candidate =
			CRuntimeAssetRoot::Resolve(filesystem::path(strBase + pExtension));
		error_code error;
		if (!candidate.empty() && filesystem::exists(candidate, error) && !error)
		{
			path = candidate;
			break;
		}
	}
	if (path.empty())
	{
		OutputDebugStringA(("[UserSettings] Cursor art missing: " + strBase + "\n").c_str());
		return;
	}

	/* LoadImage with no size and no LR_DEFAULTSIZE keeps the file's own pixel size (48 .. 192).
	LoadCursorFromFile would shrink every one of them to the 32 px system cursor size, which is
	why the size choice looked like it did nothing. Works for .ani as well as .cur. */
	const HCURSOR hCursor = static_cast<HCURSOR>(LoadImageW(nullptr, path.c_str(),
		IMAGE_CURSOR, 0, 0, LR_LOADFROMFILE));
	if (nullptr == hCursor)
	{
		OutputDebugStringA(("[UserSettings] LoadImage(cursor) failed: " + strBase + "\n").c_str());
		return;
	}
	SetClassLongPtrW(g_hWnd, GCLP_HCURSOR, reinterpret_cast<LONG_PTR>(hCursor));
	SetCursor(hCursor);
	if (nullptr != s_hUserCursor)
		DestroyCursor(s_hUserCursor);
	s_hUserCursor = hCursor;
}
