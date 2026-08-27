# 발탄 1페이즈·2페이즈 4연속 PRODUCT 분리 복구

## 반영 위치와 원인

실제 적용 위치는 `C:/Users/user/Desktop/LostArk`의
`codex/valtan-arena-next-desktop` 브랜치다. 다른 worktree에만 적용한 결과가 아니다.

`VALTAN_FOUR_SLASH/SLASHES`와 `VALTAN_SEQUENCE_FOUR/STEP_01`이 같은
`effect.valtan.carrier-v1.attack.four-slash.active.clip-01`을 참조했다.
이름 충돌이 아니라 저장 파일 공유로 인해 2페이즈 편집 내용이 1페이즈에도 적용됐다.

## 복구 내용

| 패턴·stage | PRODUCT | 보존한 Element |
|---|---|---:|
| 1페이즈 `VALTAN_FOUR_SLASH/SLASHES`, `mesh_att_battle_10_01` | `effect.valtan.carrier-v1.attack.four-slash.active.clip-01` | 마지막 정상 저장본 7개 복구 |
| 1페이즈 `VALTAN_FOUR_SLASH/SPIN`, `mesh_att_battle_10_02` | `effect.valtan.carrier-v1.attack.four-slash.active.clip-02` | 기존 2개, 파일 변경 없음 |
| 2페이즈 `VALTAN_SEQUENCE_FOUR/STEP_01`, `mesh_att_battle_19_01` | `effect.valtan.project-tuned.sequence.four` | 현재 사용자 저장 21개 전부 보존 |

1페이즈 원본은 `f82489a9` 및 기존 보존 백업의 동일한 7행이다. 위치·회전·크기 튜닝을
포함한 파일 전체를 바이트 그대로 복구했다. 2페이즈 새 문서는 최상위 Effect ID와 표시 이름만
변경했으며 Element ID·순서·시간·색·리소스·모듈·particleSystem·modelCues는 모두 같다.

EffectCatalog에 새 문서를 등록하고 Client project/filter에 `None`으로 추가했다.
phase-two 생성기의 최초 기본 매핑도 독립 ID로 바꿨다. 기존 저장 cue를 보존하는 generator
정책은 유지한다. C++ 코드와 Server gameplay·애니메이션 clip·타이밍은 변경하지 않았다.

정본 `valtan_tuning_pipeline.py project-products`로 생성한 Product 중 effect cue 문서만
갱신했다. 변경은 P2 cue의 `effectAssetId` 한 값이며 나머지 6개 projection은 바이트 불변이다.

## 확인한 결과

- split master Validate: exit 0, managed 29 / legacy 26.
- Effect source validation: exit 0, direct 197 / reference 269 / resources 1029.
- P1/P2 PRODUCT ID가 서로 겹치지 않음, P1 두 cue와 P2의 기존 cue ID·시간 보존.
- 2페이즈 21개 Element 전체 값이 복구 전 사용자 저장본과 일치.
- 변경 JSON/XML parse 및 scoped `git diff --check` 통과.
- 이전 반원 DDS가 Desktop Git index에 빠져 있던 부분도 해당 승인 파일 하나만 등록했다.

증거와 변경 전 백업은
`C:/Users/user/Desktop/LostArk/_work/valtan-four-product-recovery/20260828-070956/receipt.json` 및
같은 폴더의 `before/`, `after/`, `projected/`에 있다.

## 사용자가 다시 여는 경로

`F1 → Effect Tool → All Effects → Refresh → Core Server Patterns → 4연속 공격 →
첫 PRODUCT → Open Editor`에서 복구된 7개를 확인한다. 두 번째 PRODUCT는 기존 2개다.
2페이즈 4방향 공격은 별도 `effect.valtan.project-tuned.sequence.four` 21개를 연다.
복구 전 내용이 남은 편집창을 다시 저장하지 말고 PRODUCT를 새로 연다.

Client/UI를 에이전트가 실행·조작하거나 캡처하지 않았다. 화면의 최종 모양과 재생 확인은
사용자 소유이며 visual PASS로 기록하지 않는다. 이 데이터 복구를 위해 Client 재컴파일은 필요 없다.
