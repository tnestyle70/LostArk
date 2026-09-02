#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cmath>
#include <cstddef>

NS_BEGIN(Client)

/* Authored in metres in the wrist-bone frame.  Valtan model bones can carry
   either the imported 0.01 conversion scale or an already-normalized basis,
   so the translation is applied through normalized hand axes and converted
   to the Client world centimetre convention exactly once. */
struct PLAYER_HAND_GRIP_LOCAL_OFFSET final
{
	f32_t fForwardM = 0.f;
	f32_t fUpM = 0.f;
	f32_t fRightM = 0.f;

	bool operator==(const PLAYER_HAND_GRIP_LOCAL_OFFSET&) const = default;
};

/* Pure presentation math shared by replication and the headless contract
   harness. Capture position never becomes a hand-local displacement. */
class CPlayerHandGripTransform final
{
public:
	static constexpr f32_t MAX_GRIP_OFFSET_COMPONENT_M = 10.f;
	static constexpr f32_t WORLD_UNITS_PER_METRE = 100.f;

	static bool_t Is_ValidGripLocalOffset(
		const PLAYER_HAND_GRIP_LOCAL_OFFSET& gripLocalOffset)
	{
		return Is_ValidGripComponent(gripLocalOffset.fForwardM) &&
			Is_ValidGripComponent(gripLocalOffset.fUpM) &&
			Is_ValidGripComponent(gripLocalOffset.fRightM);
	}

	static bool_t Build_LocalOffset(
		const float4x4_t& playerWorld,
		const float4x4_t& handWorld,
		float4x4_t& outLocalOffset)
	{
		if (!Is_UsableAffineMatrix(playerWorld) ||
			!Is_UsableAffineMatrix(handWorld))
			return false;

		matrix_t playerBasis = DirectX::XMLoadFloat4x4(&playerWorld);
		matrix_t handBasis = DirectX::XMLoadFloat4x4(&handWorld);
		playerBasis.r[3] = DirectX::XMVectorSet(0.f, 0.f, 0.f, 1.f);
		handBasis.r[3] = DirectX::XMVectorSet(0.f, 0.f, 0.f, 1.f);
		matrix_t local = playerBasis *
			DirectX::XMMatrixInverse(nullptr, handBasis);
		local.r[3] = DirectX::XMVectorSet(0.f, 0.f, 0.f, 1.f);
		float4x4_t staged{};
		DirectX::XMStoreFloat4x4(&staged, local);
		if (!Is_UsableAffineMatrix(staged))
			return false;
		outLocalOffset = staged;
		return true;
	}

	static bool_t Compose_World(
		const float4x4_t& localOffset,
		const float4x4_t& handWorld,
		float4x4_t& outWorld)
	{
		if (!Is_UsableAffineMatrix(localOffset) ||
			!Is_UsableAffineMatrix(handWorld))
			return false;
		float4x4_t staged{};
		DirectX::XMStoreFloat4x4(&staged,
			DirectX::XMLoadFloat4x4(&localOffset) *
			DirectX::XMLoadFloat4x4(&handWorld));
		if (!Is_UsableAffineMatrix(staged))
			return false;
		outWorld = staged;
		return true;
	}

	static bool_t Compose_World(
		const float4x4_t& localOffset,
		const float4x4_t& handWorld,
		const PLAYER_HAND_GRIP_LOCAL_OFFSET& gripLocalOffset,
		float4x4_t& outWorld)
	{
		if (!Is_UsableAffineMatrix(localOffset) ||
			!Is_UsableAffineMatrix(handWorld) ||
			!Is_ValidGripLocalOffset(gripLocalOffset))
		{
			return false;
		}
		if (0.f == gripLocalOffset.fForwardM &&
			0.f == gripLocalOffset.fUpM &&
			0.f == gripLocalOffset.fRightM)
		{
			return Compose_World(localOffset, handWorld, outWorld);
		}

		matrix_t adjustedHand = DirectX::XMLoadFloat4x4(&handWorld);
		const vector_t right = DirectX::XMVector3Normalize(adjustedHand.r[0]);
		const vector_t up = DirectX::XMVector3Normalize(adjustedHand.r[1]);
		const vector_t forward = DirectX::XMVector3Normalize(adjustedHand.r[2]);
		const vector_t displacement =
			right * (gripLocalOffset.fRightM * WORLD_UNITS_PER_METRE) +
			up * (gripLocalOffset.fUpM * WORLD_UNITS_PER_METRE) +
			forward * (gripLocalOffset.fForwardM * WORLD_UNITS_PER_METRE);
		adjustedHand.r[3] = DirectX::XMVectorSetW(
			adjustedHand.r[3] + displacement, 1.f);

		float4x4_t stagedHand{};
		DirectX::XMStoreFloat4x4(&stagedHand, adjustedHand);
		return Compose_World(localOffset, stagedHand, outWorld);
	}

private:
	static bool_t Is_ValidGripComponent(const f32_t value)
	{
		return std::isfinite(value) &&
			std::abs(value) <= MAX_GRIP_OFFSET_COMPONENT_M;
	}

	static bool_t Is_UsableAffineMatrix(const float4x4_t& value)
	{
		for (std::size_t row = 0u; row < 4u; ++row)
			for (std::size_t column = 0u; column < 4u; ++column)
				if (!std::isfinite(value.m[row][column]))
					return false;
		if (std::abs(value._14) > 1.e-5f ||
			std::abs(value._24) > 1.e-5f ||
			std::abs(value._34) > 1.e-5f ||
			std::abs(value._44 - 1.f) > 1.e-5f)
			return false;

		// Normal Valtan hand bones carry approximately 0.01 scale. Judge
		// basis degeneracy independently of that valid model conversion.
		double normalized[3u][3u]{};
		for (std::size_t row = 0u; row < 3u; ++row)
		{
			double lengthSquared = 0.0;
			for (std::size_t column = 0u; column < 3u; ++column)
			{
				const double component = value.m[row][column];
				lengthSquared += component * component;
			}
			const double length = std::sqrt(lengthSquared);
			if (!std::isfinite(length) || !(length > 0.0))
				return false;
			for (std::size_t column = 0u; column < 3u; ++column)
				normalized[row][column] = value.m[row][column] / length;
		}
		const double determinant =
			normalized[0u][0u] * (normalized[1u][1u] * normalized[2u][2u] -
				normalized[1u][2u] * normalized[2u][1u]) -
			normalized[0u][1u] * (normalized[1u][0u] * normalized[2u][2u] -
				normalized[1u][2u] * normalized[2u][0u]) +
			normalized[0u][2u] * (normalized[1u][0u] * normalized[2u][1u] -
				normalized[1u][1u] * normalized[2u][0u]);
		return std::isfinite(determinant) && std::abs(determinant) > 1.e-6;
	}
};

NS_END
