#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Engine)
class CModel;
NS_END

NS_BEGIN(Client)

/* Applies the character-creation base tab's face MorphTargets to a character's mesh.

Reads a class' cooked <Class>.facemorphs (deltas, indexed by retail wedge) and
<Class>.facemorphmap (wedge -> one or more runtime (meshIndex, localVertexIndex) targets on
the model's own CModel::m_Meshes) once, then on a weight change recomputes every touched
vertex as base + sum(weight * delta) over every currently-nonzero morph and pushes the
result through CModel::Update_Mesh_Vertices. The base itself is never cached here -- it is
read back from the model each time via CModel::Get_MorphBaseVertex(), so this class holds no
per-character mesh data of its own beyond the deltas (which are shared, read-only, per class,
not per character).

Nothing is opt-in at load time: the first actual weight change is what makes that character's
mesh give itself its own vertex buffer (CModel::Make_MeshVertexBuffer_Unique), so a class with
no face morph data, or a character nobody ever edits, costs nothing. See
Tools/CharacterCustomizing/FACEMORPH_FORMAT.md for the on-disk formats. */
class CFaceMorphApplier final
{
public:
	/* Reads both files against pModel's mesh layout (out-of-range mesh/vertex indices fail
	the whole load rather than silently dropping one morph's vertex -- a corrupt or
	out-of-sync pair of files is exactly the case that must not run half-applied). Returns
	false, leaving this applier empty (Apply() then does nothing), if either file is
	missing/malformed or the two disagree on wedge count. */
	bool_t Initialize(
		const shared_ptr<Engine::CModel>& pModel,
		const std::filesystem::path& FaceMorphsPath,
		const std::filesystem::path& FaceMorphMapPath);

	size_t Get_MorphCount() const {
		return m_Morphs.size();
	}
	const std::string& Get_MorphName(size_t iMorph) const;
	f32_t Get_Weight(size_t iMorph) const;
	/* Clamped to [0, 1]. Returns false (and leaves the weight unchanged) for a bad index or
	a non-finite value. Marks this applier dirty on an actual change; Apply() only does work
	when dirty. */
	bool_t Set_Weight(size_t iMorph, f32_t fWeight);
	/* Matched case-insensitively: .facemorphs carries the retail name table's lowercase
	spelling (mm_eye_meshtype_01_ui) and the preset document the authoring one
	(MM_Eye_MeshType_01_UI). Returns false for a name this class has no morph for. */
	bool_t Set_Weight_ByName(const std::string& strName, f32_t fWeight);
	void Reset_Weights();

	/* Recomputes and pushes every vertex touched by a currently- or previously-nonzero
	morph. Cheap no-op when nothing changed since the last call. Safe to call every frame;
	only actually touches the GPU buffer on a real weight change. */
	void Apply(const shared_ptr<Engine::CModel>& pModel);

private:
	struct VERTEX_TARGET
	{
		uint32_t iMeshIndex = 0u;
		uint32_t iVertexIndex = 0u;
		float3_t vPositionDelta{};
		float3_t vNormalDelta{};
	};
	struct MORPH
	{
		std::string strName;
		f32_t fWeight = 0.f;
		std::vector<VERTEX_TARGET> Targets;
	};
	struct TOUCHED_KEY
	{
		uint32_t iMeshIndex;
		uint32_t iVertexIndex;
		bool_t operator==(const TOUCHED_KEY& other) const {
			return iMeshIndex == other.iMeshIndex && iVertexIndex == other.iVertexIndex;
		}
	};
	struct TOUCHED_KEY_HASH
	{
		size_t operator()(const TOUCHED_KEY& key) const {
			return (static_cast<size_t>(key.iMeshIndex) << 32) ^ key.iVertexIndex;
		}
	};

private:
	std::vector<MORPH>	m_Morphs;
	/* Every (meshIndex, vertexIndex) any morph has ever touched while active, so a morph
	going back to weight 0 still gets its vertices recomputed (and reverted to base, or to
	whatever other still-active morph now owns them) instead of staying stuck at its last
	pushed value. */
	std::vector<TOUCHED_KEY>	m_EverTouched;
	bool_t				m_isDirty = { false };
};

NS_END
