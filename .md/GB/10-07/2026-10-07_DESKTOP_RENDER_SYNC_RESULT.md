# 2026-10-07 데스크탑 렌더링 변경 동기화 결과

## 최종 동기화 범위

사용자가 데스크탑과 동기화하기 위해 현재 Git status의 모든 변경을 PR로 전달하도록 요청했다.
현재 `codex/fix-render-quality-performance`의 선행 세 커밋도 함께 포함한다. 이들은 static
lighting bank의 중복 material 검사 감소, movie/composition sequencer lane 통합,
Colosseum 용병의 상황 판단과 가중 스킬 선택이다. 새 변경은 고성능 GPU 선택·시작 로그,
선택창의 숨겨진 showcase 해제, 텍스처 기본값 정합화, 아래 fog 최종값과 관련 문서다.
기존 Valtan 데이터 흐름 가이드와 로컬 endpoint 실행 결과도 사용자 지정 status 범위에 포함한다.
EXE/DLL/PDB/CSO·Resources·out 진단 산출물과 별도 Unreal 저장소는 포함하지 않는다.

## Fog OFF 정본

`Data/Rendering/Authored/RenderingProfiles.json`의 `scene.bern.neutral-day.v1.fog.enabled`는
false, revision은 93이다. 이전 요청의 Git 10월 2일 복원으로 켰던 revision 92의 fog ON을
사용자의 최신 명시 요청으로 대체했다. region의 원본 수치를 덮어쓰지 않고 기존 profile fog
master gate를 사용한다. 다른 profile, FXAA/SSAO/bloom/exposure/gamma와 최적화는 유지한다.
공식 publisher로 `Client/Bin/DataFiles/Rendering/RenderingProfiles.runtime.json`을 생성했다.
origin/main의 revision 91도 해당 fog는 OFF였으며 최종 게시 값은 같은 OFF다.

후보 JSON Validate와 공식 Publish는 exit 0이다. 최신 source/runtime hash 재확인 후 원자
교체했고 원본과 백업은 Git 제외 `out/DesktopRenderSync20261007`에 보존했다.
파일 게시가 실행 중 Client의 메모리 draft를 Reload했다는 뜻은 아니다.

## 실제 GPU와 무비 시작 직전 멈춤

사용자가 실행한 Release PID 22260의 11:25:38 시작 로그는 실제 RTX 4050 Laptop GPU를
기록한다. gotchas에는 이 노트북의 Debug/Release 제품 Client가 RTX 4050을 선택했는지
실제 `Graphics.Adapter` 로그로 반드시 확인하는 규칙을 추가했다. 다른 PC의 하드웨어 GPU
선택을 막는 이름·LUID 하드코딩은 추가하지 않았다.

사용자는 차원술사·워로드 무비의 재생 직전에 순간 멈춤을 보고했다. 같은 Release PID의
`Client/Bin/Release/Diagnostics/client-session-22260.jsonl` 8~10행에는 11:26:53.203,
11:27:01.760, 11:27:08.182에 메인 펌프 간격 2047ms, 1734ms, 1078ms가 기록돼 있다.
앞선 category click은 `Client/Default/CharacterSelectBrowse.user.log`의 11:26:50.834,
11:26:59.782, 11:27:06.810이다. 이들은 클릭 뒤 메인 스레드 진행 지연의 관측 근거이며
개별 class나 함수의 실행 시간을 기록한 자료는 아니다.

정적 조사에서는 `ClassSelectionPresentation::Start_Phase`가 메인 스레드에서 전체 WorldSequence
문서를 검증·복사하고 각 instance의 Prepare/Play/초기 Seek와 첫 Sample_Frame을 수행한다.
사전 모델·이펙트 준비가 이미 있으므로 이 구간의 신규 디스크 로드를 원인으로 확정할 수 없다.
당시 메모리 압박도 있어 책임 함수·페이지인·GPU 비용은 분리되지 않았다. 최신 Release의
상세 capture는 없고 이전 Debug 캡처는 이 실행의 근거로 사용할 수 없다.

사용자는 원인이 명확하지 않으면 먼저 PR을 올리도록 요청했다. 따라서 이번 동기화에서는
무비 런타임에 추측성 수정을 넣지 않았고, 순간 멈춤 해결이나 FPS 개선을 완료로 기록하지 않는다.
에이전트는 Client/UI를 자율 실행·조작하지 않았다.

## 검증 범위

GPU/showcase 변경의 기존 Debug·Release Product 빌드와 실제 RTX 선택 확인은
`2026-10-07_CHARACTER_SELECT_GPU_SELECTION_RESULT.md`의 증거를 따른다.
이 빌드는 뒤에 반영된 텍스처 기본값 변경의 검증을 대신하지 않는다.
이번 fog 변경은 C++ 빌드 없이 공식 publisher 검증·게시 대상이다.

이번 동기화에서 실제 `UserSettingsDocument.cpp`, JSON 저장 구현과 UI 기본값 함수를
사용하는 기존 계약 harness를 Debug/Release 각각 컴파일·실행했다. 두 구성 모두
134 assertions, failures 0으로 통과했다. Engine 소비자는 stub이며 제품 화면 검증은 아니다.
각 receipt와 로그는 `out/DesktopRenderSync20261007/harness-debug`, `harness-release`에 있다.
최종 authored/runtime JSON 의미값 일치와 Bern fog OFF, revision 93을 확인했고
`git diff --check`도 통과했다. Product 재빌드나 Client 자율 실행은 하지 않았다.
