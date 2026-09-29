# 쿠크 기믹 실패·제한시간·접촉 복구 결과

## G00. 범위와 적용 상태

2026-09-30 현재 저장본 기준 사용자 승인으로 authoring 필드를 반영했다. 기존 dirty 변경과 rendering 옵션은 보존했다. 이 문서는 쿠크 서버 기믹과 관련 저작값을 기록하며, 전체 배포와 Debug/Release 통합 검증은 같은 날짜의 통합 RESULT가 소유한다. Client 실행과 화면·청감 확인은 수행하지 않았다.

## G01. Mario 입장·실패·90초 제한

`ServerPlayer::iKoukuMinigameEndTick`은 절대 서버 tick이며 0은 비활성이다. `Update_KoukuPlayerModes`는 Mario 또는 카드미로에 들어간 인간에게 30 Hz 기준 2,700 tick을 부여한다. Mario 제한 초과는 해당 플레이어, 카드미로는 인간 전원을 즉사 처리하며 전투 사망 경로가 shield·무적과 관계없이 판정을 반영한다. 탈출·사망 후 deadline을 지워 다음 입장에 재사용하지 않는다. snapshot 할당을 연결했으며 Shared codec와 Release HUD 소비자는 HUD 담당 변경이다.

Mario Parent에서 입장 창의 끝을 무시하던 `honorWindowEnd` 분기를 제거했다. 저장된 입장 start/duration이 자식 패턴 수명과 독립적으로 전멸을 판정한다. timeout의 anchor 정리는 중복 판정을 막으며 phase2 예약을 막지 않는다. 실패, 솔로 플레이어 사망 및 즉시 부활 경로도 phase2를 유지한다.

포탈 검사를 `Prepare_KoukuAuditionTick`에 연결하여 자식 사이 대기에도 검사하고, `Update_WorldEntities`의 entity 순회 전에 pending 입장을 확정한다. 기존에는 자식 피해 판정 후 검사하고 모든 entity 갱신 뒤에 확정하여 같은 tick의 부착·상태 변경이 입장을 거절할 수 있었다. 실제 Release 간헐 재현의 유일 원인으로 단정하지 않으며, 남는 admission 거절은 제한된 `[MarioEntryDeferred]` 로그에 player/action/attachment/bind와 상태 사유가 남는다.

현재 Mario2 정본은 `KAKULSAYDON_G1_PATTERN_91`이며 포탈은 MAP `(1.344, 1.318, 943.005)`, 창은 8,083–38,563 ms, Parent 끝은 43,748 ms다. 사용자가 조절한 start/duration을 덮어쓰지 않았다.

## G02. 갈고리·폭발·망치·빙고

| 변경 | 실제 연결 |
| --- | --- |
| 다섯 갈고리 묶음 | P33.world.2 → world.37 → `sequence.kouku.hook.original_preview`. 15개 emission, 5개씩 3묶음 모두 동일 template 소비. 시작 위치를 유지하고 이동 종료 local X를 1 m 앞당김 |
| giant 등장 공 | P8.world.1의 10개 공을 기존 seed의 명시 emission/trajectory로 저장한 전용 template에 연결. 각 공 motion end에 기존 ENTER_AREA → 10% 피해, 10 m/1,500 ms 강제 ballistic knockback 연결 |
| 조커 실패 마지막 타격 | P13.logic.29 / presentation.53, 기존 29,085 ms 유지. P86 2연속 내려치기 마지막 collider/result와 동일 |
| 피자 마지막 타격 | P26.logic.1 / presentation.7, 기존 16,997 ms 유지. P86 마지막 타격과 동일 |
| 빙고 머리 표식 | `sequence.kouku.bingo.bomb.marked` key scale 0.12 → 0.18, 높이 보존 |
| 빙고 폭발음 | planted template의 4,000 ms 폭발에 쇼타임 `G_Satan1_Attack06_ProjExp1.variant01.wav`, 2,067 ms, volume 1 연결 |

망치 기준은 WEAPON `b_rpct_01`, offset `(0,0,0.6)`, scale `(1.4,1,1)`, CYLINDER resource `.283`, 100 ms다. 동일 결과 `.522`가 피해와 넉백을 소유한다. 원본 모델과 Collider를 별개로 새로 구현하지 않았다.

갈고리는 설치된 `MN_UMAX_00.wmodel`과 실제 clip/b_hook_01을 sampler로 대조했다. 9,834 ms의 15개 grip 모두 기존값에서 이동 방향 반대로 정확히 1 m 이동했다. 무대 방향 묶음의 최외곽 예는 `(-0.173,2.279,955.060)` → `(-0.346,2.279,954.075)`다. 기존 runtime의 실제 nav floor projection과 상승 전 하차 계약은 유지한다. 전체 표는 `out/KoukuMechanics20260930/hook-release-samples.json`에 있다.

Composition은 최초 적용 revision 2493→2494, root의 독립 damage-reduction 변경 후 등장 공 속도 정규화까지 2496이다. WorldSequences는 2285→2288이다. 같은 stable record 내 관련 필드만 바꿨으며 교체 직전 byte와 ReplaceFileW가 백업한 이전 파일이 동일함을 검증했다. 영수증과 이전 파일은 `out/KoukuMechanics20260930/`에 있다.

## G03. 강제 사망 대상 누락 보완

공통 root 변경은 instant/maxHP100% 결과를 방어·보호막·무적과 분리한다. 추가로 roulette/gaze/pose의 대상 열거가 `Is_Judgeable` 때문에 살아 있는 not-ready/falling/grab 인원을 누락하던 경로를 수정했다. 실제 선택된 fail/timeout 결과가 lethal일 때만 인간 생존자를 포함하며, 카드 위치·시선·입력의 성공 판정은 보존한다. ENTER_AREA의 바깥 timeout도 같은 처리를 사용한다.

빙고 블랙홀은 일반 `iInvulnerableEndTick`으로 실패 판정을 회피하던 경로를 분리했다. 실제 줄 완성 보상의 `iKoukuBingoLineProtectionEndTick`과 이난나 zone에서 발급된 `iEstherZoneProtectionEndTick`만 성공 조건으로 소비한다. 줄 완성 30초 및 zone 마지막 접촉 후 2 tick을 유지하며, source가 만료되면 다른 일반 무적·보호막의 수명과 관계없이 사망한다. 새 판과 사망·재입장에는 source를 지운다. 두 값은 서버 내부 판정용이며 snapshot 필드는 아니다.

## G04. 검증 증거와 남은 확인

- JSON parse를 완료했다. 최초 `--check`는 projected Product stale을 보고했지만, 최종 inventory 대조에서 P8이 emission delay / combined playback speed의 극소 소수점 오차로 제외된 것을 발견했다. 전용 clone과 cue의 실효속도를 1×1로 정규화했다. read-only 전체 prepare_publication으로 Product 114개를 생성하고 P8/P13/P26/P33/P88/P91/P92/P93의 Product 포함과 empty unavailableReason을 모두 확인했다. 영수증은 out/KoukuMechanics20260930/required-product-validation.json이다. 설치 publish는 root가 수행한다.
- 실제 WModel/clip 기반 hook grip 15개 이동거리 1 m 검증을 완료했다. nav의 실제 release regression은 `--kouku-object-overlap-contract-test`에 포함된다.
- focused 회귀를 추가했다: Mario/card-maze 90초 경계·보호막·무적·비전투 상태, authored ball/doll contact 100 ms당 2% 및 HP완전흡수 시 광기, P88/P91 접촉 첫 tick 입장, 4개 Mario timeout 뒤 P33 재생, 죽은/즉시부활 솔로의 phase2 유지, lethal verdict 4종×상태3종 성공·실패 분리, Bingo 실제 줄·이난나 출처 보호와 일반 무적 분리.
- Debug 통합 컴파일 중 테스트 블록 교체의 남은 구문 오류를 수정했다. 초기 Debug product 실행에서 신규 제한시간 8개 및 authored 공/인형 광기 6개 검증은 PASS를 확인했다. 기존 optional Mario fixture의 entry 창을 늘린 뒤 pattern lifetime을 함께 늘리지 않았던 admission 실패와, HP 감소로 stagger를 유도하던 마지막 tick fixture를 독립 무력화 수치에 맞게 수정했다. 최종 전체 컴파일·test 통과 여부는 root 통합 로그가 확인한다.
- `git diff --check`는 담당 C++/JSON에서 오류 없이 완료했다.
- Client/UI를 실행하지 않았으므로 실제 Release 간헐 입장·시각·청감의 최종 확인은 사용자 검증 대상이다.

## G05. Resources 전달

재사용 폭발 WAV가 GBResources에 없어서 `C:/Users/user/Desktop/GBResources/Sound/KoukuSaton/Events/G_Satan1_Attack06_ProjExp1.variant01.wav`로 복사했다. 원본과 대상 SHA-256은 `DEE470531167A8EEB3B58E96D1D4502E57586ACF0B0877BD092F0205E7A41D67`, 크기는 729,164 bytes다. 영수증은 `out/KoukuMechanics20260930/GBResources-sound-receipt.json`이다. 새 모델·텍스처·이펙트 resource는 추가하지 않았다.

## G06. 최종 통합 검증

최종 publish는 source2496/Product114/WorldSequences2288로 완료됐다. Debug Product350, Bingo152, ObjectOverlap1099 및 Release Raid1750 assertion이 모두 PASS/실패0다. ObjectOverlap은 현재 설치된 66개 carrier의 하차·floor·이동 복귀를 확인했다. Debug 광역 Raid는 중단한 partial log이며 성공 집계에서 제외했다.

최종 빌드 영수증·ZIP·한계는 [통합 RESULT](2026-09-30_RAID_GAMEPLAY_REPAIR_RESULT.md)를 따른다.
