#pragma once

namespace LostArk::Shared::WorldCollision
{
	// Server 이동/충돌과 Client 캐릭터 collider가 공유하는 m 단위 몸체 규격이다. 높이 원점은 발 위치다.
	// half extent는 반크기이며 center offset을 더하면 몸체 중심이 된다. 표시 모델 전체의 bounds와는 별개다.
	inline constexpr float CONTACT_MARGIN = 0.001f;
	inline constexpr float PLAYER_HALF_EXTENT_X = 0.45f;
	inline constexpr float PLAYER_HALF_EXTENT_Y = 0.90f;
	inline constexpr float PLAYER_HALF_EXTENT_Z = 0.45f;
	inline constexpr float PLAYER_CENTER_OFFSET_Y = 0.90f;
}
