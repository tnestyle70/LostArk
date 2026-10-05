# 베른 플레이어 시작 위치 변경

## G00. 범위

사용자가 요청한 `(137, ,42, -13.4)`를 XYZ `(137, 42, -13.4)`m로 반영한다.
현재 정본 `Data/Worlds/LV_BER_BERNCASTLE/Gameplay.world.json` revision 929의
첫 입장 슬롯 `player_1` 위치는 `(137.586334, 42.2498169, -22.4640217)`이다.
이 행의 position과 문서 revision만 변경하고, 다른 입장 슬롯과 NPC·트리거는 보존한다.

## G01. 데이터와 소비 경로

정본의 `player_1` position을 `[137, 42, -13.4]`, revision을 930으로 저장한다.
`Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Publish -WorldId BERN`으로
Server worldbootstrap과 Client World viewer를 생성한다. 생성물을 직접 편집하지 않는다.
Server가 슬롯을 선택하고 기존 navigation projection을 거쳐 실제 위치를 확정한다.
실행 중 Server의 메모리를 강제로 갱신하거나 Client Transform을 변경하지 않는다.

## G02. 검증과 적용

JSON parse, 해당 World publisher Validate/Publish, 정본·게시 좌표 일치,
navigation의 보행 가능 셀·높이, 다른 placement 보존 및 `git diff --check`를 확인한다.
데이터만 변경하므로 C++·shader 빌드는 필요하지 않다.
Server 재시작 후 베른 첫 입장 확인은 사용자가 수행한다.
