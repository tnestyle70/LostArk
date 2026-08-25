# Character Select 중앙 교차 바닥 depth 분리 구현 계획서

## 목표

`LV_LOBBY_CLASSSELECT_SL00` 중앙과 시작 지점 왼쪽의 `BG_ELG_ARYANORB_BRIDGE01E_SM` 교차 조각이 카메라 이동 때 서로 번갈아 선택되는 현상을 제거한다. 원본 actor, mesh section과 texture는 유지하고 실제로 겹치는 placement에만 안정적인 깊이 간격을 부여한다.

## 현재 실측

- 제품 배치는 `Data/Maps/Authoring/LV_LOBBY_CLASSSELECT_SL00/LV_LOBBY_CLASSSELECT_SL00.mapplacements`가 정본이고 `Client/Bin/DataFiles/Map/LV_LOBBY_CLASSSELECT_SL00.mapplacements`가 런타임 publish 결과다.
- 원본 보존본 `Data/Maps/Imported/LV_LOBBY_CLASSSELECT_SL00/LV_LOBBY_CLASSSELECT_SL00.mapplacements`에는 803개 배치가 있다.
- 중앙 좌표는 `(-772.017, -142.7, 197.538)`이며, export 410/490과 411/495가 같은 bridge mesh를 각각 90도 회전해 교차시킨다.
- 시작 지점 왼쪽의 export 442/444/458/474도 같은 mesh를 `+45/-45/+135/-135`도로 회전해 모두 Y `-142.735918`에 둔다. 실제 submesh0 triangle 교차 결과 여섯 placement pair가 모두 면적으로 겹친다.
- 원본 UE3 component는 export 410/490에 같은 material override `bg_elg_aryanorb_bridge01a_02_mi_khy_01`, export 411/495에 같은 override `bg_rad_abrelshud_landmark03b_01_mi_psy_02`를 둔다. 두 material parent는 모두 opaque다.
- glTF 삼각형과 런타임 placement transform을 결합한 수치 검사에서 410/490의 교차 표면 높이 차이는 최대 `7.92e-10m`, 411/495는 최대 `4.19e-5m`였다. 기본 Character Select 카메라는 near `0.1m`, far `2000m`, 중앙까지 약 `8.7m`이므로 두 번째 쌍도 깊이 양자화 경계에 걸린다.
- WModel은 static submesh 2개, animation 0개, skeleton 없음이다. destroy/damage state mesh가 함께 재생되는 경로가 아니며, 네 독립 placement의 coplanar overlap이 직접 원인이다.
- 442/444/458/474의 submesh0 overlap은 인접 회전 pair가 약 `3.58m²`, 반대 방향 pair가 약 `27.31m²`다. D24 depth에서 8.7m 지점의 1 LSB는 약 `0.045mm`이므로 2mm step은 약 44 LSB다.
- 410과 411은 서로 다른 layer이며 표면 간격이 약 `7.59mm`다. 이 둘을 삭제하거나 합치지 않는다.

## 원인


```text
원본의 회전된 bridge 조각을 동일 높이에 반복 배치
  -> 런타임 좌표 변환
  -> 중앙/왼쪽 교차 영역에서 같은 depth 또는 약 1 depth step
  -> 카메라 위치에 따라 depth winner가 변경
  -> 중앙 텍스처가 울렁이거나 번갈아 보임
```

material override 손실은 별도 복원 대상이지만 이번 z-fighting의 직접 원인은 독립 static placement의 coplanar geometry다. texture/material 교체, actor 삭제, alpha 전환이나 파괴 상태 분기를 이번 수정에 섞지 않는다.

## 변경 파일

- `Data/Maps/Authoring/LV_LOBBY_CLASSSELECT_SL00/LV_LOBBY_CLASSSELECT_SL00.mapplacements`
- `Client/Bin/DataFiles/Map/LV_LOBBY_CLASSSELECT_SL00.mapplacements`
- `Tools/ProjectAudit/Test-CharacterSelectCenterDepthSeparation.ps1`

원본 증거인 `Data/Maps/Imported/...`는 수정하지 않는다. 새 C++ 파일과 project/filter 등록은 없다.

## G00. 중앙 교차 배치의 안정적인 깊이 간격

- export 490의 Y를 `-142.711572`에서 `-142.713572`로 2mm 낮춘다.
- export 495의 Y를 `-142.71916`에서 `-142.72116`으로 2mm 낮춘다.
- export 442는 Y `-142.735918`을 anchor로 유지한다.
- export 444/458/474의 Y를 각각 `-142.737918/-142.739918/-142.741918`로 2mm step만큼 낮춘다.
- export 410/411/442와 나머지 transform, stable placement ID, 원래 asset ID, asset count 55와 placement count 803은 유지한다.
- 2mm는 기본 카메라 거리에서 depth step보다 충분히 크고, 원본 7.59mm layer 간격보다 작아서 layer 순서를 뒤집지 않는다.
- 하단 네 조각의 전체 spread는 6mm이며 직상 layer와 14.758mm 이상 간격을 보존한다. 0.1mm 이내의 면적 coplanar overlap은 교정 뒤 0이다.

## G01. 회귀 하네스

- imported 보존본이 원본 Y와 원래 textured asset ID를 유지하는지 검사한다.
- authoring은 imported와 export 444/458/474/490/495의 Y에서만 다를 수 있다.
- authoring과 runtime publish가 정확한 2mm correction을 함께 가지며 byte-equivalent인지 검사한다.
- 여덟 대상 placement가 모두 원래 bridge asset을 사용하고 catalog 55, placement 803이 유지되는지 검사한다.
- material/texture/asset ID나 다른 위치·회전·스케일이 함께 바뀌면 실패한다.

## 검증

```powershell
powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Test-CharacterSelectCenterDepthSeparation.ps1
powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Invoke-ProjectAudit.ps1
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release
git diff --check
```

사용자가 제공한 스크린샷은 occurrence 진단 입력으로만 사용한다. 에이전트는 Client를 실행하거나 visual PASS를 대신 판정하지 않으며 자동 검증은 loader/data/depth 계약으로 수행한다.
