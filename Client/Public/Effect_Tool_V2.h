#pragma once

#include "Client_Defines.h"
#include "EffectV2_Document.h"
#include "EffectV2_Object.h"
#include "Engine_Defines.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

NS_BEGIN(Engine)
class CModel;
class CShader;
NS_END

NS_BEGIN(Client)

class CNpc;

class CEffect_Tool_V2 final
{
public:
	using EFFECT_TYPE = EFFECT_V2_TYPE;

	enum class RESOURCE_SLOT : int32_t
	{
		MESH,
		BASE,
		NOISE,
		MASK,
		EMISSIVE,
		DISSOLVE,
		END
	};

	enum class RESOURCE_KIND : int32_t
	{
		TEXTURE,
		MODEL,
		END
	};

private:
	struct TEXTURE_USAGE final
	{
		std::array<uint32_t, static_cast<size_t>(RESOURCE_SLOT::END)> Counts{};
		std::vector<std::string> Params;
	};

	struct RESOURCE_ENTRY final
	{
		std::string strAssetId;
		std::string strDomain;
		std::string strFileName;
		RESOURCE_KIND eKind = RESOURCE_KIND::TEXTURE;
		const TEXTURE_USAGE* pUsage = nullptr;
	};

	struct PREVIEW_ENTRY final
	{
		ComPtr<ID3D11ShaderResourceView> pTextureView;
		std::string strError;
		std::string strInfo;
		uint32_t iWidth = 0u;
		uint32_t iHeight = 0u;
	};

	using SLOT_BINDINGS =
		std::array<std::string, static_cast<size_t>(RESOURCE_SLOT::END)>;

	using PART_OVERRIDE = EFFECT_V2_PART_OVERRIDE;
	using PIVOT_ROTATION = CEffectV2Object::PIVOT_ROTATION;
	using EFFECT_BINDING = EFFECT_V2_BINDING;

	enum class PIVOT_MODE : int32_t
	{
		WORLD,
		TARGET_BONE,
		END
	};

public:
	CEffect_Tool_V2(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	~CEffect_Tool_V2();

	void Render();

private:
	void Scan_Resources();
	void Load_TextureUsage();
	void Rebuild_VisibleResources();
	const PREVIEW_ENTRY* Request_Preview(
		const std::string& strAssetId, RESOURCE_KIND eKind);
	bool_t Create_ModelThumbnail(
		const std::filesystem::path& Path,
		ComPtr<ID3D11ShaderResourceView>& OutTextureView,
		std::string& strOutError,
		std::string& strOutInfo);
	static bool_t Compute_SkinnedBounds(
		const std::filesystem::path& Path,
		const Engine::CModel& Model,
		float3_t& OutMinimum,
		float3_t& OutMaximum,
		std::string& strOutInfo);

	void Render_TypeSelector();
	void Render_SlotCards();
	void Render_ResourceBrowser();
	void Render_PreviewPanel();
	void Render_CreatePanel();
	void Render_DocumentPanel();
	void Render_TuningPanel();
	bool_t Try_CreatePreview();
	bool_t Spawn_Preview(
		const CEffectV2Object::DESC& Desc,
		const std::vector<PART_OVERRIDE>& Parts,
		const std::string& strAnimationClip);
	void Scan_Documents();
	bool_t Save_Document();
	bool_t Load_Document(const std::string& strEffectId);

	void Render_AttachWindow();
	bool_t Spawn_Target(const std::string& strArchetypeId);
	void Despawn_Target();
	void Move_Target(const float3_t& vPosition, f32_t fYawDegrees);
	void Update_Attach(f32_t fTimeDelta);
	bool_t Save_Bindings();
	bool_t Load_Bindings(const std::string& strArchetypeId);
	static bool_t Collect_BoneNames(
		const std::string& strModelAssetId,
		std::vector<std::string>& OutNames);

	SLOT_BINDINGS& Current_Bindings();
	std::string& Current_SlotAssetId();
	bool_t Slot_VisibleForType(RESOURCE_SLOT eSlot) const;

	static RESOURCE_KIND Slot_Kind(RESOURCE_SLOT eSlot);
	static std::string Domain_FromRelativePath(
		const std::filesystem::path& EffectRelative);
	static const char* Type_Label(EFFECT_TYPE eType);
	static const char* Slot_Label(RESOURCE_SLOT eSlot);
	static const char* Slot_Description(RESOURCE_SLOT eSlot);

private:
	ComPtr<ID3D11Device> m_pDevice;
	ComPtr<ID3D11DeviceContext> m_pContext;
	shared_ptr<Engine::CShader> m_pModelShader;
	shared_ptr<Engine::CShader> m_pAnimModelShader;

	EFFECT_TYPE m_eType = EFFECT_TYPE::MESH;
	RESOURCE_SLOT m_eSelectedSlot = RESOURCE_SLOT::BASE;
	std::array<SLOT_BINDINGS, static_cast<size_t>(EFFECT_TYPE::END)> m_SlotBindings;

	bool_t m_bScanned = false;
	std::vector<RESOURCE_ENTRY> m_Resources;
	std::vector<std::string> m_Domains;
	std::vector<size_t> m_VisibleResources;
	RESOURCE_KIND m_eVisibleKind = RESOURCE_KIND::END;
	std::string m_strDomainFilter;
	char m_szNameFilter[128] = {};
	bool_t m_bVisibleDirty = true;
	std::unordered_map<std::string, TEXTURE_USAGE> m_TextureUsage;
	std::string m_strUsageStatus;

	std::unordered_map<std::string, PREVIEW_ENTRY> m_Previews;
	uint32_t m_iLoadsThisFrame = 0u;

	std::weak_ptr<CEffectV2Object> m_pPreview;
	EFFECT_TYPE m_ePreviewType = EFFECT_TYPE::MESH;
	bool_t m_bPreviewPrototypeRegistered = false;
	bool_t m_bTuningWindowOpen = false;
	std::string m_strPreviewStatus;

	char m_szEffectId[96] = {};
	std::vector<std::string> m_Documents;
	bool_t m_bDocumentsScanned = false;
	std::string m_strDocumentStatus;

	bool_t m_bAttachWindowOpen = false;
	std::weak_ptr<CNpc> m_pTarget;
	std::string m_strTargetArchetypeId;
	int32_t m_iTargetArchetypeSelection = -1;
	float3_t m_vTargetPosition = { 0.f, 0.f, 0.f };
	f32_t m_fTargetYawDegrees = 0.f;
	std::vector<std::string> m_TargetBoneNames;
	bool_t m_bTargetClipLoop = true;
	bool_t m_bRuntimeOnTarget = false;
	f32_t m_fTargetLastClipSeconds = -1.f;
	PIVOT_MODE m_ePivotMode = PIVOT_MODE::WORLD;
	PIVOT_ROTATION m_ePivotRotation = PIVOT_ROTATION::TARGET_YAW;
	std::string m_strPivotBone;
	int32_t m_iSpawnFrame = 0;
	std::vector<EFFECT_BINDING> m_Bindings;
	std::string m_strAttachStatus;

	std::string m_strStatus;
};

NS_END
