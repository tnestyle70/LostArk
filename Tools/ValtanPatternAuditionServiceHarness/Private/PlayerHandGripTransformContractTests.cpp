#include "PlayerHandGripTransform.h"

#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace
{
	constexpr f32_t EPSILON = 1.e-4f;

	void Require(const bool_t condition, const char* const message)
	{
		if (!condition)
			throw std::runtime_error(message);
	}

	bool_t MatricesNear(
		const float4x4_t& left,
		const float4x4_t& right)
	{
		for (std::size_t row = 0u; row < 4u; ++row)
			for (std::size_t column = 0u; column < 4u; ++column)
				if (std::abs(left.m[row][column] - right.m[row][column]) >
					EPSILON)
				{
					return false;
				}
		return true;
	}

	float4x4_t Store(const matrix_t value)
	{
		float4x4_t stored{};
		DirectX::XMStoreFloat4x4(&stored, value);
		return stored;
	}

	void VerifyH3CaptureBasisAndDistanceContract()
	{
		const float4x4_t handWorld = Store(
			DirectX::XMMatrixScaling(0.01f, 0.01f, 0.01f) *
			DirectX::XMMatrixRotationRollPitchYaw(0.4f, -0.8f, 0.2f) *
			DirectX::XMMatrixTranslation(5.f, 3.f, -7.f));
		const float4x4_t playerWorld = Store(
			DirectX::XMMatrixScaling(1.25f, 0.75f, 1.5f) *
			DirectX::XMMatrixRotationRollPitchYaw(0.2f, 0.6f, -0.1f) *
			DirectX::XMMatrixTranslation(40.f, 2.f, -10.f));
		float4x4_t distantPlayerWorld = playerWorld;
		distantPlayerWorld._41 = -80.f;
		distantPlayerWorld._42 = 7.f;
		distantPlayerWorld._43 = 90.f;

		float4x4_t localOffset{};
		float4x4_t distantLocalOffset{};
		Require(Client::CPlayerHandGripTransform::Build_LocalOffset(
			playerWorld, handWorld, localOffset),
			"H3 local offset rejected the normal 0.01 hand scale");
		Require(Client::CPlayerHandGripTransform::Build_LocalOffset(
			distantPlayerWorld, handWorld, distantLocalOffset),
			"H3 local offset rejected the distant capture fixture");
		Require(MatricesNear(localOffset, distantLocalOffset) &&
			0.f == localOffset._41 && 0.f == localOffset._42 &&
			0.f == localOffset._43,
			"H3 local offset retained capture distance");

		float4x4_t attachedWorld{};
		Require(Client::CPlayerHandGripTransform::Compose_World(
			localOffset, handWorld, attachedWorld),
			"H3 local times hand composition failed");
		float4x4_t expectedWorld = playerWorld;
		expectedWorld._41 = handWorld._41;
		expectedWorld._42 = handWorld._42;
		expectedWorld._43 = handWorld._43;
		Require(MatricesNear(attachedWorld, expectedWorld),
			"H3 composition changed player basis or retained Valtan scale");
	}

	void VerifyAuthoredGripUsesNormalizedHandAxes()
	{
		const Client::PLAYER_HAND_GRIP_LOCAL_OFFSET grip{
			0.25f, -0.9f, 0.1f };
		float4x4_t identity{};
		DirectX::XMStoreFloat4x4(&identity, DirectX::XMMatrixIdentity());
		float4x4_t reference{};
		bool_t hasReference = false;
		for (const f32_t handScale : { 0.01f, 0.009999995f, 1.f })
		{
			const float4x4_t handWorld = Store(
				DirectX::XMMatrixScaling(handScale, handScale, handScale) *
				DirectX::XMMatrixRotationY(DirectX::XM_PIDIV2) *
				DirectX::XMMatrixTranslation(10.f, 20.f, 30.f));
			float4x4_t attached{};
			Require(Client::CPlayerHandGripTransform::Compose_World(
				identity, handWorld, grip, attached),
				"authored grip rejected a valid normal or ghost hand scale");

			const matrix_t hand = DirectX::XMLoadFloat4x4(&handWorld);
			const vector_t displacement =
				DirectX::XMVector3Normalize(hand.r[0]) *
					(grip.fRightM * 100.f) +
				DirectX::XMVector3Normalize(hand.r[1]) *
					(grip.fUpM * 100.f) +
				DirectX::XMVector3Normalize(hand.r[2]) *
					(grip.fForwardM * 100.f);
			float3_t expectedDisplacement{};
			DirectX::XMStoreFloat3(&expectedDisplacement, displacement);
			Require(std::abs(attached._41 -
				(handWorld._41 + expectedDisplacement.x)) <= EPSILON &&
				std::abs(attached._42 -
					(handWorld._42 + expectedDisplacement.y)) <= EPSILON &&
				std::abs(attached._43 -
					(handWorld._43 + expectedDisplacement.z)) <= EPSILON,
				"authored metre offset was not applied in normalized hand axes");
			if (!hasReference)
			{
				reference = attached;
				hasReference = true;
			}
			else
			{
				Require(std::abs(attached._41 - reference._41) <= EPSILON &&
					std::abs(attached._42 - reference._42) <= EPSILON &&
					std::abs(attached._43 - reference._43) <= EPSILON,
					"grip translation was multiplied by imported bone scale");
			}
		}

		float4x4_t legacy{};
		float4x4_t missingFallback{};
		const float4x4_t handWorld = Store(
			DirectX::XMMatrixRotationY(0.3f) *
			DirectX::XMMatrixTranslation(4.f, 5.f, 6.f));
		Require(Client::CPlayerHandGripTransform::Compose_World(
			identity, handWorld, legacy) &&
			Client::CPlayerHandGripTransform::Compose_World(
				identity, handWorld,
				Client::PLAYER_HAND_GRIP_LOCAL_OFFSET{}, missingFallback) &&
			MatricesNear(legacy, missingFallback),
			"missing optional grip offset changed the legacy H3 contract");
	}

	void VerifyInvalidInputRollsBackOutput()
	{
		float4x4_t identity{};
		DirectX::XMStoreFloat4x4(&identity, DirectX::XMMatrixIdentity());
		const float4x4_t committed = Store(
			DirectX::XMMatrixTranslation(3.f, 4.f, 5.f));
		for (const Client::PLAYER_HAND_GRIP_LOCAL_OFFSET invalid : {
			Client::PLAYER_HAND_GRIP_LOCAL_OFFSET{
				(std::numeric_limits<f32_t>::quiet_NaN)(), 0.f, 0.f },
			Client::PLAYER_HAND_GRIP_LOCAL_OFFSET{
				0.f, (std::numeric_limits<f32_t>::infinity)(), 0.f },
			Client::PLAYER_HAND_GRIP_LOCAL_OFFSET{ 0.f, 0.f, 10.01f } })
		{
			float4x4_t unchanged = committed;
			Require(!Client::CPlayerHandGripTransform::Compose_World(
				identity, identity, invalid, unchanged) &&
				MatricesNear(unchanged, committed),
				"invalid grip offset changed committed presentation output");
		}

		std::array<float4x4_t, 3u> invalidMatrices{
			identity, identity, identity };
		invalidMatrices[0]._11 = 0.f;
		invalidMatrices[1]._41 =
			(std::numeric_limits<f32_t>::quiet_NaN)();
		invalidMatrices[2]._14 = 1.f;
		for (const float4x4_t& invalid : invalidMatrices)
		{
			float4x4_t unchanged = committed;
			Require(!Client::CPlayerHandGripTransform::Compose_World(
				identity, invalid,
				Client::PLAYER_HAND_GRIP_LOCAL_OFFSET{}, unchanged) &&
				MatricesNear(unchanged, committed),
				"invalid hand matrix changed committed presentation output");
		}
	}
}

int Run_PlayerHandGripTransformContractTests()
{
	try
	{
		VerifyH3CaptureBasisAndDistanceContract();
		VerifyAuthoredGripUsesNormalizedHandAxes();
		VerifyInvalidInputRollsBackOutput();
		std::cout << "PlayerHandGripTransformContractTests: 3/3 passed\n";
		return 0;
	}
	catch (const std::exception& error)
	{
		std::cerr << "PlayerHandGripTransformContractTests: FAIL: " <<
			error.what() << '\n';
		return 1;
	}
}
