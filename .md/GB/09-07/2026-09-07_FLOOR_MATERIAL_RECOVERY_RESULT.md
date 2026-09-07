# 쿠크·Character Select 바닥 재질 복구 결과

작성일: 2026-09-07

최신 갱신: 2026-09-08. G00~G07은 앞서 완료한 쿠크 선택 재질의 기록이다.
Character Select에 추가한 실제 구현·검증과 남은 맵 동일성 문제는 G08~G11을 따른다.

## G00. 반영 범위와 판단

쿠크의 지정된 바닥 두 재질에서 원본 입력 → CMaterial → 일반/인스턴스 shader →
G-buffer → deferred 광원 → Rendering Workbench 진단을 연결했고, 제품 publish·컴파일·수치 검증을 마쳤다.
원본 shader 캐시를 실제 MIC와 연결해 확인한 표면식을 사용하고, 원본 근거가 닫히지 않은
반사 좌표·vertex alpha·조명은 아래 G05에 근사 경계로 구분한다.

현재 확인된 문제는 Specular RGB 손실, 활성 material 기능/수식 미전달, 두 메시의 tangent
handedness 손실이다. 이들이 실제 장면의 탈색감에서 차지하는 비율과 GPU 성능 병목은 측정하지 않았다.
Exposure/Gamma/채도만 바꾸는 것으로 누락된 식을 복구할 수는 없다.

이 쿠크 단계에서는 Character Select의 원본 material 복구를 선택 적용에 포함하지 않았다. 해당 장면은 공통
renderer 변경의 회귀 확인 대상이다. 이펙트의 원본 식·레이어·타이밍과 조작감도 별도 범위다.

## G01. 원본 근거와 모델 수정

| 대상 | 아래 바닥 | 위 바닥 |
|---|---|---|
| Asset ID | `MAP_61FA80B72343_BG_RAD_KOUKUSATON_FLOOR08_SM_OVR_04DCDAA75173` | `MAP_0BE6EBEE4924_BG_RAD_KOUKUSATON_FLOOR08A_SM_OVR_84D65232B541` |
| Placement ID | `12451899878577673092` | `10472891450652540488` |
| WModel material | `SLOT_000_bg_rad_koukusaton_floor08_mi` | `SLOT_000_bg_rad_koukusaton_floor08a_mi` |
| 실제 배치의 원본 material | `lv_lut_midnightc.mat.bg_rad_koukusaton_floor09b_mi` | `lv_lut_midnightc.mat.bg_rad_koukusaton_floor08b_mi` |
| Family | `bg_seamless-specular_msk` | `bg_base_msk` |
| Diffuse brightness / normal intensity | 0.6 / 0.5 | 0.7 / 0.5 |
| Specular intensity / power | 0.2 / 30 | 0.2 / 100 |
| Reflection intensity / contrast | 0.2 / 0.5 | 0.2 / 0.5 |
| Specular 입력 | S.rgb | 밝기·채도 변경 전 D.rgb |

두 MIC는 installed shader cache의 material map에 각각 하나로 대응했다. BasePass와
DirectionalLightPass의 표면 연산과 binding을 확인했다. 원본 Normal.A는 roughness가 아니라
반사 mask다. D/S/reflection은 sRGB, Normal RG/A는 linear다. 위 material의 S/Opacity texture
비활성 분기를 보존하며 파일이 있다는 이유로 켜지 않는다.

반사는 `Emissive` 가산이 아니라 조명 전 바닥 고유색을 바꾸는 연산이다.
Normal RG에서 Z를 먼저 복원하고 XY에 강도를 곱한 뒤 normalize한다.
구체적인 식과 캐시 연결 근거는
[원본 수식 조사](/C:/Users/user/Desktop/LostArk/out/FloorMaterialRestore20260907/source_equations.md)에 보존했다.

현재 설치 WModel은 tangent.w가 모두 +1이었다. 기존 geometry cooker를 재사용해 두 파일만
교체했다. 아래 663정점/2,034인덱스, 위 362정점/1,152인덱스의 position/N/UV/T.xyz와 index
바이트, material section 전체를 보존했다. 원본에 있던 아래 COLOR0도 보존했다.
정점 단위와 100배 어긋나 있던 embedded bounds는 실제 정점 단위로 교정했으며,
제품의 정점 기반 실제 bounds와 배치 크기는 유지했다. 복구된 TBN에는 추가 binormal 반전이 없다.

근거: [모델 교체·원본 대조·C++ decode 결과](/C:/Users/user/Desktop/LostArk/out/FloorMaterialRestore20260907/source_geometry_recovery.json).

## G02. 실제 데이터와 소비 경로

정본은 `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.mapmaterials.json`이다.
schema `lostark.map-materials`, formatVersion 1이며 `assetId + materialName`으로 연결한다.
`MapCatalog.json`의 sourceMaterials/materials 쌍과 함께 publish한다.

```text
mapmaterials JSON → Publish-MapAuthoring → runtime mapassets v5 필수 참조
 → CMapAssetCatalog → Loader / MapTool MODEL_ASSET_LOAD_DESC
 → CModel의 named material join → CMaterial의 immutable 입력과 texture SRV
 → MapAssetRenderUtils → 공통 Shader_MapMaterialSurface.hlsli
 → 일반/인스턴스 MRT → Shader_Deferred directional / point / spot
```

- 기존 catalog v1~v4와 mapset v1은 읽으며, v5는 v4 row 구조에 필수 재질 문서 참조를 추가한다.
- publisher는 실제 요청 material 이름이 WModel에 정확히 한 번 존재하는지 확인한다.
  사용하지 않는 빈 material 행은 유지한다. unknown asset/family/field, 중복, 잘못된 경로,
  float overflow와 누락된 필수 reflection을 거부한다.
- catalog 로드와 publish는 stage 성공 뒤 교체한다. 실패 시 기존 자료와 catalog를 유지한다.
- CModel은 GPU 생성 전 override를 검증하며 clone 사이에 공유하는 Material을 뒤늦게 변경하지 않는다.
- source D/S/reflection SRV는 기존 A/B texture 해석과 별도로 준비한다. 일반 Area에는 선택 적용하지 않는다.
- 새 JSON은 Client 프로젝트 `96.DataFiles`, 공통 HLSLI는 `97.ShaderFiles`에 등록했다.
- 현재 material 숫자 편집은 JSON 정본 수정 → 명시적 map publish → Client 재시작 순서다.
  기존 Runtime Reload/Quality Save는 material prototype 재생성 기능이 아니다.

## G03. Renderer와 비교 기능

`Target_MaterialSpecular` FP16 RGBA를 여섯 번째 G-buffer로 추가해 RGB를 유지한다.
기존 `Target_Specular`는 계속 광원 누적 결과다. Depth.w는 0=미선택 표면,
1=복구 계산, 2=대상 재질의 기존 계산이다. 방향광·Point·Spot은 marker와 RGB를 같은 픽셀에서
Load하므로 앞쪽 캐릭터가 바닥을 가렸을 때 남은 바닥 RGB를 소비하지 않는다.
기존 Emissive overlay는 새 target에 쓰지 않는다. clear marker는 0이다.

F1 Developer Tools → Rendering Workbench → **Floor Materials**에 다음을 추가했다.

- `Recovered floor materials (A/B)`: 체크=복구 B, 해제=기존 재질 계산 A.
- `Material debug view`: Final, Base color, Normal, Direct specular, Reflection delta.
- 최근 1초 동안 성공적으로 바인딩된 asset/material/family/program 표. 최대 32행이며 Level 변경 시 비운다.

진단은 조명·Bloom·tone mapping 영향을 분리한다. Reflection delta는 반사로 바뀐 고유색의
절댓값이며 원본 반사 RGB 자체나 Emissive가 아니다. 대상 밖 픽셀은 진단에서 검게 표시한다.
Direct specular가 검다면 광원의 specular 입력도 확인해야 한다.

A/B는 모델을 다시 읽지 않고 준비된 입력을 선택한다. 두 모드 모두 handedness가 교정된 모델을
사용하므로 A가 수정 전 프로그램의 모든 픽셀을 재현하는 것은 아니다.
선택 상태는 세션 진단이며 RenderingProfiles JSON에 저장하지 않는다.

## G04. 검증 기록

| 검증 | 실제 결과 |
|---|---|
| 원본 식 CPU 검산 | 21 fixture PASS |
| 기존 WModelGeometryContractHarness 최소 재컴파일 | Debug PASS |
| 지정 두 WModel C++ candidate / semantic dump | 2개 모두 exit 0, source/writer/reader 채널 해시 일치 |
| material publisher fixture | 3 tests PASS; single/shard, 미사용 빈 슬롯, target 중복, 잘못된 입력과 교체 실패 rollback 포함 |
| 기존 Kouku world admission unit tests | 9 tests PASS |
| 제품 map Validate / Publish / Check | 모두 PASS, 3,231 placements / 8 files |
| Debug Product | 최종 Engine → Shared → Server → Client 컴파일·SDK/shader/DLL 배포 PASS, 20260907T144208267Z |
| 실제 shader WARP 수치 검증 | 최종 89/89 PASS, 실패 0; source/include hash 변화 0 |
| 실제 catalog → CModel 입력 로드 | Native WARP 23/23 PASS, 실제 323 entries/2 material assets, 두 모델 생성·Clone 수명·실패 거절·기존 catalog 보존 |
| 변경 JSON/XML / diff | JSON 3개·XML 2개 parse, 관련 `git diff --check` PASS |
| Client/UI 실행·화면 캡처·사용자 육안 판정 | 수행하지 않음, visual PASS 미기록 |

주요 검증 자료:

- [최종 Debug Product 결과](/C:/Users/user/Desktop/LostArk/out/BuildPipeline/runs/20260907T144208267Z-debug-product.json)
- [제품 map publish](/C:/Users/user/Desktop/LostArk/out/FloorMaterialRestore20260907/publish.log)와 [일치 Check](/C:/Users/user/Desktop/LostArk/out/FloorMaterialRestore20260907/publish_check.log)
- [GPU shader 89개 결과](/C:/Users/user/Desktop/LostArk/out/FloorMaterialRestore20260907/floor_warp_results.json)와 [검증 범위](/C:/Users/user/Desktop/LostArk/out/FloorMaterialRestore20260907/floor_warp_report.md)
- [실제 입력·CModel·Clone 23개 결과](/C:/Users/user/Desktop/LostArk/out/FloorMaterialRestore20260907/catalog_native_result.json)
- [JSON/XML·관련 diff 확인](/C:/Users/user/Desktop/LostArk/out/FloorMaterialRestore20260907/floor_final_static_checks.json)

초기 컴파일에서 누락된 타입 전방 선언과 Windows max 매크로 충돌을 수정했다.
초기 publisher의 미사용 빈 material 행 거절도 수정하고 해당 회귀 fixture를 추가했다.
마지막 WARP 검증 중 발견한 source shadow의 UV/alpha 및 presentation opacity 차이도 수정해 재검증했다.
위 표는 이 수정 이후의 결과다.

자동 검증 자료는 `out/FloorMaterialRestore20260907/`에 보존한다.
WARP는 실제 product shader와 합성 texture/geometry를 사용하는 수치 검사다. 원본 게임 화면과
비교하거나 Client를 실행한 결과가 아니다. 최종 화면의 선명도와 프레임 비용은 사용자 확인이 남는다.
컴파일에는 기존 문자 인코딩 경고와 third-party PDB 경고가 있으며 경고 0으로 보고하지 않는다.

## G05. 남은 근사와 후속 범위

1. 아래 09b의 reflectionUV에서 CB0 엔진 원점 보정의 정체가 미확정이다. 해당 offset은 0으로 둔다.
2. 위 08b의 원본 추출 mesh에는 COLOR0가 없어 vertex alpha 1을 사용한다. native 배치 override-color
   기본 의미를 전부 복원한 상태는 아니다.
3. texture sampling은 현재 linear/wrap 경로를 사용한다. 원본 per-texture address override는 없지만
   native constructor의 정확한 기본 addressing은 별도 확인이 필요하다.
4. 기존 제품의 Phong 조명은 원본 Blinn/간접광/lightmap과 동일하지 않다. 원본 실장면의 lightmap
   pass 선택·장면 광원·후처리를 이번 표면식 구현으로 복구했다고 주장하지 않는다.
5. Character Select는 아래 G08에서 별도 원본 입력으로 확장했다. 이펙트와 조작감은 이번 구현 범위가 아니다.
6. 여섯 번째 target의 메모리·대역폭 비용이 추가된다. 실제 장면 FPS/GPU ms는 측정하지 않았다.

## G06. 사용자가 확인할 정확한 순서

1. 최신 데이터·셰이더를 읽도록 Client를 새로 시작한다. 이 PC는 LAN 설정상 server-host이므로
   Visual Studio의 `Server + Client` profile을 `Ctrl+F5`로 시작한다.
2. Lobby → KoukuSaydon에서 기존 관문 이동의 **1관문 - 세이튼**으로 이동한다.
   `Debug_ActivateGate`가 요청하는 플레이어 위치는 `(-2.45, 1.32, 945.17)`이며 이번 복구 바닥이다.
   **3관문 - 세이튼**도 같은 바닥 위치를 사용한다. 최초 진입 지점과 별도 g1.kouku 배치는 다르다.
   SL05는 원본 서브레벨 식별자다. 별도 StageMarkers 이동은 현재 결함이 있다.
   disabled waypoint를 GameRoom이 그대로 Is_PlayerSpawnClear에 넘기는데 이 함수가 disabled를 거절한다.
   따라서 SL01~05가 조용히 거절되는 현재 코드 경로를 확인했다. 이번 작업에서 Server 코드는 수정하지 않았으며,
   이 경로를 사용자 확인 경로로 사용하거나 동작 검증 완료로 안내하지 않는다.
   이전 결과 보고의 SL05 안내는 관문 이동과 내부 구역 식별자를 혼동한 안내 오류로 정정한다.
3. Rendering Workbench → Floor Materials에서 최근 바인딩 표에 위의 대상 재질이 나타나는지 확인한다.
4. 카메라·광원·Exposure/Gamma/Bloom을 유지한 상태에서 A/B를 전환한다.
   Base color → Normal → Direct specular → Reflection delta → Final 순서로 비교한다.
5. Final로 되돌리고 Character Select에서 바닥·캐릭터·투명 이펙트의 회귀를 확인한다.
   이 확인은 Character Select 원본 재질 복구 판정이 아니다.

## G07. 전달과 작업 상태

Resources에서 바뀐 것은 다음 두 폴더 안의 동명 `.wmodel`이다.

```text
Client/Bin/Resources/Map/LV_LUT_MIDNIGHTC_ED/MAP_61FA80B72343_BG_RAD_KOUKUSATON_FLOOR08_SM_OVR_04DCDAA75173/
Client/Bin/Resources/Map/LV_LUT_MIDNIGHTC_ED/MAP_0BE6EBEE4924_BG_RAD_KOUKUSATON_FLOOR08A_SM_OVR_84D65232B541/
```

Reflection은 각 폴더의 기존 `textures/d89521db7863_ambientreflection_15.dds`를 사용한다.
원본 두 WModel의 백업은 `out/FloorMaterialRestore20260907/root_baseline/`의 같은 상대 경로에 있다.
다른 PC에 전달할 대상은 식별됐으며 팀장 Drive에 실제 업로드하지 않았다. binary payload는 Git에 추가하지 않았다.

브랜치는 `codex/kouku-gate-pattern-bundles`다. 다른 세션의 대규모 미커밋 변경이 함께 있어
자동 stage/commit/push하지 않았다. 해당 변경을 되돌리거나 이번 기능의 검증 완료로 묶지 않았다.

최종 준비 시점에 Client와 Server 프로세스는 실행 중이지 않았다. Server CMD도 에이전트가 새로 띄우지 않았다.
사용자의 실제 아레나 재생·스크린샷 관찰·visual fidelity 판정은 아직 남아 있다.

## G08. Character Select의 실제 반영 범위

2026-09-08에는 `LV_LOBBY_CLASSSELECT_SL00`의 원본 component가 선택한 네 material 조합을
기존 CModel → CMaterial 경로에 연결했다. 여섯 배치에 네 asset variant와 다섯 named material
행을 사용한다. 410/490의 slot 1은 원본의 기본 material 상속을 유지한다.

| source export 번호 | 모델 | 실제 적용한 재질 |
|---|---|---|
| 336 | FLOOR12 | `bg_elg_aryanorb_floor12_01_mi_wingart` |
| 371 | FLOOR12 | `bg_elg_aryanorb_floor17_mi_ksr_02` |
| 410 / 490 | BRIDGE01E | slot 0 `bg_elg_aryanorb_bridge01a_02_mi_khy_01`; slot 1 기본 `bg_elg_aryanorb_bridge01b_mi_khy` |
| 411 / 495 | BRIDGE01E | 두 slot 모두 `bg_rad_abrelshud_landmark03b_01_mi_psy_02` |

위 번호는 원본 export의 0-based 번호이며 runtime 저장 ID를 vector index로 바꾼 것이 아니다.
기존 803 placements 중 이 여섯 행의 asset 참조만 변경했다. 모든 non-asset token과 승인된
아홉 Y 보정을 보존했다. Imported catalog는 기존 55개에 variant 네 개를 추가한 59개다.

| 연결한 경계 | 실제 동작 |
|---|---|
| mapmaterials v2 / MapCatalog / publisher | 원본 두 PBR family의 texture·수치·색 공간과 optional baked/environment 입력을 검증·전달 |
| CMaterial / 일반·인스턴스 표면 shader | 원본 diffuse/normal/detail/ORM/반사 채널과 연산 순서를 실제로 소비 |
| G-buffer / deferred | roughness·metallic·F0와 geometric normal을 유지하고 선택 PBR 표면의 GGX 직접광을 계산 |
| 원본 lightmap | UV1과 배치별 atlas 좌표·색 계수로 RNM baked 조명을 계산; 해당 표면에 기존 ambient를 중복 가산하지 않음 |
| 원본 environment | 지정된 세 배치의 RGBM cube를 decode하고 프로젝트 BRDF lookup으로 환경반사를 계산 |
| 모델 / sampler | UV1을 운반하는 WModel 1.2, tangent handedness, 선택 표면의 mip·anisotropic sampling |
| Floor Materials | 기존 A/B와 진단에 Roughness / Metallic / Material AO 추가 |
| Character Select light runtime / Lighting Workbench | published maplights와 저작 preview를 기존 runtime에 연결하고 실패 시 기존 광원 보존 |

PBR 직접 반사는 해당 광원의 diffuse 색·밝기와 재질 F0/roughness를 사용한다. 기존 Phong 경로의
Specular RGB slider를 PBR 반사 세기와 동일하게 해석하지 않는다. PickPos의 보조 normal payload는
finite 값으로 저장하며, picking의 world position 반환에서는 w=1을 보장한다.

원본 FLOOR12의 tangent.w는 직접 보존했다. BRIDGE 추출물의 tangent.w/COLOR0는 재추출마다
달라지는 것을 확인해 UV 미분으로 handedness를 재구성했고 `PROJECT_RECONSTRUCTED`로 기록했다.
불안정한 COLOR0를 원본이라고 저장하지 않았으며 vertex alpha는 1을 사용한다. 모델의 기존 위치,
정점·인덱스 순서와 UV0는 유지하고 실제 source UV1을 추가했다. 이전 WModel 1.0/1.1도 읽는다.

선택 texture 15개의 top mip은 원본 값과 대조했다. 하위 mip은 texture별 색 공간과 alpha를 유지해
offline 생성한 프로젝트 축소본이다. 원본 전체 mip이라고 기록하지 않는다. 환경 cube는 원본
여섯 면 × 여덟 mip을 보존했다. Lightmap 네 장은 확보한 top mip을 사용한다.

원본 SL00의 Point 6개·Spot 1개를 조사해 대상 채널과 활성 상태에 맞는 `Original Point 86`,
`Original Spot 99` 두 개를 maplights에 연결했다. 이미 구워진 조명, 꺼진 조명과 대상 밖 채널을
모두 다시 켜지 않았다. Brightness·반경·감쇠의 엔진 단위 일치까지 검증한 것은 아니므로 이 데이터는
`PROJECT_AUTHORED`다. 사용자가 편집하던 RenderingProfiles와 카메라 저장값은 변경하지 않았다.

## G09. Character Select 검증과 배포 상태

| 검증 | 실행 결과 |
|---|---|
| 기존 geometry cooker 검사 | 7 tests PASS |
| 실제 엔진 WModel decode | 네 variant candidate/dump 모두 exit 0, UV1 해시·evidence 일치; 별도 기존 1.1 golden exit 0 |
| 원본 식 / GPU 수치 | 실제 product shader WARP 148/148 PASS; 기존 쿠크 89개 포함 |
| shader 컴파일 | 네 effect FXC PASS |
| 실제 CS catalog → CModel → Clone / SRV binding | 57/57 PASS, 누락·손상·UV1 없는 baked 입력 거절과 기존 catalog 보존 포함 |
| 기존 쿠크 catalog / CModel 회귀 | 23/23 PASS |
| CS map Validate / Publish / Check | 803 placements, runtime 네 파일 PASS |
| Debug Product | Engine → Shared → Server → Client와 shader/SDK/DLL 배포 PASS, `20260907T162427801Z` |
| 문서 정리 후 정적 확인 | JSON 5개·XML 2개 parse, `git diff --check` PASS; 줄바꿈 경고 존재 |
| Client 실행 / 사용자 육안 판정 | 에이전트 미실행; 사용자가 새 화면에서 색감 개선을 관찰해 PNG 첨부. 최종 원작 일치 PASS는 미기록 |

Native 검증은 별도 out fixture와 실제 Engine.dll/제품 shader를 사용했다. 제품 입력의 전후 해시가
동일함을 확인했다. 합성 입력의 수치·생성·바인딩 검사이며 실제 Client 화면 비교는 아니다.
Release 빌드나 실제 장면 FPS/GPU ms를 이번 검증에 포함하지 않았다.

이후 사용자는 `codex-clipboard-06850d1d-e681-4c28-b2f6-02493b8fa900.png`를 첨부하며
"확실히 때깔이 달라진 게 느껴지네"라고 개선을 보고했다. 첨부 이미지에서 이전의 회색 표면에 비해
밝은 석재색과 갈색 무늬·테두리의 구분이 관찰된다. 사용자 조명 조절과 카메라 차이를 통제한 A/B는
아니므로 개별 수정의 기여 비율이나 원작 최종 일치 판정으로 기록하지 않는다. 중앙 십자 구성,
어두운 ring 및 외곽 계단은 추가 진단 대상으로 남는다. 에이전트가 화면을 캡처한 것은 아니다.

주요 근거는 다음과 같다.

- [연결한 데이터와 배치 보존](C:/Users/user/Desktop/LostArk/out/CharacterSelectRestore20260908/data_connection.json)
- [실제 모델 decode](C:/Users/user/Desktop/LostArk/out/CharacterSelectRestore20260908/native_geometry_validation.json)
- [shader 수치 검증](C:/Users/user/Desktop/LostArk/out/CharacterSelectVisualReview20260908/floor_warp_results.json)
- [CS·쿠크 native 검증과 제품 입력 보존](C:/Users/user/Desktop/LostArk/out/CharacterSelectRestore20260908/catalog_native_receipt.json)
- [최종 Debug Product](C:/Users/user/Desktop/LostArk/out/BuildPipeline/runs/20260907T162427801Z-debug-product.json)

## G10. 아직 닫히지 않은 화면 차이

사용자 원본 PNG는 중앙의 완전한 원판·방사형 석재 무늬·사각 장식이 두드러지고, 제공된 모작
PNG에는 넓은 십자형 bridge가 중앙을 가로지른다. 이 차이를 조명이나 shader만으로 해결됐다고
판정하지 않는다. 원본 SL00 배치의 아래 FLOOR12 삼각형은 실제 존재하고 위 opaque bridge가
덮는다. 선택 diffuse의 alpha도 255여서 임의 투명화나 bridge 삭제·높이 변경의 근거가 없다.

현재 제품은 SL00다. 원본 설치 자료에서 관련 prefix 패키지 14개와 별도 SL01 패키지가 확인됐다.
세 중앙 mesh의 정확한 import/local export는 SL00에만 있고 다른 13개에는 없다. SL01은
BG_SHS_RCARENA 계열 바닥·다리를 쓰는 별도 환경이므로, 같은 중앙 원판의 다른 높이 배치를 가진
대체 SL이라는 근거는 없다. SL00에는 FLOOR12 13개, BRIDGE01E 8개, MAGICFLOOR03D 15개가
있으며 선택 여섯 배치를 포함한 중앙 세 mesh 계열에는 명시적인 HiddenGame/Layer/ForcedLOD
태그가 없다. 명시적인 HiddenGame 24개는 별도 navigation mesh component다.

사용자 PNG가 같은 SL00/버전/표시 조합을 사용한다는 증거는 아직 없다. 이번 좁은 대조로 다른 SL
선택이나 숨김 flag 하나를 원인으로 확정할 수 없었다. 원본 표시 조건과 중앙 구성의 추가 확인은
남아 있으며 동일 모델 이름만으로 장면 전체가 동일하다고 확정하지 않는다.

다음 입력은 원본과 완전 일치하지 않는 프로젝트 근사로 남는다.

- ORM·lightmap SRV의 원본 기본 색 공간 미확정: 현재 linear.
- BRDF LUT: 원본 엔진 LUT가 아닌 프로젝트 적분 결과.
- 원본 hemisphere/SH 엔진 상수 미확보. 임의의 전역 ambient로 채우지 않음.
- minimum roughness 0.04, world reflection 원점 0, environment 색·회전 기본값.
- BRIDGE tangent 부호 재구성, vertex alpha 1, 생성 mip과 광원 단위·감쇠 보정.

재질과 광원 입력의 실행 준비는 완료했으나 원작과의 최종 시각 유사도는 미판정이다.
전체 이펙트·후처리·조작감 복구나 성능 병목 해소로 범위를 확대해 보고하지 않는다.

## G11. 사용자 확인과 전달

최신 Debug 빌드 후 Client를 새로 시작하고 Lobby → Character Select로 들어간다.
이 PC의 LAN 역할은 server-host이므로 Server가 필요하면 `Server + Client` profile을 사용한다.
F1 Developer Tools → Rendering Workbench → Floor Materials에서 최근 바인딩의 네 variant를
확인한다. 같은 카메라와 광원에서 A/B를 비교하고 Base color / Normal / Roughness / Metallic /
Material AO / Final을 본다. 저작한 map light는 Lighting Workbench의 Character Select에서
`Original Point 86`, `Original Spot 99`로 구분한다. 사용자의 첫 색감 개선 관찰은 G09에 기록했으며,
고정 카메라 A/B와 최종 원작 일치 판정은 남아 있다.

새로운 local Resources 입력은 아래 네 variant 폴더와 lighting 폴더다.

```text
Map/CHARACTERSELECTMAP/MAP_54B7E9F1F9F0_BG_ELG_ARYANORB_FLOOR12_SM_WINGART_OVR_42610589A3EB/
Map/CHARACTERSELECTMAP/MAP_54B7E9F1F9F0_BG_ELG_ARYANORB_FLOOR12_SM_WINGART_OVR_75EF6B0F204F/
Map/CHARACTERSELECTMAP/MAP_AC527A4AF171_BG_ELG_ARYANORB_BRIDGE01E_SM_OVR_229A4D91C10C/
Map/CHARACTERSELECTMAP/MAP_AC527A4AF171_BG_ELG_ARYANORB_BRIDGE01E_SM_OVR_E64DE25941B5/
Map/Lighting/CharacterSelect/
```

물리 위치는 모두 `Client/Bin/Resources/` 아래다. 네 WModel, 선택 texture 15종과 조명 입력
DDS 여섯 개를 사용한다. 구체적 경로는 `out/CharacterSelectRestore20260908/asset_handoff.json`과
`material_path_handoff.json`에 있다. Drive 전달은 수행하지 않았고 binary payload는 Git에 추가하지 않았다.
다른 작업의 대규모 미커밋 변경이 있어 자동 stage/commit/push하지 않았다.

## G12. 전체 맵 확대 진행 중: 중앙 링 연결

2026-09-08 전체 Character Select 복구 요청에 따라 원본 803배치/55 mesh/75 실제 사용 재질을
조사했다. ordered-material 조합 62개이며 atlas·환경반사까지 구분하면 179개다. 75개 재질의
native scalar/vector/default/texture 선택과 14개 terminal family를 확인했다. 이 숫자는 실제
제품 연결 완료 범위를 뜻하지 않는다. 원본 texture 178개와 lightmap 58개/cube 2개를 out에 준비했다.

중앙 MAGICFLOOR03D의 15배치를 원본 bg_pcselect14.mat.bg_elg_kayangel_floor02a_mio_ksr와
program 5에 연결했다. UV tiling 4, source specular intensity 7/power 60, source RGB tint와
2D reflection을 사용한다. atlas 74/227의 두 그룹은 _LGT_6143955C3FE8/_LGT_49D0D3A94A87로
나누고 같은 물리 WModel을 공유한다. 각 배치의 atlas scale/bias와 decode scale은 별도 유지한다.
새 모델/텍스처는 Map/CHARACTERSELECTMAP/MAP_FB0D0FFCBE97_BG_GDOGODS_MAGICFLOOR03D_SM_OVR_43CE9773CA87/
아래에 있으며 lighting은 Map/Lighting/CharacterSelect/ 아래다. Drive 전달은 아직 수행하지 않았다.

| 상태 | 실제 확인 내용 |
|---|---|
| 모델 수치 | 984정점/1920인덱스의 기존 P/N/T.xyz/UV0/삼각형 유지, native decode에서 UV1 SHA 일치 |
| 채널 한계 | 원본 packed tangent 부호 미확정, UV 미분 재구성. 비결정 COLOR0 제외; 해당 ring shader는 미소비 |
| 자동 shader 검사 | program 5까지 180/180 PASS. 후속 6/7/8 추가 분기 검증은 진행 중 |
| C++ 빌드 | 다른 작업의 01:59 Debug Product 4프로젝트 PASS가 program 5 연결 코드 포함. 후속 수정은 재빌드 필요 |
| 입력·배포 | 중앙 링 추가 후 catalog 61, material 7, placementLighting 21. Area Validate/Publish 803배치/4파일 PASS |
| 수동 실행·화면 | 이번 중앙 링 연결 이후 에이전트 Client 미실행, 사용자 육안 검증 미실시 |
| 전체 확대 | 기본/PBR/투명/발광/vertex-color 계열 연결 및 최종 검증 진행 중 |
| Git | 사용자 PR/merge/pull 승인. 현재 main 충돌 분석 중이며 아직 commit/PR/merge 하지 않음 |

오래된 WModelGeometryContractHarness 실행 파일은 확장된 MODEL_MATERIAL_DATA와 ABI가 맞지 않아
첫 실행이 access violation으로 실패했다. 현재 헤더로 해당 하네스만 다시 빌드한 뒤 native decode는
exit 0으로 통과했다. 실패 시도와 재빌드를 구분하며, 이를 실제 Client 실행 성공으로 기록하지 않는다.

중앙 ring의 원본·현재 위치 삼각형은 동일하고 실제 형상은 내부가 빈 좁은 띠다. 원작 이미지의 내부
원판·문양 소유 배치는 아직 미확정이며, 이 결과로 임의 Y·가시성·다리 삭제를 수행하지 않았다.
기존 사용자 조명·밝기·카메라 및 앞서 여섯 배치의 재질/조명 입력과 아홉 Y 보정은 보존한다.

## G13. 병합 우선 전환과 렌더링 보류

2026-09-08 사용자가 충돌 해결·PR·merge·pull을 우선하도록 요청했다. 제품은 기존 바닥 6배치와
중앙 링 15배치, program 0~5까지만 유지한다. 전체 맵 계열 6~8의 shader 초안, COLOR0 운반,
미연결 optional catalog 입력은 제품 병합에서 제외하고 `out/GitIntegration20260908/pre_integration/`
및 `out/CharacterSelectFullRestore20260908/`에 보존했다. G12의 전체 확대는 현재 보류 상태다.

program5 병합 후보 4개 shader는 기존 0~5의 180항목과 이전 PBR 상태 누수 18항목을 합쳐
WARP 198/198 PASS였다. 보존된 이전 실행의 공통 180항목은 실제 수치 최대 차이 0이었다.
해당 후보를 제품에 적용하고 static vertex layout은 UV1을 포함한 stride64 계약으로 유지했다.
이는 최종 병합 코드의 C++ 빌드나 사용자 육안 검증을 대신하지 않는다. 통합 뒤 검증은 별도 기록한다.
