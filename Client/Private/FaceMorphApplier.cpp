#include "FaceMorphApplier.h"

#include "Model.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <unordered_map>
#include <unordered_set>

namespace
{
	constexpr f32_t WEIGHT_EPSILON = 1e-4f;

	bool_t Is_Same_Name(const std::string& strLeft, const std::string& strRight)
	{
		if (strLeft.size() != strRight.size())
			return false;
		for (size_t i = 0; i < strLeft.size(); ++i)
		{
			const int32_t iLeft = std::tolower(static_cast<unsigned char>(strLeft[i]));
			const int32_t iRight = std::tolower(static_cast<unsigned char>(strRight[i]));
			if (iLeft != iRight)
				return false;
		}
		return true;
	}

	bool_t Read_File(const std::filesystem::path& Path, std::vector<uint8_t>& OutBytes)
	{
		std::ifstream stream(Path, std::ios::binary | std::ios::ate);
		if (!stream.is_open())
			return false;
		const std::streamsize iSize = stream.tellg();
		if (iSize <= 0)
			return false;
		stream.seekg(0, std::ios::beg);
		OutBytes.resize(static_cast<size_t>(iSize));
		return !!stream.read(reinterpret_cast<char_t*>(OutBytes.data()), iSize);
	}

	template <typename T>
	bool_t Read_Value(const std::vector<uint8_t>& Bytes, size_t& iAt, T& OutValue)
	{
		if (iAt + sizeof(T) > Bytes.size())
			return false;
		std::memcpy(&OutValue, Bytes.data() + iAt, sizeof(T));
		iAt += sizeof(T);
		return true;
	}

	/* One .facemorphs morph record, decoded straight off disk: sparse (wedgeIndex,
	positionDelta, normalDelta) triples, in the file's own ascending-wedgeIndex order. Kept
	separate from CFaceMorphApplier::MORPH because a wedge does not become a runtime vertex
	until it is looked up in the .facemorphmap. */
	struct RAW_MORPH
	{
		std::string strName;
		std::vector<uint32_t> WedgeIndices;
		std::vector<float3_t> PositionDeltas;
		std::vector<float3_t> NormalDeltas;
	};

	bool_t Read_FaceMorphs(const std::filesystem::path& Path,
		uint32_t& OutVertexIndexBound, std::vector<RAW_MORPH>& OutMorphs)
	{
		std::vector<uint8_t> Bytes;
		if (!Read_File(Path, Bytes))
			return false;

		size_t iAt = 0u;
		char_t Magic[8]{};
		if (iAt + 8u > Bytes.size())
			return false;
		std::memcpy(Magic, Bytes.data(), 8u);
		iAt += 8u;
		if (0 != std::memcmp(Magic, "LAFMORPH", 8u))
			return false;

		uint32_t iVersion = 0u, iBaseVertexCount = 0u, iVertexIndexBound = 0u,
			iMorphCount = 0u, iTotalVertexCount = 0u;
		if (!Read_Value(Bytes, iAt, iVersion) || 1u != iVersion ||
			!Read_Value(Bytes, iAt, iBaseVertexCount) ||
			!Read_Value(Bytes, iAt, iVertexIndexBound) ||
			!Read_Value(Bytes, iAt, iMorphCount) ||
			!Read_Value(Bytes, iAt, iTotalVertexCount))
		{
			return false;
		}

		OutVertexIndexBound = iVertexIndexBound;
		OutMorphs.clear();
		OutMorphs.reserve(iMorphCount);
		for (uint32_t iMorph = 0u; iMorph < iMorphCount; ++iMorph)
		{
			uint32_t iNameLength = 0u;
			if (!Read_Value(Bytes, iAt, iNameLength) || iAt + iNameLength > Bytes.size())
				return false;

			RAW_MORPH Morph;
			Morph.strName.assign(
				reinterpret_cast<const char_t*>(Bytes.data() + iAt), iNameLength);
			iAt += iNameLength;
			iAt += (4u - (iNameLength % 4u)) % 4u;

			uint32_t iVertexCount = 0u;
			if (!Read_Value(Bytes, iAt, iVertexCount))
				return false;

			Morph.WedgeIndices.reserve(iVertexCount);
			Morph.PositionDeltas.reserve(iVertexCount);
			Morph.NormalDeltas.reserve(iVertexCount);
			for (uint32_t iVertex = 0u; iVertex < iVertexCount; ++iVertex)
			{
				uint32_t iWedgeIndex = 0u;
				float3_t vPositionDelta{}, vNormalDelta{};
				if (!Read_Value(Bytes, iAt, iWedgeIndex) ||
					!Read_Value(Bytes, iAt, vPositionDelta) ||
					!Read_Value(Bytes, iAt, vNormalDelta))
				{
					return false;
				}
				Morph.WedgeIndices.push_back(iWedgeIndex);
				Morph.PositionDeltas.push_back(vPositionDelta);
				Morph.NormalDeltas.push_back(vNormalDelta);
			}
			OutMorphs.push_back(std::move(Morph));
		}
		return true;
	}

	bool_t Read_FaceMorphMap(const std::filesystem::path& Path,
		uint32_t& OutSourceWedgeCount,
		std::vector<std::vector<std::pair<uint32_t, uint32_t>>>& OutTargetsByWedge)
	{
		std::vector<uint8_t> Bytes;
		if (!Read_File(Path, Bytes))
			return false;

		size_t iAt = 0u;
		char_t Magic[8]{};
		if (iAt + 8u > Bytes.size())
			return false;
		std::memcpy(Magic, Bytes.data(), 8u);
		iAt += 8u;
		if (0 != std::memcmp(Magic, "LAFMVMAP", 8u))
			return false;

		uint32_t iVersion = 0u, iSourceWedgeCount = 0u, iMeshCount = 0u;
		if (!Read_Value(Bytes, iAt, iVersion) || 2u != iVersion ||
			!Read_Value(Bytes, iAt, iSourceWedgeCount) ||
			!Read_Value(Bytes, iAt, iMeshCount))
		{
			return false;
		}
		/* Per-mesh vertex counts follow the header for bounds validation on the tool side;
		this reader trusts CModel's own Get_MeshVertexCount() instead, so it only needs to
		skip past them here. */
		iAt += static_cast<size_t>(iMeshCount) * sizeof(uint32_t);

		OutSourceWedgeCount = iSourceWedgeCount;
		OutTargetsByWedge.assign(iSourceWedgeCount, {});
		for (uint32_t iWedge = 0u; iWedge < iSourceWedgeCount; ++iWedge)
		{
			uint32_t iTargetCount = 0u;
			if (!Read_Value(Bytes, iAt, iTargetCount))
				return false;
			std::vector<std::pair<uint32_t, uint32_t>>& Targets = OutTargetsByWedge[iWedge];
			Targets.reserve(iTargetCount);
			for (uint32_t iTarget = 0u; iTarget < iTargetCount; ++iTarget)
			{
				uint32_t iMeshIndex = 0u, iVertexIndex = 0u;
				if (!Read_Value(Bytes, iAt, iMeshIndex) ||
					!Read_Value(Bytes, iAt, iVertexIndex))
				{
					return false;
				}
				Targets.emplace_back(iMeshIndex, iVertexIndex);
			}
		}
		return iAt == Bytes.size();
	}
}

bool_t Client::CFaceMorphApplier::Initialize(
	const shared_ptr<Engine::CModel>& pModel,
	const std::filesystem::path& FaceMorphsPath,
	const std::filesystem::path& FaceMorphMapPath)
{
	m_Morphs.clear();
	m_EverTouched.clear();
	m_isDirty = false;
	if (nullptr == pModel)
		return false;

	uint32_t iVertexIndexBound = 0u;
	std::vector<RAW_MORPH> RawMorphs;
	if (!Read_FaceMorphs(FaceMorphsPath, iVertexIndexBound, RawMorphs))
	{
		OutputDebugStringA(("[FaceMorph] failed to read " + FaceMorphsPath.string() + "\n").c_str());
		return false;
	}

	uint32_t iSourceWedgeCount = 0u;
	std::vector<std::vector<std::pair<uint32_t, uint32_t>>> TargetsByWedge;
	if (!Read_FaceMorphMap(FaceMorphMapPath, iSourceWedgeCount, TargetsByWedge))
	{
		OutputDebugStringA(("[FaceMorph] failed to read " + FaceMorphMapPath.string() + "\n").c_str());
		return false;
	}

	if (iSourceWedgeCount < iVertexIndexBound)
	{
		OutputDebugStringA("[FaceMorph] .facemorphmap has fewer wedges than .facemorphs references\n");
		return false;
	}

	m_Morphs.reserve(RawMorphs.size());
	for (const RAW_MORPH& Raw : RawMorphs)
	{
		MORPH Morph;
		Morph.strName = Raw.strName;
		for (size_t i = 0; i < Raw.WedgeIndices.size(); ++i)
		{
			const uint32_t iWedgeIndex = Raw.WedgeIndices[i];
			if (iWedgeIndex >= TargetsByWedge.size())
				return false;
			for (const std::pair<uint32_t, uint32_t>& Target : TargetsByWedge[iWedgeIndex])
			{
				if (Target.second >= pModel->Get_MeshVertexCount(Target.first))
					return false;
				VERTEX_TARGET VertexTarget;
				VertexTarget.iMeshIndex = Target.first;
				VertexTarget.iVertexIndex = Target.second;
				VertexTarget.vPositionDelta = Raw.PositionDeltas[i];
				VertexTarget.vNormalDelta = Raw.NormalDeltas[i];
				Morph.Targets.push_back(VertexTarget);
			}
		}
		m_Morphs.push_back(std::move(Morph));
	}
	return !m_Morphs.empty();
}

const std::string& Client::CFaceMorphApplier::Get_MorphName(const size_t iMorph) const
{
	static const std::string Empty;
	return iMorph < m_Morphs.size() ? m_Morphs[iMorph].strName : Empty;
}

f32_t Client::CFaceMorphApplier::Get_Weight(const size_t iMorph) const
{
	return iMorph < m_Morphs.size() ? m_Morphs[iMorph].fWeight : 0.f;
}

bool_t Client::CFaceMorphApplier::Set_Weight(const size_t iMorph, const f32_t fWeight)
{
	if (iMorph >= m_Morphs.size() || !std::isfinite(fWeight))
		return false;
	const f32_t fClamped = std::clamp(fWeight, 0.f, 1.f);
	MORPH& Morph = m_Morphs[iMorph];
	if (std::fabs(fClamped - Morph.fWeight) > WEIGHT_EPSILON)
	{
		Morph.fWeight = fClamped;
		m_isDirty = true;
	}
	return true;
}

bool_t Client::CFaceMorphApplier::Set_Weight_ByName(const std::string& strName, const f32_t fWeight)
{
	for (size_t i = 0; i < m_Morphs.size(); ++i)
	{
		if (Is_Same_Name(m_Morphs[i].strName, strName))
			return Set_Weight(i, fWeight);
	}
	return false;
}

void Client::CFaceMorphApplier::Reset_Weights()
{
	for (MORPH& Morph : m_Morphs)
	{
		if (std::fabs(Morph.fWeight) > WEIGHT_EPSILON)
		{
			Morph.fWeight = 0.f;
			m_isDirty = true;
		}
	}
}

void Client::CFaceMorphApplier::Apply(const shared_ptr<Engine::CModel>& pModel)
{
	if (!m_isDirty || nullptr == pModel || m_Morphs.empty())
		return;
	m_isDirty = false;

	std::unordered_map<TOUCHED_KEY, std::pair<float3_t, float3_t>, TOUCHED_KEY_HASH> AccumulatedByVertex;
	for (const TOUCHED_KEY& Key : m_EverTouched)
		AccumulatedByVertex.emplace(Key, std::pair<float3_t, float3_t>{});

	for (const MORPH& Morph : m_Morphs)
	{
		if (std::fabs(Morph.fWeight) <= WEIGHT_EPSILON)
			continue;
		for (const VERTEX_TARGET& Target : Morph.Targets)
		{
			const TOUCHED_KEY Key{ Target.iMeshIndex, Target.iVertexIndex };
			std::pair<float3_t, float3_t>& Accumulated = AccumulatedByVertex[Key];
			Accumulated.first.x += Morph.fWeight * Target.vPositionDelta.x;
			Accumulated.first.y += Morph.fWeight * Target.vPositionDelta.y;
			Accumulated.first.z += Morph.fWeight * Target.vPositionDelta.z;
			Accumulated.second.x += Morph.fWeight * Target.vNormalDelta.x;
			Accumulated.second.y += Morph.fWeight * Target.vNormalDelta.y;
			Accumulated.second.z += Morph.fWeight * Target.vNormalDelta.z;
		}
	}

	std::unordered_map<uint32_t, std::vector<uint32_t>> IndicesByMesh;
	std::unordered_map<uint32_t, std::vector<float3_t>> PositionsByMesh;
	std::unordered_map<uint32_t, std::vector<float3_t>> NormalsByMesh;
	for (const std::pair<const TOUCHED_KEY, std::pair<float3_t, float3_t>>& Entry : AccumulatedByVertex)
	{
		/* The base pose is captured when a mesh is first given its own vertex buffer, so a
		mesh has to be made unique before it can be read back. Doing it in the write loop
		below was too late: every vertex failed this read, the write loop then had nothing
		to iterate, and no mesh was ever made unique -- the morphs never applied at all. */
		if (!pModel->Has_MorphBaseVertices(Entry.first.iMeshIndex) &&
			FAILED(pModel->Make_MeshVertexBuffer_Unique(Entry.first.iMeshIndex)))
		{
			continue;
		}
		float3_t vBasePosition{}, vBaseNormal{};
		if (!pModel->Get_MorphBaseVertex(Entry.first.iMeshIndex, Entry.first.iVertexIndex,
			vBasePosition, vBaseNormal))
		{
			continue;
		}
		const float3_t& vAccumulatedPosition = Entry.second.first;
		const float3_t& vAccumulatedNormal = Entry.second.second;
		IndicesByMesh[Entry.first.iMeshIndex].push_back(Entry.first.iVertexIndex);
		PositionsByMesh[Entry.first.iMeshIndex].push_back(float3_t{
			vBasePosition.x + vAccumulatedPosition.x,
			vBasePosition.y + vAccumulatedPosition.y,
			vBasePosition.z + vAccumulatedPosition.z });
		NormalsByMesh[Entry.first.iMeshIndex].push_back(float3_t{
			vBaseNormal.x + vAccumulatedNormal.x,
			vBaseNormal.y + vAccumulatedNormal.y,
			vBaseNormal.z + vAccumulatedNormal.z });
	}

	std::string strReport = "[FaceMorph] apply:";
	for (const std::pair<const uint32_t, std::vector<uint32_t>>& Entry : IndicesByMesh)
	{
		const uint32_t iMeshIndex = Entry.first;
		strReport += " mesh" + std::to_string(iMeshIndex) + "=" +
			std::to_string(Entry.second.size());
		pModel->Update_Mesh_Vertices(iMeshIndex, Entry.second,
			PositionsByMesh[iMeshIndex], NormalsByMesh[iMeshIndex]);
	}

	strReport += " (morphs=" + std::to_string(m_Morphs.size()) +
		" accumulated=" + std::to_string(AccumulatedByVertex.size()) + ")\n";
	OutputDebugStringA(strReport.c_str());
	m_EverTouched.clear();
	m_EverTouched.reserve(AccumulatedByVertex.size());
	for (const std::pair<const TOUCHED_KEY, std::pair<float3_t, float3_t>>& Entry : AccumulatedByVertex)
		m_EverTouched.push_back(Entry.first);
}
