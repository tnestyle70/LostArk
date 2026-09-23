# 공용 Balance Test와 현재 관문 보스 처치 결과

## G01. 공용 수치 UI와 실제 저장

`F1 -> Balance Test`는 Players/Skills/Damage/Bosses의 숫자 scalar를 편집한다. 새 panel은
Valtan 문서를 로드하지 않는다. 기존 CBalanceTool의 Valtan typed source/CAS/후보 게시 owner는
보존했고, 기존 창의 Players/Bosses 탭과 base-stat editor를 제거했다. Animation Workbench의
세 숫자 편집 진입점도 공용 panel로 연결했다. 기존 typed Valtan 패턴/stage/animation/effect
호출을 제거하거나 별도 런타임을 만들지 않았다.

일반 숫자 저장은 stable ID/field/이전값 patch를 별도 job으로 실행한다. 최신 저장본의 같은
field가 변경됐으면 거부하고, 무관 field와 미지원 schema 항목은 보존한다. provenance와
gameplay publisher를 candidate overlay에서 먼저 검증하고 최신 bytes를 재확인한 뒤 원자
교체한다. 여러 파일 중 뒤 파일의 교체가 실패하면 앞서 쓴 자기 변경만 원래 bytes로 복구한다.
공용 Save는 Valtan draft나 Server runtime을 자동 Reload/Hot Reload하지 않는다.

`Publish Server Data`는 저장된 source의 기존 통합 publisher를 비동기로 실행한다. 저장과
게시 성공 메시지는 구분하며 Server와 Client 재시작이 필요한 상태를 명시한다. panel을 닫거나
Client owner가 소멸해도 진행 중 원자 writer를 TerminateProcess로 중단하지 않는다.

## G02. 현재 관문 보스 처치

Balance Test와 F1 Valtan/KoukuSaydon Arena의 `Kill Current Gate Boss`는 같은 typed command를
사용한다. `IPlayerCommandSink -> NetworkPlayerCommandSink -> Shared -> ServerApp -> RoomCommand
-> CGameRoom::Apply_DebugKillGateBosses`로 연결했다.

- Server가 현재 room과 현재 gate의 stable primary placement를 결정한다.
- Valtan은 `boss.valtan.center/BOSS_VALTAN`, 쿠크는 현재 관문 primary가 대상이다.
- G2는 두 actor를 함께 대상으로 잡고 다른 관문·종속 소환체를 보존한다.
- 요청의 HUD archetype은 대상 선택 권한이 아니라 오래된 화면 요청을 거부하는 조건이다.
- session별 sequence 재전송과 바뀐 boss의 지연 요청을 거부한다.
- HP/DEAD 입력만 변경하며 기존 `Update_WorldEntities`가 사망 정리, G3 Encore, gate clear,
  보상/MVP를 처리한다. 최종 clear flag를 직접 켜거나 DespawnAll로 우회하지 않는다.
- Release Server는 DISABLED를 응답하고 상태를 변경하지 않는다. 명령 성공/거부를 typed result로
  UI에 표시한다.

프로토콜은 108이다. 미병합 #454가 사용하는 107과 다른 snapshot 계약을 같은 버전으로 표시하지
않기 위해 구분했다. 이 변경은 #454 Retail snapshot field를 포함하지 않는다.

## G03. 실행한 자동 검증

| 검증 | 결과 |
|---|---|
| PowerShell 저장 transaction focused test | PASS, 6 tests |
| 최신 무관 field + 미지원 schema 보존 | PASS |
| 같은 field 충돌 시 source/receipt bytes 보존 | PASS |
| candidate 검증 실패 시 source/receipt bytes 보존 | PASS |
| 검증 중 외부 저장 freshness 거부 | PASS |
| 허용하지 않은 field 거부 | PASS |
| 두 번째 파일 promotion 실패 시 첫 파일 byte-exact rollback | PASS |
| Save/Provenance PowerShell AST parse | PASS |
| Client.vcxproj/.filters XML parse | PASS |
| 담당 변경 git diff --check | PASS |
| Debug Product 빌드 | PASS, 신규 Balance Test Client 소스 포함 |
| Release Product 빌드 | PASS, 신규 Balance Test Client 소스 포함 |
| Server Debug debug-teleport 계약 | PASS, `failures : 0`, 현재 관문 Kill 범위·기존 상태 보존·중복/오래된 요청 거부 포함 |
| Server Release debug-teleport 계약 | PASS, `failures : 0`, Kill 명령 거부와 gameplay 보존 포함 |
| NetworkProtocolHarness x64 Debug build | PASS, MSBuild 18 Insiders, 기존 Out/Int |
| NetworkProtocolHarness x64 Debug 실행 | PASS, 1,263 PASS / `failures : 0`, Kill request/result round-trip·거부 조건 포함 |
| NetworkProtocolHarness x64 Release build | PASS, MSBuild 18 Insiders, 기존 Out/Int, BuildProjectReferences=false |
| NetworkProtocolHarness x64 Release 실행 | PASS, 1,263 PASS / `failures : 0`, Kill request/result 계약 포함 |

검증 명령은 `python -B Tools/GameplayPipeline/test_balance_test_transaction.py`다. 테스트는 임시
저장소에서 실제 저장 transaction을 실행하고 validator 성공/실패/동시 저장 시점을 주입한다.
이 임시 저장소 테스트는 제품 source/runtime 파일을 변경하지 않았다. Root의 통합 검증으로
Debug/Release Product 빌드와 각 Server debug-teleport 계약을 통과했다. 빌드 로그는
`out/RaidRelease20260924/product-debug-final.log`, `product-release-final.log`, Server 계약 로그는
`debug-debug-teleport.log`, `release-debug-teleport.log`다. 별도 Hook 종단 projection의 후속 Server
증분 빌드는 이 기능의 소스에 영향을 주지 않으며 해당 통합 RESULT가 결과를 소유한다. Debug packet build/run
로그는 `out/RaidRelease20260924/protocol-debug-build.log`, `protocol-debug-run.log`이고 Release는
같은 위치의 `protocol-release-build.log`, `protocol-release-run.log`다. 두 구성 모두 1,263 PASS다. 이 담당 작업은
제품 publisher와 Client/UI를 실행하지 않았다.

추가한 C++ 계약 테스트는 request/result round trip, sequence/count 거부, Server wrong-world/player,
G2 actor 범위, 다음 관문과 소환체 보존, replay/stale HUD 및 Release 거부를 포함한다.
위 Server Debug/Release와 packet Debug/Release 계약은 모두 PASS다. 실제 4인 Debug 사용,
수치 Save/Publish 이후 재시작, G3 처치 뒤 clear/Encore 및 최종 화면 판정은 사용자 확인 항목이다.

## G04. 미병합 PR #454 수치 영향 리뷰

#454 Retail profile은 발탄 전용 보정이 아니다. 7 class, 114 skill, 128 damage profile,
Valtan/쿠크 7 boss, 10 monster 후보에 영향을 준다. Server의 공용 CGameplayCatalog를 소비하는
Character Select/Bern/Valtan/Kouku 등의 전투에 적용되며 Debug/Release 전용 분기는 없다.
PR에는 Retail Gameplay.bootstrap이 게시되어 있으나 World monster bootstrap 변경은 없다.

| 항목 | 현재 source | #454 Retail |
|---|---:|---:|
| Player AP | 1,000 | 23,000 |
| Player HP | class별 기존 값 | 120,000~150,000 |
| 치명타 | profile 없음 | 확률 70%, 피해 200% |
| 피해 편차 | 없음 | ±10% |
| 창술사 ALT_V 34630 쿨타임 | 3초 | 300초 |
| ALT_V 원시 총 피해 | 676,520 | 37,534,467 |
| Valtan HP | 600,000 | 741,285,439 |
| G3 Saydon HP | 600,000 | 1,280,621,510 |
| Bingo Saydon HP | 600,000 | 1,868,133,028 |

ALT_V Retail 피해는 `23000 * 16286651 / 10000 + 75170`이며 치명타/편차/히트분할/방어 전이다.
보스 대비 비율은 약 2~5%로 바뀌어 기존 즉사 시험을 대신하지 못한다. Kill Boss를 전투 수치와
분리하는 이유다.

후속 통합 전에 고쳐야 할 연결은 다음과 같다.

1. Server Retail cooldown/damage만 덮이고 Client PlayerSkillCatalog는 기존 JSON을 읽는다.
   CombatHUDViewModel tooltip/쿨타임 duration과 MainApp cooldown arc의 분모가 실제 Server와
   달라진다. end tick에 따른 남은 초 표기와 전체 duration 분모를 구분해야 한다.
2. Publish-BalanceRuntimeSet에는 BalanceProfile 인자와 전달이 없다. 기존 F1/통합 게시가
   Retail bootstrap을 기본 빠른 전투 값으로 되돌린다. profile 선택을 게시 계약까지 연결해야 한다.
3. 프로필의 10 monster 값은 별도 World Retail 게시 전에는 실제 배포값이 아니다.
4. boss별 staggerGaugeMaximum을 boss overlay가 읽지 않는다. 실제 구현은
   SET_STAGGER_GAUGE에 전역 400배를 적용하므로 기입값과 적용값을 동일시할 수 없다.
5. Apply_SkillBuffs의 ENEMY 목록이 BOSS만 포함해 MONSTER stun 분기는 제품 호출에서 도달하지
   않는다. attackSpeedPercent도 행동 길이에 아직 소비되지 않는다.

이번 변경은 #454를 병합하거나 Retail 수치를 게시하지 않았다.
