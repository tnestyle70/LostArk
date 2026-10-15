#pragma once

// 기본 Animation 삽입 계약이 자원 값을 참조하고 derived transfer를 검사하므로 해당 타입 정의가 필요하다.
#include "CompositionAnimationResource.h"
// 공용 편집 명령과 다형적 snapshot을 연결한다. inline 기본 구현의 dynamic_pointer_cast도 여기서 타입을 알아야 한다.
#include "CompositionEditing.h"

// category 개수는 enum COUNT로 고정된다. 힙의 가변 목록 대신 고정 배열로 표시 순서를 보관한다.
#include <array>
// enum의 표현 폭을 uint8_t로 명시한다. 이 UI enum 자체를 네트워크 packet 형식으로 쓰는 것은 아니다.
#include <cstdint>
// 실패 이유를 호출자의 status에 남긴다. UI에 보이는 설명 문자열과 저장 ID는 구별한다.
#include <string>

// Client의 저작 계약임을 이름으로 구분한다. namespace는 객체나 별도 메모리 소유자를 만들지 않는다.
namespace Client
{

// 쿠크 항목은 아레나 관문 선택이다. 하나의 저작 Composition을 같은 세션에서 편집하고, 관문에 따라 표시할 보스 목록만 바꾼다.
enum class COMPOSITION_WORKBENCH_BOSS : std::uint8_t
{
    VALTAN,
    KOUKU_SAYDON,
    KOUKU_SAYDON_GATE2,
    KOUKU_SAYDON_GATE3,
    KOUKU_SAYDON_ENCORE,
};

// 편집 대상은 저작 상태의 소유자를 고르는 값이다. gameplay의 직업 ID나 보스 ID와 혼용하지 않는다.
enum class COMPOSITION_WORKBENCH_TARGET : std::uint8_t
{
    BOSS, CHARACTER, OBJECT, SEQUENCE, WORLD
};

// 공용 자원 목록의 표시 순서다. 각 문서가 소유하는 typed 데이터의 저장 순서를 대신하지 않는다.
enum class COMPOSITION_RESOURCE_DOMAIN : std::uint8_t
{
    ANIMATION, LOGIC, SUMMON, WORLD, SCENE_PROFILE, EFFECT, COLLIDER,
    SOUND, CAMERA, LIGHT, PATTERN, COUNT
};
// constexpr는 고정 초기값을 컴파일 시점에 사용할 수 있게 하고, inline 변수는 여러 CPP가 이 헤더를 포함해도 같은 정의를 허용한다.
// 화면 label은 문자열 literal을 가리킨다. 입력창처럼 문자를 수정하거나 매 프레임 문자열을 새로 만들지 않는다.
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

// 세션이 창을 직접 열지 않고 값 요청을 반환한다. shell이 Consume 호출 뒤 표시 정책을 적용하므로 문서와 창 배치의 의존을 분리한다.
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

// 문서·draft·선택·저장은 각 보스 세션이 소유한다. 공용 shell은 최상위 창의 배치와 표시를 소유하고
// 선택한 세션에 한 프레임의 Begin, pane별 Render, End 호출을 전달한다.
// 저장·선택 변경을 지연하는 구현은 모든 pane가 이번 프레임의 행 참조를 놓은 뒤 이를 처리한다.
// C++의 별도 interface 문법이 아니라 virtual 함수로 호출 계약을 정한 추상 클래스다.
// shell은 이 타입만 알면 보스별 구현을 호출할 수 있다. 문서 구조까지 같은 상속 모델로 강제하지 않는다.
class ICompositionWorkbenchSession
{
public:
    // unique_ptr<ICompositionWorkbenchSession>가 소유하는 세션도 있어 base pointer를 통해 derived 소멸자를 호출할 수 있어야 한다.
    virtual ~ICompositionWorkbenchSession() = default;
    virtual void On_WorkbenchDeactivated() {}
    // = 0인 세 함수는 세션이 구현해야 한다. 비어 있는 기본 hook은 해당 기능이 없는 세션도 같은 호출 순서를 따르게 한다.
    virtual void Begin_WorkbenchFrame() = 0;
    virtual void Render_WorkbenchPane(COMPOSITION_WORKBENCH_PANE pane) = 0;
    virtual void End_WorkbenchFrame() = 0;
    virtual COMPOSITION_WORKBENCH_VIEW_REQUEST Consume_WorkbenchViewRequest() { return {}; }
    // shell이 보스 항목을 고르면 여러 관문을 담당하는 세션은 표시 범위를 좁힌다. 단일 항목 세션은 기본 no-op을 사용한다.
    virtual void Select_WorkbenchBoss(COMPOSITION_WORKBENCH_BOSS) {}
    virtual void Select_WorkbenchTarget(COMPOSITION_WORKBENCH_TARGET) {}
    // 지원하지 않는 명령은 false와 상태 문구를 반환한다. shell이 임의로 원본 문서를 고치는 fallback은 없다.
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
