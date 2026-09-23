# 발탄 강제 이동과 지면 지지 분리 결과

## G00. 이번 완료 기준

사용자의 마지막 요청은 우선 현재 수정분을 컴파일 오류 없이 마무리하는 것이다. 기능 확장을 중단하고 Release Product 컴파일·링크와 이미 추가한 focused 검사 결과를 기록한다. 이 문서의 빌드 성공은 모든 끼임 문제의 해결이나 실제 4인 화면 검증을 뜻하지 않는다. 기존 다른 작업의 미커밋 변경을 보존했으며 자동 stage/commit은 하지 않았다.

## G01. 확인한 원인

- `156634cde`(8월 19일)는 기존 스킬 root-motion용 `Clamp_StepToWalkable`을 일반 피격 넉백에 재사용했다. 당시 사용자 결정과 별도로 구현 과정에 들어간 제한이다.
- `feae67183`(8월 28일)은 뒤잡기 `ARENA_EJECTION`만 walkable clamp에서 제외했다. 그래서 뒤잡기는 날아가지만 일반 피격은 붕괴 경계를 넘지 못했다.
- 기존 발탄 `.navpolicy`의 최대 단차 0은 무제한이다. 실제 인접 셀 `(142.25,23.1116,-114.25)`과 `(141.75,10.8561,-114.25)`의 높이 차이는 약12.26m다. 낮은 셀은 void가 아니므로 보행 경로에서 허용되면 보스 Y가 아래 지형으로 이동할 수 있었다. 이는 코드·데이터상 재현 가능한 경로이며, 사용자가 본 특정 occurrence의 실행 로그를 확인한 것은 아니다.
- 버러지 일부 포획은 일반 피해, 생존 플레이어 전원 포획은 처형이라는 기존 데이터/서버 분기다. 이번 이동 변경에서는 그 계약을 바꾸지 않았다. 전원 포획인데도 생존했다면 별도 실행 재현이 필요하다.

## G02. 반영한 코드·데이터

- `GameRoom_PlayerSimulation.cpp`: 발탄 일반 피격만 별도 처리한다. 보행 경로 clamp 대신 Server 벽/몸체 직선 sweep 뒤 `Sample_SurfacePosition`으로 지면을 검사한다. void/지지면 부재/큰 하강은 기존 `Begin_PlayerFall`, 큰 상승은 마지막 지지점에서 중단한다. 뒤잡기와 쿠크의 기존 분기는 유지한다.
- `ValtanBrain.cpp`: 추적 이동은 캐시된 먼 경로점의 Y를 직접 대입하지 않고 실제 이동 구간의 지면 높이를 검사한다. 붕괴 지점의 복구는 같은 높이의 지면으로 연속 이동하도록 연결했다.
- `Publish-ServerNavigation.ps1`: 발탄 최대 보행 단차를1m로 게시한다. 후보와 실제 publish 모두 실행했다. 격자·surface·blocker 파일은 후보 대조에서 기존과 byte 동일했고, Server/Client의 발탄 `.navpolicy`만0→1로 바뀌었다.
- 기존 Server 계약 테스트 파일에 `--valtan-arena-support-contract-test`를 추가했다. 새 C++ 파일은 없다.

## G03. 검증 증거

1. `Publish-ServerNavigation.ps1 -Mode Publish -AreaId LV_LUT_HEARTRB_ED`: 성공. 392×312, cellSize0.5, walkable21524. 출력의 `maxStep=19.892...`는 원본 격자의 최대 인접 높이차이며, 런타임 허용값은 두 `.navpolicy`의1이다.
2. `Invoke-BuildAndRegression.ps1 -Configuration Release`: 최종 Product 빌드 exit0, Engine/Shared/Server/Client 모두 컴파일·링크 PASS, 91.103초. 최초 실패의 원인은 `NetworkManager.cpp` 공통 호출부가 Debug 전용 `HashBytesSha256`을 참조하는 C3861 두 건이었다. 기존 다른 작업의 함수 본문을 보존하고 전처리 경계 세 줄만 추가해 Release에서도 기존 함수를 사용할 수 있게 했다. 최종 receipt는 `out/BuildPipeline/runs/20260923T041302260Z-release-product.json`, 전체 로그는 `out/ValtanForcedMovement20260923/release-fixed-logs/console.log`다. 기존 인코딩·누락 PDB 경고는 남아 있으며 컴파일·링크 오류는 없다.
3. `Server/Bin/Release/Server.exe --valtan-arena-support-contract-test`: exit0, 12 PASS, failures0. 실제 게시 지형의 붕괴 경계·4인 중 피격자만 낙사 사망·12m 하강의 높이 보존·상승 차단·물리 지지면·벽 앞 정지·뒤잡기 발사를 검사했다. 로그는 `out/ValtanForcedMovement20260923/valtan-arena-support-release.log`다.
4. `Server/Bin/Release/Server.exe --navigation-contract-test`: exit0, failures0. 기존 navigation 회귀 검사다. 로그는 `out/ValtanForcedMovement20260923/navigation-release.log`다. 새 보스 recovery/패턴 이동의 자동 검사로 대신 기록하지 않는다.
5. `git diff --check`: 통과. 기존 다른 파일의 LF/CRLF 변환 경고만 출력했다.

## G04. 미완료 경계

- 물리 지지면은 있지만 보행 불가인 셀에 넉백이 끝나면 이후 보행 시작점 검사가 실패할 수 있다. 실제 셀 `(143.75,22.565498,-110.75)`는 surface=1, walkable=0이다. 강제 착지 이후의 정상 이동 복귀를 추가로 연결해야 한다.
- 셀 절반 간격 표본은 셀 모서리를 짧게 관통하는 대각선 구간을 놓칠 수 있다. 셀 교차 전수 추적과 모서리 회귀 검사가 남아 있다.
- 보스 scripted stride가 사용하는 기존 `Resolve_NavigableStep`은 높이 제한을 검사하지 않는다. 이번 추적 경로 보완을 모든 패턴 이동의 높이 검증으로 확대해서 설명하면 안 된다.
- 기존 광역 `ServerGameplayContractTests_ValtanPinnedGeneration.cpp`에는 일반 넉백이 navigation에서 멈춘다는 이전 기대값이 남아 있다. 새 물리 지지/벽/void 계약에 맞춘 갱신이 필요하다. 광역 suite 성공을 주장하지 않는다.
- 보스 복구의 실제 패턴 동작, Release 서버+클라이언트4인 전투, 사용자 화면 확인은 미실시다. Client/UI는 실행하지 않았다.

다음 구현을 이어갈 때에는 이 경계부터 완료하고 새 focused 검사를 확장한다. 현재 단계의 빌드가 통과해도 끼임 전면 해결로 보고하지 않는다.
