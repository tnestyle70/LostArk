#pragma once

#include "CompositionAnimationResource.h"

#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace Client
{
enum class COMPOSITION_EDIT_COMMAND : std::uint8_t { COPY, PASTE, DUPLICATE_SELECTION };

struct COMPOSITION_EDIT_INPUT
{
    bool focused = false, control = false, textInput = false, activeItem = false;
    bool popup = false, dragging = false;
    bool copyPressed = false, pastePressed = false, duplicatePressed = false;
};
inline std::optional<COMPOSITION_EDIT_COMMAND> Resolve_CompositionShortcut(const COMPOSITION_EDIT_INPUT& input)
{
    if (!input.focused || !input.control || input.textInput || input.activeItem || input.popup || input.dragging)
        return std::nullopt;
    if (input.copyPressed) return COMPOSITION_EDIT_COMMAND::COPY;
    if (input.pastePressed) return COMPOSITION_EDIT_COMMAND::PASTE;
    if (input.duplicatePressed) return COMPOSITION_EDIT_COMMAND::DUPLICATE_SELECTION;
    return std::nullopt;
}

// A transfer owns authoring values. Never retain a tool, row pointer, preview
// object, audio handle, or mutable source document in a derived snapshot.
struct COMPOSITION_TRANSFER_SNAPSHOT
{
    virtual ~COMPOSITION_TRANSFER_SNAPSHOT() = default;
    virtual std::string_view Type() const noexcept = 0;
    std::string label;
};
using COMPOSITION_TRANSFER = std::shared_ptr<const COMPOSITION_TRANSFER_SNAPSHOT>;

struct COMPOSITION_ANIMATION_TRANSFER final : COMPOSITION_TRANSFER_SNAPSHOT
{
    COMPOSITION_ANIMATION_RESOURCE resource;
    std::string_view Type() const noexcept override { return "animation.resource.v1"; }
};

// Common effect references retain their real V1/V2 identity. Importers must
// reject unsupported anchor/timing policies, rather than silently dropping them.
struct COMPOSITION_EFFECT_ITEM
{
    // Canonical kinds shared with World Sequence: V1_EFFECT, LEAF, GROUP.
    std::string resourceKind = "V1_EFFECT", resourceId, displayName;
    // startMs is relative to the first copied item; zero duration is natural life.
    std::uint32_t startMs = 0u, durationMs = 0u;
    std::array<float, 3> position{}, rotationDegrees{}, scale{1.f, 1.f, 1.f};
    std::string bone;
    bool followOwner = true;
    bool inheritOwnerRotation = true;
    bool fitToDuration = false;
    bool loopToDuration = false;
};
struct COMPOSITION_EFFECT_TRANSFER : COMPOSITION_TRANSFER_SNAPSHOT
{
    std::vector<COMPOSITION_EFFECT_ITEM> items;
    std::string_view Type() const noexcept override { return "effect.occurrences.v1"; }
};

struct WORLD_SEQUENCE_OBJECT_BUNDLE;
struct COMPOSITION_WORLD_OBJECT_TRANSFER final : COMPOSITION_TRANSFER_SNAPSHOT
{
    std::shared_ptr<const WORLD_SEQUENCE_OBJECT_BUNDLE> bundle;
    bool selectedMotionsOnly = false;
    std::string_view Type() const noexcept override { return "world.object.bundle.v1"; }
};

class CCompositionClipboard final
{
public:
    static CCompositionClipboard& Get()
    {
        static CCompositionClipboard clipboard;
        return clipboard;
    }
    const COMPOSITION_TRANSFER& Read() const noexcept { return m_Value; }
    bool Write(COMPOSITION_TRANSFER value)
    {
        if (!value) return false;
        m_Value = std::move(value);
        return true;
    }
    void Clear() noexcept { m_Value.reset(); }
private:
    COMPOSITION_TRANSFER m_Value;
};

// Copy changes the shared clipboard only after the owner has captured a valid
// snapshot. Paste/duplicate commit through the destination's existing writer.
inline bool Dispatch_CompositionEdit(const COMPOSITION_EDIT_COMMAND command,
    const std::function<COMPOSITION_TRANSFER(std::string&)>& capture,
    const std::function<bool(const COMPOSITION_TRANSFER&, std::string&)>& paste,
    const std::function<bool(std::string&)>& duplicate, std::string& status)
{
    switch (command)
    {
    case COMPOSITION_EDIT_COMMAND::COPY:
        if (capture)
        {
            auto candidate = capture(status);
            if (candidate) return CCompositionClipboard::Get().Write(std::move(candidate));
            return false;
        }
        break;
    case COMPOSITION_EDIT_COMMAND::PASTE:
        if (!CCompositionClipboard::Get().Read())
        { status = "Copy a resource or timeline selection first."; return false; }
        if (paste) return paste(CCompositionClipboard::Get().Read(), status);
        break;
    case COMPOSITION_EDIT_COMMAND::DUPLICATE_SELECTION:
        if (duplicate) return duplicate(status);
        break;
    }
    status = "This selection does not support the requested edit.";
    return false;
}
}
