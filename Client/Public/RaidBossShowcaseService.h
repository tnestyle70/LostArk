#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <string_view>

NS_BEGIN(Client)

#ifdef _DEBUG
struct RAID_BOSS_SHOWCASE_TUNING
{
	f32_t fRectX;
	f32_t fRectY;
	f32_t fRectWidth;
	f32_t fRectHeight;
	f32_t fModelYawDegrees;
	f32_t fEyeXPerHeight;
	f32_t fEyeYPerHeight;
	f32_t fDistancePerHeight;
	f32_t fAtYPerHeight;
	f32_t fFovDegrees;
};
#endif

/* Live boss render for the raid entry popup's portrait area -- the same construction the
   original client uses there: its epicgatecommanderbgtexture is a render target the engine
   draws the boss scene into every frame, not a texture asset (which is why no baked colour
   animation exists anywhere in the shipped packages). Implementation-wise this is the
   esther cutin's pattern: viewport over the already-drawn frame, own camera, the animated
   binary mesh shader's forward ScreenCutin pass, wall-clock clip advance.

   The popup calls Request_Frame every frame it wants the boss alive; staging happens on the
   first sighting through CValtanPresentationAssetService's own lazy prototype path (the
   sanctioned admission the replication and tools already use), and the service goes idle
   the frame after the calls stop. Staging failure is fail-soft: the popup's static key art
   simply stays as-is underneath. */
class CRaidBossShowcaseService final
{
public:
	CRaidBossShowcaseService() = delete;

	/* Called from the popup's own per-frame Render while the live showcase should show.
	   archetypeId names the boss ("BOSS_VALTAN"); only Valtan has a complete model today. */
	static void Request_Frame(std::string_view archetypeId);

	/* Called once when the popup opens: front-loads the one synchronous model admission
	   (Ensure_Prototypes decodes body/weapon/armor/animset on the calling thread) onto the
	   open transition, so reaching the boss's tab later costs nothing. */
	static void Request_Prewarm(std::string_view archetypeId);

	/* True once the model is staged and drawing -- the popup uses this to hide the static
	   locked-state key art, the same exclusive switch the original's
	   satisfiedChangedHandler makes between bossImage and imageTexture. */
	static bool_t Is_Live();

	/* Draws after CImGuiLayer::EndFrame and before the Draw_Text pass, next to the esther
	   cutin call -- over the popup's sprites, under its text. */
	static HRESULT Render(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

#ifdef _DEBUG
	static RAID_BOSS_SHOWCASE_TUNING& Debug_Tuning();
	static void Debug_ResetTuning();
#endif
};

NS_END
