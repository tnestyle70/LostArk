# Bern 원본 식생 wind 구현 계획

## G00. 원본 분모와 구현 경계

현 Bern foliage/grass 7,328 재질 행은 unique MIC 127개다. 설치 원본 32패키지의 부모 체인과
native static set을 해석했고 설치 RefShaderCache에서 127개 모두 engine-equality key로
정확히 하나의 material shader map을 선택했다. LocalVF NoLightmap/NoDensity VS는
E4FE 114MIC/6,216행, grass1C39 10MIC/1,047행, 별도098C 3MIC/65행이다.
E4FE 중7MIC/1,427행은 wind/noise amplitude가 둘 다0이다. 이번 시간 바람 복원 대상은
nonzero120MIC/5,901행이며 원본0 설정을 바꾸지 않는다.

Bern streaming35패키지의 실제 WindDirectionalSource는 PS에1개 존재한다. 원본 Strength2,
Rotator(-10856,84404,-103716)이며 생략된 Speed는 Engine.u의 해당 CDO에서1로 확인했다.
기존 no-wind fallback을 이 맵에 넣지 않고 native scene proxy와 shader binder 소비를 추적한다.
원본 active WindDirectionalSource가 없는 다른 Area의 입력은 별도 source census/CDO 근거를 따른다.

## G01. 공용 VS와 입력 계약

`Client/Bin/ShaderFiles/Shader_SourceFoliageWind.hlsli`에 원본1C39/098C의 displacement
명령 구간을 기존 literal DXBC translator로 옮긴다. 원본 shader ID와 instruction SHA를
고정하며 E4FE 기존 명령을 보존한다. native register 배치, cm→m, UE→Client 축 변환,
원본 vertex color swizzle, local bounds·owner position·time·player position을 실제 소비한다.
별도 shader ID를 기존 A1C6에 alias하지 않는다.

`Tools/LevelPlacementExtractor/source_foliage_wind.py`의 exact program과 source scene wind
검증을 확장한다. 현 catalog/publisher/model 입력 검증도 같은 계약을 소비하도록 맞춘다.
원본 Strength/Speed/Direction과 scalar/vector uniform을 섞거나 임의 값으로 대체하지 않는다.
공용 C++ binder 변경은 다른 세션의 terrain·map picking 변경을 보존하고 필요한 필드만 반영한다.

## G02. 원본 배치와 geometry 입력

current assetId를 source placement/component/instance와 원본 StaticMesh에 연결한다.
원본 native FBoxSphereBounds와 실제 설치 WModel의 preScale·COLOR0·변환을 대조한다.
원본 scene proxy/LocalVF binder가 소비하는 owner position과 bounds를 기준으로 계산한다.
instanced foliage의 원본 VF가 LocalVF와 다른 입력을 쓸 때 같은 owner를 추정하지 않는다.
확보되지 않은 owner는 source 근거를 더 찾고 후보에 미확정 상태를 숨기지 않는다.

## G03. 후보, 병합과 검증

Bern mapmaterials는 stable assetId/materialName/sourceMaterial로 wind 필드만 추가하는
후보를 준비한다. 설치 직전 최신 디스크 SHA를 다시 읽고 무관한 terrain/RNM/shadow/material
수정을 보존한다. 백업과 atomic replacement를 사용하고 원본0-amplitude1,427행은 보존한다.
현재 사용자 수동 OFF인 Bloom/Fog/FXAA와 모든 rendering option은 변경하지 않는다.

원본3개 DXBC와 공용 HLSL 수치를 실제 설치 모델의 COLOR0/정점·시간·TRS에서 비교한다.
변위 비율과 nonuniform/mirrored scale, zero-amplitude, finite 결과 및 source wind owner 소비를
작은 비UI 검사에서 확인한다. 관련 shader focused compile과 변경 C++ 최소 컴파일을 실행한다.
최종 Debug/Release Product Build는 root가 모든 변경을 합친 뒤 한 번씩 실행한다.
Client/UI·원작 EXE 실행 및 화면 자동 판정은 하지 않는다. 구현·자동 검증·게시·사용자 화면
확인을 RESULT에 분리하고 source VF/owner의 남은 경계를 구체적으로 적는다.
