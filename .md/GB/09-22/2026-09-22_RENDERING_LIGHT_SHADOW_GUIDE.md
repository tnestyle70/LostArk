# 현재 렌더링의 빛·그림자·색감 읽는 법

## G00. 첨부 화면에서 서로 다른 문제

왼쪽 구현은 따뜻한 바닥색이 강하고, 오른쪽 원작은 바닥 굴곡과 접촉부의 명암이 더 분명하다. 화면만으로 노멀맵 누락을 단정할 수는 없다. 현재 Character Select mapmaterials에는 normal/detail normal과 RNM lightmap 연결이 이미 있다. 연결의 정확성·누락 slot은 별도 재질 복원 세션의 범위이고, 여기서는 빛을 합성하는 소비 경로를 다룬다.

바닥 홈의 작은 명암은 재질 normal과 baked lighting/AO, 발에서 옆으로 길게 뻗는 그림자는 물체를 광원 시점에서 그린 shadow map이 주로 만든다. 두 현상은 함께 흐릴 수 있지만 같은 데이터는 아니다. normal map은 조명에 쓰는 방향을 바꾸며 실제 geometry나 그림자 윤곽을 생성하지 않는다. 화면 오른쪽 원작의 왼쪽 그림자 방향만으로 특정 전역광·GI 알고리즘을 확정할 수 없다.

## G01. 기존 Deferred 계산과 흐린 그림자

정본은 `Engine/Bin/ShaderFiles/Shader_Deferred.hlsl`, `Engine/Private/Renderer.cpp`, `Engine/Private/Shadow.cpp`다. 설명용으로 RGB 곱셈을 생략하면 기존 계산은 다음 구조다.

```text
색 = 재질색 × (직접광 × 그림자 + ambient × SSAO)
   + 직접 반사광 × 그림자
   + 구운 조명(RNM) + 환경 반사(IBL) + 실제 발광
```

원본 계열별 직접광은 별도 식을 사용하지만, 구운 조명과 실제 발광을 같은 Emissive 버퍼에 담아 마지막에 더하던 구조가 핵심이다. SSAO는 ambient만 어둡게 하고 밝은 RNM/IBL에는 영향을 주지 않았다. Shadow Strength를 올려도 point light·구운 조명·환경 반사가 그대로 남는 이유다. 전체 Emissive를 그림자로 곱하면 불꽃과 빛나는 표면까지 꺼지므로 그런 교정은 하지 않는다.

직접광의 그림자는 2048² 깊이 texture의 3×3 비교다.

```text
PCF = 가려지지 않은 표본 수 / 9
그림자 계수 = 1 - strength × (1 - PCF)
```

기존 Character Select strength 0.75에서는 완전히 가려져도 직접광의 25%가 남는다. 24m 폭을 2048칸으로 나눠 한 texel은 약 1.17cm, 세 칸의 표본 범위는 약 3.52cm다. depthBias 0.0015는 단순 0.0015m가 아니다. near 0.1m, far 80m에서 약 11.985cm의 광원 깊이 오프셋이며 normalBias 0.025m까지 있었다. 너무 큰 bias는 발과 그림자를 떼어 놓는다. 너무 작으면 표면이 자기 자신을 가려 점무늬가 생긴다.

## G02. 이번 공용 렌더링 변경

PBR map pixel(marker3)의 기존 빈 geometry RGB에 RNM/IBL을 따로 저장한다. 실제 emissive와 캐릭터 material row는 원래 경로를 유지한다. 합성 단계에서 PBR 간접광에 SSAO를 적용하고, 간접광도 fog 이전의 빛으로 합친다.

optional `shadow.dynamicBakedStrength`는 움직이는 캐릭터·Effect 모델이 구운 바닥에 만드는 그림자를 보강하는 **프로젝트 근사**다. 원본 GI나 UE3 light environment 전체를 복구했다는 의미가 아니다. 기본값0이며 기존 profile의 미기록 동작을 보존한다. static-only 캐시 깊이와 캐릭터까지 그린 최종 깊이를 비교해 새로 가까워진 caster만 감쇠한다. 따라서 이미 RNM에 구워진 정적 그림자를 다시 어둡게 하지 않는다. 캐시가 유효하지 않으면 해당 보정만 생략한다. 새로운 geometry pass는 추가하지 않는다.

Character Select 렌더링 후보는 strength1, depthBias0.00025(약2cm), normalBias0.005m, dynamicBakedStrength0.7이다. SSAO 반경은 기존4cm에서0.7m로 옮기고 bias0.025m, intensity1, power1.4, FXAA subpixel0.25를 사용한다. 이 값은 현재 renderer를 위한 시작점이며 원작 시각 일치로 인증한 preset이 아니다. 원본 postprocess에70cm 반경이 있지만 현재 SSAO의 표본 방식과 같지 않다. 최종 설치·빌드·사용자 화면 확인 상태는 대응 RESULT를 따른다.

## G03. Rendering Workbench 조절 순서

| 입력 | 실제로 바꾸는 것 | 이 입력으로 고칠 수 없는 것 |
|---|---|---|
| 재질 normal/roughness/AO | 표면 방향, 반사 폭, 재질 자체 차폐 | 긴 cast shadow, 누락 mesh |
| Light Direction | 직접광 방향과 현재 shadow camera 방향 | 재질 texture 누락 |
| Diffuse RGB·강도 | 광원이 표면에 주는 빛 | 잘못된 lightmap UV |
| Ambient | 넓고 균일한 보조광 | 원본 GI의 공간별 차이 |
| Shadow Strength·Bias | 직접광 차폐 농도·자기 그림자 오차 | 전체 색온도 |
| Dynamic Shadow on Baked PBR | 움직이는 caster의 RNM/IBL 감쇠 | 원본 간접광의 재계산 |
| SSAO Radius | 화면 깊이에서 조사하는 주변 거리(m) | 화면 밖 물체의 차폐 |
| SSAO Intensity·Power | 근접 굴곡 차폐의 강도·대비 | 긴 방향성 그림자 |
| Exposure | 모든 HDR 빛의 배율 | 노란 원본 tint·잘못된 재질 |
| Tone curve | 밝기 범위를 화면에 압축 | 없는 조명 정보 |
| Bloom | 밝은 pixel을 주변으로 퍼뜨림 | GI·normal·그림자 |
| FXAA | 최종 이미지의 계단 경계를 혼합 | texture mip·UV·재질 복원 |

먼저 Normal/Albedo/Roughness 진단으로 재질 입력을 확인한다. Bloom을 낮추고 FXAA 혼합을 낮춘 상태에서 직접광 방향과 shadow bias를 맞춘다. 그 다음 indirect와 SSAO의 접촉 명암을 확인하고 마지막에 exposure/tone/Bloom을 조절한다. 한 단계마다 기준 화면과 비교한다. 현재 Source Tone이 켜져 있으면 Hable White Point는 계산에서 사용되지 않아 UI에서도 비활성화한다. gamma를 밝기 보정으로 반복 조절하면 색 판단이 흔들린다.

노란 느낌을 없애기 위해 전체 RGB를 파랗게 곱하는 것은 여러 원인을 한 값으로 덮는 조정이다. 바닥 MIC의 tint, sRGB 해석, 원본/추가 point light, lightmap decode, 환경 반사, grading을 순서대로 대조해야 한다. 재질 복원 세션과 렌더링 세션이 같은 profile을 동시에 저장하지 않도록 최신 저장본의 변경 필드를 병합한다.

## G04. GI·Nanite와 현재 엔진의 관계

GI는 표면에서 반사된 빛이 다른 표면을 밝히는 간접광이다. 현재 엔진은 추출된 RNM lightmap과 cube/BRDF 입력, 별도 ambient를 사용하며 동적 다중 반사 GI는 구현하지 않았다. SSAO는 주변 깊이로 차폐를 추정하는 보조 계산이고 GI를 대신하지 않는다. [Epic GI 설명](https://dev.epicgames.com/documentation/unreal-engine/global-illumination-in-unreal-engine)

Lumen은 동적 간접광과 반사를 계산하는 UE5 시스템이다. distance field 또는 hardware ray tracing 등의 별도 장면 표현·추적·cache가 필요하다. 현재 DX11 renderer의 설정값 하나로 켤 수 없다. 지금 확보한 RNM을 제대로 연결하고 움직이는 caster의 차폐를 처리하는 작업과 Lumen 도입은 규모가 다르다. [Epic Lumen 문서](https://dev.epicgames.com/documentation/en-us/unreal-engine/lumen-global-illumination-and-reflections-in-unreal-engine)

Nanite는 매우 많은 삼각형을 화면 크기에 맞춰 처리하는 가상화 geometry 시스템이다. 재질 mapping, normal texture, shadow bias, 색온도를 자동 복원하는 기능이 아니다. 현재 비교에서 먼저 확인할 것은 source geometry/normal/lightmap과 합성 누락이다. [Epic Nanite 문서](https://dev.epicgames.com/documentation/en-us/unreal-engine/nanite-virtualized-geometry-in-unreal-engine)

향후 넓은 맵의 그림자 해상도에는 cascade shadow map, 작은 접촉부에는 화면 공간 contact shadow나 더 나은 AO, 움직이는 물체의 간접광에는 probe/SH 계열을 별도 검토할 수 있다. 각각 메모리·표본 비용·화면 밖 누락·시간 누적 잔상 등의 대가가 있다. 이번 변경에는 이들을 도입하지 않으며 기존 shadow/RNM/IBL 경로의 누락을 먼저 교정한다.
