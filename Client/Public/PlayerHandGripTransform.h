#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Network/PacketMessages.h"

#include <cmath>
#include <cstddef>

NS_BEGIN(Client)

/* Authored CAPTURE hit value in metres, resolved from the admitted Product
   encounter document. The Client composes it every frame onto the owner's
   replicated attachment socket (Valtan left hand) as a presentation-only
   position; the Server keeps its boss-local anchor for judgement, release and
   ejection. The struct is also the typed shape that Balance Tool, Composition
   Detail, the encounter reference parser and the gameplay publisher validate. */
struct PLAYER_HAND_GRIP_LOCAL_OFFSET final
{
	f32_t fForwardM = 0.f;
	f32_t fUpM = 0.f;
	f32_t fRightM = 0.f;

	bool operator==(const PLAYER_HAND_GRIP_LOCAL_OFFSET&) const = default;
};

/* One frame of the owner's presentation socket. SocketWorld is the bone
   combined matrix already multiplied by the owner's presentation root, so its
   translation is the palm in world metres; its basis may still carry the
   imported 0.01 scale and is never used for displacement. OwnerYawBasis is the
   owner's yaw-only actor transform whose right/look rows orient rightM/forwardM. */
struct PLAYER_HAND_GRIP_SOCKET_VIEW final
{
	float4x4_t SocketWorld{};
	float4x4_t OwnerYawBasis{};
};

/* Implemented by the replicated owner presentation (CValtan). CCharacter holds
   it weakly and asks every Update while the Server reports GRABBED; a false
   answer keeps the Server fallback transform for that frame. */
class IPlayerHandGripSocketSource
{
public:
	virtual ~IPlayerHandGripSocketSource() = default;
	virtual bool_t Try_Get_PlayerHandGripSocketView(
		LostArk::Shared::PLAYER_ATTACHMENT_SLOT slot,
		PLAYER_HAND_GRIP_SOCKET_VIEW& outView) const = 0;
	virtual bool_t Try_Get_PlayerHandGripLocalOffset(
		LostArk::Shared::PLAYER_ATTACHMENT_SLOT slot,
		PLAYER_HAND_GRIP_LOCAL_OFFSET& outOffset) const = 0;
};

class CPlayerHandGripTransform final
{
public:
	static constexpr f32_t MAX_GRIP_OFFSET_COMPONENT_M = 10.f;

	static bool_t Is_ValidGripLocalOffset(
		const PLAYER_HAND_GRIP_LOCAL_OFFSET& gripLocalOffset)
	{
		return Is_ValidGripComponent(gripLocalOffset.fForwardM) &&
			Is_ValidGripComponent(gripLocalOffset.fUpM) &&
			Is_ValidGripComponent(gripLocalOffset.fRightM);
	}

	/* Presentation position of the grabbed character's feet origin: the socket
	   translation displaced by the authored grip in the owner's yaw frame
	   (forward = owner look, right = owner right) and world up. The socket basis
	   scale never leaks into the result, so a 0.01-scaled bone and a normalized
	   bone place the character identically. */
	static bool_t Compose_WorldPosition(
		const PLAYER_HAND_GRIP_SOCKET_VIEW& view,
		const PLAYER_HAND_GRIP_LOCAL_OFFSET& gripLocalOffset,
		float3_t& outPosition)
	{
		if (!Is_FiniteMatrix(view.SocketWorld) ||
			!Is_FiniteMatrix(view.OwnerYawBasis) ||
			!Is_ValidGripLocalOffset(gripLocalOffset))
		{
			return false;
		}
		const matrix_t socket = DirectX::XMLoadFloat4x4(&view.SocketWorld);
		const matrix_t yawBasis = DirectX::XMLoadFloat4x4(&view.OwnerYawBasis);
		vector_t right = DirectX::XMVectorSetY(yawBasis.r[0], 0.f);
		vector_t forward = DirectX::XMVectorSetY(yawBasis.r[2], 0.f);
		if (DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(right)) <= 1.e-8f ||
			DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(forward)) <= 1.e-8f)
		{
			return false;
		}
		right = DirectX::XMVector3Normalize(right);
		forward = DirectX::XMVector3Normalize(forward);
		const vector_t position =
			DirectX::XMVectorSetW(socket.r[3], 1.f) +
			right * gripLocalOffset.fRightM +
			DirectX::XMVectorSet(0.f, gripLocalOffset.fUpM, 0.f, 0.f) +
			forward * gripLocalOffset.fForwardM;
		float3_t staged{};
		DirectX::XMStoreFloat3(&staged, position);
		if (!std::isfinite(staged.x) || !std::isfinite(staged.y) ||
			!std::isfinite(staged.z))
		{
			return false;
		}
		outPosition = staged;
		return true;
	}

private:
	static bool_t Is_ValidGripComponent(const f32_t value)
	{
		return std::isfinite(value) &&
			std::abs(value) <= MAX_GRIP_OFFSET_COMPONENT_M;
	}

	static bool_t Is_FiniteMatrix(const float4x4_t& value)
	{
		for (std::size_t row = 0u; row < 4u; ++row)
		{
			for (std::size_t column = 0u; column < 4u; ++column)
			{
				if (!std::isfinite(value.m[row][column]))
					return false;
			}
		}
		return true;
	}
};

NS_END
