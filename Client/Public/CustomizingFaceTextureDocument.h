#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <string>
#include <unordered_map>
#include <vector>

NS_BEGIN(Client)

/* Data/UI/Customizing/CustomizingFaceTextures.json: the cooked face textures each retail
customizing list offers, per class, in that list's own icon order.

Only "iris" is a whole texture slot -- a 512x512 eye diffuse that replaces the eye material's
own. The other categories (eyemake 512x128, lip 256x128, cheek 256x256, decal13..16 512x512)
are stamps the retail engine composites onto the face texture, and this renderer has no
compositing path for them, so nothing applies them yet; they are read here because the
document is the source of truth for what exists, not because they are wired. */
class CCustomizingFaceTextureDocument final
{
public:
	/* Reads and validates the whole document; on failure the previous contents are kept and
	Get_Status explains why. */
	bool_t Load();
	/* The class's texture asset ids for one category ("iris", "lip", "decal13", ...), in icon
	order. Null when the class or the category is absent. */
	const std::vector<std::string>* Find(
		const std::string& strAssetId, const std::string& strCategory) const;
	const std::string& Get_Status() const { return m_strStatus; }

private:
	std::unordered_map<std::string,
		std::unordered_map<std::string, std::vector<std::string>>> m_Classes;
	std::string m_strStatus = "Customizing face texture document is not loaded";
};

NS_END
