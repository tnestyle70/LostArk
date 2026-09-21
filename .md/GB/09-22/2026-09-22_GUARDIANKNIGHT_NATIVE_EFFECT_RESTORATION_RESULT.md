# 가디언 나이트 원본 이펙트·소환 모델 복원 결과

대응 계획은 [Character/Bone/Dragon/Guardian 계획](2026-09-22_CHARACTER_BONE_DRAGON_GUARDIAN_RESTORATION_PLAN.md)이다. 통합 Git, Product 빌드, 최종 배포 상태는 [바다 용·가디언 통합 결과](2026-09-22_ANCIENT_SEA_GUARDIAN_INTEGRATION_RESULT.md)를 따른다. 이 문서는 실제 완료한 Guardian particle·native material·model cue·저장 연결의 증거를 기록한다.

## 실제 반영 범위

PR #437 병합 기준선 위 통합 worktree에 입자·모델 43개와 owner-control 전용 1개를 합친 direct authored effect 문서 44개, EffectCatalog 행 44개, GuardianKnight.animevents의 asset effectref 44개와 Client 프로젝트의 Data None 참조를 반영했다. 원래 animevents의 2,203개 source 행은 유지했고 header를 v6으로 올려 총 2,247행으로 저장했다. 다른 캐릭터와 기존 사용자 저작 문서는 덮어쓰지 않았다.

최종 44문서에는 element 1,608개, Model Cue 23개와 Owner Control 40개가 있다. 실제 PlayerSkills의 Guardian 20개 스킬, skillbindings의 50개 슬롯을 조사했으며 두 슬롯의 같은 source stage 재사용을 제외하면 48개 source clip stage다. 이는 사용자 추정 약 18개와 달리 현재 저장소의 실제 ACTIVE·COMBO·변신·이동기 binding으로 센 수다. 원본 source action 번호로 등록했으며 비슷한 다른 직업 효과를 대신 연결하지 않았다.

| 저장 경로 | 실제 역할 |
|---|---|
| Data/Effects/Authored/effect.guardianknight.skill.*.full.restore.effect.json | 각 source stage의 기존 Effect 문서 계약 |
| Data/Effects/EffectCatalog.json | DIRECT_AUTHORED_DOCUMENT stable ID 44개 |
| Data/Animation/Authored/GuardianKnight/GuardianKnight.animevents | 실제 clip에서 effectref를 선택·재생하는 연결 |
| Client/Public/Effect_AuthoringDocument.h 및 Effect_DocumentCodec 계열 | 기존 Model Cue의 optional SourceMaterialProfile·clipPlayRate·COLOR4 roundtrip |
| Client/Private/Effect_Renderer_ResourceStaging.cpp·Catalog.cpp·Rendering.cpp | 기존 CModel → CMaterial source native profile 로드·clone·상수 track 소비 |
| Tools/EffectPipeline/decode_source_model_actor.py | 원본 compound PlaySkeletalMesh payload decoder |

실제 All Effects authored filesystem scan과 PlayerSkills join 결과 GuardianKnight category에서 신규 문서 44개와 스킬 20개를 확인했다. 설치된 GuardianKnight.wmodel의 실제 animation catalog는 165 clip이며, 새 effectref 44개 모두 이 모델 clip으로 resolve된다. Open Editor와 Play Effect의 기존 stable owner/linked-animation 경로를 사용한다. 이 검사는 실제 C++ consumer를 호출했으며 UI를 자동 실행하지 않았다.

## 원본 추출과 native shader

원본은 설치된 retail DRAGONKNIGHT.loa이며 SHA256은 `4b16616bb32f3f6eda1d10321ab8a0aa59527df5e1fce149c9d94add110a7fbe`다. LPK 원본에서 Action/LookInfo와 실제 object package를 추출하고 기존 UModel·SourcePack·particle decoder·native compiler·ModelAssetConverter를 사용했다. 원본 notify payload와 byte offset/SHA, typed distribution, original socket을 후보 receipt에 보존했다.

| 생성 범위 | 완료 수 |
|---|---:|
| main particle material permutation | 449: 신규 336 + 정확히 재사용 113 |
| main native distortion | 185 |
| nested CEF/camera particle material permutation | 45: 신규 18 + 정확히 재사용 27 |
| auxiliary native distortion | 13 |
| PostProcessChain native program | 2: 4304, 4305 |
| ALT+V StaticMesh native program | 2: 4530, 4531 |
| 소환 모델 SourceCharacter native program | 8: 101~108 |

신규 particle native program은 main 3968~4303, auxiliary 4336~4353, post 4304/4305, static 4530/4531의 총 358개다. per-route 재사용 수를 distinct source material 수로 합산하지 않는다. main과 auxiliary 모두 deferred source material은 0이다. Shader family descriptor의 기존 group별 범위는 유지하고, 실제 runtime admission의 전체 상한만 새 program 범위와 일치시켰다.

main particle 요소 1,467개, nested CEF/camera 요소 122개, PostProcess 요소 12개, StaticMesh 요소 7개를 합쳐 최종 1,608개다. enabled/disabled 원본 선택을 보존했다. source ERM_None/Point의 비표시 emitter는 임의 sprite로 표시하지 않고 기존 provider로 유지하며, 같은 source emitter의 typed light는 별도 LIGHT consumer로 연결한다. 원본 vector field의 typed asset도 기존 wvectorfield 경로로 저장했다.

source ScalarRand의 scalar min/max typed 위치를 고쳤고 vector constant를 scalar로 오독하지 않게 했다. 역사적 native reuse에는 실제 원본 scalar/vector/texture 일치와 정확한 texture SHA alias 검증을 유지했다. candidate Resources 경로를 generator에 명시적으로 전달하여 존재하는 texture를 없다고 판단하던 경로 혼선을 없앴다. 누락 검사를 제거하거나 미지원 재질에 다른 shader를 대입하지 않았다.

모션 블러 4006/4060의 원본 matrix ABI와 SINCOS source-alias 수정 및 108조건 원본 DXBC 비교는 통합 결과에 기록했다. 그 검증을 다른 모든 native program의 pixel 동일성 검증으로 확대하지 않는다.

## V·Alt+V의 소환 모델

V의 DRR body/head/neck 세 재질과 Alt+V의 DRG body/head/neck/wing1/wing 다섯 재질을 원본 source material 계산으로 연결했다. Model Cue 11개가 이 8개 모델 section을 재사용하며 원본 source animation과 dead 등 scalar/COLOR4 track을 소비한다. 그 밖의 Model Cue 12개는 별도로 복원한 live owner afterimage다.

source profile은 existing WorldSequence material profile을 재사용한다. resource staging에서 profile을 MODEL_MATERIAL_OVERRIDE로 변환해 CModel을 만들고 animated material variant를 생성한다. 재생 중 track sample을 Configure하여 해당 owner material 상수에 반영한다. 원래 prototype과 다른 owner의 재질은 보존하며 Clear로 원형 값을 복구한다. 같은 역할의 별도 model renderer는 만들지 않았다.

WModel 내부 기본 diffuse/normal/ORM/emissive 참조 28개가 최초 JSON resource 검사에서 빠진 사실을 실제 CModel 생성으로 발견했다. CMaterial 기본 입장이 native override보다 먼저 수행되므로 native texture 목록만 검사해도 모델 생성은 실패할 수 있다. 정확한 cooked DDS 28개를 추가한 뒤 8개 CModel의 native program 101~108, source constants, animated variant, 원본 clip 선택, override 적용값, prototype 불변과 Clear 원복을 모두 확인했다. 이 검사는 GPU device를 가진 실제 모델 생성/재질 소비 검사이며 전체 원본 PBR pixel 비교나 사용자 화면 판정은 아니다.

## 실제 bone과 source lifetime 0

Guardian 외부 attachment는 실제 모델의 import root와 preScale에 맞춰 기존 bone scale normalization 경로를 사용한다. 새 Model Cue 자체의 DRR/DRG bone과 camera attachment에는 이 외부 character 보정을 중복 적용하지 않는다.

source Lifetime 0을 양수 임의값으로 바꾸지 않았다. 원본은 normalized age 0으로 유지되어 원본 color/size over-life의 첫 값을 소비한다. 단, 그 source occurrence가 끝나면 입자를 정리한다. 처음 구현은 source0 입자가 effect 문서 전체 끝까지 남았고, 실제 bound bone/clip probe에서 이를 검출했다. 각 element의 source 끝과 owner reset을 따라 제거하도록 고쳤다.

실제 Guardian CModel의 bip001-l-hand와 원본 ddk_sk_flame_scale_01/02를 사용한 Seek_WithTransformHistory 검사 28조건을 통과했다. 시작 후 .05/.2초는 active 1·normalized age 0·alpha 1, 각 source notify 끝 이후는 active 0이며 explicit owner Reset도 frame을 비운다. synthetic anchor count만으로 부착 성공이라고 기록하지 않았다.

## 최종 데이터·Resources 검증

| 실제 실행 검증 | 결과 | 로컬 증거 |
|---|---|---|
| CEffectDocumentCodec load → Validate_Drawable → serialize → parse canonical roundtrip | 44문서, 1,608element, 23ModelCue, 40OwnerControl PASS | out/GuardianEffects20260922/cpu/owner-44-final-pass.log |
| native model parameter track sample | 66 sample 모두 finite | 같은 codec log |
| StaticMesh wrong asset / particle kind / missing SourceTransformTrack | 3개 모두 reject | 같은 codec log |
| 실제 authored index + PlayerSkills + WModel clip + animevents join | index44 / skills20 / clips165 / linked44 / unresolved0 | out/GuardianEffects20260922/cpu/editor-owner-result.log |
| 실제 native CModel create + animated material variant + source override/Clear | 8개 PASS, native program101~108 | out/GuardianEffects20260922/cpu/model-result.log |
| source0 실제 Guardian bone/clip 및 owner cleanup | 28조건 PASS | out/CharacterWorkbench20260922/zero-lifetime-result.log |
| Sea 최신 7문서 같은 C++ codec | 121element / 1afterimage PASS | out/GuardianEffects20260922/cpu/sea-validation.log |

최종 Resources 참조는 731개다. 새 파일 230개/124,457,360 bytes를 Desktop 저장소와 integration worktree Resources 양쪽에 설치했고 기존 501개는 같은 SHA256을 확인했다. 기존 파일 변경은 0개다. 이 수에는 actual WModel에서 추가로 발견한 28개 DDS가 포함된다. 전체 731개를 새 리소스로 부풀리지 않는다. 분류 정본은 `out/GuardianEffects20260922/installation/resource-manifest.json`이며 `existingSame=false`가 이번 배포 대상이다. 최종 전체 GBResources 묶음 경로와 hash 결과는 통합 결과를 따른다.

신규 43문서의 JSON과 프로젝트 XML parse, 관련 C++ consumer compile 및 git diff --check를 실행했다. 최종 Product 빌드는 parent 통합 결과에서 별도 기록한다. unrelated 기존 direct source index의 unavailable 12행과 owner 없는 기존 editor-only row를 이번 Guardian 43행의 실패로 감추거나 전체 repository 통과로 표현하지 않는다.

## 입자 문서가 없는 source stage와 남은 제어 경계

48 distinct bound source stage 중 다음 5개에는 enabled particle carrier가 없다. 이를 애니메이션 전체가 비어 있다고 표현하지 않는다.

| source stage | particle 문서가 없는 원본 이유 |
|---|---|
| 49110 stage1, ddk_sk_wildupper_01 | AK sound와 gameplay/action metadata만 존재 |
| 49120 stage1, ddk_sk_thrust_01 | 원본 particle notify10개가 모두 disabled; sound와 PawnMaterialParam은 별도 |
| 49130 stage1, ddk_sk_guillotinespin_01 | 원본 particle notify5개가 모두 disabled; sound와 PawnMaterialParam은 별도 |
| 49150 stage1, ddk_sk_grandfinale_01 | AK sound와 gameplay/action metadata만 존재 |
| 49330 stage1, ddk_sk_deepimpact_02 | ViewShake·HidePawn 등 제어만 존재 |

최종 source audit에서 PawnMaterialParam42·DominantDirectionalLight_Control6의 48개 중 28개가 활성 제어이고, 14개는 source disabled, 6개는 원본 empty no-op였다. 활성 material23·light5와 IdentityParts9·HidePawn3을 합친 40개 Owner Control을 24stage에 설치했다. UltimateSkillCameraControl2는 기존 Alt+V effectsequence 카메라로 연결하며 실제 Character 소비·종료와 camera 검증은 각 담당 결과를 따른다. 원본 payload inventory는 `out/GuardianEffects20260922/remaining_owner_controls.json`이다. 입자·모델 검증, owner control 문서·clock 검증과 실제 Character/camera 소비자 검증은 각각의 증거로 구분한다. 후속 완료 상태는 [통합 결과](2026-09-22_ANCIENT_SEA_GUARDIAN_INTEGRATION_RESULT.md)를 따른다.

원본 Effect61과 ParticleHit17 같은 gameplay_reference는 Server 전투 정본과 별개인 읽기 전용 원본 자료다. Client에서 피해/충돌을 새로 판정하지 않는다. 기존 animevents의 sound/shake source 자료도 검증 범위를 따로 기록한다.

TrailGhost12개는 원본 timing·flags·source RGBA와 live owner 장비/무기 pose를 기존 CSkeletalAfterimage로 소비한다. 원본 UE rim/fade shader ABI가 확인되지 않아 appearanceBasis는 PROJECT_AUTHORED다. 자세·시점 복원과 원본 pixel appearance를 구분한다.

Client나 editor UI는 자율 실행하지 않았다. 사용자 화면에서의 최종 이펙트 방향·크기·가시성 및 편집 조작 판정은 통합 인수 확인에 남긴다.


## 기존 source sound·shake와 owner control 편집 검증

48개 선택 source stage의 ViewShake 48개를 기존 animevents와 대조했다. clip·시작 시각·duration/fade·XYZ 진폭/주파수·FOV가 모두 일치한다. 원본 회전 lane은 전부 0이므로 회전 shake를 임의 생략한 사례는 없다. 기존 CCharacter::Update_CameraShakeCues가 실제 action clip wall-time과 로컬 플레이어의 shake 설정을 소비한다.

AKEvent 114 occurrence 중 네 행의 시작 시각이 Action LOA와 달랐다. wildUpper03 Vox는 250→150ms, eurosloof03 FX는 100→0ms와 Vox 1→200ms, deepimpact03 FX는 200→180ms로 기존 src=orig 행의 startms만 교정했다. 최신 디스크 hash 재확인·백업·원자적 교체로 다른 source/asset cue를 보존했다. 최종 114개가 모두 clip·event·시각으로 일치하며 중복 사운드 행은 추가하지 않았다. 증거는 `out/GuardianEffects20260922/existing-shake-sound-coverage.json` 및 `installation/sound-timing.receipt.json`이다.

unique sound event 97개는 Guardian의 playable 95개, Common Ultimate 1개, 원본 Stop-only 1개다. Stop-only에는 재생할 WAV를 만들지 않았다. 기존 다운로드된 WAV 519개/122,715,614 bytes를 원본 Resources에서 확인했고, 격리 worktree에 빠져 있던 정확한 파일만 복사해 양쪽 SHA256을 확인했다. 모두 기존 리소스 재사용이며 신규 원본 파일 수는 0이다. 이 WAV를 앞의 신규 230개 수나 GBResources 신규 묶음에 합산하지 않는다. 증거는 `installation/reused-sound-resources.receipt.json`이다.

Owner Controls 편집은 기존 Effect Tool의 Current Effect 트리에 추가했다. stable ID 선택·추가·삭제, kind/대상/시작시각/키 시간·값·가시성을 변경하면 기존 Try_CommitDocument의 validate → stage → commit을 거쳐 같은 Save 문서와 preview clock에 반영한다. 다른 unapplied detail draft가 있으면 이를 보존한다. raw SourceValues는 읽기 전용으로 표시한다. 새 별도 저장 형식이나 가짜 particle은 만들지 않는다.

ownerControls의 마지막 키까지 playback/preview duration을 계산하며 실제 제어 전용 문서의 Validate_Drawable을 허용한다. component assembly split/compile 양방향에서도 ownerControls를 보존한다. 제어 전용 및 particle 혼합 문서의 곡선·깊은 복사 보존을 포함한 Python regression 30개를 통과했다. 최신 실제 C++ Codec/Playback의 5개 kind fixture는 serialize/parse roundtrip, 중간 재생, 정확한 종료, 생성 particle/element 0개를 통과했고 duplicate-time/unknown-kind/빈 문서 변조 15개는 모두 거부했다. 이 fixture 결과와 실제 source controls가 들어간 최종 문서 결과는 구분한다.


최종 Owner Control 설치는 기존 effect 문서 23개에 필드만 추가하고, 49330 clip1에 가짜 particle 없는 제어 전용 문서 1개를 추가했다. 기존 element/model/native field는 그대로 보존했다. 새 control-only v15 후보는 runtimeExtensions가 빠져 actual codec에서 처음 거부됐고, 정식 빈 runtimeExtensions를 명시한 뒤 같은 codec/clock 검사를 통과했다. 검증 조건을 낮추지 않았다. Catalog의 44번째 stable ID, animevents의 시작 cue, 신규 effect와 Alt+V effectsequence의 프로젝트 None 등록까지 최신 디스크 SHA를 확인해 28파일을 원자적으로 반영했다. 증거는 `out/GuardianEffects20260922/owner-install/installed.receipt.json`이다.

설치 후 실제 All Effects filesystem index·PlayerSkills·WModel animation·animevents join을 다시 실행했다. `index=44, skills=20, actualModelClips=165, linkedAnimationEffects=44, unresolved=0`을 통과했다(`cpu/editor-owner-result.log`). 기존 source sound 4행 시각 교정과 모든 무관한 기존 행은 이 추가 설치에서 보존했다.

최종 재검사는 44개 설치 문서 전체를 한 실행에서 처리해 `documents=44, elements=1608, modelCues=23, sourceParameterSamples=66, ownerControls=40`, exit 0을 확인했다(`cpu/owner-44-final-pass.log`). 검증한 후보 44개와 실제 설치 파일의 SHA256도 모두 일치한다.


## Native named vector 제어와 기본 스택 검증

`SourceCharacterMaterial::Patch_NamedVector`는 현재 61개 Configure family의 실제 직접 대입식에서 TransColor/BuffColor의 Base/Light 상수 위치를 생성한다. 프로그램 80~83의 family 뒤 공통 대입도 순서대로 반영한다. 현재 지원하는 program/parameter 조합은 116개이며 해당 두 이름의 Light 직접 읽기는 없다. 원본에 없는 Light 상수나 AUTO/program 0, Map program 33~79에는 값을 추정해 넣지 않는다. 다른 계산식·mask·UV·program 값은 보존하고 미지원 이름이나 유한 범위를 벗어난 값은 무변경으로 거부한다.

vehicle source material installer는 source packing을 검사한 뒤 helper를 재생성한다. 직접 vector 복사와 배열 복사만 추적하며, 다른 수식 안의 파라미터 또는 부분 lane 덮어쓰기는 추정하지 않고 생성을 중단한다. 원본 61개 lambda packing의 모든 바이트가 helper 추가 전과 같음을 확인했다. 이를 통해 default 1MB 스택 overflow를 고친 family 분리도 보존했다.

실제 C++에서 61개 family의 원본 Configure와 116개 named-vector 변경 후 전체 재계산 결과를 MODEL_SOURCE_CHARACTER_PARAMETERS 전체 바이트로 비교했다. 734 checks / 116 patches / 0 failures였으며 PE SizeOfStackReserve는 1,048,576 bytes다. 비정상 값·미지원 program/name의 무변경도 확인했다. 직접 복사·공통 대입·간접 수식 거부·부분 덮어쓰기 거부·생성 일치의 Python 회귀 검사는 6개 PASS다. 증거는 `out/GuardianEffects20260922/stack-packing-audit/named-vector-result.log`, `named-vector-preservation.receipt.json`, `named-vector-bindings.json`이다. 실제 Character의 재질 대상 선택·owner 취소 복구 검증은 렌더링 담당 결과와 구분한다.

기존 base GuardianKnight body의 5개 AUTO 슬롯은 이번 helper에서 native로 임의 변경하지 않았다. #437의 `GUARDIANKNIGHT_RESTORE_SYNC_RESULT`에는 eye program5가 source EXACT여도 WModel1.0의 UV1/UV2 부재로 전체 모델 admission에 실패했고, eyeAO/face는 원본 추출기 미지원 경계가 남았다고 기록돼 있다. 기존 native99 hair 및 native 장비/무기, 이번 DRR/DRG101~108만 해당 원본 이름을 실제 소비할 때 patch가 적용된다.
