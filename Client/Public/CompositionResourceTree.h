#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

NS_BEGIN(Client)

/* Cached in-memory hierarchy for one explicitly loaded resource catalog.
   Leaves are stable indices into that catalog snapshot; rebuilding the tree
   never performs file I/O and happens only after Reload or search changes.
   Every boss Workbench shares this one component over its own catalog
   snapshot, so the browser is shared UI while the catalog stays per boss. */
struct COMPOSITION_RESOURCE_TREE_NODE final
{
	std::string strSegment;
	std::string strStablePath;
	std::vector<std::size_t> LeafIndices;
	std::vector<COMPOSITION_RESOURCE_TREE_NODE> Children;
	std::size_t iRecursiveLeafCount = 0u;
};

void InsertResourceTree(
	COMPOSITION_RESOURCE_TREE_NODE& Root,
	const std::vector<std::string>& CategorySegments,
	std::size_t iLeafIndex);
std::size_t FinalizeResourceTree(COMPOSITION_RESOURCE_TREE_NODE& Node);
void RenderResourceTree(
	const COMPOSITION_RESOURCE_TREE_NODE& Node,
	const std::function<void(std::size_t)>& RenderLeaf);

NS_END
