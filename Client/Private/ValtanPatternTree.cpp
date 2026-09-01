#include "ValtanPatternTree.h"

#include "AnimationSkillBindingDocument.h"
#include "DataJson.h"
#include "EncounterPatternReference.h"
#include "ProjectDataRoot.h"
#include "ValtanPatternEffectCueDocument.h"
#include "ValtanPatternFlowDocument.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <initializer_list>
#include <limits>
#include <locale>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string_view>

namespace
{
	using Client::DATA_JSON_TYPE;
	using Client::DATA_JSON_VALUE;
	constexpr const wchar_t* VALTAN_PATTERN_TRANSACTION_LOCK_RELATIVE =
		L"out\\ValtanPatternTransactions\\create-pattern.lock";
	constexpr const wchar_t* VALTAN_PATTERN_ACTIVE_GENERATION_RELATIVE =
		L"out\\ValtanPatternTransactions\\active-generation.json";

	/* Canonical readers share the same byte range that every Python/C++
	   Create/Project writer locks exclusively.  Holding the handle through the
	   entire strict join prevents one reader from observing a mixture of owner
	   and generated Product generations.  This reader never repairs a crashed
	   writer: a durable active-generation journal is an explicit admission
	   failure that the next writer transaction must recover. */
	class SCOPED_VALTAN_PATTERN_TRANSACTION_READ_LOCK final
	{
	public:
		~SCOPED_VALTAN_PATTERN_TRANSACTION_READ_LOCK()
		{
			if (INVALID_HANDLE_VALUE == m_hFile)
				return;
			UnlockFileEx(m_hFile, 0u, 1u, 0u, &m_Overlap);
			CloseHandle(m_hFile);
		}

		bool_t Try_Acquire(
			const std::filesystem::path& ProjectRoot,
			std::string& strOutError)
		{
			if (ProjectRoot.empty())
			{
				strOutError = "project root is unavailable";
				return false;
			}
			const std::filesystem::path LockPath =
				ProjectRoot / VALTAN_PATTERN_TRANSACTION_LOCK_RELATIVE;
			std::error_code DirectoryError;
			std::filesystem::create_directories(
				LockPath.parent_path(), DirectoryError);
			if (DirectoryError)
			{
				strOutError = "lock directory creation failed: " +
					DirectoryError.message();
				return false;
			}

			m_hFile = CreateFileW(
				LockPath.c_str(), GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
				nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (INVALID_HANDLE_VALUE == m_hFile)
			{
				strOutError = "lock open failed with Win32 error " +
					std::to_string(GetLastError());
				return false;
			}

			/* Python's msvcrt writer initializes the same one-byte range.
			   Initialize it before taking the shared lock so a first writer never
			   needs to write through an already locked, zero-length file. */
			LARGE_INTEGER Size{};
			if (FALSE == GetFileSizeEx(m_hFile, &Size))
			{
				strOutError = "lock size query failed with Win32 error " +
					std::to_string(GetLastError());
				CloseHandle(m_hFile);
				m_hFile = INVALID_HANDLE_VALUE;
				return false;
			}
			if (Size.QuadPart < 1)
			{
				const char_t Byte = '\0';
				DWORD iWritten = 0u;
				LARGE_INTEGER Begin{};
				if (FALSE == SetFilePointerEx(
						m_hFile, Begin, nullptr, FILE_BEGIN) ||
					FALSE == WriteFile(
						m_hFile, &Byte, 1u, &iWritten, nullptr) ||
					1u != iWritten || FALSE == FlushFileBuffers(m_hFile))
				{
					strOutError =
						"lock initialization failed with Win32 error " +
						std::to_string(GetLastError());
					CloseHandle(m_hFile);
					m_hFile = INVALID_HANDLE_VALUE;
					return false;
				}
			}

			/* Omitting LOCKFILE_EXCLUSIVE_LOCK is the Windows shared/read mode.
			   FAIL_IMMEDIATELY keeps a UI reload bounded while a publisher owns
			   the corresponding exclusive Python msvcrt byte-range lock. */
			if (FALSE == LockFileEx(m_hFile, LOCKFILE_FAIL_IMMEDIATELY,
				0u, 1u, 0u, &m_Overlap))
			{
				strOutError =
					"Create/Project transaction is active (Win32 " +
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

	struct VALTAN_CANONICAL_PRODUCT_READ_STATE final
	{
		SCOPED_VALTAN_PATTERN_TRANSACTION_READ_LOCK Lock;
		std::filesystem::path ProjectRoot;
	};

	bool_t Require_NoActiveValtanPatternGeneration(
		const std::filesystem::path& ProjectRoot,
		std::string& strOutError)
	{
		const std::filesystem::path ActiveGeneration =
			ProjectRoot / VALTAN_PATTERN_ACTIVE_GENERATION_RELATIVE;
		std::error_code ExistsError;
		const bool_t bExists =
			std::filesystem::exists(ActiveGeneration, ExistsError);
		if (ExistsError)
		{
			strOutError = "active-generation journal query failed: " +
				ExistsError.message();
			return false;
		}
		if (bExists)
		{
			strOutError =
				"active-generation journal requires writer recovery: " +
				ActiveGeneration.generic_string();
			return false;
		}
		return true;
	}

	constexpr std::string_view CINEMATIC_ENTRY_PATTERN_ID =
		"VALTAN_ENTRANCE_CINEMATIC";
	constexpr std::string_view IDLE_CINEMATIC_ENTRY_PATTERN_ID =
		"VALTAN_ENTRANCE_CINEMATIC_IDLE";

	bool_t Is_OptionalEntryPatternId(const std::string_view PatternId)
	{
		return CINEMATIC_ENTRY_PATTERN_ID == PatternId ||
			IDLE_CINEMATIC_ENTRY_PATTERN_ID == PatternId;
	}

	bool Read_TextDocument(
		const std::filesystem::path& Path,
		std::string& OutText,
		std::string& strOutError)
	{
		if (Path.empty())
		{
			strOutError = "path escaped the Data root";
			return false;
		}
		std::ifstream Input(Path, std::ios::binary);
		if (!Input)
		{
			strOutError = "could not open " + Path.string();
			return false;
		}
		OutText.assign(std::istreambuf_iterator<char>(Input),
			std::istreambuf_iterator<char>());
		return true;
	}

	bool Parse_Document(
		const std::filesystem::path& Relative,
		DATA_JSON_VALUE& OutRoot,
		std::string& strOutError)
	{
		std::string Text;
		if (!Read_TextDocument(
				Client::CProjectDataRoot::Resolve(Relative), Text, strOutError))
		{
			return false;
		}
		std::string ParseError;
		if (!Client::CDataJson::Parse(Text, OutRoot, ParseError) ||
			!OutRoot.Is_Object())
		{
			strOutError = Relative.generic_string() + ": " + ParseError;
			return false;
		}
		return true;
	}

	bool Parse_DocumentPath(
		const std::filesystem::path& Path,
		DATA_JSON_VALUE& OutRoot,
		std::string& strOutError)
	{
		std::string Text;
		if (!Read_TextDocument(Path, Text, strOutError))
			return false;
		std::string ParseError;
		if (!Client::CDataJson::Parse(Text, OutRoot, ParseError) ||
			!OutRoot.Is_Object())
		{
			strOutError = Path.generic_string() + ": " + ParseError;
			return false;
		}
		return true;
	}

	std::string Read_String(
		const DATA_JSON_VALUE& Object, const std::string_view Key)
	{
		const DATA_JSON_VALUE* pValue = Object.Find(Key);
		return nullptr == pValue || DATA_JSON_TYPE::STRING != pValue->Get_Type() ?
			std::string{} : pValue->Get_String();
	}

	double Read_Number(
		const DATA_JSON_VALUE& Object, const std::string_view Key)
	{
		const DATA_JSON_VALUE* pValue = Object.Find(Key);
		return nullptr == pValue || DATA_JSON_TYPE::NUMBER != pValue->Get_Type() ?
			0.0 : pValue->Get_Number();
	}

	bool_t Read_RequiredUInt32(
		const DATA_JSON_VALUE& Object,
		const std::string_view Key,
		uint32_t& OutValue)
	{
		const DATA_JSON_VALUE* pValue = Object.Find(Key);
		if (nullptr == pValue || !pValue->Is_Number() ||
			!std::isfinite(pValue->Get_Number()) ||
			std::floor(pValue->Get_Number()) != pValue->Get_Number() ||
			pValue->Get_Number() < 0.0 ||
			pValue->Get_Number() > static_cast<double>(
				(std::numeric_limits<uint32_t>::max)()))
		{
			return false;
		}
		OutValue = static_cast<uint32_t>(pValue->Get_Number());
		return true;
	}

	bool_t Read_RequiredFiniteFloat(
		const DATA_JSON_VALUE& Object,
		const std::string_view Key,
		f32_t& OutValue)
	{
		const DATA_JSON_VALUE* pValue = Object.Find(Key);
		if (nullptr == pValue || !pValue->Is_Number() ||
			!std::isfinite(pValue->Get_Number()) ||
			std::abs(pValue->Get_Number()) > static_cast<double>(
				(std::numeric_limits<f32_t>::max)()))
		{
			return false;
		}
		OutValue = static_cast<f32_t>(pValue->Get_Number());
		return true;
	}

	const DATA_JSON_VALUE* Required(
		const DATA_JSON_VALUE& Object,
		const std::string_view Key,
		const DATA_JSON_TYPE eType)
	{
		const DATA_JSON_VALUE* pValue = Object.Find(Key);
		return nullptr != pValue && pValue->Get_Type() == eType ?
			pValue : nullptr;
	}

	bool_t Has_ExactProperties(
		const DATA_JSON_VALUE& Object,
		const std::initializer_list<std::string_view> Names)
	{
		if (!Object.Is_Object() || Object.Get_Object().size() != Names.size())
			return false;
		for (const std::string_view Name : Names)
		{
			if (nullptr == Object.Find(Name))
				return false;
		}
		return true;
	}

	bool_t Has_ExactPropertiesWithOptional(
		const DATA_JSON_VALUE& Object,
		const std::initializer_list<std::string_view> RequiredNames,
		const std::initializer_list<std::string_view> OptionalNames)
	{
		if (!Object.Is_Object())
			return false;
		for (const std::string_view Name : RequiredNames)
		{
			if (nullptr == Object.Find(Name))
				return false;
		}
		size_t iExpectedPropertyCount = RequiredNames.size();
		for (const std::string_view Name : OptionalNames)
		{
			if (nullptr != Object.Find(Name))
				++iExpectedPropertyCount;
		}
		return Object.Get_Object().size() == iExpectedPropertyCount;
	}

	bool_t Is_StableToken(const std::string_view Value)
	{
		if (Value.empty() || Value.size() > 255u)
			return false;
		for (const unsigned char Character : Value)
		{
			if (!(Character >= 'a' && Character <= 'z') &&
				!(Character >= 'A' && Character <= 'Z') &&
				!(Character >= '0' && Character <= '9') &&
				'_' != Character && '-' != Character && '.' != Character)
			{
				return false;
			}
		}
		return true;
	}

	bool_t Is_FiniteNumber(const DATA_JSON_VALUE* pValue)
	{
		return nullptr != pValue && pValue->Is_Number() &&
			std::isfinite(pValue->Get_Number());
	}

	bool_t Is_NonNegativeInteger(const DATA_JSON_VALUE* pValue)
	{
		return Is_FiniteNumber(pValue) && pValue->Get_Number() >= 0.0 &&
			std::floor(pValue->Get_Number()) == pValue->Get_Number() &&
			pValue->Get_Number() <= static_cast<double>(
				(std::numeric_limits<uint32_t>::max)());
	}

	bool_t Read_NullableStableToken(
		const DATA_JSON_VALUE& Object,
		const std::string_view Key,
		std::string& OutValue)
	{
		const DATA_JSON_VALUE* pValue = Object.Find(Key);
		if (nullptr == pValue)
			return false;
		if (pValue->Is_Null())
		{
			OutValue.clear();
			return true;
		}
		if (!pValue->Is_String() || !Is_StableToken(pValue->Get_String()))
			return false;
		OutValue = pValue->Get_String();
		return true;
	}

	bool_t Read_StageMotion(
		const DATA_JSON_VALUE* pValue,
		const uint32_t iStageDurationMs,
		std::optional<Client::VALTAN_STAGE_MOTION_VIEW>& Out)
	{
		Out.reset();
		if (nullptr == pValue || pValue->Is_Null())
			return true;
		const DATA_JSON_VALUE* pKind = Required(
			*pValue, "kind", DATA_JSON_TYPE::STRING);
		Client::VALTAN_STAGE_MOTION_VIEW Motion;
		if (nullptr == pKind)
			return false;
		Motion.strKind = pKind->Get_String();
		if ("PORTAL_TARGET_RUSH" == Motion.strKind)
		{
			if (!Has_ExactProperties(*pValue,
					{ "kind", "retargetDelayMs", "speedMps", "distanceM" }) ||
				!Read_RequiredUInt32(
					*pValue, "retargetDelayMs", Motion.iRetargetDelayMs) ||
				!Read_RequiredFiniteFloat(
					*pValue, "speedMps", Motion.fSpeedMps) ||
				!Read_RequiredFiniteFloat(
					*pValue, "distanceM", Motion.fDistance) ||
				Motion.fSpeedMps <= 0.f || Motion.fSpeedMps > 1000.f ||
				Motion.fDistance <= 0.f || Motion.fDistance > 1000.f ||
				static_cast<double>(Motion.iRetargetDelayMs) +
					static_cast<double>(Motion.fDistance) /
					static_cast<double>(Motion.fSpeedMps) * 1000.0 >
					static_cast<double>(iStageDurationMs) + 0.000001)
				return false;
		}
		else if ("PORTAL_CROSS_ARENA" == Motion.strKind)
		{
			const DATA_JSON_VALUE* pExtents = Required(
				*pValue, "halfExtentsM", DATA_JSON_TYPE::ARRAY);
			if (!Has_ExactProperties(*pValue, { "kind", "cornerIndex", "halfExtentsM" }) ||
				!Read_RequiredUInt32(*pValue, "cornerIndex", Motion.iCornerIndex) ||
				Motion.iCornerIndex > 3u || nullptr == pExtents ||
				pExtents->Get_Array().size() != 2u)
				return false;
			for (size_t iAxis = 0u; iAxis < 2u; ++iAxis)
			{
				const DATA_JSON_VALUE& Extent = pExtents->Get_Array()[iAxis];
				if (!Is_FiniteNumber(&Extent) || Extent.Get_Number() < 1.0 ||
					Extent.Get_Number() > 100.0)
					return false;
				Motion.HalfExtentsM[iAxis] = static_cast<f32_t>(Extent.Get_Number());
			}
		}
		else if ("FORWARD" != Motion.strKind ||
			!Has_ExactProperties(*pValue, { "kind", "distance" }) ||
			!Read_RequiredFiniteFloat(*pValue, "distance", Motion.fDistance))
		{
			return false;
		}
		Out = std::move(Motion);
		return true;
	}

	bool_t Read_StageActions(
		const DATA_JSON_VALUE* pValue,
		const uint32_t iStageDurationMs,
		std::vector<Client::VALTAN_STAGE_ACTION_VIEW>& Out)
	{
		Out.clear();
		if (nullptr == pValue)
			return true;
		if (!pValue->Is_Array())
			return false;
		for (const DATA_JSON_VALUE& Value : pValue->Get_Array())
		{
			if ("SPAWN_COMBAT_OBJECT_VOLLEY" == Read_String(Value, "kind"))
			{
				const DATA_JSON_VALUE* pTargeting = Required(
					Value, "targetingPolicy", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* pLayout = Required(
					Value, "layout", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* pAllowOverlap = Required(
					Value, "allowOverlap", DATA_JSON_TYPE::BOOLEAN);
				const DATA_JSON_VALUE* pArenaAnchor = Required(
					Value, "arenaAnchorPolicy", DATA_JSON_TYPE::STRING);
				const std::string strTargeting = nullptr == pTargeting ?
					std::string{} : pTargeting->Get_String();
				const bool_t bPerAlivePlayer =
					"PER_ALIVE_PLAYER" == strTargeting;
				const bool_t bBossRelative = "BOSS_RELATIVE" == strTargeting;
				if (!Has_ExactProperties(Value,
						{ "trigger", "kind", "targetId", "targetingPolicy",
						  "countPerResolvedTarget", "layout", "radiusM",
						  "startAngleDegrees", "angleStepDegrees", "allowOverlap",
						  "maximumTotalObjects", "spawnCount", "spawnIntervalMs",
						  "arenaRandomCount", "arenaRandomRadiusM",
						  "arenaHeightToleranceM", "arenaAnchorPolicy" }) ||
					nullptr == Required(Value, "trigger", DATA_JSON_TYPE::STRING) ||
					"ENTER" != Read_String(Value, "trigger") ||
					nullptr == Required(Value, "targetId", DATA_JSON_TYPE::STRING) ||
					nullptr == pTargeting ||
					(!bPerAlivePlayer && !bBossRelative) ||
					nullptr == pLayout ||
					("SINGLE" != pLayout->Get_String() &&
					 "RADIAL" != pLayout->Get_String()) ||
					!Is_NonNegativeInteger(Value.Find("countPerResolvedTarget")) ||
					0.0 == Value.Find("countPerResolvedTarget")->Get_Number() ||
					Value.Find("countPerResolvedTarget")->Get_Number() > 8.0 ||
					!Is_FiniteNumber(Value.Find("radiusM")) ||
					!Is_FiniteNumber(Value.Find("startAngleDegrees")) ||
					!Is_FiniteNumber(Value.Find("angleStepDegrees")) ||
					nullptr == pAllowOverlap ||
					!Is_NonNegativeInteger(Value.Find("maximumTotalObjects")) ||
					0.0 == Value.Find("maximumTotalObjects")->Get_Number() ||
					Value.Find("maximumTotalObjects")->Get_Number() > 64.0 ||
					!Is_NonNegativeInteger(Value.Find("spawnCount")) ||
					0.0 == Value.Find("spawnCount")->Get_Number() ||
					Value.Find("spawnCount")->Get_Number() > 8.0 ||
					!Is_NonNegativeInteger(Value.Find("spawnIntervalMs")) ||
					Value.Find("spawnIntervalMs")->Get_Number() >
						static_cast<double>(iStageDurationMs) ||
					(Value.Find("spawnCount")->Get_Number() > 1.0 &&
					 0.0 == Value.Find("spawnIntervalMs")->Get_Number()) ||
					(Value.Find("spawnCount")->Get_Number() == 1.0 &&
					 0.0 != Value.Find("spawnIntervalMs")->Get_Number()) ||
					(Value.Find("spawnCount")->Get_Number() - 1.0) *
						Value.Find("spawnIntervalMs")->Get_Number() >=
						static_cast<double>(iStageDurationMs) ||
					!Is_NonNegativeInteger(Value.Find("arenaRandomCount")) ||
					Value.Find("arenaRandomCount")->Get_Number() > 32.0 ||
					!Is_FiniteNumber(Value.Find("arenaRandomRadiusM")) ||
					Value.Find("arenaRandomRadiusM")->Get_Number() < 0.0 ||
					Value.Find("arenaRandomRadiusM")->Get_Number() > 1000.0 ||
					!Is_FiniteNumber(Value.Find("arenaHeightToleranceM")) ||
					Value.Find("arenaHeightToleranceM")->Get_Number() < 0.0 ||
					Value.Find("arenaHeightToleranceM")->Get_Number() > 1000.0 ||
					nullptr == pArenaAnchor ||
					Value.Find("maximumTotalObjects")->Get_Number() <
						Value.Find("countPerResolvedTarget")->Get_Number() +
						Value.Find("arenaRandomCount")->Get_Number())
				{
					return false;
				}
				const double fCount =
					Value.Find("countPerResolvedTarget")->Get_Number();
				const double fRadius = Value.Find("radiusM")->Get_Number();
				const double fStartAngle =
					Value.Find("startAngleDegrees")->Get_Number();
				const double fAngleStep =
					Value.Find("angleStepDegrees")->Get_Number();
				const bool_t bSingle = 1.0 == fCount;
				if ((bSingle && ("SINGLE" != pLayout->Get_String() ||
						0.0 != fRadius || 0.0 != fStartAngle ||
						0.0 != fAngleStep || pAllowOverlap->Get_Boolean())) ||
					(!bSingle && ("RADIAL" != pLayout->Get_String() ||
						fRadius <= 0.0 || fAngleStep <= 0.0 ||
						fAngleStep * fCount > 360.000001 ||
						pAllowOverlap->Get_Boolean())))
				{
					return false;
				}
				const bool_t bPerAlivePlayerContract = bPerAlivePlayer &&
					"BOSS_SPAWN_POSITION" == pArenaAnchor->Get_String() &&
					Value.Find("arenaRandomCount")->Get_Number() > 0.0 &&
					Value.Find("arenaRandomRadiusM")->Get_Number() > 0.0 &&
					Value.Find("arenaHeightToleranceM")->Get_Number() > 0.0;
				const bool_t bBossRelativeContract = bBossRelative &&
					"RADIAL" == pLayout->Get_String() && fCount >= 2.0 &&
					fRadius > 0.0 && fAngleStep > 0.0 &&
					fAngleStep * fCount <= 360.000001 &&
					Value.Find("maximumTotalObjects")->Get_Number() >= fCount &&
					1.0 == Value.Find("spawnCount")->Get_Number() &&
					0.0 == Value.Find("spawnIntervalMs")->Get_Number() &&
					0.0 == Value.Find("arenaRandomCount")->Get_Number() &&
					0.0 == Value.Find("arenaRandomRadiusM")->Get_Number() &&
					0.0 == Value.Find("arenaHeightToleranceM")->Get_Number() &&
					"NONE" == pArenaAnchor->Get_String();
				if (!bPerAlivePlayerContract && !bBossRelativeContract)
					return false;
				/* The expanded volley is validated and joined separately as a
				   combat-object owner. It is not lossy-packed into the legacy
				   five-field stage-action display view. */
				continue;
			}
			if ("RELEASE_GRABBED_PLAYERS" == Read_String(Value, "kind"))
			{
				const DATA_JSON_VALUE* pTrigger = Required(
					Value, "trigger", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* pKind = Required(
					Value, "kind", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* pTarget = Required(
					Value, "targetId", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* pReleaseMode = Required(
					Value, "releaseMode", DATA_JSON_TYPE::STRING);
				Client::VALTAN_STAGE_ACTION_VIEW Action;
				if (!Has_ExactProperties(Value,
						{ "trigger", "kind", "targetId", "releaseMode",
						  "speedMps", "durationMs", "yawOffsetDegrees" }) ||
					nullptr == pTrigger || nullptr == pKind ||
					nullptr == pTarget || nullptr == pReleaseMode ||
					("ENTER" != pTrigger->Get_String() &&
					 "EXIT" != pTrigger->Get_String()) ||
					"boss.attachment.left-hand" != pTarget->Get_String() ||
					!Read_RequiredFiniteFloat(
						Value, "speedMps", Action.fSpeedMps) ||
					!Read_RequiredUInt32(
						Value, "durationMs", Action.iDurationMs) ||
					!Read_RequiredFiniteFloat(
						Value, "yawOffsetDegrees", Action.fYawOffsetDegrees) ||
					Action.fSpeedMps < 0.f || Action.fSpeedMps > 50.f ||
					Action.iDurationMs > 5000u ||
					std::abs(Action.fYawOffsetDegrees) > 180.f)
				{
					return false;
				}
				const bool_t bHold = "HOLD" == pReleaseMode->Get_String() &&
					0.f == Action.fSpeedMps && 0u == Action.iDurationMs &&
					0.f == Action.fYawOffsetDegrees;
				const bool_t bOppositeKnockback =
					("OPPOSITE_KNOCKBACK" == pReleaseMode->Get_String() ||
					 "ARENA_EJECTION" == pReleaseMode->Get_String()) &&
					Action.fSpeedMps > 0.f && Action.iDurationMs > 0u &&
					("ARENA_EJECTION" == pReleaseMode->Get_String() ||
					 0.f == Action.fYawOffsetDegrees);
				if (!bHold && !bOppositeKnockback)
					return false;
				Action.strTrigger = pTrigger->Get_String();
				Action.strKind = pKind->Get_String();
				Action.strTargetId = pTarget->Get_String();
				Action.strReleaseMode = pReleaseMode->Get_String();
				Out.push_back(std::move(Action));
				continue;
			}
			if (!Has_ExactProperties(Value,
					{ "trigger", "kind", "targetId", "value", "durationMs" }))
			{
				return false;
			}
			const DATA_JSON_VALUE* pTrigger = Required(
				Value, "trigger", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pKind = Required(
				Value, "kind", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pTarget = Required(
				Value, "targetId", DATA_JSON_TYPE::STRING);
			Client::VALTAN_STAGE_ACTION_VIEW Action;
			if (nullptr == pTrigger || nullptr == pKind || nullptr == pTarget ||
				!Read_RequiredFiniteFloat(Value, "value", Action.fValue) ||
				!Read_RequiredUInt32(Value, "durationMs", Action.iDurationMs))
			{
				return false;
			}
			Action.strTrigger = pTrigger->Get_String();
			Action.strKind = pKind->Get_String();
			Action.strTargetId = pTarget->Get_String();
			if ("RETURN_TO_ARENA_CENTER" == Action.strKind ||
				"SPAWN_COMBAT_OBJECT" == Action.strKind)
			{
				const bool_t bTargetValid = "RETURN_TO_ARENA_CENTER" == Action.strKind ?
					"boss.arena.center" == Action.strTargetId :
					Is_StableToken(Action.strTargetId);
				if ("ENTER" != Action.strTrigger || !bTargetValid ||
					1.f != Action.fValue || 0u != Action.iDurationMs)
					return false;
			}
			if ("DAMAGE_GRABBED_PLAYERS" == Action.strKind ||
				"EXECUTE_GRABBED_PLAYERS" == Action.strKind)
			{
				const bool_t bTargetValid = "DAMAGE_GRABBED_PLAYERS" == Action.strKind ?
					(Is_StableToken(Action.strTargetId) && Action.strTargetId.starts_with("damage.")) :
					"boss.attachment.left-hand" == Action.strTargetId;
				if ("ENTER" != Action.strTrigger || !bTargetValid ||
					0.f != Action.fValue || 0u != Action.iDurationMs)
					return false;
			}
			Out.push_back(std::move(Action));
		}
		return true;
	}

	bool_t Read_StageBranches(
		const DATA_JSON_VALUE* pValue,
		std::vector<Client::VALTAN_STAGE_BRANCH_VIEW>& Out)
	{
		Out.clear();
		if (nullptr == pValue)
			return true;
		if (!pValue->Is_Array())
			return false;
		std::set<std::string, std::less<>> Outcomes;
		for (const DATA_JSON_VALUE& Value : pValue->Get_Array())
		{
			if (!Has_ExactPropertiesWithOptional(
					Value, { "outcome", "nextActionId" }, { "nextPatternId" }))
				return false;
			const DATA_JSON_VALUE* pOutcome = Required(
				Value, "outcome", DATA_JSON_TYPE::STRING);
			std::string strNextActionId;
			std::string strNextPatternId;
			const DATA_JSON_VALUE* const pNextPatternId =
				Value.Find("nextPatternId");
			if (nullptr == pOutcome || !Is_StableToken(pOutcome->Get_String()) ||
				!Outcomes.insert(pOutcome->Get_String()).second ||
				!Read_NullableStableToken(
					Value, "nextActionId", strNextActionId) ||
				(nullptr != pNextPatternId &&
				 !Read_NullableStableToken(
					Value, "nextPatternId", strNextPatternId)) ||
				(!strNextActionId.empty() && !strNextPatternId.empty()))
			{
				return false;
			}
			Client::VALTAN_STAGE_BRANCH_VIEW Branch;
			Branch.strOutcome = pOutcome->Get_String();
			if (!strNextActionId.empty())
				Branch.strNextActionId = std::move(strNextActionId);
			if (!strNextPatternId.empty())
				Branch.strNextPatternId = std::move(strNextPatternId);
			Out.push_back(std::move(Branch));
		}
		return true;
	}

	bool_t Has_ValidNavigationBlockedCapture(
		const std::vector<Client::VALTAN_STAGE_BRANCH_VIEW>& Branches,
		const std::string_view strPlayerResponse)
	{
		return "CAPTURE" == strPlayerResponse || std::none_of(
			Branches.begin(), Branches.end(),
			[](const Client::VALTAN_STAGE_BRANCH_VIEW& Branch)
			{ return "NAVIGATION_BLOCKED" == Branch.strOutcome; });
	}

	bool_t Has_ClosedSplitStageFlag(
		const DATA_JSON_VALUE& Stage,
		const std::string_view strFlagId)
	{
		const DATA_JSON_VALUE* pEvents = Required(
			Stage, "events", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pEvents)
			return false;
		bool_t bEntered = false;
		bool_t bExited = false;
		for (const DATA_JSON_VALUE& Event : pEvents->Get_Array())
		{
			if ("SET_BOSS_FLAG" != Read_String(Event, "kind") ||
				strFlagId != Read_String(Event, "flagId"))
			{
				continue;
			}
			const DATA_JSON_VALUE* pEnabled = Required(
				Event, "enabled", DATA_JSON_TYPE::BOOLEAN);
			if (nullptr == pEnabled)
				continue;
			const std::string strTrigger = Read_String(Event, "trigger");
			bEntered = bEntered ||
				("ENTER" == strTrigger && pEnabled->Get_Boolean());
			bExited = bExited ||
				("EXIT" == strTrigger && !pEnabled->Get_Boolean());
		}
		return bEntered && bExited;
	}

	bool_t Read_StageGameplayExtensions(
		const DATA_JSON_VALUE& Stage,
		std::string& strOutPartDamagePolicy,
		std::optional<Client::VALTAN_COUNTER_PROXY_VIEW>& OutCounterProxy)
	{
		strOutPartDamagePolicy = "NORMAL";
		OutCounterProxy.reset();

		const DATA_JSON_VALUE* pPartDamagePolicy =
			Stage.Find("partDamagePolicy");
		if (nullptr != pPartDamagePolicy)
		{
			if (!pPartDamagePolicy->Is_String() ||
				("NORMAL" != pPartDamagePolicy->Get_String() &&
				 "DESTROY_FIRST_ELIGIBLE" !=
					pPartDamagePolicy->Get_String()))
			{
				return false;
			}
			strOutPartDamagePolicy = pPartDamagePolicy->Get_String();
		}

		const DATA_JSON_VALUE* pCounterProxy = Stage.Find("counterProxy");
		if (nullptr == pCounterProxy)
			return true;

		Client::VALTAN_COUNTER_PROXY_VIEW CounterProxy;
		if (!Has_ExactProperties(*pCounterProxy,
				{ "space", "forwardOffsetM", "rightOffsetM", "radiusM" }) ||
			"BOSS_LOCAL" != Read_String(*pCounterProxy, "space") ||
			!Read_RequiredFiniteFloat(*pCounterProxy, "forwardOffsetM",
				CounterProxy.fForwardOffsetM) ||
			!Read_RequiredFiniteFloat(*pCounterProxy, "rightOffsetM",
				CounterProxy.fRightOffsetM) ||
			!Read_RequiredFiniteFloat(*pCounterProxy, "radiusM",
				CounterProxy.fRadiusM) ||
			CounterProxy.fForwardOffsetM < -20.f ||
			CounterProxy.fForwardOffsetM > 20.f ||
			CounterProxy.fRightOffsetM < -20.f ||
			CounterProxy.fRightOffsetM > 20.f ||
			CounterProxy.fRadiusM < 0.1f || CounterProxy.fRadiusM > 20.f)
		{
			return false;
		}
		CounterProxy.strSpace = "BOSS_LOCAL";
		OutCounterProxy = std::move(CounterProxy);
		return true;
	}

	bool_t Read_StageHitAuthority(
		const DATA_JSON_VALUE& Stage,
		const uint32_t iStageDurationMs,
		bool_t& bOutHasAnchor,
		std::string& strOutAnchorKind,
		f32_t& fOutForwardOffsetM,
		f32_t& fOutRightOffsetM,
		f32_t& fOutYawOffsetDegrees,
		bool_t& bOutHasActivation,
		uint32_t& iOutStartMs,
		uint32_t& iOutLifetimeMs)
	{
		bOutHasAnchor = false;
		strOutAnchorKind = "BOSS_CURRENT";
		fOutForwardOffsetM = 0.f;
		fOutRightOffsetM = 0.f;
		fOutYawOffsetDegrees = 0.f;
		bOutHasActivation = false;
		iOutStartMs = 0u;
		iOutLifetimeMs = 0u;

		const DATA_JSON_VALUE* pAnchor = Stage.Find("hitAnchor");
		if (nullptr != pAnchor)
		{
			if (!Has_ExactProperties(*pAnchor,
					{ "kind", "forwardOffsetM", "rightOffsetM",
					  "yawOffsetDegrees" }) ||
				!Read_RequiredFiniteFloat(*pAnchor, "forwardOffsetM",
					fOutForwardOffsetM) ||
				!Read_RequiredFiniteFloat(*pAnchor, "rightOffsetM",
					fOutRightOffsetM) ||
				!Read_RequiredFiniteFloat(*pAnchor, "yawOffsetDegrees",
					fOutYawOffsetDegrees) ||
				std::fabs(fOutForwardOffsetM) > 1000.f ||
				std::fabs(fOutRightOffsetM) > 1000.f ||
				std::fabs(fOutYawOffsetDegrees) > 360.f)
			{
				return false;
			}
			strOutAnchorKind = Read_String(*pAnchor, "kind");
			if ("BOSS_CURRENT" != strOutAnchorKind &&
				"STAGE_ORIGIN" != strOutAnchorKind)
			{
				return false;
			}
			bOutHasAnchor = true;
		}

		const DATA_JSON_VALUE* pActivation = Stage.Find("hitActivation");
		if (nullptr == pActivation)
			return true;
		if (!Has_ExactProperties(*pActivation,
				{ "kind", "startMs", "lifetimeMs", "perTargetPolicy" }) ||
			"ACTIVE_WINDOW" != Read_String(*pActivation, "kind") ||
			"ONCE" != Read_String(*pActivation, "perTargetPolicy") ||
			!Read_RequiredUInt32(*pActivation, "startMs", iOutStartMs) ||
			!Read_RequiredUInt32(*pActivation, "lifetimeMs", iOutLifetimeMs) ||
			0u == iOutLifetimeMs ||
			static_cast<uint64_t>(iOutStartMs) + iOutLifetimeMs >
				iStageDurationMs)
		{
			return false;
		}
		bOutHasActivation = true;
		return true;
	}

	bool_t Validate_SplitGameplayStageExtensions(
		const DATA_JSON_VALUE& Stage,
		const std::vector<Client::VALTAN_STAGE_BRANCH_VIEW>& Branches,
		const std::string_view strPatternId,
		const std::string_view strStageId,
		std::string& strOutError)
	{
		const auto HasBranch = [&Branches](const std::string_view strOutcome)
		{
			return Branches.end() != std::find_if(
				Branches.begin(), Branches.end(),
				[strOutcome](const Client::VALTAN_STAGE_BRANCH_VIEW& Branch)
				{ return Branch.strOutcome == strOutcome; });
		};

		std::string strPartDamagePolicy;
		std::optional<Client::VALTAN_COUNTER_PROXY_VIEW> CounterProxy;
		if (!Read_StageGameplayExtensions(
				Stage, strPartDamagePolicy, CounterProxy))
		{
			strOutError = "split gameplay stage extension values are invalid: " +
				std::string(strPatternId) + "/" + std::string(strStageId);
			return false;
		}
		const DATA_JSON_VALUE* pHit = Stage.Find("hit");
		if (!Has_ValidNavigationBlockedCapture(Branches,
				nullptr == pHit ? std::string{} : Read_String(*pHit, "playerResponse")))
		{
			strOutError = "split navigation-blocked branch requires a capture rush: " +
				std::string(strPatternId) + "/" + std::string(strStageId);
			return false;
		}
		if ("DESTROY_FIRST_ELIGIBLE" == strPartDamagePolicy &&
			(!HasBranch("PART_DESTROYED") ||
			 !Has_ClosedSplitStageFlag(Stage, "boss.flag.groggy")))
		{
			strOutError =
				"split gameplay instant part destruction contract is invalid: " +
				std::string(strPatternId) + "/" + std::string(strStageId);
			return false;
		}
		if (CounterProxy.has_value() &&
			"WINDUP" != Read_String(Stage, "stageKind"))
		{
			strOutError = "split gameplay counterProxy preset requires WINDUP: " +
				std::string(strPatternId) + "/" + std::string(strStageId);
			return false;
		}
		return true;
	}

	bool_t Validate_SplitCounterBranchContract(
		const DATA_JSON_VALUE& GameplayPattern,
		const std::string_view strPatternId,
		std::string& strOutError)
	{
		const DATA_JSON_VALUE* pStages = Required(
			GameplayPattern, "stages", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pStages)
			return false;

		const auto FlagState = [](const DATA_JSON_VALUE& Stage,
			const std::string_view strFlagId)
		{
			const DATA_JSON_VALUE* pEvents = Required(
				Stage, "events", DATA_JSON_TYPE::ARRAY);
			if (nullptr == pEvents)
				return -1;
			uint32_t iTotal = 0u;
			uint32_t iEnter = 0u;
			uint32_t iExit = 0u;
			for (const DATA_JSON_VALUE& Event : pEvents->Get_Array())
			{
				if ("SET_BOSS_FLAG" != Read_String(Event, "kind") ||
					strFlagId != Read_String(Event, "flagId"))
				{
					continue;
				}
				++iTotal;
				const DATA_JSON_VALUE* pEnabled = Required(
					Event, "enabled", DATA_JSON_TYPE::BOOLEAN);
				if (nullptr == pEnabled)
					continue;
				const std::string strTrigger = Read_String(Event, "trigger");
				if ("ENTER" == strTrigger && pEnabled->Get_Boolean())
					++iEnter;
				if ("EXIT" == strTrigger && !pEnabled->Get_Boolean())
					++iExit;
			}
			if (0u == iTotal)
				return 0;
			return 2u == iTotal && 1u == iEnter && 1u == iExit ? 1 : -1;
		};

		const auto& Stages = pStages->Get_Array();
		for (std::size_t iStageIndex = 0u;
			iStageIndex < Stages.size(); ++iStageIndex)
		{
			const DATA_JSON_VALUE& Stage = Stages[iStageIndex];
			const std::string strStageId = Read_String(Stage, "stageId");
			std::vector<Client::VALTAN_STAGE_BRANCH_VIEW> Branches;
			if (!Read_StageBranches(Stage.Find("branches"), Branches))
				return false;
			const std::size_t iCounterCount = static_cast<std::size_t>(
				std::count_if(
					Branches.begin(), Branches.end(),
					[](const Client::VALTAN_STAGE_BRANCH_VIEW& Branch)
					{ return "COUNTER_HIT" == Branch.strOutcome; }));
			const auto Counter = std::find_if(
				Branches.begin(), Branches.end(),
				[](const Client::VALTAN_STAGE_BRANCH_VIEW& Branch)
				{ return "COUNTER_HIT" == Branch.strOutcome; });
			const std::size_t iTimeoutCount = static_cast<std::size_t>(
				std::count_if(
					Branches.begin(), Branches.end(),
					[](const Client::VALTAN_STAGE_BRANCH_VIEW& Branch)
					{ return "TIMEOUT" == Branch.strOutcome; }));
			const auto Timeout = std::find_if(
				Branches.begin(), Branches.end(),
				[](const Client::VALTAN_STAGE_BRANCH_VIEW& Branch)
				{ return "TIMEOUT" == Branch.strOutcome; });
			const int iCounterState = FlagState(Stage, "boss.flag.counterable");
			if (0u == iCounterCount)
			{
				if (0 != iCounterState)
				{
					strOutError = "split counterable flag has no COUNTER_HIT owner: " +
						std::string(strPatternId) + "/" + strStageId;
					return false;
				}
			}
			else
			{
				if (1u != iCounterCount || 1u != iTimeoutCount ||
					Branches.end() == Counter || Branches.end() == Timeout ||
					!Counter->strNextActionId.has_value() ||
					!Timeout->strNextActionId.has_value())
				{
					strOutError =
						"split Counter source requires exactly one COUNTER_HIT and TIMEOUT edge: " +
						std::string(strPatternId) + "/" + strStageId;
					return false;
				}
				const DATA_JSON_VALUE* pTarget = nullptr;
				const DATA_JSON_VALUE* pTimeoutTarget = nullptr;
				std::size_t iTargetIndex = Stages.size();
				std::size_t iTimeoutTargetIndex = Stages.size();
				for (std::size_t iCandidate = 0u;
					iCandidate < Stages.size(); ++iCandidate)
				{
					const DATA_JSON_VALUE& Candidate = Stages[iCandidate];
					const std::string strActionId =
						Read_String(Candidate, "actionId");
					if (*Counter->strNextActionId == strActionId)
					{
						pTarget = &Candidate;
						iTargetIndex = iCandidate;
					}
					if (*Timeout->strNextActionId == strActionId)
					{
						pTimeoutTarget = &Candidate;
						iTimeoutTargetIndex = iCandidate;
					}
				}
				const std::string strTargetKind = nullptr == pTarget ?
					std::string{} : Read_String(*pTarget, "stageKind");
				const bool bTypedSuccessTarget = "WINDUP" == strTargetKind ||
					"GROGGY" == strTargetKind || "RECOVERY" == strTargetKind;
				const int iTargetGroggyState = nullptr == pTarget ? -1 :
					FlagState(*pTarget, "boss.flag.groggy");
				if ("WINDUP" != Read_String(Stage, "stageKind") ||
					1 != iCounterState || nullptr == pTarget ||
					nullptr == pTimeoutTarget || !bTypedSuccessTarget ||
					iTargetIndex <= iStageIndex ||
					iTimeoutTargetIndex <= iStageIndex ||
					("GROGGY" == strTargetKind && 1 != iTargetGroggyState) ||
					("GROGGY" != strTargetKind && 0 != iTargetGroggyState))
				{
					strOutError =
						"split Counter branch requires a closed WINDUP window plus forward typed success/TIMEOUT targets: " +
						std::string(strPatternId) + "/" + strStageId;
					return false;
				}
			}

			const int iGroggyState = FlagState(Stage, "boss.flag.groggy");
			if (("GROGGY" == Read_String(Stage, "stageKind") &&
				 1 != iGroggyState) ||
				("GROGGY" != Read_String(Stage, "stageKind") &&
				 0 != iGroggyState))
			{
				strOutError = "split groggy stage flag transition is invalid: " +
					std::string(strPatternId) + "/" + strStageId;
				return false;
			}
		}
		return true;
	}

	bool_t Read_RequiredHitOffsets(
		const DATA_JSON_VALUE& Object,
		std::vector<uint32_t>& Out)
	{
		Out.clear();
		const DATA_JSON_VALUE* pOffsets = Required(
			Object, "hitOffsetsMs", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pOffsets || pOffsets->Get_Array().size() > 1000u)
			return false;
		uint32_t iPrevious = 0u;
		for (size_t i = 0u; i < pOffsets->Get_Array().size(); ++i)
		{
			const DATA_JSON_VALUE& Value = pOffsets->Get_Array()[i];
			if (!Is_NonNegativeInteger(&Value))
				return false;
			const uint32_t iCurrent = static_cast<uint32_t>(Value.Get_Number());
			if (0u != i && iCurrent <= iPrevious)
				return false;
			Out.push_back(iCurrent);
			iPrevious = iCurrent;
		}
		return true;
	}

	bool_t Read_PatternServerMotion(
		const DATA_JSON_VALUE* pValue,
		std::optional<Client::VALTAN_PATTERN_SERVER_MOTION_VIEW>& Out)
	{
		Out.reset();
		if (nullptr == pValue || pValue->Is_Null())
			return true;
		if (!Has_ExactPropertiesWithOptional(*pValue,
				{ "kind", "anchorId", "landingPosition", "apexHeight",
				  "travelStageId", "takeoffStartMs", "takeoffEndMs",
				  "travelStartMs", "travelEndMs" },
				{ "moveToAnchorBeforeTakeoff" }))
		{
			return false;
		}
		const DATA_JSON_VALUE* pKind = Required(
			*pValue, "kind", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pAnchor = Required(
			*pValue, "anchorId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pTravel = Required(
			*pValue, "travelStageId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pLanding = Required(
			*pValue, "landingPosition", DATA_JSON_TYPE::ARRAY);
		Client::VALTAN_PATTERN_SERVER_MOTION_VIEW Motion;
		if (nullptr == pKind || nullptr == pAnchor || nullptr == pTravel ||
			("LEAP_TO_ANCHOR" != pKind->Get_String() &&
			 "LEAP_TO_TARGET" != pKind->Get_String()) ||
			!Is_StableToken(pAnchor->Get_String()) ||
			!Is_StableToken(pTravel->Get_String()) || nullptr == pLanding ||
			3u != pLanding->Get_Array().size() ||
			!Is_NonNegativeInteger(pValue->Find("takeoffStartMs")) ||
			!Is_NonNegativeInteger(pValue->Find("takeoffEndMs")) ||
			!Is_NonNegativeInteger(pValue->Find("travelStartMs")) ||
			!Is_NonNegativeInteger(pValue->Find("travelEndMs")) ||
			!Read_RequiredFiniteFloat(*pValue, "apexHeight", Motion.fApexHeight) ||
			Motion.fApexHeight <= 0.f || Motion.fApexHeight > 200.f)
		{
			return false;
		}
		for (size_t i = 0u; i < Motion.LandingPosition.size(); ++i)
		{
			const DATA_JSON_VALUE& Coordinate = pLanding->Get_Array()[i];
			if (!Is_FiniteNumber(&Coordinate) ||
				std::abs(Coordinate.Get_Number()) > static_cast<double>(
					(std::numeric_limits<f32_t>::max)()))
			{
				return false;
			}
			Motion.LandingPosition[i] = static_cast<f32_t>(Coordinate.Get_Number());
		}
		Motion.strKind = pKind->Get_String();
		Motion.strAnchorId = pAnchor->Get_String();
		Motion.strTravelStageId = pTravel->Get_String();
		Motion.iTakeoffStartMs = static_cast<uint32_t>(
			pValue->Find("takeoffStartMs")->Get_Number());
		Motion.iTakeoffEndMs = static_cast<uint32_t>(
			pValue->Find("takeoffEndMs")->Get_Number());
		Motion.iTravelStartMs = static_cast<uint32_t>(
			pValue->Find("travelStartMs")->Get_Number());
		Motion.iTravelEndMs = static_cast<uint32_t>(
			pValue->Find("travelEndMs")->Get_Number());
		if (const DATA_JSON_VALUE* pApproach = pValue->Find("moveToAnchorBeforeTakeoff"))
		{
			if (!pApproach->Is_Boolean())
				return false;
			Motion.bMoveToAnchorBeforeTakeoff = pApproach->Get_Boolean();
		}
		if ((Motion.bMoveToAnchorBeforeTakeoff && 0u == Motion.iTakeoffStartMs) ||
			Motion.iTakeoffStartMs >= Motion.iTakeoffEndMs ||
			Motion.iTravelStartMs >= Motion.iTravelEndMs)
		{
			return false;
		}
		Out = std::move(Motion);
		return true;
	}

	bool_t Read_PatternFinale(
		const DATA_JSON_VALUE* pValue,
		std::optional<Client::VALTAN_PATTERN_FINALE_VIEW>& Out)
	{
		Out.reset();
		if (nullptr == pValue)
			return true;
		if (!Has_ExactProperties(*pValue,
				{ "kind", "ghostArchetypeId", "ghostPatternIds",
				  "spawnHalfExtentsM", "maximumActiveGhosts" }))
			return false;
		Client::VALTAN_PATTERN_FINALE_VIEW Finale;
		Finale.strKind = Read_String(*pValue, "kind");
		Finale.strGhostArchetypeId = Read_String(*pValue, "ghostArchetypeId");
		const DATA_JSON_VALUE* pPatterns = Required(
			*pValue, "ghostPatternIds", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pExtents = Required(
			*pValue, "spawnHalfExtentsM", DATA_JSON_TYPE::ARRAY);
		if ("GHOST_PORTAL_LOOP" != Finale.strKind ||
			!Is_StableToken(Finale.strGhostArchetypeId) ||
			!Read_RequiredUInt32(*pValue, "maximumActiveGhosts", Finale.iMaximumActiveGhosts) ||
			Finale.iMaximumActiveGhosts < 1u || Finale.iMaximumActiveGhosts > 64u ||
			nullptr == pPatterns || pPatterns->Get_Array().empty() ||
			pPatterns->Get_Array().size() > 64u ||
			nullptr == pExtents || 2u != pExtents->Get_Array().size())
			return false;
		std::set<std::string, std::less<>> PatternIds;
		Finale.GhostPatternIds.reserve(pPatterns->Get_Array().size());
		for (const DATA_JSON_VALUE& Pattern : pPatterns->Get_Array())
		{
			if (!Pattern.Is_String() || !Is_StableToken(Pattern.Get_String()) ||
				!PatternIds.insert(Pattern.Get_String()).second)
				return false;
			Finale.GhostPatternIds.push_back(Pattern.Get_String());
		}
		for (size_t iAxis = 0u; iAxis < 2u; ++iAxis)
		{
			const DATA_JSON_VALUE& Extent = pExtents->Get_Array()[iAxis];
			if (!Is_FiniteNumber(&Extent) || Extent.Get_Number() < 1.0 ||
				Extent.Get_Number() > 100.0)
				return false;
			Finale.SpawnHalfExtentsM[iAxis] = static_cast<f32_t>(Extent.Get_Number());
		}
		Out = std::move(Finale);
		return true;
	}

	struct MASTER_EFFECT_REFERENCE final
	{
		std::string strType;
		std::string strId;
		std::string strClipOccurrenceId;
		std::string strMappingBasis;
		uint32_t iSourceStartMs = 0u;
		uint32_t iSourceEndMs = 0u;
		bool_t bHasSourceEnd = false;
	};

	struct MASTER_STAGE final
	{
		std::string strStageId;
		std::string strSequenceRole;
		std::string strActionId;
		std::string strStageKind;
		uint32_t iDurationMs = 0u;
		uint32_t iRepeatCount = 0u;
		std::string strAnimationEndPolicy;
		bool_t bSuppressAnimation = false;
		std::string strHitShape;
		f32_t fHitOuterRadius = 0.f;
		f32_t fHitInnerRadius = 0.f;
		f32_t fHitAngleDegrees = 0.f;
		f32_t fHitLength = 0.f;
		f32_t fHitHalfWidth = 0.f;
		uint32_t iHitCount = 0u;
		uint32_t iHitIntervalMs = 0u;
		uint32_t iHitDelayMs = 0u;
		std::vector<uint32_t> HitOffsetsMs;
		bool_t bHasHitAnchor = false;
		std::string strHitAnchorKind = "BOSS_CURRENT";
		f32_t fHitAnchorForwardOffsetM = 0.f;
		f32_t fHitAnchorRightOffsetM = 0.f;
		f32_t fHitAnchorYawOffsetDegrees = 0.f;
		bool_t bHasHitActivation = false;
		uint32_t iHitActivationStartMs = 0u;
		uint32_t iHitActivationLifetimeMs = 0u;
		std::string strServerDamageProfileId;
		std::string strPlayerResponse = "DAMAGE";
		std::string strAttachmentSlot = "NONE";
		std::string strPartDamagePolicy = "NORMAL";
		std::optional<Client::VALTAN_COUNTER_PROXY_VIEW> CounterProxy;
		f32_t fPushRangeM = 0.f;
		uint32_t iPushMs = 0u;
		bool_t bKnockdown = false;
		uint32_t iDownMs = 0u;
		std::optional<Client::VALTAN_STAGE_MOTION_VIEW> Motion;
		std::vector<Client::VALTAN_STAGE_ACTION_VIEW> Actions;
		std::vector<Client::VALTAN_STAGE_BRANCH_VIEW> Branches;
		std::vector<Client::VALTAN_CLIP_OCCURRENCE_VIEW> Occurrences;
		std::vector<Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW> AuthoredCues;
		std::vector<MASTER_EFFECT_REFERENCE> EffectReferences;
		std::vector<Client::VALTAN_CAMERA_INVOCATION_VIEW> CameraInvocations;
	};

	template <typename TStage>
	bool_t Validate_PatternServerMotionStageWindows(
		const std::optional<Client::VALTAN_PATTERN_SERVER_MOTION_VIEW>& Motion,
		const std::vector<TStage>& Stages)
	{
		if (!Motion.has_value())
			return true;
		if (Stages.empty() ||
			Motion->iTakeoffStartMs >= Motion->iTakeoffEndMs ||
			Motion->iTakeoffEndMs > Stages.front().iDurationMs ||
			Motion->iTravelStartMs >= Motion->iTravelEndMs)
		{
			return false;
		}
		const auto Travel = std::find_if(
			Stages.begin() + 1u, Stages.end(),
			[&Motion](const TStage& Stage)
			{ return Stage.strStageId == Motion->strTravelStageId; });
		return Stages.end() != Travel &&
			Motion->iTravelEndMs <= Travel->iDurationMs;
	}

	struct MASTER_PATTERN final
	{
		std::string strPatternId;
		std::string strCategory;
		uint32_t iMinimumPhase = 0u;
		uint32_t iMaximumPhase = 0u;
		std::string strTargetPolicy;
		std::string strAimPolicy;
		std::string strDisplayName;
		std::string strActionId;
		std::vector<uint32_t> SourceActionIds;
		std::string strSelectionMode;
		int32_t iMinimumHealthBar = 0;
		int32_t iMaximumHealthBar = 0;
		int32_t iTriggerHealthBar = 0;
		uint32_t iTriggerOrder = 0u;
		std::string strArmorRequirement;
		std::string strPhaseRequirement;
		bool_t bInvulnerableWhileRunning = false;
		uint32_t iSelectionWeight = 0u;
		uint32_t iMaximumConsecutiveUses = 0u;
		f32_t fMinimumRange = 0.f;
		f32_t fMaximumRange = 0.f;
		std::optional<Client::VALTAN_PATTERN_SERVER_MOTION_VIEW> ServerMotion;
		std::optional<Client::VALTAN_PATTERN_FINALE_VIEW> Finale;
		uint32_t iSourceSequenceIndex = 0u;
		std::vector<Client::VALTAN_PRESENTATION_SOURCE_VIEW> PresentationSources;
		std::vector<Client::VALTAN_PATTERN_REACTION_VIEW> Reactions;
		std::vector<std::string> CameraCueIds;
		std::vector<Client::VALTAN_WORLD_EVENT_TRIGGER_REF_VIEW>
			WorldEventTriggerRefs;
		bool_t bManualServerAudition = false;
		std::string strSourceAnimationChainId;
		uint32_t iAuthoringPhase = 0u;
		std::string strAdmissionState;
		std::vector<MASTER_STAGE> Stages;
	};

	bool_t Validate_FinitePatternStageGraph(
		const MASTER_PATTERN& Pattern, std::string& strOutError)
	{
		std::map<std::string, size_t, std::less<>> StageByAction;
		for (size_t iStage = 0u; iStage < Pattern.Stages.size(); ++iStage)
		{
			if (!StageByAction.emplace(Pattern.Stages[iStage].strActionId, iStage).second)
			{
				strOutError = "finite stage graph has a duplicated action: " + Pattern.strPatternId;
				return false;
			}
		}
		std::vector<std::vector<size_t>> Successors(Pattern.Stages.size());
		std::vector<size_t> Incoming(Pattern.Stages.size(), 0u);
		for (size_t iStage = 0u; iStage < Pattern.Stages.size(); ++iStage)
		{
			const MASTER_STAGE& Stage = Pattern.Stages[iStage];
			const auto AddSuccessor = [&](const std::string& ActionId)
			{
				const auto Target = StageByAction.find(ActionId);
				if (StageByAction.end() == Target)
				{
					strOutError = "finite stage graph has a dangling action: " +
						Pattern.strPatternId + "/" + ActionId;
					return false;
				}
				Successors[iStage].push_back(Target->second);
				++Incoming[Target->second];
				return true;
			};
			const auto Timeout = std::find_if(Stage.Branches.begin(), Stage.Branches.end(),
				[](const Client::VALTAN_STAGE_BRANCH_VIEW& Branch)
				{ return "TIMEOUT" == Branch.strOutcome; });
			// Split validation already requires the explicit default edge to
			// match TIMEOUT, or the next sequential stage when TIMEOUT is absent.
			if (Stage.Branches.end() == Timeout && iStage + 1u < Pattern.Stages.size() &&
				!AddSuccessor(Pattern.Stages[iStage + 1u].strActionId))
				return false;
			for (const Client::VALTAN_STAGE_BRANCH_VIEW& Branch : Stage.Branches)
				if (Branch.strNextActionId.has_value() && !AddSuccessor(*Branch.strNextActionId))
					return false;
		}
		// Check every authored stage, including unreachable alternatives, without
		// recursive traversal or a duration cap that could hide a graph loop.
		std::vector<size_t> Ready;
		Ready.reserve(Pattern.Stages.size());
		for (size_t iStage = 0u; iStage < Incoming.size(); ++iStage)
			if (0u == Incoming[iStage])
				Ready.push_back(iStage);
		for (size_t iReady = 0u; iReady < Ready.size(); ++iReady)
			for (const size_t iSuccessor : Successors[Ready[iReady]])
				if (0u == --Incoming[iSuccessor])
					Ready.push_back(iSuccessor);
		if (Ready.size() != Pattern.Stages.size())
		{
			strOutError = "finite stage graph contains a cycle: " + Pattern.strPatternId;
			return false;
		}
		return true;
	}

	struct MASTER_SCRIPTED_SEQUENCE_VIEW final
	{
		std::string strSequenceId;
		std::string strMode;
		uint32_t iInterStepPursuitMs = 0u;
		std::vector<std::string> PatternIds;
	};

	struct MASTER_DOCUMENT final
	{
		std::vector<std::string> RetiredPatternIds;
		MASTER_SCRIPTED_SEQUENCE_VIEW ScriptedSequence;
		Client::VALTAN_NORMAL_SELECTION_VIEW NormalSelection;
		std::vector<Client::VALTAN_SELECTION_SET_VIEW> SelectionSets;
		std::vector<Client::VALTAN_SELECTION_WINDOW_VIEW> SelectionWindows;
		std::vector<Client::VALTAN_MECHANIC_VIEW> Mechanics;
		std::vector<Client::VALTAN_MANUAL_AUDITION_VIEW> ManualAuditions;
		std::vector<Client::VALTAN_COUNTER_REACTION_LAYER_VIEW>
			CounterReactionLayers;
		std::vector<Client::VALTAN_INDEPENDENT_EFFECT_VIEW> IndependentEffects;
		std::vector<MASTER_PATTERN> Patterns;
	};

	bool_t Parse_MasterOccurrence(
		const DATA_JSON_VALUE& Value,
		Client::VALTAN_CLIP_OCCURRENCE_VIEW& Out,
		std::string& strOutError)
	{
		if (!Has_ExactProperties(Value,
				{ "clipOccurrenceId", "clip", "mappingBasis",
				  "sourceStartMs", "playMs", "playRate",
				  "repeatUntilStageEnd" }))
		{
			strOutError = "master animation occurrence has unexpected properties";
			return false;
		}
		const DATA_JSON_VALUE* pOccurrenceId = Required(
			Value, "clipOccurrenceId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pClip = Required(
			Value, "clip", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pBasis = Required(
			Value, "mappingBasis", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pSourceStart = Value.Find("sourceStartMs");
		const DATA_JSON_VALUE* pPlay = Value.Find("playMs");
		const DATA_JSON_VALUE* pRate = Value.Find("playRate");
		const DATA_JSON_VALUE* pRepeat = Required(
			Value, "repeatUntilStageEnd", DATA_JSON_TYPE::BOOLEAN);
		if (nullptr == pOccurrenceId || nullptr == pClip || nullptr == pBasis ||
			!Is_StableToken(pOccurrenceId->Get_String()) ||
			!Is_StableToken(pClip->Get_String()) || pBasis->Get_String().empty() ||
			!Is_NonNegativeInteger(pSourceStart) ||
			!Is_NonNegativeInteger(pPlay) || !Is_FiniteNumber(pRate) ||
			pRate->Get_Number() <= 0.05 || pRate->Get_Number() > 16.0 ||
			nullptr == pRepeat)
		{
			strOutError = "master animation occurrence is invalid";
			return false;
		}
		Out.strClipOccurrenceId = pOccurrenceId->Get_String();
		Out.strClipName = pClip->Get_String();
		Out.strMappingBasis = pBasis->Get_String();
		Out.iSourceStartMs = static_cast<uint32_t>(pSourceStart->Get_Number());
		Out.iPlayMs = static_cast<uint32_t>(pPlay->Get_Number());
		Out.fPlayRate = static_cast<f32_t>(pRate->Get_Number());
		Out.bLoop = pRepeat->Get_Boolean();
		return true;
	}

	bool_t Validate_MasterStageGameplayShape(
		const DATA_JSON_VALUE& Value,
		std::string& strOutError)
	{
		const std::initializer_list<std::string_view> NumericFields = {
			"hitOuterRadius", "hitInnerRadius", "hitAngleDegrees", "hitLength",
			"hitHalfWidth", "hitCount", "hitIntervalMs", "hitDelayMs",
			"pushRangeM", "pushMs", "downMs" };
		for (const std::string_view Field : NumericFields)
		{
			if (!Is_FiniteNumber(Value.Find(Field)))
			{
				strOutError = "master stage numeric field is invalid: " +
					std::string(Field);
				return false;
			}
		}
		const DATA_JSON_VALUE* pOffsets = Required(
			Value, "hitOffsetsMs", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pKnockdown = Required(
			Value, "knockdown", DATA_JSON_TYPE::BOOLEAN);
		const DATA_JSON_VALUE* pMotion = Value.Find("motion");
		const DATA_JSON_VALUE* pActions = Required(
			Value, "actions", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pPlayerResponse = Value.Find("playerResponse");
		const DATA_JSON_VALUE* pAttachmentSlot = Value.Find("attachmentSlot");
		if (nullptr == pOffsets || nullptr == pKnockdown || nullptr == pMotion ||
			nullptr == pActions ||
			(nullptr == pPlayerResponse) != (nullptr == pAttachmentSlot))
		{
			strOutError = "master stage gameplay fields are invalid";
			return false;
		}
		if (nullptr != pPlayerResponse &&
			(!pPlayerResponse->Is_String() || !pAttachmentSlot->Is_String() ||
			 "CAPTURE" != pPlayerResponse->Get_String() ||
			 "BOSS_LEFT_HAND" != pAttachmentSlot->Get_String() ||
			 "NONE" == Read_String(Value, "hitShape") ||
			 0.0 != Read_Number(Value, "pushRangeM") ||
			 0.0 != Read_Number(Value, "pushMs") ||
			 pKnockdown->Get_Boolean() ||
			 0.0 != Read_Number(Value, "downMs")))
		{
			strOutError = "master stage capture hit contract is invalid";
			return false;
		}
		for (const DATA_JSON_VALUE& Offset : pOffsets->Get_Array())
		{
			if (!Is_NonNegativeInteger(&Offset))
			{
				strOutError = "master stage hitOffsetsMs is invalid";
				return false;
			}
		}
		if (!pMotion->Is_Null())
		{
			std::optional<Client::VALTAN_STAGE_MOTION_VIEW> Motion;
			if (!Read_StageMotion(pMotion,
					static_cast<uint32_t>(Read_Number(Value, "durationMs")), Motion))
			{
				strOutError = "master stage motion is invalid";
				return false;
			}
		}
		for (const DATA_JSON_VALUE& Action : pActions->Get_Array())
		{
			if ("RELEASE_GRABBED_PLAYERS" == Read_String(Action, "kind"))
			{
				if (!Has_ExactProperties(Action,
						{ "trigger", "kind", "targetId", "releaseMode",
						  "speedMps", "durationMs", "yawOffsetDegrees" }) ||
					nullptr == Required(
						Action, "trigger", DATA_JSON_TYPE::STRING) ||
					nullptr == Required(
						Action, "targetId", DATA_JSON_TYPE::STRING) ||
					nullptr == Required(
						Action, "releaseMode", DATA_JSON_TYPE::STRING) ||
					!Is_FiniteNumber(Action.Find("speedMps")) ||
					!Is_NonNegativeInteger(Action.Find("durationMs")) ||
					!Is_FiniteNumber(Action.Find("yawOffsetDegrees")))
				{
					strOutError = "master grabbed-player release action is invalid";
					return false;
				}
				const std::string strTrigger = Read_String(Action, "trigger");
				const std::string strTargetId = Read_String(Action, "targetId");
				const std::string strReleaseMode =
					Read_String(Action, "releaseMode");
				const double fSpeedMps = Read_Number(Action, "speedMps");
				const double fDurationMs = Read_Number(Action, "durationMs");
				const double fYawOffsetDegrees =
					Read_Number(Action, "yawOffsetDegrees");
				const bool_t bHold = "HOLD" == strReleaseMode &&
					0.0 == fSpeedMps && 0.0 == fDurationMs &&
					0.0 == fYawOffsetDegrees;
				const bool_t bLaunch =
					("OPPOSITE_KNOCKBACK" == strReleaseMode ||
					 "ARENA_EJECTION" == strReleaseMode) &&
					fSpeedMps > 0.0 && fSpeedMps <= 50.0 &&
					fDurationMs > 0.0 && fDurationMs <= 5000.0 &&
					("ARENA_EJECTION" == strReleaseMode ||
					 0.0 == fYawOffsetDegrees);
				if (("ENTER" != strTrigger && "EXIT" != strTrigger) ||
					"boss.attachment.left-hand" != strTargetId ||
					std::abs(fYawOffsetDegrees) > 180.0 ||
					(!bHold && !bLaunch))
				{
					strOutError =
						"master grabbed-player release action is incoherent";
					return false;
				}
				continue;
			}
			if (!Has_ExactProperties(Action,
					{ "trigger", "kind", "targetId", "value", "durationMs" }) ||
				nullptr == Required(Action, "trigger", DATA_JSON_TYPE::STRING) ||
				nullptr == Required(Action, "kind", DATA_JSON_TYPE::STRING) ||
				nullptr == Required(Action, "targetId", DATA_JSON_TYPE::STRING) ||
				!Is_FiniteNumber(Action.Find("value")) ||
				!Is_NonNegativeInteger(Action.Find("durationMs")))
			{
				strOutError = "master stage action is invalid";
				return false;
			}
		}
		return true;
	}

	bool_t Read_MasterCameraInvocations(
		const DATA_JSON_VALUE* pValue,
		const uint32_t iStageDurationMs,
		std::vector<Client::VALTAN_CAMERA_INVOCATION_VIEW>& Out)
	{
		if (nullptr == pValue || !pValue->Is_Array())
			return false;
		std::set<std::string, std::less<>> InvocationIds;
		for (const DATA_JSON_VALUE& Value : pValue->Get_Array())
		{
			if (!Has_ExactProperties(Value,
					{ "cameraInvocationId", "cameraCueId", "trigger",
					  "startOffsetMs", "durationPolicy", "durationMs" }))
			{
				return false;
			}
			const DATA_JSON_VALUE* pInvocationId = Required(
				Value, "cameraInvocationId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pCueId = Required(
				Value, "cameraCueId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pTrigger = Required(
				Value, "trigger", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pDurationPolicy = Required(
				Value, "durationPolicy", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pStartOffset = Value.Find("startOffsetMs");
			const DATA_JSON_VALUE* pDuration = Value.Find("durationMs");
			if (nullptr == pInvocationId || nullptr == pCueId ||
				nullptr == pTrigger || nullptr == pDurationPolicy ||
				!Is_StableToken(pInvocationId->Get_String()) ||
				!Is_StableToken(pCueId->Get_String()) ||
				"ENTER" != pTrigger->Get_String() ||
				"EXPLICIT" != pDurationPolicy->Get_String() ||
				!Is_NonNegativeInteger(pStartOffset) ||
				!Is_NonNegativeInteger(pDuration) ||
				!InvocationIds.insert(pInvocationId->Get_String()).second)
			{
				return false;
			}
			const uint64_t iEnd = static_cast<uint64_t>(
				pStartOffset->Get_Number()) + static_cast<uint64_t>(
				pDuration->Get_Number());
			if (iEnd > iStageDurationMs)
				return false;
			Client::VALTAN_CAMERA_INVOCATION_VIEW Invocation;
			Invocation.strCameraInvocationId = pInvocationId->Get_String();
			Invocation.strCameraCueId = pCueId->Get_String();
			Invocation.strTrigger = pTrigger->Get_String();
			Invocation.iStartOffsetMs = static_cast<uint32_t>(
				pStartOffset->Get_Number());
			Invocation.strDurationPolicy = pDurationPolicy->Get_String();
			Invocation.iDurationMs = static_cast<uint32_t>(
				pDuration->Get_Number());
			Out.push_back(std::move(Invocation));
		}
		return true;
	}

	bool_t Parse_MasterStage(
		const DATA_JSON_VALUE& Value,
		MASTER_STAGE& Out,
		std::string& strOutError)
	{
		const bool_t bBaseShape = Has_ExactPropertiesWithOptional(Value,
				{ "stageId", "sequenceRole", "actionId", "stageKind",
				  "durationMs", "hitShape", "hitOuterRadius", "hitInnerRadius",
				  "hitAngleDegrees", "hitLength", "hitHalfWidth", "hitCount",
				  "hitIntervalMs", "hitDelayMs", "hitOffsetsMs",
				  "serverDamageProfileId", "pushRangeM", "pushMs", "knockdown",
				  "downMs", "motion", "actions", "branches", "animation",
				  "effectRefs", "cameraInvocations" },
				{ "partDamagePolicy", "counterProxy", "hitAnchor",
				  "hitActivation" });
		const bool_t bCaptureShape = Has_ExactPropertiesWithOptional(Value,
				{ "stageId", "sequenceRole", "actionId", "stageKind",
				  "durationMs", "hitShape", "hitOuterRadius", "hitInnerRadius",
				  "hitAngleDegrees", "hitLength", "hitHalfWidth", "hitCount",
				  "hitIntervalMs", "hitDelayMs", "hitOffsetsMs",
				  "serverDamageProfileId", "playerResponse", "attachmentSlot",
				  "pushRangeM", "pushMs", "knockdown", "downMs", "motion",
				  "actions", "branches", "animation", "effectRefs",
				  "cameraInvocations" },
				{ "partDamagePolicy", "counterProxy", "hitAnchor",
				  "hitActivation" });
		if (!bBaseShape && !bCaptureShape)
		{
			strOutError = "master stage has unexpected properties";
			return false;
		}
		const DATA_JSON_VALUE* pStageId = Required(
			Value, "stageId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pRole = Required(
			Value, "sequenceRole", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pAction = Required(
			Value, "actionId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pKind = Required(
			Value, "stageKind", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pDuration = Value.Find("durationMs");
		const DATA_JSON_VALUE* pShape = Required(
			Value, "hitShape", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pDamage = Required(
			Value, "serverDamageProfileId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pAnimation = Required(
			Value, "animation", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* pBranches = Required(
			Value, "branches", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pEffects = Required(
			Value, "effectRefs", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pStageId || nullptr == pRole || nullptr == pAction ||
			nullptr == pKind || !Is_NonNegativeInteger(pDuration) ||
			0.0 == pDuration->Get_Number() || nullptr == pShape ||
			nullptr == pDamage || nullptr == pAnimation || nullptr == pBranches ||
			nullptr == pEffects ||
			!Is_StableToken(pStageId->Get_String()) ||
			!Is_StableToken(pRole->Get_String()) ||
			!Is_StableToken(pAction->Get_String()))
		{
			strOutError = "master stage identity is invalid";
			return false;
		}
		if (!Validate_MasterStageGameplayShape(Value, strOutError))
			return false;

		const bool_t bSuppressAnimation = Has_ExactProperties(
			*pAnimation, { "mode" });
		const DATA_JSON_VALUE* pRepeatCount = nullptr;
		const DATA_JSON_VALUE* pEndPolicy = nullptr;
		const DATA_JSON_VALUE* pOccurrences = nullptr;
		if (bSuppressAnimation)
		{
			const DATA_JSON_VALUE* pMode = Required(
				*pAnimation, "mode", DATA_JSON_TYPE::STRING);
			if (nullptr == pMode || "NONE" != pMode->Get_String())
			{
				strOutError = "master stage animation mode is invalid";
				return false;
			}
		}
		else
		{
			if (!Has_ExactProperties(
					*pAnimation, { "repeatCount", "endPolicy", "occurrences" }))
			{
				strOutError = "master stage animation has unexpected properties";
				return false;
			}
			pRepeatCount = pAnimation->Find("repeatCount");
			pEndPolicy = Required(
				*pAnimation, "endPolicy", DATA_JSON_TYPE::STRING);
			pOccurrences = Required(
				*pAnimation, "occurrences", DATA_JSON_TYPE::ARRAY);
			if (!Is_NonNegativeInteger(pRepeatCount) ||
				pRepeatCount->Get_Number() < 1.0 ||
				pRepeatCount->Get_Number() > 64.0 || nullptr == pOccurrences ||
				pOccurrences->Get_Array().empty() || nullptr == pEndPolicy ||
				("EXACT" != pEndPolicy->Get_String() &&
				 "HOLD_LAST_POSE" != pEndPolicy->Get_String() &&
				 "LOOP_TO_STAGE_END" != pEndPolicy->Get_String()) ||
				pOccurrences->Get_Array().size() > 64u)
			{
				strOutError = "master stage repeatCount/occurrences is invalid";
				return false;
			}
		}

		Out.strStageId = pStageId->Get_String();
		Out.strSequenceRole = pRole->Get_String();
		Out.strActionId = pAction->Get_String();
		Out.strStageKind = pKind->Get_String();
		Out.iDurationMs = static_cast<uint32_t>(pDuration->Get_Number());
		Out.bSuppressAnimation = bSuppressAnimation;
		Out.iRepeatCount = bSuppressAnimation ? 0u :
			static_cast<uint32_t>(pRepeatCount->Get_Number());
		Out.strAnimationEndPolicy = bSuppressAnimation ? "NONE" :
			pEndPolicy->Get_String();
		Out.strHitShape = pShape->Get_String();
		Out.strServerDamageProfileId = pDamage->Get_String();
		Out.strPlayerResponse = bCaptureShape ?
			Read_String(Value, "playerResponse") : "DAMAGE";
		Out.strAttachmentSlot = bCaptureShape ?
			Read_String(Value, "attachmentSlot") : "NONE";
		if (!Read_RequiredFiniteFloat(
				Value, "hitOuterRadius", Out.fHitOuterRadius) ||
			!Read_RequiredFiniteFloat(
				Value, "hitInnerRadius", Out.fHitInnerRadius) ||
			!Read_RequiredFiniteFloat(
				Value, "hitAngleDegrees", Out.fHitAngleDegrees) ||
			!Read_RequiredFiniteFloat(Value, "hitLength", Out.fHitLength) ||
			!Read_RequiredFiniteFloat(
				Value, "hitHalfWidth", Out.fHitHalfWidth) ||
			!Read_RequiredUInt32(Value, "hitCount", Out.iHitCount) ||
			!Read_RequiredUInt32(Value, "hitIntervalMs", Out.iHitIntervalMs) ||
			!Read_RequiredUInt32(Value, "hitDelayMs", Out.iHitDelayMs) ||
			!Read_RequiredHitOffsets(Value, Out.HitOffsetsMs) ||
			!Read_RequiredFiniteFloat(Value, "pushRangeM", Out.fPushRangeM) ||
			!Read_RequiredUInt32(Value, "pushMs", Out.iPushMs) ||
			!Read_RequiredUInt32(Value, "downMs", Out.iDownMs) ||
			!Read_StageMotion(
				Value.Find("motion"), Out.iDurationMs, Out.Motion) ||
			!Read_StageActions(
				Value.Find("actions"), Out.iDurationMs, Out.Actions) ||
			!Read_StageBranches(pBranches, Out.Branches) ||
			!Has_ValidNavigationBlockedCapture(Out.Branches, Out.strPlayerResponse) ||
			!Read_StageGameplayExtensions(Value, Out.strPartDamagePolicy,
				Out.CounterProxy) ||
			!Read_StageHitAuthority(
				Value, Out.iDurationMs, Out.bHasHitAnchor,
				Out.strHitAnchorKind, Out.fHitAnchorForwardOffsetM,
				Out.fHitAnchorRightOffsetM, Out.fHitAnchorYawOffsetDegrees,
				Out.bHasHitActivation, Out.iHitActivationStartMs,
				Out.iHitActivationLifetimeMs) ||
			!Read_MasterCameraInvocations(
				Value.Find("cameraInvocations"), Out.iDurationMs,
				Out.CameraInvocations))
		{
			strOutError = "master stage gameplay values are invalid";
			return false;
		}
		const bool_t bHasExplicitOffsets = !Out.HitOffsetsMs.empty();
		const bool_t bValidPulse = !Out.bHasHitActivation &&
			((bHasExplicitOffsets && Out.HitOffsetsMs.size() == Out.iHitCount &&
			  0u == Out.iHitIntervalMs && 0u == Out.iHitDelayMs &&
			  Out.HitOffsetsMs.back() < Out.iDurationMs) ||
			 (!bHasExplicitOffsets && Out.iHitCount > 0u &&
			  (1u == Out.iHitCount ? 0u == Out.iHitIntervalMs :
				Out.iHitIntervalMs > 0u) &&
			  static_cast<uint64_t>(Out.iHitDelayMs) +
				static_cast<uint64_t>(Out.iHitCount - 1u) *
					Out.iHitIntervalMs < Out.iDurationMs));
		const bool_t bValidActive = Out.bHasHitActivation &&
			!bHasExplicitOffsets && 0u == Out.iHitCount &&
			0u == Out.iHitIntervalMs && 0u == Out.iHitDelayMs;
		const bool_t bEmptyHit = "NONE" == Out.strHitShape &&
			!Out.bHasHitAnchor && !Out.bHasHitActivation &&
			!bHasExplicitOffsets && 0u == Out.iHitCount &&
			0u == Out.iHitIntervalMs && 0u == Out.iHitDelayMs;
		if (!bEmptyHit && !bValidPulse && !bValidActive)
		{
			strOutError = "master stage hit authority/schedule is invalid";
			return false;
		}
		Out.bKnockdown = Required(
			Value, "knockdown", DATA_JSON_TYPE::BOOLEAN)->Get_Boolean();
		if (Out.Motion.has_value() &&
			"PORTAL_TARGET_RUSH" == Out.Motion->strKind)
		{
			const VALTAN_STAGE_MOTION_VIEW& Motion = *Out.Motion;
			const double fTravelEndMs =
				static_cast<double>(Motion.iRetargetDelayMs) +
				static_cast<double>(Motion.fDistance) /
				static_cast<double>(Motion.fSpeedMps) * 1000.0;
			uint32_t iExpectedOffsetMs = Motion.iRetargetDelayMs;
			if (Out.HitOffsetsMs.empty())
			{
				strOutError =
					"portal target-rush requires explicit swept-hit offsets";
				return false;
			}
			for (const uint32_t iOffsetMs : Out.HitOffsetsMs)
			{
				if (iOffsetMs != iExpectedOffsetMs ||
					static_cast<double>(iOffsetMs) >= fTravelEndMs)
				{
					strOutError =
						"portal target-rush hit offsets must cover only its 50 ms travel samples";
					return false;
				}
				iExpectedOffsetMs += 50u;
			}
			if (static_cast<double>(iExpectedOffsetMs) + 0.000001 <
				fTravelEndMs)
			{
				strOutError =
					"portal target-rush hit offsets do not cover its full travel";
				return false;
			}
		}
		std::set<std::string, std::less<>> OccurrenceIds;
		if (nullptr != pOccurrences)
		{
			for (const DATA_JSON_VALUE& OccurrenceValue :
				pOccurrences->Get_Array())
			{
				Client::VALTAN_CLIP_OCCURRENCE_VIEW Occurrence;
				if (!Parse_MasterOccurrence(
						OccurrenceValue, Occurrence, strOutError) ||
					!OccurrenceIds.insert(Occurrence.strClipOccurrenceId).second)
				{
					if (strOutError.empty())
						strOutError = "master stage duplicates a clip occurrence";
					return false;
				}
				Out.Occurrences.push_back(std::move(Occurrence));
			}
		}
		/* repeatCount is a semantic promise that one authored clip is repeated,
		   not merely a display label for an arbitrary occurrence list. Keeping
		   this check in the shared master admission makes both Tools fail closed
		   before they can build different timelines. */
		if (1u < Out.iRepeatCount)
		{
			if (Out.Occurrences.size() != Out.iRepeatCount)
			{
				strOutError =
					"master repeatCount must own exactly its explicit occurrences: " +
					Out.strActionId;
				return false;
			}
			const std::string& strRepeatedClip =
				Out.Occurrences.front().strClipName;
			if (std::any_of(Out.Occurrences.begin(), Out.Occurrences.end(),
				[&strRepeatedClip](
					const Client::VALTAN_CLIP_OCCURRENCE_VIEW& Occurrence)
				{
					return Occurrence.strClipName != strRepeatedClip;
				}))
			{
				strOutError =
					"master repeatCount occurrences must use one clip: " +
					Out.strActionId;
				return false;
			}
		}
		std::set<std::string, std::less<>> EffectReferenceIds;
		for (const DATA_JSON_VALUE& EffectValue : pEffects->Get_Array())
		{
			const DATA_JSON_VALUE* pType = Required(
				EffectValue, "refType", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pId = Required(
				EffectValue, "refId", DATA_JSON_TYPE::STRING);
			if (nullptr == pType || nullptr == pId ||
				("CUE_BINDING" != pType->Get_String() &&
				 "INDEPENDENT_EFFECT" != pType->Get_String()) ||
				!Is_StableToken(pId->Get_String()) ||
				!EffectReferenceIds.insert(
					pType->Get_String() + ":" + pId->Get_String()).second)
			{
				strOutError = "master effect reference is invalid or duplicated";
				return false;
			}
			MASTER_EFFECT_REFERENCE Reference;
			Reference.strType = pType->Get_String();
			Reference.strId = pId->Get_String();
			if ("INDEPENDENT_EFFECT" == Reference.strType)
			{
				if (!Has_ExactProperties(EffectValue, { "refType", "refId" }))
				{
					strOutError =
						"master independent Effect reference has unexpected properties";
					return false;
				}
			}
			else
			{
				if (!Has_ExactProperties(
						EffectValue, { "refType", "refId", "cueProjection" }))
				{
					strOutError =
						"master cue reference has unexpected properties";
					return false;
				}
				const DATA_JSON_VALUE* pProjection = Required(
					EffectValue, "cueProjection", DATA_JSON_TYPE::OBJECT);
				if (nullptr == pProjection || !Has_ExactProperties(*pProjection,
						{ "clipOccurrenceId", "sourceStartMs", "sourceEndMs",
						  "mappingBasis" }))
				{
					strOutError = "master cueProjection contract is invalid";
					return false;
				}
				const DATA_JSON_VALUE* pClipOccurrence = Required(
					*pProjection, "clipOccurrenceId", DATA_JSON_TYPE::STRING);
				const DATA_JSON_VALUE* pSourceStart =
					pProjection->Find("sourceStartMs");
				const DATA_JSON_VALUE* pSourceEnd =
					pProjection->Find("sourceEndMs");
				const DATA_JSON_VALUE* pMappingBasis = Required(
					*pProjection, "mappingBasis", DATA_JSON_TYPE::STRING);
				if (nullptr == pClipOccurrence ||
					!Is_StableToken(pClipOccurrence->Get_String()) ||
					!Is_NonNegativeInteger(pSourceStart) ||
					nullptr == pSourceEnd ||
					(!pSourceEnd->Is_Null() &&
					 !Is_NonNegativeInteger(pSourceEnd)) ||
					nullptr == pMappingBasis ||
					pMappingBasis->Get_String().empty())
				{
					strOutError = "master cueProjection values are invalid";
					return false;
				}
				Reference.strClipOccurrenceId = pClipOccurrence->Get_String();
				Reference.iSourceStartMs = static_cast<uint32_t>(
					pSourceStart->Get_Number());
				Reference.bHasSourceEnd = !pSourceEnd->Is_Null();
				Reference.iSourceEndMs = Reference.bHasSourceEnd ?
					static_cast<uint32_t>(pSourceEnd->Get_Number()) : 0u;
				Reference.strMappingBasis = pMappingBasis->Get_String();
			}
			Out.EffectReferences.push_back(std::move(Reference));
		}
		return true;
	}

	bool_t Validate_MasterServerMotion(
		const DATA_JSON_VALUE& Pattern,
		std::string& strOutError)
	{
		const DATA_JSON_VALUE* pMotion = Pattern.Find("serverMotion");
		std::optional<Client::VALTAN_PATTERN_SERVER_MOTION_VIEW> Motion;
		if (nullptr == pMotion || !Read_PatternServerMotion(pMotion, Motion))
		{
			strOutError = "master serverMotion is invalid";
			return false;
		}
		return true;
	}

	bool_t Parse_MasterPattern(
		const DATA_JSON_VALUE& Value,
		MASTER_PATTERN& Out,
		std::string& strOutError)
	{
		if (!Has_ExactPropertiesWithOptional(Value,
				{ "patternId", "category", "minimumPhase", "maximumPhase",
				  "targetPolicy", "aimPolicy", "displayName", "actionId",
				  "sourceActionIds", "sourceSequenceIndex", "presentationSources",
				  "selectionMode",
				  "minimumHealthBar", "maximumHealthBar", "triggerHealthBar",
				  "triggerOrder", "armorRequirement", "phaseRequirement",
				  "invulnerableWhileRunning", "selectionWeight",
				  "maximumConsecutiveUses", "minimumRange", "maximumRange",
				  "serverMotion", "reactions", "cameraCueIds",
				  "worldEventTriggerRefs", "stages" }, { "finale" }))
		{
			strOutError = "master pattern has unexpected properties";
			return false;
		}
		const DATA_JSON_VALUE* pPatternId = Required(
			Value, "patternId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pDisplay = Required(
			Value, "displayName", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pAction = Required(
			Value, "actionId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pSourceSequence = Value.Find("sourceSequenceIndex");
		const DATA_JSON_VALUE* pStages = Required(
			Value, "stages", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pPatternId || nullptr == pDisplay || nullptr == pAction ||
			!Is_StableToken(pPatternId->Get_String()) ||
			pDisplay->Get_String().empty() || !Is_StableToken(pAction->Get_String()) ||
			!Is_NonNegativeInteger(pSourceSequence) ||
			4096.0 < pSourceSequence->Get_Number() || nullptr == pStages ||
			pStages->Get_Array().empty())
		{
			strOutError = "master pattern identity is invalid";
			return false;
		}

		for (const std::string_view Field :
			{ "category", "targetPolicy", "aimPolicy", "selectionMode",
			  "armorRequirement", "phaseRequirement" })
		{
			const DATA_JSON_VALUE* pText = Required(
				Value, Field, DATA_JSON_TYPE::STRING);
			if (nullptr == pText || pText->Get_String().empty())
			{
				strOutError = "master pattern string field is invalid: " +
					std::string(Field);
				return false;
			}
		}
		for (const std::string_view Field :
			{ "minimumPhase", "maximumPhase", "minimumHealthBar",
			  "maximumHealthBar", "triggerHealthBar", "triggerOrder",
			  "selectionWeight", "maximumConsecutiveUses" })
		{
			if (!Is_NonNegativeInteger(Value.Find(Field)))
			{
				strOutError = "master pattern integer field is invalid: " +
					std::string(Field);
				return false;
			}
		}
		if (!Is_FiniteNumber(Value.Find("minimumRange")) ||
			!Is_FiniteNumber(Value.Find("maximumRange")) ||
			nullptr == Required(
				Value, "invulnerableWhileRunning", DATA_JSON_TYPE::BOOLEAN) ||
			!Validate_MasterServerMotion(Value, strOutError))
		{
			if (strOutError.empty())
				strOutError = "master pattern selection/motion fields are invalid";
			return false;
		}

		const DATA_JSON_VALUE* pSourceActions = Required(
			Value, "sourceActionIds", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pSourceActions || pSourceActions->Get_Array().empty() ||
			std::any_of(pSourceActions->Get_Array().begin(),
				pSourceActions->Get_Array().end(),
				[](const DATA_JSON_VALUE& SourceAction)
				{
					return !Is_NonNegativeInteger(&SourceAction) ||
						0.0 == SourceAction.Get_Number();
				}))
		{
			strOutError = "master sourceActionIds is invalid";
			return false;
		}
		const DATA_JSON_VALUE* pPresentationSources = Required(
			Value, "presentationSources", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pPresentationSources ||
			pPresentationSources->Get_Array().empty())
		{
			strOutError = "master presentationSources is invalid";
			return false;
		}
		std::set<std::string, std::less<>> PresentationSourceIdentities;
		for (const DATA_JSON_VALUE& Source :
			pPresentationSources->Get_Array())
		{
			if (!Has_ExactProperties(
					Source, { "sourceActionId", "sequenceIndex", "role" }) ||
				!Is_NonNegativeInteger(Source.Find("sourceActionId")) ||
				0.0 == Source.Find("sourceActionId")->Get_Number() ||
				!Is_NonNegativeInteger(Source.Find("sequenceIndex")) ||
				4096.0 < Source.Find("sequenceIndex")->Get_Number() ||
				nullptr == Required(Source, "role", DATA_JSON_TYPE::STRING))
			{
				strOutError = "master presentation source row is invalid";
				return false;
			}
			const std::string Identity = std::to_string(static_cast<uint32_t>(
				Source.Find("sourceActionId")->Get_Number())) + ":" +
				std::to_string(static_cast<uint32_t>(
					Source.Find("sequenceIndex")->Get_Number())) + ":" +
				Required(Source, "role", DATA_JSON_TYPE::STRING)->Get_String();
			if (!PresentationSourceIdentities.insert(Identity).second)
			{
				strOutError = "master presentation source row is duplicated";
				return false;
			}
		}
		const DATA_JSON_VALUE* pReactions = Required(
			Value, "reactions", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pCameraCues = Required(
			Value, "cameraCueIds", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pWorldRefs = Required(
			Value, "worldEventTriggerRefs", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pReactions || nullptr == pCameraCues ||
			nullptr == pWorldRefs)
		{
			strOutError = "master reactions/cue references are invalid";
			return false;
		}
		std::set<std::string, std::less<>> ReactionIdentities;
		for (const DATA_JSON_VALUE& Reaction : pReactions->Get_Array())
		{
			const DATA_JSON_VALUE* pTrigger = Required(
				Reaction, "triggerKind", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pStage = Required(
				Reaction, "stageId", DATA_JSON_TYPE::STRING);
			if (!Has_ExactProperties(Reaction, { "triggerKind", "stageId" }) ||
				nullptr == pTrigger || nullptr == pStage ||
				!Is_StableToken(pTrigger->Get_String()) ||
				!Is_StableToken(pStage->Get_String()) ||
				!ReactionIdentities.insert(
					pTrigger->Get_String() + ":" + pStage->Get_String()).second)
			{
				strOutError = "master reaction is invalid or duplicated";
				return false;
			}
			Client::VALTAN_PATTERN_REACTION_VIEW ReactionView;
			ReactionView.strTriggerKind = pTrigger->Get_String();
			ReactionView.strStageId = pStage->Get_String();
			Out.Reactions.push_back(std::move(ReactionView));
		}
		std::set<std::string, std::less<>> CameraCueIdentities;
		for (const DATA_JSON_VALUE& CameraCue : pCameraCues->Get_Array())
		{
			if (!CameraCue.Is_String() ||
				!Is_StableToken(CameraCue.Get_String()) ||
				!CameraCueIdentities.insert(CameraCue.Get_String()).second)
			{
				strOutError = "master cameraCueIds is invalid or duplicated";
				return false;
			}
			Out.CameraCueIds.push_back(CameraCue.Get_String());
		}
		std::set<std::string, std::less<>> WorldReferenceIdentities;
		for (const DATA_JSON_VALUE& WorldRef : pWorldRefs->Get_Array())
		{
			const DATA_JSON_VALUE* pWorldPattern = Required(
				WorldRef, "patternId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pWorldStage = Required(
				WorldRef, "stageId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pWorldTrigger = Required(
				WorldRef, "triggerKind", DATA_JSON_TYPE::STRING);
			if (!Has_ExactProperties(
					WorldRef, { "patternId", "stageId", "triggerKind" }) ||
				nullptr == pWorldPattern || nullptr == pWorldStage ||
				nullptr == pWorldTrigger ||
				!Is_StableToken(pWorldPattern->Get_String()) ||
				!Is_StableToken(pWorldStage->Get_String()) ||
				!Is_StableToken(pWorldTrigger->Get_String()) ||
				!WorldReferenceIdentities.insert(
					pWorldPattern->Get_String() + ":" +
					pWorldStage->Get_String() + ":" +
					pWorldTrigger->Get_String()).second)
			{
				strOutError =
					"master worldEventTriggerRefs is invalid or duplicated";
				return false;
			}
			Client::VALTAN_WORLD_EVENT_TRIGGER_REF_VIEW Reference;
			Reference.strPatternId = pWorldPattern->Get_String();
			Reference.strStageId = pWorldStage->Get_String();
			Reference.strTriggerKind = pWorldTrigger->Get_String();
			Out.WorldEventTriggerRefs.push_back(std::move(Reference));
		}

		Out.strPatternId = pPatternId->Get_String();
		Out.strCategory = Required(
			Value, "category", DATA_JSON_TYPE::STRING)->Get_String();
		Out.iMinimumPhase = static_cast<uint32_t>(
			Value.Find("minimumPhase")->Get_Number());
		Out.iMaximumPhase = static_cast<uint32_t>(
			Value.Find("maximumPhase")->Get_Number());
		Out.strTargetPolicy = Required(
			Value, "targetPolicy", DATA_JSON_TYPE::STRING)->Get_String();
		Out.strAimPolicy = Required(
			Value, "aimPolicy", DATA_JSON_TYPE::STRING)->Get_String();
		Out.strDisplayName = pDisplay->Get_String();
		Out.strActionId = pAction->Get_String();
		for (const DATA_JSON_VALUE& SourceAction : pSourceActions->Get_Array())
		{
			Out.SourceActionIds.push_back(static_cast<uint32_t>(
				SourceAction.Get_Number()));
		}
		Out.strSelectionMode = Required(
			Value, "selectionMode", DATA_JSON_TYPE::STRING)->Get_String();
		Out.iMinimumHealthBar = static_cast<int32_t>(
			Value.Find("minimumHealthBar")->Get_Number());
		Out.iMaximumHealthBar = static_cast<int32_t>(
			Value.Find("maximumHealthBar")->Get_Number());
		Out.iTriggerHealthBar = static_cast<int32_t>(
			Value.Find("triggerHealthBar")->Get_Number());
		Out.iTriggerOrder = static_cast<uint32_t>(
			Value.Find("triggerOrder")->Get_Number());
		Out.strArmorRequirement = Required(
			Value, "armorRequirement", DATA_JSON_TYPE::STRING)->Get_String();
		Out.strPhaseRequirement = Required(
			Value, "phaseRequirement", DATA_JSON_TYPE::STRING)->Get_String();
		Out.bInvulnerableWhileRunning = Required(
			Value, "invulnerableWhileRunning",
			DATA_JSON_TYPE::BOOLEAN)->Get_Boolean();
		Out.iSelectionWeight = static_cast<uint32_t>(
			Value.Find("selectionWeight")->Get_Number());
		Out.iMaximumConsecutiveUses = static_cast<uint32_t>(
			Value.Find("maximumConsecutiveUses")->Get_Number());
		Out.fMinimumRange = static_cast<f32_t>(
			Value.Find("minimumRange")->Get_Number());
		Out.fMaximumRange = static_cast<f32_t>(
			Value.Find("maximumRange")->Get_Number());
		if (!Read_PatternServerMotion(Value.Find("serverMotion"), Out.ServerMotion) ||
			!Read_PatternFinale(Value.Find("finale"), Out.Finale) ||
			(Out.Finale.has_value() && Out.bInvulnerableWhileRunning))
		{
			strOutError = "master serverMotion/finale values are invalid";
			return false;
		}
		Out.iSourceSequenceIndex = static_cast<uint32_t>(
			pSourceSequence->Get_Number());
		for (const DATA_JSON_VALUE& Source :
			pPresentationSources->Get_Array())
		{
			Client::VALTAN_PRESENTATION_SOURCE_VIEW Presentation;
			Presentation.iSourceActionId = static_cast<uint32_t>(
				Source.Find("sourceActionId")->Get_Number());
			Presentation.iSequenceIndex = static_cast<uint32_t>(
				Source.Find("sequenceIndex")->Get_Number());
			Presentation.strRole = Required(
				Source, "role", DATA_JSON_TYPE::STRING)->Get_String();
			Out.PresentationSources.push_back(std::move(Presentation));
		}
		std::set<std::string, std::less<>> StageIds;
		for (const DATA_JSON_VALUE& StageValue : pStages->Get_Array())
		{
			MASTER_STAGE Stage;
			if (!Parse_MasterStage(StageValue, Stage, strOutError) ||
				!StageIds.insert(Stage.strStageId).second)
			{
				if (strOutError.empty())
					strOutError = "master pattern duplicates a stageId";
				return false;
			}
			Out.Stages.push_back(std::move(Stage));
		}
		for (const Client::VALTAN_PATTERN_REACTION_VIEW& Reaction :
			Out.Reactions)
		{
			if (StageIds.end() == StageIds.find(Reaction.strStageId))
			{
				strOutError = "master reaction targets an unknown stage: " +
					Reaction.strStageId;
				return false;
			}
		}
		for (const Client::VALTAN_WORLD_EVENT_TRIGGER_REF_VIEW& Reference :
			Out.WorldEventTriggerRefs)
		{
			if (Reference.strPatternId != Out.strPatternId ||
				StageIds.end() == StageIds.find(Reference.strStageId))
			{
				strOutError =
					"master world Event trigger leaves its managed pattern/stage: " +
					Reference.strPatternId + "/" + Reference.strStageId;
				return false;
			}
		}
		if (!Validate_PatternServerMotionStageWindows(
				Out.ServerMotion, Out.Stages))
		{
			strOutError =
				"master serverMotion window is outside its authored stage: " +
				Out.strPatternId;
			return false;
		}
		return true;
	}

	bool_t Parse_MasterIndependentEffect(
		const DATA_JSON_VALUE& Value,
		Client::VALTAN_INDEPENDENT_EFFECT_VIEW& Out,
		std::string& strOutError)
	{
		if (!Has_ExactProperties(Value,
				{ "independentEffectId", "displayName", "effectAssetId",
				  "ownership", "ownerPatternId", "ownerStageId",
				  "triggerPolicy", "combatObjectArchetypeId", "clientVisualId",
				  "effectCueBindingId", "cueProjection" }))
		{
			strOutError = "master independent Effect has unexpected properties";
			return false;
		}
		const DATA_JSON_VALUE* pId = Required(
			Value, "independentEffectId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pDisplay = Required(
			Value, "displayName", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pAsset = Required(
			Value, "effectAssetId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pOwnership = Required(
			Value, "ownership", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pPattern = Required(
			Value, "ownerPatternId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pStage = Required(
			Value, "ownerStageId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pTrigger = Required(
			Value, "triggerPolicy", DATA_JSON_TYPE::STRING);
		if (nullptr == pId || nullptr == pDisplay || nullptr == pAsset ||
			nullptr == pOwnership || nullptr == pPattern || nullptr == pStage ||
			nullptr == pTrigger || !Is_StableToken(pId->Get_String()) ||
			pDisplay->Get_String().empty() || !Is_StableToken(pAsset->Get_String()) ||
			!Is_StableToken(pPattern->Get_String()) ||
			!Is_StableToken(pStage->Get_String()))
		{
			strOutError = "master independent Effect identity is invalid";
			return false;
		}
		Out.strIndependentEffectId = pId->Get_String();
		Out.strDisplayName = pDisplay->Get_String();
		Out.strEffectAssetId = pAsset->Get_String();
		Out.strOwnership = pOwnership->Get_String();
		Out.strOwnerPatternId = pPattern->Get_String();
		Out.strOwnerStageId = pStage->Get_String();
		Out.strTriggerPolicy = pTrigger->Get_String();
		if (!Read_NullableStableToken(Value, "combatObjectArchetypeId",
				Out.strCombatObjectArchetypeId) ||
			!Read_NullableStableToken(Value, "clientVisualId",
				Out.strClientVisualId) ||
			!Read_NullableStableToken(Value, "effectCueBindingId",
				Out.strEffectCueBindingId))
		{
			strOutError = "master independent Effect owner identity is invalid";
			return false;
		}
		const DATA_JSON_VALUE* pProjection = Value.Find("cueProjection");
		if (nullptr == pProjection)
		{
			strOutError = "master independent Effect cueProjection is missing";
			return false;
		}
		const bool_t bCombatObject = "SERVER_COMBAT_OBJECT" == Out.strOwnership;
		const bool_t bPatternStage = "SERVER_PATTERN_STAGE" == Out.strOwnership;
		if ((!bCombatObject && !bPatternStage) ||
			(bCombatObject && (Out.strCombatObjectArchetypeId.empty() ||
				Out.strClientVisualId.empty() ||
				!Out.strEffectCueBindingId.empty())) ||
			(bPatternStage && (!Out.strCombatObjectArchetypeId.empty() ||
				!Out.strClientVisualId.empty() ||
				Out.strEffectCueBindingId.empty())))
		{
			strOutError = "master independent Effect ownership tuple is invalid";
			return false;
		}
		if (bCombatObject)
		{
			if (!pProjection->Is_Null())
			{
				strOutError =
					"master combat-object Effect cannot project a boss-root cue";
				return false;
			}
			return true;
		}
		if (!pProjection->Is_Object())
		{
			strOutError =
				"master pattern-stage Effect cueProjection contract is invalid";
			return false;
		}
		if (Has_ExactProperties(
				*pProjection, { "timingBasis", "stageOffsetMs" }))
		{
			const DATA_JSON_VALUE* pTimingBasis = Required(
				*pProjection, "timingBasis", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pStageOffset =
				pProjection->Find("stageOffsetMs");
			if (nullptr == pTimingBasis ||
				"STAGE_CLOCK" != pTimingBasis->Get_String() ||
				!Is_NonNegativeInteger(pStageOffset))
			{
				strOutError =
					"master pattern-stage Effect stage-clock projection is invalid";
				return false;
			}
			Out.bHasCueProjection = true;
			Out.bUsesCueStageClock = true;
			Out.iCueStageOffsetMs = static_cast<uint32_t>(
				pStageOffset->Get_Number());
			return true;
		}
		if (!Has_ExactProperties(*pProjection,
				{ "clipOccurrenceId", "sourceStartMs", "sourceEndMs",
				  "mappingBasis" }))
		{
			strOutError =
				"master pattern-stage Effect cueProjection contract is invalid";
			return false;
		}
		const DATA_JSON_VALUE* pClipOccurrence = Required(
			*pProjection, "clipOccurrenceId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pSourceStart = pProjection->Find("sourceStartMs");
		const DATA_JSON_VALUE* pSourceEnd = pProjection->Find("sourceEndMs");
		const DATA_JSON_VALUE* pMappingBasis = Required(
			*pProjection, "mappingBasis", DATA_JSON_TYPE::STRING);
		if (nullptr == pClipOccurrence ||
			!Is_StableToken(pClipOccurrence->Get_String()) ||
			!Is_NonNegativeInteger(pSourceStart) || nullptr == pSourceEnd ||
			(!pSourceEnd->Is_Null() && !Is_NonNegativeInteger(pSourceEnd)) ||
			nullptr == pMappingBasis || pMappingBasis->Get_String().empty())
		{
			strOutError =
				"master pattern-stage Effect cueProjection values are invalid";
			return false;
		}
		Out.bHasCueProjection = true;
		Out.strCueClipOccurrenceId = pClipOccurrence->Get_String();
		Out.iCueSourceStartMs = static_cast<uint32_t>(
			pSourceStart->Get_Number());
		Out.bHasCueSourceEnd = !pSourceEnd->Is_Null();
		Out.iCueSourceEndMs = Out.bHasCueSourceEnd ?
			static_cast<uint32_t>(pSourceEnd->Get_Number()) : 0u;
		Out.strCueMappingBasis = pMappingBasis->Get_String();
		return true;
	}

	bool_t Parse_MasterNormalSelection(
		const DATA_JSON_VALUE& Value,
		Client::VALTAN_NORMAL_SELECTION_VIEW& Out,
		std::string& strOutError)
	{
		if (!Has_ExactProperties(
				Value, { "selectionMode", "ranges", "patternIds" }))
		{
			strOutError = "master normalSelection has unexpected properties";
			return false;
		}
		const DATA_JSON_VALUE* pMode = Required(
			Value, "selectionMode", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pRanges = Required(
			Value, "ranges", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pPatternIds = Required(
			Value, "patternIds", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pMode || "WEIGHTED_POOL" != pMode->Get_String() ||
			nullptr == pRanges || pRanges->Get_Array().empty() ||
			nullptr == pPatternIds || pPatternIds->Get_Array().empty())
		{
			strOutError = "master normalSelection is invalid";
			return false;
		}
		Out.strSelectionMode = pMode->Get_String();
		std::set<std::string, std::less<>> RangeIds;
		uint32_t iPreviousFromHealthBar =
			(std::numeric_limits<uint32_t>::max)();
		for (const DATA_JSON_VALUE& Range : pRanges->Get_Array())
		{
			const DATA_JSON_VALUE* pRangeId = Required(
				Range, "rotationId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pFrom = Range.Find("fromHealthBar");
			const DATA_JSON_VALUE* pTo = Range.Find("toHealthBar");
			if (!Has_ExactProperties(
					Range, { "rotationId", "fromHealthBar", "toHealthBar" }) ||
				nullptr == pRangeId || !Is_StableToken(pRangeId->Get_String()) ||
				!Is_NonNegativeInteger(pFrom) || !Is_NonNegativeInteger(pTo) ||
				0.0 == pFrom->Get_Number() || 0.0 == pTo->Get_Number())
			{
				strOutError = "master normalSelection range is invalid";
				return false;
			}
			Client::VALTAN_NORMAL_SELECTION_RANGE_VIEW RangeView;
			RangeView.strRotationId = pRangeId->Get_String();
			RangeView.iFromHealthBar = static_cast<uint32_t>(pFrom->Get_Number());
			RangeView.iToHealthBar = static_cast<uint32_t>(pTo->Get_Number());
			if (!RangeIds.insert(RangeView.strRotationId).second ||
				RangeView.iFromHealthBar <= RangeView.iToHealthBar ||
				RangeView.iFromHealthBar > iPreviousFromHealthBar)
			{
				strOutError = "master normalSelection range is duplicated or inverted";
				return false;
			}
			iPreviousFromHealthBar = RangeView.iFromHealthBar;
			Out.Ranges.push_back(std::move(RangeView));
		}
		std::set<std::string, std::less<>> PatternIds;
		for (const DATA_JSON_VALUE& PatternId : pPatternIds->Get_Array())
		{
			if (!PatternId.Is_String() ||
				!Is_StableToken(PatternId.Get_String()) ||
				!PatternIds.insert(PatternId.Get_String()).second)
			{
				strOutError =
					"master normalSelection patternId is invalid or duplicated";
				return false;
			}
			Out.PatternIds.push_back(PatternId.Get_String());
		}
		return true;
	}

	bool_t Parse_MasterCounterReactionLayer(
		const DATA_JSON_VALUE& Value,
		Client::VALTAN_COUNTER_REACTION_LAYER_VIEW& Out,
		std::string& strOutError)
	{
		if (!Has_ExactProperties(Value,
				{ "reactionLayerId", "admissionScope", "ownerPatternId",
				  "ownerStageId", "windowActionId", "successActionId",
				  "failureActionId" }))
		{
			strOutError =
				"master counter reaction layer has unexpected properties";
			return false;
		}
		const auto ReadStable = [&Value](const std::string_view Name)
			-> const DATA_JSON_VALUE*
		{
			const DATA_JSON_VALUE* pValue = Required(
				Value, Name, DATA_JSON_TYPE::STRING);
			return nullptr != pValue && Is_StableToken(pValue->Get_String()) ?
				pValue : nullptr;
		};
		const DATA_JSON_VALUE* pLayerId = ReadStable("reactionLayerId");
		const DATA_JSON_VALUE* pScope = Required(
			Value, "admissionScope", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pPattern = ReadStable("ownerPatternId");
		const DATA_JSON_VALUE* pStage = ReadStable("ownerStageId");
		const DATA_JSON_VALUE* pWindow = ReadStable("windowActionId");
		const DATA_JSON_VALUE* pSuccess = ReadStable("successActionId");
		const DATA_JSON_VALUE* pFailure = ReadStable("failureActionId");
		if (nullptr == pLayerId || nullptr == pScope ||
			"REFERENCE_ONLY_LEGACY" != pScope->Get_String() ||
			nullptr == pPattern || nullptr == pStage || nullptr == pWindow ||
			nullptr == pSuccess || nullptr == pFailure)
		{
			strOutError = "master counter reaction layer is invalid";
			return false;
		}
		Out.strReactionLayerId = pLayerId->Get_String();
		Out.strAdmissionScope = pScope->Get_String();
		Out.strOwnerPatternId = pPattern->Get_String();
		Out.strOwnerStageId = pStage->Get_String();
		Out.Window.strActionId = pWindow->Get_String();
		Out.Success.strActionId = pSuccess->Get_String();
		Out.Failure.strActionId = pFailure->Get_String();
		return true;
	}

	bool_t Parse_MasterDocument(
		const DATA_JSON_VALUE& Root,
		const std::set<std::string, std::less<>>& ScriptedEntryOnlyPatternIds,
		MASTER_DOCUMENT& Out,
		std::string& strOutError)
	{
		if (!Has_ExactProperties(Root,
				{ "schema", "formatVersion", "bossArchetypeId", "encounterId",
				  "scope", "previewPaths", "retiredPatternIds",
				  "normalSelection", "counterReactionLayers",
				  "independentEffects", "patterns" }) ||
			nullptr == Required(Root, "schema", DATA_JSON_TYPE::STRING) ||
			"lostark.valtan-pattern-master" !=
				Required(Root, "schema", DATA_JSON_TYPE::STRING)->Get_String() ||
			!Is_NonNegativeInteger(Root.Find("formatVersion")) ||
			1.0 != Root.Find("formatVersion")->Get_Number() ||
			nullptr == Required(Root, "bossArchetypeId", DATA_JSON_TYPE::STRING) ||
			"BOSS_VALTAN" != Required(
				Root, "bossArchetypeId", DATA_JSON_TYPE::STRING)->Get_String() ||
			nullptr == Required(Root, "encounterId", DATA_JSON_TYPE::STRING) ||
			"ENCOUNTER_VALTAN" != Required(
				Root, "encounterId", DATA_JSON_TYPE::STRING)->Get_String() ||
			nullptr == Required(Root, "scope", DATA_JSON_TYPE::STRING) ||
			"PHASE_ONE" != Required(
				Root, "scope", DATA_JSON_TYPE::STRING)->Get_String())
		{
			strOutError = "joined authoring compatibility root is invalid";
			return false;
		}
		const DATA_JSON_VALUE* pPreviewPaths = Required(
			Root, "previewPaths", DATA_JSON_TYPE::OBJECT);
		if (nullptr == pPreviewPaths || !Has_ExactProperties(*pPreviewPaths,
				{ "encounter", "animationBindings", "effectCues",
				  "combatObjects", "bossCatalog", "effectCatalog",
				  "damageProfiles", "cinematicCamera", "worldEvents",
				  "patternRotations", "sourceClipSequences" }))
		{
			strOutError = "joined authoring compatibility previewPaths contract is invalid";
			return false;
		}
		for (const auto& [Name, Value] : pPreviewPaths->Get_Object())
		{
			(void)Name;
			if (!Value.Is_String() || !Value.Get_String().starts_with("Data/"))
			{
				strOutError = "joined authoring compatibility preview path is invalid";
				return false;
			}
		}

		const DATA_JSON_VALUE* pRetired = Required(
			Root, "retiredPatternIds", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pIndependent = Required(
			Root, "independentEffects", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pNormalSelection = Required(
			Root, "normalSelection", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* pCounterReactions = Required(
			Root, "counterReactionLayers", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pPatterns = Required(
			Root, "patterns", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pRetired || nullptr == pIndependent ||
			nullptr == pNormalSelection || nullptr == pCounterReactions ||
			pCounterReactions->Get_Array().empty() ||
			nullptr == pPatterns || pIndependent->Get_Array().empty() ||
			pPatterns->Get_Array().empty())
		{
			strOutError = "joined authoring compatibility inventories are invalid";
			return false;
		}

		std::set<std::string, std::less<>> RetiredIds;
		for (const DATA_JSON_VALUE& Retired : pRetired->Get_Array())
		{
			if (!Retired.Is_String() || !Is_StableToken(Retired.Get_String()) ||
				!RetiredIds.insert(Retired.Get_String()).second)
			{
				strOutError = "master retiredPatternIds is invalid or duplicated";
				return false;
			}
			Out.RetiredPatternIds.push_back(Retired.Get_String());
		}
		if (!Parse_MasterNormalSelection(
				*pNormalSelection, Out.NormalSelection, strOutError))
		{
			return false;
		}
		std::set<std::string, std::less<>> CounterLayerIds;
		std::set<std::string, std::less<>> CounterOwnerStages;
		for (const DATA_JSON_VALUE& LayerValue :
			pCounterReactions->Get_Array())
		{
			Client::VALTAN_COUNTER_REACTION_LAYER_VIEW Layer;
			if (!Parse_MasterCounterReactionLayer(
					LayerValue, Layer, strOutError) ||
				!CounterLayerIds.insert(Layer.strReactionLayerId).second ||
				!CounterOwnerStages.insert(
					Layer.strOwnerPatternId + "/" + Layer.strOwnerStageId).second)
			{
				if (strOutError.empty())
					strOutError =
						"master counter reaction identity is duplicated";
				return false;
			}
			Out.CounterReactionLayers.push_back(std::move(Layer));
		}

		std::set<std::string, std::less<>> IndependentIds;
		std::set<std::string, std::less<>> IndependentAssets;
		for (const DATA_JSON_VALUE& IndependentValue : pIndependent->Get_Array())
		{
			Client::VALTAN_INDEPENDENT_EFFECT_VIEW Independent;
			if (!Parse_MasterIndependentEffect(
					IndependentValue, Independent, strOutError) ||
				!IndependentIds.insert(
					Independent.strIndependentEffectId).second ||
				!IndependentAssets.insert(Independent.strEffectAssetId).second)
			{
				if (strOutError.empty())
					strOutError = "master independent Effect is duplicated";
				return false;
			}
			Out.IndependentEffects.push_back(std::move(Independent));
		}

		std::set<std::string, std::less<>> PatternIds;
		for (const DATA_JSON_VALUE& PatternValue : pPatterns->Get_Array())
		{
			MASTER_PATTERN Pattern;
			if (!Parse_MasterPattern(PatternValue, Pattern, strOutError) ||
				!PatternIds.insert(Pattern.strPatternId).second ||
				RetiredIds.contains(Pattern.strPatternId))
			{
				if (strOutError.empty())
					strOutError = "master pattern identity is duplicated or retired";
				return false;
			}
			Out.Patterns.push_back(std::move(Pattern));
		}
		for (const MASTER_PATTERN& Pattern : Out.Patterns)
		{
			for (const MASTER_STAGE& Stage : Pattern.Stages)
			{
				for (const Client::VALTAN_STAGE_BRANCH_VIEW& Branch :
					Stage.Branches)
				{
					if (Branch.strNextPatternId.has_value() &&
						!PatternIds.contains(*Branch.strNextPatternId))
					{
						strOutError =
							"master branch targets an unknown pattern: " +
							*Branch.strNextPatternId;
						return false;
					}
				}
			}
		}
		std::set<std::string, std::less<>> FinitePatternIds = {
			"VALTAN_TRASH", "VALTAN_TRASH_CATCH_IF",
			"VALTAN_TRASH_CATCH_SUCCESS", "VALTAN_TRASH_CATCH_FAIL" };
		for (const MASTER_PATTERN& Pattern : Out.Patterns)
		{
			if (!Pattern.Finale.has_value())
				continue;
			FinitePatternIds.insert(Pattern.strPatternId);
			for (const std::string& ChildId : Pattern.Finale->GhostPatternIds)
			{
				const auto Child = std::find_if(Out.Patterns.begin(), Out.Patterns.end(),
					[&ChildId](const MASTER_PATTERN& Candidate)
					{ return Candidate.strPatternId == ChildId; });
				if (Child == Out.Patterns.end() || ChildId == Pattern.strPatternId ||
					Child->Finale.has_value() || !Child->WorldEventTriggerRefs.empty())
				{
					strOutError = "master finale has an unresolved, recursive or terrain ghost attack: " + ChildId;
					return false;
				}
				FinitePatternIds.insert(ChildId);
			}
		}
		for (const MASTER_PATTERN& Pattern : Out.Patterns)
			if (FinitePatternIds.contains(Pattern.strPatternId) &&
				!Validate_FinitePatternStageGraph(Pattern, strOutError))
				return false;
		std::set<std::string, std::less<>> ManagedNormalPatternIds;
		for (const MASTER_PATTERN& Pattern : Out.Patterns)
		{
			if ("NORMAL" == Pattern.strSelectionMode)
				ManagedNormalPatternIds.insert(Pattern.strPatternId);
		}
		const std::set<std::string, std::less<>> WeightedPatternIds(
			Out.NormalSelection.PatternIds.begin(),
			Out.NormalSelection.PatternIds.end());
		/* Optional entry definitions retain NORMAL Product metadata, but the
		   split decision model does not put either one in the weighted pool. */
		for (const std::string& PatternId : ScriptedEntryOnlyPatternIds)
		{
			if (0u == ManagedNormalPatternIds.erase(PatternId) ||
				WeightedPatternIds.contains(PatternId))
			{
				strOutError =
					"master optional entry-only gate must be an unweighted NORMAL pattern";
				return false;
			}
		}
		if (WeightedPatternIds != ManagedNormalPatternIds)
		{
			strOutError =
				"master normalSelection is not a subset of the managed normal pattern set";
			return false;
		}
		for (const Client::VALTAN_COUNTER_REACTION_LAYER_VIEW& Layer :
			Out.CounterReactionLayers)
		{
			if (PatternIds.contains(Layer.strOwnerPatternId))
			{
				strOutError =
					"reference-only counter reaction admits a managed pattern";
				return false;
			}
		}
		return true;
	}

	bool_t Read_OptionalOrderedHitOffsets(
		const DATA_JSON_VALUE& Object,
		std::vector<uint32_t>& OutOffsets)
	{
		OutOffsets.clear();
		const DATA_JSON_VALUE* pOffsets = Object.Find("hitOffsetsMs");
		if (nullptr == pOffsets)
			return true;
		if (!pOffsets->Is_Array() || pOffsets->Get_Array().empty() ||
			pOffsets->Get_Array().size() > 1000u)
		{
			return false;
		}

		uint32_t iPrevious = 0u;
		for (size_t i = 0u; i < pOffsets->Get_Array().size(); ++i)
		{
			const DATA_JSON_VALUE& Value = pOffsets->Get_Array()[i];
			if (!Value.Is_Number() || !std::isfinite(Value.Get_Number()) ||
				std::floor(Value.Get_Number()) != Value.Get_Number() ||
				Value.Get_Number() < 0.0 ||
				Value.Get_Number() > static_cast<double>(
					(std::numeric_limits<uint32_t>::max)()))
			{
				return false;
			}
			const uint32_t iCurrent = static_cast<uint32_t>(Value.Get_Number());
			if (0u != i && iCurrent <= iPrevious)
				return false;
			OutOffsets.push_back(iCurrent);
			iPrevious = iCurrent;
		}
		return true;
	}

	struct COMBAT_OBJECT_EFFECT_REFERENCE final
	{
		std::string strClientVisualId;
		std::string strEffectAssetId;
		std::string strOwnerPatternId;
		std::string strOwnerStageActionId;
		std::string strKind;
		std::string strOriginPolicy;
		std::string strDirectionPolicy;
		f32_t fSpeedMps = 0.f;
		f32_t fMaximumDistanceM = 0.f;
		uint32_t iLifetimeMs = 0u;
		std::vector<std::string> HitIds;
		std::vector<uint32_t> HitOffsetsMs;
		std::vector<Client::VALTAN_COMBAT_OBJECT_PRESENTATION_EVENT_VIEW>
			PresentationEvents;
	};

	Client::VALTAN_CLIP_OCCURRENCE_VIEW Build_ClipOccurrenceView(
		const Client::BOSS_PATTERN_ANIMATION_CLIP& Clip)
	{
		Client::VALTAN_CLIP_OCCURRENCE_VIEW View;
		View.strClipOccurrenceId = Clip.strClipOccurrenceId;
		View.strClipName = Clip.strClipName;
		View.strMappingBasis = Clip.strMappingBasis;
		View.iSourceStartMs = Clip.iSourceStartMs;
		View.iPlayMs = Clip.iPlayMs;
		View.fPlayRate = Clip.fPlayRate;
		View.bLoop = Clip.bLoop;
		return View;
	}

	Client::VALTAN_PATTERN_VIEW* Find_Pattern(
		Client::VALTAN_PATTERN_TREE_VIEW& View,
		const std::string_view strPatternId)
	{
		for (std::vector<Client::VALTAN_PATTERN_VIEW>* pGroup :
			{ &View.Gimmicks, &View.Rotation })
		{
			const auto Found = std::find_if(
				pGroup->begin(), pGroup->end(),
				[strPatternId](const Client::VALTAN_PATTERN_VIEW& Pattern)
				{
					return Pattern.strPatternId == strPatternId;
				});
			if (Found != pGroup->end())
				return &*Found;
		}
		return nullptr;
	}

	const Client::VALTAN_PATTERN_VIEW* Find_Pattern(
		const Client::VALTAN_PATTERN_TREE_VIEW& View,
		const std::string_view strPatternId)
	{
		for (const std::vector<Client::VALTAN_PATTERN_VIEW>* pGroup :
			{ &View.Gimmicks, &View.Rotation })
		{
			const auto Found = std::find_if(
				pGroup->begin(), pGroup->end(),
				[strPatternId](const Client::VALTAN_PATTERN_VIEW& Pattern)
				{
					return Pattern.strPatternId == strPatternId;
				});
			if (Found != pGroup->end())
				return &*Found;
		}
		return nullptr;
	}

	Client::VALTAN_STAGE_VIEW* Find_Stage(
		Client::VALTAN_PATTERN_VIEW& Pattern,
		const std::string_view strStageId)
	{
		const auto Found = std::find_if(
			Pattern.Stages.begin(), Pattern.Stages.end(),
			[strStageId](const Client::VALTAN_STAGE_VIEW& Stage)
			{
				return Stage.strStageId == strStageId;
			});
		return Found == Pattern.Stages.end() ? nullptr : &*Found;
	}

	bool_t Equal_MasterOccurrence(
		const Client::VALTAN_CLIP_OCCURRENCE_VIEW& Product,
		const Client::VALTAN_CLIP_OCCURRENCE_VIEW& Master)
	{
		return Product.strClipOccurrenceId == Master.strClipOccurrenceId &&
			Product.strClipName == Master.strClipName &&
			Product.strMappingBasis == Master.strMappingBasis &&
			Product.iSourceStartMs == Master.iSourceStartMs &&
			Product.iPlayMs == Master.iPlayMs &&
			std::abs(Product.fPlayRate - Master.fPlayRate) <= 0.000001f &&
			Product.bLoop == Master.bLoop;
	}

	bool_t Equal_AuthoredCue(
		const Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW& Product,
		const Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW& Authored)
	{
		return Product.strBindingId == Authored.strBindingId &&
			Product.strOccurrenceId == Authored.strOccurrenceId &&
			Product.strPatternId == Authored.strPatternId &&
			Product.strStageId == Authored.strStageId &&
			Product.strActionId == Authored.strActionId &&
			Product.strClipOccurrenceId == Authored.strClipOccurrenceId &&
			Product.strEffectAssetId == Authored.strEffectAssetId &&
			Product.strAnchorSlotId == Authored.strAnchorSlotId &&
			Product.eFollowPolicy == Authored.eFollowPolicy &&
			Product.eStopPolicy == Authored.eStopPolicy &&
			Product.strRepeatPolicy == Authored.strRepeatPolicy &&
			Product.eScalePolicy == Authored.eScalePolicy &&
			Product.strScalePolicy == Authored.strScalePolicy &&
			Product.bHasExplicitScalePolicy ==
				Authored.bHasExplicitScalePolicy &&
			Product.bUsesStageClock == Authored.bUsesStageClock &&
			Product.iStageOffsetMs == Authored.iStageOffsetMs &&
			Product.vWorldScale.x == Authored.vWorldScale.x &&
			Product.vWorldScale.y == Authored.vWorldScale.y &&
			Product.vWorldScale.z == Authored.vWorldScale.z &&
			Product.iSourceStartMs == Authored.iSourceStartMs &&
			Product.bHasSourceEnd == Authored.bHasSourceEnd &&
			(!Product.bHasSourceEnd ||
			 Product.iSourceEndMs == Authored.iSourceEndMs) &&
			Product.LocalTransform.vPosition.x == Authored.LocalTransform.vPosition.x &&
			Product.LocalTransform.vPosition.y == Authored.LocalTransform.vPosition.y &&
			Product.LocalTransform.vPosition.z == Authored.LocalTransform.vPosition.z &&
			Product.LocalTransform.vRotationDegrees.x ==
				Authored.LocalTransform.vRotationDegrees.x &&
			Product.LocalTransform.vRotationDegrees.y ==
				Authored.LocalTransform.vRotationDegrees.y &&
			Product.LocalTransform.vRotationDegrees.z ==
				Authored.LocalTransform.vRotationDegrees.z &&
			Product.LocalTransform.vScale.x == Authored.LocalTransform.vScale.x &&
			Product.LocalTransform.vScale.y == Authored.LocalTransform.vScale.y &&
			Product.LocalTransform.vScale.z == Authored.LocalTransform.vScale.z;
	}

	bool_t Equal_StageMotion(
		const std::optional<Client::VALTAN_STAGE_MOTION_VIEW>& Left,
		const std::optional<Client::VALTAN_STAGE_MOTION_VIEW>& Right)
	{
		if (Left.has_value() != Right.has_value())
			return false;
		return !Left.has_value() ||
			(Left->strKind == Right->strKind &&
			 Left->iRetargetDelayMs == Right->iRetargetDelayMs &&
			 Left->fSpeedMps == Right->fSpeedMps &&
			 Left->fDistance == Right->fDistance &&
			 Left->iCornerIndex == Right->iCornerIndex &&
			 Left->HalfExtentsM == Right->HalfExtentsM);
	}

	bool_t Equal_StageActions(
		const std::vector<Client::VALTAN_STAGE_ACTION_VIEW>& Left,
		const std::vector<Client::VALTAN_STAGE_ACTION_VIEW>& Right)
	{
		if (Left.size() != Right.size())
			return false;
		for (size_t i = 0u; i < Left.size(); ++i)
		{
			if (Left[i].strTrigger != Right[i].strTrigger ||
				Left[i].strKind != Right[i].strKind ||
				Left[i].strTargetId != Right[i].strTargetId ||
				Left[i].fValue != Right[i].fValue ||
				Left[i].strReleaseMode != Right[i].strReleaseMode ||
				Left[i].fSpeedMps != Right[i].fSpeedMps ||
				Left[i].iDurationMs != Right[i].iDurationMs ||
				Left[i].fYawOffsetDegrees != Right[i].fYawOffsetDegrees)
			{
				return false;
			}
		}
		return true;
	}

	bool_t Equal_StageBranches(
		const std::vector<Client::VALTAN_STAGE_BRANCH_VIEW>& Left,
		const std::vector<Client::VALTAN_STAGE_BRANCH_VIEW>& Right)
	{
		if (Left.size() != Right.size())
			return false;
		for (size_t i = 0u; i < Left.size(); ++i)
		{
			if (Left[i].strOutcome != Right[i].strOutcome ||
				Left[i].strNextActionId != Right[i].strNextActionId ||
				Left[i].strNextPatternId != Right[i].strNextPatternId)
			{
				return false;
			}
		}
		return true;
	}

	bool_t Equal_CounterProxy(
		const std::optional<Client::VALTAN_COUNTER_PROXY_VIEW>& Left,
		const std::optional<Client::VALTAN_COUNTER_PROXY_VIEW>& Right)
	{
		if (Left.has_value() != Right.has_value())
			return false;
		return !Left.has_value() ||
			(Left->strSpace == Right->strSpace &&
			 Left->fForwardOffsetM == Right->fForwardOffsetM &&
			 Left->fRightOffsetM == Right->fRightOffsetM &&
			 Left->fRadiusM == Right->fRadiusM);
	}

	bool_t Equal_MasterStageGameplay(
		const Client::VALTAN_STAGE_VIEW& Product,
		const MASTER_STAGE& Master)
	{
		const bool_t bCounterEnabled = Master.Branches.end() != std::find_if(
			Master.Branches.begin(), Master.Branches.end(),
			[](const Client::VALTAN_STAGE_BRANCH_VIEW& Branch)
			{ return "COUNTER_HIT" == Branch.strOutcome; });
		const std::optional<Client::VALTAN_COUNTER_PROXY_VIEW> ProductCounterProxy =
			bCounterEnabled ? Master.CounterProxy : std::nullopt;
		return Product.strStageId == Master.strStageId &&
			Product.strActionId == Master.strActionId &&
			Product.strStageKind == Master.strStageKind &&
			Product.iDurationMs == Master.iDurationMs &&
			Product.strHitShape == Master.strHitShape &&
			Product.fHitOuterRadius == Master.fHitOuterRadius &&
			Product.fHitInnerRadius == Master.fHitInnerRadius &&
			Product.fHitAngleDegrees == Master.fHitAngleDegrees &&
			Product.fHitLength == Master.fHitLength &&
			Product.fHitHalfWidth == Master.fHitHalfWidth &&
			Product.iHitCount == Master.iHitCount &&
			Product.iHitIntervalMs == Master.iHitIntervalMs &&
			Product.iHitDelayMs == Master.iHitDelayMs &&
			Product.HitOffsetsMs == Master.HitOffsetsMs &&
			Product.bHasHitAnchor == Master.bHasHitAnchor &&
			Product.strHitAnchorKind == Master.strHitAnchorKind &&
			Product.fHitAnchorForwardOffsetM ==
				Master.fHitAnchorForwardOffsetM &&
			Product.fHitAnchorRightOffsetM == Master.fHitAnchorRightOffsetM &&
			Product.fHitAnchorYawOffsetDegrees ==
				Master.fHitAnchorYawOffsetDegrees &&
			Product.bHasHitActivation == Master.bHasHitActivation &&
			Product.iHitActivationStartMs == Master.iHitActivationStartMs &&
			Product.iHitActivationLifetimeMs ==
				Master.iHitActivationLifetimeMs &&
			Product.strServerDamageProfileId ==
				Master.strServerDamageProfileId &&
			Product.strPlayerResponse == Master.strPlayerResponse &&
			Product.strAttachmentSlot == Master.strAttachmentSlot &&
			Product.strPartDamagePolicy == Master.strPartDamagePolicy &&
			Equal_CounterProxy(Product.CounterProxy, ProductCounterProxy) &&
			Product.fPushRangeM == Master.fPushRangeM &&
			Product.iPushMs == Master.iPushMs &&
			Product.bKnockdown == Master.bKnockdown &&
			Product.iDownMs == Master.iDownMs &&
			Equal_StageMotion(Product.Motion, Master.Motion) &&
			Equal_StageActions(Product.Actions, Master.Actions) &&
			Equal_StageBranches(Product.Branches, Master.Branches);
	}

	bool_t Equal_PatternServerMotion(
		const std::optional<Client::VALTAN_PATTERN_SERVER_MOTION_VIEW>& Left,
		const std::optional<Client::VALTAN_PATTERN_SERVER_MOTION_VIEW>& Right)
	{
		if (Left.has_value() != Right.has_value())
			return false;
		return !Left.has_value() ||
			(Left->strKind == Right->strKind &&
			 Left->strAnchorId == Right->strAnchorId &&
			 Left->LandingPosition == Right->LandingPosition &&
			 Left->fApexHeight == Right->fApexHeight &&
			 Left->strTravelStageId == Right->strTravelStageId &&
			 Left->iTakeoffStartMs == Right->iTakeoffStartMs &&
			 Left->iTakeoffEndMs == Right->iTakeoffEndMs &&
			 Left->iTravelStartMs == Right->iTravelStartMs &&
			 Left->iTravelEndMs == Right->iTravelEndMs &&
			 Left->bMoveToAnchorBeforeTakeoff == Right->bMoveToAnchorBeforeTakeoff);
	}

	bool_t Equal_MasterPatternGameplay(
		const Client::VALTAN_PATTERN_VIEW& Product,
		const MASTER_PATTERN& Master)
	{
		return Product.strPatternId == Master.strPatternId &&
			Product.strCategory == Master.strCategory &&
			Product.iMinimumPhase == Master.iMinimumPhase &&
			Product.iMaximumPhase == Master.iMaximumPhase &&
			Product.strTargetPolicy == Master.strTargetPolicy &&
			Product.strAimPolicy == Master.strAimPolicy &&
			Product.strDisplayName == Master.strDisplayName &&
			Product.strActionId == Master.strActionId &&
			Product.SourceActionIds == Master.SourceActionIds &&
			Product.strSelectionMode == Master.strSelectionMode &&
			Product.iMinimumHealthBar == Master.iMinimumHealthBar &&
			Product.iMaximumHealthBar == Master.iMaximumHealthBar &&
			Product.iTriggerHealthBar == Master.iTriggerHealthBar &&
			Product.iTriggerOrder == Master.iTriggerOrder &&
			Product.strArmorRequirement == Master.strArmorRequirement &&
			Product.strPhaseRequirement == Master.strPhaseRequirement &&
			Product.bInvulnerableWhileRunning ==
				Master.bInvulnerableWhileRunning &&
			Product.iSelectionWeight == Master.iSelectionWeight &&
			Product.iMaximumConsecutiveUses ==
				Master.iMaximumConsecutiveUses &&
			Product.fMinimumRange == Master.fMinimumRange &&
			Product.fMaximumRange == Master.fMaximumRange &&
			Equal_PatternServerMotion(Product.ServerMotion, Master.ServerMotion) &&
			Product.Finale == Master.Finale;
	}

	bool_t Assign_MasterWallBudgets(
		Client::VALTAN_STAGE_VIEW& Stage,
		std::string& strOutError)
	{
		if (Stage.bSuppressAnimation)
		{
			if (!Stage.ClipOccurrences.empty() ||
				"NONE" != Stage.strAnimationEndPolicy)
			{
				strOutError =
					"NONE animation cannot own a clip wall budget: " +
					Stage.strActionId;
				return false;
			}
			return true;
		}
		uint64_t iKnownWallMs = 0u;
		size_t iUnknownCount = 0u;
		size_t iUnknownIndex = 0u;
		for (size_t i = 0u; i < Stage.ClipOccurrences.size(); ++i)
		{
			Client::VALTAN_CLIP_OCCURRENCE_VIEW& Clip =
				Stage.ClipOccurrences[i];
			Clip.iAuthoringWallMs = 0u;
			if (0u == Clip.iPlayMs)
			{
				++iUnknownCount;
				iUnknownIndex = i;
				continue;
			}
			const double WallMs = static_cast<double>(Clip.iPlayMs) /
				static_cast<double>(Clip.fPlayRate);
			if (!std::isfinite(WallMs) || WallMs <= 0.0 ||
				WallMs > static_cast<double>(
					(std::numeric_limits<uint32_t>::max)()))
			{
				strOutError = "master animation wall budget is invalid: " +
					Clip.strClipOccurrenceId;
				return false;
			}
			Clip.iAuthoringWallMs = static_cast<uint32_t>(std::llround(WallMs));
			iKnownWallMs += Clip.iAuthoringWallMs;
		}
		if (1u < iUnknownCount || iKnownWallMs >
			static_cast<uint64_t>(Stage.iDurationMs) + 2u)
		{
			strOutError =
				"master stage cannot derive one deterministic animation wall budget: " +
				Stage.strActionId;
			return false;
		}
		const bool_t bUnknownLoops = 1u == iUnknownCount &&
			Stage.ClipOccurrences[iUnknownIndex].bLoop;
		if (bUnknownLoops &&
			"LOOP_TO_STAGE_END" != Stage.strAnimationEndPolicy)
		{
			strOutError =
				"repeatUntilStageEnd requires LOOP_TO_STAGE_END: " +
				Stage.strActionId;
			return false;
		}
		if ("LOOP_TO_STAGE_END" == Stage.strAnimationEndPolicy)
		{
			if (1u != iUnknownCount || !bUnknownLoops ||
				iKnownWallMs >= Stage.iDurationMs)
			{
				strOutError =
					"LOOP_TO_STAGE_END requires one looping remainder: " +
					Stage.strActionId;
				return false;
			}
			const uint64_t iRemaining =
				static_cast<uint64_t>(Stage.iDurationMs) - iKnownWallMs;
			Stage.ClipOccurrences[iUnknownIndex].iAuthoringWallMs =
				static_cast<uint32_t>(iRemaining);
			iKnownWallMs += iRemaining;
		}
		else if ("HOLD_LAST_POSE" == Stage.strAnimationEndPolicy)
		{
			if (bUnknownLoops || iKnownWallMs >= Stage.iDurationMs + 2ull)
			{
				strOutError = "HOLD_LAST_POSE animation budget is invalid: " +
					Stage.strActionId;
				return false;
			}
			if (1u == iUnknownCount)
			{
				if (iKnownWallMs >= Stage.iDurationMs)
					return false;
				Stage.ClipOccurrences[iUnknownIndex].iAuthoringWallMs =
					static_cast<uint32_t>(Stage.iDurationMs - iKnownWallMs);
				iKnownWallMs = Stage.iDurationMs;
			}
			else if (iKnownWallMs < Stage.iDurationMs &&
				!Stage.ClipOccurrences.empty())
			{
				Stage.ClipOccurrences.back().iAuthoringWallMs +=
					static_cast<uint32_t>(Stage.iDurationMs - iKnownWallMs);
				iKnownWallMs = Stage.iDurationMs;
			}
		}
		else if ("EXACT" == Stage.strAnimationEndPolicy)
		{
			const int64_t iDifference = static_cast<int64_t>(Stage.iDurationMs) -
				static_cast<int64_t>(iKnownWallMs);
			if (0u != iUnknownCount || std::abs(iDifference) > 2 ||
				Stage.ClipOccurrences.empty())
			{
				strOutError = "EXACT animation does not fill its Server stage: " +
					Stage.strActionId;
				return false;
			}
			const int64_t iCorrected = static_cast<int64_t>(
				Stage.ClipOccurrences.back().iAuthoringWallMs) + iDifference;
			if (iCorrected <= 0)
				return false;
			Stage.ClipOccurrences.back().iAuthoringWallMs =
				static_cast<uint32_t>(iCorrected);
			iKnownWallMs = Stage.iDurationMs;
		}
		else
		{
			strOutError = "master animation endPolicy is invalid: " +
				Stage.strActionId;
			return false;
		}
		if (iKnownWallMs != Stage.iDurationMs)
		{
			strOutError = "master animation wall budget did not commit exactly: " +
				Stage.strActionId;
			return false;
		}
		return true;
	}

	struct SPLIT_MECHANIC final
	{
		uint32_t iHealthBar = 0u;
		uint32_t iTriggerOrder = 0u;
	};

	struct SPLIT_CUE_OWNER final
	{
		std::string strPatternId;
		std::string strStageId;
		const DATA_JSON_VALUE* pCue = nullptr;
	};

	struct SPLIT_SPAWN_OWNER final
	{
		std::string strPatternId;
		std::string strStageId;
		std::string strCombatObjectArchetypeId;
	};

	struct SPLIT_INDEPENDENT_DECLARATION final
	{
		std::string strIndependentEffectId;
		std::string strDisplayName;
		std::string strOwnership;
		std::string strReferenceId;
	};

	DATA_JSON_VALUE Build_StringArray(
		const std::vector<std::string>& Values)
	{
		DATA_JSON_VALUE::ARRAY Result;
		Result.reserve(Values.size());
		for (const std::string& Value : Values)
			Result.push_back(DATA_JSON_VALUE::String(Value));
		return DATA_JSON_VALUE::Array(std::move(Result));
	}

	bool_t Validate_SplitRootIdentity(
		const DATA_JSON_VALUE& Gameplay,
		const DATA_JSON_VALUE& Presentation,
		std::string& strOutError)
	{
		if (!Has_ExactProperties(Gameplay,
				{ "schema", "formatVersion", "bossArchetypeId", "encounterId",
				  "scope", "previewPaths", "retiredPatternIds", "decisionModel",
				  "counterReactionLayers", "patterns" }) ||
			!Has_ExactProperties(Presentation,
				{ "schema", "formatVersion", "bossArchetypeId", "encounterId",
				  "scope", "patterns", "independentEffects" }))
		{
			strOutError = "split authoring root has unexpected properties";
			return false;
		}
		const DATA_JSON_VALUE* pGameplaySchema = Required(
			Gameplay, "schema", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pPresentationSchema = Required(
			Presentation, "schema", DATA_JSON_TYPE::STRING);
		if (nullptr == pGameplaySchema ||
			"lostark.valtan-gameplay-authoring" != pGameplaySchema->Get_String() ||
			nullptr == pPresentationSchema ||
			"lostark.valtan-pattern-presentation-authoring" !=
				pPresentationSchema->Get_String() ||
			!Is_NonNegativeInteger(Gameplay.Find("formatVersion")) ||
			1.0 != Gameplay.Find("formatVersion")->Get_Number() ||
			!Is_NonNegativeInteger(Presentation.Find("formatVersion")) ||
			1.0 != Presentation.Find("formatVersion")->Get_Number())
		{
			strOutError = "split authoring schema/version is invalid";
			return false;
		}
		for (const std::string_view Field :
			{ "bossArchetypeId", "encounterId", "scope" })
		{
			const DATA_JSON_VALUE* pGameplayValue = Required(
				Gameplay, Field, DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pPresentationValue = Required(
				Presentation, Field, DATA_JSON_TYPE::STRING);
			if (nullptr == pGameplayValue || nullptr == pPresentationValue ||
				pGameplayValue->Get_String() != pPresentationValue->Get_String())
			{
				strOutError = "split authoring root identity mismatch: " +
					std::string(Field);
				return false;
			}
		}
		if ("BOSS_VALTAN" != Read_String(Gameplay, "bossArchetypeId") ||
			"ENCOUNTER_VALTAN" != Read_String(Gameplay, "encounterId") ||
			"PHASE_ONE" != Read_String(Gameplay, "scope"))
		{
			strOutError = "split authoring root identity is not Valtan Phase One";
			return false;
		}
		return true;
	}

	bool_t Validate_SplitCue(
		const DATA_JSON_VALUE& Cue,
		std::string& strOutError)
	{
		const bool_t bUsesStageClock = nullptr != Cue.Find("timingBasis");
		const bool_t bValidShape = bUsesStageClock ?
			Has_ExactProperties(Cue,
				{ "cueId", "occurrenceId", "effectAssetId", "timingBasis",
				  "stageOffsetMs", "anchorSlotId", "followPolicy", "stopPolicy",
				  "repeatPolicy", "localTransform", "scalePolicy" }) :
			Has_ExactProperties(Cue,
				{ "cueId", "occurrenceId", "effectAssetId", "clipOccurrenceId",
				  "sourceStartMs", "sourceEndMs", "anchorSlotId", "followPolicy",
				  "stopPolicy", "repeatPolicy", "localTransform",
				  "scalePolicy", "mappingBasis" });
		if (!bValidShape)
		{
			strOutError = "split presentation cue has unexpected properties";
			return false;
		}
		for (const std::string_view Field :
			{ "cueId", "occurrenceId", "effectAssetId", "anchorSlotId" })
		{
			const DATA_JSON_VALUE* pValue = Required(
				Cue, Field, DATA_JSON_TYPE::STRING);
			if (nullptr == pValue || !Is_StableToken(pValue->Get_String()))
			{
				strOutError = "split presentation cue identity is invalid: " +
					std::string(Field);
				return false;
			}
		}
		if (bUsesStageClock)
		{
			if ("STAGE_CLOCK" != Read_String(Cue, "timingBasis") ||
				!Is_NonNegativeInteger(Cue.Find("stageOffsetMs")) ||
				("follow" != Read_String(Cue, "followPolicy") &&
				 "snapshot" != Read_String(Cue, "followPolicy")) ||
				"natural" != Read_String(Cue, "stopPolicy") ||
				"once" != Read_String(Cue, "repeatPolicy"))
			{
				strOutError =
					"split stage-clock cue timing/policy is invalid";
				return false;
			}
		}
		else
		{
			const DATA_JSON_VALUE* pClipOccurrence = Required(
				Cue, "clipOccurrenceId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* pSourceEnd = Cue.Find("sourceEndMs");
			if (nullptr == pClipOccurrence ||
				!Is_StableToken(pClipOccurrence->Get_String()) ||
				!Is_NonNegativeInteger(Cue.Find("sourceStartMs")) ||
				nullptr == pSourceEnd ||
				(!pSourceEnd->Is_Null() && !Is_NonNegativeInteger(pSourceEnd)) ||
				(!pSourceEnd->Is_Null() && pSourceEnd->Get_Number() <
					Cue.Find("sourceStartMs")->Get_Number()) ||
				Read_String(Cue, "mappingBasis").empty() ||
				("follow" != Read_String(Cue, "followPolicy") &&
				 "snapshot" != Read_String(Cue, "followPolicy")) ||
				("natural" != Read_String(Cue, "stopPolicy") &&
				 "cue_end" != Read_String(Cue, "stopPolicy")) ||
				("once" != Read_String(Cue, "repeatPolicy") &&
				 "each_loop" != Read_String(Cue, "repeatPolicy")))
			{
				strOutError = "split presentation cue timing/policy is invalid";
				return false;
			}
		}
		const DATA_JSON_VALUE* pTransform = Required(
			Cue, "localTransform", DATA_JSON_TYPE::OBJECT);
		if (nullptr == pTransform || !Has_ExactProperties(*pTransform,
				{ "position", "rotationDegrees", "scale" }))
		{
			strOutError = "split presentation cue transform is invalid";
			return false;
		}
		for (const std::string_view Field :
			{ "position", "rotationDegrees", "scale" })
		{
			const DATA_JSON_VALUE* pValues = Required(
				*pTransform, Field, DATA_JSON_TYPE::ARRAY);
			if (nullptr == pValues || 3u != pValues->Get_Array().size() ||
				std::any_of(pValues->Get_Array().begin(),
					pValues->Get_Array().end(),
					[](const DATA_JSON_VALUE& Value)
					{
						return !Is_FiniteNumber(&Value);
					}))
			{
				strOutError = "split presentation cue transform vector is invalid";
				return false;
			}
		}
		const DATA_JSON_VALUE* pScalePolicy = Required(
			Cue, "scalePolicy", DATA_JSON_TYPE::OBJECT);
		const std::string strScalePolicy = nullptr == pScalePolicy ?
			std::string{} : Read_String(*pScalePolicy, "kind");
		if (nullptr == pScalePolicy ||
			("OWNER_RELATIVE" == strScalePolicy &&
			 !Has_ExactProperties(*pScalePolicy, { "kind" })) ||
			(("GAMEPLAY_FOOTPRINT" == strScalePolicy ||
			  "ARENA_ABSOLUTE" == strScalePolicy) &&
				 (!Has_ExactProperties(*pScalePolicy, { "kind", "worldScale" }) ||
				  nullptr == Required(*pScalePolicy, "worldScale",
					  DATA_JSON_TYPE::ARRAY))) ||
			("OWNER_RELATIVE" != strScalePolicy &&
			 "GAMEPLAY_FOOTPRINT" != strScalePolicy &&
			 "ARENA_ABSOLUTE" != strScalePolicy))
		{
			strOutError = "split presentation cue scalePolicy is invalid";
			return false;
		}
		if ("OWNER_RELATIVE" != strScalePolicy)
		{
			const DATA_JSON_VALUE::ARRAY& WorldScale =
				pScalePolicy->Find("worldScale")->Get_Array();
			if (3u != WorldScale.size() ||
				std::any_of(WorldScale.begin(), WorldScale.end(),
					[](const DATA_JSON_VALUE& Value)
					{
						return !Is_FiniteNumber(&Value) ||
							Value.Get_Number() != 1.5;
					}))
			{
				strOutError =
					"split presentation cue worldScale must preserve 1.5";
				return false;
			}
		}
		return true;
	}

	Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW Build_SplitCueView(
		const DATA_JSON_VALUE& Cue,
		const std::string& strPatternId,
		const std::string& strStageId,
		const std::string& strActionId,
		const uint32_t iStageDurationMs)
	{
		Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW View;
		View.strBindingId = Read_String(Cue, "cueId");
		View.strOccurrenceId = Read_String(Cue, "occurrenceId");
		View.strPatternId = strPatternId;
		View.strStageId = strStageId;
		View.strActionId = strActionId;
		View.bUsesStageClock = nullptr != Cue.Find("timingBasis");
		View.strClipOccurrenceId = View.bUsesStageClock ? std::string{} :
			Read_String(Cue, "clipOccurrenceId");
		View.strEffectAssetId = Read_String(Cue, "effectAssetId");
		View.strAnchorSlotId = Read_String(Cue, "anchorSlotId");
		View.strFollowPolicy = Read_String(Cue, "followPolicy");
		View.strStopPolicy = Read_String(Cue, "stopPolicy");
		View.strRepeatPolicy = Read_String(Cue, "repeatPolicy");
		const DATA_JSON_VALUE& ScalePolicy = *Cue.Find("scalePolicy");
		View.strScalePolicy = Read_String(ScalePolicy, "kind");
		View.bHasExplicitScalePolicy = true;
		if ("GAMEPLAY_FOOTPRINT" == View.strScalePolicy)
			View.eScalePolicy =
				Client::VALTAN_PATTERN_EFFECT_SCALE_POLICY::GAMEPLAY_FOOTPRINT;
		else if ("ARENA_ABSOLUTE" == View.strScalePolicy)
			View.eScalePolicy =
				Client::VALTAN_PATTERN_EFFECT_SCALE_POLICY::ARENA_ABSOLUTE;
		else
			View.eScalePolicy =
				Client::VALTAN_PATTERN_EFFECT_SCALE_POLICY::OWNER_RELATIVE;
		if (Client::VALTAN_PATTERN_EFFECT_SCALE_POLICY::OWNER_RELATIVE !=
			View.eScalePolicy)
		{
			const DATA_JSON_VALUE::ARRAY& WorldScale =
				ScalePolicy.Find("worldScale")->Get_Array();
			View.vWorldScale = {
				static_cast<f32_t>(WorldScale[0].Get_Number()),
				static_cast<f32_t>(WorldScale[1].Get_Number()),
				static_cast<f32_t>(WorldScale[2].Get_Number()) };
		}
		View.eFollowPolicy = "snapshot" == View.strFollowPolicy ?
			Client::EFFECT_FOLLOW_POLICY::SNAPSHOT :
			Client::EFFECT_FOLLOW_POLICY::FOLLOW;
		View.eStopPolicy = "cue_end" == View.strStopPolicy ?
			Client::EFFECT_STOP_POLICY::CUE_END :
			Client::EFFECT_STOP_POLICY::NATURAL;
		if (View.bUsesStageClock)
		{
			View.iStageOffsetMs = static_cast<uint32_t>(
				Cue.Find("stageOffsetMs")->Get_Number());
			/* Keep the legacy display position meaningful while consumers move to
			   the explicit tagged field. It is not a clip-source position. */
			View.iSourceStartMs = View.iStageOffsetMs;
		}
		else
		{
			View.iSourceStartMs = static_cast<uint32_t>(
				Cue.Find("sourceStartMs")->Get_Number());
			View.bHasSourceEnd = !Cue.Find("sourceEndMs")->Is_Null();
			View.iSourceEndMs = View.bHasSourceEnd ? static_cast<uint32_t>(
				Cue.Find("sourceEndMs")->Get_Number()) : 0u;
		}
		View.iStageDurationMs = iStageDurationMs;
		const DATA_JSON_VALUE& Transform = *Cue.Find("localTransform");
		const DATA_JSON_VALUE::ARRAY& Position =
			Transform.Find("position")->Get_Array();
		const DATA_JSON_VALUE::ARRAY& Rotation =
			Transform.Find("rotationDegrees")->Get_Array();
		const DATA_JSON_VALUE::ARRAY& Scale =
			Transform.Find("scale")->Get_Array();
		View.LocalTransform.vPosition = {
			static_cast<f32_t>(Position[0].Get_Number()),
			static_cast<f32_t>(Position[1].Get_Number()),
			static_cast<f32_t>(Position[2].Get_Number()) };
		View.LocalTransform.vRotationDegrees = {
			static_cast<f32_t>(Rotation[0].Get_Number()),
			static_cast<f32_t>(Rotation[1].Get_Number()),
			static_cast<f32_t>(Rotation[2].Get_Number()) };
		View.LocalTransform.vScale = {
			static_cast<f32_t>(Scale[0].Get_Number()),
			static_cast<f32_t>(Scale[1].Get_Number()),
			static_cast<f32_t>(Scale[2].Get_Number()) };
		return View;
	}

	bool_t Build_SplitHitProjection(
		const DATA_JSON_VALUE& Hit,
		DATA_JSON_VALUE::OBJECT& OutStage,
		std::string& strOutError)
	{
		const DATA_JSON_VALUE* pShape = Required(
			Hit, "shape", DATA_JSON_TYPE::OBJECT);
		const std::string strKind = nullptr == pShape ? std::string{} :
			Read_String(*pShape, "kind");
		if (nullptr == pShape || strKind.empty())
		{
			strOutError = "split gameplay hit shape is invalid";
			return false;
		}
		auto PutNumber = [&OutStage](const std::string& Name, const double Value)
		{
			OutStage.emplace(Name, DATA_JSON_VALUE::Number(Value, true));
		};
		OutStage.emplace("hitShape", DATA_JSON_VALUE::String(strKind));
		for (const std::string& Name :
			{ "hitOuterRadius", "hitInnerRadius", "hitAngleDegrees",
			  "hitLength", "hitHalfWidth" })
		{
			PutNumber(Name, 0.0);
		}

		const auto RequireShape = [pShape, &strOutError](
			const std::initializer_list<std::string_view> Fields)
		{
			if (!Has_ExactProperties(*pShape, Fields))
			{
				strOutError = "split gameplay hit shape has unexpected properties";
				return false;
			}
			return true;
		};
		const auto ReadShapeNumber = [pShape, &OutStage, &strOutError](
			const std::string_view Source, const std::string& Target)
		{
			const DATA_JSON_VALUE* pValue = pShape->Find(Source);
			if (!Is_FiniteNumber(pValue) || pValue->Get_Number() < 0.0)
			{
				strOutError = "split gameplay hit shape numeric value is invalid";
				return false;
			}
			OutStage[Target] = *pValue;
			return true;
		};
		if ("NONE" == strKind)
		{
			if (!RequireShape({ "kind" }) ||
				!Has_ExactProperties(Hit, { "shape" }))
				return false;
			OutStage.emplace("hitCount", DATA_JSON_VALUE::Number(0));
			OutStage.emplace("hitIntervalMs", DATA_JSON_VALUE::Number(0));
			OutStage.emplace("hitDelayMs", DATA_JSON_VALUE::Number(0));
			OutStage.emplace("hitOffsetsMs", DATA_JSON_VALUE::Array({}));
			OutStage.emplace("serverDamageProfileId", DATA_JSON_VALUE::String(""));
			PutNumber("pushRangeM", 0.0);
			OutStage.emplace("pushMs", DATA_JSON_VALUE::Number(0));
			OutStage.emplace("knockdown", DATA_JSON_VALUE::Boolean(false));
			OutStage.emplace("downMs", DATA_JSON_VALUE::Number(0));
			return true;
		}

		const bool_t bCaptureHit = nullptr != Hit.Find("playerResponse") ||
			nullptr != Hit.Find("attachmentSlot");
		const DATA_JSON_VALUE* pSchedule = Hit.Find("schedule");
		const DATA_JSON_VALUE* pActivation = Hit.Find("activation");
		const DATA_JSON_VALUE* pAnchor = Hit.Find("anchor");
		if (!Has_ExactPropertiesWithOptional(Hit,
				{ "shape", "serverDamageProfileId", "pushRangeM", "pushMs",
				  "knockdown", "downMs" },
				{ "schedule", "activation", "anchor", "playerResponse",
				  "attachmentSlot" }) ||
			((nullptr == pSchedule) == (nullptr == pActivation)) ||
			((nullptr == Hit.Find("playerResponse")) !=
			 (nullptr == Hit.Find("attachmentSlot"))))
		{
			strOutError = "split gameplay hit has unexpected properties";
			return false;
		}
		if ("CIRCLE" == strKind)
		{
			if (!RequireShape({ "kind", "outerRadiusM" }) ||
				!ReadShapeNumber("outerRadiusM", "hitOuterRadius"))
				return false;
		}
		else if ("RING" == strKind)
		{
			if (!RequireShape({ "kind", "innerRadiusM", "outerRadiusM" }) ||
				!ReadShapeNumber("innerRadiusM", "hitInnerRadius") ||
				!ReadShapeNumber("outerRadiusM", "hitOuterRadius"))
				return false;
		}
		else if ("CONE" == strKind)
		{
			if (!RequireShape({ "kind", "angleDegrees", "lengthM" }) ||
				!ReadShapeNumber("angleDegrees", "hitAngleDegrees") ||
				!ReadShapeNumber("lengthM", "hitLength"))
				return false;
		}
		else if ("BOX" == strKind || "CROSS" == strKind ||
			"SIX_DIRECTIONS" == strKind)
		{
			if (!RequireShape({ "kind", "lengthM", "halfWidthM" }) ||
				!ReadShapeNumber("lengthM", "hitLength") ||
				!ReadShapeNumber("halfWidthM", "hitHalfWidth"))
				return false;
		}
		else
		{
			strOutError = "split gameplay hit shape kind is unsupported";
			return false;
		}

		if (nullptr != pAnchor)
		{
			f32_t fForward = 0.f;
			f32_t fRight = 0.f;
			f32_t fYaw = 0.f;
			const std::string strAnchorKind = Read_String(*pAnchor, "kind");
			if (!Has_ExactProperties(*pAnchor,
					{ "kind", "forwardOffsetM", "rightOffsetM",
					  "yawOffsetDegrees" }) ||
				("BOSS_CURRENT" != strAnchorKind &&
				 "STAGE_ORIGIN" != strAnchorKind) ||
				!Read_RequiredFiniteFloat(*pAnchor, "forwardOffsetM", fForward) ||
				!Read_RequiredFiniteFloat(*pAnchor, "rightOffsetM", fRight) ||
				!Read_RequiredFiniteFloat(*pAnchor, "yawOffsetDegrees", fYaw) ||
				std::fabs(fForward) > 1000.f || std::fabs(fRight) > 1000.f ||
				std::fabs(fYaw) > 360.f)
			{
				strOutError = "split gameplay hit anchor is invalid";
				return false;
			}
			OutStage.emplace("hitAnchor", *pAnchor);
		}
		if (nullptr != pActivation)
		{
			if (!Has_ExactProperties(*pActivation,
					{ "kind", "startMs", "lifetimeMs", "perTargetPolicy" }) ||
				"ACTIVE_WINDOW" != Read_String(*pActivation, "kind") ||
				"ONCE" != Read_String(*pActivation, "perTargetPolicy") ||
				!Is_NonNegativeInteger(pActivation->Find("startMs")) ||
				!Is_NonNegativeInteger(pActivation->Find("lifetimeMs")) ||
				0.0 == pActivation->Find("lifetimeMs")->Get_Number())
			{
				strOutError = "split gameplay hit activation is invalid";
				return false;
			}
			OutStage.emplace("hitCount", DATA_JSON_VALUE::Number(0));
			OutStage.emplace("hitIntervalMs", DATA_JSON_VALUE::Number(0));
			OutStage.emplace("hitDelayMs", DATA_JSON_VALUE::Number(0));
			OutStage.emplace("hitOffsetsMs", DATA_JSON_VALUE::Array({}));
			OutStage.emplace("hitActivation", *pActivation);
		}
		const std::string strSchedule = nullptr == pSchedule ? std::string{} :
			Read_String(*pSchedule, "kind");
		if (nullptr != pSchedule && "INTERVAL" == strSchedule && Has_ExactProperties(*pSchedule,
				{ "kind", "count", "firstOffsetMs", "intervalMs" }) &&
			Is_NonNegativeInteger(pSchedule->Find("count")) &&
			Is_NonNegativeInteger(pSchedule->Find("firstOffsetMs")) &&
			Is_NonNegativeInteger(pSchedule->Find("intervalMs")))
		{
			OutStage.emplace("hitCount", *pSchedule->Find("count"));
			OutStage.emplace("hitIntervalMs", *pSchedule->Find("intervalMs"));
			OutStage.emplace("hitDelayMs", *pSchedule->Find("firstOffsetMs"));
			OutStage.emplace("hitOffsetsMs", DATA_JSON_VALUE::Array({}));
		}
		else if (nullptr != pSchedule && "EXPLICIT_OFFSETS" == strSchedule &&
			Has_ExactProperties(*pSchedule, { "kind", "offsetsMs" }))
		{
			const DATA_JSON_VALUE* pOffsets = Required(
				*pSchedule, "offsetsMs", DATA_JSON_TYPE::ARRAY);
			if (nullptr == pOffsets || pOffsets->Get_Array().empty())
			{
				strOutError = "split gameplay explicit hit schedule is invalid";
				return false;
			}
			uint32_t iPrevious = 0u;
			for (size_t i = 0u; i < pOffsets->Get_Array().size(); ++i)
			{
				const DATA_JSON_VALUE& Offset = pOffsets->Get_Array()[i];
				if (!Is_NonNegativeInteger(&Offset) ||
					(0u != i && Offset.Get_Number() <= iPrevious))
				{
					strOutError = "split gameplay explicit hit offsets are invalid";
					return false;
				}
				iPrevious = static_cast<uint32_t>(Offset.Get_Number());
			}
			OutStage.emplace("hitCount", DATA_JSON_VALUE::Number(
				static_cast<double>(pOffsets->Get_Array().size())));
			OutStage.emplace("hitIntervalMs", DATA_JSON_VALUE::Number(0));
			OutStage.emplace("hitDelayMs", DATA_JSON_VALUE::Number(0));
			OutStage.emplace("hitOffsetsMs", *pOffsets);
		}
		else if (nullptr != pSchedule)
		{
			strOutError = "split gameplay hit schedule is invalid";
			return false;
		}

		const DATA_JSON_VALUE* pDamage = Required(
			Hit, "serverDamageProfileId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* pKnockdown = Required(
			Hit, "knockdown", DATA_JSON_TYPE::BOOLEAN);
		if (nullptr == pDamage || !Is_StableToken(pDamage->Get_String()) ||
			nullptr == pKnockdown || !Is_FiniteNumber(Hit.Find("pushRangeM")) ||
			!Is_NonNegativeInteger(Hit.Find("pushMs")) ||
			!Is_NonNegativeInteger(Hit.Find("downMs")))
		{
			strOutError = "split gameplay hit payload is invalid";
			return false;
		}
		if (bCaptureHit &&
			("CAPTURE" != Read_String(Hit, "playerResponse") ||
			 "BOSS_LEFT_HAND" != Read_String(Hit, "attachmentSlot") ||
			 0.0 != Hit.Find("pushRangeM")->Get_Number() ||
			 0.0 != Hit.Find("pushMs")->Get_Number() ||
			 pKnockdown->Get_Boolean() ||
			 0.0 != Hit.Find("downMs")->Get_Number()))
		{
			strOutError = "split gameplay capture hit contract is invalid";
			return false;
		}
		OutStage.emplace("serverDamageProfileId", *pDamage);
		if (bCaptureHit)
		{
			OutStage.emplace("playerResponse", *Hit.Find("playerResponse"));
			OutStage.emplace("attachmentSlot", *Hit.Find("attachmentSlot"));
		}
		OutStage.emplace("pushRangeM", *Hit.Find("pushRangeM"));
		OutStage.emplace("pushMs", *Hit.Find("pushMs"));
		OutStage.emplace("knockdown", *pKnockdown);
		OutStage.emplace("downMs", *Hit.Find("downMs"));
		return true;
	}

	bool_t Build_SplitDecisionProjection(
		const DATA_JSON_VALUE& Decision,
		DATA_JSON_VALUE& OutNormalSelection,
		MASTER_SCRIPTED_SEQUENCE_VIEW& OutScriptedSequence,
		std::vector<Client::VALTAN_SELECTION_SET_VIEW>& OutSelectionSets,
		std::vector<Client::VALTAN_SELECTION_WINDOW_VIEW>& OutSelectionWindows,
		std::vector<Client::VALTAN_MECHANIC_VIEW>& OutMechanicViews,
		std::map<std::string, SPLIT_MECHANIC, std::less<>>& OutMechanics,
		std::vector<Client::VALTAN_MANUAL_AUDITION_VIEW>& OutManualViews,
		std::map<std::string, Client::VALTAN_MANUAL_AUDITION_VIEW,
			std::less<>>& OutManualAuditions,
		std::string& strOutError)
	{
		if (!Has_ExactProperties(Decision,
				{ "scriptedSequence", "selectionSets", "selectionWindows",
				  "mechanics", "manualAuditions" }))
		{
			strOutError = "split gameplay decisionModel has unexpected properties";
			return false;
		}
		const DATA_JSON_VALUE* pScriptedSequence = Required(
			Decision, "scriptedSequence", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* pSets = Required(
			Decision, "selectionSets", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pWindows = Required(
			Decision, "selectionWindows", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pMechanics = Required(
			Decision, "mechanics", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pManualAuditions = Required(
			Decision, "manualAuditions", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pScriptedPatterns = nullptr == pScriptedSequence ?
			nullptr : Required(*pScriptedSequence,
				"patternIds", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pScriptedSequence ||
			!Has_ExactProperties(*pScriptedSequence,
				{ "sequenceId", "mode", "interStepPursuitMs", "patternIds" }) ||
			!Is_StableToken(Read_String(*pScriptedSequence, "sequenceId")) ||
			"ORDERED_ONCE_THEN_IDLE" !=
				Read_String(*pScriptedSequence, "mode") ||
			!Is_NonNegativeInteger(
				pScriptedSequence->Find("interStepPursuitMs")) ||
			pScriptedSequence->Find("interStepPursuitMs")->Get_Number() < 100.0 ||
			pScriptedSequence->Find("interStepPursuitMs")->Get_Number() > 10000.0 ||
			nullptr == pScriptedPatterns ||
			pScriptedPatterns->Get_Array().empty() ||
			pScriptedPatterns->Get_Array().size() >
				Client::CValtanPatternFlowDocument::MAX_SLOTS)
		{
			strOutError = "split gameplay scriptedSequence is invalid";
			return false;
		}
		MASTER_SCRIPTED_SEQUENCE_VIEW ScriptedSequence;
		ScriptedSequence.strSequenceId = Read_String(
			*pScriptedSequence, "sequenceId");
		ScriptedSequence.strMode = Read_String(*pScriptedSequence, "mode");
		ScriptedSequence.iInterStepPursuitMs = static_cast<uint32_t>(
			pScriptedSequence->Find("interStepPursuitMs")->Get_Number());
		for (const DATA_JSON_VALUE& PatternId : pScriptedPatterns->Get_Array())
		{
			if (DATA_JSON_TYPE::STRING != PatternId.Get_Type() ||
				!Is_StableToken(PatternId.Get_String()))
			{
				strOutError =
					"split gameplay scriptedSequence pattern is invalid";
				return false;
			}
			ScriptedSequence.PatternIds.push_back(PatternId.Get_String());
		}
		if (nullptr == pSets || pSets->Get_Array().empty() ||
			nullptr == pWindows || pWindows->Get_Array().empty() ||
			nullptr == pMechanics || nullptr == pManualAuditions)
		{
			strOutError = "split gameplay decisionModel inventories are invalid";
			return false;
		}

		std::map<std::string, std::vector<std::string>, std::less<>>
			PatternsBySet;
		for (const DATA_JSON_VALUE& Set : pSets->Get_Array())
		{
			if (!Has_ExactProperties(Set,
					{ "selectionSetId", "mode", "candidates" }) ||
				"WEIGHTED_POOL" != Read_String(Set, "mode"))
			{
				strOutError = "split gameplay selection set is invalid";
				return false;
			}
			const std::string strSetId = Read_String(Set, "selectionSetId");
			const DATA_JSON_VALUE* pCandidates = Required(
				Set, "candidates", DATA_JSON_TYPE::ARRAY);
			if (!Is_StableToken(strSetId) || nullptr == pCandidates ||
				pCandidates->Get_Array().empty() ||
				PatternsBySet.contains(strSetId))
			{
				strOutError = "split gameplay selection set identity is invalid or duplicated";
				return false;
			}
			std::vector<std::string> PatternIds;
			std::set<std::string, std::less<>> UniquePatterns;
			Client::VALTAN_SELECTION_SET_VIEW SetView;
			SetView.strSelectionSetId = strSetId;
			SetView.strMode = Read_String(Set, "mode");
			uint32_t iEnabledCount = 0u;
			for (const DATA_JSON_VALUE& Candidate : pCandidates->Get_Array())
			{
				const DATA_JSON_VALUE* pEnabled = Required(
					Candidate, "enabled", DATA_JSON_TYPE::BOOLEAN);
				const std::string strPatternId = Read_String(
					Candidate, "patternId");
				if (!Has_ExactProperties(Candidate,
						{ "patternId", "weight", "enabled" }) ||
					!Is_StableToken(strPatternId) ||
					!Is_NonNegativeInteger(Candidate.Find("weight")) ||
					0.0 == Candidate.Find("weight")->Get_Number() ||
					Candidate.Find("weight")->Get_Number() > 100000.0 ||
					nullptr == pEnabled ||
					!UniquePatterns.insert(strPatternId).second)
				{
					strOutError = "split gameplay selection candidate is invalid or duplicated";
					return false;
				}
				const uint32_t iWeight = static_cast<uint32_t>(
					Candidate.Find("weight")->Get_Number());
				Client::VALTAN_SELECTION_CANDIDATE_VIEW CandidateView;
				CandidateView.strPatternId = strPatternId;
				CandidateView.iWeight = iWeight;
				CandidateView.bEnabled = pEnabled->Get_Boolean();
				if (CandidateView.bEnabled)
					++iEnabledCount;
				SetView.Candidates.push_back(std::move(CandidateView));
				PatternIds.push_back(strPatternId);
			}
			if (0u == iEnabledCount)
			{
				strOutError = "split gameplay weighted selection set has no enabled candidate";
				return false;
			}
			PatternsBySet.emplace(strSetId, std::move(PatternIds));
			OutSelectionSets.push_back(std::move(SetView));
		}

		DATA_JSON_VALUE::ARRAY Ranges;
		std::vector<std::string> NormalPatternIds;
		std::set<std::string, std::less<>> NormalPatternSet;
		std::set<std::string, std::less<>> WindowIds;
		std::set<std::string, std::less<>> RotationIds;
		std::set<std::string, std::less<>> WindowSetIds;
		std::map<uint32_t, uint32_t> PreviousMinimumByPhase;
		for (const DATA_JSON_VALUE& Window : pWindows->Get_Array())
		{
			if (!Has_ExactProperties(Window,
					{ "windowId", "gameplayPhase", "maximumHealthBarInclusive",
					  "minimumHealthBarExclusive", "selectionSetId",
					  "compatibilityRotationId" }) ||
				!Is_StableToken(Read_String(Window, "windowId")) ||
				!Is_StableToken(Read_String(Window, "compatibilityRotationId")) ||
				!Is_NonNegativeInteger(Window.Find("gameplayPhase")) ||
				0.0 == Window.Find("gameplayPhase")->Get_Number() ||
				Window.Find("gameplayPhase")->Get_Number() > 3.0 ||
				!Is_NonNegativeInteger(
					Window.Find("maximumHealthBarInclusive")) ||
				!Is_NonNegativeInteger(
					Window.Find("minimumHealthBarExclusive")))
			{
				strOutError = "split gameplay selection window is invalid";
				return false;
			}
			const std::string strWindowId = Read_String(Window, "windowId");
			const std::string strRotationId = Read_String(
				Window, "compatibilityRotationId");
			const std::string strSetId = Read_String(Window, "selectionSetId");
			const auto Patterns = PatternsBySet.find(strSetId);
			const uint32_t iPhase = static_cast<uint32_t>(
				Window.Find("gameplayPhase")->Get_Number());
			const uint32_t iMaximum = static_cast<uint32_t>(
				Window.Find("maximumHealthBarInclusive")->Get_Number());
			const uint32_t iMinimum = static_cast<uint32_t>(
				Window.Find("minimumHealthBarExclusive")->Get_Number());
			const auto Previous = PreviousMinimumByPhase.find(iPhase);
			if (Patterns == PatternsBySet.end() || iMaximum <= iMinimum ||
				(Previous != PreviousMinimumByPhase.end() &&
				 Previous->second != iMaximum) ||
				!WindowIds.insert(strWindowId).second ||
				!RotationIds.insert(strRotationId).second ||
				!WindowSetIds.insert(strSetId).second)
			{
				strOutError = "split gameplay selection window order/closure is invalid";
				return false;
			}
			PreviousMinimumByPhase[iPhase] = iMinimum;
			for (const std::string& strPatternId : Patterns->second)
			{
				if (NormalPatternSet.insert(strPatternId).second)
					NormalPatternIds.push_back(strPatternId);
			}
			Client::VALTAN_SELECTION_WINDOW_VIEW WindowView;
			WindowView.strWindowId = strWindowId;
			WindowView.iGameplayPhase = iPhase;
			WindowView.iMaximumHealthBarInclusive = iMaximum;
			WindowView.iMinimumHealthBarExclusive = iMinimum;
			WindowView.strSelectionSetId = strSetId;
			WindowView.strCompatibilityRotationId = strRotationId;
			OutSelectionWindows.push_back(std::move(WindowView));
			DATA_JSON_VALUE::OBJECT Range;
			Range.emplace("rotationId", DATA_JSON_VALUE::String(
				strRotationId));
			Range.emplace("fromHealthBar",
				*Window.Find("maximumHealthBarInclusive"));
			Range.emplace("toHealthBar",
				*Window.Find("minimumHealthBarExclusive"));
			Ranges.push_back(DATA_JSON_VALUE::Object(std::move(Range)));
		}
		if (WindowSetIds.size() != PatternsBySet.size())
		{
			strOutError = "split gameplay selection windows do not cover every set";
			return false;
		}

		std::set<std::string, std::less<>> MechanicIds;
		std::set<std::pair<uint32_t, uint32_t>> MechanicTriggers;
		uint32_t iPreviousHealthBar = (std::numeric_limits<uint32_t>::max)();
		uint32_t iPreviousOrder = 0u;
		for (const DATA_JSON_VALUE& Mechanic : pMechanics->Get_Array())
		{
			if (!Has_ExactProperties(Mechanic,
					{ "mechanicId", "patternId", "trigger", "triggerOrder",
					  "oncePerEncounter", "failurePolicy" }) ||
				!Is_StableToken(Read_String(Mechanic, "mechanicId")) ||
				!Is_StableToken(Read_String(Mechanic, "patternId")) ||
				!Is_NonNegativeInteger(Mechanic.Find("triggerOrder")) ||
				nullptr == Required(
					Mechanic, "oncePerEncounter", DATA_JSON_TYPE::BOOLEAN) ||
				!Required(Mechanic, "oncePerEncounter",
					DATA_JSON_TYPE::BOOLEAN)->Get_Boolean() ||
				"ABORT_ENCOUNTER_REQUIRE_RESET" !=
					Read_String(Mechanic, "failurePolicy"))
			{
				strOutError = "split gameplay mechanic is invalid";
				return false;
			}
			const DATA_JSON_VALUE* pTrigger = Required(
				Mechanic, "trigger", DATA_JSON_TYPE::OBJECT);
			if (nullptr == pTrigger || !Has_ExactProperties(
					*pTrigger, { "kind", "healthBar" }) ||
				"HEALTH_BAR_CROSSING" != Read_String(*pTrigger, "kind") ||
				!Is_NonNegativeInteger(pTrigger->Find("healthBar")) ||
				0.0 == pTrigger->Find("healthBar")->Get_Number() ||
				0.0 == Mechanic.Find("triggerOrder")->Get_Number())
			{
				strOutError = "split gameplay mechanic trigger is invalid";
				return false;
			}
			SPLIT_MECHANIC Projection;
			Projection.iHealthBar = static_cast<uint32_t>(
				pTrigger->Find("healthBar")->Get_Number());
			Projection.iTriggerOrder = static_cast<uint32_t>(
				Mechanic.Find("triggerOrder")->Get_Number());
			const std::string strMechanicId = Read_String(
				Mechanic, "mechanicId");
			const std::string strPatternId = Read_String(
				Mechanic, "patternId");
			if (!MechanicIds.insert(strMechanicId).second ||
				!MechanicTriggers.insert({ Projection.iHealthBar,
					Projection.iTriggerOrder }).second ||
				Projection.iHealthBar > iPreviousHealthBar ||
				(Projection.iHealthBar == iPreviousHealthBar &&
				 Projection.iTriggerOrder <= iPreviousOrder) ||
				!OutMechanics.emplace(strPatternId, Projection).second)
			{
				strOutError = "split gameplay mechanic identity/order is invalid";
				return false;
			}
			iPreviousHealthBar = Projection.iHealthBar;
			iPreviousOrder = Projection.iTriggerOrder;
			Client::VALTAN_MECHANIC_VIEW MechanicView;
			MechanicView.strMechanicId = strMechanicId;
			MechanicView.strPatternId = strPatternId;
			MechanicView.strTriggerKind = Read_String(*pTrigger, "kind");
			MechanicView.iHealthBar = Projection.iHealthBar;
			MechanicView.iTriggerOrder = Projection.iTriggerOrder;
			MechanicView.bOncePerEncounter = true;
			MechanicView.strFailurePolicy = Read_String(
				Mechanic, "failurePolicy");
			OutMechanicViews.push_back(std::move(MechanicView));
		}

		std::set<std::string, std::less<>> ManualSourceChainIds;
		for (const DATA_JSON_VALUE& Manual : pManualAuditions->Get_Array())
		{
			if (!Has_ExactProperties(Manual,
					{ "patternId", "sourceChainId", "authoringPhase",
					  "admissionState" }) ||
				!Is_StableToken(Read_String(Manual, "patternId")) ||
				!Is_StableToken(Read_String(Manual, "sourceChainId")) ||
				!Is_NonNegativeInteger(Manual.Find("authoringPhase")) ||
				0.0 == Manual.Find("authoringPhase")->Get_Number() ||
				Manual.Find("authoringPhase")->Get_Number() > 3.0 ||
				("MANUAL_SERVER_AUDITION" != Read_String(Manual, "admissionState") &&
				 "DERIVED_SERVER_PATTERN" != Read_String(Manual, "admissionState")))
			{
				strOutError = "split gameplay manual audition is invalid";
				return false;
			}
			Client::VALTAN_MANUAL_AUDITION_VIEW View;
			View.strPatternId = Read_String(Manual, "patternId");
			View.strSourceChainId = Read_String(Manual, "sourceChainId");
			View.iAuthoringPhase = static_cast<uint32_t>(
				Manual.Find("authoringPhase")->Get_Number());
			View.strAdmissionState = Read_String(Manual, "admissionState");
			if (NormalPatternSet.contains(View.strPatternId) ||
				OutMechanics.contains(View.strPatternId) ||
				!ManualSourceChainIds.insert(View.strSourceChainId).second ||
				!OutManualAuditions.emplace(
					View.strPatternId, View).second)
			{
				strOutError =
					"split gameplay manual audition ownership is duplicated or overlaps";
				return false;
			}
			OutManualViews.push_back(std::move(View));
		}
		const auto ArenaBreak = OutMechanics.find("VALTAN_ARENA_BREAK_109");
		uint32_t iPhaseOneBoundary = 0u;
		for (const Client::VALTAN_SELECTION_WINDOW_VIEW& Window :
			OutSelectionWindows)
		{
			if (1u == Window.iGameplayPhase)
				iPhaseOneBoundary = Window.iMinimumHealthBarExclusive;
		}
		if (0u == iPhaseOneBoundary || ArenaBreak == OutMechanics.end() ||
			ArenaBreak->second.iHealthBar != iPhaseOneBoundary)
		{
			strOutError =
				"VALTAN_ARENA_BREAK_109 health bar must match the final phase-1 window boundary";
			return false;
		}

		DATA_JSON_VALUE::OBJECT Normal;
		Normal.emplace("selectionMode",
			DATA_JSON_VALUE::String("WEIGHTED_POOL"));
		Normal.emplace("ranges", DATA_JSON_VALUE::Array(std::move(Ranges)));
		Normal.emplace("patternIds", Build_StringArray(NormalPatternIds));
		OutNormalSelection = DATA_JSON_VALUE::Object(std::move(Normal));
		OutScriptedSequence = std::move(ScriptedSequence);
		return true;
	}

	bool_t Validate_SplitCameraInvocation(
		const DATA_JSON_VALUE& Invocation,
		const uint32_t iStageDurationMs,
		std::string& strOutCameraCueId,
		std::string& strOutError)
	{
		if (!Has_ExactProperties(Invocation,
				{ "cameraInvocationId", "cameraCueId", "trigger",
				  "startOffsetMs", "durationPolicy", "durationMs" }) ||
			!Is_StableToken(Read_String(
				Invocation, "cameraInvocationId")) ||
			!Is_StableToken(Read_String(Invocation, "cameraCueId")) ||
			"ENTER" != Read_String(Invocation, "trigger") ||
			"EXPLICIT" != Read_String(Invocation, "durationPolicy") ||
			!Is_NonNegativeInteger(Invocation.Find("startOffsetMs")) ||
			!Is_NonNegativeInteger(Invocation.Find("durationMs")))
		{
			strOutError = "split presentation camera invocation is invalid";
			return false;
		}
		const uint64_t iEnd = static_cast<uint64_t>(
			Invocation.Find("startOffsetMs")->Get_Number()) +
			static_cast<uint64_t>(Invocation.Find("durationMs")->Get_Number());
		if (iEnd > iStageDurationMs)
		{
			strOutError = "split presentation camera invocation leaves its Server stage";
			return false;
		}
		strOutCameraCueId = Read_String(Invocation, "cameraCueId");
		return true;
	}

	bool_t Build_SplitEventProjection(
		const DATA_JSON_VALUE& Event,
		const uint32_t iStageDurationMs,
		DATA_JSON_VALUE* pOutAction,
		std::string& strOutSpawnArchetypeId,
		bool_t& bOutWorldEvent,
		std::string& strOutError)
	{
		*pOutAction = DATA_JSON_VALUE::Null();
		strOutSpawnArchetypeId.clear();
		bOutWorldEvent = false;
		const std::string strEventId = Read_String(Event, "eventId");
		const std::string strTrigger = Read_String(Event, "trigger");
		const std::string strKind = Read_String(Event, "kind");
		if (!Is_StableToken(strEventId) || !Is_StableToken(strTrigger) ||
			!Is_StableToken(strKind))
		{
			strOutError = "split gameplay event identity is invalid";
			return false;
		}
		DATA_JSON_VALUE::OBJECT Action;
		if ("SET_BOSS_FLAG" == strKind)
		{
			const DATA_JSON_VALUE* pEnabled = Required(
				Event, "enabled", DATA_JSON_TYPE::BOOLEAN);
			if (!Has_ExactProperties(Event,
					{ "eventId", "trigger", "kind", "flagId", "enabled" }) ||
				!Is_StableToken(Read_String(Event, "flagId")) ||
				nullptr == pEnabled)
			{
				strOutError = "split gameplay SET_BOSS_FLAG event is invalid";
				return false;
			}
			Action.emplace("targetId", DATA_JSON_VALUE::String(
				Read_String(Event, "flagId")));
			Action.emplace("value", DATA_JSON_VALUE::Number(
				pEnabled->Get_Boolean() ? 1.0 : 0.0));
		}
		else if ("SET_STAGGER_GAUGE" == strKind)
		{
			const DATA_JSON_VALUE* pValue = Event.Find("value");
			if (!Has_ExactProperties(Event,
					{ "eventId", "trigger", "kind", "value" }) ||
				("ENTER" != strTrigger && "EXIT" != strTrigger) ||
				!Is_NonNegativeInteger(pValue) ||
				pValue->Get_Number() > 100000.0 ||
				(("ENTER" == strTrigger) !=
				 (pValue->Get_Number() > 0.0)))
			{
				strOutError = "split gameplay SET_STAGGER_GAUGE event is invalid";
				return false;
			}
			Action.emplace("targetId",
				DATA_JSON_VALUE::String("boss.gauge.stagger"));
			Action.emplace("value", *pValue);
		}
		else if ("SET_PLAYER_BIND" == strKind)
		{
			const DATA_JSON_VALUE* pHeightM = Event.Find("heightM");
			const DATA_JSON_VALUE* pDurationMs = Event.Find("durationMs");
			if (!Has_ExactProperties(Event,
					{ "eventId", "trigger", "kind", "heightM", "durationMs" }) ||
				("ENTER" != strTrigger && "EXIT" != strTrigger) ||
				!Is_FiniteNumber(pHeightM) ||
				!Is_NonNegativeInteger(pDurationMs) ||
				(("ENTER" == strTrigger &&
				  (10.0 != pHeightM->Get_Number() ||
				   pDurationMs->Get_Number() != iStageDurationMs ||
				   pDurationMs->Get_Number() < 100.0 ||
				   pDurationMs->Get_Number() > 120000.0)) ||
				 ("EXIT" == strTrigger &&
				  (0.0 != pHeightM->Get_Number() ||
				   0.0 != pDurationMs->Get_Number()))))
			{
				strOutError = "split gameplay SET_PLAYER_BIND event is invalid";
				return false;
			}
			Action.emplace("targetId",
				DATA_JSON_VALUE::String("player.status.bind"));
			Action.emplace("value", DATA_JSON_VALUE::Number(
				pHeightM->Get_Number() * 1000.0));
			Action.emplace("durationMs", *pDurationMs);
		}
		else if ("SET_PLAYER_SILENCE" == strKind)
		{
			const DATA_JSON_VALUE* pDurationMs = Event.Find("durationMs");
			if (!Has_ExactProperties(Event,
					{ "eventId", "trigger", "kind", "durationMs" }) ||
				("ENTER" != strTrigger && "EXIT" != strTrigger) ||
				!Is_NonNegativeInteger(pDurationMs) ||
				(("ENTER" == strTrigger &&
				  (pDurationMs->Get_Number() != iStageDurationMs ||
				   pDurationMs->Get_Number() < 100.0 ||
				   pDurationMs->Get_Number() > 120000.0)) ||
				 ("EXIT" == strTrigger && 0.0 != pDurationMs->Get_Number())))
			{
				strOutError = "split gameplay SET_PLAYER_SILENCE event is invalid";
				return false;
			}
			Action.emplace("targetId",
				DATA_JSON_VALUE::String("player.status.silence"));
			Action.emplace("value", DATA_JSON_VALUE::Number(
				"ENTER" == strTrigger ? 1.0 : 0.0));
			Action.emplace("durationMs", *pDurationMs);
		}
		else if ("SET_GAMEPLAY_PHASE" == strKind)
		{
			if (!Has_ExactProperties(Event,
					{ "eventId", "trigger", "kind", "gameplayPhase" }) ||
				!Is_NonNegativeInteger(Event.Find("gameplayPhase")))
			{
				strOutError = "split gameplay SET_GAMEPLAY_PHASE event is invalid";
				return false;
			}
			Action.emplace("targetId",
				DATA_JSON_VALUE::String("boss.phase.gameplay"));
			Action.emplace("value", *Event.Find("gameplayPhase"));
		}
		else if ("TRIGGER_WORLD_EVENT_SET" == strKind)
		{
			if (!Has_ExactProperties(Event,
					{ "eventId", "trigger", "kind", "worldEventSetId" }) ||
				!Is_StableToken(Read_String(Event, "worldEventSetId")))
			{
				strOutError = "split gameplay world event is invalid";
				return false;
			}
			bOutWorldEvent = true;
			return true;
		}
		else if ("RETARGET_RANDOM_ALIVE" == strKind ||
			"RETURN_TO_ARENA_CENTER" == strKind)
		{
			if (!Has_ExactProperties(Event,
					{ "eventId", "trigger", "kind" }) ||
				"ENTER" != strTrigger)
			{
				strOutError = "split gameplay retarget/arena return event is invalid";
				return false;
			}
			Action.emplace("targetId", DATA_JSON_VALUE::String(
				"RETURN_TO_ARENA_CENTER" == strKind ?
					"boss.arena.center" : "boss.target.pattern"));
			Action.emplace("value", DATA_JSON_VALUE::Number(1));
		}
		else if ("DAMAGE_GRABBED_PLAYERS" == strKind ||
			"EXECUTE_GRABBED_PLAYERS" == strKind)
		{
			const bool_t bDamage = "DAMAGE_GRABBED_PLAYERS" == strKind;
			const std::string strDamageProfileId = Read_String(Event, "damageProfileId");
			const bool_t bPropertiesValid = bDamage ?
				Has_ExactProperties(Event, { "eventId", "trigger", "kind", "damageProfileId" }) :
				Has_ExactProperties(Event, { "eventId", "trigger", "kind" });
			if (!bPropertiesValid || "ENTER" != strTrigger ||
				(bDamage && (!Is_StableToken(strDamageProfileId) ||
					!strDamageProfileId.starts_with("damage."))))
			{
				strOutError = "split gameplay grabbed-player terminal event is invalid";
				return false;
			}
			Action.emplace("targetId", DATA_JSON_VALUE::String(
				bDamage ? strDamageProfileId : "boss.attachment.left-hand"));
			Action.emplace("value", DATA_JSON_VALUE::Number(0));
		}
		else if ("RELEASE_GRABBED_PLAYERS" == strKind)
		{
			const std::string strReleaseMode = Read_String(Event, "releaseMode");
			const DATA_JSON_VALUE* pSpeedMps = Event.Find("speedMps");
			const DATA_JSON_VALUE* pDurationMs = Event.Find("durationMs");
			const DATA_JSON_VALUE* pYawOffsetDegrees =
				Event.Find("yawOffsetDegrees");
			if (!Has_ExactProperties(Event,
					{ "eventId", "trigger", "kind", "releaseMode",
					  "speedMps", "durationMs", "yawOffsetDegrees" }) ||
				("ENTER" != strTrigger && "EXIT" != strTrigger) ||
				!Is_FiniteNumber(pSpeedMps) ||
				!Is_NonNegativeInteger(pDurationMs) ||
				!Is_FiniteNumber(pYawOffsetDegrees) ||
				pSpeedMps->Get_Number() < 0.0 ||
				pSpeedMps->Get_Number() > 50.0 ||
				pDurationMs->Get_Number() > 5000.0 ||
				std::abs(pYawOffsetDegrees->Get_Number()) > 180.0)
			{
				strOutError =
					"split gameplay grabbed-player release event is invalid";
				return false;
			}
			const bool_t bHold = "HOLD" == strReleaseMode &&
				0.0 == pSpeedMps->Get_Number() &&
				0.0 == pDurationMs->Get_Number() &&
				0.0 == pYawOffsetDegrees->Get_Number();
			const bool_t bOppositeKnockback =
				("OPPOSITE_KNOCKBACK" == strReleaseMode ||
				 "ARENA_EJECTION" == strReleaseMode) &&
				pSpeedMps->Get_Number() > 0.0 &&
				pDurationMs->Get_Number() > 0.0 &&
				("ARENA_EJECTION" == strReleaseMode ||
				 0.0 == pYawOffsetDegrees->Get_Number());
			if (!bHold && !bOppositeKnockback)
			{
				strOutError =
					"split gameplay grabbed-player release policy is invalid";
				return false;
			}
			Action.emplace("targetId",
				DATA_JSON_VALUE::String("boss.attachment.left-hand"));
			Action.emplace("releaseMode",
				DATA_JSON_VALUE::String(strReleaseMode));
			Action.emplace("speedMps", *pSpeedMps);
			Action.emplace("durationMs", *pDurationMs);
			Action.emplace("yawOffsetDegrees", *pYawOffsetDegrees);
		}
		else if ("SPAWN_COMBAT_OBJECT_VOLLEY" == strKind)
		{
			const DATA_JSON_VALUE* pLayout = Required(
				Event, "layout", DATA_JSON_TYPE::OBJECT);
			const DATA_JSON_VALUE* pSpawnSchedule = Required(
				Event, "spawnSchedule", DATA_JSON_TYPE::OBJECT);
			const DATA_JSON_VALUE* pArenaRandom = Required(
				Event, "arenaRandom", DATA_JSON_TYPE::OBJECT);
			const DATA_JSON_VALUE* pAllowOverlap = Required(
				Event, "allowOverlap", DATA_JSON_TYPE::BOOLEAN);
			const std::string strVolleyPolicy =
				Read_String(Event, "volleyPolicy");
			const bool_t bPerAlivePlayer =
				"PER_ALIVE_PLAYER" == strVolleyPolicy;
			const bool_t bBossRelative = "BOSS_RELATIVE" == strVolleyPolicy;
			if (!Has_ExactProperties(Event,
					{ "eventId", "trigger", "kind", "combatObjectArchetypeId",
					  "volleyPolicy", "countPerResolvedTarget", "layout",
					  "allowOverlap", "maximumTotalObjects", "spawnSchedule",
					  "arenaRandom" }) ||
				"ENTER" != strTrigger ||
				!Is_StableToken(Read_String(
					Event, "combatObjectArchetypeId")) ||
				(!bPerAlivePlayer && !bBossRelative) ||
				!Is_NonNegativeInteger(Event.Find("countPerResolvedTarget")) ||
				0.0 == Event.Find("countPerResolvedTarget")->Get_Number() ||
				Event.Find("countPerResolvedTarget")->Get_Number() > 8.0 ||
				!Is_NonNegativeInteger(Event.Find("maximumTotalObjects")) ||
				0.0 == Event.Find("maximumTotalObjects")->Get_Number() ||
				Event.Find("maximumTotalObjects")->Get_Number() > 64.0 ||
				nullptr == pAllowOverlap || pAllowOverlap->Get_Boolean() ||
				nullptr == pLayout || nullptr == pSpawnSchedule ||
				nullptr == pArenaRandom)
			{
				strOutError = "split gameplay combat-object volley is invalid";
				return false;
			}
			const std::string strLayoutKind = Read_String(*pLayout, "kind");
			const bool_t bTargetCenter = "TARGET_CENTER" == strLayoutKind &&
				Has_ExactProperties(*pLayout, { "kind" });
			const bool_t bRadial = "RADIAL_AROUND_TARGET" == strLayoutKind &&
				Has_ExactProperties(*pLayout,
					{ "kind", "radiusM", "startAngleDegrees",
					  "angleStepDegrees" }) &&
				Is_FiniteNumber(pLayout->Find("radiusM")) &&
				Is_FiniteNumber(pLayout->Find("startAngleDegrees")) &&
				Is_FiniteNumber(pLayout->Find("angleStepDegrees"));
			const bool_t bBossRadial = "RADIAL_AROUND_BOSS" == strLayoutKind &&
				Has_ExactProperties(*pLayout,
					{ "kind", "radiusM", "startAngleDegrees",
					  "angleStepDegrees", "mappingBasis" }) &&
				"PROJECT_TUNED" == Read_String(*pLayout, "mappingBasis") &&
				Is_FiniteNumber(pLayout->Find("radiusM")) &&
				Is_FiniteNumber(pLayout->Find("startAngleDegrees")) &&
				Is_FiniteNumber(pLayout->Find("angleStepDegrees"));
			const double fCount =
				Event.Find("countPerResolvedTarget")->Get_Number();
			if ((bPerAlivePlayer && !bTargetCenter && !bRadial) ||
				(bBossRelative && !bBossRadial))
			{
				strOutError = "split gameplay combat-object volley layout is invalid";
				return false;
			}
			if ((bPerAlivePlayer && bTargetCenter && 1.0 != fCount) ||
				((bRadial || bBossRadial) && (fCount < 2.0 ||
					pLayout->Find("radiusM")->Get_Number() <= 0.0 ||
					pLayout->Find("angleStepDegrees")->Get_Number() <= 0.0 ||
					pLayout->Find("angleStepDegrees")->Get_Number() * fCount >
						360.000001)))
			{
				strOutError =
					"split gameplay combat-object volley layout/count is invalid";
				return false;
			}
			if (!Has_ExactProperties(*pSpawnSchedule,
					{ "kind", "count", "firstOffsetMs", "intervalMs" }) ||
				"INTERVAL" != Read_String(*pSpawnSchedule, "kind") ||
				!Is_NonNegativeInteger(pSpawnSchedule->Find("count")) ||
				0.0 == pSpawnSchedule->Find("count")->Get_Number() ||
				pSpawnSchedule->Find("count")->Get_Number() > 8.0 ||
				!Is_NonNegativeInteger(pSpawnSchedule->Find("firstOffsetMs")) ||
				0.0 != pSpawnSchedule->Find("firstOffsetMs")->Get_Number() ||
				!Is_NonNegativeInteger(pSpawnSchedule->Find("intervalMs")) ||
				(pSpawnSchedule->Find("count")->Get_Number() > 1.0 &&
				 0.0 == pSpawnSchedule->Find("intervalMs")->Get_Number()) ||
				(pSpawnSchedule->Find("count")->Get_Number() == 1.0 &&
				 0.0 != pSpawnSchedule->Find("intervalMs")->Get_Number()))
			{
				strOutError =
					"split gameplay combat-object volley spawn schedule is invalid";
				return false;
			}
			const double fArenaRandomCount =
				pArenaRandom->Is_Object() && nullptr != pArenaRandom->Find("count") &&
				Is_NonNegativeInteger(pArenaRandom->Find("count")) ?
				pArenaRandom->Find("count")->Get_Number() : 0.0;
			const bool_t bPerAliveArenaContract = bPerAlivePlayer &&
				Has_ExactProperties(*pArenaRandom,
					{ "kind", "anchor", "count", "radiusM",
					  "heightToleranceM" }) &&
				"RANDOM_NAVIGABLE_CIRCLE" ==
					Read_String(*pArenaRandom, "kind") &&
				"BOSS_SPAWN_POSITION" == Read_String(*pArenaRandom, "anchor") &&
				Is_NonNegativeInteger(pArenaRandom->Find("count")) &&
				Is_FiniteNumber(pArenaRandom->Find("radiusM")) &&
				Is_FiniteNumber(pArenaRandom->Find("heightToleranceM")) &&
				fArenaRandomCount > 0.0 &&
				pArenaRandom->Find("radiusM")->Get_Number() > 0.0 &&
				pArenaRandom->Find("heightToleranceM")->Get_Number() > 0.0 &&
				Event.Find("maximumTotalObjects")->Get_Number() >=
					Event.Find("countPerResolvedTarget")->Get_Number() +
						fArenaRandomCount;
			const bool_t bBossRelativeContract = bBossRelative &&
				Has_ExactProperties(*pArenaRandom, { "kind" }) &&
				"NONE" == Read_String(*pArenaRandom, "kind") &&
				1.0 == pSpawnSchedule->Find("count")->Get_Number() &&
				0.0 == pSpawnSchedule->Find("intervalMs")->Get_Number() &&
				Event.Find("maximumTotalObjects")->Get_Number() >= fCount;
			if (!bPerAliveArenaContract && !bBossRelativeContract)
			{
				strOutError =
					"split gameplay combat-object volley arena random contract is invalid";
				return false;
			}
			/* The repository-facing Product projection uses one flat, typed volley
			   row. Preserve every radial field here so Parse_MasterDocument and the
			   local Arena Clone join see the same contract as the Server bootstrap. */
			Action.emplace("targetId", DATA_JSON_VALUE::String(
				Read_String(Event, "combatObjectArchetypeId")));
			Action.emplace("targetingPolicy",
				DATA_JSON_VALUE::String(strVolleyPolicy));
			Action.emplace("countPerResolvedTarget",
				*Event.Find("countPerResolvedTarget"));
			Action.emplace("layout", DATA_JSON_VALUE::String(
				bTargetCenter ? "SINGLE" : "RADIAL"));
			Action.emplace("radiusM", DATA_JSON_VALUE::Number(
				bTargetCenter ? 0.0 : Read_Number(*pLayout, "radiusM")));
			Action.emplace("startAngleDegrees", DATA_JSON_VALUE::Number(
				bTargetCenter ? 0.0 :
					Read_Number(*pLayout, "startAngleDegrees")));
			Action.emplace("angleStepDegrees", DATA_JSON_VALUE::Number(
				bTargetCenter ? 0.0 :
					Read_Number(*pLayout, "angleStepDegrees")));
			Action.emplace("allowOverlap", *pAllowOverlap);
			Action.emplace("maximumTotalObjects",
				*Event.Find("maximumTotalObjects"));
			Action.emplace("spawnCount", *pSpawnSchedule->Find("count"));
			Action.emplace("spawnIntervalMs",
				*pSpawnSchedule->Find("intervalMs"));
			Action.emplace("arenaRandomCount", DATA_JSON_VALUE::Number(
				bPerAlivePlayer ? Read_Number(*pArenaRandom, "count") : 0.0));
			Action.emplace("arenaRandomRadiusM", DATA_JSON_VALUE::Number(
				bPerAlivePlayer ? Read_Number(*pArenaRandom, "radiusM") : 0.0));
			Action.emplace("arenaHeightToleranceM", DATA_JSON_VALUE::Number(
				bPerAlivePlayer ?
					Read_Number(*pArenaRandom, "heightToleranceM") : 0.0));
			Action.emplace("arenaAnchorPolicy", DATA_JSON_VALUE::String(
				bPerAlivePlayer ? Read_String(*pArenaRandom, "anchor") : "NONE"));
			strOutSpawnArchetypeId = Read_String(
				Event, "combatObjectArchetypeId");
			return true;
		}
		else if ("SPAWN_COMBAT_OBJECT" == strKind)
		{
			const std::string strCombatObjectArchetypeId = Read_String(
				Event, "combatObjectArchetypeId");
			if (!Has_ExactProperties(Event,
					{ "eventId", "trigger", "kind",
					  "combatObjectArchetypeId", "count" }) ||
				"ENTER" != strTrigger ||
				!Is_StableToken(strCombatObjectArchetypeId) ||
				!Is_NonNegativeInteger(Event.Find("count")) ||
				1.0 != Event.Find("count")->Get_Number())
			{
				strOutError = "split gameplay combat-object spawn is invalid";
				return false;
			}
			strOutSpawnArchetypeId = strCombatObjectArchetypeId;
			Action.emplace("targetId",
				DATA_JSON_VALUE::String(strCombatObjectArchetypeId));
			Action.emplace("value", DATA_JSON_VALUE::Number(1));
		}
		else
		{
			strOutError = "split gameplay event kind is unsupported: " + strKind;
			return false;
		}
		Action.emplace("trigger", DATA_JSON_VALUE::String(strTrigger));
		Action.emplace("kind", DATA_JSON_VALUE::String(strKind));
		Action.emplace("durationMs", DATA_JSON_VALUE::Number(0));
		*pOutAction = DATA_JSON_VALUE::Object(std::move(Action));
		return true;
	}

	bool_t Parse_SplitMasterDocument(
		const DATA_JSON_VALUE& Gameplay,
		const DATA_JSON_VALUE& Presentation,
		MASTER_DOCUMENT& Out,
		std::string& strOutError)
	{
		if (!Validate_SplitRootIdentity(Gameplay, Presentation, strOutError))
			return false;

		const DATA_JSON_VALUE* pPreviewPaths = Required(
			Gameplay, "previewPaths", DATA_JSON_TYPE::OBJECT);
		const std::initializer_list<std::string_view> PreviewFields = {
			"encounter", "animationBindings", "effectCues", "combatObjects",
			"bossCatalog", "effectCatalog", "damageProfiles", "cinematicCamera",
			"worldEvents", "patternRotations", "sourceClipSequences",
			"combatObjectAuthoring", "worldEventSets", "legacyCompatibility" };
		if (nullptr == pPreviewPaths ||
			!Has_ExactProperties(*pPreviewPaths, PreviewFields))
		{
			strOutError = "split gameplay previewPaths contract is invalid";
			return false;
		}
		for (const auto& [Name, Value] : pPreviewPaths->Get_Object())
		{
			(void)Name;
			if (!Value.Is_String() || !Value.Get_String().starts_with("Data/"))
			{
				strOutError = "split gameplay preview path is invalid";
				return false;
			}
		}

		DATA_JSON_VALUE NormalSelection;
		MASTER_SCRIPTED_SEQUENCE_VIEW ScriptedSequence;
		std::vector<Client::VALTAN_SELECTION_SET_VIEW> SelectionSets;
		std::vector<Client::VALTAN_SELECTION_WINDOW_VIEW> SelectionWindows;
		std::vector<Client::VALTAN_MECHANIC_VIEW> MechanicViews;
		std::map<std::string, SPLIT_MECHANIC, std::less<>> Mechanics;
		std::vector<Client::VALTAN_MANUAL_AUDITION_VIEW> ManualViews;
		std::map<std::string, Client::VALTAN_MANUAL_AUDITION_VIEW,
			std::less<>> ManualAuditions;
		const DATA_JSON_VALUE* pDecision = Required(
			Gameplay, "decisionModel", DATA_JSON_TYPE::OBJECT);
		if (nullptr == pDecision || !Build_SplitDecisionProjection(
				*pDecision, NormalSelection, ScriptedSequence, SelectionSets,
				SelectionWindows,
				MechanicViews, Mechanics, ManualViews, ManualAuditions,
				strOutError))
		{
			return false;
		}
		std::set<std::string, std::less<>> CandidatePatterns;
		for (const Client::VALTAN_SELECTION_SET_VIEW& Set : SelectionSets)
		{
			for (const Client::VALTAN_SELECTION_CANDIDATE_VIEW& Candidate :
				Set.Candidates)
			{
				CandidatePatterns.insert(Candidate.strPatternId);
			}
		}

		const DATA_JSON_VALUE* pIndependent = Required(
			Presentation, "independentEffects", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pIndependent || pIndependent->Get_Array().empty())
		{
			strOutError = "split presentation independent Effect inventory is invalid";
			return false;
		}
		std::map<std::string, SPLIT_INDEPENDENT_DECLARATION, std::less<>>
			IndependentById;
		std::map<std::string, std::string, std::less<>>
			IndependentByCue;
		std::map<std::string, std::string, std::less<>>
			IndependentBySpawnEvent;
		for (const DATA_JSON_VALUE& Value : pIndependent->Get_Array())
		{
			SPLIT_INDEPENDENT_DECLARATION Declaration;
			Declaration.strIndependentEffectId = Read_String(
				Value, "independentEffectId");
			Declaration.strDisplayName = Read_String(Value, "displayName");
			Declaration.strOwnership = Read_String(Value, "ownership");
			const bool_t bPatternStage =
				"SERVER_PATTERN_STAGE" == Declaration.strOwnership;
			const bool_t bCombatObject =
				"SERVER_COMBAT_OBJECT" == Declaration.strOwnership;
			if ((!bPatternStage && !bCombatObject) ||
				!Is_StableToken(Declaration.strIndependentEffectId) ||
				Declaration.strDisplayName.empty() ||
				!Has_ExactProperties(Value,
					bPatternStage ?
						std::initializer_list<std::string_view>{
							"independentEffectId", "displayName", "ownership", "cueId" } :
						std::initializer_list<std::string_view>{
							"independentEffectId", "displayName", "ownership", "spawnEventId" }))
			{
				strOutError = "split presentation independent Effect is invalid";
				return false;
			}
			Declaration.strReferenceId = Read_String(
				Value, bPatternStage ? "cueId" : "spawnEventId");
			if (!Is_StableToken(Declaration.strReferenceId) ||
				!IndependentById.emplace(
					Declaration.strIndependentEffectId, Declaration).second)
			{
				strOutError = "split presentation independent Effect identity is duplicated";
				return false;
			}
			auto& ReferenceMap = bPatternStage ?
				IndependentByCue : IndependentBySpawnEvent;
			if (!ReferenceMap.emplace(
					Declaration.strReferenceId,
					Declaration.strIndependentEffectId).second)
			{
				strOutError = "split presentation independent Effect owner reference is duplicated";
				return false;
			}
		}

		const DATA_JSON_VALUE* pGameplayPatterns = Required(
			Gameplay, "patterns", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pPresentationPatterns = Required(
			Presentation, "patterns", DATA_JSON_TYPE::ARRAY);
		if (nullptr == pGameplayPatterns || nullptr == pPresentationPatterns ||
			pGameplayPatterns->Get_Array().empty() ||
			pGameplayPatterns->Get_Array().size() !=
				pPresentationPatterns->Get_Array().size())
		{
			strOutError = "split authoring patternId closure mismatch";
			return false;
		}

		std::map<std::string, SPLIT_CUE_OWNER, std::less<>> CueOwners;
		std::map<std::string, SPLIT_SPAWN_OWNER, std::less<>> SpawnOwners;
		std::set<std::string, std::less<>> CueOccurrenceIds;
		std::set<std::string, std::less<>> EventIds;
		std::set<std::string, std::less<>> CameraInvocationIds;
		DATA_JSON_VALUE::ARRAY LegacyPatterns;
		std::set<std::string, std::less<>> ScriptedEntryOnlyPatternIds;
		for (size_t iPattern = 0u;
			iPattern < pGameplayPatterns->Get_Array().size(); ++iPattern)
		{
			const DATA_JSON_VALUE& GameplayPattern =
				pGameplayPatterns->Get_Array()[iPattern];
			const DATA_JSON_VALUE& PresentationPattern =
				pPresentationPatterns->Get_Array()[iPattern];
			if (!Has_ExactPropertiesWithOptional(GameplayPattern,
					{ "patternId", "displayName", "category",
					  "compatibilitySelectionWeight", "actionId",
					  "entryActionId", "targetPolicy", "aimPolicy", "eligibility",
					  "invulnerableWhileRunning", "sourceActionIds", "serverMotion",
					  "reactions", "stages" }, { "finale" }) ||
				!Has_ExactProperties(PresentationPattern,
					{ "patternId", "sourceSequenceIndex", "presentationSources",
					  "stages" }))
			{
				strOutError = "split authoring pattern has unexpected properties";
				return false;
			}
			const std::string strPatternId = Read_String(
				GameplayPattern, "patternId");
			if (!Is_StableToken(strPatternId) || strPatternId !=
				Read_String(PresentationPattern, "patternId"))
			{
				strOutError = "split authoring patternId order/identity mismatch";
				return false;
			}

			const DATA_JSON_VALUE* pGameplayStages = Required(
				GameplayPattern, "stages", DATA_JSON_TYPE::ARRAY);
			const DATA_JSON_VALUE* pPresentationStages = Required(
				PresentationPattern, "stages", DATA_JSON_TYPE::ARRAY);
			if (nullptr == pGameplayStages || nullptr == pPresentationStages ||
				pGameplayStages->Get_Array().empty() ||
				pGameplayStages->Get_Array().size() !=
					pPresentationStages->Get_Array().size())
			{
				strOutError = "split authoring stageId closure mismatch: " +
					strPatternId;
				return false;
			}
			std::set<std::string, std::less<>> StageIds;
			std::set<std::string, std::less<>> StageActionIds;
			for (const DATA_JSON_VALUE& Stage : pGameplayStages->Get_Array())
			{
				const std::string strStageId = Read_String(Stage, "stageId");
				const std::string strActionId = Read_String(Stage, "actionId");
				if (!Is_StableToken(strStageId) || !Is_StableToken(strActionId) ||
					!StageIds.insert(strStageId).second ||
					!StageActionIds.insert(strActionId).second)
				{
					strOutError = "split gameplay stage identity is invalid or duplicated: " +
						strPatternId;
					return false;
				}
			}
			const std::string strEntryActionId = Read_String(
				GameplayPattern, "entryActionId");
			if (!StageActionIds.contains(strEntryActionId) ||
				strEntryActionId != Read_String(
					pGameplayStages->Get_Array().front(), "actionId"))
			{
				strOutError = "split gameplay entryActionId is not the first joined stage: " +
					strPatternId;
				return false;
			}

			DATA_JSON_VALUE::ARRAY LegacyStages;
			DATA_JSON_VALUE::ARRAY WorldReferences;
			std::vector<std::string> CameraCueIds;
			std::set<std::string, std::less<>> PatternCameraCueIds;
			for (size_t iStage = 0u;
				iStage < pGameplayStages->Get_Array().size(); ++iStage)
			{
				const DATA_JSON_VALUE& GameplayStage =
					pGameplayStages->Get_Array()[iStage];
				const DATA_JSON_VALUE& PresentationStage =
					pPresentationStages->Get_Array()[iStage];
				if (!Has_ExactPropertiesWithOptional(GameplayStage,
						{ "stageId", "actionId", "stageKind", "durationMs",
						  "defaultNextActionId", "hit", "motion", "events",
						  "branches" },
						{ "partDamagePolicy", "counterProxy" }) ||
					!Has_ExactProperties(PresentationStage,
						{ "stageId", "actionId", "sequenceRole", "animation",
						  "effectCues", "cameraInvocations" }))
				{
					strOutError = "split authoring stage has unexpected properties: " +
						strPatternId;
					return false;
				}
				const std::string strStageId = Read_String(
					GameplayStage, "stageId");
				const std::string strActionId = Read_String(
					GameplayStage, "actionId");
				if (strStageId != Read_String(PresentationStage, "stageId") ||
					strActionId != Read_String(PresentationStage, "actionId"))
				{
					strOutError = "split authoring stageId/actionId mismatch: " +
						strPatternId + "/" + strStageId;
					return false;
				}
				const DATA_JSON_VALUE* pDuration = GameplayStage.Find("durationMs");
				if (!Is_NonNegativeInteger(pDuration) ||
					0.0 == pDuration->Get_Number())
				{
					strOutError = "split gameplay stage duration is invalid: " +
						strPatternId + "/" + strStageId;
					return false;
				}

				std::vector<Client::VALTAN_STAGE_BRANCH_VIEW> BranchViews;
				const DATA_JSON_VALUE* pBranches = Required(
					GameplayStage, "branches", DATA_JSON_TYPE::ARRAY);
				if (nullptr == pBranches ||
					!Read_StageBranches(pBranches, BranchViews) ||
					!Validate_SplitGameplayStageExtensions(
						GameplayStage, BranchViews, strPatternId, strStageId,
						strOutError))
				{
					if (strOutError.empty())
					{
						strOutError = "split gameplay stage branches are invalid: " +
							strPatternId + "/" + strStageId;
					}
					return false;
				}
				std::optional<std::string> ExpectedDefault;
				const auto Timeout = std::find_if(
					BranchViews.begin(), BranchViews.end(),
					[](const Client::VALTAN_STAGE_BRANCH_VIEW& Branch)
					{
						return "TIMEOUT" == Branch.strOutcome;
					});
				if (Timeout != BranchViews.end())
					ExpectedDefault = Timeout->strNextActionId;
				else if (iStage + 1u < pGameplayStages->Get_Array().size())
					ExpectedDefault = Read_String(
						pGameplayStages->Get_Array()[iStage + 1u], "actionId");
				std::string strDefault;
				if (!Read_NullableStableToken(
						GameplayStage, "defaultNextActionId", strDefault) ||
					ExpectedDefault.has_value() != !strDefault.empty() ||
					(ExpectedDefault.has_value() &&
					 *ExpectedDefault != strDefault))
				{
					strOutError = "split gameplay defaultNextActionId drifted: " +
						strPatternId + "/" + strStageId;
					return false;
				}
				for (const Client::VALTAN_STAGE_BRANCH_VIEW& Branch : BranchViews)
				{
					if (Branch.strNextActionId.has_value() &&
						!StageActionIds.contains(*Branch.strNextActionId))
					{
						strOutError = "split gameplay branch targets an unknown action: " +
							*Branch.strNextActionId;
						return false;
					}
				}

				DATA_JSON_VALUE::OBJECT LegacyStage;
				LegacyStage.emplace("stageId", DATA_JSON_VALUE::String(strStageId));
				LegacyStage.emplace("sequenceRole", DATA_JSON_VALUE::String(
					Read_String(PresentationStage, "sequenceRole")));
				LegacyStage.emplace("actionId", DATA_JSON_VALUE::String(strActionId));
				LegacyStage.emplace("stageKind", *GameplayStage.Find("stageKind"));
				LegacyStage.emplace("durationMs", *pDuration);
				if (const DATA_JSON_VALUE* pPartDamagePolicy =
						GameplayStage.Find("partDamagePolicy"))
				{
					LegacyStage.emplace("partDamagePolicy", *pPartDamagePolicy);
				}
				if (const DATA_JSON_VALUE* pCounterProxy =
						GameplayStage.Find("counterProxy"))
				{
					LegacyStage.emplace("counterProxy", *pCounterProxy);
				}
				const DATA_JSON_VALUE* pHit = Required(
					GameplayStage, "hit", DATA_JSON_TYPE::OBJECT);
				if (nullptr == pHit || !Build_SplitHitProjection(
						*pHit, LegacyStage, strOutError))
					return false;
				const DATA_JSON_VALUE* pMotion = GameplayStage.Find("motion");
				if (nullptr == pMotion ||
					(!pMotion->Is_Null() && !pMotion->Is_Object()))
				{
					strOutError = "split gameplay stage motion is invalid";
					return false;
				}
				LegacyStage.emplace("motion", *pMotion);

				DATA_JSON_VALUE::ARRAY Actions;
				DATA_JSON_VALUE::ARRAY EffectReferences;
				const DATA_JSON_VALUE* pEvents = Required(
					GameplayStage, "events", DATA_JSON_TYPE::ARRAY);
				if (nullptr == pEvents)
				{
					strOutError = "split gameplay stage events are invalid";
					return false;
				}
				for (const DATA_JSON_VALUE& Event : pEvents->Get_Array())
				{
					const std::string strEventId = Read_String(Event, "eventId");
					if (!EventIds.insert(strEventId).second)
					{
						strOutError = "split gameplay eventId is duplicated: " + strEventId;
						return false;
					}
					DATA_JSON_VALUE Action;
					std::string strSpawnArchetypeId;
					bool_t bWorldEvent = false;
					if (!Build_SplitEventProjection(Event,
							static_cast<uint32_t>(pDuration->Get_Number()), &Action,
							strSpawnArchetypeId, bWorldEvent, strOutError))
						return false;
					if (!Action.Is_Null())
						Actions.push_back(std::move(Action));
					if (!strSpawnArchetypeId.empty())
					{
						SPLIT_SPAWN_OWNER Owner;
						Owner.strPatternId = strPatternId;
						Owner.strStageId = strStageId;
						Owner.strCombatObjectArchetypeId = strSpawnArchetypeId;
						if (!SpawnOwners.emplace(strEventId, std::move(Owner)).second)
						{
							strOutError = "split gameplay spawn event owner is duplicated";
							return false;
						}
						const auto Independent = IndependentBySpawnEvent.find(strEventId);
						if (Independent != IndependentBySpawnEvent.end())
						{
							DATA_JSON_VALUE::OBJECT Reference;
							Reference.emplace("refType",
								DATA_JSON_VALUE::String("INDEPENDENT_EFFECT"));
							Reference.emplace("refId",
								DATA_JSON_VALUE::String(Independent->second));
							EffectReferences.push_back(
								DATA_JSON_VALUE::Object(std::move(Reference)));
						}
					}
					if (bWorldEvent)
					{
						if ("ENTER" != Read_String(Event, "trigger"))
						{
							strOutError = "split gameplay world event trigger is unsupported";
							return false;
						}
						DATA_JSON_VALUE::OBJECT Reference;
						Reference.emplace("patternId",
							DATA_JSON_VALUE::String(strPatternId));
						Reference.emplace("stageId",
							DATA_JSON_VALUE::String(strStageId));
						Reference.emplace("triggerKind",
							DATA_JSON_VALUE::String("STAGE_ENTER"));
						WorldReferences.push_back(
							DATA_JSON_VALUE::Object(std::move(Reference)));
					}
				}
				LegacyStage.emplace("actions", DATA_JSON_VALUE::Array(std::move(Actions)));
				LegacyStage.emplace("branches", *pBranches);

				const DATA_JSON_VALUE* pCues = Required(
					PresentationStage, "effectCues", DATA_JSON_TYPE::ARRAY);
				if (nullptr == pCues)
				{
					strOutError = "split presentation effectCues is invalid";
					return false;
				}
				for (const DATA_JSON_VALUE& Cue : pCues->Get_Array())
				{
					if (!Validate_SplitCue(Cue, strOutError))
						return false;
					const bool_t bUsesStageClock =
						nullptr != Cue.Find("timingBasis");
					const DATA_JSON_VALUE* pAnimation = Required(
						PresentationStage, "animation", DATA_JSON_TYPE::OBJECT);
					if (bUsesStageClock)
					{
						if (nullptr == pAnimation ||
							!Has_ExactProperties(*pAnimation, { "mode" }) ||
							"NONE" != Read_String(*pAnimation, "mode") ||
							Cue.Find("stageOffsetMs")->Get_Number() >=
								pDuration->Get_Number() ||
							IndependentByCue.end() == IndependentByCue.find(
								Read_String(Cue, "cueId")))
						{
							strOutError =
								"split stage-clock cue must be an independent NONE-stage Effect";
							return false;
						}
					}
					else
					{
						const DATA_JSON_VALUE* pOccurrences =
							nullptr == pAnimation ? nullptr : Required(
								*pAnimation, "occurrences", DATA_JSON_TYPE::ARRAY);
						const DATA_JSON_VALUE* pCueOccurrence = nullptr;
						if (nullptr != pOccurrences)
						{
							for (const DATA_JSON_VALUE& Occurrence :
								pOccurrences->Get_Array())
							{
								if (Read_String(Occurrence, "clipOccurrenceId") !=
									Read_String(Cue, "clipOccurrenceId"))
								{
									continue;
								}
								if (nullptr != pCueOccurrence)
								{
									strOutError =
										"split cue clip occurrence is duplicated in its stage";
									return false;
								}
								pCueOccurrence = &Occurrence;
							}
						}
						if (nullptr == pCueOccurrence ||
							Read_String(*pCueOccurrence, "mappingBasis") !=
								Read_String(Cue, "mappingBasis") ||
							!Is_NonNegativeInteger(
								pCueOccurrence->Find("sourceStartMs")) ||
							Cue.Find("sourceStartMs")->Get_Number() <
								pCueOccurrence->Find("sourceStartMs")->Get_Number())
						{
							strOutError =
								"split cue did not join its saved animation occurrence";
							return false;
						}
					}
					const std::string strCueId = Read_String(Cue, "cueId");
					const std::string strOccurrenceId = Read_String(
						Cue, "occurrenceId");
					SPLIT_CUE_OWNER CueOwner;
					CueOwner.strPatternId = strPatternId;
					CueOwner.strStageId = strStageId;
					CueOwner.pCue = &Cue;
					if (!CueOwners.emplace(strCueId, std::move(CueOwner)).second ||
						!CueOccurrenceIds.insert(strOccurrenceId).second)
					{
						strOutError = "split presentation cue identity is duplicated";
						return false;
					}
					DATA_JSON_VALUE::OBJECT Reference;
					const auto Independent = IndependentByCue.find(strCueId);
					if (Independent != IndependentByCue.end())
					{
						Reference.emplace("refType",
							DATA_JSON_VALUE::String("INDEPENDENT_EFFECT"));
						Reference.emplace("refId",
							DATA_JSON_VALUE::String(Independent->second));
					}
					else
					{
						Reference.emplace("refType",
							DATA_JSON_VALUE::String("CUE_BINDING"));
						Reference.emplace("refId", DATA_JSON_VALUE::String(strCueId));
						DATA_JSON_VALUE::OBJECT Projection;
						Projection.emplace("clipOccurrenceId",
							*Cue.Find("clipOccurrenceId"));
						Projection.emplace("sourceStartMs", *Cue.Find("sourceStartMs"));
						Projection.emplace("sourceEndMs", *Cue.Find("sourceEndMs"));
						Projection.emplace("mappingBasis", *Cue.Find("mappingBasis"));
						Reference.emplace("cueProjection",
							DATA_JSON_VALUE::Object(std::move(Projection)));
					}
					EffectReferences.push_back(
						DATA_JSON_VALUE::Object(std::move(Reference)));
				}
				LegacyStage.emplace("animation", *PresentationStage.Find("animation"));
				LegacyStage.emplace("effectRefs",
					DATA_JSON_VALUE::Array(std::move(EffectReferences)));

				const DATA_JSON_VALUE* pCamera = Required(
					PresentationStage, "cameraInvocations", DATA_JSON_TYPE::ARRAY);
				if (nullptr == pCamera)
				{
					strOutError = "split presentation cameraInvocations is invalid";
					return false;
				}
				for (const DATA_JSON_VALUE& Invocation : pCamera->Get_Array())
				{
					std::string strCameraCueId;
					if (!Validate_SplitCameraInvocation(Invocation,
							static_cast<uint32_t>(pDuration->Get_Number()),
							strCameraCueId, strOutError) ||
						!CameraInvocationIds.insert(Read_String(
							Invocation, "cameraInvocationId")).second ||
						!PatternCameraCueIds.insert(strCameraCueId).second)
					{
						if (strOutError.empty())
							strOutError = "split presentation camera identity is duplicated";
						return false;
					}
					CameraCueIds.push_back(std::move(strCameraCueId));
				}
				LegacyStage.emplace("cameraInvocations", *pCamera);
				LegacyStages.push_back(
					DATA_JSON_VALUE::Object(std::move(LegacyStage)));
			}
			if (!Validate_SplitCounterBranchContract(
					GameplayPattern, strPatternId, strOutError))
			{
				return false;
			}

			const DATA_JSON_VALUE* pEligibility = Required(
				GameplayPattern, "eligibility", DATA_JSON_TYPE::OBJECT);
			if (nullptr == pEligibility || !Has_ExactProperties(*pEligibility,
					{ "armorRequirement", "phaseRequirement", "minimumGameplayPhase",
					  "maximumGameplayPhase", "minimumHealthBarInclusive",
					  "maximumHealthBarInclusive", "minimumRangeM", "maximumRangeM",
					  "cooldownPolicy", "selectionCooldownMs", "cooldownGroupId",
					  "repeatPolicy" }))
			{
				strOutError = "split gameplay pattern eligibility is invalid: " +
					strPatternId;
				return false;
			}
			const DATA_JSON_VALUE* pRepeatPolicy = Required(
				*pEligibility, "repeatPolicy", DATA_JSON_TYPE::OBJECT);
			if (nullptr == pRepeatPolicy || !Has_ExactProperties(
					*pRepeatPolicy, { "kind", "limit" }) ||
				"SOFT_AVOID_UNLESS_ONLY_ELIGIBLE" !=
					Read_String(*pRepeatPolicy, "kind") ||
				!Is_NonNegativeInteger(pRepeatPolicy->Find("limit")) ||
				"DERIVED_SOURCE_ACTION" !=
					Read_String(*pEligibility, "cooldownPolicy") ||
				nullptr == pEligibility->Find("selectionCooldownMs") ||
				!pEligibility->Find("selectionCooldownMs")->Is_Null() ||
				nullptr == pEligibility->Find("cooldownGroupId") ||
				!pEligibility->Find("cooldownGroupId")->Is_Null())
			{
				strOutError = "split gameplay eligibility policy is invalid: " +
					strPatternId;
				return false;
			}
			for (const std::string_view Field :
				{ "minimumGameplayPhase", "maximumGameplayPhase",
				  "minimumHealthBarInclusive", "maximumHealthBarInclusive" })
			{
				if (!Is_NonNegativeInteger(pEligibility->Find(Field)))
				{
					strOutError = "split gameplay eligibility integer is invalid";
					return false;
				}
			}
			if (!Is_FiniteNumber(pEligibility->Find("minimumRangeM")) ||
				!Is_FiniteNumber(pEligibility->Find("maximumRangeM")))
			{
				strOutError = "split gameplay eligibility range is invalid";
				return false;
			}

			const auto Mechanic = Mechanics.find(strPatternId);
			const bool_t bMechanic = Mechanic != Mechanics.end();
			const bool_t bCandidate = CandidatePatterns.contains(strPatternId);
			const auto Manual = ManualAuditions.find(strPatternId);
			const bool_t bManual = Manual != ManualAuditions.end();
			const DATA_JSON_VALUE* pCompatibilityWeight =
				GameplayPattern.Find("compatibilitySelectionWeight");
			const uint32_t iOwnerCount =
				(bMechanic ? 1u : 0u) + (bCandidate ? 1u : 0u) +
				(bManual ? 1u : 0u);
			/* Both reviewed cinematics stay entry-only definitions when a custom
			   Flow omits them. Never promote an arbitrary unowned pattern. */
			const bool_t bOptionalEntryPattern =
				Is_OptionalEntryPatternId(strPatternId);
			const bool_t bScriptedEntryOnly =
				bOptionalEntryPattern && 0u == iOwnerCount;
			const auto EntryInSequence = std::find(ScriptedSequence.PatternIds.begin(),
				ScriptedSequence.PatternIds.end(), strPatternId);
			if (bScriptedEntryOnly && EntryInSequence != ScriptedSequence.PatternIds.end() &&
				(EntryInSequence != ScriptedSequence.PatternIds.begin() ||
				 std::count(ScriptedSequence.PatternIds.begin(),
					 ScriptedSequence.PatternIds.end(), strPatternId) != 1))
			{
				strOutError = "entry-only decision owner must occur only at the first slot: " + strPatternId;
				return false;
			}
			if ((bOptionalEntryPattern && !bScriptedEntryOnly) ||
				(!bOptionalEntryPattern && 1u != iOwnerCount) ||
				!Is_NonNegativeInteger(pCompatibilityWeight) ||
				((bMechanic || bManual) &&
				 0.0 != pCompatibilityWeight->Get_Number()) ||
				((bCandidate || bScriptedEntryOnly) &&
				 0.0 == pCompatibilityWeight->Get_Number()))
			{
				strOutError = "split gameplay pattern requires exactly one decision owner or an optional entry-only gate: " +
					strPatternId;
				return false;
			}
			if (bScriptedEntryOnly)
				ScriptedEntryOnlyPatternIds.insert(strPatternId);
			DATA_JSON_VALUE::OBJECT LegacyPattern;
			LegacyPattern.emplace("patternId", DATA_JSON_VALUE::String(strPatternId));
			for (const std::string& Field :
				{ "category", "targetPolicy", "aimPolicy", "displayName",
				  "actionId", "sourceActionIds", "serverMotion", "reactions" })
			{
				const DATA_JSON_VALUE* pValue = GameplayPattern.Find(Field);
				if (nullptr == pValue)
				{
					strOutError = "split gameplay pattern field is missing: " + Field;
					return false;
				}
				LegacyPattern.emplace(Field, *pValue);
			}
			if (const DATA_JSON_VALUE* pFinale = GameplayPattern.Find("finale"))
				LegacyPattern.emplace("finale", *pFinale);
			LegacyPattern.emplace("minimumPhase",
				*pEligibility->Find("minimumGameplayPhase"));
			LegacyPattern.emplace("maximumPhase",
				*pEligibility->Find("maximumGameplayPhase"));
			LegacyPattern.emplace("sourceSequenceIndex",
				*PresentationPattern.Find("sourceSequenceIndex"));
			LegacyPattern.emplace("presentationSources",
				*PresentationPattern.Find("presentationSources"));
			LegacyPattern.emplace("selectionMode",
				DATA_JSON_VALUE::String(bMechanic ? "HEALTH_BAR" :
					(bManual ? "AUDITION_ONLY" : "NORMAL")));
			LegacyPattern.emplace("minimumHealthBar",
				*pEligibility->Find("minimumHealthBarInclusive"));
			LegacyPattern.emplace("maximumHealthBar",
				*pEligibility->Find("maximumHealthBarInclusive"));
			LegacyPattern.emplace("triggerHealthBar", DATA_JSON_VALUE::Number(
				bMechanic ? Mechanic->second.iHealthBar : 0u));
			LegacyPattern.emplace("triggerOrder", DATA_JSON_VALUE::Number(
				bMechanic ? Mechanic->second.iTriggerOrder : 0u));
			LegacyPattern.emplace("armorRequirement",
				*pEligibility->Find("armorRequirement"));
			LegacyPattern.emplace("phaseRequirement",
				*pEligibility->Find("phaseRequirement"));
			LegacyPattern.emplace("invulnerableWhileRunning",
				*GameplayPattern.Find("invulnerableWhileRunning"));
			LegacyPattern.emplace("selectionWeight", DATA_JSON_VALUE::Number(
				pCompatibilityWeight->Get_Number()));
			LegacyPattern.emplace("maximumConsecutiveUses",
				*pRepeatPolicy->Find("limit"));
			LegacyPattern.emplace("minimumRange",
				*pEligibility->Find("minimumRangeM"));
			LegacyPattern.emplace("maximumRange",
				*pEligibility->Find("maximumRangeM"));
			LegacyPattern.emplace("cameraCueIds", Build_StringArray(CameraCueIds));
			LegacyPattern.emplace("worldEventTriggerRefs",
				DATA_JSON_VALUE::Array(std::move(WorldReferences)));
			LegacyPattern.emplace("stages",
				DATA_JSON_VALUE::Array(std::move(LegacyStages)));
			LegacyPatterns.push_back(
				DATA_JSON_VALUE::Object(std::move(LegacyPattern)));
		}

		DATA_JSON_VALUE::ARRAY LegacyIndependent;
		for (const auto& [IndependentId, Declaration] : IndependentById)
		{
			DATA_JSON_VALUE::OBJECT Legacy;
			Legacy.emplace("independentEffectId",
				DATA_JSON_VALUE::String(IndependentId));
			Legacy.emplace("displayName",
				DATA_JSON_VALUE::String(Declaration.strDisplayName));
			Legacy.emplace("ownership",
				DATA_JSON_VALUE::String(Declaration.strOwnership));
			if ("SERVER_PATTERN_STAGE" == Declaration.strOwnership)
			{
				const auto Owner = CueOwners.find(Declaration.strReferenceId);
				if (Owner == CueOwners.end() || nullptr == Owner->second.pCue)
				{
					strOutError = "split independent Effect cue did not resolve exactly: " +
						IndependentId;
					return false;
				}
				const DATA_JSON_VALUE& Cue = *Owner->second.pCue;
				Legacy.emplace("effectAssetId", *Cue.Find("effectAssetId"));
				Legacy.emplace("ownerPatternId",
					DATA_JSON_VALUE::String(Owner->second.strPatternId));
				Legacy.emplace("ownerStageId",
					DATA_JSON_VALUE::String(Owner->second.strStageId));
				Legacy.emplace("triggerPolicy",
					DATA_JSON_VALUE::String("PATTERN_TIMELINE"));
				Legacy.emplace("combatObjectArchetypeId", DATA_JSON_VALUE::Null());
				Legacy.emplace("clientVisualId", DATA_JSON_VALUE::Null());
				Legacy.emplace("effectCueBindingId",
					DATA_JSON_VALUE::String(Declaration.strReferenceId));
				DATA_JSON_VALUE::OBJECT Projection;
				if (nullptr != Cue.Find("timingBasis"))
				{
					Projection.emplace("timingBasis", *Cue.Find("timingBasis"));
					Projection.emplace("stageOffsetMs", *Cue.Find("stageOffsetMs"));
				}
				else
				{
					Projection.emplace("clipOccurrenceId",
						*Cue.Find("clipOccurrenceId"));
					Projection.emplace("sourceStartMs", *Cue.Find("sourceStartMs"));
					Projection.emplace("sourceEndMs", *Cue.Find("sourceEndMs"));
					Projection.emplace("mappingBasis", *Cue.Find("mappingBasis"));
				}
				Legacy.emplace("cueProjection",
					DATA_JSON_VALUE::Object(std::move(Projection)));
			}
			else
			{
				const auto Owner = SpawnOwners.find(Declaration.strReferenceId);
				if (Owner == SpawnOwners.end())
				{
					strOutError = "split independent Effect spawn did not resolve exactly: " +
						IndependentId;
					return false;
				}
				Legacy.emplace("effectAssetId", DATA_JSON_VALUE::String(
					"pending.effect." + IndependentId));
				Legacy.emplace("ownerPatternId",
					DATA_JSON_VALUE::String(Owner->second.strPatternId));
				Legacy.emplace("ownerStageId",
					DATA_JSON_VALUE::String(Owner->second.strStageId));
				Legacy.emplace("triggerPolicy",
					DATA_JSON_VALUE::String("STAGE_ENTER_PER_ALIVE_PLAYER"));
				Legacy.emplace("combatObjectArchetypeId",
					DATA_JSON_VALUE::String(
						Owner->second.strCombatObjectArchetypeId));
				Legacy.emplace("clientVisualId", DATA_JSON_VALUE::String(
					"pending.visual." + IndependentId));
				Legacy.emplace("effectCueBindingId", DATA_JSON_VALUE::Null());
				Legacy.emplace("cueProjection", DATA_JSON_VALUE::Null());
			}
			LegacyIndependent.push_back(
				DATA_JSON_VALUE::Object(std::move(Legacy)));
		}

		DATA_JSON_VALUE::OBJECT LegacyPreview;
		for (const std::string& Field :
			{ "encounter", "animationBindings", "effectCues", "combatObjects",
			  "bossCatalog", "effectCatalog", "damageProfiles", "cinematicCamera",
			  "worldEvents", "patternRotations", "sourceClipSequences" })
		{
			LegacyPreview.emplace(Field, *pPreviewPaths->Find(Field));
		}
		DATA_JSON_VALUE::OBJECT LegacyRoot;
		LegacyRoot.emplace("schema",
			DATA_JSON_VALUE::String("lostark.valtan-pattern-master"));
		LegacyRoot.emplace("formatVersion", DATA_JSON_VALUE::Number(1));
		LegacyRoot.emplace("bossArchetypeId", *Gameplay.Find("bossArchetypeId"));
		LegacyRoot.emplace("encounterId", *Gameplay.Find("encounterId"));
		LegacyRoot.emplace("scope", *Gameplay.Find("scope"));
		LegacyRoot.emplace("previewPaths",
			DATA_JSON_VALUE::Object(std::move(LegacyPreview)));
		LegacyRoot.emplace("retiredPatternIds", *Gameplay.Find("retiredPatternIds"));
		LegacyRoot.emplace("normalSelection", std::move(NormalSelection));
		LegacyRoot.emplace("counterReactionLayers",
			*Gameplay.Find("counterReactionLayers"));
		LegacyRoot.emplace("independentEffects",
			DATA_JSON_VALUE::Array(std::move(LegacyIndependent)));
		LegacyRoot.emplace("patterns",
			DATA_JSON_VALUE::Array(std::move(LegacyPatterns)));
		if (!Parse_MasterDocument(
				DATA_JSON_VALUE::Object(std::move(LegacyRoot)),
				ScriptedEntryOnlyPatternIds, Out, strOutError))
		{
			return false;
		}
		for (const std::string& strPatternId : ScriptedSequence.PatternIds)
		{
			const bool_t bExists = std::any_of(
				Out.Patterns.begin(), Out.Patterns.end(),
				[&strPatternId](const MASTER_PATTERN& Pattern)
				{
					return Pattern.strPatternId == strPatternId;
				});
			if (!bExists)
			{
				strOutError =
					"split gameplay scriptedSequence pattern is missing from Master: " +
					strPatternId;
				return false;
			}
		}
		Out.SelectionSets = std::move(SelectionSets);
		Out.SelectionWindows = std::move(SelectionWindows);
		Out.Mechanics = std::move(MechanicViews);
		Out.ManualAuditions = std::move(ManualViews);
		Out.ScriptedSequence = std::move(ScriptedSequence);
		for (size_t iPattern = 0u; iPattern < Out.Patterns.size(); ++iPattern)
		{
			MASTER_PATTERN& Pattern = Out.Patterns[iPattern];
			const auto Manual = ManualAuditions.find(Pattern.strPatternId);
			if (Manual != ManualAuditions.end())
			{
				Pattern.bManualServerAudition = true;
				Pattern.strSourceAnimationChainId =
					Manual->second.strSourceChainId;
				Pattern.iAuthoringPhase = Manual->second.iAuthoringPhase;
				Pattern.strAdmissionState =
					Manual->second.strAdmissionState;
			}
			const DATA_JSON_VALUE& PresentationPattern =
				pPresentationPatterns->Get_Array()[iPattern];
			const DATA_JSON_VALUE& GameplayPattern =
				pGameplayPatterns->Get_Array()[iPattern];
			const DATA_JSON_VALUE& PresentationStages =
				*PresentationPattern.Find("stages");
			const DATA_JSON_VALUE& GameplayStages =
				*GameplayPattern.Find("stages");
			for (size_t iStage = 0u; iStage < Pattern.Stages.size(); ++iStage)
			{
				MASTER_STAGE& Stage = Pattern.Stages[iStage];
				const DATA_JSON_VALUE& PresentationStage =
					PresentationStages.Get_Array()[iStage];
				const DATA_JSON_VALUE& GameplayStage =
					GameplayStages.Get_Array()[iStage];
				const DATA_JSON_VALUE& Cues =
					*PresentationStage.Find("effectCues");
				for (const DATA_JSON_VALUE& Cue : Cues.Get_Array())
				{
					const std::string Anchor = Read_String(Cue, "anchorSlotId");
					if (Anchor.starts_with("pattern.target."))
					{
						const bool_t bLockedTarget =
							Pattern.strTargetPolicy == "LOCK_NEAREST_ON_START" ||
							Pattern.strTargetPolicy == "LOCK_RANDOM_ALIVE_ON_START" ||
							Pattern.strTargetPolicy ==
								"LOCK_RANDOM_ALIVE_BEHIND_ON_START";
						if (Anchor != "pattern.target.snapshot" ||
							Read_String(Cue, "followPolicy") != "snapshot" ||
							!bLockedTarget)
						{
							strOutError =
								"split target cue requires its exact snapshot/locked-target contract: " +
								Pattern.strPatternId + "/" + Stage.strStageId;
							return false;
						}
					}
					else if (Anchor.starts_with("arena.center"))
					{
						const bool_t bFixedFacing =
							"arena.center.facing" == Anchor;
						const bool_t bTargetFollow =
							"arena.center.target-follow" == Anchor;
						if (("arena.center" != Anchor && !bFixedFacing &&
							 !bTargetFollow) ||
							!Pattern.ServerMotion.has_value() ||
							Pattern.ServerMotion->strKind != "LEAP_TO_ANCHOR" ||
							!Pattern.ServerMotion->bMoveToAnchorBeforeTakeoff ||
							Read_String(Cue, "followPolicy") !=
								(bTargetFollow ? "follow" : "snapshot") ||
							(bFixedFacing &&
							 (Pattern.strAimPolicy != "LOCK_FACING_ON_START" ||
							  Pattern.strTargetPolicy != "LOCK_RANDOM_ALIVE_ON_START")) ||
							(bTargetFollow &&
							 (Pattern.strAimPolicy != "TRACK_TARGET_EACH_TICK" ||
							  Pattern.strTargetPolicy != "LOCK_RANDOM_ALIVE_ON_START")))
						{
							strOutError = "split arena center cue requires its exact fixed/follow approach contract: " +
								Pattern.strPatternId + "/" + Stage.strStageId;
							return false;
						}
					}
					Stage.AuthoredCues.push_back(Build_SplitCueView(
						Cue,
						Pattern.strPatternId,
						Stage.strStageId,
						Stage.strActionId,
						static_cast<uint32_t>(GameplayStage.Find(
							"durationMs")->Get_Number())));
				}
			}
		}
		for (const MASTER_PATTERN& Pattern : Out.Patterns)
		{
			for (const MASTER_STAGE& SourceStage : Pattern.Stages)
			{
				Client::VALTAN_STAGE_VIEW Stage;
				Stage.strActionId = SourceStage.strActionId;
				Stage.iDurationMs = SourceStage.iDurationMs;
				Stage.bSuppressAnimation = SourceStage.bSuppressAnimation;
				Stage.strAnimationEndPolicy = SourceStage.strAnimationEndPolicy;
				Stage.ClipOccurrences = SourceStage.Occurrences;
				if (!Assign_MasterWallBudgets(Stage, strOutError))
				{
					strOutError = "split presentation/Server stage wall join failed: " +
						strOutError;
					return false;
				}
			}
		}
		return true;
	}

	bool_t Has_StableRestoreTopology(
		const Client::VALTAN_PATTERN_VIEW& Product,
		const MASTER_PATTERN& Master)
	{
		return Product.strPatternId == Master.strPatternId &&
			Product.strActionId == Master.strActionId &&
			Product.SourceActionIds == Master.SourceActionIds &&
			Product.Is_Gimmick() == (0 != Master.iTriggerHealthBar) &&
			Product.strSelectionMode == Master.strSelectionMode &&
			Product.Stages.size() == Master.Stages.size() &&
			std::equal(Product.Stages.begin(), Product.Stages.end(),
				Master.Stages.begin(),
				[](const Client::VALTAN_STAGE_VIEW& ProductStage,
					const MASTER_STAGE& MasterStage)
				{
					return ProductStage.strStageId == MasterStage.strStageId &&
						ProductStage.strActionId == MasterStage.strActionId;
				});
	}

	void Overlay_MasterGameplay(
		const MASTER_PATTERN& Master,
		Client::VALTAN_PATTERN_VIEW& Out)
	{
		Out.strCategory = Master.strCategory;
		Out.iMinimumPhase = Master.iMinimumPhase;
		Out.iMaximumPhase = Master.iMaximumPhase;
		Out.strTargetPolicy = Master.strTargetPolicy;
		Out.strAimPolicy = Master.strAimPolicy;
		Out.strDisplayName = Master.strDisplayName;
		Out.iMinimumHealthBar = Master.iMinimumHealthBar;
		Out.iMaximumHealthBar = Master.iMaximumHealthBar;
		Out.iTriggerHealthBar = Master.iTriggerHealthBar;
		Out.iTriggerOrder = Master.iTriggerOrder;
		Out.strArmorRequirement = Master.strArmorRequirement;
		Out.strPhaseRequirement = Master.strPhaseRequirement;
		Out.bInvulnerableWhileRunning = Master.bInvulnerableWhileRunning;
		Out.iSelectionWeight = Master.iSelectionWeight;
		Out.iMaximumConsecutiveUses = Master.iMaximumConsecutiveUses;
		Out.fMinimumRange = Master.fMinimumRange;
		Out.fMaximumRange = Master.fMaximumRange;
		Out.ServerMotion = Master.ServerMotion;
		Out.Finale = Master.Finale;
		for (size_t iStage = 0u; iStage < Master.Stages.size(); ++iStage)
		{
			const MASTER_STAGE& Source = Master.Stages[iStage];
			Client::VALTAN_STAGE_VIEW& Stage = Out.Stages[iStage];
			Stage.strStageKind = Source.strStageKind;
			Stage.iDurationMs = Source.iDurationMs;
			Stage.strHitShape = Source.strHitShape;
			Stage.fHitOuterRadius = Source.fHitOuterRadius;
			Stage.fHitInnerRadius = Source.fHitInnerRadius;
			Stage.fHitAngleDegrees = Source.fHitAngleDegrees;
			Stage.fHitLength = Source.fHitLength;
			Stage.fHitHalfWidth = Source.fHitHalfWidth;
			Stage.iHitCount = Source.iHitCount;
			Stage.iHitIntervalMs = Source.iHitIntervalMs;
			Stage.iHitDelayMs = Source.iHitDelayMs;
			Stage.HitOffsetsMs = Source.HitOffsetsMs;
			Stage.bHasHitAnchor = Source.bHasHitAnchor;
			Stage.strHitAnchorKind = Source.strHitAnchorKind;
			Stage.fHitAnchorForwardOffsetM = Source.fHitAnchorForwardOffsetM;
			Stage.fHitAnchorRightOffsetM = Source.fHitAnchorRightOffsetM;
			Stage.fHitAnchorYawOffsetDegrees =
				Source.fHitAnchorYawOffsetDegrees;
			Stage.bHasHitActivation = Source.bHasHitActivation;
			Stage.iHitActivationStartMs = Source.iHitActivationStartMs;
			Stage.iHitActivationLifetimeMs =
				Source.iHitActivationLifetimeMs;
			Stage.strServerDamageProfileId = Source.strServerDamageProfileId;
			Stage.strPlayerResponse = Source.strPlayerResponse;
			Stage.strAttachmentSlot = Source.strAttachmentSlot;
			Stage.strPartDamagePolicy = Source.strPartDamagePolicy;
			Stage.CounterProxy = Source.CounterProxy;
			Stage.fPushRangeM = Source.fPushRangeM;
			Stage.iPushMs = Source.iPushMs;
			Stage.bKnockdown = Source.bKnockdown;
			Stage.iDownMs = Source.iDownMs;
			Stage.Motion = Source.Motion;
			Stage.Actions = Source.Actions;
			Stage.Branches = Source.Branches;
		}
	}

	bool_t Apply_MasterDocument(
		const MASTER_DOCUMENT& Master,
		const DATA_JSON_VALUE& PatternRotations,
		Client::VALTAN_PATTERN_TREE_VIEW& View,
		std::string& strOutError,
		const Client::VALTAN_PATTERN_TREE_LOAD_POLICY ePolicy)
	{
		if (Client::VALTAN_PATTERN_TREE_LOAD_POLICY::
				REQUIRE_ACTIVE_PRODUCT_PARITY != ePolicy &&
			Client::VALTAN_PATTERN_TREE_LOAD_POLICY::
				RESTORE_AUTHORING_SNAPSHOT != ePolicy)
		{
			strOutError = "Valtan pattern-tree load policy is invalid";
			return false;
		}
		const bool_t bRequireProductParity =
			Client::VALTAN_PATTERN_TREE_LOAD_POLICY::
				REQUIRE_ACTIVE_PRODUCT_PARITY == ePolicy;
		for (const std::string& strRetiredPatternId : Master.RetiredPatternIds)
		{
			if (nullptr != Find_Pattern(View, strRetiredPatternId))
			{
				strOutError = "retired master pattern still exists in Product: " +
					strRetiredPatternId;
				return false;
			}
		}

		std::map<std::string,
			const Client::VALTAN_INDEPENDENT_EFFECT_VIEW*, std::less<>>
			IndependentById;
		for (const Client::VALTAN_INDEPENDENT_EFFECT_VIEW& Independent :
			Master.IndependentEffects)
		{
			IndependentById.emplace(
				Independent.strIndependentEffectId, &Independent);
		}
		std::set<std::string, std::less<>> ReferencedIndependentIds;

		for (const MASTER_PATTERN& MasterPattern : Master.Patterns)
		{
			Client::VALTAN_PATTERN_VIEW* pPattern = Find_Pattern(
				View, MasterPattern.strPatternId);
			if (nullptr == pPattern ||
				(bRequireProductParity &&
				 !Equal_MasterPatternGameplay(*pPattern, MasterPattern)) ||
				(!bRequireProductParity &&
				 !Has_StableRestoreTopology(*pPattern, MasterPattern)))
			{
				strOutError = bRequireProductParity ?
					"master/Product pattern projection changed: " +
						MasterPattern.strPatternId :
					"saved authoring pattern topology changed: " +
					MasterPattern.strPatternId;
				return false;
			}
			if (!bRequireProductParity)
				Overlay_MasterGameplay(MasterPattern, *pPattern);

			pPattern->iSourceSequenceIndex =
				MasterPattern.iSourceSequenceIndex;
			pPattern->PresentationSources = MasterPattern.PresentationSources;
			pPattern->Reactions = MasterPattern.Reactions;
			pPattern->CameraCueIds = MasterPattern.CameraCueIds;
			pPattern->WorldEventTriggerRefs =
				MasterPattern.WorldEventTriggerRefs;
			pPattern->bAuthoringMasterManaged = true;
			pPattern->strEntryActionId = MasterPattern.Stages.empty() ?
				std::string{} : MasterPattern.Stages.front().strActionId;
			pPattern->bManualServerAudition =
				MasterPattern.bManualServerAudition;
			pPattern->strSourceAnimationChainId =
				MasterPattern.strSourceAnimationChainId;
			pPattern->iAuthoringPhase = MasterPattern.iAuthoringPhase;
			pPattern->strAdmissionState = MasterPattern.strAdmissionState;
			for (size_t iStage = 0u; iStage < MasterPattern.Stages.size();
				++iStage)
			{
				const MASTER_STAGE& MasterStage = MasterPattern.Stages[iStage];
				Client::VALTAN_STAGE_VIEW& Stage = pPattern->Stages[iStage];
				if (bRequireProductParity &&
					(!Equal_MasterStageGameplay(Stage, MasterStage) ||
					 Stage.bSuppressAnimation !=
						MasterStage.bSuppressAnimation ||
					 Stage.ClipOccurrences.size() !=
						MasterStage.Occurrences.size() ||
					 Stage.ProductCues.size() != MasterStage.AuthoredCues.size()))
				{
					strOutError = "master/Product stage projection changed: " +
						MasterPattern.strPatternId + "/" + MasterStage.strStageId;
					return false;
				}
				if (bRequireProductParity)
				{
					for (size_t iClip = 0u;
						iClip < MasterStage.Occurrences.size(); ++iClip)
					{
						if (!Equal_MasterOccurrence(
								Stage.ClipOccurrences[iClip],
								MasterStage.Occurrences[iClip]))
						{
							strOutError =
								"master/Product animation occurrence changed: " +
								MasterStage.Occurrences[iClip].strClipOccurrenceId;
							return false;
						}
					}
					for (size_t iCue = 0u;
						iCue < MasterStage.AuthoredCues.size(); ++iCue)
					{
						if (!Equal_AuthoredCue(
								Stage.ProductCues[iCue],
								MasterStage.AuthoredCues[iCue]))
						{
							strOutError =
								"master/Product Effect cue changed: " +
								MasterStage.AuthoredCues[iCue].strBindingId;
							return false;
						}
					}
				}
				else
				{
					Stage.ClipOccurrences = MasterStage.Occurrences;
					Stage.RuntimeClipNames.clear();
					for (const Client::VALTAN_CLIP_OCCURRENCE_VIEW& Occurrence :
						Stage.ClipOccurrences)
					{
						Stage.RuntimeClipNames.push_back(Occurrence.strClipName);
					}
					Stage.strRuntimeClipName = Stage.RuntimeClipNames.empty() ?
						std::string{} : Stage.RuntimeClipNames.front();
					Stage.ProductCues = MasterStage.AuthoredCues;
					Stage.ProductCue = Stage.ProductCues.empty() ?
						std::optional<Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW>{} :
						std::optional<Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW>{
							Stage.ProductCues.front() };
					for (Client::VALTAN_CLIP_OCCURRENCE_VIEW& Occurrence :
						Stage.ClipOccurrences)
					{
						Occurrence.ProductCues.clear();
						for (const Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue :
							Stage.ProductCues)
						{
							if (Cue.strClipOccurrenceId ==
								Occurrence.strClipOccurrenceId)
							{
								Occurrence.ProductCues.push_back(Cue);
							}
						}
					}
				}
				Stage.strSequenceRole = MasterStage.strSequenceRole;
				Stage.iAuthoringRepeatCount = MasterStage.iRepeatCount;
				Stage.strAnimationEndPolicy =
					MasterStage.strAnimationEndPolicy;
				Stage.bSuppressAnimation = MasterStage.bSuppressAnimation;
				Stage.CameraInvocations = MasterStage.CameraInvocations;
				if (!Assign_MasterWallBudgets(Stage, strOutError))
					return false;

				for (const MASTER_EFFECT_REFERENCE& Reference :
					MasterStage.EffectReferences)
				{
					if ("CUE_BINDING" == Reference.strType)
					{
						const bool_t bFound = std::any_of(
							Stage.ProductCues.begin(), Stage.ProductCues.end(),
							[&Reference](
								const Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue)
							{
								return Cue.strBindingId == Reference.strId &&
									Cue.strClipOccurrenceId ==
										Reference.strClipOccurrenceId &&
									Cue.iSourceStartMs == Reference.iSourceStartMs &&
									Cue.bHasSourceEnd == Reference.bHasSourceEnd &&
									(!Reference.bHasSourceEnd ||
									 Cue.iSourceEndMs == Reference.iSourceEndMs);
							});
						if (!bFound)
						{
							strOutError =
								"master cue reference left its Product stage: " +
								Reference.strId;
							return false;
						}
						continue;
					}
					const auto Independent = IndependentById.find(Reference.strId);
					if (Independent == IndependentById.end() ||
						Independent->second->strOwnerPatternId !=
							pPattern->strPatternId ||
						Independent->second->strOwnerStageId != Stage.strStageId)
					{
						strOutError = "master independent Effect reference is stale: " +
							Reference.strId;
						return false;
					}
					Stage.IndependentEffectIds.push_back(Reference.strId);
					ReferencedIndependentIds.insert(Reference.strId);
				}
			}
		}

		const DATA_JSON_VALUE* pProductScriptedSequence = Required(
			PatternRotations, "scriptedSequence", DATA_JSON_TYPE::OBJECT);
		const DATA_JSON_VALUE* pProductScriptedPatterns =
			nullptr == pProductScriptedSequence ? nullptr : Required(
				*pProductScriptedSequence, "patternIds", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pRotationRows = Required(
			PatternRotations, "rotations", DATA_JSON_TYPE::ARRAY);
		if (!Has_ExactProperties(PatternRotations,
				{ "schema", "formatVersion", "encounterId", "bossArchetypeId",
				  "scriptedSequence", "rotations" }) ||
			nullptr == Required(
				PatternRotations, "schema", DATA_JSON_TYPE::STRING) ||
			"lostark.valtan-pattern-rotations" != Required(
				PatternRotations, "schema", DATA_JSON_TYPE::STRING)->Get_String() ||
			!Is_NonNegativeInteger(PatternRotations.Find("formatVersion")) ||
			4.0 != PatternRotations.Find("formatVersion")->Get_Number() ||
			nullptr == pProductScriptedSequence ||
			!Has_ExactProperties(*pProductScriptedSequence,
				{ "sequenceId", "mode", "interStepPursuitMs", "patternIds" }) ||
			!Is_StableToken(Read_String(
				*pProductScriptedSequence, "sequenceId")) ||
			"ORDERED_ONCE_THEN_IDLE" != Read_String(
				*pProductScriptedSequence, "mode") ||
			!Is_NonNegativeInteger(
				pProductScriptedSequence->Find("interStepPursuitMs")) ||
			pProductScriptedSequence->Find(
				"interStepPursuitMs")->Get_Number() < 100.0 ||
			pProductScriptedSequence->Find(
				"interStepPursuitMs")->Get_Number() > 10000.0 ||
			nullptr == pProductScriptedPatterns ||
			pProductScriptedPatterns->Get_Array().empty() ||
			pProductScriptedPatterns->Get_Array().size() >
				Client::CValtanPatternFlowDocument::MAX_SLOTS ||
			nullptr == pRotationRows)
		{
			strOutError =
				"Valtan selection-window/scripted-sequence Product v4 is invalid";
			return false;
		}
		std::vector<std::string> ProductScriptedPatternIds;
		for (const DATA_JSON_VALUE& PatternId :
			pProductScriptedPatterns->Get_Array())
		{
			const Client::VALTAN_PATTERN_VIEW* pPattern = PatternId.Is_String() ?
				Find_Pattern(View, PatternId.Get_String()) : nullptr;
			if (!PatternId.Is_String() ||
				!Is_StableToken(PatternId.Get_String()) ||
				nullptr == pPattern)
			{
				strOutError =
					"Valtan scripted-sequence Product pattern is invalid";
				return false;
			}
			ProductScriptedPatternIds.push_back(PatternId.Get_String());
		}
		/* The gameplay scriptedSequence is the editable order authority and the
		   rotations document is its generated Product.  An admitted canonical
		   generation must therefore contain exact sequence identity, timing and
		   ordered Pattern parity. */
		if (bRequireProductParity &&
			(Read_String(*pProductScriptedSequence, "sequenceId") !=
				Master.ScriptedSequence.strSequenceId ||
			 Read_String(*pProductScriptedSequence, "mode") !=
				Master.ScriptedSequence.strMode ||
			 static_cast<uint32_t>(pProductScriptedSequence->Find(
					"interStepPursuitMs")->Get_Number()) !=
					Master.ScriptedSequence.iInterStepPursuitMs ||
				 ProductScriptedPatternIds !=
					Master.ScriptedSequence.PatternIds))
		{
			strOutError = "Valtan scripted-sequence Product parity drifted";
			return false;
		}
		View.strScriptedSequenceId = Master.ScriptedSequence.strSequenceId;
		View.strScriptedSequenceMode = Master.ScriptedSequence.strMode;
		View.iScriptedSequenceInterStepPursuitMs =
			Master.ScriptedSequence.iInterStepPursuitMs;
		View.ScriptedSequencePatternIds = Master.ScriptedSequence.PatternIds;
		std::map<std::string, const Client::VALTAN_SELECTION_WINDOW_VIEW*,
			std::less<>> WindowByRotation;
		for (const Client::VALTAN_SELECTION_WINDOW_VIEW& Window :
			Master.SelectionWindows)
		{
			WindowByRotation.emplace(Window.strCompatibilityRotationId, &Window);
		}
		std::map<std::string, const Client::VALTAN_SELECTION_SET_VIEW*,
			std::less<>> SetById;
		for (const Client::VALTAN_SELECTION_SET_VIEW& Set : Master.SelectionSets)
			SetById.emplace(Set.strSelectionSetId, &Set);
		struct LEGACY_ROTATION_IDENTITY
		{
			std::string_view strRotationId;
			uint32_t iFromHealthBar;
			uint32_t iToHealthBar;
		};
		static constexpr LEGACY_ROTATION_IDENTITY LegacyRotationOrder[] = {
			{ "rotation.valtan.109.100", 108u, 100u },
			{ "rotation.valtan.100.84", 99u, 84u },
			{ "rotation.valtan.84.73", 83u, 73u },
			{ "rotation.valtan.73.62", 72u, 62u },
			{ "rotation.valtan.62.30", 61u, 30u },
			{ "rotation.valtan.28.14", 29u, 14u },
		};
		static constexpr size_t iLegacyRotationCount =
			sizeof(LegacyRotationOrder) / sizeof(LegacyRotationOrder[0]);
		std::set<std::string, std::less<>> ProductRotationIds;
		std::set<std::string, std::less<>> SeenManagedRotations;
		size_t iManagedRotationOrdinal = 0u;
		size_t iLegacyRotationOrdinal = 0u;
		View.LegacyRotations.clear();
		for (const DATA_JSON_VALUE& Row : pRotationRows->Get_Array())
		{
			const std::string strRotationId = Read_String(Row, "rotationId");
			if (!Is_StableToken(strRotationId) ||
				!ProductRotationIds.insert(strRotationId).second)
			{
				strOutError = "Valtan Product rotation identity is invalid/duplicated";
				return false;
			}
			const auto Managed = WindowByRotation.find(strRotationId);
			if (Managed != WindowByRotation.end())
			{
				if (0u != iLegacyRotationOrdinal ||
					iManagedRotationOrdinal >= Master.SelectionWindows.size() ||
					strRotationId != Master.SelectionWindows[
						iManagedRotationOrdinal].strCompatibilityRotationId)
				{
					strOutError =
						"Valtan managed Product rotation order drifted";
					return false;
				}
				++iManagedRotationOrdinal;
				if (!Has_ExactProperties(Row,
						{ "rotationId", "selectionMode", "fromHealthBar",
						  "toHealthBar", "windowId", "gameplayPhase",
						  "selectionSetId", "candidates" }))
				{
					strOutError = "Valtan managed Product row is not the v4 tagged shape";
					return false;
				}
				SeenManagedRotations.insert(strRotationId);
				if (!bRequireProductParity)
					continue;
				const Client::VALTAN_SELECTION_WINDOW_VIEW& Window =
					*Managed->second;
				const auto Set = SetById.find(Window.strSelectionSetId);
				const DATA_JSON_VALUE* pCandidates = Required(
					Row, "candidates", DATA_JSON_TYPE::ARRAY);
				if (Set == SetById.end() ||
					Read_String(Row, "selectionMode") != Set->second->strMode ||
					Read_String(Row, "windowId") != Window.strWindowId ||
					Read_String(Row, "selectionSetId") != Window.strSelectionSetId ||
					!Is_NonNegativeInteger(Row.Find("gameplayPhase")) ||
					!Is_NonNegativeInteger(Row.Find("fromHealthBar")) ||
					!Is_NonNegativeInteger(Row.Find("toHealthBar")) ||
					Window.iGameplayPhase != static_cast<uint32_t>(
						Row.Find("gameplayPhase")->Get_Number()) ||
					Window.iMaximumHealthBarInclusive != static_cast<uint32_t>(
						Row.Find("fromHealthBar")->Get_Number()) ||
					Window.iMinimumHealthBarExclusive != static_cast<uint32_t>(
						Row.Find("toHealthBar")->Get_Number()) ||
					nullptr == pCandidates || pCandidates->Get_Array().size() !=
						Set->second->Candidates.size())
				{
					strOutError = "Valtan managed Product window/set projection drifted: " +
						strRotationId;
					return false;
				}
				for (size_t i = 0u; i < Set->second->Candidates.size(); ++i)
				{
					const DATA_JSON_VALUE& ProductCandidate =
						pCandidates->Get_Array()[i];
					const Client::VALTAN_SELECTION_CANDIDATE_VIEW& Candidate =
						Set->second->Candidates[i];
					const DATA_JSON_VALUE* pEnabled = Required(
						ProductCandidate, "enabled", DATA_JSON_TYPE::BOOLEAN);
					if (!Has_ExactProperties(ProductCandidate,
							{ "patternId", "weight", "enabled" }) ||
						Read_String(ProductCandidate, "patternId") !=
							Candidate.strPatternId ||
						!Is_NonNegativeInteger(ProductCandidate.Find("weight")) ||
						Candidate.iWeight != static_cast<uint32_t>(
							ProductCandidate.Find("weight")->Get_Number()) ||
						nullptr == pEnabled ||
						Candidate.bEnabled != pEnabled->Get_Boolean())
					{
						strOutError = "Valtan managed Product candidate projection drifted: " +
							strRotationId;
						return false;
					}
				}
				continue;
			}

			if (iManagedRotationOrdinal != Master.SelectionWindows.size() ||
				iLegacyRotationOrdinal >= iLegacyRotationCount ||
				strRotationId != LegacyRotationOrder[
					iLegacyRotationOrdinal].strRotationId)
			{
				strOutError = "Valtan legacy Product rotation order/identity drifted";
				return false;
			}
			const LEGACY_ROTATION_IDENTITY& ExpectedLegacy =
				LegacyRotationOrder[iLegacyRotationOrdinal];
			const DATA_JSON_VALUE* pPatternIds = Required(
				Row, "patternIds", DATA_JSON_TYPE::ARRAY);
			if (!Has_ExactProperties(Row,
					{ "rotationId", "selectionMode", "fromHealthBar",
					  "toHealthBar", "patternIds" }) ||
				!Is_NonNegativeInteger(Row.Find("fromHealthBar")) ||
				!Is_NonNegativeInteger(Row.Find("toHealthBar")) ||
				nullptr == pPatternIds || pPatternIds->Get_Array().empty() ||
				"ORDERED_INTRO_THEN_WEIGHTED" != Read_String(Row, "selectionMode") ||
				ExpectedLegacy.iFromHealthBar != static_cast<uint32_t>(
					Row.Find("fromHealthBar")->Get_Number()) ||
				ExpectedLegacy.iToHealthBar != static_cast<uint32_t>(
					Row.Find("toHealthBar")->Get_Number()))
			{
				strOutError = "Valtan legacy Product rotation shape is invalid";
				return false;
			}
			Client::VALTAN_LEGACY_ROTATION_VIEW Legacy;
			Legacy.strRotationId = strRotationId;
			Legacy.strSelectionMode = Read_String(Row, "selectionMode");
			Legacy.iFromHealthBar = static_cast<uint32_t>(
				Row.Find("fromHealthBar")->Get_Number());
			Legacy.iToHealthBar = static_cast<uint32_t>(
				Row.Find("toHealthBar")->Get_Number());
			for (const DATA_JSON_VALUE& PatternId : pPatternIds->Get_Array())
			{
				if (!PatternId.Is_String() ||
					!Is_StableToken(PatternId.Get_String()) ||
					nullptr == Find_Pattern(View, PatternId.Get_String()))
				{
					strOutError = "Valtan legacy Product rotation patternId is invalid";
					return false;
				}
				Legacy.PatternIds.push_back(PatternId.Get_String());
			}
			View.LegacyRotations.push_back(std::move(Legacy));
			++iLegacyRotationOrdinal;
		}
		if (SeenManagedRotations.size() != Master.SelectionWindows.size() ||
			iManagedRotationOrdinal != Master.SelectionWindows.size() ||
			iLegacyRotationOrdinal != iLegacyRotationCount)
		{
			strOutError =
				"Valtan Product selection-window/legacy rotation coverage is incomplete";
			return false;
		}
		View.SelectionSets = Master.SelectionSets;
		View.SelectionWindows = Master.SelectionWindows;
		View.Mechanics = Master.Mechanics;
		View.ManualAuditions = Master.ManualAuditions;
		View.NormalSelection = Master.NormalSelection;
		for (const std::string& PatternId : View.NormalSelection.PatternIds)
		{
			Client::VALTAN_PATTERN_VIEW* pPattern = Find_Pattern(View, PatternId);
			if (nullptr == pPattern || !pPattern->bAuthoringMasterManaged ||
				"NORMAL" != pPattern->strSelectionMode)
			{
				strOutError =
					"Valtan normal-selection pool left the managed normal set: " +
					PatternId;
				return false;
			}
		}
		for (const Client::VALTAN_MANUAL_AUDITION_VIEW& Manual :
			View.ManualAuditions)
		{
			Client::VALTAN_PATTERN_VIEW* pPattern = Find_Pattern(
				View, Manual.strPatternId);
			if (nullptr == pPattern || !pPattern->bAuthoringMasterManaged ||
				!pPattern->bManualServerAudition ||
				"AUDITION_ONLY" != pPattern->strSelectionMode ||
				pPattern->strSourceAnimationChainId != Manual.strSourceChainId ||
				pPattern->iAuthoringPhase != Manual.iAuthoringPhase ||
				pPattern->strAdmissionState != Manual.strAdmissionState)
			{
				strOutError =
					"Valtan manual audition left its managed Product pattern: " +
					Manual.strPatternId;
				return false;
			}
		}

		std::set<std::string, std::less<>> ProductCounterOwnerStages;
		for (std::vector<Client::VALTAN_PATTERN_VIEW>* pGroup :
			{ &View.Gimmicks, &View.Rotation })
		{
			for (Client::VALTAN_PATTERN_VIEW& Pattern : *pGroup)
			{
				/* counterReactionLayers are explicitly REFERENCE_ONLY_LEGACY.
				   Split-owned counters validate through their authored gameplay
				   stages and must not expand this legacy exact-cover set. */
				if (Pattern.bAuthoringMasterManaged)
					continue;
				for (Client::VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
				{
					const bool_t bCounterable = std::any_of(
						Stage.Actions.begin(), Stage.Actions.end(),
						[](const Client::VALTAN_STAGE_ACTION_VIEW& Action)
						{
							return "ENTER" == Action.strTrigger &&
								"SET_BOSS_FLAG" == Action.strKind &&
								"boss.flag.counterable" == Action.strTargetId &&
								1.f == Action.fValue;
						});
					if (bCounterable)
						ProductCounterOwnerStages.insert(
							Pattern.strPatternId + "/" + Stage.strStageId);
				}
			}
		}
		std::set<std::string, std::less<>> MasterCounterOwnerStages;
		for (const Client::VALTAN_COUNTER_REACTION_LAYER_VIEW& MasterLayer :
			Master.CounterReactionLayers)
		{
			Client::VALTAN_PATTERN_VIEW* pOwnerPattern = Find_Pattern(
				View, MasterLayer.strOwnerPatternId);
			Client::VALTAN_STAGE_VIEW* pWindow = nullptr == pOwnerPattern ?
				nullptr : Find_Stage(*pOwnerPattern, MasterLayer.strOwnerStageId);
			const auto FindActionStage = [pOwnerPattern](
				const std::string& ActionId) -> Client::VALTAN_STAGE_VIEW*
			{
				if (nullptr == pOwnerPattern)
					return nullptr;
				Client::VALTAN_STAGE_VIEW* pFound = nullptr;
				for (Client::VALTAN_STAGE_VIEW& Stage : pOwnerPattern->Stages)
				{
					if (Stage.strActionId != ActionId)
						continue;
					if (nullptr != pFound)
						return nullptr;
					pFound = &Stage;
				}
				return pFound;
			};
			Client::VALTAN_STAGE_VIEW* pSuccess = FindActionStage(
				MasterLayer.Success.strActionId);
			Client::VALTAN_STAGE_VIEW* pFailure = FindActionStage(
				MasterLayer.Failure.strActionId);
			const bool_t bHasCounterEnter = nullptr != pWindow && std::any_of(
				pWindow->Actions.begin(), pWindow->Actions.end(),
				[](const Client::VALTAN_STAGE_ACTION_VIEW& Action)
				{
					return "ENTER" == Action.strTrigger &&
						"SET_BOSS_FLAG" == Action.strKind &&
						"boss.flag.counterable" == Action.strTargetId &&
						1.f == Action.fValue;
				});
			const bool_t bHasCounterExit = nullptr != pWindow && std::any_of(
				pWindow->Actions.begin(), pWindow->Actions.end(),
				[](const Client::VALTAN_STAGE_ACTION_VIEW& Action)
				{
					return "EXIT" == Action.strTrigger &&
						"SET_BOSS_FLAG" == Action.strKind &&
						"boss.flag.counterable" == Action.strTargetId &&
						0.f == Action.fValue;
				});
			const auto HasBranch = [pWindow](
				const std::string_view Outcome, const std::string& ActionId)
			{
				return nullptr != pWindow && std::any_of(
					pWindow->Branches.begin(), pWindow->Branches.end(),
					[Outcome, &ActionId](
						const Client::VALTAN_STAGE_BRANCH_VIEW& Branch)
					{
						return Branch.strOutcome == Outcome &&
							Branch.strNextActionId.has_value() &&
							*Branch.strNextActionId == ActionId;
					});
			};
			if (nullptr == pOwnerPattern || pOwnerPattern->bAuthoringMasterManaged ||
				nullptr == pWindow || pWindow->strActionId !=
					MasterLayer.Window.strActionId ||
				nullptr == pSuccess || nullptr == pFailure ||
				pWindow->ClipOccurrences.empty() ||
				pSuccess->ClipOccurrences.empty() ||
				pFailure->ClipOccurrences.empty() ||
				!bHasCounterEnter || !bHasCounterExit ||
				!HasBranch("COUNTER_HIT", MasterLayer.Success.strActionId) ||
				!HasBranch("TIMEOUT", MasterLayer.Failure.strActionId))
			{
				strOutError =
					"reference-only counter reaction Product join changed: " +
					MasterLayer.strReactionLayerId;
				return false;
			}
			Client::VALTAN_COUNTER_REACTION_LAYER_VIEW Layer = MasterLayer;
			Layer.Window.ClipOccurrences = pWindow->ClipOccurrences;
			Layer.Success.ClipOccurrences = pSuccess->ClipOccurrences;
			Layer.Failure.ClipOccurrences = pFailure->ClipOccurrences;
			View.CounterReactionLayers.push_back(std::move(Layer));
			MasterCounterOwnerStages.insert(
				MasterLayer.strOwnerPatternId + "/" +
				MasterLayer.strOwnerStageId);
		}
		if (ProductCounterOwnerStages != MasterCounterOwnerStages)
		{
			strOutError =
				"counter reaction master does not cover exact Product counter stages";
			return false;
		}

		if (ReferencedIndependentIds.size() != Master.IndependentEffects.size())
		{
			strOutError =
				"master independent Effect inventory contains an unreferenced identity";
			return false;
		}
		for (const Client::VALTAN_INDEPENDENT_EFFECT_VIEW& Independent :
			Master.IndependentEffects)
		{
			Client::VALTAN_PATTERN_VIEW* pOwnerPattern = Find_Pattern(
				View, Independent.strOwnerPatternId);
			Client::VALTAN_STAGE_VIEW* pOwnerStage = nullptr == pOwnerPattern ?
				nullptr : Find_Stage(*pOwnerPattern, Independent.strOwnerStageId);
			if (nullptr == pOwnerPattern || !pOwnerPattern->bAuthoringMasterManaged ||
				nullptr == pOwnerStage)
			{
				strOutError = "master independent Effect owner is stale: " +
					Independent.strIndependentEffectId;
				return false;
			}
			bool_t bOwnerJoined = false;
			if ("SERVER_COMBAT_OBJECT" == Independent.strOwnership)
			{
				bOwnerJoined = std::any_of(
					pOwnerStage->CombatObjectEffects.begin(),
					pOwnerStage->CombatObjectEffects.end(),
					[&Independent](
						const Client::VALTAN_COMBAT_OBJECT_EFFECT_VIEW& Effect)
					{
						return Effect.strEffectAssetId ==
								Independent.strEffectAssetId &&
							Effect.strCombatObjectArchetypeId ==
								Independent.strCombatObjectArchetypeId &&
							Effect.strClientVisualId ==
								Independent.strClientVisualId;
					});
			}
			else
			{
				bOwnerJoined = std::any_of(
					pOwnerStage->ProductCues.begin(),
					pOwnerStage->ProductCues.end(),
					[&Independent](
						const Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue)
					{
						return Cue.strEffectAssetId == Independent.strEffectAssetId &&
							Cue.strBindingId == Independent.strEffectCueBindingId &&
							Independent.bHasCueProjection &&
							(Cue.bUsesStageClock ==
								Independent.bUsesCueStageClock) &&
							(Independent.bUsesCueStageClock ?
								(Cue.iStageOffsetMs ==
									Independent.iCueStageOffsetMs) :
								(Cue.strClipOccurrenceId ==
									Independent.strCueClipOccurrenceId &&
								 Cue.iSourceStartMs ==
									Independent.iCueSourceStartMs &&
								 Cue.bHasSourceEnd ==
									Independent.bHasCueSourceEnd &&
								 (!Independent.bHasCueSourceEnd ||
								  Cue.iSourceEndMs ==
									Independent.iCueSourceEndMs)));
					});
			}
			const std::filesystem::path EffectDocument =
				Client::CProjectDataRoot::Resolve(
					std::filesystem::path(L"Effects") / L"Authored" /
					std::filesystem::path(
						Independent.strEffectAssetId + ".effect.json"));
			std::error_code FileError;
			if (!bOwnerJoined || EffectDocument.empty() ||
				!std::filesystem::is_regular_file(EffectDocument, FileError))
			{
				strOutError =
					"master independent Effect did not resolve to one Product owner/document: " +
					Independent.strIndependentEffectId;
				return false;
			}
		}

		View.IndependentEffects = Master.IndependentEffects;
		return true;
	}

	std::string Describe_FollowPolicy(
		const Client::EFFECT_FOLLOW_POLICY ePolicy)
	{
		switch (ePolicy)
		{
		case Client::EFFECT_FOLLOW_POLICY::FOLLOW: return "follow";
		case Client::EFFECT_FOLLOW_POLICY::SNAPSHOT: return "snapshot";
		default: return {};
		}
	}

	std::string Describe_StopPolicy(
		const Client::EFFECT_STOP_POLICY ePolicy)
	{
		switch (ePolicy)
		{
		case Client::EFFECT_STOP_POLICY::NATURAL: return "natural";
		case Client::EFFECT_STOP_POLICY::CUE_END: return "cue_end";
		default: return {};
		}
	}

	std::string Describe_RepeatPolicy(
		const Client::VALTAN_PATTERN_EFFECT_REPEAT_POLICY ePolicy)
	{
		switch (ePolicy)
		{
		case Client::VALTAN_PATTERN_EFFECT_REPEAT_POLICY::ONCE: return "once";
		case Client::VALTAN_PATTERN_EFFECT_REPEAT_POLICY::EACH_LOOP:
			return "each_loop";
		default: return {};
		}
	}

	std::string Describe_ScalePolicy(
		const Client::VALTAN_PATTERN_EFFECT_SCALE_POLICY ePolicy)
	{
		switch (ePolicy)
		{
		case Client::VALTAN_PATTERN_EFFECT_SCALE_POLICY::OWNER_RELATIVE:
			return "OWNER_RELATIVE";
		case Client::VALTAN_PATTERN_EFFECT_SCALE_POLICY::GAMEPLAY_FOOTPRINT:
			return "GAMEPLAY_FOOTPRINT";
		case Client::VALTAN_PATTERN_EFFECT_SCALE_POLICY::ARENA_ABSOLUTE:
			return "ARENA_ABSOLUTE";
		default: return {};
		}
	}
}

Client::CValtanCanonicalProductReadAdmission::
	~CValtanCanonicalProductReadAdmission()
{
	delete static_cast<VALTAN_CANONICAL_PRODUCT_READ_STATE*>(m_pState);
	m_pState = nullptr;
}

bool_t Client::CValtanCanonicalProductReadAdmission::Acquire(
	std::string& strOutStatus)
{
	if (nullptr != m_pState)
	{
		strOutStatus = "Valtan canonical Product read admission is already held.";
		return false;
	}
	std::unique_ptr<VALTAN_CANONICAL_PRODUCT_READ_STATE> State =
		std::make_unique<VALTAN_CANONICAL_PRODUCT_READ_STATE>();
	State->ProjectRoot = CProjectDataRoot::Get().parent_path();
	std::string AdmissionError;
	if (!State->Lock.Try_Acquire(State->ProjectRoot, AdmissionError) ||
		!Require_NoActiveValtanPatternGeneration(
			State->ProjectRoot, AdmissionError))
	{
		strOutStatus =
			"Valtan canonical Product read admission failed: " +
			AdmissionError;
		return false;
	}
	m_pState = State.release();
	strOutStatus =
		"Valtan canonical Product generation admitted for a shared read.";
	return true;
}

bool_t Client::CValtanCanonicalProductReadAdmission::Validate_StillCurrent(
	std::string& strOutStatus) const
{
	const auto* const pState =
		static_cast<const VALTAN_CANONICAL_PRODUCT_READ_STATE*>(m_pState);
	if (nullptr == pState)
	{
		strOutStatus = "Valtan canonical Product read admission is not held.";
		return false;
	}
	std::string AdmissionError;
	if (!Require_NoActiveValtanPatternGeneration(
			pState->ProjectRoot, AdmissionError))
	{
		strOutStatus =
			"Valtan canonical Product read admission became stale: " +
			AdmissionError;
		return false;
	}
	strOutStatus =
		"Valtan canonical Product generation remained stable through the read.";
	return true;
}

size_t Client::VALTAN_PATTERN_TREE_VIEW::Get_StageCount() const
{
	size_t iCount = 0u;
	for (const VALTAN_PATTERN_VIEW& Pattern : Gimmicks)
		iCount += Pattern.Stages.size();
	for (const VALTAN_PATTERN_VIEW& Pattern : Rotation)
		iCount += Pattern.Stages.size();
	return iCount;
}

size_t Client::VALTAN_PATTERN_TREE_VIEW::Get_EffectCount() const
{
	size_t iCount = 0u;
	const auto Count = [&iCount](const std::vector<VALTAN_PATTERN_VIEW>& Group)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Group)
		{
			iCount += static_cast<size_t>(std::count_if(
				Pattern.Stages.begin(), Pattern.Stages.end(),
				[](const VALTAN_STAGE_VIEW& Stage)
				{
					return Stage.Has_Effect();
				}));
		}
	};
	Count(Gimmicks);
	Count(Rotation);
	return iCount;
}

size_t Client::VALTAN_PATTERN_TREE_VIEW::Get_EffectDocumentCount() const
{
	size_t iCount = 0u;
	const auto Count = [&iCount](const std::vector<VALTAN_PATTERN_VIEW>& Group)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Group)
		{
			for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
			{
				iCount += Stage.Effects.size();
				iCount += Stage.CombatObjectEffects.size();
			}
		}
	};
	Count(Gimmicks);
	Count(Rotation);
	return iCount;
}

size_t Client::VALTAN_PATTERN_TREE_VIEW::Get_ClipBoundStageCount() const
{
	size_t iCount = 0u;
	const auto Count = [&iCount](const std::vector<VALTAN_PATTERN_VIEW>& Group)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Group)
		{
			iCount += static_cast<size_t>(std::count_if(
				Pattern.Stages.begin(), Pattern.Stages.end(),
				[](const VALTAN_STAGE_VIEW& Stage)
				{
					return Stage.Has_ClipBinding();
				}));
		}
	};
	Count(Gimmicks);
	Count(Rotation);
	return iCount;
}

size_t Client::VALTAN_PATTERN_TREE_VIEW::Get_ProductCueStageCount() const
{
	size_t iCount = 0u;
	const auto Count = [&iCount](const std::vector<VALTAN_PATTERN_VIEW>& Group)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Group)
		{
			iCount += static_cast<size_t>(std::count_if(
				Pattern.Stages.begin(), Pattern.Stages.end(),
				[](const VALTAN_STAGE_VIEW& Stage)
				{
					return Stage.Has_ProductCue();
				}));
		}
	};
	Count(Gimmicks);
	Count(Rotation);
	return iCount;
}

size_t Client::VALTAN_PATTERN_TREE_VIEW::Get_ClipOccurrenceCount() const
{
	size_t iCount = 0u;
	const auto Count = [&iCount](const std::vector<VALTAN_PATTERN_VIEW>& Group)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Group)
		{
			for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
				iCount += Stage.ClipOccurrences.size();
		}
	};
	Count(Gimmicks);
	Count(Rotation);
	return iCount;
}

size_t Client::VALTAN_PATTERN_TREE_VIEW::Get_ProductCueCount() const
{
	size_t iCount = 0u;
	const auto Count = [&iCount](const std::vector<VALTAN_PATTERN_VIEW>& Group)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Group)
		{
			for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
				iCount += Stage.ProductCues.size();
		}
	};
	Count(Gimmicks);
	Count(Rotation);
	return iCount;
}

size_t Client::VALTAN_PATTERN_TREE_VIEW::Get_CombatObjectEffectCount() const
{
	size_t iCount = 0u;
	const auto Count = [&iCount](const std::vector<VALTAN_PATTERN_VIEW>& Group)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Group)
		{
			for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
				iCount += Stage.CombatObjectEffects.size();
		}
	};
	Count(Gimmicks);
	Count(Rotation);
	return iCount;
}

bool_t Client::VALTAN_TOOL_AUDITION_INVENTORY::Contains(
	const std::string& strPatternId) const
{
	if (strPatternId.empty())
		return false;
	const auto ContainsIn = [&strPatternId](
		const std::vector<std::string>& PatternIds)
	{
		return PatternIds.end() != std::find(
			PatternIds.begin(), PatternIds.end(), strPatternId);
	};
	return ContainsIn(CorePatternIds) || ContainsIn(AnimatorPatternIds) ||
		ContainsIn(DerivedPatternIds);
}

bool_t Client::CValtanPatternTree::Build_PlayablePatternInventory(
	const VALTAN_PATTERN_TREE_VIEW& View,
	VALTAN_TOOL_AUDITION_INVENTORY& OutInventory,
	std::string& strOutError)
{
	std::map<std::string, const VALTAN_MANUAL_AUDITION_VIEW*, std::less<>> ManualByPattern;
	std::set<std::string, std::less<>> ManualSourceChains;
	for (const VALTAN_MANUAL_AUDITION_VIEW& Manual : View.ManualAuditions)
	{
		const VALTAN_PATTERN_VIEW* pPattern = Find_Pattern(View, Manual.strPatternId);
		if (!Is_StableToken(Manual.strPatternId) || !Is_StableToken(Manual.strSourceChainId) ||
			0u == Manual.iAuthoringPhase || Manual.iAuthoringPhase > 3u ||
			("MANUAL_SERVER_AUDITION" != Manual.strAdmissionState &&
			 "DERIVED_SERVER_PATTERN" != Manual.strAdmissionState) ||
			!ManualByPattern.emplace(Manual.strPatternId, &Manual).second ||
			!ManualSourceChains.insert(Manual.strSourceChainId).second ||
			nullptr == pPattern || !pPattern->bAuthoringMasterManaged ||
			!pPattern->bManualServerAudition || "AUDITION_ONLY" != pPattern->strSelectionMode ||
			pPattern->strSourceAnimationChainId != Manual.strSourceChainId ||
			pPattern->iAuthoringPhase != Manual.iAuthoringPhase ||
			pPattern->strAdmissionState != Manual.strAdmissionState)
		{
			strOutError = "Playable Pattern manual owner is missing, invalid, or duplicated: " +
				Manual.strPatternId;
			return false;
		}
	}

	std::vector<const VALTAN_PATTERN_VIEW*> StagedPatterns;
	std::map<std::string, size_t, std::less<>> IdentityCounts;
	for (const auto* pPatterns : { &View.Gimmicks, &View.Rotation })
		for (const VALTAN_PATTERN_VIEW& Pattern : *pPatterns)
			++IdentityCounts[Pattern.strPatternId];
	for (const auto* pPatterns : { &View.Gimmicks, &View.Rotation })
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : *pPatterns)
		{
			if (!Pattern.bAuthoringMasterManaged)
				continue;
			if (!Is_StableToken(Pattern.strPatternId) ||
				1u != IdentityCounts.at(Pattern.strPatternId) ||
				Find_Pattern(View, Pattern.strPatternId) != &Pattern ||
				Pattern.Stages.empty() || !Is_StableToken(Pattern.strEntryActionId) ||
				Pattern.strEntryActionId != Pattern.Stages.front().strActionId)
			{
				strOutError = "Playable Pattern split-owned identity or entry stage is invalid: " + Pattern.strPatternId;
				return false;
			}
			if (Pattern.bManualServerAudition != ManualByPattern.contains(Pattern.strPatternId) ||
				(!Pattern.bManualServerAudition &&
				 (!Pattern.strAdmissionState.empty() || !Pattern.strSourceAnimationChainId.empty() ||
				  0u != Pattern.iAuthoringPhase)))
			{
				strOutError = "Playable Pattern manual metadata has no matching owner: " + Pattern.strPatternId;
				return false;
			}
			std::set<std::string, std::less<>> StageIds;
			std::set<std::string, std::less<>> ActionIds;
			for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
			{
				if (!Is_StableToken(Stage.strStageId) || !Is_StableToken(Stage.strActionId) ||
					!StageIds.insert(Stage.strStageId).second ||
					!ActionIds.insert(Stage.strActionId).second)
				{
					strOutError = "Playable Pattern stage identity is invalid or duplicated: " + Pattern.strPatternId;
					return false;
				}
			}
			StagedPatterns.push_back(&Pattern);
		}
	}
	if (StagedPatterns.empty())
	{
		strOutError = "Playable Pattern requires at least one strictly joined split-owned Product pattern.";
		return false;
	}
	std::sort(StagedPatterns.begin(), StagedPatterns.end(),
		[](const VALTAN_PATTERN_VIEW* Left, const VALTAN_PATTERN_VIEW* Right)
		{
			return Left->iSourceSequenceIndex != Right->iSourceSequenceIndex ?
				Left->iSourceSequenceIndex < Right->iSourceSequenceIndex :
				Left->strPatternId < Right->strPatternId;
		});
	VALTAN_TOOL_AUDITION_INVENTORY Staged;
	for (const VALTAN_PATTERN_VIEW* pPattern : StagedPatterns)
		if (!pPattern->bManualServerAudition)
			Staged.CorePatternIds.push_back(pPattern->strPatternId);
	/* Preserve the authored manual display order independently of Next's
	   source-sequence ordering. Membership was validated in both directions. */
	for (const VALTAN_MANUAL_AUDITION_VIEW& Manual : View.ManualAuditions)
		("DERIVED_SERVER_PATTERN" == Manual.strAdmissionState ?
			Staged.DerivedPatternIds : Staged.AnimatorPatternIds).push_back(Manual.strPatternId);
	OutInventory = std::move(Staged);
	strOutError.clear();
	return true;
}

bool_t Client::CValtanPatternTree::Build_NextPatternInventory(
	const VALTAN_PATTERN_TREE_VIEW& View,
	std::vector<std::string>& OutPatternIds,
	std::string& strOutError)
{
	VALTAN_TOOL_AUDITION_INVENTORY Inventory;
	if (!Build_PlayablePatternInventory(View, Inventory, strOutError))
		return false;
	std::vector<std::string> StagedIds;
	StagedIds.reserve(Inventory.Get_PatternCount());
	for (const auto* pIds : { &Inventory.CorePatternIds, &Inventory.AnimatorPatternIds,
		&Inventory.DerivedPatternIds })
		StagedIds.insert(StagedIds.end(), pIds->begin(), pIds->end());
	std::sort(StagedIds.begin(), StagedIds.end(),
		[&View](const std::string& LeftId, const std::string& RightId)
		{
			const VALTAN_PATTERN_VIEW* Left = Find_Pattern(View, LeftId);
			const VALTAN_PATTERN_VIEW* Right = Find_Pattern(View, RightId);
			return Left->iSourceSequenceIndex != Right->iSourceSequenceIndex ?
				Left->iSourceSequenceIndex < Right->iSourceSequenceIndex : LeftId < RightId;
		});
	OutPatternIds = std::move(StagedIds);
	strOutError.clear();
	return true;
}

std::string Client::CValtanPatternTree::Build_PatternIdentitySummary(
	const VALTAN_PATTERN_VIEW& Pattern)
{
	std::ostringstream Summary;
	Summary.imbue(std::locale::classic());
	Summary << std::fixed << std::setprecision(3)
		<< "Pattern ID: " << Pattern.strPatternId << "\nAuthoring: ";
	if ("DERIVED_SERVER_PATTERN" == Pattern.strAdmissionState)
		Summary << "Derived server";
	else if (Pattern.bManualServerAudition)
		Summary << "Phase " << Pattern.iAuthoringPhase << " manual audition";
	else if (Pattern.bAuthoringMasterManaged)
		Summary << "Core server";
	else
		Summary << "outside shared audition inventory";
	Summary << " | Runtime eligibility: phase " << Pattern.iMinimumPhase;
	if (Pattern.iMinimumPhase != Pattern.iMaximumPhase)
		Summary << '-' << Pattern.iMaximumPhase;
	Summary << "\nClips (authored order):";
	if (Pattern.Stages.empty())
		Summary << " none";
	for (size_t iStage = 0u; iStage < Pattern.Stages.size(); ++iStage)
	{
		const VALTAN_STAGE_VIEW& Stage = Pattern.Stages[iStage];
		Summary << '\n' << iStage + 1u << ". " << Stage.strStageId
			<< " [wall " << Stage.iDurationMs / 1000.0 << "s";
		if (Stage.iAuthoringRepeatCount > 1u)
			Summary << "; repeats " << Stage.iAuthoringRepeatCount;
		Summary << ']';
		if (Stage.bSuppressAnimation)
		{
			Summary << ": animation suppressed";
			continue;
		}
		if (Stage.ClipOccurrences.empty())
		{
			Summary << ": no authored clip occurrence";
			continue;
		}
		for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip : Stage.ClipOccurrences)
		{
			Summary << "\n  " << Clip.strClipName
				<< " [source start " << Clip.iSourceStartMs / 1000.0 << "s; ";
			if (0u != Clip.iPlayMs)
				Summary << "source duration " << Clip.iPlayMs / 1000.0 << 's';
			else
				Summary << "natural end";
			Summary << "; rate " << Clip.fPlayRate << "x; wall ";
			if (0u != Clip.iAuthoringWallMs)
				Summary << Clip.iAuthoringWallMs / 1000.0 << 's';
			else
				Summary << "unresolved";
			if (Clip.bLoop)
				Summary << "; loop";
			Summary << ']';
		}
	}
	return Summary.str();
}

std::string Client::CValtanPatternTree::Build_StageEffectAssetId(
	const std::string& strPatternActionId,
	const VALTAN_STAGE_VIEW& Stage)
{
	if (strPatternActionId.empty() || Stage.strActionId.empty())
		return {};
	/* valtan.attack.whirlwind -> whirlwind; the category is what the tree
	   already shows, so it is not repeated inside the asset id. */
	std::string strPattern = strPatternActionId;
	size_t iCut = strPattern.find('.');
	if (std::string::npos != iCut)
	{
		iCut = strPattern.find('.', iCut + 1u);
		if (std::string::npos != iCut)
			strPattern = strPattern.substr(iCut + 1u);
	}
	const std::string strPrefix = strPatternActionId + ".";
	std::string strStage;
	if (Stage.strActionId.starts_with(strPrefix) &&
		Stage.strActionId.size() > strPrefix.size())
	{
		strStage = Stage.strActionId.substr(strPrefix.size());
	}
	else
	{
		strStage = Stage.strStageId;
		std::transform(strStage.begin(), strStage.end(), strStage.begin(),
			[](const unsigned char Character)
			{
				return '_' == Character ? '-' :
					static_cast<char>(std::tolower(Character));
			});
	}
	if (strPattern.empty() || strStage.empty())
		return {};
	return "effect.valtan." + strPattern + "." + strStage;
}

bool_t Client::CValtanPatternTree::Build_PreviewStagePath(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_PATTERN_PREVIEW_PATH ePath,
	std::vector<const VALTAN_STAGE_VIEW*>& OutStages,
	std::string& strOutStatus)
{
	if (VALTAN_PATTERN_PREVIEW_PATH::END == ePath || Pattern.Stages.empty())
	{
		strOutStatus = "Valtan preview path request is invalid.";
		return false;
	}

	std::map<std::string, size_t, std::less<>> StageByAction;
	for (size_t i = 0u; i < Pattern.Stages.size(); ++i)
	{
		const VALTAN_STAGE_VIEW& Stage = Pattern.Stages[i];
		if (Stage.strActionId.empty() ||
			!StageByAction.emplace(Stage.strActionId, i).second)
		{
			strOutStatus = "Valtan preview path has a missing or duplicated action: " +
				Stage.strActionId;
			return false;
		}
		std::set<std::string, std::less<>> Outcomes;
		for (const VALTAN_STAGE_BRANCH_VIEW& Branch : Stage.Branches)
		{
			if (Branch.strOutcome.empty() ||
				!Outcomes.insert(Branch.strOutcome).second)
			{
				strOutStatus = "Valtan preview path has ambiguous branches: " +
					Stage.strActionId;
				return false;
			}
		}
	}
	for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
	{
		for (const VALTAN_STAGE_BRANCH_VIEW& Branch : Stage.Branches)
		{
			if (Branch.strNextActionId.has_value() &&
				!StageByAction.contains(*Branch.strNextActionId))
			{
				strOutStatus =
					"Valtan preview path branches to an unknown action: " +
					*Branch.strNextActionId;
				return false;
			}
		}
	}

	std::vector<const VALTAN_STAGE_VIEW*> StagedPath;
	std::set<std::string, std::less<>> VisitedActions;
	size_t iStage = 0u;
	bool_t bWallContactTaken = false;
	bool_t bCounterHitTaken = false;
	for (;;)
	{
		const VALTAN_STAGE_VIEW& Stage = Pattern.Stages[iStage];
		if (!VisitedActions.insert(Stage.strActionId).second)
		{
			strOutStatus = "Valtan preview path contains a cycle: " +
				Stage.strActionId;
			return false;
		}
		StagedPath.push_back(&Stage);

		if (Stage.Branches.empty())
		{
			if (iStage + 1u == Pattern.Stages.size())
				break;
			++iStage;
			continue;
		}

		const VALTAN_STAGE_BRANCH_VIEW* pSelected = nullptr;
		const auto SelectOutcome = [&Stage, &pSelected](
			const std::string_view strOutcome) -> bool_t
		{
			for (const VALTAN_STAGE_BRANCH_VIEW& Branch : Stage.Branches)
			{
				if (Branch.strOutcome != strOutcome)
					continue;
				if (nullptr != pSelected)
					return false;
				pSelected = &Branch;
			}
			return true;
		};
		if (VALTAN_PATTERN_PREVIEW_PATH::COUNTER_GROGGY == ePath)
		{
			if (!SelectOutcome("COUNTER_HIT"))
			{
				strOutStatus =
					"Valtan preview path has ambiguous COUNTER_HIT edges.";
				return false;
			}
			bCounterHitTaken = bCounterHitTaken || nullptr != pSelected;
		}

		if (VALTAN_PATTERN_PREVIEW_PATH::PART_BREAK == ePath &&
			bWallContactTaken)
		{
			if (!SelectOutcome("PART_DESTROYED"))
			{
				strOutStatus =
					"Valtan preview path has ambiguous PART_DESTROYED edges.";
				return false;
			}
		}
		if (nullptr == pSelected &&
			(VALTAN_PATTERN_PREVIEW_PATH::WALL_GROGGY == ePath ||
			 VALTAN_PATTERN_PREVIEW_PATH::PART_BREAK == ePath) &&
			!bWallContactTaken)
		{
			if (!SelectOutcome("WALL_CONTACT"))
			{
				strOutStatus =
					"Valtan preview path has ambiguous WALL_CONTACT edges.";
				return false;
			}
			bWallContactTaken = nullptr != pSelected;
		}
		if (nullptr == pSelected && !SelectOutcome("TIMEOUT"))
		{
			strOutStatus =
				"Valtan preview path has ambiguous TIMEOUT edges.";
			return false;
		}
		if (nullptr == pSelected)
		{
			strOutStatus = "Valtan preview path has no policy-selectable edge: " +
				Stage.strActionId;
			return false;
		}
		if (!pSelected->strNextActionId.has_value())
			break;
		const auto Next = StageByAction.find(*pSelected->strNextActionId);
		if (Next == StageByAction.end())
		{
			strOutStatus = "Valtan preview path branches to an unknown action: " +
				*pSelected->strNextActionId;
			return false;
		}
		iStage = Next->second;
	}

	if (VALTAN_PATTERN_PREVIEW_PATH::COUNTER_GROGGY == ePath &&
		!bCounterHitTaken)
	{
		strOutStatus =
			"Valtan Counter/Groggy preview path has no COUNTER_HIT branch.";
		return false;
	}
	OutStages = std::move(StagedPath);
	strOutStatus = "Valtan preview path resolved " +
		std::to_string(OutStages.size()) + " stages.";
	return true;
}

bool_t Client::CValtanPatternTree::Load(
	VALTAN_PATTERN_TREE_VIEW& OutView,
	std::string& strOutStatus)
{
	CValtanCanonicalProductReadAdmission Admission;
	if (!Admission.Acquire(strOutStatus))
		return false;
	return Load_WhileAdmitted(Admission, OutView, strOutStatus);
}

bool_t Client::CValtanPatternTree::Load_WhileAdmitted(
	const CValtanCanonicalProductReadAdmission& Admission,
	VALTAN_PATTERN_TREE_VIEW& OutView,
	std::string& strOutStatus)
{
	std::string AdmissionStatus;
	if (!Admission.Is_Acquired() ||
		!Admission.Validate_StillCurrent(AdmissionStatus))
	{
		strOutStatus =
			"Valtan canonical graph read admission failed: " +
			AdmissionStatus;
		return false;
	}

	VALTAN_PATTERN_TREE_VIEW StagedView;
	if (!Load_FromAuthoringPaths(
		CProjectDataRoot::Resolve(
			std::filesystem::path(L"Valtan") / L"Valtan.gameplay.json"),
		CProjectDataRoot::Resolve(
			std::filesystem::path(L"Valtan") / L"Valtan.presentation.json"),
		StagedView,
		strOutStatus,
		VALTAN_PATTERN_TREE_LOAD_POLICY::REQUIRE_ACTIVE_PRODUCT_PARITY))
	{
		return false;
	}
	/* The post-join check makes a non-cooperating journal writer visible while
	   preserving the caller's previously admitted view. Cooperating writers
	   cannot reach this point because the shared byte-range lock is still held. */
	if (!Admission.Validate_StillCurrent(AdmissionStatus))
	{
		strOutStatus =
			"Valtan canonical graph read admission failed: " +
			AdmissionStatus;
		return false;
	}

	OutView = std::move(StagedView);
	return true;
}

bool_t Client::CValtanPatternTree::Load_FromAuthoringPaths(
	const std::filesystem::path& GameplayPath,
	const std::filesystem::path& PresentationPath,
	VALTAN_PATTERN_TREE_VIEW& OutView,
	std::string& strOutStatus,
	const VALTAN_PATTERN_TREE_LOAD_POLICY ePolicy)
{
	if (VALTAN_PATTERN_TREE_LOAD_POLICY::REQUIRE_ACTIVE_PRODUCT_PARITY !=
			ePolicy &&
		VALTAN_PATTERN_TREE_LOAD_POLICY::RESTORE_AUTHORING_SNAPSHOT != ePolicy)
	{
		strOutStatus = "Valtan pattern-tree load policy is invalid.";
		return false;
	}
	DATA_JSON_VALUE Encounter;
	DATA_JSON_VALUE GameplayRoot;
	DATA_JSON_VALUE PresentationRoot;
	DATA_JSON_VALUE PatternRotations;
	std::string Error;
	const std::filesystem::path EncounterRelative =
		std::filesystem::path(L"Encounters") / L"Valtan" /
		L"ValtanEncounter.json";
	const std::filesystem::path BindingRelative =
		std::filesystem::path(L"Animation") / L"Authored" / L"Valtan" /
		L"Valtan.patternbindings.json";
	if (!Parse_Document(EncounterRelative, Encounter, Error))
	{
		strOutStatus = "Valtan encounter load failed: " + Error;
		return false;
	}
	if (!Parse_DocumentPath(GameplayPath, GameplayRoot, Error))
	{
		strOutStatus = "Valtan split gameplay authoring load failed: " + Error;
		return false;
	}
	if (!Parse_DocumentPath(PresentationPath, PresentationRoot, Error))
	{
		strOutStatus =
			"Valtan split presentation authoring load failed: " + Error;
		return false;
	}
	if (!Parse_Document(std::filesystem::path(L"Encounters") / L"Valtan" /
			L"ValtanPatternRotations.json", PatternRotations, Error))
	{
		strOutStatus = "Valtan normal-selection product load failed: " + Error;
		return false;
	}
	MASTER_DOCUMENT MasterDocument;
	if (!Parse_SplitMasterDocument(
			GameplayRoot, PresentationRoot, MasterDocument, Error))
	{
		strOutStatus = "Valtan split authoring strict join failed: " + Error;
		return false;
	}

	/* The tree consumes the same typed, fail-closed documents as Product
	   runtime.  It does not maintain a second permissive JSON interpretation. */
	std::string BindingText;
	BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT BindingDocument;
	if (!Read_TextDocument(CProjectDataRoot::Resolve(BindingRelative),
			BindingText, Error) ||
		!CValtanPatternAnimationBindingDocument::Parse_Text(
			BindingText, BindingDocument, Error))
	{
		strOutStatus = "Valtan pattern bindings load failed: " + Error;
		return false;
	}
	std::vector<std::string> DeclaredClipNames;
	for (const BOSS_PATTERN_ANIMATION_BINDING& Binding :
		BindingDocument.Bindings)
	{
		for (const BOSS_PATTERN_ANIMATION_CLIP& Clip : Binding.Clips)
			DeclaredClipNames.push_back(Clip.strClipName);
	}
	if (!CValtanPatternAnimationBindingDocument::Validate(
			BindingDocument, "BOSS_VALTAN", DeclaredClipNames, Error))
	{
		strOutStatus = "Valtan pattern bindings validation failed: " + Error;
		return false;
	}

	VALTAN_PATTERN_EFFECT_CUE_DOCUMENT CueDocument;
	if (!CValtanPatternEffectCueDocument::Load_ReadOnlyProduct(CueDocument, Error))
	{
		strOutStatus = "Valtan Product Effect cue load failed: " + Error;
		return false;
	}

	/* Moving pattern visuals are owned by Server combat objects rather than
	   boss-root cues.  Join the encounter spawn action, combat-object owner,
	   and BossCatalog visual explicitly so the authoring tree cannot hide the
	   document under an unrelated "unmapped" bucket or double-own it. */
	DATA_JSON_VALUE BossCatalog;
	DATA_JSON_VALUE CombatObjects;
	if (!Parse_Document(std::filesystem::path(L"Actors") /
			L"BossCatalog.json", BossCatalog, Error) ||
		!Parse_Document(std::filesystem::path(L"Encounters") / L"Valtan" /
			L"ValtanCombatObjects.json", CombatObjects, Error))
	{
		strOutStatus = "Valtan combat-object authoring join failed: " + Error;
		return false;
	}
	std::map<std::string, COMBAT_OBJECT_EFFECT_REFERENCE, std::less<>>
		CombatObjectEffectsByArchetype;
	const DATA_JSON_VALUE* pBosses = BossCatalog.Find("bosses");
	const DATA_JSON_VALUE* pObjects = CombatObjects.Find("objects");
	if (nullptr == pBosses || !pBosses->Is_Array() ||
		nullptr == pObjects || !pObjects->Is_Array())
	{
		strOutStatus =
			"Valtan combat-object authoring documents have no rows.";
		return false;
	}
	const DATA_JSON_VALUE* pValtanBoss = nullptr;
	std::set<std::string, std::less<>> BossArchetypeIds;
	for (const DATA_JSON_VALUE& Boss : pBosses->Get_Array())
	{
		const std::string strBossArchetypeId = Boss.Is_Object() ?
			Read_String(Boss, "archetypeId") : std::string{};
		if (!Is_StableToken(strBossArchetypeId) ||
			!BossArchetypeIds.insert(strBossArchetypeId).second)
		{
			strOutStatus = "BossCatalog archetype identity is invalid or duplicated.";
			return false;
		}
		if (strBossArchetypeId == "BOSS_VALTAN")
		{
			pValtanBoss = &Boss;
		}
	}
	const DATA_JSON_VALUE* pVisuals = nullptr == pValtanBoss ? nullptr :
		pValtanBoss->Find("combatObjectVisuals");
	if (nullptr == pVisuals || !pVisuals->Is_Array() ||
		pVisuals->Get_Array().empty())
	{
		strOutStatus = "BOSS_VALTAN has no combat-object Effect visuals.";
		return false;
	}
	for (const DATA_JSON_VALUE& Visual : pVisuals->Get_Array())
	{
		if (!Visual.Is_Object())
		{
			strOutStatus = "BOSS_VALTAN combat-object visual row is invalid.";
			return false;
		}
		const std::string strArchetypeId = Read_String(
			Visual, "combatObjectArchetypeId");
		COMBAT_OBJECT_EFFECT_REFERENCE Reference;
		Reference.strClientVisualId = Read_String(Visual, "clientVisualId");
		Reference.strEffectAssetId = Read_String(Visual, "effectAssetId");
		if (strArchetypeId.empty() || Reference.strClientVisualId.empty() ||
			Reference.strEffectAssetId.empty() ||
			!CombatObjectEffectsByArchetype.emplace(
				strArchetypeId, std::move(Reference)).second)
		{
			strOutStatus =
				"BOSS_VALTAN combat-object visual identity is invalid or duplicated.";
			return false;
		}
	}
	std::set<std::string, std::less<>> DescribedCombatObjects;
	for (const DATA_JSON_VALUE& Object : pObjects->Get_Array())
	{
		if (!Object.Is_Object())
			continue;
		const std::string strArchetypeId = Read_String(
			Object, "combatObjectArchetypeId");
		auto Reference = CombatObjectEffectsByArchetype.find(strArchetypeId);
		if (Reference == CombatObjectEffectsByArchetype.end())
			continue;
		const std::string strClientVisualId = Read_String(
			Object, "clientVisualId");
		Reference->second.strOwnerPatternId = Read_String(
			Object, "ownerPatternId");
		Reference->second.strOwnerStageActionId = Read_String(
			Object, "ownerStageActionId");
		Reference->second.strKind = Read_String(Object, "kind");
		Reference->second.strOriginPolicy = Read_String(
			Object, "originPolicy");
		Reference->second.strDirectionPolicy = Read_String(
			Object, "directionPolicy");
		if (!Read_RequiredFiniteFloat(
				Object, "speedMps", Reference->second.fSpeedMps) ||
			!Read_RequiredFiniteFloat(Object, "maximumDistanceM",
				Reference->second.fMaximumDistanceM) ||
			!Read_RequiredUInt32(
				Object, "lifeMs", Reference->second.iLifetimeMs) ||
			Reference->second.strKind.empty() ||
			Reference->second.strOriginPolicy.empty() ||
			Reference->second.strDirectionPolicy.empty() ||
			Reference->second.fSpeedMps < 0.f ||
			Reference->second.fMaximumDistanceM < 0.f ||
			0u == Reference->second.iLifetimeMs)
		{
			strOutStatus =
				"Valtan combat-object Product motion/timing is invalid: " +
				strArchetypeId;
			return false;
		}
		const DATA_JSON_VALUE* pHits = Object.Find("hits");
		const DATA_JSON_VALUE* pPresentationEvents =
			Object.Find("presentationEvents");
		if (nullptr == pHits || !pHits->Is_Array() ||
			(nullptr != pPresentationEvents && !pPresentationEvents->Is_Array()))
		{
			strOutStatus =
				"Valtan combat-object has no Server hit or presentation rows: " +
				strArchetypeId;
			return false;
		}
		const size_t iPresentationEventCount =
			nullptr == pPresentationEvents ? 0u :
			pPresentationEvents->Get_Array().size();
		const size_t iEventRowCount =
			pHits->Get_Array().size() + iPresentationEventCount;
		if (0u == iEventRowCount || iEventRowCount > 16u)
		{
			strOutStatus =
				"Valtan combat-object Server hit/presentation row count is invalid: " +
				strArchetypeId;
			return false;
		}
		for (const DATA_JSON_VALUE& Hit : pHits->Get_Array())
		{
			uint32_t iHitOffsetMs = 0u;
			const std::string strHitId = Read_String(Hit, "hitId");
			if (!Hit.Is_Object() ||
				strHitId.empty() ||
				!Read_RequiredUInt32(Hit, "atMs", iHitOffsetMs) ||
				std::find(Reference->second.HitIds.begin(),
					Reference->second.HitIds.end(), strHitId) !=
					Reference->second.HitIds.end())
			{
				strOutStatus =
					"Valtan combat-object Server hit identity or clock is invalid: " +
					strArchetypeId;
				return false;
			}
			Reference->second.HitIds.push_back(strHitId);
			Reference->second.HitOffsetsMs.push_back(iHitOffsetMs);
		}
		if (nullptr != pPresentationEvents)
		{
			std::set<std::string, std::less<>> PresentationEventIds;
			for (const DATA_JSON_VALUE& Event :
				pPresentationEvents->Get_Array())
			{
				uint32_t iEventOffsetMs = 0u;
				const std::string strEventId = Read_String(
					Event, "presentationEventId");
				if (!Event.Is_Object() ||
					!Is_StableToken(strEventId) ||
					!Read_RequiredUInt32(Event, "atMs", iEventOffsetMs) ||
					iEventOffsetMs > Reference->second.iLifetimeMs ||
					!PresentationEventIds.insert(strEventId).second ||
					std::find(Reference->second.HitIds.begin(),
						Reference->second.HitIds.end(), strEventId) !=
						Reference->second.HitIds.end())
				{
					strOutStatus =
						"Valtan combat-object presentation identity or clock is invalid: " +
						strArchetypeId;
					return false;
				}
				Client::VALTAN_COMBAT_OBJECT_PRESENTATION_EVENT_VIEW View;
				View.strPresentationEventId = strEventId;
				View.iAtMs = iEventOffsetMs;
				Reference->second.PresentationEvents.push_back(std::move(View));
			}
		}
		if (strClientVisualId != Reference->second.strClientVisualId ||
			Reference->second.strOwnerPatternId.empty() ||
			Reference->second.strOwnerStageActionId.empty() ||
			!DescribedCombatObjects.insert(strArchetypeId).second)
		{
			strOutStatus =
				"Valtan combat-object owner or visual identity changed: " +
				strArchetypeId;
			return false;
		}
	}
	if (DescribedCombatObjects.size() !=
		CombatObjectEffectsByArchetype.size())
	{
		strOutStatus =
			"BossCatalog combat-object Effect visual has no Valtan owner row.";
		return false;
	}
	for (VALTAN_INDEPENDENT_EFFECT_VIEW& Independent :
		MasterDocument.IndependentEffects)
	{
		if ("SERVER_COMBAT_OBJECT" != Independent.strOwnership)
			continue;
		const auto Product = CombatObjectEffectsByArchetype.find(
			Independent.strCombatObjectArchetypeId);
		if (Product == CombatObjectEffectsByArchetype.end() ||
			Product->second.strOwnerPatternId != Independent.strOwnerPatternId ||
			Product->second.strOwnerStageActionId.empty())
		{
			strOutStatus =
				"Valtan split combat-object independent Effect owner changed: " +
				Independent.strIndependentEffectId;
			return false;
		}
		Independent.strClientVisualId = Product->second.strClientVisualId;
		Independent.strEffectAssetId = Product->second.strEffectAssetId;
	}

	/* Effect bindings are optional: a freshly seeded stage document is not in
	   patterneffects.json yet, and that must not fail the whole tree. */
	std::map<std::string, std::pair<std::string, std::string>, std::less<>>
		EffectByAction;
	DATA_JSON_VALUE Effects;
	if (Parse_Document(std::filesystem::path(L"Animation") / L"Authored" /
			L"Valtan" / L"Valtan.patterneffects.json", Effects, Error))
	{
		const DATA_JSON_VALUE* pRows = Effects.Find("bindings");
		if (nullptr != pRows && pRows->Is_Array())
		{
			for (const DATA_JSON_VALUE& Row : pRows->Get_Array())
			{
				if (!Row.Is_Object())
					continue;
				const std::string strAction = Read_String(Row, "actionId");
				if (strAction.empty())
					continue;
				EffectByAction[strAction] = {
					Read_String(Row, "effectAssetId"),
					Read_String(Row, "effectDocument") };
			}
		}
	}

	std::map<std::string, std::vector<VALTAN_CLIP_OCCURRENCE_VIEW>, std::less<>>
		ClipsByAction;
	std::set<std::string, std::less<>> SuppressedAnimationActions;
	for (const BOSS_PATTERN_ANIMATION_BINDING& Binding :
		BindingDocument.Bindings)
	{
		std::vector<VALTAN_CLIP_OCCURRENCE_VIEW> Occurrences;
		Occurrences.reserve(Binding.Clips.size());
		for (const BOSS_PATTERN_ANIMATION_CLIP& Clip : Binding.Clips)
			Occurrences.push_back(Build_ClipOccurrenceView(Clip));
		ClipsByAction.emplace(Binding.strActionId, std::move(Occurrences));
		if (Binding.bSuppressAnimation)
			SuppressedAnimationActions.insert(Binding.strActionId);
	}

	std::map<std::string, std::vector<VALTAN_PRODUCT_EFFECT_CUE_VIEW>,
		std::less<>>
		CueByAction;
	const size_t iCueCount = CueDocument.Cues.size();
	for (const VALTAN_PATTERN_EFFECT_CUE& SourceCue : CueDocument.Cues)
	{
		VALTAN_PRODUCT_EFFECT_CUE_VIEW Cue;
		Cue.strBindingId = SourceCue.strBindingId;
		Cue.strOccurrenceId = SourceCue.strOccurrenceId;
		Cue.strPatternId = SourceCue.strPatternId;
		Cue.strStageId = SourceCue.strStageId;
		Cue.strActionId = SourceCue.strActionId;
		Cue.strClipOccurrenceId = SourceCue.strClipOccurrenceId;
		Cue.strEffectAssetId = SourceCue.strEffectAssetId;
		Cue.strV1EffectAssetId = SourceCue.strV1EffectAssetId;
		Cue.strAnchorSlotId = SourceCue.strAnchorSlotId;
		Cue.LocalTransform = SourceCue.LocalTransform;
		Cue.eFollowPolicy = SourceCue.eFollowPolicy;
		Cue.eStopPolicy = SourceCue.eStopPolicy;
		Cue.strFollowPolicy = Describe_FollowPolicy(SourceCue.eFollowPolicy);
		Cue.strStopPolicy = Describe_StopPolicy(SourceCue.eStopPolicy);
		Cue.strRepeatPolicy = Describe_RepeatPolicy(SourceCue.eRepeatPolicy);
		Cue.eScalePolicy = SourceCue.eScalePolicy;
		Cue.strScalePolicy = Describe_ScalePolicy(SourceCue.eScalePolicy);
		Cue.vWorldScale = SourceCue.vWorldScale;
		Cue.bHasExplicitScalePolicy = SourceCue.bHasExplicitScalePolicy;
		Cue.bUsesStageClock = SourceCue.bUsesStageClock;
		Cue.iStageOffsetMs = SourceCue.bUsesStageClock ?
			SourceCue.iStartMs : 0u;
		Cue.iSourceStartMs = SourceCue.iStartMs;
		Cue.iSourceEndMs = SourceCue.iEndMs;
		Cue.iStageDurationMs = SourceCue.iStageDurationMs;
		Cue.bHasSourceEnd = SourceCue.bHasSourceEnd;
		CueByAction[Cue.strActionId].push_back(std::move(Cue));
	}

	const DATA_JSON_VALUE* pPatterns = Encounter.Find("patterns");
	if (nullptr == pPatterns || !pPatterns->Is_Array())
	{
		strOutStatus = "Valtan encounter has no patterns array.";
		return false;
	}

	VALTAN_PATTERN_TREE_VIEW Staged;
	size_t iResolvedCueCount = 0u;
	std::set<std::string, std::less<>> ResolvedCombatObjectEffects;
	const std::string strEncounterBossArchetypeId = Read_String(
		Encounter, "bossArchetypeId");
	if (!Is_StableToken(strEncounterBossArchetypeId) ||
		!BossArchetypeIds.contains(strEncounterBossArchetypeId))
	{
		strOutStatus = "Valtan encounter boss archetype reference is invalid.";
		return false;
	}
	for (const DATA_JSON_VALUE& PatternValue : pPatterns->Get_Array())
	{
		if (!PatternValue.Is_Object())
			continue;
		VALTAN_PATTERN_VIEW Pattern;
		Pattern.strPatternId = Read_String(PatternValue, "patternId");
		Pattern.strCategory = Read_String(PatternValue, "category");
		Pattern.strTargetPolicy = Read_String(PatternValue, "targetPolicy");
		Pattern.strAimPolicy = Read_String(PatternValue, "aimPolicy");
		Pattern.strDisplayName = Read_String(PatternValue, "displayName");
		Pattern.strActionId = Read_String(PatternValue, "actionId");
		Pattern.strSelectionMode = Read_String(PatternValue, "selectionMode");
		Pattern.strArmorRequirement = Read_String(
			PatternValue, "armorRequirement");
		Pattern.strPhaseRequirement = Read_String(
			PatternValue, "phaseRequirement");
		uint32_t iMinimumHealthBar = 0u;
		uint32_t iMaximumHealthBar = 0u;
		uint32_t iTriggerHealthBar = 0u;
		const DATA_JSON_VALUE* pSourceActionIds = Required(
			PatternValue, "sourceActionIds", DATA_JSON_TYPE::ARRAY);
		const DATA_JSON_VALUE* pInvulnerable = Required(
			PatternValue, "invulnerableWhileRunning", DATA_JSON_TYPE::BOOLEAN);
		if (Pattern.strPatternId.empty() || Pattern.strCategory.empty() ||
			Pattern.strTargetPolicy.empty() || Pattern.strAimPolicy.empty() ||
			Pattern.strDisplayName.empty() || Pattern.strActionId.empty() ||
			Pattern.strSelectionMode.empty() ||
			Pattern.strArmorRequirement.empty() ||
			Pattern.strPhaseRequirement.empty() || nullptr == pInvulnerable ||
			nullptr == pSourceActionIds || pSourceActionIds->Get_Array().empty() ||
			!Read_RequiredUInt32(
				PatternValue, "minimumPhase", Pattern.iMinimumPhase) ||
			!Read_RequiredUInt32(
				PatternValue, "maximumPhase", Pattern.iMaximumPhase) ||
			!Read_RequiredUInt32(
				PatternValue, "minimumHealthBar", iMinimumHealthBar) ||
			!Read_RequiredUInt32(
				PatternValue, "maximumHealthBar", iMaximumHealthBar) ||
			!Read_RequiredUInt32(
				PatternValue, "triggerHealthBar", iTriggerHealthBar) ||
			!Read_RequiredUInt32(
				PatternValue, "triggerOrder", Pattern.iTriggerOrder) ||
			!Read_RequiredUInt32(
				PatternValue, "selectionWeight", Pattern.iSelectionWeight) ||
			!Read_RequiredUInt32(PatternValue, "maximumConsecutiveUses",
				Pattern.iMaximumConsecutiveUses) ||
			!Read_RequiredFiniteFloat(
				PatternValue, "minimumRange", Pattern.fMinimumRange) ||
			!Read_RequiredFiniteFloat(
				PatternValue, "maximumRange", Pattern.fMaximumRange) ||
			!Read_PatternServerMotion(
				PatternValue.Find("serverMotion"), Pattern.ServerMotion) ||
			!Read_PatternFinale(PatternValue.Find("finale"), Pattern.Finale) ||
			(Pattern.Finale.has_value() &&
				(pInvulnerable->Get_Boolean() ||
				 Pattern.Finale->strGhostArchetypeId == strEncounterBossArchetypeId ||
				 !BossArchetypeIds.contains(Pattern.Finale->strGhostArchetypeId))))
		{
			strOutStatus =
				"Valtan encounter pattern gameplay projection is invalid: " +
				Pattern.strPatternId;
			return false;
		}
		Pattern.iMinimumHealthBar = static_cast<int32_t>(iMinimumHealthBar);
		Pattern.iMaximumHealthBar = static_cast<int32_t>(iMaximumHealthBar);
		Pattern.iTriggerHealthBar = static_cast<int32_t>(iTriggerHealthBar);
		Pattern.bInvulnerableWhileRunning = pInvulnerable->Get_Boolean();
		for (const DATA_JSON_VALUE& SourceAction :
			pSourceActionIds->Get_Array())
		{
			if (!Is_NonNegativeInteger(&SourceAction) ||
				0.0 == SourceAction.Get_Number())
			{
				strOutStatus = "Valtan encounter sourceActionIds is invalid: " +
					Pattern.strPatternId;
				return false;
			}
			Pattern.SourceActionIds.push_back(static_cast<uint32_t>(
				SourceAction.Get_Number()));
		}

		const DATA_JSON_VALUE* pStages = PatternValue.Find("stages");
		if (nullptr != pStages && pStages->Is_Array())
		{
			for (const DATA_JSON_VALUE& StageValue : pStages->Get_Array())
			{
				if (!StageValue.Is_Object())
					continue;
				VALTAN_STAGE_VIEW Stage;
				Stage.strStageId = Read_String(StageValue, "stageId");
				Stage.strActionId = Read_String(StageValue, "actionId");
				Stage.strStageKind = Read_String(StageValue, "stageKind");
				Stage.strHitShape = Read_String(StageValue, "hitShape");
				const bool_t bValidStageKind =
					"WINDUP" == Stage.strStageKind ||
					"ACTIVE" == Stage.strStageKind ||
					"RECOVERY" == Stage.strStageKind ||
					"GROGGY" == Stage.strStageKind ||
					"PART_BREAK" == Stage.strStageKind;
				const bool_t bValidHitShape =
					"NONE" == Stage.strHitShape ||
					"CIRCLE" == Stage.strHitShape ||
					"RING" == Stage.strHitShape ||
					"CONE" == Stage.strHitShape ||
					"BOX" == Stage.strHitShape ||
					"CROSS" == Stage.strHitShape ||
					"SIX_DIRECTIONS" == Stage.strHitShape;
				if (Stage.strStageId.empty() || Stage.strActionId.empty() ||
					!bValidStageKind || !bValidHitShape)
				{
					strOutStatus =
						"Valtan encounter stage identity is missing or invalid";
					return false;
				}
				if (!Read_RequiredUInt32(
						StageValue, "durationMs", Stage.iDurationMs) ||
					!Read_RequiredUInt32(
						StageValue, "hitCount", Stage.iHitCount) ||
					!Read_RequiredUInt32(
						StageValue, "hitIntervalMs", Stage.iHitIntervalMs) ||
					!Read_RequiredUInt32(
						StageValue, "hitDelayMs", Stage.iHitDelayMs) ||
					!Read_RequiredFiniteFloat(
						StageValue, "hitOuterRadius", Stage.fHitOuterRadius) ||
					!Read_RequiredFiniteFloat(
						StageValue, "hitInnerRadius", Stage.fHitInnerRadius) ||
					!Read_RequiredFiniteFloat(
						StageValue, "hitAngleDegrees", Stage.fHitAngleDegrees) ||
					!Read_RequiredFiniteFloat(
						StageValue, "hitLength", Stage.fHitLength) ||
					!Read_RequiredFiniteFloat(
						StageValue, "hitHalfWidth", Stage.fHitHalfWidth))
				{
					strOutStatus =
						"Valtan encounter stage numeric field is missing or invalid: " +
						Stage.strActionId;
					return false;
				}
				if (!Read_OptionalOrderedHitOffsets(
						StageValue, Stage.HitOffsetsMs))
				{
					strOutStatus =
						"Valtan encounter stage hitOffsetsMs is invalid: " +
						Stage.strActionId;
					return false;
				}
				Stage.strServerDamageProfileId = Read_String(
					StageValue, "serverDamageProfileId");
				const DATA_JSON_VALUE* pPlayerResponse =
					StageValue.Find("playerResponse");
				const DATA_JSON_VALUE* pAttachmentSlot =
					StageValue.Find("attachmentSlot");
				if ((nullptr == pPlayerResponse) != (nullptr == pAttachmentSlot) ||
					(nullptr != pPlayerResponse &&
					 (!pPlayerResponse->Is_String() ||
					  !pAttachmentSlot->Is_String())))
				{
					strOutStatus =
						"Valtan encounter capture hit fields are invalid: " +
						Stage.strActionId;
					return false;
				}
				if (nullptr != pPlayerResponse)
				{
					Stage.strPlayerResponse = pPlayerResponse->Get_String();
					Stage.strAttachmentSlot = pAttachmentSlot->Get_String();
				}
				const DATA_JSON_VALUE* pKnockdown = Required(
					StageValue, "knockdown", DATA_JSON_TYPE::BOOLEAN);
				if (nullptr == Required(StageValue, "serverDamageProfileId",
						DATA_JSON_TYPE::STRING) ||
					nullptr == pKnockdown ||
					!Read_RequiredFiniteFloat(
						StageValue, "pushRangeM", Stage.fPushRangeM) ||
					!Read_RequiredUInt32(
						StageValue, "pushMs", Stage.iPushMs) ||
					!Read_RequiredUInt32(
						StageValue, "downMs", Stage.iDownMs) ||
					!Read_StageMotion(StageValue.Find("motion"),
						Stage.iDurationMs, Stage.Motion) ||
					!Read_StageActions(StageValue.Find("actions"),
						Stage.iDurationMs, Stage.Actions) ||
					!Read_StageBranches(StageValue.Find("branches"), Stage.Branches) ||
					!Has_ValidNavigationBlockedCapture(Stage.Branches, Stage.strPlayerResponse) ||
					!Read_StageGameplayExtensions(StageValue,
						Stage.strPartDamagePolicy, Stage.CounterProxy) ||
					!Read_StageHitAuthority(
						StageValue, Stage.iDurationMs, Stage.bHasHitAnchor,
						Stage.strHitAnchorKind,
						Stage.fHitAnchorForwardOffsetM,
						Stage.fHitAnchorRightOffsetM,
						Stage.fHitAnchorYawOffsetDegrees,
						Stage.bHasHitActivation,
						Stage.iHitActivationStartMs,
						Stage.iHitActivationLifetimeMs))
				{
					strOutStatus =
						"Valtan encounter stage gameplay projection is invalid: " +
						Stage.strActionId;
					return false;
				}
				Stage.bKnockdown = pKnockdown->Get_Boolean();
				if (("DAMAGE" == Stage.strPlayerResponse &&
					 "NONE" != Stage.strAttachmentSlot) ||
					("CAPTURE" == Stage.strPlayerResponse &&
					 ("BOSS_LEFT_HAND" != Stage.strAttachmentSlot ||
					  "NONE" == Stage.strHitShape ||
					  0.f != Stage.fPushRangeM || 0u != Stage.iPushMs ||
					  Stage.bKnockdown || 0u != Stage.iDownMs)) ||
					("DAMAGE" != Stage.strPlayerResponse &&
					 "CAPTURE" != Stage.strPlayerResponse))
				{
					strOutStatus =
						"Valtan encounter player response contract is invalid: " +
						Stage.strActionId;
					return false;
				}

				const bool_t bHasExplicitOffsets = !Stage.HitOffsetsMs.empty();
				const bool_t bValidExplicitSchedule = bHasExplicitOffsets &&
					Stage.HitOffsetsMs.size() == Stage.iHitCount &&
					0u == Stage.iHitIntervalMs && 0u == Stage.iHitDelayMs &&
					Stage.HitOffsetsMs.back() < Stage.iDurationMs;
				const bool_t bValidLegacySchedule = !bHasExplicitOffsets &&
					Stage.iHitCount > 0u &&
					(1u == Stage.iHitCount ? 0u == Stage.iHitIntervalMs :
						Stage.iHitIntervalMs > 0u) &&
					static_cast<uint64_t>(Stage.iHitDelayMs) +
						static_cast<uint64_t>(Stage.iHitCount - 1u) *
							Stage.iHitIntervalMs < Stage.iDurationMs;
				const bool_t bValidEmptySchedule = 0u == Stage.iHitCount &&
					!bHasExplicitOffsets && 0u == Stage.iHitIntervalMs &&
					0u == Stage.iHitDelayMs;
				const bool_t bValidActiveWindow = Stage.bHasHitActivation &&
					bValidEmptySchedule && "NONE" != Stage.strHitShape;
				if ((!Stage.bHasHitActivation && !bValidExplicitSchedule &&
					 !bValidLegacySchedule && !bValidEmptySchedule) ||
					(Stage.bHasHitActivation && !bValidActiveWindow) ||
					("NONE" == Stage.strHitShape &&
					 (Stage.bHasHitAnchor || Stage.bHasHitActivation)))
				{
					strOutStatus =
						"Valtan encounter stage hit schedule is invalid: " +
						Stage.strActionId;
					return false;
				}

				const DATA_JSON_VALUE* pActions = StageValue.Find("actions");
				if (nullptr != pActions && pActions->Is_Array())
				{
					for (const DATA_JSON_VALUE& Action : pActions->Get_Array())
					{
						const std::string strSpawnKind =
							Read_String(Action, "kind");
						if (!Action.Is_Object() ||
							("SPAWN_COMBAT_OBJECT" != strSpawnKind &&
							 "SPAWN_COMBAT_OBJECT_VOLLEY" != strSpawnKind))
						{
							continue;
						}
						const std::string strTargetId = Read_String(
							Action, "targetId");
						const auto Reference =
							CombatObjectEffectsByArchetype.find(strTargetId);
						const double SpawnValue =
							"SPAWN_COMBAT_OBJECT_VOLLEY" == strSpawnKind ?
								Read_Number(Action, "countPerResolvedTarget") :
								Read_Number(Action, "value");
						if (Reference == CombatObjectEffectsByArchetype.end() ||
							Reference->second.strOwnerPatternId !=
								Pattern.strPatternId ||
							Reference->second.strOwnerStageActionId !=
								Stage.strActionId ||
							!std::isfinite(SpawnValue) || SpawnValue < 1.0 ||
							SpawnValue >
								static_cast<double>((std::numeric_limits<uint32_t>::max)()) ||
							std::floor(SpawnValue) != SpawnValue ||
							!ResolvedCombatObjectEffects.insert(strTargetId).second)
						{
							strOutStatus =
								"Valtan SPAWN_COMBAT_OBJECT authoring join changed: " +
								strTargetId;
							return false;
						}
						VALTAN_COMBAT_OBJECT_EFFECT_VIEW View;
						View.strCombatObjectArchetypeId = strTargetId;
						View.strClientVisualId =
							Reference->second.strClientVisualId;
						View.strEffectAssetId =
							Reference->second.strEffectAssetId;
						View.strTrigger = Read_String(Action, "trigger");
						View.iSpawnValue = static_cast<uint32_t>(SpawnValue);
						if ("SPAWN_COMBAT_OBJECT_VOLLEY" == strSpawnKind)
						{
							View.strVolleyPolicy = Read_String(
								Action, "targetingPolicy");
							View.strVolleyLayout = Read_String(Action, "layout");
							View.fVolleyRadiusM = static_cast<f32_t>(
								Read_Number(Action, "radiusM"));
							View.fVolleyStartAngleDegrees = static_cast<f32_t>(
								Read_Number(Action, "startAngleDegrees"));
							View.fVolleyAngleStepDegrees = static_cast<f32_t>(
								Read_Number(Action, "angleStepDegrees"));
						}
						View.strKind = Reference->second.strKind;
						View.strOriginPolicy =
							Reference->second.strOriginPolicy;
						View.strDirectionPolicy =
							Reference->second.strDirectionPolicy;
						View.fSpeedMps = Reference->second.fSpeedMps;
						View.fMaximumDistanceM =
							Reference->second.fMaximumDistanceM;
						View.iLifetimeMs = Reference->second.iLifetimeMs;
						View.HitIds = Reference->second.HitIds;
						View.HitOffsetsMs = Reference->second.HitOffsetsMs;
						View.PresentationEvents =
							Reference->second.PresentationEvents;
						Stage.CombatObjectEffects.push_back(std::move(View));
					}
				}

				const auto ClipIterator = ClipsByAction.find(Stage.strActionId);
				if (ClipIterator != ClipsByAction.end())
				{
					Stage.bSuppressAnimation =
						SuppressedAnimationActions.contains(Stage.strActionId);
					Stage.ClipOccurrences = ClipIterator->second;
					if (Stage.bSuppressAnimation &&
						!Stage.ClipOccurrences.empty())
					{
						strOutStatus =
							"Valtan NONE animation binding unexpectedly owns clips: " +
							Stage.strActionId;
						return false;
					}
					Stage.RuntimeClipNames.reserve(
						Stage.ClipOccurrences.size());
					for (const VALTAN_CLIP_OCCURRENCE_VIEW& Occurrence :
						Stage.ClipOccurrences)
					{
						Stage.RuntimeClipNames.push_back(
							Occurrence.strClipName);
					}
					Stage.strRuntimeClipName = Stage.RuntimeClipNames.empty() ?
						std::string{} : Stage.RuntimeClipNames.front();
				}

				const auto CueIterator = CueByAction.find(Stage.strActionId);
				if (CueIterator != CueByAction.end())
				{
					for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue :
						CueIterator->second)
					{
						if (Cue.strPatternId != Pattern.strPatternId ||
							Cue.strStageId != Stage.strStageId)
						{
							strOutStatus =
								"Valtan Product Effect cue pattern/stage tuple changed for " +
								Stage.strActionId + ".";
							return false;
						}
						if (Cue.bUsesStageClock)
						{
							if (!Stage.bSuppressAnimation ||
								!Stage.ClipOccurrences.empty() ||
								Cue.iStageOffsetMs >= Stage.iDurationMs)
							{
								strOutStatus =
									"Valtan stage-clock Effect cue left its NONE stage: " +
									Cue.strBindingId;
								return false;
							}
						}
						else
						{
							const auto Clip = std::find_if(
								Stage.ClipOccurrences.begin(),
								Stage.ClipOccurrences.end(),
								[&Cue](
									const VALTAN_CLIP_OCCURRENCE_VIEW& Occurrence)
								{
									return Occurrence.strClipOccurrenceId ==
										Cue.strClipOccurrenceId;
								});
							if (Clip == Stage.ClipOccurrences.end())
							{
								strOutStatus =
									"Valtan Product Effect cue clip occurrence left its stage: " +
									Cue.strClipOccurrenceId;
								return false;
							}
							Clip->ProductCues.push_back(Cue);
						}
						Stage.ProductCues.push_back(Cue);
						++iResolvedCueCount;

					}
					if (!Stage.ProductCues.empty())
						Stage.ProductCue = Stage.ProductCues.front();
				}

				const auto EffectIterator =
					EffectByAction.find(Stage.strActionId);
				if (EffectIterator != EffectByAction.end() &&
					!EffectIterator->second.first.empty() &&
					!EffectIterator->second.second.empty())
				{
					const auto Existing = std::find_if(
						Stage.Effects.begin(), Stage.Effects.end(),
						[&EffectIterator](const VALTAN_STAGE_EFFECT_VIEW& Effect)
						{
							return Effect.strEffectAssetId ==
								EffectIterator->second.first;
						});
					if (Existing != Stage.Effects.end())
					{
						Existing->bPatternEffectBinding = true;
					}
					else
					{
						VALTAN_STAGE_EFFECT_VIEW Bound;
						Bound.strEffectAssetId = EffectIterator->second.first;
						Bound.DocumentPath = CProjectDataRoot::Get() /
							std::filesystem::path(
								EffectIterator->second.second).lexically_normal();
						Bound.eOrigin =
							VALTAN_STAGE_EFFECT_ORIGIN::PATTERN_EFFECT_BINDING;
						Bound.bPatternEffectBinding = true;
						Stage.Effects.push_back(std::move(Bound));
					}
				}

				/* Product cues own the authoring entry. The naming rule is only
				   a fallback for a stage that has no Product cue, so the old
				   whirlwind seed cannot appear beside the admitted 420633 row. */
				if (!Stage.Has_ProductCue() &&
					Stage.CombatObjectEffects.empty())
				{
					const std::string strCandidate = Build_StageEffectAssetId(
						Pattern.strActionId, Stage);
					const bool_t bCandidateAlreadyBound = std::any_of(
						Stage.Effects.begin(), Stage.Effects.end(),
						[&strCandidate](const VALTAN_STAGE_EFFECT_VIEW& Effect)
						{
							return Effect.strEffectAssetId == strCandidate;
						});
					if (!strCandidate.empty() && !bCandidateAlreadyBound)
					{
						const std::filesystem::path Candidate =
							CProjectDataRoot::Resolve(
								std::filesystem::path(L"Effects") / L"Authored" /
								std::filesystem::path(
									strCandidate + ".effect.json"));
						std::error_code FileError;
						if (!Candidate.empty() && std::filesystem::exists(
								Candidate, FileError))
						{
							VALTAN_STAGE_EFFECT_VIEW Seeded;
							Seeded.strEffectAssetId = strCandidate;
							Seeded.DocumentPath = Candidate;
							Seeded.eOrigin =
								VALTAN_STAGE_EFFECT_ORIGIN::NAMING_RULE;
							Stage.Effects.push_back(std::move(Seeded));
						}
					}
				}
				Pattern.Stages.push_back(std::move(Stage));
			}
		}
		if (!Validate_PatternServerMotionStageWindows(
				Pattern.ServerMotion, Pattern.Stages))
		{
			strOutStatus =
				"Valtan encounter serverMotion window is outside its authored stage: " +
				Pattern.strPatternId;
			return false;
		}
		if (Pattern.Is_Gimmick())
			Staged.Gimmicks.push_back(std::move(Pattern));
		else
			Staged.Rotation.push_back(std::move(Pattern));
	}

	if (iResolvedCueCount != iCueCount)
	{
		strOutStatus = "Valtan Product Effect cue action did not resolve into the encounter: resolved " +
			std::to_string(iResolvedCueCount) + " of " +
			std::to_string(iCueCount) + ".";
		return false;
	}
	if (ResolvedCombatObjectEffects.size() !=
		CombatObjectEffectsByArchetype.size())
	{
		strOutStatus =
			"Valtan combat-object Effect visual did not resolve into its owner stage.";
		return false;
	}
	if (!Apply_MasterDocument(
			MasterDocument, PatternRotations, Staged, Error, ePolicy))
	{
		strOutStatus = "Valtan split authoring/Product join failed: " + Error;
		return false;
	}
	if (Staged.Gimmicks.empty() && Staged.Rotation.empty())
	{
		strOutStatus = "Valtan encounter produced no patterns.";
		return false;
	}
	/* Gimmicks read as a timeline down the health bar, so order them the way
	   the fight actually presents them. */
	std::sort(Staged.Gimmicks.begin(), Staged.Gimmicks.end(),
		[](const VALTAN_PATTERN_VIEW& Left, const VALTAN_PATTERN_VIEW& Right)
		{
			return Left.iTriggerHealthBar > Right.iTriggerHealthBar;
		});

	/* A phase is the band of health bars that ends when its gimmick fires.
	   The encounter document has no phase field, so this band is derived from
	   triggerHealthBar and used only as a display grouping. */
	int32_t iTopHealthBar = 0;
	const auto RaiseTop = [&iTopHealthBar](
		const std::vector<VALTAN_PATTERN_VIEW>& Group)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Group)
		{
			iTopHealthBar = (std::max)(iTopHealthBar,
				(std::max)(Pattern.iMaximumHealthBar, Pattern.iTriggerHealthBar));
		}
	};
	RaiseTop(Staged.Gimmicks);
	RaiseTop(Staged.Rotation);

	uint32_t iPhaseNumber = 0u;
	int32_t iBandTop = iTopHealthBar;
	for (size_t iGimmick = 0u; iGimmick < Staged.Gimmicks.size(); ++iGimmick)
	{
		const VALTAN_PATTERN_VIEW& Gimmick = Staged.Gimmicks[iGimmick];
		VALTAN_PHASE_VIEW Phase;
		Phase.iPhaseNumber = ++iPhaseNumber;
		Phase.iBandTopHealthBar = (std::max)(iBandTop, Gimmick.iTriggerHealthBar);
		Phase.iBandBottomHealthBar = Gimmick.iTriggerHealthBar;
		Phase.strGatePatternId = Gimmick.strPatternId;
		Phase.iGateTriggerHealthBar = Gimmick.iTriggerHealthBar;
		Phase.GimmickIndices.push_back(iGimmick);
		Staged.Phases.push_back(std::move(Phase));
		iBandTop = Gimmick.iTriggerHealthBar - 1;
	}
	if (iBandTop >= 1)
	{
		VALTAN_PHASE_VIEW Phase;
		Phase.iPhaseNumber = ++iPhaseNumber;
		Phase.iBandTopHealthBar = iBandTop;
		Phase.iBandBottomHealthBar = 1;
		Staged.Phases.push_back(std::move(Phase));
	}

	/* A rotation pattern belongs to every band its own bar range overlaps.
	   Most are 1-160 and therefore appear in all of them; two are narrower
	   and that is exactly the information the band grouping exposes. */
	for (VALTAN_PHASE_VIEW& Phase : Staged.Phases)
	{
		for (size_t iRotation = 0u; iRotation < Staged.Rotation.size();
			++iRotation)
		{
			const VALTAN_PATTERN_VIEW& Rotation = Staged.Rotation[iRotation];
			if (Rotation.iMinimumHealthBar > Phase.iBandTopHealthBar ||
				Rotation.iMaximumHealthBar < Phase.iBandBottomHealthBar)
			{
				continue;
			}
			Phase.RotationIndices.push_back(iRotation);
		}
	}

	const std::string strIntroPatternId = Read_String(
		Encounter, "introPatternId");
	if (!strIntroPatternId.empty())
	{
		const auto Intro = std::find_if(
			Staged.Rotation.begin(), Staged.Rotation.end(),
			[&strIntroPatternId](const VALTAN_PATTERN_VIEW& Pattern)
			{
				return Pattern.strPatternId == strIntroPatternId;
			});
		if (Intro != Staged.Rotation.end())
		{
			Staged.iIntroRotationIndex = static_cast<size_t>(
				std::distance(Staged.Rotation.begin(), Intro));
		}
	}

	OutView = std::move(Staged);
	strOutStatus = "Valtan tree: " +
		std::to_string(OutView.Phases.size()) + " phases, " +
		std::to_string(OutView.Get_PatternCount()) + " patterns, " +
		std::to_string(OutView.Get_StageCount()) + " stages, " +
		std::to_string(OutView.Get_ClipBoundStageCount()) + " clip-bound (" +
		std::to_string(OutView.Get_ClipOccurrenceCount()) + " occurrences), " +
		std::to_string(OutView.Get_ProductCueStageCount()) + " cue stages (" +
		std::to_string(OutView.Get_ProductCueCount()) + " Product cues), " +
		std::to_string(OutView.Get_CombatObjectEffectCount()) +
		" combat-object visuals, " +
		std::to_string(OutView.IndependentEffects.size()) +
		" independent Effects, " +
		std::to_string(OutView.Get_EffectCount()) + " with an Effect (" +
		std::to_string(OutView.Get_EffectDocumentCount()) + " documents).";
	return true;
}
