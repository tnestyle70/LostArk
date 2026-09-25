# 공용 Sequencer 리소스 복사와 편집 명령 구현 계획

## G00. 목표와 현재 계약

Boss, Valtan, Character, Object가 같은 Resources와 Sequencer에서 선택·드래그·복사·붙여넣기
명령을 사용하도록 연결한다. 현재 공통 ICompositionWorkbenchSession/CSequencerTool은 창과
Animation append를 공유하며 CompositionTimeline은 그리기·시간 계산만 공유한다.
실제 문서·검증·저장·실행은 각 typed owner가 유지한다. 새로운 Client gameplay 실행기를 만들지 않는다.

복사는 원본과 독립인 저작 값 snapshot을 만들고 붙여넣기는 새로운 stable ID를 발급한다.
Animation model/rig, anchor, gameplay 지원 범위를 검사하며 실패하면 기존 초안 전체를 보존한다.
실행 중 model/effect/sound handle과 수정 가능한 vector index를 clipboard에 저장하지 않는다.
공통 명령은 세션과 generation을 검사하고 한 후보를 검증한 후 한 번에 commit한다.

## G01. World Object와 Motion 전체 복제

WorldSequenceDocument.h/.cpp에 값으로 소유하는 Object bundle의 Capture/Paste API를 추가한다.
bundle은 모델 resource, 선택한 root Motion ID, template와 instance를 소유한다. 선택한 Motion이
없으면 해당 Object의 직접 연결 Motion을 모두 캡처하고 NEXT 연결의 닫힌 집합을 함께 포함한다.
모델 없는 합성 alias와 실제 Map placement 복제는 이 모델 Object API에서 이유를 표시해 거절한다.

새 Object 붙여넣기와 기존 빈 Object 채우기를 지원한다. 후자는 목적지 이름·ID·parentId를 유지한다.
기존 모델 Object에 Motion을 더할 때에는 model/donor/animated/anchor의 호환성을 검사한다.
Object/template/instance ID와 binding·NEXT·default Motion을 함께 재배선한다. animation, transform,
effect, collider, sound, subtitle, emission, material과 combatBody 값은 보존한다.
공유 Effect asset과 모델 파일은 ID로 참조하며 내용을 암묵적으로 복사하거나 변경하지 않는다.

기존 다른 미완성 Create Object 초안 때문에 복제를 막지 않으면서, 가져온 bundle은 독립적인
완전한 문서 projection으로 검사한다. 전체 후보의 계층과 대상 참조도 검사한 후 commit한다.
기존 Save의 전체 validate·readback·CAS·rollback·publisher 경로는 유지한다.

## G02. 검증과 반영 경계

실제 원본_갈고리의 15 emission, 4 animation, HOOK_CAPTURE/b_hook_01/grip 보존과 원본 불변,
새 ID·NEXT remap, 독립적인 위치·방향·개수 편집, 실패 후보 보존 및 Save/Load를 검증한다.
공통 명령은 선택/focus/stale generation과 혼합 명령 실패의 원자성을 검사한다.
기존 owner의 실제 호출 경로를 사용하며 Character 기본 화면은 EffectAuthoringSequencer 경로다.

현재 저작 파일을 임의 재배치하거나 publish하지 않는다. 코드 후보의 컴파일·단위 검증을 먼저
완료한다. Client 실행과 화면 입력·시각 판정은 사용자가 직접 수행한다. 새 C++ 파일이 생기면
Client.vcxproj와 filters에 필요한 항목만 등록하고 정상 Product Build로 확인한다.

## G03. 공통 편집 API와 owner 연결

CompositionEditing.h는 immutable snapshot, 하나의 clipboard, focus를 검사하는 shortcut resolver와
Copy/Paste/Duplicate dispatch를 소유한다. 새 헤더는 Client.vcxproj와 filters의 기존 Sequencer
그룹에만 등록한다. 정상 Debug의 object section 한도를 넘는 KoukuSaydonActionWorkbench.cpp에만
/bigobj를 적용한다. ICompositionWorkbenchSession은 실행·삽입 virtual API를 제공한다.
SequencerTool은 모든 pane의 참조 사용을 마친 뒤 owner API를 호출한다. Resource drag payload는
snapshot 자체 포인터 대신 token을 전달하며 실제 snapshot은 값 소유로 수명을 유지한다.

WorldObjectTool은 document bundle API, Kouku는 기존 timeline candidate clone, Character는 실제
EffectAuthoringSequencer의 authoring row와 runtime handle 초기화 경로를 사용한다. Valtan은
Balance/Effect V2/Sound 등 기존 typed owner의 candidate stage/commit을 연결한다. Valtan의
독립 Summon 복제는 기존 정의와 visual/spawn template을 복사하고 모든 관련 stable ID를 재발급하는
CLONE_COMBAT_OBJECT writer op로 저장한다. 게시된 제품 데이터는 직접 수정하지 않는다.

공통 Effect 값은 source의 owner 전용 정책을 유실하지 않도록 완전히 표현 가능한 경우만
다른 owner가 소비한다. 서로 다른 전투 Logic 조립과 Albion 장판의 도끼 부착은 이 편집 API의
구현 완료와 구분한다. 현재 발탄 도끼는 생성 위치 고정이며 이동 추적을 새로 켜지 않는다.

## G04. 새 공통 헤더 전체 코드

```cpp
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

```
