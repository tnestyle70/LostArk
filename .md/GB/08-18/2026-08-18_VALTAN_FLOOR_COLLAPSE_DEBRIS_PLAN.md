# 2026-08-18 발탄 아레나 바닥 붕괴 파편 계획

바닥 붕괴 G1의 정본은 `.md/GB/08-16/2026-08-16_VALTAN_ARENA_FLOOR_COLLAPSE_PLAN.md`와 같은 폴더의
RESULT다. 이 문서는 그 RESULT §7-3이 남긴 `파편이 없다`를 닫는 후속 G를 다룬다.

## 1. 이번 G의 경계

G1이 끝난 시점의 실제 상태는 다음과 같다.

- `Client/Bin/DataFiles/World/LV_LUT_HEARTRB_ED.worlddestruction.json` projection group 105개
- `Client/Bin/DataFiles/World/LV_LUT_HEARTRB_ED.worlddestructionpresentation.json` 파편 profile 99개
- 차이 6개가 바닥 sector이고, 이들은 250 ms `BREAKING` 뒤 파편 없이 사라진다.

이번 G는 그 6개에 파편 profile을 주어 105/105로 맞춘다. 낙사, 84 패턴 타이밍 근거,
기둥 제품 트리거는 이번 범위가 아니다.

## 2. 파편 모델을 어디서 가져오는가

`Client/Bin/Resources/Deploy/LV_LUT_HEARTRB_ED/`에는 벽 8종만 `fractured/` 12조각을 갖는다.
바닥 3종(`VALTAN_FLOOR_RAIL`, `VALTAN_FLOOR_BRICK_A`, `VALTAN_FLOOR_BRICK_B`)은
`.deployassets`의 fractured 두 칸이 비어 있고 실제 `.wmodel`도 없다. G1이 STATIC의 fractured 쌍을
optional로 만든 이유가 이것이다.

`Client/Bin/Resources`는 팀장이 관리하는 물리 폴더이므로 이번 G는 새 에셋을 넣지 않는다.
대신 이미 배포된 Valtan 잡석 4종을 재사용한다. 이는 08-16 RESULT §7-3이 적어 둔
`기존 generic stone 4종을 재사용한 파편은 별도 단계다`를 그대로 따른 것이다.

```text
Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_001.wmodel
Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_002.wmodel
Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_004.wmodel
Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_010.wmodel
```

## 3. 제품 파편 경로의 불변식

`CWorldDestructionDebrisPresentationRuntime`의 제품 경로는 다음을 강제한다.

- `Find_Recipe(placement.assetId)`가 정확히 `ACTORS_PER_EMITTER`(12)개를 돌려줘야 한다.
- recipe는 `CDestructionSimulationRuntime::Get_ProjectAuthoredDebrisModelSpecs()`의
  `sourceDeployAssetId`가 비어 있지 않은 항목만 사용한다. 빈 값은 MapTool preview 전용이다.
- prototype tag는 exact spec 전체에서 유일해야 한다(`uniqueTags.emplace`).
- 한 spec의 `vSourceLocalPivotMeters`가 그 조각의 spawn 위치다. placement의
  `uniformScale`을 곱하고 placement 회전을 적용한 뒤 placement 위치에 더한다.

따라서 바닥 asset 3종에 각각 12개, 합계 36개의 exact spec을 추가해야 한다.

## 4. 12개 pivot을 어떻게 정했는가

pivot을 `{0,0,0}`으로 두면 잡석이 전부 아레나 중심에서 나온다. 바닥은 고리 모양이라
중심은 아직 멀쩡한 자리다. 그래서 G1이 이미 저작해 둔 실측값에서 뽑았다.

`Data/Navigation/LV_LUT_HEARTRB_ED.navblockers`의 바닥 collapse region은 그 sector가 실제로
덮는 nav cell 목록이다. grid는 `392x312`, cell `0.5 m`, origin `(-6, -165)`이고 아레나 중심은
placement 위치 `(156.279007, -121.976997)`이다. 각 cell의 world 좌표에서 중심을 빼면 sector의
실제 반경과 각도 분포가 나온다.

| region | cell | 최소 r | 중앙 r | 최대 r |
|---|---|---|---|---|
| `floor84.rail.7000000000000000001` | 347 | 13.52 | 14.67 | 16.26 |
| `floor84.rail.7000000000000000005` | 325 | 13.58 | 14.68 | 16.68 |
| `floor30.brick.7000000000000000002` | 365 | 7.81 | 11.19 | 13.83 |
| `floor30.brick.7000000000000000003` | 427 | 7.52 | 11.23 | 13.78 |
| `floor30.brick.7000000000000000006` | 360 | 7.66 | 11.19 | 13.80 |
| `floor30.brick.7000000000000000007` | 425 | 7.49 | 11.21 | 13.75 |

identity 회전 placement의 region을 각도 12등분하고 각 구간의 중앙 반경 cell을 하나씩 골랐다.
y는 바닥면 바로 위인 `+0.25 m`로 고정했다.

같은 asset을 쓰는 나머지 placement 3개는 `yaw180`(`0 1 0 0`)이라 같은 local pivot이 180도
회전해 반대편에 놓인다. 실제로 대칭인지 region 각도로 확인했다.

```text
VALTAN_FLOOR_RAIL      identity 335.0~161.4   identity+180 155.0~341.4   yaw180 156.1~337.9
VALTAN_FLOOR_BRICK_A   identity 341.8~ 67.1   identity+180 161.8~247.1   yaw180 162.8~247.4
VALTAN_FLOOR_BRICK_B   identity  65.0~166.7   identity+180 245.0~346.7   yaw180 244.8~344.1
```

오차 0.1~3.5도는 cell 양자화 범위다. 그래서 asset당 12개 pivot 한 벌만 저작한다.

## 5. 파일 목록

| 구분 | 경로 | 역할 |
|---|---|---|
| 수정 | `Client/Private/DestructionSimulationRuntime.cpp` | 바닥 exact debris spec 36개 추가 |
| 수정 | `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.destructionsimulation.json` | 바닥 profile 6개 추가 (99 → 105) |
| 수정 | `Tools/WorldPipeline/Publish-ValtanWorldDestruction.ps1` | 바닥 면제 규칙 3곳 제거, 계약 테스트 기대값 교체 |
| 생성물 | `Client/Bin/DataFiles/World/LV_LUT_HEARTRB_ED.worlddestructionpresentation.json` | publisher가 99 → 105 profile로 교체 |

새 C++ 파일이 없으므로 `.vcxproj`와 `.vcxproj.filters` 등록 변경은 없다.

## 6. Client/Private/DestructionSimulationRuntime.cpp

변경 종류: 함수 내부 데이터 블록 추가

적용 위치: `Get_ProjectAuthoredDebrisModelSpecs()`의 `specs` 초기화 목록. 마지막 항목
`Prototype_Component_Model_DestructionWall_02311_Chunk11`의 닫는 `}`를 `},`로 바꾸고 그 아래,
`	};` 바로 위에 아래 블록을 넣는다.

```cpp
		/* The arena floor owns no fractured mesh, so its collapse reuses the
		   four Valtan rubble meshes already shipped for destruction debris. The
		   twelve pivots per sector are measured median-radius nav cells of that
		   sector's authored collapse region, spread over its angular span, so the
		   rubble appears across the floor that is actually giving way instead of
		   at the arena centre. The yaw180 twin of each sector reuses the same
		   local pivots because its region is the 180-degree mirror. */
		{
			L"Prototype_Component_Model_DestructionFloor_Rail_Chunk00",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_001.wmodel",
			3.5f, "VALTAN_FLOOR_RAIL", { 13.970993000f, 0.250000000f, -4.773003000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_Rail_Chunk01",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_002.wmodel",
			3.5f, "VALTAN_FLOOR_RAIL", { 14.470993000f, 0.250000000f, 1.226997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_Rail_Chunk02",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_004.wmodel",
			3.5f, "VALTAN_FLOOR_RAIL", { 13.970993000f, 0.250000000f, 4.226997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_Rail_Chunk03",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_010.wmodel",
			3.5f, "VALTAN_FLOOR_RAIL", { 13.470993000f, 0.250000000f, 5.726997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_Rail_Chunk04",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_001.wmodel",
			3.5f, "VALTAN_FLOOR_RAIL", { 10.470993000f, 0.250000000f, 10.226997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_Rail_Chunk05",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_002.wmodel",
			3.5f, "VALTAN_FLOOR_RAIL", { 5.970993000f, 0.250000000f, 13.226997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_Rail_Chunk06",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_004.wmodel",
			3.5f, "VALTAN_FLOOR_RAIL", { 4.970993000f, 0.250000000f, 13.726997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_Rail_Chunk07",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_010.wmodel",
			3.5f, "VALTAN_FLOOR_RAIL", { -0.029007000f, 0.250000000f, 14.726997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_Rail_Chunk08",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_001.wmodel",
			3.5f, "VALTAN_FLOOR_RAIL", { -3.029007000f, 0.250000000f, 14.226997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_Rail_Chunk09",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_002.wmodel",
			3.5f, "VALTAN_FLOOR_RAIL", { -8.529007000f, 0.250000000f, 12.226997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_Rail_Chunk10",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_004.wmodel",
			3.5f, "VALTAN_FLOOR_RAIL", { -11.529007000f, 0.250000000f, 9.726997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_Rail_Chunk11",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_010.wmodel",
			3.5f, "VALTAN_FLOOR_RAIL", { -13.529007000f, 0.250000000f, 5.726997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickA_Chunk00",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_001.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_A", { 11.470993000f, 0.250000000f, -2.773003000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickA_Chunk01",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_002.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_A", { 10.970993000f, 0.250000000f, -1.273003000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickA_Chunk02",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_004.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_A", { 11.470993000f, 0.250000000f, -0.273003000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickA_Chunk03",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_010.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_A", { 10.970993000f, 0.250000000f, 1.726997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickA_Chunk04",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_001.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_A", { 10.970993000f, 0.250000000f, 2.226997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickA_Chunk05",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_002.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_A", { 10.470993000f, 0.250000000f, 4.226997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickA_Chunk06",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_004.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_A", { 9.970993000f, 0.250000000f, 5.226997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickA_Chunk07",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_010.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_A", { 9.470993000f, 0.250000000f, 6.226997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickA_Chunk08",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_001.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_A", { 8.470993000f, 0.250000000f, 7.726997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickA_Chunk09",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_002.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_A", { 7.470993000f, 0.250000000f, 8.226997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickA_Chunk10",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_004.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_A", { 6.470993000f, 0.250000000f, 9.226997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickA_Chunk11",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_010.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_A", { 4.970993000f, 0.250000000f, 9.726997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickB_Chunk00",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_001.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_B", { 4.470993000f, 0.250000000f, 10.726997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickB_Chunk01",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_002.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_B", { 1.970993000f, 0.250000000f, 11.226997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickB_Chunk02",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_004.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_B", { 0.470993000f, 0.250000000f, 11.226997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickB_Chunk03",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_010.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_B", { -1.029007000f, 0.250000000f, 11.226997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickB_Chunk04",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_001.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_B", { -2.529007000f, 0.250000000f, 10.726997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickB_Chunk05",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_002.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_B", { -4.029007000f, 0.250000000f, 10.726997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickB_Chunk06",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_004.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_B", { -5.529007000f, 0.250000000f, 9.726997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickB_Chunk07",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_010.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_B", { -7.029007000f, 0.250000000f, 8.726997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickB_Chunk08",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_001.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_B", { -8.029007000f, 0.250000000f, 8.226997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickB_Chunk09",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_002.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_B", { -9.029007000f, 0.250000000f, 6.726997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickB_Chunk10",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_004.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_B", { -10.029007000f, 0.250000000f, 4.726997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickB_Chunk11",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_010.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_B", { -10.529007000f, 0.250000000f, 3.226997000f }
		}
```

## 7. LV_LUT_HEARTRB_ED.destructionsimulation.json

변경 종류: `profiles` 배열에 항목 6개 추가

적용 위치: 마지막 wall profile의 닫는 `}` 뒤에 `,`를 붙이고 그 아래 이어서 넣는다.

기존 wall profile과 같은 형식이며 `durationSeconds` 5, `previewGroundEnabled` true,
`previewGroundHeight` 20, `previewGroundHalfExtents` `[30, 30]`, element 1개를 쓴다.
element의 값은 다음과 같다.

| groupId | sourceRuntimePlacementId | direction | speed | gravity | lifetime |
|---|---|---|---|---|---|
| `destroyable.group.valtan.floor30.brick.7000000000000000002` | `7000000000000000002` | `[0, -1, 0]` | 6 | 2 | 4 |
| `destroyable.group.valtan.floor30.brick.7000000000000000003` | `7000000000000000003` | `[0, -1, 0]` | 6 | 2 | 4 |
| `destroyable.group.valtan.floor30.brick.7000000000000000006` | `7000000000000000006` | `[0, -1, 0]` | 6 | 2 | 4 |
| `destroyable.group.valtan.floor30.brick.7000000000000000007` | `7000000000000000007` | `[0, -1, 0]` | 6 | 2 | 4 |
| `destroyable.group.valtan.floor84.rail.7000000000000000001` | `7000000000000000001` | `[0, -1, 0]` | 6 | 2 | 4 |
| `destroyable.group.valtan.floor84.rail.7000000000000000005` | `7000000000000000005` | `[0, -1, 0]` | 6 | 2 | 4 |

`direction`이 `[0, -1, 0]`인 이유는 바닥이 아래로 꺼지기 때문이다. 런타임이
`UPWARD_SPEED_METERS_PER_SECOND`(3.25)를 y에 무조건 더하므로 speed 6이면 초기 y 속도가
약 -2.75 m/s가 되어 잡석이 확실히 아래로 떨어진다. `DIRECTION_SPREAD_RADIANS`(28도)가
조각마다 방향을 흩고, `Build_SpreadDirection`은 `upDot > 0.95f`에서 기준축을 X로 바꾸므로
수직 방향에서도 퇴화하지 않는다.

publisher 제약도 모두 만족한다. `lifetime`은 `(0, duration]`이고
`triggerTime + lifetime <= duration`이며 direction의 크기는 정확히 1이다.

전체 profile 한 개의 형식은 다음과 같다. 나머지 5개는 `groupId`와 placement ID만 다르다.

```json
                     {
                         "profileId":  "destroyable.group.valtan.floor30.brick.7000000000000000002.preview",
                         "groupId":  "destroyable.group.valtan.floor30.brick.7000000000000000002",
                         "durationSeconds":  5,
                         "previewGroundEnabled":  true,
                         "previewGroundHeight":  20,
                         "previewGroundHalfExtents":  [
                                                          30,
                                                          30
                                                      ],
                         "elements":  [
                                          {
                                              "elementId":  "debris.7000000000000000002",
                                              "sourceRuntimePlacementId":  "7000000000000000002",
                                              "suppressionAliasPlacementIds":  [

                                                                               ],
                                              "spawnOffset":  [
                                                                  0,
                                                                  0,
                                                                  0
                                                              ],
                                              "direction":  [
                                                                0,
                                                                -1,
                                                                0
                                                            ],
                                              "speedMetersPerSecond":  6,
                                              "gravityScale":  2,
                                              "lifetimeSeconds":  4,
                                              "trigger":  {
                                                              "kind":  "TIMELINE_TIME",
                                                              "timeSeconds":  0,
                                                              "receiverCollisionId":  ""
                                                          }
                                          }
                                      ]
                     }
```

## 8. Tools/WorldPipeline/Publish-ValtanWorldDestruction.ps1

변경 종류: 검증 분기 제거와 계약 테스트 기대값 교체

`Compile-ValtanDebrisProfiles`의 coverage 검사에서 바닥 면제를 없앤다. `$floorGroupIds` 집합과
그것을 쓰던 세 곳이 사라지고, coverage는 모든 canonical group을 요구한다.

```powershell
    # Every canonical group owns exactly one debris simulation profile. A floor
    # sector has no fractured mesh of its own, so its profile drives the shipped
    # Valtan rubble meshes instead, and it is no longer exempt from coverage.
    if ($profileByGroupId.Count -ne $WorldGroupById.Count) {
        throw 'Destruction simulation must cover every canonical world destruction group exactly once.'
    }
    $compiledProfiles = [Collections.Generic.List[object]]::new()
    foreach ($includedGroup in @(Sort-OrdinalByProperty @($IncludedGroups) 'GroupId')) {
```

projection 조립에서도 `$isFloorProjectionGroup` 분기를 없앤다.

```powershell
        if ($null -eq $profile) {
            throw "Projection group is missing its debris presentation profile: $($_.GroupId)"
        }
```

`Invoke-ContractTests`의 기대값은 `비-바닥 부분집합`에서 `projection 전체를 덮되 그중 6개가 바닥`으로 바뀐다.

```powershell
	if ($canonicalPresentation.formatVersion -ne 1 -or
		$presentationGroupIds.Count -ne $expectedCanonicalGroupCount -or
		@($presentationGroupIds | Where-Object {
			-not $projectionGroupIdSet.Contains($_)
		}).Count -ne 0 -or
		$projectionOnlyGroupIds.Count -ne 0 -or
		@($presentationGroupIds | Where-Object {
			$_.StartsWith($floorGroupIdPrefix, [StringComparison]::Ordinal)
		}).Count -ne $expectedFloorGroupCount) {
		throw 'Canonical debris presentation must cover every projection group, including every floor collapse sector.'
	}
```

## 9. 적용 순서와 검증

1. 위 세 파일을 순서대로 반영한다.
2. `powershell -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-ValtanWorldDestruction.ps1 -Mode Validate`
3. 같은 스크립트 `-Mode ContractTest`
4. 같은 스크립트 `-Mode Publish`
5. Client를 다시 빌드한다. spec 테이블이 C++이므로 재빌드 전에는 바닥 cue가 fail-closed다.
6. `ClientFrontendHarness.exe`로 World Destruction 계열 회귀를 확인한다.
7. `Server.exe --contract-test`로 Server 계약이 그대로인지 확인한다.
8. Lobby → Valtan → F1 → `Reset + Play 84`, `Reset + Play 30`으로 사용자가 육안 확인한다.

실패 입력 확인은 publisher가 담당한다. 바닥 profile을 하나라도 빼면 2단계에서
`Destruction simulation must cover every canonical world destruction group exactly once.`로
멈추고 기존 런타임 문서를 교체하지 않는다.
