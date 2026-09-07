# 발탄 통로 진행과 두 번째 입장 컷신 통합 결과

## 1. 구현 상태

`claude.00.txt`는 현재 구현을 교체하는 명령이 아니라 이전 좌표와 흐름을 복구하기 위한 참고 자료로
사용했다. 현재 저장소의 최신 `Gameplay.world.json`, navigation, 발탄 encounter 계약을 우선하여
다음 수직 슬라이스를 실제 프로젝트에 통합했다.

- 발탄 입장 시작 위치를 통로 초입의 4인 spawn으로 정렬했다.
- `Stage_MiniBoss -> Stage_2 -> Stage_3 -> Stage_Boss_ArenaEntry -> Stage_Boss` 진행을
  authoring trigger action으로 연결했다.
- Debug 전용 하드코딩 우회를 제거하여 Debug와 Release가 같은 world authoring을 소비한다.
- SpawnGroup의 오래된 선행 조건을 제거해 각 trigger activation이 해당 stage의 유일한 활성화 권위가
  되도록 했다.
- 기존 `VALTAN_ENTRANCE_CINEMATIC`은 삭제하지 않고 dormant authoring으로 보존했다.
- 두 번째 입장 컷신 `VALTAN_ENTRANCE_CINEMATIC_IDLE`을 복원하여 첫 scripted slot으로 지정했다.
- 두 번째 컷신 동안 발탄은 `mesh_idle_battle_1`을 반복하고 무적 상태를 유지한다.
- `camera.valtan.entrance-idle.orbit`은 발탄 중심을 추적하며 12.5초 동안 9개 keyframe을
  CATMULL_ROM으로 보간한다. 모든 카메라 점은 arena 중심 XZ 반경 12m 안에 있다.
- 컷신 종료 뒤 scripted sequence가 다음 전투 패턴으로 계속 진행한다.
- Tool, runtime, publisher가 두 입장 컷신 ID를 모두 optional entrance로 인식하되 동시에 두 개를
  활성 flow에 넣거나 일반 회전에 섞는 데이터는 거부하도록 계약을 일반화했다.

런타임 `Client/Bin/Resources`의 모델, 텍스처, 음원은 수정하지 않았다. 이번 변경은 Data authoring,
publisher 산출물, Server/Client 연결 코드와 계약 테스트만 포함한다.

## 2. 자동 검증

- Valtan tuning publisher validate/publish: PASS
  - source revision: `f2955a814ff80d53ca4054a9bdffbce0689f48c79526297dc9fce58a4f0c0b7b`
  - candidate revision: `bd6358e5d71f8231f24bd164ede32e652968d77189de0e9637ec6a732025f355`
  - flow revision: `6c5de4ca34788e891e15218cf198c0e1c76614e082f091c7115601144e7e22f7`
  - 58 patterns / 256 stages / 34 managed patterns / 9 projected artifacts
- World gameplay validate/publish: PASS
  - 154 placements / 3 SpawnGroups
- Valtan Python regression: 73 tests, PASS
- focused camera contract: PASS
- saved flow contract: 27/27 PASS
- pattern tree contract: 26/26 PASS
- Boss Tool contract: 21/21 PASS
- Debug Server build: PASS
- Debug `Server.exe --contract-test`: `failures : 0`
- Release Server build: PASS
- Release `Server.exe --contract-test`: `failures : 0`
- Debug Client build: PASS
- Release Client build: PASS
- Debug EffectRenderContractHarness build/run: PASS
- Release EffectRenderContractHarness build/run: PASS

빌드에는 기존 코드페이지 `C4819/C4828`과 DirectXTK PDB `LNK4099` 경고가 남지만 컴파일과 링크,
계약 실행은 모두 exit code 0으로 완료됐다. protocol 구조를 바꾸지 않았으므로 새 network protocol
revision은 필요하지 않다.

## 3. 수동 확인 경계

Client 시각 결과는 자동 PASS로 판정하지 않는다. 최종 사용자는 Server와 Client를 재시작한 뒤
`Lobby -> Valtan`으로 진입하여 시작 위치, 각 트리거, 몬스터 활성화, 발탄 Idle 유지, 카메라 벽 가림과
컷신 종료 뒤 전투 전환을 직접 확인한다. 특히 카메라가 arena 벽이나 기둥에 가려지는지는 실제 화면의
최종 projection과 배경 geometry가 관여하므로 사용자 육안 승인이 남아 있다.
