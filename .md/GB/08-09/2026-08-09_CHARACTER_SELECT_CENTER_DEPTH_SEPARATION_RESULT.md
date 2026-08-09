# Character Select 중앙 교차 바닥 depth 분리 수정 결과

## 완료 상태

`LV_LOBBY_CLASSSELECT_SL00` 중앙의 교차 bridge 조각 두 쌍에 안정적인 2mm 깊이 간격을 적용했다. placement 개수, stable placement ID, asset ID와 Imported 원본 증거는 유지했다.

이미지·스크린샷 확인은 수행하지 않았다. 원인과 결과는 원본 package actor/material, glTF triangle, placement transform, depth 수치와 audit로만 판정했다.

## 확인한 원인

- 중앙 mesh는 `MAP_AC527A4AF171_BG_ELG_ARYANORB_BRIDGE01E_SM`이다.
- export 410/490과 411/495는 같은 bridge mesh를 각각 회전해 교차 배치한다.
- 원본 UE3 actor의 material override는 서로 다른 layer 의도를 보존하지만 모두 opaque parent다.
- glTF triangle과 runtime scale `0.01`을 적용한 교차면 계산 결과:
  - 410/490 최대 높이 차이: 약 `7.913e-10m`
  - 411/495 최대 높이 차이: 약 `4.187e-5m`
- 두 값 모두 중앙 교차부에서 안정적인 depth 우선순위를 만들지 못하므로 카메라 이동에 따라 z-fighting이 발생할 수 있다.

교차 조각은 각자 넓은 고유 영역을 가지므로 중복 actor 삭제는 올바른 수정이 아니다. material을 임의의 alpha 경로로 바꾸는 것도 원본 opaque 계약을 훼손한다.

## 실제 변경

- export 490 Y: `-142.711572 -> -142.713572`
- export 495 Y: `-142.71916 -> -142.72116`
- 다음 authoring/runtime 문서를 같은 내용으로 갱신했다.
  - `Data/Maps/Authoring/LV_LOBBY_CLASSSELECT_SL00/LV_LOBBY_CLASSSELECT_SL00.mapplacements`
  - `Client/Bin/DataFiles/Map/LV_LOBBY_CLASSSELECT_SL00.mapplacements`
- `Data/Maps/Imported/...`는 원본 증거로 유지했다.
- `Tools/ProjectAudit/Test-CharacterSelectCenterDepthSeparation.ps1`을 추가했다.

2mm는 원본의 서로 다른 layer 간격 약 7.59mm보다 작고, 해당 교차면의 기존 오차보다 충분히 크다. 다른 transform이나 placement는 변경하지 않았다.

## 자동 검증

| 검증 | 결과 |
|---|---|
| `Test-CharacterSelectCenterDepthSeparation.ps1` | PASS |
| Imported 원본 좌표 유지 | PASS |
| Authoring/runtime byte identity | PASS |
| placement count 803 및 대상 ID 유지 | PASS |
| ProjectAudit `maps.character-select-area-contract` | PASS, assets 55 / placements 803 / manifest 55 |
| Release Client build | PASS |
| Release Client 10초 startup probe | PASS |

전체 ProjectAudit의 남은 실패 2개는 `maps.product-editor-visual-scope`와 다른 세션의 rendering profile FXAA 범위이며 Character Select area 계약과 depth separation audit는 통과했다.

## 수동 검증과 남은 경계

- 이미지 확인 금지 규칙에 따라 중앙 바닥의 육안 비교는 미실행이다.
- 이번 변경은 현재 runtime이 실제로 소비하는 placement 좌표의 deterministic depth 순서를 고정한다.
- 원본 actor별 material override를 converter가 완전히 보존하는 작업은 별도 asset-import 수직 슬라이스이며 이번 z-fighting 수정 범위에는 포함하지 않았다.
