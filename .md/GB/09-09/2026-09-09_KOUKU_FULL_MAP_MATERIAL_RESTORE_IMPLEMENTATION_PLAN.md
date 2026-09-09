# 쿠크 전체 맵 원본 재질 복구 구현 계획

기준일: 2026-09-09. 현재 제품의 CModel → CMaterial 경로를 확장한다. 기존 바닥 복원 결과와 원본 shader cache의 실제 Base/Directional/Baked 분기를 근거로 삼는다. 사용자 placement ID·Transform·visibility, 조명·노출 저작값은 유지한다.

## G00. 현재 실측과 입력 정본

- LV_LUT_MIDNIGHTC_ED catalog 323개 중 사용 asset 305개, 사용 material slot 379개, placement 3,231개(visible 2,897개).
- 기존 mapmaterials는 floor 2행과 diffuse sampler 3행이다.
- exact native material map 216/216개, DXBC 142개, Base+Light 조합 49개를 확보했다. texture가 없는 숨김 navigation cul01/02의 6개 binding helper 결과는 별도 미지원 경계다.
- native slot identity는 원본 material variant와 StaticMesh 기본 slot/컴포넌트 override로 대조한다. leaf-name 단독 일치는 조사 후보이며 자동 admission 근거로 쓰지 않는다.
- 실측과 원본 추출 자료는 out/KoukuFullMaterialRestore20260909에 둔다. 원본 설치 UPK와 추출 pack은 수정하지 않는다.

## G01. 원본 classic BG의 공통 표면 계산

Engine/Public/BinaryAsset/ModelAssetData.h의 MODEL_SURFACE_FAMILY에 SOURCE_BG_OPAQUE_MASKED=8을 추가한다. MODEL_SURFACE_PARAMETERS에는 선택된 native 분기 비트와 bumpOffset/bumpIntensity/bumpBrightness, UV 회전/이동, emissive flicker mode를 추가한다. diffuse·normal·specular·reflection·emissive와 기존 lightmap 경로를 재사용한다.

비트는 normal=1, bump=2, specular=4, separate specular texture=8, reflection=16, world reflection=32, masked=64, COLOR0.a normal strength=128이다. 알 수 없는 비트와 specular 없이 specular texture, reflection 없이 world reflection은 거부한다. UV 회전은 원본 uv_rotate × 2.000000476837158의 sin/cos이며 중심 0.5를 유지한다. UV 이동은 원본의 고정 uv_move_x/y 값이며 시간 속도로 바꾸지 않는다.

Client/Bin/ShaderFiles/Shader_MapMaterialSurface.hlsli에서 프로그램 8을 선택한다. diffuse alpha를 원래 UV에서 읽고 `(alpha-bumpOffset)*bumpIntensity*normalize(tangentView).xy`만큼 이동한다. masked는 이동 전후 alpha의 saturate 합을 0.3333과 비교하며, diffuse brightness에는 원래 alpha+bumpBrightness의 saturate 값을 곱한다. normal RG의 길이는 강도·vertex alpha를 곱하기 전에 Z 복원에 사용한다. specular는 원본 D.rgb 또는 명시 S.rgb를 사용하며 diffuse saturation/brightness를 다시 곱하지 않는다. reflection의 mask는 normal 분기가 있으면 normal.A, 없으면 1이다.

linear flicker는 `(2*(frac(time*speed)-0.5))²+minimum`, nested flicker는 원본 sin/cos 식을 유지한다. 원본 engine origin에 의존하는 phase/reflection offset은 명시적인 별도 입력이며 해결되지 않은 값을 복구 완료로 쓰지 않는다. 원본 texture sampler address와 sRGB는 source properties로 확인해 연결한다.

## G02. 실제 소비자와 실패 계약

- Engine/Private/Model.cpp: named material override 검사에서 프로그램 8의 유한값/분기/필수 texture 및 Resources 경로를 검사한다. 선택하지 않은 입력을 강제로 요구하지 않는다. 로드 실패는 기존 stage rollback을 유지한다.
- Engine/Private/Material.cpp: source BG의 선택된 SRV만 생성한다. CMaterial의 불변 입력과 clone 공유 계약을 유지한다.
- Client/Private/MapAssetCatalog.cpp: formatVersion 2 mapmaterials의 `bg-source-opaque-masked`를 strict parse하고 기존 staged commit에 연결한다.
- Client/Private/MapAssetRenderUtils.cpp: 프로그램 8의 source flags/UV/bump/flicker 값을 draw별로 바인딩해 이전 재질 상태가 남지 않게 한다.
- Shader_VtxMeshBinary.hlsl와 Shader_VtxMeshMapInstance.hlsl: COLOR0를 실제 표면 계산에 전달하고 같은 marker와 HDR albedo scale을 사용한다. masked shadow도 선택된 분기만 clip한다.
- Shader_Deferred.hlsl: marker 8의 Direct는 원본 BG의 abs(N·H), zero threshold, pow/clamp-to-1 식을 사용한다. 프로그램 5에 있던 RGB cap 2를 프로그램 8에 적용하지 않는다. RNM indirect와 기존 project ambient는 baked marker로 중복하지 않는다.
- Tools/MapPipeline/Publish-MapAuthoring.ps1: runtime과 같은 strict field/bit/경로/필수입력/placement lighting 검증 후에만 publish한다.

새 C++ 파일은 만들지 않는다. HLSL helper를 분리한다면 기존 FxCompile include 경계와 vcxproj/filters의 필요한 항목만 추가한다. Character program 6과 사용자 다른 세션의 sourceCharacter 수정은 보존한다.

## G03. 재질별 원본 입력 설치와 연결

216 native material 각각의 MIC 상속·default uniform·static switch·실제 texture register를 결합한다. 동일 body의 parameter만 다른 재질은 공통 표면 함수를 재사용한다. simple/seamless/overlay/translucent/spotlight/sky는 실제 분기 비교 없이 classic BG에 넣지 않는다.

Resources에는 실제 연결될 DDS의 원본 mip/sRGB/address를 보존해 설치한다. 기존 floor/sampler 행은 source identity가 같고 새 프로그램으로 동등성이 확인된 경우에만 교체한다. 추가 channel은 native source topology와 WModel topology를 일치시켜 COLOR0·UV1·tangent handedness를 설치하며 position/index/material slot과 사용자 placement transform을 보존한다. placement별 RNM atlas와 scale/bias는 기존 version 2 placementLighting 구조로 연결한다. source에 없는 channel을 UV0 복사나 임의 흰색으로 복구 완료 처리하지 않는다.

## G04. 검증과 인계

변경한 최소 C++ 컴파일, MeshBinary/MapInstance/Deferred FX 컴파일, JSON/PowerShell parse, publisher ValidateOnly 및 실제 publish, git diff --check를 수행한다. original DXBC와 새 shared surface의 동일 texture/CB/UV/view/vertex alpha 입력을 headless 수치로 비교한다. alpha discard, parallax UV, diffuse/specular, normal, reflection, flicker 시간과 RNM 입력을 구분해 검증한다.

Product 전체 빌드는 root가 수행한다. Client·UI 실행과 캡처는 하지 않는다. 화면 결과와 visual fidelity는 사용자가 직접 판단한다. RESULT에는 실제 설치·연결한 수, 자동 수치/컴파일 결과, 남은 원본 engine 입력과 특수 family, Drive로 전달할 Resources 상대 폴더를 별도로 기록한다.
