# Rendering Workbench 원본 근거 조사 결과

## G00. 조사 범위와 결론

2026-10-04에 기준 commit `a11c72015d34d09fe2678ababed908cfea8ace20`의 복원 문서,
추출기와 현재 설치 입력을 읽었다. 조사 중 작업 브랜치는
`codex/rendering-workbench-presentation`으로 전환됐다. 이 조사는 제품 코드·Data·Resources를
변경하거나 Client를 실행하지 않았다. 본 문서 외 파일 수정과 별도 원본 재추출도 하지 않았다.

확보된 근거는 UE3 cooked package 계열과 그 안의 material/static-set/ShaderMap/DXBC다.
현대적인 PBR·SH·BRDF·DirectX 11 사용만으로 일부 렌더러가 UE4에서 이식됐다고 판정할 수 없다.
이번에 읽은 공식 공지와 도구 유지자 자료에는 해당 부분의 UE4 이식을 확정하는 근거가 없다.
반대로 모든 내부 코드에 UE4 계열 기술이 전혀 없다고 증명한 것도 아니다.

현재 Workbench의 복원 단계는 현재 구현에서 입력·기여를 순서대로 비교하는 세션이다.
과거 개발일의 EXE·WModel·재질 전체를 재실행하는 기능으로 표시해서는 안 된다.
원본 연산의 수치 일치, 현재 runtime 연결, 최신 원작 전체 화면의 동등성은 각각 별도 판정이다.

## G01. 외부 1차 근거와 의미

| 자료 | 직접 확인한 내용 | 이 자료로 확정하지 않는 내용 |
|---|---|---|
| [Lost Ark 공식 DirectX 11 일원화 공지](https://lostark.game.onstove.com/News/Notice/Views/2245), 2023-01-04 | 2021-12-22 DX11 적용 후 DX9를 병행했고, 2023-01-18부터 DX11로 일원화한다는 공지 | UE3→UE4 엔진 전환, 개별 shader의 UE4 출처 |
| [Epic UE3 Samaritan 설명](https://www.unrealengine.com/blog/samaritan), 2012-03-20 | UE3 자체의 DX11·Shader Model 5, image-based reflection, SSS 등 렌더링 기능 | Lost Ark가 이 데모의 모든 기능을 사용한다는 주장 |
| [Gildor 포럼의 Lost Ark 전용 UModel 빌드 안내](https://www.gildor.org/smf/index.php/topic,3055.msg41220.html), 유지자 spiritovod 본문 최종 수정 2026-02-20 | UE3 분류 아래 게임별 override와 지역 선택을 사용한다. mesh/texture/animation, ACL, EFTexture2D, morph 및 scale-key 지원 변경을 설명하며 일부 KR package의 다른 암호화 문제를 명시한다 | Gildor 기본 배포본이 모든 Lost Ark package를 지원하거나, 최신 원작의 모든 shader·scene 상태를 export한다는 보장 |
| [UE Viewer 공식 FAQ](https://www.gildor.org/projects/umodel/faq) | cooked asset과 editor source asset은 다르다. UModel은 일부 객체 유형만 지원한다. `.mat`은 importer용 texture 연결 설명이며 `.tfc`의 texture metadata는 package에 있다 | export에 안 보이는 기능이 원본에 없다는 결론, `.mat`이 완전한 원본 material program이라는 결론 |

Gildor 포럼은 Smilegate의 엔진 발표가 아니라 해당 extractor 유지자의 지원 기록이다.
그 기록과 이 저장소의 UE3 v868 parser는 확보한 자료의 계열을 뒷받침하지만, 현재 원작의
모든 지역·빌드가 동일하다는 증거로 확장하지 않는다. 일반 UEViewer master/compat 목록만
보고 Lost Ark 전용 빌드의 존재와 지원 범위를 누락해서도 안 된다.

## G02. 저장소에 남은 package·ShaderMap 근거

| 현재 파일과 위치 | 확인한 계약 |
|---|---|
| `Tools/LevelPlacementExtractor/extract_ue3_placements.py:27`, `:335`, `:358`, `:382`, `:1606` | UE package tag, package/licensee version과 engine version을 별도로 읽고 receipt에 기록한다. package version 868을 UE4 버전 번호로 해석하지 않는다. |
| `Tools/LevelPlacementExtractor/README.md:191` | 현재 source ABI를 KR v868로 명시한다. native static-set record와 원본 offset/hash를 보존하며 미지원 layout은 실패한다. |
| `Tools/LevelPlacementExtractor/extract_artist_31470_shader_cache_oracle.py:52`, `:64`, `:743` | 원본 ReleasePC 경로와 global material/shader-cache package의 size·SHA256·v868 identity를 고정한다. ShaderMap suffix는 version868/licensee16을 검사한다. |
| `Tools/EffectPipeline/extract_ue3_material_shader_maps.py:1` | MIC effective static set→정확한 RefShaderCache map→VF/pass→shader ID→DXBC slice→uniform/register binding을 연결한다. 구조 receipt 성공만으로 runtime·수치·화면 동등성을 선언하지 않는다. |
| `Tools/LpkPipeline/unpack_lpk.py:1` | 프로젝트의 LPK reader는 EFTable DB, config, font 등의 archive를 읽는 별도 경로다. package/material shader 해석기와 역할이 다르다. |
| `Engine/Bin/ShaderFiles/Shader_SourceCharacterBaseGroup640.hlsli:915`, `:916` | native702의 source program ID `3b3abe5b3d623749aeec90310df73939`와 원본 DXBC 명령 주석을 가진 번역 함수가 남아 있다. 이는 현재 원본 bytecode 파일이 존재한다는 뜻은 아니다. |

ShaderCache oracle에 고정된 global material package는
`DKV6KRSCXY3T6D9CJIK3G.upk`, SHA256
`c0c3e35b48d8589d2e5014c99c64c0c32e05eace7ae02cfc8e6566f4eaf40150`이며,
shader cache는 `9XUFAXIP8BXBAP1NIEG66EF.upk`, SHA256
`be77e8af4443c4cca5614bec0545c0c735ab04a8b68a3781fb9dfb5a5f2123ad`다.
이번에는 추출기 안의 기존 identity를 확인했으며 해당 package를 다시 읽거나 hash 검증하지 않았다.

따라서 누락은 최소한 `원본 자료 미확보`, `추출기 해당 형식 미지원`, `추출됐지만 runtime 미연결`로
구분해야 한다. LPK unpack 성공, UModel mesh export 성공, material shader 복원 성공을
서로 대신 사용하지 않는다. archive 해제만으로 원작의 engine source나 runtime binding이
생성되지 않으며, exporter가 반환하지 않은 기능을 UE4 전용 기능이라고 이름 붙이지 않는다.

## G03. 현재 원본 간접광과 후처리의 실제 근거

이전 미복구 기록만 인용하면 현재 구현을 잘못 설명한다.
[09-22 복원 RESULT G13](../09-22/2026-09-22_CHARACTER_SELECT_MATERIAL_RESTORE_IMPLEMENTATION_RESULT.md#g13-원본-pbr-간접광-코드와-입력-owner-복원)은
FLOOR12 exact static key와 source PS82f66791, native binder, EFEngine SH packer를 근거로
SH9→packed7 및 component/view owner를 연결했다고 기록한다. BRDF는 원본 generator와
bit-exact인 128×32 RG16 lookup이며, 이전 프로젝트 128×128 근사와 별도다.
이는 새 색 grading LUT를 복원했다는 뜻이 아니다.

현재 `Data/Maps/Authoring/LV_LOBBY_CLASSSELECT_SL00/LV_LOBBY_CLASSSELECT_SL00.mapmaterials.json:83`에는
`sourceIndirect.model=UE3_NATIVE_PBR`, native BRDF asset, cube, packedSH가 실제로 있다.
현재 JSON parse에서도 `sourceIndirect` 블록181개를 확인했다.
`Client/Private/MapAssetRenderUtils.cpp:1209`는 해당 surface와 현재 source-indirect selector를
함께 검사하며, `:1237`부터 native SH·sky·ambient를 shader에 bind한다.
[10-04 A/B RESULT](2026-10-04_RENDERING_TECHNIQUE_AB_RESULT.md)는 이 selector의 독립 비교와
복원 transaction을 기록한다. 모든 재질이나 전체 GI를 켜고 끄는 선택자는 아니다.

이번에 다음 네 개의 **정확한 설치 경로만** 읽어 존재와 SHA256을 확인했다.
Resources 전체 검색은 하지 않았다. 경로 기준은 `Client/Bin/Resources/Map/Lighting/CharacterSelect/`다.

| 설치 파일 | 이번 SHA256 |
|---|---|
| `character_select_native_brdf_rg16.dds` | `c374749515c1a950135dda67463219531c6285697732e986b30955a518afea8b` |
| `pbr_cubemap_00.rgbm.cube.dds` | `67620fe0c6ae9716a1eadea1cc45d77d9bbae9fd305348ec78822e32cbfa8afd` |
| `lv_lut_valhatrond_04_hdr01.rgbm.cube.dds` | `f1f158442574dee0a2f008ef3cd5c908148eabf9f1bfe7293b3f352a57a57ea0` |
| `lv_lut_valhatrond_04_hdr02.rgbm.cube.dds` | `a88e0c252166e170f0cb23afdff336dd6dcdb19d07a7ccbf6d371cd9a5b52e12` |

앞의 두 hash는 G13 설치 기록과 같다. 이 일치는 해당 설치 산출물 보존의 증거이며,
이번에 원본 생성기를 재실행하거나 최신 retail 입력과 재대조했다는 증거는 아니다.

후처리는 `Data/Rendering/Authored/RenderingProfiles.json:82`의 `UE3_CUSTOMIZABLE` 입력과
`Engine/Bin/ShaderFiles/Shader_Deferred.hlsl:1633`의 실제 tone 계산이 있다.
source postprocess selector는 tone·grading 묶음을 선택하며 OFF는 fallback tone 경로다.
BRDF lookup과 color-grading LUT를 같은 단계나 같은 파일로 설명하지 않는다.
기존 G13은 정적 LUT 후보가 neutral인 사실과 전체 camera/UI/volume 활성 체인 미확정을 구분한다.

## G04. 현재 로컬 원본의 가용 범위

아래는 2026-10-04 `Test-Path -LiteralPath`로 확인한 정확한 경로다. 모두 존재하지 않았다.
다른 드라이브나 사용자가 별도로 보관한 위치까지 검색한 것은 아니다.

- `C:/Users/user/Desktop/Resource_LostArk`
- `C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC`
- `C:/Users/user/Desktop/LostArk/out/ClassMovies20260925`
- `C:/Users/user/Desktop/LostArk/out/RenderingIndirect20260923`
- `Data/Effects/Imported/Artist/Materials/skill.31470.shader-cache-oracle.receipt.json`
- `Data/Effects/Imported/Artist/Materials/skill.31470.typed-material-evidence-contract.json`

현존하는 것은 위의 설치 DDS, authoring JSON, 번역 shader·명령 주석, 추출기/생성기 소스와
기존 RESULT다. [primitive opacity RESULT G03](2026-10-04_MOVIE_WARLORD_PRIMITIVE_OPACITY_RESULT.md#g03-근거와-화면-판정의-범위)의
raw 자료 부재 경계와도 일치한다. 이전 WARP 수치와 추출 receipt의 결과를 역사적 기록으로
인용할 수 있지만 이번 조사에서 그 raw oracle을 재검증했다고 표시할 수 없다.

## G05. 촬영 설명과 추가 복원 판정

Workbench의 짧은 안내 문구는 다음 수준이 적절하다.

> 현재 모델에 기본 재질부터 원본 재질·환경광·후처리까지 순서대로 적용해 비교합니다.
> 현재 구현의 복원 단계 비교이며, 과거 개발 버전이나 원작 전체 화면을 재현하는 기능은 아닙니다.

원본 근거 설명은 다음처럼 제한한다.

> UE3 cooked 자료에서 확인한 재질·셰이더·환경 입력을 연결했습니다.
> UModel export에서 빠진 항목은 원본 자료, 추출기 지원, runtime 연결을 따로 확인합니다.
> UE4 이식 여부는 확인되지 않았습니다.

새 원본 복원은 대상의 stable asset/material ID와 원본 package/build/hash, effective static set,
VF/pass/shader ID, texture·uniform binding 및 실제 CPU/GPU 소비자가 이어질 때만 진행한다.
기존 번역본에서 단순 누락을 고치는 작업은 보존된 원본 명령과 현재 입력 owner로 범위를
증명하고, 원본 자료를 새로 확보한 복원과 구분한다. 원본과 후보의 동일 입력 수치 비교,
OFF 시 기존 결과 보존, 설치·publish 및 사용자 장면 판정은 각각 보고한다.

현재 일반 SSGI·SSR 실험을 원작의 GI 복원이라고 표시하지 않는다.
[10-03 Workbench RESULT G13](../10-03/2026-10-03_PROFILER_RENDERING_WORKBENCH_RESULT.md#g13-실제-ssgi-ssr-렌더링)은
MapPBR marker3에 대한 현재 화면의 가산 실험이며, 기존 baked GI·RNM·IBL을 유지한다고 명시한다.
GTAO·volumetric·planar 같은 미구현 기법을 촬영 단계의 작동하는 toggle로 보여서도 안 된다.

## G06. 이번 검증과 미실행

공식 웹 원문 열람, 현재 소스와 문서의 해당 구간 대조, 알려진 raw 경로의 존재 확인,
현재 mapmaterials JSON parse와 sourceIndirect181개, 정확한 설치 DDS4개 SHA256 확인을 수행했다.
본 문서의 UTF-8 no BOM, 상대 링크 존재, 줄 끝 공백 부재도 확인했다.
제품 기능 변경은 없으며 JSON/publisher 변경,
원본 package 재추출, raw DXBC 재생, 새 C++/shader 컴파일과 Client/UI 화면 검증은 실행하지 않았다.
최신 원작의 내부 엔진 구현·전체 활성 postprocess·전체 화면 동등성은 확정하지 않았다.

## G07. 재확인한 원본 자료와 복원 경계

이 문서의 앞선 G04 원본 미설치 판정 이후 사용자가 게임을 설치했다. 현재
`C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC`의 대상 package를 실제로 읽었다.
Lost Ark 전용 `umodel_lostark_v7.exe`와 기존 source parser로 LAND01의 physical/logical/object identity,
package868/licensee16/engine12097, LC622의 정확한 static key와 shader map을 확인했다. 이 숫자는
Unreal Engine 제품 버전 번호가 아니며 설치본 전체를 검증했다는 뜻도 아니다.

Bern LC622의 fresh original VS/PS는 기존 planar XY UV 전달과 일치하고 height 기반 절벽 투영은
발견되지 않았다. height/weight 세 texture의 전체 mip payload는 설치 DDS와 일치했다. 3969정점·7688면의
oriented topology/winding도 source와 일치하며 최대 위치 오차는2.441406e-6m다. 명시적 TwoSided=true
override는 없었으나 native/CDO의 미직렬화 기본값까지 해석한 것은 아니므로 임의 cull/UV 변경 근거로
쓰지 않는다. 근거는 `out/BernSourceReload20261004`의 계약·texture·winding receipt에 있다.

Character Select SL00 현재 material280개 중 **181개**의 `environment.sourceIndirect`에 native SH·cube·
BRDF 입력이 있다. RNM과 placement lighting도 별도로 연결돼 있다. 09-22 RESULT G13의 native SH packing과
128×32 RG16 BRDF 복구를 이전의 미확정 기록으로 되돌려 설명하면 틀린다. 반면 다음 경계는 아직 원본과
같다고 확정하지 않는다.

- Static shadow CPU penumbra 값은 `PROJECT_ADAPTER`다. 현재 원본 CPU owner/SetMesh에서 정확한 값을
  확보하기 전에는 추정 width를 원본 복원값으로 바꾸지 않는다.
- 일부 native hair의 projected shadow owner와 dynamic source-character SH/probe는 MapPBR의181개
  복구 입력과 다른 문제다. 현재 scene ambient/shadow adapter를 원본 동적 환경 전체 복원으로 표시하지 않는다.
- 전체 camera/UI/volume의 실제 postprocess 활성 체인, nonuniform-scale VS basis와 원작 전체 화면
  동등성은 미확정이다. 빈/중립 정적 LUT가 전체 활성 체인의 부재를 증명하지 않는다.

이번 audit에서 이 경계의 정확한 새 교체값까지 확보한 추가 복구는 없다. 후보·미확정은 미복원으로
분리하고, 밝기·파란 tint·환경·LUT를 근거 없이 바꾸지 않는다.

## G08. assembler로 확인할 수 있는 것과 engine owner의 한계

정확한 ShaderMap에서 뽑은 DXBC/assembly는 그 permutation의 연산, 분기, sample 순서, register와
semantic 사용을 보여준다. source static set·uniform expression·texture metadata·mesh/VF 채널을
함께 연결하면 UV 수식, COLOR0/alpha, material constant, BRDF·normal·blend 같은 static material 경로를
현재 엔진에 복구하고 동일 입력의 원본 DXBC와 WARP 출력으로 대조할 수 있다.

하지만 `cbN[index]`를 읽는 명령만으로 실제 게임이 매 프레임 그 register에 넣은 값을 알 수는 없다.
view/primitive transform, 골격 basis, dynamic SH/probe, shadow projection·penumbra, scene HDR snapshot,
카메라/volume의 노출·LUT 선택과 lifetime은 원본 CPU binder와 scene owner의 책임이다. 이 입력이
미확정이면 shader 수식을 옮겨도 전체 화면 복원이 아니다. UModel의 `.mat` 연결 설명이나 LPK unpack
성공도 이 owner를 대신하지 않는다. 누락은 원본 미확보·extractor 형식 미지원·runtime 미연결로 나눠 기록한다.
[UE Viewer FAQ](https://www.gildor.org/projects/umodel/faq)와
[Lost Ark 전용 UModel 지원 기록](https://www.gildor.org/smf/index.php/topic,3055.msg41220.html)을
게임 내부 engine source의 공개 또는 전체 최신 package 지원 보장으로 해석하지 않는다.

원작의 DX11 전환은 [Lost Ark 공식 공지](https://lostark.game.onstove.com/News/Notice/Views/2245)로
확인되지만 UE4 이식을 뜻하지 않는다. UE3에도 DX11/SM5 렌더링이 있었음은
[Epic Samaritan 설명](https://www.unrealengine.com/blog/samaritan)으로 확인된다.
현대적 PBR·SH·BRDF가 보인다는 이유만으로 'UE4로 올라간 부분이라 UModel로 못 뽑는다'고 단정하지 않는다.

## G09. 설치된 UE5와 별도 custom 확장

로컬에는 `C:/Program Files/Epic Games/UE_5.8/Engine`의 Build.version5.8.3/CL58210709와 Lumen·RayTracing
renderer/shader source가 있고, `C:/Users/user/Desktop/UnrealEngine/UnrealEngine`은5.7.4 source checkout이다.
`C:/UE5`는 Aura 프로젝트다. 경로와 파일 존재를 확인했으며 UE 자체를 실행·빌드한 것은 아니다.

현재 제품은 `Graphic_Device.cpp`에서 D3D11 device를 생성하고 ps_5_0 경로를 사용한다. 설치 UE의 Lumen은
FScene/FRDGBuilder, Surface Cache/Mesh Cards, Global Distance Field와 scene update에 결합돼 있다.
따라서 단일 checkbox나 shader 파일 복사로 같은 시스템이 되지 않는다. Epic도 screen trace 뒤에
software distance field 또는 hardware scene tracing을 사용하는 구조로 설명한다.
[Epic Lumen Technical Details](https://dev.epicgames.com/documentation/en-us/unreal-engine/lumen-technical-details-in-unreal-engine)

Windows의 hardware Lumen/DXR 방향은 D3D12 장치·resource/descriptor/synchronization, BLAS/TLAS,
동적/skinned geometry 갱신, hit material과 fallback을 실제로 구현하는 별도 기반 작업이다.
현재 저장소에 제품 DXR 실행 경로가 있다는 의미가 아니다.
[Epic hardware Lumen 요구와 갱신 비용](https://dev.epicgames.com/documentation/en-us/unreal-engine/lumen-technical-details-in-unreal-engine)

Epic이 제시한 Lumen의 해상도·플랫폼별 성능 예산은 이 프로젝트의 FPS 보증으로 옮기지 않는다.
[Epic Lumen Performance Guide](https://dev.epicgames.com/documentation/en-us/unreal-engine/lumen-performance-guide-for-unreal-engine)

이번 첫 custom 단계는 기존 D3D11 SSGI를 독립적으로 확장한 half gather + spatial resolve다.
UE 원문 코드를 복사하지 않았고 history·motion vector·화면 밖 scene cache·hardware RT는 추가하지 않았다.
후속 temporal 또는 scene-space GI도 실제 입력·pass·reset·실패 복원·A/B와 GPU 비용까지 연결한 뒤
구현 완료로 표시한다. 설명 항목에 버튼만 추가해서 구현된 것처럼 표시하지 않는다.


재설치 후 상세 근거는 [Bern LC622 결과 G08](2026-10-04_BERN_CLIFF_UV_RESULT.md#g08-재설치-완료-후-현재-판정),
[차원술사V 결과](2026-10-04_DIMENSIONMASTER_V_OWNER_ANCHOR_RESULT.md),
[워로드V 결과 G19](../09-09/2026-09-09_WARLORD_ASVF_FULL_RESTORE_IMPLEMENTATION_RESULT.md#g19-10-04-v-두-번째-클립-번개의-실제-본-배율-누락-복구)로 연결한다.
Bern의 UV/주변 geometry에서 교체할 차이는 찾지 못했다. 차원술사V의 원본 연결·분포를 대조했고
과거 사용자 삭제·조정을 추출 누락으로 복구하지 않았다. 워로드V는 실제 모델 본의0.01 재축소를
확인해 기존 정규화 범위만 수정했다. 각 항목의 원본 대조·수정·제품 빌드·사용자 화면 판정을 구분한다.
