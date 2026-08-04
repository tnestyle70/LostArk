# 2026-08-05 전 class Skill/Animation Binding RESULT

## 1. 결과

다섯 playable class의 quick slot ACTIVE 스킬과 LMB COMBO 평타를 Server-authoritative gameplay와
animator-authored Character presentation까지 연결했다. DimensionMaster의 최종 배열은
`Q W E R A S D F T V + LMB(4단)`이며 `Z`, `ALT_V`, candidate `2050550`은 포함하지 않는다.

Animation Tool에는 현재 Scene Character model의 clip을 key/skill에 연결해 저장하는
`Key -> Skill Animation` panel이 추가됐다. ACTIVE는 ordered clip sequence, COMBO는 Server
`comboStages`와 같은 고정 BA1/BA2/BA3/BA4 row를 사용한다. 저장 성공 뒤 runtime reload는 실행 중
action의 chain을 즉시 교체하지 않고 다음 action 경계에서 commit한다.

## 2. 실패했던 이전 접근의 원인

1. Character Select preview에서 local clip을 재생하는 것과 Server gameplay action을 같은 것으로
   취급했다. 제품 경로는 `CPlayerController -> IPlayerCommandSink -> Server approval -> snapshot ->
   CCharacter`이므로 local preview만으로 skill 사용이 닫히지 않는다.
2. DimensionMaster에는 stable Server skill definition이 없었고 Gunslinger/Slayer/Artist도 Q/W만
   존재했다. Client animation mapping을 먼저 늘려도 Server가 skillId를 승인·복제할 수 없었다.
3. `.clipseq/.clipmap` reference를 runtime mapping처럼 사용하려 했다. 이 파일은 원작 추출 참고
   자료이고 animator의 수정 가능한 저장 정본이 아니었다.
4. key, skill 수치, animation clip의 소유권이 분리되지 않았다. 이번에는 PlayerSkills의 stable
   skillId와 별도 authored presentation document를 strict join해 중복 authority를 제거했다.
5. F6 free camera와 gameplay command가 불필요하게 결합돼 있었다. camera mode gate를 제거해 F6가
   command 제출 가능 여부를 바꾸지 않게 했다.

## 3. 최종 데이터 계약

| 데이터 | 소유 내용 |
|---|---|
| `Data/Balance/PlayerSkills.json` | class/inputSlot/skillId/kind/Server timing/resource/range/comboStages |
| `Data/Balance/DamageProfiles.json` | Server damage rate profile |
| `Data/Animation/Authored/<Asset>/<Asset>.skillbindings.json` | animation asset/class owner와 skillId별 ordered model clips |
| `Data/Animation/Reference`, `.skilltiming/.clipmap/.animnotify/.clipseq` | read-only 저작 참고 |
| snapshot | Server가 승인한 action/skillId/actionStartTick/iComboStage |

현재 PlayerSkills는 53개의 skill definition을 가진다. publisher는 COMBO stage를 펼쳐 72 runtime
skill row와 54 damage profile을 검증한다.

```text
Lance Master    Q W E R A S T V ALT_V + LMB 34010(4단)
Gunslinger      Q W E R A S D F T V ALT_V + LMB 38000(3단)
Slayer          Q W E R A S D F V ALT_V + LMB 45000(4단)
Artist          Q W E R A S V ALT_V + LMB 31000(4단)
DimensionMaster Q W E R A S D F T V + LMB 2050010(4단)
```

authored binding loader는 exact field, format version, stable asset token, expected asset/class owner,
중복/누락/unknown skill, 실제 model에 없는 clip, COMBO stage 수를 검사한다. Save는 sibling temp에
write/flush한 뒤 strict reparse와 equality 검사를 거쳐 replace/write-through한다. 실패 시 기존 문서를
보존한다.

## 4. Runtime과 Tool 반영

- `CCharacter` spawn은 binding 문서 누락/오류 때문에 실패하지 않는다. presentation miss는 해당
  action edge만 한 번 격리하고 transform/HUD/다른 replication을 계속 적용한다.
- Server action edge마다 이전 chain을 지우고 stable skillId에서 새 chain을 resolve한다.
- COMBO는 Server `iComboStage`가 2 이상으로 처음 보이더라도 올바른 chain을 시작한 뒤 정확한 stage로
  직접 이동한다. Client가 combo 단계를 추측하지 않는다.
- ACTIVE ordered clips는 순차 재생하고 마지막 pose를 Server `NONE`까지 유지한다.
- Tool의 event document Dirty와 skill binding Dirty는 분리된다. target 전환 guard는 둘 다 확인한다.
- 잘못되거나 없는 binding에는 `Create Repair Draft from Current Clip`이 모든 현재 Server skill row를
  포함한 Dirty draft를 만든다.
- Tool row는 고정 Q/W 목록이 아니라 PlayerSkills를 읽어 정렬하므로 이후 합법적으로 추가되는 slot도
  숨기지 않는다.
- Dimension Core/Summon/reference preview는 playable Scene Character가 아니므로 binding Save를 막는다.
- F6 camera mode와 gameplay command gate의 결합을 제거했다.

## 5. 함께 닫은 Effect/시작 오류

Effect Tool의 미완성 formatVersion 4/vector resource header가 실행 중인 G04 구현과 섞여 compile이
깨져 있었다. `Effect_AuthoringDocument`를 현재 검증된 formatVersion 2 typed singular resource
contract로 맞추고 `CEffect_Tool` 선언을 실제 caller와 일치시켜 Debug/Release Client build를 복구했다.

비어 있던 `Client/Bin/Resources/Effect/DimensionMaster`는 2026-08-05에 기존 원본과 영수증으로
재추출했다. historical Git blob `47b8e670b3095244bbdc642cc60a8f123d270794`의
`export_effect_runtime_assets.py`, 외부 intake manifest, 보존된 exact-object source를 사용했으며 결과는
693 DDS + 139 WModel, 합계 832파일/90,608,380 bytes다. DDS magic 오류 0, duplicate/bad asset ID 0,
WModel `info` decode 실패 0이고 sorted runtime inventory SHA-256은
`685dfd24f32f510905ad8d4be69ba2c0afda7a78d13e87373f84aa71e1017811`이다. 이 payload는 현재
`2026.08.04.1` immutable manifest에 포함돼 있지 않은 로컬 복구분이므로 lock/manifest를 수정하지 않았다.

Debug Client 시작의 `abort()`는 필수 font가 없을 때 DirectXTK `SpriteFont` 생성자가 던진 예외가
`HRESULT` 경계를 건너 `std::terminate()`로 진행한 것이 원인이었다. `CCustomFont::Initialize()`가
표준/비표준 예외를 포착해 객체를 정리하고 `E_FAIL`을 반환하게 수정했다. 같은 빈 Resources 조건에서
프로세스 abort 대신 기존 Client 초기화 실패 메시지 경계까지 도달하는 것을 확인했다. 폰트 fallback은
추가하지 않았으며 실제 성공은 정확한 locked resource pack이 필요하다.

## 6. 자동 검증

| 검증 | 결과 |
|---|---|
| Gameplay balance Validate | PASS — 5 profiles, 72 skills, 54 damage profiles, 1 boss |
| Navigation publish | PASS — Character Select `62x62`, walkable 2176, maxStep 0.474029541015625 |
| NetworkProtocolHarness Debug/Release | PASS — `failures : 0` |
| ClientFrontendHarness Debug/Release | PASS — 5문서 53/53 coverage, atomic save/rollback 포함 `failures : 0` |
| Server Debug/Release `--contract-test` | PASS — 전 class quick skill/BA combo 포함 `failures : 0` |
| Engine/Shared/Server/Client Debug/Release build | PASS |
| ProjectAudit의 신규 계약 | PASS — skill animation authoring 53/53, data boundary, F6 independent gameplay, Effect G4 boundary |
| Debug Client 빈 Resources error smoke | PASS — `abort()` 없음, controlled startup failure |

NetworkProtocolHarness의 World Snapshot payload size 한 항목은 damage-event count 1바이트가 추가된 뒤
기대값 `150`이 남아 실패했다. 직렬화 round-trip은 정상이었고, 하네스 기대값을 header/player/cooldown/
entity 필드별 크기식으로 교정한 뒤 Debug/Release 모두 통과했다.

## 7. 수동 검증 BLOCKED

현재 `Data/AssetPacks.lock.json`의 정확한 pack은 `2026.08.04.1`이지만 로컬/외부 pack 저장소에 해당
payload가 없다. `Client/Bin/Resources`에는 위에서 복구한 `Effect/DimensionMaster`만 있고 lock이 요구하는
Character/Deploy/Fonts/Map/UI와 `Effect/Shared`는 여전히 없다. 따라서 Lobby 첫 render, Character Select 진입,
class 변경, Q/W/E/R/A/S/D/F/T/V/LMB 육안 animation, remote player 동작은 실행 증거를 만들 수 없었다.
이전 `2026.08.03.4` pack을 강제로 섞지 않았다.

정확한 pack을 받은 뒤 남은 절차는 다음과 같다.

1. lock의 immutable ZIP SHA-256 확인 후 Hydrate/Verify한다.
2. Server + Debug Client를 `Client/Default` working directory로 실행한다.
3. Lobby → Character Select → class 선택 → Enter Test로 Server gameplay 재진입한다.
4. F1 Animation Tool에서 Scene Character의 key/skill/BA mapping을 수정·Save·reload한다.
5. 다섯 class의 전체 ACTIVE key와 LMB combo, remote snapshot stage, F6 전후 동일 입력을 확인한다.
6. Client/Server 종료 후 잔류 process와 7777 listener가 없는지 확인한다.

## 8. 문서 인계

Animation 담당자의 실제 수정 절차와 정본 경계는
`.md/TEAM/ANIMATION_TOOL_OWNER_HANDOFF.md` 15장에 반영했다. 팀 전체 public contract는
`AGENTS.md`, `CLAUDE.md`, `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`에 갱신했다.
