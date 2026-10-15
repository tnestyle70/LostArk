// 가이드·보스·Kouku ledger와 게시 패턴을 현재 룸에서 조회한다.
#include "GameRoom.h"
// 룸 tick을 ms로 바꾸는 SERVER_TICK_HZ 상수를 사용한다.
#include "GameRoom_Internal.h"
// 빙고 망치 검사에 실제 player 반범위와 접촉 여유를 반경으로 사용한다.
#include "Gameplay/WorldCollisionContract.h"
// 현재 pattern ID 조회와 망치 진행 비율의 clamp를 사용한다.
#include <algorithm>
// 망치 이동 경로의 XZ 길이를 계산한다.
#include <cmath>

using namespace LostArk::Server;
using namespace LostArk::Shared;

// Update_Guides의 danger 평가가 후보 world XZ 위치(m)의 접촉 위험 점수를 읽는다. guide의 ID·높이를 지역 probe에 복사하며 실제 player 위치/HP는 변경하지 않는다.
// 현재 게시 세대/occurrence owner는 호출 중 차용하고 맞는 패턴이나 owner가 없으면 해당 항목만 0으로 건너뛴다. 반환값은 피해량이나 확률이 아니다.
float CGameRoom::Predict_GuideContactRisk(const SERVER_PLAYER& guide, float x, float z)
{
	SERVER_PLAYER probe;
	probe.iPlayerId = guide.iPlayerId;
	probe.iNetEntityId = guide.iNetEntityId;
	probe.fPositionX = x;
	probe.fPositionY = guide.fPositionY;
	probe.fPositionZ = z;

	// Guide의 예측 초를 패턴 평가기가 받는 ms로 바꾼다. 제품 catalog가 있으면 그것을 사용하고 아니면 현재 활성 gameplay 세대를 조회한다.
	const auto horizon = static_cast<std::uint32_t>(m_GuideCatalog.HorizonSeconds * 1000.f);
	float risk = 0.f;
	const auto* product = Resolve_KoukuProductCatalog();
	const auto* catalog = product ? product : &m_GameplayCatalog.Active();

	// 살아 있고 pattern ID가 있는 world entity의 현재 pattern을 찾아 기존 Valtan 접촉 평가기에 같은 probe를 전달한다.
	for (const auto& boss : m_WorldEntities)
	{
		if (!boss.iCurrentHp || boss.strPatternId.empty())
			continue;
		const auto* patterns = catalog->Find_BossPatterns(boss.strEncounterId);
		if (!patterns)
			continue;
		const auto pattern = std::find_if(patterns->begin(), patterns->end(),
		                                  [&](const auto& row) { return row.strPatternId == boss.strPatternId; });
		if (pattern != patterns->end())
			risk += CValtanBrain::Predict_ContactRisk(boss, *pattern, probe, m_iServerTick, horizon);
	}

	// Kouku Product의 살아 있는 occurrence ledger와 잔류 tail을 함께 평가한다. tail owner는 tail의 소유 포인터를 이 호출 동안만 빌린다.
	if (product)
	{
		const auto sample = [&](const auto& member, const SERVER_WORLD_ENTITY* owner)
		{
			if (!owner || !member.LogicLedger.Is_Active())
				return 0.f;
			std::string status;
			const auto* pattern =
			    CKoukuSaydonBrain::Find_AnimationOnlyPattern(*product, member.LogicLedger.strPatternId, status);
			return pattern ? CKoukuSaydonLogicRuntime::Predict_ContactRisk(*owner, *pattern, member.LogicLedger, probe,
			                                                               m_iServerTick, horizon)
			               : 0.f;
		};
		for (const auto& member : m_KoukuSaydonPatternAudition.Members)
			risk += sample(member, Find_KoukuOccurrenceOwner(member.iBossEntityId, member.iPatternSequence));
		for (const auto& tail : m_KoukuSaydonPatternAudition.Tails)
			risk += sample(tail.Member, tail.pOwner.get());
	}
	// 지속 빙고 망치는 개별 pattern 시계와 별개인 룸 위험이다. 경고+하강 이후 sweep과 예측 구간이 겹치는 망치만 평가한다.
	for (const auto& hammer : m_KoukuBingoDuration.Hammers)
	{
		if (hammer.anchor < 0 || !hammer.startTick)
			continue;
		const float age =
		    static_cast<float>(m_iServerTick - hammer.startTick) * 1000.f / GameRoomDetail::SERVER_TICK_HZ;
		const float begin = float(KOUKU_BINGO_HAMMER_WARNING_MS + KOUKU_BINGO_HAMMER_DESCEND_MS);
		if (age + horizon < begin || age >= begin + KOUKU_BINGO_HAMMER_SWEEP_MS)
			continue;
		const auto path = Kouku_BingoHammerPath(hammer.anchor);
		const float dx = path.fEndX - path.fStartX, dz = path.fEndZ - path.fStartZ;
		const float length = std::hypot(dx, dz);
		if (length <= 0.f)
			continue;

		// 현재/예측 끝 시각을 sweep의 0~1 진행률로 제한한다. 진행한 선분 길이에 망치 앞뒤 반길이를 더해 쓸고 지나갈 world XZ 상자를 만든다.
		const float start = std::clamp((age - begin) / KOUKU_BINGO_HAMMER_SWEEP_MS, 0.f, 1.f);
		const float end = std::clamp((age + horizon - begin) / KOUKU_BINGO_HAMMER_SWEEP_MS, 0.f, 1.f);
		const float fx = dx / length, fz = dz / length;

		// 실제 player 반범위+접촉 여유 반경의 원이 sweep 상자와 겹치면 즉사급 위험5를 더한다. 높이 축의 별도 충돌 검사는 이 상자 판정에 포함되지 않는다.
		if (CombatCollision::Circle_IntersectsForwardBox(
		        {x, z, WorldCollision::PLAYER_HALF_EXTENT_X + WorldCollision::CONTACT_MARGIN},
		        path.fStartX + dx * start - fx * m_KoukuBingoDuration.fHammerHalfForwardM,
		        path.fStartZ + dz * start - fz * m_KoukuBingoDuration.fHammerHalfForwardM, fx, fz,
		        2.f * m_KoukuBingoDuration.fHammerHalfForwardM + length * (end - start),
		        m_KoukuBingoDuration.fHammerHalfWidthM))
			risk += 5.f;
	}
	return risk;
}
