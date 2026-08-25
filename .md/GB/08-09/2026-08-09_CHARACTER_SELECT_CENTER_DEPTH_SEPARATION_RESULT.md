# Character Select 중앙 교차 바닥 depth 분리 수정 결과

## 완료 상태

`LV_LOBBY_CLASSSELECT_SL00`의 원래 bridge/fence mesh와 texture를 전부 유지하면서 중앙 두 교차 pair와 시작 지점 왼쪽의 4-way 교차 group에 deterministic 2mm depth step을 적용했다. placement 803개, stable placement ID, asset catalog 55개와 Imported 원본 증거는 유지했다.

2026-08-25 사용자 첨부 화면으로 material variant 회귀를 확인했다. 잘못 추가했던 floor texture variant, 여섯 placement의 variant asset ID, catalog 증분과 전용 WModel/DDS는 모두 제거했다. 사용자가 제공한 화면은 occurrence 진단 입력으로만 사용했으며 에이전트가 Client를 실행하거나 visual PASS를 판정하지 않았다.

## 확인한 본질

- 대상 runtime asset은 `MAP_AC527A4AF171_BG_ELG_ARYANORB_BRIDGE01E_SM` 하나다.
- WModel은 mesh payload 1개와 material payload 1개를 가지며 `animations=0`, `skeleton=no`다. `sections=2`는 정상/파괴 상태 두 개가 아니다.
- 원본 glTF도 node 1, mesh 1, primitive 2이고 animation, skin, morph target이 없다.
- primitive 0은 상부 bridge 판 680 triangles, primitive 1은 의도된 하부 fence/truss 216 triangles다. 두 primitive 사이 동일 triangle은 0개이므로 파괴용 중복 mesh가 아니다.
- 원본 대상 8개 `StaticMeshActor/StaticMeshComponent`에는 `DepthBias`, `TranslucencySortPriority`, `HiddenGame`, destruction/fracture replacement가 없다.
- z-fighting은 texture 파일이 아니라 독립 static placement의 near-coplanar geometry가 같은 depth를 쓰기 때문에 발생한다.

중앙의 export 410/490과 411/495는 같은 mesh를 90도 회전해 교차한다. 시작 지점 왼쪽의 export 442/444/458/474는 모두 Y `-142.735918`에서 `+45/-45/+135/-135`도로 교차한다. 후자 네 placement의 submesh0 triangle overlap graph는 모든 pair가 연결된 K4다.

- 인접 회전 pair의 0.1mm 이내 coplanar overlap: 약 `3.58m²`
- 반대 방향 pair의 overlap: 약 `27.31m²`
- Character Select camera 약 8.7m, near 0.1m, D24 depth에서 1 LSB: 약 `0.045mm`
- 2mm step: 약 44 LSB

texture 교체는 외형을 훼손하고, actor 삭제는 각 회전 조각의 고유 영역을 없앤다. geometry trimming은 별도 WModel과 UV/normal/tangent/collision 재생성이 필요하고, per-placement render depth bias는 현재 schema/batch/rasterizer 계약에 없으며 원본에도 근거가 없다. 따라서 authoring placement Y만 분리하는 것이 현재 가장 좁고 원본 보존적인 해결이다.

## 실제 변경

- 기존 중앙 correction 유지:
  - export 490 Y: `-142.711572 -> -142.713572`
  - export 495 Y: `-142.71916 -> -142.72116`
- 누락됐던 왼쪽 4-way group correction:
  - export 442 Y: `-142.735918` 유지, anchor
  - export 444 Y: `-142.735918 -> -142.737918`
  - export 458 Y: `-142.735918 -> -142.739918`
  - export 474 Y: `-142.735918 -> -142.741918`
- 전체 6mm spread 뒤에도 직상 layer와 14.758mm 이상 간격을 유지한다.
- Imported placement, MapCatalog, mapassets, WModel과 texture는 변경하지 않았다.
- publisher로 `Client/Bin/DataFiles/Map/LV_LOBBY_CLASSSELECT_SL00.mapplacements`를 갱신했다.
- `Tools/ProjectAudit/Test-CharacterSelectCenterDepthSeparation.ps1`은 Imported 대비 허용 차이를 위 다섯 Y 필드로 제한하고 원래 textured asset ID와 55/803 count를 함께 검사한다.

## 자동 검증

| 검증 | 결과 |
|---|---|
| `Publish-MapAuthoring.ps1 -AreaId LV_LOBBY_CLASSSELECT_SL00` | PASS, placements 803, SHA-256 `b13a6aa3c557699fea0477d71f896fa726290516082a6cef92f71cb7aa4420c1` |
| `Test-CharacterSelectCenterDepthSeparation.ps1` | PASS |
| Imported 원본과 authoring 차이 제한 | PASS, export 444/458/474/490/495 Y만 변경 |
| Authoring/runtime placement byte identity | PASS |
| Source/runtime asset catalog identity | PASS, 55 assets, SHA-256 `bbfdffd2f80177226f1cda8ba9dc4b17921fe973c579e4a98c67b2da25332098` |
| 여덟 대상 original asset ID 유지 | PASS |
| Obsolete material variant 참조/resource | PASS, 0 |
| MapCatalog JSON 및 55/803 contract | PASS |
| `ModelAssetConverter info` | PASS, sections 2 / animations 0 / skeleton no |
| `git diff --check` | PASS |

C++/shader/public header를 변경하지 않은 data placement, PowerShell harness와 문서 변경이므로 이번 교정에서는 Client build를 다시 실행하지 않았다.

## 수동 검증과 남은 경계

- 실행 중인 Client는 이미 로드한 placement를 유지할 수 있으므로 Server + Client를 다시 시작하거나 Lobby에서 Character Select에 재진입한다.
- 사용자가 시작 지점 왼쪽 격자와 중앙 교차부에서 카메라를 움직이며 깜빡임/무늬 전환이 사라졌는지 확인한다.
- 원래 bridge/fence diffuse와 normal, 하부 truss, 중앙 실루엣이 그대로인지 함께 확인한다.
- 사용자 관찰 전에는 visual PASS로 기록하지 않는다.
