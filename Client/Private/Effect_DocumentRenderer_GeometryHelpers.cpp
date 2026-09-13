#include "Effect_DocumentRenderer_Internal.h"
#include "GameInstance.h"
#include <d3d11sdklayers.h>
#include <algorithm>
#include <bit>
#include <cctype>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <limits>
#include <optional>
#include <span>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "Model.h"
#include "Shader.h"
#include "Engine_RenderTypes.h"
#include "VIBuffer_DynamicTrail.h"
#include "VIBuffer_ParticleRect.h"
#include "VIBuffer_Rect.h"

namespace EffectDocumentRendererDetail
{


	float3_t To_Float3(const vector_t Value)
	{
		float3_t Result{};
		XMStoreFloat3(&Result, Value);
		return Result;
	}

	bool_t Normalize_Safe(const vector_t Value, vector_t& Out)
	{
		const f32_t LengthSquared = XMVectorGetX(XMVector3LengthSq(Value));
		if (!std::isfinite(LengthSquared) || LengthSquared <= 1.e-8f)
			return false;
		Out = XMVector3Normalize(Value);
		return true;
	}

	Client::EFFECT_COLOR_DESC Evaluate_CommonColor(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const f32_t fNormalizedLife)
	{
		Client::EFFECT_COLOR_DESC Color = Element.Detail.Color;
		const Client::EFFECT_LINEAR_LERP_DESC& Lerp =
			Element.Detail.LinearLerp;
		const f32_t T = std::clamp(fNormalizedLife, 0.f, 1.f);
		auto LerpValue = [T](const f32_t A, const f32_t B)
		{
			return A + (B - A) * T;
		};
		if (Lerp.bColorOffset)
		{
			Color.vColorOffset = {
				LerpValue(Color.vColorOffset.x, Lerp.vEndColorOffset.x),
				LerpValue(Color.vColorOffset.y, Lerp.vEndColorOffset.y),
				LerpValue(Color.vColorOffset.z, Lerp.vEndColorOffset.z),
				LerpValue(Color.vColorOffset.w, Lerp.vEndColorOffset.w) };
		}
		if (Lerp.bColorMultiply)
		{
			Color.vColorMultiply = {
				LerpValue(Color.vColorMultiply.x, Lerp.vEndColorMultiply.x),
				LerpValue(Color.vColorMultiply.y, Lerp.vEndColorMultiply.y),
				LerpValue(Color.vColorMultiply.z, Lerp.vEndColorMultiply.z),
				LerpValue(Color.vColorMultiply.w, Lerp.vEndColorMultiply.w) };
		}
		if (Lerp.bEmissiveIntensity)
		{
			Color.fEmissiveIntensity = LerpValue(
				Color.fEmissiveIntensity, Lerp.fEndEmissiveIntensity);
		}
		return Color;
	}

	float4x4_t Make_BillboardWorld(
		const float4x4_t& Source,
		const f32_t fRollDegrees)
	{
		vector_t Scale;
		vector_t Rotation;
		vector_t Translation;
		if (!XMMatrixDecompose(&Scale, &Rotation, &Translation,
			XMLoadFloat4x4(&Source)))
		{
			return Source;
		}
		matrix_t CameraWorld = XMLoadFloat4x4(
			CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW));
		CameraWorld.r[3] = XMVectorSet(0.f, 0.f, 0.f, 1.f);
		float4x4_t Result{};
		XMStoreFloat4x4(&Result,
			XMMatrixScalingFromVector(Scale) *
			XMMatrixRotationZ(XMConvertToRadians(fRollDegrees)) * CameraWorld *
			XMMatrixTranslationFromVector(Translation));
		return Result;
	}

	bool_t Make_ParticleSpriteWorld(
		const Client::EFFECT_EVALUATED_PARTICLE& Particle,
		float4x4_t& OutWorld,
		const uint32_t iSourceMaterialProfile,
		PARTICLE_SPRITE_WORLD_CONTEXT& Context)
	{
		float4x4_t Source = Particle.World;
		// The draw batch cannot change the camera. Keep this snapshot local to
		// the batch so another view/pass always captures its own camera.
		if (!Context.bCameraCaptured)
		{
			Context.CameraWorld = *CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW);
			Context.bCameraCaptured = true;
		}
		const matrix_t CameraWorldWithTranslation = XMLoadFloat4x4(&Context.CameraWorld);
		XMStoreFloat4x4(&Source,
			XMLoadFloat4x4(&Source) * XMMatrixTranslationFromVector(
				XMVectorScale(CameraWorldWithTranslation.r[2],
					Particle.fCameraOffset)));
		vector_t Scale;
		vector_t Rotation;
		vector_t Translation;
		if (!XMMatrixDecompose(&Scale, &Rotation, &Translation,
			XMLoadFloat4x4(&Source)))
		{
			return false;
		}
		const float3_t DecomposedMagnitude = {
			std::abs(XMVectorGetX(Scale)),
			std::abs(XMVectorGetY(Scale)),
			std::abs(XMVectorGetZ(Scale))
		};
		Client::EFFECT_PARTICLE_SPRITE_SCALE_DESC ResolvedScale;
		if (!Client::CEffectDocumentRenderer::Resolve_ParticleSpriteScale(
			Particle, DecomposedMagnitude, ResolvedScale))
		{
			return false;
		}
		Scale = XMVectorSet(
			ResolvedScale.vScale.x, ResolvedScale.vScale.y,
			ResolvedScale.vScale.z, 0.f);
		matrix_t CameraWorld = CameraWorldWithTranslation;
		CameraWorld.r[3] = XMVectorSet(0.f, 0.f, 0.f, 1.f);
		matrix_t Orientation = CameraWorld;
		const f32_t fRoll = XMConvertToRadians(
			Particle.fSpriteRotationDegrees);
		switch (Particle.eSpriteAlignment)
		{
		case Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::CAMERA_SQUARE:
		{
			Orientation = XMMatrixRotationZ(fRoll) * CameraWorld;
			break;
		}
		case Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::CAMERA_VELOCITY:
		{
			const vector_t Velocity = XMLoadFloat3(&Particle.vWorldVelocity);
			// Native-family lookup is invariant for particles from this owner.
			// A mixed-owner batch still re-evaluates the exact original predicate.
			if (!Context.bNativeVelocityResolved ||
				Context.pNativeVelocityElement != Particle.pElement ||
				Context.iNativeVelocityProfile != iSourceMaterialProfile)
			{
				Context.bNativeVelocityBasis = Particle.pElement &&
					Particle.pElement->SourceRecipe.bEnabled &&
					(iSourceMaterialProfile == 43u || iSourceMaterialProfile == 45u ||
					 iSourceMaterialProfile == 46u || iSourceMaterialProfile == 47u ||
					 iSourceMaterialProfile == 48u || iSourceMaterialProfile == 49u ||
					 iSourceMaterialProfile == 51u || iSourceMaterialProfile == 58u ||
					 iSourceMaterialProfile == 63u || iSourceMaterialProfile == 65u ||
					 iSourceMaterialProfile == 67u || iSourceMaterialProfile == 69u ||
					 iSourceMaterialProfile == 80u || iSourceMaterialProfile == 81u ||
					 iSourceMaterialProfile == 82u || iSourceMaterialProfile == 127u ||
					 iSourceMaterialProfile == 129u || iSourceMaterialProfile == 169u ||
					 iSourceMaterialProfile == 185u || iSourceMaterialProfile == 191u ||
					 iSourceMaterialProfile == 194u || iSourceMaterialProfile == 195u ||
					 iSourceMaterialProfile == 196u || iSourceMaterialProfile == 200u ||
					 iSourceMaterialProfile == 201u || iSourceMaterialProfile == 202u ||
					 (iSourceMaterialProfile >= 208u && iSourceMaterialProfile <= 263u || (iSourceMaterialProfile >= 277u && iSourceMaterialProfile <= 280u)) ||
					 nullptr != Client::Find_ArtistProgram(Particle.pElement->Material.SourceMaterial.strRuntimeShaderProfileId) ||
					 nullptr != Client::Find_WarlordNativeProgram(Particle.pElement->Material.SourceMaterial.strRuntimeShaderProfileId) ||
					 nullptr != Client::Find_LanceMasterVAProgram(Particle.pElement->Material.SourceMaterial.strRuntimeShaderProfileId));
				Context.pNativeVelocityElement = Particle.pElement;
				Context.iNativeVelocityProfile = iSourceMaterialProfile;
				Context.bNativeVelocityResolved = true;
			}
			const bool_t bNativeVelocityBasis = Context.bNativeVelocityBasis;
			if (bNativeVelocityBasis)
			{
				const vector_t ToCamera = CameraWorldWithTranslation.r[3] - Translation;
				const f32_t fVelocityLengthSq = XMVectorGetX(XMVector3LengthSq(Velocity));
				const f32_t fViewLengthSq = XMVectorGetX(XMVector3LengthSq(ToCamera));
				if (!std::isfinite(fVelocityLengthSq) || !std::isfinite(fViewLengthSq))
					return false;
				if (fVelocityLengthSq > 1.e-8f && fViewLengthSq > 1.e-8f)
				{
					const vector_t Motion = XMVector3Normalize(Velocity);
					const vector_t Across = XMVector3Cross(XMVector3Normalize(ToCamera), Motion);
					if (XMVectorGetX(XMVector3LengthSq(Across)) > 1.e-8f)
					{
						// Native U=cross(toCamera,motion), V=-motion. Our rect
						// stores local Y=.5-v, so row1 must be +motion. Preserve
						// the authored pivot and both sizes; PSA_Velocity ignores roll.
						Orientation = XMMatrixIdentity();
						Orientation.r[0] = XMVector3Normalize(Across);
						Orientation.r[1] = Motion;
						Orientation.r[2] = XMVector3Normalize(XMVector3Cross(Orientation.r[0], Motion));
						break;
					}
				}
				// Retain the existing finite camera-plane fallback for zero motion
				// or a view-parallel direction where the native plane degenerates.
			}
			const f32_t fRight = XMVectorGetX(XMVector3Dot(
				Velocity, CameraWorld.r[0]));
			const f32_t fUp = XMVectorGetX(XMVector3Dot(
				Velocity, CameraWorld.r[1]));
			const f32_t fVelocityRoll =
				std::abs(fRight) + std::abs(fUp) > 1.e-6f ?
				std::atan2(fUp, fRight) : 0.f;
			Orientation = XMMatrixRotationZ(
				fVelocityRoll + fRoll) * CameraWorld;
			break;
		}
		case Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::AXIS_POSITIVE_X:
			Orientation = XMMatrixRotationZ(fRoll) *
				XMMatrixRotationY(XM_PIDIV2);
			break;
		case Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::AXIS_NEGATIVE_X:
			Orientation = XMMatrixRotationZ(fRoll) *
				XMMatrixRotationY(-XM_PIDIV2);
			break;
		case Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::AXIS_POSITIVE_Y:
			/* The shared rect's indexed front face has local normal -Z.  Rotate
			   that normal toward +Y so one-sided EPAL_Z particles face the
			   gameplay camera instead of being deterministically backface-culled. */
			Orientation = XMMatrixRotationZ(fRoll) *
				XMMatrixRotationX(XM_PIDIV2);
			break;
		case Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::AXIS_NEGATIVE_Y:
			Orientation = XMMatrixRotationZ(fRoll) *
				XMMatrixRotationX(-XM_PIDIV2);
			break;
		case Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::AXIS_NEGATIVE_Z:
			Orientation = XMMatrixRotationZ(fRoll) *
				XMMatrixRotationY(XM_PI);
			break;
		case Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::AXIS_POSITIVE_Z:
			Orientation = XMMatrixRotationZ(fRoll);
			break;
		case Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::ROTATE_Z:
		{
			const vector_t CameraPosition = CameraWorldWithTranslation.r[3];
			const vector_t Up = XMVectorSet(0.f, 0.f, 1.f, 0.f);
			vector_t Facing = XMVectorSubtract(CameraPosition, Translation);
			Facing = XMVectorSubtract(Facing,
				XMVectorScale(Up, XMVectorGetX(XMVector3Dot(Facing, Up))));
			if (XMVectorGetX(XMVector3LengthSq(Facing)) > 1.e-8f)
			{
				Facing = XMVector3Normalize(Facing);
				const vector_t Right = XMVector3Normalize(
					XMVector3Cross(Up, Facing));
				Orientation = XMMatrixIdentity();
				Orientation.r[0] = Right;
				Orientation.r[1] = Up;
				Orientation.r[2] = Facing;
			}
			Orientation = XMMatrixRotationZ(fRoll) * Orientation;
			break;
		}
		case Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::ROTATE_X:
		case Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::ROTATE_Y:
		case Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::CAMERA_RECTANGLE:
		case Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::END:
		default:
			Orientation = XMMatrixRotationZ(fRoll) * CameraWorld;
			break;
		}
		const matrix_t Pivot = XMMatrixTranslation(
			0.5f - Particle.vSpritePivot.x,
			Particle.vSpritePivot.y - 0.5f, 0.f);
		XMStoreFloat4x4(&OutWorld,
			Pivot * XMMatrixScalingFromVector(Scale) * Orientation *
			XMMatrixTranslationFromVector(Translation));
		return true;
	}

	bool_t Is_ParticleQuadOutsideViewXY(
		const float4x4_t& World, const PARTICLE_CLIP_CONTEXT& Context)
	{
		if (!Context.bEnabled) return false;
		const matrix_t WorldMatrix = XMLoadFloat4x4(&World);
		const matrix_t WorldViewProjection =
			(WorldMatrix * Context.View) * Context.Projection;
		matrix_t MagnitudeWorld;
		for (uint32_t Row = 0u; Row < 4u; ++Row)
			MagnitudeWorld.r[Row] = XMVectorAbs(WorldMatrix.r[Row]);
		const matrix_t MagnitudeWVP =
			MagnitudeWorld * Context.MagnitudeViewProjection;
		float4_t Magnitude;
		XMStoreFloat4(&Magnitude, XMVector4Transform(
			XMVectorSet(0.5f, 0.5f, 0.f, 1.f), MagnitudeWVP));
		// The bound uses absolute matrix products, so cancellation cannot make
		// the CPU/GPU roundoff margin shrink near a clip boundary. XY clipping
		// is independent of depth clipping, depth tests and pixel-shader alpha.
		const f32_t MarginX = 1.e-4f * (1.f + Magnitude.x + Magnitude.w);
		const f32_t MarginY = 1.e-4f * (1.f + Magnitude.y + Magnitude.w);
		if (!std::isfinite(MarginX) || !std::isfinite(MarginY)) return false;
		uint32_t Outside = 0x0fu;
		for (uint32_t Vertex = 0u; Vertex < 4u; ++Vertex)
		{
			float4_t Clip;
			XMStoreFloat4(&Clip, XMVector4Transform(XMVectorSet(
				0u != (Vertex & 1u) ? 0.5f : -0.5f,
				0u != (Vertex & 2u) ? 0.5f : -0.5f, 0.f, 1.f),
				WorldViewProjection));
			if (!std::isfinite(Clip.x) || !std::isfinite(Clip.y) ||
				!std::isfinite(Clip.z) || !std::isfinite(Clip.w)) return false;
			const uint32_t VertexOutside =
				(Clip.x + Clip.w < -MarginX ? 1u : 0u) |
				(Clip.w - Clip.x < -MarginX ? 2u : 0u) |
				(Clip.y + Clip.w < -MarginY ? 4u : 0u) |
				(Clip.w - Clip.y < -MarginY ? 8u : 0u);
			Outside &= VertexOutside;
			if (0u == Outside) return false;
		}
		return 0u != Outside;
	}


	float4x4_t Apply_ParticleCameraOffset(
		const Client::EFFECT_EVALUATED_PARTICLE& Particle)
	{
		if (std::abs(Particle.fCameraOffset) <= 1.e-6f)
			return Particle.World;
		const matrix_t CameraWorld = XMLoadFloat4x4(
			CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW));
		float4x4_t Result{};
		XMStoreFloat4x4(&Result,
			XMLoadFloat4x4(&Particle.World) * XMMatrixTranslationFromVector(
				XMVectorScale(CameraWorld.r[2], Particle.fCameraOffset)));
		return Result;
	}

	bool_t Uses_SourceLockedAxisMeshFacing(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		if (!Element.SourceRecipe.bEnabled ||
			Element.SourceRecipe.strRendererShape != "mesh")
			return false;
		bool_t bTypeSpecific = false;
		bool_t bLockedAxis = false;
		bool_t bSourceZLocked = false;
		for (const auto& Module : Element.SourceRecipe.Modules)
		{
			if (std::ranges::any_of(Module.Literals, [](const auto& Literal)
				{ return Literal.strPropertyPath == "benabled" &&
					Literal.eKind == Client::EFFECT_SOURCE_LITERAL_KIND::BOOLEAN &&
					!Literal.bBoolean; }))
				continue;
			for (const auto& Literal : Module.Literals)
			{
				if (Literal.eKind != Client::EFFECT_SOURCE_LITERAL_KIND::STRING)
					continue;
				if (Module.strClassName == "particlemodulerequired" &&
					Literal.strPropertyPath == "screenalignment")
					bTypeSpecific = Literal.strString == "psa_typespecific";
				else if (Module.strClassName == "particlemoduletypedatamesh" &&
					Literal.strPropertyPath == "meshalignment")
					bLockedAxis = Literal.strString == "psma_meshfacecamerawithlockedaxis";
				else if (Module.strClassName == "particlemoduleorientationaxislock" &&
					Literal.strPropertyPath == "lockaxisflags")
					bSourceZLocked = Literal.strString == "epal_rotate_z";
			}
		}
		return bTypeSpecific && bLockedAxis && bSourceZLocked;
	}

	bool_t Make_SourceLockedAxisMeshWorld(
		const Client::EFFECT_EVALUATED_PARTICLE& Particle,
		float4x4_t& OutWorld)
	{
		OutWorld = Apply_ParticleCameraOffset(Particle);
		const matrix_t Source = XMLoadFloat4x4(&OutWorld);
		const f32_t fDeterminant = XMVectorGetX(XMMatrixDeterminant(Source));
		if (!std::isfinite(fDeterminant)) return false;
		// Retain the native renderer's existing valid zero-size suppression.
		if (std::abs(fDeterminant) <= std::numeric_limits<f32_t>::epsilon())
			return true;
		const float3_t& Degrees = Particle.pElement->Detail.Mesh.vSourceTypeDataRotationDegrees;
		// Equivalent to Playback's UE3 Euler matrix conjugated to Client X/Y-up.
		// Remove only this pre-rotation before extracting size; decomposing the
		// already pre-rotated nonuniform mesh would lose its authored basis.
		const matrix_t PreRotation =
			XMMatrixRotationX(-XMConvertToRadians(Degrees.x)) *
			XMMatrixRotationZ(XMConvertToRadians(Degrees.y)) *
			XMMatrixRotationY(XMConvertToRadians(Degrees.z));
		vector_t Scale, Rotation, Translation;
		if (!XMMatrixDecompose(&Scale, &Rotation, &Translation,
			XMMatrixTranspose(PreRotation) * Source))
			return false;
		// Playback already maps the source EPAL_Rotate_Z lock to Client Y.
		if (Particle.eSpriteAlignment != Client::EFFECT_PARTICLE_SPRITE_ALIGNMENT::ROTATE_Y)
			return false;
		const vector_t Up = XMVectorSet(0.f, 1.f, 0.f, 0.f);
		const matrix_t CameraWorld = XMLoadFloat4x4(
			CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW));
		const auto ProjectFacing = [Up](const vector_t Direction)
		{
			return Direction - XMVectorScale(Up,
				XMVectorGetX(XMVector3Dot(Direction, Up)));
		};
		vector_t Facing = ProjectFacing(CameraWorld.r[3] - Translation);
		f32_t fFacingLengthSq = XMVectorGetX(XMVector3LengthSq(Facing));
		if (!std::isfinite(fFacingLengthSq)) return false;
		if (fFacingLengthSq <= 1.e-8f)
		{
			// A camera on the locked axis has no unique azimuth. Use the view
			// direction, then a deterministic perpendicular axis, without NaNs.
			Facing = ProjectFacing(XMVectorNegate(CameraWorld.r[2]));
			fFacingLengthSq = XMVectorGetX(XMVector3LengthSq(Facing));
			if (!std::isfinite(fFacingLengthSq)) return false;
			if (fFacingLengthSq <= 1.e-8f)
				Facing = ProjectFacing(std::abs(XMVectorGetX(Up)) < .5f ?
					XMVectorSet(1.f, 0.f, 0.f, 0.f) : XMVectorSet(0.f, 0.f, 1.f, 0.f));
		}
		Facing = XMVector3Normalize(Facing);
		matrix_t Orientation = XMMatrixIdentity();
		// Native mesh alignment faces +X toward the camera and locks +Z up;
		// source-to-Client conversion maps the latter to +Y. PreRotation keeps
		// the source arrow's five-degree tilt and original mesh-facing basis.
		Orientation.r[0] = Facing;
		Orientation.r[1] = Up;
		Orientation.r[2] = XMVector3Normalize(XMVector3Cross(Facing, Up));
		XMStoreFloat4x4(&OutWorld, PreRotation * XMMatrixScalingFromVector(Scale) *
			Orientation * XMMatrixTranslationFromVector(Translation));
		return true;
	}

	f32_t SourceLiteralNumber(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const std::string_view strPropertyPath,
		const f32_t fFallback)
	{
		for (const Client::EFFECT_SOURCE_MODULE_DESC& Module :
			Element.SourceRecipe.Modules)
		{
			if (Module.strClassName != "particlemodulerequired")
				continue;
			for (const Client::EFFECT_SOURCE_LITERAL_DESC& Literal :
				Module.Literals)
			{
				if (Literal.strPropertyPath == strPropertyPath &&
					Literal.eKind ==
						Client::EFFECT_SOURCE_LITERAL_KIND::NUMBER)
				{
					return static_cast<f32_t>(Literal.fNumber);
				}
			}
		}
		return fFallback;
	}

	bool_t SourceLiteralBool(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const std::string_view strPropertyPath,
		const bool_t bFallback)
	{
		for (const Client::EFFECT_SOURCE_MODULE_DESC& Module :
			Element.SourceRecipe.Modules)
		{
			if (Module.strClassName != "particlemodulerequired")
				continue;
			for (const Client::EFFECT_SOURCE_LITERAL_DESC& Literal :
				Module.Literals)
			{
				if (Literal.strPropertyPath == strPropertyPath &&
					Literal.eKind ==
						Client::EFFECT_SOURCE_LITERAL_KIND::BOOLEAN)
				{
					return Literal.bBoolean;
				}
			}
		}
		return bFallback;
	}

	std::string_view SourceLiteralString(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const std::string_view strPropertyPath)
	{
		for (const Client::EFFECT_SOURCE_MODULE_DESC& Module :
			Element.SourceRecipe.Modules)
		{
			if (Module.strClassName != "particlemodulerequired")
				continue;
			for (const Client::EFFECT_SOURCE_LITERAL_DESC& Literal : Module.Literals)
			{
				if (Literal.strPropertyPath == strPropertyPath &&
					Literal.eKind == Client::EFFECT_SOURCE_LITERAL_KIND::STRING)
					return Literal.strString;
			}
		}
		return {};
	}

	bool_t Requires_SourceSpriteDepthSort(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		using PROFILE = Client::EFFECT_RENDER_PROFILE;
		return Element.SourceRecipe.bEnabled &&
			Element.SourceRecipe.strRendererShape == "sprite" &&
			(Element.Material.eRenderProfile == PROFILE::ALPHA_TWO_SIDED_DEPTH_READ ||
			 Element.Material.eRenderProfile == PROFILE::ALPHA_ONE_SIDED_DEPTH_READ) &&
			SourceLiteralString(Element, "sortmode") == "psortmode_viewprojdepth";
	}

	bool_t Build_SourceSpriteDepthOrder(
		const std::span<const Client::EFFECT_EVALUATED_PARTICLE> Particles,
		const Client::EFFECT_ELEMENT_DESC& Source,
		const float4x4_t& View, const float4x4_t& Projection,
		std::vector<SOURCE_SPRITE_DEPTH_ORDER>& OutOrder,
		std::string& OutError)
	{
		OutOrder.clear();
		const matrix_t ViewProjection = XMLoadFloat4x4(&View) *
			XMLoadFloat4x4(&Projection);
		for (const vector_t& Row : ViewProjection.r)
		{
			if (XMVector4IsNaN(Row) || XMVector4IsInfinite(Row))
			{
				OutError = "Source sprite depth sort view-projection is non-finite.";
				return false;
			}
		}
		OutOrder.reserve(Particles.size());
		for (const Client::EFFECT_EVALUATED_PARTICLE& Particle : Particles)
		{
			if (nullptr == Particle.pElement ||
				Particle.pElement->strElementId != Source.strElementId)
			{
				OutError = "Source sprite depth sort element span is invalid.";
				return false;
			}
			const vector_t Position = XMVectorSet(Particle.World._41,
				Particle.World._42, Particle.World._43, 1.f);
			const f32_t ClipDepth = XMVectorGetZ(
				XMVector4Transform(Position, ViewProjection));
			if (XMVector4IsNaN(Position) || XMVector4IsInfinite(Position) ||
				!std::isfinite(ClipDepth))
			{
				OutError = "Source sprite depth sort particle center is non-finite.";
				return false;
			}
			OutOrder.push_back({ &Particle, ClipDepth });
		}
		// UE3 ViewProjDepth uses center clip-Z before camera offset/pivot.
		// Preserve input order on ties; the original comparator had no stable tie contract.
		std::stable_sort(OutOrder.begin(), OutOrder.end(),
			[](const SOURCE_SPRITE_DEPTH_ORDER& Left,
				const SOURCE_SPRITE_DEPTH_ORDER& Right)
			{ return Left.fClipDepth > Right.fClipDepth; });
		return true;
	}

	bool_t SourceMaterialIdentityMatches(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		const std::string_view strProfileId,
		const std::string_view strParentMaterialPath)
	{
		return Source.strProfileId == strProfileId &&
			Source.strParentMaterialPath == strParentMaterialPath;
	}

	uint32_t SourceMaterialProfileIndex(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source)
	{
		if (!Source.bEnabled ||
			Source.strRuntimeShaderProfileId ==
				"effect.ue3.reconstructed-standard.v1")
			return 0u;
		if (const auto* pNative = Client::Find_LanceMasterVAProgram(Source.strRuntimeShaderProfileId))
			return pNative->iProfileIndex;
		if (const auto* pNative = Client::Find_ArtistProgram(Source.strRuntimeShaderProfileId))
			return pNative->iProfileIndex;
		if (const auto* pNative = Client::Find_WarlordNativeProgram(Source.strRuntimeShaderProfileId))
			return pNative->iProfileIndex;
		if (const auto* pNativeSD = Client::Find_DimensionMasterSDProgram(Source.strRuntimeShaderProfileId))
			return pNativeSD->iProfileIndex;
		if (const auto* pNativeWR = Client::Find_DimensionMasterWRProgram(Source.strRuntimeShaderProfileId))
			return pNativeWR->iProfileIndex;
		if (const auto* pNativeALTV = Client::Find_DimensionMasterALTVProgram(Source.strRuntimeShaderProfileId))
			return pNativeALTV->iProfileIndex;
		if (const auto* pNativeV = Client::Find_DimensionMasterVProgram(Source.strRuntimeShaderProfileId))
			return pNativeV->iProfileIndex;
		if (const auto* pNativeQ = Client::Find_DimensionMasterQProgram(Source.strRuntimeShaderProfileId))
			return pNativeQ->iProfileIndex;
		if (Source.strRuntimeShaderProfileId == Client::EFFECT_SLICE_SCENE_DEPTH_RUNTIME_PROFILE_ID)
			return 43u;
		if (Source.strRuntimeShaderProfileId == Client::EFFECT_CUBESAMPLE_SCENE_RUNTIME_PROFILE_ID)
			return 42u;
		if (Source.strRuntimeShaderProfileId == "effect.ue3.circle.v1")
			return 1u;
		if (Source.strRuntimeShaderProfileId == "effect.ue3.dot.v1")
			return 2u;
		if (Source.strRuntimeShaderProfileId == "effect.ue3.ring.v1")
			return 3u;
		if (Source.strRuntimeShaderProfileId == "effect.ue3.aura.v1")
			return 4u;
		if (Source.strRuntimeShaderProfileId ==
			"effect.ue3.one-layer-distortion.v1")
			return 5u;
		if (Source.strRuntimeShaderProfileId ==
			"effect.ue3.grouped-translucent.v1")
			return 6u;
		if (Source.strRuntimeShaderProfileId ==
			"effect.ue3.shine.v1")
			return 7u;
		if (Source.strRuntimeShaderProfileId ==
			"effect.ue3.blackline-aura.v1")
			return 8u;
		if (Source.strRuntimeShaderProfileId ==
			"effect.ue3.local-crack.v1")
		{
			if (Source.Textures.empty())
				return 0u;
			return Has_LocalCrackSourceTextureContract(Source) ? 9u :
				UINT32_MAX;
		}
		if (Source.strRuntimeShaderProfileId ==
			"effect.ue3.procedural-center-glow.v1")
			return 10u;
		if (Source.strRuntimeShaderProfileId ==
			"effect.ue3.linearflow-02.v1")
			return 11u;
		if (Source.strRuntimeShaderProfileId == "effect.ue3.slice.v1")
			return 12u;
		if (Source.strRuntimeShaderProfileId ==
			Client::EFFECT_MISSILETRAIL_RUNTIME_PROFILE_ID)
		{
			return SourceMaterialIdentityMatches(Source,
				"ue3.material.fx.m.mi.03.fx.m.fx.m.pa."
				"missiletrail.01.tr.9641f8d91e6a",
				"fx_m_mi_03.fx_m.fx_m_pa_missiletrail_01_tr") &&
				Source.StaticSwitches.empty() ? 13u : UINT32_MAX;
		}
		if (Source.strRuntimeShaderProfileId ==
			Client::EFFECT_WATERTRAIL_RUNTIME_PROFILE_ID)
		{
			return SourceMaterialIdentityMatches(Source,
				"ue3.material.fx.m.mi.03.fx.m.fx.m.me.watertrail.01.tr.afa4aeba0c50",
				"fx_m_mi_03.fx_m.fx_m_me_watertrail_01_tr") &&
				Source.StaticSwitches.empty() &&
				Client::Has_EffectWaterTrailNamedTextureContract(Source) ?
				14u : UINT32_MAX;
		}
		if (Source.strRuntimeShaderProfileId ==
			Client::EFFECT_MISSILETRAIL_TWO_EMISSIVE_RUNTIME_PROFILE_ID)
			return 15u;
		if (Source.strRuntimeShaderProfileId ==
			"effect.ue3.glasshole-02.v1")
			return SourceMaterialIdentityMatches(Source,
				"ue3.material.fx.m.mi.j.00.fx.m.fx.j.pa.glasshole.02.tr.175266c16bb2",
				"fx_m_mi_j_00.fx_m.fx_j_pa_glasshole_02_tr") &&
				Client::Has_EffectGlasshole02NamedTextureContract(Source) ?
				29u : UINT32_MAX;
		if (Source.strRuntimeShaderProfileId ==
			"effect.ue3.fluidninja-01.v1")
			return SourceMaterialIdentityMatches(Source,
				"ue3.material.fx.m.mi.k.00.fx.m.fx.k.pa.fluidninja.01.tr.534340d78128",
				"fx_m_mi_k_00.fx_m.fx_k_pa_fluidninja_01_tr") &&
				Client::Has_EffectFluidNinja01NamedTextureContract(Source) ?
				30u : UINT32_MAX;
		if (Source.strRuntimeShaderProfileId ==
			"effect.ue3.customparticle-01.v1")
			return SourceMaterialIdentityMatches(Source,
				"ue3.material.fx.m.mi.j.00.fx.m.fx.j.pa.customparticle.01.ad.e6b959010967",
				"fx_m_mi_j_00.fx_m.fx_j_pa_customparticle_01_ad") &&
				Client::Has_EffectCustomParticle01NamedTextureContract(Source) ?
				31u : UINT32_MAX;
		if (Source.strRuntimeShaderProfileId ==
			"effect.ue3.crackholev2-01.v1")
			return SourceMaterialIdentityMatches(Source,
				"ue3.material.fx.m.mi.k.00.fx.m.fx.k.crackholev2.01.3aac97e0fcad",
				"fx_m_mi_k_00.fx_m.fx_k_crackholev2_01") &&
				Client::Has_EffectCrackholeV2NamedTextureContract(Source) ?
				32u : UINT32_MAX;
		if (Source.strRuntimeShaderProfileId ==
			"effect.ue3.simple-01.v1")
			return SourceMaterialIdentityMatches(Source,
				"ue3.material.fx.mastermaterial.fx.mm.fx.mm.simple.01.ad.9b97b139cca2",
				"fx_mastermaterial.fx_mm.fx_mm_simple_01_ad") &&
				Source.StaticSwitches.empty() &&
				Client::Has_EffectSimple01NamedTextureContract(Source) ?
				33u : UINT32_MAX;
		return UINT32_MAX;
	}

	bool_t BindingMatches(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const Client::EFFECT_RESOURCE_SLOT eSlot,
		const std::string_view strAssetId)
	{
		const Client::EFFECT_RESOURCE_BINDING_DESC* pBinding =
			Find_Binding(Element, eSlot);
		return nullptr != pBinding && pBinding->strAssetId == strAssetId;
	}

	bool_t NamedTextureMatches(
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source,
		const std::string_view strName,
		const std::string_view strAssetId)
	{
		return std::ranges::count_if(Source.Textures,
			[strName, strAssetId](
				const Client::EFFECT_NAMED_TEXTURE_DESC& Texture)
			{
				return Texture.strName == strName &&
					Texture.strAssetId == strAssetId;
			}) == 1;
	}

	bool_t Is_DimensionMasterDBoundarySpriteWaveContract(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		return Client::Resolve_EffectPortableOriginElementId(Element) ==
				"authored.source-particle.03ae2d86558a1627f9d867e7" &&
			Element.eKind == Client::EFFECT_ELEMENT_KIND::PARTICLE &&
			Element.SourceRecipe.strRendererShape == "sprite" &&
			Element.ResourceBindings.size() == 3u &&
			SourceMaterialIdentityMatches(Element.Material.SourceMaterial,
				"ue3.material.fx.m.mi.m.00.fx.m.fx.m.pa.spritewave.01.tr.21401ca3cd92",
				"fx_m_mi_m_00.fx_m.fx_m_pa_spritewave_01_tr") &&
			Element.Material.strSourceMaterialPath ==
				"fx_m_mi_w_00.mi.fx_w_pa_spritewave_01_106_tr" &&
			BindingMatches(Element,
				Client::EFFECT_RESOURCE_SLOT::BASE_TEXTURE,
				"Effect/DimensionMaster/Textures/FX_TEX_06/fx_m_tilelinenoise_01.dds") &&
			BindingMatches(Element,
				Client::EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE,
				"Effect/DimensionMaster/Textures/FX_TEX_06/fx_w_atypical_016_xcl.dds") &&
			BindingMatches(Element,
				Client::EFFECT_RESOURCE_SLOT::NOISE_TEXTURE,
				"Effect/DimensionMaster/Textures/FX_TEX_05/fx_m_noise_001.dds");
	}

	bool_t Is_DimensionMasterDBoundaryParticleMasterContract(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		const std::string_view OriginElementId =
			Client::Resolve_EffectPortableOriginElementId(Element);
		const bool_t bExactDimensionMasterDOccurrence =
			OriginElementId ==
				"authored.source-particle.20e58ca3740942649576b818" ||
			OriginElementId ==
				"authored.source-particle.cd12d28ee975a182b849dae0" ||
			OriginElementId ==
				"authored.source-particle.1d400b300d15f98b78e45a92";
		if (!bExactDimensionMasterDOccurrence ||
			Element.eKind != Client::EFFECT_ELEMENT_KIND::PARTICLE ||
			Element.SourceRecipe.strRendererShape != "sprite" ||
			Element.ResourceBindings.size() != 4u ||
			!SourceMaterialIdentityMatches(Element.Material.SourceMaterial,
				"ue3.material.fx.m.mi.00.fx.m.fx.d.pa.master.01.tr.47fde102a56b",
				"fx_m_mi_00.fx_m.fx_d_pa_master_01_tr") ||
			!BindingMatches(Element,
				Client::EFFECT_RESOURCE_SLOT::BASE_TEXTURE,
				"Effect/DimensionMaster/Textures/FX_TEX_04/fx_i_noise_03.dds") ||
			!BindingMatches(Element,
				Client::EFFECT_RESOURCE_SLOT::NOISE_TEXTURE,
				"Effect/DimensionMaster/Textures/FX_TEX_02/fx_d_noise_009.dds") ||
			!BindingMatches(Element,
				Client::EFFECT_RESOURCE_SLOT::MASK_TEXTURE,
				"Effect/DimensionMaster/Textures/FX_TEX_06/fx_j_environment_tile_02.dds"))
		{
			return false;
		}
		const bool_t bMaster04 = Element.Material.strSourceMaterialPath ==
				"fx_m_mi_w_00.mi.fx_w_pa_master_01_04_dt_tr" &&
			BindingMatches(Element,
				Client::EFFECT_RESOURCE_SLOT::EMISSIVE_TEXTURE,
				"Effect/DimensionMaster/Textures/FX_TEX_04/fx_j_flowsmoke_01_cl.dds");
		const bool_t bMaster05 = Element.Material.strSourceMaterialPath ==
				"fx_m_mi_w_00.mi.fx_w_pa_master_01_05_dt_tr" &&
			BindingMatches(Element,
				Client::EFFECT_RESOURCE_SLOT::EMISSIVE_TEXTURE,
				"Effect/DimensionMaster/Textures/FX_TEX_00/fx_a_cloud_021.dds");
		return bMaster04 || bMaster05;
	}

	bool_t Is_FamilyProfileCarrierContractSatisfied(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const uint32_t iProfile)
	{
		if (Element.eKind != Client::EFFECT_ELEMENT_KIND::PARTICLE ||
			Element.SourceRecipe.strRendererShape != "sprite")
		{
			return false;
		}
		if (29u == iProfile || 30u == iProfile)
		{
			return Element.Material.eRenderProfile ==
				Client::EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ;
		}
		if (31u == iProfile)
		{
			return Element.Material.eRenderProfile ==
				Client::EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ;
		}
		if (33u == iProfile)
		{
			/* This family is authored both one- and two-sided across the corpus
			   (279 / 89 of 368). Sidedness is raster state, not a different
			   formula, so both carriers resolve to the same profile. */
			return Element.Material.eRenderProfile ==
					Client::EFFECT_RENDER_PROFILE::
						ADDITIVE_ONE_SIDED_DEPTH_READ ||
				Element.Material.eRenderProfile ==
					Client::EFFECT_RENDER_PROFILE::
						ADDITIVE_TWO_SIDED_DEPTH_READ;
		}
		return 32u == iProfile && Element.Material.eRenderProfile ==
			Client::EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ;
	}

	bool_t Is_StrictParticleShapeCarrierContractSatisfied(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const std::string_view strRequiredShape)
	{
		if (Element.eKind != Client::EFFECT_ELEMENT_KIND::PARTICLE ||
			Element.SourceRecipe.strRendererShape != strRequiredShape)
		{
			return false;
		}
		const Client::EFFECT_RESOURCE_BINDING_DESC* pMesh =
			Find_Binding(Element, Client::EFFECT_RESOURCE_SLOT::MESH_MODEL);
		return strRequiredShape == "mesh" ?
			(nullptr != pMesh && !pMesh->strAssetId.empty()) : nullptr == pMesh;
	}

	bool_t Is_StrictParticleBlendCarrierContractSatisfied(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const bool_t bAdditive)
	{
		if (!Is_StrictParticleShapeCarrierContractSatisfied(Element, "sprite") &&
			!Is_StrictParticleShapeCarrierContractSatisfied(Element, "mesh"))
		{
			return false;
		}
		if (bAdditive)
		{
			return Element.Material.eRenderProfile ==
					Client::EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ ||
				Element.Material.eRenderProfile ==
					Client::EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ;
		}
		return Element.Material.eRenderProfile ==
				Client::EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ ||
			Element.Material.eRenderProfile ==
				Client::EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ;
	}

	bool_t Is_StrictTwoSidedAlphaMeshCarrierContractSatisfied(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		return Is_StrictParticleShapeCarrierContractSatisfied(Element, "mesh") &&
			Element.Material.eRenderProfile ==
				Client::EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ;
	}

	bool_t Is_MissileTrailFourLaneCarrierContractSatisfied(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source =
			Element.Material.SourceMaterial;
		if (!Is_StrictParticleShapeCarrierContractSatisfied(Element, "mesh") ||
			Element.Material.eRenderProfile !=
				Client::EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ ||
			Element.ResourceBindings.size() != 5u ||
			!SourceMaterialIdentityMatches(Source,
				"ue3.material.fx.m.mi.03.fx.m.fx.m.pa."
				"missiletrail.01.tr.9641f8d91e6a",
				"fx_m_mi_03.fx_m.fx_m_pa_missiletrail_01_tr") ||
			!Source.StaticSwitches.empty())
		{
			return false;
		}
		const Client::EFFECT_RESOURCE_BINDING_DESC* pBase = Find_Binding(
			Element, Client::EFFECT_RESOURCE_SLOT::BASE_TEXTURE);
		const Client::EFFECT_RESOURCE_BINDING_DESC* pMask = Find_Binding(
			Element, Client::EFFECT_RESOURCE_SLOT::MASK_TEXTURE);
		const Client::EFFECT_RESOURCE_BINDING_DESC* pNoise = Find_Binding(
			Element, Client::EFFECT_RESOURCE_SLOT::NOISE_TEXTURE);
		const Client::EFFECT_RESOURCE_BINDING_DESC* pDissolve = Find_Binding(
			Element, Client::EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE);
		if (nullptr == pBase || pBase->strAssetId.empty() || nullptr == pMask ||
			pMask->strAssetId.empty() || nullptr == pNoise ||
			pNoise->strAssetId.empty() || nullptr == pDissolve ||
			pDissolve->strAssetId.empty() || nullptr != Find_Binding(
				Element, Client::EFFECT_RESOURCE_SLOT::EMISSIVE_TEXTURE))
		{
			return false;
		}
		/* Older sealed programs can lack named texture provenance. When names
		   are present, bind every executable lane back to the same typed
		   resource and require the one unsampled dependency receipt. Duplicate
		   aliases must not be admitted by a first-match lookup. */
		if (Source.Textures.empty())
			return true;
		static constexpr std::array<std::string_view, 3u> CONSUMED_NAMES = {{
			"alpha_tex", "uv_noise_tex", "uv_dissolve_tex"
		}};
		const size_t iDependencyCount = std::ranges::count_if(
			Source.Textures,
			[](const Client::EFFECT_NAMED_TEXTURE_DESC& Texture)
			{
				return Texture.strName == "umodel_dependency" &&
					Texture.strAssetId.empty();
			});
		return Source.Textures.size() == 4u && iDependencyCount == 1u &&
			Client::Has_EffectUniqueNamedTextureContract(
				Source, CONSUMED_NAMES) &&
			NamedTextureMatches(Source, "alpha_tex", pMask->strAssetId) &&
			NamedTextureMatches(Source, "uv_noise_tex", pNoise->strAssetId) &&
			NamedTextureMatches(Source, "uv_dissolve_tex",
				pDissolve->strAssetId);
	}

	uint32_t EffectiveSourceMaterialProfileIndex(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		const Client::EFFECT_SOURCE_MATERIAL_DESC& Source =
			Element.Material.SourceMaterial;
		const uint32_t iStoredProfile = SourceMaterialProfileIndex(Source);
		if (nullptr != Client::Find_LanceMasterVAProgram(Source.strRuntimeShaderProfileId))
			return Client::Has_LanceMasterVAMaterialContract(Element) ? iStoredProfile : UINT32_MAX;
		if (nullptr != Client::Find_ArtistProgram(Source.strRuntimeShaderProfileId))
			return Client::Has_ArtistMaterialContract(Element) ? iStoredProfile : UINT32_MAX;
		if (nullptr != Client::Find_WarlordNativeProgram(Source.strRuntimeShaderProfileId))
			return Client::Has_WarlordNativeMaterialContract(Element) ? iStoredProfile : UINT32_MAX;
		if (nullptr != Client::Find_DimensionMasterSDProgram(Source.strRuntimeShaderProfileId))
			return Client::Has_DimensionMasterSDMaterialContract(Element) ? iStoredProfile : UINT32_MAX;
		if (iStoredProfile >= 208u && iStoredProfile <= 263u || (iStoredProfile >= 277u && iStoredProfile <= 280u))
			return Client::Has_DimensionMasterWRMaterialContract(Element) ? iStoredProfile : UINT32_MAX;
		if (iStoredProfile >= 80u && iStoredProfile <= 205u)
			return Client::Has_DimensionMasterALTVMaterialContract(Element) ? iStoredProfile : UINT32_MAX;
		if (iStoredProfile >= 52u && iStoredProfile <= 76u)
			return Client::Has_DimensionMasterVMaterialContract(Element) ? iStoredProfile : UINT32_MAX;
		if (iStoredProfile >= 44u && iStoredProfile <= 51u)
			return Client::Has_DimensionMasterQMaterialContract(Element) ? iStoredProfile : UINT32_MAX;
		if (43u == iStoredProfile)
			return Client::Has_EffectSliceSceneDepthContract(Element) ? 43u : UINT32_MAX;
		if (42u == iStoredProfile)
			return Client::Has_EffectCubeSampleSceneContract(Element) ? 42u : UINT32_MAX;
		if (13u == iStoredProfile)
		{
			return Is_MissileTrailFourLaneCarrierContractSatisfied(Element) ?
				13u : UINT32_MAX;
		}
		if (14u == iStoredProfile)
		{
			return Is_StrictTwoSidedAlphaMeshCarrierContractSatisfied(Element) &&
				Source.StaticSwitches.empty() &&
				Client::Has_EffectWaterTrailNamedTextureContract(Source) ?
				14u : UINT32_MAX;
		}
		if (iStoredProfile >= 29u && iStoredProfile <= 33u)
		{
			if (33u == iStoredProfile &&
				!Is_StrictParticleShapeCarrierContractSatisfied(Element, "sprite"))
			{
				return UINT32_MAX;
			}
			return Is_FamilyProfileCarrierContractSatisfied(
				Element, iStoredProfile) ? iStoredProfile : UINT32_MAX;
		}
		if (6u == iStoredProfile &&
			Client::Resolve_EffectStrictTypedSourceProfile(
				Element.Material.strSourceMaterialPath, Source) ==
				Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::FLOWRIBBON01)
		{
			return Client::Has_EffectFlowRibbon01TrailContract(Element) ?
				35u : UINT32_MAX;
		}
		if (6u != iStoredProfile ||
			Element.eKind != Client::EFFECT_ELEMENT_KIND::PARTICLE ||
			(Element.SourceRecipe.strRendererShape != "mesh" &&
			 Element.SourceRecipe.strRendererShape != "sprite"))
		{
			return iStoredProfile;
		}
		if (Is_DimensionMasterDBoundarySpriteWaveContract(Element) ||
			Is_DimensionMasterDBoundaryParticleMasterContract(Element))
		{
			/* These exact DimensionMaster-D cards lost only their named texture
			   lanes during Effect Tool compaction. Their opaque DDS alpha cannot
			   define coverage, so reuse the already bounded Slice evaluator used
			   by the W Voronoi card instead of exposing grouped profile 6. */
			return 12u;
		}

		switch (Client::Resolve_EffectStrictTypedSourceProfile(
			Element.Material.strSourceMaterialPath, Source))
		{
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::MISSILETRAIL:
			if (BindingMatches(Element,
					Client::EFFECT_RESOURCE_SLOT::MESH_MODEL,
					"Effect/LanceMaster/Meshes/fm_m_ring_001.wmodel") &&
				BindingMatches(Element,
					Client::EFFECT_RESOURCE_SLOT::BASE_TEXTURE,
					"Effect/LanceMaster/Textures/fx_h_atypical_01_1.dds") &&
				BindingMatches(Element,
					Client::EFFECT_RESOURCE_SLOT::MASK_TEXTURE,
					"Effect/LanceMaster/Textures/fx_m_trail_007.dds") &&
				BindingMatches(Element,
					Client::EFFECT_RESOURCE_SLOT::EMISSIVE_TEXTURE,
					"Effect/LanceMaster/Textures/fx_m_noise_003.dds") &&
				BindingMatches(Element,
					Client::EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE,
					"Effect/LanceMaster/Textures/fx_m_noise_001.dds") &&
				BindingMatches(Element,
					Client::EFFECT_RESOURCE_SLOT::NOISE_TEXTURE,
					"Effect/LanceMaster/Textures/fx_d_noise_030.dds") &&
				NamedTextureMatches(Source, "alpha_tex",
					"Effect/LanceMaster/Textures/fx_m_trail_007.dds") &&
				NamedTextureMatches(Source, "emissive_tex01",
					"Effect/LanceMaster/Textures/fx_h_atypical_01_1.dds") &&
				NamedTextureMatches(Source, "emissive_tex02",
					"Effect/LanceMaster/Textures/fx_m_noise_003.dds") &&
				NamedTextureMatches(Source, "uv_dissolve_tex",
					"Effect/LanceMaster/Textures/fx_m_noise_001.dds") &&
				NamedTextureMatches(Source, "uv_noise_tex",
					"Effect/LanceMaster/Textures/fx_d_noise_030.dds"))
			{
				return 15u;
			}
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::WATERTRAIL:
			if (Is_StrictTwoSidedAlphaMeshCarrierContractSatisfied(Element) &&
				Source.StaticSwitches.empty() &&
				Client::Has_EffectWaterTrailNamedTextureContract(Source))
			{
				return 14u;
			}
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::LINEARFLOW_02:
			if (Element.SourceRecipe.strRendererShape == "mesh" &&
				Element.ResourceBindings.size() == 4u &&
				BindingMatches(Element,
					Client::EFFECT_RESOURCE_SLOT::MESH_MODEL,
					"Effect/DimensionMaster/Meshes/fm_h_swing_02.wmodel") &&
				BindingMatches(Element,
					Client::EFFECT_RESOURCE_SLOT::BASE_TEXTURE,
					"Effect/DimensionMaster/Textures/FX_TEX_04/fx_j_mirnoise_02.dds") &&
				BindingMatches(Element,
					Client::EFFECT_RESOURCE_SLOT::MASK_TEXTURE,
					"Effect/DimensionMaster/Textures/FX_TEX_06/fx_j_auraline_19_ycl.dds") &&
				BindingMatches(Element,
					Client::EFFECT_RESOURCE_SLOT::NOISE_TEXTURE,
					"Effect/DimensionMaster/Textures/FX_TEX_02/fx_d_noise_014.dds"))
			{
				return 11u;
			}
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::MAKEFLOW_02:
			if (Is_StrictTwoSidedAlphaMeshCarrierContractSatisfied(Element) &&
				Source.StaticSwitches.empty() &&
				Client::Has_EffectMakeFlowMeshNamedTextureContract(Source))
			{
				return 36u;
			}
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::MAKEFLOW_03:
			if (Is_StrictTwoSidedAlphaMeshCarrierContractSatisfied(Element) &&
				Source.StaticSwitches.empty() &&
				Client::Has_EffectMakeFlowMeshNamedTextureContract(Source))
			{
				return 16u;
			}
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::MAKEFLOW_03_SPRITE:
			if (Is_StrictParticleShapeCarrierContractSatisfied(Element, "sprite") &&
				Element.Material.eRenderProfile ==
					Client::EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ &&
				Source.StaticSwitches.empty() &&
				Client::Has_EffectMakeFlow03SpriteNamedTextureContract(Source))
			{
				return 37u;
			}
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::RING_01:
			if (Element.SourceRecipe.strRendererShape == "sprite" &&
				Element.ResourceBindings.size() == 1u &&
				BindingMatches(Element,
					Client::EFFECT_RESOURCE_SLOT::BASE_TEXTURE,
					"Effect/DimensionMaster/Textures/FX_TEX_05/fx_m_trail_002.dds"))
			{
				return 17u;
			}
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::PARTICLETRAIL_01:
			if (Element.SourceRecipe.strRendererShape == "sprite" &&
				Element.ResourceBindings.size() == 3u)
			{
				const bool_t bTrail03 =
					Element.Material.strSourceMaterialPath ==
						"fx_m_mi_s_00.fx_mi.fx_s_pa_trail_03_01_tr" &&
					BindingMatches(Element,
						Client::EFFECT_RESOURCE_SLOT::BASE_TEXTURE,
						"Effect/DimensionMaster/Textures/FX_TEX_00/fx_a_trail_007.dds") &&
					BindingMatches(Element,
						Client::EFFECT_RESOURCE_SLOT::MASK_TEXTURE,
						"Effect/DimensionMaster/Textures/FX_TEX_00/fx_a_trail_007.dds") &&
					BindingMatches(Element,
						Client::EFFECT_RESOURCE_SLOT::NOISE_TEXTURE,
						"Effect/DimensionMaster/Textures/FX_TEX_05/fx_m_noise_004.dds");
				const bool_t bTrail01 =
					Element.Material.strSourceMaterialPath ==
						"fx_m_mi_m_00.fx_mi.fx_m_pa_trail_01_6_tr" &&
					BindingMatches(Element,
						Client::EFFECT_RESOURCE_SLOT::BASE_TEXTURE,
						"Effect/DimensionMaster/Textures/FX_TEX_05/fx_m_trail_005.dds") &&
					BindingMatches(Element,
						Client::EFFECT_RESOURCE_SLOT::MASK_TEXTURE,
						"Effect/DimensionMaster/Textures/FX_TEX_05/fx_m_trail_005.dds") &&
					BindingMatches(Element,
						Client::EFFECT_RESOURCE_SLOT::NOISE_TEXTURE,
						"Effect/DimensionMaster/Textures/FX_TEX_02/fx_d_noise_030.dds");
				if (bTrail03 || bTrail01)
					return 18u;
			}
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::PARTICLE_MASTER_01:
			if (Source.StaticSwitches.empty() &&
				Is_StrictParticleBlendCarrierContractSatisfied(Element,
					Source.strParentMaterialPath.ends_with("_ad")) &&
				Client::Has_EffectParticleMasterNamedTextureContract(Source))
				return 19u;
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::SPRITEWAVE_01:
			if (Source.StaticSwitches.empty() &&
				Is_StrictParticleBlendCarrierContractSatisfied(Element,
					Source.strParentMaterialPath.ends_with("_ad")) &&
				Client::Has_EffectSpriteWaveNamedTextureContract(Source))
				return 20u;
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::PARTICLETRAIL_SINGLE_ALPHA:
			if (Element.SourceRecipe.strRendererShape == "sprite" &&
				Client::Has_EffectParticleTrailSingleAlphaNamedTextureContract(Source))
			{
				return 21u;
			}
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::ARTIST_SPLA01:
			if (Element.SourceRecipe.strRendererShape == "sprite" &&
				Client::Has_EffectArtistSpla01NamedTextureContract(Source))
				return 22u;
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::ARTIST_SPLA05:
			if (Element.SourceRecipe.strRendererShape == "sprite" &&
				Client::Has_EffectArtistSpla05NamedTextureContract(Source))
				return 23u;
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::ARTIST_TWINKLE:
			if (Element.SourceRecipe.strRendererShape == "sprite" &&
				Client::Has_EffectArtistTwinkleNamedTextureContract(Source))
				return 24u;
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::ARTIST_FLUID01:
			if (Element.SourceRecipe.strRendererShape == "sprite" &&
				Client::Has_EffectArtistFluid01NamedTextureContract(Source))
				return 25u;
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::ARTIST_WORLDOFFSET01:
			if (Element.SourceRecipe.strRendererShape == "sprite" &&
				Client::Has_EffectArtistWorldOffset01NamedTextureContract(Source))
				return 26u;
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::ARTIST_MAKEFLOW01:
			if (Element.SourceRecipe.strRendererShape == "sprite" &&
				Client::Has_EffectArtistMakeFlow01NamedTextureContract(Source))
				return 27u;
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::ARTIST_LENSFLARE01:
			if (Element.SourceRecipe.strRendererShape == "sprite" &&
				Client::Has_EffectArtistLensFlare01NamedTextureContract(Source))
				return 28u;
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::ARTIST_MM_FLUID01:
			/* The source family spans mesh and sprite carriers with four role
			   sets. Admit only the exact two-lane sprite child used by the two
			   DimensionMaster F occurrences; the mesh variants remain on their
			   existing fail-closed/grouped boundary. */
			if (Element.SourceRecipe.strRendererShape == "sprite" &&
				Element.ResourceBindings.size() == 2u &&
				Element.Material.strSourceMaterialPath ==
					"fx_m_mi_w_00.mi.fx_w_pa_fd_01_3_tr" &&
				BindingMatches(Element,
					Client::EFFECT_RESOURCE_SLOT::BASE_TEXTURE,
					"Effect/DimensionMaster/Textures/FX_TEX_02/fx_d_cloud_035.dds") &&
				BindingMatches(Element,
					Client::EFFECT_RESOURCE_SLOT::EMISSIVE_TEXTURE,
					"Effect/DimensionMaster/Textures/FX_TEX_HIGH_03/fx_o_glass_01.dds"))
			{
				return 34u;
			}
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::FLOWRIBBON01:
			/* Trail carriers are resolved before the particle-only grouped branch. */
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::SIMPLE01:
			if (Is_StrictParticleShapeCarrierContractSatisfied(Element, "sprite") &&
				Is_StrictParticleBlendCarrierContractSatisfied(Element, true) &&
				Source.StaticSwitches.empty() &&
				Client::Has_EffectSimple01NamedTextureContract(Source))
			{
				return 33u;
			}
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::SIMPLE02:
			if (Is_StrictParticleShapeCarrierContractSatisfied(Element, "sprite") &&
				Element.Material.eRenderProfile ==
					Client::EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ &&
				Source.StaticSwitches.empty() &&
				Client::Has_EffectSimple02NamedTextureContract(Source))
			{
				return 38u;
			}
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::GLASSHOLE02:
			return Is_FamilyProfileCarrierContractSatisfied(Element, 29u) &&
				Client::Has_EffectGlasshole02NamedTextureContract(Source) ?
				29u : UINT32_MAX;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::FLUIDNINJA01:
			return Is_FamilyProfileCarrierContractSatisfied(Element, 30u) &&
				Client::Has_EffectFluidNinja01NamedTextureContract(Source) ?
				30u : UINT32_MAX;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::CUSTOMPARTICLE01:
			return Is_FamilyProfileCarrierContractSatisfied(Element, 31u) &&
				Client::Has_EffectCustomParticle01NamedTextureContract(Source) ?
				31u : UINT32_MAX;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::CRACKHOLEV2:
			return Is_FamilyProfileCarrierContractSatisfied(Element, 32u) &&
				Client::Has_EffectCrackholeV2NamedTextureContract(Source) ?
				32u : UINT32_MAX;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::MM_BASIC01:
			/* The corpus authors this family on both sprite and mesh particle
			   carriers and in additive and alpha blends.  Carrier geometry and
			   blend are render state; the equation is shared, so both admit. */
			if ((Is_StrictParticleShapeCarrierContractSatisfied(
					Element, "sprite") ||
				Is_StrictParticleShapeCarrierContractSatisfied(
					Element, "mesh")) &&
				Source.StaticSwitches.empty() &&
				Client::Has_EffectMmBasic01NamedTextureContract(Source))
			{
				return 39u;
			}
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::MESH_MASKED_CHAIN01:
			/* The parent owns no texture parameter, so the admission gate is the
			   mesh carrier plus the base binding the converter attached. */
			if (Is_StrictParticleShapeCarrierContractSatisfied(Element, "mesh") &&
				Source.StaticSwitches.empty() &&
				nullptr != Find_Binding(Element,
					Client::EFFECT_RESOURCE_SLOT::BASE_TEXTURE))
			{
				return 41u;
			}
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::MM_LIGHT01:
			/* Reuses the simple_01 evaluator and its lane staging.  With no
			   scalar authored, every simple_01 input falls back to neutral and
			   the equation reduces to the single emissive sample this family
			   owns.  Both blends admit because sidedness and blend are raster
			   state, not a different formula. */
			if (Is_StrictParticleShapeCarrierContractSatisfied(
					Element, "sprite") &&
				Source.StaticSwitches.empty() &&
				Client::Has_EffectSimple01NamedTextureContract(Source))
			{
				return 33u;
			}
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::FLOWTRAIL01:
			if (Is_StrictTwoSidedAlphaMeshCarrierContractSatisfied(Element) &&
				Source.StaticSwitches.empty() &&
				Client::Has_EffectFlowTrail01NamedTextureContract(Source))
			{
				return 40u;
			}
			break;
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::NONE:
		case Client::EFFECT_STRICT_TYPED_SOURCE_PROFILE::END:
		default:
			break;
		}
		return iStoredProfile;
	}

	uint32_t DynamicParameterSemanticIndex(const std::string& strSemantic)
	{
		if (strSemantic == "opacity") return 1u;
		if (strSemantic == "emissive") return 2u;
		if (strSemantic == "dissolve") return 3u;
		if (strSemantic == "uv_pan") return 4u;
		if (strSemantic == "distortion") return 5u;
		if (strSemantic == "radial_size") return 6u;
		if (strSemantic == "mask_a_offset") return 7u;
		if (strSemantic == "mask_b_offset") return 8u;
		if (strSemantic == "mask_a_distort") return 9u;
		if (strSemantic == "mask_b_distort") return 10u;
		if (strSemantic == "mask_a_pan") return 11u;
		if (strSemantic == "flow_strength") return 12u;
		if (strSemantic == "mask_b_pan") return 13u;
		if (strSemantic == "diffuse_pan") return 14u;
		if (strSemantic == "missile_alpha_pan") return 15u;
		if (strSemantic == "missile_noise_strength") return 16u;
		if (strSemantic == "missile_noise_pan") return 17u;
		if (strSemantic == "missile_dissolve") return 18u;
		if (strSemantic == "water_alpha_pan") return 19u;
		if (strSemantic == "water_noise_pan") return 20u;
		if (strSemantic == "water_dissolve") return 21u;
		if (strSemantic == "water_noise_strength") return 22u;
		return 0u;
	}

	uint32_t TypedDynamicParameterSemanticIndex(
		const uint32_t iProfile,
		const std::string_view strParameterName)
	{
		if (13u == iProfile || 15u == iProfile)
		{
			if (strParameterName == "alpha_pan") return 15u;
			if (strParameterName == "uv_noise_velue" ||
				strParameterName == "uv_noise_value" ||
				strParameterName == "noise_velue") return 16u;
			if (strParameterName == "uv_noise_pan") return 17u;
			if (strParameterName == "alpha_dissolve" ||
				strParameterName == "dissolve") return 18u;
		}
		else if (14u == iProfile)
		{
			if (strParameterName == "alpha_pan") return 19u;
			if (strParameterName == "uv_noise_pan") return 20u;
			if (strParameterName == "dissolve" ||
				strParameterName == "alpha_dissolve") return 21u;
			if (strParameterName == "noise_velue" ||
				strParameterName == "uv_noise_velue" ||
				strParameterName == "uv_noise_value") return 22u;
		}
		else if (11u == iProfile)
		{
			if (strParameterName == "mask_a_offset") return 7u;
			if (strParameterName == "mask_b_offset") return 8u;
			if (strParameterName == "mask_a_distort") return 9u;
			if (strParameterName == "mask_b_distort") return 10u;
		}
		else if (18u == iProfile)
		{
			if (strParameterName == "alpha_pan") return 23u;
			if (strParameterName == "lerp") return 24u;
			if (strParameterName == "noise_pan") return 25u;
			if (strParameterName == "noise_velue") return 26u;
		}
		else if (16u == iProfile || 36u == iProfile)
		{
			if (strParameterName == "flow_str") return 27u;
			if (strParameterName == "opacity_pan_v") return 28u;
			if (strParameterName == "opacity_pan_u") return 29u;
			if (strParameterName == "param4") return 30u;
		}
		else if (19u == iProfile)
		{
			if (strParameterName == "alphadissolve[0-1]") return 31u;
			if (strParameterName == "pan[0-2]") return 32u;
			if (strParameterName == "edgestr[0-x]") return 33u;
			if (strParameterName == "disrotion[0-x]") return 34u;
		}
		else if (20u == iProfile)
		{
			if (strParameterName == "maintex_tile_pan") return 35u;
			if (strParameterName == "dissolve") return 36u;
			if (strParameterName == "uv_noisevelue") return 37u;
			if (strParameterName == "uv_sphery_uv_noisepan") return 38u;
		}
		else if (21u == iProfile)
		{
			if (strParameterName == "alpha_pan") return 23u;
			if (strParameterName == "lerp") return 24u;
			if (strParameterName == "noise_pan") return 25u;
			if (strParameterName == "noise_velue") return 26u;
		}
		else if (23u == iProfile)
		{
			if (strParameterName == "_") return 47u;
			if (strParameterName == "dissolve[0-1]") return 39u;
			if (strParameterName == "uvdistort[0-1]") return 40u;
			if (strParameterName == "distortion[0-x]") return 41u;
		}
		else if (24u == iProfile)
		{
			if (strParameterName == "dissolve_density(0~1)") return 42u;
			if (strParameterName == "alpha_power(1~)") return 43u;
			if (strParameterName == "emissive_tiling(0.5~2)") return 44u;
			if (strParameterName == "lamp_time(0~1)") return 45u;
		}
		else if (26u == iProfile)
		{
			if (strParameterName == "dissolve") return 46u;
			if (strParameterName == "param2" ||
				strParameterName == "param3" ||
				strParameterName == "param4") return 47u;
		}
		else if (34u == iProfile)
		{
			if (strParameterName == "trasition_speed(0~1.5)") return 48u;
			if (strParameterName == "alpha_power(1~)") return 49u;
			if (strParameterName == "fresnel_alpha(1~)") return 50u;
			if (strParameterName == "param4") return 51u;
		}
		return 0u;
	}

	bool_t Try_ResolveTypedDynamicParameterSemantics(
		const Client::EFFECT_ELEMENT_DESC& Element,
		const uint32_t iProfile,
		std::array<uint32_t, 4u>& OutSemantics)
	{
		if (13u != iProfile && 15u != iProfile && 14u != iProfile &&
			16u != iProfile && 18u != iProfile && 19u != iProfile &&
			20u != iProfile && 21u != iProfile && 23u != iProfile &&
			24u != iProfile && 26u != iProfile && 34u != iProfile)
			return false;
		std::array<uint32_t, 4u> Staged{};
		std::array<bool_t, 4u> Seen{};
		uint32_t iDynamicModuleCount = 0u;
		for (const Client::EFFECT_SOURCE_MODULE_DESC& Module :
			Element.SourceRecipe.Modules)
		{
			if (Module.strClassName != "particlemoduleparameterdynamic")
				continue;
			++iDynamicModuleCount;
			for (const Client::EFFECT_SOURCE_LITERAL_DESC& Literal : Module.Literals)
			{
				if (Literal.eKind != Client::EFFECT_SOURCE_LITERAL_KIND::STRING)
					continue;
				for (size_t iChannel = 0u; iChannel < Staged.size(); ++iChannel)
				{
					const std::string strExpected = "dynamicparams[" +
						std::to_string(iChannel) + "].paramname";
					if (Literal.strPropertyPath != strExpected)
						continue;
					if (Seen[iChannel])
						return false;
					Staged[iChannel] = TypedDynamicParameterSemanticIndex(
						iProfile, Literal.strString);
					Seen[iChannel] = true;
				}
			}
		}
		if (1u != iDynamicModuleCount ||
			!std::ranges::all_of(Seen, [](const bool_t bSeen) { return bSeen; }) ||
			!std::ranges::all_of(
				Staged, [](const uint32_t iSemantic) { return 0u != iSemantic; }))
		{
			return false;
		}
		OutSemantics = Staged;
		return true;
	}

	SOURCE_SUBUV_LAYOUT Resolve_SourceSubUVLayout(
		const Client::EFFECT_ELEMENT_DESC& Element)
	{
		SOURCE_SUBUV_LAYOUT Layout;
		Layout.pElement = &Element;
		if (!Element.SourceRecipe.bEnabled)
			return Layout;
		// Required's atlas layout is immutable for the whole particle batch.
		const std::string_view strSubUVMode = SourceLiteralString(
			Element, "interpolationmethod");
		Layout.iColumns = static_cast<uint32_t>((std::max)(1.f,
			SourceLiteralNumber(Element, "subimages_horizontal", 1.f)));
		Layout.iRows = static_cast<uint32_t>((std::max)(1.f,
			SourceLiteralNumber(Element, "subimages_vertical", 1.f)));
		const uint64_t iFrameCount =
			static_cast<uint64_t>(Layout.iColumns) * Layout.iRows;
		Layout.bEnabled = iFrameCount > 1u && iFrameCount <= UINT32_MAX &&
			!strSubUVMode.empty() && strSubUVMode != "none" &&
			strSubUVMode != "psuvim_none";
		Layout.bLinearBlend = Client::Is_EffectSourceLinearBlendSubUVMode(
			strSubUVMode);
		return Layout;
	}

	Client::EFFECT_SUBUV_FRAME_DESC Resolve_SubUVFrames(
		const Client::EFFECT_EVALUATED_PARTICLE& Particle,
		const SOURCE_SUBUV_LAYOUT& BatchLayout)
	{
		if (nullptr == Particle.pElement)
		{
			return {};
		}
		if (!Particle.pElement->SourceRecipe.bEnabled)
		{
			const Client::EFFECT_ELEMENT_DESC& Element = *Particle.pElement;
			const Client::EFFECT_UV_DESC& UV = Element.Detail.UV;
			if (!Element.Material.Execution.bEnabled ||
				UV.iTileColumns <= 0 || UV.iTileRows <= 0)
			{
				return {};
			}
			const uint32_t iColumns = static_cast<uint32_t>(UV.iTileColumns);
			const uint32_t iRows = static_cast<uint32_t>(UV.iTileRows);
			const uint64_t iFrameCount64 =
				static_cast<uint64_t>(iColumns) * iRows;
			if (iFrameCount64 <= 1u || iFrameCount64 > UINT32_MAX)
				return {};
			const uint32_t iFrameCount = static_cast<uint32_t>(iFrameCount64);

			/* Authored Track A copies intentionally do not retain the source
			   module graph.  Their typed material still requires the particle VF
			   to provide atlas coordinates, so derive a deterministic current/next
			   frame from the editable generic UV atlas and particle lifetime.  The
			   typed opcodes consume the current frame only; keeping blend at zero
			   also preserves active030's admitted packet contract. */
			const f32_t fLife = std::clamp(
				Particle.fNormalizedLife, 0.f, 1.f);
			f32_t fFrame = static_cast<f32_t>(UV.iTileIndex) +
				fLife * static_cast<f32_t>(
					UV.bLoop ? iFrameCount : iFrameCount - 1u);
			if (!UV.bLoop)
			{
				fFrame = (std::min)(fFrame,
					static_cast<f32_t>(iFrameCount - 1u));
			}
			return Client::CEffectPlayback::Resolve_SourceSubUVFrame(
				iColumns, iRows, fFrame, false, false,
				Particle.fDistributionRandom, false);
		}
		/* SubUV interpolation is a particle-VF property carried by Required,
		   not a material-backend property. Portable authored Track A recipes
		   intentionally clear native SourceMaterial after lowering to a typed
		   execution packet, so requiring that profile here discarded otherwise
		   complete 6x6/2x2 frame programs. */
		// Submission groups particles by element. Preserve the standalone
		// particle contract if a caller ever supplies a different owner.
		const SOURCE_SUBUV_LAYOUT Layout =
			BatchLayout.pElement == Particle.pElement ? BatchLayout :
			Resolve_SourceSubUVLayout(*Particle.pElement);
		if (!Layout.bEnabled)
			return {};
		/* The evaluated signed Size is the sole image-flip authority.  Build an
		   unmirrored atlas frame here; the shared Playback helper applies those
		   signs once after billboard geometry uses positive extents. */
		return Client::CEffectPlayback::Resolve_SourceSubUVFrame(
			Layout.iColumns, Layout.iRows, Particle.fSubImageIndex,
			false, false, Particle.fDistributionRandom,
			Layout.bLinearBlend);
	}
}
