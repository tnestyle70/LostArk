# 발탄 Play Preview 비정상 종료 조사·검증 결과

## G00. 확인된 비정상 종료와 원인

사용자가 경험한 종료는 `return 0`에 의한 정상 종료가 아니다.
사용자는 후속 확인에서 `Play Preview`를 누르자마자 종료됐다고 설명했다.
Windows WER/이벤트의 `Client.exe` 오류는 2026-09-27 22:09:19의
`0xc0000005` 접근 위반이다. WER PE timestamp `0x6ab912e8`와 당시 설치 EXE가
일치하며 fault RVA `0x1bf0354`는 MSVC 14.44 `xstring:2848`의
`std::basic_string<char>::_Equal`이다. 오른쪽 문자열 객체의 크기와 data 포인터
읽기는 통과했고, 왼쪽 문자열 객체의 크기를 읽는 명령에서 중단했다.

현재 Debug 프로그램에는 같은 `CAnimation_Tool` 클래스의 서로 다른 멤버 배치가
혼합되어 있었다. `Animation_Tool.h`는 9월 27일 08:20:48 갱신됐고 대부분의
관련 OBJ는 21:53에 다시 생성됐지만 다음 두 OBJ는 9월 24일 07:29 상태였다.

- `Animation_Tool_CompositionSounds.obj`
- `Animation_Tool_ValtanInspector.obj`

Disassembly로 확인한 `Can_CommitValtanCompositionPatternSoundGeneration`의
`m_pValtanBossTool` 주소는 기존 unit에서 `this + 0x1348`, 동일 현재 소스를
재컴파일한 unit에서 `this + 0x13A8`이다. 현재 객체의 필드를 96바이트 이전에서
읽는 ABI 불일치다. 기존 소스의 null 검사도 다른 필드를 읽으므로 보호가 되지 않는다.

## G01. 실제 제품 코드 A/B 재현

진단 산출물: `out/ValtanPreviewCrash20260927/native`.
`probe.cpp`는 WARP D3D를 사용하고 창/Client UI/서버를 만들지 않는다.
실제 `CLevel_ValtanArena` placement owner, `CBalanceTool`, `CValtanBossTool`,
`CCharacterPreviewPanel`, `CAnimation_Tool`과 설치된 발탄 model/animation,
실제 `VALTAN_WHIRLWIND` 문서를 사용했다. 제품 frame 전체나 render는 호출하지 않았다.

| 구성 | 실제 실행 결과 |
|---|---|
| 당시 제품 Debug OBJ archive | `Play_ValtanCompositionDraftPattern` 도중 `0xc0000005`, exit 3221225477 |
| 나머지 OBJ 동일, 위 두 unit만 현재 소스로 재컴파일 | Play 성공, Seek 0 성공, 300회 Update 성공, 중지 성공, exit 0 |
| 정상 Debug 빌드 후 제품 OBJ archive만 다시 링크 | 같은 Play/Seek/300 Update/중지 성공, exit 0 |

기존 구성의 native stack은
`Play_ValtanCompositionDraftPattern -> Start_ValtanPatternMasterPreview ->
Reload_ValtanPatternSoundCues -> Can_CommitValtanCompositionPatternSoundGeneration ->
CValtanBossTool::Can_CommitPatternSoundGeneration -> optional::operator bool`이다.
BossTool을 실제 객체로 제공해도 같은 잘못된 offset 때문에 실패했다.
현재 소스로 컴파일한 동일 경로는 boss 스폰 없이 정상 통과했다.

이 A/B는 설치 Debug의 mixed layout 결함과 해당 local preview 경로 실패를
직접 확인한다. 당시 사용자 WER fault는 문자열 비교이고 이번 native fault는
잘못된 BossTool 포인터의 optional 읽기이므로 동일 과거 stack을 복원했다고
기록하지 않는다. 과거 dump는 소실되어 원래 caller를 완전히 확정할 수 없다.
미스폰은 정상 local preview 생성 경로를 처음 실행하는 조건이며, Server boss가
반드시 있어야 하는 기능이라는 근거는 없다.

증거 파일은 `baseline-crash.log`, `candidate-pass.log`,
`stale-composition.disasm.txt`, `fresh-composition.disasm.txt`이다.
상위 폴더에는 원래 WER, symbol, crash evidence가 보존돼 있다.

## G02. 제품 반영과 남은 검증 경계

통합 담당의 정상 Debug Product 빌드는 22:28:06 성공했다.
변경된 컴파일 산출물은 문제의 OBJ 2개이며 PCH 0개, shader CSO 0개,
Client binary 1개다. 빌드 기록은
`out/BuildPipeline/runs/20260927T132806835Z-debug-product.json`과
`out/ValtanPreviewCrash20260927/product-debug.log`다.

빌드 전 두 stale unit의 CL tracking에는 `Animation_Tool.h` dependency가 빠져 있었다.
정상 빌드 후 현재 헤더를 직접 include하는 15개 unit 모두 헤더를 추적하며
헤더보다 과거의 OBJ가 없다. `tracking-before.json`과 `tracking-after.json`으로
확인했다. 새 null 검사나 영구적인 프로젝트 입력 우회 없이 정상 dependency가 복구됐다.

그 뒤 native probe를 `--refresh-archive --skip-compile --run`으로 실행했다.
현재 제품 OBJ로 archive를 다시 만들고 기존 진단 probe에 링크했으며,
임시로 새로 컴파일한 candidate unit을 우선 링크하지 않았다.
실제 제품 산출물에서 같은 no-server Play/Seek 0/300 Update가 exit 0으로 끝났다.
이 최종 제품 검증은 당시 도구 응답의 `archive 0 / link 0 / run 0`와
`PASS 300 updates no server/no windows` 출력으로 확인했다.
`native/run.log`는 이후 같은 진단을 재사용한 타임라인 이펙트 시계 검증으로
교체되었으므로 현재 파일을 이 제품 OBJ-only 검증의 로그로 인용하지 않는다.
당시 진단 소스는 `native/crash-probe.cpp.saved`에 보존돼 있다.
300 Update는 해당 호출들이 예외 없이 완료된 검증이며, 제품 전체 frame/render 또는
사용자 화면 동작을 검증했다는 뜻은 아니다.

제품 preview C++ 동작은 변경하지 않았다. 기존 local clone 및 실패 처리
계약을 바꿀 필요 없이 같은 source의 일관된 빌드로 A/B 문제가 사라졌다.

사용자의 실제 아레나 화면에서 `Play Preview`를 누르는 검증은 수행하지 않았다.
texture 수정/화면 결과는 별도 바닥 수정 RESULT가 소유한다.
