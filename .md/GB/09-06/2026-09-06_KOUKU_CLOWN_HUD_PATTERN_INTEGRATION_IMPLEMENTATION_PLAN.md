# Kouku Clown·HUD·패턴 통합 구현 계획

## G00. 현재 입력과 변경 경계

기준 브랜치는 `GB/koukusaydon-pattern-1-complete`, 시작 HEAD는
`d9b7c2d8a445ca4e1eb1813dd0b5f85d4aef8f34`다. Claude의 미커밋 Logic 런타임과
사용자가 Save Tuning으로 저장한 BossCatalog/Gameplay 값을 보존한다.
현재 전체 diff와 변경 파일의 시작 바이트는 저장소 밖
`C:/Users/user/AppData/Local/Temp/lostark-kouku-integration-20260906`에 보존했다.

## G01. Clown 리소스와 승인된 스킬 표현

`Polymorph 4134`의 조사 기록과 원본 재질이 가리키는 `MN_RPCZ_00-1`을
`Resources/Character/KoukuSaton/MN_RPCZ_00-1`에 별도로 쿠킹한다.
기존 보스 `MN_RPCZ_00`과 세이튼 본체를 덮어쓰지 않는다. 원본 23클립을 보존하고,
대상 골격에 없는 네 춤과 망치는 공통 몸체 본의 world-rest 보정으로 오프라인 베이킹한다.
이 5클립은 project tuned로 구분하고 사용자의 육안 판정을 받는다.
Animation Resource와 Change to Clown이 같은 CModel을 사용한다.
Server의 INTERACTION action, mode, skill index, action start tick을 Character가
소비해 애니메이션을 재생한다. 입력에서 애니메이션을 직접 실행하지 않는다.

## G02. HUD와 판정

광기 100과 Debug Clown은 Clown HUD 및 같은 avatar를 사용한다. Mario, Dance,
Card Maze의 진입은 해당 HUD와 interaction 스킬을 선택한다. Return to Player는
원래 class body와 HUD로 돌아간다. 쿨다운은 Server 기준 3초다.
춤 Q/W/E/R은 양팔모으기/슈퍼맨/양팔벌리기/한다리올리기 순서다.
틀린 입력은 fail, 유효 입력 없이 창이 끝나면 timeout이며 저작 RESULT를 실행한다.
광기 수치와 HUD는 Server snapshot을 따른다.

## G03. 순간이동·가짜와 저작 오류

진짜 찾기 trigger가 실제 보스를 `(-6.36, 1.3, 937.92)`로 옮기고, 최초 spawn의
중심과 해당 점의 XZ 반경으로 4/7/10시 clone을 생성한다. 네 actor는 본체의 +X 정면을
반영한 `faceCenterYawOffsetDegrees=-90`으로 중심을 바라본다.
가짜 pattern은 trigger 시점에 시작한다. 기존 Shared world entity 복제를 재사용한다.
시야 UI는 같은 GAZE_REAL_BOSS 각도·거리를 편집하고 debug geometry를 표시한다.
무력화 성공 follow-up의 실제 존재 여부를 전체 패턴 집합에서 검증하도록 수정한다.
기존 V2 방패를 clip 반복과 독립된 단일 Composition occurrence로 연결한다.

## G04. 튜닝·WORLD·부활

G2 Kouku scale, G2 Big Saydon transform과 무기 scale/rotation을 기존 Save Tuning에
연결한다. Animation Resource preview가 저장된 본체·뿅망치 튜닝을 소비한다.
WORLD 커튼·룰렛 Play는 기존 WorldSequencePlayer를 통해 visibility와 clock을 적용한다.
룰렛은 실제 UV의 8칸을 WORLD 앵커 Collider로 배치하고 Server 카드 문양·색과 비교한다.
Kouku 사망 화면은 기존 DeadScene JSON과
typed revive command를 사용하며 Server는 사망 위치에 부활시킨다.

## G05. 검증

각 변경의 JSON parse, 기존 focused 계약 검사, `git diff --check`, Debug Product
최소 컴파일/배포와 필요한 domain publisher를 수행한다. 새 C++ 파일을 추가하면
Client/Server의 vcxproj 및 filters에 필요한 항목만 등록한다.
Client/UI 자율 실행과 캡처는 하지 않는다. 실제 모습·포즈·무기 각도·패턴 타이밍과
클릭 결과는 사용자 검증이며 실행하지 않은 검사를 PASS로 기록하지 않는다.

## G06. 사용자 재생 피드백과 Composition 리소스 확장

후속 첨부 화면과 서면 관찰은 즉사·부활 동작을 확인했고, HUD Preview 분리와 Big Saydon
컨트롤 위치, 분신 4시 배치·몸체 방향, 방패 반복 fade 및 리소스 lane 누락을 지적했다.
별도 baseline은 `C:/Users/user/AppData/Local/Temp/lostark-kouku-followup-20260906`에 보존한다.

최신 사용자 지시에 따라 UI Preview의 실제 typed 명령 전환은 취소하고 기존 표시 전용 override를
유지한다. 실제 모델·HUD·스킬 변경은 기존 F1 Clown의 Server 승인 명령을 사용한다.
Big Saydon 위치 조절은 해당 boss 행에 둔다. Clown scale은 기존 admission에 0.709를 곱한다.
프라이팬으로 확인된 별도 `IT_GSTFP_00` 장착 part를 제외하고 Clown 본체와 망치 몸동작은 보존한다.
분신은 1/4/7/10시 정간격으로 바꾸고 원본 animation의 root yaw와 Server facing을 대조한다.
Dance/Roulette 시작에는 명시적 resetBossToSpawn을 적용한다. WORLD는 보스 spawn 앵커와
원본 world 중심의 차이로 배치하며 룰렛의 지역 이름은 판정 종류를 암묵적으로 결정하지 않는다.

Composition document에 optional presentationResources/presentationOccurrences를 추가한다.
Workbench는 V2 Effect group/leaf, Sound, Camera, Scene Profile, WORLD와 Collider의 실제 목록,
Create/Preview/Append 및 모든 lane의 Box Detail/시간 편집을 제공한다. 사각형·반원·부채꼴
Collider는 geometry와 활성 창을 저작하고 Preview에 표시한다. `colliderKind`가 판정 지역의
용도를 명시하고 각 배치의 `logicOccurrenceId`가 기존 Logic box에 연결된다. 결과는 해당
Logic의 success/fail/timeout RESULT를 재사용하며 Collider 이름으로 damage를 추측하지 않는다.

`KoukuSaydonPresentationPlayer.h/.cpp`는 제품과 로컬 preview의 occurrence 수명만 소유한다.
Server가 주는 pattern start tick과 Client Product를 소비해 V2 group, 독립 sound channel,
카메라와 scene profile을 기존 서비스로 재생한다. 새 파일은 Client vcxproj/filters에 등록한다.
원본 V2 디자인을 바꾸지 않고 box별 수명·fade·dissolve와 transform을 clone에 적용한다.
방패는 반복 clip binding에서 pattern 전체 창의 단일 occurrence로 옮긴다.
G1 Saydon이 활성화되면 Server가 카드 8종 중 하나를 배정하고 Client가 머리 위 V2로 표현한다.
실제 진짜 Saydon에 하트를 연결하며 owner 종료/재생 중단 시 해당 effect handle만 정리한다.

wire 62의 카드 색과 pattern clock, projector/parser roundtrip, 도메인 publish, 관련 native 검사와
Product 빌드를 수행한다. 사용자가 실행 중인 Client/Server는 자동 종료하지 않으며 필요 시
빌드 대상 파일 잠금을 해제할 시점에 종료를 요청한다.

## G06-1. 최종 저작 데이터와 소비 경계

이 절은 앞 단계의 source 61 범위다. 네 회차와 WORLD/Collider 후속의 변경값은 G07을 따른다.

카드미로 Debug gate의 도착점은 `(0.09, -0.01, 1351.48)`이며 보스를 생성하지 않는다.
Mario2~4의 미등록 진입점은 이번 좌표로 대체하지 않는다.
G1 Saydon 활성 동안 Server가 문양 4종 × RED/BLACK 중 하나를 배정하고 snapshot으로 복제한다.
Client는 해당 카드 V2를 머리 위에 유지하며 NONE·퇴장·owner 종료 때 정리한다.

기존 6칸 검토는 실제 UV 재확인 후 8칸 저작으로 대체한다. mesh yaw 0의 +Z를 기준으로
각 칸 중심을 `22.5 + 45 × n`도로 두고 다음 순서로 저장한다.

| n | 문양 | 색 |
|---:|---|---|
| 0 | SPADE | RED |
| 1 | CLUB | BLACK |
| 2 | DIAMOND | RED |
| 3 | HEART | BLACK |
| 4 | CLUB | RED |
| 5 | SPADE | BLACK |
| 6 | HEART | RED |
| 7 | DIAMOND | BLACK |

8개 지역을 세 판정 창에 배치하여 Collider 24 box를 만들고 판정 창별 DURATION 1개씩,
총 3개 Logic/RESULT 연결을 공유한다. `anchorKind=WORLD`, `worldId`, `regionId`,
`cardSymbol`, `cardColor`, `logicOccurrenceId`는 배치가 소유한다. WORLD 재생 속도는
`22200/34072`로 두어 마지막 판정 뒤에 룰렛이 숨겨지도록 한다. Server는 창 종료 시 XZ
지역과 문양·색을 비교한다. 일치하면 success, 다른 카드면 fail, 모든 지역 밖이면 timeout이다.
일반 영역 판정도 `AREA_OVERLAP` DURATION과 `ENTER_AREA` TRIGGER가 기존 RESULT를 소비한다.
Client Collider는 같은 저작 geometry를 보여 주는 debug mirror이며 PhysX hit 권위를 추가하지 않는다.

하트 3 box는 실제 Saydon Pattern2, 별 3 box는 가짜 Pattern5에 둔다. 방패는 Pattern1의
5263..15947ms 단일 box로 두어 반복 body clip마다 fade가 다시 시작되지 않게 한다.
방패 occurrence yaw와 Server 반사 normal offset에 각각 +90도를 적용하고 보스 body admission은
회전시키지 않는다. 기존 MN_RPCT_05의 방패·별 clip binding은 이관 뒤 제거한다.

| 수정 위치 | 책임과 소비자 |
|---|---|
| `Client/Public/KoukuSaydonCompositionDocument.h`, 대응 cpp와 `KoukuSaydonActionWorkbench.cpp` | optional resource/occurrence, Collider 의미와 Logic 연결의 parse·validate·Save/Reload, Resources 및 Box Detail |
| `Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py`, Shared/Server의 기존 Kouku 계약 | Product `patterns[]`, worldSequenceInstanceId와 typed region 투영, snapshot clock·카드·Server 판정 |
| 새 `Client/Public/KoukuSaydonPresentationPlayer.h`, `Client/Private/KoukuSaydonPresentationPlayer.cpp` | Product/Preview occurrence별 effect·sound·camera·scene·debug Collider 수명 |
| `MainApp.cpp`, `Level_KakulSaydonArena.cpp`, `Animation_Tool.cpp` | 실제 inventory와 preview dispatcher, WORLD pivot·camera·owner view 연결 |
| `EffectV2_Runtime/Object/Target` 및 `Data/Effects/V2` | 기존 Clone/handle 경로의 box별 수명·fade·dissolve·pivot override |

Scene Profile은 기존 서비스를 통해 즉시 적용하고 종료 시 복원한다. 이번 범위에서 blendMs
시간 보간을 구현했다고 기록하지 않는다. 새 Player h/cpp는 Client vcxproj와 filters에 필요한
항목만 등록한다. 최종 Product 링크·runtime 배포와 실제 아레나 화면 확인은 RESULT에서 별도로
완료 여부를 기록한다.

### G04-1. 관문 표시 후속 수정

Kouku와 Large Saydon 모델을 선택한 Pattern 목록은 2관문으로 표시한다.
모델 필터와 무관하게 남아 있던 1관문 안내 문구를 제거하며, 패턴의 저장 ID와 Product 판정은 변경하지 않는다.
보라색 방패는 기존 단일 occurrence(5263..15947ms, 10684ms 유지)를 보존한다.

## G07. 룰렛 네 회차·Collider 저작·커튼 후속

2026-09-06 사용자 저장 revision 73을 기준으로 기존 네 번째 Duration(27487..32231ms)과
Result 연결, 기존 세 판정 창을 보존한다. 룰렛은 world instance 8의 하나의 transform track에서
네 회차를 샘플한다. STAGE_1 4333ms 정지 후 STAGE_3 4667ms 회전, 마지막 1500ms는
각속도가 선형으로 0에 이르는 곡선을 quaternion key로 베이크한다. 첫 정지는 전체 9000ms,
짧은 회전은 10428..11625ms, 다음 회차는 현재 Stage의 7334ms 주기로 맞춘다.
네 회차의 key와 창은 `out/KoukuIntegration-roulette-four-cycle-timing.json`의 source 74 수치로
대조한다. WORLD 속도 1과 33773ms track을 사용하며 앞 checkpoint의 `22200/34072` 시간 늘리기를
대체한다. 기존 24개 카드 지역과 세 판정은 유지하고, 네 번째 판정은 `AREA_OVERLAP`,
`insideOutcome=FAIL`에 WORLD 기준 Circle 1개를 연결한다. 원본 radius 2m에 placement XZ scale 4를
적용한 실제 반경은 8m다. 사용자 결과 목록을 새 목록으로 재작성하지 않는다.

Collider resource에 CIRCLE, occurrence에 debugRender(default true), AREA_OVERLAP에
insideOutcome(SUCCESS/FAIL)를 추가한다. Duration은 기존 Result를 소비하고 Trigger는
ENTER_AREA + MAX_HP_PERCENT_DAMAGE를 원자적으로 생성/재사용한다. 피해 수치는 기존 정본의
최대 HP %로 표시한다. WORLD Trigger도 동일한 source transform key를 Server tick에서 샘플한다.
Client 단독 Collider preview는 유효 아레나 위치를 사용하고 끝 프레임에 머물며, Box Detail 수정과
Debug Render 체크를 반영한다. 실패 이유는 Preview 상태에 표시한다.

씬프로필_진짜쿠크세이튼찾기는 이미 있는 sceneprofile.1과 Pattern2의 2007..26134ms 박스를
보존하고 배포 closure를 검증한다. 커튼은 원본 KoukuSaton_Curtain의 post-process material
참조를 기존 Effect V2 `TexturedOverlay`와 `Add_ScreenOverlay`에 연결한다. 원본 재질
`FX_M_MI_D_00.FX_M.FX_D_Po_Curtain`이 참조하는 `fx_tex_high_00.fx_d_symbol_100_ycl` DDS를
`Effect/KoukuSaydon/Screen/fx_d_symbol_100_ycl.dds`에 설치하고 leaf `boss.kouku.curtain_1`이 소비한다.
원본 cooked graph가 제거한 이동식은 동일 재현으로 단정하지 않으며, 하강·유지·상승 비율은
프로젝트 조정값으로 저장한다. 새 렌더러나 맵 mesh 변형 경로를 만들지 않는다.

WORLD 정의의 optional `companionEffectResourceId`가 같은 Composition의 EFFECT resource를
참조하고, 배치된 EFFECT box의 optional `worldOccurrenceId`는 같은 pattern의 WORLD box를
참조한다. WORLD Preview/Append는 이를 함께 준비한다. 제품은 명시적으로 저장된 Effect box를
소비하며 숨은 effect를 추가 재생하지 않는다. 연결된 Effect box의 시간·속성은 독립 편집할 수 있다.
Preview는 source draft generation 변경 시 현재 선택과 clock에서 다시 stage하고, 단독 Collider
Preview의 마지막 유효 시점을 유지해 Box Detail/Debug Render 수정이 즉시 반영되게 한다.
실패 시 정상 기존 source와 다른 instance를 유지한다. 기존 맵 커튼 11개는 native 3500ms의
0/1050/2450/3500ms에 Y 18→0→0→18과 마지막 hide를 저작한다. Pattern6의 2671ms WORLD 창은
speed `3500/2671`로 맞추고 명시적 EFFECT도 2671ms로 배치한다. 단독 World Resource Preview는
native 3500ms를 사용한다. 새 Effect JSON은 기존 `96.DataFiles/Effects/V2/Authored`에 등록한다.

검증은 기존 Composition/Server 계약, source/Product 및 worldsequence parse와 실제 키 샘플,
해당 C++ 컴파일, 마지막 Debug Product 빌드·배포로 수행한다. Client 시각 확인은 사용자가 한다.

## G08. Collider 실제 WORLD pivot과 Preview 표시 재검토

사용자가 Debug Render를 켠 Box Detail Preview와 Sequencer Play 모두 선이 없으며 기존
진짜 쿠크세이튼 찾기 시야 Collider는 보인다고 보고했다. 실제 두 경로는 CHitAreaWire와 같은
ImGui frame/background drawlist를 사용한다. 그림 레이어 변경은 근거가 없어 하지 않는다.

WorldSequencePlayer는 샘플한 TRS를 실제 object/batch에 적용하고 placement.record는 재생 복구용
원본으로 보존한다. Level_KakulSaydonArena::Try_GetCompositionWorldPivot이 이 원본을 읽어서
보스 앵커 오프셋·회전이 적용된 룰렛과 Collider 위치가 달라진다. 기존 Player에 현재 재생 중인
map placement TRS 조회를 추가하고 Level은 preview/제품 시퀀스의 그 조회를 사용한다. 데이터
원본 record를 덮어쓰거나 Server 판정 경로를 바꾸지 않는다. 조회 실패는 해당 프레임의 표시만
보류하며 다음 프레임에 정상 pivot이 생겼을 때 복구할 수 있도록 한다.

단독 Collider Preview는 살아 있는 아레나 player를 기준으로 공간을 확인하도록 연결하고, 일반
Animation/Effect Preview의 기존 animation target 계약은 유지한다. 생성 시점과 표시 상태를
actual consumer에서 확인하고 최소 컴파일·수치 검증·diff check를 기록한다. 이전의 3초 수명
유지 수정은 프레임 stall 원인 규명과 별개이며 실제 3초 stall 해결로 보고하지 않는다.

## G09. G2 Big Saydon 저장 Transform과 실제 생성 높이

사용자가 2026-09-06 23:28:20에 저장한 BossCatalog/Gameplay.world를 보존한다. body scale
.0692, hammer scale .0103829, hammer rotation(-96.5,-6.5,6.5), position(10.24,8.63,317.75),
yaw226.5가 현재 정본이다. World bootstrap은 이전 위치라 공식 World publisher로 재배포한다.
Client body/weapon은 ActorCatalog를 직접 읽으므로 source를 다시 변경하지 않는다.

CGameRoom::Build_WorldEntity는 boss 높이를 무조건 navigation ground로 교체하고 있다.
KAKULSAYDON_ARENA의 BOSS_KAKULSAYDON_G2_BIG_SAYDON은 저작한 대형 무대 보스 Transform을
유지하도록 Y를 보존하되 기존 XZ walkability 검사·projection·실패 처리는 그대로 사용한다.
다른 보스/다른 world는 기존 ground Y 동작을 유지한다. 실제 생성·spawn baseline·다음 tick에서
높이가 유지되는지를 기존 ServerGameplayContractTests에서 확인하고 새 하네스는 만들지 않는다.
이후 표준 Debug Product 빌드와 공식 World publish, 사용자 저장 원본 byte 보존을 확인한다.

이번 카메라 compile 오류는 ArenaCameraProfile.cpp의 numeric_limits::max()와 Windows max
macro 충돌이다. (numeric_limits::max)()로 수정해 프로젝트의 기존 macro 설정을 유지한다.
이전 Temp 최소 컴파일의 NOMINMAX 옵션은 제품 설정과 달라 오류를 놓쳤으며 최종 Product
빌드 결과로 구분해 기록한다.
