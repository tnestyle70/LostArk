# 쿠크 보스·컷신·소품 원본 재질 구현 결과

## G00. 반영 상태

`BossCatalog.json`의 optional `modelMaterialOverrides`에 Resources-relative 모델 33개, 재질 slot 50개를 연결했다. 전투 쿠크·세이튼, 컷신 세이튼, 망치와 공, 카드 미로 몬스터, 마리오 공 변형이 기존 `CModel → CMaterial → SourceCharacter Base/Light` 경로를 소비한다. 기존 공의 명시적 World Object `materialProfile`은 같은 slot에서 최종 우선권을 유지한다.

작업 시작 `before` 사본 대비 기존 program 1~21의 Base/Light 함수 42개는 양쪽 HLSLI에서 각각 동일하고, 기존 parameter packing 21분기도 동일하다. BossCatalog에는 이전 root override가 0개였으며 새 50행을 제외한 root·boss entry는 JSON semantic이 전부 같다. 증거는 `out/KoukuBossNative20260911/baseline_preservation.json`이다. 사용자가 승인한 두 재질의 정확한 이름을 임의로 추정하지 않고 이 범위에서 실제 보존된 기준을 기록한다.

Client/UI를 실행하거나 캡처하지 않았다. 원작과의 최종 시각적 일치 여부는 사용자 화면 확인 전이다. 환경광·RNM·맵 표면·베른 크기는 통합 작업의 별도 범위다.

## G01. 원본 입력의 정체

원본 UPK의 MIC 상속, static shader map, uniform expression, texture parameter를 직접 읽었다. 36개 MIC 전부에서 원본 shader map과 texture 입력을 해석했으며 closure 오류와 기본값 충돌은 0건이다. 고유 native shader 42개에서 Base/Light 조합 13개를 확인했다. 기존 program 14·21을 공유하고 새 program 22~32를 추가했다. 공의 profile을 다른 재질에 일괄 복사하지 않았다.

| program | 원본 분기와 적용 대상 예 |
|---|---|
| 14 | 기존 weapon emissive |
| 21 | 기존 monster PBR masked: 공 `mn_rhcn_00_mi`, 세이튼 `05-1_loc_int`·`05-2`, 큰 망치 `WP_MN_RPCT_06` |
| 22 | legacy monster masked: `IT_GSTFP_00` |
| 23 | 카드 미로 `MN_CDMD`의 skin/emissive |
| 24·25 | 카드 미로 `MN_PPCH_00-1`·`MN_PPCH_00` |
| 26 | 공통 PBR masked: 쿠크 `MN_RPCZ`, 세이튼 05 body·05-3, 컷신 parts, 마리오 색상 공 등 |
| 27·28 | `MN_PPCT_00_1`·`MN_PPCT_00_2` |
| 29 | `MN_PPCT` hair translucent |
| 30 | PBR opaque: 컷신 `MN_RPCT_01` import material, 해골 공 `MN_RHCN_01` 등 |
| 31 | 컷신 `MN_RPCT_00`·03 body의 skin |
| 32 | 뿅망치 `wp_mn_rhkp_07_mi_dead`; 원본 effective `dead=0` |

컷신 `MN_RPCT_00.wmodel`의 세 번째 재질은 같은 이름의 로컬 후보가 아니라 native SkeletalMesh material array가 참조하는 `mn_rpct_01.mat.mn_rpct_01_mi`다. 원본 array의 import reference까지 따라 이 차이를 반영했다. `MN_RPCT_03`도 같은 방식으로 import를 확인했다.

광대 변신 `MN_RPCZ_00-1.wmodel`은 WModel materialName이 `mn_rpcz_00_mi`로 남아 있지만 실제 source MIC는 `mn_rpcz_00.mat.mn_rpcz_00-1_mi`다. variant diffuse/specular를 반영했다. 마리오 RedStar·Striped·Yellow·Blue 공도 각각의 `mn_ppcc_00` MIC를 사용한다.

## G02. 실제 소비 경로

`Client/Private/ActorCatalog.cpp`의 공통 parser는 character owner 제한을 유지하면서 BossCatalog root의 공유 모델 override를 stage한다. `Build_ModelLoadDescription`은 정확한 Resources-relative 모델 ID로 override를 descriptor에 복사하며 실패 시 호출자의 기존 출력은 보존한다.

| 생성 소비자 | 새 연결 |
|---|---|
| `CKoukuSaydonPresentationAssetService::Ensure_Prototypes` | 보스 body·분리 weapon의 shared descriptor |
| `Ensure_ClownBodyPrototype` | 변신 광대 모델의 variant descriptor |
| `CDeployPropRuntime::Ensure_AreaPrototypes` | 컷신 animated body와 intact/fractured 모델 descriptor |
| `CWorldSequencePlayer::Prepare_ObjectResources` | 공·망치 등 모델 descriptor 후 기존 명시 profile 우선 적용 |
| `CMonsterPresentationAssetService::Ensure_Prototypes` | 카드 미로 몬스터의 descriptor |

컷신 `CDeployPropObject::Render_Animated`와 보스 `CNpc`는 이미 공통 `Bind_DeferredMaterialInputs → CModel::Bind_SourceCharacter`를 사용한다. 정적 World Object는 `CMapAssetRenderUtils::Bind_Material`에서 같은 source input을 소비한다. gameplay actor, animation skeleton, boss 판정 경로는 교체하지 않았다.

`SourceCharacterMaterialParameters.h`와 양쪽 `Shader_SourceCharacterPrograms.hlsli`에 source pair와 strict named parameter packing을 추가했다. `Shader_SourceCharacterMaterial.hlsli`에는 program 22의 legacy 입력 및 program 29의 원본 hair fog/UV/coverage 입력 분기를 연결했다. `Model.cpp`의 native program 검증 상한은 통합 담당의 source.map 33~37까지 포함한다.

program 29의 hair는 기존 character hair와 같은 ordered coverage를 사용한다. 원작 sorted translucency는 아직 이 character draw 경로에 없으므로 투명 표현까지 완전히 동일하다고 판정하지 않는다. 이 원본 PS는 UV0만 사용하고 direct shadow를 이미 소비하므로 기존 UV1 요구 및 별도 hair shadowAdapter 목록에는 추가하지 않았다.

`Publish-GameplayBalance.ps1`과 `valtan_presentation_generation.py`의 BossCatalog root 검사도 새 optional property를 허용한다. 기존 root와 기존 boss row 계약은 유지한다. 새 production C++ 파일은 없으므로 이 범위의 project/filter 항목 추가는 없다.

마리오 공 5개 map row는 `out/KoukuBossNative20260911/mario_map_material_rows.json`으로 맵 담당에게 전달했다. MapAssetCatalog에서 동일 native family/parameter/texture를 소비하며 별도 material runtime을 추가하지 않는다.

## G03. Resources 물리 입력

122개 source texture를 원본 object 경로로 headless export하고 다음 위치에 DDS로 설치했다.

`Client/Bin/Resources/Character/SourceMaterials/Kouku/<logical-package>/<texture>.dds`

합계는 120,661,752 bytes다. 원본 sRGB property와 linear normal/lookup 입력을 유지했다. 원본 compressed top mip block은 보존하고 필요한 하위 mip는 기존 texconv의 BOX·separate-alpha 방식으로 생성했다. 생성한 하위 mip를 원본에서 복구한 mip라고 취급하지 않는다. 해당 Resources는 Git index에 추가하지 않았다.

정확한 source object, Resources-relative ID, 색 공간, 설치 byte/hash와 mip 생성 근거는 `out/KoukuBossNative20260911/resource_inputs.json`에 기록했다. 이 파일은 작업 증거이며 별도 runtime manifest 계약이 아니다.

## G04. 수행한 검증

| 검사 | 실행 결과 |
|---|---|
| 원본 MIC closure | 36/36 성공, 오류·default conflict 0 |
| 실제 C++ ActorCatalog load·descriptor | 모델 33개·override 50개, 실패 0 |
| 최신 Product CModel create → clone → native bind | 모델 33개·mesh 50개 전부 성공, 실패 0·exit 0 |
| descriptor 경로 실패 보존 | `../outside.wmodel`, 절대 경로 거부 및 기존 출력 보존 |
| 실제 C++ parameter packing | 50행 texture mask 정확 일치, finite·missing·extra·실패 시 기존값 보존 검사 실패 0 |
| 통합 source.map forward packing | 14행 동일 strict 검사 실패 0; 통합 담당용 `base.bin`·manifest 생성 |
| native DXBC 대 production HLSL 수치 비교 | 13 pair × Base/Light × view 3종 × IBL 3단계 = 234 case, 239,616 pixel |
| 위 비교 finite·오차 | nonfinite 0, 상대 허용 오차 1e-3 초과 0 |
| BossCatalog JSON parse | 50행·모델 ID 33개 확인 |
| publisher 문법 | PowerShell AST 및 Python AST 성공 |
| Resources 존재 검사 | 122/122 존재 |
| 변경 범위 `git diff --check` | 성공 |

WARP 비교는 게임 창이나 캡처 없이 수치 RT를 읽었다. 원본 mask variation·flat black·상태 효과 입력을 반영해 정상 body/skin/emissive/hair 분기가 평가되는 fixture를 사용했다. PBR program은 IBL 변화에 응답하고 legacy program 22는 원본과 같이 Base가 0이며 direct light에 응답했다. 이 fixture 검사는 모든 실제 장면·시점의 visual fidelity 판정을 대신하지 않는다.

FXC는 일부 native branch의 잠재적 미초기화 및 최적화된 division 경고를 출력했다. 평가한 234 case에서는 비유한 값이 없었다. 관련 증거는 `out/KoukuBossNative20260911/gpu_results.csv`, `gpu_run.log`, `packed_manifest.csv`와 probe 실행 파일이다.

## G05. 실제 CModel 소비 확인

최종 Product 빌드 산출물(out/BuildPipeline/runs/20260910T185256023Z-debug-product.json)의 Engine DLL과 Shader CSO로 catalog_probe.exe <repo> --models를 실행했다. 실제 decoder의 hasSkeleton을 사용하고 CModel create → clone → prototype 해제 → 모든 mesh의 native surface → Bind_SourceCharacter를 검사했다. **models=33,overrideRows=50,boundMeshes=50,failures=0,exit0**이다. 전체 결과는 out/KoukuBossNative20260911/catalog_probe_final.log다. CSO는 exe 인접 사본을 사용했고 Client·swapchain·UI는 생성하지 않았다. 이 검사는 최종 Debug shader 크기 조정 이후의 실제 CShader 생성과 native material 바인딩도 포함한다.

사용자는 통합 배포 후 쿠크 전투·컷신, 세이튼, 뿅망치, 월드 공과 마리오 색상 공을 직접 확인한다. 현재 문서는 사용자 화면 확인을 PASS로 기록하지 않는다.
