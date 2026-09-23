# PR453 앵콜·낙사 통합 결과

## G00. 통합한 코드와 데이터

현재 revision2235와 PR453 revision2224를 merge base부터 stable ID/필드 단위로 비교했다.
1250개 노드의 양쪽 변경 보존 검사를 통과했고 병합 revision은2236이다. 근거는
`out/Pr453EncoreIntegration20260924/composition-merge.json`이다. 최신 G3/Bingo 흐름과
피해/충돌 저작값을 보존하면서 P97의 앵콜 단축과 자막 스타일을 받았다.
P10/P97 전체21.322초, 배우/카메라18.333초, 첫 균열7.5초와 파편10.433초를 확인했다.

Server WAIT_GATE의5초는 유지한다. 이 구간은 전체가 무재생인 공백이 아니다.
`Apply_GateProgressState`의G3 clear edge가 제품 `Update_RaidClear`를 시작한다. 기존
40fps 저작 UI의92frame/2.3초에flash와clear SFX,126frame/3.15초에 던전 클리어
caption이 나온다. 이 authored lead-in과 사용자가 지적한 컷씬 내부 무재생은 구분한다.
WAIT_GATE는cinematic suppression 조건이 아니며 제품 clear UI/caption은Debug/Release
공통코드다.5초 handoff에서 P10 local0이 source5000ms의clear를 이어받는다.

PR453의 가짜 던전 클리어 UI는 이 P10 첫 화면부터 local10433ms까지 표시한다.
일반 UI가 컷씬 suppression으로 사라지던 시간에도 공통 PresentationPlayer Sample과
SCENE_UI 경로가 유지하므로 Debug 전용 도구에 의존하지 않는다. fake clear는glass
ScreenPosts보다 먼저그려지고3초 cue/7.5초균열/10.433초파편으로 이어진다.
Kill Boss의Debug command도HP0/DEAD 입력만확정하고 기존boss death→WAIT_GATE→Bingo
cinematic 경로를사용한다. Release의일반전투죽음도같은공통 소비자를사용한다. UI 수명은 기존
Release_Sprites를 호출하는 session 소유권으로 바꿔 Stop/생성 실패/재생 교체에서 정리한다.
caption font를 생성 전에 검증하고, scene UI sprite의 draw 실패를 해당 sprite에서 숨겨
전체 world Render 실패로 확대하지 않게 했다.

Server 충돌은5m 낙사면·일반1/3관문 펜스·현재 승인 관문 시작점 부활을 유지했다.
관문이 미승인인 도박장 보행 테스트는 낙하 전 검증한 중앙 pin을 fallback으로 사용한다.
Mario는 별도 내부 지면이므로3관문 펜스에서 제외하고 HP0/DEAD 복귀를 연결했다.
authored TriggerMove의 낙하 보호도 유지했다. 기존 검사는 옛 tick deadline 한 번 호출에서
실제 중력의101~200tick 적분으로 바꿨고 Mario fixture를 실제3관문 상태로 보강했다.

## G01. 실행한 검증

- 기존 `test_trim_encore_lead_in.py`7개 PASS. 데이터/시간/설치 참조 검사이며 화면 검사가 아니다.
- Composition JSON parse와1250노드 병합 보존 PASS.
- 수정 source/authoring의 `git diff --check` PASS, 소유 source4개 충돌 마커0개.
- 기존 수정 C++의 UTF-8 BOM 없음과 CRLF를 보존했다. 새 C++ 파일이나 등록 변경은 없다.

## G02. 통합 담당자에게 남긴 경계

생성물은 직접 편집하지 않았다. 통합 담당자는 Kouku projector, Gameplay/Composition
publisher 성공과merge commit`0c5cfaa5f`를보고했다. Debug/Release Product 빌드와 기존
`--debug-teleport-contract-test`/쿠크 overlap 검사는 통합 담당자가 실행하여 모두 PASS했다.
debug-teleport는 raid integration을 포함하며 최종 Debug 8097개, Release 7527개 PASS다.
Client/UI 실행과4인 Release에서 clear→앵콜 연속 표시의 최종 화면 확인은 하지 않았다.

## G03. Native 검사에서 발견한 merge 회귀와 보정

통합 담당자가 새Debug Product로 실행한
`out/RaidRelease20260924/debug-kouku-object-overlap-first-failure.log`에서7개 실패가 보고됐다.
그중새hook source66개 검사는 runner의빈 synthetic catalog를참조한 fixture 오류였고,
나머지6개는bounded push/일반1·3관문펜스/body support에관한기존검사였다.

PR453이 `Advance_PlayerKnockback` 시작에추가한무조건 `Try_KoukuWalkOffFloor`가
`bKnockbackCanLeaveArena=false`나gateFence와무관하게걷기목적지에서낙사를먼저시작해,
기존bounded clamp와authored force의swept collision/Trace_ForcedSurface를우회했다.
보행입력의바닥이탈은유지하고, 넉백의해당call은Mario+명시적arena-exit에만한정했다.
일반casino force는기존swept-surface 경로, bounded/fenced force는기존clamp 경로를쓴다.
검사의 기존 조건은 완화하지 않았다. 보정 후 Debug/Release overlap은 각각660/628개
PASS, failures 0이다. 최종 로그는 같은 위치의 `{debug,release}-kouku-object-overlap.log`다.

동일실행의debug-teleport 및내장Kouku raid, 별도card-maze/npc-raid-return은통합담당자가
exit0/실패0을확인했다. 이번PlayerSimulation변경의영향검사는overlap와debug-teleport를
다시 실행하여 모두 통과했고, 독립적인 card-maze/NPC도 두 구성에서 PASS했다.
