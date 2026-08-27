# 발악 빈 Effect·사자후와 3시·9시 점프 0초 기준 수정 결과

작업일: 2026-08-28

정본: `C:/Users/user/Desktop/LostArk/Framework.sln`

브랜치: `codex/valtan-arena-next-desktop`, 기준 commit `a6871a2f`.
이 결과는 최초 두 수정과 이후 승인된 9시 동일 수정만 다룬다. 다른 작업의 전체 통합 검증을 완료했다고
기록하지 않으며, 큰 미커밋 변경 집합을 자동 stage/commit하지 않았다.

## 1. 구현 완료

### 발악: 빈 Effect 하나

- `effect.valtan.sequence.warp-jump-four-hand-twohand-roar-roar-dead`의 기존 39개 Element를
  사용자 요청대로 비웠다. `elements=[]`, `modelCues=[]`인 유효한 v13 문서다.
- `ValtanPatternAuthoringEffects.json`에 `VALTAN_STRUGGLING`의 `DRAFT_ATTACHED` 한 건을
  추가했다. 기존 RUSH Draft는 보존했다. 삭제된 runtime Product cue는 복원하지 않았다.
- All Effects의 해당 패턴은 `Effects 0 | Draft 1`로 분류되며 Draft의 `Open Editor`에서
  Element를 추가하는 경로를 사용한다. 빈 문서는 편집 대상이지 재생 가능한 Product가 아니다.
- 생성기의 Draft 정리는 같은 패턴에 같은 Effect의 Product cue가 실제로 있을 때만 실행한다.
  과거 promotion 기록이나 catalog 항목만으로 사용자가 만든 빈 Draft를 다시 지우지 않는다.

### 사자후·3시 점프: Start Delay 0의 기준 수정

원인은 Effect가 전체 패턴 첫 clip이 아니라 뒤쪽 clip의 cue에 연결되어 있었기 때문이다.
사자후의 Effect 0은 패턴 2.3초였고, 화면의 2.612초는 Effect 로컬 0.312초였다.
Start Delay 값을 변경하거나 표기만 바꾸지 않고, 요청한 두 cue의 실제 원점을 옮겼다.

| 패턴 | 이전 cue 소유 | 최종 cue 소유 | 최종 sourceStartMs |
|---|---|---|---|
| `VALTAN_ROAR_CHARGE` | `STEP_03` | `STEP_01`, 첫 clip | 0 |
| `VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK` | `IMPACT` | `TAKEOFF`, 첫 clip | 0 |

`Data/Valtan/Valtan.presentation.json`이 정본이며, canonical `project-products`의 출력 중
변경된 `Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json`만 검토 후 반영했다.
cue ID와 Effect ID는 유지했다. 두 Effect의 Element 값은 바꾸지 않았고, native emitter 지연도
추가로 빼지 않았다. 최초 수정에서는 9시 IMPACT cue를 유지했다. 이후 승인된 9시 수정은
아래 5절에 기록하며, native 지연 `0.230894초`는 그 수정에서도 유지한다.

Timeline의 공용 역변환 `Resolve_CuePreviewTimelineTime`을 Tool이 사용한다. 유효하지 않은 시간,
배속과 overflow를 거부하고 source/wall time의 왕복 변환을 native 테스트로 검증했다.
모든 패턴의 Start Delay UI를 전역 시간 입력으로 바꾸는 변경은 포함하지 않는다.

## 2. 최종 변경 파일

- `Client/Private/Effect_Tool.cpp`: 기존 시간 역변환을 공용 Timeline 함수로 연결.
- `Client/Public/ActionPresentationTimeline.h`, `Client/Private/ActionPresentationTimeline.cpp`:
  source/wall time 역변환 함수.
- `Data/Effects/Authored/effect.valtan.sequence.warp-jump-four-hand-twohand-roar-roar-dead.effect.json`:
  빈 문서.
- `Data/Effects/ValtanPatternAuthoringEffects.json`: 발악 Draft 한 건.
- `Data/Valtan/Valtan.presentation.json`, `Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json`:
  두 cue의 시작 clip과 시간.
- `Tools/ValtanPipeline/author_valtan_phase_two_mechanics.py`: 두 패턴의 생성 기본값.
- `Tools/ValtanPipeline/author_valtan_requested_effect_elements.py`: unlink 후 빈 Draft 보존.
- 기존 Action/Effect native harness와 EffectPipeline 테스트 4개: 시간 변환, 생성·저장·재로드,
  사용자 삭제 보존과 잘못된 문서의 거부 검증.
- 대응 구현 PLAN의 최상단에 최종 요청을 기록했다. `Effect_Tool.h`에는 이번 최종 요청으로
  추가한 메서드가 없고, 이전 통합 변경을 그대로 유지했다.

## 3. 자동 검증

로그 루트: `_work/effect-timing-desktop-check/`.

| 검증 | 실제 결과 | 로그 |
|---|---|---|
| Desktop `Framework.sln /t:Client`, Debug x64, `BuildProjectReferences=false` | exit 0, `Client/Bin/Debug/Client.exe` 링크 완료 | `framework-client-debug-final.log` |
| ActionPresentationTimelineHarness Debug | exit 0 PASS | `action-timeline-debug-final.log` |
| ActionPresentationTimelineHarness Release | exit 0 PASS | `action-timeline-release-final.log` |
| EffectRenderContractHarness Debug | 새 빌드·실행 exit 0 | `effect-render-debug-build.log`, `effect-render-debug-final.log` |
| 관련 Python 테스트 4개 | 총 104건, 성공 97건, 기존 skip 7건 | `focused-final.log` |
| Valtan pipeline validate | exit 0, managed 29 / legacy 26, projection 일치 | `valtan-validate-final.log` |
| Effect source validate | exit 0, direct 197 / reference 269 | `effect-sources-validate-final.log` |

전체 `git diff --check`에서는 별도 작업 문서 두 곳의 기존 공백 오류가 나왔다.
`2026-08-28_VALTAN_BOSS_TOOL_RESETLESS_NEXT_PATTERN_IMPLEMENTATION_PLAN.md`의 EOF 빈 줄과
`2026-08-28_VALTAN_SAVED_FLOW_JSON_REVIEW_RESULT.md`의 trailing whitespace는 범위 밖이라
변경하지 않았다. 이번 수정 파일에 한정한 diff 검사와 JSON/XML parse 결과는 최종 receipt에 기록한다.
이번 실행에서 전체 Engine/Server/Client Release regression을 다시 수행한 것은 아니다.

## 4. 보존·수동 확인 경계

기존 발악 39개 문서와 변경 전 cue/sidecar의 byte 백업은
`_work/effect-timing-desktop-check/empty-draft-pattern-zero-before/`에 남겼다.
변경 내용과 SHA는 같은 루트의 `empty-draft-pattern-zero-receipt.json`에 기록했다.
1페이즈 7·2개 / 2페이즈 21개 Element, 사용자 반원 DDS, 3시의 현재 세 Element와 삭제 내역,
다른 작업의 Six Pizza camera 변경은 되돌리지 않았다.

Client/UI와 Server를 실행하지 않았고 화면 PASS를 선언하지 않는다.
사용자가 정본 Debug Client를 실행한 뒤 `All Effects → Refresh → 3페이즈 전 발악패턴 →
Draft → Open Editor`에서 빈 Effect에 Element를 추가하면 된다. 사자후·3시 점프는 해당 Product를
다시 열어 `Start Delay 0`과 전체 Timeline 0의 일치를 확인한다. 원본 emitter가 가진 별도 생성
지연은 유지되므로 해당 native particle의 첫 픽셀 시점까지 모두 강제로 0으로 만든 것은 아니다.

## 5. 추가 승인: 9시 점프도 패턴 0초에 연결

`VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK`의
`cue.valtan.requested.20260827.terrain-9.semicircle`을 `IMPACT/sourceStartMs=200`에서
`TAKEOFF/sourceStartMs=0`으로 옮겼다. 정본 presentation과 canonical `project-products`가
생성한 runtime cue에 반영했다. 7개 생성물 중 실제 달라진 것은 pattern effect cue 파일 하나이며,
그 안에서도 9시 cue의 stage/action/clipOccurrence/sourceStart 네 필드만 변경했다.
생성기 기본값과 기존 회귀 테스트의 9시 기대값도 같은 기준으로 맞췄다.

사용자가 저장한 9시 Effect 두 Element는 파일 전체 bytes를 보존했다. 반원의 회전
`[-90, -180, 0]`, Start Delay 약 1.3초, 착지의 Start Delay 3.5초를 다시 쓰지 않았다.
변경 전후 SHA256은 `8327e4764a14ea074ab0e6e5a53506a15dea77f0a7b18ce622de276957e85aba`다.
사용자가 추가한 TWOHAND/WHIRLWIND Draft도 보존하고, 추가 Draft를 잘못 거부하던 이전
테스트의 고정 개수 기대만 수정했다. 문서의 ID·경로·중복 검증은 그대로 유지한다.

처음에는 실행 중인 Client가 presentation 파일을 열고 있어 원자적 교체가 거부됐다.
Restart Manager로 Client 소유를 확인했으며 임의 종료·파일 잠금 해제는 하지 않았다.
사용자가 Client를 종료한 뒤 현재 파일과 백업을 대조해 적용했다. C++/exe는 변경하지 않아
이번 데이터 수정에 새 빌드는 필요 없다. Client/UI는 실행하지 않았다.

이번 검증·백업·receipt는 `_work/effect-timing-desktop-check/terrain-nine-zero/`에 둔다.
ActionPresentationTimelineHarness Debug/Release, Effect source validator와 Valtan validator가
각각 exit 0이다. Python은 총 104건 중 97건 성공·기존 skip 7건이며 수정 파일의
`git diff --check`도 통과했다. 상세 결과는 이 폴더의 receipt와 로그로 남겼다.
사용자는 Client 재실행 후 `All Effects → Refresh → 점프후지형파괴9시 → Open Editor`로
다시 열어 확인한다. 최종 육안 결과를 대신 PASS로 기록하지 않는다.
