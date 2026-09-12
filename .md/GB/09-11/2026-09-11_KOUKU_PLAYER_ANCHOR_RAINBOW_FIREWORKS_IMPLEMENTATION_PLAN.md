# 쿠크 이펙트 플레이어 앵커 재생과 무지개·폭죽 구현 계획

## G00. 현재 호출과 입력

All Effects의 KoukuSaydon 행은 `Try_PlayActiveUnifiedEffect`에서 복원 재생으로 들어간다.
`Prepare_RecoveryPreviewTarget -> Select_KoukuEffect`가 저장 Composition의 유일한 보스 패턴을
요구하고 `Resolve_Root`가 그 보스 위치를 사용한다. 본이 없는 독립 이펙트도 이 조건에 걸린다.
현재 draft의 실제 source attachment 요구를 먼저 읽고 본이 필요한 경우에만 원본 모델을 준비한다.

첨부 이미지는 넓은 무지개 대각선 띠, 네 갈래 별, 색종이와 폭발을 조사하는 입력이다.
원본 Action 4219916 stage002의 SkillEffect 421991412~421991421은 45도·135도 격자를
0.6초·2.3초에 생성한다. Projectile 421991401~421991405는 고정 영역에서
`Par_V_RPCT_fire_Decal_T_01_LOC_INT`와 `Par_V_RPCT_fire_BIgearthwave_01_LOC_INT`를
양방향으로 재생한다. 전자는 0.3초, 후자는 행마다 2.0/1.8/1.6/1.4/1.2초의 원본 Timer를
사용한다. 조사 초기에 찾은 Ray/Dance/Ready 후보는 이 조합으로 대체한다.
1관문 SCENE03A/interpdata_0의 festival과 interpdata_7의 fireworks는 서로 다른 원본
연출이다. 각 Emitter와 Matinee toggle 시각을 사용한다.
화면에 맞추어 임의 입자나 배치·타이밍을 원본 값으로 기록하지 않는다.

## G01. Effect Tool와 기존 Sequencer

수정 파일은 `Client/Private/Effect_Tool.cpp`, `Client/Private/EffectAuthoringSequencer.cpp`,
`Client/Public/EffectAuthoringSequencer.h`다. 실제 SceneCharacter의 yaw와 위치를 Play 시점에
고정하고 같은 authoring occurrence의 root로 전달한다. Source bone은 기존 Kouku 모델 공급자와
원본 pose를 사용하며 기존 inverse(ownerRoot) × previewRoot 변환으로 기준점만 옮긴다.
CAMERA_VIEW 또는 root-only 문서는 보스 패턴 없이 기존 V1 factory에서 재생한다.

Play All은 0ms부터 다시 시작한다. Solo/Play Group은 현재 draft와 원본 element 시각을 유지한다.
실패 시 이유를 All Effects에서도 표시하고 기존 문서의 dirty guard를 유지한다.
임시 재생은 Append나 저장 sequence로 바꾸지 않는다. Character·Valtan 기존 재생도 유지한다.

Action Workbench의 기존 V1_EFFECT/V1_ELEMENT/GROUP Append 경로에 EFFECT의 MAP 앵커를
연결한다. 고정 월드 위치·회전·크기를 occurrence가 소유하고 `Use Player Position`으로 현재
플레이어 위치를 복사할 수 있다. MAP은 follow/bone/world 참조를 함께 저장할 수 없다.
Composition C++ parser, Python projector와 PresentationPlayer가 같은 규칙을 사용한다.

## G02. 원본 Effect 문서와 재질

`Tools/EffectPipeline`의 기존 source/module importer와 native material translator를 재사용한다.
신규 rainbow/fireworks builder는 선택한 실제 first-LOD 모듈·분포·재질·리소스와 원본 시각을
`Data/Effects/Authored/*.effect.json`으로 구성한다. 폭죽의 배치와 입자 이동은 각각 원본
actor/Matinee와 Location/Velocity/Acceleration 모듈에서 읽는다.

새 native 프로그램이 필요하면 기존 Artist material descriptor와 Kouku shader carrier에 연결한다.
현재 프로그램을 재번호하거나 덮지 않는다. EffectCatalog는 direct authored ID와 정확한 상대 경로를
등록한다. Resources에는 사용되는 DDS/WModel만 기존 Effect/KoukuSaydon 경로로 설치하며 Git에
추적하지 않는다. 원본 전체 추출물과 수치 진단은 out에 둔다.

폭죽의 원본 `EPET_Death`는 기존 bounded particle event queue를 재사용해 입자의 실제
수명 종료 위치·속도에서 후속 emitter를 생성한다. source `ERM_None` provider는 원본 수명과
루프를 보존하고 문서 내 event/location 소비자가 있을 때만 허용한다. 문서의 재생 시간은
원본 emission 종료와 event chain의 입자 tail까지 포함하며 새 입자를 임의로 더 만들지 않는다.

## G03. 등록과 검증

새 C++ 파일은 만들지 않는다. 새 authored JSON은 Client project와 filters의 기존 96.DataFiles에
None으로 등록한다. 새 shader producer가 필요한 경우 기존 FxCompile/97.ShaderFiles에 등록한다.
실제 원본 입력과 문서의 모듈·분포, Resources 상대 경로와 존재, JSON/XML parse를 확인한다.
기존 CPU Playback과 shader 검사 방식으로 생성·수명·finite transform 및 실패 보존을 검증한다.
변경 C++ 최소 컴파일과 shader 컴파일, git diff --check를 수행한다.

같은 작업 폴더에서 다른 기능 작업과 사용자 Client/Server가 실행 중이다. 공유 산출물 빌드는
겹치지 않고 필요한 최소 컴파일은 별도 out에서 수행한다. Client/UI를 실행·조작하거나 캡처하지
않는다. 마지막 RESULT에는 실제 빌드 범위와 사용자 화면 확인을 분리한다. 사용자는
F1 → Effect Tool V1 → All Effects → KoukuSaydon → Play All/Open Editor에서 확인한다.

## G04. 아레나 local player와 Effect preview target 수명 연결

`Enter an arena with a scene player before Play All. The player is the Effect anchor.`는
`Select_SceneEffectTarget → Resolve_SceneCharacter`에서 발생하는 생성 전 오류다.
카메라·입력에 local player가 연결돼도 `CAnimationTargetService` 등록이 없으면 Effect를
준비할 수 없다. local player transaction의 소유자인 `CClientReplication`에서 연결한다.

`Commit_PlayerSpawn`과 `Replace_CharacterClass`의 local character commit 성공 뒤 Bind한다.
실패 rollback과 remote player는 기존 target을 보존하고, local despawn·Reset_World·destructor는
해당 character만 Unbind한다. destructor에서 이미 제거됐을 수 있는 Layer를 다시 조작하지 않는다.
Character Select의 기존 exact-pointer Bind/Unbind 계약과 Effect factory·shader는 유지한다.

기존 target service를 재사용하므로 새 C++ 파일이나 프로젝트 항목은 없다. local/remote 생성,
class 교체 성공·실패, 퇴장·reset·종료의 target 소유권과 변경 번역 단위 컴파일을 확인한다.
Effect의 시작 시각이나 MAP 위치를 이 오류의 우회 수단으로 바꾸지 않는다.

## G05. 무대 앞 금빛 이동 축포의 원본 Move·반복·속도 연결

첨부 이미지의 금빛 선단과 곡선 잔광은 SCENE03A Matinee0의
`FX_Q_W_01.FX_Par_02.Par_Q_Trail_01`과 actor Move를 함께 사용한다. 이 효과의 mesh sparkle과
smoke tail은 SpawnPerUnit이므로 실제 이동 거리가 없으면 꼬리 입자가 생성되지 않는다.

`build_kouku_sequence_effect_groups.py::actor_groups`의 UE FName 연결을 대소문자 비구분으로
수정한다. `Pc01tr`/`pc01tr` 차이로 누락된 Move를 복구하고 actor/group 불일치와 모호한 이름은
거부한다. 기존 SourceTransformTrack schema와 source key의 시간·값·접선은 유지한다.

이번 main 기준 적용 대상은 아래 두 source 문서다.

- `effect.kouku.sequence.lv_lut_midnightc_ed_scene03a.efseqact_matinee_0.1`: node 12개, loop 24개.
- `effect.kouku.sequence.lv_lut_midnightc_ed_scene03a.efseqact_matinee_0.2`: node 24개, loop 24개.

원본 Required 인스턴스와 완전한 CDO 체인이 확인한 EmitterDuration 1초·EmitterLoops 0을
사용한다. 선택한 trail의 임시 loop 1만 교정하고 원본 Toggle 종료와 KillOnDeactivate/Completed는
보존한다. 기존 P4가 이 source `.1/.2`를 소비하므로 occurrence를 중복 추가하지 않는다.
main에 없는 `authored.portal-arrival.1/.2` 복사본이나 58.81초 retiming은 이번 PR 대상이 아니다.
`build_kouku_gold_trails_restore.py`는 source 두 문서를 필수로 읽고 authored 복사본은 존재할 때만
같은 node·loop 보완 대상으로 처리한다. 나머지 저작 값과 정상 node를 보존한다.

`Effect_Playback.h/.cpp`의 기존 ELEMENT_STATE와 prepared 목록은 SourceTransformTrack과
VelocityInheritParent가 함께 있는 요소의 world origin 속도를 fixed step에서 계산한다.
속도를 birth simulation basis로 옮기고 원본 scale을 한 번 적용한다. source track 없는 기존
root/local/bone 속도 경로는 유지한다.

무대 앞 actor15~18의 4경로·24행은 독립
`effect.kouku.gate1.intro.gold-trails.full.restore`로도 제공한다. 첫 발생만 0초로 옮기고,
SL04 floor08 원본 위치 `[0,-73728,0]`cm를 preview origin으로 사용한다. 원본 경로와 상대 시각,
node scale 2는 보존한다. EffectCatalog·EffectResourceTree·Boss/Sequence Composition의 추가 가능
resource와 Client project/filter의 기존 96.DataFiles None 항목에 등록한다. resource duration은
tail을 포함한 9413ms다. 새 shader나 Resources payload는 필요하지 않다.

원본 26.9194545746초의 CONSTANT 위치 도약과 같은 시각의 즉시 camera cut은 보존한다.
독립 문서는 원본 카메라·Slomo 연출을 포함하지 않으므로 이를 임의 평활·속도 clamp로 숨기지 않는다.

검증은 source 연결·key 보존, node/loop 외 값 보존, 실제 codec/playback의 이동 거리·거리 방출·
world 잔광·ribbon·수명·seek 결정성과 속도 상속 경계로 제한한다. 공유 작업 폴더의 기존 Product
성공 기록과 main에서 분리한 이번 작업 폴더의 컴파일 결과를 구분한다. Client/UI 실행과 최종
색상·크기·움직임 판정은 사용자에게 남기며, 실제 결과는 같은 RESULT의 G06–G07에 기록한다.
