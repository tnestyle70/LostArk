#include "NavGridBaker.h"

#include "BinaryAsset/ModelAssetData.h"
#include "BinaryAsset/ModelDecoderRegistry.h"
#include "RuntimeAssetRoot.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>
#include <unordered_map>

namespace
{
	constexpr const char* SOURCE_MAGIC = "LOSTARK_NAVGRID_SOURCE";
	constexpr uint32_t SOURCE_VERSION = 2;
	constexpr f32_t MAP_MODEL_PRE_SCALE = 0.01f;
	constexpr f32_t GEOMETRY_EPSILON = 0.00001f;

	struct BAKE_GEOMETRY final
	{
		std::vector<float3_t> positions;
		std::vector<uint32_t> indices;
	};

	bool_t IsFinite(const float3_t& value)
	{
		return
			std::isfinite(value.x) &&
			std::isfinite(value.y) &&
			std::isfinite(value.z);
	}

	bool_t IsValidBakeDesc(const Client::NAVGRID_BAKE_DESC& desc)
	{
		return
			desc.isReady &&
			IsFinite(desc.position) &&
			IsFinite(desc.size) &&
			desc.size.x >= 0.1f &&
			desc.size.y >= 0.1f &&
			desc.size.z >= 0.1f &&
			std::isfinite(desc.yawDegrees) &&
			std::isfinite(desc.cellSize) &&
			desc.cellSize >= 0.05f &&
			desc.cellSize <= 10.f &&
			std::isfinite(desc.maxSlopeDegrees) &&
			desc.maxSlopeDegrees >= 0.f &&
			desc.maxSlopeDegrees < 90.f;
	}

	bool_t IsInsideBoundsXZ(
		const Client::NAVGRID_BAKE_DESC& desc,
		f32_t worldX,
		f32_t worldZ)
	{
		const f32_t radians = XMConvertToRadians(desc.yawDegrees);
		const f32_t cosine = std::cos(radians);
		const f32_t sine = std::sin(radians);
		const f32_t offsetX = worldX - desc.position.x;
		const f32_t offsetZ = worldZ - desc.position.z;
		const f32_t localX =
			cosine * offsetX - sine * offsetZ;
		const f32_t localZ =
			sine * offsetX + cosine * offsetZ;
		return
			std::abs(localX) <= desc.size.x * 0.5f + GEOMETRY_EPSILON &&
			std::abs(localZ) <= desc.size.z * 0.5f + GEOMETRY_EPSILON;
	}

	bool_t TryProjectHeight(
		const float3_t& first,
		const float3_t& second,
		const float3_t& third,
		f32_t worldX,
		f32_t worldZ,
		f32_t& outHeight)
	{
		const f32_t denominator =
			(second.z - third.z) * (first.x - third.x) +
			(third.x - second.x) * (first.z - third.z);
		if (std::abs(denominator) <= GEOMETRY_EPSILON)
			return false;

		const f32_t firstWeight =
			((second.z - third.z) * (worldX - third.x) +
				(third.x - second.x) * (worldZ - third.z)) /
			denominator;
		const f32_t secondWeight =
			((third.z - first.z) * (worldX - third.x) +
				(first.x - third.x) * (worldZ - third.z)) /
			denominator;
		const f32_t thirdWeight = 1.f - firstWeight - secondWeight;
		if (firstWeight < -GEOMETRY_EPSILON ||
			secondWeight < -GEOMETRY_EPSILON ||
			thirdWeight < -GEOMETRY_EPSILON)
		{
			return false;
		}

		outHeight =
			firstWeight * first.y +
			secondWeight * second.y +
			thirdWeight * third.y;
		return std::isfinite(outHeight);
	}

	bool_t DecodeGeometry(
		const std::filesystem::path& modelPath,
		BAKE_GEOMETRY& outGeometry,
		std::string& outStatus)
	{
		MODEL_ASSET_LOAD_DESC loadDesc{};
		loadDesc.assetRoot = Client::CRuntimeAssetRoot::Get();
		loadDesc.meshPath = modelPath;

		MODEL_ASSET_DATA asset{};
		if (!CModelDecoderRegistry::Get().Decode(loadDesc, asset))
		{
			const MODEL_DECODE_REPORT report =
				CModelDecoderRegistry::Get().Get_LastReport();
			outStatus =
				"Failed to decode bake mesh: " +
				modelPath.string();
			if (!report.error.empty())
				outStatus += " (" + report.error + ")";
			return false;
		}

		BAKE_GEOMETRY staged;
		for (const MODEL_MESH_DATA& mesh : asset.meshes)
		{
			if (MODEL_VERTEX_KIND::STATIC != mesh.vertexKind)
				continue;
			if (mesh.indices.size() % 3 != 0)
			{
				outStatus =
					"Bake mesh index count is not triangular: " +
					modelPath.string();
				return false;
			}

			const uint64_t newVertexCount =
				static_cast<uint64_t>(staged.positions.size()) +
				mesh.vertices.size();
			if (newVertexCount >
				(static_cast<uint64_t>(
					(std::numeric_limits<uint32_t>::max)())))
			{
				outStatus =
					"Bake mesh has too many vertices: " +
					modelPath.string();
				return false;
			}

			const uint32_t baseVertex =
				static_cast<uint32_t>(staged.positions.size());
			staged.positions.reserve(
				static_cast<size_t>(newVertexCount));
			for (const VTXMESH& vertex : mesh.vertices)
			{
				const float3_t position(
					vertex.vPosition.x * MAP_MODEL_PRE_SCALE,
					vertex.vPosition.y * MAP_MODEL_PRE_SCALE,
					vertex.vPosition.z * MAP_MODEL_PRE_SCALE);
				if (!IsFinite(position))
				{
					outStatus =
						"Bake mesh contains a non-finite vertex: " +
						modelPath.string();
					return false;
				}
				staged.positions.push_back(position);
			}

			staged.indices.reserve(
				staged.indices.size() + mesh.indices.size());
			for (uint32_t index : mesh.indices)
			{
				if (index >= mesh.vertices.size())
				{
					outStatus =
						"Bake mesh contains an invalid index: " +
						modelPath.string();
					return false;
				}
				staged.indices.push_back(baseVertex + index);
			}
		}

		if (staged.positions.empty() || staged.indices.empty())
		{
			outStatus =
				"Bake mesh has no static triangles: " +
				modelPath.string();
			return false;
		}

		outGeometry = std::move(staged);
		return true;
	}

	bool_t CommitTemporaryFile(
		const std::filesystem::path& destination,
		const std::filesystem::path& temporary)
	{
		std::error_code error;
		const bool_t destinationExists =
			std::filesystem::exists(destination, error);
		if (error)
			return false;

		if (destinationExists &&
			ReplaceFileW(
				destination.c_str(),
				temporary.c_str(),
				nullptr,
				REPLACEFILE_WRITE_THROUGH,
				nullptr,
				nullptr))
		{
			return true;
		}

		return MoveFileExW(
			temporary.c_str(),
			destination.c_str(),
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
	}
}

bool_t Client::CNavGridBaker::Build_Desc(
	const std::string& areaId,
	const NAVGRID_BAKE_DESC& bakeDesc,
	NAVGRID_AUTHORING_DESC& outDesc,
	std::string& outStatus)
{
	if (areaId.empty() || !IsValidBakeDesc(bakeDesc))
	{
		outStatus = "Nav Bounds or bake settings are invalid";
		return false;
	}

	const f32_t radians = XMConvertToRadians(bakeDesc.yawDegrees);
	const f32_t cosine = std::abs(std::cos(radians));
	const f32_t sine = std::abs(std::sin(radians));
	const f32_t halfExtentX =
		cosine * bakeDesc.size.x * 0.5f +
		sine * bakeDesc.size.z * 0.5f;
	const f32_t halfExtentZ =
		sine * bakeDesc.size.x * 0.5f +
		cosine * bakeDesc.size.z * 0.5f;
	const f32_t widthValue =
		std::ceil((halfExtentX * 2.f) / bakeDesc.cellSize);
	const f32_t heightValue =
		std::ceil((halfExtentZ * 2.f) / bakeDesc.cellSize);
	if (!std::isfinite(widthValue) ||
		!std::isfinite(heightValue) ||
		widthValue < 1.f ||
		heightValue < 1.f ||
		widthValue >
			static_cast<f32_t>((std::numeric_limits<uint32_t>::max)()) ||
		heightValue >
			static_cast<f32_t>((std::numeric_limits<uint32_t>::max)()))
	{
		outStatus = "Nav Bounds produce an invalid grid size";
		return false;
	}

	NAVGRID_AUTHORING_DESC staged;
	staged.areaId = areaId;
	staged.width = static_cast<uint32_t>(widthValue);
	staged.height = static_cast<uint32_t>(heightValue);
	staged.cellSize = bakeDesc.cellSize;
	staged.originX =
		bakeDesc.position.x -
		static_cast<f32_t>(staged.width) * staged.cellSize * 0.5f;
	staged.originZ =
		bakeDesc.position.z -
		static_cast<f32_t>(staged.height) * staged.cellSize * 0.5f;
	staged.bake = bakeDesc;

	const uint64_t cellCount =
		static_cast<uint64_t>(staged.width) * staged.height;
	if (0 == cellCount ||
		cellCount > CNavGridPaintDocument::MAX_CELL_COUNT)
	{
		outStatus =
			"Nav Bounds exceed the one-million-cell safety limit";
		return false;
	}

	outDesc = std::move(staged);
	return true;
}

bool_t Client::CNavGridBaker::Build(
	const std::string& areaId,
	const NAVGRID_BAKE_DESC& bakeDesc,
	const std::vector<NAVGRID_BAKE_PLACEMENT>& placements,
	NAVGRID_BAKE_RESULT& outResult,
	std::string& outStatus)
{
	NAVGRID_BAKE_RESULT staged;
	if (!Build_Desc(areaId, bakeDesc, staged.desc, outStatus))
		return false;
	if (placements.empty())
	{
		outStatus = "No static map meshes overlap Nav Bounds";
		return false;
	}

	const uint32_t cellCount =
		staged.desc.width * staged.desc.height;
	staged.cells.assign(cellCount, NAV_SOURCE_CELL{});
	staged.placementCount =
		static_cast<uint32_t>(placements.size());

	std::unordered_map<std::string, BAKE_GEOMETRY> geometryCache;
	const f32_t minimumY =
		bakeDesc.position.y - bakeDesc.size.y * 0.5f;
	const f32_t maximumY =
		bakeDesc.position.y + bakeDesc.size.y * 0.5f;
	const f32_t minimumWalkableNormalY =
		std::cos(XMConvertToRadians(bakeDesc.maxSlopeDegrees));

	for (const NAVGRID_BAKE_PLACEMENT& placement : placements)
	{
		auto geometry = geometryCache.find(placement.assetId);
		if (geometry == geometryCache.end())
		{
			BAKE_GEOMETRY decoded;
			if (!DecodeGeometry(
				placement.modelPath,
				decoded,
				outStatus))
			{
				return false;
			}
			geometry = geometryCache.emplace(
				placement.assetId,
				std::move(decoded)).first;
		}

		const BAKE_GEOMETRY& mesh = geometry->second;
		const matrix_t world = XMLoadFloat4x4(&placement.world);
		for (size_t triangle = 0;
			triangle < mesh.indices.size();
			triangle += 3)
		{
			const uint32_t firstIndex = mesh.indices[triangle];
			const uint32_t secondIndex = mesh.indices[triangle + 1];
			const uint32_t thirdIndex = mesh.indices[triangle + 2];
			if (firstIndex >= mesh.positions.size() ||
				secondIndex >= mesh.positions.size() ||
				thirdIndex >= mesh.positions.size())
			{
				outStatus =
					"Decoded bake mesh contains an invalid triangle";
				return false;
			}

			float3_t first{};
			float3_t second{};
			float3_t third{};
			XMStoreFloat3(
				&first,
				XMVector3TransformCoord(
					XMLoadFloat3(&mesh.positions[firstIndex]),
					world));
			XMStoreFloat3(
				&second,
				XMVector3TransformCoord(
					XMLoadFloat3(&mesh.positions[secondIndex]),
					world));
			XMStoreFloat3(
				&third,
				XMVector3TransformCoord(
					XMLoadFloat3(&mesh.positions[thirdIndex]),
					world));
			if (!IsFinite(first) ||
				!IsFinite(second) ||
				!IsFinite(third))
			{
				outStatus =
					"World-space bake triangle is non-finite";
				return false;
			}
			++staged.triangleCount;

			const vector_t edgeA =
				XMLoadFloat3(&second) - XMLoadFloat3(&first);
			const vector_t edgeB =
				XMLoadFloat3(&third) - XMLoadFloat3(&first);
			const vector_t normal = XMVector3Cross(edgeA, edgeB);
			const f32_t normalLength =
				XMVectorGetX(XMVector3Length(normal));
			if (!std::isfinite(normalLength) ||
				normalLength <= GEOMETRY_EPSILON)
			{
				continue;
			}
			const f32_t normalY =
				std::abs(XMVectorGetY(normal) / normalLength);
			const bool_t triangleWalkable =
				normalY + GEOMETRY_EPSILON >=
				minimumWalkableNormalY;

			const f32_t triangleMinimumX =
				(std::min)({ first.x, second.x, third.x });
			const f32_t triangleMaximumX =
				(std::max)({ first.x, second.x, third.x });
			const f32_t triangleMinimumZ =
				(std::min)({ first.z, second.z, third.z });
			const f32_t triangleMaximumZ =
				(std::max)({ first.z, second.z, third.z });
			int32_t minimumCellX = static_cast<int32_t>(std::floor(
				(triangleMinimumX - staged.desc.originX) /
				staged.desc.cellSize));
			int32_t maximumCellX = static_cast<int32_t>(std::floor(
				(triangleMaximumX - staged.desc.originX) /
				staged.desc.cellSize));
			int32_t minimumCellZ = static_cast<int32_t>(std::floor(
				(triangleMinimumZ - staged.desc.originZ) /
				staged.desc.cellSize));
			int32_t maximumCellZ = static_cast<int32_t>(std::floor(
				(triangleMaximumZ - staged.desc.originZ) /
				staged.desc.cellSize));
			minimumCellX = (std::max)(minimumCellX, 0);
			minimumCellZ = (std::max)(minimumCellZ, 0);
			maximumCellX = (std::min)(
				maximumCellX,
				static_cast<int32_t>(staged.desc.width) - 1);
			maximumCellZ = (std::min)(
				maximumCellZ,
				static_cast<int32_t>(staged.desc.height) - 1);
			if (minimumCellX > maximumCellX ||
				minimumCellZ > maximumCellZ)
			{
				continue;
			}

			for (int32_t cellZ = minimumCellZ;
				cellZ <= maximumCellZ;
				++cellZ)
			{
				for (int32_t cellX = minimumCellX;
					cellX <= maximumCellX;
					++cellX)
				{
					const f32_t worldX =
						staged.desc.originX +
						(static_cast<f32_t>(cellX) + 0.5f) *
						staged.desc.cellSize;
					const f32_t worldZ =
						staged.desc.originZ +
						(static_cast<f32_t>(cellZ) + 0.5f) *
						staged.desc.cellSize;
					if (!IsInsideBoundsXZ(
						bakeDesc,
						worldX,
						worldZ))
					{
						continue;
					}

					f32_t surfaceHeight = {};
					if (!TryProjectHeight(
						first,
						second,
						third,
						worldX,
						worldZ,
						surfaceHeight) ||
						surfaceHeight <
							minimumY - GEOMETRY_EPSILON ||
						surfaceHeight >
							maximumY + GEOMETRY_EPSILON)
					{
						continue;
					}

					const uint32_t cellIndex =
						static_cast<uint32_t>(cellZ) *
						staged.desc.width +
						static_cast<uint32_t>(cellX);
					NAV_SOURCE_CELL& cell =
						staged.cells[cellIndex];
					if (!cell.surfaceResolved ||
						surfaceHeight > cell.height)
					{
						cell.surfaceResolved = true;
						cell.baseWalkable = triangleWalkable;
						cell.height = surfaceHeight;
					}
				}
			}
		}
	}

	for (const NAV_SOURCE_CELL& cell : staged.cells)
	{
		if (cell.surfaceResolved)
			++staged.resolvedCellCount;
	}
	if (0 == staged.resolvedCellCount)
	{
		outStatus =
			"No mesh surface was resolved inside Nav Bounds";
		return false;
	}

	outResult = std::move(staged);
	outStatus =
		"Baked " +
		std::to_string(outResult.resolvedCellCount) +
		" surface cells from " +
		std::to_string(outResult.placementCount) +
		" placements";
	return true;
}

bool_t Client::CNavGridBaker::Save_Source(
	const NAVGRID_BAKE_RESULT& result,
	const std::filesystem::path& path,
	std::string& outStatus)
{
	const uint64_t cellCount =
		static_cast<uint64_t>(result.desc.width) *
		result.desc.height;
	if (result.desc.areaId.empty() ||
		0 == cellCount ||
		cellCount != result.cells.size() ||
		cellCount > CNavGridPaintDocument::MAX_CELL_COUNT ||
		!IsValidBakeDesc(result.desc.bake))
	{
		outStatus = "Baked NavGrid source is invalid";
		return false;
	}
	for (const NAV_SOURCE_CELL& cell : result.cells)
	{
		if ((cell.baseWalkable && !cell.surfaceResolved) ||
			!std::isfinite(cell.height))
		{
			outStatus = "Baked NavGrid cell data is invalid";
			return false;
		}
	}

	std::error_code directoryError;
	std::filesystem::create_directories(
		path.parent_path(),
		directoryError);
	if (directoryError)
	{
		outStatus = "Could not create NavGrid source directory";
		return false;
	}

	std::filesystem::path temporary = path;
	temporary += L".tmp";
	std::ofstream output(
		temporary,
		std::ios::binary | std::ios::trunc);
	if (!output)
	{
		outStatus = "Could not create temporary NavGrid source";
		return false;
	}

	output <<
		SOURCE_MAGIC << ' ' <<
		SOURCE_VERSION << ' ' <<
		std::quoted(result.desc.areaId) << ' ' <<
		result.desc.width << ' ' <<
		result.desc.height << ' ' <<
		std::setprecision(9) <<
		result.desc.cellSize << ' ' <<
		result.desc.originX << ' ' <<
		result.desc.originZ << ' ' <<
		result.desc.bake.position.x << ' ' <<
		result.desc.bake.position.y << ' ' <<
		result.desc.bake.position.z << ' ' <<
		result.desc.bake.size.x << ' ' <<
		result.desc.bake.size.y << ' ' <<
		result.desc.bake.size.z << ' ' <<
		result.desc.bake.yawDegrees << ' ' <<
		result.desc.bake.maxSlopeDegrees << ' ' <<
		(result.desc.bake.isReady ? 1 : 0) << ' ' <<
		cellCount << '\n';

	for (uint32_t cellZ = 0;
		cellZ < result.desc.height;
		++cellZ)
	{
		for (uint32_t cellX = 0;
			cellX < result.desc.width;
			++cellX)
		{
			const NAV_SOURCE_CELL& cell =
				result.cells[
					cellZ * result.desc.width + cellX];
			output <<
				cellX << ' ' <<
				cellZ << ' ' <<
				(cell.surfaceResolved ? 1 : 0) << ' ' <<
				(cell.baseWalkable ? 1 : 0) << ' ' <<
				(cell.surfaceResolved ? cell.height : 0.f) <<
				'\n';
		}
	}

	output.flush();
	const bool_t wroteSuccessfully = output.good();
	output.close();
	if (!wroteSuccessfully ||
		!CommitTemporaryFile(path, temporary))
	{
		std::error_code removeError;
		std::filesystem::remove(temporary, removeError);
		outStatus = "Failed to commit NavGrid source atomically";
		return false;
	}

	outStatus =
		"Saved baked NavGrid source: " +
		path.string();
	return true;
}
