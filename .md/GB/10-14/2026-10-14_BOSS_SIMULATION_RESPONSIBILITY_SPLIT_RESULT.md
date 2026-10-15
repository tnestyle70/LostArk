# G0. 보스 시뮬레이션 분리 — 소스 반영 완료, 전체 검증은 사용자 편집 종료 후 재개

2026-10-14. 구조 정리는 에이전트가 반영하고 패턴 동작은 사용자가 직접 구현하는 범위다.
분리 코드는 현재 제품 소스에 남아 있다. **마지막 요청에 따라 추가 빌드와 진행 중 계약 검사를 중단했다.**
사용자가 Client를 편집 중임을 확인했으므로 그 변경은 수정하거나 되돌리지 않았다.

## 실제 수정한 파일

| 파일 | 현재 책임 |
|---|---|
| [GameRoom_BossSimulation.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_BossSimulation.cpp) | 기존 3,184행에서 55행으로 분리. 한 tick의 호출 순서만 조율 |
| [GameRoom_WorldEntitySimulation.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_WorldEntitySimulation.cpp) | 개체 종류별 분기, 발탄 보상, 종료 개체 제거 |
| [GameRoom_ValtanSimulation.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_ValtanSimulation.cpp) | 발탄 Brain 갱신, stage 진입 transaction, 이동 적용 |
| [GameRoom_ValtanGhost.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_ValtanGhost.cpp) | 유령 전환·재배치·포탈 scheduler와 부속 보스 수명 |
| [GameRoom_KoukuTargeting.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_KoukuTargeting.cpp) | 표식·랜덤 발사·추적 투사체 |
| [GameRoom_KoukuMechanics.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_KoukuMechanics.cpp) | 공중 이동 확정, 기믹·소환·접촉·진짜/가짜 세이튼 |
| [GameRoom.h](C:/Users/tnest/Desktop/LostArk/Server/Public/GameRoom.h) | private helper 여섯 개 선언. 새 상태·public API 없음 |
| [Server.vcxproj](C:/Users/tnest/Desktop/LostArk/Server/Default/Server.vcxproj), [.filters](C:/Users/tnest/Desktop/LostArk/Server/Default/Server.vcxproj.filters) | 새 CPP 다섯 개를 기존 물리 경로·필터에 각 한 번 등록 |
| [ValtanClearRewards.h](C:/Users/tnest/Desktop/LostArk/Server/Public/ValtanClearRewards.h), [ValtanBrain.h](C:/Users/tnest/Desktop/LostArk/Server/Public/ValtanBrain.h), [KoukuSaydonBrain.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/KoukuSaydonBrain.cpp) | 옮겨진 직접 호출자 참조 주석 세 줄 교정 |

전체 코드는 [구조 분리 PLAN](2026-10-14_BOSS_SIMULATION_RESPONSIBILITY_SPLIT_PLAN.md)에 있다.
상태 소유자는 계속 CGameRoom 하나이며 같은 Room worker에서 순서대로 실행한다. 새 manager·캐시·병렬 실행은 없다.
기존 continue는 개체 helper의 true, 전체 갱신 중단은 false로 전달한다. 내부 loop의 continue와 이동 lambda의 return은 유지한다.
개체 vector의 생성·제거 경계, stage rollback, 부착 해제, 보상 후 despawn 순서를 보존했다.

## 최신 변경 보존과 검증 근거

이전에는 다른 세션의 저장과 겹쳐 자기 변경만 회수하고 대기했다. 사용자가 재개를 요청한 뒤 적용 대상 12경로를 다시 읽었다.
기존 파일은 보관한 before와 byte가 같았고 신규 파일 다섯 개는 없었다. 이 최신 상태 위에 적용하고 무관한 미커밋 변경은 보존했다.
적용 영수증은 `out/BossSimulationRefactor20261014/resume-application.json`이다.

| 실행한 검사 | 결과 |
|---|---|
| 원본과 분리본 독립 검토 | 실행 토큰, 제어 범위, 실패 복원·호출 순서에서 결함 발견 없음 |
| 현재 소스·PLAN·후보 대조 | 12파일 일치, 기존14+신규6 함수 정의 각 한 번 |
| 프로젝트/XML·자기 변경 diff 검사 | 통과. 새 CPP 등록 각 한 번. 기존 Brain 사용자 주석의 공백 두 곳 보존 |
| 첫 Product 시도 | 분리 파일의 내부 helper include/namespace 누락 발견 후 수정 |
| 수정 후 Product Debug | Engine·Shared·Server 컴파일/링크 통과. Client MainApp.cpp의 편집 중 오류로 전체 FAIL |
| 최신 Client 재시도 | 진행 중 자기 Server 계약 검사에 대해 Product output guard가 거절. 우회하지 않음 |
| 새 Server Kouku product 계약 검사 | 사용자 정리 요청으로 중단. 중간 PASS를 전체 통과로 기록하지 않음 |
| 새 Server overlap·Valtan lifecycle | 순차 대기 중이었으며 실행하지 않음 |
| Client 화면·실제 사용자 플레이 | 실행하지 않음 |

수정 후 빌드 정본: [Product 결과 JSON](C:/Users/tnest/Desktop/LostArk/out/BuildPipeline/runs/20261014T130645453Z-debug-product.json).
Server는 21:59:45에 새 EXE 링크가 끝났다. 전체 Product 성공이나 배포·실행 중 Server 갱신 완료를 뜻하지 않는다.
MainApp.cpp의 첫 보고 오류는 2228행 fTimeDelta이며 이번 분리 코드와 별도다. 파일은 빌드 중 다시 저장됐고 사용자가 직접 편집 중임을 확인했다.

사용자 마지막 요청에 따라 자기 검사 wrapper PID25284와 그 자식 Server 계약 검사 PID22824만 종료했다.
다른 세션의 프로세스는 종료하지 않았다. [중단 기록](C:/Users/tnest/Desktop/LostArk/out/BossSimulationRefactor20261014/user-paused-validation.json),
[정적 검사](C:/Users/tnest/Desktop/LostArk/out/BossSimulationRefactor20261014/static-verification.json),
[새 EXE·bootstrap hash](C:/Users/tnest/Desktop/LostArk/out/BossSimulationRefactor20261014/resumed-regression-inputs.json)를 보존했다.

이전 제품 EXE의 Kouku product/overlap 검사 통과 기록은 baseline으로만 남기며 새 분리본의 회귀 통과로 사용하지 않는다.
Q/W/E/R/A/S와 빙고 Space는 계속 보류한다. Server·Engine 전체 정비나 패턴 버그 수정 완료를 주장하지 않는다.

## 수동 패턴 구현 자료

- [진짜 세이튼·조커 PLAN](2026-10-14_KOUKU_RANDOM_MANUAL_IMPLEMENTATION_PLAN.md): 정확한 교체 블록과 전체 H/CPP. 제품에는 미적용.
- [발탄 포탈 PLAN](2026-10-14_VALTAN_PORTAL_MANUAL_IMPLEMENTATION_PLAN.md): 기존8구간을4회로 연결하는 완전 JSON, 현재 포탈 방향 실측, 원본 carrier 연결의 미구현 경계.
- [잡기·피자 PLAN](2026-10-14_KOUKU_COLLIDER_MANUAL_IMPLEMENTATION_PLAN.md): 손 추적 후보와 현재 bundle의 수치 재현. 제품에는 미적용.

피자 P26은 현재 WORLD 콜라이더·push가 게시돼 있다. 별도 Room tick fixture에서 bundle5의 510tick에 생존 플레이어 HP10000→9048,
push1.5초와 다음 tick 실제 이동을 관측했다. 처음부터 같은 중심에 있던 시나리오는 앞선 P25 피해로 이미 DEAD였다.
현재 코드에서 마지막 콜라이더가 항상 실패한다는 가설은 배제되지만 사용자의 실제 재현 원인이 확정된 것은 아니다.
잡기는 Client 실제 손 본과 Server 저장 root offset의 불일치를 실제 설치 모델·현재 판정창으로 확인했다. 자세한 후보·한계는 위 PLAN에 둔다.

## 다음 빌드 재개

사용자 편집 정리 후 최신 디스크와 이번12파일/PLAN을 다시 대조하고 정상 Product Debug 빌드를 실행한다.
그 뒤 새 EXE의 관련 계약 검사를 완료한다. 빌드 중에는 제품 EXE 계약 검사를 동시에 새로 시작하지 않는다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product -MaxCompilerProcesses 4 -BuildLogDirectory out/BossSimulationRefactor20261014/build-after-user-edit
& .\Server\Bin\Debug\Server.exe --kouku-product-contract-test
& .\Server\Bin\Debug\Server.exe --kouku-object-overlap-contract-test
& .\Server\Bin\Debug\Server.exe --valtan-lifecycle-contract-test
```
