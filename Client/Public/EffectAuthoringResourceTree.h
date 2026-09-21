#pragma once

#include "EffectResourceCatalog.h"
#include "Network/PacketType.h"

#include <array>

#include <deque>
#include <filesystem>
#include <functional>
#include <string_view>
#include <unordered_map>
#include <string>
#include <vector>

NS_BEGIN(Client)

// Shared owner menu for All Effects and Composition resource selection.
enum class EFFECT_TOOL_ALL_EFFECTS_OWNER_KIND : uint8_t
{
	PLAYER_CLASS,
	VALTAN_BOSS,
	KOUKU_BOSS,
	WORLD
};

struct EFFECT_TOOL_ALL_EFFECTS_OWNER_OPTION final
{
	EFFECT_TOOL_ALL_EFFECTS_OWNER_KIND eKind =
		EFFECT_TOOL_ALL_EFFECTS_OWNER_KIND::PLAYER_CLASS;
	LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
		LostArk::Shared::CHARACTER_CLASS_ID::END;
	std::string_view strLabel;
};

inline constexpr std::array<EFFECT_TOOL_ALL_EFFECTS_OWNER_OPTION, 10u>
	EFFECT_TOOL_ALL_EFFECTS_OWNER_OPTIONS = {{
		{ EFFECT_TOOL_ALL_EFFECTS_OWNER_KIND::PLAYER_CLASS,
			LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER,
			"Lance Master" },
		{ EFFECT_TOOL_ALL_EFFECTS_OWNER_KIND::PLAYER_CLASS,
			LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER,
			"Gunslinger" },
		{ EFFECT_TOOL_ALL_EFFECTS_OWNER_KIND::PLAYER_CLASS,
			LostArk::Shared::CHARACTER_CLASS_ID::SLAYER,
			"Slayer" },
		{ EFFECT_TOOL_ALL_EFFECTS_OWNER_KIND::PLAYER_CLASS,
			LostArk::Shared::CHARACTER_CLASS_ID::ARTIST,
			"Artist" },
		{ EFFECT_TOOL_ALL_EFFECTS_OWNER_KIND::PLAYER_CLASS,
			LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONMASTER,
			"Dimension Master" },
		{ EFFECT_TOOL_ALL_EFFECTS_OWNER_KIND::PLAYER_CLASS,
			LostArk::Shared::CHARACTER_CLASS_ID::WARLORD,
			"Warlord" },
		{ EFFECT_TOOL_ALL_EFFECTS_OWNER_KIND::PLAYER_CLASS,
			LostArk::Shared::CHARACTER_CLASS_ID::GUARDIANKNIGHT,
			"Guardian Knight" },
		{ EFFECT_TOOL_ALL_EFFECTS_OWNER_KIND::VALTAN_BOSS,
			LostArk::Shared::CHARACTER_CLASS_ID::END,
			"Valtan" },
		{ EFFECT_TOOL_ALL_EFFECTS_OWNER_KIND::KOUKU_BOSS,
			LostArk::Shared::CHARACTER_CLASS_ID::END,
			"KoukuSaydon" },
		{ EFFECT_TOOL_ALL_EFFECTS_OWNER_KIND::WORLD,
			LostArk::Shared::CHARACTER_CLASS_ID::END,
			"World" }
	}};


/* One display projection for the Kouku V1 catalog in All Effects and Composition.
   Classification changes neither source identity nor its admission/preview owner. */
struct EFFECT_TOOL_KOUKU_EFFECT_VIEW final
{
    int gateOrder = 4;
    std::string assetId, text, originalName, fullPath;
};
EFFECT_TOOL_KOUKU_EFFECT_VIEW Describe_KoukuSavedEffect(const std::string& assetId,
    const std::string& displayName, std::vector<std::string> categoryPath);
bool Matches_KoukuSavedEffect(const EFFECT_TOOL_KOUKU_EFFECT_VIEW& view, const std::string& search);
std::vector<std::size_t> Order_KoukuSavedEffects(const std::vector<EFFECT_TOOL_KOUKU_EFFECT_VIEW>& views);
struct EFFECT_TOOL_KOUKU_EFFECT_TREE final
{
    std::vector<EFFECT_TOOL_KOUKU_EFFECT_VIEW> views;
    std::vector<std::size_t> order;
    std::unordered_map<std::string, std::size_t> assetIndices;
};
EFFECT_TOOL_KOUKU_EFFECT_TREE Build_KoukuSavedEffectTree(std::vector<EFFECT_TOOL_KOUKU_EFFECT_VIEW> views);
void Render_KoukuSavedEffectTree(const EFFECT_TOOL_KOUKU_EFFECT_TREE& tree,
    const std::function<void(std::size_t)>& renderLeaf, bool expandAll = false,
    std::string_view revealAssetId = {}, const std::function<bool(std::size_t)>& isVisible = {});

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

    explicit CEffectAuthoringResourceTree(EFFECT_RESOURCE_OWNER_KIND owner = EFFECT_RESOURCE_OWNER_KIND::END);
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
    struct RESOURCE final
    {
        EFFECT_RESOURCE_OWNER_KIND eKind = EFFECT_RESOURCE_OWNER_KIND::END;
        std::string strAssetId, strDisplayName, strStatus;
        std::vector<std::string> CategoryPath;
    };
    // Shared metadata inventory for the saved-resource tree and Composition browser.
    static bool Read_V1Organization(std::vector<RESOURCE>& OutRows, std::string& strOutError);
    static bool Read_V1Inventory(std::vector<RESOURCE>& OutRows, std::string& strOutError);

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

    static bool Parse(const std::string& strBytes, DOCUMENT& OutDocument, std::string& strOutError);
    static bool Validate(const DOCUMENT& Document, std::string& strOutError);
    static std::string Serialize(const DOCUMENT& Document);
    static std::string Root_For(const DOCUMENT& Document, const std::string& strNodeId);
    static bool Read_Source(const std::filesystem::path& Path, std::string& strOutBytes,
        bool& bOutExists, std::string& strOutError);
    bool Save_Staged(DOCUMENT Candidate, std::string& strOutStatus);
    bool Create_Parent(bool bCategory);
    void Queue_Selected(COMMAND_KIND eCommand);
    void Render_Branch(const std::string& strId, const std::string& strName, bool bRoot);
    void Render_Resources(const std::string& strParentId);
    const REFERENCE* Find_Reference(EFFECT_RESOURCE_OWNER_KIND eKind, const std::string& strId) const;

    EFFECT_RESOURCE_OWNER_KIND m_eOwnerFilter = EFFECT_RESOURCE_OWNER_KIND::END;
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
    char m_szCopyId[129] = {}, m_szCopyName[257] = {};
};

NS_END
