#include "BinaryAsset/WModelDecoder.h"

#include "BinaryAsset/BinaryReader.h"

#include "Winters/WAnimationReader.h"
#include "Winters/WFormatTypes.h"
#include "Winters/WMaterialReader.h"
#include "Winters/WMeshReader.h"
#include "Winters/WSkeletonReader.h"

#include <cstring>
#include <algorithm>
#include <fstream>
#include <unordered_set>

using namespace Engine::WintersFormat;

namespace
{
	bool_t HasMagic(const void* pValue, const char* pMagic)
	{
		return 0 == memcmp(pValue, pMagic, 4);
	}

	void ConvertToStaticBindPose(vector<MODEL_MESH_DATA>& meshes)
	{
		for (MODEL_MESH_DATA& mesh : meshes)
		{
			mesh.vertices.resize(mesh.skinnedVertices.size());
			for (size_t i = 0; i < mesh.skinnedVertices.size(); ++i)
			{
				const VTXANIMMESH& source = mesh.skinnedVertices[i];
				VTXMESH& destination = mesh.vertices[i];
				destination.vPosition = source.vPosition;
				destination.vNormal = source.vNormal;
				destination.vTangent = source.vTangent;
				destination.vBinormal = source.vBinormal;
				destination.vTexcoord = source.vTexcoord;
			}
			mesh.skinnedVertices.clear();
			mesh.skinnedVertices.shrink_to_fit();
			mesh.vertexKind = MODEL_VERTEX_KIND::STATIC;
		}
	}

	struct MODEL_SECTION_VIEW
	{
		MODEL_SECTION_TYPE type{};
		uint32_t index = {};
		string name;
		const uint8_t* pData = { nullptr };
		size_t size = {};
	};

	string ReadFixedName(const char_t* pName, size_t capacity)
	{
		size_t length = {};
		while (length < capacity && '\0' != pName[length])
			++length;
		return string(pName, length);
	}

	string MakeUniqueAnimationName(
		const MODEL_SECTION_VIEW& section,
		unordered_set<string>& animationNames)
	{
		if (section.name.empty())
			return {};
		if (animationNames.emplace(section.name).second)
			return section.name;

		// MODEL_SECTION_DESC stores only 40 bytes. Long ActorX action names can
		// therefore share the same truncated prefix. The section index is stable
		// inside the package, so use it to preserve every clip deterministically.
		const string suffix = "#" + to_string(section.index);
		string candidate = section.name;
		if (candidate.size() + suffix.size() >= MAX_PATH)
			candidate.resize(MAX_PATH - suffix.size() - 1u);
		candidate += suffix;
		if (animationNames.emplace(candidate).second)
			return candidate;

		for (uint32_t discriminator = 1u; ; ++discriminator)
		{
			const string fallbackSuffix = suffix + "_" +
				to_string(discriminator);
			candidate = section.name;
			if (candidate.size() + fallbackSuffix.size() >= MAX_PATH)
				candidate.resize(MAX_PATH - fallbackSuffix.size() - 1u);
			candidate += fallbackSuffix;
			if (animationNames.emplace(candidate).second)
				return candidate;
		}
	}

	bool_t ValidateMeshSkeleton(W_MESH_READ_RESULT& meshResult,
		MODEL_ASSET_DATA& outAsset,
		MODEL_DECODE_REPORT& outReport)
	{
		if (meshResult.bones.size() != outAsset.skeleton.bones.size())
		{
			outReport.error = "WMSH and WSKL bone counts do not match.";
			return false;
		}

		for (uint32_t i = 0; i < meshResult.bones.size(); ++i)
		{
			if (meshResult.bones[i].nameHash != outAsset.skeleton.bones[i].nameHash)
			{
				outReport.error = "WMSH and WSKL bone order or name hash does not match.";
				return false;
			}
			outAsset.skeleton.bones[i].inverseBind = meshResult.bones[i].inverseBind;
		}
		return true;
	}

	bool_t DecodeWModelPackage(const MODEL_ASSET_LOAD_DESC& desc,
		MODEL_ASSET_DATA& outAsset,
		MODEL_DECODE_REPORT& outReport)
	{
		vector<uint8_t> bytes;
		if (!CBinaryReader::LoadFile(desc.meshPath, bytes))
		{
			outReport.error = "Could not open the WModel package.";
			return false;
		}

		try
		{
			CBinaryReader fileReader(bytes.data(), bytes.size());
			const FILE_HEADER fileHeader = fileReader.Read<FILE_HEADER>();
			if (!HasMagic(fileHeader.magic, WINTERS_MAGIC) ||
				WINT_VERSION_MAJOR != fileHeader.versionMajor ||
				fileHeader.versionMinor > WINT_GEOMETRY_VERSION_MINOR ||
				0 != fileHeader.flags ||
				fileHeader.contentSize != fileReader.Remaining())
			{
				outReport.error = "Invalid WINT WModel file header.";
				return false;
			}

			const uint8_t* pContent = fileReader.Peek();
			CBinaryReader reader(pContent, fileHeader.contentSize);
			const MODEL_META_HEADER modelHeader = reader.Read<MODEL_META_HEADER>();
			if (!HasMagic(modelHeader.magic, WMODEL_MAGIC) ||
				modelHeader.sectionCount < 2 || modelHeader.sectionCount > MAX_MODEL_SECTIONS ||
				modelHeader.animationCount > modelHeader.sectionCount)
			{
				outReport.error = "Invalid WMOD metadata.";
				return false;
			}

			const bool_t geometryContract =
				WINT_GEOMETRY_VERSION_MINOR == fileHeader.versionMinor;
			if (geometryContract &&
				(2 != modelHeader.sectionCount || 0 != modelHeader.animationCount ||
				0 != modelHeader.flags || 0 != modelHeader.reserved[0] ||
				0 != modelHeader.reserved[1] || 0 != modelHeader.reserved[2] ||
				0 != modelHeader.reserved[3]))
			{
				outReport.error = "WModel 1.1 requires canonical static WMOD metadata.";
				return false;
			}

			if (sizeof(MODEL_SECTION_DESC) > reader.Remaining() / modelHeader.sectionCount)
			{
				outReport.error = "WMOD section table is truncated.";
				return false;
			}

			vector<MODEL_SECTION_DESC> descriptors(modelHeader.sectionCount);
			reader.ReadBytes(descriptors.data(), descriptors.size() * sizeof(MODEL_SECTION_DESC));
			vector<MODEL_SECTION_VIEW> sections;
			sections.reserve(descriptors.size());
			uint32_t animationCount = {};
			size_t expectedGeometryOffset = sizeof(MODEL_META_HEADER) +
				descriptors.size() * sizeof(MODEL_SECTION_DESC);
			for (const MODEL_SECTION_DESC& source : descriptors)
			{
				if (source.offset > fileHeader.contentSize ||
					source.size > fileHeader.contentSize - source.offset ||
					source.size > SIZE_MAX)
				{
					outReport.error = "A WMOD section points outside the package.";
					return false;
				}

				if (geometryContract &&
					(source.offset != expectedGeometryOffset || 0 == source.size))
				{
					outReport.error = "WModel 1.1 sections must be non-empty and contiguous.";
					return false;
				}
				if (geometryContract)
					expectedGeometryOffset += static_cast<size_t>(source.size);

				const MODEL_SECTION_TYPE type = static_cast<MODEL_SECTION_TYPE>(source.type);
				if (type < MODEL_SECTION_TYPE::MESH || type > MODEL_SECTION_TYPE::ANIMATION)
				{
					outReport.error = "WMOD contains an unknown section type.";
					return false;
				}
				if (MODEL_SECTION_TYPE::ANIMATION == type)
					++animationCount;

				sections.push_back(MODEL_SECTION_VIEW{
					type,
					source.index,
					ReadFixedName(source.name, sizeof(source.name)),
					pContent + static_cast<size_t>(source.offset),
					static_cast<size_t>(source.size) });
			}

			if (geometryContract && expectedGeometryOffset != fileHeader.contentSize)
			{
				outReport.error = "WModel 1.1 contains trailing section payload.";
				return false;
			}

			if (animationCount != modelHeader.animationCount)
			{
				outReport.error = "WMOD animation count does not match its section table.";
				return false;
			}

			const auto FindSection = [&](MODEL_SECTION_TYPE type) -> const MODEL_SECTION_VIEW*
			{
				const MODEL_SECTION_VIEW* pResult = { nullptr };
				for (const MODEL_SECTION_VIEW& section : sections)
				{
					if (section.type != type)
						continue;
					if (nullptr != pResult)
						return nullptr;
					pResult = &section;
				}
				return pResult;
			};

			const MODEL_SECTION_VIEW* pMesh = FindSection(MODEL_SECTION_TYPE::MESH);
			const MODEL_SECTION_VIEW* pMaterial = FindSection(MODEL_SECTION_TYPE::MATERIAL);
			if (nullptr == pMesh || nullptr == pMaterial)
			{
				outReport.error = "WMOD requires exactly one mesh and one material section.";
				return false;
			}

			W_MESH_READ_RESULT meshResult{};
			if (!CWMeshReader{}.ReadMemory(pMesh->pData, pMesh->size, meshResult, outReport))
				return false;
			if (meshResult.fileVersionMinor != fileHeader.versionMinor)
			{
				outReport.error = "WMOD and WMSH geometry format versions do not match.";
				return false;
			}
			if (!CWMaterialReader{}.ReadMemory(
				pMaterial->pData, pMaterial->size, desc, desc.meshPath,
				meshResult.materialCount, outAsset.materials, outReport))
				return false;

			outAsset.meshes = move(meshResult.meshes);
			outAsset.geometryMetadata = meshResult.geometryMetadata;
			const MODEL_SECTION_VIEW* pSkeleton = FindSection(MODEL_SECTION_TYPE::SKELETON);
			if (meshResult.skinned)
			{
				if (nullptr == pSkeleton ||
					!CWSkeletonReader{}.ReadMemory(
						pSkeleton->pData, pSkeleton->size, outAsset.skeleton, outReport))
				{
					if (outReport.error.empty())
						outReport.error = "A skinned WMOD requires one skeleton section.";
					return false;
				}
				if (!ValidateMeshSkeleton(meshResult, outAsset, outReport))
					return false;
				outAsset.hasSkeleton = true;

				vector<const MODEL_SECTION_VIEW*> animations;
				for (const MODEL_SECTION_VIEW& section : sections)
				{
					if (MODEL_SECTION_TYPE::ANIMATION == section.type)
						animations.push_back(&section);
				}
				sort(animations.begin(), animations.end(), [](const auto* pLeft, const auto* pRight)
					{ return pLeft->index < pRight->index; });

				unordered_set<string> animationNames;
				outAsset.animations.reserve(animations.size());
				for (const MODEL_SECTION_VIEW* pAnimation : animations)
				{
					const string animationName = MakeUniqueAnimationName(
						*pAnimation, animationNames);
					if (animationName.empty())
					{
						outReport.error = "WMOD contains an unnamed animation clip.";
						return false;
					}

					MODEL_ANIMATION_DATA animation{};
					if (!CWAnimationReader{}.ReadMemory(
						pAnimation->pData, pAnimation->size, animationName,
						outAsset.skeleton, animation, outReport))
						return false;
					outAsset.animations.push_back(move(animation));
				}
			}
			else if (nullptr != pSkeleton || 0 != animationCount)
			{
				outReport.error = "A static WMOD cannot contain skeleton or animation sections.";
				return false;
			}

			outReport.usedSkinnedBindPose = meshResult.skinned && outAsset.animations.empty();
			outReport.meshCount = static_cast<uint32_t>(outAsset.meshes.size());
			outReport.materialCount = static_cast<uint32_t>(outAsset.materials.size());
			outReport.boneCount = static_cast<uint32_t>(outAsset.skeleton.bones.size());
			outReport.animationCount = static_cast<uint32_t>(outAsset.animations.size());
			outReport.hasGeometryMetadata = outAsset.geometryMetadata.present;
			outReport.geometryEvidenceFlags = outAsset.geometryMetadata.evidenceFlags;
			outReport.succeeded = true;
			return true;
		}
		catch (const exception& exception)
		{
			outReport.error = exception.what();
			return false;
		}
	}
}

const char_t* CWModelDecoder::Get_Name() const
{
	return "Winters WINT model package v1/v1.1 geometry";
}

bool_t CWModelDecoder::CanDecode(const filesystem::path& meshPath) const
{
	ifstream stream(meshPath, ios::binary);
	if (!stream)
		return false;

	char_t probe[20]{};
	if (!stream.read(probe, sizeof(probe)))
		return false;

	return HasMagic(probe, WINTERS_MAGIC) &&
		(HasMagic(probe + sizeof(FILE_HEADER), WMESH_MAGIC) ||
			HasMagic(probe + sizeof(FILE_HEADER), WMODEL_MAGIC));
}

bool_t CWModelDecoder::Decode(const MODEL_ASSET_LOAD_DESC& desc,
	MODEL_ASSET_DATA& outAsset,
	MODEL_DECODE_REPORT& outReport) const
{
	outAsset = {};
	MODEL_ASSET_DATA stagedAsset{};
	outReport.meshPath = desc.meshPath;
	outReport.decoderName = Get_Name();

	ifstream stream(desc.meshPath, ios::binary);
	char_t probe[20]{};
	if (!stream.read(probe, sizeof(probe)))
	{
		outReport.error = "Could not probe the Winters model asset.";
		return false;
	}
	if (HasMagic(probe + sizeof(FILE_HEADER), WMODEL_MAGIC))
	{
		if (!DecodeWModelPackage(desc, stagedAsset, outReport))
			return false;
		outAsset = move(stagedAsset);
		return true;
	}

	W_MESH_READ_RESULT meshResult{};
	if (!CWMeshReader{}.Read(desc.meshPath, meshResult, outReport))
		return false;

	if (!CWMaterialReader{}.Read(
		desc,
		meshResult.materialCount,
		stagedAsset.materials,
		outReport))
		return false;

	stagedAsset.meshes = move(meshResult.meshes);
	stagedAsset.geometryMetadata = meshResult.geometryMetadata;
	bool_t usedBindPoseFallback = false;
	if (meshResult.skinned)
	{
		filesystem::path skeletonPath = desc.skeletonPath;
		if (skeletonPath.empty())
		{
			skeletonPath = desc.meshPath;
			skeletonPath.replace_extension(L".wskel");
		}

		const bool_t skeletonWasRequested =
			!desc.skeletonPath.empty() || !desc.animationPaths.empty();
		if (!filesystem::exists(skeletonPath) && !skeletonWasRequested)
		{
			ConvertToStaticBindPose(stagedAsset.meshes);
			meshResult.skinned = false;
			usedBindPoseFallback = true;
		}
		else if (!CWSkeletonReader{}.Read(skeletonPath, stagedAsset.skeleton, outReport))
		{
			return false;
		}

		if (!meshResult.skinned)
		{
			outReport.usedSkinnedBindPose = true;
		}
		else
		{
		if (meshResult.bones.size() != stagedAsset.skeleton.bones.size())
		{
			outReport.error = "WMSH and WSKL bone counts do not match.";
			return false;
		}

		for (uint32_t i = 0; i < meshResult.bones.size(); ++i)
		{
			if (meshResult.bones[i].nameHash != stagedAsset.skeleton.bones[i].nameHash)
			{
				outReport.error = "WMSH and WSKL bone order or name hash does not match.";
				return false;
			}
			stagedAsset.skeleton.bones[i].inverseBind = meshResult.bones[i].inverseBind;
		}
		stagedAsset.hasSkeleton = true;

		unordered_set<string> animationNames;
		stagedAsset.animations.reserve(desc.animationPaths.size());
		for (filesystem::path animationPath : desc.animationPaths)
		{
			if (animationPath.is_relative())
				animationPath = desc.meshPath.parent_path() / animationPath;

			MODEL_ANIMATION_DATA animation{};
			if (!CWAnimationReader{}.Read(
				animationPath.lexically_normal(),
				stagedAsset.skeleton,
				animation,
				outReport))
				return false;
			if (!animationNames.emplace(animation.name).second)
			{
				outReport.error = "The model package contains duplicate animation clip names.";
				return false;
			}
			stagedAsset.animations.push_back(move(animation));
		}
		}
	}
	else if (!desc.skeletonPath.empty() || !desc.animationPaths.empty())
	{
		outReport.error = "A static WMSH cannot declare skeleton or animation dependencies.";
		return false;
	}

	outReport.usedSkinnedBindPose = usedBindPoseFallback ||
		(meshResult.skinned && stagedAsset.animations.empty());
	outReport.meshCount = static_cast<uint32_t>(stagedAsset.meshes.size());
	outReport.materialCount = static_cast<uint32_t>(stagedAsset.materials.size());
	outReport.boneCount = static_cast<uint32_t>(stagedAsset.skeleton.bones.size());
	outReport.animationCount = static_cast<uint32_t>(stagedAsset.animations.size());
	outReport.hasGeometryMetadata = stagedAsset.geometryMetadata.present;
	outReport.geometryEvidenceFlags = stagedAsset.geometryMetadata.evidenceFlags;
	outReport.succeeded = true;
	outAsset = move(stagedAsset);
	return true;
}
