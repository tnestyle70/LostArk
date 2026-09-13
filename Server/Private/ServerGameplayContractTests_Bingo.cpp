#include "ServerGameplayContractTests_Runner.h"
#include "Network/PacketMessages.h"
#include "KoukuSaydonLogicRuntime.h"
#include "ServerGameplayContractTests.h"
#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"
#include "WorldDestructionBootstrapContractTests.h"
#include <Windows.h>
#include <process.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <span>
#include <sstream>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>


using namespace LostArk::Server;
using namespace LostArk::Shared;



int LostArk::Server::Run_ServerBingoContractTests()
{
	using namespace LostArk::Shared;
	TESTS tests;
	constexpr float HALF = KOUKU_BINGO_CELL_SIZE_M * 0.5f;

	/* Every authored tile centre has to answer with its own index, otherwise
	the board constants do not describe the floor that is actually placed. */
	bool centresRoundTrip = true;
	for (std::int32_t cell = 0; cell < KOUKU_BINGO_CELL_COUNT; ++cell)
	{
		if (Kouku_BingoCellAt(Kouku_BingoCellCenterX(cell),
			Kouku_BingoCellCenterZ(cell)) != cell)
		{
			centresRoundTrip = false;
			break;
		}
	}
	tests.Require(centresRoundTrip, "Every bingo tile centre resolves to its own cell");

	tests.Require(Kouku_BingoCellAt(-6.08f, 1140.8f) == 0 &&
		Kouku_BingoCellAt(6.08f, 1152.96f) == 24 &&
		Kouku_BingoCellAt(0.f, 1146.88f) == 12,
		"The two authored corners and the board centre are cells 0, 24 and 12");

	/* Row runs along +Z and column along +X. Swapping them still round trips
	on the centres, so the axes are pinned separately. */
	tests.Require(Kouku_BingoCellAt(Kouku_BingoCellCenterX(0) + KOUKU_BINGO_CELL_SIZE_M,
			Kouku_BingoCellCenterZ(0)) == 1 &&
		Kouku_BingoCellAt(Kouku_BingoCellCenterX(0),
			Kouku_BingoCellCenterZ(0) + KOUKU_BINGO_CELL_SIZE_M) == 5,
		"One cell along +X advances the column and one along +Z advances the row");

	/* Just inside each outer edge is still on the board; just outside is not. */
	const float minX = KOUKU_BINGO_ORIGIN_X - HALF;
	const float minZ = KOUKU_BINGO_ORIGIN_Z - HALF;
	const float maxX = Kouku_BingoCellCenterX(KOUKU_BINGO_SIDE - 1) + HALF;
	const float maxZ = Kouku_BingoCellCenterZ(KOUKU_BINGO_CELL_COUNT - 1) + HALF;
	tests.Require(Kouku_BingoCellAt(minX + 0.01f, minZ + 0.01f) == 0 &&
		Kouku_BingoCellAt(maxX - 0.01f, maxZ - 0.01f) == 24,
		"The inside of the outer edge belongs to the corner cells");
	tests.Require(Kouku_BingoCellAt(minX - 0.01f, 1146.88f) == KOUKU_BINGO_OFF_BOARD &&
		Kouku_BingoCellAt(maxX + 0.01f, 1146.88f) == KOUKU_BINGO_OFF_BOARD &&
		Kouku_BingoCellAt(0.f, minZ - 0.01f) == KOUKU_BINGO_OFF_BOARD &&
		Kouku_BingoCellAt(0.f, maxZ + 0.01f) == KOUKU_BINGO_OFF_BOARD,
		"A step past any outer edge is off the board");

	/* The card maze sits far along +Z in the same world; it must not read as
	a bingo cell. A NaN must not truncate into an arbitrary index either. */
	tests.Require(Kouku_BingoCellAt(0.28f, 1351.65f) == KOUKU_BINGO_OFF_BOARD &&
		Kouku_BingoCellAt(std::numeric_limits<float>::quiet_NaN(), 1146.88f) ==
			KOUKU_BINGO_OFF_BOARD,
		"A far position and a NaN both report off board instead of a cell");

	/* The cross the bomb paints: centre, up, down, left, right. */
	tests.Require(Kouku_BingoNeighbour(12, -1, 0) == 7 &&
		Kouku_BingoNeighbour(12, 1, 0) == 17 &&
		Kouku_BingoNeighbour(12, 0, -1) == 11 &&
		Kouku_BingoNeighbour(12, 0, 1) == 13,
		"The cross around the middle cell is 7, 17, 11 and 13");

	/* A column step at the edge must leave the board rather than wrap into
	the next row, which plain index arithmetic would do. */
	tests.Require(Kouku_BingoNeighbour(4, 0, 1) == KOUKU_BINGO_OFF_BOARD &&
		Kouku_BingoNeighbour(5, 0, -1) == KOUKU_BINGO_OFF_BOARD &&
		Kouku_BingoNeighbour(0, -1, 0) == KOUKU_BINGO_OFF_BOARD &&
		Kouku_BingoNeighbour(24, 1, 0) == KOUKU_BINGO_OFF_BOARD,
		"An edge step leaves the board instead of wrapping into the next row");
	tests.Require(Kouku_BingoNeighbour(KOUKU_BINGO_OFF_BOARD, 0, 1) == KOUKU_BINGO_OFF_BOARD &&
		Kouku_BingoNeighbour(KOUKU_BINGO_CELL_COUNT, 0, 0) == KOUKU_BINGO_OFF_BOARD,
		"Stepping from a cell that is not on the board stays off the board");

	/* G2/G3: the board state, the twelve lines and the safe judgement. */
	{
		std::uint32_t seen = 0u;
		bool lineCellsAreFive = true;
		for (std::int32_t line = 0; line < KOUKU_BINGO_LINE_COUNT; ++line)
		{
			const std::uint32_t mask = Kouku_BingoLineMask(line);
			seen |= mask;
			std::int32_t cells = 0;
			for (std::uint32_t bit = mask; 0u != bit; bit &= bit - 1u)
				++cells;
			if (5 != cells || 0u != (mask & ~KOUKU_BINGO_ALL_CELLS_MASK))
				lineCellsAreFive = false;
		}
		tests.Require(lineCellsAreFive && KOUKU_BINGO_ALL_CELLS_MASK == seen,
			"Twelve bingo lines each hold five on-board cells and together cover the board");
		tests.Require(Kouku_BingoLineMask(0) == 0x1Fu &&
			Kouku_BingoLineMask(KOUKU_BINGO_SIDE) == 0x108421u &&
			Kouku_BingoLineMask(KOUKU_BINGO_SIDE * 2) == 0x1041041u &&
			Kouku_BingoLineMask(KOUKU_BINGO_SIDE * 2 + 1) == 0x111110u &&
			Kouku_BingoLineMask(KOUKU_BINGO_LINE_COUNT) == 0u,
			"Row, column and both diagonal line masks are the expected cells");
	}
	{
		CKoukuBingoRuntime board;
		/* The Debug check the user drives: Play1 paints 0,1,2 and Play2 paints
		3,4, which completes row 0 and turns those five red. */
		board.Fill(0x7u);
		tests.Require(0x7u == board.Get_WhiteMask() && 0u == board.Get_RedMask(),
			"Three cells of a row are white and no line has completed yet");
		board.Fill(0x18u);
		tests.Require(0x1Fu == board.Get_WhiteMask() && 0x1Fu == board.Get_RedMask(),
			"Completing the fifth cell of a row turns that whole row red");
		tests.Require(board.Is_Safe(Kouku_BingoCellCenterX(2), Kouku_BingoCellCenterZ(2)) &&
			!board.Is_Safe(Kouku_BingoCellCenterX(12), Kouku_BingoCellCenterZ(12)) &&
			!board.Is_Safe(0.28f, 1351.65f),
			"A completed cell is safe while an unfinished cell and a position off the board are not");
		board.Reset();
		tests.Require(0u == board.Get_WhiteMask() && 0u == board.Get_RedMask() &&
			!board.Is_Safe(Kouku_BingoCellCenterX(2), Kouku_BingoCellCenterZ(2)),
			"Reset clears both masks and with them the safe cells");
	}
	{
		CKoukuBingoRuntime board;
		/* A diagonal counts, which is the rule the user chose. */
		board.Fill(0x1041041u);
		tests.Require(0x1041041u == board.Get_RedMask(),
			"A completed diagonal turns red like a row or a column");
		board.Fill(~0u);
		tests.Require(KOUKU_BINGO_ALL_CELLS_MASK == board.Get_WhiteMask() &&
			KOUKU_BINGO_ALL_CELLS_MASK == board.Get_RedMask(),
			"Bits above the board are dropped and a full board completes every line");
	}
	{
		S2C_WORLD_SNAPSHOT snapshot{};
		snapshot.iServerTick = 7u;
		snapshot.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
		GameplayDataRevision revision{};
		revision.Bytes.front() = 1u;
		snapshot.ActiveGameplayRevision = revision;
		PLAYER_SNAPSHOT player{};
		player.iNetEntityId = 41u;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		snapshot.Players.push_back(player);
		snapshot.Bingo.iWhiteMask = 0x1Fu;
		snapshot.Bingo.iRedMask = 0x1Fu;
		CPacketWriter writer;
		tests.Require(Write_Message(writer, snapshot), "Bingo board snapshot writes");
		CPacketReader reader{ writer.Get_Buffer() };
		S2C_WORLD_SNAPSHOT decoded{};
		tests.Require(Read_Message(reader, decoded) && 0u == reader.Get_RemainingSize() &&
			0x1Fu == decoded.Bingo.iWhiteMask && 0x1Fu == decoded.Bingo.iRedMask,
			"Bingo board masks round trip on the world snapshot");
		snapshot.Bingo.iRedMask = 0x3Fu;
		CPacketWriter strayRed;
		tests.Require(!Write_Message(strayRed, snapshot) && strayRed.Get_Buffer().empty(),
			"A red cell that is not white refuses the whole snapshot before writing");
		snapshot.Bingo.iRedMask = 0x1Fu;
		snapshot.Bingo.iWhiteMask = 0x2000000u;
		CPacketWriter strayCell;
		tests.Require(!Write_Message(strayCell, snapshot) && strayCell.Get_Buffer().empty(),
			"A bit above the twenty-fifth cell refuses the whole snapshot before writing");
	}
	{
		C2S_DEBUG_BINGO_FILL fill{};
		fill.iRequestSequence = 3u;
		fill.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
		fill.iCellMask = 0x18u;
		fill.bReset = true;
		CPacketWriter writer;
		tests.Require(Write_Message(writer, fill), "Bingo debug fill writes");
		CPacketReader reader{ writer.Get_Buffer() };
		C2S_DEBUG_BINGO_FILL decoded{};
		tests.Require(Read_Message(reader, decoded) &&
			3u == decoded.iRequestSequence && 0x18u == decoded.iCellMask && decoded.bReset,
			"Bingo debug fill round trips its cells and reset flag");
		fill.iCellMask = 0x2000000u;
		CPacketWriter strayCell;
		tests.Require(!Write_Message(strayCell, fill) && strayCell.Get_Buffer().empty(),
			"A fill outside the board refuses before writing");
	}
	{
		/* The bomb clock. One mark per carrier, planted where the room says
		the carrier was, and the slot freed only by an explicit clear. */
		CKoukuBingoRuntime bombs;
		tests.Require(bombs.Start_Bomb(41u, 90u) &&
			BINGO_BOMB_PHASE::MARKED == bombs.Get_Bombs()[0].ePhase &&
			41u == bombs.Get_Bombs()[0].iCarrierNetEntityId &&
			90u == bombs.Get_Bombs()[0].iDetonateTick,
			"Starting a bomb marks the carrier with its detonate tick");
		tests.Require(!bombs.Start_Bomb(41u, 120u) &&
			90u == bombs.Get_Bombs()[0].iDetonateTick,
			"A second mark on the same carrier is refused and keeps the first clock");
		tests.Require(!bombs.Start_Bomb(INVALID_NET_ENTITY_ID, 120u),
			"A bomb without a carrier is refused");
		/* The cross a bomb paints, including the edges it has to drop. */
		tests.Require(Kouku_BingoCrossMask(12) ==
			((1u << 12) | (1u << 7) | (1u << 17) | (1u << 11) | (1u << 13)),
			"A centre bomb paints its own cell and all four neighbours");
		tests.Require(Kouku_BingoCrossMask(0) ==
			((1u << 0) | (1u << 1) | (1u << 5)),
			"A corner bomb drops the two neighbours that fall off the board");
		tests.Require(0u == Kouku_BingoCrossMask(KOUKU_BINGO_OFF_BOARD),
			"A bomb that missed the board paints nothing");
		bombs.Plant_Bomb(0u, 1.5f, 1141.f, 180u);
		tests.Require(BINGO_BOMB_PHASE::PLANTED == bombs.Get_Bombs()[0].ePhase &&
			INVALID_NET_ENTITY_ID == bombs.Get_Bombs()[0].iCarrierNetEntityId &&
			1.5f == bombs.Get_Bombs()[0].fPositionX &&
			1141.f == bombs.Get_Bombs()[0].fPositionZ &&
			180u == bombs.Get_Bombs()[0].iDetonateTick,
			"Planting drops the carrier and keeps the position and fuse it was given");
		bombs.Plant_Bomb(0u, 9.f, 9.f, 200u);
		tests.Require(1.5f == bombs.Get_Bombs()[0].fPositionX,
			"Planting an already planted bomb does not move it");
		/* One cross, then the row it completes, through the same Fill the
		board buttons use. */
		bombs.Fill(Kouku_BingoCrossMask(Kouku_BingoCellAt(
			Kouku_BingoCellCenterX(1), Kouku_BingoCellCenterZ(1))));
		tests.Require(bombs.Get_WhiteMask() ==
			((1u << 1) | (1u << 0) | (1u << 2) | (1u << 6)) &&
			0u == bombs.Get_RedMask(),
			"A bomb on cell 1 paints four cells and completes no line");
		bombs.Fill(Kouku_BingoCrossMask(3));
		tests.Require(0x1Fu == (bombs.Get_RedMask() & 0x1Fu),
			"A second cross that finishes row 0 turns that row red");
		/* A blast toggles: empty lights, white clears, red is untouchable. */
		CKoukuBingoRuntime blast;
		blast.Detonate(Kouku_BingoCrossMask(12));
		const std::uint32_t cross12 = Kouku_BingoCrossMask(12);
		tests.Require(cross12 == blast.Get_WhiteMask() && 0u == blast.Get_RedMask(),
			"A blast on an empty board lights its whole cross");
		blast.Detonate(cross12);
		tests.Require(0u == blast.Get_WhiteMask(),
			"A second blast on the same cross clears every cell it lit");
		blast.Detonate(Kouku_BingoCrossMask(11));
		const std::uint32_t before = blast.Get_WhiteMask();
		blast.Detonate(Kouku_BingoCrossMask(13));
		tests.Require(before != blast.Get_WhiteMask() &&
			0u == (blast.Get_WhiteMask() & (1u << 12)),
			"An overlapping blast clears the cells the two crosses share");
		blast.Reset();
		blast.Fill(0x1Fu);
		tests.Require(0x1Fu == blast.Get_RedMask(), "Row 0 is red before the blast");
		blast.Detonate(Kouku_BingoCrossMask(2));
		tests.Require(0x1Fu == (blast.Get_WhiteMask() & 0x1Fu) &&
			0x1Fu == blast.Get_RedMask(),
			"A blast never clears a cell that belongs to a completed line");
		tests.Require(0u != (blast.Get_WhiteMask() & (1u << 7)),
			"The same blast still lights the empty cell below the red row");
		tests.Require(bombs.Start_Bomb(41u, 150u) &&
			BINGO_BOMB_PHASE::MARKED == bombs.Get_Bombs()[1].ePhase,
			"The same carrier can be marked again once its bomb is planted");
		bombs.Reset();
		tests.Require(BINGO_BOMB_PHASE::NONE == bombs.Get_Bombs()[0].ePhase &&
			BINGO_BOMB_PHASE::NONE == bombs.Get_Bombs()[1].ePhase,
			"Resetting the board clears every bomb slot");
	}
	{
		/* Both phases have to survive the wire, and a mark that lost its
		carrier must be refused rather than drawn on nobody. */
		S2C_WORLD_SNAPSHOT snapshot{};
		snapshot.iServerTick = 7u;
		snapshot.eWorldId = WORLD_ID::KAKULSAYDON_ARENA;
		GameplayDataRevision revision{};
		revision.Bytes.front() = 1u;
		snapshot.ActiveGameplayRevision = revision;
		PLAYER_SNAPSHOT player{};
		player.iNetEntityId = 41u;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		snapshot.Players.push_back(player);
		snapshot.Bingo.iBombCount = 2u;
		snapshot.Bingo.Bombs[0].ePhase = BINGO_BOMB_PHASE::MARKED;
		snapshot.Bingo.Bombs[0].iCarrierNetEntityId = 41u;
		snapshot.Bingo.Bombs[1].ePhase = BINGO_BOMB_PHASE::PLANTED;
		snapshot.Bingo.Bombs[1].fPositionX = -3.04f;
		snapshot.Bingo.Bombs[1].fPositionZ = 1143.84f;
		CPacketWriter writer;
		tests.Require(Write_Message(writer, snapshot), "Bingo bombs write");
		CPacketReader reader{ writer.Get_Buffer() };
		S2C_WORLD_SNAPSHOT decoded{};
		tests.Require(Read_Message(reader, decoded) &&
			2u == decoded.Bingo.iBombCount &&
			BINGO_BOMB_PHASE::MARKED == decoded.Bingo.Bombs[0].ePhase &&
			41u == decoded.Bingo.Bombs[0].iCarrierNetEntityId &&
			BINGO_BOMB_PHASE::PLANTED == decoded.Bingo.Bombs[1].ePhase &&
			-3.04f == decoded.Bingo.Bombs[1].fPositionX &&
			1143.84f == decoded.Bingo.Bombs[1].fPositionZ,
			"Both bomb phases round trip on the world snapshot");
		snapshot.Bingo.Bombs[0].iCarrierNetEntityId = INVALID_NET_ENTITY_ID;
		CPacketWriter orphan;
		tests.Require(!Write_Message(orphan, snapshot) && orphan.Get_Buffer().empty(),
			"A mark without a carrier refuses the whole snapshot before writing");
	}
	std::cout << "bingo failures: " << tests.failures << '\n';
	return tests.failures == 0 ? 0 : 1;
}
