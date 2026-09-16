# 쿠크 Sequence 재생과 Object 편집 구현 계획

## G26. 1관문 연출 종료와 네 플레이어 도착 Logic

2026-09-15 사용자 요청이다. 일반 Play가 enterCombatOnFinish를 보고 Complete Play를
요청하던 분기를 제거한다. 일반 Play는 끝에서 플레이어 follow 카메라와 입력을 돌려주고,
명시적 Complete Play / Sequences + Pattern Flow만 기존 Server Gate activation과 저장된
Pattern Flow를 이어간다. 종료에서 follow를 켜는 요구는 이전 G03의 종료 정책을 대체하며,
Pause와 중간 F6의 저작 동작은 유지한다.

ROOM_PLAYER_ARRIVAL Trigger 하나와 occurrence별 playerSlot/position을 가진 네 박스를
현재 Sequence의 쥐 불꽃 시점에 둔다. MainApp은 admission된 불변 Pattern의 앞으로 진행하는
시계만 소비하고 한 run에서 각 occurrence를 한 번 제출한다. 정지·scrub은 이동을 보내지 않는다.
CPlayerController → IPlayerCommandSink → 기존 Debug World Playback command로 연결하며,
Server가 같은 방의 PlayerId 순 참가자를 run 시작에 고정하고 목적지의 navigation·충돌을
검증한다. 빈 참가자 슬롯은 이동 없이 완료한다. 다른 World의 플레이어는 대상으로 삼지 않는다.

Server 응답은 기존 Sequence Viewer 소비 지점에서 request ID로 분배한다. 거절·시간 초과는
해당 Sequence 실패로 남기며 전투로 넘어가지 않는다. Complete 종료의 기본 Gate 이동이
이미 완료한 네 도착 위치를 덮어쓰지 않도록 Gate activation에 명시적 위치 보존 옵션을 둔다.
사용자가 저장한 다른 Composition·Effect·맵 값은 변경하지 않는다.

수정 범위는 기존 MainApp/SequenceViewer, Workbench, Arena, Composition codec,
Shared Debug World Playback, Server room과 대응 데이터다. 새 런타임 파일은 만들지 않는다.
최소 TU 컴파일, packet/서버 이동 계약, Sequence parse/저장 왕복, 일반/Complete/실패·중복
시계 검사를 수행한다. Product 빌드와 실제 Client 화면 판정은 사용자가 수행한다.

## G00. 현재 연결과 변경 범위

`MainApp.cpp`는 Resource Preview를 시작한 뒤 이전 Animation Preview를 정리한다.
현재 정리 결과를 공용 상태 문자열에 기록해 `No KoukuSaydon composition preview is playing`이
정상 Resource 시작 결과를 가린다. Presentation Player는 occurrence별 실패 여부만 저장하고
이유는 공용 문자열에 두므로, 뒤의 정상 Effect admission 메시지가 앞의 실패를 덮어쓴다.

기존 사용자 변경과 미저장 draft freshness 경계는 유지한다. Client/UI를 실행하거나 캡처하지
않는다. MainApp Preview 정리, Presentation Player의 capture 경계와 실패 전달,
WorldObjectTool의 timeline 선택·복제 및 Kouku Workbench animation 행을 수정한다.
Server Mario logic와 Composition schema 변경은 병렬 Server 작업에서 소유한다.

## G01. 재생 실패 보존과 실제 Effect 준비

작은 오망성의 실제 정본 `effect.kouku.gate3.mario.boss.pentagram.full.restore`를 현재
codec·Product admission·설치 Resources로 검증한다. 실패의 구체 원인을 교정하고
준비 큐의 원인 문자열을 occurrence에서 보존한다. capture는 아직 준비 중인 대상과 실제
실패를 구분하고, 해당 capture와 현재 활성 occurrence의 실패만 처리한다.

## G02. Object timeline

`CWorldObjectTool`은 기존 `WORLD_SEQUENCE_TEMPLATE`의 transform, animationTracks,
effectTracks를 소비한다. Stage/Transform/Animation/Effect 행 이름을 표시하고 animation
clip은 slot별 한 행에 배치한다. 개별 box 선택을 Detail에 연결하고 duplicate 명령은
현재 문서 복사 → 타이밍·ID 검증 → template 교체로 처리한다. Animation duplicate는
원래 window 뒤에 같은 clip을 반복하며 필요한 motion 종료 key만 연장한다.

## G03. Pattern과 팝업북

Workbench의 Pattern animation clip별 행을 stage/profile의 한 행으로 통합한다. 팝업북의
기존 펼친 기본 자세, WORLD Sequence, 조명, 실제 Saydon target/clip 소비를 대조하여
소비가 끊긴 부분을 수정한다. 저장된 사용자 pattern 배치와 연출 타이밍은 유지한다.

## G04. 검증

변경 TU의 Debug 컴파일, 실제 Effect codec/Product staging, Object 문서 duplicate의
저장·재로드와 실패 보존, 변경 JSON/XML parse, `git diff --check`를 확인한다.
새 C++ 파일은 추가하지 않으므로 project/filter 등록 변경은 없다. Product 전체 빌드는
root 작업에서 조율하며 사용자에게 F1 재생·선택·복제·저장 확인 경로를 전달한다.

## G05. 비활성 Client 입력·사운드와 Map Light 삭제 저장

`CInput_Device`의 BACKGROUND 키보드·마우스 폴링을 foreground 수집으로 바꾸고,
장치 획득 실패와 focus 상실 시 raw 상태와 edge 상태를 함께 비운다. 포커스 복귀 시
이미 눌린 입력은 release 후 새 press부터 허용한다. `CPlayerController`는 비활성 창에서
targeting·hold와 예약 입력을 취소하며, MainApp의 직접 Escape 폴링에도 focus를 검사한다.

사용자가 지정한 `C:/Users/user/Desktop/Winters/Engine/Private/Sound/Sound_Manager.cpp`의
foreground PID 기반 master group mute 방식을 기존 LostArk `CSound_Manager`에 연결한다.
다른 프로세스 창과 다른 Client EXE가 활성화되면 현재 Client의 master 출력만 mute하고,
같은 프로세스 도구 창은 유지한다. 채널별 볼륨·pause·seek·재생 시각을 변경하지 않는다.

`MainApp_RenderingLighting.cpp`의 Map Delete는 삭제한 stable ID와 Map 저장 도메인을
유지한다. 삭제된 선택 행이 없어도 Map Save/Publish를 제공해 기본 directional의
RenderingProfiles Save로 넘어가는 경로를 막는다. 실제 저장 JSON은 삭제 대상이 특정되기
전 임의로 변경하지 않는다. 기존 source freshness 검사와 미저장 draft를 보존한다.

검증은 입력 focus 상실·재획득·held key/button, mute 전이와 개별 sound 상태 보존,
Map Delete 이후 Save/Publish 소비 경로, 실제 authoring/runtime JSON 비교, 증분 Product
Build 및 diff 검사로 한다. Client/UI 조작과 여러 EXE의 실제 청취 판정은 사용자가 한다.

## G06. 느린 프레임에서 반복되는 19,819ms 캡처 대기

`CKoukuSaydonPresentationPlayer::Sample`은 이전 sample과 150ms 이상 차이가 나면
`Stop_Session`으로 V1 handle을 지운다. 이 handle의 renderer가 고정 화면을 소유하므로
다음 `Resolve_PreviewCaptureClock`이 다시 19,819ms로 돌아가 캡처를 기다린다.
기존 resolver 검사에서 ready를 직접 설정한 것은 이 수명 연결을 검사하지 못했다.

단일·bundle Preview session은 프레임 간격을 occurrence 종료로 처리하지 않는다.
기존 V1/V2 외부 시계의 forward/rewind 샘플을 사용하고, 실제 박스 종료·Stop·새 Preview가
handle을 정리한다. Server 제품 session의 기존 discontinuity 처리는 유지한다.
같은 Preview session 판정은 V1 capture sample에도 재사용한다. 시간·카메라·shader와
사용자가 저작한 JSON 값은 변경하지 않는다.

실제 Sample의 session 정리 분기와 resolver를 연결해 100/200/500ms 프레임으로
캡처 이후 커서 진행·소유 유지·박스 종료를 검사한다. 실제 portal Product Stage와
Playback 4.260초의 ScreenPost, renderer Build/Bind의 고정 캡처 완료를 별도로 확인한다.
이는 Client/UI 장면 실행이나 사용자의 최종 화면 판정과 구분한다.

## G08. 3관문 기본 외곽불 Despawn 버튼

F1 Gate Controls의 `Despawn Arena Bosses` 바로 아래 `Despawn Fire Object`를 추가한다.
Level의 기존 Gate Object presentation owner 중 gateIndex=2인 D/E/F 여섯 motion을 정리한다.
Boss despawn이 active gate 표시를 초기화하므로 실제 object owner를 기준으로 판정한다.
Gate/player 이동 대기 중에는 실행을 막고 기존 release의 실패 이유를 상태 줄에 전달한다.

정상 release 뒤 owner를 제거해 다음 Update나 Gate 전환 실패 rollback에서 불이 다시
생기지 않도록 한다. 다음 명시적인 Gate 3 activation은 기존 준비·commit 경로로 복원한다.
사용자가 편집하는 WORLD/Pattern preview와 저장된 emission·반경·모델 데이터는 유지한다.
변경 MainApp와 WorldObjects TU의 Debug 컴파일 및 diff를 확인하고 실제 버튼 화면은
사용자가 새 빌드 후 확인한다. 새 C++ 파일과 project/filter 등록은 필요하지 않다.

## G09. Parent Complete Play의 빈 타임라인 진단과 Effect 트리 공유

현재 저장된 `쿠크세이튼_분신소환_기분나빠` Parent(folder.17)의 backing Pattern P52는
15초 duration만 있고 실행 행과 childPatterns가 없다. 같은 folder의 P50/P53/P54/P55 소속은
재생 occurrence가 아니며 네 Pattern은 모두 동일한 `boss.kakulsaydon.g3.saydon`을 대상으로 한다.
Complete Play는 현재 backing timeline을 올바로 요청하지만 빈 타임라인이라 실제 행동이 없다.
기존 Parent 전개·Product·Server 소비를 유지하면서 빈 Parent의 실행 요청에 이유와
`Append Pattern at Cursor` 연결 방법을 표시한다. 정상적인 공통 연출만 가진 Parent는 유지하고,
폴더 소속을 임의 순차·동시 재생으로 바꾸거나 저작 Pattern을 자동 배치하지 않는다.

Effect_Tool_ResourceBrowser의 Kouku All Effects와 ActionWorkbench의 Composition Effect는
각자 label/category 트리를 만든다. 기존 EffectAuthoringResourceTree의 공용 view 구성에
1관문/2관문/3관문/공통 네 묶음과 표시명·검색·정렬을 모으고 두 소비자를 연결한다.
정본 organization/gate metadata를 우선하고 기존 gate asset ID 근거를 사용하며, 관문 근거가
없는 항목은 공통에 보존한다. 각 소비자의 Open/Play/Append와 세부 element 동작은 유지한다.
Effect body·resource ID·저장 category metadata를 임의 재저작하지 않는다.

Parent 재생은 현재 요청/확장 경로의 집중 검증, Effect 목록은 동일 입력의 분류·검색·정렬·항목
보존 비교, 변경 TU의 최소 Debug 컴파일과 diff를 확인한다. Client/UI 실행은 사용자 몫이며
최종 Product Build는 기존 EXE 종료 대기를 따른다. 동일 모델 Bundle 제약과 진짜/가짜
개체의 소유 구조는 실제 codec/Server 계약을 조사한 결과에 근거해 사용자에게 제시한다.

## G10. 전방 Summon에서 세 방향 분신 Pattern 재생

사용자는 전방 P50 안에 Summon.2를 저장하고 후방 P53·왼쪽 P54·오른쪽 P55를
각각 새 분신에서 재생하도록 연결을 요청했다. 전방 본체의 추가 시퀀스와 이펙트는
사용자가 이어서 저작한다. 이번 작업에서는
Pattern child actor selector나 Bundle actor slot 확장을 추가하지 않는다.
기존 본체가 전방 Pattern을 실행하고, 가짜 세이튼은 생성된 개체에서 각 방향 Pattern을
실행하는 역할 구분을 권한다. 모델·Pattern 정의와 실행 개체의 identity는 구분한다.

현재 일반 Summon 정의는 이름, occurrence는 시작/수명만 저장한다. 실제 분신의 archetype,
배치와 재생 Pattern을 일반 Summon에 연결한 계약은 아직 없다. 기존 Gaze는
REAL_GAZE_TELEPORT와 정확히 세 clock-hour 위치·공통 clone Pattern을 쓰는 전용 경로다.
따라서 Summon 이름만 추가하면 3관문에서도 실행된다고 설명하지 않는다.
3관문에 재사용하려면 분신별 위치·Pattern, dependent actor admission, Server 판정과
owner/run 종료 시 despawn을 같은 기존 생성·복제 경로로 연결해야 한다.

현재 네 방향 Pattern은 동일한 boss.kakulsaydon.g3.saydon을 가리킨다. Bundle의 중복
거부는 동일 모델 파일 자체가 아니라 같은 boss placement/NetEntity의 동시 행동 소유를
막는다. 이 검사를 삭제하여 본체 하나를 여러 분신처럼 취급하지 않는다.
Parent folder.17의 P52는 실행 행이 없는 15초 타임라인이며, 폴더의 패턴 소속은 실행
연결이 아니다. P53/P54/P55의 NORMAL category, 빈 P51을 참조하는 Bundle.11,
왼쪽과 같은 action/clip을 가진 오른쪽 P55도 실제 저작을 완성할 때 확인해야 한다.

Summon occurrence에 optional patternSpawns를 추가한다. spawnId, patternId, 본체 기준
positionOffset와 yawOffsetDegrees를 저장하며 같은 이름을 공유하는 기존 P44 Summon에는
영향을 주지 않는다. 현재는 같은 Gate·actor의 animation-only MECHANIC Pattern을
1~4개 허용하고 재귀 Summon·Logic·공통 world/scene/effect 실행은 거부한다.
Publisher는 SUMMON_PATTERNS와 PATTERNSUMMONSPAWN을 생성하며 Server는 기존
Build_WorldEntity → Begin_Pattern → spawn payload 전부 준비 후 commit한다.
본체는 이동시키지 않고 각 분신은 기존 owner ID·sequence·lifetime으로 정리한다.
각 방향 원본 clip과 Server root motion이 같은 소환 기준에서 진행하며 임의 반경은 넣지 않는다.
Client는 기존 dependent spawn/snapshot과 CModel/CNpc를 사용하고 본체 HUD를 보존한다.
저장된 P50 박스만 세 Pattern에 연결하고 세 자식 category를 MECHANIC으로 바꾼다.
오른쪽은 원본 P44의 확인된 오른쪽 action/clip으로 연결해 왼쪽 복제 클립을 교정한다.
변경 전 저장본을 보존하고 현재 파일 hash가 일치할 때 필요한 JSON 객체만 갱신한다.
검증은 실제 projection/negative tests·Server/Client 최소 컴파일·기존 clone 수명과
root motion 소비 확인으로 수행하며 실행 중 EXE와 UI는 조작하지 않는다.

## G11. 공용 WORLD 앵커 검사로 차단된 기존 통합 카메라 복구

사용자 화면은 Sequence 정본의 P4.presentation.23에서 worldId 누락 오류를 표시한다.
이 행은 고정 좌표 이펙트가 아닌 기존 WORLD camera shot이다. 최근 공용 Validate에
추가된 WORLD 참조 검사가 kind와 무관해 별도 Sequence 정본의 기존 카메라를 거부한다.
Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json과 gameplay Composition은
같은 codec/Workbench를 소비하므로 양쪽 정본에서 해당 kind를 함께 검증해야 한다.

WORLD 오브젝트 참조를 실제 소비하는 Effect/Light/Collider에는 worldId 검사를 유지한다.
Camera/Sound의 전역 재생에는 빈 worldId를 허용하며 카메라 좌표·shot·시간을 변경하지 않는다.
Client authoring/PRODUCT 검사, Product reader, Python validator의 kind 범위를 일치시킨다.
원본 Sequence 파일을 MAP으로 일괄 전환하거나 camera shot을 지우지 않는다.
실제 저장 Sequence의 Parse/Validate/Expand와 Camera/Product reader, Python projection을
검증하고 두 변경 TU만 컴파일한다. Client가 실행 중이면 최종 링크는 종료 뒤 수행한다.
gotchas와 렌더링 복원 문서에 공용 검사 변경 시의 소비자 범위·실패 종류를 기록한다.

## G12. 캡처 시간 초과 확인과 임시 제외안 — G13에서 원복

실행 중 Client의 읽기 전용 상태 기록에서 P4는 단일 member Bundle backend이며,
19,819ms에서 capture allowed 상태로 120 preview updates를 기다린 뒤 timeout으로
종료됨을 확인했다. Portal 객체는 존재하지만 color/bloom capture pair는 없고 WORLD
오류는 없다. GPU Bind 직접 호출 검증은 이 라이브 완료 경로를 증명하지 못한다.

사용자는 캡처 실패를 계속 추적하는 대신 해당 연출을 빼고 팝업북으로 진행하는 대안을
허용했다. 전용 portal Effect의 authored.scene-image-collapse만 visible=false로 바꾼다. shrink timing 검증에 필요한 enabled 메타데이터는 보존한다. 이 asset의 다른 30 particle, Sequence의 WORLD,
Camera, 팝업북 시작 21,010ms와 전체 길이는 보존한다. 별도 backdrop은 존재하지 않는다.
현재 source hash를 확인하고 원본 byte를 out에 보존한 뒤 visible bool 하나만 수정한다.
실제 Product document staging, 경계 전후 ScreenPost 미생성 및 Sequence capture 대상
제외를 확인한다. 데이터 변경이므로 EXE Build는 불필요하며, catalog는 시작 때 읽으므로
현재 Client의 재시작이 필요하다. 사용자 편집을 Save한 후 사용자가 직접 재실행한다.

## G13. 암전 DDS의 알파 채널 검증과 정상 캡처 제출 보존

사용자는 구조적으로 불가능할 때만 연출을 제외하고, 수정 가능하면 유지하라고 명확히 했다.
G12의 임시 visible=false 변경은 원본 byte로 복원한다. 현재 설치 Engine DLL의 실제
Render_ScreenPosts에서 원본 portal capture가 정상 완료되어 제거의 구조적 근거는 없다.

P4.presentation.19의 kouku.gate1.authored.fade.black은 전체 구간에 걸쳐 V2
TexturedOverlay를 제출한다. BASE fx_a_blankwhite_01.dds는 DXT1이고 실제 loader는
BC1/BC1_SRGB SRV를 유지한다. 기본 coverage A를 Engine의 HasOverlayCoverageChannel이
거부하는 것이 첫 결함이다. BC1은 불투명/1-bit alpha를 공급하므로 A를 허용한다.
다른 형식의 기존 channel 검증, SRGB 계약과 텍스처 데이터는 변경하지 않는다.

Engine/Private/Presentation_Manager.cpp의 Submit_FrameProviders는 Renderer::Draw가
현재 프레임의 provider들을 제출할 때 호출한다. 기존 local isolated 실패는 전체 queue를
clear하고 S_FALSE를 반환하여 Client는 계속 그리되 정상 캡처는 실행되지 않는다.
실제 DLL과 정상 portal + 실패 provider를 섞은 검사에서 이 상태가 20회 반복됐다.

provider마다 light/post/overlay vector 길이와 submission stats를 보존한다. 명시적인
LOCAL_PROVIDER_CONTRACT이며 Is_PresentationFailureIsolated인 실패만 해당 checkpoint로
돌리고 그 provider를 Finalize(false) 후 나머지 제출을 계속한다. 정상 provider는 마지막
channel 검사 뒤 Finalize(true)한다. 실패 횟수와 S_FALSE 진단은 남긴다. budget 초과,
전역 실패, scope가 불명확한 실패는 기존 전체 rollback과 실패 HRESULT를 보존한다.
이미 Finalize(false)한 provider는 뒤의 전역 실패 때 중복 완료하지 않는다.

새 public 타입/파일은 없고 프로젝트와 filters 등록도 변경하지 않는다. 실제 DDS admission,
정상/실패 provider의 순서 및 부분 제출 rollback, 전역 실패, 정상 capture의 renderer 소비를
검증한다. 이후 정식 Debug 빌드하고 실행 파일 갱신과 사용자 화면 확인을 구분한다.

### HasOverlayCoverageChannel의 A 반환부

```cpp
		return eFormat == DXGI_FORMAT_R8G8B8A8_UNORM ||
			eFormat == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB ||
			eFormat == DXGI_FORMAT_B8G8R8A8_UNORM ||
			eFormat == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB ||
			eFormat == DXGI_FORMAT_R16G16B16A16_FLOAT ||
			eFormat == DXGI_FORMAT_R32G32B32A32_FLOAT ||
			// BC1 supplies alpha too: opaque blocks return 1, transparent mode 0/1.
			eFormat == DXGI_FORMAT_BC1_UNORM ||
			eFormat == DXGI_FORMAT_BC1_UNORM_SRGB ||
			eFormat == DXGI_FORMAT_BC2_UNORM ||
			eFormat == DXGI_FORMAT_BC2_UNORM_SRGB ||
			eFormat == DXGI_FORMAT_BC3_UNORM ||
			eFormat == DXGI_FORMAT_BC3_UNORM_SRGB ||
			eFormat == DXGI_FORMAT_BC7_UNORM ||
			eFormat == DXGI_FORMAT_BC7_UNORM_SRGB;
```

### Submit_FrameProviders 전체 반영 코드

```cpp
HRESULT CPresentation_Manager::Submit_FrameProviders()
{
	const HRESULT hPendingProviderFailure = m_hPendingProviderFailure;
	m_hPendingProviderFailure = S_OK;
	m_LastSubmissionStats = {};
	m_eLastFailureScope = PRESENTATION_FAILURE_SCOPE::NONE;
	m_iLastTransientLightCount = 0u;
	m_iLastScreenPostCount = 0u;
	m_iLastScreenOverlayCount = 0u;
	m_bSubmissionTransactionActive = true;
	m_TransientLights.clear();
	m_ScreenPosts.clear();
	m_ScreenOverlays.clear();
	vector<shared_ptr<IPresentationProvider>> Providers =
		std::move(m_FrameProviders);
	m_FrameProviders.clear();
	bool_t bIsolatedProviderFailureSeen = false;
	const auto FinalizeProviders =
		[&Providers](const bool_t bCommitted)
		{
			for (const shared_ptr<IPresentationProvider>& Provider :
				Providers)
			{
				if (nullptr != Provider)
					Provider->Finalize_PresentationSubmission(bCommitted);
			}
		};
	for (const shared_ptr<IPresentationProvider>& Provider : Providers)
	{
		if (nullptr != Provider)
			Provider->Begin_PresentationSubmission();
	}
	if (FAILED(hPendingProviderFailure))
	{
		++m_LastSubmissionStats.iProviderFailures;
		m_LastSubmissionStats.bCompleted = true;
		m_LastSubmissionStats.bCommitted = false;
		m_eLastFailureScope = PRESENTATION_FAILURE_SCOPE::GLOBAL_RUNTIME;
		m_bSubmissionTransactionActive = false;
		FinalizeProviders(false);
		Clear_Frame();
		return hPendingProviderFailure;
	}
	for (shared_ptr<IPresentationProvider>& Provider : Providers)
	{
		if (nullptr == Provider)
		{
			++m_LastSubmissionStats.iProviderFailures;
			m_LastSubmissionStats.bCompleted = true;
			m_LastSubmissionStats.bCommitted = false;
			m_eLastFailureScope = PRESENTATION_FAILURE_SCOPE::GLOBAL_RUNTIME;
			m_bSubmissionTransactionActive = false;
			FinalizeProviders(false);
			Clear_Frame();
			return E_FAIL;
		}
		// Each provider stages one contribution. An isolated local rejection must
		// not discard a healthy capture, light or overlay from another owner.
		const size_t iLightCheckpoint = m_TransientLights.size();
		const size_t iPostCheckpoint = m_ScreenPosts.size();
		const size_t iOverlayCheckpoint = m_ScreenOverlays.size();
		const PRESENTATION_SUBMISSION_STATS StatsCheckpoint = m_LastSubmissionStats;
		const HRESULT hProviderResult = Provider->Submit_Presentation();
		if (FAILED(hProviderResult))
		{
			const PRESENTATION_FAILURE_SCOPE eFailureScope =
				Provider->Get_PresentationFailureScope();
			// The provider owns failure provenance.  Preserve that exact scope
			// through frame finalization; the HRESULT value must never be used to
			// guess whether the failure is object-local or shared runtime state.
			m_eLastFailureScope = eFailureScope;
			const bool_t bIsolatedProviderFailure =
				Provider->Is_PresentationFailureIsolated() &&
				eFailureScope ==
					PRESENTATION_FAILURE_SCOPE::LOCAL_PROVIDER_CONTRACT;
			if (bIsolatedProviderFailure)
			{
				m_TransientLights.resize(iLightCheckpoint);
				m_ScreenPosts.resize(iPostCheckpoint);
				m_ScreenOverlays.resize(iOverlayCheckpoint);
				m_LastSubmissionStats = StatsCheckpoint;
				++m_LastSubmissionStats.iProviderFailures;
				bIsolatedProviderFailureSeen = true;
				Provider->Finalize_PresentationSubmission(false);
				// Already finalized: later global rollback must not finalize it twice.
				Provider.reset();
				continue;
			}
			++m_LastSubmissionStats.iProviderFailures;
			m_LastSubmissionStats.bCompleted = true;
			m_LastSubmissionStats.bCommitted = false;
			m_bSubmissionTransactionActive = false;
			FinalizeProviders(false);
			Clear_Frame();
			return hProviderResult;
		}
	}
	if (!Is_CompleteChannelSubmission(m_LastSubmissionStats.Lights) ||
		!Is_CompleteChannelSubmission(m_LastSubmissionStats.ScreenPosts) ||
		!Is_CompleteChannelSubmission(m_LastSubmissionStats.ScreenOverlays))
	{
		++m_LastSubmissionStats.iProviderFailures;
		m_LastSubmissionStats.bCompleted = true;
		m_LastSubmissionStats.bCommitted = false;
		m_eLastFailureScope = PRESENTATION_FAILURE_SCOPE::GLOBAL_RUNTIME;
		m_bSubmissionTransactionActive = false;
		FinalizeProviders(false);
		Clear_Frame();
		return E_FAIL;
	}
	m_iLastTransientLightCount = static_cast<uint32_t>(m_TransientLights.size());
	m_iLastScreenPostCount = static_cast<uint32_t>(m_ScreenPosts.size());
	std::stable_sort(m_ScreenOverlays.begin(), m_ScreenOverlays.end(),
		[](const PRESENTATION_SCREEN_OVERLAY_DESC& Left,
			const PRESENTATION_SCREEN_OVERLAY_DESC& Right)
		{
			return Left.iSourceOrder < Right.iSourceOrder;
		});
	m_iLastScreenOverlayCount =
		static_cast<uint32_t>(m_ScreenOverlays.size());
	m_LastSubmissionStats.bCompleted = true;
	m_LastSubmissionStats.bCommitted = true;
	m_bSubmissionTransactionActive = false;
	FinalizeProviders(true);
	if (bIsolatedProviderFailureSeen)
		m_eLastFailureScope = PRESENTATION_FAILURE_SCOPE::LOCAL_PROVIDER_CONTRACT;
	return bIsolatedProviderFailureSeen || 0u < m_LastSubmissionStats.Lights.iSuppressed ||
		0u < m_LastSubmissionStats.ScreenPosts.iSuppressed ||
		0u < m_LastSubmissionStats.ScreenOverlays.iSuppressed ? S_FALSE : S_OK;
}
```


## G14. 캡처 축소와 팝업북 첫 프레임의 시간 연결

사용자가 확정한 캡처 19,819ms와 팝업북 21,010ms 사이 1.191초를 소비한다. 기존 capture는 한 번 찍힌 Color/Bloom을 유지하고 왜곡은 capture 전에 실행된다. 현재 결함은 V2의 두 번째 검정 overlay가 capture 뒤 화면 전체를 가리는 것과, Composition box를 줄여도 V1 내부 4.130초 shrink가 그대로인 데 있다.

`KoukuSaydonPresentationPlayer`가 실제 box 길이를 기존 `Seek_WorldRoot`에 전달한다. `EFFECT_SPAWN_DESC::fExternalPlaybackEndSeconds`는 pending spawn부터 Object/Renderer로 전달하는 occurrence 시간이며 prepared resource나 다른 skill과 공유하지 않는다. Renderer는 scene-collapse에서만 authored shrink를 box 끝까지 남은 시간으로 제한한다. 0은 기존 standalone 동작이고 cube/model-cue는 바꾸지 않는다. capture SRV와 particle clock은 유지한다. 유효하지 않은 외부 시간은 seek/spawn에서 거부한다.

저장본의 사용자 위치/회전과 다른 occurrence는 보존하고 capture box는 15,559~21,010ms로 맞춘다. 내부 delay 4.26초가 19,819ms를 만든다. 첫 0~2초 fade는 보존하고 popup을 덮는 두 번째 fade만 제거한다. 실행 중 미저장 draft를 먼저 확인하고 파일 freshness를 지킨다. P36 stage 공백은 G15에서 stable animation pair를 보존해 해결한다.

새 C++ 파일이나 프로젝트 항목은 추가하지 않는다. 변경 TU 최소 컴파일, 실제 renderer 시간/동결 capture 확인, JSON parse와 domain publish를 실행하고 Product 링크는 실행 EXE 종료 후 수행한다. 화면 판정은 사용자 확인으로 남긴다.


### G14 변경 함수 코드

`Client/Private/Effect_PresentationService.cpp`

```cpp
bool_t Client::CEffectPresentationService::Seek_WorldRoot(
	const EFFECT_WORLD_ROOT_HANDLE Handle,
	const f32_t fSampleTimeSeconds,
	const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER& TransformProvider,
	const bool_t bRebuildHistory,
	const f32_t fPlaybackEndSeconds)
{
	if (!Handle.Is_Valid() || !std::isfinite(fSampleTimeSeconds) ||
		fSampleTimeSeconds < 0.f || !std::isfinite(fPlaybackEndSeconds) || fPlaybackEndSeconds < 0.f)
	{
		return false;
	}
	for (PENDING_EFFECT_SPAWN& pending : g_PendingEffectSpawns)
	{
		if (pending.Desc.iWorldRootHandle == Handle.iValue)
		{
			if ((TransformProvider || fPlaybackEndSeconds > 0.f) && !pending.Desc.bExternallySampled) return false;
			pending.Desc.fExternalPlaybackEndSeconds = fPlaybackEndSeconds;
			pending.Desc.fInitialSampleTimeSeconds = fSampleTimeSeconds;
			pending.Desc.ExternalTransformProvider = TransformProvider;
			return true;
		}
	}
	for (ACTIVE_EFFECT& effect : g_ActiveEffects)
	{
		if (effect.iWorldRootHandle == Handle.iValue && nullptr != effect.pObject)
		{
			if ((TransformProvider || fPlaybackEndSeconds > 0.f) && !effect.bExternallySampled) return false;
			effect.pObject->Set_ScreenPostPlaybackEnd(fPlaybackEndSeconds);
			effect.fPendingInitialSampleTimeSeconds = fSampleTimeSeconds;
			effect.fElapsedCueTimeSeconds = fSampleTimeSeconds;
			effect.bPendingInitialSeek = true;
			effect.ExternalTransformProvider = TransformProvider;
			if (bRebuildHistory || !TransformProvider) effect.bExternalHistorySampled = false;
			return true;
		}
	}
	return false;
}
```

`Client/Private/Effect_DocumentRenderer_Rendering.cpp`

```cpp
HRESULT Client::CEffectDocumentRenderer::Build_NativeScreenPost(
    const EFFECT_EVALUATED_FRAME& Frame, const EFFECT_EVALUATED_SCREEN_POST& Evaluated,
    std::shared_ptr<const Engine::IPresentationScreenPostMaterial>& OutMaterial,
    std::string& strOutError)
{
    OutMaterial.reset();
    if (!Evaluated.pElement) { strOutError="Native screen-post has no source element."; return E_INVALIDARG; }
    const auto& Element=*Evaluated.pElement;
    if (Evaluated.eProfile == EFFECT_SCREEN_POST_PROFILE::SCENE_COLLAPSE_CAPTURE_V1 ||
        Evaluated.eProfile == EFFECT_SCREEN_POST_PROFILE::SCENE_CAPTURE_CUBE_V1)
    {
        if (!m_pNativeScreenPostShader || !std::isfinite(Evaluated.fNormalizedLife))
        { strOutError = "Scene image capture has no prepared shader or valid time."; return E_FAIL; }
        auto& capture = m_ScreenPostCaptures[Element.strElementId];
        const auto& timing = Element.Detail.Timing;
        if (!capture || capture->fStartSeconds != timing.fStartDelaySeconds ||
            capture->fDurationSeconds != timing.fLifeTimeSeconds)
        {
            capture = std::make_shared<EFFECT_SCENE_CAPTURE_STATE>();
            capture->pDevice = m_pDevice; capture->pContext = m_pContext;
            capture->fStartSeconds = timing.fStartDelaySeconds;
            capture->fDurationSeconds = timing.fLifeTimeSeconds;
        }
        EFFECT_NATIVE_SCREEN_POST_SNAPSHOT snapshot;
        snapshot.pShader = m_pNativeScreenPostShader;
        snapshot.bSceneCollapse = true;
        const float shrinkSeconds = Element.Detail.ScreenPost.fCaptureShrinkSeconds;
        snapshot.fCaptureProgress = shrinkSeconds > 0.f ?
            std::clamp((Frame.fSampleTimeSeconds - timing.fStartDelaySeconds) / shrinkSeconds, 0.f, 1.f) :
            std::clamp(Evaluated.fNormalizedLife, 0.f, 1.f);
        if (Evaluated.eProfile == EFFECT_SCREEN_POST_PROFILE::SCENE_COLLAPSE_CAPTURE_V1 &&
            m_fScreenPostPlaybackEndSeconds > 0.f)
        {
            // An externally trimmed box must finish shrinking before its owner removes it.
            // Retiming the image keeps its frozen capture and every particle's authored clock.
            const float availableSeconds = m_fScreenPostPlaybackEndSeconds - timing.fStartDelaySeconds;
            if (!std::isfinite(availableSeconds) || availableSeconds <= 0.f)
            { strOutError = "Scene collapse starts at or after its owning Effect box ends."; return E_INVALIDARG; }
            const float effectiveShrinkSeconds = (std::min)(availableSeconds,
                shrinkSeconds > 0.f ? shrinkSeconds : timing.fLifeTimeSeconds);
            if (!std::isfinite(effectiveShrinkSeconds) || effectiveShrinkSeconds <= 0.f)
            { strOutError = "Scene collapse has no valid shrink interval."; return E_INVALIDARG; }
            snapshot.fCaptureProgress = std::clamp(
                (Frame.fSampleTimeSeconds - timing.fStartDelaySeconds) / effectiveShrinkSeconds, 0.f, 1.f);
        }
        const auto* view = Engine::CGameInstance::Get().Get_Transform(D3DTS::VIEW);
        const auto* projection = Engine::CGameInstance::Get().Get_Transform(D3DTS::PROJ);
        if (view && projection)
        {
            float4_t clip{};
            XMStoreFloat4(&clip, XMVector4Transform(XMLoadFloat4x4(&Evaluated.SourceWorld).r[3],
                XMLoadFloat4x4(view) * XMLoadFloat4x4(projection)));
            if (std::isfinite(clip.x) && std::isfinite(clip.y) && std::isfinite(clip.w) && clip.w > 1.e-5f)
                snapshot.vCaptureDestinationUV = {.5f + .5f * clip.x / clip.w, .5f - .5f * clip.y / clip.w};
        }
        snapshot.pCapture = capture;
        snapshot.bCaptureAllowed = m_bScreenPostCaptureAllowed;
        snapshot.bCaptureOverLiveScene = Evaluated.eProfile == EFFECT_SCREEN_POST_PROFILE::SCENE_CAPTURE_CUBE_V1;
        if (snapshot.bCaptureOverLiveScene)
        {
            if (!Try_ProjectCaptureTargetBounds(Frame, Element.Detail.ScreenPost.strCaptureTargetModelCueId,
                timing.fStartDelaySeconds + timing.fLifeTimeSeconds,
                snapshot.vCaptureDestinationUV, snapshot.vCaptureDestinationSizeUV))
            { strOutError = "Screen capture cube first-pose bounds could not be projected."; return E_FAIL; }
            // Stage froze the completed world frame before the action camera moved.
            // Recapturing here would replace it with the cinematic camera's first frame.
            if (!capture->pColor || !capture->pBloom)
            {
                if (!m_pStartingSceneCapture || !m_pStartingSceneBloomCapture)
                { strOutError = "Screen capture cube has no completed starting scene pair."; return E_FAIL; }
                capture->pColor = m_pStartingSceneCapture;
                capture->pBloom = m_pStartingSceneBloomCapture;
                capture->hLastResult = S_OK;
            }
            // Keep the requested screen-centered shrink independent of camera/root motion.
            // The target model supplies the ending size, not a drifting screen position.
            snapshot.vCaptureDestinationUV = {.5f, .5f};
            capture->vDestinationUV = snapshot.vCaptureDestinationUV;
        }
        OutMaterial = std::make_shared<CEffectNativeScreenPostMaterial>(std::move(snapshot));
        return S_OK;
    }
    const auto& Source=Element.Material.SourceMaterial;
    const auto* V=Find_DimensionMasterVProgram(Source.strRuntimeShaderProfileId);
    const auto* ALTV=Find_DimensionMasterALTVProgram(Source.strRuntimeShaderProfileId);
    const auto* Warlord=Find_WarlordNativeProgram(Source.strRuntimeShaderProfileId);
    const auto* Artist=Find_ArtistProgram(Source.strRuntimeShaderProfileId);
    const auto* Lance=Find_LanceMasterVAProgram(Source.strRuntimeShaderProfileId);
    if (!V && !ALTV && !Warlord && !Artist && !Lance) return S_FALSE;
    const bool Valid=(V && V->bScreenPost && Has_DimensionMasterVMaterialContract(Element)) ||
        (ALTV && ALTV->strRendererShape=="screenPost" && Has_DimensionMasterALTVMaterialContract(Element)) ||
        (Warlord && Warlord->strRendererShape=="screenPost" && Has_WarlordNativeMaterialContract(Element)) ||
        (Artist && Artist->strRendererShape=="screenPost" && Has_ArtistMaterialContract(Element)) ||
        (Lance && Lance->strRendererShape=="screenPost" && Has_LanceMasterVAMaterialContract(Element));
    const auto* Resource=Find_Resource(Element.strElementId);
    if (!Valid || !Resource || !m_pNativeScreenPostShader ||
        !Is_NativeScreenPostShaderProfile(Resource->iSourceMaterialProfile))
    { strOutError="Native screen-post material inputs are not prepared: "+Element.strElementId; return E_INVALIDARG; }
    EFFECT_NATIVE_SCREEN_POST_SNAPSHOT Snapshot;
    Snapshot.pShader=m_pNativeScreenPostShader;
    Snapshot.SourceTextures=Resource->SourceTextures;
    for (auto& Texture : Snapshot.SourceTextures) if (!Texture) Texture=m_pBlackTexture;
    Snapshot.Parameters=Lance ? Resource->LanceVASourceMaterialParameters : Artist ? Resource->ArtistSourceMaterialParameters :
        ALTV ? Resource->ALTVSourceMaterialParameters : Resource->VSourceMaterialParameters;
    Snapshot.iProfile=Resource->iSourceMaterialProfile;
    Snapshot.iTextureMask=Resource->iSourceTextureMask;
    Snapshot.iClampUMask=Resource->iSourceTextureClampUMask;
    Snapshot.iClampVMask=Resource->iSourceTextureClampVMask;
    Snapshot.vSourceColor=Evaluated.vSourceColor;
    Snapshot.vDynamicParameter=Evaluated.vSourceDynamicParameter;
    Snapshot.fLocalTimeSeconds=Evaluated.fSampleTimeSeconds;
    Snapshot.fBloomIntensity=Get_BloomIntensity();
    const auto* View=Engine::CGameInstance::Get().Get_Transform(D3DTS::VIEW);
    const auto* Projection=Engine::CGameInstance::Get().Get_Transform(D3DTS::PROJ);
    const auto* ViewInverse=Engine::CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW);
    if (!View || !Projection || !ViewInverse)
    { strOutError="Native screen-post has no active scene camera."; return E_FAIL; }
    vector_t Position=XMLoadFloat4x4(&Evaluated.SourceWorld).r[3];
    const vector_t ToCamera=XMLoadFloat4x4(ViewInverse).r[3]-Position;
    const float Distance=XMVectorGetX(XMVector3Length(ToCamera));
    if (!std::isfinite(Distance) || !std::isfinite(Evaluated.fSourceCameraOffset))
    { strOutError="Native screen-post camera offset is invalid."; return E_INVALIDARG; }
    if (Distance>1.e-6f) Position+=ToCamera*(Evaluated.fSourceCameraOffset/Distance);
    Position=XMVectorSetW(Position,1.f);
    const vector_t Clip=XMVector4Transform(Position,
        XMLoadFloat4x4(View)*XMLoadFloat4x4(Projection));
    Snapshot.fProjectionW=XMVectorGetW(Clip);
    if (!std::isfinite(Snapshot.fProjectionW) ||
        !std::isfinite(Snapshot.fLocalTimeSeconds) || Snapshot.fLocalTimeSeconds<0.f)
    { strOutError="Native screen-post source sample is invalid."; return E_INVALIDARG; }
    auto Candidate=std::make_shared<CEffectNativeScreenPostMaterial>(std::move(Snapshot));
    OutMaterial=std::move(Candidate);
    return S_OK;
}
```

`Client/Private/Effect_Object.cpp`

```cpp
void Client::CEffectObject::Set_ScreenPostPlaybackEnd(const f32_t endSeconds)
{
	if (m_pRenderer) m_pRenderer->Set_ScreenPostPlaybackEnd(endSeconds);
}
```


## G15. Stage 공백 축소와 Animation Blend 연결 유지

`Client/Private/KoukuSaydonActionWorkbench.cpp`의 `Set_StageDuration`과
`Set_PatternStartOffset`은 Stage 길이 변경에 따라 뒤 애니메이션의 절대 시작을 이동하지만,
Pattern 절대 시간으로 저장된 `ANIMATION_BLEND` Logic은 이동하지 않는다. 이 때문에 기존에
유효했던 전환 박스가 두 pose owner의 경계를 벗어나면 전체 candidate 검증에서 거절된다.

파일 내부 `Retime_AnimationBlendWindows`는 변경 전 문서의 실제 blend resolver에서
연결된 source/target occurrence ID와 target pose 시작을 읽는다. candidate의 새 pose 시작을
계산하여 같은 target의 이동량만큼 blend 시작을 이동하고, 기존 resolver로 길이·겹침·정확히
한 경계와 동일한 source/target ID를 다시 확인한다. 실패 시 setter의 local candidate만 폐기한다.
일반 Logic, Effect, World 등 독립 lane의 절대 시간은 바꾸지 않는다. 새 저장 필드나 runtime 경로는 없다.

선택 Stage 상세의 `Fit Stage to Animation`은 현재 animation들의 마지막 끝을 계산하고 기존
`Set_StageDuration`에 제출한다. 빈 Stage와 이미 맞는 Stage는 비활성화한다. 기존 Duration 입력과
timeline Stage edge drag도 같은 setter를 사용하여 같은 검증과 retime을 받는다.

실제 저장 P36과 사용자가 walk를 0ms로 옮긴 candidate를 사용해 15,274→4,997ms 축소와
5개 blend의 stable pair를 확인한다. 음수 시간, 다른 pair, 다중 경계·겹침, animation 침범은
거절하고 미저장 draft와 generation을 보존한다. Source JSON은 직접 바꾸지 않는다.




### G15 변경 함수 코드

`Client/Private/KoukuSaydonActionWorkbench.cpp`

```cpp
	bool_t Retime_AnimationBlendWindows(
		const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
		const KOUKU_SAYDON_COMPOSITION_PATTERN& previous,
		KOUKU_SAYDON_COMPOSITION_PATTERN& candidate, std::string& status)
	{
		std::vector<KOUKU_SAYDON_ANIMATION_BLEND_WINDOW> previousWindows;
		if (!CKoukuSaydonCompositionDocument::Try_ResolveAnimationBlendWindows(
			document, previous, previousWindows, status)) return false;
		if (previousWindows.empty()) return true;
		// A blend belongs to the same two pose owners even when a Stage edge moves.
		std::unordered_map<std::string, std::uint64_t> poseStarts;
		std::uint64_t origin = 0u;
		for (const auto& stage : candidate.Stages)
		{
			const auto first = std::min_element(stage.AnimationOccurrences.begin(),
				stage.AnimationOccurrences.end(), [](const auto& a, const auto& b) {
					return a.iStartOffsetMs != b.iStartOffsetMs ? a.iStartOffsetMs < b.iStartOffsetMs :
						a.strOccurrenceId < b.strOccurrenceId;
				});
			for (const auto& animation : stage.AnimationOccurrences)
				poseStarts.emplace(animation.strOccurrenceId, origin +
					(animation.strOccurrenceId == first->strOccurrenceId ? 0u : animation.iStartOffsetMs));
			origin += stage.iDurationMs;
		}
		for (const auto& window : previousWindows)
		{
			const auto target = poseStarts.find(window.Target.strOccurrenceId);
			const auto box = std::find_if(candidate.LogicOccurrences.begin(), candidate.LogicOccurrences.end(),
				[&](const auto& row) { return row.strOccurrenceId == window.strLogicOccurrenceId; });
			if (target == poseStarts.end() || box == candidate.LogicOccurrences.end())
			{ status = "Animation blend retime lost its pose owner or Logic box."; return false; }
			const std::int64_t start = std::int64_t(window.iStartMs) +
				std::int64_t(target->second) - window.Target.iPoseStartMs;
			if (start < 0 || start + window.iDurationMs > MAX_EDITOR_TIME_MS)
			{ status = "Animation blend retime would leave the 0..600000 ms timeline."; return false; }
			box->iStartMs = static_cast<std::uint32_t>(start);
		}
		std::vector<KOUKU_SAYDON_ANIMATION_BLEND_WINDOW> nextWindows;
		if (!CKoukuSaydonCompositionDocument::Try_ResolveAnimationBlendWindows(
			document, candidate, nextWindows, status)) return false;
		for (const auto& previousWindow : previousWindows)
		{
			const auto next = std::find_if(nextWindows.begin(), nextWindows.end(),
				[&](const auto& window) { return window.strLogicOccurrenceId == previousWindow.strLogicOccurrenceId; });
			if (next == nextWindows.end() || next->Source.strOccurrenceId != previousWindow.Source.strOccurrenceId ||
				next->Target.strOccurrenceId != previousWindow.Target.strOccurrenceId)
			{ status = "Animation blend retime would change its consecutive pose owners."; return false; }
		}
		return true;
	}
```

```cpp
bool_t Client::CKoukuSaydonActionWorkbench::Set_PatternStartOffset(
	const std::string_view patternId,
	const std::uint32_t startOffsetMs,
	std::string& outStatus)
{
	auto candidate = m_Draft;
	auto* pattern = Find_Pattern(candidate, patternId);
	if (!pattern || pattern->Stages.empty() || pattern->Stages.front().AnimationOccurrences.empty())
	{
		outStatus = m_strStatus = "Pattern Start Offset needs an animation in the first Stage.";
		return false;
	}
	const auto previous = *pattern;
	auto& firstStage = pattern->Stages.front();
	const auto first = std::min_element(firstStage.AnimationOccurrences.begin(),
		firstStage.AnimationOccurrences.end(), [](const auto& a, const auto& b) {
			return a.iStartOffsetMs < b.iStartOffsetMs;
		});
	const std::int64_t delta = std::int64_t(startOffsetMs) - first->iStartOffsetMs;
	const std::int64_t stageDuration = std::int64_t(firstStage.iDurationMs) + delta;
	const std::int64_t patternDuration = std::int64_t(Pattern_DurationMs(*pattern)) + delta;
	if (startOffsetMs > MAX_EDITOR_TIME_MS || stageDuration < 1 ||
		stageDuration > MAX_EDITOR_TIME_MS || patternDuration > MAX_EDITOR_TIME_MS)
	{
		outStatus = m_strStatus = "Pattern Start Offset would exceed the 600-second timeline.";
		return false;
	}
	if (delta == 0)
	{
		outStatus = m_strStatus = "Pattern Start Offset is unchanged.";
		return true;
	}
	for (auto& animation : firstStage.AnimationOccurrences)
		animation.iStartOffsetMs = static_cast<std::uint32_t>(std::int64_t(animation.iStartOffsetMs) + delta);
	firstStage.iDurationMs = static_cast<std::uint32_t>(stageDuration);
	if (!Retime_AnimationBlendWindows(m_Draft, previous, *pattern, outStatus))
	{ outStatus = m_strStatus = "KoukuSaydon edit rejected; draft preserved: " + outStatus; return false; }
	Mark_Draft(candidate, *pattern);
	return Commit_Candidate(std::move(candidate),
		"Changed Pattern Start Offset and kept Animation Blend boxes with their transitions. Other lane times are preserved. Press Save.", outStatus);
}
```

```cpp
bool_t Client::CKoukuSaydonActionWorkbench::Set_StageDuration(
	const std::string_view patternId,
	const std::string_view stageId,
	const std::uint32_t durationMs,
	std::string& outStatus)
{
	KOUKU_SAYDON_COMPOSITION_DOCUMENT candidate = m_Draft;
	KOUKU_SAYDON_COMPOSITION_PATTERN* const pattern = Find_Pattern(candidate, patternId);
	KOUKU_SAYDON_COMPOSITION_STAGE* const stage = nullptr == pattern ?
		nullptr : Find_Stage(*pattern, stageId);
	if (nullptr == stage)
	{
		outStatus = "KoukuSaydon Stage duration target is absent.";
		return false;
	}
	const auto previous = *pattern;
	stage->iDurationMs = durationMs;
	if (!Retime_AnimationBlendWindows(m_Draft, previous, *pattern, outStatus))
	{ outStatus = m_strStatus = "KoukuSaydon edit rejected; draft preserved: " + outStatus; return false; }
	Mark_Draft(candidate, *pattern);
	return Commit_Candidate(std::move(candidate),
		"Changed Stage duration and kept Animation Blend boxes with their transitions. Press Save.", outStatus);
}
```

## G16. Parent 소유 관계 복구와 공백 없는 세이튼 패턴 배치

같은 폴더 오류는 Sequence Parent 두 개가 가리키는 타임라인의 folderId 누락이다.
실제 현재 codec과 새 Parent 생성 흐름의 Save/Reload를 확인하고, 정본의 P1→folder.2,
P4→folder.3만 복구한다. 같은 Gate/모델/target 검증과 손상 항목 보존은 유지한다.

최신 저장 Gameplay P36과 Sequence P8의 첫 Stage 공백10,277ms를 제거하고,
기존5개 Animation Blend의 source/target pair를 유지하며 같은 이동량을 반영한다.
Sequence P4에는 다음 stable occurrence ordinal을 사용하여 P8을23,698ms부터
30,375ms간 배치한다. P8의 새 Effect를 포함한 다른 사용자 lane/placement는 보존한다.

실제 실행 owner인 P4와 독립 실행 P36은 resetBossToSpawn=true로 저장하여 기존 World
placement 좌표(6.43,1.3,730)를 사용한다. Child reset을 지원하지 않는 현재 확장 계약에 따라
P8은false를 유지하며, 새로운 spawn/teleport runtime은 추가하지 않는다. Parent 시작 시
스폰 좌표를 적용하고 child 애니메이션은23,698ms에 시작한다.

두 JSON은 최신 hash를 확인하고 대상 field만 바꾼 candidate를 actual codec/Expand로
검증한 뒤 CAS로 적용한다. 재직렬화 왕복의 folderId·새 Effect 보존, 확장 actor/placement,
walk 시작·전체 길이·blend5개를 확인하고 root가 기존 domain publish와 Product Build를 수행한다.

## G17. Composition 게시 검증과 실제 World/Camera owner 계약 동기화

목표는 사용자가 저장한 World Object 이펙트와 110개 Camera Shot을 삭제하거나 변환하지 않고
기존 제품 소비자와 같은 규칙으로 Composition에 게시하는 것이다. 변경 파일은
`Tools/CompositionPipeline/composition_pipeline.py`와 해당 단위 검사 파일이다.

`_validate_world_sequence_effect_tracks`는 WorldSequenceDocument와 Map publisher가 이미 읽고
저장하는 optional `followObject`, `bone`을 검증한다. 전자는 JSON boolean, 후자는 빈 문자열을
허용하는 유효 UTF-8 문자열이며 최대 256 bytes, 제어 문자 금지다. 기존 `LEAF`/`GROUP`과 함께
실제 WorldSequencePlayer가 소비하는 `V1_EFFECT`도 허용한다. WorldSequencePlayer의 기존
object clock/본 샘플과 V1 Effect spawn 경로를 유지하므로 새 C++ 계약이나 런타임은 필요 없다.

`CAMERA_SHOT_MAX_COUNT`는 Arena reader와 Map publisher의 기존 한도인 128로 맞춘다.
나머지 Camera ID, sequence binding, pose, timing 검증을 유지하고 129개는 계속 거부한다.
검증 성공 여부와 무관하게 원본 입력을 변경하지 않는 것이 불변식이다.

종료 증거는 World Effect optional field의 정상/비정상 타입·UTF-8 경계 검사, Camera 128/129개
경계, 현재 두 owner 문서 전체 검증과 입력 보존, Python 컴파일 및 scoped diff 검사다.
전체 Composition 게시 결과는 최종 publish 로그에 별도로 기록한다.

## G19. Parent 타임라인 조회 확인 — 추가 UI 변경 미적용

G16의 폴더 연결 복구로 P4는 Gate-root의 단독 leaf에서 빠지고 기존 `1관문_통합_시퀀스 [Parent]` 행으로 선택된다. 현재 Render_PatternTree는 폴더 owner를 중복 leaf로 표시하지 않고 Parent 이름 클릭을 Select_Hierarchy → Select_PatternById로 연결한다. matched-PDB VM_READ에서 Sequence49/GATE1/P4/folder3와 빈 load error를 확인했다.

명시적인 중복 Sequence 항목을 추가하는 변경을 잠시 준비했으나, 사용자가 첨부 화면에서 기존 Parent를 직접 확인하고 접근 문제를 정정했다. 따라서 추가 UI 변경과 재빌드는 필요 없으며 해당 새 변경만 원복한다. G14~G18의 소스·데이터·설치본은 유지한다. 이 목록 함수는 발탄 패턴 검증을 실행하지 않는다. 사용자 확인은 목록 접근에 대한 확인이며 캡처 연출 전체의 visual PASS로 확대하지 않는다.

## G20. 1관문 연출의 첫 걷기 구간과 중앙 도착 위치 연결

사용자는 Sequence P8 `1관문_연출`의 걷기 애니메이션 동안 중앙으로 이동하도록 요청했고, 도착점은 옮기기 전 세이튼 스폰(-0.07,1.32,737.53)으로 확인했다. 현재 시작점은(6.43,1.3,730), 첫 walk 구간은4,997ms/1배속이다. 설치 모델의 해당 walk는 제자리 clip이므로 애니메이션 배속 변경만으로 이동하지 않는다.

XZ 이동거리9.947406697m를4.997초로 나눈 속도1.990675745m/s를 기존 BossMotion의 시작/끝 시간과 좌표로 정의한다. +Z forward의 yaw는-40.801213684도다. 기존 BossMotion은 지면 높이를 유지하는 계약이므로 양끝Y=1.3으로 두고 XZ의 목표를 정확히 적용한다. 실제 ServerNavigation의 시작/끝/LOS와21개 지면 샘플이 통과했으며 지면 높이는1.2990~1.3176m다.

새 C++ 이동 경로는 만들지 않는다. P8과 대응 Gameplay P36에 기존 bossMotion을 넣고 서로 충돌하는 resetBossToSpawn을 해제한다. P8을 소비하는 Parent들은 기존 single complete-child 조건에 맞춰 참조 길이와 reset을 조정하고 전체 Parent 길이·다른 presentation/logic/world 자료는 보존한다. Parent 확장은 child 이동 시작/끝 시간을 실제 box 시작만큼 옮긴다. 이동 구간 전에는 시작점, 후에는 중앙에 고정되는 기존 계약으로 후속 연출을 진행한다. native 자동 XZ root motion은 해당 패턴에서 사용하지 않으며 원래 수직 pose는 유지한다.

Sequence의 미저장 편집을 발견하여 사용자 Save를 받은 뒤 matched-PDB VM_READ로 최신 revision51/clean 및 디스크 원문 일치를 확인했다. Gameplay revision561도clean이다. 이 최신 baseline에서 필요한 field만 CAS로 수정한다. actual codec/Parent expansion/motion sampler와 도착 시간·위치를 검증한 뒤 Kouku domain publisher를 수행한다. 데이터만 변경하므로 Product Build와 Client/UI 자동 실행은 하지 않는다. 사용자는 Stop → Reload Patterns 후 기존 Parent 또는 P8을 선택해 재생한다.

## G21. 캡처 경계의 반복 history seek 제거와 포탈/캡처 시계 분리

사용자는 캡처 순간 멈칫함, 짧은 축소, 종료 후 남은 왜곡을 마지막으로 수정하고 결과가 여전히 어색하면 축소 흡입을 제거하기로 했다. 이번 변경은 제거가 아니라 수정 시도다. 현재 사용자 저장 P4.presentation.36은15559~20080ms로, 내부 capture delay4.26초 이후에는261ms만 남는다. SceneCollapse는 별도 UV 왜곡을 만들지 않지만 SCENE_RESOLVE의 현재 굴절을 포함한 이미지를 복사하며, 다른 portal-arrival context의 입자 tail도 별도로 남는다.

PresentationPlayer는 capture hold 중 모든 V1 row의 bRebuildHistory를 매번true로 보내고 Commit_WorldRootCaptureSample도 무조건 전체 history seek를 수행한다. 동일 시각을 기다리는 동안 이력 재생을 반복하는 것이 불필요한 CPU 정지 구간이다. 기존 external update와 capture commit이 같은 internal history commit을 사용하도록 하고, 최초 sample/되감기/0.5초 초과 seek/명시적인 배치 변경에만 전체 seek를 유지한다. 연속 frame과 동일 시각 hold는 기존 Advance 경로를 사용한다. WORLD/animation을 먼저 샘플하는 순서와 첫 pass-through render, capture-ready 확인, 실패 제한은 그대로 유지한다. 새 C++ 파일과 runtime 경로는 추가하지 않는다.

기존 저작 계약으로 포탈 입자와 전용 화면 캡처를 분리한다. 원본 포탈 occurrence는19819ms에 종료한다. 전용 asset은 사용자가 조절한 capture 요소 하나만 복사하고 내부delay만0으로 바꾼다. 전용 V1_EFFECT occurrence는19819ms 시작/19959ms 종료의140ms 박스로 표시한다. 원본capture의lifetime·shrink·transform 값은 보존하고 기존 owner-end clamp로140ms에 맞춘다. 최종 사용자 지정은140ms 축소와 동시 페이드아웃이다. popup WORLD는19959ms에 시작하고300ms간 페이드인한다. 별도 P4 전용 V2 TexturedOverlay는19819ms부터440ms 동안 alpha0→140ms의1→440ms의0을 소비하며 기존 display-space 합성으로 캡처 결과와 Bloom을 함께 가린다. 캡처 shader나 Cube 전환은 변경하지 않는다. 별도 context는 전반 occurrence를19819ms까지로 제한하고, 후반48개 element의 stable ID·원문과 원래 local delay를 유지하는 P4 전용 variant를 같은start0에 연결한다. 전반72개 행은 전용 variant에서 제외하고 공유 원본은 보존한다. 숨긴 runtimeCarrier 요소를 남기면 기존 admission이 거부하므로 의존성 검사 후 필요한48행만 구성한다. 공유 원본 asset과 후반42초 이후 연출을 보존한다.

현재 Gameplay 새 미저장 편집이 발견되어 G20의 CAS 적용분만 원래561/52 bytes로 돌려 Save 기준본을 보존했다. 사용자 Save 이후 최신 source hash를 기준으로 G20/G21을 함께 stage/validate/CAS 적용한다. 실제 codec/element admission/시간 경계/history 호출 검증, 변경 C++ 최소 컴파일, JSON parse/diff-check와 필요한 owner 게시 후 EXE 종료가 확인되면 normal incremental Product Build를 수행한다. 최종 움직임과 연결 품질은 사용자가 직접 재생해 판단한다.

### G21-1. 정확한 표시 시각과 이력 유지

입자는 기존 고정 간격으로 계산하고, 화면·root·source anchor는 요청한 정확한 시각으로 평가한다. Update_WithTransformHistory는 모든 고정 간격 sample과 마지막 정확한 sample을 변경 전에 검증한다. 마지막 sample이 고정 간격과 일치하면 재사용하고, 사이에 있거나 동일 시각 재평가이면 provider를 한 번 호출한다. 성공한 뒤 기존 accumulator, 정확한 표시 시각과 source anchor dirty setter를 사용한다. 4.250→4.260초에서도 캡처 요소가 나타나므로 처음부터 particle history를 다시 만드는 우회가 필요 없다.

변경 파일은 기존 PresentationPlayer, Effect_PresentationService, Effect_Playback 세 CPP다. 새 파일·프로젝트 등록은 없으며 새 저작 JSON 세 개는 기존96.DataFiles의 None 항목과 실제 Effect resource catalog/tree에만 등록한다. 효과 JSON은 정본을 직접 소비하고 별도 runtime 사본으로 게시하지 않는다.

`Client/Private/Effect_PresentationService.cpp`의 변경 함수 전체 코드:

```cpp
bool_t Commit_ExternalTransformHistorySample(
		ACTIVE_EFFECT& Effect, std::string& strOutError)
	{
		const f32_t fTarget = std::clamp(Effect.fPendingInitialSampleTimeSeconds,
			0.f, Effect.pObject->Get_PreviewDurationSeconds());
		const f64_t fClock = Effect.pObject->Get_PreviewFixedStepClockSeconds();
		const f64_t fDelta = static_cast<f64_t>(fTarget) - fClock;
		// Rebuild on first sample, rewind, large seek, or an authored edit.
		// A held capture frame keeps the already committed history and anchors.
		const bool_t bSeek = !Effect.bExternalHistorySampled ||
			!std::isfinite(fClock) || fDelta < -0.000001 || fDelta > 0.5;
		const bool_t bCommitted = bSeek ?
			Effect.pObject->Set_SampleTimeWithTransformHistory(fTarget,
				Effect.ExternalTransformProvider, strOutError) :
			Effect.pObject->Advance_PreviewWithTransformHistory(
				static_cast<f32_t>((std::max)(0.0, fDelta)),
				Effect.ExternalTransformProvider, strOutError);
		if (!bCommitted) return false;
		Effect.bExternalHistorySampled = true;
		Effect.bPendingInitialSeek = false;
		Effect.fElapsedCueTimeSeconds = fTarget;
		return true;
	}
```

`Client/Private/Effect_PresentationService.cpp`의 변경 함수 전체 코드:

```cpp
HRESULT Client::CEffectPresentationService::Commit_WorldRootCaptureSample(
	const EFFECT_WORLD_ROOT_HANDLE Handle)
{
	if (!Handle.Is_Valid()) return E_INVALIDARG;
	const auto effect = std::find_if(g_ActiveEffects.begin(), g_ActiveEffects.end(),
		[Handle](const ACTIVE_EFFECT& value) { return value.iWorldRootHandle == Handle.iValue; });
	if (effect == g_ActiveEffects.end())
		return std::any_of(g_PendingEffectSpawns.begin(), g_PendingEffectSpawns.end(),
			[Handle](const PENDING_EFFECT_SPAWN& value) { return value.Desc.iWorldRootHandle == Handle.iValue; }) ? S_FALSE : E_FAIL;
	if (!effect->pObject || !effect->bExternallySampled || effect->bFollowAnchorMissing ||
		effect->pObject->Is_RenderFailureIsolated())
	{
		g_strStatus = "Scene capture sample needs a live externally sampled Effect handle.";
		return E_FAIL;
	}
	if (!effect->bPendingInitialSeek) return S_OK;
	if (!effect->ExternalTransformProvider)
	{
		g_strStatus = "Scene capture sample needs its admitted transform-history provider.";
		return E_INVALIDARG;
	}
	std::string historyError;
	// MainApp's final preview sample follows the service's normal update.
	// Commit that exact sample before FrameProviders build this render frame.
	if (!Commit_ExternalTransformHistorySample(*effect, historyError))
	{
		g_strStatus = "Scene capture transform-history sample failed: " + historyError;
		return E_FAIL;
	}
	return S_OK;
}
```

`Client/Private/Effect_Playback.cpp`의 변경 함수 전체 코드:

```cpp
bool_t Client::CEffectPlayback::Update_WithTransformHistory(
	const f32_t fTimeDelta,
	const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER& TransformProvider,
	std::string& strOutError)
{
	Engine::CProfilerScope profile(
		CGameInstance::Get().Get_Profiler(), "Effect.Playback.HistoryUpdate");
	std::string GateStatus;
	if ((!m_bReconstructedSourceRuntimeActive &&
		 !m_bSourceVisualProgramActive &&
		 !m_ReconstructedRuntimeBoundary.Admit_Execution(GateStatus)) ||
		!std::isfinite(fTimeDelta) || fTimeDelta < 0.f)
	{
		strOutError = GateStatus.empty() ?
			"Effect transform-history update request is invalid." : GateStatus;
		return false;
	}

	const f64_t fPendingAccumulator =
		m_fAccumulatorSeconds + static_cast<f64_t>(fTimeDelta);
	if (!std::isfinite(fPendingAccumulator) || fPendingAccumulator < 0.0)
	{
		strOutError = "Effect transform-history accumulator is invalid.";
		return false;
	}
	/* This typed path must never claim animation-clock equality after applying
	   only part of a backlog.  The deferred Tool start absorbs cache-build wall
	   time; a later hitch larger than the bounded 60-step transaction is rejected
	   before any provider call or playback mutation. */
	if (fPendingAccumulator >
		(static_cast<f64_t>(MAX_CATCH_UP_STEPS) + 1.0) *
			FIXED_STEP_SECONDS_EXACT)
	{
		strOutError =
			"Effect transform-history update exceeds the 60-step catch-up transaction.";
		return false;
	}
	const uint64_t iAvailableSteps = static_cast<uint64_t>(std::floor(
		(fPendingAccumulator + FIXED_STEP_EPSILON) /
		FIXED_STEP_SECONDS_EXACT));
	if (iAvailableSteps > static_cast<uint64_t>(MAX_CATCH_UP_STEPS))
	{
		strOutError =
			"Effect transform-history update exceeds the 60-step catch-up transaction.";
		return false;
	}
	const uint32_t iStepCount = static_cast<uint32_t>(iAvailableSteps);
	std::vector<EFFECT_FIXED_STEP_TRANSFORM_SAMPLE> Samples;
	Samples.reserve(iStepCount);
	for (uint32_t iStep = 0u; iStep < iStepCount; ++iStep)
	{
		const f32_t fStepTime = static_cast<f32_t>(
			static_cast<f64_t>(m_iSimulationStep + iStep + 1u) *
			FIXED_STEP_SECONDS_EXACT);
		EFFECT_FIXED_STEP_TRANSFORM_SAMPLE Sample;
		if (!Collect_TransformHistorySample(
				fStepTime, TransformProvider, Sample, strOutError))
		{
			return false;
		}
		Samples.push_back(std::move(Sample));
	}

	const f32_t fTargetSampleTime = static_cast<f32_t>(
		static_cast<f64_t>(m_iSimulationStep) * FIXED_STEP_SECONDS_EXACT +
		fPendingAccumulator);
	const f64_t fSteppedTime = static_cast<f64_t>(m_iSimulationStep + iStepCount) *
		FIXED_STEP_SECONDS_EXACT;
	EFFECT_FIXED_STEP_TRANSFORM_SAMPLE FinalSample;
	if (!Samples.empty() &&
		std::abs(static_cast<f64_t>(fTargetSampleTime) - fSteppedTime) <= FIXED_STEP_EPSILON)
	{
		FinalSample = Samples.back();
	}
	else if (!Collect_TransformHistorySample(
		fTargetSampleTime, TransformProvider, FinalSample, strOutError))
	{
		return false;
	}

	if (m_bShowtimeBurstPresentation && fTimeDelta > 0.f)
    {
        m_MissedShowtimeBursts.clear();
        m_bFrameInputsDirty = true;
    }
	f64_t fCommittedAccumulator = fPendingAccumulator;
	float4x4_t FinalRoot = m_Frame.RootWorld;
	for (EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& Sample : Samples)
	{
		m_SourceAnchorWorlds = std::move(Sample.SourceAnchorWorlds);
		FinalRoot = Sample.RootWorld;
		if (!Step(FIXED_STEP_SECONDS, FinalRoot))
		{
			strOutError = m_strSourceVisualProgramStatus;
			return false;
		}
		Capture_MissedShowtimeBursts(FinalRoot);
		fCommittedAccumulator = (std::max)(
			0.0, fCommittedAccumulator - FIXED_STEP_SECONDS_EXACT);
	}
	m_fAccumulatorSeconds = fCommittedAccumulator;
	m_fSampleTimeSeconds = fTargetSampleTime;
	Set_SourceAnchorWorlds(std::move(FinalSample.SourceAnchorWorlds));
	Rebuild_Frame(FinalSample.RootWorld);
	Append_MissedShowtimeBursts();
	strOutError.clear();
	return true;
}
```

## G22. 책·팝업 맵 재질과 광원의 기존 복원 연결 확인

현재 설치37개 WModel의136개 배치,40개 material slot/41개 asset-material을 actual source/runtime descriptor와 대조한다. 책3개slot의 World mapMaterialBindings, D/N/S와 선택된 shader 분기, 기존7개 movable light와32개 popup MapLight 사본의 source 원문·위치·enabled 보존을 확인한다. 새로 누락된 입력이 없으면 동일 자산을 다시 만들거나 임의 강도·반사 값을 적용하지 않는다. 이번 타임라인 변경과 함께 WORLD/camera/profile 및 .33조명/.34재질 carrier의 시작을19959ms popup에 맞춘다. 사용자가 편집한122개 MapLight 정본의 다른 값은 보존한다.


## G23. 팝업 구간 잔류 표시와 스포트라이트 감쇠 수정

사용자가 제공한 20244ms 화면에서 그네 두 개, 중앙 형상, 붉은 평면을 관찰했다. 후속 20067ms UV 잔류 요청을 최우선으로 처리한다. G21 후보를 아직 적용하지 않은 현재 Sequence54에는 full portal과 full context가 함께 있고, portal 종료20080ms와 popup 시작21010ms 사이가 비어 있다. 암전 프로필로 살아 있는 왜곡을 덮지 않는다. 원본 context의 전반 왜곡과 후반 요소를 기존 V1 문서로 분리하고, capture140ms와 popup 시작을 연속으로 연결한다. 20.067/20.081/20.5/21.010초에서 실제 Playback frame에 남는 element ID를 전후 대조한다.

`Client/Private/MapPlacementRuntime.cpp`의 `Sample_SelfMotions`는 batch transform 갱신 시 저장 배치의 visible 값을 복사했다. 이를 현재 runtime 가시성으로 보존해 Level/WorldSequence가 숨긴 그네가 회전 갱신 때문에 다시 나타나지 않게 한다. 원본 배치와 정상 모션을 삭제하지 않는다. 중앙 형상은 실제 owner가 확인된 뒤 동일한 경계에서 처리하며, Pattern 소유 Saydon과 임의의 다른 Preview 배우를 삭제하지 않는다.

`effect.kouku.gate1.popup.source-material-carriers`의 actor811은 `plan02 / scene_a.mat.white_t` 섬광용 평면이다. 원본 op0.3과 붉은 color의 첫 키가 긴 pre-roll에 평가된다. 원본 이동/섬광 시작 시점까지 해당 요소의 활성 구간을 제한하고 후반 섬광, 다른5개 carrier와 원래 source clock은 보존한다. 실제 codec 저장 왕복 및 시간별 frame으로 확인한다.

`Data/Rendering/Authored/RenderingProfiles.json`에는 P4 전용 `scene.kakulsaydon.g1.spotlight-stage.v1`을 추가한다. 기존 spider-blackout의 방향광·ambient0과 MapLight multiplier0을 유지하고 exposureMultiplier를1, fog.enabled를false로 둔다. P4만 새 Scene Profile 정의를 참조한다. `Collect_FrameLights → CLightResourceCatalog::Try_BuildAnchoredLightDesc → FRAME_LIGHT_PROVIDER`는 MapLight multiplier와 별개로 LIGHT lane을 제출하므로 저작 스포트라이트는 켜지고, 기존 맵 배경광은 계속 억제된다. 다른 Pattern이 사용하는 spider-blackout 원문은 보존한다. 전체 rendering catalog를 정상 publisher로 Validate/Publish한다.

재질은 파일/slot 존재 감사에서 끝내지 않고 원본 MIC static parameter set과 native geometry를 대조한다. 원본책의 material 이름이 floor인 사실만으로 잘못된 재질로 판정하지 않는다. 원본에서 선택하지 않은 bump/reflection을 임의로 켜지 않는다. 기존 사용자 승인 F1 표면 치환, 원본 SH·환경광의 미지원 부분, 실제 입력 손실을 구분해 결과에 기록한다. 자료에서 확정하지 못한 원작 동등성을 완료로 기록하지 않는다.

저장본의 이후 편집은 최신 source SHA와 revision으로 보존한다. source 수정, authoring 후보, 실제 적용, Product 빌드, 사용자 화면 확인을 서로 구분하며 실행 중 EXE를 자동 종료하거나 UI를 조작하지 않는다.


## G24. 미적용 후보의 실제 설치와 레거시 연출 호출 제거

이전 G23 후보의 sourceWritten=false 상태를 최신 디스크와 대조한다. 삭제된 P4.presentation.20은 직전 저장 원문과 resource ID를 검증하여 작업용 bytes에서 복원하고, 같은 후보 안에서 전반 소유를19819ms에 끝낸다. Sequence의 최신 revision55와 Gameplay의 최신 사용자 편집을 기준으로 G20/G21/G23을 재생성하며 이전 Gameplay579 또는606 문서 전체를 덮지 않는다. 기존13파일의 before/after SHA와 검증 manifest를 묶고 실제 Client/Server 종료 상태를 확인한 CAS로 설치한다. 종료된 process의 메모리 clean-save 결과를 임의로 만들어 기록하지 않는다.

`Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json`의2-1Stage_Move만 비활성화한다. `CLevel_KakulSaydonArena`의 기존 Deploy5 팝업북 직접 생성 함수를 제거하고, `CGameRoom`의 world sequence 방송 및 실제 trigger callback이 unowned original_kouku PLAY/REPLAY를 실패로 처리하도록 연결한다. 이 거절을 정상 Pattern preview 중지나 파티 이동보다 먼저 평가한다. Pattern-owned run epoch와 original_* 맵·책 WORLD 자료는 보존한다. 중앙 이동은 P8/P36 bossMotion과 기존 Server 경로를 사용하고 임의 Client Transform 이동을 추가하지 않는다.

공 낙하 Open Editor의 ImGui assertion은 연결된 그룹 입력창까지 같은 원인으로 조사한다. 현재 ImGui InputScalar가 금지하는 EnterReturnsTrue를 `Effect_Tool_Detail.cpp`의 Group Center·감전빔 도착점과 `EffectAuthoringSequencer.cpp`의 World Position/Yaw 네 곳에서 제거한다. 값 변경 반환으로 기존 stage/commit을 실행하고 설명문을 즉시 편집 방식에 맞춘다. 문자열 이름 변경 InputText는 기존 Enter 동작을 유지한다.

설치 뒤 Composition·Rendering·Kouku gameplay의 공식 publisher를 실행하고 Product Debug EXE를 빌드한다. legacy 거절의 실제 Server 소비자 검사와 JSON/XML parse, 변경 범위 diff 검사를 수행한다. 사용자 UI 실행·캡처·최종 시각 판정은 수행하지 않는다. 공 낙하의 원본 크기·낙하 연기 복원은 해당 Effect 계획/결과에서 원본 단위와 실제 낙하 샘플로 설명한다.


## G27. Effect 그룹 중심을 기준으로 XYZ 회전 편집

`CKoukuSaydonActionWorkbench::Render_EffectGroupDetails`의 Group Center Offset 옆에 XYZ degree 입력을 추가한다. 선택한 그룹의 기존 평균 pivot을 중심으로 모든 occurrence 위치와 방향에 같은 delta quaternion을 적용한다. 기존 `XMMatrixRotationRollPitchYaw` 소비 순서를 유지하고 DirectXTK quaternion의 Euler 변환을 재사용한다. 비균일 Scale, anchor, 시간, stable ID와 그룹 상대 형상은 보존한다. 선택이 바뀌면 회전 입력의 상대 기준을 0으로 시작한다.

기존 `Collect_EffectPlacements`의 그룹 확장·좌표계 검사와 전체 geometry 검증, staged preview 및 Save 경로를 그대로 사용한다. 잘못된 입력이나 범위 초과는 전부 반영 전에 거절한다. 회전 없이 중심만 이동할 때 기존 Euler 값을 다시 쓰지 않는다. Collider와 Save/Commit_Candidate 구현은 수정하지 않는다. 현재 실행 중인 Client의 미저장 편집, Composition/Resources와 게시 산출물에는 접근하지 않는다.

변경된 실제 transform 함수의 수치 실행으로 정사각형 중심·변 길이, XYZ 기존 방향과 비균일 크기, 역회전, gimbal 근처 및 거절 시 전체 보존을 확인한다. 실제 Workbench TU만 out에 분리 컴파일하며 제품 재빌드·실행·UI 조작과 시각 판정은 하지 않는다. 새 production 파일이나 프로젝트 등록은 없다.


## G28-P. SHOWTIME_PLAYER_TARGETS 게시 투영

DURATION 정의의 `fixedSelectionGroupId`, `trackingPresentationOccurrenceId`, `spawnIntervalMs`, `followSpeedScale`을 Python projector와 Gameplay publisher에 연결한다. 미완료 저작 참조는 보존하고 Product 승격 시 같은 Pattern의 EFFECT 그룹/occurrence를 resolve한다. 하나 또는 둘을 선택할 수 있고 둘 다 없으면 거절한다. interval은1~600000ms, speed는0.01~10이다.

선택된 MAP Effect 행의 원본 시간 차이, TRS, fade/dissolve 및 resourceDurationMs를 기존 serializer로 유지한다. fixed의 가장 이른 marker와 tracking 자신의 XZ만 상대화하고 Y는 원래 지면 기준 offset을 유지한다. 고정은 원본 그룹의 마지막 종료까지, 추적은 원본 duration 반복으로 템플릿을 만든다. 원본 selectionGroup과 행은 저작에 보존하며 Product의 static playback 목록에서 해당 template 행만 제외한다. 내부 stable ID 정규화 후 내용 hash를 visual ID로 사용하고 patternbindings root에서 중복을 제거한다. Parent 반복은 해당 그룹 참조만 scope remap하고 잘린 원본 template를 정상값처럼 게시하지 않는다.

Encounter Pattern의 optional showtimeTargets를 기존 Server bootstrap의 11-field PATTERNSHOWTIMETARGETS로 게시한다. 같은 revision의 patternbindings template ID/archetype/duration/loop를 exact-join하고 빈 ID는 `-`, fixed 없음의 lifetime은0을 사용한다. 실제 Python fixture와 PowerShell 기존 publisher 본문 추출 검사로 양쪽 계약, 참조 오류, 범위, 중복, 이전 출력 보존을 확인한다. canonical JSON, publish와 EXE는 이 작업에서 변경하지 않는다.

World placement admission의 `Publish-WorldGameplay.ps1::Get-EncounterProfiles`도 Kouku Pattern의 optional showtimeTargets 배열(최대64행)을 받아야 한다. 값·template join과 Server TSV 소유는 Gameplay publisher에 유지하고, 다른 Encounter나 미지의 필드는 계속 거절한다. 실제 Owner 게시에서 드러난 이 누락을 동일 변경으로 닫는다.


## G29. Stage 길이 편집에서 이펙트와 비애니메이션 시간 보존

사용자의 “이펙트 시간 그대로” 지시에 따라 이전 모든 lane 압축 정책을 폐기한다. `Set_StageDuration`은 선택 Stage 길이와 그 안의 animation playback window만 제한한다. animation source start/end, play rate는 보존하며 Logic/Summon/World/Scene Profile/Presentation/Pattern occurrence와 BossMotion은 시작·길이·fade·TRS·재질·그룹·연결 ID를 포함해 수정하지 않는다. ANIMATION_BLEND도 절대 시간을 움직이지 않으며 새 pose 경계와 맞지 않으면 기존 검증으로 candidate 전체를 거절한다.

Stage 합보다 늦게 끝나는 저작 행이 있으면 기존 explicit Pattern duration을 사용해 마지막 행까지 수명을 연장한다. 이미 명시된 수명은 축소하지 않는다. `Mark_Draft → Commit_Candidate`의 검증 및 Save freshness를 유지하고 현재 사용자 JSON은 직접 설치하지 않는다. Workbench CPP의 시간 helper, Set_StageDuration, Stage Details 설명 및 실제 leaf tail 소비자만 수정한다. 새 C++ 파일·header·project 등록은 없다.

현재 revision856 out 사본에서 여러 Stage 길이와 모든 비애니메이션 행의 전체 구조 동일성, source animation window 보존, candidate 실패 시 원문 보존, codec Save/Reload/freshness 및 실제 투영을 검사한다. 변경 TU를 out로 격리 컴파일하며 Product 빌드·배포·Client/UI 실행은 하지 않는다.

## G30-P. 빈 마지막 Stage와 explicit lifetime의 마지막 pose hold 게시

기존 leaf의 마지막 빈 Stage는 직전 단일 animation의 끝 pose를 유지한다. Stage 축소로 explicit Pattern lifetime이 animation Stage 합보다 길어질 때도 같은 정책을 사용한다. Preview의 파생 사본은 마지막 Stage 수명만 확장하고, publisher 사본은 같은 stageKind·retarget 없는 연속 빈 tail을 직전 단일 clip Stage로 합친다. 원본 clip·source window·playMs·rate 및 모든 비애니메이션 행과 source Stage ID는 저장 원본에서 유지한다.

기존 implicit clock에서는 합치기 전후 30Hz tick 수 일치 조건을 유지한다. explicit duration은 기존 fixedTimeline 소비자와 절대 Stage 경계를 사용하므로 별도 Stage별 ceil tick 합을 요구하지 않는다. Parent의 idle 정책과 내부 빈 구간은 그대로 두며 지원하지 않는 clip 구조를 임의 애니메이션으로 대체하지 않는다. actual current Pattern의 prepare_publication, presentation duration, targeted templates와 끝 pose/root-motion hold를 확인한다.


## G31. Preview Effect 준비 실패의 terminal 정산과 대기 문구

런타임 worker의 structural failure는 소유 target이 FIFO front에 고정되어 있을 때 실패 receipt를 먼저 commit하고 owner를 해제한다. 이후 worker 종료 정리의 중복 실패는 기존 receipt를 유지한다. 실제 identity가 손상되어 commit할 수 없는 경우 기존 revision latch를 유지하되 target probe가 blocking failure를 전달하고 Preview의 기존 Fail_Preview 경로에서 시계를 중지하고 이유를 보존한다. 실패한 개별 asset과 누락 target의 기존 terminal isolation, catalog revision rebase, Loading owner와 yield pacing은 유지한다.

Prepare_PreviewEffects는 준비 중 target 수를 표시하고 준비 완료 시 이전 held 문구만 교체한다. 다른 occurrence 실패 상태를 덮지 않는다. 현재 사용자 저작과 실행 프로세스는 변경하지 않는다. 실제 queue/worker failure 순서와 Preview 소비 함수로 준비→완료, 실패 격리, fatal 정체 종료와 revision 변경을 검사하고 변경 Client TU만 out에 컴파일한다. 제품 링크·실행과 화면 검증은 수행하지 않는다.

### G31 반영 코드

기존 worker·Preview 소유 함수의 전체 변경 분기는 다음과 같다. queue의 strict front commit과 MainApp failure 소비는 기존 구현을 재사용한다.

```diff
--- Client/Private/Effect_PresentationService.cpp.before
+++ Client/Private/Effect_PresentationService.cpp
@@ -474,6 +474,7 @@
    std::string g_RuntimePreparationFailureStatus;
    bool g_RuntimePreparationAbandoned = false;
    uint64_t g_RuntimePreparationFailureRevision = 0u;
+   std::string g_RuntimePreparationBlockingStatus;
    PRODUCT_PREPARATION_WORKER g_RuntimePreparationWorker;

    void Fail_RuntimePreparationTarget(const std::string& effectId,
@@ -497,7 +498,8 @@
        {
            // A broken queue identity must not turn into an unbounded retry every frame.
            g_RuntimePreparationFailureRevision = revision;
-           g_strStatus = status + " Failure receipt could not settle: " + commitStatus;
+           g_RuntimePreparationBlockingStatus = status + " Failure receipt could not settle: " + commitStatus;
+           g_strStatus = g_RuntimePreparationBlockingStatus;
        }
    }

@@ -2868,8 +2870,12 @@
 Client::CEffectPresentationService::Get_ProductCuePreparationProbe(
    const std::vector<std::string>& EffectAssetIds)
 {
-   return g_ProductPrewarmQueue.Get_TargetProbe(
-       EffectAssetIds, CEffectCatalog::Get_RuntimeRevision());
+   const uint64_t revision = CEffectCatalog::Get_RuntimeRevision();
+   auto probe = g_ProductPrewarmQueue.Get_TargetProbe(EffectAssetIds, revision);
+   if (revision != 0u && g_RuntimePreparationFailureRevision == revision &&
+       probe.bCatalogRevisionCurrent && probe.iPendingCount != 0u)
+       probe.strBlockingFailure = g_RuntimePreparationBlockingStatus;
+   return probe;
 }

 std::string Client::CEffectPresentationService::Get_ProductCuePreparationFailure(
@@ -3148,7 +3154,15 @@
           resources away from the UI frame. */
        Staged.reset();
        WorkerResult.pImmutablePayload.reset();
-       if (pJob == g_RuntimePreparationJob) g_RuntimePreparationFailureStatus = Status;
+       if (pJob == g_RuntimePreparationJob)
+       {
+           g_RuntimePreparationFailureStatus = Status;
+           // Settle the known target while its owner still pins the FIFO front.
+           // Releasing first lets next-frame priority enqueue move it behind another
+           // target before the cancelled worker finishes and its failure is consumed.
+           Fail_RuntimePreparationTarget(g_RuntimePreparationEffectId,
+               pJob->Get_CurrentCatalogRevision(), E_FAIL, Status);
+       }
        g_strStatus = Status;
        OutputDebugStringA(("[Client][EffectPresentation] " + Status + "\n").c_str());
        std::string ReleaseStatus;
@@ -3749,16 +3763,16 @@
    if (epoch == 0u || !job->Open(epoch, iCatalogRevision, status) ||
        !Begin_LoadingProductCuePreparation(job, epoch, {EffectId}, status))
    {
-       Cancel_LoadingProductCuePreparation(job, epoch);
        g_strStatus = "Product Effect preparation could not start: " + status;
        Fail_RuntimePreparationTarget(EffectId, iCatalogRevision, E_FAIL, g_strStatus);
+       Cancel_LoadingProductCuePreparation(job, epoch);
        return;
    }
    if (!g_RuntimePreparationWorker.Submit(pDevice, pContext, job))
    {
-       Cancel_LoadingProductCuePreparation(job, epoch);
        g_strStatus = "Product Effect preparation worker could not be started.";
        Fail_RuntimePreparationTarget(EffectId, iCatalogRevision, E_FAIL, g_strStatus);
+       Cancel_LoadingProductCuePreparation(job, epoch);
        return;
    }
    g_RuntimePreparationEpoch = epoch;
@@ -5890,6 +5904,7 @@
    g_RuntimePreparationJob.reset(); g_RuntimePreparationEpoch = 0u;
    g_RuntimePreparationEffectId.clear(); g_RuntimePreparationFailureStatus.clear(); g_RuntimePreparationAbandoned = false;
    g_RuntimePreparationFailureRevision = 0u;
+   g_RuntimePreparationBlockingStatus.clear();
     Release_ProductCamera(); g_ProductCameraCache.clear();
    g_ProductPrewarmQueue.Clear();
    g_ProductEffectBudgetCosts.clear();
--- Client/Private/KoukuSaydonPresentationPlayer.cpp.before
+++ Client/Private/KoukuSaydonPresentationPlayer.cpp
@@ -2929,8 +2929,24 @@
     const auto probe = CEffectPresentationService::Get_ProductCuePreparationProbe(m_PreviewPreparationTargets);
     // Failed targets are reported by their own occurrence. Preparation time
     // never consumes a short Effect window on the authoring clock.
-    if (probe.bCatalogRevisionCurrent && probe.bSettled) return true;
-    m_strStatus = "Preparing preview Effects; timeline is held at " + std::to_string(Preview_ClockMs()) + " ms.";
+    if (probe.bCatalogRevisionCurrent && probe.bSettled)
+    {
+        // Do not leave a held-clock message visible after this gate has opened,
+        // or overwrite a later occurrence-specific failure on every sample.
+        if (m_strStatus.starts_with("Preparing preview Effects;"))
+            m_strStatus = "Preview Effects prepared: " + std::to_string(probe.iPreparedCount) + " ready, " +
+                std::to_string(probe.iFailedCount) + " failed, " + std::to_string(probe.iUnavailableCount) + " unavailable.";
+        return true;
+    }
+    if (!probe.strBlockingFailure.empty())
+    {
+        Fail_Preview("Preview Effect preparation stopped: " + probe.strBlockingFailure);
+        return false;
+    }
+    m_strStatus = "Preparing preview Effects; " + std::to_string(probe.iTargetCount - probe.iPendingCount) +
+        "/" + std::to_string(probe.iTargetCount) + " settled (" + std::to_string(probe.iFailedCount) +
+        " failed, " + std::to_string(probe.iUnavailableCount) + " unavailable); timeline is held at " +
+        std::to_string(Preview_ClockMs()) + " ms.";
     return false;
 }

--- Client/Public/Effect_ProductPrewarmQueue.h.before
+++ Client/Public/Effect_ProductPrewarmQueue.h
@@ -44,6 +44,8 @@
    uint32_t iQueuePendingCount = 0u;
    bool bCatalogRevisionCurrent = false;
    bool bSettled = false;
+   // Service-level queue failure; individual failed targets remain terminal.
+   std::string strBlockingFailure;
 };

 struct EFFECT_PRODUCT_PREWARM_FAILURE_RECEIPT final

```


## G32-S. SHOWTIME duration에 맞춘 지정 타겟 회전 보간


`CGameRoom::Update_KoukuPlayerTargets`에서 기존 server-owned 지정 타겟 선택과 사망 시 재선택을 유지한다. 즉시 atan2 대입을 현재 yaw에서 목표 yaw까지의 최단 각도 보간으로 바꾼다. 보간 비율은 경과 server tick / 남은 duration tick이며 마지막 유효 tick에1이다. 정지 타겟은 저작 duration 동안 일정 각속도로 향하고 움직이는 타겟은 현재 방향에서 새 목표를 계속 따라간다. fixed30Hz와 기존 tick wrap helper를 사용해 호출 횟수나 Client frame rate에 의존하지 않는다. 같은/이전 tick은 중복 소비하지 않는다.

첫 활성 tick은 한 tick만큼 진행하며 duration 밖에서는 기존 exclusive-end 종료를 유지한다. 큰 Saydon의 기존-90도 basis, 시각 객체 MAP축과 타겟 이동속도는 변경하지 않는다. 새 yaw speed field·JSON·packet·runtime 경로는 없다. 실제 room을 쓰는 기존 KoukuSupportSurface 검사에 shortest arc, duration별 속도, tick 묶음/개별 일치, 마지막 tick·end, 이동 타겟과 중복/역행 tick 검증을 추가하고 필요한 서버 TU만 격리 컴파일한다.

### G32-S 반영 코드

```diff
--- Server/Private/GameRoom_BossSimulation.cpp.before
+++ Server/Private/GameRoom_BossSimulation.cpp
@@ -812,7 +812,9 @@
    for (auto& window : ledger.PlayerTargetWindows)
    {
        if (window.bClosed || window.iLastUpdateTick == serverTick || window.iTriggerIndex >= pattern.MechanicTriggers.size()) continue;
-       if (!Clock::Has_ReachedTick(serverTick, window.iStartTick)) continue;
+       if (!Clock::Has_ReachedTick(serverTick, window.iStartTick) ||
+           (window.iLastUpdateTick != 0u && !Clock::Has_ReachedTick(serverTick, window.iLastUpdateTick))) continue;
+       const std::uint32_t previousUpdateTick = window.iLastUpdateTick;
        window.iLastUpdateTick = serverTick;
        const auto& trigger = pattern.MechanicTriggers[window.iTriggerIndex];
        if (Clock::Has_ReachedTick(serverTick, window.iEndTick))
@@ -847,8 +849,21 @@
            const float dx = facingTarget->fPositionX - boss.fPositionX, dz = facingTarget->fPositionZ - boss.fPositionZ;
            if (std::isfinite(dx) && std::isfinite(dz) && dx * dx + dz * dz > .000001f)
            {
-               boss.fYawDegrees = std::atan2(dx, dz) * RADIANS_TO_DEGREES;
-               if (boss.strArchetypeId == "BOSS_KAKULSAYDON_G2_BIG_SAYDON") boss.fYawDegrees -= 90.f;
+               float targetYaw = std::atan2(dx, dz) * RADIANS_TO_DEGREES;
+               if (boss.strArchetypeId == "BOSS_KAKULSAYDON_G2_BIG_SAYDON") targetYaw -= 90.f;
+               // Each active fixed tick consumes one slice of the authored duration.
+               // Remaining-time interpolation preserves constant speed for a fixed
+               // target and reaches a moving target on the last tick without a snap at start.
+               const auto totalTicks = Elapsed_ServerTicksSkippingReservedZero(window.iStartTick, window.iEndTick);
+               const auto previousTicks = previousUpdateTick == 0u ? 0u :
+                   Elapsed_ServerTicksSkippingReservedZero(window.iStartTick, previousUpdateTick) + 1u;
+               const auto currentTicks = Elapsed_ServerTicksSkippingReservedZero(window.iStartTick, serverTick) + 1u;
+               if (std::isfinite(boss.fYawDegrees) && totalTicks > previousTicks && currentTicks > previousTicks)
+               {
+                   const double ratio = (std::min)(1.0, double(currentTicks - previousTicks) / double(totalTicks - previousTicks));
+                   const double turn = std::remainder(double(targetYaw) - boss.fYawDegrees, 360.0);
+                   boss.fYawDegrees = static_cast<float>(std::remainder(double(boss.fYawDegrees) + turn * ratio, 360.0));
+               }
            }
        }
        else
--- Server/Private/ServerGameplayContractTests_KoukuSupportSurface.cpp.before
+++ Server/Private/ServerGameplayContractTests_KoukuSupportSurface.cpp
@@ -409,7 +409,7 @@
    albionOwner.fPositionX = 6.f; albionOwner.fPositionZ = 6.f;
    updateTargets(2000u);
    tests.Require(albionOwner.iPatternTargetEntityId == 102u && std::abs(albionOwner.fYawDegrees - 90.f) < .0001f,
-       "Showtime keeps the existing server pattern target and faces that player once, independently of roster order");
+       "Showtime keeps the existing server pattern target and an already aligned yaw, independently of roster order");
    auto& trackingIds = showtimeLedger.PlayerTargetWindows.front().TrackingObjects;
    tests.Require(countTargetObjects(false) == 2 && countTargetObjects(true) == 2 && trackingIds.size() == 2u &&
        std::all_of(room->m_CombatObjectRuntime.Get_LiveObjects().begin(), room->m_CombatObjectRuntime.Get_LiveObjects().end(),
@@ -494,9 +494,12 @@
        "A rejected target transaction retries after valid ground returns");
    albionOwner.iPatternTargetEntityId = 101u; albionOwner.fPositionX = 6.f; albionOwner.fPositionZ = 6.f;
    room->m_Players[1u].fPositionX = 9.f; room->m_Players[1u].fPositionZ = 9.f;
+   const float yawBeforeMovingTarget = albionOwner.fYawDegrees;
    updateTargets(2402u);
-   tests.Require(albionOwner.iPatternTargetEntityId == 101u && std::abs(albionOwner.fYawDegrees - 45.f) < .0001f,
-       "A moving selected player changes server boss yaw without selecting the last roster member");
+   const float expectedMovingYaw = static_cast<float>(std::remainder(double(yawBeforeMovingTarget) +
+       std::remainder(45.0 - yawBeforeMovingTarget, 360.0) / 148.0, 360.0));
+   tests.Require(albionOwner.iPatternTargetEntityId == 101u && std::abs(albionOwner.fYawDegrees - expectedMovingYaw) < .0001f,
+       "A moving selected player changes server boss yaw over the remaining duration without selecting the last roster member");
    for (const bool completed : { true, false })
    {
        room->m_CombatObjectRuntime.Reset(); albionOwner.strPatternId = showtime.strPatternId;
@@ -508,6 +511,60 @@
        room->Clear_KoukuSaydonPatternAudition(completed);
        tests.Require(countTargetObjects(true) == 0 && countTargetObjects(false) == (completed ? 4 : 0),
            completed ? "Natural completion clears Showtime trackers while retaining fixed tails" : "Explicit stop cancels every owned Showtime instance");
+   }
+
+
+   // Rotation consumes the same duration clock as the target window, not render frames.
+   {
+       const auto savedPlayers = room->m_Players;
+       const auto savedOwner = std::make_unique<SERVER_WORLD_ENTITY>(albionOwner);
+       room->m_Players.clear();
+       auto& target = room->m_Players[1u];
+       target.iPlayerId = 1u; target.iNetEntityId = 101u; target.iCurrentHp = target.iMaximumHp = 100u;
+       target.isCombatReady = true; target.fMoveSpeed = 6.f; target.fPositionY = 1.f;
+       const auto setDirection = [&](const float yaw) {
+           const double radians = double(yaw) * 3.14159265358979323846 / 180.0;
+           target.fPositionX = 8.f + static_cast<float>(std::sin(radians));
+           target.fPositionZ = 8.f + static_cast<float>(std::cos(radians));
+       };
+       const auto beginFacing = [&](const float yaw, const std::uint32_t durationMs) {
+           room->m_CombatObjectRuntime.Reset(); showtime.MechanicTriggers.front() = targets;
+           showtime.MechanicTriggers.front().iDurationMs = durationMs;
+           albionOwner.strPatternId = showtime.strPatternId; albionOwner.strArchetypeId = "BOSS_KAKULSAYDON_G1_KOUKU";
+           albionOwner.fPositionX = albionOwner.fPositionZ = 8.f; albionOwner.fYawDegrees = yaw;
+           albionOwner.iPatternTargetEntityId = albionOwner.iTargetEntityId = 101u;
+           CKoukuSaydonLogicRuntime::Build(showtime, albionOwner, 5000u, showtimeLedger);
+       };
+       const auto nearYaw = [](const float actual, const float expected) {
+           return std::abs(std::remainder(actual - expected, 360.f)) < .001f;
+       };
+       setDirection(90.f); beginFacing(0.f, 1000u); updateTargets(4999u);
+       tests.Require(albionOwner.fYawDegrees == 0.f, "Showtime interpolation preserves yaw before the duration begins");
+       updateTargets(5000u);
+       tests.Require(nearYaw(albionOwner.fYawDegrees, 3.f), "The first of 30 duration ticks rotates three degrees instead of snapping ninety degrees");
+       updateTargets(5000u); updateTargets(4999u);
+       tests.Require(nearYaw(albionOwner.fYawDegrees, 3.f), "Duplicate and older ticks do not rotate the target twice");
+       for (std::uint32_t tick = 5001u; tick <= 5014u; ++tick) updateTargets(tick);
+       const float singleTickHalfYaw = albionOwner.fYawDegrees;
+       tests.Require(nearYaw(singleTickHalfYaw, 45.f), "Half of a fixed-target duration traverses half the shortest arc");
+       beginFacing(0.f, 1000u); updateTargets(5000u); updateTargets(5014u);
+       tests.Require(nearYaw(albionOwner.fYawDegrees, singleTickHalfYaw), "The same elapsed ticks give the same yaw when updates are grouped");
+       updateTargets(5029u);
+       tests.Require(nearYaw(albionOwner.fYawDegrees, 90.f), "The last valid duration tick reaches the current target exactly");
+       setDirection(-90.f); updateTargets(5030u);
+       tests.Require(nearYaw(albionOwner.fYawDegrees, 90.f), "The exclusive duration end cannot turn toward a changed target");
+       setDirection(90.f); beginFacing(0.f, 2000u); updateTargets(5000u);
+       tests.Require(nearYaw(albionOwner.fYawDegrees, 1.5f), "Doubling the authored duration halves the initial angular speed");
+       setDirection(-179.f); beginFacing(179.f, 1000u); updateTargets(5014u);
+       tests.Require(nearYaw(albionOwner.fYawDegrees, 180.f), "Crossing positive 180 degrees uses the short two-degree arc");
+       setDirection(179.f); beginFacing(-179.f, 1000u); updateTargets(5014u);
+       tests.Require(nearYaw(albionOwner.fYawDegrees, -180.f), "Crossing negative 180 degrees uses the short reverse arc");
+       setDirection(90.f); beginFacing(0.f, 1000u); updateTargets(5014u); setDirection(-90.f); updateTargets(5015u);
+       tests.Require(nearYaw(albionOwner.fYawDegrees, 36.f) && albionOwner.iPatternTargetEntityId == 101u,
+           "A moving selected target is followed from the current yaw using the remaining duration");
+       setDirection(90.f); beginFacing(-90.f, 1000u); albionOwner.strArchetypeId = "BOSS_KAKULSAYDON_G2_BIG_SAYDON"; updateTargets(5014u);
+       tests.Require(nearYaw(albionOwner.fYawDegrees, -45.f), "Big Saydon keeps its existing minus-ninety-degree model forward basis during interpolation");
+       room->m_CombatObjectRuntime.Reset(); room->m_Players = savedPlayers; albionOwner = *savedOwner;
    }

    // Teleport rebases only XZ; the next actual animation root sample retains height and clocks.
```


## G32-B. 박스 lifetime에 맞춘 V1 재생과 편집 줌 보존

사용자가 늘린 부채꼴 박스 2.5~2.7초와 실제 원본 1.5초가 달라 먼저 사라지는 문제를 해결한다. occurrence optional `fitEffectToDuration`은 기본 false이며 V1_EFFECT/V1_ELEMENT만 허용한다. Box Detail에서 켜고 Lifetime을 변경한 뒤 Apply/Save한다. source clock을 resource duration / box duration 비율로 한 번 재생하고, owner/bone 기록은 역비율로 실제 box clock에 조회한다. source document, spawn 횟수, 전체 재질/크기는 변경하지 않는다. Screen Post capture boundary도 같은 시간변환을 쓴다. 저작 parse/validate/save, Preview 및 Product projector/parser에 같은 필드를 연결한다. 새 CPP나 프로젝트 항목은 없다.

삭제/복제 후 자동 Fit 요청을 제거하고 같은 Pattern 재선택에서는 줌을 초기화하지 않는다. 명시 Fit 및 다른 Pattern 첫 선택은 유지한다. 각 변경 CPP를 격리 컴파일하고 실제 source의 serialization/투영 및 시간역변환을 검증한다. 사용자가 저장 중인 Composition은 최신 hash와 백업을 먼저 보존하고 대상 bool만 병합한다.


## G33. 별도 추적회전 DURATION의 실제 logic65 연결


사용자의 source947 새 logic65는 SHOWTIME_PLAYER_TARGETS인 logic64와 다른 회전 전용 Duration이다. 추가 field 없는 `BOSS_TRACK_TARGET` judgement kind를 기존 저작 enum목록/typed값검증/UI에 등록한다. Collider·결과·Hold·Effect template은 받지 않는다. Python은 기존 mechanicTriggers/25-field PATTERNMECHANICTRIGGER로 start/duration을 그대로 투영하며 PowerShell publisher와 Server Catalog가 같은 kind를 읽는다. 새 JSON root나 packet은 없다.

LogicRuntime은 같은 PlayerTargetWindows clock을 만들고 GameRoom은 같은 지정 타겟 선택·죽은 타겟 재선택 경로를 소비한다. BOSS_TRACK_TARGET만 G32 남은duration shortest arc 보간을 적용하고 visual 생성 전에 종료한다. 별도 logic64 SHOWTIME_PLAYER_TARGETS는 이전 즉시 facing과 target visual 계약을 유지한다. source947 logic65의 judgementKind 한 field만 바꾸는 out candidate와 root CAS patch를 제공하며 정본 저장은 이 작업에서 하지 않는다. 실제 전체947 prepare/project 및 네 duration, Server row/room, C++ codec/저작 선택과 관련TU를 확인한다.

### G33 반영 코드

```diff
--- Client/Public/KoukuSaydonCompositionDocument.h.before
+++ Client/Public/KoukuSaydonCompositionDocument.h
@@ -63,8 +63,8 @@
    /* The judgement a DURATION Logic runs and the outcome a RESULT Logic
       applies. Both are the Server's typed vocabulary; a definition that is
       only a name keeps the kind empty and stays DRAFT-only. */
-   inline constexpr std::array<const char_t*, 11u> KOUKU_SAYDON_JUDGEMENT_KINDS = {
-       "ROULETTE_CARD_MATCH", "GAZE_REAL_BOSS", "POSE_INPUT", "STAGGER_WINDOW", "COUNTER_WINDOW", "AREA_OVERLAP", "OBJECT_OVERLAP", "EXTERNAL_SIGNAL", "ATTACHMENT_HOLD", "PATTERN_COMPLETION_COUNT", "SHOWTIME_PLAYER_TARGETS" };
+   inline constexpr std::array<const char_t*, 12u> KOUKU_SAYDON_JUDGEMENT_KINDS = {
+       "ROULETTE_CARD_MATCH", "GAZE_REAL_BOSS", "POSE_INPUT", "STAGGER_WINDOW", "COUNTER_WINDOW", "AREA_OVERLAP", "OBJECT_OVERLAP", "EXTERNAL_SIGNAL", "ATTACHMENT_HOLD", "PATTERN_COMPLETION_COUNT", "SHOWTIME_PLAYER_TARGETS", "BOSS_TRACK_TARGET" };
    inline constexpr std::array<const char_t*, 12u> KOUKU_SAYDON_OUTCOME_KINDS = {
        "INSTANT_DEATH", "MAX_HP_PERCENT_DAMAGE", "MADNESS_GAUGE_ADD_PERCENT",
        "CLOWN_TRANSFORM", "FEAR", "FOLLOWUP_PATTERN", "PLAY_WORLD_OBJECT_MOTION",
@@ -94,7 +94,7 @@
        const std::string_view judgementKind,
        const KOUKU_SAYDON_OUTCOME_SLOT slot)
    {
-       if (judgementKind == "ATTACHMENT_HOLD" || judgementKind == "SHOWTIME_PLAYER_TARGETS") return false;
+       if (judgementKind == "ATTACHMENT_HOLD" || judgementKind == "SHOWTIME_PLAYER_TARGETS" || judgementKind == "BOSS_TRACK_TARGET") return false;
        if (judgementKind == "PATTERN_COMPLETION_COUNT") return slot == KOUKU_SAYDON_OUTCOME_SLOT::SUCCESS;
        if (KOUKU_SAYDON_OUTCOME_SLOT::TIMEOUT == slot)
            return judgementKind != "GAZE_REAL_BOSS" && judgementKind != "OBJECT_CONTACT";
@@ -192,12 +192,12 @@

    inline bool_t Kouku_LogicOwnsOutcomes(const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& logic)
    {
-       return (logic.strLogicType == "DURATION" && logic.strJudgementKind != "ATTACHMENT_HOLD" && logic.strJudgementKind != "SHOWTIME_PLAYER_TARGETS") ||
+       return (logic.strLogicType == "DURATION" && logic.strJudgementKind != "ATTACHMENT_HOLD" && logic.strJudgementKind != "SHOWTIME_PLAYER_TARGETS" && logic.strJudgementKind != "BOSS_TRACK_TARGET") ||
            (logic.strLogicType == "TRIGGER" && (logic.strTriggerKind == "ENTER_AREA" || logic.strTriggerKind == "OBJECT_CONTACT"));
    }
    inline bool_t Kouku_LogicAcceptsColliders(const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& logic)
    {
-       return (logic.strLogicType == "DURATION" && logic.strJudgementKind != "PATTERN_COMPLETION_COUNT" && logic.strJudgementKind != "EXTERNAL_SIGNAL" && logic.strJudgementKind != "COUNTER_WINDOW" && logic.strJudgementKind != "ATTACHMENT_HOLD" && logic.strJudgementKind != "SHOWTIME_PLAYER_TARGETS") ||
+       return (logic.strLogicType == "DURATION" && logic.strJudgementKind != "PATTERN_COMPLETION_COUNT" && logic.strJudgementKind != "EXTERNAL_SIGNAL" && logic.strJudgementKind != "COUNTER_WINDOW" && logic.strJudgementKind != "ATTACHMENT_HOLD" && logic.strJudgementKind != "SHOWTIME_PLAYER_TARGETS" && logic.strJudgementKind != "BOSS_TRACK_TARGET") ||
            (logic.strLogicType == "TRIGGER" && (logic.strTriggerKind == "ENTER_AREA" || logic.strTriggerKind == "OBJECT_CONTACT"));
    }
    inline const std::string& Kouku_LogicOutcomeKind(const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& logic)
--- Client/Private/KoukuSaydonActionWorkbench.cpp.before
+++ Client/Private/KoukuSaydonActionWorkbench.cpp
@@ -8507,7 +8507,7 @@
                if (box.strLogicId == logicId && found->strTriggerKind != "ENTER_AREA") box.strHoldLogicOccurrenceId.clear();
                if (const auto* hold = Find_LogicBox(pattern, box.strHoldLogicOccurrenceId);
                    hold && hold->strLogicId == logicId && found->strJudgementKind != "ATTACHMENT_HOLD") box.strHoldLogicOccurrenceId.clear();
-               if (box.strLogicId == logicId && (found->strJudgementKind == "ATTACHMENT_HOLD" || found->strJudgementKind == "SHOWTIME_PLAYER_TARGETS"))
+               if (box.strLogicId == logicId && (found->strJudgementKind == "ATTACHMENT_HOLD" || found->strJudgementKind == "SHOWTIME_PLAYER_TARGETS" || found->strJudgementKind == "BOSS_TRACK_TARGET"))
                {
                    box.OnSuccessLogicIds.clear(); box.OnFailLogicIds.clear(); box.OnTimeoutLogicIds.clear();
                    for (auto& collider : pattern.PresentationOccurrences)
@@ -12061,6 +12061,8 @@
            if (ImGui::InputFloat("Player move speed multiplier", &speed, .05f, .1f, "%.2f")) draft.fFollowSpeedScale = std::clamp(static_cast<double>(speed), .01, 10.0);
            ImGui::TextWrapped("Creates one fixed group per alive player at each interval. One tracking target follows each player until Duration ends. The selected Effect rows become templates for Server playback; their relative timing and transforms are preserved.");
        }
+       else if ("BOSS_TRACK_TARGET" == draft.strJudgementKind)
+           ImGui::TextWrapped("Rotate toward the Server-selected target over this box Duration. Uses the shortest arc, with no Effect template, Collider or outcome slots.");
        else if ("ATTACHMENT_HOLD" == draft.strJudgementKind)
            ImGui::TextWrapped("Keeps captured players attached until this window ends. It has no Collider or outcome slots. Select this window in the capture Trigger's Hold connection.");
        else if ("AREA_OVERLAP" == draft.strJudgementKind)
--- Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py.before
+++ Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py
@@ -153,6 +153,7 @@
     "EXTERNAL_SIGNAL": {"endsPatternOnSuccess"},
     "COUNTER_WINDOW": {"endsPatternOnSuccess"},
     "ATTACHMENT_HOLD": set(),
+    "BOSS_TRACK_TARGET": set(),
     "PATTERN_COMPLETION_COUNT": {"patternIds", "completionCount"},
     "SHOWTIME_PLAYER_TARGETS": {"fixedSelectionGroupId", "trackingPresentationOccurrenceId", "spawnIntervalMs", "followSpeedScale"},
 }
@@ -1777,11 +1778,11 @@
                 if enabled and status == "PRODUCT" and (not hold.get("enabled", True) or
                         hold_start > capture_start or hold_start + hold_duration < capture_start + capture_duration):
                     raise CompositionError(f"{box_context} Hold must be enabled and cover the complete Trigger window")
-            if kind == "ATTACHMENT_HOLD" and any(outcomes.values()):
-                raise CompositionError(f"{box_context} ATTACHMENT_HOLD has no outcomes")
-            if kind == "ATTACHMENT_HOLD" and any(row.get("logicOccurrenceId") == box_id and
+            if kind in {"ATTACHMENT_HOLD", "BOSS_TRACK_TARGET"} and any(outcomes.values()):
+                raise CompositionError(f"{box_context} {kind} has no outcomes")
+            if kind in {"ATTACHMENT_HOLD", "BOSS_TRACK_TARGET"} and any(row.get("logicOccurrenceId") == box_id and
                     presentation_resources[row["resourceId"]]["kind"] == "COLLIDER" for row in pattern.get("presentationOccurrences", [])):
-                raise CompositionError(f"{box_context} ATTACHMENT_HOLD has no Collider")
+                raise CompositionError(f"{box_context} {kind} has no Collider")
             if sum(logic_defs[target].get("kind") == "CAPTURE_PLAYER" for target in outcomes["Success"]) > 1:
                 raise CompositionError(f"{box_context} allows at most one CAPTURE_PLAYER result")
             if kind in END_TICK_KINDS and outcomes["Timeout"]:
@@ -3958,7 +3959,7 @@
                 continue
             logic = logics[box["logicId"]]
             kind = logic.get("judgementKind", logic.get("triggerKind"))
-            if kind == "SHOWTIME_PLAYER_TARGETS":
+            if kind in {"SHOWTIME_PLAYER_TARGETS", "BOSS_TRACK_TARGET"}:
                 continue
             if logic["logicType"] != "DURATION" and kind not in {"ENTER_AREA", "OBJECT_CONTACT"}:
                 continue
@@ -3981,14 +3982,15 @@
             if not box.get("enabled", True):
                 continue
             logic = logics[box["logicId"]]
-            if logic["logicType"] != "TRIGGER" or "triggerKind" not in logic or logic["triggerKind"] in {"ENTER_AREA", "OBJECT_CONTACT", "ANIMATION_BLEND"}:
+            kind = logic.get("judgementKind") if logic["logicType"] == "DURATION" else logic.get("triggerKind")
+            if kind != "BOSS_TRACK_TARGET" and (logic["logicType"] != "TRIGGER" or kind in {None, "ENTER_AREA", "OBJECT_CONTACT", "ANIMATION_BLEND"}):
                 continue
             clone_id = logic.get("clonePatternId", "")
             if clone_id and not any(p["patternId"] == clone_id and p["authoringStatus"] == "PRODUCT"
                                     for p in document["patterns"]):
                 raise CompositionError(f"{box['occurrenceId']} clone pattern must be PRODUCT: {clone_id}")
             mechanic_triggers.append({
-                "triggerId": box["occurrenceId"], "kind": logic["triggerKind"],
+                "triggerId": box["occurrenceId"], "kind": kind,
                 "startMs": box["startMs"], "durationMs": box["durationMs"],
                 "hudMode": logic.get("hudMode", "NONE"),
                 "teleportPosition": logic.get("teleportPosition", [0.0, 0.0, 0.0]),
--- Tools/GameplayPipeline/Publish-GameplayBalance.ps1.before
+++ Tools/GameplayPipeline/Publish-GameplayBalance.ps1
@@ -4129,7 +4129,7 @@
        $modes = @('NONE','POLYMORPH','MARIO','DANCE','MAZE')
        $triggerHudMode = [Array]::IndexOf($modes, [string]$trigger.hudMode)
        if (-not $triggerIds.Add([string]$trigger.triggerId) -or $triggerHudMode -lt 0 -or
-           $trigger.kind -cnotin @('REAL_GAZE_TELEPORT','BOSS_TELEPORT_XZ','HUD_ENTER','CARD_MAZE_HIDE_NEXT','CARD_MAZE_ENTER','ALBION_BLUE_CIRCLE','SUMMON_PATTERNS') -or
+           $trigger.kind -cnotin @('REAL_GAZE_TELEPORT','BOSS_TELEPORT_XZ','BOSS_TRACK_TARGET','HUD_ENTER','CARD_MAZE_HIDE_NEXT','CARD_MAZE_ENTER','ALBION_BLUE_CIRCLE','SUMMON_PATTERNS') -or
            ([uint64]$trigger.startMs + [uint64]$trigger.durationMs) -gt $koukuPatternDurationMs -or
            $trigger.teleportPosition -isnot [Array] -or @($trigger.teleportPosition).Count -ne 3 -or
            $trigger.clockHours -isnot [Array]) {
@@ -4138,9 +4138,9 @@
        $position = @($trigger.teleportPosition)
        foreach ($coordinate in $position) { Assert-JsonNumber $coordinate 'KoukuSaydon teleport coordinate' }
        if ($trigger.kind -ceq 'BOSS_TELEPORT_XZ' -and @($position | Where-Object { [Math]::Abs([double]$_) -gt 100000 }).Count -ne 0) { throw 'Boss XZ teleport coordinates exceed the world bounds' }
-       if ($trigger.kind -cin @('CARD_MAZE_HIDE_NEXT','CARD_MAZE_ENTER','BOSS_TELEPORT_XZ') -and
+       if ($trigger.kind -cin @('CARD_MAZE_HIDE_NEXT','CARD_MAZE_ENTER','BOSS_TELEPORT_XZ','BOSS_TRACK_TARGET') -and
            ($triggerHudMode -ne 0 -or $trigger.faceCenterYawOffsetDegrees -ne 0 -or
-            ($trigger.kind -ceq 'CARD_MAZE_HIDE_NEXT' -and @($position | Where-Object { $_ -ne 0 }).Count -ne 0))) {
+            ($trigger.kind -cin @('CARD_MAZE_HIDE_NEXT','BOSS_TRACK_TARGET') -and @($position | Where-Object { $_ -ne 0 }).Count -ne 0))) {
            throw "KoukuSaydon card maze trigger carries unrelated values"
        }
        Assert-JsonInteger $trigger.countPerPlayer 'KoukuSaydon circles per player' 0 8
--- Server/Public/GameplayCatalog.h.before
+++ Server/Public/GameplayCatalog.h
@@ -753,7 +753,8 @@
        ALBION_BLUE_CIRCLE,
        SUMMON_PATTERNS,
        SHOWTIME_PLAYER_TARGETS,
-       BOSS_TELEPORT_XZ
+       BOSS_TELEPORT_XZ,
+       BOSS_TRACK_TARGET
    };

    struct BOSS_PATTERN_SUMMON_PATTERN_SPAWN final
--- Server/Private/GameplayCatalog.cpp.before
+++ Server/Private/GameplayCatalog.cpp
@@ -3123,6 +3123,14 @@
                    mode != 0u || fields[11] != "-" || fields[12] != "0" || fields[13] != "0" || fields[14] != "0" ||
                    trigger.fTeleportX != 0.f || trigger.fTeleportY != 0.f || trigger.fTeleportZ != 0.f || trigger.fFaceCenterYawOffsetDegrees != 0.f)
                { m_strStatus = "KoukuSaydon Albion layout is invalid"; return false; }
+           }
+           else if (fields[4] == "BOSS_TRACK_TARGET")
+           {
+               trigger.eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TRACK_TARGET;
+               if (mode != 0u || trigger.iDurationMs > 600000u || fields[11] != "-" ||
+                   fields[12] != "0" || fields[13] != "0" || fields[14] != "0" ||
+                   trigger.fTeleportX != 0.f || trigger.fTeleportY != 0.f || trigger.fTeleportZ != 0.f || trigger.fFaceCenterYawOffsetDegrees != 0.f)
+               { m_strStatus = "Boss tracking duration carries unrelated values"; return false; }
            }
            else if (fields[4] == "BOSS_TELEPORT_XZ")
            {
--- Server/Private/KoukuSaydonBrain.cpp.before
+++ Server/Private/KoukuSaydonBrain.cpp
@@ -253,6 +253,14 @@
             trigger.iSpawnIntervalMs == 0u || trigger.iSpawnIntervalMs > 600000u ||
             !std::isfinite(trigger.fFollowSpeedScale) || trigger.fFollowSpeedScale < .01f || trigger.fFollowSpeedScale > 10.f))
        { status = "Showtime player-target visual or timing contract is invalid"; return false; }
+       if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TRACK_TARGET &&
+           (trigger.iDurationMs > 600000u || !trigger.strFixedVisualId.empty() || !trigger.strTrackingVisualId.empty() ||
+            trigger.iFixedLifetimeMs != 0u || trigger.iSpawnIntervalMs != 0u || trigger.fFollowSpeedScale != 0.f ||
+            trigger.eHudMode != LostArk::Shared::KOUKU_HUD_MODE::NONE || trigger.fTeleportX != 0.f || trigger.fTeleportY != 0.f ||
+            trigger.fTeleportZ != 0.f || !trigger.strClonePatternId.empty() || !trigger.ClockHours.empty() ||
+            trigger.fFaceCenterYawOffsetDegrees != 0.f || trigger.iCountPerPlayer != 0u || trigger.fPlayerEffectRadiusM != 0.f ||
+            trigger.iEffectLifetimeMs != 0u || trigger.iArenaRandomCount != 0u || trigger.bRandomPlayerOnly || !trigger.PatternSpawns.empty()))
+       { status = "Boss tracking duration must not create visuals or carry another mechanic's values"; return false; }
        if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SUMMON_PATTERNS)
        {
            std::unordered_set<std::string> spawnIds;
--- Server/Private/KoukuSaydonLogicRuntime.cpp.before
+++ Server/Private/KoukuSaydonLogicRuntime.cpp
@@ -156,7 +156,8 @@
    for (std::size_t index = 0u; index < pattern.MechanicTriggers.size(); ++index)
    {
        const auto& trigger = pattern.MechanicTriggers[index];
-       if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SHOWTIME_PLAYER_TARGETS)
+       if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SHOWTIME_PLAYER_TARGETS ||
+           trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TRACK_TARGET)
        {
            KOUKUSAYDON_PLAYER_TARGET_WINDOW_STATE state;
            state.iTriggerIndex = static_cast<std::uint32_t>(index);
--- Server/Private/GameRoom_BossSimulation.cpp.before
+++ Server/Private/GameRoom_BossSimulation.cpp
@@ -824,10 +824,11 @@
            window.TrackingObjects.clear(); window.bClosed = true;
            continue;
        }
-       if (trigger.eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SHOWTIME_PLAYER_TARGETS ||
+       const bool rotateOnly = trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TRACK_TARGET;
+       if (!rotateOnly && (trigger.eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SHOWTIME_PLAYER_TARGETS ||
            trigger.iSpawnIntervalMs == 0u || trigger.strFixedVisualId == trigger.strTrackingVisualId ||
            trigger.strFixedVisualId.empty() != (trigger.iFixedLifetimeMs == 0u) ||
-           !std::isfinite(trigger.fFollowSpeedScale) || trigger.fFollowSpeedScale < .01f || trigger.fFollowSpeedScale > 10.f)
+           !std::isfinite(trigger.fFollowSpeedScale) || trigger.fFollowSpeedScale < .01f || trigger.fFollowSpeedScale > 10.f))
        { m_strStatus = "Showtime target window lost its validated definition"; continue; }
        // Body facing has one server-owned target; independent player visuals keep map axes.
        const auto findFacingTarget = [&](const NET_ENTITY_ID id) -> SERVER_PLAYER* {
@@ -858,7 +859,8 @@
                const auto previousTicks = previousUpdateTick == 0u ? 0u :
                    Elapsed_ServerTicksSkippingReservedZero(window.iStartTick, previousUpdateTick) + 1u;
                const auto currentTicks = Elapsed_ServerTicksSkippingReservedZero(window.iStartTick, serverTick) + 1u;
-               if (std::isfinite(boss.fYawDegrees) && totalTicks > previousTicks && currentTicks > previousTicks)
+               if (!rotateOnly) boss.fYawDegrees = targetYaw;
+               else if (std::isfinite(boss.fYawDegrees) && totalTicks > previousTicks && currentTicks > previousTicks)
                {
                    const double ratio = (std::min)(1.0, double(currentTicks - previousTicks) / double(totalTicks - previousTicks));
                    const double turn = std::remainder(double(targetYaw) - boss.fYawDegrees, 360.0);
@@ -871,6 +873,7 @@
            boss.iTargetEntityId = boss.iPatternTargetEntityId = INVALID_NET_ENTITY_ID;
            boss.bHasPatternTargetLastPosition = false;
        }
+       if (rotateOnly) continue;
        for (auto it = window.TrackingObjects.begin(); it != window.TrackingObjects.end();)
        {
            const auto player = m_Players.find(it->first);
--- Server/Private/ServerGameplayContractTests_KoukuSupportSurface.cpp.before
+++ Server/Private/ServerGameplayContractTests_KoukuSupportSurface.cpp
@@ -494,12 +494,9 @@
        "A rejected target transaction retries after valid ground returns");
    albionOwner.iPatternTargetEntityId = 101u; albionOwner.fPositionX = 6.f; albionOwner.fPositionZ = 6.f;
    room->m_Players[1u].fPositionX = 9.f; room->m_Players[1u].fPositionZ = 9.f;
-   const float yawBeforeMovingTarget = albionOwner.fYawDegrees;
    updateTargets(2402u);
-   const float expectedMovingYaw = static_cast<float>(std::remainder(double(yawBeforeMovingTarget) +
-       std::remainder(45.0 - yawBeforeMovingTarget, 360.0) / 148.0, 360.0));
-   tests.Require(albionOwner.iPatternTargetEntityId == 101u && std::abs(albionOwner.fYawDegrees - expectedMovingYaw) < .0001f,
-       "A moving selected player changes server boss yaw over the remaining duration without selecting the last roster member");
+   tests.Require(albionOwner.iPatternTargetEntityId == 101u && std::abs(albionOwner.fYawDegrees - 45.f) < .0001f,
+       "The existing SHOWTIME_PLAYER_TARGETS keeps immediate facing independently of the new rotate-only duration");
    for (const bool completed : { true, false })
    {
        room->m_CombatObjectRuntime.Reset(); albionOwner.strPatternId = showtime.strPatternId;
@@ -528,7 +525,9 @@
            target.fPositionZ = 8.f + static_cast<float>(std::cos(radians));
        };
        const auto beginFacing = [&](const float yaw, const std::uint32_t durationMs) {
-           room->m_CombatObjectRuntime.Reset(); showtime.MechanicTriggers.front() = targets;
+           room->m_CombatObjectRuntime.Reset(); showtime.MechanicTriggers.front() = {};
+           showtime.MechanicTriggers.front().strTriggerId = "test.boss.track.duration";
+           showtime.MechanicTriggers.front().eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TRACK_TARGET;
            showtime.MechanicTriggers.front().iDurationMs = durationMs;
            albionOwner.strPatternId = showtime.strPatternId; albionOwner.strArchetypeId = "BOSS_KAKULSAYDON_G1_KOUKU";
            albionOwner.fPositionX = albionOwner.fPositionZ = 8.f; albionOwner.fYawDegrees = yaw;
@@ -541,6 +540,8 @@
        setDirection(90.f); beginFacing(0.f, 1000u); updateTargets(4999u);
        tests.Require(albionOwner.fYawDegrees == 0.f, "Showtime interpolation preserves yaw before the duration begins");
        updateTargets(5000u);
+       tests.Require(room->m_CombatObjectRuntime.Get_LiveObjects().empty() && showtimeLedger.MechanicTriggers.empty() &&
+           showtimeLedger.PlayerTargetWindows.size() == 1u, "Rotate-only duration uses the actual clock without visual templates or spawned objects");
        tests.Require(nearYaw(albionOwner.fYawDegrees, 3.f), "The first of 30 duration ticks rotates three degrees instead of snapping ninety degrees");
        updateTargets(5000u); updateTargets(4999u);
        tests.Require(nearYaw(albionOwner.fYawDegrees, 3.f), "Duplicate and older ticks do not rotate the target twice");
@@ -664,6 +665,18 @@
            "test.fixed\ttest.tracking\t4384\t2000\tNaN", "test.fixed\ttest.tracking\t4384\t2000\t11" })
            rejectedInvalid = !loadTargetRow(fields) && parsedTargets.Get_ActiveRevision() == admittedRevision && rejectedInvalid;
        tests.Require(rejectedInvalid, "Invalid Showtime identity, lifetime, interval and speed preserve the previous admitted catalog");
+       const std::string trackPrefix = "PATTERNMECHANICTRIGGER\t" + encounter + "\t" + patternId +
+           "\ttest.track.duration\tBOSS_TRACK_TARGET\t612\t1079\t0\t";
+       const std::string trackSuffix = "\t-\t0\t0\t0\t1\t0\t0\t0\t0\t0\t0\t0\t0\t0";
+       const bool parsedTrack = loadSupplement(trackPrefix + "0\t0\t0" + trackSuffix);
+       const auto* trackPattern = parsedTrack ? CKoukuSaydonBrain::Find_AnimationOnlyPattern(parsedTargets, patternId, status) : nullptr;
+       tests.Require(trackPattern && trackPattern->MechanicTriggers.size() == 1u &&
+           trackPattern->MechanicTriggers.front().eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TRACK_TARGET &&
+           trackPattern->MechanicTriggers.front().iStartMs == 612u && trackPattern->MechanicTriggers.front().iDurationMs == 1079u,
+           "The existing 25-field row admits a rotation-only duration with no visual templates");
+       const auto trackRevision = parsedTargets.Get_ActiveRevision();
+       tests.Require(!loadSupplement(trackPrefix + "1\t0\t0" + trackSuffix) && parsedTargets.Get_ActiveRevision() == trackRevision,
+           "Rotation-only duration rejects unrelated teleport values and preserves the admitted generation");
        const std::string teleportPrefix = "PATTERNMECHANICTRIGGER\t" + encounter + "\t" + patternId + "\ttest.teleport.xz\tBOSS_TELEPORT_XZ\t0\t1000\t0\t";
        const std::string teleportSuffix = "\t-\t0\t0\t0\t1\t0\t0\t0\t0\t0\t0\t0\t0\t0";
        const bool parsedTeleport = loadSupplement(teleportPrefix + "2.57\t1.3\t952.27" + teleportSuffix);
```


## G34. 쇼타임 Duration 랜덤 낙하 세트

사용자가 2026-09-15 두 묶음에서 낙하 한 세트씩 번갈아 500ms마다 아레나의 이동 가능한 바닥에 생성하도록 확정했다. Source952의 groups797/853은 각 15행이며 A1→B1→A2→B2→A3→B3 여섯 세트로 연결한다. 각 세트는 총구 BOSS 1행과 MAP 예고·낙하·충돌·장판 4행이다. 기존 fixed636의 플레이어 위치 2000ms 생성과 tracking454의 이동 속도0.5배 추적을 유지한다.

### G34-01. Composition과 게시

기존 CompositionDocument H/CPP와 Workbench, projector에 optional `randomVolleyOccurrenceSets`를 추가한다. 저장된 stable occurrence ID의 순서 있는 배열이며 각 세트의 원본 시간 차이·수명·TRS를 보존한다. `randomSpawnIntervalMs`, `randomArenaRadiusM`, `randomArenaHeightToleranceM`은 ms/m 단위다. 누락 시 기존 출력은 동일하다. 총구는 BOSS basis, MAP XZ는 첫 MAP 예고의 기준점에서 상대화하고 Y는 원문을 보존한다. 지정된30행은 Product의 정적 재생에서 제외하고 저작 Preview는 보존한다. 서버에는 visual ID와 수명 pool만 게시한다. source Save 실패는 기존 문서·파일을 유지한다.

### G34-02. Server 고정 틱과 기존 CombatObject

Gameplay publisher/catalog의 기존 SHOWTIME row에 supplemental random pool 행을 exact join한다. GameRoom의 기존 PlayerTargetWindow에 독립 반복 시계와 순번을 추가한다. 500ms=15tick마다 전역 한 세트를 생성하고 순번대로 순환한다. 기존 `Resolve_ArenaRandomVolleyOrigins`의 균등 원판·exact navigation·높이 검사를 재사용한다. 실패하면 기존 객체와 다음 순번을 보존하고, 지연된 tick을 한 번에 몰아 생성하지 않는다. Duration 종료는 새 생성만 닫고 이미 시작한 세트의 수명을 보존한다.

### G34-03. Client 혼합 기준점 재생

KoukuSaydonPresentationPlayer H/CPP의 targeted visual이 MAP과 BOSS 행을 각 기존 Sample 세션으로 재생한다. MAP은 복제된 CombatObject 위치, BOSS는 같은 source entity의 실제 CNpc transform/model을 사용한다. 공통 clock과 콘텐츠 정의는 하나이며 총구를 랜덤 지점으로 옮기거나 보스 yaw를 MAP에 전파하지 않는다. 원본 보스가 아직 생성되지 않았다면 총구만 해당 source를 기다린다.

### G34-04. 종료 증거

실제 codec Save/Expand와 projector의 원문 시간·TRS 보존, malformed pool 거절, 실제 room 1~4인 및 15tick 간격·종료·랜덤 분포·실패 보존을 검사한다. 혼합 anchor와 lifecycle CPU 검사, 변경 TU 최소 컴파일, JSON parse와 diff check 뒤 최신 사용자 Save를 CAS로 등록하고 KoukuSaydon owner를 게시한다. 새 CPP나 프로젝트 등록은 없다. Client/Server 전체 빌드는 사용자가 수행하고 화면 판정도 사용자에게 남긴다.


## G35. 알비온 전기 폭발 분류와 부채꼴 채움

사용자 첨부 이미지 첫 장의 십자형 전기 폭발과 둘째 장의 전방 세 갈래를 원본 MN_RPCT_07 액션의 서로 다른 occurrence로 대조한다. 기존 추출·변환된 particle mesh/native material을 재사용하며 쿠크의 전방축을 기준으로 source notify 방향·개수·크기를 한 번 적용한다. Effect 분류/이름은 `알비온 / 십자 전기 폭발 | 알비온_십자전기폭발`, `알비온 / 전방 3갈레 전기 폭발 | 알비온_전방세갈레전기폭발`, `알비온 / 소멸 이펙트 | 알비온_사라지기이펙트`다. 소멸 항목은 기존 쇼타임 `effect.kouku.gate3.showtime.saydon.disappear`를 공유한다.

기존 Tools/EffectPipeline 생성기와 Effect library/catalog 등록 경로를 사용한다. 알비온 fourfan warning은 기존 원·도넛의 native inner material parameter track으로 채움 곡선을 연결하고 해당 V1 박스의 source clock은 `fitEffectToDuration`으로 타임라인 길이를 따른다. 기존 쇼타임 부채꼴 inner 제거 결정은 별개이며 다시 채움으로 바꾸지 않는다. 사용자 저작 타임라인의 시작·길이·위치·회전·크기를 바꾸지 않는다.

원본 particle call·실제 설치 mesh transform과 CPU playback 수치로 방향/크기/개수를 검사하고 문서 parse·native profile 연결·후보 before SHA를 확인한다. 최신 사용자 Composition에는 신규 resource와 요청된 이름/fit만 CAS로 등록한다. Rendering/KoukuSaydon 정본 publisher로 배포하고 source 반영·게시와 사용자 화면 판정을 분리한다. 새 제품 C++ 파일·프로젝트 등록은 예정하지 않는다.

### G35-01. 백스텝 감전빔의 명시적인 리소스 교체

사용자가 새 알비온 전방 세 갈래를 기존 백스텝 후 감전빔에도 사용하도록 요청했다. 기존 `effect.kouku.gate3.backstep.electric.threeway.authored`의 stable asset/resource ID와 분류 위치를 유지하면서 내부 payload를 새 세 갈래 조합으로 교체한다. 이전 세 갈래 투사체의 ribbon/도착점 표현과 새 전기 줄기를 겹쳐 실행하지 않는다. 원본 source leaf나 손 트레일은 다른 소비자가 있으므로 삭제하지 않는다. 이 교체는 사용자 선택한 연출이며 원본4219962의 투사체를 그대로 복원했다는 의미가 아니다. 현재 P46에는 Effect occurrence가 없어 임의의 새 재생 시간을 넣지 않고 Effect library 리소스를 교체한다.

## G36. Showtime bootstrap 부모 의존 순서 복구

Server 시작의 `Showtime random volley owner, order or repeated settings do not match`를 실제 게시 데이터와 `CGameplayCatalog::Load`로 재현한다. `New-KoukuShowtimeTargetRows`의 부모 우선 생성 순서를 최종 `Get-BootstrapRowSortKey`가 보존하도록 TARGETS/RANDOM을 encounter·pattern·trigger와 부모 우선순위로 묶는다. 기존 숫자 정렬과 출력 행 내용은 유지하며 서버 검증 조건과 저작 설정은 변경하지 않는다.

실제 publisher 정렬 함수의 회귀에서 32개 순번, 다중 소유자, 부모만 있는 경우와 기존 World/Logic/Stage 순서를 확인한다. 공식 Gameplay publisher로 생성한 전체 bootstrap을 실제 CGameplayCatalog에서 읽어 수정 전 실패·수정 후 성공과 잘못된 소유자/순번/반복 설정 거절을 확인한다. 검증 후 공식 publisher로 runtime을 교체하고 RESULT에 적용·실행 경계를 기록한다. 새 제품 C++ 파일이나 프로젝트 등록, Client 실행은 필요하지 않다.

## G37. 십자 전기 위치 고정과 세 갈래별 그룹 편집

십자 전기의 authored particle `localSpace`를 끄고, 현재 배치된 십자 occurrence의 `followBoss`도 꺼 생성 시 보스 위치·방향으로 만든 기준점을 유지한다. BOSS anchor 자체와 사용자가 저장한 offset·회전·크기·시작·수명은 보존한다. 원본 source leaf와 다른 이펙트에 같은 설정을 전파하지 않는다.

알비온 전방 세 갈래와 이를 재사용한 백스텝 문서는 각 원본 호출의 14개 요소를 각각 한 그룹으로 묶는다. 현재 `Build_AttachmentElementGroups`는 root 기준의 요소를 group ID와 무관하게 합친다. `Effect_Tool_Helpers.cpp`에서 명시적인 수동 그룹을 root-local 그룹 키에 포함하고, 기존 `Group Center → Translate_AttachmentElementGroup`으로 세 그룹을 따로 선택·이동하게 한다. 본별 그룹과 inheritance 검증은 보존한다. 위치는 사용자가 직접 조절하며 요소 ID·재질·입자·TRS·시간은 그대로 둔다. 생성기에도 같은 정책을 반영해 재생성 때 그룹이 다시 합쳐지지 않게 한다.

최신 저장본에서 요청 필드만 변경한 후보를 만들고, 실제 문서 codec·그룹 이동·이동하는 root의 CPU 재생과 변경 TU 최소 컴파일을 검사한다. 원문 해시와 Composition writer lock을 확인해 CAS 설치한 뒤 변경 occurrence를 공식 KoukuSaydon owner로 게시한다. 새 파일·shader·프로젝트 등록은 필요하지 않다. 그룹 중심 편집 코드가 바뀌므로 Client 재빌드와 화면 판정은 사용자에게 남긴다.

## G38. 알비온 점프·등장·중앙 착지와 파란 원 채움

저장 충돌은 G37의 외부 occurrence 변경을 현재 파일 해시로 확인하고, 해당 follow 값과 revision만 역변경해 사용자 저장 기준 bytes와 일치시킨다. 새 이펙트 문서와 그룹 편집은 유지하며 사용자 Save 성공 뒤 최신 파일에서만 후속 후보를 만든다. 기존 freshness 검사와 미저장 메모리는 수정하지 않는다.

`ALBION_AIRBORNE` TRIGGER에 `airbornePhase`, `airborneHeightM`, `airborneDurationMs`, `teleportPosition`을 명시한다. phase는 `SELECT_PLAYER/JUMP/APPEAR_PLAYER/DISAPPEAR/CENTER/SLAM`이다. JUMP는 원본 후속 낙하의 합으로 구한 약13.678813m를200ms 동안 상승하고 정점에서 유지한다. APPEAR_PLAYER는 선택한 살아 있는 플레이어 위치로 이동해 `_04`의 약3.278311m 하강을 소비한다. DISAPPEAR는 현재 XZ에서 처음 점프 높이, CENTER는3관문 스폰 절대 XZ에서 같은 높이를 사용한다. SLAM은 trigger 시각 이후 `_05`에 남은 하강 곡선을 현재 높이에 맞춰 정규화해 지면까지 내려온다. Stage·animation·Effect 시작과 길이, 원본 root suppression은 유지한다.

Composition H/CPP와 Workbench는 typed 값의 편집·검증·저장을, Python projector는 같은 입력 검증과 mechanicTriggers 출력을 소유한다. Gameplay publisher와 Server catalog는 기존 trigger 행에 settings를 결합하는7열 `PATTERNALBIONAIRBORNE`를 검증한다. Server GameRoom이 random player·navigation·collision을 확인해 순간이동과 phase를 commit하고 기존 `Apply_StageRootMotion`이 Y만 phase 규칙으로 계산한다. 선택은 같은 시각의 등장보다 먼저 처리하며 반복·종료 때 per-pattern 상태를 정리한다. 기존 root 원점을 함께 옮겨 다음 프레임에 이전 위치로 돌아가지 않게 한다.

일반 Play는 기존 preview clone과 PreviewRootMotion/PresentationPlayer에서 같은 phase와 source clock을 소비한다. 서버가 복제한 플레이어의 읽기 전용 위치를 preview run에 보존해 seek에서도 같은 대상을 사용하며 실제 gameplay actor를 Client에서 이동시키지 않는다. Server Play는 서버가 선택한 실제 위치를 snapshot으로 표시한다. 두 경로 모두 원본 root를 몸체에 중복 적용하지 않는다.

사용자가 재확인한 중앙 시작은 해당 P39의 `resetBossToSpawn=true`로 기존 Gate3 spawn 경로를 사용한다. SELECT는 ID를, APPEAR는 실제 등장 시점 위치를 고정하고 원본 하강의 prefix minimum으로 착지 후 지면을 유지한다. 최신 사용자 파란 원7개와 Stage11 hold 길이는 보존한다.

파란 원은 combined runtime과 standalone warning의 native3600 `inner` lane만0초0→2초1로 바꾼다. combined 안의 낙뢰 시작2초와 맞추며 기존5680ms 박스의 fit 설정·낙뢰 시점·색·크기·alpha는 유지한다. 실제 native parameter binding, phase 수치·서버 이동 및 실패 보존, codec Save/Reload, 최소 변경 TU 컴파일과 공식 게시를 확인한다. 최종 JSON은 사용자 최신 Save를 보존해 CAS 설치하고 제품 빌드·화면 확인은 사용자에게 남긴다.


## G39. 네 방향 분신 이동의 서버 선택과 그룹 중심 회전

저작 Pattern P50/P53/P54/P55는 각 방향 이동 STAGE_1 1600ms 뒤에 기존 P60의 브레스 STAGE_2 5167ms와 Effect를 연결한다. 원본 P60은 보존한다. 부모 P52에는 겹친 네 Pattern 대신 DURATION `CROSS_DIRECTION_CLONES`를 배치한다. `directionPatternIds`는 전방·후방·왼쪽·오른쪽 순서의 네 stable ID이며 `cloneEndStageId`는 `STAGE_1`이다. 현재 전방의 Summon 배치는 중복 생성을 막도록 새 Logic으로 교체한다.

서버는 Logic 시작 시 같은 보스의 위치·yaw와 각 방향의 STAGE_1 끝 root motion을 사용해 월드 목적지를 계산하고 arena spawn 중심과 XZ 거리가 가장 작은 방향을 한 번 선택한다. 같은 거리는 저작 배열 순서로 결정한다. Parent의 clock/identity는 보존하고 선택된 child만 실제 본체의 animation/root motion/presentation을 소유한다. 다른 세 방향은 기존 dependent Summon entity 생성·snapshot·despawn 경로를 사용하고 STAGE_1 deadline에서 끝낸다. 창 안에서 다른 animation/Pattern owner가 겹치면 거절하며 일반 one-action-owner 검사는 유지한다. 네 방향의 단독 재생은 브레스까지 유지한다.

CompositionDocument는 새 Logic의 parse/validate/serialize와 참조·window 검사를, Workbench는 네 방향 선택 UI를 소유한다. projector와 Gameplay publisher/catalog는 typed 소비 데이터를 생성하며 Server/Shared snapshot은 parent와 active child의 시계를 분리한다. Client는 같은 본체에서 child animation과 Effect를 소비하고 parent의 다른 presentation을 유지한다. 일반 Preview에서도 네 배우 구성과 종료 구간을 확인하도록 기존 preview 경로를 확장한다. 새 C++ 파일 없이 기존 파일을 사용하며 필요한 protocol 변경은 writer/reader/consumer를 함께 수정한다.

Effect Tool의 Group Center 옆에 Group Rotation (deg) 슬라이더를 추가한다. 첫 요소의 저장 회전을 기준으로 공통 quaternion delta를 구해 그룹 중심 주위 위치·orientation과 관련 선형 운동 벡터를 함께 회전한다. 기존 source/bone basis는 보존하고 지원하지 않는 animation/revolution의 곡선은 명시적으로 거절한다. per-element Transform에 저장하므로 새 문서 schema나 별도 누적 회전 상태를 만들지 않는다. 기존 transactional commit과 현재 커서 preview 재샘플링을 사용한다.

사용자 Save 완료 후의 최신 Composition을 기준으로 후보를 준비하고 설치 전 writer lock/CAS로 다시 확인한다. 검증은 변경 codec의 저장 왕복/실패 보존, 서버 방향 선택·분신 종료·parent 유지, snapshot 왕복, 그룹 회전 수치, Python/JSON/XML parse, 증분 Product Build와 scoped diff-check다. Client/UI 실행·화면 캡처는 하지 않으며 실제 네 방향 표시와 브레스·슬라이더의 최종 시각 확인은 사용자에게 남긴다.


### G39-01. 저장된 Parent Summon과 일반 Play 연결

사용자가 추가 저장한 revision993의 P52.summon.1(start0/duration15000)을 보존한다. 새 Logic의 `summonOccurrenceId`는 이 실제 배치를 참조하고 같은 시작·Logic 전체 수명 포함을 요구한다. 독립 patternSpawns와 동시에 연결하지 않는다. 네 방향은 explicit duration6767ms로 저장해 각 Stage의 개별 fixed-tick 반올림 대신 기존 fixedTimeline 시계를 사용한다. 일반 Play는 기존 Bundle preview actor 준비에서 본체1+분신3을 stage하고, Summon의 explicit patternSpawns도 같은 기존 CNpc preview 생성 경로로 재생한다. 사용자 Bundle의 동일 boss 중복 검사는 유지한다.

## G40. 일반 Play의 순간이동과 추적 회전 Logic

쇼타임 P35의 왼쪽 위 이동은 logic63의 기존 BOSS_TELEPORT_XZ이며 배치 시작39209ms와 목적지[2.57,1.3,952.27]를 그대로 소비한다. 중앙 복귀 배치17은55869ms에 있지만 logic47은 이름만 있는 TRIGGER다. 이 정의에 같은 BOSS_TELEPORT_XZ와 실제 Gate3 boss spawn 좌표[-0.0700000003,1.32000005,942.330017]를 연결한다. 사용자 배치 시간·Stage·Effect는 수정하지 않는다.

PreviewRootMotion과 PresentationPlayer의 기존 Preview actor에서 typed 순간이동 이벤트를 source clock으로 평가한다. XZ만 바꾸고 Y와 재생 animation은 유지한다. 이벤트 시각의 누적 root 변위를 뺀 새 원점을 사용하여 다음 프레임에 이전 위치로 되돌아가지 않게 하며, 커서를 과거로 옮기면 이전 원점과 이벤트 순서를 다시 계산한다. 모델이 없는 Logic 전용 구간과 BossMotion 충돌, 동일 시각 이벤트 순서를 현재 저작 계약과 맞춘다. 저장되지 않은 이름만의 Logic을 표시 이름으로 실행하지 않는다.

이미 저작된 BOSS_TRACK_TARGET 구간은 복제된 살아 있는 플레이어의 읽기 전용 위치를 사용해 Preview actor의 회전을 재현한다. 실제 player·boss Transform이나 damage 판정은 이 Preview 경로에서 변경하지 않는다. 기존 알비온 airborne와 Summon의 시간·root 억제·rollback 경로를 유지한다. 순간이동 전후·중앙 복귀·역방향 seek·재시작·Y 보존과 변경 TU 최소 컴파일을 확인한다. 새 제품 CPP와 project/filter 등록은 없으며, 실행 파일 빌드는 사용자가 수행한다.

## G41. 알비온 플레이어 위치 장판과 세 갈래 전기 Local Space

P39.logic.12에 저장된4426ms의 새 logic72를 기존 ALBION_BLUE_CIRCLE로 연결한다. countPerPlayer1·radiusM0·effectLifetimeMs7000을 사용하여 Trigger가 발생할 때 살아 있는 플레이어 위치마다 원 예고·폭발을 한 개 생성한다. 자산 연결은 기존 BossCatalog의 combatvisual.kouku.albion.bluecircle → effect.kouku.albion.bluecircle.warning.impact.runtime을 사용한다. 기존 MAP 장판7개의 위치·시간과 알비온 animation/airborne 배치는 보존한다. Collider 전용 logicOccurrenceId를 Effect 연결 용도로 바꾸지 않는다.

일반 Play의 Trigger 이펙트는 KoukuSaydonPresentationPlayer_LogicPreview.cpp에 기존 class method로 분리한다. 기존 SESSION/Sample와 Effect 준비·시간 샘플을 재사용하고, 발생 시점의 복제 플레이어 위치를 고정하여 seek에서도 같은 원을 재현한다. 새 재생에서는 선택과 위치를 초기화한다. 이름만 있는 Logic은 실행하지 않는다. 새 CPP를 Client.vcxproj와 기존 해당 filter에 최소 등록하고 TU 컴파일·프로젝트 XML parse·발생/종료/seek/대상 없음 보존을 확인한다. 새로운 Server combatobject 또는 두 번째 Effect runtime은 만들지 않는다.

세 갈래 전기 폭발 effect.kouku.albion.frontthree.electric.impact와 같은 payload의 파생 백스텝 문서는 detail.particle.localSpace만 false로 바꾼다. 원본 Required literal과 source leaf, 사용자가 조절한 그룹·Transform·시간·Composition follow 설정을 보존한다. 생성기의 같은 정책과 최신 저작에서 해당 필드만 바꾸는 후보 경로를 함께 갱신한다. 편집 중인 정본을 덮어쓰지 않으며 최종 저장본 hash/CAS 확인 후 설치한다.


## G42. 화염링 fixed-axis Sprite의 편집 회전

화염링_동일화염포의 Sprite Particle 02는 원본 orientation axis lock이 있는 Sprite이고 SourceTransformTrack은 없다. 최종 Billboard 면을 만드는 단계가 Element/Group 회전을 버린다. Sprite detail에 선택적 `followEmitterAxisRotation`(기본 false)을 추가하고, 활성 SourceRecipe의 fixed-axis Sprite에 한해 authored emitter basis를 최종 면에 한 번 적용한다. Local Space에서는 현재 emitter basis, World Space에서는 생성 시점 basis를 사용한다. 기존 SourceTransformTrack/local 경로와 일반 camera/velocity billboard, 워로드의 수동 roll 보정은 유지한다. UI checkbox와 JSON 저장/로드를 연결하고 화염링 두 조립 문서의 해당 요소만 켠 후보를 준비한다. 실행 중 편집기의 정본은 교체하지 않는다. 기존 파일을 확장하므로 새 C++ 프로젝트 등록은 없다. 실제 codec round trip, Playback/Geometry 수치와 관련 TU 컴파일로 확인하며 화면 판정은 사용자가 한다.


## G43. Summon 하나로 재사용하는 분신 Parent

Parent는 Summon occurrence 하나를 배치하고 그 start/duration만 실행 시계로 사용한다. 재사용 Summon definition의 optional `summonKind=CROSS_DIRECTION_CLONES`, `directionPatternIds` 네 개, `cloneEndStageId`가 방향별 Animation/Effect leaf를 지정한다. 기존 DURATION Logic 연결도 유지하되 두 입력을 같은 실행 window로 resolve한다. 서버의 기존 중앙 거리/실제 대시 endpoint 본체 선택 경로를 재사용하며 별도 숨은 Logic을 저장하지 않는다. MainApp, ordinary Play, publisher 모두 같은 설정을 소비한다. 기분나빠, 십자 화염폭발, 3갈래 불뿜기는 각각의 Summon 설정으로 동일 기능을 재사용한다. Parent의 빈 Pattern 안내는 Summon/Logic 실행 가능 상태를 반영한다. 현재 사용자 편집과 공유 Summon 참조를 보존하며 fresh source에서 필요한 후보만 작성한다. 기존 파일 확장만 사용하고 새 C++ 파일은 없다. 코드/codec/preview/projector/서버 카탈로그 검증 후 사용자 저장·종료 상태가 확인된 경우에만 해시 비교로 정본을 설치한다.


## G44. PR #393과 #392의 Composition·Sequence 병합 — 2026-09-16

기준은 우리 `560741ac`와 #392가 반영된 main `af056661`이며 공통 기준은 `c65b2cf4`다. Gate1 Composition은 우리 revision1051의 기존72패턴과392개 직접 애니메이션을 그대로 유지한다. 상대의 새 컷신5개(P63~67)는73~77로 옮기고 그 내부 action/occurrence 참조를 함께 재발급한다. 충돌 WORLD23은27, Presentation64/65는67/68로 옮기며 새 의존 행을 모두 추가한다. 최종 revision1052/nextPattern78/nextWorld28/nextPresentation69로 저장한다.

Sequence는 이전 작업자의 카메라·컷신 수정과 빙고 엔딩을 기준으로 병합한다. 겹치지 않는 우리 플레이어 도착 트리거4개와 바주카·절단칼 등 WORLD 수정도 보존한다. WorldSequences의 촛대·HandBook 수정은 상대 값을 유지하며 authoring/runtime을 같은 게시 결과로 맞춘다. 자동 병합 C++6파일은 카메라 편집 연결과 기존 preview 호출 흐름을 대조한다.

원본 작업 폴더의 미커밋 RESULT는 별도 보존하고, 분리 worktree에서 병합·참조/ID/수명 검사와 해당 publisher를 수행한다. 변경 JSON/XML parse, diff-check와 필요한 컴파일을 확인하고 PR을 병합한 뒤 원래 작업 폴더에서 main을 fast-forward pull한다. 실행 중 편집기의 미저장 상태를 버리거나 Client를 자동 실행·종료·조작하지 않는다. 화면 최종 판정은 사용자에게 남긴다.

## G45. 명시 Pattern 길이가 있는 클립 추가의 편집 거절 — 2026-09-16

현재 P67은 durationMs6267과 Stage 합6267이 같다. 기존 append는 Stage만 늘려 정상적인 explicit lifetime 검증에서 거절된다. `KoukuSaydonActionWorkbench.cpp`의 Stage 추가, 단일 clip append/bind, action append, cinematic group append와 Pattern Start Offset 변경에서 기존 `Extend_PatternLifetimeForAuthoredLanes`를 candidate commit 전에 호출한다. Stage 합·기존 수명·저작 lane 끝의 최댓값으로만 늘리고 Effect/Logic의 시간·TRS·stable ID는 보존한다. 길이 검사를 삭제하거나 정본 JSON의 duration을 임의로 지우지 않는다.

기존 파일만 수정하므로 신규 H/CPP와 project/filter 등록은 없다. 실제 함수와 현행 codec으로 P67 거절 재현, 추가/기존 Stage 양쪽 입력, 긴 tail 보존, implicit clock, 상한·잘못된 입력 rollback 및 out 사본 Save/Reload를 검사한다. 실행 중 Client의 미저장 편집은 유지하며 Product 빌드에는 사용자 저장·종료가 필요하다. 실제 UI 입력 확인은 사용자에게 남긴다.

## G46. 편집 중 외부 Composition 변경의 보존 병합 — 2026-09-16

작은 오망성 resource/occurrence duration을 외부에서 수정하자 열린 편집기의 Save가 기준본 불일치로 거절됐다. 해당 외부 수정만 writer lock과 정확한 before/after SHA로 되돌렸고 사용자가 Save 성공을 확인했다. 기존 Save_Atomic은 끝에 추가된 presentation resource만 특별히 병합하며 기존 항목의 서로 다른 필드 수정도 모두 거절한다.

Document 내부에서 로드 기준본·사용자 draft·현재 디스크의 세 상태를 비교한다. 객체 필드와 stable ID 배열의 겹치지 않는 변경은 함께 보존하고, 비-ID 배열은 하나의 값으로 취급한다. 동일 필드의 서로 다른 변경, 삭제와 수정의 경쟁, 충돌하는 순서 변경은 정확한 경로를 알려 거절한다. writer lock, freshness/CAS, 후보 validation, 원자 교체·재개방을 그대로 유지하고 Save 성공 후 기존 LastGood 소비 경로로 draft에 병합 결과를 돌려준다. 별도 Workbench 저장 경로나 무조건 덮어쓰기 기능은 만들지 않는다.

KoukuSaydonCompositionDocument.cpp와 기존 public 계약 주석을 수정하며 새 C++ 파일/project/filter 항목은 없다. 실제 codec의 Save/Reload로 겹치지 않는 편집·동일 필드 충돌·추가·삭제·순서·실패 시 원본/draft 보존을 검증하고 해당 TU를 격리 컴파일한다. 실행 중 구버전 Client는 소스 수정만으로 갱신되지 않으므로 사용자 빌드·재실행과 실제 Save 결과를 별도로 기록한다. 현재 사용자가 계속 편집 중인 정본 Composition에는 추가 외부 수정을 하지 않는다.


## G47. 화염링 Sprite의 양면 저작 옵션 — 2026-09-16

화염링_동일화염포 Sprite Particle02의 native2876은 Additive One Sided이며 회전 후 뒷면이 컬링된다. detail.sprite.twoSided 선택 bool을 기본false로 추가한다. 원본 material renderProfile과 native descriptor 일치 검사는 유지한다. Artist registry의 실행 가능한 source Sprite 중 Alpha/Additive One Sided만 허용하고 실제 particle draw에서 기존 양면 pass1/2로 선택한다. 일반Validate는 미지원요소의true를 거절하고 source-contract 문서는 이 저작옵션을 거절한다. 사용자 후속 지시대로 ImGui 항목은 추가하지 않는다. 지정된 조립 Effect의 Sprite Particle02에만 twoSided=true를 적용한다. 기본false는 JSON에서 생략하며 재수입의 기존Detail보존 경로를 사용한다. 기존 파일만 확장하므로 프로젝트 등록은 없다. 현재 사용자 effect와 Composition은 직접 덮어쓰지 않고 지정element 하나의 byte-preserving 후보를 준비한다. 현재헤더로 codec roundtrip·오류거절·원본재질보존·패스상태검사와 수정TU의 격리컴파일을 수행한다. 제품빌드·재실행과 화면검증은 사용자 담당이다.


## G48. 패턴 사이 Animation·Effect 선택 복사/붙여넣기 — 2026-09-16

기존 타임라인의 Stage/Animation/Presentation stable 선택을 Ctrl+C로 세션 내 값 snapshot에 보관하고, 다른 Pattern 선택 뒤 Ctrl+V로 그 Pattern 끝에 추가한다. OS 문자열 clipboard나 다른 런타임을 만들지 않는다. snapshot은 패턴 전환과 원본 이후 편집에서 독립적이며 유효한 새 Copy만 기존 내용을 교체한다. Ctrl/Shift 클릭과 marquee로 선택한 Animation·Effect 혼합과 Stage 자식을 한 번만 복사하고 지원하지 않는 lane이 섞이면 전체를 거절한다.

clip의 원본 profile/action/stage/slot/refRevision과 sourceIn/out/playRate/endPolicy, Effect의 상대 시각·수명·follow/fit/TRS·그룹을 보존한다. 새 Stage/action/animation/presentation/group ID는 대상 Pattern allocator로 재발급하며 복사된 WORLD anchor owner도 새 occurrence로 연결한다. 선택된 두 clip 사이의 ANIMATION_BLEND window는 함께 옮기고 외부 clip과의 boundary만 제외한다. 일반 gameplay Logic에 연결된 Effect는 무관한 전투를 복제하지 않도록 전체를 거절한다. 복사 시 현재 유효한 미저장 geometry를 함께 캡처한다. 누락·변경된 의존 정의, actor가 다른 animation, ID 소진·600000ms 상한은 명확히 거절한다.

Paste는 기존 Commit_Candidate와 lifetime 확장/Mark_Draft를 통해 한 번만 commit한다. 원본과 대상 기존 행의 timing·공유 정의를 보존하고 실패 시 draft·clipboard·선택을 유지한다. 새 항목을 선택하고 다음 Save가 기존 원자 저장 경로를 사용한다. 사용자가 편집 중인 Composition JSON을 외부 수정하지 않는다.

Workbench H/CPP만 확장한다. 독립 Patterns/Sequencer pane과 standalone 모두 focus 소유권을 확인하고 textinput·활성 widget·popup·drag/marquee에는 키를 소비하지 않는다. 프레임의 row 포인터 소비 뒤 한 번 처리하고 Ctrl+D/Delete의 기존 동작을 유지한다. 새 C++ 파일과 project/filter 등록은 없다. 실제 copy/paste API와 현재 Composition codec으로 혼합 선택·반복 Paste·ID/참조·상한·rollback·저장 왕복을 검사하고 Debug TU 컴파일을 수행한다. Client UI는 실행하지 않으며 새 제품 빌드·재실행과 실제 단축키 확인은 사용자가 한다.

## G49. 마리오 본무대 인형·공의 생존 재생과 Object 편집 기능 — 2026-09-16

첨부 두 이미지에서는 공 둘레의 무지개빛 고리·상부 문양과 붉어진 후속 연출을 관찰했다. 원본 NPC 자료의 본무대 서커스 공 480713은 MN_PPCC_00이며 미니게임 안의 빨강·노랑·파랑 공 480715~480717과 별도다. 대상 공의 활성 action/buff 원본을 추적해 하나의 재사용 Effect로 조합하고 색 전환 시각·크기·재질을 원본 입력으로 확인한다. 이미 설치한 미니게임 공의 다른 동작은 유지한다. 인형의 회전 화염 Motion은 현재 작은/큰 두 형태 모두 17314ms이며 3개 animation track과 object-sustain15 Effect를 사용한다. 사용자 요청대로 공통 불뿜기 25요소를 실제 입 본에 연결한 조립 Effect로 교체하고 현재 Object의 크기·배치를 보존한다.

World Object는 현재 HP가 없는 Client 표현이다. 기존 colliderTracks는 Object가 플레이어에게 피해를 주는 판정이므로 피격 body와 구분한다. objectResources의 선택적 전투 body에 HP·실측 local bounds·생존 정책을 저장하고 projector가 실제 resource scale과 WORLD placement를 반영해 Server 정의로 게시한다. 일반 몬스터로 위장하지 않고 기존 world entity/전투 hit 경로의 명시적 WORLD_OBJECT 대상으로 처리한다. Server가 HP 감소·0·소멸을 결정하고 기존 owned WORLD cue의 정확한 식별자로 Client의 모델·Effect를 함께 정리한다. 일반 종료 뒤 생존, 취소·전투 중단·방 정리, 늦게 들어온 플레이어와 지연된 cue의 재생 방지도 같은 계약에서 처리한다. 인형 두 크기와 본무대 공은 HP 2000을 사용하며 단위와 모델 범위를 확인한 뒤 후보를 만든다.

Object Tool은 공용 Sequencer 창 안에서 별도 CWorldObjectTool 편집 코드를 사용한다. Stage 끝 드래그·직접 길이 편집·Fit Stage to Animation을 candidate 검증 경로로 연결하며 기존 animation/effect의 시작과 속도·Transform을 보존한다. Zoom 슬라이더·Ctrl+wheel·timeline Fit을 제공하고 Effect의 선택적 lifetime fit을 codec·publisher·재생 시계까지 연결한다. Animation과 Effect의 마지막 tail을 포함한 전체 단위 반복은 명시적 옵션으로 저장해 기존 모션 반복 결과를 바꾸지 않는다. 같은 주기를 모델과 Effect가 소비하며 사망 시 두 재생을 함께 끝낸다.

기존 WorldSequence Document/Tool/Player, Kouku projector와 Gameplay 정의, Shared 메시지, Server 전투와 owned cue, Client cue 소비자를 확장한다. 새 CPP가 필요하면 실제 소유자에 두고 vcxproj/filters에 함께 등록한다. 사용 중인 Composition/WorldSequences는 외부에서 덮어쓰지 않는다. 최종 후보는 최신 사용자 저장본과 비교해 요청 필드만 반영하고 해당 domain publisher로 게시한다. 실제 codec 저장 왕복·실패 보존, loop 경계·fit source clock, 서버 공격→HP0→정확한 cue 종료·정상 완료/취소·late join, 변경 TU 최소 컴파일을 검증한다. 제품 빌드와 사용자 화면 확인은 별도 상태로 기록한다.

## G50. 중앙 포탈·쇼타임 노이즈의 화면 중복 보호 — 2026-09-16

사용자가 중앙 오망성에서 캐릭터/쿠크와 노이즈가 겹쳐 두 개처럼 보이고 쇼타임 폭탄도 같은 현상이라고 보고했다. 실제 정본6문서를 native dispatch와 조인하면 중앙 문양2584와 폭탄 도화선2847/2805/2848은 화면 샘플이나 별도 왜곡이 없지만, 중앙 합본 포탈과 작은/큰 폭발에는 별도 distortion pass를 가진 native14개가 있다. 직접 SceneColor 샘플은 없다. 기존 노란 장판은 Decal receiver에서만 actor를 제외하며 이 별도 화면 resolve에는 적용되지 않는다.

별도 source pass14개 중11개는 상수0이므로 유지하고, 실제 왜곡을 만드는 native2461/2587/3682만 기존 RG distortion 대신 같은 RGBA16 MRT의 BA에 원본 offset을 기록한다. native 식·색·시각·크기는 유지하고 설치 생성기에도 같은 명시 범위를 반영한다. 일반 효과의 RG 왜곡은 그대로 둔다. EffectCommon의 alpha/additive/opaque RT1 write mask를RGBA로 연결하고 reveal/fill coverage를 BA에도 적용한다. 실제 compiled adapter가 소비하는 같은 fixed-function 계약을 함께 갱신한다.

Engine 정본 Deferred의 Scene resolve는 BA만 receiver를 검사한다. 현재 픽셀과 이동 후 bilinear 샘플에 actor 표식이 있으면 BA를 적용하지 않고, 다른 깊이 표면의 영상이 넘어오는 경계도 차단한다. 일반 RG를 적용한 기존 샘플과 HDR/bloom 쌍은 유지한다. marker0/5의 skinned bit8과 source skin/equipment program 범위만 actor로 해석하며 다른 Map marker의 payload를 잘못 읽지 않는다. Renderer는 이 resolve에서 Depth와 PickPos SRV를 명시적으로 바인딩한다. 별도 캐릭터 캡처나 화면 복사 pass는 만들지 않는다.

기존 Engine/Renderer·Deferred, EffectCommon과 material fixed-function 검사 및 native installer/dispatch만 확장한다. 새 제품 CPP나 프로젝트 등록은 없다. 실제 full shader compile, WARP 수치 출력으로 일반RG 보존·BA 배경 유지·actor/이동 샘플/깊이 경계 차단·HDR/bloom 동일좌표·MRT BA 전달을 검증한다. 사용자 요청대로 실행 중 Client/Server와 열린 저작 데이터는 그대로 두고 소스 검증을 진행한다. 설치 대기 중인 G49 공·인형 데이터와 제품 빌드·사용자 화면 판정은 분리한다.


## G51. F1 게시 패턴 목록의 대용량 로드 — 2026-09-16

현재 revision1143 Encounter는16,061,098bytes이며 F1 BossTool의8MiB 선행 상한에서 거절된다. Reload가 Flow 로드 전에 종료되므로 저장된6/11/5개 Flow까지 없는 것으로 표시된다. BossTool의 파일·JSON byte 상한을64MiB로 일치시키고 value 상한은4,000,000개, depth64로 제한한다. 현재385,408values/depth12를 확인했다. 실패 원인을 별도로 보존해 두 목록 안에 표시하고, 마지막 정상 목록과 미저장 Flow draft는 유지한다. 게시기는 같은 Encounter 상한을 게시 전에 검사한다. 기존 파일만 수정하며 새 C++/프로젝트 등록은 없다. 실제 생성물을 기존 로더로 읽고 잘못된 문서·상한 거절과 마지막 정상 상태 보존을 검사하며 변경TU를 격리 컴파일한다. 실행 중 Client와 Composition은 변경하지 않으며 EXE 반영과 사용자 화면 확인은 별도로 보고한다.
