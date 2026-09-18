#pragma once

#include "Client_Defines.h"
#include "MapAssetCatalog.h"
#include "MapPlacementDocument.h"
#include "MapPlacementRuntime.h"
#include "WorldSequencePlayer.h"

#include <string>
#include <vector>

NS_BEGIN(Client)

class CDeployPropRuntime;

/* A product Level that lets the Debug Map Tool edit its live map in place.
   The Level keeps owning its placements, batches and, when it has any, its
   Deploy runtime; the tool borrows them while it is attached and never
   stages a second copy of the Area. The KoukuSaydon arena and Character
   Select implement it; Development keeps the tool's own staged Area. */
class IMapAuthoringHost
{
public:
	virtual ~IMapAuthoringHost() = default;

	virtual uint32_t Get_MapAuthoringLevelIndex() const = 0;
	/* Short display name for status text, e.g. "Kouku". */
	virtual const char_t* Get_MapAuthoringLabel() const = 0;
	virtual const CMapAssetCatalog& Get_MapAuthoringCatalog() const = 0;
	virtual std::vector<MAP_RUNTIME_PLACED_ENTRY>& Get_MapAuthoringPlacements() = 0;
	virtual std::vector<MAP_RUNTIME_STATIC_BATCH_ENTRY>& Get_MapAuthoringBatches() = 0;
	/* Null when the Level owns no Deploy props. The tool then skips every
	   Deploy-bound panel instead of staging an empty runtime of its own. */
	virtual CDeployPropRuntime* Get_MapAuthoringDeployRuntime() = 0;
	/* False while something on the Level borrows the placements: a sequence
	   or object preview, the customizing stage, a Debug floor swap. */
	virtual bool_t Can_ChangeMapAuthoringStructure(std::string& outReason) const = 0;
	virtual void Rebase_MapAuthoringSelfMotions(
		const std::vector<MAP_PLACEMENT_RECORD>& records) = 0;
	virtual void Set_MapAuthoringActive(bool_t active) = 0;
	/* Live targets for the shared World Sequence tooling. A Level without
	   Deploy props leaves pDeployRuntime null. */
	virtual CWorldSequencePlayer::TARGET_SET Make_MapAuthoringTargets() = 0;
};

/* The host of the current Level, or null when that Level exposes no live map
   (Lobby, Bern, Valtan, Development). Always null outside Debug builds. */
IMapAuthoringHost* Find_ActiveMapAuthoringHost();

NS_END
