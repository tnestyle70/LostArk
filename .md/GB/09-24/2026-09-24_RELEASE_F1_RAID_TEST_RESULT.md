# Release F1 레이드 테스트 통합 결과

## 구현 범위

PR #454의 Retail 수치를 `d9361f98f`로 병합했다. Player/Skill/Damage/Boss와 World monster 게시가 같은 Retail profile을 사용한다. 적용 범위는 해당 Server catalog를 사용하는 모든 world와 Debug/Release 구성이다. Client에만 피해 배율을 덧붙이거나 창술사 ALT_V를 보스 제거 수단으로 사용하지 않는다.

F1은 두 구성에서 닫힌 상태로 시작하며 Balance Test, Profiler, Valtan/Kouku Boss Tool과 Arena/Load Pattern/Complete Play를 연다. F6 카메라, F7 Profiler 및 FPS 상시 표시는 유지한다. Profiler 창을 열어도 Capture를 자동 시작하지 않는다. 패턴 문서는 명시적인 Load에서 읽는다. Release에서 일반 Map/Effect/Sequence 저작 창은 열지 않는다.

UI 진입점뿐 아니라 기존 dependency preparation, 서버 승인, fixed tick, 응답·수명 처리와 종료 정리도 연결했다. Valtan은 기존 stable pattern/flow 명령을, Kouku는 기존 raid admission과 모든 참가자의 준비 완료 응답을 사용한다. Release publish 요청은 기존 Composition publisher backend를 사용하며 별도 데이터 게시 경로를 만들지 않는다.

Kouku 시작점 복귀는 기존 trigger system 설정을 복사한 후보에서 참가자 접촉/입력 상태와 latch를 초기화한 뒤 commit한다. 새 빈 system으로 교체하며 유실되던 world·지면 조회·로그·빌드별 wave 정책을 보존한다. Release Book1 자동 진입과 Debug F1 wave suppression을 실제 overlap 진입으로 검증한다.

기존 Valtan Players/Bosses 수치 편집 진입은 F1 공용 Balance Test로 옮겼다. 패턴 저작과 저장을 소비하는 CBalanceTool backend는 유지한다. 편집 값은 Retail override를 합친 실제 값이며 field 소유 문서에 저장한다. Save+Validate와 Publish Server Data 뒤 Server 재시작으로 수치를 적용한다.

`Debug (3s)` / `Release (Retail)` 버튼은 빌드 구성과 독립적인 현재 room 정책이다. 기본값은 두 빌드 모두 3초이며, 기존 0초 평타·콤보와 기믹/차량 타이머는 유지한다. Retail 선택 시 ALT_V는 300초다. 같은 room의 모든 플레이어에게 즉시 반영하고 이미 진행 중인 쿨타임은 최초 사용 시점에서 재계산한다. HUD는 Server가 보낸 실제 duration으로 원형 진행률을 표시한다. Shared protocol은 109이므로 Client와 Server를 함께 갱신한다.

Kill Current Gate Boss는 현재 관문 primary boss의 HP를 0으로 만든다. 일반 사망 판정과 clear/Encore 소비자가 후속을 결정하며 다른 관문이나 소환체를 일괄 제거하지 않는다. Debug/Release 모두 같은 검증을 사용한다.

## 기존 수정과 연결

PR #453 통합의 앵콜은 앞부분 5초를 줄인 21.322초 sequence와 기존 Server clear 후 5초 대기를 사용한다. 관문 clear UI는 기존 공통 UI presentation을 sequence 시간에 맞춰 유지한다. World mesh/Effect로 UI를 승격한 방식이 아니다. 시작 cue 3초, 첫 균열 7.5초, 파편 약 10.433초의 저작 시간을 사용한다.

카드미로 spawn Y 기준 navigation 재게시, 마지막 상승 시작점에서 갈고리 해제, Release Character Select 선행 로드, 지연 snapshot 이동 보정, Bern의 불필요한 LOD 경계 계산 제거는 이전 통합 RESULT의 완료 상태를 유지한다. 이번 변경이 4인 화면 체감 검증까지 수행한 것은 아니다.

공굴리기 P81/P118의 누락된 counter 성공 간선을 기존 같은 관문의 groggy 후속에 연결했다. 1167+1667+1333ms의 무력화 완료 뒤 원래 flow로 돌아간다. Composition revision 2237을 공식 publisher로 게시했다.

Retail 통합 검증에서 GameplayCatalog 재로드의 버프 두 lookup, damage formula, ember map이 기존 transaction에 포함되지 않은 문제도 확인했다. 동일 generation 교체와 late failure rollback에 네 map을 함께 묶어 중복 버프 누적과 이전 damage 값 잔류를 막았다. 실제 제품 bootstrap의 반복 로드·값 교체·실패 후 이전값 보존을 두 구성에서 검사했다.

## 실제 F1 화면과 체력별 flow 후속

optional entryGroups의 저장·표시·게시와 Server HP 반복 scheduler, Complete Play 연결은 구현했다. 반복 중 HP와 run epoch를 유지하고 패턴/후속 완료 시 기믹 전환 경계를 판단한다. 여러 임계를 넘긴 경우 아직 시작하지 않은 일반 묶음은 건너뛰고 기믹을 차례대로 한 번씩 실행한다.

사용자가 기존 평면 목록을 확인한 뒤 실제 반영을 요청하여 G1 authoring과 공식 publisher 출력을 revision 2238로 갱신했다. 130 무력화 / 110 진짜세이튼 / 85 댄스타임 / 60 무력화 / 50 룰렛 / 30 진짜세이튼 사이에 일반 12패턴과 기존 추적 연결을 반복한다. 쓰리투원투하를 포함하며 현재 패턴과 카운터 후속 완료 후 기믹으로 전환한다.

반복 7구간과 기믹 6개, 총 159 entry / 13 group을 실제 Data와 Server bootstrap에 게시했다. 기존 28개 stable entry와 다른 관문을 보존했고 F1은 반복 청크와 일회 기믹을 구분한다. 상세 검증은 G1 HEALTH_FLOW RESULT에 기록했다. 이 게시가 이미 실행 중인 room의 자동 교체를 의미하지는 않는다.

Player Follow Camera ImGui 패널과 전용 draft를 제거했다. 사용자의 최종 선택에 따라 그 자리에 `Open Balance Test` 버튼을 배치하고 기존 별도 수치 창을 사용한다. F1이 닫혀도 저장·게시 결과를 수거한다. 카메라 runtime profile과 F6 동작은 유지한다.

접힌 Balance Test 창을 다시 Open하면 펼침·초점 요청을 공용 패널에도 적용하도록 조기 반환 순서를 수정했다. F1 공통 Character Select Movie는 Guardian Knight를 기본 선택하며 기존 Level의 연출 소유자로 Play/Restart/Stop을 실행한다. 다른 Level에서는 Character Select 진입을 안내한다. 기존 설치 리소스를 재사용하며 영화 재생은 사용자 확인 대상이다.

08:33:32의 Complete Play 준비 실패는 고정 Action 2237과 게시 중 저장 Action 2238의 불일치였다. 이후 START도 같은 게시 진행 구간에 발생했다. 기존 검증을 유지하면서 성공한 목록 로드 안내가 실패 이유로 남지 않도록 expected/current revision을 표시하고, Server 게시 잠금·로드·baseline·Action/Sequence 불일치를 구분한다. 기존 실행 로그를 화면 성공 근거로 사용하지 않는다.

## PR #456 최초 통합 검증 기록

아래는 Composition revision 2237과 최초 Release F1 통합 시점의 제품 빌드·native 검사 기록이다. 이후 F1 launcher·Character Select Movie·HP Flow revision 2238 변경의 최종 검증은 다음 섹션으로 구분한다. 이 최초 통합 로그는 `out/ReleaseRaidTools20260924`에 있다.

- Composition publish: PASS, revision 2237, product pattern 113개 / stage 574개.
- Retail Balance Runtime Set publish: PASS, Gameplay·4개 World·Items transaction 완료.
- NetworkProtocolHarness: Debug/Release 각각 1,269 PASS, 실패 0.
- Debug Product: PASS, `out/BuildPipeline/runs/20260923T224833539Z-debug-product.json`.
- Release Product: PASS, `out/BuildPipeline/runs/20260923T224850148Z-release-product.json`.
- Balance 저장 transaction: 10 PASS. HP flow 및 실제 PowerShell schema 검사: 13 PASS.
- 변경 JSON/XML 5개 parse, PowerShell 6개 AST, C++ 62개 기존 인코딩/BOM 및 혼합 개행 검사: PASS.
- `git diff --check`: PASS.

| Server 검사 | Debug PASS | Release PASS | 실패 |
|---|---:|---:|---:|
| Valtan pattern control | 8 | 9 | 0 |
| Kouku draft / HP flow | 20 | 12 | 0 |
| Kouku support / counter / profile reload | 307 | 28 | 0 |
| Kouku object overlap | 660 | 628 | 0 |
| Teleport / raid integration / cooldown / Kill | 8,109 | 7,576 | 0 |

각 CLI는 `Server.exe --<검사명>-contract-test`로 실행했다. 실제 이름은 순서대로
`valtan-pattern-control`, `kouku-draft`, `kouku-support-surface`, `kouku-object-overlap`, `debug-teleport`다.
증거 목록은 `out/ReleaseRaidTools20260924/verified-native-summary.json`이다. Debug 최종 support와
teleport 로그는 `-verified.log`, object overlap은 `-final.log`, 나머지는 기본 `.log`를 따른다.
실패했던 최초 검사를 최종 통과로 덮어 기록하지 않고 각 최종 로그 경로를 구분했다.

## 2026-09-24 최종 F1·HP Flow 후속 검증

F1 Balance Test launcher 이동, 공용 창 작업 결과 수거와 펼침 처리, Character Select Movie, 실제 G1 HP 그룹 및 준비 실패 원인 표시를 반영한 최종 제품 빌드를 확인했다. 두 receipt 모두 `profile=Product`, `result=PASS`, `skippedBuild=false`다.

| 검사 | Debug | Release | 증거 |
|---|---:|---:|---|
| Product build | PASS | PASS | `out/BuildPipeline/runs/20260923T234847724Z-debug-product.json`, `20260923T234951599Z-release-product.json` |
| Kouku raid 계약 | 1,252 PASS / 0 FAIL | 978 PASS / 0 FAIL | `out/F1BalanceGate1-Debug-raid-contract.log`, `F1BalanceGate1-Release-raid-contract.log` |
| Kouku draft / HP Flow 계약 | 20 PASS / 0 FAIL | 12 PASS / 0 FAIL | `out/F1BalanceGate1-Debug-hp-contract.log`, `F1BalanceGate1-Release-hp-contract.log` |

- Balance 저장 transaction: 최종 14 tests PASS. 위 최초 통합의 10 tests와 별개인 후속 검증이다.
- Movie 설치 참조: 기존 758개 파일의 존재·비어 있지 않음과 JSON 9개 parse PASS. source/runtime WorldSequence 객체가 일치한다. 증거는 `out/CharacterSelectMovie20260924/installed-resource-check.json`이다. 이 검사는 실제 movie 재생·GPU 표시 성공을 의미하지 않는다.
- 최종 변경 C++ 13개 인코딩·BOM·개행 검사 및 변경 JSON 3개 parse: PASS. 파일별 기존 형식을 유지했다. 증거는 `out/F1BalanceGate1-final-encoding.json`이다.
- 이 문서 변경의 `git diff --check`: PASS.

Debug bundle 최초 검사는 구형 fixture가 DAMAGE 행의 마지막 spread 열을 rate로 가정해 유효 범위를 벗어난 값으로 바꾸는 문제로 1건 실패했다. rate 열만 수정하고 실제 admission이 기존 rate/coefficient/addend/spread를 보존하는지 검사하도록 고쳤다. 제품 runtime과 Release 전처리 산출물은 변경하지 않았다.

후속 Debug Server 증분 빌드는 PASS이며 `out/F1BalanceGate1-Debug-server-bundle-fix-build.log`에 기록했다. 최종 `--kouku-bundle-contract-test`는 95 PASS / 0 FAIL이다. 실패 이력은 `out/F1BalanceGate1-Debug-bundle-contract.log`, 최종 통과는 `out/F1BalanceGate1-Debug-bundle-contract-final.log`로 구분한다. 검사 뒤 Gameplay 및 세 spawn-group 게시 hash가 최종 Retail 검증 값과 일치했다.

Client/UI는 실행하지 않았다. F1 동작 화면, 4인 모드 변경과 실제 전투 체감, Encore UI 타이밍 및 FPS 비교는 사용자 수동 확인으로 남는다.
