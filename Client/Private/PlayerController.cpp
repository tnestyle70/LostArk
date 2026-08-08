#include "imgui.h"

#include "PlayerController.h"

#include "Character.h"
#include "CombatHUDViewModel.h"
#include "GameInstance.h"
#include "PlayerCommandSink.h"
#include "PlayerSkillCatalog.h"
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

	/* Tighter than the narrowest combo window the balance data declares (183 ms
	on the Artist third basic-attack stage), so a held button cannot skip one. */
	constexpr std::chrono::milliseconds BASIC_ATTACK_RESEND_INTERVAL{ 100 };

	constexpr std::chrono::milliseconds MOVE_GOAL_RESEND_INTERVAL{ 100 };
	constexpr f32_t MOVE_GOAL_DEADZONE_RADIUS = 1.f;
	constexpr f32_t MOVE_GOAL_RESEND_EPSILON = 0.25f;
}

void Client::CPlayerController::Set_LocalCharacter(const shared_ptr<CCharacter>& character)
{
	if (m_pLocalCharacter.lock() == character)
		return;

	m_pLocalCharacter = character;
	m_iNextMoveSequence = 1;
	m_iNextActionSequence = 1;
	m_wasRightMouseDown = false;
	m_wasKeyDown.fill(false);
	m_wasLeftMouseDown = false;
	m_LastBasicAttackSentAt = {};
	m_LastMoveGoalSentAt = {};
	m_LastSentMoveGoal = {};
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

	const CHARACTER_SPEC* pSpec =
		nullptr != character ? character->Get_Spec() : nullptr;
	const LostArk::Shared::PLAYER_STANCE_ID stance =
		CCombatHUDViewModel::Get().Get_Player().eStance;

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
		if (isKeyboardBlocked || slot.requiresAlt != isAltDown ||
			!isDown[index] || m_wasKeyDown[slot.byKeyCode] ||
			nullptr == pSpec ||
			LostArk::Shared::INVALID_SKILL_ID != outSkillId)
		{
			continue;
		}

		/* An unbound slot is normal: a class simply has no skill there. */
		const PLAYER_SKILL_DEFINITION* pSkill = CPlayerSkillCatalog::Find_BySlot(
			pSpec->eCharacterClass, slot.pInputSlot, stance);
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
	const bool_t isDown =
		!CGameInstance::Get().IsMouseInputBlocked() &&
		0 != (CGameInstance::Get().Get_DIMouseState(DIM::LB) & 0x80);
	if (!isDown)
	{
		m_wasLeftMouseDown = false;
		return;
	}

	const bool_t wasDown = m_wasLeftMouseDown;
	m_wasLeftMouseDown = true;
	if (commandSuppressed || nullptr == pSpec ||
		LostArk::Shared::INVALID_SKILL_ID != outSkillId)
	{
		return;
	}

	/* The first press goes out immediately; holding repeats on an interval
	narrower than the tightest combo window in the data (183 ms), so a held
	button always lands at least one press inside it. */
	const auto now = std::chrono::steady_clock::now();
	if (wasDown && now - m_LastBasicAttackSentAt < BASIC_ATTACK_RESEND_INTERVAL)
		return;

	const PLAYER_SKILL_DEFINITION* pSkill = CPlayerSkillCatalog::Find_BySlot(
		pSpec->eCharacterClass, "LMB", stance);
	if (nullptr == pSkill)
		return;

	m_LastBasicAttackSentAt = now;
	outSkillId = pSkill->iSkillId;
}

void Client::CPlayerController::Set_CommandSink(
	const shared_ptr<IPlayerCommandSink>& commandSink)
{
	m_pCommandSink = commandSink;
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
	f32_t groundY, float3_t & outPosition) const
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
