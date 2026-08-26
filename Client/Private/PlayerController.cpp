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

#include <cmath>

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
	m_LastMoveGoalSentAt = {};
	m_LastSentMoveGoal = {};
	m_LastSkillAimSentAt = {};
	m_LastSentSkillAim = {};
}

void Client::CPlayerController::Update(const bool_t gameplayCommandsEnabled)
{
	//imgui로 mouse block된 상태가 아니고, Right Button
	const bool_t isRightMouseDown =
		!CGameInstance::Get().IsMouseInputBlocked() &&
		0 != (CGameInstance::Get().Get_DIMouseState(DIM::RB) & 0x80);
	const bool_t isKeyboardBlocked =
		CGameInstance::Get().IsKeyboardInputBlocked();
	const bool_t useRawKeyboard =
		m_allowCapturedKeyboardInput &&
		GetForegroundWindow() == g_hWnd;
	const bool_t suppressKeyboard = useRawKeyboard ?
		ImGui::GetIO().WantTextInput : isKeyboardBlocked;
	//weak_ptr로 가지고 있는 character lock
	const shared_ptr<CCharacter> character =
		m_pLocalCharacter.lock();
	const shared_ptr<IPlayerCommandSink> commandSink =
		m_pCommandSink;
	const bool_t isLeftMousePhysicallyDown =
		0 != (CGameInstance::Get().Get_DIMouseStateRaw(DIM::LB) & 0x80);
	const bool_t isRightMousePhysicallyDown =
		0 != (CGameInstance::Get().Get_DIMouseStateRaw(DIM::RB) & 0x80);

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

				const bool_t cancelEdge = isRightMousePhysicallyDown &&
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
		m_wasRightMouseDown = isRightMouseDown;
		return;
	}

	if (gameplayCommandsEnabled && isRightMouseDown &&
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
					goal) &&
				commandSink->Request_MoveGoal(
					m_iNextMoveSequence,
					goal.x,
					goal.z))
			{
				m_LastMoveGoalSentAt = std::chrono::steady_clock::now();
				m_LastSentMoveGoal = goal;
				if (nullptr != m_pClickMoveEffect)
					m_pClickMoveEffect->Play(goal);
				/* Poll_BasicAttack runs later this frame.  Its current physical
				state either keeps this suppression (held) or clears it (up). */
				m_BasicAttackResendGate.Suppress_UntilRelease();
				++m_iNextMoveSequence;
				if (0 == m_iNextMoveSequence)
					m_iNextMoveSequence = 1;
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
					m_BasicAttackPressEdgeGate.Commit_Submission();
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
	const auto readKeyState = [useRawKeyboard](const uint8_t keyCode)
	{
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
	const bool_t isPhysicallyDown =
		0 != (CGameInstance::Get().Get_DIMouseStateRaw(DIM::LB) & 0x80);
	const bool_t isGameplayDown =
		0 != (CGameInstance::Get().Get_DIMouseState(DIM::LB) & 0x80);
	const bool_t resendSuppressed =
		m_BasicAttackResendGate.Observe_Button(isPhysicallyDown);
	if (!isPhysicallyDown)
	{
		(void)m_BasicAttackPressEdgeGate.Should_Submit(false, false);
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
		isGameplayDown && !commandSuppressed && !resendSuppressed && nullptr != pSkill &&
		LostArk::Shared::INVALID_SKILL_ID == outSkillId;
	if (!m_BasicAttackPressEdgeGate.Should_Submit(
			isPhysicallyDown, commandEligible))
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
	const auto readKeyState = [useRawKeyboard](const uint8_t keyCode)
	{
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

void Client::CPlayerController::Set_CommandSink(
	const shared_ptr<IPlayerCommandSink>& commandSink)
{
	m_pCommandSink = commandSink;
}

bool_t Client::CPlayerController::Request_Revive()
{
	if (nullptr == m_pCommandSink)
		return false;
	if (!m_pCommandSink->Request_RevivePlayer(m_iNextActionSequence))
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
	if (!m_pCommandSink->Request_DebugKillSelf(m_iNextActionSequence))
		return false;
	++m_iNextActionSequence;
	if (0u == m_iNextActionSequence)
		m_iNextActionSequence = 1u;
	return true;
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

bool_t Client::CPlayerController::Try_PickGroundPlane(
	f32_t groundY, float3_t & outPosition)
{
	//현재 마우스 위치에서 world ray를 만들고, 지정된 Y 평면과 교차시킨다
	//추후 navigation 사용해서 피킹과 스킬 사용에 따른 collider와도 연동을 시킨다.
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

	const vector_t direction = farPoint - nearPoint;
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
