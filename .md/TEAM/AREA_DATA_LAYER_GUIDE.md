# LostArk Area 데이터 레이어 가이드

## 1. 결론

Area별 visual, gameplay placement, navigation은 분리 저장된다. 어떤 entity placement가 없으면 해당 entity는 생성되지 않는다. 그러나 모든 레이어가 완전한 plug-in 구조인 것은 아니다. 현재 일반 몬스터, wave/증분 spawn, trigger, Area별 balance override, NPC client presentation은 제품 계약이 없다.

```text
LevelCatalog scenario
  -> MapCatalog area
     -> visual asset admission / placement
     -> optional deploy asset / placement pair
     -> Gameplay.world.json (playerSpawn / npc / boss)
     -> navigation authoring -> runtime navgrid
     -> stable actor / encounter / balance ID 참조
```

## 2. 현재 Area 실측

| Area | Visual | Gameplay | Navigation | 추가 데이터 |
|---|---|---|---|---|
| `LV_BER_BERNCASTLE` | shard-set, 50,017 placements | class-neutral player spawn 4 | Server 제품 navgrid 없음 | NPC/boss 없음 |
| `LV_LUT_HEARTRB_ED` | 13,103 placements | player spawn 4 + `BOSS_VALTAN` 1 | Valtan 62×63 | deploy pair, BossProfile, ValtanEncounter |
| `LV_DEV_TRAINING_GROUND` | RCArena 10 assets / 18 placements | class-neutral player spawn 4 | uniform 32×32 | NPC/boss/monster/trigger 없음 |

수련장은 Lobby의 `Enter Training`에서 Server 승인을 받은 뒤 `LEVEL::DEVELOPMENT`로 진입한다. Debug/Release network smoke는 map load, player spawn, Q command, Server action 승인, cooldown HUD 반영까지 검사한다.

## 3. 레이어별 생략 규칙

| 레이어 | 없을 때 | 불완전할 때 |
|---|---|---|
| visual map | scenario가 Map domain을 요구하면 load 실패 | catalog/placement 참조 오류는 rollback |
| deploy | `.deployassets`와 `.deployplacements`가 모두 없으면 skip | 둘 중 하나만 있으면 오류 |
| NPC/boss placement | 해당 kind 행이 없으면 spawn하지 않음 | unknown archetype/encounter는 publish 실패 |
| navigation | 현재 Bern처럼 Server nav를 사용하지 않는 world는 생략 가능 | nav를 요구하는 Valtan/Training은 누락·손상 시 room 기동 실패 |
| balance definition | 사용하지 않는 actor/skill 정의는 runtime state를 만들지 않음 | placement/action이 없는 stable ID를 참조하면 publish 또는 Server load 실패 |

`Gameplay.world.json` 자체는 Server가 여는 world마다 필요하다. 접속 가능한 world는 최소 하나의 활성 `playerSpawn`이 필요하므로, 빈 placements 문서를 제품 world의 정상값으로 취급하지 않는다.

## 4. MapTool이 지금 편집하는 것

- visual asset admission과 stable numeric placement
- Valtan deploy asset/placement pair
- `playerSpawn`, `npc`, `boss` gameplay placement
- navigation bake/walkability와 현재 Valtan runtime blocker 조건

`NpcCatalog.json`은 현재 비어 있고 `CClientReplication`의 world entity presentation은 Valtan boss만 완성되어 있다. 따라서 NPC 버튼과 저장 형식은 있어도 제품 NPC의 catalog → Loader → replication → `CNpc` 표현은 아직 닫히지 않았다.

일반 몬스터, spawn wave, 증분 생산, trigger volume/time/condition은 저장 schema와 Server consumer가 없다. 수업용 `CMonster`를 재사용하거나 `npc`/`boss`로 위장하지 않는다. 실제 요구가 생기면 archetype catalog, spawn/trigger 문서, Server brain, replication, Client presentation, balance, rollback harness를 한 수직 슬라이스로 추가한다.

## 5. Balance 소유권

Balance는 현재 Area별 파일이 아니라 전역 stable definition이다.

| 정본 | 소유 값 |
|---|---|
| `Data/Balance/PlayerProfiles.json` | class HP/resource/move speed |
| `Data/Balance/PlayerSkills.json` | skill slot/timing/cost/range/damage ID |
| `Data/Balance/DamageProfiles.json` | Server damage |
| `Data/Balance/BossProfiles.json` | boss HP/range/speed/phase threshold |
| `Data/Encounters/<Boss>/...json` | stage/encounter state와 pattern timeline |

Area는 수치를 복사하지 않고 stable actor/encounter ID를 참조한다. 지금은 ImGui Balance editor가 없으므로 JSON 수정 → `Publish-GameplayBalance.ps1 -Mode Validate` → Server contract/smoke 순서로 튜닝한다. 레벨별 override가 필요하면 별도 schema와 우선순위/rollback 계약부터 정해야 한다.

## 6. 네 캐릭터 roster 상태

Lobby는 Lance Master, Gunslinger, Slayer, Artist 네 slot을 모두 보여주고 선택할 수 있다. 실제 world 진입은 `Is_Supported_Playable_Character_Class`가 승인한 class만 가능하다.

| Class | Lobby 선택 | Resource pack | Client Loader/Spec | Server profile | World 진입 |
|---|---:|---:|---:|---:|---:|
| Lance Master | 가능 | 있음 | 완료 | 완료 | 가능 |
| Gunslinger | 가능 | 있음 | 완료 | training baseline | 가능 |
| Slayer | 가능 | 있음 | 완료 | training baseline | 가능 |
| Artist | 가능 | 있음 | 완료 | training baseline | 가능 |

네 class는 body·equipment·weapon `.wmodel`과 texture를 6-root resource pack에 admission하고, `CharacterCatalog`, `CCharacterCatalog`, Loader prototype, Server `PlayerProfiles`, class parser, spawn/remote presentation까지 연결한다. 현재 비-Lance class의 profile 수치는 class별 스킬 계약 전까지 명시적인 training baseline이며, LanceMaster로 조용히 대체하는 identity fallback은 금지한다.

Area 진입 시 네 class binary를 모두 선로드하지 않는다. Lobby 선택 class만 먼저 준비하고 다른 class는 실제 remote spawn에서 최초 한 번만 같은 `CPlayableCharacterAssetService`로 admission한다. 이 규칙은 mixed-class 표현을 유지하면서 Level 로딩 시간을 네 배로 늘리지 않기 위한 고정 경계다.

## 7. 새 Area 추가 체크리스트

1. `LevelCatalog.json` stable scenario와 기존 Engine Level 매핑
2. `MapCatalog.json` Area 및 실제 사용하는 visual admission만 등록
3. authoring placement → 원자 publish → runtime placement
4. class-neutral player spawn과 필요한 NPC/boss placement
5. navigation authoring, spawn/boss cell·height 검증
6. 필요한 actor/encounter/balance stable ID 연결
7. Loader/registry/publisher/ProjectAudit 등록
8. Debug/Release scenario smoke와 process cleanup
