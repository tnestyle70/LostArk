# 2026-09-30 베른 배 NPC 누락 원인 확정과 전수 리소스 수집 RESULT

조사와 리소스 복사만 했다. 코드·Data 소스 수정, 커밋, 푸시, 빌드, 게임 실행은 하지 않았다.
표기: **사실**(파일·git·채팅 기록에서 직접 읽음), **추론**, **불확실**을 구분한다.

## 1. 배를 타기 위한 NPC의 정체 (사실)

베른성 Area `LV_BER_BERNCASTLE`의 `Data/Worlds/LV_BER_BERNCASTLE/Gameplay.world.json`(revision 928) 세 개다.

- `npc.bern.ship.shipwright.1` = `NPC_SHIP_SHIPWRIGHT` (271.59, 12.43, -196.70)
- `npc.bern.ship.shipwright.2` = `NPC_SHIP_SHIPWRIGHT` (229.61, 12.47, -196.50)
- `npc.bern.ship.harbormaster.1` = `NPC_SHIP_HARBORMASTER` (264.41, 11.79, -204.03)

`Data/Actors/NpcCatalog.json`이 각 archetype에 모델을 연결한다.

- 조선공: `Character/NPC/Npc_MN_RHKP_02_2/Npc_MN_RHKP_02_2.wmodel`, idle `idle_normal_1_1`
- 항만 관리인: `Character/NPC/Npc_NP_LRKK_01/Npc_NP_LRKK_01.wmodel`, idle `idle_normal_1`
- 두 archetype 모두 `modelMaterialOverrides`가 없다. 필요한 리소스는 모델 하나와 그 텍스처 4개씩이다.
- 최소 폐쇄(NPC가 보이기 위한 리소스): 위 두 폴더 각 5개, 합계 10개 4.6MB. 이 PC에 전부 있다(09-25 03:45~03:47 생성).

배 NPC를 눌러 여는 배 선택 창과 탑승에는 같은 작업의 배 모델 9종(`Character/Vehicle/Ship_*`)과 아이콘 9개(`UI/Vehicle/Icons/ship_82xx.png`)도 필요하다.

## 2. 작업 시점 (사실)

- 배 모델·NPC 모델·아이콘 생성: 09-25 03:26~03:47 (파일 mtime). 배포 목록 `Resource_Distribution_2026-09-25_Bern3Ship.txt` 작성: 09-25 03:53.
- 사용자가 "Bern3에 NPC 어떤 거 깔아야 하지"라고 물은 시각: 09-25 14:05(KST, 채팅 기록 UTC 05:05Z). NPC 3개 배치와 게시는 그날 14:16~14:43, 커밋 `20c8be4a`(09-27 11:26).
- 09-25 결과 문서(`.md/GB/09-25/2026-09-25_BERN3_SHIP_NPC_VISIBILITY_RESULT.md`): 두 NPC 모델은 크기 계약이 어긋나 약 1.3cm로 그려져 보이지 않았고, 수정은 클라이언트 코드(`NpcPresentationAssetService.cpp`의 `Resolve_NpcModelPreScale`)로 했다.

## 3. 원인 확정

### 3-1. (가) 데이터·코드는 원인이 아니다 (사실)

- NPC 배치, `NpcCatalog.json`의 배 NPC 두 항목, 크기 수정 코드(`Resolve_NpcModelPreScale`)는 모두 `origin/main`에 있다. `origin/main`의 `Gameplay.world.json`에 `NPC_SHIP` 3건, `NpcCatalog.json`에 2건, 크기 수정 코드 3건을 확인했다.
- 들어간 시점: 커밋 `20c8be4a`(09-27)가 PR #467(09-27 18:14 병합)로 main에 들어갔다. 이 브랜치는 현재 `origin/main`의 조상이라(고유 커밋 0개) PR #487도 병합된 상태다.
- 게시본 `Server/Bin/DataFiles/World/BERN.worldbootstrap`에 배 NPC 3행(40~42행)이 있고 main에 포함돼 있다. 작업 트리의 미커밋 변경은 이번 배 NPC와 무관한 콜로세움·초상·대기열 작업이다.
- 결론: 팀원이 main과 같은 코드를 받았다면 서버는 배 NPC를 스폰하고 클라이언트도 배율 수정이 적용돼 있다. 안 보이는 원인은 데이터가 아니다.

### 3-2. (나) 리소스 누락이 원인이다 (사실 + 추론)

- 사실: 배 NPC 모델 두 개는 클라이언트가 그릴 때 `Client/Bin/Resources`에서 읽는다. 리소스는 Git 비추적이라 팀원은 Drive의 `CY_Resources`로만 받는다. 없으면 NPC 생성이 실패하고 `npc.presentation.unavailable` 로그만 남는다(09-25 결과 문서 6절).
- 사실(채팅 기록): 사용자의 실제 전달 방식은 `CY_Resources` 폴더를 Drive/홈페이지로 직접 올리는 것이다(09-20 "CY_Resources… drive 배포용", 09-29 "이거 drive로 보내는 건 내가 알아서 홈페이지로 보내는 건데 왜 저런 걸 만드는 거냐"). 저장소 루트의 `Resource_Distribution_*.txt`/`Copy_ResourceDistribution_*.ps1`은 내가 만든 보조 목록이고 전달 방식이 아니다.
- 사실(채팅 기록): 09-25 17:48에 "Drive로 보낼 것: 배 탑승 46개 57.7MB(배 모델 9종, 배 NPC 2종, 아이콘 9개)"라고 보고하고 "병합이 끝난 뒤 보내라"고 했다. 그 뒤 `CY_Resources`가 처음 만들어진 것은 09-27 05:20(채팅 UTC 09-26 20:20Z)이다. 그때 내가 보고한 내용은 245개 89.1MB, 구성이 `Character/NPC/Maharaka` 30, `Map/LV_OCN_EVENTIS_MHP*` 203, `Sound/Maharaka` 12였다. 배·배 NPC는 없다. 이 245개는 "09-26 이후 수정된 파일"만 모은 시각 델타였고(09-27 02:18 보고에서 내가 그 목록이 틀렸다고 정정했다), 배 리소스는 09-25 파일이라 시각 기준에서 빠졌다.
- 사실: 이어서 `CY_Resources`는 "마하라카 참조 폐쇄"(1,869개 937.8MB, 09-27), 09-28 물총 20개, 09-30 마하라카 보충(479개)으로만 채워졌다. 이 세 번의 스테이징 어디에도 `Character/Vehicle/Ship_*`, `Npc_MN_RHKP_02_2`, `Npc_NP_LRKK_01`, `UI/Vehicle/Icons`가 없다(매니페스트와 채팅 보고서 대조).
- 사실: 09-30에 한 마하라카 누락 점검(`2026-09-30_CY_RESOURCES_MAHARAKA_RECONCILE_RESULT.md`)은 "예전 배포 목록에 있고 마지막 목록보다 오래됐으면 팀원이 갖고 있다"고 가정했다. 그런데 이 목록은 내가 만든 파일 목록이지 업로드 기록이 아니다. 배 46개는 그 가정 때문에 복사 대상에서 빠졌다.
- 추론(가장 유력): 배 NPC 두 모델은 어떤 `CY_Resources`에도 한 번도 담기지 않아 Drive로 나간 적이 없다. 그래서 팀원 PC에는 모델이 없고 NPC가 안 만들어진다. 같은 이유로 배 모델, 배 아이콘, 배 물살 이펙트도 없다.
- 불확실: 사용자가 09-25~09-26에 목록 txt/ps1로 직접 전달했을 가능성은 채팅에서 확인되지 않는다. 팀원 PC를 직접 본 것도 아니다. 팀원이 안 보인다고 한 NPC가 위 세 개인지 확인이 필요하다면 팀원의 `Client/Default/EffectFailure.user.log`에서 `npc.presentation.unavailable`(모델 준비 실패)과 `npc.ship.spawned`가 없는지 보면 된다.

### 3-3. 어디서부터 누락됐나

09-25 03:26~03:47에 배 리소스가 생성됐고, 09-25 03:53에 목록만 만들어졌다. 09-27 05:20에 `CY_Resources`가 시각 기준(09-26 이후)으로 처음 만들어지면서 이 파일들이 첫 번째로 빠졌다. 이후 09-27~09-30의 세 번의 보충은 마하라카 범위만 다뤄서 되돌리지 못했다. 시작점은 "09-27 `CY_Resources` 최초 생성 시각 델타"이다.

## 4. 채팅·파일시스템 기반 전수 목록

수집 방법: (1) 이 세션 기록(09-18~09-30, 150MB)에서 도구 호출·결과에 나온 Resources 상대 경로 633종을 뽑아 현재 파일 존재·mtime과 교차, (2) 09-25 Bern3Ship·ShipWake·MaharakaMaterials·MaharakaRestore 배포 목록, (3) 09-27 마하라카 Drive 매니페스트(`out/MaharakaDriveHandoff/resources-drive-manifest.json`, 1,869개), (4) 09-27 이후 우리 영역(마하라카 NPC·물총·이펙트·사운드·HUD·섬·닻)의 mtime 규칙. 이 네 출처의 합집합이다.

- 파일 mtime만으로 09-25 이후 수정 파일을 세면 13,897개 8.1GB가 나온다. 다른 팀원 Drive 리소스와 복사본이 섞여서 정확하지 않아 쓰지 않았다(09-25 17:48 보고에서도 같은 판단).
- 합집합: **3,910개, 1,297.5MB**. 실제 파일이 이 PC에 있는 것만 담았다.

분류별(경로 접두사):

- 배 NPC 모델 2종(조선공·항만 관리인): 10개, 4.6MB. **스테이징 기록 없음(누락 유력)**
- 배 모델 9종 `Character/Vehicle/Ship_*`: 27개, 53.1MB. **스테이징 기록 없음(누락 유력)**
- 배 선택 창 아이콘 `UI/Vehicle/Icons/ship_8200~8208.png`: 9개. **스테이징 기록 없음(누락 유력)**
- 배 물살 이펙트 `Effect/Vehicle/Ship/*`: 3개, 0.3MB. **스테이징 기록 없음(누락 유력)** (09-29 ShipWake 목록만 있음)
- 미니맵·로딩·상호작용 아이콘 16개, 4.9MB: `UI/Minimap/Maps/BernSea.png`, `UI/Minimap/marker_island.png`, `UI/Loading/Loading_Background_Maharaka.png`·`Sea_0.png`, `UI/Interact/Icon_move.png`·`Icon_lever.png`, `UI/Minimap/Maps/BernCastleIndoor_*.png` 10개. 스테이징 기록 없음. 앞의 네 종은 우리 작업물(사실, PR #487 본문), `UI/Interact`와 `BernCastleIndoor`는 **불확실**(우리가 만든 것인지 팀원 리소스 복사인지 확정 못 함).
- 배 HUD·물총 HUD `UI/HUD`: 62개, 0.5MB. 09-30에 CY로 스테이징된 기록 있음(업로드 여부 불확실).
- 닻 마커 이펙트 `Effect/Bern/AnchorMarker`: 3개. 09-29~09-30 CY 기록 있음.
- 바다 위 마하라카 섬 `Map/LV_BER_BERNCASTLE`: 10개, 9.8MB. 09-30 CY 기록 있음.
- 마하라카 NPC·물총 모델·애니메이션: 170개, 216.8MB. 09-27(30개)과 09-30(152개+…)에 CY 기록 있음.
- 마하라카 이펙트·사운드: 35개, 13.0MB. 09-28~09-30 CY 기록 있음.
- 마하라카 Area 맵: 3,565개, 994.5MB. 그중 1,839개는 09-27 폐쇄 매니페스트나 09-30 보충에 기록이 있고, **1,726개는 09-25 MaharakaRestore 목록에만 있고 09-27 폐쇄에는 없다**(참조 폐쇄로 쓰이지 않은 예전 변종·중간 텍스처 가능성, 09-30 점검이 "실패 후보 0"이라고 했으므로 우선순위 낮음).

우선순위: 위 "누락 유력" 65개(배 NPC 10 + 배 모델 27 + 아이콘 9 + 물살 3 + 미니맵·로딩·아이콘 16)가 반드시 확인할 대상이다.

## 5. 새 Desktop 폴더

- 위치: `C:\Users\USER\OneDrive\바탕 화면\CY_Resources_Bern_NPC_전수`
- 내용: 위 합집합 3,910개를 `Client/Bin/Resources` 상대 경로 그대로 복사했다(래퍼 폴더·README·목록 파일 없음).
- 검증: 3,910개 전부 크기 일치, 무작위 150개 + 8MB 초과 파일 + 배 46개 전부, 합계 196개의 sha256이 원본과 일치(불일치 0). 기존 `CY_Resources`는 읽기만 했고 수정하지 않았다. 이전에 있던 `CY_Resources2`, `CY_Resources_보류_이미배포추정` 폴더는 현재 Desktop에 없다.
- 이번 복사에 쓴 임시 스크립트는 `C:\Users\USER\.claude\jobs\46aea322\tmp`에만 있다.

## 6. 불확실 항목

- 사용자가 09-25~09-26에 배 46개를 목록 txt/ps1로 직접 전달했는지(채팅에 없음).
- 09-30에 `CY_Resources`에 스테이징한 마하라카 보충 479개를 실제로 Drive에 올렸는지(그 뒤 `CY_Resources`가 콜로세움용으로 다시 만들어져 현재 없음).
- `UI/Interact/Icon_move.png`·`Icon_lever.png`, `BernCastleIndoor_*.png`가 우리 산출물인지.
- 09-25 MaharakaRestore 목록에만 있는 맵 파일 1,726개가 현재 카탈로그에서 참조되는지(참조되지 않으면 보낼 필요 없음).
- 쿠크 BGM 12개(09-24 목록)는 팀장 파일로 대체될 가능성이 커서 제외했다. `Sound/KoukuSaton`·`Effect/Vehicle/AncientSea` 등 다른 팀원 작업분도 제외했다.

## 7. 사용자가 할 일

1. 우선 배 NPC 폐쇄 10개(`Character/NPC/Npc_MN_RHKP_02_2`, `Character/NPC/Npc_NP_LRKK_01`)를 Drive로 올린다. 배 탑승·선택 창까지 쓰려면 `Character/Vehicle/Ship_*`(27개), `UI/Vehicle/Icons`(9개), `Effect/Vehicle/Ship`(3개), 그리고 `UI/Minimap`·`UI/Loading`의 배·바다 관련 파일까지 올린다.
2. 팀원은 받은 폴더를 자기 `Client/Bin/Resources`에 같은 상대 경로로 덮어쓴다. 코드는 main을 pull(크기 수정 `Resolve_NpcModelPreScale` 포함)해서 다시 빌드해야 한다.
3. 폴더 전체(1,297.5MB)를 그대로 올릴 필요는 없다. 마하라카 Area 맵 3,565개(994.5MB)는 이미 09-27 폐쇄로 전달됐을 가능성이 크고 중복이므로, 위 "누락 유력" 65개를 먼저 올려 보고 팀원 로그로 확인하는 것을 권한다.
4. 코드·Data는 이미 main에 있어 push할 것이 없다.
