# MapTool Force Walkable Override RESULT

작성일: 2026-08-05

## 1. 결론

MapTool Navigation의 노란색 `BLOCKED` 셀 중 높이가 해석된 셀을 사용자가 강제로
초록색 `WALKABLE` 셀로 바꾸고 저장할 수 있게 구현했다. 기존 bake/source 데이터는
삭제하거나 재작성하지 않는다. 수동 보정은 `Data/Navigation/<AreaId>.navpaint`에
override로 저장되고 `Publish-ServerNavigation.ps1`이 Client/Server runtime
`.navgrid`에 같은 최종 판정을 반영한다.

## 2. 실제 구현

### 2.1 문서 상태

`CNavGridPaintDocument`는 셀마다 다음 세 상태를 소유한다.

```text
INHERIT          -> bake의 baseWalkable을 사용
FORCE_BLOCKED    -> 최종 이동 불가
FORCE_WALKABLE   -> 최종 이동 가능
```

- 기존 `.navpaint` version 1의 `x z` 행은 `FORCE_BLOCKED`로 호환 로드한다.
- 신규 저장은 version 2이며 `x z BLOCKED|WALKABLE` 형식이다.
- 중복 좌표, 잘못된 상태, source와 다른 Area/grid header, 높이가 없는 셀은 전체 load를 실패시킨다.
- 저장은 임시 파일을 만든 뒤 원자 교체한다.
- `NO_SURFACE` 셀은 강제로 이동 가능하게 만들지 않는다.

### 2.2 MapTool UI

Navigation의 `Walkability` 모드에 다음 명령을 연결했다.

- `Block`: 강제 이동 불가
- `Force Walkable`: bake에서 막힌 노란 셀도 강제 이동 가능
- `Reset`: 수동 override 제거 후 bake 상태 상속

`Force Walkable`로 칠한 셀은 `Get_CellState()`에서 즉시 `WALKABLE`로 판정되므로
overlay가 초록색으로 바뀐다. `Destruction Area`의 `Add Cells / Remove Cells` 계약은
변경하지 않았다.

### 2.3 Publisher

`Publish-ServerNavigation.ps1`은 navpaint v1과 v2를 모두 읽는다. 최종 walkable 계산은
resolved surface에 한해 `WALKABLE override -> BLOCKED override -> baseWalkable` 우선순위를
사용한다. runtime `.navgrid` binary 형식은 바꾸지 않았다.

`-Mode ContractTest`를 추가해 다음을 synthetic 2-cell fixture로 검증한다.

- baked walkable 셀을 `BLOCKED`로 반전
- baked blocked 셀을 `WALKABLE`로 반전
- 지원하지 않는 version 거부
- 중복 override 거부
- 다른 Area ID 거부

## 3. 변경 파일

- `Client/Public/NavGridPaintDocument.h`
- `Client/Private/NavGridPaintDocument.cpp`
- `Client/Public/MapTool.h`
- `Client/Private/MapTool.cpp`
- `Tools/NavigationPipeline/Publish-ServerNavigation.ps1`
- `.md/TEAM/AREA_DATA_LAYER_GUIDE.md`
- `.md/GB/08-05/2026-08-05_MAPTOOL_FORCE_WALKABLE_OVERRIDE_PLAN.md`
- `.md/GB/08-05/2026-08-05_MAPTOOL_FORCE_WALKABLE_OVERRIDE_RESULT.md`

새 C++ 파일은 없으므로 `.vcxproj`와 `.vcxproj.filters`는 변경하지 않았다.

## 4. 검증 결과

### PASS

1. `Publish-ServerNavigation.ps1 -Mode ContractTest`
   - navpaint v2 강제 차단/이동 가능 반전과 세 가지 거부 사례 통과
2. `Publish-ServerNavigation.ps1 -Mode Validate`
   - Valtan: `392x312`, walkable `20,761`
   - Training: `32x32`, walkable `1,024`
   - Character Select: `62x62`, walkable `2,176`
   - 기존 v1 authoring 결과 수치 유지
3. Client x64 Debug build
   - 오류 0개, `Client/Bin/Debug/Client.exe` 생성
4. 변경 파일 `git diff --check`
   - 공백 오류 없음
5. 인코딩
   - `MapTool.h/.cpp`: CP949, BOM 없음, CRLF 유지
   - 나머지 수정 코드/스크립트/Markdown: UTF-8, BOM 없음

### 전체 ProjectAudit의 기존 실패

ProjectAudit 내부의 gameplay balance와 Server navigation 검증은 통과했다. 전체 감사의
최종 결과는 이번 변경과 무관한 기존 항목 때문에 실패했다.

- `Character/DimensionMaster/DimensionMaster_Character.wmodel` payload 누락
- Effect G1 document boundary 감사 항목 미충족

이번 작업에서는 리소스 payload와 Effect 문서를 수정하지 않았다.

### 수동 확인 필요

실행 화면에서 다음 smoke는 사용자가 확인해야 한다.

1. Debug Client 실행
2. Lobby -> Test -> F1 -> Map Tool
3. Character Select 또는 Valtan -> Navigation -> Walkability
4. 노란 셀에 `Force Walkable` 브러시 적용
5. 즉시 초록색 표시 확인
6. `Save Navigation` 후 Area 재로드
7. 같은 셀이 계속 초록색인지 확인
8. publisher `Publish` 후 Server 재시작 및 실제 이동 확인

## 5. 사용 시 주의사항

- 이 기능은 높이가 존재하지만 slope/기본 판정 때문에 막힌 셀을 수동 승인하는 기능이다.
- 실제 바닥 메시가 없거나 높이를 해석하지 못한 `NO_SURFACE` 영역을 만드는 기능은 아니다.
- 절벽, 낙하 구간, 벽 내부를 강제로 초록색으로 만들면 Server path가 그 셀을 사용할 수 있으므로
  화면과 충돌 구조를 확인한 셀에만 적용한다.
- MapTool 저장만으로 제품 Server에 즉시 반영되지 않는다. publisher Publish와 Server 재시작이 필요하다.
