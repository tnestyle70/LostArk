# Bern3 배 NPC가 화면에 안 보이던 원인과 배치·게시 (2026-09-25)

정직 표기: **사실**(파일·수치·코드에서 직접 읽음), **추론**, **미확인**을 구분한다.
Client는 실행하지 않았다. 화면에서 NPC가 실제로 보이는지, 우클릭·창·탑승·이동이 손에 잡히는지는 사용자가 판정한다.

## 1. 결론

이전 배 NPC 2명은 서버에 스폰됐지만, 클라이언트가 모델을 **약 1.3cm 높이**로 그려서 사람이 볼 수 없었다. (사실, 아래 수치)
원인은 "NPC 모델 크기 계약"을 배 NPC 모델 두 개가 지키지 않은 것이다. 서버·거리·표시 문서·텍스처는 원인이 아니었다.

## 2. 층마다 대조한 결과 (잘 보이는 기존 NPC와 배 NPC)

- **서버 스폰**: 성립. 게시본 `BERN.worldbootstrap`에 배 NPC 행이 있었고, 서버 계약 테스트에서 방 안에 엔티티로 만들어지는 것을 확인했다(아래 5절).
- **서버 거리 필터**: 없음. `Server/Private`, `Shared/Public/Network`에서 관심 반경·스냅샷 엔티티 상한을 찾았으나 NPC 스냅샷을 거리로 거르는 코드는 없다. 클라이언트에도 NPC 거리 컬링이 없다. (사실, grep)
- **클라이언트 표시 문서(`BERN.npcpresentation.json`)**: 원인 아님. 문서에 항목이 없어도 카탈로그의 기본 idle 클립으로 NPC를 만든다(`Client/Private/ClientReplication.cpp`). 일반 NPC(`npc.bern.25002`)도 이 문서에 항목이 없다. 이전 보고에서 "이것이 원인일 수 있다"고 한 추정은 틀렸다.
- **텍스처**: 원인 아님. 두 모델이 참조하는 `textures/*.dds`가 모두 실제 파일로 있다. (사실)
- **idle 클립 이름**: 원인 아님. 카탈로그의 `idle_normal_1_1`, `idle_normal_1`이 각 모델 안에 실제로 들어 있다. (사실)
- **모델 크기 계약**: **원인**. 아래 3절.

## 3. 원인: NPC 모델 크기 계약 (사실, 수치)

`Client/Private/NpcPresentationAssetService.cpp`는 모든 NPC 모델을 고정 배율 `0.0001`(그리고 Y축 -90도)로 불러온다.
이 배율이 미터가 되려면 모델이 다음 구조여야 한다. 모든 정상 NPC WModel은 뼈대 맨 앞이 `RootNode`이고 그 아래 노드가 **크기 ×100**(과 Z-up을 Y-up으로 돌리는 회전)을 갖는다. 정점은 센티미터다. 센티미터 × 100 × 0.0001 = 0.01, 즉 미터가 된다.

- 게시된 NPC 모델 파일 110개를 전부 읽어 봤다(디코드 전용).
  - **108개**는 첫 뼈대가 `RootNode`이고 그 아래 뼈대의 크기가 정확히 100이다.
  - **2개**(`Npc_MN_RHKP_02_2`, `Npc_NP_LRKK_01`, 곧 배 NPC 두 종)만 그 노드가 없고 아래 뼈대 크기가 1이다.
- 정상 NPC(베다): 정점 높이 125.5 × 100 × 0.0001 = **약 1.26 m**.
- 배 NPC 조선공: 정점 높이 133.3 × 0.0001 = **약 0.013 m (1.3 cm)**. 항만 관리인: 135.8 × 0.0001 = **약 0.014 m**.
- 배 NPC 모델은 원작 NPC 조합 도구(저장소 밖 `build_npc.py`)가 이 PC에 없어서 배 작업에서 다른 파이프라인(`Tools/ShipPipeline/cook_npc.py`, 정점을 그냥 센티미터로 굽고 `RootNode` 노드를 만들지 않음)으로 조립했다. 그래서 크기 계약이 빠졌다. (추론: 조립 시점에 이 계약을 몰랐다.)

## 4. 수정

**데이터를 다시 만들지 않고 클라이언트가 모델 구조를 보고 배율을 정한다.**
재조립하려면 원본 추출물이 필요한데 배 NPC의 raw 추출물이 이 PC의 `out/`에 남아 있지 않고, 원작 조합 도구도 없다. 또 이 방식은 앞으로 같은 종류의 모델이 들어와도 조용히 안 보이는 일을 막는다.

- `Client/Private/NpcPresentationAssetService.cpp`: `Resolve_NpcModelPreScale`이 모델을 한 번 불러 **`RootNode` 뼈대가 하나도 없고** 뼈대 1번의 저작 크기가 1 근처(0.5~2)일 때만 센티미터 모델로 보고 배율 **0.01**로 다시 불러온다. `RootNode`가 있는 모델은 무조건 기존 `0.0001`을 쓴다. 안전장치다: 정상 NPC 108개는 전부 `RootNode`가 있어서, 엔진과 파이썬의 뼈대 순서가 다르더라도 100배 커지는 일이 생길 수 없다. 기존 정상 NPC 108개는 크기 100이라 한 번만 불러오고 결과가 같다. 배율을 바꾸는 경우에만 `EffectFailure.user.log`에 `npc.model.unit` 한 줄을 남긴다.
- 결과 높이: 조선공 133.3 × 0.01 = **1.33 m**, 항만 관리인 135.8 × 0.01 = **1.36 m**. 정상 NPC(1.26~1.6 m)와 같은 크기다.
- 방향(정면)은 같은 조립 체인에서 나온 모델이라 다른 캐릭터와 같은 규칙일 것으로 **추정**한다. 실제 정면이 틀어져 있으면 MapTool에서 yaw만 바꾸면 된다. **미확인.**

## 5. 배치와 게시

`Data/Worlds/LV_BER_BERNCASTLE/Gameplay.world.json`에 NPC 3개를 추가했다(다른 배치는 바이트까지 그대로, 64개에서 67개).

- `npc.bern.ship.shipwright.1` = `NPC_SHIP_SHIPWRIGHT` (271.59, 12.43, -196.70), yaw 259.6
- `npc.bern.ship.shipwright.2` = `NPC_SHIP_SHIPWRIGHT` (229.61, 12.47, -196.50), yaw 259.6
- `npc.bern.ship.harbormaster.1` = `NPC_SHIP_HARBORMASTER` (264.41, 11.79, -204.03), yaw 2.4

yaw는 사용자가 지정하지 않아 이전 값을 그대로 썼다. 세 좌표 모두 게시된 Bern3 네비 격자에서 걸을 수 있는 셀이고, 셀 높이가 사용자가 준 y와 정확히 같다(차이 0.00). Bern3 네비 정본은 수정하지 않았다.

게시: `Publish-WorldGameplay.ps1 -Mode Validate -WorldId BERN` 통과 후 `-Mode Publish -WorldId BERN`.
- `Server/Bin/DataFiles/World/BERN.worldbootstrap`: 배 NPC 3행 추가, 헤더 개수 64에서 67(리비전 922 그대로), 다른 행은 하나도 안 바뀜.
- 다른 월드 출력 22개(Server 10 + Client 12)는 게시 전과 바이트 동일. 클라이언트 `BERN.npcpresentation.json`도 동일.
- 게시 전 백업: `out/Bern3ShipNpcPlace20260925/backup/published/`. 원본 편집 백업: `.../backup/`.

## 6. 진단 로그 (안 보일 때 원인을 바로 알기 위한 것)

클라이언트: `Client/Default/EffectFailure.user.log` (같은 줄은 처음과 2의 거듭제곱 번째만 남는다)
- `npc.model.unit`: 센티미터 모델이라 배율을 0.01로 바꿔 불러왔다.
- `npc.ship.spawned placement=... pos=... idleClip=...`: 배 NPC가 실제로 몸을 얻었다. **이 줄이 없으면 클라이언트에서 안 만들어진 것이다.**
- `npc.presentation.unavailable placement=... reason=...`: NPC를 못 만든 이유(카탈로그 없음, 모델 태그 없음, 모델 준비 실패, 오브젝트 생성 실패, 네트워크 상태 적용 실패).
- `ship.npc.ready area=... count=N`: 클릭 대상으로 잡힌 배 NPC 수(월드 문서 없음/거부 사유 포함). 3이어야 한다.
- `ship.npc.clicked index=...`: 우클릭이 배 NPC에 맞았다.
- `ship.window.open`: 걸어가서 배 선택 창을 열었다.
- `vehicle.riding ...`: 탑승 요청 전송, 서버 응답(탑승됨/거절 사유), 요청이 거절된 사유.
- `vehicle.model ready vehicleId=... model=...` 또는 실패 사유: 배 모델을 준비했는지.

서버 콘솔(Debug/Release 공통):
- `[ShipNpc] spawned placement=... archetype=... pos=(...)` 와 `[ShipNpc] 3 ship NPC(s) live in this world`: 방이 배 NPC를 실제로 만들었다.
- `[VehicleRiding] player=... requested=... result=... active=... world=...`: 서버가 받은 탑승 요청과 결과(result 0이 ACCEPTED).

## 7. 전 구간 정적 점검

1. 서버 스폰: 성립(테스트). 세 NPC가 방 안 엔티티로 만들어지고 좌표가 게시 값과 같다.
2. 클라이언트 NPC 생성: **이번 수정으로 성립**(크기). 그 외 단계(카탈로그, 모델 준비, 객체 생성)에서 걸리는 조건은 코드에서 찾지 못했다. 모델을 엔진이 실제로 불러 그리는지는 미확인(Client 실행 없음).
3. `Ready_ShipNpcs`: 레벨 초기화에서 `Data/Worlds/LV_BER_BERNCASTLE/Gameplay.world.json`을 읽어 `NPC_SHIP_`으로 시작하는 켜진 NPC 위치를 모은다. 이번에 3개가 잡힌다(코드 확인).
4. 우클릭 → 걸어가서 창 열기: 클릭한 광선이 NPC 발 위치에서 1.5 m 안이면 잡히고, 캐릭터가 NPC 3 m 안에 들어오면 `Open_ShipWindow`를 부른다. **플레이어가 그 NPC와 같은 네비 영역(Bern3)에 있어야** 걸어갈 수 있다.
5. 배 선택 → `C2S_SET_VEHICLE_RIDING` → 서버 승인: 서버 계약 테스트(탈것 68건)에서 Bern의 배 8200~8208 탑승, 교체, 하차, 사망 시 강제 하차, 배 속도가 통과한다.
6. 배 모델·자세: `CVehiclePresentationAssetService`가 첫 탑승 때 동기 준비한다. 배 모델 9개를 읽어 보면 길이 약 4.4~9.4 m(모델 배율 0.01), 배마다 seat 뼈와 idle/run 클립이 있다. 실제 표시는 미확인.
7. 우클릭 이동: 기존 이동 경로 그대로이고 서버가 배 속도(1.8~2.2 m/s)로 이동한다(테스트). 배 이동 범위는 Bern3 네비 안이다.
8. 하차: 테스트로 확인.

## 8. 사용자가 Debug에서 확인하는 순서

1. 서버와 클라이언트를 **둘 다 재시작**한다(서버 게시본이 바뀌었다).
2. 로비에서 Bern으로 입장한다.
3. F1 → `Arena Camera / Player` → F6(자유 카메라) → 세 NPC 좌표 근처로 날아가 `Move Player` → 그 부두 바닥을 클릭한다. (Bern3는 다른 네비 영역이라 걸어서는 못 간다.)
4. F6으로 follow 카메라로 돌아와 NPC를 우클릭한다. 걸어가서 배 선택 창이 뜨고, 배를 골라 탑승 버튼을 누르고, 우클릭으로 이동한다.
5. 안 보이면 `Client/Default/EffectFailure.user.log`에서 `npc.ship.spawned`가 3줄 있는지, `npc.presentation.unavailable`이 있는지, `ship.npc.ready`의 count를 본다. 서버 콘솔에서 `[ShipNpc]`이 3줄인지 본다.

## 9. 미확인·한계

- 화면에서 NPC가 실제로 보이는지, 정면 방향, 대기 동작이 자연스러운지는 아무도 못 봤다.
- NPC 대화창(인사말과 기능 버튼)의 원본 UI는 복원하지 않았다. 우클릭으로 걸어가면 배 선택 창이 바로 열린다.
- 배 선택 창은 탈것 창 크롬을 재사용한 목록이다(원작 항해 창 아님). 이전 결과 문서의 미완료 항목은 그대로다.
- 바탕화면 Release 폴더들은 이 수정 이전 스냅샷이다.
