#include "ServerGameplayContractTests_Runner.h"
#include "Network/PacketMessages.h"
#include "KoukuSaydonLogicRuntime.h"
#include "GameRoom.h"
#include <bit>
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
		/* A blast paints empty cells, promotes ordinary skulls, and preserves red. */
		CKoukuBingoRuntime blast;
		blast.Detonate(Kouku_BingoCrossMask(12));
		const std::uint32_t cross12 = Kouku_BingoCrossMask(12);
		tests.Require(cross12 == blast.Get_WhiteMask() && 0u == blast.Get_RedMask(),
			"A blast on an empty board lights its whole cross");
		blast.Detonate(cross12);
		tests.Require(cross12 == blast.Get_WhiteMask() && cross12 == blast.Get_RedMask(),
			"A second blast promotes all existing ordinary skulls to red");
		blast.Detonate(Kouku_BingoCrossMask(11));
		const std::uint32_t before = blast.Get_WhiteMask();
		blast.Detonate(Kouku_BingoCrossMask(13));
		tests.Require(before != blast.Get_WhiteMask() &&
			0u != (blast.Get_RedMask() & (1u << 12)),
			"An overlapping blast preserves and promotes the shared skulls");
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

#ifdef _DEBUG
    {
        auto room = std::make_unique<CGameRoom>(WORLD_ID::KAKULSAYDON_ARENA);
        auto& audition = room->m_KoukuSaydonPatternAudition;
        audition.ePhase = CGameRoom::KOUKUSAYDON_PATTERN_AUDITION_PHASE::ACTIVE;
        audition.iRoomAuditionEpoch = 71u;
        audition.PinnedGameplayRevision = room->m_GameplayCatalog.Get_ActiveRevision();
        audition.pProductGeneration = std::make_shared<CGameplayCatalog>(room->m_GameplayCatalog.Active());
        audition.iPinnedSourceRevision = CKoukuSaydonBrain::Resolve_ProductSourceRevision(*audition.pProductGeneration);
        tests.Require(room->Resolve_KoukuProductCatalog() != nullptr, "Bingo hit results use the exact admitted Product source revision");
        const auto* placement = room->Find_Placement("boss.kakulsaydon.bingo.saydon");
        SERVER_WORLD_ENTITY boss;
        const bool ready = placement && room->Build_WorldEntity(*placement, 900u, boss);
        tests.Require(ready, "Bingo duration uses an admitted real Server boss and navigation catalog");
        if (ready)
        {
            boss.iPatternSequence = 1u; boss.strPatternId = "test.bingo.duration";
            room->m_WorldEntities.clear(); room->m_WorldEntities.push_back(boss);
            auto& owner = room->m_WorldEntities.front();
            auto& player = room->m_Players[1u];
            player.iPlayerId = 1u; player.iNetEntityId = 101u; player.isCombatReady = true;
            player.iCurrentHp = player.iMaximumHp = 100u; player.iMaximumMadness = 100u;
            player.fPositionX = Kouku_BingoCellCenterX(12) + .4f;
            player.fPositionZ = Kouku_BingoCellCenterZ(12) - .4f;
            BOSS_PATTERN_MECHANIC_TRIGGER trigger; trigger.eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BINGO_BOARD;
            trigger.iDurationMs = 50000u;
            constexpr std::uint32_t start = 1000u;
            room->Begin_KoukuBingoDuration(owner, trigger, start);
            tests.Require(std::popcount(room->m_KoukuBingo.Get_WhiteMask()) == 2 && room->m_KoukuBingo.Get_RedMask() == 0u,
                "Bingo begins with exactly two distinct ordinary skulls");
            room->m_KoukuBingo.Reset(); room->m_KoukuBingo.Fill(1u << 12u);
            room->Update_KoukuBingo(start + 29u);
            tests.Require(player.iCurrentMadness == 0u, "Ordinary skull madness waits for the one-second boundary");
            room->Update_KoukuBingo(start + 30u);
            tests.Require(player.iCurrentMadness == 3u, "Ordinary skull adds exactly three madness per second");
            room->m_KoukuBingo.Detonate(1u << 12u);
            room->Update_KoukuBingo(start + 60u);
            tests.Require(player.iCurrentMadness == 6u, "Red skull adds the same three madness per second");
            const auto countPhase = [&](BINGO_BOMB_PHASE phase) { return std::count_if(room->m_KoukuBingo.Get_Bombs().begin(), room->m_KoukuBingo.Get_Bombs().end(), [&](const auto& b) { return b.ePhase == phase; }); };
            room->Update_KoukuBingo(start + 149u);
            tests.Require(countPhase(BINGO_BOMB_PHASE::MARKED) == 0, "No automatic head mark before five seconds");
            room->Update_KoukuBingo(start + 150u);
            tests.Require(countPhase(BINGO_BOMB_PHASE::MARKED) == 1 && room->m_KoukuBingo.Get_Bombs()[0].iDetonateTick == start + 300u,
                "Five-second head mark receives a separate five-second planting deadline");
            room->Update_KoukuBingo(start + 300u);
            tests.Require(countPhase(BINGO_BOMB_PHASE::MARKED) == 1 && countPhase(BINGO_BOMB_PHASE::PLANTED) == 1,
                "Solo player plants and receives the next mark at exactly ten seconds");
            const auto planted = std::find_if(room->m_KoukuBingo.Get_Bombs().begin(), room->m_KoukuBingo.Get_Bombs().end(), [](const auto& b) { return b.ePhase == BINGO_BOMB_PHASE::PLANTED; });
            tests.Require(planted->iDetonateTick == start + 390u && planted->fPositionX == Kouku_BingoCellCenterX(12) && planted->fPositionZ == Kouku_BingoCellCenterZ(12),
                "Planting snaps to the player's grid cell centre and retains a three-second fuse");
            const auto h0 = room->m_KoukuBingoDuration.Hammers[0].anchor, h1 = room->m_KoukuBingoDuration.Hammers[1].anchor;
            tests.Require(h0 >= 0 && h1 >= 0 && h0 / 10 == h1 / 10 && std::abs(h0 / 2 - h1 / 2) >= 2,
                "Ten-second hammers choose two distinct parallel paths without crossing");
            room->m_KoukuBingo.Fill(1u << 11u);
            room->Update_KoukuBingo(start + 389u);
            tests.Require(countPhase(BINGO_BOMB_PHASE::PLANTED) == 1 && !(room->m_KoukuBingo.Get_RedMask() & (1u << 11u)),
                "Bomb cannot paint or recolour before its fuse deadline");
            room->Update_KoukuBingo(start + 390u);
            const auto cross = Kouku_BingoCrossMask(12);
            tests.Require((room->m_KoukuBingo.Get_WhiteMask() & cross) == cross && (room->m_KoukuBingo.Get_RedMask() & (1u << 11u)),
                "Detonation paints centre and four neighbours and promotes an existing ordinary skull");
            room->m_KoukuBingoDuration.Hammers = {}; // Subsequent timing checks isolate bomb scheduling.
            room->Update_KoukuBingo(start + 450u);
            tests.Require(countPhase(BINGO_BOMB_PHASE::MARKED) == 1 && countPhase(BINGO_BOMB_PHASE::PLANTED) == 1,
                "Solo player receives the third consecutive head mark at fifteen seconds");
            for (std::size_t i = 0; i < room->m_KoukuBingo.Get_Bombs().size(); ++i) room->m_KoukuBingo.Clear_Bomb(i);
            room->m_KoukuBingo.Start_Bomb(player.iNetEntityId, start + 1500u);
            room->m_KoukuBingoDuration.iNextBombTick = start + 1500u;
            room->Update_KoukuBingo(start + 1500u);
            const auto retained = room->m_KoukuBingo.Get_WhiteMask();
            tests.Require(room->m_KoukuBingoDuration.iOwnerId == 0u && countPhase(BINGO_BOMB_PHASE::PLANTED) == 1,
                "The fifty-second duration ends while a final marked bomb keeps its planting/fuse tail");
            room->Begin_KoukuBingoDuration(owner, trigger, start + 1501u);
            tests.Require(room->m_KoukuBingo.Get_WhiteMask() == retained && countPhase(BINGO_BOMB_PHASE::PLANTED) == 1,
                "Repeating the Parent preserves board state and pending bomb tails within the same run");
            room->Update_KoukuBingo(start + 1501u);
            tests.Require(countPhase(BINGO_BOMB_PHASE::MARKED) == 1 && countPhase(BINGO_BOMB_PHASE::PLANTED) == 1,
                "Repeating the Parent resumes the due fifty-second mark without a new five-second delay");
            room->Update_KoukuBingo(start + 1590u);
            tests.Require(countPhase(BINGO_BOMB_PHASE::PLANTED) == 0, "A previous-loop bomb completes its fuse during the next Parent");

            // At the authored hammer head; the raised chain must never deal damage.
            auto& hammer = room->m_KoukuBingoDuration.Hammers[0]; hammer.anchor = 4; hammer.startTick = 4000u;
            room->m_KoukuBingoDuration.iEndTick = 5000u;
            room->m_KoukuBingoDuration.iNextHammerTick = 5000u;
            room->m_KoukuBingoDuration.iNextBombTick = 5000u;
            room->m_KoukuBingoDuration.iNextMadnessTick = 5000u;
            player.iCurrentHp = 100u; player.eAction = PLAYER_ACTION_STATE::NONE;
            player.fPositionX = 0.f; player.fPositionZ = Kouku_BingoCellCenterZ(12);
            room->Update_KoukuBingo(4089u);
            tests.Require(player.iCurrentHp == 100u, "Red-arrow warning has no hammer damage before three seconds");
            room->Update_KoukuBingo(4131u);
            tests.Require(player.iCurrentHp == 100u, "Descending hammer chain has no premature ground hit");
            room->Update_KoukuBingo(4156u);
            tests.Require(player.iCurrentHp == 0u && player.eAction == PLAYER_ACTION_STATE::DEAD,
                "The swept original-sized ground hammer head applies Server instant death");
            room->Stop_KoukuBingoDuration(true);
            tests.Require(room->m_KoukuBingo.Get_WhiteMask() == 0u && room->m_KoukuBingo.Get_RedMask() == 0u && countPhase(BINGO_BOMB_PHASE::MARKED) == 0,
                "Leaving or restarting the run clears board, marks, and periodic ownership");

            // Native hook reaches the last authored grip before its visual tail ends.
            player.iCurrentHp = 100u; player.eAction = PLAYER_ACTION_STATE::GRABBED;
            player.iAttachmentOwnerNetEntityId = owner.iNetEntityId; player.iAttachmentPatternSequence = owner.iPatternSequence;
            player.eAttachmentSlot = PLAYER_ATTACHMENT_SLOT::WORLD_HOOK_TIP;
            player.iAttachmentWindowIndex = player.iAttachmentRegionIndex = 0u; player.iAttachmentReleaseTick = 7000u;
            BOSS_PATTERN_DEFINITION hookPattern; hookPattern.strPatternId = owner.strPatternId;
            auto& window = hookPattern.LogicWindows.emplace_back(); window.iDurationMs = 4000u;
            auto& region = window.CardRegions.emplace_back(); region.WorldTrack.bEnabled = true; region.WorldTrack.iDurationMs = 3000u;
            auto& keys = region.WorldTrack.Keys; keys.resize(3u);
            keys[0].bHasGripPosition = keys[1].bHasGripPosition = keys[2].bHasGripPosition = true;
            keys[0].GripPosition = {0.f, 0.f, 1146.88f}; keys[1].GripPosition = keys[2].GripPosition = {10.f, 0.f, 1146.88f};
            keys[1].iTimeMs = 1000u; keys[2].iTimeMs = 3000u; keys[2].bVisible = false;
            KOUKUSAYDON_LOGIC_LEDGER ledger; CKoukuSaydonLogicRuntime::Build(hookPattern, owner, 6000u, ledger);
            KOUKUSAYDON_LOGIC_OUTPUT output; std::vector<DAMAGE_EVENT> damage;
            CKoukuSaydonLogicRuntime::Update(owner, hookPattern, ledger, room->m_Players, room->m_GameplayCatalog, nullptr, 6029u, damage, output);
            tests.Require(player.iAttachmentReleaseTick == 7000u && player.fPositionX < 10.f,
                "Hook keeps the player attached while approaching its final grip");
            CKoukuSaydonLogicRuntime::Update(owner, hookPattern, ledger, room->m_Players, room->m_GameplayCatalog, nullptr, 6030u, damage, output);
            tests.Require(player.iAttachmentReleaseTick == 6030u && player.fPositionX == 10.f,
                "Hook releases at the exact final grip before its trailing visual hold");
            const auto remainedAttached = room->Update_PlayerAttachment(player, 6030u);
            tests.Require(!remainedAttached && player.eAttachmentSlot == PLAYER_ATTACHMENT_SLOT::NONE && player.eAction == PLAYER_ACTION_STATE::NONE,
                "Hook endpoint clears movement lock without a synthetic knockdown delay");
        }
    }
#endif
	std::cout << "bingo failures: " << tests.failures << '\n';
	return tests.failures == 0 ? 0 : 1;
}
