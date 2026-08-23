# 차원술사 BA·A occurrence 동기화 구현 결과

## 1. 결론과 현재 상태

차원술사 기본 공격 `2050010`의 BA1/BA2 동시 재생 원인을 playable animation
source window로 분리했고, BA1~BA4의 Server stage, animation, root motion과 Product
Effect를 같은 4단 cadence로 맞췄다. 차원술사 A `2050210`은 기존 aggregate 12개
Element를 A1~A4의 `9/1/1/1` occurrence 문서로 무손실 분리했다.

현재 구현은 `codex/artist-ba-dm-a-stage-sync` 분리 worktree에 있으며 시작 HEAD와
`origin/main`은 모두 `111e06debd17bee48ee26308b3ebc90644280cae`다. 이 RESULT를
작성하는 시점에는 변경이 아직 commit되지 않았다. focused 자동 검증은 아래 범위까지
통과했지만 전체 Engine/Shared/Server/Client Debug·Release 정본 build와 새 통합 PR은 아직
결과를 기록하지 않는다. Client 화면과 조작에 대한 최종 visual 판정도 사용자가 직접 해야 한다.

| 구분 | 현재 판정 |
|---|---|
| 구현 | BA source split, A occurrence split, runtime/tool clock 연결 완료 |
| focused 자동 검증 | PASS, 아래 실행 증거에 한정 |
| 전체 Debug 정본 build | Engine/Shared/Server/Client compile·link PASS; 기존 Effect project 361-file stale gate에서 회귀 스크립트 exit 1 |
| 전체 Release 정본 build | Engine/Shared/Server compile·link PASS; 긴 Client build는 긴급 merge 요청에 따라 완료 전 중단 |
| 사용자 visual fidelity | `PENDING_USER_VISUAL_GATE` |
| 새 통합 PR 번호와 merge | 아직 기록하지 않음, 후속 검증 대상 |

## 2. BA1~BA4 구현 결과

### 2.1 같은 source clip을 두 Server stage로 분리

`ANIMATION_SKILL_CLIP`에 optional `sourceStartMs`를 추가했다. 문자열 clip과 기존
`playMs/playRate` 문서는 source start 0으로 그대로 읽히며, object clip은 strict known-field
검사 뒤 source offset을 보존해 다시 직렬화한다.

차원술사 `2050010`의 binding은 다음 source window를 사용한다.

| BA stage | clip | source window | playRate | wall duration |
|---|---|---:|---:|---:|
| BA1 | `pc_sp_m_00_sk_att_battle_1_01` | `[0, 300)ms` | `1.08695652` | `276ms` |
| BA2 | `pc_sp_m_00_sk_att_battle_1_01` | `[300, 600)ms` | `1.11524164` | `269ms` |
| BA3 | `pc_sp_m_00_sk_att_battle_1_03` | `[0, 1067)ms` | `2.15991903` | `494ms` |
| BA4 | `pc_sp_m_00_sk_att_battle_1_04` | `[0, 1700)ms` | `1.34175217` | `1267ms` |

Character는 binding의 `sourceStartMs`를 `CLIP_STEP`과
`ACTION_PRESENTATION_CLIP_TIMING`에 전달한다. clip 시작과 forward seek는 해당 source
position을 즉시 sample하고, explicit clip의 종료는 `sourceStartMs + playMs`에서 판정한다.
문서 load 때 model duration 밖 source start는 staged chain을 commit하지 않고 거부한다.

explicit trim의 cue 범위는 `[start,end)`다. 따라서 source `300ms` cue는 BA1 끝에는
포함되지 않고 BA2 시작에만 포함된다. `playMs=0`인 기존 natural clip은 model 끝에 정확히
놓인 legacy cue를 계속 허용해 기존 문서를 회귀시키지 않는다.

### 2.2 Server cadence와 root motion

`PlayerSkills.json`의 네 combo stage는 도화가의 빠른 4단 cadence에 맞춘
`276/269/494/1267ms`를 사용한다. hit time은 `92/90/13/250ms`, input open은
`92/179/93/0ms`, input close는 `276/269/494/0ms`다. Server contract test의 기대값과
BA1 tap boundary도 같은 값으로 교체했다.

`DimensionMaster.rootmotion.json`은 같은 네 wall duration으로 다시 slice했다. 각 stage는
time 0에서 시작하고 마지막 sample time이 각각 `276/269/494/1267ms`다. 마지막 forward
값은 focused test에서 `0.1055/0.3031/0.2802/1.0404`로 고정했다.

### 2.3 BA Product Effect

Product cue는 다음처럼 정리했다.

- BA1: clip01 source `0ms` → `effect.dimensionmaster.skill.2050010.ba1.unified`
- BA2: clip01 source `300ms` → `effect.dimensionmaster.skill.2050010.ba2.unified`
- BA3: clip03 source `0ms` → `effect.dimensionmaster.skill.2050010.ba3.unified`
- BA4: clip04 source `0ms` → `effect.dimensionmaster.skill.2050010.ba4.unified`

네 cue는 모두 `root / follow / action_facing / natural`이다. BA1 문서는 source 200ms의
검격 한 Element만 남겼고, BA2는 다음 stage 신호 tail을 제외한 8개 Element를 100ms
local delay에 둔다. BA3/BA4는 각각 자기 Product 문서를 사용한다. 첫 LMB 입력에서 BA2
Effect를 같이 고르는 기존 clip-name-only join은 제거됐다.

## 3. A1~A4 Product occurrence 구현 결과

기존 `effect.dimensionmaster.skill.2050210.unified`는 12개 Element를 보존하는 aggregate
evidence다. Product playback은 이를 직접 cue하지 않고 다음 네 direct-authored 문서를 사용한다.

| occurrence | Product Effect | source cue | Element 수 | 문서 내부 delay |
|---|---|---:|---:|---:|
| A1 | `effect.dimensionmaster.skill.2050210.a1.unified` | `250ms` | 9 | 전부 `0s` |
| A2 | `effect.dimensionmaster.skill.2050210.a2.unified` | `600ms` | 1 | `0s` |
| A3 | `effect.dimensionmaster.skill.2050210.a3.unified` | `900ms` | 1 | `0s` |
| A4 | `effect.dimensionmaster.skill.2050210.a4.unified` | `1300ms` | 1 | `0s` |

네 문서의 ordered Element ID union은 aggregate 12개와 정확히 같고 중복이 없다. 각
문서는 occurrence source time만 animevents에 남기고 내부 start delay를 0으로 rebase했다.
네 cue는 모두 `pc_sp_m_00_sk_sk_willowrend`의 `root / follow / action_facing / natural`을
사용한다. 기존 aggregate Product cue는 제거했으므로 aggregate와 split 문서가 이중 draw되지
않는다.

네 문서는 `Data/Effects/EffectCatalog.json`과 Client project/filter의 `96.DataFiles`에
등록했다. materializer는 기존 12행 aggregate identity와 순서를 먼저 검증하고 A1~A4를
atomic하게 생성한다. 이미 존재하는 split 문서가 예상값과 다르면 덮어쓰지 않고 실패하며,
두 번째 실행은 byte-idempotent다. restoration inventory와 four-class authored rollout도
네 Product target을 현재 정본으로 소비하도록 갱신했다.

## 4. Character와 Effect Tool 공통 clock

All Effects와 saved direct Effect preview는 더 이상 같은 이름의 첫 clip을 고르지 않는다.
Product candidate가 ordered stage index, stage clip index, exact `ANIMATION_SKILL_CLIP`과 cue를
함께 보존하고 cue source time이 그 clip의 half-open window 안에 있을 때만 admission한다.

Product preview의 cue start/end, Effect sample time과 timeline seek는 다음 관계를 사용한다.

```text
cue wall start = (cue source start - clip source start) / playRate
effect sample  = max(0, timeline wall - cue wall start) * playRate
```

source-trim clip은 Engine의 전체 clip loop에 맡기지 않고 Effect Tool sequence가 source
start, trim end, pause/restart를 소유한다. explicit non-loop clip이 끝 pose에서 멈춘 뒤에는
animation clock을 놓아 natural Effect tail이 wall clock으로 끝까지 진행하게 한다.

A1~A4처럼 짧은 occurrence Effect를 전체 A animation 안에서 손튜닝해야 하는 경우도 닫았다.
Product preview duration은 cue와 Effect tail만으로 끝내지 않고, 정확히 staged된 class model의
선택 clip source-window wall duration과 비교해 더 긴 값을 사용한다. model generation, exact
clip identity, source window, 중복 clip name 또는 model duration 검증이 실패하면 duration을
추정하지 않는다. 따라서 네 A 문서 각각을 열어도 약 `2.767s` Willowrend animation 전체를
보면서 occurrence를 조정할 수 있다.

이 clock 계약은 새 `Tools/ActionPresentationTimelineHarness`의 C++ source와
`Default/*.vcxproj/.filters`로 독립 실행 가능하게 만들고 `Framework.sln`에 등록했다.

## 5. 자동 검증 결과

아래 항목만 현재 PASS로 기록한다.

| 검증 | 관찰 결과 |
|---|---|
| `ActionPresentationTimelineHarness` Debug | PASS |
| `ActionPresentationTimelineHarness` Release | PASS |
| explicit `[0,300)/[300,600)` 경계와 natural-end 호환 | PASS |
| Product source/playRate clock과 2.767s animation duration floor | PASS |
| `test_dimensionmaster_2050010_stage_split.py` | 2 tests PASS |
| `test_materialize_dimensionmaster_2050210_occurrences.py` | 7 tests PASS |
| `test_build_character_effect_restoration_inventory.py` | 23 tests PASS |
| `test_publish_four_class_authored_rollout.py` | 14 tests PASS |
| Gameplay balance publisher `-Mode Validate` | PASS |
| Effect publisher `-Mode Validate` | 161 entries PASS |
| 변경·추가 JSON 21개 parse | PASS |
| 변경·추가 XML 4개 parse | PASS |
| `git diff --check` | PASS |

Effect publisher는 분리 worktree에 Git 비관리 `Client/Bin/Resources`가 없으므로 기본
`ResourceRoot`로 실행하면 resource lookup에서 실패한다. 정본 폴더
`C:/Users/user/Desktop/LostArk/Client/Bin/Resources`를 명시한 Validate가 161 entries PASS한
결과를 위 표에 기록했다. 이것은 구현이 resource fallback을 추가했다는 뜻이 아니다.

## 6. 기존 frozen/stale 실패와 이번 판정에서 제외한 항목

다음 실패를 이번 변경의 성공으로 바꾸거나 숨기지 않았다.

- `test_build_four_class_combat_source_intake.py`는 Warlord stage-count 불일치와
  LanceMaster source artifact hash 불일치를 계속 보고한다. 두 실패는 이번 diff가 수정하지
  않은 파일과 `origin/main`의 기존 frozen 상태에서도 재현되므로 BA/A PASS 분모에 넣지 않았다.
- `Sync-EffectDataProject.ps1 -Check`는 현재 `Data/Effects` Git 관리 파일 2,301개 중 project와
  filters에 각각 361개가 누락돼 실패한다. 분류는 Contracts 9, CookedShaders 165,
  TranslatedShaders 169, V2 18이다. 새 A1~A4 네 문서는 project와 filters에 각각 정확히 한 번
  등록돼 있고 추가·중복 등록은 없다. 이 광역 stale index를 이번 수직 슬라이스에서 일괄
  재배치하지 않았다.
- 분리 worktree status에 진행 중인 광역 regression이 만든 WorldDestruction runtime 출력이
  보일 수 있으나 BA/A 구현 범위나 완료 증거로 사용하지 않는다. 최종 commit은 대응 PLAN에
  적힌 수직 슬라이스 파일만 exact stage해야 한다.

## 7. 기존 PR #173, #174 처리 판단

두 PR은 현재 GitHub에서 모두 `CLOSED`, `mergedAt=null`이며 이번 branch에 merge 또는
cherry-pick하지 않았다.

| PR | 판정 | 근거 |
|---|---|---|
| #173 `Effect V1: add four-character horizontal application ledgers` | 폐기 유지 | 16만 행 규모 horizontal ledger·packet 조사 PR로 현재 main과 merge state가 `DIRTY`이며 BA/A source-window, Server timing/root motion, stage별 Product split을 구현하지 않는다. 오래된 Effect catalog/authoring 계약의 생성물이 현재 `Client.vcxproj(.filters)`, Effect contracts/tooling과 넓게 겹쳐 merge/cherry-pick하면 stale 산출물을 되살린다. 필요하면 최신 main에서 다시 생성한다. |
| #174 `docs: inventory effect shaders and data folders` | 폐기 유지 | 당시 shader/Data 폴더를 기록한 documentation-only snapshot이고 BA/A runtime/data consumer가 아니다. catalog/profile/translated-shader 수치와 ownership 설명이 현재 Effect Tool V2와 161-entry catalog보다 오래됐고 `.md/TEAM/README.md` 정본과도 겹치므로 cherry-pick하지 않는다. 필요하면 최신 main 실측으로 새 inventory를 작성한다. |

#173은 commit `b8ef813a10cb51359d1857c3046a1e16f0e4c270`, #174는
`25dfc9c1be89aab178cf8faae811a254eaea0396`에서 닫혔다. 두 remote head branch는 복구 가능한
상태로 남아 있으며 destructive branch 삭제는 수행하지 않았다. 즉 폐기는 기존 PR을 main에
반영하지 않는 결정이다.

## 8. 수동 visual 검증과 후속 기록 대상

에이전트는 Client/UI를 실행하거나 화면을 대신 판정하지 않았다. 전체 build와 최종 publish가
끝난 뒤 사용자가 다음을 직접 확인해야 한다.

1. Character Select에서 DimensionMaster를 선택하고 LMB를 한 번 눌렀을 때 BA1 검격만
   보이는지 확인한다. 다음 입력에서 BA2, 이후 BA3/BA4가 빠른 4단 cadence로 이어져야 한다.
2. 이동·회전 중 네 BA Effect가 캐릭터 `root`를 follow하고 action facing을 유지하는지 확인한다.
3. `F1 > Effect Tool > All Effects > DimensionMaster > 2050010`에서 BA1과 BA2가 같은
   clip01을 쓰더라도 각 Product Play가 자기 source window와 자기 Effect만 재생하는지 확인한다.
4. `2050210`의 A1~A4 Product 문서를 각각 열어 occurrence가 하나씩 선택되고, 짧은 Effect가
   끝난 뒤에도 Willowrend animation 전체 약 2.767초를 scrub/loop하며 손튜닝할 수 있는지 확인한다.
5. A1~A4를 실제 Server 승인 A action으로 재생했을 때 검격 네 번의 위치, 방향, 색, coverage,
   lifetime과 잔상을 최종 판정한다.

추가로 실행한 광역 build 결과는 다음처럼 제한해 기록한다.

- Debug는 Engine → UpdateLib → Shared/NetworkProtocolHarness → Server → Client가 모두
  compile·link됐다. 이어진 Balance, Navigation, Effect 161-entry Validate도 통과했지만,
  이후 기존 `Sync-EffectDataProject.ps1 -Check`의 361-file stale 상태에서 표준 회귀
  스크립트가 exit 1로 끝났다.
- Release는 Engine → UpdateLib → Shared/NetworkProtocolHarness → Server까지 compile·link됐고
  Client를 빌드하던 중 사용자의 긴급 merge 요청에 따라 중단했다. 따라서 전체 Release PASS로
  기록하지 않는다.

아래 항목은 이 문서 작성 시점에 아직 완료로 기록하지 않는다.

- Debug/Release 표준 회귀의 기존 361-file stale gate 이후 단계
- Server+Client 실제 실행과 DimensionMaster BA/A 사용자 visual 승인
- 새 통합 PR 생성, 번호 기록, review/merge
- merge 뒤 정본 `C:/Users/user/Desktop/LostArk`의 `main` pull 및 실행 산출물 동기화

이 후속 증거가 실제로 끝난 뒤에만 해당 결과를 이 RESULT에 추가한다.
