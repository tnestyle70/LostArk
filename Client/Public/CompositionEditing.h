#pragma once

// Animation 전달체는 자원 정보를 포인터가 아닌 값 멤버로 소유하므로 완전한 struct 정의가 필요하다.
#include "CompositionAnimationResource.h"

#include <array>
#include <cstdint>
// 도구별 capture/paste/duplicate 구현을 closure로 받는다. 공용 dispatch가 구체 Workbench 헤더를 포함하지 않게 한다.
#include <functional>
// 복사한 값을 여러 도구와 clipboard가 같은 수명으로 보유하는 shared_ptr를 사용한다.
#include <memory>
// 입력이 편집 명령이 아닌 경우를 nullopt로 표현한다. 없는 명령을 임의 enum 값으로 실행하지 않는다.
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace Client
{
enum class COMPOSITION_EDIT_COMMAND : std::uint8_t { COPY, PASTE, DUPLICATE_SELECTION };
//editor를 기준으로 적용할 수 있는 input 상태를 모아둔 구조체
struct COMPOSITION_EDIT_INPUT
{
    bool focused = false, control = false, textInput = false, activeItem = false;
    bool popup = false, dragging = false;
    bool copyPressed = false, pastePressed = false, duplicatePressed = false;
};

// 입력 상태를 읽어 명령 한 개 또는 없음으로 해석하는 순수 함수다. 실제 clipboard/문서 변경은 아래 dispatch와 대상 writer가 맡는다.
inline std::optional<COMPOSITION_EDIT_COMMAND> Resolve_CompositionShortcut(const COMPOSITION_EDIT_INPUT& input)
{
    //단축키 호출 API
    if (!input.focused || !input.control || input.textInput || input.activeItem || input.popup || input.dragging)
        return std::nullopt;
    if (input.copyPressed) return COMPOSITION_EDIT_COMMAND::COPY;
    if (input.pastePressed) return COMPOSITION_EDIT_COMMAND::PASTE;
    if (input.duplicatePressed) return COMPOSITION_EDIT_COMMAND::DUPLICATE_SELECTION;
    return std::nullopt;
}

// 전달 snapshot은 저작 값을 소유한다. 파생 snapshot에 도구나 문서 행의 포인터, preview 객체,
// 오디오 handle 또는 수정 가능한 원본 문서를 보관하면 원본 변경·종료 뒤 붙여넣기의 수명이 깨진다.
struct COMPOSITION_TRANSFER_SNAPSHOT
{
    virtual ~COMPOSITION_TRANSFER_SNAPSHOT() = default;
    // 파생체가 literal 타입 이름을 반환한다. string_view는 그 문자를 소유하지 않으며 snapshot 종류의 설명/검증에 쓰인다.
    virtual std::string_view Type() const noexcept = 0;
    std::string label;
};
//composition transfer가 의미하는 게 뭐지? 어떤 거를 의미하는 거고 왜 shared_ptr로 선언이 되어있는 걸까?
// 여기서 transfer는 복사한 저작 값의 묶음이다. shared_ptr는 clipboard와 붙여넣기 요청이 같은 snapshot의 수명을 공유하게 한다.
// const는 이 경로로 snapshot을 수정하지 못하게 한다. 다른 별칭의 변경을 막거나 thread 동기화를 제공하는 장치는 아니다.
using COMPOSITION_TRANSFER = std::shared_ptr<const COMPOSITION_TRANSFER_SNAPSHOT>;

struct COMPOSITION_ANIMATION_TRANSFER final : COMPOSITION_TRANSFER_SNAPSHOT
{
    COMPOSITION_ANIMATION_RESOURCE resource;
    std::string_view Type() const noexcept override { return "animation.resource.v1"; }
};

// 공용 Effect 참조는 실제 V1/V2 식별을 유지한다. 붙여넣는 쪽이 anchor나 시간 정책을 지원하지 않으면
// 그 필드를 조용히 버리지 않고 기존 writer의 검증에서 거부해야 한다.
struct COMPOSITION_EFFECT_ITEM
{
    // World Sequence와 공유하는 종류 식별값은 V1_EFFECT, LEAF, GROUP이다.
    std::string resourceKind = "V1_EFFECT", resourceId, displayName;
    // startMs는 첫 복사 항목 기준 상대 ms다. durationMs가 0이면 원래 자원 수명을 사용한다.
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

// shared_ptr 멤버로만 보유하므로 여기서는 전방 선언이면 충분하다. 내용에 접근하는 구현에서 실제 정의를 포함한다.
struct WORLD_SEQUENCE_OBJECT_BUNDLE;

struct COMPOSITION_WORLD_OBJECT_TRANSFER final : COMPOSITION_TRANSFER_SNAPSHOT
{
    std::shared_ptr<const WORLD_SEQUENCE_OBJECT_BUNDLE> bundle;
    bool selectedMotionsOnly = false;
    std::string_view Type() const noexcept override { return "world.object.bundle.v1"; }
};

// 편집 UI가 쓰는 process-local clipboard다. raw row pointer 대신 불변 값 snapshot을 보관한다.
// Read가 반환한 shared_ptr의 참조를 장기 보관하지 않는다. Write/Clear가 그 멤버를 교체할 수 있으며 자체 동기화는 없다.
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

// 복사는 소유자가 유효한 snapshot을 만든 뒤에만 공용 clipboard를 교체한다.
// 붙여넣기·복제는 목적지의 기존 writer를 통해 검증하고 commit한다.
// callback은 각 세션의 private 검증/commit 경로를 그대로 호출한다. std::function은 다른 closure를 같은 서명으로 받는 대신 간접 호출과 경우에 따른 할당 비용이 있다.
// status는 실패 이유를 돌려주는 출력 참조다. COPY 캡처 실패는 기존 clipboard 값을 유지하고, PASTE의 rollback 책임은 목적지 writer에 있다.
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
