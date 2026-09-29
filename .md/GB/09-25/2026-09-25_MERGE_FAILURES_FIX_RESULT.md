# 병합 실패 항목 원인 재조사와 해결 (2026-09-25)

작성 상태: 진행 중 (A단계 조사 완료, 게이트 대기 중). 이 문서의 마지막 줄에 `FIX_FORK_DONE` 표식이 있으면 작업이 끝난 것이다.

기준: 브랜치 `codex/merge-main-0925`, 병합 커밋 `cd58d12b`(origin/main `ca02c873` 병합). origin은 2026-09-25 20:32에 `git fetch origin`으로 갱신했고 origin/main은 여전히 `ca02c873`이라 팀장이 더 새로 고친 커밋은 없다. 다른 원격 브랜치에도 Compositions 검증기를 고친 커밋은 없다.

## 1. 실패 항목 목록

직접 확인한 것과 병합 fork 결과 파일에서 읽은 것을 구분한다.

- 직접 확인: Compositions 게시 실패 (`shape` 필드).
- 직접 확인: 카드미로 계약 테스트 실패 2건.
- 직접 확인: Compositions 단위 테스트 3건 오류 (같은 `shape` 원인).
- 병합 fork 결과 파일에서 읽음: 쿠크 Product·raid 계약 테스트는 첫 시도가 제한 시간에 걸려 종료됐고(rc=124) 더 긴 제한으로 다시 도는 중이다. 이것은 실패가 아니라 미완료다.
- 미확인: 이전부터 알려진 `--kouku-object-overlap-contract-test`, `--world-playback-contract-test`, 기본 `--contract-test`의 실패군. 비교용 서버(origin/main)에서 실행 중이며 결과는 5절에 채운다.

## 2. 항목 1: Compositions 게시 실패

원인 (확정):
- 로그: `World Sequence source.templates[256].colliderTracks[0] field mismatch: missing=[] unknown=['shape']`
- 팀장 커밋 `88fa743d`(2026-09-25)가 `LV_LUT_MIDNIGHTC_ED.worldsequences.json`의 콜라이더 트랙 5개(templates 256~259)에 `shape: "CYLINDER"`를 넣고, `Tools/MapPipeline/Publish-MapAuthoring.ps1`, `Client/Private/WorldSequenceDocument.cpp`, `Tools/MapPipeline/test_world_sequence_authoring_contract.py`에는 `shape` 지원을 넣었다. 같은 커밋이 `Tools/CompositionPipeline`은 건드리지 않았다.
- `Tools/CompositionPipeline/composition_pipeline.py`의 `_validate_world_sequence_collider_tracks`(약 2636행)는 콜라이더 트랙에 필수 10필드와 선택 `attachmentBone`만 허용한다. 그래서 팀장 main 그대로도 Compositions 게시가 실패한다.
- 팀장의 마지막 Compositions 게시는 2026-09-24(`0c5cfaa5`)로 `shape` 도입 전이다. 팀장이 그 뒤로 Compositions 게시를 다시 돌리지 않은 것과 맞는다.
- 병합이 만든 문제가 아니다. 이 검증기는 병합 HEAD가 origin/main과 같고, `shape` 데이터도 origin/main에 이미 있다.
- 같은 원인의 단위 테스트 오류 3건이 현재 저장소에서 재현된다(29개 실행, 오류 3). 그중 하나는 `CompositionPipelineTests.setUpClass` 오류라서 그 클래스의 테스트가 아예 실행되지 못했다.
- 재현 명령 결과: `out/MergeFailuresFix20260925/logs/compositions_validate_before.txt` (rc=2), `unittest_composition_full_before.clean.txt`.

수정안 (`patches/01_composition_collider_shape.patch`):
- 선택 필드 `shape`를 허용하고 규칙을 Map 게시자와 클라이언트 코덱에 맞춘다. `shape`는 `BOX` 또는 `CYLINDER`(생략 시 BOX), `CYLINDER`는 X/Z 반지름이 같아야 하고(허용 오차 0.0001) `HOOK_CAPTURE`가 될 수 없다.
- 단위 테스트 `test_box_and_cylinder_shapes_follow_the_map_owner_rules`를 추가했다(허용 4건, 거부 5건).

저장소를 건드리지 않은 사전 검증:
- 수정 사본을 메모리에서 불러와 Compositions 단위 테스트 전체를 돌렸다: 54개 실행, 실패 0, 오류 0 (`logs/unittest_composition_full_proposed.clean.txt`). 기준선에서 준비 단계 오류로 실행되지 못하던 테스트들도 이제 실행되어 통과했다.
- 같은 수정을 넣은 게시를 저장소 밖 폴더(`out/MergeFailuresFix20260925/comp_publish_scratch`)로 시험했고 24초에 통과했다.

코디네이터 메모 정정:
- 항목 14가 고친 `Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json`은 Compositions 게시의 입력이 아니다. 게시 입력 목록 422개 중 이 파일은 0건이고 `KoukuSaydonComposition.json`, `KoukuSaydonEncounter.json`도 0건이다. 항목 14의 변화는 Compositions 게시본에 나타나지 않는다.
- 게시하면 바뀌는 파일은 3개다: `Bosses/Valtan.bosscomposition.json`, `Sequences/KoukuSaydonArena.sequencer.json`, `Composition.publish.receipt.json`. `Bosses/KoukuSaydonGate1.bosscomposition.json`과 `Sequences/ValtanArena.sequencer.json`은 바이트까지 같다. 바뀌는 내용은 전부 원본 해시·크기와 revision 값이다.
- 지난 게시 이후 내용이 바뀐 게시 입력 원본은 5개다: `Data/Effects/EffectCatalog.json`(336122→358176), `Data/Effects/V2/Authored/kouku.bingo.encore.fade.black.effectv2.json`(2690→2571), `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.camerashots.json`(1762063→1718760), `.../LV_LUT_MIDNIGHTC_ED.worldsequences.json`(13505276→13499931), `Data/Rendering/Authored/RenderingProfiles.json`(121019→122194). 마지막 것은 팀장의 렌더링 정본이며 게시가 읽기만 하고 수정하지 않는다. 앞의 카메라·월드시퀀스 두 개에는 병합 fork가 적용한 항목 11(미커밋)의 변경이 들어 있다.
- 비교 상세: `logs/compositions_diff_current_vs_scratch.txt`.

## 3. 항목 2: 카드미로 계약 테스트 실패 2건

실패 항목: "Q deals 500 and destroys the full-health clown box without claiming the telescope", "Center Q hit facing away starts solo telescope and actually spawns a target". 결과는 91 통과 / 실패 2.

원인 (확정, 실험으로 검증):
- 게시된 `Server/Bin/DataFiles/World/KAKULSAYDON_ARENA.spawngroupsbootstrap` 6행의 상자 프로필(`MONSTER_KOUKU_CLOWN_BOX`)이 최대 체력 587,993, 공격 1,843이다. 저작 원본 `Data/Balance/MonsterProfiles.json`은 500과 1이다. `Data/Balance/Profiles/Retail.balanceprofile.json`의 `monsters[5]`가 원작 NPC 480720의 값으로 덮어쓰고, 월드 게시자(`Publish-WorldGameplay.ps1`, 기본 프로필 Retail)가 그 값을 게시한다.
- 서버의 Q 망치는 고정 500 피해다(`GameRoom_KoukuMiniGames.cpp` 197~198행). 상자가 죽어야 `m_bCardMazeClownBoxDestroyed`가 켜지는데(241행) 587,993 체력은 한 번에 죽지 않는다. 그래서 첫째 검증(체력 500 확인과 파괴)이 깨진다.
- 상자가 살아 있으면 205행 조건에서 망원경 획득 경로로 넘어가지 못한다. 그래서 둘째 검증(솔로 망원경 시작)이 깨진다. 둘째 실패는 첫째의 결과다.
- 실험: 같은 비교용 서버 실행 파일을 데이터 두 벌로 실행했다(`LOSTARK_SERVER_DATA_ROOT`). 원본 데이터는 91 통과/실패 2, 상자 프로필 한 줄(6행)만 500과 1로 바꾼 사본은 93 통과/실패 0이다. 두 사본은 이 한 줄 외에 바이트까지 같다. 결과: `logs/exp_a_original_card_maze.txt`, `logs/exp_b_box500_card_maze.txt`.
- 병합이 만든 문제가 아니다. origin/main 소스(Server+Shared 155개 파일이 origin/main과 줄바꿈 정규화 후 완전 동일)로 만든 비교용 서버에서도 같은 2건이 실패한다.
- 실제 플레이에도 영향이 있다. Q 500 피해로 587,993 체력을 깎으려면 1,176번을 때려야 하고(587,993 ÷ 500, 방어력 0), 일반 공격 100 피해는 5,880번이다. 걸리는 시간은 Q 동작 길이(테스트가 확인한 2.5초 잠금)와 연타 간격에 좌우되어 이 문서는 시간을 추정하지 않는다. 다만 횟수만으로도 지금 origin/main 데이터에서 카드미로 망원경 게이트를 여는 것은 사실상 불가능하다고 판단한다(횟수는 계산값이고 실제 플레이로 확인한 것은 아니다).

이력:
- 상자와 체력 500(저작 원본): 2026-09-14 KCY, 커밋 `8a351ac8`. 체력 500은 원본 값이 아니라 프로젝트 값이라고 기록했다.
- Q 피해 500과 이를 확인하는 테스트: 2026-09-20 `b1a84dc4`.
- Retail 프로필과 생성기가 상자를 원작 값으로 넣음: 2026-09-22 `24869176`(생성기 `build_retail_balance_profile.py`의 `EXTRA_MONSTER_NPC_KEY`에 CLAUDE.md를 근거로 명시).
- 팀장 문서 `.md/GB/09-24/2026-09-24_KOUKU_GATE1_HEALTH_FLOW_RESULT.md` G08이 상자 체력 500→587,993 반영을 기록했으나 카드미로 게이트에 미치는 영향은 다루지 않았다.

미수정. 사용자 결정 필요. 두 설계 의도가 부딪히고 어느 쪽이 맞는지는 이 저장소의 근거만으로 확정할 수 없다(Retail 생성기는 상자를 의도적으로 포함했고, 카드미로 게이트 테스트는 500 체력을 의도적으로 검사한다). 선택지:
- 옵션 1 (권장): 상자를 Retail에서 뺀다. 저작 체력 500을 유지하므로 Q 500 피해와 테스트, 게이트 설계가 그대로 맞는다. 실험으로 테스트 통과가 확인됐다. 제안 패치 `patches/02_clownbox_retail_exclusion_PROPOSAL.patch`(Retail JSON 행 삭제, 생성기 표 비움). 적용하면 `Publish-WorldGameplay.ps1`을 다시 돌려 `KAKULSAYDON_ARENA.spawngroupsbootstrap`을 갱신하고 Server를 재시작해야 한다.
- 옵션 2: 원작 체력을 유지하고 Q 피해를 원작 수준으로 키우거나 Q가 상자를 한 번에 부수게 서버 코드를 바꾼다. Server 코드 수정과 재빌드가 필요하고 "일반 몬스터 피해로 상자를 먼저 때린다"는 문서 계약과 테스트를 함께 바꿔야 한다.
- 옵션 3: 테스트만 현재 체력에 맞춘다. 게이트가 사실상 열리지 않는 상태를 테스트가 승인하게 되므로 권하지 않는다.

## 4. 항목 3: 월드 재생 계약 테스트 실패 2건 (`--world-playback-contract-test`)

origin/main 소스로 만든 비교용 서버에서 50 통과 / 실패 2다. 병합이 만든 문제가 아니다. 두 실패는 서로 독립이며 각각 낡은 테스트 기대가 현재 게시 데이터와 어긋난 것이다.

실패 A: "Viewer switching to Valtan clears the previous world's sequence IDs" (`ServerGameplayContractTests_WorldPlayback.cpp` 43~44행)
- 테스트는 발탄 월드를 불러온 뒤 시퀀스 인스턴스 ID 목록이 비어 있기를 기대한다. 이 기대는 2026-09-13(`3fc23750`)에 쓰였다.
- 현재 게시된 `VALTAN_ARENA.worldbootstrap` 헤더의 시퀀스 개수가 11이고 `world.sequence.instance.valtan.source-preview.*` ID 11개가 들어 있다. 발탄 컷신 저작(2026-09-20~22, `34d7117b` 등)에서 생긴 정상 데이터다.
- 검증: 발탄 시퀀스 ID 11개만 뺀 데이터 사본으로 같은 서버를 돌리면 이 실패가 사라지고(51/1) 다른 실패만 남는다(`logs/exp_c2_world_playback.txt`).

실패 B: "Legacy PLAY/REPLAY/motion reject; STOP and other sequences remain admitted" (117행)
- 5번째 시나리오가 `world.sequence.instance.circusfinale`를 "일반 시퀀스이므로 PLAY가 허용되어야 한다"고 기대한다.
- 게시된 `Gameplay.bootstrap`의 1관문 RAIDGATE 행은 입장 시퀀스가 정확히 `world.sequence.instance.circusfinale`다(13번째 열). `GameRoom_PartyWorld.cpp` 572~594행은 쿠크 아레나에서 이 ID의 PLAY/REPLAY를 일반 재생이 아니라 레이드 시작 준비(`Begin_KoukuRaidPreparation`)로 보낸다. 테스트 방은 세션→플레이어 연결이 없어서 `GameRoom_KoukuRaidFlow.cpp`의 첫 검사("Invalid raid START owner or scope")에서 거부되고 시나리오 4의 기대와 어긋난다.
- 검증: 1관문 입장 시퀀스만 `NONE`으로 바꾼 데이터 사본으로 같은 서버를 돌리면 이 실패가 사라지고(51/1) 다른 실패만 남는다(`logs/exp_c1_world_playback.txt`). 입장 시퀀스가 있는 관문은 1관문뿐이다(BINGO/GATE2/GATE3은 NONE).

수정안 (`patches/03_world_playback_test_expectations.patch`, 테스트 파일만, 코드·데이터 변경 없음): 테스트 의도는 그대로 두고 현재 데이터에 맞춘다.
- A: 쿠크 ID 목록을 먼저 저장해 두고, 발탄 로드 뒤에 쿠크 ID가 하나도 남지 않았는지 확인한다.
- B: 5번째 시나리오를 입장 시퀀스가 아닌 다른 게시 시퀀스(`world.sequence.instance.arena_rise_00`)로 바꾼다.
- 파일은 ASCII 전용 CRLF다. 패치는 CRLF를 유지하고 앵커가 정확히 한 번씩 맞을 때만 쓴다.

## 5. 게이트, 수정, 검증

사용자가 "지금 작업하고있는거 전부 완료처리부터 하자, 실패한건 원인 다시 찾고 해결까지 fork로"라고 지시했다. 이어서 "문제 먼저 해결하고 퍼블리셔만 돌려, 빌드는 내가 직접 VS로 한다"고 다시 지시했다. 이 절은 그 지시에 따라 코드/데이터를 고치고 해당 도메인 퍼블리셔만 실행한 기록이다. **빌드는 하지 않았다.**

게이트: 이전 세션(첫 A단계 fork)에서 원인 조사까지 끝난 상태를 이어받았다. 이번 실행은 사용자가 세 fork(A 이 문서/실패 수정, B 병합 마무리, C 마하라카)를 동시에 새로 띄운 것이라 별도 게이트 대기 없이 바로 시작했다.

### 항목 1: Compositions shape 검증기

- 적용: `Tools/CompositionPipeline/composition_pipeline.py`의 콜라이더 트랙 검증에 선택 필드 `shape`(BOX/CYLINDER, CYLINDER는 X/Z 반지름 오차 0.0001 이내·HOOK_CAPTURE 불가)를 허용. Client `WorldSequenceDocument.cpp`(851~859행)와 `Tools/MapPipeline/Publish-MapAuthoring.ps1`(2024~2049행)의 같은 규칙과 대조해 일치시켰다. 앵커 기반 python 패치로 적용(백업 `out/MergeFailuresFix20260925/backup/composition_pipeline.py.before_fix01`, `test_composition_pipeline.py.before_fix01`).
- 단위 테스트: 새 테스트 `test_box_and_cylinder_shapes_follow_the_map_owner_rules`(BOX/CYLINDER 허용, 반지름 불일치·HOOK_CAPTURE·소문자·미지원 값 거부 10 케이스) 0.175초 통과. 전체 Compositions 단위 테스트(`python -m unittest Tools.CompositionPipeline.test_composition_pipeline -v`) 51개, 실패 0, 에러 0, 551초. 로그: `out/MergeFailuresFix20260925/logs/unittest_shape_only_after.txt`, `unittest_composition_full_after.txt`.
- 게시: `Tools/CompositionPipeline/Publish-Compositions.ps1 -Mode Validate` 통과(Valtan patterns=42, KoukuSaydon actions=349, arena sequencers=2) → `-Mode Publish` 통과(sourceManifestId=690f11581d427e829db910dbeb83884889e9c7d1f8b54dbd8f0763f1d8bc84bb). 게시 전 `Client/Bin/DataFiles/Compositions` 전체 백업(`out/MergeFailuresFix20260925/backup/Compositions_before_publish/`). 실제로 바뀐 파일은 예상대로 정확히 3개: `Bosses/Valtan.bosscomposition.json`, `Sequences/KoukuSaydonArena.sequencer.json`, `Composition.publish.receipt.json`(22:23:25 갱신, JSON parse 확인). `Bosses/KoukuSaydonGate1.bosscomposition.json`, `Sequences/ValtanArena.sequencer.json`은 이번에도 바이트 동일. 로그: `out/MergeFailuresFix20260925/logs/compositions_validate_final.txt`, `compositions_publish_final.txt`.
- 빌드 뒤 확인 필요: 없음(파이썬 도구와 데이터 게시만이라 빌드와 무관하게 이미 유효).

### 항목 2: world-playback 테스트 2건

- 적용: `Server/Private/ServerGameplayContractTests_WorldPlayback.cpp`만 수정(CRLF·ASCII 보존, 앵커 유일 매치 확인, 백업 `out/MergeFailuresFix20260925/backup/ServerGameplayContractTests_WorldPlayback.cpp.before_fix03`). A: 발탄 로드 전 쿠크 시퀀스 ID 목록을 저장해 두고, 발탄 로드 뒤 "쿠크 ID가 하나도 남지 않았는지"로 검증(발탄 자체가 게시한 `world.sequence.instance.valtan.source-preview.*` 11개는 남아 있어도 통과). B: 5번째 시나리오의 재생 대상을 입장 시퀀스 `world.sequence.instance.circusfinale`(GATE1 RAIDGATE 13번째 열, 이제는 `Broadcast_WorldSequencePlay`가 레이드 준비 경로로 가로챈다)에서 게시된 다른 일반 시퀀스 `world.sequence.instance.arena_rise_00`(`KAKULSAYDON_ARENA.worldbootstrap`에 존재 확인)로 교체.
- 근거 재확인: `Server/Private/GameRoom_PartyWorld.cpp` 572~630행을 직접 읽어 `Broadcast_WorldSequencePlay`의 실제 admission 규칙(월드가 KAKULSAYDON_ARENA이고 PLAY/REPLAY이며 id가 GATE1 entrySequenceInstanceId와 같으면 레이드 준비 분기로 감; `original_kouku`는 PLAY/REPLAY 시 무조건 거부; 그 외는 일반 broadcast로 항상 true)를 확인했고, 손으로 5개 시나리오를 재계산해 이전 fork의 실험 결과(exp_c1/exp_c2, 51/1)와 일치함을 검증했다.
- 검증: `cl.exe`가 PATH/`vcvarsall` 없이는 쓸 수 없어 실제 컴파일은 하지 못했다. 대신 중괄호·괄호·대괄호 균형을 스크립트로 확인(모두 0, 균형 맞음). **빌드 뒤 확인 필요: 그렇다.** 사용자가 VS로 빌드한 뒤 `Server.exe --world-playback-contract-test`를 실행해 실패 0/51 이상을 기대한다.

### 항목 3: 카드미로 상자 체력

- 적용: `Data/Balance/Profiles/Retail.balanceprofile.json`에서 `MONSTER_KOUKU_CLOWN_BOX`(587,993 HP/1,843 공격) 행 삭제, `Tools/GameplayPipeline/build_retail_balance_profile.py`의 `EXTRA_MONSTER_NPC_KEY` 표에서 같은 항목 제거(주석으로 이유 남김). 두 파일 모두 실제 줄바꿈(Retail JSON은 CRLF, 생성기는 LF)을 확인해 그대로 유지하는 앵커 패치. JSON parse와 python 구문 확인 완료. 백업 `out/MergeFailuresFix20260925/backup/Retail.balanceprofile.json.before`, `build_retail_balance_profile.py.before`.
- 게시: `Tools/WorldPipeline/Publish-WorldGameplay.ps1 -Mode Validate -WorldId KAKULSAYDON_ARENA`(114 placements·7 spawn groups) → `-Mode Publish -WorldId KAKULSAYDON_ARENA`. 게시 전 대상 4개 파일 백업(`out/MergeFailuresFix20260925/backup/World_before_cardmaze/`). 실제로 바뀐 추적 파일은 `Server/Bin/DataFiles/World/KAKULSAYDON_ARENA.spawngroupsbootstrap` 하나뿐이고, diff는 상자 PROFILE 행 한 줄(587993/1843 → 500/1)뿐이다. `KAKULSAYDON_ARENA.worldbootstrap`, `KAKULSAYDON_ARENA.npcpresentation.json`, `KAKULSAYDON_ARENA.stagemarkers.json`은 바이트 동일. 게시가 함께 다시 쓴 `SequenceViewer.labels.json`, `LV_LUT_MIDNIGHTC_ED.viewer.world.json`도 추적 diff 없음(내용 동일). 로그: `out/MergeFailuresFix20260925/logs/worldgameplay_validate_after.txt`, `worldgameplay_publish_after.txt`.
- 재테스트: 이미 빌드된 `Server\Bin\Debug\Server.exe --card-maze-contract-test`를 실제 게시 데이터로 실행. **결과: 실패 0(이전 91/2 → 93/0), 3분 47초.** 로그: `out/MergeFailuresFix20260925/logs/card_maze_after_fix.txt`. 빌드 뒤 재확인 불필요(이미 빌드된 exe로 직접 검증 완료).
- 이 변경은 설계 결정이다(4절 참고). 팀장님께 전달할 설명은 8절.

### 항목 4: kouku-support-surface 2건

두 실패의 코드 위치를 직접 찾아 원인을 각각 확정했다(`Server/Private/ServerGameplayContractTests_KoukuSupportSurface.cpp`).

**"Showtime catalog admission requires the configured isolated Server data root"(1910행)** — 원인 확정, 코드 결함 아님. 이 assertion은 1528행 `if (dataRootLength && dataRootLength < dataRootBuffer.size())`의 else 분기다. `dataRootLength`는 `GetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT", ...)`의 반환값이며, 이 환경변수가 프로세스 시작 시 설정돼 있지 않으면 0을 반환해 무조건 이 else로 떨어진다. 즉 **테스트를 돌리는 실행 조건(환경변수 누락) 문제이지 Server 코드나 데이터의 결함이 아니다.** 코드나 데이터는 고치지 않았다.
검증: `$env:LOSTARK_SERVER_DATA_ROOT = (Resolve-Path 'Server\Bin\DataFiles').Path`를 설정한 뒤 같은(이미 빌드된) `Server.exe --kouku-support-surface-contract-test`를 재실행했다. 결과 파일에 PASS 226줄이 쌓이는 동안 "Showtime catalog admission" 텍스트는 FAILURE로도 PASS로도 전혀 나타나지 않았다(그 문자열은 else 분기에만 있으므로 안 나타난 것은 if 분기를 탔다는 뜻). 다만 실행이 14분 넘게 멈춰(정확한 원인 미확인 — 이후 코드 경로가 매우 느리거나, PowerShell `Tee-Object`의 콘솔 파이프 오버헤드로 대용량 출력이 지연됐을 가능성) 15분 한도로 프로세스를 강제 종료해(pid 13468) 최종 "failures: N" 요약 줄은 얻지 못했다. **사실: else 분기 미도달(강한 정황). 미확인: 최종 통과 여부 수치.** 로그: `out/MergeFailuresFix20260925/logs/support_surface_with_env_after.txt`(20,624바이트, PASS 226, FAILURE 1건은 아래 항목).
빌드 뒤 확인 필요: 아니오(코드 변경 없음). 다만 **실행 조건**이 필요하다: 사용자가 이 테스트를 다시 돌릴 때는 `LOSTARK_SERVER_DATA_ROOT` 환경변수를 먼저 설정해야 한다(6절 명령 목록 참고). 이 환경변수를 테스트 스스로 설정하지 않는 것이 맞는 설계인지, 아니면 CI/러너가 항상 설정해 줘야 하는지는 이 저장소 문서에서 확인하지 못했다 — 사용자 결정 필요(8절).

**"Grouped pursuit ticks consume the same bounded angular time"(1087행)** — **원인 미확정.** `Update_KoukuPlayerTargets`(`Server/Private/GameRoom_BossSimulation.cpp` 959~1057행)의 이동 추적(pursuit) yaw 계산을 직접 손으로 역추적했다: `beginFacing`이 매번 archetype을 `BOSS_KAKULSAYDON_G1_KOUKU`(오프셋 0)로 리셋하므로, tick 5000에서 turn=90°, clamp(±6°)=6°가 되어 기대값 6°와 일치. 이어서 target을 +100 이동 후 tick 5014로 건너뛰면 `Elapsed_ServerTicksSkippingReservedZero(5000,5000)=0`, `(5000,5014)=14`로 currentTicks-previousTicks=14, clamp(84°, ±84°)=84°, 6+84=90°가 되어 기대값 90°와도 정확히 일치한다. 즉 **손 계산으로는 코드가 정확히 기대값을 내야 하는데도 실제로는 실패한다.** 부동소수점 오차, 이전 assertion들이 남긴 상태(archetype/이월 상태)에 대한 내 트레이스의 실수, 또는 이 함수의 다른 분기(예: `RotateOnlyFraction`)가 실제로는 선택된다는 가능성 중 어느 것이 맞는지는 디버거나 print 계측 없이는 확정할 수 없다. **빌드가 금지돼 있어 코드에 임시 로그를 넣고 재컴파일해 확인하는 것도 이번 범위에서는 할 수 없었다.**
Server 코드는 수정하지 않았다(원인이 애매한 상태에서 공유 보스 추적 로직을 손대는 위험을 피했다). origin/main 서버에서도 같은 assertion이 실패함(이전 조사에서 확인)은 그대로 유효하며, 병합이 만든 문제가 아니다.
사용자 결정 필요: 이 실패는 미해결로 남았다. 다음 단계는 디버거(VS)로 이 테스트만 중단점을 걸어 `turn`, `currentTicks`, `previousTicks`, `boss.fYawDegrees`의 실제 값을 확인하는 것을 권한다(8절).

## 6. 나머지 테스트 분류

이번에 실제로 돌린 것(모두 이미 빌드된 `Server\Bin\Debug\Server.exe`, 19:58 빌드 그대로 사용, 재빌드 없음):

- `Tools.CompositionPipeline.test_composition_pipeline`(python 단위 테스트) — 51개, 실패 0.
- `--card-maze-contract-test` — 93개, 실패 0(수정 후).
- `--kouku-support-surface-contract-test`(환경변수 설정 후) — 226 PASS까지 확인, 실패 1건("Grouped pursuit ticks", 미해결), 강제 종료로 최종 수치 미획득.

시간·범위 때문에 이번에 다시 돌리지 않은 것(원인 조사는 앞선 A단계에서 이미 끝났고, 병합이 만든 것이 아니라 origin/main에도 있는 실패로 확인됨):

- `--kouku-object-overlap-contract-test`, 기본 `--contract-test`, `--debug-teleport-contract-test`, `--kouku-raid-contract-test`, `--kouku-product-contract-test`, `--valtan-lifecycle-contract-test` — 이전(B fork) 실행에서 전부 실패 0으로 끝까지 통과한 기록이 있다(`out/MergeBackup20260925/server_tests/`). 이번 A단계의 수정(Compositions shape, world-playback 테스트 기대치, 카드미로 상자)은 이 목록의 테스트들과 겹치지 않으므로 재실행하지 않았다.

사용자가 VS로 빌드한 뒤 다시 돌릴 명령과 기대 결과:

```
Server\Bin\Debug\Server.exe --world-playback-contract-test
```
기대: 실패 0(이전 2건이 이번 수정으로 없어져야 한다). 안 되면 3절의 admission 규칙 재확인 필요.

```
$env:LOSTARK_SERVER_DATA_ROOT = (Resolve-Path 'Server\Bin\DataFiles').Path
Server\Bin\Debug\Server.exe --kouku-support-surface-contract-test
```
기대: "Showtime catalog admission" 실패는 사라지고, "Grouped pursuit ticks" 실패 1건만 남는다(이번에 해결하지 못함). 이번에는 실행이 비정상적으로 느려 15분에 강제 종료했으므로, 시간을 넉넉히 주거나 콘솔 출력을 파일로 직접 리다이렉트(`> out.txt 2>&1`, PowerShell `Tee-Object` 대신)해서 돌릴 것을 권한다.

```
Server\Bin\Debug\Server.exe --card-maze-contract-test
Server\Bin\Debug\Server.exe --kouku-object-overlap-contract-test
Server\Bin\Debug\Server.exe --world-playback-contract-test
```
회귀 확인용(이미 이번 세션에서 통과 확인했거나 영향받지 않는 것들이 그대로인지).

## 7. 팀장님께 전달할 것

- **patches/01_composition_collider_shape.patch**: Compositions 검증기가 `colliderTracks`의 `shape` 필드(팀장님 커밋 `88fa743d`가 데이터에 추가)를 못 받던 문제. 이미 이 브랜치에는 적용·게시까지 완료했다. 팀장님 main에는 아직 없으므로 같은 패치를 드려야 한다.
- **patches/02_clownbox_retail_exclusion_PROPOSAL.patch**(제안, 이미 이 브랜치에는 적용함): `MONSTER_KOUKU_CLOWN_BOX`를 Retail 프로필에서 제외해 저작 체력 500(Q 뿅망치 500 피해로 한 방에 파괴)을 유지한다. 원래 팀장님이 `.md/GB/09-24/2026-09-24_KOUKU_GATE1_HEALTH_FLOW_RESULT.md` G08에서 상자 체력을 587,993으로 올린 의도(원작 NPC 480720 값 반영)와, 2026-09-14 카드미로 게이트 설계(500 HP를 Q 한 방에 부숨, 커밋 `8a351ac8`)가 서로 부딪힌다. 전달 문장: "카드미로 망원경 게이트의 삐에로 상자가 Retail 프로필의 원작 체력(587,993)을 받으면서 500 피해 고정인 Q 뿅망치로는 사실상 못 부수게 됐습니다(1,176번 필요). 카드미로 계약 테스트 2건도 이 때문에 실패합니다. 상자를 Retail 원작 반영 대상에서 빼서 저작 500 HP를 유지하는 패치를 임시로 적용했습니다. 원작 체력을 유지하고 싶으시면 대신 Q 피해를 올리거나 서버 로직을 바꿔야 합니다." 되돌리는 방법: `Data/Balance/Profiles/Retail.balanceprofile.json`에 상자 행(archetypeId MONSTER_KOUKU_CLOWN_BOX, sourceNpcId 480720, maxHp 587993, attackPower 1843)을 되살리고 `Tools/GameplayPipeline/build_retail_balance_profile.py`의 `EXTRA_MONSTER_NPC_KEY`에 다시 추가한 뒤 `Publish-WorldGameplay.ps1 -Mode Publish -WorldId KAKULSAYDON_ARENA`.
- **patches/03_world_playback_test_expectations.patch**(이미 적용함): 낡은 테스트 기대치를 현재 게시 데이터(발탄 컷신 11개 저작, 쿠크 1관문 입장 시퀀스가 레이드 시작 경로로 편입됨)에 맞춤. 코드·데이터 변경이 아니라 테스트 파일만 바꿨으므로 팀장님 main에 그대로 적용해도 안전하다.
- **미해결 보고**: "Grouped pursuit ticks consume the same bounded angular time" 계약 테스트가 origin/main에서도 실패합니다(병합과 무관). `Server/Private/GameRoom_BossSimulation.cpp`의 `Update_KoukuPlayerTargets`, 1087행 근처입니다. 손 계산으로는 정확한 값이 나와야 하는데 실제 실행은 다릅니다. 디버거로 직접 확인이 필요합니다.

## 8. 사용자 결정이 필요한 것

1. 카드미로 상자를 Retail에서 뺀 패치(항목 3)를 이대로 유지할지, 팀장님 확인 전까지 되돌릴지.
2. "Grouped pursuit ticks" 실패는 미해결이다. VS 빌드 뒤 디버거로 직접 값을 확인할지, 아니면 이번 병합 작업에서는 보류하고 넘어갈지.
3. `LOSTARK_SERVER_DATA_ROOT` 환경변수가 필요한 것이 의도된 테스트 전제조건인지 팀장님께 확인이 필요한지.

## 9. 최종 재확인

확인한 것:
- 내가 바꾼 추적 파일 9개(4절 각 항목에 정확히 나열)를 `git status --short`로 시작 전/후 대조해 그 외 아무것도 건드리지 않았음을 확인했다. 같은 저장소에서 동시에 도는 다른 두 fork(B 병합 마무리, C 마하라카)의 변경(`Client/Private/Level_Development.cpp`, `Data/Maps/MapCatalog.json`, 마하라카 관련 파일들, `.md/GB/09-25/2026-09-25_MERGE_RESULT.md` 등)은 내가 만든 것이 아니며 손대지 않았다.
- 렌더링 보호 파일 무변경: `git diff cd58d12b -- Engine/Private/Renderer.cpp Engine/Public/Renderer.h Client/Bin/ShaderFiles Data/Rendering Client/Bin/DataFiles/Rendering`가 비어 있다(0줄).
- Compositions 단위 테스트 51개 실패 0, 카드미로 서버 테스트 93개 실패 0(둘 다 실제 실행 결과, 통과라고 적기 전에 로그 파일로 재확인했다).
- 빌드는 하지 않았다(사용자 지시). C++ 두 파일(world-playback 테스트, 이미 빌드된 카드미로/support-surface는 서버 코드 변경 없음)은 문법 균형 검사만 했고 실제 컴파일 확인은 못 했다.

못 한 것:
- world-playback 수정의 실제 컴파일·테스트 통과 여부(빌드 필요).
- kouku-support-surface의 최종 통과 수치(강제 종료로 못 얻음).
- "Grouped pursuit ticks" 원인 확정과 수정(미해결로 남김).
- 병합이 만든 문제가 아니라고 이전에 확인한 나머지 테스트군(`--contract-test` 등)의 재실행(이번엔 범위 밖으로 판단해 생략).

## 10. 빌드 뒤 확인 (조정자 실행, 2026-09-25 22:29~22:33)

사용자 지시로 조정자가 정본 러너 `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug`(정상 증분 Build)를 실행했다.

- 결과: PASS, 총 15초. Engine 0.4초·Shared 0.3초(변경 없음), Server 2.3초, Client 8.2초. 결과 JSON `out/BuildPipeline/runs/20260925T133004381Z-debug-product.json`(missing/invalidRuntimeInputs 비어 있음, runtimeDataChecks 38건 PASS). 로그 `out/FinalBuild20260925/product_debug_build.log`.
- 다시 컴파일된 OBJ는 `ServerGameplayContractTests_WorldPlayback.obj`와 `Level_Development.obj` 둘뿐이다. 셰이더 쓰기 0. `Server.exe` 22:29:52, `Client.exe` 22:30:00 갱신, `Engine.dll` 19:56 유지.
- 컴파일 오류 0. 경고 19개는 전부 기존 C4819(코드 페이지) 경고이며 이번 수정 두 파일에서 나온 경고는 없다.
- `--world-playback-contract-test`(새 Server.exe, 22:30:44~22:33:09, rc=0): **52 PASS / 실패 0**. 이전에 실패하던 두 항목("Viewer switching to Valtan clears the previous world's sequence IDs", "Legacy PLAY/REPLAY/motion reject; STOP and other sequences remain admitted")이 PASS로 바뀌었다. 결과 `out/FinalBuild20260925/world_playback_after_build.txt`. 이로써 3절의 "빌드 확인 대기"는 해소됐다.
- 여전히 미해결: `--kouku-support-surface-contract-test`의 "Grouped pursuit ticks consume the same bounded angular time" 1건(원인 미확정, origin/main에도 있음). 빌드 후에도 다시 돌리지 않았다.

FIX_FORK_DONE (partial): Compositions shape 검증기·게시, 카드미로 상자 데이터·게시, world-playback 테스트 수정(빌드 확인 대기)은 끝났다. kouku-support-surface의 "Grouped pursuit ticks" 원인은 못 찾았고 수정하지 않았다. 위 8절 사용자 결정 3가지가 남아 있다.
