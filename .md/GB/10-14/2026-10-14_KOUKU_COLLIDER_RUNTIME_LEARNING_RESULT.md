# Collider가 실제 피해로 이어지는 경계

읽는 순서: [00 처음부터 읽기](C:/Users/tnest/Desktop/LostArk/.md/GB/10-14/2026-10-14_KOUKU_FULL_PATTERN_WALKTHROUGH_LEARNING_RESULT.md) → [01 Python 파싱·투영](C:/Users/tnest/Desktop/LostArk/.md/GB/10-14/2026-10-14_KOUKU_JSON_PIPELINE_LEARNING_RESULT.md) → [02 Server 구조체·판정](C:/Users/tnest/Desktop/LostArk/.md/GB/10-14/2026-10-14_KOUKU_SERVER_RUNTIME_LEARNING_RESULT.md) → [03 Client 저작·애니메이션](C:/Users/tnest/Desktop/LostArk/.md/GB/10-14/2026-10-14_KOUKU_CLIENT_ANIMATION_LEARNING_RESULT.md) → [04 Collider·좌표·피해](C:/Users/tnest/Desktop/LostArk/.md/GB/10-14/2026-10-14_KOUKU_COLLIDER_RUNTIME_LEARNING_RESULT.md) → [05 전체 패턴 지도](C:/Users/tnest/Desktop/LostArk/.md/GB/10-14/2026-10-14_KOUKU_PATTERN_ATLAS_LEARNING_RESULT.md)

## 1. Collider 그림과 판정창을 연결하는 필드

저작 resource의 `kind=COLLIDER`, `shape`, `halfExtents`, `radiusM`은 기본 도형이다. pattern의 presentation occurrence는 이 도형을 언제·어떤 anchor·어느 bone·어떤 TRS로 사용할지 정한다. `logicOccurrenceId`가 그 도형을 판정할 Logic 시간 배치에 연결한다. 같은 logic 정의 ID를 직접 가리키는 필드가 아니다.

[projector의 _project_collider_regions](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:4683)는 resourceId→resource 표를 만들고 COLLIDER occurrence를 모은다. 일반 접촉은 `row["logicOccurrenceId"] == logic_box["occurrenceId"]`인 것만 고른다. 룰렛은 명시된 여덟 regionId와 문양·색 조합을 추가 검증한다. 시간·모델·도형이 맞아도 다른 Logic occurrence에 연결돼 있으면 기대한 창에서 평가되지 않는다.

도형 크기는 resource 기본값에 occurrence scale을 곱한다. 양수 scale, 지원되는 회전, 같은 패턴의 World 참조, World 수명 안의 판정창 등을 검사한다. 따라서 에디터에서 보이는 임의의 shape를 그대로 렌더 콜라이더 포인터로 Server에 전달하는 구조가 아니다.

## 2. 저작 anchor 이름과 Server anchor 이름은 같은 목록이 아니다

| 저작 occurrence | 게시된 Server region | 책임 |
|---|---|---|
| MAP | WORLD | 저장된 맵 월드 위치를 직접 사용한다. |
| BOSS + followBoss=true | BOSS_CURRENT | 현재 Server boss 위치·yaw를 마지막에 합성한다. |
| 고정 BOSS 접촉 + followBoss=false | BOSS_START | 지정된 capture 시점의 boss 기준을 고정해 이후 따라가지 않는다. 지원 kind 조건이 있다. |
| WORLD + worldId/정확한 world occurrence | WORLD 또는 BOSS_SPAWN | 해당 WorldSequence의 placement·motion을 샘플하며 저작 World의 기준도 보존한다. |

핵심은 **저작 WORLD가 단순히 “절대 월드 좌표”라는 뜻이 아니라 WorldSequence object를 기준으로 한다는 점**이다. `MAP → Server WORLD`도 있으므로 파일 층을 생략하고 “WORLD로 바꿨다”고만 말하면 다른 뜻이 섞인다. 실제 변환은 [4725~4758행](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:4725)에 있다.

BOSS_START는 시작 때 `Capture_StartRegions`가 계산한 도형을 WindowState의 FrozenRegions에 보존한다. 일반 transform evaluator에 아직 고정되지 않은 BOSS_START를 그대로 넘기면 실패한다. `Window_Region`을 통해 현재 창의 고정 도형을 얻는 흐름까지 읽어야 한다.

## 3. 손·무기 본을 Server가 어떻게 따라가는가

Client는 실제 CModel의 skeleton pose를 계산한다. 반면 Server의 이 판정 경로는 WModel의 CBone 포인터를 가지고 매 프레임 GPU pose를 평가하지 않는다. Python 게시 단계가 실제 모델/애니메이션의 해당 본을 시간별로 표본화해 `worldTrack` 같은 수치 키를 만든다.

BOSS의 bone Collider는 `_project_bone_collider_track`, World object의 bone은 `_project_world_bone_collider_track` 경로를 쓴다. 원본 clip 구간·재생 속도·blend·root suppression·body/weapon 기준이 표본의 의미를 결정한다. `boneRotation=BONE` 표본에 이미 occurrence local TRS가 들어갔다면 projector가 region의 local position/yaw를 0으로 만드는 이유는 같은 offset을 두 번 적용하지 않기 위해서다.

[Resolve_LogicRegionTransform](C:/Users/tnest/Desktop/LostArk/Server/Private/KoukuSaydonLogicRuntime.cpp:263)은 현재 pattern 경과 tick을 track 시각으로 바꾸고 양옆 key를 찾아 위치·크기·회전을 보간한다. visibility가 꺼진 표본이나 유효하지 않은 scale/시각은 평가를 거절한다. track이 있다는 사실만으로 모든 tick에 도형이 유효한 것은 아니다.

BOSS_CURRENT의 마지막 XZ 합성은 실제 코드상 다음 의미다. `localX/localZ`는 표본과 offset을 이미 합친 boss-local 중심, `θ`는 현재 Server boss yaw의 radian 값이다.

```text
worldX = bossX + cos(θ) × localX + sin(θ) × localZ
worldZ = bossZ − sin(θ) × localX + cos(θ) × localZ
worldYaw = localYaw + bossYaw
```

이는 이 프로젝트의 Y 회전 부호와 축 정의를 따른다. 임의 좌표계의 일반 회전 공식을 가져와 부호를 바꾸면 안 된다. 현재 코드에서 Y의 boss 높이 합산은 cylinder/linear-motion 조건과 연결되므로 모든 XZ 도형이 완전한 3D OBB 판정을 수행한다고 설명하지 않는다. 실제 도형별 Contains/Intersects 분기를 읽는다.

## 4. 겹침에서 HP와 넉백까지

player를 대상으로 하는 ENTER_AREA와 카드 object를 대상으로 하는 OBJECT_CONTACT는 다른 소비자다. P13 카드 뒤집기는 target 카드의 XZ 원과 region을 비교하고 World motion 결과를 낸다. P26 마지막 망치는 player 접촉을 판단하고 피해 RESULT를 실행한다.

현재 P26 `대형세이튼_피자`의 `P26.logic.1`은 start16997ms/duration100ms, 정의L507의 ENTER_AREA, 성공 결과L522의 MAX_HP_PERCENT_DAMAGE다. bundle.5에서 P25 `쿠크_피자`와 같은 offset0으로 시작한다. 명시된 pattern 길이는19498ms지만 stage 합은17600ms다. 피해 창은 마지막 animation stage의 시간대에 별도 배치돼 있다.

[Apply_Result](C:/Users/tnest/Desktop/LostArk/Server/Private/KoukuSaydonLogicRuntime.cpp:1438)는 피해 결과를 `SERVER_WORLD_TO_PLAYER_HIT` 값으로 바꾼다.

- 즉사면 현재 HP, 고정 피해면 damageAmount, 최대 HP 비율이면 maximumHp×percent 기반 rawDamage를 만든다.
- pushRange/pushMs/forcePush/arena 이탈/ballistic/높이를 같은 hit에 옮긴다.
- AWAY_FROM_CONTACT이면 실제 접촉 중심을 source로 쓴다. 이 방향인데 접촉 중심이 없으면 해당 결과를 실행하지 않는다.
- BOSS_FORWARD이면 boss yaw와 authored offset으로 밀기 방향을 만든다.
- 현재 패턴이 소유한 손 포획 상태면 capture owner/sequence도 hit에 전달한다.
- `CServerCombatHitRuntime::Apply_WorldToPlayer`로 실제 player 상태 변경을 맡긴다.

[공통 hit 적용](C:/Users/tnest/Desktop/LostArk/Server/Private/ServerCombatHitRuntime.cpp:664)은 생존·전투 자격·무적/보호·포획 소유권 등의 조건을 확인하고, damage와 shield를 해석한 뒤 HP를 갱신한다. HP 차이로 DamageEvent를 만들고 살아 있는 대상의 push 상태를 적용한다. 따라서 “도형 안에 있었다”와 “HP 감소 및 push가 확정됐다”는 서로 다른 지점이다.

## 5. anchor만 바꿨는데도 실패할 수 있는 이유를 좁히는 순서

여기서는 새로운 실패 원인을 단정하지 않는다. 현재 코드에서 확인할 수 있는 경계를 순서대로 적는다.

1. 현재 저장한 Composition revision과 실행에 채택된 Product revision이 같은가?
2. Collider occurrence가 정확한 Logic occurrence에 연결됐고 enabled 시간창 안인가?
3. 저작 anchor에서 게시 anchor로 어떤 변환이 됐고, bone/World track은 어떤 기준 좌표인가?
4. 해당 Server tick의 body 위치·yaw, track key, 최종 center/shape/높이가 실제 player body와 겹치는가?
5. 이미 맞은 기록·재접촉 설정·사망·무적·포획 자격 때문에 판정을 건너뛰었는가?
6. RESULT의 피해/넉백 값이 게시됐고 공통 hit 함수가 HP/push 상태를 바꿨는가?
7. 그 상태가 snapshot/event로 내려와 Client가 표시했는가?

앞선 별도 조사에서 만든 P17 손 추적 수정안이나 P26 실행 fixture 결과를 이 학습 조사에서 새로 적용·재검증했다고 기록하지 않는다. 이번 문서는 원인 추적에 필요한 실제 데이터 경계와 함수 책임을 설명한다.
