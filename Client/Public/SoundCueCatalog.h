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
	using EVENT_VARIANTS = std::unordered_map<std::string,
		std::vector<std::string>>;

	static bool_t Load(std::string& strOutStatus);
	/* Reads one class from the current physical catalog without mutating the
	   process-global startup cache. Exact boss presentation loaders use this
	   while holding their generation admission, then pin the returned asset
	   IDs into the admitted cue rows. */
	static bool_t Load_ClassSnapshot(
		const std::string& strClassName,
		EVENT_VARIANTS& InOutEvents,
		std::string& strOutStatus);

	/* Empty return means "no match" -- not a failure. strClassName is the
	   same Data/Animation/Authored/<Class> folder name Character already
	   uses for CHARACTER_SPEC::pAssetName (e.g. "LanceMaster"). */
	static const std::vector<std::string>& Find_Variants(
		const std::string& strClassName,
		const std::string& strEventName);
	/* Sorted copy for authoring UI. Runtime lookup remains Find_Variants;
	   callers never retain references into the mutable load cache. */
	static std::vector<std::string> Collect_EventNames(
		const std::string& strClassName);

private:
	static std::unordered_map<std::string, EVENT_VARIANTS> s_ClassEvents;
	static bool_t s_bLoaded;
};

NS_END
