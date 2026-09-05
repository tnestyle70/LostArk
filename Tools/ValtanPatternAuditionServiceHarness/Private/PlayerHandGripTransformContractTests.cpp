#include "PlayerHandGripTransform.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace
{
	void Require(const bool_t bCondition, const char* const pMessage)
	{
		if (!bCondition)
			throw std::runtime_error(pMessage);
	}

	bool_t Near(const f32_t a, const f32_t b)
	{
		return std::abs(a - b) <= 1.e-4f;
	}

	Client::PLAYER_HAND_GRIP_SOCKET_VIEW MakeView(
		const DirectX::XMMATRIX& Socket, const DirectX::XMMATRIX& YawBasis)
	{
		Client::PLAYER_HAND_GRIP_SOCKET_VIEW View{};
		DirectX::XMStoreFloat4x4(&View.SocketWorld, Socket);
		DirectX::XMStoreFloat4x4(&View.OwnerYawBasis, YawBasis);
		return View;
	}

	void VerifyScaledSocketBasisDoesNotLeak()
	{
		const DirectX::XMMATRIX Socket =
			DirectX::XMMatrixScaling(0.01f, 0.01f, 0.01f) *
			DirectX::XMMatrixRotationRollPitchYaw(0.7f, -1.1f, 0.3f) *
			DirectX::XMMatrixTranslation(3.f, 2.f, 1.f);
		const Client::PLAYER_HAND_GRIP_LOCAL_OFFSET Grip{ 0.f, -0.9f, 0.f };
		float3_t Position{};
		Require(Client::CPlayerHandGripTransform::Compose_WorldPosition(
			MakeView(Socket, DirectX::XMMatrixIdentity()), Grip, Position),
			"a 0.01-scaled rotated socket was rejected");
		Require(Near(Position.x, 3.f) && Near(Position.y, 1.1f) &&
			Near(Position.z, 1.f),
			"socket basis scale or rotation leaked into the feet position");
	}

	void VerifyYawBasisOrientsForwardAndRight()
	{
		const DirectX::XMMATRIX Socket =
			DirectX::XMMatrixTranslation(3.f, 2.f, 1.f);
		const DirectX::XMMATRIX YawBasis =
			DirectX::XMMatrixScaling(5.f, 5.f, 5.f) *
			DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(90.f)) *
			DirectX::XMMatrixTranslation(-40.f, 9.f, 12.f);
		const Client::PLAYER_HAND_GRIP_LOCAL_OFFSET Grip{ 1.f, 0.f, 0.5f };
		float3_t Position{};
		Require(Client::CPlayerHandGripTransform::Compose_WorldPosition(
			MakeView(Socket, YawBasis), Grip, Position),
			"a scaled yaw basis was rejected");
		Require(Near(Position.x, 4.f) && Near(Position.y, 2.f) &&
			Near(Position.z, 0.5f),
			"forwardM/rightM did not follow the normalized owner look/right rows");
	}

	void VerifyOwnerTranslationNeverMovesTheGrip()
	{
		const DirectX::XMMATRIX Socket =
			DirectX::XMMatrixTranslation(-2.f, 4.f, 6.f);
		const DirectX::XMMATRIX YawBasis =
			DirectX::XMMatrixTranslation(100.f, 100.f, 100.f);
		const Client::PLAYER_HAND_GRIP_LOCAL_OFFSET Grip{};
		float3_t Position{};
		Require(Client::CPlayerHandGripTransform::Compose_WorldPosition(
			MakeView(Socket, YawBasis), Grip, Position),
			"an identity-rotation owner basis was rejected");
		Require(Near(Position.x, -2.f) && Near(Position.y, 4.f) &&
			Near(Position.z, 6.f),
			"the owner translation displaced a zero grip");
	}

	void VerifyInvalidInputsAreRejected()
	{
		const DirectX::XMMATRIX Socket =
			DirectX::XMMatrixTranslation(1.f, 1.f, 1.f);
		float3_t Position{ 7.f, 7.f, 7.f };
		Client::PLAYER_HAND_GRIP_SOCKET_VIEW View =
			MakeView(Socket, DirectX::XMMatrixIdentity());
		View.SocketWorld._42 = std::numeric_limits<f32_t>::quiet_NaN();
		Require(!Client::CPlayerHandGripTransform::Compose_WorldPosition(
			View, Client::PLAYER_HAND_GRIP_LOCAL_OFFSET{}, Position),
			"a non-finite socket translation was accepted");

		View = MakeView(Socket, DirectX::XMMatrixScaling(0.f, 0.f, 0.f));
		Require(!Client::CPlayerHandGripTransform::Compose_WorldPosition(
			View, Client::PLAYER_HAND_GRIP_LOCAL_OFFSET{}, Position),
			"a degenerate owner yaw basis was accepted");

		View = MakeView(Socket, DirectX::XMMatrixIdentity());
		Require(!Client::CPlayerHandGripTransform::Compose_WorldPosition(
			View, Client::PLAYER_HAND_GRIP_LOCAL_OFFSET{ 0.f, -10.01f, 0.f },
			Position),
			"an out-of-range grip was accepted");
		Require(Near(Position.x, 7.f) && Near(Position.y, 7.f) &&
			Near(Position.z, 7.f),
			"a rejected composition mutated the caller's position");
	}
}

int Run_PlayerHandGripTransformContractTests()
{
	try
	{
		VerifyScaledSocketBasisDoesNotLeak();
		VerifyYawBasisOrientsForwardAndRight();
		VerifyOwnerTranslationNeverMovesTheGrip();
		VerifyInvalidInputsAreRejected();
		std::cout << "PlayerHandGripTransformContractTests: 4/4 passed\n";
		return 0;
	}
	catch (const std::exception& Error)
	{
		std::cerr << "PlayerHandGripTransformContractTests: FAIL: " <<
			Error.what() << '\n';
		return 1;
	}
}
