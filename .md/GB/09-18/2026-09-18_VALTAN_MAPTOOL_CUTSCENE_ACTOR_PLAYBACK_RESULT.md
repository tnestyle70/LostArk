# 2026-09-18 발탄 Map Tool 컷신 배우 동시 재생(G05) RESULT

## 목표

F1 → Map Tool → 발탄 Area → Camera 에서 컷신을 골라 Play 하면 아레나에 발탄(또는 늑대) 모델이
나타나고, 그 컷신의 원본 애니메이션이 카메라와 같은 세션 시계로 재생된다. 멈추면 같은 시각에
같이 멈춘다. Save → Reload 후에도 유지된다. 전투 패턴·사망 카메라는 바꾸지 않는다.

## 결과 요약

- 5개 컷신 전부 World 배우가 붙었다(발탄 4개 + 1관문 입장 늑대 1개). 배우 인스턴스 7개.
- 카메라 문서 `worldInstanceIds` 5개 채움, World Sequence 문서 신규 작성·publish 완료.
- 코드 변경은 World Object 리소스에 `animationSetAssetId` 하나를 추가한 것뿐(발탄 몸체는 clip 이
  별도 AnimSet WModel 에 있어서 붙여야 한다). Map Tool 세션 코드는 기존 fork 구현을 그대로 쓴다.
- 격리 컴파일 2 TU 통과, publisher Validate/Publish 통과, `git diff --check` 통과.
- **Client 실행·화면 확인은 하지 않았다.** 사용자가 Client Build 후 직접 확인한다.

## 바뀐 파일

| 파일 | 변경 |
|---|---|
| `Client/Public/WorldSequenceDocument.h` | `WORLD_SEQUENCE_OBJECT_RESOURCE::animationSetAssetId` 추가 (+4줄, 88~92행 부근) |
| `Client/Private/WorldSequenceDocument.cpp` | 파서 optional 필드 허용(437행), 파싱(508~513), 저장(1027~1028), 모델 없는 group 금지(1281), 검증(1332~1335), 동등성(1842) |
| `Client/Private/WorldSequencePlayer_Objects.cpp` | `Same_ObjectModelInputs` 에 포함(251), 모델 admission 직후 AnimSet 을 `CModel::Attach_AnimationSet` 으로 prototype 에 부착(441~455) |
| `Tools/MapPipeline/Publish-MapAuthoring.ps1` | objectResources optional 에 `animationSetAssetId` 허용, alias/비animated/group 이면 거부, Resources 상대 `.wmodel` 경로 검사 |
| `Data/Maps/MapCatalog.json` | `LV_LUT_HEARTRB_ED` 에 `sourceSequences`/`sequences` 선언 추가 |
| `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.worldsequences.json` | 신규(formatVersion 3, 리소스 3·template 7·instance 7) |
| `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.camerashots.json` | 5개 `cutscenes[].worldInstanceIds` 채움 |
| `Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.worldsequences.json` | publisher 출력(Git 미추적, 저작본과 바이트 동일, SHA256 `1b8d0501…`) |

건드리지 않은 것: `Data/Valtan/*`, `Data/Encounters/Valtan/*`, `valtan_tuning_pipeline.py`,
`Gameplay.world.json`, `Client.vcxproj`/`.filters`, MapTool 소스.

## 데이터 근거

원본 매티니의 배우 그룹(`interpgroup`)에서 활성 `interptrackanimcontrol` 키와 Move 트랙 월드 포즈를
그대로 뽑았다(`out/ValtanCameraReplace20260917/dump_actor_tracks.py` → `ORIGINAL_ACTOR_TRACKS.json`,
`resolve_more.py` → 메쉬 import 이름·drawscale). 컷신별 원본 배우:

| 컷신 | 원본 그룹 | 원본 메쉬 | drawscale | 저장소 리소스 |
|---|---|---|---|---|
| 1관문 입장 | 흰늑대, 검늑대 | `mn_rprs_02_sk` | 1.2 | `Character/Monster/NPC_480005_MN_RPRS_02/…wmodel` (루가루, clip 내장) |
| 발탄 등장 | 마수군단장발탄·발탄동기화 | `mn_rpbf_01_sk_loc_int` | 1.4 | `Character/Valtan/MN_RPBF_01.wmodel` + `AnimSets/MN_RPBF_01_AnimSet.wmodel` |
| 발탄 등장 | 발탄무채색 | `mn_rpbf_00_sk` | 1.4 | **없음** → 유령 몸체 `Ghost/MN_RPBF_02.wmodel` 로 대역 |
| 발탄 최후 | 발탄 | `mn_rpbf_02_sk` | 1.4 | `Character/Valtan/Ghost/MN_RPBF_02.wmodel` + Ghost AnimSet (원본과 같은 메쉬) |
| 버러지·포효 | 발탄 | `mn_rpbf_01_sk_loc_int` | 1.4 | 본체 + AnimSet |

스케일: 발탄 본체 `modelPreScale 0.0001`·유령 `0.01` 은 `BossCatalog` 의 `bodyModelPreScale`,
`scale 1.4` 는 `presentationScale` 이자 원본 actor drawscale. 루가루 `0.01` 은 `MonsterCatalog.modelScale`,
`scale 1.2` 는 원본 drawscale. yaw: 발탄은 원본 UE yaw 그대로(등장 끝 −135 = 배치 225 와 일치),
루가루는 UE yaw −90(`MonsterCatalog.modelYawDegrees` 를 키에 접음).

### 컷신별 클립 체인(시작 ms, 클립, 재생 배율, loop)

- **1관문 입장(늑대) 13000ms** — 흰늑대: 0 idle_battle_1(loop) → 3567 att_battle_12_02 ×0.8 → 4500 att_battle_16_03 → 5167 att_battle_13_01(sourceStart 400) → 6600 att_battle_21_03 ×0.8. 3.6s 에 나타남.
  검늑대: 0 idle_battle_1(loop, 편집 filler) → 5733 att_battle_4_01 → 9833 att_battle_16_03. 7.5~10.3s 만 보임(원본 주차 위치 y 112m).
- **발탄 등장 24708ms** — 본체(13.0s 부터 보임): 0 idle_normal_1(loop) → 3106 abn_groggy_1_loop(loop) → 4905 abn_groggy_1_end ×0.3 → 10593 idle_battle_1(loop) → 12059 idle_normal_1(loop) → 15059 att_battle_5_01_end → 18645 walk_normal_1(원본 역재생) → 19493 idle_battle_1 ×0.89(loop).
  무채색(유령 대역, 13.0s 까지 보임): 같은 체인. 위치는 바닥(156.81, 23.09, −121.97) 고정, yaw 45.
- **발탄 최후 23000ms** — 유령 몸체 고정(156.57, 23.12, −122.43) yaw −135: 0 abn_groggy_1_start → 1828 abn_groggy_1_loop(loop) → 4422 abn_groggy_1_end ×0.6 → 5566 att_battle_5_01_start → 7477 evt1_att_battle_5_01_end ×0.7 → 8609 dead_1 → 끝까지 hold.
- **버러지 6374ms** — 본체(156.17, 23.02, −121.80) yaw −135: 0 att_battle_13_02 → 2274 att_battle_13_02-1. 마지막 1ms 는 원본 주차 이동이라 숨김.
- **포효 7003ms** — 본체 60키(4.0~6.5s 에 y 23→85m 도약 후 착지): 0 att_battle_12_04(sourceStart 2644, 원본은 −2.64s 시작) → 1 att_battle_12_05.

### 편집 판단(원본 데이터가 아닌 것)

1. 슬롯 블렌딩 없음: 원본은 슬롯 a/b/up_a/up_b 를 동시에 섞지만 플레이어는 한 슬롯 체인이라 시작 시각 순으로 한 줄로 합쳤다(같은 시각은 앞 슬롯 우선).
2. `breverse`(등장 18.645s walk_normal_1) 는 정방향 재생.
3. 등장 컷신: 무채색은 13.0s 까지, 본체는 13.0s 부터만 보이게 잘라 같은 자리 겹침을 피했다. 18.5~19.0s 의 두 본체 교차(하나 가라앉고 하나 솟음)는 건너뛰고 바닥 자세 유지. 원본의 재질 파라미터(색 복원) 트랙은 재현하지 않는다.
4. 검늑대 체인 앞에 idle_battle_1 filler(첫 트랙은 0ms 필수).
5. 주차 위치(y>100 또는 y<0)는 숨김 키로 처리. 발탄동기화(버러지·포효)는 먼 곳에서 끝에 순간이동하는 핸드오프 배우라 넣지 않았다.

## 검증

- JSON parse: worldsequences(리소스 3·template 7·instance 7)·camerashots·MapCatalog 모두 OK.
- 클립 전수 대조: 문서의 34개 clipName 이 각 wmodel/AnimSet 에 실재(누락 0). 모델·AnimSet 경로 5개 실재.
- publisher: `Publish-MapAuthoring.ps1 -AreaId LV_LUT_HEARTRB_ED -Scope WorldSequences` Validate OK, Publish OK(FileCount 1). runtime 과 저작본 바이트 동일.
- 격리 컴파일: `WorldSequenceDocument.cpp`, `WorldSequencePlayer_Objects.cpp` 를 `out/IsolatedCompile20260918/` 로 `/Z7` 컴파일 → 둘 다 OK(경고는 기존 C4819 뿐). VS 빌드와 겹치지 않음(cl/link/fxc 0개 확인 후 실행).
- `git diff --check` OK. 편집한 C++/ps1/MapCatalog 는 CRLF·BOM 보존 확인.
- 미실행: Client 실행, Product 빌드(사용자 VS Build), 화면 확인.

## 사용자가 할 것

1. VS 에서 Client Build(Debug|x64, 증분). 헤더 `WorldSequenceDocument.h` 가 바뀌어 World Sequence 관련 TU 여러 개가 다시 컴파일된다.
2. Lobby → Valtan → F1 → Map Tool → Area 발탄 → Camera → 컷신 → **발탄 포효** 선택 → Play.
   정상이면 Time 이 흐르며 아레나 중앙(156, 23, −122)에 발탄이 나타나 포효 후 4.0s 에 도약, 7.0s 에 착지하고 PAUSED 로 멈춘다. 카메라는 0/1461/3621ms 에 컷이 바뀐다.
3. 발탄 최후: 유령 몸체가 그로기 → 8.6s 에 사망 → 마지막 자세 유지. 버러지: 13_02 → 13_02-1. 등장: 13.0s 까지 유령(무채색 대역), 이후 본체. 1관문 입장: 3.6s 흰늑대, 7.5~10.3s 검늑대.
4. 배우가 안 보이면 Camera 패널의 상태 문구(`World instance could not start: …`, `World Object …`)를 그대로 알려달라. 모델 admission 실패 사유가 거기 찍힌다.
5. Save → Reload: 카메라 문서 Save 가 `worldInstanceIds` 를 그대로 다시 쓰고, World 문서는 Object Tool Save 가 `animationSetAssetId` 를 보존한다.

## 실수·주의

- 첫 생성에서 검늑대 filler 클립 이름을 두 번 포맷해 `…ao_mn_rprs_02_sk.ao_idle_battle_1` 로 만들었다 → 수정 후 재생성.
- 등장 본체를 19.0s 에서 나누자 18.7~19.0s 에 바닥 아래로 잠깐 사라졌다 → 교차 구간을 건너뛰도록 수정.
- 격리 컴파일 .bat 을 UTF-8 로 써서 한글 경로가 깨졌다 → cp949 로 다시 씀. PCH(/Yu)+PDB 충돌을 피하려고 /Z7·PCH 미사용으로 컴파일했다(옵션은 tlog 의 실제 명령줄 기준).
- publish 를 다른 명령과 `&&` 로 묶었더니 앞 단계 종료 코드 때문에 실행되지 않았다 → 단독 실행으로 확인.
- 부활 컷신의 무채색 발탄(`mn_rpbf_00`)은 저장소에 자산이 없어 유령 몸체로 대역을 세웠다. 색·재질은 원본과 다르다.


---

## 후속 수정 — 배우가 화면에 나오지 않던 원인 (2026-09-18)

사용자가 빌드 후 실제로 실행해 `발탄 최후` 를 Play 했을 때, 카메라와 세션 시계는
돌지만 발탄 모델이 전혀 나오지 않았고 화면에는 오류 문구도 없었다.

### 근본 원인 — World Object 프로토타입이 발탄 Level 에 등록되지 않았다

배우는 `Apply_Objects` 안에서 지연 생성된다.

```
Client/Private/WorldSequencePlayer_Objects.cpp:912-915
    if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(targets.levelIndex,
        CWorldSequenceObject::PROTOTYPE_TAG, targets.levelIndex,
        CWorldSequenceObject::LAYER_TAG, &desc, &staged)))
    { m_Status = "World Object clone/shader creation failed: " + ...; return false; }
```

`CWorldSequenceObject::PROTOTYPE_TAG` 을 등록하는 곳은 저장소 전체에서 두 군데뿐이다.

- `Client/Private/Level_KakulSaydonArena.cpp:1213-1215` — `ETOUI(LEVEL::KAKULSAYDON_ARENA)` 에만 등록
- `Client/Private/MapTool_Cutscenes.cpp:1262` (수정 전) — `Play_CardMiroMarch` 안에서만,
  그것도 `!m_bWorldObjectPrototypeReady && !m_bRuntimeAuthoring` 조건으로 등록

`Client/Private/Level_ValtanArena.cpp` 에는 `Add_Prototype` 호출이 하나도 없다(실측).
따라서 Map Tool 이 발탄 아레나에 런타임 attach 한 상태에서는

- `m_bRuntimeAuthoring == true` 라 툴이 등록을 건너뛰고,
- 발탄 Level 도 등록하지 않았으므로,
- `Clone_Prototype(VALTAN_ARENA, PROTOTYPE_TAG)` 이 null 을 돌려주고 배우가 만들어지지 않는다.

`!m_bRuntimeAuthoring` 은 "Level 이 이미 등록했다" 의 대용 조건이었는데, 실제로 등록하는
Level 은 쿠크뿐이라 발탄에서 틀린 판정이 됐다. 또한 컷신 경로(`Prepare_EditorCutsceneWorld`)
는 등록 코드를 아예 갖고 있지 않았다(등록은 `Play_CardMiroMarch` 안에만 있었다).

셰이더 쪽은 문제가 아니었다. `CWorldSequenceObject::Initialize` 가 요구하는
`Prototype_Component_Shader_VtxAnimMeshBinary` 는 `Ready_For_ValtanArena` →
`Ready_Character_Rendering(ETOUI(LEVEL::VALTAN_ARENA), ...)` → `Ready_AnimatedMeshShader`
(`Client/Private/Loader.cpp:1183-1196, 1276`)로 발탄 Level 에 이미 등록돼 있다.

### 왜 오류 문구가 안 떴나

`Seek_EditorCutsceneWorld` 가 결과를 버렸다.

```
(수정 전) (void)m_ArenaRisePlayer.Seek_AllToMs(m_fCutsceneSessionMs, targets);
```

`Seek_AllToMs` 는 적용에 실패한 인스턴스를 `Stop_Instance` 로 내리고 false 를 돌려주는데
(`WorldSequencePlayer.cpp:936-961`), 그 false 를 버렸기 때문에 플레이어가 기록한
`"World Object clone/shader creation failed: ..."` 가 `m_CutsceneStatus` 로 올라오지 못했다.
화면에는 `Play_EditorCutscene` 이 마지막으로 쓴 `"Playing 발탄 최후"` 만 남았다.

요약줄의 `World %zu개` 는 `cutscene.worldInstanceIds.size()`(문서의 문자열 수)를 셀 뿐이라
(`MapTool_CameraShots.cpp:1182-1184`) 실제 생성 성공 여부와 무관하다. `World 1개` 표시는
정상 판정 근거가 아니었다.

### 고친 내용

| 파일 | 변경 |
|---|---|
| `Client/Public/MapTool.h` | `Ensure_WorldObjectPrototype()` 선언 추가(`Build_CutsceneTargets` 아래) |
| `Client/Private/MapTool_Cutscenes.cpp` | `Ensure_WorldObjectPrototype()` 구현 추가(`Build_CutsceneTargets` 정의 뒤) |
| `Client/Private/MapTool_Cutscenes.cpp` | `Prepare_EditorCutsceneWorld` 가 Load/Play 전에 프로토타입을 보장하고, 실패를 `m_CutsceneStatus` 에 남김 |
| `Client/Private/MapTool_Cutscenes.cpp` | `Seek_EditorCutsceneWorld` 가 `Seek_AllToMs` 실패를 `m_CutsceneStatus` 로 보고하고 세션의 World 준비 플래그를 내림 |
| `Client/Private/MapTool_Cutscenes.cpp` | `Play_CardMiroMarch` 의 인라인 등록 블록을 같은 헬퍼 호출로 교체 |

`Ensure_WorldObjectPrototype()` 은 `m_bWorldObjectPrototypeReady` 로 Level 당 한 번만 등록하고
(플래그는 `MapTool_Area.cpp:1384` Level 전환에서 리셋된다), 쿠크 아레나 인덱스일 때만 Level 이
소유한 것으로 보고 등록을 건너뛴다. 조건을 `m_bRuntimeAuthoring` 이라는 대용값이 아니라 실제로
프로토타입을 등록하는 Level 로 바꾼 것이 수정의 핵심이다.

### 발탄 최후 기준 경로 추적

1. `Play_EditorCutscene` → `Prepare_EditorCutsceneWorld`
2. `Build_CutsceneTargets` — `levelIndex=VALTAN_ARENA`, catalog/placements/deploy/device/context 채움
3. **(추가)** `Ensure_WorldObjectPrototype()` → `Add_Prototype(VALTAN_ARENA, PROTOTYPE_TAG, ...)`
4. `m_ArenaRisePlayer.Load_Area("LV_LUT_HEARTRB_ED", targets)` — 게시본
   `Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.worldsequences.json`(리소스 3·template 7·instance 7)
5. `Play("world.sequence.instance.valtan.source-preview.finale")` — 모델 준비,
   `Character/Valtan/Ghost/MN_RPBF_02.wmodel`(42,166,008B)·
   `MN_RPBF_02_AnimSet.wmodel`(46,540,308B) 실재 확인
6. 매 프레임 `CMapTool::Update`(`MapTool.cpp:193`) → `Update_EditorCutscene` →
   `Seek_EditorCutsceneWorld` → `Seek_AllToMs` → `Apply_Instance` → `Apply_Objects`
7. `Add_GameObject_to_Layer(VALTAN_ARENA, PROTOTYPE_TAG, VALTAN_ARENA, LAYER_TAG)` 성공 →
   `CWorldSequenceObject` 가 `Layer` 에 올라가 현재 Level 의 `CObject_Manager` 가 Update/Render
8. 트랙 키 `timeMs 0 / visible true`, WORLD 앵커 `position [156.5747, 23.117, -122.4292]`,
   `scale [1.4,1.4,1.4]` → 아레나 중앙에서 그로기 → 8609ms `mesh_dead_1` 유지

### 검증

- 격리 컴파일: `MapTool_Cutscenes.cpp` (`/c /Z7 /std:c++20 /D _UNICODE /D UNICODE`,
  출력 `out/IsolatedCompile20260918b/`) **EXITCODE=0**, 오류 0건, 경고는 기존 C4819 뿐.
- `git diff --check` 경고 없음. 두 파일 UTF-8 noBOM·CRLF·lone LF 0 유지.
- 데이터 무변경. `Data/Valtan`, `Data/Encounters`, `Gameplay.world.json`, vcxproj 미수정.

### 확인하지 않은 것

- 실제 화면에 배우가 보이는지(사용자 전용). 코드·데이터 경로 추적까지만 했다.
- 발탄 최후 외 4개 컷신(등장·1관문·버러지·포효)도 같은 프로토타입을 쓰므로 함께 풀릴 것으로
  보지만, 각각의 실행 확인은 하지 않았다.

### 이번 수정에서 한 실수

1. 첫 격리 컴파일 .bat 을 만들 때 Python 문자열에 Windows 경로를 그대로 넣어 백슬래시+U
   이스케이프 SyntaxError 를 냈다. 세션에 이미 기록된 gotcha 인데 반복했다.
2. `/Fo` 에 디렉터리 경로를 주면서 끝 백슬래시 뒤에 따옴표를 붙여 `cl` 이 소스 파일명을
   삼켰다(D8003). obj 파일명을 명시해 해결.

## 후속 3 — 배우가 "청록 점무늬 조각"으로 보이던 원인과 수정 (09-18)

사용자 관찰: 컷신 재생 중 아레나 중앙 바닥 근처에 반투명 청록색, 화면문(dither) 점무늬, 몸 실루엣이 아닌 삼각형·스파이크 조각이 보였다. 프로토타입 등록 수정 뒤 "생성은 되는데 형상이 깨진" 단계였다.

### 측정

- K1 스키닝: `CModel::Attach_AnimationSet`(`Engine/Private/Model.cpp:2597-2622`)은 skeleton hash와 본 수가 다르면 `E_FAIL`이고, world sequence는 그 실패를 `World Object animation set does not match the body`로 막는다(`WorldSequencePlayer_Objects.cpp:452-461`). 클립 이름이 없어도 `World Object clip is absent`로 막는다. 배우가 그려졌다는 사실 자체가 두 검사를 통과했다는 뜻이라 본 팔레트 불일치는 기각. 제품(`ValtanPresentationAssetService.cpp:148-169`)도 같은 model+AnimSet+preScale 조합이다.
- K2 스케일: BossCatalog `BOSS_VALTAN_GHOST` bodyModelPreScale 0.01, `BOSS_VALTAN` 0.0001, presentationScale 1.4 — world sequence 리소스와 같다. preScale은 두 경로 모두 `CModel::Create` PreTransform에 한 번 굽힌다(world 행렬에서 다시 곱하지 않음, `WorldSequencePlayer_Objects.cpp:780-793`). 기각.
- K3 위치: Valtan navsource 앵커 셀 지면 22.9975(최후 앵커 셀 325,85). 앵커 y 23.117/23.086은 바닥 위 0.09~0.12m, 제품 `boss.valtan.center` y 22.99751과 같은 높이. 파묻힘 기각.
- K3 방향: 제품 발탄 몸체는 배치 yaw(225, `CTransform::Rotation` = Y축 쿼터니언)에 `CBody_Valtan::Initialize`의 `Rotation(0,-90,0)`을 더 적용한다(`Body_Valtan.cpp:50`). world sequence 키는 −135(=225)만 가지고 있어 제품보다 90° 틀어져 있었다. **방향 차이 확정.**
- K4 재질·패스: 유령 재질 3개는 전부 family `source.character.monster-8d18db0756e4.v1` = **program 84**(`SourceCharacterMaterialParameters.h:1729-1731`), 본체는 program 21. 제품 `CBody_Valtan`은 program 84 메시를 NONBLEND 패스 0에서 **건너뛰고**(`Body_Valtan.cpp:98`), BLEND 그룹에서 패스 10 `SourceCharacterTranslucentOneSided` + forward light로만 그린다(`Body_Valtan.cpp:122-145`). `CWorldSequenceObject`는 모든 스키닝 메시를 NONBLEND 패스 0으로 그렸다. 패스 0의 source character G-buffer 경로(`Shader_SourceCharacterMaterial.hlsli` `EvaluateSourceCharacterGeometry`)는 program 84를 **4×4 순서 디더 `clip(opacity − threshold)`** 대상에 넣는다. program 84의 opacity(`Shader_SourceCharacterBaseGroup084.hlsli` 20~41행)는 `pow(1−max(N·V,0),0.3)` 프레넬에 스크롤 노이즈 텍스처와 마스크 알파를 곱한 값이라, 디더를 통과하는 픽셀이 가장자리·노이즈 고점에만 흩어진다. **스크린샷의 청록 점무늬 조각과 일치. 확정 원인.** 두 패스는 같은 `EffectSourceModelVS`를 쓰므로 스키닝 결과는 같고 PS·블렌드만 다르다.

### 수정

1. `Client/Public/WorldSequenceObject.h`, `Client/Private/WorldSequenceObject.cpp` — 제품 owner 규칙(CBody_Valtan 84 → 패스 10, CPart_Vehicle 18 → 9, 88 → 10)을 그대로 따른다. 스키닝 모델에서 이 program 메시는 NONBLEND에서 건너뛰고, 있으면 BLEND에도 등록해 `Render_Group(BLEND)` → `Render_Translucent()`가 forward light·재질·본 행렬을 바인딩해 해당 패스로 그린다. 반투명 실패는 별도 문자열에 남기고 `Get_RenderStatus()`가 불투명/반투명 중 비어 있지 않은 쪽을 돌려준다(기존 소비자 `WorldSequencePlayer_Objects.cpp:650, 935`가 그대로 실패를 읽는다). 정적 모델 경로는 변경 없음.
2. `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.worldsequences.json` — 발탄 본체·유령 템플릿 5개(entrance 24, entrance.colorless 6, finale 2, trash 4, roar 60 = 키 96개)의 순수 yaw 쿼터니언에 제품과 같은 −90°를 접었다(최후 −135 → 135 = 225 − 90). 루가루 두 템플릿은 이미 MonsterCatalog −90을 접었으므로 무변경. LF·3350줄 유지, 차이 384줄(키 96 × 값 2 × 전후).
3. `Publish-MapAuthoring.ps1 -AreaId LV_LUT_HEARTRB_ED -Scope WorldSequences` Validate → Publish. FileCount 1(world sequence 런타임만), 저작본 = 런타임본 SHA-256 `6c153e9e60eab124527d2bcdbc4e9362a9ecd297965d4a0ea5dd6326ce0a5cb4`. 조명·deploy 파일은 쓰지 않았다.

### 검증

- 격리 `/Zs`(`out/IsolatedCompile20260918d/`): `WorldSequenceObject.cpp` EXIT 0, `WorldSequencePlayer_Objects.cpp`(헤더 소비자) EXIT 0, 경고는 기존 C4819만. 저장소 IntDir/OutDir 미사용, 산출물 없음.
- `git diff --check` 두 C++ 파일 통과. C++ 두 파일 ASCII·CRLF 유지(패치 스크립트가 바이트로 확인).
- 화면 확인은 안 했다(사용자 몫).

### 남은 것

- 등장 본체는 13000ms 한 키에서 offset 0(y 10.99, 바닥 12m 아래)으로 보이기 시작해 13100ms에 +12.09로 올라온다. 100ms 동안 바닥 아래에서 솟는 구간이며 이번 증상과 무관해 손대지 않았다.
- BLEND 정렬은 Transform 컴포넌트 위치를 쓰는데 world sequence 오브젝트는 `m_World`로 그려 정렬 위치가 원점이다. 제품 `CBody_Valtan`도 파츠 로컬 Transform(원점)으로 정렬되므로 동작은 같다.
