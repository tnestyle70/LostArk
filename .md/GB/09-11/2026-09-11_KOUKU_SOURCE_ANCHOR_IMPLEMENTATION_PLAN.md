# 쿠크 V1 원본 소켓과 재생 모델 연결 구현 계획

기준일 2026-09-11. 기존 원본 이펙트 복원 요청에서 실제 Wand source socket을 소비하기 위한 연결이다.

## G00. 현재 결함

`CEffect_Tool::Resolve_AuthoringSourceAnchors`는 `useKouku && needsBones`를 즉시 거절한다.
`CEffectAuthoringSequencer::Record_V1Anchors`의 과거 pose 채움도 쿠크 모델 sequence를 제외한다.
`CKoukuSaydonPresentationPlayer`의 V1 world-root provider는 `RootWorld`만 공급하며
`EFFECT_FIXED_STEP_TRANSFORM_SAMPLE::SourceAnchorWorlds`는 비어 있다.

따라서 재질과 emitter를 복원해도 원본 follow socket을 가진 occurrence의 attachment가 성립하지 않는다.
원본 `MidControl` socket의 runtime bone/local transform은 source effect 담당이 원본 metadata로 확정한다.
이 작업은 asset ID, bone 이름, offset을 추측해서 하드코딩하지 않는다.

## G01. 기존 typed 모델과 SourceAnchorWorlds 연결

`KoukuSaydonPresentationPlayer.h/.cpp`에 현재 typed model view와 immutable Effect document에서
source attachment world를 계산하는 공통 함수를 둔다. `CModel`의 실제 bone을 사용하고 원본
SocketLocalTransform, orientation, Resources import scale 계약을 적용한다. missing bone과 잘못된
matrix는 오류로 격리하며 root로 대체하지 않는다.

`EffectAuthoringSequencer.h/.cpp`는 선택한 Kouku pattern/bundle member의 기존
`CEffectCompositionModelPreview::Resolve_Target`만 소비한다. `Effect_Tool.cpp`의 해당 anchor 함수는
이 typed owner에 위임한다. tool이 level/layer나 actor vector index를 추측하지 않는다.

Tool에서 forward/seek가 요청한 과거 pose는 기존 Kouku model-reference sampler와 같은 60Hz 단계로
기록하고 cursor를 복구한다. stop/restart는 기존 preview lifetime에 따라 actor와 history를 해제한다.

## G02. 패턴 V1 provider와 수명

Product/Composition Preview는 기존 `Spawn_LevelPlacement → Seek_WorldRoot`를 유지한다.
외부 고정 시간 sample의 `SourceAnchorWorlds`에 실제 actor source bone pose 기록을 공급하고, seek에는
해당 occurrence의 같은 history를 사용한다. free/root element의 occurrence 배치와 source socket의
actor-relative 배치를 두 번 곱하지 않는다. 원본 emit/end lifetime은 Effect document가 계속 소유한다.

새 runtime이나 placeholder는 만들지 않는다. 기존 CModel의 명시적 transition pose 샘플러를 읽기 전용 public API로 노출한다. Product pattern에는 sourceAnchorAnimations를 optional 파생 출력으로 추가해 stage 절대 시각, 원본 clip, sourceStart/play/rate/endPolicy/blend를 보존한다. Product는 현재 pose를 과거에 복제하지 않고 같은 CModel로 요청 시각을 평가한다. Tool camera-view는 실제 scene camera를 기록하고, Product camera-view는 과거 카메라 기록이 없어 명시적으로 거절한다. source model-cue attachment는 기존 renderer-owned 경로에 남는다.

원본 socket과 particle 위치는 cm→m 0.01로 변환돼 있다. G1 bodyModelPreScale 0.017의 RawBone 3×3에 100을 곱해 원본 미터 단위의 local 값을 연결하며 이미 runtime 단위인 translation은 보존한다. 따라서 보스 본의 실제 1.7 표현 배율과 애니메이션 scale도 유지된다. GeometryPreScale metadata와 instance pre-transform을 혼동하지 않는다.

## G03. 수정 소유권과 검증

소유 파일은 `Client/Private/Effect_Tool.cpp`의 anchor 함수,
`Client/Public/EffectAuthoringSequencer.h`, `Client/Private/EffectAuthoringSequencer.cpp`,
`Client/Public/KoukuSaydonPresentationPlayer.h`, `Client/Private/KoukuSaydonPresentationPlayer.cpp`다.
Effect Tool의 목록/dispatch 변경은 pattern 담당의 최신 diff를 보존한다.

수정 전에 실제 파일 bytes를 저장하고 기존 UTF-8/CRLF를 유지한다. root가 최소 compile을 단일 조율한다.
변경 public 함수는 Tool/패턴 두 실제 소비자에 연결한다. project/filter 새 등록은 없다.
source socket 계약, missing-model/bone 실패, seek/backfill cursor 복구, natural tail과 stop/reset을 코드로
확인하고 `git diff --check`를 실행한다. Client/UI 자율 실행·조작·캡처와 visual PASS는 수행하지 않는다.

추가 소유 파일: `Engine/Public/Model.h`, `Engine/Private/Model.cpp`의 read-only transition sampler와 `Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py`의 pattern presentation projection. 기존 formatVersion 1의 optional 파생 field이며 Composition 정본 schema와 Server gameplay 정본은 바뀌지 않는다.

## G04. 실제 standalone 입력과 원본 시간의 연결

`Select_KoukuEffect(assetId)`는 Composition의 V1 resource와 이를 참조하는 유일 pattern을 조회해 기존
model-reference owner를 선택한다. `Prepare_RecoveryPreviewTarget`은 쿠크만 이 typed 선택을 사용하며,
Play All/Current Effect Play/Timeline Solo는 기존 Sequencer occurrence 경로에 합류한다. 캐릭터 skill
lookup에 보스 ID를 넣지 않는다. resource→pattern이 없거나 모호하면 이유를 반환한다.

revision 323의 신규 P29/P30은 root 승인에 따라 occurrence `followBoss=false`만 설정한다. 두 패턴은
AUDITION_ONLY/selectionWeight 0의 제자리 시험 패턴이며 시작 root를 고정하고 원본 socket의 animation은
계속 샘플한다. 기존 28개 pattern의 follow 정책은 유지한다. 실제 과거 transform이 없는 first snapshot
이전 root를 현재 위치로 보간하지 않는다. `LOOP_TO_WINDOW` preview는 source playMs에서 시계를 멈추지
않고 다음 clip 또는 pattern 창까지 idle을 반복한다.
