#pragma once

#include "Client_Defines.h"
#include "NavGrid.h"

#include <string>
#include <vector>

#ifdef _DEBUG
NS_BEGIN(Client)

// Read-only inspection of published files. This is never a movement query owner.
class CLevelNavigationDebug final
{
public:
	void Sync_Level(LEVEL level);
	void Render_Controls();
	void Render_Overlay();

private:
	enum class CELL_KIND : uint8_t { WALKABLE, BLOCKED, NO_SURFACE, UNKNOWN_BLOCKED };
	struct GRID final
	{
		std::string name;
		Engine::CNavGrid runtime;
		std::vector<CELL_KIND> kinds;
		std::vector<float> missingSurfaceDisplayHeights;
		float maximumStepHeight = 0.f;
		uint32_t walkableCount = 0;
		uint32_t blockedCount = 0;
		uint32_t noSurfaceCount = 0;
		bool sourceMatched = false;
		std::string sourceStatus;
	};
	bool Reload();
	size_t Selected_Grid(float cameraX, float cameraZ) const;

	LEVEL m_Level = LEVEL::END;
	std::string m_AreaId;
	std::string m_Status;
	std::vector<GRID> m_Grids;
	bool m_Show = false;
	bool m_ShowMissing = true;
	int m_SelectedGrid = -1;
	float m_Radius = 80.f;
	float m_EffectiveRadius = 0.f;
	uint32_t m_DrawnCells = 0;
	uint32_t m_VisibleOmittedCells = 0;
	uint32_t m_SupersededCells = 0;
	// Per-frame diagnostics shown by Render_Controls (values from the last Render_Overlay).
	struct DRAWN_CELL_SAMPLE final
	{
		bool valid = false;
		int cellX = 0;
		int cellZ = 0;
		float worldX = 0.f;
		float worldZ = 0.f;
		float height = 0.f;
		float screenX = 0.f;
		float screenY = 0.f;
	};
	float3_t m_CameraPosition{};
	uint32_t m_FrustumRejectedCells = 0;
	DRAWN_CELL_SAMPLE m_FirstDrawnCell;
};

NS_END
#endif
