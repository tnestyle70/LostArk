# 쿠크 이동 마커 가시성·재생 비용 개선 결과

## G00. 완료 상태와 원인

쿠크의 입구·마리오 이동 마커 23개에 최종 카메라 가시성 기반 생성·sample·제출을 연결했다.
사용자가 20:16에 직접 Debug 제품을 빌드·실행한 뒤 **“fps 복구됐어 확인됐어”**라고 확인했다.
정확한 회복 FPS 숫자와 변경 후 profiler JSON은 제공되지 않았으므로 80~100fps를 수치 확정하지 않는다.
소스 구현, focused 컴파일·CPU 검사, 사용자 빌드의 링크·배포 확인과 사용자 FPS 회복 확인은 완료했다.
에이전트는 Client/Server 실행·종료, UI 조작과 화면 캡처를 하지 않았다.

진단 기준 pull은 `29ad2df5 -> 6307ad2a`, 관련 변경은 `fb9a8bb9`의 입구 5개 → 전체 23개
marker 목록 확장이다. 사용자 JSON `profiler_20260914_194637_078_frame57_3948_0.json`은
19:46 저장이므로 뒤의 `6307ad2a -> 904303a9` pull 이전 기록이다.
새 마리오 18개는 입구에서 약 1.5~2.6km 떨어져 있지만 기존 루프가 모두 생성하고 sample했다.
비슷한 입구 기록의 map placement/visible/batch 증가는 각 1개였지만 effect occurrence와
frame rebuild는 5 → 23개였다. 문제의 주된 증가는 화면 밖 마커의 CPU 이펙트 처리다.

문제 JSON의 안정 구간 10~53프레임은 interval 평균 18.480ms, 약 54.11fps,
CPU 평균 17.725ms였다. `Effect.Render` 4.202ms와 `Effect.Service.Update` 1.814ms는
중첩되지 않는 합계 6.015ms였다. 이 합계는 기존 5개와 기타 이펙트를 포함하므로 이번 증가분
전체의 시간으로 해석하지 않는다. 과거 80~100fps의 동일 카메라 A/B 기록은 확보되지 않았다.

## G01. 실제 코드 연결

`Level_KakulSaydonArena.h/.cpp`의 기존 marker owner를 확장했다.
`Update_EntranceTriggerMarkerClocks`는 retired를 제외한 23개의 7초 scalar phase만 진행한다.
`MainApp.cpp`는 portrait 카메라를 복귀한 뒤 `Render.World` 직전에
`Submit_EntranceTriggerMarkers`를 한 번 호출한다. 실제 follow/free/연출 카메라의 view와
projection을 기존 `MapAssetRenderUtils` sphere 판정에 전달한다.

고정 asset `effect.world.move_destination`의 sphere 반경은 8m이고 신규 진입 여유 16m,
활성 유지 여유 32m를 사용한다. 작은 카메라 왕복은 24/40m sphere 차이로 흡수한다.
카메라나 predicate 입력을 평가할 수 없으면 표시를 유지한다. 임의의 플레이어 거리 제한은 없다.
최초 가시 진입에만 prepared `Spawn_LevelPlacement`와 해당 handle의 선택 commit을 수행한다.
화면 밖에서는 객체와 7초 phase를 유지하며 입자 sample과 render 제출을 생략한다.
재진입에는 같은 handle을 현재 phase로 history seek한 뒤 표시한다. 7초 wrap은 기존 seek 경로를 쓴다.

`Effect_Object.h/.cpp`는 기존 `Late_Update`의 등록 본문을 `Submit_RenderGroups`로 추출했다.
일반 Effect는 기존 자동 제출을 유지한다. Level marker는 기본 false인 명시 제출 상태를 켜고
자동 제출을 생략하므로, 최종 카메라 이후 최초 표시·재진입 프레임도 한 번만 제출한다.
`Effect_PresentationService.h/.cpp`의 `Submit_LevelPlacementSample`은 현재 Level-owned,
externally-sampled handle만 받아 기존 transform-history commit 후 같은 제출 함수를 호출한다.
숨길 때 pending sample을 취소하고 실패는 해당 marker의 stop·retire와 이유 로그로 격리한다.

Server trigger·배치 ID·retire/reload/Level 종료와 원본 입자·shader는 바꾸지 않았다.
새 제품 파일과 데이터 변경이 없어 vcxproj/filters 등록과 publisher 실행은 필요하지 않았다.
소스 7파일의 기존 UTF-8/BOM 유무와 CRLF를 보존했다. 별도 진행 중인 Composition·도구 변경은
이 기능 변경에 포함하지 않는다. 구현 브랜치는 `codex/kouku-marker-visibility-perf`이며,
같은 checkout의 완료된 피날레 복구 merge 이후 기반 HEAD는 `16d62e74`다.

## G02. 실행한 검증

| 검증 | 실제 결과와 범위 |
|---|---|
| 변경 CPP 4개 focused Debug 컴파일 | PASS, 20:12, 입력 7파일 hash 유지, out OBJ/PDB만 출력 |
| 실제 현재 메서드 본문을 추출한 lifecycle CPU 검사 | 40개 검사, 실패 0. spawn/cull/history/render는 대역 |
| 실제 Product codec/playback 검사 | 850 step, 7초 wrap 2회, 재진입 11회, 입자 행 7,587개 |
| 현재 phase 복구 대조 | 입자 개수·순서·world matrix·normalized lifetime의 최대 차이 0 |
| 원본 carrier 보수적 bounds | 중심 1.130587m, camera offset 0.15m, carrier 상계 3.310587m. 사용 sphere 8m |
| 저작 시작 카메라 fixture | margin 16/32, far 2,000/40,000 네 경우 모두 입구 5개 유지·마리오 18개 제외 |
| 기존 Effect 제출 본문 대조 | 반환값과 함수 분리 외 기존 조건·순서·부수효과 일치 |
| 최종 소스·문서 diff | `git diff --check` PASS. 프로젝트·Resources·shader 변경 없음 |

Lifecycle 검사는 최초 비가시 0 spawn/seek, 현재 phase 첫 표시, 자동 Late 중복 0,
활성 clone 재사용, 여유 영역 왕복, 비가시 100프레임 추가 sample/render 0, 재진입 rebuild,
7초 wrap, invalid delta·camera, retired 표시 억제, 실패 격리와 무관한 owner 보존을 다뤘다.
실제 카메라 fixture는 `KoukuSaydon.camera.json`, 저작 `player.spawn.kakul.party01`,
실제 camera profile helper와 MapAssetRenderUtils를 사용했다. 사용자 캡처 camera를 재현한 것은 아니다.

CPU Playback만 비교한 240프레임 × 5개 교대 표본의 중앙값은 23개 1.444571ms/frame,
5개 0.285202ms/frame으로 1.159369ms, 약 80.26% 감소했다. Stage·Spawn·service·render·GPU는
제외한 검사 수치이며 게임 전체 FPS 개선율이 아니다. 재진입 history seek 11회는 평균
3.853582ms, 최대 9.0445ms였다. 여러 마커가 한 프레임에 늦은 phase로 재진입하면 일시적인
CPU 부하가 남을 수 있다. 현재 16m 진입 여유는 화면 가장자리에 닿기 전 준비를 시작한다.

로컬 검증 산출물은 다음 경로에 보존했다. `out`은 Git에 포함하지 않는다.

- `out/KoukuMarkerPerf20260914/compile-receipt.json`, `compile.log`
- `out/KoukuMarkerPerf20260914/marker_cpu_probe.result.json`, `marker_cpu_probe.README.md`
- `out/KoukuMarkerVisibility20260914/lifecycle/result.json`, `source_receipt.json`
- `out/KoukuMarkerPerf20260914/user-build-observation.json`

## G03. 제품 빌드와 사용자 실행 확인

사용 중인 Client를 보존해 에이전트의 제품 링크는 처음 보류했다. 이후 사용자가 직접 빌드하고
**“내가 방금 반영한 거 exe 실행했어”**라고 알렸다. 다음 파일·로그를 읽어 반영을 확인했다.

- 변경 CPP 4개가 `Client/Default/x64/Debug/Client.log`의 컴파일 목록에 모두 존재한다.
- 각 최종 소스 수정 시각보다 OBJ가 새롭고, OBJ보다 제품 EXE가 새롭다. 최종 실패 이유 로그 보완도 포함한다.
- 로그 마지막에 `Client.vcxproj -> Client/Bin/Debug/Client.exe`와 shader/runtime DLL 배포가 기록됐다.
- `Client/Bin/Debug/Client.exe`는 2026-09-14 20:16:56 KST 출력, 58,573,824 bytes다.
  SHA-256은 `17afd29302b6c4f1b908dce074ba0c31b03c4e7a59b40b1fcd8b1fff1c020d79`다.
- 해당 EXE에서 새 `Level.Kouku.Markers.Prepare` profiler label과 Level placement sample 계약 문자열을 확인했다.
- 사용자 실행 프로세스는 Client PID 6392, Server PID 40908이며 둘 다 20:16:57 KST에 시작했다.

이는 사용자의 VS Debug x64 빌드·실행을 관찰한 증거다. 에이전트가 정본 Product runner를
실행했다고 기록하지 않으며 실행 중인 제품을 다시 빌드하거나 조작하지 않았다.
사용자는 이어 **“fps 복구됐어 확인됐어”**라고 결과를 확인했다. 실제 FPS 회복은 사용자 확인 완료다.
변경 후 숫자 A/B, 모든 마리오 구역·F6 재진입의 최종 visual fidelity는 별도 확인되지 않았다.
광역 하네스와 ProjectAudit은 이 C++ 변경의 완료 조건이 아니므로 실행하지 않았다.

## G04. 브랜치 전환 뒤 복구와 통합 빌드

20:16:56 EXE의 FPS 회복은 위 사용자 확인대로 유지한다. 이후 main을 거쳐 pattern-3로
checkout된 뒤 20:23:45 EXE가 변경 전 소스로 다시 만들어져 이 최적화가 빠졌음을
확인했다. 사용자 요청으로 `4de76dd6`의 관련7개 C++ 파일을 현재 pattern-3에 복구했다.
최종 lifecycle 입력 hash 및 이전 사용자 빌드 입력 hash가 복구7파일과 일치한다.
이번 복구에서 사용자 Effect/Composition 저작 JSON은 덮어쓰지 않았다.

사용자 EXE 종료와 빌드 승인 뒤 20:45:00 KST Debug Product가 PASS했다. Engine → Shared →
Server → Client 경로로 65.907초, Client54 OBJ 갱신, CSO 변경0이다. 컴파일·링크 오류와
runtime input 누락은 없다. 양손 자동 총 표시와 Composition Resources 캐시가 같은 EXE에
포함된다. 근거는 `out/BuildPipeline/runs/20260914T114500918Z-debug-product.json`과
`out/CompositionResourceCache20260914/ProductBuild`다. 새 실행의 수치 FPS와 화면 밖/재진입
동작은 사용자가 확인하며 에이전트는 Client/Server 실행·UI 조작을 하지 않았다.
