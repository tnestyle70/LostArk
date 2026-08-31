#include "EstherCutinPresentationService.h"

#include "ActorCatalog.h"
#include "CombatHUDViewModel.h"
#include "DeferredMaterialRenderUtils.h"
#include "GameInstance.h"
#include "Model.h"
#include "NpcPresentationAssetService.h"
#include "Shader.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>

namespace
{
	/* Wall-clock seconds for the cutin's own animation advance -- product presentation must not
	depend on ImGui's frame clock (gameplay paths are ImGui-free). */
	f64_t Cutin_NowSeconds()
	{
		return std::chrono::duration_cast<std::chrono::duration<f64_t>>(
			std::chrono::steady_clock::now().time_since_epoch()).count();
	}
}

namespace
{
	/* ScreenCutin pass of the dedicated esther NPC shader, whose pass order is
	   pinned. The shared animated-mesh shader already shifted this index once
	   when an effect pass was inserted ahead of it. */
	constexpr uint32_t CUTIN_SHADER_PASS = 2u;

	constexpr f32_t CUTIN_RECT_X = 680.f;
	constexpr f32_t CUTIN_RECT_Y = 100.f;
	constexpr f32_t CUTIN_RECT_WIDTH = 800.f;
	constexpr f32_t CUTIN_RECT_HEIGHT = 800.f;

	/* User-tuned on 2026-08-21 with the F1 Esther Cutin (Debug) panel. */
	constexpr f32_t CUTIN_MODEL_YAW_DEGREES = 244.f;
	constexpr f32_t CUTIN_CAMERA_EYE_X_PER_HEIGHT = -0.65f;
	constexpr f32_t CUTIN_CAMERA_EYE_Y_PER_HEIGHT = -0.05f;
	constexpr f32_t CUTIN_CAMERA_DISTANCE_PER_HEIGHT = 1.06f;
	constexpr f32_t CUTIN_CAMERA_AT_Y_PER_HEIGHT = 0.52f;
	constexpr f32_t CUTIN_CAMERA_FOV_DEGREES = 39.f;
	constexpr f32_t CUTIN_CAMERA_NEAR = 0.1f;
	constexpr f32_t CUTIN_CAMERA_FAR = 60.f;
	constexpr f32_t CUTIN_FALLBACK_MODEL_HEIGHT = 2.2f;
	constexpr f32_t CUTIN_MODEL_HEIGHT_MARGIN = 1.12f;
	constexpr f32_t CUTIN_MODEL_HEIGHT_MINIMUM = 1.8f;
	constexpr f32_t CUTIN_MODEL_HEIGHT_MAXIMUM = 12.f;

	constexpr f32_t CUTIN_MAX_STEP_SECONDS = 0.1f;

	constexpr const char_t* CUTIN_ROOT_MOTION_BONE = "b_root";
	constexpr int32_t CUTIN_ROOT_MOTION_LOCK_ALL_AXES = -1;

#ifdef _DEBUG
	constexpr Client::ESTHER_CUTIN_TUNING CUTIN_TUNING_DEFAULTS
	{
		CUTIN_RECT_X,
		CUTIN_RECT_Y,
		CUTIN_RECT_WIDTH,
		CUTIN_RECT_HEIGHT,
		CUTIN_MODEL_YAW_DEGREES,
		CUTIN_CAMERA_EYE_X_PER_HEIGHT,
		CUTIN_CAMERA_EYE_Y_PER_HEIGHT,
		CUTIN_CAMERA_DISTANCE_PER_HEIGHT,
		CUTIN_CAMERA_AT_Y_PER_HEIGHT,
		CUTIN_CAMERA_FOV_DEGREES,
	};
	Client::ESTHER_CUTIN_TUNING g_Tuning = CUTIN_TUNING_DEFAULTS;
#endif

	struct CUTIN_STATE
	{
		shared_ptr<Engine::CModel> pModel;
		shared_ptr<Engine::CShader> pShader;
		std::vector<std::string> Clips;
		size_t iClipIndex = 0u;
		uint32_t iConsumedGeneration = 0;
		uint32_t iActiveLevelIndex = 0;
		uint32_t iWindowStartMs = 0;
		uint32_t iWindowEndMs = 0;
		f64_t dLastAdvanceSeconds = -1.0;
		f32_t fElapsedSeconds = 0.f;
		f32_t fModelHeight = CUTIN_FALLBACK_MODEL_HEIGHT;
		bool_t isActive = false;
	};
	CUTIN_STATE g_Cutin;

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
			return CUTIN_FALLBACK_MODEL_HEIGHT;
		return std::clamp(
			maxY * CUTIN_MODEL_HEIGHT_MARGIN,
			CUTIN_MODEL_HEIGHT_MINIMUM,
			CUTIN_MODEL_HEIGHT_MAXIMUM);
	}

	void End_Cutin()
	{
		g_Cutin.pModel.reset();
		g_Cutin.pShader.reset();
		g_Cutin.Clips.clear();
		g_Cutin.iClipIndex = 0u;
		g_Cutin.iWindowStartMs = 0u;
		g_Cutin.iWindowEndMs = 0u;
		g_Cutin.dLastAdvanceSeconds = -1.0;
		g_Cutin.fElapsedSeconds = 0.f;
		g_Cutin.isActive = false;
	}

	HRESULT Begin_Cutin()
	{
		const Client::HUD_ESTHER_CUTIN_REQUEST& request =
			Client::CCombatHUDViewModel::Get().Get_EstherCutinRequest();
		if (request.strArchetypeId.empty() || request.Clips.empty() ||
			request.Clips.front().empty())
			return E_FAIL;

		const uint32_t iLevelIndex =
			CGameInstance::Get().Get_CurrentLevelID();
		const wstring_t modelTag =
			Client::CNpcPresentationAssetService::Get_ModelPrototypeTag(
				request.strArchetypeId);
		if (modelTag.empty())
			return E_FAIL;

		shared_ptr<Engine::CShader> stagedShader =
			dynamic_pointer_cast<Engine::CShader>(
				CGameInstance::Get().Clone_Prototype(
					iLevelIndex,
					TEXT("Prototype_Component_Shader_VtxEstherNpc")));
		shared_ptr<Engine::CModel> stagedModel =
			dynamic_pointer_cast<Engine::CModel>(
				CGameInstance::Get().Clone_Prototype(
					iLevelIndex, modelTag));
		if (nullptr == stagedShader || nullptr == stagedModel)
			return E_FAIL;

		const Client::NPC_ACTOR_ENTRY* pActor =
			Client::CActorCatalog::Find_Npc(request.strArchetypeId);
		f32_t modelHeight = CUTIN_FALLBACK_MODEL_HEIGHT;
		if (nullptr != pActor &&
			stagedModel->Set_Animation(pActor->idleClip.c_str(), false))
		{
			stagedModel->Play_Animation(0.f);
			modelHeight = Measure_PoseHeight(*stagedModel);
		}
		if (!stagedModel->Set_Animation(request.Clips.front().c_str(), false))
			return E_FAIL;
		stagedModel->Enable_RootMotionSuppression(
			CUTIN_ROOT_MOTION_BONE, CUTIN_ROOT_MOTION_LOCK_ALL_AXES);
		stagedModel->Play_Animation(0.f);

		g_Cutin.pShader = std::move(stagedShader);
		g_Cutin.pModel = std::move(stagedModel);
		g_Cutin.Clips = request.Clips;
		g_Cutin.iClipIndex = 0u;
		g_Cutin.iWindowStartMs = nullptr != pActor ? pActor->cutinStartMs : 0u;
		g_Cutin.iWindowEndMs = nullptr != pActor ? pActor->cutinEndMs : 0u;
		g_Cutin.fModelHeight = modelHeight;
		g_Cutin.iActiveLevelIndex = iLevelIndex;
		g_Cutin.dLastAdvanceSeconds = Cutin_NowSeconds();
		g_Cutin.fElapsedSeconds = 0.f;
		g_Cutin.isActive = true;
		return S_OK;
	}

	void Consume_Request()
	{
		const Client::HUD_ESTHER_CUTIN_REQUEST& request =
			Client::CCombatHUDViewModel::Get().Get_EstherCutinRequest();
		if (request.iGeneration == g_Cutin.iConsumedGeneration)
			return;

		g_Cutin.iConsumedGeneration = request.iGeneration;
		End_Cutin();
		if (0u != request.iGeneration && FAILED(Begin_Cutin()))
			End_Cutin();
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
		const f32_t rectX = CUTIN_RECT_X;
		const f32_t rectY = CUTIN_RECT_Y;
		const f32_t rectWidth = CUTIN_RECT_WIDTH;
		const f32_t rectHeight = CUTIN_RECT_HEIGHT;
		const f32_t modelYawDegrees = CUTIN_MODEL_YAW_DEGREES;
		const f32_t eyeXPerHeight = CUTIN_CAMERA_EYE_X_PER_HEIGHT;
		const f32_t eyeYPerHeight = CUTIN_CAMERA_EYE_Y_PER_HEIGHT;
		const f32_t distancePerHeight = CUTIN_CAMERA_DISTANCE_PER_HEIGHT;
		const f32_t atYPerHeight = CUTIN_CAMERA_AT_Y_PER_HEIGHT;
		const f32_t fovDegrees = CUTIN_CAMERA_FOV_DEGREES;
#endif
		const float2_t viewportSize =
			CGameInstance::Get().Get_ViewportSize();
		if (viewportSize.x <= 0.f || viewportSize.y <= 0.f)
			return S_FALSE;

		const f32_t scaleX = viewportSize.x / 1280.f;
		const f32_t scaleY = viewportSize.y / 720.f;

		D3D11_VIEWPORT cutinViewport{};
		cutinViewport.TopLeftX = rectX * scaleX;
		cutinViewport.TopLeftY = rectY * scaleY;
		cutinViewport.Width = rectWidth * scaleX;
		cutinViewport.Height = rectHeight * scaleY;
		cutinViewport.MinDepth = 0.f;
		cutinViewport.MaxDepth = 1.f;
		if (cutinViewport.Width <= 0.f || cutinViewport.Height <= 0.f)
			return S_FALSE;

		std::array<D3D11_VIEWPORT,
			D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE>
			previousViewports{};
		uint32_t previousViewportCount =
			static_cast<uint32_t>(previousViewports.size());
		pContext->RSGetViewports(
			&previousViewportCount, previousViewports.data());

		ComPtr<ID3D11RenderTargetView> currentRTV;
		ComPtr<ID3D11DepthStencilView> currentDSV;
		pContext->OMGetRenderTargets(1, &currentRTV, &currentDSV);
		if (nullptr != currentDSV)
		{
			pContext->ClearDepthStencilView(
				currentDSV.Get(), D3D11_CLEAR_DEPTH, 1.f, 0);
		}

		pContext->RSSetViewports(1, &cutinViewport);

		const HRESULT result = [&]() -> HRESULT
		{
			const matrix_t worldMatrix = XMMatrixRotationY(
				XMConvertToRadians(modelYawDegrees));
			const f32_t height = g_Cutin.fModelHeight;
			const vector_t eye = XMVectorSet(
				eyeXPerHeight * height,
				eyeYPerHeight * height,
				-distancePerHeight * height,
				1.f);
			const vector_t at = XMVectorSet(
				0.f,
				atYPerHeight * height,
				0.f,
				1.f);
			const matrix_t viewMatrix = XMMatrixLookAtLH(
				eye, at, XMVectorSet(0.f, 1.f, 0.f, 0.f));
			const f32_t aspect =
				cutinViewport.Width / cutinViewport.Height;
			const matrix_t projectionMatrix = XMMatrixPerspectiveFovLH(
				XMConvertToRadians(fovDegrees),
				aspect,
				CUTIN_CAMERA_NEAR,
				CUTIN_CAMERA_FAR);

			float4x4_t world{};
			float4x4_t view{};
			float4x4_t projection{};
			XMStoreFloat4x4(&world, worldMatrix);
			XMStoreFloat4x4(&view, viewMatrix);
			XMStoreFloat4x4(&projection, projectionMatrix);

			if (FAILED(g_Cutin.pShader->Bind_Matrix(
					"g_WorldMatrix", &world)) ||
				FAILED(g_Cutin.pShader->Bind_Matrix(
					"g_ViewMatrix", &view)) ||
				FAILED(g_Cutin.pShader->Bind_Matrix(
					"g_ProjMatrix", &projection)))
				return E_FAIL;

			const uint32_t iNumMeshes = g_Cutin.pModel->Get_NumMeshes();
			for (uint32_t i = 0; i < iNumMeshes; ++i)
			{
				if (FAILED(Client::Bind_DeferredMaterialInputs(
						*g_Cutin.pModel, g_Cutin.pShader, i, {}, nullptr)) ||
					FAILED(g_Cutin.pModel->Bind_BoneMatrices(
						g_Cutin.pShader, "g_BoneMatrices", i)) ||
					FAILED(g_Cutin.pShader->Begin(CUTIN_SHADER_PASS)) ||
					FAILED(g_Cutin.pModel->Render(i)))
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

HRESULT Client::CEstherCutinPresentationService::Render(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	if (nullptr == pDevice || nullptr == pContext)
		return E_INVALIDARG;

	Consume_Request();
	if (!g_Cutin.isActive)
		return S_FALSE;

	if (CGameInstance::Get().Get_CurrentLevelID() !=
		g_Cutin.iActiveLevelIndex)
	{
		End_Cutin();
		return S_FALSE;
	}

	const f64_t dNowSeconds = Cutin_NowSeconds();
	const f32_t fTimeDelta = std::clamp(
		static_cast<f32_t>(dNowSeconds - g_Cutin.dLastAdvanceSeconds),
		0.f,
		CUTIN_MAX_STEP_SECONDS);
	g_Cutin.dLastAdvanceSeconds = dNowSeconds;
	g_Cutin.fElapsedSeconds += fTimeDelta;
	if (g_Cutin.pModel->Play_Animation(fTimeDelta))
	{
		if (g_Cutin.iClipIndex + 1u >= g_Cutin.Clips.size() ||
			!g_Cutin.pModel->Set_Animation(
				g_Cutin.Clips[g_Cutin.iClipIndex + 1u].c_str(), false))
		{
			End_Cutin();
			return S_FALSE;
		}
		++g_Cutin.iClipIndex;
		g_Cutin.pModel->Play_Animation(0.f);
	}

	/* The clip clock keeps running outside the visibility window so the
	corner model stays in sync with the world summon; only drawing is
	gated. */
	const f32_t elapsedMs = g_Cutin.fElapsedSeconds * 1000.f;
	if (0u != g_Cutin.iWindowEndMs &&
		elapsedMs >= static_cast<f32_t>(g_Cutin.iWindowEndMs))
	{
		End_Cutin();
		return S_FALSE;
	}
	if (elapsedMs < static_cast<f32_t>(g_Cutin.iWindowStartMs))
		return S_FALSE;

	const HRESULT result = Draw_Model(pContext);
	if (FAILED(result))
	{
		End_Cutin();
		return result;
	}
	return S_OK;
}

#ifdef _DEBUG
Client::ESTHER_CUTIN_TUNING&
Client::CEstherCutinPresentationService::Debug_Tuning()
{
	return g_Tuning;
}

void Client::CEstherCutinPresentationService::Debug_ResetTuning()
{
	g_Tuning = CUTIN_TUNING_DEFAULTS;
}

bool_t Client::CEstherCutinPresentationService::Debug_Preview(
	const std::string& archetypeId)
{
	const Client::NPC_ACTOR_ENTRY* pActor =
		Client::CActorCatalog::Find_Npc(archetypeId);
	if (nullptr == pActor)
		return false;
	const auto chain = pActor->actionClips.find("esther.strike");
	if (chain == pActor->actionClips.end())
		return false;
	Client::CCombatHUDViewModel::Get().Apply_EstherCutinAction(
		archetypeId, chain->second);
	return true;
}
#endif
