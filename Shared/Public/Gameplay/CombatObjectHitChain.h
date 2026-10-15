#pragma once

// XZ 몸체 원의 유효성과 부채꼴 접촉을 공통 기하 판정으로 계산한다.
#include "Gameplay/CombatCollisionContract.h"
// 부채꼴 원점·전방·길이·각도가 유한한 수치인지 검사한다.
#include <cmath>
// 연쇄 지연의 ms 값을 32비트 정수로 전달한다.
#include <cstdint>
// 호출자의 몸체 목록을 순서대로 읽고 같은 순서의 지연 목록을 출력이 소유한다.
#include <vector>

namespace LostArk::Shared::CombatObjectHitChain
{
	// 주체의 공격 한 번을 XZ 원점·전방·길이(m)·전체 각도(degree)로 전달한다. 시퀀스 ID는 호출자가 따로 보관한다.
	struct CONE_HIT final
	{
		float fOriginX = 0.f, fOriginZ = 0.f;
		float fForwardX = 0.f, fForwardZ = 1.f;
		float fLength = 0.f, fAngleDegrees = 0.f;
	};

	// Server CombatObjectRuntime과 Client Valtan 표현이 같은 몸체 배열 순서로 연쇄 지연을 만든다.
	// 닿은 항목은 0ms, 나머지는 delayMs다. 하나도 닿지 않거나 입력이 잘못되면 출력을 비우고 false다.
	// 시작할 때 기존 출력을 지우므로 실패 시 이전 결과를 보존하는 API는 아니다.
	// 한 공격을 다시 처리하지 않는 identity/latch와 실제 연쇄 시작은 호출자가 소유한다.
	// Server Apply_OwnerHits와 Client Valtan의 로컬 전투 오브젝트 preview가 호출한다. 함수는 입력 몸체를 바꾸지 않으며 Y 높이는 판정하지 않는다.
	// 내부 공유 상태나 잠금은 없고 입력 목록과 출력 vector를 호출 중 다른 스레드가 변경하지 않아야 한다. vector 할당 예외는 false로 바꾸지 않는다.
	inline bool Resolve_ConeDelays(const CONE_HIT& hit, const std::vector<CombatCollision::BODY_CIRCLE_XZ>& bodies,
		const std::uint32_t delayMs, std::vector<std::uint32_t>& outDelays)
	{
		outDelays.clear();
		if (bodies.empty() || delayMs == 0u || delayMs > 600000u || !std::isfinite(hit.fOriginX) ||
			!std::isfinite(hit.fOriginZ) || !std::isfinite(hit.fForwardX) || !std::isfinite(hit.fForwardZ) ||
			!std::isfinite(hit.fLength) || !std::isfinite(hit.fAngleDegrees) || hit.fLength <= 0.f ||
			hit.fAngleDegrees <= 0.f || hit.fAngleDegrees > 360.f ||
			hit.fForwardX * hit.fForwardX + hit.fForwardZ * hit.fForwardZ < 0.000001f)
			return false;

		// 각 몸체의 유효성을 먼저 확인해 뒤의 결과 목록에 잘못된 몸체의 지연이 섞이지 않게 한다.
		for (const auto& body : bodies)
			if (!CombatCollision::Is_Valid(body))
				return false;

		// 한 개라도 직접 닿아야 전체 연쇄가 시작된다. 직접 닿은 항목은 즉시, 나머지는 동일한 delayMs 뒤로 분류하며 거리순 전파를 계산하지 않는다.
		bool anyHit = false;
		outDelays.reserve(bodies.size());
		for (const auto& body : bodies)
		{
			const bool intersects = CombatCollision::Circle_IntersectsCone(
				body, hit.fOriginX, hit.fOriginZ, hit.fForwardX, hit.fForwardZ, hit.fLength, hit.fAngleDegrees);
			anyHit = anyHit || intersects;
			outDelays.push_back(intersects ? 0u : delayMs);
		}

		// 접촉이 하나도 없으면 임시로 만든 지연 목록도 비운다. 호출자는 true일 때만 자신의 무장 시각/세대 latch를 반영한다.
		if (!anyHit)
			outDelays.clear();
		return anyHit;
	}
}
