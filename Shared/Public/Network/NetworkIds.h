#pragma once

// player/entity/vehicle/title의 32비트 ID와 combat object의 64비트 ID 표현을 Client와 Server에서 동일하게 고정한다.
#include <cstdint>
//playerid와 netentity id의 의미를 고정
//invalid 값은 0
//shared 소유
//client와 server가 동일하게 사용

//payload를 8byte로 통일 시킨다
// 현재 PLAYER_ID/NET_ENTITY_ID는 각각 4바이트, COMBAT_OBJECT_ID는 8바이트다. payload 전체 크기는 메시지별 serializer가 정한다.
namespace LostArk::Shared
{
	// 네트워크에서 사용하는 ID 타입들의 정본
	// using 별칭은 의미를 구분하지만 별개의 강한 타입은 아니다. 같은 정수 표현이라도 식별 대상과 조회 저장소를 혼용하지 않는다.
	using PLAYER_ID = std::uint32_t;

	using NET_ENTITY_ID = std::uint32_t;

	// Room 소유 CombatObjectRuntime이 별도 공간에서 발급하는 ID다. actor의 NET_ENTITY_ID로 조회하지 않는다.
	// 발급자는 정수 최댓값에서 1로 돌아가며 충돌을 확인하므로 영구적인 전역 단조 증가 ID는 아니다.
	using COMBAT_OBJECT_ID = std::uint64_t;

	inline constexpr PLAYER_ID INVALID_PLAYER_ID = 0;

	inline constexpr NET_ENTITY_ID INVALID_NET_ENTITY_ID = 0;

	inline constexpr COMBAT_OBJECT_ID INVALID_COMBAT_OBJECT_ID = 0;

	// EFTable_Vehicle의 key를 담는다. Server의 게시 vehicle 정의에서 승인하며 0은 탑승하지 않음을 뜻한다.
	using VEHICLE_ID = std::uint32_t;

	inline constexpr VEHICLE_ID INVALID_VEHICLE_ID = 0;

	// EFTable_HonorTitle의 key다. Server가 게시 칭호 정의와 요청 조건을 검사하며 0은 칭호 해제다.
	// 이 별칭 자체는 요청 승인이나 Catalog 조회를 수행하지 않는다.
	using HONOR_TITLE_ID = std::uint32_t;

	inline constexpr HONOR_TITLE_ID INVALID_HONOR_TITLE_ID = 0;
}
