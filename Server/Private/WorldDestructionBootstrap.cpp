#include "WorldDestructionBootstrap.h"

#include <Windows.h>
#include <bcrypt.h>

#include <algorithm>
#include <charconv>
#include <cctype>
#include <fstream>
#include <limits>
#include <map>
#include <set>
#include <string_view>

#pragma comment(lib, "bcrypt.lib")

namespace
{
	using namespace LostArk::Server;

	constexpr std::uint32_t MAX_GROUP_COUNT = 256u;
	constexpr std::uint32_t MAX_MUTATION_COUNT = 256u;
	constexpr std::uint32_t MAX_BINDING_COUNT = 512u;
	constexpr std::uint32_t MAX_MEMBER_COUNT = 4096u;
	constexpr std::string_view EXPECTED_ENCOUNTER_ID = "ENCOUNTER_VALTAN";
	constexpr std::string_view EXPECTED_AREA_ID = "LV_LUT_HEARTRB_ED";

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

	void StripCarriageReturn(std::string& line)
	{
		if (!line.empty() && '\r' == line.back())
			line.pop_back();
	}

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

	template<typename T>
	bool ParseNumber(const std::string_view value, T& outValue)
	{
		const auto result = std::from_chars(
			value.data(), value.data() + value.size(), outValue);
		return std::errc{} == result.ec &&
			result.ptr == value.data() + value.size();
	}

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
		if (nullptr != hash) BCryptDestroyHash(hash);
		if (nullptr != algorithm) BCryptCloseAlgorithmProvider(algorithm, 0u);
		return succeeded;
	}
}

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
	const std::string stagedEncounterId(header[2]);
	const std::string stagedAreaId(header[3]);
	const std::string stagedRevision(header[4]);
	if (EXPECTED_ENCOUNTER_ID != stagedEncounterId ||
		EXPECTED_AREA_ID != stagedAreaId)
	{
		m_strStatus = "World destruction bootstrap identity is invalid";
		return false;
	}
	std::string semanticPayload = "IDENTITY\t" + stagedEncounterId + "\t" +
		stagedAreaId + "\t" + std::to_string(fixedTickHz) + "\n";

	WORLD_DESTRUCTION_DESCRIPTOR_GRAPH stagedGraph;
	stagedGraph.Groups.reserve(groupCount);
	stagedGraph.Mutations.reserve(mutationCount);
	stagedGraph.Bindings.reserve(bindingCount);
	std::set<std::string> groupIds;
	std::set<std::string> mutationIds;
	std::set<std::string> bindingIds;
	std::set<std::string> ownedMemberIds;
	std::string previousGroupId;
	std::string previousMutationId;
	std::string previousBindingId;
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
			memberIndex < memberCount; ++memberIndex)
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
			/* A hole is the navigation condition's cells. A mutation that claims
			to remove ground without owning one has nothing to open. */
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
		if ("-" != fields[5]) mutation.strCollisionStateId = fields[5];
		if ("-" != fields[6]) mutation.strNavigationStateId = fields[6];
		mutation.bRemovesGround = 0u != removesGround;
		stagedGraph.Mutations.emplace_back(std::move(mutation));
	}

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
		/* A contact binding answers geometry, so its pattern, stage and action
		columns carry the empty marker instead of a schedule to match. */
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
		if ("-" != fields[8]) binding.strImpactReceiverId = fields[8];
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
	std::string calculatedRevision;
	if (!Calculate_Sha256LowerHex(semanticPayload, calculatedRevision) ||
		calculatedRevision != stagedRevision)
	{
		m_strStatus = "World destruction bootstrap revision is invalid";
		return false;
	}

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
