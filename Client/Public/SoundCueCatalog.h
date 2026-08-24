#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <string>
#include <unordered_map>
#include <vector>

NS_BEGIN(Client)

/* Loads Data/Sound/CharacterSoundCatalog.json once and answers "given this
   class and this SOUND cue's event name, which wav files (Resources-relative
   asset IDs) can play for it". Multiple entries are equally-weighted
   variations (the same hit/vox line recorded several times) -- the caller
   picks one, this catalog does not. A missing class or event name is not an
   error: the caller silently skips that cue (Client-only presentation, no
   gameplay authority depends on sound actually playing). */
class CSoundCueCatalog final
{
public:
	static bool_t Load(std::string& strOutStatus);

	/* Empty return means "no match" -- not a failure. strClassName is the
	   same Data/Animation/Authored/<Class> folder name Character already
	   uses for CHARACTER_SPEC::pAssetName (e.g. "LanceMaster"). */
	static const std::vector<std::string>& Find_Variants(
		const std::string& strClassName,
		const std::string& strEventName);

private:
	static std::unordered_map<std::string,
		std::unordered_map<std::string, std::vector<std::string>>> s_ClassEvents;
	static bool_t s_bLoaded;
};

NS_END
