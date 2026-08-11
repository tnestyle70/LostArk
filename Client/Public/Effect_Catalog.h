#pragma once

#include "Client_Defines.h"
#include "Effect_AuthoringDocument.h"
#include "Effect_ComponentDocument.h"
#include "Effect_RuntimeAuthority.h"

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

NS_BEGIN(Client)

struct EFFECT_RUNTIME_PROGRAM_CATALOG_IDENTITY final
{
	uint64_t iCatalogRevision = 0u;
	uint32_t iArtifactRevision = 0u;
	uint32_t iProgramVersion = 0u;
	uint32_t iInputArtifactCount = 0u;
	uint64_t iCandidateByteCount = 0u;
	std::string strEffectAssetId;
	std::string strCompilerRevision;
	std::string strCandidateBuilderCommitId;
	std::string strCandidateBuilderTreeId;
	std::string strCandidateBlobId;
	std::string strProgramId;
	std::string strProgramSha256;
	std::string strCandidateRawSha256;
	std::string strResourceBindingHash;
	std::string strInputArtifactsOrderedSha256;
	std::string strReconstructedRuntimeProgramSha256;
	std::string strPublishReceiptSha256;
};

class CEffectCatalog;

class EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY final
{
public:
	const EFFECT_RUNTIME_PROGRAM_CATALOG_IDENTITY& Get_Identity() const
	{
		return m_Identity;
	}
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM>
		Get_Program() const
	{
		return m_pProgram;
	}

private:
	friend class CEffectCatalog;
	EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY(
		const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY&) = delete;
	EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY& operator=(
		const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY&) = delete;
	EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY(
		EFFECT_RUNTIME_PROGRAM_CATALOG_IDENTITY Identity,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> pProgram)
		: m_Identity(std::move(Identity)), m_pProgram(std::move(pProgram))
	{
	}

private:
	EFFECT_RUNTIME_PROGRAM_CATALOG_IDENTITY m_Identity;
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM> m_pProgram;
};

struct EFFECT_RECONSTRUCTED_ANCHOR_BINDING final
{
	std::string strOwnerEmitterId;
	EFFECT_RUNTIME_PROGRAM_ANCHOR_REQUEST Request;
};

class EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION final
{
public:
	std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY>
		Get_CatalogEntry() const
	{
		return m_pCatalogEntry;
	}
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM>
		Get_Program() const
	{
		return nullptr == m_pCatalogEntry ? nullptr :
			m_pCatalogEntry->Get_Program();
	}
	const std::vector<EFFECT_RECONSTRUCTED_ANCHOR_BINDING>&
		Get_AnchorRequests() const
	{
		return m_AnchorRequests;
	}

private:
	friend class CEffectCatalog;
	EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION(
		const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION&) = delete;
	EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION& operator=(
		const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION&) = delete;
	EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION(
		std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY> pCatalogEntry,
		std::vector<EFFECT_RECONSTRUCTED_ANCHOR_BINDING> AnchorRequests)
		: m_pCatalogEntry(std::move(pCatalogEntry)),
		  m_AnchorRequests(std::move(AnchorRequests))
	{
	}

private:
	std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY> m_pCatalogEntry;
	std::vector<EFFECT_RECONSTRUCTED_ANCHOR_BINDING> m_AnchorRequests;
};

enum class EFFECT_RECONSTRUCTED_RUNTIME_SEAM : uint8_t
{
	OBJECT,
	PLAYBACK,
	RENDERER,
	END
};

class CEffectReconstructedRuntimeBoundary final
{
public:
	static bool_t Prepare_Presentation(
		const std::string& strEffectAssetId,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>&
			OutPreparation,
		std::string& strOutError);
	static bool_t Admit_ProductSpawn(
		const std::string& strEffectAssetId,
		std::string& strOutError);
	bool_t Stage(
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
			pPreparation,
		EFFECT_RECONSTRUCTED_RUNTIME_SEAM eSeam,
		std::string& strOutError);
	bool_t Admit_Execution(std::string& strOutError) const;
	bool_t Admit_Submit(std::string& strOutError) const;
	bool_t Admit_Render(std::string& strOutError) const;
	void Clear() { m_pPreparation.reset(); }
	bool_t Is_Staged() const { return nullptr != m_pPreparation; }
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
		Get_Preparation() const
	{
		return m_pPreparation;
	}
	std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY>
		Get_CatalogEntry() const
	{
		return nullptr == m_pPreparation ? nullptr :
			m_pPreparation->Get_CatalogEntry();
	}
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM>
		Get_Program() const
	{
		return nullptr == m_pPreparation ? nullptr :
			m_pPreparation->Get_Program();
	}

private:
	std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>
		m_pPreparation;
};

class CEffectCatalog final
{
public:
    struct RUNTIME_SNAPSHOT;

    static bool_t Load(std::string& strOutStatus);
    static std::shared_ptr<const RUNTIME_SNAPSHOT> Capture_Runtime();
    static bool_t Restore_Runtime(
        std::shared_ptr<const RUNTIME_SNAPSHOT> pSnapshot,
        std::string& strOutStatus);
    static std::shared_ptr<const EFFECT_DOCUMENT_DESC> Find(
        const std::string& strEffectAssetId);
    static std::shared_ptr<const EFFECT_ASSEMBLY_DESC> Find_Assembly(
        const std::string& strEffectAssetId);
    static std::shared_ptr<const EFFECT_COMPONENT_DESC> Find_Component(
        const std::string& strComponentAssetId);
	static std::shared_ptr<const EFFECT_COMPILED_RUNTIME_DOCUMENT>
		Find_RuntimeAuthority(const std::string& strEffectAssetId);
	static std::shared_ptr<const EFFECT_RUNTIME_PROGRAM_CATALOG_ENTRY>
		Find_RuntimeProgramEntry(const std::string& strEffectAssetId);
	static std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PROGRAM>
		Find_ReconstructedRuntimeProgram(const std::string& strEffectAssetId);
	static bool_t Prepare_ReconstructedRuntimeProgram(
		const std::string& strEffectAssetId,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>&
			OutPreparation,
		std::string& strOutError);
    static bool_t Contains(const std::string& strEffectAssetId);
	static bool_t Contains_RuntimeAuthority(
		const std::string& strEffectAssetId);
	static bool_t Contains_ReconstructedRuntimeProgram(
		const std::string& strEffectAssetId);
	static bool_t Is_ReconstructedRuntimeProgramAssetId(
		const std::string& strEffectAssetId);
    static std::vector<std::string> Get_EffectAssetIds();
    static std::vector<std::string> Get_ComponentAssetIds();
	static std::vector<std::string> Get_RuntimeAuthorityAssetIds();
	static std::vector<std::string> Get_ReconstructedRuntimeProgramAssetIds();
    static uint64_t Get_RuntimeRevision();
    static const std::string& Get_Status();
    static void Clear();
};

NS_END

