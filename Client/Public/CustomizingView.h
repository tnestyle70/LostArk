#pragma once

#include "Client_Defines.h"
#include "CustomizingIconDocument.h"
#include "Engine_Defines.h"

#include <memory>
#include <random>
#include <string>
#include <utility>
#include <vector>

NS_BEGIN(Client)

class CCharacter;
class CUILayoutRuntime;

/* The retail character customizing screen (charactercustomizing.gfx): right tab column, the
face "detail" fold with its six part sub-tabs and their sliders, the top guide plate and the
bottom decide/back/reset bar. Every rect comes from that movie's own placement trace, baked
into Data/UI/Customizing/CustomizingUI.json by
Tools/CharacterCustomizing/build_customizing_ui.py, and draws as real CUI_Sprite GameObjects
through CUILayoutRuntime like every other migrated screen.

Slider values are not stored here: they are read from and written straight to the character's
own CFaceCustomizeApplier (the retail add_*_ui additive poses), so what the screen shows is
always what the skeleton is actually doing.

Only the face tab has data today (Data/Customizing/FaceSliders/<race>.facesliders.json). The
other six retail tabs draw dimmed and do not respond -- hair meshes, morph presets, material
tints and voice are separate slices that have no runtime contract yet.

The camera is not touched here: Get_CameraPositionOffset/Get_CameraLookOffset expose the orbit
this screen's drag/wheel gesture produces, and the owning Level applies them to its own
follow camera. */
class CCustomizingView final
{
public:
	CCustomizingView(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iGameObjectLevelIndex);
	~CCustomizingView();

public:
	bool_t Is_Open() const { return m_bOpen; }
	/* Resets the orbit to the retail-style three-quarter framing and shows every slot. */
	void Open();
	void Close();

	/* Per frame while the owning Level is in its Server arena. pCharacter may be null (the
	replicated character has not arrived yet) -- the sliders then draw at their neutral
	position and do not accept input. No-op while closed. */
	void Update(f32_t fTimeDelta, const shared_ptr<CCharacter>& pCharacter);
	/* LOA-font labels; call from the post-CImGuiLayer::EndFrame() text pass like every other
	runtime screen's text half. No-op while closed. */
	void Render_Text();
	/* Forces every owned sprite invisible without changing m_bOpen -- for the frames the
	owning Level stops calling Update (level teardown, replication failure). */
	void Hide();

	/* Camera pose this screen wants, as offsets from the character's own position, plus the
	field of view the retail framing uses. */
	float3_t Get_CameraPositionOffset() const;
	float3_t Get_CameraLookOffset() const;
	f32_t Get_FieldOfViewDegrees() const;

	/* One-shot edges the owning Level consumes: the decide button (proceed to the nickname
	step) and the back icon (leave customizing). */
	bool_t Try_Consume_Decide();
	bool_t Try_Consume_Back();

private:
	void Update_Tabs();
	void Update_FaceTab(const shared_ptr<CCharacter>& pCharacter);
	/* Puts the retail icons into the left column and the preset grid. The document
	is per class, so this reruns whenever the class on screen changes. */
	void Apply_ListIcons(const shared_ptr<CCharacter>& pCharacter);
	/* The face tab is two collapsible categories; the second moves as a block, so its
	authored rects are captured once and every frame positions it from those. */
	void Capture_FaceDetailLayout();
	void Apply_FaceAccordionLayout();
	void Update_FaceCategoryHeader(
		const char_t* pSlotId, bool_t& isExpanded, bool_t isVisible);
	void Update_Buttons(const shared_ptr<CCharacter>& pCharacter);
	void Update_Orbit(f32_t fTimeDelta);
	/* Reads the eye line off the body model on screen, so the face zoom frames every class
	at its own height instead of the tallest one's. */
	void Update_SubjectMetrics(const shared_ptr<CCharacter>& pCharacter);
	/* Sideways shift applied to both the eye and the look point so the character lands in the
	middle of the area the right panel leaves free, not the middle of the window. */
	float3_t Get_LateralOffset() const;
	f32_t Get_LookHeight() const;
	f32_t Get_FaceLookHeight() const;
	f32_t Get_Distance() const;
	f32_t Get_Pitch() const;
	/* Shows only the sliders of the selected part and hides every other part's. */
	void Apply_SliderVisibility();
	bool_t Get_SlotRect(const char_t* pSlotId,
		f32_t& outX, f32_t& outY, f32_t& outWidth, f32_t& outHeight) const;
	bool_t Is_Hovered(f32_t fX, f32_t fY, f32_t fWidth, f32_t fHeight) const;
	bool_t Is_Clicked(f32_t fX, f32_t fY, f32_t fWidth, f32_t fHeight) const;

private:
	unique_ptr<CUILayoutRuntime> m_pView;
	bool_t m_bOpen = false;
	/* Retail tab order: base, face, hair, eye, skin, adorn, voice. Only face has data. */
	int32_t m_iSelectedTab = 1;
	/* Face detail part: eye, eyebrow, cheekbone, jaw, nose, mouth. */
	int32_t m_iSelectedPart = 0;
	/* Index into FACE_SLIDER_ROWS while a slider thumb is held, -1 otherwise. */
	int32_t m_iDraggingRow = -1;
	/* Set by Update: false when the replicated character's class names no face slider
	race, so the text pass can say why the bar is dead instead of leaving it silent. */
	bool_t m_bHasFaceSliders = false;

	/* Turntable framing. The retail screen stands the character on the spot and spins the
	model (its own guide calls the gesture "rotate character"), so this yaw is measured from
	the character's own facing rather than from world axes -- otherwise a character that
	happens to face away shows its back. Pitch is in degrees, distance in metres, and the look
	height rises toward the head as the distance shrinks so zooming in frames the face. */
	f32_t m_fOrbitYaw = 0.f;
	/* 0 = the retail full-body framing, 1 = its face close-up. The wheel is a two-state
	toggle in the original, not a continuous zoom, so this only ever eases between the two. */
	f32_t m_fZoomTarget = 0.f;
	f32_t m_fZoomBlend = 0.f;
	/* The character's own facing this frame, captured in Update. */
	f32_t m_fCharacterYawDegrees = 0.f;
	f32_t m_fMeasuredEyeHeight = 0.f;
	CCustomizingIconDocument m_IconDocument;
	std::string m_strIconClassAssetId;
	bool_t m_isFaceDefaultExpanded = false;
	bool_t m_isFaceDetailExpanded = true;
	std::vector<std::pair<std::string, float2_t>> m_FaceDetailAuthoredRects;
	bool_t m_bOrbitDragging = false;
	f32_t m_fLastMouseX = 0.f;
	f32_t m_fLastMouseY = 0.f;

	bool_t m_bDecideRequested = false;
	bool_t m_bBackRequested = false;

	std::mt19937 m_Random{ std::random_device{}() };
};

NS_END
