# Bern / Valtan 기본 하늘 배경 연결 결과

## 구현 상태

Bern의 `MAP_EDDEDF2CF6A1_SKY_MIRROR_SM`을 기존 Map renderer의 `Sky/None` profile로 연결했다. 별도 SkyBox 클래스, cubemap renderer, Level 하드코딩 오브젝트, 두 번째 모델 런타임은 추가하지 않았다.

재생성 정본은 `LV_BER_BERNCASTLE.renderprofiles.json`이다. shard builder는 이 문서를 읽고 각 child shard에 포함된 stable assetId profile만 전달한다. 존재하지 않는 assetId, 중복 assetId, 잘못된 Area는 정본 교체 전에 거부한다. 기존 default-hidden visibility override도 같은 child manifest에 합쳐져 동작한다.

Valtan은 이미 source/runtime catalog에서 같은 자산이 `Sky/None`이고, 보이는 placement 1개와 `includeBackground=true` full map scope를 사용한다. 따라서 자산을 설치하거나 복제하지 않았다. 붉은 최종 페이즈의 spacehole/hugechaosgate 조합도 변경하지 않았다.

## Valtan MapTool Area 전환 차단 수정

Valtan의 현재 world destruction 문서는 13개 파괴 binding을 `STAGE_ENTER`로 활성화하고 각 group의 `navigationRegionIds`는 비어 있다. 이는 publisher가 허용하는 현재 제품 계약이다. 기존 MapTool 검증은 trigger 종류와 관계없이 모든 활성 binding에 navigation blocker를 요구해, Valtan Area를 전부 stage한 뒤 external-reference 검증에서 rollback했다.

`CMapTool::Validate_DestructionExternalReferences`는 이제 실제 receiver 충돌과 navigation 전환을 함께 쓰는 `COLLISION_IMPACT` binding에만 navigation region 존재 및 cell 보유를 요구한다. `STAGE_ENTER`, `STAGE_TIME`, `STAGE_EXIT`는 wall member와 encounter pattern/stage만 검증한다. 따라서 비어 있는 Valtan navblockers 문서를 억지로 채우거나 파괴 binding을 비활성화하지 않고 현재 데이터 계약 그대로 MapTool에 진입할 수 있다.

추가 확인:

- Valtan destruction publisher Validate/ContractTest: 13 groups, 13 bindings PASS
- Client world-destruction projection fast harness: 33 checks, failures 0
- Valtan Map/Deploy WModel 373개 `ModelAssetConverter info`: 실패 0
- 위 WModel 내부 texture reference: 누락 0
- Client x64 Debug build: PASS

Client 화면 조작은 사용자 검증 경계이므로 MapTool의 최종 Area 전환은 사용자가 새 Debug Client를 재시작한 뒤 확인한다.

## 원본 하늘 자산 판정

두 Area의 기본 배경은 프로젝트에서 새로 만든 cubemap이 아니다. 원본 Lost Ark UE3 데이터의 `lv_matte.mesh.sky_mirror_sm`, material `sky_base_opa`, texture `lv_sky_0161_d`를 WModel로 변환한 실제 map sky mesh다. Valtan source placement `LV_LUT_HEARTRB_ED_PS:export:528`와 stable placement ID `12912593380788004153`도 보존돼 있다.

다만 현재 프로젝트의 `Sky/None` render profile과 dark color tint는 기존 renderer에 맞춘 presentation 값이므로 원본 UE3 shader를 픽셀 단위로 그대로 재현했다는 뜻은 아니다. 또한 Valtan 최종 붉은 하늘은 이 기본 sky 하나가 아니라 spacehole/hugechaosgate particle layer 조합이며 이번 기본 하늘 범위와 분리돼 있다.

## 보존된 Bern placement

```text
placementId: 12120733164161297609
position:    138.929795 47.6818555 -140.012109
quaternion:  0 0.828904115 0 0.559390712
scale:       72.5086498 72.5086498 72.5086498
visible:     1
```

Authoring, regenerated Imported BASE, published runtime BASE에서 위 행이 동일하다. 전체 Bern placement 수도 50,017로 유지됐다.

## 자동 검증

PASS:

- Bern source/runtime 기본 Sky 행 각 1개
- Valtan source/runtime 기본 Sky 행 각 1개
- 두 Area의 source catalog → placement → runtime catalog → placement join
- Bern/Valtan WModel 존재 및 `ModelAssetConverter info` 성공
- Bern `textures/lv_sky_0161_d.dds` 존재 및 material base slot 연결
- shard builder 전체 9 test, MapTool scene 포함 총 24 test
- full Bern shard 재생성 및 receipt 갱신
- `Publish-MapAuthoring.ps1 -AreaId LV_BER_BERNCASTLE`
- publisher `FailureAfterPromote=1` 실패 주입 후 기존 runtime hash 복원
- Client x64 Debug/Release build
- 변경 범위 `git diff --check`

전체 `ProjectAudit`는 실행했지만 현재 작업트리의 기존 미완성 데이터/렌더링 계약과 시스템 PATH의 `python` 부재를 포함한 36개 항목 때문에 실패했다. 이번 sky의 focused test, publisher, source/runtime join, build에서는 실패가 없다.

## 리소스 인계

새 Resources 파일은 추가하지 않았다. 기존 두 폴더를 그대로 사용한다.

```text
Map/LV_BER_BERNCASTLE/MAP_EDDEDF2CF6A1_SKY_MIRROR_SM
Map/LV_LUT_HEARTRB_ED/MAP_EDDEDF2CF6A1_SKY_MIRROR_SM
```

따라서 이번 변경으로 팀장에게 별도 Resources 폴더를 전달할 필요는 없다.

## 수동 화면 확인

아직 visual PASS가 아니다. 사용자가 Lobby에서 Bern과 Valtan에 각각 진입해 다음을 확인해야 한다.

- 이동 시 하늘이 카메라에서 밀려나지 않는다.
- 회전 시 모든 방향을 채운다.
- 건물과 성벽 앞을 덮지 않는다.
- 검은 배경, clipping, z-fighting이 없다.
- 원하는 기준 이미지와 색·밝기가 맞는다.
