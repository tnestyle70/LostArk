# Valtan 작업 상태와 통합 세션 인계

갱신일: 2026-08-28

> 아래는 PR250 병합 전 인계 기록이다. 이후 사용자가 Next/Trash와 파괴 벽 이동 수정을 명시적으로 재개했다.
> 초기 구현 worktree는 `C:/w/valtan-arena-next`였다. 최종 적용·빌드는 PR #251 `a6871a2f` 기반
> `codex/valtan-arena-next-desktop`의 `C:/Users/user/Desktop/LostArk/Framework.sln`을 기준으로 한다.
> 구현 범위는 갱신된 Next/Trash PLAN, 완료·검증 상태는 대응 RESULT를 따른다. 아래의 보류 표기를 현재 지시로
> 재사용하지 않는다. 저장 Flow를 임의 수정하지 않는 경계는 계속 유지한다.

## 1. 최신 사용자 요청을 우선한다

현재 완료할 단위는 **저장 Flow를 초기 28개 순서에 고정한 테스트 수정**이다.
Trash 통합·전멸 작업은 사용자가 명시적으로 보류했다. Next/Trash C++나 gameplay JSON은 구현하지 않았다.
이 문서를 신규 runtime 구현 허가 또는 구현 완료 기록으로 사용하지 않는다.

## 2. 통합에 포함할 완료 변경

1. [Flow 테스트](C:/Users/user/Desktop/LostArk/Tools/ValtanPipeline/test_valtan_boss_tool_pattern_flow_contract.py):
   실제 저장본 validation / 별도 seed / 독립 mutation fixture / 편집·경계 회귀.
2. [기존 Flow PLAN 추가 절](C:/Users/user/Desktop/LostArk/.md/GB/08-27/2026-08-27_BOSS_TOOL_PATTERN_FLOW_AUDITION_IMPLEMENTATION_PLAN.md):
   editable 저장본을 seed와 분리하는 검증 계약.
3. [기존 Flow RESULT 추가 안내](C:/Users/user/Desktop/LostArk/.md/GB/08-27/2026-08-27_BOSS_TOOL_PATTERN_FLOW_AUDITION_RESULT.md).
4. [저장 JSON 검토 및 수정 결과](C:/Users/user/Desktop/LostArk/.md/GB/08-28/2026-08-28_VALTAN_SAVED_FLOW_JSON_REVIEW_RESULT.md):
   실제 29슬롯 순서, Reload 의미, 수정 전/후 테스트, 원본 JSON 미변경 증거.

검증은 원본 checkout 기준 Flow 22 + 관련 51 = 73 tests PASS, V2 Validate PASS다.
7가지 메모리 저장본을 각각 전체 Flow suite에 주입한 검증도 모두 PASS다.
UI/Save 재실행이나 새 C++ 빌드는 하지 않았다.

## 3. 통합 세션과의 합의

- 전달 받은 통합 위치: `C:/w/p247`, `codex/valtan-party-integrated-review`.
- 통합 대상: PR247/249 + 통합 담당자가 확보한 원본 diff. 이 작업은 별도 PR/merge를 만들지 않는다.
- 통합 protocol은 41이다. 원본 checkout의 테스트에 남은 protocol 39 assertion으로 덮어쓰지 않는다.
- 테스트 파일의 helper/fixture/메서드만 합치고 카메라·97-wall·기타 승인된 통합 변경을 보존한다.
- 사용자 Flow 29슬롯은 검토만 했으며 source bytes는 수정하지 않았다.
- 통합 담당자가 최종 파일을 반영한 뒤 통합본 focused/regression을 다시 실행한다.
- 실제 PR merge와 다른 세션의 pull 완료는 이 문서에서 선언하지 않는다.

원본 checkout의 dirty 변경은 기존 작업자 소유다. 현재 원격 main이 5커밋 앞서며 일부 incoming 파일과
겹치므로 원본 폴더에서 자동 pull/stash/reset 또는 광범위 stage를 하지 않는다.

## 4. 보류 중인 다음 기능

[Next Pattern 구현 초안](C:/Users/user/Desktop/LostArk/.md/GB/08-28/2026-08-28_VALTAN_BOSS_TOOL_RESETLESS_NEXT_PATTERN_IMPLEMENTATION_PLAN.md)은
코드 조사 기반 설계 문서다. 이번 통합의 runtime 변경 목록이 아니다. protocol 41 및 최종 통합 코드를 기준으로
재확인하고 독립 검토를 마친 뒤 구현해야 한다.

핵심 목표는 Ordered Slots와 무관한 1칸 예약, 오른쪽 아래 카드, A 정상 commit 뒤 현재 상태 그대로 B 시작,
기존 NONE + STAGE_CLOCK 도넛 경로 재사용이다. 중앙 이동/맵 복구를 Next 연결 절차에 넣지 않는다.

Trash의 최신 방향은 네 개를 순서대로 실행하는 것이 아니라 `VALTAN_TRASH` 하나의 내부 조건 분기로
성공/실패/재조준 역할을 통합하는 것이다. 그러나 현재 보류 상태이므로 이전 Trash 계획서의 안전 해제,
보조 슬롯 삭제 지시를 실행하면 안 된다. 재개 시 전원 포획→실제 impact 전멸 요구를 반영하여 계획을 다시 확정한다.
현재 사용자 Flow를 자동 삭제/정렬/재발급하지 않는다는 경계는 계속 유지한다.

## 5. 보존할 사용자 상태

```text
Flow: flow.valtan.boss-tool.default
slots: 29 / unique slotId: 29 / unique patternId: 28
nextSlotOrdinal: 30
첫 슬롯: .000029 / VALTAN_FLOOR_WIPE_130
14번째: .000026 / VALTAN_COUNTER, VALTAN_SEQUENCE_FOUR 바로 다음
원래 FLOOR_WIPE_130: 6번째에 유지
```

현재 JSON의 검토 당시 SHA-256은
`635fdbbcafe3b4b9abc455f206575091e79f6b0a6aa56f8993f656397842afcb`다.
이후 사용자가 다시 Save하면 그 최신 저장본이 우선이며 이 배열로 강제 복원하지 않는다.
