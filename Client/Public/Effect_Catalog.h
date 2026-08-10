#pragma once

#include "Client_Defines.h"
#include "Effect_AuthoringDocument.h"
#include "Effect_ComponentDocument.h"
#include "Effect_RuntimeAuthority.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)

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
    static bool_t Contains(const std::string& strEffectAssetId);
	static bool_t Contains_RuntimeAuthority(
		const std::string& strEffectAssetId);
    static std::vector<std::string> Get_EffectAssetIds();
    static std::vector<std::string> Get_ComponentAssetIds();
	static std::vector<std::string> Get_RuntimeAuthorityAssetIds();
    static uint64_t Get_RuntimeRevision();
    static const std::string& Get_Status();
    static void Clear();
};

NS_END

