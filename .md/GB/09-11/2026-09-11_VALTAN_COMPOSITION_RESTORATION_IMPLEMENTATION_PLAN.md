# 발탄 Composition 패턴·World Object·V2 편집 복구 구현 계획

작성일: 2026-09-11. 기존 split source와 동일한 Composition shell을 유지한다.

## G00. 현재 실측과 변경 범위

`CSequencerTool`의 Valtan session은 이미 `CValtanActionWorkbench`다. 42개 managed pattern과
기존 Animation, Collider, V1/V2, Sound, Camera owner를 사용한다. 도넛을 포함한 combat object는
Stage의 Server spawn event와 별도 lifetime을 갖고 있지만 Effect/Collider 행에만 흩어져 있다.

V2 UI는 아직 convenience `strStage`로 clock을 구분하고 transform으로 선택 ID를 합성한다.
typed `CLIP_OCCURRENCE`가 같은 Stage의 0ms로 합쳐지고 서로 다른 저장 binding을 선택할 수 없다.

## G01. 같은 owner로 World Object 목록과 선택 연결

`ValtanActionWorkbench.h/.cpp`에 World Object lane과 resource tab을 추가한다. 기존 canonical
Pattern/Stage에 연결된 object archetype과 World event를 나열하고, owner 선택은 기존 deferred
Pattern selection의 Save/Discard/Cancel 경로를 사용한다. 선택 뒤 기존 ring radius 편집과
Pattern Preview/Server Play를 사용한다. 별도 boss·object runtime이나 저장 ID를 만들지 않는다.

World object의 origin/direction/arena anchor와 own lifetime을 표시한다. 도넛의 판정은 기존 Server
combat-object owner를 유지하고 V1으로 저작한 자산에 임의 V2 group을 덮지 않는다.

## G02. V2 stable identity와 clock 복구

`BuildEffectV2BindingStableId`는 저장된 bindingId와 정확한 clipOccurrenceId를 사용한다.
`Build_Timeline`은 pattern/stage/action scope와 `eClockBasis`를 먼저 확인한 뒤 source window와
playRate로 CLIP_OCCURRENCE를 투영한다. convenience clip/stage 문자열로 clock을 선택하지 않는다.

`EffectV2_Catalog.h/.cpp`에 exact binding replacement adapter를 추가한다. 선택 bindingId와
scope/resource는 유지하고 anchor/follow/rotation/local transform/repeat/stop/start를 stage한다.
기존 strict validate, immutable snapshot commit, baseline/CAS와 Composition Save를 재사용한다.
실패하면 기존 snapshot, selection, dirty owner를 보존한다.

Box Detail의 편집 buffer는 Apply 전까지 session state다. Apply가 typed owner mutation을 수행하고
성공한 snapshot만 기존 local preview에 전달한다.

## G03. 검증과 사용자 인계

기존 V2 projection/catalog·Composition resource regression을 변경된 계약에 맞춰 확인한다.
변경 C++ 최소 컴파일은 통합 담당자의 단일 build와 조율한다. 신규 Product C++ 파일은 없다.
기존 native admission harness에서 typed Catalog API와 실패 시 snapshot 보존을 검사하도록
기존 `EffectV2_Catalog.h/.cpp`만 harness project/filter에 등록한다. 변경 문서 UTF-8과
`git diff --check`를 확인한다.

Client/UI 실행·캡처는 하지 않는다. 사용자 확인은 `F1 → Open Action Workbench → Boss Valtan →
Composition Resources → World Objects → owner 선택 → Box Detail` 뒤
`Action Workbench → Server Playback → Play on Server`다. 일반 `Play`는 local Animation/Effect
preview이며 전체 World/Sound/Camera 재생을 안내하지 않는다.
V2는 Effect box 선택 후 Box Detail의 Apply와 Save/Reopen, occurrence 시간과 최종 화면을 직접 확인한다.
