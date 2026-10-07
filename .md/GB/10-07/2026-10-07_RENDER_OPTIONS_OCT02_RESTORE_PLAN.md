# 2026-10-07 렌더링 옵션의 10월 2일 값 복원 계획

## G1. 실제 반영 상태와 이번 변경 경계

사용자가 렌더링 옵션을 Git 10월 2일 기준으로 복원하고 무비 최적화·오클루전 등 최적화와 새 실험 기능은 유지하도록 요청했다. 기준은 `a0ff0185cc7fea171835cde15777c0f520d9d0ae`다. 계획서는 제품 파일을 바꾸기 전에 현재 파일에서 생성했다.

현재 `RenderingProfiles.json`과 기준 commit의 실제 옵션 차이는 `scene.bern.neutral-day.v1`의 `fog.enabled` 한 곳이다. 이를 `false`에서 `true`로 되돌리고 저장 revision은 현재 91보다 큰 92로 올린다. 기준 revision 90을 덮어쓰지 않는다. 품질·조명·region 등 나머지 옵션은 기준과 동일하다.

텍스처 초기 기본값은 기준 `SystemOptionRows.json`의 최상 0에 맞춘다. 현재 기본 함수의 Debug `3.f`만 `0.f`로 바꾸며 Release `0.f`, 명시 저장값 우선, `iTextureMinMip` 선택 기능, sampler/mip 처리 및 최적화 코드는 유지한다. 기존 계약 검사의 Debug 기대값과 성공 메시지도 같은 값에 맞춘다. 개인 `UserSettings.json`은 쓰지 않는다.

현재 측정된 AMD iGPU의 9.55 FPS는 이번 옵션 변경 효과의 증거가 아니다. GPU 선택 수정은 다른 작업의 범위이며 이번 복원의 빌드·화면/FPS 검증과 구분한다.

## G1. 수정 파일과 정확한 위치

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 수정 | C:/Users/tnest/Desktop/LostArk/Client/Private/UserSettingsDocument.cpp | `CUserSettings::Get_DefaultTextureQuality`의 Debug fallback을 0으로 복원 |
| 수정 | C:/Users/tnest/Desktop/LostArk/Tools/UserSettingsContractHarness/UserSettingsContractHarness.cpp | 기존 fixture의 Debug 기대값과 성공 설명 정합화 |
| 수정 | C:/Users/tnest/Desktop/LostArk/Data/Rendering/Authored/RenderingProfiles.json | neutral-day fog만 켜고 revision 92 기록 |
| publisher 생성 | C:/Users/tnest/Desktop/LostArk/Client/Bin/DataFiles/Rendering/RenderingProfiles.runtime.json | 검증된 저작 정본 게시 |
| 작성 | C:/Users/tnest/Desktop/LostArk/.md/GB/10-07/2026-10-07_RENDER_OPTIONS_OCT02_RESTORE_PLAN.md | 구현 전 전체 코드와 데이터 블록 |
| 작성 | C:/Users/tnest/Desktop/LostArk/.md/GB/10-07/2026-10-07_RENDER_OPTIONS_OCT02_RESTORE_RESULT.md | 실제 검증과 남은 경계 |
| 정합화, root 담당 | C:/Users/tnest/Desktop/LostArk/CLAUDE.md | Debug와 Release 모두 최상인 기본값 설명 |
| 정합화, root 담당 | C:/Users/tnest/Desktop/LostArk/.md/GB/gotchas.md | Debug/Release 모두 최상이라는 계약 설명 |
| 정합화, root 담당 | C:/Users/tnest/Desktop/LostArk/.md/GB/렌더링이펙트복원V2.md | 기본 0·명시 저장 우선·옵션 복원과 최적화 회귀 구분 |
| 정합화, root 담당 | C:/Users/tnest/Desktop/LostArk/Tools/UserSettingsContractHarness/README.md | 기존 fixture의 기본값 설명 |

## G1. UserSettingsDocument.cpp

변경 종류: 기존 함수의 상수 하나 변경. 적용 위치: `CUserSettings::Get_DefaultTextureQuality()`의 `_DEBUG` 분기.

이 CPP는 사용자 옵션의 로드·검증·메모리 preview·저장·소비를 소유한다. 공개 H의 선언, include, enum, struct와 member는 그대로다. `Get_DefaultTextureQuality`는 누락된 텍스처 선택과 UI 초기화·Reset에서 사용할 인덱스를 돌려준다. `SystemOptionWindowView.cpp`의 기존 `Effective_Default`가 이 함수를 사용하고 `CUserSettings::Apply_Video`도 같은 fallback을 사용한다.

`Set_Default`는 명시적으로 들어온 값을 덮어쓰지 않는다. `Load_Persisted`는 누락 행에만 기본값을 합치고 `Apply_Video`는 선택된 0/1/2/3을 `iTextureMinMip`으로 전달한다. `Commit`의 freshness·백업·원자 교체·실패 rollback은 그대로 유지한다. 이번 변경에 새 저장이나 런타임 경로는 없다.

적용 후 파일 전문:

```cpp
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

f32_t Client::CUserSettings::Get_DefaultTextureQuality() noexcept
{
#ifdef _DEBUG
	return 0.f;
#else
	return 0.f;
#endif
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
	/* Texture choices cap the finest sampled mip; malformed saved row values must not
	be converted to an unsigned index or change the authored surface's original quality. */
	const f32_t fTextureQuality = m_Settings.Get(SystemOptionRowId::TEXTURE_QUALITY, Get_DefaultTextureQuality());
	Quality.iTextureMinMip = std::isfinite(fTextureQuality) && fTextureQuality >= 0.f &&
		fTextureQuality <= 3.f && std::floor(fTextureQuality) == fTextureQuality ?
		static_cast<uint32_t>(fTextureQuality) : 0u;

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

```

## G1. UserSettingsContractHarness.cpp

변경 종류: 기존 Debug 기대값 하나와 그 성공 설명 변경. 적용 위치: `wmain`의 `textureDefault` 선언과 첫 texture 기본값 검사.

이 fixture는 실제 UserSettings CPP와 DataJson을 실행하고 UI의 `Effective_Default` 본문을 추출해 검사한다. 기존 값 선택·누락 행·Reset·취소·재로드·미저장·외부 저장 충돌·실제 Win32 원자 교체 실패 검사는 유지한다. 테스트 프로세스의 `LOCALAPPDATA`만 저장소 `out` 하위로 옮기므로 개인 설정 파일을 사용하지 않는다. 새 검사는 추가하지 않는다.

적용 후 파일 전문:

```cpp
#include <string>
#include "UserSettingsDocument.h"
#include "RuntimeAssetRoot.h"
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>
float EffectiveDefaultForHarness(const std::string& rowId, float authoredDefault);
HINSTANCE g_hInst = nullptr;
HWND g_hWnd = nullptr;
std::filesystem::path Client::CRuntimeAssetRoot::Resolve(const std::filesystem::path&) { return {}; }
namespace {
std::filesystem::path base;
unsigned assertions = 0;
void Check(bool value, const char* description) {
    if (!value) throw std::runtime_error(description);
    ++assertions; std::cout << "PASS " << description << "\n";
}
void Select(const wchar_t* name) {
    auto path=base/name; std::filesystem::create_directories(path);
    if (!SetEnvironmentVariableW(L"LOCALAPPDATA",path.c_str())) throw std::runtime_error("SetEnvironmentVariable");
}
std::string Read() { std::ifstream in(CUserSettings::Get_SettingsPath(),std::ios::binary); return {std::istreambuf_iterator<char>(in),{}}; }
void Write(const std::string& text) { auto p=CUserSettings::Get_SettingsPath(); std::filesystem::create_directories(p.parent_path()); std::ofstream out(p,std::ios::binary|std::ios::trunc); out << text; out.flush(); if(!out) throw std::runtime_error("Write fixture"); }
USER_SETTINGS Candidate(const CUserSettings& settings) { auto v=settings.Get_Settings(); v.Display.width=1600; v.Display.height=900; v.Values["unknown.future\"\nrow"]=12.25f; v.Values[SystemOptionRowId::MASTER_VOLUME]=73.f; return v; }
void Accept(CUserSettings& settings) { settings.Set_DisplayApplyCallback([](const USER_DISPLAY_SETTINGS&,std::string& status){status="test display applied";return true;}); }
void NoTemps() { for(const auto& file:std::filesystem::directory_iterator(CUserSettings::Get_SettingsPath().parent_path())) Check(file.path().extension()!=L".tmp","staged temporary removed"); }
RENDER_QUALITY_SETTINGS Video(const CUserSettings& settings, bool profileEffects=true) {
    RENDER_QUALITY_SETTINGS quality;
    quality.fGamma=1.73f;
    quality.bBloomEnabled=quality.bFXAAEnabled=quality.bSSAOEnabled=profileEffects;
    settings.Apply_Video(quality);
    return quality;
}
bool SameOtherVideo(const RENDER_QUALITY_SETTINGS& left, const RENDER_QUALITY_SETTINGS& right) {
    return left.fGamma==right.fGamma && left.bBloomEnabled==right.bBloomEnabled &&
        left.bFXAAEnabled==right.bFXAAEnabled && left.bSSAOEnabled==right.bSSAOEnabled &&
        left.iColorFilterType==right.iColorFilterType && left.fColorFilterStrength==right.fColorFilterStrength;
}
}
int wmain(int argc,wchar_t** argv) {
 try {
    if(argc!=3) return 2;
    const auto allowed=std::filesystem::weakly_canonical(std::filesystem::path(argv[1])/L"out").wstring()+L"\\";
    const auto requested=std::filesystem::weakly_canonical(std::filesystem::path(argv[2])).wstring();
    if(requested.size()<=allowed.size() || _wcsnicmp(requested.c_str(),allowed.c_str(),allowed.size())!=0)
        throw std::runtime_error("Fixture output must remain inside repository out");
    base=std::filesystem::path(requested)/(std::to_wstring(GetCurrentProcessId())+L"-"+std::to_wstring(GetTickCount64()));
    std::filesystem::create_directories(base);
    std::string status;
    Select(L"roundtrip");
    CUserSettings original; original.Set_Default("unknown.default",4.f);
    Check(original.Load_Persisted(status),"missing file loads defaults");
    Check(!std::filesystem::exists(CUserSettings::Get_SettingsPath()),"load does not create file");
    Accept(original); auto selected=Candidate(original);
    Check(original.Commit(selected,status),"first save succeeds");
    const auto firstBytes=Read();
    Check(!firstBytes.empty(),"first save creates real JSON");
    CUserSettings reload; Check(reload.Load_Persisted(status),"new instance reload succeeds");
    Check(reload.Get_Settings().Has_SameValues(selected),"reload preserves display and all numeric rows");
    reload.Set_Default("unknown.future\"\nrow",999.f);
    Check(reload.Get_Settings().Get("unknown.future\"\nrow",0.f)==12.25f,"default seeding preserves loaded unknown row");
    auto next=selected; next.Display.width=1280; next.Display.height=720;
    Check(original.Commit(next,status),"second save succeeds");
    bool matchingBackup=false;
    for(const auto& file:std::filesystem::directory_iterator(CUserSettings::Get_SettingsPath().parent_path())) if(file.path().extension()==L".bak") {std::ifstream in(file.path(),std::ios::binary); std::string text{std::istreambuf_iterator<char>(in),{}}; matchingBackup |= text==firstBytes;}
    Check(matchingBackup,"backup exactly preserves previous file bytes");
    NoTemps();
    for(const auto mode:{USER_WINDOW_MODE::WINDOWED,USER_WINDOW_MODE::BORDERLESS,USER_WINDOW_MODE::FULLSCREEN}) { next.Display.mode=mode; Check(original.Commit(next,status),"window mode serializes"); CUserSettings other; Check(other.Load_Persisted(status) && other.Get_DisplaySettings()==next.Display,"window mode reload matches"); }

    Select(L"preview"); CUserSettings preview; Check(preview.Load_Persisted(status),"preview initial load"); Accept(preview); auto previewDraft=Candidate(preview);
    int calls=0; preview.Set_DisplayApplyCallback([&](const auto&,auto&){++calls;return true;});
    const auto initial=preview.Get_Settings();
    Check(preview.Preview(previewDraft,status),"live preview accepted");
    Check(calls==0 && preview.Get_DisplaySettings()==initial.Display,"preview never invokes display or changes committed display");
    Check(preview.Get_Settings().Values==previewDraft.Values,"preview exposes non-display values");
    Check(!std::filesystem::exists(CUserSettings::Get_SettingsPath()),"preview never writes disk");
    Check(preview.Preview(initial,status) && preview.Get_Settings().Has_SameValues(initial),"cancel preview restores snapshot without file");

#ifdef _DEBUG
    constexpr uint32_t textureDefault=0u;
#else
    constexpr uint32_t textureDefault=0u;
#endif
    const float uiTextureDefault=EffectiveDefaultForHarness(SystemOptionRowId::TEXTURE_QUALITY,0.f);
    Check(CUserSettings::Get_DefaultTextureQuality()==float(textureDefault),"configuration texture default matches Debug and Release best");
    Check(uiTextureDefault==float(textureDefault),"actual UI default used by initial seed and Reset matches configuration");
    Check(EffectiveDefaultForHarness("unrelated.row",7.f)==7.f &&
        EffectiveDefaultForHarness(SystemOptionRowId::BATTLE_FONT_SIZE,-1.f)==1.f,
        "texture default leaves unrelated authored defaults unchanged");
    Select(L"texture_default_missing_file"); CUserSettings seeded;
    Check(Video(seeded).iTextureMinMip==textureDefault,"pre-UI missing row uses build texture default");
    seeded.Set_Default(SystemOptionRowId::TEXTURE_QUALITY,uiTextureDefault);
    Check(seeded.Load_Persisted(status) && Video(seeded).iTextureMinMip==textureDefault &&
        seeded.Get_Settings().Get(SystemOptionRowId::TEXTURE_QUALITY,-1.f)==uiTextureDefault,
        "new settings and initial UI seed agree with build texture default");
    Check(!std::filesystem::exists(CUserSettings::Get_SettingsPath()),"default initialization does not save personal settings");
    Select(L"texture_default_missing_row");
    const std::string missingTexture=R"({"schema":"lostark.user-settings","formatVersion":1,"display":{"width":1920,"height":1080,"mode":"WINDOWED"},"values":{"unknown.preserved":42}})";
    Write(missingTexture); CUserSettings earlyDefault; earlyDefault.Set_Default(SystemOptionRowId::TEXTURE_QUALITY,uiTextureDefault);
    Check(earlyDefault.Load_Persisted(status) && Video(earlyDefault).iTextureMinMip==textureDefault &&
        earlyDefault.Get_Settings().Get("unknown.preserved",0.f)==42.f,
        "missing persisted texture row merges defaults seeded before load");
    CUserSettings lateDefault;
    Check(lateDefault.Load_Persisted(status) && Video(lateDefault).iTextureMinMip==textureDefault,
        "missing persisted texture row uses fallback before UI seed");
    lateDefault.Set_Default(SystemOptionRowId::TEXTURE_QUALITY,uiTextureDefault);
    Check(lateDefault.Get_Settings().Has_SameValues(earlyDefault.Get_Settings()) && Read()==missingTexture,
        "seeding after load agrees and preserves exact saved bytes");
    Select(L"texture_quality"); CUserSettings texture; Check(texture.Load_Persisted(status),"texture initial load"); Accept(texture);
    Check(Video(texture).iTextureMinMip==textureDefault,"missing texture row uses configuration default");
    auto textureBaseline=texture.Get_Settings();
    textureBaseline.Values[SystemOptionRowId::TEXTURE_QUALITY]=2.f;
    textureBaseline.Values[SystemOptionRowId::BRIGHTNESS]=72.f;
    textureBaseline.Values[SystemOptionRowId::COLOR_FILTER_TYPE]=2.f;
    textureBaseline.Values[SystemOptionRowId::COLOR_FILTER_VALUE]=63.f;
    Check(texture.Commit(textureBaseline,status),"texture baseline save");
    for(uint32_t choice=0;choice!=4;++choice) {
        const auto snapshot=texture.Get_Settings();
        const auto savedBytes=Read();
        const auto before=Video(texture);
        const auto disabledBefore=Video(texture,false);
        auto draft=snapshot; draft.Values[SystemOptionRowId::TEXTURE_QUALITY]=static_cast<float>(choice);
        calls=0; texture.Set_DisplayApplyCallback([&](const auto&,auto&){++calls;return true;});
        Check(texture.Preview(draft,status),"texture choice previews");
        Check(Video(texture).iTextureMinMip==choice,"texture choice reaches effective minimum mip");
        Check(calls==0 && Read()==savedBytes,"texture preview does not apply display or save");
        Check(SameOtherVideo(Video(texture),before),"texture choice preserves effective gamma bloom FXAA SSAO and color filter");
        const auto disabledAfter=Video(texture,false);
        Check(SameOtherVideo(disabledAfter,disabledBefore) && !disabledAfter.bBloomEnabled &&
            !disabledAfter.bFXAAEnabled && !disabledAfter.bSSAOEnabled,"texture choice preserves disabled scene effects");
        Check(texture.Preview(snapshot,status) && Video(texture).iTextureMinMip==before.iTextureMinMip &&
            texture.Get_Settings().Has_SameValues(snapshot) && Read()==savedBytes,"texture cancel restores previous mip and exact snapshot without saving");
        Check(texture.Commit(draft,status),"texture choice commits");
        CUserSettings textureReload; textureReload.Set_Default(SystemOptionRowId::TEXTURE_QUALITY,uiTextureDefault);
        Check(textureReload.Load_Persisted(status),"texture saved choice reloads");
        textureReload.Set_Default(SystemOptionRowId::TEXTURE_QUALITY,uiTextureDefault);
        Check(textureReload.Get_Settings().Has_SameValues(draft) && Video(textureReload).iTextureMinMip==choice,
            "texture reload retains selected mip and unrelated settings");
        auto resetDraft=textureReload.Get_Settings(); const auto resetBefore=resetDraft;
        const auto resetVideoBefore=Video(textureReload); const auto resetBytes=Read();
        resetDraft.Values[SystemOptionRowId::TEXTURE_QUALITY]=EffectiveDefaultForHarness(SystemOptionRowId::TEXTURE_QUALITY,0.f);
        Check(textureReload.Preview(resetDraft,status) && Video(textureReload).iTextureMinMip==textureDefault &&
            SameOtherVideo(Video(textureReload),resetVideoBefore) && Read()==resetBytes,
            "Reset texture preview uses actual UI configuration default without saving or changing other video rows");
        Check(textureReload.Preview(resetBefore,status) && Video(textureReload).iTextureMinMip==choice && Read()==resetBytes,
            "cancel after Reset preserves explicit saved texture choice");
    }
    const auto textureSaved=texture.Get_Settings();
    const auto textureBytes=Read();
    for(const float value:{-1.f,4.f,0.5f}) {
        auto draft=textureSaved; draft.Values[SystemOptionRowId::TEXTURE_QUALITY]=value;
        Check(texture.Preview(draft,status) && Video(texture).iTextureMinMip==0u,"out-of-range or fractional texture choice safely uses mip zero");
        Check(Read()==textureBytes && texture.Preview(textureSaved,status),"invalid texture preview remains unsaved and cancellable");
    }
    for(const float value:{std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::infinity(),-std::numeric_limits<float>::infinity()}) {
        auto draft=textureSaved; draft.Values[SystemOptionRowId::TEXTURE_QUALITY]=value;
        Check(!texture.Preview(draft,status) && texture.Get_Settings().Has_SameValues(textureSaved) && Read()==textureBytes,
            "nonfinite texture preview is rejected without changing memory or file");
        CUserSettings invalidDefault; invalidDefault.Set_Default(SystemOptionRowId::TEXTURE_QUALITY,value);
        Check(invalidDefault.Get_Settings().Values.count(SystemOptionRowId::TEXTURE_QUALITY)==0 && Video(invalidDefault).iTextureMinMip==textureDefault,
            "nonfinite texture default is ignored and missing-row build default remains");
    }
    auto oversized=textureSaved; oversized.Values[SystemOptionRowId::TEXTURE_QUALITY]=std::numeric_limits<float>::max();
    Check(!texture.Preview(oversized,status) && texture.Get_Settings().Has_SameValues(textureSaved) && Read()==textureBytes,
        "oversized texture preview is rejected without changing memory or file");
    CUserSettings oversizedDefault; oversizedDefault.Set_Default(SystemOptionRowId::TEXTURE_QUALITY,std::numeric_limits<float>::max());
    Check(Video(oversizedDefault).iTextureMinMip==0u,"oversized finite texture default safely uses mip zero");

    Select(L"malformed");
    const std::vector<std::string> invalid={"{", "[]", "{}", R"({"schema":"lostark.user-settings","formatVersion":1,"display":{"width":1920.5,"height":1080,"mode":"WINDOWED"},"values":{}})",R"({"schema":"lostark.user-settings","formatVersion":1,"display":{"width":1920,"height":1080,"mode":"BAD"},"values":{}})",R"({"schema":"lostark.user-settings","formatVersion":1,"display":{"width":1920,"height":1080,"mode":"WINDOWED"},"values":{"bad":"text"}})"};
    for(const auto& bytes:invalid) {Write(bytes); CUserSettings broken; broken.Set_Default("preserved",22.f); const auto before=broken.Get_Settings(); Check(!broken.Load_Persisted(status),"malformed/schema-invalid file rejected"); Check(broken.Get_Settings().Has_SameValues(before) && Read()==bytes,"invalid load preserves memory and file"); Accept(broken); Check(!broken.Commit(Candidate(broken),status) && Read()==bytes,"invalid source cannot be overwritten by Apply");}

    Select(L"callback_failure"); CUserSettings failure; Check(failure.Load_Persisted(status),"callback initial load"); Accept(failure); Check(failure.Commit(failure.Get_Settings(),status),"callback baseline save"); auto baselineBytes=Read(); const auto baseline=failure.Get_Settings();
    failure.Set_DisplayApplyCallback([&](const auto&,auto& s){s="injected callback failure";++calls;return false;});
    Check(!failure.Commit(Candidate(failure),status),"display callback failure rejects commit");
    Check(Read()==baselineBytes && failure.Get_Settings().Has_SameValues(baseline),"callback failure preserves exact disk and memory"); NoTemps();
    auto bad=baseline; bad.Values["nan"]=std::numeric_limits<float>::quiet_NaN();
    Check(!failure.Commit(bad,status) && Read()==baselineBytes,"nonfinite staged value rejected before mutation");

    Select(L"external_before"); CUserSettings stale; Check(stale.Load_Persisted(status),"external initial load"); Accept(stale); Check(stale.Commit(stale.Get_Settings(),status),"external baseline save");
    auto external=Read()+"\n "; Write(external); calls=0; stale.Set_DisplayApplyCallback([&](const auto&,auto&){++calls;return true;});
    Check(!stale.Commit(Candidate(stale),status) && calls==0 && Read()==external,"external prior edit rejects before display apply and preserves edit");

    Select(L"external_during"); CUserSettings race; Check(race.Load_Persisted(status),"race initial load"); Accept(race); Check(race.Commit(race.Get_Settings(),status),"race baseline save"); const auto raceOld=race.Get_Settings(); external=Read()+"\n\n"; calls=0; USER_DISPLAY_SETTINGS actual=raceOld.Display;
    race.Set_DisplayApplyCallback([&](const auto& target,auto&){actual=target; ++calls; if(calls==1) Write(external); return true;});
    Check(!race.Commit(Candidate(race),status),"external edit during callback rejects final freshness check");
    Check(calls==2 && actual==raceOld.Display && race.Get_Settings().Has_SameValues(raceOld),"race rollback restores display and retains memory");
    Check(Read()==external,"race leaves external bytes untouched"); NoTemps();

    Select(L"replace_failure"); CUserSettings locked; Check(locked.Load_Persisted(status),"locked initial load"); Accept(locked); Check(locked.Commit(locked.Get_Settings(),status),"locked baseline save"); const auto lockedOld=locked.Get_Settings(); baselineBytes=Read(); calls=0; actual=lockedOld.Display;
    locked.Set_DisplayApplyCallback([&](const auto& target,auto&){actual=target;++calls;return true;});
    HANDLE file=CreateFileW(CUserSettings::Get_SettingsPath().c_str(),GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
    Check(file!=INVALID_HANDLE_VALUE,"fixture holds target without delete sharing");
    const bool committed=locked.Commit(Candidate(locked),status); CloseHandle(file);
    Check(!committed,"real Win32 file lock causes atomic replacement failure");
    Check(calls==2 && actual==lockedOld.Display,"disk failure invokes previous display rollback");
    Check(Read()==baselineBytes && locked.Get_Settings().Has_SameValues(lockedOld),"disk failure preserves exact disk and memory"); NoTemps();
    Check(locked.Commit(Candidate(locked),status),"retry after lock release succeeds");
    std::cout << "SUCCESS assertions=" << assertions << " fixtures=" << base.string() << "\n";
    return 0;
 } catch(const std::exception& ex) {std::cerr<<"FAIL "<<ex.what()<<"\n"; return 1;}
}

```

## G1. RenderingProfiles.json과 publisher 생성물

변경 종류: root 메타데이터의 revision과 stable profile의 fog 블록에서 enabled만 변경.

root 메타데이터의 세 속성은 다음 값이다. 나머지 root 속성은 현재 저장본을 유지한다.

```json
{"schema": "lostark.rendering-profiles", "formatVersion": 1, "revision": 92}
```

`profiles` 안에서 `profileId == "scene.bern.neutral-day.v1"`인 유일한 profile의 `fog` 속성을 다음 완전한 블록으로 반영한다. 색·밀도·방향·region·qualityOverride는 바꾸지 않는다.

```json
{
"fog": {
        "enabled": true,
        "color": [3.55294108, 4.42352962, 6, 1],
        "density": 0.300000012,
        "heightFalloff": 0.800000012,
        "topHeight": 0,
        "startDistance": 16,
        "maximumOpacity": 1,
        "driftSpeed": 0.119999997,
        "driftHeightAmplitude": 1.5,
        "driftDensityAmplitude": 0.0599999987,
        "coveragePercent": 0.449999988,
        "windDirectionX": 0.800000012,
        "windDirectionZ": 0.600000024,
        "windSpeed": 1.5,
        "patchScale": 0.0120000001,
        "patchSoftness": 0.180000007,
        "sourceExponential": {"inscatteringColor": [0.960784316, 0.831372559, 0.160784319, 1], "lightDirection": [0.383024991, 0.642730474, 0.663467705, 0.707106769]}
      }
}
```

실행 데이터는 같은 root 메타데이터와 fog 값을 `Tools/RenderingPipeline/Publish-RenderingProfiles.ps1`가 직렬화한다. 실행 JSON을 수동 편집하지 않는다. publisher의 schema·중복 key·finite 값·stable profile 검사 후 임시 파일과 원자 교체를 사용한다. 게시 파일과 실행 중 메모리 draft는 별도다.

## G1. 정합 문서의 교체 블록

root는 `CLAUDE.md`의 `기본값은 Debug `하`, Release `최상`이다.`를 `기본값은 Debug와 Release 모두 `최상`이다.`로 바꾼다. `gotchas.md`의 `Debug 하/Release 최상은`을 `Debug/Release 모두 최상이`로 바꾼다. 기존 하네스 README의 누락 텍스처 행 설명은 `Debug/Release 모두 mip0(최상)을 사용한다.`로 교정한다.

렌더링 복원 V2의 텍스처 품질 절에는 다음 계약을 기록한다.

> 텍스처 기본 인덱스는 Debug/Release 모두 0(최상)이며 명시적으로 저장한 선택값이 우선한다. 렌더링 옵션 값 복원은 최적화·mip 선택 기능의 코드 회귀와 구분한다.

## G1. 프로젝트 등록과 반영·검증

`Client/Default/Client.vcxproj:724`와 `.filters:1401`에 기존 CPP가 등록돼 있고 정본 JSON도 같은 프로젝트의 `None` 항목과 filter에 있다. 새 제품 파일·선언이 없어 project/filter 변경은 필요 없다. fixture는 기존 standalone runner가 직접 컴파일한다.

1. 현재 파일의 SHA-256와 원본 bytes를 `out/RenderOptionsOct02Restore20261007`에 보관하고 최종 CPP/JSON 후보를 만든다. 후보 JSON을 먼저 publisher Validate로 검증한다.
2. 교체 직전에 해당 파일 hash를 다시 비교한다. 다르면 최신 파일에 stable profile ID와 단일 기본값 변경을 다시 병합해야 하며 덮어쓰지 않는다. 일치하면 원본 백업을 유지하고 임시 파일에서 원자 교체한다.
3. 저작 정본과 기존 실행 파일 hash를 재확인하고 publisher Publish를 실행한다. 게시 실패 시 자신이 쓴 후보 hash가 유지되는 파일만 원본으로 rollback한다. 다른 저장이 확인된 파일은 보존한다.
4. JSON 의미 비교에서 원본 대비 revision과 neutral-day fog enabled 두 경로만 달라야 한다. 기준 commit과는 revision만 달라야 하며 authored/runtime 의미가 같아야 한다. 개인 설정 hash, CRLF/BOM 여부, 계획서 전문과 실제 CPP 동일성, scoped `git diff --check`를 확인한다.
5. 아래 standalone 검사는 root가 GPU 수정·빌드와의 충돌을 조정한 뒤 Debug, Release 순서로 실행한다. 이 구현 담당은 빌드나 Client/UI를 실행하지 않는다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/RenderingPipeline/Publish-RenderingProfiles.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/RenderingPipeline/Publish-RenderingProfiles.ps1 -Mode Publish
powershell -ExecutionPolicy Bypass -File Tools/UserSettingsContractHarness/Run-UserSettingsContractHarness.ps1 -Configuration Debug -OutputDirectory out/RenderOptionsOct02Restore20261007/harness-debug
powershell -ExecutionPolicy Bypass -File Tools/UserSettingsContractHarness/Run-UserSettingsContractHarness.ps1 -Configuration Release -OutputDirectory out/RenderOptionsOct02Restore20261007/harness-release
```

VS x64 C++ 도구와 Windows SDK가 standalone 검사의 의존성이다. 정본 제품 빌드가 필요하면 root가 `powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`를 조정해 실행한다. 이 명령은 계획이며 실행 성공 증거가 아니다.

사용자의 화면 확인은 기존 Lobby에서 Bern에 진입한 뒤 fog와 옵션 표시를 확인하는 절차다. F1 Workbench에서 저장·Reload를 자동 수행하지 않는다. 새 기본값은 C++ 빌드 후 누락 행·새 UI 초기값·Reset에 적용되며 기존 명시 저장은 보존된다. 이번 JSON 검증과 소스 diff만으로 화면·FPS·GPU 효과를 완료 판정하지 않는다. 독립 리뷰자는 변경범위와 PLAN 전문·실제 소스 일치를 확인한다.


## G2. 데스크탑 동기화의 최종 fog OFF 요청

2026-10-07 사용자가 fog OFF를 최종 정본으로 명시했다. 위 G1의 과거값 복원 중 Bern fog ON은 이 요청으로 대체한다. `scene.bern.neutral-day.v1`의 `fog`만 아래 완전한 블록으로 교체하고 root revision을 92에서 93으로 올린다. 다른 profile과 품질, 최적화는 유지한다. 최신 hash를 재확인하고 백업 후 원자 교체하며 공식 publisher의 Validate/Publish로 실행 데이터를 생성한다.

```json
{
  "enabled": false,
  "color": [
    3.55294108,
    4.42352962,
    6,
    1
  ],
  "density": 0.300000012,
  "heightFalloff": 0.800000012,
  "topHeight": 0,
  "startDistance": 16,
  "maximumOpacity": 1,
  "driftSpeed": 0.119999997,
  "driftHeightAmplitude": 1.5,
  "driftDensityAmplitude": 0.0599999987,
  "coveragePercent": 0.449999988,
  "windDirectionX": 0.800000012,
  "windDirectionZ": 0.600000024,
  "windSpeed": 1.5,
  "patchScale": 0.0120000001,
  "patchSoftness": 0.180000007,
  "sourceExponential": {
    "inscatteringColor": [
      0.960784316,
      0.831372559,
      0.160784319,
      1
    ],
    "lightDirection": [
      0.383024991,
      0.642730474,
      0.663467705,
      0.707106769
    ]
  }
}
```

GPU 선택과 기존 작업의 현재 변경을 사용자 요청에 따라 같은 PR로 전달한다. gotchas에는 RTX 4050 장착 노트북의 실제 adapter 검증을 명시한다. 이번 단계의 무비 시작 지연은 기존 로그만 조사하며, 책임 구간이 확정되지 않으면 런타임을 변경하지 않는다.
