# 2026-08-20 발탄 갑옷 — 표현 복구와 Server 상태 (G1)

## 1. 이번 변경의 경계

여덟 개의 독립한 계약을 닫았다.

```text
A. Client 표현    발탄이 어깨/상완 갑옷과 팔뚝 갑옷을 실제로 착용한다
B. Server 상태    갑옷이 피해를 감쇄하고 GROGGY 창에서만 내구도를 잃는다
C. 돌진 루프      돌진이 벽에 부딪히면 기절하고, 반복되므로 판 두 개를 모두 벗길 수 있다
D. Client 반영    파괴된 판이 snapshot으로 복제되고 해당 파츠가 사라진다
E. 부위 파괴 연출  판이 떨어지는 순간 원본의 부위 파괴 반응 애니메이션이 나온다
F. 갑옷 분기      갑옷이 남아 있는 동안에만 나오는 패턴을 저작할 수 있다
G. 페이즈 분기    페이즈로도 분기하며, 페이즈 경계를 109줄 전환 기믹에 맞췄다
H. 130줄 전멸     전멸기가 130줄에 걸리고 그 동안 발탄이 무적이 된다
```

A~C는 Shared protocol을 건드리지 않는다. D에서 protocol 25 -> 26으로 판 파괴만 복제한다.

## 2. A. Client 표현 — 실제 결함 세 가지

### 2.1 스킨 웨이트가 통째로 깨져 있었다

UModel glTF는 `WEIGHTS_0`을 `componentType=5121`(UNSIGNED_BYTE) + `normalized=true`로
저장한다. `ModelAssetConverter`(Assimp)가 그 바이트를 float으로 읽어 denormal을 만든다.

```text
원본 .bin        1584 정점 전부 weights_u8=(255,0,0,0), 합 255
구워진 Parts1    1583 정점 weight 0, 정점 0번은 (inf, inf, nan, inf)
구워진 Parts2    2194 중 2190 정점 weight 0
```

`WMeshReader.cpp:383`의 `!isfinite(weight)`가 false를 반환하므로 **Parts1은 정점 하나 때문에
모델 전체가 디코드 실패**했다. 그것이 `[CModel] Prototype creation failed.` 한 줄의 정체다.
Parts2는 디코드에 성공하지만 디코더가 weight 0인 정점을 전부 본 0(RootNode)에 재바인딩한다.

첫 쿠킹(13:47)부터 이미 깨져 있었다. glTF 계층 수정이 원인이 아니다.

### 2.2 좌표계가 본체와 달랐다

본체는 `PSK -> Blender -> FBX`, 갑옷은 `glTF` 경로로 구워져 서로 다른 bind 공간에 있었다.
`C:\LostArkValtanCook\MN_RPBF_01_AnimSet`(08-16 산출물, 본체와 skeletonHash 동일)로 변환을
실측했다.

```text
PSK -> 본체 wmodel     (x, y, -z)      13670 / 13670 점 일치 (100.00%)
PSK -> 갑옷 wmodel     (px, pz, py)    1230/1230, 1660/1660 점 일치
따라서 갑옷 -> 본체     (x, z, -y)      det +1, 순수 회전이라 와인딩 보존
```

### 2.3 스킨드 파츠가 본체 회전을 못 받았다

`CBody_Valtan::Initialize`는 `Rotation(0.f, -90.f, 0.f)`으로 모델 전방축을 엔진 LOOK에 맞춘다.
`CPart_Equipment::Update`는 `pSocketRootMatrix`를 소켓 분기 **안에서만** 적용했으므로 스킨드
갑옷은 그 frame을 받지 못했다. 캐릭터 장비는 `CPart_Body`에 회전이 없어 이 구멍이 드러나지
않았다.

### 2.4 적용

`MN_RPBF_01_Parts1.wmodel` / `_Parts2.wmodel`을 ActorX PSK 기준으로 재작성했다. PSK는 본체를
구운 것과 같은 소스이며 84본 이름·부모·rest가 동일하다(최대 오차 0.07).

```text
정점        (x, z, -y) 회전, normal/tangent 동일 회전
웨이트      PSK RAWWEIGHTS의 float 값으로 교체
본 테이블   본체 wmodel의 87본 MESH_BONE_ENTRY를 그대로 복사
골격 섹션   본체 wmodel의 WSKL 22448 바이트를 그대로 복사
바운드      submesh별 재계산
```

`Client/Private/Part_Equipment.cpp`의 `Update`에서 `pSocketRootMatrix` 적용을 소켓 분기 밖으로
뺐다. 이 필드를 세팅하는 곳은 발탄의 무기와 갑옷뿐이므로 캐릭터 장비 동작은 바뀌지 않는다.

### 2.5 검증

디코더 규칙을 재현한 전수 검사(헤더·플래그·stride·WMSH/WSKL 이름 해시·인덱스 범위·정점
유한성·트레일링 바이트)에서 세 파일 모두 OK.

```text
                        본 기준 |offset| 중앙값   본체 표면까지 중앙값
본체 자체 정점            49.9 cm                  -
고친 Parts1               46.5 cm                  13.9 cm
고친 Parts2               41.1 cm                  10.3 cm
기존 Parts1(깨진 상태)   355   cm                 155   cm
기존 Parts2(깨진 상태)   266   cm                  85   cm
```

사용자가 실제 Client에서 갑옷 착용을 육안으로 확인했다.

## 3. B. Server 갑옷 상태

### 3.1 데이터 계약

`Data/Balance/BossProfiles.json`

```json
"armorPlates": [
  { "plateIndex": 0, "durability": 4000, "defense": 50 },
  { "plateIndex": 1, "durability": 4000, "defense": 50 }
]
```

`Data/Encounters/Valtan/ValtanEncounter.json`의 `VALTAN_ARMOR_BREAK_OPENING` GROGGY 스테이지가
`stageKind: "ACTIVE"`에서 `"GROGGY"`로 바뀌었다. 문자열 접미사 추측 대신 typed 계약을 쓴다.

publisher는 `BOSSARMOR<TAB>archetypeId<TAB>plateIndex<TAB>durability<TAB>defense` 행을 만들고
plateIndex가 0부터 조밀한지, 판이 4개 이하인지, defense가 1~10000인지 검사한다.

### 3.2 런타임

```text
BOSS_ARMOR_PLATE                    profile이 소유하는 저작 정의
SERVER_BOSS_ARMOR_PLATE_STATE       entity가 소유하는 남은 내구도
SERVER_WORLD_ENTITY::bPatternGroggy  GROGGY 스테이지 동안만 true
BOSS_PATTERN_STAGE_KIND::GROGGY      PATTERN_ACTIVE로 복제되어 wire 변경 없음
```

`ApplyPlayerHitDamage`가 유일한 player -> world entity 피해 지점이다. 보스는 이제 성한 판의
defense 합으로 `Apply_Defense`를 통과하고, `bPatternGroggy`일 때만 그 피해만큼 앞쪽 판의
내구도를 깎는다. **한 창은 판을 최대 하나만 부순다** — 판이 0이 되는 순간 창을 닫는다.

판이 없는 entity는 defense 0이므로 갑옷 도입 이전과 같은 raw damage를 받는다.

### 3.3 검증

```text
Publish-GameplayBalance.ps1 -Mode Validate   PASS
Publish-GameplayBalance.ps1 -Mode Publish    PASS  boss 1, armour plate 2
Gameplay.bootstrap                            BOSSARMOR 2행 생성 확인
Server x64 Debug 빌드                         성공
Server.exe --contract-test                    failures : 0 (갑옷 계약 5건 포함)
NetworkProtocolHarness                        failures : 0
git diff --check                              공백 오류 없음
```

추가한 계약 테스트 5건.

```text
[PASS] Load Valtan's two authored armour plates from the gameplay bootstrap
[PASS] Mitigate a boss hit by each intact plate and by nothing once all break
[PASS] Leave armour durability untouched by damage outside a groggy stage
[PASS] Spend groggy damage on the front plate only, at the mitigated amount
[PASS] Break one plate per groggy window without spilling overkill onto the next
```

## 4. 함께 고친 기존 실패 두 건

두 실패 모두 HEAD에 커밋된 상태였고 버전을 올린 쪽이 상대편을 갱신하지 않아 생겼다.
같은 원인이 다시 나지 않도록 숫자를 박아 둔 자리를 없앴다.

### 4.1 gameplay bootstrap version 10 vs 11

`GameplayCatalog.cpp`는 version 11만 읽는데 wall-contact 픽스처가 10을 찍고 있어
`Accept one exact ACTIVE axe wall-contact row`가 내용이 아니라 헤더 때문에 거부됐다.
같은 파일의 다른 픽스처 세 곳은 이미 11이었다.

`Server/Public/GameplayCatalog.h`에 `GAMEPLAY_BOOTSTRAP_VERSION`을 두고 loader와
wall-contact 픽스처가 함께 읽는다. 구버전 거부 케이스는 `GAMEPLAY_BOOTSTRAP_VERSION - 1u`가
되어 다음 bump에도 뜻이 유지된다.

### 4.2 NETWORK_PROTOCOL_VERSION 24 vs 25

`Test_WorldDestructionProtocol`이 `24u == NETWORK_PROTOCOL_VERSION`을 검사하고 있었다.
이 테스트가 소유하는 계약은 world destruction 패킷 타입 두 개이지 버전 숫자가 아니다.
harness의 다른 열 곳은 전부 심볼을 그대로 쓰고, 불일치 검사도 `+ 1u`로 상대 비교한다.

숫자 고정을 제거하고 이름을 `World Destruction Packet Types Are Known`으로 바꿨다.

### 4.3 결과

```text
Server.exe --contract-test        failures : 0
NetworkProtocolHarness            failures : 0
```

## 4.5 C. 돌진 루프 — 갑옷 두 개를 벗길 수 있게

### 4.5.1 왜 한 개밖에 못 벗겼나

돌진·벽 충돌·기절이 `ValtanBrain.cpp`에서 문자열 세 개로 한 패턴에 묶여 있었다.

```text
ARMOR_BREAK_PATTERN_ID  "VALTAN_ARMOR_BREAK_OPENING"
ARMOR_BREAK_STAGE_ID    "WALL_CHARGE"
ARMOR_BREAK_ACTION_ID   "valtan.mechanic.armor-break-opening.charge"
```

이 세 상수를 `Update`, `Try_BuildImpactMotion`, `Complete_ImpactStage` 세 곳이 비교했다.
그 패턴은 159줄 HEALTH_BAR 트리거라 판당 하나씩 필요한 GROGGY 창이 fight 전체에 한 번뿐이었다.
`VALTAN_DASH_CHARGE`는 audition에 5회 등장하고 selectionWeight 30인 반복 패턴이지만 저 세
문자열에 해당하지 않아 벽에 부딪혀도 아무 일이 없었다.

### 4.5.2 저작 계약으로 바꿨다

`Data/Encounters/Valtan/ValtanChargeImpactActions.json` (format v1)이 어느 스테이지가
돌진인지 소유한다. 기존 `ValtanWallContactActions.json`과 같은 `(patternId, stageId, actionId)`
모양이고 balance receipt 대상이 아니다.

```json
{ "patternId": "VALTAN_ARMOR_BREAK_OPENING", "stageId": "WALL_CHARGE", "actionId": "valtan.mechanic.armor-break-opening.charge" },
{ "patternId": "VALTAN_DASH_CHARGE",         "stageId": "CHARGE",      "actionId": "valtan.attack.dash-charge.active" }
```

publisher가 join을 검증하고 `PATTERNSTAGECHARGE` 행을 만든다. 검증 조항은 세 가지다.
스테이지가 정확히 하나로 join될 것, 바로 뒤 스테이지가 `GROGGY`일 것, 패턴의 `maximumRange`가
양수일 것. 마지막 둘이 없으면 돌진이 벽에 멈춘 뒤 그냥 계속 걸어간다.

행 이름이 `PATTERNSTAGECHARGE`인 이유는 publisher가 모든 행을 `Sort-Object`로 정렬하기
때문이다. `PATTERNCHARGEIMPACT`로 두면 `PATTERNSTAGE`보다 먼저 파싱되어 스테이지가 아직
없다. `PATTERNWALLCONTACT`가 이미 같은 정렬 규칙에 기대고 있어서 두 번째 규칙을 만들지 않았다.

### 4.5.3 기절은 벽 파괴가 아니라 충돌을 따른다

기존 `GameRoom.cpp`는 `Apply_WorldDestructionImpact`가 실제로 벽을 부순 경우(`triggered`)에만
GROGGY로 넘어갔다. 아레나의 impact receiver는 12개뿐이고 한 번 부서지면 사라지므로 그 규칙은
루프를 스스로 닫아 버린다. 이제 receiver에 닿기만 하면 스테이지가 넘어간다. 벽 파괴는 그대로
별개의 결과다.

그리고 GROGGY는 시계로 진입할 수 없다. `NextClockStageIndex`가 GROGGY 스테이지를 건너뛴다.
돌진이 아무것도 못 맞히면 기절 없이 RECOVERY로 떨어진다. 이건 `VALTAN_ARMOR_BREAK_OPENING`에도
같이 적용되는 동작 변경이다 — 이전에는 못 맞혀도 시계가 GROGGY로 넘겼다.

### 4.5.4 돌진 패턴

`VALTAN_DASH_CHARGE`의 스테이지가 3개에서 4개가 됐다.

```text
[0] WINDUP    WINDUP    600ms   양손 지면 3회
[1] CHARGE    ACTIVE    500ms   BOX 10x2.5, 저작 maximumRange 20m를 500ms에 주파
[2] GROGGY    GROGGY   5000ms   충돌로만 진입
[3] RECOVERY  RECOVERY  900ms
```

selectionMode는 NORMAL, selectionWeight 30, maximumConsecutiveUses 2 그대로다. 즉 fight 내내
반복되고, 벽에 닿을 때마다 창이 열린다. 창 하나가 판 하나를 부수므로 두 판 모두 벗겨진다.

### 4.5.5 검증

```text
Publish  1 boss, 2 armour plates, 33 patterns, 127 pattern stages
bootstrap  PATTERNSTAGECHARGE 2행 (line 308-309, PATTERNSTAGE 뒤)
Server.exe --contract-test   failures : 0
NetworkProtocolHarness       failures : 0
```

추가·갱신한 계약 테스트.

```text
[PASS] Load a charge stage backed by a groggy stage for both the opening and the dash
[PASS] Author exactly one groggy stage behind each of the two charge stages
[PASS] Keep the armour-opening dash a repeatable weighted pattern with travel distance
[PASS] Leave a stage the encounter never authored as a charge without impact motion
[PASS] Advance the opening charge from Server-authored fixed-tick motion
[PASS] Advance the authoritative charge action to GROGGY only after impact
```

`catalog.Load()` 실패 시 `Get_Status()`를 출력하도록 계약 테스트에 한 줄 추가했다. 이번에
`Boss pattern charge-impact has no stage owner`를 이 줄 없이는 빌드 한 번을 더 태워야 알 수
있었다.

### 4.5.6 남은 경계

아레나의 impact receiver는 12개(wall159 10 + 입구 2)이고 중앙 기준 방위 89~158도와 314~318도
두 구간에만 있다. 즉 돌진이 항상 벽을 맞히지는 않는다. 플레이어를 향해 달리므로 방향이 맞을
때만 창이 열린다. 원작처럼 어느 방향으로 달려도 벽에 닿게 하려면 아레나 둘레에 receiver를 더
배치해야 하고, 그건 맵 담당의 collision 저작 범위다.

## 4.6 D. 갑옷 파괴의 Client 반영

### 4.6.1 무엇을 복제하는가

`WORLD_ENTITY_SNAPSHOT`에 `iBrokenArmorMask`(uint8) 하나를 더했다. bit i가 서면 저작 판 i가
파괴된 상태다. 남은 내구도는 보내지 않는다 — 표현이 필요한 것은 어느 파츠를 숨길지뿐이고,
판 수는 곧 파츠 순서라 별도 ID 공간이 필요 없다.

`NETWORK_PROTOCOL_VERSION`은 25에서 26이 됐다.

판 개수 상한은 이제 wire가 소유한다.

```text
Shared::MAX_WORLD_ENTITY_ARMOR_PLATES = 4
  <- Shared 검증  mask < (1 << 4)
  <- Server 파서  MAXIMUM_BOSS_ARMOR_PLATES
  <- Client       CValtan::MAX_ARMOR_PART_COUNT
```

G1에서 Server .cpp에 따로 두었던 상수 4를 이 하나로 합쳤다. publisher만 PowerShell이라 자체
리터럴 4를 유지한다.

### 4.6.2 경로

```text
SERVER_BOSS_ARMOR_PLATE_STATE.iRemainingDurability == 0
  -> GameRoom  snapshot.iBrokenArmorMask |= 1 << plateIndex
  -> Shared    Write_U8 / Read_U8, 상한 초과 mask는 write와 read 모두 거부
  -> CClientReplication  entity.iBrokenArmorMask 전달
  -> CValtan::Apply_ArmorBreakState  Find_PartObject -> CPart_Equipment::Set_Visible(false)
```

`Apply_ArmorBreakState`는 직전에 적용한 mask를 기억해 값이 그대로면 파츠를 훑지 않는다.
모델 로드에 실패해 태그가 없는 판은 건너뛴다. `Ready_ArmorParts`와 같은 격리다.

spawn 메시지는 갑옷 상태를 싣지 않는다. 새로 들어온 Client는 판이 모두 붙은 상태로 시작하고
다음 tick의 snapshot이 바로잡는다. 상태 채널은 snapshot 하나로 유지한다.

### 4.6.3 검증

```text
NetworkProtocolHarness                failures : 0
Server.exe --contract-test            failures : 0
ClientFrontendHarness
  --character-select-valtan-prewarm-fast   failures : 0
  --valtan-pattern-effects-fast            failures : 0
Client x64 Debug 빌드                  성공
```

추가한 protocol 계약.

```text
[PASS] World Snapshot Entities Round Trip            mask 0x01이 판 identity로 왕복
[PASS] World Snapshot Payload Size                   entity 1바이트 증가 반영
[PASS] Reject A Broken Armour Mask That Names An Unsupported Plate
```

## 4.7 E. 부위 파괴 연출

### 4.7.1 원본이 주는 것

`Valtan.clipseq`의 420627 `1번째 부위 파괴`와 420628 `2번째 부위 파괴`는 둘 다 같은 체인을
HOLD 모드로 쓴다.

```text
mesh_dmg_parts_start_1 -> mesh_dmg_parts_loop_1 -> mesh_dmg_parts_end_1 -> mesh_idle_battle_1
```

즉 두 액션의 **애니메이션은 동일**하고 다른 것은 `Par_D_RPBF_PartsDestruction_01` / `_02`
이펙트뿐이다. 그 이펙트는 `Data/Effects/Imported/RawResourceInventory`에 이름만 있고 Authored
Effect 문서가 없다. 그래서 이번에 연결한 것은 애니메이션 반응이고, 이펙트는 미복구로 남는다.

### 4.7.2 이미 붙어 있던 것과 비어 있던 것

`Valtan.patternbindings.json`을 열어 보니 오프닝은 이미 이 체인을 쓰고 있었다.

```text
armor-break-opening.charge    -> mesh_dmg_parts_start_1
armor-break-opening.groggy    -> mesh_dmg_parts_loop_1
armor-break-opening.recovery  -> mesh_dmg_parts_end_1
```

비어 있던 것은 두 가지다. G2에서 추가한 `dash-charge.groggy`에 바인딩이 없어 스턴 중에 generic
공격 clip이 나왔고, 판이 실제로 떨어지는 순간에 해당하는 stage 자체가 없었다.

### 4.7.3 PART_BREAK stage

`BOSS_PATTERN_STAGE_KIND::PART_BREAK`을 추가했다. `GROGGY`와 함께 **event-entered** stage이며
`Is_EventEnteredStage`가 그 규칙 하나를 소유한다. 시계는 이런 stage를 건너뛴다.

배치는 pattern의 **마지막**이다. 판이 떨어지면 평소 RECOVERY 대신 이 반응이 나오므로 clip이 한
번씩만 재생되고, 기존 바인딩을 하나도 고치지 않아도 된다.

```text
VALTAN_ARMOR_BREAK_OPENING   WALL_CHARGE GROGGY RECOVERY PART_BREAK
VALTAN_DASH_CHARGE           WINDUP CHARGE GROGGY RECOVERY PART_BREAK
```

publisher는 charge를 선언한 pattern이 PART_BREAK stage를 정확히 하나 가질 것을 요구한다.

### 4.7.4 전이 소유권

내구도를 깎는 곳은 `ApplyPlayerHitDamage`이고 그 자리에는 catalog도 brain도 없다. 그래서 판이
0이 되면 `bPendingArmorBreakReaction`만 세우고, stage 전이는 다음 tick에 brain이 소유한다.
damage 경로가 보스를 직접 움직이지 않는다.

brain은 `+1`이 아니라 pattern의 PART_BREAK stage를 찾아 들어간다. 그 stage가 마지막이므로
끝나면 pattern이 종료된다. `PART_BREAK`은 `PATTERN_RECOVERY`로 복제되어 wire 변경이 없다.

### 4.7.5 추가한 바인딩

기존 항목은 건드리지 않고 세 줄만 더했다.

```text
valtan.attack.dash-charge.groggy              -> mesh_dmg_parts_loop_1
valtan.attack.dash-charge.part-break          -> mesh_dmg_parts_end_1
valtan.mechanic.armor-break-opening.part-break -> mesh_dmg_parts_end_1
```

세 clip 모두 실제 모델에 있음을 확인했다. AnimSet 146 + body 27 = 173 clip을 직접 읽어 대조했다.
바인딩 문서는 clip 하나만 없어도 통째로 거부되고 catalog generic clip으로 조용히 떨어지므로
합성 데이터만 쓰는 harness 테스트로는 이 확인을 대신할 수 없다.

### 4.7.6 검증

```text
Publish                              33 patterns, 129 pattern stages
Server.exe --contract-test           failures : 0
NetworkProtocolHarness               failures : 0
ClientFrontendHarness                failures : 0
Client x64 Debug 빌드                 성공
model clip 대조                       dmg_parts start/loop/end 3종 실재 확인
```

```text
[PASS] Break one plate per groggy window, close it, and queue the part-break reaction
[PASS] Author one trailing part-break reaction for each charge pattern
[PASS] Keep the stun and the part-break reaction out of the pattern clock
```

## 4.8 F. 갑옷 유무에 따른 패턴 분기

### 4.8.1 계약

pattern마다 `armorRequirement`를 선언한다. 값은 `ANY`, `ARMORED`, `STRIPPED`이며 기존 33개
pattern은 전부 `ANY`로 시작한다. `iMinimumHealthBar`/`iMaximumHealthBar` 밴드와 같은 자리에
같은 방식으로 붙는 선택 조건이다.

판을 하나도 저작하지 않은 boss는 **stripped로 읽는다.** 그래야 갑옷이 없는 encounter가
이 필드가 생기기 전과 똑같은 패턴을 계속 고른다.

publisher는 `HEALTH_BAR` pattern이 `ANY`가 아닌 값을 선언하면 거부한다. 대본이 있는 기믹을
갑옷 상태로 막으면 fight가 의존하는 단계가 조용히 사라진다.

### 4.8.2 실제로 건 게이트

`VALTAN_DASH_CHARGE`만 `ARMORED`다. 이 encounter에서 돌진의 존재 이유가 갑옷 루프
(돌진 -> 벽 -> 기절 -> 판)이고, 판이 전부 떨어지면 그 루프에 남은 일이 없다.

audition에서도 돌진은 occurrence 2, 5, 10, 18, 24에만 나오고 그 뒤로는 한 번도 없다.
다만 24가 109줄이라 이 관찰은 **2페이즈 전환 이후 사라진다**는 뜻이지 갑옷과 직접 연결된
증거는 아니다. 즉 이 게이트는 우리 encounter의 설계 판단이고, 원본이 확정해 준 값이 아니다.

`iPhase`는 여전히 쓰기만 하고 아무도 읽지 않는다. audition이 실제로 가리키는 것은 phase
게이트이며 그것은 별도 작업으로 남는다.

### 4.8.3 네 번째 소비자까지

`ValtanEncounter.json`의 pattern 속성 집합을 읽는 곳이 네 군데였다.

```text
Tools/GameplayPipeline/Publish-GameplayBalance.ps1   PATTERN 행 발행
Tools/WorldPipeline/Publish-WorldGameplay.ps1        Server pre-build에서 같은 문서 검증
Client/Private/EncounterPatternReference.cpp         Effect cue join의 encounter 참조
Client/Private/BalanceTool.cpp                       F1 Balance Tool 읽기와 저장
```

BalanceTool은 encounter를 **다시 쓰기까지** 하므로 필드를 넣지 않으면 저장 한 번에 게이트가
사라진다. 읽기, 검증, 저장 세 군데를 모두 연결했다.

`PATTERN` bootstrap 행이 14 필드에서 15 필드가 되면서 계약 테스트의 합성 픽스처 4개가 한꺼번에
깨졌다. 76건 실패가 전부 이 폭 하나였다.

### 4.8.4 규칙을 공개한 이유

`Is_ArmorRequirementMet`을 `CValtanBrain`의 public static으로 두었다. 익명 namespace에 두면
계약 테스트가 RNG 선택을 400번 돌려 간접 관찰해야 한다. 규칙 자체를 이름 붙여 공개하니
선택 경로와 테스트가 같은 한 함수를 소비한다.

### 4.8.5 검증

```text
Publish-GameplayBalance -Mode Validate/Publish   PASS
Publish-WorldGameplay   -Mode Validate/Publish   PASS
Server.exe --contract-test                       failures : 0
NetworkProtocolHarness                           failures : 0
ClientFrontendHarness                            failures : 0
Client x64 Debug 빌드                             성공
```

```text
[PASS] Offer the armour-opening dash only while a plate is still on
[PASS] Meet an ARMORED requirement only while some plate is still on
[PASS] Read a boss with no plate left, and one with none authored, as stripped
```

## 4.9 G. 페이즈에 따른 패턴 분기

### 4.9.1 먼저 고친 것 — 페이즈가 두 군데서 다르게 정의돼 있었다

`iPhase`는 `phaseTwoHpPercent`가 50이라 **80줄**에서 2로 바뀌었다. 그런데 encounter의 전환
기믹 `VALTAN_ARENA_BREAK_109`는 **109줄**에 걸려 있고, 선배 설명도 `체력이 109줄 이하가 되면
컷씬이 나오면서 2페이즈로 넘어갑니다`이다. 즉 50은 두 정본과 어긋난 값이다.

health bar는 `ceil(hp * 160 / 60000)`이므로 실측하면 이렇다.

```text
phaseTwoHpPercent=50  ->  hp<=30000  ->  health bar  80
phaseTwoHpPercent=68  ->  hp<=40800  ->  health bar 109
phaseTwoHpPercent=69  ->  hp<=41400  ->  health bar 111
```

68로 고쳤다. 전환 기믹은 bar 109에 들어서는 순간(hp<=40875) 발화하고 `iPhase`는 그 bar 안쪽
hp<=40800에서 뒤집힌다. 75 HP 차이는 같은 bar 안이라 서로 어긋나지 않는다.

정확히 하려면 percent 대신 bar를 저작하는 편이 낫지만, 그건 `phaseTwoHpPercent` 자체를 교체하는
별도 계약이다. 이번에는 값만 정정했다.

### 4.9.2 계약

`armorRequirement`와 같은 형태로 `phaseRequirement`를 pattern마다 선언한다. 값은 `ANY`,
`PHASE_ONE`, `PHASE_TWO`이고 33개 pattern 전부 선언한다. publisher는 `HEALTH_BAR` pattern이
`ANY`가 아닌 값을 선언하면 거부한다.

`CValtanBrain::Is_PhaseRequirementMet`을 public static으로 두어 선택 경로와 계약 테스트가 같은
함수를 소비한다. 이 함수는 상태를 읽기만 하고 언제 페이즈가 바뀌는지는 정하지 않는다.

### 4.9.3 실제로 건 게이트

`VALTAN_DASH_CHARGE`가 `PHASE_ONE`이다. audition에서 돌진은 occurrence 2, 5, 10, 18, 24에만
나오고 24가 109줄이며 그 뒤로 한 번도 없다. 이번에 페이즈 경계를 109로 맞췄으므로 이 게이트는
관찰된 사실을 그대로 재현한다.

돌진은 이제 `ARMORED`이면서 `PHASE_ONE`이다. 선배 설명의 `갑옷 여부나 보스 페이즈 변수에
따라`가 그대로 두 조건으로 들어간 셈이고, 둘 중 하나라도 깨지면 돌진이 목록에서 빠진다.

### 4.9.4 소비자와 행 폭

pattern 속성을 읽는 네 소비자를 모두 갱신했다. `PATTERN` bootstrap 행은 15에서 16 필드가 됐고
계약 테스트의 합성 픽스처 4개도 함께 넓혔다.

```text
Publish-GameplayBalance.ps1      필드 16번 발행과 검증
Publish-WorldGameplay.ps1        같은 문서 tolerate
EncounterPatternReference.cpp    Effect cue join
BalanceTool.cpp / .h             읽기, 검증, 저장 round-trip
```

### 4.9.5 검증

```text
Publish-GameplayBalance  Validate/Publish   PASS
Publish-WorldGameplay    Validate           PASS
Server.exe --contract-test                  failures : 0
NetworkProtocolHarness                      failures : 0
ClientFrontendHarness                       failures : 0
Client x64 Debug 빌드                        성공
```

```text
[PASS] Offer a phase-gated pattern only in the phase it names
[PASS] Advance to phase two on the bar the authored transition mechanic fires on
[PASS] Advance Valtan phase from server HP
```

## 4.10 H. 130줄 전멸과 무적
### 4.10.1 실측해 보니 즉사는 이미 되어 있었다

선배 설명의 `플레이어 모두를 죽이는 스킬`은 데이터가 이미 만족한다.

```text
damage.valtan.omnidirectional-wipe-130   rate 100000
boss attackPower                         100
=> Resolve_Damage(100, 100000) = 100000 피해
player maximumHp                         5000 ~ 6500
SECOND_SMASH                             CIRCLE outer 100.0 = 아레나 전체
```

defense 105를 물려도 `Apply_Defense(100000, 105) = 48780`으로 여전히 최대 체력의 7배가 넘는다.
즉 형태도 위력도 전멸기가 맞고, 실제로 빠져 있던 것은 **무적 하나**였다.

### 4.10.2 트리거 115 -> 130

이름은 `VALTAN_FLOOR_WIPE_130`인데 `triggerHealthBar`가 115였다. 사용자 결정에 따라 130으로
옮겼다. 함께 움직인 곳은 셋이다.

```text
ValtanEncounter.json          triggerHealthBar 115 -> 130
ValtanDebugAudition.json      occurrence 19 targetHealthBar 115 -> 130
receipt                       같은 field의 sourceValue/resultValue
```

계약 테스트 픽스처도 옮겼다. 보스를 43125 HP(bar 115)에 세우고 116에서 내려오게 하던 것을
48750 HP(bar 130)와 131로 바꿨다. `ceil(48750 * 160 / 60000) = 130`이다.

### 4.10.3 무적

pattern 단위 저작 flag `invulnerableWhileRunning`을 33개 pattern에 선언하고 wipe만 `true`다.
stage가 아니라 pattern에 둔 이유는 `발탄이 무적상태가 되고 ... 전멸 패턴이 등장합니다`가
패턴 전체의 성질이기 때문이다.

```text
BeginPattern      boss.bPatternInvulnerable = pattern.bInvulnerableWhileRunning
FinishPattern     false 로 되돌림
ApplyPlayerHitDamage  invulnerable 이면 즉시 return
```

`ApplyPlayerHitDamage`에서 그냥 return 하는 것이 계약의 핵심이다. HP도 안 움직이고 damage
event도 안 나가므로 **데미지 숫자가 아예 뜨지 않는다.** 클라이언트는 기존 채널만으로 무적을
알아보므로 protocol 변경이 없다. 별도의 무적 아이콘이 필요해지면 그때 snapshot에 실으면 된다.

`PATTERN` bootstrap 행은 16에서 17 필드가 됐고 소비자 넷과 합성 픽스처 4개를 함께 넓혔다.

### 4.10.4 검증

```text
Publish-GameplayBalance  Validate/Publish   PASS  (2760 field entries)
Publish-WorldGameplay    Validate           PASS
Server.exe --contract-test                  failures : 0
NetworkProtocolHarness                      failures : 0
ClientFrontendHarness                       failures : 0
Client x64 Debug 빌드                        성공
```

```text
[PASS] Absorb every player hit while an invulnerable pattern runs
[PASS] Fire the invulnerable wipe on bar 130 and author no other invulnerable pattern
[PASS] Reach the whole arena with a wipe hit that outdamages the toughest player pool
[PASS] Apply the queued 130-bar Valtan six-direction hit once
[PASS] Queue and advance the staged 130-bar scripted mechanic
```

## 5. 아직 하지 않은 것

```text
부위 파괴 이펙트           I절에서 저작 문서까지 만들었다. 남은 것은 runtime 등록
                           (EffectCatalog 행 + patterneffectcues 행)과 사용자 육안 판정이다.
아레나 receiver 배치       12개가 방위 89~158도와 314~318도 두 구간에만 있어 돌진이 항상
                           벽을 맞히지는 않는다. 맵 담당의 collision 저작 범위다.
페이즈 경계의 정확한 저작  phaseTwoHpPercent 68은 bar 109 안쪽 hp<=40800에서 뒤집힌다.
                           bar를 직접 저작하면 정확해지지만 필드 교체가 필요하다.
실행 검증                  갑옷 착용만 화면으로 확인됐다. 돌진 루프, 부위 파괴 반응,
                           갑옷 사라짐, 패턴 분기, 109줄 페이즈, 130줄 전멸과 무적은
                           계약 테스트만 통과한 상태다.
```

## 6. 사용자 확인이 필요한 것

Server 재시작 후 발탄에게 딜을 넣으면 데미지 숫자가 갑옷 상태에 따라 달라진다.

```text
판 2개 성함    피해 절반
판 1개 성함    피해 약 2/3
판 0개         감쇄 없음
```

159줄에서 `VALTAN_ARMOR_BREAK_OPENING`이 돌 때 5초 GROGGY 동안 4000 피해를 넣으면 첫 판이
깨지고 이후 데미지 숫자가 올라간다.

## 7. I. 부위 파괴 이펙트 저작 문서 (2026-08-21)

### 7.1 5절의 전제가 틀렸다

`Par_D_RPBF_PartsDestruction_01/02`은 "이름만 있는" 상태가 아니었다. 두 시스템 모두
`Valtan.effect-resource-catalog.json`에 그래프까지 추출돼 있고, 이를 소비하는 asset request
35건(mesh 5 + texture 30)도 payload가 실물로 존재한다.

```text
par_d_rpbf_partsdestruction_01   emitter 11   module 75   resource binding 13
par_d_rpbf_partsdestruction_02   emitter 10   module 75   resource binding 12
```

그리고 그 emitter들은 이미 element 18개(각 9개)로 만들어져
`effect.valtan.armor-break-opening.charge.effect.json` 안에 들어가 있었다.

### 7.2 전용 문서가 없던 진짜 이유

`build_valtan_stage_effects.py`는 stage -> clip -> source system으로 join한다. 두
PartsDestruction 시스템의 `clipNames`는 둘 다 `Dmg_Parts_Start_1`인데, E절이 만든 PART_BREAK
stage는 `mesh_dmg_parts_end_1`에 물려 있다. `Dmg_Parts_Loop_1`이나 `Dmg_Parts_End_1`을 named
하는 소스 시스템은 하나도 없으므로 seeder가 `stages_without_element`로 건너뛴다.

원본 체인은 이렇고, 액션 420627이 이를 3회 반복한다.

```text
Dmg_Parts_Start_1  1.5s     <- PartsDestruction_01/02 가 t=0 에 발화
Dmg_Parts_Loop_1   1.333s
Dmg_Parts_End_1    3.0s
Idle_Battle_1      2.333s
```

따라서 현재 이 이펙트는 판이 떨어지는 순간이 아니라 **WALL_CHARGE 단계, 곧 발탄이 벽에
부딪히는 순간** 재생된다. 이건 남은 결함이다.

### 7.3 판 identity는 소스가 직접 준다

```text
_01   material mn_rpbf_01_1_mi   mesh fm_d_rpbf_02 / fm_d_rpbf_03
_02   material mn_rpbf_01_2_mi   mesh fm_d_rpbf_04
```

`mn_rpbf_01-1` / `-2`는 A절이 다시 구운 `MN_RPBF_01_Parts1/Parts2.wmodel`, 곧 plateIndex
0과 1의 텍스처다. 즉 원본도 판마다 다른 이펙트를 쓴다. 그런데 cue 계약에는 plate 축이 없다.
그래서 한 문서 안에 두 시스템을 넣고 `groupId`로만 갈랐다. 판별 재생은 별도 계약이다.

### 7.4 만든 것

```text
Tools/EffectPipeline/build_valtan_part_break_effects.py
Data/Effects/Authored/effect.valtan.armor-break-opening.part-break.effect.json
Data/Effects/Authored/effect.valtan.dash-charge.part-break.effect.json
```

각 18 element다. emitter row 추출, slot 배정, render profile, mesh pre-scale, 입자 seed
기본값은 전부 형제 seeder의 함수를 import해서 쓴다. 사람이 정한 것은 join 하나뿐이다.
`lifeTimeSeconds` 1.4는 encounter의 PART_BREAK `durationMs` 1400에서 나온다.

### 7.5 EffectCatalog에 등록하지 않은 이유

`Publish-Effects.ps1:917`이 cue 없이 catalog에 오른 Valtan Effect를 거부한다. 실제로 행을
넣어 한 번 깨뜨려 확인했다. 같은 자리의 주석이 "디스크에 문서만 있고 catalog 행이 없는 상태"를
정상 authoring-only로 명시하고, Effect Tool의 Valtan pattern tree는
`VALTAN_STAGE_EFFECT_ORIGIN::NAMING_RULE`로 경로에서 직접 찾으므로 지금도 열린다.
catalog 행과 `Valtan.patterneffectcues.json` cue 행은 한 변경으로 같이 넣어야 한다.

### 7.6 남은 추출 결함

각 시스템의 9개 emitter가 전부 같은 mesh를 단다. 소스 그래프는 material binding을 export
105~113에, mesh binding을 export 175~178에 따로 기록하는데 emitter -> module 간선이
직렬화돼 있지 않아 join할 근거가 없고, seeder는 시스템의 첫 mesh로 fallback한다. 결과적으로
`fm_d_rpbf_03`, `fm_c_elid_01`, `fm_a_stone_002`는 추출돼 있으나 아무도 참조하지 않는다.
추측 배정하지 않았다. 또 emitter 11개 중 9개, 10개 중 9개만 element가 된다. 나머지는 material
binding이 없다.

### 7.7 검증

```text
resource binding 176개 전부 Client/Bin/Resources 에 실재      missing 0
charge 문서의 같은 18 element 와 필드 대조                    groupId·lifeTime 외 차이 0
Publish-Effects.ps1 -Mode Validate                            PASS  exit 0  (192 catalog entries)
두 번 실행                                                     결과 동일
JSON parse / git diff --check                                  clean
```

사용자 육안 판정 경로는 `F1 -> Effect Tool -> Valtan pattern tree ->
VALTAN_ARMOR_BREAK_OPENING / PART_BREAK` 및 `VALTAN_DASH_CHARGE / PART_BREAK` -> `Open`이다.
에이전트는 Client를 실행하지 않았고 visual PASS를 기록하지 않았다.

## 8. 커밋 분할 (2026-08-21)

미커밋은 두 세션이 아니라 네 세션 것이 섞여 있었다.

```text
S1 갑옷            이 문서의 A~H와 I
S2 109 붉은 하늘   카메라, 셰이더, 엔진 종료 순서
S3 참조 아레나 복구 철탑 phase registration 포함
S4 패턴 세션       회오리 SPIN_2/3, 4기둥 prop 파괴, audition 후보 확정
```

A~H를 여덟 커밋으로 나누지 않았다. 트리에 최종 상태만 남아 있고 `PATTERN` bootstrap 행 폭이
F/G/H에서 14 -> 15 -> 16 -> 17로, 프로토콜이 D에서 25 -> 26으로 움직였으므로, 중간 커밋을
만들려면 존재한 적 없는 중간 상태를 새로 지어내야 한다. 하나의 검증 단위로 묶었다.

네 문서는 S1과 S4가 한 파일 안에 섞여 있어 워크트리를 건드리지 않고 인덱스에만 갑옷 내용을
올렸다. 분류는 세 규칙을 순서대로 적용한다. 블록 안의 갑옷 표식이 최우선, 다음이 패턴 세션
키워드, 마지막이 가장 가까운 owner 행(`"patternId"` / `"targetId"`)이다. 회오리 WINDUP의
`800 -> 1200`은 아무 이름도 담지 않은 숫자 한 줄이라 세 번째 규칙으로만 잡힌다.

```text
ValtanEncounter.json                 갑옷 블록 35 유지   패턴 블록  1 되돌림
ValtanDebugAudition.json             갑옷 블록  1 유지   패턴 블록  5 되돌림
balance-provenance.receipt.json      갑옷 블록 47 유지   패턴 블록  4 되돌림
ServerGameplayContractTests.cpp      갑옷 블록 32 유지   패턴 블록  2 되돌림
```

되돌린 계약 테스트 두 줄은 패턴 세션 데이터에 직접 의존한다. `46 -> 48 hit stages /
72 -> 71 pulses`는 회오리 SPIN_2/SPIN_3가 만든 값이고, `61-start 8-idle -> 66-start 3-idle`은
audition 5건을 UNRESOLVED에서 PRODUCT_CANDIDATE로 바꾼 결과다. 갑옷만 있는 상태에서는 둘 다
HEAD 값이 정답이다.

인덱스 상태를 별도 worktree로 꺼내 실제로 검증했다.

```text
Publish-GameplayBalance.ps1      -Mode Validate   PASS
   6 player profiles, 136 skills, 108 damage profiles, 1 boss,
   2 armour plates, 33 patterns, 127 stages, 67 audition occurrences
Publish-WorldGameplay.ps1        -Mode Validate   PASS
Publish-ValtanWorldDestruction.ps1 -Mode Validate PASS
   groups=105 bindings=117 outer109=30 impact159=10 contacts=69
```

S2, S3, S4의 미커밋 변경은 그대로 두었다. 특히 S4는 작업 중이다.
