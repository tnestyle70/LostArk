#pragma once

#include "CompositionAnimationResource.h"

#include <cstdint>
#include <string>

namespace Client
{

enum class COMPOSITION_WORKBENCH_BOSS : std::uint8_t
{
    VALTAN,
    KOUKU_SAYDON,
};

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
    virtual void Begin_WorkbenchFrame() = 0;
    virtual void Render_WorkbenchPane(COMPOSITION_WORKBENCH_PANE pane) = 0;
    virtual void End_WorkbenchFrame() = 0;
    virtual COMPOSITION_WORKBENCH_VIEW_REQUEST Consume_WorkbenchViewRequest() { return {}; }
    virtual bool Can_AppendCompositionAnimationResource(
        const COMPOSITION_ANIMATION_RESOURCE&, bool, std::string& status) const
    {
        status = "The selected boss session does not accept physical animation rows.";
        return false;
    }
    virtual bool Append_CompositionAnimationResource(
        const COMPOSITION_ANIMATION_RESOURCE&, bool, std::string& status)
    {
        status = "The selected boss session does not accept physical animation rows.";
        return false;
    }
};

}
