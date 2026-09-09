# 베른 국소 조명과 세 클래스 핵심 스킬 복원 구현 계획

2026-09-09 사용자 요청 범위다. `codex/dimensionmaster-tool-round3`의 기존 미커밋 변경을 보존한다. 베른 조명, Artist, Warlord의 세부 계획은 같은 날짜의 해당 IMPLEMENTATION_PLAN을 정본으로 사용한다.

## G00. 복원 범위와 실제 소비 경로

베른은 캐릭터 스폰과 현재 카메라 연출 경로에 닿는 원본 조명부터 연결한다. 지형 전체 노출, 전역 재질과 물 복원은 이번 국소 작업에 포함하지 않는다. `MapCatalog → maplights authoring → map publisher → Level_Bern → shared frame light`를 연결한다.

스킬은 창술사 V34610/Alt V34630, 워로드 A17090/S17040/F/V, 도화가 D31490/T31950/V31910/Alt V31930을 대상으로 한다. 기존 unified와 사용자 튜닝은 유지하고 Effect Tool이 이미 검색하는 같은 이름의 `.full.restore` 문서를 사용한다. 워로드 S의 기준은 정면으로 나가는 파란 방패 세 개다.

## G01. 원본 입력에서 셰이더 출력까지

설치 원본 Action의 활성 발생과 첫 LOD를 정확한 stage별로 읽는다. MIC와 부모의 실제 텍스처·수치·static switch를 회수하고 해당 vertex factory의 compiled VS/PS를 선택한다. 차원술사 cube/glass 복원에서 사용한 DXBC 계산 변환과 source-profile 검증을 재사용한다. 같은 부모 이름이나 근사 렌더링을 원본 계산의 증거로 삼지 않는다.

클래스별 전용 header/HLSLI는 원본 프로그램과 입력 계약을 소유한다. 공용 `Effect_MaterialTemplate`, `Effect_DocumentCodec`, `Effect_DocumentRenderer`, mesh/particle shader는 기존 native 분기에 이 계약을 연결한다. 추가 C++ header와 HLSLI, authored JSON은 `Client.vcxproj` 및 기존 대응 filter에 등록한다.

## G02. 도화가 skeletal 모델 재질

기존 `EFFECT_MODEL_CUE_DESC → Stage_ModelCueResource → CModel clone → Render_ModelCues`에 선택적인 원본 material 입력을 연결한다. 모델과 clip은 기존 CModel 경로로 로드한다. 기존 modelCue에 새 입력이 없으면 현재 동작을 유지한다. JSON은 parse/validate/stage/commit을 따르고 실패한 교체는 현재 Preview를 보존한다. 새로운 모델 런타임을 만들지 않는다.

## G03. Solo 재생과 제외 분류

복구한 행은 실제 Codec 읽기/저장, renderer 준비, 고정 시간축의 발생과 finite 입력, 필요한 shader 컴파일로 확인한다. 재질, geometry, resource, attachment, source 입력이 끝내 닫히지 않은 행은 source ID와 이유를 RESULT에 분리하고 새 full restore의 elements에서 제외한다. 사슬·방패·번개·호랑이·용 같은 핵심을 제거해 복원 완료로 간주하지 않는다. 원본 비교 자료와 Resources는 삭제하지 않는다.

## G04. 통합 검증과 사용자 확인

베른 publisher의 해당 map 검증과 publish, 변경 JSON/XML parse, 필요한 최소 C++ 컴파일과 shader 컴파일, `git diff --check`를 수행한다. 현재 사용자가 실행한 Server/Client는 종료하거나 조작하지 않는다. Product link가 실행 파일 잠금으로 불가능하면 컴파일 결과와 잠금을 구분해 기록한다.

Client 화면 실행·조작·캡처와 최종 시각 판정은 사용자 소유다. 실행 준비 뒤 `Server + Client` profile의 Ctrl+F5, Bern 진입 또는 Character Select의 해당 클래스와 F1 Effect Tool의 full restore Solo/Play All 경로를 전달한다. RESULT는 구현, 자동 검증, 미실행 화면 확인, 남은 복원을 분리한다.

## G05. 세이튼 등장 공의 텍스처 연결

추가 요청의 실제 경로는 `KAKULSAYDON_G1_PATTERN_8 → world.object.instance.kouku.ball_bounce → world.object.kouku.ball → Effect/KoukuSaydon/Meshes/wp_mn_rhcn_00/mesh/fm_d_rhcn_00.wmodel`이다. 현재 source World revision412의 타이밍·배치·diffuse override를 보존한다.

모델의 WMA2 material0 `mn_rhcn_00_mi`에 원본 MIC가 명시한 `mn_rhcn_00_d/n/s/e.dds`를 연결한다. 네 파일은 이미 `Effect/KoukuSaydon/Textures/MN_RHCN_00/tex`에 있다. 기존 CModel의 WMaterialReader와 CMaterial, WorldSequenceObject의 MapAssetRenderUtils가 normal/specular/emissive를 실제 소비한다. 모델의 mesh section과 dummy material은 바꾸지 않으며 material section의 경로만 갱신한다. 이 추가 요청은 원본 PBR shader 전체나 emissive 시간 계산까지 복원했다고 주장하지 않는다.

검증은 바뀐 material 경로와 DDS 실물, mesh section 보존, 실제 CModel material 로드로 확인한다. Resources는 Git에 추가하지 않고 필요한 물리 위치와 변경 모델을 RESULT에 기록한다.

## G06. 소환 모델과 최종 우선순위

창술사 Alt V의 말은 원본 `SK_FLM_HOR_00_SK` 소환이다. 원본 세 단계 clip과 네 material section을 기존 skeletal CModel/ModelCue로 연결한다. 각 section을 같은 skeleton과 animation을 가진 모델로 cook하고 해당 source 재질을 하나씩 적용한다. 도화가 호랑이·용과 함께 모델 이름만 있는 상태, 모델에 없는 clip, source 호출에서 빠진 mesh와 animation을 대조하고 필요한 Resources를 추가한다.

사용자의 마지막 범위 조정에 따라 카메라 이동·회전·FOV·shake 및 sequencer 확장은 다른 세션에서 진행한다. 이 작업은 해당 파일을 수정하지 않고 확보한 원본 조사 자료만 보존한다. 스킬의 실제 입자·소환·mesh 연출 복원은 계속한다. 맵은 이미 구현한 베른 국소 조명과 세이튼 등장 공 텍스처의 검증까지만 수행하며, 발탄·쿠크의 신규 조명·안개 전수조사와 베른 전역 확장은 다음 작업으로 미룬다.

## G07. 원본 ScreenPost의 실제 소비

현재 Engine의 `PRESENTATION_SCREEN_POST_DESC::pMaterial → IPresentationScreenPostMaterial::Bind → Renderer::Render_ScreenPostPass`는 준비된 재질과 scene color/depth를 소비한다. 새 효과의 native ScreenPost는 이 경로에 연결한다. Client에서 immutable frame 입력과 source texture/parameter를 준비하고 기존 typed submission에 전달한다. 원본 shader 파일 존재나 profile 이름만으로 generic noise/blur가 원본 계산을 실행한다고 판정하지 않는다.

## G08. 2026-09-10 전체 스킬과 소환 후속 요청

워로드·도화가·창술사의 현재 LMB와 quick-slot 전체를 source action/stage별 full restore로 확대한다. 사용자가 승인한 워로드 F와 창술사 V clip2의 기존 문서와 재질 계산을 보존한다. 각 클래스 전용 generator/header/HLSLI와 full JSON이 원본 발생·재질을 소유하며, 공용 shader family와 프로젝트 등록은 통합한다. 실제 소비 가능한 full 문서는 기존 EffectCatalog와 animevents 경로까지 연결한다.

도화가 미르새김은 `SK_SDM_DRA_00`의 geometry·skin·clip과 바닥 전진 transform을 원본 자료로 대조한다. D 호랑이는 `SK_SDM_TIG_00`의 시간별 실제 pose 변화를 확인하고 모델만 나오는 원인을 교정한다. 창술사 Alt V는 말의 네 material section 및 세 clip을 보존하고 전체 문서 로드를 막는 SubUV Movie를 기존 source particle 경로에서 처리한다. 도화가 Alt V 꽃밭은 다른 스킬보다 후순위로 검토하되 미지원 발생을 정상 발생과 분리한다.

워로드 V는 source 방패 wmodel 다섯 방향과 caster를 감싸는 방패를 연결한다. Alt V는 큰 원 여섯 개, caster 주변 여섯 개의 독립 stable element로 배치한다. 정상 지연 발생이나 필요한 model dependency를 미지원 Solo와 혼동해 제거하지 않는다.

차원술사 dust는 현재 Q51 기반 black-core/clean-core 저작 보강의 소비 문서를 확인한 뒤 사용자 요청의 은은한 보라 경계 표현으로 조정한다. 원본 native 식과 사용자 저작 보강은 분리한다. 공포는 실제 캐릭터 모델의 clip과 Shared/Server/Client 상태 소비자를 조사해 존재하는 모션과 없는 gameplay 상태 정의를 구분한다.

추가 C++ 파일 없이 기존 consumer를 확장하고, 추가 header/HLSLI/FX/JSON은 기존 물리 경로와 project/filter에 등록한다. 변경 JSON/XML, 실제 codec 저장·Solo, 시간별 모델·입자 수치, 필요한 C++/FX 컴파일 및 최종 링크를 검증한다. 실행 중 Client/Server는 조작하지 않는다. 사용자 화면 확인과 Drive 전달은 자동 검사와 별도로 RESULT에 남긴다.

## G09. full 광원과 도화가 꽃밭의 원본 정체 추가 감사 — 2026-09-10

사용자가 광원의 전체 복원 여부와 꽃밭 구성의 정체를 추가로 질문했다. 현재 물리적으로 존재하는
full106문서를 읽고 typed light 연결 여부를 선택된 source first-LOD 및 제외·cleanup 전 자료와
대조한다. Warlord V 통합 검토 문서와 실제clip의 중복은 별도 집계한다. 미선택 action stage와
현재 full이 없는 스킬을 기존 full의 누락으로 더하지 않는다. 입자 광원 외 directional-light control,
PostProcessChain 등 action-level 제어는 입자 후보 수와 분리한다.

광원 연결 행마다 enabled/visibility/range/intensity뿐 아니라 원본 initial color/alpha/lifetime과
typed 값 및 실제 Playback/LightPresentation/Engine shader 소비를 확인한다. 빛나는 sprite,
emissive 재질, bloom 결과를 주변 표면을 비추는 point light로 집계하지 않는다. original radius와
brightness는 상속 component 및 원본 size 연동 규칙까지 확인한 범위만 확정한다.

발견한 일반 pass와 source-character pass 사이의 transient light 소비 수명 문제는 실제 호출 순서와
marker 분기를 교차 검토한다. 후속 수정에서는 광원 목록을 모든 light pass가 끝날 때까지 유지하고
성공·실패의 정리를 프레임 소유자에게 둔다. source particle의 개별 발생·이동·색·수명·반경은
기존 Playback 안에서 연결해야 하며 typed 기본값만 일괄 치환해 원본 복원 완료로 표시하지 않는다.

꽃밭은 두clip의 전체222행과 제외39행을 원본 발생계열로 나누고 실제 WModel topology와
texture atlas UV를 대조한다. 꽃 묶음·낱꽃잎·나비·바닥 풀꽃·카메라 연출·조명·후처리를
분리해 설명한다. source sibling particle provider가 없는 Field_01을 static 대체로 승인하지 않으며
원본 burst와 runtime maxParticles 차이도 기록한다. 이번 검토의 수치·확정 경계는 대응 RESULT에
남기고 구조 구현·실제 FPS·사용자 시각 결과는 별도 상태로 관리한다.


## G16. 사용자 튜닝 후 도화가·워로드 연출과 입력 연결 (2026-09-10)

사용자의 저장 완료 답변 뒤 Artist/Warlord full 문서와 animevents 48개를 `out/ArtistWarlordVisualFollowup20260910/before`에 보존했다. 원작과 현재 화면 일곱 장을 열람했으며 저장 위치·각도·크기 중 이번 결함과 관계없는 값은 유지한다. G29 단일 element Sequencer 코드는 컴파일 완료/미배포 상태로 보존한다.

도화가 하늘/꽃밭 연출 대상은 정확히 `31930.clip2.full.restore`(Alt V)다. 몽유도원 V는 `31910.full.restore`이며 두 문서를 혼동하지 않는다. source action의 animation/camera/sky/field 자료를 현재 CModel·Camera override·Effect row 경로에 연결한다. 카메라만 사용하는 연출을 Server 좌표 이동이나 전역 맵 교체로 구현하지 않는다. 현재 원본 pose/카메라 데이터로 입증할 수 있는 범위를 사용하고 원본이 미해독인 부분을 임의 native 복원으로 기록하지 않는다.

`31910` Sprite Particle29의 stable ID는 `authored.source-particle.artist-full.42875d1a88aaef7067025288`이며 원본 EPAL_Z 축잠금과 billboard renderer가 최종 quad 방향을 소유한다. 일반 Transform rotation이 그 방향을 바꾸지 않는 경로를 확인했다. 기존 Particle Billboard/Transform 설정으로 바닥 plane을 유지하면서 명시적 회전을 소비하게 할 수 있는지 실제 codec/geometry로 검증하여 수정한다. 동일 시점의 두 카메라 사진 자체만으로 원본 축잠금을 오복원으로 단정하지 않는다.

Cone45/46은 `31930.clip2`의 `fa3288742e1ed6b222eff195`와 `fbb4539c2ea12fd35cf42374` 두 source-particle 행이다. 실패는 mesh 부재가 아니라 CAMERA_VIEW FOLLOW를 bone/animation FOLLOW와 같은 경계로 거절하는 Portable copy 조건이다. `Effect_DocumentCodec.cpp`의 기존 Build_PortableAuthoredElementStartingCopy에서 현재 카메라 자체로 닫히는 부착만 보존해 복사 가능하게 한다. 모델 cue·bone·trail·transform-master 의존은 계속 검증하며 문자열 prefix만 같은 것으로 owner 호환을 가정하지 않는다. 기존 Saved Element Add 소비자가 그대로 사용한다. 실제 함수로 두 행을 V31910에 추가하고 start delay와 source emitter delay를 0초에 맞추며 기존 위치/회전 payload를 보존한다.

미르새김은 `31950.full.restore`의 skeletal 용 cue와 별도 helixline 세 emitter를 구분한다. cue 스케일이 독립 element를 바꾸지 않는 것은 정상이며, 본체 geometry·animation palette·재질 alpha와 실제 모델 크기를 source부터 검증한다. 손으로 맞춘 cue 위치/방향을 보존하면서 실제 누락 원인과 폭을 교정한다.

워로드 V/Alt V는 현재 native1124 masked 방패와 source PBR01/02의 별도 모델/반사·BRDF 입력을 구분한다. 기존 CModel/CMaterial 및 Effect의 native material 소비자를 확장하며 새 모델 런타임을 만들지 않는다. 워로드 F의 확인된 낙뢰를 V full의 독립 stable occurrence로 추가하고 사용자 요청의 노란색·증가 수량을 저작 튜닝으로 기록한다.

변경 전 파일별 소유권을 나누고 공용 Renderer/Playback/Codec/shader는 root가 조정한다. 신규 파일은 실제 소비가 필요한 경우에만 project/filter 등록과 함께 추가한다. 데이터는 baseline과 직전 파일을 대조하여 동시 사용자 저장이 있으면 해당 현재 값을 기준으로 병합한다. 검증은 변경 문서의 실제 codec/저장, 필요한 모델/particle/material 수치와 shader 비교, 최소 C++/FX 컴파일, JSON/XML parse, scoped diff check다. Client 종료 확인 전 EXE 최종 링크·배포와 UI/화면 조작은 수행하지 않는다.


## G17. 워로드 본체·장비·무기 원본 재질 연결

사용자가 방패 외 워로드 자체의 재질·조명 복구도 명시했다. 현재 CharacterCatalog의
WARLORD 본체1·장비6·무기2모델, 실제사용14재질의 source program 소비가0이다.
정확 MIC 계층과 shader-map은 CharacterMaterialParity20260909의 원본조사로 확보되어
있으므로 현재 binary material 이름/section과 다시 대조하고 동일 PS pair는 기존
SourceCharacter 경로에 연결한다. 새5종의 pair는 원본계산·uniform packing만 확장하고
기존9종의ID/계산을 보존한다. 이름유사나 가족명만으로 다른 PS를 대신 연결하지 않는다.

CharacterCatalog의 WARLORD modelMaterialOverrides가 실제 CModel→CMaterial→기존
SourceCharacter GBuffer/light pass로 전달하는 정본이다. 필요한 exact texture를
Resources/Character에 놓고 UV·sampler·색공간과 재질상수를 원본에 맞춘다. body/equipment/
weapon의 기존 render consumer와 point/directional light수신을 수치검증하며 자산변경은
재질입력으로 제한한다. 캐릭터Transform·skeleton·사용자 스킬tuning은 보존한다.

Engine source program 상한과 Client parameter compiler·HLSLI의새switch를 같은변경으로
연결한다. 미결정 engine input을0으로 위장해새프로그램을활성화하지 않으며 원본재질의
재사용2D반사와scene-ownedcube를구분한다. 검증은 실제catalog stage/적용/CModel재질
로드·필수shadercompile·검증용수치입력에대한finite출력과직접광응답, 최소Client/Engine
compile, JSON/XML parse/diff check다. 실행중Client/Server는사용자종료전건드리지않는다.

## G20. 쿠크 F1 시야 설정의 실패 재로드와 갈고리 admission 교정

사용자가 1관문 idle 3.5FPS와 CPU 288.948ms 화면을 제공했고 F1과의 연관을 확인했다.
현재 MainApp의 시야 설정은 `gazeLoaded`에 성공 여부와 최초 시도 여부를 함께 저장한다.
새 GRAB_TO_WORLD_OBJECT가 Client outcome 목록에 없어 Reload가 실패하면 매 프레임
446,862-byte Composition 전체를 다시 읽는다. 수정 전 실제 C++ Reload 12회가 모두
logic.28의 Unknown outcome으로 실패했고 평균 254.665ms였다.

`Client/Private/MainApp.cpp`의 기존 시야 설정 블록에 `gazeLoadAttempted`를 둔다.
최초 시도 전에 true로 기록하고 이후에는 명시적 Reload Sight Settings에서만 재시도한다.
성공한 `gazeLoaded`는 값 읽기와 저장 가능 여부를 계속 소유한다. 실패 메시지와 last-good
보존은 기존 CKoukuSaydonCompositionDocument가 유지한다.

`Client/Public/KoukuSaydonCompositionDocument.h`의 기존 outcome 배열에
GRAB_TO_WORLD_OBJECT를 추가한다. `Client/Private/KoukuSaydonCompositionDocument.cpp`는
기존 outcome 연결 검증에서 해당 결과를 ENTER_AREA의 SUCCESS에만 허용한다.
percent/duration/followup/grip 등 불필요한 값의 기존 거절은 유지해 Server/publisher와 맞춘다.
Source JSON, 갈고리 경로, Sequence Viewer 목록과 게임플레이 판정은 변경하지 않는다.

새 제품 파일과 project/filter 등록은 필요 없다. 실제 Reload의 수정 전후 결과와
잘못된 outcome/slot의 거절·실패 시 이전 문서 보존, 해당 Client 최소 컴파일,
PR 범위 diff check로 확인한다. 실행 중인 Client는 유지하고 최종 링크는 사용자가 종료한
뒤 진행한다. 프레임 회복의 최종 화면 확인은 사용자가 한다.

## G21. 병합 후 쿠크 Play 입력과 실행용 데이터의 불일치 교정

Play는 선택 패턴을 기존 단일 Bundle 배우 경로로 전달한다. 요청·선택 세션에는 누락이
없으나 incoming PR의 맵 배치137개가 실행용 맵에 배포되지 않았다. Authoring3368개와
runtime3231개의 차이 때문에 WorldSequence의 source_ball_679 참조가 runtime에서
해결되지 않고 전체 문서가 거절된다. 기존 Area publisher로 맵과 연결 문서를 함께 배포하고,
WorldSequences만 배포할 때에는 현재 runtime map/deploy 대상과 참조를 검증해 같은
불일치를 승격 전에 거절한다. Area 배포는 이번에 stage한 맵/배치 정본으로 검증한다.

실제 Client session 로그에서 Gameplay.bootstrap4122행을 Client4096상한이 거절한
별도 문제도 확인했다. Server/publisher8192와 Client admission, 기존 Python 후보검사의
상한을 맞춘다. 의미가 다른 animation/curve 개수의4096상한은 변경하지 않는다.

재생 시작 실패의 진단은 두 곳에서 보존한다. Workbench가 Pattern/Bundle 요청을 소비한
직후의 결과는 같은 실패 문구여도 한 번 다시 표시하고, 그 후 매프레임 동일 상태가 편집
메시지를 지우지는 않는다. Bundle WORLD 준비가 실패하면 이전 Set_Document 성공
문구가 아니라 Prepare_InstanceResources의 실제 오류를 전달한다.

기존 preview transport/admission 계약 검사를 필요한 항목만 실행하고, Area publish/check,
WorldSequence 참조와 최소 Client compile을 확인한다. 신규 제품 파일은 없으며 사용자가
Client를 종료하기 전에는 최종 EXE 링크를 하지 않는다. 실제 재생/화면 결과는 사용자가
확인한다.

## G22. 실행용 맵의 Git 전달 계약 복구

사용자가 실행용 맵의 Git 제외 규칙 수정과 G21 수정본의 PR·병합을 요청했다.
`.gitignore`의 Map `*.mapassets`·`*.mapplacements` 제외 두 줄을 제거하고 기존
`.gitattributes`의 LFS 규칙으로 두 확장자를 관리한다. 현재 기존 runtime 36개는 이미
추적 중이며, 이번에 추가할 파일은 publisher가 생성한 쿠크 catalog·placement 두 개다.
Data 원본을 편집하고 publisher로 실행용 snapshot을 만든다는 소유권은 유지한다.
새 배치와 이를 참조하는 시퀀스 및 catalog는 같은 PR에서 전달한다.

AGENTS의 Git 전달 계약, CLAUDE의 실행 준비 설명, Area 가이드와 Map pipeline 사용서를
현재 정책으로 맞춘다. Resources, Navigation과 다른 domain의 제외 규칙은 변경하지 않는다.
G21의 실패 참조 거절·쿠크 배포 owner·재생 오류 표시·bootstrap 허용 범위도 함께 검토한다.

쿠크 Area Check와 기존 관련 회귀 검사를 확인하고, 두 파일의 LFS 저장 및 Git에서
복원한 실행 데이터의 참조 일치를 검증한다. Git 정책·문서 변경은 추가 C++ 컴파일을
요구하지 않으며, G21의 실제 Debug compile/link 증거를 보존한다. 사용자 화면 확인은
자동 검사와 분리한다. 커밋된 변경을 독립 검토한 뒤 PR을 생성하고 main에 병합한다.
