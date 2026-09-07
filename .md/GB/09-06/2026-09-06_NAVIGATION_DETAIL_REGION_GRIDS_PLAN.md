# 2026-09-06 네비게이션 세부 영역 격자(Detail Region) 계획서

## 문제와 제약

- 네비는 Area당 `.navgrid` 하나이고 그 안의 칸은 전부 같은 크기다. 쿠크
  (`LV_LUT_MIDNIGHTC_ED`)는 bake 범위가 2131×4662 m 라 100만 칸 상한 때문에 칸이
  4 m 이고, 서버 `.navpolicy` 낙차 1 m 와 합치면 허용 경사가 14° 로 떨어진다.
  bake 는 50° 까지 걷힘으로 넣으니 스폰(8.64 m)이 6칸짜리 섬이 되고, 4 m 보다 좁은
  메시는 아래 바닥과 한 칸을 나눠 써 어느 높이를 고르든 반쪽이 틀린다.
- 칸 크기를 줄이려면 bake 범위를 좁혀야 하는데, Area 가 격자 하나만 가지면
  미로·3~5 스테이지가 네비를 잃는다. 상한을 올리면 publisher(PowerShell 행 파싱)가
  수십 분 걸린다.
- 쿠크 스테이지는 서로 걸어서 못 간다. `jump.*` 트리거의 `movePlayer` 와 스테이지
  이동 시퀀스로만 넘어간다. 따라서 영역 사이 A* 는 필요 없다.

## 설계

Area 의 네비 = **기본 격자 1개 + 세부 영역(region) 격자 0~64개**.

- 영역 격자 id 는 `<AreaId>.<regionId>` 이고 이 문자열이 그대로 `.navsource`
  `.navpaint` `.navblockers` 헤더의 areaId, 런타임 `.navgrid` `.navpolicy`
  `.navblockers` 파일 stem 이 된다. `regionId` 는 `[A-Za-z0-9_-]{1,32}` 로 점을
  금지해 분리가 유일하다.
- 저작 매니페스트 `Data/Navigation/<AreaId>.navregions`:

```text
LOSTARK_NAVGRID_REGIONS 1 "LV_LUT_MIDNIGHTC_ED" 1
REGION "stage1" 1
```

  행은 `REGION "<regionId>" <runtimeMaximumStepHeight>`. publisher 가 같은 형식을
  `Client|Server/Bin/DataFiles/Navigation/<AreaId>.navregions` 로 다시 쓰고 서버는
  행의 step 이 영역의 `.navpolicy` 와 같은지 대조한다(정본은 하나, 교차 검증).
- 서버 `CServerNavigation` 은 이름·공개 API 를 유지한 채 `std::vector<CServerNavigation>
  m_Regions` 를 갖는다. 영역 객체는 하위 영역이 없는 평범한 `CServerNavigation` 이다.
  XZ 를 받는 공개 함수는 첫 점을 담는 영역이 있으면 그 격자에 위임하고 없으면 기존
  본문을 실행한다. 두 점 함수(`Find_Path` `Resolve_TraversalStep` `Has_LineOfSight`
  `Smooth_Path` `Find_PathToReachablePointWithinRadius`)는 시작점으로 고른다.
- 불변식
  - 영역 footprint 는 서로 겹치지 않는다(publisher·서버 둘 다 거부).
  - 영역 안에서 시작한 질의는 영역 밖 칸을 보지 않는다. 영역은 그 스테이지의 걷는
    범위 전체를 덮어야 하며, 플레이어는 영역을 걸어서 나갈 수 없다.
  - 기본 격자에서 시작한 경로는 영역 footprint 위를 기본 격자 칸으로 지날 수 있다.
    기본 격자는 어디서나 fallback 이고 영역은 그 위를 세밀하게 덮는다.
  - 영역은 runtime blocker/void/condition 을 갖지 않는다. 선언돼 있으면 로드 실패.
    condition 계열 함수는 기본 격자에만 적용한다.
  - `Get_CellSize()` 는 기본+영역 중 최소값이다. 호출자 5곳(GameRoom 4, PlayerSkillSystem
    1)이 전부 샘플링 간격으로 쓰므로 더 작은 값은 항상 안전하다.
  - 런타임 매니페스트가 **없으면** 영역 0개로 종전과 동일하게 동작한다. 있는데
    손상됐거나 가리키는 파일이 없으면 Area 로드 자체가 실패한다(transactional).
- MapTool 은 Navigation 패널 맨 위에 `Region` 콤보와 `Create Region` 을 둔다. 영역을
  고르면 문서·bake·paint·save 가 전부 그 격자 파일을 가리킨다. Bake 성공 시
  매니페스트에 영역을 등록한다.
- 이번 변경은 코드·publisher·검증기·문서까지이고 **매니페스트 데이터는 싣지 않는다**.
  `stage1` 영역의 `.navsource` 는 사용자가 툴에서 Nav Bounds 를 찍고 Bake 해야 생긴다.
  publisher 는 영역 0개인 쿠크를 종전과 같이 처리한다.

## 호출 흐름

```text
Server room 기동
  CGameRoom::Initialize -> CServerNavigation::Load(areaId)
    -> 기본 navgrid/navpolicy/navblockers (기존)
    -> Load_Regions(areaId): <AreaId>.navregions 없음 -> 0개
                              있음 -> 행마다 CServerNavigation::Load("<AreaId>.<regionId>")
                                       blocker 0개, step 일치, footprint 비겹침 검사
  질의 (예 Resolve_TraversalStep(from, to))
    -> Select_Region(from) -> 있으면 region.Resolve_TraversalStep(...)
                           -> 없으면 기존 본문

Publisher
  $grids 의 각 기본 격자마다
    Read-NavigationRegionManifest -> Convert-NavigationRegionGrids (영역별 source+paint)
    Assert-NavigationGrid (기본·영역 각각) + Assert-NavigationPlacements (합성 dispatch)
    Publish: 기본 6 + 매니페스트 2 + 영역당 6 파일을 한 transaction 으로 승격

MapTool
  Render_NavigationPanel -> Render_NavigationRegionControls (콤보/생성)
  Select_NavigationRegion -> Load_NavigationDocument (Resolve_SelectedNavigationContract)
  Bake_Navigation -> gridId 로 Build/Save_Source -> Commit_NavigationRegionManifest
  Save_Navigation -> 선택 격자의 navpaint (+ 매니페스트)
```

## 파일 역할

| 파일 | 변경 |
|---|---|
| `Server/Public/ServerNavigation.h` | `m_Regions`, `Get_RegionCount`, `Get_CellSize` 비인라인, private 영역 함수 4개 |
| `Server/Private/ServerNavigation.cpp` | `Load` 에 `Load_Regions`, 새 함수 5개, 공개 함수 10곳 dispatch prologue |
| `Server/Private/ServerGameplayContractTests.cpp` | `Run_ServerNavigationContractTests` 에 영역 fixture 8 case |
| `Tools/NavigationPipeline/Publish-ServerNavigation.ps1` | 매니페스트 읽기/영역 변환/겹침·합성 placement 검증/런타임 매니페스트/publish target/ContractTest |
| `Tools/Build/BuildDomains.json` | navigation outputs 에 `*.navregions` |
| `Client/Public/MapNavigationContract.h`, `Client/Private/MapNavigationContract.cpp` | `MAP_NAVIGATION_REGION`, `Resolve_Region`, 매니페스트 read/write, `Is_ValidRegionId` |
| `Client/Public/MapTool.h`, `Client/Private/MapTool.cpp` | 영역 선택 상태, `Resolve_SelectedNavigationContract`, `Load/Save/Bake` 가 선택 격자를 쓰도록, `Render_NavigationRegionControls` |
| `Tools/KoukuSaydonPipeline/validate_kouku_saydon_world_admission.py`, `test_...py` | 매니페스트·영역 격자 파싱, 합성 spawn 검증, 테스트 1건 |
| `CLAUDE.md`, `.md/TEAM/AREA_DATA_LAYER_GUIDE.md` | 네비 계약 한 단락 |

새 C++ 파일은 없다. `.vcxproj`/`.filters` 변경 없음.

`Client/Private/MapTool.cpp` 와 `MapTool.h` 에는 다른 세션의 미커밋 변경(컷신·도박판·
Mario 루프·카메라 샷, 61~3042행·14194~14353행)이 있다. 네비 편집은 8247·8601·14481·
15497·15715행의 함수 안에서만 앵커 기준으로 넣고 파일을 통째로 다시 쓰지 않는다.

---

## G1. Server — `CServerNavigation` 세부 영역

### G1-1. `Server/Public/ServerNavigation.h`

파일: `Server/Public/ServerNavigation.h`
작업: 교체
기준점: `float Get_CellSize() const { return m_fCellSize; }`
위치: 그 한 줄을 아래 블록으로 교체
필요한 이유: 영역이 있으면 셀 크기가 하나가 아니다. 호출자는 샘플링 간격으로만 쓰므로 최소값을 돌려준다.
연결되는 부분: `GameRoom.cpp` 13519/13758/14302/15174, `PlayerSkillSystem.cpp` 740

```cpp
		/* The smallest cell size among the base grid and its detail regions.
		Every caller uses it as a sampling step or probe distance, and a finer
		step is always safe. */
		float Get_CellSize() const;
		/* Detail grids nested inside this Area. A region owns its XZ footprint:
		a query whose first point lies inside it runs on that grid and never
		sees base cells, so a region has to cover the whole walkable extent of
		the stage it refines. Stages joined only by authored moves need no
		cross-region paths, which is why none exist. */
		std::size_t Get_RegionCount() const noexcept { return m_Regions.size(); }
```

파일: `Server/Public/ServerNavigation.h`
작업: 추가
기준점: `void Rebuild_InitialRuntimeBlockers() noexcept;`
위치: 바로 아래 (private 함수 구역)
정의 위치: `Server/Private/ServerNavigation.cpp`, `Rebuild_InitialRuntimeBlockers` 정의 뒤
필요한 이유: 매니페스트 로드, 점→영역 선택, footprint 판정, 영역 blocker 금지 확인
연결되는 부분: `Load`, 공개 XZ 함수 10곳

```cpp
		bool Load_Regions(const std::string& areaId);
		const CServerNavigation* Select_Region(float x, float z) const;
		bool Contains_Point(float x, float z) const;
		bool Overlaps_Grid(const CServerNavigation& other) const;
		std::size_t Get_DeclaredBlockerRegionCount() const noexcept
		{
			return m_RuntimeBlockerRegions.size();
		}
```

파일: `Server/Public/ServerNavigation.h`
작업: 추가
기준점: `std::string m_strStatus;`
위치: 바로 아래 (마지막 멤버)
필요한 이유: 영역 격자 소유. `std::vector` 는 불완전 타입 멤버를 허용해 자기 타입을 담을 수 있고 복사 가능성이 유지된다.
연결되는 부분: `Load_Regions` 가 채우고 `Select_Region` 이 읽는다

```cpp
		/* Owner container of the detail regions in manifest order. Each entry is
		a fully loaded grid whose own m_Regions stays empty. */
		std::vector<CServerNavigation> m_Regions;
```

### G1-2. `Server/Private/ServerNavigation.cpp`

파일: `Server/Private/ServerNavigation.cpp`
작업: 추가
기준점: `#include <algorithm>`
위치: 바로 아래
필요한 이유: `Load_Regions` 의 regionId 문자 검사

```cpp
#include <cctype>
```

파일: `Server/Private/ServerNavigation.cpp`
작업: 교체 (`Load` 안)
기준점: `m_BlockCounts.clear();` 다음의 `m_iRevision = 0u;` (함수 첫 리셋 블록)
위치: 그 줄을 아래 두 줄로 교체

```cpp
	m_iRevision = 0u;
	m_Regions.clear();
```

파일: `Server/Private/ServerNavigation.cpp`
작업: 교체 (`Load` 안)
기준점: `if (!Load_RuntimePolicy(areaId) || !Load_RuntimeBlockers(areaId))` 부터 `return true;` 까지의 블록
위치: 블록 전체 교체

```cpp
	if (!Load_RuntimePolicy(areaId) || !Load_RuntimeBlockers(areaId) ||
		!Load_Regions(areaId))
	{
		// Loading is transactional: a malformed sidecar must never leave a
		// partially usable grid behind for callers that inspect Is_Loaded().
		m_Walkable.clear();
		m_Heights.clear();
		m_RuntimeBlockerRegions.clear();
		m_ConditionValues.clear();
		m_BlockCounts.clear();
		m_VoidCounts.clear();
		m_Regions.clear();
		m_fMaximumTraversalStepHeight = 0.f;
		return false;
	}
	m_strStatus = "Loaded server navigation: " + std::to_string(m_iWidth) +
		"x" + std::to_string(m_iHeight) + ", runtime blockers=" +
		std::to_string(m_RuntimeBlockerRegions.size()) + ", regions=" +
		std::to_string(m_Regions.size());
	return true;
```

파일: `Server/Private/ServerNavigation.cpp`
작업: 추가 (함수 정의 5개)
기준점: `bool LostArk::Server::CServerNavigation::Is_CellWalkable(`
위치: 이 정의 바로 위 (`Rebuild_InitialRuntimeBlockers` 정의 바로 뒤)
헤더 선언: G1-1 의 private 4개 + `Get_CellSize`

```cpp
bool LostArk::Server::CServerNavigation::Load_Regions(const std::string& areaId)
{
	/* A detail region is itself a CServerNavigation loaded by its grid id
	"<AreaId>.<regionId>". The dot is how a region knows it must not look for
	regions of its own, so nesting stops at one level. */
	if (areaId.find('.') != std::string::npos)
		return true;
	const std::filesystem::path path = Resolve_DataRoot() / L"Navigation" /
		std::filesystem::path(areaId + ".navregions");
	if (!std::filesystem::exists(path))
		return true;
	std::ifstream input(path);
	std::string magic;
	std::string stagedAreaId;
	std::uint32_t version = 0u;
	std::uint32_t regionCount = 0u;
	if (!(input >> magic >> version >> std::quoted(stagedAreaId) >>
		regionCount) ||
		magic != "LOSTARK_NAVGRID_REGIONS" || 1u != version ||
		stagedAreaId != areaId || regionCount > 64u)
	{
		m_strStatus = "Server navigation region manifest is invalid: " +
			path.string();
		return false;
	}
	std::vector<CServerNavigation> regions;
	std::set<std::string> regionIds;
	regions.reserve(regionCount);
	for (std::uint32_t regionIndex = 0u; regionIndex < regionCount;
		++regionIndex)
	{
		std::string rowMagic;
		std::string regionId;
		float manifestStepHeight = 0.f;
		if (!(input >> rowMagic >> std::quoted(regionId) >>
			manifestStepHeight) ||
			rowMagic != "REGION" || regionId.empty() || regionId.size() > 32u ||
			!std::all_of(regionId.begin(), regionId.end(),
				[](const char value)
				{
					return 0 != std::isalnum(
						static_cast<unsigned char>(value)) ||
						'_' == value || '-' == value;
				}) ||
			!std::isfinite(manifestStepHeight) || manifestStepHeight < 0.f ||
			!regionIds.insert(regionId).second)
		{
			m_strStatus =
				"Server navigation region manifest row is invalid: " +
				path.string();
			return false;
		}
		CServerNavigation region;
		if (!region.Load(areaId + "." + regionId))
		{
			m_strStatus = region.m_strStatus;
			return false;
		}
		if (0u != region.Get_DeclaredBlockerRegionCount())
		{
			m_strStatus = "Server navigation region declares runtime blockers, "
				"which detail regions do not support: " + regionId;
			return false;
		}
		if (std::abs(region.m_fMaximumTraversalStepHeight -
			manifestStepHeight) > 0.000001f)
		{
			m_strStatus = "Server navigation region step policy differs from "
				"its manifest row: " + regionId;
			return false;
		}
		for (const CServerNavigation& other : regions)
		{
			if (region.Overlaps_Grid(other))
			{
				m_strStatus = "Server navigation regions overlap: " + regionId;
				return false;
			}
		}
		regions.push_back(std::move(region));
	}
	input >> std::ws;
	if (!input.eof())
	{
		m_strStatus =
			"Server navigation region manifest has trailing data: " +
			path.string();
		return false;
	}
	m_Regions = std::move(regions);
	return true;
}

const LostArk::Server::CServerNavigation*
LostArk::Server::CServerNavigation::Select_Region(
	const float x,
	const float z) const
{
	for (const CServerNavigation& region : m_Regions)
	{
		if (region.Contains_Point(x, z))
			return &region;
	}
	return nullptr;
}

bool LostArk::Server::CServerNavigation::Contains_Point(
	const float x,
	const float z) const
{
	return Is_Loaded() && std::isfinite(x) && std::isfinite(z) &&
		x >= m_fOriginX && z >= m_fOriginZ &&
		x < m_fOriginX + static_cast<float>(m_iWidth) * m_fCellSize &&
		z < m_fOriginZ + static_cast<float>(m_iHeight) * m_fCellSize;
}

bool LostArk::Server::CServerNavigation::Overlaps_Grid(
	const CServerNavigation& other) const
{
	const float maxX =
		m_fOriginX + static_cast<float>(m_iWidth) * m_fCellSize;
	const float maxZ =
		m_fOriginZ + static_cast<float>(m_iHeight) * m_fCellSize;
	const float otherMaxX = other.m_fOriginX +
		static_cast<float>(other.m_iWidth) * other.m_fCellSize;
	const float otherMaxZ = other.m_fOriginZ +
		static_cast<float>(other.m_iHeight) * other.m_fCellSize;
	return m_fOriginX < otherMaxX && other.m_fOriginX < maxX &&
		m_fOriginZ < otherMaxZ && other.m_fOriginZ < maxZ;
}

float LostArk::Server::CServerNavigation::Get_CellSize() const
{
	float cellSize = m_fCellSize;
	for (const CServerNavigation& region : m_Regions)
		cellSize = (std::min)(cellSize, region.m_fCellSize);
	return cellSize;
}

```

파일: `Server/Private/ServerNavigation.cpp`
작업: 추가 (dispatch prologue 10곳)
위치: 각 함수 정의의 여는 `{` 바로 아래 첫 문장. 기준점은 함수 시그니처.

```cpp
// Project_Point(x, z, outPoint)
	if (const CServerNavigation* region = Select_Region(x, z))
		return region->Project_Point(x, z, outPoint);

// Project_PointOnSameLevel(x, z, outPoint)
	if (const CServerNavigation* region = Select_Region(x, z))
		return region->Project_PointOnSameLevel(x, z, outPoint);

// Sample_Position(x, z, outPoint)
	if (const CServerNavigation* region = Select_Region(x, z))
		return region->Sample_Position(x, z, outPoint);

// Resolve_TraversalStep(fromX, fromZ, toX, toZ, outPoint)
	if (const CServerNavigation* region = Select_Region(fromX, fromZ))
		return region->Resolve_TraversalStep(fromX, fromZ, toX, toZ, outPoint);

// Has_LineOfSight(startX, startZ, endX, endZ)
	if (const CServerNavigation* region = Select_Region(startX, startZ))
		return region->Has_LineOfSight(startX, startZ, endX, endZ);

// Find_Path(startX, startZ, goalX, goalZ, outPath)  -- 기존 첫 문장 outPath.clear() 보다 위
	if (const CServerNavigation* region = Select_Region(startX, startZ))
		return region->Find_Path(startX, startZ, goalX, goalZ, outPath);

// Smooth_Path(startX, startZ, goalX, goalZ, path)  -- 기존 첫 문장 if (path.empty()) 보다 위
	if (const CServerNavigation* region = Select_Region(startX, startZ))
	{
		region->Smooth_Path(startX, startZ, goalX, goalZ, path);
		return;
	}

// Find_PathToReachablePointWithinRadius(startX, startZ, centerX, centerZ, radius, minimumDestinationDistance, outPath)
	if (const CServerNavigation* region = Select_Region(startX, startZ))
	{
		return region->Find_PathToReachablePointWithinRadius(
			startX, startZ, centerX, centerZ, radius,
			minimumDestinationDistance, outPath);
	}

// Is_PointWalkableExact(x, z)
	if (const CServerNavigation* region = Select_Region(x, z))
		return region->Is_PointWalkableExact(x, z);

// Is_PointInVoidRegion(x, z)
	if (const CServerNavigation* region = Select_Region(x, z))
		return region->Is_PointInVoidRegion(x, z);
```

`Is_HeightTransitionAllowed`, `Prepare/Commit_ConditionChanges`, `Reset_RuntimeBlockers`,
`Has_Condition`, `Get_ActiveBlockerRegionCount`, `Set_VoidConditions`, `Get_Revision` 은
기본 격자에만 적용한다. 영역은 로드 시 blocker 0개를 강제하므로 조건 변경이 영역에
닿을 일이 없다.

### G1-3. `Server/Private/ServerGameplayContractTests.cpp`

파일: `Server/Private/ServerGameplayContractTests.cpp`
작업: 추가 (`Run_ServerNavigationContractTests` 안)
기준점: 함수 끝부분의
```cpp
	SetEnvironmentVariableW(
		L"LOSTARK_SERVER_DATA_ROOT", hadConfiguredRoot ? pathBuffer.data() : nullptr);
	fs::remove_all(invalidPolicyRoot, fixtureError);
```
위치: 이 두 문장 바로 위. 이 시점에는 `LOSTARK_SERVER_DATA_ROOT` 가 아직 `invalidPolicyRoot` 를 가리킨다.
필요한 이유: 영역 로드·dispatch·경로 격리·거부 4종·매니페스트 없음 호환

```cpp
	/* Detail regions: a finer grid nested inside an Area answers every query
	   whose first point lies inside it, and the base grid answers the rest. */
	const auto writeRegionGrid = [&](
		const wchar_t* gridStem,
		const std::uint32_t width,
		const std::uint32_t height,
		const float cellSize,
		const float originX,
		const float originZ,
		const float cellHeight)
	{
		std::ofstream grid(
			invalidPolicyRoot / L"Navigation" /
				(std::wstring(gridStem) + L".navgrid"),
			std::ios::binary | std::ios::trunc);
		grid.write(reinterpret_cast<const char*>(&width), sizeof(width));
		grid.write(reinterpret_cast<const char*>(&height), sizeof(height));
		grid.write(reinterpret_cast<const char*>(&cellSize), sizeof(cellSize));
		grid.write(reinterpret_cast<const char*>(&originX), sizeof(originX));
		grid.write(reinterpret_cast<const char*>(&originZ), sizeof(originZ));
		const std::size_t cellCount =
			static_cast<std::size_t>(width) * height;
		const std::uint8_t walkable = 1u;
		for (std::size_t index = 0u; index < cellCount; ++index)
			grid.write(reinterpret_cast<const char*>(&walkable), sizeof(walkable));
		for (std::size_t index = 0u; index < cellCount; ++index)
			grid.write(reinterpret_cast<const char*>(&cellHeight), sizeof(cellHeight));
		return grid.good();
	};
	const auto writeRegionPolicy = [&](
		const wchar_t* gridStem,
		const char* gridId,
		const float stepHeight)
	{
		std::ofstream policy(
			invalidPolicyRoot / L"Navigation" /
				(std::wstring(gridStem) + L".navpolicy"),
			std::ios::binary | std::ios::trunc);
		policy << "LOSTARK_NAVIGATION_POLICY 1 \"" << gridId << "\" " <<
			stepHeight << '\n';
		return policy.good();
	};
	const auto writeRegionManifest = [&](const char* text)
	{
		std::ofstream manifest(
			invalidPolicyRoot / L"Navigation" / L"NAV_REGION_CONTRACT.navregions",
			std::ios::binary | std::ios::trunc);
		manifest << text;
		return manifest.good();
	};
	const bool regionFixtureReady =
		writeRegionGrid(L"NAV_REGION_CONTRACT", 4u, 4u, 1.f, 0.f, 0.f, 0.f) &&
		writeRegionPolicy(L"NAV_REGION_CONTRACT", "NAV_REGION_CONTRACT", 1.f) &&
		writeRegionGrid(L"NAV_REGION_CONTRACT.fine", 4u, 4u, 0.5f, 1.f, 1.f, 2.f) &&
		writeRegionPolicy(
			L"NAV_REGION_CONTRACT.fine", "NAV_REGION_CONTRACT.fine", 0.75f) &&
		writeRegionManifest(
			"LOSTARK_NAVGRID_REGIONS 1 \"NAV_REGION_CONTRACT\" 1\n"
			"REGION \"fine\" 0.75\n");
	CServerNavigation regionNavigation;
	const bool regionLoaded = regionFixtureReady &&
		regionNavigation.Load("NAV_REGION_CONTRACT");
	tests.Require(
		regionLoaded && 1u == regionNavigation.Get_RegionCount() &&
		std::abs(regionNavigation.Get_CellSize() - 0.5f) < 0.000001f &&
		std::abs(regionNavigation.Get_MaximumTraversalStepHeight() - 1.f) <
			0.000001f,
		"Load a detail region beside its base grid and report the finer cell size");
	SERVER_NAV_POINT insideRegion{};
	SERVER_NAV_POINT outsideRegion{};
	tests.Require(
		regionLoaded &&
		regionNavigation.Sample_Position(1.25f, 1.25f, insideRegion) &&
		std::abs(insideRegion.y - 2.f) < 0.000001f &&
		regionNavigation.Sample_Position(0.5f, 0.5f, outsideRegion) &&
		std::abs(outsideRegion.y) < 0.000001f,
		"Answer a point inside the region from the region grid and the rest from the base grid");
	std::vector<SERVER_NAV_POINT> regionPath;
	bool regionPathStaysInside = regionLoaded && regionNavigation.Find_Path(
		1.25f, 1.25f, 2.75f, 2.75f, regionPath) && !regionPath.empty();
	for (const SERVER_NAV_POINT& point : regionPath)
	{
		regionPathStaysInside = regionPathStaysInside &&
			std::abs(point.y - 2.f) < 0.000001f &&
			point.x >= 1.f && point.x < 3.f && point.z >= 1.f && point.z < 3.f;
	}
	std::vector<SERVER_NAV_POINT> escapePath;
	const bool regionPathCannotLeave = regionLoaded &&
		regionNavigation.Find_Path(1.25f, 1.25f, 0.5f, 0.5f, escapePath) &&
		(escapePath.empty() ||
			(escapePath.back().x >= 1.f && escapePath.back().z >= 1.f));
	tests.Require(
		regionPathStaysInside && regionPathCannotLeave,
		"Path inside a region on its own cells and never walk out of it on foot");
	CServerNavigation missingRegionNavigation;
	tests.Require(
		regionFixtureReady && writeRegionManifest(
			"LOSTARK_NAVGRID_REGIONS 1 \"NAV_REGION_CONTRACT\" 1\n"
			"REGION \"missing\" 0.75\n") &&
		!missingRegionNavigation.Load("NAV_REGION_CONTRACT") &&
		!missingRegionNavigation.Is_Loaded(),
		"Reject a region manifest whose grid files are missing and leave nothing loaded");
	CServerNavigation mismatchedStepNavigation;
	tests.Require(
		regionFixtureReady && writeRegionManifest(
			"LOSTARK_NAVGRID_REGIONS 1 \"NAV_REGION_CONTRACT\" 1\n"
			"REGION \"fine\" 0.5\n") &&
		!mismatchedStepNavigation.Load("NAV_REGION_CONTRACT"),
		"Reject a region whose manifest step differs from its published policy");
	CServerNavigation overlappingNavigation;
	tests.Require(
		regionFixtureReady &&
		writeRegionGrid(
			L"NAV_REGION_CONTRACT.overlap", 4u, 4u, 0.5f, 2.f, 2.f, 2.f) &&
		writeRegionPolicy(
			L"NAV_REGION_CONTRACT.overlap", "NAV_REGION_CONTRACT.overlap", 0.75f) &&
		writeRegionManifest(
			"LOSTARK_NAVGRID_REGIONS 1 \"NAV_REGION_CONTRACT\" 2\n"
			"REGION \"fine\" 0.75\n"
			"REGION \"overlap\" 0.75\n") &&
		!overlappingNavigation.Load("NAV_REGION_CONTRACT"),
		"Reject two regions whose footprints overlap");
	bool blockerFixtureReady = regionFixtureReady && writeRegionManifest(
		"LOSTARK_NAVGRID_REGIONS 1 \"NAV_REGION_CONTRACT\" 1\n"
		"REGION \"fine\" 0.75\n");
	if (blockerFixtureReady)
	{
		std::ofstream blockers(
			invalidPolicyRoot / L"Navigation" /
				L"NAV_REGION_CONTRACT.fine.navblockers",
			std::ios::binary | std::ios::trunc);
		blockers <<
			"LOSTARK_NAVGRID_BLOCKERS 1 \"NAV_REGION_CONTRACT.fine\" 4 4 0.5 1 1 1\n"
			"REGION \"contract.region.wall\" \"contract.region.wall.open\" 0 1\n"
			"1 1\n";
		blockerFixtureReady = blockers.good();
	}
	CServerNavigation blockedRegionNavigation;
	tests.Require(
		blockerFixtureReady &&
		!blockedRegionNavigation.Load("NAV_REGION_CONTRACT"),
		"Reject runtime blockers declared inside a detail region");
	std::error_code regionCleanupError;
	fs::remove(
		invalidPolicyRoot / L"Navigation" / L"NAV_REGION_CONTRACT.fine.navblockers",
		regionCleanupError);
	fs::remove(
		invalidPolicyRoot / L"Navigation" / L"NAV_REGION_CONTRACT.navregions",
		regionCleanupError);
	CServerNavigation manifestlessNavigation;
	tests.Require(
		regionFixtureReady &&
		manifestlessNavigation.Load("NAV_REGION_CONTRACT") &&
		0u == manifestlessNavigation.Get_RegionCount() &&
		std::abs(manifestlessNavigation.Get_CellSize() - 1.f) < 0.000001f,
		"Load an Area without a region manifest exactly as before");
```

---

## G2. Publisher — 영역 격자 변환과 매니페스트 publish

`Convert-NavigationAuthoringGrid` 는 이미 임의의 `-RelativeSourcePath`/`-RelativePaintPath`
를 받고 헤더의 areaId 를 그대로 `AreaId` 로 돌려준다. 영역 격자는 이 함수를 그대로
재사용하고, 새로 필요한 것은 (1) 매니페스트 읽기, (2) 영역 묶음 변환, (3) footprint
비겹침 검사, (4) placement 검증을 기본+영역 **합성 dispatch** 로 바꾸는 것, (5) publish
target 확장이다.

`Assert-NavigationGrid` 는 현재 격자 검증과 placement 검증을 한 함수에서 한다. placement
는 Area 단위이지 격자 단위가 아니므로 둘을 분리한다.

### G2-1. 매니페스트 읽기와 영역 변환

파일: `Tools/NavigationPipeline/Publish-ServerNavigation.ps1`
작업: 추가
기준점: `function Assert-NavigationGrid {`
위치: 바로 위 (`Convert-NavigationAuthoringGrid` 의 닫는 `}` 바로 아래)
필요한 이유: 매니페스트가 저작 정본이고, 영역마다 같은 변환을 돌려 격자 묶음을 만든다.
연결되는 부분: `$grids` 구성, `Assert-NavigationRegionFootprints`, publish target

```powershell
function Read-NavigationRegionManifest {
    param(
        [string]$AreaId,
        [string]$AuthoringRoot = $repoRoot
    )
    $path = [IO.Path]::GetFullPath(
        (Join-Path $AuthoringRoot "Data/Navigation/$AreaId.navregions"))
    if (-not [IO.File]::Exists($path)) { return @() }
    $lines = @([IO.File]::ReadAllLines($path, [Text.Encoding]::UTF8))
    if ($lines.Count -lt 1) { throw "Navigation region manifest is empty: $path" }
    $header = @(Split-NavigationTokens $lines[0])
    if ($header.Count -ne 4 -or $header[0] -ne 'LOSTARK_NAVGRID_REGIONS' -or
        $header[1] -ne '1' -or $header[2] -cne $AreaId -or
        [uint32]$header[3] -gt 64) {
        throw "Navigation region manifest header is invalid: $path"
    }
    $regionCount = [uint32]$header[3]
    if ($lines.Count -ne 1 + $regionCount) {
        throw "Navigation region manifest row count is invalid: $path"
    }
    $culture = [Globalization.CultureInfo]::InvariantCulture
    $seen = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $regions = [Collections.Generic.List[object]]::new()
    for ($index = 0; $index -lt $regionCount; ++$index) {
        $tokens = @(Split-NavigationTokens $lines[$index + 1])
        if ($tokens.Count -ne 3 -or $tokens[0] -ne 'REGION' -or
            $tokens[1] -cnotmatch '^[A-Za-z0-9_-]{1,32}$' -or
            -not $seen.Add([string]$tokens[1])) {
            throw "Navigation region manifest row is invalid: $path row=$index"
        }
        $step = [single]::Parse($tokens[2], $culture)
        if ([single]::IsNaN($step) -or [single]::IsInfinity($step) -or
            $step -lt 0.0) {
            throw "Navigation region step height is invalid: $path row=$index"
        }
        $regions.Add([pscustomobject]@{ RegionId = [string]$tokens[1]; StepHeight = $step })
    }
    return @($regions.ToArray())
}

function Convert-NavigationRegionGrids {
    param(
        [string]$AreaId,
        [string]$AuthoringRoot = $repoRoot
    )
    $manifest = @(Read-NavigationRegionManifest -AreaId $AreaId -AuthoringRoot $AuthoringRoot)
    $grids = [Collections.Generic.List[object]]::new()
    foreach ($region in $manifest) {
        $gridId = "$AreaId.$($region.RegionId)"
        # A region grid is a normal authoring grid whose id carries the dot, so
        # the same converter and the same paint contract apply unchanged.
        $grid = Convert-NavigationAuthoringGrid `
            -RelativeSourcePath "Data/Navigation/$gridId.navsource" `
            -RelativePaintPath "Data/Navigation/$gridId.navpaint" `
            -RuntimeMaximumStepHeight $region.StepHeight `
            -AuthoringRoot $AuthoringRoot
        if ($grid.AreaId -cne $gridId) {
            throw "Navigation region source declares a different id: $gridId"
        }
        $grids.Add($grid)
    }
    return @($grids.ToArray())
}

function Get-NavigationGridBounds {
    param([object]$Grid)
    $bytes = [byte[]]$Grid.Bytes
    $width = [BitConverter]::ToUInt32($bytes, 0)
    $height = [BitConverter]::ToUInt32($bytes, 4)
    $cellSize = [BitConverter]::ToSingle($bytes, 8)
    $originX = [BitConverter]::ToSingle($bytes, 12)
    $originZ = [BitConverter]::ToSingle($bytes, 16)
    return [pscustomobject]@{
        MinX = [double]$originX
        MinZ = [double]$originZ
        MaxX = [double]$originX + [double]$width * $cellSize
        MaxZ = [double]$originZ + [double]$height * $cellSize
        CellSize = [double]$cellSize
        Width = $width
        Height = $height
    }
}

function Assert-NavigationRegionFootprints {
    param([object[]]$RegionGrids)
    for ($outer = 0; $outer -lt $RegionGrids.Count; ++$outer) {
        $a = Get-NavigationGridBounds $RegionGrids[$outer]
        for ($inner = $outer + 1; $inner -lt $RegionGrids.Count; ++$inner) {
            $b = Get-NavigationGridBounds $RegionGrids[$inner]
            if ($a.MinX -lt $b.MaxX -and $b.MinX -lt $a.MaxX -and
                $a.MinZ -lt $b.MaxZ -and $b.MinZ -lt $a.MaxZ) {
                throw ("Navigation regions overlap: " +
                    "$($RegionGrids[$outer].AreaId) and $($RegionGrids[$inner].AreaId)")
            }
        }
    }
}

function Convert-NavigationRuntimeRegionManifest {
    param(
        [string]$AreaId,
        [object[]]$RegionGrids
    )
    $culture = [Globalization.CultureInfo]::InvariantCulture
    $lines = [Collections.Generic.List[string]]::new()
    $lines.Add("LOSTARK_NAVGRID_REGIONS 1 `"$AreaId`" $($RegionGrids.Count)")
    foreach ($grid in $RegionGrids) {
        $regionId = $grid.AreaId.Substring($AreaId.Length + 1)
        $stepText = ([single]$grid.RuntimeMaximumStepHeight).ToString('R', $culture)
        $lines.Add("REGION `"$regionId`" $stepText")
    }
    return [string[]]$lines.ToArray()
}
```

### G2-2. placement 검증을 합성 dispatch 로 분리

파일: `Tools/NavigationPipeline/Publish-ServerNavigation.ps1`
작업: 삭제
기준점: `Assert-NavigationGrid` 함수 안의 `$world = Get-Content -LiteralPath (Join-Path $repoRoot $Grid.WorldPath) ...` 줄
위치: 그 줄부터 이어지는 `foreach ($placement in @($world.placements | Where-Object {` 블록의 닫는 `}` 까지 전부 삭제
필요한 이유: spawn/boss 는 Area 하나에 속하고 격자마다 있는 게 아니다. 영역이 있으면 서버와 같은 규칙(점을 담는 영역 우선)으로 골라 검사해야 한다.

삭제 후 `Assert-NavigationGrid` 의 끝은 다음 두 줄만 남는다.

```powershell
    return "${width}x${height}, cellSize=$cellSize, walkable=$walkableCount, maxStep=$maximumObservedStep"
}
```

파일: `Tools/NavigationPipeline/Publish-ServerNavigation.ps1`
작업: 추가
기준점: `function Invoke-NavigationPaintContractTest {`
위치: 바로 위 (`Assert-NavigationGrid` 의 닫는 `}` 바로 아래)
필요한 이유: 서버 `Select_Region` 과 같은 판정으로 placement 를 검사한다.

```powershell
function Assert-NavigationPlacements {
    param(
        [object]$BaseGrid,
        [object[]]$RegionGrids
    )
    $world = Get-Content -LiteralPath (Join-Path $repoRoot $BaseGrid.WorldPath) `
        -Raw -Encoding UTF8 | ConvertFrom-Json
    foreach ($placement in @($world.placements | Where-Object {
        $_.kind -eq 'boss' -or ($_.enabled -and $_.kind -eq 'playerSpawn') })) {
        $x = [double]$placement.position[0]
        $y = [double]$placement.position[1]
        $z = [double]$placement.position[2]
        # Same dispatch as CServerNavigation::Select_Region: the first region
        # whose footprint contains the point owns it, otherwise the base grid.
        $owner = $BaseGrid
        foreach ($region in $RegionGrids) {
            $bounds = Get-NavigationGridBounds $region
            if ($x -ge $bounds.MinX -and $x -lt $bounds.MaxX -and
                $z -ge $bounds.MinZ -and $z -lt $bounds.MaxZ) {
                $owner = $region
                break
            }
        }
        $bytes = [byte[]]$owner.Bytes
        $bounds = Get-NavigationGridBounds $owner
        $cellX = [int][Math]::Floor(($x - $bounds.MinX) / $bounds.CellSize)
        $cellZ = [int][Math]::Floor(($z - $bounds.MinZ) / $bounds.CellSize)
        if ($cellX -lt 0 -or $cellZ -lt 0 -or
            $cellX -ge $bounds.Width -or $cellZ -ge $bounds.Height) {
            throw "Gameplay placement is outside server navigation: $($placement.placementId)"
        }
        $index = $cellZ * $bounds.Width + $cellX
        if ($bytes[20 + $index] -ne 1) {
            throw "Gameplay placement is not on a walkable server cell: $($placement.placementId) grid=$($owner.AreaId)"
        }
        $cellCount = [uint64]$bounds.Width * [uint64]$bounds.Height
        $heightOffset = 20 + [int]$cellCount + 4 * $index
        $cellHeight = [BitConverter]::ToSingle($bytes, $heightOffset)
        if ([Math]::Abs($y - $cellHeight) -gt 0.25) {
            throw "Gameplay placement height differs from server navigation: $($placement.placementId) grid=$($owner.AreaId)"
        }
    }
}
```

### G2-3. `$grids` 구성과 publish target

파일: `Tools/NavigationPipeline/Publish-ServerNavigation.ps1`
작업: 교체
기준점: `$validated = foreach ($grid in $grids) {` 부터 그 블록의 닫는 `}` 까지
위치: 블록 전체 교체
필요한 이유: Area 마다 영역 묶음을 만들고, 격자 검증은 기본+영역 각각, placement 검증은 Area 당 한 번, 런타임 매니페스트는 영역이 하나라도 있을 때만 만든다.

```powershell
$validated = foreach ($grid in $grids) {
    $regionGrids = @(Convert-NavigationRegionGrids -AreaId $grid.AreaId)
    Assert-NavigationRegionFootprints -RegionGrids $regionGrids
    $detail = Assert-NavigationGrid $grid
    $regionEntries = foreach ($regionGrid in $regionGrids) {
        [pscustomobject]@{
            Grid = $regionGrid
            Detail = Assert-NavigationGrid $regionGrid
            BlockerLines = @(Convert-NavigationRuntimeBlockers `
                -AreaId $regionGrid.AreaId -GridBytes ([byte[]]$regionGrid.Bytes))
            PolicyLines = @(Convert-NavigationRuntimePolicy -Grid $regionGrid)
        }
    }
    Assert-NavigationPlacements -BaseGrid $grid -RegionGrids $regionGrids
    [pscustomobject]@{
        Grid = $grid
        Detail = $detail
        BlockerLines = @(Convert-NavigationRuntimeBlockers `
            -AreaId $grid.AreaId -GridBytes ([byte[]]$grid.Bytes))
        PolicyLines = @(Convert-NavigationRuntimePolicy -Grid $grid)
        Regions = @($regionEntries)
        ManifestLines = $(if ($regionGrids.Count -gt 0) {
            @(Convert-NavigationRuntimeRegionManifest `
                -AreaId $grid.AreaId -RegionGrids $regionGrids)
        } else { @() })
    }
}
```

파일: `Tools/NavigationPipeline/Publish-ServerNavigation.ps1`
작업: 교체
기준점: `        $targets = @(` 부터 그 배열의 닫는 `)` 까지
위치: 배열 리터럴 전체 교체
필요한 이유: 영역 격자 6개와 매니페스트 2개를 같은 transaction 으로 승격해야 부분 반영이 없다.

```powershell
        $targets = [Collections.Generic.List[object]]::new()
        $targets.Add(@{ Destination=(Join-Path $root "$($entry.Grid.AreaId).navgrid"); Bytes=[byte[]]$entry.Grid.Bytes })
        $targets.Add(@{ Destination=(Join-Path $root "$($entry.Grid.AreaId).navblockers"); Lines=[string[]]$entry.BlockerLines })
        $targets.Add(@{ Destination=(Join-Path $root "$($entry.Grid.AreaId).navpolicy"); Lines=[string[]]$entry.PolicyLines })
        $targets.Add(@{ Destination=(Join-Path $clientRoot "$($entry.Grid.AreaId).navgrid"); Bytes=[byte[]]$entry.Grid.Bytes })
        $targets.Add(@{ Destination=(Join-Path $clientRoot "$($entry.Grid.AreaId).navblockers"); Lines=[string[]]$entry.BlockerLines })
        $targets.Add(@{ Destination=(Join-Path $clientRoot "$($entry.Grid.AreaId).navpolicy"); Lines=[string[]]$entry.PolicyLines })
        foreach ($regionEntry in @($entry.Regions)) {
            $regionId = $regionEntry.Grid.AreaId
            $targets.Add(@{ Destination=(Join-Path $root "$regionId.navgrid"); Bytes=[byte[]]$regionEntry.Grid.Bytes })
            $targets.Add(@{ Destination=(Join-Path $root "$regionId.navblockers"); Lines=[string[]]$regionEntry.BlockerLines })
            $targets.Add(@{ Destination=(Join-Path $root "$regionId.navpolicy"); Lines=[string[]]$regionEntry.PolicyLines })
            $targets.Add(@{ Destination=(Join-Path $clientRoot "$regionId.navgrid"); Bytes=[byte[]]$regionEntry.Grid.Bytes })
            $targets.Add(@{ Destination=(Join-Path $clientRoot "$regionId.navblockers"); Lines=[string[]]$regionEntry.BlockerLines })
            $targets.Add(@{ Destination=(Join-Path $clientRoot "$regionId.navpolicy"); Lines=[string[]]$regionEntry.PolicyLines })
        }
        if (@($entry.ManifestLines).Count -gt 0) {
            $targets.Add(@{ Destination=(Join-Path $root "$($entry.Grid.AreaId).navregions"); Lines=[string[]]$entry.ManifestLines })
            $targets.Add(@{ Destination=(Join-Path $clientRoot "$($entry.Grid.AreaId).navregions"); Lines=[string[]]$entry.ManifestLines })
        }
```

파일: `Tools/NavigationPipeline/Publish-ServerNavigation.ps1`
작업: 교체
기준점: 스크립트 마지막 `foreach ($entry in $validated) {` 블록
위치: 블록 전체 교체
필요한 이유: 영역도 결과를 보고해야 사용자가 무엇이 발행됐는지 안다.

```powershell
foreach ($entry in $validated) {
    Write-Host "Server navigation $Mode succeeded: $($entry.Grid.AreaId) $($entry.Detail)."
    foreach ($regionEntry in @($entry.Regions)) {
        Write-Host "  region: $($regionEntry.Grid.AreaId) $($regionEntry.Detail)."
    }
}
```

### G2-4. ContractTest 에 영역 케이스

파일: `Tools/NavigationPipeline/Publish-ServerNavigation.ps1`
작업: 추가
기준점: `Invoke-NavigationPaintContractTest` 안의 `$validBlocker = @(` 줄
위치: 그 줄 바로 위 (paint v3 높이 검사 직후, blocker 케이스 직전)
필요한 이유: 매니페스트 정상 1건과 거부 5건을 실행형으로 고정한다.

```powershell
        $regionSource = @(
            'LOSTARK_NAVGRID_SOURCE 2 "TEST_NAV_PAINT.fine" 2 1 0.5 10 10 0 0 0 1 1 1 0 50 1 2',
            '0 0 1 1 3',
            '1 0 1 1 3'
        ) -join "`n"
        $regionPaint = 'LOSTARK_NAVGRID_PAINT 3 "TEST_NAV_PAINT.fine" 2 1 0.5 10 10 0'
        [IO.File]::WriteAllText(
            (Join-Path $navigationRoot 'TEST_NAV_PAINT.fine.navsource'),
            $regionSource, [Text.Encoding]::UTF8)
        [IO.File]::WriteAllText(
            (Join-Path $navigationRoot 'TEST_NAV_PAINT.fine.navpaint'),
            $regionPaint, [Text.Encoding]::UTF8)
        [IO.File]::WriteAllText(
            (Join-Path $navigationRoot 'TEST_NAV_PAINT.navregions'),
            (@(
                'LOSTARK_NAVGRID_REGIONS 1 "TEST_NAV_PAINT" 1',
                'REGION "fine" 0.75'
            ) -join "`n"), [Text.Encoding]::UTF8)
        $regionGrids = @(Convert-NavigationRegionGrids `
            -AreaId 'TEST_NAV_PAINT' -AuthoringRoot $fixtureRoot)
        if ($regionGrids.Count -ne 1 -or
            $regionGrids[0].AreaId -cne 'TEST_NAV_PAINT.fine' -or
            [BitConverter]::ToSingle([byte[]]$regionGrids[0].Bytes, 8) -ne 0.5) {
            throw 'Navigation region grid did not convert from its manifest'
        }
        Assert-NavigationRegionFootprints -RegionGrids $regionGrids
        $manifestLines = @(Convert-NavigationRuntimeRegionManifest `
            -AreaId 'TEST_NAV_PAINT' -RegionGrids $regionGrids)
        if ($manifestLines.Count -ne 2 -or
            $manifestLines[0] -ne 'LOSTARK_NAVGRID_REGIONS 1 "TEST_NAV_PAINT" 1' -or
            $manifestLines[1] -ne 'REGION "fine" 0.75') {
            throw 'Navigation runtime region manifest was not emitted canonically'
        }
        $invalidManifests = @(
            @('LOSTARK_NAVGRID_REGIONS 2 "TEST_NAV_PAINT" 1', 'REGION "fine" 0.75'),
            @('LOSTARK_NAVGRID_REGIONS 1 "WRONG_AREA" 1', 'REGION "fine" 0.75'),
            @('LOSTARK_NAVGRID_REGIONS 1 "TEST_NAV_PAINT" 1', 'REGION "fine.dotted" 0.75'),
            @('LOSTARK_NAVGRID_REGIONS 1 "TEST_NAV_PAINT" 2', 'REGION "fine" 0.75', 'REGION "fine" 0.75'),
            @('LOSTARK_NAVGRID_REGIONS 1 "TEST_NAV_PAINT" 1', 'REGION "absent" 0.75')
        )
        foreach ($invalidManifest in $invalidManifests) {
            [IO.File]::WriteAllText(
                (Join-Path $navigationRoot 'TEST_NAV_PAINT.navregions'),
                ($invalidManifest -join "`n"), [Text.Encoding]::UTF8)
            $rejected = $false
            try {
                Convert-NavigationRegionGrids `
                    -AreaId 'TEST_NAV_PAINT' -AuthoringRoot $fixtureRoot | Out-Null
            }
            catch {
                $rejected = $true
            }
            if (-not $rejected) {
                throw 'Navigation region manifest invalid contract was accepted'
            }
        }
        [IO.File]::Delete((Join-Path $navigationRoot 'TEST_NAV_PAINT.navregions'))
        [IO.File]::WriteAllText($paintPath, $validPaint, [Text.Encoding]::UTF8)
```

### G2-5. `Tools/Build/BuildDomains.json`

파일: `Tools/Build/BuildDomains.json`
작업: 교체
기준점: `"id": "navigation"` domain 의 `outputs` 배열
위치: 배열 전체 교체
필요한 이유: 매니페스트도 publisher 산출물이라 receipt 대상에 포함해야 재사용 판정이 맞다.

```json
      "outputs": [
        "Server/Bin/DataFiles/Navigation/*.navgrid",
        "Server/Bin/DataFiles/Navigation/*.navblockers",
        "Server/Bin/DataFiles/Navigation/*.navpolicy",
        "Server/Bin/DataFiles/Navigation/*.navregions",
        "Client/Bin/DataFiles/Navigation/*.navgrid",
        "Client/Bin/DataFiles/Navigation/*.navblockers",
        "Client/Bin/DataFiles/Navigation/*.navpolicy",
        "Client/Bin/DataFiles/Navigation/*.navregions"
      ],
```

`requiredOutputPatterns` 는 그대로 둔다. 영역은 선택 사항이므로 필수 산출물이 아니다.

---

## G3. Client — `CMapNavigationContract` 영역 경로와 매니페스트

MapTool 은 지금 `Resolve_Area(areaId)` 로 경로 네 개를 얻는다. 영역을 편집하려면 같은
함수가 `<AreaId>.<regionId>` 를 받아야 하고, 매니페스트를 읽고 쓸 수 있어야 한다.
`Resolve_Area` 는 이미 `Is_ValidAreaId` 로 `[A-Za-z0-9_.-]` 를 허용하므로 점이 든 grid id
를 그대로 넘겨도 경로 계산은 맞다. 필요한 것은 (1) regionId 검사, (2) 매니페스트 read/write,
(3) 영역 경로를 한 번에 얻는 얇은 wrapper 다.

### G3-1. `Client/Public/MapNavigationContract.h`

파일: `Client/Public/MapNavigationContract.h`
작업: 추가
기준점: `#include <string>`
위치: 바로 아래
필요한 이유: 매니페스트를 벡터로 돌려준다

```cpp
#include <vector>
```

파일: `Client/Public/MapNavigationContract.h`
작업: 추가
기준점: `struct MAP_NAVIGATION_CONTRACT final` 의 닫는 `};`
위치: 바로 아래, `class CMapNavigationContract final` 바로 위
필요한 이유: 매니페스트 한 행. `regionId` 가 저작 ID 이고 `stepHeight` 는 그 영역의 런타임 낙차 한도다.
연결되는 부분: `Read_RegionManifest`, `Write_RegionManifest`, MapTool 의 영역 콤보

```cpp
/* One row of Data/Navigation/<AreaId>.navregions. regionId is the stable
   authoring id; the grid it names is "<AreaId>.<regionId>" and owns its own
   navsource/navpaint/navgrid. stepHeight is that grid's runtime adjacent-step
   limit, republished into its .navpolicy. */
struct MAP_NAVIGATION_REGION final
{
	std::string regionId;
	f32_t stepHeight = 1.f;
};
```

파일: `Client/Public/MapNavigationContract.h`
작업: 추가
기준점: `static bool_t Is_ValidAreaId(const std::string& areaId);`
위치: 바로 아래 (public 구역 마지막)
정의 위치: `Client/Private/MapNavigationContract.cpp`, `Is_ValidAreaId` 정의 뒤
필요한 이유: 영역 경로 해석과 매니페스트 입출력의 유일한 소유자
연결되는 부분: `CMapTool::Resolve_SelectedNavigationContract`, `Commit_NavigationRegionManifest`

```cpp
	/* Resolves the paths of the detail grid "<AreaId>.<regionId>". The grid is
	   an ordinary navigation grid, so this only builds the composed id and
	   defers to Resolve_Area. */
	static bool_t Resolve_Region(
		const std::string& areaId,
		const std::string& regionId,
		MAP_NAVIGATION_CONTRACT& outContract,
		std::string& outStatus);
	/* A missing manifest is not an error: it means the Area has no detail
	   regions. A malformed one is, so the caller keeps its previous list. */
	static bool_t Read_RegionManifest(
		const std::string& areaId,
		std::vector<MAP_NAVIGATION_REGION>& outRegions,
		std::string& outStatus);
	static bool_t Write_RegionManifest(
		const std::string& areaId,
		const std::vector<MAP_NAVIGATION_REGION>& regions,
		std::string& outStatus);
	static std::filesystem::path Resolve_RegionManifestPath(
		const std::string& areaId);
	static bool_t Is_ValidRegionId(const std::string& regionId);
```

### G3-2. `Client/Private/MapNavigationContract.cpp`

파일: `Client/Private/MapNavigationContract.cpp`
작업: 추가
기준점: `#include <iomanip>`
위치: 바로 아래
필요한 이유: 매니페스트 쓰기의 원자 교체와 수치 서식

```cpp
#include <sstream>
#include <system_error>
```

파일: `Client/Private/MapNavigationContract.cpp`
작업: 추가
기준점: 익명 namespace 안의 `constexpr const wchar_t* PROTOTYPE_PREFIX =` 블록의 닫는 `;`
위치: 바로 아래, 익명 namespace 의 `std::wstring ToWideAscii(` 바로 위
필요한 이유: 매니페스트 magic/version/상한을 한 곳에 둔다

```cpp
	constexpr const char* REGION_MANIFEST_MAGIC =
		"LOSTARK_NAVGRID_REGIONS";
	constexpr uint32_t REGION_MANIFEST_VERSION = 1;
	constexpr size_t MAX_REGION_ID_LENGTH = 32;
	constexpr size_t MAX_REGION_COUNT = 64;
```

파일: `Client/Private/MapNavigationContract.cpp`
작업: 추가 (함수 정의 5개)
기준점: `bool_t Client::CMapNavigationContract::Is_ValidAreaId(` 정의의 닫는 `}` (파일 마지막)
위치: 그 아래, 파일 끝
헤더 선언: G3-1 의 public 5개

```cpp
bool_t Client::CMapNavigationContract::Is_ValidRegionId(
	const std::string& regionId)
{
	/* The dot is what separates the Area from its region in a grid id, so a
	   region id must not contain one. */
	if (regionId.empty() || regionId.size() > MAX_REGION_ID_LENGTH)
		return false;

	return std::all_of(
		regionId.begin(),
		regionId.end(),
		[](const char value)
		{
			const unsigned char character =
				static_cast<unsigned char>(value);
			return 0 != std::isalnum(character) ||
				'_' == value || '-' == value;
		});
}

bool_t Client::CMapNavigationContract::Resolve_Region(
	const std::string& areaId,
	const std::string& regionId,
	MAP_NAVIGATION_CONTRACT& outContract,
	std::string& outStatus)
{
	if (!Is_ValidAreaId(areaId) || !Is_ValidRegionId(regionId))
	{
		outStatus = "Navigation region ID is invalid";
		return false;
	}
	return Resolve_Area(areaId + "." + regionId, outContract, outStatus);
}

std::filesystem::path
Client::CMapNavigationContract::Resolve_RegionManifestPath(
	const std::string& areaId)
{
	const std::filesystem::path authoringRoot =
		CProjectDataRoot::Resolve(L"Navigation");
	if (authoringRoot.empty() || !Is_ValidAreaId(areaId))
		return {};
	return authoringRoot / (ToWideAscii(areaId) + L".navregions");
}

bool_t Client::CMapNavigationContract::Read_RegionManifest(
	const std::string& areaId,
	std::vector<MAP_NAVIGATION_REGION>& outRegions,
	std::string& outStatus)
{
	outRegions.clear();
	const std::filesystem::path path = Resolve_RegionManifestPath(areaId);
	if (path.empty())
	{
		outStatus = "Navigation data root is unavailable";
		return false;
	}

	std::error_code existsError;
	const bool_t exists = std::filesystem::exists(path, existsError);
	if (existsError && !IsMissingPathError(existsError))
	{
		outStatus = "Could not inspect navigation region manifest: " +
			path.string();
		return false;
	}
	if (!exists)
	{
		// No manifest means no detail regions, which is the normal state.
		outStatus = "No navigation regions for " + areaId;
		return true;
	}

	std::ifstream input(path, std::ios::binary);
	std::string magic;
	std::string stagedAreaId;
	uint32_t version = {};
	uint32_t regionCount = {};
	if (!input || !(input >> magic >> version >> std::quoted(stagedAreaId) >>
		regionCount) ||
		magic != REGION_MANIFEST_MAGIC ||
		version != REGION_MANIFEST_VERSION ||
		stagedAreaId != areaId ||
		regionCount > MAX_REGION_COUNT)
	{
		outStatus = "Navigation region manifest header is invalid: " +
			path.string();
		return false;
	}

	std::vector<MAP_NAVIGATION_REGION> staged;
	staged.reserve(regionCount);
	for (uint32_t index = 0; index < regionCount; ++index)
	{
		std::string rowMagic;
		MAP_NAVIGATION_REGION region;
		if (!(input >> rowMagic >> std::quoted(region.regionId) >>
			region.stepHeight) ||
			rowMagic != "REGION" ||
			!Is_ValidRegionId(region.regionId) ||
			!std::isfinite(region.stepHeight) ||
			region.stepHeight < 0.f ||
			staged.end() != std::find_if(
				staged.begin(),
				staged.end(),
				[&region](const MAP_NAVIGATION_REGION& existing)
				{
					return existing.regionId == region.regionId;
				}))
		{
			outStatus = "Navigation region manifest row is invalid: " +
				path.string();
			return false;
		}
		staged.push_back(std::move(region));
	}

	input >> std::ws;
	if (!input.eof())
	{
		outStatus = "Navigation region manifest has trailing data: " +
			path.string();
		return false;
	}

	outRegions = std::move(staged);
	outStatus = "Loaded " + std::to_string(outRegions.size()) +
		" navigation regions for " + areaId;
	return true;
}

bool_t Client::CMapNavigationContract::Write_RegionManifest(
	const std::string& areaId,
	const std::vector<MAP_NAVIGATION_REGION>& regions,
	std::string& outStatus)
{
	const std::filesystem::path path = Resolve_RegionManifestPath(areaId);
	if (path.empty() || regions.size() > MAX_REGION_COUNT)
	{
		outStatus = "Navigation region manifest target is invalid";
		return false;
	}

	std::ostringstream text;
	text << REGION_MANIFEST_MAGIC << ' ' << REGION_MANIFEST_VERSION << ' ' <<
		std::quoted(areaId) << ' ' << regions.size() << '\n';
	for (const MAP_NAVIGATION_REGION& region : regions)
	{
		if (!Is_ValidRegionId(region.regionId) ||
			!std::isfinite(region.stepHeight) ||
			region.stepHeight < 0.f)
		{
			outStatus = "Navigation region row is invalid: " + region.regionId;
			return false;
		}
		text << "REGION " << std::quoted(region.regionId) << ' ' <<
			std::setprecision(9) << region.stepHeight << '\n';
	}

	/* Written beside the target and moved over it so a failed write never
	   leaves a half manifest that would fail the next Area load. */
	std::filesystem::path staged = path;
	staged += L".staging";
	std::error_code error;
	std::filesystem::remove(staged, error);
	error.clear();
	{
		std::ofstream output(staged, std::ios::binary | std::ios::trunc);
		if (!output)
		{
			outStatus = "Could not open navigation region manifest for writing";
			return false;
		}
		const std::string payload = text.str();
		output.write(payload.data(),
			static_cast<std::streamsize>(payload.size()));
		output.flush();
		if (!output.good())
		{
			output.close();
			std::filesystem::remove(staged, error);
			outStatus = "Could not write navigation region manifest";
			return false;
		}
	}
	std::filesystem::rename(staged, path, error);
	if (error)
	{
		std::error_code cleanupError;
		std::filesystem::remove(staged, cleanupError);
		outStatus = "Could not promote navigation region manifest";
		return false;
	}

	outStatus = "Saved " + std::to_string(regions.size()) +
		" navigation regions for " + areaId;
	return true;
}
```

`<algorithm>`, `<cctype>`, `<fstream>`, `<iomanip>` 은 이미 이 파일에 포함돼 있다.
`std::isfinite` 를 쓰므로 `#include <cmath>` 가 없으면 `<sstream>` 아래에 함께 추가한다.

---

## G4. MapTool — 영역 선택과 생성

MapTool 은 `m_NavigationSourcePath` / `m_NavigationPaintPath` / `m_RuntimeBlockerPath` /
`m_NavigationRuntimePath` 네 경로로 문서를 읽고 쓴다. 영역 편집은 **이 네 경로를 선택한
격자 것으로 바꾸는 것**이 전부다. Bake·Paint·Save 본문은 손대지 않는다.

주의: `Load_NavigationDocument` 은 workspace 활성 여부로 두 갈래다. 앞 갈래는
`EDITOR_AREA_DESCRIPTOR` 의 고정 경로를 쓰고 `Save_Navigation` 이
`HasSameNavigationPath` 로 그 경로와 일치하는지 검사한다. 영역을 고르면 경로가 달라지므로
두 함수 모두 "선택이 base 일 때만 descriptor 경로를 강제한다" 로 바꾼다.

### G4-1. `Client/Public/MapTool.h`

파일: `Client/Public/MapTool.h`
작업: 추가
기준점: `bool_t Save_Navigation();`
위치: 바로 아래
정의 위치: `Client/Private/MapTool.cpp`, `Save_Navigation` 정의 뒤
필요한 이유: 선택 격자의 경로 해석, 영역 전환, 매니페스트 커밋, 영역 UI
연결되는 부분: `Load_NavigationDocument`, `Bake_Navigation`, `Render_NavigationPanel`

```cpp
	/* Resolves the paths of the grid currently selected in the Navigation
	   panel: the Area's base grid when no region is selected, otherwise
	   "<AreaId>.<regionId>". */
	bool_t Resolve_SelectedNavigationContract(
		MAP_NAVIGATION_CONTRACT& outContract,
		std::string& outStatus) const;
	/* Switches the panel to another grid and reloads its documents. A failed
	   load restores the previous selection so the editor never shows one
	   grid's paint over another grid's cells. */
	bool_t Select_NavigationRegion(const std::string& regionId);
	/* Adds the freshly baked region to the Area manifest. Called only after
	   Bake_Navigation has written its navsource. */
	bool_t Commit_NavigationRegionManifest();
	void Render_NavigationRegionControls();
```

파일: `Client/Public/MapTool.h`
작업: 추가
기준점: `bool_t m_bShowUnresolvedCells = false;`
위치: 바로 아래
필요한 이유: 선택 상태와 매니페스트 캐시. 빈 `m_NavigationRegionId` 가 base 격자를 뜻한다.
연결되는 부분: `Resolve_SelectedNavigationContract`, `Render_NavigationRegionControls`

```cpp
	/* Empty means the Area's base grid. Otherwise the region whose grid id is
	   "<AreaId>.<regionId>"; every navigation path in this tool then points at
	   that grid instead. */
	std::string m_NavigationRegionId;
	/* The Area manifest as loaded, so the combo does not read the file every
	   frame. Rewritten by Commit_NavigationRegionManifest. */
	std::vector<MAP_NAVIGATION_REGION> m_NavigationRegions;
	char m_NewNavigationRegionId[33] = "stage1";
	f32_t m_NewNavigationRegionStepHeight = 1.f;
```

`MapTool.h` 는 이미 `#include "MapNavigationContract.h"` 를 갖지 않는다면 헤더 상단
`#include "NavGridPaintDocument.h"` 바로 아래에 추가한다. (`MapTool.cpp` 는 16행에 이미 있다.)

### G4-2. `Client/Private/MapTool.cpp` — 선택 경로 해석

파일: `Client/Private/MapTool.cpp`
작업: 추가 (함수 정의 1개)
기준점: `bool_t Client::CMapTool::Try_PickNavigationCell(` 정의
위치: 그 정의 바로 위 (`Save_Navigation` 정의의 닫는 `}` 바로 아래)
헤더 선언: `Resolve_SelectedNavigationContract`

```cpp
bool_t Client::CMapTool::Resolve_SelectedNavigationContract(
	MAP_NAVIGATION_CONTRACT& outContract,
	std::string& outStatus) const
{
	const std::string& areaId = m_Catalog.Get_AreaId();
	if (m_NavigationRegionId.empty())
		return CMapNavigationContract::Resolve_Area(areaId, outContract, outStatus);
	return CMapNavigationContract::Resolve_Region(
		areaId, m_NavigationRegionId, outContract, outStatus);
}
```

파일: `Client/Private/MapTool.cpp`
작업: 교체 (`Load_NavigationDocument` 안, workspace 갈래)
기준점: workspace 갈래의
```cpp
		m_NavigationSourcePath = active->navigationSource;
		m_NavigationPaintPath = active->navigationPaint;
		m_RuntimeBlockerPath = active->navigationBlockers;
		m_NavigationRuntimePath.clear();
```
위치: 이 네 줄을 아래 블록으로 교체
필요한 이유: 영역이 선택돼 있으면 descriptor 의 base 경로가 아니라 영역 격자 경로를 써야 한다.

```cpp
		/* The descriptor owns the Area's base grid. A selected detail region is
		   a different grid beside it, so its paths come from the contract. */
		MAP_NAVIGATION_CONTRACT selectedContract;
		std::string selectedStatus;
		if (!m_NavigationRegionId.empty())
		{
			if (!Resolve_SelectedNavigationContract(
				selectedContract, selectedStatus))
			{
				m_NavigationStatus = selectedStatus;
				return false;
			}
			m_NavigationSourcePath = selectedContract.sourcePath;
			m_NavigationPaintPath = selectedContract.paintPath;
			m_RuntimeBlockerPath = selectedContract.blockerPath;
			m_NavigationRuntimePath = selectedContract.runtimePath;
		}
		else
		{
			m_NavigationSourcePath = active->navigationSource;
			m_NavigationPaintPath = active->navigationPaint;
			m_RuntimeBlockerPath = active->navigationBlockers;
			m_NavigationRuntimePath.clear();
		}
```

같은 갈래에서 그 아래의 `hasSource` 검사와 `stagedNavigation.Load(...)` 는 지금
`active->navigationSource` / `active->navigationPaint` / `active->navigationBlockers` 를
직접 쓴다. 세 인자를 각각 `m_NavigationSourcePath`, `m_NavigationPaintPath`,
`m_RuntimeBlockerPath` 로 바꾼다. 그리고 areaId 대조는 다음으로 바꾼다.

파일: `Client/Private/MapTool.cpp`
작업: 교체 (`Load_NavigationDocument` 안, workspace 갈래)
기준점: `stagedNavigation.Get_Desc().areaId != active->areaId ||`
위치: 그 한 줄 교체
필요한 이유: 영역 격자의 헤더 areaId 는 `<AreaId>.<regionId>` 다.

```cpp
			stagedNavigation.Get_Desc().areaId !=
				(m_NavigationRegionId.empty() ?
					active->areaId :
					active->areaId + "." + m_NavigationRegionId) ||
```

파일: `Client/Private/MapTool.cpp`
작업: 교체 (`Load_NavigationDocument` 안, non-workspace 갈래)
기준점: `	if (!CMapNavigationContract::Resolve_Area(\n\t\tm_Catalog.Get_AreaId(), stagedContract, stagedStatus))`
위치: 그 호출을 아래로 교체

```cpp
	if (!Resolve_SelectedNavigationContract(stagedContract, stagedStatus))
```

이 갈래의 나머지(`stagedContract.sourcePath` 등)는 그대로다. 다만 areaId 대조 한 줄도
합성 id 를 쓰도록 바꾼다.

파일: `Client/Private/MapTool.cpp`
작업: 교체 (`Load_NavigationDocument` 안, non-workspace 갈래)
기준점:
```cpp
	if (stagedNavigationDocument.Get_Desc().areaId !=
		stagedContract.areaId)
```
위치: 두 줄 교체
필요한 이유: `stagedContract.areaId` 는 이미 합성 id 이므로 그대로 맞다. 메시지만 격자 id 를 밝힌다.

```cpp
	if (stagedNavigationDocument.Get_Desc().areaId !=
		stagedContract.areaId)
	{
		m_NavigationStatus =
			"NavGrid source area does not match the selected grid: " +
			stagedContract.areaId;
		return false;
	}
```

(기존 `m_NavigationStatus = "NavGrid source area does not match active map area";`
한 줄을 위 메시지로 교체한다.)

### G4-3. `Save_Navigation` 경로 검사 완화

파일: `Client/Private/MapTool.cpp`
작업: 교체 (`Save_Navigation` 안)
기준점:
```cpp
		if (!HasSameNavigationPath(
			m_NavigationSourcePath, active->navigationSource) ||
			!HasSameNavigationPath(
				m_NavigationPaintPath, active->navigationPaint) ||
			!HasSameNavigationPath(
				m_RuntimeBlockerPath, active->navigationBlockers))
		{
			m_NavigationStatus =
				"Navigation paths do not match the immutable active Area";
			return false;
		}
```
위치: 블록 전체 교체
필요한 이유: 영역을 편집 중이면 경로가 descriptor 와 다른 게 정상이다. 대신 선택한 영역의
정확한 경로와 일치하는지를 검사해 격자를 섞어 쓰는 것을 막는다.

```cpp
		MAP_NAVIGATION_CONTRACT expected;
		std::string expectedStatus;
		if (!Resolve_SelectedNavigationContract(expected, expectedStatus))
		{
			m_NavigationStatus = expectedStatus;
			return false;
		}
		const std::filesystem::path& expectedSource =
			m_NavigationRegionId.empty() ?
			active->navigationSource : expected.sourcePath;
		const std::filesystem::path& expectedPaint =
			m_NavigationRegionId.empty() ?
			active->navigationPaint : expected.paintPath;
		const std::filesystem::path& expectedBlockers =
			m_NavigationRegionId.empty() ?
			active->navigationBlockers : expected.blockerPath;
		if (!HasSameNavigationPath(m_NavigationSourcePath, expectedSource) ||
			!HasSameNavigationPath(m_NavigationPaintPath, expectedPaint) ||
			!HasSameNavigationPath(m_RuntimeBlockerPath, expectedBlockers))
		{
			m_NavigationStatus =
				"Navigation paths do not match the selected grid";
			return false;
		}
```

같은 함수의 아래쪽 `m_NavigationDocument.Save_Paint(active->navigationPaint, status)` 와
`m_RuntimeBlockerDocument.Save(active->navigationBlockers, status)` 의 인자를 각각
`expectedPaint`, `expectedBlockers` 로 바꾼다.

### G4-4. Bake 가 선택 격자를 쓰도록

파일: `Client/Private/MapTool.cpp`
작업: 교체 (`Bake_Navigation` 안, 두 곳)
기준점: `	if (!CNavGridBaker::Build_Desc(\n\t\tm_Catalog.Get_AreaId(),`
위치: `Build_Desc` 와 `Build` 두 호출의 첫 인자
필요한 이유: 영역 격자의 navsource 헤더 areaId 는 합성 id 여야 서버·publisher 가 그 격자로 인식한다.

두 호출 위에 지역 변수를 하나 만들고 그것을 넘긴다. `Bake_Navigation` 의 첫 문장
`if (!Is_ValidNavigationBakeDesc(m_NavigationBakeDesc))` 블록 바로 아래에 추가:

```cpp
	/* A detail region bakes into its own grid id so its navsource, navpaint and
	   published navgrid all carry "<AreaId>.<regionId>". */
	const std::string bakeGridId = m_NavigationRegionId.empty() ?
		m_Catalog.Get_AreaId() :
		m_Catalog.Get_AreaId() + "." + m_NavigationRegionId;
```

그리고 `CNavGridBaker::Build_Desc(` 와 `CNavGridBaker::Build(` 의 첫 인자
`m_Catalog.Get_AreaId()` 를 `bakeGridId` 로 바꾼다.

파일: `Client/Private/MapTool.cpp`
작업: 교체 (`Bake_Navigation` 끝)
기준점:
```cpp
	cleanupBackups();
	m_eNavigationMode = NAVIGATION_MODE::WALKABILITY;
```
위치: `cleanupBackups();` 와 그 다음 줄 사이에 매니페스트 커밋을 넣는다.
필요한 이유: 영역의 navsource 가 생긴 뒤에야 매니페스트에 등록해야 publisher 가 없는 파일을 가리키지 않는다.

```cpp
	cleanupBackups();
	if (!m_NavigationRegionId.empty() && !Commit_NavigationRegionManifest())
		return false;
	m_eNavigationMode = NAVIGATION_MODE::WALKABILITY;
```

### G4-5. 영역 전환과 매니페스트 커밋

파일: `Client/Private/MapTool.cpp`
작업: 추가 (함수 정의 2개)
기준점: `bool_t Client::CMapTool::Resolve_SelectedNavigationContract(` 정의의 닫는 `}`
위치: 바로 아래
헤더 선언: `Select_NavigationRegion`, `Commit_NavigationRegionManifest`

```cpp
bool_t Client::CMapTool::Select_NavigationRegion(const std::string& regionId)
{
	if (!regionId.empty() &&
		!CMapNavigationContract::Is_ValidRegionId(regionId))
	{
		m_NavigationStatus = "Navigation region ID is invalid";
		return false;
	}
	if (regionId == m_NavigationRegionId)
		return true;

	const std::string previous = m_NavigationRegionId;
	m_NavigationRegionId = regionId;
	if (Load_NavigationDocument())
		return true;

	/* A region without a baked navsource is a normal state, and so is a
	   corrupt one: either way the editor must not keep showing the previous
	   grid's cells under the new selection. Load_NavigationDocument already
	   left an empty document and a status, so only restore the selection when
	   the previous grid still loads. */
	m_NavigationRegionId = previous;
	if (!Load_NavigationDocument())
		m_NavigationRegionId = regionId;
	return false;
}

bool_t Client::CMapTool::Commit_NavigationRegionManifest()
{
	if (m_NavigationRegionId.empty())
		return true;

	const std::string& areaId = m_Catalog.Get_AreaId();
	std::vector<MAP_NAVIGATION_REGION> staged;
	std::string status;
	if (!CMapNavigationContract::Read_RegionManifest(areaId, staged, status))
	{
		m_NavigationStatus = status;
		return false;
	}

	const auto existing = std::find_if(
		staged.begin(),
		staged.end(),
		[this](const MAP_NAVIGATION_REGION& region)
		{
			return region.regionId == m_NavigationRegionId;
		});
	if (existing != staged.end())
	{
		existing->stepHeight = m_NewNavigationRegionStepHeight;
	}
	else
	{
		MAP_NAVIGATION_REGION added;
		added.regionId = m_NavigationRegionId;
		added.stepHeight = m_NewNavigationRegionStepHeight;
		staged.push_back(std::move(added));
	}

	if (!CMapNavigationContract::Write_RegionManifest(areaId, staged, status))
	{
		m_NavigationStatus = status;
		return false;
	}
	m_NavigationRegions = std::move(staged);
	m_NavigationStatus = status;
	return true;
}
```

### G4-6. Navigation 패널의 영역 UI

파일: `Client/Private/MapTool.cpp`
작업: 추가 (함수 정의 1개)
기준점: `void Client::CMapTool::Render_NavigationBakeControls()` 정의
위치: 그 정의 바로 위
헤더 선언: `Render_NavigationRegionControls`

```cpp
void Client::CMapTool::Render_NavigationRegionControls()
{
	ImGui::SeparatorText("Grid");
	const std::string& areaId = m_Catalog.Get_AreaId();
	const std::string preview = m_NavigationRegionId.empty() ?
		areaId + " (base)" : m_NavigationRegionId;
	if (ImGui::BeginCombo("Region", preview.c_str()))
	{
		if (ImGui::Selectable(
			(areaId + " (base)").c_str(),
			m_NavigationRegionId.empty()))
		{
			Select_NavigationRegion(std::string{});
		}
		for (const MAP_NAVIGATION_REGION& region : m_NavigationRegions)
		{
			if (ImGui::Selectable(
				region.regionId.c_str(),
				region.regionId == m_NavigationRegionId))
			{
				m_NewNavigationRegionStepHeight = region.stepHeight;
				Select_NavigationRegion(region.regionId);
			}
		}
		ImGui::EndCombo();
	}
	ImGui::SameLine();
	if (ImGui::Button("Reload Regions"))
	{
		std::string status;
		if (CMapNavigationContract::Read_RegionManifest(
			areaId, m_NavigationRegions, status))
		{
			m_NavigationStatus = status;
		}
		else
		{
			m_NavigationStatus = status;
		}
	}

	ImGui::InputText(
		"New Region ID",
		m_NewNavigationRegionId,
		sizeof(m_NewNavigationRegionId));
	ImGui::DragFloat(
		"Region Step Height",
		&m_NewNavigationRegionStepHeight,
		0.05f,
		0.f,
		10.f);
	const std::string newRegionId = m_NewNavigationRegionId;
	ImGui::BeginDisabled(
		!CMapNavigationContract::Is_ValidRegionId(newRegionId));
	if (ImGui::Button("Create Region"))
	{
		/* Selecting a region with no navsource yet fails on purpose: the panel
		   drops into Bake with empty documents, which is exactly the state
		   needed to place Nav Bounds for it. */
		Select_NavigationRegion(newRegionId);
		m_eNavigationMode = NAVIGATION_MODE::BAKE;
	}
	ImGui::EndDisabled();
	ImGui::TextDisabled(
		"A region is a finer grid over part of this Area. Place Nav Bounds "
		"around the whole stage it covers, then Bake.");
}
```

파일: `Client/Private/MapTool.cpp`
작업: 추가 (`Render_NavigationPanel` 안)
기준점:
```cpp
	if (ImGui::RadioButton(
		"Bake",
		NAVIGATION_MODE::BAKE == m_eNavigationMode))
```
위치: 이 `if` 바로 위
필요한 이유: 어느 격자를 편집 중인지가 모드보다 먼저 보여야 한다.

```cpp
	Render_NavigationRegionControls();
	ImGui::Separator();
```

파일: `Client/Private/MapTool.cpp`
작업: 추가 (`Load_NavigationDocument` 안)
기준점: non-workspace 갈래의 `MAP_NAVIGATION_CONTRACT stagedContract;` 선언
위치: 그 선언 바로 위
필요한 이유: Area 를 열 때 매니페스트를 한 번 읽어 콤보를 채운다. 실패해도 로드를 막지 않는다.

```cpp
	std::string manifestStatus;
	(void)CMapNavigationContract::Read_RegionManifest(
		m_Catalog.Get_AreaId(), m_NavigationRegions, manifestStatus);
```

workspace 갈래에도 같은 두 문장을 `MAP_NAVIGATION_CONTRACT selectedContract;` 선언 바로 위에 넣는다.

---

## G5. 검증기·문서·검증 절차

### G5-1. `Tools/KoukuSaydonPipeline/validate_kouku_saydon_world_admission.py`

이 검증기는 쿠크 spawn 이 walkable 셀 위에 있는지 본다. 영역이 생기면 spawn 이 영역
안일 수 있으므로 서버와 같은 dispatch 로 골라야 한다. 안 고치면 영역 안 spawn 을
base 격자 기준으로 보고 잘못된 `navigation.spawn.blocked` 를 낸다.

파일: `Tools/KoukuSaydonPipeline/validate_kouku_saydon_world_admission.py`
작업: 추가
기준점: `def _validate_spawn_nav(`
위치: 그 정의 바로 위
필요한 이유: 매니페스트를 읽고 영역 격자를 함께 불러온다. 없으면 빈 리스트라 기존 동작 그대로다.

```python
def _parse_region_manifest(
    root: Path, area_id: str, stage: _Stage
) -> list[tuple[str, float]]:
    """Reads Data/Navigation/<AreaId>.navregions.

    A missing manifest means the Area has no detail regions, which is the
    normal state and not a finding.
    """
    relative = Path(f"Data/Navigation/{area_id}.navregions")
    path = root / relative
    if not path.is_file():
        return []
    try:
        tokens = path.read_text(encoding="utf-8").split()
    except OSError as error:
        stage.issue("navigation.regions.unreadable", relative, str(error))
        return []
    if len(tokens) < 4 or tokens[0] != "LOSTARK_NAVGRID_REGIONS" or tokens[1] != "1":
        stage.issue("navigation.regions.invalid", relative, "header is invalid")
        return []
    if tokens[2].strip('"') != area_id:
        stage.issue("navigation.regions.area.mismatch", relative, tokens[2])
        return []
    try:
        count = int(tokens[3])
    except ValueError:
        stage.issue("navigation.regions.invalid", relative, "region count is not an integer")
        return []
    if count > 64 or len(tokens) != 4 + 3 * count:
        stage.issue("navigation.regions.invalid", relative, f"expected {count} rows")
        return []
    regions: list[tuple[str, float]] = []
    seen: set[str] = set()
    for index in range(count):
        row = tokens[4 + 3 * index : 7 + 3 * index]
        region_id = row[1].strip('"')
        if row[0] != "REGION" or not re.fullmatch(r"[A-Za-z0-9_-]{1,32}", region_id):
            stage.issue("navigation.regions.invalid", relative, f"row {index} is invalid")
            return []
        if region_id in seen:
            stage.issue("navigation.regions.duplicate", relative, region_id)
            return []
        seen.add(region_id)
        try:
            step = float(row[2])
        except ValueError:
            stage.issue("navigation.regions.invalid", relative, f"row {index} step is invalid")
            return []
        if not math.isfinite(step) or step < 0.0:
            stage.issue("navigation.regions.invalid", relative, f"row {index} step is invalid")
            return []
        regions.append((region_id, step))
    return regions


def _select_grid(
    grids: list[tuple[str, _NavGrid]], x: float, z: float
) -> tuple[str, _NavGrid] | None:
    """Same dispatch as CServerNavigation::Select_Region.

    grids[0] is the base grid; the rest are detail regions in manifest order.
    The first region whose footprint contains the point owns it.
    """
    for grid_id, grid in grids[1:]:
        max_x = grid.origin_x + grid.width * grid.cell_size
        max_z = grid.origin_z + grid.height * grid.cell_size
        if grid.origin_x <= x < max_x and grid.origin_z <= z < max_z:
            return grid_id, grid
    return grids[0] if grids else None
```

`re` 와 `math` 는 이 파일에 이미 import 돼 있다. 없으면 상단 import 에 추가한다.

파일: `Tools/KoukuSaydonPipeline/validate_kouku_saydon_world_admission.py`
작업: 교체
기준점: `def _validate_spawn_nav(` 의 시그니처와 본문 전체
위치: 함수 전체 교체
필요한 이유: 단일 격자 대신 격자 목록을 받아 점마다 소유 격자를 고른다.

```python
def _validate_spawn_nav(
    spawns: list[dict[str, Any]],
    grids: list[tuple[str, _NavGrid]],
    stage: _Stage,
) -> None:
    if not grids:
        return
    for spawn in spawns:
        position = spawn.get("position")
        placement_id = spawn.get("placementId", "<invalid>")
        if not isinstance(position, list) or len(position) != 3:
            continue
        try:
            x, y, z = (float(value) for value in position)
        except (TypeError, ValueError):
            continue
        selected = _select_grid(grids, x, z)
        if selected is None:
            continue
        grid_id, grid = selected
        cell_x = math.floor((x - grid.origin_x) / grid.cell_size)
        cell_z = math.floor((z - grid.origin_z) / grid.cell_size)
        if not (0 <= cell_x < grid.width and 0 <= cell_z < grid.height):
            stage.issue(
                "navigation.spawn.outside",
                GAMEPLAY_PATH,
                f"{placement_id}: grid={grid_id} cell=({cell_x},{cell_z})",
            )
            continue
        index = cell_z * grid.width + cell_x
        if grid.walkable[index] != 1:
            stage.issue(
                "navigation.spawn.blocked",
                GAMEPLAY_PATH,
                f"{placement_id}: grid={grid_id} cell=({cell_x},{cell_z}) is not walkable",
            )
        if abs(y - grid.heights[index]) > 0.25:
            stage.issue(
                "navigation.spawn.height",
                GAMEPLAY_PATH,
                f"{placement_id}: grid={grid_id} worldY={y} navY={grid.heights[index]}",
            )
```

파일: `Tools/KoukuSaydonPipeline/validate_kouku_saydon_world_admission.py`
작업: 교체
기준점:
```python
    grid = _parse_navgrid(root, server_grid_path, stage)
    _validate_spawn_nav(spawns, grid, stage)
```
위치: 두 줄 교체
필요한 이유: 매니페스트의 영역 격자도 로드하고 Client/Server 동일성도 영역까지 검사한다.

```python
    grid = _parse_navgrid(root, server_grid_path, stage)
    grids: list[tuple[str, _NavGrid]] = []
    if grid is not None:
        grids.append((AREA_ID, grid))
    for region_id, _step in _parse_region_manifest(root, AREA_ID, stage):
        region_grid_id = f"{AREA_ID}.{region_id}"
        region_server = Path(f"Server/Bin/DataFiles/Navigation/{region_grid_id}.navgrid")
        region_client = Path(f"Client/Bin/DataFiles/Navigation/{region_grid_id}.navgrid")
        region_grid = _parse_navgrid(root, region_server, stage)
        if region_grid is not None:
            grids.append((region_grid_id, region_grid))
        _require_same_bytes(
            root,
            region_server,
            region_client,
            stage,
            "navigation.client-server.drift",
        )
    _validate_spawn_nav(spawns, grids, stage)
```

### G5-2. `Tools/KoukuSaydonPipeline/test_kouku_saydon_world_admission.py`

파일: `Tools/KoukuSaydonPipeline/test_kouku_saydon_world_admission.py`
작업: 추가
기준점: `    def test_random_or_non_walkable_spawn_is_never_admitted(self) -> None:`
위치: 그 메서드 바로 위
필요한 이유: 영역이 spawn 판정을 가로채는 것과, 영역 안 막힌 칸이 잡히는 것을 함께 고정한다.

```python
    def test_detail_region_owns_the_spawn_cell_it_covers(self) -> None:
        """A spawn inside a region footprint is judged on the region grid."""
        self.fixture.install_product()
        # The base grid is 2x2 of 1 m at the origin. Put a 2x2 region of 0.5 m
        # over the cell that holds the spawn and make its cells blocked, so a
        # finding can only come from the region grid.
        self.fixture.write_text(
            f"Data/Navigation/{AREA_ID}.navregions",
            f'LOSTARK_NAVGRID_REGIONS 1 "{AREA_ID}" 1\nREGION "fine" 0.75\n',
        )
        region_grid = (
            struct.pack("<IIfff", 2, 2, 0.5, 0.0, 0.0)
            + bytes((0, 0, 0, 0))
            + struct.pack("<4f", 0.0, 0.0, 0.0, 0.0)
        )
        for output in ("Server/Bin/DataFiles", "Client/Bin/DataFiles"):
            self.fixture.write_bytes(
                f"{output}/Navigation/{AREA_ID}.fine.navgrid", region_grid
            )

        report = validate_repository(self.fixture.root)
        findings = report.result(PRODUCT_MODE).own_findings
        blocked = [
            finding for finding in findings
            if finding.code == "navigation.spawn.blocked"
        ]
        self.assertTrue(blocked)
        self.assertIn(f"{AREA_ID}.fine", blocked[0].detail)
```

`install_product` 의 spawn 은 base 격자가 전부 walkable 이라 원래는 findings 가 없다.
영역을 전부 막았으므로 finding 이 나오면 dispatch 가 영역을 골랐다는 뜻이다.

### G5-3. 문서

파일: `CLAUDE.md`
작업: 교체
기준점: `- 서버 길찾기: `Data/Navigation`이 정본이다.` 로 시작하는 항목 (564행)
위치: 그 항목의 마지막 문장 뒤에 두 문장 추가
필요한 이유: public 계약이 바뀌므로 정본 문서에 반영한다.

추가할 문장:

```text
Area는 선택적으로 `Data/Navigation/<AreaId>.navregions`에 세부 영역 격자를 선언한다. 각 영역은 `<AreaId>.<regionId>` grid ID로 자기 `.navsource/.navpaint`와 런타임 `.navgrid/.navpolicy`를 갖고, Server는 질의의 첫 점을 담는 영역이 있으면 그 격자에서만 판정한다. 영역은 서로 겹칠 수 없고 runtime blocker를 갖지 않으며, 매니페스트가 없으면 Area는 기본 격자 하나로 종전과 동일하게 동작한다.
```

파일: `.md/TEAM/AREA_DATA_LAYER_GUIDE.md`
작업: 추가
기준점: navigation 레이어를 설명하는 표의 `LV_LUT_MIDNIGHTC_ED` 행
위치: navigation 열의 설명 끝에 `+ optional detail regions` 표기, 그리고 문서 하단
`navigation` 절에 한 문단 추가
필요한 이유: Area 담당자가 영역을 언제 쓰는지 알아야 한다.

```text
한 Area 안에서 스테이지마다 필요한 정밀도가 다르면 세부 영역 격자를 쓴다.
`Data/Navigation/<AreaId>.navregions`에 `REGION "<regionId>" <stepHeight>` 행을 두면
MapTool의 Navigation 패널에서 그 영역을 골라 별도 Nav Bounds와 Cell Size로 Bake한다.
영역은 자기가 덮는 스테이지의 걷는 범위 전체를 덮어야 한다. 플레이어가 걸어서 영역
밖으로 나가는 지형에는 쓰지 않는다. 영역끼리 겹치면 publisher와 Server가 모두 거부한다.
```

### G5-4. 검증 절차

```text
1. Server + Shared + Client Debug 빌드
   powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product
   -> Visual Studio를 닫고, 실행 중인 Client.exe/Server.exe를 먼저 종료한다.

2. publisher 계약 (영역 0개 회귀 + 영역 케이스)
   powershell -ExecutionPolicy Bypass -File Tools/NavigationPipeline/Publish-ServerNavigation.ps1 -Mode ContractTest
   -> 기대: 'Server navigation ContractTest succeeded'

3. publisher 검증 (매니페스트 없는 현재 데이터가 종전과 같은지)
   powershell -ExecutionPolicy Bypass -File Tools/NavigationPipeline/Publish-ServerNavigation.ps1 -Mode Validate
   -> 기대: 5개 Area 모두 succeeded, LV_LUT_MIDNIGHTC_ED가 524x800 cellSize=4 walkable=2278,
      region 줄 없음

4. Server 계약 테스트
   Server\Bin\Debug\Server.exe --contract-test
   -> 기대: 'navigation failures : 0'

5. Python 검증기
   python Tools/KoukuSaydonPipeline/test_kouku_saydon_world_admission.py
   -> 기대: 새 테스트 포함 전부 통과

6. 사용자 수동 검증 (에이전트가 대신 판정하지 않는다)
   F1 -> Map Tool -> Navigation
   a. Region 콤보에 '<AreaId> (base)' 하나만 보이고 기존 편집이 종전대로 되는지
   b. New Region ID 'stage1' -> Create Region -> Bake 모드로 전환되는지
   c. 1스테이지 아레나에 Nav Bounds를 놓고 Cell Size 0.5로 Bake
   d. Walkability로 전환해 파란 메시 위를 Force Walkable로 칠했을 때
      바닥이 아니라 그 메시 높이에 셀이 생기는지
   e. Save Navigation -> Data/Navigation/LV_LUT_MIDNIGHTC_ED.navregions와
      LV_LUT_MIDNIGHTC_ED.stage1.navsource/.navpaint가 생겼는지
   f. Publish 후 Server 재시작 -> 1스테이지에서 이동/추적이 되는지,
      다른 스테이지가 종전대로인지
```

### G5-5. 완료 조건

```text
구현 완료: G1~G5의 코드가 빌드되고 2~5가 통과
자동 검증: ContractTest, Validate, Server --contract-test, Python 테스트
수동 검증: 6번 a~f를 사용자가 직접 실행하고 서면으로 판정
미포함: stage1 영역의 실제 navsource/navpaint 데이터. 사용자가 툴에서 Bake해야 생긴다.
미포함: 4 m 기본 격자 자체의 개선. 영역을 만들기 전까지 쿠크는 종전과 동일하다.
```

## 남은 경계

- 영역은 A* 가 넘나들지 않는다. 걸어서 이동하는 두 구역을 서로 다른 영역으로 나누면
  경로가 끊긴다. 쿠크는 스테이지 사이가 `movePlayer`/시퀀스라 문제가 없지만, Bern 처럼
  연속된 맵에는 이 기능을 쓰지 않는다.
- 영역은 runtime blocker/void condition 을 갖지 않는다. 발탄 파괴처럼 조건부로 바닥이
  사라지는 구역은 기본 격자에 남겨야 한다.
- `Get_CellSize()` 가 최소값을 돌려주므로 영역을 아주 잘게 만들면 Server 의 body sweep
  샘플링 횟수가 늘어난다. 현재 호출자는 전부 `clamp`/`max` 로 하한을 두고 있어 정확성
  문제는 없지만, 0.1 m 미만 영역은 피한다.




