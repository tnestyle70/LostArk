# 물리 폴더 통합 저작 환경 선택 병합 결과

## 결론

`C:\Users\user\Desktop\LostArk` 하나에서 `Framework.sln`을 열어 통합 기능과 원본 물리 리소스를 함께 사용할 수 있게
선택 병합했다. 통합 worktree 전체 병합은 하지 않았고 원본 Effect V1 `171/171` 정리 상태와 다른 세션의 최신 변경을 보존했다.
최신 `origin/main`에서 만든 `codex/physical-authoritative-unification` 브랜치에 물리 정본을 검증 단위별 커밋으로 승격했고,
열린 PR #265의 Camera/Bern 기능도 현재 Tool 계약과 semantic union했다.

Debug와 Release의 Product 및 Core가 모두 통과했다. Client·Server·Shared와 protocol harness는 protocol v46으로 같은 시점에
재빌드됐다. Client/UI는 에이전트가 실행하지 않았으며 시각·청각 최종 판정은 사용자 수동 검증으로 남아 있다.

## 실제 반영 상태

통합 브랜치의 구현 경로 117개를 원본과 비교한 결과는 다음과 같다.

- 통합본과 동일: 79개
- 원본 변경을 함께 보존한 semantic union: 27개
- 삭제 상태 일치: 6개
- 미반영 구현 코드: 0개
- 별도 기록으로 대체한 통합 worktree 문서: 5개

Git 승격은 원본의 대규모 dirty 상태를 건드리지 않도록 별도 worktree에서 수행했다.

- 물리 정본: `C:\Users\user\Desktop\LostArk`
- 통합 Git worktree: `C:\Users\user\Desktop\CodexWorkTree\LostArk-physical-authoritative`
- 통합 브랜치: `codex/physical-authoritative-unification`
- 기준: `origin/main` `68cabd25`
- 원본 물리 폴더에서는 stash, reset, checkout을 실행하지 않았다.

검증 단위별 주요 커밋은 Effect 정리 `4e013459`, Kakul intake `283add42`, Valtan Sound/8인 계약
`df1b6e2b`, 통합 Tool/Workbench `db9d8fee`, Lobby/UI `5b89d1d9`, 빌드 병목 정리 `8ae3dcc1`,
공용 문서 이동 `5e919724`, PR #265 Camera 세 커밋 `0bad3aea`/`169ec389`/`b79678fc`, Bern camera/ESC
`14f155c1`, 도끼 WAV LFS closure `834b4a3d`다. PR #265의 오래된 RESULT 문서는 복사하지 않고 이 결과서로 대체했다.
main 통합 단위는 GitHub PR #266 `feat: promote physical canonical authoring integration`이다.

## Tool과 Complete Play

- Debug Developer Tools는 여러 Tool을 동시에 렌더한다.
- 같은 toolbar 버튼을 다시 누르면 해당 Tool만 숨긴다.
- 명시적 viewport/input owner가 Map, Workbench, Camera 입력 충돌을 막는다.
- Effect V1, Effect V2, Boss, Camera, Map, HUD/UI, Workbench에 Complete Play 진입점이 연결됐다.
- Complete Play는 Valtan Arena 입장 뒤 Server-admitted saved pattern을 typed command로 재생한다.
- raw clip과 저장되지 않은 local asset preview는 Complete Play로 승격하지 않는다.
- Release Server는 debug audition을 `REJECTED_RELEASE_BUILD`로 거부하는 기존 제품 정책을 유지한다.

## Arena Active와 Debris

공용 `Server Arena Active` 패널과 Workbench가 같은 Server 복제 상태를 표시한다. Tool마다 별도 wall/debris local state를
복제하지 않으므로 Effect V1/V2, Boss, Map, UI를 동시에 열어도 상태가 갈라지지 않는다.

표시되는 read-only Active 항목은 다음과 같다.

- ordinary walls / debris sources
- 109 outer ring
- 3 o'clock floor / collision / Nav
- 9 o'clock floor / collision / Nav

변경은 Fresh, Phase 2 Walls Gone, Break 3, Break 9, Break 3+9의 exact Server preset으로만 수행한다.
패널은 debris actor 수, active collision 수, active nav region 수와 navigation revision을 함께 표시한다.

## Action Presentation Workbench

- Server stage/hit/collider/schedule/push/knockdown typed inspector
- animation occurrence와 blank duration 편집
- Effect, Sound, Camera/Shake, World event, Combat Object joined lanes
- combat-object hit sound picker와 저장
- Camera/Effect owner Tool deep-link
- 한 개의 사용자 `Save`
- Workbench 안의 Arena preset/Active와 Complete Play

현재 Resource Files는 Developer Tools 안의 읽기 전용 orchestration index다. 독립된 넓은 하단 browser, waveform 편집,
drag/drop V1/V2/Sound picker는 아직 구현 완료가 아니다.

## Effect와 Sound 물리 closure

- Effect V1 Catalog 171
- Effect V1 Authored 171
- 미등록 Authored 0
- generated artifact 0
- `Client.vcxproj`의 Data/Effects 개별 `None` 0
- V2 파일 80, Authored 75

통합 worktree의 Catalog 298 / Authored 468과 약 996 MiB 추가 Effect pack은 가져오지 않았다.

발탄 high-jump axe hit은 `G_Voltan2_Attack09_ProjExp1`에 연결됐고 WAV 4개가 원본 물리 Resources에 존재한다.
Workbench Core gate가 catalog ID뿐 아니라 실제 WAV 파일 존재도 검사한다. 새 binding이 요구하는 정확한 네 파일,
총 1,542,724 bytes는 commit `834b4a3d`에서 Git LFS로 추적했다. 다른 Valtan WAV 248개와 전체 Sound pack은 이번
feature closure에 넣지 않았다.

실제 물리 폴더가 이미 `Sound`를 독립 root로 사용하므로 public 계약도
`Fonts, Character, Deploy, Effect, Map, Sound, UI` 일곱 root로 교정했다. C++ runtime은 원래부터 Resources containment만
검사해 코드 변경은 필요 없었고, 과거 날짜별 RESULT의 여섯-root 기록은 당시 증거로 보존했다.

## KoukuSaton

물리 Resources의 사용자 컬렉션 이름은 `KoukuSaton`이며, protocol·world·Area·authoring의 기존 stable ID는
`KAKULSAYDON_ARENA`, `LV_LUT_MIDNIGHTC_ED`, `Data/Animation/.../KakulSaydon`으로 보존한다.

- `Resources/Map/LV_LUT_MIDNIGHTC_ED`
- `Resources/Character/KoukuSaton`
- `Resources/Effect/KoukuSaton`
- `Resources/Sound/KoukuSaton`

현재 Product Server Level admission, 2,951개 map placement, selected replicated player와 F1 `SL01`~`SL05` typed Server
teleport가 구현되어 있다. Character Select가 승인된 world transfer 중 live socket을 닫던 수명 버그도 수정했다.
KoukuSaton boss/NPC/pattern vertical slice와 Arena 안의 별도 Workbench character preview는 아직 구현 완료가 아니며,
이번 검증에서 맵 진입·플레이어 이동·맵 배치를 넘어 완료로 주장하지 않는다.

## Solution과 빌드 병목

- `Framework.sln` 프로젝트 12개
- 기본 Build 대상: Engine, Shared, Server, Client만
- Product CPP entry/unique: 259/259, 중복 0
- Core CPP entry/unique: 261/261, 중복 0
- EffectRenderContractHarness와 ValtanFourPlayerHarness는 solution/runner/물리 project source에서 제거
- RenderingPipeline의 한 파일 Product WARP probe는 ProjectReference 0, 제품 CPP 재컴파일 0
- Debug/Release x64 제품 프로젝트의 `/MP` 활성 상태 보존

남은 빌드 구조 병목은 Engine direct-project build와 solution build가 서로 다른 Intermediate cache를 사용하는 문제다.
현재 물리 중복 cache는 약 1.15 GiB이며 다음 빌드 최적화 우선순위다. FullDiagnostic에는 broad harness의 제품 CPP 중복
30회가 남아 있지만 Product/Core 일상 빌드에는 포함되지 않는다.

## 자동 검증

기존 warm/incremental 실측과 최종 Git worktree cold build를 분리했다.

| 검증 | 결과 | 실측 |
|---|---|---:|
| Debug Product | PASS | 약 53초 |
| Release Product | PASS | 약 78초 |
| Debug Core + local Resources | PASS | 약 72초 |
| Release Core + local Resources | PASS | 약 72초 |
| 최종 물리 폴더 Debug Core | PASS | stopwatch 미적용; Client link 14:58:59 KST |
| 최종 통합 worktree Release Core cold build | PASS | 619.69초 |

최종 Release cold 수치에는 비어 있던 worktree에서 Engine과 23개 FxCompile producer, Client 50,708 함수 LTCG를 처음 생성한
비용이 포함된다. 같은 cache의 두 번째 증분 측정은 사용자의 Ctrl+F5 검증 요청을 우선해 중단했으며 PASS 시간으로 기록하지
않는다. 중단 전에 Product Client 링크와 compiled shader closure까지는 다시 통과했다.

Core에서 함께 통과한 계약은 다음과 같다.

- Release client surface 4/4
- Build profile 7/7
- Workbench 10/10
- Kakul admission 7/7
- Valtan 8-player capacity 3/3
- Effect Tool V2 5/5
- Team LAN endpoint 3/3
- Effect source validation: direct 171, unbound 0, generated 0
- protocol v46 harness: failures 0
- Character Select private/shared world isolation live harness: failures 0
- Valtan pattern, gameplay balance, navigation, wall footprint, world destruction, rendering validators
- Client/Server/Shared/Engine project/filter XML parse
- conflict marker scan 0
- `git diff --check`: whitespace error 0, line-ending warnings만 존재

Core를 `-AllowLocalEffectResources` 없이 실행하면 Git 비추적 DDS 3개 때문에 의도적으로 실패한다. 현재 물리 pack 검증의
정본 명령은 다음과 같다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Core -AllowLocalEffectResources
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release -Profile Core -AllowLocalEffectResources
```

## 자동 검증하지 않은 경계

- Client와 UI를 에이전트가 실행·조작하지 않았다.
- 다중 Tool의 실제 layout, reclick hide, Complete Play 화면 결과, Arena wall/debris/Nav 변화와 axe sound는 사용자 판정이 필요하다.
- FullDiagnostic은 Ghost Valtan `MN_RPBF_02.wmodel`, AnimSet과 DDS 6개가 없어 차단된다.
- Release에서는 ImGui 비노출과 Lobby 4버튼 제품 화면을 검증하고, Workbench/Complete Play는 Debug에서 검증해야 한다.

## 사용자 수동 검증 순서

1. `Framework.sln`에서 Debug의 `Server + Client` profile을 실행한다.
2. Client endpoint가 `192.168.0.14:7777`인지 확인하고 Lobby의 Valtan으로 입장한다.
3. F1 Developer Tools에서 Effect V1, V2, Boss, Map, Camera, Workbench를 두 개 이상 동시에 연다.
4. 같은 Tool 버튼을 다시 눌러 그 Tool만 닫히는지 확인한다.
5. Server Arena Active의 다섯 preset을 순서대로 눌러 벽, debris, collision, Nav 상태를 육안 확인한다.
6. 각 Tool의 Complete Play로 같은 saved Valtan pattern이 Server 실제 패턴과 모든 연결 cue를 재생하는지 확인한다.
7. high-jump axe impact에서 네 variant 중 하나의 충돌음이 들리는지 확인한다.
8. Release에서는 Lobby 4버튼과 ImGui 비노출만 확인한다.
