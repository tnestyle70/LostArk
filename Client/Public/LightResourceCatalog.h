#pragma once

#include "Client_Defines.h"
#include "Engine_RenderTypes.h"
#include "Engine_Defines.h"
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

struct LIGHT_RESOURCE final
{
    std::string strLightResourceId;
    std::string strDisplayName;
    std::string strDefaultAnchorKind = "BOSS";
    LIGHT eType = LIGHT::POINT;
    float3_t vLocalOffset = {0.f, 8.f, 0.f};
    float3_t vLocalRotationDegrees = {90.f, 0.f, 0.f};
    f32_t fRangeMeters = 16.f;
    f32_t fFalloffExponent = 2.f;
    f32_t fInnerConeDegrees = 12.f;
    f32_t fOuterConeDegrees = 26.f;
    float4_t vColor = {1.f, 1.f, 1.f, 1.f};
    f32_t fBrightness = 4.f;
};

/* Authoring edits never mutate the admitted Product catalog. */
class CLightResourceCatalog final
{
public:
    bool_t Load_Runtime(std::string& status);
    bool_t Reload_Runtime(std::string& status);
    bool_t Load_Authored(std::string& status);
    bool_t Refresh_MapResources(const std::string& areaId, std::string& status);
    const std::vector<LIGHT_RESOURCE>& Get_Resources() const { return m_ResourceView; }
    const LIGHT_RESOURCE* Find_Resource(std::string_view id) const;
    const LIGHT_RESOURCE* Find_RuntimeResource(std::string_view id) const;
    bool_t Is_MapResource(std::string_view id) const;
    bool_t Create(const std::string& name, const std::string& anchor, LIGHT type,
        std::string& outId, std::string& status);
    bool_t Update(const LIGHT_RESOURCE& resource, std::string& status);
    bool_t Delete(std::string_view id, std::string& status);
    bool_t Save_Authored(std::string& status);
    bool_t Publish_Runtime(std::string& status) const;
    static bool_t Validate_Resource(const LIGHT_RESOURCE& resource, std::string& status);
    static bool_t Try_BuildLightDesc(const LIGHT_RESOURCE& resource,
        const float4x4_t& pivot, f32_t brightnessMultiplier, LIGHT_DESC& light,
        std::string& status);
    static const char* Kind_Name(LIGHT type);
    static bool_t Run_Publisher(const std::filesystem::path& relativeScript,
        const std::wstring& arguments, std::string& status);
private:
    void Rebuild_ResourceView();
    std::vector<LIGHT_RESOURCE> m_Authored;
    std::vector<LIGHT_RESOURCE> m_Runtime;
    std::vector<LIGHT_RESOURCE> m_MapAuthored;
    std::vector<LIGHT_RESOURCE> m_MapRuntime;
    std::vector<LIGHT_RESOURCE> m_ResourceView;
    std::string m_MapAreaId;
    std::string m_SourceBytes;
    uint32_t m_Revision = 1u;
    uint32_t m_NextOrdinal = 1u;
    bool_t m_AuthoredReady = false;
};

NS_END
