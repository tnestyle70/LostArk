#include "ServerGameplayContractTests_Runner.h"
#include "ServerApp.h"
#include "Network/PacketMessages.h"
#include "KoukuSaydonLogicRuntime.h"
#include "GameRoom.h"
#include "ServerCombatHitRuntime.h"
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
		/* Diagonal marks remain ordinary skulls; only rows and columns promote. */
		board.Fill(0x1041041u);
		tests.Require(0x1041041u == board.Get_WhiteMask() && !board.Get_RedMask(),
			"A completed diagonal never turns red or earns a completed line");
		board.Fill(~0u);
		tests.Require(KOUKU_BINGO_ALL_CELLS_MASK == board.Get_WhiteMask() &&
			KOUKU_BINGO_ALL_CELLS_MASK == board.Get_RedMask(),
			"Bits above the board are dropped and a full board completes every line");
		tests.Require(board.Count_CompletedRowsAndColumns() == 10u && board.Count_UnconsumedRowsAndColumns() == 10u &&
			board.Consume_CompletedRowsAndColumns(3u) && board.Count_UnconsumedRowsAndColumns() == 7u &&
			board.Consume_CompletedRowsAndColumns(3u) && board.Count_UnconsumedRowsAndColumns() == 4u,
			"Each reward consumes exactly three distinct rows/columns while preserving extra unused lines");
		tests.Require(!board.Consume_CompletedRowsAndColumns(5u) && !board.Consume_CompletedRowsAndColumns(0u) &&
			board.Count_UnconsumedRowsAndColumns() == 4u && board.Get_RedMask() == KOUKU_BINGO_ALL_CELLS_MASK,
			"Insufficient or zero-sized reward claims preserve every unused line and all permanent red cells");
		board.Reset(); board.Fill(0x7fffu);
		tests.Require(board.Count_UnconsumedRowsAndColumns() == 3u && board.Consume_CompletedRowsAndColumns(3u) &&
			!board.Count_UnconsumedRowsAndColumns() && !board.Consume_CompletedRowsAndColumns(3u),
			"A fresh board resets reward identity and the same three permanent lines cannot be reused");
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
		/* A blast toggles ordinary skulls and promotes only completed rows/columns. */
		CKoukuBingoRuntime blast;
		blast.Detonate(Kouku_BingoCrossMask(12));
		const std::uint32_t cross12 = Kouku_BingoCrossMask(12);
		tests.Require(cross12 == blast.Get_WhiteMask() && !blast.Get_RedMask(),
			"A blast on empty floor creates ordinary skulls without prematurely turning them red");
		blast.Detonate(cross12);
		tests.Require(!blast.Get_WhiteMask() && !blast.Get_RedMask(),
			"A second blast on the same unfinished cross removes its ordinary skulls");
		blast.Detonate(Kouku_BingoCrossMask(11));
		const std::uint32_t before = blast.Get_WhiteMask();
		blast.Detonate(Kouku_BingoCrossMask(13));
		tests.Require(blast.Get_WhiteMask() == (before ^ Kouku_BingoCrossMask(13)) &&
			!(blast.Get_WhiteMask() & (1u << 12)) && !blast.Get_RedMask(),
			"Overlapping crosses toggle the shared unfinished skull exactly once per blast");
		blast.Reset(); blast.Fill(1u << 12u); blast.Detonate(Kouku_BingoCrossMask(12));
		tests.Require(!(blast.Get_WhiteMask() & (1u << 12u)) && !(blast.Get_RedMask() & (1u << 12u)),
			"A bomb removes an existing ordinary skull while its empty neighbours become ordinary skulls");
		blast.Reset();
		blast.Fill(0x1Fu);
		tests.Require(0x1Fu == blast.Get_RedMask(), "Row 0 is red before the blast");
		blast.Detonate(Kouku_BingoCrossMask(2));
		tests.Require(0x1Fu == (blast.Get_WhiteMask() & 0x1Fu) &&
			0x1Fu == (blast.Get_RedMask() & 0x1Fu),
			"A blast never clears a cell that belongs to a completed line");
		tests.Require(0u != (blast.Get_WhiteMask() & (1u << 7)),
			"The same blast still lights the empty cell below the red row");
		blast.Reset(); blast.Fill((1u << 1u) | (1u << 2u) | (1u << 3u) | (1u << 4u));
		blast.Detonate(Kouku_BingoCrossMask(0));
		tests.Require(blast.Get_WhiteMask() == ((1u << 0u) | (1u << 2u) | (1u << 3u) | (1u << 4u) | (1u << 5u)) &&
			!blast.Get_RedMask(),
			"With a2-a5 black, an a1 bomb toggles all affected cells first: a1 black, a2 empty and no transient red row");
		blast.Detonate(Kouku_BingoCrossMask(6));
		tests.Require((blast.Get_WhiteMask() & Kouku_BingoLineMask(0)) == Kouku_BingoLineMask(0) &&
			blast.Get_RedMask() == Kouku_BingoLineMask(0),
			"The following b2 bomb restores a2 and only then promotes the completed a row to permanent red");
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
        if (room->Is_Ready()) CServerGameplayContractRunner::Run_InannaProtection(tests, room->m_GameplayCatalog.Active());
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
            const auto* bingoPatterns = audition.pProductGeneration->Find_BossPatterns("ENCOUNTER_KAKULSAYDON_G1");
            const BOSS_PATTERN_MECHANIC_TRIGGER* publishedBoard = nullptr;
            if (bingoPatterns) for (const auto& pattern : *bingoPatterns)
                for (const auto& candidate : pattern.MechanicTriggers)
                    if (candidate.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BINGO_BOARD && candidate.BingoHammerHalfExtentsM)
                        publishedBoard = &candidate;
            tests.Require(publishedBoard != nullptr, "Bingo duration fixture loads its actual published hammer geometry");
            if (publishedBoard) trigger = *publishedBoard;
            trigger.iDurationMs = 50000u;
            constexpr std::uint32_t start = 1000u;
            room->Begin_KoukuBingoDuration(owner, trigger, start);
            tests.Require(std::popcount(room->m_KoukuBingo.Get_WhiteMask()) == 2 && room->m_KoukuBingo.Get_RedMask() == 0u,
                "Bingo begins with exactly two distinct ordinary skulls");
            room->m_KoukuBingo.Reset(); room->m_KoukuBingo.Fill(1u << 12u);
            room->Update_KoukuBingo(start + 29u);
            tests.Require(player.iCurrentMadness == 0u, "Ordinary skull madness waits for the one-second boundary");
            room->Update_KoukuBingo(start + 30u);
            tests.Require(player.iCurrentMadness == 5u, "Ordinary skull adds five percent madness per second");
            room->m_KoukuBingo.Reset(); room->m_KoukuBingo.Fill(Kouku_BingoLineMask(2));
            room->Update_KoukuBingo(start + 60u);
            tests.Require(player.iCurrentMadness == 10u, "Red skull adds the same five percent madness per second");
            player.iMaximumMadness = 200u;
            room->Update_KoukuBingo(start + 90u);
            tests.Require(player.iCurrentMadness == 20u, "Skull madness scales with the player's maximum instead of adding fixed points");
            player.iMaximumMadness = 100u;
            room->m_KoukuBingo.Reset();
            const auto countPhase = [&](BINGO_BOMB_PHASE phase) { return std::count_if(room->m_KoukuBingo.Get_Bombs().begin(), room->m_KoukuBingo.Get_Bombs().end(), [&](const auto& b) { return b.ePhase == phase; }); };
            room->m_KoukuBingoDuration.bEncounterOwned = true;
            room->m_KoukuBingoDuration.iEndTick = 0u;
            room->m_KoukuBingoDuration.iNextMadnessTick = 99999u;
            room->m_KoukuBingoDuration.iNextHammerTick = 99999u;
            room->Update_KoukuBingo(start + 899u);
            tests.Require(countPhase(BINGO_BOMB_PHASE::MARKED) == 0, "The first mark waits ten seconds plus twenty seconds");
            room->Update_KoukuBingo(start + 900u);
            tests.Require(countPhase(BINGO_BOMB_PHASE::MARKED) == 1 && room->m_KoukuBingo.Get_Bombs()[0].iDetonateTick == start + 1080u,
                "The thirty-second mark lives exactly six seconds");
            room->Update_KoukuBingo(start + 1080u);
            tests.Require(countPhase(BINGO_BOMB_PHASE::MARKED) == 0 && countPhase(BINGO_BOMB_PHASE::PLANTED) == 0 &&
                room->m_KoukuBingo.Get_Bombs()[0].iPlantTick == start + 1140u,
                "Mark expiry captures a cell and hides the bomb for two seconds");
            player.fPositionX = Kouku_BingoCellCenterX(0); player.fPositionZ = Kouku_BingoCellCenterZ(0);
            room->Update_KoukuBingo(start + 1139u);
            tests.Require(countPhase(BINGO_BOMB_PHASE::PLANTED) == 0, "The captured bomb cannot plant before its two-second delay");
            room->Update_KoukuBingo(start + 1140u);
            const auto& planted = room->m_KoukuBingo.Get_Bombs()[0];
            tests.Require(planted.ePhase == BINGO_BOMB_PHASE::PLANTED && planted.iDetonateTick == start + 1260u &&
                planted.fPositionX == Kouku_BingoCellCenterX(12) && planted.fPositionZ == Kouku_BingoCellCenterZ(12),
                "Ground bomb uses the captured cell despite carrier movement and burns for four seconds");
            room->m_KoukuBingo.Fill(1u << 11u);
            room->Update_KoukuBingo(start + 1259u);
            tests.Require(countPhase(BINGO_BOMB_PHASE::PLANTED) == 1, "Bomb retains its full four-second fuse");
            room->Update_KoukuBingo(start + 1260u);
            const auto cross = Kouku_BingoCrossMask(12);
            tests.Require(countPhase(BINGO_BOMB_PHASE::PLANTED) == 0 &&
                (room->m_KoukuBingo.Get_WhiteMask() & cross) == (cross & ~(1u << 11u)) &&
                !(room->m_KoukuBingo.Get_WhiteMask() & (1u << 11u)),
                "The bomb changes its cross at mark plus twelve seconds");
            ++owner.iPatternSequence; owner.strPatternId = "different.normal.pattern";
            room->Update_KoukuBingo(start + 1500u);
            tests.Require(countPhase(BINGO_BOMB_PHASE::MARKED) == 1 && room->m_KoukuBingoDuration.iMarkedBombCount == 2u,
                "A different independent normal pattern cannot reset the fifty-second mark");
            room->Update_KoukuBingo(start + 1680u); room->Update_KoukuBingo(start + 1740u);
            room->Update_KoukuBingo(start + 1860u); room->Update_KoukuBingo(start + 2100u);
            tests.Require(room->m_KoukuBingoDuration.iMarkedBombCount == 3u && room->m_KoukuBingoDuration.bSpecialPatternPending,
                "The seventy-second third mark requests the authored special sequence once");
            room->m_KoukuBingo.Fill(0x7fffu);
            room->Update_KoukuBingo(start + 2280u); room->Update_KoukuBingo(start + 2340u);
            room->Update_KoukuBingo(start + 2460u);
            tests.Require(room->m_KoukuBingoDuration.iLastLineJudgementTick == start + 2460u && room->m_KoukuBingo.Count_CompletedRowsAndColumns() >= 3u,
                "The third bomb detonates at twelve seconds and snapshots completed red rows and columns after its flip");
            for (std::size_t slot = 0u; slot < room->m_KoukuBingo.Get_Bombs().size(); ++slot) room->m_KoukuBingo.Clear_Bomb(slot);
            room->m_KoukuBingoDuration.bSpecialPatternPending = false;
            player.iInvulnerableEndTick = 0u; // The following fixture isolates hammer contact damage.

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
            SERVER_NAV_POINT hookFloor{};
            tests.Require(room->m_ServerNavigation.Sample_Position(1.f, 1146.88f, hookFloor), "Hook release fixture has a real admitted floor");
            SERVER_NAV_POINT hookReleaseFloor{};
            tests.Require(room->m_ServerNavigation.Project_PointOnSameLevel(hookFloor.x, hookFloor.z, hookReleaseFloor, hookFloor.y),
                "Hook release fixture resolves the authoritative same-level landing projection");
            keys[0].GripPosition = {0.f, hookFloor.y, 1146.88f}; keys[1].GripPosition = keys[2].GripPosition = {hookFloor.x, hookFloor.y, hookFloor.z};
            keys[1].iTimeMs = 1000u; keys[2].iTimeMs = 3000u; keys[2].bVisible = false;
            KOUKUSAYDON_LOGIC_LEDGER ledger; CKoukuSaydonLogicRuntime::Build(hookPattern, owner, 6000u, ledger);
            KOUKUSAYDON_LOGIC_OUTPUT output; std::vector<DAMAGE_EVENT> damage;
            CKoukuSaydonLogicRuntime::Update(owner, hookPattern, ledger, room->m_Players, room->m_GameplayCatalog, nullptr, 6029u, damage, output, &room->m_ServerNavigation);
            tests.Require(player.iAttachmentReleaseTick == 7000u && player.fPositionX < hookFloor.x,
                "Hook keeps the player attached while approaching its final grip");
            CKoukuSaydonLogicRuntime::Update(owner, hookPattern, ledger, room->m_Players, room->m_GameplayCatalog, nullptr, 6030u, damage, output, &room->m_ServerNavigation);
            tests.Require(player.iAttachmentReleaseTick == 6030u && std::abs(player.fPositionX - hookReleaseFloor.x) < .001f,
                "Hook releases at the exact final grip before its trailing visual hold");
            const auto remainedAttached = room->Update_PlayerAttachment(player, 6030u);
            tests.Require(!remainedAttached && player.eAttachmentSlot == PLAYER_ATTACHMENT_SLOT::NONE && player.eAction == PLAYER_ACTION_STATE::NONE,
                "Hook endpoint clears movement lock without a synthetic knockdown delay");

            // The small special Parent owns one detonation Trigger after its Medusa and black hole.
            player.isCombatReady = true; player.iCurrentHp = 100u;
            player.fPositionX = Kouku_BingoCellCenterX(2); player.fPositionZ = Kouku_BingoCellCenterZ(2);
            auto& unsafe = room->m_Players[2u]; unsafe = player;
            unsafe.iPlayerId = 2u; unsafe.iNetEntityId = 102u;
            unsafe.fPositionX = Kouku_BingoCellCenterX(12); unsafe.fPositionZ = Kouku_BingoCellCenterZ(12);
            room->m_KoukuBingo.Reset(); room->m_KoukuBingo.Fill(Kouku_BingoLineMask(KOUKU_BINGO_SIDE * 2u));
            tests.Require(room->m_KoukuBingo.Count_CompletedRowsAndColumns() == 0u, "Diagonals never count toward the row/column reward");
            room->m_KoukuBingo.Reset(); room->m_KoukuBingo.Fill(0x1fu);
            owner.iMaximumHp = owner.iCurrentHp = 10000u; owner.iMaximumHealthBars = 100u;
            BOSS_PATTERN_DEFINITION linePattern; linePattern.strPatternId = owner.strPatternId;
            auto& lines = linePattern.LogicWindows.emplace_back(); lines.eKind = BOSS_PATTERN_LOGIC_KIND::BINGO_COMPLETED_LINES;
            lines.iDurationMs = 32628u; lines.iThreshold = 1u;
            BOSS_PATTERN_LOGIC_RESULT protect; protect.eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::PLAYER_INVULNERABILITY; protect.iDurationMs = 30000u;
            lines.OnSuccess.push_back(protect);
            KOUKUSAYDON_LOGIC_LEDGER lineLedger; CKoukuSaydonLogicRuntime::Build(linePattern, owner, 6100u, lineLedger);
            lineLedger.iBingoLineJudgementTick = 6200u; lineLedger.iBingoCompletedLines = room->m_KoukuBingo.Count_CompletedRowsAndColumns();
            KOUKUSAYDON_LOGIC_OUTPUT lineOutput;
            CKoukuSaydonLogicRuntime::Update(owner, linePattern, lineLedger, room->m_Players, room->m_GameplayCatalog, nullptr, 6200u, damage, lineOutput);
            room->m_KoukuBingoDuration.bLastLineCompletionSucceeded = lineOutput.BingoLineCompletion.value_or(false);
            tests.Require(lineOutput.BingoLineCompletion == true && player.iInvulnerableEndTick == 7100u && unsafe.iInvulnerableEndTick == 7100u &&
                player.iKoukuBingoLineProtectionEndTick == 7100u && unsafe.iKoukuBingoLineProtectionEndTick == 7100u,
                "One completed red row grants every living player a thirty-second buff independent of tile position");
            BOSS_PATTERN_MECHANIC_TRIGGER detonation; detonation.eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BINGO_DETONATION;
            room->m_PendingKoukuMechanicTriggers.push_back({owner.iNetEntityId, owner.iPatternSequence, detonation});
            room->Commit_KoukuMechanicTriggers(6500u);
            tests.Require(player.iCurrentHp == 100u && unsafe.iCurrentHp == 100u && owner.iCurrentHp == 8700u,
                "Successful new-line judgement makes the later black hole remove thirteen boss bars while the party survives");
            tests.Require(player.iInvulnerabilityZoneContactTick == 6500u && player.iInvulnerabilityZonePulseTick == 6500u &&
                unsafe.iInvulnerabilityZoneContactTick == 6500u && unsafe.iInvulnerabilityZonePulseTick == 6500u,
                "A line-protected black-hole verdict emits the same per-player invulnerable combat-text pulse as a safe zone");
            room->Commit_KoukuMechanicTriggers(6501u);
            tests.Require(owner.iCurrentHp == 8700u, "A drained detonation queue cannot apply thirteen bars twice");
            room->m_KoukuBingoDuration.bLastLineCompletionSucceeded = false;
            player.iShield = unsafe.iShield = 5000u;
            room->m_PendingKoukuMechanicTriggers.push_back({owner.iNetEntityId, owner.iPatternSequence, detonation});
            room->Commit_KoukuMechanicTriggers(6550u);
            tests.Require(player.iCurrentHp == 100u && unsafe.iCurrentHp == 100u && owner.iCurrentHp == 8700u,
                "An active thirty-second line reward survives failed Bingo detonation without awarding boss damage");
            room->m_PendingKoukuMechanicTriggers.push_back({owner.iNetEntityId, owner.iPatternSequence, detonation});
            player.iInvulnerableEndTick = unsafe.iInvulnerableEndTick = 8000u; // An unrelated immunity cannot extend earned line protection.
            room->Commit_KoukuMechanicTriggers(7100u);
            tests.Require(player.iCurrentHp == 0u && unsafe.iCurrentHp == 0u && owner.iCurrentHp == 8700u,
                "At the exact protection deadline failed Bingo detonation again wipes through shields without damaging the boss");
            tests.Require(player.iInvulnerabilityZonePulseTick == 6550u && unsafe.iInvulnerabilityZonePulseTick == 6550u &&
                player.iInvulnerabilityZoneContactTick != 7100u && unsafe.iInvulnerabilityZoneContactTick != 7100u,
                "An expired black-hole verdict never reports another invulnerable combat-text occurrence");

            player.iCurrentHp = unsafe.iCurrentHp = 100u;
            player.eAction = unsafe.eAction = PLAYER_ACTION_STATE::NONE;
            player.iInvulnerableEndTick = unsafe.iInvulnerableEndTick = 8000u;
            player.iKoukuBingoLineProtectionEndTick = unsafe.iKoukuBingoLineProtectionEndTick = 0u;
            player.iEstherZoneProtectionEndTick = unsafe.iEstherZoneProtectionEndTick = 0u;
            unsafe.fPositionX = player.fPositionX + 20.f;
            room->Open_EstherZone(EstherStrike::ZONES[0], player.fPositionX, player.fPositionZ, 7200u);
            room->Update_EstherZones(7200u);
            room->m_PendingKoukuMechanicTriggers.push_back({owner.iNetEntityId, owner.iPatternSequence, detonation});
            room->Commit_KoukuMechanicTriggers(7200u);
            tests.Require(player.iCurrentHp == 100u && player.iEstherZoneProtectionEndTick == 7202u && !unsafe.iCurrentHp,
                "Only the real Inanna zone source satisfies Bingo detonation; identical generic immunity outside does not");
            room->m_EstherZones.clear();
            room->m_PendingKoukuMechanicTriggers.push_back({owner.iNetEntityId, owner.iPatternSequence, detonation});
            room->Commit_KoukuMechanicTriggers(7202u);
            tests.Require(!player.iCurrentHp,
                "Bingo protection expires at Inanna's exact source deadline even when generic immunity lasts longer");

            player.iCurrentHp = 100u; player.eAction = PLAYER_ACTION_STATE::NONE; player.isCombatReady = true;
            player.fPositionX = owner.fPositionX + 1.f; player.fPositionZ = owner.fPositionZ;
            unsafe.iCurrentHp = 100u; unsafe.eAction = PLAYER_ACTION_STATE::NONE; unsafe.isCombatReady = true;
            unsafe.fPositionX = owner.fPositionX; unsafe.fPositionZ = owner.fPositionZ + 10.f;
            owner.iTargetEntityId = owner.iPatternTargetEntityId = unsafe.iNetEntityId;
            BOSS_PATTERN_DEFINITION tracking; tracking.strPatternId = owner.strPatternId;
            auto& track = tracking.MechanicTriggers.emplace_back(); track.eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TRACK_TARGET;
            track.iDurationMs = 34u;
            KOUKUSAYDON_LOGIC_LEDGER trackingLedger; CKoukuSaydonLogicRuntime::Build(tracking, owner, 6200u, trackingLedger);
            room->Update_KoukuPlayerTargets(owner, tracking, trackingLedger, room->m_GameplayCatalog.Active(), 6200u);
            tests.Require(owner.iPatternTargetEntityId == player.iNetEntityId && std::abs(owner.fYawDegrees) < .001f,
                "The one-shot facing Trigger immediately chooses the closest living player using the Saydon model basis");
            tracking.bFixedTimelineClock = true;
            tracking.Stages.emplace_back().iDurationMs = 10000u;
            track.iDurationMs = 10000u; track.fFollowSpeedScale = 1.f;
            CKoukuSaydonLogicRuntime::Build(tracking, owner, 6300u, trackingLedger);
            room->Update_KoukuPlayerTargets(owner, tracking, trackingLedger, room->m_GameplayCatalog.Active(), 6300u);
            tests.Require(!trackingLedger.bTrackingTargetReached && !trackingLedger.PlayerTargetWindows.front().bClosed,
                "An arbitrary whole-pattern pursuit does not inherit the explicit one-second pursuit completion rule");
            tracking.LogicWindows.emplace_back().iDurationMs = 10000u;
            CKoukuSaydonLogicRuntime::Build(tracking, owner, 6301u, trackingLedger);
            room->Update_KoukuPlayerTargets(owner, tracking, trackingLedger, room->m_GameplayCatalog.Active(), 6301u);
            tests.Require(!trackingLedger.bTrackingTargetReached && !trackingLedger.PlayerTargetWindows.front().bClosed,
                "A contact or counter attack keeps its remaining pattern and tracking window after touching the player");
            tracking.LogicWindows.clear(); track.iStartMs = 100u; track.iDurationMs = 1000u;
            CKoukuSaydonLogicRuntime::Build(tracking, owner, 6302u, trackingLedger);
            room->Update_KoukuPlayerTargets(owner, tracking, trackingLedger, room->m_GameplayCatalog.Active(), 6305u);
            tests.Require(!trackingLedger.bTrackingTargetReached && !trackingLedger.PlayerTargetWindows.front().bClosed,
                "An interior tracking window never truncates later landing or attack stages");
            for (const auto* ballPatternId : {"KAKULSAYDON_G1_PATTERN_81", "KAKULSAYDON_G1_PATTERN_118"})
            {
                std::string ballStatus;
                const auto* ballPattern = CKoukuSaydonBrain::Find_AnimationOnlyPattern(room->m_GameplayCatalog.Active(), ballPatternId, ballStatus);
                tests.Require(ballPattern != nullptr, "Published rolling-ball attack is available for contact regression");
                if (!ballPattern) continue;
                auto ballOwner = owner; ballOwner.strPatternId = ballPatternId;
                ballOwner.fPositionY += 3.f; // The rolling clip is above ground when contact occurs.
                const float ballHeight = ballOwner.fPositionY;
                KOUKUSAYDON_LOGIC_LEDGER ballLedger;
                CKoukuSaydonLogicRuntime::Build(*ballPattern, ballOwner, 6300u, ballLedger);
                room->Update_KoukuPlayerTargets(ballOwner, *ballPattern, ballLedger, room->m_GameplayCatalog.Active(), 6391u);
                tests.Require(!ballLedger.bTrackingTargetReached && !ballLedger.PlayerTargetWindows.empty() &&
                    !ballLedger.PlayerTargetWindows.front().bClosed && ballOwner.fPositionY == ballHeight,
                    "Published rolling-ball contact preserves its landing clock and cannot advance the raid flow");
            }

            for (const auto& [pursuitId, completesOnContact] : {std::pair{"KAKULSAYDON_G1_PATTERN_101", false}, std::pair{"KAKULSAYDON_G1_PATTERN_104", true}})
            {
                std::string pursuitStatus;
                const auto* pursuit = CKoukuSaydonBrain::Find_AnimationOnlyPattern(room->m_GameplayCatalog.Active(), pursuitId, pursuitStatus);
                tests.Require(pursuit != nullptr, "Published standalone pursuit is available for contact regression");
                if (!pursuit) continue;
                auto pursuitOwner = owner; pursuitOwner.strPatternId = pursuitId;
                KOUKUSAYDON_LOGIC_LEDGER pursuitLedger;
                CKoukuSaydonLogicRuntime::Build(*pursuit, pursuitOwner, 6392u, pursuitLedger);
                room->Update_KoukuPlayerTargets(pursuitOwner, *pursuit, pursuitLedger, room->m_GameplayCatalog.Active(), 6392u);
                tests.Require(pursuitLedger.bTrackingTargetReached == completesOnContact &&
                    pursuitLedger.PlayerTargetWindows.front().bClosed == completesOnContact,
                    completesOnContact ? "Only published one-second pursuit P104 finishes on player contact" :
                    "Published ordinary player pursuit P101 preserves its authored duration after player contact");
            }

            // Compare the selected-object hit to the original RESULT consumer, including
            // a moved boss origin and an already active reaction that force=false must retain.
            for (unsigned pushCase = 0u; pushCase < 4u; ++pushCase)
            {
                auto pushOwner = owner; pushOwner.fPositionX = 0.f; pushOwner.fPositionZ = -5.f;
                std::vector<SERVER_WORLD_ENTITY> pushOwners{pushOwner};
                SERVER_PLAYER victim; victim.iPlayerId = 1u; victim.iNetEntityId = 991u;
                victim.iCurrentHp = victim.iMaximumHp = 1000u;
                victim.fPositionX = 1.f; victim.fPositionY = 1.f; victim.fPositionZ = 0.f;
                if (pushCase == 3u) { victim.fKnockbackRemainingSeconds = .7f; victim.fKnockbackDirectionZ = -1.f; }
                auto expected = victim;
                std::map<PLAYER_ID, SERVER_PLAYER> victims{{1u, victim}};
                ATTACK_HIT_TEMPLATE authored; authored.strHitId = "selected.push.parity";
                authored.strTrigger = "CONTACT"; authored.iEndMs = 100u; authored.fRadiusM = 3.0;
                authored.fPushRangeM = 1.0; authored.iPushMs = 250u;
                authored.fRiseHeightM = pushCase == 0u ? 0.0 : 2.0;
                authored.ForcePush = pushCase == 2u;
                authored.strPushDirection = pushCase == 2u ? "AWAY_FROM_CONTACT" : "AWAY_FROM_BOSS";
                BOSS_COMBAT_OBJECT_DEFINITION definition;
                definition.strEncounterId = pushOwner.strEncounterId; definition.strOwnerPatternId = pushOwner.strPatternId;
                definition.strOwnerStageActionId = "selected.push.parity"; definition.iLifeMs = 1000u;
                definition.strCombatObjectArchetypeId = "combatobject.kouku.showtime.fixed";
                definition.strClientVisualId = "test.selected.push";
                definition.eOriginPolicy = BOSS_COMBAT_OBJECT_ORIGIN_POLICY::LOCKED_TARGET_PER_ALIVE_PLAYER;
                definition.AttackTemplates = {authored};
                SERVER_COMBAT_OBJECT_LOCKED_TARGET center;
                center.iNetEntityId = victim.iNetEntityId; center.fPositionX = -1.f; center.fPositionY = 1.f; center.fPositionZ = 0.f;
                CCombatObjectRuntime runtime; auto transaction = runtime.Begin_Transaction(); std::string pushStatus;
                const bool staged = runtime.Stage_BossCombatObject(transaction, pushOwner, &center, definition, nullptr,
                    room->m_GameplayCatalog.Active(), 1u, 20000u, pushStatus) && runtime.Commit(std::move(transaction));
                tests.Require(staged, "Selected hit stages horizontal-only or ballistic motion with explicit original push policy");
                if (!staged) continue;
                // The direction is sampled at damage, not cached at selection.
                pushOwners.front().fPositionX = 0.f; pushOwners.front().fPositionZ = -4.f;
                SERVER_WORLD_TO_PLAYER_HIT original;
                original.iRawDamage = 100u; original.bIgnoreDefense = original.bIgnoreCounter = true;
                original.fSourceX = pushCase == 2u ? center.fPositionX : pushOwners.front().fPositionX;
                original.fSourceZ = pushCase == 2u ? center.fPositionZ : pushOwners.front().fPositionZ;
                original.fPushRangeM = 1.f; original.iPushMs = 250u;
                original.bForcePush = pushCase == 2u; original.bPushBallistic = pushCase != 0u;
                original.fPushHeightM = float(authored.fRiseHeightM); original.iServerTick = 20001u;
                std::vector<DAMAGE_EVENT> expectedDamage, actualDamage;
                (void)CServerCombatHitRuntime::Apply_WorldToPlayer(expected, original, room->m_GameplayCatalog.Active(), expectedDamage);
                runtime.Update(victims, pushOwners, room->m_GameplayCatalog.Active(), 1.f / 30.f, 20001u, actualDamage);
                const auto& actual = victims.at(1u);
                tests.Require(actual.iCurrentHp >= 890u && actual.iCurrentHp <= 910u && actual.iCurrentHp == expected.iCurrentHp &&
                    actual.iIncomingDamageSampleSerial == 1u && expected.iIncomingDamageSampleSerial == 1u &&
                    actualDamage.size() == 1u && expectedDamage.size() == 1u &&
                    actualDamage.front().iAmount == 1000u - actual.iCurrentHp &&
                    expectedDamage.front().iAmount == actualDamage.front().iAmount &&
                    actual.eAction == expected.eAction && actual.bKnockbackBallistic == expected.bKnockbackBallistic &&
                    std::abs(actual.fKnockbackDirectionX - expected.fKnockbackDirectionX) < .00001f &&
                    std::abs(actual.fKnockbackDirectionZ - expected.fKnockbackDirectionZ) < .00001f &&
                    std::abs(actual.fKnockbackRemainingSeconds - expected.fKnockbackRemainingSeconds) < .00001f &&
                    std::abs(actual.fKnockbackVelocityY - expected.fKnockbackVelocityY) < .00001f,
                    "Selected hit preserves original RESULT damage, current boss/contact origin, ballistic height and force=false immunity");
            }

            BOSS_PATTERN_DEFINITION contact; contact.strPatternId = owner.strPatternId;
            auto& flame = contact.LogicWindows.emplace_back(); flame.eKind = BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP;
            flame.iDurationMs = 500u; flame.iRepeatIntervalMs = 100u;
            auto& flameRegion = flame.CardRegions.emplace_back(); flameRegion.eAnchor = BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT;
            flameRegion.fHalfX = flameRegion.fHalfZ = 2.f;
            BOSS_PATTERN_LOGIC_RESULT burn; burn.eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::FIXED_DAMAGE; burn.iDamageAmount = 10u;
            flame.OnSuccess.push_back(burn);
            owner.iPatternStartTick = 6400u; owner.KoukuContactLedger.reset();
            auto copy = owner; copy.iNetEntityId = owner.iNetEntityId + 1u; copy.fPositionX += 30.f;
            player.iCurrentHp = 100u; player.fPositionX = owner.fPositionX; player.fPositionZ = owner.fPositionZ;
            unsafe.iCurrentHp = 100u; unsafe.fPositionX = copy.fPositionX; unsafe.fPositionZ = copy.fPositionZ;
            const auto contactEventStart = room->m_TickDamageEvents.size();
            room->Update_KoukuActorContacts(owner, contact, room->m_GameplayCatalog.Active(), 6400u);
            room->Update_KoukuActorContacts(copy, contact, room->m_GameplayCatalog.Active(), 6400u);
            room->Update_KoukuActorContacts(owner, contact, room->m_GameplayCatalog.Active(), 6400u);
            const auto firstContactHp = player.iCurrentHp, cloneContactHp = unsafe.iCurrentHp;
            tests.Require(firstContactHp >= 89u && firstContactHp <= 91u && cloneContactHp >= 89u && cloneContactHp <= 91u &&
                room->m_TickDamageEvents.size() == contactEventStart + 2u &&
                room->m_TickDamageEvents[contactEventStart].iTargetNetEntityId == player.iNetEntityId &&
                room->m_TickDamageEvents[contactEventStart].iAmount == 100u - firstContactHp &&
                room->m_TickDamageEvents[contactEventStart + 1u].iTargetNetEntityId == unsafe.iNetEntityId &&
                room->m_TickDamageEvents[contactEventStart + 1u].iAmount == 100u - cloneContactHp &&
                owner.KoukuContactLedger != copy.KoukuContactLedger,
                "Direction actors own separate periodic contact ledgers and use their actual positions without duplicate-tick damage");
            owner.fPositionX += 60.f;
            room->Update_KoukuActorContacts(owner, contact, room->m_GameplayCatalog.Active(), 6403u);
            room->Update_KoukuActorContacts(copy, contact, room->m_GameplayCatalog.Active(), 6403u);
            tests.Require(player.iCurrentHp == firstContactHp && cloneContactHp - unsafe.iCurrentHp >= 9u &&
                cloneContactHp - unsafe.iCurrentHp <= 11u && room->m_TickDamageEvents.size() == contactEventStart + 3u &&
                room->m_TickDamageEvents.back().iTargetNetEntityId == unsafe.iNetEntityId &&
                room->m_TickDamageEvents.back().iAmount == cloneContactHp - unsafe.iCurrentHp,
                "Moving the real direction actor moves its fire region while a clone retains its own next contact tick");

            // Drive the real planted-bomb consumer with the pinned authored one-line reward.
            const auto* product = room->Resolve_KoukuProductCatalog();
            const auto* bingoGate = product ? product->Find_KoukuRaidGate("BINGO") : nullptr;
            std::string rewardStatus;
            const auto* rewardPattern = bingoGate ? CKoukuSaydonBrain::Find_AnimationOnlyPattern(
                *product, bingoGate->strBingoSpecialPatternId, rewardStatus) : nullptr;
            const BOSS_PATTERN_LOGIC_WINDOW* savedReward = nullptr;
            if (rewardPattern) for (const auto& row : rewardPattern->LogicWindows)
                if (row.eKind == BOSS_PATTERN_LOGIC_KIND::BINGO_COMPLETED_LINES) savedReward = &row;
            tests.Require(savedReward && savedReward->iThreshold == 1u && savedReward->OnSuccess.size() == 1u &&
                savedReward->OnSuccess.front().eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::PLAYER_INVULNERABILITY &&
                savedReward->OnSuccess.front().iDurationMs == 30000u,
                "The pinned Bingo special owns the one-new-line threshold and thirty-second typed reward");
            if (savedReward)
            {
                owner.strPatternId = rewardPattern->strPatternId; owner.iCurrentHp = owner.iMaximumHp;
                player.iCurrentHp = player.iMaximumHp = 100u; player.eAction = PLAYER_ACTION_STATE::NONE;
                player.iInvulnerableEndTick = 0u;
                unsafe.iCurrentHp = unsafe.iMaximumHp = 100u; unsafe.eAction = PLAYER_ACTION_STATE::NONE;
                unsafe.iInvulnerableEndTick = 0u; unsafe.fPositionZ = 1351.65f;
                auto& deadPlayer = room->m_Players[3u]; deadPlayer.iPlayerId = 3u; deadPlayer.iNetEntityId = 103u;
                deadPlayer.iCurrentHp = 0u; deadPlayer.eAction = PLAYER_ACTION_STATE::DEAD;
                auto& outsider = room->m_Players[4u]; outsider.iPlayerId = 4u; outsider.iNetEntityId = 104u;
                outsider.iCurrentHp = outsider.iMaximumHp = 100u; outsider.eAction = PLAYER_ACTION_STATE::NONE;
                room->m_KoukuRaid.State.ePhase = KOUKUSAYDON_RAID_PHASE::COMBAT;
                room->m_KoukuRaid.State.strGateId = "BINGO"; room->m_KoukuRaid.State.iRunEpoch = 71u;
                room->m_KoukuRaid.PlayerIds = {1u, 2u, 3u}; room->m_KoukuRaid.pCatalog = audition.pProductGeneration;
                room->Begin_KoukuBingoDuration(owner, trigger, 20000u);
                room->m_KoukuBingoDuration.iNextBombTick = room->m_KoukuBingoDuration.iNextHammerTick =
                    room->m_KoukuBingoDuration.iNextMadnessTick = 30000u;
                room->m_KoukuBingoDuration.bLineRewardSinceLastJudgement = false;
                room->m_KoukuBingo.Reset();
                BOSS_PATTERN_DEFINITION rewardOnly; rewardOnly.strPatternId = owner.strPatternId;
                rewardOnly.LogicWindows.push_back(*savedReward);
                audition.Members.clear();
                auto& rewardMember = audition.Members.emplace_back(); rewardMember.strMemberId = "bingo.reward.contract";
                rewardMember.iBossEntityId = owner.iNetEntityId;
                rewardMember.iPatternSequence = owner.iPatternSequence;
                CKoukuSaydonLogicRuntime::Build(rewardOnly, owner, 20000u, rewardMember.LogicLedger);
                const auto detonate = [&](int cell, unsigned tick, unsigned ordinal) {
                    tests.Require(room->m_KoukuBingo.Start_Bomb(player.iNetEntityId, tick, ordinal),
                        "Focused line reward fixture acquires a real bomb slot");
                    room->m_KoukuBingo.Plant_Bomb(0u, Kouku_BingoCellCenterX(cell), Kouku_BingoCellCenterZ(cell), tick);
                    room->Update_KoukuBingo(tick);
                };
                room->m_KoukuBingo.Fill(0x1eu); detonate(0, 20001u, 1u);
                tests.Require(!room->m_KoukuBingo.Get_RedMask() && !player.iInvulnerableEndTick && !unsafe.iInvulnerableEndTick,
                    "The a1 bomb cannot earn a transient line while a2 is toggled away in the same blast");
                detonate(6, 20002u, 2u);
                const auto rewardEnd = 20902u;
                tests.Require(room->m_KoukuBingo.Count_CompletedRowsAndColumns() == 1u &&
                    !room->m_KoukuBingo.Count_UnconsumedRowsAndColumns() &&
                    player.iInvulnerableEndTick == rewardEnd && unsafe.iInvulnerableEndTick == rewardEnd &&
                    !deadPlayer.iInvulnerableEndTick && !outsider.iInvulnerableEndTick && !room->m_KoukuBingoDuration.iLastLineJudgementTick,
                    "The second bomb completes one new row and immediately protects living raid participants for thirty seconds, excluding dead members and nonparticipants");
                detonate(2, 20003u, 3u);
                KOUKUSAYDON_LOGIC_OUTPUT rewardedOutput;
                CKoukuSaydonLogicRuntime::Update(owner, rewardOnly, rewardMember.LogicLedger, room->m_Players,
                    *product, nullptr, 20004u, damage, rewardedOutput);
                room->m_KoukuBingoDuration.bLastLineCompletionSucceeded = false;
                (void)room->Apply_KoukuLogicOutput(rewardedOutput, owner, 20004u);
                tests.Require(rewardedOutput.BingoLineCompletion == true && rewardMember.LogicLedger.bBingoLineRewardApplied &&
                    room->m_KoukuBingoDuration.bLastLineCompletionSucceeded && player.iInvulnerableEndTick == rewardEnd &&
                    unsafe.iInvulnerableEndTick == rewardEnd && !outsider.iInvulnerableEndTick && !room->m_KoukuBingo.Count_UnconsumedRowsAndColumns(),
                    "The delayed Parent acknowledges an already rewarded line without renewing expiry, paying outsiders or consuming again");
                CKoukuSaydonLogicRuntime::Build(rewardOnly, owner, 20005u, rewardMember.LogicLedger);
                detonate(2, 20005u, 6u);
                KOUKUSAYDON_LOGIC_OUTPUT reusedOutput;
                CKoukuSaydonLogicRuntime::Update(owner, rewardOnly, rewardMember.LogicLedger, room->m_Players,
                    *product, nullptr, 20006u, damage, reusedOutput);
                tests.Require(reusedOutput.BingoLineCompletion == false && player.iInvulnerableEndTick == rewardEnd,
                    "A later judgement cannot reuse the old permanent red line to renew protection");
                room->m_KoukuBingo.Detonate(1u << 7u); // Prepare the sole missing ordinary cell before the real blast.
                room->m_KoukuBingo.Fill(Kouku_BingoLineMask(1) & ~(1u << 7u));
                detonate(2, 20007u, 7u);
                tests.Require(room->m_KoukuBingo.Count_CompletedRowsAndColumns() == 2u &&
                    !room->m_KoukuBingo.Count_UnconsumedRowsAndColumns() && player.iInvulnerableEndTick == 20907u &&
                    unsafe.iInvulnerableEndTick == 20907u && !outsider.iInvulnerableEndTick,
                    "Completing a different single red row immediately refreshes protection from that new formation tick");
                room->m_KoukuBingo.Fill((Kouku_BingoLineMask(2) & ~(1u << 12u)) |
                    (Kouku_BingoLineMask(4) & ~(1u << 22u)));
                detonate(17, 20008u, 8u);
                tests.Require(room->m_KoukuBingo.Count_CompletedRowsAndColumns() == 7u &&
                    !room->m_KoukuBingo.Count_UnconsumedRowsAndColumns() && player.iInvulnerableEndTick == 20908u,
                    "One blast completes multiple rows and columns atomically and consumes every newly rewarded line at that same tick");
                detonate(17, 20009u, 9u);
                tests.Require(player.iInvulnerableEndTick == 20908u && !room->m_KoukuBingo.Count_UnconsumedRowsAndColumns(),
                    "Simultaneous extra lines cannot remain queued to create a delayed reward on the next explosion");
                room->m_KoukuBingo.Reset(); room->m_KoukuBingo.Fill(Kouku_BingoLineMask(0));
                player.iCurrentHp = unsafe.iCurrentHp = 0u; player.eAction = unsafe.eAction = PLAYER_ACTION_STATE::DEAD;
                detonate(2, 20010u, 10u);
                tests.Require(room->m_KoukuBingo.Count_UnconsumedRowsAndColumns() == 1u && !outsider.iInvulnerableEndTick,
                    "A living nonparticipant cannot consume a reward while every pinned raid participant is dead");
                player.iCurrentHp = 100u; player.eAction = PLAYER_ACTION_STATE::NONE;
                detonate(2, 20011u, 11u);
                tests.Require(!room->m_KoukuBingo.Count_UnconsumedRowsAndColumns() && player.iInvulnerableEndTick == 20911u &&
                    !outsider.iInvulnerableEndTick && unsafe.iInvulnerableEndTick == 20908u,
                    "An eligible revived participant may claim the retained new line without renewing dead or unpinned players");
            }

            // Authored encounter windows use the raid clock and drain already marked bombs.
            room->Stop_KoukuBingoDuration(true);
            owner.iCurrentHp = owner.iMaximumHp; owner.iPatternSequence = 901u;
            room->m_KoukuRaid.State.ePhase = KOUKUSAYDON_RAID_PHASE::COMBAT;
            room->m_KoukuRaid.State.strGateId = "BINGO"; room->m_KoukuRaid.State.iRunEpoch = 72u;
            room->m_KoukuRaid.State.iStartTick = 1000u;
            room->m_KoukuRaid.pCatalog = audition.pProductGeneration;
            player.iCurrentHp = 100u; player.eAction = PLAYER_ACTION_STATE::NONE;
            player.fPositionX = Kouku_BingoCellCenterX(12); player.fPositionZ = Kouku_BingoCellCenterZ(12);
            for (auto& [id, other] : room->m_Players) if (id != 1u) other.iCurrentHp = 0u;
            BOSS_BINGO_BOARD_SETTINGS tuned;
            tuned.bWindowOnly = true; tuned.iFirstBombDelayMs = 1000u; tuned.iBombIntervalMs = 5000u;
            tuned.iBombMarkMs = 2000u; tuned.iBombDropDelayMs = 1000u; tuned.iBombFuseMs = 8000u;
            tuned.iInitialMarkedCells = 7u;
            auto authoredBoard = trigger;
            authoredBoard.iStartMs = 2000u; authoredBoard.iDurationMs = 2500u; authoredBoard.BingoBoardSettings = tuned;
            room->Begin_KoukuBingoDuration(owner, authoredBoard, 1000u);
            room->Update_KoukuBingo(1059u);
            tests.Require(!room->m_KoukuBingoDuration.bActivated && !room->m_KoukuBingo.Get_WhiteMask(),
                "Authored Bingo start offset delays initial cells and every periodic scheduler");
            room->Update_KoukuBingo(1060u);
            tests.Require(room->m_KoukuBingoDuration.bActivated && std::popcount(room->m_KoukuBingo.Get_WhiteMask()) == 7 &&
                room->m_KoukuBingoDuration.iNextBombTick == 1090u && room->m_KoukuBingoDuration.iEndTick == 1135u,
                "Window starts from raid combat time with seven distinct cells and the authored first delay");
            room->m_KoukuBingo.Reset();
            room->m_KoukuBingoDuration.iNextMadnessTick = room->m_KoukuBingoDuration.iNextHammerTick = 99999u;
            room->Update_KoukuBingo(1089u);
            tests.Require(countPhase(BINGO_BOMB_PHASE::MARKED) == 0, "A tuned first mark cannot begin before its exact deadline");
            room->Update_KoukuBingo(1090u);
            tests.Require(countPhase(BINGO_BOMB_PHASE::MARKED) == 1 && room->m_KoukuBingo.Get_Bombs()[0].iDetonateTick == 1150u &&
                room->m_KoukuBingoDuration.iNextBombTick == 1240u,
                "First delay, mark duration and recurring interval independently consume authored values");
            room->Update_KoukuBingo(1135u);
            room->Begin_KoukuBingoDuration(owner, authoredBoard, 1136u);
            tests.Require(room->m_KoukuBingoDuration.iOwnerId == owner.iNetEntityId &&
                room->m_KoukuBingoDuration.iActivationTick == 1060u && countPhase(BINGO_BOMB_PHASE::MARKED) == 1,
                "Window expiry retains its encounter owner and cannot restart or discard an in-flight mark");
            room->m_KoukuBingoDuration.Settings.iBombDropDelayMs = 2000u;
            room->m_KoukuBingoDuration.Settings.iBombFuseMs = 4000u;
            room->Update_KoukuBingo(1150u);
            tests.Require(room->m_KoukuBingo.Get_Bombs()[0].iPlantTick == 1180u,
                "A pending mark retains its captured drop delay after the active window or settings change");
            room->Update_KoukuBingo(1180u);
            tests.Require(countPhase(BINGO_BOMB_PHASE::PLANTED) == 1 && room->m_KoukuBingo.Get_Bombs()[0].iDetonateTick == 1420u,
                "The pending ground bomb retains its authored eight-second fuse beyond the board window");
            room->Update_KoukuBingo(1420u); room->Update_KoukuBingo(1500u);
            tests.Require(!countPhase(BINGO_BOMB_PHASE::MARKED) && !countPhase(BINGO_BOMB_PHASE::PLANTED) &&
                room->m_KoukuBingo.Get_WhiteMask() == Kouku_BingoCrossMask(12) && room->m_KoukuBingoDuration.iMarkedBombCount == 1u,
                "A window drains its last bomb and flips the board without issuing later marks");
            for (const auto count : {0u, 25u})
            {
                room->Stop_KoukuBingoDuration(true); ++room->m_KoukuRaid.State.iRunEpoch;
                authoredBoard.iStartMs = 0u; authoredBoard.BingoBoardSettings->iInitialMarkedCells = count;
                room->Begin_KoukuBingoDuration(owner, authoredBoard, 1000u);
                tests.Require(std::popcount(room->m_KoukuBingo.Get_WhiteMask()) == int(count),
                    count ? "Initial-cell maximum fills exactly the full board" : "Zero initial cells preserves an empty board");
            }
            room->Stop_KoukuBingoDuration(true);

            // Exercise the real transactional catalog reader with the new supplemental rows.
            namespace fs = std::filesystem;
            auto baseRows = audition.pProductGeneration->Export_BootstrapBytes();
            std::istringstream sourceRows(baseRows); std::string line, stripped;
            while (std::getline(sourceRows, line))
                if (!line.starts_with("PATTERNBINGOBOARD\t") && !line.starts_with("PATTERNBINGOCONTROL\t")) stripped += line + "\n";
            baseRows = std::move(stripped);
            const std::string identity = "\tENCOUNTER_KAKULSAYDON_G1\tKAKULSAYDON_G1_PATTERN_129\tKAKULSAYDON_G1_PATTERN_129.logic.1";
            const std::string settingsRow = "PATTERNBINGOBOARD" + identity + "\tWINDOW\t1000\t5000\t2000\t1000\t8000\t7\n";
            const std::string controlRow = "PATTERNBINGOCONTROL" + identity + "\n";
            const auto directory = fs::temp_directory_path() / (L"LostArkBingoSettings-" + std::to_wstring(GetCurrentProcessId()));
            std::error_code fileError; fs::create_directories(directory, fileError);
            const auto path = directory / L"Gameplay.bootstrap";
            auto admitted = std::make_unique<CGameplayCatalog>();
            const auto readRows = [&](const std::string& addition) {
                auto bytes = baseRows + addition;
                const auto end = bytes.find('\n'), field = bytes.rfind('\t', end);
                bytes.replace(field + 1u, end - field - 1u, std::to_string(std::count(bytes.begin(), bytes.end(), '\n') - 1u));
                { std::ofstream file(path, std::ios::binary | std::ios::trunc); file.write(bytes.data(), static_cast<std::streamsize>(bytes.size())); }
                GameplayDataRevision hash; std::string status;
                return !fileError && CServerApp::Hash_GameplayFileForAdmission(path, hash, status) &&
                    admitted->Load_FromBootstrap(fs::canonical(path), hash, hash);
            };
            const bool validSettings = readRows(settingsRow + controlRow);
            if (!validSettings) std::cout << "[BINGO SETTINGS] " << admitted->Get_Status() << '\n';
            tests.Require(validSettings, "Native catalog admits authored Bingo settings and the explicit pure-control marker");
            const auto locateSettings = [&]() -> const BOSS_PATTERN_MECHANIC_TRIGGER* {
                const auto* patterns = admitted->Find_BossPatterns("ENCOUNTER_KAKULSAYDON_G1");
                if (patterns) for (const auto& pattern : *patterns) if (pattern.strPatternId == "KAKULSAYDON_G1_PATTERN_129")
                    for (const auto& item : pattern.MechanicTriggers) if (item.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BINGO_BOARD) return &item;
                return nullptr;
            };
            const auto* parsed = locateSettings();
            tests.Require(parsed && parsed->BingoBoardSettings && parsed->BingoBoardSettings->iBombFuseMs == 8000u &&
                parsed->BingoBoardSettings->bWindowOnly && parsed->bBingoControlOnly,
                "Native catalog retains timing, active mode and certified control ownership");
            if (validSettings)
            {
                auto controlCatalog = std::make_shared<CGameplayCatalog>(*admitted);
                auto* patterns = const_cast<std::vector<BOSS_PATTERN_DEFINITION>*>(controlCatalog->Find_BossPatterns("ENCOUNTER_KAKULSAYDON_G1"));
                auto control = std::find_if(patterns->begin(), patterns->end(), [](const auto& pattern) { return pattern.strPatternId == "KAKULSAYDON_G1_PATTERN_129"; });
                control->Stages.front().iDurationMs = control->iTimelineDurationMs = 600000u;
                control->MechanicTriggers.front().iDurationMs = 600000u;
                auto& run = room->m_KoukuRaid; run.pCatalog = controlCatalog; run.iPrimaryBossId = owner.iNetEntityId;
                run.State.iFlowEntryIndex = 0u; run.bEntryRunning = false;
                tests.Require(room->Start_KoukuRaidEntry(2000u) && !run.bEntryRunning && run.State.iFlowEntryIndex == 1u && run.iNextEntryTick == 2002u,
                    "A certified ten-minute board control advances after the historical 34ms instead of blocking ordinary boss attacks");
                control->MechanicTriggers.front().bBingoControlOnly = false;
                run.State.iFlowEntryIndex = 0u; run.State.strFlowEntryId.clear();
                tests.Require(!room->Start_KoukuRaidEntry(2100u) && run.State.iFlowEntryIndex == 0u && run.State.strFlowEntryId.empty(),
                    "An unmarked Pattern is never inferred to be an empty control and skipped");
            }
            for (const auto& invalid : {
                settingsRow + settingsRow, settingsRow + controlRow + controlRow,
                "PATTERNBINGOBOARD" + identity + "\tWINDOW\t1000\t5000\t2000\t1000\t249\t7\n",
                "PATTERNBINGOBOARD" + identity + "\tWINDOW\t1000\t1000\t2000\t1000\t8000\t7\n",
                "PATTERNBINGOBOARD" + identity + "\tWINDOW\t1000\t100\t1\t1\t398\t7\n"})
            {
                tests.Require(!readRows(invalid), "Native Bingo settings reject duplicates, unsupported fuse rates and bomb-slot overflow");
                parsed = locateSettings();
                tests.Require(parsed && parsed->BingoBoardSettings && parsed->BingoBoardSettings->iBombFuseMs == 8000u,
                    "Rejected Bingo settings preserve the previously admitted catalog");
            }
            fs::remove(path, fileError); fs::remove(directory, fileError);
        }
    }
#endif
	std::cout << "bingo failures: " << tests.failures << '\n';
	return tests.failures == 0 ? 0 : 1;
}
