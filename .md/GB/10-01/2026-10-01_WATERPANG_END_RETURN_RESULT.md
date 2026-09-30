# 워터팡 경기 종료와 최초 입장 스폰 복귀 결과

## G00. 구현 범위

기존 종료 처리가 물총 skill ID만 지우고 cast tick을 남겨 Shared snapshot 검증을 실패시키던 원인을 수정했다. packet validator나 Client 위치 권위는 완화하지 않았다.

## G01. 서버 종료와 재입장

- `Server/Public/ServerPlayer.h`: 현재 경기의 인간 참가 여부를 기억한다. 아레나를 벗어나 낙사 복귀 중이거나 죽은 참가자도 종료 대상에 포함한다. 일반 섬 방문자는 이동시키지 않는다. 참가 여부는 종료 commit에서 지우고 퇴장 시 player와 함께 폐기한다. AI tuning으로 AI만 줄일 때 인간의 참가 기록은 유지한다.
- `Server/Private/GameRoom_MaharakaAI.cpp`: 참가자별 `strSpawnPlacementId`를 사용해 최초 admission 스폰의 nav/collision 목적지를 모두 검증한다. 하나라도 실패하면 위치, 경기 예약, AI와 물총을 유지한다. 성공 시 같은 tick에서 AI/물총/도입 예약을 정리하고 참가자의 물총 ID/tick, cast 종료, 속도 버프, 물총 cooldown, 발사/낙사/ballistic 상태를 초기화하여 살아 있는 상태로 복귀시킨다. 기존 시작 sequence trigger를 다시 활성화할 수 있게 한다.
- 공용 `Reset_PlayerForDebugTeleport`, Shared packet 구조/엄격한 validator, Client 코드는 변경하지 않았다.

## G02. 실제 실행 가능한 회귀 테스트

`Server/Private/Main.cpp`에 `--maharaka-ai-contract-test`를 연결했다. 기존 `Run_MaharakaAI`는 선언과 정의만 있고 호출 경로가 없었다.

`Server/Private/ServerGameplayContractTests_MaharakaAI.cpp`는 기존 실제 방 fixture에서 인간 물총 사용, 이미 밖으로 떨어진 참가자, HP 0, 발사/낙사 잔류 상태를 만든다. 잘못된 목적지 하나가 전체 commit을 막는지, 정상 expiry room Tick이 각 최초 스폰으로 복귀시키는지, 네 세션 모두 실제 writer/reader를 통과하는 snapshot을 받는지 검사한다. 복귀 후 이동과 실제 jump3 G 입장을 거쳐 다음 countdown/AI roster가 생성되는지 확인한다. 신규 C++ 파일은 없으며 기존 project/filter 등록을 유지한다.

## G03. 검증 상태

실행 완료:

- 변경한 네 C++ 파일의 `git diff --check` 성공. 원래 LF인 테스트 파일은 Git의 향후 CRLF 변환 경고만 있다.
- 네 파일 UTF-8 인코딩 확인. 원래 CRLF인 세 파일과 원래 LF인 테스트의 줄바꿈 유지.
- 독립 에이전트의 읽기 전용 diff 검토에서 추가 blocking 결함 없음. 참가 수명, 목적지 전체 검증 뒤 commit, cast pair 초기화 및 실제 snapshot 회귀 연결 확인.

통합 담당으로부터 확인한 검증:

- Server 전체 Release `ClCompile` 성공(exit0). `ServerPlayer.h`의 상태 배치를 모든 참조
  translation unit에 반영했으며 `out/WaterpangEndInspection20261001/server-compile-all.log`를 보존했다.
- Product runner는 실행 중인 Release Client PID23052와 Server PID32460 점유로 차단됐다.
  사용자가 현재 실행 유지를 선택하여 두 제품 프로세스와 제품 EXE를 유지했다.
  기존 OBJ를 읽고 모든 출력을 out에 격리한 검증 EXE로 아래 계약을 확인하며, 제품 링크는 보류했다.

- Release `SelectedFiles ClCompile`로 `GameRoom_MaharakaAI.cpp`, `Main.cpp`, `ServerGameplayContractTests_MaharakaAI.cpp` 실제 OBJ 생성 성공, 경고 0/오류 0. 로그는 `out/WaterpangEndInspection20261001/compile-maharaka.log`, `compile-Main.log`, `compile-ServerGameplayContractTests_MaharakaAI.log`다.

격리 실행 검증:

- 전체 Server OBJ를 최신 상태로 컴파일한 뒤 검증 EXE·PDB·LTCG·IMPLIB만
  `out/WaterpangEndInspection20261001/isolated-return`에 링크했다. 실제 제품의
  IntDir/OutDir 설정, Server.exe, link command tracking, 기존 LTCG 입력은 변경하지 않았다.
- `LOSTARK_SERVER_DATA_ROOT`로 게시 데이터를 읽고 `Server.exe --maharaka-ai-contract-test`
  최종 exit0 PASS. `out/WaterpangEndInspection20261001/maharaka-final.log`에 실제 종료 tick,
  네 session의 snapshot writer/reader, 참가자3명 모두의 C2S_MOVE와 다음 tick 이동,
  실제 jump3 G 재입장 및 다음 intro/AI20 생성 성공을 보존했다. listener나 live Server 연결은 없다.
- 초기 검사에서 party03이 최초 admission 위치와 약0.779m 달라 실패했다. 실제 authored marker는
  navigation 허용·collision 거부였고, 안전 착지는 marker에서0.750013m 보정된 위치에서 둘 다
  허용됐다. 제품의 충돌 검사를 약화하지 않고 테스트를 자기 spawn3m 범위·nav 높이·실제 충돌·
  보정이 필요한 원인 및 전원 이동까지 검사하도록 수정한 뒤 위 성공을 확인했다.
- 원본 스냅샷 실패 재현은 `repro.log`의 `before=1 ended_with_stale_cast_tick=0 both_fields_cleared=1`로
  별도 보존했다. Shared validator를 완화하거나 데이터를 지우지 않았다.

남은 적용·확인:

- 위 격리 검사 당시에는 사용자가 실행 유지를 선택하여 제품 EXE 링크/교체를 보류했다.
  현재 Server 제품 링크의 후속 완료는 G04를 따르며, 기존 실행 프로세스 자동 갱신을 의미하지 않는다.
- 실제3분 경기 화면의 동시 복귀와 재입장은 사용자 확인 대상이다. Client/UI 실행,
  화면 캡처, 실행 중 제품 프로세스 종료를 수행하지 않았다.

## G04. 후속 Server 제품 링크와 종료·재입장 계약 완료

2026-10-01 정상 Server [Debug 빌드](../../../out/GuidePersonal20261001/server-guide-final-debug-build.log)와
[Release 빌드](../../../out/GuidePersonal20261001/server-guide-final-release-build.log)가 실제 제품 EXE
링크까지 성공했다(각 경고 0/오류 0). G03의 Server 링크 보류는 이 후속 결과로 해소됐다.
최신 Release `--maharaka-ai-contract-test`는 **199 PASS, 0 FAIL, exit 0**이다.

[실행 로그](../../../out/GuidePersonal20261001/maharaka-ai-release-complete.log)는 실제 expiry tick,
참가자별 최초 admission 근처의 nav/collision 안전 복귀, 모든 cast/motion 잔류 상태 정리,
네 session의 정상 snapshot, 귀환 뒤 이동과 실제 G 재입장·다음 AI roster 생성을 확인한다.
[종료값 JSON](../../../out/GuidePersonal20261001/contracts-release-complete.json)에 성공을 기록했다.
최신 Client 전체 Product 빌드는 아직 대기 중이며 실제 경기·화면 확인은 사용자에게 남아 있다.

## G05. 최종 종료 복귀 제품 빌드

이동 보완과 모든 최신 Client/Server 소스를 포함한 정상 Product Debug/Release가 모두 성공했다.
앞선 EXE 잠금·최종 링크/제품 빌드 대기는 해소됐으며 실행 파일과 실제 로그는
`../09-27/2026-09-27_GUIDE_AI_TOOL_IMPLEMENTATION_RESULT.md`의 G09에 기록했다.
이 결과는 기존 기능별 검증을 대체하거나 실제 Client 화면·다인 플레이·성능 확인으로 확대하지 않는다.
