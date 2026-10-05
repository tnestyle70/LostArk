# 베른 플레이어 시작 위치 변경 결과

## G00. 반영 완료

`Data/Worlds/LV_BER_BERNCASTLE/Gameplay.world.json`의 `player_1` position을
`[137, 42, -13.4]`m로 변경하고 revision을 929에서 930으로 올렸다.
다른 68개 placement와 player_1의 나머지 필드는 그대로다.

BERN 전용 World publisher가 다음 게시본을 생성했다.

- `Server/Bin/DataFiles/World/BERN.worldbootstrap`: revision과 player_1 좌표 변경.
- `Client/Bin/DataFiles/World/LV_BER_BERNCASTLE.viewer.world.json`: 같은 변경.
- `BERN.npcpresentation.json`: 게시 후 기존과 동일하여 Git 변경 없음.

## G01. 검증

- 정본과 게시 viewer JSON parse, 좌표 일치: PASS.
- 수정 전후 JSON 객체 비교: player_1 position과 revision만 변경, PASS.
- `Publish-WorldGameplay.ps1 -Mode Validate -WorldId BERN`: 69 placements, PASS.
- `Publish-WorldGameplay.ps1 -Mode Publish -WorldId BERN`: PASS.
- `git diff --check`: PASS. 다른 작업 문서의 기존 LF/CRLF 경고는 별도다.
- C++·HLSL 변경이 없어 빌드는 수행하지 않았다.

설치된 `LV_BER_BERNCASTLE.Bern.navgrid`는 570×560, 셀 0.5m다.
요청 XZ가 속한 (244,463) 셀과 주변 3×3은 모두 walkable이며,
바닥 높이는 42.276634~42.276642m다. Bern region에만 속하고
해당 navblockers는 영역 0개다. navigation 재생성은 필요하지 않다.

## G02. 실제 적용 경계

`GameRoom_Admission.cpp`의 기본 입장 경로는 높이 미지정으로 Server navigation에
투영하므로 최종 위치는 셀 중심·바닥으로 보정된다. 현재 게시 격자에서 예상 좌표는
`(137.25, 42.2766418457, -13.25)`m이며 요청 XZ와의 차이는 약 0.292m다.
이는 실제 Client 실행 관찰이 아니라 기존 소비 규칙과 설치 격자에 근거한 값이다.

실행 중 Server/Client를 종료하거나 조작하지 않았다. Server 재시작 후 베른 첫 입장으로
사용자가 적용을 확인한다. 이미 접속한 캐릭터나 다른 입장 슬롯을 이동시키지 않는다.
백업은 Git 제외 `out/BernPlayerEntry20261004/`에 보존했다.
