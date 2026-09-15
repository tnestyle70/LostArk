# 기존 컷신 카메라·맵 애니메이션 계획서 사실 감사

검사일: 2026-09-15 KST. 검사 저장소: `C:/Users/USER/source/졸업팀폴/LostArk`.

## G00. 판정과 수행 범위

이전 계획서는 완벽하지 않았다. 현재 저장 JSON의 직접 참조 조인, 현재 C++의 저장/재생 호출 경로, 기존 원본 추출물의 header/property를 대조하여 실행 지시의 누락을 확인했고 PLAN/PROMPT를 수정했다. 문장 일관성 검토만으로 구현 경로가 안전하다고 판단할 수 없다는 사례다.

수정 범위는 PLAN/PROMPT 및 이 감사 기록뿐이다. 제품 C++·데이터·Resources·runtime DataFiles를 수정하지 않았다. Client/UI 실행·조작·캡처, 빌드, publisher 실행과 새 시각 결과 판정도 하지 않았다.

## G01. 재현한 문제와 설명서 교정

### G01-1. 2관문 카메라 연결 오류는 조건부 추측이 아닌 현재 저장본의 사실

Composition revision59의 P3 occurrence→presentation resource.assetId→Area camera shot을 조인했다.

| 대상 | 현재 데이터 | 확인 결과 |
|---|---|---|
| presentation.4 | 시작19490ms, 길이4460ms → camera.4의 마지막 키2224ms | 마지막 키 이후2236ms를 박스가 더 사용 |
| presentation.5 | 시작23950ms, 길이3050ms → camera.5의 마지막 키1104ms | 표시명은cam6지만 실제 camera.5는cam4 중간 조각, 마지막 키 이후1946ms를 더 사용 |
| camera.6 / camera.7 | 카메라 파일에1132ms/3050ms 샷 존재 | 현재P3 occurrence가 사용하지 않음 |

`Client/Private/KoukuSaydonPresentationPlayer.cpp:1850`이 occurrence age를 `Sample_CompositionCamera`에 전달하고, `Level_KakulSaydonArena.cpp:3926` 부근에서 `Sample_Cue`를 호출한다. `ValtanCinematicCameraController.cpp:244`의 `Sample_Cue`는 마지막 키 이후 마지막 pose를 반환한다. 따라서 해당 샷이 정상 재생 중이라는 조건에서21714~23950ms 및25054~27000ms는 마지막 pose 유지 경로다. 새 Client에서 직접 관찰한 재생 증거가 아니라 코드·데이터로 확인한 결과다.

기존7샷의 길이를 잇는 경계는0/2100/13950/19490/21714/22818/23950/27000ms다. 기존 샷을 재사용하고 Composition의 필요한 참조/박스만 복구하도록 명시했다. 샷 전체 재추출/원본 동등성까지 PASS로 확대하지 않는다.

### G01-2. raw.json도 이미 좌표·FOV가 변환된 자료

`out/GateIntroPlan/SCENE04A.raw.json`, `SCENE02A.raw.json`, `out/EncoreFinalPlan/SCENE01B.raw.json`, `SCENE01C.raw.json`의 header는 m/Y-up, `(x*.01,z*.01,-y*.01)`, FOV 변환 aspect1.500을 명시한다.

현재 `Tools/KoukuSaydonPipeline/build_gate2_intro_composition.py:513`은 수평FOV를16/9 기준으로 수직FOV로 바꾼다. 따라서 raw의 변환 좌표를 재변환하거나 서로 다른 aspect의 fovY를 직접 비교하면 검증 기준이 잘못된다. source 수평FOV/UPK속성과 동일한 검증조건으로 비교하도록 경고를 추가했다. 어느 aspect가 현재 실행에 맞는지는 실제 viewport 확인 전 확정하지 않았다.

동일 파일의 `extract_scene():73`은 property 해석의 `ExtractionError`를 빈 properties로 처리한다. 추출 함수가0개를 반환했다는 것만으로 원본 맵 동작 없음이라고 선언하지 않도록 실패/필터/부모/event 커버리지 검사를 추가했다.

### G01-3. Map Tool Save는 World 단독 저장이 아님

`WorldSequenceToolPanel.cpp:1311`→`MapTool.cpp:796`→`Save_AllAuthoring():591`을 확인했다. dirty Deploy, Map/World, SpawnGroups, Gameplay, Navigation, Destruction을 저장할 수 있다. 다른 도메인 dirty를 먼저 확인·보존하고, 사용자 승인 없이 함께 Save/Discard하지 않는 사전조건을 넣었다.

### G01-4. Object Save에는 자동 게시 부작용이 있음

`WorldObjectTool.cpp:624`는 전투/Sequence Composition의 emission 참조를 동기화할 수 있다. 저장 성공 후`:676`의 `Start_Publish`와`:699`의 WorldSequences Publish를 거친다. 이후`:729`의 linked callback과 `MainApp.cpp:7476`에서 연결 전투 Pattern이 있으면 `Publish_AllPatterns()`까지 실행할 수 있다.

Sequence 자체의 로컬 Save와 구분했고, Save 전 공유 소비자·emission 변경·자동 게시 대상 검사를 필수로 했다. 임시 미리보기를 위해 먼저Save하는 방법과 자동/수동 publisher 병렬실행을 금지했다. 현재Table이 실제 전투참조를 갖는지까지 이번에 확정했다는 뜻은 아니다.

### G01-5. 편집 버튼 존재와 전체 draft 미리보기 지원은 다름

`MainApp.cpp:1903,1928`은 전체 Sequence preview에 `WorldObjectTool::Get_SavedDocument()`를 전달한다. Object의 미저장 draft가 전체 장면에 바로 반영된다고 보장할 수 없다.

`WorldSequenceToolPanel.cpp:266`은 targetId를 uint64로 파싱한다. `:776` 이후 DEPLOY가 아닌 binding은 MAP placement로 찾는다. 현재Table의 binding은 `OBJECT_RESOURCE / world.object.kouku.gate2.intro.table`이므로 이 경로에는 단순ID전달로 진입할 수 없다.

PLAN은 전체 Pause/Seek→draft편집→동일시각재평가를 추가 구현·검증 요구로 구분한다. Table은 기존Object편집기를 우선 사용하며, MapToolfallback이 필요하면 기존Object평가 경로를 재사용한 target해석/객체준비/애니메이션/복구까지 연결해야 한다. 버튼이 있으므로 이미 가능하다는 안내를 제거했다.

## G02. 다시 확인된 현재 값

- Sequence revision 59, 패턴 8개. P3는 WORLD 31 / CAMERA 5, P7은 18,658ms / WORLD 1 / CAMERA 8, P8은 GATE1 `1관문_연출`이다. 이 Sequence 문서에는 BINGO 종료 패턴이 없다. 다른 저장 위치 전부에서 종료가 없다는 뜻은 아니다.
- 현재 2관문 backdrop instance 14개, binding 130개다. 개수 확인이지 130개 pose의 원본 동등성 검증은 아니다.
- Table template은 27,000ms이며 `gate2_intro_27s`, start 0, rate 1, loop false, holdLastFrame true를 참조한다. 전체 WModel 뼈 pose를 이번 감사에서 비교하지 않았다.
- 기존 SCENE04A `rows` 추출물의 Table 클립은 11.0초 reverse 및 15.6599998474초 forward를 기록한다. 최근 UPK와의 바이트 대조/재추출은 하지 않았다.
- raw 요약의 선택 group/Director 수는 SCENE04A 98/5, SCENE02A 67/13, SCENE01B 23/10, SCENE01C 23/10이다. 대응 UPK 파일 4개의 존재를 확인했다. 존재/캐시 파싱은 최신 UPK 내용 검증과 다르다.
- World 13,238,360 bytes, reader 16MiB 상한(`WorldSequenceDocument.cpp:31`). Camera 1,652,277 bytes, reader 2MiB 상한(`Level_KakulSaydonArena.cpp:3305`).
- publisher scope는 Area/WorldSequences/Lights이며 Camera-only는 없다. Area 게시가 다른 도메인 출력을 포함하므로 pending 조명뿐 아니라 Effect/Material/배치 차이도 사전 확인 대상으로 고쳤다.

## G03. 검사 입력과 재현

읽기전용 JSON 조인 재현 스크립트:
`C:/Users/USER/.codex/worktrees/7395/LostArk/out/CutscenePlanAudit20260915/audit_current.ps1`.
실행 시각은 2026-09-15T12:03:57+09:00. 저장 자료·변환 요약을 읽고 표준 출력에만 결과를 낸다.

| 입력 | 검사 당시SHA256 |
|---|---|
| Sequence Composition | `085B53075F79073B3B89CD9A4130860CDC82FB9D52E2941DC6711641E38301A9` |
| Area camerashots | `D3B0056E23FA77500561468E32D54E2F4AEC0F3043C60E7EC977309965A646C0` |
| Area worldsequences | `0A6B08B99852D5DB30094A30E83A464B82DA27DEB61AF2B64348684E2A7A1975` |

위 값은 감사 입력 식별용이며 새 resource 배포 계약이 아니다. 변경된 JSON이나 미저장 draft에는 기존 검사 결과를 재사용하지 않는다. 현재 branch는 `feature/kouku-cardmaze-visual-fix`, 다른 작업의 C++/셰이더/Encounter/Animation dirty 변경이 있어 보존했다. `git fetch`는 FETCH_HEAD 접근 거부로 실패했으므로 remote 최신 상태를 검증했다고 하지 않는다.

## G04. 아직 주장할 수 없는 것

1. 네 컷신 전체의 카메라 곡선/뼈 pose/부모 변환이 원본과 동등하다는 판정. 이번에는 현행 연결·코드·캐시의 주요 주장을 감사했다.
2. 모델 확대가 모든 구도 불일치의 확정 원인이라는 판정. 현재 pose/scale 및 동기화된 화면을 개별 대조해야 한다.
3. 1관문 볼륨 카메라의 시간 재생/경로 진행도 매핑. 이 감사에서 해결하지 않았다.
4. 현재 없는 종료 패턴의 자동 추가, 3관문 앞부분 확장으로 제외 트랙 시각을 바꾸는 권한. 기존 승인 경계를 유지한다.
5. 미저장 draft의 전체 실시간 미리보기·Save/Reload 왕복·게시 성공·새 실행 파일의 화면 동작. 코드 존재와 실행 결과를 구분한다.

따라서 수정본은 **확인된 사실을 기준으로 다음 조사/선택 수정을 시작할 설명서**이지, 그대로 실행하면 네 영상이 완벽히 같아진다는 보증서가 아니다. 확인된 연결 오류 복구와 나머지 원본 대조를 분리하고, 수동 검증 전 전체 완료를 선언하지 않는다.

## G05. 실행 결과 — 연결 복구와 네 컷신 원본 대조 (2026-09-15 Claude)

작업 시작 시 정본 해시가 G03 감사 값과 같았다(Composition rev 59, camerashots rev 84, worldsequences rev 1848). 백업은 `out/KoukuFourCutscenes20260915/backup/`에 있다. Client는 실행 중이 아니었고, 툴 draft는 없었다. 연결 인벤토리는 `out/KoukuFourCutscenes20260915/inventory.txt`(`inventory.py`)에 있다.

### G05-1. 오류 복구 (적용, Composition만)

| Pattern | 확인한 오류 | 적용 | 근거 |
|---|---|---|---|
| P3 2관문_진입컷씬 | 박스 4 (4460ms)가 샷4 (2224ms)를 참조했다. 박스 5 (cam6 표시)는 실제로 cam4 중간 샷5 (1104ms)를 참조했다. 샷 6·7은 연결이 없었다 | presentation.4 → 2224ms. presentation.5 → 21714+1104ms (리소스 표시명 cam4). 리소스 `presentation.kouku.gate2.intro.camera.6/.7` 추가. 박스 `.presentation.9` (22818+1132)·`.10` (23950+3050) 추가. nextPresentationOccurrenceOrdinal 9→11 | 경계 0/2100/13950/19490/21714/22818/23950/27000ms. 각 박스 길이 = 샷 마지막 키. 재파싱 결과 = 원본+의도 변경 |
| P7 3관문_진입 | 도착 구간 박스 8개 (c4…c6, 3851…2244ms)가 원본 앞부분 샷 `kouku.gate3.intro.camera.1~8` (c1…c3)을 참조했다 | 리소스 8개의 assetId만 camera.1~8 → camera.11~18로 바꿨다. 박스 이름·시각·길이는 그대로 | 샷 11~18의 길이·원본 시계 위치 (P5 −16710ms)가 박스와 정확히 같다. 숫자는 `kouku.gate2.clear.camera.11~18`과 동일 (sceneId 이름만 다름). 리소스는 P7 전용 |

- 원인: 병합 `400439b2`가 Composition만 G13 이전으로 되돌리고, 카메라 파일은 G13 샷 번호를 유지했다.
- 결과: Composition rev 59→60 (P3)→61 (P7), SHA-256 `87DED1AE25D7B19F…`. 샷·World·런타임 파일은 바꾸지 않았다.
- 검사:
  - C++ reader의 해당 검증 규칙과 대조했다 (`KoukuSaydonCompositionDocument.cpp` 박스 ID 순번·시간 범위·다른 소유자 간 CAMERA 겹침).
  - CRLF·`git diff --check`를 확인했다.
  - 적용 스크립트: `fix_p3_camera_links.py`, `fix_p7_camera_links.py`. 기준 해시가 다르면 쓰지 않는다.
  - Sequence Composition은 Sequence 편집기가 직접 읽으므로 게시·빌드가 필요 없다.

### G05-2. 네 컷신 대조 판정 요약

대조표 본문은 컷신별 폴더에 있다: `gate2/GATE2_P3_원본대조표.md`, `gate3intro/`, `gate1start/GATE1_START_COMPARISON.md`, `gate3end/`. 평가는 설치 생성기와 다른 독립 코드로 했다.

| 컷신 | 동일 유지 | 오류 복구 | 현재 모델 대응 구도 보정 | 미확정 / 연결 없음 |
|---|---|---|---|---|
| 2관문 진입 (P3) | 카메라 7샷 (키 오차 0, 키 사이 ≤25mm)<br>세이튼·쿠크·의자·책·테이블 (역재생 11.0/정재생 15.66)·받침 기둥 25조각 (≤0.02mm/0.03°)<br>카드 박스 시각<br>모델 유효 scale 전부 원본과 같음 | G05-1 연결 복구 | 후보 없음 (모델 크기 변화 없음) | 데스크기둥 상대 트랙 해석<br>매달린 카드 흔들림 파형<br>손 부착 35조각·HandBook/촛대 lookup 구간<br>화면비 (16:9 vs 영상 1.5)<br>영상 첫 21프레임 페이드 |
| 3관문 진입 (P7) | 샷 11~18 (위치 ≤5.2mm, 방향 ≤0.1°, 샷15 한 표본 0.82°)<br>분할 이음새<br>SaydonArrival 루트 | G05-1 연결 복구 | 수치 후보 없음. 원본 카메라에서도 일부 샷에서 루트가 화면 아래로 나감 | 원본 앞 0~16.71초 구간 없음 (늘리면 배우·암전·FX 시각 이동 필요)<br>slomo 0.2× (28.94~29.51s)·카메라 흔들림 4건 미반영<br>화면비<br>원본 맵 애니메이션 없음 (정상) |
| 1관문 시작 (P4, 공유 P1/P2/AUTO) | `.24~.27` 카메라 (≤0.03m)<br>벽 placement 3·419 (≤0.05°)<br>책 속도 0.7 (원본 `animplayrate` 0.7과 같음)<br>팝업맵 61개 (≤5cm, scale 같음) | 없음 | 세이튼 bodyModelPreScale 0.017 (원본 drawscale 1.2 대비 약 1.42배는 cm 단위 전제의 추정). 얼굴 잘림 후보 P4 36959/37246/50530ms. P8 배우 위치 미대조라 보정 수치는 제안하지 않음 | `.23` 피날레 정지 샷 (원본 직접 카메라 없음, 볼륨 cm01 매핑 미확정)<br>`.29` 책 카메라 로컬 0~12099ms가 원본과 다름 (원본과 같은 `kouku.gate1.full.camera.6~18`은 있으나 미사용). 교체 시 맵·책 시계와 카메라 시계 중 선택 필요<br>책 카메라 FOV 3:2 vs 나머지 16:9<br>팝업맵 나머지 75개 (G06 P1 저작 채택 기록)<br>`.28` 21ms 박스는 `d970b2c1` 편집 (의도된 편집으로 유지) |
| 3관문 종료 | 남은 샷 `kouku.bingo.ending.camera.1~12`가 원본 SCENE01B 10컷과 같음 (eye ≤4.9mm, 방향 ≤0.31°) | 없음 | 해당 없음 | **현재 연결 없음**. 병합 `400439b2`로 패턴이 사라졌고 샷 12개는 소비자 0. 원본 맵 애니메이션 없음. 신규 패턴 (P9, CAMERA 12박스, WORLD 0) 구성안만 있고 미적용 |

### G05-3. 사용자 결정 대기

1. 3관문 종료 패턴을 새로 추가할지 (기존 샷 12개 재사용, `gate3end/` 구성안). 빙고 gate가 툴 목록에 표시되는지는 확인하지 못했다.
2. P7 원본 앞부분 (0~16.71초)·slomo·카메라 흔들림을 범위에 넣을지. 넣으면 배우·암전·FX 시각이 함께 움직인다.
3. P4 책 카메라 0~12099ms를 원본 샷 (`full.camera.6~12`)으로 바꿀지, 바꾼다면 어느 시계에 맞출지.
4. 화면비 기준 (16:9 저장 vs 영상·일부 샷 3:2).
5. 1관문 세이튼 확대에 따른 구도 보정. 사용자 화면 확인 후 해당 샷의 시점·주시점만 보정한다.

### G05-4. 사용자 확인 경로

F1 → Action Workbench → Composition Actions → Sequence에서 각 Pattern을 재생한다.

- **2관문 진입:** P3 `2관문_진입컷씬`. 19.49s (cam4 시작)·21.71s·22.82s (샷 분할 이음새, 끊김 없어야 함)·23.95s (cam6 근접 컷)에서 23.95초 이후가 cam4 중간 구도에 멈춰 있지 않아야 한다.
- **3관문 진입:** P7 `3관문_진입`. 0s 텐트 전경 (c4)·3.85s 세이튼 클로즈업 (c4_a1)·9.71s 이후 무대 (c6). 이전처럼 광대 인형·폭죽·보라색 터널 (원본 앞부분 카메라)이 나오지 않아야 한다.
- **편집:** CAMERA 박스 → Open Composition Camera (Save Camera → `camerashots.json`), 박스 시각은 Sequence Save (`KoukuSaydonSequenceComposition.json`).

화면 판정은 아직 없다. 커밋·푸시는 하지 않았다.

## G06. 1관문 책 카메라 얼굴 구도 수정 (2026-09-15 Claude, 적용)

사용자가 비교 캡처(`바탕 화면/1관문 컷신`)에서 922·1115~1147·1602번과 스크린샷 3장(P4 30125/36014/36886ms)을 지목했다. 원본은 세이튼 얼굴·상반신을 확대하는데 우리는 다리가 보였다. 사용자 지시는 "카메라 이동만 수정, 나머지는 건드리지 않음, 분석 끝나면 바로 1관문만 수정"이다.

### G06-1. 원인 (계산 사실, `out/KoukuFourCutscenes20260915/gate1start/framing_fix/`)

- **크기:** 우리 세이튼 얼굴은 루트 위 2.03m, 원본은 1.47m다(preScale 0.017 vs 원본 0.012, 1.417배). 원본 카메라 조준점으로 찍으면 얼굴이 화면 위로 나간다.
- **카메라 키 값:** 책 로컬 9600~12099는 cm02 고정 카메라 대신 약 1m 미끄러지는 경로다. 16800~17228의 4키는 원본 경로 밖이다(눈 0.36~0.56m, fovY 5.78°).
- **시계:** 로컬 15729까지 카메라는 맵·책 시계(P4 = 114 + 9740.4ms)다. P8 세이튼 동작은 P4 = 114 + 13421→12585ms라, 동작이 3.68→2.84초 늦는다. 37초 캡처의 옆모습/정면 차이는 이 동작 시각 차에서 온다(카메라 방위각 차 0.0°).
- **1602:** 로컬 17229~37800이 원본 26.92~41.33초를 1.41배로 늘린 것이 맞다. 그래서 카메라가 동작보다 2.11초 뒤처진다.
- **이전 표 정정:** `GATE1_START_COMPARISON.md`의 책 카메라 재배치표(0→8752 등)는 실제 키 시계가 아니다. 바탕 화면 비교 캡처의 1관문 책 구간 원본 짝은 최대 약 −1.47초 어긋나 있다.

### G06-2. 적용

- **파일:** `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.camerashots.json`
  - revision 84→85.
  - 샷 `kouku.gate1.authored.book`의 `cameraTrack.keyframes`만 60개→64개(A_ACTOR 안)로 바뀌었다.
  - durationMs 37800과 나머지 샷 109개, `2Stage.book`은 같다.
- **스크립트:** `framing_fix/apply_book_camera_fix.py`. 기준 SHA 확인, 앵커 splice, CRLF 유지.
- **백업:** `framing_fix/backup_camerashots_rev84.json`.
- **A_ACTOR 안:** 원본 카메라를 원본 배우 기준으로 표현한 뒤 우리 보스 루트(−0.07, 1.3, 737.53)·yaw(+2.79°)·크기 k=1.417에 맞춰 다시 놓았다. FOV는 원본 수평 FOV를 16:9로 환산했다. 원본 camera roll은 up으로 넣었다.
  - 구간 시계는 cm01·cm02가 맵·책 시계, cm03 22.2438초 이후가 P8 시계다.
  - 카메라 움직임은 로컬 34114(P8 끝)에 끝나고 37800까지 유지한다.
- **이 안으로 함께 바뀐 부분 (근거):**
  - 로컬 0~9521은 경로가 같다. FOV는 3:2 환산(약 54~58°)에서 16:9(약 43~50°)로 좁아지고, 시작 roll 43°가 2.4초 동안 0°로 돌아온다.
  - 원본 영상 대조: 책 로컬 300·900ms에서 원본 책은 대각선이고 우리는 서 있다. 4000ms에서 우리 화면이 원본보다 넓다.
  - 1500ms의 책 각도 차이는 책 애니메이션 쪽이라 범위 밖이다.

### G06-3. 검증 (실행한 것만)

- **파서 규칙:** `Level_KakulSaydonArena.cpp:118-126, 418-540` 규칙을 통과했다(키 ≤64, 첫 키 0, 시간 증가, 마지막 = 37800, 좌표·시선 거리·FOV·up 조건).
- **독립 재계산:** 원본과 같은 `kouku.gate1.full.camera.14`에서 원본 cm03 26.724초를 재조준 식으로 다시 계산했다. 제안 키 k32와 눈 위치 차이는 최대 0.006m, FOV 6.767°로 같다.
- **파일 비교:** 적용 전후 JSON 비교에서 바뀐 샷은 `kouku.gate1.authored.book` 하나다. CRLF만 쓰고 크기는 1,661,278 bytes(2MiB 미만)다. `git diff --check` 출력은 없다.
- **분석 포크 투영 검증:** 세 캡처와 1602의 원본 장면에서 얼굴 화면 좌표 차 ≤0.33, 어깨 방향 차 ≤0.4°다.
- **남는 불확실성:**
  - frame 922·1602 캡처는 계산보다 몸이 크게 잡힌다. 녹화 당시 보스 위치·크기 등 실행 상태 차이는 미확인이다.
  - cm02 유지 구간(로컬 12025~15705)에서 우리 동작 때문에 얼굴 6샘플이 화면 밖이다.
  - 로컬 15706 이후 카메라는 원본 영상보다 3.68→2.84초 늦게 진행한다(P8 동작에 맞춤).

### G06-4. 반영 경로와 사용자 확인

- **미리보기 재생:** 저작 원본 camerashots를 읽는다(`KoukuSaydonPresentationPlayer.cpp:1822` preview = !product, `Level_KakulSaydonArena.cpp:3896-3897`). 게시 없이 반영된다. 이미 켜진 Client면 Composition Camera의 Reload Cameras가 필요하다.
- **제품 재생(F1 Complete Play 등):** 게시 사본 `Client/Bin/DataFiles/Map/LV_LUT_MIDNIGHTC_ED.camerashots.json`을 읽는다(현재 rev 84). `Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Scope Area -Mode Publish`가 필요하며, 아직 실행하지 않았다(Visual Studio 열림).
- **볼 시각 (P4, 새 키 기준으로 계산):**
  - 30.0초: 원본 20.308초 장면, 얼굴·가슴 근접.
  - 39.4초: 원본 25.999초 장면, 옆모습 전신.
  - 40.2초: 원본 26.799초 장면, 얼굴 대형 근접.
  - 50.2초: 원본 37.508초 장면, 상반신.
  - 54.1초: 카메라 움직임 끝(= P8 끝).
  - 이전 캡처의 36.0/36.9/52.1초는 옛 키 기준이라 새 카메라에서는 위 시각으로 옮겨졌다.
  - 책 시작 0~2.4초 기울기와 좁아진 화각도 함께 본다.
- 화면 판정은 아직 없다. 커밋·푸시는 하지 않았다.
- 2026-09-15 사용자 관찰(서면): "1관문 카메라는 된거같아". 세부 시각별 판정은 받지 않았다.

## G07. 3관문(P5 2관문_클리어·P7 3관문_진입) 카메라 구도 수정 (2026-09-15 Claude, 적용)

사용자 스크린샷 6장: P5 11373 / 14884 / 23620 / 26614 / 27198 / 28851ms. 사용자 지시: "1관문처럼 해봐"(카메라 키만).

### G07-1. 원인 (분석 `out/KoukuFourCutscenes20260915/gate3intro/framing_fix/GATE3_FRAMING_FIX_ANALYSIS.md`)

- **카메라:** 6곳 모두 원본 디렉터 컷 카메라와 같다(위치 ≤4.8mm, 방향 ≤0.57°, FOV ≤0.015°). 원본 영상 11.433초·23.633초 변화는 컷이 아니라 섬광이고, 화면비는 둘 다 16:9다.
- **배우 크기:**
  - P5 배우 템플릿의 `scaleMultiplier`는 모든 키에서 [1,1,1]이다(직접 확인). 원본은 7.657~8.257초에 대형 세이튼 drawscale을 6→2, 쿠크를 1→0.3으로 줄인다.
  - 크기 비율(원본/우리): 대형 세이튼 0.289, 쿠크 0.249(8.257초 이후), 도착 세이튼 0.588(원본 drawscale×0.01 전제, 뼈대 겹침으로 확인, 표 값 미확인).
- **샷 분할:** 원본 컷 하나가 64키 상한 때문에 여러 샷으로 나뉘어 있다. c3 = 샷 8·9, c3_a1 = 10, c4_1 = 13, c6 = 14~18.

### G07-2. 적용

- **파일:** `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.camerashots.json`, revision 85→86.
- **바뀐 값:** 키의 `eye`·`lookAt`만 바뀌었다. 키 시간·up·FOV·키 수·샷 길이는 같다.
  - P5: `kouku.gate2.clear.camera.8/9/10`(기준 배우 대형 세이튼), `13/14/15/16/17`(기준 도착 세이튼).
  - P7 쌍둥이: `kouku.gate3.intro.camera.13~17`. 원래 키가 P5와 수치로 같고, P7 도착 세이튼 경로가 P5를 16710ms 당긴 것과 같다(샷 13~17 구간 위치·회전 차 0.0).
- **사용처:** 샷마다 사용처는 하나다(P5 presentation.8~10·13~17, P7 presentation.3~7). AUTO와 다른 패턴은 없다.
- **식:** `eye' = T + (eye − T)/k`, `lookAt' = T + (lookAt − T)/k`. T는 키 시각의 기준 배우 뿌리이고, 1관문 G06과 같은 원리다(배우가 뿌리 기준으로 커진 만큼 카메라를 멀리 둔다).
- **샷 17:** c6 컷의 넓은 끝이다. 전량 적용하면 70~80m까지 멀어져서, 배율을 1/k에서 1로 smoothstep으로 되돌렸다. 샷 18은 그대로다.
- **스크립트와 백업:** `prepare_gate3_camera_fix.py`(계산·검증) → `gate3_camera_fix_keys.json`, `apply_gate3_camera_fix.py`(eye/lookAt 배열만 제자리 교체). 백업은 `backup_camerashots_rev85.json`.

### G07-3. 검증 (실행한 것만)

- **기준 배우 머리 화면 오차(원본 화면 안 샘플, 100ms 간격, 전→후):**
  - 8: 2.281→0.003
  - 9: 2.408→0.005
  - 10: 비교 불가(우리 머리가 카메라 뒤)→0.077
  - 13: 3.208→0.010
  - 14: 1.659→0.004
  - 15: 5.014→0.001
  - 16: 1.770→0.002
  - 17: 0.099→0.029
- **같은 컷 샷 경계(8→9, 14→15→16→17→18):** 카메라 위치 끊김 0.000m.
- **파서 규칙:** 좌표·시선 거리·up 조건을 통과했다.
- **파일 비교:** 적용 전후 JSON 비교에서 바뀐 샷은 계획한 13개뿐이다. CRLF만 쓰고 크기 1,633,119 bytes, `git diff --check` 출력 0.
- **남는 한계:**
  - 샷 8·9·10에서 쿠크는 기준이 아니라 화면 위치가 원본과 최대 0.71/0.74/0.49 어긋난다.
  - 카메라 거리가 늘어난다: 8 11.6→40m, 9 4.7→16.2m, 10 7.8→27.1m, 13 1.76→3.0m, 14 3.3→5.6m, 15·16 최대 4.2m, 16 끝~17 최대 46m. 배경·무대가 원본보다 작게 보이고, 소품이나 벽 가림은 미확인이다.
  - 샷 11(오차 0.15)·12(0.45, 대형 세이튼·쿠크는 이미 일치)와 샷 1~7(≤0.5)은 바꾸지 않았다.

### G07-4. 확인

- **반영 경로:** 1관문 G06-4와 같다. 미리보기 재생은 저작본을 바로 읽고, 제품 재생은 Area 게시가 필요하다(미실행).
- **볼 시각 (카메라 시간은 바뀌지 않았으므로 기존과 같다):**
  - 2관문_클리어(P5): 11.4초, 14.9초, 23.6초, 26.6초, 27.2초, 28.9초.
  - 3관문_진입(P7): 6.9초, 9.9초, 10.5초, 12.1초.
- 화면 판정은 아직 없다. 커밋·푸시는 하지 않았다.

### G07-5. 샷 17 커튼 진입 정정 (2026-09-15, 적용)

- **사용자 관찰(재녹화 `3관문 (다시).mp4` 비교, 스크린샷 P5 31496/32213/32929ms):** 원본은 카메라가 올라왔다가 커튼 사이로 들어가는데, 우리는 커튼 위쪽이 다 보인다.
- **원인(사실):**
  - 수정 전 녹화의 같은 시각 프레임에서는 원본처럼 무대를 내려다본 뒤 커튼 틈으로 향했다.
  - G07-2에서 샷 17에 넣은 거리 배율 ramp가 카메라를 뒤·위로 밀어냈다. 이 샷은 월드 구조물(커튼 틈)을 기준으로 움직이는 경로라 배우 기준 재조준이 맞지 않는다.
- **정정:**
  - `kouku.gate2.clear.camera.17`과 `kouku.gate3.intro.camera.17`의 keyframes를 revision 85 백업 텍스트에서 바이트 그대로 복원했다(원본 경로).
  - ramp는 샷 16 안으로 옮겼다(시작 1/k → 끝 1, smoothstep). `kouku.gate2.clear.camera.16`과 `kouku.gate3.intro.camera.16`이 대상이다.
  - revision 86→87. 스크립트는 `prepare_gate3_camera_fix_v2.py`(입력 = revision 85 원래 키)와 `apply_gate3_camera_fix_v2.py`, 백업은 `backup_camerashots_rev86.json`이다.
- **검증:**
  - v2 계산의 샷 8~15 새 키가 적용본(v1)과 완전히 같다. 계산 입력이 원래 키라는 확인이다.
  - 샷 16 머리 오차 1.77→0.152, 끝 거리 14.67m(원래 경로).
  - 경계 15→16·16→17·17→18 카메라 끊김 0.000m.
  - 바뀐 샷은 16·17(P5·P7) 4개이고, 샷 17은 revision 85와 JSON이 같다. CRLF만 쓰고 `git diff --check` 출력 0.
- **처음 v2 계산의 오류(기록):** 분석 라이브러리가 이미 수정된 현재 파일을 원래 키로 읽어, 이중 적용 결과가 나왔다. 백업 revision 85를 입력으로 바꿔 다시 계산했고, 잘못된 결과는 적용하지 않았다.
- 화면 판정은 아직 없다.

## G08. 빙고 최종 엔딩 카메라 패턴 복원 (2026-09-15, 적용)

- **사용자 요청:** 3관문 빙고 끝(`3관문 끝.mp4`)이 시퀀스 편집기 빙고 게이트에 없으니, 원작 카메라만 1·2·3관문처럼 넣는다(맵 애니메이션·모델 제외).
- **원인(사실):**
  - `dd93fe6e`(09-13)가 넣은 `KAKULSAYDON_G1_PATTERN_8` "빙고_최종엔딩씬"(BINGO, 카메라 12박스)이 병합 `400439b2`(09-14)에서 빠졌다. 부모2 rev 23에는 있고 병합 결과 rev 41에는 없다.
  - 그 뒤 PATTERN_8 번호는 "1관문_연출"이 쓴다.
  - 샷 `kouku.bingo.ending.camera.1~12`는 camerashots에 소비자 0으로 남아 있었다.
- **재검증 (camerashots 수정 없음):**
  - 원본 SCENE01B Matinee0 대비 eye ≤4.9mm, 전방 ≤0.306°, up/roll ≤0.295°, fovY 0.0°(16:9 환산, 다른 관문과 같은 기준).
  - `3관문 끝.mp4` 내부 컷 9개가 ±33ms로 일치한다(원본 슬로모 반영 대응).
- **적용:**
  - `Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json` rev 61→62(SHA `55B99E8A…`).
  - `KAKULSAYDON_G1_PATTERN_9` "빙고_최종엔딩씬": gate BINGO, target `boss.kakulsaydon.bingo.saydon`, stage 49,083ms, CAMERA 12박스(0/1232/2384/4933/9567/12967/18800/21800/25000/26875/30933/37000ms 시작, 샷 길이와 같음).
  - presentationResources 12행(`presentation.kouku.bingo.ending.camera.1~12`), nextPatternOrdinal 9→10.
  - WORLD·Logic·Summon·SceneProfile 없음, enterCombatOnFinish false. 폴더는 필요 없다(`KoukuSaydonActionWorkbench.cpp:4444-4455`).
- **검증(메인 세션 재확인):**
  - 기존 패턴 8개와 다른 최상위 항목이 백업과 같다.
  - 박스가 0~49,083ms를 빈틈 없이 채우고 각 박스 = 샷 마지막 키다.
  - CRLF만 쓰고 `git diff --check` 0. 포크의 파서 규칙 재현 통과.
- **백업·스크립트:** `out/KoukuFourCutscenes20260915/gate3end/bingo_pattern/`(`backup_sequence_composition_rev61.json`, `add_bingo_ending_pattern.py`).
- **확인 경로:** F1 → Action Workbench → Composition Actions → Sequence → Gate 빙고 → Reload Patterns → `빙고_최종엔딩씬` → Play. 미리보기는 저작 camerashots(rev 87)를 읽는다.
- **한계:** 배우·암전·조명·페이드·FX 없음. 원본 슬로모(+3.2초) 미반영. 빙고 보스 미스폰 상태의 Play 동작은 미확인. 화면 판정 없음. 커밋·푸시 안 함.

## G09-A. 2관문 진입(P3) 촛대·HandBook 적용과 WorldSequences 게시 (2026-09-15, 적용·게시, 화면 판정 전)

- **사용자 지시:** 7개 조건(현재 변경 보존·부위별 백업과 되돌림, 촛대 B_Ry180, 손책 41키, 카드는 로컬 후보까지, 의자 불변, WorldSequences 범위 Validate → Publish → Check, World Object Tool Save·전체 빌드 금지).
- **백업:** `out/KoukuFourCutscenes20260915/gate2/mapobject_apply/backup/`
  - `authoring_worldsequences_rev1848.json`, `runtime_worldsequences_rev1848.json`, `CardEruption_original.wmodel`(SHA `3865B111…`), `BACKUP_MANIFEST.json`.
- **적용 스크립트:** `gate2_mapobject_apply.py apply|revert candle|handbook [--write]`.
  - 템플릿 키 안의 `rotationQuaternion`/`positionOffset` 배열만 앵커로 바꾸고, 기대 문서와 JSON 전체 비교한다.
  - revert는 백업의 원래 줄을 바이트 그대로 되돌린다. 기록은 `APPLY_LOG.json`.
- **변경 (authoring `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json`):**
  - rev 1848 → 1849: `sequence.kouku.gate2.intro.candle1`·`candle2` 182키의 `rotationQuaternion`만(B_Ry180, w≥0 정규화).
  - rev 1849 → 1850: `sequence.kouku.gate2.intro.handbook` 4530~10079ms 41키의 `positionOffset`·`rotationQuaternion`(H3). SHA `A72DB4F2…`.
  - 키 시각·키 수·`scaleMultiplier`·`visible`·durationMs·다른 템플릿·`objectResources`·`instances`는 같다. 의자·카드 템플릿과 Composition은 바꾸지 않았다.
- **게시 (별도 기록 `out/KoukuFourCutscenes20260915/gate2/mapobject_apply/PUBLISH_LOG.md`):**
  - `Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Scope WorldSequences`의 Validate(349.8초)·Publish(297.4초)·Check(456.9초)가 모두 종료 코드 0.
  - runtime `Client/Bin/DataFiles/Map/LV_LUT_MIDNIGHTC_ED.worldsequences.json`은 rev 1850, SHA `DAF437B9…`이고 JSON이 authoring과 같다.
  - runtime 줄바꿈은 게시 도구 형식대로 LF다(게시 전 사본은 CRLF). 바뀐 runtime 파일은 이것 하나다.
- **자동 검증 (`VERIFY_RUNTIME.json`, `WorldSequencePlayer.cpp:488-531` 보간 재현, 10ms 간격):**
  - 촛대 기준 자세 대비 최대 회전 오차: 촛대1 1.686°(17,030ms), 촛대2 3.363°(17,780ms). 위치는 전 구간 수정 전과 같다.
  - 촛대 기울기(촛대1/촛대2, 수직 0°): 16.5초 95.7°/93.8°, 17.5초 48.0°/39.6°, 18.0초 4.0°/0.7°, 18.731초 0.2°/0.4°. 수정 전에는 같은 시각에 89~112°였다.
  - 촛대 끝 화면 좌표(NDC): 18.731초 (−0.35, 0.746)/(0.517, 0.516), 23.266초 (−0.738, 0.736)/(0.728, 0.727).
  - HandBook H3 대비 최대 오차: 위치 0.0133m, 회전 1.997°(둘 다 10,000ms). 구간 밖(0~4,529ms, 10,080ms~) 샘플은 수정 전과 같다.
  - HandBook 구간 안(4,530~10,079ms) 10ms 사이 최대 변화: 수정 후 0.0076m/0.624°, 수정 전 0.0035m/0.347°. 구간 안에 새 점프는 없다.
  - HandBook 경계 1ms 변화:
    - 진입 4,529→4,530ms: 수정 후 11.631m/43.98°, 수정 전 11.572m/97.42°.
    - 이탈 10,079→10,080ms: 수정 후 1.738m/31.44°, 수정 전 1.926m/98.05°.
    - 회전 점프는 줄었고, 위치 점프는 원래 있던 크기 그대로 남아 있다.
  - 이전 기록의 `maxStep10msInsideWindowM`(수정 후 1.733m)은 이탈 경계를 포함한 값이라 구간 안 값이 아니다. 위 0.0076m가 경계를 뺀 값이다.
  - 경계 시각에 켜진 P3 이펙트(`efseqact_matinee_2.1` "입장 · 도박 테이블"):
    - 4.53초에는 `fx_mn_rpct_03.par_d_rpct03_sk14_31`(4.08~5.64초) 한 계열.
    - 10.08초에는 `fx_q_w_01.fx_par_02.par_q_rpct_exp_02` 4개 등.
    - 이것들이 경계 점프를 가리는 연기인지는 확인하지 않았다.
  - 두 worldsequences 파일 `git diff --check` 출력 0.
- **미확정:**
  - 원본 엔진의 촛대 회전 조회 규칙(B 가설)은 입증되지 않았다.
  - 원작 촛대의 앞/뒷면, 18.3초 이전 오른쪽 촛대 모습.
  - HandBook 90.02° 차이의 원인.
  - 기존 위치 점프(4.53초·10.08초)가 원작처럼 가려져 안 보이는지.
- **확인 경로:**
  - 재생: F1 → Action Workbench → Composition Actions → Sequence → Gate 2관문 → `2관문_진입컷씬` → Play/Seek. 촛대·HandBook WORLD 박스는 모두 0ms 시작, 속도 1이라 박스 시각 = 컷씬 시각이다.
  - 키 편집: 같은 패턴의 WORLD 박스 `KAKULSAYDON_G1_PATTERN_3.world.5`(HandBook)·`.world.10`(Candle1)·`.world.11`(Candle2) → Box Detail `Edit This Motion` → World Object Tool "Selected Key".
- **재로드:** 적용·게시 때 Client는 꺼져 있었다. 다음 Client 실행부터 두 파일(rev 1850)을 읽는다.
- **되돌리기:** `python out/KoukuFourCutscenes20260915/gate2/mapobject_apply/gate2_mapobject_apply.py revert handbook --write`, `... revert candle --write`(부위별 독립). 그 뒤 같은 WorldSequences 게시를 다시 한다.
- **이후 툴 편집 Save 제한:**
  - 전투 Composition `worlds[]`가 세 인스턴스를 참조한다(`Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json:776/821/830`).
  - 그래서 World Object Tool에서 이 템플릿을 Save하면 WorldSequences 게시 뒤 `Publish_AllPatterns`가 자동 실행된다(`WorldObjectTool.cpp:623-639`, `MainApp.cpp:7476-7478`).
  - `Publish_AllPatterns`는 `Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon`(koukusaydon.product·map.kakulsaydon Area 전체·world.gameplay·gameplay.balance)이다. Area 전체 게시에는 아직 게시하지 않은 camerashots rev 87도 들어간다.
- 화면 판정 없음. 커밋·푸시 안 함.

## G10. 3관문 날아가는 장면(P5 2관문_클리어) 터널 방출 반복 복원 (2026-09-15, 적용, 화면 판정 전)

- **사용자 결정:** 원본 컷신 이펙트 복원 때 들어간 값이고 툴에서 직접 작업한 적이 없으니 원본대로 늘린다.
- **원인 (사실, `out/KoukuFourCutscenes20260915/gate3intro/tunnel/TUNNEL_REPORT.md`):**
  - 원작 `par_q_warpspace_01`·`par_q_movingtrail_01` Required 모듈에 `emitterloops`가 없다(UE3 기본 0 = 무한).
  - 우리 문서는 `emitterLoopCount` 1이라 방출이 1초로 끝났다.
  - glow·darkfield·field는 원작에 1이 명시돼 있어 바꾸지 않았다. 카메라 수정은 원인이 아니다(수정 전 녹화도 10.5초부터 검다).
- **변경:** `Data/Effects/Authored/effect.kouku.sequence.lv_lut_midnightc_ed_scene02a.efseqact_matinee_10.{1,2,3}.effect.json`의 `sourceRecipe.emitterLoopCount` 1 → 0.
  - 10.1 warpspace 12요소: SHA `1D1B1A98…` → `4B86E549…`.
  - 10.2 movingtrail 10요소: `4B3E75B8…` → `644064F0…`.
  - 10.3 movingtrail 10요소: `2017132A…` → `ACF3E5A2…`.
  - 합계 32줄이다. 스크립트가 JSON 전체 비교로 다른 필드가 같은지 확인했다.
- **런타임 방출 창 (`Effect_Playback.cpp:4321-4329`, 반복 0 → 요소 수명):** warpspace 8.084~8.716초·8.716~21.123초, movingtrail 10.793~17.655초.
- **스크립트·백업:** `out/KoukuFourCutscenes20260915/gate3intro/tunnel/apply/tunnel_emitter_loops.py apply|revert [--write]`, `backup/`, `APPLY_LOG.json`.
- **툴 확인 경로:**
  - 재생: F1 → Action Workbench → Composition Actions → Sequence → Gate 2관문 → `2관문_클리어` → Play, 8~21초.
  - 설정 보기: F1 → Effect Tool V1 → All Effects → `KoukuSaydon > 2관문 > 연출 > 쿠크 > 2관문_클리어` → "2관문_클리어 / 원본 이펙트 1/2/3" → 요소 → Original Emitter / Module Stack의 `Delay | Duration | Loops`(읽기 전용, Loops 0).
  - 이펙트는 Client 실행 때 읽으므로 재시작해야 한다.
- **하지 않은 것·미확정:**
  - `Validate-EffectSources.ps1` 미실행.
  - 반복 0에서의 파티클 밀도·프레임 비용 미측정.
  - 생성기 `Tools/EffectPipeline/build_kouku_sequence_effect_groups.py`를 다시 돌리면 1로 돌아간다(생성기 수정 안 함).
- 화면 판정 없음.

## G11. 2관문 진입 카드분출 모델 병합 원인과 로컬 후보 (2026-09-15, 후보까지, Resources 미교체)

- **재현:** 바탕 화면 원본 추출 입력으로 굽기 도구를 경로만 바꿔 돌리면 현재 Resources `CardEruption.wmodel`과 바이트가 같다(SHA `3865B111…`).
- **원인 단계 (사실):**
  - gltf 중간본은 카드마다 정점을 가진다(6,624/1,728/3,312, 카드 뼈 108개).
  - ModelAssetConverter 출력은 88/32/88 정점, 카드 뼈 3개다. 위치·법선·UV가 같으면 뼈 가중치와 무관하게 합쳐진다(가중치 섞인 정점 16).
  - 시간 보정 단계는 정점을 바꾸지 않는다.
  - 기전이 Assimp 계열 동일 정점 병합이라는 것은 추정이다(변환기 소스 없음).
- **수정 (사본):** gltf에 카드별 `COLOR_0`를 추가했다. WModel 스킨 정점(76바이트)에는 색이 저장되지 않는다(형식 플래그 0x1f·stride 76 동일). 원본 도구용 diff는 `out/KoukuFourCutscenes20260915/gate2/cards_candidate/tools_fix.diff`이고 `Tools` 원본은 수정 안 했다.
- **후보:** `out/KoukuFourCutscenes20260915/gate2/cards_candidate/CardEruption.wmodel`(3,365,164 bytes, SHA `977F01E7…`).
  - 재질·뼈대·애니메이션 섹션은 바이트가 같다.
  - 정점 11,232(카드 108개 각 104), 삼각형 수는 같다. 엔진 로더 규칙 재현을 통과했다.
- **예측 (런타임 방식 계산, 가림 미반영):** P3 화면 안 카드 수(현재 모델 대비)는 22.0초 96(12), 22.4초 145(12), 23.3초 205(12), 23.7초 232(12), 24.8초 59(0)장이다.
- **보존:** 카드 수·방출·수명·시드·재질·P3 world.12~17 시작 21,040ms·길이 5,920ms는 그대로다. 밀도 변경·Effect 교체는 없다.
- **교체 (사용자 승인 "일단 교체먼저"):** `Client/Bin/Resources/Map/KakulSaydon/Gate2Intro/CardEruption/CardEruption.wmodel`을 후보로 바꿨다.
  - 해시: SHA `3865B111…` → `977F01E7…`, 3,365,164 bytes.
  - 교체 때 Client.exe·Server.exe가 실행 중이었다. 그래서 같은 폴더 임시 파일에 복사하고 해시를 확인한 뒤 이름 바꾸기로 한 번에 덮어썼다.
  - 실행 중인 Client가 이미 읽은 모델에는 반영되지 않을 수 있다. 재진입으로 다시 읽는지는 확인하지 않았다.
  - 되돌리기: `cards_candidate/backup_current/CardEruption.wmodel`(또는 `mapobject_apply/backup/CardEruption_original.wmodel`)을 같은 경로에 복사한다.
  - Drive 배포는 사용자 화면 확인 뒤에 한다.
- **미확정:** Client 화면 일치, 프레임 비용, 실행 중 교체 시 반영 시점.
- **원인 상세 (포크 G, `out/KoukuFourCutscenes20260915/gate2/cards_candidate/ROOT_CAUSE_REPORT.md`):**
  - `ModelAssetConverter.exe`는 assimp 6.0.4이고 읽기 플래그 `0x0180022F`로 `JoinIdenticalVertices`를 항상 켠다. 끄는 옵션은 없다.
  - assimp 병합은 위치·법선·UV·정점색만 비교하고 뼈·가중치·역바인드는 보지 않는다. 합쳐진 정점은 처음 나온 뼈만 남는다(assimp `JoinVerticesProcess.cpp:103-143`, `:338-349`). 전부 합쳐진 뼈에는 옛 가중치가 남는다(`:354-361`, 테두리 정점 16개가 가중치 합 2.0).
  - 굽기 도구가 108장을 같은 원본 메쉬 좌표와 단위 역바인드로 넣고 움직임은 뼈 애니메이션에만 둔다(`build_gate2_card_eruption.py:81`, `:140`).
  - 통제 실험 9개로 규칙을 확정했다. 형상이 같으면 뼈·역바인드가 달라도 합쳐지고, `COLOR_0`가 다르면 합쳐지지 않는다.
  - 09-11~09-14 기록의 검증은 로드·클립·키 시각·뼈 궤적까지였고, 뼈별 정점 수·화면 카드 수 확인은 없었다.
  - Resources skinned 모델 559개에서 옛 가중치 흔적은 0개다. 흔적 없는 병합 여부는 판정하지 못했다.
- **굽기 도구 수정 (Git 추적, 커밋 안 함):** `Tools/KoukuSaydonPipeline/build_gate2_card_eruption.py`에 카드별 `COLOR_0` 3곳(4줄 추가·2줄 삭제)을 넣었다.
  - 수정 도구로 구운 결과가 교체 모델(`977F01E7…`)과 바이트가 같다. `git diff --check` 0.
  - 이 PC에서는 한글 경로 변환기 실패와 입력 폴더 `out/KoukuGate2Restore20260911` 부재로 그대로 실행되지 않는다(우회 코드 미반영).
  - 원본 백업: `cards_candidate/tool_fix_apply/backup/build_gate2_card_eruption.py`.
