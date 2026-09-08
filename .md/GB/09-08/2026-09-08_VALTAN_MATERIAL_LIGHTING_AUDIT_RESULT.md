# 발탄 원본 재질·구운 조명 전수조사와 세 캐릭터 반사 연결 상태

> 이 문서는 구현 전 조사 시점의 기록이다. 이후 중앙 바닥·바위 7배치 구현은
> `2026-09-08_VALTAN_ARENA_STONE_RESTORATION_RESULT.md`에서 추적한다. 추가 해독으로
> 원본 component의 비백색 정점색과 정확한 UV1·tangent W를 확보했다. 앞선 설명의
> Base/Direct overlay weight 차이는 레지스터 덮어쓰기 재검산으로 정정했다. 세 pass의
> weight는 같고 mixed normal의 정규화 및 specular normal 선택이 다르다.

## G00. 조사 상태와 범위

2026-09-08, 사용자가 발탄 원본 재질·baked light·BRDF 자료의 전수조사를 요청한 결과다.
현재 branch는 `codex/kouku-ball-motion-effects`, HEAD는
`f92178f054a759c128a61cebd8e2ff4b8712eb12`이다. 대규모 다른 작업의 미커밋 변경을
포함한 현재 파일을 읽었다. `git fetch`만 수행했고 stage/commit/merge는 하지 않았다.

이번 작업은 조사다. **제품 C++·셰이더·Data·Resources를 변경하거나 빌드하지 않았다.**
사용자가 다른 이펙트 확인용 빌드를 시작한다고 했으므로 공유 빌드 파일을 수정하지 않았다.
생성물은 이 RESULT, 공통 복원 가이드의 확인사항과 `out/ValtanMaterialLightingAudit20260908/`
안의 조사 코드·JSON·원본 셰이더·단일 원본 geometry 추출이다. Drive로 공유할 새 runtime
리소스는 없다. 아래 복구 순서는 아직 구현하지 않았다.

전수의 분모는 다음과 같이 구분한다.

- 현재 발탄 Level이 입장 시 준비하는 맵·Deploy·파편의 전체 모델과 사용 재질 슬롯.
- 해당 정적 맵과 대응하는 원본 PS/SL00~SL05 7패키지의 모든 StaticMeshComponent,
  lightmap texture, light component와 환경·후처리 자료.
- PS의 streaming level 19참조를 따라 추가 13패키지까지 읽은 export/관련 연출 목록.
- 주 정적 맵의 현재 WMAT 기반 원본 재질 후보와 원본 component override의 합집합.
- 차원술사·도화가·워로드의 현재 body/equipment/weapon 전체 모델과 실제 사용 슬롯.

보스/NPC 전체, 모든 파괴 파편의 원본 shader permutation, 원작 실시간 phase 활성 상태나
모든 GPU draw를 전수 검증한 것은 아니다. 원본 package에 존재한다는 사실과 실제 게임의
특정 프레임에 활성화됐다는 사실을 구분한다. Client 실행·조작·캡처·육안 PASS는 하지 않았다.

## G01. 차원술사·도화가·워로드가 현재 소비하는 반사

| 대상 | 모델 | 실제 사용 슬롯 | 원본 프로그램 연결 | 확인된 남은 입력/계산 |
|---|---:|---:|---:|---|
| 차원술사 | 5 | 18 | 18 | real-PBR program3/8/9의 13슬롯에서 장면 cube sample이 0, source SH 미연결 |
| 도화가 | 7 | 19 | 0 | 일반 셰이더의 scalar specular, 원본 마스크·피부 구분·재질별 반사/색 계산 미연결 |
| 워로드 | 9 | 14 | 0 | 일반 셰이더의 scalar specular, 원본 색/피부 마스크·상속 IBL·재질별 계산 미연결 |

차원술사의 override 정의는 17개지만 동일 MIC를 쓰는 별도 무기 슬롯도 포함해 18개 사용
슬롯이 매칭된다. 이것은 프로그램 선택 범위이며 환경 입력까지 복원 완료했다는 뜻이 아니다.
도화가 D/N/S는 19/18/16, 워로드는 14/14/13슬롯에 존재한다. 조사한 모든 물리 참조는
존재했다. 노멀이나 반사 텍스처가 전부 없어서 어둡다고 설명하면 틀리다.

`Part_Body/Part_Equipment -> Bind_DeferredMaterialInputs -> CMaterial` 경로가 실제 소비자다.
legacy animated/static shader는 S.rgb를 `dot(S.rgb, (.299,.587,.114))`로 줄이고 공통
specular exponent(기본50)를 쓴다. 직접광 하이라이트는 존재하지만 원본 RGB 반사색과
재질별 채널 해석은 보존하지 않는다. S.a를 무조건 roughness로 바꾸는 것도 원본 식 확인
전에는 금지한다.

차원술사 cube 미연결은 `Engine/Bin/ShaderFiles/Shader_SourceCharacterPrograms.hlsli`
6364/8976/9790행 주변의 원본 sample 주석과 다음 `float4(0)`에서 확인했다. 도화가·워로드는
기존 `family_scope.json`의 source identity와 대표 원본 props를 다시 대조했다. classic 계열의
material-owned 2D IBL과 real-PBR의 scene-owned cube/SH는 같은 입력이 아니다.

**어두움에 기여할 구체적인 공백은 확인했지만 기여 비율은 측정하지 않았다.** 기존 맵의
lightmap이나 CS 전용 환경 큐브가 캐릭터에 자동 적용되지 않는다. 색·roughness·마스크,
직접광, 간접광, 표시 노출을 나눠 비교해야 한다.

근거: [캐릭터 상세 조사](../../../out/ValtanMaterialLightingAudit20260908/character_findings.md),
[51슬롯 목록](../../../out/ValtanMaterialLightingAudit20260908/character_inventory.json).

## G02. 실제 발탄 환경 로드와 현재 재질 입력

`CLevelRegistry`는 `LV_LUT_HEARTRB_ED`와 full map scope를 선택한다.
`Level_ValtanArena.cpp`의 MapRuntime, DeployRuntime, WorldDestructionDebrisPresentationRuntime이
아래 모델을 준비한다. 비표시 배치와 아직 재생되지 않은 파편 prototype도 로드 분모에 포함된다.

| 환경 경로 | 고유 모델 | 사용 submesh/material 슬롯 | D | N | S | E |
|---|---:|---:|---:|---:|---:|---:|
| 주 정적 맵 | 263 | 333 | 333 | 305 | 50 | 9 |
| Deploy 정상/파괴 상태 | 20 | 54 | 54 | 48 | 44 | 48 |
| 입장 시 준비하는 파괴 파편 | 100 | 294 | 289 | 289 | 289 | 289 |
| 합계 | **383** | **681** | **676** | **642** | **383** | **346** |

- 주 catalog272개 중 실제 참조263개. mapplacements13,184개, visible12,874개.
- 원본 component 배치13,091개 + EDITOR90개 + hidden DEBUG_REFERENCE3개다.
- Deploy 정의12개/배치151개. 파편 recipe144개가 고유 모델100개를 사용한다.
- WModel 383개는 모두 v1.0이며 UV1/COLOR0/새 explicit tangent handedness 보존 플래그가 없다.
- ORM 및 별도 metallic/roughness/AO 경로는0이다. 이것은 현재 운반 상태이며 원본에 해당
  의미가 없다는 증거가 아니다. 원본이 PBR이라는 증거도 아니다.
- 참조된 모델/텍스처 물리 파일 누락0. 파편의5슬롯은 D/N/S/E 경로가 비어 gray fallback을
  사용한다. fallback과 파일 경로가 있으나 파일이 없는 결함은 다르다.
- 별도 LANDSCAPE catalog와6모델은 있으나 현재 area loader에 연결되지 않는다.

발탄에는 현재 `.mapmaterials.json`의 source material/placementLighting 연결이 없다.
따라서 **baked lightmap·환경 cube 연결0**이다. 현재 일반 shader의 D/N/S/E 처리는 있으므로
발탄 전체가 diffuse 한 장만 그리는 상태는 아니다.

근거: [runtime 요약](../../../out/ValtanMaterialLightingAudit20260908/runtime_summary.json),
[모델·슬롯 CSV](../../../out/ValtanMaterialLightingAudit20260908/runtime_material_slots.csv),
[소비 코드·분모 설명](../../../out/ValtanMaterialLightingAudit20260908/runtime_findings.md).

## G03. 원본 배치가 지정한 재질과 셰이더 계산

13,091개 원본 component는 모두 현재 sourcePlacementId와 매칭됐다. 배치별 사용 재질
16,251슬롯 중 원본의 non-null explicit override는9,701슬롯이다. 이 중 **6,710슬롯,
5,836배치**는 현재 WMAT 이름을 원본 소유 package에서 유일하게 찾은 후보와 다르다.
이 비교는 원본 component가 별도 재질을 지정한다는 근거다. WMAT 후보를 원본 StaticMesh
native default의 직접 증명으로 승격하지 않았고, 서로 다른 MIC가 반드시 다른 화면을 만든다고
일괄 판정하지 않았다. 동일 텍스처여도 원본 상수·static switch가 다를 수 있다.

현재 배치에서 식별된 effective 재질은279개 identity/14,689슬롯이고1,562슬롯의 native
default identity는 미확정이다. 일반 source material 조사와 분리한 특수 catalog는 Landscape6,
BG_RAD_VALTAN overlay3, debug phase proxy3개다. 아래363개 조사 분모를 이 특수12개나
Deploy·파편 전체의 원본 재질 복구 완료로 확대하지 않는다.

주 정적 맵 WMAT 후보193종과 component override181종의 합집합은363종이며, 부모 포함
395개 Material/MIC 객체를 읽었다. 이 source cohort의 terminal family는20개다.
주요 계열은 `bg_base_opa`185, `bg_simple_opa`59, `bg_base_msk`40,
`bg_foliage_msk`38개이며 모두를 CS 금속 PBR family로 취급할 근거가 없다.

363종 중361종은 BaseMaterial GUID와 engine-equality static parameter set으로 exact
원본 shader map을 연결했다. Base/Directional/Baked/VS의 고유 DXBC292개를 회수했다.
2개 volcano 재질은 uniform-expression parser의 constant type 제한으로 미해석이다.
추출 프로그램 중6개는 texture/vector 입력이0인 경우를 현재 binding helper가 허용하지 않아
해당 바인딩 확인이 미완료다. 원본 바이트 존재, 바인딩 해석, 제품 실행을 구분한다.

확인한 Base+Directional PS pair 중 현재 활성 쿠크/CS의 비교 가능한 식과 exact ID 일치는0,
기존 CS 전체 조사(미활성 후보 포함)와 같은 pair는51종, 그 밖은310종이다.
이는 새 shader310개를 만들어야 한다는 뜻이 아니다. **같은 연산·입력을 묶는 family 복원과
원본 static 분기 대조가 필요하다.** 현재 공용 로더·재질 variant·RNM 운반을 재사용할 수 있지만,
CS PBR 계산에 발탄 텍스처만 꽂으면 원본 복원이 된다고 할 수 없다.

근거: [재질·부모 목록](../../../out/ValtanMaterialLightingAudit20260908/source_material_targets.json),
[원본 shader map](../../../out/ValtanMaterialLightingAudit20260908/source_material_cache.json),
[DXBC·바인딩 목록](../../../out/ValtanMaterialLightingAudit20260908/source_material_program_index.json),
[상속값·texture 입력](../../../out/ValtanMaterialLightingAudit20260908/source_material_effective_inputs.json),
[기존 식 대응](../../../out/ValtanMaterialLightingAudit20260908/source_material_reuse_classification.json).
대표 식과 미확정 경계의 해설은 [원본 재질 상세 결과](../../../out/ValtanMaterialLightingAudit20260908/source_material_findings.md)에 있다.

## G04. 구운 조명·환경·후처리 자료

| 원본 정적 패키지 | StaticMeshComponent | LightMap2D 부분 해독 | LightMapTexture2D | Light component |
|---|---:|---:|---:|---:|
| PS | 142 | 138 | 26 | 2 |
| SL00 | 682 | 664 | 66 | 5 |
| SL01 | 3,603 | 3,563 | 76 | 35 |
| SL02 | 2,094 | 2,069 | 32 | 21 |
| SL03 | 2,543 | 2,500 | 86 | 25 |
| SL04 | 2,324 | 2,138 | 114 | 51 |
| SL05 | 1,703 | 1,672 | 20 | 19 |
| 합계 | **13,091** | **12,744** | **420** | **158** |

420개는 평균색/방향 정보210쌍이다. 처음 중간 설명의520은 합산 오류로 즉시 정정했다.
420개 모두 원본 package 내부에 mip payload가 있으며 총3,578mip의 범위·크기를 확인했다.
bulk flags512(Crunch 계열)1,048개와128 압축 계열2,530개다. DDS 변환·배포는 하지 않았다.

12,744개는 shadowmap 참조 배열을 따라가며 LightMap2D subrecord의 텍스처·RGB scale·
atlas scale/bias·baked Light GUID를 회수한 수다. 현재 visible 배치와의 교집합은12,593개다.
나머지347개에는 다른 native lightmap/vertex stream 등이 섞여 있어 lightmap 없음으로 세지
않았다. 해독한839개에도 미해석 후속 bytes가 남는다. 모든 component ABI 완료가 아니다.
두 번째 shadow 배열은 관찰 자료에서 모두 비어 있어 nonempty 형식은 아직 검증하지 않았다.

원본 조명158개는 DominantDirectional1, Sky1, Point156개다. 현재 제품의 Point22개는
모두 SL04 출처이고 실제 Level provider에 연결되어 있다. **22개 모두 그 LightMapGuid가
원본 구운 조명 component 목록에서 확인됐다.** 향후 lightmap 추가 시 같은 diffuse 조명을
중복 합산하지 않도록 static/dynamic, specular, shadow 기여를 나눠야 한다. 즉시22개를
끄거나158개를 모두 동적 광원으로 추가하라는 뜻은 아니다.

PS에서 읽은 대표 원본 값은 다음과 같다. 원본 엔진의 단위·상속·lighting channel·연출
활성 조건을 포함한 값이므로 현재 Rendering Tool 슬라이더와 숫자가 같다고 동일 조명이 아니다.

- DominantDirectional: brightness0.9, RGB byte(245,236,220), pitch 약-51.015도.
- Sky: brightness0.4, lowerbrightness0.3, RGB(193,208,222). 별도 lighting channel 설정 존재.
- Lightmass: environment color(199,218,219), intensity0.8, AO 사용, max distance40.
- ExponentialHeightFog: density1.5, max opacity0.2, start distance1600(원본 단위), 별도 색 두 종류.
- 환경 볼륨2개: 주광원·안개·후처리 override와 blend time. 그중 하나는 명시 enabled=false.
- 기본 후처리: desaturation0.3, midtone(1.1,1,1), bloom threshold0.3 및 색 보정 LUT 참조.
- 실제 색 보정 LUT `lv_lut_heartrb.tex.lv_lut_heartrb_lut`: 256×16, A8R8G8B8,
  SRGB=false, ColorLookupTable group. **이것은 BRDF LUT가 아니다.**
- 주광원의 LightFunction은 `lv_lut_heartrb.mat.lv_lut_heartrb_lightfunction_midday_01`이다.

현재 프로젝트 발탄 profile은 푸른 직접광 diffuse(.24,.28,.34), 별도 ambient/specular,
exposure multiplier1.55, fog=false다. 원본의 전체 환경·후처리를 이미 옮긴 설정이 아니다.
원본 LightFunction/ShadowMap, environment blend, color grading 적용도 별도 남은 범위다.

PS streaming19참조까지 따라간 추가13패키지에서 LAND01의 landscape6개와 별도
LightMapTexture2D114개, SCENE07A의 Point2개·SCENE06A의 Point1개, 연출 track을 찾았다.
이114개는 위 정적420개와 다른 집합이다. 현재 제품 미로드 LAND01과 컷신을 섞어서
현재 바닥의 누락 배치나 항상 켜야 할 광원으로 집계하지 않았다.

근거: [원본 조명 목록](../../../out/ValtanMaterialLightingAudit20260908/source_lighting_inventory.json),
[mip 저장 검증](../../../out/ValtanMaterialLightingAudit20260908/source_lightmap_payloads.json),
[GUID·현재 profile](../../../out/ValtanMaterialLightingAudit20260908/source_light_guid_and_runtime_profile.json),
[추가 streaming 패키지](../../../out/ValtanMaterialLightingAudit20260908/source_auxiliary_levels.json),
[환경 texture](../../../out/ValtanMaterialLightingAudit20260908/source_environment_textures.json).

## G05. 중앙 석재 바닥의 첫 복구 단위

첫 비교 대상으로 아래 한 배치를 특정했다.

| 항목 | 실제 확인값 |
|---|---|
| source placement | `LV_LUT_HEARTRB_ED_SL00:export:1271` |
| runtime placement | `11789713951910399058` |
| asset | `MAP_4A6CF4B84315_LV_LUT_HEARTRB_FLOOR01_SM` |
| pivot | 약(156.2791,23.2420,-121.9766)m |
| 원본 MIC | `lv_lut_heartrb.mat.bg_pap_stone_rock04_mi_ksr` |
| 원본 계열 | `bg_base_opa` |
| lightmap pair | SL00 `normalizedaveragecolor1_55` / `directionalmaxcomponent1_55` |
| UV scale / bias | (.2421875,.2421875) / (.25390625,.00390625) |
| 평균 RGB scale | (1,1,1) |
| 방향 RGB scale | (4.33382845,4.41132402,4.23090124) |

같은 중앙 mesh 배치는1271/1299/1304/1337 네 개이며 맵 전체에는24개가 있다.
주변 streetfloor 중 같은 Base/Direct/VS를 쓰는 원본 MIC도 있어 첫 표면의 계산을 검증한 뒤
확장할 후보가 된다. 현재 중앙에는 별도 Deploy RAIL/BRICK_A/BRICK_B 여섯 배치와 더 낮은
정적 원형 바닥도 있다. pivot이 같다고 눈에 보이는 최상단 triangle이 동일하다고 단정하지 않는다.
Deploy를 정적맵과 같이 한꺼번에 교체하거나 파괴 동작을 바꾸지 않는다.

이 원본 석재 program은 기본 D/N과 overlay D/N 네 장, D alpha와 overlay alpha,
vertex color, 별도 saturation/brightness/normal/specular 상수를 소비한다. 현재 모델은
기본 D/N 두 장을 일반 shader로 읽는다. 확인된 대표 원본 값은 normal intensity1.5,
diffuse saturation0.2/brightness0.8, overlay tiling2.5/brightness1.2,
specular intensity0.2/power60이다.

원본 직접 반사는 해당 power를 쓰는 Blinn 계열이며 이 선택식에 ORM 텍스처는 없다.
D alpha는 이 Base pass에서 opacity clip 용도가 아니다. vertex R은 overlay coverage,
vertex alpha는 normal XY 강도에 들어간다. **원본 canary가 실제로 비백색 vertex color를
갖는지는 아직 미확정**이다. source glTF에 COLOR_0가 없으므로 흰색 default 의미와
component override를 확인해야 하며, overlay가 실제 화면에서 소실됐다고 확정하지 않는다.

단일 원본 mesh를 `-notex -gltf -obj=lv_lut_heartrb_floor01_sm`으로 out에만 추출했다.
LightMapCoordinateIndex1/Resolution64 및 실제 UV1을 확인했다. 원본 glTF33,002정점은
모두 UV0와 UV1이 다르며 tangent W는 +1 18,950개/-1 14,052개다. 현재 WModel은
32,940정점이고 양쪽 index 수33,063은 같다. **UV1을 같은 정점 번호에 바로 복사하면
안 된다.** 기존 geometry 변환의 topology 대응을 검증하거나 해당 모델을 필요한 입력과 함께
정확히 재쿠킹해야 한다. 모든 모델의 형상이 틀렸다는 결론은 아니다.

실제 구현 순서는 아직 수행하지 않은 다음 변경 단위로 정리한다.

1. 해당 source placement·MIC·기본/overlay 입력·vertex 기본값을 확정하고 기존 재질 variant를 만든다.
2. 확인한 원본 UV1/기준축을 기존 WModel 경로에 보존한다. COLOR0는 존재/기본값 근거에 따라 처리한다.
3. `bg_base_opa`의 필요한 표면·직접 반사·baked 분기를 공용 material 경로에 연결한다.
   CS PBR family 번호나 미활성 out의 enum 번호를 그대로 재사용하지 않는다.
4. 같은 RNM texture pair와 배치별 scale/bias를 기존 mapmaterials/placementLighting에 연결한다.
5. 현재 조명과 baked의 중복 기여를 분리한 A/B를 준비하고 사용자 중앙 바닥 비교를 받는다.
6. 같은 계산의 다른 배치로 확대하고 Deploy 정상/파괴 표면, 다른 석재/식생, 안개/환경/후처리 순서로 확장한다.

이 과정의 예상 신규 runtime 입력은 선택 바닥의 수정 WModel, 실제 누락된 overlay texture,
선택 lightmap DDS pair 등이다. **아직 Resources에 추가하지 않았다.** 이번에 찾은
색 보정 LUT나420개 lightmap 전체를 첫 단계에서 배포할 필요는 없다. 추가 시 정확한
Resources 상대 경로와 Drive 공유 필요 여부를 별도로 보고한다.

근거: [중앙 배치·재질·lightmap](../../../out/ValtanMaterialLightingAudit20260908/runtime_central_floor_canary.json),
[원본 mesh tagged 값](../../../out/ValtanMaterialLightingAudit20260908/source_representative_mesh_inputs.json),
[원본 UV1·tangent 실측](../../../out/ValtanMaterialLightingAudit20260908/runtime_floor_geometry_audit.json).

## G06. 실행한 검증과 남은 경계

실행: 현재 모델/재질 참조 전수 읽기, source ID join,7주패키지+13추가패키지 원본 읽기,
420texture/3,578mip inline 범위 검사, 원본 GUID join, shader map/정적분기/선택 DXBC 추출,
대표 원본 geometry 단일 추출·유한값 검사, 조사 JSON parse, 문서 `git diff --check`.

LightMap2D decoder는 별도로 SL00/SL04의4개 원본 component에서 tagged 끝/native tail과
shadowmap2d/texture refs를 재확인했다. 12,744개 전체 UV scale/bias 범위와 유한값도 확인했다.
검증 기록: [원본 표본 대조](../../../out/ValtanMaterialLightingAudit20260908/character_lightmap_sample_review.json).

미실행: 제품 변경·컴파일·새 EXE 생성·실행·사용자 화면 A/B. 미완료: 원본 두 volcano 식,
일부 zero-input shader binding, native lightmap1D/색 stream, 원본 shadow와 lightfunction,
scene-owned environment/SH와 정확한 frame 활성 조건, 파편/Deploy 전체 원본 shader 연결.
current WMAT 후보를 native mesh default 정답으로 일괄 승격하지 않았다.

사용자가 다른 작업의 EXE를 빌드·검토하는 것은 이번 조사와 충돌하지 않는다. 이 조사 결과가
그 빌드에 발탄 재질 복구를 새로 포함한다는 뜻은 아니다.
