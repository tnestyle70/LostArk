#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Engine_RenderTypes.h"

#include <map>
#include <string>

NS_BEGIN(Client)

/* Everything the system option window owns, keyed by the retail row id
(EFTable_SystemOption.CompName -- `slider_mainsound`, `checkbox_bgm`, `combobox_mouse_cursor`)
so a value round-trips to the row that edits it without a second naming scheme. Sliders and
spinners store their number, check boxes 0/1, combo boxes and radio lists the choice index.
Rows the engine cannot act on are stored all the same: the window shows them, the file keeps
them, nothing reads them yet.

Audio maps onto the Engine mixer buses; video is folded onto whatever quality the active scene
profile resolved to, so a Level change cannot silently drop it. */
struct USER_SETTINGS final
{
	map<string, f32_t> Values;

	f32_t Get(const string& strRowId, f32_t fFallback) const;
	bool_t Get_Flag(const string& strRowId, bool_t bFallback) const;
	bool_t Has_SameValues(const USER_SETTINGS& Other) const;
};

/* Who a replicated player is to the local one, for the nametag / chat balloon rows that
retail splits by relation (own / party / other). */
enum class PLAYER_RELATION { LOCAL, PARTY, OTHER };

/* Process-wide user preferences for this run. Not persisted: the window edits them, the
engine consumers read them, and the next launch starts over from the retail defaults. */
class CUserSettings final
{
public:
	static CUserSettings& Get();
	~CUserSettings();

public:
	const USER_SETTINGS& Get_Settings() const { return m_Settings; }

	/* Retail defaults for the rows the engine acts on, so a missing file and a missing key
	both mean "what the retail table says". Row ids and defaults come from
	Data/UI/SystemOption/SystemOptionRows.json through CSystemOptionRowsDocument; the window
	seeds them on load, the fallbacks below only cover a run without that document. */
	void Set_Default(const string& strRowId, f32_t fDefault);

	/* Pushes the defaults into the mixer and the cursor once the Engine exists. Nothing is
	read from disk: every run starts from the retail table's defaults, on purpose -- keeping a
	user's settings between runs is a later step. */
	void Initialize();
	/* Commits the staged struct and pushes the audio buses. */
	bool_t Commit(const USER_SETTINGS& Staged, string& strOutStatus);

	/* Pushes the bus volumes into the live Engine mixer. */
	void Apply_Audio() const;
	/* Renderer hook: folds the video settings onto an already resolved effective quality.
	CRenderingProfileService::Resolve_EffectiveQuality calls this, which is the one funnel
	every Level / region commit goes through, so the preference survives scene changes. */
	void Apply_Video(RENDER_QUALITY_SETTINGS& Quality) const;
	/* Retail cursor / size / outline -> the .ani/.cur the window class cursor becomes. */
	void Apply_Cursor() const;
	/* Per frame from CMainApp: with the lock row on, the OS cursor is confined to the client
	area while this window is in front, and released the moment it is not. */
	void Update_CursorLock(bool_t bWindowFocused);
	/* Pointer speed / acceleration rows -> the Windows mouse settings for this session only
	(not written to the registry); the values found at first change come back on exit. */
	void Apply_PointerSpeed();

	/* Read-side helpers for the consumers the rows drive. */
	f32_t Get_DamageFontScale() const;            // combobox_battle_font_size: 0.75 .. 3.0
	bool_t Is_DamageNumberShown() const;          // checkbox_damage
	bool_t Is_ConditionMessageShown() const;      // checkbox_condition_message
	bool_t Is_SkillCameraShakeOn() const;         // checkbox_skill_camera_shake
	bool_t Is_MouseButtonSwapped() const;         // checkbox_action_mousebuttonswap
	/* combobox_fps: 0 always, 1 in combat only, 2 never. */
	int32_t Get_FpsDisplayMode() const;
	/* 0 when the matching "use" checkbox is off, else the spinner's frames per second. */
	int32_t Get_FrameLimit(bool_t bForeground) const;
	bool_t Is_NametagShown(PLAYER_RELATION eRelation, bool_t bHonorTitle) const;
	bool_t Is_ChatBubbleShown(PLAYER_RELATION eRelation) const;

private:
	USER_SETTINGS m_Settings{};
	map<string, f32_t> m_Defaults;
	bool_t m_bCursorClipped = false;
	/* Windows mouse settings as found before the first pointer-speed change; restored on exit. */
	bool_t m_bPointerBaselineTaken = false;
	bool_t m_bPointerChanged = false;
	int32_t m_iBaselineMouseSpeed = 10;
	int32_t m_BaselineMouseAcceleration[3] = { 6, 10, 1 };
};

/* Row ids the engine acts on (EFTable_SystemOption.CompName). Anything else is display-only. */
namespace SystemOptionRowId
{
	constexpr const char* MASTER_VOLUME = "slider_mainsound";
	constexpr const char* MASTER_ON = "checkbox_mainsound";
	constexpr const char* UI_VOLUME = "slider_uisound";
	constexpr const char* UI_ON = "checkbox_uisound";
	constexpr const char* MUSIC_VOLUME = "slider_bgm";
	constexpr const char* MUSIC_ON = "checkbox_bgm";
	constexpr const char* EFFECT_VOLUME = "slider_effectsound";
	constexpr const char* EFFECT_ON = "checkbox_effectsound";
	constexpr const char* BACKGROUND_SOUND = "checkbox_backgroundsound";
	constexpr const char* BRIGHTNESS = "slider_monitor_gamma";
	constexpr const char* BLOOM = "checkbox_filter_effect_bloom";
	constexpr const char* ANTIALIASING = "checkbox_antialiasing";     // combo: high/low/off
	constexpr const char* SSAO = "combobox_antialiasing"; // combo: high/low/off
	constexpr const char* CURSOR_PRESET = "combobox_mouse_cursor";
	constexpr const char* CURSOR_PRESET_SIZE = "combobox_mouse_cursor_size";
	constexpr const char* CURSOR_PRESET_OUTLINE = "CursorOutline";
	constexpr const char* MOUSE_LOCK = "checkbox_mouselock";
	constexpr const char* POINTER_SPEED = "slider_pointer_speed";
	constexpr const char* POINTER_ACCELERATE = "slider_pointer_accelerate";
	constexpr const char* COLOR_FILTER_TYPE = "combobox_color_filter_type";   // off / protan / deutan / tritan
	constexpr const char* COLOR_FILTER_VALUE = "slider_color_filter_value";   // 0..100
	constexpr const char* GRAPHICS_PRESET = "combobox_totalgrapicoption";    // 0..3 presets, 4 custom
	constexpr const char* FPS_LIMIT_FOREGROUND = "slider_fps_limet_foreground";
	constexpr const char* FPS_LIMIT_FOREGROUND_ON = "checkbox_fps_limet_foreground_use";
	constexpr const char* FPS_LIMIT_BACKGROUND = "slider_fps_limet_background";
	constexpr const char* FPS_LIMIT_BACKGROUND_ON = "checkbox_fps_limet_background_use";
	constexpr const char* FPS_DISPLAY = "combobox_fps";
	constexpr const char* BATTLE_FONT_SIZE = "combobox_battle_font_size";   // 75/100/150/200/300 %
	constexpr const char* SHOW_DAMAGE = "checkbox_damage";
	constexpr const char* SHOW_CONDITION_MESSAGE = "checkbox_condition_message";
	constexpr const char* SKILL_CAMERA_SHAKE = "checkbox_skill_camera_shake";
	constexpr const char* MOUSE_BUTTON_SWAP = "checkbox_action_mousebuttonswap";
	constexpr const char* PLAYER_NAMETAG_NAME = "checkbox_player_nametag_name";
	constexpr const char* PLAYER_NAMETAG_TITLE = "checkbox_player_nametag_honortitle";
	constexpr const char* PLAYER_BALLOON = "checkbox_messageballoon_local";
	constexpr const char* PARTY_NAMETAG_NAME = "checkbox_party_nametag_name";
	constexpr const char* PARTY_NAMETAG_TITLE = "checkbox_party_nametag_honortitle";
	constexpr const char* PARTY_BALLOON = "checkbox_messageballoon_party";
	constexpr const char* OTHER_NAMETAG_NAME = "checkbox_other_nametag_name";
	constexpr const char* OTHER_NAMETAG_TITLE = "checkbox_other_nametag_honortitle";
	constexpr const char* OTHER_BALLOON = "checkbox_messageballoon_other";
}

NS_END
