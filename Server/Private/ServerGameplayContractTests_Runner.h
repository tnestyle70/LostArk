#pragma once

#include "ServerGameplayContractTests_Internal.h"

namespace LostArk::Shared
{
struct C2S_USE_SKILL;
}

namespace LostArk::Server
{
class CServerNavigation;
class CValtanBrain;
struct SERVER_WORLD_ENTITY;
struct PLAYER_SKILL_DEFINITION;

// The executable contract keeps its private probes in one named test owner.
// Public runtime classes expose no additional accessors for these fixtures.
class CServerGameplayContractRunner final
{
public:
    static int Run(ServerGameplayContractDetail::CONTRACT_TEST_RUN_CONTEXT& context);

private:
    static int Run_WorldPlayback(TESTS& tests);
    static int Run_DebugTeleport(TESTS& tests);
    static void Run_KoukuBundles(TESTS& tests);
    static void Run_KoukuProduct(TESTS& tests, CGameplayCatalog& catalog);
    static void Run_ValtanLifecycle(TESTS& tests, CGameplayCatalog& catalog);
    static void Run_ValtanPinnedGeneration(TESTS& tests, CGameplayCatalog& catalog);
    static void Run_ValtanRevision(TESTS& tests, CGameplayCatalog& catalog);
    static void Run_RevisionProtocol(TESTS& tests);
    static void Run_GenerationRetention(TESTS& tests, CGameplayCatalog& catalog);
    static void Run_SessionTransport(TESTS& tests);
    static void Run_RoomIngress(TESTS& tests);
    static void Run_CharacterAdmission(TESTS& tests, CGameplayCatalog& catalog);
    static void Run_GroundTarget(TESTS& tests, CGameplayCatalog& catalog);
    static void Run_PlayerCombos(TESTS& tests, CGameplayCatalog& catalog);
    static void Run_PlayerActions(TESTS& tests, CGameplayCatalog& catalog, CServerNavigation& navigation, const float& navCellSize, const float& boundaryProbeZ, float& lastWalkableX, float& firstBlockedX, SERVER_WORLD_ENTITY& boss, const PLAYER_SKILL_DEFINITION*& talonStrike, LostArk::Shared::C2S_USE_SKILL& useSkill);
    static void Run_ValtanMechanicLedger(TESTS& tests, CGameplayCatalog& catalog, CServerNavigation& navigation, CValtanBrain& brain);
    static void Run_ValtanSkyAxe(TESTS& tests);
    static void Run_ValtanTimelines(TESTS& tests);
    static void Run_WorldTriggers(TESTS& tests, CGameplayCatalog& catalog);
    static void Run_SkillStages(TESTS& tests, CGameplayCatalog& catalog);
    static void Run_SpawnGroups(TESTS& tests, CGameplayCatalog& catalog);
    static void Run_ValtanDash(TESTS& tests, CGameplayCatalog& catalog, const char* VALTAN_WALL_COLLISION_STATE, float VALTAN_WALL_CENTER_X, float VALTAN_WALL_CENTER_Y, float VALTAN_WALL_CENTER_Z);
    static void Run_ValtanAudition(TESTS& tests);
    static void Run_ValtanResetlessNext(TESTS& tests, const char* VALTAN_WALL_COLLISION_STATE);
    static void Run_ValtanReleaseControl(TESTS& tests);
    static void Run_WorldDestruction(TESTS& tests, CGameplayCatalog& catalog, CServerNavigation& navigation);
};
}
