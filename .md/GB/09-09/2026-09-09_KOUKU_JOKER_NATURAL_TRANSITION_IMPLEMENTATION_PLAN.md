# 조커 찾기 자연스러운 전환과 접촉 궤적 정합 구현 계획서

작성일: 2026-09-09. 상태: **현재 코드·실제 CONTACT 시간 조사 후 작성 / 블렌딩 코드 구현 전**.

목표는 조커 찾기의 시퀀스 경계, 같은 clip 재시작과 망치 전환을 100ms 동안 자연스럽게 연결하고,
화면의 망치와 Server CONTACT가 같은 동작을 소비하게 하는 것이다.
현재 차원술사 1.5배 적용과 별개의 변경이다. 이 문서는 적용 상태와 수정 방법을 묻는 사용자 요청에
대한 구체적인 구현 범위이며, 작성만으로 crossfade를 반영했다고 보고하지 않는다.

조사 정본은 [블렌딩 RESULT](C:/Users/user/Desktop/LostArk/.md/GB/09-09/2026-09-09_KOUKU_JOKER_SEQUENCE_BLEND_AUDIT_RESULT.md),
시간 대조는 [transition_contact_audit.json](C:/Users/user/Desktop/LostArk/out/JokerBlend20260909/transition_contact_audit.json)이다.

## G00. 조커 찾기의 전환과 판정 범위

Source, Encounter, animation binding revision은 조사 시점 176이다.
P12는 2개 clip, P13 대형 세이튼은 21개 clip/21개 stage, P14 성공은 1개 clip이며 현재 playRate는 모두 1이다.
일반 CCharacter 0.12초 설정이 이 경로에 적용되지는 않는다. 현재 Preview는 0초와 절대 seek,
Product는 P13 전체의 `unblendedBoneContact` 및 action-age `Skip_Blend()`를 사용한다.

| 범위 | 내부 전환 | 100ms 구간의 CONTACT | 구현 시 Server 궤적 처리 |
|---|---:|---|---|
| P12 | 1 | 없음 | 기존 궤적 변경 불필요 |
| P13의 일반 전환 | 17 | 없음. 같은 clip 재시작 5곳 포함 | 블렌드 종료 후 기존 pose와 같으면 기존 궤적 유지 가능 |
| P13의 타격 중 전환 | 3 | 있음 | 동일한 보간 pose로 기존 bone bake 재생성 |
| P14 | 0 | 내부 전환 없음 | 없는 이전 pose를 가정해 첫 spawn을 blend하지 않음 |

타격 중인 세 곳은 모두 `ao_att_battle_1_03 → ao_att_battle_1_01`이다.
Product 전환 시각은 5666.667, 12000, 21033.333ms이며, 해당 100ms 안에서 실제 Server tick
170–172, 360–362, 631–633이 접촉을 평가한다. 각 타격의 중앙 1개와 측면 2개,
총 9개 region에는 해당 구간의 기존 baked key가 5개씩 있다.
다른 17개 전환은 블렌드가 끝난 뒤 다음 CONTACT까지 최소 71.667ms의 여유가 있다.

17곳은 우선 적용할 수 있는 검증 단위지만, 최종 자연스러운 P13 완료는 나머지 3곳도 포함한다.
세 곳을 영구 hard cut으로 남기거나 CONTACT window를 임의로 줄여 완료시키지 않는다.
Server 판정 알고리즘 전체를 바꾸는 것이 아니라 기존 publisher가 저장하는 뼈 궤적을 새 pose와 맞추는 작업이다.

## G01. 저장 가능한 occurrence 전환 시간과 기준 pose

### 파일과 데이터 계약

`KoukuSaydonCompositionDocument.h/.cpp`의 `KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE`에
optional `blendInMs`를 추가한다. 단위는 해당 occurrence 경계 이후의 timeline wall milliseconds이며
clip playbackRate와 독립적이다. 지원 범위는 0~1000ms, 0은 전환 없음이다.
현재 format 3 parser/serializer, equality, Copy/Duplicate/Box Detail를 같은 변경에서 연결한다.
기존 문서의 부재 값은 기존 동작으로 유지하고, 조커 찾기의 실제 내부 전환에 100을 명시한다.
첫 occurrence나 재생 창보다 긴 전환은 readiness에서 정확한 이유를 표시하며 임의 이전 clip을 만들지 않는다.

수정할 source는 `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json`이다.
`Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py`가 이를 기존 animation Product binding으로
투영하고 `KoukuSaydonPresentationAssetService.h/.cpp`가 실제 재생에 전달한다.
Product는 전환의 이전 occurrence·clip·source 끝점과 현재 occurrence 시작점, blendInMs를 함께 resolve할 수 있어야 한다.
표시 이름·vector index·현재 render frame의 pose를 저장 identity나 bake 정본으로 쓰지 않는다.
기존 generated `unblendedBoneContact`를 일괄 삭제하지 않는다. 새 전환 계약과 bake가 연결된 항목에서만
그 임시 제한을 대체하며 이전 Product 호환 읽기는 보존한다.

### 절대 시간의 전환 정의

source pose는 이전 occurrence가 경계에서 마지막으로 표시할 정확한 pose다.
target pose는 현재 occurrence의 sourceStart/playRate/endPolicy로 계산한 현재 pose다.
`alpha = clamp((clockMs - transitionStartMs) / blendInMs, 0, 1)`로 계산한다.
이전 render frame의 pose를 시작점으로 쓰면 FPS나 snapshot 지연에 따라 Server bake와 달라지므로
고정된 경계 pose를 원본 clip에서 직접 sample한다.

같은 clip이라도 `strOccurrenceId`가 달라지면 다른 전환이다.
LOOP_TO_WINDOW의 wrap, 잘린 source window와 마지막 pose 유지도 현재 timeline의 endPolicy를 그대로 따른다.
clip의 원래 재생 시간을 늘리거나, 실제 타격 애니메이션을 100ms 늦추지 않는다.
100ms가 끝나면 원래 target clip의 같은 시각 pose와 같아야 한다.

## G02. CModel의 기존 pose sampler와 body·weapon을 같은 alpha로 평가

### Engine 경계

`Engine/Public/Model.h`, `Engine/Private/Model.cpp`의 기존
`Sample_AnimationBoneCombinedMatrices`, `Sample_CurrentAnimationBoneCombinedMatricesAtBlendElapsed`,
`Sample_LocalBoneTransforms`와 local matrix blend 계산을 재사용한다.
현재 saved live blend state가 있어야 하는 sampler를 그대로 호출하는 것만으로 외부 clock의 전환이 완성되지는 않는다.
기존 내부 local pose sampling을 확장해 **명시한 두 clip/sample time과 alpha**를 받아 stage한 뒤
성공 시 live local/combined palette를 한 번에 교체하는 경계를 연결한다.

Engine은 pattern ID, occurrence ID, Server tick이나 조커라는 이름을 알지 않는다.
position/scale lerp와 quaternion slerp, root-motion suppression/vertical scale과 preTransform 적용 순서를 유지한다.
양쪽 clip의 key가 없는 bone은 immutable skeleton rest pose에서 시작한다.
현재 live pose를 기본값으로 쓰면 seek 순서나 이전 cloth pose에 따라 결과가 달라지고,
rest에서 시작하는 기존 Python bake와도 어긋난다.
잘못된 clip·범위·비유한 행렬이면 기존 live palette를 보존한다.
기존 CCharacter 0.12초 전환과 일반 NPC의 기존 `Update_Animation` 동작은 이 opt-in 때문에 바뀌지 않는다.

### 실제 망치 동기화

`NpcPresentationAssetService.cpp::Synchronize_SaydonHammerPose`는 현재 body의 단일 clip/time만 읽어
weapon에 즉시 target pose를 덮어쓴다. 이 서비스는 `CNpc::Update`, Bundle sampler, Model View가 모두 호출한다.
그러므로 body만 blend하거나 한 caller에서만 망치를 수정하면 다음 동기화가 결과를 지운다.

이 서비스가 기존 body→weapon clip mapping을 source/target 각각에 resolve하고 동일한 sample time과
alpha로 weapon pose를 평가하게 한다. mapping이 없는 쪽은 현재 문서화된 rest pose를 사용하며,
없는 animation 이름을 다른 clip으로 정상화하지 않는다.
body socket과 weapon 자체 애니메이션 모두 같은 전환을 소비해야 끝부분 위치가 일치한다.
bone Effect, trail과 historical pose도 이 최종 pose에서 anchor를 읽는다.
성공 시 현재 clip index/track과 검증한 source/target sample descriptor를 palette와 함께 commit한다.
descriptor는 해당 model의 현재 표시 pose 수명 동안 읽기 전용으로 제공하여 서비스의 반복 호출도
단일 clip 값으로 돌아가지 않고 같은 전환을 재평가하게 한다.

## G03. 단독·Bundle·Pause·Seek·Product의 같은 시각을 연결

### Preview 호출자

`KoukuSaydonPresentationPlayer.h/.cpp`의 `BUNDLE_PREVIEW_MEMBER`는
`Preview_Generation + memberId + occurrenceId`로 전환을 식별하고 해석된 두 pose 입력만 캐시한다.
Effect 전용 `SESSION.lastClockMs`를 animation 상태로 빌리지 않는다.

`Sample_BundlePreview`의 매 sample `Set_Animation(..., 0)`/pause/seek 즉시 평가를 G02의
절대 시간 pose 평가로 연결한다. 단독 P13 Play도 one-member Bundle을 사용하므로 같은 변경이 적용된다.
`Begin_BundlePreview`, `Seek_Preview`, `Sample_ModelReferencePreview`, WORLD 편집의 재평가와
연속 `Update` 모두 같은 clock에서 같은 pose를 반환해야 한다.

특히 MainApp의 Pause/Stop 입력은 현재 `Seek_Preview(displayedClock)` 후 pause를 수행한다.
Seek를 무조건 `Skip_Blend`로 바꾸면 정지 순간 다시 튄다. 같은 clock의 반복 sample, pause/resume,
authoring refresh에 alpha를 추가 누적하지 않고 표시한 시각의 보간 pose를 재평가한다.
사용자가 다른 시각으로 scrub해도 그 시각의 정의된 보간 pose를 바로 표시한다.
Stop의 기존 actor/session 해제에서는 이전 전환 캐시를 함께 비운다.

현재 `CNpc::Update`는 body raw animation→weapon sync→V2 Tick을 실행하고,
MainApp의 Preview sampler는 Engine Update 뒤에 다시 pose를 덮어쓴다.
새 전환 opt-in 중에는 CNpc의 raw 평가가 보간 palette를 다시 지우지 않도록 한다.
Preview timeline 입력을 해당 NPC update가 소비하기 전에 stage하고, Product도 매 frame 같은
절대 pose evaluator를 소비하게 한다. 최종 body→weapon→anchor/V2 Tick 순서를 한 곳에서 닫는다.
단순히 action edge나 화면 draw 직전에만 palette를 교체해 한 프레임 이전 anchor가 발생하게 하지 않는다.
같은 clock의 UI 재평가가 effect 시간을 두 번 진행시키지 않는지도 기존 session 경계에서 확인한다.

### Product 호출자

`ClientReplication.h/.cpp`의 Kouku action edge는 현재 action ID/pattern sequence/stage index에 더해
actionStartTick를 식별한다. `Npc.h/.cpp`와 `KoukuSaydonPresentationAssetService`가 기존 Server action age로
현재 source time과 전환 alpha를 계산한다.
무조건 `Skip_Blend`하는 action-age seek를 제거하는 대신 G01의 resolve된 transition을 명시적으로 sample한다.
클립 포인터가 바뀌었다는 사실만으로 전환을 시작하지 않는다.

late join도 Product에 이전/현재 occurrence의 정의가 있으면 같은 action age의 보간 pose를 재구성한다.
단순히 늦게 들어왔다는 이유로 활성 CONTACT 도중 다른 pose를 표시하지 않는다.
첫 spawn처럼 이전 occurrence가 없는 경우는 전환 없이 해당 pose를 표시한다.
누락된 Product definition은 기존 admission 실패로 처리하고 임의 previous pose를 만들지 않는다.

## G04. 세 활성 전환의 기존 bone bake를 같은 pose로 재생성

### 소유자와 실패 보존

`project_kouku_saydon_composition.py`의 기존 bone sampling/`_bone_bake_stage_origins` 경로가
G01의 동일한 두 clip/source time/alpha와 G02의 local pose 합성 순서를 소비하게 한다.
몸의 socket과 망치 자체 bone animation을 함께 계산한 최종 접촉점으로 기존 worldTrack key를 만든다.
두 최종 world 위치만 직선으로 섞는 근사로 local bone의 회전을 대체하지 않는다.

현재 Server의 `KoukuSaydonPatternLogicRuntime`은 baked track과 BOSS_CURRENT transform을 평가한다.
그 판정 owner를 Client로 옮기거나 새 네트워크 bone 전송을 추가하지 않는다.
CONTACT window·카드 목표·성공/실패 조건은 기존 값으로 두고, 전환에 영향받는 기존 궤적만 다시 생성한다.
추출·projection·runtime parser 검증이 성공한 candidate를 기존 publisher로 원자 반영한다.
실패하면 이전 Source 저장본과 Product 배포 상태를 구분해서 표시하고 이전 Product/Server revision을 보존한다.

### 시간 비교

Source UI의 누적 millisecond와 Server의 30Hz stage 누적 clock을 혼동하지 않는다.
이번 P13 마지막 경계에서는 두 값이 181.667ms 차이였다. Product/Server 비교는 현재 projector가 만드는
실제 stage origin과 CONTACT 시작/종료 tick을 사용한다.
Preview는 자신의 source schedule, Product와 bake는 같은 projected schedule을 소비하며,
동일 occurrence의 stage-local 위치와 각 경계의 정확 시각을 각각 검증한다.
source 시간을 Product 절대 시간으로 그대로 비교해 “겹침 없음”을 판정하지 않는다.

종료 검증은 위 세 구간의 모든 활성 Server tick과 경계 직전/직후에서 body·weapon 접촉점의 일치다.
추가로 기존 contact sweep/track interpolation이 쓰는 중간 sample에서도 허용 오차를 확인하고,
기존 normal/end/contact 우선순위와 성공/실패 동작을 같은 focused test에 넣는다.
17개 비접촉 전환도 100ms 종료 이후 pose와 기존 해당 구간의 궤적이 유지되는지 확인한다.

## G05. 네 번의 방향 전환을 pose crossfade와 분리해 부드럽게 표시

P13은 Source 기준 3664, 9930, 18891, 25185ms의 네 stage에서 `retargetOnEnter=true`다.
`Sample_BundlePreviewFacing`과 `CNpc::Apply_NetworkState`가 현재 방향을 즉시 적용하므로
bone pose만 blend해도 이 world yaw snap은 남는다.

현재 stage 시작에 확정하는 target yaw와 Server 방향 권위는 유지한다.
이전 stage에서 확정한 yaw(첫 stage는 initialYaw)를 시작값으로 고정하고,
현재 stage의 절대 age로 alpha를 계산해 target yaw까지 shortest arc로 100ms 보간하는 presentation을 연결한다.
직전 render frame의 yaw를 시작값으로 사용하지 않는다.
진행 중 player를 매 프레임 다시 추적해서 stage target을 바꾸지 않는다.
existing stage facing cache와 Server action-start 정보를 소비하며, 같은 clock의 seek에서는 같은 표시 방향을 재현한다.
Product late join에서 이전 동적 target yaw를 관측하지 못한 경우는 현재 Server 권위 yaw를 즉시 표시한다.
현재 snapshot에는 이전 target yaw를 복원할 근거가 없으므로 source clip처럼 문서에서 재구성한다고 가정하지 않는다.

이 단계는 네 retarget 경계의 실제 projected tick과 모든 CONTACT region의 활성 창을 먼저 대조해
전환이 다음 판정 전에 끝나는 경우에 적용한다. CONTACT와 겹치면 root yaw까지 같은 Server sampling 계약을
닫기 전에는 화면만 먼저 바꾸지 않는다. 코드의 즉시 yaw를 단순 smoothing 상수로 교체해
타격 시점까지 이전 방향이 남게 하지 않는다. body/weapon/attachment는 같은 presentation 방향을 사용한다.

## G06. 구현·검증 단위

G01~G03을 P12와 P13 비접촉 17개 경계의 첫 구현 단위로 묶고, 이어 G04로 세 타격 전환을 닫는다.
G05까지 확인해야 시퀀스 pose와 방향의 서로 다른 끊김을 모두 다뤘다고 할 수 있다.
계획은 기존 H/CPP/Python과 source/product schema의 확장이며 새 C++ runtime 파일이나 manager는 없다.
`.vcxproj/.filters`는 현재 등록 파일을 사용한다. 새 helper가 실제로 필요해지면 해당 소비자·등록·검증을 먼저 기록한다.

- 기존 document contract에서 optional field 부재/100ms/불법값, Duplicate와 Save/Reopen 보존을 확인한다.
- 기존 CModel pose sampler 검사에 같은 clip의 다른 source window, 30/60/144FPS와 같은 절대 clock,
  역방향 scrub, pause/resume, 반복 same-clock 평가, invalid 입력 시 palette 보존을 추가한다.
- body와 weapon의 source/target mapping 및 같은 alpha, root-motion 처리, Effect anchor를 확인한다.
- publisher의 기존 bone bake와 Server CONTACT focused 검사로 세 구간의 tick/접촉점을 검증한다.
- 변경 Engine/Client 최소 컴파일과 해당 HLSL 영향 여부, Python/JSON round trip, scoped `git diff --check`를 확인한다.
  최종 제품 빌드·publish·요구되는 Server 재시작은 실제 완료 여부를 RESULT에 따로 기록한다.
- 사용자는 F1 Action Workbench에서 조커 찾기 단독/Bundle Play, Pause/Seek, 같은 clip 반복,
  망치 세 전환과 방향 네 전환을 직접 확인한다. 그 뒤 Server Play의 카드 접촉·성공 결과를 확인한다.
  에이전트는 Client/UI 실행·조작·캡처나 visual PASS를 대신하지 않는다.

현재 이 계획의 구현/새 bone bake/publish/Client 재생은 미실행이다.
차원술사 외형 확대의 컴파일·링크 결과를 조커 찾기 블렌딩의 완료 증거로 사용하지 않는다.
