# Monster·Boss Animation Preview 구현 결과

## 1. 완료 상태

기존 Character Select의 Debug Animation Tool 경로에 일반 몬스터 3종, 통솔자 루가루, 발탄 본체를 playback-only target으로 추가했다.

이 변경은 모델과 애니메이션을 육안 검증하기 위한 Debug authoring 기능이다. Monster 제품 runtime, Server spawn·AI, MapTool placement와 World Gameplay 계약은 추가하지 않았다.

| 분류 | Preview ID | Tool 표시 이름 | Animation |
|---|---|---|---:|
| 일반 몬스터 | `monster.480001.mn-padd-01` | `[Monster] Normal 01 - MN_PADD_01` | 36 |
| 일반 몬스터 | `monster.480002.mn-sjfc-00-4` | `[Monster] Normal 02 - MN_SJFC_00_4` | 29 |
| 일반 몬스터 | `monster.480003.mn-0019-05` | `[Monster] Normal 03 - MN_0019_05` | 25 |
| 미니보스 | `monster.480005.lugaru` | `[Mid Boss] Commander Lugaru - MN_RPRS_02` | 91 |
| 보스 | `boss.valtan` | `[Boss] Valtan - MN_RPBF_01` | 27 |

원본 DB 추적 근거인 물리 폴더의 `NPC_` 이름은 바꾸지 않았다. NPC와 전투 몬스터가 혼동되지 않도록 Preview ID, asset name, Tool 표시 이름에서만 Monster, Mid Boss, Boss 역할을 명시했다.

## 2. 구현 내용

### 2.1 Preview descriptor

`Client/Public/AnimationPreviewAssets.h`의 기존 descriptor에 다음 값을 추가했다.

- 모델별 preview scale
- 모델별 yaw
- playback-only 여부
- 선택적인 Boss archetype 검증 ID

일반 몬스터와 루가루는 ActorX cook 계약에 맞춰 scale `0.01`, yaw `-90`을 사용한다. 발탄은 기존 제품 본체 모델 계약에 맞춰 scale `0.0001`, yaw `-90`을 사용한다.

### 2.2 기존 CModel 경로 재사용

`Client/Private/Loader.cpp::Ready_AnimationPreviewModels`는 각 descriptor의 transform으로 기존 `CModel::Create -> Add_Prototype` 경로를 그대로 사용한다.

발탄 descriptor는 `BOSS_VALTAN`을 `CActorCatalog`에서 조회해 `Data/Actors/BossCatalog.json`의 `bodyModel`과 preview 모델 경로가 다르면 로드를 거부한다. 현재 정본은 다음과 일치한다.

```text
Character/Valtan/MN_RPBF_01.wmodel
```

선택적 Monster preview 리소스가 없는 팀원 환경에서는 해당 prototype만 건너뛴다. 리소스 누락 때문에 Character Select 전체 진입이 실패하지 않는다.

### 2.3 Playback-only 보호

`Client/Private/Animation_Tool.cpp::Render`는 `bPlaybackOnly` target을 선택하면 다음 기능만 제공한다.

- clip 이름 필터
- clip 선택
- Play, Pause, Loop, frame scrub

그 지점에서 즉시 반환하므로 다음 제품·저작 데이터 경로를 읽거나 저장하지 않는다.

- `Data/Animation/Authored`
- skill binding
- animation event
- effect transfer
- hit event

Monster/Boss target은 `bPlayableClassBody=false`이므로 Effect Tool의 playable character 목록에도 추가되지 않는다.

## 3. 변경 파일

- `Client/Public/AnimationPreviewAssets.h`
- `Client/Private/Loader.cpp`
- `Client/Private/Animation_Tool.cpp`
- `.md/GB/08-06/2026-08-06_MONSTER_BOSS_ANIMATION_PREVIEW_PLAN.md`
- `.md/GB/08-06/2026-08-06_MONSTER_BOSS_ANIMATION_PREVIEW_RESULT.md`

Engine, Shared, Server, MapTool, Navigation, World Gameplay, Level enum, Lobby command, `CCharacter`, `CValtan`, `BossCatalog.json`은 수정하지 않았다.

## 4. 자동 검증

### 4.1 WModel metadata와 정본 경로

`ModelAssetConverter info` 결과는 다음과 같다.

```text
480001: sections=39 animations=36 skeleton=yes
480002: sections=32 animations=29 skeleton=yes
480003: sections=28 animations=25 skeleton=yes
480005: sections=94 animations=91 skeleton=yes
Valtan: sections=30 animations=27 skeleton=yes
```

다섯 모델 파일 존재, preview ID 존재, `BOSS_VALTAN` body 경로 일치, playback-only 분기를 함께 검사해 `STATIC_PREVIEW_CONTRACT_PASS`를 확인했다.

### 4.2 빌드

- Client x64 Debug: PASS
- Client x64 Release: PASS

두 빌드 모두 Engine과 Shared 의존 프로젝트를 포함해 완료됐다. Release에서도 Debug authoring 변경 때문에 제품 빌드가 깨지지 않는다.

### 4.3 저장소 검사

- `git diff --check`: PASS
- `Tools/ProjectAudit/Invoke-ProjectAudit.ps1`: 기존 저장소 문제 4건으로 FAIL

ProjectAudit가 보고한 항목은 다음과 같다.

1. `projects.data-source-visibility`: expected 217, project/filters 215
2. `effect.g09-authoring-world-runtime-boundary`
3. `actors.catalog-assets`: `Character/DimensionMaster/DimensionMaster_Character.wmodel` 누락
4. `actors.dimensionmaster-runtime-animation`
이번 Preview 변경 파일은 위 네 검사 대상의 project 등록, Effect G09, DimensionMaster asset을 수정하지 않았다. 따라서 전역 감사 실패를 Preview 구현 완료로 숨기지 않고 기존·병행 작업 문제로 분리한다.

## 5. 수동 검증 절차와 남은 상태

### 5.1 Character Select 진입 실패 원인과 수정

초기 구현은 `Ready_AnimationPreviewModels`에서 플레이 가능한 다섯 class prototype을 전부 선로딩했다. 현재 로컬 runtime 입력에는 `Character/DimensionMaster/DimensionMaster_Character.wmodel`이 없으므로 DimensionMaster prototype 생성이 실패했고, Character Select loader 전체가 실패하여 Loading Level이 Lobby로 rollback했다.

Character Select에는 class를 선택할 때 `CPlayableCharacterAssetService::Ensure_Prototypes`를 호출하는 기존 lazy-load 계약이 이미 있다. 따라서 Animation Preview loader의 다섯 class 강제 선로딩을 제거하고, 플레이 가능한 class는 기존 Character Select lazy-load만 사용하도록 복구했다. Monster/Boss authoring preview는 optional 입력으로 유지하여 개별 WModel을 만들 수 없더라도 Character Select 진입 전체를 실패시키지 않는다. 잘못된 descriptor, BossCatalog 불일치, prototype 등록 실패는 여전히 명시적으로 실패한다.

### 5.2 실행 검증

Client Debug를 `Client/Default` 작업 디렉터리에서 실행하고 다음 순서로 수동 화면 검증했다.

```text
Client Debug 실행
-> Lobby
-> Character Select
-> F1
-> Animation Tool
-> Target 목록에서 [Monster], [Mid Boss], [Boss] 항목 선택
-> clip 선택과 Play/Pause/Loop/scrub 확인
-> target 교체와 Character Select 이탈 후 정리 확인
```

검증 결과 Character Select 선택 뒤 Lobby로 rollback하지 않았고, F1 Animation Tool에서 일반 몬스터 3종, 루가루, 발탄 target과 clip 목록이 표시됐다. 루가루 target은 91개 clip 중 첫 clip이 재생되며 frame 값이 진행되는 것까지 확인했다.

현재 누락된 DimensionMaster 본체 WModel을 선택하는 경우 해당 class preview만 로드하지 못할 수 있다. 이 누락은 ProjectAudit의 기존 `actors.catalog-assets` 실패로 계속 보고되지만, 다른 class의 Character Select 진입과 Monster/Boss preview 전체를 함께 종료시키지는 않는다.

발탄 target은 현재 `BossCatalog`의 본체 WModel만 검증한다. 무기까지 결합한 `CValtan` 전체 제품 presentation 검증은 이 playback-only preview 범위가 아니다.

## 6. 후속 경계

이 결과로 확인할 수 있는 것은 다섯 WModel의 모델·스켈레톤·clip 재생 상태다. 다음 항목은 별도 수직 슬라이스와 Server 계약이 필요하다.

- 일반 몬스터와 루가루의 제품 archetype/catalog
- Server authority spawn, 이동, 전투 AI, stage trigger 연동
- Client snapshot 기반 presentation
- MapTool authoring과 World publisher 지원
- 실제 발탄 무기 결합 presentation 검증

Preview가 정상이라는 이유만으로 Monster 제품 runtime이 완료된 것으로 처리하지 않는다.
