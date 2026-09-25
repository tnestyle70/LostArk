#pragma once

#include "CompositionAnimationResource.h"
#include "CompositionEditing.h"

#include <array>
#include <cstdint>
#include <string>

namespace Client
{

/* The Kouku entries are the arena gates: one authored composition serves
   them all, so they share a session and only change which bosses it lists. */
enum class COMPOSITION_WORKBENCH_BOSS : std::uint8_t
{
    VALTAN,
    KOUKU_SAYDON,
    KOUKU_SAYDON_GATE2,
    KOUKU_SAYDON_GATE3,
    KOUKU_SAYDON_ENCORE,
};

// The target selects an authoring owner; gameplay class and boss IDs remain separate.
enum class COMPOSITION_WORKBENCH_TARGET : std::uint8_t
{
    BOSS, CHARACTER, OBJECT, SEQUENCE, WORLD
};

// Shared resource ordering is independent from each document's typed owner.
enum class COMPOSITION_RESOURCE_DOMAIN : std::uint8_t
{
    ANIMATION, LOGIC, SUMMON, WORLD, SCENE_PROFILE, EFFECT, COLLIDER,
    SOUND, CAMERA, LIGHT, PATTERN, COUNT
};
inline constexpr std::array<const char*, static_cast<std::size_t>(COMPOSITION_RESOURCE_DOMAIN::COUNT)>
    COMPOSITION_RESOURCE_CATEGORIES = {
        "Animation", "Logic", "Summon", "World", "Scene Profile", "Effect", "Collider",
        "Sound", "Camera", "Light", "Pattern" };

enum class COMPOSITION_WORKBENCH_PANE : std::uint8_t
{
    SEQUENCER,
    PATTERNS,
    RESOURCES,
    DETAILS,
    PREVIEW,
    BOSS_PATTERN,
    TOOLBAR,
    COUNT,
};

struct COMPOSITION_WORKBENCH_VIEW_REQUEST final
{
    bool showResources = false;
    bool focusResources = false;
    bool expandResources = false;
    bool showPatterns = false;
    bool focusPatterns = false;
    bool maximizeSequencer = false;
    bool restoreSequencer = false;
    bool resetLayout = false;
};

// Each boss keeps its own document, draft, selection and save owner. The shell
// owns every top-level window and calls these methods on one session per frame.
// Deferred saves and selections run only after all panes release the frame view.
class ICompositionWorkbenchSession
{
public:
    virtual ~ICompositionWorkbenchSession() = default;
    virtual void On_WorkbenchDeactivated() {}
    virtual void Begin_WorkbenchFrame() = 0;
    virtual void Render_WorkbenchPane(COMPOSITION_WORKBENCH_PANE pane) = 0;
    virtual void End_WorkbenchFrame() = 0;
    virtual COMPOSITION_WORKBENCH_VIEW_REQUEST Consume_WorkbenchViewRequest() { return {}; }
    /* The shell picked one boss entry; a session serving several entries
       narrows what it lists. Sessions with one entry ignore it. */
    virtual void Select_WorkbenchBoss(COMPOSITION_WORKBENCH_BOSS) {}
    virtual bool Execute_CompositionEdit(COMPOSITION_EDIT_COMMAND, std::string& status)
    {
        status = "Select an editable resource or timeline box in this session.";
        return false;
    }
    virtual bool Insert_CompositionTransfer(const COMPOSITION_TRANSFER& transfer, std::string& status)
    {
        if (const auto animation = std::dynamic_pointer_cast<const COMPOSITION_ANIMATION_TRANSFER>(transfer))
            return Append_CompositionAnimationResource(animation->resource, false, status);
        status = "This session cannot insert the selected resource type.";
        return false;
    }
    virtual bool Can_AppendCompositionAnimationResource(
        const COMPOSITION_ANIMATION_RESOURCE&, bool, std::string& status) const
    {
        status = "The selected action session does not accept physical animation rows.";
        return false;
    }
    virtual bool Append_CompositionAnimationResource(
        const COMPOSITION_ANIMATION_RESOURCE&, bool, std::string& status)
    {
        status = "The selected action session does not accept physical animation rows.";
        return false;
    }
};

}
