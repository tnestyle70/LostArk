# 쿠크 광기 계산·수치 튜닝·마리오 입장 결과

## G01. 실제 완료

`Apply_WorldToPlayer`의 기존 광기 계산은 최대 100 단위에서 hit마다 정수 나눗셈을 하므로
최대 HP의 1% 미만 피해가 버려졌다. 보호막이 막은 양도 피해로 보았고, 쿠크 외 월드의
기본 gauge에도 적용됐다. 실제 피해 해결 뒤 `hpBefore - currentHp`를 사용하고 player가
소수 잔여량을 보존하게 변경했다. 쿠크 room의 policy가 피해 배율을 설정하며 다른 room은
0이다. 변신·부활·profile/관문 초기화에서 잔여량을 지운다.

기존 damageable WORLD cue는 파괴용 HP와 시각 효과를 가졌지만 광기 접촉 소비자가 없었다.
공/인형 stable sequence ID를 현재 cue 수명에 연결했다. Server tick에서 플레이어 body와
공 원/인형 앞뒤 sector의 접촉을 판단하고 interval마다 한 번만 충전한다. 동일 tick의 두 번째
world update는 중복 충전하지 않는다. 살아 있는 오브젝트는 자연 pattern 종료 뒤에도 유지하고
파괴·취소·소유 보스 사망 시 충전을 끝낸다.

패턴 `Can_EnterMarioEntry`와 기존 Intro의 `Update_MarioControlState` 모두 CLOWN을 요구한다.
일반 player의 자동 변신 입장을 제거했다. 이에 따라 자연 변신 유지시간이 Mario 중 끝났을 때
만료 tick을 지워 영구 광대가 되는 기존 문제도 수정했다. Mario 중 form은 유지하고 퇴장 뒤
다음 mode update에서 일반 player로 복원한다. Debug 무기한 광대는 그대로 유지한다.

Balance Test에 `Madness` domain과 다음 세 배율을 포함한 8개 숫자, 계산식, 근거 설명을 추가했다.
Retail `madness[policyId=KOUKUSAYDON]`가 정본이며 기존 scalar draft/CAS/원자 교체/publisher를
확장했다. bootstrap formatVersion을 37로 올렸으며 `KOUKUMADNESS`는 v37의 base 비교4열과
Retail 수치를 가진12열만 검증한다. v36 이하 전체 bootstrap은 generation admission에서 거부한다.
Server/Client는 Shared37 상수를 소비한다. Valtan Python generation parser의 과거33/8192행 제한도
Shared37/65536행과 맞췄다(실제 기존 게시본은36/40699행이므로 이전 Python 경계는 불일치했다).
Sequence 메모리 draft를 admission할 때 현재 Balance Test 수치를 보존하므로 draft projector의
기본값이 활성 수치 튜닝을 덮지 않는다. 새 C++ 파일이나 packet/project 등록은 없다.

```text
damageGain = maximumGauge × actualHpLost / maximumHp × damageGainPercent / 100
specialPulse = maximumGauge × sourceGainPercent / 100 × sourceMultiplierPercent / 100
total = clamp(previous + damageGain + specialPulses + authoredMechanicGain, 0, maximumGauge)
```

## G02. 원본 근거와 초기값의 구분

원본은 설치된 `C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/data2.lpk`, `data3.lpk`에서
기존 LpkPipeline으로 필요한 table과 native action만 읽기 전용 추출했다. 작업 산출물은
`out/KoukuMadness20260924/tables`, `native`, `native-actions.json`에 있으며 정본 리소스를
교체하지 않았다.

| 근거 | 확인한 값 | 실제 적용 경계 |
|---|---|---|
| ZoneContentsGauge 3708100 | max100, start0, interval0, threshold49/100, maxGaugeHold15000ms | 기존 encounter max/hold 유지 |
| SkillBuff4219994 광기오라_NPC용 | ValueA421991716, Interval1000, FirstInterval1000 | NPC aura를 공 충전의 원본 참고값으로 사용 |
| SkillEffect421991716 → 421990117 | 원형200cm, 후속 gauge3708100 +10 | 공 기본량10%, 반경2m, 주기1초 |
| MN_CDMD_00.loa att_battle_2_01 → 422230501/502/504 | AreaType3, AreaRange400, AreaAngle30 | 인형 저장 yaw의 양쪽4m/30도 sector |
| 피해 비례 계수/인형 광기 amount | 원작 Server 정확한 계수 미확인 | PROJECT_TUNED로 명시 |

검토 후보의 값은 아래와 같다. 공 NPC aura 참고 수치와 인형 공격 영역 외의 프로젝트 선택을
원작 계수로 가장하지 않는다. 특수 배율 200%는 사용자의 증가 요청에 따른 초기값이다.

| 필드 | 후보값 | 근거 |
|---|---:|---|
| damageGainPercent | 100 | PROJECT_TUNED: HP 1% 감소 = gauge1% |
| ballGainPercent | 10 | 원본 NPC aura +10 / 최대100 참고 |
| ballMultiplierPercent | 200 | PROJECT_TUNED: 요청된 증가 |
| ballRadiusM | 2.0 | 원본 NPC aura200cm 참고 |
| dollGainPercent | 10 | PROJECT_TUNED: 인형 광기 정확값 미확인 |
| dollMultiplierPercent | 200 | PROJECT_TUNED: 요청된 증가 |
| dollRadiusM | 4.0 | 원본 인형 공격400cm |
| specialIntervalMs | 1000 | 원본 NPC aura 간격 |

인형 영역은 실제 회전하는 mouth bone의 순간 방향과 화염 mesh 전체를 복원한 것이 아니다.
저장 placement yaw의 앞뒤 sector와 고정 높이 차2m 제한을 사용하는 현재 Server 판정이며
panel과 팀 사용서에 이 경계를 표시했다. 화면과 gameplay 감각은 사용자 확인이 필요하다.

## G03. 검증 증거

- `Publish-GameplayBalance.ps1 -Mode Validate -InputOverlayRoot out/KoukuMadness20260924/candidate -BalanceProfile Retail`: 광기 후보 및 bootstrap37 변경 후 재검증 모두 exit0. 현재 composition revision2254, 8 player profiles, 313 skill rows, 128 damage profiles를 포함한 실제 publisher 검증 통과.
- `python -B Tools/GameplayPipeline/test_balance_test_transaction.py`: 16 tests PASS. 신규 Madness float radius/배율 field 저장 및 동시 same-field 충돌 보존 포함.
- 변경 PowerShell3개 AST parse: PASS.
- 변경 범위 `git diff --check`: PASS. 기존 파일별 encoding/newline 유지(CRLF/LF 정리는 root 통합 확인).
- root 통합 `Server/Client Debug/Release ClCompile`: 광기 기능 반영본 모두 exit0(2026-09-24). 이후 bootstrap37 compatibility 보강과 KoukuProduct admission test 추가분은 root가 다음 통합 빌드에서 확인한다. 실행 중 EXE를 대체하는 Product 링크는 별도.
- Valtan Python targeted 계약2개 PASS: publisher/Shared/Python의 version37/행한도65536 일치, row count 경계, 직전36버전 거부. 설치 bootstrap 자체는 승인 후37로 재생성하므로 이 시점의 구버전36 설치본 admission 성공으로 기록하지 않는다.

추가한 native 검증은 다음 실제 실행 경로에 포함된다. 이 RESULT 작성 시점에는 아직 실행하지
않았으며 컴파일 성공을 실행 성공으로 기록하지 않는다.

| 실행 명령 | 포함한 계약 |
|---|---|
| `Server.exe --contract-test` | 작은 피해10회 누적, shield 실제 HP손실, 비쿠크 비활성, 피해 배율, 무적, clamp, CLOWN 제외, Mario 중 만료 후 복원 |
| `Server.exe --debug-teleport-contract-test` | 공/인형 circle/sector·interval 중복 억제·다른 층 제외·파괴 후 중단, 일반 player 거부/CLOWN 입장과 기존 Mario 루트 |
| `Server.exe --kouku-object-overlap-contract-test` | Mario entry collider의 CLOWN fixture와 높이 제외 |
| `Server.exe --kouku-draft-contract-test` | v37의4/12열 승인,11/13열·NaN·배율범위 거부, active tuning 보존 및 기존 실패 보존 경로 |

## G04. 최종 데이터 교체와 남은 범위

검토 후보: `out/KoukuMadness20260924/candidate/Data/Balance/Profiles/Retail.balanceprofile.json`.
기준 저장본 hash: `out/KoukuMadness20260924/profile-baseline.sha256`.
기존 profile 전체를 재생성하지 않고 `madness` row만 추가했다. 이 RESULT 작성 시점의 정본
`Data/Balance/Profiles/Retail.balanceprofile.json`과 설치 bootstrap에는 아직 반영하지 않았다.
root가 저장본 적용 승인을 한 번 받은 뒤 최신 저장본의 해당 stable row/field만 CAS 병합하고
공식 publisher로 runtime을 생성한다. Client/Server가 실행 중이라는 사실은 데이터 교체의
선행 종료 조건이 아니며 Product link가 필요할 때만 사용자의 종료가 필요하다.

실행 중 Server 자동 reload, Client/UI 실행·조작, 사용자 미저장 draft reload, 최종 화면 확인은
수행하지 않았다. 통합 링크·native 계약 실행·승인 후 데이터 게시 결과는 root가 이어서 갱신한다.

2026-09-24 최종 v37 변경 뒤 Debug/Release Server·Client ClCompile은 모두 exit0이다.
로그는 `out/KoukuMadnessRaid20260924-{server,client}-{debug,release}-v37-compile.log`다.
최종 후보 v37 publisher Validate도 통과했다. 현재 Client PID36716·49220과 Server PID49304의
실행 파일은 이전 상태다. 사용자에게 현재 저장본 병합·게시 및 저장 후 종료를 한 번 요청했고
아직 응답을 받지 않아 정본·실행 데이터를 교체하지 않았다. 원자 병합 준비 스크립트는
`out/KoukuMadness20260924/Apply-MadnessPolicy.ps1`이며 아직 실행하지 않았다.

## G05. 변경 파일

Server: `GameplayCatalog.h/.cpp`, `ServerPlayer.h`, `ServerCombatHitRuntime.h/.cpp`,
`KoukuSaydonLogicRuntime.cpp`, `GameRoom.h`, `GameRoom_KoukuAudition.cpp`,
`GameRoom_KoukuPlayerCommands.cpp`, `GameRoom_Admission.cpp`, `GameRoom_Helpers.cpp`,
`GameRoom_PlayerCommands.cpp`; root 소유 `GameRoom_GateProgress.cpp`, `GameRoom_KoukuRaidFlow.cpp`는
광기 잔여량 reset 줄만 공유했다.

Client: `BalanceTestPanel.cpp`.

Pipeline: `Publish-GameplayBalance.ps1`, `Save-BalanceTestDraft.ps1`,
`build_retail_balance_profile.py`, `KoukuBootstrapRows.ps1`,
`Shared/Public/GameplayDataRevision.h`, `Tools/ValtanPipeline/valtan_tuning_pipeline.py`와
`test_valtan_cross_pattern_followup_pipeline.py`, `test_valtan_pattern_master_v2.py`의 연결 admission fixture.

검증: `test_balance_test_transaction.py`, `ServerGameplayContractTests_KoukuLogic.cpp`,
`ServerGameplayContractTests_KoukuProduct.cpp`, `ServerGameplayContractTests_KoukuOverlap.cpp`,
`ServerGameplayContractTests_DebugTeleport.cpp`.

문서: 이 PLAN/RESULT, `CLAUDE.md`의 Balance/광기 문단,
`TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`의 Balance/Mario 문단, `BALANCE_TOOL_OWNER_HANDOFF.md`.

## G06. 승인 후 v37 게시와 실제 Server 초기화 복구

사용자가 `Process gameplay generation failed to initialize. Status=Gameplay bootstrap header is
invalid` 오류와 v37 코드/v36 게시본 불일치를 제시하고 반영을 요청했다. G04의 미반영 상태는
이번 작업에서 해소했다. 당시 게시 header는 `LOSTARK_GAMEPLAY_BOOTSTRAP 36 40699`였으며
행 수 40699는 정상이고 exact version 검사에서 거부되는 상태였다. 직전 Product compile/deploy
PASS는 Gameplay schema 호환성 검사까지 포함한 실행 준비 완료가 아니었다.

현재 Retail 저장본과 준비 후보를 다시 비교했으며 차이는 `madness` 하나였다.
`out/KoukuMadness20260924/Apply-MadnessPolicy.ps1`로 최신 bytes를 읽어 해당 항목만 삽입했다.
canonical writer/destination mutex, candidate Validate, 교체 직전 hash 재확인, 백업 및 원자 교체를
사용했고 JSON 비교로 다른 모든 profile 항목의 불변을 확인했다. 백업은
`out/KoukuMadness20260924/final-9a9f1769a55d4d10ad5bbf6e9aef2415/Retail.before.json`이다.

공식 `Publish-GameplayBalance.ps1 -Mode Publish -BalanceProfile Retail`은 PASS다.
새 게시 header는 `LOSTARK_GAMEPLAY_BOOTSTRAP 37 40699`이며 기존 bootstrap과의 행 multiset
차이는 `KOUKUMADNESS` 한 행뿐이다. 4열 기존 행이 아래 12열 수치 행으로 교체됐다.

```text
KOUKUMADNESS ENCOUNTER_KAKULSAYDON_G1 100 15000 100 10 200 2 10 200 4 1000
```

게시본 SHA256은 `cfb6654078f0bf879bf50d57fb2b984704950f8dffb96bed860650e153ad6676`이다.
헤더를 직접 수정하거나 버전 검사를 느슨하게 하지 않았다. 생성물은 공식 publisher가 기록했다.

현재 설치 Debug Server.exe로 다음 검사를 실행했다.

- `--kouku-draft-contract-test`: 실제 `CGameplayCatalog::Load()` 및 v37 admission 등 29항목
  PASS, `failures : 0`, exit 0.
- `--headless --bind-address 127.0.0.1 --port 17778 --smoke-timeout-ms 1000`: 실제 process
  gameplay generation과 world simulation 초기화 후 Listening, 자동 종료, exit 0.
  이 loopback 주소는 격리된 단기 검사에만 사용했으며 팀 LAN 설정은 변경하지 않았다.
- JSON parse, profile의 무관한 field 보존, bootstrap 버전·실제 행 수·행 차이, 변경 범위
  `git diff --check`: PASS.

근거는 `out/KoukuMadness20260924/{apply-policy-final.log,publish-v37-final.log,
publish-v37-verification.json,server-v37-draft-contract.log,server-v37-startup.log}`다.
이번에는 C++/EXE를 변경하지 않아 재빌드가 필요하지 않았다. 검사 Server는 자체 timeout으로
종료됐고 사용자 프로세스를 종료하거나 Client/UI를 실행하지 않았다. 이제 같은 Server.exe를
다시 실행해 사용할 수 있으며 Character Select 입장·컷신 외형은 사용자 화면 확인으로 남는다.
