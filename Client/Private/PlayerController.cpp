#include "PlayerController.h"

#include "Character.h"
#include "GameInstance.h"
#include "PlayerCommandSink.h"
#include "Transform.h"

#include <cmath>

void Client::CPlayerController::Set_LocalCharacter(const shared_ptr<CCharacter>& character)
{
	if (m_pLocalCharacter.lock() == character)
		return;

	m_pLocalCharacter = character;
	m_iNextMoveSequence = 1;
	m_iNextActionSequence = 1;
	m_wasRightMouseDown = false;
	m_wasQDown = false;
	m_wasWDown = false;
}

void Client::CPlayerController::Update()
{
	//imgui로 mouse block된 상태가 아니고, Right Button
	const bool_t isRightMouseDown =
		!CGameInstance::Get().IsMouseInputBlocked() &&
		0 != (CGameInstance::Get().Get_DIMouseState(DIM::RB) & 0x80);
	const bool_t isKeyboardBlocked =
		CGameInstance::Get().IsKeyboardInputBlocked();
	const bool_t isQDown = !isKeyboardBlocked &&
		0 != (CGameInstance::Get().Get_DIKeyState(DIK_Q) & 0x80);
	const bool_t isWDown = !isKeyboardBlocked &&
		0 != (CGameInstance::Get().Get_DIKeyState(DIK_W) & 0x80);
	//weak_ptr로 가지고 있는 character lock
	const shared_ptr<CCharacter> character =
		m_pLocalCharacter.lock();
	const shared_ptr<IPlayerCommandSink> commandSink =
		m_pCommandSink;

	if (isRightMouseDown &&
		!m_wasRightMouseDown &&
		nullptr != character &&
		nullptr != commandSink)
	{
		const shared_ptr<CTransform> transform =
			character->Get_Transform();

		if (nullptr != transform)
		{
			const f32_t groundY = XMVectorGetY(
				transform->Get_State(STATE::POSITION));

			float3_t goal{};

			if (Try_PickGroundPlane(groundY, goal) &&
				commandSink->Request_MoveGoal(
					m_iNextMoveSequence,
					goal.x,
					goal.z))
			{
				++m_iNextMoveSequence;
				if (0 == m_iNextMoveSequence)
					m_iNextMoveSequence = 1;
			}
		}
	}

	LostArk::Shared::SKILL_ID requestedSkillId =
		LostArk::Shared::INVALID_SKILL_ID;
	if (isQDown && !m_wasQDown)
		requestedSkillId = 34060;
	else if (isWDown && !m_wasWDown)
		requestedSkillId = 34100;
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
	m_wasQDown = isQDown;
	m_wasWDown = isWDown;
}

void Client::CPlayerController::Set_CommandSink(
	const shared_ptr<IPlayerCommandSink>& commandSink)
{
	m_pCommandSink = commandSink;
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
