# Character Select 링과 캐릭터 재질 복구 구현 계획

## 현재 실제 상태와 이번 구현 범위

2026-09-08 사용자가 중앙 갈색 링과 창술사를 포함한 캐릭터의 축소 시 픽셀 깨짐 복구를 요청했다.
현재 여섯 playable class의 실제 body/equipment/weapon 45개 모델은 고유 texture 259개를
참조한다. 247개는 TGA이며 CMaterial의 decoder가 mip 1개만 만든다. 나머지 12개는
DimensionMaster DDS이며 파일에도 축소 mip이 없다. 누락 파일은 0개다.
중앙 링 15배치의 source5, atlas74/227, D/N/S/reflection은 이미 연결돼 있고 전체 mip과
Aniso16도 있다. 링에 같은 mip 수정을 반복하지 않고 원본 재질 식과 현재 소비자의 차이를 조사한다.

캐릭터는 Shader_VtxAnimMeshBinary와 socket 장비의 Shader_VtxMeshBinary를 사용한다.
현재 _s를 흑백 specularMask로 쓰지만 원본 일부 재질은 PBR이며 일부 specularPath는 BRDF LUT다.
모든 _s를 RGB 반사나 ORM으로 동일 해석하는 변경은 하지 않는다. 확정한 원본 의미만 연결한다.

## G01. 실제 텍스처 로드와 축소 샘플링

| 파일 | 변경 위치와 책임 |
|---|---|
| Engine/Private/Material.cpp | LoadTgaTexture: 기존 RGBA decode 뒤 CPU에서 전체 mip 생성, immutable texture의 모든 subresource 업로드 |
| Client/Bin/ShaderFiles/Shader_VtxAnimMeshBinary.hlsl | 실제 body/skinned 장비 texture 샘플에 16배 이방성 sampler 적용 |
| Client/Bin/ShaderFiles/Shader_VtxMeshBinary.hlsl | socket 장비의 표면 texture 샘플에 기존 SurfaceAnisotropicSampler 적용 |

원본 mip0 픽셀과 TGA 경로는 유지한다. diffuse/emissive는 sRGB를 linear로 풀어서 평균하고
다시 저장한다. alpha, normal, specular, dye mask는 데이터이므로 linear 평균한다. 렌더링
중인 immediate context를 Loader worker에서 호출하지 않는다. mip 계산은 모델 로드 시에만 한다.
기존 mip이 있는 DDS, lightmap/cube와 UI의 별도 CTexture 경로는 이 변경에서 유지한다.
DimensionMaster DDS는 현재 offline mip 도구의 지원 형식과 mip0 보존을 확인한 뒤 추가한다.

호출은 CPlayableCharacterAssetService → CModel → CMaterial → texture SRV → body/equipment
draw다. 기존 생성 실패 HRESULT와 rollback 소비자를 유지하며 별도 모델 런타임을 추가하지 않는다.
새 C++ 파일과 프로젝트/filter 등록은 필요 없다.

## G02. 링 및 캐릭터 원본 재질 연결

사용자가 의미한 안쪽 넓은 장식 링은 BG_ELG_ARYANORB_FLOOR13H_SM 439/482/486이다.
앞서 연결한 MAGICFLOOR03D 15개는 얇은 외곽 띠이며 이 세 배치를 포함하지 않았다.
실제 floor13b_01 MIC는 기존 PBR program4로 처리한다. 원본 D/N/detail/ORM/reflection,
atlas74의 UV1과 배치별 decode scale을 연결하고 482에만 확인된 environment override를 연결한다.
source geometry의 UV1 및 tangent handedness는 반복 추출에서 동일하다. COLOR0 absent의
프로젝트 기본 alpha1, 원본 미확정 hemisphere/BRDF LUT 차이는 완전 복원으로 기록하지 않는다.

Data/Maps/Imported의 CS catalog, Authoring의 CS placement/material과 MapCatalog의 CS count를
변경한다. 모델·DDS는 Resources/Map/CHARACTERSELECTMAP의 새 FLOOR13H variant 폴더에 둔다.
기존 다른 배치와 모든 Transform·카메라·조명 저작값을 유지한다. publisher로 런타임을 배포한다.

### source emissive 선택 입력

PBR material row에 optional `emissive`를 추가한다. 필드는 `texture`, `color` RGBA,
`intensity`, `uvTiling` XY, `colorSpace`, `flicker:{minimum,speed,phaseOffset}`다.
미지정 기존 재질은 발광을 추가하지 않는다. 원본 식은 다음과 같다.

```text
phase = phaseOffset + elapsedTime * speed
flicker = 0.5 * (1 + sin((phase + cos(phase * 3.524534)) * 1.328987)) + minimum
emission = Sample(E, UV0 * emissiveUVTiling).rgb * color.rgb * intensity * flicker
```

floor13b_01은 intensity10, minimum0, speed0.5, color(.534076333,1,.9258000851), UV(1,1)이다.
원본 engine-origin 상수 미확정 부분은 phaseOffset0의 프로젝트 근사로 구분한다.

| 파일 | 책임 |
|---|---|
| Engine/Public/BinaryAsset/ModelAssetData.h | immutable surface 입력의 hasEmissive와 색/강도/UV/위상/색공간, override/data의 surfaceEmissivePath |
| Engine/Private/Model.cpp | PBR 발광의 유한값·범위·Resources root 검사 후 staged material로 복사 |
| Engine/Private/Material.cpp, Engine/Public/Material.h | 선택된 발광 SRV 로드와 Bind_SurfaceTexture EMISSIVE 슬롯 |
| Client/Private/MapAssetCatalog.cpp | optional JSON exact fields, 입력 경로·범위 검증과 기존 parse/stage 계약 |
| Client/Private/MapAssetRenderUtils.cpp | 매 draw flag 초기화, 해당 material SRV와 원래 elapsedTime 바인딩 |
| Tools/MapPipeline/Publish-MapAuthoring.ps1 | 동일 optional 계약 검사 후 기존 transaction publish |
| Shader_MapMaterialSurface.hlsli, Shader_VtxMeshBinary.hlsl, Shader_VtxMeshMapInstance.hlsl | 원본 발광 식과 static/instance 두 경로의 MRT4 합산 |

새 C++ 파일은 없다. 공개 material 구조가 바뀌므로 Engine SDK와 Client까지 재빌드한다.
다른 작업의 Model.cpp root-motion 변경과 publisher 변경은 보존한다.

창술사 WModel의 eyelashes/eye/hair에 잘못 들어간 BRDF LUT path 3개만 제거한다.
geometry/skeleton/animation과 다른 material 값은 유지한다. 이는 LUT를 meshUV로 읽는
오류 수정이며, 특수 eye/hair lobe와 Mokoko 전체 원본 셰이더 복구 완료를 뜻하지 않는다.

## G03. Character Select 경계 필터 활성화

현재 global FXAA는 enabled=true이지만 subpixel=0이어서 실제 shader가 즉시 우회한다.
CS warm-high-key profile에 현재 global quality를 보존한 override를 추가하고 FXAA 강도만
0.75로 변경한다. 다른 맵의 profile과 노출/감마/조명은 바꾸지 않는다. 정식 temporal AA나
원본 UE3 후처리 복원은 아니며, 사용자에게 Rendering Workbench에서 비교할 값을 전달한다.

## 검증과 사용자 확인

실제 texture의 mip 수, mip0 보존, sRGB와 linear 평균, 작은/비정사각 texture를 확인한다.
변경 shader를 컴파일하고 기존 focused shader 검사를 재사용한다. C++는 필요한 최소 빌드를 하고
최종 Product 배포가 필요하면 Engine → Shared → Server → Client 순서로 진행한다.
다른 작업의 미커밋 변경은 보존하며 실행 중인 Client/Server를 강제 종료하지 않는다.

Client는 사용자가 새 빌드로 재시작한다. Lobby → Character Select에서 창술사를 선택하고
F6 free camera로 같은 표면을 가까이/멀리 비교한다. 다른 다섯 클래스도 몸·머리·무기를 확인한다.
현재 조명값을 유지한 상태에서 비교하며 에이전트는 Client 조작·캡처나 visual PASS 판정을 하지 않는다.
실행한 자동 검증, 실제 배포 파일, 남은 원본 재질 범위는 대응 RESULT에 기록한다.
