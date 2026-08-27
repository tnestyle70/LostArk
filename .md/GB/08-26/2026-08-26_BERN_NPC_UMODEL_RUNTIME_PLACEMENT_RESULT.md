# 베른 NPC UModel 런타임 배치 결과

## 구현 완료

- 베른성 `zoneId=11102` 배치 ID에서 5종을 선별했다.
- `NPC_25287`, `NPC_25184`, `NPC_25029`, `NPC_25001`, `NPC_25002`를
  `Data/Actors/NpcCatalog.json`에 `runtimeStatus=supported`로 추가했다.
- 5종의 병합 `.wmodel`과 명시적으로 매핑한 diffuse/normal/specular/emissive 텍스처를
  `Client/Bin/Resources/Character/NPC/Npc_<ID>`에 배치했다.
- 여왕의 기사단 무기는 `b_wp_2`, 근위대장 쌍무기는 `b_wp_1/b_wp_2`에 결합했다.
- 두 경비병은 기존 `Character/NPC/AnimSets/HM_MA02/HM_MA02.wmodel`을 공유한다.
- 근위대장 아리안느 22개, 에아달린 84개, 아델 28개 클립은 각 모델에 포함했다.
- Map Tool은 기존 `CActorCatalog::Get_Npcs()` 경로를 사용하므로 새 5종이 Test Bern의
  `NPC Archetype` 드롭다운에 자동 노출된다. 별도 Client local spawn은 추가하지 않았다.

## 자동 검증

- UModel PSK/PSA 30개 signature: PASS
- build manifest 5종 전체 파일 참조: PASS, 누락 0
- ModelAssetConverter cook/info 5종: PASS
- skeleton hash:
  - `NPC_25287`, `NPC_25184`, 기존 `HM_MA02`: `0x2d32c3c861ce74d6`
  - 자체 애니메이션 134개 trailer mismatch: 0
- `Publish-WorldGameplay.ps1`: PASS
  - BERN 8, VALTAN_ARENA 154, TRAINING_GROUND 4, CHARACTER_SELECT_ARENA 5 placements
- Server x64 Debug build: PASS
- Client x64 Debug build: PASS
  - DirectXTK PDB `LNK4099` 기존 경고만 발생했으며 링크는 성공했다.
- `Server.exe --contract-test`: 기존 Bern 정본 불일치 2건으로 전체 FAIL
  - 현재 `BERN.worldbootstrap`은 authoring과 같이 8 placements인데 테스트는 제거된
    `trigger.bern.to-valtan`을 포함한 9 placements를 기대한다.
  - 같은 제거된 trigger까지의 navigation 도달 테스트도 함께 실패한다.
  - 신규 NPC catalog/model/animation admission과 무관하며 이 작업에서 Bern authoring을
    임의 복원하지 않았다. 나머지 계약은 통과했다.

## 수동 검증 경계

Client를 대신 실행하거나 화면을 판정하지 않았다. 사용자가 로비 → Test → Map Tool에서
Area `Bern`, `World Gameplay`, kind `NPC`를 고른 다음 신규 archetype을 배치·저장하고 외형,
무기 위치, 대기 애니메이션을 직접 확인해야 한다.
