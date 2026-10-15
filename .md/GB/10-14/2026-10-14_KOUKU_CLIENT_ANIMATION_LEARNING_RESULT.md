# 쿠크 Client 저작·복제·애니메이션 학습 근거

읽는 순서: [00 처음부터 읽기](C:/Users/tnest/Desktop/LostArk/.md/GB/10-14/2026-10-14_KOUKU_FULL_PATTERN_WALKTHROUGH_LEARNING_RESULT.md) → [01 Python 파싱·투영](C:/Users/tnest/Desktop/LostArk/.md/GB/10-14/2026-10-14_KOUKU_JSON_PIPELINE_LEARNING_RESULT.md) → [02 Server 구조체·판정](C:/Users/tnest/Desktop/LostArk/.md/GB/10-14/2026-10-14_KOUKU_SERVER_RUNTIME_LEARNING_RESULT.md) → [03 Client 저작·애니메이션](C:/Users/tnest/Desktop/LostArk/.md/GB/10-14/2026-10-14_KOUKU_CLIENT_ANIMATION_LEARNING_RESULT.md) → [04 Collider·좌표·피해](C:/Users/tnest/Desktop/LostArk/.md/GB/10-14/2026-10-14_KOUKU_COLLIDER_RUNTIME_LEARNING_RESULT.md) → [05 전체 패턴 지도](C:/Users/tnest/Desktop/LostArk/.md/GB/10-14/2026-10-14_KOUKU_PATTERN_ATLAS_LEARNING_RESULT.md)

2026-10-14 현재 저장된 코드를 읽은 자료다. 제품 코드·데이터·PPT를 수정하거나 빌드·게시·실행하지 않았다. 아래 경로는 `C:/Users/tnest/Desktop/LostArk` 기준이다. 기술소개서 설명을 위한 Client 측 근거이며 Server의 실제 판정 함수와 projector 변환은 별도 Server 자료와 연결해서 읽는다.

## G01. 먼저 구분할 다섯 종류의 시간과 이름

원본에서 뽑은 Action을 골라 Composition에 배치하고, 게시자가 실행용 계약으로 변환한다. Server는 패턴 진행과 판정을 실행하고, Client는 그 결과에 맞는 모델 자세와 효과를 표시한다. 같은 `Action`, `Stage`, `Pattern`이라는 단어라도 각 층에서 소유하는 자료가 다르다.

| 이름 | 실제 저장 정보 | 책임과 차이 |
|---|---|---|
| 원본 Action | `iSourceActionId`, source stage/slot, reference revision | 추출된 동작의 출처. 숫자 ID 0도 유효하며 RAW 클립 참조에 쓰인다. 이것 자체가 Server 패턴 실행 명령은 아니다. |
| 원본/runtime clip | `strRuntimeClip`, CModel animation 이름 | 뼈별 scale/rotation/translation 키가 들어 있는 물리 애니메이션. 한 clip의 길이는 animation ticks와 ticks-per-second로 계산한다. |
| Composition Stage | `strStageId`, `strActionId`, kind, duration, animation occurrences | 저작 순서와 길이를 가진 단계. `STAGE_n`과 `<pattern>.stage.n`은 다른 필드다. 한 stage에 복수 occurrence를 담는 자료 구조다. |
| Composition Pattern | stable pattern ID, actor, gate, stages, 각 lane occurrence | 단계와 Logic/World/Presentation 등을 함께 편집하는 단위. 표시명과 stable ID는 다르다. |
| Server action/stage | snapshot action ID, stage index, action/pattern start tick | Server가 확정한 진행 상태. Client가 이것을 Product animation binding으로 찾아 실제 clip과 source window를 얻는다. |

따라서 **Server stage 하나가 WModel clip 하나와 같은 개념이라고 설명하면 틀린다.** stage 길이와 원본 clip 길이는 다를 수 있고, cropped range·delay·rate·loop·hold를 가진 occurrence가 그 사이를 연결한다. child presentation action이 body에 적용되는 동안 ordinary snapshot pattern 필드는 부모 gameplay clock을 유지하는 경우도 있다.

시간은 (1) 30Hz Server tick, (2) pattern 시작 기준 ms, (3) stage/action 시작 기준 ms, (4) occurrence가 시작된 뒤의 ms, (5) native clip ticks로 나눈다. `iStartOffsetMs`를 무조건 pattern-global로 읽으면 안 된다. 원래 stage 안 occurrence의 offset과, 미리보기/블렌드용으로 펼친 global offset이 같은 struct를 재사용한다. `KOUKU_SAYDON_ANIMATION_BLEND_WINDOW`의 Source/Target offset은 명시적으로 pattern-global이다.

근거: [Composition 타입](C:/Users/tnest/Desktop/LostArk/Client/Public/KoukuSaydonCompositionDocument.h:28), [원본 Action 타입](C:/Users/tnest/Desktop/LostArk/Client/Public/KoukuSaydonAnimationActionDocument.h:16), [별도 Animation Tool 조합 문서](C:/Users/tnest/Desktop/LostArk/Client/Public/KoukuSaydonAnimationPatternDocument.h:16), [child presentation 선택](C:/Users/tnest/Desktop/LostArk/Client/Private/ClientReplication.cpp:4357).

## G02. Action 리소스가 Composition의 클립 박스가 되는 과정

`KOUKU_SAYDON_ANIMATION_ACTION_REFERENCE_DOCUMENT`는 profile/model/source evidence/revision과 Actions를 가진다. Action 안에 원본 stages, 각 stage 안에 slots가 있으며 slot이 runtime clip·source start·play ms·rate·loop를 제공한다. 별도 authored action binding은 이 정확한 action/stage/slot을 지정하는 편집 자료다. `KoukuSaydonAnimationPatternDocument` 역시 `REFERENCE_ONLY` 로컬 조합 자료이므로 정식 Encounter와 혼동하지 않는다.

Action 참조 경로는 `Data/Animation/Reference/KoukuSaydon/<profile>.actionreference.json`이고 profiles는 `MN_RPCZ_00`, `MN_RPCT_07`, `MN_RPCT_05`, `MN_RPCT_06`이다. 원본 에셋 경로의 `KoukuSaton` 별칭은 현재 `KoukuSaydon` 저장 ID와 구분한다.

Workbench의 `Build_ReferenceOccurrence`는 slot에서 profile/action/stage/slot/revision/clip/sourceStart/playMs/rate를 값으로 복사한다. 원본 `bLoop`는 `LOOP_TO_WINDOW`, 아니면 `EXACT`가 된다. 원본 catalog를 수정하는 함수가 아니다.

`Append_AnimationAsStage`는 다음 순서다.

1. `Is_AppendAdmitted`로 clip을 확인하고 선택 pattern의 actor와 맞는지 검사한다.
2. `m_Draft` 전체를 값으로 복사한 candidate에서 stage/animation ordinal을 발급한다.
3. 새 stage의 kind=`ACTIVE`, duration=`source.iPlayMs`, ID=`STAGE_n`, action ID=`<pattern>.stage.n`을 만든다.
4. source occurrence를 복사해 ID=`<pattern>.animation.n`, stage-local offset=0으로 배치하고 end policy를 맞춘다.
5. pattern lifetime을 정리하고 `Commit_Candidate`가 통과해야 draft를 교체한다. 실패 때 기존 draft와 선택은 보존한다.

이미 만든 stage에 배치할 때는 `Bind_Animation`, 드래그는 `Move_Animation`, source 구간 손잡이는 `Trim_Animation`, 다른 stage로 옮기는 명령은 `Move_AnimationToStage` 계열을 사용한다. ImGui widget은 편집 의도를 전달하며 JSON을 임의 위치에서 직접 덮어쓰는 주체가 아니다. vector 변경 후 예전 row 포인터를 계속 쥐지 않고 stable ID로 다시 찾는 이유도 이 candidate 교체 구조 때문이다.

근거: [원본 slot 복사](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench_Animation.cpp:207), [새 Stage](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench_Animation.cpp:228), [Animation 추가](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench_Animation.cpp:273), [기존 Stage 바인딩](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench_Animation.cpp:566), [candidate commit](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench_Document.cpp:311).

## G03. JSON에서 typed struct까지, Save와 Publish의 차이

`KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE`는 출처 identity와 재생 수치를 함께 가진다. `sourceStartMs/sourceEndMs`는 원본에서 사용할 구간, `startOffsetMs/playMs`는 타임라인에서 배치한 구간, `playRate`는 원본 시간이 흐르는 배율이다. `sourceEndMs=0`은 native clip 끝이다. `iPoseStartMs`는 미리보기에서 계산하는 값으로 직렬화하지 않는다.

`Parse_Text`는 JSON tree를 읽어 임시 typed document를 만든다. animation parse는 occurrence/profile/sourceAction/sourceStage/sourceSlot/referenceRevision/runtimeClip/startOffset/sourceStart/playMs/playRate/endPolicy를 읽고 blendIn/sourceEnd 같은 optional 값을 처리한다. JSON key를 읽는 단계와 실제 WModel에 그 clip이 있는지 확인하는 단계는 별개다.

`Reload_FromPath`는 경로·텍스트·parse·문서 검증을 통과한 임시 값을 준비한 후 LastGood/path/baseline bytes/generation을 교체한다. 실패하면 기존 LastGood을 보존하고 Fresh를 false로 둔다. Action 참조 로드 실패는 별도 진단으로 남길 수 있으므로 `Is_Fresh()==true`가 네 profile의 모든 클립 검증 성공이라는 뜻은 아니다.

**현재 `Validate(document, references, ...)`는 `Validate_Shape` 뒤 `references`를 사용하지 않는다.** 함수 인자 이름만 보고 원본 Action 참조와 모델을 모두 검증한다고 기술소개서에 쓰면 안 된다. 모델/clip/range admission은 append·preview·Product model load 같은 실제 소비 단계에서 추가로 한다.

`Workbench::Save`는 staged presentation geometry를 candidate에 합친 뒤 `Document::Save_Atomic`을 호출한다. 이 함수는 기존 baseline이 있어야 하고, 최신 디스크 저장본이 달라졌으면 stable ID와 필드 기준 병합을 거친다. revision 증가, 직렬화, temp 재parse, 디스크 재확인, atomic 교체, 다시 열기가 이어진다. `COMMIT_SUCCEEDED_REOPEN_FAILED`는 파일은 써졌지만 reopen이 실패한 별도 상태다. World animation 편집은 소유자 callback으로 따로 저장하며 뒤쪽 World 저장 실패가 앞쪽 Composition 저장을 되돌리지는 않는다.

`Publish_AllPatterns`는 저장된 fresh source만 받는다. 코드가 실행하는 명령은 `Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision <revision>`이다. 비동기 숨김 process의 handle을 Workbench가 소유하고 MainApp background tick이 완료를 확인한다. 여기서 **Save 완료, Product 생성 완료, 실행 중 Server의 승인/로드 완료, Client의 표시 완료를 각각 확인해야 한다.** 이번 조사에서는 명령을 실행하지 않았다.

근거: [Parse_Text](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonCompositionDocument.cpp:4292), [animation 필드 읽기](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonCompositionDocument.cpp:5273), [현재 Validate](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonCompositionDocument.cpp:6491), [Reload](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonCompositionDocument.cpp:7420), [Save_Atomic](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonCompositionDocument.cpp:7468), [Workbench Save](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench_Document.cpp:202), [Publish](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench_Publish.cpp:41).

## G04. 로컬 미리보기와 실제 Server Play

로컬 Composition 미리보기는 `CAnimation_Tool`이 선택 모델의 pose clock을 샘플한다. `Start_PendingKoukuSaydonCompositionPreview`에서 model/clip/source window를 준비하고, `Sample_KoukuSaydonCompositionPreview`가 현재 occurrence를 고른 뒤 source ms→ticks로 변환해 `Set_AnimTrackPosition`과 `Play_Animation(0)`을 실행한다. 시간을 두 번 더하지 않도록 모델은 pause 상태에서 정확한 시점의 pose를 평가한다.

MainApp은 같은 최종 cursor로 animation, arena의 World preview, `PresentationPlayer::Sample_Preview`를 맞춘다. capture boundary가 필요한 경우 effective cursor로 animation clock도 맞춘다. `CAnimationTargetService`가 선택 모델/root transform을 제공한다. target이 바뀌거나 범위가 잘못되면 이전 preview를 중단하는 경계가 있다.

`CKoukuSaydonPreviewRootMotion`은 preview의 원본 `b_root` 이동을 샘플해 actor displacement를 만들고 모델 안 중복 root 이동은 억제한다. explicit BossMotion, boss charge, REAL_GAZE_TELEPORT가 있으면 자동 root motion을 적용하지 않는 조건이 있다. Reset은 자신이 임시로 바꾼 suppression 상태를 복구한다. 이 preview 변위가 Server의 실제 위치를 쓰는 것은 아니다.

`Sample_LogicPreview`는 로컬 preview owner에 대해서만 일부 Logic의 시각 결과를 만든다. 현재 pursuit projectiles, 선택 airborne 그룹, blue circle 등 해당 함수가 지원하는 분기만 재현한다. floor/circle/effect가 보여도 체력 감소, 성공/실패, 넉백, 랜덤 대상 선정의 Server 실행 성공 근거가 되지 않는다.

반면 Draft Server Play는 Play 시점의 문서를 고정한 `snapshot.json`을 고유 out 디렉터리에 저장하고 `Prepare-KoukuDraftPlay.ps1`로 임시 Product를 만든다. 준비가 성공해야 request를 소비할 수 있고 연결된 arena가 필요하다. MainApp이 `KoukuSaydonBossTool::Play_Draft`/`Play_PatternById` 경로로 전달한다. 미저장 draft의 이 경로와 정식 source Save/Publish는 같은 작업이 아니다. 제품 admission에는 source revision, draft rows revision, run epoch를 함께 확인한다.

근거: [미리보기 준비](C:/Users/tnest/Desktop/LostArk/Client/Private/Animation_Tool_KoukuPlayback.cpp:667), [pose 평가](C:/Users/tnest/Desktop/LostArk/Client/Private/Animation_Tool_KoukuPlayback.cpp:902), [preview root motion](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonPreviewRootMotion.cpp:10), [root 상태 복구](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonPreviewRootMotion.cpp:443), [Logic preview](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonPresentationPlayer_LogicPreview.cpp:155), [임시 Product 준비](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench_Publish.cpp:203), [Server Play 전달](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:3526), [preview 시계 일치](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:3730).

## G05. Server spawn과 snapshot이 실제 보스 몸체로 가는 경로

```text
Server spawn / snapshot
  → CClientReplication
  → CActorCatalog의 archetype + CKoukuSaydonPresentationAssetService
  → 기존 Prototype/Clone/Layer 경로로 CNpc 생성
  → Product action binding 조회
  → CNpc::Play_NetworkAction / Set_NetworkAnimationWindow
  → CModel → CAnimation → CChannel → CBone → skin palette → shader
```

`Ensure_Prototypes`는 ActorCatalog의 bodyModel과 bodyModelPreScale로 `CModel::Create(MODEL::ANIM, ...)`를 만든다. animationSetId가 별도 donor이면 전체 skeleton identity와 중복 clip을 검증한 뒤 같은 body prototype에 붙인다. 무기가 있는 Saydon은 catalog의 weapon model/preScale/preRotation과 body의 `b_wp_1` socket을 사용한다. 몸체를 보이게 만드는 것과 모든 Product action을 admit하는 것은 구별되며 binding 로드 실패 때 몸체를 남길 수도 있다.

Product animation 문서는 `CProjectDataRoot::Resolve("Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json")`로 읽는다. `Load_PresentationBindings`는 schema/version/sourceRevision와 각 row의 clip/source window/rate를 실제 body 모델 기준으로 검증하고 archetype별 action table을 구성한다. 다른 rig용 clip을 같은 body에 무조건 붙이지 않는다. 실행 중 run의 pinned revision과 다른 자료를 조용히 사용하지 않도록 admission 경계가 있다.

spawn은 위치/yaw/collision radius를 `CNpc::NPC_DESC`로 넘기며 `bSuppressRootMotion=true`, `bInterpolateNetworkTransform=true`를 설정한다. CNpc는 Layer에 들어가는 실제 게임 오브젝트이고 자신의 CModel component와 Transform을 가진다. replication/view는 약한 참조를 이용해 이미 사라진 body를 계속 소유하지 않는다. PresentationPlayer가 보스 gameplay entity를 새로 생성해서 대체하는 구조가 아니다.

snapshot에서 먼저 `Apply_NetworkState`로 body world transform을 갱신한다. action ID/start tick/sequence/stage 변경이 새 action 경계다. ordinary action 대신 `strPresentationPatternId`가 있는 child action을 선택할 수 있다. binding을 찾으면 `Play_NetworkAction`, 이어 서버 action age로 `Set_NetworkAnimationWindow`, 필요시 transition/blend window를 설치한다. 늦게 받은 snapshot도 시작 pose부터 다시 재생하는 대신 서버 기준 age에 진입한다. binding이 없어서 idle로 표시되더라도 코드가 Product animation 성공으로 인정하지 않고 failure를 기록한다.

근거: [body 모델 admission](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonPresentationAssetService.cpp:671), [Product 읽기](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonPresentationAssetService.cpp:244), [run admission](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonPresentationAssetService.cpp:957), [실제 CNpc 생성](C:/Users/tnest/Desktop/LostArk/Client/Private/ClientReplication.cpp:3166), [snapshot action 경계](C:/Users/tnest/Desktop/LostArk/Client/Private/ClientReplication.cpp:4357), [CNpc 모델 소유](C:/Users/tnest/Desktop/LostArk/Client/Public/Npc.h:417).

## G06. source range·속도·반복·hold가 만드는 현재 자세

Server clock 30Hz에서 action age는 `tick 차이 / 30`초다. Client는 이를 ms로 바꾸고 occurrence offset을 뺀다. crop sampler가 사용하는 수식은 다음과 같다.

```text
S = min(sourceStartMs, nativeDurationMs)
E = sourceEndMs != 0 ? min(sourceEndMs, nativeDurationMs) : nativeDurationMs
A = max(0, occurrenceElapsedMs) × playRate
LOOP_TO_WINDOW: sourceMs = S + fmod(A, E − S)
그 외:         sourceMs = min(E, S + A)
sourceTicks = sourceMs × ticksPerSecond / 1000
```

예를 들어 원본 400~1000ms 구간을 rate 2로 쓰면 실제 경과 200ms 때 원본 800ms 자세다. 경과 300ms 때 단발은 1000ms에 멈추고, LOOP_TO_WINDOW는 정확한 한 주기 끝이므로 400ms로 돌아간다. 이 숫자 예제는 수식 설명이며 특정 제품 패턴의 측정값은 아니다.

`playMs`는 타임라인에서 그 occurrence에 할당한 길이다. `sourceEndMs-sourceStartMs`와 동일한 값일 필요가 없다. `HOLD_LAST_POSE` 또는 Product hold가 있으면 network sampler에 넘기는 age를 할당 window 끝으로 제한한다. source start/end를 자르는 것, 타임라인 박스 길이를 바꾸는 것, rate를 바꾸는 것은 다른 편집이다.

`CNpc::Set_NetworkAnimationWindow`는 crop sampler가 반복을 담당하도록 `CModel::Start_Animation(clip, false)`로 full-clip loop를 끈다. 그 뒤 pause 상태에서 track position을 직접 주고 `Update_Animation(0)`으로 평가한다. CNpc update가 이후 frame delta를 network age에 누적해 다시 샘플하므로 pause가 화면 정지라는 뜻은 아니다. crop/window 평가 실패 때 전체 원본 clip으로 슬쩍 재개하지 않고 마지막 유효 palette를 보존하는 경계가 있다.

근거: [공통 source sampler](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonCompositionDocument.cpp:7682), [crop 편집](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonCompositionDocument.cpp:7714), [Network window 설정](C:/Users/tnest/Desktop/LostArk/Client/Private/Npc.cpp:242), [CNpc frame 평가](C:/Users/tnest/Desktop/LostArk/Client/Private/Npc.cpp:826), [늦은 snapshot age](C:/Users/tnest/Desktop/LostArk/Client/Private/ClientReplication.cpp:4407).

## G07. crossfade는 무엇을 섞는가

여기에는 clip 진입의 일반 blend, occurrence의 `blendInMs`, Logic에서 파생한 pattern-global `AnimationBlendWindows`가 있다. 같은 뜻으로 한 번 더 적용하면 안 된다. network window 설치는 일반 model blend를 건너뛰고, 명시된 transition pose/window를 별도로 적용한다. `bUnblendedBoneContact`인 action은 body action 시작의 일반 blend도 0으로 지정한다.

일반 occurrence blend-in은 이전 occurrence의 끝 source pose를 고정한 source와 현재 occurrence의 진행 중 target을 섞는다. Logic blend window는 window에 저장한 Source와 Target을 각각 pattern clock에서 샘플한다. `CKoukuSaydonAnimationBlend::Sample_Pose`가 active window를 찾아 `CModel::ANIMATION_TRANSITION_POSE`를 구성하고, 모델이 각 뼈의 local SRT를 섞는다. weight는 `min(elapsedSeconds/durationSeconds, 1)`이고 scale/translation은 lerp, rotation은 quaternion 보간이다. 이미 world transform까지 곱한 행렬을 단순 산술 평균하는 방식이 아니다.

같은 sampled pose를 효과의 bone anchor 계산에도 사용해야 손/무기 이펙트가 body와 어긋나지 않는다. PresentationPlayer의 source anchor 샘플은 animation window와 blend window를 읽어 해당 시각의 bone combined matrices를 얻는다. 이것은 Client 시각 부착의 일치 근거이며 Server collision 성공을 대신 입증하지는 않는다.

근거: [blend window 샘플](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonAnimationBlend.cpp:101), [CNpc transition](C:/Users/tnest/Desktop/LostArk/Client/Private/Npc.cpp:333), [preview blend](C:/Users/tnest/Desktop/LostArk/Client/Private/Animation_Tool_KoukuPlayback.cpp:1008), [모델 blend 수식](C:/Users/tnest/Desktop/LostArk/Engine/Private/Model.cpp:449), [source anchor pose](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonPresentationPlayer.cpp:923).

## G08. CModel의 애니메이션에서 화면의 뼈·무기까지

`CAnimation`은 native animation tick의 현재 위치를 소유하고, `CChannel`은 뼈의 keyframe을 찾는다. scale/translation vector는 선형 보간, rotation은 quaternion slerp로 local SRT를 만든다. immutable channel/key 자료나 샘플 결과를 재사용해도 각 모델의 CBone과 현재 자세는 따로 가진다. 카드 두 장이 같은 prototype을 써도 한 장의 pose 변경이 다른 장을 함께 뒤집는 구조가 아니다.

이 프로젝트는 행 벡터 곱 순서를 사용한다. root bone의 combined는 `local × modelPreTransform`, 자식은 `local × parentCombined`다. WModel의 authored unit을 meter 단위로 옮기는 preScale이 여기에 이미 포함된다. GPU skin palette는 `inverseBind(offset) × boneCombined`이고 여기에 object world/view/projection 단계가 이어진다. **boneCombined를 곧바로 world 좌표라고 부르거나 preScale을 두 번 곱하면 안 된다.**

Server가 body world 위치를 복제하므로 body의 원본 root translation을 동시에 그대로 이동량으로 적용하면 이중 이동이 된다. CNpc는 root suppression을 사용하고 `CModel::Apply_RootMotionTranslation`이 nonvertical translation을 rest pose 기준으로 제거하며 vertical scale은 별도로 처리한다. source root축과 runtime world Y축을 같은 축이라고 가정하지 말고, 호출자가 선택한 원본 vertical axis와 pretransform을 함께 본다. preview root sampler는 source axis 2를 명시하는 실제 경로가 있다.

무기는 body socket pose와 별도 weapon model pretransform이 합성된다. 임의로 크기·회전을 맞추기 전에 body preScale, source bone, socket offset, weapon preScale/preRotation을 분리해 봐야 한다. WorldSequence object attachment도 `Try_GetAttachmentWorld`에서 이미 preScale이 포함된 bone combined의 축을 정규화하되 pivot translation은 보존한 뒤 object world를 곱한다.

근거: [CAnimation 평가](C:/Users/tnest/Desktop/LostArk/Engine/Private/Animation.cpp:207), [channel 보간](C:/Users/tnest/Desktop/LostArk/Engine/Private/Channel.cpp:53), [bone 결합](C:/Users/tnest/Desktop/LostArk/Engine/Private/Bone.cpp:73), [root suppression](C:/Users/tnest/Desktop/LostArk/Engine/Private/Model.cpp:855), [root translation 처리](C:/Users/tnest/Desktop/LostArk/Engine/Private/Model.cpp:886), [skin palette](C:/Users/tnest/Desktop/LostArk/Engine/Private/Mesh.cpp:352), [palette 업로드](C:/Users/tnest/Desktop/LostArk/Engine/Private/Model.cpp:1446), [카드 부착 world](C:/Users/tnest/Desktop/LostArk/Client/Private/WorldSequenceObject.cpp:175).

## G09. PresentationPlayer는 별도의 pattern 시계를 소비한다

MainApp은 replication에서 보스/플레이어 presentation views를 수집해 `CKoukuSaydonPresentationPlayer::Update(dt, bosses, cards)`를 호출한다. player는 Product의 presentation pattern을 읽고 효과·카메라·상태 표식 등의 session을 유지한다. session은 entity ID, pattern ID/sequence/start tick과 run ownership으로 구분한다. 같은 ID의 예전 실행 효과를 새 run에 붙이지 않도록 admitted source/draft revision/run epoch를 확인한다.

body clip의 source clock과 effect pattern clock은 다르다. pattern session은 Server pattern start tick 또는 bundle common start tick에서 age를 계산하고 frame delta로 이어서 표시한다. Mario 입장 member는 member entry start tick/hold도 사용한다. Server run receipt가 snapshot보다 먼저 왔을 때 sequence/id를 맞추는 이유도 서로 다른 실행을 합치지 않기 위해서다.

`SESSION`은 effect/runtime handle, world preview player 등을 소유한다. 보스 CNpc는 weak reference로 빌려 쓰고 소멸한 owner 또는 오래된 run의 session은 정리한다. `Sample`은 현재 document/pattern/clock/pivot에 해당하는 presentation을 평가한다. 효과가 본체를 따라다니는 것은 해당 anchor와 pose sampling 계약 덕분이며 별도 Server entity가 생겼다는 뜻은 아니다.

근거: [MainApp 연결](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:2483), [view 수집](C:/Users/tnest/Desktop/LostArk/Client/Private/ClientReplication.cpp:2087), [Product parse](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonPresentationPlayer.cpp:1857), [Sample](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonPresentationPlayer.cpp:2798), [runtime Update](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonPresentationPlayer.cpp:3761), [session 소유 자료](C:/Users/tnest/Desktop/LostArk/Client/Public/KoukuSaydonPresentationPlayer.h:186).

## G10. 조커 카드 한 장이 실제로 뒤집히는 경로

현재 Composition의 P13 stable ID는 `KOUKUSAYDON_G1_PATTERN_13`이지만 실제 gate 필드는 `GATE2`, actor는 `MN_RPCT_06`, display name은 `대형세이튼_조커찾기`다. ID 문자열의 G1만 보고 관문을 확정하면 안 된다. 조사 시 source revision은 2500이며 source P13은 DRAFT다. 게시본에 같은 ID가 있더라도 현재 source와 게시된 Product는 별도 상태이므로 저장본만으로 실행 중 내용을 단정하지 않는다.

P13의 contact World motion mapping은 logic.24의 flip과 logic.25의 hop으로 구성되어 있다. world.5~world.10은 일반 카드 motion, world.11은 joker motion을 가리킨다. 실제 Server contact 판정과 target cue 생성은 Server 담당 코드의 책임이고 Client는 대상 cue identity와 motion ID를 소비한다.

| 구분 | 일반 카드 | 조커 카드 |
|---|---|---|
| resource ID | `world.object.kouku.card` | `world.object.kouku.joker_card` |
| WModel | `Character/KoukuSaton/MN_RHOC_00/MN_RHOC_00.wmodel` | `Character/KoukuSaton/MN_RHOC_00-1/MN_RHOC_00-1.wmodel` |
| resource 설정 | animated=true, modelPreScale≈0.01, scale=[2,2,2] | 동일 수치, 다른 model asset |
| flip instance | `world.object.instance.kouku.card_flip` | `world.object.instance.kouku.joker_card_flip` |
| hop instance | `world.object.instance.kouku.card_hop` | `world.object.instance.kouku.joker_card_hop` |
| flip template clip | `mn_rhoc_00_sk.ao_att_battle_1_start`, 667ms, rate1, loop=false, holdLastFrame=true | 동일 clip 이름 |
| hop template clip | `mn_rhoc_00_sk.ao_att_battle_2_01`, 500ms, rate1, loop=false, holdLastFrame=true | 동일 clip 이름 |
| motion 종료 | flip=HOLD, hop=NEXT→기본 card instance | flip=HOLD, hop=NEXT→기본 joker_card instance |

이 네 template의 object transform keys는 시작/끝 모두 positionOffset=[0,0,0], quaternion=[0,0,0,1], scale=[1,1,1]이다. velocity/acceleration/angular velocity도 0이다. 따라서 여기서 뒤집힘/들썩임의 모양은 외부 world TRS 키가 아니라 **animated card WModel의 해당 clip**에서 나온다.

`CLevel_KoukuSaydonArena::Consume_OwnedWorldCue`는 `runEpoch/member/sequence/cue` 소유 정보를 검사하고 중복 reliable cue를 제거한다. STOP을 먼저 받은 cue를 나중 PLAY가 되살리지 않도록 consumed ID도 보관한다. 카드 탄생 cue가 아직 없으면 motion cue를 제한된 pending queue로 미루고, 이미 있는 대상은 같은 owner와 target cue로 유일하게 찾는다.

motion은 대상 WorldSequence의 Server relative start tick 시각까지 seek→`Apply_ObjectMotion`→현재 시각까지 seek 순서로 적용한다. `Apply_ObjectMotion`은 활성 object와 motion의 object resource 및 slot이 일치하는지, 단일 WORLD object인지 확인한다. **같은 실제 카드 instance에 motion ID/start time을 교체한다.** 새 카드를 복제해 겹치는 방식이 아니다. 실패하면 이전 motion과 시작 시각을 복구한다.

처음 탄생하는 경우는 level이 `shared_ptr<CWorldSequencePlayer>`를 만들고 subset/placement/emission anchor/clock을 준비한 뒤 소유 map에 보관한다. `WorldSequencePlayer`의 object resource 준비가 기존 CModel prototype을 만들고 `CWorldSequenceObject`는 그 model을 clone한다. object는 매 sample에서 animation track의 source ticks를 계산해 model에 track position을 지정하고 `Play_Animation(0)`으로 정확한 pose를 평가한다. clip 전체를 매번 새로 로드하는 구조가 아니다.

world transform에는 resource scale, transform track, object motion, instance/Composition placement, owner anchor가 각각 들어간다. `Sample_ObjectWorld`는 WORLD placement 또는 server emission override가 있으면 원본 instance offset을 중복 적용하지 않는다. resource modelPreScale=0.01과 occurrence scale=2를 같은 의미의 값으로 합쳐 설명하지 않는다. 전자는 model authored unit 변환이고 후자는 occurrence/resource 크기다.

WorldSequence의 `Try_SampleAnimationTicks`는 별도 계약이다. timeline end에서 hold면 end pose, loop면 나머지, nonloop가 끝을 넘으면 hold 여부에 따라 end 또는 source start를 반환한다. Composition의 기본 min(end, ...) sampler와 완전히 같다고 설명하면 안 된다.

근거: [현재 Composition](C:/Users/tnest/Desktop/LostArk/Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json), [카드 template/resource 정본](C:/Users/tnest/Desktop/LostArk/Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json), [cue 소유·소비](C:/Users/tnest/Desktop/LostArk/Client/Private/Level_KoukuSaydonArena.cpp:6180), [기존 instance motion 교체](C:/Users/tnest/Desktop/LostArk/Client/Private/WorldSequencePlayer.cpp:840), [object resource 준비](C:/Users/tnest/Desktop/LostArk/Client/Private/WorldSequencePlayer_Objects.cpp:703), [world 합성](C:/Users/tnest/Desktop/LostArk/Client/Private/WorldSequencePlayer_Objects.cpp:1327), [실제 card pose 평가](C:/Users/tnest/Desktop/LostArk/Client/Private/WorldSequenceObject.cpp:137), [WorldSequence source ticks](C:/Users/tnest/Desktop/LostArk/Client/Private/WorldSequenceDocument.cpp:2528).

## G11. 카드미로·Mario·빙고의 Client 경로와 입력 경계

특수 기믹을 모두 새로운 보스 클래스로 구현한 구조가 아니다. 공통 replication/character/controller/HUD에 상태를 추가하고 arena와 PresentationPlayer가 각 상태에 맞는 표시를 제공한다.

| 기믹 | Client에서 실제 쓰는 상태와 클래스 | 입력·표시 경계 |
|---|---|---|
| 카드미로 | Player snapshot의 CardMaze role/suit/flags/exit, `CCharacter::Apply_MazePresentation`, Arena의 maze presentation/camera, PresentationPlayer의 marks | Q는 interaction slot, 왼쪽 클릭 망치는 slot W로 보내며 HUD는 mouse 망치 슬롯 노출을 별도로 제한한다. telescope/hunter 역할·문양·목표·출구는 복제값에 따른다. |
| Mario | snapshot `iMarioStage`, Arena의 stage/progress/balls/bomb visual, PresentationPlayer Mario marks/entry clock | 좌우 화살표는 LEFT/RIGHT/STOP intent, ↑ rising edge는 Server jump/제공된 이동 trigger, 0은 Server return 요청. 점프 API의 Debug 이름은 남아 있어도 로컬 자유 이동이 아니다. |
| 빙고 | `CCombatHUDViewModel::Get_BingoBoard`, PresentationPlayer의 bombs/marks, WorldSequence hammer | 폭탄/25칸 표식은 복제된 board 상태를 표시한다. hammer는 방향별 authored template와 Server가 고른 instance를 사용한다. Debug fill/bomb/hammer typed command가 있다고 Space 연타 기믹이 구현됐다고 볼 수 없다. |
| 춤·변신 | snapshot HUD mode, mode별 slot mapping/cooldown, Character interaction animation, `CKoukuMadnessGaugeView` | 일반 class skill 대신 interaction slot을 전송하고 mode JSON의 아이콘을 표시한다. 현재 DANCE 정의는 4개 자세다. Q/W/E/R/A/S 여섯 입력을 순서대로 띄우는 별도 퍼즐과 동일하지 않다. |
| 주사위·공포·카드 표식 | PresentationPlayer DiceBind/Fear/card 효과, replicated player states | 표시와 상태 시간은 복제값을 따르며 Client 효과 성공이 Server 구속/피해 성공은 아니다. |

`PlayerController::Poll_SkillSlots`는 non-NONE Kouku HUD mode에서 Q/W/E/R/A/S/D/F의 rising edge를 `IPlayerCommandSink::Request_InteractionSlot`로 보낸다. MAZE는 Q만 키보드 slot으로 받고 왼쪽 클릭을 별도로 W에 매핑한다. `NetworkPlayerCommandSink`는 `CNetworkManager`로 전달하고 후자가 `C2S_INTERACTION_SLOT` packet을 만든다. **키 입력→요청→Server 허용/판정→snapshot→UI 갱신**이 경로이며 UI에서 icon을 눌렀다고 HP나 성공 판정을 직접 바꾸지 않는다.

Mario의 이동은 방향이 바뀌거나 100ms keepalive일 때 intent를 전송한다. foreground/text input/free camera/captured/dead 등 입력 gate와 request sequence가 있고 reply는 같은 sequence/world인지 확인한다. ↑를 UI 포커스 중 누른 채 복귀해도 새로운 점프로 오인하지 않도록 physical edge를 계속 관찰한다. G 상호작용은 Server가 현재 제안한 trigger ID 또는 HERE를 보내며 Client가 임의 목적지를 만들어 teleport하지 않는다.

제품 HUD는 `MainApp::Load_KoukuHudModes`가 `UI/KoukuSaydon/KoukuHudModes.json`을 읽고 `Update_KoukuHudMode`가 서버 `ModeSkillIndexBySlot`/cooldown에 맞춰 기존 `HUDRuntimeView`의 stable slots를 바꾼다. ownerClass는 `KoukuSaydonInteraction`이다. 현재 JSON의 modes는 POLYMORPH 3개, MARIO 2개, DANCE 4개, MAZE 2개이며 icon은 `UI/KoukuSaydon/Hud/...` Resources-relative ID다. 화면의 ImGui 저작 도구 자체가 제품 HUD인 것은 아니다.

근거: [interaction 입력](C:/Users/tnest/Desktop/LostArk/Client/Private/PlayerController.cpp:746), [maze LMB](C:/Users/tnest/Desktop/LostArk/Client/Private/PlayerController.cpp:393), [Mario controls](C:/Users/tnest/Desktop/LostArk/Client/Private/PlayerController.cpp:1380), [↑ jump](C:/Users/tnest/Desktop/LostArk/Client/Private/PlayerController.cpp:1790), [typed sink](C:/Users/tnest/Desktop/LostArk/Client/Private/NetworkPlayerCommandSink.cpp:175), [packet 작성](C:/Users/tnest/Desktop/LostArk/Client/Private/NetworkManager.cpp:1706), [HUD 복제](C:/Users/tnest/Desktop/LostArk/Client/Private/CombatHUDViewModel.cpp:236), [HUD 정의 읽기](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:6488), [HUD 표시](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:6555), [HUD JSON](C:/Users/tnest/Desktop/LostArk/Data/UI/KoukuSaydon/KoukuHudModes.json), [maze 표시](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonPresentationPlayer.cpp:5922), [빙고 폭탄](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonPresentationPlayer.cpp:6005), [빙고 칸](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonPresentationPlayer.cpp:6084).

## G12. 직접 따라 읽을 때 한 패턴에서 기록할 값

처음에는 P13 카드 한 장만 고르고 아래 identity와 시각을 종이에 이어 적으면 구조를 검증하기 쉽다. 아래는 실행을 권하는 지시가 아니라 소스 읽기와 사용자의 후속 디버깅 관찰 목록이다.

1. Composition의 patternId/actorProfile/gate, stageId/actionId, animation occurrenceId와 source identity를 적는다. 표시명 대신 stable ID로 연결한다.
2. 원본 `runtimeClip/sourceStart/sourceEnd/playMs/playRate/endPolicy`를 적고 어느 값이 timeline, 어느 값이 native source인지 표시한다.
3. 게시 animation binding에 대응 action ID와 revision이 있는지 본다. 현재 source draft와 Product가 같은 revision인지 확인한다.
4. snapshot의 run epoch/pattern sequence/start tick/action start tick과 child presentation 필드를 분리한다.
5. body `Try_Resolve_Action → Play_NetworkAction → Set_NetworkAnimationWindow`에서 최종 sourceMs/ticks를 위 수식으로 재계산한다.
6. World 카드라면 spawn cue의 owner/target cue/occurrence와 motion cue identity를 연결한다. 일반 카드와 조커의 resource/model이 실제로 다른지 확인한다.
7. object model clone의 animation index/name, modelPreScale, resource scale, placement, bone combined, world를 따로 기록한다.
8. Client에서 화면이 맞는지와 Server에서 contact/HP/넉백이 발생했는지는 각자 결과를 확인한다. 이번 읽기 전용 조사는 후자의 실제 실행 성공을 검증한 것이 아니다.

설명할 때는 “JSON을 읽어 애니메이션을 실행한다”에서 멈추지 않고, **누가 정의를 소유하고, 누가 시간을 결정하고, 어떤 stable ID로 실제 모델 인스턴스를 찾으며, 실패 때 어느 상태를 남기는지**까지 한 흐름으로 연결하면 된다.

## G13. ImGui에서 만든 손·뿅망치 Collider의 anchor는 어디서 결정되는가

### 저작 입력에서 저장까지

`Render_PresentationResources(COLLIDER)`는 BOX/CIRCLE/SECTOR/REVERSE_SECTOR/CYLINDER 등의 preset을 보여 준다. `Create_PresentationResource`는 후보 document의 `PresentationResources`에 stable resource ID와 모양 정의를 추가한다. 이것만으로 특정 패턴에 Collider가 생기지는 않는다. `Append_PresentationBox` 또는 source 직접 추가의 `Append_PresentationSource → Append_PresentationCandidate`가 선택 pattern의 `PresentationOccurrences`에 별도 occurrence ID/resource ID/시작/수명을 만든다.

`Render_ColliderBoxDetails`는 현재 occurrence의 사본 `m_PresentationBoxEdit`를 편집한다. 그 안의 `Render_PresentationAnchor(pattern, edit, false)`가 `anchorKind`, `boneTarget`, `bone`, 회전 추종 정책을 바꾼다. BOSS의 BODY는 body model, WEAPON은 별도 weapon model의 실제 bone 목록을 조회하며 target을 바꾸면 기존 bone을 지워 잘못된 rig 참조를 남기지 않는다. WORLD는 `worldId` 외에 실제 `worldOccurrenceId`/emission index와 필요시 object bone을 지정한다. 같은 카드 resource를 여러 장 쓸 때 resource ID만으로 어느 카드인지 정할 수 없기 때문이다.

`Shared Logic window` 선택은 resource의 logic ID가 아닌 **pattern에 배치된 `logicOccurrenceId`**를 연결한다. Apply는 모드에 따라 `Set_PresentationBox`, `Set_ColliderLogicValues`, `Set_ColliderTriggerDamage`로 간다. 뒤 함수는 접촉 Trigger·성공 Damage Result·시간 구간·Collider 연결을 하나의 candidate에서 준비한다. `Set_PresentationBox`도 연결된 Logic window와 그 window를 공유하는 Collider의 시작/수명을 함께 맞춘다. 크기/위치 드래그의 preview overlay와 아직 Apply하지 않은 Logic/Damage 입력은 별개 상태다. 최종 Save는 G03의 `Workbench::Save → Document::Save_Atomic`으로 이어지며 최신 저장본과 병합한다.

실제 현재 P13 예로 `KOUKUSAYDON_G1_PATTERN_13.presentation.17`은 `koukusaydon.g1.presentation.30`(COLLIDER/BOX)을 사용하고 `anchorKind=BOSS`, `boneTarget=WEAPON`, `bone=b_rpct_01`, `followBoss=true`, `logicOccurrenceId=KOUKUSAYDON_G1_PATTERN_13.logic.22`다. 이 행은 보스 몸체의 이름만 같은 bone을 찾는 것이 아니라 **별도 무기 모델의 bone**을 찾는 연결이다. 손 Collider는 자신이 선택한 BODY bone 경로를 확인해야 하며, 모든 손·망치에 같은 offset이나 rotation을 일괄 적용할 근거는 없다.

근거: [Collider preset UI](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench_Presentation.cpp:1746), [resource 생성](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench_Presentation.cpp:914), [occurrence 배치](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench_Presentation.cpp:1010), [Collider 편집 상태](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench_Collider.cpp:723), [anchor UI](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench_Presentation.cpp:2269), [BODY/WEAPON/bone](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench_Presentation.cpp:2445), [Logic window 선택](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench_Collider.cpp:1261), [Apply 분기](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench_Collider.cpp:793), [Logic 연결 commit](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench_Collider.cpp:303), [Damage 연결 commit](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench_Collider.cpp:456), [geometry와 window 동기화](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench_Presentation.cpp:1163).

### Client preview에서 world로 합성되는 위치

`PresentationPlayer::Sample`의 Collider 분기는 `Sample_ColliderGeometry`로 그 시각의 geometry를 얻고 anchor를 선택한다. MAP은 identity, BOSS는 body pivot/bone 또는 weapon target view, WORLD는 `resolveWorldPivot`가 preview의 실제 WorldSequence instance나 실행 중 owned cue에서 anchor를 찾는다. WORLD의 첫 pose가 아직 없으면 다음 frame을 기다리며 임의 원점에 붙이지 않는다. `followBoss=false`인 Collider는 확보한 placement anchor를 고정하고, true이면 현재 anchor를 따라 갱신한다.

최종 합성은 같은 파일의 `Make_Pivot`이다. BODY는 `model + root`, WEAPON은 `weaponView.Model + weaponView.BoneRoot`를 `CEffectV2Object::Resolve_TargetPivot`에 주고 `BONE` 또는 `TARGET_YAW` 회전을 선택한다. 얻은 anchor의 축을 정규화한 뒤 **`occurrence Scale × occurrence Rotation × local PositionOffset × anchor basis`**로 world를 만든다. WORLD anchor의 scale은 앞 단계에서 occurrence scale/offset에 반영하고 bone을 다시 적용하지 않는다. Collider의 `BONE` 회전은 완전한 bone/socket basis에서 offset/rotation을 합성한 **뒤** 최종 전방을 XZ 평면에 투영한다. 본의 단순 +Z를 처음부터 수평 방향으로 취하는 것과 결과가 다를 수 있다.

이 `row.pivot`과 `Collider_Wire`는 Client의 표시·preview 근거다. 실제 damage/knockback은 같은 저작 anchor/clock을 publisher와 Server가 어떻게 받아 별도 collision world를 만드는지까지 이어서 검증해야 한다. “망치에 wire가 보인다”만으로 Server contact·대상 필터·Result 실행·넉백 적용까지 완료됐다고 설명하지 않는다.

근거: [최종 Make_Pivot](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonPresentationPlayer.cpp:687), [World 실제 instance 조회](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonPresentationPlayer.cpp:2905), [anchor 선택·scale·wire 생성](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonPresentationPlayer.cpp:3093), [WorldSequence bone/world 합성](C:/Users/tnest/Desktop/LostArk/Client/Private/WorldSequenceObject.cpp:175).
