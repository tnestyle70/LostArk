#pragma once

#include "AnimationEffectCueDocument.h"
#include "Client_Defines.h"
#include "Effect_Catalog.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CCharacter;
class CEffectObject;

struct EFFECT_SPAWN_DESC final
{
    std::string strEffectAssetId;
    std::weak_ptr<CCharacter> pOwner;
    std::string strAnchorSlotId = "root";
    EFFECT_TRANSFORM_DESC LocalTransform{};
    EFFECT_FOLLOW_POLICY eFollowPolicy = EFFECT_FOLLOW_POLICY::FOLLOW;
    EFFECT_STOP_POLICY eStopPolicy = EFFECT_STOP_POLICY::NATURAL;
    uint32_t iCueDurationMs = 0u;
    uint32_t iActionStartTick = 0u;
    uint32_t iCueStartMs = 0u;
	std::string strOccurrenceId;
    f32_t fPlaybackRate = 1.f;
    f32_t fInitialSampleTimeSeconds = 0.f;
};

struct EFFECT_SOURCE_BONE_ANCHOR_BUILD_DESC final
{
	float4x4_t RawBone{};
	float4x4_t OwnerWorld{};
};

class CEffectPresentationService final
{
public:
	static bool_t Build_SourceBoneAnchorWorld(
		const EFFECT_SOURCE_BONE_ANCHOR_BUILD_DESC& Desc,
		float4x4_t& OutWorld);
    static bool_t Prepare_ProductCues(
        ComPtr<ID3D11Device> pDevice,
        ComPtr<ID3D11DeviceContext> pContext,
        const std::vector<ANIMATION_EFFECT_CUE>& Cues,
        std::string& strOutStatus);
    static bool_t Reprepare_ProductTargets(
        ComPtr<ID3D11Device> pDevice,
        ComPtr<ID3D11DeviceContext> pContext,
        const std::vector<std::string>& AdditionalEffectAssetIds,
        std::string& strOutStatus);
    static bool_t Spawn(
        const EFFECT_SPAWN_DESC& Desc,
        std::string& strOutStatus);
	/* Product cue requests can originate while Object Manager is iterating its
	   layer map.  Commit them only after Update_Engine finishes. */
	static void Commit_PendingSpawns();
	static bool_t Prepare_ReconstructedRuntimeProgram(
		const std::string& strEffectAssetId,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>&
			OutPreparation,
		std::string& strOutStatus);
	/* Debug/non-Product Artist F keeps its source runtime separate from Product
	   cue admission.  Preparation performs all document/resource work before an
	   authoritative action edge; Spawn only attaches the immutable result. */
	static bool_t Prepare_ReconstructedArtist31470(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		std::string& strOutStatus);
	static bool_t Acquire_ReconstructedArtist31470(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		std::shared_ptr<const EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>&
			OutPreparation,
		std::string& strOutStatus);
	static bool_t Stage_ReconstructedArtist31470Preview(
		const std::shared_ptr<CEffectObject>& pObject,
		const std::shared_ptr<const
			EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>& pExpectedPreparation,
		std::string& strOutStatus);
	static bool_t Spawn_ReconstructedArtist31470(
		const EFFECT_SPAWN_DESC& Desc,
		std::string& strOutStatus);
    static void Update(f32_t fTimeDelta);
    static void Synchronize_FollowAnchors();
    static void Stop_Owner(const std::shared_ptr<CCharacter>& pOwner);
    static void Clear_Level(uint32_t iLevelIndex);
    static void Clear_All();
    static void Release_PreparedResources();
    static const std::string& Get_Status();

private:
	static bool_t Spawn_Immediate(
		const EFFECT_SPAWN_DESC& Desc,
		std::string& strOutStatus);
};

NS_END

