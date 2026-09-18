# 쿠크 패턴 재생 연결 및 저작 데이터 복구 계획

## 목표와 확인한 원인
현재 저장본을 기준으로 공굴리기 회전·불 앵커, 입 본 선택, 피자 10명 실제 패턴 소환, 왼손 트레일, 칼날 이동·효과·Resources를 끝까지 연결한다. 사용자 방금 저장한 수명과 effect scale/model 제거를 보존한다.

- 피자 summon의 patternSpawns가 비어 있고 서버 게시본에도 해당 트리거가 없다.
- 본 선택은 Composition의 실제 CNpc 모델 대신 전역 Animation Tool 선택에 묶여 있다. BONE 지원 코드는 있으나 큰 세이튼 occurrence의 bone은 비어 있다.
- 추적 이동의 회전 속도가 전체 duration에 종속되어 수명을 늘리면 조준도 늦어진다.
- 공 템플릿은 바깥 WORLD 박스보다 먼저 hidden된다. 불·왼손 트레일도 바깥 duration만으로 원본 방출 시간이 늘어나지 않는다.
- 왼손 흰 carrier만 +100cm 원본 위치와 segment clipping이 남아 있다.

## 구현 단위
1. 기존 CModel/CNpc owner의 typed model-target query를 Workbench에 연결한다. Preview member와 실제 replicated actor를 profile/placement로 검증하고 BODY/WEAPON 목록을 제공한다. 다른 도구 전역 선택에 의존하지 않는다.
2. 추적 이동만 180도/초의 최단 회전 후 전진을 Preview/Server 양쪽에 적용한다. 이동 없는 기존 회전 창의 deadline 보간은 보존한다.
3. 현재 피자 summon stable occurrence에 실측한 2관문 table 좌우 각5명 MAP 배치를 넣고 child/parent 생존창을 맞춘다. 실제 child의 animation/effect/JUMP/SLAM 경로를 소비한다.
4. 큰 세이튼과 공굴리기 불은 실제 skeleton 본 basis와 원본 source 축을 확인해 입 anchor에 연결한다. 반복 방출은 기존 occurrence loopEffectToDuration 계약으로 box 수명에 맞춘다.
5. 왼손 white source 위치0/clip 해제를 오른손과 맞춘다. 현재 저장된24891ms 박스 수명은 보존한다.
6. World blade emission은 보존본과 latest를 stable ID로 비교 복구한다. 수정된 source blade effect를 Object anchor로 연결한다. 기존 hook은 보존하고 원본 hook/fatal blade를 새 append resource로 추가한다.

## 데이터 반영
최신 디스크 저장본을 반영하라는 사용자 요청으로 승인됐다. 파일별 최신 hash 재확인, backup, field별 병합, 원자 교체를 사용한다. 사용자 scale/내부 mesh 제거/무관한 최신 필드를 보존한다. publisher가 생성물을 만들며 실행 중 메모리 Reload를 자동 수행하지 않는다.

## 검증과 완료 경계
기존 codec/projector와 Server 실제 소비자 focused 검증, 변경 TU 컴파일, JSON parse, git diff --check를 수행한다. 현재 코드→원본→게시본 연결을 각각 검증한다. Client 실행·화면 조작은 하지 않는다. 실제 화면과 실행 중 process의 새 binary/data 수신 여부는 별도로 보고한다. 새 C++ 파일 추가 계획 없음; 기존 vcxproj/filter 등록 변경 없음.

## G07. 쇼타임 Seek 비용과 후속 저작 연결 (2026-09-18)

최신 사용자 Profiler frame451~458은 CPU 1808~2170ms 중 Effect.Service.Update와 HistoryUpdate가 1644~1994ms다. Preview exactRoot가 각 이펙트의 시점마다 Sample_BundlePreviewPose에서 처음부터 30Hz 이벤트를 평가하는 경로를 수정한다.

- `KoukuSaydonPresentationPlayer.h/.cpp`: member 수명의 facing checkpoint가 yaw, animation별 yaw, follow XZ를 보존한다. 같은 시각의 stage/추적 이벤트 전체가 끝난 상태만 저장하고 이전 checkpoint부터 재개한다. 미관측 target 입력은 계속 실패하며 seek로 실제 배우 pose를 바꾸지 않는다. 배치 수정 시 cache를 비운다.
- `KoukuSaydonActionWorkbench.h/.cpp`, `MainApp.cpp`, PresentationPlayer: Summon Box Detail의 위치/yaw를 typed 요청으로 기존 preview clone에 적용한다. 열 명 전부 재생성하지 않고 대상 clone만 갱신한다.
- `Shared`의 공용 추적 회전 계산과 Server/Preview: 회전 전용의 남은 tick 보간 gain을 1에서10으로 올리고 목표각 초과를 막는다. 이동추적180도/초는 유지한다. 신규 header는 Shared vcxproj와 filters에 등록한다.
- `EffectAuthoringSequencer.h/.cpp`: finite source 반복과 EmitterLoops0의 방출 연장을 구분하여 Composition player와 같은 occurrence lifetime을 소비한다. 공통 불 asset은 복제하지 않는다.
- World 배우가 이미 animation을 소유한 세 연출은 실제 World clip/time을 읽기전용 timeline과 detail에 보여준다. 별도 boss 애니메이션을 중복 생성하지 않는다.
- 화이트의 액션B 15333ms 후보는 최신 저장본의 stable ID별 필드로 준비한다. 이번 후속 데이터 반영 승인은 후보 완성 후 한 번 확인하며 이전 작업의 승인을 새 반영 승인으로 간주하지 않는다.

## G08. 장판·Collider·Sound 설계 범위

현재 병목 수정과 별개로 기존 Duration/CombatObject/visual template 경계를 재사용한다. 하나의 gameplay 발생이 stable ID, 생성시점, 위치/방향, 판정창을 소유하고 Client는 동일 clock에서 Effect 및 Sound를 표현한다. 장판의 시각 요소별 Collider 생성이나 하나의 거대 Effect로 평탄화는 하지 않는다. 기존 random/fixed/tracking 소비자의 입력 누락을 조사하여 RESULT에 실제 지원 범위와 후속 구현을 구분한다.

검증은 기존 native spatial preview probe의 순방향/역방향/미래 미관측 실패와 cache 동등성, finite loop 소비, 최소 컴파일, JSON/XML parse와 diff check다. Product 링크는 실제 출력 점유 여부를 먼저 확인한다. Client/UI 실행 및 화면 판정은 사용자가 한다.

### G08의 실제 경계와 구현 순서

현재 `SHOWTIME_PLAYER_TARGETS`는 fixedSelectionGroupId, trackingPresentationOccurrenceId, randomVolleyOccurrenceSets를 이미 소비한다. 현재 logic64에는6개 랜덤 세트·500ms 간격·16m 반경이 저장돼 있다. `_showtime_visual_template`은 EFFECT만 허용하고, Server가 만드는 showtime CombatObject는 PresentationPulses만 갖고 Hits는 비어 있다. 이 제한 때문에 시각 그룹에 Collider나 Sound를 넣어도 현재 경로에서 제품 판정/소리가 생기지 않는다.

권장 소유 구조는 다음과 같다.

| 소유자 | 저장·소비할 값 | 기존 연결 |
|---|---|---|
| Duration 발생 규칙 | 시작/종료, 생성 간격, 고정·선택 플레이어·추적·랜덤 위치 정책 | SHOWTIME_PLAYER_TARGETS 및 기존 ground capture |
| 공격1회 정의 | stable definition ID, 상대 Effect/Sound 시각, 상대 pivot, 의미별 hit window | 현재 visual template을 stage→validate→commit하는 publisher |
| Server instance | instance ID, spawn tick, 고정된 위치/방향, 정의 revision, HitId별 피해 정책 | CCombatObjectRuntime / BOSS_COMBAT_OBJECT_DEFINITION::Hits |
| Client presentation | 같은 instance ID와 spawn tick으로 Effect/Sound 표현 | TargetedCombatVisual → PresentationPlayer::Sample |

다음 구현은 기존 `BOSS_COMBAT_OBJECT_HIT`의 TIMED/CONTACT, CIRCLE/RING/CONE/BOX와 DamageProfile을 공격 정의에 연결하고, 같은 spawn tick과 pinned revision으로 Server에서 평가한다. 예고 장판, 타격, 잔불은 하나의 공격 발생의 서로 다른 시각/판정창이다. 빛·연기·파편 등의 렌더 요소마다 Collider를 만드는 구조는 사용하지 않는다.

Sound는 같은 공격 정의의 cue로 확장한다. loop는 seek 위치에서 복원하고, one-shot은 instance+cue+cycle별 최초 forward crossing에서만 재생한다. seek가 과거 소리를 일괄 재생하지 않게 하고 중단/rewind에는 기존 handle을 정리한다. 현재 EFFECT-only validation을 넓히기 전에 실제 publisher, Server hit 소비, Client sound 소비를 같은 기능 단위로 연결해야 한다. 본 작업에서는 구조를 설계하며 임의 피해량·판정창·sound asset을 생성하지 않는다.

노란 장판 전체를 지우지 않는다. 고정 authored 공격과 랜덤 반복 발생을 구분하고, 반복 세트만 공통 정의로 만든다. MAP 지면과 BOSS 총구 anchor는 한 root로 합치지 않는다. UI에서 묶어 접는 저작 단위와 GPU 배칭/공유 리소스는 별개이며 stable child ID를 유지한다. 먼저 이번 seek 병목 수정 후 사용자 profiler 재캡처로 남은 비용을 측정한다.

### G07 신규 공용 회전 계산 전체 코드

`Shared/Public/Gameplay/KoukuTargetTracking.h`는 Server와 Preview의 회전 전용 응답 상수와 경과 tick 합성을 공유한다. Shared 프로젝트의 Gameplay include/filter에 등록한다. 이동 속도·피해·목표 선정을 소유하지 않는다.

```cpp
#pragma once

#include <algorithm>
#include <cstdint>

namespace LostArk::Shared::KoukuTargetTracking
{
// Rotate-only tracking initially consumes ten times the old remaining-arc
// fraction. Movement-enabled tracking keeps its independent 180 deg/s limit.
inline constexpr std::uint32_t ROTATE_ONLY_RESPONSE_SCALE = 10u;

inline double RotateOnlyFraction(const std::uint64_t elapsedTicks,
    const std::uint64_t remainingTicks) noexcept
{
    if (!elapsedTicks || !remainingTicks) return 0.0;
    const auto elapsed = (std::min)(elapsedTicks, remainingTicks);
    const auto gain = (std::min<std::uint64_t>)(ROTATE_ONLY_RESPONSE_SCALE, remainingTicks);
    if (elapsed >= remainingTicks - gain + 1u) return 1.0;
    // Product of (remaining - gain) / remaining for every elapsed fixed tick,
    // telescoped to at most gain factors. Grouped updates equal individual ticks.
    double retained = 1.0;
    for (std::uint64_t index = 0u; index < gain; ++index)
        retained *= double(remainingTicks - elapsed - index) / double(remainingTicks - index);
    return 1.0 - retained;
}
}
```

G07 데이터 실측 보완: 최신 저장본 revision1720의 화이트(P90) `presentation.7`이 참조하는 `쿠크_훌라후프액션B`는 이미15333ms다. 동일 값을 다시 쓰거나 내부 emitter 수명을 임의 변경하지 않는다. 해당 값과 불뿜기 네 occurrence lifetime을 보존하며 이번 후속 작업은 authored JSON 변경 없이 진행한다.
