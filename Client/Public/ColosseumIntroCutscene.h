#pragma once

#include "Client_Defines.h"
#include "Camera_Free.h"
#include "Character.h"
#include "ClientReplication.h"
#include "DataJson.h"
#include "GameInstance.h"
#include "ProjectDataRoot.h"
#include "Transform.h"
#include "UILabelFont.h"
#include "UILayoutRuntime.h"
#include "UITextOcclusion.h"
#include "WorldPlayerNameplateView.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

NS_BEGIN(Client)

/* Colosseum match intro, restored from the retail LV_PVP_COLOSSEUM_SCENE01A Matinee
   (Data/Camera/ColosseumIntro.cutscene.json): fade in, an aerial shot of the arena, a cut to the
   lineup shot with both teams facing each other, VS and one "job / nickname" label pair under every
   player's feet, fade to black. Afterwards the Level's own follow camera and the Server's pen
   positions are back, so each team waits in its holding pen.

	   The Server owns the match clock and roster. The players are the real replicated Characters: while the
   cutscene plays each one is shown at its lineup slot through Character::Set_CutscenePoseOverride
   (presentation transform only), and the pen position from the snapshot returns the moment the
   override is cleared. Input is blocked for the whole time through Is_Active(); the Level gates
   its PlayerController with it and MainApp hides the HUD with it. A missing or invalid document
   only skips the cutscene. */
class CColosseumIntroCutscene final
{
public:
	CColosseumIntroCutscene() = default;
	CColosseumIntroCutscene(const CColosseumIntroCutscene&) = delete;
	CColosseumIntroCutscene& operator=(const CColosseumIntroCutscene&) = delete;
	~CColosseumIntroCutscene() { Release(); }

	bool_t Initialize(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext,
		const uint32_t iLevelIndex, const std::shared_ptr<CCamera_Free>& pCamera)
	{
		m_pDevice = std::move(pDevice);
		m_pContext = std::move(pContext);
		m_iLevelIndex = iLevelIndex;
		m_pCamera = pCamera;
		if (nullptr == pCamera || !Load_Document() || !Create_View())
		{
			m_ePhase = PHASE::FAILED;
			return false;
		}
		Show_Overlay(false, 1.f);
		m_ePhase = PHASE::WAITING;
		return true;
	}

	/* True from level entry until the closing fade has finished: input and HUD stay blocked. */
	bool_t Is_Active() const { return PHASE::WAITING == m_ePhase || PHASE::PLAYING == m_ePhase || PHASE::RETURN == m_ePhase; }
	const std::string& Get_Status() const { return m_strStatus; }

	/* F1 replay controls. Restart re-reads Data/Camera/ColosseumIntro.cutscene.json, so an edited
	   camera/FOV/timing document is picked up without a rebuild. It also recovers from a document
	   that was rejected at level entry. */
	bool_t Restart()
	{
		Stop();
		if (nullptr == m_pCamera.lock())
		{
			m_strStatus = "no gameplay camera to drive";
			m_ePhase = PHASE::FAILED;
			return false;
		}
		if (!Load_Document() || !Create_View())
		{
			m_ePhase = PHASE::FAILED;
			return false;
		}
		m_strStatus.clear();
		m_bPaused = false;
		m_fClockMs = 0.f;
		Show_Overlay(false, 1.f);
		m_ePhase = PHASE::WAITING;
		return true;
	}
	void Stop()
	{
		Release();
		m_bPaused = false;
		m_fClockMs = 0.f;
		m_ePhase = PHASE::DONE;
	}
	void Set_Paused(const bool_t bPaused) { if (PHASE::PLAYING == m_ePhase) m_bPaused = bPaused; }
	void Seek(const f32_t fMs)
	{
		if (PHASE::PLAYING == m_ePhase && std::isfinite(fMs))
			m_fClockMs = std::clamp(fMs, 0.f, m_Doc.fDurationMs - 1.f);
	}
	bool_t Is_Paused() const { return m_bPaused; }
	bool_t Is_Playing() const { return PHASE::PLAYING == m_ePhase; }
	f32_t Get_ClockMs() const { return m_fClockMs; }
	f32_t Get_DurationMs() const { return m_Doc.fDurationMs; }
	f32_t Get_FovXDegrees() const { return m_Doc.fFovXDegrees; }
	size_t Get_ActorCount() const { return m_Actors.size(); }
	const char* Get_PhaseLabel() const
	{
		switch (m_ePhase)
		{
		case PHASE::WAITING: return "waiting for the local character";
		case PHASE::PLAYING: return m_bPaused ? "paused" : "playing";
		case PHASE::RETURN: return "closing fade";
		case PHASE::FAILED: return "failed";
		default: return "stopped";
		}
	}

	void Update(const f32_t fTimeDelta, const CClientReplication& replication)
	{
		const auto& match = replication.Get_ColosseumMatchState();
		if (match.iMatchId)
		{
			Update_ServerTimeline(replication, match, replication.Get_ColosseumServerTick());
			return;
		}
		if (!Is_Active())
			return;
		const std::shared_ptr<CCamera_Free> pCamera = m_pCamera.lock();
		if (nullptr == pCamera || !std::isfinite(fTimeDelta) || fTimeDelta < 0.f)
		{
			Release();
			m_ePhase = PHASE::DONE;
			return;
		}
		/* Level activation blocks the frame that first runs this cutscene for seconds. That hitch is
		   load time, not cutscene time: it once skipped the whole 3 s aerial shot. No single frame
		   may advance the clock by more than MAX_STEP_MS, and the frame that starts playback adds
		   nothing. */
		const f32_t fDeltaMs = (std::min)(fTimeDelta * 1000.f, MAX_STEP_MS);
		f32_t fPlayStepMs = fDeltaMs;

		if (PHASE::WAITING == m_ePhase)
		{
			m_fClockMs += fDeltaMs;
			const std::shared_ptr<CCharacter> pLocal = replication.Get_LocalCharacter();
			LostArk::Shared::PLAYER_ACTION_STATE eAction{};
			if (nullptr != pLocal && nullptr != pLocal->Get_Transform() &&
				pLocal->Try_Get_NetworkActionState(eAction))
			{
				Build_Actors(replication);
				m_fClockMs = 0.f;
				fPlayStepMs = 0.f;
				m_ePhase = PHASE::PLAYING;
			}
			else if (m_fClockMs >= m_Doc.fWaitTimeoutMs)
			{
				m_strStatus = "local character never became ready; cutscene skipped";
				OutputDebugStringA("[Colosseum.Intro] local character never became ready; cutscene skipped\n");
				m_fClockMs = 0.f;
				m_ePhase = PHASE::RETURN;
			}
			else
			{
				Show_Overlay(false, 1.f);
				return;
			}
		}

		if (PHASE::PLAYING == m_ePhase)
		{
			if (!m_bPaused)
				m_fClockMs += fPlayStepMs;
			if (m_fClockMs >= m_Doc.fDurationMs)
			{
				Clear_Actors();
				(void)pCamera->End_PresentationOverride(CAMERA_OWNER);
				m_fClockMs = 0.f;
				m_ePhase = PHASE::RETURN;
			}
			else
			{
				for (const ACTOR& actor : m_Actors)
					if (const std::shared_ptr<CCharacter> pCharacter = actor.pCharacter.lock())
						pCharacter->Set_CutscenePoseOverride(actor.vPosition, actor.fYawDegrees);
				Apply_Camera(*pCamera);
				Show_Overlay(true, Sample_Fade(m_fClockMs));
				Update_Vs();
				return;
			}
		}

		// RETURN: the follow camera and the pen positions are already back; reveal them.
		m_fClockMs += fDeltaMs;
		const f32_t fRatio = m_Doc.fReturnFadeMs > 0.f ? m_fClockMs / m_Doc.fReturnFadeMs : 1.f;
		if (fRatio >= 1.f)
		{
			Release();
			m_ePhase = PHASE::DONE;
			return;
		}
		Show_Overlay(false, 1.f - fRatio);
	}

	void Render()
	{
		if (PHASE::PLAYING != m_ePhase)
			return;
		CGameInstance& gameInstance = CGameInstance::Get();
		const float4x4_t* const pView = gameInstance.Get_Transform(D3DTS::VIEW);
		const float4x4_t* const pProj = gameInstance.Get_Transform(D3DTS::PROJ);
		const float2_t vViewport = gameInstance.Get_ViewportSize();
		if (nullptr == pView || nullptr == pProj || vViewport.x <= 0.f || vViewport.y <= 0.f)
			return;
		const f32_t fRefToScreen = vViewport.y / 720.f;
		for (const ACTOR& actor : m_Actors)
		{
			const ROW_TIMING& row = m_Doc.Rows[actor.iRow];
			const f32_t fAlpha = Ramp(m_fClockMs, row.fShowMs, row.fHideMs, LABEL_RAMP_MS);
			if (fAlpha <= 0.f)
				continue;
			float2_t vFeet{};
			if (!CWorldPlayerNameplateView::Try_ProjectWorldPosition(
				float3_t(actor.vPosition.x, m_Doc.fFloorY, actor.vPosition.z), *pView, *pProj, vViewport, vFeet))
				continue;
			Draw_Line(gameInstance, actor.strJob, vFeet.x, vFeet.y + JOB_OFFSET_REF * fRefToScreen,
				JOB_FONT_REF * fRefToScreen, XMVectorSet(0.66f, 0.82f, 1.f, fAlpha), fAlpha, fRefToScreen);
			Draw_Line(gameInstance, actor.strName, vFeet.x, vFeet.y + NAME_OFFSET_REF * fRefToScreen,
				NAME_FONT_REF * fRefToScreen, XMVectorSet(0.96f, 0.97f, 1.f, fAlpha), fAlpha, fRefToScreen);
		}
	}

private:
	void Update_ServerTimeline(const CClientReplication& replication,
		const LostArk::Shared::S2C_COLOSSEUM_MATCH_STATE& match, const double serverTick)
	{
		using P = LostArk::Shared::COLOSSEUM_MATCH_PHASE;
		const auto camera = m_pCamera.lock();
		if (!camera || !m_pView || m_ePhase == PHASE::FAILED) return;
		if (match.ePhase == P::LOADING ||
			(match.ePhase == P::INTRO && serverTick < match.iPhaseStartTick))
		{
			m_ePhase = PHASE::WAITING;
			Show_Overlay(false, 0.f); // The shared loading view owns this entire interval.
			return;
		}
		if (match.ePhase != P::INTRO)
		{
			if (m_ePhase != PHASE::DONE)
			{
				Clear_Actors();
				(void)camera->End_PresentationOverride(CAMERA_OWNER);
				Show_Overlay(false, 0.f);
				m_ePhase = PHASE::DONE;
			}
			return;
		}
		const f32_t elapsedMs = static_cast<f32_t>((std::max)(0.0, serverTick - match.iPhaseStartTick) * (1000.0 / 30.0));
		if (elapsedMs < m_Doc.fDurationMs)
		{
			m_ePhase = PHASE::PLAYING;
			m_fClockMs = elapsedMs;
			// Reconcile actual stable IDs each frame: a departed participant is removed,
			// and a late presentation may never be permanently lost from the lineup.
			Clear_Actors();
			Build_Actors(replication);
			for (const auto& actor : m_Actors)
				if (const auto character = actor.pCharacter.lock()) character->Set_CutscenePoseOverride(actor.vPosition, actor.fYawDegrees);
			Apply_Camera(*camera);
			Show_Overlay(true, Sample_Fade(m_fClockMs));
			Update_Vs();
		}
		else
		{
			Clear_Actors();
			(void)camera->End_PresentationOverride(CAMERA_OWNER);
			m_ePhase = PHASE::RETURN;
			m_fClockMs = elapsedMs - m_Doc.fDurationMs;
			Show_Overlay(false, 1.f - std::clamp(m_fClockMs / (std::max)(1.f, m_Doc.fReturnFadeMs), 0.f, 1.f));
		}
	}

	enum class PHASE { WAITING, PLAYING, RETURN, DONE, FAILED };

	struct SHOT_KEY { f32_t fTimeMs = 0.f; float3_t vEye = {}; };
	struct SHOT
	{
		f32_t fStartMs = 0.f, fEndMs = 0.f;
		std::vector<SHOT_KEY> Keys;
		float3_t vForward = {};
	};
	struct ROW_TIMING { f32_t fShowMs = 0.f, fHideMs = 0.f; };
	struct LINEUP_SLOT { f32_t fX = 0.f, fZ = 0.f, fYawDegrees = 0.f; uint32_t iRow = 0u; };
	struct DOCUMENT
	{
		f32_t fDurationMs = 0.f, fReturnFadeMs = 0.f, fWaitTimeoutMs = 0.f;
		f32_t fFloorY = 0.f, fArenaCenterX = 0.f, fFovXDegrees = 75.f, fLetterboxAspect = 2.35f;
		std::vector<std::pair<f32_t, f32_t>> Fade;
		std::vector<SHOT> Shots;
		f32_t fVsShowMs = 0.f, fVsHideMs = 0.f;
		ROW_TIMING Rows[4];
		std::vector<LINEUP_SLOT> Teams[2];
	};
	struct ACTOR
	{
		std::weak_ptr<CCharacter> pCharacter;
		std::wstring strJob, strName;
		float3_t vPosition = {};
		f32_t fYawDegrees = 0.f;
		uint32_t iRow = 0u;
	};
	struct VS_SLOT { std::string strId; f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f; };

	static constexpr uint64_t CAMERA_OWNER = 0x434f4c4f494e5452ull;
	static constexpr f32_t MAX_STEP_MS = 200.f;
	static constexpr f32_t VS_RAMP_IN_MS = 220.f, VS_RAMP_OUT_MS = 150.f, LABEL_RAMP_MS = 150.f;
	static constexpr f32_t JOB_FONT_REF = 13.f, NAME_FONT_REF = 15.f;
	static constexpr f32_t JOB_OFFSET_REF = 10.f, NAME_OFFSET_REF = 28.f;
	static constexpr const char* VS_SLOTS[] = {
		"Intro_VS_Glow", "Intro_VS_Beam", "Intro_VS_SparksA", "Intro_VS_SparksB",
		"Intro_VS_Flare", "Intro_VS_V", "Intro_VS_S" };

	// ---- overlay view ------------------------------------------------------------------
	bool_t Create_View()
	{
		m_pView = std::make_unique<CUILayoutRuntime>(m_pDevice, m_pContext, m_iLevelIndex,
			TEXT("Layer_ColosseumIntro"), L"UI/Colosseum/IntroCutscene_Layout.json");
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (!m_pView->Get_SlotRect("Intro_Fade", fX, fY, fW, fH) ||
			!m_pView->Get_SlotRect("Intro_BarTop", fX, fY, fW, fH) ||
			!m_pView->Get_SlotRect("Intro_BarBottom", fX, fY, fW, fH))
		{
			m_strStatus = "IntroCutscene_Layout.json is missing its bar/fade slots";
			Release();
			return false;
		}
		m_pView->Set_UISortLayer(UI_TEXT_LAYER::MODAL);
		m_VsSlots.clear();
		for (const char* pId : VS_SLOTS)
		{
			VS_SLOT slot;
			slot.strId = pId;
			if (m_pView->Get_SlotRect(pId, slot.fX, slot.fY, slot.fW, slot.fH))
				m_VsSlots.push_back(std::move(slot));
		}
		/* While this cutscene runs the UI router is in cinematic suppression, which hides every
		   sprite that is not flagged as a cinematic overlay -- the bars, the fade plate and the VS
		   art were all invisible in the first playtest. */
		for (const std::string& strId : m_pView->Get_SlotIds())
			m_pView->Set_SlotCinematicOverlay(strId, true);
		return true;
	}

	// ---- document ----------------------------------------------------------------------
	static const DATA_JSON_VALUE& Field(const DATA_JSON_VALUE& row, const char* pKey)
	{
		const DATA_JSON_VALUE* pValue = row.Find(pKey);
		if (nullptr == pValue)
			throw std::runtime_error(std::string("missing field: ") + pKey);
		return *pValue;
	}
	static f32_t Number(const DATA_JSON_VALUE& row, const char* pKey, double lo, double hi)
	{
		const DATA_JSON_VALUE& value = Field(row, pKey);
		if (!value.Is_Number() || !std::isfinite(value.Get_Number()) ||
			value.Get_Number() < lo || value.Get_Number() > hi)
			throw std::runtime_error(std::string("invalid number: ") + pKey);
		return static_cast<f32_t>(value.Get_Number());
	}
	static const DATA_JSON_VALUE::ARRAY& Array(const DATA_JSON_VALUE& row, const char* pKey)
	{
		const DATA_JSON_VALUE& value = Field(row, pKey);
		if (!value.Is_Array())
			throw std::runtime_error(std::string("invalid array: ") + pKey);
		return value.Get_Array();
	}
	static float3_t Vector(const DATA_JSON_VALUE& row, const char* pKey)
	{
		const DATA_JSON_VALUE::ARRAY& values = Array(row, pKey);
		if (3u != values.size())
			throw std::runtime_error(std::string("invalid vector: ") + pKey);
		float3_t out{};
		f32_t* const pOut[3] = { &out.x, &out.y, &out.z };
		for (size_t i = 0; i < 4u; ++i)
		{
			if (!values[i].Is_Number() || !std::isfinite(values[i].Get_Number()) ||
				std::abs(values[i].Get_Number()) > 100000.0)
				throw std::runtime_error(std::string("invalid coordinate: ") + pKey);
			*pOut[i] = static_cast<f32_t>(values[i].Get_Number());
		}
		return out;
	}

	bool_t Load_Document()
	{
		try
		{
			const std::filesystem::path path = CProjectDataRoot::Resolve(L"Camera/ColosseumIntro.cutscene.json");
			if (std::filesystem::file_size(path) > 1048576u)
				throw std::runtime_error("document exceeds 1 MiB");
			std::ifstream input(path, std::ios::binary);
			std::ostringstream bytes;
			bytes << input.rdbuf();
			DATA_JSON_VALUE root;
			std::string error;
			if (!input || !CDataJson::Parse(bytes.str(), root, error))
				throw std::runtime_error("parse failed: " + error);
			const DATA_JSON_VALUE& schema = Field(root, "schema");
			if (!schema.Is_String() || "lostark.colosseum-intro" != schema.Get_String() ||
				1.f != Number(root, "formatVersion", 1, 1))
				throw std::runtime_error("wrong schema");

			DOCUMENT doc;
			doc.fDurationMs = Number(root, "durationMs", 1000, 60000);
			doc.fReturnFadeMs = Number(root, "returnFadeMs", 0, 5000);
			doc.fWaitTimeoutMs = Number(root, "waitForCharacterTimeoutMs", 500, 60000);
			doc.fFloorY = Number(root, "floorY", -1000, 1000);
			doc.fArenaCenterX = Number(root, "arenaCenterX", -1000, 1000);
			doc.fFovXDegrees = Number(root, "fovXDegrees", 20, 150);
			doc.fLetterboxAspect = Number(root, "letterboxAspect", 1, 4);
			for (const DATA_JSON_VALUE& key : Array(root, "fade"))
				doc.Fade.emplace_back(Number(key, "timeMs", 0, 60000), Number(key, "value", 0, 1));
			if (doc.Fade.empty())
				throw std::runtime_error("empty fade");
			for (const DATA_JSON_VALUE& shotRow : Array(root, "shots"))
			{
				SHOT shot;
				shot.fStartMs = Number(shotRow, "startMs", 0, 60000);
				shot.fEndMs = Number(shotRow, "endMs", 0, 60000);
				shot.vForward = Vector(shotRow, "forward");
				for (const DATA_JSON_VALUE& keyRow : Array(shotRow, "keys"))
					shot.Keys.push_back({ Number(keyRow, "timeMs", 0, 60000), Vector(keyRow, "eye") });
				if (shot.Keys.empty() || shot.fEndMs <= shot.fStartMs ||
					(!doc.Shots.empty() && shot.fStartMs < doc.Shots.back().fEndMs))
					throw std::runtime_error("invalid shot order");
				doc.Shots.push_back(std::move(shot));
			}
			if (doc.Shots.empty() || 0.f != doc.Shots.front().fStartMs)
				throw std::runtime_error("cutscene must start with a shot at 0");
			const DATA_JSON_VALUE& vs = Field(root, "vs");
			doc.fVsShowMs = Number(vs, "showMs", 0, 60000);
			doc.fVsHideMs = Number(vs, "hideMs", 0, 60000);
			const DATA_JSON_VALUE::ARRAY& rows = Array(root, "rows");
			if (4u != rows.size())
				throw std::runtime_error("expected four name rows");
			for (size_t i = 0; i < 4u; ++i)
			{
				doc.Rows[i].fShowMs = Number(rows[i], "showMs", 0, 60000);
				doc.Rows[i].fHideMs = Number(rows[i], "hideMs", 0, 60000);
			}
			const DATA_JSON_VALUE& teams = Field(root, "teams");
			const char* const pTeamKeys[2] = { "A", "B" };
			for (size_t t = 0; t < 2u; ++t)
			{
				for (const DATA_JSON_VALUE& slotRow : Array(teams, pTeamKeys[t]))
				{
					LINEUP_SLOT slot;
					slot.fX = Number(slotRow, "x", -1000, 1000);
					slot.fZ = Number(slotRow, "z", -1000, 1000);
					slot.fYawDegrees = Number(slotRow, "yawDegrees", -720, 720);
					slot.iRow = static_cast<uint32_t>(Number(slotRow, "row", 0, 3));
					doc.Teams[t].push_back(slot);
				}
				if (doc.Teams[t].size() != 4u)
					throw std::runtime_error("expected four lineup slots per team");
			}
			m_Doc = std::move(doc);
			return true;
		}
		catch (const std::exception& e)
		{
			m_strStatus = std::string("Colosseum intro document rejected: ") + e.what();
			OutputDebugStringA(("[Colosseum.Intro] " + m_strStatus + "\n").c_str());
			return false;
		}
	}

	// ---- actors ------------------------------------------------------------------------
	static const wchar_t* Job_Name(const LostArk::Shared::CHARACTER_CLASS_ID eClass)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		switch (eClass)
		{
		case CHARACTER_CLASS_ID::WARLORD: return L"\xC6CC\xB85C\xB4DC";
		case CHARACTER_CLASS_ID::LANCE_MASTER: return L"\xCC3D\xC220\xC0AC";
		case CHARACTER_CLASS_ID::ARTIST: return L"\xB3C4\xD654\xAC00";
		case CHARACTER_CLASS_ID::GUARDIANKNIGHT: return L"\xAC00\xB514\xC5B8\xB098\xC774\xD2B8";
		case CHARACTER_CLASS_ID::DIMENSIONMASTER: return L"\xCC28\xC6D0\xC220\xC0AC";
		case CHARACTER_CLASS_ID::SLAYER: return L"\xC2AC\xB808\xC774\xC5B4";
		case CHARACTER_CLASS_ID::GUNSLINGER: return L"\xAC74\xC2AC\xB9C1\xC5B4";
		default: return L"";
		}
	}

	/* The actual server roster is the only team/slot authority, not world X or entity order. */
	void Build_Actors(const CClientReplication& replication)
	{
		std::vector<REPLICATED_PLAYER_VIEW> players;
		replication.Collect_PlayerViews(players);
		const auto& match = replication.Get_ColosseumMatchState();
		std::vector<std::pair<LostArk::Shared::NET_ENTITY_ID, ACTOR>> teams[2];
		for (const REPLICATED_PLAYER_VIEW& player : players)
		{
			const std::shared_ptr<CCharacter> pCharacter = player.pCharacter.lock();
			if (nullptr == pCharacter || nullptr == pCharacter->Get_Transform() ||
				pCharacter->Is_ShipPresentation())
				continue;
			ACTOR actor;
			actor.pCharacter = pCharacter;
			actor.strJob = Job_Name(player.eCharacterClass);
			(void)CWorldPlayerNameplateView::Try_ConvertUtf8(player.strNickname, actor.strName);
			const auto row = std::find_if(match.Participants.begin(), match.Participants.end(), [&](const auto& entry) {
				return entry.iNetEntityId == player.iNetEntityId && entry.iPlayerId == player.iPlayerId;
			});
			if (row != match.Participants.end() && row->bParticipant && row->iTeam < 2u &&
				row->iArrivalIndex < LostArk::Shared::MAX_COLOSSEUM_COMBAT_PLAYERS)
				teams[row->iTeam].emplace_back(row->iArrivalIndex, std::move(actor));
			else if (!match.iMatchId && player.isLocal) // Explicit no-match F1 camera preview only.
				teams[0].emplace_back(0u, std::move(actor));
		}
		m_Actors.clear();
		for (size_t t = 0; t < 2u; ++t)
		{
			std::sort(teams[t].begin(), teams[t].end(),
				[](const auto& a, const auto& b) { return a.first < b.first; });
			for (auto& participant : teams[t])
			{
				const auto slotIndex = participant.first / 2u;
				if (slotIndex >= m_Doc.Teams[t].size()) continue;
				ACTOR actor = std::move(participant.second);
				const LINEUP_SLOT& slot = m_Doc.Teams[t][slotIndex];
				actor.vPosition = float3_t(slot.fX, m_Doc.fFloorY, slot.fZ);
				actor.fYawDegrees = slot.fYawDegrees;
				actor.iRow = slot.iRow;
				m_Actors.push_back(std::move(actor));
			}
		}
	}

	void Clear_Actors()
	{
		for (const ACTOR& actor : m_Actors)
			if (const std::shared_ptr<CCharacter> pCharacter = actor.pCharacter.lock())
				pCharacter->Clear_CutscenePoseOverride();
		m_Actors.clear();
	}

	// ---- camera / overlay --------------------------------------------------------------
	f32_t Sample_Fade(const f32_t fMs) const
	{
		const auto& keys = m_Doc.Fade;
		if (fMs <= keys.front().first)
			return keys.front().second;
		for (size_t i = 1; i < keys.size(); ++i)
		{
			if (fMs <= keys[i].first)
			{
				const f32_t fSpan = keys[i].first - keys[i - 1].first;
				const f32_t fRatio = fSpan > 0.f ? (fMs - keys[i - 1].first) / fSpan : 1.f;
				return keys[i - 1].second + (keys[i].second - keys[i - 1].second) * fRatio;
			}
		}
		return keys.back().second;
	}

	void Apply_Camera(CCamera_Free& camera)
	{
		const SHOT* pShot = &m_Doc.Shots.front();
		for (const SHOT& shot : m_Doc.Shots)
			if (shot.fStartMs <= m_fClockMs)
				pShot = &shot;
		float3_t vEye = pShot->Keys.front().vEye;
		for (size_t i = 1; i < pShot->Keys.size(); ++i)
		{
			if (m_fClockMs > pShot->Keys[i].fTimeMs)
			{
				vEye = pShot->Keys[i].vEye;
				continue;
			}
			const SHOT_KEY& a = pShot->Keys[i - 1];
			const SHOT_KEY& b = pShot->Keys[i];
			const f32_t fSpan = b.fTimeMs - a.fTimeMs;
			const f32_t fRatio = fSpan > 0.f ? std::clamp((m_fClockMs - a.fTimeMs) / fSpan, 0.f, 1.f) : 1.f;
			vEye = float3_t(a.vEye.x + (b.vEye.x - a.vEye.x) * fRatio,
				a.vEye.y + (b.vEye.y - a.vEye.y) * fRatio, a.vEye.z + (b.vEye.z - a.vEye.z) * fRatio);
			break;
		}
		const float3_t vLook(vEye.x + pShot->vForward.x * 10.f, vEye.y + pShot->vForward.y * 10.f,
			vEye.z + pShot->vForward.z * 10.f);
		/* The retail FOV is horizontal for the letterboxed frame; keep that horizontal angle at any
		   window aspect and let the bars cut the vertical range. */
		const f32_t fAspect = camera.Get_AspectRatio() > 0.1f ? camera.Get_AspectRatio() : 16.f / 9.f;
		const f32_t fHalfX = XMConvertToRadians(m_Doc.fFovXDegrees) * 0.5f;
		const f32_t fFovY = XMConvertToDegrees(2.f * std::atan(std::tan(fHalfX) / fAspect));
		if (camera.Begin_PresentationOverride(CAMERA_OWNER, Engine::CCamera::PRESENTATION_PRIORITY::SERVER_CINEMATIC))
			(void)camera.Apply_PresentationPose(CAMERA_OWNER, vEye, vLook, fFovY);
	}

	static f32_t Ramp(const f32_t fMs, const f32_t fShowMs, const f32_t fHideMs, const f32_t fRampMs)
	{
		if (fMs < fShowMs || fMs >= fHideMs + fRampMs)
			return 0.f;
		if (fMs < fShowMs + fRampMs)
			return (fMs - fShowMs) / fRampMs;
		if (fMs < fHideMs)
			return 1.f;
		return 1.f - (fMs - fHideMs) / fRampMs;
	}

	void Show_Overlay(const bool_t bBars, const f32_t fFadeAlpha)
	{
		if (nullptr == m_pView)
			return;
		const float2_t vViewport = CGameInstance::Get().Get_ViewportSize();
		f32_t fBarRef = 0.f;
		if (vViewport.x > 0.f && vViewport.y > 0.f)
		{
			const f32_t fVisible = (std::min)(1.f, (vViewport.x / vViewport.y) / m_Doc.fLetterboxAspect);
			fBarRef = 720.f * (1.f - fVisible) * 0.5f;
		}
		m_pView->Set_SlotVisible("Intro_BarTop", bBars && fBarRef > 0.f);
		m_pView->Set_SlotVisible("Intro_BarBottom", bBars && fBarRef > 0.f);
		if (bBars && fBarRef > 0.f)
		{
			m_pView->Set_SlotRect("Intro_BarTop", 0.f, 0.f, 1280.f, fBarRef);
			m_pView->Set_SlotRect("Intro_BarBottom", 0.f, 720.f - fBarRef, 1280.f, fBarRef);
		}
		m_pView->Set_SlotVisible("Intro_Fade", fFadeAlpha > 0.001f);
		m_pView->Set_SlotTintMultiplier("Intro_Fade", float4_t(1.f, 1.f, 1.f, std::clamp(fFadeAlpha, 0.f, 1.f)));
		if (!bBars)
			for (const VS_SLOT& slot : m_VsSlots)
				m_pView->Set_SlotVisible(slot.strId, false);
	}

	void Update_Vs()
	{
		const f32_t fShow = m_Doc.fVsShowMs;
		f32_t fAlpha = 0.f, fScale = 1.f;
		if (m_fClockMs >= fShow && m_fClockMs < m_Doc.fVsHideMs + VS_RAMP_OUT_MS)
		{
			if (m_fClockMs < fShow + VS_RAMP_IN_MS)
			{
				const f32_t fRatio = (m_fClockMs - fShow) / VS_RAMP_IN_MS;
				fAlpha = fRatio;
				fScale = 1.f + 0.35f * (1.f - fRatio);
			}
			else if (m_fClockMs < m_Doc.fVsHideMs)
				fAlpha = 1.f;
			else
				fAlpha = 1.f - (m_fClockMs - m_Doc.fVsHideMs) / VS_RAMP_OUT_MS;
		}
		for (const VS_SLOT& slot : m_VsSlots)
		{
			m_pView->Set_SlotVisible(slot.strId, fAlpha > 0.001f);
			if (fAlpha <= 0.001f)
				continue;
			const f32_t fCenterX = slot.fX + slot.fW * 0.5f, fCenterY = slot.fY + slot.fH * 0.5f;
			m_pView->Set_SlotRect(slot.strId, fCenterX - slot.fW * fScale * 0.5f,
				fCenterY - slot.fH * fScale * 0.5f, slot.fW * fScale, slot.fH * fScale);
			m_pView->Set_SlotTintMultiplier(slot.strId, float4_t(1.f, 1.f, 1.f, std::clamp(fAlpha, 0.f, 1.f)));
		}
	}

	static void Draw_Line(CGameInstance& gameInstance, const std::wstring& strText, const f32_t fCenterX,
		const f32_t fTop, const f32_t fPx, const fvector_t vColor, const f32_t fAlpha, const f32_t fRefToScreen)
	{
		if (strText.empty())
			return;
		f32_t fScale = 1.f;
		const wstring_t strFont = UILabelFont::Resolve(TEXT("Font_YG760"), fPx, fScale);
		const float2_t vSize = gameInstance.Measure_Text(strFont, strText.c_str());
		const float2_t vPosition(std::round(fCenterX - vSize.x * fScale * 0.5f), std::round(fTop));
		const f32_t fStroke = fRefToScreen >= 1.8f ? 2.f : 1.f;
		static constexpr f32_t OFFSETS[8][2] = {
			{ -1.f, 0.f }, { 1.f, 0.f }, { 0.f, -1.f }, { 0.f, 1.f },
			{ -1.f, -1.f }, { 1.f, -1.f }, { -1.f, 1.f }, { 1.f, 1.f } };
		const fvector_t vOutline = XMVectorSet(0.f, 0.f, 0.f, 0.9f * fAlpha);
		for (const auto& offset : OFFSETS)
			gameInstance.Draw_Text(strFont, strText.c_str(),
				float2_t(vPosition.x + offset[0] * fStroke, vPosition.y + offset[1] * fStroke),
				vOutline, 0.f, float2_t(0.f, 0.f), fScale);
		gameInstance.Draw_Text(strFont, strText.c_str(), vPosition, vColor, 0.f, float2_t(0.f, 0.f), fScale);
	}

	void Release()
	{
		Clear_Actors();
		if (const std::shared_ptr<CCamera_Free> pCamera = m_pCamera.lock())
			(void)pCamera->End_PresentationOverride(CAMERA_OWNER);
		if (m_pView)
		{
			m_pView->Set_AllSlotsVisible(false);
			m_pView->Release_Sprites();
			m_pView.reset();
		}
		m_VsSlots.clear();
	}

private:
	PHASE m_ePhase = PHASE::DONE;
	ComPtr<ID3D11Device> m_pDevice;
	ComPtr<ID3D11DeviceContext> m_pContext;
	uint32_t m_iLevelIndex = 0u;
	bool_t m_bPaused = false;
	std::weak_ptr<CCamera_Free> m_pCamera;
	DOCUMENT m_Doc;
	std::unique_ptr<CUILayoutRuntime> m_pView;
	std::vector<VS_SLOT> m_VsSlots;
	std::vector<ACTOR> m_Actors;
	f32_t m_fClockMs = 0.f;
	std::string m_strStatus;
};

NS_END
