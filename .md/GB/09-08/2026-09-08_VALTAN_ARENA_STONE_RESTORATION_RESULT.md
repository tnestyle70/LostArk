# 발탄 전투 아레나 중앙 석재 복원 결과

## G00. 실제 상태

2026-09-08에 시작해 09-09에 이어서 작업했다. 중앙 바닥4와 주변 바위3의 원본 석재
재질·배치색·조명 입력을 기존 CModel/CMaterial에 연결했다. 코드와 저작 문서, Resources는
설치했다. **09-09 사용자 요청에 따라 긴 Product Debug 빌드는 중단했고, 발탄 runtime
데이터 Publish는 완료했다. 중단 시점까지 컴파일 오류는 없었지만 Client 최종 링크는
완료하지 않았다. 다른 세션의 전체 빌드 완료 후 실행해야 한다. 사용자 화면 PASS는 아니다.**

기준 브랜치는 `codex/kouku-ball-motion-effects`다. 다른 작업의 미커밋 변경을 보존했고
commit/PR/merge/Drive 업로드는 수행하지 않았다.

## G01. 적용한 배치와 실제 입력

| 구분 | sourcePlacementId의 package/export | stable placement |
|---|---|---|
| 중앙 바닥 | SL00:1271 | `11789713951910399058` |
| 중앙 바닥 | SL00:1299 | `10806181101101653675` |
| 중앙 바닥 | SL00:1304 | `13791464513326679792` |
| 중앙 바닥 | SL00:1337 | `12311929615219314487` |
| 주변 바위 | SL00:1643 | `12796097868571959148` |
| 주변 바위 | SL04:2950 | `17009176391647657700` |
| 주변 바위 | SL03:4893 | `15038678637319957642` |

package prefix는 모두 `LV_LUT_HEARTRB_ED_`다. 중앙 약(156.28,23.24,-121.98)m을 기준으로
주변 세 바위는 약3.7/21.1/22.8m에 있다. 선택 MIC는 모두
`lv_lut_heartrb.mat.bg_pap_stone_rock04_mi_ksr`다. 기존 WMat 재질 이름은 floor의
`bg_pap_stone_rock04_mi_ksr`와 rock의 `bg_pap_stone_rock02_mi_ksr`로 달라 각각 정확히 매칭했다.

기존 geometry 두 종류에 원본 component별 COLOR0와 UV1·tangent handedness를 보존한
7개의 WModel1.2 variant를 추가했다. 기존 asset/placement를 지우지 않고 선택7배치의 asset
참조만 바꿨다. placement ID, 위치·회전·크기 및 다른 배치는 유지했다. imported catalog는
272→279개, 배치는13,184개 그대로다. MapCatalog의 오래된275/13,186 표기도 실측에 맞췄다.

- 원본 mesh 자체 색 buffer는 비어 있지만 component override에는 floor33,002색,
  rock827색이 존재한다. floor4는 모두 다르고 rock3는2그룹이다.
- A는 모두255이며 R에는 실제 분포가 있다. 원본 BGRA→PS RGBA 변환을 한 번만 수행했다.
- source triangle 대응으로 floor의 완전히 같은 정점을 합쳐 기존과 같은32,940정점/
  33,063인덱스를 보존했다. rock은827정점/3,252인덱스다. 형상·기존 WMat section은 보존했다.
- UModel rock의 비결정적 COLOR0/W와 floor UV1의5개 half-zero 해석 차이를 원본 native
  stream과 대조해 수정했다. 단순히 glTF 정점 번호대로 덮어쓴 작업이 아니다.

근거: [geometry](../../../out/ValtanStoneRestore20260908/geometry_handoff.json),
[선택 배치](../../../out/ValtanStoneRestore20260908/selection.json),
[정점색·기준축 원본 대조](../../../out/ValtanStoneRestore20260908/vertex_color_and_light_guid_findings.md).

## G02. 재질과 조명 계산

새 enum `SOURCE_OVERLAY_OPAQUE=7`, mapmaterials family `bg_base_opa_overlay`를 추가했다.
이것은 확인한 `bg_base_opa` 분기이며 모든 같은 부모 재질을 지원한다는 뜻이 아니다.
ORM/PBR/BRDF LUT를 임의로 연결하지 않았다. 기본 D/N과 overlay D/N 네 장 및 원본 상수로
돌무늬를 섞고, native specular RGB와 power60을 사용한다.

주요 원본 값은 normal1.5, diffuse saturation0.2/brightness0.8, overlay tiling2.5,
brightness1.2/saturation1.3, specular intensity0.2, overlay specular intensity0이다.
정점 R은 nonlinear coverage에 들어가며 diffuse alpha는 opacity clip이 아니다.

원본 asm 레지스터를 재검산해 앞선 해석을 정정했다. Base/Direct/Baked 모두
`overlayD.a² * (1-saturate(baseNormal.z²*D.a))`를 coverage 입력으로 쓴다. Base/Baked는
mixed normal을 정규화하지만 Direct diffuse는 미정규화 mixed normal, Direct specular는
base normal을 사용한다. 기존8 MRT의 RT6/7을 marker7에 한해 재사용해 이 차이를 보존했다.
Base의 구운 diffuse·specular와 Direct의 완성 radiance를 따로 더해 albedo를 중복 곱하지 않는다.

이번 조명 입력은 SL00 pair55, SL03 pair50, SL04 pair58의 **평균색/방향 DDS3쌍**이다.
원본 정적 lightmap420개를 새로 굽거나 동적 광원420개를 배치한 것이 아니다. UV1과 배치별
atlas scale/bias, RGB coefficient를 원본대로 연결했다. 기존22 Point의 원본 두 GUID와
선택7배치 baked GUID를 raw export에서 다시 비교했고 교집합은0이다. 공통 ambient는 이
분기에 중복 추가하지 않는다. 기존 scene profile의 직접광·노출을 이 작업에서 튜닝하지 않았다.

원본 hemisphere/scene global color·shadow texture/lightfunction, 정확한 scene 활성 조건은
미복원이며 shader adapter에 해당 경계를 명시했다. diffuse/overlayD의 SRGB 태그는
직접 선언이 없어 기존 inherited-default 해석을 사용한다. normal의 linear는 원본 명시값이다.

## G03. 코드·저작·배포 경로

- Engine `ModelAssetData.h`, `Material.h/.cpp`, `Model.cpp`: 명시 overlay 입력과 SRV,
  static COLOR0/tangent 및 baked UV1 검증. 실패한 필수 입력을 흰색으로 대체하지 않는다.
- Engine `Engine_Struct.h`, `Mesh.cpp`: CPU에만 보존되던 정점색을 실제 GPU 입력으로
  전달한다. VTXMESH stride68/COLOR0 offset64, 일반 layout7항목·instance layout18항목이다.
  색상이 없는 기존 모델은 흰색 기본값을 유지하고 WModel wire format과 instance payload는
  변경하지 않았다. Engine/Client 전체 재빌드가 필요한 public layout 변경이다.
- Client `MapAssetCatalog.cpp`, `MapAssetRenderUtils.cpp`: field/경로/색공간 검증,
  선택 프로그램7과 네 SRV 및 배치별 조명 바인딩. MainApp 재질 진단에 계열명을 표시한다.
- Engine `Shader_SourceStoneSurface.hlsli`가 순수 공용 수식을 소유하고 Client의
  `Shader_MapMaterialSurface.hlsli`, static/instanced shader와 Engine deferred가 소비한다.
  새 include와 Data 문서는 기존 프로젝트/filters에 필요한 항목만 등록했다.
- Engine `Renderer.cpp`와 deferred: SSAO/FINAL에서 RT6/7 SRV를 명시적으로 연결하고
  marker7의 실제 normal과 HDR 입력으로 진단한다. static/instanced shadow의 기존 masked
  fallback에서 program7을 제외해 D alpha0도 석재 그림자가 유지되도록 했다.
  이 분기의 `Direct specular` 진단 화면은 기존 SpecularTarget에 담긴 **직접광 diffuse와
  specular 합계**다. 순수 반짝임만으로 해석하지 않는다. Reflection/ORM 진단은 이 입력이
  없는 계열임을 표시한다.
- 정본은 `Data/Maps/Imported/LV_LUT_HEARTRB_ED/*.mapassets`,
  `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/*.mapplacements/*.mapmaterials.json`, MapCatalog다.
  runtime은 기존 `Publish-MapAuthoring.ps1`의 검증·배포만 사용한다.

별도 모델 런타임, 새 render target, Server/gameplay·Deploy 파괴 동작, 캐릭터·이펙트 변경은
이 작업에 추가하지 않았다. 필수 Resources와 새 enum reader를 함께 배포해야 한다.

## G04. 추가 Resources와 Drive 공유

모두 **신규 추가17파일 / 14,790,364bytes**이며 기존 Resources 파일 교체는0이다.
다음 두 폴더를 그대로 공유하면 이번에 추가한 입력을 포함한다.

| Resources 상대 폴더 | 실제 추가 |
|---|---|
| `Map/LV_LUT_HEARTRB_ED/SourceStoneRestore/` | Geometry 하위 WModel7개, Textures 하위 DDS4개 |
| `Map/Lighting/Valtan/` | 평균색/방향 lightmap DDS6개 |

표면 DDS는 `bg_pap_stone_rock02_d_ksr`, `bg_pap_stone_rock02_n_ksr`,
`bg_rad_drlands_tile_floor02a_d_ksr`, `bg_rad_drlands_tile_floor02_n_ksr`다.
RNM 이름에는 SL00/SL03/SL04 prefix가 있어 같은 leaf 이름의 package 충돌을 피한다.

최상위 압축 블록은 원본 UModel 추출물과 그대로 유지했다. 하위9/11단계 mip는 기존 CS
adapter의 색공간별 BOX+separate alpha로 **프로젝트에서 생성**했으며 원본 lower mip
복원으로 표시하지 않는다. 줄어든 화면의 sampling 입력을 확보한 것이며 AA 전체 해결이나
사용자 화면 품질 PASS를 뜻하지 않는다. ZIP 생성/Drive 전송은 하지 않았다.

정확한17경로: [설치 목록](../../../out/ValtanStoneRestore20260908/installed_authoring.json).

## G05. 검증과 남은 단계

완료:

- 7개 geometry CPU topology/채널 oracle, native component COLOR0/UV1/W 대조.
- 10개 DDS 전체 mip와 최상위 압축 블록 보존, 설치파일17개 확인.
- map publisher `-Mode Validate` 성공:279assets /13,184placements /7배포문서.
- 09-09 `-Mode Publish` 성공: 같은 범위의 runtime 문서7개 배포. 실행 중인 Client/Server는
  없는 상태에서 수행했다. 다음 전체 빌드가 이번 재질/placement 문서를 읽을 수 있도록 준비했다.
- 변경 C++7파일 `/Zs` 컴파일 성공: Material/Model/Mesh/Renderer/MapAssetCatalog/
  MapAssetRenderUtils/MainApp. 마지막 GPU layout 및 SRV 연결 수정 이후 다시 검사했다.
  기존 C4819 codepage 경고는 별도이며 Product 링크 검증은 아니다.
- static/instanced/deferred/animated shader4종 FXC 컴파일 성공.
- WARP 수치 검사291/291 통과. 기존198건, 원본 DXBC 직접 대조36건(최대 절대오차
  1.43e-6), marker7 전달48건, opaque shadow2건, SSAO1건, FINAL 진단6건이다.
  synthetic geometry/입력으로 검사한 수식·전달 증거이며 실제 CModel GPU 업로드 또는
  사용자 화면 검증을 대신하지 않는다. FP16 HDR 출력은 채널별 표현 간격을 기준으로 비교했다.
- 실제 CModel/SRV/GPU 버퍼 읽기 검사는 out 프로브로 준비했으나 아직 실행하지 않았다.
- 프로젝트/XML4개 parse, 변경 범위 `git diff --check` 성공.

09-09 Product 빌드에서는 Engine/Shared/Server 빌드가 끝났고 Client shader 컴파일 이후
Client C++ 컴파일까지 진행했다. 사용자가 빌드 대기를 종료하도록 요청해 root 소유 runner
PID53988의 빌드 자식만 중단했다. 다른 세션 프로세스는 중단하지 않았다. 기록된 컴파일
오류는0이며 경고는 존재한다. 전체 Product PASS나 Client 최종 링크 성공으로 기록하지 않는다.
로그는 `out/ValtanStoneRestore20260908/product_build_apply.log.txt`와 `publish_apply.log.txt`다.

전체 worktree `git diff --check`에는 다른 작업의
`ArenaCameraProfile.cpp:53`, `Effect_DocumentRenderer.cpp:20290` trailing whitespace가 남아
전체 PASS로 기록하지 않았다. 해당 파일을 임의로 정리하지 않았다.

남음:

- 다른 세션에서 Product Debug 빌드·Client 최종 링크 및 EXE/DLL/shader 정상 배포.
- 새 Engine/Client 산출물이 일치하는 상태에서 실제 CModel/SRV 바인딩 검사. 이번 사용자
  중단 요청에 따라 추가 native 프로브 빌드·실행도 보류했다.
- 사용자가 Server + Client를 직접 시작해 Lobby → Valtan 중앙 바닥·주변 바위 비교.
  F1 Rendering Workbench의 Floor Materials source/legacy A/B에서 같은 시점·광원 상태로 확인.
- 나머지 석재·하층·Deploy 바닥/난간·파편, 안개/전체 조명·원본 scene 환경은 후속 적용 범위.

중앙에는 기존 Deploy 바닥이 함께 있으므로 같은 pivot만으로 어떤 triangle이 화면을
차지하는지 단정하지 않았다. 바닥 전체가 이7배치로 완성됐다는 선언이나 사용자 대신
시각 유사도 판정은 하지 않는다.

후속 실행 순서는 Product Debug 빌드 완료 → fresh DLL/CSO로 실제 모델 입력 검사 또는
사용자 실제 아레나 확인이다. map publisher Publish는 이미 완료했으며 저작 수정이 없다면
반복 배포하지 않는다.
프로브 준비 위치는 `out/ValtanStoneRestore20260908/catalog_probe_handoff.md`, shader 검증
기록은 같은 폴더의 `shader_verification_receipt.json`이다. 빌드 중단으로 SDK와 일부 산출물은
갱신됐지만 Client 최종 링크/정상 배포는 끝나지 않았다. 현재 디렉터리의 기존 EXE를 실행
가능한 완성본으로 안내하지 않는다. 291건은 앞선 발탄 shader checkpoint의 검증이며,
이번 전체 빌드 시작 전에 다른 세션이 수정한 WR 이펙트의 새 동작까지 검증한 수치가 아니다.
