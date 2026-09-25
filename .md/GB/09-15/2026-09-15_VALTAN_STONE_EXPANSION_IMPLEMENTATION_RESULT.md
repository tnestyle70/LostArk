# 발탄 정적 석재·중앙 소품 원본 입력 확장 결과

## G00. 실제 설치 범위

기존 09-08 석재7배치와 09-11 중앙 circle1을 보존하면서 원본 floor01의24배치·rock02의419배치,
합443배치에 실제 component MIC·색·UV1·RNM을 연결했다. 새436행을 더했으며 source MIC04는27,
MIC05는164, MIC02는252배치다. MIC02의 native static set에서 vertex paint가 꺼진 것을 확인하고
기존 overlay의 world-up 분기로 연결했다. 다른 MIC 값을 일괄 복사하지 않았다.

중앙 Deploy A/B와 난간은 원본 LOD0 material array를 읽고 새 native geometry와 실제 재질8행을
ActorCatalog에 연결했다. source527의 구름은 같은 모델을 쓰던525와 별도 asset으로 분리해,
원본 native static set이 기존 program59와 일치하는 실제 molding 투명 재질로 연결했다.

| 현재 데이터 | 확인 수치 |
|---|---:|
| Valtan catalog | 717 assets |
| Map material override | 446행: 석재443 + circle2slot + 구름1 |
| RNM placement | 444: 석재443 + circle1 |
| Native directional static shadow | 234행 |
| 전체 placement | 13,184, stable ID·source ID·TRS·visibility 보존 |
| 이번 placement asset 참조 변경 | 437: 새 석재436 + 구름527의1 |
| 이번 연결 Resources 집합 | 221파일,49,328,801 bytes: WModel77 + DDS144 |

Data 정본은 Valtan mapassets/mapmaterials/mapplacements/deployassets, MapCatalog의 Valtan count와
BossCatalog의 해당 modelMaterialOverrides8행이다. 기존 다른 ActorCatalog 값과 다른 에이전트의
Valtan 기본 particle·scale 변경은 보존했다. runtime Map 출력은 기존 Area publisher가 생성했다.

## G01. geometry와 재질 근거

A는25,819정점, B는34,306정점이며 각각 UV2/3개와 native BGRA를 보존했다. 두 모델의 slot0은
rock04 overlay, slot1은 원본 crack-floor BG다. crack는 emissive가 없고 normal·saturation·specular를
사용한다. 같은 base/static 계약의 기존 source BG와 대조해 `sourceFlags=133`을 명시했다.
root가 native material 연결 모델에서 이전 crack-emission overlay를 차단하는 소비자를 연결했다.
다른 static component의 RNM을 합성 Deploy에 복사하지 않았다.

난간20,440정점/61,884index/UV2개에는 native COLOR가 없다. 실제4slot은 detail-normal BG,
simple BG, 방향 overlay, detail-normal/subspecular BG이며 서로 다른 MIC·15 native texture를
각각 선택했다. 구름527은4정점/6index/UV2개와 source material
`lv_lut_heartrb.mat.lv_rad_redsanddst_deco03_mi_khk2`를 사용한다. 실제 engine-static SHA
`d27d8653f5fd28a7133054cd8db284feff42b25165a6e70f75161ceb6e6eb472`가 기존 program59와 일치한다.
CMapAssetObject의 translucent→BLEND→pass3와 source binder/forward PS 소비를 확인했다.

## G02. 원본 접선 부호 오류의 교정

초기 신규 cook에서 UE packed normal.W를 그대로 glTF tangent.W로 옮긴 오류를 발견했다.
UE→glTF(X,Z,Y)와 glTF→runtime(X,Y,-Z)는 각각 determinant -1이다. 준비 glTF W는 native sign의
음수여야 하며 최종 runtime W는 native sign과 같아야 한다. native UV 미분으로 얻은 종법선과
cross(N,T)를 독립 비교했다. rock3,252corner 및 floor33,045 nondegenerate corner가 이 기준과
일치했다. floor의 degenerate UV triangle6개는 이 부호 판정에서 제외했다.

이번 신규 stone/A/B75파일의152,609정점 W를 교정하고 재설치했다. 전후 position/N/T.xyz/UV0/
COLOR/UV1/UV2/index는 모두 그대로였다. 기존 정상 석재7파일은 native 비교에서 이미 정답이므로
변경하지 않았다. 난간과 구름은 교정 후에 cook했다. UModel이 native half 0을3.0517578125e-5로
내보낸 구름 UV0 세 정점도 native 값으로 교정했다. 다른 오차를 숨기기 위해 허용 오차를 넓히지 않았다.
이전 단계 receipt의 geometry SHA는 최종 `final-installed-resource-inventory.json`이 대체한다.

## G03. RNM·shadow·광원과 텍스처 경계

234 native shadow의 LightGuid는 원본 DominantDirectionalLight의
`4ba587b9fa985e4b91c324a665a0ef33`과 같다. 현재 source point light22개의 LightGuid/LightmapGuid와
443석재의 baked GUID 교집합은0이다. 기존7배치 중 원본 shadow가 있는6개에 누락된 shadow 입력만
추가했고 기존 재질 상수·색은 바꾸지 않았다. shadow exponent2 및 penumbraWidth.05는 기존
PROJECT_ADAPTER 계약이며 원작 CPU penumbra 설정 복원으로 표현하지 않는다.

이번 DDS144개 중37개 G8 static-shadow는 원본 mip7~11개를 모두 보존했다. 나머지107개 surface/RNM
DDS는 UModel이 내보낸 mip0만 설치돼 있다. 원본 compressed texture 하위 mip 전체를 복원했다고
기록하지 않는다. native SRGB 여부와 normal/RNM의 linear 데이터는 별도로 확인했다.

## G04. 실행한 검증

- 실제 source native index와 glTF corner 대응, cooker topology/channel 검사 및 최종 WModel parse 통과.
- 기존7 native tangent 비교와 신규75 W 교정 전후 다른 채널 보존 통과.
- 전체13,184 placement를 이전 snapshot과 대조해 asset 참조 외 stable ID·TRS·visibility 보존 확인.
- Valtan Area `Validate → Publish → Check` 통과.7출력,13,184배치이며 마지막 placement SHA는
  `0f1815b04645bf4adbe843f414d0a226f6689493698ebb5b35a2b0848c586547`이다.
- source527 row에 처음 추가한 unsupported castsShadow field를 publisher가 거절했다. 기존 native
  contract대로 제거한 뒤 위 검증을 통과했다. translucent queue는 shadow draw를 제출하지 않는다.

실측 자료는 `out/FullMapRestoration20260915/Valtan`의 stone/deploy/rail/cloud install receipt,
`native-tangent-basis-audit.json`, `existing-seven-tangent-audit.json`, `tangent-repair-install.json`,
`final-placement-preservation.json`, `final-installed-resource-inventory.json`이다. 새 광역 하네스는
추가하지 않았다. 공용 C++/shader/Product 빌드는 root 통합 작업이 소유한다.

## G05. 아직 연결하지 않은 범위

PS525 sky-cinema와 PS528 sky는 서로 다른 원본 shader를 사용한다.528의 native2,208정점/
11,904index/UV2개 및 lv_sky_0043_d texture를 out candidate로 준비했다. native MIC는 MLM_Custom이며
모든 feature switch가 꺼졌지만, 이것만으로 unlit이라고 단정할 수 없다. 실제 shader-cache의
NoLightmap/SH/direct 정책과 Skybox lighting channel 소비를 조사 중이며 제품에는 아직 적용하지 않았다.
527의 원본 component translucentSortPriority=-5 및 원작 전체 alpha/depth/fog 합성도 현재 공통
renderer adapter와 구분한다. 이 문서는 석재 이외 전투공간 전체 object·원경·원본 CPU 조명·후처리
복원이 끝났다고 주장하지 않는다.

Client/UI 실행·조작·캡처와 visual PASS는 수행하지 않았다. 사용자 화면 비교는 수치 검증과 별개다.

## G06. 원본 바닥 재질의 Deploy staging 실패 교정

사용자가 `overlay:BG_RAD_VALTAN_A:bg_rad_valtan_floor01a_sm`에서 입장 rollback을 보고했다.
설치 A/B는 모두 mesh2개이며 mesh1은 `bg_rad_valtan_crack_floor01_mi_lsj`, emissive 경로는
빈 문자열이다. BossCatalog의 정확한 model/material binding은 `bg-source-opaque-masked`이고
참조 texture 누락은0이다. 기존 cook receipt와 설치 SHA도 일치했다.

`CDeployPropObject::Initialize`가 native surface에도 legacy EMISSIVE texture를 요구한 것이
이 입력의 실패 조건이다. 기존 `Should_RenderDeferredEmissiveOverlay`와 동일하게 native
surface를 구분하고 LEGACY surface에만 기존 필수 검사를 유지했다. native surface는 기존
MapAssetRenderUtils/CModel/CMaterial 경로로 그린다. public header와 shader는 변경하지 않았다.

deploy flag를0으로 바꾸는 시도는 Map Effect의 파괴 바닥 owner 검증에서 거절됐다. 해당 두 행은
즉시 원복했고 catalog·placement·파괴 상태·Resources는 기존 입력을 유지했다. Area publisher의
Validate 및 Publish/Check를 통과했다. publish는 Git checkout의 CRLF를 정본 LF로 정규화했으며
runtime의 의미상 Git diff는 없다. 전체13,184배치 SHA도 이전값과 같다.

실제 변경 CPP를 동일 VS Insiders/v143 14.44 x64 Debug 도구로 독립 컴파일해 통과했다.
`out/ValtanAdmission20260915/compile-deploy.log`, `installed-model-audit.json`에 근거를 남겼다.
Product 링크는 사용 중인 Client/Server 종료를 기다리므로 아직 실행하지 않았다. 기존 프로세스를
임의 종료하거나 수정 EXE 적용·실제 입장 성공·visual PASS로 기록하지 않는다.

## G07. 전체 맵 재질 연결 재개 결과

앞선 443석재/717asset 이후 전체 static component13,091개, native mesh260개, effective MIC286개를
실측했다. 중앙 아래층 SL00 export1196의2slot을 먼저 연결하고 지원 family10,589배치와 native NULL
fallback을 입증한49배치를 추가 배포했다. 현재 전체 배치는13,184, material4,511행,
placement lighting11,076행이다. Mapset은 BASE/PS/SL00~SL05 여덟 shard,4,056 catalog행,
3,967 unique asset이다. 최대 SL01은1,099asset으로 기존2,048개 한도를 유지한다.

single catalog의 모든 정의를 shard로 분배하고 기존 mapset 자동 선택 loader/publisher를 재사용했다.
Client.vcxproj/.filters의 single catalog None1항목을 mapset와8쌍 shard17항목으로 교체했다.
두 프로젝트 XML parse를 통과했다. 새 모델 runtime나 별도 placement ID는 만들지 않았다.

중앙 아래층 native1,542정점/4,374index/2slot에 component COLOR와 tangent.W/UV1을 복구했다.
원본 MIC는 lv_lut_heartrb.mat.bg_pvp_retown_floor02_mi와
bg_shs_rcarena_c.mat.bg_shs_rcarena_module01_mi_kyo이며 sourceOverlayFlags263/BG133을 사용한다.
RNM normalizedaveragecolor1_55/directionalmaxcomponent1_55의 원본 mip0이 기존 설치와 일치해
추가된 mip를 보존했다. shadow는 원본 DOM GUID와 component atlas 좌표를 사용한다.

broad geometry300group은 원본 정점/index와 fresh converter를 대응해 COLOR/UV/TBN을 보존했다.
서로 다른 package의 같은 leaf6쌍은 native 출력 경로를 source 전체 경로로 분리했다. 잘못 결합된
native/glTF는 corner 검사에서 차단하고 다시 검증했다. 큰 원경 mesh의 float32 위치 반올림은
원본 위치에 비례한1.2e-7 한도와2e-5 최소 한도로 검사했다. 원본 normal/tangent가 평행하거나
퇴화한2mesh/6배치는 임의 basis를 만들어 설치하지 않았다.

NULL texture13항목은12 MIC의 native uniform expression와 referencedTextures index로 확인했다.
정확한 fallback normal/spec/flat_white/flat_gray/ambientreflection을 사용하는49배치,
37variant/50material행을 두 번째로 배포했다. PF_A8R8G8B8은 native mip를 보존했다. PF_V8U8 normal은
UModel decoded RGBA를 재표본화 없이 DDS로 포장해 TGA pixels와 일치를 확인했다. Crunch flag512인
RNM은 압축 payload를 DXT로 오인하지 않고 UModel의 package별 복수 -obj 추출로 DDS를 얻었다.
UModel surface/RNM은 mip0이며 원본 compressed 하위 mip 전체 복원으로 표현하지 않는다.
G8 shadow는 native mip 전체를 보존했다.

원본3MIC/189variant의 use_specular=false/use_subspecular=true와 specular texture 입력을 parser가
거절하는 불일치를 발견했다. root가 C++와 publisher의 bit8 조건을 bit4 또는 검증된
sourceSubspecular.x>0으로 일치시켰다. 일반 반사광을 임의 활성화하지 않았다.

두 번째 Area Publish는8shards/22runtime파일/13,184배치로 통과했다. mapset SHA는
5a3d65a4ea8ad499a567751d0a8e4e657fe78c78417c4424208498e031d3161b다.
아래층 작업 시작 전과 asset 참조가 다른 배치는10,639이며 stable ID/source ID/TRS/visibility는
전량 보존했다. 필수 resource1,651개의 누락은0이고 material/placement-lighting identity 중복도0이다.
별도 최종 Area Check 결과는 아래 후속 검증에 기록한다.

근거는 out/ValtanArenaMaterial20260915의 current-installed-audit.json, Pass1/broad-install.json,
Pass2-install.json, all-source-geometry.json, all-resolved-mics.json, null-source-fallbacks.json,
NullNative와 NativeCooked receipt다. root Product build는
out/BuildPipeline/runs/20260915T043231163Z-debug-product.json을 참조한다. 그 뒤49배치는 데이터
배포이며 새 C++ 변경을 포함하지 않는다. Client/UI 실행·조작·캡처·visual PASS는 수행하지 않았다.

## G08. 전체 복구의 남은 입력

현재 mapper에43 MIC가 미지원으로 남는다. overlay subspecular/specular saturation/emissive/bump,
wind15 MIC, monster material, molding, mossfog/volcano/lava, water와 sky-cinema를 계속 복구한다.
invisible navigation helper와 native suffix319개의 legacy vertex lightmap/LOD 경계도 복구 수에
포함하지 않았다. PS525/528 하늘과 dominant LightFunction의 projection/time 소비자도 미완료다.
위 배포 수치를 전체 원작 화면 복원 완료로 해석하지 않는다.

후속 Area Check도8shards/22runtime파일/13,184배치 전량 일치로 통과했다. 이번 Data·프로젝트 XML·
PLAN/RESULT의 git diff --check도 통과했다. 사용자 화면 확인은 이 수치 검증에 포함하지 않는다.


## G09. overlay 추가 9 MIC의 소비 경로와 검증용 세트

기존 program7에 subspecular, specular saturation, constant emissive, bump 분기를 연결했다.
새 family나 두 번째 model 경로를 만들지 않았다. MapAssetCatalog/Publisher의 필드 검증,
Engine Model의 surface 검증, Material의 발광 texture 로드와 MapAssetRenderUtils의 초기화·바인딩을
함께 수정했다. 기존 PBR masked, BG subspecular-only 및 Deferred marker1 변경은 보존했다.

- subspecular: `mixedSpecular * intensity * pow(saturate(dot(directMixedNormal, unitView))², power)`.
  direct specular가 꺼진 cargobox에서도 subspecular는 남는다. mixed normal을 정규화하지 않는다.
- bump bit512: 원래 diffuse alpha로 UV를 이동하고 diffuse/normal/specular를 다시 읽는다.
  opacity는 원래 alpha+이동 alpha의 saturate, base diffuse 밝기는 원래 alpha+bump brightness의
  saturate다. overlay UV와 emissive UV는 원래 mesh UV를 유지한다.
- emission: overlay weight와 독립이며 이번 세 MIC는 flicker 없는 원본 분기다.
- mixed specular bit1024: native 방향 기반 overlay는 mixed normal, vertex-paint는 base normal의
  half-dot을 사용한다. GBuffer geometry.w로 선택을 전달해 기존 승인 floor를 보존했다.

`overlay-native-warp-check.json`: exact native map 9개 × 시선/정점 alpha/텍스처 alpha 8조건,
72개 WARP 비교 PASS, 최대 절대오차 8.285045623779297e-6(허용1e-4). native DXBC 명령을
직접 번역한 HLSL과 현재 product wrapper를 비교했다. UV에 따라 값이 달라지는 analytic texture
seam을 사용해 bump UV 변화도 대조했다. Base diffuse/추가 radiance/Direct/clip을 검사했으며
실제 DDS 필터링·화면 색감·RNM 전체 조합의 시각 일치를 대신하지 않는다. 원본 Depth-only
bump mask와 현재 shadow pass의 parallax 차이는 남아 있다. 원본 static shadow texture는 보존한다.

검증용 `BroadAuthoring`은 922 placement, 새 variant305/material387/lighting922를 포함한다.
29 geometry group 및 texture225개의 실제 파일이 준비됐다(기존219, 추가6 export). native basis
퇴화2그룹/6배치는 계속 제외했다. 이 세트는 **현재 Data authoring/Resources/runtime에 설치하지
않았다**. 현 제품맵은 G07의 unique3,967/material4,511/lighting11,076 상태를 유지한다.
C++/전체 shader의 isolated compile 및 product 배포 여부는 root 통합 결과에서 확인한다.


G09 후속 검증: 공식 `Publish-MapAuthoring.ps1 -Mode Validate`를 제품과 분리된
`out/OV` 검증용 ProjectRoot에서 실행해 8 shards / 22 files / 13,184 placements PASS를
확인했다. 실제 파일을 사용하는 검증이며 제품 authoring/Resources/runtime에는 설치하지 않았다.
긴 staging 경로의 .NET 파일 열기 제한은 짧은 검증용 junction으로 해결했다.
root 통합에서 overlay 관련 Engine Model/Material, Client MapAssetCatalog/MapAssetRenderUtils
4개 TU의 isolated compile과 Shader_VtxMeshBinary/Shader_VtxMeshMapInstance/Shader_Deferred
FXC compile도 PASS로 확인했다. 실행 중 제품 EXE/DLL/CSO의 교체 완료를 뜻하지 않는다.

## G10. 하늘·블랙홀의 원본 소유자 대조

PS525와 PS527은 같은 cloudplane mesh의 다른 MIC이며 각각 원본 actor layer
`lv_kismet_layer_hidden_normal`, `lv_kismet_layer_hidden_extreme`에 속한다.
SCENE06A의 원본 ref660 `extreme_mood`는 ref51 extreme layer Show와 ref52 normal layer Hide를,
ref661 `normal_mood`는 그 반대의 Show/Hide를 실행한다. 이름에 따른 추정 대신 실제 Kismet
output-link input index까지 대조했다. PS526도 extreme layer다. 현 상태에서 두 구름을 함께
보여 주는 것을 원본 분위기 완료로 기록하지 않는다. 이 조사에서는 설치 visibility를 바꾸지 않았다.

PS528의 `sky_base_trn`은 native custom-lit material이다. exact native Base shader
`dddf2e0416908b478f0ae8e790661470`, Direct shader `9499dce597251a44a59bc75d3f6bf2c0`를 추출했다.
source diffuse brightness20, color[1,.376288414,.210773408,1]과 sky_0043_d를 사용한다.
원본 InterpActor CDO는 StaticMeshComponent에서 MyLightEnvironment를 상속한다.
따라서 인스턴스에 명시적 LightEnvironment property가 없다는 이유로 unlit 재질로 바꾸지 않는다.
Skybox 조명 채널과 동적 LightEnvironment의 실제 입력 소비자는 아직 닫히지 않았다.

기존 `boss.valtan.blackhole`은 V2 Independent에 등록된 authored preview group이다.
leaf는 위치(0,50,0), scale(50,50,1), 붉은 colorMul과 일반 texture/mask 조합을 사용한다.
이 문서 작성 시 BOSS_VALTAN binding/pattern cue/Valtan presentation에서 이 group을 참조하지 않으며,
원본 source provenance도 없다. 이를 PS525/527/528 원본 하늘 또는 제품 전투 cue로 대신 연결하지
않았다. CMapAssetObject의 legacy PresentationVortex 설정도 현재 외부 호출자가 없는 상태다.
새 FX product cue를 활성화하지 않았다.

원본 근거는 `out/ValtanArenaMaterial20260915/SkyNative`, `sky-source-actor-lightenvironment.json`,
`sky-engine-owner-defaults.json`, `sky-sequence-owner.json` 및 root가 추출한
`out/FullMapRestoration20260915/ValtanSequences/LV_LUT_HEARTRB_ED_SCENE06A.json`이다.


G10 후속 수치 검증은 `SkyNative/warp-check.json`에 기록했다. 원본 cached DXBC 자체를 WARP로
재생한 PS525/528 Base·Direct 총32조건과 정리한 source surface 계산의 최대 오차는
1.1920928955078125e-7이다. 원본 ISGN에 맞춘 VS carrier, MIC 파라미터, 명시적으로 바꾸는
engine/pass 입력과 단색 검증 texture를 사용했다. 실제 texture UV/필터링과 엔진의 실효 기본값은
이 검사로 승인하지 않는다. 두 material의 vertex uniform/texture 식은0이며 원본 no-density
VS도 추출했다. cloud parent의 bAllowFog=false를 확인했으므로 해당 cloud에는 fog identity를
사용한다. 원본 NULL texture_cloud_opacity의 native 참조는 plane_cloudtex_01_d다.

`LightFunctionNative/warp-check.json`은 원본68명령 PS와 분리한 함수의16조건 비교 PASS,
최대 오차4.76837158203125e-7을 기록한다. 시간·projection·fade/desaturation·analytic texture
UV를 바꾸며 검사했다. 원본 global PS는 d0a06e30c601e54086519ed833b52ae7, VS는
30bc97cbe876374d895723a54ab2fd90다. CDO Scale은[1024,1024,1024]cm,
DisabledBrightness는1이다. 실제 ScreenToLight, fade/desaturation producer는 미확인이다.
source program90이나 제품 기본 조명 변경을 추가하지 않았다.

SCENE06A extreme_mood는 env_on도 호출한다. PS의 실제 remote link는 환경 volume_0을 켜고
volume_3을 끈다. volume_0 DDL override는 brightness1.5/RGB236,138,129,
volume_3 기본 DDL override는 brightness2/RGB191,164,154다. 원본 환경 데이터는
sky-environment-moods.json에 따로 저장했으며 사용자 rollback 대상 기본밝기/profile44를
변경하지 않았다.

## G11. 이번 실행 준비 시점의 설치 결정

새 overlay9 MIC/922배치는 기존 원본11파일의 SHA가 모두 staging 당시와 일치한다.
현재 authoring과 후보를 직접 비교해 stable ID/source ID/TRS/visibility 전량 보존 및
정확히922개의 asset 참조 변경을 확인했다. native 수치 검증, 실제 소비자 compile,
별도 공식 Area Validate를 마쳤으므로 대응 Product EXE/DLL/CSO가 설치된 뒤 적용 가능한
후보다. 이번 인계에서는 기존 지시대로 out-only를 유지한다. live Data/Maps/Authoring 및
Client/Bin/DataFiles/Map을 이 후보로 덮어쓰지 않았다. SHA 일치는 실행 중 MapTool의 미저장
메모리 draft 상태를 대신 확인하지 않는다.

최종 재실측도 unique assets3,967/material4,511/lighting11,076 및 resource 누락0이다.
추가 overlay의 실제 설치, native basis 퇴화2그룹/6배치, shadow-only parallax,
하늘 engine producer, 나머지 wind/물/용암/특수 재질과 사용자 화면 판정은 남은 작업이다.
install_incremental.py의 과거 Pass2 backup 이름을 그대로 재사용하지 않는다. 후속 적용은
별도 overlay backup/receipt와 현재 source freshness를 사용해야 한다.
근거는 overlay-install-readiness.json과 current-installed-audit.json이다.

## G12. overlay 추가 설치 재개

root의 후속 설치 요청을 받아 원본11파일을 재검사했고 staging 시점 SHA와 모두 일치했다.
검증 완료9 MIC/922배치의 최소 추가 설치를 준비한다. G11의 out-only 기록은 그 인계 시점의
상태이며, 이번 설치의 실제 완료 여부는 이 절의 후속 실행 결과로 구분한다.

현재 사용자 VS 빌드가 진행 중이라는 root 확인에 따라 authoring/runtime commit과 publisher는
대기한다. 별도 OverlayInstall staging과 CAS 기준본만 준비하며, 하늘/LightFunction은 제외한다.

## G13. 2026-09-25 3시·9시 파괴 바닥의 재질 소비 재확인

사용자가 지정한 대상은 파괴되는 A/B 석재와 난간 여섯 Deploy 배치다. 작은 원형 바닥
SL00 export1274의 static shadow 수정과 다른 작업이다. 현재 floor84의 placement는
7000000000000000005/6/7, floor30은7000000000000000001/2/3이다.

현재 제품의 세 모델은 `SourceDeployRestore`를 사용하며 A/B 각2슬롯과 rail4슬롯이
BossCatalog override에 정확히 대응한다. A/B6종·rail15종 texture는 실제로 존재하며,
원본/게시 deploy catalog와 placement의 의미도 일치한다. A/B rock04와 crack의
원본 scalar/color21항목 및 texture6개의 SRGB/linear·WRAP 설정이 현재 데이터와 일치했다.
원래 static 배치에 동일 A/B/rail asset을 참조하는 행은0개다.

제품은 `DeployPropRuntime → ActorCatalog descriptor → CModel/CMaterial → DeployPropObject
→ MapAssetRenderUtils`를 사용한다. Clone은 재질을 보존하며 native surface에서는 legacy
발광 overlay를 건너뛴다. 현재 제품의 텍스처 누락이나 원본 재질 override 유실은 발견하지 못했다.

확인된 결함은 MapTool의 `Ensure_DeployAuthoringPrototypes`가 raw path로 모델을 생성하던
부분이다. `MapTool_Area.cpp`에서 intact/fractured 모두 상대 asset ID로 ActorCatalog
descriptor를 만든 뒤 기존 CModel에 전달하도록 수정했다. 생성 전 descriptor 실패는
상태 메시지를 남기고 해당 prototype을 추가하지 않는다. 기존 경로 fingerprint·.01 사전 배율·
model kind·배치·파괴 동작을 보존했다. 독립 코드 검토와 UTF-8/CRLF 보존·diff 검사는 PASS다.
새 C++ 파일·프로젝트 등록·데이터/Resources 변경·추가 게시가 없다. 기존 메모리 prototype을
자동으로 다시 만들거나 사용자 편집을 Reload하지 않았다. 통합 Debug Product Build는
Engine·Shared·Server·Client 모두 PASS이며 실제 `MapTool_Area.cpp` 재컴파일과 Client 링크를
확인했다. `out/BuildPipeline/runs/20260925T013413528Z-debug-product.json`과
`out/KoukuAuthoring20260925/product-build-confirm.log`에 근거가 있다. 기존 컴파일·셰이더·
외부 라이브러리 PDB 경고는 남아 있으며 빌드 오류는0이다.

제품 화면의 색 차이를 위 저작 경로의 결함으로 단정하지 않는다. family7의 원본 hemisphere와
scene color 입력을0으로 둔 것은 09-08부터 명시된 미복원 경계다. 정적 석재는 RNM을 소비하지만
합성 Deploy A/B에는 연결된 RNM이 없고, 다른 배치의 atlas를 복사할 근거도 없다.
원본 실제 생성자의 material override·동적 조명 입력과 사용자 현재 화면은 별도 확인 대상이다.
공통 렌더링 옵션과 현재 재질 밝기·색상·텍스처는 임의로 바꾸지 않았다. Client/UI 실행과
사용자 화면의 시각 검증은 수행하지 않았다.

원본15 UPK의 actor/component·컷신 재질 track 조사와 정확한6배치/재질 근거는
`out/ValtanDestructible20260925/source-owner-lighting-review.json`에 기록했다.
원본 EFStaticMeshActor의 LightEnvironment 기본 연결은 확인했지만 해당 바닥의 실제
spawn class join을 확인하지 못했으므로 그 값을 바닥에 적용하지 않았다.
같은 receipt의 추가 조사에서는 원본 Map37051의 DeployData169 actor를 Prop27정의와
LookInfo17모델에 연결했으나 A/B/rail의 직접 참조는 없었다. 이는 검사한 범위의 부재이며
게임 전체에서 생성자가 없다는 결론이 아니다. 실제 프로젝트의 문제 화면은 아직 받지 못했다.
