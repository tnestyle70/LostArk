Session - LostArk Logo Yasuo sound bootstrap result

대응 계획: `2026-07-28_LOSTARK_LOGO_SOUND_PLAN.md`

## 1. 결과

목표를 달성했다. 수업 코드 규칙인 `NS_BEGIN(Engine)`, `CSound_Manager`, `Create()` 형태를 유지하면서 Sound의 수명은 Winters 방식대로 `CGameInstance`가 단독 소유한다. Client는 FMOD type을 직접 알지 않고 `CGameInstance::Play_Sound()` facade만 호출한다.

- 새 물리 경로: `Engine/Public/Sound/Sound_Manager.h`, `Engine/Private/Sound/Sound_Manager.cpp`
- VS filter: `Public/Sound`, `Private/Sound`
- Engine: FMOD System 생성, lazy WAV cache, channel 재생/볼륨, 매 frame update, sound/system 해제
- Client: `CLevel_Logo::Initialize()`에서 `Yasuo_Q.wav` 한 번 재생
- runtime DLL: `UpdateLib.bat`이 `fmod.dll`을 `Client/Bin`에 복사하고 실패 code를 전파
- resource: Winters 원본 WAV를 `Client/Bin/Resources/Sound/Yasuo/Yasuo_Q.wav`로 복사
- ImGui build fix: Winters 전용 `WINTERS_ENGINE_EXPORTS`를 이 프로젝트의 `ENGINE_EXPORTS`로 교체
- 이전 root의 빈 `Engine/Public/Sound_Manager.h`, `Engine/Private/Sound_Manager.cpp` placeholder는 새 물리 경로로 대체

FMOD 바이너리가 x64이므로 Sound 구현과 Logo 호출은 `_WIN64`에 한정했다.

## 2. 기대와 실제

- 기대: Engine build가 FMOD header/lib를 찾는다.
  - 실제: `Engine Debug|x64` exit code 0. `Sound_Manager.cpp` compile 및 `fmod_vc.lib` link 성공.
- 기대: 배치 후 Client 실행 폴더에 필요한 파일이 있다.
  - 실제: `Client/Bin/fmod.dll` 1,879,040 bytes, `Yasuo_Q.wav` 70,172 bytes 존재.
- 기대: Client가 새 Engine facade와 함께 링크된다.
  - 실제: `Client Debug|x64` exit code 0, `Client/Bin/Client.exe` 생성.
- 기대: Logo 진입 전에 FMOD/WAV 오류가 없고 Logo가 렌더링된다.
  - 실제: 실행 창 제목이 `Client -> 로딩이 완료되었습니다. -> 로고레벨입니다.`로 전환. `CLevel_Logo::Initialize()`의 Play 실패는 `E_FAIL`을 반환하므로 Logo 렌더 도달은 FMOD 생성·WAV load·play·volume 성공을 뜻한다.

## 3. 검증 기록

```text
Engine Debug|x64 build: PASS
UpdateLib.bat: PASS
Client Debug|x64 build: PASS
Engine.dll dependency: fmod.dll 확인
WAV duration: 0.795102 sec
Logo runtime transition: PASS
Engine.vcxproj XML: PASS
Engine.vcxproj.filters XML: PASS
git diff --check: PASS
Plan independent critique: PASS — P0 0 / P1 0
Build adaptation critique: PASS — P0 0 / P1 0
```

기존 경고는 FXC Effects deprecated, 일부 수치 축소 변환, DirectXTK/Effects11 PDB 미포함이다. build 뒤 `pwsh.exe` 미발견 문구는 기존 전역 vcpkg applocal의 PowerShell 7 탐색/fallback이며 exit code에는 영향을 주지 않았다.

## 4. 사용자가 재현하는 순서

1. Visual Studio에서 `Engine`을 `Debug|x64`로 build한다.
2. 저장소 root에서 `UpdateLib.bat`을 실행한다.
3. `Client`를 `Debug|x64`로 build하고 Ctrl+F5한다.
4. 창 제목이 `로딩이 완료되었습니다.`가 되면 Enter를 누른다.
5. Logo 화면으로 바뀌는 순간 약 0.8초 Yasuo 효과음을 확인한다.

## 5. 인계 판단

결정: 이 slice는 완료다. 다음 Sound 확장 전까지 BGM/channel enum/3D sound를 추가하지 않는다.

확신도: 높음. compile, link, DLL 배치, resource 경로, 실제 Logo 전환을 모두 검증했다. 최종 음량/청감만 사용자 환경의 스피커로 확인하면 된다.

Git commit/push는 수행하지 않았다. 기존 사용자 파일 `Client_move_plan.txt`, `Engine_move_plan.txt`, `filter_tree_audit.py`는 수정하지 않았다.
