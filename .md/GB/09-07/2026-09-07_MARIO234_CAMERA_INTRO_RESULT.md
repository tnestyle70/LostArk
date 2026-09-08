# 2·3·4마리오 등장 카메라 / 구간 추적 결과

## 후속 반영: 사용자 조정 마지막 키 → 컷신 종료·추적 포즈

사용자가 13:32/13:34 화면에서 조정하고 저장한 revision 67의 마지막 키를 기준으로
2·3마리오 추적 설정을 revision 68로 갱신하고 Map publisher로 Client runtime에 배포했다.

- 2Mario 마지막 Eye: `[-1441.39, -8.02, -1172.34]`.
- 3Mario 마지막 Eye: `[-1902.27, -9.66594, -1640.5]`.
- 저장된 마지막 Look At / FOV도 그대로 사용했다. 화면 반올림 값으로 원본 키를 덮어쓰지 않았다.
- 상위 Eye / Look At / FOV가 이전 먼 포즈를 유지하던 부분을 마지막 키와 일치시켰다.
- 2마리오 a~c, 3마리오 a~f를 마지막 키 - 실제 go 도착점 offset으로 재계산했다.
  각 층의 기존 상대 yaw 차이는 유지했다. 정적 Preview용 Eye/Look도 새 offset에 맞췄다.
- 모든 cameraTrack 키와 box, 1·4마리오 등 다른 13개 샷은 변경하지 않았다.

검증: Map Validate/Publish PASS, JSON parse PASS, 기대 변경 전체 비교 PASS,
Authoring/Runtime 의미 값 일치, 첫 follow와 마지막 키 Eye/Look 오차는 2마리오 약 1e-8m,
3마리오 약 3e-7m다. git diff --check PASS. C++/Server 변경이 없어 이번에는 재컴파일하지 않았다.
아래 Product 빌드 기록은 최초 구현 시점 기록이다. 후속 화면 확인은 사용자에게 남겼다.

현재 열려 있는 MapTool에서는 **Reload Shots를 먼저 눌러** 갱신된 추적값을 읽는다.
이전 메모리 상태로 Save Shots를 누르면 이전 offset을 다시 저장할 수 있다.
제품 확인은 Client 재시작 후 F1 → 2마리오/3마리오. 이번 변경만으로 Server 재시작은 필요 없다.
추가로 마지막 키를 다시 편집했을 때 자동으로 follow 값까지 저장하는 기능을 새로 추가한 것은 아니며,
이번에 사용자가 저장한 최종 키를 기준으로 맞춘 데이터 보정이다.

## 구현 상태

실제 작업 경로: `C:/Users/USER/source/졸업팀폴/LostArk`.
작업 브랜치: `codex/mario234-camera-intros`, 시작 HEAD `0f05b7c5`.
기존 Claude 작업과 1마리오 사용자 튜닝을 보존했다. 자동 stage/commit/push는 하지 않았다.

| 구간 | Camera 목록 | 등장 시간 / 키 | 자동 추적 목록 | F1 빠른 입장 |
|---|---|---|---|---|
| 2마리오 | 2Mario | 3초 / 37개 | shot.mario2.a~c | 활성화 |
| 3마리오 | 3Mario | 3초 / 37개 | shot.mario3.a~f | 활성화 |
| 4마리오 | 4Mario | 3초 / 33개 | shot.mario4.a~f | 활성화 |

기존 넓은 단일 `shot.mario2`만 세 구간으로 교체했다.
`1Mario`, `shot.mario1.a/b/c`, `Mario1_Intro`, 기존 소품/이동 트리거와 시퀀스는 변경하지 않았다.
다른 기존 카메라·시퀀스·트리거 200개를 시작 시점 JSON object hash와 대조하여 모두 동일함을 확인했다.
MapTool.cpp의 기존 대규모 미커밋 diff는 이번 변경이 아니다. 이번 MapTool 수정은 9줄이다.

### 연결 방식

1. F1 gate는 기존 `MarioN_go` 목적지를 사용해 typed command → Server navigation 승인으로 이동한다.
2. 새 `Mario2_Intro` / `Mario3_Intro` / `Mario4_Intro` 영역 진입이 Server `playSequence`를 발행한다.
3. `world.sequence.instance.mario_mN_intro`의 3초 시계를 해당 `<N>Mario` 카메라 트랙이 사용한다.
   실제 ASCII Shot ID는 `2Mario`, `3Mario`, `4Mario`다.
4. 마지막 카메라 키와 첫 follow 포즈를 일치시켰다. 출발 후 층·갈림길 박스가 해당 follow 샷을 선택한다.
5. 새 intro trigger는 `triggerOnce=false`다. F1 재이동은 서버의 기존 trigger membership 초기화를 사용한다.

시계는 기존 1마리오와 같은 항등 map-placement 트랙이며 기존 오브젝트를 움직이거나 새 모델을 생성하지 않는다.
다른 sequence 및 mapmotions가 사용하지 않는 표시 중인 배치만 연결했다.

| 인트로 | 시계 대상 placement ID |
|---|---|
| mario_m2_intro | 11284422400836074445 |
| mario_m3_intro | 13607882261321843072 |
| mario_m4_intro | 17873503318011911402 |

MapTool의 명시적 `Play mN` 동안은 해당 intro만 카메라를 소유하도록 했다.
이전에 중간 중지한 컷신 시계가 남아 있어도 다른 intro의 우선순위 때문에 엉뚱한 샷이 선택되지 않는다.
Play 성공 시 Camera 목록도 해당 샷을 선택한다. 기존 Hold / 키 반복용 시계 보존은 유지했다.

## 원본 확인과 저작 보정

첨부 영상 세 개와 `마리오.txt`, 원본 actor/Matinee 자료를 대조했다.
원본 내부 번호는 사용자 번호와 다르다. 사용자 3마리오는 원본 마리오4 조명/Matinee9,
사용자 4마리오는 원본 마리오3 카드/Matinee20이다. 기존 소품 시퀀스 이름은 바꾸지 않았다.

- disabled 트랙을 제거하고 활성 부모/자식 카메라와 3마리오 `cam_a`까지 합성했다.
- 원본 Hermite 곡선을 원본 키 시점 + 100ms 간격으로 샘플링해 기존 LINEAR 저장 계약에 넣었다.
- 원본 UPK camera volume의 8정점을 읽어 레일 대응을 확인했다.
- 원본 박스 그대로는 현재 프로젝트의 점프 중간 궤적을 덮지 못한다. 제품 데이터는 층/갈림길을 구분하면서
  점프도 포함하는 넓은 저작 박스를 사용한다. 원본 OBB를 그대로 복제한 데이터가 아니다.
- 3마리오 두 타워의 경계 X를 -1904로 두었다. 타워 이동 중 `c → e → d`로 잠깐 중층 카메라를
  거치는 대신 `c → d`로 바로 넘어가도록 수치 검사했다.

새 Shot/Template/Instance/Trigger 전체 값과 원본 선택 근거는 대응 PLAN의 G3에 기록했다.
보조 조사 산출물은 `out/Mario234/source_intro_keys.json`, `source_rails.json`이며 제품은 이를 읽지 않는다.
새 Resources 입력, 쿠킹, vcxproj/filters 등록, Shared 패킷 변경은 없다.

## 자동 검증

| 항목 | 결과 |
|---|---|
| Debug Product Engine / Shared / Server / Client | 컴파일·링크·정상 배포 PASS |
| Map publisher Validate / Publish | PASS, placements 3231 |
| World publisher Validate / Publish | PASS, Kakul placements 67 |
| JSON parse / ID 중복 / sequence 참조 / 키 순서·유한값 | PASS |
| 카메라 키 최대 64개 제한 / 3초 길이 일치 | PASS |
| 마지막 키 ↔ 첫 follow Eye/Look 연속성 | 오차 0.00001m 미만 |
| 기존 JSON 항목 보존 | 200개 동일 |
| Authoring ↔ Client runtime 카메라/시퀀스 | 의미 값 동일 |
| 원본 레일 15개 × 101점 | 잘못된 구간 선택·누락·중복 0 |
| 기존 점프 12개 × 1001점 + held margin 0.5m | 카메라 없음 0 |
| git diff --check | PASS, LF/CRLF 경고만 있음 |

입장점은 Server 실제 상세 navgrid에서 모두 walkable=1이고 published Client/Server grid가 동일했다.

| 구간 | 입장 좌표 | nav cell | 바닥 높이 오차 |
|---|---|---|---|
| 2 | (-1434.48999, -9.02000999, -1175.96997) | (137,164) | 0.000425m |
| 3 | (-1889.68994, -11.5299997, -1646.20996) | (175,179) | 0.002977m |
| 4 | (-1632.57, -20.49, -1400.92) | (166,127) | 0.000437m |

실행한 명령:

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Mode Publish
powershell -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Publish
git diff --check
```

빌드 기록: `out/BuildPipeline/runs/20260907T041422745Z-debug-product.json`.
최초 sandbox 빌드는 Windows SDK 경로 접근 거부로 중단되었고, 권한 승인 후 위 Product 빌드가 성공했다.
기존 C4819/FXC/DirectXTK PDB 경고는 남아 있다. 광역 regression/harness는 실행하지 않았다.
저장 데이터 수치 검사는 `out/Mario234/check_saved_data.mjs`로 재확인했다.

## 사용자가 확인할 경로

### 런타임에서 바로 입장

1. 공유 Server PC에 새 `Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json`을 반영하고
   World publisher 실행 후 Server를 재시작한다. 해당 Server가 새 intro trigger를 읽어야 한다.
2. 이 PC의 기존 Client를 종료한 뒤 x64 Debug의 Client 프로젝트를 Ctrl+F5로 시작한다.
3. Lobby → KoukuSaydon → F1 → KoukuSaydon Arena → `2마리오` / `3마리오` / `4마리오`.
4. F6 자유 카메라 상태면 follow 모드로 돌아온 뒤 확인한다. 자유 카메라는 자동 컷신보다 우선한다.
5. 입장 카메라가 끝난 뒤 이동·점프하며 층과 갈림길 전환을 확인한다. 같은 F1 버튼으로 intro를 다시 확인한다.

### 맵툴에서 카메라만 미리보기·조정

1. Lobby → Test → F1 → Open Map Tool → KoukuSaydon / MidnightC ED.
2. World Sequence → Cutscene Arena Preview → Stage Intro Camera → `Play m2` / `Play m3` / `Play m4`.
   이 미리보기는 플레이어와 Server 없이 기존 MapTool 시계를 사용한다.
3. Camera → Camera Shots에서 `2Mario` / `3Mario` / `4Mario`를 펼쳐 키를 선택한다.
   Eye / Look At / Fov Y와 Hold / Jump To This Key / Play This Key / Set From Free Camera 등 기존 기능을 사용한다.
4. 플레이 중 자동 보정은 `shot.mario2.a~c`, `shot.mario3.a~f`, `shot.mario4.a~f`의
   Box Center / Half Extents와 Follow Eye Offset / Follow Look Offset으로 조정한다.
5. Save Shots는 authoring 저장이다. 제품 런타임 반영에는 위 Map publisher를 다시 실행하고 Client 재진입한다.
   새 값이 기존 실행 중인 Client에 자동 hot reload된다고 간주하지 않는다.

세션 시작 LAN 설정은 role=client, endpoint=192.168.0.4:7777, probe=not-listening이었다.
서버 PC의 데이터 배포·재시작은 이 로컬 작업에서 실행하지 않았다.

## 남은 화면 검증과 범위

Client/UI를 실행·조작·캡처하지 않았다. 사용자가 아직 확인하지 않은 2·3·4마리오 화면을 visual PASS로 표시하지 않는다.
원본 roll, fade, postprocess와 화면비별 FOV 변환은 이번 기존 스키마 확장 범위에 포함하지 않았다.
특히 2마리오 원본 첫 키에는 높은 위치·상향 시선이 있다. 단위/행렬 계산 오류로 재현되지 않았지만,
원작의 첫 키 이전 처리·fade를 포함한 첨부 영상과 동일한 첫 화면인지 사용자가 확인해야 한다.
이 부분도 저장된 키 목록에서 수정할 수 있다.

기존 sequence event는 방 전체 broadcast다. 여러 플레이어가 서로 다른 intro를 동시에 시험할 때의
개인 카메라 격리 계약은 새로 구현하지 않았다. 이번 작업은 입장 카메라·자동 추적·편집·빠른 확인이며,
제한 시간/피해/클리어 판정 등 마리오 게임 규칙을 새로 완성했다고 보고하지 않는다.

## G5. 2026-09-08 3관문 마리오 입구 네 곳 비활성화

Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json의 Mario1_go, Mario2_go, Mario3_go, Mario4_go만 enabled=false로 변경했다. 각 trigger의 transform/action/destination과 내부 intro·이동 trigger는 보존했다. 수정 전 JSON 대비 명시된 enabled 네 개 외 의미 값이 같음을 확인했다.

World publisher Publish PASS. Server/Bin/DataFiles/World/KAKULSAYDON_ARENA.worldbootstrap의 네 enabled field=0과 Client viewer 결과도 확인했다. 변경 JSON strict parse PASS. 통합 Debug Product build는 out/BuildPipeline/runs/20260908T025058075Z-debug-product.json에서 PASS했다. 이 결과는 새 Server 시작 후 소비된다. 실제 Client에서 네 입구가 작동하지 않는지는 사용자 확인 대기다.
