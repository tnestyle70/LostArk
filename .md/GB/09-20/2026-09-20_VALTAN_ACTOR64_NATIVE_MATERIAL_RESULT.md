# 발탄 입장 actor64 원본 재질 복구 RESULT

## 반영 범위

입장 Matinee 53의 actor64 본체·무기는 메시 기본 ghost 재질이나 LookInfo의 일반 `-1_mi` 대신, 실제 Scene component가 지정한 `mn_tslc_00-1.mat.mn_tslc_00-1_mi_dead`와 `wp_mn_tslc_00-1.mat.wp_mn_tslc_00-1_mi_dead`를 소비한다. Scene component 815/816의 override가 정본이며, group133/track309의 단일 `dead=1` 키(0.209898초)를 두 재질의 상수로 반영했다. 시간에 따른 추가 재질 전환은 만들지 않았다.

`Data/Actors/BossCatalog.json`의 root `modelMaterialOverrides`에 다음 두 행을 추가했다. 최신 저장본의 기존 69행과 다른 모든 필드를 보존하고 백업·hash 확인·원자 교체했다.

| 모델 | 쿠킹된 슬롯 이름 | 프로그램 |
|---|---|---|
| `Map/Valtan/Cinematics/Actor64/Body/MN_TSLC_00-1.wmodel` | `mn_tslc_00_mi` | 92 |
| `Map/Valtan/Cinematics/Actor64/Weapon/WP_MN_TSLC_00-1.wmodel` | `wp_mn_tslc_00_mi` | 93 |

본체의 같은 이름인 두 material slot은 기존 `CModel`의 동일 이름 전체 교체 계약을 사용한다. 모델·원본 골격·24.708초 Matinee bake·World Sequence 연결은 통합 담당의 별도 변경이며, 이 결과의 소유 범위는 재질이다.

## 원본 연산과 런타임 연결

원본 MIC의 effective parameters, static shader-map join, GPU-skin Base/Directional Light DXBC 및 texture expression을 실제 retail RefShaderCache에서 추출했다. 기존 SourceCharacter 전체 함수와 주석·함수번호를 제외하고 비교했지만 같은 프로그램이 없어 두 프로그램을 추가했다.

| 대상 | Base 원본 shader ID / instruction | Light 원본 shader ID / instruction |
|---|---|---|
| 본체 | `16902fea865cdf40ae574a752c0d427b` / 189 | `d3ba25a33251444ebadc4fd36f06cf45` / 181 |
| 무기 | `e15390cf9645c648bc3b54f8d1dfc178` / 165 | `0bf9e24a749ec345badf51ebb62cf268` / 167 |

`SourceCharacterMaterialParameters.h`에 원본 uniform packing을 추가하고, Engine/Client의 Base/Light group084와 dispatcher에 원본 함수를 추가했다. `Engine/Private/Model.cpp`, `Shader.cpp`, `native_shader_dispatch.py`의 프로그램 범위를 84~93으로 함께 확장했다. 기존 group084 wrapper를 사용하므로 새 shader 파일·프로젝트 항목은 없다.

두 Light PS의 실제 입력은 `v2=UV`, `v3=tangent light`, `v5=tangent view`, `v6=source position`이다. `Shader_SourceCharacterMaterial.hlsli`의 기존 legacy Light adapter에 92/93만 연결했다. Base 입력은 기존 common ABI와 일치한다. 기존 native 함수 본문은 모두 보존했다.

원본 DDS 10개를 `Resources/Map/Valtan/Cinematics/Actor64/SourceMaterials`에 설치했다. 본체는 normal/emissive/diffuse/reflection/specular/state/dead texture 7개, 무기는 normal/diffuse/state/specular/dead texture 5개를 사용하며 공통 두 texture를 공유한다. normal만 linear, 나머지는 원본 sRGB 속성을 유지한다. 원본 `fx_a_ice_003`, `ambientreflection_01`을 포함하며 유사한 그림으로 대체하지 않았다.

## 검증

- 두 native 프로그램의 Base/Light 생성 함수와 설치 함수, CPU parameter packing의 exact verify PASS.
- 변경된 `Model.cpp`, `Shader.cpp`, `ActorCatalog.cpp` 3 TU 최소 컴파일 PASS.
- 실제 anim/static/deferred SourceGroup084 세 carrier `fxc /T fx_5_0` PASS. native expression의 기존 유형인 X4000 경고는 로그에 남아 있다.
- 최신 `CActorCatalog`와 `CMapAssetCatalog`를 함께 빌드한 실제 catalog CPU 소비자: 두 모델의 정확한 슬롯 이름, 프로그램 92/93, texture mask, `dead=1`, 128개 packed constant의 각 lane, source texture 경로·색 공간 총 1,064 checks PASS.
- 기존 native 함수 본문, Engine/Client shader mirror, 원본 DDS SHA, 최신 BossCatalog의 무관한 필드 보존 검사 PASS.
- 변경 JSON parse 및 `git diff --check` PASS.

증거·원본 dump·후보·백업·검사 로그: `out/ValtanRestoration20260920/Actor64Native/`. 원본 texture/MIC export: `out/ValtanRestoration20260920/SourceActor64Dead/`.

## 남은 경계

이 검증은 실제 catalog 소비 및 shader 컴파일까지다. Client를 실행하지 않았으며, 전체 입장 컷신에서 배우의 최종 색·빙결 표현·조명과 모델 부착은 사용자 화면 확인 대상이다. Product 통합 빌드와 runtime 데이터 배포는 통합 담당이 수행한다. 원본 연산의 새 해석이나 화면 일치가 검증됐다고 기록하지 않는다.
