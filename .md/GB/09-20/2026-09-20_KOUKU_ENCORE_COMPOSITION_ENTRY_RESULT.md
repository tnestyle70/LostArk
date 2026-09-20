# 2026-09-20 쿠크 앵콜컷신 Composition 항목 추가 RESULT

대상: F1 → Action Workbench → Composition Actions 의 **Boss 탭**(빙고 게이트)과 **Sequence 탭**(빙고 게이트)에
`앵콜컷신` 항목을 하나씩 추가했다. 원본 자료 조사는 `2026-09-20_KOUKU_ENCORE_CUTSCENE_ORIGINAL_DATA_RESULT.md`,
게이트 구조는 `2026-09-20_KOUKU_GATE_ARCHITECTURE_MAP_RESULT.md`를 따른다.

## 0. 먼저: 가정, 확정하지 못한 것, 못 한 것

확정하지 못한 것.

- **화면 확인은 하지 않았다.** Client/Server/UI를 실행하지 않았고 항목이 Workbench 트리에 실제로 보이는지, Preview에서 카메라와 암전이 원본처럼 보이는지는 사용자만 판정한다. 트리 표시는 `Render_PatternTree` 소스를 읽어 만든 시뮬레이션으로만 확인했다.
- 영상(`앵콜 컷신 .mp4`)은 이 컷신의 **뒷부분**에 해당한다(영상 시간 + 약 5.4초로 추정). 이 추정은 확정이 아니다. 그래서 원본 전체 창 0..23.333초를 그대로 썼다. 영상 구간만 쓰려면 `CONFIG['start']`를 바꿔야 한다.
- 카메라는 원본 cam 그룹의 이동 트랙이 위치 키 1개·회전 키 1개라 **정지 카메라**다. 숫자는 원본과 일치하지만, 영상에서 카메라가 움직이는 것처럼 보이면 그 움직임은 카메라 그룹이 아니라 다른 요소(배우 이동·후처리)에서 온다는 뜻이고, 이 항목은 그것을 표현하지 못한다.
- 새 Pattern은 `DRAFT`다. 제품 재생(Complete Play, Server Play)에 연결되지 않았다.
- Client `Reload Patterns`, `Publish` 이후의 동작(새 V2 페이드 이펙트를 Client가 인식하는지)은 실행하지 않아 미확인이다.

못 한 것(원본에는 있지만 이 항목에 **넣지 못한 요소**). 표현 수단이 없거나 baker가 재현하지 못하고, 가짜로 만들지 않았다.

| 원본 요소 | 원본 값 | 못 넣은 이유 |
|---|---|---|
| 자막 3줄 (`cin.37081_12_01/02/03`) | 「누구 맘대로 끝을 내?!」 10.233초부터 1.70초, 「무효야, 전부 무효!」 12.367초부터 3.60초, 「진짜 시작은 지금부터라고!」 16.700초부터 3.05초 | Composition에 자막 resource 종류와 그것을 그리는 UI가 없다 |
| "던전 클리어" 가짜 엠블럼 UI | `PlaySWF vs.epicgatecommanderresulttest`, 15.45초에 unload, `fakeui` 이벤트 0초 | Flash UI를 재생하는 resource 종류가 없다 |
| 깨진 유리 후처리 | 재질 `fx_mi.fx_d_brokenglass_01_tr`, opacity 12.467초=0 → 12.5초=1 → 15.4초=1 → 15.433초=0, type 12.5초=0 → 13.533초=1, 카메라 흔들림 이벤트 `s1` 12.5/13.533/15.433초 | post-render 재질 resource 종류가 없다 |
| 보스 액터 쿠크세이튼_03 (group 51, actor export 24) | 이동 키 14개, anim 슬롯 a 8키, 슬롯 b 8키, fc1 0키, skelcontrolstrength 트랙 5개(60/3/37/3/4점) | **1차에는 넣지 않았다. 2차(10절)에서 이동 키 14개와 anim 슬롯 a/b를 넣었다.** 그래도 skelcontrolstrength 5개, fc1, `hit_color` 재질 트랙은 baker가 굽지 못해 여전히 없다(10절) |
| 쿠크세이튼_02 (group 48) | 연결 액터 없음 | 굽을 대상 없음 |
| 파티클/이펙트 그룹 | spark0, spark, disappear2, pung, move, break … break12 | 원본 emitter를 이 항목의 occurrence로 옮기는 경로 없음 |
| 조명 쿠크라이트 (group 47) | 조명 그룹 | 컷신 patterns에 조명 occurrence를 굽는 경로 없음 |
| 사운드 AkEvent | `bgm_midnightc_ed_m18_scene_fakeclear` 0초, `..._skip` 23.322초, `scene_midnightc_ed_popup1` 0초, `..._koukustopclearingdungeon` 2.1초 (sound group 54) | 사운드 resource 매핑을 만들지 않았다 |
| autoblend 값 트랙 | 8.267초 4000 → 1200(DOF로 추정) | 이 항목에 대응하는 resource 없음 |
| 서버 쪽 | 전이, 클리어 비트 보류, 입력 잠금, 전원 이동, 빙고 제품 로직 | 범위 밖(Server gameplay) |

1차에서 넣은 것은 **카메라(원본 sampling)와 원본 암전 fade**, 두 가지뿐이었다. 2차에서 보스 액터(세이튼)의 이동·애니메이션을 더했다(10절). 지금도 "앵콜 컷신 전체를 복원했다"가 아니다.

## 1. 기존 방식(`빙고_최종엔딩씬`) 분석과 재현

- Sequence 문서 `KAKULSAYDON_G1_PATTERN_9`(KCY 2119e772, 09-15, SCENE01B matinee 32 / data 45, 49083ms)와 Boss 문서 `PATTERN_75`(tnestyle70 34ec277e, 09-19, Workbench 복사본)는 ID와 Boss 문서의 pattern-level `durationMs`만 다르다.
- 구성은 BINGO 게이트 Pattern 1개(`actorProfileId MN_RPCT_05`, `targetBossPlacementId boss.kakulsaydon.bingo.saydon`, STAGE_1 ACTIVE 1개)이고 `presentationOccurrences`가 CAMERA resource(assetId = `camerashots.json`의 shotId)와 EFFECT/LEAF resource(assetId = V2 이펙트 파일)를 참조한다.
- 생성 파이프라인은 `Tools/KoukuSaydonPipeline/build_gate_cutscenes_g12.py`(CONFIGS 한 줄당 컷신 하나)이고 내부적으로 `build_gate2_intro_composition.make_cameras`와 `build_source_sequences.fade/resource/occurrence/merge`를 쓴다.
- **g12를 다시 돌리지 않았다.** CONFIGS 전체를 다시 만들고 id=8이 지금 문서의 `1관문_연출`(PATTERN_8)과 충돌한다. 대신 같은 함수를 쓰는 `Tools/KoukuSaydonPipeline/build_encore_cutscene.py`를 새로 만들었고, 각 문서의 기존 `빙고_최종엔딩씬` Pattern을 복제해 Workbench가 저장하는 필드 형태를 그대로 유지한다.

## 2. 원본 앵콜 변환 입력

- 패키지 `B9AVB2VAZIQRPQCJVKAVYRAVOKYPY8L6.upk`(SCENE07A).
- `efseqact_matinee_23` = export **21**, `interpdata_23` = export **44**, 길이 23.3333초, 그룹 23개. `KoukuSaydon.cinematicreference.json`(`matineeExport 21`, `interpDataExport 44`)과 일치한다.
- 09-10 PLAN은 이름(matinee_23 / interpdata_23)만 적었고 export 번호는 적지 않았다. 번호 충돌은 없다. PLAN이 이 항목을 만들었다고 적은 부분은 없고, 이번이 처음 생성이다.
- 설정: `name=앵콜컷신`, `prefix=kouku.bingo.encore`, `scene=SCENE07A`, `matinee=21`, `data=44`, `duration=23333`, `start=0`, `gate=BINGO`.

## 3. 만든 것

파일별 추가(기존 행은 수정하지 않았고 카운터만 올렸다).

| 파일 | 변경 | 결과 |
|---|---|---|
| `Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json` | Pattern `KAKULSAYDON_G1_PATTERN_10` "앵콜컷신"(BINGO, stage 23333ms, occurrence 2개), resource 2개, revision 65→66, nextPatternOrdinal 10→11 | 569553B, sha256 `4d6f460f36d951a9…` (백업 `c0f884a5441d48ab…`, 565486B) |
| `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json` | Pattern `KAKULSAYDON_G1_PATTERN_94` "앵콜컷신", 같은 resource 2개, revision 1753→1754, nextPatternOrdinal 94→95 | 2579017B, sha256 `8898052cca5ce422…` (백업 `90611844a94c821e…`, 2574922B) |
| `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.camerashots.json` | shot `kouku.bingo.encore.camera.1` 텍스트 삽입, shots 114→115, revision 90→91 | 1756170B, sha256 `aedf918174085ec8…` (백업 `393c2f78b6d943bf…`, 1754011B) |
| `Data/Effects/V2/Independent.json` | `effects` 목록에 `kouku.bingo.encore.fade.black` 추가 | 2028B, sha256 `67c767bb7d8a96f0…` (백업 `b35915e5e19e4193…`) |
| `Data/Effects/V2/Authored/kouku.bingo.encore.fade.black.effectv2.json` | **신규** V2 페이드 leaf(강도 키 16개, lifetime 23.333, "앵콜컷신 / 원본 암전") | sha256 `dfda2ad8ce1e7282…` |
| `Tools/KoukuSaydonPipeline/build_encore_cutscene.py` | **신규** 추가 전용 생성기 | - |

- 백업: `out/KoukuEncoreComposition20260920/backup/`, 후보: `.../candidate/`, 보고서: `.../report.json`.
- 설치는 쓰기 직전 4개 대상의 바이트 baseline을 다시 확인(CAS)한 뒤 `.encore.tmp` → `os.replace` 원자 교체로 했다.
- 새 Pattern은 folders, bundles, patternFlows, playAllPatternIds에 넣지 않았다(빙고_최종엔딩씬과 같다).
- **1차에는 새 Resources 파일이 없었다.** 2차(10절)에서 baked wmodel과 textures가 새로 생겼고 배포 목록 `Resource_Distribution_2026-09-20_KoukuEncoreActor.txt`가 있다.
- `Client.vcxproj(.filters)`에 새 leaf의 `None` 항목은 등록하지 않았다(프로젝트 파일은 건드리지 않는다는 규칙). VS 탐색용 링크일 뿐 runtime과 무관하다.

## 4. 두 화면에서 보이는 모습(시뮬레이션)

`Render_PatternTree`(`KoukuSaydonActionWorkbench.cpp:4741-4827`)와 `Gate_Label`(4533), `Actor_Label`(500-506)을 읽어 만든 모델(`enc_tree_sim.py`)의 결과.

```
== Boss 탭     (KoukuSaydonComposition.json)
   빙고
     빙고_최종엔딩씬 [Saydon]
     앵콜컷신 [Saydon]
== Sequence 탭 (KoukuSaydonSequenceComposition.json)  [Gate 드롭다운 = 빙고]
   빙고
     빙고_최종엔딩씬 [Saydon]
     앵콜컷신 [Saydon]
SIMULATION PASS
```

- Workbench가 이미 열려 있으면 **`Reload Patterns`** 전에는 보이지 않는다(메모리 draft가 디스크를 자동으로 다시 읽지 않는다).
- Sequence 탭은 선택한 게이트의 항목만 보여 주므로 게이트를 빙고로 바꿔야 한다.

## 5. 원본 대비 숫자 검증(`enc_fidelity.py`)

| 항목 | 결과 |
|---|---|
| 카메라 위치, 0..23333ms, 100ms 간격 | 최대 오차 0.000000m |
| 카메라 fovY | 최대 오차 0.000000° |
| 길이 | 원본 23333.334ms → 설치 23333ms |
| 카메라 키 | 2개, eye (-9.606, 12.261, 950.012), fovY 29.395 |
| 암전 fade, 20ms 간격 | 최대 |원본 − 설치| 0.00265 (reducer 허용 0.003) |
| 암전 끝 | 22.5초 → 0.0, 23.333초 → 1.0 |
| leaf 구조 | 자매 leaf `kouku.gate3.intro.fade.black`과 id/이름/lifetime/키를 제외하면 동일 |

`git diff`(관련 4개 데이터): 382줄 추가, 6줄 삭제. 삭제 6줄은 두 문서의 revision/nextPatternOrdinal 카운터 4줄, camerashots revision 1줄, Independent.json 마지막 원소의 쉼표 변경 1줄이다. 이 수치에는 이 작업과 무관한 미커밋 변경(`Gameplay.world.json` 등)을 넣지 않았다.

## 6. 검증 등급

| 항목 | 등급 | 근거 |
|---|---|---|
| A. Boss 트리에 항목이 있다 | 데이터 + 소스 읽기 | 문서에 Pattern 존재, 트리 시뮬레이션. **화면 확인 없음** |
| B. Sequence 탭에 항목이 있다 | 데이터 + 소스 읽기 | 같음. **화면 확인 없음** |
| C. `빙고_최종엔딩씬`과 같은 방식이다 | 데이터 | 템플릿 Pattern 복제, 같은 함수(`make_cameras`, `fade`, `resource`, `occurrence`, `merge`) 사용, 추가 전용 검증 29항목 × 2문서 PASS |
| D. 원본 앵콜 내용 | **부분적** | 1차: 카메라와 암전만 숫자 일치. 2차: 보스 액터의 이동·애니메이션 추가(10절, skelcontrol·소켓 파츠 제외). 자막·엠블럼·깨진 유리·파티클·조명·사운드는 없음(0절 표) |
| 문서 로더 검증 | 읽기 전용 실행 | `Publish-Compositions.ps1 -Mode Validate` 설치 전후 exit 0 |
| 카메라 문서 검증 | 읽기 전용 실행 | `Publish-MapAuthoring.ps1 -Scope CameraShots -Mode Validate` 설치 전후 exit 0 |
| 프로젝션 일치 검증 | 아래 7절 | `project_kouku_saydon_composition.py --check` |
| 빌드 | 하지 않음 | 이 작업은 JSON/Python만 변경 |
| 게임 실행 | 하지 않음 | 사용자 전용 |

## 7. 프로젝션 검사(`project_kouku_saydon_composition.py --check`)

읽기 전용으로 약 11분 실행했고(12:56 시작, 13:07 종료) 결과는 **exit 1**, 출력은 다음 한 줄이다.

```
KoukuSaydon composition validate failed: projected Product is stale: Data\Encounters\KoukuSaydon\KoukuSaydonEncounter.json
```

- 확정한 것: 검사는 소스 검증 오류(`CompositionError`)가 아니라 **게시된 생성물이 재투영 결과와 다르다**는 비교 실패에서 멈췄다. 즉 새 Pattern이 들어간 Composition을 읽고 투영하는 데까지는 오류 없이 진행됐다.
- 확정하지 못한 것: 이 stale이 **이번 추가만으로 생긴 것인지, 이미 있던 것인지**는 구분하지 못했다. 설치 전 기준으로 같은 검사를 돌리지 않았다(약 11분이 걸려 생략). 그러므로 "이번 변경이 생성물을 깨뜨렸다"도, "원래 정상이었다"도 말할 수 없다.
- 이 검사 뒤에 남은 생성물(첫 stale 파일 이후의 나머지 비교)은 실행되지 않았다. 생성물이 stale인 것은 소스를 고친 뒤 게시 전에 늘 있는 상태이며, 생성물은 손으로 고치지 않고 8절의 `Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon`으로 다시 만든다. 게시는 VS/Client/Server를 끈 뒤 사용자가 결정한다. 이번 작업에서는 게시하지 않았다.
- 검사가 생성물을 쓰지 않았음을 수정 시각으로 확인했다: 12:50 이후 `Data/Encounters`, `Client/Bin/DataFiles`, `Server/Bin/DataFiles`, `Data/Maps`, `Data/Effects`, `Data/KoukuSaydon`, `Data/Compositions`에서 바뀐 파일은 Bern의 `LV_BER_BERNCASTLE.worldsequences.json.linked-save.lock` 하나뿐이며, 이것은 이 작업과 무관한 다른 도구의 잠금 파일이다.

## 8. 적용에 필요한 것

1. Workbench에서 Boss 탭과 Sequence 탭 각각 **`Reload Patterns`**. 이미 저장하지 않은 draft가 있다면 먼저 Save하거나 Reload 후 다시 편집한다(오래된 draft를 저장하면 새 항목을 덮어쓸 수 있다).
2. 새 V2 페이드 이펙트를 Client가 인식하는지는 확인하지 못했다. 안 보이면 Client 재시작이 필요할 수 있다(미확인).
3. 게시(선택, VS/Client/Server가 켜져 있어 이번에 실행하지 않았다). 둘 다 쓰기 명령이므로 VS/Client/Server를 끈 뒤 실행한다.
   - `Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Mode Publish -Scope CameraShots`
   - `Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon` (`Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json` 등 생성물 갱신, 손으로 고치지 않는다)
4. 이 Pattern은 `DRAFT`이며 제품 재생에는 연결되지 않는다.

## 9. 사용자가 결정할 것

- 영상 구간(뒷부분 약 18초)만 쓸지, 원본 전체 창(23.3초, 현재 값)을 쓸지.
- 자막·엠블럼·깨진 유리·보스 액터를 어떻게 표현할지. 각각 새 resource 종류나 UI 작업이 필요하다.
- 이 변경은 PR #417에 들어 있지 않은 작업 트리 변경이다(커밋/푸시하지 않았다).

## 10. 2차: 세이튼 액터 애니메이션 (2026-09-20)

요청: 원본 앵콜 컷신의 보스 액터(쿠크세이튼_03)의 모델 애니메이션과 이동을 **지금 프로젝트가 쓰는 세이튼 모델**로 두 `앵콜컷신` 항목(Boss 탭 `KAKULSAYDON_G1_PATTERN_94`, Sequence 탭 `KAKULSAYDON_G1_PATTERN_10`)에 넣는다. 새 모델은 만들지 않았다. 생성기는 1차의 `Tools/KoukuSaydonPipeline/build_encore_cutscene.py`에 `--actor`(후보만) / `--install-actor`(설치)를 더한 것이다.

### 10.0 가정, 확정하지 못한 것, 못 한 것

- 화면 확인은 하지 않았다. Client/Server/UI를 실행하지 않았고, 액터가 Workbench Preview에서 실제로 보이고 움직이는지는 사용자만 판정한다. "재생된다"가 아니라 "재생되도록 데이터를 넣었다"까지가 이 문서의 주장이다.
- 원본 배우의 **`skelcontrolstrength` 트랙 5개(J_Dn 60점, H_Dn 3점, H_Up 37점, EE_Up 3점, L_T2 4점)는 굽지 못했다.** 이 이름들은 스켈레톤 뼈 이름이 아니라 AnimTree `scene_g.animtree.animblending_kuk9_mix`의 SkelControl 노드 이름이다(168개 뼈 중 일치 0개). 그 트리는 이 PC에 추출돼 있지 않고(기존 baker가 쓰는 `animblending_kuk2` 증거 폴더도 없다) 기존 baker는 이 값을 읽지 않는다. 의미는 추정이다: 머리/턱/귀 같은 작은 뼈의 보정 세기로 보이며, 화면에서 얼굴·머리 자세가 원본과 조금 다를 수 있다. 크기는 확인하지 못했다.
- **`fc1` 슬롯(키 0개, 값 1 하나)과 `hit_color` 재질 트랙(벡터 4점)은 넣지 않았다.** `fc1`은 애니메이션 키가 없는 슬롯이라 재생할 clip이 없다(의미는 표정 슬롯으로 추정). `hit_color`는 피격 색 재질 파라미터로 보이며 이 경로에는 대응 표현이 없다.
- 슬롯 b의 마지막 키(23.667초, att_battle_13_02)는 창(0..23.333초) 밖이라 들어가지 않았다. 슬롯 a는 8키 전부 창 안이고 슬롯 b는 8키 중 7키가 창 안이다. 이동 키는 14개 중 13개가 창 안이고, 마지막 키(23.3333초)는 창 끝 뒤 0.3ms라 들어가지 않았다(그 시각 이후에만 값이 바뀐다).
- 원본 액터 LookInfo(`EFDLChar_MN_RPCT_07`)에는 **소켓에 붙는 추가 파츠 2개**가 있다. 이 방식(기존 SCENE02A 세이튼과 같은 baker)은 몸체만 굽는다. 파츠가 무엇인지 확인하지 못했고 화면에서 빠져 보일 수 있다.
- 새 wmodel과 텍스처는 Resources(Git 비추적)에 있다. 다른 PC는 배포 목록으로 받아야 한다.
- **실제 보스 몸체(`boss.kakulsaydon.bingo.saydon`)는 Server가 그대로 두고 있다.** 이 액터는 컷신용 별도 사본이라 재생 중 세이튼이 둘로 보일 수 있다(기존 컷신도 같은 구조이며, 이 경우가 보이는지는 확인하지 못했다).
- 1차의 나머지 미포함 요소(자막, 엠블럼 UI, 깨진 유리, 파티클, 조명, 사운드, autoblend, Server 쪽)는 그대로다.

### 10.1 원본 액터와 프로젝트 모델

| 항목 | 값 | 근거 |
|---|---|---|
| 원본 그룹/액터 | group 51 `쿠크세이튼_03`, actor export 24 (`efskeletalmeshactorlookinfomat`) | 원본 Matinee 행 |
| 원본 스켈레탈 메시 | `mn_rpct_05.mesh.mn_rpct_05_sk` | 컴포넌트 224의 import |
| 원본 AnimSet | `mn_rpct_00_ani`, `mn_rpct_00_evt2_ani` (2개) | 컴포넌트 224의 import |
| 프로젝트 모델 | `Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel` (뼈 168, clip 249 = ani 176 + evt2 73) | 실측 |
| 원본 부착 | 카메라 액터(row 3)에 hard attach, 상대 위치 (0.44, -0.01, -538.09)cm | 액터 행 `base=3` |

- 원본 메시·AnimSet은 프로젝트 세이튼 몸체(MN_RPCT_05, rpct00 clip 계열)와 같다. 같은 LookInfo를 쓰는 기존 SCENE02A/3관문 도착 세이튼(`kouku.gate3.intro`)이 같은 모델·`modelPreScale` 0.017로 구워져 있어 같은 값을 썼다.
- 원본 AnimSet `mn_rpct_00_ani`의 track 뼈 165개 이름이 프로젝트 스켈레톤 168개에 **전부 있다**(없는 것 0개, 나머지 3개 `RootNode`/`rpct00`/`mn_rpct_05_sk.mo`는 변환기가 더한 노드). 원본 메시 `mn_rpct_05_sk`의 뼈 목록을 직접 대조한 것은 아니다. 구운 wmodel의 뼈 해시·순서·정점 수·서브메시 수는 프로젝트 모델과 같다.
- 원본 액터는 카메라에 붙어 있어 이동 키는 **카메라 로컬 좌표**(x 앞, z 위)다. 첫 키 값(z=+3932cm)은 카메라 바로 위 39m라 0~7.83초에는 화면 밖에 있고, 7.833~8.167초 키(z=-538cm)는 카메라 바로 아래 5.4m라 역시 화면 밖이다.

### 10.2 원본 이동/슬롯 값

- 이동 트랙: postrack 14점 + eulertrack 14점(7.800, 7.833, 8.167, 8.400, 9.333, 9.667, 16.067, 16.100, 20.333, 20.400, 20.867, 20.933, 23.300, 23.333초). 위치가 실제로 뛰는 `cim_constant` 계단은 창 안에 4곳(7.833, 8.167, 20.400, 20.933초)이고 곡선 구간은 8.167→9.667초뿐이다.
- 슬롯 a(8키): 0.000 idle_normal_1(루프) / 9.333 att_battle_12_06(offset 1.0) / 12.533 att_battle_1_01(루프) / 14.533 att_battle_13_01 / 19.667·20.667·21.667·22.667 att_battle_13_02(offset 0.9, end 1.0, rate 0.4).
- 슬롯 b(8키): 7.833 att_phase1_1_06 / 11.500 att_battle_1_01 / 14.000 att_battle_1_01(rate 0.7) / 18.533 att_battle_13_02(rate 0.8) / 20.167·21.167·22.167 att_battle_13_02(offset 0.9, end 1.0, rate 0.4) / 23.667 att_battle_13_02(창 밖).
- 슬롯 a의 floattrack 25점은 a/b 섞음 가중치이고 기존 baker가 그대로 읽는다.

### 10.3 clip 이름 대응

| 원본 clip | 프로젝트 clip | 길이 | 상태 |
|---|---|---|---|
| idle_normal_1 | `rpct00_idle_normal_1` | 3.000초 | 있음(그대로) |
| att_battle_12_06 | `rpct00_att_battle_12_06` | 4.333초 | 있음 |
| att_battle_1_01 | `rpct00_att_battle_1_01` | 2.000초 | 있음 |
| att_battle_13_01 | `rpct00_att_battle_13_01` | 4.667초 | 있음 |
| att_battle_13_02 | `rpct00_att_battle_13_02` | 2.667초 | 있음 |
| att_phase1_1_06 | `rpct00_att_phase1_1_06` | 1.833초 | 있음 |

- 6개 모두 이름이 `rpct00_` 접두사 하나만 다르고(개명), 없는 clip은 없다. evt2 AnimSet에는 같은 이름이 없어(원본 PSA 이름 대조: 겹침 0) 재생 clip이 evt2로 바뀌는 모호함은 없다. 그래서 새 clip을 추출하거나 기존 wmodel에 더하지 않았다.
- 구운 wmodel의 clip 1개는 30 ticks/s로 직접 썼다(700 ticks = 23.333초). 재측정: `wmodel_clip_probe.py` 결과 clips=1, rates=[30.0], 23.333초.

### 10.4 표현 경로: B(World Sequence 액터)

- 선택: 기존 컷신 액터와 같은 **B 경로**(패턴 `worldOccurrences` → `worlds[]` → World Sequence instance/template → `objectResources`의 baked wmodel). 카메라가 있는 컷신 패턴은 모두 이 경로를 쓰고 `animationOccurrences`는 0개다.
- A(패턴 `stages/animationOccurrences`)를 쓰지 않은 이유: A는 Server가 소유한 실제 보스 몸체의 clip을 재생하는 경로다. 이 액터는 카메라에 붙은 컷신용 별도 배우이고 위치가 카메라 상대라, 보스 몸체의 위치·clip으로 옮길 수 없다. 1차의 다른 컷신도 A를 쓰지 않는다.

### 10.5 만든 것

| 파일 | 변경 |
|---|---|
| `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json` | 텍스트 삽입만: objectResource `world.object.kouku.bingo.encore.saydon`, template `sequence.kouku.bingo.encore.saydon`(키 241개, 애니 clip 1개), instance `world.sequence.instance.kouku.bingo.encore.saydon`, revision 2129→2130. 8,075,438B→8,134,085B, 수정 첫 바이트 98(revision 자릿수) |
| `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json` | worlds 행 `kakulsaydon.g1.world.40`, PATTERN_94 `worldOccurrences` 1개(0~23333ms), nextWorldOrdinal 40→41, revision 1754→1755. 2,579,017B→2,579,648B |
| `Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json` | worlds 행 `kakulsaydon.g1.world.34`, PATTERN_10 `worldOccurrences` 1개, nextWorldOrdinal 34→35, revision 66→67. 569,553B→570,184B |
| `Client/Bin/Resources/Map/KakulSaydon/SourceSequences/kouku.bingo.encore/Saydon/Saydon.wmodel` | **신규** 19,809,704B, sha256 `bd9b8945aeffab6e…` |
| 같은 폴더 `textures/` | **신규** 17개(원본 MN_RPCT_05 textures와 바이트 동일), 33.3MB |
| `Resource_Distribution_2026-09-20_KoukuEncoreActor.txt`, `Copy_ResourceDistribution_2026-09-20_KoukuEncoreActor.ps1` | **신규** 18개 파일 53,104,168B |
| `Tools/KoukuSaydonPipeline/build_encore_cutscene.py` | `--actor`/`--install-actor` 추가 |

- 기존 Resources wmodel은 수정하지 않았고 원본 설치 폴더에도 쓰지 않았다.
- 추가 전용 확인: 세 문서 모두 "추가한 행과 카운터를 되돌리면 백업 baseline과 파싱 결과가 같다"(True). Boss/Sequence는 줄 diff에서 **baseline 4줄만 바뀌고**(revision, nextWorldOrdinal, nextWorldOccurrenceOrdinal, `"worldOccurrences": [],`) 29줄이 추가됐다. CRLF·들여쓰기는 그대로다.
- 백업: `out/KoukuEncoreComposition20260920/backup_actor/`, 후보: `.../candidate_actor/`, 보고서: `.../report_actor.json`. 설치는 쓰기 직전 baseline 바이트를 다시 확인한 뒤 `.encore.tmp` → `os.replace`로 했다.
- **설치를 한 번 되돌렸다.** 첫 설치 직후 검증기 두 개가 실패했다: (1) Composition/World Sequence 검증기가 quaternion의 w<0을 거부(`must be normalized with non-negative w`), (2) 투영기가 Boss 문서의 `world.kouku.bingo.encore.saydon`을 거부(`worldId must use kakulsaydon.g1.world.<N> below nextWorldOrdinal`). 백업으로 세 문서를 롤백(해시 baseline 복구 확인)한 뒤 quaternion 부호 정규화와 `kakulsaydon.g1.world.<N>` 형식으로 고쳐 다시 설치했다. 아래 검증은 모두 다시 설치한 최종 상태에 대한 것이다.

### 10.6 숫자 검증

| 항목 | 결과 |
|---|---|
| 이동 template vs 원본(`base.world_pose`), 1ms 격자 0..23333ms | 위치 최대 오차 0.039m(8183ms, 곡선 구간), 회전 오차 0.000° |
| 원본 이동 키 14개 | 13개는 template 키가 ±1ms 안에 있음, 1개(23.3333초)는 창 끝 뒤라 없음 |
| `cim_constant` 계단 | 7800~7840ms, 20395~20440ms, 20925~20970ms 창의 오차 0.000m(1ms 안에 도착), 8160~8175ms(8.167초 계단과 곡선 시작) 창은 0.029m. 첫 후보는 7833ms 계단이 167ms 미끄러짐이었고 `add_step_keys`로 고쳤다 |
| template 키 수 | 241개(상한 256), 시각 단조 증가, 최대 간격 200ms, quaternion 최소 w 0.242, norm 오차 1e-16 |
| 구운 clip vs 프로젝트 원본 clip 재계산(a/b 섞음 포함) | 28개 프레임에서 뼈 위치 최대 차 0.000000, quaternion(1-\|dot\|) 최대 9.8e-8 |
| 구운 wmodel | clips=1, 30 t/s, 700 ticks(23.333초), 뼈 168 = 원본 해시·순서 동일, 정점/서브메시 수 동일 |
| 슬롯 키 | a 8/8, b 7/8이 창 안(b 1개는 23.667초라 밖) |

카메라 절두체 투영(카메라 `kouku.bingo.encore.camera.1`: eye (-9.606, 12.261, 950.012), lookAt (-4.608, 5.190, 945.011), fovY 29.395°, 16:9 수평 50°). 뼈 168개를 30fps 701프레임 모두 실제 clip 포즈와 template 자세로 투영한 값이다(뼈 위치 기준이고 스킨 메시 경계는 아니다).

| 시각(초) | 루트와 카메라 거리 | 루트 NDC (x, y) | 화면 안 뼈 | 머리 뼈 NDC (x, y) |
|---|---|---|---|---|
| 0.0~8.0 | 전방 거리 0(카메라 바로 위 39m / 바로 아래 5.4m) | 화면 밖 | 0/168 | - |
| 8.3 | 2.75m | (0.03, -2.37) | 7/168 | (-0.17, 2.68) |
| 8.4 | 3.79m | (0.01, -0.75) | 39/168 | (-0.02, 1.56) |
| 9.0 | 3.79m | (0.01, -0.75) | 103/168 | (0.06, 0.91) |
| 10.0 | 4.24m | (0.01, -0.82) | 35/168 | (0.09, 1.57) |
| 12.0 | 4.24m | (0.01, -0.82) | 67/168 | (0.08, 1.17) |
| 14.0 | 4.24m | (0.01, -0.82) | 107/168 | (-0.27, 0.50) |
| 18.0 | 4.24m | (0.01, -0.90) | 123/168 | (0.12, 1.12) |
| 20.0 | 4.24m | (0.01, -0.90) | 66/168 | (0.10, 1.26) |
| 20.5 | 1.62m | (-0.03, -3.11) | 34/168 | (0.18, 2.76) |
| 21.5~23.3 | 0.80m | (-0.17, -6.58) | 30/168 | (0.23, 6.14) |

NDC는 ±1이 화면 가장자리다. 표는 30fps 701프레임 중 대표 시각이다.

- 701프레임 중 뼈가 하나라도 화면 안에 들어오는 프레임은 456개(첫 약 8.2초는 0개).
- 루트-카메라 거리가 원본 카메라 로컬 x 키(379/424/162/80cm)와 3.79/4.24/1.62/0.80m로 일치한다. 이것은 카메라 부착 좌표 해석이 맞다는 강한 정황이다(위치를 임의로 넣었다면 이 네 값이 맞을 이유가 없다).
- 해석(추정): 8.4~20.4초는 세이튼이 카메라 앞 4m에서 화면 아래쪽 중앙에 서고 머리는 화면 위쪽 가장자리 근처(y 0.5~1.6)에서 자주 잘리며, 20.4초 이후 카메라 쪽 1.6m→0.8m로 돌진해 화면 대부분을 가린다. 깨진 유리 후처리와 겹치는 장면으로 보이지만 이것은 화면으로 확인하지 못한 추정이다.

### 10.7 검증기 실행(모두 읽기 전용, 설치 전후)

| 검증기 | 설치 전 | 설치 후(최종) |
|---|---|---|
| `Publish-Compositions.ps1 -Mode Validate` | exit 0, patterns 42 / actions 349 | exit 0, 같은 수치 |
| `Publish-MapAuthoring.ps1 -Scope WorldSequences -Mode Validate` | exit 0(435초) | exit 0(353초) |
| `Publish-MapAuthoring.ps1 -Scope WorldSequences -Mode Check` | 실패: runtime이 authoring과 다름 | 실행하지 않음(게시 전이라 당연히 다르다) |
| `Publish-MapAuthoring.ps1 -Scope CameraShots -Mode Validate/Check` | Validate 통과, Check 실패(runtime과 다름) | 이번 변경 대상 아님 |
| `project_kouku_saydon_composition.py --check` | exit 1, `projected Product is stale: Data\Encounters\KoukuSaydon\KoukuSaydonEncounter.json` (876초) | exit 1, 같은 메시지 `projected Product is stale: Data\Encounters\KoukuSaydon\KoukuSaydonEncounter.json`(736초) |

- Map Check 실패는 이번 변경 전부터다(게시하지 않은 다른 authoring 변경이 이미 있다). 이번 변경 때문에 생긴 실패가 아니다.
- 프로젝션 `--check`는 설치 전(1차 설치 뒤)과 후 모두 같은 파일에서 stale로 멈췄다. 소스 검증 단계(worldId 규칙 포함)는 이번 설치 후에도 통과했다(첫 설치 후에는 `worldId must use ...`로 그 단계에서 실패했다). 게시본 비교는 첫 stale 파일에서 중단되므로 이번 추가가 뒤쪽 생성물(예: 패턴 바인딩)에 미칠 차이는 확인하지 못했다. 1차 설치 전 기준은 없어서 그 stale의 원인이 1차 이전부터인지는 여전히 확정하지 못한다. 게시(`Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon`)를 하면 이 stale이 해소되는지는 실행하지 않아 미확인이다.

### 10.8 검증 등급

| 항목 | 등급 |
|---|---|
| 원본 액터·이동·슬롯 값 | 원본 확인(Matinee 행 읽기) |
| 새 wmodel과 template 실측 | 데이터 실측 |
| 검증기 통과 | Composition Validate, Map WorldSequences Validate. 프로젝션 `--check`는 설치 전후 같은 stale에서 멈춤 |
| 카메라 절두체 투영 | 수치 투영(뼈 기준) |
| 트리/미리보기 | 시뮬레이션만(1차 트리 모델 유지, World lane이 추가되는 것은 `KoukuSaydonActionWorkbench.cpp`의 `includeLane(pattern.WorldOccurrences)`로 확인) |
| 게시(publish) | **하지 않음** (VS 실행 중, 쓰기 명령) |
| 빌드 | 하지 않음(JSON/Python/리소스 파일만) |
| 화면 확인 | **하지 않음**(사용자 전용) |

### 10.9 적용에 필요한 것

1. Workbench Boss 탭·Sequence 탭 각각 **`Reload Patterns`**(1차와 같은 조건). Preview가 authoring `worldsequences.json`을 읽는지는 소스로 추정만 했고 확인하지 못했다.
2. Server/Client 제품에서 쓰려면 게시가 필요하다. VS/Client/Server를 끈 뒤: `Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Mode Publish -Scope WorldSequences`, `-Scope CameraShots`, 그리고 `Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon`.
3. 다른 PC에는 `Resource_Distribution_2026-09-20_KoukuEncoreActor.txt`의 파일을 같은 상대 경로로 전달한다(복사 스크립트: `Copy_ResourceDistribution_2026-09-20_KoukuEncoreActor.ps1 -Source <받은 Resources> -Destination <본인 Client/Bin/Resources>`, 임시 폴더로 복사 실행해 18개·53,104,168B 확인).
4. 이 Pattern은 여전히 `DRAFT`이며 제품 재생에는 연결되지 않았다.

### 10.10 사용자가 결정할 것

- 화면에서 세이튼이 실제 보스 몸체와 겹쳐 둘로 보이면, 컷신 동안 실제 보스를 숨길지(별도 작업).
- skelcontrol(얼굴·머리 보정)이 눈에 띄게 다르면 `animblending_kuk9_mix` AnimTree를 새로 추출해 baker를 확장할지.
- LookInfo의 소켓 추가 파츠 2개를 표현해야 하는지(무엇인지부터 조사 필요).
