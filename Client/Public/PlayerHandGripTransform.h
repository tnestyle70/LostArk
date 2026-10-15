#pragma once

// 클라이언트 공통 선언과 WinSock/Windows 헤더의 기존 포함 순서를 유지한다.
#include "Client_Defines.h"
// bool_t·f32_t·행렬/벡터 별칭과 DirectX 좌표 계산, namespace 매크로를 사용한다.
#include "Engine_Defines.h"
// Server가 복제하는 PLAYER_ATTACHMENT_SLOT으로 어느 부착 지점을 조회할지 구분한다.
#include "Network/PacketMessages.h"

// 행렬/거리의 유한성과 허용 범위를 검사한다.
#include <cmath>
// 행렬의 행·열을 순회하는 std::size_t를 사용한다.
#include <cstddef>

NS_BEGIN(Client)

// 승인된 Product encounter의 CAPTURE 설정에서 읽은 잡기 위치 보정값이다. 단위는 미터다.
// Balance Tool·Composition Detail·encounter 파서가 같은 형태를 사용하며,
// gameplay 게시 과정도 이 거리 범위를 검증한다.
// Client는 보스의 손 소켓에 이 값을 합성해 표시 위치만 바꾼다.
// 잡기 판정·해제·방출은 Server의 보스 로컬 기준점과 전투 상태가 계속 결정한다.
struct PLAYER_HAND_GRIP_LOCAL_OFFSET final
{
	// 전방/오른쪽은 보스의 수평 방향, 위쪽은 월드 +Y 방향이다.
	f32_t fForwardM = 0.f;
	f32_t fUpM = 0.f;
	f32_t fRightM = 0.f;

	// encounter 안의 여러 CAPTURE 설정이 같은지 확인할 때 성분 값을 그대로 비교한다.
	bool operator==(const PLAYER_HAND_GRIP_LOCAL_OFFSET&) const = default;
};

// 한 프레임의 손 위치와 보스 방향을 값으로 복사한 조회 결과다. 본/객체 포인터를 보관하지 않는다.
struct PLAYER_HAND_GRIP_SOCKET_VIEW final
{
	// 본의 누적 행렬에 표시용 루트 행렬까지 곱한 값이다. 마지막 행의 XYZ가 월드 손 위치다.
	// 본 기저에 남은 0.01 같은 반입 배율은 거리 보정에 사용하지 않는다.
	float4x4_t SocketWorld{};

	// 보스 Transform의 오른쪽/전방 행에서 수평 방향만 추출한다. 이 행렬의 위치는 사용하지 않는다.
	float4x4_t OwnerYawBasis{};
};

// CValtan/CNpc가 구현하는 손 소켓 조회 경계다. CCharacter는 weak_ptr로 연결해 보스 수명을 늘리지 않는다.
// Apply_NetworkAttachment가 보정값을 복사하고, GRABBED 중 Update가 매 프레임 소켓을 조회한다.
// 조회/합성 실패 시 그 프레임에 먼저 적용한 Server 기반 위치를 유지한다.
// 호출은 Client의 표시 갱신 흐름에서 직렬화한다. 이 인터페이스가 별도 잠금을 제공하지는 않는다.
class IPlayerHandGripSocketSource
{
public:
	// 인터페이스 포인터를 통한 삭제에서도 파생 소멸자가 실행되도록 한다.
	virtual ~IPlayerHandGripSocketSource() = default;

	// 해당 slot의 현재 손 위치/방향을 값으로 반환한다. 지원하지 않거나 본을 준비하지 못하면 false다.
	// 현재 CValtan/CNpc 구현은 성공할 때만 outView를 대입한다.
	virtual bool_t Try_Get_PlayerHandGripSocketView(
		LostArk::Shared::PLAYER_ATTACHMENT_SLOT slot,
		PLAYER_HAND_GRIP_SOCKET_VIEW& outView) const = 0;

	// 해당 slot에 승인된 거리 보정값을 조회한다. 없거나 범위가 잘못되면 false다.
	// 현재 CValtan/CNpc 구현은 성공할 때만 outOffset을 대입한다.
	virtual bool_t Try_Get_PlayerHandGripLocalOffset(
		LostArk::Shared::PLAYER_ATTACHMENT_SLOT slot,
		PLAYER_HAND_GRIP_LOCAL_OFFSET& outOffset) const = 0;
};

// 저작 값 검증과 표시 좌표 합성을 공유하는 무상태 계산 경계다.
// 객체·본을 소유하지 않으며 Server의 전투 상태나 실제 Transform을 직접 변경하지 않는다.
class CPlayerHandGripTransform final
{
public:
	// 각 성분의 절댓값 상한이다. 세 성분을 합친 벡터 길이의 상한을 뜻하지 않는다.
	static constexpr f32_t MAX_GRIP_OFFSET_COMPONENT_M = 10.f;

	// 저작/로드/부착 요청자가 공통으로 사용한다. 세 성분 모두 유한하고 허용 범위 안이어야 한다.
	static bool_t Is_ValidGripLocalOffset(
		const PLAYER_HAND_GRIP_LOCAL_OFFSET& gripLocalOffset)
	{
		return Is_ValidGripComponent(gripLocalOffset.fForwardM) &&
			Is_ValidGripComponent(gripLocalOffset.fUpM) &&
			Is_ValidGripComponent(gripLocalOffset.fRightM);
	}

	// CCharacter의 부착 갱신과 CNpc의 소켓 검증이 호출한다.
	// 손의 월드 위치에 보스의 수평 전방/오른쪽과 월드 위쪽 거리 보정을 더해 캐릭터 발 원점을 만든다.
	// 입력 행렬이나 범위가 잘못되거나 수평 축이 퇴화하면 false이며 outPosition은 보존한다.
	static bool_t Compose_WorldPosition(
		const PLAYER_HAND_GRIP_SOCKET_VIEW& view,
		const PLAYER_HAND_GRIP_LOCAL_OFFSET& gripLocalOffset,
		float3_t& outPosition)
	{
		// 사용하지 않는 행까지 포함해 두 행렬의 모든 성분을 검사한 뒤 계산을 시작한다.
		if (!Is_FiniteMatrix(view.SocketWorld) ||
			!Is_FiniteMatrix(view.OwnerYawBasis) ||
			!Is_ValidGripLocalOffset(gripLocalOffset))
		{
			return false;
		}

		// 본 기저 대신 보스 Transform의 오른쪽/전방을 XZ 평면으로 내려 사용한다.
		// 보스 기저의 수직 성분이 수평 거리 보정에 섞이지 않게 한다.
		const matrix_t socket = DirectX::XMLoadFloat4x4(&view.SocketWorld);
		const matrix_t yawBasis = DirectX::XMLoadFloat4x4(&view.OwnerYawBasis);

		vector_t right = DirectX::XMVectorSetY(yawBasis.r[0], 0.f);
		vector_t forward = DirectX::XMVectorSetY(yawBasis.r[2], 0.f);

		// 정규화할 수 없는 짧은 수평 축을 거른다. 1.e-8은 축 길이 제곱에 대한 임계값이다.
		if (DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(right)) <= 1.e-8f ||
			DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(forward)) <= 1.e-8f)
		{
			return false;
		}

		// 축 길이를 제거해 보정값을 그대로 미터 거리로 적용한다.
		// 두 방향은 각각 정규화하며 서로 직교하도록 다시 만드는 연산은 하지 않는다.
		right = DirectX::XMVector3Normalize(right);
		forward = DirectX::XMVector3Normalize(forward);

		// 위치 = 손의 월드 위치 + 오른쪽 거리 + 월드 높이 + 전방 거리다.
		// SocketWorld의 회전/배율과 OwnerYawBasis의 위치는 이 변위 계산에 사용하지 않는다.
		const vector_t position =
			DirectX::XMVectorSetW(socket.r[3], 1.f) +
			right * gripLocalOffset.fRightM +
			DirectX::XMVectorSet(0.f, gripLocalOffset.fUpM, 0.f, 0.f) +
			forward * gripLocalOffset.fForwardM;

		// 계산 결과도 검증한 뒤 한 번만 출력에 대입한다. 실패 시 호출자의 기존 위치가 남는다.
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

	// 한 축의 NaN/무한대와 ±10미터를 벗어난 값을 거른다. 경계값 자체는 허용한다.
	static bool_t Is_ValidGripComponent(const f32_t value)
	{
		return std::isfinite(value) &&
			std::abs(value) <= MAX_GRIP_OFFSET_COMPONENT_M;
	}

	// 유한성만 검사한다. 행렬의 가역성·직교성이나 affine 형태를 보장하는 검사는 아니다.
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
