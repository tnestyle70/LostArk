# 2026-09-18 베른 NPC 미표시 근본 원인 추적 RESULT

사용자 관찰: F6 자유 카메라로 돌아다녀도 베른성에 NPC가 보이지 않았다.
직전 RESULT(`2026-09-18_BERN_19NPC_NAVGRID_RELOCATION_RESULT.md`)의
"네비 격자 밖이라 걸어갈 수 없었다"는 설명은 자유 카메라 관찰을 설명하지 못하므로 기각한다.

## 1. 가설별 판정

### H1. Client가 다른 PC의 Server에 접속 — 기각

| 항목 | 실측값 |
|---|---|
| `Client/Default/Client.vcxproj.user` | `LOSTARK_SERVER_HOST=127.0.0.1` (Debug/Release 동일) |
| `Tools/Network/TeamLanEndpoint.json` | `serverHost 192.168.0.14` |
| 이 PC IPv4 | `192.168.0.14`, `192.168.137.1` |
| 오늘 Server 세션 | `server-session-14224.jsonl`(08:25:35), `server-session-30560.jsonl`(08:30:46) |
| 세션 peerAddress | 전부 `127.0.0.1` |

Client는 로컬 루프백으로 접속했고 이 PC 자체가 팀 endpoint다. 세션 로그의
`worldId 1`(= `WORLD_ID::BERN`, `Shared/Public/Network/PacketType.h:89`)이 08:25 세션에 존재하므로
사용자는 로컬 Server의 베른에 실제로 들어갔다.

### H2. Client NPC 카탈로그 미게시 — 기각

`Client/Private/ActorCatalog.cpp:1154`가 `ReadDocument(L"Actors/NpcCatalog.json", npcs)`를 호출하고,
`ReadDocument`(:43-53)는 `CProjectDataRoot::Resolve()`로 `Data/` 저작 정본을 직접 읽는다.
별도 publish 산출물이 없으므로 `Publish-WorldGameplay.ps1`만 실행한 것으로 충분하다.

### H3. Server가 NPC placement를 조용히 스킵 — 기각

`Server/Private/GameRoom_WorldEntities.cpp:155`의 `if (WORLD_BOOTSTRAP_KIND::BOSS == staged.eKind)`
블록 안에만 네비 검증이 있다(:266-300, 실패 시 `"Boss placement is outside server navigation"`).
**NPC는 네비 투영·거부 대상이 아니다.** 격자 밖이라도 저작 좌표 그대로 스폰된다.

bootstrap 행 형식도 신규/기존이 동일하다(탭 구분 10필드, active=1, behavior=0).

```
npc.bern.25001     npc NPC_25001 - 137.162415 53.9794579 -167.909286 0      1 0
npc.bern.src.29    npc NPC_25007 - 147.238    42.623     -70.688     178.88 1 0
```

Server는 `NpcCatalog.json`을 읽지 않는다(`Server/` 전체에 참조 0건).

### H4. Client 표현 경로 — 데이터 계약은 전부 통과

`ParseNpcs`(`ActorCatalog.cpp:695-`) 규칙을 그대로 구현해 현재 파일을 검사한 결과 **위반 0건**:

- `schema=lostark.npc-catalog`, `formatVersion=2`, 126 항목
- 필드 수 정확 일치(기본 6 + optional), `runtimeStatus="supported"` 전원
- `archetypeId`/`clientPresentationId` 중복 0
- `modelAssetId` 전원 `IsResourceId` 통과(`Character/` 시작 + `.wmodel` 확장자)

bootstrap이 참조하는 NPC 49행 전수 검사:

| 구분 | 행 수 | archetype 미등록 | 모델 파일 없음 |
|---|---|---|---|
| 신규 `npc.bern.src.*` | 19 | 0 | 0 |
| 기존 | 30 | 0 | 0 |

설치 구조도 기존에 동작하는 팀 NPC와 동일하다.

| 패키지 | wmodel 버전 | 텍스처 |
|---|---|---|
| `Npc_Beda`(팀) | WINT 1.0 | `textures/*.tga` 3개 |
| `Npc_MN_CNAB_00`(신규) | WINT 1.0 | `textures/*.tga` 3개 |
| `Npc_NP_SJWD_00`(신규) | WINT 1.0 | `textures/*.tga` 2개 |

5개 액터 카탈로그 전부 parse 성공(`CharacterCatalog` v4, `BossCatalog` v8, `NpcCatalog` v2,
`MonsterCatalog` v2, `VehicleCatalog` v4). `git status` 상 수정된 것은 `NpcCatalog.json` 하나뿐이다.
따라서 `CActorCatalog::Initialize()`(:1143-1172)가 전체를 비우는 경로에는 해당하지 않는다.

### H5. 필드 누락 / npcpresentation — 기각

`BERN.npcpresentation.json`은 22개 항목뿐이지만 이는 **behavior override만 담는 문서**다.
팀의 정적 NPC(`npc.bern.25001`, `npc.bern.aylara`, `npc.bern.beda.guide` 등)도 여기 없다.
신규 19개가 없는 것은 정상이다.

## 2. 타임라인 (파일 mtime 실측)

| 시각 | 사건 |
|---|---|
| 06:37:03 | 19개 모델 Resources 설치 |
| 06:42:57 | `NpcCatalog.json` 기록 |
| 06:44:31 | `Resource_Distribution_2026-09-18.txt` |
| 06:46:09 | 배치 RESULT 기록 → **revision 550 게시 완료** |
| 07:37 | `Server.exe` 재빌드 |
| ~08:23 | 베른 세션 마지막 수신 |
| 08:25:35 | 베른 세션 종료(worldId 1) |
| 08:39:42 | `Gameplay.world.json` 재배치 |
| 08:40:35 | **revision 551 게시** |

Server.exe가 07:37에 재빌드됐으므로 그 시점 이전의 Server 프로세스는 파일 잠금으로 남아 있을 수 없다.
사용자가 테스트한 Server는 07:37 이후 시작됐고, 그때 디스크의 bootstrap은 **revision 550(19명 포함)** 이었다.

즉 **Server는 19명을 스폰했고 NPC는 네비 필터를 받지 않는다.**

## 3. 남은 두 가능성과 조치

데이터·계약은 전부 통과하므로 남는 것은 런타임에서만 관측 가능한 두 가지다.

1. Client의 모델 준비(`CNpcPresentationAssetService::Ensure_Prototypes`) 실패
2. 자유 카메라 이동이 rev550 당시의 통로 밖 좌표까지 실제로는 닿지 않음

`Client/Private/ClientReplication.cpp:2328-2336`의 실패 경로는 **아무 기록 없이 `return false`** 였다.
발탄 컷신 배우 미표시와 같은 유형의 조용한 실패다. 이 자리에 진단 출력을 추가했다.

```cpp
OutputDebugStringA(("[NpcPresentation] placement " +
    spawned.strPlacementId + " archetype " +
    spawned.strArchetypeId + " is unavailable (" +
    (nullptr == actor ?
        "no catalog entry: " + CActorCatalog::Get_Status() :
        modelTag.empty() ?
            std::string("no model prototype tag") :
            std::string("model prototype preparation failed")) +
    ").\n").c_str());
```

이제 NPC가 안 나오면 Visual Studio 출력 창에 placement ID·archetype·실패 단계가 남는다.

## 4. 좌표 검증 — 이 저장소 베른성 기준

`Server/Bin/DataFiles/Navigation/LV_BER_BERNCASTLE.navgrid`를 직접 디코드했다.
헤더 20바이트(cols, rows, cell, originX, originZ) + walkable 플래그 1바이트 × 셀 + 높이 float32 × 셀.

```
cols=50 rows=347 cell=0.50 origin=(123.988, -175.938)
덮는 범위 x 123.99~148.99 / z -175.94~-2.44
walkable 7674 / 17350 셀
```

`playerSpawn`은 4개이며 `player_1 = [137.586, 42.250, -22.464]`이다.

### 현재(revision 551) 검증 결과

| 구분 | 인원 | walkable 아님 | 지면 높이 오차 >0.15m |
|---|---|---|---|
| 신규 19 | 19 | **0** | **0** |
| 기존 30 | 30 | 1 (`npc.bern.25029`) | 0 |

신규 19명은 전원 walkable 셀 위, 격자 지면 높이와 오차 0이다.
`npc.bern.25029`의 non-walkable은 팀이 기존에 놓은 배치이며 이번 작업과 무관하다.

`player_1` 기준 거리: 신규 3.2~150.8 m, 기존 40.2~145.9 m — **같은 대역**이다.

| placementId | 거리 | 좌표 |
|---|---|---|
| `npc.bern.src.72` | 3.2 m | 140.2, 42.3, -24.2 |
| `npc.bern.src.669` | 14.1 m | 126.7, 47.5, -15.2 |
| `npc.bern.src.34` | 15.0 m | 143.7, 50.7, -33.2 |
| `npc.bern.src.42` | 16.8 m | 138.7, 42.3, -5.7 |
| `npc.bern.src.33` | 22.4 m | 145.7, 47.2, -42.7 |
| … | … | … |
| `npc.bern.src.46` | 150.8 m | 139.7, 54.7, -172.7 |

### 테스트 당시(revision 550) 원본 좌표

60.5~137.0 m(`npc.bern.src.42`만 315.2 m)로 거리 자체는 기존 30명과 비슷했으나,
x가 58~246으로 통로(x 124~149) 바깥 좌우 60~100 m에 흩어져 있었다.

### 되돌릴지 판단

되돌리지 않는다. 원본 좌표는 걸어서 도달할 수 없는 구역이고,
지금 목적은 모델·텍스처·미니맵을 눈으로 확인하는 것이다.
원본값은 `Data/Worlds/LV_BER_BERNCASTLE/bern19.source-coordinates.json`에 보존돼 있으며,
베른 네비를 마을 전체로 다시 bake한 뒤 복원하면 된다.

## 5. 변경 사항

| 파일 | 변경 |
|---|---|
| `Client/Private/ClientReplication.cpp` | +12줄 (NPC 표현 실패 진단 출력) |

- 격리 구문 검사: `out/IsolatedCompile20260918c/` — **EXITCODE=0**, error 0건
  (경고는 격리 플래그로 `/utf-8`을 강제해 CP949 EngineSDK 헤더에서 나는 기존 C4828뿐)
- UTF-8 noBOM·CRLF 4450·lone LF 0 유지
- `git diff --check` 이 파일 경고 없음
- 데이터 파일은 이번 작업에서 수정하지 않았다

## 6. 확인하지 못한 것

- **팀이 손으로 놓은 기존 30명이 화면에 보이는지**는 사용자만 답할 수 있다.
  보인다면 원인은 신규 19명의 런타임 모델 준비 또는 좌표이고,
  안 보인다면 베른 NPC 표현 경로 전체의 문제다. 이 답에 따라 다음 조치가 갈린다.
- Client 실행·화면 판정은 하지 않았다.
