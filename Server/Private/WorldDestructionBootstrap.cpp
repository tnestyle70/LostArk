#include "WorldDestructionBootstrap.h"

#include <Windows.h>
#include <bcrypt.h>

#include <algorithm>
#include <charconv>
#include <cctype>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <limits>
#include <map>
#include <set>
#include <string_view>

#pragma comment(lib, "bcrypt.lib")

namespace
{
	using namespace LostArk::Server;

	// 로드할 group/mutation/binding 수와 그룹별 member 수를 제한한다. 파일 전체 byte 상한을 두는 값은 아니다.
	constexpr std::uint32_t MAX_GROUP_COUNT = 256u;
	constexpr std::uint32_t MAX_MUTATION_COUNT = 256u;
	constexpr std::uint32_t MAX_BINDING_COUNT = 512u;
	constexpr std::uint32_t MAX_MEMBER_COUNT = 4096u;
	constexpr std::string_view EXPECTED_ENCOUNTER_ID = "ENCOUNTER_VALTAN";
	constexpr std::string_view EXPECTED_AREA_ID = "LV_LUT_HEARTRB_ED";

	// LOSTARK_SERVER_DATA_ROOT를 우선하고 없거나 버퍼를 넘으면 실행 파일 기준 DataFiles를 사용한다. 실행 파일 경로 실패는 빈 path다.
	std::filesystem::path Resolve_DataRoot()
	{
		wchar_t configured[32768]{};
		const DWORD configuredLength = GetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", configured,
			static_cast<DWORD>(std::size(configured)));
		if (0u != configuredLength && configuredLength < std::size(configured))
			return std::filesystem::path(configured).lexically_normal();

		wchar_t modulePath[32768]{};
		const DWORD moduleLength = GetModuleFileNameW(
			nullptr, modulePath, static_cast<DWORD>(std::size(modulePath)));
		if (0u == moduleLength || moduleLength >= std::size(modulePath))
			return {};
		return std::filesystem::path(modulePath).parent_path().parent_path() /
			L"DataFiles";
	}

	// getline이 남긴 마지막 CR만 제거하여 revision 계산에 LF 행을 사용한다.
	void StripCarriageReturn(std::string& line)
	{
		if (!line.empty() && '\r' == line.back())
			line.pop_back();
	}

	// 각 view는 line의 메모리를 차용한다. 다음 행을 읽기 전에 descriptor의 소유 문자열로 복사해야 한다.
	std::vector<std::string_view> SplitTabs(const std::string& line)
	{
		std::vector<std::string_view> fields;
		const std::string_view view(line);
		std::size_t start = 0u;
		while (true)
		{
			const std::size_t tab = view.find('\t', start);
			fields.push_back(view.substr(start,
				std::string_view::npos == tab ? tab : tab - start));
			if (std::string_view::npos == tab)
				break;
			start = tab + 1u;
		}
		return fields;
	}

	// 숫자 뒤의 불필요한 글자까지 거부하도록 전체 입력 소비를 요구한다. 실패 시 출력 보존 함수는 아니므로 임시 변수에 읽는다.
	template<typename T>
	bool ParseNumber(const std::string_view value, T& outValue)
	{
		const auto result = std::from_chars(
			value.data(), value.data() + value.size(), outValue);
		return std::errc{} == result.ec &&
			result.ptr == value.data() + value.size();
	}

	// 길이와 alnum/점/밑줄/하이픈 형식을 확인한다. 다른 파일에 같은 ID가 존재하는지는 별도의 admission 검사다.
	bool Is_StableId(const std::string_view value)
	{
		return !value.empty() && value.size() <= 128u &&
			std::all_of(value.begin(), value.end(),
				[](const unsigned char character)
				{
					return 0 != std::isalnum(character) ||
						'.' == character || '_' == character || '-' == character;
				});
	}

	// 소문자 16진수 64글자인지 확인한다. 실제 내용 hash와 일치하는지는 로드 마지막에서 비교한다.
	bool Is_Revision(const std::string_view value)
	{
		return 64u == value.size() &&
			std::all_of(value.begin(), value.end(),
				[](const unsigned char character)
				{
					return ('0' <= character && character <= '9') ||
						('a' <= character && character <= 'f');
				});
	}

	// 초기 group에는 INTACT만, mutation의 도착 상태에는 FRACTURED/DESPAWNED만 읽는다. 실패는 state를 유지한다.
	bool ParseState(
		const std::string_view value,
		WORLD_DESTRUCTION_STATE& state,
		const bool allowIntact)
	{
		if (allowIntact && "INTACT" == value)
			state = WORLD_DESTRUCTION_STATE::INTACT;
		else if (!allowIntact && "FRACTURED" == value)
			state = WORLD_DESTRUCTION_STATE::FRACTURED;
		else if (!allowIntact && "DESPAWNED" == value)
			state = WORLD_DESTRUCTION_STATE::DESPAWNED;
		else
			return false;
		return true;
	}

	// 검증할 semantic payload의 SHA-256을 Windows BCrypt로 계산한다. payload는 호출 중만 차용한다.
	// 일반 API 실패 경로에서 생성된 hash/provider handle을 닫으며, 모든 BCrypt 단계가 성공한 뒤 revision 문자열을 작성한다.
	bool Calculate_Sha256LowerHex(
		const std::string_view payload,
		std::string& revision)
	{
		BCRYPT_ALG_HANDLE algorithm = nullptr;
		BCRYPT_HASH_HANDLE hash = nullptr;
		DWORD hashObjectSize = 0u;
		DWORD bytesWritten = 0u;
		DWORD hashSize = 0u;
		std::vector<unsigned char> hashObject;
		std::vector<unsigned char> digest;
		bool succeeded = false;
		if (0 <= BCryptOpenAlgorithmProvider(
			&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0u) &&
			0 <= BCryptGetProperty(algorithm, BCRYPT_OBJECT_LENGTH,
				reinterpret_cast<PUCHAR>(&hashObjectSize), sizeof(hashObjectSize),
				&bytesWritten, 0u) &&
			0 <= BCryptGetProperty(algorithm, BCRYPT_HASH_LENGTH,
				reinterpret_cast<PUCHAR>(&hashSize), sizeof(hashSize),
				&bytesWritten, 0u))
		{
			hashObject.resize(hashObjectSize);
			digest.resize(hashSize);
			if (0 <= BCryptCreateHash(algorithm, &hash,
				hashObject.data(), hashObjectSize, nullptr, 0u, 0u) &&
				0 <= BCryptHashData(hash,
					reinterpret_cast<PUCHAR>(const_cast<char*>(payload.data())),
					static_cast<ULONG>(payload.size()), 0u) &&
				0 <= BCryptFinishHash(hash, digest.data(), hashSize, 0u))
			{
				constexpr char HEX[] = "0123456789abcdef";
				revision.resize(digest.size() * 2u);
				for (std::size_t index = 0u; index < digest.size(); ++index)
				{
					revision[index * 2u] = HEX[digest[index] >> 4u];
					revision[index * 2u + 1u] = HEX[digest[index] & 0x0fu];
				}
				succeeded = true;
			}
		}
		// BCrypt 일반 실패 여부와 관계없이 확보된 두 handle을 닫는다.
		if (nullptr != hash)
			BCryptDestroyHash(hash);
		if (nullptr != algorithm)
			BCryptCloseAlgorithmProvider(algorithm, 0u);
		return succeeded;
	}
}

// GameRoom의 발탄 초기화가 사용하는 표준 진입점이다. 파일 내용을 직접 다루는 책임은 Load_FromFile에 모은다.
bool LostArk::Server::CWorldDestructionBootstrap::Load_ValtanArena()
{
	const std::filesystem::path root = Resolve_DataRoot();
	if (root.empty())
	{
		m_strStatus = "Could not resolve server data root";
		return false;
	}
	return Load_FromFile(root / L"World" /
		L"VALTAN_ARENA.worlddestructionbootstrap");
}

// GameRoom 초기화와 계약 테스트가 사용한다. group→mutation→binding 순서로 지역 graph를 만들고 끝에서 교체한다.
// false는 기존 정의와 identity를 보존하고 상태 문자열만 갱신한다. 문자열 할당 예외를 포괄적으로 복구하는 함수는 아니다.
bool LostArk::Server::CWorldDestructionBootstrap::Load_FromFile(
	const std::filesystem::path& path)
{
	std::ifstream input(path, std::ios::binary);
	if (!input)
	{
		m_strStatus = "Missing world destruction bootstrap: " + path.string();
		return false;
	}

	std::string line;
	if (!std::getline(input, line))
	{
		m_strStatus = "World destruction bootstrap is empty";
		return false;
	}
	StripCarriageReturn(line);
	const std::vector<std::string_view> header = SplitTabs(line);
	std::uint32_t version = 0u;
	std::uint32_t fixedTickHz = 0u;
	std::uint32_t groupCount = 0u;
	std::uint32_t mutationCount = 0u;
	std::uint32_t bindingCount = 0u;
	if (9u != header.size() ||
		"LOSTARK_WORLD_DESTRUCTION_BOOTSTRAP" != header[0] ||
		!ParseNumber(header[1], version) || 2u != version ||
		!Is_StableId(header[2]) || !Is_StableId(header[3]) ||
		!Is_Revision(header[4]) ||
		!ParseNumber(header[5], fixedTickHz) || 30u != fixedTickHz ||
		!ParseNumber(header[6], groupCount) || groupCount > MAX_GROUP_COUNT ||
		!ParseNumber(header[7], mutationCount) ||
		mutationCount > MAX_MUTATION_COUNT ||
		!ParseNumber(header[8], bindingCount) ||
		bindingCount > MAX_BINDING_COUNT ||
		((0u == groupCount) != (0u == mutationCount)) ||
		((0u == groupCount) != (0u == bindingCount)))
	{
		m_strStatus = "World destruction bootstrap header is invalid";
		return false;
	}
	// header view가 다음 getline에서 무효화되기 전에 identity와 예상 revision을 소유 문자열로 복사한다.
	const std::string stagedEncounterId(header[2]);
	const std::string stagedAreaId(header[3]);
	const std::string stagedRevision(header[4]);
	if (EXPECTED_ENCOUNTER_ID != stagedEncounterId ||
		EXPECTED_AREA_ID != stagedAreaId)
	{
		m_strStatus = "World destruction bootstrap identity is invalid";
		return false;
	}
	// revision은 파일 header 전체가 아니라 IDENTITY 행과 CR을 제거한 G/M/B 행을 LF로 연결한 내용에서 계산한다.
	std::string semanticPayload = "IDENTITY\t" + stagedEncounterId + "\t" +
		stagedAreaId + "\t" + std::to_string(fixedTickHz) + "\n";

	WORLD_DESTRUCTION_DESCRIPTOR_GRAPH stagedGraph;
	stagedGraph.Groups.reserve(groupCount);
	stagedGraph.Mutations.reserve(mutationCount);
	stagedGraph.Bindings.reserve(bindingCount);
	// 각 종류의 중복 ID와 참조를 검사한다. vector는 게시 순서를 보존하고 set은 이미 읽은 정의의 존재를 판별한다.
	std::set<std::string> groupIds;
	std::set<std::string> mutationIds;
	std::set<std::string> bindingIds;
	// placement ID 문자열은 전체 graph에서 한 번만 소유한다. 그룹 내부 순서는 uint64로 해석한 값의 엄격한 증가를 요구한다.
	std::set<std::string> ownedMemberIds;
	std::string previousGroupId;
	std::string previousMutationId;
	std::string previousBindingId;
	// 초기 상태와 member 목록을 읽는다. group ID는 문자열 증가 순서이며 member는 숫자 증가 순서를 따로 검사한다.
	for (std::uint32_t index = 0u; index < groupCount; ++index)
	{
		if (!std::getline(input, line))
		{
			m_strStatus = "World destruction group rows are truncated";
			return false;
		}
		StripCarriageReturn(line);
		semanticPayload += line + "\n";
		const std::vector<std::string_view> fields = SplitTabs(line);
		std::uint32_t memberCount = 0u;
		WORLD_DESTRUCTION_GROUP_DESCRIPTOR group;
		if (fields.size() < 4u || "G" != fields[0] ||
			!Is_StableId(fields[1]) ||
			!ParseState(fields[2], group.eInitialState, true) ||
			!ParseNumber(fields[3], memberCount) || 0u == memberCount ||
			memberCount > MAX_MEMBER_COUNT || fields.size() != 4u + memberCount)
		{
			m_strStatus = "World destruction group row is invalid";
			return false;
		}
		group.strGroupId = fields[1];
		if ((!previousGroupId.empty() &&
			previousGroupId >= group.strGroupId) ||
			!groupIds.emplace(group.strGroupId).second)
		{
			m_strStatus = "World destruction group order or identity is invalid";
			return false;
		}
		previousGroupId = group.strGroupId;
		group.MemberPlacementIds.reserve(memberCount);
		std::uint64_t previousMemberId = 0u;
		for (std::uint32_t memberIndex = 0u;
			memberIndex < memberCount;
			++memberIndex)
		{
			const std::string_view member = fields[4u + memberIndex];
			std::uint64_t numericMemberId = 0u;
			if (!Is_StableId(member) ||
				!ParseNumber(member, numericMemberId) || 0u == numericMemberId ||
				(0u != memberIndex && previousMemberId >= numericMemberId) ||
				!ownedMemberIds.emplace(member).second)
			{
				m_strStatus = "World destruction placement ownership is invalid";
				return false;
			}
			previousMemberId = numericMemberId;
			group.MemberPlacementIds.emplace_back(member);
		}
		stagedGraph.Groups.emplace_back(std::move(group));
	}

	// 앞서 읽은 group만 참조할 수 있다. 파괴 기간의 INT32_MAX 상한은 runtime의 부호 있는 tick 차 비교 범위와 연결된다.
	for (std::uint32_t index = 0u; index < mutationCount; ++index)
	{
		if (!std::getline(input, line))
		{
			m_strStatus = "World destruction mutation rows are truncated";
			return false;
		}
		StripCarriageReturn(line);
		semanticPayload += line + "\n";
		const std::vector<std::string_view> fields = SplitTabs(line);
		WORLD_DESTRUCTION_MUTATION_DESCRIPTOR mutation;
		std::uint32_t removesGround = 0u;
		if (8u != fields.size() || "M" != fields[0] ||
			!Is_StableId(fields[1]) || !Is_StableId(fields[2]) ||
			!ParseState(fields[3], mutation.eFinalState, false) ||
			!ParseNumber(fields[4], mutation.iBreakingDurationTicks) ||
			mutation.iBreakingDurationTicks >
				static_cast<std::uint32_t>((std::numeric_limits<std::int32_t>::max)()) ||
			("-" != fields[5] && !Is_StableId(fields[5])) ||
			("-" != fields[6] && !Is_StableId(fields[6])) ||
			!ParseNumber(fields[7], removesGround) || 1u < removesGround ||
			// 바닥을 제거하는 mutation은 실제 void로 만들 navigation condition ID를 반드시 연결해야 한다.
			(0u != removesGround && "-" == fields[6]))
		{
			m_strStatus = "World destruction mutation row is invalid";
			return false;
		}
		mutation.strMutationId = fields[1];
		mutation.strGroupId = fields[2];
		if ((!previousMutationId.empty() &&
			previousMutationId >= mutation.strMutationId) ||
			!mutationIds.emplace(mutation.strMutationId).second ||
			!groupIds.contains(mutation.strGroupId))
		{
			m_strStatus = "World destruction mutation reference is invalid";
			return false;
		}
		previousMutationId = mutation.strMutationId;
		if ("-" != fields[5])
			mutation.strCollisionStateId = fields[5];
		if ("-" != fields[6])
			mutation.strNavigationStateId = fields[6];
		mutation.bRemovesGround = 0u != removesGround;
		stagedGraph.Mutations.emplace_back(std::move(mutation));
	}

	// 어떤 사건이 어떤 mutation을 적용할지 읽는다. stage 사건과 receiver 접촉 사건의 필수 열을 구분한다.
	for (std::uint32_t index = 0u; index < bindingCount; ++index)
	{
		if (!std::getline(input, line))
		{
			m_strStatus = "World destruction binding rows are truncated";
			return false;
		}
		StripCarriageReturn(line);
		semanticPayload += line + "\n";
		const std::vector<std::string_view> fields = SplitTabs(line);
		WORLD_DESTRUCTION_BINDING_DESCRIPTOR binding;
		// 도형 접촉 binding은 시간표 action tuple을 사용하지 않으므로 pattern/stage/action에는 빈 표식, stage index에는 0을 요구한다.
		// 현재 코드는 열 수 검사보다 먼저 fields[3..7]을 참조한다. 잘린 열 입력을 모두 false로 거부한다는 보장은 이 순서에 없다.
		const bool isContactRow = "COLLIDER_CONTACT" == fields[3];
		const bool hasActionColumns = isContactRow ?
			("-" == fields[4] && "-" == fields[5] && "-" == fields[6] &&
				"0" == fields[7]) :
			(Is_StableId(fields[4]) && Is_StableId(fields[5]) &&
				Is_StableId(fields[6]));
		if (9u != fields.size() || "B" != fields[0] ||
			!Is_StableId(fields[1]) || !Is_StableId(fields[2]) ||
			!hasActionColumns ||
			!ParseNumber(fields[7], binding.iStageIndex) ||
			("-" != fields[8] && !Is_StableId(fields[8])))
		{
			m_strStatus = "World destruction binding row is invalid";
			return false;
		}
		if ("STAGE" == fields[3])
			binding.eTriggerKind = WORLD_DESTRUCTION_TRIGGER_KIND::STAGE;
		else if ("BOSS_IMPACT" == fields[3])
			binding.eTriggerKind = WORLD_DESTRUCTION_TRIGGER_KIND::BOSS_IMPACT;
		else if (isContactRow)
		{
			binding.eTriggerKind =
				WORLD_DESTRUCTION_TRIGGER_KIND::COLLIDER_CONTACT;
		}
		else
		{
			m_strStatus = "World destruction trigger kind is invalid";
			return false;
		}
		binding.strBindingId = fields[1];
		binding.strMutationId = fields[2];
		if (!isContactRow)
		{
			binding.strPatternId = fields[4];
			binding.strStageId = fields[5];
			binding.strActionId = fields[6];
		}
		if ("-" != fields[8])
			binding.strImpactReceiverId = fields[8];
		if ((!previousBindingId.empty() &&
			previousBindingId >= binding.strBindingId) ||
			!bindingIds.emplace(binding.strBindingId).second ||
			!mutationIds.contains(binding.strMutationId) ||
			(WORLD_DESTRUCTION_TRIGGER_KIND::STAGE == binding.eTriggerKind &&
				!binding.strImpactReceiverId.empty()) ||
			(WORLD_DESTRUCTION_TRIGGER_KIND::STAGE != binding.eTriggerKind &&
				binding.strImpactReceiverId.empty()))
		{
			m_strStatus = "World destruction binding reference is invalid";
			return false;
		}
		previousBindingId = binding.strBindingId;
		stagedGraph.Bindings.emplace_back(std::move(binding));
	}

	if (std::getline(input, line))
	{
		m_strStatus = "World destruction bootstrap has trailing rows";
		return false;
	}
	// 모든 행을 읽은 뒤 정규 payload의 hash가 header와 같은지 확인한다. 정의를 파싱했다는 사실만으로 승인하지 않는다.
	std::string calculatedRevision;
	if (!Calculate_Sha256LowerHex(semanticPayload, calculatedRevision) ||
		calculatedRevision != stagedRevision)
	{
		m_strStatus = "World destruction bootstrap revision is invalid";
		return false;
	}

	// 명시적인 실패 반환이 모두 끝난 지점이다. graph와 metadata를 교체하며 이후 GameRoom이 외부 상태 ID를 검증한다.
	std::swap(m_DescriptorGraph, stagedGraph);
	m_strAreaId = stagedAreaId;
	m_strEncounterId = stagedEncounterId;
	m_strCombatRuntimeRevision = stagedRevision;
	m_iFixedTickHz = fixedTickHz;
	m_strStatus = groupCount ?
		"World destruction bootstrap loaded" :
		"Dormant world destruction bootstrap loaded";
	return true;
}


// GameRoom::Initialize_WorldPickups가 사용하는 표준 파일 경로다. 데이터 루트 선택 뒤 실제 검증/출력 교체는 아래 reader에 맡긴다.
bool LostArk::Server::Load_ValtanWorldPickups(
	std::vector<WORLD_PICKUP_DESCRIPTOR>& descriptors, std::string& status)
{
	return Load_WorldPickupsFromFile(Resolve_DataRoot() / L"World" /
		L"VALTAN_ARENA.worldpickupsbootstrap", descriptors, status);
}

// quoted ID와 낙하 수치를 지역 목록으로 읽는다. 0~16개를 허용하고 성공할 때만 호출자의 descriptors를 교체한다.
// 벽 ID의 존재나 착지 지형은 이 파일만으로 확인하지 않으며 GameRoom 초기화가 파괴 runtime/navigation과 대조한다.
bool LostArk::Server::Load_WorldPickupsFromFile(const std::filesystem::path& path,
	std::vector<WORLD_PICKUP_DESCRIPTOR>& descriptors, std::string& status)
{
	std::ifstream input(path, std::ios::binary);
	std::string line,
		magic,
		area;
	std::uint32_t version = 0u,
		tickHz = 0u,
		count = 0u;
	if (!input || !std::getline(input, line))
	{
		status = "Missing world pickups bootstrap: " + path.string();
		return false;
	}
	std::istringstream header(line);
	if (!(header >> magic >> version >> std::quoted(area) >> tickHz >> count) ||
		!(header >> std::ws).eof() ||
		magic != "LOSTARK_WORLD_PICKUPS" || version != 1u ||
		area != EXPECTED_AREA_ID || tickHz != 30u || count > 16u)
	{
		status = "Invalid world pickups bootstrap header";
		return false;
	}
	std::vector<WORLD_PICKUP_DESCRIPTOR> staged;
	std::set<std::string> ids;
	// 시작/착지 월드 좌표를 finite이고 절댓값 100000m 이하로 제한한다. navigation의 도달 가능성과는 별도 검사다.
	const auto finitePosition = [](const float value)
	{
		return std::isfinite(value) && std::fabs(value) <= 100000.f;
	};
	for (std::uint32_t index = 0u; index < count; ++index)
	{
		WORLD_PICKUP_DESCRIPTOR row{};
		if (!std::getline(input, line))
		{
			status = "World pickup row count is short";
			return false;
		}
		std::istringstream fields(line);
		if (!(fields >> std::quoted(row.strPlacementId) >> std::quoted(row.strWallGroupId) >>
			row.fStartX >> row.fStartY >> row.fStartZ >> row.fLandingX >> row.fLandingY >>
			row.fLandingZ >> row.iFallDurationTicks >> row.fPickupRadiusM) ||
			!(fields >> std::ws).eof() || !Is_StableId(row.strPlacementId) ||
			row.strWallGroupId.empty() || row.strWallGroupId.size() > 160u ||
			!std::all_of(row.strWallGroupId.begin(), row.strWallGroupId.end(),
				[](const unsigned char c)
				{
					return std::isalnum(c) || c == '.' || c == '_' || c == '-';
				}) ||
			!ids.insert(row.strPlacementId).second ||
			!finitePosition(row.fStartX) ||
			!finitePosition(row.fStartY) || !finitePosition(row.fStartZ) ||
			!finitePosition(row.fLandingX) ||
			!finitePosition(row.fLandingY) || !finitePosition(row.fLandingZ) ||
			row.fStartY - row.fLandingY < 0.01f ||
			row.iFallDurationTicks == 0u || row.iFallDurationTicks > 1800u ||
			!std::isfinite(row.fPickupRadiusM) || row.fPickupRadiusM < 0.001f || row.fPickupRadiusM > 20.f)
		{
			status = "Invalid world pickup row";
			return false;
		}
		staged.push_back(std::move(row));
	}
	if (!(input >> std::ws).eof())
	{
		status = "Unexpected trailing world pickup row";
		return false;
	}
	// 성공한 목록의 순서를 placement ID 문자열 순으로 고정한다. 인덱스를 저장 identity로 사용하는 계약은 아니다.
	std::sort(staged.begin(), staged.end(), [](const auto& a, const auto& b)
	{
		return a.strPlacementId < b.strPlacementId;
	});
	descriptors = std::move(staged);
	status = "World pickups bootstrap loaded";
	return true;
}
