# 발탄 점프·중앙 착지·돌진 충돌·전투 가속 구현 결과

## 0. 결론

요청한 구현과 변경 범위의 자동 검증은 완료했다. 최종 코드 리뷰에서 P0/P1은 발견되지 않았다.
Client는 에이전트가 실행하지 않았으므로 visual PASS는 아니며, 아래 육안 항목은 사용자가 확인해야 한다.

- 피자와 3시/9시 지형 파괴 점프는 800ms까지 지상 예고 뒤 300ms 안에 정점으로 상승한다.
- 3시/9시 파괴 이벤트는 각각 기존 floor84/floor30을 유지하고 발탄 본체만 중앙
  `[156.03, 22.99751, -122.06]`에 착지한다.
- 여섯 직업의 non-LMB 87개 스킬은 모두 3초, LMB BA 7개는 기존 0초다.
- 여섯 직업 공격력은 모두 1,000이며 기존 damage rate에 대해 원시 피해가 10배가 된다.
- 도넛은 다른 애니메이션을 추측 연결하지 않고 `INNER + animation.mode:NONE + STAGE_CLOCK`
  tombstone을 유지했다.
- 고공 점프 TAKEOFF에 잘못 복제돼 4.51초/6.24초 뒤 발화하던 도넛 파생 요소 두 개를 제거했다.
- Dash Charge는 일반벽, 활성 파괴 receiver, 아직 blocking인 소모 receiver 중 첫 접촉면의 안전 위치에
  즉시 정지하고 같은 fixed tick에 GROGGY로 전환한다.

## 1. 원인과 실제 수정

### 1.1 점프와 3시/9시 착지

- 피자와 두 지형 파괴 패턴의 TAKEOFF는 0..900ms 전체에 걸쳐 10m를 선형 상승하고 있어 체감이 느렸다.
  `takeoffStartMs=800`, `takeoffEndMs=1100`으로 바꿔 예고와 급상승을 분리했다.
- 고공 도끼 점프는 이미 1133..1500ms의 367ms 상승과 0..267ms 강하를 사용하므로 다시 변경하지 않았다.
- 3시/9시의 옆쪽 착지는 방향별 `landingPosition` 때문이었다. 바닥 파괴는 별도 stable world event set이
  소유하므로 landing만 중앙으로 통일하고 floor84/floor30 ID는 유지했다.

### 1.2 쿨타임과 공격력

- 입력 목록을 하드코딩하지 않고 `PlayerSkills.json`의 `inputSlot != LMB`를 기준으로 87행을 3,000ms로
  통일했다. SPACE, T, V/ALT_V, X, Z를 포함하며 LMB BA 7행만 제외된다.
- `PlayerProfiles.json`의 6개 profile `attackPower`를 100에서 1,000으로 변경했다.
- 모든 변경 field를 공식 provenance receipt의 `PROJECT_TUNED` 결과와 동기화했고 publisher가 전체
  230개 skill/109개 damage profile을 다시 검증했다.

### 1.3 도넛과 고공 점프 레거시 Effect

- 도넛이 멈추는 직접 원인은 Effect cue가 아니라 현재 presentation의 `animation.mode:NONE`이다.
  실제 재생 중인 정본 animation/action이 확인되지 않아 과거 프리뷰용 4-stage와 19_02/19_04 clip은
  복원하지 않았다. independent 도넛 cue도 다른 animation에 새로 연결하지 않았다.
- TAKEOFF Effect의 길이는 1.933초인데 `sprite_particle_2`가 4.51초, 도넛 wave가 6.24초에 시작하도록
  살아 있어 착지 뒤 다른 stage에 누출됐다. 이 두 요소만 제거하고 source-native 요소 하나를 남겼다.
- 제거된 donor를 참조하던 두 생성 문서는 cataloged center-landing source로 provenance를 옮겼다.
- `fx_e_decal_007_2.dds`의 owner는 floor-wipe second-smash, swing clip-02, four-pillars target-cone 세 곳뿐이며
  도넛과 고공 점프에는 없다는 계약을 테스트로 고정했다.

### 1.4 Dash의 일반벽·파괴벽 접촉

- 기존 sweep은 impact receiver만 보았다. Valtan의 대상 wall 99개 중 receiver 결합 벽은 40개이고,
  69개 일반벽은 collider-contact binding만 가져 중앙 Dash가 receiver보다 먼저 일반벽을 통과할 수 있었다.
- Dash에만 모든 활성 player-blocking collisionBox sweep을 적용했다. 같은 면의 base collision ID와 optional
  receiver ID를 함께 보존하고, exact receiver mutation을 먼저 시도한 뒤 일반벽이면 base contact mutation을
  한 번 적용한다. Armor Break Opening 등 비-Dash charge는 기존 receiver-only 경로를 유지한다.
- 신규 파괴가 없더라도 실제 blocking surface 접촉 자체를 `WALL_CONTACT`로 발행한다. 따라서 이미 소모된
  receiver의 남은 벽면도 파괴를 반복하지 않으면서 이동을 멈추고 같은 tick에 GROGGY로 간다.
- 기존 몸체가 벽 반경과 겹친 ratio-zero 상황은 벽에서 멀어지거나 접선으로 가는 이동을 허용하고,
  벽 안쪽으로 더 진행하는 경우만 contact로 남겨 영구 정지를 피했다.
- Dash graph는 `WINDUP -> CHARGE -> GROGGY -> RECOVERY -> PART_BREAK`다. CHARGE의 WALL_CONTACT만
  GROGGY, TIMEOUT은 RECOVERY이며 GROGGY 중 part 파괴는 PART_BREAK로 직접 전이한다.

### 1.5 검증 중 발견한 정본 드리프트

- Debug fight-page는 boundary HP를 설정해도 Product ordered sequence가 threshold 평가를 먼저 가로채 네
  페이지 모두 `VALTAN_WHIRLWIND`를 시작했다. page start에만 audition override를 켜 entrance/threshold를
  먼저 소비하고, 기존 Brain이 첫 pattern 선택 직후 override를 해제하도록 교정했다.
- 작업 시작 base의 Effect V2 파일 20개가 Client project에 누락돼 정본 gate가 실패했다. generator로
  예상 diff가 `Client.vcxproj` 20행과 `.filters` 60행뿐임을 확인했다. PR 직전 최신 main이 같은 등록을
  포함해 재정렬 뒤 branch의 별도 project diff는 없어졌고, 현재 2,395 files/219 filters가 exact 일치한다.
- PR 직전 최신 main의 Valtan four-hit 단일 clip 변경이 cue 하나와 damaging pulse 하나를 제거했지만 전수
  count 검증은 이전 81 cues, 67 stages/103 pulses를 유지하고 있었다. 최신 정본의 80 cues,
  66 stages/102 pulses로 기대값을 동기화하고 Debug/Release Server contract를 다시 통과시켰다.

## 2. 자동 검증

| 검증 | 결과 |
|---|---|
| Valtan 빠른 전투 튜닝 focused contract | PASS, 2 tests |
| Valtan pattern tree contract | PASS, 21 tests |
| Valtan All Effects contract | PASS, 17 tests |
| Valtan Pattern Master V2 | PASS, 49 tests, 296.060s |
| Animation Tool pattern master | PASS, 11 tests |
| Effect Tool saved rows | PASS, 32 tests, 7 skipped |
| Gameplay balance runtime set Validate | PASS |
| Server navigation Validate | PASS |
| Effect source validation | PASS, direct sources 196, generated artifacts 0 |
| Effect project registration Check | PASS, 2,395 files / 219 filters |
| Server x64 Debug build | PASS |
| Server Debug `--contract-test` | PASS, `failures : 0` |
| Server x64 Release build | PASS |
| Server Release `--contract-test` | PASS, `failures : 0` |
| NetworkProtocolHarness Debug | PASS |
| Valtan four-player live harness Debug | PASS, `failures : 0` |
| 독립 최종 diff 리뷰 | P0/P1 없음 |

`Invoke-BuildAndRegression.ps1 -Configuration Debug -SkipBuild`는 이번 변경 gate와 Server/Valtan 실행형
하네스까지 통과했으나 저장소 기존 무관 회귀 세 곳 때문에 전체 프로세스 exit 0은 아니었다.

- floor emissive contract는 현재 구현의 local `emissiveIntensity`를 과거 `m_fEmissiveIntensity` 문자열로
  찾고 있어 1건 실패한다. 관련 구현과 테스트는 이번 diff에 없다.
- ground-target preview contract는 현재 코드에 없는 `constexpr void Commit_Submission()` 표식을 찾아
  1건 실패한다. 관련 구현과 테스트는 이번 diff에 없다.
- Character Select isolation harness는 스킬을 쓰기 전 monster center 2m 이내 이동을 기다리다가 timeout된다.
  이번 attackPower가 소비되기 전 단계이며 harness/이동/collision 코드는 이번 diff에 없다.

사용자가 치명적 회귀가 아니면 검증을 닫기로 한 경계에 따라 위 세 baseline drift는 이번 PR에서 확장
수정하지 않았다. 변경 C++가 포함된 Server는 Debug/Release 모두 전체 contract `failures : 0`으로 닫았다.

## 3. 사용자 육안 검증

에이전트는 Client를 실행하거나 화면을 대신 판정하지 않았다. 최신 Server를 재시작하고 사용자가
`Framework.slnLaunch`의 `Server + Client` profile로 다음을 확인한다.

1. 피자 패턴이 800ms 예고 뒤 약 300ms 안에 빠르게 상승하는지 확인한다.
2. 3시/9시 패턴에서 지정된 방향의 바닥만 파괴되고 발탄은 두 경우 모두 중앙에 착지하는지 확인한다.
3. 고공 점프 착지 뒤 4.51초/6.24초 시점에 레거시 링/데칼이 뒤늦게 나타나지 않는지 확인한다.
4. Dash가 일반벽, 활성 파괴벽, 이미 한 번 파괴를 시작한 blocking 벽에 닿자마자 멈추고 GROGGY
   animation으로 즉시 전환하는지 확인한다.
5. BA를 제외한 Q/W/E/R/T/A/S/D/F/SPACE/V/ALT_V/X/Z 슬롯이 3초인지 확인한다.
6. 공격력 1,000으로 발탄 처치 속도가 의도한 검증 수준인지 확인한다.

도넛 animation은 이번 변경에서 보류했으므로 visual PASS 대상이 아니다. tombstone 상태에서 다른 패턴
animation이나 과거 프리뷰 clip이 재생되지 않는지만 회귀 관찰하면 된다.

## 4. 남은 경계

- ratio-zero에서 outward/tangent 이동을 허용하는 코드는 최종 리뷰로 확인했지만 해당 두 방향만 떼어낸
  별도 unit case는 없다. 일반벽/receiver/소모 receiver 통합 계약은 Debug와 Release에서 모두 통과했다.
- 벽 mutation 뒤 stage hook이 실패하는 내부 오류 경로는 기존 receiver 구현과 같이 boss를 복원하고 room을
  즉시 fail-close하지만, 이미 commit·broadcast한 벽 mutation 자체를 되돌리지는 않는다. 승인된 Dash graph의
  정상 경로에서는 해당 실패가 재현되지 않았고 room이 불일치 상태로 계속 실행되지 않으므로 이번 merge의
  치명 회귀로 분류하지 않았다. 완전한 cross-runtime rollback은 별도 transaction 설계 범위다.
- Client visual fidelity와 실제 animation 전환 프레임은 사용자의 육안 승인 전까지 PASS로 기록하지 않는다.
- 팀 LAN sync 결과 현재 PC는 `server-host`다. TCP 7777 LocalSubnet 방화벽 규칙 추가가 필요하면 관리자
  PowerShell로 실행해야 하지만 endpoint의 로컬 설정 완료와 이번 빌드/검증을 막지는 않는다.
