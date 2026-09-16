# 워로드 A·S·V·F 원본 전체 복원 구현 계획

## G00. 원본 발생과 현재 저작본 분리

현재 브랜치는 `codex/dimensionmaster-tool-round3`, 기준 HEAD는 `591012dbebf7eeab0b660baec42852b9396e77d4`다. 사용자 손튜닝과 다른 작업의 dirty 변경을 보존한다. A17090 사슬, S17040 파란 방패 세 개의 정면 발사, V17170과 F17140 번개가 목표다. S를 D로 바꾸지 않는다.

최신 설치 data3.lpk의 GUNLANCER.loa에서 exact action/stage/clip을 먼저 선택한다. 같은 clip의 여러 stage는 하나의 동시 연출로 합치지 않는다. Particle notify의 enabled 값과 현재 package의 첫 LOD를 대조한 뒤 해당 발생만 복구한다. 보존 Imported JSON은 geometry와 module의 참고 입력이며 원본 활성 근거를 대신하지 않는다.

현재 A14/S11/V9/F4 element와 원본 활성의 수는 다르다. full restore는 별도 `effect.warlord.skill.<id>[.clipN].full.restore` 문서로 저장하고 현재 unified 문서를 수정하지 않는다. 원본을 따라가면서 끝내 실행할 수 없는 요소는 사유와 원본 ID를 RESULT에 남기고 새 full 문서에서 제외한다. 핵심 사슬·방패·번개의 복원이 우선이며 generic shader 교환이나 approximate 허용만으로 복원 완료를 주장하지 않는다.

## G01. 원본 재질·입력 계약 회수

source MIC → parent hierarchy → static parameter set → native shader map → 실제 carrier VF/PS/VS → texture/sampler/uniform을 회수한다. 원본 PS/VS가 같을 때만 차원술사 native 경로를 재사용한다. A는 Masked Chain의 WPO, S는 Shield02의 본체 두 층과 발생 세 회, F는 electric vertexcolor mesh의 sinwave WPO, V는 rectan3cross 전기 geometry와 재질을 먼저 닫는다.

새 `Effect_WarlordNativeMaterial.h`는 source identity와 명시 texture·parameter 계약을 검증한다. 새 `Shader_EffectWarlordNative.hlsli`는 회수한 원본 RT0 식과 필요한 정점 변형을 소유한다. 프로그램 번호 400~459와660~719를 사용하며 기존 native profile과 충돌하지 않는지 확인한다. 새 C++ 번역 단위 Effect_NativeScreenPostMaterial.cpp는 기존 Engine IPresentationScreenPostMaterial 소비자에 native snapshot을 연결한다. header/cpp와 shader의 vcxproj/filter 등록은 root 통합 변경에 포함한다.

## G02. 기존 Renderer와 재생 경로 연결

기존 EffectDocumentRenderer/Playback/Tool, CModel/CMaterial을 사용한다. 새 runtime을 만들지 않는다. 공용 파일과 project/filter 등록은 통합 담당 root가 수정하며, 워로드 담당은 exact include·dispatch·binding patch를 별도로 전달한다. profile의 parameter/texture 실패는 해당 occurrence를 거절하고 기존 문서를 보존한다.

source cm·좌표계·Dynamic/Orbit의 원본 기본값을 유지한다. 외부 emitter state를 필요로 하는 경우 owner 동시 재생이 필요한지를 판별하고, 원본과 동등하지 않은 authored path로 바꾸면 원본 복원과 구분한다.

원본 screen-post도 기존 PREPARED_MATERIAL callback을 통해 실제 scene color/depth, SRV9개와 native parameter32행을 연결한다. immutable frame snapshot이 source color/dynamic, 실제 scene camera projectionW, source local time을 보존한다. Effect_Object의 일반 native refraction도 scene capture를 요청한다.

## G03. 검증과 인계

변경 JSON parse·stable ID·실제 resource 존재, 선택 PS/VS input contract, 필요 FXC 및 focused CPU 검사를 수행한다. 최소 Product 컴파일과 project/filter XML parse·전체 diff check는 통합 root가 한 번 수행한다. Engine.lib를 여는 별도 compile/link는 하지 않는다.

Client/UI는 실행·조작·캡처하지 않는다. 화면의 번개 움직임·사슬 실루엣·방패 세 개·색·타이밍의 최종 판정은 사용자 몫이다. Resources 신규 설치가 필요하면 정확한 상대 asset ID와 물리 위치를 RESULT에 기록하며 Git에 binary를 넣지 않는다.


## G04. 2026-09-10 전체 슬롯과 방패 배치 확대

사용자가 승인한 F17140 full 문서와 해당 native 함수는 바이트 단위로 보존한다. LMB17000부터 Q/W/E/R/A/S/D/F/T/X/V/ALT_V의 현재 skillbindings와 source receipt를 대조해 나머지 full 문서를 생성한다. 원본 활성 occurrence는 현재 raw action의 enabled, 실제 첫 LOD와 stage를 다시 확인한다. 설치 원본의 shader cache layout은 09-09 저장 layout을 재사용하지 않고 현재 package에서 읽는다.

V의 원본 PlaySkeletalMesh notify는 mesh/clip이 None이므로 이 빈 notify를 모델 복원 근거로 쓰지 않는다. 실제 FX_SM의 방패 geometry와 원본 재질을 조사한 뒤 사용자가 요청한 5방향 배치를 별도 저작 요소로 연결한다. ALT_V는 원본 SuperGProtection의 sk_wgl_gdd_01과 fx_w_wgl_gdd_02를 조사하며 외곽 큰 원6개와 캐릭터 주변6개 배치를 명시적으로 구성한다. 요청한 배치 수와 반지름 보강은 project-authored이고 원본 occurrence와 구분한다. 정상 source shield와 번개를 유지하고 실제 미지원 Solo 의존성은 full에서 제외해 정확 ID와 사유를 기록한다.

기존 build_warlord_asvf_full_restore.py를 전체 생성 옵션으로 확장한다. 공용 Runtime/Tool/Catalog/ResourceTree/project 등록과 통합 컴파일은 root가 수행한다. 워로드 전용 shader/header는 기존 native 함수의 변경 없이 새 프로그램1000~1199/2000~2199를 연결한다. 생성 JSON의 stable ID, source anchor, 모든 Resources 존재, 실제 codec Load/Save/Solo를 검증하고 사용자 실행은 대기한다.

## G05. 사용자 재검토의 V·Alt V 방패와 번개 교정

사용자가 현재 EXE에서 V/Alt V 방패 미표시와 V의 F 낙뢰 미표시를 보고했다. 기존 full 저작본과 사용자 위치·회전·수명을 보존하고 실제 재질 입력을 고친다. `Shader_EffectWarlordNativeGroup1088.hlsli`의1122/1123 원본 local-vertex material은 `meshemitterdynamicparameter` uniform을 소비한다. 이를 Dynamic 모듈 없는 particle stream0으로 바꾸면 원본 dissolve 식이 모든 fragment를 버린다. 저장 material parameter의 해당 lane을 base/light 양쪽에서 소비하도록 교정하고 기존 PS probe로 실제 alpha와 finite 출력을 확인한다.

V에 이미 추가된 F17140 native446의4개 wave는 원본 F와 같은 geometry/material을 참조하지만, 이전 저작 patch가 F의 HDR startColor와 ColorScaleOverLife를 낮은 상수로 바꿨다. 이 두 분포가 이전 patch 값인 경우에만 승인된 F 원본 값으로 복구하며 현재 radius·시각과 사용자 다른 편집은 보존한다. F 원문은 수정하지 않는다. Alt V는 실제 lightning texture를 쓰는 native1166의6개 occurrence를 독립 seed로 하나씩 복제해12개로 늘린다. 기존 발생 시각과 shape를 유지하며 다른 에너지·방패 요소를 일괄 복제하지 않는다.

기존 `patch_warlord_v_guardian_lightning.py`를 확장하고 전체 full generator는 다시 실행하지 않는다. JSON은 현재 bytes를 보존한 뒤 CAS 확인하고 저장한다. 기존 PS 수치 검사, 실제 codec 저장·재로드, 변경 shader의 Product 빌드와 diff check를 수행하며 Client/UI와 최종 시각 판정은 사용자에게 남긴다. 새 C++ 파일이나 project/filter 등록은 없다.

## G06. 워로드 Q source follow 크기 복구

Q full의 원본 native PS12개는 실제 Playback119행을 소비한357case에서 nonfinite0이며 붉은 HDR 출력이 있다. 제품 Warlord admission transform0.0001과 -90도 yaw를 적용한 실제 CModel의 손·worldzero·spine bone은 축0.01과 이미 m단위인 translation을 함께 가진다. 일반 source follow가 이0.01을 이미 m단위인 particle 크기에 다시 곱해 핵심 효과가 작아진다. 실제 Renderer/현재CSO 대조에서 축만 정규화하면 동일0.6667초의 maxRGB0.119533이81.4834로 증가했다.

`Effect_PresentationService.h/cpp`의 기존 엄격한0.01 검증과 translation 보존 함수를 재사용한다. 새 선택 helper는 `effect.warlord.skill.17030.full.restore`만 허용하며 다른 class·slot·기존 Artist31470 경로는 확장하지 않는다. `Effect_Tool.cpp`의 현재 pose와 과거 pose source anchor, Product source anchor가 같은 선택과 matrix 함수를 소비한다. Q JSON·shader 수식·원본 색과 발생 수는 보존한다. 새 파일이나 project/filter 등록은 없다. 변경 번역 단위 최소 컴파일과 actual CModel/Playback/Renderer GPU 숫자 대조, diff check를 수행하고 Client/UI와 화면 판정은 사용자에게 남긴다.

## G07. 2026-09-11 Alt V에 F의 황금 낙뢰 직접 연결

현재 Alt V17250 clip1/clip2는218/180행이고 F17140의 native446 mesh 낙뢰는0개다.
이전 패치가 F 낙뢰를 추가한 문서는 V17170뿐이다. Alt V에는 별도 native1166 sprite의
b_effectroot FOLLOW6개를 복제했으므로 사용자 요청한 F의 재생 경로가 연결되지 않았다.
실제 F의 `authored.source-particle.full-warlord-f.7ba81a7899d33b48c48c`는 root snapshot,
source basis yaw-90, vertexcolor electric mesh와 native446 WPO/dynamic/dissolve를 사용한다.

기존 `Tools/EffectPipeline/patch_warlord_v_guardian_lightning.py`에 `--altv-f-gold-only`
선택을 추가한다. Alt V 실제 제품 clip 두 문서에 F 낙뢰를 그대로 복제하고 각 발생의
시각·독립 seed·반경 및 startcolor만 저작한다. F의 HDR peak5를 유지한 황금 RGB
[5,3.6,0.4]와 기존 grayscale ColorScaleOverLife를 사용한다. 낮은 SDR 색으로 원본
방출량을 덮지 않는다. 실제 재질·geometry·수명·버스트4개·Dynamic과 root snapshot은
그대로 사용한다. clip1은0.6473/0.8472/1.0472/1.247/1.447/1.647초마다 두 반경
200~220/360~400cm의12개 파동, clip2는0.172/0.526/0.876초에 같은 두 반경의6개
파동을 연결한다. 합계18개 발생, 명시 burst72개이며 기존 Alt V 요소398개를 유지한다.

원본 F와 기존 V 문서, camera sidecar와 skillbindings/animevents는 수정하지 않는다.
현재 Alt V 두 문서 ID의 제품 cue가 이미 연결되어 있으므로 별도 연결·publish나
새 runtime·C++·project/filter 등록은 필요 없다. 출력은 기존 단일 Data 정본이다.
CAS 저장으로 동시에 편집된 문서를 덮지 않고 재실행 시 기존 복제 행을 유지한다.
변경 JSON parse, 실제 리소스 존재, 보존·멱등성·기존 Effect 구조 검사와 가능한 기존
codec/playback 수치 검사를 수행한다. 제품 통합 빌드는 root가 소유한다. 최종 황금색,
개수와 크기 확인은 사용자가 Character Select → Warlord → Alt+V에서 수행한다.

## G08. 2026-09-15 Alt V 본 스케일과 카메라 배경 추적 복구

`Effect_PresentationService.cpp`의 `Requires_SourceBoneImportScaleNormalization`에 실제 제품 cue인17250 clip1/clip2 두 ID를 추가한다. 설치 Warlord WModel의 두 action clip과 b_effectroot·spine2를 40시점/본 조합으로 읽었을 때 admission0.0001과 root100이 남기는 축은0.009999996~0.010000002다. 기존 엄격한 basis 검증과 translation 보존 함수를 그대로 소비하고 source StartSize·사용자 ring 반경은 바꾸지 않는다. 새 C++·project/filter 등록은 없다.

두 Alt V 저작 문서의 camera_view10개는 Required의 bUseLocalSpace=true와 달리 detail.particle.localSpace=false다. 이10개만 true로 맞춰 이미 태어난 배경 particle도 이후 camera anchor를 따른다. 다른 occurrence와 기존 본·root attachment는 보존한다. JSON은 현재 bytes를 보관한 뒤 필드 단위 CAS로 변경한다. 실제 Catalog/Playback의 camera 이동 전후 수치와 본 행렬을 검증하고 최소 TU 컴파일과 diff check를 수행한다. 사용자의 실행 중 Client/Server와 미저장 저작 도구에는 접근하지 않는다.

낙하 방패는 원본 SD_00의 활성 mesh emitter16/5와 정지6+6 프로젝트 ring을 분리해 조사한다. 원본 package·module·mesh·native material과 실제 재생 경로가 닫히기 전 임의 높이·시간 또는 modelCue를 추가하지 않는다. 기존 사용자 반경과 수량 변경은 보존하고 원본 복구값과 구분한다.

## G09. Alt V 기존 6+6 방패의 순차 낙하 연결

원본 SD_00의 emitter16/5는 sk_wgl_gdd_01/PBR1123와 fx_w_wgl_gdd_02/PBR1122를0.31초 동안16m에서0으로 내린다. SD_01은 같은 pair를 다시 지면에 만든다. 이미 저장된 사용자6+6 ring과 정확히 같은 자산이므로 새12개를 겹치지 않는다. clip1의24 stable ID와 XY 평면 반경6/1.4m, 크기2/1, 재질·방향은 그대로 두고 원본 action의6시작 시각0.497280011/0.697200000/0.897199988/1.097000003/1.297000051/1.496999979초를 연결한다. 각 낙하 후2.4초 clip 끝까지 지면에 유지하며 clip2 ring은 그대로 이어받는다.

기존 LocationDirect 소비자에 원본 location lookup[0,1600,0,0,1600,0,0,0]을 사용하고, 현재 ring의 남은 lifetime/0.31로 lookup clock을 바꾼다. 이 조합은 원본 SD_00과 SD_01을 사용자의 persistent occurrence 하나로 결합한 저작이다. 원본의 빈 ScaleFactor를 현재 runtime에서0배율로 오인하지 않도록 이 저작 adapter만 명시적 identity를 넣는다. 원본 module 자체·공용 C++의 기본값을 바꾸지 않는다. sourceRecipe의 spawn/lifetime, detail의 timing/lifetime만 같은 span으로 맞추며 다른310개 world-space 선택은 유지한다.

실제 Codec/Playback의24 pair carrier를2016 frame sample로 검사해 조기 출현0, 원본 하강 곡선 최대 오차0.000001908m와 clip1 지면→clip2 첫 frame 행렬 오차0을 확인한 후보만CAS 적용한다. Client 실행·화면 검증은 사용자 소유다.


## G17. 09-15 Alt V 시작 배경 cylinder의 native 재질 연결

사용자는 Alt V 원본 배경 전체 복원을 요청했다. clip1 notify010의0.01초 활성 FX_PC_WGL_08.Par_W_WGL_superAction_01_01/emitter12는 기존 unified에 남고 최초 full부터 빠졌다. 실제 carrier는 fm_d_cylinder_019이며 사용자 삭제로 간주하지 않는다. legacy grouped-translucent와 미해결 cloud032 입력을 유지한 채 완료하지 않고, 현재 설치 MIC fx_m_mi_o_00.fx_mi.fx_o_me_superactionspace_01_02_tr의 parent/map/LocalVF PS·VS/native texture·uniform을 회수한다. 확인한 MIC texture overrides는 noise043/noise031/atypical006_1_xcl이며 셋 모두 기존 Warlord Resources에 있다.

정확한 원본 PS closure와 WPO 여부를 확인한 뒤 비어 있는 기존 Warlord673/group0에 한 프로그램을 등록한다. 기존 Warlord material table·group000 함수·group0 dispatch를 확장하고 새 carrier를 만들지 않는다. generator에서 동일 source MIC·PS/VS와 명명 parameter/texture 계약을 재현하도록 연결한다. occurrence의 원본 module/notify TRS·원본 수명·카메라 배경 데이터 복구는 해당 발생 담당과 결합하고 다른 사용자 요소를 보존한다. 새 C++ 파일이나 새 shader group은 제안하지 않으며 실제 family/range/CSO 선택·Catalog Stage·Playback과 source PS/WARP를 확인한다. 최소 컴파일은 out에서 수행하고 Client/UI 실행·사용자 최종 화면 판정은 대신하지 않는다.

## G18. 09-15 사용자 화면 확인 후 큰 방패·낙뢰·마지막 균열 방향 정리

사용자는 번개가 복구되고 방패가 순차 생성되는 것을 확인했으며, 큰 방패가 순서대로 내려오도록 보이는 범위와 낙뢰 위치를 조정하고 마지막 검은 균열을 원본의6방향으로 요청했다. 기존 clip1의 inner12 carrier는09-15 낙하 추가 전0.01초 동시 생성·정지로 되돌리고 outer12만 원본 시작·0.31초 하강을 유지한다. outer는 두 clip에서 반경6m를 원본 SD00의2.625m로 맞추고 사용자 크기2는 유지한다. 원본 반경과 사용자 확대값을 구분한다.

설치 shield 두 mesh의 bounds와 실제 Sequence 카메라를 계산했다. 반경6m는 거의179도와 카메라 뒤 꼭짓점을 요구하므로 FOV만 확대하지 않는다. 반경2.625m의 착지한 shield bounds는0.81~2.0초603개 원본 key에서 최대111.9998도다. clip1의 원본 eye/lookAt/up과 모든 key time을 보존하고450~700ms/2000~2150ms smoothstep으로 수평FOV114도를 연결한다. 마지막2150ms 이후 원본 close-up은 그대로 두며 카메라 source 문자열에 PROJECT_TUNED를 명시한다. 시야 위16m에서 진입하는 순간까지 전부 화면 안이라고 주장하지 않는다.

낙뢰1166의12개와 마지막 바닥1140의6개만 전용 runtimeAnchorSlotId를 사용한다. 실제 WModel b_effectroot의20샘플은 X반전·Y/Z교환 basis를 가진다. 기존 source scale 정규화 뒤 socketLocalTransform의[90,180,0]도 역행렬로 이18개를 지면 basis에 연결하고 본 위치와 기존 world-space 출생을 보존한다. SourceRecipe에서 금지한 owner_yaw 검사를 완화하지 않는다. 낙뢰12개는 큰 방패의6개 XZ 위치에 같은 순서로 연결하고 추가 Y=-0.3m만 사용자가 요청한 낮춤으로 기록한다.

마지막 native1140은 원본 notify024~029 raw payload690bytes가 FRotator yaw 필드464..467만 다르다. 기존 Albion decoder의 정확한 named-anchor 경계로 [0,-10922,-21845,32768,21845,10922]를 읽고 해당6개에만 회전을 연결한다. emitter 회전은 중심을6방향으로 배치하고 axis-locked quad의 별도 billboardRollDegrees=-yaw는 띠의 장축을 같은 방사 방향으로 향하게 한다. 원본 StartRotation90도는 보존하며 서로 다른 위치/quad 소비자가 읽는 것이므로 이중회전이 아니다. 발생 개수와 재질은 그대로다.

out의 실제 Codec/serialize/Stage/Playback, 설치 본20개와 실제 Make_ParticleSpriteWorld 본문 추출을 사용해 inner 정지/outer 하강, clip handoff, 낙뢰 XZ 및 양수 opacity, 균열6중심·법선·장축을 검증한다. 제품은 현재 bytes SHA가 동일할 때 해당 요소와 camera key만 CAS로 바꾸고 다른 저작 변경·실행 중 draft·프로세스에는 접근하지 않는다. 새 C++/shader/Resources/project 등록은 없다.
