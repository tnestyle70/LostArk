# 베른 NPC UModel 런타임 배치 계획

## 목표

베른성 `zoneId=11102`에 실제 포함된 NPC 가운데 경비병과 주요 인물 5종을 UModel에서
선별 추출하고, 기존 NPC 수직 슬라이스에 추가한다. 로비의 Test 진입 후 Map Tool에서
Bern Area를 선택했을 때 새 archetype을 골라 `Gameplay.world.json`에 배치할 수 있어야 한다.

## 대상

| NPC ID | 이름 | 런타임 archetype |
|---:|---|---|
| 25287 | 경비병 로이아드 | `NPC_25287` |
| 25184 | 여왕의 기사단 | `NPC_25184` |
| 25029 | 근위대장 아리안느 | `NPC_25029` |
| 25001 | 에아달린 | `NPC_25001` |
| 25002 | 아델 | `NPC_25002` |

## 구현 경로

1. `Prepare/11102.loa`와 NPC/LookInfo 테이블로 베른 배치 ID와 모델 의존성을 확정한다.
2. UModel PSK/PSA/재질/TGA를 선택 패키지 단위로 추출한다.
3. 몸·머리·무기를 하나의 `npc_body`로 병합하고, 공유 가능 rig는 기존 HM_MA02 animset을 사용한다.
4. 자체 rig NPC는 해당 PSA를 모델에 포함하고 모든 애니메이션 trailer의 skeleton hash를 검증한다.
5. `Client/Bin/Resources/Character/NPC/Npc_<ID>`에 `.wmodel`과 참조 텍스처를 배치한다.
6. `Data/Actors/NpcCatalog.json`에 5종을 `supported`로 추가한다. Map Tool의 기존 catalog 기반
   드롭다운을 그대로 소비하며 별도 로컬 spawn 경로를 만들지 않는다.
7. World publisher, 모델 구조, catalog 참조, Server/Client 빌드를 검증한다.

## 완료 조건

- 5개 modelAssetId와 모든 외부 텍스처가 실재한다.
- HM_MA02 공유 대상의 skeleton hash가 기존 animset과 동일하다.
- 자체 애니메이션의 trailer hash 불일치가 없다.
- `Publish-WorldGameplay.ps1`이 전 Area를 통과한다.
- Test → Bern → Map Tool → World Gameplay → NPC에서 5개 archetype을 선택할 수 있다.
- 최종 화면 배치와 외형은 사용자가 직접 확인한다.
