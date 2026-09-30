#include "ColosseumMatchView.h"
#include "Camera_Free.h"
#include "Character.h"
#include "ClientReplication.h"
#include "DataJson.h"
#include "GameInstance.h"
#include "ProjectDataRoot.h"
#include "UIInputRouter.h"
#include "UILabelFont.h"
#include "UILayoutRuntime.h"
#include "UITextOcclusion.h"
#include "WorldPlayerNameplateView.h"
#include "Network/PacketMessages.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>

using namespace Client;
using namespace LostArk::Shared;

namespace
{
	constexpr uint64_t VICTORY_CAMERA_OWNER = 0x434F4C4F57494E31ull;
	constexpr double TICKS_PER_SECOND = 30.0;
	const DATA_JSON_VALUE& Field(const DATA_JSON_VALUE& row, const char* key)
	{
		const auto* value = row.Find(key);
		if (!value) throw std::runtime_error(std::string("missing field: ") + key);
		return *value;
	}
	float Number(const DATA_JSON_VALUE& row, const char* key, double minimum, double maximum)
	{
		const auto& value = Field(row, key);
		if (!value.Is_Number() || !std::isfinite(value.Get_Number()) ||
			value.Get_Number() < minimum || value.Get_Number() > maximum)
			throw std::runtime_error(std::string("invalid number: ") + key);
		return static_cast<float>(value.Get_Number());
	}
	const DATA_JSON_VALUE::ARRAY& Array(const DATA_JSON_VALUE& row, const char* key)
	{
		const auto& value = Field(row, key);
		if (!value.Is_Array()) throw std::runtime_error(std::string("invalid array: ") + key);
		return value.Get_Array();
	}
	float3_t Vector(const DATA_JSON_VALUE& row, const char* key)
	{
		const auto& values = Array(row, key);
		if (values.size() != 3u) throw std::runtime_error("vector must have three coordinates");
		float3_t result;
		float* out[] = { &result.x, &result.y, &result.z };
		for (size_t i = 0; i < 3u; ++i)
		{
			if (!values[i].Is_Number() || !std::isfinite(values[i].Get_Number()) ||
				std::abs(values[i].Get_Number()) > 100000.0)
				throw std::runtime_error("invalid vector coordinate");
			*out[i] = static_cast<float>(values[i].Get_Number());
		}
		return result;
	}
	float3_t Lerp(const float3_t& a, const float3_t& b, float t)
	{
		return { a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t };
	}
	void Text(const std::wstring& text, float centerX, float topY, float pixels, const float4_t& color, float maxWidth = 0.f)
	{
		if (text.empty()) return;
		auto& game = CGameInstance::Get();
		float scale = 1.f;
		const auto font = UILabelFont::Resolve(TEXT("Font_YG760"), pixels, scale);
		const auto size = game.Measure_Text(font, text.c_str());
		if (maxWidth > 0.f && size.x * scale > maxWidth) scale = maxWidth / size.x;
		const float2_t position(std::round(centerX - size.x * scale * .5f), std::round(topY));
		game.Draw_Text(font, text.c_str(), float2_t(position.x + 1.f, position.y + 1.f),
			XMVectorSet(0.f, 0.f, 0.f, color.w), 0.f, float2_t(), scale);
		game.Draw_Text(font, text.c_str(), position, XMLoadFloat4(&color), 0.f, float2_t(), scale);
	}
}

struct CColosseumMatchView::IMPLEMENTATION
{
	struct RECT { std::string id; float x, y, width, height; };
	struct KEY { float time = 0.f; float3_t eye{}, forward{}; };
	struct SHOT { float start = 0.f, end = 0.f; std::vector<KEY> keys; };
	struct TITLE_KEY
	{
		float time = 0.f, x = 0.f, y = 0.f, width = 0.f, height = 0.f, alpha = 0.f;
	};
	struct ACTOR_SLOT
	{
		float3_t position{};
		float yaw = 0.f, clipStart = 0.f;
		std::string clip;
		bool loop = false;
	};
	struct DOCUMENT
	{
		float bannerDuration = 3475.f, duration = 0.f, returnFade = 0.f;
		float fovX = 0.f, aspect = 2.35f;
		std::vector<std::pair<float, float>> fade;
		std::vector<SHOT> shots;
		std::vector<ACTOR_SLOT> actors;
		std::array<std::vector<TITLE_KEY>, 3> titles;
		std::array<float, 3> bannerDurations{};
	};
	struct ACTOR
	{
		std::weak_ptr<CCharacter> character;
		NET_ENTITY_ID id = INVALID_NET_ENTITY_ID;
		std::wstring name;
		bool winner = false, previouslySuppressed = false, clipFailed = false;
		size_t slot = 0;
	};

	struct COMBAT_TEXT { std::string slot; std::wstring text; float pixels; float4_t color; };
	std::vector<COMBAT_TEXT> combatText;
	std::unique_ptr<CUILayoutRuntime> hud, result, bars, combat;
	std::weak_ptr<CCamera_Free> camera;
	std::vector<RECT> hudRects, resultRects, combatRects;
	bool combatReady = false;
	std::vector<ACTOR> actors;
	DOCUMENT document;
	S2C_COLOSSEUM_MATCH_STATE state;
	std::string status;
	bool documentReady = false, uiReady = false, cinematic = false, returnIntent = false;
	bool showHud = false, showReturn = false, showingActors = false;
	float resultMs = 0.f, sceneMs = 0.f;
	float returnRequestCooldown = 0.f;
	double serverTick = 0.0;
	uint64_t matchId = 0;
	uint8_t localTeam = COLOSSEUM_DRAW_TEAM;
	std::string bannerId;
#ifdef _DEBUG
	DEBUG_PREVIEW debugPreview = DEBUG_PREVIEW::NONE;
	float debugClockMs = 0.f;
	bool debugPaused = false;
#endif

	void Diagnose(const std::string& message)
	{
		status = message;
		OutputDebugStringA(("[Colosseum.MatchView] " + message + "\n").c_str());
	}
	bool Load_Document()
	{
		try
		{
			const auto path = CProjectDataRoot::Resolve(L"Camera/ColosseumVictory.cutscene.json");
			if (std::filesystem::file_size(path) > 8u * 1024u * 1024u)
				throw std::runtime_error("victory document exceeds 8 MiB");
			std::ifstream input(path, std::ios::binary);
			std::ostringstream bytes;
			bytes << input.rdbuf();
			DATA_JSON_VALUE root;
			std::string error;
			if (!input || !CDataJson::Parse(bytes.str(), root, error))
				throw std::runtime_error("victory parse failed: " + error);
			const auto& schema = Field(root, "schema");
			if (!schema.Is_String() || schema.Get_String() != "lostark.colosseum-victory-cutscene" ||
				Number(root, "formatVersion", 1, 1) != 1.f)
				throw std::runtime_error("unsupported victory schema");
			DOCUMENT staged;
			staged.bannerDuration = Number(root, "bannerDurationMs", 100, 10000);
			staged.duration = Number(root, "durationMs", 100, 60000);
			staged.returnFade = Number(root, "returnFadeMs", 0, 5000);
			staged.fovX = Number(root, "fovXDegrees", 20, 150);
			staged.aspect = Number(root, "letterboxAspect", 1, 4);
			const auto& titleTracks = Field(root, "bannerTitles");
			const char* titleNames[] = { "Victory", "Defeat", "Draw" };
			for (size_t index = 0; index < staged.titles.size(); ++index)
			{
				const auto& track = Field(titleTracks, titleNames[index]);
				const float frameRate = Number(track, "frameRate", 1, 120);
				for (const auto& row : Array(track, "keys"))
				{
					TITLE_KEY key;
					key.time = Number(row, "timeMs", 0, 10000);
					key.x = Number(row, "x", -1280, 2560);
					key.y = Number(row, "y", -720, 1440);
					key.width = Number(row, "width", 1, 1280);
					key.height = Number(row, "height", 1, 720);
					key.alpha = Number(row, "alpha", 0, 1);
					auto& keys = staged.titles[index];
					if ((keys.empty() && key.time != 0.f) ||
						(!keys.empty() && key.time <= keys.back().time))
						throw std::runtime_error("unordered result title track");
					keys.push_back(key);
				}
				if (staged.titles[index].empty()) throw std::runtime_error("empty result title track");
				staged.bannerDurations[index] = staged.titles[index].back().time + 1000.f / frameRate;
				// Keep the final Defeat/Draw frame and one common ceremony start for all peers.
				staged.bannerDuration = (std::max)(staged.bannerDuration, staged.bannerDurations[index]);
			}
			for (const auto& row : Array(root, "fade"))
			{
				const float time = Number(row, "timeMs", 0, staged.duration);
				if (!staged.fade.empty() && time <= staged.fade.back().first)
					throw std::runtime_error("unordered victory fade");
				staged.fade.emplace_back(time, Number(row, "value", 0, 1));
			}
			for (const auto& row : Array(root, "shots"))
			{
				SHOT shot;
				shot.start = Number(row, "startMs", 0, staged.duration);
				shot.end = Number(row, "endMs", 0, staged.duration);
				const auto forward = Vector(row, "forward");
				if (shot.end <= shot.start || (staged.shots.empty() ? shot.start != 0.f :
					std::abs(shot.start - staged.shots.back().end) > .1f))
					throw std::runtime_error("victory shots have a gap/overlap");
				for (const auto& keyRow : Array(row, "keys"))
				{
					KEY key;
					key.time = Number(keyRow, "timeMs", shot.start, shot.end);
					key.eye = Vector(keyRow, "eye");
					key.forward = keyRow.Find("forward") ? Vector(keyRow, "forward") : forward;
					const float length2 = key.forward.x * key.forward.x + key.forward.y * key.forward.y + key.forward.z * key.forward.z;
					if (length2 < .001f || (!shot.keys.empty() && key.time <= shot.keys.back().time))
						throw std::runtime_error("invalid victory camera key");
					shot.keys.push_back(key);
				}
				if (shot.keys.empty() || shot.keys.front().time != shot.start)
					throw std::runtime_error("victory shot has no initial key");
				staged.shots.push_back(std::move(shot));
			}
			for (const auto& row : Array(root, "actors"))
			{
				ACTOR_SLOT actor;
				actor.position = Vector(row, "position");
				actor.yaw = Number(row, "yawDegrees", -720, 720);
				actor.clipStart = Number(row, "clipStartMs", 0, staged.duration);
				const auto& clip = Field(row, "clipName");
				const auto& loop = Field(row, "loop");
				if (!clip.Is_String() || clip.Get_String().empty() || !loop.Is_Boolean())
					throw std::runtime_error("invalid exact victory animation binding");
				actor.clip = clip.Get_String();
				actor.loop = loop.Get_Boolean();
				staged.actors.push_back(std::move(actor));
			}
			if (staged.fade.empty() || staged.shots.empty() || staged.actors.size() != 4u ||
				std::abs(staged.shots.back().end - staged.duration) > .1f)
				throw std::runtime_error("incomplete victory document");
			document = std::move(staged);
			return true;
		}
		catch (const std::exception& e) { Diagnose(e.what()); return false; }
	}
	static std::vector<RECT> Capture_Rects(const CUILayoutRuntime& view)
	{
		std::vector<RECT> rects;
		for (const auto& id : view.Get_SlotIds())
		{
			RECT rect; rect.id = id;
			if (view.Get_SlotRect(id, rect.x, rect.y, rect.width, rect.height)) rects.push_back(rect);
		}
		return rects;
	}
	static void Anchor(CUILayoutRuntime& view, const std::vector<RECT>& rects)
	{
		const auto viewport = CGameInstance::Get().Get_ViewportSize();
		if (viewport.x <= 0.f || viewport.y <= 0.f) return;
		const float xScale = (viewport.y / 720.f) * (1280.f / viewport.x);
		for (const auto& rect : rects)
			view.Set_SlotRect(rect.id, 640.f + (rect.x - 640.f) * xScale,
				rect.y, rect.width * xScale, rect.height);
	}
	void Update_Combat(const CClientReplication& replication)
	{
		combatText.clear();
		if (!combat) return;
		combat->Set_AllSlotsVisible(false);
		if (!showHud || !combatReady) return;
		const auto viewport = CGameInstance::Get().Get_ViewportSize();
		if (viewport.x <= 0.f || viewport.y <= 0.f) return;
		const float xScale = (viewport.y / 720.f) * (1280.f / viewport.x);
		for (const auto& rect : combatRects)
		{
			const bool left = rect.id.rfind("Left", 0) == 0;
			combat->Set_SlotRect(rect.id, left ? rect.x * xScale : 1280.f - (1280.f - rect.x) * xScale,
				rect.y, rect.width * xScale, rect.height);
		}
		std::vector<REPLICATED_PLAYER_VIEW> players;
		replication.Collect_PlayerViews(players);
		const float4_t white(1.f, 1.f, 1.f, 1.f), gold(1.f, .85f, .3f, 1.f);
		for (const auto& row : state.Participants)
		{
			if (!row.bParticipant || row.iTeam > 1u || row.iArrivalIndex >= MAX_COLOSSEUM_COMBAT_PLAYERS) continue;
			const std::string prefix = std::string(row.iTeam == 0u ? "Left" : "Right") + std::to_string(row.iArrivalIndex / 2u) + "_";
			const auto player = std::find_if(players.begin(), players.end(), [&](const auto& item) {
				return item.iPlayerId == row.iPlayerId && item.iNetEntityId == row.iNetEntityId;
			});
			const auto health = player != players.end() ? replication.Get_PlayerHealth().Find(row.iNetEntityId) : REPLICATED_PLAYER_HEALTH{};
			std::wstring name = L"...";
			if (player != players.end()) (void)CWorldPlayerNameplateView::Try_ConvertUtf8(player->strNickname, name);
			for (const char* part : { "BG", "HP", "Frame" }) combat->Set_SlotVisible(prefix + part, true);
			combat->Set_SlotFillRatio(prefix + "HP", std::clamp(health.Get_Ratio(), 0.f, 1.f));
			const bool local = player != players.end() && player->isLocal;
			combatText.push_back({prefix + "Name", name, 11.f, local ? gold : white});
			combatText.push_back({prefix + "Number", std::to_wstring(row.iArrivalIndex / 2u + 1u), 15.f, white});
			combatText.push_back({prefix + "Kills", std::to_wstring(row.iKills) + L"\xD0AC", 12.f, white});
			const std::wstring hp = !health.hasSnapshot ? L"..." : health.iCurrentHp == 0u ? L"\xC0AC\xB9DD" :
				std::to_wstring(health.iCurrentHp) + L" / " + std::to_wstring(health.iMaximumHp);
			combatText.push_back({prefix + "Health", hp, 10.f, white});
		}
		size_t slot = 0u;
		for (auto it = state.RecentKills.rbegin(); it != state.RecentKills.rend() && slot < 3u; ++it)
		{
			const double age = serverTick - static_cast<double>(it->iServerTick);
			if (age < 0.0 || age >= 6.0 * TICKS_PER_SECOND) continue;
			const std::string prefix = "Feed" + std::to_string(slot++) + "_";
			const bool red = it->iKillerTeam == 0u;
			combat->Set_SlotVisible(prefix + "BG", true);
			combat->Set_SlotVisible(prefix + (red ? "Red" : "Blue"), true);
			combat->Set_SlotVisible(prefix + (red ? "RedIcon" : "BlueIcon"), true);
			std::wstring killer, victim;
			(void)CWorldPlayerNameplateView::Try_ConvertUtf8(it->strKillerNickname, killer);
			(void)CWorldPlayerNameplateView::Try_ConvertUtf8(it->strVictimNickname, victim);
			combatText.push_back({prefix + "Killer", killer, 12.f, white});
			combatText.push_back({prefix + "Victim", victim, 12.f, white});
		}
	}
	void Clear_Actors()
	{
		for (auto& actor : actors)
			if (const auto character = actor.character.lock())
			{
				character->Clear_CutscenePoseOverride();
				character->Clear_CutsceneAnimation();
				character->Set_CinematicPresentationSuppressed(actor.previouslySuppressed);
			}
		actors.clear();
		showingActors = false;
	}
	void End_Camera()
	{
		Clear_Actors();
		if (const auto activeCamera = camera.lock())
			(void)activeCamera->End_PresentationOverride(VICTORY_CAMERA_OWNER);
		cinematic = false;
		if (bars) bars->Set_AllSlotsVisible(false);
	}
	void Build_Actors(const CClientReplication& replication)
	{
		std::vector<REPLICATED_PLAYER_VIEW> players;
		replication.Collect_PlayerViews(players);
		auto participants = state.Participants;
		std::erase_if(participants, [](const auto& row) {
			return !row.bParticipant || row.iTeam > 1u || row.iArrivalIndex >= MAX_COLOSSEUM_COMBAT_PLAYERS;
		});
		std::sort(participants.begin(), participants.end(), [](const auto& a, const auto& b) {
			return a.iArrivalIndex < b.iArrivalIndex;
		});
		size_t present = 0;
		bool changed = !showingActors;
		for (const auto& participant : participants)
		{
			const auto player = std::find_if(players.begin(), players.end(), [&](const auto& item) {
				return item.iNetEntityId == participant.iNetEntityId && item.iPlayerId == participant.iPlayerId;
			});
			if (player == players.end() || player->pCharacter.expired()) continue;
			++present;
			const auto found = std::find_if(actors.begin(), actors.end(), [&](const auto& actor) {
				return actor.id == participant.iNetEntityId && actor.character.lock() == player->pCharacter.lock();
			});
			changed = changed || found == actors.end();
		}
		if (!changed && present == actors.size()) return;
		Clear_Actors();
		for (const auto& participant : participants)
		{
			const auto it = std::find_if(players.begin(), players.end(), [&](const auto& player) {
				return player.iNetEntityId == participant.iNetEntityId && player.iPlayerId == participant.iPlayerId;
			});
			if (it == players.end()) continue;
			const auto character = it->pCharacter.lock();
			if (!character) continue;
			ACTOR actor;
			actor.character = character;
			actor.id = participant.iNetEntityId;
			actor.winner = participant.iTeam == state.iWinningTeam;
			actor.previouslySuppressed = character->Is_CinematicPresentationSuppressed();
			if (actor.winner) actor.slot = participant.iArrivalIndex / 2u;
			(void)CWorldPlayerNameplateView::Try_ConvertUtf8(it->strNickname, actor.name);
			actors.push_back(std::move(actor));
		}
		showingActors = true;
	}
	float Fade(float time) const
	{
		if (time <= document.fade.front().first) return document.fade.front().second;
		for (size_t i = 1; i < document.fade.size(); ++i)
			if (time <= document.fade[i].first)
			{
				const auto& a = document.fade[i - 1]; const auto& b = document.fade[i];
				return a.second + (b.second - a.second) * ((time - a.first) / (b.first - a.first));
			}
		return document.fade.back().second;
	}
	void Apply_Camera(float time)
	{
		const auto activeCamera = camera.lock();
		if (!activeCamera) { End_Camera(); return; }
		const SHOT* shot = &document.shots.front();
		for (const auto& candidate : document.shots) if (candidate.start <= time) shot = &candidate;
		KEY key = shot->keys.front();
		for (size_t i = 1; i < shot->keys.size(); ++i)
		{
			const auto& b = shot->keys[i];
			if (time > b.time) { key = b; continue; }
			const auto& a = shot->keys[i - 1];
			const float ratio = std::clamp((time - a.time) / (b.time - a.time), 0.f, 1.f);
			key.eye = Lerp(a.eye, b.eye, ratio);
			key.forward = Lerp(a.forward, b.forward, ratio);
			break;
		}
		const auto look = float3_t(key.eye.x + key.forward.x * 10.f,
			key.eye.y + key.forward.y * 10.f, key.eye.z + key.forward.z * 10.f);
		const float aspect = (std::max)(.1f, activeCamera->Get_AspectRatio());
		const float fovY = XMConvertToDegrees(2.f * std::atan(std::tan(XMConvertToRadians(document.fovX) * .5f) / aspect));
		cinematic = activeCamera->Begin_PresentationOverride(VICTORY_CAMERA_OWNER, Engine::CCamera::PRESENTATION_PRIORITY::SERVER_CINEMATIC);
		if (cinematic) (void)activeCamera->Apply_PresentationPose(VICTORY_CAMERA_OWNER, key.eye, look, fovY);
	}
	void Overlay(bool letterbox, float fade)
	{
		if (!bars) return;
		const auto viewport = CGameInstance::Get().Get_ViewportSize();
		if (viewport.x <= 0.f || viewport.y <= 0.f) return;
		const float height = 360.f * (1.f - (std::min)(1.f, (viewport.x / viewport.y) / document.aspect));
		bars->Set_SlotVisible("Intro_BarTop", letterbox && height > 0.f);
		bars->Set_SlotVisible("Intro_BarBottom", letterbox && height > 0.f);
		bars->Set_SlotRect("Intro_BarTop", 0.f, 0.f, 1280.f, height);
		bars->Set_SlotRect("Intro_BarBottom", 0.f, 720.f - height, 1280.f, height);
		bars->Set_SlotVisible("Intro_Fade", fade > .001f);
		bars->Set_SlotAlpha("Intro_Fade", std::clamp(fade, 0.f, 1.f));
	}
	void Slot_Text(CUILayoutRuntime& view, const char* id, const std::wstring& text,
		float pixels, const float4_t& color) const
	{
		float x, y, width, height;
		const auto viewport = CGameInstance::Get().Get_ViewportSize();
		if (viewport.x <= 0.f || viewport.y <= 0.f || !view.Get_SlotRect(id, x, y, width, height)) return;
		Text(text, (x + width * .5f) * viewport.x / 1280.f,
			y * viewport.y / 720.f, pixels * viewport.y / 720.f, color, width * viewport.x / 1280.f);
	}
	float Banner_Duration() const
	{
		if (bannerId == "Result_Victory") return document.bannerDurations[0];
		if (bannerId == "Result_Defeat") return document.bannerDurations[1];
		if (bannerId == "Result_Draw") return document.bannerDurations[2];
		return 0.f;
	}
	void Render_BannerTitle(const std::wstring& title)
	{
		if (!documentReady) return;
		const size_t index = bannerId == "Result_Victory" ? 0u : (bannerId == "Result_Defeat" ? 1u : 2u);
		const auto& keys = document.titles[index];
		const auto after = std::upper_bound(keys.begin(), keys.end(), resultMs,
			[](float time, const TITLE_KEY& key) { return time < key.time; });
		const auto& key = after == keys.begin() ? keys.front() : *(after - 1);
		if (key.alpha <= 0.f) return;
		const auto viewport = CGameInstance::Get().Get_ViewportSize();
		if (viewport.x <= 0.f || viewport.y <= 0.f) return;
		const float4_t color = index == 1u ? float4_t(1.f, .5f, .4f, key.alpha) :
			float4_t(.45f, .85f, 1.f, key.alpha);
		Text(title, (key.x + key.width * .5f) * viewport.x / 1280.f,
			key.y * viewport.y / 720.f, 30.f * viewport.y / 720.f, color);
	}
};

CColosseumMatchView::CColosseumMatchView() : m_Impl(std::make_unique<IMPLEMENTATION>()) {}
CColosseumMatchView::~CColosseumMatchView()
{
	m_Impl->End_Camera();
	for (auto* view : { m_Impl->hud.get(), m_Impl->result.get(), m_Impl->bars.get(), m_Impl->combat.get() })
		if (view) view->Release_Sprites();
}

bool_t CColosseumMatchView::Initialize(ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> context,
	uint32_t level, const std::shared_ptr<CCamera_Free>& camera)
{
	auto& p = *m_Impl;
	p.camera = camera;
	p.documentReady = p.Load_Document();
	p.hud = std::make_unique<CUILayoutRuntime>(device, context, level, L"Layer_ColosseumScore", L"UI/Colosseum/MatchHUD_Layout.json");
	p.result = std::make_unique<CUILayoutRuntime>(device, context, level, L"Layer_ColosseumResult", L"UI/Colosseum/Result_Layout.json");
	p.bars = std::make_unique<CUILayoutRuntime>(device, context, level, L"Layer_ColosseumVictoryBars", L"UI/Colosseum/IntroCutscene_Layout.json");
	p.hud->Set_UISortLayer(UI_TEXT_LAYER::HUD);
	p.combat = std::make_unique<CUILayoutRuntime>(device, context, level, L"Layer_ColosseumCombat", L"UI/Colosseum/CombatHUD_Layout.json");
	p.combat->Set_UISortLayer(UI_TEXT_LAYER::HUD);
	p.combat->Set_AllSlotsVisible(false);
	p.combatRects = IMPLEMENTATION::Capture_Rects(*p.combat);
	p.combatReady = p.combatRects.size() == 49u;
	if (!p.combatReady) p.Diagnose("team HP/kill feed layout is incomplete; other HUD remains available");
	p.result->Set_UISortLayer(UI_TEXT_LAYER::MODAL);
	p.bars->Set_UISortLayer(UI_TEXT_LAYER::MODAL + 1);
	for (auto* view : { p.hud.get(), p.result.get(), p.bars.get() }) view->Set_AllSlotsVisible(false);
	for (const auto& id : p.result->Get_SlotIds()) p.result->Set_SlotCinematicOverlay(id, true);
	for (const auto& id : p.bars->Get_SlotIds()) p.bars->Set_SlotCinematicOverlay(id, true);
	p.hudRects = IMPLEMENTATION::Capture_Rects(*p.hud);
	p.resultRects = IMPLEMENTATION::Capture_Rects(*p.result);
	float x, y, width, height;
	p.uiReady = p.hud->Get_SlotRect("Score_Frame", x, y, width, height) &&
		p.result->Get_SlotRect("Result_Return", x, y, width, height);
	for (const char* id : { "Result_Victory", "Result_Defeat", "Result_Draw" })
		p.uiReady = p.result->Get_SlotRect(id, x, y, width, height) && p.uiReady;
	if (!p.uiReady) p.Diagnose("match HUD/result layout or required image could not be loaded");
	return p.uiReady && p.documentReady;
}

void CColosseumMatchView::Update(f32_t delta, const CClientReplication& replication,
	const S2C_COLOSSEUM_MATCH_STATE& state, const double serverTick)
{
	auto& p = *m_Impl;
	if (!std::isfinite(serverTick) || !std::isfinite(delta) || delta < 0.f || !state.iMatchId) return;
#ifdef _DEBUG
	// A real Server match always supersedes any presentation-only audition.
	if (Is_DebugPreviewActive()) Stop_DebugPreview();
#endif
	if (state.iMatchId != p.matchId)
	{
		p.End_Camera(); p.matchId = state.iMatchId;
		p.returnIntent = false;
		p.returnRequestCooldown = 0.f;
	}
	p.returnRequestCooldown = (std::max)(0.f, p.returnRequestCooldown - delta);
	p.state = state; p.serverTick = serverTick;
	Sample_Presentation(replication, true);
}

void CColosseumMatchView::Sample_Presentation(const CClientReplication& replication, const bool allowReturn)
{
	auto& p = *m_Impl;
	const auto& state = p.state;
	p.showHud = state.ePhase == COLOSSEUM_MATCH_PHASE::PLAYING;
	p.hud->Set_AllSlotsVisible(p.showHud);
	p.Update_Combat(replication);
	IMPLEMENTATION::Anchor(*p.hud, p.hudRects);
	p.result->Set_AllSlotsVisible(false);
	p.showReturn = false;
	p.bannerId.clear();
	if (state.ePhase != COLOSSEUM_MATCH_PHASE::FINISHED)
	{
		p.End_Camera();
		return;
	}
	p.resultMs = static_cast<float>((std::max)(0.0, p.serverTick - state.iPhaseStartTick) * 1000.0 / TICKS_PER_SECOND);
	p.sceneMs = p.resultMs - p.document.bannerDuration;
	std::vector<REPLICATED_PLAYER_VIEW> players;
	replication.Collect_PlayerViews(players);
	p.localTeam = COLOSSEUM_DRAW_TEAM;
	for (const auto& player : players) if (player.isLocal)
		for (const auto& participant : state.Participants)
			if (player.iPlayerId == participant.iPlayerId && player.iNetEntityId == participant.iNetEntityId)
				p.localTeam = participant.iTeam;
	if (state.iWinningTeam != COLOSSEUM_DRAW_TEAM && (state.iWinningTeam > 1u || p.localTeam > 1u))
	{
		p.End_Camera();
		// A missing local identity is not evidence of a defeat. Retry the exact join next frame.
		const std::string failure = "Result waiting for the exact local player/team in the Server roster.";
		if (p.status != failure) p.Diagnose(failure);
		return;
	}
	p.bannerId = state.iWinningTeam == COLOSSEUM_DRAW_TEAM ? "Result_Draw" :
		(p.localTeam == state.iWinningTeam ? "Result_Victory" : "Result_Defeat");
	if (p.resultMs < p.document.bannerDuration)
	{
		if (p.resultMs < p.Banner_Duration())
		{
			p.result->Set_SlotVisible(p.bannerId, true);
			(void)p.result->Sample_KeyframeAnimation(p.bannerId, "start", p.resultMs / 1000.f);
		}
		return;
	}
	const bool winnerScene = p.documentReady && state.iWinningTeam != COLOSSEUM_DRAW_TEAM;
	if (winnerScene && p.sceneMs < p.document.duration)
	{
		p.Build_Actors(replication);
		for (auto& actor : p.actors)
			if (const auto character = actor.character.lock())
			{
				character->Set_CinematicPresentationSuppressed(!actor.winner);
				if (!actor.winner || actor.slot >= p.document.actors.size()) continue;
				const auto& slot = p.document.actors[actor.slot];
				character->Set_CutscenePoseOverride(slot.position, slot.yaw);
				if (p.sceneMs >= slot.clipStart && !actor.clipFailed &&
					!character->Sample_CutsceneAnimation(slot.clip, (p.sceneMs - slot.clipStart) / 1000.f, slot.loop))
				{
					actor.clipFailed = true;
					p.Diagnose("winner body has no exact source clip: " + slot.clip + "; entity=" + std::to_string(actor.id));
				}
			}
		p.Apply_Camera(p.sceneMs);
		p.Overlay(p.cinematic, p.Fade(p.sceneMs));
		return;
	}
	p.End_Camera();
	if (winnerScene && p.sceneMs < p.document.duration + p.document.returnFade)
	{
		p.cinematic = true;
		p.Overlay(false, 1.f - (p.sceneMs - p.document.duration) / p.document.returnFade);
		return;
	}
	// F1 auditions must never expose or synthesize the Server-authorized return action.
	if (!allowReturn) return;
	p.showReturn = true;
	p.result->Set_SlotVisible("Result_Return", true);
	IMPLEMENTATION::Anchor(*p.result, p.resultRects);
	CUIPointerScope pointer(this);
	auto& router = CUIInputRouter::Get();
	float x, y, width, height;
	if (p.result->Get_SlotRect("Result_Return", x, y, width, height))
	{
		if (router.Is_Hovered(x, y, width, height, 1280.f, 720.f)) router.Claim_Mouse_This_Frame();
		if (p.returnRequestCooldown <= 0.f && router.Is_Clicked(x, y, width, height, 1280.f, 720.f))
		{
			p.returnIntent = true;
			// Bound retries without trapping a rejected/failed transition in a disabled UI.
			p.returnRequestCooldown = 1.f;
		}
	}
}

#ifdef _DEBUG
bool_t CColosseumMatchView::Play_DebugPreview(const DEBUG_PREVIEW preview, const CClientReplication& replication)
{
	auto& p = *m_Impl;
	if (replication.Get_ColosseumMatchState().iMatchId)
	{
		p.Diagnose("F1 preview is unavailable during a Server match; use Debug Lobby > Colosseum Preview.");
		return false;
	}
	if (preview != DEBUG_PREVIEW::VICTORY_CUTSCENE && preview != DEBUG_PREVIEW::SCORE_HUD &&
		preview != DEBUG_PREVIEW::VICTORY_UI && preview != DEBUG_PREVIEW::DEFEAT_UI)
	{
		p.Diagnose("Unknown Colosseum preview kind; previous presentation preserved.");
		return false;
	}
	if (!p.documentReady || !p.uiReady || !p.hud || !p.result || !p.bars)
	{
		p.Diagnose("Preview unavailable: required victory document or UI was not initialized.");
		return false;
	}
	std::vector<REPLICATED_PLAYER_VIEW> players;
	replication.Collect_PlayerViews(players);
	const auto local = std::find_if(players.begin(), players.end(), [](const auto& player) {
		return player.isLocal && !player.pCharacter.expired();
	});
	if (local == players.end() || (preview == DEBUG_PREVIEW::VICTORY_CUTSCENE && p.camera.expired()))
	{
		p.Diagnose("Preview waiting for the actual local character/camera; retry after entry finishes.");
		return false;
	}
	// This read-only view fixture has no match ID and is never stored in replication.
	S2C_COLOSSEUM_MATCH_STATE staged;
	staged.ePhase = preview == DEBUG_PREVIEW::SCORE_HUD ? COLOSSEUM_MATCH_PHASE::PLAYING : COLOSSEUM_MATCH_PHASE::FINISHED;
	staged.iPhaseEndTick = 120u * 30u;
	const bool defeat = preview == DEBUG_PREVIEW::DEFEAT_UI;
	staged.iLeftScore = defeat ? 1u : 3u;
	staged.iRightScore = defeat ? 3u : 1u;
	staged.iWinningTeam = defeat ? 1u : 0u;
	staged.iWinnerTeam = staged.iWinningTeam;
	COLOSSEUM_MATCH_PLAYER_STATE participant;
	participant.iPlayerId = local->iPlayerId;
	participant.iNetEntityId = local->iNetEntityId;
	participant.bReady = true;
	participant.bParticipant = true;
	participant.iTeam = 0u;
	participant.iArrivalIndex = 0u;
	staged.Participants.push_back(participant);
	Stop_DebugPreview();
	p.state = std::move(staged);
	p.debugPreview = preview;
	p.status.clear();
	Update_DebugPreview(0.f, replication);
	return true;
}

void CColosseumMatchView::Update_DebugPreview(const f32_t delta, const CClientReplication& replication)
{
	auto& p = *m_Impl;
	if (!Is_DebugPreviewActive()) return;
	if (replication.Get_ColosseumMatchState().iMatchId)
	{
		Stop_DebugPreview();
		return;
	}
	if (!std::isfinite(delta) || delta < 0.f) return;
	if (!p.debugPaused) p.debugClockMs += (std::min)(delta, .2f) * 1000.f;
	const float duration = p.debugPreview == DEBUG_PREVIEW::VICTORY_UI ? p.document.bannerDurations[0] :
		(p.debugPreview == DEBUG_PREVIEW::DEFEAT_UI ? p.document.bannerDurations[1] :
		(p.debugPreview == DEBUG_PREVIEW::VICTORY_CUTSCENE ? p.document.duration + p.document.returnFade : 120000.f));
	if (p.debugPreview != DEBUG_PREVIEW::SCORE_HUD && p.debugClockMs >= duration)
	{
		Stop_DebugPreview();
		return;
	}
	p.debugClockMs = (std::min)(p.debugClockMs, duration);
	const float offset = p.debugPreview == DEBUG_PREVIEW::VICTORY_CUTSCENE ? p.document.bannerDuration : 0.f;
	p.serverTick = static_cast<double>(offset + p.debugClockMs) * TICKS_PER_SECOND / 1000.0;
	Sample_Presentation(replication, false);
}

void CColosseumMatchView::Stop_DebugPreview()
{
	auto& p = *m_Impl;
	p.End_Camera();
	for (auto* view : { p.hud.get(), p.result.get(), p.bars.get(), p.combat.get() })
		if (view) view->Set_AllSlotsVisible(false);
	p.debugPreview = DEBUG_PREVIEW::NONE;
	p.debugClockMs = 0.f;
	p.debugPaused = false;
	p.showHud = p.showReturn = p.returnIntent = false;
	p.state = {};
	p.matchId = 0u;
	p.resultMs = p.sceneMs = p.returnRequestCooldown = 0.f;
	p.bannerId.clear();
}

bool_t CColosseumMatchView::Is_DebugPreviewActive() const { return m_Impl->debugPreview != DEBUG_PREVIEW::NONE; }
bool_t CColosseumMatchView::Is_DebugPreviewPaused() const { return m_Impl->debugPaused; }
void CColosseumMatchView::Set_DebugPreviewPaused(const bool_t paused) { m_Impl->debugPaused = paused; }
f32_t CColosseumMatchView::Get_DebugPreviewClockMs() const { return m_Impl->debugClockMs; }
#endif

void CColosseumMatchView::Render()
{
	auto& p = *m_Impl;
	if (p.showHud && !CUIInputRouter::Get().Is_CinematicSuppressed())
	{
		CUITextLayerScope scope(UI_TEXT_LAYER::HUD);
		for (const auto& text : p.combatText)
			p.Slot_Text(*p.combat, text.slot.c_str(), text.text, text.pixels, text.color);
		const auto remaining = static_cast<uint32_t>(std::ceil((std::max)(0.0,
			static_cast<double>(p.state.iPhaseEndTick) - p.serverTick) / TICKS_PER_SECOND));
		p.Slot_Text(*p.hud, "Score_Time", std::to_wstring(remaining), 20.f, {1.f, 1.f, 1.f, 1.f});
		p.Slot_Text(*p.hud, "Score_Left", std::to_wstring(p.state.iLeftScore), 22.f, {1.f, .3f, .15f, 1.f});
		p.Slot_Text(*p.hud, "Score_Right", std::to_wstring(p.state.iRightScore), 22.f, {.15f, .65f, 1.f, 1.f});
		p.Slot_Text(*p.hud, "Score_TimeLabel", L"TIME", 12.f, {.85f, .85f, .85f, 1.f});
	}
	if (p.state.ePhase == COLOSSEUM_MATCH_PHASE::FINISHED && !p.bannerId.empty() && p.resultMs < p.Banner_Duration())
	{
		CUITextLayerScope scope(UI_TEXT_LAYER::MODAL);
		const std::wstring title = p.state.iWinningTeam == COLOSSEUM_DRAW_TEAM ? L"\xBB34\xC2B9\xBD80" :
			(p.localTeam == p.state.iWinningTeam ? L"\xC2B9\xB9AC" : L"\xD328\xBC30");
		p.Render_BannerTitle(title);
	}
	if (p.showReturn)
	{
		CUITextLayerScope scope(UI_TEXT_LAYER::MODAL);
		const std::wstring title = p.state.iWinningTeam == COLOSSEUM_DRAW_TEAM ? L"\xBB34\xC2B9\xBD80" :
			(p.localTeam == p.state.iWinningTeam ? L"\xC2B9\xB9AC" : L"\xD328\xBC30");
		p.Slot_Text(*p.result, "Result_Title", title + L"  " + std::to_wstring(p.state.iLeftScore) + L" : " +
			std::to_wstring(p.state.iRightScore), 26.f, {1.f, .9f, .65f, 1.f});
		p.Slot_Text(*p.result, "Result_Return", L"\xBCA0\xB978\xC73C\xB85C \xB3CC\xC544\xAC00\xAE30", 16.f, {1.f, 1.f, 1.f, 1.f});
	}
}

bool_t CColosseumMatchView::Is_CinematicActive() const { return m_Impl->cinematic; }
bool_t CColosseumMatchView::Consume_ReturnIntent()
{
	const bool result = m_Impl->returnIntent;
	m_Impl->returnIntent = false;
	return result;
}
const std::string& CColosseumMatchView::Get_Status() const { return m_Impl->status; }
