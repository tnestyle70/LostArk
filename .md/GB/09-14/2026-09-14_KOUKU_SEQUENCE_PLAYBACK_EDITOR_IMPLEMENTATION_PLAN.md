# 쿠크 Sequence 재생과 Object 편집 구현 계획

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
