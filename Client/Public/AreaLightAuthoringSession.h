#pragma once
#include "MapLightPresentationRuntime.h"
#include <memory>
#include <string>

NS_BEGIN(Client)

class CAreaLightAuthoringSession final
{
public:
    bool_t Open(const std::string& areaId, std::string& status);
    bool_t Reload(std::string& status);
    bool_t Is_Open() const { return m_Document.Is_Ready(); }
    bool_t Is_ReadOnly() const { return m_Document.Get_FormatVersion() != 2u; }
    const std::string& Get_AreaId() const { return m_Document.Get_AreaId(); }
    const CMapLightDocument& Get_Document() const { return m_Document; }
    std::shared_ptr<CMapLightPresentationRuntime> Get_Preview() const { return m_Preview; }
    bool_t Create(const std::string& name, LIGHT type, const float3_t& worldPosition,
        std::string& outId, std::string& status);
    bool_t Update(const MAP_POINT_LIGHT_RECORD& record, std::string& status);
    bool_t Delete(const std::string& id, std::string& status);
    bool_t Save_Authored(std::string& status);
    bool_t Publish_Runtime(std::string& status) const;
private:
    bool_t Commit(std::vector<MAP_POINT_LIGHT_RECORD> records, uint32_t nextOrdinal,
        std::string& status);
    CMapLightDocument m_Document;
    std::shared_ptr<CMapLightPresentationRuntime> m_Preview;
    std::filesystem::path m_Path;
    std::string m_SourceBytes;
};

NS_END
