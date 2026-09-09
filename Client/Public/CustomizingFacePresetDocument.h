#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <string>
#include <unordered_map>
#include <vector>

NS_BEGIN(Client)

/* One preset of the character-creation base tab: the face MorphTarget weights it sets. Names
are the authoring spelling (MM_Eye_MeshType_01_UI); the cooked .facemorphs spells the same
morph lowercase, so consumers match case-insensitively (see
Tools/CharacterCustomizing/FACEMORPH_FORMAT.md). A weight of 0 is carried as written rather
than dropped -- a preset says what every morph is, not only what it turns on. */
struct CUSTOMIZING_FACE_PRESET
{
	std::string strSourcePresetId;
	std::vector<std::pair<std::string, f32_t>> Morphs;
};

/* Data/UI/Customizing/CustomizingFacePresets.json, written by
Tools/CharacterCustomizing/build_face_morphs.py from the retail
XmlData/CharacterCustomizing/EFDLChar_PC_<TAG>.PC_<TAG>_<NN>.loa documents. Per class, the
preset list is in the same order as that class's preset icon row in CustomizingIcons.json. */
class CCustomizingFacePresetDocument final
{
public:
	/* Reads and validates the whole document; on failure the previous contents are kept and
	Get_Status explains why. */
	bool_t Load();
	/* The class's presets in icon order. Null when the class is absent. */
	const std::vector<CUSTOMIZING_FACE_PRESET>* Find(const std::string& strAssetId) const;
	const std::string& Get_Status() const { return m_strStatus; }

private:
	std::unordered_map<std::string, std::vector<CUSTOMIZING_FACE_PRESET>> m_Classes;
	std::string m_strStatus = "Customizing face preset document is not loaded";
};

NS_END
