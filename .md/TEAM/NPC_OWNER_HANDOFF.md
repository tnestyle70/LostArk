# NPC 배치·상호작용 인수인계서

인계일: 2026-08-26

이 문서는 NPC를 맵에 놓고, 화면에 세우고, 말을 걸어 동작시키는 작업 전체의 정본이다.
읽는 순서와 다른 문서의 역할은 `README.md`를 따른다. 여기서는 NPC만 다룬다.

## 1. 한 장으로 보는 데이터 흐름

NPC는 네 계층으로 나뉘고 각 계층의 정본이 다르다. 어떤 작업이든 먼저 "내가 지금 어느
계층을 고치는가"를 정하고 시작한다.

```text
[1] 무엇을 만들 수 있나 (archetype 정의)
    Data/Actors/NpcCatalog.json                    formatVersion 2, 75 archetype
        archetypeId / clientPresentationId / modelAssetId / animationSetId / idleClip / runtimeStatus

[2] 어디에 몇 마리 놓고 어떻게 행동시키나 (placement 인스턴스)
    Data/Worlds/<AreaId>/Gameplay.world.json       formatVersion 6, kind="npc"
        placementId / archetypeId / encounterId / idleClip / behavior /
        position / yawDegrees / enabled
        <- Debug Test -> Map Tool -> World Gameplay 탭에서 저작

[3] publish (여기서만 런타임 문서가 바뀐다)
    Tools/WorldPipeline/Publish-WorldGameplay.ps1  Server pre-build가 강제 실행
        -> Server/Bin/DataFiles/World/<WorldId>.worldbootstrap v7
        -> Client/Bin/DataFiles/World/<WorldId>.npcpresentation.json v2

[4] 런타임
    Server  bootstrap -> SERVER_WORLD_ENTITY(kind=NPC) -> Shared spawn/snapshot
    Client  CClientReplication -> CActorCatalog::Find_Npc
            -> CNpcPresentationAssetService::Ensure_Prototypes  (모델·애니셋 on-demand 등록)
            -> CNpcPlacementPresentationService::Try_Get_Presentation
               (placement별 idle/walk/action clip binding)
            -> CNpc  (Prototype_GameObject_Npc, Server transform/action 표현)
```

핵심 규칙 세 가지다.

- catalog는 "정의", placement는 "인스턴스"다. 섞지 않는다.
- `Client/Bin/DataFiles`와 `Server/Bin/DataFiles`는 publisher만 쓴다. 직접 편집 금지.
- 저장 ID는 `archetypeId`와 `placementId`뿐이다. prototype tag, 포인터, vector index를
  저장 계약으로 쓰지 않는다.

## 2. 지금 실제로 되는 것과 안 되는 것

되는 것.

```text
NpcCatalog 75 archetype, 전부 runtimeStatus=supported
  모델 75개 + 공유 애니셋 파일 전부 Client/Bin/Resources에 실재 (2026-08-26 실측, 누락 0)
Map Tool에서 archetype 드롭다운으로 배치 / 이동 / yaw / idleClip 선택 / 삭제 / 저장
publisher가 catalog 참조·필드 정확일치·clip 이름 규칙을 검증
Server가 NPC를 world entity로 세우고 Shared spawn/snapshot으로 복제
Client가 archetype별 모델·애니셋을 on-demand 등록해 CNpc로 표현
Map Tool에서 stationary/patrol/wander, waypoint, 생활 action과 일괄 배치 저작
Server가 navigation 위에서 NPC 위치·방향·semantic action을 fixed tick으로 결정
Client가 snapshot transform을 보간하고 action edge에 실제 clip을 재생
Bern에서 가이드 NPC 우클릭 -> 확인창 -> 발탄 입장 (2026-08-26 배선 완료, 육안 검증 대기)
```

안 되는 것 / 아직 없는 것.

```text
범용 대화·퀘스트 시스템 없음. 상호작용은 발탄 입장 확인창 한 종류뿐이고 NPC ID가 코드에 박혀 있다
NPC nameplate(이름표) 없음
Bern은 전체 placement 16개 중 NPC 10개다. v6 초기 이관에서는 10개 모두 `behavior: null`이라
  pre-authored 순찰·생활 행동 샘플은 없다
Valtan / Character Select / Training Ground에는 NPC placement 0개
encounterId 필드는 NPC row에 존재하지만 제품 경로에서 소비되지 않는다
```

## 3. NPC 한 마리 배치하기 (가장 흔한 작업)

Server를 띄운 상태로 Client를 실행하고 로비에서 `Test`를 누른다.

```text
1. Map Tool 상단 Area 콤보에서 대상 Area 선택
2. World Gameplay 라디오 선택
3. kind 라디오에서 NPC 선택
4. NPC Archetype 드롭다운에서 archetype 고르기
   (목록은 NpcCatalog.json의 runtimeStatus=supported 행이다)
5. Placement ID 입력 - 안정적인 이름을 쓴다. 예: npc.bern.market.guard.01
6. Arm World Placement 누르고 맵에서 위치 클릭
7. 필요하면 Selected Gameplay Placement에서 position / yaw / idleClip 조정
8. 행동이 필요하면 Enable Behavior를 켜고 mode/path/action을 편집한다
9. Apply NPC Behavior
10. Save Gameplay
11. Publish-WorldGameplay 또는 Server 재빌드
12. 실행 중인 Server를 재시작한다
```

`Apply -> Save -> Publish -> Server restart` 중 하나라도 빼먹으면 화면 draft, authoring 원본,
runtime 생성물, 실행 중 room의 상태가 서로 달라진다. 저장은 됐는데 제품 Bern에서 움직이지 않는
가장 흔한 원인이다.

`idleClip`을 비우면 catalog의 기본 clip(`npc_idle_normal_1`)을 쓴다. 채우면 그 placement만
다른 대기 동작을 쓴다. `behavior: null`은 기존 정적 idle NPC와 같은 의미다. 현재 Bern 10개는
초기 이관 상태가 모두 `behavior: null`이므로 행동 확인을 하려면 하나를 선택해 직접 저작한다.

## 4. 새 NPC archetype 추가하기 (모델부터)

이 경로는 쿠킹 스크립트가 저장소 밖에 있다. 인수인계에서 가장 중요한 항목이다.

```text
위치: C:\Users\95jus\Desktop\buildScript\   (JS 개인 PC)
  build_npc.py          아키타입 psk -> 병합 메시 .wmodel
  build_npc_animset.py  rig psk + psa 여러 개 -> 공유 애니셋 .wmodel
```

이 두 스크립트를 JS에게 받아 두지 않으면 새 NPC를 만들 수 없다. 인계받는 즉시 확보한다.

애니셋 공유 구조는 이렇다. 같은 rig를 쓰는 NPC들은 몸 메시만 가진 작은 `.wmodel`을 쓰고,
클립은 `Character/NPC/AnimSets/<Rig>/<Rig>.wmodel` 한 벌을 공유한다. Engine `CModel`이
로드 시 `skeletonHash`를 들고 있다가 `Attach_AnimationSet`에서 ANIM 타입·본 개수·해시를
전부 대조하고, 하나라도 어긋나면 그 archetype 전체를 fail-closed한다.

여기 함정이 하나 있다.

```text
컨버터의 본 테이블이 "메시 노드 이름"을 포함한다.
따라서 병합 메시 이름이 다르면 같은 rig라도 skeletonHash가 어긋나 attach가 거부된다.
build_npc.py가 메시 이름을 상수 npc_body로 고정하는 이유가 이것이다.
새로 쿠킹할 때 이 상수를 바꾸지 않는다.
```

현재 rig 구성.

```text
HM_MA01 (45클립)  포먼, 슈미트 등
HM_FE04 (33클립)  아일라라 등
자체 rig 내장      베다 (animationSetId = null, 1.85MB)
```

추가 절차.

```text
1. 스크립트로 모델(.wmodel) 쿠킹, 필요하면 애니셋도 쿠킹
2. validate_wmodel 통과 확인, 공유 애니셋을 쓸 거면 skeletonHash 일치 확인
3. Client/Bin/Resources/Character/NPC/... 에 배치 (팀장이 관리하는 물리 폴더)
4. Data/Actors/NpcCatalog.json에 6필드 정확히 맞춰 row 추가
   archetypeId / clientPresentationId / modelAssetId / animationSetId(null 가능) /
   idleClip / runtimeStatus
5. Publish-WorldGameplay.ps1 통과 확인
6. Map Tool 드롭다운에 뜨는지, Preview Clip에 클립이 나오는지 확인
```

`runtimeStatus`가 `supported`가 아니면 publisher가 archetype 목록에서 제외하고
Map Tool 드롭다운에도 안 뜬다. 작업 중인 archetype을 숨기는 스위치로 쓴다.

## 5. 상호작용 계약 (발탄 입장 확인창)

현재 유일한 NPC 상호작용이다. 2026-08-26에 주석 해제로 활성화했고 육안 검증은 아직이다.

```text
Client  CLevel_Bern::Ready_ValtanEntryNpcs        Gameplay.world.json에서 두 NPC 위치를 직접 읽음
        CLevel_Bern::Update_ValtanEntryInteraction 우클릭 지점이 NPC 3m 안이면 확인창 open
        CLevel_Bern::Render_ValtanEntryModal       ImGui 팝업이 패널·버튼 이미지를 그림
        CLevel_Bern::Render_ValtanEntryModalText   LOA 폰트 패스가 글자를 그림 (같은 Render 안, 팝업 뒤)
        입장 -> IPlayerCommandSink::Request_ConfirmNpcEntry
Shared  C2S_CONFIRM_NPC_ENTRY  (sequence, npcPlacementId)
Server  CGameRoom::Handle_ConfirmNpcEntry
          world가 BERN인지, 세션·플레이어 유효한지
          npcPlacementId가 허용 목록인지
          그 placement의 world entity가 실재하는지
          플레이어가 그 entity 3m 안인지
          HP > 0 이고 eAction == NONE 인지
          -> SERVER_WORLD_TRANSFER_REQUEST 로 VALTAN_ARENA 전송
```

허용 NPC ID는 Client와 Server 양쪽에 각각 하드코딩되어 있다. 늘리려면 두 곳을 같이 고친다.

```text
Client/Private/Level_Bern.cpp   GUIDE_NPC_PLACEMENT_IDS[]
Server/Private/GameRoom.cpp     VALTAN_ENTRY_GUIDE_NPCS[]   (ID + 목적지 WORLD_ID 쌍)
```

확인창 UI 계약.

```text
레이아웃  Data/UI/Bern/BernValtanEntry_Layout.json   (CProjectDataRoot 기준)
슬롯      ValtanEntry_Panel / ValtanEntry_ConfirmButton / ValtanEntry_CancelButton
이미지    UI/ClassSelect/Common/CreateCharacterModalPanel.png
          UI/ClassSelect/Common/NormalButton.png / NormalButtonHover.png
          (CRuntimeAssetRoot 기준, Client/Bin/Resources/UI/...)
```

슬롯 ID가 저장 계약이다. 레이아웃에서 슬롯 이름을 바꾸면 코드도 같이 고쳐야 한다.

중요: 이 상호작용을 쓰려면 같은 Area의 자동 이동 triggerBox를 꺼야 한다.
`CServerTriggerSystem`은 `isEnabled=false`인 triggerBox를 건너뛴다. Map Tool에서 해당
triggerBox의 `Enabled` 체크를 풀고 저장한 뒤 Server를 재빌드한다. 켜 두면 NPC에 다가가는
것만으로 자동 입장돼서 확인창이 의미가 없어진다.

## 6. 함정 모음

```text
[문서가 낡았다] AGENTS.md / CLAUDE.md / AREA_DATA_LAYER_GUIDE.md가
  "NPC presentation은 NPC_BEDA 한 archetype만 지원"이라고 적고 있었다.
  실제 target catalog는 75 archetype 전부 supported다. 2026-08-26에 교정했다.
  앞으로도 문서와 코드가 다르면 코드·데이터를 먼저 실측한다.

[Apply/Save만 하고 끝냈다] Gameplay.world.json 저장은 authoring일 뿐이다.
  Publish 뒤 실행 중 Server를 재시작해야 새 room에 반영된다.

[skeletonHash 불일치] 공유 애니셋 attach 실패는 해당 archetype 전체를 fail-closed한다.
  화면에 아무것도 안 나오면 병합 메시 이름(npc_body)부터 의심한다.

[클립 이름 충돌] 같은 rig에 ani_* 그룹을 더 붙일 때 클립 이름 dedupe가 필요하다.
  Attach_AnimationSet은 이름 충돌을 전체 선검증 후 통째로 거부한다(부분 커밋 없음).

[확인창 거리 불일치] 확인창은 "클릭한 지점"이 NPC 3m 안이면 열리지만,
  Server 승인은 "캐릭터 위치"가 3m 안일 것을 요구한다.
  멀리서 클릭하면 창은 뜨는데 입장이 조용히 거부된다. 아직 안 고쳤다.

[Engine public header] 애니셋 attach는 Engine 공개 헤더를 건드린 변경이었다.
  머지 후 각자 UpdateLib.bat을 다시 돌려야 Client가 새 헤더를 본다.
```

## 7. 검증

```powershell
powershell -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-WorldGameplay.ps1
```

catalog 참조, 필드 정확일치, idleClip 이름 규칙, placement 참조를 한 트랜잭션으로 검사한다.
Server pre-build가 같은 스크립트를 강제로 돌리므로 Server 빌드 성공 자체가 1차 검증이다.

그다음은 실행 확인이다.

```text
1. Ctrl+F5로 Server + Client 실행
2. 로비 -> Bern 진입 (Bern은 Character Create로 만든 캐릭터가 있어야 들어간다)
3. 배치한 NPC가 그 자리에 서 있고 설정한 순찰·배회·생활 동작이 도는지
4. 같은 action clip을 새 start tick에서 다시 시작하는지, 없는 clip은 그 NPC만 idle로 돌아가는지
5. 가이드 NPC 우클릭 -> 확인창 -> 입장 -> 발탄으로 넘어가는지
```

에이전트는 Client를 대신 실행·조작하거나 화면을 판정하지 않는다. 최종 육안 판정은 담당자 몫이다.

## 8. 다음에 할 만한 것

우선순위 순이다.

```text
1. 확인창 거리 판정을 캐릭터 위치 기준으로 맞추기 (위 함정 항목)
2. NPC ID 하드코딩을 데이터로 빼기
   - Gameplay.world.json NPC row에 interaction 종류와 목적지를 저작하고
     Server가 그 값을 검증하도록 바꾸면 코드 수정 없이 NPC를 늘릴 수 있다
   - 지금 비어 있는 encounterId 필드가 이 확장의 자연스러운 자리다
3. 범용 대화 시스템 (여러 줄 대사, 분기, 퀘스트 훅)
4. NPC nameplate - 플레이어 nameplate가 CClientReplication에 이미 있으니 그 경로를 재사용
5. Bern 외 Area에 NPC 배치
```

2번을 하기 전까지는 NPC를 늘릴 때마다 Client/Server 양쪽 배열을 같이 고쳐야 한다는 것을
기억한다. 한쪽만 고치면 확인창은 뜨는데 서버가 조용히 거부한다.

## 9. 원본 작업 기록

```text
.md/JS/08-12/2026-08-12_NPC_PLACEMENT_TOOL_PLAN.md / _RESULT.md      Map Tool NPC 배치
.md/JS/08-12/2026-08-12_NPC_PLACEMENT_IDLECLIP_PLAN.md / _RESULT.md  placement별 idleClip
.md/JS/08-12/2026-08-12_NPC_SHARED_ANIMSET_PLAN.md / _RESULT.md      공유 애니셋과 skeletonHash
```

커밋 기준 원작성자는 catalog·Map Tool·애니셋이 JS(2026-08-12~13), 발탄 입장 확인창이
Taejun(0de73b08, 2026-08-20)이다. 세부 구현 의도는 위 문서와 커밋을 먼저 읽는다.
