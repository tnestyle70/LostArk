#pragma once

#include "Client_Defines.h"
#include "Effect_AuthoringDocument.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CEffectCatalog final
{
public:
    static bool_t Load(std::string& strOutStatus);
    static std::shared_ptr<const EFFECT_DOCUMENT_DESC> Find(
        const std::string& strEffectAssetId);
    static bool_t Contains(const std::string& strEffectAssetId);
    static std::vector<std::string> Get_EffectAssetIds();
    static uint64_t Get_RuntimeRevision();
    static const std::string& Get_Status();
    static void Clear();
};

NS_END

