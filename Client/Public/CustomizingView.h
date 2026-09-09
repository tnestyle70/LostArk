#pragma once

#include "Client_Defines.h"
#include "CustomizingFacePresetDocument.h"
#include "CustomizingFaceTextureDocument.h"
#include "CustomizingIconDocument.h"
#include "Engine_Defines.h"

#include <array>
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
	/* Which try-on costume the left column is showing, as the table's Object_Unit. The edge is
	one-shot so the owning Level only runs an equipment transaction when the pick changes. */
	int32_t Get_SelectedCostume() const { return m_iSelectedCostume; }
	bool_t Try_Consume_CostumeChange();
	/* Which hairstyle cell the hair tab is showing, and its one-shot change edge. */
	int32_t Get_SelectedHair() const { return m_iSelectedHair; }
	/* Degrees the drag gesture has turned the subject. The camera stays where the retail
	framing puts it; the model is what rotates, so its cloth chains react. */
	f32_t Get_SubjectYawOffsetDegrees() const { return -m_fOrbitYaw; }
	bool_t Try_Consume_HairChange();

private:
	void Update_Tabs();
	void Update_FaceTab(const shared_ptr<CCharacter>& pCharacter);
	/* Hair, eye, skin and adorn: their retail chrome, shown per selected tab. The controls are
	placed and clickable; what they would change has no runtime contract yet. */
	void Update_SecondaryTabs(const shared_ptr<CCharacter>& pCharacter);
	/* A grid shows one window onto a longer list. These keep the window in range and fill the
	visible cells from it; the wheel over the grid moves it. */
	int32_t Clamp_ScrollRow(int32_t iScrollRow, int32_t iEntryCount, int32_t iCellCount) const;
	void Fill_ScrollingGrid(const char_t* pPrefix, int32_t iCellCount, int32_t iScrollRow,
		const std::vector<std::string>& Entries);
	/* Moves whichever grid the pointer is over, and reports whether it took the notch so the
	camera zoom does not also act on it. */
	bool_t Consume_GridScroll(const char_t* pPrefix, int32_t iCellCount, int32_t iEntryCount,
		int32_t& iScrollRow, int32_t iWheel);
	/* Routes one wheel notch to whichever tab's grid is showing. Returns true when a grid took
	it, so the camera zoom leaves it alone. */
	bool_t Consume_TabGridScroll(int32_t iWheel);
	/* The adorn grid's current page, chosen by its sub-tab. */
	const std::vector<std::string>& Get_AdornPage() const;
	/* Puts the retail icons into the left column and the preset grid. The document
	is per class, so this reruns whenever the class on screen changes. */
	void Apply_ListIcons(const shared_ptr<CCharacter>& pCharacter);
	/* The face tab is two collapsible categories; the second moves as a block, so its
	authored rects are captured once and every frame positions it from those. */
	void Capture_FaceDetailLayout();
	void Apply_FaceAccordionLayout();
	/* One accordion over the two face categories: isOther is the sibling's flag, which this
	closes whenever the clicked one opens. */
	void Update_FaceCategoryHeader(
		const char_t* pSlotId, bool_t& isExpanded, bool_t& isOther, bool_t isVisible);
	/* Puts one face preset's morph weights on the character. iPreset < 0 clears every
	weight instead, which is what the reset button and a class with no preset data do. */
	void Apply_FacePreset(const shared_ptr<CCharacter>& pCharacter, int32_t iPreset);
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
	bool_t m_hasCapturedCharacterYaw = false;
	f32_t m_fMeasuredEyeHeight = 0.f;
	CCustomizingIconDocument m_IconDocument;
	/* Morph weights each face preset sets, in the same order as the grid's icons. Loaded
	next to the icons, per class. */
	CCustomizingFacePresetDocument m_FacePresetDocument;
	/* Which cooked face texture each list offers. Only the iris list is applied today; the
	rest are stamps this renderer cannot composite yet (see the document's own header). */
	CCustomizingFaceTextureDocument m_FaceTextureDocument;
	int32_t m_iSelectedEyeIris = -1;
	/* Hair two-tone, as the retail sliders express it: how much of the second colour blends
	and how far up the strand it reaches. The authored defaults until the player moves them. */
	f32_t m_fHairTwoToneStrength = 1.f;
	f32_t m_fHairTwoToneRange = 0.5f;
	/* Which slot the pointer grabbed, so a drag keeps following it off the track. */
	std::string m_strDraggingSlider;

	/* The colour picker, over the panel of whichever swatch opened it. Hue and saturation
	come from the wheel (angle and radius, the way the source art is drawn) and value from
	the bar beside it. The choice only reaches the model on Apply, so cancelling leaves the
	character exactly as it was. */
	/* Which surface the open picker paints. 0-2 are CCharacter::DYE_SURFACE (hair, eye,
	skin) so this header keeps its forward declaration instead of pulling the character in
	for one enum; the rest are the face material's own make-up colours, which are named
	parameters of the retail head material rather than dyes. PICKER_SURFACE_NONE is none. */
	static constexpr int32_t PICKER_SURFACE_NONE = -1;
	/* CCharacter::DYE_SURFACE::SKIN. */
	static constexpr int32_t SKIN_SURFACE_INDEX = 2;
	static constexpr int32_t FIRST_MAKEUP_SURFACE_INDEX = 3;
	/* Lip, cheek, eye line, eyebrow -- the four adorn pages, in sub-tab order -- then the
	eye shadow the eye-line page carries alongside its own colour. */
	static constexpr int32_t EYESHADOW_SURFACE_INDEX = 7;
	/* The eye tab's two swatches. Its colours are parameters of the retail eye material, not
	dyes, and which pair they write depends on the odd-eye sub-tab. */
	static constexpr int32_t EYE_BASE_SURFACE_INDEX = 8;
	static constexpr int32_t EYE_IRIS_SURFACE_INDEX = 9;
	static constexpr size_t SURFACE_COLOR_COUNT = 10;
	int32_t m_iPickerSurface = PICKER_SURFACE_NONE;
	f32_t m_fPickerHue = 0.f;
	f32_t m_fPickerSaturation = 0.f;
	f32_t m_fPickerValue = 1.f;
	bool_t m_isPickerWheelDragging = false;
	/* What the surface wore when the picker opened, so cancelling puts it back. The choice
	is applied live while the wheel is dragged -- that is the only way to judge it. */
	float4_t m_vPickerRestore = { 0.f, 0.f, 0.f, -1.f };
	bool_t m_isPickerBarDragging = false;
	bool_t m_bLastDyeApplied = false;
	/* What each surface is currently wearing, so a swatch shows the choice and re-opening
	the picker starts from it. w < 0 means the authored colour is still in place. */
	std::array<float4_t, SURFACE_COLOR_COUNT> m_SurfaceColors{};
	/* How strongly each make-up layer shows. Measured on the retail values: the cheek colour
	is authored with alpha 0 and the lip with 1, so alpha is the layer's strength and the
	page's strength slider is that channel. Indexed by adorn sub-tab, with the eye-line page's
	second slider (the eye shadow) last. */
	std::array<f32_t, 5> m_AdornStrength{ 1.f, 0.f, 1.f, 1.f, 1.f };
	/* Which stamp each adorn page is showing, as an index into that page's icon list. -1 is
	the authored one, which is the transparent Null the retail material ships. */
	std::array<int32_t, 3> m_SelectedAdornItems{ -1, -1, -1 };
	/* Wrinkle, gloss and freckle, in SKIN_SLIDERS order. Seeded from the class' authored
	values the first frame the tab is shown, so a thumb starts where retail put it; -1 is
	"not seeded yet". */
	std::array<f32_t, 3> m_SkinSliderValues{ -1.f, -1.f, -1.f };
	/* Pupil size: var_eye_irissize_ui, seeded from the class' authored value like the skin
	sliders. -1 is "not seeded yet". */
	f32_t m_fEyeIrisSize = -1.f;
	/* Iris clarity: the alpha of var_eye_iriscolor_ui, which the eye program uses as the weight of
	the iris colour over the base colour. Seeded like the size, -1 until then. */
	f32_t m_fEyeIrisAlpha = -1.f;
	/* The left column's six preset slots. A slot holds what this player made, so it is not
	project data: it is written next to the executable, the way the session diagnostics are,
	and the class is part of the file name so a Warlord face cannot be loaded onto an Artist. */
	static constexpr int32_t SAVE_SLOT_COUNT = 6;
	int32_t m_iSelectedSaveSlot = 0;
	std::array<bool_t, SAVE_SLOT_COUNT> m_SaveSlotFilled{};
	void Refresh_SaveSlots();
	bool_t Save_Slot(const shared_ptr<CCharacter>& pCharacter, int32_t iSlot);
	bool_t Load_Slot(const shared_ptr<CCharacter>& pCharacter, int32_t iSlot);
	/* Every material control back to what the class was authored with, and the view's own
	copies of those values with it. Shared by the first frame of a class and by the whole
	reset, which is why it is not inline in either. */
	void Seed_MaterialControls(const shared_ptr<CCharacter>& pCharacter);
	/* The bottom bar's reset. The face button next to the preset grid resets the face only;
	this one is the retail avatar reset and puts colour, make-up, hair and eyes back too. */
	void Reset_All(const shared_ptr<CCharacter>& pCharacter);
	/* The left column's action row. Cell 0 is the creation pose the screen stands in; the
	other four play a social action once and fall back to it. */
	int32_t m_iSelectedAction = 0;
	void Update_ActionList(const shared_ptr<CCharacter>& pCharacter);
	/* Applies one picker colour to whatever that surface actually is -- a dye for hair, eye
	and a non-native skin, a named head-material parameter for everything else. */
	bool_t Apply_SurfaceColor(const shared_ptr<CCharacter>& pCharacter,
		int32_t iSurface, const float4_t& vColor) const;
	/* Set for the whole frame the picker is up, so the panel underneath reads no clicks --
	the wheel sits directly over the swatches and sliders that opened it. The picker tests
	its own widgets against the router instead of the view's helper. */
	bool_t m_isPickerCapturingPointer = false;
	bool_t Update_ColorPicker(const shared_ptr<CCharacter>& pCharacter);
	static float4_t HsvToRgb(f32_t fHue, f32_t fSaturation, f32_t fValue);
	/* Which preset cell is picked, or -1 for none (the face is whatever the sliders and the
	unmodified mesh give). */
	int32_t m_iSelectedFacePreset = -1;
	std::string m_strIconClassAssetId;
	/* Which of the five try-on costumes the left column has picked. The screen owns the
	choice; applying it to the model belongs to the equipment presentation service. */
	int32_t m_iSelectedCostume = 0;
	bool_t m_bCostumeChanged = false;
	int32_t m_iSelectedHair = 0;
	int32_t m_iHairScrollRow = 0;
	int32_t m_iEyeIrisScrollRow = 0;
	int32_t m_iAdornScrollRow = 0;
	bool_t m_bHairChanged = false;
	/* Sub-tab state of the secondary tabs. */
	bool_t m_isHairTwoTone = false;
	bool_t m_isEyeOddSelected = false;
	int32_t m_iSelectedAdornSub = 0;
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
