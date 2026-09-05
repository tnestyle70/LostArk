# 2026-09-05 아레나 카메라와 피킹 위치 플레이어 배치 구현 계획

최초 기준은 PR #316 충돌 해결 commit `d3d9a21e`이며 작업 브랜치는 `codex/arena-camera-player-placement`다.
해당 PR이 병합된 `origin/main`의 `96f907b2`는 같은 tree이므로 최종 커밋은 이 main 위에 정리한다.
원본 작업 폴더의 다른 작업과 실행 중인 Client/Server는 보존한다.

## G00. 아레나별 자유 카메라 속도

발탄은 camera span 180에서 `max(20, span * 0.08)`을 적용해 기본 20m/s다.
쿠크는 같은 식에 거대한 맵 범위를 넣어 훨씬 빨라진다. 두 아레나의 기본값을 20m/s로 맞춘다.

`CCamera_Free`는 유한한 양수 속도를 검증하고 기존 Transform 이동량에 비율로 적용한다.
각 Level은 현재 아레나 카메라와 process-session 속도를 소유한다. F1의 현재 아레나 항목에서
즉시 조절·기본값 복원을 제공하고 F6 전환 및 같은 process의 재진입에서 선택을 유지한다.
Shift의 기존 30배 이동은 표시한다. 이 요청에는 디스크 저장을 추가하지 않는다.

변경 위치는 Camera_Free H/CPP와 두 Arena Level의 camera 생성·접근 함수, MainApp F1 표시다.
Engine Transform과 컷신 카메라 시계는 변경하지 않는다.

## G01. F6 상태에서 명시적으로 지면을 선택해 플레이어 배치

F1 `Move Player`를 누르면 마우스 회전을 끄고 지면 선택을 한 번 대기한다. UI 밖의 새 좌클릭만
기존 world-position picking으로 읽는다. Esc/우클릭/F6 follow 복귀는 아직 제출하지 않은 선택을 취소한다.
일반 좌클릭·gameplay 이동·스킬과 이 Debug 명령은 분리한다.

호출 흐름은 `MainApp F1 -> CPlayerController -> IPlayerCommandSink -> Shared -> Server GameRoom`이다.
Controller가 좌표 의도를 제출하고 Server가 현재 session/world, 순서, 생존 상태, 유한 좌표,
navigation 높이·walkability와 collision을 검증한다. 정상 위치만 기존 teleport reset 경로로 commit하고
기존 snapshot으로 표시한다. 거절은 이전 player/action을 보존하고 typed result 이유를 돌려준다.
Client는 send 성공을 이동 완료로 표시하거나 Character Transform을 직접 바꾸지 않는다.

Shared request/result와 protocol version, ServerApp/RoomCommand/GameRoom 소비자, NetworkManager 및
NetworkPlayerCommandSink의 결과 큐, PlayerController의 한 번 선택·응답 상태를 같은 변경에 연결한다.
Release Server는 명시적으로 거절한다. 적어도 발탄·쿠크 Arena에서 실제 F1 consumer를 제공한다.

## 관련 변경 단위

함께 요청한 Stage 자동 접기는 별도 커밋으로 묶고
[Composition Resources Stage 구현 계획](2026-09-05_COMPOSITION_RESOURCE_STAGE_ACCORDION_IMPLEMENTATION_PLAN.md)을 따른다.

## 검증과 완료 경계

- 기존 파일의 인코딩과 필요한 project/filter 등록을 확인한다. 새 C++ 파일과 새 JSON은 예정하지 않는다.
- 변경한 Client/Shared/Server의 최소 컴파일과 해당 protocol·Server 검사를 수행한다.
- 입력의 UI 차단·한 번 제출·취소, stale result, Server의 실패 시 기존 상태 보존을 확인한다.
- `git diff --check`와 변경 XML/JSON이 있을 경우 parse를 수행한다.
- 실제 F1 속도·F6 피킹·Stage 펼침은 사용자가 새 Server/Client에서 직접 확인한다.
  에이전트는 Client/UI 실행, 캡처 또는 visual PASS 판정을 하지 않는다.
