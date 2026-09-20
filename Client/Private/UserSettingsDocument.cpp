#include "UserSettingsDocument.h"

#include "DataJson.h"
#include "GameInstance.h"
#include "RuntimeAssetRoot.h"
#include "Sound/Sound_Manager.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <atomic>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>

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

	bool Valid_Settings(const Client::USER_SETTINGS& value, string& error)
	{
		const auto& display = value.Display;
		if (display.width < 640 || display.width > 16384 || display.height < 480 || display.height > 16384 ||
			(display.mode != Client::USER_WINDOW_MODE::WINDOWED && display.mode != Client::USER_WINDOW_MODE::BORDERLESS &&
			 display.mode != Client::USER_WINDOW_MODE::FULLSCREEN))
		{ error = "Invalid display dimensions or window mode."; return false; }
		if (value.Values.size() > 4096) { error = "Too many user setting rows."; return false; }
		for (const auto& [key, number] : value.Values)
			if (key.empty() || key.size() > 256 || !std::isfinite(number) || std::abs(number) > 1000000.f)
			{ error = "Invalid user setting row: " + key; return false; }
		return true;
	}

	bool Read_SettingsBytes(const filesystem::path& path, bool& exists, string& bytes, string& error)
	{
		std::error_code ec;
		exists = filesystem::exists(path, ec);
		if (ec) { error = "Cannot inspect user settings file."; return false; }
		bytes.clear();
		if (!exists) return true;
		const auto size = filesystem::file_size(path, ec);
		if (ec || size > 1024 * 1024) { error = "User settings file is unreadable or too large."; return false; }
		ifstream stream(path, ios::binary);
		if (!stream) { error = "Cannot read user settings file."; return false; }
		bytes.assign(istreambuf_iterator<char>(stream), istreambuf_iterator<char>());
		if (stream.bad() || bytes.size() != size) { error = "User settings file changed during read."; return false; }
		return true;
	}

	bool Parse_Settings(const string& bytes, Client::USER_SETTINGS& settings, string& error)
	{
		using namespace Client;
		DATA_JSON_VALUE root;
		if (!CDataJson::Parse(bytes, root, error)) return false;
		if (!root.Is_Object()) { error = "User settings root must be an object."; return false; }
		const auto schema = root.Find("schema"), version = root.Find("formatVersion");
		const auto display = root.Find("display"), rows = root.Find("values");
		if (!schema || !schema->Is_String() || schema->Get_String() != "lostark.user-settings" ||
			!version || !version->Is_Number() || version->Get_Number() != 1 ||
			!display || !display->Is_Object() || !rows || !rows->Is_Object())
		{ error = "Invalid user settings schema."; return false; }
		const auto width = display->Find("width"), height = display->Find("height"), mode = display->Find("mode");
		if (!width || !height || !mode || !width->Is_Number() || !height->Is_Number() || !mode->Is_String() ||
			!std::isfinite(width->Get_Number()) || !std::isfinite(height->Get_Number()) ||
			width->Get_Number() < 640 || width->Get_Number() > 16384 ||
			height->Get_Number() < 480 || height->Get_Number() > 16384 ||
			std::floor(width->Get_Number()) != width->Get_Number() || std::floor(height->Get_Number()) != height->Get_Number())
		{ error = "Invalid saved display settings."; return false; }
		USER_SETTINGS staged;
		staged.Display.width = static_cast<uint32_t>(width->Get_Number());
		staged.Display.height = static_cast<uint32_t>(height->Get_Number());
		if (mode->Get_String() == "WINDOWED") staged.Display.mode = USER_WINDOW_MODE::WINDOWED;
		else if (mode->Get_String() == "BORDERLESS") staged.Display.mode = USER_WINDOW_MODE::BORDERLESS;
		else if (mode->Get_String() == "FULLSCREEN") staged.Display.mode = USER_WINDOW_MODE::FULLSCREEN;
		else { error = "Unknown saved window mode."; return false; }
		for (const auto& [key, value] : rows->Get_Object())
		{
			if (!value.Is_Number() || !std::isfinite(value.Get_Number()) || std::abs(value.Get_Number()) > 1000000.)
			{ error = "Invalid saved user setting: " + key; return false; }
			staged.Values.emplace(key, static_cast<f32_t>(value.Get_Number()));
		}
		if (!Valid_Settings(staged, error)) return false;
		settings = std::move(staged);
		return true;
	}

	string Serialize_Settings(const Client::USER_SETTINGS& settings)
	{
		using namespace Client;
		const char* mode = settings.Display.mode == USER_WINDOW_MODE::FULLSCREEN ? "FULLSCREEN" :
			settings.Display.mode == USER_WINDOW_MODE::BORDERLESS ? "BORDERLESS" : "WINDOWED";
		ostringstream out;
		out.imbue(std::locale::classic());
		out << std::setprecision(std::numeric_limits<f32_t>::max_digits10);
		out << "{\n  \"schema\": \"lostark.user-settings\",\n  \"formatVersion\": 1,\n  \"display\": {\"width\": "
			<< settings.Display.width << ", \"height\": " << settings.Display.height << ", \"mode\": \"" << mode
			<< "\"},\n  \"values\": {";
		bool first = true;
		for (const auto& [key, value] : settings.Values)
		{
			out << (first ? "\n" : ",\n") << "    \"" << CDataJson::Escape(key) << "\": " << value;
			first = false;
		}
		out << "\n  }\n}\n";
		return out.str();
	}

	struct SettingsSaveLock
	{
		HANDLE handle = CreateMutexW(nullptr, FALSE, L"Local\\LostArk.UserSettings.Save");
		bool owned = false;
		SettingsSaveLock()
		{
			if (!handle) return;
			const DWORD result = WaitForSingleObject(handle, 0);
			owned = result == WAIT_OBJECT_0 || result == WAIT_ABANDONED;
		}
		~SettingsSaveLock() { if (owned) ReleaseMutex(handle); if (handle) CloseHandle(handle); }
	};

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
	return Values == Other.Values && Display == Other.Display;
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
	Apply_PointerSpeed();
}

filesystem::path Client::CUserSettings::Get_SettingsPath()
{
	const DWORD count = GetEnvironmentVariableW(L"LOCALAPPDATA", nullptr, 0);
	if (!count) return {};
	wstring base(count, L'\0');
	const DWORD written = GetEnvironmentVariableW(L"LOCALAPPDATA", base.data(), count);
	if (!written || written >= count) return {};
	base.resize(written);
	return filesystem::path(base) / L"LostArk" / L"UserSettings.json";
}

bool_t Client::CUserSettings::Load_Persisted(string& strOutStatus)
{
	if (m_bPersistenceLoaded)
	{
		strOutStatus = m_bPersistenceValid ? "User settings already loaded." : "The saved settings file is invalid; it was preserved.";
		return m_bPersistenceValid;
	}
	m_bPersistenceLoaded = true;
	const auto path = Get_SettingsPath();
	USER_SETTINGS staged;
	bool exists = false;
	string bytes;
	if (path.empty() || !Read_SettingsBytes(path, exists, bytes, strOutStatus) ||
		(exists && !Parse_Settings(bytes, staged, strOutStatus)))
	{
		m_bPersistenceValid = false;
		if (path.empty()) strOutStatus = "LOCALAPPDATA is unavailable.";
		return false;
	}
	if (exists)
	{
		for (const auto& [key, value] : m_Defaults) staged.Values.emplace(key, value);
		m_Settings = std::move(staged);
	}
	m_bPersistedExists = exists;
	m_PersistedBytes = std::move(bytes);
	strOutStatus = exists ? "User settings loaded." : "No saved settings; using defaults.";
	return true;
}

bool_t Client::CUserSettings::Preview(const USER_SETTINGS& Staged, string& strOutStatus)
{
	if (!Valid_Settings(Staged, strOutStatus)) return false;
	auto preview = Staged;
	preview.Display = m_Settings.Display;
	Apply_Values(preview);
	strOutStatus = "User settings previewed.";
	return true;
}

void Client::CUserSettings::Apply_Values(const USER_SETTINGS& Staged)
{
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
}

bool_t Client::CUserSettings::Commit(const USER_SETTINGS& Staged, string& strOutStatus)
{
	if (!m_bPersistenceLoaded && !Load_Persisted(strOutStatus)) return false;
	if (!m_bPersistenceValid) { strOutStatus = "The saved settings file is invalid; it was preserved."; return false; }
	if (!Valid_Settings(Staged, strOutStatus)) return false;
	SettingsSaveLock lock;
	if (!lock.owned) { strOutStatus = "Another client is saving settings. Try Apply again."; return false; }
	const auto path = Get_SettingsPath();
	const auto fresh = [&]() {
		bool exists = false;
		string bytes;
		if (!Read_SettingsBytes(path, exists, bytes, strOutStatus)) return false;
		if (exists != m_bPersistedExists || bytes != m_PersistedBytes)
		{ strOutStatus = "Saved settings changed outside this client. Restart to load them before saving."; return false; }
		return true;
	};
	if (path.empty() || !fresh()) return false;
	std::error_code ec;
	filesystem::create_directories(path.parent_path(), ec);
	if (ec) { strOutStatus = "Cannot create the user settings directory."; return false; }
	static std::atomic<unsigned long> serial{0};
	const auto suffix = L"." + std::to_wstring(GetCurrentProcessId()) + L"." +
		std::to_wstring(GetTickCount64()) + L"." + std::to_wstring(++serial);
	const filesystem::path temporary = path.wstring() + suffix + L".tmp";
	const filesystem::path backup = path.wstring() + suffix + L".bak";
	const string bytes = Serialize_Settings(Staged);
	USER_SETTINGS verified;
	if (!Parse_Settings(bytes, verified, strOutStatus) || !verified.Has_SameValues(Staged))
	{ strOutStatus = "User settings serialization failed."; return false; }
	HANDLE file = CreateFileW(temporary.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (file == INVALID_HANDLE_VALUE) { strOutStatus = "Cannot stage user settings."; return false; }
	DWORD written = 0;
	const bool wrote = WriteFile(file, bytes.data(), static_cast<DWORD>(bytes.size()), &written, nullptr) &&
		written == bytes.size() && FlushFileBuffers(file);
	CloseHandle(file);
	if (!wrote) { DeleteFileW(temporary.c_str()); strOutStatus = "Cannot flush user settings."; return false; }
	const auto previous = m_Settings.Display;
	const bool displayChanged = Staged.Display != previous;
	if (displayChanged && (!m_DisplayApply || !m_DisplayApply(Staged.Display, strOutStatus)))
	{
		DeleteFileW(temporary.c_str());
		if (!m_DisplayApply) strOutStatus = "Display changes are not ready yet.";
		return false;
	}
	const bool unchanged = fresh();
	const bool saved = unchanged && (m_bPersistedExists ?
		ReplaceFileW(path.c_str(), temporary.c_str(), backup.c_str(), 0, nullptr, nullptr) != FALSE :
		MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_WRITE_THROUGH) != FALSE);
	if (!saved)
	{
		DeleteFileW(temporary.c_str());
		if (unchanged)
		{
			strOutStatus = "Cannot atomically save user settings.";
			/* ReplaceFile can move the old target to its backup before a later rename fails.
			Restore only our exact baseline, and only if the target is absent or our candidate. */
			bool targetExists = false, backupExists = false;
			string targetBytes, backupBytes, recoveryError;
			if (Read_SettingsBytes(backup, backupExists, backupBytes, recoveryError) && backupExists &&
				backupBytes == m_PersistedBytes && Read_SettingsBytes(path, targetExists, targetBytes, recoveryError) &&
				(!targetExists || targetBytes == bytes))
			{
				const DWORD flags = MOVEFILE_WRITE_THROUGH | (targetExists ? MOVEFILE_REPLACE_EXISTING : 0);
				if (!MoveFileExW(backup.c_str(), path.c_str(), flags))
					strOutStatus += " Restore from the preserved .bak file is required.";
			}
		}
		if (displayChanged)
		{
			string rollback;
			if (!m_DisplayApply(previous, rollback)) strOutStatus += " Display rollback failed: " + rollback;
		}
		return false;
	}
	m_PersistedBytes = bytes;
	m_bPersistedExists = true;
	Apply_Values(Staged);
	strOutStatus = "User settings saved.";
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
	/* The retail table defaults the size row to -1 ("pick by resolution"); until the user
	chooses another size, preserve the normal 48 px art independently of the render size. */
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
