#include "ValtanPatternFlowDocument.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <Windows.h>
#include <bcrypt.h>
#include <io.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

#pragma comment(lib, "bcrypt.lib")

namespace
{
	using namespace Client;

	constexpr std::string_view DOCUMENT_SCHEMA =
		"lostark.valtan-boss-audition-flows";
	constexpr std::uint32_t DOCUMENT_VERSION = 2u;
	constexpr std::size_t MAX_DOCUMENT_BYTES = 256u * 1024u;
	constexpr std::size_t MAX_STABLE_ID_LENGTH = 128u;
	constexpr std::uint64_t MAX_NODE_ORDINAL = 999999u;
	constexpr std::uint64_t MAX_EDGE_ORDINAL = 999999u;
	constexpr std::size_t SHA256_BYTE_COUNT = 32u;
	constexpr const wchar_t* VALTAN_PATTERN_TRANSACTION_LOCK_RELATIVE =
		L"out\\ValtanPatternTransactions\\create-pattern.lock";
	constexpr std::string_view OPTIONAL_ENTRY_PATTERN_ID =
		"VALTAN_ENTRANCE_CINEMATIC";

	/* Flow authoring participates in the same one-byte writer admission as
	   Create Pattern and every Product projector.  The lock is held from the
	   first source-revision read through post-replace verification/rollback so
	   no second writer can be overwritten by this document's recovery path. */
	class SCOPED_VALTAN_PATTERN_FLOW_WRITE_LOCK final
	{
	public:
		~SCOPED_VALTAN_PATTERN_FLOW_WRITE_LOCK()
		{
			if (INVALID_HANDLE_VALUE == m_hFile)
				return;
			UnlockFileEx(m_hFile, 0u, 1u, 0u, &m_Overlap);
			CloseHandle(m_hFile);
		}

		bool_t Try_Acquire(
			const std::filesystem::path& projectRoot,
			std::string& outStatus)
		{
			if (projectRoot.empty())
			{
				outStatus = "project root is unavailable";
				return false;
			}
			const std::filesystem::path lockPath =
				projectRoot / VALTAN_PATTERN_TRANSACTION_LOCK_RELATIVE;
			std::error_code directoryError;
			std::filesystem::create_directories(
				lockPath.parent_path(), directoryError);
			if (directoryError)
			{
				outStatus = "lock directory creation failed: " +
					directoryError.message();
				return false;
			}

			m_hFile = CreateFileW(
				lockPath.c_str(), GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
				nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (INVALID_HANDLE_VALUE == m_hFile)
			{
				outStatus = "lock open failed with Win32 error " +
					std::to_string(GetLastError());
				return false;
			}

			LARGE_INTEGER size{};
			if (FALSE == GetFileSizeEx(m_hFile, &size))
			{
				outStatus = "lock size query failed with Win32 error " +
					std::to_string(GetLastError());
				CloseHandle(m_hFile);
				m_hFile = INVALID_HANDLE_VALUE;
				return false;
			}
			if (size.QuadPart < 1)
			{
				const char byte = '\0';
				DWORD written = 0u;
				LARGE_INTEGER begin{};
				if (FALSE == SetFilePointerEx(
						m_hFile, begin, nullptr, FILE_BEGIN) ||
					FALSE == WriteFile(
						m_hFile, &byte, 1u, &written, nullptr) ||
					1u != written || FALSE == FlushFileBuffers(m_hFile))
				{
					outStatus = "lock initialization failed with Win32 error " +
						std::to_string(GetLastError());
					CloseHandle(m_hFile);
					m_hFile = INVALID_HANDLE_VALUE;
					return false;
				}
			}

			if (FALSE == LockFileEx(
				m_hFile,
				LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY,
				0u, 1u, 0u, &m_Overlap))
			{
				outStatus = "another canonical writer owns the lock (Win32 " +
					std::to_string(GetLastError()) + ")";
				CloseHandle(m_hFile);
				m_hFile = INVALID_HANDLE_VALUE;
				return false;
			}
			return true;
		}

	private:
		HANDLE m_hFile = INVALID_HANDLE_VALUE;
		OVERLAPPED m_Overlap{};
	};

	const DATA_JSON_VALUE* Required(
		const DATA_JSON_VALUE& object,
		const char* name,
		const DATA_JSON_TYPE type)
	{
		const DATA_JSON_VALUE* value = object.Find(name);
		return nullptr != value && value->Get_Type() == type ? value : nullptr;
	}

	bool_t Has_ExactProperties(
		const DATA_JSON_VALUE& object,
		const std::initializer_list<std::string_view> names)
	{
		if (!object.Is_Object() || object.Get_Object().size() != names.size())
			return false;
		for (const std::string_view name : names)
		{
			if (nullptr == object.Find(name))
				return false;
		}
		return true;
	}

	bool_t Is_StableId(const std::string_view value)
	{
		if (value.empty() || value.size() > MAX_STABLE_ID_LENGTH)
			return false;
		for (const unsigned char character : value)
		{
			if (!(character >= 'a' && character <= 'z') &&
				!(character >= 'A' && character <= 'Z') &&
				!(character >= '0' && character <= '9') &&
				character != '_' && character != '-' && character != '.')
			{
				return false;
			}
		}
		return true;
	}

	template <typename T>
	bool_t Try_ParseUnsignedInteger(
		const DATA_JSON_VALUE& value,
		const std::uint64_t maximum,
		T& outValue)
	{
		if (!value.Is_Number() || value.Was_FloatingPointToken())
			return false;
		const double number = value.Get_Number();
		if (!std::isfinite(number) || number < 0.0 ||
			number > static_cast<double>(maximum) ||
			std::floor(number) != number)
		{
			return false;
		}
		outValue = static_cast<T>(number);
		return true;
	}

	bool_t Try_ParseOrdinal(
		const std::string_view flowId,
		const std::string_view stableId,
		const std::initializer_list<std::string_view> kinds,
		const std::uint64_t maximum,
		std::uint64_t& outOrdinal)
	{
		std::size_t ordinalOffset = std::string_view::npos;
		for (const std::string_view kind : kinds)
		{
			const std::string prefix =
				std::string(flowId) + "." + std::string(kind) + ".";
			if (stableId.starts_with(prefix) &&
				stableId.size() == prefix.size() + 6u)
			{
				ordinalOffset = prefix.size();
				break;
			}
		}
		if (std::string_view::npos == ordinalOffset)
		{
			return false;
		}
		std::uint64_t ordinal = 0u;
		for (std::size_t index = ordinalOffset; index < stableId.size(); ++index)
		{
			const char character = stableId[index];
			if (character < '0' || character > '9')
				return false;
			ordinal = ordinal * 10u +
				static_cast<std::uint64_t>(character - '0');
		}
		if (0u == ordinal || ordinal > maximum)
			return false;
		outOrdinal = ordinal;
		return true;
	}

	std::string Build_OrdinalId(
		const std::string_view flowId,
		const std::string_view kind,
		const std::uint64_t ordinal)
	{
		std::ostringstream output;
		output << flowId << "." << kind << "." << std::setw(6) << std::setfill('0')
			<< ordinal;
		return output.str();
	}

	const VALTAN_PATTERN_FLOW_NODE* Find_Node(
		const VALTAN_PATTERN_FLOW_DEFINITION& flow,
		const std::string_view nodeId)
	{
		const auto found = std::find_if(
			flow.Nodes.begin(), flow.Nodes.end(),
			[nodeId](const VALTAN_PATTERN_FLOW_NODE& node)
			{
				return node.strNodeId == nodeId;
			});
		return flow.Nodes.end() == found ? nullptr : &*found;
	}

	const VALTAN_PATTERN_FLOW_EDGE* Find_CompletedEdge(
		const VALTAN_PATTERN_FLOW_DEFINITION& flow,
		const std::string_view fromNodeId)
	{
		const auto found = std::find_if(
			flow.Edges.begin(), flow.Edges.end(),
			[fromNodeId](const VALTAN_PATTERN_FLOW_EDGE& edge)
			{
				return edge.strFromNodeId == fromNodeId &&
					edge.eOutcome ==
						VALTAN_PATTERN_FLOW_EDGE_OUTCOME::COMPLETED;
			});
		return flow.Edges.end() == found ? nullptr : &*found;
	}

	bool_t Build_LegacyProjection(VALTAN_PATTERN_FLOW_DEFINITION& flow)
	{
		flow.iNextSlotOrdinal = flow.iNextNodeOrdinal;
		flow.iInterStepPursuitMs = flow.iDefaultPursuitMs;
		flow.Slots.clear();
		if (flow.strEntryNodeId.empty() || flow.Nodes.empty())
			return false;

		std::unordered_set<std::string> visited;
		std::string currentNodeId = flow.strEntryNodeId;
		while (true)
		{
			const VALTAN_PATTERN_FLOW_NODE* node =
				Find_Node(flow, currentNodeId);
			if (nullptr == node || !visited.insert(currentNodeId).second)
			{
				flow.Slots.clear();
				return false;
			}
			if (0u != node->iWatchdogMs)
			{
				flow.Slots.clear();
				return false;
			}
			flow.Slots.push_back({ node->strNodeId, node->strPatternId });
			const VALTAN_PATTERN_FLOW_EDGE* edge =
				Find_CompletedEdge(flow, currentNodeId);
			if (nullptr == edge)
				break;
			if (edge->iMaxTraversals.has_value() ||
				edge->iPursuitMs != flow.iDefaultPursuitMs)
			{
				flow.Slots.clear();
				return false;
			}
			currentNodeId = edge->strToNodeId;
		}
		if (visited.size() != flow.Nodes.size())
		{
			flow.Slots.clear();
			return false;
		}
		return true;
	}

	std::vector<std::string> Build_CurrentPatternInventory(
		const VALTAN_PATTERN_FLOW_DEFINITION& flow)
	{
		std::vector<std::string> patternIds;
		std::unordered_set<std::string> seen;
		for (const VALTAN_PATTERN_FLOW_NODE& node : flow.Nodes)
		{
			if (seen.insert(node.strPatternId).second)
				patternIds.push_back(node.strPatternId);
		}
		return patternIds;
	}

	bool_t Rebuild_LinearFlow(
		VALTAN_PATTERN_FLOW_DEFINITION& flow,
		const std::vector<std::string>& orderedNodeIds,
		std::string& outStatus)
	{
		if (orderedNodeIds.empty() ||
			orderedNodeIds.size() != flow.Nodes.size())
		{
			outStatus = "The compatibility Flow must retain every node.";
			return false;
		}

		std::unordered_map<std::string, VALTAN_PATTERN_FLOW_NODE> nodes;
		for (const VALTAN_PATTERN_FLOW_NODE& node : flow.Nodes)
		{
			if (!nodes.emplace(node.strNodeId, node).second)
			{
				outStatus = "The compatibility Flow contains duplicate nodes.";
				return false;
			}
		}
		std::unordered_map<std::string, VALTAN_PATTERN_FLOW_EDGE> oldOutgoing;
		for (const VALTAN_PATTERN_FLOW_EDGE& edge : flow.Edges)
			oldOutgoing.emplace(edge.strFromNodeId, edge);

		std::vector<VALTAN_PATTERN_FLOW_NODE> orderedNodes;
		orderedNodes.reserve(orderedNodeIds.size());
		for (const std::string& nodeId : orderedNodeIds)
		{
			const auto found = nodes.find(nodeId);
			if (nodes.end() == found)
			{
				outStatus = "The compatibility Flow references a missing node.";
				return false;
			}
			orderedNodes.push_back(found->second);
		}

		std::vector<VALTAN_PATTERN_FLOW_EDGE> edges;
		edges.reserve(orderedNodes.size() - 1u);
		for (std::size_t index = 0u; index + 1u < orderedNodes.size(); ++index)
		{
			const std::string& fromNodeId = orderedNodes[index].strNodeId;
			VALTAN_PATTERN_FLOW_EDGE edge;
			const auto oldEdge = oldOutgoing.find(fromNodeId);
			if (oldOutgoing.end() != oldEdge)
			{
				edge = oldEdge->second;
			}
			else
			{
				if (flow.iNextEdgeOrdinal > MAX_EDGE_ORDINAL)
				{
					outStatus =
						"Valtan Boss Flow exhausted its stable edge ID range.";
					return false;
				}
				edge.strEdgeId = Build_OrdinalId(
					flow.strFlowId, "edge", flow.iNextEdgeOrdinal++);
			}
			edge.strFromNodeId = fromNodeId;
			edge.eOutcome = VALTAN_PATTERN_FLOW_EDGE_OUTCOME::COMPLETED;
			edge.strToNodeId = orderedNodes[index + 1u].strNodeId;
			edge.iPursuitMs = flow.iDefaultPursuitMs;
			edge.iMaxTraversals.reset();
			edges.push_back(std::move(edge));
		}

		flow.strEntryNodeId = orderedNodes.front().strNodeId;
		flow.Nodes = std::move(orderedNodes);
		flow.Edges = std::move(edges);
		if (!Build_LegacyProjection(flow))
		{
			outStatus = "Could not rebuild the ordered compatibility Flow.";
			return false;
		}
		return true;
	}

	bool_t Read_Bytes(
		const std::filesystem::path& path,
		std::string& outBytes,
		std::string& outStatus)
	{
		std::ifstream input(path, std::ios::binary);
		if (path.empty() || !input)
		{
			outStatus = "Valtan Boss Flow document is missing: " + path.string();
			return false;
		}
		std::string staged{
			std::istreambuf_iterator<char>(input),
			std::istreambuf_iterator<char>() };
		if (input.bad() || staged.empty() ||
			staged.size() > MAX_DOCUMENT_BYTES)
		{
			outStatus = "Valtan Boss Flow document is empty, oversized, or unreadable.";
			return false;
		}
		outBytes = std::move(staged);
		return true;
	}

	bool_t Compute_Sha256(
		const std::string_view bytes,
		std::array<std::uint8_t, SHA256_BYTE_COUNT>& outDigest)
	{
		if (bytes.size() > static_cast<std::size_t>(ULONG_MAX))
			return false;

		BCRYPT_ALG_HANDLE algorithm = nullptr;
		BCRYPT_HASH_HANDLE hash = nullptr;
		DWORD objectSize = 0u;
		DWORD resultSize = 0u;
		std::vector<std::uint8_t> object;
		bool_t succeeded = false;
		if (BCryptOpenAlgorithmProvider(
				&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0u) < 0)
		{
			goto cleanup;
		}
		if (BCryptGetProperty(
				algorithm, BCRYPT_OBJECT_LENGTH,
				reinterpret_cast<PUCHAR>(&objectSize), sizeof(objectSize),
				&resultSize, 0u) < 0 ||
			resultSize != sizeof(objectSize) || 0u == objectSize)
		{
			goto cleanup;
		}
		object.resize(objectSize);
		if (BCryptCreateHash(
				algorithm, &hash, object.data(), objectSize,
				nullptr, 0u, 0u) < 0)
		{
			goto cleanup;
		}
		if (BCryptHashData(
				hash,
				reinterpret_cast<PUCHAR>(
					const_cast<char*>(bytes.data())),
				static_cast<ULONG>(bytes.size()), 0u) < 0 ||
			BCryptFinishHash(
				hash, outDigest.data(),
				static_cast<ULONG>(outDigest.size()), 0u) < 0)
		{
			goto cleanup;
		}
		succeeded = true;

	cleanup:
		if (nullptr != hash)
			BCryptDestroyHash(hash);
		if (nullptr != algorithm)
			BCryptCloseAlgorithmProvider(algorithm, 0u);
		return succeeded;
	}

	bool_t Build_Revision(
		const std::string_view bytes,
		std::string& outRevision)
	{
		std::array<std::uint8_t, SHA256_BYTE_COUNT> digest{};
		if (!Compute_Sha256(bytes, digest))
			return false;
		constexpr char HEX[] = "0123456789abcdef";
		std::string revision;
		revision.reserve(digest.size() * 2u);
		for (const std::uint8_t byte : digest)
		{
			revision.push_back(HEX[(byte >> 4u) & 0x0fu]);
			revision.push_back(HEX[byte & 0x0fu]);
		}
		outRevision = std::move(revision);
		return true;
	}

	bool_t Write_Durable(
		const std::filesystem::path& path,
		const std::string_view bytes)
	{
		FILE* file = nullptr;
		if (0 != _wfopen_s(&file, path.c_str(), L"wb") || nullptr == file)
			return false;
		const bool_t wrote = bytes.size() ==
			fwrite(bytes.data(), 1u, bytes.size(), file);
		const bool_t flushed = 0 == fflush(file) &&
			0 == _commit(_fileno(file));
		const bool_t closed = 0 == fclose(file);
		return wrote && flushed && closed;
	}

	void Remove_BestEffort(const std::filesystem::path& path)
	{
		std::error_code error;
		std::filesystem::remove(path, error);
	}

	bool_t Documents_AreEqual(
		const VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT& left,
		const VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT& right)
	{
		return left == right;
	}
}

std::filesystem::path Client::CValtanPatternFlowDocument::Resolve_Path()
{
	return CProjectDataRoot::Resolve(
		std::filesystem::path(L"Encounters/Valtan/ValtanBossAuditionFlows.json"));
}

bool_t Client::CValtanPatternFlowDocument::Compute_SourceRevision(
	const std::string_view text,
	std::string& outRevision)
{
	return Build_Revision(text, outRevision);
}

bool_t Client::CValtanPatternFlowDocument::Parse_Text(
	const std::string_view text,
	VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT& outDocument,
	std::string& outStatus)
{
	DATA_JSON_PARSE_LIMITS limits;
	limits.iMaximumBytes = MAX_DOCUMENT_BYTES;
	limits.iMaximumDepth = 8u;
	// Root/flow metadata is bounded and every edge contributes at most one
	// object plus six scalar values.
	limits.iMaximumValues = 32u + MAX_NODES * 4u + MAX_EDGES * 7u;
	DATA_JSON_VALUE root;
	std::string parseError;
	if (!CDataJson::Parse(text, root, parseError, limits) ||
		!Has_ExactProperties(root, { "schema", "formatVersion", "flows" }))
	{
		outStatus = "Valtan Boss Flow JSON is malformed or has unknown properties: " +
			parseError;
		return false;
	}

	const DATA_JSON_VALUE* schema = Required(
		root, "schema", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* version = Required(
		root, "formatVersion", DATA_JSON_TYPE::NUMBER);
	const DATA_JSON_VALUE* flows = Required(
		root, "flows", DATA_JSON_TYPE::ARRAY);
	VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT staged;
	if (nullptr == schema || schema->Get_String() != DOCUMENT_SCHEMA ||
		nullptr == version ||
		!Try_ParseUnsignedInteger(*version, DOCUMENT_VERSION,
			staged.iFormatVersion) ||
		staged.iFormatVersion != DOCUMENT_VERSION ||
		nullptr == flows || 1u != flows->Get_Array().size())
	{
		outStatus = "Valtan Boss Flow header or version is invalid.";
		return false;
	}

	for (const DATA_JSON_VALUE& flowValue : flows->Get_Array())
	{
		if (!Has_ExactProperties(flowValue,
			{ "flowId", "entryNodeId", "nextNodeOrdinal",
			  "nextEdgeOrdinal", "defaultPursuitMs",
			  "maxTransitionsPerRun", "nodes", "edges" }))
		{
			outStatus = "Valtan Boss Flow row has unknown or missing properties.";
			return false;
		}
		const DATA_JSON_VALUE* flowId = Required(
			flowValue, "flowId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* entryNodeId = Required(
			flowValue, "entryNodeId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* nextNodeOrdinal = Required(
			flowValue, "nextNodeOrdinal", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* nextEdgeOrdinal = Required(
			flowValue, "nextEdgeOrdinal", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* defaultPursuit = Required(
			flowValue, "defaultPursuitMs", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* maxTransitions = Required(
			flowValue, "maxTransitionsPerRun", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* nodes = Required(
			flowValue, "nodes", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* edges = Required(
			flowValue, "edges", DATA_JSON_TYPE::ARRAY);
		VALTAN_PATTERN_FLOW_DEFINITION stagedFlow;
		if (nullptr == flowId || !Is_StableId(flowId->Get_String()) ||
			nullptr == entryNodeId || !Is_StableId(entryNodeId->Get_String()) ||
			nullptr == nextNodeOrdinal ||
			!Try_ParseUnsignedInteger(
				*nextNodeOrdinal, MAX_NODE_ORDINAL + 1u,
				stagedFlow.iNextNodeOrdinal) ||
			nullptr == nextEdgeOrdinal ||
			!Try_ParseUnsignedInteger(
				*nextEdgeOrdinal, MAX_EDGE_ORDINAL + 1u,
				stagedFlow.iNextEdgeOrdinal) ||
			nullptr == defaultPursuit ||
			!Try_ParseUnsignedInteger(
				*defaultPursuit,
				(std::numeric_limits<std::uint32_t>::max)(),
				stagedFlow.iDefaultPursuitMs) ||
			nullptr == maxTransitions ||
			!Try_ParseUnsignedInteger(
				*maxTransitions,
				(std::numeric_limits<std::uint32_t>::max)(),
				stagedFlow.iMaxTransitionsPerRun) ||
			nullptr == nodes || nodes->Get_Array().size() > MAX_NODES ||
			nullptr == edges || edges->Get_Array().size() > MAX_EDGES)
		{
			outStatus = "Valtan Boss Flow row is invalid or oversized.";
			return false;
		}
		stagedFlow.strFlowId = flowId->Get_String();
		stagedFlow.strEntryNodeId = entryNodeId->Get_String();
		stagedFlow.Nodes.reserve(nodes->Get_Array().size());
		for (const DATA_JSON_VALUE& nodeValue : nodes->Get_Array())
		{
			if (!Has_ExactProperties(
				nodeValue, { "nodeId", "patternId", "watchdogMs" }))
			{
				outStatus = "Valtan Boss Flow node has unknown or missing properties.";
				return false;
			}
			const DATA_JSON_VALUE* nodeId = Required(
				nodeValue, "nodeId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* patternId = Required(
				nodeValue, "patternId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* watchdog = Required(
				nodeValue, "watchdogMs", DATA_JSON_TYPE::NUMBER);
			VALTAN_PATTERN_FLOW_NODE stagedNode;
			if (nullptr == nodeId || !Is_StableId(nodeId->Get_String()) ||
				nullptr == patternId || !Is_StableId(patternId->Get_String()) ||
				nullptr == watchdog ||
				!Try_ParseUnsignedInteger(
					*watchdog,
					(std::numeric_limits<std::uint32_t>::max)(),
					stagedNode.iWatchdogMs))
			{
				outStatus = "Valtan Boss Flow node identity or watchdog is invalid.";
				return false;
			}
			stagedNode.strNodeId = nodeId->Get_String();
			stagedNode.strPatternId = patternId->Get_String();
			stagedFlow.Nodes.push_back(std::move(stagedNode));
		}

		stagedFlow.Edges.reserve(edges->Get_Array().size());
		for (const DATA_JSON_VALUE& edgeValue : edges->Get_Array())
		{
			const bool_t hasMaxTraversals =
				nullptr != edgeValue.Find("maxTraversals");
			if ((!hasMaxTraversals && !Has_ExactProperties(
					edgeValue,
					{ "edgeId", "fromNodeId", "outcome", "toNodeId",
					  "pursuitMs" })) ||
				(hasMaxTraversals && !Has_ExactProperties(
					edgeValue,
					{ "edgeId", "fromNodeId", "outcome", "toNodeId",
					  "pursuitMs", "maxTraversals" })))
			{
				outStatus = "Valtan Boss Flow edge has unknown or missing properties.";
				return false;
			}
			const DATA_JSON_VALUE* edgeId = Required(
				edgeValue, "edgeId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* fromNodeId = Required(
				edgeValue, "fromNodeId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* outcome = Required(
				edgeValue, "outcome", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* toNodeId = Required(
				edgeValue, "toNodeId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pursuit = Required(
				edgeValue, "pursuitMs", DATA_JSON_TYPE::NUMBER);
			VALTAN_PATTERN_FLOW_EDGE stagedEdge;
			if (nullptr == edgeId || !Is_StableId(edgeId->Get_String()) ||
				nullptr == fromNodeId ||
				!Is_StableId(fromNodeId->Get_String()) ||
				nullptr == outcome || outcome->Get_String() != "COMPLETED" ||
				nullptr == toNodeId || !Is_StableId(toNodeId->Get_String()) ||
				nullptr == pursuit ||
				!Try_ParseUnsignedInteger(
					*pursuit,
					(std::numeric_limits<std::uint32_t>::max)(),
					stagedEdge.iPursuitMs))
			{
				outStatus = "Valtan Boss Flow edge identity or timing is invalid.";
				return false;
			}
			stagedEdge.strEdgeId = edgeId->Get_String();
			stagedEdge.strFromNodeId = fromNodeId->Get_String();
			stagedEdge.strToNodeId = toNodeId->Get_String();
			if (hasMaxTraversals)
			{
				const DATA_JSON_VALUE* maximum = Required(
					edgeValue, "maxTraversals", DATA_JSON_TYPE::NUMBER);
				std::uint32_t stagedMaximum = 0u;
				if (nullptr == maximum ||
					!Try_ParseUnsignedInteger(
						*maximum,
						(std::numeric_limits<std::uint32_t>::max)(),
						stagedMaximum))
				{
					outStatus = "Valtan Boss Flow edge traversal cap is invalid.";
					return false;
				}
				stagedEdge.iMaxTraversals = stagedMaximum;
			}
			stagedFlow.Edges.push_back(std::move(stagedEdge));
		}
		(void)Build_LegacyProjection(stagedFlow);
		staged.Flows.push_back(std::move(stagedFlow));
	}

	outDocument = std::move(staged);
	outStatus = "Parsed Valtan Boss Flow authoring document.";
	return true;
}

bool_t Client::CValtanPatternFlowDocument::Validate(
	const VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT& document,
	const std::vector<std::string>& admittedPatternIds,
	std::string& outStatus)
{
	if (DOCUMENT_VERSION != document.iFormatVersion ||
		1u != document.Flows.size())
	{
		outStatus = "Valtan Boss Flow document version or flow count is invalid.";
		return false;
	}

	std::unordered_set<std::string> admitted;
	admitted.reserve(admittedPatternIds.size());
	for (const std::string& patternId : admittedPatternIds)
	{
		if (!Is_StableId(patternId) || !admitted.insert(patternId).second)
		{
			outStatus = "Valtan Boss Flow inventory is empty, invalid, or duplicate.";
			return false;
		}
	}
	if (admitted.empty())
	{
		outStatus = "Valtan Boss Flow inventory is empty, invalid, or duplicate.";
		return false;
	}

	const VALTAN_PATTERN_FLOW_DEFINITION& flow = document.Flows.front();
	if (flow.Nodes.empty())
	{
		outStatus = "The default Valtan Flow must contain at least one node.";
		return false;
	}
	if (flow.strFlowId != DEFAULT_FLOW_ID ||
		!Is_StableId(flow.strEntryNodeId) ||
		flow.iDefaultPursuitMs < MIN_INTER_STEP_PURSUIT_MS ||
		flow.iDefaultPursuitMs > MAX_INTER_STEP_PURSUIT_MS ||
		flow.iMaxTransitionsPerRun < MIN_TRANSITIONS_PER_RUN ||
		flow.iMaxTransitionsPerRun > MAX_TRANSITIONS_PER_RUN ||
		0u == flow.iNextNodeOrdinal ||
		flow.iNextNodeOrdinal > MAX_NODE_ORDINAL + 1u ||
		0u == flow.iNextEdgeOrdinal ||
		flow.iNextEdgeOrdinal > MAX_EDGE_ORDINAL + 1u ||
		flow.Nodes.size() > MAX_NODES || flow.Edges.size() > MAX_EDGES)
	{
		outStatus =
			"Valtan Boss Flow identity, limits, default pursuit, or size is invalid.";
		return false;
	}

	std::unordered_map<std::string, const VALTAN_PATTERN_FLOW_NODE*> nodesById;
	std::unordered_set<std::uint64_t> nodeOrdinals;
	nodesById.reserve(flow.Nodes.size());
	nodeOrdinals.reserve(flow.Nodes.size());
	std::uint64_t maximumNodeOrdinal = 0u;
	std::size_t entryCinematicCount = 0u;
	for (const VALTAN_PATTERN_FLOW_NODE& node : flow.Nodes)
	{
		std::uint64_t ordinal = 0u;
		if (!Is_StableId(node.strNodeId) ||
			!Try_ParseOrdinal(
				flow.strFlowId, node.strNodeId, { "slot", "node" },
				MAX_NODE_ORDINAL, ordinal) ||
			!nodeOrdinals.insert(ordinal).second ||
			!nodesById.emplace(node.strNodeId, &node).second ||
			!Is_StableId(node.strPatternId) ||
			!admitted.contains(node.strPatternId) ||
			(0u != node.iWatchdogMs &&
			 (node.iWatchdogMs < MIN_NODE_WATCHDOG_MS ||
			  node.iWatchdogMs > MAX_NODE_WATCHDOG_MS)))
		{
			outStatus =
				"Valtan Boss Flow node is duplicate, malformed, outside watchdog limits, or not in All Effects inventory.";
			return false;
		}
		maximumNodeOrdinal = (std::max)(maximumNodeOrdinal, ordinal);
		if (OPTIONAL_ENTRY_PATTERN_ID == node.strPatternId)
		{
			++entryCinematicCount;
			if (node.strNodeId != flow.strEntryNodeId)
			{
				outStatus =
					"The optional entry cinematic must be the Flow entry node.";
				return false;
			}
		}
	}
	if (!nodesById.contains(flow.strEntryNodeId))
	{
		outStatus = "Valtan Boss Flow entryNodeId is dangling.";
		return false;
	}
	if (entryCinematicCount > 1u)
	{
		outStatus = "The optional entry cinematic may occur only once.";
		return false;
	}
	if (flow.iNextNodeOrdinal <= maximumNodeOrdinal)
	{
		outStatus =
			"Valtan Boss Flow nextNodeOrdinal would reuse a stable node ID.";
		return false;
	}

	std::unordered_set<std::string> edgeIds;
	std::unordered_map<std::string, const VALTAN_PATTERN_FLOW_EDGE*> outgoing;
	edgeIds.reserve(flow.Edges.size());
	outgoing.reserve(flow.Edges.size());
	std::uint64_t maximumEdgeOrdinal = 0u;
	for (const VALTAN_PATTERN_FLOW_EDGE& edge : flow.Edges)
	{
		std::uint64_t ordinal = 0u;
		if (!Is_StableId(edge.strEdgeId) ||
			!Try_ParseOrdinal(
				flow.strFlowId, edge.strEdgeId, { "edge" },
				MAX_EDGE_ORDINAL, ordinal) ||
			!edgeIds.insert(edge.strEdgeId).second ||
			edge.eOutcome != VALTAN_PATTERN_FLOW_EDGE_OUTCOME::COMPLETED ||
			!nodesById.contains(edge.strFromNodeId) ||
			!nodesById.contains(edge.strToNodeId) ||
			!outgoing.emplace(edge.strFromNodeId, &edge).second ||
			edge.iPursuitMs < MIN_INTER_STEP_PURSUIT_MS ||
			edge.iPursuitMs > MAX_INTER_STEP_PURSUIT_MS ||
			(edge.iMaxTraversals.has_value() &&
			 (0u == *edge.iMaxTraversals ||
			  *edge.iMaxTraversals > MAX_EDGE_TRAVERSALS)))
		{
			outStatus =
				"Valtan Boss Flow edge is duplicate, dangling, non-deterministic, or outside timing/traversal limits.";
			return false;
		}
		if (entryCinematicCount > 0u &&
			edge.strToNodeId == flow.strEntryNodeId)
		{
			outStatus =
				"The entry cinematic node cannot be targeted by another edge.";
			return false;
		}
		maximumEdgeOrdinal = (std::max)(maximumEdgeOrdinal, ordinal);
	}
	if (flow.iNextEdgeOrdinal <= maximumEdgeOrdinal)
	{
		outStatus =
			"Valtan Boss Flow nextEdgeOrdinal would reuse a stable edge ID.";
		return false;
	}

	/* With one COMPLETED edge per node, the entry walk is the entire
	   executable topology. A repeated target is the one cycle-closing
	   back-edge and must own the only finite traversal cap. */
	std::unordered_set<std::string> reachable;
	reachable.reserve(flow.Nodes.size());
	reachable.insert(flow.strEntryNodeId);
	const VALTAN_PATTERN_FLOW_EDGE* cycleClosingEdge = nullptr;
	std::string currentNodeId = flow.strEntryNodeId;
	while (true)
	{
		const auto edgeAt = outgoing.find(currentNodeId);
		if (outgoing.end() == edgeAt)
			break;
		const VALTAN_PATTERN_FLOW_EDGE* edge = edgeAt->second;
		if (reachable.contains(edge->strToNodeId))
		{
			cycleClosingEdge = edge;
			break;
		}
		reachable.insert(edge->strToNodeId);
		currentNodeId = edge->strToNodeId;
	}
	if (reachable.size() != flow.Nodes.size())
	{
		outStatus = "Valtan Boss Flow contains an unreachable node or edge.";
		return false;
	}
	for (const VALTAN_PATTERN_FLOW_EDGE& edge : flow.Edges)
	{
		const bool_t isCycleClosingEdge = &edge == cycleClosingEdge;
		if ((isCycleClosingEdge && !edge.iMaxTraversals.has_value()) ||
			(!isCycleClosingEdge && edge.iMaxTraversals.has_value()))
		{
			outStatus =
				"Only a cycle-closing back-edge may own maxTraversals, and every back-edge must be capped.";
			return false;
		}
	}

	/* Simulate the deterministic cursor. A capped back-edge becomes a
	   terminal hold when exhausted; maxTransitionsPerRun must not pre-empt
	   the authored terminal. */
	std::unordered_map<std::string, std::uint32_t> traversalCounts;
	currentNodeId = flow.strEntryNodeId;
	std::uint32_t transitionCount = 0u;
	while (true)
	{
		const auto edgeAt = outgoing.find(currentNodeId);
		if (outgoing.end() == edgeAt)
			break;
		const VALTAN_PATTERN_FLOW_EDGE& edge = *edgeAt->second;
		std::uint32_t& traversals = traversalCounts[edge.strEdgeId];
		if (edge.iMaxTraversals.has_value() &&
			traversals >= *edge.iMaxTraversals)
		{
			break;
		}
		if (transitionCount >= flow.iMaxTransitionsPerRun)
		{
			outStatus =
				"Valtan Boss Flow cannot reach its terminal within maxTransitionsPerRun.";
			return false;
		}
		++traversals;
		++transitionCount;
		currentNodeId = edge.strToNodeId;
	}

	outStatus = "Validated Valtan Boss Flow v2: " +
		std::to_string(flow.Nodes.size()) + " node(s), " +
		std::to_string(flow.Edges.size()) + " edge(s).";
	return true;
}

std::string Client::CValtanPatternFlowDocument::Serialize_Text(
	const VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT& document)
{
	std::ostringstream output;
	output << "{\n"
		<< "  \"schema\": \"" << DOCUMENT_SCHEMA << "\",\n"
		<< "  \"formatVersion\": " << document.iFormatVersion << ",\n"
		<< "  \"flows\": [\n";
	for (std::size_t flowIndex = 0u;
		flowIndex < document.Flows.size(); ++flowIndex)
	{
		const VALTAN_PATTERN_FLOW_DEFINITION& flow =
			document.Flows[flowIndex];
		output << "    {\n"
			<< "      \"flowId\": \""
			<< CDataJson::Escape(flow.strFlowId) << "\",\n"
			<< "      \"entryNodeId\": \""
			<< CDataJson::Escape(flow.strEntryNodeId) << "\",\n"
			<< "      \"nextNodeOrdinal\": "
			<< flow.iNextNodeOrdinal << ",\n"
			<< "      \"nextEdgeOrdinal\": "
			<< flow.iNextEdgeOrdinal << ",\n"
			<< "      \"defaultPursuitMs\": "
			<< flow.iDefaultPursuitMs << ",\n"
			<< "      \"maxTransitionsPerRun\": "
			<< flow.iMaxTransitionsPerRun << ",\n"
			<< "      \"nodes\": [\n";
		for (std::size_t nodeIndex = 0u;
			nodeIndex < flow.Nodes.size(); ++nodeIndex)
		{
			const VALTAN_PATTERN_FLOW_NODE& node = flow.Nodes[nodeIndex];
			output << "        { \"nodeId\": \""
				<< CDataJson::Escape(node.strNodeId)
				<< "\", \"patternId\": \""
				<< CDataJson::Escape(node.strPatternId)
				<< "\", \"watchdogMs\": " << node.iWatchdogMs << " }"
				<< (nodeIndex + 1u < flow.Nodes.size() ? "," : "")
				<< "\n";
		}
		output << "      ],\n"
			<< "      \"edges\": [\n";
		for (std::size_t edgeIndex = 0u;
			edgeIndex < flow.Edges.size(); ++edgeIndex)
		{
			const VALTAN_PATTERN_FLOW_EDGE& edge = flow.Edges[edgeIndex];
			output << "        { \"edgeId\": \""
				<< CDataJson::Escape(edge.strEdgeId)
				<< "\", \"fromNodeId\": \""
				<< CDataJson::Escape(edge.strFromNodeId)
				<< "\", \"outcome\": \"COMPLETED\", \"toNodeId\": \""
				<< CDataJson::Escape(edge.strToNodeId)
				<< "\", \"pursuitMs\": " << edge.iPursuitMs;
			if (edge.iMaxTraversals.has_value())
				output << ", \"maxTraversals\": " << *edge.iMaxTraversals;
			output << " }"
				<< (edgeIndex + 1u < flow.Edges.size() ? "," : "")
				<< "\n";
		}
		output << "      ]\n"
			<< "    }"
			<< (flowIndex + 1u < document.Flows.size() ? "," : "")
			<< "\n";
	}
	output << "  ]\n}\n";
	return output.str();
}

bool_t Client::CValtanPatternFlowDocument::Load(
	const std::vector<std::string>& admittedPatternIds,
	std::string& outStatus)
{
	outStatus.clear();
	const std::filesystem::path path = Resolve_Path();
	std::string bytes;
	VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT staged;
	std::string revision;
	if (path.empty())
	{
		outStatus = "Could not resolve the Valtan Boss Flow document path.";
		return false;
	}
	if (!Read_Bytes(path, bytes, outStatus) ||
		!Parse_Text(bytes, staged, outStatus) ||
		!Validate(staged, admittedPatternIds, outStatus))
	{
		return false;
	}
	if (!Build_Revision(bytes, revision))
	{
		outStatus = "Could not compute the Valtan Boss Flow source revision.";
		return false;
	}

	m_Path = path;
	m_Baseline = staged;
	m_Draft = std::move(staged);
	m_strSourceRevision = std::move(revision);
	m_bReady = true;
	m_bExternalConflict = false;
	outStatus = "Loaded Valtan Boss Flow revision " + m_strSourceRevision + ".";
	return true;
}

bool_t Client::CValtanPatternFlowDocument::Reload(
	const std::vector<std::string>& admittedPatternIds,
	std::string& outStatus)
{
	return Load(admittedPatternIds, outStatus);
}

bool_t Client::CValtanPatternFlowDocument::Save(
	const std::vector<std::string>& admittedPatternIds,
	std::string& outStatus)
{
	outStatus.clear();
	if (!m_bReady || m_Path.empty())
	{
		outStatus = "Valtan Boss Flow must be loaded before Save.";
		return false;
	}
	if (!Validate(m_Draft, admittedPatternIds, outStatus))
		return false;

	SCOPED_VALTAN_PATTERN_FLOW_WRITE_LOCK writerLock;
	std::string lockStatus;
	if (!writerLock.Try_Acquire(
		CProjectDataRoot::Get().parent_path(), lockStatus))
	{
		outStatus =
			"Valtan Boss Flow Save rejected before mutation: " + lockStatus +
			". Draft and source bytes were preserved.";
		return false;
	}

	std::string diskBytes;
	std::string diskRevision;
	if (!Read_Bytes(m_Path, diskBytes, outStatus))
	{
		return false;
	}
	if (!Build_Revision(diskBytes, diskRevision))
	{
		outStatus = "Could not compute the current Valtan Boss Flow revision.";
		return false;
	}
	if (diskRevision != m_strSourceRevision)
	{
		m_bExternalConflict = true;
		outStatus = "Valtan Boss Flow changed on disk; Reload before Save.";
		return false;
	}

	const std::string serialized = Serialize_Text(m_Draft);
	VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT verifiedDraft;
	std::string verifyStatus;
	if (!Parse_Text(serialized, verifiedDraft, verifyStatus) ||
		!Validate(verifiedDraft, admittedPatternIds, verifyStatus) ||
		!Documents_AreEqual(m_Draft, verifiedDraft))
	{
		outStatus = "Valtan Boss Flow serialization verification failed: " +
			verifyStatus;
		return false;
	}

	const std::wstring uniqueSuffix =
		L"." + std::to_wstring(GetCurrentProcessId()) + L"." +
		std::to_wstring(GetTickCount64());
	std::filesystem::path temporary = m_Path;
	temporary += L".tmp" + uniqueSuffix;
	std::filesystem::path backup = m_Path;
	backup += L".backup" + uniqueSuffix;
	if (!Write_Durable(temporary, serialized))
	{
		Remove_BestEffort(temporary);
		outStatus = "Could not durably write the temporary Valtan Boss Flow document.";
		return false;
	}

	std::string temporaryBytes;
	VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT temporaryDocument;
	if (!Read_Bytes(temporary, temporaryBytes, verifyStatus) ||
		temporaryBytes != serialized ||
		!Parse_Text(temporaryBytes, temporaryDocument, verifyStatus) ||
		!Validate(temporaryDocument, admittedPatternIds, verifyStatus) ||
		!Documents_AreEqual(m_Draft, temporaryDocument))
	{
		Remove_BestEffort(temporary);
		outStatus = "Valtan Boss Flow temporary verification failed: " +
			verifyStatus;
		return false;
	}

	if (!CopyFileW(m_Path.c_str(), backup.c_str(), TRUE))
	{
		Remove_BestEffort(temporary);
		outStatus = "Could not create the Valtan Boss Flow recovery backup.";
		return false;
	}

	std::string preCommitBytes;
	std::string preCommitRevision;
	if (!Read_Bytes(m_Path, preCommitBytes, verifyStatus) ||
		!Build_Revision(preCommitBytes, preCommitRevision) ||
		preCommitRevision != m_strSourceRevision)
	{
		Remove_BestEffort(temporary);
		Remove_BestEffort(backup);
		m_bExternalConflict = true;
		outStatus = "Valtan Boss Flow changed during Save; disk and draft were preserved.";
		return false;
	}

	if (!MoveFileExW(
			temporary.c_str(), m_Path.c_str(),
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
	{
		Remove_BestEffort(temporary);
		Remove_BestEffort(backup);
		outStatus = "Could not atomically replace the Valtan Boss Flow document.";
		return false;
	}

	std::string committedBytes;
	std::string committedRevision;
	VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT committedDocument;
	if (!Read_Bytes(m_Path, committedBytes, verifyStatus) ||
		committedBytes != serialized ||
		!Parse_Text(committedBytes, committedDocument, verifyStatus) ||
		!Validate(committedDocument, admittedPatternIds, verifyStatus) ||
		!Documents_AreEqual(m_Draft, committedDocument) ||
		!Build_Revision(committedBytes, committedRevision))
	{
		if (MoveFileExW(
				backup.c_str(), m_Path.c_str(),
				MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
		{
			outStatus =
				"Valtan Boss Flow post-replace verification failed; the baseline was restored: " +
				verifyStatus;
		}
		else
		{
			m_bExternalConflict = true;
			outStatus =
				"Valtan Boss Flow post-replace verification and rollback failed; recovery backup remains at " +
				backup.string() + ": " + verifyStatus;
		}
		return false;
	}

	Remove_BestEffort(backup);
	m_Baseline = committedDocument;
	m_Draft = std::move(committedDocument);
	m_strSourceRevision = std::move(committedRevision);
	m_bExternalConflict = false;
	outStatus = "Saved Valtan Boss Flow revision " + m_strSourceRevision + ".";
	return true;
}

bool_t Client::CValtanPatternFlowDocument::Verify_SourceRevision(
	std::string& outStatus)
{
	outStatus.clear();
	if (!m_bReady || m_Path.empty() || m_strSourceRevision.empty())
	{
		outStatus =
			"Valtan Boss Flow must be loaded before checking its source revision.";
		return false;
	}

	std::string diskBytes;
	std::string diskRevision;
	if (!Read_Bytes(m_Path, diskBytes, outStatus))
	{
		m_bExternalConflict = true;
		return false;
	}
	if (!Build_Revision(diskBytes, diskRevision))
	{
		m_bExternalConflict = true;
		outStatus = "Could not compute the current Valtan Boss Flow revision.";
		return false;
	}
	if (diskRevision != m_strSourceRevision)
	{
		m_bExternalConflict = true;
		outStatus =
			"Valtan Boss Flow changed on disk; Reload before Server playback.";
		return false;
	}

	m_bExternalConflict = false;
	outStatus = "Valtan Boss Flow source revision is current.";
	return true;
}

bool_t Client::CValtanPatternFlowDocument::Add_Slot(
	const std::string_view patternId,
	const std::vector<std::string>& admittedPatternIds,
	std::string& outSlotId,
	std::string& outStatus)
{
	outStatus.clear();
	const VALTAN_PATTERN_FLOW_DEFINITION* current = Get_DefaultFlow();
	if (nullptr == current || !Has_LegacyLinearProjection(*current) ||
		current->Nodes.size() >= MAX_NODES ||
		current->iNextNodeOrdinal > MAX_NODE_ORDINAL ||
		admittedPatternIds.end() == std::find(
			admittedPatternIds.begin(), admittedPatternIds.end(), patternId))
	{
		outStatus =
			"Valtan Boss Flow cannot add this pattern, is not a linear compatibility graph, or has reached 255 nodes.";
		return false;
	}

	VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT staged = m_Draft;
	VALTAN_PATTERN_FLOW_DEFINITION& flow = staged.Flows.front();
	const std::string nodeId = Build_OrdinalId(
		flow.strFlowId, "node", flow.iNextNodeOrdinal);
	const bool_t bEntryPattern = OPTIONAL_ENTRY_PATTERN_ID == patternId;
	flow.Nodes.push_back({
		nodeId, std::string(patternId), DEFAULT_NODE_WATCHDOG_MS });
	std::vector<std::string> orderedNodeIds;
	orderedNodeIds.reserve(flow.Slots.size() + 1u);
	for (const VALTAN_PATTERN_FLOW_SLOT& slot : flow.Slots)
		orderedNodeIds.push_back(slot.strSlotId);
	if (bEntryPattern)
	{
		orderedNodeIds.insert(orderedNodeIds.begin(), nodeId);
	}
	else
	{
		/* Authoring convenience: a newly chosen Pattern becomes Pattern 01 so
		   Restart Flow can audition it immediately.  The optional entrance
		   cinematic remains the immutable entry when it is present. */
		const auto insertAt = !flow.Slots.empty() &&
			OPTIONAL_ENTRY_PATTERN_ID == flow.Slots.front().strPatternId ?
			std::next(orderedNodeIds.begin()) : orderedNodeIds.begin();
		orderedNodeIds.insert(insertAt, nodeId);
	}
	++flow.iNextNodeOrdinal;
	if (!Rebuild_LinearFlow(flow, orderedNodeIds, outStatus))
		return false;
	if (!Validate(staged, admittedPatternIds, outStatus))
		return false;
	m_Draft = std::move(staged);
	outSlotId = nodeId;
	outStatus = "Added " + std::string(patternId) + " as " + nodeId +
		(bEntryPattern ? " at the Flow entry node." :
			" at Pattern 01 (after the entrance cinematic when present).");
	return true;
}

bool_t Client::CValtanPatternFlowDocument::Move_Slot(
	const std::string_view slotId,
	const std::int32_t delta,
	const std::vector<std::string>& admittedPatternIds,
	std::string& outStatus)
{
	outStatus.clear();
	const VALTAN_PATTERN_FLOW_DEFINITION* current =
		m_bReady && 1u == m_Draft.Flows.size() ?
			&m_Draft.Flows.front() : nullptr;
	if (nullptr == current || !Has_LegacyLinearProjection(*current) ||
		(delta != -1 && delta != 1))
	{
		outStatus =
			"Valtan Boss Flow move requires a loaded linear graph and delta -1 or +1.";
		return false;
	}
	const auto found = std::find_if(
		current->Slots.begin(), current->Slots.end(),
		[slotId](const VALTAN_PATTERN_FLOW_SLOT& slot)
		{
			return slot.strSlotId == slotId;
		});
	if (found == current->Slots.end())
	{
		outStatus = "Valtan Boss Flow slot no longer exists.";
		return false;
	}
	const std::ptrdiff_t source = found - current->Slots.begin();
	const std::ptrdiff_t destination = source + delta;
	if (destination < 0 ||
		destination >= static_cast<std::ptrdiff_t>(current->Slots.size()))
	{
		outStatus = "Valtan Boss Flow slot is already at that boundary.";
		return false;
	}

	VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT staged = m_Draft;
	VALTAN_PATTERN_FLOW_DEFINITION& flow = staged.Flows.front();
	std::vector<std::string> orderedNodeIds;
	orderedNodeIds.reserve(flow.Slots.size());
	for (const VALTAN_PATTERN_FLOW_SLOT& slot : flow.Slots)
		orderedNodeIds.push_back(slot.strSlotId);
	std::iter_swap(
		orderedNodeIds.begin() + source,
		orderedNodeIds.begin() + destination);
	if (!Rebuild_LinearFlow(flow, orderedNodeIds, outStatus))
		return false;
	if (!Validate(staged, admittedPatternIds, outStatus))
		return false;
	m_Draft = std::move(staged);
	outStatus = "Moved Valtan Boss Flow slot " + std::string(slotId) + ".";
	return true;
}

bool_t Client::CValtanPatternFlowDocument::Remove_Slot(
	const std::string_view slotId,
	std::string& outStatus)
{
	outStatus.clear();
	const VALTAN_PATTERN_FLOW_DEFINITION* current =
		m_bReady && 1u == m_Draft.Flows.size() ?
			&m_Draft.Flows.front() : nullptr;
	if (nullptr == current || !Has_LegacyLinearProjection(*current))
	{
		outStatus =
			"Valtan Boss Flow must be a loaded linear graph before removing a node.";
		return false;
	}
	const auto found = std::find_if(
		current->Slots.begin(), current->Slots.end(),
		[slotId](const VALTAN_PATTERN_FLOW_SLOT& slot)
		{
			return slot.strSlotId == slotId;
		});
	if (found == current->Slots.end())
	{
		outStatus = "Valtan Boss Flow node no longer exists.";
		return false;
	}
	if (1u == current->Slots.size())
	{
		outStatus = "The default Valtan Flow must retain at least one node.";
		return false;
	}

	VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT staged = m_Draft;
	VALTAN_PATTERN_FLOW_DEFINITION& flow = staged.Flows.front();
	flow.Nodes.erase(std::remove_if(
		flow.Nodes.begin(), flow.Nodes.end(),
		[slotId](const VALTAN_PATTERN_FLOW_NODE& node)
		{
			return node.strNodeId == slotId;
		}), flow.Nodes.end());
	std::vector<std::string> orderedNodeIds;
	orderedNodeIds.reserve(current->Slots.size() - 1u);
	for (const VALTAN_PATTERN_FLOW_SLOT& slot : current->Slots)
	{
		if (slot.strSlotId != slotId)
			orderedNodeIds.push_back(slot.strSlotId);
	}
	if (!Rebuild_LinearFlow(flow, orderedNodeIds, outStatus))
		return false;
	const std::vector<std::string> admitted =
		Build_CurrentPatternInventory(flow);
	if (!Validate(staged, admitted, outStatus))
		return false;
	m_Draft = std::move(staged);
	outStatus = "Removed Valtan Boss Flow node " + std::string(slotId) + ".";
	return true;
}

bool_t Client::CValtanPatternFlowDocument::Set_InterStepPursuitMs(
	const std::uint32_t milliseconds,
	std::string& outStatus)
{
	outStatus.clear();
	const VALTAN_PATTERN_FLOW_DEFINITION* current =
		m_bReady && 1u == m_Draft.Flows.size() ?
			&m_Draft.Flows.front() : nullptr;
	if (nullptr == current || milliseconds < MIN_INTER_STEP_PURSUIT_MS ||
		milliseconds > MAX_INTER_STEP_PURSUIT_MS)
	{
		outStatus = "Valtan Boss Flow pursuit interval must be 100..10000 ms.";
		return false;
	}
	VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT staged = m_Draft;
	VALTAN_PATTERN_FLOW_DEFINITION& flow = staged.Flows.front();
	flow.iDefaultPursuitMs = milliseconds;
	if (Has_LegacyLinearProjection(*current))
	{
		for (VALTAN_PATTERN_FLOW_EDGE& edge : flow.Edges)
			edge.iPursuitMs = milliseconds;
	}
	(void)Build_LegacyProjection(flow);
	const std::vector<std::string> admitted =
		Build_CurrentPatternInventory(flow);
	if (!Validate(staged, admitted, outStatus))
		return false;
	m_Draft = std::move(staged);
	outStatus = "Updated Valtan Boss Flow default pursuit interval.";
	return true;
}

bool_t Client::CValtanPatternFlowDocument::Set_NodeWatchdogMs(
	const std::string_view nodeId,
	const std::uint32_t milliseconds,
	const std::vector<std::string>& admittedPatternIds,
	std::string& outStatus)
{
	outStatus.clear();
	if (!m_bReady ||
		(0u != milliseconds &&
		 (milliseconds < MIN_NODE_WATCHDOG_MS ||
		  milliseconds > MAX_NODE_WATCHDOG_MS)))
	{
		outStatus =
			"Node watchdog must be disabled (0) or within 1000..300000 ms.";
		return false;
	}
	VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT staged = m_Draft;
	VALTAN_PATTERN_FLOW_DEFINITION& flow = staged.Flows.front();
	const auto found = std::find_if(
		flow.Nodes.begin(), flow.Nodes.end(),
		[nodeId](const VALTAN_PATTERN_FLOW_NODE& node)
		{
			return node.strNodeId == nodeId;
		});
	if (flow.Nodes.end() == found)
	{
		outStatus = "Valtan Boss Flow node no longer exists.";
		return false;
	}
	found->iWatchdogMs = milliseconds;
	(void)Build_LegacyProjection(flow);
	if (!Validate(staged, admittedPatternIds, outStatus))
		return false;
	m_Draft = std::move(staged);
	outStatus = "Updated Valtan Boss Flow node watchdog.";
	return true;
}

bool_t Client::CValtanPatternFlowDocument::Set_MaxTransitionsPerRun(
	const std::uint32_t transitions,
	const std::vector<std::string>& admittedPatternIds,
	std::string& outStatus)
{
	outStatus.clear();
	if (!m_bReady || transitions < MIN_TRANSITIONS_PER_RUN ||
		transitions > MAX_TRANSITIONS_PER_RUN)
	{
		outStatus =
			"Flow transition watchdog must be within 1..4096 transitions.";
		return false;
	}
	VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT staged = m_Draft;
	staged.Flows.front().iMaxTransitionsPerRun = transitions;
	if (!Validate(staged, admittedPatternIds, outStatus))
		return false;
	m_Draft = std::move(staged);
	outStatus = "Updated Valtan Boss Flow transition watchdog.";
	return true;
}

bool_t Client::CValtanPatternFlowDocument::Insert_Node_After(
	const std::string_view afterNodeId,
	const std::string_view patternId,
	const std::vector<std::string>& admittedPatternIds,
	std::string& outNodeId,
	std::string& outStatus)
{
	outNodeId.clear();
	outStatus.clear();
	const VALTAN_PATTERN_FLOW_DEFINITION* current = Get_DefaultFlow();
	if (nullptr == current || current->Nodes.size() >= MAX_NODES ||
		current->Edges.size() >= MAX_EDGES ||
		current->iNextNodeOrdinal > MAX_NODE_ORDINAL ||
		current->iNextEdgeOrdinal > MAX_EDGE_ORDINAL ||
		admittedPatternIds.end() == std::find(
			admittedPatternIds.begin(), admittedPatternIds.end(), patternId))
	{
		outStatus =
			"Valtan Boss Flow cannot insert this Pattern or has reached its graph limits.";
		return false;
	}
	if (OPTIONAL_ENTRY_PATTERN_ID == patternId)
	{
		outStatus =
			"The optional entrance cinematic can only be the existing Flow entry node.";
		return false;
	}

	VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT staged = m_Draft;
	VALTAN_PATTERN_FLOW_DEFINITION& flow = staged.Flows.front();
	auto after = flow.Nodes.end();
	if (!afterNodeId.empty())
	{
		after = std::find_if(
			flow.Nodes.begin(), flow.Nodes.end(),
			[afterNodeId](const VALTAN_PATTERN_FLOW_NODE& node)
			{
				return node.strNodeId == afterNodeId;
			});
	}
	else
	{
		after = std::find_if(
			flow.Nodes.begin(), flow.Nodes.end(),
			[&flow](const VALTAN_PATTERN_FLOW_NODE& node)
			{
				return flow.Edges.end() == std::find_if(
					flow.Edges.begin(), flow.Edges.end(),
					[&node](const VALTAN_PATTERN_FLOW_EDGE& edge)
					{
						return edge.strFromNodeId == node.strNodeId;
					});
			});
	}
	if (flow.Nodes.end() == after)
	{
		outStatus = afterNodeId.empty() ?
			"The graph has no terminal node. Remove its repeat edge before appending." :
			"The selected Valtan Boss Flow node no longer exists.";
		return false;
	}

	const std::string insertedNodeId = Build_OrdinalId(
		flow.strFlowId, "node", flow.iNextNodeOrdinal++);
	const std::string afterId = after->strNodeId;
	flow.Nodes.insert(
		std::next(after),
		{ insertedNodeId, std::string(patternId), DEFAULT_NODE_WATCHDOG_MS });

	auto outgoing = std::find_if(
		flow.Edges.begin(), flow.Edges.end(),
		[&afterId](const VALTAN_PATTERN_FLOW_EDGE& edge)
		{
			return edge.strFromNodeId == afterId;
		});
	VALTAN_PATTERN_FLOW_EDGE insertedEdge;
	insertedEdge.strEdgeId = Build_OrdinalId(
		flow.strFlowId, "edge", flow.iNextEdgeOrdinal++);
	insertedEdge.eOutcome = VALTAN_PATTERN_FLOW_EDGE_OUTCOME::COMPLETED;
	if (flow.Edges.end() == outgoing)
	{
		insertedEdge.strFromNodeId = afterId;
		insertedEdge.strToNodeId = insertedNodeId;
		insertedEdge.iPursuitMs = flow.iDefaultPursuitMs;
	}
	else
	{
		insertedEdge.strFromNodeId = insertedNodeId;
		insertedEdge.strToNodeId = outgoing->strToNodeId;
		insertedEdge.iPursuitMs = outgoing->iPursuitMs;
		insertedEdge.iMaxTraversals = outgoing->iMaxTraversals;
		outgoing->strToNodeId = insertedNodeId;
		outgoing->iMaxTraversals.reset();
	}
	flow.Edges.push_back(std::move(insertedEdge));
	(void)Build_LegacyProjection(flow);
	if (!Validate(staged, admittedPatternIds, outStatus))
		return false;

	m_Draft = std::move(staged);
	outNodeId = insertedNodeId;
	outStatus = "Inserted " + std::string(patternId) + " as " +
		insertedNodeId + " after " + afterId + ".";
	return true;
}

bool_t Client::CValtanPatternFlowDocument::Remove_Node(
	const std::string_view nodeId,
	const std::vector<std::string>& admittedPatternIds,
	std::string& outStatus)
{
	outStatus.clear();
	const VALTAN_PATTERN_FLOW_DEFINITION* current = Get_DefaultFlow();
	if (nullptr == current || current->Nodes.size() <= 1u)
	{
		outStatus = "The default Valtan Flow must retain at least one node.";
		return false;
	}

	VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT staged = m_Draft;
	VALTAN_PATTERN_FLOW_DEFINITION& flow = staged.Flows.front();
	const auto node = std::find_if(
		flow.Nodes.begin(), flow.Nodes.end(),
		[nodeId](const VALTAN_PATTERN_FLOW_NODE& candidate)
		{
			return candidate.strNodeId == nodeId;
		});
	if (flow.Nodes.end() == node)
	{
		outStatus = "The selected Valtan Boss Flow node no longer exists.";
		return false;
	}

	const auto outgoing = std::find_if(
		flow.Edges.begin(), flow.Edges.end(),
		[nodeId](const VALTAN_PATTERN_FLOW_EDGE& edge)
		{
			return edge.strFromNodeId == nodeId;
		});
	const bool_t hasUsableOutgoing = flow.Edges.end() != outgoing &&
		outgoing->strToNodeId != nodeId;
	const VALTAN_PATTERN_FLOW_EDGE outgoingCopy = hasUsableOutgoing ?
		*outgoing : VALTAN_PATTERN_FLOW_EDGE{};

	std::vector<VALTAN_PATTERN_FLOW_EDGE> rebuiltEdges;
	rebuiltEdges.reserve(flow.Edges.size());
	for (VALTAN_PATTERN_FLOW_EDGE edge : flow.Edges)
	{
		if (edge.strFromNodeId == nodeId)
			continue;
		if (edge.strToNodeId == nodeId)
		{
			if (!hasUsableOutgoing)
				continue;
			edge.strToNodeId = outgoingCopy.strToNodeId;
			if (outgoingCopy.iMaxTraversals.has_value())
			{
				edge.iPursuitMs = outgoingCopy.iPursuitMs;
				edge.iMaxTraversals = outgoingCopy.iMaxTraversals;
			}
		}
		rebuiltEdges.push_back(std::move(edge));
	}
	if (flow.strEntryNodeId == nodeId)
	{
		if (!hasUsableOutgoing)
		{
			outStatus =
				"The Flow entry node has no successor to promote before removal.";
			return false;
		}
		flow.strEntryNodeId = outgoingCopy.strToNodeId;
	}
	flow.Nodes.erase(node);
	flow.Edges = std::move(rebuiltEdges);
	(void)Build_LegacyProjection(flow);
	if (!Validate(staged, admittedPatternIds, outStatus))
		return false;

	m_Draft = std::move(staged);
	outStatus = "Removed Valtan Boss Flow node " + std::string(nodeId) +
		" and rejoined its deterministic path.";
	return true;
}

bool_t Client::CValtanPatternFlowDocument::Set_EntryNode(
	const std::string_view nodeId,
	const std::vector<std::string>& admittedPatternIds,
	std::string& outStatus)
{
	outStatus.clear();
	const VALTAN_PATTERN_FLOW_DEFINITION* current = Get_DefaultFlow();
	if (nullptr == current)
	{
		outStatus = "Load the Valtan Boss Flow before changing its start node.";
		return false;
	}
	const auto selected = std::find_if(
		current->Nodes.begin(), current->Nodes.end(),
		[nodeId](const VALTAN_PATTERN_FLOW_NODE& node)
		{
			return node.strNodeId == nodeId;
		});
	if (current->Nodes.end() == selected)
	{
		outStatus = "The selected Valtan Boss Flow node no longer exists.";
		return false;
	}
	if (current->strEntryNodeId == nodeId)
	{
		outStatus = "The selected node is already the Valtan Boss Flow start.";
		return true;
	}
	const auto entrance = std::find_if(
		current->Nodes.begin(), current->Nodes.end(),
		[](const VALTAN_PATTERN_FLOW_NODE& node)
		{
			return OPTIONAL_ENTRY_PATTERN_ID == node.strPatternId;
		});
	if (current->Nodes.end() != entrance)
	{
		outStatus =
			"Remove the optional entrance cinematic before choosing a different start node.";
		return false;
	}

	VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT staged = m_Draft;
	VALTAN_PATTERN_FLOW_DEFINITION& flow = staged.Flows.front();
	const auto cappedEdge = std::find_if(
		flow.Edges.begin(), flow.Edges.end(),
		[](const VALTAN_PATTERN_FLOW_EDGE& edge)
		{
			return edge.iMaxTraversals.has_value();
		});
	if (flow.Edges.end() != cappedEdge)
	{
		const std::uint32_t cap = *cappedEdge->iMaxTraversals;
		cappedEdge->iMaxTraversals.reset();
		const auto closesAtNewEntry = std::find_if(
			flow.Edges.begin(), flow.Edges.end(),
			[nodeId](const VALTAN_PATTERN_FLOW_EDGE& edge)
			{
				return edge.strToNodeId == nodeId;
			});
		if (flow.Edges.end() == closesAtNewEntry)
		{
			outStatus =
				"This repeat graph cannot rotate to the selected start node without breaking its finite back-edge.";
			return false;
		}
		closesAtNewEntry->iMaxTraversals = cap;
		flow.strEntryNodeId = std::string(nodeId);
	}
	else
	{
		std::vector<std::string> orderedNodeIds;
		orderedNodeIds.reserve(flow.Nodes.size());
		std::string cursor = flow.strEntryNodeId;
		while (!cursor.empty())
		{
			if (orderedNodeIds.end() != std::find(
					orderedNodeIds.begin(), orderedNodeIds.end(), cursor))
				break;
			orderedNodeIds.push_back(cursor);
			const VALTAN_PATTERN_FLOW_EDGE* edge =
				Find_CompletedEdge(flow, cursor);
			cursor = nullptr == edge ? std::string{} : edge->strToNodeId;
		}
		if (orderedNodeIds.size() != flow.Nodes.size())
		{
			outStatus =
				"The current Flow path cannot be rotated because it is not one admitted acyclic chain.";
			return false;
		}
		const auto start = std::find(
			orderedNodeIds.begin(), orderedNodeIds.end(), nodeId);
		std::rotate(orderedNodeIds.begin(), start, orderedNodeIds.end());

		std::unordered_map<std::string, VALTAN_PATTERN_FLOW_NODE> nodesById;
		std::unordered_map<std::string, VALTAN_PATTERN_FLOW_EDGE> edgesBySource;
		for (const VALTAN_PATTERN_FLOW_NODE& node : flow.Nodes)
			nodesById.emplace(node.strNodeId, node);
		for (const VALTAN_PATTERN_FLOW_EDGE& edge : flow.Edges)
			edgesBySource.emplace(edge.strFromNodeId, edge);
		std::vector<VALTAN_PATTERN_FLOW_NODE> orderedNodes;
		std::vector<VALTAN_PATTERN_FLOW_EDGE> orderedEdges;
		orderedNodes.reserve(orderedNodeIds.size());
		orderedEdges.reserve(orderedNodeIds.size() - 1u);
		for (const std::string& orderedNodeId : orderedNodeIds)
			orderedNodes.push_back(nodesById.at(orderedNodeId));
		for (std::size_t index = 0u; index + 1u < orderedNodeIds.size(); ++index)
		{
			VALTAN_PATTERN_FLOW_EDGE edge;
			const auto oldEdge = edgesBySource.find(orderedNodeIds[index]);
			if (edgesBySource.end() != oldEdge)
				edge = oldEdge->second;
			else
			{
				if (flow.iNextEdgeOrdinal > MAX_EDGE_ORDINAL)
				{
					outStatus =
						"Valtan Boss Flow exhausted its stable edge ID range.";
					return false;
				}
				edge.strEdgeId = Build_OrdinalId(
					flow.strFlowId, "edge", flow.iNextEdgeOrdinal++);
				edge.iPursuitMs = flow.iDefaultPursuitMs;
			}
			edge.strFromNodeId = orderedNodeIds[index];
			edge.strToNodeId = orderedNodeIds[index + 1u];
			edge.eOutcome = VALTAN_PATTERN_FLOW_EDGE_OUTCOME::COMPLETED;
			edge.iMaxTraversals.reset();
			orderedEdges.push_back(std::move(edge));
		}
		flow.strEntryNodeId = std::string(nodeId);
		flow.Nodes = std::move(orderedNodes);
		flow.Edges = std::move(orderedEdges);
	}
	(void)Build_LegacyProjection(flow);
	if (!Validate(staged, admittedPatternIds, outStatus))
		return false;

	m_Draft = std::move(staged);
	outStatus = "Set Valtan Boss Flow start node to " + std::string(nodeId) +
		" and preserved one deterministic reachable path.";
	return true;
}

bool_t Client::CValtanPatternFlowDocument::Connect_CompletedEdge(
	const std::string_view fromNodeId,
	const std::string_view toNodeId,
	const std::uint32_t pursuitMs,
	const std::uint32_t maximumTraversals,
	const std::vector<std::string>& admittedPatternIds,
	std::string& outEdgeId,
	std::string& outStatus)
{
	outEdgeId.clear();
	outStatus.clear();
	const VALTAN_PATTERN_FLOW_DEFINITION* current = Get_DefaultFlow();
	if (nullptr == current || current->Edges.size() >= MAX_EDGES ||
		current->iNextEdgeOrdinal > MAX_EDGE_ORDINAL ||
		pursuitMs < MIN_INTER_STEP_PURSUIT_MS ||
		pursuitMs > MAX_INTER_STEP_PURSUIT_MS ||
		0u == maximumTraversals || maximumTraversals > MAX_EDGE_TRAVERSALS)
	{
		outStatus =
			"A Flow link needs 100..10000 ms pursuit and 1..255 finite back-edge traversals.";
		return false;
	}
	if (nullptr == Find_Node(*current, fromNodeId) ||
		nullptr == Find_Node(*current, toNodeId))
	{
		outStatus = "A Flow link endpoint no longer exists.";
		return false;
	}
	if (nullptr != Find_CompletedEdge(*current, fromNodeId))
	{
		outStatus =
			"The source node already owns its deterministic COMPLETED edge. Remove that edge before reconnecting.";
		return false;
	}

	VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT staged = m_Draft;
	VALTAN_PATTERN_FLOW_DEFINITION& flow = staged.Flows.front();
	const std::string edgeId = Build_OrdinalId(
		flow.strFlowId, "edge", flow.iNextEdgeOrdinal++);
	flow.Edges.push_back({
		edgeId, std::string(fromNodeId),
		VALTAN_PATTERN_FLOW_EDGE_OUTCOME::COMPLETED,
		std::string(toNodeId), pursuitMs, maximumTraversals });
	(void)Build_LegacyProjection(flow);
	if (!Validate(staged, admittedPatternIds, outStatus))
		return false;

	m_Draft = std::move(staged);
	outEdgeId = edgeId;
	outStatus = "Connected finite Valtan Boss Flow back-edge " + edgeId +
		" (maxTraversals=" + std::to_string(maximumTraversals) + ").";
	return true;
}

bool_t Client::CValtanPatternFlowDocument::Remove_Edge(
	const std::string_view edgeId,
	const std::vector<std::string>& admittedPatternIds,
	std::string& outStatus)
{
	outStatus.clear();
	const VALTAN_PATTERN_FLOW_DEFINITION* current = Get_DefaultFlow();
	if (nullptr == current)
	{
		outStatus = "Load the Valtan Boss Flow before removing an edge.";
		return false;
	}
	VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT staged = m_Draft;
	VALTAN_PATTERN_FLOW_DEFINITION& flow = staged.Flows.front();
	const auto edge = std::find_if(
		flow.Edges.begin(), flow.Edges.end(),
		[edgeId](const VALTAN_PATTERN_FLOW_EDGE& candidate)
		{
			return candidate.strEdgeId == edgeId;
		});
	if (flow.Edges.end() == edge)
	{
		outStatus = "The selected Valtan Boss Flow edge no longer exists.";
		return false;
	}
	flow.Edges.erase(edge);
	(void)Build_LegacyProjection(flow);
	if (!Validate(staged, admittedPatternIds, outStatus))
	{
		outStatus =
			"Removing this edge would orphan part of the graph. Remove its nodes or reconnect atomically instead: " +
			outStatus;
		return false;
	}
	m_Draft = std::move(staged);
	outStatus = "Removed Valtan Boss Flow edge " + std::string(edgeId) + ".";
	return true;
}

bool_t Client::CValtanPatternFlowDocument::Set_EdgePursuitMs(
	const std::string_view edgeId,
	const std::uint32_t pursuitMs,
	const std::vector<std::string>& admittedPatternIds,
	std::string& outStatus)
{
	outStatus.clear();
	if (!m_bReady || pursuitMs < MIN_INTER_STEP_PURSUIT_MS ||
		pursuitMs > MAX_INTER_STEP_PURSUIT_MS)
	{
		outStatus = "Flow edge pursuit must be within 100..10000 ms.";
		return false;
	}
	VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT staged = m_Draft;
	VALTAN_PATTERN_FLOW_DEFINITION& flow = staged.Flows.front();
	const auto edge = std::find_if(
		flow.Edges.begin(), flow.Edges.end(),
		[edgeId](const VALTAN_PATTERN_FLOW_EDGE& candidate)
		{
			return candidate.strEdgeId == edgeId;
		});
	if (flow.Edges.end() == edge)
	{
		outStatus = "The selected Valtan Boss Flow edge no longer exists.";
		return false;
	}
	edge->iPursuitMs = pursuitMs;
	(void)Build_LegacyProjection(flow);
	if (!Validate(staged, admittedPatternIds, outStatus))
		return false;
	m_Draft = std::move(staged);
	outStatus = "Updated Valtan Boss Flow edge pursuit.";
	return true;
}

bool_t Client::CValtanPatternFlowDocument::Set_EdgeMaxTraversals(
	const std::string_view edgeId,
	const std::uint32_t maximumTraversals,
	const std::vector<std::string>& admittedPatternIds,
	std::string& outStatus)
{
	outStatus.clear();
	if (!m_bReady || 0u == maximumTraversals ||
		maximumTraversals > MAX_EDGE_TRAVERSALS)
	{
		outStatus = "Flow back-edge traversals must be within 1..255.";
		return false;
	}
	VALTAN_PATTERN_FLOW_AUTHORING_DOCUMENT staged = m_Draft;
	VALTAN_PATTERN_FLOW_DEFINITION& flow = staged.Flows.front();
	const auto edge = std::find_if(
		flow.Edges.begin(), flow.Edges.end(),
		[edgeId](const VALTAN_PATTERN_FLOW_EDGE& candidate)
		{
			return candidate.strEdgeId == edgeId;
		});
	if (flow.Edges.end() == edge)
	{
		outStatus = "The selected Valtan Boss Flow edge no longer exists.";
		return false;
	}
	edge->iMaxTraversals = maximumTraversals;
	(void)Build_LegacyProjection(flow);
	if (!Validate(staged, admittedPatternIds, outStatus))
		return false;
	m_Draft = std::move(staged);
	outStatus = "Updated finite Valtan Boss Flow repeat traversal cap.";
	return true;
}

bool Client::CValtanPatternFlowDocument::Has_LegacyLinearProjection(
	const VALTAN_PATTERN_FLOW_DEFINITION& flow) noexcept
{
	return !flow.Nodes.empty() && flow.Slots.size() == flow.Nodes.size();
}

bool_t Client::CValtanPatternFlowDocument::Is_Dirty() const noexcept
{
	return m_bReady && !Documents_AreEqual(m_Baseline, m_Draft);
}

const Client::VALTAN_PATTERN_FLOW_DEFINITION*
Client::CValtanPatternFlowDocument::Get_DefaultFlow() const noexcept
{
	return m_bReady && 1u == m_Draft.Flows.size() ?
		&m_Draft.Flows.front() : nullptr;
}
