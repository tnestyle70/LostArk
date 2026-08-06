# 2026-08-05 MapTool Trigger Box 저작 기반 결과

## 1. 완료 범위

이번 변경은 Development MapTool 저작 기반과 제품 Server-authoritative `movePlayer` trigger runtime을
한 수직 슬라이스로 연결했다.

- `CTrigger_Box`를 Development 전용 3D wire OBB presentation으로 구현했다.
- MapTool World Gameplay에서 `Trigger Box` kind를 선택할 수 있다.
- 맵 표면을 클릭해 disabled draft를 배치할 수 있다.
- 선택한 박스의 position, yaw, half extents, trigger-once 정책을 편집할 수 있다.
- Area 전환과 Gameplay reload에서 새 박스를 먼저 stage하고 전부 성공한 뒤 기존 presentation을 교체한다.
- 저장된 disabled draft는 다시 로드해 같은 wire box로 복원된다.
- 선택 상태는 노란색, disabled 상태는 주황색, enabled 상태는 초록색 wire로 구분한다.

## 2. 저장 안전장치

typed action이 아직 없는 박스는 다음 계약으로만 저장된다.

```json
{
  "kind": "triggerBox",
  "enabled": false,
  "events": []
}
```

`events`가 빈 박스를 enabled 상태로 바꾸는 것은 문서 validation과 MapTool UI가 거부한다.
기존 제품 publisher는 `triggerBox`와 `destroyable`을 계속 fail-closed로 거부한다. 따라서 이번 완료는
플레이어 overlap 판정이나 점프·컷신·몬스터 생성 동작이 완료됐다는 뜻이 아니다.

## 3. 변경 파일

- `Client/Public/Trigger_Box.h`
- `Client/Private/Trigger_Box.cpp`
- `Client/Public/MapTool.h`
- `Client/Private/MapTool.cpp`
- `Client/Private/Loader.cpp`
- `Client/Private/WorldGameplayDocument.cpp`
- `Client/Default/Client.vcxproj`
- `Client/Default/Client.vcxproj.filters`
- `CLAUDE.md`
- `.md/TEAM/AREA_DATA_LAYER_GUIDE.md`
- `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`
- `.md/TEAM/UNIFIED_DATA_MANAGEMENT_ARCHITECTURE.md`
- `.md/GB/08-05/2026-08-05_MAP_GAMEPLAY_TRIGGER_DESTROYABLE_NAV_EXTENSION_PLAN.md`

## 4. 검증 결과

### PASS

- Client x64 Debug build 및 link
- `Client.vcxproj`, `Client.vcxproj.filters` XML parse
- `Publish-WorldGameplay.ps1 -Mode Validate`
  - BERN 4
  - VALTAN_ARENA 5
  - TRAINING_GROUND 4
  - CHARACTER_SELECT_ARENA 5
- 관련 파일 `git diff --check`

### 기존 저장소 상태로 인한 전체 감사 실패

`Tools/ProjectAudit/Invoke-ProjectAudit.ps1`은 이번 Trigger Box 코드가 아니라 다음 기존 항목으로 실패했다.

- asset lock inventory 불일치
- effect G1 document boundary
- `Character/DimensionMaster/DimensionMaster_Character.wmodel` 누락
- DimensionMaster runtime animation 감사 항목

### 수동 검증 필요

실행 화면에서 다음 smoke는 아직 사용자가 확인해야 한다.

1. Debug Client 실행
2. Lobby `Test` → Development Map Editor 진입
3. F1 → Map Tool → World Gameplay
4. `Trigger Box` 선택 후 고유 Placement ID 입력
5. `Arm World Placement` → 맵 표면 클릭
6. 주황색 wire box 생성 확인
7. position/yaw/half extents 편집 확인
8. `Save Gameplay` → `Reload Gameplay` 후 동일 박스 복원 확인

## 5. 추가 완료: 제품 movePlayer runtime

- 선택한 Trigger Box에 `targetPosition`, `durationSeconds`, `arcHeight`를 가진 `movePlayer`를 추가한다.
- `Publish-WorldGameplay.ps1`은 enabled box의 action이 정확히 하나일 때만 bootstrap v3으로 publish한다.
- Server는 yaw가 적용된 OBB enter edge를 판정한다.
- `triggerOnce=true`는 room 전체 한 번, false는 player가 나갔다 다시 들어올 때 재실행한다.
- Server가 직선 보간과 `4 * arcHeight * t * (1 - t)` 포물선 높이를 계산한다.
- Shared protocol 10의 `TRIGGER_MOVE` action과 player transform snapshot으로 Client에 복제한다.
- Client는 같은 action tick의 중복 실행을 무시하며 전용 jump clip 계약 전까지 RUN locomotion을 사용한다.

## 6. 검증 증거

### 자동 검증 PASS

- `Publish-WorldGameplay.ps1 -Mode Validate`: 4개 world 통과
- 임시 enabled movePlayer JSON을 별도 output root에 publish해 trigger bootstrap 20-field 행 생성 확인
- Shared/NetworkProtocolHarness/Server/Client x64 Debug 빌드 통과
- Shared/NetworkProtocolHarness/Server/Client x64 Release 빌드 통과
- Debug/Release `NetworkProtocolHarness.exe`: failures 0
- Debug/Release `Server.exe --contract-test`: failures 0
- Server trigger test: bootstrap v3 실제 행 파싱, 잘못된 duration rollback, OBB 진입, 포물선 중간점,
  정확한 도착, non-once 재진입 PASS
- 프로젝트 XML parse와 전체 `git diff --check` PASS

### 전체 ProjectAudit의 기존 실패

ProjectAudit은 이번 Trigger 계약이 아니라 기존 워크트리의 다음 4개 항목 때문에 실패했다.

- asset-lock inventory
- effect G1 document boundary
- `Character/DimensionMaster/DimensionMaster_Character.wmodel` 누락
- DimensionMaster runtime animation 감사 항목

### 수동 검증 필요

실제 Area에는 이 변경에서 Trigger Box placement를 넣지 않았다. 사용자가 MapTool에서 위치와 목적지를
저장한 뒤 publisher 실행, Server/Client 재시작, 제품 Valtan/Bern 진입으로 육안 smoke해야 한다.

## 7. 후속 수직 슬라이스

파티 전원 대기/타이머, 컷신, 몬스터 자동 생성, destroyable/dynamic navigation은 구현하지 않았다.
각 기능은 별도 Server authority·replication·rollback harness와 함께 추가해야 한다.
