# 쿠크 전체 맵 원본 조명·geometry 연결 계획

## G00. 원본과 현재 소비 경로

사용자가 기존 두 바닥 재질을 완벽하다고 확인한 값을 보존한다. 원본 PS/SL01~05는 3,081 StaticMeshComponent, LightMap2D 2,645개, LightMapTexture2D 640개와 광원 116개를 보유한다. 원본 GUID가 RNM에 포함된 84 Point/Spot와 독립 직접광 32개를 구분한다. `CModel -> CMaterial`과 기존 `placementLighting`이 유일한 렌더 소비 경로다.

## G01. geometry·배치 입력

원본 native triangle corner를 기존 glTF의 위치·normal·tangent·UV0와 대조한 뒤 UV1·native tangent W·COLOR0을 기존 WModel geometry 계약으로 보존한다. 원본 component COLOR override는 해당 배치 전용 variant로 분리한다. atlas가 다른 배치는 별도 catalog asset variant를 사용하고, UV scale/bias와 RGB coefficient는 stable sourcePlacementId로 연결한다. 저작 `mapplacements`의 transform·visibility·runtime placement ID는 변경하지 않는다.

원본 LightMapTexture2D의 DDS를 Resources `Map/Lighting/KoukuSaydon/`에 설치한다. 기존 판독기와 같은 linear interpretation을 사용하며 생략된 원본 SRGB class default까지 확정했다고 표현하지 않는다. Native LightMap1D와 임의로 이동한 EDITOR 배치는 RNM 입력으로 위장하지 않는다.

## G02. 광원의 수광 대상

`Engine/Public/Engine_Struct.h`의 LIGHT_DESC에 ALL(기존 기본값)/SOURCE_CHARACTER typed receiver를 추가한다. `MapLightDocument`의 optional receiver는 strict parse·serialize를 거쳐 `MapLightPresentationRuntime`, `CPresentation_Manager`, `CLight`와 실제 Deferred shader까지 전달한다. RNM에 포함된 원본 84 광원은 SOURCE_CHARACTER로 연결하여 컷신·실제 보스·원본 캐릭터 재질 공에 직접광을 주고 구운 배경에는 중복 합산하지 않는다.

Authoring 광원 상한은 256개로 확장하고 기존 화면 frustum과 광원 sphere의 교차 여부로 active submission을 줄인다. 292개 저작 camera/volume sample의 보수 frustum에서 최대 101개가 검출되어 transient renderer 상한을 128개, map budget을 120개로 연결하고 effect 여유 8개를 유지한다. SOURCE_CHARACTER 광원은 일반 조명 pass에서 CPU draw도 생략한다. capacity와 culling은 runtime 상태에 구분해 표시한다. 원본 독립 방향광은 현재 scene 방향광과 중복 생성하지 않는다.

## G03. 확인

geometry의 native 대응·WModel payload·material section 보존, DDS dimension·실제 원본 파일 대응, atlas/배치 ID join, JSON parse와 Area publisher, 수정 C++ 최소 컴파일 및 실제 shader 컴파일을 확인한다. map light parse → serialize → reparse 및 잘못된 receiver 실패 시 기존 문서 보존을 확인한다. Product 전체 빌드는 root가 수행한다. Client/UI 실행·캡처·최종 시각 판정은 사용자가 수행한다.

## G04. 원본 fog·공간 환경

PS의 ExponentialHeightFogComponent, WorldInfo, EFGame/Engine의 원본 class CDO를 읽어 11개 EF 환경 volume의 생략된 기본값까지 resolve한다. 원본 BSP brush의 여섯 평면과 AABB가 실제 camera 공간 선택에 사용된다. EFEnvironmentInfoVolume CDO의 BlendTimeIn/Out 1초와 instance override를 fog와 두 light color의 시간 보간에 사용한다. 선형 보간 곡선은 프로젝트 연결이며 원본 easing까지 동일하다고 주장하지 않는다.

기존 HEIGHT_FOG_SETTINGS에 optional sourceExponential 모델을 연결한다. `Shader_SceneHeightFog.hlsli`의 동일한 fog transfer를 Deferred combine과 source map forward PS의 원본 v5 입력이 함께 소비한다. 원본 TExponentialHeightFogPixelShader의 전체 ray 적분, exp2 투과도, 최소 수직 ray와 hemisphere pow 식을 유지한다. Native CPU 상수 packing의 density/falloff 단위 및 terminator exponent는 source shader PS만으로 확정할 수 없는 adapter 경계로 기록한다.

RenderingProfiles의 strict parser, float32 Save roundtrip, region·plane·색·시간 검증과 publisher를 함께 확장한다. 원본 클래스의 값이 생략됐다는 이유로 PS 전역 fog를 지역 CDO 대신 상속하지 않는다. `CModel -> CMaterial`과 기존 renderer 외에 별도 맵 런타임을 만들지 않는다.
