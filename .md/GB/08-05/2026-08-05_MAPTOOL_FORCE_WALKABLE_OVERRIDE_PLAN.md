# MapTool Force Walkable Override PLAN

작성일: 2026-08-05

## C1~C8

- C1 기준계 ★★★: override는 NavGrid의 stable `(cellX, cellZ)` 좌표를 사용한다.
- C2 이동>계산 ★★☆: MapTool은 authoring override만 저장하고 runtime `.navgrid` 계산은 publisher가 수행한다.
- C3 공유는 비싸다 ★☆☆: 기존 source cell과 수동 override를 중복 복사하지 않는다.
- C4 수명은 선언된다 ★☆☆: override는 Area별 `.navpaint` 수명이며 Level 객체 수명과 분리한다.
- C5 이산화와 오차 ★★☆: 이미 높이가 해결된 셀만 Force Walkable 대상으로 허용한다.
- C6 가지치기 ★★☆: 같은 상태 재도장은 dirty로 만들지 않는다.
- C7 권위와 정합성 ★★★: `Data/Navigation/*.navpaint`가 수동 수정 정본이고 publisher만 runtime을 교체한다.
- C8 검증이 병목 ★★★: legacy v1, 신규 v2, 저장·재로드, publisher 결과를 함께 검증한다.

## 문제 해결 ①~⑤

① 문제·제약: 현재 `Erase`는 manual block만 지우므로 Bake가 `baseWalkable=false`로 만든 노란 셀을 초록 walkable로 바꿀 수 없다.
② 단순 해법의 문제: `.navsource`를 직접 고치면 재베이크 때 사라지고 Bake 결과와 수동 판단의 소유권이 섞인다.
③ 해결 방식: `.navpaint` version 2에 `BLOCKED`와 `WALKABLE` override를 저장하고 최종 상태에서 override를 Bake보다 우선한다.
④ 비교: 기존 v1 blocked 행은 모두 `FORCE_BLOCKED`로 staged load해 손실 없이 호환한다.
⑤ 대가: paint parser와 UI action이 삼중 상태가 되지만 runtime `.navgrid`와 Engine/Server 형식은 바꾸지 않는다.

## 자료구조와 불변식

```text
INHERIT          -> baseWalkable 사용
FORCE_BLOCKED    -> 최종 walkable false
FORCE_WALKABLE   -> 최종 walkable true
```

- `FORCE_WALKABLE`은 `surfaceResolved=true`인 셀에만 저장한다.
- paint v1의 `(x,z)` 행은 `FORCE_BLOCKED`로 읽는다.
- paint v2의 행은 `(x,z,BLOCKED|WALKABLE)`이다.
- 중복 좌표, 범위 밖 좌표, 높이 미해결 override, 알 수 없는 상태는 전체 load를 실패시키고 기존 문서를 유지한다.
- 저장은 cell Z, cell X 순서로 결정적이며 임시 파일을 원자 교체한다.
- MapTool은 runtime `.navgrid`를 직접 쓰지 않는다.

## 파일 목록

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 수정 | `C:/Users/USER/source/졸업팀폴/LostArk/Client/Public/NavGridPaintDocument.h` | paint override enum과 문서 계약 |
| 수정 | `C:/Users/USER/source/졸업팀폴/LostArk/Client/Private/NavGridPaintDocument.cpp` | v1/v2 staged load, v2 save, 최종 walkable 판정 |
| 수정 | `C:/Users/USER/source/졸업팀폴/LostArk/Client/Public/MapTool.h` | Force Walkable edit action |
| 수정 | `C:/Users/USER/source/졸업팀폴/LostArk/Client/Private/MapTool.cpp` | 브러시 UI·명령·진단 연결 |
| 수정 | `C:/Users/USER/source/졸업팀폴/LostArk/Tools/NavigationPipeline/Publish-ServerNavigation.ps1` | v1/v2 override를 runtime walkable에 적용 |
| 수정 | `C:/Users/USER/source/졸업팀폴/LostArk/.md/GB/08-05/2026-08-05_MAPTOOL_FORCE_WALKABLE_OVERRIDE_RESULT.md` | 실제 검증 증거 |

새 C++ 파일은 없으므로 `.vcxproj`와 `.vcxproj.filters` 등록 변경은 없다.

## 데이터 형식

legacy v1:

```text
LOSTARK_NAVGRID_PAINT 1 "LV_LUT_HEARTRB_ED" 392 312 0.5 -6 -165 1
46 292
```

신규 v2:

```text
LOSTARK_NAVGRID_PAINT 2 "LV_LUT_HEARTRB_ED" 392 312 0.5 -6 -165 2
46 292 BLOCKED
47 292 WALKABLE
```

## H 계약

```cpp
enum class NAVGRID_PAINT_OVERRIDE : uint8_t
{
	INHERIT,
	FORCE_BLOCKED,
	FORCE_WALKABLE,
};

bool_t Paint(
	int32_t cellX,
	int32_t cellZ,
	uint32_t brushRadius,
	NAVGRID_PAINT_OVERRIDE overrideState);

uint32_t Get_BlockedCount() const;
uint32_t Get_ForcedWalkableCount() const;
```

`CNavGridPaintDocument`가 source와 override의 최종 합성을 소유한다. `CMapTool`은 선택한 edit action만 전달하고 저장 형식을 해석하지 않는다.

## CPP 호출 흐름

```text
CMapTool::Update_WorldInteraction
-> CMapTool::Try_PaintNavigation
-> CNavGridPaintDocument::Paint
-> Save Navigation
-> CNavGridPaintDocument::Save_Paint(v2)
-> Publish-ServerNavigation.ps1
-> Client/Server runtime .navgrid
```

최종 walkable 판정:

```cpp
bool_t isWalkable = false;
if (hasSurface)
{
	switch (m_CellOverrides[index])
	{
	case NAVGRID_PAINT_OVERRIDE::FORCE_WALKABLE:
		isWalkable = true;
		break;
	case NAVGRID_PAINT_OVERRIDE::FORCE_BLOCKED:
		isWalkable = false;
		break;
	case NAVGRID_PAINT_OVERRIDE::INHERIT:
	default:
		isWalkable = m_SourceCells[index].baseWalkable;
		break;
	}
}
```

## 적용 순서와 검증

1. 문서 자료형과 v1/v2 load/save를 교체한다.
2. MapTool에 `Force Walkable`과 `Reset`을 연결한다.
3. publisher에 동일한 override 계산을 반영한다.
4. 기존 v1 두 Area를 Validate해 호환을 확인한다.
5. 임시 v2 fixture에서 BLOCKED/WALKABLE 결과가 반대로 직렬화되는지 확인한다.
6. Client x64 Debug를 빌드한다.
7. `Publish-ServerNavigation.ps1 -Mode Validate`, Server contract test, ProjectAudit, `git diff --check`를 실행한다.
8. Debug Test → Map Tool → Valtan/Character Select → Navigation에서 노란 셀을 Force Walkable로 칠하고 Save → 재진입 시 초록색 유지 여부를 수동 확인한다.
