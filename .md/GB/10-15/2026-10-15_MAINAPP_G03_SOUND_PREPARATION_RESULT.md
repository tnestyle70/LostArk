# G03 UI 음원 준비와 category 적용 결과

2026-10-15 KST. 사용자가 작성한 G03을 검토하고 누락·오류를 실제 소스에 보정했다.
Debug x64 Product 빌드·링크·배포와 실제 Sound Manager/FMOD 반환값 검사 9건이 통과했다.
Client 실행, UI 조작, 청취, volume/focus의 실제 재생 확인은 수행하지 않았다.

- 반영 위치와 현재 7파일 전문: [G03 PLAN](2026-10-15_MAINAPP_G03_SOUND_PREPARATION_PLAN.md)
- 전체 책임·단계: [MainApp DESIGN](../10-14/2026-10-14_MAINAPP_RESPONSIBILITY_REFACTOR_DESIGN.md)

## 검토 후 실제 보정

1. `Sound_Manager.cpp`의 `Play_OneShotSound`가 `E_NOTIMPL`만 반환하던 상태였다.
   기존 두 인자 Play_Sound의 focus/cache/playSound/volume 본문을 이 공통 함수로 옮기고,
   두 인자 함수도 이 구현을 호출하도록 연결했다. 기존 실패 시 채널 정리 정책은 변경하지 않았다.
2. `Prepare_Sound`가 `S_OK/E_FAIL`을 `bool_t` 변수에 담아 반환했다.
   E_FAIL이 bool의 true, 다시 HRESULT의 1(S_FALSE)로 바뀌어 SUCCEEDED가 실패를 성공으로 판단했다.
   이제 `nullptr != Find_Or_LoadSound(...) ? S_OK : E_FAIL`을 HRESULT 그대로 반환한다.
3. `Play_UISound`의 마지막 호출이 기존 두 인자 API에 남아 있었다.
   세 인자 API에 `Engine::SOUND_CATEGORY::INTERFACE`를 전달하도록 연결했다.
4. `CMainApp::Initialize`의 Esther sound catalog 준비 뒤, 첫 HUD 생성 앞에
   `Prepare_UISounds()`를 한 번 연결했다.

사용자가 작성한 GameInstance 중계 선언·정의, UISoundPlayer 선언·준비 루프는 유지했다.
category switch의 질문에는 END/미지원 값의 E_INVALIDARG와 유효 category의 group 부재 E_FAIL을
구분하는 주석을 추가했다. asset ID와 실제 경로의 차이를 설명하고 실패 로그의 구분자를 보완했다.
사용자의 질문 메모·무관한 코드·파일 인코딩을 보존했다. Sound_Manager.cpp는 CP949 그대로다.

이번 검토의 직접 수정은 Sound_Manager.h/.cpp, UISoundPlayer.cpp, MainApp.cpp 네 파일이다.
G03 전체 계약은 기존 일곱 파일에 걸쳐 있고 신규 H/CPP는 없다. 프로젝트 등록 추가도 없다.
EngineSDK는 공식 Product 빌드로 갱신했다. 소스 API 소비 규칙은 팀 사용서의 사운드 설명에 반영했다.

## 준비 범위와 소유권

네 항목은 현재 UI_SOUND_CUE의 BUTTON_CLICK/WINDOW_OPEN/WINDOW_CLOSE/POPUP_REQUEST다.
프로젝트의 모든 UI/게임 음원이 네 개라는 뜻은 아니다. 시작부터 쓰는 공통 UI cue를 이 목록으로
선적재하며, 다른 음원의 선택·준비 시점은 기존 캐릭터·보스·Sequence 기능이 맡는다.

`Initialize → Prepare_UISounds → GameInstance → Sound_Manager → Find_Or_LoadSound`가 실제 연결이다.
재생 시 최초 적재는 기존 cache 경로에 이미 있고, Prepare는 같은 자원을 먼저 준비하는 API다.
별도 cache나 channel manager를 만들지 않았고 channel 생성 비용·첫 클릭 시간 개선은 측정하지 않았다.
UI 함수 묶음에는 소유할 장치·cache·handle 상태가 없어 namespace 함수로 두었다.
H는 외부 호출 선언, CPP는 정의, 익명 namespace는 해당 CPP 안의 cue 매핑 세부 구현이다.

## 실행한 검증

| 검증 | 실제 결과 |
|---|---|
| 독립 코드 검토 | 공통 재생 본문·HRESULT·INTERFACE·초기화 위치 재검토 통과 |
| Debug Product | Engine/Shared/Server/Client 단계 PASS, SkipBuild=false |
| Engine/SDK/Client 연결 | Engine 컴파일·링크 및 SDK 갱신, Client 컴파일·링크·배포 성공 |
| 런타임 파일 검사 | missingRuntimeInputs/invalidRuntimeInputs 비어 있음. publish 없음 |
| 실제 Sound_Manager + FMOD 콘솔 | 컴파일·링크·실행 exit 0, assertions 9, failures 0 |
| 이번 보정 diff | before/after `git diff --no-index --check` 출력 없음 |
| Client/UI/청취 | 실행하지 않음 |

실제 Product 명령:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File 'Tools/Build/Invoke-BuildAndRegression.ps1' -Configuration Debug -Profile Product -MaxCompilerProcesses 4 -BuildLogDirectory 'out/MainAppRefactor/g03-review/build-debug'
```

빌드 시작은 2026-10-15 01:00:45 KST, 종료는 01:09:33 KST다.
전체 528338ms, Engine 42243ms, Client 473232ms였다. Client 산출물 쓰기 수는 OBJ=232,
PCH=0, CSO=0, binaries=2다. 이 수는 변경된 G03 파일 수가 아니다.
사용자·다른 세션의 기존 미커밋 변경을 포함하는 당시 작업본의 빌드 결과다.

반환값 검사는 제품의 Sound_Manager.cpp와 실제 fmod_vc.lib를 독립 콘솔에 링크했다.
빈 경로 E_INVALIDARG, 없는 파일 E_FAIL, 실제 WAV의 one-shot/loop 준비 각각 2회 S_OK,
성공 뒤 없는 파일 E_FAIL, END/범위 밖 category E_INVALIDARG를 확인했다.
실제 FMOD 초기화·음원 적재는 수행했으나 유효 category의 Play, cue, music은 요청하지 않았다.
반복 준비 성공을 private cache 개수나 동일 포인터 검증으로 해석하지 않는다.
검사에 사용한 Resources WAV는 읽기만 했으며 이동·삭제하지 않았다.

- [Product 결과](C:/Users/tnest/Desktop/LostArk/out/BuildPipeline/runs/20261014T160933729Z-debug-product.json)
- [빌드 로그](C:/Users/tnest/Desktop/LostArk/out/MainAppRefactor/g03-review/product-debug.log)
- [보정 diff](C:/Users/tnest/Desktop/LostArk/out/MainAppRefactor/g03-review/repair.diff)
- [보정 hash·인코딩](C:/Users/tnest/Desktop/LostArk/out/MainAppRefactor/g03-review/repair-manifest.json)
- [반환값 검사 코드](C:/Users/tnest/Desktop/LostArk/out/MainAppRefactor/g03-review/hresult-harness/SoundPrepareHarness.cpp)
- [반환값 검사 로그](C:/Users/tnest/Desktop/LostArk/out/MainAppRefactor/g03-review/hresult-harness/run.log)
- [최종 소스·문서·XML 대조](C:/Users/tnest/Desktop/LostArk/out/MainAppRefactor/g03-review/final-verification.json): G03 실제 7파일과 G04/G05/G06-a 후보 15파일 전문 일치, XML 6개 parse, 보정 hash 4개 일치.

## 남아 있는 검증과 경고

Release Product 빌드는 실행하지 않았다. 첫 클릭, 네 창 토글, 팝업 소리, UI volume 0,
focus mute, 문자 입력·시네마틱 입력 억제는 사용자가 Client에서 확인한다.
이번 검사로 channel routing의 실제 청취나 첫 재생 시간·메모리 개선을 주장하지 않는다.

기존 문자셋 C4819/C4828, 변환 경고, DirectXTK.pdb 누락 LNK4099가 남아 있다.
변경 대상 파일 전체의 `git diff --check`에는 사용자의 기존 공백과 CP949 전처리 경계상
유지한 MainApp 주석 공백 등이 잡힌다. 전체 작업본이 whitespace-clean하다고 기록하지 않는다.
이번 보정 전후 diff의 추가 공백 오류는 없다. 사용자 프로세스를 종료하거나 Client/Server를 실행하지 않았다.

## 다음 직접 구현 단계

G04 도끼 편집, G05 용·카메라 패널, G06-a Profiler overlay의 PLAN은 후보 코드다.
제품에는 적용하지 않았다. 각각 앞쪽의 추가·교체 위치를 먼저 보고 뒤의 전체 파일과 대조한다.
후보 문법 검사를 제품 링크·사용자 화면 검증과 구분한다.
