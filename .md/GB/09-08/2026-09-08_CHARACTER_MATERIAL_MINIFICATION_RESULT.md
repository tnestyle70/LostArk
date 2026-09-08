# Character Select 링·캐릭터 축소 품질 수정 결과

## 현재 상태

2026-09-08 후속 확인: 사용자가 Character Select에 직접 입장해 중앙 링이 금색 재질로
표현되는 것을 확인했다고 보고했다. 이는 중앙 링의 금색 변화에 대한 사용자 확인이며,
원작 전체 일치·모든 캐릭터 재질·성능 또는 발광의 별도 자동 검사 통과를 뜻하지 않는다.
현재 runtime mapmaterials는 실제 9행/placementLighting24행이며 FLOOR13H의 새 재질
2행과 3배치가 연결되어 있다. 아래의 runtime7행/publish 대기 문장은 작업 중단 당시 기록이다.
후속 배포·빌드를 이 조사에서 직접 수행한 것으로 기록하지 않는다.

## 빌드 대기 요청을 받은 당시 상태

2026-09-08 사용자가 다른 작업의 마무리를 우선하고 빌드를 기다리도록 요청했다.
이 작업은 추가 제품 수정·빌드·publish를 중단했다. 하위 agent 3개도 interrupted 상태다.
확인 시 Client 37804와 Server 10020은 계속 실행 중이고, cl/link/MSBuild/fxc/python 및
floor_warp_probe 프로세스는 없었다. 실행 파일을 종료하거나 교체하지 않았다.
대응 구현 계획은 `2026-09-08_CHARACTER_MATERIAL_MINIFICATION_IMPLEMENTATION_PLAN.md`다.

## 반영한 코드·리소스

- 캐릭터 45개 실제 body/equipment/weapon 모델의 고유 texture 259개를 조사했다.
  247개 TGA에는 CMaterial의 CPU full mip 생성과 immutable subresource 업로드를 구현했다.
  diffuse/emissive는 sRGB를 풀어 linear 평균하고 alpha/normal/mask는 linear 데이터로 처리한다.
  렌더링 immediate context를 Loader worker에서 호출하지 않는다.
- 차원술사 DDS 12개는 동일 경로에 11단계 mip을 설치했다. 원본 압축 mip0와 decode 픽셀은
  보존했고 하위 mip만 생성했다. 기존 파일 7,341,568바이트가 9,788,432바이트가 됐다.
- 실제 skinned shader와 static 몸체·장비 표면 샘플에 Aniso16을 연결했다.
- 창술사 `Character/LanceMaster/LanceMaster.wmodel`의 eyelashes/eye/hair에 잘못 들어간
  `brdf_beckmann_spec.tga` 경로 3개를 제거했다. 전체 113,764,868바이트 중 93바이트만
  변경했고 geometry/skeleton/223 animations 및 다른 material 값은 그대로다.
- 사용자 화면의 안쪽 넓은 갈색 링은 기존 MAGICFLOOR03D 15배치와 다른
  FLOOR13H 439/482/486이었다. 이 세 배치에 원본 floor13b_01의 PBR program4 입력을 연결했다.
  D/N/detail/ORM/reflection, atlas74와 native UV1/tangent 부호, 선택 발광을 준비했다.
  environment override는 근거가 있는 482에만 연결했다. 모든 배치 Transform은 유지했다.
- PBR row의 optional `emissive` 입력을 Model/Material/Map catalog/publisher와 static/instance
  shader 모두에 연결했다. 원본 nested cosine/sine 식과 기존 elapsedTime을 사용하고
  매 draw 활성 flag를 초기화한다. 이 변경은 C++ 공개 구조를 포함하므로 SDK와 Client 재빌드가 필요하다.
- Character Select warm-high-key profile만 현재 품질값을 보존한 override를 추가하고,
  FXAA 강도를 0에서 0.75로 바꿨다. 현재 구현에서 0은 FXAA 전체 우회다.
  노출·감마·조명과 다른 map profile 값은 유지했다. RenderingProfiles publish는 완료했다.

## 검증한 내용

| 구분 | 실제 확인 |
|---|---|
| TGA mip | 실제 18개와 합성 1×1/3×5/1×7 총 21개 loader/SRV/readback 확인. mip0 보존, sRGB/linear, alpha 독립 평균과 홀수 가장자리 기여 확인 |
| DDS mip | 설치한 12개 모두 실제 DirectXTK/WARP SRV 11mip 및 mip0 압축/decoded RGBA 보존 확인 |
| 창술사 모델 | 수정 허용 3필드 밖 변경 0, 기존 decoder와 ModelAssetConverter info exit0 |
| 새 링 geometry | 1120정점/3072인덱스 원래 순서와 P/N/Txyz/UV0 보존, UV1 및 원본 tangent 부호 확인 |
| shader compile | skinned/static/instance 3개 fxc 성공. 이후 다른 작업이 공유 sampler 코드를 수정 중이므로 최종 통합 컴파일을 대신하지 않음 |
| CS authoring | catalog63/material9/placementLighting24, 803배치 Area Validate 성공 |
| 문서/공백 | 중단 시 CS authoring/runtime material, rendering authoring/runtime JSON parse 성공. 관련 파일 git diff --check 오류 없음 |
| 제품 C++/실행 | 이번 변경의 Product 빌드 미실행. Client/UI 조작·캡처·육안 판정 미실행 |

새 발광을 포함한 기존 WARP 수치 검사 확장은 중단 시 완료 보고를 받지 못했다.
이전 program5의 198 PASS를 이번 새 링/발광의 PASS로 재사용하지 않는다.

## 중단 당시 배포·검증하지 않은 부분

중단 당시 CS map authoring은 수정했지만 **이 작업에서 map publish는 하지 않았다**. 당시 runtime
mapmaterials는 기존 7행이며 새 authoring은 9행이다. 따라서 새 링은 EXE 빌드만으로 연결되지 않는다.
재개 시 기존 publisher로 CS를 publish하고, 공유 sampler 변경이 끝난 코드로 Product 빌드와
새 발광의 focused 검증을 수행한 뒤 사용자가 Character Select에서 확인해야 한다.

원본과 완전히 같은 피부·눈·머리의 특수 반사, Mokoko PBR shader 전체, 모든 캐릭터 재질의
원본 식을 구현한 상태는 아니다. 기존 `_s` 전체를 RGB specular나 ORM으로 임의 해석하지 않았다.
눈·머리의 잘못된 LUT 경로만 제거했으며 기존 constant specular 동작은 남아 있다.
링의 COLOR0 absent 기본 alpha1, 생성 mip, 프로젝트 BRDF LUT와 원본 engine-origin/hemisphere
미확정 항목도 원본 완전 일치로 기록하지 않는다. 이후 사용자가 금색 변화를 확인한 범위는
맨 위 현재 상태에 추가했으며, 전체 원작 유사도 판정을 확대하지 않는다.

## 다른 작업과의 조율

공유 작업 `01a07eb5-aef5-77e0-918c-30a9e2981a46`에 중단과 파일 소유권 해제,
CS publish 미완료, 최종 빌드/검증 필요를 전달했다. 그 작업의 포커판 Mirror U 수정과
기존 root-motion/게임플레이 변경을 보존한다. 이 작업은 commit/stage/push/merge하지 않았다.

리소스 변경은 로컬 Resources에 설치돼 있으며 Drive 업로드나 ZIP 생성은 하지 않았다.
새 링은 `Resources/Map/CHARACTERSELECTMAP/MAP_D6DB636BED17_BG_ELG_ARYANORB_FLOOR13H_SM_OVR_E42AC6730343/`
의 WModel+6DDS이며 기존 lighting을 재사용한다. 창술사 WModel과 차원술사 DDS 12개도 공유 시
갱신 대상이다. 구체적인 설치·이전 파일은 다음 기록에 있다.

- `out/CharacterSelectFullRestore20260908/floor13h_connection.json`
- `out/CharacterMinification20260908/dds_mip_stage.json` 및 `dds_before_install/`
- `out/CharacterMaterialReview20260908/lookup_path_correction.json`
- `out/CharacterRenderingRestore20260908/before/`와 `shaders/`

## 후속 조사에서 재확인한 공유 경로

- 링을 포함한 CS 전체: `Resources/Map/CHARACTERSELECTMAP/`
- 새 링만: `Resources/Map/CHARACTERSELECTMAP/MAP_D6DB636BED17_BG_ELG_ARYANORB_FLOOR13H_SM_OVR_E42AC6730343/` — WModel1개/DDS6개 존재 확인.
- 창술사 교체 파일: `Resources/Character/LanceMaster/LanceMaster.wmodel`
- 차원술사 교체 폴더: `Resources/Character/WP_WSWP_M_06/textures/` — `wp_wswp_m_06{l,s,e}_{d,n,e,orm}.dds` 12개 모두 mip11 헤더 확인.

이번 후속 조사는 위 파일을 읽기만 했고 새 Resources 설치·ZIP·Drive 업로드는 하지 않았다.
사용자 요청에 따라 이후 추가/교체 때마다 정확한 Resources 상대 경로와 Drive 공유 필요 여부를
해당 반영 보고에 함께 적는다.
