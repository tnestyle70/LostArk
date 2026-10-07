#include "CharacterSelectShowcase.h"
#include "Character.h"
#include "CharacterCatalog.h"
#include "DataJson.h"
#include "GameInstance.h"
#include "MapPlacementRuntime.h"
#include "PlayableCharacterAssetService.h"
#include "ProjectDataRoot.h"
#include "Transform.h"

#include <array>
#include <charconv>
#include <cmath>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <utility>

using namespace Client;
using namespace LostArk::Shared;

namespace
{
    constexpr uint32_t STAGE_LEVEL = ETOUI(LEVEL::CHARACTER_SELECT);
    constexpr size_t MAXIMUM_DOCUMENT_BYTES = 16u * 1024u;
    constexpr const wchar_t* CHARACTER_LAYER = L"Layer_CharacterSelectShowcase";
    constexpr const char* AREA_ID = "LV_LOBBY_CLASSSELECT_SL00";
    constexpr std::array<const char*, 2> SOURCE_IDS = {
        "LV_LOBBY_CLASSSELECT_SL00:export:1053",
        "LV_LOBBY_CLASSSELECT_SL00:export:1054"
    };
    constexpr std::array<uint64_t, 2> PLACEMENT_IDS = { 9000101u, 9000102u };

    const DATA_JSON_VALUE& Field(const DATA_JSON_VALUE& object, const char* name)
    {
        const auto* value = object.Is_Object() ? object.Find(name) : nullptr;
        if (!value)
            throw std::runtime_error(std::string("Missing showcase field: ") + name);
        return *value;
    }

    const std::string& String(const DATA_JSON_VALUE& value)
    {
        if (!value.Is_String())
            throw std::runtime_error("Showcase field must be a string");
        return value.Get_String();
    }

    float Number(const DATA_JSON_VALUE& value)
    {
        if (!value.Is_Number() || !std::isfinite(value.Get_Number()) ||
            std::abs(value.Get_Number()) > (std::numeric_limits<float>::max)())
            throw std::runtime_error("Showcase number must be a finite float");
        return static_cast<float>(value.Get_Number());
    }

    template<size_t Count>
    std::array<float, Count> Vector(const DATA_JSON_VALUE& value)
    {
        if (!value.Is_Array() || value.Get_Array().size() != Count)
            throw std::runtime_error("Invalid showcase vector size");
        std::array<float, Count> result{};
        for (size_t index = 0; index < Count; ++index)
            result[index] = Number(value.Get_Array()[index]);
        return result;
    }

    float3_t Position(const DATA_JSON_VALUE& value)
    {
        const auto v = Vector<3>(value);
        return float3_t(v[0], v[1], v[2]);
    }

    uint64_t PlacementId(const DATA_JSON_VALUE& value)
    {
        const std::string& text = String(value);
        uint64_t result = 0u;
        const auto parsed = std::from_chars(text.data(), text.data() + text.size(), result);
        if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size() ||
            0u == result || result > CMapPlacementDocument::MAX_EDITOR_PLACEMENT_ID)
            throw std::runtime_error("Invalid showcase placement ID");
        return result;
    }

    struct STAGE_CONFIG
    {
        std::array<MAP_PLACEMENT_RECORD, 2> placements;
        float3_t characterPosition{};
        float characterYaw = 0.f;
        float3_t lightingTranslation{};
    };

    STAGE_CONFIG Read_Config(const CMapPlacementRuntime& sourceMap)
    {
        if (!sourceMap.Get_Catalog().Is_Ready() ||
            sourceMap.Get_Catalog().Get_AreaId() != AREA_ID)
            throw std::runtime_error("Showcase requires the loaded Character Select map");

        std::ifstream input(CProjectDataRoot::Resolve(
            L"Rendering/Authored/CharacterSelectShowcase.json"), std::ios::binary | std::ios::ate);
        if (!input)
            throw std::runtime_error("Character Select showcase document could not be opened");
        const std::streamoff length = input.tellg();
        if (length <= 0 || length > static_cast<std::streamoff>(MAXIMUM_DOCUMENT_BYTES))
            throw std::runtime_error("Character Select showcase document exceeds its size limit");
        std::string text(static_cast<size_t>(length), '\0');
        input.seekg(0);
        if (!input.read(text.data(), static_cast<std::streamsize>(text.size())) ||
            input.peek() != std::char_traits<char>::eof())
            throw std::runtime_error("Character Select showcase document changed while reading");

        DATA_JSON_VALUE root;
        std::string parseError;
        DATA_JSON_PARSE_LIMITS limits;
        limits.iMaximumBytes = MAXIMUM_DOCUMENT_BYTES;
        limits.iMaximumDepth = 8u;
        limits.iMaximumValues = 256u;
        if (!CDataJson::Parse(text, root, parseError, limits))
            throw std::runtime_error("Invalid showcase JSON: " + parseError);
        const auto& version = Field(root, "formatVersion");
        if (String(Field(root, "schema")) != "lostark.character-select-showcase" ||
            String(Field(root, "areaId")) != AREA_ID || !version.Is_Number() ||
            version.Was_FloatingPointToken() || version.Get_Number() != 1.0)
            throw std::runtime_error("Unsupported Character Select showcase schema");
        const auto& rows = Field(root, "placements");
        if (!rows.Is_Array() || rows.Get_Array().size() != SOURCE_IDS.size())
            throw std::runtime_error("Showcase requires exactly its floor and star placements");

        STAGE_CONFIG config;
        std::array<bool, 2> seen{};
        for (const auto& row : rows.Get_Array())
        {
            const std::string& sourceId = String(Field(row, "sourcePlacementId"));
            size_t slot = SOURCE_IDS.size();
            for (size_t index = 0; index < SOURCE_IDS.size(); ++index)
                if (sourceId == SOURCE_IDS[index]) slot = index;
            if (slot == SOURCE_IDS.size() || seen[slot])
                throw std::runtime_error("Unknown or duplicate showcase source placement");
            seen[slot] = true;

            const uint64_t placementId = PlacementId(Field(row, "placementId"));
            if (placementId != PLACEMENT_IDS[slot])
                throw std::runtime_error("Unexpected showcase runtime placement ID");
            const std::string runtimeSourceId = "showcase:character-select:" + std::to_string(placementId);
            const MAP_PLACEMENT_RECORD* sourceRecord = nullptr;
            for (const auto& entry : sourceMap.Get_Placements())
            {
                if (entry.record.placementId == placementId ||
                    entry.record.sourcePlacementId == runtimeSourceId)
                    throw std::runtime_error("Showcase runtime placement ID collides with the map");
                if (entry.record.sourcePlacementId == sourceId)
                {
                    if (sourceRecord)
                        throw std::runtime_error("Showcase source placement is not unique");
                    sourceRecord = &entry.record;
                }
            }
            if (!sourceRecord)
                throw std::runtime_error("Showcase source placement is not loaded");

            /* Preserve the loaded asset, RNM, source wind and all catalog inputs.
            Low runtime IDs are overlays, never imported actor IDs. */
            auto& record = config.placements[slot];
            record = *sourceRecord;
            record.placementId = placementId;
            record.sourcePlacementId = runtimeSourceId;
            record.transformSource = "overlay";
            record.position = Position(Field(row, "position"));
            const auto rotation = Vector<4>(Field(row, "rotationQuaternion"));
            double normSquared = 0.0;
            for (float component : rotation)
                normSquared += static_cast<double>(component) * component;
            if (std::abs(normSquared - 1.0) > 1.e-4)
                throw std::runtime_error("Showcase quaternion must be unit length");
            record.rotationQuaternion = float4_t(rotation[0], rotation[1], rotation[2], rotation[3]);
            record.signedScale = Position(Field(row, "signedScale"));
            if (record.signedScale.x < 1.e-6f || record.signedScale.y < 1.e-6f ||
                record.signedScale.z < 1.e-6f)
                throw std::runtime_error("Showcase scale must be positive and nondegenerate");
            record.visible = false;
            if (!CMapPlacementDocument::Is_Valid(record, sourceMap.Get_Catalog()))
                throw std::runtime_error("Invalid showcase placement record");
        }
        const auto& character = Field(root, "character");
        config.characterPosition = Position(Field(character, "position"));
        config.characterYaw = Number(Field(character, "yawDegrees"));
        const float3_t lightingOrigin = Position(Field(root, "lightingOrigin"));
        config.lightingTranslation = float3_t(
            config.characterPosition.x - lightingOrigin.x,
            config.characterPosition.y - lightingOrigin.y,
            config.characterPosition.z - lightingOrigin.z);
        if (!std::isfinite(config.lightingTranslation.x) ||
            !std::isfinite(config.lightingTranslation.y) || !std::isfinite(config.lightingTranslation.z))
            throw std::runtime_error("Showcase lighting translation is not finite");
        return config;
    }

    struct STAGE_SCENE
    {
        STAGE_CONFIG config;
        CMapAssetCatalog catalog;
        std::vector<MAP_RUNTIME_PLACED_ENTRY> placements;
        std::vector<MAP_RUNTIME_STATIC_BATCH_ENTRY> batches;

        ~STAGE_SCENE()
        {
            for (auto& entry : placements)
                CMapPlacementRuntime::Set_RuntimeVisible(entry, false);
            CMapPlacementRuntime::Remove_PlacementRuntime(STAGE_LEVEL, placements, batches);
        }
    };

    struct STAGE_CHARACTER
    {
        std::shared_ptr<Engine::CGameObject> object;
        std::shared_ptr<CCharacter> character;
        ~STAGE_CHARACTER()
        {
            if (character) character->Set_CinematicPresentationSuppressed(true);
            if (object)
                CGameInstance::Get().Remove_GameObject_from_Layer(STAGE_LEVEL, CHARACTER_LAYER, object);
        }
    };

    PLAYER_STANCE_ID Default_Stance(CHARACTER_CLASS_ID characterClass)
    {
        switch (characterClass)
        {
        case CHARACTER_CLASS_ID::LANCE_MASTER: return PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
        case CHARACTER_CLASS_ID::WARLORD: return PLAYER_STANCE_ID::WARLORD_NORMAL;
        case CHARACTER_CLASS_ID::GUARDIANKNIGHT: return PLAYER_STANCE_ID::GUARDIANKNIGHT_HUMAN;
        default: return PLAYER_STANCE_ID::NONE;
        }
    }

    std::unique_ptr<STAGE_CHARACTER> Create_Character(
        CHARACTER_CLASS_ID characterClass, const STAGE_CONFIG& config)
    {
        const CHARACTER_SPEC* spec = CCharacterCatalog::Find_Spec(characterClass);
        if (!spec)
            throw std::runtime_error("Showcase class has no default character spec");
        auto result = std::make_unique<STAGE_CHARACTER>();
        CCharacter::CHARACTER_DESC desc{};
        desc.iPrototypeLevelIndex = STAGE_LEVEL;
        desc.pSpec = spec;
        desc.eCharacterClass = characterClass;
        desc.fSpeedPerSec = 6.f;
        desc.fRotationPerSec = 180.f;
        desc.vPosition = config.characterPosition;
        desc.isLocallyControlled = false;
        if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(STAGE_LEVEL,
            L"Prototype_GameObject_Character", STAGE_LEVEL, CHARACTER_LAYER, &desc, &result->object)))
            throw std::runtime_error("Showcase default character clone failed");
        result->character = std::dynamic_pointer_cast<CCharacter>(result->object);
        if (!result->character || !result->character->Get_Transform())
            throw std::runtime_error("Showcase default character is incomplete");
        auto& character = *result->character;
        character.Set_CinematicPresentationSuppressed(true);
        character.Apply_NetworkStance(Default_Stance(characterClass));
        if (!character.Set_Animation(CHARACTER_ANIM::IDLE, true))
            throw std::runtime_error("Showcase default idle animation is unavailable");
        // This display clone has no network yaw. Keep cloth/hair chain yaw in the
        // same basis as its world transform without enabling creation-only parts.
        character.Set_CreationPreviewYawOffset(config.characterYaw);
        character.Get_Transform()->Rotation(0.f, config.characterYaw, 0.f);
        character.Set_Position(XMVectorSet(config.characterPosition.x,
            config.characterPosition.y, config.characterPosition.z, 1.f));
        return result;
    }
}

struct CCharacterSelectShowcase::STATE
{
    std::unique_ptr<STAGE_SCENE> scene;
    std::unique_ptr<STAGE_CHARACTER> display;
    std::weak_ptr<CCharacter> suppressedReplica;
    bool replicaWasSuppressed = false;
    bool holdsSuppression = false;
    bool visible = false;
    std::weak_ptr<CCharacter> failedApprovedCharacter;
    CHARACTER_CLASS_ID failedClass = CHARACTER_CLASS_ID::END;
    std::string failureStatus;
};

CCharacterSelectShowcase::CCharacterSelectShowcase() : m_State(std::make_unique<STATE>()) {}
CCharacterSelectShowcase::~CCharacterSelectShowcase() { Clear(); }

bool CCharacterSelectShowcase::Show(const CMapPlacementRuntime& sourceMap,
    const std::shared_ptr<CCharacter>& approvedCharacter, std::string& status)
{
    status.clear();
    if (!approvedCharacter)
    {
        Hide();
        return false;
    }
    const CHARACTER_CLASS_ID characterClass = approvedCharacter->Get_CharacterClass();
    if (m_State->failedApprovedCharacter.lock() == approvedCharacter &&
        m_State->failedClass == characterClass)
    {
        Hide();
        status = m_State->failureStatus;
        return false;
    }
    if (!CPlayableCharacterAssetService::Is_Ready(STAGE_LEVEL, characterClass))
    {
        Hide();
        return false;
    }
    try
    {
        std::unique_ptr<STAGE_SCENE> stagedScene;
        STAGE_SCENE* scene = m_State->scene.get();
        if (!scene)
        {
            stagedScene = std::make_unique<STAGE_SCENE>();
            stagedScene->config = Read_Config(sourceMap);
            stagedScene->catalog = sourceMap.Get_Catalog();
            stagedScene->placements.resize(stagedScene->config.placements.size());
            for (size_t index = 0; index < stagedScene->placements.size(); ++index)
            {
                const auto& record = stagedScene->config.placements[index];
                const auto* asset = stagedScene->catalog.Find(record.assetId);
                if (!asset || asset->materialOverrides.empty())
                    throw std::runtime_error("Showcase floor or star material is unavailable");
                auto materials = asset->materialOverrides;
                for (auto& material : materials)
                {
                    // PBR adds the reflection texture after diffuse brightness;
                    // direct/indirect specular also survives a black diffuse.
                    // Only these display clones receive the black stage variant.
                    material.surface.diffuseBrightness = 0.f;
                    material.surface.reflectionIntensity = 0.f;
                    material.surface.specularPBRIntensity = 0.f;
                }
                if (!CMapPlacementRuntime::Create_Placement(STAGE_LEVEL, stagedScene->catalog,
                    record, stagedScene->placements[index], {}, &materials))
                    throw std::runtime_error("Showcase floor or star clone failed");
            }
            scene = stagedScene.get();
        }
        if (!m_State->display || m_State->display->character->Get_CharacterClass() != characterClass)
        {
            auto stagedCharacter = Create_Character(characterClass, scene->config);
            Hide();
            if (stagedScene) m_State->scene = std::move(stagedScene);
            m_State->display = std::move(stagedCharacter);
            m_State->failedApprovedCharacter.reset();
            m_State->failureStatus.clear();
            /* Level Update follows Object Update. Keeping the clone hidden for
            this first frame lets the standard next update form every part's
            combined world and animated pose before normal render submission. */
            return false;
        }

        if (m_State->holdsSuppression && m_State->suppressedReplica.lock() != approvedCharacter)
            Hide();
        for (auto& entry : m_State->scene->placements)
            if (!CMapPlacementRuntime::Set_RuntimeVisible(entry, true))
                throw std::runtime_error("Showcase placement visibility failed");
        if (!m_State->holdsSuppression)
        {
            m_State->suppressedReplica = approvedCharacter;
            m_State->replicaWasSuppressed = approvedCharacter->Is_CinematicPresentationSuppressed();
            m_State->holdsSuppression = true;
        }
        approvedCharacter->Set_CinematicPresentationSuppressed(true);
        m_State->display->character->Set_CinematicPresentationSuppressed(false);
        m_State->visible = true;
        m_State->failedApprovedCharacter.reset();
        m_State->failureStatus.clear();
        return true;
    }
    catch (const std::exception& error)
    {
        Hide();
        m_State->failedApprovedCharacter = approvedCharacter;
        m_State->failedClass = characterClass;
        m_State->failureStatus = std::string("Character Select showcase unavailable: ") + error.what();
        status = m_State->failureStatus;
        return false;
    }
}

void CCharacterSelectShowcase::Hide()
{
    m_State->visible = false;
    if (m_State->display)
        m_State->display->character->Set_CinematicPresentationSuppressed(true);
    if (m_State->scene)
        for (auto& entry : m_State->scene->placements)
            CMapPlacementRuntime::Set_RuntimeVisible(entry, false);
    if (m_State->holdsSuppression)
    {
        if (auto character = m_State->suppressedReplica.lock())
            character->Set_CinematicPresentationSuppressed(m_State->replicaWasSuppressed);
        m_State->suppressedReplica.reset();
        m_State->holdsSuppression = false;
    }
}

void CCharacterSelectShowcase::Leave()
{
    Hide();
    // Suppression only skips rendering. Remove the display clone from its Layer
    // so trial/customizing/movies do not keep evaluating an invisible character.
    // The two static floor placements stay cached for the next preview.
    m_State->display.reset();
}

void CCharacterSelectShowcase::Clear()
{
    Leave();
    m_State->scene.reset();
    m_State->failedApprovedCharacter.reset();
    m_State->failedClass = CHARACTER_CLASS_ID::END;
    m_State->failureStatus.clear();
}

std::shared_ptr<CCharacter> CCharacterSelectShowcase::Get_Character() const
{
    return m_State->display ? m_State->display->character : nullptr;
}

bool CCharacterSelectShowcase::Is_Visible() const { return m_State->visible; }

float3_t CCharacterSelectShowcase::Get_LightingTranslation() const
{
    return m_State->scene ? m_State->scene->config.lightingTranslation : float3_t{};
}
