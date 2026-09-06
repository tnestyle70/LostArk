#include "imgui.h"

#include "PlayerController.h"

#include "Character.h"
#include "CombatHUDViewModel.h"
#include "GameInstance.h"
#include "PlayerCommandSink.h"
#include "PlayerSkillCatalog.h"
#include "SkillGroundTargetPreview.h"
#include "ClickMoveEffect.h"
#include "Transform.h"
#include "UIInputRouter.h"

#include <cmath>
#include <cstdio>

namespace
{
	/* Which physical key each quick slot sits on. This table is class-agnostic on
	purpose: the slot name to skill pairing lives in Data/Balance/PlayerSkills.json
	per class, so adding another class's skills there binds them with no code
	change here. Slot names match the document's inputSlot strings.

	ALT_V shares DIK_V with V, so the modifier decides which of the two is
	eligible and holding Alt never also fires the unmodified slot. */
	struct SLOT_KEY
	{
		const char* pInputSlot;
		uint8_t byKeyCode;
		bool_t requiresAlt;
	};

	constexpr SLOT_KEY SlotKeys[] =
	{
		{ "Q",     DIK_Q,     false },
		{ "W",     DIK_W,     false },
		{ "E",     DIK_E,     false },
		{ "R",     DIK_R,     false },
		{ "A",     DIK_A,     false },
		{ "S",     DIK_S,     false },
		{ "D",     DIK_D,     false },
		{ "F",     DIK_F,     false },
		{ "T",     DIK_T,     false },
		{ "X",     DIK_X,     false },
		{ "Z",     DIK_Z,     false },
		{ "V",     DIK_V,     false },
		{ "ALT_V", DIK_V,     true  },
		{ "SPACE", DIK_SPACE, false },
	};

	constexpr size_t SlotKeyCount = sizeof(SlotKeys) / sizeof(SlotKeys[0]);

	constexpr std::chrono::milliseconds MOVE_GOAL_RESEND_INTERVAL{ 50 };
	constexpr f32_t MOVE_GOAL_DEADZONE_RADIUS = 0.5f;
	constexpr f32_t MOVE_GOAL_RESEND_EPSILON = 0.25f;
	constexpr std::chrono::milliseconds SKILL_AIM_RESEND_INTERVAL{ 50 };
	constexpr f32_t SKILL_AIM_RESEND_EPSILON = 0.1f;

	bool_t Is_GroundTargetSkillAvailable(
		const Client::PLAYER_SKILL_DEFINITION& skill,
		const Client::HUD_PLAYER_STATE& player)
	{
		if (!player.isValid || player.isPreview || 0u == player.iCurrentHp ||
			player.eCharacterClass != skill.eCharacterClass ||
			LostArk::Shared::PLAYER_SKILL_KIND::ACTIVE != skill.eSkillKind ||
			LostArk::Shared::SKILL_TARGET_INTENT_KIND::GROUND_POINT !=
				skill.eTargetIntent)
		{
			return false;
		}

		const bool canStartImmediately =
			LostArk::Shared::PLAYER_ACTION_STATE::NONE == player.eAction;
		bool canStageAfterCombo = false;
		if (LostArk::Shared::PLAYER_ACTION_STATE::SKILL == player.eAction &&
			LostArk::Shared::INVALID_SKILL_ID != player.iCurrentSkillId &&
			player.iCurrentSkillId != skill.iSkillId)
		{
			const Client::PLAYER_SKILL_DEFINITION* running =
				Client::CPlayerSkillCatalog::Find_ById(player.iCurrentSkillId);
			canStageAfterCombo = nullptr != running &&
				LostArk::Shared::PLAYER_SKILL_KIND::COMBO == running->eSkillKind;
		}
		if (!canStartImmediately && !canStageAfterCombo)
			return false;

		/* The Server admits an explicit skill while a COMBO runs as a pending
		command and revalidates it when the full motion ends. Still reject a preview
		that the latest snapshot already proves cannot pay or use the skill; otherwise
		the confirm would close the overlay and silently fail at that boundary. */
		if (player.iCurrentResource < skill.iResourceCost ||
			player.iCurrentIdentity < skill.iIdentityCost ||
			(LostArk::Shared::PLAYER_STANCE_ID::NONE != skill.eRequiredStance &&
				player.eStance != skill.eRequiredStance))
		{
			return false;
		}

		for (const Client::HUD_SKILL_STATE& hudSkill : player.Skills)
		{
			if (hudSkill.iSkillId == skill.iSkillId)
				return hudSkill.Is_Ready(player.iServerTick);
		}
		return false;
	}

	bool_t Is_PlayerControlCaptured(const Client::HUD_PLAYER_STATE& player)
	{
		return player.isPatternBound ||
			LostArk::Shared::PLAYER_ACTION_STATE::GRABBED == player.eAction;
	}
}

void Client::CPlayerController::Set_LocalCharacter(const shared_ptr<CCharacter>& character)
{
	if (m_pLocalCharacter.lock() == character)
		return;

	m_pLocalCharacter = character;
	Cancel_GroundTargeting();
	m_iNextMoveSequence = 1;
	m_iNextActionSequence = 1;
	m_wasRightMouseDown = false;
	m_wasKeyDown.fill(false);
	m_iHeldSkillId = LostArk::Shared::INVALID_SKILL_ID;
	m_byHeldKeyCode = 0;
	m_iHeldBasicAttackSkillId = LostArk::Shared::INVALID_SKILL_ID;
	m_BasicAttackPressEdgeGate.Reset();
	m_BasicAttackResendGate.Reset();
	m_CaptureInputGate.Reset();
	m_LastMoveGoalSentAt = {};
	m_LastSentMoveGoal = {};
	m_LastSkillAimSentAt = {};
	m_LastSentSkillAim = {};
}

void Client::CPlayerController::Rebind_LocalCharacter(
	const shared_ptr<CCharacter>& character)
{
	if (m_pLocalCharacter.lock() == character)
		return;
	m_pLocalCharacter = character;
	Cancel_GroundTargeting();
	m_wasRightMouseDown = false;
	m_wasKeyDown.fill(false);
	m_iHeldSkillId = LostArk::Shared::INVALID_SKILL_ID;
	m_byHeldKeyCode = 0;
	m_iHeldBasicAttackSkillId = LostArk::Shared::INVALID_SKILL_ID;
	m_BasicAttackPressEdgeGate.Reset();
	m_BasicAttackResendGate.Reset();
	m_CaptureInputGate.Reset();
	m_LastMoveGoalSentAt = {};
	m_LastSentMoveGoal = {};
	m_LastSkillAimSentAt = {};
	m_LastSentSkillAim = {};
}

void Client::CPlayerController::Update(
	const bool_t gameplayCommandsEnabled, const bool_t debugPlacementEnabled)
{
#ifdef _DEBUG
	Update_DebugPlayerPlacement(debugPlacementEnabled && !gameplayCommandsEnabled);
#else
	(void)debugPlacementEnabled;
#endif
	/* One-shot, consumed here regardless of which branch below actually
	runs this frame -- see Suppress_MoveClickThisFrame's own comment. */
	const bool_t isMoveClickSuppressed = m_isMoveClickSuppressed;
	m_isMoveClickSuppressed = false;
	const bool_t isControlCaptured = Is_PlayerControlCaptured(
		CCombatHUDViewModel::Get().Get_Player());
	const bool_t isLeftMousePhysicallyDown =
		0 != (CGameInstance::Get().Get_DIMouseStateRaw(DIM::LB) & 0x80);
	const bool_t isRightMousePhysicallyDown =
		0 != (CGameInstance::Get().Get_DIMouseStateRaw(DIM::RB) & 0x80);
	for (std::size_t key = 0u; key < m_wasKeyDown.size(); ++key)
	{
		const bool_t down = 0 != (CGameInstance::Get().Get_DIKeyStateRaw(
			static_cast<uint8_t>(key)) & 0x80);
		m_CaptureInputGate.Observe(key, down, isControlCaptured);
		if (isControlCaptured)
			m_wasKeyDown[key] = down;
	}
	m_CaptureInputGate.Observe(CPLAYER_CAPTURE_INPUT_GATE::LEFT_MOUSE,
		isLeftMousePhysicallyDown, isControlCaptured);
	m_CaptureInputGate.Observe(CPLAYER_CAPTURE_INPUT_GATE::RIGHT_MOUSE,
		isRightMousePhysicallyDown, isControlCaptured);
	if (isControlCaptured)
	{
		Cancel_GroundTargeting();
		m_iHeldSkillId = LostArk::Shared::INVALID_SKILL_ID;
		m_byHeldKeyCode = 0u;
		m_iHeldBasicAttackSkillId = LostArk::Shared::INVALID_SKILL_ID;
		(void)m_BasicAttackPressEdgeGate.Should_Submit(
			isLeftMousePhysicallyDown, false, std::chrono::steady_clock::now());
		m_wasRightMouseDown = isRightMousePhysicallyDown;
		/* No command sink is called, including ReleaseSkill for a hold which
		was interrupted by the authoritative capture. */
		return;
	}

	//imgui로 mouse block된 상태가 아니고, Right Button
	const bool_t isRightMouseDown =
		!m_CaptureInputGate.Is_Blocked(CPLAYER_CAPTURE_INPUT_GATE::RIGHT_MOUSE) &&
		!CGameInstance::Get().IsMouseInputBlocked() &&
		0 != (CGameInstance::Get().Get_DIMouseState(DIM::RB) & 0x80);
	const bool_t isKeyboardBlocked =
		CGameInstance::Get().IsKeyboardInputBlocked();
	const bool_t useRawKeyboard =
		m_allowCapturedKeyboardInput &&
		GetForegroundWindow() == g_hWnd;
	/* The raw-keyboard passthrough path bypasses SetInputBlocked, so it must consult the
	runtime nickname field (CUIInputRouter) alongside ImGui's own text-input state itself. */
	const bool_t suppressKeyboard = useRawKeyboard ?
		(ImGui::GetIO().WantTextInput ||
			CUIInputRouter::Get().Is_TextInputActive()) : isKeyboardBlocked;
	//weak_ptr로 가지고 있는 character lock
	const shared_ptr<CCharacter> character =
		m_pLocalCharacter.lock();
	const shared_ptr<IPlayerCommandSink> commandSink =
		m_pCommandSink;

	if (m_GroundTargeting.Is_Active())
	{
		const auto& playerState = CCombatHUDViewModel::Get().Get_Player();
		const PLAYER_SKILL_DEFINITION* targetingDefinition =
			CPlayerSkillCatalog::Find_ById(m_GroundTargeting.Get_SkillId());
		if (!gameplayCommandsEnabled || nullptr == character ||
			nullptr == commandSink || nullptr == m_pGroundTargetPreview ||
			nullptr == targetingDefinition ||
			!Is_GroundTargetSkillAvailable(*targetingDefinition, playerState))
		{
			Cancel_GroundTargeting();
		}
		else
		{
			const shared_ptr<CTransform> transform = character->Get_Transform();
			if (nullptr == transform)
			{
				Cancel_GroundTargeting();
			}
			else
			{
				const vector_t position = transform->Get_State(STATE::POSITION);
				float3_t caster{};
				XMStoreFloat3(&caster, position);
				float3_t rawCursor{};
				if (Try_PickGroundPlane(caster.y, rawCursor) &&
					m_GroundTargeting.Update_Cursor(caster, rawCursor))
				{
					float3_t sampled{};
					const float3_t& clamped =
						m_GroundTargeting.Get_TargetPosition();
					if (character->Try_SampleTargetGround(
							clamped.x, clamped.z, sampled))
					{
						(void)m_GroundTargeting.Apply_WalkableSample(sampled);
					}
				}
				else
				{
					m_GroundTargeting.Invalidate_Cursor(caster);
				}
				m_pGroundTargetPreview->Set_State(
					caster, m_GroundTargeting.Get_TargetPosition(),
					m_GroundTargeting.Can_Confirm());

				const bool_t cancelEdge = isRightMouseDown &&
					isRightMousePhysicallyDown &&
					!m_wasTargetingRightMouseDown;
				const bool_t confirmEdge =
					!CGameInstance::Get().IsMouseInputBlocked() &&
					0 != (CGameInstance::Get().Get_DIMouseState(DIM::LB) & 0x80) &&
					isLeftMousePhysicallyDown &&
					!m_wasTargetingLeftMouseDown;
				m_wasTargetingLeftMouseDown = isLeftMousePhysicallyDown;
				m_wasTargetingRightMouseDown = isRightMousePhysicallyDown;
				if (cancelEdge)
				{
					Cancel_GroundTargeting();
				}
				else if (confirmEdge && m_GroundTargeting.Can_Confirm())
				{
					const float3_t target =
						m_GroundTargeting.Get_TargetPosition();
					const auto requestGroundTargetSkill = [&]()
					{
						return commandSink->Request_UseGroundTargetSkill(
							m_iNextActionSequence,
							m_GroundTargeting.Get_SkillId(),
							target.x, target.z);
					};
					if (Try_Commit_GroundTargetConfirmation(
							m_BasicAttackResendGate, requestGroundTargetSkill))
					{
						++m_iNextActionSequence;
						if (0u == m_iNextActionSequence)
							m_iNextActionSequence = 1u;
						Cancel_GroundTargeting();
					}
				}
			}
		}

		LostArk::Shared::SKILL_ID ignoredSkill =
			LostArk::Shared::INVALID_SKILL_ID;
		LostArk::Shared::SKILL_ID ignoredRelease =
			LostArk::Shared::INVALID_SKILL_ID;
		Poll_SkillSlots(true, useRawKeyboard, character,
			ignoredSkill, ignoredRelease);
		(void)Poll_EstherSlot(true, useRawKeyboard);
		(void)Poll_InteractKey(true, useRawKeyboard);
		m_wasRightMouseDown = isRightMouseDown;
		return;
	}

	if (gameplayCommandsEnabled && isRightMouseDown &&
		!isMoveClickSuppressed &&
		nullptr != character &&
		nullptr != commandSink)
	{
		const shared_ptr<CTransform> transform =
			character->Get_Transform();

		if (nullptr != transform)
		{
			const vector_t position =
				transform->Get_State(STATE::POSITION);
			const f32_t groundY = XMVectorGetY(position);

			float3_t goal{};

			if (Try_PickGroundPlane(groundY, goal) &&
				Should_SendMoveGoal(
					m_wasRightMouseDown,
					XMVectorGetX(position),
					XMVectorGetZ(position),
					goal))
			{
				Request_MoveToPoint(goal);
			}
		}
	}

	LostArk::Shared::SKILL_ID requestedSkillId =
		LostArk::Shared::INVALID_SKILL_ID;
	LostArk::Shared::SKILL_ID releasedSkillId =
		LostArk::Shared::INVALID_SKILL_ID;
	Poll_SkillSlots(
		suppressKeyboard || !gameplayCommandsEnabled,
		useRawKeyboard,
		character,
		requestedSkillId,
		releasedSkillId);

	if (LostArk::Shared::INVALID_SKILL_ID != releasedSkillId &&
		nullptr != commandSink &&
		commandSink->Request_ReleaseSkill(
			m_iNextActionSequence, releasedSkillId))
	{
		++m_iNextActionSequence;
		if (0 == m_iNextActionSequence)
			m_iNextActionSequence = 1;
	}

	if (LostArk::Shared::INVALID_SKILL_ID != requestedSkillId &&
		nullptr != character && nullptr != commandSink)
	{
		const shared_ptr<CTransform> transform = character->Get_Transform();
		if (nullptr != transform)
		{
			const PLAYER_SKILL_DEFINITION* requestedDefinition =
				CPlayerSkillCatalog::Find_ById(requestedSkillId);
			if (nullptr != requestedDefinition &&
				LostArk::Shared::SKILL_TARGET_INTENT_KIND::GROUND_POINT ==
					requestedDefinition->eTargetIntent)
			{
				const auto& playerState =
					CCombatHUDViewModel::Get().Get_Player();
				if (Is_GroundTargetSkillAvailable(
						*requestedDefinition, playerState) &&
					nullptr != m_pGroundTargetPreview &&
					m_GroundTargeting.Begin(
						requestedSkillId,
						requestedDefinition->fTargetMaximumRange) &&
					m_pGroundTargetPreview->Begin(*requestedDefinition))
				{
					m_wasTargetingLeftMouseDown =
						isLeftMousePhysicallyDown;
					m_wasTargetingRightMouseDown =
						isRightMousePhysicallyDown;
				}
				else
				{
					Cancel_GroundTargeting();
				}
				m_wasRightMouseDown = isRightMouseDown;
				return;
			}
			const vector_t position = transform->Get_State(STATE::POSITION);
			float3_t aim{};
			const f32_t groundY = XMVectorGetY(position);
			if (!Try_PickGroundPlane(groundY, aim))
			{
				const vector_t look = XMVector3Normalize(
					transform->Get_State(STATE::LOOK));
				aim.x = XMVectorGetX(position) + XMVectorGetX(look) * 5.f;
				aim.y = groundY;
				aim.z = XMVectorGetZ(position) + XMVectorGetZ(look) * 5.f;
			}
			if (commandSink->Request_UseSkill(
				m_iNextActionSequence,
				requestedSkillId,
				aim.x,
				aim.z))
			{
				const bool_t requestedBasicAttack =
					requestedSkillId == m_iHeldBasicAttackSkillId;
				if (requestedBasicAttack)
				{
					m_BasicAttackPressEdgeGate.Commit_Submission(
						std::chrono::steady_clock::now());
				}
				/* Poll_BasicAttack has already observed this frame.  Suppress only
				when an explicit skill won while LMB is physically still held. */
				const bool_t isBasicAttackPhysicallyHeld =
					0 != (CGameInstance::Get().Get_DIMouseStateRaw(DIM::LB) & 0x80);
				if (!requestedBasicAttack && isBasicAttackPhysicallyHeld)
					m_BasicAttackResendGate.Suppress_UntilRelease();
				++m_iNextActionSequence;
				if (0 == m_iNextActionSequence)
					m_iNextActionSequence = 1;
				if (requestedSkillId == m_iHeldSkillId)
				{
					m_LastSkillAimSentAt = std::chrono::steady_clock::now();
					m_LastSentSkillAim = aim;
				}
			}
		}
	}

	if (LostArk::Shared::INVALID_SKILL_ID != m_iHeldSkillId &&
		gameplayCommandsEnabled &&
		nullptr != character && nullptr != commandSink)
	{
		const auto now = std::chrono::steady_clock::now();
		const shared_ptr<CTransform> transform = character->Get_Transform();
		float3_t aim{};
		if (now - m_LastSkillAimSentAt >= SKILL_AIM_RESEND_INTERVAL &&
			nullptr != transform &&
			Try_PickGroundPlane(
				XMVectorGetY(transform->Get_State(STATE::POSITION)), aim))
		{
			const f32_t driftX = aim.x - m_LastSentSkillAim.x;
			const f32_t driftZ = aim.z - m_LastSentSkillAim.z;
			if (driftX * driftX + driftZ * driftZ >=
				SKILL_AIM_RESEND_EPSILON * SKILL_AIM_RESEND_EPSILON &&
				commandSink->Request_SkillAim(
					m_iNextActionSequence,
					m_iHeldSkillId,
					aim.x,
					aim.z))
			{
				m_LastSkillAimSentAt = now;
				m_LastSentSkillAim = aim;
				++m_iNextActionSequence;
				if (0 == m_iNextActionSequence)
					m_iNextActionSequence = 1;
			}
		}
	}

	if (Poll_InteractKey(
		suppressKeyboard || !gameplayCommandsEnabled, useRawKeyboard) &&
		nullptr != commandSink)
	{
		/* Only the box the Server is offering right now can be answered; with
		   no offer standing the press is simply nothing. */
		const std::string& offered =
			CCombatHUDViewModel::Get().Get_InteractPromptTriggerId();
		if (!offered.empty() &&
			commandSink->Request_InteractTrigger(m_iNextActionSequence, offered))
		{
			++m_iNextActionSequence;
			if (0 == m_iNextActionSequence)
				m_iNextActionSequence = 1;
		}
	}

	const std::uint8_t estherSlot = Poll_EstherSlot(
		suppressKeyboard || !gameplayCommandsEnabled, useRawKeyboard);
	if (0 != estherSlot && nullptr != character && nullptr != commandSink)
	{
		const shared_ptr<CTransform> transform = character->Get_Transform();
		if (nullptr != transform)
		{
			const vector_t position = transform->Get_State(STATE::POSITION);
			float3_t aim{};
			const f32_t groundY = XMVectorGetY(position);
			if (!Try_PickGroundPlane(groundY, aim))
			{
				const vector_t look = XMVector3Normalize(
					transform->Get_State(STATE::LOOK));
				aim.x = XMVectorGetX(position) + XMVectorGetX(look) * 5.f;
				aim.y = groundY;
				aim.z = XMVectorGetZ(position) + XMVectorGetZ(look) * 5.f;
			}
			if (commandSink->Request_EstherSkill(
				m_iNextActionSequence,
				estherSlot,
				aim.x,
				aim.z))
			{
				++m_iNextActionSequence;
				if (0 == m_iNextActionSequence)
					m_iNextActionSequence = 1;
			}
		}
	}

	m_wasRightMouseDown = isRightMouseDown;
}

void Client::CPlayerController::Poll_SkillSlots(
	const bool_t isKeyboardBlocked,
	const bool_t useRawKeyboard,
	const shared_ptr<CCharacter>& character,
	LostArk::Shared::SKILL_ID& outSkillId,
	LostArk::Shared::SKILL_ID& outReleaseSkillId)
{
	const auto readKeyState = [this, useRawKeyboard](const uint8_t keyCode)
	{
		if (m_CaptureInputGate.Is_Blocked(keyCode) &&
			keyCode != DIK_LMENU && keyCode != DIK_RMENU &&
			keyCode != DIK_LCONTROL && keyCode != DIK_RCONTROL)
			return static_cast<int8_t>(0);
		return useRawKeyboard ?
			CGameInstance::Get().Get_DIKeyStateRaw(keyCode) :
			CGameInstance::Get().Get_DIKeyState(keyCode);
	};
	const bool_t isAltDown =
		(0 != (readKeyState(DIK_LMENU) & 0x80) ||
			0 != (readKeyState(DIK_RMENU) & 0x80));
	/* A Ctrl frame belongs to the Esther slots. Ruling the whole table out here
	keeps Warlord's plain Z/X stance keys from firing under Ctrl+Z / Ctrl+X. */
	const bool_t isCtrlDown =
		(0 != (readKeyState(DIK_LCONTROL) & 0x80) ||
			0 != (readKeyState(DIK_RCONTROL) & 0x80));

	const CHARACTER_SPEC* pSpec =
		nullptr != character ? character->Get_Spec() : nullptr;
	const LostArk::Shared::PLAYER_STANCE_ID stance =
		CCombatHUDViewModel::Get().Get_Player().eStance;
	/* While the server holds the player in KNOCKDOWN a slot press resolves to
	its STANDUP skill, mirroring the admission rule the server applies. */
	const bool_t isKnockedDown =
		LostArk::Shared::PLAYER_ACTION_STATE::KNOCKDOWN ==
		CCombatHUDViewModel::Get().Get_Player().eAction;

	/* Two slots share one key (V and ALT_V), so every slot compares against the
	state this frame started with and the new state is committed afterwards.
	Writing inside the decision loop let the plain V entry consume the press
	before the ALT_V entry could see it, which made Alt+V do nothing at all. */
	bool_t isDown[SlotKeyCount]{};
	for (size_t index = 0; index < SlotKeyCount; ++index)
	{
		isDown[index] =
			0 != (readKeyState(SlotKeys[index].byKeyCode) & 0x80);
	}

	for (size_t index = 0; index < SlotKeyCount; ++index)
	{
		const SLOT_KEY& slot = SlotKeys[index];
		if (isKeyboardBlocked || isCtrlDown ||
			slot.requiresAlt != isAltDown ||
			!isDown[index] || m_wasKeyDown[slot.byKeyCode] ||
			nullptr == pSpec ||
			LostArk::Shared::INVALID_SKILL_ID != outSkillId)
		{
			continue;
		}

		/* An unbound slot is normal: a class simply has no skill there. */
		const PLAYER_SKILL_DEFINITION* pSkill = CPlayerSkillCatalog::Find_BySlot(
			pSpec->eCharacterClass, slot.pInputSlot, stance, isKnockedDown);
		if (nullptr != pSkill)
		{
			outSkillId = pSkill->iSkillId;
			if (LostArk::Shared::PLAYER_SKILL_KIND::HOLD == pSkill->eSkillKind)
			{
				m_iHeldSkillId = pSkill->iSkillId;
				m_byHeldKeyCode = slot.byKeyCode;
			}
		}
	}

	if (LostArk::Shared::INVALID_SKILL_ID != m_iHeldSkillId)
	{
		bool_t stillDown = false;
		for (size_t index = 0; index < SlotKeyCount; ++index)
		{
			if (SlotKeys[index].byKeyCode == m_byHeldKeyCode && isDown[index])
				stillDown = true;
		}
		if (isKeyboardBlocked || !stillDown)
		{
			outReleaseSkillId = m_iHeldSkillId;
			m_iHeldSkillId = LostArk::Shared::INVALID_SKILL_ID;
			m_byHeldKeyCode = 0;
		}
	}

	/* Committed for every slot, including ones the modifier ruled out, so
	releasing Alt while V is still held does not read as a fresh press. */
	for (size_t index = 0; index < SlotKeyCount; ++index)
		m_wasKeyDown[SlotKeys[index].byKeyCode] = isDown[index];

	Poll_BasicAttack(pSpec, stance, outSkillId, isKeyboardBlocked);
}

void Client::CPlayerController::Poll_BasicAttack(
	const CHARACTER_SPEC* pSpec,
	const LostArk::Shared::PLAYER_STANCE_ID stance,
	LostArk::Shared::SKILL_ID& outSkillId,
	const bool_t commandSuppressed)
{
	const std::chrono::steady_clock::time_point now =
		std::chrono::steady_clock::now();
	const bool_t isPhysicallyDown =
		0 != (CGameInstance::Get().Get_DIMouseStateRaw(DIM::LB) & 0x80);
	const bool_t isGameplayDown =
		0 != (CGameInstance::Get().Get_DIMouseState(DIM::LB) & 0x80);
	const bool_t resendSuppressed =
		m_BasicAttackResendGate.Observe_Button(isPhysicallyDown);
	if (!isPhysicallyDown)
	{
		(void)m_BasicAttackPressEdgeGate.Should_Submit(false, false, now);
		m_iHeldBasicAttackSkillId = LostArk::Shared::INVALID_SKILL_ID;
		return;
	}

	const PLAYER_SKILL_DEFINITION* pSkill = nullptr;
	if (nullptr != pSpec)
	{
		pSkill = CPlayerSkillCatalog::Find_BySlot(
			pSpec->eCharacterClass, "LMB", stance);
	}
	const bool_t commandEligible =
		!m_CaptureInputGate.Is_Blocked(CPLAYER_CAPTURE_INPUT_GATE::LEFT_MOUSE) &&
		isGameplayDown && !commandSuppressed && !resendSuppressed && nullptr != pSkill &&
		LostArk::Shared::INVALID_SKILL_ID == outSkillId;
	if (!m_BasicAttackPressEdgeGate.Should_Submit(
			isPhysicallyDown, commandEligible, now))
	{
		return;
	}

	m_iHeldBasicAttackSkillId = pSkill->iSkillId;
	outSkillId = pSkill->iSkillId;
}

std::uint8_t Client::CPlayerController::Poll_EstherSlot(
	const bool_t isKeyboardBlocked,
	const bool_t useRawKeyboard)
{
	const auto readKeyState = [this, useRawKeyboard](const uint8_t keyCode)
	{
		if (m_CaptureInputGate.Is_Blocked(keyCode) &&
			keyCode != DIK_LMENU && keyCode != DIK_RMENU &&
			keyCode != DIK_LCONTROL && keyCode != DIK_RCONTROL)
			return static_cast<int8_t>(0);
		return useRawKeyboard ?
			CGameInstance::Get().Get_DIKeyStateRaw(keyCode) :
			CGameInstance::Get().Get_DIKeyState(keyCode);
	};
	const bool_t isCtrlDown =
		(0 != (readKeyState(DIK_LCONTROL) & 0x80) ||
			0 != (readKeyState(DIK_RCONTROL) & 0x80));
	constexpr uint8_t EstherKeys[3] = { DIK_Z, DIK_X, DIK_C };

	std::uint8_t pressedSlot = 0;
	for (size_t index = 0; index < 3; ++index)
	{
		const bool_t isDown =
			0 != (readKeyState(EstherKeys[index]) & 0x80);
		if (!isKeyboardBlocked && isCtrlDown && isDown &&
			!m_wasEstherKeyDown[index] && 0 == pressedSlot)
		{
			pressedSlot = static_cast<std::uint8_t>(index + 1);
		}
		/* Committed whether or not Ctrl was down, so pressing Ctrl while Z is
		already held does not read as a fresh Esther press. */
		m_wasEstherKeyDown[index] = isDown;
	}
	return pressedSlot;
}

bool_t Client::CPlayerController::Poll_InteractKey(
	const bool_t isKeyboardBlocked,
	const bool_t useRawKeyboard)
{
	const int8_t state = m_CaptureInputGate.Is_Blocked(DIK_G) ?
		static_cast<int8_t>(0) :
		(useRawKeyboard ?
			CGameInstance::Get().Get_DIKeyStateRaw(DIK_G) :
			CGameInstance::Get().Get_DIKeyState(DIK_G));
	const bool_t isDown = 0 != (state & 0x80);
	const bool_t pressed =
		!isKeyboardBlocked && isDown && !m_wasInteractKeyDown;
	/* Committed regardless of the block so releasing under a block cannot
	   leave a stale edge waiting to fire on the next unblocked frame. */
	m_wasInteractKeyDown = isDown;
	return pressed;
}

void Client::CPlayerController::Set_CommandSink(
	const shared_ptr<IPlayerCommandSink>& commandSink)
{
#ifdef _DEBUG
	if (m_pCommandSink != commandSink)
	{
		Cancel_DebugPlayerPlacement();
		m_pendingDebugPlacementSequence = 0u;
		m_debugPlacementStatus.clear();
	}
#endif
	m_pCommandSink = commandSink;
}

bool_t Client::CPlayerController::Request_Revive()
{
	if (nullptr == m_pCommandSink)
		return false;
	const bool_t captured = Is_PlayerControlCaptured(
		CCombatHUDViewModel::Get().Get_Player());
	if (!Try_SubmitUncapturedPlayerCommand(captured, [this]()
		{ return m_pCommandSink->Request_RevivePlayer(m_iNextActionSequence); }))
		return false;
	++m_iNextActionSequence;
	if (0u == m_iNextActionSequence)
		m_iNextActionSequence = 1u;
	return true;
}

#ifdef _DEBUG
bool_t Client::CPlayerController::Request_DebugKillSelf()
{
	if (nullptr == m_pCommandSink)
		return false;
	const bool_t captured = Is_PlayerControlCaptured(
		CCombatHUDViewModel::Get().Get_Player());
	if (!Try_SubmitUncapturedPlayerCommand(captured, [this]()
		{ return m_pCommandSink->Request_DebugKillSelf(m_iNextActionSequence); }))
		return false;
	++m_iNextActionSequence;
	if (0u == m_iNextActionSequence)
		m_iNextActionSequence = 1u;
	return true;
}
#endif

#ifdef _DEBUG
bool_t Client::CPlayerController::Begin_DebugPlayerPlacement(
	const LostArk::Shared::WORLD_ID worldId)
{
	using LostArk::Shared::WORLD_ID;
	if (!m_debugPlacementEnabled || Is_DebugPlayerPlacementPending() ||
		m_pLocalCharacter.expired() || nullptr == m_pCommandSink ||
		(WORLD_ID::VALTAN_ARENA != worldId && WORLD_ID::KAKULSAYDON_ARENA != worldId))
	{
		if (!Is_DebugPlayerPlacementPending())
			m_debugPlacementStatus = "Move Player requires a live player and F6 free camera.";
		return false;
	}
	Cancel_GroundTargeting();
	m_debugPlacementWorld = worldId;
	m_debugPlacementArmed = true;
	// The button's own press must be released before the viewport can consume a click.
	m_wasDebugPlacementLeftDown = true;
	m_debugPlacementStatus = "Click visible ground once. Esc or right-click cancels.";
	return true;
}

void Client::CPlayerController::Cancel_DebugPlayerPlacement()
{
	if (m_debugPlacementArmed)
		m_debugPlacementStatus = "Player placement cancelled.";
	m_debugPlacementArmed = false;
}

void Client::CPlayerController::Update_DebugPlayerPlacement(const bool_t enabled)
{
	using namespace LostArk::Shared;
	m_debugPlacementEnabled = enabled;
	const bool_t leftDown =
		0 != (CGameInstance::Get().Get_DIMouseStateRaw(DIM::LB) & 0x80);
	const bool_t leftPressed = leftDown && !m_wasDebugPlacementLeftDown;
	m_wasDebugPlacementLeftDown = leftDown;

	if (nullptr != m_pCommandSink)
	{
		S2C_DEBUG_TELEPORT_TO_POSITION_RESULT result{};
		while (m_pCommandSink->Consume_DebugTeleportResult(result))
		{
			if (0u == m_pendingDebugPlacementSequence ||
				result.iRequestSequence != m_pendingDebugPlacementSequence ||
				result.eWorldId != m_debugPlacementWorld)
				continue;
			m_pendingDebugPlacementSequence = 0u;
			m_debugPlacementReplyDelayed = false;
			switch (result.eResult)
			{
			case DEBUG_TELEPORT_RESULT::ACCEPTED:
			{
				char status[160]{};
				sprintf_s(status, "Server moved player to (%.2f, %.2f, %.2f).",
					result.fPositionX, result.fPositionY, result.fPositionZ);
				m_debugPlacementStatus = status;
				break;
			}
			case DEBUG_TELEPORT_RESULT::REJECTED_DISABLED:
				m_debugPlacementStatus = "Move Player requires a Debug Server."; break;
			case DEBUG_TELEPORT_RESULT::REJECTED_SESSION:
				m_debugPlacementStatus = "Server rejected placement: player session is unavailable."; break;
			case DEBUG_TELEPORT_RESULT::REJECTED_WRONG_WORLD:
				m_debugPlacementStatus = "Server rejected placement: the current world changed."; break;
			case DEBUG_TELEPORT_RESULT::REJECTED_STALE_SEQUENCE:
				m_debugPlacementStatus = "Server rejected an old or duplicate placement request."; break;
			case DEBUG_TELEPORT_RESULT::REJECTED_PLAYER_STATE:
				m_debugPlacementStatus = "Server rejected placement: player is dead, falling or captured."; break;
			case DEBUG_TELEPORT_RESULT::REJECTED_INVALID_POSITION:
				m_debugPlacementStatus = "Server rejected placement: invalid picked position."; break;
			case DEBUG_TELEPORT_RESULT::REJECTED_NAVIGATION:
				m_debugPlacementStatus = "Server rejected placement: choose walkable ground."; break;
			case DEBUG_TELEPORT_RESULT::REJECTED_HEIGHT:
				m_debugPlacementStatus = "Server rejected placement: picked surface is not the navigation floor."; break;
			case DEBUG_TELEPORT_RESULT::REJECTED_COLLISION:
				m_debugPlacementStatus = "Server rejected placement: player would overlap a blocker."; break;
			default:
				m_debugPlacementStatus = "Server returned an unsupported placement result."; break;
			}
		}
	}
	if (Is_DebugPlayerPlacementPending() && !m_debugPlacementReplyDelayed &&
		std::chrono::steady_clock::now() - m_debugPlacementSentAt > std::chrono::seconds(5))
	{
		m_debugPlacementReplyDelayed = true;
		m_debugPlacementStatus = "Server reply delayed; placement is not yet confirmed.";
	}
	if (m_pLocalCharacter.expired() || nullptr == m_pCommandSink)
	{
		if (m_debugPlacementArmed || Is_DebugPlayerPlacementPending())
			m_debugPlacementStatus = "Player placement ended: player connection is unavailable.";
		m_debugPlacementArmed = false;
		m_pendingDebugPlacementSequence = 0u;
		return;
	}
	if (!enabled || Is_PlayerControlCaptured(CCombatHUDViewModel::Get().Get_Player()))
	{
		Cancel_DebugPlayerPlacement();
		return;
	}
	if (!m_debugPlacementArmed || GetForegroundWindow() != g_hWnd)
		return;
	if (ImGui::GetIO().WantTextInput || CUIInputRouter::Get().Is_TextInputActive())
		return;
	if (CGameInstance::Get().Get_DIKeyPressedRaw(DIK_ESCAPE) ||
		0 != (CGameInstance::Get().Get_DIMouseStateRaw(DIM::RB) & 0x80))
	{
		Cancel_DebugPlayerPlacement();
		return;
	}
	if (!leftPressed || ImGui::GetIO().WantCaptureMouse ||
		CUIInputRouter::Get().Is_MouseClaimedThisFrame() ||
		CGameInstance::Get().IsMouseInputBlocked() ||
		0 == (CGameInstance::Get().Get_DIMouseState(DIM::LB) & 0x80))
		return;

	CGameInstance::Get().SetMouseButtonBlocked(DIM::LB, true);
	CUIInputRouter::Get().Claim_Mouse_This_Frame();
	m_BasicAttackResendGate.Suppress_UntilRelease();
	float4_t picked{};
	if (!CGameInstance::Get().Picking(picked) || !std::isfinite(picked.x) ||
		!std::isfinite(picked.y) || !std::isfinite(picked.z))
	{
		m_debugPlacementStatus = "No surface under cursor. Click visible ground or cancel.";
		return;
	}
	// Only submit intent. The server owns floor projection, reset and the snapshot.
	if (!m_pCommandSink->Request_DebugTeleportToPosition(
		m_nextDebugPlacementSequence, picked.x, picked.y, picked.z))
	{
		m_debugPlacementStatus = "Could not send placement request. Click again to retry.";
		return;
	}
	m_pendingDebugPlacementSequence = m_nextDebugPlacementSequence;
	if (0u == ++m_nextDebugPlacementSequence)
		m_nextDebugPlacementSequence = 1u;
	m_debugPlacementSentAt = std::chrono::steady_clock::now();
	m_debugPlacementReplyDelayed = false;
	m_debugPlacementArmed = false;
	m_debugPlacementStatus = "Waiting for Server placement approval...";
}
#endif

bool_t Client::CPlayerController::Initialize_TargetingPreview(
	const uint32_t levelIndex)
{
	if (nullptr != m_pGroundTargetPreview)
		return true;
	if (levelIndex >= ETOUI(LEVEL::END))
		return false;
	CGameObject::GAMEOBJECT_DESC desc{};
	shared_ptr<CGameObject> object;
	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		ETOUI(LEVEL::STATIC),
		CSkillGroundTargetPreview::PROTOTYPE_TAG,
		levelIndex,
		TEXT("Layer_SkillGroundTargetPreview"),
		&desc,
		&object)))
	{
		return false;
	}
	m_pGroundTargetPreview =
		dynamic_pointer_cast<CSkillGroundTargetPreview>(object);
	if (nullptr == m_pGroundTargetPreview)
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			levelIndex, TEXT("Layer_SkillGroundTargetPreview"), object);
		return false;
	}
	return true;
}

bool_t Client::CPlayerController::Initialize_ClickMoveEffect(
	const uint32_t levelIndex)
{
	if (nullptr != m_pClickMoveEffect)
		return true;
	if (levelIndex >= ETOUI(LEVEL::END))
		return false;
	CGameObject::GAMEOBJECT_DESC desc{};
	shared_ptr<CGameObject> object;
	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		ETOUI(LEVEL::STATIC),
		CClickMoveEffect::PROTOTYPE_TAG,
		levelIndex,
		TEXT("Layer_ClickMoveEffect"),
		&desc,
		&object)))
	{
		return false;
	}
	shared_ptr<CClickMoveEffect> clickMoveEffect =
		dynamic_pointer_cast<CClickMoveEffect>(object);
	if (nullptr == clickMoveEffect ||
		!clickMoveEffect->Initialize_Textures())
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			levelIndex, TEXT("Layer_ClickMoveEffect"), object);
		return false;
	}
	m_pClickMoveEffect = std::move(clickMoveEffect);
	return true;
}

void Client::CPlayerController::Cancel_GroundTargeting()
{
	m_GroundTargeting.Cancel();
	if (nullptr != m_pGroundTargetPreview)
		m_pGroundTargetPreview->Clear();
	m_wasTargetingLeftMouseDown = false;
	m_wasTargetingRightMouseDown = false;
}

bool_t Client::CPlayerController::Should_SendMoveGoal(
	const bool_t wasRightMouseDown,
	const f32_t characterX,
	const f32_t characterZ,
	const float3_t& goal) const
{
	if (!wasRightMouseDown)
		return true;

	if (std::chrono::steady_clock::now() - m_LastMoveGoalSentAt <
		MOVE_GOAL_RESEND_INTERVAL)
	{
		return false;
	}

	const f32_t reachX = goal.x - characterX;
	const f32_t reachZ = goal.z - characterZ;
	if (reachX * reachX + reachZ * reachZ <
		MOVE_GOAL_DEADZONE_RADIUS * MOVE_GOAL_DEADZONE_RADIUS)
	{
		return false;
	}

	const f32_t driftX = goal.x - m_LastSentMoveGoal.x;
	const f32_t driftZ = goal.z - m_LastSentMoveGoal.z;
	return driftX * driftX + driftZ * driftZ >=
		MOVE_GOAL_RESEND_EPSILON * MOVE_GOAL_RESEND_EPSILON;
}

bool_t Client::CPlayerController::Try_PickWorldRay(
	vector_t& outOrigin, vector_t& outDirection)
{
	//현재 마우스 위치에서 world ray를 만든다 (near/far 언프로젝트).
	::POINT cursor{};

	if (!GetCursorPos(&cursor) ||
		!ScreenToClient(g_hWnd, &cursor))
	{
		return false;
	}

	const float2_t viewport =
		CGameInstance::Get().Get_ViewportSize();

	if (viewport.x <= 0.f || viewport.y <= 0.f ||
		cursor.x < 0 || cursor.y < 0 ||
		cursor.x >= static_cast<LONG>(viewport.x) ||
		cursor.y >= static_cast<LONG>(viewport.y))
	{
		return false;
	}

	const matrix_t view = XMLoadFloat4x4(
		CGameInstance::Get().Get_Transform(D3DTS::VIEW));
	const matrix_t projection = XMLoadFloat4x4(
		CGameInstance::Get().Get_Transform(D3DTS::PROJ));

	const vector_t nearPoint = XMVector3Unproject(
		XMVectorSet(
			static_cast<f32_t>(cursor.x),
			static_cast<f32_t>(cursor.y),
			0.f,
			1.f),
		0.f,
		0.f,
		viewport.x,
		viewport.y,
		0.f,
		1.f,
		projection,
		view,
		XMMatrixIdentity());

	const vector_t farPoint = XMVector3Unproject(
		XMVectorSet(
			static_cast<f32_t>(cursor.x),
			static_cast<f32_t>(cursor.y),
			1.f,
			1.f),
		0.f,
		0.f,
		viewport.x,
		viewport.y,
		0.f,
		1.f,
		projection,
		view,
		XMMatrixIdentity());

	outOrigin = nearPoint;
	outDirection = farPoint - nearPoint;
	return true;
}

bool_t Client::CPlayerController::Try_PickGroundPlane(
	f32_t groundY, float3_t & outPosition)
{
	//추후 navigation 사용해서 피킹과 스킬 사용에 따른 collider와도 연동을 시킨다.
	vector_t nearPoint{}, direction{};
	if (!Try_PickWorldRay(nearPoint, direction))
		return false;

	const f32_t directionY = XMVectorGetY(direction);

	if (std::abs(directionY) < 0.00001f)
		return false;

	const f32_t distance =
		(groundY - XMVectorGetY(nearPoint)) /
		directionY;

	if (distance < 0.f)
		return false;

	XMStoreFloat3(
		&outPosition,
		nearPoint + direction * distance);

	outPosition.y = groundY;

	return
		std::isfinite(outPosition.x) &&
		std::isfinite(outPosition.y) &&
		std::isfinite(outPosition.z);
}

bool_t Client::CPlayerController::Request_MoveToPoint(const float3_t& goal)
{
	if (Is_PlayerControlCaptured(
			CCombatHUDViewModel::Get().Get_Player()))
	{
		return false;
	}
	const shared_ptr<IPlayerCommandSink> commandSink = m_pCommandSink;
	if (nullptr == commandSink)
		return false;
	if (!commandSink->Request_MoveGoal(m_iNextMoveSequence, goal.x, goal.z))
		return false;

	m_LastMoveGoalSentAt = std::chrono::steady_clock::now();
	m_LastSentMoveGoal = goal;
	if (nullptr != m_pClickMoveEffect)
		m_pClickMoveEffect->Play(goal);
	m_BasicAttackResendGate.Suppress_UntilRelease();
	++m_iNextMoveSequence;
	if (0 == m_iNextMoveSequence)
		m_iNextMoveSequence = 1;
	return true;
}
