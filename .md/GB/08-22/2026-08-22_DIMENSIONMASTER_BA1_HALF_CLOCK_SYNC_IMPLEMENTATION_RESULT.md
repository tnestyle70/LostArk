# 차원술사 BA1 반속 재생·이펙트 동기화 구현 결과

## 1. 완료 상태

차원술사 `2050010` BA1의 Server 권위 시간을 1.4초에서 0.7초로 줄였다. 캐릭터는 기존 exact source clip의 1.4초 공격 구간을 2배속으로 한 번 재생하고, clip local 0ms의 BA1 unified 이펙트도 같은 2배속 action clock을 소비한다.

| 계약 | 완료 내용 |
|---|---|
| Server action | BA1 duration/advance/input close `700ms`, hit/input open `50ms` |
| animation | `pc_sp_m_00_sk_att_battle_1_01`, `playMs=1400`, `playRate=2.0`, stage당 clip 1개 |
| effect | BA1 exact clip의 Product `effectref=asset` 1개를 0ms에 유지, playback rate 2.0 전달 |
| root motion | stage 0의 43개 sample 시간을 0~700ms로 압축, 변위와 최종점 유지 |
| combo authority | Server snapshot stage만 소비하며 Client 자동 진행을 추가하지 않음 |
| rollback | `playRate=0` replacement를 거부하고 직전 valid binding을 보존 |

## 2. 원본·제품 증거

- 제품 WModel: `Character/DimensionMaster/DimensionMaster_Character.wmodel`
- SHA-256: `405FADB7D9F82B36BA7E54F6482B10056B770C6BE9A612E01D8429DFFD844E96`
- exact clip: `pc_sp_m_00_sk_att_battle_1_01`
- clip 실측: 30 TPS, 4.0초
- 무기 bone: `b_wp_swm_m_1`
- focused harness가 source 0.0/0.1/1.4초의 무기 pose가 유한하고 서로 달라 실제 비정적 찌르기 구간임을 확인했다.

## 3. 자동 검증

| 검증 | 결과 |
|---|---|
| `Publish-GameplayBalance.ps1 -Mode Validate` | PASS, 230 skills / 108 damage profiles |
| `Publish-BalanceRuntimeSet.ps1 -Mode Validate` | PASS |
| Engine x64 Debug/Release build + `UpdateLib.bat` | PASS |
| Shared x64 Debug/Release build | PASS |
| Server x64 Debug/Release build | PASS |
| ClientFrontendHarness x64 Debug/Release build | PASS |
| Debug `--dimension-ba1-sync-fast` | PASS 6/6, failures 0 |
| Release `--dimension-ba1-sync-fast` | PASS 6/6, failures 0 |
| Debug `Server.exe --contract-test` | PASS, failures 0 |
| Release `Server.exe --contract-test` | PASS, failures 0 |

전체 `--skill-binding-fast`의 BA1 신규 assertion은 PASS했다. 같은 실행의 `2050240` delayed cue와 DimensionMaster exact roster 두 실패는 통합 기준점부터 존재하는 비관련 기준이다. 후자는 이미 제품에 존재하는 STANDUP `2050030` 때문에 test의 13행 기대와 실제 14 binding이 불일치하며, 이 변경에서는 범위를 넓혀 수정하지 않았다.

## 4. 수동 검증 경계

Client를 에이전트가 실행하거나 화면 결과를 대신 판정하지 않았다. 사용자는 빌드 통합 후 차원술사 LMB를 한 번 눌러 다음을 확인한다.

1. BA1 unified 검격이 시작될 때 캐릭터가 동시에 한 번 찌른다.
2. 공격과 이펙트가 약 0.7초 안에 함께 끝나며 캐릭터 pose가 중간에 잘리지 않는다.
3. 추가 입력이 없으면 BA2로 자동 진행하지 않는다.
4. BA1 입력 창 안의 두 번째 실제 클릭만 BA2로 이어진다.

최종 visual fidelity는 사용자 육안 승인 전까지 `manual pending`이다.
