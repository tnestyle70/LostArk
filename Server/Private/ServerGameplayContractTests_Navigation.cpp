#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "PlayerSkillSystem.h"
#include "ServerNavigation.h"
#include "WorldDestructionBootstrapContractTests.h"
#include <Windows.h>
#include <process.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <span>
#include <sstream>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>


using namespace LostArk::Server;
using namespace LostArk::Shared;



int LostArk::Server::Run_ServerNavigationContractTests()
{
	TESTS tests;
	CServerNavigation bernNavigation;
	const bool loaded = bernNavigation.Load("LV_BER_BERNCASTLE");
	tests.Require(
		loaded &&
		std::abs(
			bernNavigation.Get_MaximumTraversalStepHeight() - 1.f) < 0.001f &&
		bernNavigation.Is_HeightTransitionAllowed(47.f, 47.9f) &&
		!bernNavigation.Is_HeightTransitionAllowed(47.f, 53.f),
		"Load Bern navigation with a one-metre deck-step guard");

	SERVER_NAV_POINT lockedGround{};
	SERVER_NAV_POINT sampledGround{};
	const bool lockedWalkStep = loaded &&
		bernNavigation.Sample_Position(
			138.238007f, -110.188004f, sampledGround) &&
		bernNavigation.Resolve_TraversalStep(
			138.238007f,
			-110.188004f,
			138.300003f,
			-110.099998f,
			lockedGround);
	tests.Require(
		lockedWalkStep &&
		std::abs(lockedGround.y - sampledGround.y) < 0.000001f,
		"Lock a live Bern walk step to the destination navigation ground");

	SERVER_NAV_POINT rejectedDeckJump{};
	tests.Require(
		loaded && !bernNavigation.Resolve_TraversalStep(
			125.238007f,
			-170.688004f,
			125.238007f,
			-170.188004f,
			rejectedDeckJump),
		"Reject the real Bern 17-metre adjacent deck jump at live movement time");

	SERVER_NAV_POINT clampedDeckStep{
		125.238007f, 39.04832f, -170.688004f };
	bool deckStepClamped = false;
	if (loaded)
	{
		CPlayerSkillSystem::Clamp_StepToWalkable(
			bernNavigation,
			125.238007f,
			-170.688004f,
			125.238007f,
			-170.188004f,
			clampedDeckStep,
			deckStepClamped);
	}
	tests.Require(
		loaded && deckStepClamped && clampedDeckStep.y < 40.f &&
		clampedDeckStep.z < -170.438004f,
		"Clamp Bern skill and knockback movement before the upper deck");

	std::vector<SERVER_NAV_POINT> stairPath;
	const bool stairPathFound = loaded && bernNavigation.Find_Path(
		137.586334f,
		-22.4640217f,
		137.238007f,
		-116.688004f,
		stairPath);
	bool transitionsSafe = stairPathFound && stairPath.size() > 100u;
	float maximumPathStep = 0.f;
	for (std::size_t index = 1u; index < stairPath.size(); ++index)
	{
		const float step = std::abs(
			stairPath[index].y - stairPath[index - 1u].y);
		maximumPathStep = (std::max)(maximumPathStep, step);
		transitionsSafe = transitionsSafe &&
			bernNavigation.Is_HeightTransitionAllowed(
				stairPath[index - 1u].y, stairPath[index].y);
	}
	tests.Require(
		transitionsSafe && maximumPathStep <= 1.000001f &&
		!stairPath.empty() && stairPath.back().y > 49.f &&
		stairPath.back().y - stairPath.front().y > 6.f,
		"Reach the Bern stairs without crossing an arch-roof height jump");

    std::vector<SERVER_NAV_POINT> southPath;
    SERVER_NAV_POINT southRidge{};
    const bool southConnected = loaded && bernNavigation.Find_Path(
        137.586334f, -22.4640217f, 137.162415f, -167.909286f, southPath);
    tests.Require(southConnected && !southPath.empty() && southPath.back().y > 53.f &&
        bernNavigation.Sample_Position(137.238007f, -152.188004f, southRidge) &&
        std::abs(southRidge.y - 50.5766411f) < 0.001f &&
        std::adjacent_find(southPath.begin(), southPath.end(), [&](const auto& a, const auto& b) {
            return !bernNavigation.Is_HeightTransitionAllowed(a.y, b.y);
        }) == southPath.end(), "Reach the south Bern NPC stairs on the installed STAIR02D floor while preserving the 1m guard");
	SERVER_NAV_POINT firstRidge{};
	SERVER_NAV_POINT secondRidge{};
	SERVER_NAV_POINT thirdRidge{};
	tests.Require(
		loaded &&
		bernNavigation.Sample_Position(
			138.238007f, -110.188004f, firstRidge) &&
		bernNavigation.Sample_Position(
			138.238007f, -97.688004f, secondRidge) &&
		bernNavigation.Sample_Position(
			137.738007f, -67.688004f, thirdRidge) &&
		firstRidge.y < 48.f && secondRidge.y < 48.f && thirdRidge.y < 44.f,
		"Publish all three repaired Bern corridor ridges on the ground deck");

	namespace fs = std::filesystem;
	std::vector<wchar_t> pathBuffer(32768u);
	const DWORD configuredLength = GetEnvironmentVariableW(
		L"LOSTARK_SERVER_DATA_ROOT",
		pathBuffer.data(),
		static_cast<DWORD>(pathBuffer.size()));
	const bool hadConfiguredRoot =
		0u != configuredLength && configuredLength < pathBuffer.size();
	fs::path packagedDataRoot;
	if (hadConfiguredRoot)
	{
		packagedDataRoot = fs::path(pathBuffer.data()).lexically_normal();
	}
	else
	{
		const DWORD moduleLength = GetModuleFileNameW(
			nullptr,
			pathBuffer.data(),
			static_cast<DWORD>(pathBuffer.size()));
		if (0u != moduleLength && moduleLength < pathBuffer.size())
		{
			packagedDataRoot = fs::path(pathBuffer.data()).parent_path().
				parent_path() / L"DataFiles";
		}
	}
	const fs::path invalidPolicyRoot = fs::temp_directory_path() /
		(L"LostArkNavigationPolicyContractTest-" +
			std::to_wstring(_getpid()));
	std::error_code fixtureError;
	fs::remove_all(invalidPolicyRoot, fixtureError);
	fixtureError.clear();
	fs::create_directories(
		invalidPolicyRoot / L"Navigation", fixtureError);
	if (!fixtureError)
	{
		fs::copy_file(
			packagedDataRoot / L"Navigation" /
				L"LV_BER_BERNCASTLE.navgrid",
			invalidPolicyRoot / L"Navigation" /
				L"LV_BER_BERNCASTLE.navgrid",
			fs::copy_options::overwrite_existing,
			fixtureError);
	}
	if (!fixtureError)
	{
		std::ofstream invalidPolicy(
			invalidPolicyRoot / L"Navigation" /
				L"LV_BER_BERNCASTLE.navpolicy",
			std::ios::binary | std::ios::trunc);
		invalidPolicy <<
			"LOSTARK_NAVIGATION_POLICY 1 \"WRONG_AREA\" 1\n";
		if (!invalidPolicy.good())
			fixtureError = std::make_error_code(std::errc::io_error);
	}
	bool rejectedInvalidPolicyTransactionally = false;
	if (!fixtureError && SetEnvironmentVariableW(
		L"LOSTARK_SERVER_DATA_ROOT", invalidPolicyRoot.c_str()))
	{
		CServerNavigation rejectedNavigation;
		rejectedInvalidPolicyTransactionally =
			!rejectedNavigation.Load("LV_BER_BERNCASTLE") &&
			!rejectedNavigation.Is_Loaded();
		SetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT",
			hadConfiguredRoot ? pathBuffer.data() : nullptr);
	}
	// A sub-cell corner crossing can be shorter than any fixed LOS sampling
	// interval. Reuse the navigation fixture directory for the exact cell walk.
	std::array<float, 16u> losHeights{};
	losHeights[15u] = 1.5f;
	const auto writeLosGrid = [&]()
	{
		std::ofstream grid(
			invalidPolicyRoot / L"Navigation" / L"NAV_LOS_CONTRACT.navgrid",
			std::ios::binary | std::ios::trunc);
		const std::uint32_t width = 4u;
		const std::uint32_t height = 4u;
		const float cellSize = 1.f;
		const float origin = 0.f;
		const std::array<std::uint8_t, 16u> walkable{
			1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u,
			1u, 1u, 1u, 1u, 1u, 1u, 1u, 1u };
		grid.write(reinterpret_cast<const char*>(&width), sizeof(width));
		grid.write(reinterpret_cast<const char*>(&height), sizeof(height));
		grid.write(reinterpret_cast<const char*>(&cellSize), sizeof(cellSize));
		grid.write(reinterpret_cast<const char*>(&origin), sizeof(origin));
		grid.write(reinterpret_cast<const char*>(&origin), sizeof(origin));
		grid.write(reinterpret_cast<const char*>(walkable.data()), walkable.size());
		grid.write(reinterpret_cast<const char*>(losHeights.data()),
			static_cast<std::streamsize>(losHeights.size() * sizeof(float)));
		return grid.good();
	};
	const auto writeLosPolicy = [&](const float maximumStep)
	{
		std::ofstream policy(
			invalidPolicyRoot / L"Navigation" / L"NAV_LOS_CONTRACT.navpolicy",
			std::ios::binary | std::ios::trunc);
		policy << "LOSTARK_NAVIGATION_POLICY 1 \"NAV_LOS_CONTRACT\" " <<
			maximumStep << '\n';
		return policy.good();
	};
	bool losFilesReady = !fixtureError && writeLosGrid() && writeLosPolicy(1.f);
	if (losFilesReady)
	{
		std::ofstream blockers(
			invalidPolicyRoot / L"Navigation" / L"NAV_LOS_CONTRACT.navblockers",
			std::ios::binary | std::ios::trunc);
		blockers <<
			"LOSTARK_NAVGRID_BLOCKERS 1 \"NAV_LOS_CONTRACT\" 4 4 1 0 0 2\n"
			"REGION \"contract.wall\" \"contract.wall.open\" 0 1\n1 1\n"
			"REGION \"contract.floor\" \"contract.floor.collapsed\" 1 1\n2 1\n";
		losFilesReady = blockers.good();
	}
	CServerNavigation cornerNavigation;
	const bool cornerLoaded = losFilesReady && SetEnvironmentVariableW(
		L"LOSTARK_SERVER_DATA_ROOT", invalidPolicyRoot.c_str()) &&
		cornerNavigation.Load("NAV_LOS_CONTRACT");
	tests.Require(cornerLoaded, "Load the exact-cell LOS contract fixture");
	tests.Require(
		cornerLoaded &&
		!cornerNavigation.Has_LineOfSight(0.25f, 1.751f, 1.751f, 0.25f) &&
		!cornerNavigation.Has_LineOfSight(1.751f, 0.25f, 0.25f, 1.751f) &&
		!cornerNavigation.Has_LineOfSight(0.25f, 1.75f, 1.75f, 0.25f),
		"Reject a clipped live-blocker corner and an exact diagonal corner in both directions");
	tests.Require(
		cornerLoaded &&
		!cornerNavigation.Has_LineOfSight(2.f, 0.5f, 2.f, 2.5f) &&
		!cornerNavigation.Has_LineOfSight(2.f, 2.5f, 2.f, 0.5f) &&
		cornerNavigation.Has_LineOfSight(0.f, 0.5f, 0.f, 3.5f),
		"Check both sides of an internal grid edge without rejecting the valid outer grid edge");
	std::vector<SERVER_NAV_POINT> cornerPath;
	bool cornerDetourSafe = cornerLoaded && cornerNavigation.Find_Path(
		0.25f, 1.751f, 1.751f, 0.25f, cornerPath);
	if (cornerDetourSafe)
	{
		cornerNavigation.Smooth_Path(
			0.25f, 1.751f, 1.751f, 0.25f, cornerPath);
		cornerDetourSafe = cornerPath.size() >= 2u;
		float fromX = 0.25f;
		float fromZ = 1.751f;
		for (const SERVER_NAV_POINT& point : cornerPath)
		{
			cornerDetourSafe = cornerDetourSafe &&
				cornerNavigation.Has_LineOfSight(fromX, fromZ, point.x, point.z);
			fromX = point.x;
			fromZ = point.z;
		}
	}
	tests.Require(cornerDetourSafe, "Keep the A-star detour when smoothing would clip a blocked corner");
	SERVER_NAVIGATION_CONDITION_STAGE openedWall;
	std::string losStatus;
	const bool wallOpened = cornerLoaded && cornerNavigation.Prepare_ConditionChanges(
		{ { "contract.wall.open", true } }, openedWall, losStatus);
	if (wallOpened)
		cornerNavigation.Commit_ConditionChanges(std::move(openedWall));
	if (wallOpened)
		cornerNavigation.Smooth_Path(0.25f, 1.751f, 1.751f, 0.25f, cornerPath);
	tests.Require(
		wallOpened && cornerPath.size() == 1u &&
		cornerNavigation.Has_LineOfSight(0.25f, 1.751f, 1.751f, 0.25f) &&
		cornerNavigation.Has_LineOfSight(2.f, 0.5f, 2.f, 2.5f),
		"Allow the same corner and grid-edge shortcuts after the wall condition commits");
	SERVER_NAVIGATION_CONDITION_STAGE collapsedFloor;
	const bool floorCollapsed = wallOpened && cornerNavigation.Set_VoidConditions(
		{ "contract.floor.collapsed" }, losStatus) &&
		cornerNavigation.Prepare_ConditionChanges(
			{ { "contract.floor.collapsed", true } }, collapsedFloor, losStatus);
	if (floorCollapsed)
		cornerNavigation.Commit_ConditionChanges(std::move(collapsedFloor));
	tests.Require(
		floorCollapsed &&
		!cornerNavigation.Has_LineOfSight(2.5f, 0.5f, 2.5f, 2.5f) &&
		!cornerNavigation.Has_LineOfSight(2.5f, 1.5f, 2.5f, 1.5f) &&
		cornerNavigation.Is_PointInVoidRegion(2.5f, 1.5f),
		"Reject a committed floor void even when the wall has opened");
	tests.Require(
		cornerLoaded &&
		!cornerNavigation.Has_LineOfSight(2.5f, 3.5f, 3.5f, 3.5f) &&
		!cornerNavigation.Has_LineOfSight(0.5f, 0.5f, 4.f, 0.5f) &&
		!cornerNavigation.Has_LineOfSight(
			(std::numeric_limits<float>::quiet_NaN)(), 0.5f, 0.5f, 0.5f) &&
		cornerNavigation.Has_LineOfSight(0.5f, 0.5f, 0.5f, 0.5f),
		"Preserve the live deck-step guard, exact endpoint validation, and an open zero-length query");
	CServerNavigation unrestrictedNavigation;
	const bool unrestrictedLoaded = cornerLoaded && writeLosPolicy(0.f) &&
		unrestrictedNavigation.Load("NAV_LOS_CONTRACT");
	tests.Require(
		unrestrictedLoaded &&
		unrestrictedNavigation.Has_LineOfSight(2.5f, 3.5f, 3.5f, 3.5f),
		"Preserve maps that deliberately disable the adjacent deck-step guard");
	losHeights[15u] = 3.f;
	CServerNavigation tallStepNavigation;
	tests.Require(
		unrestrictedLoaded && writeLosGrid() &&
		tallStepNavigation.Load("NAV_LOS_CONTRACT") &&
		!tallStepNavigation.Has_LineOfSight(2.5f, 3.5f, 3.5f, 3.5f),
		"Retain the LOS height-interpolation limit when deck-step policy is disabled");
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
		{
			grid.write(
				reinterpret_cast<const char*>(&walkable), sizeof(walkable));
		}
		for (std::size_t index = 0u; index < cellCount; ++index)
		{
			grid.write(
				reinterpret_cast<const char*>(&cellHeight), sizeof(cellHeight));
		}
		grid.close();
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
		policy.close();
		return policy.good();
	};
	const auto writeRegionManifest = [&](const char* text)
	{
		std::ofstream manifest(
			invalidPolicyRoot / L"Navigation" /
				L"NAV_REGION_CONTRACT.navregions",
			std::ios::binary | std::ios::trunc);
		manifest << text;
		manifest.close();
		return manifest.good();
	};
	const bool regionFixtureReady =
		writeRegionGrid(L"NAV_REGION_CONTRACT", 4u, 4u, 1.f, 0.f, 0.f, 0.f) &&
		writeRegionPolicy(L"NAV_REGION_CONTRACT", "NAV_REGION_CONTRACT", 1.f) &&
		writeRegionGrid(
			L"NAV_REGION_CONTRACT.fine", 4u, 4u, 0.5f, 1.f, 1.f, 2.f) &&
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
		std::abs(
			regionNavigation.Get_MaximumTraversalStepHeight() - 1.f) <
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
			point.x >= 1.f && point.x < 3.f &&
			point.z >= 1.f && point.z < 3.f;
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
			L"NAV_REGION_CONTRACT.overlap",
			"NAV_REGION_CONTRACT.overlap",
			0.75f) &&
		writeRegionManifest(
			"LOSTARK_NAVGRID_REGIONS 1 \"NAV_REGION_CONTRACT\" 2\n"
			"REGION \"fine\" 0.75\n"
			"REGION \"overlap\" 0.75\n") &&
		!overlappingNavigation.Load("NAV_REGION_CONTRACT"),
		"Reject two regions whose footprints overlap");
	/* Stacked height layers: two regions may share one XZ footprint, and the
	   height hint of a query tells them apart. */
	const auto writeLayer = [&](
		const wchar_t* gridStem,
		const char* gridId,
		const float cellHeight,
		const float step)
	{
		return writeRegionGrid(gridStem, 4u, 4u, 0.5f, 1.f, 1.f, cellHeight) &&
			writeRegionPolicy(gridStem, gridId, step);
	};
	const auto punchGroundHole = [&](
		const wchar_t* gridStem, const std::streamoff cellIndex)
	{
		std::fstream grid(
			invalidPolicyRoot / L"Navigation" /
				(std::wstring(gridStem) + L".navgrid"),
			std::ios::binary | std::ios::in | std::ios::out);
		const char blocked = 0;
		grid.seekp(20 + cellIndex);
		grid.write(&blocked, 1);
		grid.close();
		return grid.good();
	};
	const auto loadLayerPair = [&](
		const char* firstId,
		const char* secondId,
		const float step,
		CServerNavigation& navigation)
	{
		const std::string stepText = std::to_string(step);
		const std::string manifest =
			"LOSTARK_NAVGRID_REGIONS 1 \"NAV_REGION_CONTRACT\" 2\n"
			"REGION \"" + std::string(firstId) + "\" " + stepText + "\n"
			"REGION \"" + std::string(secondId) + "\" " + stepText + "\n";
		return writeRegionManifest(manifest.c_str()) &&
			navigation.Load("NAV_REGION_CONTRACT");
	};
	const auto sampleLayerAt = [&](
		const CServerNavigation& navigation,
		const float x, const float z, const float hintY, SERVER_NAV_POINT& out)
	{ return navigation.Sample_Position(x, z, out, hintY); };
	const auto stepLayerFrom = [&](
		const CServerNavigation& navigation,
		const float fromX, const float fromZ, const float toX, const float toZ,
		const float fromY, SERVER_NAV_POINT& out)
	{ return navigation.Resolve_TraversalStep(fromX, fromZ, toX, toZ, out, fromY); };
	const auto pathLayerFrom = [&](
		const CServerNavigation& navigation,
		const float startX, const float startZ, const float goalX, const float goalZ,
		const float startY, std::vector<SERVER_NAV_POINT>& out)
	{ return navigation.Find_Path(startX, startZ, goalX, goalZ, out, startY); };
	const auto walkableLayerAt = [&](
		const CServerNavigation& navigation,
		const float x, const float z, const float hintY)
	{ return navigation.Is_PointWalkableExact(x, z, hintY); };
	const auto projectLayerAt = [&](
		const CServerNavigation& navigation,
		const float x, const float z, const float hintY, SERVER_NAV_POINT& out)
	{ return navigation.Project_Point(x, z, out, hintY); };
	const auto losLayerFrom = [&](
		const CServerNavigation& navigation,
		const float startX, const float startZ, const float endX, const float endZ,
		const float startY)
	{ return navigation.Has_LineOfSight(startX, startZ, endX, endZ, startY); };
	const bool layersReady = regionFixtureReady &&
		writeLayer(L"NAV_REGION_CONTRACT.upper", "NAV_REGION_CONTRACT.upper", 10.f, 0.75f) &&
		writeLayer(L"NAV_REGION_CONTRACT.lower", "NAV_REGION_CONTRACT.lower", 2.f, 0.75f);
	CServerNavigation layeredNavigation;
	const bool layersLoaded = layersReady &&
		loadLayerPair("upper", "lower", 0.75f, layeredNavigation);
	tests.Require(
		layersLoaded && 2u == layeredNavigation.Get_RegionCount(),
		"Load two regions that share one XZ footprint as separate height layers");
	SERVER_NAV_POINT upperGround{};
	SERVER_NAV_POINT lowerGround{};
	SERVER_NAV_POINT unhintedGround{};
	tests.Require(
		layersLoaded &&
		sampleLayerAt(layeredNavigation, 1.25f, 1.25f, 10.3f, upperGround) &&
		std::abs(upperGround.y - 10.f) < 0.000001f &&
		sampleLayerAt(layeredNavigation, 1.25f, 1.25f, 1.7f, lowerGround) &&
		std::abs(lowerGround.y - 2.f) < 0.000001f &&
		layeredNavigation.Sample_Position(1.25f, 1.25f, unhintedGround) &&
		std::abs(unhintedGround.y - 10.f) < 0.000001f,
		"Answer a stacked query from the layer nearest its height hint, and from the first declared layer without one");
	SERVER_NAV_POINT upperStep{};
	SERVER_NAV_POINT lowerStep{};
	tests.Require(
		layersLoaded &&
		stepLayerFrom(layeredNavigation, 1.25f, 1.25f, 1.75f, 1.25f, 10.f, upperStep) &&
		std::abs(upperStep.y - 10.f) < 0.000001f &&
		stepLayerFrom(layeredNavigation, 1.25f, 1.25f, 1.75f, 1.25f, 2.f, lowerStep) &&
		std::abs(lowerStep.y - 2.f) < 0.000001f,
		"Walk a live step on the layer that carries the mover's height");
	std::vector<SERVER_NAV_POINT> upperPath;
	std::vector<SERVER_NAV_POINT> lowerPath;
	bool layerPathsStayPut = layersLoaded &&
		pathLayerFrom(layeredNavigation, 1.25f, 1.25f, 2.75f, 2.75f, 10.f, upperPath) &&
		!upperPath.empty() &&
		pathLayerFrom(layeredNavigation, 1.25f, 1.25f, 2.75f, 2.75f, 2.f, lowerPath) &&
		!lowerPath.empty();
	for (const SERVER_NAV_POINT& point : upperPath)
		layerPathsStayPut = layerPathsStayPut && std::abs(point.y - 10.f) < 0.000001f;
	for (const SERVER_NAV_POINT& point : lowerPath)
		layerPathsStayPut = layerPathsStayPut && std::abs(point.y - 2.f) < 0.000001f;
	tests.Require(
		layerPathsStayPut,
		"Path on the layer chosen by the start height and never hop to the other layer");
	SERVER_NAV_POINT projectedLower{};
	tests.Require(
		layersLoaded &&
		walkableLayerAt(layeredNavigation, 1.25f, 1.25f, 10.f) &&
		walkableLayerAt(layeredNavigation, 1.25f, 1.25f, 2.f) &&
		losLayerFrom(layeredNavigation, 1.25f, 1.25f, 2.25f, 1.25f, 2.f) &&
		projectLayerAt(layeredNavigation, 1.25f, 1.25f, 2.f, projectedLower) &&
		std::abs(projectedLower.y - 2.f) < 0.000001f,
		"Project, probe and sight-check on the layer chosen by the height hint");
	CServerNavigation reversedLayers;
	SERVER_NAV_POINT reversedUnhinted{};
	SERVER_NAV_POINT reversedHinted{};
	tests.Require(
		layersReady && loadLayerPair("lower", "upper", 0.75f, reversedLayers) &&
		reversedLayers.Sample_Position(1.25f, 1.25f, reversedUnhinted) &&
		std::abs(reversedUnhinted.y - 2.f) < 0.000001f &&
		sampleLayerAt(reversedLayers, 1.25f, 1.25f, 10.3f, reversedHinted) &&
		std::abs(reversedHinted.y - 10.f) < 0.000001f,
		"Keep manifest order as the only tie-break: reordering flips an unhinted query but never a hinted one");
	CServerNavigation holeLayers;
	SERVER_NAV_POINT holeFallback{};
	SERVER_NAV_POINT holeSolid{};
	tests.Require(
		layersReady && punchGroundHole(L"NAV_REGION_CONTRACT.upper", 0) &&
		loadLayerPair("upper", "lower", 0.75f, holeLayers) &&
		sampleLayerAt(holeLayers, 1.25f, 1.25f, 10.3f, holeFallback) &&
		std::abs(holeFallback.y - 2.f) < 0.000001f &&
		sampleLayerAt(holeLayers, 1.75f, 1.25f, 10.3f, holeSolid) &&
		std::abs(holeSolid.y - 10.f) < 0.000001f,
		"Prefer the layer that has ground at the XZ over a nearer layer that has none");
	const auto layersLoadWith = [&](
		const float upperHeight, const float lowerHeight, const float step)
	{
		CServerNavigation navigation;
		return writeLayer(
				L"NAV_REGION_CONTRACT.upper", "NAV_REGION_CONTRACT.upper",
				upperHeight, step) &&
			writeLayer(
				L"NAV_REGION_CONTRACT.lower", "NAV_REGION_CONTRACT.lower",
				lowerHeight, step) &&
			loadLayerPair("upper", "lower", step, navigation);
	};
	tests.Require(
		regionFixtureReady &&
		!layersLoadWith(3.f, 2.f, 0.75f) &&
		!layersLoadWith(3.9f, 2.f, 0.75f) &&
		layersLoadWith(4.f, 2.f, 0.75f),
		"Reject stacked layers closer than 2 m and accept them from exactly 2 m");
	tests.Require(
		regionFixtureReady &&
		!layersLoadWith(4.5f, 2.f, 1.5f) &&
		layersLoadWith(5.f, 2.f, 1.5f),
		"Scale the required layer separation to twice the larger step policy");
	bool blockerFixtureReady = regionFixtureReady && writeRegionManifest(
		"LOSTARK_NAVGRID_REGIONS 1 \"NAV_REGION_CONTRACT\" 1\n"
		"REGION \"fine\" 0.75\n");
	if (blockerFixtureReady)
	{
		std::ofstream regionBlockers(
			invalidPolicyRoot / L"Navigation" /
				L"NAV_REGION_CONTRACT.fine.navblockers",
			std::ios::binary | std::ios::trunc);
		regionBlockers <<
			"LOSTARK_NAVGRID_BLOCKERS 1 \"NAV_REGION_CONTRACT.fine\" "
			"4 4 0.5 1 1 1\n"
			"REGION \"contract.region.wall\" "
			"\"contract.region.wall.open\" 0 1\n"
			"1 1\n";
		regionBlockers.close();
		blockerFixtureReady = regionBlockers.good();
	}
	CServerNavigation blockedRegionNavigation;
	tests.Require(
		blockerFixtureReady &&
		!blockedRegionNavigation.Load("NAV_REGION_CONTRACT"),
		"Reject runtime blockers declared inside a detail region");
	std::error_code regionCleanupError;
	fs::remove(
		invalidPolicyRoot / L"Navigation" /
			L"NAV_REGION_CONTRACT.fine.navblockers",
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
	SetEnvironmentVariableW(
		L"LOSTARK_SERVER_DATA_ROOT", hadConfiguredRoot ? pathBuffer.data() : nullptr);
	fs::remove_all(invalidPolicyRoot, fixtureError);
	tests.Require(
		rejectedInvalidPolicyTransactionally,
		"Roll back Bern navigation when its runtime policy is invalid");

	std::cout << "navigation failures : " << tests.failures << '\n';
	return 0 == tests.failures ? 0 : 1;
}
