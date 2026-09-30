# 배틀 아이템 투척 조준 구현 계획

## G00. 요청과 현재 소비 경로

숫자 1~4에 등록한 투척 아이템은 첫 키 입력에서 로컬 사거리와 목표 표시를 열고,
물리 좌클릭에서 투척을 확정한다. 물리 우클릭은 조준만 취소한다. 조준을 끝낸 클릭은
버튼을 뗄 때까지 이동·평타·핑으로 다시 소비하지 않는다. 기존 스킬의 마우스 교환 정책은 유지한다.

현재 MainApp::Update는 숫자키 edge에서 Request_UseItem을 즉시 호출하고,
PlayerController는 그 프레임 cursor를 range 안으로 clamp해 C2S_USE_ITEM을 제출한다.
Server::Handle_UseItem은 이미 투척/아군/본인 대상을 분리하고, 수량·쿨다운·상태·거리·navigation을
검증한 뒤 기존 CombatObjectRuntime 투사체 또는 승인 presentation을 만든다.
이번 입력 작업은 그 Server 권한과 protocol을 바꾸지 않는다.

기존 08-22 DIMENSIONMASTER_GROUND_TARGET PLAN/RESULT와 09-29
VALTAN_ARMOR_AND_BATTLE_ITEM_EFFECTS PLAN/RESULT를 기준으로, 스킬용 targeting state와
CSkillGroundTargetPreview의 동일 runtime 경로를 확장한다. Client UI 자동 실행은 하지 않는다.

## G01. PlayerController.h의 명시적인 아이템 조준 상태

CGROUND_TARGETING_STATE는 NONE/SKILL/ITEM 구분을 소유한다. SKILL은 기존 skill ID를,
ITEM은 stable item ID와 예약 request sequence를 별도로 소유한다. 아이템을 가짜 skill ID로
변환하지 않는다. 거리 clamp·target sample·Cancel은 두 종류가 공유한다. 스킬만 walkable sample 승인을 요구한다.

아이템 arm은 packet·수량·쿨다운·action sequence를 소비하지 않는다. MainApp의 item request
sequence는 성공한 로컬 arm 시 예약하며, 실제 Server command는 확정 후에만 그 번호를 사용한다.
Cancel은 예약된 번호를 폐기한다. Server sequence는 연속 번호가 아니라 newer 판정이므로
취소로 생기는 번호 간격은 권한이나 소모 상태를 변경하지 않는다.

Controller는 arm한 character class와 level ID를 기록하고, character/sink 교체와 Level 수명 종료에서
조준을 폐기한다. 기존 CaptureInputGate로 조준 중 물리 마우스의 남은 hold를 소유하여
확정·취소·UI 전환 뒤 같은 hold가 이동이나 평타가 되지 않게 한다.

## G02. PlayerController.cpp의 arm·업데이트·확정 경계

Request_UseItem은 실제 catalog의 투척 정의에만 ITEM 조준을 연다. 비투척 아이템은 기존 경로를
유지한다. 시작과 업데이트는 최신 HUD의 생존·combat-ready·action·폼·탑승·속박 상태,
read-only inventory 수량과 복제 cooldown을 검증한다. 아이템의 사거리와 preview 자료는
ItemCatalog의 검증된 정의를 사용한다.

ITEM Update는 cursor를 caster 기준 범위에 clamp하고 기본 표시 높이는 caster plane으로 정한다.
기존 walkable sample이 있으면 3m 이내 높이를 표시 참고로 사용하지만, Client의 지면 정보 부재나
blocked cell을 투척 거절 근거로 쓰지 않는다. 범위 내 finite target의 물리 LMB fresh edge에서
기존 C2S_USE_ITEM을 구성하여 IPlayerCommandSink로
제출한다. sink 거절은 조준을 유지하고, 성공은 preview를 정리한다. 물리 RMB는 전송 없이 정리한다.
UI mouse/keyboard capture·focus 상실·free camera·사망·실격·맵/클래스 변경·수량 소실·쿨다운·서버
행동 변경은 조준을 취소한다. held 입력은 release까지 소비된다.

기존 Guardian 화신화 S와 차원술사 T는 SKILL 경로를 그대로 사용하며, 스킬 ID·action sequence·
마우스 교환 동작을 아이템과 섞지 않는다. Server 승인 후 투사체·아이템 효과를 생성하는 기존
replication/presentation 경로를 유지하고 로컬 arm에서 사용 효과를 먼저 재생하지 않는다.

## G03. MainApp 입력과 Inventory UI

MainApp 숫자키는 focus/UI/carry로 금지된 프레임에도 raw down 상태를 관찰한다. capture 해제나
focus 복귀 때 누르고 있던 숫자키가 새 조준으로 바뀌지 않게 한다. InventoryView는 계속 slot 등록과
equipment 요청만 담당하고, UI 클릭 소비는 CUIInputRouter를 통해 Controller까지 전달한다.
불필요한 Inventory packet 경로나 새 renderer를 추가하지 않는다.

## G04. 검증과 전달

수정 파일은 기존 PlayerController.h/.cpp, MainApp.cpp이며 새 제품 C++ 파일이나 프로젝트 등록은
없다. ItemCatalog/SkillGroundTargetPreview의 같은 경로 확장은 별도 owner와 조율한다.

실제 pure state와 입력 gate/availability consumer를 실행하는 작은 native 검증으로 item arm 무전송,
범위 clamp, 아이템의 Client 미확정 지면 제출과 스킬의 navigation 실패, 좌클릭 once, 우클릭 무전송·hold 차단, sink 거절 유지, 수량/쿨다운/사망/
실격/클래스·Level/UI/focus 취소와 기존 SKILL 동작을 확인한다. 통합 owner가 Debug/Release
제품 컴파일을 수행하며, 변경 JSON/XML parse와 diff --check를 기록한다. 실제 바닥 표시·소리·
투사체의 Client 화면 판정은 사용자 확인 범위로 남긴다.

## G05. 기존 renderer와 아이템 정의의 연결

ItemCatalog는 기존 battleUse kind에서 DESTRUCTION/WHIRLWIND만 isGroundTargeted로 명시한다.
검증된 range/radius 숫자에서 RangePreview/TargetPreview 지름을 만들며 별도 gameplay 수치를
중복 저장하지 않는다. 두 preview descriptor는 기존 PLAYER_SKILL_TARGET_PREVIEW 타입을 쓴다.
원작 battle item DDS 자료를 기존 renderer의 단일 coverage/tint 계약으로 검증해 연결한다.

CSkillGroundTargetPreview::Begin(rangePreview,targetPreview) overload를 추가하고 기존 스킬
Begin은 같은 함수로 위임한다. 두 texture의 준비와 descriptor 검증을 모두 성공한 뒤 commit하고,
실패하면 부분 준비 리소스를 보존하지 않는다. renderer/shader를 새로 만들지 않는다.

## G06. 아이템 지면 판정의 기존 Server 권한 보존

Server Sample_SurfacePosition은 독립 navsurface와 runtime void 조건을 사용하지만, Client NavGrid는
walkable/height만 보유한다. 같은 의미가 없는 두 함수를 동등한 판정으로 연결하지 않는다.
이번 기능에 NavGrid·sidecar·publisher·동적 void 전송 확장을 추가하지 않는다.

아이템은 기존 즉시 투척 입력과 같은 clamp XZ 제출 범위를 유지한다. Client preview는 사거리와
목표 의도만 표시하며, 보행 불가 지면이나 미확인 지면이 실제 투척 가능하다는 확정 표시가 아니다.
Server가 surface/void/height/상태/소지 조건을 최종 승인한 뒤에만 수량·쿨다운·투사체가 변경된다.
CGROUND_TARGETING_STATE의 Apply_TargetSample은 finite한 같은 XZ 목표를 승인하고,
기존 Apply_WalkableSample은 그 공통 검증에 위임한다. 스킬 호출자는 계속 navigation 성공 때만
호출하며, 아이템 호출자는 clamp된 목표를 사용한다.

native 입력 probe는 supported-but-blocked와 Client 미확인 surface에서도 item command 전달과
Client 수량 불변을 검증한다. 실제 void 거절은 Server의 기존 guard가 담당하며, 이번 Client fixture의 좌표 상태만으로
Server 승인·거절 실행 검증을 주장하지 않는다.

## G07. 원본 조준 표시와 이미지 여백 보정

원본 GR_BattleItem_01의 Par_C_CircleTarget_01 재질이 참조하는 완성 원형 DDS
fx_tex_high_00.fx_b_decal_005를 기존 두 quad에 사용한다. 커서와 범위의 tint는 각각
GR_BattleItem_01 및 DefaultRange의 ActiveColorValue/DeactiveColorValue를 소비한다.
범위 원작 재질은 직선 texture를 원으로 감는 native shader이므로 완성 원형 mask를 범위에도
사용하는 부분은 PROJECT_COMPOSITION으로 구분한다. 원작 전체 particle/light/UV shader 재생을
구현했다고 쓰지 않는다.

DDS의 R coverage는 512px 중앙 행에서 x29..482이다. visible ring 직경 454px/512px를
textureDiameterFraction으로 명시하고 ItemCatalog가 battleUse range/radius의 2배를 이 비율로
나누어 quad 크기를 만든다. gameplay 거리 수치는 한 곳에 유지하고 기존 skill descriptor와
shader는 바꾸지 않는다. 이 비율은 finite 0초과 1이하만 허용한다.

Publisher는 Client-only descriptor의 정확한 필드, Resources 상대 DDS 경로, finite tint와
provenance를 검증한다. Items.bootstrap 행은 기존 battleUse만 사용한다. 신규 DDS는 같은
Resources 상대 경로로 로컬 GBResources에 함께 복사하고 원본/두 목적지 hash를 비교한다.
