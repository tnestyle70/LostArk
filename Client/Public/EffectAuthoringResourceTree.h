#pragma once

#include "EffectResourceCatalog.h"

#include <deque>
#include <filesystem>
#include <string>
#include <vector>

NS_BEGIN(Client)

/* Saved-resource organization only. Effect documents keep their native owner,
   stable ID, source location, codec and runtime. Render never opens a model. */
class CEffectAuthoringResourceTree final
{
public:
    enum class COMMAND_KIND { OPEN, PREVIEW, APPEND, CREATE_EFFECT, CREATE_V1_COPY };
    struct COMMAND final
    {
        COMMAND_KIND eCommand = COMMAND_KIND::OPEN;
        EFFECT_RESOURCE_OWNER_KIND eKind = EFFECT_RESOURCE_OWNER_KIND::END;
        std::string strAssetId;
        std::string strDisplayName;
        std::string strParentId;
        std::string strSourceAssetId;
    };

    void Render();
    void Set_V1CopySource(const std::string& strAssetId, const std::string& strDisplayName);
    bool Take_Command(COMMAND& OutCommand);
    bool Reload(std::string& strOutStatus);
    bool Attach_Saved(EFFECT_RESOURCE_OWNER_KIND eKind,
        const std::string& strAssetId, const std::string& strDisplayName,
        const std::string& strParentId, std::string& strOutStatus);
    const std::string& Selected_ParentId() const { return m_strSelectedParentId; }
    std::string Selected_ParentName() const;
    EFFECT_RESOURCE_OWNER_KIND Selected_Kind() const;
    void Set_Status(std::string strStatus) { m_strStatus = std::move(strStatus); }

private:
    struct NODE final
    {
        std::string strId, strParentId, strDisplayName;
        bool bCategory = false;
    };
    struct REFERENCE final
    {
        EFFECT_RESOURCE_OWNER_KIND eKind = EFFECT_RESOURCE_OWNER_KIND::END;
        std::string strAssetId, strDisplayName, strParentId;
    };
    struct DOCUMENT final
    {
        std::vector<NODE> Nodes;
        std::vector<REFERENCE> References;
    };
    struct RESOURCE final
    {
        EFFECT_RESOURCE_OWNER_KIND eKind = EFFECT_RESOURCE_OWNER_KIND::END;
        std::string strAssetId, strDisplayName, strStatus;
    };

    static bool Parse(const std::string& strBytes, DOCUMENT& OutDocument, std::string& strOutError);
    static bool Validate(const DOCUMENT& Document, std::string& strOutError);
    static std::string Serialize(const DOCUMENT& Document);
    static std::string Root_For(const DOCUMENT& Document, const std::string& strNodeId);
    static bool Read_Source(const std::filesystem::path& Path, std::string& strOutBytes,
        bool& bOutExists, std::string& strOutError);
    static bool Read_V1Inventory(std::vector<RESOURCE>& OutRows, std::string& strOutError);
    bool Save_Staged(DOCUMENT Candidate, std::string& strOutStatus);
    bool Create_Parent(bool bCategory);
    void Queue_Selected(COMMAND_KIND eCommand);
    void Render_Branch(const std::string& strId, const std::string& strName, bool bRoot);
    void Render_Resources(const std::string& strParentId);
    const REFERENCE* Find_Reference(EFFECT_RESOURCE_OWNER_KIND eKind, const std::string& strId) const;

    DOCUMENT m_Document;
    std::vector<RESOURCE> m_V1Resources, m_V2Resources;
    std::deque<COMMAND> m_Commands;
    std::string m_strBaselineBytes, m_strStatus;
    bool m_bBaselineExists = false, m_bMetadataReady = false, m_bReloadAttempted = false;
    std::string m_strSelectedParentId = "root.v1";
    EFFECT_RESOURCE_KEY m_SelectedResource;
    EFFECT_RESOURCE_OWNER_KIND m_eCreateKind = EFFECT_RESOURCE_OWNER_KIND::V2_LEAF;
    char m_szName[257] = {};
    char m_szSearch[257] = {};
    std::string m_strV1CopySource;
    char m_szCopyId[129] = {}, m_szCopyName[65] = {};
};

NS_END
