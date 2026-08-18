#include "DestructionSimulationRuntime.h"

#include "DeployPropObject.h"
#include "DeployPropRuntime.h"
#include "GameInstance.h"
#include "RigidBody.h"
#include "WorldDestructionDocument.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace
{
	constexpr f32_t PREVIEW_GROUND_HALF_THICKNESS = 0.25f;
	constexpr f32_t MINIMUM_SHAPE_HALF_EXTENT = 0.01f;
	constexpr f32_t PI = 3.14159265358979323846f;
	constexpr f32_t DEBRIS_DIRECTION_SPREAD_RADIANS = 28.f * PI / 180.f;
	constexpr f32_t DEBRIS_UPWARD_SPEED_METERS_PER_SECOND = 3.25f;
	constexpr f32_t DEBRIS_MIN_SPEED_SCALE = 0.8f;
	constexpr f32_t DEBRIS_MAX_SPEED_SCALE = 1.2f;
	constexpr f32_t DEBRIS_MIN_ANGULAR_SPEED_RADIANS_PER_SECOND = 5.f;
	constexpr f32_t DEBRIS_MAX_ANGULAR_SPEED_RADIANS_PER_SECOND = 11.f;
	constexpr f32_t DEBRIS_MIN_VISUAL_SCALE = 0.8f;
	constexpr f32_t DEBRIS_MAX_VISUAL_SCALE = 1.2f;
	constexpr uint32_t PREVIEW_GROUND_COLLISION_GROUP = 1u << 0u;
	constexpr uint32_t PREVIEW_DEBRIS_COLLISION_GROUP = 1u << 1u;
	std::string Build_FragmentId(
		const std::string& elementId,
		const uint32_t pieceIndex)
	{
		return elementId + ".fragment." +
			(pieceIndex < 10u ? "0" : "") + std::to_string(pieceIndex);
	}

	uint64_t Hash_Bytes(uint64_t seed, const std::string_view value)
	{
		constexpr uint64_t FNV_PRIME = 1099511628211ull;
		for (const unsigned char byte : value)
		{
			seed ^= static_cast<uint64_t>(byte);
			seed *= FNV_PRIME;
		}
		return seed;
	}

	uint64_t Build_DebrisSeed(
		const std::string& profileId,
		const std::string& elementId,
		const uint32_t pieceIndex)
	{
		uint64_t seed = 14695981039346656037ull;
		seed = Hash_Bytes(seed, profileId);
		seed = Hash_Bytes(seed, elementId);
		for (uint32_t shift = 0u; shift < 32u; shift += 8u)
		{
			seed ^= static_cast<uint64_t>((pieceIndex >> shift) & 0xffu);
			seed *= 1099511628211ull;
		}
		return 0u == seed ? 0x9e3779b97f4a7c15ull : seed;
	}

	uint64_t Next_Random(uint64_t& state)
	{
		state ^= state >> 12u;
		state ^= state << 25u;
		state ^= state >> 27u;
		return state * 2685821657736338717ull;
	}

	f32_t Random_Unit(uint64_t& state)
	{
		return static_cast<f32_t>((Next_Random(state) >> 40u) & 0xffffffu) /
			static_cast<f32_t>(0xffffffu);
	}

	f32_t Random_Signed(uint64_t& state)
	{
		return Random_Unit(state) * 2.f - 1.f;
	}

	float3_t Rotate_Vector(
		const float3_t& value,
		const float4_t& rotation)
	{
		float3_t result{};
		XMStoreFloat3(&result, XMVector3Rotate(
			XMLoadFloat3(&value),
			XMQuaternionNormalize(XMLoadFloat4(&rotation))));
		return result;
	}

	float3_t InverseRotate_Vector(
		const float3_t& value,
		const float4_t& rotation)
	{
		float3_t result{};
		XMStoreFloat3(&result, XMVector3InverseRotate(
			XMLoadFloat3(&value),
			XMQuaternionNormalize(XMLoadFloat4(&rotation))));
		return result;
	}

	float3_t Normalize_Vector(const float3_t& value)
	{
		float3_t result{};
		XMStoreFloat3(&result, XMVector3Normalize(XMLoadFloat3(&value)));
		return result;
	}

	float3_t Build_SpreadDirection(
		const float3_t& authoredDirection,
		uint64_t& randomState)
	{
		const vector_t forward = XMVector3Normalize(
			XMLoadFloat3(&authoredDirection));
		const f32_t upDot = std::abs(XMVectorGetX(XMVector3Dot(
			forward, XMVectorSet(0.f, 1.f, 0.f, 0.f))));
		const vector_t reference = upDot > 0.95f ?
			XMVectorSet(1.f, 0.f, 0.f, 0.f) :
			XMVectorSet(0.f, 1.f, 0.f, 0.f);
		const vector_t right = XMVector3Normalize(
			XMVector3Cross(reference, forward));
		const vector_t secondary = XMVector3Normalize(
			XMVector3Cross(forward, right));
		const f32_t angle = DEBRIS_DIRECTION_SPREAD_RADIANS *
			std::sqrt(Random_Unit(randomState));
		const f32_t azimuth = 2.f * PI * Random_Unit(randomState);
		const vector_t radial = right * std::cos(azimuth) +
			secondary * std::sin(azimuth);
		float3_t result{};
		XMStoreFloat3(&result, XMVector3Normalize(
			forward * std::cos(angle) + radial * std::sin(angle)));
		return result;
	}

	float4_t Build_RandomRotation(uint64_t& randomState)
	{
		float4_t result{};
		XMStoreFloat4(&result, XMQuaternionNormalize(
			XMQuaternionRotationRollPitchYaw(
				Random_Signed(randomState) * PI,
				Random_Signed(randomState) * PI,
				Random_Signed(randomState) * PI)));
		return result;
	}

	float3_t Build_RandomAngularVelocity(uint64_t& randomState)
	{
		float3_t axis = {
			Random_Signed(randomState),
			Random_Signed(randomState),
			Random_Signed(randomState)
		};
		const f32_t lengthSquared = axis.x * axis.x + axis.y * axis.y +
			axis.z * axis.z;
		if (lengthSquared <= 0.000001f)
			axis = { 0.f, 1.f, 0.f };
		else
			axis = Normalize_Vector(axis);
		const f32_t speed = DEBRIS_MIN_ANGULAR_SPEED_RADIANS_PER_SECOND +
			(DEBRIS_MAX_ANGULAR_SPEED_RADIANS_PER_SECOND -
				DEBRIS_MIN_ANGULAR_SPEED_RADIANS_PER_SECOND) *
			Random_Unit(randomState);
		return { axis.x * speed, axis.y * speed, axis.z * speed };
	}

	const Client::DEPLOY_RUNTIME_ENTRY* Find_Entry(
		const Client::CDeployPropRuntime& runtime,
		const uint64_t placementId)
	{
		const auto& entries = runtime.Get_Entries();
		const auto found = std::find_if(entries.begin(), entries.end(),
			[placementId](const Client::DEPLOY_RUNTIME_ENTRY& entry)
			{
				return entry.placement.runtimePlacementId == placementId;
			});
		return found == entries.end() ? nullptr : &*found;
	}

	bool_t Is_FinitePose(
		const float3_t& position,
		const float4_t& rotation)
	{
		return std::isfinite(position.x) && std::isfinite(position.y) &&
			std::isfinite(position.z) && std::isfinite(rotation.x) &&
			std::isfinite(rotation.y) && std::isfinite(rotation.z) &&
			std::isfinite(rotation.w);
	}
}

const std::vector<Client::DESTRUCTION_SIMULATION_DEBRIS_MODEL_SPEC>&
Client::CDestructionSimulationRuntime::Get_ProjectAuthoredDebrisModelSpecs()
{
	static const std::vector<DESTRUCTION_SIMULATION_DEBRIS_MODEL_SPEC> specs = {
		{
			L"Prototype_Component_Model_DestructionProxy_ValtanStone001",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_001.wmodel",
			3.5f
		},
		{
			L"Prototype_Component_Model_DestructionProxy_ValtanStone002",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_002.wmodel",
			3.5f
		},
		{
			L"Prototype_Component_Model_DestructionProxy_ValtanStone004",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_004.wmodel",
			3.5f
		},
		{
			L"Prototype_Component_Model_DestructionProxy_ValtanStone010",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_010.wmodel",
			3.5f
		},
		{
			L"Prototype_Component_Model_DestructionWall_02316_Chunk00",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02316/fractured/DEPLOY_ITR_02316_CHUNK_00.wmodel",
			1.f, "DEPLOY_ITR_02316", { -0.299745076f, 0.074321074f, -0.651884384f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02316_Chunk01",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02316/fractured/DEPLOY_ITR_02316_CHUNK_01.wmodel",
			1.f, "DEPLOY_ITR_02316", { -0.106431940f, 0.070914871f, -0.242190697f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02316_Chunk02",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02316/fractured/DEPLOY_ITR_02316_CHUNK_02.wmodel",
			1.f, "DEPLOY_ITR_02316", { 0.140550723f, 0.125839876f, 0.348527786f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02316_Chunk03",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02316/fractured/DEPLOY_ITR_02316_CHUNK_03.wmodel",
			1.f, "DEPLOY_ITR_02316", { -0.304660486f, 0.672847632f, -0.626061295f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02316_Chunk04",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02316/fractured/DEPLOY_ITR_02316_CHUNK_04.wmodel",
			1.f, "DEPLOY_ITR_02316", { -0.085336233f, 0.779443420f, -0.253681080f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02316_Chunk05",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02316/fractured/DEPLOY_ITR_02316_CHUNK_05.wmodel",
			1.f, "DEPLOY_ITR_02316", { 0.117441025f, 0.693299927f, 0.100609192f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02316_Chunk06",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02316/fractured/DEPLOY_ITR_02316_CHUNK_06.wmodel",
			1.f, "DEPLOY_ITR_02316", { -0.214777685f, 1.639257667f, -0.575360006f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02316_Chunk07",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02316/fractured/DEPLOY_ITR_02316_CHUNK_07.wmodel",
			1.f, "DEPLOY_ITR_02316", { -0.066782238f, 1.632209446f, -0.270373189f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02316_Chunk08",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02316/fractured/DEPLOY_ITR_02316_CHUNK_08.wmodel",
			1.f, "DEPLOY_ITR_02316", { -0.030778841f, 1.591320606f, 0.024464214f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02316_Chunk09",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02316/fractured/DEPLOY_ITR_02316_CHUNK_09.wmodel",
			1.f, "DEPLOY_ITR_02316", { -0.304499721f, 2.687915050f, -0.565307807f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02316_Chunk10",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02316/fractured/DEPLOY_ITR_02316_CHUNK_10.wmodel",
			1.f, "DEPLOY_ITR_02316", { -0.101834067f, 2.797022654f, -0.245301745f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02316_Chunk11",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02316/fractured/DEPLOY_ITR_02316_CHUNK_11.wmodel",
			1.f, "DEPLOY_ITR_02316", { 0.057664415f, 2.775639976f, -0.103123999f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02315_Chunk00",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02315/fractured/DEPLOY_ITR_02315_CHUNK_00.wmodel",
			1.f, "DEPLOY_ITR_02315", { -0.033496052f, 0.204034559f, -1.096159982f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02315_Chunk01",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02315/fractured/DEPLOY_ITR_02315_CHUNK_01.wmodel",
			1.f, "DEPLOY_ITR_02315", { -0.016332751f, 0.202313194f, 0.146141227f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02315_Chunk02",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02315/fractured/DEPLOY_ITR_02315_CHUNK_02.wmodel",
			1.f, "DEPLOY_ITR_02315", { 0.022437251f, 0.207860004f, 1.311927325f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02315_Chunk03",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02315/fractured/DEPLOY_ITR_02315_CHUNK_03.wmodel",
			1.f, "DEPLOY_ITR_02315", { -0.076481746f, 0.642837210f, -1.014285155f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02315_Chunk04",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02315/fractured/DEPLOY_ITR_02315_CHUNK_04.wmodel",
			1.f, "DEPLOY_ITR_02315", { -0.044129217f, 0.670340510f, 0.160370814f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02315_Chunk05",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02315/fractured/DEPLOY_ITR_02315_CHUNK_05.wmodel",
			1.f, "DEPLOY_ITR_02315", { 0.005046527f, 0.635590525f, 1.241675034f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02315_Chunk06",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02315/fractured/DEPLOY_ITR_02315_CHUNK_06.wmodel",
			1.f, "DEPLOY_ITR_02315", { -0.063802826f, 0.850468018f, -0.897982432f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02315_Chunk07",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02315/fractured/DEPLOY_ITR_02315_CHUNK_07.wmodel",
			1.f, "DEPLOY_ITR_02315", { -0.087943051f, 0.846026086f, 0.124011051f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02315_Chunk08",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02315/fractured/DEPLOY_ITR_02315_CHUNK_08.wmodel",
			1.f, "DEPLOY_ITR_02315", { -0.043536248f, 0.844825906f, 1.066360374f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02315_Chunk09",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02315/fractured/DEPLOY_ITR_02315_CHUNK_09.wmodel",
			1.f, "DEPLOY_ITR_02315", { -0.119356901f, 1.221537122f, -0.805073882f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02315_Chunk10",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02315/fractured/DEPLOY_ITR_02315_CHUNK_10.wmodel",
			1.f, "DEPLOY_ITR_02315", { -0.121444100f, 1.117028897f, 0.113987303f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02315_Chunk11",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02315/fractured/DEPLOY_ITR_02315_CHUNK_11.wmodel",
			1.f, "DEPLOY_ITR_02315", { -0.021922151f, 1.126421284f, 0.996424876f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02306_Chunk00",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02306/fractured/DEPLOY_ITR_02306_CHUNK_00.wmodel",
			1.f, "DEPLOY_ITR_02306", { -0.413609834f, 0.502041143f, 0.014695302f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02306_Chunk01",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02306/fractured/DEPLOY_ITR_02306_CHUNK_01.wmodel",
			1.f, "DEPLOY_ITR_02306", { 0.206001368f, 0.498764852f, 0.031917819f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02306_Chunk02",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02306/fractured/DEPLOY_ITR_02306_CHUNK_02.wmodel",
			1.f, "DEPLOY_ITR_02306", { 0.786687923f, 0.490523317f, -0.037660099f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02306_Chunk03",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02306/fractured/DEPLOY_ITR_02306_CHUNK_03.wmodel",
			1.f, "DEPLOY_ITR_02306", { -0.344671356f, 1.870687497f, 0.003419492f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02306_Chunk04",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02306/fractured/DEPLOY_ITR_02306_CHUNK_04.wmodel",
			1.f, "DEPLOY_ITR_02306", { 0.223126491f, 1.920743282f, -0.000579594f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02306_Chunk05",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02306/fractured/DEPLOY_ITR_02306_CHUNK_05.wmodel",
			1.f, "DEPLOY_ITR_02306", { 0.716736253f, 1.808157311f, -0.013036868f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02306_Chunk06",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02306/fractured/DEPLOY_ITR_02306_CHUNK_06.wmodel",
			1.f, "DEPLOY_ITR_02306", { -0.382843584f, 3.456244987f, -0.008279195f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02306_Chunk07",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02306/fractured/DEPLOY_ITR_02306_CHUNK_07.wmodel",
			1.f, "DEPLOY_ITR_02306", { 0.226871493f, 3.463111220f, 0.014070998f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02306_Chunk08",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02306/fractured/DEPLOY_ITR_02306_CHUNK_08.wmodel",
			1.f, "DEPLOY_ITR_02306", { 0.716452582f, 3.462258880f, -0.028121681f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02306_Chunk09",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02306/fractured/DEPLOY_ITR_02306_CHUNK_09.wmodel",
			1.f, "DEPLOY_ITR_02306", { -0.443971466f, 4.413128896f, 0.007977752f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02306_Chunk10",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02306/fractured/DEPLOY_ITR_02306_CHUNK_10.wmodel",
			1.f, "DEPLOY_ITR_02306", { 0.237917975f, 4.597841509f, 0.001708896f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02306_Chunk11",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02306/fractured/DEPLOY_ITR_02306_CHUNK_11.wmodel",
			1.f, "DEPLOY_ITR_02306", { 2.265354279f, 6.168880197f, -0.000246972f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02307_Chunk00",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02307/fractured/DEPLOY_ITR_02307_CHUNK_00.wmodel",
			1.f, "DEPLOY_ITR_02307", { -0.585852233f, 0.419190480f, 0.027871948f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02307_Chunk01",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02307/fractured/DEPLOY_ITR_02307_CHUNK_01.wmodel",
			1.f, "DEPLOY_ITR_02307", { 0.011797362f, 0.380123801f, -0.029185121f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02307_Chunk02",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02307/fractured/DEPLOY_ITR_02307_CHUNK_02.wmodel",
			1.f, "DEPLOY_ITR_02307", { 0.567135085f, 0.422135281f, 0.042870590f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02307_Chunk03",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02307/fractured/DEPLOY_ITR_02307_CHUNK_03.wmodel",
			1.f, "DEPLOY_ITR_02307", { -0.515047040f, 1.385693281f, -0.001939032f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02307_Chunk04",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02307/fractured/DEPLOY_ITR_02307_CHUNK_04.wmodel",
			1.f, "DEPLOY_ITR_02307", { -0.011219164f, 1.462219338f, -0.029601135f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02307_Chunk05",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02307/fractured/DEPLOY_ITR_02307_CHUNK_05.wmodel",
			1.f, "DEPLOY_ITR_02307", { 0.514072051f, 1.348621066f, -0.005922144f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02307_Chunk06",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02307/fractured/DEPLOY_ITR_02307_CHUNK_06.wmodel",
			1.f, "DEPLOY_ITR_02307", { -0.496609403f, 2.983730951f, -0.025763108f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02307_Chunk07",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02307/fractured/DEPLOY_ITR_02307_CHUNK_07.wmodel",
			1.f, "DEPLOY_ITR_02307", { -0.011199639f, 2.916075232f, 0.009701721f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02307_Chunk08",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02307/fractured/DEPLOY_ITR_02307_CHUNK_08.wmodel",
			1.f, "DEPLOY_ITR_02307", { 0.466735315f, 2.999294838f, -0.003009702f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02307_Chunk09",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02307/fractured/DEPLOY_ITR_02307_CHUNK_09.wmodel",
			1.f, "DEPLOY_ITR_02307", { -0.474510838f, 3.894155375f, 0.005616023f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02307_Chunk10",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02307/fractured/DEPLOY_ITR_02307_CHUNK_10.wmodel",
			1.f, "DEPLOY_ITR_02307", { 0.004285712f, 4.035916554f, 0.000064679f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02307_Chunk11",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02307/fractured/DEPLOY_ITR_02307_CHUNK_11.wmodel",
			1.f, "DEPLOY_ITR_02307", { 0.456327499f, 3.874061156f, -0.000170814f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02308_Chunk00",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02308/fractured/DEPLOY_ITR_02308_CHUNK_00.wmodel",
			1.f, "DEPLOY_ITR_02308", { 0.074499871f, 0.386916761f, -1.686325331f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02308_Chunk01",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02308/fractured/DEPLOY_ITR_02308_CHUNK_01.wmodel",
			1.f, "DEPLOY_ITR_02308", { -0.004715975f, 0.347878380f, -0.057331228f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02308_Chunk02",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02308/fractured/DEPLOY_ITR_02308_CHUNK_02.wmodel",
			1.f, "DEPLOY_ITR_02308", { 0.078319845f, 0.379259147f, 1.584189110f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02308_Chunk03",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02308/fractured/DEPLOY_ITR_02308_CHUNK_03.wmodel",
			1.f, "DEPLOY_ITR_02308", { 0.182887240f, 1.341541559f, -1.647869051f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02308_Chunk04",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02308/fractured/DEPLOY_ITR_02308_CHUNK_04.wmodel",
			1.f, "DEPLOY_ITR_02308", { -0.175817378f, 1.373196202f, -0.051017456f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02308_Chunk05",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02308/fractured/DEPLOY_ITR_02308_CHUNK_05.wmodel",
			1.f, "DEPLOY_ITR_02308", { 0.194124203f, 1.368392160f, 1.743283575f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02308_Chunk06",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02308/fractured/DEPLOY_ITR_02308_CHUNK_06.wmodel",
			1.f, "DEPLOY_ITR_02308", { 0.322830517f, 2.251236420f, -1.649542701f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02308_Chunk07",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02308/fractured/DEPLOY_ITR_02308_CHUNK_07.wmodel",
			1.f, "DEPLOY_ITR_02308", { 0.263706754f, 2.282005394f, 0.012970754f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02308_Chunk08",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02308/fractured/DEPLOY_ITR_02308_CHUNK_08.wmodel",
			1.f, "DEPLOY_ITR_02308", { 0.378998144f, 2.279920078f, 1.647072685f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02308_Chunk09",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02308/fractured/DEPLOY_ITR_02308_CHUNK_09.wmodel",
			1.f, "DEPLOY_ITR_02308", { 0.307291099f, 3.009768894f, -1.586292113f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02308_Chunk10",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02308/fractured/DEPLOY_ITR_02308_CHUNK_10.wmodel",
			1.f, "DEPLOY_ITR_02308", { 0.296702922f, 2.971980638f, 0.012278425f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02308_Chunk11",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02308/fractured/DEPLOY_ITR_02308_CHUNK_11.wmodel",
			1.f, "DEPLOY_ITR_02308", { 0.316204455f, 3.006900205f, 1.564351069f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02309_Chunk00",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02309/fractured/DEPLOY_ITR_02309_CHUNK_00.wmodel",
			1.f, "DEPLOY_ITR_02309", { 0.073325735f, 0.677552147f, 2.324659396f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02309_Chunk01",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02309/fractured/DEPLOY_ITR_02309_CHUNK_01.wmodel",
			1.f, "DEPLOY_ITR_02309", { -0.102490311f, 0.570092539f, 0.210167638f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02309_Chunk02",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02309/fractured/DEPLOY_ITR_02309_CHUNK_02.wmodel",
			1.f, "DEPLOY_ITR_02309", { 0.117411292f, 0.628846063f, -2.205660784f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02309_Chunk03",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02309/fractured/DEPLOY_ITR_02309_CHUNK_03.wmodel",
			1.f, "DEPLOY_ITR_02309", { 0.176356617f, 1.833057845f, 2.248257987f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02309_Chunk04",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02309/fractured/DEPLOY_ITR_02309_CHUNK_04.wmodel",
			1.f, "DEPLOY_ITR_02309", { -0.063706070f, 1.808965217f, 0.128822783f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02309_Chunk05",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02309/fractured/DEPLOY_ITR_02309_CHUNK_05.wmodel",
			1.f, "DEPLOY_ITR_02309", { 0.176218776f, 1.808509897f, -2.256522781f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02309_Chunk06",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02309/fractured/DEPLOY_ITR_02309_CHUNK_06.wmodel",
			1.f, "DEPLOY_ITR_02309", { 0.310044094f, 2.898194620f, 2.326502971f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02309_Chunk07",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02309/fractured/DEPLOY_ITR_02309_CHUNK_07.wmodel",
			1.f, "DEPLOY_ITR_02309", { 0.220534921f, 2.907076210f, 0.145195755f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02309_Chunk08",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02309/fractured/DEPLOY_ITR_02309_CHUNK_08.wmodel",
			1.f, "DEPLOY_ITR_02309", { 0.318956850f, 2.889124480f, -2.238975701f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02309_Chunk09",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02309/fractured/DEPLOY_ITR_02309_CHUNK_09.wmodel",
			1.f, "DEPLOY_ITR_02309", { 0.283080723f, 3.727127999f, 2.250254531f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02309_Chunk10",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02309/fractured/DEPLOY_ITR_02309_CHUNK_10.wmodel",
			1.f, "DEPLOY_ITR_02309", { 0.242067212f, 3.741598931f, 0.178424930f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02309_Chunk11",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02309/fractured/DEPLOY_ITR_02309_CHUNK_11.wmodel",
			1.f, "DEPLOY_ITR_02309", { 0.311613830f, 3.683327563f, -2.184207953f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02310_Chunk00",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02310/fractured/DEPLOY_ITR_02310_CHUNK_00.wmodel",
			1.f, "DEPLOY_ITR_02310", { -0.144224395f, 0.645974287f, 2.212936597f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02310_Chunk01",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02310/fractured/DEPLOY_ITR_02310_CHUNK_01.wmodel",
			1.f, "DEPLOY_ITR_02310", { -0.083607712f, 0.676070559f, -0.127374782f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02310_Chunk02",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02310/fractured/DEPLOY_ITR_02310_CHUNK_02.wmodel",
			1.f, "DEPLOY_ITR_02310", { -0.193558835f, 0.662441474f, -2.302550143f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02310_Chunk03",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02310/fractured/DEPLOY_ITR_02310_CHUNK_03.wmodel",
			1.f, "DEPLOY_ITR_02310", { 0.002123307f, 1.754389499f, 2.149854229f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02310_Chunk04",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02310/fractured/DEPLOY_ITR_02310_CHUNK_04.wmodel",
			1.f, "DEPLOY_ITR_02310", { -0.146820287f, 1.734151878f, -0.104092730f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02310_Chunk05",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02310/fractured/DEPLOY_ITR_02310_CHUNK_05.wmodel",
			1.f, "DEPLOY_ITR_02310", { -0.214844601f, 1.706374078f, -2.379587929f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02310_Chunk06",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02310/fractured/DEPLOY_ITR_02310_CHUNK_06.wmodel",
			1.f, "DEPLOY_ITR_02310", { -0.132985725f, 2.856021812f, 2.201294314f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02310_Chunk07",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02310/fractured/DEPLOY_ITR_02310_CHUNK_07.wmodel",
			1.f, "DEPLOY_ITR_02310", { 0.024623762f, 2.840472211f, -0.046166610f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02310_Chunk08",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02310/fractured/DEPLOY_ITR_02310_CHUNK_08.wmodel",
			1.f, "DEPLOY_ITR_02310", { -0.060246357f, 2.870232273f, -2.447429824f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02310_Chunk09",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02310/fractured/DEPLOY_ITR_02310_CHUNK_09.wmodel",
			1.f, "DEPLOY_ITR_02310", { -0.145861477f, 3.754597375f, 2.301883577f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02310_Chunk10",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02310/fractured/DEPLOY_ITR_02310_CHUNK_10.wmodel",
			1.f, "DEPLOY_ITR_02310", { 0.033351245f, 3.752719969f, 0.016963585f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02310_Chunk11",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02310/fractured/DEPLOY_ITR_02310_CHUNK_11.wmodel",
			1.f, "DEPLOY_ITR_02310", { -0.019418219f, 3.763921271f, -2.311486805f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02311_Chunk00",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02311/fractured/DEPLOY_ITR_02311_CHUNK_00.wmodel",
			1.f, "DEPLOY_ITR_02311", { -0.014174476f, 0.199173338f, -2.948491269f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02311_Chunk01",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02311/fractured/DEPLOY_ITR_02311_CHUNK_01.wmodel",
			1.f, "DEPLOY_ITR_02311", { 0.311911016f, 0.158361609f, 0.137515049f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02311_Chunk02",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02311/fractured/DEPLOY_ITR_02311_CHUNK_02.wmodel",
			1.f, "DEPLOY_ITR_02311", { 0.313310977f, 0.238010426f, 3.131556558f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02311_Chunk03",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02311/fractured/DEPLOY_ITR_02311_CHUNK_03.wmodel",
			1.f, "DEPLOY_ITR_02311", { -0.036693606f, 1.597539832f, -3.014707970f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02311_Chunk04",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02311/fractured/DEPLOY_ITR_02311_CHUNK_04.wmodel",
			1.f, "DEPLOY_ITR_02311", { 0.207935500f, 1.633287492f, -0.124113489f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02311_Chunk05",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02311/fractured/DEPLOY_ITR_02311_CHUNK_05.wmodel",
			1.f, "DEPLOY_ITR_02311", { 0.089330470f, 1.567338439f, 3.236542540f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02311_Chunk06",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02311/fractured/DEPLOY_ITR_02311_CHUNK_06.wmodel",
			1.f, "DEPLOY_ITR_02311", { -0.177695469f, 3.014410616f, -2.943148775f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02311_Chunk07",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02311/fractured/DEPLOY_ITR_02311_CHUNK_07.wmodel",
			1.f, "DEPLOY_ITR_02311", { 0.426726987f, 3.036723726f, 0.039974364f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02311_Chunk08",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02311/fractured/DEPLOY_ITR_02311_CHUNK_08.wmodel",
			1.f, "DEPLOY_ITR_02311", { 0.271216621f, 3.048757692f, 3.164656944f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02311_Chunk09",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02311/fractured/DEPLOY_ITR_02311_CHUNK_09.wmodel",
			1.f, "DEPLOY_ITR_02311", { -0.273329594f, 4.002954292f, -2.969686619f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02311_Chunk10",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02311/fractured/DEPLOY_ITR_02311_CHUNK_10.wmodel",
			1.f, "DEPLOY_ITR_02311", { -0.017746531f, 3.988279169f, 0.064272791f }
		},
		{
			L"Prototype_Component_Model_DestructionWall_02311_Chunk11",
			"Deploy/LV_LUT_HEARTRB_ED/DEPLOY_ITR_02311/fractured/DEPLOY_ITR_02311_CHUNK_11.wmodel",
			1.f, "DEPLOY_ITR_02311", { -0.078028878f, 3.978823340f, 3.146037265f }
		},
		/* The arena floor owns no fractured mesh, so its collapse reuses the
		   four Valtan rubble meshes already shipped for destruction debris. The
		   twelve pivots per sector are measured median-radius nav cells of that
		   sector's authored collapse region, spread over its angular span, so the
		   rubble appears across the floor that is actually giving way instead of
		   at the arena centre. The yaw180 twin of each sector reuses the same
		   local pivots because its region is the 180-degree mirror. */
		{
			L"Prototype_Component_Model_DestructionFloor_Rail_Chunk00",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_001.wmodel",
			3.5f, "VALTAN_FLOOR_RAIL", { 13.970993000f, 0.250000000f, -4.773003000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_Rail_Chunk01",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_002.wmodel",
			3.5f, "VALTAN_FLOOR_RAIL", { 14.470993000f, 0.250000000f, 1.226997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_Rail_Chunk02",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_004.wmodel",
			3.5f, "VALTAN_FLOOR_RAIL", { 13.970993000f, 0.250000000f, 4.226997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_Rail_Chunk03",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_010.wmodel",
			3.5f, "VALTAN_FLOOR_RAIL", { 13.470993000f, 0.250000000f, 5.726997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_Rail_Chunk04",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_001.wmodel",
			3.5f, "VALTAN_FLOOR_RAIL", { 10.470993000f, 0.250000000f, 10.226997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_Rail_Chunk05",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_002.wmodel",
			3.5f, "VALTAN_FLOOR_RAIL", { 5.970993000f, 0.250000000f, 13.226997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_Rail_Chunk06",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_004.wmodel",
			3.5f, "VALTAN_FLOOR_RAIL", { 4.970993000f, 0.250000000f, 13.726997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_Rail_Chunk07",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_010.wmodel",
			3.5f, "VALTAN_FLOOR_RAIL", { -0.029007000f, 0.250000000f, 14.726997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_Rail_Chunk08",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_001.wmodel",
			3.5f, "VALTAN_FLOOR_RAIL", { -3.029007000f, 0.250000000f, 14.226997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_Rail_Chunk09",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_002.wmodel",
			3.5f, "VALTAN_FLOOR_RAIL", { -8.529007000f, 0.250000000f, 12.226997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_Rail_Chunk10",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_004.wmodel",
			3.5f, "VALTAN_FLOOR_RAIL", { -11.529007000f, 0.250000000f, 9.726997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_Rail_Chunk11",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_010.wmodel",
			3.5f, "VALTAN_FLOOR_RAIL", { -13.529007000f, 0.250000000f, 5.726997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickA_Chunk00",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_001.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_A", { 11.470993000f, 0.250000000f, -2.773003000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickA_Chunk01",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_002.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_A", { 10.970993000f, 0.250000000f, -1.273003000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickA_Chunk02",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_004.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_A", { 11.470993000f, 0.250000000f, -0.273003000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickA_Chunk03",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_010.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_A", { 10.970993000f, 0.250000000f, 1.726997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickA_Chunk04",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_001.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_A", { 10.970993000f, 0.250000000f, 2.226997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickA_Chunk05",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_002.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_A", { 10.470993000f, 0.250000000f, 4.226997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickA_Chunk06",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_004.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_A", { 9.970993000f, 0.250000000f, 5.226997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickA_Chunk07",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_010.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_A", { 9.470993000f, 0.250000000f, 6.226997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickA_Chunk08",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_001.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_A", { 8.470993000f, 0.250000000f, 7.726997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickA_Chunk09",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_002.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_A", { 7.470993000f, 0.250000000f, 8.226997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickA_Chunk10",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_004.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_A", { 6.470993000f, 0.250000000f, 9.226997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickA_Chunk11",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_010.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_A", { 4.970993000f, 0.250000000f, 9.726997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickB_Chunk00",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_001.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_B", { 4.470993000f, 0.250000000f, 10.726997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickB_Chunk01",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_002.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_B", { 1.970993000f, 0.250000000f, 11.226997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickB_Chunk02",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_004.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_B", { 0.470993000f, 0.250000000f, 11.226997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickB_Chunk03",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_010.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_B", { -1.029007000f, 0.250000000f, 11.226997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickB_Chunk04",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_001.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_B", { -2.529007000f, 0.250000000f, 10.726997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickB_Chunk05",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_002.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_B", { -4.029007000f, 0.250000000f, 10.726997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickB_Chunk06",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_004.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_B", { -5.529007000f, 0.250000000f, 9.726997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickB_Chunk07",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_010.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_B", { -7.029007000f, 0.250000000f, 8.726997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickB_Chunk08",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_001.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_B", { -8.029007000f, 0.250000000f, 8.226997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickB_Chunk09",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_002.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_B", { -9.029007000f, 0.250000000f, 6.726997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickB_Chunk10",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_004.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_B", { -10.029007000f, 0.250000000f, 4.726997000f }
		},
		{
			L"Prototype_Component_Model_DestructionFloor_BrickB_Chunk11",
			"Effect/Valtan/Meshes/FX_SM_00/fm_a_stone_010.wmodel",
			3.5f, "VALTAN_FLOOR_BRICK_B", { -10.529007000f, 0.250000000f, 3.226997000f }
		}
	};
	return specs;
}

class Client::CDestructionSimulationRuntime::CPhysicsAdapter final
{
public:
	~CPhysicsAdapter()
	{
		Clear_Ground();
		End_ExclusiveClock();
	}

	bool_t Begin_ExclusiveClock(std::string& outStatus)
	{
		if (m_ownsClock)
			return true;
		m_pManager = CGameInstance::Get().Get_PhysicsManager();
		if (nullptr == m_pManager)
		{
			outStatus = "Physics manager is unavailable for destruction preview";
			return false;
		}
		m_wasPaused = m_pManager->Get_StepStats().isPaused;
		m_pManager->Set_DebugPaused(true);
		m_ownsClock = true;
		return true;
	}

	void End_ExclusiveClock()
	{
		Engine::CPhysics_Manager* current =
			CGameInstance::Get().Get_PhysicsManager();
		if (m_ownsClock && nullptr != m_pManager && current == m_pManager)
			current->Set_DebugPaused(m_wasPaused);
		m_pManager = nullptr;
		m_ownsClock = false;
		m_wasPaused = false;
	}

	bool_t Simulate_Steps(
		const uint32_t stepCount,
		std::string& outStatus)
	{
		Engine::CPhysics_Manager* current =
			CGameInstance::Get().Get_PhysicsManager();
		if (!m_ownsClock || nullptr == m_pManager || current != m_pManager ||
			0u == stepCount ||
			FAILED(current->Simulate_DebugSteps(stepCount)))
		{
			outStatus = "PhysX rejected synchronous destruction preview steps";
			return false;
		}
		return true;
	}

	bool_t Create_DynamicBox(
		const uint32_t levelId,
		const float3_t& position,
		const float4_t& rotation,
		const float3_t& shapeLocalCentre,
		const float3_t& halfExtents,
		const float3_t& linearVelocity,
		const float3_t& angularVelocity,
		const f32_t gravityScale,
		shared_ptr<Engine::CRigidBody>& outBody,
		std::string& outStatus)
	{
		Engine::PHYSICS_RIGID_BODY_DESC desc{};
		desc.iLevelID = levelId;
		desc.eActorType = Engine::PHYSICS_ACTOR_TYPE::DYNAMIC;
		desc.eMotionMode = Engine::PHYSICS_MOTION_MODE::SIMULATED;
		desc.eShapeType = Engine::PHYSICS_SHAPE_TYPE::BOX;
		desc.Pose.vPosition = position;
		desc.Pose.vRotationQuaternion = rotation;
		desc.ShapeLocalPose.vPosition = shapeLocalCentre;
		desc.vBoxHalfExtents = {
			(std::max)(MINIMUM_SHAPE_HALF_EXTENT, halfExtents.x),
			(std::max)(MINIMUM_SHAPE_HALF_EXTENT, halfExtents.y),
			(std::max)(MINIMUM_SHAPE_HALF_EXTENT, halfExtents.z)
		};
		desc.fDensity = 10.f;
		desc.fStaticFriction = 0.6f;
		desc.fDynamicFriction = 0.5f;
		desc.fRestitution = 0.15f;
		desc.fLinearDamping = 0.05f;
		desc.fAngularDamping = 0.1f;
		desc.isGravityEnabled = gravityScale > 0.f;
		desc.fGravityScale = gravityScale;
		desc.isActive = true;
		/* Macro-shard AABBs touch in their assembled pose. Let them collide
		   with the preview ground/world but not explode from self-overlap before
		   the authored launch velocity has separated them. */
		desc.iCollisionGroup = PREVIEW_DEBRIS_COLLISION_GROUP;
		desc.iCollisionMask = PREVIEW_GROUND_COLLISION_GROUP;

		shared_ptr<Engine::CRigidBody> staged =
			Engine::CRigidBody::Create_Runtime(desc);
		if (nullptr == staged ||
			FAILED(staged->Set_LinearVelocity(linearVelocity)) ||
			FAILED(staged->Set_AngularVelocity(angularVelocity)))
		{
			if (nullptr != staged)
				staged->Destroy_Actor();
			outStatus = "PhysX rejected a destruction debris actor";
			return false;
		}
		outBody = std::move(staged);
		return true;
	}

	bool_t Create_Ground(
		const uint32_t levelId,
		const float3_t& centre,
		const f32_t topHeight,
		const float2_t& halfExtents,
		std::string& outStatus)
	{
		Engine::PHYSICS_RIGID_BODY_DESC desc{};
		desc.iLevelID = levelId;
		desc.eActorType = Engine::PHYSICS_ACTOR_TYPE::STATIC;
		desc.eMotionMode = Engine::PHYSICS_MOTION_MODE::SIMULATED;
		desc.eShapeType = Engine::PHYSICS_SHAPE_TYPE::BOX;
		desc.Pose.vPosition = {
			centre.x,
			topHeight - PREVIEW_GROUND_HALF_THICKNESS,
			centre.z
		};
		desc.vBoxHalfExtents = {
			halfExtents.x,
			PREVIEW_GROUND_HALF_THICKNESS,
			halfExtents.y
		};
		desc.isGravityEnabled = false;
		desc.fGravityScale = 0.f;
		desc.isActive = true;
		desc.iCollisionGroup = PREVIEW_GROUND_COLLISION_GROUP;
		desc.iCollisionMask = PREVIEW_DEBRIS_COLLISION_GROUP;
		m_pGround = Engine::CRigidBody::Create_Runtime(desc);
		if (nullptr == m_pGround)
		{
			outStatus = "PhysX rejected the tool-only preview ground";
			return false;
		}
		return true;
	}

	void Clear_Ground()
	{
		if (nullptr != m_pGround)
			m_pGround->Destroy_Actor();
		m_pGround.reset();
	}

private:
	shared_ptr<Engine::CRigidBody> m_pGround;
	Engine::CPhysics_Manager* m_pManager = nullptr;
	bool_t m_ownsClock = false;
	bool_t m_wasPaused = false;
};

struct Client::CDestructionSimulationRuntime::ELEMENT_RUNTIME final
{
	struct SUPPRESSION_ALIAS_RUNTIME final
	{
		uint64_t runtimePlacementId = 0u;
		shared_ptr<CDeployPropObject> pObject;
		DEPLOY_PROP_STATE ePreviousState = DEPLOY_PROP_STATE::INTACT;
		bool_t isSuppressed = false;
	};

	struct DEBRIS_PIECE_RUNTIME final
	{
		std::string fragmentId;
		std::string modelAssetId;
		uint32_t iVisualIndex = UINT32_MAX;
		shared_ptr<Engine::CRigidBody> pBody;
		float3_t vInitialPosition{};
		float4_t vInitialRotation = { 0.f, 0.f, 0.f, 1.f };
		float3_t vShapeLocalCentre{};
		float3_t vHalfExtents = { 0.05f, 0.05f, 0.05f };
		float3_t vInitialLinearVelocity{};
		float3_t vInitialAngularVelocity{};
		float3_t vCurrentPosition{};
		float4_t vCurrentRotation = { 0.f, 0.f, 0.f, 1.f };
		float3_t vCurrentLinearVelocity{};
		f32_t fActivatedAtSeconds = 0.f;
		DESTRUCTION_SIMULATION_ELEMENT_STATE eState =
			DESTRUCTION_SIMULATION_ELEMENT_STATE::WAITING;
	};

	DESTRUCTION_SIMULATION_ELEMENT Desc;
	shared_ptr<CDeployPropObject> pObject;
	float3_t vInitialPosition{};
	float4_t vInitialRotation = { 0.f, 0.f, 0.f, 1.f };
	float3_t vShapeLocalCentre{};
	float3_t vHalfExtents = { 0.5f, 0.5f, 0.5f };
	float3_t vCurrentPosition{};
	float4_t vCurrentRotation = { 0.f, 0.f, 0.f, 1.f };
	float3_t vLinearVelocity{};
	std::string sourceDeployAssetId;
	f32_t fPlacementUniformScale = 1.f;
	std::vector<SUPPRESSION_ALIAS_RUNTIME> SuppressionAliases;
	std::vector<DEBRIS_PIECE_RUNTIME> DebrisPieces;
	f32_t fActivatedAtSeconds = 0.f;
	DESTRUCTION_SIMULATION_ELEMENT_STATE eState =
		DESTRUCTION_SIMULATION_ELEMENT_STATE::WAITING;
};

Client::CDestructionSimulationRuntime::CDestructionSimulationRuntime()
	: m_pPhysics(std::make_unique<CPhysicsAdapter>())
{
}

Client::CDestructionSimulationRuntime::~CDestructionSimulationRuntime()
{
	Clear();
}

bool_t Client::CDestructionSimulationRuntime::Validate_Stage(
	const DESTRUCTION_SIMULATION_PROFILE& profile,
	const std::string& selectedGroupId,
	const CWorldDestructionDocument& destructionDocument,
	const CDeployPropRuntime& deployRuntime,
	std::string& outStatus) const
{
	if (!CDestructionSimulationDocument::Validate_Profile(profile, outStatus))
		return false;
	if (profile.groupId != selectedGroupId)
	{
		outStatus = "Simulation profile does not belong to the selected group";
		return false;
	}
	const DESTRUCTION_GROUP* group = destructionDocument.Find_Group(
		selectedGroupId);
	if (nullptr == group || group->memberPlacementIds.empty())
	{
		outStatus = "Selected destruction group is missing or empty";
		return false;
	}
	std::vector<uint64_t> projectedPlacementIds;
	projectedPlacementIds.reserve(group->memberPlacementIds.size());
	for (const DESTRUCTION_SIMULATION_ELEMENT& element : profile.Elements)
	{
		if (group->memberPlacementIds.end() == std::find(
			group->memberPlacementIds.begin(), group->memberPlacementIds.end(),
			element.sourceRuntimePlacementId))
		{
			outStatus = "Simulation element is outside the selected group: " +
				element.elementId;
			return false;
		}
		if (projectedPlacementIds.end() != std::find(
			projectedPlacementIds.begin(), projectedPlacementIds.end(),
			element.sourceRuntimePlacementId))
		{
			outStatus = "Simulation source placement is projected more than once: " +
				element.elementId;
			return false;
		}
		projectedPlacementIds.push_back(element.sourceRuntimePlacementId);
		const shared_ptr<CDeployPropObject> object = deployRuntime.Find(
			element.sourceRuntimePlacementId);
		float3_t localCentre{};
		float3_t halfExtents{};
		if (nullptr == object || !object->Is_Destructible() ||
			!object->Get_PhysicsPreviewLocalBounds(
				localCentre, halfExtents))
		{
			outStatus = "Simulation element has no loaded destructible bounds: " +
				element.elementId;
			return false;
		}
		for (const uint64_t aliasPlacementId :
			element.suppressionAliasPlacementIds)
		{
			if (group->memberPlacementIds.end() == std::find(
				group->memberPlacementIds.begin(),
				group->memberPlacementIds.end(), aliasPlacementId))
			{
				outStatus = "Simulation suppression alias is outside the selected group: " +
					element.elementId;
				return false;
			}
			if (projectedPlacementIds.end() != std::find(
				projectedPlacementIds.begin(), projectedPlacementIds.end(),
				aliasPlacementId))
			{
				outStatus = "Simulation placement is projected more than once: " +
					std::to_string(aliasPlacementId);
				return false;
			}
			const shared_ptr<CDeployPropObject> aliasObject =
				deployRuntime.Find(aliasPlacementId);
			if (nullptr == aliasObject || !aliasObject->Is_Destructible() ||
				!aliasObject->Is_StaticDeployModel())
			{
				outStatus =
					"Simulation suppression alias must be a loaded static destructible: " +
					std::to_string(aliasPlacementId);
				return false;
			}
			projectedPlacementIds.push_back(aliasPlacementId);
		}
	}
	if (projectedPlacementIds.size() != group->memberPlacementIds.size() ||
		std::any_of(group->memberPlacementIds.begin(),
			group->memberPlacementIds.end(),
			[&projectedPlacementIds](const uint64_t placementId)
			{
				return projectedPlacementIds.end() == std::find(
					projectedPlacementIds.begin(),
					projectedPlacementIds.end(), placementId);
			}))
	{
		outStatus =
			"Simulation elements and suppression aliases must project every selected group member";
		return false;
	}
	return true;
}

bool_t Client::CDestructionSimulationRuntime::Stage_Profile(
	const DESTRUCTION_SIMULATION_PROFILE& profile,
	const std::string& selectedGroupId,
	const CWorldDestructionDocument& destructionDocument,
	CDeployPropRuntime& deployRuntime,
	const uint32_t levelId,
	std::string& outStatus)
{
	if (levelId >= ETOUI(LEVEL::END) ||
		!Validate_Stage(profile, selectedGroupId, destructionDocument,
			deployRuntime, outStatus))
	{
		if (outStatus.empty())
			outStatus = "Destruction simulation stage input is invalid";
		return false;
	}

	std::vector<ELEMENT_RUNTIME> stagedElements;
	stagedElements.reserve(profile.Elements.size());
	float3_t groupCentre{};
	for (const DESTRUCTION_SIMULATION_ELEMENT& element : profile.Elements)
	{
		const DEPLOY_RUNTIME_ENTRY* entry = Find_Entry(
			deployRuntime, element.sourceRuntimePlacementId);
		if (nullptr == entry || nullptr == entry->object)
		{
			outStatus = "DeployProp runtime changed during simulation staging";
			return false;
		}
		float3_t localCentre{};
		float3_t boundsHalfExtents{};
		if (!entry->object->Get_PhysicsPreviewLocalBounds(
			localCentre, boundsHalfExtents))
		{
			outStatus = "DeployProp bounds changed during simulation staging";
			return false;
		}

		ELEMENT_RUNTIME runtime;
		runtime.Desc = element;
		runtime.pObject = entry->object;
		runtime.vInitialPosition = {
			entry->placement.position.x + element.vSpawnOffset.x,
			entry->placement.position.y + element.vSpawnOffset.y,
			entry->placement.position.z + element.vSpawnOffset.z
		};
		runtime.vInitialRotation = entry->placement.rotationQuaternion;
		runtime.vShapeLocalCentre = localCentre;
		runtime.vHalfExtents = boundsHalfExtents;
		runtime.vCurrentPosition = runtime.vInitialPosition;
		runtime.vCurrentRotation = runtime.vInitialRotation;
		runtime.vLinearVelocity = {
			element.vDirection.x * element.fSpeedMetersPerSecond,
			element.vDirection.y * element.fSpeedMetersPerSecond,
			element.vDirection.z * element.fSpeedMetersPerSecond
		};
		runtime.sourceDeployAssetId = entry->placement.assetId;
		runtime.fPlacementUniformScale = entry->placement.uniformScale;
		runtime.SuppressionAliases.reserve(
			element.suppressionAliasPlacementIds.size());
		for (const uint64_t aliasPlacementId :
			element.suppressionAliasPlacementIds)
		{
			const DEPLOY_RUNTIME_ENTRY* aliasEntry = Find_Entry(
				deployRuntime, aliasPlacementId);
			if (nullptr == aliasEntry || nullptr == aliasEntry->object ||
				!aliasEntry->object->Is_Destructible() ||
				!aliasEntry->object->Is_StaticDeployModel())
			{
				outStatus =
					"DeployProp suppression alias admission changed during simulation staging";
				return false;
			}
			ELEMENT_RUNTIME::SUPPRESSION_ALIAS_RUNTIME alias;
			alias.runtimePlacementId = aliasPlacementId;
			alias.pObject = aliasEntry->object;
			runtime.SuppressionAliases.push_back(std::move(alias));
		}
		groupCentre.x += entry->placement.position.x;
		groupCentre.y += entry->placement.position.y;
		groupCentre.z += entry->placement.position.z;
		stagedElements.push_back(std::move(runtime));
	}
	const f32_t inverseCount = 1.f /
		static_cast<f32_t>(stagedElements.size());
	groupCentre.x *= inverseCount;
	groupCentre.y *= inverseCount;
	groupCentre.z *= inverseCount;

	/* Actor creation uses the global Engine scene, but this preview owns its
	   clock while staged. Auto stepping stays paused and Controller performs
	   every exact 1/60 step synchronously. */
	Clear();
	unique_ptr<CPhysicsAdapter> stagedPhysics =
		std::make_unique<CPhysicsAdapter>();
	if (!stagedPhysics->Begin_ExclusiveClock(outStatus))
		return false;
	if (profile.isPreviewGroundEnabled &&
		!stagedPhysics->Create_Ground(
			levelId, groupCentre, profile.fPreviewGroundHeight,
			profile.vPreviewGroundHalfExtents, outStatus))
	{
		stagedPhysics->End_ExclusiveClock();
		return false;
	}

	const auto& modelSpecs = Get_ProjectAuthoredDebrisModelSpecs();
	if (modelSpecs.empty())
	{
		stagedPhysics->Clear_Ground();
		stagedPhysics->End_ExclusiveClock();
		outStatus = "Project-authored debris model recipe is empty";
		return false;
	}

	auto releaseStagedDebris = [&stagedElements]()
	{
		for (ELEMENT_RUNTIME& runtime : stagedElements)
		{
			if (nullptr != runtime.pObject &&
				runtime.pObject->Is_DebrisPreviewActive())
			{
				runtime.pObject->End_DebrisPreview();
			}
		}
	};
	bool_t usedGenericFallback = false;
	for (ELEMENT_RUNTIME& runtime : stagedElements)
	{
		std::vector<const DESTRUCTION_SIMULATION_DEBRIS_MODEL_SPEC*>
			exactModelSpecs;
		std::vector<const DESTRUCTION_SIMULATION_DEBRIS_MODEL_SPEC*>
			genericModelSpecs;
		for (const DESTRUCTION_SIMULATION_DEBRIS_MODEL_SPEC& spec : modelSpecs)
		{
			if (!spec.sourceDeployAssetId.empty() &&
				spec.sourceDeployAssetId == runtime.sourceDeployAssetId)
			{
				exactModelSpecs.push_back(&spec);
			}
			else if (spec.sourceDeployAssetId.empty())
			{
				genericModelSpecs.push_back(&spec);
			}
		}
		std::vector<const DESTRUCTION_SIMULATION_DEBRIS_MODEL_SPEC*>
			selectedModelSpecs = exactModelSpecs.empty() ?
			genericModelSpecs : exactModelSpecs;
		bool_t usesSourceWallGeometry = !exactModelSpecs.empty();
		if (selectedModelSpecs.empty())
		{
			releaseStagedDebris();
			stagedPhysics->Clear_Ground();
			stagedPhysics->End_ExclusiveClock();
			outStatus = "No admitted debris recipe matches Deploy asset " +
				runtime.sourceDeployAssetId;
			return false;
		}
		uint32_t pieceCount = 0u;
		CDeployPropObject::DEBRIS_PREVIEW_DESC previewDesc;
		std::vector<uint64_t> randomStates;
		auto beginSelectedRecipe = [&](std::string& outRecipeError)
		{
			pieceCount = usesSourceWallGeometry ?
				static_cast<uint32_t>(selectedModelSpecs.size()) :
				PROJECT_AUTHORED_DEBRIS_PIECES_PER_ELEMENT;
			previewDesc = CDeployPropObject::DEBRIS_PREVIEW_DESC{};
			/* Both the exact macro-shard recipe and the generic fallback replace
			   the primary Deploy presentation while their rigid debris is active. */
			previewDesc.suppressSource = true;
			previewDesc.instances.reserve(pieceCount);
			randomStates.clear();
			randomStates.reserve(pieceCount);
			for (uint32_t pieceIndex = 0u;
				pieceIndex < pieceCount;
				++pieceIndex)
			{
				uint64_t randomState = Build_DebrisSeed(
					profile.profileId, runtime.Desc.elementId, pieceIndex);
				const DESTRUCTION_SIMULATION_DEBRIS_MODEL_SPEC& modelSpec =
					*selectedModelSpecs[pieceIndex % selectedModelSpecs.size()];
				CDeployPropObject::DEBRIS_PREVIEW_INSTANCE_DESC instanceDesc;
				instanceDesc.modelPrototypeTag = modelSpec.prototypeTag;
				instanceDesc.uniformScale = usesSourceWallGeometry ?
					runtime.fPlacementUniformScale : DEBRIS_MIN_VISUAL_SCALE +
						(DEBRIS_MAX_VISUAL_SCALE - DEBRIS_MIN_VISUAL_SCALE) *
						Random_Unit(randomState);
				previewDesc.instances.push_back(std::move(instanceDesc));
				randomStates.push_back(randomState);
			}
			return runtime.pObject->Begin_DebrisPreview(
				previewDesc, outRecipeError) &&
				runtime.pObject->Get_DebrisPreviewInstanceCount() == pieceCount;
		};

		std::string recipeError;
		bool_t recipeReady = beginSelectedRecipe(recipeError);
		if (!recipeReady &&
			usesSourceWallGeometry && !genericModelSpecs.empty())
		{
			const std::string exactRecipeError = recipeError;
			selectedModelSpecs = genericModelSpecs;
			usesSourceWallGeometry = false;
			usedGenericFallback = true;
			std::string genericRecipeError;
			recipeReady = beginSelectedRecipe(genericRecipeError);
			if (!recipeReady)
			{
				recipeError = exactRecipeError +
					"; generic fallback failed: " + genericRecipeError;
			}
			else
				recipeError.clear();
		}
		if (!recipeReady)
		{
			releaseStagedDebris();
			stagedPhysics->Clear_Ground();
			stagedPhysics->End_ExclusiveClock();
			outStatus = "Project-authored debris prototypes are unavailable";
			if (!recipeError.empty())
				outStatus += ": " + recipeError;
			return false;
		}

		runtime.DebrisPieces.reserve(pieceCount);
		const float3_t localImpactDirection = InverseRotate_Vector(
			runtime.Desc.vDirection, runtime.vInitialRotation);
		for (uint32_t pieceIndex = 0u;
			pieceIndex < pieceCount;
			++pieceIndex)
		{
			ELEMENT_RUNTIME::DEBRIS_PIECE_RUNTIME piece;
			const DESTRUCTION_SIMULATION_DEBRIS_MODEL_SPEC& modelSpec =
				*selectedModelSpecs[pieceIndex % selectedModelSpecs.size()];
			piece.fragmentId = Build_FragmentId(
				runtime.Desc.elementId, pieceIndex);
			piece.modelAssetId = modelSpec.assetId;
			piece.iVisualIndex = pieceIndex;
			if (!runtime.pObject->Get_DebrisPreviewLocalBounds(
				pieceIndex, piece.vShapeLocalCentre, piece.vHalfExtents))
			{
				releaseStagedDebris();
				stagedPhysics->Clear_Ground();
				stagedPhysics->End_ExclusiveClock();
				outStatus = "Project-authored debris model has no usable bounds";
				return false;
			}

			uint64_t randomState = randomStates[pieceIndex];
			float3_t localSpawn = usesSourceWallGeometry ? float3_t{
				modelSpec.vSourceLocalPivotMeters.x *
					runtime.fPlacementUniformScale,
				modelSpec.vSourceLocalPivotMeters.y *
					runtime.fPlacementUniformScale,
				modelSpec.vSourceLocalPivotMeters.z *
					runtime.fPlacementUniformScale
			} : float3_t{
				runtime.vShapeLocalCentre.x + runtime.vHalfExtents.x *
					Random_Signed(randomState) * 0.75f,
				runtime.vShapeLocalCentre.y + runtime.vHalfExtents.y *
					Random_Signed(randomState) * 0.75f,
				runtime.vShapeLocalCentre.z + runtime.vHalfExtents.z *
					Random_Signed(randomState) * 0.75f
			};
			/* Bias roots toward the authored impact-facing surface. This keeps
			   the stones distributed over the visible wall volume without
			   making every actor emerge from the placement origin. */
			const f32_t surfaceFactor = 0.65f +
				Random_Unit(randomState) * 0.3f;
			const f32_t absX = std::abs(localImpactDirection.x);
			const f32_t absY = std::abs(localImpactDirection.y);
			const f32_t absZ = std::abs(localImpactDirection.z);
			if (!usesSourceWallGeometry && absX >= absY && absX >= absZ)
			{
				localSpawn.x = runtime.vShapeLocalCentre.x +
					(localImpactDirection.x < 0.f ? -1.f : 1.f) *
					runtime.vHalfExtents.x * surfaceFactor;
			}
			else if (!usesSourceWallGeometry && absY >= absZ)
			{
				localSpawn.y = runtime.vShapeLocalCentre.y +
					(localImpactDirection.y < 0.f ? -1.f : 1.f) *
					runtime.vHalfExtents.y * surfaceFactor;
			}
			else if (!usesSourceWallGeometry)
			{
				localSpawn.z = runtime.vShapeLocalCentre.z +
					(localImpactDirection.z < 0.f ? -1.f : 1.f) *
					runtime.vHalfExtents.z * surfaceFactor;
			}
			const float3_t worldSpawnOffset = Rotate_Vector(
				localSpawn, runtime.vInitialRotation);
			piece.vInitialPosition = {
				runtime.vInitialPosition.x + worldSpawnOffset.x,
				runtime.vInitialPosition.y + worldSpawnOffset.y,
				runtime.vInitialPosition.z + worldSpawnOffset.z
			};
			piece.vInitialRotation = usesSourceWallGeometry ?
				runtime.vInitialRotation : Build_RandomRotation(randomState);
			const float3_t spreadDirection = Build_SpreadDirection(
				runtime.Desc.vDirection, randomState);
			const f32_t speedScale = DEBRIS_MIN_SPEED_SCALE +
				(DEBRIS_MAX_SPEED_SCALE - DEBRIS_MIN_SPEED_SCALE) *
				Random_Unit(randomState);
			const f32_t upwardSpeed =
				DEBRIS_UPWARD_SPEED_METERS_PER_SECOND *
				(0.8f + Random_Unit(randomState) * 0.4f);
			piece.vInitialLinearVelocity = {
				spreadDirection.x * runtime.Desc.fSpeedMetersPerSecond *
					speedScale,
				spreadDirection.y * runtime.Desc.fSpeedMetersPerSecond *
					speedScale + upwardSpeed,
				spreadDirection.z * runtime.Desc.fSpeedMetersPerSecond *
					speedScale
			};
			piece.vInitialAngularVelocity =
				Build_RandomAngularVelocity(randomState);
			piece.vCurrentPosition = piece.vInitialPosition;
			piece.vCurrentRotation = piece.vInitialRotation;
			piece.vCurrentLinearVelocity = piece.vInitialLinearVelocity;
			runtime.DebrisPieces.push_back(std::move(piece));
		}
	}

	m_pPhysics = std::move(stagedPhysics);
	m_Profile = profile;
	m_Elements = std::move(stagedElements);
	m_pDeployRuntime = &deployRuntime;
	m_iLevelId = levelId;
	m_isStaged = true;
	m_isPhysicsPaused = true;
	m_Status = "Staged destruction simulation: " + profile.profileId;
	if (usedGenericFallback)
		m_Status += " (source wall recipe unavailable; generic fallback)";
	if (!Reset(outStatus))
	{
		Clear();
		return false;
	}
	m_Status = "Staged destruction simulation: " + profile.profileId;
	if (usedGenericFallback)
		m_Status += " (source wall recipe unavailable; generic fallback)";
	outStatus = m_Status;
	return true;
}

bool_t Client::CDestructionSimulationRuntime::Destroy_Actors(
	std::string* outStatus)
{
	bool_t restoredAllAliases = true;
	for (ELEMENT_RUNTIME& runtime : m_Elements)
	{
		for (ELEMENT_RUNTIME::DEBRIS_PIECE_RUNTIME& piece :
			runtime.DebrisPieces)
		{
			if (nullptr != piece.pBody)
				piece.pBody->Destroy_Actor();
			piece.pBody.reset();
			if (nullptr != runtime.pObject &&
				runtime.pObject->Is_DebrisPreviewActive())
			{
				runtime.pObject->Apply_DebrisPreviewPose(
					piece.iVisualIndex,
					piece.vCurrentPosition,
					piece.vCurrentRotation,
					false);
			}
		}
		if (nullptr != runtime.pObject &&
			runtime.pObject->Is_PhysicsPreviewActive())
		{
			runtime.pObject->End_PhysicsPreview();
		}
		std::string restoreStatus;
		if (!Restore_SuppressionAliases(runtime, &restoreStatus))
		{
			if (nullptr != outStatus && restoredAllAliases)
				*outStatus = std::move(restoreStatus);
			restoredAllAliases = false;
		}
	}
	return restoredAllAliases;
}

void Client::CDestructionSimulationRuntime::Release_DebrisPreviews()
{
	for (ELEMENT_RUNTIME& runtime : m_Elements)
	{
		if (nullptr != runtime.pObject &&
			runtime.pObject->Is_DebrisPreviewActive())
		{
			runtime.pObject->End_DebrisPreview();
		}
	}
}

void Client::CDestructionSimulationRuntime::Clear()
{
	std::string restoreStatus;
	if (!Destroy_Actors(&restoreStatus))
	{
		/* Keep the staged alias records so a later Clear/Reset can retry. Static
		   alias admission makes this path exceptional, but never erase the only
		   saved pre-suppression state after a failed restore. */
		m_Status = restoreStatus.empty() ?
			"Failed to restore a destruction suppression alias" : restoreStatus;
		return;
	}
	Release_DebrisPreviews();
	if (nullptr != m_pPhysics)
	{
		m_pPhysics->Clear_Ground();
		m_pPhysics->End_ExclusiveClock();
	}
	m_Profile = {};
	m_Elements.clear();
	m_Frame = {};
	m_pDeployRuntime = nullptr;
	m_iLevelId = ETOUI(LEVEL::END);
	m_fSampleTimeSeconds = 0.f;
	m_eScope = DESTRUCTION_SIMULATION_SCOPE::ALL_DEBRIS;
	m_SelectedElementId.clear();
	m_SelectedFragmentId.clear();
	m_Status = "Destruction simulation is not staged";
	m_isStaged = false;
	m_isPhysicsPaused = true;
	if (nullptr == m_pPhysics)
		m_pPhysics = std::make_unique<CPhysicsAdapter>();
}

bool_t Client::CDestructionSimulationRuntime::Reset(
	std::string& outStatus)
{
	if (!m_isStaged)
	{
		outStatus = "Destruction simulation reset requires a staged profile";
		return false;
	}
	if (!Destroy_Actors(&outStatus))
	{
		m_Status = outStatus;
		return false;
	}
	m_fSampleTimeSeconds = 0.f;
	for (ELEMENT_RUNTIME& runtime : m_Elements)
	{
		runtime.vCurrentPosition = runtime.vInitialPosition;
		runtime.vCurrentRotation = runtime.vInitialRotation;
		runtime.vLinearVelocity = {
			runtime.Desc.vDirection.x *
				runtime.Desc.fSpeedMetersPerSecond,
			runtime.Desc.vDirection.y *
				runtime.Desc.fSpeedMetersPerSecond,
			runtime.Desc.vDirection.z *
				runtime.Desc.fSpeedMetersPerSecond
		};
		runtime.fActivatedAtSeconds = 0.f;
		if (nullptr == runtime.pObject ||
			!runtime.pObject->Is_DebrisPreviewActive() ||
			runtime.pObject->Get_DebrisPreviewInstanceCount() !=
				runtime.DebrisPieces.size())
		{
			outStatus = "Destruction debris preview resources changed during reset";
			return false;
		}
		for (ELEMENT_RUNTIME::DEBRIS_PIECE_RUNTIME& piece :
			runtime.DebrisPieces)
		{
			piece.vCurrentPosition = piece.vInitialPosition;
			piece.vCurrentRotation = piece.vInitialRotation;
			piece.vCurrentLinearVelocity = piece.vInitialLinearVelocity;
			piece.fActivatedAtSeconds = 0.f;
			piece.eState = Is_FragmentInScope(piece.fragmentId) ?
				DESTRUCTION_SIMULATION_ELEMENT_STATE::WAITING :
				DESTRUCTION_SIMULATION_ELEMENT_STATE::FILTERED;
			if (!runtime.pObject->Apply_DebrisPreviewPose(
				piece.iVisualIndex,
				piece.vInitialPosition,
				piece.vInitialRotation,
				false))
			{
				outStatus = "DeployProp rejected a hidden debris reset pose";
				return false;
			}
		}
		runtime.eState = Is_ElementInScope(runtime.Desc.elementId) ?
			DESTRUCTION_SIMULATION_ELEMENT_STATE::WAITING :
			DESTRUCTION_SIMULATION_ELEMENT_STATE::FILTERED;
	}
	Rebuild_Frame();
	m_Status = "Reset destruction simulation to 0 s";
	outStatus = m_Status;
	return true;
}

bool_t Client::CDestructionSimulationRuntime::Is_ElementInScope(
	const std::string& elementId) const
{
	if (DESTRUCTION_SIMULATION_SCOPE::ALL_DEBRIS == m_eScope)
		return true;
	if (DESTRUCTION_SIMULATION_SCOPE::SOLO_SELECTED == m_eScope)
		return elementId == m_SelectedElementId;
	if (DESTRUCTION_SIMULATION_SCOPE::SOLO_FRAGMENT != m_eScope)
		return false;
	const auto element = std::find_if(m_Elements.begin(), m_Elements.end(),
		[&elementId](const ELEMENT_RUNTIME& runtime)
		{
			return runtime.Desc.elementId == elementId;
		});
	return m_Elements.end() != element && std::any_of(
		element->DebrisPieces.begin(), element->DebrisPieces.end(),
		[this](const ELEMENT_RUNTIME::DEBRIS_PIECE_RUNTIME& piece)
		{
			return piece.fragmentId == m_SelectedFragmentId;
		});
}

bool_t Client::CDestructionSimulationRuntime::Is_FragmentInScope(
	const std::string& fragmentId) const
{
	if (DESTRUCTION_SIMULATION_SCOPE::ALL_DEBRIS == m_eScope)
		return true;
	if (DESTRUCTION_SIMULATION_SCOPE::SOLO_FRAGMENT == m_eScope)
		return fragmentId == m_SelectedFragmentId;
	if (DESTRUCTION_SIMULATION_SCOPE::SOLO_SELECTED != m_eScope)
		return false;
	const auto element = std::find_if(m_Elements.begin(), m_Elements.end(),
		[this](const ELEMENT_RUNTIME& runtime)
		{
			return runtime.Desc.elementId == m_SelectedElementId;
		});
	return m_Elements.end() != element && std::any_of(
		element->DebrisPieces.begin(), element->DebrisPieces.end(),
		[&fragmentId](const ELEMENT_RUNTIME::DEBRIS_PIECE_RUNTIME& piece)
		{
			return piece.fragmentId == fragmentId;
		});
}

bool_t Client::CDestructionSimulationRuntime::Set_Scope(
	const DESTRUCTION_SIMULATION_SCOPE scope,
	const std::string& selectedElementId,
	std::string& outStatus)
{
	if (!m_isStaged || DESTRUCTION_SIMULATION_SCOPE::END == scope)
	{
		outStatus = "Destruction simulation scope is invalid";
		return false;
	}
	std::string selectedOwnerElementId;
	if (DESTRUCTION_SIMULATION_SCOPE::SOLO_SELECTED == scope)
	{
		const bool_t exists = std::any_of(m_Elements.begin(), m_Elements.end(),
			[&selectedElementId](const ELEMENT_RUNTIME& runtime)
			{
				return runtime.Desc.elementId == selectedElementId;
			});
		if (!exists)
		{
			outStatus = "Solo debris element is not in the staged profile";
			return false;
		}
		selectedOwnerElementId = selectedElementId;
	}
	else if (DESTRUCTION_SIMULATION_SCOPE::SOLO_FRAGMENT == scope)
	{
		for (const ELEMENT_RUNTIME& runtime : m_Elements)
		{
			const bool_t found = std::any_of(
				runtime.DebrisPieces.begin(), runtime.DebrisPieces.end(),
				[&selectedElementId](
					const ELEMENT_RUNTIME::DEBRIS_PIECE_RUNTIME& piece)
				{
					return piece.fragmentId == selectedElementId;
				});
			if (found)
			{
				selectedOwnerElementId = runtime.Desc.elementId;
				break;
			}
		}
		if (selectedOwnerElementId.empty())
		{
			outStatus = "Solo debris fragment is not in the staged profile";
			return false;
		}
	}

	const DESTRUCTION_SIMULATION_SCOPE previousScope = m_eScope;
	const std::string previousSelection = m_SelectedElementId;
	const std::string previousFragmentSelection = m_SelectedFragmentId;
	m_eScope = scope;
	m_SelectedElementId = selectedOwnerElementId;
	m_SelectedFragmentId =
		DESTRUCTION_SIMULATION_SCOPE::SOLO_FRAGMENT == scope ?
			selectedElementId : std::string{};
	if (!Reset(outStatus))
	{
		m_eScope = previousScope;
		m_SelectedElementId = previousSelection;
		m_SelectedFragmentId = previousFragmentSelection;
		return false;
	}
	if (DESTRUCTION_SIMULATION_SCOPE::ALL_DEBRIS == scope)
		m_Status = "Preview scope: All Debris";
	else if (DESTRUCTION_SIMULATION_SCOPE::SOLO_FRAGMENT == scope)
		m_Status = "Preview scope: Solo Fragment " + selectedElementId;
	else
		m_Status = "Preview scope: Solo Wall " + selectedElementId;
	outStatus = m_Status;
	return true;
}

bool_t Client::CDestructionSimulationRuntime::Activate_Element(
	ELEMENT_RUNTIME& runtime,
	std::string& outStatus)
{
	if (DESTRUCTION_SIMULATION_ELEMENT_STATE::WAITING != runtime.eState ||
		nullptr == runtime.pObject || nullptr == m_pPhysics ||
		!runtime.pObject->Is_DebrisPreviewActive() ||
		runtime.pObject->Get_DebrisPreviewInstanceCount() !=
			runtime.DebrisPieces.size() ||
		runtime.DebrisPieces.empty())
	{
		outStatus = "Destruction debris activation state is invalid";
		return false;
	}

	std::vector<std::pair<size_t, shared_ptr<Engine::CRigidBody>>>
		stagedBodies;
	stagedBodies.reserve(runtime.DebrisPieces.size());
	for (size_t pieceIndex = 0u;
		pieceIndex < runtime.DebrisPieces.size(); ++pieceIndex)
	{
		const ELEMENT_RUNTIME::DEBRIS_PIECE_RUNTIME& piece =
			runtime.DebrisPieces[pieceIndex];
		if (DESTRUCTION_SIMULATION_ELEMENT_STATE::WAITING != piece.eState)
			continue;
		shared_ptr<Engine::CRigidBody> stagedBody;
		if (!m_pPhysics->Create_DynamicBox(
			m_iLevelId,
			piece.vInitialPosition,
			piece.vInitialRotation,
			piece.vShapeLocalCentre,
			piece.vHalfExtents,
			piece.vInitialLinearVelocity,
			piece.vInitialAngularVelocity,
			runtime.Desc.fGravityScale,
			stagedBody,
			outStatus))
		{
			for (auto& staged : stagedBodies)
				staged.second->Destroy_Actor();
			return false;
		}
		stagedBodies.emplace_back(pieceIndex, std::move(stagedBody));
	}
	if (stagedBodies.empty())
	{
		outStatus = "Destruction debris activation has no fragment in scope";
		return false;
	}

	if (!runtime.pObject->Begin_PhysicsPreview(
		DEPLOY_PROP_STATE::FRACTURED))
	{
		for (auto& staged : stagedBodies)
			staged.second->Destroy_Actor();
		if (runtime.pObject->Is_PhysicsPreviewActive())
			runtime.pObject->End_PhysicsPreview();
		outStatus = "DeployProp rejected its fractured source preview state";
		return false;
	}

	for (size_t index = 0u; index < runtime.DebrisPieces.size(); ++index)
	{
		ELEMENT_RUNTIME::DEBRIS_PIECE_RUNTIME& piece =
			runtime.DebrisPieces[index];
		const bool_t isVisible =
			DESTRUCTION_SIMULATION_ELEMENT_STATE::WAITING == piece.eState;
		if (!runtime.pObject->Apply_DebrisPreviewPose(
			piece.iVisualIndex,
			piece.vInitialPosition,
			piece.vInitialRotation,
			isVisible))
		{
			for (const ELEMENT_RUNTIME::DEBRIS_PIECE_RUNTIME& hiddenPiece :
				runtime.DebrisPieces)
			{
				runtime.pObject->Apply_DebrisPreviewPose(
					hiddenPiece.iVisualIndex,
					hiddenPiece.vInitialPosition,
					hiddenPiece.vInitialRotation,
					false);
			}
			for (auto& staged : stagedBodies)
				staged.second->Destroy_Actor();
			runtime.pObject->End_PhysicsPreview();
			outStatus = "DeployProp rejected an initial debris proxy pose";
			return false;
		}
		piece.vCurrentPosition = piece.vInitialPosition;
		piece.vCurrentRotation = piece.vInitialRotation;
		piece.vCurrentLinearVelocity = piece.vInitialLinearVelocity;
	}

	/* Suppress-only aliases cover source Deploy rows that occupy the same
	   authored wall but must not produce a second debris emitter. Capture each
	   persistent state at the activation boundary and commit all aliases as one
	   transaction. Expiry intentionally leaves them hidden; Reset/Clear owns
	   restoration. */
	for (ELEMENT_RUNTIME::SUPPRESSION_ALIAS_RUNTIME& alias :
		runtime.SuppressionAliases)
	{
		if (nullptr == alias.pObject || alias.isSuppressed)
		{
			std::string restoreStatus;
			const bool_t restoredAliases =
				Restore_SuppressionAliases(runtime, &restoreStatus);
			for (const ELEMENT_RUNTIME::DEBRIS_PIECE_RUNTIME& hiddenPiece :
				runtime.DebrisPieces)
			{
				runtime.pObject->Apply_DebrisPreviewPose(
					hiddenPiece.iVisualIndex,
					hiddenPiece.vInitialPosition,
					hiddenPiece.vInitialRotation,
					false);
			}
			for (auto& staged : stagedBodies)
				staged.second->Destroy_Actor();
			runtime.pObject->End_PhysicsPreview();
			outStatus = "Destruction suppression alias state is invalid";
			if (!restoredAliases && !restoreStatus.empty())
				outStatus += "; rollback failed: " + restoreStatus;
			return false;
		}
		alias.ePreviousState = alias.pObject->Get_State();
		if (!alias.pObject->Set_State(DEPLOY_PROP_STATE::DESPAWNED))
		{
			std::string restoreStatus;
			const bool_t restoredAliases =
				Restore_SuppressionAliases(runtime, &restoreStatus);
			for (const ELEMENT_RUNTIME::DEBRIS_PIECE_RUNTIME& hiddenPiece :
				runtime.DebrisPieces)
			{
				runtime.pObject->Apply_DebrisPreviewPose(
					hiddenPiece.iVisualIndex,
					hiddenPiece.vInitialPosition,
					hiddenPiece.vInitialRotation,
					false);
			}
			for (auto& staged : stagedBodies)
				staged.second->Destroy_Actor();
			runtime.pObject->End_PhysicsPreview();
			outStatus = "DeployProp rejected a suppression alias preview state: " +
				std::to_string(alias.runtimePlacementId);
			if (!restoredAliases && !restoreStatus.empty())
				outStatus += "; rollback failed: " + restoreStatus;
			return false;
		}
		alias.isSuppressed = true;
	}
	runtime.eState = DESTRUCTION_SIMULATION_ELEMENT_STATE::ACTIVE;
	runtime.fActivatedAtSeconds =
		DESTRUCTION_SIMULATION_TRIGGER_KIND::TIMELINE_TIME ==
			runtime.Desc.Trigger.eKind ?
			runtime.Desc.Trigger.fTimeSeconds : m_fSampleTimeSeconds;
	for (auto& staged : stagedBodies)
	{
		ELEMENT_RUNTIME::DEBRIS_PIECE_RUNTIME& piece =
			runtime.DebrisPieces[staged.first];
		piece.pBody = std::move(staged.second);
		piece.eState = DESTRUCTION_SIMULATION_ELEMENT_STATE::ACTIVE;
		piece.fActivatedAtSeconds = runtime.fActivatedAtSeconds;
	}
	return true;
}

bool_t Client::CDestructionSimulationRuntime::Restore_SuppressionAliases(
	ELEMENT_RUNTIME& runtime,
	std::string* outStatus)
{
	bool_t restoredAll = true;
	for (auto alias = runtime.SuppressionAliases.rbegin();
		alias != runtime.SuppressionAliases.rend(); ++alias)
	{
		if (!alias->isSuppressed)
			continue;
		if (nullptr == alias->pObject)
		{
			restoredAll = false;
			if (nullptr != outStatus && outStatus->empty())
			{
				*outStatus = "Suppression alias object is no longer available: " +
					std::to_string(alias->runtimePlacementId);
			}
			continue;
		}
		if (alias->pObject->Set_State(alias->ePreviousState))
		{
			alias->isSuppressed = false;
			continue;
		}
		restoredAll = false;
		if (nullptr != outStatus && outStatus->empty())
		{
			*outStatus = "Failed to restore suppression alias state: " +
				std::to_string(alias->runtimePlacementId);
		}
	}
	return restoredAll;
}

void Client::CDestructionSimulationRuntime::Expire_Element(
	ELEMENT_RUNTIME& runtime)
{
	for (ELEMENT_RUNTIME::DEBRIS_PIECE_RUNTIME& piece :
		runtime.DebrisPieces)
	{
		if (DESTRUCTION_SIMULATION_ELEMENT_STATE::FILTERED == piece.eState)
			continue;
		if (nullptr != piece.pBody)
			piece.pBody->Destroy_Actor();
		piece.pBody.reset();
		if (nullptr != runtime.pObject &&
			runtime.pObject->Is_DebrisPreviewActive())
		{
				runtime.pObject->Apply_DebrisPreviewPose(
				piece.iVisualIndex,
				piece.vCurrentPosition,
				piece.vCurrentRotation,
				false);
		}
		piece.eState = DESTRUCTION_SIMULATION_ELEMENT_STATE::EXPIRED;
	}
	/* The debris pieces are transient. Any source suppression selected by the
	   recipe remains active until Reset/Clear restores the exact authored state
	   and animation cursor. */
	runtime.eState = DESTRUCTION_SIMULATION_ELEMENT_STATE::EXPIRED;
}

bool_t Client::CDestructionSimulationRuntime::Advance_Timeline(
	const f32_t fixedDeltaSeconds,
	std::string& outStatus)
{
	if (!m_isStaged || !std::isfinite(fixedDeltaSeconds) ||
		fixedDeltaSeconds <= 0.f || fixedDeltaSeconds > 0.1f)
	{
		outStatus = "Destruction simulation fixed step is invalid";
		return false;
	}
	if (Is_Finished())
	{
		outStatus = "Destruction simulation is already finished";
		return false;
	}

	const f32_t previousTime = m_fSampleTimeSeconds;
	m_fSampleTimeSeconds = (std::min)(m_Profile.fDurationSeconds,
		m_fSampleTimeSeconds + fixedDeltaSeconds);
	for (ELEMENT_RUNTIME& runtime : m_Elements)
	{
		if (DESTRUCTION_SIMULATION_ELEMENT_STATE::WAITING == runtime.eState)
		{
			const DESTRUCTION_SIMULATION_TRIGGER& trigger = runtime.Desc.Trigger;
			const bool_t shouldActivate =
				DESTRUCTION_SIMULATION_TRIGGER_KIND::IMMEDIATE == trigger.eKind ||
				(DESTRUCTION_SIMULATION_TRIGGER_KIND::TIMELINE_TIME ==
					trigger.eKind &&
					previousTime <= trigger.fTimeSeconds &&
					m_fSampleTimeSeconds >= trigger.fTimeSeconds);
			if (shouldActivate && !Activate_Element(runtime, outStatus))
				return false;
		}
		if (DESTRUCTION_SIMULATION_ELEMENT_STATE::ACTIVE == runtime.eState &&
			(nullptr == runtime.pObject ||
				!runtime.pObject->Advance_PhysicsPreviewAnimation(
					fixedDeltaSeconds)))
		{
			outStatus = "DeployProp rejected its fixed destruction animation tick";
			return false;
		}
		if (DESTRUCTION_SIMULATION_ELEMENT_STATE::ACTIVE == runtime.eState &&
			m_fSampleTimeSeconds - runtime.fActivatedAtSeconds >=
				runtime.Desc.fLifetimeSeconds)
		{
			Expire_Element(runtime);
		}
	}
	if (m_fSampleTimeSeconds >= m_Profile.fDurationSeconds)
	{
		for (ELEMENT_RUNTIME& runtime : m_Elements)
		{
			if (DESTRUCTION_SIMULATION_ELEMENT_STATE::ACTIVE == runtime.eState ||
				DESTRUCTION_SIMULATION_ELEMENT_STATE::WAITING == runtime.eState)
			{
				Expire_Element(runtime);
			}
		}
	}
	Rebuild_Frame();
	m_Status = "Destruction simulation sampled at " +
		std::to_string(m_fSampleTimeSeconds) + " s";
	outStatus = m_Status;
	return true;
}

bool_t Client::CDestructionSimulationRuntime::Post_Physics_Update(
	std::string& outStatus)
{
	if (!m_isStaged)
		return true;
	bool_t succeeded = true;
	for (ELEMENT_RUNTIME& runtime : m_Elements)
	{
		if (DESTRUCTION_SIMULATION_ELEMENT_STATE::ACTIVE != runtime.eState)
		{
			continue;
		}
		if (nullptr == runtime.pObject ||
			!runtime.pObject->Is_DebrisPreviewActive())
		{
			succeeded = false;
			continue;
		}

		float3_t aggregatePosition{};
		float3_t aggregateVelocity{};
		size_t sampledPieces = 0u;
		for (ELEMENT_RUNTIME::DEBRIS_PIECE_RUNTIME& piece :
			runtime.DebrisPieces)
		{
			if (DESTRUCTION_SIMULATION_ELEMENT_STATE::ACTIVE != piece.eState)
				continue;
			Engine::PHYSICS_POSE pose{};
			float3_t velocity{};
			if (nullptr == piece.pBody ||
				FAILED(piece.pBody->Get_Pose(pose)) ||
				FAILED(piece.pBody->Get_LinearVelocity(velocity)) ||
				!Is_FinitePose(pose.vPosition, pose.vRotationQuaternion) ||
				!std::isfinite(velocity.x) || !std::isfinite(velocity.y) ||
				!std::isfinite(velocity.z) ||
				!runtime.pObject->Apply_DebrisPreviewPose(
					piece.iVisualIndex,
					pose.vPosition,
					pose.vRotationQuaternion,
					true))
			{
				succeeded = false;
				continue;
			}
			piece.vCurrentPosition = pose.vPosition;
			piece.vCurrentRotation = pose.vRotationQuaternion;
			piece.vCurrentLinearVelocity = velocity;
			aggregatePosition.x += pose.vPosition.x;
			aggregatePosition.y += pose.vPosition.y;
			aggregatePosition.z += pose.vPosition.z;
			aggregateVelocity.x += velocity.x;
			aggregateVelocity.y += velocity.y;
			aggregateVelocity.z += velocity.z;
			if (0u == sampledPieces)
				runtime.vCurrentRotation = pose.vRotationQuaternion;
			++sampledPieces;
		}
		if (0u == sampledPieces)
		{
			succeeded = false;
			continue;
		}
		const f32_t inverseCount = 1.f /
			static_cast<f32_t>(sampledPieces);
		runtime.vCurrentPosition = {
			aggregatePosition.x * inverseCount,
			aggregatePosition.y * inverseCount,
			aggregatePosition.z * inverseCount
		};
		runtime.vLinearVelocity = {
			aggregateVelocity.x * inverseCount,
			aggregateVelocity.y * inverseCount,
			aggregateVelocity.z * inverseCount
		};
	}

	Rebuild_Frame();
	if (!succeeded)
	{
		m_Status = "One or more debris proxy poses could not be pulled";
		outStatus = m_Status;
		return false;
	}
	outStatus = m_Status;
	return true;
}

bool_t Client::CDestructionSimulationRuntime::Notify_Collision(
	const std::string_view receiverCollisionId,
	std::string& outStatus)
{
	if (!m_isStaged || receiverCollisionId.empty())
	{
		outStatus = "Collision preview needs a staged receiver ID";
		return false;
	}
	size_t activated = 0u;
	for (ELEMENT_RUNTIME& runtime : m_Elements)
	{
		if (DESTRUCTION_SIMULATION_ELEMENT_STATE::WAITING != runtime.eState ||
			DESTRUCTION_SIMULATION_TRIGGER_KIND::COLLISION_IMPACT !=
				runtime.Desc.Trigger.eKind ||
			runtime.Desc.Trigger.receiverCollisionId != receiverCollisionId)
		{
			continue;
		}
		if (!Activate_Element(runtime, outStatus))
			return false;
		++activated;
	}
	if (0u == activated)
	{
		outStatus = "No waiting debris element matches collision receiver: " +
			std::string(receiverCollisionId);
		return false;
	}
	Rebuild_Frame();
	m_Status = "Collision preview activated " + std::to_string(activated) +
		" debris elements";
	outStatus = m_Status;
	return true;
}

bool_t Client::CDestructionSimulationRuntime::Set_PhysicsPaused(
	const bool_t isPaused,
	std::string& outStatus)
{
	if (!m_isStaged)
	{
		outStatus = "Physics pause requires a staged simulation";
		return false;
	}
	/* The Engine manager deliberately remains debug-paused for the entire
	   staged lifetime. This flag is playback state only; Step_Once drives the
	   scene through Simulate_DebugSteps so no second clock can advance it. */
	m_isPhysicsPaused = isPaused;
	outStatus = isPaused ? "Destruction physics paused" :
		"Destruction physics playing";
	return true;
}

bool_t Client::CDestructionSimulationRuntime::Simulate_PhysicsSteps(
	const uint32_t stepCount,
	std::string& outStatus)
{
	if (!m_isStaged || nullptr == m_pPhysics || 0u == stepCount ||
		!m_pPhysics->Simulate_Steps(stepCount, outStatus))
	{
		if (outStatus.empty())
			outStatus = "Destruction simulation cannot advance PhysX";
		return false;
	}
	return true;
}

bool_t Client::CDestructionSimulationRuntime::Is_Finished() const
{
	return m_isStaged &&
		m_fSampleTimeSeconds >= m_Profile.fDurationSeconds;
}

void Client::CDestructionSimulationRuntime::Rebuild_Frame()
{
	m_Frame = {};
	m_Frame.profileId = m_Profile.profileId;
	m_Frame.groupId = m_Profile.groupId;
	m_Frame.fSampleTimeSeconds = m_fSampleTimeSeconds;
	m_Frame.fDurationSeconds = m_Profile.fDurationSeconds;
	m_Frame.eScope = m_eScope;
	m_Frame.selectedElementId = m_SelectedElementId;
	m_Frame.selectedFragmentId = m_SelectedFragmentId;
	m_Frame.Elements.reserve(m_Elements.size());
	for (const ELEMENT_RUNTIME& runtime : m_Elements)
	{
		DESTRUCTION_SIMULATION_ELEMENT_FRAME frame;
		frame.elementId = runtime.Desc.elementId;
		frame.sourceRuntimePlacementId =
			runtime.Desc.sourceRuntimePlacementId;
		frame.eState = runtime.eState;
		frame.vWorldPosition = runtime.vCurrentPosition;
		frame.vWorldRotationQuaternion = runtime.vCurrentRotation;
		frame.vLinearVelocity = runtime.vLinearVelocity;
		frame.fNormalizedLife =
			DESTRUCTION_SIMULATION_ELEMENT_STATE::ACTIVE == runtime.eState ?
				(std::clamp)(
					(m_fSampleTimeSeconds - runtime.fActivatedAtSeconds) /
						runtime.Desc.fLifetimeSeconds,
					0.f, 1.f) :
			DESTRUCTION_SIMULATION_ELEMENT_STATE::EXPIRED == runtime.eState ?
				1.f : 0.f;
		frame.Fragments.reserve(runtime.DebrisPieces.size());
		for (uint32_t pieceIndex = 0u;
			pieceIndex < runtime.DebrisPieces.size(); ++pieceIndex)
		{
			const ELEMENT_RUNTIME::DEBRIS_PIECE_RUNTIME& piece =
				runtime.DebrisPieces[pieceIndex];
			DESTRUCTION_SIMULATION_FRAGMENT_FRAME fragmentFrame;
			fragmentFrame.fragmentId = piece.fragmentId;
			fragmentFrame.modelAssetId = piece.modelAssetId;
			fragmentFrame.pieceIndex = pieceIndex;
			fragmentFrame.eState = piece.eState;
			fragmentFrame.vWorldPosition = piece.vCurrentPosition;
			fragmentFrame.vWorldRotationQuaternion = piece.vCurrentRotation;
			fragmentFrame.vLinearVelocity = piece.vCurrentLinearVelocity;
			fragmentFrame.fNormalizedLife =
				DESTRUCTION_SIMULATION_ELEMENT_STATE::ACTIVE == piece.eState ?
					(std::clamp)(
						(m_fSampleTimeSeconds - piece.fActivatedAtSeconds) /
							runtime.Desc.fLifetimeSeconds,
						0.f, 1.f) :
				DESTRUCTION_SIMULATION_ELEMENT_STATE::EXPIRED == piece.eState ?
					1.f : 0.f;
			frame.Fragments.push_back(std::move(fragmentFrame));
		}
		m_Frame.Elements.push_back(std::move(frame));
	}
}
