# 발탄 체력 기믹·반복 순서와 부채꼴 회전 결과

## G00. 적용된 정본

기존 디스크 저장본에서 stable ID/필드 69개를 병합했다. writer admission 아래 최신 source manifest와
각 파일 bytes를 재확인하고 백업·원자 교체·자기 변경 rollback을 사용했다.
근거는 `out/ValtanHealthFlow20260929/source-install-receipt.json`과 같은 폴더의 `backup/`이다.
사용자 저장 이펙트의 위치·크기·시작 시간, Play All의 70개 참조 순서와 전환 지연을 보존했다.
이번 작업은 Resources 파일을 추가하지 않는다. 앞선 GBResources 전달은 유지된다.

## G01. 자동 전투와 시연

`HEALTH_BAR_ROTATIONS` 자동 모드가 8개의 `ORDERED_LOOP`와 7개의 체력 기믹을 소비한다.

| 구간 | 반복 패턴 |
|---|---|
| 160→130, 130→115 | 휠윈드 → 3회 구르기 후 돌진 → 추적도끼 → 4연속 → 십자돌 → 3회 구르기 후 돌진 |
| 115→105 | 침묵 → 4연속 → 속박 → 휠윈드 → 마력구 → 십자돌 → 돌진 → 추적도끼 |
| 105→80 | 침묵 → 4연속 → 속박 → 2페이즈4방향 → 휠윈드 → 마력구 → 십자돌 → 돌진 → 추적도끼 |
| 80→65 | 뒤잡기 → 침묵 → 2페이즈4방향 → 4연속 → 속박 → 휠윈드 → 마력구 → 십자돌 → 돌진 → 워프 |
| 65→30 | 뒤잡기 → 침묵 → 4연속 → 2페이즈4방향 → 3연속내려치기카운터 → 휠윈드 → 마력구 → 십자돌 → 돌진 → 워프 |
| 30→15 | 뒤잡기 → 침묵 → 2페이즈4방향 → 4연속 → 3연속내려치기카운터 → 휠윈드 → 마력구 → 십자돌 → 돌진 → 워프 |
| 유령40→0 | 4연속 → 뒤잡기 → 휠윈드 → 십자돌 → 2페이즈4방향 |

체력 기믹은 130 전멸, 115 외곽 파괴, 105 피자, 80 3시 파괴, 65 버러지, 30 9시 파괴,
15 발악이다. 15의 기존 death/respawn 후속 경로로 유령40줄에 진입한다. 큰 피해로 여러 문턱을
넘어도 기존 HP queue/once ledger로 높은 줄부터 처리한다. 옛 자동 기믹 5개는 sealed Product에서
AUDITION_ONLY로 옮겨 중복 자동 발동을 막았다. `FIST_IN_OUT`은 derived 수동 시연으로 보존한다.
기존 stable ID의 109/106과 stage 이름은 변경하지 않았다.

Play All은 저장된 scriptedSequence를 명시적으로 실행하는 기존 override다. source/Client/Server
reader와 Save가 두 mode를 구분하고 현재 mode를 유지한다. Balance Tool은 ordered members를
순번대로 표시하고 지원하지 않는 순서 변경은 저장에서 거부한다. 무관한 이펙트·타이밍 Save를
막던 구형 mode guard를 수정했다.

## G02. 80줄 중앙 이동과 최신 저장 길이

terrain3의 COMBO_STEP_23/24/18/08/16/17/09/10/11은 이미 TO_ARENA_CENTER를 사용한다.
Server Try_GetMovementProposal이 root motion보다 먼저 이 경로를 처리하고 navigation이 이동을
소비한다. 휠윈드 COMBO_STEP_16/17도 포함된다. 기존 ownerHitChain의 내부 돌 우선 파괴를 유지했다.

최신 저장본의 COMBO_STEP_11은 3시2320ms, 9시2523ms였다. 이전 Product/root-motion의9670/7870ms를
저장본에 맞췄다. 원래2130ms까지의 모든 움직임 sample과 hold endpoint를 그대로 보존했다.
`out/ValtanHealthFlow20260929/rootmotion-verification.json`에 두 track 검사 결과가 있다.

## G03. 바훈투르와 130줄 무적 표시

사용자가 다른 담당자의 기존 구현 유지를 요청하여 이번 full-immunity 추가만 제거했다.
기존 50% 피해 감소·전멸 공격 차단·30초 지속·부여 범위는 유지한다.
마력구 전용 damage profile 분리에 맞춰 기존 전멸 차단 목록에 같은 공격의 새 ID를 연결했다.

130줄 발탄은 기존 pattern invulnerability를 유지하며 blocked hit를 INVINCIBLE/0damage로 보낸다.
Shared codec은 무적 이벤트에 damage/stagger/기믹 credit이 섞이는 것을 거부한다. HUD는 파란 `무적`을
표시하되 DPS/MVP에 합산하지 않는다. source/public packet에 새 enum이나 field는 추가하지 않았다.

## G04. 부채꼴 최종 면의 회전

stage004 팬3개의 EPAL_Z sprite는 최종 quad에서 emitter 회전을 버려 중심만 움직였다.
이3개에만 `followEmitterAxisRotation=true`와 socket rotation `[0,90,-90]`을 적용했다.
B_Root source 부착, 다른 weapon 요소10개, 저장 scale/position/timing은 보존했다.
피자·지형파괴3시·9시가 같은 asset을 소비한다.

실제 설치 모델·Codec·Playback·renderer Make_ParticleSpriteWorld로13,381검사를 수행했다.
static432/dynamic726 quad, 비원점3위치·yaw4개·model scale2개를 포함한다.
최대 pivot matrix 오차55.3900→1.53e-5, 전방 오차3.94e-5도, 수평법선 오차1.14e-5도다.
`out/ValtanEffectAnchor20260929/verification.json`에 기록했다. GPU draw/사용자 화면 검증은 아니다.

## G05. 게시·빌드 상태

최종 Debug Product Engine/Shared/Server/Client build PASS.
`out/BuildPipeline/runs/20260928T183417428Z-debug-product.json`에서 확인한다.
기존 C4819/C4828/LNK4099 warning은 남아 있다. Client/UI 자동 실행은 하지 않았다.
최종 Gameplay Publish와 Composition Publish, Save & Publish의 PublishCandidate 모두 PASS다.
Gameplay bootstrap은109157행/32220182bytes이며 SHA256은 `b8d1c142dfdbb191330c67e586edf51050e6537ef568b09d7239dfa2d1215e66`다.
Composition source manifest는 `bb30e55064821615391c4e0476a591ee95c7ddb6208dc28b7509d58d52c421f2`다.
Candidate revision은 `57632234e5534272768b9e0591508829e767f32bff76f7eeb9b841636c55f196`이고, source manifest는 `ed17f1cfd4bd2bc03eda8ef0a3b3001ea805574babf4bf3da03cb42650df28ad`다.
설치된 bootstrap과 candidate의 presentation generation은 `36bdc263cdc3f5f07351a4515a8dda1ff5daafd63aa64308a5b738bae085af66`로 같고
175개 artifact의 내용과 manifest를 검증했다. projected Product9개도 source와 일치한다.
Gameplay Publish가 실제 실행 파일 데이터를 설치했고 PublishCandidate는 저장본의 불변 후보를 만들었다.
이 둘의 완료를 실행 중 Server 또는 Client 메모리 Reload와 동일하게 설명하지 않는다.

최종 보존 검증은 out/ValtanHealthFlow20260929/final-verification.json과 followup-verification.json이다.
기존69필드와 추가27필드 및 워프 후속10필드, Play All70개 및 사용자 저장 돌진 hash `456d1b3911d9541e919c148969c23e29ad01936f8e7e033e7c966fb7eae2cf76`를 확인했다.
GBResources의 현재manifest53파일은 설치Resources와 모두 hash가 같고 이번 후속변경은 새Resources가 없다.
최종 git diff --check PASS. 사용자가 실행할 Debug Client/Server 파일의 경로와 존재도 확인했다.

## G06. G 입장과 기존 등장 휠윈드

기존 typed G 입장을 유지한다. 입장 이동 완료 후 optional entranceCinematicPatternId의
VALTAN_ENTRANCE_CINEMATIC을 한 번 실행하고 기존 introPatternId의
VALTAN_ENTRANCE_WHIRLWIND를 반드시 실행한 뒤 체력 기믹·반복 순서로 진입한다.
초기 이동거리나 컷씬 중 HP crossing 때문에 등장 휠윈드가 생략되지 않는다.
Play All 70 occurrence, 사용자 저장 돌진 이펙트, G trigger 계약은 보존했다.
Python 입장 5개와 실제 Client decision reader 검증은 PASS다.

## G07. 마력구 피해와 무력화 바

STAGGER_SLOT의 12초 channel은 실제 response progress를 HUD STAGGER bar로 보여준다.
시작 1000, 확정 피해 250이면 750, 성공·실패·취소 시 숨김, 다음 시도 초기화를
실제 CCombatHUDViewModel::Apply_Boss native 12개에서 확인했다.
기존 이름만 읽던 mapping에 현재 패턴/action의 정확한 ID를 추가했다.

최종 실패 공격은 전용 damage.valtan.magic-orb-failure의 rate4376을 사용한다.
현재 게시 Retail attack2285에서 방어 전 기본 피해는 floor(2285*4376/100)=99991이다.
공용 130줄 전멸 profile의 rate100000은 변경하지 않았다. 방어·보호 적용 후 실제 HP 감소는 달라진다.
마력구 source2필드와 PROJECT_TUNED 근거를 원자 병합했다.

## G08. 발악 전체 실행과 원본 크기·사운드

저장된 18 stage, 27766ms 흐름은 포탈 → 중앙 → 4방향 → 손 공격 및 플레이어 장판
6회 → 작은 원 → 바깥 원 → 4방향 돌 생성 → 사자후 및 동시 폭파 → 사망 → 부활이다.
4방향 돌은 안/밖 원 공격 이후 생성된다. 돌 source asset은 3시·9시 지형 파괴와 같다.
전체 Room Tick 검증 중 실제 바닥이 있지만 walkable bit가 없는 플레이어 위치에서 장판 생성이
방 전체를 실패시키는 결함을 재현했다. 플레이어 발밑의 정지 SINGLE 장판에 한해 실제 surface를
검증하고 Y를 맞춘다. RADIAL·이동 투사체·바닥 없는 공간은 기존 strict 거부를 유지한다.
최종 Product 기반 surface/거부/transaction 검증 9개 PASS다.

원본 SkillEffect42062411~13의 80cm 원·80~160cm ring·160~240cm ring union에 맞춰
추적 도끼와 같은 native2614 성장 원을 반경 2.4m로 추가했다. 1500ms에 내부가 가득 차고
같은 시점 Server 장판이 폭발한다. 기존 폭발 요소16개와 6개 ActorVisual worldScale1은 보존한다.
실제 codec/playback/shader projection은 월드 지름4.8m, inner 7샘플, 기존 lane889검사 PASS다.
추가 Resources나 shader는 없다. in/out 원본은 4m 원 at1000ms와 3~15m ring at3000ms다.
3~4m의 겹침은 원본의 시간차 공격이므로 임의로 제거하지 않았다.

Sound는 pattern50개·CombatObject8개, 28 event/55 WAV의 설치와 FMOD NOSOUND 해석을 확인했다.
원본 Wwise event/media 연결도 확인했다. STEP09의 기존 Cast2/3400ms는 stage 범위를 벗어나
재생되지 않아 정확한 source notify의 Cast1/400ms로 복원했다. sourceStart399인 cue609는
실제 wall210ms여서 정상이다. 저장된 PROJECT_AUTHORED 5개 sound 선택은 보존했다.
모든 애니메이션 편집과 소리가 원작 전체와 완전히 동일하거나 실제 청취를 했다는 뜻은 아니다.
근거는 out/ValtanEffects20260929/struggling-warning 및 struggling-sound에 있다.

버러지 통합 성공 CATCH_SLAM/EXECUTE_TAIL에 누락된 원본 내려찍기3FX를 연결했다.
TRASH/IF/SUCCESS 두 terminal stage에 총18cue이며 counter/GROGGY에는 추가하지 않는다.
원본1487ms 두 cue는1500ms stage 시작에 age13ms로, 세 번째는1510ms에 재생한다.
원본 position[2,0,0], scale1.8/2.5/1.5와 saved owner scale을 유지한다.
기존 damage/capture/execute와 타이밍을 유지했고 실제 모델·입자13,597검사 PASS다.
근거는 out/ValtanTrashImpact20260929에 있다.

## G09. 유령 체력바와 보조 유령·삼각 포탈

서버는 Respawn 3000ms 완료 후 같은 primary entity의 current/max HP를 ghost profile의
197222731로 채우고40줄을 적용한다. HUD는 archetype가 BOSS_VALTAN으로 유지되어160줄을
읽던 결함을 수정했다. Respawn 재생 중에는 기존 HP 단계를 유지하고 완료된 phase3에서 ghost profile의40줄을 읽는다. HP는 snapshot을 그대로 사용한다.
최대HP 변경 시 이전 피격/줄 감소 cache를 초기화한다. 기존 색 순환과 저장 UI 배치는 보존한다.
실제 Apply_Boss 및 MainApp 막대 계산9개는 변경 전3실패, 변경 후 모두 PASS다.
40줄 full → 반 줄 손상 →39줄 full, 사망0, 일반160줄 재입장을 포함한다.

보조 유령은 소멸 후5000ms에 다음 개체를 생성하며 최대1개다. 사용하는 패턴은 휠윈드,
4연속,2페이즈4방향,십자돌 네 종류다. 메인 유령40줄의 다섯 반복 순서는 그대로다.
삼각 포탈은 시작 시점 간10000ms이며 기존 오브젝트 수명은 유지한다.
finale optional auxiliarySpawnIntervalMs/portalSpawnIntervalMs → PATTERNFINALEINTERVAL →
Catalog → 기존 simulation을 연결했다. legacy 미지정 데이터와 기존6개 pool은 계속 읽을 수 있다.
Python5개·실제 Client finale parser18개·입장 회귀5개 및 전체 관련 TU compile PASS다.
입력·저장·Product parity와 field 부재 보존도 포함한다.

## G10. 병합과 남은 확인 경계

추가 source 반영은 entrance/install-receipt.json, magic-damage/install-receipt.json,
final-followups/install-receipt.json의 최신 디스크 CAS·backup·원자 교체 기록을 따른다.
마지막 batch는 성장 element1, Trash cue18, sound field2, ghost field3의 총24개다.
현재 Client/UI는 실행하지 않았고 Reload나 미저장 draft 교체도 하지 않았다.
최종 화면과 실제 청취는 사용자가 재실행해 확인하는 경계다.

최종 게시 bootstrap의 실제 실행: Catalog/Brain428/428, typed G 입장7/7, 유령 메인5패턴 두 순환·보조4pool·소멸후150tick·포탈300tick5/5 PASS. 전체 Room 발악 검증은24개 플레이어 장판 및 in/out collider,4개 돌 동시 tick649 폭파, Death→Respawn→FourSlash tick1615에서 HP197222731/197222731·40줄을 확인했다. out/ValtanHealthFlow20260929/agent와 struggling-final.run.log에 남겼다.

## G11. 워프 후속 추가

사용자 최종 정정에 따라80→65,65→30,30→15의 마지막에 VALTAN_WARP를 추가했다.
각 구간은 기존9개→Warp→CatchBreath로10개 순환한다. 앞선 다른5개 loop·메인/보조 유령은 보존했다.
Warp는 기존 수동 전용 decision owner에서 자동 owner로 옮기고 phase2/HP16~80, positive weight/repeat
admission을 연결했다. 기존10stage/8rush·총18567ms, eligibility range1, 이펙트·사운드·Play All은 유지했다.
ORDERED_LOOP는 거리 기반 가중 선택을 사용하지 않으므로 range 확대는 하지 않았다.
원본과 projected rotations는 기존 compiler로 함께 계산해 최신 byte CAS·원자 교체했다.
warp-followup/install-receipt.json은 배열3개·승격7개와 무관한 값 보존을 기록한다.
C++ 변경이 없으므로 마지막 Debug Product 빌드를 재사용한다. 워프를 포함한 Gameplay/Composition/PublishCandidate 재게시와 최종 source/Product/hash 검증을 모두 완료했다.

워프 추가 후 실제 게시 데이터로 전체 HP loop446/446, CGameRoom::Tick 세 구간30/30 PASS다.
70/50줄은3시 파괴,20줄은양쪽 파괴를 정본 destruction/nav transaction으로 적용한 상태다.
각 구간 tick1 Dash →474 Warp →1032 CatchBreath(cursor0),8개의16m rush와300/600ms대기,
대기중판정없음, 경로고정/완료를 확인했다. 선택시거리1.22778m에서도 ORDERED_LOOP는 정상이다.
근거: agent/warp-room.run.log, agent/health-runtime-warp.run.log, warp-followup/verification.json.
원작 화면/실제 청취 및 실행 중 도구 Reload는 사용자 확인 경계로 남는다.

## G12. GHOST_FINALE로 인한 반복 입장 실패 수정

이전 G09에서 검사한 Client finale parser 18개는 에디터 ValtanPatternTree 범위였다.
실제 아레나 입장의 별도 CEncounterPatternReference::Load가 두 optional 생성 간격
필드를 거부하는 누락을 발견했다. 따라서 이전 Server 입장 회귀 성공은 Client의
전체 Level 초기화 성공을 뜻하지 않는다. 같은 저장 데이터와 수정 직전 CPP를 실행해
사용자 화면의 Encounter pattern extensions are invalid: VALTAN_GHOST_FINALE를 재현했다.

입장 파서에 auxiliarySpawnIntervalMs/portalSpawnIntervalMs의 동일한 strict 계약을
연결했다. required 필드, 명시 정수 범위 1..600000, 현재 4종/legacy 6종 순서와
기존 child/graph/invulnerability 검증을 유지한다. Data·사용자 저장본은 변경하지 않았다.

수정 후 실제 CProjectDataRoot::Resolve 경로의 전체 Encounter 67패턴 Load,
전체 CinematicCamera 16cue Load, CinematicCameraController Initialize가 모두 PASS다.
각 메서드 전체 소스를 컴파일했다. 독립 native 검사의 attack-contact 의존 함수와
exact-property helper만 현재 ValtanPatternTree에서 그대로 추출했다. Client UI 또는
전체 Level을 실행한 검사는 아니며 화면 확인은 사용자 재실행으로 구분한다.
근거: out/ValtanEncounterFinaleAdmission20260929의 before-run.log, after-run.log,
verification.json 및 수정 직전 EncounterPatternReference.before.cpp.

정식 Debug Product 빌드와 배포 PASS: out/BuildPipeline/runs/
20260928T185613314Z-debug-product.json. Engine/Shared/Server는 최신 상태였고
Client의 EncounterPatternReference 1개 TU를 컴파일해 EXE를 2026-09-29 03:56:12 KST에
교체했다. 기존 헤더의 C4819 경고 외 컴파일·링크 오류는 없다. 새 Data·Resources가
없어 재게시와 GBResources 추가는 필요하지 않다. 기존 워프 포함 게시 결과를 사용한다.

G12 영구 회귀도 PASS다. 실제 ValtanEncounterReferenceContractTests.cpp의 전체 Load로
optional 간격 정상 입력 5종, 기존 Trash/portal/finale와 잘못된 입력 거부·rollback 105종을
확인했다. Python dynamic consumer 계약 3종과 git diff --check도 PASS다.
현재 Death→Respawn에는 없는 과거 terminal SUPPRESS의 5개 거부 검사는 명시적인
테스트 전용 legacy fixture에서 유지한다. 근거: regression-run.log.

## G13. Play Pattern 비활성의 실제 strict parity 수정

사용자 재실행 후 전체 CValtanPatternTree::Load에서 FIST의 STALE_PROJECTION을 재현했다.
수동 시연 원본 repeatPolicy.limit=1은 저장되어 있었고 Python 게시기는 manual의
maximumConsecutiveUses=0을 파생했다. Client만 1을 기대한 것이 원인이며 게시 누락이 아니다.
ValtanPatternTree.cpp:6358에서 manual은0, 자동 후보는 저장 limit를 그대로 기대하도록
맞췄다. 전체 manual24종의 관련5개 필드를 대조했고 차이는 이1건이었다.

최신 실제 Client OBJ와 수정한 전체 Tree TU를 연결한 native 검사10/10 PASS다.
67패턴/328stage/366clip/141cue, 실제 Play Pattern 목록과 source44개 재열기,
수동 임시 limit2→Product0 허용, 자동 휠윈드 limit2→3 불일치의 엄격한 거부와 이전
view 보존을 확인했다. 원본 SHA 전후 동일이며 UI·창·socket은 열지 않았다.
근거: out/ValtanHealthFlow20260929/agent/readiness-RESULT.json, readiness.run.log.

Client Debug 빌드·링크·배포 PASS(2026-09-29 04:11:40 KST). 최신 Tree1개 TU만 변경됐으며
G12 수정도 포함한다. 로그: out/ValtanEncounterFinaleAdmission20260929/client-readiness-build.log.
사용자 Save & Publish 후보 결과도 changedCount0, 기존 source ed17f1... 및 candidate576322...
동일이었다. 이후 Full DataOnly는 별도 world.destruction 필드 검사 실패로 중단됐으므로
이 시점은 전체 게시 완료로 기록하지 않는다. G14에서 해당 소비자를 갱신하고 재검증한다.

## G14. Full DataOnly의 World destruction aim 검증 누락

사용자 실행 Full DataOnly는 world.destruction의 stage exact-field 검사가
VALTAN_SIX_PIZZA_106의 aim을 몰라 실패했다. 전체67패턴/328stage 필드 합집합에서
남은 누락은 aim 하나였고 실제 aim17개를 Client/Server/Gameplay publisher와 대조했다.
World publisher에 optional aim과 targetPolicy 2종, endMs 0..durationMs 정수,
responseScale .01..10 유한 수치 검증을 연결했다. 다른 unknown 필드는 계속 거부한다.
전체 Validate 및 정상7종/비정상22종, 전체 publisher unknown-field2종 검사 PASS다.
Data 원본은 그대로이며 PS UTF-8 noBOM/CRLF와 기존 dirty28줄을 보존했다.
근거: out/ValtanWorldAimAdmission20260929/verification.json, validate.log.
정식 Full DataOnly 재실행 로그는 out/ValtanEncounterFinaleAdmission20260929/
full-dataonly-retry.log다. 최종 게시 완료 여부는 아래 실행 결과로 구분한다.

G14 최종 재게시 완료: Full DataOnly exit0, 총115.0초. 선행7개 검증/domain은 캐시를
재사용했고 World destruction/pickups, Gameplay, Items, Vehicles, HonorTitles,
Valtan rewards까지 모두 PASS다. 기존 source revision ed17f1... 및 Gameplay bootstrap
b8d1c142dfdbb191330c67e586edf51050e6537ef568b09d7239dfa2d1215e66를 유지한다.
World destruction revision은50e5ee40352acad8d6ef6d4738d6a9eb90d31b851db422a8b6e45bf02bfef8ae다.
Client G12/G13 수정은04:11:40 빌드에 포함됐으며 추가 게시기 수정은 C++ 재빌드를
필요로 하지 않는다. 최종 근거는 out/ValtanEncounterFinaleAdmission20260929/final-result.json.
새 Resources는 없고 Client/Server를 자동 실행하지 않았다. 사용자는 Server와 새 Client를
재실행해 화면·실제 조작을 확인한다. 추가 Save & Publish 없이 최종 게시본을 사용할 수 있다.


## G15~G19. 2026-09-29 추가 요청의 Server/Data 반영

사용자 최종 정정을 따라 높이·중앙 착지는 `VALTAN_HIGH_JUMP` 추적 도끼에 적용했다.
serverMotion은 LEAP_TO_TARGET→LEAP_TO_ANCHOR, apex는9→30m다. 기존 중앙
[156.03,22.99751,-122.06]과 1133/1500ms 이륙,267ms 착지는 유지했다. AIRBORNE의
빨간 장판과 LAND cue는 원래 pattern.landing.snapshot이므로 같은 Server 중앙 좌표를
소비한다. 플레이어별 투사체 생성은 보존했다. 이번에 준비했던 피자 변경은 자기
필드만 시작 백업값으로 복구했으며 presentation JSON은 이 하위 작업 시작본과 같다.

SEQUENCE_FOUR의 target/aim은 NONE이며 BeginPattern 직전의 nearest-target 자동
회전도 이 패턴에서만 제외했다. STRUGGLING STEP_04는 기존 aim 계약
PATTERN_TARGET/endMs=0으로 단계 진입 TO_ARENA_CENTER FacePoint와 매 tick aim을
막는다. 다른 발악 단계는 보존했다. 네 contact는1200/2200/3200/4200ms 및
0/180/270/90도 그대로다. Client는 Server yaw를 보간하며 player target yaw를
다시 적용하지 않는다. 부채꼴은 같은 owner basis, contact는 basis+저작 offset이다.

3시·9시 COMBO_STEP_18과 발악 STEP_08의 정확한 세 rock event만 반경을
6.3639610307→5.8639610307m로 줄였다. 중심·개수·방향·돌 scale·피자 돌은 보존했다.
이는 방사 반경0.5m 감소이며45도 배치의 각 X/Z offset 감소는 약0.353553m다.

ValtanBrain::ApplyPatternHit는 TRIPLE_COUNTER/FAIL_3와 STAGGER_SLOT/FINAL_ATTACK을
기존 bEncounterWipe에 연결한다. 살아 있는 인간은 거리·cover·combatReady·counter·
보호막·무적·바훈 보호와 관계없이 최종 실패에서 처리하고 Guide는 제외한다.
첫 두 카운터 실패와 성공 분기는 유지했다.

GHOST_FINALE.portalSpawnIntervalMs=0은 자동 삼각 포탈 중지다. Python/Gameplay
publisher/Server Catalog/scheduler를 연결했다. 미지정 legacy7900ms와 양수1..600000ms,
보조 유령 네 패턴/최대1명/소멸 후5000ms 생성 및 수동 GHOST_PORTAL_ONCE를 유지했다.
Client 두 reader와 tests, 일반→유령 전환 ending 제외는 Client 담당 변경이다.
Server의 실제 primary는 동일 BOSS_VALTAN/boss.valtan.center이고 phase3에 ghost
profile40줄을 적용한다. BOSS_VALTAN_GHOST는 별도 보조 유령이므로 ending owner와 다르다.

세 구르기 후 DASH_CHARGE/CHARGE의 TIMEOUT과 defaultNextActionId는 null로 바꿔
기존 정상 완료를 사용한다. WALL_CONTACT만 recovery/GROGGY로 연결한다. 동일 deadline
WALL도 먼저 소비하므로 Client가 그 action edge에서만 돌진 tail을 강제 정리한다.
정상 완료의 cue lifetime과 tail을 보존하고 새 packet/tick 추정은 추가하지 않았다.

### 수행한 검증

- JSON parse 및 gameplay/presentation authoring validation, 전체8 Product 후보 projection PASS.
  후보만 out/ValtanPatternCorrections20260929/projection에 기록했다.
- test_valtan_finale_interval_contract의5 tests PASS. portal0/양수/미지정, auxiliary0 및
  malformed 거부, projection/provenance/save 보존을 확인했다.
- 제품3 TU(ValtanBrain/GameplayCatalog/GameRoom_BossSimulation)와 기존 contract6 TU를
  MSVC C++20 개별 컴파일하여9 TU exit0. out/ValtanPatternCorrections20260929/compile.log.
  기존 Brain uint64→uint32 C4244 두 경고는 남았으며 이번 변경 줄이 아니다.
- 기존 tests에 protected/distant 실패 전멸·Guide 제외, 독립/발악4방향 yaw, dash 정상
  deadline/동시 wall 우선순위, 자동 portal 부재·양수 triangle 유지와 보조 유령 지속을
  추가했다. Struggling rock fixture의 오래된 STEP04/833ms/5000ms/6200ms는 현재
  STEP08/1200ms/4133ms/5333ms로 갱신했다.
- 변경 파일 git diff --check PASS. 시작본 대비 JSON diff는 gameplay 요청11필드뿐이며
  presentation 차이는0개다. 코드 인코딩을 유지하고 원문 백업은 동일 out/*.before에 있다.

### 통합 검증과 남은 경계

하위 작업은 publisher/Server/Client를 실행하지 않았다. 정식 projection·Composition·
Gameplay 게시와 Debug/Release 제품 빌드, 실제 Server contract 실행 및 PR/merge는
root 통합 작업이 수행한다. 이 절 작성 시 native tests는 컴파일까지만 확인했으며
실행 PASS로 기록하지 않는다. 화면상의 높이·중앙 장판·부채꼴 방향·벽 충돌 tail·
최종 ending은 사용자가 판정한다. Resources 추가는 없다.
