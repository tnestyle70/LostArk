#pragma once

// 요청 순번과 spawn 추적 token의 고정폭 정수를 사용한다.
#include <cstdint>
// placement ID를 호출 동안 차용하며 인터페이스가 문자열을 소유하지 않는다.
#include <string_view>

namespace Client
{
	// Level·도구가 world spawn·해제·진입·이동 의도를 전달하는 경계다. 구현체가 요청을 처리한다.
	class IWorldEntityCommandSink
	{
	public:
		// 기반 인터페이스 포인터로 해제해도 실제 구현체의 소멸자가 실행되게 한다.
		virtual ~IWorldEntityCommandSink() = default;
		// stable placement ID의 spawn을 요청한다. 네트워크 구현의 true는 Server의 spawn 완료를 뜻하지 않는다.
		virtual bool Request_SpawnWorldEntity(
			std::string_view placementId, std::uint64_t* outRequestToken = nullptr) = 0;
		// 현재 world entity의 일괄 해제를 요청한다. 실제 제거는 Server 응답·복제 경로가 반영한다.
		virtual bool Request_DespawnAllWorldEntities(
			std::uint32_t requestSequence) = 0;
		// Debug 저작 진입을 요청한다. 실제 Level 진입은 Server의 S2C_ENTER_ACCEPTED 승인으로 결정한다.
		virtual bool Request_EnterKoukuSaydonArena(
			std::uint32_t requestSequence) = 0;
		// stable playerSpawn 배치로 이동을 요청한다. Server가 navigation을 검사하고 결과 위치를 복제한다.
		virtual bool Request_StageTeleport(
			std::uint32_t requestSequence,
			std::string_view placementId) = 0;
	};
}
