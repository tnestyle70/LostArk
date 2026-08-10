#include "BinaryAsset/ModelAssetData.h"
#include "BinaryAsset/ModelDecoderRegistry.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>

namespace
{
	using namespace Engine;

	constexpr uint32_t kStaticBase = 0x0fu;
	constexpr uint32_t kTangentHandedness = 1u << 5;
	constexpr uint32_t kColor0 = 1u << 6;
	constexpr uint32_t kProductSourceFidelity =
		MODEL_GEOMETRY_CLEAN_SOURCE_EXPORT |
		MODEL_GEOMETRY_UPK_TO_GLTF_EXACT |
		MODEL_GEOMETRY_PIVOT_EXACT;

	bool Nearly_Equal(float left, float right, float tolerance = 1e-5f)
	{
		const float scale = (std::max)(1.f, (std::max)(std::fabs(left), std::fabs(right)));
		return std::fabs(left - right) <= tolerance * scale;
	}

	template <size_t Size>
	bool Has_NonZero_Digest(const std::array<uint8_t, Size>& digest)
	{
		for (const uint8_t value : digest)
		{
			if (0 != value)
				return true;
		}
		return false;
	}

	bool Decode_Desc(
		const MODEL_ASSET_LOAD_DESC& desc,
		MODEL_ASSET_DATA& outAsset,
		MODEL_DECODE_REPORT& outReport)
	{
		const bool succeeded = CModelDecoderRegistry::Get().Decode(desc, outAsset);
		outReport = CModelDecoderRegistry::Get().Get_LastReport();
		return succeeded;
	}

	bool Decode(
		const std::filesystem::path& path,
		MODEL_ASSET_DATA& outAsset,
		MODEL_DECODE_REPORT& outReport)
	{
		MODEL_ASSET_LOAD_DESC desc{};
		desc.assetRoot = path.parent_path();
		desc.meshPath = path;
		return Decode_Desc(desc, outAsset, outReport);
	}

	bool Is_Default_Asset(const MODEL_ASSET_DATA& asset)
	{
		return asset.meshes.empty() && asset.materials.empty() &&
			!asset.geometryMetadata.present && !asset.hasSkeleton &&
			asset.skeleton.bones.empty() && asset.skeleton.sockets.empty() &&
			asset.animations.empty();
	}

	bool Validate_Common_V11(
		const MODEL_ASSET_DATA& asset,
		const MODEL_DECODE_REPORT& report)
	{
		if (!report.succeeded || !report.hasGeometryMetadata ||
			!asset.geometryMetadata.present ||
			1 != asset.geometryMetadata.versionMajor ||
			1 != asset.geometryMetadata.versionMinor ||
			!Nearly_Equal(asset.geometryMetadata.sourceToWModelScale, 100.f) ||
			!Nearly_Equal(asset.geometryMetadata.geometryPreScale, 0.01f) ||
			!Has_NonZero_Digest(asset.geometryMetadata.payloadSha256) ||
			!Has_NonZero_Digest(asset.geometryMetadata.metadataIdentitySha256) ||
			0 != (asset.geometryMetadata.evidenceFlags & kProductSourceFidelity) ||
			asset.meshes.size() != 1 || asset.meshes.front().vertices.size() != 3 ||
			asset.meshes.front().indices != std::vector<uint32_t>({ 0, 2, 1 }) ||
			!asset.meshes.front().embeddedBounds.present)
		{
			return false;
		}

		const MODEL_MESH_BOUNDS_DATA& bounds = asset.meshes.front().embeddedBounds;
		return Nearly_Equal(bounds.minimum.x, 100.f) &&
			Nearly_Equal(bounds.minimum.y, 200.f) &&
			Nearly_Equal(bounds.minimum.z, -800.f) &&
			Nearly_Equal(bounds.maximum.x, 400.f) &&
			Nearly_Equal(bounds.maximum.y, 600.f) &&
			Nearly_Equal(bounds.maximum.z, -300.f) &&
			Nearly_Equal(bounds.center.x, 250.f) &&
			Nearly_Equal(bounds.center.y, 400.f) &&
			Nearly_Equal(bounds.center.z, -550.f);
	}

	bool Validate_Color_V11(const std::filesystem::path& path)
	{
		MODEL_ASSET_DATA asset{};
		MODEL_DECODE_REPORT report{};
		if (!Decode(path, asset, report) || !Validate_Common_V11(asset, report))
			return false;

		const MODEL_MESH_DATA& mesh = asset.meshes.front();
		const uint32_t expectedMask = kStaticBase | kTangentHandedness | kColor0;
		if (!mesh.hasColor0 || asset.geometryMetadata.channelMask != expectedMask ||
			0 == (asset.geometryMetadata.evidenceFlags &
				MODEL_GEOMETRY_COLOR0_PRESERVED_FROM_GLTF))
		{
			return false;
		}

		const std::array<float, 3> expectedW = { -1.f, 1.f, -1.f };
		const std::array<float, 3> expectedBinormalZ = { 1.f, -1.f, 1.f };
		const std::array<uint32_t, 3> expectedColor = {
			0x44332211u, 0x88776655u, 0xccbbaa99u
		};
		if (mesh.tangentHandedness.size() != mesh.vertices.size() ||
			mesh.color0Rgba8.size() != mesh.vertices.size())
			return false;
		for (size_t i = 0; i < mesh.vertices.size(); ++i)
		{
			const VTXMESH& vertex = mesh.vertices[i];
			if (!Nearly_Equal(mesh.tangentHandedness[i], expectedW[i]) ||
				!Nearly_Equal(vertex.vBinormal.z, expectedBinormalZ[i]) ||
				mesh.color0Rgba8[i] != expectedColor[i])
			{
				return false;
			}
		}
		return true;
	}

	bool Validate_NoColor_V11(const std::filesystem::path& path)
	{
		MODEL_ASSET_DATA asset{};
		MODEL_DECODE_REPORT report{};
		if (!Decode(path, asset, report) || !Validate_Common_V11(asset, report))
			return false;

		const MODEL_MESH_DATA& mesh = asset.meshes.front();
		return !mesh.hasColor0 &&
			mesh.tangentHandedness.size() == mesh.vertices.size() &&
			mesh.color0Rgba8.empty() &&
			asset.geometryMetadata.channelMask == (kStaticBase | kTangentHandedness) &&
			0 == (asset.geometryMetadata.evidenceFlags &
				MODEL_GEOMETRY_COLOR0_PRESERVED_FROM_GLTF);
	}

	bool Validate_Legacy_Metadata_Absent(const std::filesystem::path& path)
	{
		MODEL_ASSET_DATA asset{};
		MODEL_DECODE_REPORT report{};
		if (!Decode(path, asset, report))
			return false;
		return report.succeeded && !report.hasGeometryMetadata &&
			!asset.geometryMetadata.present &&
			asset.meshes.size() == 1 &&
			!asset.meshes.front().embeddedBounds.present &&
			!asset.meshes.front().hasColor0 &&
			asset.meshes.front().tangentHandedness.empty() &&
			asset.meshes.front().color0Rgba8.empty();
	}

	bool Validate_Legacy_Static_Multisubmesh(const std::filesystem::path& path)
	{
		MODEL_ASSET_DATA asset{};
		MODEL_DECODE_REPORT report{};
		if (!Decode(path, asset, report))
			return false;
		return report.succeeded && !report.hasGeometryMetadata &&
			asset.meshes.size() == 2 && asset.materials.size() == 2 &&
			std::all_of(asset.meshes.begin(), asset.meshes.end(),
				[](const MODEL_MESH_DATA& mesh)
				{
					return mesh.vertices.size() == 3 && mesh.indices.size() == 3 &&
						!mesh.embeddedBounds.present && !mesh.hasColor0 &&
						mesh.tangentHandedness.empty() && mesh.color0Rgba8.empty();
				});
	}

	bool Validate_Legacy_Skinned(const std::filesystem::path& root)
	{
		MODEL_ASSET_LOAD_DESC desc{};
		desc.assetRoot = root;
		desc.meshPath = root / L"legacy_skinned.wmesh";
		desc.skeletonPath = root / L"legacy_skinned.wskel";
		desc.animationPaths.push_back(root / L"legacy_skinned.wanim");
		MODEL_ASSET_DATA asset{};
		MODEL_DECODE_REPORT report{};
		if (!Decode_Desc(desc, asset, report))
			return false;
		return report.succeeded && asset.hasSkeleton &&
			asset.skeleton.bones.size() == 1 && asset.animations.size() == 1 &&
			asset.meshes.size() == 1 &&
			asset.meshes.front().skinnedVertices.size() == 3 &&
			asset.meshes.front().vertices.empty() &&
			asset.meshes.front().tangentHandedness.empty() &&
			asset.meshes.front().color0Rgba8.empty();
	}

	bool Reject_Corrupt_Dependency(
		const std::filesystem::path& root,
		bool corruptSkeleton)
	{
		MODEL_ASSET_LOAD_DESC desc{};
		desc.assetRoot = root;
		desc.meshPath = root / L"legacy_skinned.wmesh";
		desc.skeletonPath = root /
			(corruptSkeleton ? L"corrupt_skeleton.wskel" : L"legacy_skinned.wskel");
		if (!corruptSkeleton)
			desc.animationPaths.push_back(root / L"corrupt_animation.wanim");
		MODEL_ASSET_DATA asset{};
		MODEL_DECODE_REPORT report{};
		return !Decode_Desc(desc, asset, report) && !report.succeeded &&
			!report.error.empty() && Is_Default_Asset(asset);
	}

	bool Validate_Artist_Candidate(const std::filesystem::path& path)
	{
		MODEL_ASSET_DATA asset{};
		MODEL_DECODE_REPORT report{};
		if (!Decode(path, asset, report) || !report.succeeded ||
			!report.hasGeometryMetadata || !asset.geometryMetadata.present ||
			!Nearly_Equal(asset.geometryMetadata.sourceToWModelScale, 100.f) ||
			!Nearly_Equal(asset.geometryMetadata.geometryPreScale, 0.01f) ||
			0 != (asset.geometryMetadata.evidenceFlags & kProductSourceFidelity) ||
			asset.meshes.empty())
			return false;

		for (const MODEL_MESH_DATA& mesh : asset.meshes)
		{
			if (mesh.vertices.empty() || mesh.indices.empty() ||
				0 != mesh.indices.size() % 3 || !mesh.embeddedBounds.present ||
				mesh.tangentHandedness.size() != mesh.vertices.size() ||
				(mesh.hasColor0 != !mesh.color0Rgba8.empty()) ||
				(mesh.hasColor0 && mesh.color0Rgba8.size() != mesh.vertices.size()))
				return false;
		}
		return true;
	}

	bool Reject_Corrupt(const std::filesystem::path& path)
	{
		MODEL_ASSET_DATA asset{};
		MODEL_DECODE_REPORT report{};
		return !Decode(path, asset, report) && !report.succeeded &&
			!report.error.empty() && Is_Default_Asset(asset);
	}
}

int wmain(int argc, wchar_t** argv)
{
	if (3 == argc && std::wstring_view(argv[1]) == L"--candidate")
	{
		const std::filesystem::path candidate =
			std::filesystem::absolute(argv[2]).lexically_normal();
		const bool valid = Validate_Artist_Candidate(candidate);
		std::cout << "WModel Artist candidate decoder gate: selfConsistentUnauthenticated="
			<< valid << " externallyAuthenticated=0 product=false\n";
		return valid ? 0 : 1;
	}
	if (2 != argc)
	{
		std::wcerr << L"usage: WModelGeometryContractHarness <suite-directory> "
			L"| --candidate <wmodel>\n";
		return 2;
	}

	const std::filesystem::path root =
		std::filesystem::absolute(argv[1]).lexically_normal();
	const bool validColor = Validate_Color_V11(root / L"valid_color.wmodel");
	const bool validNoColor = Validate_NoColor_V11(root / L"valid_no_color.wmodel");
	const bool legacyAbsent = Validate_Legacy_Metadata_Absent(root / L"legacy_v10.wmodel");
	const bool legacyStatic = Validate_Legacy_Static_Multisubmesh(
		root / L"legacy_static_multisubmesh_bounds.wmesh");
	const bool legacySkinned = Validate_Legacy_Skinned(root);
	const bool corruptSkeletonTransactional = Reject_Corrupt_Dependency(root, true);
	const bool corruptAnimationTransactional = Reject_Corrupt_Dependency(root, false);
	const std::array<const wchar_t*, 27> corruptFiles = {
		L"corrupt_truncated.wmodel",
		L"corrupt_header.wmodel",
		L"corrupt_outer_version.wmodel",
		L"corrupt_outer_metadata.wmodel",
		L"corrupt_section_gap.wmodel",
		L"corrupt_mesh_version.wmodel",
		L"corrupt_material_container.wmodel",
		L"corrupt_stride.wmodel",
		L"corrupt_material_index.wmodel",
		L"corrupt_channel.wmodel",
		L"corrupt_unknown_channel.wmodel",
		L"corrupt_tangent_w.wmodel",
		L"corrupt_zero_normal.wmodel",
		L"corrupt_zero_tangent.wmodel",
		L"corrupt_parallel_basis.wmodel",
		L"corrupt_scale_overflow_pair.wmodel",
		L"corrupt_scale_wrong_reciprocal.wmodel",
		L"corrupt_payload_hash.wmodel",
		L"corrupt_metadata_hash.wmodel",
		L"corrupt_bounds.wmodel",
		L"corrupt_bounds_nonfinite.wmodel",
		L"corrupt_bounds_inverted.wmodel",
		L"corrupt_bounds_float_intermediate.wmodel",
		L"corrupt_metadata_version.wmodel",
		L"corrupt_provenance.wmodel",
		L"corrupt_evidence.wmodel",
		L"corrupt_unverified_source_fidelity.wmodel",
	};
	bool corruptRejected = true;
	for (const wchar_t* file : corruptFiles)
		corruptRejected = Reject_Corrupt(root / file) && corruptRejected;

	std::cout << "WModel geometry decoder contract: validColor=" << validColor
		<< " validNoColor=" << validNoColor
		<< " legacyMetadataAbsent=" << legacyAbsent
		<< " legacyStaticMultiBounds=" << legacyStatic
		<< " legacySkinned=" << legacySkinned
		<< " corruptSkeletonTransactional=" << corruptSkeletonTransactional
		<< " corruptAnimationTransactional=" << corruptAnimationTransactional
		<< " corruptRejected=" << corruptRejected
		<< " selfConsistentUnauthenticated=true externallyAuthenticated=0 "
		<< "preScaleConsumed=false product=false\n";
	return validColor && validNoColor && legacyAbsent && legacyStatic &&
		legacySkinned && corruptSkeletonTransactional &&
		corruptAnimationTransactional && corruptRejected ? 0 : 1;
}
