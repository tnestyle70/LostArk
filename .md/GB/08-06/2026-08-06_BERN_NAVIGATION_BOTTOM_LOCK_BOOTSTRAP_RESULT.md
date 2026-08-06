# Bern Navigation 바닥 고정 Bounds / Authoring Bootstrap 결과

## 1. 완료 상태

| 항목 | 상태 | 실제 결과 |
|---|---|---|
| Bottom Y 고정 편집 | 구현 완료 | Position XZ, Bottom Y, Size XZ, Height from Bottom, 계산 Top Y로 Nav Bounds 편집 |
| 기존 bake 형식 호환 | 완료 | `NAVGRID_BAKE_DESC`와 `.navsource` 형식 변경 없음 |
| Bern navigation authoring 등록 | 완료 | MapCatalog에 Bern source/paint 경로 등록 |
| Bern source 부재 bootstrap | 완료 | source가 없어도 Bern Area stage 후 Place Nav Bounds/Bake 가능 |
| source/paint-only rollback | 완료 | 빈 blocker path를 backup/reset/restore 대상에서 제외 |
| Client Debug 빌드 | PASS | Client.exe 링크 성공, 오류 0 |
| 실제 Bern navsource 생성 | 사용자 MapTool bake 대기 | 범위를 임의 추측해 생성하지 않음 |
| Bern Server runtime 적용 | 미실시 | 실제 bake 통계·연결성 검증 뒤 별도 변경 단위 |
| Bern Area 카메라 재이동 | 구현 완료·수동 확인 대기 | 현재 선택 재클릭과 `Focus Area`가 stable Bern player spawn으로 재포커스 |

## 2. 변경 내용

### MapTool

- `Place Nav Bounds`가 고른 바닥은 기존처럼 Bounds bottom이 된다.
- `Height from Bottom`을 바꾸면 다음 식으로 중심만 재계산하므로 Bottom Y가 유지된다.

```cpp
size.y = height;
position.y = bottomY + height * 0.5f;
```

- X/Z 위치와 크기는 별도 2축 값으로 편집한다.
- Top Y는 `bottomY + size.y` 계산값으로 표시한다.
- Bern만 navigation source가 없을 때 정상 bootstrap을 허용한다.
- blocker 문서가 없는 source/paint-only Area에서 초기 Bake가 빈 path 삭제 오류로 rollback되지 않게 했다.

### 데이터와 팀 계약

- `Data/Maps/MapCatalog.json` Bern row:
  - `Data/Navigation/LV_BER_BERNCASTLE.navsource`
  - `Data/Navigation/LV_BER_BERNCASTLE.navpaint`
- ProjectAudit는 위 두 경로를 Bern의 승인된 bootstrap 경로로 검사한다. 실제 source/paint 부재는 최초 bake 전까지만 허용한다.
- 팀 Area 가이드와 gameplay handbook은 Bern을 authoring bootstrap 상태로 표시하고 Server runtime 미활성 경계를 유지한다.

### Area 선택과 카메라 이동

- TEST Map Editor 진입 시 첫 Area인 Bern을 자동 stage하는 기존 흐름은 유지했다.
- 이미 활성화된 Bern은 기존에 다시 눌러도 `selected` 분기에서 아무 동작도 하지 않았지만, 이제 현재 Area를 다시 누르면 카메라 재포커스를 실행한다.
- `Focus Area` 버튼을 추가해 50,017 placement를 다시 stage하지 않고 현재 Area의 카메라만 즉시 되돌릴 수 있다.
- Bern은 `player.spawn.bern.entry`, Valtan은 `player.spawn.valtan.entry`를 우선 사용한다. stable spawn이 없을 때만 placement bounds로 fallback한다.
- camera weak reference가 만료된 경우 기존 Development `Layer_Camera`에서 `CCamera_Free`를 다시 취득한다. Area 데이터 전환 성공과 카메라 포커스 실패는 분리해 상태로 보고한다.

## 3. 자동 검증

| 검증 | 결과 |
|---|---|
| MapCatalog JSON parse | PASS |
| 기존 Server navigation Validate | PASS: Valtan 392×312, Training 32×32, Character Select 62×62 |
| Client x64 Debug build | PASS, `Client/Bin/Debug/Client.exe` 링크 성공 |
| Client x64 Release build | PASS, `Client/Bin/Release/Client.exe` 링크 성공 |
| MapTool Area refocus 컴파일 | PASS, `Focus_ActiveEditorAreaCamera` consumer와 링크 확인 |
| MapTool.cpp CP949 decode | PASS |
| `git diff --check` | PASS |
| 전체 ProjectAudit | 기존/동시 작업 4건으로 FAIL |

ProjectAudit 실패 항목은 이번 Bern 변경이 아니다.

- `projects.data-source-visibility`: expected 220, project/filter 218
- `effect.g09-authoring-world-runtime-boundary`
- `actors.catalog-assets`: DimensionMaster model 리소스 부재
- `actors.dimensionmaster-runtime-animation`

Bern editor workspace policy와 navigation publisher 검증은 해당 실행에서 통과했다.

실제 TEST → F1 → Area → Bern 화면 이동은 GUI 수동 확인이 남아 있다. 자동 검증에서는 Bern gameplay 문서의 `player.spawn.bern.entry` 존재와 Client 링크 성공까지만 확인했다.

## 4. 수동 Bern bake 절차

1. Debug Server와 Client를 실행한다.
2. Lobby → Test → F1 → Map Tool → Bern을 선택하고 50,017 placement stage가 끝날 때까지 기다린다.
3. Navigation → Bake → Place Nav Bounds를 누르고 실제 이동 바닥을 클릭한다.
4. `Position XZ`와 `Size XZ`로 실제 이동 구역만 감싼다.
5. `Bottom Y`는 선택 바닥에 유지하고 `Height from Bottom`을 올려 필요한 도로 높이만 포함한다.
6. 표시된 Top Y가 지붕·장식 상단까지 올라가지 않았는지 확인한다.
7. Cell Size 0.5, Max Slope 50을 첫 측정 기준으로 Bake한다.
8. Walkability overlay의 실제 도로 연결, player spawn `(144.8, 42.7, -70.3)`, NPC와 Valtan 이동 trigger 주변을 확인한다.
9. 잘못된 표면은 즉시 Force Walkable로 덮지 말고 Bounds Y와 포함 범위를 먼저 수정해 다시 Bake한다.
10. 결과가 맞을 때 Save Navigation으로 paint authoring을 저장한다.

## 5. 다음 단계

실제 bake 뒤 다음 수치를 기록하고 제품 runtime 적용 여부를 결정한다.

- Bounds position/size, Bottom Y/Top Y
- width/height/cell count
- resolved/base walkable/no-surface 수
- player spawn, NPC, trigger cell의 walkable/height
- 시작점 기준 연결 component 수
- 고립된 맵 밖·지하·지붕 surface 수
- bake와 Area load 시간

이 결과가 확인되기 전에는 Bern을 Server 필수 navigation room으로 바꾸거나 publisher 출력 목록에 추가하지 않는다.
