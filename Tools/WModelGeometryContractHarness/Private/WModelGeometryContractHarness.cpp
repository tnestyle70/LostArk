#include "BinaryAsset/ModelAssetData.h"
#include "BinaryAsset/ModelDecoderRegistry.h"

#include <bcrypt.h>
#include <algorithm>
#include <array>
#include <bit>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <cwctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <process.h>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#pragma comment(lib, "bcrypt.lib")

namespace
{
	using namespace Engine;

	constexpr uint32_t kStaticBase = 0x0fu;
	constexpr uint32_t kTangentHandedness = 1u << 5;
	constexpr uint32_t kColor0 = 1u << 6;
	constexpr float kTangentHandednessAbsoluteTolerance = 1e-6f;
	constexpr uint32_t kExpectedLegacyResourceCorpusCount = 2586u;
	constexpr uint32_t kExpectedLegacyStaticCount = 2535u;
	constexpr uint32_t kExpectedLegacySkinnedCount = 51u;
	constexpr uint32_t kExpectedLegacyBoundsCount = 2586u;
	constexpr uint32_t kExpectedLegacyMultiSubmeshCount = 665u;
	constexpr size_t kWriterIndependentGoldenByteCount = 850u;
	constexpr std::string_view kWriterIndependentGoldenSha256 =
		"6bb409094185d9c41f6cb241d42bdc767b0a3868f7ed981d194e8fe1ccd23627";
	constexpr std::string_view kWriterIndependentManifestSha256 =
		"73b1753200514f534baf622d1732623f70842afddc4c83da551faac5df19ccdd";
	constexpr std::string_view kWriterIndependentPayloadSha256 =
		"c50feb075b6ef7509238f4cdabe0a247ad44036c31a0664b923d118f2c828dfe";
	constexpr std::string_view kWriterIndependentMetadataIdentitySha256 =
		"e99ee5ce041cc1f2d050f9737bb81e6acebe56c02370f7022418914144164f6a";
	constexpr uint32_t kProductSourceFidelity =
		MODEL_GEOMETRY_CLEAN_SOURCE_EXPORT |
		MODEL_GEOMETRY_UPK_TO_GLTF_EXACT |
		MODEL_GEOMETRY_PIVOT_EXACT;

	bool Nearly_Equal(float left, float right, float tolerance = 1e-5f)
	{
		const float scale = (std::max)(1.f, (std::max)(std::fabs(left), std::fabs(right)));
		return std::fabs(left - right) <= tolerance * scale;
	}

	bool Is_Valid_Tangent_Handedness(float value)
	{
		return std::isfinite(value) &&
			std::fabs(std::fabs(value) - 1.f) <=
			kTangentHandednessAbsoluteTolerance;
	}

	std::string Hex_Bytes(const uint8_t* pBytes, size_t byteCount)
	{
		std::ostringstream stream;
		stream << std::hex << std::setfill('0');
		for (size_t i = 0; i < byteCount; ++i)
			stream << std::setw(2) << static_cast<uint32_t>(pBytes[i]);
		return stream.str();
	}

	template <size_t Size>
	std::string Hex_Digest(const std::array<uint8_t, Size>& digest)
	{
		return Hex_Bytes(digest.data(), digest.size());
	}

	void Append_U32_LE(std::vector<uint8_t>& bytes, uint32_t value)
	{
		bytes.push_back(static_cast<uint8_t>(value & 0xffu));
		bytes.push_back(static_cast<uint8_t>((value >> 8u) & 0xffu));
		bytes.push_back(static_cast<uint8_t>((value >> 16u) & 0xffu));
		bytes.push_back(static_cast<uint8_t>((value >> 24u) & 0xffu));
	}

	void Append_F32_LE(std::vector<uint8_t>& bytes, float value)
	{
		Append_U32_LE(bytes, std::bit_cast<uint32_t>(value));
	}

	std::string Sha256_Hex(const std::vector<uint8_t>& bytes)
	{
		BCRYPT_ALG_HANDLE algorithm = { nullptr };
		BCRYPT_HASH_HANDLE hash = { nullptr };
		DWORD objectSize = {};
		DWORD resultSize = {};
		std::vector<uint8_t> object;
		std::array<uint8_t, 32> digest{};
		bool succeeded = false;
		if (BCryptOpenAlgorithmProvider(
			&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0) < 0)
			goto cleanup;
		if (BCryptGetProperty(
			algorithm, BCRYPT_OBJECT_LENGTH,
			reinterpret_cast<PUCHAR>(&objectSize), sizeof(objectSize),
			&resultSize, 0) < 0 || sizeof(objectSize) != resultSize || 0 == objectSize)
			goto cleanup;
		object.resize(objectSize);
		if (BCryptCreateHash(
			algorithm, &hash, object.data(), objectSize,
			nullptr, 0, 0) < 0)
			goto cleanup;
		if (!bytes.empty() && (bytes.size() > ULONG_MAX || BCryptHashData(
			hash, const_cast<PUCHAR>(bytes.data()),
			static_cast<ULONG>(bytes.size()), 0) < 0))
			goto cleanup;
		if (BCryptFinishHash(
			hash, digest.data(), static_cast<ULONG>(digest.size()), 0) < 0)
			goto cleanup;
		succeeded = true;

	cleanup:
		if (nullptr != hash)
			BCryptDestroyHash(hash);
		if (nullptr != algorithm)
			BCryptCloseAlgorithmProvider(algorithm, 0);
		return succeeded ? Hex_Digest(digest) : std::string{};
	}

	bool Read_File_Bytes(
		const std::filesystem::path& path,
		std::vector<uint8_t>& outBytes)
	{
		std::ifstream stream(path, std::ios::binary | std::ios::ate);
		if (!stream)
			return false;
		const std::streamoff length = stream.tellg();
		if (length < 0 || static_cast<uint64_t>(length) > SIZE_MAX)
			return false;
		outBytes.resize(static_cast<size_t>(length));
		stream.seekg(0, std::ios::beg);
		return outBytes.empty() ||
			static_cast<bool>(stream.read(
				reinterpret_cast<char*>(outBytes.data()), length));
	}

	bool Canonicalize_Utf8_Lf(
		const std::vector<uint8_t>& source,
		std::vector<uint8_t>& outBytes)
	{
		if (source.size() >= 3u && source[0] == 0xefu &&
			source[1] == 0xbbu && source[2] == 0xbfu)
		{
			return false;
		}
		outBytes.clear();
		outBytes.reserve(source.size());
		for (size_t index = 0; index < source.size(); ++index)
		{
			if ('\r' == source[index])
			{
				if (index + 1u < source.size() && '\n' == source[index + 1u])
					++index;
				outBytes.push_back('\n');
			}
			else
			{
				outBytes.push_back(source[index]);
			}
		}
		return true;
	}

	int Hex_Nibble(uint8_t value)
	{
		if (value >= '0' && value <= '9')
			return value - '0';
		if (value >= 'a' && value <= 'f')
			return value - 'a' + 10;
		return -1;
	}

	bool Decode_Lowercase_Hex_File(
		const std::filesystem::path& path,
		std::vector<uint8_t>& outBytes)
	{
		std::vector<uint8_t> encoded;
		if (!Read_File_Bytes(path, encoded))
			return false;
		std::vector<uint8_t> nibbles;
		nibbles.reserve(encoded.size());
		for (const uint8_t value : encoded)
		{
			if (0 != std::isspace(static_cast<unsigned char>(value)))
				continue;
			if (Hex_Nibble(value) < 0)
				return false;
			nibbles.push_back(value);
		}
		if (nibbles.empty() || 0 != nibbles.size() % 2u)
			return false;
		outBytes.clear();
		outBytes.reserve(nibbles.size() / 2u);
		for (size_t index = 0; index < nibbles.size(); index += 2u)
		{
			outBytes.push_back(static_cast<uint8_t>(
				(Hex_Nibble(nibbles[index]) << 4) |
				Hex_Nibble(nibbles[index + 1u])));
		}
		return true;
	}

	std::string Json_Escape(const std::string& value)
	{
		std::ostringstream stream;
		for (const unsigned char character : value)
		{
			switch (character)
			{
			case '\\': stream << "\\\\"; break;
			case '"': stream << "\\\""; break;
			case '\b': stream << "\\b"; break;
			case '\f': stream << "\\f"; break;
			case '\n': stream << "\\n"; break;
			case '\r': stream << "\\r"; break;
			case '\t': stream << "\\t"; break;
			default:
				if (character < 0x20u)
				{
					stream << "\\u" << std::hex << std::setw(4) << std::setfill('0')
						<< static_cast<uint32_t>(character) << std::dec;
				}
				else
					stream << static_cast<char>(character);
				break;
			}
		}
		return stream.str();
	}

	std::string F32_Hex(float value)
	{
		std::ostringstream stream;
		stream << std::hex << std::setw(8) << std::setfill('0')
			<< std::bit_cast<uint32_t>(value);
		return stream.str();
	}

#pragma pack(push, 1)
	struct LEGACY_FILE_HEADER
	{
		char magic[4];
		uint16_t versionMajor;
		uint16_t versionMinor;
		uint32_t flags;
		uint32_t contentSize;
	};

	struct LEGACY_MODEL_HEADER
	{
		char magic[4];
		uint32_t sectionCount;
		uint32_t animationCount;
		uint32_t flags;
		uint32_t reserved[4];
	};

	struct LEGACY_SECTION_DESC
	{
		uint32_t type;
		uint32_t index;
		uint64_t offset;
		uint64_t size;
		char name[40];
	};

	struct LEGACY_MESH_HEADER
	{
		char magic[4];
		uint32_t submeshCount;
		uint32_t boneCount;
		uint32_t vertexFormatFlags;
		uint32_t vertexStride;
		uint32_t totalVertexCount;
		uint32_t totalIndexCount;
		uint32_t indexStride;
		uint8_t hasBounding;
		uint8_t reserved[3];
	};
#pragma pack(pop)

	static_assert(sizeof(LEGACY_FILE_HEADER) == 16);
	static_assert(sizeof(LEGACY_MODEL_HEADER) == 32);
	static_assert(sizeof(LEGACY_SECTION_DESC) == 64);
	static_assert(sizeof(LEGACY_MESH_HEADER) == 36);

	struct LEGACY_LAYOUT
	{
		uint32_t submeshCount = {};
		bool skinned = { false };
		bool hasBounds = { false };
	};

	template <typename Value>
	bool Read_At(const std::vector<uint8_t>& bytes, size_t offset, Value& outValue)
	{
		if (offset > bytes.size() || sizeof(Value) > bytes.size() - offset)
			return false;
		memcpy(&outValue, bytes.data() + offset, sizeof(Value));
		return true;
	}

	bool Has_Magic(const char* pValue, const char* pExpected)
	{
		return 0 == memcmp(pValue, pExpected, 4);
	}

	bool Inspect_Legacy_WModel(
		const std::filesystem::path& path,
		LEGACY_LAYOUT& outLayout)
	{
		std::ifstream stream(path, std::ios::binary | std::ios::ate);
		if (!stream)
			return false;
		const std::streamoff length = stream.tellg();
		if (length <= 0 || static_cast<uint64_t>(length) > SIZE_MAX)
			return false;
		std::vector<uint8_t> bytes(static_cast<size_t>(length));
		stream.seekg(0, std::ios::beg);
		if (!stream.read(reinterpret_cast<char*>(bytes.data()), length))
			return false;

		LEGACY_FILE_HEADER outer{};
		LEGACY_MODEL_HEADER model{};
		if (!Read_At(bytes, 0, outer) ||
			!Has_Magic(outer.magic, "WINT") || outer.versionMajor != 1 ||
			outer.versionMinor != 0 || outer.flags != 0 ||
			outer.contentSize != bytes.size() - sizeof(outer) ||
			!Read_At(bytes, sizeof(outer), model) ||
			!Has_Magic(model.magic, "WMOD") || model.sectionCount < 2 ||
			model.sectionCount > 4096)
			return false;

		const size_t tableOffset = sizeof(outer) + sizeof(model);
		if (model.sectionCount >
			(bytes.size() - (min)(bytes.size(), tableOffset)) / sizeof(LEGACY_SECTION_DESC))
			return false;
		uint32_t meshSections = {};
		LEGACY_SECTION_DESC meshSection{};
		for (uint32_t row = 0; row < model.sectionCount; ++row)
		{
			LEGACY_SECTION_DESC section{};
			if (!Read_At(bytes,
				tableOffset + static_cast<size_t>(row) * sizeof(section), section))
				return false;
			if (1 == section.type)
			{
				meshSection = section;
				++meshSections;
			}
		}
		if (1 != meshSections || meshSection.offset > outer.contentSize ||
			meshSection.size > outer.contentSize - meshSection.offset ||
			meshSection.size < sizeof(LEGACY_FILE_HEADER) + sizeof(LEGACY_MESH_HEADER))
			return false;

		const size_t meshOffset = sizeof(outer) + static_cast<size_t>(meshSection.offset);
		LEGACY_FILE_HEADER meshOuter{};
		LEGACY_MESH_HEADER mesh{};
		if (!Read_At(bytes, meshOffset, meshOuter) ||
			!Has_Magic(meshOuter.magic, "WINT") || meshOuter.versionMajor != 1 ||
			meshOuter.versionMinor != 0 || meshOuter.flags != 0 ||
			meshOuter.contentSize != meshSection.size - sizeof(meshOuter) ||
			!Read_At(bytes, meshOffset + sizeof(meshOuter), mesh) ||
			!Has_Magic(mesh.magic, "WMSH") || 0 == mesh.submeshCount ||
			mesh.hasBounding > 1)
			return false;

		outLayout.submeshCount = mesh.submeshCount;
		outLayout.skinned = 0 != (mesh.vertexFormatFlags & (1u << 4));
		outLayout.hasBounds = 0 != mesh.hasBounding;
		return true;
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
		const std::array<float3_t, 3> expectedPosition = {
			float3_t{ 100.f, 200.f, -300.f },
			float3_t{ 400.f, 200.f, -300.f },
			float3_t{ 100.f, 600.f, -800.f },
		};
		const std::array<float2_t, 3> expectedUv0 = {
			float2_t{ 0.f, 0.f },
			float2_t{ 1.f, 0.f },
			float2_t{ 0.f, 1.f },
		};
		const std::array<uint32_t, 3> expectedColor = {
			0x44332211u, 0x88776655u, 0xccbbaa99u
		};
		if (mesh.tangentHandedness.size() != mesh.vertices.size() ||
			mesh.color0Rgba8.size() != mesh.vertices.size() ||
			mesh.name != "fixture" || 0 != mesh.materialIndex)
			return false;
		for (size_t i = 0; i < mesh.vertices.size(); ++i)
		{
			const VTXMESH& vertex = mesh.vertices[i];
			if (!Nearly_Equal(vertex.vPosition.x, expectedPosition[i].x) ||
				!Nearly_Equal(vertex.vPosition.y, expectedPosition[i].y) ||
				!Nearly_Equal(vertex.vPosition.z, expectedPosition[i].z) ||
				!Nearly_Equal(vertex.vNormal.x, 0.f) ||
				!Nearly_Equal(vertex.vNormal.y, 1.f) ||
				!Nearly_Equal(vertex.vNormal.z, 0.f) ||
				!Nearly_Equal(vertex.vTangent.x, 1.f) ||
				!Nearly_Equal(vertex.vTangent.y, 0.f) ||
				!Nearly_Equal(vertex.vTangent.z, 0.f) ||
				!Nearly_Equal(vertex.vTexcoord.x, expectedUv0[i].x) ||
				!Nearly_Equal(vertex.vTexcoord.y, expectedUv0[i].y) ||
				!Nearly_Equal(mesh.tangentHandedness[i], expectedW[i]) ||
				!Nearly_Equal(vertex.vBinormal.z, expectedBinormalZ[i]) ||
				mesh.color0Rgba8[i] != expectedColor[i])
			{
				return false;
			}
		}
		return true;
	}

	bool Validate_Writer_Independent_Golden(
		const std::filesystem::path& hexPath,
		const std::filesystem::path& manifestPath)
	{
		std::vector<uint8_t> manifestBytes;
		std::vector<uint8_t> canonicalManifestBytes;
		std::vector<uint8_t> goldenBytes;
		if (!Read_File_Bytes(manifestPath, manifestBytes) ||
			!Canonicalize_Utf8_Lf(manifestBytes, canonicalManifestBytes) ||
			Sha256_Hex(canonicalManifestBytes) != kWriterIndependentManifestSha256 ||
			!Decode_Lowercase_Hex_File(hexPath, goldenBytes) ||
			goldenBytes.size() != kWriterIndependentGoldenByteCount ||
			Sha256_Hex(goldenBytes) != kWriterIndependentGoldenSha256)
		{
			return false;
		}

		std::error_code error;
		const std::filesystem::path temporaryPath =
			std::filesystem::temp_directory_path(error) /
			(L"lostark-wmodel-v11-independent-" +
				std::to_wstring(static_cast<uint32_t>(_getpid())) +
				L".wmodel");
		if (error)
			return false;
		{
			std::ofstream stream(temporaryPath, std::ios::binary | std::ios::trunc);
			if (!stream || !stream.write(
				reinterpret_cast<const char*>(goldenBytes.data()),
				static_cast<std::streamsize>(goldenBytes.size())))
			{
				std::filesystem::remove(temporaryPath, error);
				return false;
			}
		}

		MODEL_ASSET_DATA asset{};
		MODEL_DECODE_REPORT report{};
		const bool valid = Validate_Color_V11(temporaryPath) &&
			Decode(temporaryPath, asset, report) && report.succeeded &&
			Hex_Digest(asset.geometryMetadata.payloadSha256) ==
				kWriterIndependentPayloadSha256 &&
			Hex_Digest(asset.geometryMetadata.metadataIdentitySha256) ==
				kWriterIndependentMetadataIdentitySha256;
		std::filesystem::remove(temporaryPath, error);
		return valid && !error;
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

	bool Validate_Tangent_W_Boundary(const std::filesystem::path& path)
	{
		MODEL_ASSET_DATA asset{};
		MODEL_DECODE_REPORT report{};
		if (!Decode(path, asset, report) || !Validate_Common_V11(asset, report))
			return false;
		const MODEL_MESH_DATA& mesh = asset.meshes.front();
		return !mesh.tangentHandedness.empty() &&
			std::bit_cast<uint32_t>(mesh.tangentHandedness.front()) == 0xbf800008u &&
			Is_Valid_Tangent_Handedness(mesh.tangentHandedness.front());
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

	bool Is_Valid_Artist_Candidate(
		const MODEL_ASSET_DATA& asset,
		const MODEL_DECODE_REPORT& report)
	{
		if (!report.succeeded ||
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

	bool Validate_Artist_Candidate(const std::filesystem::path& path)
	{
		MODEL_ASSET_DATA asset{};
		MODEL_DECODE_REPORT report{};
		return Decode(path, asset, report) &&
			Is_Valid_Artist_Candidate(asset, report);
	}

	bool Dump_Artist_Candidate(const std::filesystem::path& path)
	{
		MODEL_ASSET_DATA asset{};
		MODEL_DECODE_REPORT report{};
		if (!Decode(path, asset, report) ||
			!Is_Valid_Artist_Candidate(asset, report))
			return false;

		std::ostringstream output;
		output << "{\"schema\":\"lostark.wmodel-decoded-semantic-dump\",";
		output << "\"formatVersion\":1,";
		output << "\"channelMask\":" << asset.geometryMetadata.channelMask << ',';
		output << "\"evidenceFlags\":" << asset.geometryMetadata.evidenceFlags << ',';
		output << "\"sourceToWModelScaleF32Hex\":\""
			<< F32_Hex(asset.geometryMetadata.sourceToWModelScale) << "\",";
		output << "\"geometryPreScaleF32Hex\":\""
			<< F32_Hex(asset.geometryMetadata.geometryPreScale) << "\",";
		output << "\"payloadSha256\":\""
			<< Hex_Digest(asset.geometryMetadata.payloadSha256) << "\",";
		output << "\"metadataIdentitySha256\":\""
			<< Hex_Digest(asset.geometryMetadata.metadataIdentitySha256) << "\",";
		output << "\"meshes\":[";
		for (size_t meshIndex = 0; meshIndex < asset.meshes.size(); ++meshIndex)
		{
			const MODEL_MESH_DATA& mesh = asset.meshes[meshIndex];
			std::vector<uint8_t> positions;
			std::vector<uint8_t> normals;
			std::vector<uint8_t> tangents;
			std::vector<uint8_t> uv0;
			std::vector<uint8_t> tangentW;
			std::vector<uint8_t> color0;
			std::vector<uint8_t> indices;
			positions.reserve(mesh.vertices.size() * 12u);
			normals.reserve(mesh.vertices.size() * 12u);
			tangents.reserve(mesh.vertices.size() * 12u);
			uv0.reserve(mesh.vertices.size() * 8u);
			tangentW.reserve(mesh.tangentHandedness.size() * 4u);
			color0.reserve(mesh.color0Rgba8.size() * 4u);
			indices.reserve(mesh.indices.size() * 4u);
			for (const VTXMESH& vertex : mesh.vertices)
			{
				Append_F32_LE(positions, vertex.vPosition.x);
				Append_F32_LE(positions, vertex.vPosition.y);
				Append_F32_LE(positions, vertex.vPosition.z);
				Append_F32_LE(normals, vertex.vNormal.x);
				Append_F32_LE(normals, vertex.vNormal.y);
				Append_F32_LE(normals, vertex.vNormal.z);
				Append_F32_LE(tangents, vertex.vTangent.x);
				Append_F32_LE(tangents, vertex.vTangent.y);
				Append_F32_LE(tangents, vertex.vTangent.z);
				Append_F32_LE(uv0, vertex.vTexcoord.x);
				Append_F32_LE(uv0, vertex.vTexcoord.y);
			}
			for (const float value : mesh.tangentHandedness)
				Append_F32_LE(tangentW, value);
			for (const uint32_t value : mesh.color0Rgba8)
				Append_U32_LE(color0, value);
			for (const uint32_t value : mesh.indices)
				Append_U32_LE(indices, value);

			const std::string positionSha = Sha256_Hex(positions);
			const std::string normalSha = Sha256_Hex(normals);
			const std::string tangentSha = Sha256_Hex(tangents);
			const std::string uvSha = Sha256_Hex(uv0);
			const std::string tangentWSha = Sha256_Hex(tangentW);
			const std::string colorSha = mesh.hasColor0 ? Sha256_Hex(color0) : std::string{};
			const std::string indexSha = Sha256_Hex(indices);
			if (positionSha.empty() || normalSha.empty() || tangentSha.empty() ||
				uvSha.empty() || tangentWSha.empty() || indexSha.empty() ||
				(mesh.hasColor0 && colorSha.empty()))
				return false;

			if (0 != meshIndex)
				output << ',';
			output << "{\"name\":\"" << Json_Escape(mesh.name) << "\",";
			output << "\"materialIndex\":" << mesh.materialIndex << ',';
			output << "\"vertexCount\":" << mesh.vertices.size() << ',';
			output << "\"indexCount\":" << mesh.indices.size() << ',';
			output << "\"positionSha256\":\"" << positionSha << "\",";
			output << "\"normalSha256\":\"" << normalSha << "\",";
			output << "\"tangentXyzSha256\":\"" << tangentSha << "\",";
			output << "\"uv0Sha256\":\"" << uvSha << "\",";
			output << "\"tangentWSha256\":\"" << tangentWSha << "\",";
			output << "\"hasColor0\":" << (mesh.hasColor0 ? "true" : "false") << ',';
			output << "\"color0Sha256\":";
			if (mesh.hasColor0)
				output << '"' << colorSha << '"';
			else
				output << "null";
			output << ",\"indexU32Sha256\":\"" << indexSha << "\",";
			output << "\"boundsF32Hex\":[";
			const float bounds[] = {
				mesh.embeddedBounds.minimum.x,
				mesh.embeddedBounds.minimum.y,
				mesh.embeddedBounds.minimum.z,
				mesh.embeddedBounds.maximum.x,
				mesh.embeddedBounds.maximum.y,
				mesh.embeddedBounds.maximum.z,
				mesh.embeddedBounds.center.x,
				mesh.embeddedBounds.center.y,
				mesh.embeddedBounds.center.z,
				mesh.embeddedBounds.radius,
			};
			for (size_t valueIndex = 0; valueIndex < std::size(bounds); ++valueIndex)
			{
				if (0 != valueIndex)
					output << ',';
				output << '"' << F32_Hex(bounds[valueIndex]) << '"';
			}
			output << "]}";
		}
		output << "]}\n";
		std::cout << output.str();
		return std::cout.good();
	}

	bool Sweep_Legacy_Resource_Corpus(const std::filesystem::path& root)
	{
		std::error_code error;
		if (!std::filesystem::is_directory(root, error) || error)
			return false;
		std::vector<std::filesystem::path> files;
		for (std::filesystem::recursive_directory_iterator iterator(
			root, std::filesystem::directory_options::skip_permission_denied, error), end;
			iterator != end; iterator.increment(error))
		{
			if (error)
				return false;
			if (!iterator->is_regular_file(error) || error)
				continue;
			std::wstring extension = iterator->path().extension().wstring();
			std::transform(
				extension.begin(), extension.end(), extension.begin(),
				[](wchar_t value) { return static_cast<wchar_t>(std::towlower(value)); });
			if (extension == L".wmodel")
				files.push_back(iterator->path());
		}
		std::sort(files.begin(), files.end());

		uint32_t staticCount = {};
		uint32_t skinnedCount = {};
		uint32_t hasBoundsCount = {};
		uint32_t multiSubmeshCount = {};
		for (const std::filesystem::path& path : files)
		{
			LEGACY_LAYOUT layout{};
			MODEL_ASSET_DATA asset{};
			MODEL_DECODE_REPORT report{};
			MODEL_ASSET_LOAD_DESC desc{};
			desc.assetRoot = root;
			desc.meshPath = path;
			if (!Inspect_Legacy_WModel(path, layout) ||
				!Decode_Desc(desc, asset, report) || !report.succeeded ||
				report.hasGeometryMetadata || asset.geometryMetadata.present ||
				asset.meshes.size() != layout.submeshCount || asset.meshes.empty())
			{
				std::wcerr << L"Legacy WModel corpus rejection: " << path << L'\n';
				return false;
			}
			const bool decodedSkinned = std::all_of(
				asset.meshes.begin(), asset.meshes.end(),
				[](const MODEL_MESH_DATA& mesh)
				{
					return MODEL_VERTEX_KIND::SKINNED == mesh.vertexKind;
				});
			const bool decodedStatic = std::all_of(
				asset.meshes.begin(), asset.meshes.end(),
				[](const MODEL_MESH_DATA& mesh)
				{
					return MODEL_VERTEX_KIND::STATIC == mesh.vertexKind;
				});
			const bool legacySidecarsAbsent = std::all_of(
				asset.meshes.begin(), asset.meshes.end(),
				[](const MODEL_MESH_DATA& mesh)
				{
					return !mesh.embeddedBounds.present && !mesh.hasColor0 &&
						mesh.tangentHandedness.empty() && mesh.color0Rgba8.empty() &&
						!mesh.indices.empty() &&
						(!mesh.vertices.empty() || !mesh.skinnedVertices.empty());
				});
			if ((layout.skinned && !decodedSkinned) ||
				(!layout.skinned && !decodedStatic) || !legacySidecarsAbsent)
				return false;
			if (layout.skinned)
				++skinnedCount;
			else
				++staticCount;
			if (layout.hasBounds)
				++hasBoundsCount;
			if (layout.submeshCount > 1)
				++multiSubmeshCount;
		}

		const bool valid = files.size() == kExpectedLegacyResourceCorpusCount &&
			staticCount == kExpectedLegacyStaticCount &&
			skinnedCount == kExpectedLegacySkinnedCount &&
			hasBoundsCount == kExpectedLegacyBoundsCount &&
			multiSubmeshCount == kExpectedLegacyMultiSubmeshCount;
		std::cout << "Legacy WModel C++ corpus sweep: files=" << files.size()
			<< " static=" << staticCount << " skinned=" << skinnedCount
			<< " hasBounds=" << hasBoundsCount
			<< " multiSubmesh=" << multiSubmeshCount
			<< " metadataAbsent=" << files.size()
			<< " valid=" << valid << '\n';
		return valid;
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
	if (3 == argc && std::wstring_view(argv[1]) == L"--dump-candidate")
	{
		const std::filesystem::path candidate =
			std::filesystem::absolute(argv[2]).lexically_normal();
		return Dump_Artist_Candidate(candidate) ? 0 : 1;
	}
	if (3 == argc && std::wstring_view(argv[1]) == L"--legacy-corpus")
	{
		const std::filesystem::path resourceRoot =
			std::filesystem::absolute(argv[2]).lexically_normal();
		return Sweep_Legacy_Resource_Corpus(resourceRoot) ? 0 : 1;
	}
	if (4 == argc &&
		std::wstring_view(argv[1]) == L"--writer-independent-golden")
	{
		const bool valid = Validate_Writer_Independent_Golden(
			std::filesystem::absolute(argv[2]).lexically_normal(),
			std::filesystem::absolute(argv[3]).lexically_normal());
		std::cout << "WModel writer-independent immutable golden: bytes="
			<< kWriterIndependentGoldenByteCount << ' '
			<< "decodedSemantic=" << valid
			<< " externallyAuthenticated=0 product=false\n";
		return valid ? 0 : 1;
	}
	if (2 != argc)
	{
		std::wcerr << L"usage: WModelGeometryContractHarness <suite-directory> "
			L"| --candidate <wmodel> | --dump-candidate <wmodel> "
			L"| --legacy-corpus <Resources> "
			L"| --writer-independent-golden <hex> <expected-json>\n";
		return 2;
	}

	const std::filesystem::path root =
		std::filesystem::absolute(argv[1]).lexically_normal();
	const bool validColor = Validate_Color_V11(root / L"valid_color.wmodel");
	const bool validNoColor = Validate_NoColor_V11(root / L"valid_no_color.wmodel");
	const bool validTangentBoundary = Validate_Tangent_W_Boundary(
		root / L"valid_tangent_w_boundary_in.wmodel");
	const bool legacyAbsent = Validate_Legacy_Metadata_Absent(root / L"legacy_v10.wmodel");
	const bool legacyStatic = Validate_Legacy_Static_Multisubmesh(
		root / L"legacy_static_multisubmesh_bounds.wmesh");
	const bool legacySkinned = Validate_Legacy_Skinned(root);
	const bool corruptSkeletonTransactional = Reject_Corrupt_Dependency(root, true);
	const bool corruptAnimationTransactional = Reject_Corrupt_Dependency(root, false);
	const std::array<const wchar_t*, 29> corruptFiles = {
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
		L"corrupt_tangent_w_boundary_out.wmodel",
		L"corrupt_tangent_w_minus_1_000005.wmodel",
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
		<< " validTangentBoundary=" << validTangentBoundary
		<< " legacyMetadataAbsent=" << legacyAbsent
		<< " legacyStaticMultiBounds=" << legacyStatic
		<< " legacySkinned=" << legacySkinned
		<< " corruptSkeletonTransactional=" << corruptSkeletonTransactional
		<< " corruptAnimationTransactional=" << corruptAnimationTransactional
		<< " corruptRejected=" << corruptRejected
		<< " selfConsistentUnauthenticated=true externallyAuthenticated=0 "
		<< "preScaleConsumed=false product=false\n";
	return validColor && validNoColor && validTangentBoundary &&
		legacyAbsent && legacyStatic &&
		legacySkinned && corruptSkeletonTransactional &&
		corruptAnimationTransactional && corruptRejected ? 0 : 1;
}
