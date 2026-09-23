# 쿠크 관문 BGM 이동·시퀀스 재개 결과

## G00. 실제 변경

`Client/Private/Level_KakulSaydonArena.cpp`만 수정했다. 현재 UTF-8 BOM/CRLF를 유지했다. `jump.2`의 Y=3.56 도착점이 시작 구역 최저 Y=5.64 아래이므로 기존 선택기가 음악을 지우던 것을 확인했다. 전투 전 준비·이동은 시작 BGM을 유지한다. COMBAT/WAIT_MINIGAME/WAIT_GATE는 현재 관문을 따르며 G1/G2/BINGO는 `midnightc_ed__398225682.wav`, G3는 `midnightc_ed__1053752270.wav`를 선택한다. G3 진입 전 발판은 시작 곡이 우선한다. 사용자가 적은 398225682 파일명에 해당하는 실제 설치 stable asset ID를 사용했고 파일을 복제/개명하지 않았다.

시퀀스의 기존 억제 조건에 실제 camera override를 소유한 Composition cinematic track을 추가했다. follow/static camera는 음악을 끄지 않는다. 시퀀스가 소유권을 반납하면 같은 기존 edge 처리로 음악이 재개된다. Mario/Maze 우선순위, 완료·중단·local player 없음, 누락 음원당 한 번의 시도와 transactional Music 교체는 유지한다. raid phase가 INACTIVE인 Release 직접 관문 입장/F1 승인도 이미 commit된 gate의 stable placement ID로 곡을 선택한다. COMPLETE/ABORTED의 중지는 유지한다. 새 H/public/Shared/Engine 계약·authoring 데이터·프로젝트 항목은 없다.

## G01. 자동 검증

실제 작업 소스의 선택기와 `Update_RaidBgm` 본문을 추출한 native C++ probe 46 checks, 0 failures. 실제 Shared ready-area 함수를 사용하여 jump.1/2/3 도착 좌표, G3 발판/전투 바닥, 관문·기믹 전환, Sequence/Composition camera 억제·재개, 같은 snapshot 반복과 실패 retry 억제를 확인했다. 나머지 Client API는 stub이며 Client 실행 검증은 아니다. `out/KoukuCollider20260923/bugfix-bgm/bgm_probe.cpp`와 `build_bgm_probe.cmd`가 증거다.

설치 두 WAV의 RIFF/fmt/data, 양수 길이와 SHA256을 `assets-validation.json`에 기록했다. 실제 오디오는 재생하지 않았다. 변경 TU는 통합 담당의 Client Debug/Release 빌드 대상이다. live authoring JSON 변경과 후보 교체는 필요 없다. 화면·실청과 최종 설치 빌드는 통합 결과와 구분한다.
