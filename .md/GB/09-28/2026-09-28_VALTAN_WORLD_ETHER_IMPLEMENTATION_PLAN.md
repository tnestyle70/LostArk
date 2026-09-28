# 발탄 World 에테르 구슬 구현 계획

## G00. 기존 경로와 범위

`effect.valtan.ether.orb`는 V1 이펙트 catalog에 존재하지만 월드 배치와 획득 판정은 없다.
기존 Area `CMapEffectDocument`와 `CMapEffectPresentationRuntime`을 확장한다. Effect Tool의
World 목록에 `에테르 구슬` parent와 개별1~6 항목을 둔다. 별도 모델 오브젝트나 두 번째
이펙트 renderer를 만들지 않는다. 초기 위치는 실제 외곽 벽 배치와 접근 가능한 nav cell을
기준으로 정하고, 사용자가 월드 화면에서 위치와 크기를 조절할 수 있게 한다.

## G01. 저장과 게시

정본은 `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.mapeffects.json`이다.
기존 world row의 stable placementId·position·effectAssetId를 유지하고 optional
`pickup {wallGroupId, landingPosition, fallDurationMs, pickupRadiusM}`을 추가한다.
activationPolicy는 SERVER_PICKUP, playbackPolicy는 SOURCE_LOOP이며 activationSetId와
activationWindows는 비어 있다. 원본 이펙트의 유한 emitter 수명 때문에 일반 LOCAL_LOOP를
강요하지 않고 기존 owner-sustained SOURCE_LOOP를 사용한다.

Map publisher의 Effects scope에서 기존 Client mapeffects와 새 Server World bootstrap을
같은 stage/validate/commit/rollback으로 게시한다. header는
`LOSTARK_WORLD_PICKUPS 1 "LV_LUT_HEARTRB_ED" 30 N`, 각 행은
`"placementId" "wallGroupId" startXYZ landingXYZ fallDurationTicks pickupRadiusM`이다.
벽 참조, stable ID 중복, finite·범위, 낙하 방향·기간, nav 착지 가능 여부를 검사한다.
부재·파싱 실패는 이전 활성 데이터 보존 또는 해당 room 초기화 실패로 처리한다.

## G02. Server 권위와 복제

기존 GameRoom의 world destruction 소유 상태에 최대16개 pickup을 연결한다. 해당 벽의
실제 파괴 commit이 성공한 순간 WALL→FALLING, 기간 종료 후 GROUNDED로 진행한다.
낙하 중 획득은 금지하고 GROUNDED와 살아 있는 인간 player collider가 겹치면 한 명에게만
COLLECTED를 commit한다. 중복 접촉은 다시 지급하지 않는다. 외곽109 지형 파괴 commit은
WALL/FALLING/GROUNDED 전체를 REMOVED로 정리하며 낙하보다 정리를 우선한다.

Shared protocol119의 WorldPickups는 placementId/state/XYZ/stateStartTick을 전송한다.
player의 bRonaunGuard와 iRonaunGrantTick은 획득 상태와 문구 중복 방지를 전달한다.
버프는 일반 피해 무적이 아니라 FLOOR_WIPE의 방어 가능한 전멸기1회 보호다. 소모 때 기존
파란 무적 pulse를 사용하고 사망·아레나 reset에는 guard를 지운다. 시간제한은 두지 않는다.
reset은 구슬을 WALL로 복구하며 방 종료·최종 퇴장은 기존 room 수명에 따라 정리한다.

## G03. Client 도구·표현

문서 parser/save와 World 편집 UI가 같은 pickup 필드를 소비한다. 위치·연결 벽·착지점·기간·
획득 반경을 편집하고 기존 CAS 저장을 유지한다. Wait/Fall/Landed의 위치와 낙하 재생을
기존 world preview root로 표시하되 샘플링 좌표로 저작 position을 덮어쓰지 않는다.
Product는 Server snapshot이 오기 전 추측 생성하지 않고, 복제된 위치로 기존 World Effect를
그린다. COLLECTED/REMOVED는 이펙트를 정리한다. 최초 획득 grantTick에 `로나운의 기운`
문구를 표시하고 재전송 snapshot에서는 중복 표시하지 않는다.

## G04. 검증

새 C++ 파일 없이 기존 파일을 확장하므로 프로젝트·filters 추가가 필요 없다. parser와
publisher의 잘못된 참조/위치/기간 및 rollback, 실제 room의 벽 commit/중복접촉/낙하 중
접촉 금지/단일 지급/전멸1회 방어/외곽 정리/reset을 검사한다. Shared codec roundtrip과
malformed 입력 보존을 검사하고 Debug Product를 빌드한다. 화면·청감·최종 위치는 사용자가
확인하며 수치 검증과 구분해 RESULT에 기록한다.

## G05. SOURCE_LOOP 입장 거절 후속 수정

09-29 사용자 화면과 `Client/Default/EffectFailure.user.log`에서 실제 발탄 진입 실패를 확인했다.
구슬의 5개 sprite와 1개 portable Cascade Ribbon 중 TRAIL carrier가 기존 owner-sustained의
sprite/mesh-only 허용 검사에 걸린다. 기존 playback은 해당 ribbon의 지속 방출과 loop 경계
history 보존을 이미 지원한다. `Enable_OwnerSustainedSourceLoops`의 허용 조건에 기존 검증된
portable ribbon을 연결하고, 구조·duration·최소 infinite emitter·model/owner 제외 검사는 유지한다.
Map admission 오류는 Stop으로 지워지기 전에 service 이유를 복사해 placement와 함께 표시한다.

이 수정에는 구슬 Element 삭제, playbackPolicy 변경, 데이터 재게시나 별도 renderer가 필요 없다.
실제 구슬의 장시간 CPU 재생과 LOADING Level의 Spawn/Commit/Update/Stop 경로를 창 없는
native 검사로 확인하고, 정상 Debug Product를 빌드한다. 실제 아레나 화면 확인은 사용자가 한다.
