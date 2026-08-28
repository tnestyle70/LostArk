# Character Select 중앙 회귀·모서리 표면 일렁임 수정 구현 계획서

- 갱신일: 2026-08-28
- 문서 종류: 구현 계획서. 2026-08-28 사용자 승인 뒤 여섯 겹침 쌍을 다섯 placement의 최소 Y 교정으로 분리하고, 안전한 visual publisher 계약과 focused 검증까지 반영했다. asset·shader·near/far는 변경하지 않았고 Client를 실행하지 않았다.
- 같은 Area의 기존 작업이므로 08-09 파일을 갱신한다. [기존 중앙부 RESULT](C:/Users/user/Desktop/LostArk/.md/GB/08-09/2026-08-09_CHARACTER_SELECT_CENTER_DEPTH_SEPARATION_RESULT.md)의 완료 이력과 이번 모서리 재발을 구분한다.
- 베른의 덩어리 단위 소멸은 [베른 프러스텀 컬링 계획](C:/Users/user/Desktop/LostArk/.md/GB/08-25/2026-08-25_BERN_FRUSTUM_CULLING_DIAGNOSTIC_AND_STABILITY_PLAN.md)에서 다룬다.
- 베른 core 수학 수정과 Character Select 배치 교정은 서로 다른 원인과 변경 단위로 유지한다. Character Select는 승인된 여섯 geometry 겹침만 교정했으며 새 ImGui 진단 패널은 제품에 반영하지 않았다.

## G00. 이미 수정된 중앙부와 새 모서리 후보를 구분한다

### 2026-08-28 교정 전 진단 체크포인트

[surface 진단 test](C:/Users/user/Desktop/LostArk/Tools/MapPipeline/test_map_surface_depth_contract.py)의 합성 21 tests와 실제 resource를 읽는 geometry CLI가 통과했다. 803개 stable placement/source ID, 제품 scope 779 placement / 54 asset, 기존 490/495의 보정을 보존했고 여섯 후보 쌍의 교차 geometry를 다시 확인했다. 수치와 실행 증거는 [RESULT의 이번 검증](C:/Users/user/Desktop/LostArk/.md/GB/08-09/2026-08-09_CHARACTER_SELECT_CENTER_DEPTH_SEPARATION_RESULT.md)에 기록한다.

카메라 JSON은 아직 없으며 결과의 `cameraDepthStatus`는 `not_requested`다. 이 문단은 교정 전 진단 상태를 보존한다. 이후의 실제 배치 반영과 검증은 G01·G02 및 RESULT의 최신 섹션을 따른다. 실제 camera-depth와 visual PASS는 여전히 확보하지 않았다.

### 목표와 원인 구분

사용자가 관찰한 모서리의 표면 일렁임을 제거하되, 서로 다른 바닥 조각과 material layer, 원래 외곽 형태를 보존한다. 기존 중앙부 2mm 보정은 다시 적용하지 않는다.

텍스처 자체에 독립적인 월드 Y가 있는 것은 아니다. 텍스처가 입혀진 두 삼각형 표면이 거의 같은 깊이를 기록하면 camera 이동에 따라 보이는 표면이 번갈아 바뀔 수 있다. 이것이 z-fighting 후보이고, 프러스텀 컬링으로 객체 draw 전체가 생략되는 현상과는 구분한다. Coplanar geometry와 depth bias의 관계는 [Microsoft Depth Bias 문서](https://learn.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-output-merger-stage-depth-bias)를 참고한다.

또한 **한 면만 남겨도 무늬가 반짝이는 현상**은 texture 축소 sampling 문제일 수 있다. 이번 조사에서 거의 같은 높이의 겹침과 mip 부재가 모두 확인됐지만, 어느 후보가 사용자가 본 모서리인지는 아직 대응시키지 못했다. 두 원인을 한꺼번에 확정하거나 같은 처방으로 처리하지 않는다.

### 현재 데이터 정본과 제품 소비 경로

| 대상 | 현재 실측 |
|---|---|
| Area | `LV_LOBBY_CLASSSELECT_SL00` |
| [Imported 배치](C:/Users/user/Desktop/LostArk/Data/Maps/Imported/LV_LOBBY_CLASSSELECT_SL00/LV_LOBBY_CLASSSELECT_SL00.mapplacements) | 추출 보존본, 803 placement |
| [Authoring 배치](C:/Users/user/Desktop/LostArk/Data/Maps/Authoring/LV_LOBBY_CLASSSELECT_SL00/LV_LOBBY_CLASSSELECT_SL00.mapplacements) | 수정 정본, 803 placement |
| [runtime 배치](C:/Users/user/Desktop/LostArk/Client/Bin/DataFiles/Map/LV_LOBBY_CLASSSELECT_SL00.mapplacements) | publisher 생성물, Authoring과 현재 byte-identical |
| catalog / render profile | 55 asset, OpaqueBack·DEFERRED |
| 제품 map scope | X `[-792,-750]`, Z `[158,218]`의 placement origin 범위, 실제 779 placement / 54 asset |
| 기존 중앙 보정 | export 490 Y `-142.713572`, export 495 Y `-142.72116` 유지 |
| 완전히 같은 asset+transform+visible 중복 | export 307/308은 원점에 있고 제품 scope 밖. 이번 제품 모서리 증상의 근거로 삭제하지 않음 |

제품 opaque map은 다음 경로를 사용한다.

`CMapPlacementRuntime::Is_BatchEligible → CMapStaticBatchObject → CMapAssetRenderUtils::Bind_Material / Select_Pass → CModel::Render_Instanced → Shader_VtxMeshMapInstance.hlsl`

fallback은 `CMapAssetObject → Shader_VtxMeshBinary.hlsl`이다. Binary shader만 바꾸고 실제 instance 경로는 그대로 두는 수정은 하지 않는다. 이번 기본안은 shader·Engine depth format을 바꾸지 않고 원인 배치만 수정한다.

### 현재 cooked geometry에서 확인한 후보

기존 `parse_legacy_wmodel` decoder로 실제 wmodel vertex를 읽고 모델 단위 변환·placement S×R×T를 적용했다. 이번 surface CLI에서 같은 XZ 점이 두 삼각형 내부에 있는지와 보간한 월드 Y의 차이를 다시 확인했다.

아래 export 표기는 모두 stable source ID `LV_LOBBY_CLASSSELECT_SL00:export:<번호>`를 뜻한다. 삼각형 번호는 0부터 시작하는 **이번 수치 검사 위치**이며 저장용 placement ID를 대신하지 않는다.

| source export 쌍 | mesh / triangle 쌍 | 검사 XZ(m) | 표면 Y 차이(m) | stable placement ID 쌍 |
|---|---|---|---|---|
| 402 / 405 | mesh 1, 736 / 987 | -762.85, 197.60 | 약 `4.03e-8` | `14467830425018779524` / `18313157716296743356` |
| 458 / 474 | mesh 0, 28 / 129 | -773.95, 197.30 | 약 `2.63e-8` | `17033911184007117021` / `9567686591551344922` |
| 442 / 458 | mesh 0, 234 / 359 | -772.00, 195.20 | 약 `5.85e-5` | `13004387245150734382` / `17033911184007117021` |
| 425 / 427 | mesh 0, 81 / 21 | -770.20, 187.70 | 약 `4.86e-5` | `15241669838743709595` / `11064700097567933927` |
| 435 / 436 | mesh 0, 82 / 21 | -762.10, 199.25 | 약 `4.88e-5` | `11799140389379220100` / `11123722243846476314` |
| 469 / 471 | mesh 0, 178 / 244 | -778.60, 198.20 | 약 `1.48e-7` | `11531784870313684855` / `12265842867223881631` |

| export | asset ID |
|---|---|
| 402/405 | `MAP_F442BCF81552_LV_ELG_ARYANORB_FLOOR02_SM_01` |
| 442/458/474 | `MAP_AC527A4AF171_BG_ELG_ARYANORB_BRIDGE01E_SM` |
| 425/427/435/436 | `MAP_E5357DD78673_BG_ELG_ARYANORB_FLOOR22_SM` |
| 469/471 | `MAP_FB0D0FFCBE97_BG_GDOGODS_MAGICFLOOR03D_SM` |

이는 실제 겹침의 수치 증거이지만, 모든 모서리를 전수 검사했다거나 해당 면이 화면에서 다른 geometry에 가려지지 않는다는 증거는 아니다. CLI는 지정한 여섯 placement 쌍에만 1m spatial bin과 XZ AABB 필터를 적용하고, 높이 범위 `[-143.3,-142.4]m`에서 삼각형 교차 polygon을 계산한다. G02의 남은 작업은 사용자 발생 위치 대응과 실제 카메라에서의 depth 여유·인접 seam 확인이다.

### depth와 texture의 별도 기준

- [Level_CharacterSelect.cpp](C:/Users/user/Desktop/LostArk/Client/Private/Level_CharacterSelect.cpp)의 camera는 near 0.1m / far 2000m다.
- [Graphic_Device.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Graphic_Device.cpp)는 `D24_UNORM_S8_UINT` depth를 사용한다. [Engine_Shader_Defines.hlsli](C:/Users/user/Desktop/LostArk/Engine/Bin/ShaderFiles/Engine_Shader_Defines.hlsli)의 기본 opaque depth는 `less_equal`, write enabled다.
- 현재 일반 perspective에서 view-space 거리 z의 D24 한 단계는 대략 `z² × (far-near) / (far × near × (2^24-1))`이다. z=10m에서 0.060mm, 50m에서 1.49mm, 100m에서 5.96mm다. 이는 투영식 계산값이며 화면 실측값이 아니다. [Microsoft XMMatrixPerspectiveFovLH](https://learn.microsoft.com/en-us/windows/win32/api/directxmath/nf-directxmath-xmmatrixperspectivefovlh)
- 월드 Y 간격은 view-depth 간격과 다르다. 따라서 모든 쌍에 2mm를 넣거나 near/far만 바꾸는 처방은 사용하지 않는다.
- [Character Select Resources](C:/Users/user/Desktop/LostArk/Client/Bin/Resources/Map/CHARACTERSELECTMAP) 아래 153 DDS의 header mip count가 모두 0이다. 대표 FLOOR22 diffuse는 1024×1024 DXT1, 파일 524,416 byte로 header 128 byte와 최상위 level 524,288 byte만 있다.
- [Material.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Material.cpp)의 device-only `CreateDDSTextureFromFileEx` 경로에는 context를 통한 GenerateMips 호출이 없다. 현재 해당 diffuse는 1 level이며 0개 texture라는 뜻이 아니다. 기본 sampler는 `MIN_MAG_MIP_LINEAR`다.

G00 종료 증거는 기존 중앙 보정 유지, 제품 scope와 실제 배치 식별, 새 겹침 후보·mip 상태 기록이다. 시각 결함의 최종 발생 쌍 확정은 사용자 확인과 분리한다.

## G01. 배치를 안전하게 검증·publish할 경로를 닫는다

이 G는 2026-08-28 구현·실행했다. 세 mode가 같은 expected byte set을 소비하며, Character Select 정본에 `Validate → Publish → Check`를 순서대로 적용했다.

### 구현 전 publisher 공백과 해결

[Publish-MapAuthoring.ps1](C:/Users/user/Desktop/LostArk/Tools/MapPipeline/Publish-MapAuthoring.ps1)은 기존에 `Validate`/`Check`가 없고 곧바로 publish했다. 이번 변경에서 `-Mode Validate|Check|Publish`를 추가하고 생략 시 기존 호환 `Publish`를 유지했다.

기존 single-catalog 분기가 건너뛰던 row parse, placement/source ID 중복, catalog asset 참조와 Resources-relative model path 검증을 single/shard 공통 stage에 연결했다. 새 transaction을 만들지 않고 기존 `Invoke-FileSetTransaction`은 Publish에서만 재사용한다.

### 변경 위치·책임과 실패 계약

| 수정 파일 | 기준점 | 변경 책임 |
|---|---|---|
| [Publish-MapAuthoring.ps1](C:/Users/user/Desktop/LostArk/Tools/MapPipeline/Publish-MapAuthoring.ps1) | param, `Read-PlacementDocument`, `Parse-PlacementRow`, single/shard 분기 | 두 분기의 동일한 row·ID·asset 검증과 실행 mode |
| 같은 publisher | `Invoke-FileSetTransaction` 직전과 runtimeRoot directory 생성 위치 | 검증·stage와 파일 교체 분리, 기존 rollback 유지 |
| [test_map_effect_presentation_contract.py](C:/Users/user/Desktop/LostArk/Tools/MapPipeline/test_map_effect_presentation_contract.py) | `Fixture`, `publish`, `snapshot_runtime` | 실제 publisher를 temp repository에서 실행해 성공·거부·무변경·rollback 검증 |
| [Map pipeline README](C:/Users/user/Desktop/LostArk/Tools/MapPipeline/README.md) | publisher 사용법 | 실제 추가한 mode의 의미와 명령만 반영 |

새 mode는 `Validate / Check / Publish`이며 기존 호출 호환을 위해 기본값은 Publish다.

- **Validate:** source/catalog/placement와 해당 Area의 optional 문서를 parse → validate → 메모리 stage한다. runtime 파일·디렉터리를 만들거나 바꾸지 않는다.
- **Check:** Validate와 같은 expected file set을 생성해 현재 runtime과 byte 비교한다. 누락·차이는 실패이고 고치지는 않는다.
- **Publish:** 동일 validation 성공 후 기존 `Invoke-FileSetTransaction`으로 전체 파일 묶음을 교체한다. 중간 실패는 기존 파일 전부 복구한다.

공통 검증은 version/Area/count, uint64 placement ID 범위·중복, source ID 중복, 존재하는 asset ID 참조, finite SRT, 유효 quaternion, 0이 아닌 signed scale을 포함한다. 음수 scale 자체는 정상 입력이다. catalog의 runtime resource 경로에는 절대 경로·drive-qualified·root 탈출을 허용하지 않는다. double 값이 finite여도 runtime float 변환이 overflow하면 거부한다.

디렉터리 생성과 runtime hash 조회도 Publish branch 안으로 옮긴다. 실패한 Validate/Check가 빈 runtime directory를 남기면 무변경 계약을 통과한 것이 아니다. single뿐 아니라 shard output과 선언된 optional layer도 같은 expected file set에 포함한다. Check/Publish의 소유 범위는 **현재 Area가 선언한 expected output만**으로 한정한다. 현재 publisher처럼 다른 Area 파일과 과거 미참조 파일은 보존하며, stale output 자동 삭제는 이번 수정에 추가하지 않는다.

현재 effect publisher test fixture의 catalog는 asset 0개이고 placement row도 완전한 SRT/visible 형식이 아니다. strict single validation을 도입할 때 **fixture를 유효한 최소 catalog·placement로 고친다**. 기존 6 tests를 통과시키려고 parser를 느슨하게 만들지 않는다.

### 검증과 종료 증거

temp fixture에서 정상 single/shard publish, 잘못된 version/Area/ID/asset/path, 중복, NaN/overflow/zero scale 거부를 검사한다. `FailureAfterPromote`를 첫 파일과 중간 파일 교체 후에 주입해 기존 runtime bytes와 파일 집합 복구를 검증한다. 공유 worktree에 실패 입력을 만들지 않는다.

기존 Map Effect 6 tests를 포함한 publisher 14 tests가 통과했고, single/shard mode, 잘못된 입력과 model path, Validate/Check 무변경, 첫/중간 promote 실패 rollback을 검사했다. 실제 Character Select에서도 publish 전 Check가 불일치를 검출하면서 runtime bytes를 유지했고, Validate/Publish/Check가 차례로 exit 0이었다. 별도 manifest·hash publish 체계는 만들지 않았다.

## G02. 발생 표면을 확인하고 필요한 깊이 교정만 반영한다

surface test와 실제 geometry 진단은 구현·실행했다. 이후 사용자가 확인한 여섯 후보 쌍의 작은 높이 교정을 승인했으므로, 아래 다섯 배치만 먼저 분리한다. 실제 camera-depth와 시각 판정은 이번 데이터 교정의 완료 주장에 포함하지 않는다.

### 2026-08-28 사용자 승인 후 반영 범위

실제 삼각형 교차 영역을 2/3/4mm 하향 이동으로 비교했다. 2mm에서도 기존 여섯 근접 평면 쌍을 분리하며, 더 큰 이동은 경사진 인접 면의 기존 위아래 관계를 더 많이 바꾼다. 추가로 458만 2mm 위로 올리는 대안을 비교해 기존 중앙 495와의 새 국소 교차를 피하는 방향을 택했다. 따라서 네 배치는 2mm 아래로, 458만 2mm 위로 조정한다. 같은 Y와 가까운 원점만으로 추린 258쌍 전체에는 보정을 확대하지 않는다.

| source export | 교정 전 Y(m) | 교정 후 Y(m) | 겹침 관계 |
|---|---:|---:|---|
| 405 | -142.826436 | -142.828436 | 402 아래 |
| 427 | -142.775361 | -142.777361 | 425 아래 |
| 436 | -142.775361 | -142.777361 | 435 아래 |
| 458 | -142.735918 | -142.733918 | 442와 474 위, 한 번만 이동 |
| 471 | -142.765332 | -142.767332 | 469 아래 |

각 source의 stable placement ID와 기존 490/495 중앙 보정은 유지한다. 수정 성분은 위 다섯 행의 Y 하나뿐이다. 높이 조정은 원본 material override의 draw order를 복원했다는 뜻이 아니라, 승인된 겹침의 명시적인 배치 교정이다. 새 near-coplanar 면의 유무와 경사진 면의 국소 교차를 구분해 RESULT에 기록한다. Imported, asset, shader, depth state, camera, navigation은 수정하지 않는다.

### 수정 파일과 실제 소비자

| 파일 | 수정 내용 |
|---|---|
| [Authoring mapplacements](C:/Users/user/Desktop/LostArk/Data/Maps/Authoring/LV_LOBBY_CLASSSELECT_SL00/LV_LOBBY_CLASSSELECT_SL00.mapplacements) | 사용자 발생 위치에 대응한 최소 placement transform만 수정 |
| [runtime mapplacements](C:/Users/user/Desktop/LostArk/Client/Bin/DataFiles/Map/LV_LOBBY_CLASSSELECT_SL00.mapplacements) | 직접 편집하지 않고 G01 publisher가 생성 |
| [LevelRegistry.cpp](C:/Users/user/Desktop/LostArk/Client/Private/LevelRegistry.cpp)의 Character Select descriptor | 추가 카메라 기록이 필요할 때 별도 단계에서 diagnostics opt-in을 검토. 이번 체크포인트에서는 변경하지 않음 |
| [MapAssetRenderUtils.h](C:/Users/user/Desktop/LostArk/Client/Public/MapAssetRenderUtils.h), [CPP](C:/Users/user/Desktop/LostArk/Client/Private/MapAssetRenderUtils.cpp) | 추가 기록 단계에서는 기존 camera snapshot 권위를 재사용. Character Select JSON writer·새 진단 패널이 이미 제품에 있다는 뜻이 아님 |
| 추가 완료 [test_map_surface_depth_contract.py](C:/Users/user/Desktop/LostArk/Tools/MapPipeline/test_map_surface_depth_contract.py) | 합성 geometry·투영 depth·입력 실패·배치 보존 회귀와 실제 resource CLI |
| 기존 [cook_wmodel_geometry_contract.py](C:/Users/user/Desktop/LostArk/Tools/ModelAssetConverter/cook_wmodel_geometry_contract.py) | legacy decoder를 재사용. 같은 binary reader를 새로 복제하지 않음 |

기존 effect test에는 geometry 교차·depth 검증 책임이 없으므로 surface 전용 test를 분리했다. 최소 삼각형 fixture의 unit test와 실제 local resource를 받는 integration 검사를 구분한다. 필수 resource가 없으면 해당 asset ID를 포함한 오류와 exit 2를 반환한다. 카메라 로그가 없을 때도 geometry 검사는 가능하지만 camera-depth 검증 성공으로 취급하지 않는다.

### 사용자 발생 쌍 확인

Debug의 Lobby → Test(Map Editor) → F1 → Map Tool → Area `LV_LOBBY_CLASSSELECT_SL00` → Map Assets에서 **사용자가 직접** 동일 모서리를 확인한다. Hierarchy는 asset 이름을 표시하므로 hover tooltip의 source ID로 후보를 찾고, 선택한 뒤 Inspector의 `Placement #`와 `Source`를 표와 대조한다. `Visible`을 한 쪽씩 바꿔 두 면이 있을 때만 발생하는지 확인하고, 진단 토글은 저장하지 않고 상단 Reload로 원상복구한다.

MapTool은 제품과 로드 scope·camera가 다를 수 있으므로 이 격리 검사는 원인 선택에만 쓴다. 최종 확인은 Server 승인으로 들어간 Character Select 제품 화면에서 한다. 이 절차를 위해 agent가 UI를 조작하거나 새 시나리오·새 기능키를 추가하지 않는다.

### 교정량 결정과 반영 흐름

1. 후보의 world triangle에 대해 XZ AABB는 탐색 필터로만 쓰고 실제 triangle 교차 polygon과 표면 높이를 계산한다. 접점 하나, 틈, 위에 가려진 면을 모두 z-fighting으로 분류하지 않는다.
2. 베른 core 수학 검증 뒤에도 Character Select 교정에 실제 카메라 수치가 필요하면, 기존 render frame·camera snapshot에서 공통 capture JSON을 확보하는 별도 기록 단계를 진행한다. 현재 이 JSON은 확보하지 않았으며 새 F1 진단 메뉴를 안내하지 않는다. 확보한 실제 View/Projection으로 큰 월드 좌표의 float 경로와 double reference를 함께 계산한다.
3. 교차 영역의 샘플을 같은 화면 ray로 비교해 post-projection depth 간격과 D24 양자화 여유를 산출한다. 현재 바닥의 실제 layer 순서를 보존하는 가장 작은 변위를 후보로 선택한다.
4. 변위가 다른 layer 순서를 뒤집거나 모서리 틈·계단·새 겹침을 만들지 않는지 교차 영역 전체와 인접 조각에서 검사한다. 중앙 410/490·411/495 회귀도 함께 검사한다.
5. placement 전체 Y 이동으로 형태를 보존할 수 있는 쌍만 Authoring에 반영한다. 서로 다른 고유 영역을 가진 두 조각을 같은 asset이라는 이유로 통째 삭제하지 않는다.
6. 전체 이동이 틈을 만들면 해당 asset의 실제 중복면만 기존 ModelAssetConverter 경로에서 수정하는 범위로 전환한다. 이때만 그 모델을 사용하는 다른 placement와 필요한 resource 의존성까지 확인한다. runtime의 숨은 per-instance offset이나 별도 렌더 경로로 우회하지 않는다.
7. Validate → expected diff 검토 → Publish → Check 순서로 적용한다. load 실패 시 기존 문서·객체를 유지하는 `parse → validate → stage → commit` 경계를 유지한다.

`--camera-log` 입력은 `LOSTARK_MAP_RENDER_CAPTURE` version 1, Area `LV_LOBBY_CLASSSELECT_SL00`, `row-vector-row-major-d3d-lh-z01` convention을 검사한다. frame/revision은 0이 아닌 decimal uint64 문자열이며 frame ID는 증가해야 한다. 같은 행렬의 revision 재사용은 허용하고, revision 감소·동일 revision의 행렬 변경·missing/invalid camera·nonfinite/singular/invalid projection·camera mismatch는 오류다. 기본 camera로 대체하지 않는다.

추가 기록이 필요하면 실제 render frame을 구분하는 bounded capture를 사용하고, 베른 로그를 덮어쓰지 않는다. 이번 단계에서는 writer의 생성 경로나 ImGui 패널을 제품 기능으로 안내하지 않는다. Character Select의 bypass·diagnostics 기본값과 기존 카메라 owner는 유지한다.

수정량은 위 실측 결과로 정한다. 이미 적용된 490/495의 추가 2mm 이동, 모든 바닥의 일괄 Y 보정, 임의 alpha 처리, 전체 opaque shader depth bias, 전역 near/far 변경은 기본 수정안에서 제외한다.

### 불변식과 종료 증거

- 기본 배치 수정안은 803 row, stable placement/source/asset ID, visible, 선택하지 않은 모든 SRT를 보존한다. 제품 scope의 779 placement도 유지한다.
- Imported 보존본과 기존 중앙 보정은 변경하지 않는다.
- runtime 생성물은 Authoring과 일치해야 한다. 수정 전후 차이는 승인한 placement와 정확한 transform 성분으로 제한한다.
- coplanar, 완전히 떨어진 면, XZ만 겹치고 높이가 다른 면, 회전·mirror·nonuniform scale, 작은/큰 camera 거리, 무관한 placement 보존을 테스트한다.
- G02 자동 종료 증거는 선택한 쌍의 교차 영역 depth 여유·인접 seam 보존과 데이터 회귀 통과다. **시각 종료는 G04의 사용자 승인**이다.

## G03. 한 면에서도 남는 일렁임은 texture 축소 sampling으로 분리한다

이 G는 **사용자가 한 면만 남긴 상태에서도 무늬 일렁임이 남는다고 확인한 경우**에 적용한다. z-fighting 수정과 무관한 153개 texture 전체 재생성 작업을 기본 범위에 넣지 않는다.

대표 리소스는 다음과 같다.

`Map/CHARACTERSELECTMAP/MAP_E5357DD78673_BG_ELG_ARYANORB_FLOOR22_SM/textures/bg_elg_aryanorb_floor21_01_d_hht.dds`

1. 발생 면이 실제 사용하는 diffuse/normal/specular texture와 SRV mip 수를 확인한다. header만 보지 않고 file payload와 로딩된 mip count를 대조한다.
2. 발생 texture만 기존 offline 변환 경로에서 mip chain을 생성한다. 도구 실행 파일과 원본 입력을 확인하고 재생성 옵션을 기록한다. [Microsoft DirectXTex Texconv](https://github.com/microsoft/DirectXTex/wiki/Texconv)는 사용할 수 있는 공식 변환 도구지만 이번 조사에서는 로컬 설치·경로를 확인하지 않았으므로 실행 가능하다고 가정하지 않는다.
3. diffuse/emissive의 sRGB 해석과 normal/specular/ORM의 linear 해석을 유지한다. normal mip은 방향 재정규화를 포함해 용도에 맞게 생성하고 alpha·압축 format도 보존한다.
4. sampler 조정은 mip chain 반영 후에도 필요한 경우에 한정한다. camera 근접/경사/원거리와 같은 texture를 쓰는 다른 placement를 확인한다.
5. 기존 `CModel → CMaterial` 경로에서 소비하고 per-frame texture 재생성이나 별도 texture loader를 만들지 않는다.

전체 D32/reverse-Z 전환, 모든 재질 depth bias, 무조건적인 sampler 교체는 이번 기본안에 포함하지 않는다. depth bias는 geometry의 실제 layer 계약을 대신하지 않는다.

G03 자동 종료 증거는 대상 texture의 mip chain/format/색 공간 검증이다. 일렁임과 seam의 최종 시각 판정은 사용자에게 남긴다. 현상이 단일 면에서 재현되지 않으면 이 G는 적용하지 않았다고 명확히 기록한다.

## G04. 회귀·수동 확인·인계

### 자동 검증 명령

교정 뒤 publisher와 geometry 계약을 다음 순서로 실행했다. 자동 수치 검증은 통과했지만 제품 화면의 visual PASS를 대신하지 않는다.

```powershell
Set-Location 'C:/Users/user/Desktop/LostArk'
python -B -m unittest Tools.MapPipeline.test_map_effect_presentation_contract Tools.MapPipeline.test_map_surface_depth_contract
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LOBBY_CLASSSELECT_SL00 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LOBBY_CLASSSELECT_SL00 -Mode Publish
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LOBBY_CLASSSELECT_SL00 -Mode Check
python -B Tools/MapPipeline/test_map_surface_depth_contract.py --resource-root Client/Bin/Resources --area-id LV_LOBBY_CLASSSELECT_SL00 `
  --expected-y-change LV_LOBBY_CLASSSELECT_SL00:export:405=-0.002 `
  --expected-y-change LV_LOBBY_CLASSSELECT_SL00:export:427=-0.002 `
  --expected-y-change LV_LOBBY_CLASSSELECT_SL00:export:436=-0.002 `
  --expected-y-change LV_LOBBY_CLASSSELECT_SL00:export:458=0.002 `
  --expected-y-change LV_LOBBY_CLASSSELECT_SL00:export:471=-0.002
```

교정 후 실제 출력은 [geometry JSON](C:/Users/user/Desktop/LostArk/.codex_tmp/character_select_height_adjust_root_20260828/geometry-after.json)에 남아 있다. `status=diagnostic_only`, `cameraDepthStatus=not_requested`이며 여섯 쌍 모두 근접 평면 쌍 0, 기록된 검사점 간격 약 1.94~2.05mm다. 이 파일은 로컬 수치 증거이며 Git runtime 입력이 아니다.

publisher fixture 14 tests와 surface 21 tests는 책임이 다르다. 전자는 파일 집합 검증·무변경·rollback을, 후자는 합성 geometry와 실제 resource 배치 보존을 검사한다. 실제 카메라 JSON이 없으므로 아직 없는 로그나 새 F1 기능을 전제로 안내하지 않는다.

다음 정본 전체 회귀는 이번 focused 단계에서 다시 실행하지 않았다. 데이터와 publisher 변경의 직접 검사는 위 명령으로 닫았고, Client/UI는 사용자 전용 경계에 따라 실행하지 않았다.

```powershell
powershell -ExecutionPolicy Bypass -File 'C:/Users/user/Desktop/LostArk/Tools/Build/Invoke-BuildAndRegression.ps1' -Configuration Debug
powershell -ExecutionPolicy Bypass -File 'C:/Users/user/Desktop/LostArk/Tools/Build/Invoke-BuildAndRegression.ps1' -Configuration Release
```

Publish는 검토된 Authoring diff를 runtime에 실제 반영했다. 수정 전 Check는 예상대로 불일치를 반환했고 파일을 바꾸지 않았으며, Publish 뒤 Check는 byte 일치를 확인했다.

- 베른 shard fixture로 publisher 공통 경로도 회귀한다. 실제 Bern map에 수정할 데이터가 없으면 다시 publish하지 않는다.
- surface test의 직접 실행 모드는 구현됐다. `--resource-root`, `--area-id`로 geometry를 검사하고, 선택적인 `--camera-log`가 있을 때만 기록된 witness 삼각형의 동일 화면 ray 깊이를 계산한다. CPU float32 계산은 GPU FMA·rasterizer rounding을 완전히 재현하지 않으며 scene occlusion·실제 pixel 검증도 아니다. 새 배치 Y 교정을 검사할 때는 승인된 후보 source ID와 Imported 대비 변위를 `--expected-y-change`로 명시하며 이 옵션이 데이터를 수정하지는 않는다.
- 변경 JSON/XML을 parse하고 정본 regression의 compiled shader closure 검사도 유지한다. 기본 데이터안과 기존 C++ 진단 확장은 신규 production C++ 파일이 없어 Client/Engine project/filter 등록 변경이 없다. 공통 C++ 수치 harness 등록은 베른 계획 G03에서만 소유한다.
- 옛 `Test-CharacterSelectCenterDepthSeparation.ps1`, `Invoke-ProjectAudit.ps1`은 현재 존재하지 않는다. 현재 동작하는 test 경로로 교체하며 없는 스크립트의 PASS를 재사용하지 않는다.

### 사용자가 직접 확인할 순서

1. 현재 PC는 LAN `server-host`이며 방화벽 준비 완료, 조사 시 Server/Client 미실행이었다. Visual Studio **Server + Client** profile → **Ctrl+F5**로 사용자가 시작한다. Client 작업 디렉터리는 `Client/Default`다.
2. Lobby → Character Select를 선택해 Server 승인을 받은 제품 맵에 들어간다. 기존 중앙과 사용자가 지적한 모서리를 먼저 비교한다.
3. follow 이동/정지, F6 free, Tab mouse-look 후 마우스 회전·WASD 이동, 근접·원거리·경사 시점, F6 follow 복귀를 같은 위치에서 확인한다.
4. 두 표면이 번갈아 보이는지, 단일 표면의 무늬가 반짝이는지, 수정 후 틈·layer 순서 변경이 생겼는지 구분해 글로 남긴다.
5. Character Select 재진입과 Create Character를 통한 Bern 진입도 확인한다. 베른에서 덩어리 소멸이 남으면 이 계획의 성공으로 묶지 않고 별도 베른 계획 상태를 유지한다.

MapTool 저장/Reload/실패 시 기존 상태 보존은 사용자가 Debug에서 확인한다. Release에 없는 MapTool smoke는 PASS로 적지 않는다. 에이전트의 Client/UI 실행·조작·화면 캡처는 금지한다. 사용자가 첨부 이미지 분석을 요청하면 진단하되, 이미지만으로 자동 완료·visual PASS를 만들지 않는다.

### 상태·리소스·완료 보고

| 구분 | 2026-08-28 체크포인트 상태 | 남은 기록 |
|---|---|---|
| 기존 중앙 보정 | 490/495의 -0.002m와 803개 ID·나머지 배치 보존 확인 | 실제 카메라에서 중앙·인접 layer 확인 |
| 새 모서리 후보 | 다섯 placement Y 교정으로 여섯 쌍 근접 평면 0 | 실제 화면에서 동일 위치 확인 |
| 구현 | Authoring 수정, publisher Publish, runtime byte 일치 완료. asset·shader·near/far 미변경 | 없음 |
| 자동 검증 | publisher 14 tests, surface 21 tests, 실제 geometry와 Validate/Publish/Check PASS | 정본 전체 build/regression은 이번 단계 미실행 |
| 실제 camera-depth | JSON 미확보, `not_requested` | 실제 frame의 동일-ray depth 수치 |
| 수동 검증 | 미실행 | 사용자 조작과 서면 관찰 |
| texture 후속 | mip 부재 확인, 현상 기여는 미확정 | G03 적용 여부와 근거 |

배치만 수정하면 필요한 runtime resource는 위 4개 asset ID의 기존 `Map/CHARACTERSELECTMAP/<assetId>/<assetId>.wmodel`과 catalog가 참조하는 texture다. 물리 위치는 [Character Select Resources](C:/Users/user/Desktop/LostArk/Client/Bin/Resources/Map/CHARACTERSELECTMAP) 아래 각 asset 폴더다. 신규 resource가 없으면 Git dependency closure 추가도 없다.

G03 또는 실제 geometry 수정으로 resource가 바뀐 경우 인계에는 바뀐 Resources-relative asset ID·물리 위치·Git 포함 여부를 적는다. 현재 사용자는 Git pull만으로 재현을 요구하지 않았으므로 전체 물리 pack이나 추출 원본을 자동 추적하지 않는다. 팀장 관리 resource 배포 범위를 함께 명시하고 ZIP hash/lock/별도 manifest를 완료 조건으로 만들지 않는다.

큰 기존 dirty worktree의 Valtan·Balance·Effect·project 변경은 이 작업에 섞지 않는다. publisher 공통 검증과 표면 교정은 각각 검증 가능한 변경 단위로 분리하고, 실제 구현 때 각 단위에 필요한 계획/RESULT/test를 함께 갱신한다. RESULT는 이번 geometry 진단과 과거 중앙부 변경 이력을 분리하며, 수치 진단만으로 모서리 수정 완료를 기록하지 않는다.
