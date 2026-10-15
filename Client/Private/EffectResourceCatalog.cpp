#include "EffectResourceCatalog.h"

// typed snapshot과 직접 저작 catalog에서 이미 승인된 Valtan ID·종류·revision을 읽는다.
#include "EffectV2_Catalog.h"
#include "Effect_Catalog.h"

// 목록 정렬/검색, 예외 보고, revision 상한 검사와 후보 데이터 이동에 필요한 표준 API다.
#include <algorithm>
#include <exception>
#include <limits>
#include <utility>

namespace
{
	using namespace Client;

	// 이 통합 목록은 Valtan 저작 화면의 범위다. 다른 보스 ID를 임의로 같은 목록에 섞지 않는다.
	constexpr std::string_view DIRECT_AUTHORED_VALTAN_PREFIX =
		"effect.valtan.";
	constexpr std::string_view TYPED_VALTAN_PREFIX = "boss.valtan.";

	// 원본 catalog를 읽은 임시 값이다. 아직 ID 충돌 검사나 facade 게시를 통과하지 않았다.
	struct PARSED_VALTAN_EFFECT_RESOURCES final
	{
		uint64_t iDirectAuthoredSourceRevision = 0u;
		uint64_t iTypedSourceRevision = 0u;
		std::vector<EFFECT_RESOURCE_DESCRIPTOR> Resources;
	};

	// 소유자·분류·명령 계약과 ID 유일성을 확인한 뒤 snapshot으로 옮길 값이다.
	struct VALIDATED_VALTAN_EFFECT_RESOURCES final
	{
		uint64_t iDirectAuthoredSourceRevision = 0u;
		uint64_t iTypedSourceRevision = 0u;
		std::vector<EFFECT_RESOURCE_DESCRIPTOR> Resources;
		std::map<std::string, std::size_t, std::less<>> StableIdIndex;
	};

	bool_t Fail(std::string& strOutError, std::string strMessage)
	{
		strOutError = std::move(strMessage);
		return false;
	}

	bool_t Starts_With(
		const std::string_view strValue,
		const std::string_view strPrefix) noexcept
	{
		return 0u == strValue.rfind(strPrefix, 0u);
	}

	// 원본 문서 소유자가 처리할 수 있는 저작 명령을 고정한다. 실행 자원 상태는 여기서 검사하지 않는다.
	EFFECT_RESOURCE_CAPABILITIES Capabilities_For(
		const EFFECT_RESOURCE_OWNER_KIND eOwnerKind)
	{
		EFFECT_RESOURCE_CAPABILITIES Capabilities;
		Capabilities.bCanLoad = true;
		Capabilities.bCanAppendToStage = true;
		Capabilities.bCanSave = true;
		Capabilities.bCanReload = true;
		Capabilities.bCanPreview = true;
		switch (eOwnerKind)
		{
		case EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT:
			Capabilities.bCanAppendToClip = true;
			// V1 문서는 요소 조합 전체를 소유하므로 하나의 composite 자원으로 다룬다.
			// 내부 요소를 typed V2 leaf로 바꾸는 뜻은 아니다.
			Capabilities.bComposite = true;
			break;
		case EFFECT_RESOURCE_OWNER_KIND::V2_LEAF:
			break;
		case EFFECT_RESOURCE_OWNER_KIND::V2_GROUP:
			Capabilities.bComposite = true;
			break;
		default:
			return {};
		}
		return Capabilities;
	}

	bool_t Matches_Capabilities(
		const EFFECT_RESOURCE_CAPABILITIES& Left,
		const EFFECT_RESOURCE_CAPABILITIES& Right) noexcept
	{
		return Left.bCanLoad == Right.bCanLoad &&
			Left.bCanAppendToStage == Right.bCanAppendToStage &&
			Left.bCanAppendToClip == Right.bCanAppendToClip &&
			Left.bCanSave == Right.bCanSave &&
			Left.bCanReload == Right.bCanReload &&
			Left.bCanPreview == Right.bCanPreview &&
			Left.bComposite == Right.bComposite;
	}

	// typed Effect 종류를 브라우저 분류로 바꾼다. 지원하지 않는 종류는 null로 돌려 전체 후보 검증을 실패시킨다.
	const char* Category_For(const EFFECT_V2_TYPE eType) noexcept
	{
		switch (eType)
		{
		case EFFECT_V2_TYPE::MESH:
			return "Mesh";
		case EFFECT_V2_TYPE::TEXTURE:
			return "Texture";
		case EFFECT_V2_TYPE::PARTICLE:
			return "Particle";
		case EFFECT_V2_TYPE::DECAL:
			return "Decal";
		case EFFECT_V2_TYPE::TRAIL:
			return "Trail";
		case EFFECT_V2_TYPE::SCREEN_POST:
			return "Screen Post";
		default:
			return nullptr;
		}
	}

	// 표시 이름도 stable ID로 맞춰 동일한 자원을 화면마다 다른 이름으로 연결하지 않게 한다.
	EFFECT_RESOURCE_DESCRIPTOR Make_Descriptor(
		const EFFECT_RESOURCE_OWNER_KIND eOwnerKind,
		std::string strStableId,
		std::string strCategoryLabel)
	{
		EFFECT_RESOURCE_DESCRIPTOR Descriptor;
		Descriptor.Key.eOwnerKind = eOwnerKind;
		Descriptor.Key.strStableId = std::move(strStableId);
		Descriptor.strDisplayLabel = Descriptor.Key.strStableId;
		Descriptor.strCategoryLabel = std::move(strCategoryLabel);
		Descriptor.Capabilities = Capabilities_For(eOwnerKind);
		return Descriptor;
	}

	// 파일 재파싱 없이 준비된 두 catalog의 Valtan 행을 복사한다. 실패하면 OutParsed를 갱신하지 않는다.
	bool_t Parse_ValtanOwnerSnapshots(
		PARSED_VALTAN_EFFECT_RESOURCES& OutParsed,
		std::string& strOutError)
	{
		PARSED_VALTAN_EFFECT_RESOURCES Parsed;
		Parsed.iDirectAuthoredSourceRevision =
			CEffectCatalog::Get_RuntimeRevision();
		if (0u == Parsed.iDirectAuthoredSourceRevision)
		{
			return Fail(strOutError,
				"Unified Effect Resource reload requires a ready direct-authored catalog.");
		}

		// V1 목록 중 직접 저작 문서만 받는다. 원본 recipe나 다른 보스의 ID는 이 facade 대상이 아니다.
		const std::vector<std::string> DirectAuthoredIds =
			CEffectCatalog::Get_EffectAssetIds();
		for (const std::string& strEffectId : DirectAuthoredIds)
		{
			if (Starts_With(strEffectId, DIRECT_AUTHORED_VALTAN_PREFIX) &&
				CEffectCatalog::Is_DirectAuthoredDocument(strEffectId))
			{
				Parsed.Resources.push_back(Make_Descriptor(
					EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT,
					strEffectId, "Authored"));
			}
		}
		if (std::none_of(
			Parsed.Resources.begin(), Parsed.Resources.end(),
			[](const EFFECT_RESOURCE_DESCRIPTOR& Descriptor)
			{
				return EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT ==
					Descriptor.Key.eOwnerKind;
			}))
		{
			return Fail(strOutError,
				"Unified Effect Resource reload found no direct-authored Valtan resources.");
		}
		if (Parsed.iDirectAuthoredSourceRevision !=
			CEffectCatalog::Get_RuntimeRevision())
		{
			return Fail(strOutError,
				"Direct-authored Effect catalog changed while its snapshot was parsed.");
		}

		// typed snapshot의 공유 소유권을 유지한 채 leaf와 group을 같은 revision에서 읽는다.
		const std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> pTypedSnapshot =
			CEffectV2Catalog::Get().Get_Snapshot();
		if (nullptr == pTypedSnapshot || !pTypedSnapshot->Is_Ready())
		{
			return Fail(strOutError,
				"Unified Effect Resource reload requires a ready typed catalog snapshot.");
		}
		Parsed.iTypedSourceRevision = pTypedSnapshot->Get_Revision();

		const std::size_t iTypedBegin = Parsed.Resources.size();
		for (const EFFECT_V2_DOCUMENT& Document :
			pTypedSnapshot->Get_Documents())
		{
			if (!Starts_With(Document.strEffectId, TYPED_VALTAN_PREFIX))
			{
				continue;
			}
			const char* pCategory = Category_For(Document.eType);
			if (nullptr == pCategory)
			{
				return Fail(strOutError,
					"Typed Effect resource has an unsupported category: " +
					Document.strEffectId);
			}
			Parsed.Resources.push_back(Make_Descriptor(
				EFFECT_RESOURCE_OWNER_KIND::V2_LEAF,
				Document.strEffectId, pCategory));
		}
		for (const EFFECT_V2_GROUP& Group : pTypedSnapshot->Get_Groups())
		{
			if (Starts_With(Group.strGroupId, TYPED_VALTAN_PREFIX))
			{
				Parsed.Resources.push_back(Make_Descriptor(
					EFFECT_RESOURCE_OWNER_KIND::V2_GROUP,
					Group.strGroupId, "Composite"));
			}
		}
		if (iTypedBegin == Parsed.Resources.size())
		{
			return Fail(strOutError,
				"Unified Effect Resource reload found no typed Valtan resources.");
		}

		OutParsed = std::move(Parsed);
		return true;
	}

	// stable ID 순으로 정렬하고 소유자별 계약을 검증한다. 소유자가 달라도 ID 충돌은 허용하지 않는다.
	bool_t Validate_ValtanResources(
		PARSED_VALTAN_EFFECT_RESOURCES Parsed,
		VALIDATED_VALTAN_EFFECT_RESOURCES& OutValidated,
		std::string& strOutError)
	{
		std::sort(
			Parsed.Resources.begin(), Parsed.Resources.end(),
			[](const EFFECT_RESOURCE_DESCRIPTOR& Left,
				const EFFECT_RESOURCE_DESCRIPTOR& Right)
			{
				if (Left.Key.strStableId != Right.Key.strStableId)
				{
					return Left.Key.strStableId < Right.Key.strStableId;
				}
				return static_cast<uint8_t>(Left.Key.eOwnerKind) <
					static_cast<uint8_t>(Right.Key.eOwnerKind);
			});

		VALIDATED_VALTAN_EFFECT_RESOURCES Validated;
		Validated.iDirectAuthoredSourceRevision =
			Parsed.iDirectAuthoredSourceRevision;
		Validated.iTypedSourceRevision = Parsed.iTypedSourceRevision;
		Validated.Resources = std::move(Parsed.Resources);
		for (std::size_t iResource = 0u;
			iResource < Validated.Resources.size(); ++iResource)
		{
			const EFFECT_RESOURCE_DESCRIPTOR& Descriptor =
				Validated.Resources[iResource];
			if (!Descriptor.Key.Is_Valid() ||
				Descriptor.Key.strStableId.find_first_of(" \t\r\n") !=
					std::string::npos)
			{
				return Fail(strOutError,
					"Unified Effect Resource contains an invalid stable ID.");
			}
			const bool_t bExpectedPrefix =
				EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT ==
					Descriptor.Key.eOwnerKind ?
				Starts_With(Descriptor.Key.strStableId,
					DIRECT_AUTHORED_VALTAN_PREFIX) :
				Starts_With(Descriptor.Key.strStableId, TYPED_VALTAN_PREFIX);
			if (!bExpectedPrefix)
			{
				return Fail(strOutError,
					"Unified Effect Resource owner and stable ID prefix disagree: " +
					Descriptor.Key.strStableId);
			}
			if (Descriptor.strDisplayLabel != Descriptor.Key.strStableId ||
				Descriptor.strCategoryLabel.empty())
			{
				return Fail(strOutError,
					"Unified Effect Resource label contract is invalid: " +
					Descriptor.Key.strStableId);
			}
			if (!Matches_Capabilities(
				Descriptor.Capabilities,
				Capabilities_For(Descriptor.Key.eOwnerKind)))
			{
				return Fail(strOutError,
					"Unified Effect Resource capability contract is invalid: " +
					Descriptor.Key.strStableId);
			}
			// 검증된 배열 위치를 인덱스에 넣는다. 중복 ID는 어느 소유자를 고를지 추측하지 않고 후보 전체를 거부한다.
			const auto [Iterator, bInserted] =
				Validated.StableIdIndex.emplace(
					Descriptor.Key.strStableId, iResource);
			if (!bInserted)
			{
				return Fail(strOutError,
					"Unified Effect Resource stable ID collision: " +
					Iterator->first);
			}
		}

		OutValidated = std::move(Validated);
		return true;
	}

}

NS_BEGIN(Client)

// 미선택 key를 거르는 최소 검사다. 실제 목록 포함 여부는 Find와 소유자 비교가 맡는다.
bool_t EFFECT_RESOURCE_KEY::Is_Valid() const noexcept
{
	return EFFECT_RESOURCE_OWNER_KIND::END != eOwnerKind &&
		!strStableId.empty();
}

bool_t EFFECT_RESOURCE_KEY::operator==(
	const EFFECT_RESOURCE_KEY& Other) const noexcept
{
	return eOwnerKind == Other.eOwnerKind &&
		strStableId == Other.strStableId;
}

uint64_t EFFECT_RESOURCE_CATALOG_SNAPSHOT::Get_Revision() const
	noexcept
{
	return m_iRevision;
}

uint64_t EFFECT_RESOURCE_CATALOG_SNAPSHOT::
Get_DirectAuthoredSourceRevision() const noexcept
{
	return m_iDirectAuthoredSourceRevision;
}

uint64_t EFFECT_RESOURCE_CATALOG_SNAPSHOT::
Get_TypedSourceRevision() const noexcept
{
	return m_iTypedSourceRevision;
}

bool_t EFFECT_RESOURCE_CATALOG_SNAPSHOT::Is_Ready() const noexcept
{
	return 0u != m_iRevision;
}

const std::vector<EFFECT_RESOURCE_DESCRIPTOR>&
EFFECT_RESOURCE_CATALOG_SNAPSHOT::Get_Resources() const noexcept
{
	return m_Resources;
}

// ID를 찾은 뒤 소유자까지 확인해 오래된 UI 명령이 다른 종류의 문서로 전달되지 않게 한다.
const EFFECT_RESOURCE_DESCRIPTOR*
EFFECT_RESOURCE_CATALOG_SNAPSHOT::Find(
	const EFFECT_RESOURCE_KEY& Key) const noexcept
{
	const EFFECT_RESOURCE_DESCRIPTOR* pDescriptor = Find(Key.strStableId);
	return nullptr != pDescriptor && pDescriptor->Key == Key ?
		pDescriptor : nullptr;
}

// 불변 배열 내부의 descriptor를 차용해 반환한다. 이 포인터를 쓰는 동안 snapshot을 보유해야 한다.
const EFFECT_RESOURCE_DESCRIPTOR*
EFFECT_RESOURCE_CATALOG_SNAPSHOT::Find(
	const std::string_view strStableId) const noexcept
{
	const auto Found = m_StableIdIndex.find(strStableId);
	if (Found == m_StableIdIndex.end() ||
		Found->second >= m_Resources.size())
	{
		return nullptr;
	}
	return &m_Resources[Found->second];
}

CEffectResourceCatalog& CEffectResourceCatalog::Get()
{
	static CEffectResourceCatalog Instance;
	return Instance;
}

// 최초에는 revision 0의 빈 snapshot을 제공한다. 첫 reload 실패를 준비 완료로 해석하지 않는다.
CEffectResourceCatalog::CEffectResourceCatalog()
	: m_pSnapshot(std::shared_ptr<const EFFECT_RESOURCE_CATALOG_SNAPSHOT>(
		new EFFECT_RESOURCE_CATALOG_SNAPSHOT()))
{
}

// Effect Tool의 명시적 새로고침 경로다. parse→validate→stage가 모두 끝난 뒤 목록 포인터만 commit한다.
bool_t CEffectResourceCatalog::Reload_Valtan(
	std::string& strOutError)
{
	try
	{
		PARSED_VALTAN_EFFECT_RESOURCES Parsed;
		if (!Parse_ValtanOwnerSnapshots(Parsed, strOutError))
		{
			return false;
		}

		VALIDATED_VALTAN_EFFECT_RESOURCES Validated;
		if (!Validate_ValtanResources(
			std::move(Parsed), Validated, strOutError))
		{
			return false;
		}

		const uint64_t iDirectAuthoredSourceRevision =
			Validated.iDirectAuthoredSourceRevision;
		const uint64_t iTypedSourceRevision =
			Validated.iTypedSourceRevision;
		// 검증한 값과 인덱스를 새 snapshot에 옮긴다. 이때 기존 공개 view는 그대로 유지한다.
		const auto Stage_ValtanSnapshot =
			[&strOutError](
				VALIDATED_VALTAN_EFFECT_RESOURCES StagedResources,
				std::shared_ptr<EFFECT_RESOURCE_CATALOG_SNAPSHOT>& pOutStaged)
				-> bool_t
			{
				if (StagedResources.Resources.empty() ||
					StagedResources.StableIdIndex.empty())
				{
					return Fail(strOutError,
						"Unified Effect Resource cannot stage an empty Valtan snapshot.");
				}
				auto pCandidate =
					std::shared_ptr<EFFECT_RESOURCE_CATALOG_SNAPSHOT>(
						new EFFECT_RESOURCE_CATALOG_SNAPSHOT());
				pCandidate->m_iDirectAuthoredSourceRevision =
					StagedResources.iDirectAuthoredSourceRevision;
				pCandidate->m_iTypedSourceRevision =
					StagedResources.iTypedSourceRevision;
				pCandidate->m_Resources =
					std::move(StagedResources.Resources);
				pCandidate->m_StableIdIndex =
					std::move(StagedResources.StableIdIndex);
				pOutStaged = std::move(pCandidate);
				return true;
			};
		std::shared_ptr<EFFECT_RESOURCE_CATALOG_SNAPSHOT> pStaged;
		if (!Stage_ValtanSnapshot(
			std::move(Validated), pStaged))
		{
			return false;
		}

		// 메인 스레드 저작 흐름에서 읽은 두 원본 revision을 게시 직전에 다시 확인한다.
		// 후보를 만드는 동안 원본이 바뀌었으면 서로 다른 세대를 합친 목록을 게시하지 않는다.
		if (iDirectAuthoredSourceRevision !=
				CEffectCatalog::Get_RuntimeRevision() ||
			iTypedSourceRevision != CEffectV2Catalog::Get().Get_Revision())
		{
			return Fail(strOutError,
				"Effect Resource owner catalog changed before snapshot commit.");
		}

		{
			// 현재 공유 포인터를 잠시 잠그고 facade revision을 올린다. 기존 독자는 자기 snapshot을 계속 보유한다.
			const std::lock_guard<std::mutex> Lock(m_SnapshotMutex);
			const uint64_t iPreviousRevision = nullptr != m_pSnapshot ?
				m_pSnapshot->Get_Revision() : 0u;
			if ((std::numeric_limits<uint64_t>::max)() == iPreviousRevision)
			{
				return Fail(strOutError,
					"Effect Resource catalog revision is exhausted.");
			}
			pStaged->m_iRevision = iPreviousRevision + 1u;
			m_pSnapshot = std::move(pStaged);
		}
		strOutError.clear();
		return true;
	}
	catch (const std::exception& Exception)
	{
		return Fail(strOutError,
			"Effect Resource reload failed before commit: " +
			std::string(Exception.what()));
	}
	catch (...)
	{
		return Fail(strOutError,
			"Effect Resource reload failed before commit.");
	}
}

// 포인터 복사만 잠금 안에서 끝낸다. 반환한 불변 snapshot의 조회에는 이 mutex가 필요하지 않다.
std::shared_ptr<const EFFECT_RESOURCE_CATALOG_SNAPSHOT>
CEffectResourceCatalog::Get_Snapshot() const
{
	const std::lock_guard<std::mutex> Lock(m_SnapshotMutex);
	return m_pSnapshot;
}

uint64_t CEffectResourceCatalog::Get_Revision() const
{
	const std::shared_ptr<const EFFECT_RESOURCE_CATALOG_SNAPSHOT> pSnapshot =
		Get_Snapshot();
	return nullptr != pSnapshot ? pSnapshot->Get_Revision() : 0u;
}

NS_END
