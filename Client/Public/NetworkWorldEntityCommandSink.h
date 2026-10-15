#pragma once

// Level이 사용하는 typed world 명령 인터페이스를 구현한다.
#include "WorldEntityCommandSink.h"

namespace Client
{
	// CharacterSelect·Valtan·Kouku Level이 shared_ptr로 보유하는 네트워크 전달 구현이다.
	// 자체 객체 목록·socket은 없으며 공용 NetworkManager를 호출한다.
	class CNetworkWorldEntityCommandSink final :
		public IWorldEntityCommandSink
	{
	public:
		// NetworkManager에 placement ID와 선택적 추적 token 출력을 전달한다.
		bool Request_SpawnWorldEntity(
			std::string_view placementId, std::uint64_t* outRequestToken = nullptr) override;
		// 요청 순번을 보존해 Server의 world entity 해제를 요청한다.
		bool Request_DespawnAllWorldEntities(
			std::uint32_t requestSequence) override;
		// Level을 즉시 바꾸지 않고 Server 진입 요청의 송신 결과를 반환한다.
		bool Request_EnterKoukuSaydonArena(
			std::uint32_t requestSequence) override;
		// placement ID의 이동 요청을 전송한다. Character Transform을 직접 바꾸지 않는다.
		bool Request_StageTeleport(
			std::uint32_t requestSequence,
			std::string_view placementId) override;
	};
}
