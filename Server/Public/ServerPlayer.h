#pragma once

// 연결 identity인 SESSION_ID와 무효 sentinel을 사용한다.
#include "ServerIds.h"

// 플레이어·엔티티·스킬·탈것·칭호의 Shared stable ID 형식을 사용한다.
#include "Network/NetworkIds.h"
// 직업·제어 주체 enum과 생성 음성의 최소값을 사용한다.
#include "Network/PacketType.h"
// 복제 상태·명령 payload·아이템 snapshot·buff와 장비 슬롯 판정 함수를 재사용한다.
#include "Network/PacketMessages.h"
// 이동 경로와 비행 안전 착지점의 SERVER_NAV_POINT 값을 보관한다.
#include "ServerNavigation.h"

// 선택적으로 보관하는 Mario 복귀/쿠크 부활 위치의 XYZ 고정 배열이다.
#include <array>
// Mario 접촉별 생성 시각과 접촉 여부를 키로 구분해 저장한다.
#include <map>
// Mario 접촉 키와 hit-window 대상의 pair 값을 만든다.
#include <utility>
// 아직 정해지지 않은 복귀/부활 위치와 실제 좌표를 구분한다.
#include <optional>
// wire와 대응하는 ID·tick·수치의 고정 폭 정수 형식을 사용한다.
#include <cstdint>
// 최대 192개의 스킬/투사체 반복 판정 여부를 bit로 보관한다.
#include <bitset>
// nickname·stable placement/asset ID·appearance JSON을 값으로 소유한다.
#include <string>
// 스킬/아이템별 cooldown 종료 tick과 길이를 key로 조회한다.
#include <unordered_map>
// 경로·투사체·접촉 기록·buff·인벤토리의 가변 길이 값을 소유한다.
#include <vector>

namespace LostArk::Server
{
	// 인벤토리 슬롯과 별도로 소유하는 실링·골드다. NPC 구매·수리·재련과 월드 전환에서 사용한다.
	enum class SERVER_CURRENCY : std::uint8_t
	{
		SILVER,
		GOLD
	};
	struct SERVER_PURSE
	{
		std::uint32_t iSilver = 0;
		std::uint32_t iGold = 0;

		// ItemCatalog 초기 지급과 상점 구매가 선택 재화의 참조를 얻는다. 반환 참조는 이 purse가 살아 있는 동안만 유효하다.
		// GOLD만 골드를 선택하고 그 외 값은 실링을 선택하므로 enum 유효성은 호출 경계에서 보장해야 한다.
		std::uint32_t& Amount(const SERVER_CURRENCY eCurrency)
		{
			return SERVER_CURRENCY::GOLD == eCurrency ? iGold : iSilver;
		}
	};

	// ServerTriggerSystem이 보간할 이동 경과 시간(ms)과 절대 월드 위치(m) 한 점이다.
	struct SERVER_TRIGGER_MOVE_SAMPLE
	{
		std::uint32_t iTimeMs = 0u;
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
	};

	// 플레이어가 값으로 소유하는 trigger 이동 상태다. ServerTriggerSystem이 초 단위 fixedDelta로 갱신하고
	// 완료/무효 이동 경로에서 비운다. 원본 placement나 track 데이터의 포인터를 보관하지 않는다.
	struct SERVER_TRIGGER_MOVE
	{
		// 검증된 직접 이동(예: Debug Mario 점프)은 비어 있다.
		// 저작 trigger는 도착 lane 변경에 사용할 원본 stable placement ID를 유지한다.
		std::string strSourcePlacementId;
		float fStartX = 0.f;
		float fStartY = 0.f;
		float fStartZ = 0.f;
		float fTargetX = 0.f;
		float fTargetY = 0.f;
		float fTargetZ = 0.f;
		float fDurationSeconds = 0.f;
		float fElapsedSeconds = 0.f;
		float fArcHeight = 0.f;

		// 목록이 비면 시작/도착점과 포물선 높이로 이동한다. 값이 있으면 Server가
		// 저작된 절대 월드 위치 표본을 시간에 따라 보간하고 Client는 복제 결과를 받는다.
		std::vector<SERVER_TRIGGER_MOVE_SAMPLE> TrackSamples;
		LostArk::Shared::KOUKU_HUD_MODE eKoukuHudModeOnArrival = LostArk::Shared::KOUKU_HUD_MODE::END;
		bool isActive = false;

		// 이동 시작 전에 fHoldSeconds초 동안 현재 위치에서 기다린다.
		// Bern 성/도서관 이동에서 화면이 어두워지는 대기이며 0이면 바로 이동한다.
		float fHoldSeconds = 0.f;
		float fHeldSeconds = 0.f;
	};

	// 투사체의 특정 hit 정의가 이미 접촉한 대상별 기록이다.
	// iAppliedCount와 다음 적용 시각으로 반복 횟수·간격을 지켜 같은 접촉의 중복 피해를 막는다.
	struct SERVER_PROJECTILE_CONTACT_MARK
	{
		LostArk::Shared::NET_ENTITY_ID iNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		std::uint8_t iHitIndex = 0;
		std::uint8_t iAppliedCount = 0;
		float fNextSeconds = 0.f;
	};

	// 스킬이 만든 투사체/고정 영역의 실행 상태다. 시전 action이 끝난 뒤에도 남아
	// CPlayerSkillSystem::Update_Projectiles가 갱신한다. skill/stage/projectile 번호로
	// 정의를 다시 찾으므로 catalog 내부 포인터는 저장하지 않는다.
	// 피해 분할은 시전자 action의 sub-hit 번호를 이어서 사용한다.
	struct SERVER_SKILL_PROJECTILE
	{
		LostArk::Shared::SKILL_ID iSkillId = LostArk::Shared::INVALID_SKILL_ID;
		std::uint8_t iStageIndex = 0;
		std::uint8_t iProjectileIndex = 0;
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		float fDirectionX = 0.f;
		float fDirectionZ = 1.f;
		float fSpeed = 0.f;

		// 남은 이동 거리(m)다. 음수면 거리 제한 없이 남은 수명으로 종료한다.
		float fRemainingDistance = -1.f;
		float fRemainingSeconds = 0.f;
		float fElapsedSeconds = 0.f;
		std::uint64_t iTotalDamage = 0;
		std::uint32_t iSubHitTotal = 1;
		std::uint32_t iSubHitBase = 0;
		std::bitset<192u> iAppliedTimedMask;
		std::vector<SERVER_PROJECTILE_CONTACT_MARK> ContactMarks;
	};

	// 넘어짐이 끝난 뒤 새 피격 반응을 막는 60tick 유예다. 고정 30Hz에서 2초에 해당한다.
	// HP 피해 면제 시간과는 다르며 기상/만료 처리에서 종료 tick에 더한다.
	inline constexpr std::uint32_t PLAYER_HIT_REACTION_GRACE_TICKS = 60;
	// 띄우기 착지 후 기존 기상 전환으로 이어질 때 확보하는 넘어짐 회복 시간(ms)이다.
	inline constexpr std::uint32_t PLAYER_HIT_LANDING_RECOVERY_MS = 1000u;

	enum class PLAYER_PENDING_COMMAND_KIND : std::uint8_t
	{
		NONE,
		MOVE,
		SKILL
	};

	// COMBO action 중 수락한 명시적 이동/스킬 의도 하나를 값으로 보관한다.
	// MOVE와 SKILL의 서로 다른 sequence 공간을 비교하지 않고 나중에 수락한 의도로 교체한다.
	struct SERVER_PENDING_PLAYER_COMMAND
	{
		PLAYER_PENDING_COMMAND_KIND eKind = PLAYER_PENDING_COMMAND_KIND::NONE;
		std::uint32_t iClientSequence = 0u;
		LostArk::Shared::SKILL_ID iSkillId = LostArk::Shared::INVALID_SKILL_ID;
		LostArk::Shared::SKILL_TARGET_INTENT_KIND eTargetIntent = LostArk::Shared::SKILL_TARGET_INTENT_KIND::AIM_POINT;
		float fX = 0.f;
		float fZ = 0.f;

		// 보류 명령 소비/취소와 Set_* 교체가 호출한다. 종류·sequence·좌표를 기본값으로 함께 초기화한다.
		void Clear()
		{
			*this = {};
		}

		// GameRoom의 COMBO 중 이동 입력 수락 경로가 호출한다. 이전 스킬 payload를 지우고 이동 sequence/목표 XZ를 복사한다.
		// 이 함수는 navigation·sequence 검증을 하지 않으며 실제 실행 시 룸이 현재 상태를 다시 검사한다.
		void Set_Move(const LostArk::Shared::C2S_MOVE& move)
		{
			Clear();
			eKind = PLAYER_PENDING_COMMAND_KIND::MOVE;
			iClientSequence = move.iClientSequence;
			fX = move.fGoalX;
			fZ = move.fGoalZ;
		}

		// PlayerSkillSystem의 보류 스킬 수락 경로가 호출한다. 이전 명령을 지우고 skill ID·target intent·조준 XZ를 복사한다.
		// 입력 packet의 메모리 수명과 무관하게 값으로 남으며 실행 시 자원/현재 action 등을 다시 검사한다.
		void Set_Skill(const LostArk::Shared::C2S_USE_SKILL& skill)
		{
			Clear();
			eKind = PLAYER_PENDING_COMMAND_KIND::SKILL;
			iClientSequence = skill.iClientSequence;
			iSkillId = skill.iSkillId;
			eTargetIntent = skill.eTargetIntent;
			fX = skill.fAimX;
			fZ = skill.fAimZ;
		}
	};

	// CGameRoom::m_Players가 값으로 소유하는 한 플레이어의 Server 실행 상태다. 인간과 AI가 같은 이동/전투 경로를 사용한다.
	// 구조체 자체에는 잠금이 없으며 룸 명령 큐를 거친 룸 worker가 상태를 갱신한다. 복사본은 입장/전환 검증에도 사용한다.
	// 이 메모리 배치를 wire로 직접 보내지 않고 GameRoom의 spawn/snapshot/capture 경로가 필요한 필드를 선택해 복제한다.
	struct SERVER_PLAYER
	{
		// 독립 Colosseum 경기의 부활 예약·전적·입장 순서다. 위치로 경기 참여를 추론하지 않는다.
		std::uint32_t iColosseumRespawnTick = 0u;
		std::uint32_t iColosseumKills = 0u;
		LostArk::Shared::PLAYER_ID iColosseumKillerId = LostArk::Shared::INVALID_PLAYER_ID;
		std::uint8_t iColosseumArrivalIndex = 255u;
		bool bColosseumReady = false, bColosseumCombatActive = false;
		float fColosseumSpawnX = 0.f, fColosseumSpawnY = 0.f, fColosseumSpawnZ = 0.f, fColosseumSpawnYaw = 0.f;
		LostArk::Shared::PLAYER_CONTROL_KIND eControlKind = LostArk::Shared::PLAYER_CONTROL_KIND::HUMAN;

		// 룸의 AI·전투·복제 분기가 제어 주체만 조회한다. session/위치/HP로 인간과 AI를 추론하지 않는다.
		bool Is_Guide() const noexcept
		{
			return eControlKind == LostArk::Shared::PLAYER_CONTROL_KIND::GUIDE_AI;
		}
		bool Is_Human() const noexcept
		{
			return eControlKind == LostArk::Shared::PLAYER_CONTROL_KIND::HUMAN;
		}
		bool Is_WaterpangAI() const noexcept
		{
			return eControlKind == LostArk::Shared::PLAYER_CONTROL_KIND::WATERPANG_AI;
		}
		std::string strWaterpangNpcArchetypeId;

		// 인간 연결의 session과 gameplay player/entity identity를 구분한다. AI는 가짜 인간 세션을 만들지 않는다.
		SESSION_ID iSessionId = INVALID_SESSION_ID;
		LostArk::Shared::PLAYER_ID iPlayerId = LostArk::Shared::INVALID_PLAYER_ID;
		LostArk::Shared::NET_ENTITY_ID iNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass = LostArk::Shared::CHARACTER_CLASS_ID::END;

		// 독립 경기 룸이 설정한 Colosseum 권위 식별자다.
		std::uint64_t iColosseumMatchId = 0u;

		// 160줄 기준 피해량 계산용 HP를 실제 40줄 경기 HP 풀과 별도로 고정한다.
		std::uint32_t iColosseumDamageReferenceHp = 0u;
		std::uint8_t iColosseumTeam = 255u;
		bool bColosseumParticipant = false;

		// Colosseum의 용병 입력/복제 경로가 제어 주체를 구분한다. 경기 참여 여부는 별도 필드로 판정한다.
		bool Is_ColosseumMercenary() const noexcept
		{
			return eControlKind == LostArk::Shared::PLAYER_CONTROL_KIND::COLOSSEUM_MERCENARY_AI;
		}
		std::string strNickName;

		// 캐릭터 생성 음성 유형(1~8)이며 nickname처럼 월드 전환 때 운반한다.
		std::uint8_t iVoiceType = LostArk::Shared::MIN_VOICE_TYPE;
		std::string strAppearanceJson;
		std::string strSpawnPlacementId;

		// Server가 검증한 Bern 레이드 입장 NPC의 stable ID다. 해당 레이드 방문의 복귀에 사용한다.
		std::string strRaidReturnNpcPlacementId;

		// 룸이 확정한 월드 위치(m)와 Y축 회전(degree)다. 이동·피격·복제의 기준 pose이며 Client 화면 위치가 정본이 아니다.
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		float fYawDegrees = 0.f;

		// 명령 종류별 마지막 처리 sequence다. 서로 다른 명령의 sequence를 한 순서 공간으로 합치지 않는다.
		std::uint32_t iLastMoveSequence = 0;
		std::uint32_t iLastReviveSequence = 0;
		std::uint32_t iLastRoomPingSequence = 0u, iLastRoomPingTick = 0u;
		std::uint32_t iLastClassChangeSequence = 0;

		// 이 룸의 같은 요청에 기존 판정을 다시 돌려줄 수 있도록 보관하는 플레이어별 응답 값이다.
		LostArk::Shared::S2C_DEBUG_TELEPORT_TO_POSITION_RESULT LastDebugTeleportResult;
		LostArk::Shared::S2C_DEBUG_MARIO_JUMP_RESULT LastDebugMarioJumpResult;
		LostArk::Shared::S2C_MARIO_RETURN_RESULT LastMarioReturnResult;
		LostArk::Shared::S2C_DEBUG_SET_MADNESS_FORM_RESULT LastDebugMadnessFormResult;
		LostArk::Shared::S2C_SET_VEHICLE_RIDING_RESULT LastVehicleRidingResult;
		LostArk::Shared::S2C_SET_HONOR_TITLE_RESULT LastHonorTitleResult;

		// 탑승 중인 탈것 ID이며 도보는 INVALID_VEHICLE_ID다.
		// 허용 월드/상태 판정과 Enforce_VehicleRidingState가 snapshot 전 무효 탑승을 정리한다.
		LostArk::Shared::VEHICLE_ID iVehicleId = LostArk::Shared::INVALID_VEHICLE_ID;

		// Bern 선박의 출발 부두 위치다. 항해 중 보관했다가 하선에 사용하며 이 필드 자체는 복제하지 않는다.
		bool bShipDockValid = false;
		float fShipDockX = 0.f, fShipDockY = 0.f, fShipDockZ = 0.f, fShipDockYawDegrees = 0.f;

		// 쾌속 항해 종료 tick이며 0은 미실행이다. 이 값을 별도 복제하지 않고
		// 증가한 이동 속도를 기존 SNAPSHOT_PLAYER::fMoveSpeed로 전달한다.
		std::uint32_t iShipBoostEndTick = 0u;

		// 탈것 비행 단계와 단계 시작 tick, 고도/안전 착지점이다. 입력·속도는 축별 월드 방향이며 비행 갱신이 적분한다.
		LostArk::Shared::VEHICLE_FLIGHT_PHASE eVehicleFlightPhase = LostArk::Shared::VEHICLE_FLIGHT_PHASE::GROUNDED;
		std::uint32_t iVehicleFlightPhaseStartTick = 0u;
		float fVehicleFlightPhaseSeconds = 0.f;
		float fVehicleFlightGroundY = 0.f;
		SERVER_NAV_POINT VehicleFlightSafeLanding{};
		float fVehicleFlightStartHeight = 0.f;
		float fVehicleFlightInputX = 0.f, fVehicleFlightInputZ = 0.f, fVehicleFlightInputY = 0.f;
		float fVehicleFlightInputAge = 0.f;
		float fVehicleFlightVelocityX = 0.f, fVehicleFlightVelocityZ = 0.f, fVehicleFlightVelocityY = 0.f;

		// 칭호 bootstrap이 허용한 장착 칭호다. 전투 능력 대신 외형 표시로 사용하고 월드 전환에서 운반한다.
		LostArk::Shared::HONOR_TITLE_ID iHonorTitleId = LostArk::Shared::INVALID_HONOR_TITLE_ID;

		// 현재 Mario 단계와 장애물별 접촉 이력이다. Clear_MarioControl이 실행 경계에서 함께 초기화한다.
		std::uint8_t iMarioStage = 0u;
		std::uint8_t iMarioBombContactStage = 0u;
		std::map<std::pair<std::uint32_t, std::uint32_t>, std::int64_t> MarioBombHitBirths;
		std::uint8_t iMarioBouncingBallContactStage = 0u;
		std::map<std::uint64_t, bool> MarioBouncingBallContacts;
		std::uint8_t iMarioLayoutVariant = 0u;

		// 사망/취소 또는 복귀 착지까지 입장자가 소유하는 공 색상 과제다.
		// 0은 미실행, 1/2/3은 빨강/파랑/노랑이며 표식 대상은 바깥 플레이어일 수 있다.
		std::uint8_t iMarioRequiredColor = 0u;
		LostArk::Shared::NET_ENTITY_ID iMarioMarkerNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;

		// Mario 과제 취소/종료와 최종 복귀가 호출한다. 필요한 색과 표식 entity만 지우고 다른 이동 상태는 건드리지 않는다.
		void Clear_MarioBallChallenge()
		{
			iMarioRequiredColor = 0u;
			iMarioMarkerNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		}

		// 입장 패턴이 고정한 복귀 위치다. 아레나 단계와 최종 복귀 이동 시작을 지나서도 보존한다.
		std::optional<std::array<float, 3u>> MarioReturnPosition;

		// 추락 사망 자세가 바닥 아래로 내려가도 유지하는 안전한 아레나 부활 위치다.
		std::optional<std::array<float, 3u>> KoukuFallRevivePosition;
		bool bKoukuFallDeath = false;

		// 워터팡 발판을 벗어나도 현재 경기 참가 여부를 유지하고 경기 복귀 확정 시 정리한다.
		bool bWaterpangParticipant = false;

		// 진행 중 워터팡 아레나 밖으로 밀려난 상태다. 추락 종료 시 가까운 점프 상자로 복귀한다.
		bool bWaterpangFall = false;

		// 워터팡 대포 물줄기에 마지막으로 맞은 tick이다. 물줄기의 0.4초 재적용 간격 판정에 사용한다.
		std::uint32_t iWaterpangCannonHitTick = 0u;

		// 워터팡 폭포로 발사된 비행 상태다. 일반 착지 대신 워터팡 추락 처리로 끝난다.
		bool bWaterpangLaunch = false;

		// 물총의 최근 시전 ID/tick은 복제하고, action 종료 tick은 다음 시전 대기에 사용한다.
		// 물총 시전은 이동을 잠그지 않으며 E의 속도 증가 종료 tick/배율은 별도로 유지한다.
		std::uint32_t iWaterGunSkillId = 0u;
		std::uint32_t iWaterGunCastTick = 0u;
		std::uint32_t iWaterGunCastEndTick = 0u;
		std::uint32_t iWaterGunSpeedEndTick = 0u;
		float fWaterGunSpeedScale = 1.f;
		LostArk::Shared::PLAYER_MADNESS_FORM ePreMarioForm = LostArk::Shared::PLAYER_MADNESS_FORM::NORMAL;
		std::uint32_t iLastMarioMoveSequence = 0u;
		std::uint32_t iMarioMoveExpiryTick = 0u;

		// Mario와 카드 미로에서 사용하는 Server 절대 종료 tick이다. 0은 비활성이다.
		std::uint32_t iKoukuMinigameEndTick = 0u;
		float fMarioDirectionX = 0.f;
		float fMarioDirectionZ = 0.f;
		bool bMarioRailReady = false;
		std::string strMarioRailArrivalId;
		float fMarioRailOriginX = 0.f;
		float fMarioRailOriginZ = 0.f;
		float fMarioRailRightX = 0.f;
		float fMarioRailRightZ = 0.f;

		// 룸의 Mario 종료/사망/reset 경로가 호출한다. active stage가 있을 때만 이전 form과 이동 제어를 복구한다.
		// preserveReturnPosition이 true면 최종 복귀 이동에서 필요한 복귀 좌표와 공 과제 identity를 남긴다.
		// 마지막 수신 sequence는 여기서 되돌리지 않으며 오래된 입력의 재수락 여부는 명령 경계가 판정한다.
		void Clear_MarioControl(const bool preserveReturnPosition = false)
		{
			if (0u != iMarioStage)
			{
				iKoukuMinigameEndTick = 0u;
				eMadnessForm = ePreMarioForm;
				// Mario 단계가 form과 아레나 HUD mode를 함께 소유하므로 둘 다 돌려준다.
				// Clear_KoukuInteractionState까지 호출하면 Debug override도 지워지므로 여기서는 호출하지 않는다.
				eKoukuAreaHudMode = LostArk::Shared::KOUKU_HUD_MODE::NONE;
				hasMoveGoal = false;
				MovePath.clear();
				iMovePathIndex = 0u;
			}
			if (!preserveReturnPosition)
			{
				MarioReturnPosition.reset();
				Clear_MarioBallChallenge();
			}
			iMarioStage = 0u;
			iMarioBombContactStage = 0u;
			MarioBombHitBirths.clear();
			iMarioBouncingBallContactStage = 0u;
			MarioBouncingBallContacts.clear();
			iMarioLayoutVariant = 0u;
			iMarioMoveExpiryTick = 0u;
			fMarioDirectionX = fMarioDirectionZ = 0.f;
			bMarioRailReady = false;
			strMarioRailArrivalId.clear();
			fMarioRailOriginX = fMarioRailOriginZ = 0.f;
			fMarioRailRightX = fMarioRailRightZ = 0.f;
		}

		float fMoveGoalX = 0.f;
		float fMoveGoalZ = 0.f;

		// navigation 투영 전 Client가 요청한 원래 목표점이다. 장애물 위 목표는 투영 후
		// 멀리 이동할 수 있으므로 원래 좌표로 같은 목표 재전송과 새 목표를 구별한다.
		float fMoveRequestX = 0.f;
		float fMoveRequestZ = 0.f;
		float fMoveSpeed = 6.f;
		bool hasMoveGoal = false;

		// 입장/부활 후 첫 유효 이동·스킬 의도를 Server가 수락할 때까지 발탄의 대상 선정·피해를 막는 상태다.
		bool isCombatReady = true;

		// 직선 이동이 가능한 목표는 빈 경로로 두고 fMoveGoal을 향한다.
		// 직선이 막혀 우회 경로를 만들었을 때만 waypoint 목록을 채운다.
		std::vector<SERVER_NAV_POINT> MovePath;
		std::size_t iMovePathIndex = 0;

		// grid에 없는 장애물과 몸체 sweep이 충돌해 마지막으로 우회 경로를 재생성한 tick이다.
		// 연속된 blocked 판정이 매 tick 경로를 다시 만들지 않도록 재탐색 간격에 사용한다.
		std::uint32_t iMoveRerouteTick = 0u;

		// 보스/몬스터 피격으로 진행 중인 밀침이다. XZ 단위 방향, 속도(m/s), 남은 시간(초)을 둔다.
		// 당김은 시작 시 공격자 쪽으로 방향을 바꾸며 진행 중 밀침/넘어짐의 재시작 여부는 피격 경로가 판정한다.
		float fKnockbackDirectionX = 0.f;
		float fKnockbackDirectionZ = 0.f;
		float fKnockbackSpeed = 0.f;
		float fKnockbackRemainingSeconds = 0.f;

		// 저작된 밀침에서 명시적으로 허용한 아레나 이탈이다. 일반 밀침은 navigation 경계에 제한된다.
		bool bKnockbackCanLeaveArena = false;

		// 명시적인 기믹 포물선 비행이다. 일반 밀침은 보행 지면을 따른다.
		bool bKnockbackBallistic = false;
		float fKnockbackVelocityY = 0.f;
		float fKnockbackLaunchY = 0.f;
		float fKnockbackSupportY = 0.f;
		static constexpr float KNOCKBACK_GRAVITY_MPS2 = 9.8f;
		float fKnockbackGravityMps2 = KNOCKBACK_GRAVITY_MPS2;

		// 기존 밀침 적분기에 전달하는 아레나 방출 상태다. 방출은 navigation/충돌 제한을 우회하고 FALLING으로 이어진다.
		bool bArenaEjectionActive = false;
		LostArk::Shared::NET_ENTITY_ID iEjectionOwnerNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;

		// KNOCKDOWN 유지 종료 tick이다. 진행 중에는 이동/스킬을 제한하고 만료 시 action을 NONE으로 되돌린다.
		std::uint32_t iKnockdownEndTick = 0;

		// 밀침 전용 넘어짐 표시는 유지하되 저작된 넘어짐 면역을 추가로 부여하지 않는 상태다.
		bool bPushOnlyHitReaction = false;
		std::uint32_t iFearEndTick = 0u;
		std::string strFearPresentationId;

		// 현재 tick의 영역 접촉/효과 pulse 표시다. 영역을 나가면 표시는 만료하고 실제 보호 판정은 패턴 경계에서 처리한다.
		std::uint32_t iInvulnerabilityZoneContactTick = 0u;
		std::uint32_t iInvulnerabilityZonePulseTick = 0u;

		// 넘어짐 만료나 STANDUP 스킬 후 새 피격 반응을 열지 않는 종료 tick이다.
		// 이 유예 자체는 HP 피해를 차단하지 않는다.
		std::uint32_t iHitReactionGraceEndTick = 0;

		std::uint32_t iCurrentHp = 1000;
		std::uint32_t iMaximumHp = 1000;

		// 내구도는 장착 아이템의 Inventory[].iDurabilityPercent에 저장한다.
		// HP가 줄어든 인간 플레이어의 피격 경로가 장비 하나씩 순환해 감소시키고
		// bDurabilityDirty는 룸 tick이 소유자에게 갱신 snapshot을 보낼 필요가 있음을 나타낸다.
		std::uint8_t iDurabilityWearCursor = 0;
		bool bDurabilityDirty = false;

		// ServerCombatHitRuntime이 실제 HP가 줄어든 인간 피격에 호출한다. 최대 6개 장착 장비의 비소유 포인터를 잠시 모아
		// 순환 대상 하나의 내구도를 percentLoss만큼 0 아래로 내려가지 않게 줄인다. 이 함수 안에서는 Inventory를 재배치하지 않는다.
		// 장착 장비가 없으면 상태를 보존한다. 선택 장비가 이미 0이면 cursor만 전진하고 dirty는 추가로 세우지 않는다.
		void Wear_Durability(const std::uint8_t percentLoss) noexcept
		{
			LostArk::Shared::INVENTORY_ITEM_SNAPSHOT* worn[6] = {};
			std::size_t count = 0;
			for (LostArk::Shared::INVENTORY_ITEM_SNAPSHOT& item : Inventory)
			{
				if (LostArk::Shared::Is_Durable_Slot(item.eEquippedSlot) && count < 6u)
					worn[count++] = &item;
			}
			if (0u == count)
				return;
			LostArk::Shared::INVENTORY_ITEM_SNAPSHOT& piece = *worn[iDurabilityWearCursor % count];
			iDurabilityWearCursor = static_cast<std::uint8_t>((iDurabilityWearCursor + 1u) % 60u);
			if (0u == piece.iDurabilityPercent)
				return;
			piece.iDurabilityPercent = piece.iDurabilityPercent > percentLoss
			                               ? static_cast<std::uint8_t>(piece.iDurabilityPercent - percentLoss)
			                               : std::uint8_t{0};
			bDurabilityDirty = true;
		}

		// Server 내부 피해 표본 순번이다. 같은 tick에 들어온 여러 접촉이 하나의 피해 난수를 공유하지 않게 한다.
		std::uint64_t iIncomingDamageSampleSerial = 0u;
		std::uint32_t iCurrentResource = 100;
		std::uint32_t iMaximumResource = 100;

		// 정수 자원 회복의 나머지다. 회복 가능한 tick마다 profile의 초당 회복량을 더하고
		// SERVER_TICK_HZ만큼 쌓일 때 자원 1을 지급한다. 최대치/스킬 상태는 Update가 따로 제한한다.
		std::uint32_t iResourceAccumulator = 0;

		// 직업 identity 현재량·최대량과 정수 누적 나머지다. 직업 profile의 회복/소모 경로가 갱신하며
		// identity gauge가 없는 직업은 0으로 초기화한다.
		std::uint32_t iCurrentIdentity = 0;
		std::uint32_t iMaximumIdentity = 0;
		std::uint32_t iIdentityAccumulator = 0;

		// 가디언 나이트 잔불의 현재 보유 수, 인간형 스킬이 잠근 소켓 수, 현재 action이 소비한 수다.
		// 잠긴 소켓은 보유 한도를 줄이고 action 소비량은 해당 스킬의 피해 계산에 쓰인다.
		std::uint32_t iEmberOrbs = 0;
		std::uint32_t iEmberLockedSockets = 0;
		std::uint32_t iEmberSpentOnAction = 0;

		// 광기 게이지의 기본 최대 단위다. 룸/패턴 정책이 피해·월드 접촉 증가량을 정하며
		// 현재량·최대량·변신 상태는 모두 플레이어 snapshot으로 복제한다.
		static constexpr std::uint32_t MADNESS_GAUGE_MAXIMUM = 10000u;

		// 플레이어가 소유한 활성 buff 목록이다. tick에서 만료를 정리하고 같은 buff의 재시전은 기존 값을 교체한다.
		std::vector<LostArk::Shared::ACTIVE_BUFF> ActiveBuffs;

		// 일반 피해에서 HP보다 먼저 소모하는 보호막이다. 즉사 피해 등 우회 정책은 전투 피격 경로가 결정한다.
		std::uint32_t iShield = 0;

		// 사망 방지 buff가 HP를 0으로 만들 일반 피해를 HP 1로 제한한 뒤의 무적 종료 tick이다.
		// 명시적인 즉사(lethal) 피해는 이 보호를 우회한다.
		std::uint32_t iInvulnerableEndTick = 0;

		// 빙고/에스더 기믹 성공의 명시적 보호 종료 시각이다. 일반 무적만으로 빙고 전멸 판정을 통과하지 않는다.
		std::uint32_t iKoukuBingoLineProtectionEndTick = 0u, iEstherZoneProtectionEndTick = 0u;

		// 아이템의 시간 정지/성스러운 부적 보호를 기믹에서 주는 무적과 별도로 유지한다.
		std::uint32_t iTimeStopEndTick = 0u, iHolyCharmProtectionEndTick = 0u;
		std::uint32_t iLastItemUseSequence = 0u;
		std::unordered_map<std::string, std::uint32_t> ItemCooldownEndTicks;

		// 입력·피격·복제 경로가 현재 Server tick을 넣어 조회한다. 0은 비활성이고 종료 tick 자체에서는 false다.
		// 부호 있는 tick 차이로 비교하므로 사용하는 기간은 32비트 반주기보다 짧다는 기존 전제를 따른다.
		bool Has_TimeStop(const std::uint32_t tick) const noexcept
		{
			return iTimeStopEndTick && static_cast<std::int32_t>(tick - iTimeStopEndTick) < 0;
		}

		// 피격·패턴·복제가 성스러운 부적의 보호 기간을 조회한다. 종료/0 판정은 시간 정지와 같고 상태를 변경하지 않는다.
		bool Has_HolyCharmProtection(const std::uint32_t tick) const noexcept
		{
			return iHolyCharmProtectionEndTick && static_cast<std::int32_t>(tick - iHolyCharmProtectionEndTick) < 0;
		}
		bool bRonaunGuard = false;
		std::uint32_t iRonaunGrantTick = 0u;
		std::uint32_t iEstherGuardEndTick = 0;
		std::int32_t iEstherGuardDamageTakenPercent = 0;

		// 룸 정책이 피해 기반 광기 증가율을 정하고 소수 나머지를 보관해 작은 피해의 누적을 잃지 않는다.
		std::uint32_t iMadnessDamageGainPercent = 0u;
		double dMadnessRemainder = 0.;
		std::uint32_t iCurrentMadness = 0;
		std::uint32_t iMaximumMadness = MADNESS_GAUGE_MAXIMUM;
		LostArk::Shared::PLAYER_MADNESS_FORM eMadnessForm = LostArk::Shared::PLAYER_MADNESS_FORM::NORMAL;

		// 게이지로 발생한 광대 변신의 종료 tick이다. 0은 시각 만료가 없다는 뜻이며
		// Debug 전환은 이 값을 0으로 두어 명시적 복귀 경로에서 해제한다.
		std::uint32_t iMadnessFormEndTick = 0u;

		// CKoukuSaydonLogicRuntime::Update_PlayerModes가 갱신하는 상호작용 HUD 상태다.
		// Client에 표시할 mode와 Q~F 슬롯의 저작 icon index(-1은 없음)를 보관한다.
		// 춤 포즈 Q/W/E/R은 슈퍼맨/팔 벌리기/외발/팔짱 순서다.
		LostArk::Shared::KOUKU_HUD_MODE eKoukuHudMode = LostArk::Shared::KOUKU_HUD_MODE::NONE;
		LostArk::Shared::KOUKU_HUD_MODE eDebugKoukuHudModeOverride = LostArk::Shared::KOUKU_HUD_MODE::NONE;
		std::int8_t ModeSkillIndexBySlot[LostArk::Shared::KOUKU_HUD_SLOT_COUNT] = {-1, -1, -1, -1, -1, -1, -1, -1};
		std::uint32_t iLastKoukuInteractionSequence = 0u;
		bool bKoukuPatternOwnsClown = false;
		std::uint32_t iKoukuSuppressedPatternSequence = 0u;
		LostArk::Shared::KOUKU_HUD_MODE eKoukuAreaHudMode = LostArk::Shared::KOUKU_HUD_MODE::NONE;

		// 진행 중인 1관문 세이튼 패턴이 배정한 문양/색상이다.
		LostArk::Shared::MECHANIC_CARD_SYMBOL eMechanicCardSymbol = LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE;
		LostArk::Shared::MECHANIC_CARD_COLOR eMechanicCardColor = LostArk::Shared::MECHANIC_CARD_COLOR::NONE;

		// 망원경을 선택한 카드 미로 실행이 배정한 역할·문양·진행 상태다.
		// 같은 문양 enum을 쓰는 룰렛 상태와 구별하여 미로 실행 종료 경로에서 정리한다.
		LostArk::Shared::CARD_MAZE_ROLE eCardMazeRole = LostArk::Shared::CARD_MAZE_ROLE::NONE;
		LostArk::Shared::MECHANIC_CARD_SYMBOL eCardMazeSuit = LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE;
		std::uint8_t iCardMazeKills = 0u;
		std::uint8_t iCardMazeKillTarget = 0u;
		LostArk::Shared::CARD_MAZE_PRESENTATION CardMaze;
		float fCardMazeTransferX = 0.f, fCardMazeTransferY = 0.f, fCardMazeTransferZ = 0.f;
		bool bCardMazeTransferCommitted = false;

		// 쿠크 미로 실행 종료가 역할·문양·처치 수·표현 payload를 지운다.
		// CardMazeTransfer 좌표/commit flag와 공용 HUD 상태까지 초기화하는 함수는 아니다.
		void Clear_CardMazeState() noexcept
		{
			eCardMazeRole = LostArk::Shared::CARD_MAZE_ROLE::NONE;
			eCardMazeSuit = LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE;
			iCardMazeKills = 0u;
			iCardMazeKillTarget = 0u;
			CardMaze = {};
		}

		LostArk::Shared::S2C_DEBUG_SET_KOUKU_HUD_MODE_RESULT LastDebugKoukuHudModeResult;

		// 입장·레이드 전환·trigger 도착·reset이 쿠크 변신/HUD 소유권과 기믹 보호 타이머를 정리한다.
		// 현재 eMadnessForm이나 Mario 이동 전체를 대신 초기화하지 않으며 호출자가 필요한 별도 정리를 수행한다.
		void Clear_KoukuInteractionState()
		{
			iMadnessFormEndTick = 0u;
			bKoukuPatternOwnsClown = false;
			iKoukuSuppressedPatternSequence = 0u;
			eKoukuAreaHudMode = LostArk::Shared::KOUKU_HUD_MODE::NONE;
			iKoukuMinigameEndTick = 0u;
			iKoukuBingoLineProtectionEndTick = iEstherZoneProtectionEndTick = 0u;
			eKoukuHudMode = LostArk::Shared::KOUKU_HUD_MODE::NONE;
			eDebugKoukuHudModeOverride = LostArk::Shared::KOUKU_HUD_MODE::NONE;
			for (std::int8_t& index : ModeSkillIndexBySlot)
				index = -1;
		}

		// 1관문 카드 배정 해제와 룸 reset이 호출한다. 룰렛 문양/색만 지우므로 카드 미로 상태와 혼동하지 않는다.
		void Clear_KoukuAssignedCard()
		{
			eMechanicCardSymbol = LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE;
			eMechanicCardColor = LostArk::Shared::MECHANIC_CARD_COLOR::NONE;
		}

		LostArk::Shared::PLAYER_ACTION_STATE eAction = LostArk::Shared::PLAYER_ACTION_STATE::NONE;
		LostArk::Shared::PLAYER_STANCE_ID eStance = LostArk::Shared::PLAYER_STANCE_ID::NONE;

		// 창술사의 양방향 stance 전환(예: 34000/34500)은 서로 다른 스킬별 cooldown을 가진다.
		// 한쪽 전환 직후 반대 스킬로 즉시 돌아가지 못하도록 이 공통 종료 tick을 함께 검사한다.
		std::uint32_t iStanceSwitchCooldownEndTick = 0;
		LostArk::Shared::SKILL_ID iCurrentSkillId = LostArk::Shared::INVALID_SKILL_ID;
		std::uint32_t iActionStartTick = 0;

		// 진행 중인 구멍 통과 연주의 1부터 시작하는 행 번호이며 0은 없음이다.
		// 연주 잠금이 끝나면 해당 목적지로 이동하고 action이 SQUAREHOLE_SONG이 아니면 Update_Players가 정리한다.
		std::uint16_t iSquareHoleId = 0u;

		// 보스 패턴의 특정 occurrence가 소유한 구속 상태다. 해제 경로는 저장한 pose를
		// 검증/복원한 뒤 이동·action 제어를 돌려준다. 아래 Clear 함수 자체가 pose를 복원하지는 않는다.
		bool bPatternBound = false;
		LostArk::Shared::NET_ENTITY_ID iPatternBindOwnerNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		std::uint32_t iPatternBindSequence = 0u;
		std::uint32_t iPatternBindEndTick = 0u;
		float fPatternBindRestoreX = 0.f;
		float fPatternBindRestoreY = 0.f;
		float fPatternBindRestoreZ = 0.f;
		float fPatternBindRestoreYawDegrees = 0.f;
		bool bPatternBindRestoreCombatReady = false;

		// 침묵 자체는 이동을 막지 않는다. 적용한 보스 occurrence보다 오래 남을 수 있으며
		// 종료 tick·사망·명시적인 룸/reset 경계에서 정리한다.
		LostArk::Shared::NET_ENTITY_ID iSilenceOwnerNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		std::uint32_t iSilencePatternSequence = 0u;
		std::uint32_t iSilenceEndTick = 0u;
		std::uint32_t iSilenceDurationTicks = 0u;

		// 구속 해제/사망/reset 경로가 소유자·기한과 저장된 복원 pose를 비운다.
		// 현재 위치/action은 바꾸지 않는다. 정상 해제의 위치 검증·복원은 룸 호출자가 이 함수보다 먼저 수행한다.
		void Clear_PatternBindStatus()
		{
			bPatternBound = false;
			iPatternBindOwnerNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			iPatternBindSequence = 0u;
			iPatternBindEndTick = 0u;
			fPatternBindRestoreX = 0.f;
			fPatternBindRestoreY = 0.f;
			fPatternBindRestoreZ = 0.f;
			fPatternBindRestoreYawDegrees = 0.f;
			bPatternBindRestoreCombatReady = false;
		}

		// 침묵 만료/사망/reset이 적용 occurrence와 기한을 초기화한다. 현재 스킬·이동을 취소하는 함수는 아니다.
		void Clear_SilenceStatus()
		{
			iSilenceOwnerNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			iSilencePatternSequence = 0u;
			iSilenceEndTick = 0u;
			iSilenceDurationTicks = 0u;
		}

		// GRABBED의 부착 위치는 Server 권위다. Shared slot ID와 보스 로컬 offset으로
		// 플레이어 월드 위치·yaw를 계산하며 Client 손 본으로 다시 조합하지 않는다.
		LostArk::Shared::NET_ENTITY_ID iAttachmentOwnerNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		LostArk::Shared::PLAYER_ATTACHMENT_SLOT eAttachmentSlot = LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE;
		std::uint32_t iAttachmentPatternSequence = 0u;
		// 0이면 기존 발탄의 단계 기반 부착 수명에 맡기고 별도 시간 만료를 두지 않는다.
		std::uint32_t iAttachmentEndTick = 0u;
		float fAttachmentLocalOffsetX = 0.f;
		float fAttachmentLocalOffsetY = 0.f;
		float fAttachmentLocalOffsetZ = 0.f;
		float fAttachmentYawOffsetDegrees = 0.f;

		// WORLD_HOOK_TIP은 보스 본 대신 저작된 판정 영역을 따라간다.
		// 잡은 window/region과 월드 track의 해제 tick을 저장하며 owner ID는 해당 패턴의 보스를 유지한다.
		// 다른 slot에서는 이 세 필드를 0으로 둔다.
		std::uint32_t iAttachmentWindowIndex = 0u;
		std::uint32_t iAttachmentRegionIndex = 0u;
		std::uint32_t iAttachmentReleaseTick = 0u;

		// 잡기 해제·피격·사망/reset이 부착 identity/offset/기한과 아레나 방출·포물선 비행 표시를 비운다.
		// 현재 pose/action과 일반 밀침 잔여 시간을 여기서 복원하지 않으므로 호출 경로가 함께 처리해야 한다.
		void Clear_Attachment()
		{
			bArenaEjectionActive = false;
			bKnockbackCanLeaveArena = false;
			bKnockbackBallistic = false;
			fKnockbackVelocityY = 0.f;
			iEjectionOwnerNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			iAttachmentOwnerNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			eAttachmentSlot = LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE;
			iAttachmentPatternSequence = 0u;
			iAttachmentEndTick = 0u;
			fAttachmentLocalOffsetX = 0.f;
			fAttachmentLocalOffsetY = 0.f;
			fAttachmentLocalOffsetZ = 0.f;
			fAttachmentYawOffsetDegrees = 0.f;
			iAttachmentWindowIndex = 0u;
			iAttachmentRegionIndex = 0u;
			iAttachmentReleaseTick = 0u;
		}

		// FALLING 동안 사용하는 낙하 속도와 사망 예약 tick이다. 낙하 시작 시 속도를 초기화한 뒤
		// 중력으로 갱신한다. 이 값 자체를 복제하지 않고 Client는 snapshot 위치로 하강을 표현한다.
		float fFallVelocityY = 0.f;

		// 추락 사망 판정의 월드 Y다. Begin_PlayerFall이 월드/쿠크 영역별 깊이를 지지면 또는 비행 출발 높이에서 뺀다.
		float fFallDeathPlaneY = 0.f;
		std::uint32_t iFallDeathTick = 0u;
		SERVER_TRIGGER_MOVE TriggerMove;
		std::uint32_t iLastSkillSequence = 0;
		float fActionElapsedSeconds = 0.f;
		float fSkillAimDirectionX = 0.f;
		float fSkillAimDirectionZ = 1.f;

		// 입력한 조준점까지 시전자와의 거리(m)다. 커서 위치에 생성되는 스킬 객체의 배치에 사용한다.
		float fSkillAimDistance = 0.f;

		// 지면 목표의 유한값·거리·navigation 검증 후 확정한 월드 위치다.
		// action 동안 유지하며 SKILL snapshot에 복사해 피해 판정과 Client 표현이 같은 목표를 사용한다.
		bool hasSkillTarget = false;
		float fSkillTargetX = 0.f;
		float fSkillTargetY = 0.f;
		float fSkillTargetZ = 0.f;

		// 스킬 종료·중단·전환/reset이 확정 지면 목표의 유효 표시와 XYZ를 함께 비운다.
		// 조준 방향/거리나 action 상태는 별도 필드이므로 그대로 남는다.
		void Clear_SkillTarget()
		{
			hasSkillTarget = false;
			fSkillTargetX = 0.f;
			fSkillTargetY = 0.f;
			fSkillTargetZ = 0.f;
		}

		bool hasAppliedSkillDamage = false;
		std::bitset<192u> iAppliedHitMask;

		// 열려 있는 hit window가 이미 판정한 (collider 반복 번호, 대상 entity ID) 쌍이다.
		std::vector<std::pair<std::uint8_t, LostArk::Shared::NET_ENTITY_ID>> HitWindowTargets;

		// 현재 stage에서 이미 생성한 투사체 정의별 bit다.
		std::uint16_t iSpawnedProjectileMask = 0;
		std::vector<SERVER_SKILL_PROJECTILE> Projectiles;

		// combo 진행 중에는 1부터 시작하고 그 외에는 0이다.
		std::uint8_t iComboStage = 0;

		// 열린 combo 입력 구간에서 설정하고 stage 종료 때 소비한다.
		bool hasBufferedComboInput = false;

		// 예약한 combo 입력의 조준 방향과 거리다. 다음 stage는 첫 타의 방향 대신 이 입력으로 회전한다.
		float fBufferedComboAimX = 0.f;
		float fBufferedComboAimZ = 1.f;
		float fBufferedComboAimDistance = 0.f;
		SERVER_PENDING_PLAYER_COMMAND PendingCommand;

		// HOLD 스킬 키를 놓았을 때 설정하고 반복 구간이 끝날 때 소비한다.
		bool hasReleasedHold = false;
		std::unordered_map<LostArk::Shared::SKILL_ID, std::uint32_t> CooldownEndTickBySkillId;

		// 플레이어 스킬의 전체 cooldown 길이다. 탈것/기믹 타이머와 구별해 스킬별 종료 tick과 함께 사용한다.
		std::unordered_map<LostArk::Shared::SKILL_ID, std::uint32_t> CooldownDurationTicksBySkillId;

		// 플레이어가 소유한 아이템 목록이다. Debug 지급뿐 아니라 구매·장착·수리·재련·캡처/복원에서 사용한다.
		// Shared snapshot 형식을 재사용해 ID·수량·장착 슬롯·강화 단계·내구도를 함께 보관하며
		// 크기와 최대 중첩 수는 catalog/룸 명령의 검증 경계에서 제한한다.
		std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT> Inventory;
		SERVER_PURSE Purse;

		// 새 입장 초기 지급 뒤 첫 인벤토리 변경 또는 한 번의 C2S_RESTORE_CHARACTER까지 열리는 복원 기회다.
		// 검증에 실패한 복원 요청도 기회를 소비하며 기존 상태를 운반한 월드 전환은 이를 열지 않는다.
		bool bRestoreAvailable = false;
		std::uint32_t iLastEquipmentUpgradeSequence = 0u;
	};
}
