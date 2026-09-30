# 워터팡 경기 종료와 최초 입장 스폰 복귀 구현 계획

## G00. 현재 서버 상태와 변경 경계

사용자가 승인한 목표는 실제 경기 180초가 끝나면 참가자를 같은 종료 처리에서 최초 마하라카 입장 스폰으로 돌려보내고 다시 워터팡에 입장할 수 있게 하는 것이다. 기존 도입 20초와 실제 경기 180초, 예약 10초는 유지한다. Client/UI 실행과 프로세스 종료, 제품 빌드는 통합 담당이 수행한다. 다른 작업의 미커밋 변경과 데이터는 보존한다.

현재 `Finish_MaharakaWaterpangMatch`는 party02 한 곳을 목적지로 사용하고 종료 순간 아레나에 있는 사람만 선택한다. 물총 skill ID는 0으로 지우지만 cast tick은 남겨 Shared snapshot의 두 필드 동시 0 불변식을 깨뜨린다. `Broadcast_Snapshot`은 직렬화 실패 시 전체 방 송신을 중단하므로 Client가 마지막 위치에서 멈춘다. 공용 디버그 순간이동 reset과 Shared의 엄격한 packet validator는 유지한다.

## G01. ServerPlayer.h의 경기 참가 수명

`Server/Public/ServerPlayer.h`의 워터팡 낙사 상태 바로 앞에 `bWaterpangParticipant`를 추가한다. 이 값은 현재 방의 경기에서 아레나에 들어간 인간을 기억한다. 아레나 footprint, 발사/낙사, 아레나행 trigger 이동을 `Update_MaharakaWaterpangMatch`에서 확인한다. 밖으로 떨어져 점프대에 돌아왔거나 종료 시 죽은 참가자도 종료 복귀 대상이며, 아레나에 들어오지 않은 섬 방문자는 유지한다. 종료 commit에서 false가 되고 방 퇴장 시 player와 함께 폐기된다. 네트워크 필드나 protocol 변경은 없다.

## G02. GameRoom_MaharakaAI.cpp의 종료 transaction

`Update_MaharakaWaterpangMatch`는 참가자를 기록한 뒤 종료를 판정한다. `Finish_MaharakaWaterpangMatch`는 각 참가자의 admission 당시 `strSpawnPlacementId`를 조회하고 기존 `Find_GuideLanding`으로 nav 높이와 충돌을 검증한다. 목적지는 섬에 있어야 하고 stage한 다른 목적지와 겹치지 않아야 한다. 한 명이라도 검증에 실패하면 기존 위치, AI, 물총, 경기 예약과 sequence를 보존한다.

목적지 전체와 STOP packet 검증 뒤 한 번의 방 tick에서 commit한다. AI/물총을 지우고 기존 `Reset_PlayerForDebugTeleport`를 호출한 다음 워터팡 전용 cast ID/tick, cast 종료, 속도 버프, cannon hit tick, 발사/낙사, ballistic 상태와 물총 네 스킬 cooldown을 정리한다. 죽은 참가자도 살아 있는 상태로 복귀한다. position과 yaw는 각 최초 spawn을 기준으로 한다. 경기/DebugEvent/retry와 참가 기억을 지우고 `Reset_SequenceActivation`으로 시작 trigger를 재예약 가능 상태로 만든다. 다른 지역이 쓰는 공용 reset은 변경하지 않는다.

## G03. 기존 Server 계약 테스트

`Server/Private/ServerGameplayContractTests_MaharakaAI.cpp`에 실제 물총 사용 뒤 종료, 이미 밖으로 떨어진 참가자, HP 0, 발사/낙사 잔류 상태, 최초 admission spawn별 복귀를 추가한다. 잘못된 spawn 하나는 모든 commit을 보존해야 한다. 종료 직후 실제 `Broadcast_Snapshot` packet을 decode하여 cast ID/tick이 함께 0이고 방 전체 snapshot이 전송됨을 확인한다. 이후 이동 command가 허용되고 시작 trigger 진입으로 새 예약이 생성되며 다음 경기 AI가 다시 생성되는지 확인한다.

`Server/Private/Main.cpp`의 기존 contract-test 분기에 `--maharaka-ai-contract-test`를 추가해 선언만 있던 `Run_MaharakaAI`를 실제로 실행한다. 기존 파일만 수정하므로 Server.vcxproj와 filters 신규 등록은 필요 없다. UTF-8 및 기존 줄바꿈을 유지한다.

## G04. 검증과 완료 판정

통합 담당이 Server 증분 빌드 후 `Server.exe --maharaka-ai-contract-test`를 실행한다. 정상 종료 직후 snapshot과 이동, 두 번째 경기 진입, 실패 시 원상 보존을 계약 테스트로 판정한다. `git diff --check`를 수행한다. Client 화면의 3분 종료와 실제 재진입은 사용자가 최종 확인하며 코드 적용, 빌드, 자동 테스트, 화면 확인은 RESULT에서 구분한다.
