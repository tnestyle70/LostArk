# 쿠크 전투 진입·피격·패턴 정리 결과

## G00. 반영 범위

기존 `codex/kouku-timeline-local-preview` 저장본과 미커밋 변경을 유지했다. 이번 저작 반영은
Composition2253→2254, WorldSequences2213→2214, RenderingProfiles80→81 및 Effect10개다.
기존 ALT_V35줄, 공통 Trigger507/508, P47 source-in4201ms, Play/Play Pattern 분리를 보존한다.
사용자가 언급한 칼날은 Mario2의 **즉사칼날1개**이며 일반 칼날 수를1개로 바꾸지 않았다.

| 요청 | 실제 반영 |
|---|---|
| 연출 종료→시점/HUD/전투 | PREPARING에서 만든 조명과 profile을 연출 시작부터 baseline으로 사용. 완료 시 같은 환경을 인계한 뒤 HUD/입력을 함께 해제. 실패/연결 종료는 이전 환경 복원 |
| 3관문 파란 색 간섭 | G3 LUT01 필드4개 해제. 실제 light/fog/exposure 유지. 최종 합성 LUT이므로 배경에도 같은 색 보정 해제가 적용됨 |
| 띄우기·넉백 후 넘어짐 | Server 실제 공중 flag→착지→DOWN 연결. 지연 Idle/stance가 반응을 덮지 않음. 착지 후 최소1초 회복, 더 긴 authored down 유지 |
| 세이튼 클릭 후 이동 지속 | 몸속 목적지에 도달할 수 없어 접선 이동을 반복하던 원인 수정. 해당 목적지를 막은 몸체에 닿으면 이동 종료 |
| 플레이어/세이튼 몸체 보기 | Debug/Release F1 Load KoukuSaydon Inventory에서 켜짐. Player / Boss Body Colliders checkbox로 해제 가능 |
| 흰 잔상/뿌연 파란 장판 | 실제 Npc MODEL 잔상 brightness/alpha와 counter 값을 조정. safezone native RGB 보존·alpha0.5배, additive 경계 유지 |
| 노란 십자와 후속 폭발 피해 | P79/119/122 기존 판정을 실제 floor cross 위치/크기·폭발 시점으로 정렬. 두 십자 BOX는 같은 Trigger창 공유. 후속 원/링은 저작 MAP 위치로 정렬 |
| 바람방구180도 | P100/116/113 Collider yaw와 localXZ offset을 함께 반전. BOX의 회전만 바꿔 같은 도형으로 남는 문제 방지 |
| 빙고 진입/재생 | 기존 Server 준비·입장·반복 flow 경로 유지. P96 초기/반복 구간에 레이저를 연결하고 duration/지속 logic을 함께 연장 |
| 카드 병정 추적/공격 | 공유 미로 profile의0.01m 이동/추적/공격 값을 일반 병정 전투 값으로 교정. 미로 등록 entity는 첫 tick부터 일반 Brain에서 제외 |
| 카드비 크기 | Server uniform scale1~4. 같은 발사의 카드·노란 warning·피해를 함께 확대. 개수는 기존500ms 간격 유지 |
| 회전 카드 보라 잔상 | 실제 heart/clover/diamond/spade4개 asset에 원본 SpawnPerUnit 잔상 emitter 복구 |
| 공분열5회/착탄 피해 | ball.launch6개 중 가장 늦은6번째 제거. 기존3개 착탄에서만 피해, 반경1m/2.5~4m/5.5~7m의 빈 안전 구간 유지 |
| 비둘기 폭발 피해 |9개 FX 각각 비행3.05초 뒤4개 실제 폭발 중심에 radius2.35m. FX당4개 원형이1개 Trigger창을 공유 |
| G3/빙고 레이저 | P62/P123 발사2050~3050ms에 양눈별 BOX를 같은 Trigger창으로 연결. G3 flow와 빙고 초기·반복 순서에 추가 |
| 망치·미로 이동 문양 | 망치 문양을 모델 등장3000ms부터 실제 이동4400ms까지 유지. 미로는2초 hidden hold+문양 뒤 이동. 네 방향 문양을 진행 방향에 맞춤 |
| 무력화 성공 시 칼날 해제 | STAGGER 성공에 명시적 interruption 결과를 연결하여 해당 owner 일반·즉사 칼날/예약 판정을 즉시 해제. 다른 owner와 후속 groggy 보존 |
| Mario2 즉사칼날1개 | 실제 참조되는 Mario 전용 template의 count8/emission8→1/1. 보스에서 아이언메이든까지 기존 motion/scale/수명 보존 |

## G01. 판정·표현의 근거

새 Collider는 `트리거_대미지`508과 기존 maxHP10% Result98을 사용한다. 잡기 Trigger를
복사하지 않는다. 기존 후속 원형 폭발의 Result150과 hollow ring은 보존한다. 십자는 native
floor square의 실제 WModel vertices·preScale·particle World를 대조했다. 폭발 전체 debris
AABB나 수직 광선의 크기를 바닥 피해 크기로 사용하지 않는다.

비둘기는 Sk02_3 ring의 first3041.67ms, localX=-2.4/-1.6/-.8/0와 Z=-10.1859를 사용한다.
radius2.35m는 최대 ring 직경4.67778m의 반올림이다. 레이저는 설치 MN_RPCT_05 모델과
실제 clip, `Sample_SourceAnchorWorlds`, `Make_ParticleSpriteWorld`로 양눈 beam29를
측정했다. P123 기존 pitch/roll을 보존한 각 눈별 XZ OBB이며 기울기를 다시 곱하지 않는다.
검은 plane4개에는 기존 `followEmitterAxisRotation` opt-in만 복구했다.

이 치수는 PROJECT_AUTHORED gameplay 크기다. GPU texture alpha coverage 또는 원본
SkillEffect 피해치수를 복원했다고 주장하지 않는다. 실제 사용자 화면 확인과 분리한다.

Mario 최종 XZ 이동 delta는[-13.03000054,-16.16000061]이며 현재 보스→아이언메이든
저작 위치 차이와1mm 미만으로 일치한다. 기본 즉사칼날 template과 Mario 전용 template이
별도였기 때문에 기본 count1만 확인해서는8개 재생을 막을 수 없었다.

## G02. 컴파일·집중 검증

VS18 Insiders amd64 MSBuild, v143 HostX64의 정상 증분 Build를 사용했다. Shared protocol은
110이며 양쪽 snapshot 생산자·codec·Client 소비자를 함께 수정했다. Release 몸체 표시를
위해 기존 Component/Collider/Bounding/Renderer/GameInstance ABI를 공통화하고
Engine/SDK→Client 순서로 빌드했다. 새 제품 C++ 파일/프로젝트 등록과 HLSL 변경은 없다.

- Shared, Engine, Client, Server x64 Debug/Release 최종 Build PASS.
- Engine Release 첫 시도는 Component virtual guard 누락을 발견해 수정한 뒤 성공했다.
- 기존 C4819/C4828 및 외부 라이브러리 PDB 관련 LNK4099 경고는 남아 있다.
- 실제 Server overlap/피격/몸체 목적지 종료 검사 **673 PASS,0FAIL**.
- 실제 Character 반응 branch를 추출한 CPU fixture10 PASS. 공중 마지막 pose, 실제 착지 edge,
  DOWN, 재발사, 지연 Idle, 기상 경계를 검사했다. GPU/전체 모델 재생 검사는 아니다.
- 병정 실제 추적·이동·공격:3마리 공격,18피해 event,HP50000→49388. 미로 첫 tick 소유권은1~4인 PASS.
- 카드4종 움직임 잔상 최대4개/정지0개, particle20470개 finite, blue RGB 보존·alpha0.5배 PASS.
- 레이저 실제 본/owner yaw 대조24개 PASS, 최대 방향오차4.96e-7. 새 Resources 없음, 기존90개 확인.
- World native36미로의1999ms hidden/2000ms visible/2033ms 이동 PASS.4망치+4미로 문양
  floorY0.03·alpha·진행 방향 PASS. publisher36경로/legacy2키/잘못된 hold5종 거부 PASS.
- 환경 lease native검사: 준비 없는 시작 거부, 반복 begin, 완료 인계, abort 복원,
  G1 borrowed owner 재개, 시작 실패 rollback PASS.
- support-surface 광역 fixture는180초 제한 전272 PASS/0FAIL, 전체 완료로 기록하지 않는다.
  이번 Mario 무력화 성공 일반/즉사 칼날 종료 assertion은 그 안에서 PASS했다.
- 기존 CardMaze Q/clown-box 및 solo telescope 기대값 실패2개는 수정 전 설치 baseline에서도
  같은 데이터로 재현했다. 이번 변경의 병정/미로 소유권 신규 항목은 통과했다.

게시 후 검증에서는 실제 미로59/60tick 경계4항목이 통과했다. CardMaze는91 PASS/기존Q·telescope2FAIL, Bingo는68 PASS/기존 hook2FAIL이다. Hook2개도 이전 작업의 Server EXE에 같은 최종 데이터를 넣어 동일 재현했다. 해당 fixture는 navigation=null을 넘기지만 기존 Runtime은 검증된 floor가 없으면 release를 연장하므로 기대값이 맞지 않는다.

레이드 fixture의 이전 P96 child38/loop24/수집52개 고정 기대값은 새 레이저가 추가된 실제 계약에 맞게 갱신했다. stable loop ID로 초기25개·반복15개를 구분하고 각 구간의 레이저1개, 초기 구간과 두 번의 반복에서 총55개 실제 재생 순서를 검사한다. 해당 fixture까지 포함한 Server Debug/Release 정상 증분 Build 후 최종 게시 데이터를 사용하는 전체 레이드 검사는 **1260 PASS/0FAIL,exit0,121.152초**로 완료됐다. 검사 전후6개 데이터 hash는 일치한다. 이전 실패 로그와 실행 파일별 hash도 receipt에 보존했다.

## G03. 데이터 교체·게시

최신 저장본을 다시 읽고 stable ID·변경 필드로 병합했다. 동시 변경 보존/같은 필드 충돌
거부를 검사했으며 원본 byte의 무관한 formatting도 유지했다. writer lock, 교체 직전 hash,
원자 ReplaceFileW와 displaced backup·실패 시 자기 변경 rollback을 사용해13문서를 설치했다.
백업은 `out/transactions/kouku-raid-996453773b764ee39221553e60bc5095`다.

RenderingProfiles 공식 Publish는 PASS. 첫 Kouku owner 게시 자체는193050ms에 성공했지만,
실제 Server가 P96 Parent child 행 순서를 거부했다. Product의 시간순 배열을 최종 bootstrap
natural-ID sort가 다시 바꾸는 결함이었다. 공통 Get-BootstrapRowSortKey에 Parent 시간순을
적용하고329행×3shuffle 회귀검사와 기존 dependency/byte 보존 검사를 통과했다.

source revision2254를 고정한 공식 owner 재게시가 **49846ms,exit0**으로 완료됐다.
product/map/world는 같은 fingerprint를 재사용하고 gameplay를 재생성했다. 최종 실제
bootstrap의5 Parent owner가 시간순·비중첩이며 native catalog admission도 성공했다.
Rendering runtime revision81, World2214, Encounter2254다.

13개 설치 원본과 검토한 후보의 의미 일치, 변경 JSON19개 parse, 기존 P17/P47 및
ALT_V35줄 보존을 검사해 PASS했다. `final-data-verification.json`에 hash를 기록했다.

## G04. 남은 화면 확인 경계

Client/Server UI를 자동 실행·종료하거나 화면을 대신 판정하지 않았다. 연출 환경의 중복
복원과 뒤늦은 적용 경로는 제거했고 기존 object clone prewarm도 확인했지만, 실제 프레임
시간/GPU 표시를 측정한 것은 아니다. 입력 해제 시 HUD, 넘어짐 clip, 색과 투명도, 문양,
레이저·비둘기 판정의 체감 크기는 사용자가 새 실행 파일로 확인해야 한다.

프로토콜110이므로 실제 테스트 참가 Client와 Server를 함께 갱신해야 한다. 실행 파일은
`Client/Bin/{Debug,Release}/Client.exe`, `Server/Bin/{Debug,Release}/Server.exe`다.
F1 Load KoukuSaydon Inventory 뒤 몸체 checkbox와 각 관문 Complete Play/게시 패턴으로
검사할 수 있다. 게시 파일 교체를 이미 실행 중인 Server 메모리 자동 갱신으로 설명하지 않는다.

증거는 `out/KoukuRaidPolish20260924`의 빌드·게시 로그, `combat-note.md`, `effect-note.md`,
`entry-light-note.md`, `effects/beam-eye-collider-geometry.json`, `applied-data.receipt.json`에 있다.
