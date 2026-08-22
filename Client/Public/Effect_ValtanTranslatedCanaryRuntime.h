#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "Effect_AuthoringDocument.h"

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

NS_BEGIN(Engine)
class CModel;
class CVIBuffer_Rect;
NS_END

NS_BEGIN(Client)

/* Tool-only bridge between three exact translated UE3 pixel equations and the
   bounded carriers available in Winters.  This class deliberately owns no
   Product admission: callers must keep the gate default-off and may only
   stage the exact FRONT_BACK_FRONT occurrence identities below. */
enum class VALTAN_TRANSLATED_CANARY_FAMILY : uint8_t
{
	MASKED_DISSOLVE,
	CRACK_TRANSLUCENT,
	GROUND_DECAL,
	END
};

struct VALTAN_TRANSLATED_CANARY_ELEMENT_PACKET;

class CValtanTranslatedCanaryRuntime final
{

public:
	static constexpr std::string_view EFFECT_ASSET_ID =
		"effect.valtan.front-back-front.windup";
	static constexpr bool_t DEFAULT_ENABLED = false;
	static constexpr bool_t FAIL_CLOSED = true;
	static constexpr bool_t PRODUCT_ENABLED = false;
	static constexpr bool_t VISUAL_ADMISSION = false;
	static constexpr bool_t ACTUAL_VERTEX_FACTORY_PASS = false;
	static constexpr bool_t ENGINE_SCENE_CB_EXACT = false;
	/* Address axes serialized by the source Texture2D are honored.  The source
	   revision's TextureLODSettings hardware-filter resolution is not present,
	   so the Tool adapter uses bounded linear filtering and cannot claim exact
	   sampler parity. */
	static constexpr bool_t SAMPLER_EXACT = false;
	static constexpr bool_t RT0_ONLY = true;

public:
	CValtanTranslatedCanaryRuntime(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	~CValtanTranslatedCanaryRuntime();

	CValtanTranslatedCanaryRuntime(
		const CValtanTranslatedCanaryRuntime&) = delete;
	CValtanTranslatedCanaryRuntime& operator=(
		const CValtanTranslatedCanaryRuntime&) = delete;

	/* Transactional Tool arming.  All three Effects11 programs, every exact DDS
	   byte identity, typed colour space and bounded sampler must stage before the
	   previous state is replaced. */
	bool_t Arm(std::string& strOutError);
	void Clear();
	bool_t Is_Armed() const;

	/* A non-target element in the exact document succeeds without a packet.
	   While armed, a different Effect ID or a changed target identity fails
	   closed. */
	bool_t Stage_Packet(
		const std::string& strEffectAssetId,
		const EFFECT_ELEMENT_DESC& Element,
		std::shared_ptr<const VALTAN_TRANSLATED_CANARY_ELEMENT_PACKET>&
			pOutPacket,
		std::string& strOutError) const;

	HRESULT Draw_Mesh(
		const EFFECT_ELEMENT_DESC& Element,
		const std::shared_ptr<Engine::CModel>& pResourceModel,
		const std::shared_ptr<const VALTAN_TRANSLATED_CANARY_ELEMENT_PACKET>&
			pPacket,
		const float4x4_t& World,
		const float4x4_t& NormalMatrix,
		const float4_t& DynamicParameter,
		f32_t fLocalTimeSeconds) const;

	HRESULT Draw_Ground(
		const EFFECT_ELEMENT_DESC& Element,
		const std::shared_ptr<Engine::CVIBuffer_Rect>& pRect,
		const std::shared_ptr<const VALTAN_TRANSLATED_CANARY_ELEMENT_PACKET>&
			pPacket,
		const float4x4_t& InverseDecalWorld,
		const float2_t& vDecalSize,
		f32_t fDecalDepth,
		f32_t fLocalTimeSeconds) const;

	static VALTAN_TRANSLATED_CANARY_FAMILY Get_Family(
		const VALTAN_TRANSLATED_CANARY_ELEMENT_PACKET& Packet) noexcept;
	static bool_t Is_Target_ElementId(std::string_view strElementId) noexcept;
	static bool_t Is_Ground_ElementId(std::string_view strElementId) noexcept;

private:
	struct RUNTIME_STATE;
	std::unique_ptr<RUNTIME_STATE> m_pState;
};

NS_END
