# Character Select 중앙 교차 바닥 depth 분리 구현 계획서

## 목표

`LV_LOBBY_CLASSSELECT_SL00` 중앙의 `BG_ELG_ARYANORB_BRIDGE01E_SM` 교차 조각이 카메라 이동 때 서로 번갈아 선택되는 현상을 제거한다. 원본 actor와 서로 다른 material layer는 유지하고, 실제로 교차하는 두 쌍에만 안정적인 깊이 간격을 부여한다.

## 현재 실측

- 제품 배치는 `Data/Maps/Authoring/LV_LOBBY_CLASSSELECT_SL00/LV_LOBBY_CLASSSELECT_SL00.mapplacements`가 정본이고 `Client/Bin/DataFiles/Map/LV_LOBBY_CLASSSELECT_SL00.mapplacements`가 런타임 publish 결과다.
- 원본 보존본 `Data/Maps/Imported/LV_LOBBY_CLASSSELECT_SL00/LV_LOBBY_CLASSSELECT_SL00.mapplacements`에는 803개 배치가 있다.
- 중앙 좌표는 `(-772.017, -142.7, 197.538)`이며, export 410/490과 411/495가 같은 bridge mesh를 각각 90도 회전해 교차시킨다.
- 원본 UE3 component는 export 410/490에 같은 material override `bg_elg_aryanorb_bridge01a_02_mi_khy_01`, export 411/495에 같은 override `bg_rad_abrelshud_landmark03b_01_mi_psy_02`를 둔다. 두 material parent는 모두 opaque다.
- glTF 삼각형과 런타임 placement transform을 결합한 수치 검사에서 410/490의 교차 표면 높이 차이는 최대 `7.92e-10m`, 411/495는 최대 `4.19e-5m`였다. 기본 Character Select 카메라는 near `0.1m`, far `2000m`, 중앙까지 약 `8.7m`이므로 두 번째 쌍도 깊이 양자화 경계에 걸린다.
- 410과 411은 서로 다른 layer이며 표면 간격이 약 `7.59mm`다. 이 둘을 삭제하거나 합치지 않는다.

## 원인


```text
원본의 회전된 bridge 조각
  -> 런타임 좌표 변환
  -> 중앙 교차 영역에서 같은 depth 또는 약 1 depth step
  -> 카메라 위치에 따라 depth winner가 변경
  -> 중앙 텍스처가 울렁이거나 번갈아 보임
```

material override 손실은 별도 복원 대상이지만, 두 원본 material이 모두 opaque이므로 이번 z-fighting의 직접 원인은 깊이 간격 소실이다. material을 임의로 alpha 처리하지 않는다.

## 변경 파일

- `Data/Maps/Authoring/LV_LOBBY_CLASSSELECT_SL00/LV_LOBBY_CLASSSELECT_SL00.mapplacements`
- `Client/Bin/DataFiles/Map/LV_LOBBY_CLASSSELECT_SL00.mapplacements`
- `Tools/ProjectAudit/Test-CharacterSelectCenterDepthSeparation.ps1`

원본 증거인 `Data/Maps/Imported/...`는 수정하지 않는다. 새 C++ 파일과 project/filter 등록은 없다.

## G00. 중앙 교차 배치의 안정적인 깊이 간격

- export 490의 Y를 `-142.711572`에서 `-142.713572`로 2mm 낮춘다.
- export 495의 Y를 `-142.71916`에서 `-142.72116`으로 2mm 낮춘다.
- export 410/411과 나머지 transform, stable placement ID, asset ID, placement count 803은 유지한다.
- 2mm는 기본 카메라 거리에서 depth step보다 충분히 크고, 원본 7.59mm layer 간격보다 작아서 layer 순서를 뒤집지 않는다.

## G01. 회귀 하네스

- imported 보존본이 원본 Y를 유지하는지 검사한다.
- authoring과 runtime publish가 정확한 2mm correction을 함께 가지는지 검사한다.
- 두 파일이 byte-identical이고 placement count와 survivor ID가 유지되는지 검사한다.
- 다른 위치·회전·스케일이 함께 바뀌면 실패한다.

## 검증

```powershell
powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Test-CharacterSelectCenterDepthSeparation.ps1
powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Invoke-ProjectAudit.ps1
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release
git diff --check
```

사진이나 스크린샷은 검증에 사용하지 않는다. 런타임 검증은 loader/shader/runtime 오류 로그와 배치 계약으로만 판정한다.
