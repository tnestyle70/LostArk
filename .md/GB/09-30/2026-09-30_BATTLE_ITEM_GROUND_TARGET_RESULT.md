# 배틀 아이템 투척 조준 구현 결과

## G00. 입력 구현 상태

PlayerController.h/.cpp와 MainApp.cpp의 투척 입력 연결을 완료했다.
숫자 1~4의 파괴 폭탄·회오리 수류탄은 실제 item ID와 예약 request sequence를 ITEM 조준 상태에
보관한다. arm은 서버 요청·수량·쿨다운·스킬 action sequence를 소비하지 않는다.
거리 clamp와 target sample은 기존 스킬 targeting state를 공유하고, 범위 안 finite 목표의 물리 좌클릭에서
기존 IPlayerCommandSink::Request_UseItem에 ground target XZ를 한 번 제출한다.

물리 우클릭은 packet 없이 취소한다. CaptureInputGate가 조준 중 양쪽 마우스를 release까지
소유하므로 이동·평타 guard를 통과하지 못한다. item active 동안 ping 요청도 차단한다.
마우스 교환 설정이 켜져 있어도 아이템은 물리 좌클릭 확정/우클릭 취소이고, 기존 스킬은 원래
교환 정책과 action sequence를 유지한다. public Request_MoveToPointResolved도 item active에서
거절하여 Level의 NPC/상호작용 호출이 Controller.Update 전에 투척 조준을 이동으로 소비하지 않는다.

매 프레임 최신 수량·쿨다운·생존·combat-ready·action·속박·폼·탑승·class·Level을 다시 확인한다.
시간 정지의 기존 Server ActiveBuff 33500도 유효기간 동안 arm/confirm을 거절한다. 아이템의 기존
walkable sample은 3m 이내일 때 preview 높이에만 참고하며, sample 부재·보행 불가·높이 차이만으로
투척 intent를 거절하지 않는다. 기본 높이는 caster plane이다. UI mouse/keyboard/text capture와 focus,
free camera에서 취소한다. character/sink 교체도 기존 controller 경계에서 정리한다.

MainApp 숫자키는 UI capture/focus 상실 때도 raw edge를 관찰한다. UI를 닫거나 focus가 돌아왔을 때
이미 누르던 숫자키가 조준으로 바뀌지 않는다. InventoryView의 carry/equip 경로는 수정하지 않고
기존 CUIInputRouter claim과 QuickSlotDragView carry 플래그를 소비한다.
성스러운 부적과 시간 정지 물약의 기존 즉시 요청 경로는 유지한다.

## G01. 실제 입력 소비자 native 검증

`out/BattleItemGroundTarget20260930/generate-probe.py`는 다음 제품 source를 원문으로 추출한다.

- CGROUND_TARGETING_STATE, Poll_GroundTargetingClick, CaptureInputGate와 confirm transaction.
- Is_GroundTargetItemAvailable, Request_UseItem, Cancel_GroundTargeting 전체 함수.
- PlayerController::Update의 active targeting 분기와 실제 capture/filtered mouse guard.
- MainApp::Update의 숫자키 입력 분기.

Win32 입력·navigation·renderer·network sink는 제어 가능한 fixture로 대체했다. sink 호출 횟수,
실제 C2S_USE_ITEM 필드, state, preview valid 여부와 move/basic-attack guard의 차단 조건을
검증했다. 전체 Client Update·GPU draw·실제 네트워크를 실행한 결과로 확장하지 않는다.

컴파일 exit 0, 실행 exit 0, **68 PASS / 0 FAIL**이다.

| 범위 | 실제 판정 |
|---|---|
| arm과 confirm | arm 무전송, item identity/sequence 분리, 7m clamp, confirm once, local quantity 불변 |
| 취소와 mouse swap | RMB 무전송, cancel 우선, release 전 move/BA 차단, swap에서도 물리 LB/RB 유지 |
| 재시도와 target | 비finite 거절, sink 거절 때 유지, held 무재전송, 새 edge만 재시도 |
| 지면 권한 분리 | 아이템 blocked/미확인 surface도 intent 제출·Client 소모 없음, 높이 fallback, 스킬 blocked 거절 유지 |
| 상태 취소 | 사망·실격·속박·knockdown·falling·grab·skill·차량·Mario·clown·class·Level·owner/sink 소실 |
| 입력 취소 | mouse/keyboard capture, runtime UI/text, ImGui text, focus, free camera |
| 소지와 쿨다운 | 소진·미복제·장착 stack 거절, future/exact-end/wrapped cooldown, 33500 time-stop |
| 기존 계약 | 스킬 기본/swap confirm와 sequence, potion 즉시 요청, 키 hold/carry/capture 해제 |

증거는 `out/BattleItemGroundTarget20260930/build-probe.log`, `probe.log`, `probe-source.json`이다.
`probe-source.json`은 실제 추출 파일 hash와 fixture 범위를 기록한다. 기존 Shared 헤더의 C4819
코드페이지 경고는 있지만 컴파일 오류는 없다.

## G02. 제품 빌드와 사용자 화면 경계

입력 owner는 제품 파일을 freeze하여 통합 owner에게 전달했다. 이 문서의 입력 검증 시점에는
사용자가 제품 빌드를 미뤘으므로 이번 변경을 포함한 전체 Debug/Release 제품 빌드 완료를 주장하지 않는다. 새 제품 C++ 파일이나
vcxproj/filter 등록은 추가하지 않았다. 기존 파일의 UTF-8과 CRLF를 유지했다.

실제 Client에서 사거리/목표 표시의 원형·위치·색·가시성, 물리 마우스 확정/취소와 Server 승인 후
투사체 재생은 사용자 화면 판정 범위다. Client/UI와 실행 중인 Client/Server 프로세스를 에이전트가
실행·조작·종료하지 않았다. 기존 수동 HUD 좌표 저장 파일도 변경하지 않았다.

## G03. 아이템 조준 정의와 기존 표시기

ItemCatalog의 두 투척 정의에 Client-only groundTargetPreview를 추가했다. isGroundTargeted는
DESTRUCTION/WHIRLWIND만 사용하며 RangePreview/TargetPreview는 기존 typed descriptor다.
사거리 7m와 영향 반경 3m는 기존 battleUse rangeCm/radiusCm에서 계산한다. 별도의 gameplay
숫자나 가짜 skill ID를 추가하지 않았다. 성스러운 부적·시간 정지의 즉시 요청 계약은 유지한다.

CSkillGroundTargetPreview::Begin(rangePreview,targetPreview)를 추가했고 스킬 Begin도 동일
함수에 위임한다. 두 descriptor와 texture를 모두 준비한 뒤 commit한다. invalid extent/tint,
잘못된 asset path, missing DDS는 이전 표시를 보존한다. missing DDS는 CTexture::Create의
MessageBox 경로에 들어가기 전에 is_regular_file(error_code)로 거절한다. Clear는 두 texture와
표시 상태를 정리한다. 새 shader·renderer·제품 C++ 파일·프로젝트 등록은 없다.

## G04. 원본 근거와 의도적으로 제한한 표현 범위

원본 Item 101912/101221은 UseTargetRange 700, UseSkillEffect 32140/32310,
ProjectionSkillEffectId 32141/32311이다. SkillEffect 32141/32311의 AreaRange는 300이다.
현재 정본 숫자와 일치한다. GR_BattleItem_01 원본 LOA는
FX_BS_03.mark.Par_C_CircleTarget_01을 참조하고, 그 decal Required 재질은
fx_m_mi_04.fx_mi.fx_e_de_ri_06_1_tr이다. 재질의 emissive_tex_03은
fx_tex_high_00.fx_b_decal_005를 직접 가리킨다.

이 DDS는 완성된 원형 ring과 안쪽 soft coverage다. 원본 원형 일부인 ring018/019는 1/4원이며
그대로 쓰면 모양이 깨지므로 사용하지 않았다. 원본 DefaultRange는 CircleRange_01과
fx_c_de_circle_02_ad의 procedural material을 사용한다. 이번 구현은 확인한 완성 원형 mask를
caster와 cursor의 기존 두 quad에 사용하는 PROJECT_COMPOSITION이다. 원본 전체 native
material/UV 애니메이션/point light 복원이나 원작 최종 화면과 동일함을 주장하지 않는다.

색은 EFGame EFParticleGroundData의 실제 필드 순서로 해독했다. 범위 Active/Deactive는
[0.9,1.1,1.5,1]/[5,0.5,0.1,1], 배틀아이템 커서는 [1,1,2,1]/[1.5,0.3,0.1,1]이다.
텍스처 identity는 SOURCE_VERIFIED, 조합 usage는 PROJECT_COMPOSITION으로 저장했다.
Item ProjectionArea=7 원본 값은 보존했지만 cooked script에 없는 원작 native C++의 선택
callsite까지 증명했다고 확대하지 않는다.

512px DDS의 중앙 행에서 shader와 같은 R>=1/255 coverage는 x29..482(포함 454px)다.
textureDiameterFraction=454/512로 투명 여백을 보정했다. quad 직경은 약 15.78855/6.76652m,
보이는 ring 직경은 14/6m다. gameplay 최대거리 7m와 반경 3m 및 기존 스킬은 바꾸지 않았다.

새 리소스는 Effect/World/Targeting/Textures/fx_b_decal_005.dds 하나다. 원본 DDS와
Client/Bin/Resources 및 C:/Users/user/Desktop/GBResources의 상대 경로가 모두
131200bytes, SHA256 0a28e46d8667c16402019d9377f9d9593250243a5ff6c10680539c0eab52f978로
일치한다. 두 목적지에 없던 파일만 복사했다. 로컬 GBResources 검증이며 외부 Drive 업로드나
동기화 완료를 주장하지 않는다. Guardian 효과와 렌더링 옵션은 변경하지 않았다.

## G05. 표시·파싱·게시 검증

ItemCatalog.cpp와 SkillGroundTargetPreview.cpp 실제 TU를 독립 native 컴파일/link했다.
실제 ItemCatalog::Load는 237개 항목과 두 투척 visible ring 14/6m를 통과했고, missing descriptor,
missing target, escaped path, NaN JSON, invalid alpha, radius0, 비투척 preview, duplicate item,
fraction0/fraction>1의 10가지 입력을 거절하면서 이전 catalog를 유지했다.

실제 CSkillGroundTargetPreview::Begin에 WARP device와 설치 DDS를 전달했다. texture 두 개 로드,
첫 Set_State 전 inactive, missing DDS/NaN diameter/invalid alpha의 기존 pointer·활성상태 보존,
기존 skill overload와 Clear를 확인했다. Render·Client·UI는 실행하지 않았다.

Publisher sandbox positive1/negative10은 11 PASS다. 정본 CheckPublished도 unchanged PASS로
Items.bootstrap의 기존 gameplay byte가 그대로임을 확인했다. 두 아이템의 groundTargetPreview
외 모든 ItemCatalog 필드가 baseline과 같고 JSON parse 및 대상 git diff --check가 통과했다.
실행 증거는 out/BattleItemTargeting20260930의 source-evidence.json, resource-delivery.json,
native-catalog.log, preview-probe.log, publisher-fixture-results.json, publisher-check.log다.
제품 Debug/Release 빌드와 실제 표시 판정은 통합 owner/사용자 확인을 기다리는 경계다.

## G06. 지면 판정 보정과 최종 검증 경계

독립 검토에서 초기 입력 코드가 아이템에도 walkable sample을 강제하여 기존 투척보다 엄격해진
차이를 발견했다. Server는 Sample_SurfacePosition의 독립 surface/void 계약을 쓰고 Client는
그 정보를 모두 보유하지 않는다. 최종 입력은 기존 Request_UseItem의 clamp XZ 제출 범위를
보존한다. 아이템 조준 표시는 사거리와 목표 의도이며, 해당 지면의 실제 투척 허가가 아니다.

기존 walkable sample이 있으면 표시 높이를 참고하고, 없으면 caster plane을 사용한다.
스킬은 계속 walkable sample이 있어야 확정된다. Apply_TargetSample로 공통 함수의 의미를
정리했으며 이 지면 보정을 위해 Engine/Character/NavGrid/Navigation publisher/sidecar를 변경하지 않았다.

최종 68개 Client native 검사에는 supported-but-blocked와 Client 미확인 surface에서 아이템 intent를
서버로 보내면서 Client inventory를 바꾸지 않는 사례, blocked 스킬 거절과 높이 fallback이 있다.
별도의 out-only 실제 CGameRoom 검사는 게시 Valtan room과 현재 Debug 제품 object로
Handle_UseItem을 실행하여 **17 PASS/0 FAIL, exit0**를 확인했다. 두 투척 아이템 모두 0.5m 거리의
붕괴 void, 12.2555m 낮은 지면, 사거리 초과에서 수량5·쿨다운 없음·투사체0·outbound0을 보존했다.
이동은 막혔지만 물리 지면이 있는 목표는 수량4·쿨다운·투사체1로 정상 허용했다.
최초 positive fixture는 붕괴를 복원하지 않은 전제 오류2개가 있었으며 원로그를 보존하고 조건을
복원한 뒤 다시 실행했다. 증거는 out/BattleItemGroundTarget20260930/server-surface-result.json이다.
제품 Server source·EXE·게시 데이터는 수정하지 않았고 Client 화면 검증과도 구분한다.

## G07. 통합 컴파일·보존과 사용자가 미룬 제품 빌드

변경한 실제 Part_Vehicle.cpp, PlayerController.cpp, MainApp.cpp, ItemCatalog.cpp,
SkillGroundTargetPreview.cpp를 Debug/Release 옵션으로 독립 컴파일하여 모두 exit0를 확인했다.
입력 코드의 컴파일 전후 source hash도 같다. 증거는 out/DragonFlight20260930/compile-result.json,
out/BattleItemGroundTarget20260930/compile/compile-result.json 및
out/BattleItemTargeting20260930의 실제 TU compile 로그다. 기존 코드페이지/export 경고는 있으며,
이 검증은 제품 전체 EXE/DLL 링크·배포를 대신하지 않는다.

용 비행은 실제 설치 CModel과 현재 제품 함수 6개를 사용한 571개 이륙·비행·착륙 pose에서
snapshot 재설치 높이 차이가 0이다. 원인·수정·서버 99개 검사·한계는
../09-27/2026-09-27_DRAGON_FLIGHT_CAMERA_IMPLEMENTATION_RESULT.md의 G07에 기록했다.

사용자는 실행 중인 Release Client/Server를 유지하고 빌드를 나중에 하도록 명시했다.
전체 Debug/Release Product build, 통합 publish와 기존 ZIP 갱신은 이번 추가 변경에서 실행하지 않았다.
현재 실행 EXE와 이전 LostArk-Verification-20260930.zip에는 이번 조준·용 비행 수정이 포함되지 않는다.
Client/UI는 실행하거나 종료하지 않았다. 새 텍스처의 설치본·GBResources 복사는 완료했다.

가디언 나이트 저작 문서는 수정하지 않았다. 이번 범위 시작 뒤 49150 clip1의 새로운 사용자 저장이
다시 관찰됐으며, 과거 후보나 ZIP의 저장본으로 되돌리지 않았다. 원본 JSON parse와 기존 Client
project/filter XML parse, 작업 사본 git diff --check를 확인했다.
