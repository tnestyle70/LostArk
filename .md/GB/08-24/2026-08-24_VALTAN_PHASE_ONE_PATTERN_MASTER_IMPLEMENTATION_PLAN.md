# 발탄 1페이즈 Pattern Master 구현 계획

## 목표

`Data/Valtan/Valtan.pattern.json`을 발탄 1페이즈 pattern, body animation occurrence, Product/independent
Effect 사용 위치의 공동 authoring 정본으로 만든다. 기존 Server/Client typed 제품 경로는 publisher projection으로
유지하며 두 번째 boss runtime을 만들지 않는다.

## 실측 문제

- Effect Tool의 발탄 트리는 Encounter, patternbindings, patterneffectcues, CombatObjects를 합친 read-only view였다.
- pattern 전체를 로컬 재생하는 authoring timeline이 없고 stage/cue 단위 재생만 있었다.
- 도끼는 Server combat object였지만 HIGH_JUMP 아래 WORLD 행에 중첩됐고, 도넛은 FIST_IN_OUT cue에만 묶였다.
- DASH wall impact는 authored branch가 아니라 stage 배열의 다음 원소를 직접 선택했다.
- HIGH_JUMP의 두 번째 stage 전체가 하강 arc여서 6초 정점 대기를 표현할 수 없었다.
- FOUR_SLASH가 TRIPLE/ROTATION 두 selectable pattern으로 분리되어 같은 원본 sequence를 두 정답으로 만들었다.

## Admission 결정

1. 상세 요구의 HIGH_JUMP를 포함해 7개 pattern을 관리한다.
2. WHIRLWIND의 1.2초는 SPIN duration으로 적용한다.
3. DASH는 현행 세 source cut `600 + 600 + 2450ms`를 `repeatCount: 3`과 함께 보존한다.
4. FOUR_SLASH는 이전 stable ID를 복원하고 `WINDUP/SLASHES/SPIN/RECOVERY`로 합친다.
5. FOUR_SLASH hit는 분리형의 6회를 합치지 않고 결합형의 총 4회로 복원한다.
6. FLOOR_WIPE stable ID/trigger 130은 유지하고 모순된 115줄 표시명만 중립화한다.
7. ARENA_BREAK stable ID와 여섯 camera/world stage는 유지하고 displayName과 recovery presentation만 바꾼다.
8. damage 숫자는 중복하지 않고 `serverDamageProfileId`만 저장한다.
9. 모든 stage는 `EXACT`, `HOLD_LAST_POSE`, `LOOP_TO_STAGE_END` 중 하나의 animation `endPolicy`를
   명시하며 암묵적인 underfill/loop를 허용하지 않는다.

## 구현 단위

### G1. Master와 projector

- 새 파일: `Data/Valtan/Valtan.pattern.json`
- 새 파일: `Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1`
- root/stage/animation/independent-effect exact-property validation
- `repeatCount`와 ordered unique clip occurrence의 관계를 검증하되 publisher가 암묵적인 clip을 합성하지 않음
- animation `endPolicy`와 source wall 합계를 검증하고 설명 없는 hold/loop를 거부
- managed Encounter와 patternbindings transactional publish
- cue/combat-object/effect/damage/camera/world stable reference join
- malformed ID, duplicate, drift, missing reference와 중간 실패 rollback 검증

### G2. Server pattern projection

- DASH branch graph와 GROGGY flag lifetime을 Encounter에 저장
- `Complete_ImpactStage`가 `WALL_CONTACT` branch를 사용
- `PART_DESTROYED` typed branch가 legacy pending latch도 소비
- `serverMotion.travelStageId`를 bootstrap에 compile
- HIGH_JUMP는 AIRBORNE에서 apex hold, LAND에서 descent
- ARENA_BREAK는 기존 DROP descent 유지

### G3. Animation/Effect projection

- WHIRLWIND SPIN 1200ms와 두 회전 play rate 정렬
- FOUR_SLASH의 두 body clip과 세 Effect cue를 한 pattern으로 결합
- HIGH_JUMP AIRBORNE/axe life 6000ms 정렬
- ARENA_BREAK RECOVERY에 ledge-roar end clip과 recovery Effect cue 연결
- axe/donut을 master independent library entry로 선언

### G4. Effect Tool authoring consumer

- `VALTAN_INDEPENDENT_EFFECT_VIEW`와 master-managed metadata를 fail-closed로 load
- All Effects 최상위 `INDEPENDENT EFFECT` tree
- pattern 내부에는 같은 independent asset의 editable row를 중복하지 않고 reference만 표시
- 전체 ordered clip occurrence를 flatten하는 `Play Authoring Timeline`
- master branch graph에서 DASH normal, wall-groggy, part-break authoring path를 resolve
- 선택 Effect의 stage-global Product cue offset과 anchor/follow/stop 계약을 전체 timeline에서 유지
- donut `SERVER_PATTERN_STAGE`와 axe `SERVER_COMBAT_OBJECT` 독립 재생 경로를 분리
- 기존 `Play Server Pattern` transaction 유지

### G5. Animation Tool authoring consumer

- 기존 1~67 `Valtan.patternpreview.json`/`Valtan.clipseq` 화면은 source reference로 유지
- 별도의 primary `Valtan Pattern Master` 화면에서 admission된 7개 pattern을 표시
- master stage/occurrence wall budget으로 전체 body-animation timeline 재생·seek
- DASH normal, wall-groggy, part-break 경로를 master branch graph에서 resolve
- ordered `presentationSources`와 stage `endPolicy`를 표시

### G6. 팀 인계와 표준 검증

- `.md/TEAM/발탄인수인계서.md`에 세 담당과 UI 경계를 기록
- 팀 README와 gameplay handbook에서 새 정본으로 연결
- Client project `96.DataFiles/Valtan`에 master 노출
- 표준 Build/Regression에서 master Validate를 선행

## 불변식

- Server bootstrap에는 animation clip, Effect asset, Client visual 또는 camera 좌표가 들어가지 않는다.
- runtime은 master와 기존 제품 JSON을 동시에 읽지 않는다.
- finite repeat와 stage remainder loop는 서로 다른 필드다.
- stage/action/clip/effect occurrence는 stable ID로 join한다.
- master publish 실패는 기존 제품 문서를 부분 교체하지 않는다.
- UI damage font는 Server `DAMAGE_EVENT.iAmount`만 표시한다.
- camera/world placement 실데이터는 기존 전용 문서에만 둔다.

## 자동 검증

```text
Valtan master focused tests
Project-ValtanPatternMaster -Mode Validate
Gameplay publisher Validate/Publish
Effect publisher Validate
World destruction publisher Validate
EffectPipeline focused Python tests
Server --contract-test
Client x64 Debug/Release build
git diff --check
JSON/XML parse
```

## 사용자 수동 검증

사용자가 Effect Tool `All Effects -> Valtan`에서 independent axe/donut의 단일 노출, 일곱 pattern 전체
animation timeline, Revolution 회전, anchor 이동과 실제 Server Pattern을 확인한다. 에이전트는 화면 결과를 대신
PASS 처리하지 않는다.
