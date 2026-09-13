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

## G04. 09-13 실제 아레나 플레이어 등록 누락 교정

사용자가 확인한 오류는 `Enter an arena with a scene player before Play All. The player is the Effect anchor.`다.
`Select_SceneEffectTarget → Resolve_SceneCharacter`의 scene player가 없어서 V1 factory보다 먼저 거절된다.
현재 Bind 호출은 Character Select Level에만 있고 Kouku/Bern의 복제 player는 카메라·입력만 연결된다.
이 누락은 현재 HEAD에도 있어 최근 Effect Tool 파일 분할이 만든 새 셰이더 결함으로 단정하지 않는다.

기존 `CClientReplication`이 local player transaction을 실제 commit한 시점에만
`CAnimationTargetService::Bind`를 호출한다. local spawn과 class replacement 성공을 연결하고
실패는 기존 target을 보존한다. local despawn·Reset_World·destructor는 해당 local character만 Unbind한다.
remote player와 preview model은 바꾸지 않으며 destructor가 Layer를 다시 조작하는 경로는 추가하지 않는다.
Character Select의 기존 exact-pointer Bind/Unbind는 중복 호출에도 안전하므로 별도 재작성하지 않는다.

현재 festival 두 문서의 root attachment·발사 원점·material은 보존한다. saved P4 occurrence는
MAP 고정 위치이고 authored festival의 첫 발생은 33.40897초다. 독립 full.restore의 첫 발생은 0초다.
이 둘의 timeline을 합치거나 Play All을 위해 authored 시작 시각을 일괄 당기지 않는다.

현재 실제 codec/playback으로 festival·authored festival·fireworks를 다시 평가하고,
local 생성·교체·remote·실패·퇴장에 따른 실제 target 수명을 focused CPU 검사한다.
관련 번역 단위 컴파일 뒤 잠금이 없는 경우 기존 Product 빌드를 사용한다. 새 C++/harness/project 등록은 없고
Client/UI 실행·화면 판정은 사용자에게 남긴다. 결과와 한계는 같은 RESULT의 G06에 기록한다.

## G05. 무대 앞 금빛 이동 축포와 원본 Move 연결

새 첨부 이미지의 곡선형 금빛 잔광은 SCENE03A Matinee0의 `Par_Q_Trail_01`이다.
무대 앞 emitter15~18과 포탈 앞 4개 actor는 기존 sequence source/authored 문서에 존재한다.
`actor_groups`가 UE 이름을 대소문자 구분으로 비교해 `Pc01tr`와 `pc01tr`를 다른 그룹으로
판정했고, 6개 actor의 Move 곡선이 빈 배열로 저장됐다. 금빛 sparkle/tail의 SpawnPerUnit은
이동 거리가 없으면 생성되지 않으므로, 정지한 core만으로 원본 움직임을 대신할 수 없다.

`build_kouku_sequence_effect_groups.py`의 원본 그룹 연결을 UE 이름의 대소문자 비구분 계약으로
고친다. 원본 곡선 key/time/tangent와 기존 SourceTransformTrack schema는 유지하며 연결 실패를
정상 정지 위치로 통과시키지 않는다. 현재 `.1/.2` source와 대응 `authored.portal-arrival.1/.2`의
누락된 node 곡선만 보완한다. 사용자 저작 시작 시각, source clock 보정, 밝기, 재질, MAP 앵커와
정상 곡선은 유지한다. V1의 기존 birth transform history와 cascadeRibbonV1를 소비한다.
선택한 6 Required의 완전한 CDO 체인은 EmitterDuration=1, EmitterLoops=0을 사용한다.
임시 loop 1이 원본 Toggle보다 먼저 효과를 종료하므로 선택 trail의 SourceRecipe만 0으로 복구한다.
KillOnDeactivate/KillOnCompleted와 Matinee ON/OFF 수명은 기존 원본 값을 유지한다.

무대 앞 4경로는 독립 `effect.kouku.gate1.intro.gold-trails.full.restore` 문서로도 제공한다.
원본 경로·상대 발생 시각을 유지하고 첫 발생을 0초로 맞춰 Effect Tool에서 바로 확인할 수 있게 한다.
EffectCatalog, Sequence Composition의 추가 가능한 Effect resource, Client project/filter의
96.DataFiles None 항목에 등록한다. 기존 P4 occurrence가 소비하는 authored 문서도 함께 고치므로
기존 sequence는 저장된 시각·MAP 위치에서 같은 곡선을 재생한다.
`build_kouku_gold_trails_restore.py`는 이 4문서의 node 보완과 독립 문서 생성을 소유하며,
기존 library installer로 Catalog·Resource Tree·Composition의 추가 가능 목록을 연결한다.

원본 금빛 입자의 `VelocityInheritParent`도 고정 MAP root가 아닌 이동 emitter 속도를 필요로 한다.
기존 `Effect_Playback.h/.cpp`의 ELEMENT_STATE와 fixed step에서 해당 source track 소비자의
원점 속도를 계산해 기존 spawn 모듈에 전달한다. source track 없는 기존 root 속도 경로는 보존한다.
새 C++ 파일·shader·Resources는 필요하지 않으며, 변경한 기존 번역 단위와 제품 연결은 컴파일한다.

기존 CPU probe로 변경 전후의 emitter 이동 거리, SpawnPerUnit 생성, world-space 잔광과 ribbon,
seek 결정성을 대조한다. JSON/XML parse, importer 최소 검사와 diff 검사를 수행한다.
다른 세션의 MSBuild와 공유 출력 빌드를 겹치지 않는다. 이번 속도 상속 교정은 C++ 변경을
포함하므로 최종 제품 빌드를 수행한다. Client/UI 실행 및 최종 화면 판정은 사용자에게 남긴다.
