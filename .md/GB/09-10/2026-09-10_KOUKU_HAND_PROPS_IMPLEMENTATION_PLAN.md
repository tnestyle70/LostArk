# 쿠크 손 소품 월드오브젝트와 패턴 연결 구현 계획

## G00. 현재 상태와 목표

현재 브랜치 `codex/kouku-publish-live-update`의 기존 미커밋 변경을 보존한다.
작업 시작 Composition revision239, Area WorldSequence revision423을
`out/KoukuHandProps20260910`에 보존했다. Client와 Server는 종료 상태다.

`쿠크_레이저` P21은 5833ms, `쿠크_팡파레` P23은 10667ms이며 두 패턴 모두
2관문 쿠크를 사용한다. 원본 WP_1 소켓의 cooked BODY bone은 `b_wp_1`이다.
기존 WorldSequence WORLD/PLAYER와 Composition BOSS_SPAWN은 손 추적을 지원하지 않는다.

목표는 두 소품을 World Object Tool에 등록하고 기존 패턴의 0ms부터 끝까지 WORLD 박스로
연결하여, 보스 몸동작의 손 위치와 회전을 따라가게 하는 것이다. 쇼타임은 원본 총 리소스를
확인해 재사용 가능한 Object로 준비하고 미구현 쇼타임 패턴은 임의 생성하지 않는다.

## G01. 기존 WorldSequence의 보스 본 추적

WorldSequence v3의 objectResources와 instances에 `anchorKind=BOSS`를 추가한다.
부모 resource의 `anchorBossArchetypeId`는 BossCatalog의 stable archetype ID,
`anchorBone`은 실제 BODY bone 이름이다. instance는 부모의 앵커 정보를 소비한다.
비보스 앵커는 두 필드를 비운다. 기존 WORLD/PLAYER와 Composition의 NONE/BOSS_SPAWN은 유지한다.

WorldSequencePlayer는 기존 target provider와 CModel 경로를 확장해 살아 있는 복제 보스의
본 행렬을 매 샘플에 조회한다. 본의 orientation/translation과 확인된 import 단위를 유지하고
소품의 local Transform을 합성한다. 보스 부재 시 숨기며 잘못된 본을 root로 대체하지 않는다.
패턴 종료·Reset·보스 소멸은 기존 occurrence 정리 경로로 처리한다.

World Object Tool의 생성·상세·저장, C++ document parser/validator, Map publisher와
Composition projector가 같은 필드를 소비한다. 새 C++ 파일과 project/filter 등록은 필요하지 않다.

## G02. 리소스와 시퀀스 저작

기존 `fm_g_reup_01.wmodel`과 `fm_g_rhkp_06.wmodel`은 실제 소품 geometry지만 재질 texture 경로가
비어 있다. 기존 Effect용 입력을 보존하고 원본 texture를 연결한 WorldObjects 전용 cooked 모델을
Resources에 설치한다. runtime binary는 Git에 추가하지 않는다.

`월드오브젝트_쿠크트럼펫`, `월드오브젝트_쿠크레이저대포` 부모와 기본 정지 상태를 등록한다.
앵커는 `BOSS_KAKULSAYDON_G2_KOUKU / b_wp_1`, Composition worlds는 `NONE`으로 저장한다.
기존 P21/P23 stage·animation·Logic과 다른 패턴을 보존하고 WORLD occurrence만 추가한다.
원본 socket/particle에서 확인한 local 회전·크기를 적용하며 사용자 화면 검토가 필요한 값은 결과에 명시한다.

원본 정본을 쓰기 직전 backup bytes와 비교하며 revision을 각각 한 번 올린다.
Area WorldSequences scope Publish/Check와 KoukuSaydon domain publish로 생성물을 갱신한다.

## G03. 검증과 사용자 실행

변경 JSON/PowerShell/XML parse, 기존 focused world/document/projector 검사, 최소 Debug 컴파일과
정규 EXE 링크·배포, `git diff --check`를 실행한다. 모델·texture 물리 존재와 실제 bone 단위 및
패턴 시작/종료 시간 참조를 확인한다. 신규 검증 프레임워크는 만들지 않는다.

Client/UI 실행·조작·캡처는 하지 않는다. Server + Client profile을 사용자가 Ctrl+F5로 실행하고,
Lobby → KoukuSaydon → F1 → KoukuSaydon Complete Play → Gate 2 →
쿠크_팡파레 / 쿠크_레이저를 재생한다. 손 정렬·크기·색의 최종 판단은 사용자 검토로 남긴다.

## G05. 손 소품 Transform 편집과 휠윈드 망치

부모 Object Detail에서 기본 Motion 첫 key를 기준으로 Position/Rotation을 노출하고 변경량을 해당
Object의 연결 Motion key 전체에 적용한다. Object Scale은 기존 공통 값이다. 기존 Motion의 상대
궤적과 key 시간은 유지하며 후보 document 검증 성공 뒤 교체한다. 새 JSON 필드는 추가하지 않는다.

Action Workbench의 WORLD 박스 placement는 WORLD resource에서는 기존 절대 배치이고,
BOSS/PLAYER resource에서는 기존 Motion 뒤, 실제 anchor 앞에 합성하는 local 보정이다.
MainApp resource 공급, Workbench 입력·저장, projector admission과 WorldSequencePlayer가 동일한
anchor 의미를 소비한다. Map placement와 Server world-collider는 기존 고정 WORLD 제한을 유지한다.

레이저 대포·트럼펫은 기존 local quaternion에 소품 local Y 180도 회전을 합성한다.
휠윈드 망치는 원본 Encore Kouku의 소켓·mesh·material을 확인하고 기존 CModel/CMaterial 손 소품
경로에 등록한다. P24 WORLD occurrence 추가는 통합 writer가 수행한다.
새 C++ 파일과 project/filter 등록은 없다. 필요한 파일만 컴파일하고 JSON/publisher 검사와
실제 matrix 합성 수치 확인을 수행한다. 화면 판정은 사용자가 한다.

## G06. 09-14 쇼타임 Play의 실제 actor와 총 본 연결

P35의 양손 총과 사용자 placement는 저장돼 있다. single actor preview의 WORLD를 Level에
위임한 뒤 Bundle member.actor 대신 별도 Model View의 전역 본을 조회하는 회귀가 있다.
외부 WORLD sample에 해당 단일 actor의 현재 본 resolver를 명시 전달한다. actor의 애니메이션과
무기 pose를 먼저 갱신하고 WORLD, Effect 순서로 같은 시각을 소비한다. 기존 다중 member와
일반 Model View, Level의 팝업북 visibility·조명 정리 경로를 보존한다.

WORLD Resource/박스 Preview도 BOSS anchor가 있으면 선택 Pattern의 정확한 actor 문맥을
사용해 같은 단일 Bundle 경로를 시작한다. 부착 대상이 다르면 이유를 표시하며 다른 모델을
임의 사용하지 않는다. 회복 가능한 anchor 대기와 실제 WORLD 준비 실패가 애니메이션 정리
메시지에 가려지지 않도록 기존 preview 상태 전달을 연결한다. 총의 저작 본·위치·회전은 바꾸지 않는다.

검증은 실제 호출 순서·동일 actor BODY·양쪽 source bone 샘플·Preview 실패 상태 보존과 관련
최소 TU 컴파일이다. 새 C++ 파일이 없으므로 project/filter 추가는 없다. 실행 중 EXE가 모르는
새 데이터 필드는 제품 빌드 이후 활성화하며 구 parser의 전체 문서 로드도 확인한다.
Client/UI 실행·입력·캡처 및 최종 화면 판정은 사용자가 수행한다.

## G07. 09-14 발사 섬광의 양손·한 손 구분과 총구 연결

사용자는 총 Play/Preview가 동작함을 확인한 뒤 양손 섬광을 총 WORLD에 연결하면 위로 뜨고
회전 중 총구와 벌어진다고 보고했다. 현재 저장 revision576의 P35.presentation.48/49/50은
오른 총 WORLD에 원본 gun.signature를 연결하고 위치 [.4,-2.7,-2.65]m로 보정한다.
원본은 양손 b_wp_1/2 각각11개, 총22개 element의 source attachment와 notify 위치
[1.5,.5,0]m를 포함한다. 총 WORLD에 직접 붙일 기존 gun.signature.world는 한 손11개이며
source attachment를 끄고 element의 위치·회전을0으로 정리한 별도 리소스다.

기존 stable asset/resource ID를 유지하며 원본을 `발사 섬광 양 손`, WORLD용을
`발사 섬광 한 손`으로 이름 붙인다. Authored header, EffectResourceTree와 Composition
별칭을 일치시키고 같은 기관총 분류에서 찾게 한다. 새 asset, shader, binary Resources나
project/filter 항목은 필요하지 않다. 원본 양손의 입자와 한 손의 기존11개 발생기는 보존한다.

사용자가 편집을 저장하고 종료한 뒤 최신 JSON에 다시 기준을 맞춘다. P35에서 양손 리소스를
명시적으로 총 WORLD에 연결한 박스만 한 손 리소스로 변경한다. 해당 총의 정확한 World box를
연결하고 실제 설치 WModel의 총구 중심과 법선에서 구한 local position/rotation을 사용한다.
사용자 발사 시각·길이·scale과 BOSS용 양손 박스, WORLD 총의 본·placement는 유지한다.
단순 anchor 변경만으로 원본 Effect 내부 attachment가 제거된다고 가정하지 않는다.

JSON parse, 변경 필드 외 deep equality, 실제 설치 총구 기하와 WORLD pivot 합성, 기존
Composition projector 검증·게시를 확인한다. Effect 그룹 UI 구현은 기존 Collider 그룹
계획서의 후속 G에서 별도로 관리하며 최종 관련 Client 빌드를 함께 수행한다.
실행 중 draft와 디스크 기준이 충돌하지 않도록 외부 이름 변경은 저장·종료 전까지 stage만 한다.

## G08. V1 WORLD 기본 리소스의 Boss Workbench Append

한 손 섬광의 resource defaultAnchorKind=WORLD에는 특정 Object occurrence ID가 없다.
Append_PresentationCandidate는 이 값을 새 박스에 그대로 복사해 필수 worldId 검사에서
거절된다. Sequence workspace에는 이미 고정 MAP으로 시작한 뒤 명시 선택한 Object만
연결하는 처리가 있지만 Boss Pattern 경로에 적용되지 않았다.

기존 Set_FixedWorldEffectPlacement를 WORLD 기본값의 Effect Append에도 사용한다.
Boss Pattern에서는 현재 cursor와 수명을 유지하고 기존 보스 spawn의 고정 위치에 추가한 뒤
Box Detail에서 총을 선택한다. Sequence의 선택 Object·시작 시각 정책은 그대로 유지한다.
BOSS/MAP 기본 Effect, Light와 이미 배치한 WORLD 박스는 변경하지 않는다. resource 기본값과
사용자 JSON을 수정하거나 필수 worldId 검사를 완화하지 않는다.

기존 native editor contract에서 실제 Append의 이전 거절과 수정 후 추가·저장·재로드,
명시 WORLD 연결, 없는 worldId의 계속된 거절을 확인한다. 테스트 전용 접근은 기존 harness
매크로에 한정하며 제품 API나 객체 layout을 바꾸지 않는다. 관련 최소 컴파일 뒤 사용자의
저장·종료를 확인하고 정규 제품 빌드를 수행한다. Client/UI 입력은 사용자가 확인한다.

## G09. 09-15 쇼타임 위쪽 쿠크의 바주카

현재 P35의 양손 총은 큰 세이튼의 b_wp_1/2에 연결돼 있고, 위쪽 작은 쿠크의
원본 바주카는 별도 Bip002-R-Hand 부착이다. 2관문 레이저의 실제 LaserCannon
CModel/CMaterial과 보정된 local 회전을 재사용하되 G3 Saydon BODY의
`bip002-r-hand`를 쓰는 별도 Object/template/instance를 추가한다.

원본 Action4219939/4219985 notify를 현재 P35의 sourceStartMs, sourceEndMs,
playRate와 반복 구간에 다시 결합해 원본 바주카가 존재하는 WORLD 박스만 만든다.
기존 총, 사용자 placement, P35 애니메이션 편집 및 다른 Pattern은 보존한다.
실제 설치 BODY의 본 basis1.7, source StartSize4, notify scale.3을 분리하고
정규화하는 WorldSequence anchor에서는 소품 scale2.04로 한 번 적용한다.

실행 중 도구의 미저장 draft 보존을 먼저 확인한다. 확인 전에는 out candidate만
만들고, 설치 직전에 최신 원본으로 다시 준비하여 원본 bytes 비교 성공 시에만
바뀐 행을 합친다. 원본 모델·본과 실제 WorldSequence 함수로 수치 검증하고
기존 publisher를 사용한다. C++/shader/새 모델은 이 항목에 필요하지 않다.

### G09 저장 완료 후 현재 세션에서 등록

사용자는 저장 완료와 등록 동안 편집 중지를 명시했다. Client41664와 Server29600의
정확한 PID·기동 시각을 task 전용 receipt에 기록하고 이 세션에서 JSON 등록만 허용한다.
실제 compiler/build와 새 프로세스는 계속 거부한다. 이전 Composition805 후보는
복사하지 않고 최신 저장본에서 원본 notify를 다시 결합하여 기존 행의 bytes와 ID를
보존하는 단일 merge/CAS로 적용한다. 기준 변경은 설치 전에 거부하고 다시 준비한다.

Composition의 기존 writer lock을 획득하고 기준·후보·백업 hash를 검증한 뒤 원자적으로
교체한다. 실패 시 이번 commit만 역순 복구한다. 공용 Save의 freshness 검사와
미저장 draft 거부 정책은 바꾸지 않는다. 기존 순수 심지와 빙고 WorldObject 및
source 근거가 없는 전투 spawn은 변경하지 않는다. Area WorldSequence와 Kouku
Composition 기존 publisher/check를 수행한다. 실행 중 EXE는 모델 조명 수정을 이미
포함하므로 데이터 등록을 위한 재빌드·프로세스 종료·UI 조작은 수행하지 않는다.
