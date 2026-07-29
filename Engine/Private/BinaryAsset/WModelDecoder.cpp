#include "BinaryAsset/WModelDecoder.h"

#include "Winters/WAnimationReader.h"
#include "Winters/WFormatTypes.h"
#include "Winters/WMaterialReader.h"
#include "Winters/WMeshReader.h"
#include "Winters/WSkeletonReader.h"

#include <cstring>
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
}

const char_t* CWModelDecoder::Get_Name() const
{
	return "Winters WINT model package v1";
}

bool_t CWModelDecoder::CanDecode(const filesystem::path& meshPath) const
{
	ifstream stream(meshPath, ios::binary);
	if (!stream)
		return false;

	char_t probe[20]{};
	if (!stream.read(probe, sizeof(probe)))
		return false;

	return HasMagic(probe, WINTERS_MAGIC) && HasMagic(probe + sizeof(FILE_HEADER), WMESH_MAGIC);
}

bool_t CWModelDecoder::Decode(const MODEL_ASSET_LOAD_DESC& desc,
	MODEL_ASSET_DATA& outAsset,
	MODEL_DECODE_REPORT& outReport) const
{
	outAsset = {};
	outReport.meshPath = desc.meshPath;
	outReport.decoderName = Get_Name();

	W_MESH_READ_RESULT meshResult{};
	if (!CWMeshReader{}.Read(desc.meshPath, meshResult, outReport))
		return false;

	if (!CWMaterialReader{}.Read(
		desc,
		meshResult.materialCount,
		outAsset.materials,
		outReport))
		return false;

	outAsset.meshes = move(meshResult.meshes);
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
			ConvertToStaticBindPose(outAsset.meshes);
			meshResult.skinned = false;
			usedBindPoseFallback = true;
		}
		else if (!CWSkeletonReader{}.Read(skeletonPath, outAsset.skeleton, outReport))
		{
			return false;
		}

		if (!meshResult.skinned)
		{
			outReport.usedSkinnedBindPose = true;
		}
		else
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
		outAsset.hasSkeleton = true;

		unordered_set<string> animationNames;
		outAsset.animations.reserve(desc.animationPaths.size());
		for (filesystem::path animationPath : desc.animationPaths)
		{
			if (animationPath.is_relative())
				animationPath = desc.meshPath.parent_path() / animationPath;

			MODEL_ANIMATION_DATA animation{};
			if (!CWAnimationReader{}.Read(
				animationPath.lexically_normal(),
				outAsset.skeleton,
				animation,
				outReport))
				return false;
			if (!animationNames.emplace(animation.name).second)
			{
				outReport.error = "The model package contains duplicate animation clip names.";
				return false;
			}
			outAsset.animations.push_back(move(animation));
		}
		}
	}
	else if (!desc.skeletonPath.empty() || !desc.animationPaths.empty())
	{
		outReport.error = "A static WMSH cannot declare skeleton or animation dependencies.";
		return false;
	}

	outReport.usedSkinnedBindPose = usedBindPoseFallback ||
		(meshResult.skinned && outAsset.animations.empty());
	outReport.meshCount = static_cast<uint32_t>(outAsset.meshes.size());
	outReport.materialCount = static_cast<uint32_t>(outAsset.materials.size());
	outReport.boneCount = static_cast<uint32_t>(outAsset.skeleton.bones.size());
	outReport.animationCount = static_cast<uint32_t>(outAsset.animations.size());
	outReport.succeeded = true;
	return true;
}
