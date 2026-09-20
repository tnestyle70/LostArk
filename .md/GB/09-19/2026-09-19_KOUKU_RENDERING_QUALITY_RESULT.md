# 쿠크 렌더링 품질과 Pattern 삭제 결과

작성일: 2026-09-19. 계획은 `2026-09-19_KOUKU_RENDERING_QUALITY_IMPLEMENTATION_PLAN.md`이다. 사용자 저장·Client/Server 종료 확인 후 최신 디스크 기준으로 적용했다. source 조사, 코드·Resources 설치, 데이터 publish, 자동 검증은 완료했고 최종 화면 판정은 사용자 확인으로 남긴다. 에이전트가 Client/UI를 실행하거나 조작하지 않았다.

첨부 화면의 밝기와 평평한 인상은 한 specular 배율로 설명되지 않는다. 원본 LUT 누락, 실제 활성 alias의 source 후처리 미연결, 원본 directional exclusion 미반영, character 간접광 값을 map ambient로 올린 adapter, 일부 바닥 BRDF 차이와 보스 모델의 flat vertex normals가 함께 확인됐다. 발탄의 LUT를 쿠크에 복사하지 않고 쿠크 source package의 서로 다른 지역 입력을 복구했다.

## G01. 쿠크 바닥의 약한 specular: 원본 lobe 복원

SL05의 두 floor1/2 MIC에 원본 power 30/100과 intensity 0.2가 이미 들어 있었지만, deferred marker 1은 원본 Blinn 대신 legacy Phong으로 반사를 계산했다. 09-15의 수정은 원본 재질 반사에 들어오는 광원 RGB를 legacy Specular 채널에서 Diffuse 채널로 연결한 것이며, 반사각 계산은 당시 남아 있었다. 따라서 이번 문제를 전역 specular 배율 부족으로 처리하지 않았다.

| 실제 배치·재질 | 현재 material 계약 | 설치 원본 DXBC 근거 |
|---|---|---|
| SL05 export380 / placement 12451899878577673092 / MAP_61FA80B72343_BG_RAD_KOUKUSATON_FLOOR08_SM_OVR_04DCDAA75173 | floor09b_mi → bg_seamless-specular_msk → family1; S.rgb × intensity0.2; power30, normal0.5, diffuse brightness0.6 | LocalVF directional PS 526df319c686eb43b1c7d463fc688eda |
| SL05 export342 / placement 10472891450652540488 / MAP_0BE6EBEE4924_BG_RAD_KOUKUSATON_FLOOR08A_SM_OVR_84D65232B541 | floor08b_mi → bg_base_msk → family2; 원본 D.rgb × intensity0.2; power100, normal0.5, diffuse brightness0.7 | LocalVF directional PS b8160ab1e4eed948b85f2389b3e8523b |

기존 추출 임시 폴더는 없었으므로 설치된 게임 package와 ReleasePC/EV2LG3OVEH3HGV7THTFFTM7TOKMCC.upk shader cache에서 두 MIC의 실효 static set을 다시 읽었다. base material ID와 engine equality static parameter SHA가 일치하는 context를 각각 하나씩 골라 위 pixel shader를 얻었다. 원본은 normalize(L)+normalize(V)의 half-vector를 사용하고 abs(N·H)가 1e-6 미만이면 0, 나머지는 min(pow(abs(N·H), power), 1)을 쓴다. 09b는 shadow 뒤 RGB cap2, 08b는 cap이 없지만 현재 두 MIC의 흰 specular color × intensity0.2와 포화 texture/lobe/shadow 입력에서는 cap2에 도달하지 않는다. 이번 최소 수정은 공통 lobe만 복원한다.

Engine/Bin/ShaderFiles/Shader_Deferred.hlsl:427의 Evaluate_RecoveredFloorSpecularLobe를 directional:655와 local:768의 marker1에만 연결했다. diffuse, ambient, shadow, attenuation, material 값과 다른 native family는 보존했다. 빛이 정면이고 카메라가 20도 기울어진 예에서 power30 lobe는 0.15473→0.63175, power100은 0.001989→0.216345다. 이는 특정 기하 조건의 반사 함수 비교이며 실제 화면 밝기의 배율이 아니다.

생산·소비 경계는 Engine/Public/BinaryAsset/ModelAssetData.h:34의 family1/2 → Client/Private/MapAssetRenderUtils.cpp:1086의 g_SurfaceProgram → Shader_VtxMeshBinary.hlsl:318의 marker1 및 RT5 material RGB → deferred다. source character6은 앞선 별도 geometry 경로를 쓴다. BG8 unlit도 marker1을 쓰지만 specular RGB를 0으로 초기화한 뒤 반환하므로 반사 기여가 없다. marker0/2의 Phong과 source BG8/Valtan 등 기존 Blinn 경로는 바뀌지 않았다. 원본 normal.A는 reflection mask이며 roughness로 확대 적용하지 않았다.

SL04에 배치된 FLOOR09a/08a RNM 변형은 family BG8로 이미 다른 source Blinn 경로다. 따라서 이번 floor1/2 결함을 모든 관문 또는 첨부 화면 전체의 단독 원인으로 확대하지 않는다. 화면이 밝고 평평한 diffuse·조명·프로필 비교는 본 문서의 rendering profile 조사와 함께 판단해야 한다.

검증은 existing PointLightFalloffContractHarness를 확장했다. D3D11 WARP의 실제 deferred Directional/Point/Spot, single/batch, power30/100, camera0/20/40도, marker0/1/2, legacy Specular on/off 216 cases에서 RGB channel failure 0이다. 기존 receiver6도 통과했다. 동일 fixture에 수정 전 shader를 넣으면 예상대로 marker1 off-axis 사례에서 144 channel failures/exit1이 나와 회귀를 검출했다. fxc fx_5_0 /O1 scratch compile도 성공했다. 실제 Client 화면은 실행·조작하지 않았다.


## G02. 실제 source와 발탄·쿠크 profile 소비 비교

### 원본 LUT가 비활성이라는 이전 판정 정정

PS package `C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC/Packages/534P5WP4TCKLJK6DPE4PSL4PXI.upk`를 직접 다시 읽었다. SHA-256은 `f0e0ef3af6265839031fdb96517614936eea62eab629aeb6e02374b8c7573163`이다. BoolProperty를 확인한 결과 export46 `EFEnvironmentBlendVolume_0`와 export47 `_1`의 `boverride_scene_colorgradinglut`는 모두 true였다. 각각 원본 LUT02와 LUT01을 참조한다. WorldInfo export705는 override flag가 true지만 texture reference 자체가 없어 world neutral은 맞고, 두 volume까지 neutral로 만드는 것은 틀렸다.

| 원본 export / 실제 runtime region | 원본 texture | source toneScale / range / toe | source desaturation |
|---|---|---|---|
| world705 / 입장점에는 일치 region 없음 | reference 없음, neutral | .85 / 8 / 1 | .15 |
| 46 / kouku.ps.environment.47 / G1 | lv_lut_midnightc_lut_02 | .8 / 8 / .6 | .1 |
| G2 / kouku.ps.environment.54 | neutral | .8 / 8 / 1 | .1 |
| 47 / kouku.ps.environment.48 / G3 | lv_lut_midnightc_lut_01 | 1 / 8 / .8 | .125 |
| 카드미로 / kouku.ps.environment.55 | 현재 neutral 유지 | .9 / 8 / 1 | 0 |

이 누락은 현재 renderer의 LUT 지원 부족이 아니다. 과거 `out/FullMapRestoration20260915/apply_source_profiles.py:31`의 `source_config`가 LUT reference에 `heartrb_lut` 문자열이 있을 때만 발탄 DDS를 선택하고, 다른 활성 LUT는 빈 문자열로 바꿨다. 현재 `Tools/RenderingPipeline/Publish-RenderingProfiles.ps1`과 `CRenderingProfileService`는 임의의 유효한 Resources-relative linear LUT를 지원한다. 과거 out 일회성 importer의 whitelist를 현재 제품 소스 오류로 오인하지 않는다. 이번 추출은 exact source object 두 개를 명시적으로 resolve하고 이름·format·override flag·package hash를 검사했다. 09-18 RESULT G09의 “원본 LUT override가 비활성” 설명은 이 직접 검증으로 정정된다.

Texture package `201M2TM1Q9HIGH3AMBESDM9.upk`의 SHA는 `72effea497ec4e7f89993634a2fae36cef19ca2d93591de8f711c4c7b20be4ed`이다. exports317/318은 256×16 PF_A8R8G8B8, sRGB=false, 단일 mip다. native mip BGRA 16,384 bytes를 그대로 DDS로 저장했다. 독립 UModel TGA export와 RGBA를 대조해 각 4,096 pixels / 16,384 bytes 불일치0, alpha255를 확인했다.

| 설치 asset | DDS SHA-256 |
|---|---|
| Map/Lighting/Kouku/lv_lut_midnightc_lut_01.dds | 572a4f5d43d3387a2963b01bbbfbb2c1821e7ce09fc2cc668f67e3248f5a19a7 |
| Map/Lighting/Kouku/lv_lut_midnightc_lut_02.dds | 2fae2ecf4871c05caf14128c62a9860295fc7be0db4bea4284438df6332d8f33 |

LUT01은 중저역을 차갑고 어둡게 바꾸지만 LUT02는 일부 중간톤을 밝힌다. 예를 들어 neutral .533의 LUT02 grid sample은 약(.761,.741,.678)이다. 따라서 “LUT를 추가하면 무조건 어두워진다”는 결론은 근거가 없다. 입사광·간접광·tone과 같이 판단해야 한다. 근거는 `source-lut-recovery.receipt.json`, `lut-independent-validation.json`이다.

### Directional light는 있고, 원본에는 제외 volume도 있다

원본 export41 `DominantDirectionalLight0.excludevolumes`는 exports39/40을 참조한다. 실제 BrushComponent convex vertex와 face plane을 파싱해 cm→m와 기존 축 변환을 적용했다. 모든 정점의 plane 허용 오차는 최대 약0.00000977 m였다. 첫 volume bounds는 x[-193.8,327.67996], y[-163.84,245.76002], z[-559.59996,1244.44020]이다. 입장/G1/G2/G3 플레이어와 follow-camera 점은 모두 실제 convex 내부이고 카드미로는 외부다. 두 번째 volume은 Mario 쪽이며 source receipt에 함께 보존했다.

| 검사점 | Player XYZ | Follow camera XYZ | DDL 제외 내부 |
|---|---|---|---|
| 실제 입장 spawn | 3.29,8.64,-10.69 | -6.21,21.97503,-1.19 | 예 |
| G1 | -2.45,1.32,740.37 | -11.95,14.65503,749.87 | 예 |
| G2 | 3.38,10.56,323.92 | -6.12,23.89503,333.42 | 예 |
| G3 | -2.45,1.32,945.17 | -11.95,14.65503,954.67 | 예 |
| 카드미로 | .09,-.01,1351.48 | -9.41,13.32503,1360.98 | 아니오 |

기존 runtime은 이 원본 제외 조건을 소비하지 않아 기본 directional diffuse(.8,.8,.8), specular(.5,.5,.5)가 해당 무대에도 적용됐다. 이번에는 기존 scene/camera-region light의 diffuse/specular를 0으로 만드는 구역 단위 대응을 적용했다. **원본 receiver별 convex light exclusion을 shader에 구현한 것은 아니다.** G2 source environment bounds 일부는 light exclusion 밖으로 뻗으므로 region 전체와 원본 receiver 조건을 동일하다고 할 수 없다. 경계 밖 물체·다른 공간을 동시에 보는 자유 카메라까지 정확히 복구했다는 뜻은 아니다. Point/Spot 배치와 사용자가 만든 추가 map/anchor lights는 변경하지 않았다. 카드미로 directional/specular는 기존 값을 보존했다. 근거는 `source-ddl-exclusion.receipt.json`이다.

### Flat map ambient와 발탄이 달랐던 이유

WorldInfo Lightmass의 원본 EnvironmentColor=(104,115,120), EnvironmentIntensity=.5는 현재 adapter에서 RGB(.20392157,.22549020,.23529412)가 된다. 원본 G1 volume의 `WLE_CharacterLitIndirectBrightness=2.5`와 shadowed 값은 character lighting 입력인데, 이전 map importer는 이를 전체 uniform map ambient에 곱해(.50980395,.56372547,.58823532)로 만들었다. 원본 SH/contrast/shadowed SH를 복구한 것과 다르며 unbaked floor를 평평하게 밝히는 원인이 될 수 있다. G1/G3 등 수정 대상의 map ambient는 ×2.5/×2를 제거한 world baseline으로 되돌렸다. 명시적인 기존 g3.dark ambient0과 다른 사용자 authored lights는 보존했다. 이 baseline도 프로젝트 uniform adapter이며 원본 character SH 복원 완료로 기록하지 않는다.

발탄은 LevelRegistry에서 `scene.valtan.cool-low-key.v1`을 실제 활성화하며, 그 profile에 원본 source tone+`lv_lut_heartrb_lut.dds`가 직접 연결돼 있다. 현 저장값은 exposure.73, source toneScale1/range8/toe1/desaturation.3, bloom.5, SSAO off, directional(.8647,.83294,.77647), ambient(.16,.21,.28), specular(.12,.15,.2)이다. 원본 입력 연결과 별도 조정값의 결합이며 “발탄 LUT 하나의 비밀”이 아니다. 이번에 발탄을 포함한 무관17 profile은 직전 저장본과 JSON 완전동일을 확인했다.

쿠크의 실제 `Activate_Profile`은 `Get_ActiveLevelQuality()`를 거쳐 active Level base quality를 상속한다. alias에 qualityOverride가 없다는 이유로 globalQuality를 직접 소비한다고 판단하면 틀린다. 조사 중의 global gamma1.905/bloom2.609 상속 추정은 이 실제 호출 추적으로 기각했다. 최신 저장 base quality exposure2에 base multiplier.5를 곱하면1이지만, G1/G3 alias multiplier1을 곱하면2다. 기존 alias에는 region/sourcePostProcess도 없어 Hable로 돌아갔다. 이 상속·multiplier·source 연결을 함께 보아야 한다.

Source tone 경로는 Shader_Deferred.hlsl의 source branch에서 일찍 반환하며 Hable용 gamma/whitePoint/display desaturation을 쓰지 않는다. source formula의 일부 gamma는2.2로 고정되어 있고 exposure는 source tone 전 곱해진다. 저장 gamma 값만으로 source 화면 밝기를 설명하지 않는다.

## G03. Delete Pattern과 참조 정리

기존 삭제 확인은 Flow 행이나 Bundle membership, Parent Pattern Box도 모두 필수 참조로 취급하여 Delete from Draft를 막았다. Delete_Pattern 본체는 별도로 disk freshness를 확인하여 외부 저장 이후에는 draft 삭제 자체를 막았다. 현재 Save_Atomic이 이미 revision/hash 검증과 backup·atomic replace를 소유하므로 편집과 저장의 경계를 나눴다.

Client/Private/KoukuSaydonActionWorkbench.cpp:2735의 참조 수집은 삭제와 함께 정리할 참조와 필수 blocker를 구분한다. :2786의 삭제는 대상 Pattern, 해당 Flow PATTERN 행, Bundle member, Parent Pattern Box, Folder timeline 포인터를 하나의 candidate에서 정리한다. 영향받은 Parent와 Bundle은 기존 DRAFT 전이를 적용하고 PlayAll ID를 다시 계산한다. 전체 Validate/Commit에 성공해야 draft를 바꾸며 Save_Atomic은 그대로 유지한다. 공유 Animation/Effect/Logic/Summon 정의나 다른 Pattern timing/counter를 임의 삭제하지 않는다.

원본 필수 연결인 Logic directionPatternIds/patternIds/followupPatternId/clonePatternId, Summon directionPatternIds, 다른 Pattern의 Summon PatternSpawns는 owner를 표시하여 계속 차단한다. 안전하게 편집할 수 없는 malformed owner도 차단한다. 삭제 확인에는 함께 제거할 목록과 먼저 고칠 blocker 목록을 분리했다. 상단 Delete Selected Pattern은 publication 중이거나 개별 Pattern을 선택하지 않았을 때만 비활성화되며 tooltip에 그 이유를 표시한다. outer BeginDisabled 누수는 없었다. 개별 tree 선택은 PATTERN으로 전환되고 Parent folder도 실제 timeline Pattern으로 전환한다.

최신 read-only 참조 집계는 revision1770 / 93 Pattern / source SHA ece6183064f05d65bccd8f77724693968ddd6a89c7a80f4fd74d366ddf81b038 기준이다. serialized stable참조로 기존 전참조 정책은70개를 막고, 변경 후 필수 Logic/Summon blocker는22개다. soft참조만 있는48개는 동반 정리로 삭제할 수 있고, 무참조23개는 기존처럼 삭제할 수 있다. 이는 삭제 실행이 아닌 자료 집계이며 runtime malformed-owner quarantine은 추가로 차단할 수 있다. 상세 owner와 stable ID는 pattern-delete-reference-audit.json에 있다. 실제 authoring Pattern 삭제는 수행하지 않았다.

CPU focused --kouku-pattern-delete-contract는 원본 header/action reference를 별도 임시 Data root에 복사하고 PRODUCT child/Parent/Bundle에서 시작한다. soft 연결 정리, affected DRAFT 전이와 PlayAll 갱신, 무관한 Pattern·시간·카운터 보존, Save→Reload 동일성, Parent folder 포인터 정리, 필수 Followup 거절 시 draft/file 보존, 외부 저장 이후 draft 삭제와 stale Save 거절 시 외부 파일·pending draft 보존을 모두 통과했다. 기존 harness의 최신 Workbench link 의존 4개에는 live audition/camera 및 Effect lifetime 실행을 거부하는 test-only strict shims를 추가했다. 전체 editor suite나 Effect lifetime 성공으로 확대하지 않는다. UI 버튼 실제 조작은 사용자 확인으로 남긴다.

Publish_AllPatterns는 여전히 Save와 fresh baseline 이후 기존 domain publisher를 호출한다. publisher는 삭제된 Pattern을 source inventory에서 제거하고 현재 저장된 나머지 Pattern/Bundle을 다시 검증한다. authoringStatus는 private publication candidate에서 PRODUCT로 재평가되는 legacy 편집 field이므로 DRAFT 표시만으로 게시 차단을 보장한다고 설명하지 않는다. 비어 있는 Bundle이나 불완전 Flow의 게시 가능 여부는 기존 publisher의 개별 검증이 결정한다. 이번 검증에서 사용자 저작 데이터의 삭제나 게시를 자동 수행하지 않았다.


## G04. 검증 정합: 기존 Source Modulate 6-pass와 사전 컴파일 closure

Release Product는 성공했지만 최초 closure는 Artist448 particle에서 technique/pass count mismatch로 실패했다. Tools/RenderingPipeline/ProductEffectShaderWarpProbe.cpp가 모든 particle을5-pass로 가정한 것이 원인이다. Shader_EffectParticleFamilyCarrier.hlsli의 family7은 09-15부터 index5 MultiplyOneSidedDepthRead를 추가한6-pass이고, runtime Effect_DocumentRenderer_MaterialBinding.cpp:49도 해당 profile을5로 선택한다. native source sprite만 이 mode를 허용하는 문서 validator와도 일치한다.

probe는 ARTIST-family particle에만 정확한6개, index5 이름 MultiplyOneSidedDepthRead, SourceModulatePS를 요구하도록 정합시켰다. 실제 Pass::Apply 후 받은 blend state가 RT0 RGB의 Dst*Src, alpha 보존, 독립 RT1/RT2 write mask0인지도 검사한다. 나머지 개수·순서·carrier layout·필수 uniform·family 격리 검사는 유지했다. 제품 shader source나 CSO를 바꾸거나 검사를 삭제하지 않았다.

Debug와 Release의 Test-CompiledShaderClosure -Modules Product가 모두 PASS다. 각 configuration에서 executable shader family99, resource-root8 cases, V1/V2 lit pixels 각각1352, active FxCompile producer140, Client consumer120을 확인했다.

## 통합 빌드와 실행 범위

root의 Release Product receipt out/BuildPipeline/runs/20260919T084237404Z-release-product.json과 Debug Product receipt out/BuildPipeline/runs/20260919T084643387Z-debug-product.json은 모두 PASS다. focused harness는 Product를 병렬 빌드하지 않고 BuildProjectReferences=false로 컴파일했다. 변경 C++/HLSL과 probe의 git diff --check도 통과했다. 실행 중 Client/UI를 에이전트가 자율 실행·조작하거나 종료하지 않았다. Debug/Release 컴파일·수치 검증은 완료됐으며 첨부 화면 대비 최종 재질·조명 평가는 사용자의 실제 화면 확인으로 남긴다.

상세 근거는 out/KoukuRenderingQuality20260919/verification.receipt.json, floor-native-maps.json, 두 native DXBC/json, floor-warp.log, floor-warp-before.log, pattern-delete-test-product.log, pattern-delete-reference-audit.json, shader-closure-{debug,release}-after.log에 있다. out 산출물은 commit 대상이 아니다.

재사용 원리: 원본 exponent를 가져왔으면 사용한 Phong/Blinn BRDF도 함께 맞춘다. GBuffer marker와 payload가 다르면 source helper를 통째로 재사용하지 않는다. 표시·정렬용 참조와 필수 gameplay target을 구분하여 삭제 candidate를 만들고, 디스크 CAS는 저장 경계에 유지한다. source carrier에 pass가 추가되면 실행 table뿐 아니라 closure의 ordered pass·PS·blend 계약도 함께 갱신한다.

## G05. 원본 지역 연결, 저장값 보존과 Workbench 비교

### 설치한 실제 profile과 비교용 profile

`Data/Rendering/Authored/RenderingProfiles.json`의 base technical quality·사용자 multiplier를 유지하고, 빠졌던 source world postprocess만 추가했다. G1 book-open/popup alias는 G1의 실제 camera volume47과 LUT02, G3 dark alias는 volume48과 LUT01을 기존 `environmentRegions`로 소비한다. Popup의 이름이나 ambient×2를 보고 G3 volume을 선택하지 않았다. base·source-rendering의 G1/G3 source LUT도 복구했다. 선택 영역의 directional exclusion과 잘못된 character×map ambient multiplier를 함께 정정했다.

실제 alias에 전체 quality snapshot을 새로 저장하지 않아 현재 Level 품질 상속을 유지한다. g3.dark의 명시적인 ambient0도 유지했다. 별도 `scene.kakulsaydon.compare.{start,gate1,gate2,gate3,card-maze}.v1` 5개는 현재 카메라에 고정된 look을 비교하는 용도다. source comparison의 exposure1 등은 source tone을 평가할 프로젝트 normalization이며 원작 카메라의 전체 exposure·모든 technical quality가 완전히 복원됐다는 뜻은 아니다.

| 구역 | 저장 전 실제 effective exposure / bloom | 설치 후 실제 effective exposure / bloom | source 비교 exposure / bloom | 설치한 tone/LUT |
|---|---|---|---|---|
| 시작 | 1 / 0 | 1 / 0 | 1 / .9 | world .85/8/1, neutral |
| G1 | 2 / .8 | 2 / .9 | 1 / .9 | .8/8/.6, LUT02 |
| G2 | 1 / 0 | 1 / 0 | 1 / .9 | .8/8/1, neutral |
| G3 | 2 / .8 | 2 / .9 | 1 / .9 | 1/8/.8, LUT01 |
| 카드미로 | 1 / 0 | 1 / 0 | 1 / 0 | 기존 .9/8/1, neutral 유지 |

위 표의 값은 source/Level/profile/region 계산 후, 동일한 사용자 video overlay 전이다. alias에 source region을 추가하면서 source region의 bloom .9도 연결됐다. base 사용자 exposure2, multiplier.5와 bloom multiplier0은 저장 그대로다. 노출2인 실제 G1/G3가 source 비교보다 여전히 밝을 수 있으므로 사용자가 비교 기준을 보고 조명을 재조정할 수 있게 남겼다. 이 사용자 저장값을 source1로 강제 덮어쓰지 않았다.

카드미로의 전체 settled effective quality, light, fog, shadow, mapLightIntensityMultiplier는 설치 전=후=고정 comparison으로 같음을 검증했다. 대표값은 exposure1/gamma2.2/bloom0, diffuse(.64,.6462745,.8), ambient(.20392157,.22549020,.23529412), specular(.5,.5,.5), source tone(.9,8,1), source midtones(1,1.1,1), LUT neutral이다. 이는 profile 입력 동일성 검증이며 사용자 화면의 pixel equality를 실행한 것은 아니다.

### 기존 소비 경로의 최소 확장

`Client/Public/RenderingProfileService.h`의 `SCENE_ENVIRONMENT_REGION`에 optional `qualityOverride`와 `specularColor`를 추가했다. source profile의 다른 기준을 비교하더라도 사용자 승인 카드미로 품질과 specular를 유지하는 용도다. C++ parse/validate/serialize와 Rendering publisher가 같은 계약을 소비한다. full region quality → active scene multiplier 및 user video → region postProcess 순서로 계산하며, 이탈 시 `m_EffectiveQuality`로 복귀한다. exposure/gamma/whitePoint와 기존 source postprocess는 region 시간으로 보간하고 기술 on/off는 선택된 영역 값을 적용한다. specular는 diffuse/ambient와 함께 보간한다. 단계 실패 시 이전 quality/fog를 복구하고 region transition 상태를 commit하지 않는다.

`Client/Private/RenderingBenchmark.cpp`의 Rendering restoration에 `Kouku area profile` 선택을 연결했다. source fixed comparison profile은 environmentRegions가 없어 카메라가 그 profile을 다시 덮지 않는다. MainApp의 gate profile 변경은 요청 ID의 edge만 소비하므로 매 프레임 비교를 취소하지 않는다. 실제 gate/sequence/Level 소유자가 바뀌면 기존 비교 ownership을 해제하고 새 scene을 보존한다. Return to entry/Workbench 닫기는 여전히 도구가 소유한 preview만 복귀한다. 비교는 플레이어·카메라·Server gate를 이동시키지 않고 map light 배치를 교체하지 않는다. 현재 applied environment ID, camera XYZ와 directional specular RGB를 같은 패널에 표시한다. 프로필 선택으로 local lights까지 원본 위치·밝기로 자동 복원되는 기능이라고 설명하지 않는다.

### 검증과 데이터 반영 증거

- RenderingBenchmark.cpp/RenderingProfileService.cpp 최신 `/Zs` PASS. 통합 Debug/Release Product PASS는 G04에 기록했다.
- 실제 product parser/quality resolver/LUT decoder/serializer 함수 본문을 그대로 추출한 native headless probe에서 catalog roundtrip PASS. private 접근용 header, candidate Resources root, neutral user-video overlay만 probe boundary로 사용했다. Renderer/UI를 실행한 테스트가 아니다.
- regional exposure0, specularRGB65, region quality의 Resources 탈출 LUT(`../bad.dds`)를 각각 거절하고 이전 catalog serializer 출력이 변하지 않음을 확인했다.
- candidate publisher Validate PASS. 같은 invalid3개에 Publish가 실패하고 기존 sentinel runtime bytes를 유지했다.
- 설치 직전 source SHA `fdfbeea357b749cbd0fb85e497bc4931c7fe3c0ad252d85307d0d669ee987ef1`를 재확인하고 stable profile/region ID별57 field patch와5 profile 추가만 병합했다. native serializer 형식을 사용했다.
- LUT2개와 authoring을 backup·최종 hash 확인·원자교체했다. 실패 시 이미 반영한 자기 변경만 rollback하는 경계를 유지했다. authoring revision64→65, 설치 SHA `9cea24850b1b368b5ba2ffecac0b9ad3a05c2bc994f8902160f23954a041c1f7`.
- 정상 `Tools/RenderingPipeline/Publish-RenderingProfiles.ps1 -Mode Publish` PASS. authoring/runtime JSON 완전동일, 무관17 profile 완전동일, base 저장 품질(추가 sourcePostProcess 제외)와 세 multiplier 완전동일.

Backup은 `out/KoukuRenderingQuality20260919/rendering-backup/20260919T085114Z/`다. `rendering-install.receipt.json`, `rendering-field-patch.json`, `rendering-quality-comparison.json`, `profile_schema_probe.log`, `publisher-negative-validation.json`, `runtime-publish.log`에 검증·반영 근거가 있다. out은 임시 검증 산출물이며 제품 Resources에는 LUT DDS만 추가했다.

재발 방지 원리: active BoolProperty와 object reference를 함께 검증하고 특정 맵 이름 whitelist로 다른 source LUT를 neutral 처리하지 않는다. source profile이 존재하는지보다 실제 Level→alias→camera region 소비를 추적한다. character WLE와 world uniform ambient를 같은 필드로 간주하지 않는다. DDL exclusion의 point/convex 증거와 camera-region 근사 구현 범위를 구분한다. 정상 판정된 영역은 저장본의 전체 effective 입력을 별도 보존한 뒤 다른 영역을 복원한다.

## G06. 보스와 무기 source vertex basis

Root 조사에서 설치된 5개 쿠크 보스/무기의 유효 삼각형 normal이 face normal과 정렬돼 원본 smooth vertex basis가 손실된 사실을 확인했다. `MN_RPCZ_00`, `MN_RPCT_05`, `MN_RPCT_06`, `WP_MN_RPCT_05`, `WP_MN_RPCT_06`을 원본 UPK→독립 UModel glTF와 대조하여 normal/tangent/handedness를 복구했다. 메시 외 section, indices, 위치·UV·skin을 보존했고 원본 basis 최대 오차 약2.98e-8을 확인했다. 원본 basis를 새 format1.5로 구분하여 저장하고 legacy fallback으로 다시 flat normal을 만들지 않도록 reader/writer/consumer를 연결했다.

5개 후보 모두 native bind-pose reader에 받아들여졌고, tiny normal/과대 tangent·zero normal·parallel basis·zero handedness·nonfinite normal 같은 손상 입력은 거절했다. Root가 Resources의5 WModel을 backup·hash 확인 후 교체했다. `boss-geometry-audit.json`, 각 `<Model>-basis.json`, `G11-basis-independent-audit.json`, `G11-parser-negative-audit.json`, `boss-install.receipt.json`이 근거다. 이는 실제 모델 geometry 입력 복구이며 최종 화면의 specular·실루엣 판정은 사용자 확인을 필요로 한다.

5종은 원본 LOD0와 모든 삼각형 위치·UV가 대응하므로 최저 LOD 선택 오류가 아니었다. 307,543개 정점의 실제 native decode와 기존 형식 회귀 검사를 통과했다. 자세한 원본 추출 명령·삼각형 수·보존 byte·배포 hash는 [보스 원본 basis 복구 결과](2026-09-19_KOUKU_BOSS_SOURCE_BASIS_IMPLEMENTATION_RESULT.md)를 따른다. 원본 실루엣 자체의 다각형을 subdivision으로 바꾸지는 않았다.

## G07. 레이드 Effect 선준비 연결

구현·검증 정본은 기존 [쿠크 패턴 재생 복구 RESULT의 G10](../09-18/2026-09-18_KOUKU_PATTERN_RUNTIME_REPAIR_RESULT.md)이다. CSO는 이미 빌드 때 생성되고 있었다. 실제 누락은 첫 재생 때 준비하던 published Pattern/Sequence/World의 JSON·shader 객체·texture/model dependency였다. 확인한 게시본에서 V1 119개와 V2 45개 target, enabled World326개를 기존 Loader/V2/World 경로로 입장 전에 준비하도록 연결하고 필수 준비 실패는 입장 실패로 처리했다.

이번 Debug/Release Product에 해당 소스를 반영했으며 CSO 소비 검사는 G04처럼 두 설정 모두 PASS다. 발생별 mutable instance·particle/trail buffer와 draw 비용까지 제거한 것은 아니다. 실제 cold 첫 패턴의 프레임 시간은 사용자 실행 확인으로 남긴다.

## 남은 화면 확인과 복원 경계

사용자는 Rendering Workbench에서 같은 카메라로 시작/G1/G2/G3 source 비교와 현재 profile을 번갈아 보고, 별도 source comparison exposure1과 기존 actual exposure2의 차이를 포함해 local lights를 조정할 수 있다. 카드미로는 보존된 현재 기준을 먼저 확인한다. C++/JSON/CSO와 source texture·basis 수치 검증을 실제 화면 승인으로 대신 기록하지 않는다.

원본 receiver별 DDL convex exclusion, character WLE SH·shadowed SH·contrast, DOF·빛줄기·모든 환경 이펙트의 완전 재현은 이번 완료 범위가 아니다. 기존 user-authored maplights/light resources나 사용자 Pattern을 임의로 재저작하지 않았다. source LUT 복원만으로 과노출이나 모든 재질 차이가 해결됐다고 단정하지 않는다.
