# 쿠크 타임라인 미리보기와 서버 패턴 재생 분리 결과

## G00. 구현 완료

`Client/Private/KoukuSaydonActionWorkbench.cpp`에서 Play, Play Preview, Play Bundle을
기존 local preview 요청에 연결했다. 현재 cursor에서 재생하며 Pause/Resume와 ruler
scrub는 local preview 시간을 조작한다. 타임라인 Stop도 local preview만 일시 정지한다.

Play Pattern은 기존 applied draft snapshot의 검증, 준비, Server admission과
Collider/Logic 실행을 유지한다. Stop Pattern을 따로 두어 준비 중에는 취소를,
Server 실행 중에는 기존 audition Stop을 요청한다. 서버 준비·실행 중 local
Play/Resume/scrub를 거절하여 같은 장면에 두 재생 시간이 동시에 적용되지 않게 했다.
서버 재생 시간의 읽기 전용 표시와 Sequence Complete Play는 변경하지 않았다.

기존 MainApp의 local preview 소비자와 서버 전환 시 preview 종료 경로를 재사용했다.
새 C++ 파일, 프로젝트 등록, Shared protocol과 Server 변경은 없다. CLAUDE, 팀
인터페이스 사용서와 gotchas의 버튼 역할 설명을 실제 구현으로 수정했다.

## G01. 실행한 검증

| 검증 | 결과와 범위 |
|---|---|
| Client Release Build | PASS. VS18 Insiders amd64 MSBuild, v143 14.44.35207, x64. Client 대상만 빌드했고 project references와 runtime data publish는 끈 상태다. 오류 0개이며 기존 경고는 남아 있다. |
| Client Debug ClCompile | PASS. 같은 toolchain으로 컴파일만 수행했다. 오류 0개. Debug 링크·실행 파일 교체는 하지 않았다. |
| Native harness Debug Build | PASS. VS18 amd64 정상 toolchain으로 기존 harness를 빌드했다. |
| 집중 native transport 실행 | PASS. 실제 Workbench/Document/Shared OBJ와 기존 fixture의 transport 구간을 사용했다. cursor 시작, paused start, Pause/Resume, Pattern/Bundle scrub, one-shot 소비, 서버 준비·REQUEST_PENDING·QUEUED·ACTIVE 중 local transport 거부와 source/draft 보존을 확인했다. |
| 전체 `--kouku-preview-transport-contract` | FAIL. 새 transport 검사에 도달하기 전 `VerifyKoukuColliderSelectionGroups`의 기존 Save-conflict 단언에서 `Save conflict lost group geometry, draft or external source`가 발생했다. 사용자 파일이 아닌 임시 fixture 검사이며 이번 작업에서는 원인을 확정하거나 검사를 완화하지 않았다. 전체 harness 통과로 보지 않는다. |
| 프로젝트 XML parse | Client 프로젝트·filters와 harness 프로젝트 PASS. 등록 파일 변경 없음. |
| 변경 범위 `git diff --check` | PASS. |

집중 실행은 묶음 검사의 앞선 실패를 분리하기 위해 `out`의 TU 사본에서 기존 transport
함수를 실행했다. 원본 묶음 검사와 실패 단언을 삭제하거나 완화하지 않았다.

검증 로그와 재현 스크립트:

- `out/KoukuTimelineLocalPreview20260924-client-release.log`
- `out/KoukuTimelineLocalPreview20260924-client-debug-compile.log`
- `out/KoukuTimelineLocalPreview20260924-harness-native-toolchain-build.log`
- `out/KoukuTimelineLocalPreview20260924-preview-transport.log`
- `out/KoukuTimelineLocalPreview20260924-focused/run.log`
- `out/Run-KoukuTransportFocused20260924.ps1`

공식 Product runner는 실행 중 Debug Client/Server를 감지해 Release 작업도 preflight에서
중단했다. 실제 실행 경로와 Release 출력 경로가 다른 것을 확인한 뒤 정상 MSBuild의
Client Release 대상만 빌드했다. 이 결과를 전체 Product profile 통과로 기록하지 않는다.

## G02. 적용 상태와 남은 확인

Release Client 실행 파일은 이번 변경으로 빌드됐다. 사용자는 계속 편집하고 빌드 적용을
나중에 하겠다고 명시해 당시 Debug 실행 파일 교체는 보류했다. 이후 같은 날 피해·Effect·
Sound 편집 반영에서 Client/Server 종료 상태를 확인하고 Debug/Release 정상 Build·링크·
배포를 완료했다. 이 Play 분리 코드도 현재 Debug 실행 파일에 포함됐다. 후속 검증과 데이터
적용 상태는 `2026-09-24_KOUKU_DAMAGE_EFFECT_SOUND_EDITING_IMPLEMENTATION_RESULT.md`를 따른다.

Client와 Server의 실행·종료·UI 조작은 하지 않았다. 사용자의 최종 화면 확인과 실제
서버 연동 재생 확인은 미실행이다. 소스 및 native 검증 결과를 화면 확인으로 대신하지 않는다.

사용자가 편집 중인 `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json`의 변경은
그대로 보존했다. 이번 구현에서 authoring 데이터와 게시 runtime 데이터를 수정하거나
publish하지 않았고, 실행 중 메모리 draft를 Reload하지 않았다.
