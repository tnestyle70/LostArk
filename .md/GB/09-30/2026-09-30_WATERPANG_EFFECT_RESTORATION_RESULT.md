# 워터팡 효과·AI 리소스 정리와 통합 인계

## G00. 최종 상태와 작업 중단 범위

사용자가 다른 통합 세션에서 merge/build를 진행한다고 지시하여 이 세션의 추가 빌드와
게임 실행 검증을 중단했다. 타 세션의 MSBuild는 종료하지 않았으며 별도 채팅도 보내지 않았다.
핵심 코드는 현재 작업 트리에 있고 이펙트·저작 이름·native shader 소스·NPC 리소스는 설치했다.
통합 Product Build, 실제 경기 실행, Client 화면 검증은 완료로 기록하지 않는다.
이 작업은 commit/push하지 않았다. 기존 다수 미커밋 변경을 보존했다.

현재 기능 브랜치는 `codex/valtan-authoring-and-entry-20260930`이다.
통합 세션은 tracked diff뿐 아니라 아래 신규 파일도 포함해야 한다.

- `Client/Public/MaharakaAITool.h`, `Client/Private/MaharakaAITool.cpp`
- `Client/Private/ClientReplication_WaterGun.cpp`
- `Server/Private/GameRoom_MaharakaAI.cpp`
- `Server/Private/ServerGameplayContractTests_MaharakaAI.cpp`
- `Data/AI/MaharakaWaterpangAI.json`
- `Data/Effects/Authored/effect.maharaka.watergun.{q.flight,q.hit,w.flight,w.hit,r.flight,r.hit,shot.start,e.speed}.effect.json`
- `Tools/EffectPipeline/build_maharaka_watergun_projectile_effects.py`
- `Tools/EffectPipeline/build_maharaka_waterpang_smoke_candidates.py`
- `Tools/EffectPipeline/organize_maharaka_waterpang_library.py`

새 CPP/H/JSON의 Client·Server 프로젝트와 filter 등록은 반영했다.
그 외 Shared wire, GameRoom, CombatObjectRuntime, ClientReplication/registry,
Character preview setter, All Effects/World preview, MainApp, network sink/manager,
NPC clip 추가 도구의 변경도 같은 기능에 포함된다. `out`의 추출 원본·하네스·중간 산출물,
EXE/CSO/PDB와 `Client/Bin/Resources` 바이너리는 소스 커밋에 포함하지 않는다.

## G01. GBResources 전달 완료

전달 위치는 `C:/Users/user/Desktop/GBResources`다. `Client/Bin/Resources` 상대 경로를
그대로 유지했다. 이 기능의 전달 파일은 이펙트149개와 AI 의존477개, 합계626개
828,013,664 bytes다. 이번 복사 시 기존 충돌·교체는 없었고 모두 설치본과 SHA256이 일치했다.
폴더 전체의 파일 수가 아니라 이번 기능에서 추가한 전달분이다.

| 범위 | 파일 수 | bytes | 내용 |
|---|---:|---:|---|
| 이펙트 |149|5,910,744|23개 효과가 참조하는 메시·텍스처 등|
| AI 외형·동작 |477|822,102,920|NPC8종, 아바타12종, Guardian/Lance body·parts·기본 무기·물총·AnimSet 의존|

이미 Client에 존재하던 재사용 파일도 GBResources에 없으면 포함했다. 효과 native program의
새 DDS 생성이 필요하지 않았다는 사실과 전달 파일 추가는 별개다.
JSON과 shader 소스/CSO는 GBResources에 넣지 않았다.

증거:

- `out/WaterpangEffects20260930/effect-resource-delivery.json`
- `out/WaterpangEffects20260930/ai-resources.json`
- `out/WaterpangEffects20260930/ai-resources-delivery.json`

## G02. 효과 정본·게시 상태

기존 중앙 장치11개·총구4개와 신규8개를 총23개로 정리했다.
All Effects의 `World → 마하라카 → 워터팡` 아래 중앙 기둥·바닥, Q·W·E·R 물총,
원본 MK2 변형으로 분류한다. `워터팡 | 중앙 기둥 물 뿜기`, `워터팡 | 중앙 바닥 장판`,
`워터팡 | 중앙 바닥 터지기`, Q/W/E/R의 총구·비행·피격 이름을 실제 root displayName,
EffectResourceTree, DIRECT_AUTHORED_DOCUMENT catalog에 반영했다.

신규 물총8문서에는 원본57 emitter가 있다. Q/R 비행·피격, W 비행·폭발,
Q/R launch splash, E의 buff569200 원본 SpdFast를 연결했다. 기존4문서에 빠져 있던
중앙 장치 연기5개도 추가했다. MK2 총구2개는 원본 변형으로 보존하고 W에 오연결하지 않는다.

native 신규26개(program5273~5298)와 연기3633을 기존 material table·shader group에
추가했고 기존4개는 재사용한다. 다른 shader program과 렌더링 옵션은 보존했다.
후보와 최신 입력을 비교하고 백업·원자 교체로 설치했다.

WorldSequence의 공격 template5개 이름만 바꾸고 나머지 tracks·배치·재질·카메라를 보존했다.
`Publish-MapAuthoring.ps1 -AreaId LV_OCN_EVENTIS_MHP -Scope WorldSequences -Mode Publish`
완료, Client 런타임1파일 게시 SHA256은
`cffe044f35e5b72d343fbeca99ff4906d624ac17ffe47ba93930e96f5092bd2f`다.
Data/Effects는 기존 direct authored 소비 경로다.

중앙 효과 preview는 실제 WorldSequence object/model/bone을 사용한다.
물총 투사체는 기존 Server CombatObject의 spawn/current pose/impact/despawn으로 재생한다.
Q/R 발사 offset은 원본 UE(70,11,75)cm를 forward/right/up으로 한 번 적용하고
source+X→gameplay+Z는 effect particleSystem yawOffset -90도로 한 번 적용한다.
W의 비행 높이는 원본60~90cm·apex ratio0.4를 참조한 bounded parabola로 재구성한 값이며
원본 엔진의 전체 grenade 이동 구현까지 동일하다고 기록하지 않는다.

## G03. AI·3분 경기·AI Tool 구현 상태

protocol127에 `WATERPANG_AI(2)`, immutable NPC 외형 ID 및 AI tuning request/result를
함께 연결했다. 인간4인 roster와 별도인 기본20명은 Guardian/Lance 아바타12명과
Maharaka/Bern NPC 외형8명을 사용한다. fake session을 만들지 않고 기존 Server player의
이동·Q/W/E/R·피격·낙사·복귀를 사용한다. 인원 증가 stage 실패는 기존 AI를 보존한다.

도입 시작20초 뒤부터 실제 경기180초를 계산한다. 종료 시 STOP을 보내고 AI·물총을 정리하며
참가 인간은 같은 섬의 `island.spawn.party02` 주변 navigation/collision 검증 위치로 이동한다.
이미 탐험 중인 인간은 보존한다. Client 남은시간·STOP 소비도 구현했다.
넉백 기본6m/242ms는 사용자가 요청한 쿠크 레이저 기준이며 원작 워터팡의 작은 밀림값과 다르다.

Debug F1 `Waterpang AI Tool`에는 봇 수, 판단 tick, 이동 목표 갱신 tick, 스킬 간격 tick,
대상 거리, 이동·공격 확률, 넉백 거리·시간9개 값이 있다. Refresh/APPLY/Save+Apply는
typed command sink를 사용한다. Server가 revision CAS 및 범위를 검사하고 정본
`Data/AI/MaharakaWaterpangAI.json`을 원자 저장한다. 실패·충돌·timeout 시 draft를 보존한다.
이 동작은 코드 연결 상태이며 실제 UI/왕복 실행 검증 완료는 아니다.

## G04. 주민 애니메이션 리소스

주민4종의 WModel에 원본 `run_battle_1`, `walk_normal_1`을 추가해 설치했다.
기존 mesh/material/skeleton/idle section은 모두 byte 동일하다.
설치 WModel의 UModel-glTF basis를 기존 idle의124bone 전체 키와 비교해 확인한 뒤
`Tools/ModelAssetConverter/append_psa_clip_to_wmodel.py`의 명시 profile로 변환했다.
기존 position186,744개 오차0, quaternion186,744개 최대오차2.98e-7이며,
추가 position34,968개·quaternion34,968개는 원본 변환값과 오차0이다.

`out/WaterpangEffects20260930/npc-locomotion/installation.json`과
`run-candidate-manifest.json`에 변경 전후 hash·clip 근거를 보존했다.
이4개 수정 모델도 GBResources 전달477개에 포함했다.

## G05. 실행한 검증과 남은 경계

완료한 검증:

- source 문서8개/57 emitter의 구조 감사, 연기5개 원본 coverage와 native deferred0 확인.
- 신규 shader가 소비되는 Mesh5248/Particle5248/Particle3584/Decal/Trail 후보5개 fxc 컴파일 성공.
- Client Debug10CPP·Release11CPP 문법 검사 오류0. 이후 소규모 NPC guard/preview setter는 추가 컴파일하지 않음.
- Server 첫8TU와 투사체2TU 문법 검사 성공. 별도68TU 격리 컴파일·링크 성공.
- 신규 JSON 및 Client/Server 프로젝트4개 XML parse 성공, 변경 파일 diff whitespace 검사.
- WorldSequences scope publish 성공, GBResources626개 전달 및 해시 확인.

미완료·통합 세션 확인 항목:

1. protocol127 Client/Server의 최종 Product Build. 이 세션은 사용자 지시에 따라 추가 빌드를 중단했다.
2. 실제 AI20명 이동·스킬·넉백·낙사·3분 종료·재입장, AI Tool 왕복/저장/충돌 처리.
   Server contract 및 AI Tool headless 실행은 중지했으므로 통과로 기록하지 않는다.
3. All Effects와 실제 경기 화면 확인. Client/UI를 에이전트가 실행하거나 캡처하지 않았다.
4. program5275의 Q/W/R flight bubble3곳에는 원본 VS의 sin-wave vertex 변형이 아직 없다.
   native PS/material과 source geometry 복원, deferred0을 완전한 VS 복원으로 대신하지 않는다.
5. Beda 원본 AnimSet은 idle3개뿐이다. 다른 NPC도 전용 물총 발사 자세가 없는 경우
   현재 존재하는 동작으로 이동하며 발사는 Server 투사체로 표현한다. 임의 player rig/socket을 이식하지 않았다.

백업은 `out/WaterpangEffects20260930/restoration-install-backup`, `library/backup`,
`npc-locomotion/install-backup`에 있다. 이전 문서 전체로 롤백하지 말고 최신 저장본과
변경 필드를 비교해야 다른 세션의 편집을 보존할 수 있다.
