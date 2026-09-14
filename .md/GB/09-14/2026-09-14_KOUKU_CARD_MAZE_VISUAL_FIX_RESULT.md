# 카드미로 망원경·바닥 문양 수정 — RESULT

기준 계획: [2026-09-14_KOUKU_CARD_MAZE_VISUAL_FIX_PLAN.md](2026-09-14_KOUKU_CARD_MAZE_VISUAL_FIX_PLAN.md) 개정 R2.
상태는 **문양 구현**, **망원경 구현**, **자동 검증**, **사용자 빌드·화면 확인 대기**를 분리해 기록한다.

## R0. 작업 전 고정한 상태

- 저장소 `C:/Users/USER/source/졸업팀폴/LostArk`, HEAD `29ad2df5`(main). 기능 브랜치 `feature/kouku-cardmaze-visual-fix`를 같은 커밋에서
  만들었다(파일 변경 없음, 커밋·푸시 없음). 작업 트리에는 이 작업과 무관한 다른 미커밋 변경 22개가 있으며 건드리지 않았다.
- 계획서 부록의 기준 SHA-256 10개가 모두 현재 파일과 일치했다. 그래도 전문을 덮어쓰지 않고 앵커 문자열 치환으로 바뀌는 구간만 적용했다
  (`$CLAUDE_JOB_DIR/tmp/cardmaze/apply_cardmaze_symbol_fix.py`, 원본 백업 `tmp/cardmaze/backup/`).
- Visual Studio(devenv)가 열려 있어 에이전트는 제품 빌드를 실행하지 않았다. 빌드는 사용자가 한다.

## R1. 문양 수정 (G02~G04) — 구현 완료, 빌드·화면 확인 대기

| 파일 | 변경 |
|---|---|
| `Data/Effects/V2/Authored/cardmaze.symbol.{heart,spade,club,diamond}.effectv2.json` | `effectType` Decal→Texture, rotation start/end `[90,-90,0]`, scale start/end `[2,2,1]`. 나머지 필드는 원본과 동일(파싱 비교). LF·들여쓰기 유지 |
| `Engine/Public/BlendSortKey.h` (신규) | `BLEND_SORT_KEY`, `BlendSortBefore()` — priority 오름차순, 같은 priority는 거리 내림차순 |
| `Engine/Public/GameObject.h` | `virtual int32_t Get_BlendSortPriority() const { return 0; }` 추가 |
| `Engine/Private/Renderer.cpp` | `#include "BlendSortKey.h"`, `Render_Blend()`만 교체. 기존 설명 주석 유지. 카메라 없음도 같은 stable 정렬(거리 동률) |
| `Client/Public/CardMazeVisualPolicy.h` (신규) | `IsCardMazeFloorReceiver`(placement `10296705976280178153` + asset `MAP_3C514C107BAB_LV_OCN_FORGOTTENIS_PLANE01_SM_OVR_017DC7A6977C` 둘 다 일치), `IsCardMazeMarkGroup`(8 group), `CARD_MAZE_MARK_RETRY` |
| `Client/Public/MapAssetObject.h`, `Client/Private/MapAssetObject.cpp` | `Get_BlendSortPriority()` override — 위 바닥만 -1, 나머지 0 |
| `Client/Public/KoukuSaydonPresentationPlayer.h` | `CARD`에 `mazeRetry` 추가, 정책 헤더 include |
| `Client/Private/KoukuSaydonPresentationPlayer.cpp` | `<chrono>` include, `Sync_MazeMark()` — 카드미로 8 group만 1초 간격·occurrence/generation당 최대 3회 시도, 재시도 때 해당 `GROUP:<id>` 캐시만 제거. Bingo 등 다른 ID는 기존 본문 그대로 |

변경하지 않은 것: group 8개, DDS, 색·alpha·수명, `groundPivot`(+0.02m, translation only), HUNTER/탈출/전이 필터, Server, 네비, 카메라, 재질, HLSL.

### 프로젝트 등록 (계획 G05 준비) — 적용하지 않음

`Engine.vcxproj(.filters)`와 `Client.vcxproj(.filters)`에 두 신규 헤더 `ClInclude`를 넣는 부분은 **사용자 규칙("vcxproj/filters는 직접 수정하지 않는다")에 따라 적용하지 않았다.**
두 파일은 헤더 전용이라 컴파일은 include 경로(`Engine/Public`, `Client/Public`, SDK 복사 `Engine\Public\*.*`)로 해결되며 등록 여부와 무관하다.
솔루션 탐색기에 보이게 하려면 VS에서 각 프로젝트의 `추가 > 기존 항목`으로 넣는다.

## R2. 자동 검증 (에이전트 실행분)

- 계획서 기준 해시 10개 일치 확인(적용 전). 적용 후 `git diff --check` 통과. C++ 변경 파일 CRLF 일관·BOM 없음 유지, 새 C++ 파일 UTF-8(BOM 없음)·CRLF.
- `Tools/EffectToolV2/effect_v2_binding_pipeline._resolve_group`으로 4 leaf / 8 group 실제 파일 해석: 8개 모두 leaf 1개·span 1000ms, leaf의 base/mask DDS 실재. 출력 `Card maze inputs: 4 leaves / 8 groups validated; GPU visibility not tested`.
- 바닥 ID: authoring/runtime `mapplacements` 양쪽에서 정확히 1행이 해당 asset을 참조. asset 렌더 모드 `Alpha` → `Is_BatchEligible`(DEFERRED만 허용)에서 제외되어 개별 `CMapAssetObject`로 BLEND 제출됨을 코드로 확인.
- 회전·UV: `CEffectV2Object` 월드 `Scale * RollPitchYaw(x,y,z) * Translation * Pivot`, `Shader_EffectDecalV2.hlsl` UV `(local.x/size.x+0.5, 0.5-local.z/size.y)`, `VIBuffer_Rect` UV `(x+0.5, 0.5-y)`로 수치 비교: 네 모서리+중앙+임의점 최대 UV 오차 `5.55e-17`, 평면 Y 오차 `6.1e-17`, 앞면 법선 +Y.
- Effect V2 문서는 `decal` 블록을 타입과 무관하게 optional로 읽음(`EffectV2_Document.cpp` 853행) — 남겨 둔 블록이 Texture 로드를 막지 않음.
- 신규 헤더 단독 정책 검사: 저장소 밖(`tmp/cardmaze/build`)에서 MSVC x64 `/std:c++20 /W4 /WX` 컴파일·실행, `failures=0`
  (priority→거리 정렬, 동률 안정성, 카메라 없음, 두 ID 가드, 8 group, 3회 예산·1초 간격·generation 초기화·overflow).
- 새 이름(`Get_BlendSortPriority` 등)과 기존 코드 충돌 없음(grep).

실행하지 않은 것: 제품 Engine/Client 컴파일·링크, GPU 표시, Client 실행, 실패 주입(없는 DDS 등) 런타임 확인.

## R3. 사용자 빌드·화면 확인 (대기)

- **빌드는 Engine과 Client를 함께**(솔루션 Build 또는 Product runner). `Engine/Public/GameObject.h`에 가상 함수가 추가되어 `CGameObject` vtable이 바뀐다.
  Engine.dll만 옛 것이거나 Client만 새로 빌드되면 가상 호출 순서가 어긋난다. `EngineSDK/inc/GameObject.h`는 아직 옛 복사본(09-12)이며 Client 빌드의 `PrepareEngineSdk`가 갱신한다.
  공용 헤더라 C++ 재컴파일 범위가 넓다. Clean/Rebuild는 필요 없다.
- 확인 경로와 기대(계획서 표 그대로): 로비 → 쿠크 → 카드미로 실제 진입 → 중앙 Q 시작.
  병사 4종 발밑 문양 표시·추종(회전·크기 영향 없음), HUNTER 조건 유지, 1초 이상 유지·중복 없음, 카드 벽 근처 새 가림 회귀 없음, 처치·탈출·종료 시 제거, 재입장 시 중복 없음.
- 화면 PASS는 사용자 서면 판정 전까지 기록하지 않는다.

## R4. 망원경 (G01) — 조사 기록 (설치는 R5)

원본 입력은 게임 파일에서 찾아 스크래치에 추출했다. 원본 구조가 계획서 G01의 전제(LookInfo → Static/Skeletal mesh 하나를 Deploy STATIC/ANIM 행으로 설치)와 달라서 **catalog·placement 행, cook, publish는 하지 않았다.**

| 계획서 상태 항목 | 결과 |
|---|---|
| LookInfo 확보 | 확보 — `data4.lpk` entry 57025 `\EFGame_Extra\ClientData\XmlData\LookInfo\Prop\EFDLProp_ITR_10073.ITR_10073.loa`(1046 bytes). UModel로는 `EFDLProp_*`가 풀리지 않는다 |
| mesh 확보 | 확보(스크래치) — `FX_SM_00.fm_g_telescope_01` StaticMesh, 1442 정점, 668 삼각형, 뼈 없음 |
| material·texture 확보 | 확보(스크래치) — MIC `bfx_m_mi_00.bfx_mi.bfx_g_pa_telescope_01_ma`(부모 `bfx_e_transition_03_ma`, BLEND_Masked), diffuse `fx_g_telescope_01_cl`, normal `fx_g_telescope_01_n_cl`, spec `fx_g_telescope_02_cl`, emissive `fx_a_fire_018`, dissolve `fx_a_noise_014`. export 오류 없음 |
| cooked 검증 | 안 함 |
| 초기 표시 pose 확인 | 불명 — AnimSet 참조 없음, Prop 행의 State/Spawn/Usage action id 전부 0 |
| 게시 / 제품 표시 | 안 함 |

확인된 원본 사실:
- DeployData(zone 37081) 레코드 1개: actor 268435714, def 378134, UE 위치 (-2.4e-05, -135168.0, 0.0) → 클라 (-2.4e-07, 0.0, 1351.68), rotator (0,0,0), scale 100%. 계획서의 좌표·항등 회전·scale 1과 일치한다.
- LookInfo가 참조하는 것은 세 가지다: 숨김 재질(`hide_prop_material`) 호스트 `SkeletalMesh'ITR_00.Mesh.ITR_FX_Templete_Cylinder_256'`(3뼈), `PhysicsAsset'ITR_10022.Ani.ITR_FX_10022_Physics'`, 그리고 `B_Root`에 붙는 `ParticleSystem'BFX_Low_01.Etc.Par_G_Telescope_004'`. 두 번째 `CEFParticleData`는 파티클 참조가 비어 있다.
- **망원경 모양은 파티클 시스템의 메쉬 이미터**다. 7개 이미터 중 유일한 메쉬 이미터 `particlespriteemitter_11`이 `fm_g_telescope_01`을 override 재질로 그린다. burst 1, Lifetime 2.0, EmitterDuration 2.0, StartSize 0.5, bUseLocalSpace, Z 방향 VelocityOverLife 곡선, alpha scale over life 1→2→1 조건이다. 나머지 6개는 glow/ring/energy ray/spark sprite 이미터다.
- 저장소에는 이 망원경 이펙트·모델이 없다. `fx_g_telescope_01_cl.dds`만 3관문 총구·signalshot 이펙트의 재질 텍스처로 이미 들어와 있다.

모르는 것(추측으로 채우지 않음):
- LookInfo 부착 transform. `B_Root` 뒤 float가 (0,0,80) UE·scale 1로 읽히지만, 그 레이아웃이 LookInfo에 맞는지 증명되지 않았다.
- 이미터 반복(EmitterLoops 등 직렬화 안 된 기본값)과 MeshAlignment, 그래서 망원경이 계속 떠 있는지·위아래로 움직이는지.
- 메쉬의 위쪽 축(glTF bounds가 대칭이라 단위 m와 Y/Z 교환만 증명됨)과 최종 크기(StartSize 0.5 적용 방식).
- 호스트 실린더가 게임에서 실제로 안 보이는지, ConditionId 3708121의 의미.

설치 경로 선택지(사용자 결정 전 미착수):
1. 계획서 G01대로 `fm_g_telescope_01`만 STATIC Deploy로 설치한다. 위 불명 항목(높이·축·크기)을 임의 값으로 정해야 한다. 원본의 파티클 동작·발광·dissolve 재질은 재현되지 않는다.
2. 원본처럼 `Par_G_Telescope_004`를 이펙트(메쉬 파티클 + sprite)로 복원해 해당 위치에 붙인다. 기존 이펙트 복원 절차 범위이며 G01보다 크다.

추출물: `$CLAUDE_JOB_DIR/tmp/itr10073/`(`inventory.txt`에 전체 목록, `par_g_telescope_004.walk.txt`, `mic_telescope.walk.json`, export 로그, 스크립트).

## R5. 망원경 설치 — 사용자 지시 "트리거 위치에다가 연결" (2026-09-14 14:30~)

사용자가 R4 선택지 중 메쉬 설치를 지시했다. 원본 파티클 동작·sprite 발광·transition/dissolve 재질은 재현하지 않는다.

모양 확인: 메쉬를 텍스처 입혀 3방향 투영 렌더(`tmp/itr10073/render/telescope_views.png`) — 받침 없는 손망원경, glTF +Y가 위, 대물렌즈 쪽이 약 25° 들린 자세.

| 항목 | 값 | 근거 |
|---|---|---|
| 모델 | `Client/Bin/Resources/Map/LV_LUT_MIDNIGHTC_ED/AnimatedProps/DEPLOY_ITR_10073/DEPLOY_ITR_10073.wmodel` (+ `textures/` 3장, `DEPLOY_ITR_10073.cook.json`) | ModelAssetConverter `--scale 100 --no-auto-textures`, `dummy_material_0`에 base `fx_g_telescope_01_cl`(DXT1)·normal `fx_g_telescope_01_n_cl`(ATI2, 기존 레버 소품과 같은 형식)·specular `fx_g_telescope_02_cl` 연결. MIC의 emissive_intensity가 0이라 발광 텍스처는 연결 안 함. 결과 `skeleton=no`, 정점 1442, 로컬 bounds ±(9.53, 26.63, 57.13) cm, +Y 위 |
| catalog | `Data/Maps/Imported/.../LV_LUT_MIDNIGHTC_ED.deployassets` 4→5, `DEPLOY_ITR_10073` STATIC, 애니메이션 role 빈값 | 계획서 G01 STATIC 행, evidence 문구에 파티클 미재현 명시 |
| placement | `Data/Maps/Authoring/.../LV_LUT_MIDNIGHTC_ED.deployplacements` 6→7, `8 0 0 "cardmaze.telescope.visual" "DEPLOY_ITR_10073" 0.28 0.79 1351.65002 0 0 0 1 0.5 0 0 0 PROJECT_AUTHORED` | XZ = 트리거 `cardmaze.telescope` 중심(사용자 지시). Y = 트리거 -0.01 + 0.8 m(LookInfo `B_Root` 뒤 float 80 UE cm, 레이아웃 미증명). 회전 0(DeployData rotator 0). scale 0.5(메쉬 이미터 StartSize 0.5; 파티클 고정 bounds ±40 cm 수평에 들어가는 크기와 일치) |

Deploy 객체는 배치 좌표에 모델 원점(메쉬 중심)을 두므로(`DeployPropObject.cpp` 세계 행렬, bottom-center 보정 없음) 망원경 중심이 바닥 위 0.8 m, 화면상 길이 약 0.57 m다.
0.8 m 높이와 0.5 크기는 원본 데이터 해석값이며 화면에서 다르면 배치 행의 Y와 scale만 고치면 된다(STATIC이라 Map Tool Animated Props 목록에는 나오지 않는 것이 정상).
Server·트리거·Q 판정은 바뀌지 않는다. 초기 숨김 목록(1·4·5·7)에 8은 없어서 Area 로드부터 보인다.

게시: 게시 전 `Publish-MapAuthoring.ps1 -Scope Area -Mode Check` exit 0(3369 배치, 8파일)으로 다른 미게시 변경이 섞이지 않음을 확인했다. 이어서 Validate → Publish → Check를 실행했다: 세 모드 모두 exit 0(PlacementCount 3369, FileCount 8). 런타임 `Client/Bin/DataFiles/Map/LV_LUT_MIDNIGHTC_ED.deployassets`(14:52:15)는 헤더 5개에 `DEPLOY_ITR_10073` 1행, `.deployplacements`는 헤더 7개에 배치 8번 행을 포함한다.

상태: 망원경 **데이터·리소스 연결과 게시 완료**. Client 재실행 후 화면 확인 대기(신규 prototype이라 F1 창 재오픈만으로는 안 읽힌다). 화면 확인 전에는 원본 일치나 표시 PASS로 기록하지 않는다.

리소스 인계: `Client/Bin/Resources`는 Git 비추적이다. 다른 PC에서 망원경이 보이려면 `Map/LV_LUT_MIDNIGHTC_ED/AnimatedProps/DEPLOY_ITR_10073/` 폴더 전체(wmodel, textures 3장, cook.json)를 같은 경로로 전달해야 한다. 이 폴더 없이 새 `deployassets`만 받은 PC는 쿠크 아레나에 진입하지 못한다: `DeployPropCatalog.cpp` 행 검증이 wmodel 파일 실재를 요구하고, 실패하면 `CLoader::Ready_DeployPropArea`(`Ensure_AreaPrototypes`)와 `CLevel_KakulSaydonArena`의 `m_DeployRuntime.Load_Area`가 실패를 반환한다. 커밋·PR 전에 리소스 전달을 먼저 준비해야 한다.


## R6. 삐에로 상자 복원 — 사용자 지시 "원작처럼 맞으면 반응하는 방식으로 삐에로 상자 복원해줘" (2026-09-14 16:30~)

### 원본에서 확인한 사실

- 배치: `DeployData.loa`의 `CEFDeployActor_NPC` actor 51, NPC 480720, UE (0, -135168, 0). 망원경 Prop 378134와 같은 좌표다.
- NPC 행: Comment1 "미로 삐에로 상자", Model `EFDLChar_MN_RPPB_01.MN_RPPB_01`, DieType 2, DiePhysics 100, AiIndex 4710, AttackType 2. HP 값은 NPC 행에 없다.
- LookInfo(`data4.lpk`): 메쉬 `MN_RPPB_01.Mesh.MN_RPPB_01_SK`, 재질 `MN_RPPB_01_MI`/`MN_REUP_04_MI`, AnimSet `MN_RPPB_00.Ani.MN_RPPB_00_Ani`, 부착 파츠 `WP_MN_CMDUP_00_SK`(소켓 `WP_2_18`).
- 트리거 2201 "카드 미로_입장 후 깜짝 삐에로 상자 스폰, 타이머 시작": 볼륨 진입 → 타이머 150초, NPC 253 스폰 → `Action_Delay 1.0` → NPC 51 스폰.
- 트리거 2202 "카드 미로_삐에로 상자를 파괴해 망원경 스폰": 조건 `Condition_NPC Dead` 51 → 망원경 Prop 268435714 스폰과 웨이브 시작.

### 구현 완료 (코드·데이터·리소스, 빌드는 사용자)

| 영역 | 파일 | 내용 |
|---|---|---|
| 모델 | `Client/Bin/Resources/Character/KoukuSaton/MN_RPPB_01/` (wmodel, textures 6장, `MN_RPPB_01.cook.json`, info) | umodel glTF + PSA → `build_umodel_gltf_psa.py --scale 100` → ModelAssetConverter(두 재질 d/n/s 명시 remap, `--no-auto-textures`) → `retime_wmodel_ticks.py` 1000→30. 12클립 모두 30 ticks/s, 정점 13060, 뼈 63, weight 합 오차 4.5e-8, 높이 184.1cm. bind pose 전방 +X(toe0가 foot 앞)로 `CardMiro_Monster_Heart`와 같다 |
| catalog | `Data/Actors/MonsterCatalog.json` | `MONSTER_KOUKU_CLOWN_BOX`: scale 0.01, yaw -90(카드 병사와 같은 전방축), idle/chase `idle_normal_1`, hit `dmg_idle_1`(0.3333초), dead `dead_1`, attack `att_battle_1` |
| profile | `Data/Balance/MonsterProfiles.json` | maxHp 500, 반지름 0.6(메쉬 XZ 범위 약 ±0.6m), 카드 병사와 같은 정지·무공격 값(engage 0.01, move 0.01, attackPower 1), deadDespawnMs 3500(`dead_1` 3.333초 뒤) |
| 배포 | `Data/Worlds/LV_LUT_MIDNIGHTC_ED/SpawnGroups.world.json` | 잠자는 `spawn.kouku.cardmaze.profiles`에 1행 추가, maxAlive 4→5. `Publish-WorldGameplay.ps1` Validate·Publish exit 0, `KAKULSAYDON_ARENA.spawngroupsbootstrap`에 PROFILE/ENTRY 행 확인 |
| Server 상수 | `Server/Public/KoukuSaydonLogicRuntime.h` | `CLOWN_BOX_ARCHETYPE_ID`, `CLOWN_BOX_SPAWN_GROUP_TAG = "cardmaze.clownbox"`, `CLOWN_BOX_SPAWN_DELAY_TICKS = 30`(원본 Delay 1.0초) |
| Server 흐름 | `Server/Public/GameRoom.h`, `Server/Private/GameRoom_KoukuMiniGames.cpp` | INACTIVE에서만 `Update_CardMazeClownBox`: MAZE 생존자가 CardMiro 영역에 있으면 30tick 뒤 `cardmaze.telescope` 위치에 `Spawn_Monster`. 실패는 1초마다 재시도하고 망원경을 열지 않음. `Resolve_CardMazeHammerHit`는 시작 전 상자를 일반 `Apply_PlayerToWorld`로 때리고(몸통 안이면 방향 무관, 밖이면 2.4m+반지름·120°·높이 0.8m), 파괴 전에는 망원경 판정으로 넘어가지 않음. 다른 수단으로 죽어도 DEAD를 보고 해제. 영역에 MAZE 생존자가 없을 때, `Reset_CardMaze`, `Begin_CardMaze` 성공(Debug 우회 포함) 때 살아 있는 상자를 despawn하고 상태를 비움 |
| Client | `Client/Public/Level_KakulSaydonArena.h`, `Client/Private/Level_KakulSaydonArena.cpp` | Deploy 배치 8(망원경)을 Area 로드 때 숨김(배치가 있을 때만). 매 프레임 로컬 MAZE 모드 또는 역할 보유 시 `Collect_KoukuMazeTargets`로 살아 있는 상자를 찾고, 살아 있던 상자가 사라졌거나 역할이 배정되면 표시, MAZE를 벗어나면 기억을 지움. HUD: 상자 생존 중 `[ Q ] Break the clown box at the maze center`, 망원경 표시 중 기존 망원경 문구 |
| 테스트 | `Server/Private/ServerGameplayContractTests_CardMaze.cpp` | 기존 "진입 직후 Q로 망원경 시작" 단정을 새 흐름으로 교체: 첫 Q는 망원경을 열지 않음 → 30tick 안에 상자 스폰(archetype·spawn tag) → 매 타격 DAMAGE_EVENT와 INACTIVE 유지 속에 DEAD → 다음 Q로 HUNTING·목표 1개·관전 flag |
| 문서 | `CLAUDE.md`, `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md` | 망원경 문장(시각 모델 없음 → Deploy 8)과 상자 선행 계약 반영 |

### 자동 검증 (에이전트 실행분)

- JSON parse 3파일, 카탈로그 16행 모두 9키, 프로파일 16행 모두 22필드.
- `Publish-WorldGameplay.ps1 -Mode Validate`/`Publish` exit 0. 추적 Client 출력 변경 없음(`git status`).
- 격리 구문 컴파일 `cl /Zs /std:c++20`(프로젝트 중간 산출물 미사용): `GameRoom_KoukuMiniGames.cpp`, `ServerGameplayContractTests_CardMaze.cpp`, `Level_KakulSaydonArena.cpp` EXIT=0. 링크·실행·계약 테스트 실행은 하지 않았다.
- 인코딩: 수정 C++ 모두 CRLF 유지, Level 파일 BOM과 비ASCII 바이트 수(123/3) 변경 없음. `git diff --check` 통과.

### 판단·가정 (원본 값이 아님)

- **HP 500**: 원본 HP를 찾지 못했다. 망치 100 × 5타로 정한 프로젝트 값이다.
- **공격 없음**: NPC 행에 AI·공격 값이 있지만 원작 공격 여부·패턴은 확인하지 못했다. 카드 병사처럼 정지·무공격으로 만들었다.
- **idle은 `idle_normal_1`**: 원작이 normal/battle 중 무엇을 쓰는지 모른다. 등장 클립 `respawn_1`은 MonsterCatalog에 슬롯이 없어 재생하지 않는다.
- **사망 표현은 `dead_1`**: 원본 DiePhysics(물리 사망)는 지원 경로가 없다.
- **부착 파츠 `WP_MN_CMDUP_00` 미부착**: MonsterCatalog에 파츠 부착 계약이 없다.
- **위치**: 원본은 망원경과 같은 좌표다. 우리 망원경 트리거 (0.28, 1351.65)를 쓰며 Server가 네비 셀 중심으로 투영한다. 진입 텔레포트 지점이 같아 플레이어가 몸통 안에서 시작한다. 코드상 몸통 안에서는 바깥쪽 이동만 허용되어 갇히지 않고, 중심 거리가 engage 0.01m보다 커서 상자가 공격 대상을 잡지 않는다. 실제 체감은 사용자 확인 대상이다.
- **Client 망원경 표시**: Server 상태를 새로 복제하지 않고 기존 복제 정보로 추론한다. Debug 전체 despawn 직후 1초 동안 망원경이 잠깐 보였다 다시 숨을 수 있다. 상자 파괴 뒤 들어온 늦은 참가자는 역할 배정 전까지 망원경이 안 보인다.
- **Debug 경계**: 상자가 살아 있는 동안 같은 방에서 `CARD_MAZE_ENTER`를 다시 실행하면 목적지 충돌 검사(`Is_PlayerPositionClear`)가 상자 몸통 때문에 거부할 수 있다(코드 판독, 미실행).

### 미완료

- Server·Client 빌드와 계약 테스트 실행(사용자 빌드).

### 사용자 빌드·화면 확인 (대기)

1. Engine·Server·Client Build 후 Server/Client 재시작(Server bootstrap은 이미 게시됨).
2. 쿠크 아레나 → 카드미로 MAZE 진입 → 1초 뒤 중앙에 삐에로 상자, HUD `[ Q ] Break the clown box...`, 망원경 숨김.
3. Q 타격마다 피격 모션·데미지 숫자, 5타에 `dead_1` 후 약 3.5초에 사라짐, 망원경 표시와 망원경 문구.
4. 망원경 Q → 기존 카드미로 시작. 상자 크기·방향(yaw -90)·재질, 몸통 안 시작 시 이동 체감.

### 리소스 인계

`Client/Bin/Resources/Character/KoukuSaton/MN_RPPB_01/` 폴더 전체를 같은 경로로 전달해야 한다. 폴더가 없는 PC도 아레나 진입은 된다: `CClientReplication::Apply_WorldEntitySpawn`의 prototype 실패는 로그만 남긴다. 다만 상자가 보이지 않는다.
