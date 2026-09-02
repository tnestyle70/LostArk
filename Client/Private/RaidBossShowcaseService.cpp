#include "RaidBossShowcaseService.h"

#include "DeferredMaterialRenderUtils.h"
#include "GameInstance.h"
#include "Model.h"
#include "Shader.h"
#include "ValtanPresentationAssetService.h"

#include <array>
#include <chrono>
#include <cmath>
#include <string>

namespace
{
	/* Wall-clock advance, same reasoning as the esther cutin: this draws inside a popup that
	   can be open while gameplay timers are gated, and product presentation must not depend
	   on ImGui's frame clock. */
	f64_t Showcase_NowSeconds()
	{
		return std::chrono::duration_cast<std::chrono::duration<f64_t>>(
			std::chrono::steady_clock::now().time_since_epoch()).count();
	}

	/* Shader_VtxAnimMeshBinary's forward pass. Its pass order is pinned by every consumer
	   indexing into it; ScreenCutin sits last today (Default, Shadow, the three effect
	   model-cue passes, then this). */
	constexpr uint32_t SHOWCASE_SHADER_PASS = 5u;

	constexpr const char_t* SHOWCASE_IDLE_CLIP = "mesh_idle_battle_1";
	constexpr const char_t* SHOWCASE_ROOT_MOTION_BONE = "b_root";
	constexpr int32_t SHOWCASE_ROOT_MOTION_LOCK_ALL_AXES = -1;

	/* 1280x720-reference rect over the popup's boss portrait area (the portrait slot spans
	   (274,-48) 801x562; this sits inside its on-screen part, centred on the popup spine). */
	constexpr f32_t SHOWCASE_RECT_X = 464.f;
	constexpr f32_t SHOWCASE_RECT_Y = 20.f;
	constexpr f32_t SHOWCASE_RECT_WIDTH = 420.f;
	constexpr f32_t SHOWCASE_RECT_HEIGHT = 500.f;

	/* Starting values: the esther cutin's user-tuned per-height camera, with the yaw moved
	   by the -90 the Valtan body applies to face the engine's LOOK axis. All of these are
	   expected to be retuned by eye through the debug struct. */
	constexpr f32_t SHOWCASE_MODEL_YAW_DEGREES = 154.f;
	constexpr f32_t SHOWCASE_EYE_X_PER_HEIGHT = -0.35f;
	constexpr f32_t SHOWCASE_EYE_Y_PER_HEIGHT = 0.10f;
	constexpr f32_t SHOWCASE_DISTANCE_PER_HEIGHT = 1.05f;
	constexpr f32_t SHOWCASE_AT_Y_PER_HEIGHT = 0.48f;
	constexpr f32_t SHOWCASE_FOV_DEGREES = 39.f;
	constexpr f32_t SHOWCASE_CAMERA_NEAR = 0.1f;
	constexpr f32_t SHOWCASE_CAMERA_FAR = 120.f;

	constexpr f32_t SHOWCASE_FALLBACK_MODEL_HEIGHT = 6.f;
	constexpr f32_t SHOWCASE_MODEL_HEIGHT_MARGIN = 1.08f;
	constexpr f32_t SHOWCASE_MODEL_HEIGHT_MINIMUM = 2.f;
	constexpr f32_t SHOWCASE_MODEL_HEIGHT_MAXIMUM = 16.f;

	constexpr f32_t SHOWCASE_MAX_STEP_SECONDS = 0.1f;

#ifdef _DEBUG
	constexpr Client::RAID_BOSS_SHOWCASE_TUNING SHOWCASE_TUNING_DEFAULTS
	{
		SHOWCASE_RECT_X,
		SHOWCASE_RECT_Y,
		SHOWCASE_RECT_WIDTH,
		SHOWCASE_RECT_HEIGHT,
		SHOWCASE_MODEL_YAW_DEGREES,
		SHOWCASE_EYE_X_PER_HEIGHT,
		SHOWCASE_EYE_Y_PER_HEIGHT,
		SHOWCASE_DISTANCE_PER_HEIGHT,
		SHOWCASE_AT_Y_PER_HEIGHT,
		SHOWCASE_FOV_DEGREES,
	};
	Client::RAID_BOSS_SHOWCASE_TUNING g_Tuning = SHOWCASE_TUNING_DEFAULTS;
#endif

	struct SHOWCASE_STATE
	{
		shared_ptr<Engine::CModel> pModel;
		shared_ptr<Engine::CShader> pShader;
		std::string strArchetypeId;
		uint32_t iStagedLevelIndex = 0;
		f64_t dLastAdvanceSeconds = -1.0;
		f32_t fModelHeight = SHOWCASE_FALLBACK_MODEL_HEIGHT;
		/* Set by Request_Frame, consumed by Render -- the popup asks per frame and silence
		   means closed, so no explicit hide call can be missed. */
		bool_t wantsFrame = false;
		std::string strWantedArchetypeId;
		std::string strPrewarmArchetypeId;
	};
	SHOWCASE_STATE g_Showcase;

	f32_t Measure_PoseHeight(Engine::CModel& model)
	{
		f32_t maxY = 0.f;
		for (uint32_t iBone = 0; ; ++iBone)
		{
			matrix_t combined;
			if (!model.Get_BoneCombinedMatrix(iBone, combined))
				break;
			const f32_t boneY = XMVectorGetY(combined.r[3]);
			if (std::isfinite(boneY))
				maxY = (std::max)(maxY, boneY);
		}
		if (maxY <= 0.f || !std::isfinite(maxY))
			return SHOWCASE_FALLBACK_MODEL_HEIGHT;
		return std::clamp(
			maxY * SHOWCASE_MODEL_HEIGHT_MARGIN,
			SHOWCASE_MODEL_HEIGHT_MINIMUM,
			SHOWCASE_MODEL_HEIGHT_MAXIMUM);
	}

	void End_Showcase()
	{
		g_Showcase.pModel.reset();
		g_Showcase.pShader.reset();
		g_Showcase.strArchetypeId.clear();
		g_Showcase.iStagedLevelIndex = 0;
		g_Showcase.dLastAdvanceSeconds = -1.0;
		g_Showcase.fModelHeight = SHOWCASE_FALLBACK_MODEL_HEIGHT;
	}

	HRESULT Begin_Showcase(
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext,
		const std::string& archetypeId)
	{
		const uint32_t iLevelIndex = CGameInstance::Get().Get_CurrentLevelID();

		/* The sanctioned lazy admission -- the same call the replication path and the tools
		   use. Idempotent once committed; until it succeeds the popup keeps its static art. */
		if (FAILED(Client::CValtanPresentationAssetService::Ensure_Prototypes(
				pDevice, pContext, iLevelIndex, archetypeId)) ||
			!Client::CValtanPresentationAssetService::Is_Ready(iLevelIndex, archetypeId))
			return E_FAIL;

		const wstring_t modelTag =
			Client::CValtanPresentationAssetService::Get_BodyModelPrototypeTag(archetypeId);
		if (modelTag.empty())
			return E_FAIL;

		shared_ptr<Engine::CShader> stagedShader =
			dynamic_pointer_cast<Engine::CShader>(
				CGameInstance::Get().Clone_Prototype(
					iLevelIndex,
					TEXT("Prototype_Component_Shader_VtxAnimMeshBinary")));
		shared_ptr<Engine::CModel> stagedModel =
			dynamic_pointer_cast<Engine::CModel>(
				CGameInstance::Get().Clone_Prototype(iLevelIndex, modelTag));
		if (nullptr == stagedShader || nullptr == stagedModel)
			return E_FAIL;

		if (!stagedModel->Set_Animation(SHOWCASE_IDLE_CLIP, true))
			return E_FAIL;
		stagedModel->Enable_RootMotionSuppression(
			SHOWCASE_ROOT_MOTION_BONE, SHOWCASE_ROOT_MOTION_LOCK_ALL_AXES);
		stagedModel->Play_Animation(0.f);

		g_Showcase.pShader = std::move(stagedShader);
		g_Showcase.pModel = std::move(stagedModel);
		g_Showcase.fModelHeight = Measure_PoseHeight(*g_Showcase.pModel);
		g_Showcase.strArchetypeId = archetypeId;
		g_Showcase.iStagedLevelIndex = iLevelIndex;
		g_Showcase.dLastAdvanceSeconds = Showcase_NowSeconds();
		return S_OK;
	}

	HRESULT Draw_Model(const ComPtr<ID3D11DeviceContext>& pContext)
	{
#ifdef _DEBUG
		const f32_t rectX = g_Tuning.fRectX;
		const f32_t rectY = g_Tuning.fRectY;
		const f32_t rectWidth = g_Tuning.fRectWidth;
		const f32_t rectHeight = g_Tuning.fRectHeight;
		const f32_t modelYawDegrees = g_Tuning.fModelYawDegrees;
		const f32_t eyeXPerHeight = g_Tuning.fEyeXPerHeight;
		const f32_t eyeYPerHeight = g_Tuning.fEyeYPerHeight;
		const f32_t distancePerHeight = g_Tuning.fDistancePerHeight;
		const f32_t atYPerHeight = g_Tuning.fAtYPerHeight;
		const f32_t fovDegrees = g_Tuning.fFovDegrees;
#else
		const f32_t rectX = SHOWCASE_RECT_X;
		const f32_t rectY = SHOWCASE_RECT_Y;
		const f32_t rectWidth = SHOWCASE_RECT_WIDTH;
		const f32_t rectHeight = SHOWCASE_RECT_HEIGHT;
		const f32_t modelYawDegrees = SHOWCASE_MODEL_YAW_DEGREES;
		const f32_t eyeXPerHeight = SHOWCASE_EYE_X_PER_HEIGHT;
		const f32_t eyeYPerHeight = SHOWCASE_EYE_Y_PER_HEIGHT;
		const f32_t distancePerHeight = SHOWCASE_DISTANCE_PER_HEIGHT;
		const f32_t atYPerHeight = SHOWCASE_AT_Y_PER_HEIGHT;
		const f32_t fovDegrees = SHOWCASE_FOV_DEGREES;
#endif
		const float2_t viewportSize = CGameInstance::Get().Get_ViewportSize();
		if (viewportSize.x <= 0.f || viewportSize.y <= 0.f)
			return S_FALSE;

		const f32_t scaleX = viewportSize.x / 1280.f;
		const f32_t scaleY = viewportSize.y / 720.f;

		D3D11_VIEWPORT showcaseViewport{};
		showcaseViewport.TopLeftX = rectX * scaleX;
		showcaseViewport.TopLeftY = rectY * scaleY;
		showcaseViewport.Width = rectWidth * scaleX;
		showcaseViewport.Height = rectHeight * scaleY;
		showcaseViewport.MinDepth = 0.f;
		showcaseViewport.MaxDepth = 1.f;
		if (showcaseViewport.Width <= 0.f || showcaseViewport.Height <= 0.f)
			return S_FALSE;

		std::array<D3D11_VIEWPORT,
			D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE>
			previousViewports{};
		uint32_t previousViewportCount =
			static_cast<uint32_t>(previousViewports.size());
		pContext->RSGetViewports(
			&previousViewportCount, previousViewports.data());

		/* The scene and UI are already down; depth is cleared so the boss draws over them
		   inside its own viewport, the same way the esther cutin claims its rect. */
		ComPtr<ID3D11RenderTargetView> currentRTV;
		ComPtr<ID3D11DepthStencilView> currentDSV;
		pContext->OMGetRenderTargets(1, &currentRTV, &currentDSV);
		if (nullptr != currentDSV)
		{
			pContext->ClearDepthStencilView(
				currentDSV.Get(), D3D11_CLEAR_DEPTH, 1.f, 0);
		}

		pContext->RSSetViewports(1, &showcaseViewport);

		const HRESULT result = [&]() -> HRESULT
		{
			const matrix_t worldMatrix = XMMatrixRotationY(
				XMConvertToRadians(modelYawDegrees));
			const f32_t height = g_Showcase.fModelHeight;
			const vector_t eye = XMVectorSet(
				eyeXPerHeight * height,
				eyeYPerHeight * height,
				-distancePerHeight * height,
				1.f);
			const vector_t at = XMVectorSet(
				0.f, atYPerHeight * height, 0.f, 1.f);
			const matrix_t viewMatrix = XMMatrixLookAtLH(
				eye, at, XMVectorSet(0.f, 1.f, 0.f, 0.f));
			const f32_t aspect =
				showcaseViewport.Width / showcaseViewport.Height;
			const matrix_t projectionMatrix = XMMatrixPerspectiveFovLH(
				XMConvertToRadians(fovDegrees),
				aspect,
				SHOWCASE_CAMERA_NEAR,
				SHOWCASE_CAMERA_FAR);

			float4x4_t world{};
			float4x4_t view{};
			float4x4_t projection{};
			XMStoreFloat4x4(&world, worldMatrix);
			XMStoreFloat4x4(&view, viewMatrix);
			XMStoreFloat4x4(&projection, projectionMatrix);

			if (FAILED(g_Showcase.pShader->Bind_Matrix(
					"g_WorldMatrix", &world)) ||
				FAILED(g_Showcase.pShader->Bind_Matrix(
					"g_ViewMatrix", &view)) ||
				FAILED(g_Showcase.pShader->Bind_Matrix(
					"g_ProjMatrix", &projection)))
				return E_FAIL;

			const uint32_t iNumMeshes = g_Showcase.pModel->Get_NumMeshes();
			for (uint32_t i = 0; i < iNumMeshes; ++i)
			{
				const Client::DEFERRED_MATERIAL_PROFILE Profile =
					Client::Resolve_DeferredMaterialProfile(
						"material.valtan.monster-base.v1",
						g_Showcase.pModel->Get_MaterialName(i));
				if (FAILED(Client::Bind_DeferredMaterialInputs(
						*g_Showcase.pModel, g_Showcase.pShader, i, Profile,
						nullptr)) ||
					FAILED(g_Showcase.pModel->Bind_BoneMatrices(
						g_Showcase.pShader, "g_BoneMatrices", i)) ||
					FAILED(g_Showcase.pShader->Begin(SHOWCASE_SHADER_PASS)) ||
					FAILED(g_Showcase.pModel->Render(i)))
					return E_FAIL;
			}
			return S_OK;
		}();

		if (0u != previousViewportCount)
		{
			pContext->RSSetViewports(
				previousViewportCount, previousViewports.data());
		}

		return result;
	}
}

bool_t Client::CRaidBossShowcaseService::Is_Live()
{
	return nullptr != g_Showcase.pModel;
}

void Client::CRaidBossShowcaseService::Request_Prewarm(std::string_view archetypeId)
{
	if (!archetypeId.empty())
		g_Showcase.strPrewarmArchetypeId.assign(archetypeId);
}

void Client::CRaidBossShowcaseService::Request_Frame(std::string_view archetypeId)
{
	if (archetypeId.empty())
		return;
	g_Showcase.wantsFrame = true;
	g_Showcase.strWantedArchetypeId.assign(archetypeId);
}

HRESULT Client::CRaidBossShowcaseService::Render(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	if (nullptr == pDevice || nullptr == pContext)
		return E_INVALIDARG;

	const bool_t wantsFrame = g_Showcase.wantsFrame;
	const std::string wantedArchetypeId = g_Showcase.strWantedArchetypeId;
	g_Showcase.wantsFrame = false;

	if (!wantsFrame)
	{
		if (nullptr != g_Showcase.pModel)
			End_Showcase();
		/* The popup just opened: pay the one synchronous admission now, hidden behind the
		   open transition, instead of on the first click of the boss's tab. */
		if (!g_Showcase.strPrewarmArchetypeId.empty())
		{
			(void)Client::CValtanPresentationAssetService::Ensure_Prototypes(
				pDevice, pContext,
				CGameInstance::Get().Get_CurrentLevelID(),
				g_Showcase.strPrewarmArchetypeId);
			g_Showcase.strPrewarmArchetypeId.clear();
		}
		return S_FALSE;
	}
	g_Showcase.strPrewarmArchetypeId.clear();

	/* Level changes retire the prototypes this cloned from; a stale clone must not outlive
	   them, and a changed request restages. */
	const uint32_t iLevelIndex = CGameInstance::Get().Get_CurrentLevelID();
	if (nullptr != g_Showcase.pModel &&
		(g_Showcase.iStagedLevelIndex != iLevelIndex ||
		 g_Showcase.strArchetypeId != wantedArchetypeId))
		End_Showcase();

	if (nullptr == g_Showcase.pModel &&
		FAILED(Begin_Showcase(pDevice, pContext, wantedArchetypeId)))
	{
		End_Showcase();
		return S_FALSE;
	}

	const f64_t now = Showcase_NowSeconds();
	if (g_Showcase.dLastAdvanceSeconds >= 0.0)
	{
		const f32_t step = std::clamp(
			static_cast<f32_t>(now - g_Showcase.dLastAdvanceSeconds),
			0.f, SHOWCASE_MAX_STEP_SECONDS);
		g_Showcase.pModel->Update_Animation(step);
	}
	g_Showcase.dLastAdvanceSeconds = now;

	return Draw_Model(pContext);
}

#ifdef _DEBUG
Client::RAID_BOSS_SHOWCASE_TUNING& Client::CRaidBossShowcaseService::Debug_Tuning()
{
	return g_Tuning;
}

void Client::CRaidBossShowcaseService::Debug_ResetTuning()
{
	g_Tuning = SHOWCASE_TUNING_DEFAULTS;
}
#endif
