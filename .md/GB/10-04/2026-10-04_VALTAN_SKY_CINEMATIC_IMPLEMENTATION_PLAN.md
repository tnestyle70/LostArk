# 발탄 상승 컷신의 맵 고정 하늘 이펙트 보존 계획

## G00. 현재 확인한 원인

`VALTAN_SIX_PIZZA_106/STEP_01`의 `cue.valtan.six-pizza.sky.spacehole`과
`cue.valtan.six-pizza.sky.chaosgate`는 authoring 및 실제 게시 cue에 존재한다.
맵 고정 위치·snapshot·ARENA_ABSOLUTE를 사용하며 warmup은 각각 3000/8000ms,
표시 끝은 STEP_01 기준 20400ms다. 복구 근거는
[09-30 결과 G01](../09-30/2026-09-30_VALTAN_EFFECT_CENTER_AND_SKY_RESULT.md)다.

STEP_04/05에서 `Level_ValtanArena::Update_SourceCinematic`이 원본 `roar`를
재생하며 보스를 cinematic suppressed로 전환한다. `Set_BossCinematicSuppressed`는
기존 finite tail도 숨기며, `roar`의 16개 effect track(고유 resource 15개)에는 하늘 두 자산이 없다.
이 경로에서 보스 몸체 중복 억제가 독립된 하늘 배경까지 숨기는 결함이 확인됐다.
native5363~5372는 engine opacity `source[0].x=1`을 이미 바인딩한다.
Additive program의 원본 alpha0을 오류로 처리하거나 shader를 변경하지 않는다.

## G01. 최소 변경과 소유권

변경 코드는 `Client/Private/Effect_PresentationService.cpp` 하나다.
`Spawn_WorldRoot(CueDesc)`가 모든 anchor 이름을 root로 지우던 경로에서 map만
보존한다. 실제 world transform은 기존 `WorldRoot` 그대로 사용하며 다른 anchor의
정규화는 바꾸지 않는다. public header, JSON schema, Resources, cue 값은 변경하지 않는다.

내부 helper 하나가 다음 조건을 모두 요구한다.

- `strAnchorSlotId == map`, follow policy `SNAPSHOT`, 유효 world-root handle.
- `bPreserveBossActionTail == true`, stop policy `CUE_END`, duration > 0.
- level-owned가 아닌 boss cue. 실제 owner 일치는 각 기존 호출자에서 검증한다.

현재 MAP cue는 위 하늘 두 개뿐이다. 모든 MAP, 모든 snapshot, 모든 boss tail을
예외로 만들지 않는다. 초기 spawn, pending/active cinematic 전환, local preview가
같은 helper를 사용한다. 독립된 하늘은 기존 inspection 표시값을 보존하며 일반 몸체
cue의 제거/숨김 정책은 유지한다. cue end, 명시적 world-root 정지, owner 삭제,
level 종료와 재생 시계는 기존 경로가 계속 소유한다. `Stop_BossAction`은 원래
world-root handle을 제외하므로 그 함수에 새 pattern 취소 정리 동작을 추가하지 않는다.

`roar`에 sky track을 복제하면 STEP_01부터 흐르던 warmup/시계를 다시 설정해야 하고
컷신 전후에 별도 occurrence를 교체해야 한다. 이번 결함은 이미 존재하는 하늘의
숨김 정책이므로 기존 occurrence를 유지하는 방식을 선택한다. 원본의 정확한 활성화
시각은 09-30에도 미확정이었으며 기존 20.4초 저작 구간을 새 원본값으로 주장하지 않는다.

## G02. 검증과 반영 경계

실제 변경 함수 본문을 추출한 native 검사로 pending→active 경계, product/local preview,
map finite tail 보존, root tail 숨김, normal cue 제거, 다른 owner와 inspection 숨김 보존,
cue end/owner 정리 경로를 확인한다. 설치 JSON의 cue·source 두 개 및 WModel·texture
closure를 읽고 warmup, 위치·규모와 stage/camera 시각을 대조한다. 수치 검증과 사용자
화면 판정은 구분한다. 실제 shader/frame을 실행하지 않았다면 GPU 성공으로 기록하지 않는다.

CPP 인코딩을 유지하고 해당 TU 최소 컴파일과 `git diff --check`를 수행한다.
Product 재빌드·merge는 통합 담당이 진행하며 Client/UI를 실행하거나 Reload하지 않는다.
사용자 Data 및 병행 수정 중인 Engine Model/Material/Renderer/PathFinder와 Map batch 파일은
이번 변경 범위가 아니다. 신규 C++ 파일이 없어 프로젝트/filter 등록은 필요 없다.
