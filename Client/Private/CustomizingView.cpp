#include "CustomizingView.h"

#include "Character.h"
#include "FaceCustomizeApplier.h"
#include "GameInstance.h"
#include "MainApp.h"
#include "Model.h"
#include "Transform.h"
#include "UIInputRouter.h"
#include "UILabelFont.h"
#include "UILayoutRuntime.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>

namespace
{
	constexpr f32_t REF_WIDTH = 1280.f;
	constexpr f32_t REF_HEIGHT = 720.f;

	/* Korean labels are written as wide escapes: this project's C++ sources are compiled
	without /utf-8 and a raw Korean byte in a narrow literal breaks the CP949 lead-byte
	handling. Values are the retail pccreate.* strings. */
	constexpr const wchar_t* LABEL_TABS[] = {
		L"\xAE30\xBCF8",					/* base */
		L"\xC5BC\xAD74",					/* face */
		L"\xBA38\xB9AC",					/* hair */
		L"\xB208",							/* eye */
		L"\xD53C\xBD80",					/* skin */
		L"\xAFB8\xBBF8\xAE30",				/* adorn */
		L"\xC74C\xC131",					/* voice */
	};
	constexpr int32_t FACE_TAB_INDEX = 1;
	constexpr int32_t HAIR_TAB_INDEX = 2;
	constexpr int32_t EYE_TAB_INDEX = 3;
	constexpr int32_t SKIN_TAB_INDEX = 4;

	/* One adorn page: the retail head material's colour for that layer, the cooked stamp list
	it picks from, and the texture register that stamp goes on. The eyebrow page has no stamp
	list -- retail changes the brow with a mesh type, not a texture -- so it carries a colour
	only. See Data/UI/Customizing/CustomizingMeshTypes.json. */
	struct ADORN_PAGE
	{
		const char_t* pColorParameter;
		const char_t* pTextureKind;
		int32_t iStampRegister;
	};
	constexpr ADORN_PAGE ADORN_PAGES[] = {
		{ "var_makeup_lipcolor_ui",     "lip",     4 },
		{ "var_makeup_cheekcolor_ui",   "cheek",   6 },
		{ "var_makeup_eyelinecolor_ui", "eyemake", 5 },
		{ "var_eye_browcolor_ui",       nullptr,  -1 },
	};
	constexpr const char_t* EYESHADOW_COLOR_PARAMETER = "var_makeup_eyeshadowcolor_ui";
	constexpr const char_t* SKIN_COLOR_PARAMETER = "var_base_skincolor_ui";

	/* The left column's five action cells, in the order the table lists them.
	EFTable_CharacterCustomizing SecondaryKey 2 gives this class' row as icons 50, 5, 7, 12,
	23 with Object_Unit 0..4, and EFTable_CommonAction's creation block 85000..85004 carries
	the same five in the same order -- CharCustomizing, SC_Charming_1, SC_Greet_3, SC_Dance_1,
	SC_GroupDance_3, whose NameDev is the clip name. Three of the four action icons agree
	between the two tables (5, 7, 12); the creation screen draws its own art for the first and
	last cells. All five clips exist in every class' body AnimSet and are cooked into
	<Class>_CustomizingAnimSet.wmodel by Tools/CharacterCustomizing/trim_psa_clip.py.
	A null clip is cell 0: the pose the screen already stands in. */
	constexpr const char_t* CREATION_ACTION_CLIPS[] = {
		nullptr, "sc_charming_1", "sc_greet_3", "sc_dance_1", "sc_groupdance_3" };

	/* The eye tab drives the retail eye material's own variables. The movie exposes one pair of
	swatches plus an index that chooses which eye, so the odd-eye sub-tab both flips
	`var_eye_useordeyecolor_bool_ui` and points the same two swatches at the left-eye twins. */
	constexpr const char_t* EYE_BASE_COLOR_PARAMETER = "var_eye_basecolor_ui";
	constexpr const char_t* EYE_IRIS_COLOR_PARAMETER = "var_eye_iriscolor_ui";
	constexpr const char_t* EYE_BASE_COLOR_LEFT_PARAMETER = "var_eye_basecolorleft_ui";
	constexpr const char_t* EYE_IRIS_COLOR_LEFT_PARAMETER = "var_eye_iriscolorleft_ui";
	constexpr const char_t* EYE_IRIS_SIZE_PARAMETER = "var_eye_irissize_ui";
	constexpr const char_t* EYE_ODD_COLOR_PARAMETER = "var_eye_useordeyecolor_bool_ui";

	/* The three skin sliders, in layout order. Which head-material variable each drives is
	settled by reading program 4 rather than by the rule document, which leaves two of them
	open:

	  Wrinkle. The document offers var_base_skinnormalintensity_ui and
	  var_headbase_overlaynormalintensity_ui and names neither. In the program they are two
	channels of one constant, and each scales a different map: .z scales texture 0, the face
	normal, and .w scales texture 1, the overlay -- which is the class' wrinkle map
	(pc_ft_wrinkle_00_n) -- before adding it onto the first. The wrinkle slider is the one
	that scales the wrinkle map.

	  Freckle. The document says the Skin tab exposes a FreckleIntensity slider and that no
	freckle scalar exists anywhere; the only freckle parameter is a colour. The program reads
	that colour as `skin = lerp(skin, colour.rgb, mask * colour.a)`, where mask is the cheek
	stamp's alpha sampled on both cheeks. Alpha is the intensity, and it is the only channel
	shaped like one. */
	struct SKIN_SLIDER
	{
		const char_t* pSlotId;
		const char_t* pParameter;
		/* True when the slider is the alpha of a colour parameter rather than a scalar of
		its own, so writing it has to keep the authored RGB. */
		bool_t isColorAlpha;
	};
	constexpr SKIN_SLIDER SKIN_SLIDERS[] = {
		{ "CC_Slider_skin_age",      "var_headbase_overlaynormalintensity_ui", false },
		{ "CC_Slider_skin_shine",    "var_base_skinspecularintensity_ui",      false },
		{ "CC_Slider_skin_freckles", "var_base_skinfrecklecolor_ui",           true  },
	};
	constexpr int32_t ADORN_TAB_INDEX = 5;
	/* The base tab picks a whole appearance at once, and its list is table category 0 -- a
	curated set whose contents were never extracted. CustomizingFacePresets.json carries the
	category 1 face-shape run the face tab uses, which has the same row count per class and is
	a different list, so filling the base grid from it would put the wrong faces behind the
	wrong icons. Until category 0 is extracted the tab is dimmed rather than opened onto a
	blank panel. */
	constexpr int32_t BASE_TAB_INDEX = 0;
	/* Voice is left out on purpose: its list needs audio this project has none of, so drawing
	its rows would only add a row of dead buttons. */
	constexpr int32_t VOICE_TAB_INDEX = 6;

	/* Cell counts of each tab's own rolling lists, sized to the retail viewports. */
	/* Cells the grid draws, not entries the list holds. The retail viewports are this size and
	scroll -- a class carries 38-53 hairstyles and 25-40 irises -- so a grid shows one window
	onto its list and the wheel moves that window a row at a time. */
	constexpr int32_t GRID_COLUMNS = 5;
	constexpr int32_t HAIR_SHAPE_COUNT = 20;
	constexpr int32_t EYE_IRIS_COUNT = 15;
	constexpr int32_t ADORN_ITEM_COUNT = 10;

	constexpr const wchar_t* LABEL_DETAIL_DESC =
		L"\xC5BC\xAD74\xC758 \xC0C1\xC138\xD55C \xACE8\xACA9 \xC124\xC815";
	constexpr const wchar_t* LABEL_FOLD_DETAIL = L"\xC5BC\xAD74 \xC138\xBD80 \xC124\xC815";
	constexpr const wchar_t* LABEL_FOLD_DEFAULT = L"\xAE30\xBCF8";
	/* sys.pccreate.customizing_label_face_desc / _face_form, read out of
	EFTable_GameMsg.db in data2.lpk. */
	constexpr const wchar_t* LABEL_FACE_DESC = L"\xC5BC\xAD74 \xD615\xD0DC \xBCC0\xACBD";
	constexpr const wchar_t* LABEL_FACE_FORM = L"\xAE30\xBCF8 \xC5BC\xAD74";
	/* pccreate.customizing_label_create_change_info, in the three runs its own markup has. */
	constexpr const wchar_t* LABEL_BOTTOM_GUIDE_HEAD =
		L"\xCE90\xB9AD\xD130 \xC0DD\xC131 \xD6C4 ";
	constexpr const wchar_t* LABEL_BOTTOM_GUIDE_ACCENT =
		L"24\xC2DC\xAC04 \xC774\xB0B4 \xC678\xD615\xC744 \xC790\xC720\xB86D\xAC8C \xBCC0\xACBD";
	constexpr const wchar_t* LABEL_BOTTOM_GUIDE_TAIL =
		L" \xAC00\xB2A5\xD569\xB2C8\xB2E4.";
	/* pccreate.customizing_title on rightTitleLabel. */
	constexpr const wchar_t* LABEL_RIGHT_TITLE = L"\xCE90\xB9AD\xD130 \xC0DD\xC131";
	constexpr const wchar_t* LABEL_DECIDE = L"\xC678\xD615 \xACB0\xC815";
	constexpr const wchar_t* LABEL_BACK = L"\xB4A4\xB85C\xAC00\xAE30";
	constexpr const wchar_t* LABEL_RESET_ALL = L"\xC804\xCCB4 \xCD08\xAE30\xD654";
	constexpr const wchar_t* LABEL_FACE_RANDOM = L"\xBB34\xC791\xC704 \xC5BC\xAD74";
	constexpr const wchar_t* LABEL_FACE_RESET = L"\xC5BC\xAD74 \xCD08\xAE30\xD654";
	constexpr const wchar_t* LABEL_GUIDE_ROTATE = L"\xCE90\xB9AD\xD130 \xD68C\xC804";
	/* sys.pccreate.customizing_guide_eyetracking: the middle of the three guides.
	Character creation shows all three; the salon frame shows fewer. */
	constexpr const wchar_t* LABEL_GUIDE_EYETRACK =
		L"\xB9C8\xC6B0\xC2A4 \xBC14\xB77C\xBCF4\xAE30";
	/* Left column captions, at the y the movie places each list at. */
	constexpr const wchar_t* LABEL_LEFT_COSTUME =
		L"\xC758\xC0C1 \xC785\xC5B4\xBCF4\xAE30";		/* costume try-on */
	constexpr const wchar_t* LABEL_LEFT_ACTION = L"\xC561\xC158";	/* action */
	constexpr const wchar_t* LABEL_LEFT_SAVE =
		L"\xCEE4\xC2A4\xD130\xB9C8\xC774\xC9D5 \xC800\xC7A5/\xBD88\xB7EC\xC624\xAE30";
	constexpr const wchar_t* LABEL_LEFT_RESET = L"\xCD08\xAE30\xD654";	/* reset */
	constexpr const wchar_t* LABEL_SAVE = L"\xC800\xC7A5";
	constexpr const wchar_t* LABEL_LOAD = L"\xBD88\xB7EC\xC624\xAE30";
	/* Left panel geometry from CharCustom_LeftPanel: labels at (35,50)/(35,171), the bottom
	page at (33,426) with its own save caption at (2,127). Values are gfx units. */
	constexpr f32_t LEFT_LABEL_X = 35.f;
	constexpr f32_t LEFT_COSTUME_Y = 50.f;
	constexpr f32_t LEFT_ACTION_Y = 171.f;
	/* The background picker is hidden during character creation, and
	CharacterCustomizingLeftPanel then slides bottomPage up to the caption's own y. */
	constexpr f32_t BOTTOM_PAGE_Y = 296.f;
	constexpr f32_t LEFT_SAVE_Y = BOTTOM_PAGE_Y + 127.f;
	/* bottomPage @ (33,426): the recommended-style caption sits at its (2,2). */
	/* bottomPage @ (33,426): the recommended-style caption sits at its (2,2). */
	constexpr f32_t GFX_TO_REF = 2.f / 3.f;
	/* The right panel's content edge: every caption in it is left-aligned here, which is
	where the source screen's text fields sit. */
	constexpr f32_t RIGHT_LABEL_X = 1012.f;
	/* A text field's height is its em size plus the font's ascent/descent overshoot, which the
	sprite fonts bake into their line spacing and UILabelFont sizes by. The exact ratio survives
	in neither the .gfx nor the .spritefont, so this is the one approximated number here. */
	constexpr f32_t EM_TO_LINE_SPACING = 1.25f;
	/* CharCustom_Right_TabFaceDefaultContent is a 390x470 box, so expanding the first
	category pushes the second one down by its whole height. */
	constexpr f32_t FACE_DEFAULT_SECTION_HEIGHT = 470.f * GFX_TO_REF;
	constexpr int32_t FACE_PRESET_COUNT = 25;
	/* leftDressList and leftActionList are five cells each, and the table's SecondaryKey 6
	carries exactly five costume rows per class. */
	constexpr int32_t COSTUME_COUNT = 5;
	/* Retail point sizes, converted the same way every position in this file is. */
	constexpr f32_t SIZE_RIGHT_TITLE = 28.f * GFX_TO_REF;
	constexpr f32_t SIZE_BOTTOM_GUIDE = 16.f * GFX_TO_REF;
	constexpr f32_t SIZE_MOUSE_GUIDE = 14.f * GFX_TO_REF;
	constexpr const wchar_t* LABEL_GUIDE_ZOOM = L"\xD655\xB300/\xCD95\xC18C";
	constexpr const wchar_t* LABEL_NO_SLIDERS =
		L"\xC774 \xD074\xB798\xC2A4\xB294 \xC5BC\xAD74 \xC2AC\xB77C\xC774\xB354 "
		L"\xB370\xC774\xD130\xAC00 \xC544\xC9C1 \xC5C6\xC2B5\xB2C8\xB2E4.";
	/* The retail strings, taken from the client's own sys.pccreate.customizing_* table
	rather than retyped. Written as wide escapes because this file has to stay ASCII. */
	constexpr const wchar_t* LABEL_HAIR_SUB_BASE        = L"\xAE30\xBCF8";	/* customizing_button_hair_base */
	constexpr const wchar_t* LABEL_HAIR_SUB_TWOTONE     = L"\xD22C\xD1A4";	/* customizing_button_hair_two_tone */
	constexpr const wchar_t* LABEL_EYE_SUB_BOTH         = L"\xC591\xCABD \xB208";	/* customizing_button_eye_normal */
	constexpr const wchar_t* LABEL_EYE_SUB_ODD          = L"\xC624\xB4DC\xC544\xC774";	/* customizing_button_eye_odd_eye */
	constexpr const wchar_t* LABEL_ADORN_SUB_LIP        = L"\xC785\xC220";	/* customizing_label_lips */
	constexpr const wchar_t* LABEL_ADORN_SUB_CHEEK      = L"\xBCFC \xD130\xCE58";	/* customizing_button_cheek */
	constexpr const wchar_t* LABEL_ADORN_SUB_EYEMAKE    = L"\xB208 \xD654\xC7A5";	/* customizing_button_eye_makeup */
	constexpr const wchar_t* LABEL_ADORN_SUB_EYEBROW    = L"\xB208\xC379";	/* customizing_button_eyebrow */
	constexpr const wchar_t* LABEL_HAIR_FORM            = L"\xBA38\xB9AC \xD615\xD0DC";	/* customizing_label_hair_form */
	constexpr const wchar_t* LABEL_HAIR_DESC            = L"\xBA38\xB9AC \xD615\xD0DC, \xC0C9\xC0C1 \xBCC0\xACBD";	/* customizing_label_hair_desc */
	constexpr const wchar_t* LABEL_HAIR_COLOR           = L"\xBA38\xB9AC \xC0C9\xC0C1";	/* customizing_label_hair_color */
	constexpr const wchar_t* LABEL_HAIR_COLOR_TWOTONE   = L"\xBA38\xB9AC \xCD94\xAC00\xC0C9\xC0C1";	/* customizing_label_hair_color_two_tone */
	constexpr const wchar_t* LABEL_STRENGTH             = L"\xAC15\xB3C4";	/* customizing_label_strength */
	constexpr const wchar_t* LABEL_RANGE                = L"\xBC94\xC704";	/* customizing_label_range */
	constexpr const wchar_t* LABEL_EYE_DESC             = L"\xB208 \xD615\xD0DC\xC640 \xC0C9\xC0C1 \xBCC0\xACBD";	/* customizing_label_eye_desc */
	constexpr const wchar_t* LABEL_EYE_FORM             = L"\xB208 \xBAA8\xC591";	/* customizing_label_detail_eye_form */
	constexpr const wchar_t* LABEL_EYE_COLOR            = L"\xB208 \xC0C9\xC0C1";	/* customizing_label_eye_color */
	constexpr const wchar_t* LABEL_EYE_IRIS_COLOR       = L"\xD64D\xCC44 \xC0C9\xC0C1";	/* customizing_label_eye_iris_color */
	constexpr const wchar_t* LABEL_EYE_SIZE             = L"\xB208\xB3D9\xC790 \xD06C\xAE30";	/* customizing_label_eye_size */
	constexpr const wchar_t* LABEL_ALPHA                = L"\xC120\xBA85\xB3C4";	/* customizing_label_alpha */
	constexpr const wchar_t* LABEL_SKIN_DESC            = L"\xD53C\xBD80\xC758 \xC0C9\xC0C1, \xB098\xC774 \xB4F1 \xBCC0\xACBD";	/* customizing_label_skin_desc */
	constexpr const wchar_t* LABEL_SKIN_COLOR           = L"\xD53C\xBD80 \xC0C9\xC0C1";	/* customizing_label_skin_color */
	constexpr const wchar_t* LABEL_SKIN_WRINKLE         = L"\xD53C\xBD80 \xB098\xC774";	/* customizing_label_skin_wrinkle */
	constexpr const wchar_t* LABEL_SKIN_GLOSS           = L"\xD53C\xBD80 \xC724\xAE30";	/* customizing_label_skin_gloss */
	constexpr const wchar_t* LABEL_SKIN_FRECKLE         = L"\xC8FC\xADFC\xAE68";	/* customizing_label_skin_freckle */
	constexpr const wchar_t* LABEL_ADORN_DESC           = L"\xD654\xC7A5\xC758 \xD615\xD0DC, \xC0C9\xC0C1 \xBCC0\xACBD";	/* customizing_label_makeup_desc */
	constexpr const wchar_t* LABEL_ADORN_SHADOW_COLOR   = L"\xC544\xC774\xC100\xB3C4 \xC0C9\xC0C1";	/* customizing_label_makeup_eyeshadow_color */
	constexpr const wchar_t* LABEL_EYE_LENZ             = L"\xB3D9\xACF5 \xD615\xD0DC";	/* customizing_label_eye_lenz */
	constexpr const wchar_t* LABEL_HAIR_GLOSS           = L"\xBA38\xB9AC \xC724\xAE30";	/* customizing_label_hair_gloss */
	constexpr const wchar_t* LABEL_ADORN_LIP_FORM       = L"\xC785\xC220 \xD615\xD0DC";	/* customizing_label_makeup_lips_form */
	constexpr const wchar_t* LABEL_ADORN_LIP_COLOR      = L"\xC785\xC220 \xC0C9\xC0C1";	/* customizing_label_makeup_lips_color */
	constexpr const wchar_t* LABEL_ADORN_CHEEK_FORM     = L"\xBCFC \xD130\xCE58 \xD615\xD0DC";	/* customizing_label_makeup_cheek_form */
	constexpr const wchar_t* LABEL_ADORN_CHEEK_COLOR    = L"\xBCFC \xD130\xCE58 \xC0C9\xC0C1";	/* customizing_label_makeup_cheek_form_color */
	constexpr const wchar_t* LABEL_ADORN_EYEMAKE_FORM   = L"\xB208 \xD654\xC7A5 \xD615\xD0DC";	/* customizing_label_makeup_form */
	constexpr const wchar_t* LABEL_ADORN_EYELINE_COLOR  = L"\xC544\xC774\xB77C\xC778 \xC0C9\xC0C1";	/* customizing_label_makeup_eyeline_color */
	constexpr const wchar_t* LABEL_ADORN_BROW_COLOR     = L"\xB208\xC370 \xC0C9\xC0C1";	/* customizing_label_eyebrow_color */
	constexpr const wchar_t* LABEL_SAVE_SLOT            = L"\xCEE4\xC2A4\xD130\xB9C8\xC774\xC9D5 \xC800\xC7A5/\xBD88\xB7EC\xC624\xAE30";	/* customizing_label_save_slot */
	/* sys.pccreate.checkname_button_ok / .customizing_btn_avatarreset */
	constexpr const wchar_t* LABEL_PICKER_APPLY = L"\xD655\xC778";
	constexpr const wchar_t* LABEL_PICKER_CANCEL = L"\xCDE8\xC18C";
	constexpr const wchar_t* LABEL_NOT_READY = L"\xC900\xBE44 \xC911\xC778 \xD56D\xBAA9\xC785\xB2C8\xB2E4.";

	/* One row of CharCustom_Right_TabFaceDetailPart<N>: the slider id in
	<race>.facesliders.json, its retail row caption, and which part sprite it belongs to.
	fPartY is the row's y inside that part sprite, matching the same value
	build_customizing_ui.py placed the track and thumb slots at. */
	struct FACE_SLIDER_ROW
	{
		int32_t iPart;
		const char_t* pSliderId;
		const wchar_t* pLabel;
		f32_t fPartY;
	};

	constexpr const wchar_t* ROW_HORIZONTAL = L"\xAC00\xB85C";
	constexpr const wchar_t* ROW_VERTICAL = L"\xC138\xB85C";
	constexpr const wchar_t* ROW_GAP = L"\xAC04\xACA9";
	constexpr const wchar_t* ROW_HEIGHT = L"\xB192\xC774";
	constexpr const wchar_t* ROW_ANGLE = L"\xAC01\xB3C4";
	constexpr const wchar_t* ROW_PULL = L"\xB3CC\xCD9C";
	constexpr const wchar_t* ROW_SCALE = L"\xD06C\xAE30";

	constexpr FACE_SLIDER_ROW FACE_SLIDER_ROWS[] = {
		{ 0, "eye_scaleh", ROW_HORIZONTAL, 26.f },
		{ 0, "eye_scalev", ROW_VERTICAL, 56.f },
		{ 0, "eye_width", ROW_GAP, 119.f },
		{ 0, "eye_height", ROW_HEIGHT, 149.f },
		{ 0, "eye_angle", ROW_ANGLE, 211.f },
		{ 1, "eyebrow_height", ROW_HEIGHT, 26.f },
		{ 1, "eyebrow_angle", ROW_ANGLE, 56.f },
		{ 2, "cheekbone_pull", ROW_PULL, 26.f },
		{ 2, "cheekbone_height", ROW_HEIGHT, 56.f },
		{ 3, "jaw_scale", ROW_SCALE, 26.f },
		{ 3, "jaw_height", ROW_HEIGHT, 56.f },
		{ 3, "jaw_pull", ROW_PULL, 119.f },
		{ 3, "jaw_angle", ROW_ANGLE, 149.f },
		{ 4, "nose_scale", ROW_SCALE, 26.f },
		{ 4, "nose_height", ROW_HEIGHT, 56.f },
		{ 4, "nose_length", ROW_PULL, 86.f },
		{ 5, "mouthlip_height", ROW_HEIGHT, 26.f },
	};

	/* The group captions each part sprite draws above its rows, at their own y. */
	struct FACE_PART_HEADER
	{
		int32_t iPart;
		const wchar_t* pLabel;
		f32_t fPartY;
	};

	constexpr FACE_PART_HEADER FACE_PART_HEADERS[] = {
		{ 0, L"\xB208 \xD06C\xAE30", -1.f },
		{ 0, L"\xB208 \xC704\xCE58", 92.f },
		{ 0, L"\xD68C\xC804", 184.f },
		{ 1, L"\xB208\xC378 \xD615\xD0DC", -1.f },
		{ 2, L"\xAD11\xB300 \xD615\xD0DC", -1.f },
		{ 3, L"\xD131 \xD06C\xAE30", -1.f },
		{ 3, L"\xD131 \xBAA8\xC591", 92.f },
		{ 4, L"\xCF54 \xD615\xD0DC", -1.f },
		{ 5, L"\xC785 \xD615\xD0DC", -1.f },
	};

	constexpr const wchar_t* LABEL_PARTS[] = {
		L"\xB208",							/* eye */
		L"\xB208\xC378",					/* eyebrow */
		L"\xAD11\xB300",					/* cheekbone */
		L"\xD131",							/* jaw */
		L"\xCF54",							/* nose */
		L"\xC785",							/* mouth */
	};
	constexpr int32_t FACE_PART_COUNT = 6;

	/* Part sprite origin in reference units: CharCustom_Right_TabFace (-437,192) > detail
	(0,48) > content (34,51) > part (0,118), taken from 1920x1080 to 1280x720. */
	constexpr f32_t PART_ORIGIN_X = (1920.f - 437.f + 34.f) * 2.f / 3.f;
	constexpr f32_t PART_ORIGIN_Y = (192.f + 48.f + 51.f + 118.f) * 2.f / 3.f;
	constexpr f32_t PART_SCALE = 2.f / 3.f;

	constexpr const char_t* ASSET_TAB_BG = "UI/Customizing/tab_bg.png";
	constexpr const char_t* ASSET_TAB_BG_SELECTED = "UI/Customizing/tab_bg_selected.png";
	constexpr const char_t* ASSET_SUBTAB_NORMAL = "UI/Customizing/v2_tab2_normal.png";
	constexpr const char_t* ASSET_SUBTAB_OVER = "UI/Customizing/v2_tab2_over.png";
	constexpr const char_t* ASSET_BUTTON_NORMAL = "UI/Customizing/v2_btn_normal.png";
	constexpr const char_t* ASSET_BUTTON_OVER = "UI/Customizing/v2_btn_over.png";
	constexpr const char_t* ASSET_THUMB_NORMAL = "UI/Customizing/v2_slider_thumb.png";
	constexpr const char_t* ASSET_THUMB_OVER = "UI/Customizing/v2_slider_thumb_over.png";
	constexpr const char_t* ASSET_BACK_ICON = "UI/Customizing/back_icon.png";
	constexpr const char_t* ASSET_BACK_ICON_OVER = "UI/Customizing/back_icon_over.png";
	constexpr const char_t* ASSET_RESET_ICON = "UI/Customizing/resetall_icon.png";
	constexpr const char_t* ASSET_RESET_ICON_OVER = "UI/Customizing/resetall_icon_over.png";

	/* Framing lifted from the retail class-select stage. Three camera actors stand at the
	character's spot there: a face camera 0.92 m out at 0.83 m looking 3.9 degrees up, a
	full-body camera 5.45 m out at 1.01 m looking 2.5 degrees up with a 40 degree FOV, and a
	wide stage camera at 12 m the customizing screen does not use. Distances are the retail
	centimetre values divided by 100, the same conversion the map import pipeline applies. */
	constexpr f32_t FULLBODY_DISTANCE = 2.50f;
	constexpr f32_t FULLBODY_LOOK_HEIGHT = 0.62f;
	constexpr f32_t FULLBODY_PITCH = 2.5f;
	constexpr f32_t FACE_DISTANCE = 0.65f;
	constexpr f32_t FACE_PITCH = 3.9f;
	/* The four class bodies are not the same height, so the face framing is measured off the
	model instead of being one constant. These are the rig's own eye bones, whose combined
	matrix already carries the admission scale each class is imported with:

	    LanceMaster 1.040   DimensionMaster 1.010   Warlord 0.917   Artist 0.862   (metres)

	FACE_DISTANCE was set against the tallest of them, so the distance travels with the
	measured height and a shorter class's head fills the same share of the frame. */
	constexpr const char_t* EYE_BONE_NAMES[] = { "b_fc_l_eye_ani", "b_fc_r_eye_ani" };
	constexpr f32_t REFERENCE_EYE_HEIGHT = 1.040f;
	constexpr f32_t FACE_LOOK_HEIGHT_FALLBACK = REFERENCE_EYE_HEIGHT;
	/* The retail full-body camera carries this on its own FOVAngle property. */
	constexpr f32_t CUSTOMIZING_FOV_Y_DEGREES = 40.f;
	/* How fast the two-state wheel toggle settles. */
	constexpr f32_t ZOOM_RESPONSE = 9.f;
	/* The two panels and the bottom bar own the pointer; a drag started between them orbits.
	The left panel is 402 gfx units wide, the right one starts at 1475. */
	constexpr f32_t PANEL_LEFT_EDGE = 983.f;
	constexpr f32_t PANEL_RIGHT_OF_LEFT_EDGE = 268.f;
	constexpr f32_t BOTTOM_BAR_TOP_EDGE = 640.f;
	/* Fraction of the half-width the subject is shifted left by. */
	f32_t Subject_ScreenShiftFraction()
	{
		constexpr f32_t HALF_WIDTH = REF_WIDTH * 0.5f;
		const f32_t fFreeCentre =
			(PANEL_RIGHT_OF_LEFT_EDGE + PANEL_LEFT_EDGE) * 0.5f;
		return (HALF_WIDTH - fFreeCentre) / HALF_WIDTH;
	}

	f32_t Camera_HorizontalTangent()
	{
		const float2_t vViewport = CGameInstance::Get().Get_ViewportSize();
		const f32_t fAspect = (vViewport.x > 0.f && vViewport.y > 0.f) ?
			(vViewport.x / vViewport.y) : (16.f / 9.f);
		return std::tan(XMConvertToRadians(CUSTOMIZING_FOV_Y_DEGREES * 0.5f)) * fAspect;
	}

	string Slider_TrackSlotId(const char_t* pSliderId)
	{
		return string("CC_Slider_") + pSliderId + "_Track";
	}
	string Slider_ThumbSlotId(const char_t* pSliderId)
	{
		return string("CC_Slider_") + pSliderId + "_Thumb";
	}

	/* Index of pSliderId inside the character's applier, or -1 when this class's document has
	no such slider (a race whose bones that slider needs are all missing, or no document). */
	int32_t Find_SliderIndex(const CCharacter& Character, const char_t* pSliderId)
	{
		const CFaceCustomizeApplier& Face = Character.Get_FaceCustomize();
		for (size_t i = 0; i < Face.Get_SliderCount(); ++i)
		{
			if (Face.Get_SliderId(i) == pSliderId)
				return static_cast<int32_t>(i);
		}
		return -1;
	}
}

Client::CCustomizingView::CCustomizingView(
	ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext,
	const uint32_t iGameObjectLevelIndex)
	: m_pView{ std::make_unique<CUILayoutRuntime>(pDevice, pContext,
		iGameObjectLevelIndex, TEXT("Layer_UI"),
		L"UI/Customizing/CustomizingUI.json") }
{
	Hide();
}

Client::CCustomizingView::~CCustomizingView() = default;

void Client::CCustomizingView::Open()
{
	m_bOpen = true;
	m_iSelectedTab = FACE_TAB_INDEX;
	m_iSelectedPart = 0;
	m_iDraggingRow = -1;
	m_bOrbitDragging = false;
	m_fOrbitYaw = 0.f;
	m_hasCapturedCharacterYaw = false;
	m_fZoomTarget = 0.f;
	m_fZoomBlend = 0.f;
	m_bDecideRequested = false;
	m_bBackRequested = false;
	/* The hair a cooked body happens to draw by itself is not the retail starting look --
	one class shows a style nobody picked and another has no hair mesh at all. Asking for
	the selected style up front puts a real hairstyle on every class, and it is the same
	apply the grid uses, so nothing about it is a second path. */
	m_bHairChanged = true;
	/* Same for the outfit. The class default equipment is its starting armour, not the
	plain outfit the creation screen shows, and entry 0 of the try-on list is that
	outfit -- so the screen opens on it rather than on a suit of armour. */
	m_bCostumeChanged = true;
}

void Client::CCustomizingView::Close()
{
	m_bOpen = false;
	m_iDraggingRow = -1;
	m_bOrbitDragging = false;
	Hide();
}

void Client::CCustomizingView::Hide()
{
	if (nullptr == m_pView)
		return;
	m_pView->Set_AllSlotsVisible(false);
}

bool_t Client::CCustomizingView::Get_SlotRect(const char_t* pSlotId,
	f32_t& outX, f32_t& outY, f32_t& outWidth, f32_t& outHeight) const
{
	return nullptr != m_pView &&
		m_pView->Get_SlotRect(pSlotId, outX, outY, outWidth, outHeight) &&
		outWidth > 0.f && outHeight > 0.f;
}

bool_t Client::CCustomizingView::Is_Hovered(
	const f32_t fX, const f32_t fY, const f32_t fWidth, const f32_t fHeight) const
{
	return CUIInputRouter::Get().Is_Hovered(fX, fY, fWidth, fHeight, REF_WIDTH, REF_HEIGHT);
}

bool_t Client::CCustomizingView::Is_Clicked(
	const f32_t fX, const f32_t fY, const f32_t fWidth, const f32_t fHeight) const
{
	if (m_isPickerCapturingPointer)
		return false;
	return CUIInputRouter::Get().Is_Clicked(fX, fY, fWidth, fHeight, REF_WIDTH, REF_HEIGHT);
}

bool_t Client::CCustomizingView::Try_Consume_Decide()
{
	const bool_t bRequested = m_bDecideRequested;
	m_bDecideRequested = false;
	return bRequested;
}

bool_t Client::CCustomizingView::Try_Consume_Back()
{
	const bool_t bRequested = m_bBackRequested;
	m_bBackRequested = false;
	return bRequested;
}

bool_t Client::CCustomizingView::Try_Consume_CostumeChange()
{
	const bool_t bChanged = m_bCostumeChanged;
	m_bCostumeChanged = false;
	return bChanged;
}

bool_t Client::CCustomizingView::Try_Consume_HairChange()
{
	const bool_t bChanged = m_bHairChanged;
	m_bHairChanged = false;
	return bChanged;
}

f32_t Client::CCustomizingView::Get_FieldOfViewDegrees() const
{
	return CUSTOMIZING_FOV_Y_DEGREES;
}

f32_t Client::CCustomizingView::Get_FaceLookHeight() const
{
	return m_fMeasuredEyeHeight > 0.f ? m_fMeasuredEyeHeight : FACE_LOOK_HEIGHT_FALLBACK;
}

f32_t Client::CCustomizingView::Get_Distance() const
{
	const f32_t fFaceDistance =
		FACE_DISTANCE * (Get_FaceLookHeight() / REFERENCE_EYE_HEIGHT);
	return FULLBODY_DISTANCE + (fFaceDistance - FULLBODY_DISTANCE) * m_fZoomBlend;
}

f32_t Client::CCustomizingView::Get_Pitch() const
{
	return FULLBODY_PITCH + (FACE_PITCH - FULLBODY_PITCH) * m_fZoomBlend;
}

float3_t Client::CCustomizingView::Get_LateralOffset() const
{
	/* Shifting the eye and the look point by the same sideways vector moves the subject across
	the screen without turning the camera. The camera moves right, so the character reads as
	left of centre. */
	const f32_t fYaw = XMConvertToRadians(m_fCharacterYawDegrees);
	const f32_t fLateral =
		Subject_ScreenShiftFraction() * Camera_HorizontalTangent() * Get_Distance();
	return float3_t(-std::cos(fYaw) * fLateral, 0.f, std::sin(fYaw) * fLateral);
}

f32_t Client::CCustomizingView::Get_LookHeight() const
{
	return FULLBODY_LOOK_HEIGHT +
		(Get_FaceLookHeight() - FULLBODY_LOOK_HEIGHT) * m_fZoomBlend;
}

float3_t Client::CCustomizingView::Get_CameraPositionOffset() const
{
	/* Yaw 0 stands the camera directly in front of the character's own facing. */
	const f32_t fYaw = XMConvertToRadians(m_fCharacterYawDegrees);
	/* The retail pitch is the angle the camera looks UP at the character, so the eye sits
	that much below the point it aims at. */
	const f32_t fPitch = XMConvertToRadians(Get_Pitch());
	const f32_t fDistance = Get_Distance();
	const f32_t fHorizontal = fDistance * std::cos(fPitch);
	const float3_t vLateral = Get_LateralOffset();
	return float3_t(
		fHorizontal * std::sin(fYaw) + vLateral.x,
		Get_LookHeight() - fDistance * std::sin(fPitch),
		fHorizontal * std::cos(fYaw) + vLateral.z);
}

float3_t Client::CCustomizingView::Get_CameraLookOffset() const
{
	const float3_t vLateral = Get_LateralOffset();
	return float3_t(vLateral.x, Get_LookHeight(), vLateral.z);
}

void Client::CCustomizingView::Update(
	const f32_t fTimeDelta, const shared_ptr<CCharacter>& pCharacter)
{
	if (!m_bOpen || nullptr == m_pView)
		return;

	/* The turntable is measured from the character's own facing, so it has to be read every
	frame: the replicated transform is what decides which way the model is looking. */
	if (nullptr != pCharacter && nullptr != pCharacter->Get_Transform())
	{
		float3_t vLook{};
		XMStoreFloat3(&vLook,
			pCharacter->Get_Transform()->Get_State(Engine::STATE::LOOK));
		/* Captured once, on the frame the subject first appears. The drag turns the model,
		so re-reading its facing every frame would swing the camera along with it and cancel
		the rotation on screen -- the world-fixed lighting was the only thing that still
		showed it moving. The camera stays where the retail framing puts it. */
		if (!m_hasCapturedCharacterYaw &&
			std::isfinite(vLook.x) && std::isfinite(vLook.z) &&
			(std::fabs(vLook.x) > 1e-4f || std::fabs(vLook.z) > 1e-4f))
		{
			m_fCharacterYawDegrees =
				XMConvertToDegrees(std::atan2(vLook.x, vLook.z));
			m_hasCapturedCharacterYaw = true;
		}
	}

	/* Decided before anything reads the pointer this frame. */
	m_isPickerCapturingPointer = PICKER_SURFACE_NONE != m_iPickerSurface;
	Update_SubjectMetrics(pCharacter);
	Apply_ListIcons(pCharacter);
	Update_Orbit(fTimeDelta);
	Update_Tabs();
	Update_FaceTab(pCharacter);
	Update_SecondaryTabs(pCharacter);
	Update_Buttons(pCharacter);
	/* Last: its own widgets test the router directly, so the capture above does not block
	the picker itself. */
	Update_ColorPicker(pCharacter);
}

void Client::CCustomizingView::Update_Orbit(const f32_t fTimeDelta)
{
	CUIInputRouter& Router = CUIInputRouter::Get();
	f32_t fMouseX = 0.f, fMouseY = 0.f;
	const bool_t bHasMouse = Router.Get_MousePosition(REF_WIDTH, REF_HEIGHT, fMouseX, fMouseY);
	const bool_t bOverScene = bHasMouse &&
		fMouseX > PANEL_RIGHT_OF_LEFT_EDGE && fMouseX < PANEL_LEFT_EDGE &&
		fMouseY < BOTTOM_BAR_TOP_EDGE;

	if (m_bOrbitDragging)
	{
		if (!Router.Is_LeftDown())
		{
			m_bOrbitDragging = false;
		}
		else if (bHasMouse)
		{
			/* Yaw only: the retail guide offers rotate and zoom, no free pitch. The model
			follows the drag -- pulling left turns its near side left -- so the sign is the
			cursor's, not its opposite. */
			m_fOrbitYaw += (fMouseX - m_fLastMouseX) * 0.4f;
		}
	}
	else if (bOverScene && Router.Is_LeftClickEdge())
	{
		m_bOrbitDragging = true;
	}
	m_fLastMouseX = fMouseX;
	m_fLastMouseY = fMouseY;

	/* One wheel notch swaps the whole framing in the original, so this is a toggle rather
	than a continuous distance. */
	const int32_t iWheel = CGameInstance::Get().Get_DIMouseMove(DIMM::WHEEL);
	if (!Consume_TabGridScroll(iWheel))
	{
		if (iWheel > 0)
			m_fZoomTarget = 1.f;
		else if (iWheel < 0)
			m_fZoomTarget = 0.f;
	}
	const f32_t fStep = std::clamp((max)(0.f, fTimeDelta), 0.f, 0.1f);
	m_fZoomBlend += (m_fZoomTarget - m_fZoomBlend) *
		(1.f - std::exp(-ZOOM_RESPONSE * fStep));
}

void Client::CCustomizingView::Update_Tabs()
{
	for (int32_t i = 0; i < static_cast<int32_t>(std::size(LABEL_TABS)); ++i)
	{
		const string strBgId = "CC_Tab" + std::to_string(i) + "_Bg";
		const string strGlowId = "CC_Tab" + std::to_string(i) + "_Glow";
		const string strIconId = "CC_Tab" + std::to_string(i) + "_Icon";
		const bool_t bSelected = i == m_iSelectedTab;
		const bool_t bSupported = VOICE_TAB_INDEX != i && BASE_TAB_INDEX != i;

		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!Get_SlotRect(strBgId.c_str(), fX, fY, fWidth, fHeight))
			continue;
		const bool_t bHovered = bSupported && Is_Hovered(fX, fY, fWidth, fHeight);

		m_pView->Set_SlotVisible(strBgId, true);
		m_pView->Set_SlotTexture(strBgId,
			bSelected ? ASSET_TAB_BG_SELECTED : ASSET_TAB_BG);
		m_pView->Set_SlotVisible(strGlowId, bHovered && !bSelected);
		m_pView->Set_SlotVisible(strIconId, true);
		/* A tab whose data slice does not exist yet is dimmed instead of pretending to be
		clickable -- the same honest state the retail disabled frame carries. */
		const float4_t vTint = bSupported ?
			float4_t(1.f, 1.f, 1.f, 1.f) : float4_t(0.45f, 0.45f, 0.45f, 1.f);
		m_pView->Set_SlotTint(strBgId, vTint);
		m_pView->Set_SlotTint(strIconId, vTint);

		if (bHovered && Is_Clicked(fX, fY, fWidth, fHeight))
		{
			CMainApp::Play_UIButtonClickSound();
			m_iSelectedTab = i;
		}
	}

	for (const char_t* pSlotId : { "CC_LeftBg", "CC_RightBg", "CC_BottomBg",
		"CC_GuideRotateIcon", "CC_GuideEyeTrackIcon", "CC_GuideZoomIcon" })
	{
		m_pView->Set_SlotVisible(pSlotId, true);
	}
	/* CharCustom_TopPanel carries the paid appearance-change notice, which the creation flow
	never shows. */
	for (const char_t* pSlotId : { "CC_TopBannerA", "CC_TopBannerB", "CC_TopGuideBg" })
		m_pView->Set_SlotVisible(pSlotId, false);
	/* The left column's rolling lists: each CharCustom_LeftRenderer cell is its own plate
	under the shared V2Slot_border frame, and the panel art behind them is a translucent
	black sheet that reads as nothing on its own. */
	for (int32_t i = 0; i < COSTUME_COUNT; ++i)
	{
		for (const string& strId : { "CC_LeftDress" + std::to_string(i),
			"CC_LeftAction" + std::to_string(i) })
		{
			m_pView->Set_SlotVisible(strId, true);
			m_pView->Set_SlotVisible(strId + "_Plate", true);
		}

		/* Picking a costume only records which one is wanted. Putting it on the model is the
		equipment presentation service's job, driven from the owning Level once the catalog
		carries these sets. */
		const string strDressId = "CC_LeftDress" + std::to_string(i);
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (Get_SlotRect(strDressId.c_str(), fX, fY, fWidth, fHeight) &&
			Is_Hovered(fX, fY, fWidth, fHeight) &&
			Is_Clicked(fX, fY, fWidth, fHeight))
		{
			CMainApp::Play_UIButtonClickSound();
			if (m_iSelectedCostume != i)
				m_bCostumeChanged = true;
			m_iSelectedCostume = i;
		}
		const string strActionId = "CC_LeftAction" + std::to_string(i);
		m_pView->Set_SlotTint(strActionId, i == m_iSelectedAction ?
			float4_t(1.f, 1.f, 1.f, 1.f) : float4_t(0.7f, 0.7f, 0.7f, 1.f));
	}
	/* The six preset slots. An empty one is dimmed the same way an unsupported tab is, and
	the one the save and load buttons act on is the bright one. */
	for (int32_t i = 0; i < SAVE_SLOT_COUNT; ++i)
	{
		const string strId = "CC_LeftSave" + std::to_string(i);
		m_pView->Set_SlotVisible(strId, true);
		m_pView->Set_SlotVisible(strId + "_Plate", true);
		const bool_t bSelected = i == m_iSelectedSaveSlot;
		const f32_t fLevel = bSelected ? 1.f : (m_SaveSlotFilled[static_cast<size_t>(i)] ?
			0.7f : 0.45f);
		m_pView->Set_SlotTint(strId, float4_t(fLevel, fLevel, fLevel, 1.f));
	}
	for (const char_t* pSlotId : { "CC_LeftSaveBtn", "CC_LeftLoadBtn" })
		m_pView->Set_SlotVisible(pSlotId, true);
}

void Client::CCustomizingView::Apply_SliderVisibility()
{
	for (const FACE_SLIDER_ROW& Row : FACE_SLIDER_ROWS)
	{
		const bool_t bVisible = FACE_TAB_INDEX == m_iSelectedTab && Row.iPart == m_iSelectedPart;
		m_pView->Set_SlotVisible(Slider_TrackSlotId(Row.pSliderId), bVisible);
		m_pView->Set_SlotVisible(Slider_ThumbSlotId(Row.pSliderId), bVisible);
	}
}

namespace
{
	/* A preset slot is what this player made, not project data, so it goes next to the
	executable the way the session JSONL does. The class asset id is part of the name: the
	slider set, the make-up parameters and the eye material all differ per class, so a slot
	written on one class is not loadable on another and is simply not offered there. */
	std::filesystem::path SaveSlotPath(const std::string& strClassAssetId, const int32_t iSlot)
	{
		wchar_t modulePath[MAX_PATH]{};
		if (strClassAssetId.empty() || 0 == GetModuleFileNameW(nullptr, modulePath, MAX_PATH))
			return {};
		return std::filesystem::path(modulePath).parent_path() / L"CustomizingPresets" /
			(strClassAssetId + ".slot" + std::to_string(iSlot) + ".json");
	}

	/* Small enough to write by hand: the payload is numbers, short arrays and one string
	map, and the reader is CDataJson as everywhere else. */
	void WriteNumber(std::ostringstream& out, const f32_t fValue)
	{
		out << static_cast<double>(fValue);
	}

	void WriteVector(std::ostringstream& out, const char_t* pName, const float4_t& vValue)
	{
		out << "  \"" << pName << "\": [";
		for (const f32_t f : { vValue.x, vValue.y, vValue.z, vValue.w })
		{
			WriteNumber(out, f);
			if (&f != &vValue.w)
				out << ", ";
		}
		out << "],\n";
	}

	f32_t ReadNumber(const DATA_JSON_VALUE* pValue, const f32_t fFallback)
	{
		return nullptr != pValue && pValue->Is_Number() ?
			static_cast<f32_t>(pValue->Get_Number()) : fFallback;
	}

	bool_t ReadVector(const DATA_JSON_VALUE* pValue, float4_t& outValue)
	{
		if (nullptr == pValue || !pValue->Is_Array() || 4u != pValue->Get_Array().size())
			return false;
		f32_t* pChannels = &outValue.x;
		for (size_t i = 0; i < 4u; ++i)
		{
			const DATA_JSON_VALUE& channel = pValue->Get_Array()[i];
			if (!channel.Is_Number())
				return false;
			pChannels[i] = static_cast<f32_t>(channel.Get_Number());
		}
		return true;
	}
}

void Client::CCustomizingView::Refresh_SaveSlots()
{
	for (int32_t i = 0; i < SAVE_SLOT_COUNT; ++i)
	{
		const std::filesystem::path path = SaveSlotPath(m_strIconClassAssetId, i);
		std::error_code error;
		m_SaveSlotFilled[static_cast<size_t>(i)] =
			!path.empty() && std::filesystem::exists(path, error) && !error;
	}
}

bool_t Client::CCustomizingView::Save_Slot(
	const shared_ptr<CCharacter>& pCharacter, const int32_t iSlot)
{
	if (nullptr == pCharacter || iSlot < 0 || iSlot >= SAVE_SLOT_COUNT)
		return false;
	const std::filesystem::path path = SaveSlotPath(m_strIconClassAssetId, iSlot);
	if (path.empty())
		return false;
	std::error_code error;
	std::filesystem::create_directories(path.parent_path(), error);
	if (error)
		return false;

	std::ostringstream out;
	out << "{\n";
	out << "  \"schema\": \"lostark.customizing-preset\",\n";
	out << "  \"formatVersion\": 1,\n";
	out << "  \"class\": \"" << m_strIconClassAssetId << "\",\n";
	/* Sliders are stored by their own id rather than by index: the index is a position in
	whatever list the class' race document happens to carry, and that is not a save key. */
	out << "  \"faceSliders\": {\n";
	const CFaceCustomizeApplier& Face = pCharacter->Get_FaceCustomize();
	for (size_t i = 0; i < Face.Get_SliderCount(); ++i)
	{
		out << "    \"" << Face.Get_SliderId(i) << "\": ";
		WriteNumber(out, Face.Get_Weight(i));
		out << (i + 1u < Face.Get_SliderCount() ? ",\n" : "\n");
	}
	out << "  },\n";
	out << "  \"facePreset\": " << m_iSelectedFacePreset << ",\n";
	out << "  \"surfaceColors\": [\n";
	for (size_t i = 0; i < m_SurfaceColors.size(); ++i)
	{
		out << "    [";
		const float4_t& v = m_SurfaceColors[i];
		for (const f32_t f : { v.x, v.y, v.z, v.w })
		{
			WriteNumber(out, f);
			if (&f != &v.w)
				out << ", ";
		}
		out << (i + 1u < m_SurfaceColors.size() ? "],\n" : "]\n");
	}
	out << "  ],\n";
	out << "  \"adornStrength\": [";
	for (size_t i = 0; i < m_AdornStrength.size(); ++i)
	{
		WriteNumber(out, m_AdornStrength[i]);
		out << (i + 1u < m_AdornStrength.size() ? ", " : "");
	}
	out << "],\n";
	out << "  \"adornItems\": [";
	for (size_t i = 0; i < m_SelectedAdornItems.size(); ++i)
		out << m_SelectedAdornItems[i] << (i + 1u < m_SelectedAdornItems.size() ? ", " : "");
	out << "],\n";
	out << "  \"skinSliders\": [";
	for (size_t i = 0; i < m_SkinSliderValues.size(); ++i)
	{
		WriteNumber(out, m_SkinSliderValues[i]);
		out << (i + 1u < m_SkinSliderValues.size() ? ", " : "");
	}
	out << "],\n";
	out << "  \"eyeIrisSize\": "; WriteNumber(out, m_fEyeIrisSize); out << ",\n";
	out << "  \"eyeIrisAlpha\": "; WriteNumber(out, m_fEyeIrisAlpha); out << ",\n";
	out << "  \"eyeOdd\": " << (m_isEyeOddSelected ? "true" : "false") << ",\n";
	out << "  \"hairTwoTone\": " << (m_isHairTwoTone ? "true" : "false") << ",\n";
	out << "  \"hairTwoToneStrength\": "; WriteNumber(out, m_fHairTwoToneStrength); out << ",\n";
	out << "  \"hairTwoToneRange\": "; WriteNumber(out, m_fHairTwoToneRange); out << ",\n";
	out << "  \"hair\": " << m_iSelectedHair << ",\n";
	out << "  \"eyeIris\": " << m_iSelectedEyeIris << ",\n";
	out << "  \"costume\": " << m_iSelectedCostume << "\n";
	out << "}\n";

	std::ofstream file(path, std::ios::binary | std::ios::trunc);
	if (!file.is_open())
		return false;
	const std::string text = out.str();
	file.write(text.data(), static_cast<std::streamsize>(text.size()));
	if (!file.good())
		return false;
	file.close();
	m_SaveSlotFilled[static_cast<size_t>(iSlot)] = true;
	return true;
}

bool_t Client::CCustomizingView::Load_Slot(
	const shared_ptr<CCharacter>& pCharacter, const int32_t iSlot)
{
	if (nullptr == pCharacter || iSlot < 0 || iSlot >= SAVE_SLOT_COUNT)
		return false;
	const std::filesystem::path path = SaveSlotPath(m_strIconClassAssetId, iSlot);
	if (path.empty())
		return false;
	std::ifstream file(path, std::ios::binary);
	if (!file.is_open())
		return false;
	const std::string text((std::istreambuf_iterator<char_t>(file)),
		std::istreambuf_iterator<char_t>());
	DATA_JSON_VALUE root;
	std::string error;
	if (!CDataJson::Parse(text, root, error) || !root.Is_Object())
		return false;
	/* A slot written on another class names a slider set and a material this one does not
	have. Rather than apply the half that happens to overlap, the load is refused. */
	const DATA_JSON_VALUE* pClass = root.Find("class");
	if (nullptr == pClass || !pClass->Is_String() || pClass->Get_String() != m_strIconClassAssetId)
		return false;

	/* Back to the authored state first, so anything the slot does not mention is the class'
	own value rather than whatever the screen happened to be showing. */
	Reset_All(pCharacter);

	const DATA_JSON_VALUE* pSliders = root.Find("faceSliders");
	if (nullptr != pSliders && pSliders->Is_Object())
	{
		const CFaceCustomizeApplier& Face = pCharacter->Get_FaceCustomize();
		for (size_t i = 0; i < Face.Get_SliderCount(); ++i)
		{
			const DATA_JSON_VALUE* pWeight = pSliders->Find(Face.Get_SliderId(i));
			if (nullptr != pWeight && pWeight->Is_Number())
				pCharacter->Set_FaceSliderWeight(i, static_cast<f32_t>(pWeight->Get_Number()));
		}
	}
	const DATA_JSON_VALUE* pPreset = root.Find("facePreset");
	if (nullptr != pPreset && pPreset->Is_Number())
	{
		m_iSelectedFacePreset = static_cast<int32_t>(pPreset->Get_Number());
		Apply_FacePreset(pCharacter, m_iSelectedFacePreset);
	}

	const DATA_JSON_VALUE* pAdornStrength = root.Find("adornStrength");
	if (nullptr != pAdornStrength && pAdornStrength->Is_Array())
	{
		const auto& Values = pAdornStrength->Get_Array();
		for (size_t i = 0; i < m_AdornStrength.size() && i < Values.size(); ++i)
		{
			if (Values[i].Is_Number())
				m_AdornStrength[i] = static_cast<f32_t>(Values[i].Get_Number());
		}
	}
	m_isEyeOddSelected = nullptr != root.Find("eyeOdd") && root.Find("eyeOdd")->Is_Boolean() &&
		root.Find("eyeOdd")->Get_Boolean();
	{
		const f32_t fOdd = m_isEyeOddSelected ? 1.f : 0.f;
		pCharacter->Set_FaceMaterialParameter(EYE_ODD_COLOR_PARAMETER,
			float4_t(fOdd, fOdd, fOdd, fOdd));
	}
	m_fEyeIrisAlpha = ReadNumber(root.Find("eyeIrisAlpha"), m_fEyeIrisAlpha);

	/* Colours after the strengths and the odd-eye flag: a make-up colour carries its layer
	strength in alpha and an eye colour carries the clarity, and Apply_SurfaceColor packs
	both from the values just restored. */
	const DATA_JSON_VALUE* pColors = root.Find("surfaceColors");
	if (nullptr != pColors && pColors->Is_Array())
	{
		const auto& Rows = pColors->Get_Array();
		for (size_t i = 0; i < m_SurfaceColors.size() && i < Rows.size(); ++i)
		{
			float4_t vColor{};
			if (!ReadVector(&Rows[i], vColor) || vColor.w < 0.f)
				continue;
			m_SurfaceColors[i] = vColor;
			Apply_SurfaceColor(pCharacter, static_cast<int32_t>(i), vColor);
		}
	}

	const DATA_JSON_VALUE* pSkin = root.Find("skinSliders");
	if (nullptr != pSkin && pSkin->Is_Array())
	{
		const auto& Values = pSkin->Get_Array();
		for (size_t i = 0; i < m_SkinSliderValues.size() && i < Values.size(); ++i)
		{
			if (!Values[i].Is_Number())
				continue;
			const f32_t fValue = static_cast<f32_t>(Values[i].Get_Number());
			m_SkinSliderValues[i] = fValue;
			float4_t vAuthored{};
			if (fValue < 0.f ||
				!pCharacter->Try_Get_FaceMaterialParameter(SKIN_SLIDERS[i].pParameter, vAuthored))
			{
				continue;
			}
			pCharacter->Set_FaceMaterialParameter(SKIN_SLIDERS[i].pParameter,
				SKIN_SLIDERS[i].isColorAlpha ?
				float4_t(vAuthored.x, vAuthored.y, vAuthored.z, fValue) :
				float4_t(fValue, fValue, fValue, fValue));
		}
	}

	m_fEyeIrisSize = ReadNumber(root.Find("eyeIrisSize"), m_fEyeIrisSize);
	if (m_fEyeIrisSize >= 0.f)
	{
		pCharacter->Set_FaceMaterialParameter(EYE_IRIS_SIZE_PARAMETER,
			float4_t(m_fEyeIrisSize, m_fEyeIrisSize, m_fEyeIrisSize, m_fEyeIrisSize));
	}

	m_isHairTwoTone = nullptr != root.Find("hairTwoTone") &&
		root.Find("hairTwoTone")->Is_Boolean() && root.Find("hairTwoTone")->Get_Boolean();
	m_fHairTwoToneStrength = ReadNumber(root.Find("hairTwoToneStrength"), m_fHairTwoToneStrength);
	m_fHairTwoToneRange = ReadNumber(root.Find("hairTwoToneRange"), m_fHairTwoToneRange);
	pCharacter->Set_HairTwoTone(m_fHairTwoToneStrength, m_fHairTwoToneRange);

	/* The stamps and the meshes the lists pick are applied by the tab that owns them; the
	load states the choice and marks it changed so the owning Level re-dresses the model. */
	const DATA_JSON_VALUE* pAdornItems = root.Find("adornItems");
	if (nullptr != pAdornItems && pAdornItems->Is_Array())
	{
		const auto& Values = pAdornItems->Get_Array();
		for (size_t i = 0; i < m_SelectedAdornItems.size() && i < Values.size(); ++i)
		{
			if (Values[i].Is_Number())
				m_SelectedAdornItems[i] = static_cast<int32_t>(Values[i].Get_Number());
		}
	}
	m_iSelectedEyeIris = static_cast<int32_t>(ReadNumber(root.Find("eyeIris"), -1.f));
	const int32_t iHair = static_cast<int32_t>(ReadNumber(root.Find("hair"), 0.f));
	if (iHair != m_iSelectedHair)
	{
		m_iSelectedHair = iHair;
		m_bHairChanged = true;
	}
	const int32_t iCostume = static_cast<int32_t>(ReadNumber(root.Find("costume"), 0.f));
	if (iCostume != m_iSelectedCostume)
	{
		m_iSelectedCostume = iCostume;
		m_bCostumeChanged = true;
	}

	/* The stamps and the iris are textures, so they are re-applied here rather than waiting
	for the tab that owns the grid to be opened. A cell whose texture this client does not
	ship is skipped, the same as clicking it would be. */
	for (size_t iPage = 0; iPage < m_SelectedAdornItems.size(); ++iPage)
	{
		const int32_t iEntry = m_SelectedAdornItems[iPage];
		const ADORN_PAGE& Page = ADORN_PAGES[iPage];
		const auto* pStamps = nullptr != Page.pTextureKind ?
			m_FaceTextureDocument.Find(m_strIconClassAssetId, Page.pTextureKind) : nullptr;
		if (iEntry < 0 || nullptr == pStamps || static_cast<size_t>(iEntry) >= pStamps->size())
			continue;
		const std::string& strAsset = (*pStamps)[static_cast<size_t>(iEntry)];
		if (!strAsset.empty())
		{
			pCharacter->Set_FaceStampTexture(
				static_cast<CCharacter::FACE_STAMP>(Page.iStampRegister), strAsset);
		}
	}
	if (m_iSelectedEyeIris >= 0)
	{
		const auto* pIrisTextures = m_FaceTextureDocument.Find(m_strIconClassAssetId, "iris");
		if (nullptr != pIrisTextures &&
			static_cast<size_t>(m_iSelectedEyeIris) < pIrisTextures->size())
		{
			pCharacter->Set_FaceIrisTexture((*pIrisTextures)[static_cast<size_t>(m_iSelectedEyeIris)]);
		}
	}
	return true;
}

void Client::CCustomizingView::Update_ActionList(const shared_ptr<CCharacter>& pCharacter)
{
	if (nullptr == pCharacter)
		return;
	static_assert(std::size(CREATION_ACTION_CLIPS) == COSTUME_COUNT,
		"one clip per action cell");
	for (int32_t i = 0; i < COSTUME_COUNT; ++i)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!Get_SlotRect(("CC_LeftAction" + std::to_string(i)).c_str(),
			fX, fY, fWidth, fHeight))
		{
			continue;
		}
		if (!Is_Hovered(fX, fY, fWidth, fHeight) || !Is_Clicked(fX, fY, fWidth, fHeight))
			continue;
		CMainApp::Play_UIButtonClickSound();
		m_iSelectedAction = i;
		/* The pose cell goes back to what the screen stands in, which Set_Animation(IDLE)
		already resolves to the creation clip while the preview is up (see
		CCharacter::Set_Animation). */
		if (nullptr == CREATION_ACTION_CLIPS[i])
		{
			pCharacter->Set_Animation(CHARACTER_ANIM::IDLE, true);
			continue;
		}
		/* Start rather than Set: Set_Animation only moves the index, so a clip that already
		ran once resumes on its last frame and reads as nothing happening. Start_Animation
		rewinds it, which is also what a second click on the same cell should do. */
		const shared_ptr<Engine::CModel> pBody = pCharacter->Get_BodyModel();
		if (nullptr == pBody || !pBody->Start_Animation(CREATION_ACTION_CLIPS[i], false))
		{
			string carried;
			if (nullptr != pBody)
			{
				for (uint32_t index = 0u; index < pBody->Get_NumAnimations(); ++index)
				{
					const char_t* pName = pBody->Get_AnimationName(index);
					if (nullptr != pName && 0 == std::strncmp(pName, "sc_", 3))
						carried += string(" ") + pName;
				}
			}
			OutputDebugStringA(("[CustomizingAction] no clip " +
				string(CREATION_ACTION_CLIPS[i]) + "; body carries" +
				(carried.empty() ? string(" none") : carried) + "\n").c_str());
			m_iSelectedAction = 0;
		}
	}

	/* Back to the pose when the action runs out. The model keeps playing the last frame of a
	non-looping clip, so the end is read from its own progress rather than waited on. */
	if (0 == m_iSelectedAction)
		return;
	const shared_ptr<Engine::CModel> pModel = pCharacter->Get_BodyModel();
	if (nullptr == pModel)
		return;
	const uint32_t iAnimation = pModel->Get_CurrentAnimIndex();
	const char_t* pPlaying = pModel->Get_AnimationName(iAnimation);
	if (nullptr == pPlaying ||
		0 != std::strcmp(pPlaying, CREATION_ACTION_CLIPS[m_iSelectedAction]))
	{
		/* Something else took the model over -- a class change, a costume swap. */
		m_iSelectedAction = 0;
		return;
	}
	f32_t fPosition = 0.f, fDuration = 0.f;
	if (pModel->Get_AnimationProgress(iAnimation, fPosition, fDuration) &&
		fDuration > 0.f && fPosition >= fDuration)
	{
		m_iSelectedAction = 0;
		pCharacter->Set_Animation(CHARACTER_ANIM::IDLE, true);
	}
}

void Client::CCustomizingView::Reset_All(const shared_ptr<CCharacter>& pCharacter)
{
	if (nullptr == pCharacter)
		return;
	pCharacter->Reset_FaceSliders();
	m_iSelectedFacePreset = -1;
	Apply_FacePreset(pCharacter, -1);
	/* Dropping the overrides puts the retail head, skin and eye materials back on the values
	the catalog states, which is what every control seeds from. An unset dye colour (w < 0)
	does the same for the surfaces that are painted rather than parameterised. */
	pCharacter->Reset_FaceMaterial();
	static constexpr float4_t UNSET{ 0.f, 0.f, 0.f, -1.f };
	for (int32_t i = 0; i < static_cast<int32_t>(CCharacter::DYE_SURFACE::END); ++i)
		pCharacter->Set_DyeColor(static_cast<CCharacter::DYE_SURFACE>(i), UNSET, UNSET);
	m_SurfaceColors.fill(float4_t(0.f, 0.f, 0.f, -1.f));
	m_AdornStrength = { 1.f, 0.f, 1.f, 1.f, 1.f };
	m_fHairTwoToneStrength = 0.f;
	m_fHairTwoToneRange = 0.f;
	pCharacter->Set_HairTwoTone(m_fHairTwoToneStrength, m_fHairTwoToneRange);
	m_isHairTwoTone = false;
	m_isEyeOddSelected = false;
	m_iSelectedEyeIris = -1;
	m_iSelectedAdornSub = 0;
	m_iHairScrollRow = 0;
	m_iEyeIrisScrollRow = 0;
	m_iAdornScrollRow = 0;
	if (0 != m_iSelectedHair)
	{
		m_iSelectedHair = 0;
		m_bHairChanged = true;
	}
	if (0 != m_iSelectedCostume)
	{
		m_iSelectedCostume = 0;
		m_bCostumeChanged = true;
	}
	if (0 != m_iSelectedAction)
	{
		m_iSelectedAction = 0;
		pCharacter->Set_Animation(CHARACTER_ANIM::IDLE, true);
	}
	Seed_MaterialControls(pCharacter);
}

void Client::CCustomizingView::Seed_MaterialControls(
	const shared_ptr<CCharacter>& pCharacter)
{
	if (nullptr == pCharacter)
		return;
	/* Every face-material control starts where this class was authored: the make-up colours
	and their strengths are the retail values, so a slider or a swatch shows what is actually
	on the face rather than black at zero. A class whose face is not on a native head program
	leaves them unset, and those controls draw without moving anything. */
	m_SelectedAdornItems.fill(-1);
	m_SkinSliderValues.fill(-1.f);
	for (size_t iPage = 0; iPage < std::size(ADORN_PAGES); ++iPage)
	{
		const size_t iSurface = FIRST_MAKEUP_SURFACE_INDEX + iPage;
		float4_t vAuthored{};
		if (!pCharacter->Try_Get_FaceMaterialParameter(
			ADORN_PAGES[iPage].pColorParameter, vAuthored))
		{
			continue;
		}
		m_SurfaceColors[iSurface] = vAuthored;
		m_AdornStrength[iPage] = std::clamp(vAuthored.w, 0.f, 1.f);
	}
	{
		float4_t vShadow{};
		if (pCharacter->Try_Get_FaceMaterialParameter(EYESHADOW_COLOR_PARAMETER, vShadow))
		{
			m_SurfaceColors[EYESHADOW_SURFACE_INDEX] = vShadow;
			m_AdornStrength[m_AdornStrength.size() - 1u] = std::clamp(vShadow.w, 0.f, 1.f);
		}
	}
	{
		float4_t vSkin{};
		if (pCharacter->Try_Get_FaceMaterialParameter(SKIN_COLOR_PARAMETER, vSkin))
			m_SurfaceColors[SKIN_SURFACE_INDEX] = vSkin;
	}
	/* The eye tab reads the eye material, which is a different material from the head, so
	the odd-eye sub-tab starts wherever the class was authored rather than on both-eyes. */
	m_fEyeIrisSize = -1.f;
	m_fEyeIrisAlpha = -1.f;
	{
		float4_t vOdd{};
		m_isEyeOddSelected = pCharacter->Try_Get_FaceMaterialParameter(
			EYE_ODD_COLOR_PARAMETER, vOdd) && vOdd.x >= 0.5f;
	}
	{
		float4_t vEye{};
		if (pCharacter->Try_Get_FaceMaterialParameter(m_isEyeOddSelected ?
			EYE_BASE_COLOR_LEFT_PARAMETER : EYE_BASE_COLOR_PARAMETER, vEye))
		{
			m_SurfaceColors[EYE_BASE_SURFACE_INDEX] = float4_t(vEye.x, vEye.y, vEye.z, 1.f);
		}
		if (pCharacter->Try_Get_FaceMaterialParameter(m_isEyeOddSelected ?
			EYE_IRIS_COLOR_LEFT_PARAMETER : EYE_IRIS_COLOR_PARAMETER, vEye))
		{
			m_SurfaceColors[EYE_IRIS_SURFACE_INDEX] = float4_t(vEye.x, vEye.y, vEye.z, 1.f);
			m_fEyeIrisAlpha = std::clamp(vEye.w, 0.f, 1.f);
		}
	}
}

void Client::CCustomizingView::Apply_ListIcons(const shared_ptr<CCharacter>& pCharacter)
{
	if (nullptr == pCharacter || nullptr == pCharacter->Get_Spec())
		return;
	const char_t* pAssetName = pCharacter->Get_Spec()->pAssetName;
	if (nullptr == pAssetName || m_strIconClassAssetId == pAssetName)
		return;
	m_strIconClassAssetId = pAssetName;

	if (!m_IconDocument.Load())
	{
		OutputDebugStringA(
			("[CustomizingView][Icons] " + m_IconDocument.Get_Status() + "\n").c_str());
		return;
	}
	/* A missing preset document leaves the grid drawn and clickable but with nothing to
	apply, the same way a class with no face morph data behaves. */
	if (!m_FacePresetDocument.Load())
	{
		OutputDebugStringA(
			("[CustomizingView][FacePresets] " + m_FacePresetDocument.Get_Status() + "\n").c_str());
	}
	if (!m_FaceTextureDocument.Load())
	{
		OutputDebugStringA(
			("[CustomizingView][FaceTextures] " + m_FaceTextureDocument.Get_Status() + "\n").c_str());
	}
	m_iSelectedFacePreset = -1;
	m_iSelectedEyeIris = -1;
	Seed_MaterialControls(pCharacter);
	Refresh_SaveSlots();
	const auto* pIcons = m_IconDocument.Find(m_strIconClassAssetId);
	if (nullptr == pIcons)
		return;

	/* A cell past the end of its list keeps the authored frame: the retail lists are
	longer than the visible rows and scroll, which this screen does not do yet. */
	const auto Fn_Fill = [&](const char_t* pPrefix, int32_t iCount,
		const std::vector<std::string>& Assets)
	{
		for (int32_t i = 0; i < iCount; ++i)
		{
			const string strId = string(pPrefix) + std::to_string(i) + "_Plate";
			m_pView->Set_SlotTexture(strId,
				static_cast<size_t>(i) < Assets.size() ? Assets[i] : string());
		}
	};
	Fn_Fill("CC_LeftDress", 5, pIcons->Costumes);
	Fn_Fill("CC_LeftAction", 5, pIcons->Actions);
	/* Hair and iris are longer than their grids, so they are filled from their scroll window
	every frame rather than once per class here. */
	/* The adorn list is one grid the sub-tab re-fills, so it is filled per frame from the
	page in view rather than once here. */
	/* The face tab's grid is 기본 얼굴 -- the face-shape set -- not the base tab's whole
	appearance presets. The two have the same row count, so the wrong one looked plausible. */
	Fn_Fill("CC_FacePreset", FACE_PRESET_COUNT, pIcons->FaceShapes);
	/* The recommended-style row stays empty: it is account content the client fetches,
	not a table, and the icon package that looked like it is the background picker. */
}

void Client::CCustomizingView::Update_SubjectMetrics(const shared_ptr<CCharacter>& pCharacter)
{
	if (nullptr == pCharacter)
		return;
	const shared_ptr<Engine::CModel> pModel = pCharacter->Get_BodyModel();
	if (nullptr == pModel)
		return;

	f32_t fSum = 0.f;
	int32_t iFound = 0;
	for (const char_t* pBoneName : EYE_BONE_NAMES)
	{
		if (!pModel->Has_Bone(pBoneName))
			continue;
		float3_t vBonePosition{};
		XMStoreFloat3(&vBonePosition, pModel->Get_BoneMatrix(pBoneName).r[3]);
		if (!std::isfinite(vBonePosition.y) || vBonePosition.y <= 0.f)
			continue;
		fSum += vBonePosition.y;
		++iFound;
	}
	if (0 == iFound)
		return;
	m_fMeasuredEyeHeight = fSum / static_cast<f32_t>(iFound);
}

void Client::CCustomizingView::Update_FaceCategoryHeader(
	const char_t* pSlotId, bool_t& isExpanded, bool_t& isOther, const bool_t isVisible)
{
	if (!isVisible)
		return;
	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (!Get_SlotRect(pSlotId, fX, fY, fWidth, fHeight))
		return;
	if (!Is_Hovered(fX, fY, fWidth, fHeight) || !Is_Clicked(fX, fY, fWidth, fHeight))
		return;
	CMainApp::Play_UIButtonClickSound();
	isExpanded = !isExpanded;
	/* The two categories are one accordion, not two independent folds: opening either closes
	the other, so the section below always sits directly under the collapsed header above it. */
	if (isExpanded)
		isOther = false;
	m_iDraggingRow = -1;
}

void Client::CCustomizingView::Capture_FaceDetailLayout()
{
	if (!m_FaceDetailAuthoredRects.empty() || nullptr == m_pView)
		return;
	std::vector<string> ids = { "CC_FoldDetail_Bg", "CC_FoldDetail_Arrow", "CC_DetailDivision" };
	for (int32_t i = 0; i < FACE_PART_COUNT; ++i)
	{
		ids.push_back("CC_FaceSub" + std::to_string(i) + "_Bg");
		ids.push_back("CC_FaceSub" + std::to_string(i) + "_Selected");
	}
	for (const FACE_SLIDER_ROW& Row : FACE_SLIDER_ROWS)
	{
		ids.push_back(Slider_TrackSlotId(Row.pSliderId));
		ids.push_back(Slider_ThumbSlotId(Row.pSliderId));
	}
	for (const string& id : ids)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (m_pView->Get_SlotRect(id, fX, fY, fWidth, fHeight))
			m_FaceDetailAuthoredRects.emplace_back(id, float2_t(fX, fY));
	}
}

void Client::CCustomizingView::Apply_FaceAccordionLayout()
{
	Capture_FaceDetailLayout();
	const f32_t fOffset =
		m_isFaceDefaultExpanded ? FACE_DEFAULT_SECTION_HEIGHT : 0.f;
	for (const auto& [id, vAuthored] : m_FaceDetailAuthoredRects)
		m_pView->Set_SlotPosition(id, vAuthored.x, vAuthored.y + fOffset);
}

bool_t Client::CCustomizingView::Consume_TabGridScroll(const int32_t iWheel)
{
	if (0 == iWheel)
		return false;
	const auto* pIcons = m_IconDocument.Find(m_strIconClassAssetId);
	if (nullptr == pIcons)
		return false;

	if (HAIR_TAB_INDEX == m_iSelectedTab)
	{
		return Consume_GridScroll("CC_HairShape", HAIR_SHAPE_COUNT,
			static_cast<int32_t>(pIcons->HairShapes.size()), m_iHairScrollRow, iWheel);
	}
	if (EYE_TAB_INDEX == m_iSelectedTab)
	{
		return Consume_GridScroll("CC_EyeIris", EYE_IRIS_COUNT,
			static_cast<int32_t>(pIcons->EyeIrises.size()), m_iEyeIrisScrollRow, iWheel);
	}
	if (ADORN_TAB_INDEX == m_iSelectedTab)
	{
		return Consume_GridScroll("CC_AdornItem", ADORN_ITEM_COUNT,
			static_cast<int32_t>(Get_AdornPage().size()), m_iAdornScrollRow, iWheel);
	}
	return false;
}

const std::vector<std::string>& Client::CCustomizingView::Get_AdornPage() const
{
	static const std::vector<std::string> s_Empty;
	const auto* pIcons = m_IconDocument.Find(m_strIconClassAssetId);
	if (nullptr == pIcons)
		return s_Empty;
	/* Lip, cheek touch and eye make each have their own table category; the eyebrow page is a
	slider and a colour chip only, so its grid stays empty. */
	switch (m_iSelectedAdornSub)
	{
	case 0: return pIcons->AdornLip;
	case 1: return pIcons->AdornTouch;
	case 2: return pIcons->AdornEyeLine;
	default: return s_Empty;
	}
}

bool_t Client::CCustomizingView::Consume_GridScroll(
	const char_t* pPrefix, const int32_t iCellCount, const int32_t iEntryCount,
	int32_t& iScrollRow, const int32_t iWheel)
{
	if (0 == iWheel || iCellCount <= 0)
		return false;
	/* The grid's own bounds are the union of its cells, so the pointer test needs no separate
	authored rect. */
	f32_t fLeft = 0.f, fTop = 0.f, fRight = 0.f, fBottom = 0.f;
	bool_t bHasBounds = false;
	for (int32_t i = 0; i < iCellCount; ++i)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!Get_SlotRect((string(pPrefix) + std::to_string(i)).c_str(),
			fX, fY, fWidth, fHeight))
		{
			continue;
		}
		if (!bHasBounds)
		{
			fLeft = fX; fTop = fY; fRight = fX + fWidth; fBottom = fY + fHeight;
			bHasBounds = true;
			continue;
		}
		fLeft = (min)(fLeft, fX);
		fTop = (min)(fTop, fY);
		fRight = (max)(fRight, fX + fWidth);
		fBottom = (max)(fBottom, fY + fHeight);
	}
	if (!bHasBounds || !Is_Hovered(fLeft, fTop, fRight - fLeft, fBottom - fTop))
		return false;

	const int32_t iBefore = iScrollRow;
	iScrollRow = Clamp_ScrollRow(
		iScrollRow + (iWheel > 0 ? -1 : 1), iEntryCount, iCellCount);
	/* The notch is consumed even at the end of the list: the pointer was over the grid, so the
	camera must not zoom instead. */
	(void)iBefore;
	return true;
}

int32_t Client::CCustomizingView::Clamp_ScrollRow(
	int32_t iScrollRow, const int32_t iEntryCount, const int32_t iCellCount) const
{
	const int32_t iRows = (iEntryCount + GRID_COLUMNS - 1) / GRID_COLUMNS;
	const int32_t iVisibleRows = iCellCount / GRID_COLUMNS;
	const int32_t iMaximum = (max)(0, iRows - iVisibleRows);
	return std::clamp(iScrollRow, 0, iMaximum);
}

void Client::CCustomizingView::Fill_ScrollingGrid(
	const char_t* pPrefix, const int32_t iCellCount, const int32_t iScrollRow,
	const std::vector<std::string>& Entries)
{
	const int32_t iFirst = iScrollRow * GRID_COLUMNS;
	for (int32_t i = 0; i < iCellCount; ++i)
	{
		const size_t iEntry = static_cast<size_t>(iFirst + i);
		m_pView->Set_SlotTexture(string(pPrefix) + std::to_string(i) + "_Plate",
			iEntry < Entries.size() ? Entries[iEntry] : string());
	}
}

float4_t Client::CCustomizingView::HsvToRgb(
	const f32_t fHue, const f32_t fSaturation, const f32_t fValue)
{
	const f32_t fSector = std::fmod(std::fmax(fHue, 0.f), 360.f) / 60.f;
	const int32_t iSector = static_cast<int32_t>(fSector) % 6;
	const f32_t f = fSector - std::floor(fSector);
	const f32_t p = fValue * (1.f - fSaturation);
	const f32_t q = fValue * (1.f - fSaturation * f);
	const f32_t t = fValue * (1.f - fSaturation * (1.f - f));
	switch (iSector)
	{
	case 0: return float4_t(fValue, t, p, 1.f);
	case 1: return float4_t(q, fValue, p, 1.f);
	case 2: return float4_t(p, fValue, t, 1.f);
	case 3: return float4_t(p, q, fValue, 1.f);
	case 4: return float4_t(t, p, fValue, 1.f);
	default: return float4_t(fValue, p, q, 1.f);
	}
}

bool_t Client::CCustomizingView::Apply_SurfaceColor(
	const shared_ptr<CCharacter>& pCharacter, const int32_t iSurface,
	const float4_t& vColor) const
{
	if (nullptr == pCharacter || iSurface < 0)
		return false;
	if (iSurface < FIRST_MAKEUP_SURFACE_INDEX)
	{
		/* Skin is a parameter of the retail head material, not a dye. A class whose face is
		not on that program still takes the plain tint it took before. */
		if (SKIN_SURFACE_INDEX == iSurface &&
			pCharacter->Set_FaceMaterialParameter(SKIN_COLOR_PARAMETER, vColor))
		{
			return true;
		}
		return pCharacter->Set_DyeColor(
			static_cast<CCharacter::DYE_SURFACE>(iSurface), vColor, vColor);
	}
	if (EYE_BASE_SURFACE_INDEX == iSurface || EYE_IRIS_SURFACE_INDEX == iSurface)
	{
		/* The iris colour's alpha is not a swatch value: the eye program uses it as the
		weight of the iris over the base, which is the clarity slider, so the wheel moves
		only the hue and leaves the slider where it was set. */
		const bool_t isIris = EYE_IRIS_SURFACE_INDEX == iSurface;
		const char_t* pEyeParameter = m_isEyeOddSelected ?
			(isIris ? EYE_IRIS_COLOR_LEFT_PARAMETER : EYE_BASE_COLOR_LEFT_PARAMETER) :
			(isIris ? EYE_IRIS_COLOR_PARAMETER : EYE_BASE_COLOR_PARAMETER);
		const f32_t fAlpha = (isIris && m_fEyeIrisAlpha >= 0.f) ? m_fEyeIrisAlpha : 1.f;
		return pCharacter->Set_FaceMaterialParameter(pEyeParameter,
			float4_t(vColor.x, vColor.y, vColor.z, fAlpha));
	}
	const int32_t iPage = iSurface - FIRST_MAKEUP_SURFACE_INDEX;
	const char_t* pParameter = EYESHADOW_SURFACE_INDEX == iSurface ?
		EYESHADOW_COLOR_PARAMETER : ADORN_PAGES[iPage].pColorParameter;
	/* Alpha carries the layer strength, so the wheel moves only the hue and the page's
	strength slider keeps whatever it was set to. */
	const size_t iStrength = EYESHADOW_SURFACE_INDEX == iSurface ?
		m_AdornStrength.size() - 1u : static_cast<size_t>(iPage);
	return pCharacter->Set_FaceMaterialParameter(pParameter,
		float4_t(vColor.x, vColor.y, vColor.z, m_AdornStrength[iStrength]));
}

bool_t Client::CCustomizingView::Update_ColorPicker(
	const shared_ptr<CCharacter>& pCharacter)
{
	const bool_t bOpen = PICKER_SURFACE_NONE != m_iPickerSurface;
	/* Skin does not pick a free hue in the source game: its colour model is a named ramp
	(SkinColor / GN_F_SkinColor / SkinColor_USA) rather than the HSV wheel hair and eyes use.
	That ramp's own colours are not in anything extracted so far -- EFTable_CharacterCustomizing
	carries indices, not values -- so until they are, skin opens the same wheel as every other
	surface. Hiding the wheel and leaving the value bar alone, which is what this did before,
	showed an all but empty panel. */
	static constexpr const char_t* PICKER_SLOTS[] = {
		"CC_PickerBg", "CC_PickerBar", "CC_PickerWheel", "CC_PickerCursor",
		"CC_PickerBarThumb", "CC_PickerPreview", "CC_PickerApply", "CC_PickerCancel" };
	for (const char_t* pSlotId : PICKER_SLOTS)
		m_pView->Set_SlotVisible(pSlotId, bOpen);
	if (!bOpen)
	{
		m_isPickerWheelDragging = false;
		m_isPickerBarDragging = false;
		return false;
	}

	CUIInputRouter& Router = CUIInputRouter::Get();
	f32_t fMouseX = 0.f, fMouseY = 0.f;
	const bool_t bHasMouse = Router.Get_MousePosition(REF_WIDTH, REF_HEIGHT, fMouseX, fMouseY);
	if (!Router.Is_LeftDown())
	{
		m_isPickerWheelDragging = false;
		m_isPickerBarDragging = false;
	}

	f32_t fWheelX = 0.f, fWheelY = 0.f, fWheelW = 0.f, fWheelH = 0.f;
	if (Get_SlotRect("CC_PickerWheel", fWheelX, fWheelY, fWheelW, fWheelH) && fWheelW > 0.f)
	{
		const f32_t fRadius = fWheelW * 0.5f;
		const f32_t fCentreX = fWheelX + fRadius;
		const f32_t fCentreY = fWheelY + fWheelH * 0.5f;
		const f32_t dx = fMouseX - fCentreX;
		const f32_t dy = fMouseY - fCentreY;
		const f32_t fDistance = std::sqrt(dx * dx + dy * dy);
		if (bHasMouse && Router.Is_LeftClickEdge() && fDistance <= fRadius)
			m_isPickerWheelDragging = true;
		if (m_isPickerWheelDragging && bHasMouse)
		{
			/* The wheel is painted clockwise in screen terms: sampling the art gives hue 0
			at screen angle 0, 240 at 120 and 120 at 240. Screen y already grows downward,
			so atan2 on it reads that direction with no sign flip. */
			m_fPickerHue = XMConvertToDegrees(std::atan2(dy, dx));
			if (m_fPickerHue < 0.f)
				m_fPickerHue += 360.f;
			m_fPickerSaturation = std::clamp(fDistance / fRadius, 0.f, 1.f);
		}
		f32_t fCursorW = 0.f, fCursorH = 0.f, fIgnoreX = 0.f, fIgnoreY = 0.f;
		if (Get_SlotRect("CC_PickerCursor", fIgnoreX, fIgnoreY, fCursorW, fCursorH))
		{
			const f32_t fAngle = XMConvertToRadians(m_fPickerHue);
			m_pView->Set_SlotPosition("CC_PickerCursor",
				fCentreX + std::cos(fAngle) * m_fPickerSaturation * fRadius - fCursorW * 0.5f,
				fCentreY + std::sin(fAngle) * m_fPickerSaturation * fRadius - fCursorH * 0.5f);
		}
	}

	f32_t fBarX = 0.f, fBarY = 0.f, fBarW = 0.f, fBarH = 0.f;
	if (Get_SlotRect("CC_PickerBar", fBarX, fBarY, fBarW, fBarH) && fBarW > 0.f)
	{
		if (bHasMouse && Router.Is_LeftClickEdge() &&
			Is_Hovered(fBarX, fBarY - 4.f, fBarW, fBarH + 8.f))
		{
			m_isPickerBarDragging = true;
		}
		if (m_isPickerBarDragging && bHasMouse)
			m_fPickerValue = std::clamp((fMouseX - fBarX) / fBarW, 0.f, 1.f);
		f32_t fThumbW = 0.f, fThumbH = 0.f, fIgnoreX = 0.f, fIgnoreY = 0.f;
		if (Get_SlotRect("CC_PickerBarThumb", fIgnoreX, fIgnoreY, fThumbW, fThumbH))
		{
			m_pView->Set_SlotPosition("CC_PickerBarThumb",
				fBarX + m_fPickerValue * fBarW - fThumbW * 0.5f, fBarY - 1.f);
		}
	}

	const float4_t vChosen = HsvToRgb(m_fPickerHue, m_fPickerSaturation, m_fPickerValue);
	m_pView->Set_SlotTint("CC_PickerPreview", vChosen);
	/* Live: a colour only means anything on the character, so it goes on as the wheel moves
	and Apply just closes the picker rather than being the moment it lands. */
	if (nullptr != pCharacter)
	{
		const bool_t bApplied = Apply_SurfaceColor(pCharacter, m_iPickerSurface, vChosen);
		if (bApplied != m_bLastDyeApplied)
		{
			m_bLastDyeApplied = bApplied;
			OutputDebugStringA(("[Dye] surface " + std::to_string(m_iPickerSurface) +
				(bApplied ? " applied" : " matched nothing") + "\n").c_str());
		}
	}

	const auto Fn_PickerClicked = [&](const char_t* pSlotId)
	{
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		return Get_SlotRect(pSlotId, fX, fY, fW, fH) &&
			Router.Is_Clicked(fX, fY, fW, fH, REF_WIDTH, REF_HEIGHT);
	};
	if (Fn_PickerClicked("CC_PickerApply"))
	{
		CMainApp::Play_UIButtonClickSound();
		if (nullptr != pCharacter)
		{
			/* The second hair colour follows the first until its own picker exists; a
			hairstyle with no second colour ignores it either way. */
			Apply_SurfaceColor(pCharacter, m_iPickerSurface, vChosen);
		}
		m_SurfaceColors[static_cast<size_t>(m_iPickerSurface)] = vChosen;
		m_iPickerSurface = PICKER_SURFACE_NONE;
		return true;
	}
	if (Fn_PickerClicked("CC_PickerCancel"))
	{
		CMainApp::Play_UIButtonClickSound();
		if (nullptr != pCharacter)
		{
			Apply_SurfaceColor(pCharacter, m_iPickerSurface, m_vPickerRestore);
		}
		m_iPickerSurface = PICKER_SURFACE_NONE;
		return true;
	}
	/* While it is up it owns the panel, so nothing behind it reads the pointer. */
	return true;
}

void Client::CCustomizingView::Update_SecondaryTabs(const shared_ptr<CCharacter>& pCharacter)
{
	/* Chrome for the hair, eye, skin and adorn tabs, placed from charactercustomizing.gfx. The
	lists show the icons their table category already carries; the sliders and colour chips are
	drawn and hit-tested but change nothing yet, because hair meshes, iris/lip/decal textures
	and skin material tints are each their own slice with no runtime contract. Nothing here
	invents a value -- an unbound control simply does not move. */
	const auto Fn_ShowList = [&](const char_t* pPrefix, int32_t iCount, bool_t bVisible)
	{
		for (int32_t i = 0; i < iCount; ++i)
		{
			const string strId = string(pPrefix) + std::to_string(i);
			m_pView->Set_SlotVisible(strId, bVisible);
			m_pView->Set_SlotVisible(strId + "_Plate", bVisible);
		}
	};
	CUIInputRouter& Router = CUIInputRouter::Get();
	f32_t fMouseX = 0.f, fMouseY = 0.f;
	const bool_t bHasMouse = Router.Get_MousePosition(REF_WIDTH, REF_HEIGHT, fMouseX, fMouseY);
	if (!Router.Is_LeftDown())
		m_strDraggingSlider.clear();
	/* Shows the track and thumb, and while it is up lets the pointer move it. Returns the
	value so the caller can push it wherever it belongs; the same grab-and-follow the face
	tab's sliders use, keyed by slot id because these do not sit in a row table. */
	const auto Fn_ShowSlider = [&](const char_t* pId, bool_t bVisible, f32_t* pValue = nullptr)
	{
		const string strTrackId = string(pId) + "_Track";
		const string strThumbId = string(pId) + "_Thumb";
		m_pView->Set_SlotVisible(strTrackId, bVisible);
		m_pView->Set_SlotVisible(strThumbId, bVisible);
		if (!bVisible || nullptr == pValue)
			return false;
		f32_t fTrackX = 0.f, fTrackY = 0.f, fTrackWidth = 0.f, fTrackHeight = 0.f;
		f32_t fThumbX = 0.f, fThumbY = 0.f, fThumbWidth = 0.f, fThumbHeight = 0.f;
		if (!Get_SlotRect(strTrackId.c_str(), fTrackX, fTrackY, fTrackWidth, fTrackHeight) ||
			!Get_SlotRect(strThumbId.c_str(), fThumbX, fThumbY, fThumbWidth, fThumbHeight) ||
			fTrackWidth <= 0.f)
		{
			return false;
		}
		const bool_t bHovered = Is_Hovered(
			fTrackX, fThumbY - 4.f, fTrackWidth, fThumbHeight + 8.f);
		if (bHovered && Router.Is_LeftClickEdge())
			m_strDraggingSlider = strTrackId;
		bool_t bMoved = false;
		if (m_strDraggingSlider == strTrackId && bHasMouse)
		{
			const f32_t fNext = std::clamp((fMouseX - fTrackX) / fTrackWidth, 0.f, 1.f);
			bMoved = fNext != *pValue;
			*pValue = fNext;
		}
		m_pView->Set_SlotTexture(strThumbId,
			(bHovered || m_strDraggingSlider == strTrackId) ?
			ASSET_THUMB_OVER : ASSET_THUMB_NORMAL);
		m_pView->Set_SlotPosition(strThumbId,
			fTrackX + *pValue * fTrackWidth - fThumbWidth * 0.5f, fThumbY);
		return bMoved;
	};
	/* A swatch shows what the surface is wearing and opens the picker on it. Until a colour
	is chosen it shows the art's own tint, which is what the asset was authored in. */
	const auto Fn_ShowPicker = [&](const char_t* pId, bool_t bVisible, int32_t iSurface = -1)
	{
		const string strChipId = string(pId) + "_Chip";
		m_pView->Set_SlotVisible(pId, bVisible);
		m_pView->Set_SlotVisible(strChipId, bVisible);
		if (!bVisible || iSurface < 0)
			return;
		const float4_t& vWorn = m_SurfaceColors[static_cast<size_t>(iSurface)];
		if (vWorn.w > 0.f)
			m_pView->Set_SlotTint(strChipId, vWorn);
		f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
		if (Get_SlotRect(pId, fX, fY, fW, fH) &&
			Is_Hovered(fX, fY, fW, fH) && Is_Clicked(fX, fY, fW, fH))
		{
			CMainApp::Play_UIButtonClickSound();
			m_iPickerSurface = iSurface;
			m_vPickerRestore = vWorn;
		}
	};
	const auto Fn_ShowSubTab = [&](const char_t* pId, bool_t bVisible, bool_t bSelected)
	{
		m_pView->Set_SlotVisible(string(pId) + "_Bg", bVisible);
		m_pView->Set_SlotVisible(string(pId) + "_Selected", bVisible && bSelected);
	};

	const bool_t bHair = HAIR_TAB_INDEX == m_iSelectedTab;
	const bool_t bEye = EYE_TAB_INDEX == m_iSelectedTab;
	const bool_t bSkin = SKIN_TAB_INDEX == m_iSelectedTab;
	const bool_t bAdorn = ADORN_TAB_INDEX == m_iSelectedTab;

	m_pView->Set_SlotVisible("CC_HairDivision", bHair);
	Fn_ShowList("CC_HairShape", HAIR_SHAPE_COUNT, bHair);
	if (bHair)
	{
		/* A class carries 38-53 hairstyles against a 20-cell grid, so the grid is a window and
		a cell's list index is its position plus the scrolled rows. */
		static const std::vector<std::string> s_Empty;
		const auto* pIcons = m_IconDocument.Find(m_strIconClassAssetId);
		const std::vector<std::string>& Shapes =
			nullptr != pIcons ? pIcons->HairShapes : s_Empty;
		const int32_t iCount = static_cast<int32_t>(Shapes.size());
		m_iHairScrollRow = Clamp_ScrollRow(m_iHairScrollRow, iCount, HAIR_SHAPE_COUNT);
		Fill_ScrollingGrid("CC_HairShape", HAIR_SHAPE_COUNT, m_iHairScrollRow, Shapes);

		for (int32_t i = 0; i < HAIR_SHAPE_COUNT; ++i)
		{
			const int32_t iEntry = m_iHairScrollRow * GRID_COLUMNS + i;
			if (iEntry >= iCount)
				break;
			f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
			if (!Get_SlotRect(("CC_HairShape" + std::to_string(i)).c_str(),
				fX, fY, fWidth, fHeight))
			{
				continue;
			}
			if (Is_Hovered(fX, fY, fWidth, fHeight) && Is_Clicked(fX, fY, fWidth, fHeight))
			{
				CMainApp::Play_UIButtonClickSound();
				m_iSelectedHair = iEntry;
				m_bHairChanged = true;
			}
		}
	}
	Fn_ShowSubTab("CC_HairSubBasic", bHair, !m_isHairTwoTone);
	Fn_ShowSubTab("CC_HairSubTwoTone", bHair, m_isHairTwoTone);
	Fn_ShowPicker("CC_HairColor", bHair, 0);
	{
		const bool_t bStrength =
			Fn_ShowSlider("CC_Slider_hair_strength", bHair, &m_fHairTwoToneStrength);
		const bool_t bRange =
			Fn_ShowSlider("CC_Slider_hair_range", bHair, &m_fHairTwoToneRange);
		if ((bStrength || bRange) && nullptr != pCharacter)
		{
			pCharacter->Set_HairTwoTone(m_fHairTwoToneStrength, m_fHairTwoToneRange);
		}
	}

	m_pView->Set_SlotVisible("CC_EyeDivision", bEye);
	Fn_ShowSubTab("CC_EyeSubIris", bEye, !m_isEyeOddSelected);
	Fn_ShowSubTab("CC_EyeSubOdd", bEye, m_isEyeOddSelected);
	/* Pupil size: var_eye_irissize_ui, which the program reads against iris_size_center to
	grow or shrink the iris disc. */
	{
		float4_t vAuthored{};
		const bool_t bBound = bEye && nullptr != pCharacter &&
			pCharacter->Try_Get_FaceMaterialParameter(EYE_IRIS_SIZE_PARAMETER, vAuthored);
		if (bBound && m_fEyeIrisSize < 0.f)
			m_fEyeIrisSize = std::clamp(vAuthored.x, 0.f, 1.f);
		f32_t fSize = m_fEyeIrisSize < 0.f ? 0.f : m_fEyeIrisSize;
		if (Fn_ShowSlider("CC_Slider_eye_scale", bEye, bBound ? &fSize : nullptr) && bBound)
		{
			m_fEyeIrisSize = fSize;
			pCharacter->Set_FaceMaterialParameter(EYE_IRIS_SIZE_PARAMETER,
				float4_t(fSize, fSize, fSize, fSize));
		}
	}
	/* A class whose eye submesh is not on the retail eye program has no eye variables to
	write, so its swatches draw without opening the wheel -- the same honest state every
	other unbound control here is in. */
	const bool_t bEyeMaterial = bEye && m_fEyeIrisAlpha >= 0.f;
	Fn_ShowPicker("CC_EyeColor", bEye, bEyeMaterial ? EYE_BASE_SURFACE_INDEX : -1);
	Fn_ShowPicker("CC_EyeIrisColor", bEye, bEyeMaterial ? EYE_IRIS_SURFACE_INDEX : -1);
	{
		f32_t fAlpha = m_fEyeIrisAlpha < 0.f ? 0.f : m_fEyeIrisAlpha;
		const bool_t bBound = bEye && nullptr != pCharacter && m_fEyeIrisAlpha >= 0.f;
		if (Fn_ShowSlider("CC_Slider_eye_definition", bEye, bBound ? &fAlpha : nullptr) &&
			bBound)
		{
			m_fEyeIrisAlpha = fAlpha;
			const float4_t& vIris = m_SurfaceColors[EYE_IRIS_SURFACE_INDEX];
			pCharacter->Set_FaceMaterialParameter(m_isEyeOddSelected ?
				EYE_IRIS_COLOR_LEFT_PARAMETER : EYE_IRIS_COLOR_PARAMETER,
				float4_t(vIris.x, vIris.y, vIris.z, fAlpha));
		}
	}
	Fn_ShowList("CC_EyeIris", EYE_IRIS_COUNT, bEye);
	if (bEye)
	{
		static const std::vector<std::string> s_Empty;
		const auto* pIcons = m_IconDocument.Find(m_strIconClassAssetId);
		const std::vector<std::string>& Irises =
			nullptr != pIcons ? pIcons->EyeIrises : s_Empty;
		m_iEyeIrisScrollRow = Clamp_ScrollRow(m_iEyeIrisScrollRow,
			static_cast<int32_t>(Irises.size()), EYE_IRIS_COUNT);
		Fill_ScrollingGrid("CC_EyeIris", EYE_IRIS_COUNT, m_iEyeIrisScrollRow, Irises);

		/* The icon list and the cooked texture list are the same retail list, so a cell's
		icon index is its texture index. A cell whose texture the client does not ship stays
		clickable but changes nothing, rather than putting a neighbour's iris on the eye. */
		const auto* pIrisTextures =
			m_FaceTextureDocument.Find(m_strIconClassAssetId, "iris");
		const int32_t iIrisIconCount = static_cast<int32_t>(Irises.size());
		for (int32_t i = 0; i < EYE_IRIS_COUNT; ++i)
		{
			const int32_t iEntry = m_iEyeIrisScrollRow * GRID_COLUMNS + i;
			if (iEntry >= iIrisIconCount)
				break;
			f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
			if (!Get_SlotRect(("CC_EyeIris" + std::to_string(i)).c_str(),
				fX, fY, fWidth, fHeight))
			{
				continue;
			}
			if (!Is_Hovered(fX, fY, fWidth, fHeight) || !Is_Clicked(fX, fY, fWidth, fHeight))
				continue;

			CMainApp::Play_UIButtonClickSound();
			m_iSelectedEyeIris = iEntry;
			if (nullptr == pCharacter || nullptr == pIrisTextures)
			{
				OutputDebugStringA(nullptr == pIrisTextures ?
					"[FaceIris] no iris list for this class\n" :
					"[FaceIris] no character\n");
			}
			else if (static_cast<size_t>(iEntry) >= pIrisTextures->size() ||
				(*pIrisTextures)[iEntry].empty())
			{
				OutputDebugStringA(("[FaceIris] cell " + std::to_string(iEntry) +
					" has no cooked texture (list has " +
					std::to_string(pIrisTextures->size()) + ")\n").c_str());
			}
			else
			{
				pCharacter->Set_FaceIrisTexture((*pIrisTextures)[iEntry]);
			}
		}
	}

	m_pView->Set_SlotVisible("CC_SkinDivision", bSkin);
	Fn_ShowPicker("CC_SkinColor", bSkin, 2);
	for (size_t i = 0; i < std::size(SKIN_SLIDERS); ++i)
	{
		const SKIN_SLIDER& Slider = SKIN_SLIDERS[i];
		float4_t vAuthored{};
		const bool_t bBound = bSkin && nullptr != pCharacter &&
			pCharacter->Try_Get_FaceMaterialParameter(Slider.pParameter, vAuthored);
		if (bBound && m_SkinSliderValues[i] < 0.f)
		{
			m_SkinSliderValues[i] = std::clamp(
				Slider.isColorAlpha ? vAuthored.w : vAuthored.x, 0.f, 1.f);
		}
		f32_t fValue = m_SkinSliderValues[i] < 0.f ? 0.f : m_SkinSliderValues[i];
		if (!Fn_ShowSlider(Slider.pSlotId, bSkin, bBound ? &fValue : nullptr) || !bBound)
			continue;
		m_SkinSliderValues[i] = fValue;
		/* The freckle slider is one channel of a colour, so the authored hue stays put. */
		pCharacter->Set_FaceMaterialParameter(Slider.pParameter, Slider.isColorAlpha ?
			float4_t(vAuthored.x, vAuthored.y, vAuthored.z, fValue) :
			float4_t(fValue, fValue, fValue, fValue));
	}

	m_pView->Set_SlotVisible("CC_AdornDivision", bAdorn);
	static constexpr const char_t* ADORN_SUB_IDS[] = {
		"CC_AdornSubLip", "CC_AdornSubTouch", "CC_AdornSubEyeLine", "CC_AdornSubEyeBrow" };
	for (int32_t i = 0; i < static_cast<int32_t>(std::size(ADORN_SUB_IDS)); ++i)
	{
		Fn_ShowSubTab(ADORN_SUB_IDS[i], bAdorn, i == m_iSelectedAdornSub);
		if (!bAdorn)
			continue;
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (Get_SlotRect((string(ADORN_SUB_IDS[i]) + "_Bg").c_str(),
			fX, fY, fWidth, fHeight) &&
			Is_Hovered(fX, fY, fWidth, fHeight) && Is_Clicked(fX, fY, fWidth, fHeight))
		{
			CMainApp::Play_UIButtonClickSound();
			m_iSelectedAdornSub = i;
		}
	}
	Fn_ShowList("CC_AdornItem", ADORN_ITEM_COUNT, bAdorn);
	if (bAdorn)
	{
		const std::vector<std::string>& Page = Get_AdornPage();
		m_iAdornScrollRow = Clamp_ScrollRow(m_iAdornScrollRow,
			static_cast<int32_t>(Page.size()), ADORN_ITEM_COUNT);
		Fill_ScrollingGrid("CC_AdornItem", ADORN_ITEM_COUNT, m_iAdornScrollRow, Page);

		/* The icon list and the cooked stamp list are the same retail list, so a cell's icon
		index is its texture index -- the same join the iris grid uses. A cell whose texture
		this client does not ship stays clickable and changes nothing, rather than putting a
		neighbour's make-up on the face. The eyebrow page has no grid at all. */
		const ADORN_PAGE& AdornPage = ADORN_PAGES[m_iSelectedAdornSub];
		const auto* pStamps = nullptr != AdornPage.pTextureKind ?
			m_FaceTextureDocument.Find(m_strIconClassAssetId, AdornPage.pTextureKind) : nullptr;
		const int32_t iIconCount = static_cast<int32_t>(Page.size());
		for (int32_t i = 0; i < ADORN_ITEM_COUNT; ++i)
		{
			const int32_t iEntry = m_iAdornScrollRow * GRID_COLUMNS + i;
			if (iEntry >= iIconCount)
				break;
			f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
			if (!Get_SlotRect(("CC_AdornItem" + std::to_string(i)).c_str(),
				fX, fY, fWidth, fHeight))
			{
				continue;
			}
			if (!Is_Hovered(fX, fY, fWidth, fHeight) || !Is_Clicked(fX, fY, fWidth, fHeight))
				continue;

			CMainApp::Play_UIButtonClickSound();
			if (nullptr == pStamps || m_iSelectedAdornSub >= 3)
			{
				OutputDebugStringA("[FaceMakeup] this page has no stamp list\n");
				continue;
			}
			m_SelectedAdornItems[static_cast<size_t>(m_iSelectedAdornSub)] = iEntry;
			if (nullptr == pCharacter)
				continue;
			const std::string strAsset =
				static_cast<size_t>(iEntry) < pStamps->size() ? (*pStamps)[iEntry] : std::string();
			if (strAsset.empty())
			{
				OutputDebugStringA(("[FaceMakeup] cell " + std::to_string(iEntry) +
					" has no cooked texture\n").c_str());
				continue;
			}
			pCharacter->Set_FaceStampTexture(
				static_cast<CCharacter::FACE_STAMP>(AdornPage.iStampRegister), strAsset);
		}
	}
	/* The strength slider is the layer colour's alpha; see Apply_SurfaceColor. */
	{
		const size_t iPage = static_cast<size_t>(m_iSelectedAdornSub);
		if (Fn_ShowSlider("CC_Slider_adorn_strength", bAdorn, &m_AdornStrength[iPage]))
		{
			const int32_t iSurface = FIRST_MAKEUP_SURFACE_INDEX + m_iSelectedAdornSub;
			Apply_SurfaceColor(pCharacter, iSurface,
				m_SurfaceColors[static_cast<size_t>(iSurface)]);
		}
	}
	Fn_ShowPicker("CC_AdornColor", bAdorn,
		bAdorn ? FIRST_MAKEUP_SURFACE_INDEX + m_iSelectedAdornSub : -1);
	/* Only the eye-line page carries the second slider/chip pair. */
	const bool_t bAdornShadow = bAdorn && 2 == m_iSelectedAdornSub;
	{
		const size_t iShadow = m_AdornStrength.size() - 1u;
		if (Fn_ShowSlider("CC_Slider_adorn_shadow", bAdornShadow, &m_AdornStrength[iShadow]))
		{
			Apply_SurfaceColor(pCharacter, EYESHADOW_SURFACE_INDEX,
				m_SurfaceColors[EYESHADOW_SURFACE_INDEX]);
		}
	}
	Fn_ShowPicker("CC_AdornShadowColor", bAdornShadow,
		bAdornShadow ? EYESHADOW_SURFACE_INDEX : -1);

	if (bHair)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		for (int32_t i = 0; i < 2; ++i)
		{
			const char_t* pId = 0 == i ? "CC_HairSubBasic" : "CC_HairSubTwoTone";
			if (Get_SlotRect((string(pId) + "_Bg").c_str(), fX, fY, fWidth, fHeight) &&
				Is_Hovered(fX, fY, fWidth, fHeight) && Is_Clicked(fX, fY, fWidth, fHeight))
			{
				CMainApp::Play_UIButtonClickSound();
				m_isHairTwoTone = 1 == i;
			}
		}
	}
	if (bEye)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		for (int32_t i = 0; i < 2; ++i)
		{
			const char_t* pId = 0 == i ? "CC_EyeSubIris" : "CC_EyeSubOdd";
			if (Get_SlotRect((string(pId) + "_Bg").c_str(), fX, fY, fWidth, fHeight) &&
				Is_Hovered(fX, fY, fWidth, fHeight) && Is_Clicked(fX, fY, fWidth, fHeight))
			{
				CMainApp::Play_UIButtonClickSound();
				m_isEyeOddSelected = 1 == i;
				/* The material decides whether the left-eye colours are read at all, and the
				two swatches now stand for the other eye, so they show its colours. */
				if (nullptr != pCharacter)
				{
					const f32_t fOdd = m_isEyeOddSelected ? 1.f : 0.f;
					pCharacter->Set_FaceMaterialParameter(EYE_ODD_COLOR_PARAMETER,
						float4_t(fOdd, fOdd, fOdd, fOdd));
					float4_t vEye{};
					if (pCharacter->Try_Get_FaceMaterialParameter(m_isEyeOddSelected ?
						EYE_BASE_COLOR_LEFT_PARAMETER : EYE_BASE_COLOR_PARAMETER, vEye))
					{
						m_SurfaceColors[EYE_BASE_SURFACE_INDEX] =
							float4_t(vEye.x, vEye.y, vEye.z, 1.f);
					}
					if (pCharacter->Try_Get_FaceMaterialParameter(m_isEyeOddSelected ?
						EYE_IRIS_COLOR_LEFT_PARAMETER : EYE_IRIS_COLOR_PARAMETER, vEye))
					{
						m_SurfaceColors[EYE_IRIS_SURFACE_INDEX] =
							float4_t(vEye.x, vEye.y, vEye.z, 1.f);
						m_fEyeIrisAlpha = std::clamp(vEye.w, 0.f, 1.f);
					}
				}
			}
		}
	}
}

void Client::CCustomizingView::Apply_FacePreset(
	const shared_ptr<CCharacter>& pCharacter, const int32_t iPreset)
{
	if (nullptr == pCharacter || !pCharacter->Has_FaceMorphs())
		return;

	/* Every preset states a weight for every morph, so clearing first is only needed for the
	no-preset case; doing it always keeps the two paths identical and cheap (a weight that
	does not actually change is dropped by the applier). */
	pCharacter->Reset_FaceMorphs();
	if (iPreset < 0)
		return;

	const auto* pPresets = m_FacePresetDocument.Find(m_strIconClassAssetId);
	if (nullptr == pPresets || static_cast<size_t>(iPreset) >= pPresets->size())
		return;
	for (const std::pair<std::string, f32_t>& Morph : (*pPresets)[iPreset].Morphs)
		pCharacter->Set_FaceMorphWeight(Morph.first, Morph.second);
}

void Client::CCustomizingView::Update_FaceTab(const shared_ptr<CCharacter>& pCharacter)
{
	const bool_t bFaceTab = FACE_TAB_INDEX == m_iSelectedTab;
	m_bHasFaceSliders = nullptr != pCharacter && pCharacter->Has_FaceSliders();
	Apply_FaceAccordionLayout();

	/* Two CharCustom_CategoryCheckBox headers: the preset half and the slider half. Clicking
	either folds it, and folding the first slides the second up into its place. */
	const bool_t bDefaultOpen = bFaceTab && m_isFaceDefaultExpanded;
	const bool_t bDetailOpen = bFaceTab && m_isFaceDetailExpanded;
	m_pView->Set_SlotVisible("CC_FoldDefault_Bg", bFaceTab);
	m_pView->Set_SlotVisible("CC_FoldDefault_Arrow", bFaceTab);
	m_pView->Set_SlotVisible("CC_DefaultDivision", bDefaultOpen);
	m_pView->Set_SlotVisible("CC_FaceRandomBtn", bDefaultOpen);
	m_pView->Set_SlotVisible("CC_FaceResetBtn", bDefaultOpen);
	const auto* pFacePresets = m_FacePresetDocument.Find(m_strIconClassAssetId);
	const int32_t iFacePresetCount = nullptr != pFacePresets ?
		static_cast<int32_t>(pFacePresets->size()) : 0;
	for (int32_t i = 0; i < FACE_PRESET_COUNT; ++i)
	{
		const string strId = "CC_FacePreset" + std::to_string(i);
		m_pView->Set_SlotVisible(strId, bDefaultOpen);
		m_pView->Set_SlotVisible(strId + "_Plate", bDefaultOpen);
		if (!bDefaultOpen || i >= iFacePresetCount)
			continue;

		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!Get_SlotRect(strId.c_str(), fX, fY, fWidth, fHeight))
			continue;
		if (Is_Hovered(fX, fY, fWidth, fHeight) && Is_Clicked(fX, fY, fWidth, fHeight))
		{
			CMainApp::Play_UIButtonClickSound();
			m_iSelectedFacePreset = i;
			Apply_FacePreset(pCharacter, i);
		}
	}

	m_pView->Set_SlotVisible("CC_FoldDetail_Bg", bFaceTab);
	m_pView->Set_SlotVisible("CC_FoldDetail_Arrow", bFaceTab);
	m_pView->Set_SlotVisible("CC_DetailDivision", bDetailOpen);
	Update_FaceCategoryHeader("CC_FoldDefault_Bg",
		m_isFaceDefaultExpanded, m_isFaceDetailExpanded, bFaceTab);
	Update_FaceCategoryHeader("CC_FoldDetail_Bg",
		m_isFaceDetailExpanded, m_isFaceDefaultExpanded, bFaceTab);

	for (int32_t i = 0; i < FACE_PART_COUNT; ++i)
	{
		const string strBgId = "CC_FaceSub" + std::to_string(i) + "_Bg";
		const string strSelectedId = "CC_FaceSub" + std::to_string(i) + "_Selected";
		m_pView->Set_SlotVisible(strBgId, bDetailOpen);
		m_pView->Set_SlotVisible(strSelectedId, bDetailOpen && i == m_iSelectedPart);
		if (!bDetailOpen)
			continue;

		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!Get_SlotRect(strBgId.c_str(), fX, fY, fWidth, fHeight))
			continue;
		const bool_t bHovered = Is_Hovered(fX, fY, fWidth, fHeight);
		m_pView->Set_SlotTexture(strBgId, bHovered ? ASSET_SUBTAB_OVER : ASSET_SUBTAB_NORMAL);
		if (bHovered && Is_Clicked(fX, fY, fWidth, fHeight))
		{
			CMainApp::Play_UIButtonClickSound();
			m_iSelectedPart = i;
			m_iDraggingRow = -1;
		}
	}

	Apply_SliderVisibility();
	if (!bDetailOpen)
	{
		m_iDraggingRow = -1;
		return;
	}

	CUIInputRouter& Router = CUIInputRouter::Get();
	if (!Router.Is_LeftDown())
		m_iDraggingRow = -1;

	f32_t fMouseX = 0.f, fMouseY = 0.f;
	const bool_t bHasMouse = Router.Get_MousePosition(REF_WIDTH, REF_HEIGHT, fMouseX, fMouseY);

	for (int32_t iRow = 0; iRow < static_cast<int32_t>(std::size(FACE_SLIDER_ROWS)); ++iRow)
	{
		const FACE_SLIDER_ROW& Row = FACE_SLIDER_ROWS[iRow];
		if (Row.iPart != m_iSelectedPart)
			continue;

		f32_t fTrackX = 0.f, fTrackY = 0.f, fTrackWidth = 0.f, fTrackHeight = 0.f;
		f32_t fThumbX = 0.f, fThumbY = 0.f, fThumbWidth = 0.f, fThumbHeight = 0.f;
		const string strTrackId = Slider_TrackSlotId(Row.pSliderId);
		const string strThumbId = Slider_ThumbSlotId(Row.pSliderId);
		if (!Get_SlotRect(strTrackId.c_str(), fTrackX, fTrackY, fTrackWidth, fTrackHeight) ||
			!Get_SlotRect(strThumbId.c_str(), fThumbX, fThumbY, fThumbWidth, fThumbHeight))
		{
			continue;
		}

		const int32_t iSlider = nullptr != pCharacter ?
			Find_SliderIndex(*pCharacter, Row.pSliderId) : -1;
		f32_t fWeight = 0.5f;
		if (iSlider >= 0)
			fWeight = pCharacter->Get_FaceCustomize().Get_Weight(static_cast<size_t>(iSlider));

		/* The whole bar accepts the press, not just the thumb, matching the retail slider. */
		const f32_t fHitY = fThumbY - 4.f;
		const f32_t fHitHeight = fThumbHeight + 8.f;
		const bool_t bHovered = Is_Hovered(fTrackX, fHitY, fTrackWidth, fHitHeight);
		if (iSlider >= 0 && bHovered && Router.Is_LeftClickEdge())
			m_iDraggingRow = iRow;

		if (iSlider >= 0 && m_iDraggingRow == iRow && bHasMouse && fTrackWidth > 0.f)
		{
			fWeight = std::clamp((fMouseX - fTrackX) / fTrackWidth, 0.f, 1.f);
			pCharacter->Set_FaceSliderWeight(static_cast<size_t>(iSlider), fWeight);
		}

		m_pView->Set_SlotTexture(strThumbId,
			(bHovered || m_iDraggingRow == iRow) ? ASSET_THUMB_OVER : ASSET_THUMB_NORMAL);
		m_pView->Set_SlotPosition(strThumbId,
			fTrackX + fWeight * fTrackWidth - fThumbWidth * 0.5f,
			fThumbY);
		m_pView->Set_SlotTint(strTrackId, iSlider >= 0 ?
			float4_t(1.f, 1.f, 1.f, 1.f) : float4_t(0.4f, 0.4f, 0.4f, 1.f));
		m_pView->Set_SlotTint(strThumbId, iSlider >= 0 ?
			float4_t(1.f, 1.f, 1.f, 1.f) : float4_t(0.4f, 0.4f, 0.4f, 1.f));
	}
}

void Client::CCustomizingView::Update_Buttons(const shared_ptr<CCharacter>& pCharacter)
{
	Update_ActionList(pCharacter);

	struct TEXT_BUTTON
	{
		const char_t* pSlotId;
		const char_t* pIdle;
		const char_t* pHover;
	};
	constexpr TEXT_BUTTON BUTTONS[] = {
		{ "CC_LeftResetBtn", ASSET_BUTTON_NORMAL, ASSET_BUTTON_OVER },
		{ "CC_LeftSaveBtn", ASSET_BUTTON_NORMAL, ASSET_BUTTON_OVER },
		{ "CC_LeftLoadBtn", ASSET_BUTTON_NORMAL, ASSET_BUTTON_OVER },
		{ "CC_FaceRandomBtn", ASSET_BUTTON_NORMAL, ASSET_BUTTON_OVER },
		{ "CC_FaceResetBtn", ASSET_BUTTON_NORMAL, ASSET_BUTTON_OVER },
		{ "CC_CreateBtn", ASSET_BUTTON_NORMAL, ASSET_BUTTON_OVER },
		{ "CC_BackIcon", ASSET_BACK_ICON, ASSET_BACK_ICON_OVER },
		{ "CC_ResetAllIcon", ASSET_RESET_ICON, ASSET_RESET_ICON_OVER },
	};

	/* Picking the slot the two buttons act on. */
	for (int32_t i = 0; i < SAVE_SLOT_COUNT; ++i)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!Get_SlotRect(("CC_LeftSave" + std::to_string(i)).c_str(),
			fX, fY, fWidth, fHeight))
		{
			continue;
		}
		if (Is_Hovered(fX, fY, fWidth, fHeight) && Is_Clicked(fX, fY, fWidth, fHeight))
		{
			CMainApp::Play_UIButtonClickSound();
			m_iSelectedSaveSlot = i;
		}
	}

	m_pView->Set_SlotVisible("CC_CreateBtn", true);
	m_pView->Set_SlotVisible("CC_LeftResetBtn", true);
	m_pView->Set_SlotVisible("CC_BackIcon", true);
	m_pView->Set_SlotVisible("CC_ResetAllIcon", true);

	const auto Fn_RandomizeFace = [this, &pCharacter]()
	{
		if (nullptr == pCharacter)
			return;
		std::uniform_real_distribution<f32_t> Distribution(0.15f, 0.85f);
		const size_t iCount = pCharacter->Get_FaceCustomize().Get_SliderCount();
		for (size_t i = 0; i < iCount; ++i)
			pCharacter->Set_FaceSliderWeight(i, Distribution(m_Random));

		/* The face is both halves: the additive slider poses and the preset's MorphTargets.
		Randomising only the sliders left the shape itself untouched. */
		const auto* pPresets = m_FacePresetDocument.Find(m_strIconClassAssetId);
		if (nullptr != pPresets && !pPresets->empty())
		{
			std::uniform_int_distribution<int32_t> PresetDistribution(
				0, static_cast<int32_t>(pPresets->size()) - 1);
			m_iSelectedFacePreset = PresetDistribution(m_Random);
			Apply_FacePreset(pCharacter, m_iSelectedFacePreset);
		}
	};

	/* Set_SlotVisible only stops a slot being drawn; the rect stays where it was authored.
	These two belong to the face tab's first section, so without this they kept taking clicks
	from whatever tab was actually on screen. */
	const bool_t bFaceDefaultOpen =
		FACE_TAB_INDEX == m_iSelectedTab && m_isFaceDefaultExpanded;

	for (const TEXT_BUTTON& Button : BUTTONS)
	{
		if (!bFaceDefaultOpen &&
			(0 == std::strcmp(Button.pSlotId, "CC_FaceRandomBtn") ||
			 0 == std::strcmp(Button.pSlotId, "CC_FaceResetBtn")))
		{
			continue;
		}
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!Get_SlotRect(Button.pSlotId, fX, fY, fWidth, fHeight))
			continue;
		const bool_t bHovered = Is_Hovered(fX, fY, fWidth, fHeight);
		m_pView->Set_SlotTexture(Button.pSlotId, bHovered ? Button.pHover : Button.pIdle);
		if (!bHovered || !Is_Clicked(fX, fY, fWidth, fHeight))
			continue;

		CMainApp::Play_UIButtonClickSound();
		if (0 == std::strcmp(Button.pSlotId, "CC_FaceRandomBtn"))
			Fn_RandomizeFace();
		else if (0 == std::strcmp(Button.pSlotId, "CC_FaceResetBtn"))
		{
			/* The face button resets the face. Same reason as the randomise path: the shape
			is the preset's MorphTargets, so a reset that only cleared the sliders left the
			previous face on screen. */
			if (nullptr != pCharacter)
				pCharacter->Reset_FaceSliders();
			m_iSelectedFacePreset = -1;
			Apply_FacePreset(pCharacter, -1);
		}
		else if (0 == std::strcmp(Button.pSlotId, "CC_ResetAllIcon") ||
			0 == std::strcmp(Button.pSlotId, "CC_LeftResetBtn"))
		{
			/* The bottom bar's is the retail avatar reset: colour, make-up, hair and eyes
			go back to the class' authored values too, not the face alone. */
			Reset_All(pCharacter);
		}
		else if (0 == std::strcmp(Button.pSlotId, "CC_LeftSaveBtn"))
		{
			if (!Save_Slot(pCharacter, m_iSelectedSaveSlot))
				OutputDebugStringA("[CustomizingView] preset save failed\n");
		}
		else if (0 == std::strcmp(Button.pSlotId, "CC_LeftLoadBtn"))
		{
			if (!Load_Slot(pCharacter, m_iSelectedSaveSlot))
				OutputDebugStringA("[CustomizingView] preset load failed\n");
		}
		else if (0 == std::strcmp(Button.pSlotId, "CC_CreateBtn"))
			m_bDecideRequested = true;
		else if (0 == std::strcmp(Button.pSlotId, "CC_BackIcon"))
			m_bBackRequested = true;
	}
}

void Client::CCustomizingView::Render_Text()
{
	if (!m_bOpen || nullptr == m_pView)
		return;

	const float2_t vViewportSize = CGameInstance::Get().Get_ViewportSize();
	const f32_t fScaleX = vViewportSize.x / REF_WIDTH;
	const f32_t fScaleY = vViewportSize.y / REF_HEIGHT;
	const f32_t fUiScale = (std::min)(fScaleX, fScaleY);

	/* fSize is the em size the .gfx text field carries, brought into reference units. Two things
	follow from that. Measure_Text returns a string's line spacing, not its em, so dividing by it
	drew every label about a fifth short -- and by that string's own tallest glyph, so two labels
	asking for the same size did not come out the same. And the shipped sprite fonts are baked at
	32-42 px, which SpriteBatch resamples into a blur at label sizes. UILabelFont::Resolve answers
	both: it normalises on one reference glyph and hands back a pre-downsampled font drawn 1:1
	wherever one is baked for that size. */
	const auto Fn_Resolve = [&](const wstring& strFamily, f32_t fSize, f32_t& outScale)
	{
		return UILabelFont::Resolve(
			strFamily, fSize * EM_TO_LINE_SPACING * fUiScale, outScale);
	};
	const auto Fn_Draw = [&](const wstring& strFamily, f32_t fX, f32_t fY, f32_t fSize,
		const fvector_t& vColor, const wchar_t* pText, const float2_t& vPivot)
	{
		if (nullptr == pText || L'\0' == pText[0])
			return;
		f32_t fScale = 1.f;
		const wstring strFont = Fn_Resolve(strFamily, fSize, fScale);
		const float2_t vPos(fX * fScaleX, fY * fScaleY);
		CGameInstance::Get().Draw_Text(strFont, pText,
			float2_t(vPos.x + 1.f, vPos.y + 1.f),
			XMVectorSet(0.f, 0.f, 0.f, 0.85f), 0.f, vPivot, fScale);
		CGameInstance::Get().Draw_Text(strFont, pText, vPos, vColor, 0.f, vPivot, fScale);
	};

	/* Lays consecutive coloured runs out as a single centred line: each run is measured in
	the same font at the same size, so the joins land exactly where one string would. */
	struct TEXT_RUN { const wchar_t* pText; vector_t vColor; };
	const auto Fn_DrawRuns = [&](const wstring& strFont, f32_t fCenterX, f32_t fCenterY,
		f32_t fSize, const TEXT_RUN* pRuns, size_t iNumRuns)
	{
		f32_t fScale = 1.f;
		const wstring strResolved = Fn_Resolve(strFont, fSize, fScale);
		f32_t fTotalWidth = 0.f;
		for (size_t i = 0; i < iNumRuns; ++i)
		{
			fTotalWidth +=
				CGameInstance::Get().Measure_Text(strResolved, pRuns[i].pText).x * fScale;
		}
		f32_t fPenX = fCenterX * fScaleX - fTotalWidth * 0.5f;
		const f32_t fPenY = fCenterY * fScaleY;
		for (size_t i = 0; i < iNumRuns; ++i)
		{
			CGameInstance::Get().Draw_Text(strResolved, pRuns[i].pText,
				float2_t(fPenX + 1.f, fPenY + 1.f), XMVectorSet(0.f, 0.f, 0.f, 0.85f),
				0.f, float2_t(0.f, 0.5f), fScale);
			CGameInstance::Get().Draw_Text(strResolved, pRuns[i].pText, float2_t(fPenX, fPenY),
				pRuns[i].vColor, 0.f, float2_t(0.f, 0.5f), fScale);
			fPenX += CGameInstance::Get().Measure_Text(
				strResolved, pRuns[i].pText).x * fScale;
		}
	};

	const fvector_t vSectionColor = XMVectorSet(164.f / 255.f, 175.f / 255.f, 183.f / 255.f, 1.f);
	const fvector_t vRowColor = XMVectorSet(224.f / 255.f, 230.f / 255.f, 236.f / 255.f, 1.f);
	const fvector_t vDescColor = XMVectorSet(1.f, 1.f, 1.f, 1.f);
	const fvector_t vDimColor = XMVectorSet(0.45f, 0.45f, 0.45f, 1.f);

	/* Tab captions: CharCustom_Right_TabListRenderer keeps its textField at alpha 0 on the
	idle frame and only raises it on the over/selected frames, so only the selected tab (or
	the one under the pointer) shows a name. */
	for (int32_t i = 0; i < static_cast<int32_t>(std::size(LABEL_TABS)); ++i)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!Get_SlotRect(("CC_Tab" + std::to_string(i) + "_Bg").c_str(),
			fX, fY, fWidth, fHeight))
		{
			continue;
		}
		const bool_t bSelected = i == m_iSelectedTab;
		const bool_t bSupported = VOICE_TAB_INDEX != i && BASE_TAB_INDEX != i;
		const bool_t bHovered = bSupported && Is_Hovered(fX, fY, fWidth, fHeight);
		if (!bSelected && !bHovered)
			continue;
		Fn_Draw(TEXT("Font_YoonGasiIIM"), fX + fWidth * 0.5f, fY + fHeight - 6.f, 11.f,
			bSupported ? vDescColor : vDimColor, LABEL_TABS[i], float2_t(0.5f, 0.5f));
	}

	/* Left column captions. The lists themselves have no data yet, so only their headings
	draw -- the panel art already carries the rest of the column. */
	Fn_Draw(TEXT("Font_YG760"), LEFT_LABEL_X * GFX_TO_REF, LEFT_COSTUME_Y * GFX_TO_REF,
		12.f, vSectionColor, LABEL_LEFT_COSTUME, float2_t(0.f, 0.f));
	Fn_Draw(TEXT("Font_YG760"), LEFT_LABEL_X * GFX_TO_REF, LEFT_ACTION_Y * GFX_TO_REF,
		12.f, vSectionColor, LABEL_LEFT_ACTION, float2_t(0.f, 0.f));
	Fn_Draw(TEXT("Font_YG760"), LEFT_LABEL_X * GFX_TO_REF, LEFT_SAVE_Y * GFX_TO_REF,
		12.f, vSectionColor, LABEL_LEFT_SAVE, float2_t(0.f, 0.f));
	{
		struct LEFT_BUTTON { const char_t* pSlotId; const wchar_t* pLabel; };
		constexpr LEFT_BUTTON LEFT_BUTTONS[] = {
			{ "CC_LeftSaveBtn", LABEL_SAVE },
			{ "CC_LeftLoadBtn", LABEL_LOAD },
		};
		for (const LEFT_BUTTON& Button : LEFT_BUTTONS)
		{
			f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
			if (!Get_SlotRect(Button.pSlotId, fX, fY, fWidth, fHeight))
				continue;
			Fn_Draw(TEXT("Font_YG760"), fX + fWidth * 0.5f, fY + fHeight * 0.5f, 11.f,
				vDescColor, Button.pLabel, float2_t(0.5f, 0.5f));
		}
	}
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (Get_SlotRect("CC_LeftResetBtn", fX, fY, fWidth, fHeight))
		{
			Fn_Draw(TEXT("Font_YG760"), fX + fWidth * 0.5f, fY + fHeight * 0.5f, 11.f,
				vDescColor, LABEL_LEFT_RESET, float2_t(0.5f, 0.5f));
		}
	}

	/* CharCustom_Bottom_CreateOutGamePage's own guide label: it sits on the bottom plate just
	above the decide button, not on the top panel. */
	f32_t fGuideX = 0.f, fGuideY = 0.f, fGuideWidth = 0.f, fGuideHeight = 0.f;
	if (Get_SlotRect("CC_BottomBg", fGuideX, fGuideY, fGuideWidth, fGuideHeight))
	{
		const fvector_t vAccent = XMVectorSet(1.f, 210.f / 255.f, 0.f, 1.f);
		const TEXT_RUN Runs[] = {
			{ LABEL_BOTTOM_GUIDE_HEAD, vDescColor },
			{ LABEL_BOTTOM_GUIDE_ACCENT, vAccent },
			{ LABEL_BOTTOM_GUIDE_TAIL, vDescColor },
		};
		Fn_DrawRuns(TEXT("Font_YG760"), fGuideX + fGuideWidth * 0.5f,
			fGuideY + fGuideHeight * 0.5f, SIZE_BOTTOM_GUIDE, Runs, std::size(Runs));
	}

	/* rightTitleLabel: centred over the right panel, 18 gfx units down from its top. */
	{
		f32_t fTitleX = 0.f, fTitleY = 0.f, fTitleWidth = 0.f, fTitleHeight = 0.f;
		if (Get_SlotRect("CC_RightBg", fTitleX, fTitleY, fTitleWidth, fTitleHeight))
		{
			Fn_Draw(TEXT("Font_YoonGasiIIM"), fTitleX + fTitleWidth * 0.5f,
				fTitleY + 18.f * GFX_TO_REF, SIZE_RIGHT_TITLE,
				XMVectorSet(1.f, 252.f / 255.f, 226.f / 255.f, 1.f),
				LABEL_RIGHT_TITLE, float2_t(0.5f, 0.f));
		}
	}

	f32_t fFoldX = 0.f, fFoldY = 0.f, fFoldWidth = 0.f, fFoldHeight = 0.f;
	if (FACE_TAB_INDEX == m_iSelectedTab &&
		Get_SlotRect("CC_FoldDefault_Bg", fFoldX, fFoldY, fFoldWidth, fFoldHeight))
	{
		Fn_Draw(TEXT("Font_YG760"), fFoldX + 12.f, fFoldY + fFoldHeight * 0.5f, 11.f,
			vDescColor, LABEL_FOLD_DEFAULT, float2_t(0.f, 0.5f));
	}
	/* The first category's own two lines sit above its preset grid: a white 14pt caption and
	a 16pt accent line, the same pair CharCustom_Right_TabFaceDefaultContent carries. */
	f32_t fDefX = 0.f, fDefY = 0.f, fDefWidth = 0.f, fDefHeight = 0.f;
	if (m_isFaceDefaultExpanded && FACE_TAB_INDEX == m_iSelectedTab &&
		Get_SlotRect("CC_DefaultDivision", fDefX, fDefY, fDefWidth, fDefHeight))
	{
		Fn_Draw(TEXT("Font_YG760"), fDefX + 2.f, fDefY - 14.f, 9.33f,
			vDescColor, LABEL_FACE_DESC, float2_t(0.f, 0.5f));
		Fn_Draw(TEXT("Font_YoonGasiIIM"), fDefX + 2.f, fDefY + 10.f, 10.67f,
			vSectionColor, LABEL_FACE_FORM, float2_t(0.f, 0.5f));
	}
	if (FACE_TAB_INDEX == m_iSelectedTab &&
		Get_SlotRect("CC_FoldDetail_Bg", fFoldX, fFoldY, fFoldWidth, fFoldHeight))
	{
		Fn_Draw(TEXT("Font_YG760"), fFoldX + 12.f, fFoldY + fFoldHeight * 0.5f, 11.f,
			vDescColor, LABEL_FOLD_DETAIL, float2_t(0.f, 0.5f));
	}

	if (FACE_TAB_INDEX != m_iSelectedTab)
	{
		/* Only a tab with nothing behind it says so. Hair, eye, skin and adorn all draw
		their own captions now and the line sat on top of them. */
		f32_t fPanelX = 0.f, fPanelY = 0.f, fPanelWidth = 0.f, fPanelHeight = 0.f;
		if (VOICE_TAB_INDEX == m_iSelectedTab &&
			Get_SlotRect("CC_RightBg", fPanelX, fPanelY, fPanelWidth, fPanelHeight))
		{
			Fn_Draw(TEXT("Font_YG760"), fPanelX + fPanelWidth * 0.5f, 200.f, 12.f,
				vDimColor, LABEL_NOT_READY, float2_t(0.5f, 0.5f));
		}
	}
	else if (m_isFaceDetailExpanded)
	{
		f32_t fDivX = 0.f, fDivY = 0.f, fDivWidth = 0.f, fDivHeight = 0.f;
		if (Get_SlotRect("CC_DetailDivision", fDivX, fDivY, fDivWidth, fDivHeight))
		{
			Fn_Draw(TEXT("Font_YG760"), fDivX, fDivY - 14.f, 11.f,
				m_bHasFaceSliders ? vDescColor : vDimColor,
				m_bHasFaceSliders ? LABEL_DETAIL_DESC : LABEL_NO_SLIDERS,
				float2_t(0.f, 0.5f));
		}

		for (int32_t i = 0; i < FACE_PART_COUNT; ++i)
		{
			f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
			if (!Get_SlotRect(("CC_FaceSub" + std::to_string(i) + "_Bg").c_str(),
				fX, fY, fWidth, fHeight))
			{
				continue;
			}
			Fn_Draw(TEXT("Font_YG760"), fX + fWidth * 0.5f, fY + fHeight * 0.5f, 11.f,
				i == m_iSelectedPart ? vDescColor : vRowColor,
				LABEL_PARTS[i], float2_t(0.5f, 0.5f));
		}

		for (const FACE_PART_HEADER& Header : FACE_PART_HEADERS)
		{
			if (Header.iPart != m_iSelectedPart)
				continue;
			Fn_Draw(TEXT("Font_YoonGasiIIM"), PART_ORIGIN_X + 2.f * PART_SCALE,
				PART_ORIGIN_Y + (Header.fPartY + 10.f) * PART_SCALE, 12.f,
				vSectionColor, Header.pLabel, float2_t(0.f, 0.5f));
		}
		for (const FACE_SLIDER_ROW& Row : FACE_SLIDER_ROWS)
		{
			if (Row.iPart != m_iSelectedPart)
				continue;
			Fn_Draw(TEXT("Font_YG760"), PART_ORIGIN_X + 12.f * PART_SCALE,
				PART_ORIGIN_Y + (Row.fPartY + 14.f) * PART_SCALE, 11.f,
				vRowColor, Row.pLabel, float2_t(0.f, 0.5f));
		}

	}

	if (PICKER_SURFACE_NONE != m_iPickerSurface)
	{
		struct PICKER_LABEL { const char_t* pSlotId; const wchar_t* pLabel; };
		constexpr PICKER_LABEL PICKER_LABELS[] = {
			{ "CC_PickerApply", LABEL_PICKER_APPLY },
			{ "CC_PickerCancel", LABEL_PICKER_CANCEL },
		};
		for (const PICKER_LABEL& Entry : PICKER_LABELS)
		{
			f32_t fX = 0.f, fY = 0.f, fW = 0.f, fH = 0.f;
			if (!Get_SlotRect(Entry.pSlotId, fX, fY, fW, fH))
				continue;
			Fn_Draw(TEXT("Font_YG760"), fX + fW * 0.5f, fY + fH * 0.5f, 11.f,
				vDescColor, Entry.pLabel, float2_t(0.5f, 0.5f));
		}
	}

	/* These two sit under the preset grid, so they follow the first section, not the second. */
	if (FACE_TAB_INDEX == m_iSelectedTab && m_isFaceDefaultExpanded)
	{
		struct PANEL_BUTTON_LABEL { const char_t* pSlotId; const wchar_t* pLabel; };
		constexpr PANEL_BUTTON_LABEL PANEL_BUTTONS[] = {
			{ "CC_FaceRandomBtn", LABEL_FACE_RANDOM },
			{ "CC_FaceResetBtn", LABEL_FACE_RESET },
		};
		for (const PANEL_BUTTON_LABEL& Button : PANEL_BUTTONS)
		{
			f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
			if (!Get_SlotRect(Button.pSlotId, fX, fY, fWidth, fHeight))
				continue;
			Fn_Draw(TEXT("Font_YG760"), fX + fWidth * 0.5f, fY + fHeight * 0.5f, 11.f,
				vDescColor, Button.pLabel, float2_t(0.5f, 0.5f));
		}
	}

	/* Everything on the hair, eye, skin and adorn tabs. Each entry is one authored slot and
	the retail caption that belongs on it -- centred inside a chip or a button, left-aligned
	above a swatch or a slider track, which is where the source screen puts them. */
	{
		struct TAB_LABEL
		{
			int32_t iTab;
			const char_t* pSlotId;
			const wchar_t* pLabel;
			bool_t isCentred;
			bool_t isSectionHeader;
		};
		constexpr TAB_LABEL TAB_LABELS[] = {
			{ HAIR_TAB_INDEX,  "CC_HairSubBasic_Bg",       LABEL_HAIR_SUB_BASE,      true  , false },
			{ HAIR_TAB_INDEX,  "CC_HairSubTwoTone_Bg",     LABEL_HAIR_SUB_TWOTONE,   true  , false },
			{ HAIR_TAB_INDEX,  "CC_HairDivision",          LABEL_HAIR_DESC,          false , true  },
			{ HAIR_TAB_INDEX,  "CC_HairShape0_Plate",      LABEL_HAIR_FORM,          false , true  },
			{ HAIR_TAB_INDEX,  "CC_HairColor",             LABEL_HAIR_COLOR,         false , false },
			{ HAIR_TAB_INDEX,  "CC_Slider_hair_strength_Track", LABEL_HAIR_GLOSS,    false , true  },
			{ HAIR_TAB_INDEX,  "CC_Slider_hair_strength_Track", LABEL_STRENGTH,      false , false },
			{ HAIR_TAB_INDEX,  "CC_Slider_hair_range_Track",    LABEL_RANGE,         false , false },

			{ EYE_TAB_INDEX,   "CC_EyeSubIris_Bg",         LABEL_EYE_SUB_BOTH,       true  , false },
			{ EYE_TAB_INDEX,   "CC_EyeSubOdd_Bg",          LABEL_EYE_SUB_ODD,        true  , false },
			{ EYE_TAB_INDEX,   "CC_EyeDivision",           LABEL_EYE_DESC,           false , true  },
			{ EYE_TAB_INDEX,   "CC_EyeColor",              LABEL_EYE_COLOR,          false , false },
			{ EYE_TAB_INDEX,   "CC_EyeIrisColor",          LABEL_EYE_IRIS_COLOR,     false , false },
			{ EYE_TAB_INDEX,   "CC_EyeIris0_Plate",        LABEL_EYE_LENZ,           false , true  },
			{ EYE_TAB_INDEX,   "CC_Slider_eye_scale_Track",      LABEL_EYE_SIZE,     false , false },
			{ EYE_TAB_INDEX,   "CC_Slider_eye_definition_Track", LABEL_ALPHA,        false , false },

			{ SKIN_TAB_INDEX,  "CC_SkinDivision",          LABEL_SKIN_DESC,          false , true  },
			{ SKIN_TAB_INDEX,  "CC_SkinColor",             LABEL_SKIN_COLOR,         false , false },
			{ SKIN_TAB_INDEX,  "CC_Slider_skin_age_Track",      LABEL_SKIN_WRINKLE,  false , false },
			{ SKIN_TAB_INDEX,  "CC_Slider_skin_shine_Track",    LABEL_SKIN_GLOSS,    false , true  },
			{ SKIN_TAB_INDEX,  "CC_Slider_skin_shine_Track",    LABEL_STRENGTH,      false , false },
			{ SKIN_TAB_INDEX,  "CC_Slider_skin_freckles_Track", LABEL_SKIN_FRECKLE,  false , false },

			{ ADORN_TAB_INDEX, "CC_AdornSubLip_Bg",        LABEL_ADORN_SUB_LIP,      true  , false },
			{ ADORN_TAB_INDEX, "CC_AdornSubTouch_Bg",      LABEL_ADORN_SUB_CHEEK,    true  , false },
			{ ADORN_TAB_INDEX, "CC_AdornSubEyeLine_Bg",    LABEL_ADORN_SUB_EYEMAKE,  true  , false },
			{ ADORN_TAB_INDEX, "CC_AdornSubEyeBrow_Bg",    LABEL_ADORN_SUB_EYEBROW,  true  , false },
			{ ADORN_TAB_INDEX, "CC_AdornDivision",         LABEL_ADORN_DESC,         false , true  },
		};
		/* The adorn pages share one set of slots, so their captions come from the page in
		view rather than from the table above: retail names each page's list, swatch and
		slider after that page, and the cheek slider is a strength where the rest are an
		alpha. */
		struct ADORN_PAGE_LABEL
		{
			const wchar_t* pFormLabel;
			const wchar_t* pColorLabel;
			const wchar_t* pStrengthLabel;
		};
		constexpr ADORN_PAGE_LABEL ADORN_PAGE_LABELS[] = {
			{ LABEL_ADORN_LIP_FORM,     LABEL_ADORN_LIP_COLOR,     LABEL_ALPHA    },
			{ LABEL_ADORN_CHEEK_FORM,   LABEL_ADORN_CHEEK_COLOR,   LABEL_STRENGTH },
			{ LABEL_ADORN_EYEMAKE_FORM, LABEL_ADORN_EYELINE_COLOR, LABEL_ALPHA    },
			{ nullptr,                  LABEL_ADORN_BROW_COLOR,    LABEL_ALPHA    },
		};
		static_assert(std::size(ADORN_PAGE_LABELS) == std::size(ADORN_PAGES),
			"one caption row per adorn page");
		for (const TAB_LABEL& Entry : TAB_LABELS)
		{
			/* The picker covers the panel, so the captions underneath would read through it. */
			if (Entry.iTab != m_iSelectedTab || PICKER_SURFACE_NONE != m_iPickerSurface)
				continue;
			f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
			if (!Get_SlotRect(Entry.pSlotId, fX, fY, fWidth, fHeight))
				continue;
			if (Entry.isCentred)
			{
				/* A sub-tab chip carries its caption inside itself. */
				Fn_Draw(TEXT("Font_YG760"), fX + fWidth * 0.5f, fY + fHeight * 0.5f, 11.f,
					vDescColor, Entry.pLabel, float2_t(0.5f, 0.5f));
			}
			else if (Entry.isSectionHeader)
			{
				/* A section caption sits above its divider line. */
				Fn_Draw(TEXT("Font_YG760"), RIGHT_LABEL_X, fY - 12.f, 11.f,
					vDescColor, Entry.pLabel, float2_t(0.f, 0.5f));
			}
			else
			{
				/* A row caption is left-aligned against the panel edge and vertically
				centred on the swatch or slider track it names. */
				Fn_Draw(TEXT("Font_YG760"), RIGHT_LABEL_X, fY + fHeight * 0.5f, 11.f,
					vDescColor, Entry.pLabel, float2_t(0.f, 0.5f));
			}
		}
		if (ADORN_TAB_INDEX == m_iSelectedTab && PICKER_SURFACE_NONE == m_iPickerSurface &&
			m_iSelectedAdornSub >= 0 &&
			m_iSelectedAdornSub < static_cast<int32_t>(std::size(ADORN_PAGE_LABELS)))
		{
			const ADORN_PAGE_LABEL& Page =
				ADORN_PAGE_LABELS[static_cast<size_t>(m_iSelectedAdornSub)];
			f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
			if (nullptr != Page.pFormLabel &&
				Get_SlotRect("CC_AdornItem0_Plate", fX, fY, fWidth, fHeight))
			{
				Fn_Draw(TEXT("Font_YG760"), RIGHT_LABEL_X, fY - 12.f, 11.f,
					vDescColor, Page.pFormLabel, float2_t(0.f, 0.5f));
			}
			if (Get_SlotRect("CC_AdornColor", fX, fY, fWidth, fHeight))
			{
				Fn_Draw(TEXT("Font_YG760"), RIGHT_LABEL_X, fY + fHeight * 0.5f, 11.f,
					vDescColor, Page.pColorLabel, float2_t(0.f, 0.5f));
			}
			if (Get_SlotRect("CC_Slider_adorn_strength_Track", fX, fY, fWidth, fHeight))
			{
				Fn_Draw(TEXT("Font_YG760"), RIGHT_LABEL_X, fY + fHeight * 0.5f, 11.f,
					vDescColor, Page.pStrengthLabel, float2_t(0.f, 0.5f));
			}
			/* Only the eye make-up page carries the second swatch and slider. */
			if (2 == m_iSelectedAdornSub)
			{
				if (Get_SlotRect("CC_AdornShadowColor", fX, fY, fWidth, fHeight))
				{
					Fn_Draw(TEXT("Font_YG760"), RIGHT_LABEL_X, fY + fHeight * 0.5f, 11.f,
						vDescColor, LABEL_ADORN_SHADOW_COLOR, float2_t(0.f, 0.5f));
				}
				if (Get_SlotRect("CC_Slider_adorn_shadow_Track", fX, fY, fWidth, fHeight))
				{
					Fn_Draw(TEXT("Font_YG760"), RIGHT_LABEL_X, fY + fHeight * 0.5f, 11.f,
						vDescColor, LABEL_ALPHA, float2_t(0.f, 0.5f));
				}
			}
		}
	}

	f32_t fCreateX = 0.f, fCreateY = 0.f, fCreateWidth = 0.f, fCreateHeight = 0.f;
	if (Get_SlotRect("CC_CreateBtn", fCreateX, fCreateY, fCreateWidth, fCreateHeight))
	{
		/* AnimatedButton_renew20_V2's own textField: $YG760 at 18 gfx px. */
		Fn_Draw(TEXT("Font_YG760"), fCreateX + fCreateWidth * 0.5f,
			fCreateY + fCreateHeight * 0.5f, 12.f, vDescColor, LABEL_DECIDE,
			float2_t(0.5f, 0.5f));
	}

	struct ICON_LABEL { const char_t* pSlotId; const wchar_t* pLabel; };
	constexpr ICON_LABEL ICON_LABELS[] = {
		{ "CC_BackIcon", LABEL_BACK },
		{ "CC_ResetAllIcon", LABEL_RESET_ALL },
	};
	for (const ICON_LABEL& Icon : ICON_LABELS)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!Get_SlotRect(Icon.pSlotId, fX, fY, fWidth, fHeight))
			continue;
		Fn_Draw(TEXT("Font_YG760"), fX + fWidth * 0.5f, fY + fHeight + 8.f, 11.f,
			vDescColor, Icon.pLabel, float2_t(0.5f, 0.5f));
	}

	struct GUIDE_LABEL { const char_t* pSlotId; const wchar_t* pLabel; };
	constexpr GUIDE_LABEL GUIDE_LABELS[] = {
		{ "CC_GuideRotateIcon", LABEL_GUIDE_ROTATE },
		{ "CC_GuideEyeTrackIcon", LABEL_GUIDE_EYETRACK },
		{ "CC_GuideZoomIcon", LABEL_GUIDE_ZOOM },
	};
	for (const GUIDE_LABEL& Guide : GUIDE_LABELS)
	{
		f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
		if (!Get_SlotRect(Guide.pSlotId, fX, fY, fWidth, fHeight))
			continue;
		Fn_Draw(TEXT("Font_YG760"), fX + fWidth + 10.f, fY + fHeight * 0.5f,
			SIZE_MOUSE_GUIDE, vRowColor, Guide.pLabel, float2_t(0.f, 0.5f));
	}
}
