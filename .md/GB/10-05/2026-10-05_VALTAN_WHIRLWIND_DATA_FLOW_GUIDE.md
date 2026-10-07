# 발탄 휠윈드 JSON과 C++ 실행 흐름

이 문서는 실제 저장된 `VALTAN_WHIRLWIND`를 기준으로, 시퀀서에서 편집한 값이 어떤 파일에 저장되고 어떤 publisher를 거쳐 서버 판정과 클라이언트 화면으로 이어지는지 설명한다. 코드 옆에 열어 두고 링크의 함수와 JSON 행을 순서대로 읽는 용도다. 구현 계획서나 코드 수정안이 아니다.

**핵심은 JSON 자체가 실행되는 것이 아니라는 점이다.** 저작 데이터는 검증과 게시를 거쳐 정의 데이터가 되고, C++가 그 정의와 현재 실행 상태를 결합하여 매 Tick 판정한다. 클라이언트는 서버의 패턴 실행 상태와 별도의 연출 정의를 결합하여 애니메이션과 이펙트를 재생한다.

조사 기준은 2026-10-05, `codex/unreal-render-optimization`, HEAD `c579608efcc31210f29891798c19b351c3edc638`의 디스크 저장본이다. 아래 Tick 예제는 코드로 계산한 시나리오이며 실행 로그가 아니다. 실행 중 Server의 활성 hot-reload revision, 미저장 에디터 draft, 실제 GPU 화면은 이번 조사에서 확인하지 않았다.

여기서 '전체 의존성'은 이 패턴의 편집 → 게시 → 로드 → 판정 → 복제 → 연출 → draw를 연결하는 데이터와 직접 소비자다. 표준 라이브러리의 모든 include, 무관한 다른 보스 패턴, 모든 재질의 전체 리소스 그래프까지 복제한다는 뜻은 아니다. JSON/C++ 발췌에는 원문 위치를 붙였으며 긴 함수 전체를 새 구현 코드처럼 제시하지 않는다.

## G01 실제로 열어야 하는 파일

### 파일의 역할부터 구분하기

| 파일 | 무엇을 소유하는가 | 언제 읽는가 |
|---|---|---|
| [Data/Valtan/Valtan.gameplay.json](C:/Users/tnest/Desktop/LostArk/Data/Valtan/Valtan.gameplay.json:681) | 패턴 선택 조건, 단계 순서, 시간, 타격 형상과 횟수, 이동과 분기 | 편집/검증/게시 |
| [Data/Valtan/Valtan.presentation.json](C:/Users/tnest/Desktop/LostArk/Data/Valtan/Valtan.presentation.json:115) | 같은 패턴·단계의 애니메이션 occurrence, 이펙트 cue, 카메라 | 편집/검증/게시 |
| [Data/Valtan/Valtan.pattern.json](C:/Users/tnest/Desktop/LostArk/Data/Valtan/Valtan.pattern.json:1) | 구형 통합 형식의 migration fixture | 명시적인 마이그레이션 경로. 현재 제품 정본이 아님 |
| [Data/Encounters/Valtan/ValtanEncounter.json](C:/Users/tnest/Desktop/LostArk/Data/Encounters/Valtan/ValtanEncounter.json:1939) | 서버용으로 투영된 패턴 정의 | gameplay publisher |
| [Data/Encounters/Valtan/ValtanPatternRotations.json](C:/Users/tnest/Desktop/LostArk/Data/Encounters/Valtan/ValtanPatternRotations.json:1) | HP 구간별 일반 패턴 순서와 선택 정책 | gameplay publisher |
| [Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap](C:/Users/tnest/Desktop/LostArk/Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap:105890) | 서버가 실제 파싱하는 게시 패키지 | 서버 catalog 로드/승인된 generation 교체 |
| [Data/Animation/Authored/Valtan/Valtan.patternbindings.json](C:/Users/tnest/Desktop/LostArk/Data/Animation/Authored/Valtan/Valtan.patternbindings.json:1) | action ID에서 애니메이션 재생 체인으로 가는 게시 정의 | 클라이언트 presentation 로드 |
| [Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json](C:/Users/tnest/Desktop/LostArk/Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json:988) | pattern/action/clip occurrence에서 effect cue로 가는 게시 정의 | 클라이언트 presentation 로드 |
| [Data/Balance/DamageProfiles.json](C:/Users/tnest/Desktop/LostArk/Data/Balance/DamageProfiles.json:170) | `damage.valtan.circular-spin` 피해 배율 | 게시 후 서버 catalog |
| [Data/Balance/Profiles/Retail.balanceprofile.json](C:/Users/tnest/Desktop/LostArk/Data/Balance/Profiles/Retail.balanceprofile.json:1784) | 제품 밸런스 override. base BossProfiles와 구분 | gameplay publisher |
| [Data/Effects/Authored/effect.valtan.carrier-v1.attack.whirlwind.recovery.clip-01.effect.json](C:/Users/tnest/Desktop/LostArk/Data/Effects/Authored/effect.valtan.carrier-v1.attack.whirlwind.recovery.clip-01.effect.json:1) | 실제 시각 이펙트의 구성 요소, 재질, 리소스와 수명 | effect 준비/캐시 |
| [Data/Actors/BossCatalog.json](C:/Users/tnest/Desktop/LostArk/Data/Actors/BossCatalog.json:1) | 보스 presentation 모델과 부품의 asset ID | 클라이언트 actor/presentation 준비 |

`Valtan.gameplay.json`과 `Valtan.presentation.json`의 패턴은 각각 44개이고, 게시된 Encounter에는 호환/제품 투영을 포함하여 67개가 있다. 이름이 비슷하다고 파일을 서로 대체해서 읽으면 안 된다. 구형 `Valtan.pattern.json`의 7개 패턴만 읽으면 현재 제품 전체를 설명할 수 없다.

### 정의와 실행 상태를 구분하기

| 종류 | 휠윈드 예 | 소유자 |
|---|---|---|
| 패턴 정의 | SPIN은 1200ms, 반경 10m, 네 번 타격 | 저작 JSON → 게시 패키지 → `CGameplayCatalog` |
| 보스 실행 인스턴스 | 이 발탄은 지금 SPIN, 이번 실행 sequence는 42, 한 번 타격 완료 | Server `SERVER_WORLD_ENTITY` |
| 연출 정의 | SPIN은 `mesh_att_battle_20_03`, 이 effect cue를 사용 | Client animation/cue document |
| 연출 실행 인스턴스 | 이 clip은 현재 162.9ms 지점, 이 effect particle은 현재 age 0.36초 | `CModel`, `CEffectObject`, `CEffectPlayback` |
| GPU 입력 | 이번 draw의 world/bone 행렬, 색, DDS SRV, vertex/index buffer | C++ renderer와 D3D11 |

JSON의 배열 원소 하나가 화면의 픽셀 하나가 되는 것이 아니다. 같은 정의를 여러 실행 인스턴스가 참조할 수 있고, 그 실행 인스턴스의 현재 상태가 GPU 입력을 만든다.

## G02 저장된 휠윈드 패턴을 그대로 읽기

### Gameplay 원문

다음은 패턴 객체 하나의 원문이다. 마지막 쉼표는 이 객체가 `patterns` 배열의 한 원소이기 때문이다.

[Data/Valtan/Valtan.gameplay.json](C:/Users/tnest/Desktop/LostArk/Data/Valtan/Valtan.gameplay.json:680) 원문 발췌:

```json
    {
      "patternId": "VALTAN_WHIRLWIND",
      "displayName": "휠윈드 1.2초",
      "category": "NORMAL",
      "compatibilitySelectionWeight": 20,
      "actionId": "valtan.attack.whirlwind",
      "entryActionId": "valtan.attack.whirlwind.windup",
      "targetPolicy": "NEAREST_EACH_TICK",
      "aimPolicy": "TRACK_TARGET_EACH_TICK",
      "eligibility": {
        "armorRequirement": "ANY",
        "phaseRequirement": "ANY",
        "minimumGameplayPhase": 1,
        "maximumGameplayPhase": 3,
        "minimumHealthBarInclusive": 1,
        "maximumHealthBarInclusive": 160,
        "minimumRangeM": 0.0,
        "maximumRangeM": 12.0,
        "cooldownPolicy": "DERIVED_SOURCE_ACTION",
        "selectionCooldownMs": null,
        "cooldownGroupId": null,
        "repeatPolicy": {
          "kind": "SOFT_AVOID_UNLESS_ONLY_ELIGIBLE",
          "limit": 2
        }
      },
      "invulnerableWhileRunning": false,
      "sourceActionIds": [
        420633
      ],
      "serverMotion": null,
      "reactions": [],
      "stages": [
        {
          "stageId": "WINDUP",
          "actionId": "valtan.attack.whirlwind.windup",
          "stageKind": "WINDUP",
          "durationMs": 1333,
          "defaultNextActionId": "valtan.attack.whirlwind.active",
          "hit": {
            "shape": {
              "kind": "NONE"
            }
          },
          "motion": null,
          "events": [],
          "branches": []
        },
        {
          "stageId": "SPIN",
          "actionId": "valtan.attack.whirlwind.active",
          "stageKind": "ACTIVE",
          "durationMs": 1200,
          "defaultNextActionId": "valtan.attack.whirlwind.recovery",
          "hit": {
            "shape": {
              "kind": "CIRCLE",
              "outerRadiusM": 10.0
            },
            "schedule": {
              "kind": "INTERVAL",
              "count": 4,
              "firstOffsetMs": 0,
              "intervalMs": 350
            },
            "serverDamageProfileId": "damage.valtan.circular-spin",
            "pushRangeM": 0.0,
            "pushMs": 0,
            "knockdown": false,
            "downMs": 0
          },
          "motion": null,
          "events": [],
          "branches": []
        },
        {
          "stageId": "RECOVERY",
          "actionId": "valtan.attack.whirlwind.recovery",
          "stageKind": "RECOVERY",
          "durationMs": 1467,
          "defaultNextActionId": null,
          "hit": {
            "shape": {
              "kind": "NONE"
            }
          },
          "motion": null,
          "events": [],
          "branches": []
        }
      ]
    },
```


### 각 값이 실제로 뜻하는 것

- `patternId`는 패턴 정의의 stable ID다. `displayName`은 편집 화면의 이름이며 lookup key가 아니다.
- `entryActionId`는 패턴을 시작할 때 들어갈 단계의 action을 식별한다. `stageId`는 패턴 내부 단계 이름, `actionId`는 게시/런타임/연출을 연결하는 행동 ID다.
- `NEAREST_EACH_TICK`와 `TRACK_TARGET_EACH_TICK`은 실행 중 가장 가까운 대상을 다시 고르고 방향을 갱신하는 정책이다. 이것만으로 타깃에게 이동하라는 뜻은 아니다.
- `eligibility.maximumRangeM=12`는 패턴 선택 조건이다. `outerRadiusM=10`은 실제 타격 형상이다. 선택 거리와 공격 범위는 서로 다르다.
- `durationMs`는 각 단계의 게임 시간이다. WINDUP 1333 + SPIN 1200 + RECOVERY 1467 = 저작값 합계 4000ms다. 실제 서버 평가 시각은 30Hz 양자화 규칙을 따로 봐야 한다.
- `defaultNextActionId`는 정상 종료 시 이동할 다음 행동이다. 마지막 `null`은 패턴을 끝낸다. 게시 시 TIMEOUT branch로 변환된다.
- `shape.kind=NONE`인 WINDUP과 RECOVERY에는 이 stage hit가 없다. SPIN만 원형 타격을 갖는다.
- `firstOffsetMs + n * intervalMs`로 0, 350, 700, 1050ms의 네 pulse가 만들어진다. 이것은 네 명에게 한 번씩 때린다는 의미가 아니다.
- `serverDamageProfileId`는 피해 숫자 자체가 아니라 피해 공식에 넣을 프로필의 ID다.
- 이 패턴의 `serverMotion`, stage `motion`, `events`, `branches`가 비어 있다는 사실도 중요하다. stage event로 장판/투사체를 생성하는 다른 패턴의 흐름을 이 예에 임의로 더하면 안 된다.
- `sourceActionIds=[420633]`는 원본 출처와 파생 정책을 연결하는 ID다. 코드 포인터나 animation index가 아니다.
- `compatibilitySelectionWeight=20`만 보고 현재 일반 선택을 가중치 추첨이라고 설명하면 틀린다. 현재 상위 결정 모드는 `HEALTH_BAR_ROTATIONS`, 각 구간 일반 패턴은 `ORDERED_LOOP`다.

### 패턴 flow에는 두 층이 있다

**다음에 어떤 패턴을 고를까**는 [decisionModel](C:/Users/tnest/Desktop/LostArk/Data/Valtan/Valtan.gameplay.json:35)과 [HP 구간별 rotation](C:/Users/tnest/Desktop/LostArk/Data/Valtan/Valtan.gameplay.json:185)이 결정한다. 서버는 HP 기믹 예약을 먼저 보고, 일반 순환의 현재 ordinal과 실행 가능 조건을 확인한다.

**고른 패턴 내부에서 다음에 무엇을 할까**는 이 객체의 stage/action/branch가 결정한다.

```text
전투 전체: HP 구간 / 예약 기믹 / rotation cursor
                         |
                         v
             VALTAN_WHIRLWIND 선택
                         |
                         v
 WINDUP 1333ms -> SPIN 1200ms -> RECOVERY 1467ms -> 완료
                   |
                   +-> 타격 pulse 4개
```

시퀀서의 stage를 편집하는 것과 보스가 휠윈드를 몇 번째 일반 패턴으로 선택하는 것을 편집하는 것은 별개의 데이터다.

## G03 Presentation은 같은 단계를 어떻게 보여 주는가

[Data/Valtan/Valtan.presentation.json](C:/Users/tnest/Desktop/LostArk/Data/Valtan/Valtan.presentation.json:147) 원문 발췌:

```json
        {
          "stageId": "SPIN",
          "actionId": "valtan.attack.whirlwind.active",
          "sequenceRole": "SPIN",
          "animation": {
            "endPolicy": "EXACT",
            "repeatCount": 1,
            "occurrences": [
              {
                "clipOccurrenceId": "valtan.attack.whirlwind.active.clip.01",
                "clip": "mesh_att_battle_20_03",
                "mappingBasis": "PROJECT_AUTHORED",
                "sourceStartMs": 0,
                "playMs": 533,
                "playRate": 0.4441666667,
                "repeatUntilStageEnd": false
              }
            ]
          },
          "effectCues": [
            {
              "cueId": "cue.valtan.carrier-v1.attack.whirlwind.recovery.clip-01",
              "scalePolicy": {
                "kind": "GAMEPLAY_FOOTPRINT",
                "worldScale": [
                  1.5,
                  1.5,
                  1.5
                ]
              },
              "occurrenceId": "cue.valtan.carrier-v1.attack.whirlwind.recovery.clip-01.occurrence.01",
              "effectAssetId": "effect.valtan.carrier-v1.attack.whirlwind.recovery.clip-01",
              "clipOccurrenceId": "valtan.attack.whirlwind.active.clip.01",
              "sourceStartMs": 0,
              "sourceEndMs": null,
              "anchorSlotId": "root",
              "followPolicy": "follow",
              "stopPolicy": "natural",
              "repeatPolicy": "once",
              "localTransform": {
                "position": [
                  0,
                  0,
                  0
                ],
                "rotationDegrees": [
                  0,
                  0,
                  0
                ],
                "scale": [
                  1,
                  1,
                  1
                ]
              },
              "mappingBasis": "PROJECT_AUTHORED"
            }
          ],
          "cameraInvocations": []
        },
```


### 시간과 occurrence의 의미

`clip`은 모델의 animation 이름이다. `clipOccurrenceId`는 같은 clip을 타임라인에 여러 번 놓아도 각 배치를 구분하는 ID다. `cueId`와 `occurrenceId`도 이펙트 정의와 그 타임라인 배치를 구분한다.

SPIN은 원본 animation의 533ms 구간을 0.4441666667배 속도로 재생한다. 따라서 벽시계 재생 길이는 대략 `533 / 0.4441666667 = 1200ms`다. `playMs=533`을 '게임에서 533ms 동안 재생'이라고 읽으면 안 된다.

단계 age가 600ms라면 이 clip의 source sample은 `0 + 600 * 0.4441666667 = 266.5ms`다. 같은 방식으로 원본 수백 ms의 동작을 게임 단계 1.2초에 맞춘다. 실제 변환은 [ActionPresentationTimeline](C:/Users/tnest/Desktop/LostArk/Client/Private/ActionPresentationTimeline.cpp:43)을 따라 읽는다.

이 cue의 이름에는 `recovery`가 있지만, 현재 저장 위치는 **SPIN**이고 `clipOccurrenceId`도 active clip을 가리킨다. ID의 영어 이름을 추측해서 실행 단계를 정하지 말고 소유 stage와 참조를 확인해야 한다.

`anchorSlotId=root`는 발탄의 presentation root 기준 배치, `follow`는 추적, `once`는 해당 occurrence에서 한 번 실행, `natural`은 effect 자체의 수명 종료 정책이다. `worldScale=[1.5,1.5,1.5]`는 시각 배율이다. 이것이 서버의 반경 10m를 매 프레임 다시 계산해서 덮어쓰는 값은 아니다.

WINDUP은 `mesh_att_battle_20_02`, RECOVERY는 `mesh_att_battle_20_04`를 사용하고 두 단계의 animation end policy는 `LOOP_TO_STAGE_END`다. 이 예의 camera invocation은 비어 있다. 같은 타임라인이라도 카메라 연출이 항상 실행되는 것은 아니다.

## G04 Preview와 Append와 Save와 Publish

### 도구 안에서 일어나는 일

[Client/Private/SequencerTool.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/SequencerTool.cpp:1630)의 Preview와 Append 명령부터 보면 된다. Preview는 선택된 리소스를 저작용 시간으로 미리 보는 동작이다. 실제 서버의 HP/피해 판정 실행과 같지 않다.

[Client/Private/BalanceTool.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/BalanceTool.cpp:3608)의 Append는 리소스를 패턴의 animation occurrence로 추가한다. 일반 기존 gameplay 패턴에는 마지막 stage의 clip 목록에 붙이고 길이를 조정하는 경로가 있으며, 수동 audition 패턴에는 stage와 clip occurrence를 만드는 경로가 있다. **Append를 누르면 언제나 새 stage가 생긴다**고 일반화하면 안 된다. 모델 내부 원본 animation 데이터를 덧붙여 수정하는 기능도 아니다.

[Client/Private/ValtanActionWorkbench.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/ValtanActionWorkbench.cpp:12270)의 Save는 `m_bPublishWithPendingSave=false`, [Client/Private/ValtanActionWorkbench.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/ValtanActionWorkbench.cpp:12353)의 Save & Publish는 이를 true로 설정한다. 둘 다 save request를 만들지만 후속 게시 여부가 다르다.

### Publish는 bat 파일인가

이 흐름의 본체는 **C++ 도구 UI → PowerShell .ps1 → Python 변환기 → 게시 파일**이다. Publish라는 확장자의 파일이나 단일 .bat 기능이 아니다.

현재 Composition Save는 [Client/Private/ValtanActionWorkbench.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/ValtanActionWorkbench.cpp:6894)에서 `Begin_ValtanCompositionSave`를 요청한다. [Client/Private/BalanceTool.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/BalanceTool.cpp:5867)의 save job이 draft를 검증하고 정본 저장과 receipt를 처리한다. [Tools/ValtanPipeline/Publish-ValtanTuningRuntimeSet.ps1](C:/Users/tnest/Desktop/LostArk/Tools/ValtanPipeline/Publish-ValtanTuningRuntimeSet.ps1:1)이 Python 변환기의 진입점이다.

정본을 저장한 뒤 동일 revision의 editor graph를 다시 연 것이 확인되어야 [Client/Private/ValtanActionWorkbench.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/ValtanActionWorkbench.cpp:7232)에서 full runtime publish가 시작된다. [Client/Private/BalanceTool.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/BalanceTool.cpp:5677)의 `Publish_ServerRuntimeSet`은 source SHA-256 일치를 검사하고 다음 인자를 만든다.

```text
-DataOnly -ExpectedValtanSourceRevision <저장 완료한 source SHA-256>
```

[Tools/Build/Run-FullPipeline.ps1](C:/Users/tnest/Desktop/LostArk/Tools/Build/Run-FullPipeline.ps1:165)은 먼저 `Project-ValtanPatternMaster.ps1 -Mode PublishV2`를 실행하고, DataOnly에서는 manifest의 데이터 domain publisher들을 실행한다. **DataOnly 게시와 C++ compile/link는 같은 작업이 아니다.** 또한 full DataOnly는 이름 그대로 발탄 파일 한 개만 복사하는 버튼도 아니다.

### JSON이 실제 제품 파일로 바뀌는 순서

1. [join_v2_authoring](C:/Users/tnest/Desktop/LostArk/Tools/ValtanPipeline/valtan_tuning_pipeline.py:6375)이 gameplay/presentation을 읽어 pattern ID, stage ID, action ID의 일치를 검증한다. 그 다음 gameplay와 animation/effect/camera 정보를 결합한 내부 표현을 만든다.
2. [build_repository_product_projection](C:/Users/tnest/Desktop/LostArk/Tools/ValtanPipeline/valtan_tuning_pipeline.py:7860)이 Encounter, Rotations, animation binding/cue, combat object/world event 및 관련 제품 문서를 만든다.
3. [PublishV2](C:/Users/tnest/Desktop/LostArk/Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1:556)는 임시 product root에 먼저 생성한다. source manifest가 도중에 바뀌었는지 확인한 뒤 transaction으로 게시한다. 단순 무조건 덮어쓰기와 다르다.
4. [Tools/GameplayPipeline/Publish-GameplayBalance.ps1](C:/Users/tnest/Desktop/LostArk/Tools/GameplayPipeline/Publish-GameplayBalance.ps1:2027)이 게시된 Encounter와 밸런스 입력 등을 읽는다. [Tools/GameplayPipeline/Publish-GameplayBalance.ps1](C:/Users/tnest/Desktop/LostArk/Tools/GameplayPipeline/Publish-GameplayBalance.ps1:2807)에서 stage를 탭으로 구분한 행으로 변환한다.
5. [Tools/GameplayPipeline/Publish-GameplayBalance.ps1](C:/Users/tnest/Desktop/LostArk/Tools/GameplayPipeline/Publish-GameplayBalance.ps1:5868)의 게시 경로가 준비된 bootstrap을 교체한다.

저장 성공, 디스크 게시 성공, 실행 중 Server가 새 generation을 승인한 상태, Client가 그 generation의 presentation을 준비한 상태는 네 가지 다른 사실이다. Save & Publish가 성공했다는 이유만으로 실행 중 서버까지 자동으로 바뀌었다고 설명하면 안 된다. 활성 revision이 맞아야 Server Play가 그 데이터로 실행된다.

## G05 서버가 실제 읽는 것은 무엇인가

### 실제 게시 행

아래는 JSON이 아니라 **탭 구분 텍스트**다. 화면에서는 공백처럼 보일 수 있다.

[Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap](C:/Users/tnest/Desktop/LostArk/Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap:105890) 원문 발췌:

```text
PATTERNSTAGE	ENCOUNTER_VALTAN	VALTAN_WHIRLWIND	0	WINDUP	valtan.attack.whirlwind.windup	WINDUP	1333	NONE	0	0	0	0	0	0	0	0	-	0	0	0	0
PATTERNSTAGE	ENCOUNTER_VALTAN	VALTAN_WHIRLWIND	1	SPIN	valtan.attack.whirlwind.active	ACTIVE	1200	CIRCLE	10	0	0	0	0	4	350	0	damage.valtan.circular-spin	0	0	0	0
PATTERNSTAGE	ENCOUNTER_VALTAN	VALTAN_WHIRLWIND	2	RECOVERY	valtan.attack.whirlwind.recovery	RECOVERY	1467	NONE	0	0	0	0	0	0	0	0	-	0	0	0	0
```


열을 세어 보면 SPIN의 `fields[7]=1200`, `fields[8]=CIRCLE`, `fields[9]=10`, `fields[14]=4`, `fields[15]=350`, `fields[16]=0`, `fields[17]=damage.valtan.circular-spin`이다. 이 숫자들이 C++ stage struct의 필드로 들어간다.

[Server/Private/GameplayCatalog.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/GameplayCatalog.cpp:4885) 원문 발췌:

```cpp
		else if (!fields.empty() && "PATTERNSTAGE" == fields[0])
		{
			BOSS_PATTERN_STAGE_DEFINITION stage{};
			std::uint32_t stageIndex = 0u;
			std::uint32_t knockdownFlag = 0u;
			const bool hasPlayerResponseFields = 24u == fields.size();
			if ((22u != fields.size() && !hasPlayerResponseFields) ||
				!IsStableId(fields[1]) ||
				!IsStableId(fields[2]) ||
				!ParseNumber(fields[3], stageIndex) ||
				!IsStableId(fields[4]) || !IsStableId(fields[5]) ||
				!ParseBossPatternStageKind(fields[6], stage.eStageKind) ||
				!ParseNumber(fields[7], stage.iDurationMs) ||
				!ParseBossPatternHitShape(fields[8], stage.eHitShape) ||
				!ParseNumber(fields[9], stage.fHitOuterRadius) ||
				!ParseNumber(fields[10], stage.fHitInnerRadius) ||
				!ParseNumber(fields[11], stage.fHitAngleDegrees) ||
				!ParseNumber(fields[12], stage.fHitLength) ||
				!ParseNumber(fields[13], stage.fHitHalfWidth) ||
				!ParseNumber(fields[14], stage.iHitCount) ||
				!ParseNumber(fields[15], stage.iHitIntervalMs) ||
				!ParseNumber(fields[16], stage.iHitDelayMs) ||
				("-" != fields[17] && !IsStableId(fields[17])) ||
				!ParseNumber(fields[18], stage.fPushRangeM) ||
				!ParseNumber(fields[19], stage.iPushMs) ||
				!ParseNumber(fields[20], knockdownFlag) ||
				!ParseNumber(fields[21], stage.iDownMs) ||
```


앞의 발췌 다음에는 유한 수, 양수 duration, shape와 delay의 일관성, knockdown/downMs 등의 검증이 이어진다. 소유 encounter/pattern과 stage 순서도 확인하고 성공한 stage만 해당 pattern의 `Stages`에 넣는다. 단순히 문자열을 숫자로 바꾸면 끝나는 파서가 아니다.

[Server/Private/GameplayCatalog.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/GameplayCatalog.cpp:1421) 원문 발췌:

```cpp
bool LostArk::Server::CGameplayCatalog::Load()
{
	const std::filesystem::path dataRoot = Resolve_DataRoot();
	if (dataRoot.empty())
	{
		m_strStatus = "Gameplay data root could not be resolved";
		return false;
	}
	CGameplayCatalog staged;
	if (!staged.Load_BootstrapPath(dataRoot / L"Gameplay" / L"Gameplay.bootstrap", nullptr, nullptr))
	{ m_strStatus = staged.Get_Status(); return false; }
	const auto receiptPath = dataRoot / L"Gameplay" / L"NumericBalance.active.json";
	std::error_code error;
	const bool hasReceipt = std::filesystem::exists(receiptPath, error);
	if (error) { m_strStatus = "Numeric balance receipt cannot be inspected"; return false; }
```


`CGameplayCatalog staged`에 먼저 읽는 이유는 반쯤 읽은 데이터를 현재 전투 catalog로 노출하지 않기 위해서다. receipt 검증과 나머지 로드가 성공한 뒤 교체한다. `NumericBalance.active.json`은 게시 identity를 확인하는 receipt이지, 저작 JSON을 대신 읽는 gameplay fallback이 아니다.

### C++ 메모리 구조

- [Server/Public/GameplayCatalog.h](C:/Users/tnest/Desktop/LostArk/Server/Public/GameplayCatalog.h:1346)의 `BOSS_PATTERN_DEFINITION`: 패턴 ID, 선택/대상 정책, stage 정의 목록.
- [Server/Public/GameplayCatalog.h](C:/Users/tnest/Desktop/LostArk/Server/Public/GameplayCatalog.h:1014)의 `BOSS_PATTERN_STAGE_DEFINITION`: stage/action ID, duration, hit shape와 schedule, 피해 프로필, motion/action/branch.
- [Server/Public/GameplayCatalog.h](C:/Users/tnest/Desktop/LostArk/Server/Public/GameplayCatalog.h:1751)의 `m_BossPatterns`: encounter를 통해 패턴 정의들을 찾는 catalog 저장소.
- [Server/Public/ServerWorldEntity.h](C:/Users/tnest/Desktop/LostArk/Server/Public/ServerWorldEntity.h:356)의 실행 상태: 현재 stage index, 첫 평가 Tick, 적용한 pulse 수 등. 정의 데이터와 별개다.
- [Server/Private/GameRoom_ValtanAudition.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_ValtanAudition.cpp:2645)의 `Resolve_ValtanGameplayCatalog`: 해당 패턴 실행이 고정해 둔 revision의 catalog를 고른다. 실행 중 다른 generation과 임의로 섞지 않는다.

따라서 매 Tick은 '파일 열기 → JSON parse'가 아니라 **이미 읽은 정의 조회 → 실행 상태 갱신**이다.

## G06 서버 Tick 하나를 끝까지 따라가기

### Room Tick의 바깥 순서

[Room_Loop](C:/Users/tnest/Desktop/LostArk/Server/Private/ServerApp.cpp:2705)는 30Hz fixed step을 사용한다. [Tick_GameplaySimulations](C:/Users/tnest/Desktop/LostArk/Server/Private/ServerApp.cpp:5017)에서 control transaction을 처리하고 각 room의 Tick을 호출한다.

```text
CServerApp::Room_Loop
  -> Tick_GameplaySimulations
     -> CGameRoom::Tick
        -> 수신 command 처리
        -> 이번 Tick의 event 목록 준비
        -> Update_Players
        -> 기존 combat object / trigger / spawn / Esther 갱신
        -> Update_WorldEntities
           -> pinned gameplay catalog 선택
           -> CValtanBrain::Update
           -> stage EXIT / ENTER action 처리
           -> 타격에 따른 world destruction contact
        -> serverTick 확정
        -> 내구도 등 후처리
        -> 인간 플레이어가 있으면 Broadcast_WorldSnapshot
```

실제 위치는 [Tick](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom.cpp:840), [Update_Players](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom.cpp:1168), [Update_WorldEntities 호출](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom.cpp:1243), [snapshot 조건](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom.cpp:1318)이다. 다른 시스템의 전투 결과가 보스보다 먼저 반영될 수 있으므로 '보스만 dt를 더한다'로 줄이면 실제 실행 순서를 놓친다.

### Brain은 무엇을 읽고 무엇을 바꾸는가

[CValtanBrain::Update](C:/Users/tnest/Desktop/LostArk/Server/Private/ValtanBrain.cpp:2989)는 보스 사망/정의/대상 상태를 확인한다. HP 기믹을 예약하고, IDLE/CHASE 상태라면 패턴을 선택한다. 실행 중이면 현재 패턴과 stage를 찾는다.

선택 함수는 [SelectPattern](C:/Users/tnest/Desktop/LostArk/Server/Private/ValtanBrain.cpp:1247)이다. 예약 HP 기믹 우선, 일반 순환 선택은 [ORDERED_LOOP 처리](C:/Users/tnest/Desktop/LostArk/Server/Private/ValtanBrain.cpp:1556)를 본다. HP 구간 판정은 [Server/Private/GameplayCatalog.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/GameplayCatalog.cpp:8690)에서 `healthBar <= from && healthBar > to`다.

[BeginPattern](C:/Users/tnest/Desktop/LostArk/Server/Private/ValtanBrain.cpp:1927)은 pattern ID, start Tick, sequence, pinned revision을 설정하고 첫 stage를 시작한다. [EnterPatternStage](C:/Users/tnest/Desktop/LostArk/Server/Private/ValtanBrain.cpp:1791)는 stage 정의를 이 보스의 현재 실행 상태에 설치한다.

[Server/Private/ValtanBrain.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/ValtanBrain.cpp:1804) 원문 발췌:

```cpp
		boss.iPatternStageIndex = stageIndex;
		boss.strPatternStageId = stage.strStageId;
		boss.iPatternStageDurationMs = stage.iDurationMs;
		boss.iPatternAimLastElapsedTicks = 0u;
		boss.iPatternStageFirstEvaluationTick = evaluatesOnEntryTick ?
			serverTick : NextServerTickSkippingReservedZero(serverTick);
		boss.fPatternStageOriginX = boss.fPositionX;
```


[Server/Private/ValtanBrain.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/ValtanBrain.cpp:1818) 원문 발췌:

```cpp
		boss.strActionId = stage.strActionId;
		boss.strDamageProfileId = stage.strDamageProfileId;
		boss.ePatternHitShape = stage.eHitShape;
		boss.ePatternPlayerResponse = stage.ePlayerResponse;
		boss.ePatternAttachmentSlot = stage.eAttachmentSlot;
```


[Server/Private/ValtanBrain.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/ValtanBrain.cpp:1836) 원문 발췌:

```cpp
		boss.fPatternHitOuterRadius = stage.fHitOuterRadius;
		boss.fPatternHitInnerRadius = stage.fHitInnerRadius;
		boss.fPatternHitAngleDegrees = stage.fHitAngleDegrees;
		boss.fPatternHitLength = stage.fHitLength;
		boss.fPatternHitHalfWidth = stage.fHitHalfWidth;
		boss.iPatternHitCount = stage.iHitCount;
		boss.iPatternHitIntervalMs = stage.iHitIntervalMs;
		boss.iPatternHitDelayMs = stage.iHitDelayMs;
		boss.iAppliedPatternHitCount = 0u;
```


여기서 `iAppliedPatternHitCount=0`은 '새 단계의 타격 진행도를 초기화'한다. 같은 정의를 사용하더라도 다른 보스/다른 실행 occurrence의 카운터는 따로다.

### elapsed와 hit schedule

[Server/Private/ValtanBrain.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/ValtanBrain.cpp:100) 원문 발췌:

```cpp
	std::uint64_t StageElapsedTicks(
		const SERVER_WORLD_ENTITY& boss,
		const std::uint32_t serverTick)
	{
		const std::uint32_t firstTick =
			boss.iPatternStageFirstEvaluationTick;
		if (0u == firstTick || 0u == serverTick)
			return 0u;
		/* The Server reserves zero and wraps UINT32_MAX directly to one. Count
		only that nonzero tick cardinality, then include the first evaluation. */
		const std::uint64_t ageTicks = serverTick >= firstTick ?
			static_cast<std::uint64_t>(serverTick - firstTick) :
			static_cast<std::uint64_t>(
				(std::numeric_limits<std::uint32_t>::max)() - firstTick) +
				static_cast<std::uint64_t>(serverTick);
		return ageTicks + 1u;
	}
```


밀리초 도달 여부는 `elapsedTicks * 1000 >= milliseconds * 30`으로 비교한다. 350ms를 매번 float delta로 누적 비교하는 경로가 아니다.

[Server/Private/ValtanBrain.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/ValtanBrain.cpp:3473) 원문 발췌:

```cpp
	while (boss.iAppliedPatternHitCount < boss.iPatternHitCount)
	{
		const std::uint32_t hitOffsetMs =
			currentStage.HitOffsetsMs.empty() ?
				boss.iPatternHitDelayMs + boss.iAppliedPatternHitCount *
					boss.iPatternHitIntervalMs :
				currentStage.HitOffsetsMs[boss.iAppliedPatternHitCount];
		if (!HasElapsedMilliseconds(elapsedStageTicks, hitOffsetMs))
		{
			break;
		}
		const auto* contact = boss.iAppliedPatternHitCount < currentStage.AttackContacts.size() ?
			&currentStage.AttackContacts[boss.iAppliedPatternHitCount] : nullptr;
		ApplyPatternHit(
			boss, players, catalog, serverTick, coverCircles,
			outDamageEvents, outCaptureRequests, contact);
		if (nullptr != outOwnerHits && (contact ? contact->strShape == "CONE" : BOSS_PATTERN_HIT_SHAPE::CONE == boss.ePatternHitShape))
```


그 뒤 `iAppliedPatternHitCount`를 증가시키고 다음 pulse의 예정 시각을 확인한다. `while`이므로 한 평가에서 도달한 pulse를 처리할 수 있다. **범위에 들어온 사람 수가 아니라 처리한 타격 회차 수**가 카운터의 의미다.

### SPIN의 정확한 Tick 예제

SPIN 진입을 알리는 snapshot의 server Tick을 S라고 놓는다. 아래는 중간 중단/분기 없는 일반 진행이다.

| 서버 Tick | SPIN elapsed | 처리 |
|---|---:|---|
| S | 새 stage 설치 | `actionStartTick=S`, `firstEvaluationTick=S+1`, hit count=0 |
| S+1 | 33.333ms | offset 0ms의 첫 pulse |
| S+11 | 366.667ms | offset 350ms의 두 번째 pulse |
| S+21 | 700ms | 세 번째 pulse |
| S+32 | 1066.667ms | offset 1050ms의 네 번째 pulse |
| S+36 | 1200ms | TIMEOUT으로 RECOVERY 진입 |

SPIN 설치한 Tick S에서 hit loop를 다시 실행하지 않기 때문에 첫 pulse가 S+1이다. 모든 stage의 시작 Tick을 같은 방식으로 가정하면 안 된다. 첫 WINDUP은 [Server/Private/ValtanBrain.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/ValtanBrain.cpp:2030)의 `evaluatesOnEntryTick=true` 경로라 패턴 시작 T가 첫 평가이며 T+39에 SPIN으로 전이한다.

### 두 번째 pulse가 실행되는 S+11에서의 값

1. 직전 상태는 `iAppliedPatternHitCount=1`이다.
2. 다음 offset은 `0 + 1 * 350 = 350ms`다.
3. elapsed11Tick은 `11*1000 >= 350*30`을 만족한다.
4. `ApplyPatternHit`가 현재 보스 위치, 플레이어들의 이번 Tick 위치와 전투 상태, damage profile을 읽는다.
5. 각 대상에 실제 명중/피해를 판정하고 event를 추가한다.
6. pulse count가 2가 된다.
7. 다음 offset700ms에는 아직 도달하지 않았으므로 loop가 멈춘다.
8. stage duration1200ms도 아직이므로 SPIN을 유지한다.

### 원이 겹친 다음 실제 HP가 바뀌기까지

[ApplyPatternHit](C:/Users/tnest/Desktop/LostArk/Server/Private/ValtanBrain.cpp:2713) → [shape overlap](C:/Users/tnest/Desktop/LostArk/Server/Private/ValtanBrain.cpp:2607) → [Apply_WorldToPlayer](C:/Users/tnest/Desktop/LostArk/Server/Private/ServerCombatHitRuntime.cpp:620) 순서다. 플레이어는 중심 점 하나만이 아니라 [Shared/Public/Gameplay/WorldCollisionContract.h](C:/Users/tnest/Desktop/LostArk/Shared/Public/Gameplay/WorldCollisionContract.h:6)의 반경0.45m body를 갖는다. 단순히 중심 거리가10m 이내인지만 보는 설명보다 실제 shape overlap을 읽어야 한다.

게시된 현재 attackPower는 2285이고 damage profile은300%여서 raw damage는6855다. base [Data/Balance/BossProfiles.json](C:/Users/tnest/Desktop/LostArk/Data/Balance/BossProfiles.json:11)의100을 그대로 사용하면 틀린다. Retail override와 최종 [BOSS 행](C:/Users/tnest/Desktop/LostArk/Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap:8)이 실효값의 근거다.

실제 최종 피해는 방어력, 피해 modifier, 결정론적 ±10% 변동, 보호막, 죽음방지 등의 영향을 받는다. [Server/Private/ServerCombatHitRuntime.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/ServerCombatHitRuntime.cpp:669)부터 최종 HP 차감과 `DAMAGE_EVENT`, 내구도, 피격 반응을 따라 읽을 수 있다. 따라서 '휠윈드는 화면에 항상6855가 뜬다'는 뜻이 아니다.

### 단계 종료와 벽 파괴

[Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap](C:/Users/tnest/Desktop/LostArk/Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap:106948) 원문 발췌:

```text
PATTERNSTAGEBRANCH	ENCOUNTER_VALTAN	VALTAN_WHIRLWIND	valtan.attack.whirlwind.active	TIMEOUT	valtan.attack.whirlwind.recovery
PATTERNSTAGEBRANCH	ENCOUNTER_VALTAN	VALTAN_WHIRLWIND	valtan.attack.whirlwind.recovery	TIMEOUT	-
PATTERNSTAGEBRANCH	ENCOUNTER_VALTAN	VALTAN_WHIRLWIND	valtan.attack.whirlwind.windup	TIMEOUT	valtan.attack.whirlwind.active
```


원본의 `defaultNextActionId`가 이 TIMEOUT 행으로 만들어졌다. 실제 정상 전이는 [ApplyTimeoutBranch](C:/Users/tnest/Desktop/LostArk/Server/Private/ValtanBrain.cpp:2421) → [ApplyStageBranch](C:/Users/tnest/Desktop/LostArk/Server/Private/ValtanBrain.cpp:2210) → 다음 `EnterPatternStage`다. 마지막은 [FinishPattern](C:/Users/tnest/Desktop/LostArk/Server/Private/ValtanBrain.cpp:2072)으로 끝나고 일반 rotation cursor가 진행한다. 단순 `stageIndex++`만으로 이 사례를 설명하지 않는다.

Room은 Brain 전후 stage/sequence를 비교해서 [Server/Private/GameRoom_BossSimulation.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_BossSimulation.cpp:2680)에서 EXIT/ENTER action을 처리한다. 휠윈드의 events는 비어 있어 이 지점에서 별도 combat object를 만들지 않는다.

다만 최종 게시 패키지에는 [PATTERNWALLCONTACT](C:/Users/tnest/Desktop/LostArk/Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap:108207)도 있다. pulse 증가를 감지한 [Server/Private/GameRoom_BossSimulation.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_BossSimulation.cpp:2836)은 [Server/Private/GameRoom_WorldDestruction.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_WorldDestruction.cpp:224)로 벽/파괴물 접촉을 처리한다. JSON의 기본 stage hit뿐 아니라 **최종 게시 패키지의 부가 행까지 봐야 하는 이유**다. 이 판정도 particle mesh가 수행하지 않는다.

## G07 서버 상태가 Client에 도착하는 경로

### Snapshot에 들어가는 것과 들어가지 않는 것

[Broadcast_WorldSnapshot](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_Replication.cpp:494)은 서버 Tick과 gameplay revision, 플레이어/월드 entity 상태, damage event를 묶는다. 보스 필드는 [Server/Private/GameRoom_Replication.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_Replication.cpp:745)부터 확인할 수 있다.

| 필드 | 의미 | 왜 필요한가 |
|---|---|---|
| `serverTick` | 이 snapshot을 만든 서버 시각 | 오래된 snapshot 거절, 재생 age 계산 |
| `patternId` | 어떤 패턴인가 | presentation 정의 찾기 |
| `actionId` | 지금 패턴의 어떤 행동인가 | 현재 clip/cue 체인 선택 |
| `patternSequence` | 이 보스의 패턴 실행 회차 | 같은 패턴을 다시 쓴 경우도 새 실행으로 구분 |
| `patternStartTick` | 전체 패턴 시작 시각 | 패턴 전체 시간 기준 |
| `actionStartTick` | 현재 stage action 시작 시각 | 현재 단계의 animation/cue 시간 기준 |
| `patternStageIndex` | 현재 실행 stage 위치 | stage 일치 확인과 역행 방지 |
| `PinnedDefinitionRevision` | 이번 실행에 고정된 정의 세대 | 서로 다른 게시 세대의 판정/연출 혼합 방지 |
| 위치와 yaw | 권위 있는 보스 자세 | Client transform sample과 보간 입력 |
| HP와 `DamageEvents` | 판정의 결과 | 체력바와 피해 숫자/피격 피드백 |

Effect JSON 전체, particle 위치 배열, DDS 내용, shader 코드를 이 snapshot으로 보내지 않는다. 그 리소스는 클라이언트가 준비하고, 서버는 실행을 재구성하는 데 필요한 상태와 전투 결과를 보낸다.

### Byte로 쓰고 다시 typed message로 읽는다

[Shared/Private/Network/PacketMessages.cpp](C:/Users/tnest/Desktop/LostArk/Shared/Private/Network/PacketMessages.cpp:3278)의 `Write_Message(S2C_WORLD_SNAPSHOT)`가 typed 필드를 packet buffer에 쓴다. 보스 핵심 필드 쓰기는 [Shared/Private/Network/PacketMessages.cpp](C:/Users/tnest/Desktop/LostArk/Shared/Private/Network/PacketMessages.cpp:3458), 대응 읽기는 [Shared/Private/Network/PacketMessages.cpp](C:/Users/tnest/Desktop/LostArk/Shared/Private/Network/PacketMessages.cpp:3861)이다. Sender와 Reader의 필드 순서/자료형은 함께 맞아야 한다.

[Server/Private/GameRoom_Replication.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_Replication.cpp:881)에서 `Write_Message`를 호출하고, [Server/Private/GameRoom_Replication.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_Replication.cpp:911)에서 각 session의 `Send_Frame`에 buffer를 넘긴다.

Client 수신 thread는 [Receive_Loop](C:/Users/tnest/Desktop/LostArk/Client/Private/NetworkManager.cpp:3710)이다. 수신 byte를 stream parser에 Append하고 완성된 frame을 Try_Pop하여 inbound queue에 넣는다. **수신 thread가 CValtan이나 CModel을 직접 변경하지 않는다.**

다음 main-thread 프레임의 [Update](C:/Users/tnest/Desktop/LostArk/Client/Private/NetworkManager.cpp:855)가 raw frame queue를 비우며 Handle_Frame을 호출한다. [Client/Private/NetworkManager.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/NetworkManager.cpp:5059)의 snapshot 분기에서 decode, world/revision 검증 후 [Client/Private/NetworkManager.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/NetworkManager.cpp:5197)의 typed replication event로 큐잉한다.

[CClientReplication::Update](C:/Users/tnest/Desktop/LostArk/Client/Private/ClientReplication.cpp:352) → event 소비 → [Apply_WorldSnapshot](C:/Users/tnest/Desktop/LostArk/Client/Private/ClientReplication.cpp:4094)으로 이어진다. 오래된 Tick은 거절한다. [Client/Private/ClientReplication.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/ClientReplication.cpp:4490)의 presentation revision 확인 후 [Client/Private/ClientReplication.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/ClientReplication.cpp:4514)에서 `CValtan::Apply_NetworkState`를 호출한다.

같은 admitted revision이면 매 snapshot마다 문서를 재로드하지 않는다. revision이 바뀌었을 때 receipt와 연출 정의를 다시 검증한다. [Client/Private/ClientReplication.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/ClientReplication.cpp:994)의 이 검사가 실패하면 해당 연출을 격리하고 FX를 정지한다. 서버가 확정한 HUD 상태까지 임의로 만들어 대체하지 않는다.

## G08 Client 한 프레임의 실제 순서

Server의 30Hz Tick과 Client의 Update/Render는 **서로 다른 실행 루프**다. 아래를 하나의 동기 함수처럼 이어 붙이면 안 된다. 네트워크 수신 때문에 Client가 보는 snapshot은 도착한 최신 상태이며 서버와 같은 벽시계 순간을 뜻하지 않는다.

### MainApp과 Engine의 호출 순서

[CMainApp::Update](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:2166)의 관련 순서는 다음과 같다.

```text
이전 프레임의 level 전환 요청 적용 / UI와 입력 준비
  -> NetworkManager::Update                 MainApp.cpp:2500
  -> audition / flow / tuning 서비스 처리
  -> GameInstance::Update_Engine            MainApp.cpp:2546
       Input / Sound
       Object Priority_Update
       Camera refresh
       Object Update                       Valtan 보간 / 몸체 animation 전진
       Physics
       Object Post_Physics_Update
       Level Update                        여기에서 새 snapshot 적용
       Object Late_Update                  render provider / render group 등록
  -> Effect cue preparation
  -> EffectPresentationService::Commit_PendingSpawns
  -> Prepare_FrameCamera
  -> Synchronize_FollowAnchors
  -> EffectPresentationService::Update      dt 또는 initial seek
  -> EffectV2Runtime::Advance_ProductGroups
  -> 나머지 tool / UI 갱신
```

[Engine/Private/GameInstance.cpp](C:/Users/tnest/Desktop/LostArk/Engine/Private/GameInstance.cpp:192) 원문:

```cpp
void CGameInstance::Update_Engine(f32_t fTimeDelta)
{
	/* The deferred fog drifts on a renderer owned clock, advanced once per
	   frame so no screen pass has to be handed a time value. */
	m_pRenderer->Advance_PresentationClock(fTimeDelta);

	CProfiler* const pProfiler = m_pProfiler.get();
	{
		CProfilerScope scope(pProfiler, "Engine.Input.Update");
		m_pInput_Device->Update();
	}
#ifdef _WIN64
	{
		CProfilerScope scope(pProfiler, "Engine.Sound.Update");
		m_pSound_Manager->Update();
	}
#endif
	{
		CProfilerScope scope(pProfiler, "Engine.PriorityUpdate");
		m_pObject_Manager->Priority_Update(fTimeDelta);
	}

	{
		CProfilerScope scope(pProfiler, "Engine.Camera.Update");
		Refresh_CameraState();
	}

	{
		CProfilerScope scope(pProfiler, "Engine.ObjectUpdate");
		m_pObject_Manager->Update(fTimeDelta);
	}
	{
		CProfilerScope scope(pProfiler, "Engine.Physics");
		m_pPhysics_Manager->Update(fTimeDelta);
	}
	{
		CProfilerScope scope(pProfiler, "Engine.PostPhysicsUpdate");
		m_pObject_Manager->Post_Physics_Update(fTimeDelta);
	}

	/* Level gameplay can commit authoritative transforms and bind the
	follow camera. Run it before render submission and frustum culling. */
	{
		CProfilerScope scope(pProfiler, "Engine.LevelUpdate");
		m_pLevel_Manager->Update(fTimeDelta);
	}
	{
		CProfilerScope scope(pProfiler, "Engine.LateUpdate");
		m_pObject_Manager->Late_Update(fTimeDelta);
	}
}
```


이 순서 때문에 `Network → Replication → Valtan::Update`라고 단순 나열하면 현재 코드와 다르다. Object Update가 먼저이고 [Client/Private/Level_ValtanArena.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/Level_ValtanArena.cpp:620)의 Level Update에서 이번 snapshot이 적용된다. 적용 함수가 필요한 animation pose를 즉시 다시 샘플링하며, Late_Update는 그 다음이다.

[CValtan::Update](C:/Users/tnest/Desktop/LostArk/Client/Private/Valtan.cpp:4410)의 server-authoritative 분기는 위치 보간 등을 수행하고 super Update로 part object들을 갱신한다. Client에서 같은 패턴의 피해를 다시 판정하는 분기가 아니다.

### CValtan에 snapshot을 적용할 때

[Apply_NetworkState](C:/Users/tnest/Desktop/LostArk/Client/Private/Valtan.cpp:5889)는 오래된 Tick/sequence, 같은 실행 안에서의 stage 역행 등을 검사한다. pattern/action/sequence/stage/actionStart가 바뀌었는지 비교한다. 단순히 `PATTERN_ACTIVE` enum이 같다는 이유로 같은 애니메이션이라고 보지 않는다.

[Client/Private/ActionPresentationTimeline.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/ActionPresentationTimeline.cpp:451)에서 현재 단계 age는 `(serverTick - actionStartTick) / 30`을 기준으로 계산하며 Tick wrap도 처리한다.

[Apply_PatternPresentationSample](C:/Users/tnest/Desktop/LostArk/Client/Private/Valtan.cpp:1274)은 다음 책임을 갖는다.

1. `actionId`로 해당 clip occurrence 목록을 찾는다.
2. [Resolve_Sample](C:/Users/tnest/Desktop/LostArk/Client/Private/ActionPresentationTimeline.cpp:74)로 action age가 clip 체인의 어느 occurrence와 source time에 해당하는지 구한다.
3. clip 전환이 필요하면 Start_Animation을 한다.
4. playback speed를 설정한다.
5. source seconds를 모델의 ticks-per-second 단위로 바꾸어 Set_AnimTrackPosition에 넣는다.
6. Play_Animation(0)으로 현재 pose를 즉시 샘플링한다.

Snapshot이 없는 frame에는 [Client/Private/Body_Valtan.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/Body_Valtan.cpp:79)의 `Update_Animation(dt)`가 기존 animation을 전진시킨다. [Engine/Private/Model.cpp](C:/Users/tnest/Desktop/LostArk/Engine/Private/Model.cpp:964)가 실제 animation delta에 speed를 곱한다. 네트워크가30Hz라고 Client의 animation을30개의 정지 화면만으로 그리는 것은 아니다.

위치도 [Client/Private/Valtan.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/Valtan.cpp:5201)의 network sample queue와 [Update_NetworkTransform](C:/Users/tnest/Desktop/LostArk/Client/Private/Valtan.cpp:5266)을 사용한다. 약2Tick 지연 목표로 위치 Lerp와 yaw 보간을 한다. 서버 판정 좌표와 화면의 보간 좌표는 역할이 다르다.

### 두 번째 hit snapshot을 받은 프레임의 숫자

설명용으로 `actionStartTick=S`, 도착한 `serverTick=S+11`이라고 놓는다. sequence의 구체적인 숫자는 서버가 발급하며 여기서는 임의로 정할 필요가 없다.

- Server에서는 두 번째 pulse를 판정하여 HP와 damage event가 확정되어 있다.
- Client의 단계 age는 `11/30 = 0.3666667초`다.
- 원본 animation sample은 `0.3666667 * 0.4441666667 = 약0.1628611초`다.
- 이 effect cue를 처음 늦게 발견했다면 source clock의 약0.1628611초부터 초기 seek한다. 이미 생성된 occurrence라면 새로 생성하지 않는다.
- 다음 렌더에서 보이는 몸체와 effect는 Client의 current pose/보간 root를 사용한다. 피해량은 이 시각적 위치로 다시 계산하지 않는다.

## G09 이펙트 cue에서 살아 있는 particle까지

### cue는 재생 예약이고 effect 문서는 구성 정의다

[Client/Private/Valtan.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/Valtan.cpp:6295)가 `Spawn_DuePatternEffectCues(age)`를 호출한다. 실제 스캔 함수는 [Client/Private/Valtan.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/Valtan.cpp:3018)이다. 현재 pattern, action, stage, clip occurrence와 cue timing을 대조한다.

[Client/Private/Valtan.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/Valtan.cpp:3157)에서 이 clip-clock cue는 animation과 같은 rate를 사용한다. 초기 effect source sample은 `(현재 action age - cue의 wall 시작 시각) * playbackRate`다.

[Client/Private/Valtan.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/Valtan.cpp:3221)은 cue occurrence와 loop epoch를 한 번만 시도한다. runtime occurrence ID에는 actionStartTick, sequence, stage, cue, loop가 들어간다. `patternId`만으로 중복 방지하면 같은 패턴을 두 번째 실행할 때 재생이 막히므로 이 조합이 필요하다.

[Client/Private/Effect_PresentationService.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_PresentationService.cpp:5180)은 pending/active 양쪽에서 owner와 actionStart/occurrence 중복을 검사한다. Spawn은 즉시 draw하는 함수가 아니라 준비된 리소스를 사용할 재생 예약이다.

[Client/Private/MainApp.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:2617)에서 pending spawn을 commit한다. [Client/Private/Effect_PresentationService.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_PresentationService.cpp:6126)는 기존 EffectObject prototype을 Clone하여 Layer에 등록하고 prepared document/resources를 연결한다. 이 경로는 새 JSON parser를 매 frame 만드는 경로가 아니다. Spawn 시 준비 리소스만 붙였는지는 [Effect_PresentationService.cpp:6144](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_PresentationService.cpp:6144)의 no-I/O/no-GPU-allocation probe로도 검사한다.

### 현재 휠윈드 effect의 실제 구성

[Data/Effects/EffectCatalog.json](C:/Users/tnest/Desktop/LostArk/Data/Effects/EffectCatalog.json:134)의 ID는 `DIRECT_AUTHORED_DOCUMENT`로 등록돼 있다. 따라서 [Client/Private/Effect_Catalog.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_Catalog.cpp:932)가 Data의 authored 문서를 읽고 [Client/Private/Effect_Catalog.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_Catalog.cpp:983)에서 ID별 shared immutable document를 캐시한다. 이 예를 '반드시 Client/Bin/DataFiles에 복사한 effect JSON을 읽는다'고 설명하면 틀린다.

[현재 휠윈드 effect 원문](C:/Users/tnest/Desktop/LostArk/Data/Effects/Authored/effect.valtan.carrier-v1.attack.whirlwind.recovery.clip-01.effect.json:1)은 **121줄 / 12,555bytes / element3개**다.

| element | 가시성/형식 | 중요한 값 |
|---|---|---|
| `whirlwind.mesh.10.cyan.phase000` | visible particle + meshModel | alpha two-sided depth-read |
| 첫 mesh의 복사 요소 | visible particle + 같은 meshModel | additive two-sided depth-read |
| `whirlwind.trail.20.axe.main` | invisible trail | `b_wp_r_01` bone follow 데이터는 있지만 현재 보이지 않음 |

보이는 두 요소는 각각 maxParticles1, burstCount1, spawnRate0, particle lifetime 약0.9초, start/end size7, localSpace=true다. **한 요소가 반드시 수백 개 particle을 뜻하는 것은 아니다.** 여기서는 입자 시스템의 시간/변환 기능을 이용해 mesh 하나를 표현한다.

`detail.timing.lifeTimeSeconds=0.0166666675`는 방출 창이고 `detail.particle.lifeTimeSeconds=[0.899999976,0.899999976]`는 입자 생존 시간이다. 이 두 필드 이름이 비슷해도 소유하는 시간이 다르다.

또한 `kind=particle`인 요소에도 sprite/decal/trail의 기본 설정 블록이 저장되어 있다. 저장 포맷은 편집 가능한 공통 descriptor이고, 매번 그 모든 renderer가 동시에 실행된다는 뜻이 아니다.

### JSON 파서가 만드는 자료형

[CEffectDocumentCodec::Load](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_DocumentCodec.cpp:1294) → bounded file read → Parse → CDataJson::Parse → root/schema/version/element 파싱 → validate → staged document commit 순서다.

[Client/Public/Effect_AuthoringDocument.h](C:/Users/tnest/Desktop/LostArk/Client/Public/Effect_AuthoringDocument.h:1515)의 `EFFECT_ELEMENT_DESC`가 요소 하나, [Client/Public/Effect_AuthoringDocument.h](C:/Users/tnest/Desktop/LostArk/Client/Public/Effect_AuthoringDocument.h:2074)의 document가 이 목록을 소유한다. effectAssetId, resource binding, material, detail, sourceRecipe 등의 구조화된 필드로 변환된다.

이 휠윈드의 `sourceRecipe.enabled=false`, `modules=[]`, `material.templateId=effect.standard`, `sourceProfile.enabled=false`를 반드시 확인한다. 축포의 원본 Cascade module 해석 경로가 아니라 **수동 particle + generic material** 경로다.

### 준비와 매 frame 계산을 분리하기

[Stage_Document](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_Object.cpp:231) 계열은 재생 정의와 렌더 리소스를 준비한다. 문서/준비 리소스와 실행 시 state를 구분한다.

[CEffectPlayback::Update](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_Playback.cpp:4306)는 누적 시간으로 fixed step을 진행한다. 기본 fixed step은1/60초이며, 여기서는 cue의 playbackRate를 적용한 source clock이 입력이다. Client render frame, Server Tick, Effect fixed step은 같은 단위가 아니다.

[Step](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_Playback.cpp:4710)은 방출 시각과 burst/rate를 평가하고 [Spawn_Particles](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_Playback.cpp:5247)를 호출한다. maxParticles에서 현재 살아 있는 수를 뺀 만큼만 생성한다.

이 문서는 SourceRecipe가 꺼져 있으므로 [Client/Private/Effect_Playback.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_Playback.cpp:5347)의 authored spawn 분기다. 초기 position/velocity, 수명, 크기, 난수 등을 읽어 particle state를 만든다. 그 후 [Update_Particles](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_Playback.cpp:6620)에서 age, normalized life, 운동/색/크기와 만료를 갱신한다.

현재 velocity/acceleration이0이어도 effect가 완전히 정지한다는 뜻은 아니다. element transform의 revolution은 Y축1300 degrees/second이고, root-follow 및 element/particle 크기·회전 변환도 평가된다. 한 mesh를 움직이는 여러 변환의 소유자를 구분해야 한다.

결과는 `EFFECT_EVALUATED_FRAME` 안의 particle World, Color, normalizedLife, dynamic parameter, SubUV 등이다. GPU에 JSON을 보내는 것이 아니라 **평가된 현재 숫자와 준비된 리소스**를 보낸다.

### effect는 누가 시간과 위치를 전진시키는가

[Client/Private/Effect_PresentationService.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_PresentationService.cpp:6507)은 지연된 첫 생성의 initial seek를 처리하고, [Client/Private/Effect_PresentationService.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_PresentationService.cpp:6517)은 이후 `dt * playbackRate`로 Advance_Preview를 호출한다.

일반 autoplay EffectObject에는 [Client/Private/Effect_Object.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_Object.cpp:1009)의 Update 경로도 있지만, 이 product cue는 explicit control로 service가 clock을 관리한다. 두 함수를 보고 같은 effect가 매 frame 두 번씩 전진한다고 설명하면 안 된다.

[Synchronize_FollowAnchors](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_PresentationService.cpp:6696)는 owner root/bone의 최신 행렬을 계산하고 다음 effect update에 적용한다. 원본 mesh 크기, modelPreScale 약0.01, particle size7, element TRS, cue worldScale1.5, 발탄 root는 서로 다른 계층의 변환이다. 이 값들을 전부 '서버 공격 반경'으로 뭉치면 안 된다.

### 종료와 남아 있는 레거시 파일

`stopPolicy=natural`이므로 stage가 끝나도 이미 살아 있는 이펙트는 자연 수명을 이어갈 수 있다. [Client/Private/Effect_PresentationService.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_PresentationService.cpp:6806)은 NATURAL tail을 보존하고 [Client/Private/Effect_PresentationService.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_PresentationService.cpp:6529)는 Is_Finished인 occurrence를 제거한다. 이전 action의 아직 pending인 예약은 별도로 취소한다.

이 예의0.9초 particle lifetime은 effect source clock이다. rate0.4441666667이면 입자 생존시간만 벽시계 약2.03초에 해당한다. 정확한 종료 시각에는 방출 fixed step과 완료 조건도 포함된다. 'SPIN1.2초 종료 = effect도 즉시 제거'라는 등식은 성립하지 않는다.

[Data/Animation/Authored/Valtan/Valtan.patterneffects.json](C:/Users/tnest/Desktop/LostArk/Data/Animation/Authored/Valtan/Valtan.patterneffects.json:15)의 legacy `effect.valtan.pattern.420633.active` 연결은 남아 있다. 하지만 현재 제품 CValtan은 [Client/Private/Valtan.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/Valtan.cpp:2809)의 patterneffectcues를 로드한다. legacy 표는 [Client/Private/ValtanPatternTree.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/ValtanPatternTree.cpp:9495) 등의 저작 보조 연결이다.

V2 Sync_Stage 호출이 존재해도 [Client/Private/EffectV2_Runtime.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/EffectV2_Runtime.cpp:1134)은 일치하는 binding만 전개한다. 현재 WHIRLWIND/action의 V2 binding은0개이며 [Client/Private/EffectV2_Runtime.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/EffectV2_Runtime.cpp:1535)에서 Valtan legacy clip-name lane도 막는다. 따라서 **현재 데이터에서는 이 경로들이 추가로 중복 재생되지 않는다.** 일반적인 V1/V2 런타임 전체가 무조건 상호 배타라는 뜻은 아니다.

## G10 Resources와 Shader가 화면의 픽셀을 만드는 순서

### 실제 리소스 의존성

보이는 두 mesh particle이 사용하는 설치 파일이다. JSON에는 이 경로의 `Resources/` 아래 상대 asset ID만 들어 있다.

| 역할 | 설치 파일 |
|---|---|
| effect mesh | [Client/Bin/Resources/Effect/Valtan/Meshes/FX_SM_01/fm_m_trail_002.wmodel](C:/Users/tnest/Desktop/LostArk/Client/Bin/Resources/Effect/Valtan/Meshes/FX_SM_01/fm_m_trail_002.wmodel) |
| base와 mask texture | [Client/Bin/Resources/Effect/Valtan/Textures/FX_TEX_05/fx_m_trail_001_cl.dds](C:/Users/tnest/Desktop/LostArk/Client/Bin/Resources/Effect/Valtan/Textures/FX_TEX_05/fx_m_trail_001_cl.dds) |
| noise texture | [Client/Bin/Resources/Effect/Valtan/Textures/FX_TEX_02/fx_d_atypical_028.dds](C:/Users/tnest/Desktop/LostArk/Client/Bin/Resources/Effect/Valtan/Textures/FX_TEX_02/fx_d_atypical_028.dds) |
| dissolve texture | [Client/Bin/Resources/Effect/Valtan/Textures/FX_TEX_04/fx_h_atypical_01_1.dds](C:/Users/tnest/Desktop/LostArk/Client/Bin/Resources/Effect/Valtan/Textures/FX_TEX_04/fx_h_atypical_01_1.dds) |
| 발탄 몸체 | [Client/Bin/Resources/Character/Valtan/MN_RPBF_01.wmodel](C:/Users/tnest/Desktop/LostArk/Client/Bin/Resources/Character/Valtan/MN_RPBF_01.wmodel) |
| 발탄 무기 | [Client/Bin/Resources/Character/Valtan/ValtanWeapon.wmodel](C:/Users/tnest/Desktop/LostArk/Client/Bin/Resources/Character/Valtan/ValtanWeapon.wmodel) |
| 발탄 갑옷 부품 | [Client/Bin/Resources/Character/Valtan/MN_RPBF_01_Parts1.wmodel](C:/Users/tnest/Desktop/LostArk/Client/Bin/Resources/Character/Valtan/MN_RPBF_01_Parts1.wmodel), [Client/Bin/Resources/Character/Valtan/MN_RPBF_01_Parts2.wmodel](C:/Users/tnest/Desktop/LostArk/Client/Bin/Resources/Character/Valtan/MN_RPBF_01_Parts2.wmodel) |

WModel은 모델의 geometry와 해당 모델의 animation/material 연결을 위한 데이터이며, JSON에 삼각형 좌표와 DDS 픽셀이 전부 들어 있는 것이 아니다. 리소스 파일의 존재를 확인했지만 현재 GPU에서 렌더한 결과를 이번에 검증한 것은 아니다.

### 몸체 draw

CValtan 자체의 [Render](C:/Users/tnest/Desktop/LostArk/Client/Private/Valtan.cpp:4707)는 S_OK만 반환한다. 실제 몸체는 part object인 `CBody_Valtan`이 그린다. 이 구분을 놓치면 CValtan::Render만 읽고 '어디서 발탄을 그리는가'를 찾지 못한다.

[Late_Update](C:/Users/tnest/Desktop/LostArk/Client/Private/Body_Valtan.cpp:89)가 NONBLEND/필요한 shadow group에 몸체를 등록한다.

[Client/Private/Body_Valtan.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/Body_Valtan.cpp:114) 원문:

```cpp
HRESULT CBody_Valtan::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	for (uint32_t i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
	{
		if (Is_SourceGhostSurface(m_pModelCom->Get_MaterialSurface(i)))
			continue;
		const DEFERRED_MATERIAL_PROFILE Profile =
			Resolve_DeferredMaterialProfile(
				"material.valtan.monster-base.v1",
				m_pModelCom->Get_MaterialName(i));
		if (FAILED(Bind_DeferredMaterialInputs(
				*m_pModelCom, m_pShaderCom, i, Profile,
				m_pEmissiveOverride)) ||
			FAILED(m_pModelCom->Bind_BoneMatrices(
				m_pShaderCom, "g_BoneMatrices", i)) ||
			FAILED(m_pShaderCom->Begin(0)) ||
			FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
		(void)Render_CombatHoverMesh(*m_pModelCom, m_pShaderCom, i, m_pEmissiveOverride, true);
	}
	return S_OK;
}
```


즉 World/View/Projection, mesh별 재질 입력과 bone matrices를 shader에 바인딩한 뒤 draw한다. animation은 CPU에서 현재 골격 pose를 선택하고, 그 골격 행렬과 모델 vertex/weight가 실제 화면 geometry 계산에 사용된다.

### effect draw

[Late_Update](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_Object.cpp:1035) → Submit_RenderGroups → [Client/Private/Effect_Object.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_Object.cpp:1083)에서 presentation frame provider 등록이 이뤄진다. [Engine/Private/Renderer.cpp](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:895)가 provider들을 제출하며 현재 평가 frame에 맞는 렌더 정보를 만든다.

관련 경로는 다음과 같다.

```text
Effect 문서의 meshModel binding
  -> MESH carrier + GENERIC shader family
  -> Shader_VtxEffectMeshPreview.hlsl / 준비된 CSO
  -> CEffectDocumentRenderer::Render_Particles
  -> particle의 World / Color / life / SubUV를 mesh 입력으로 구성
  -> Render_Mesh
  -> Bind_Common: 색과 재질 상수, base/noise/mask/dissolve SRV
  -> CShader::Begin(pass)
  -> CModel::Render(meshIndex)
  -> CMesh / CVIBuffer: vertex/index buffer 바인딩
  -> ID3D11DeviceContext::DrawIndexed
```

실제 위치는 [carrier 선택](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_DocumentRenderer_ResourceStaging.cpp:211), [shader family](C:/Users/tnest/Desktop/LostArk/Client/Public/Effect_ShaderFamily.h:31), [mesh particle 분기](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_DocumentRenderer_Particles.cpp:460), [Render_Mesh 호출](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_DocumentRenderer_Particles.cpp:494), [Bind_Common](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_DocumentRenderer_Geometry.cpp:348), [Begin](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_DocumentRenderer_Geometry.cpp:448), [CModel::Render 호출](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_DocumentRenderer_Geometry.cpp:500), [DrawIndexed](C:/Users/tnest/Desktop/LostArk/Engine/Private/VIBuffer.cpp:32)다.

생성 프레임도 구분해야 한다. 일반 product cue의 Clone은 Engine의 Late_Update가 끝난 뒤 pending commit에서 만들어진다. 이 경로 자체에는 새 객체를 즉시 frame provider로 등록하는 호출이 없으므로, 처음 만든 effect가 통상적인 Layer Late_Update로 등록되는 시점은 다음 Client 프레임이다. 이미 존재하는 effect는 이번 Late_Update에서 provider를 등록하고, 그 뒤 service가 갱신한 평가 frame을 Render에서 소비한다. 따라서 'cue 발견 → 같은 프레임에 반드시 첫 픽셀 표시'라고 보장하지 않는다.

이 예는 일반 mesh particle 경로다. 다른 sprite/native mesh의 instancing 경로가 존재한다고 해서 휠윈드의 두 요소까지 무조건 단일 instanced draw로 합쳐진다고 설명하지 않는다.

### 픽셀 shader가 실제 계산하는 것

[Client/Bin/ShaderFiles/Shader_VtxEffectMeshPreview.hlsl](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_VtxEffectMeshPreview.hlsl:3) → [Client/Bin/ShaderFiles/Shader_EffectMeshFamilyCarrier.hlsli](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_EffectMeshFamilyCarrier.hlsli:528) → [Client/Bin/ShaderFiles/Shader_EffectCommon.hlsli](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_EffectCommon.hlsli:500)의 sourceProfile0 분기가 generic `Shade_Effect`를 사용한다.

[Client/Bin/ShaderFiles/Shader_EffectCommon.hlsli](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_EffectCommon.hlsli:264)에서 base texture를 sample하고, mask와 dissolve를 처리한다. [Client/Bin/ShaderFiles/Shader_EffectCommon.hlsli](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_EffectCommon.hlsli:282)에서는 대략 `(base * ColorMultiply + ColorOffset) * vertexColor`의 색을 만든다. 적용되는 추가 처리와 최종 alpha는 이 함수의 나머지 줄을 함께 읽는다.

첫 요소는 alpha blend, 두 번째는 additive blend이고 두 요소 모두 depth를 읽는 설정이다. 두 가지 blend로 동일 mesh가 달라 보이는 층을 만든다. [Client/Bin/ShaderFiles/Shader_EffectMeshFamilyCarrier.hlsli](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_EffectMeshFamilyCarrier.hlsli:546)의 PS_MAIN은 bloom 기여도 출력한다. bloom 최종 사용 여부는 팀장이 저장한 렌더링 옵션에 달려 있다.

[Engine/Private/Shader.cpp](C:/Users/tnest/Desktop/LostArk/Engine/Private/Shader.cpp:94)의 shader 로드는 EXE 옆 동일 basename의 CSO를 사용한다. JSON은 실행할 재질/입력/설정을 고르는 것이며, 매 frame HLSL을 문자열로 컴파일하는 구조가 아니다.

### 최종 화면과 Present

[CMainApp::Render](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:4138) → Render_Begin → [CGameInstance::Render](C:/Users/tnest/Desktop/LostArk/Engine/Private/GameInstance.cpp:266) → [CRenderer::Draw](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:870) 순서다.

주요 순서는 frame provider 제출, 최종 camera 관련 제출, Shadow, NonBlend/GBuffer, 선택적 SSAO, Lights, SceneHDR 합성, NonLight/Blend, ScreenPosts, 선택적 Bloom, Final, UI/Debug 등이다. SSR/SSGI와 여러 부가 pass도 옵션/상태에 따라 실행 여부가 갈린다. 모든 기능이 항상 켜져 있다고 가정하지 않는다.

Effect와 몸체 draw가 만든 scene 결과는 후처리와 UI를 거친다. [Client/Private/MainApp.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:4946) → [Render_End](C:/Users/tnest/Desktop/LostArk/Engine/Private/GameInstance.cpp:279) → [Present](C:/Users/tnest/Desktop/LostArk/Engine/Private/Graphic_Device.cpp:139)가 swap chain을 제시한다.

**화면에 보이는 것은 JSON이 아니라, JSON에서 읽은 정의와 현재 runtime state로 C++가 만든 GPU 입력을 shader와 render pass가 처리한 결과다.**

## G11 소리와 흔들림과 피해 숫자도 같은 실행에 붙는다

이 패턴을 화면의 mesh 두 개로만 설명하면 소리와 피격 UI 의존성을 빠뜨린다.

[Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json](C:/Users/tnest/Desktop/LostArk/Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json:2160)에는 SPIN의 `G_Voltan2_Attack15_Shot2` source0ms, WINDUP의 `G_Voltan2_Attack19_Cast2` source100ms와 `G_Voltan2_Attack15_ShotVox2` source300ms가 있다.

[Spawn_DuePatternSoundCues](C:/Users/tnest/Desktop/LostArk/Client/Private/Valtan.cpp:3747)는 soundBank/event로 resolve된 asset 후보와 clip occurrence를 찾는다. [Client/Private/Valtan.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/Valtan.cpp:3805)의 Resolve_CueWallOffset으로 source 시각을 action wall 시각으로 옮긴다. 그러나 사운드 재생 속도를 animation rate로 낮추는 것은 아니다. [Client/Private/Valtan.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/Valtan.cpp:3837)의 scan playbackRate는1이고 [Client/Private/Valtan.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/Valtan.cpp:3873)의 Play_SoundCue도 speed1을 사용한다. **발동 시각 변환과 WAV 자체 playback speed는 별개**다.

[Data/Animation/Authored/Valtan/Valtan.patternshakecues.json](C:/Users/tnest/Desktop/LostArk/Data/Animation/Authored/Valtan/Valtan.patternshakecues.json:590)에는 SPIN source22ms의 shake가 있다. [Spawn_DuePatternShakeCues](C:/Users/tnest/Desktop/LostArk/Client/Private/Valtan.cpp:4213)에서 clip-clock 시각을 처리한다. Presentation의 `cameraInvocations=[]`라고 해서 별도의 shake cue까지 없다는 뜻은 아니다.

전투 결과는 [Client/Private/ClientReplication.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/ClientReplication.cpp:4638)에서 `CCombatHUDViewModel::Apply_DamageEvents`로 넘어간다. [Client/Private/CombatHUDViewModel.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/CombatHUDViewModel.cpp:521)가 event를 보관하고 [RenderDamageNumbers](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:10287)가 표시할 숫자를 만든다. [Client/Private/MainApp.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:10349)에서 이미 표시한 serverTick을 걸러 중복 생성하지 않는다. 체력바는 snapshot의 authoritative HP를 사용한다. 이펙트가 화려하거나 픽셀이 플레이어와 겹친다는 이유로 HP를 깎지 않는다.

## G12 축포 JSON은 왜 13만 줄인가

### 모든 effect가 10만 줄은 아니다

앞서 조사한 `Data/Effects/Authored` 1,517개 문서 중10만 줄 초과는64개, 약4.22%였고 중앙값은10,563줄이었다. 현재 휠윈드가121줄이라는 사실도 그 반례다. 파일의 줄 수는 정렬/들여쓰기 방식에도 영향을 받으므로 byte 수와 구조를 함께 봐야 한다.

[축포 복원 JSON](C:/Users/tnest/Desktop/LostArk/Data/Effects/Authored/effect.kouku.gate1.intro.festival.full.restore.effect.json:1)은135,296줄, 약4.1MB다. 조사한 구성은80개 element이며8개 emitter를10회 activation으로 펼친 구조다. 단일 particle 하나가13만 줄인 것이 아니다.

전체에서 sourceRecipe가약77.4%, detail이약14.3%, material이약5.2%를 차지했다. 요소마다 module/literal/distribution/lookup table과 공통 편집 설정이 반복된다. 축포는890개 module 행,9,650개 literal,1,300개 distribution과12,610개 lookup 숫자를 포함했다. 같은 계열 emitter의 여러 배치를 문서 안에 풀어 저장한 중복도 있다.

### 큰 블록 안의 정보

| 블록 | 들어 있는 정보 | 실제 소비 의미 |
|---|---|---|
| element identity와 source | element ID, 출처 node, 가시성/종류 | 실행 대상과 provenance |
| resources | mesh와 base/noise/mask 등의 asset ID | 준비할 WModel/DDS 참조 |
| material | blend/depth/profile/상수/texture 설정 | shader 선택과 바인딩 |
| detail | TRS, 시간, 색, UV, particle/trail/decal 등의 공통 설정 | 저작값과 runtime descriptor |
| sourceRecipe | 원본 emitter/module/literal/distribution/curve/lookup | 지원하는 원본 동작을 해석하는 입력 |
| sourcePresentation와 override | 원본 연출 연결과 저작 변경 | 지원되는 경로의 보정/연출/편집 근거 |

이 중 sourceRecipe를 '어차피 전부 주석 같은 메타데이터'라고 버리면 안 된다. 실제 source particle에서는 spawn 수명, 속도, 크기, 시간별 변화 등이 여기서 나온다. 반대로 저장한 모든 출처 필드와 비활성 기능이 매 frame 실행된다고 해석해서도 안 된다.

### 축포와 휠윈드의 갈라지는 지점

[Client/Private/Effect_Playback.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_Playback.cpp:5280)의 `Element.SourceRecipe.bEnabled`가 대표적인 분기다.

- 휠윈드: false → authored particle 설정에서 초기값을 생성한다.
- 축포의 source 요소: true → 준비한 source module에 따라 초기값을 생성한다. [Apply_SourceSpawnModules](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_Playback.cpp:5931)와 [LIFETIME](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_Playback.cpp:5957)을 읽는다.
- distribution/curve/lookup 평가는 [Client/Private/Effect_Distribution.cpp](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_Distribution.cpp:126)에서 이어진다.
- 나이를 증가시키면서 source update module을 평가하고, 최종 particle frame을 renderer로 보낸다는 바깥 구조는 이어진다.
- source material은 해당 source shader profile과 texture/상수 경로를 사용한다. 현재 휠윈드의 generic Shade_Effect 수식을 모든 source material의 수식이라고 일반화할 수 없다.

JSON은 시간에 따른 동작을 만드는 **레시피**이지 매 frame의 완성된 pixel이나 모든 particle 좌표를 저장한 동영상이 아니다. 13만 줄을 한 줄씩 '이번 frame 명령'으로 실행하는 것도 아니다. 로드/검증/준비 때 구조화하고, frame에서는 현재 살아 있는 요소/입자의 state와 필요한 평가 데이터를 사용한다.

## G13 전체 의존성과 한 프레임을 다시 한 장으로 읽기

```text
[저작]
Composition Resources
  -> Preview: 선택 리소스의 저작용 재생
  -> Append: pattern stage / clip occurrence 편집
  -> gameplay JSON + presentation JSON + 관련 저작 문서 저장
       |
       v
[게시]
C++ Save job / Save & Publish
  -> PowerShell entry
  -> Python strict join / validate / project-products
  -> Encounter + Rotations + animation bindings + effect/sound cues
  -> Gameplay publisher + balance overrides
  -> Gameplay.bootstrap + revision receipts
       |                                    |
       v                                    v
[Server 준비]                         [Client 준비]
CGameplayCatalog                     animation/cue documents
  -> pattern/stage definitions         + BossCatalog / WModel
  + boss/damage/rotation/world data      + EffectCatalog / effect document
                                       + DDS / WModel / shader CSO
       |
       v
[Server 30Hz Tick]
player/room state
  -> CValtanBrain: choose / enter / elapsed / due pulse / timeout
  -> hit geometry + damage rules -> HP + DamageEvents
  -> stage actions / wall contact
  -> typed WORLD_SNAPSHOT -> byte buffer -> session send
       |
       v
[Client 수신과 프레임]
Receive thread -> inbound queue
  -> MainApp NetworkManager::Update -> typed replication queue
  -> Engine Object Update: previous samples / interpolation / animation
  -> Engine Level Update: Replication applies current snapshot
       -> CValtan validates Tick / sequence / revision
       -> action age -> animation source sample -> bone pose
       -> due effect/sound/shake occurrence
       -> HP / DamageEvents -> HUD state
  -> Engine Late_Update: render group / provider registration
  -> MainApp commits pending effects / follows root / advances effect clock
       -> particle state -> evaluated frame
       |
       v
[Render]
body: world + bone matrices + material + mesh
effect: particle world/color/life + texture SRV + mesh + shader pass
  -> DrawIndexed / respective renderer submission
  -> scene lighting / blend / enabled postprocess
  -> HUD / damage numbers
  -> swap chain Present
```

### 가장 중요한 변수 소유권

| 값 | 누가 결정하는가 | 화면에서의 소비자 |
|---|---|---|
| 패턴 선택, stage 전이 | Server Brain + 게시 gameplay 정의 | CValtan은 결과를 재생 |
| 명중, 피해, HP, 파괴 상태 | Server combat/room | HUD와 해당 presentation |
| 현재 재생 source time | server Tick과 Client timeline 정의 | CModel, effect cue scheduler |
| pose/bone matrices | Client model animation | 몸체 skinning, socket attachment |
| particle 위치/색/수명 | Client effect playback | effect renderer |
| final pixel | shader, blend/depth, scene/postprocess | swap chain |

### 직접 파일을 읽는 순서

1. Gameplay의 WHIRLWIND 객체를 열고 SPIN의 `durationMs / hit / defaultNextActionId`를 표시한다.
2. Presentation의 같은 stage를 열어 `actionId / clipOccurrenceId / playRate / effectAssetId`를 대응시킨다.
3. bootstrap의 세 PATTERNSTAGE 행과 세 TIMEOUT 행에서 같은 값이 게시됐는지 본다.
4. GameplayCatalog의 PATTERNSTAGE parser에서 JSON이 아니라 열 번호를 읽는 것을 확인한다.
5. EnterPatternStage에서 정의가 보스 실행 상태로 복사되는 부분을 본다.
6. hit while loop에 `elapsedTicks=11, appliedCount=1`을 대입해 두 번째 pulse를 계산한다.
7. ApplyPatternHit부터 HP 차감과 DamageEvents 생성까지 따라간다.
8. packet writer와 reader에서 pattern/action/Tick/sequence/revision이 유지되는지 본다.
9. MainApp → GameInstance → Level → ClientReplication → CValtan으로 같은 frame의 호출 순서를 확인한다.
10. action age0.3666667초를 clip source0.1628611초로 바꾸는 timeline을 본다.
11. effect ID로121줄 문서를 열고 sourceRecipe=false, meshModel, particle lifetime을 본다.
12. Render_Particles → Render_Mesh → Bind_Common → Begin → DrawIndexed → Present까지 이어 읽는다.

이 순서로 읽으면 '왜 저 숫자가 화면의 이 동작이 되는가'를 각 파일에서 한 단계씩 확인할 수 있다.

## 확인 범위

- 실제 저장 JSON과 생성 bootstrap의 WHIRLWIND 값을 읽어 대조했다.
- 현재 C++의 직접 호출자/소비자와 packet writer/reader, HLSL generic effect 경로를 추적했다.
- 설명용 Tick과 재생 source time은 코드식에 대입한 계산이다.
- 게임 코드, 저작 JSON, 게시 패키지, shader, PPTX는 수정하지 않았다.
- 이번 작업은 설명 문서 작성이므로 Client/Server 실행, publish, C++ build, GPU 화면 검증은 수행하지 않았다.
- 기술소개서의5페이지에서는 이 전체 문서를 옮기기보다, '시퀀서 저작 → 검증/게시 → 서버 권위 판정 → snapshot → Client 연출'의 책임 분리를 핵심 그림으로 쓰는 것이 적절하다.
