#pragma once

#include "WorldDestructionRuntime.h"

#include <cstdint>
#include <filesystem>
#include <string>

namespace LostArk::Server
{
	// 파괴된 벽에서 떨어질 pickup의 정의다. 시작/착지 좌표와 획득 반지름은 월드 m, 낙하 기간은 30Hz tick 수다.
	// 벽의 현재 파괴 상태와 pickup의 낙하/획득 진행은 GameRoom의 별도 runtime이 소유한다.
	struct WORLD_PICKUP_DESCRIPTOR final
	{
		std::string strPlacementId,
			strWallGroupId;
		float fStartX = 0.f,
			fStartY = 0.f,
			fStartZ = 0.f;
		float fLandingX = 0.f,
			fLandingY = 0.f,
			fLandingZ = 0.f;
		std::uint32_t iFallDurationTicks = 0u;
		float fPickupRadiusM = 0.f;
	};
	// 발탄 World 저작의 게시 pickup 정의를 읽는다. 파일 형식/수치를 먼저 검증하고 성공할 때만 descriptors를 교체한다.
	bool Load_ValtanWorldPickups(std::vector<WORLD_PICKUP_DESCRIPTOR>& descriptors,
		std::string& status);
	// 실제 파일 reader다. false이면 descriptors를 유지하고 status만 갱신한다. 벽 존재/navigation 착지 검증은 GameRoom이 이어서 한다.
	bool Load_WorldPickupsFromFile(const std::filesystem::path& path,
		std::vector<WORLD_PICKUP_DESCRIPTOR>& descriptors, std::string& status);

	// 발탄의 파괴 group/mutation/binding 게시 정의를 소유한다. 현재 벽 상태를 바꾸는 WorldDestructionRuntime과 분리한다.
	// GameRoom이 로드 뒤 runtime 초기화와 충돌/navigation 참조 검증을 수행한다. 내부 잠금은 없으므로 로드/조회 순서는 호출자가 관리한다.
	class CWorldDestructionBootstrap final
	{
	public:
		// Server 데이터 루트에서 발탄의 고정 파일명을 선택한다. 루트 해석이 실패하면 기존 정의를 유지하고 false다.
		bool Load_ValtanArena();
		// 형식/정렬/내부 참조와 semantic SHA-256을 검사한 지역 graph를 성공 시 교체한다. false 반환 경로에서는 기존 graph/metadata를 유지한다.
		// 이 API가 외부 collision ID/navigation condition/pattern action의 존재까지 검사하지는 않는다.
		bool Load_FromFile(const std::filesystem::path& path);

		// Runtime 초기화와 GameRoom 참조 검증이 빌린 const 정의다. 성공한 재로드/객체 파괴 뒤의 내부 요소 참조는 재사용하지 않는다.
		const WORLD_DESTRUCTION_DESCRIPTOR_GRAPH& Get_DescriptorGraph() const
		{
			return m_DescriptorGraph;
		}
		// 아래 identity/revision/status 조회는 객체가 소유한 문자열을 차용한다. 다음 Load에서 바뀔 수 있다.
		const std::string& Get_AreaId() const
		{
			return m_strAreaId;
		}
		const std::string& Get_EncounterId() const
		{
			return m_strEncounterId;
		}
		const std::string& Get_CombatRuntimeRevision() const
		{
			return m_strCombatRuntimeRevision;
		}
		const std::string& Get_Status() const
		{
			return m_strStatus;
		}
		// 게시 기간 값의 tick 기준이다. 현재 로더는 30Hz만 승인하며 초기값 0은 아직 승인된 정의가 없음을 뜻한다.
		std::uint32_t Get_FixedTickHz() const
		{
			return m_iFixedTickHz;
		}

	private:
		// 마지막 성공한 정의와 identity다. 상태 메시지만 실패 때도 갱신하므로 유효 정의와 최근 시도의 성공 여부를 구분한다.
		WORLD_DESTRUCTION_DESCRIPTOR_GRAPH m_DescriptorGraph;
		std::string m_strAreaId;
		std::string m_strEncounterId;
		std::string m_strCombatRuntimeRevision;
		std::string m_strStatus;
		std::uint32_t m_iFixedTickHz = 0u;
	};
}
