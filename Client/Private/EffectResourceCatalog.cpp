#include "EffectResourceCatalog.h"

#include "EffectV2_Catalog.h"
#include "Effect_Catalog.h"

#include <algorithm>
#include <exception>
#include <limits>
#include <utility>

namespace
{
	using namespace Client;

	constexpr std::string_view DIRECT_AUTHORED_VALTAN_PREFIX =
		"effect.valtan.";
	constexpr std::string_view TYPED_VALTAN_PREFIX = "boss.valtan.";

	struct PARSED_VALTAN_EFFECT_RESOURCES final
	{
		uint64_t iDirectAuthoredSourceRevision = 0u;
		uint64_t iTypedSourceRevision = 0u;
		std::vector<EFFECT_RESOURCE_DESCRIPTOR> Resources;
	};

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
			// A direct-authored document owns its complete element composition.
			// Consumers may therefore treat it as one atomic group resource
			// without pretending that its elements are typed V2 leaves.
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
				continue;
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
					return Left.Key.strStableId < Right.Key.strStableId;
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

bool_t Client::EFFECT_RESOURCE_KEY::Is_Valid() const noexcept
{
	return EFFECT_RESOURCE_OWNER_KIND::END != eOwnerKind &&
		!strStableId.empty();
}

bool_t Client::EFFECT_RESOURCE_KEY::operator==(
	const EFFECT_RESOURCE_KEY& Other) const noexcept
{
	return eOwnerKind == Other.eOwnerKind &&
		strStableId == Other.strStableId;
}

uint64_t Client::EFFECT_RESOURCE_CATALOG_SNAPSHOT::Get_Revision() const
	noexcept
{
	return m_iRevision;
}

uint64_t Client::EFFECT_RESOURCE_CATALOG_SNAPSHOT::
Get_DirectAuthoredSourceRevision() const noexcept
{
	return m_iDirectAuthoredSourceRevision;
}

uint64_t Client::EFFECT_RESOURCE_CATALOG_SNAPSHOT::
Get_TypedSourceRevision() const noexcept
{
	return m_iTypedSourceRevision;
}

bool_t Client::EFFECT_RESOURCE_CATALOG_SNAPSHOT::Is_Ready() const noexcept
{
	return 0u != m_iRevision;
}

const std::vector<Client::EFFECT_RESOURCE_DESCRIPTOR>&
Client::EFFECT_RESOURCE_CATALOG_SNAPSHOT::Get_Resources() const noexcept
{
	return m_Resources;
}

const Client::EFFECT_RESOURCE_DESCRIPTOR*
Client::EFFECT_RESOURCE_CATALOG_SNAPSHOT::Find(
	const EFFECT_RESOURCE_KEY& Key) const noexcept
{
	const EFFECT_RESOURCE_DESCRIPTOR* pDescriptor = Find(Key.strStableId);
	return nullptr != pDescriptor && pDescriptor->Key == Key ?
		pDescriptor : nullptr;
}

const Client::EFFECT_RESOURCE_DESCRIPTOR*
Client::EFFECT_RESOURCE_CATALOG_SNAPSHOT::Find(
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

Client::CEffectResourceCatalog& Client::CEffectResourceCatalog::Get()
{
	static CEffectResourceCatalog Instance;
	return Instance;
}

Client::CEffectResourceCatalog::CEffectResourceCatalog()
	: m_pSnapshot(std::shared_ptr<const EFFECT_RESOURCE_CATALOG_SNAPSHOT>(
		new EFFECT_RESOURCE_CATALOG_SNAPSHOT()))
{
}

bool_t Client::CEffectResourceCatalog::Reload_Valtan(
	std::string& strOutError)
{
	try
	{
		PARSED_VALTAN_EFFECT_RESOURCES Parsed;
		if (!Parse_ValtanOwnerSnapshots(Parsed, strOutError))
			return false;

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

		/* Owner catalogs are main-thread authoring services.  Reject a stale
		   read-set before the one facade commit instead of publishing a mixed
		   generation. */
		if (iDirectAuthoredSourceRevision !=
				CEffectCatalog::Get_RuntimeRevision() ||
			iTypedSourceRevision != CEffectV2Catalog::Get().Get_Revision())
		{
			return Fail(strOutError,
				"Effect Resource owner catalog changed before snapshot commit.");
		}

		{
			const std::lock_guard Lock(m_SnapshotMutex);
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

std::shared_ptr<const Client::EFFECT_RESOURCE_CATALOG_SNAPSHOT>
Client::CEffectResourceCatalog::Get_Snapshot() const
{
	const std::lock_guard Lock(m_SnapshotMutex);
	return m_pSnapshot;
}

uint64_t Client::CEffectResourceCatalog::Get_Revision() const
{
	const std::shared_ptr<const EFFECT_RESOURCE_CATALOG_SNAPSHOT> pSnapshot =
		Get_Snapshot();
	return nullptr != pSnapshot ? pSnapshot->Get_Revision() : 0u;
}
