# Valtan·Bern·Party 통합 결과

## 1. 범위와 병합 기준

PR #247, PR #249와 원본 작업 폴더의 승인된 소스·데이터·문서 diff를
`codex/valtan-party-integrated-review`에서 통합했다. 기준 main은 `0a08b084`다.
원본 `C:/Users/user/Desktop/LostArk`의 브랜치·index·미커밋 파일은 유지했다.
별도 index로 만든 스냅샷 `384d83a3 → 0acac6aa → e3185f78 → 6ba598a6`이
37개 경로를 보존한다. 마지막 스냅샷에는 사용자가 추가 저장한 floor-wipe second-smash
Effect도 원본 bytes 그대로 포함했다.

사용자의 최신 지시는 확인된 치명적 문제를 먼저 수정한 뒤 병합하고, 장시간 전체
회귀는 병합 후 계속하는 것이다. 이 문서는 전체 회귀나 사용자 visual PASS 선언이 아니다.
실제 원격 병합 여부와 commit은 GitHub PR 이력으로 확인한다.

## 2. 구현 및 보존

- 109 붕괴의 외벽 30개 + 내부 잔여 벽 67개, 이미 적용된 transition의 중복 격리와
  payload 충돌 시 commit 이전 실패를 연결했다. Shared snapshot event 상한은 106이다.
- Bern의 동일 frame camera/bounds 경로와 conservative culling margin·reject grace를
  함께 유지했다. 전역 culling bypass는 추가하지 않았다.
- 입장 카메라 19.867초, Server 무적 gate, 마지막 follow 복귀와 M04→M05→M06
  BGM 선택 경로를 유지했다. 카메라 도구는 기존 JSON/CAS 저장 경로를 사용한다.
- 파티 초대는 inviter identity를 검증하고, 리더의 2~4인 이동은 target admission·초기
  payload·모든 송신 큐 예약이 성공한 뒤에만 source와 binding을 일괄 commit한다.
  실패는 source 파티를 보존하고 typed reason을 Client에 알린다.
- 서로 다른 v40 변경을 합쳤으므로 최종 network protocol은 **41**이다.
  기존 Server/Client와 혼용하지 않고 둘 다 같은 revision으로 빌드·재시작해야 한다.
- Party UI가 소비한 LB/RB는 물리 release까지 gameplay로 넘기지 않는다. 파티 HP는
  replicated entity/tick에 연결하고 미수신 값을 만피로 표시하지 않는다.
- 강화 반복 효과음과 BGM의 채널 소유권을 분리하고 새 채널 준비 실패 시 기존 재생을
  보존한다. Sound parser는 명시적으로 억제된 NONE/빈 clip만 격리한다.
- Boss → Effect 도구는 정확한 product occurrence tuple을 one-shot 전달하고 자동
  재생하지 않는다. unlink 중 deep-link를 막으며 사용자 저장본과 unlink subset을 유지한다.
- Flow 검증은 실제 저장본과 in-memory seed/mutation을 분리했다. 사용자 29-slot
  JSON의 bytes는 변경하지 않았다. SHA-256은
  `635fdbbcafe3b4b9abc455f206575091e79f6b0a6aa56f8993f656397842afcb`다.

Next Pattern·Trash·네트워크 자동 탐색/복구의 다른 작업 문서는 계획/인계 자료로만
보존한다. 이번 통합에서 그 runtime을 새로 구현하거나 사용자 슬롯을 삭제하지 않았다.

## 3. 병합 전 자동 검증 증거

| 범위 | 실제 확인 상태 |
|---|---|
| Engine Debug/Release 및 UpdateLib | PASS |
| Shared/NetworkProtocolHarness Debug/Release 빌드·실행 | PASS, failures 0 |
| Server Debug/Release 빌드 | 마지막 테스트 fixture 교정 후 모두 PASS |
| Server Release contract | 최종 PASS, failures 0 |
| Server Debug contract | 파티·수정 3건 PASS; 사용자 Server의 전역 mutex 점유로 환경 실패 1건 |
| CharacterSelectIsolationHarness Debug/Release 빌드 | PASS; 새 live 2/4인 경로 실행은 후속 |
| Client Debug | 마지막 parser의 이전 object를 제거하고 frozen 최종 소스로 재컴파일·링크 PASS |
| Client Release | 최종 통합 빌드 PASS |
| ActionPresentationTimelineHarness Debug | 최종 실제 parser/input/audio/HP/notice 회귀 PASS |
| Flow + PatternTree + All Effects + Boss Tool | 최종 통합본 79 tests PASS (Flow 자체 22) |
| Effect/Boss focused suite | 최종 Flow 인계 전 115 tests OK, 7 skipped |
| Valtan V2 suite / 직접 validator | 51 tests OK; source 196 / resources 1028 검사 PASS |
| World destruction publisher Validate | PASS, groups 105 / bindings 224 |
| Artist HLSL runtime oracle | PASS, samples 200, maxError 1.14440918e-05, statePilots 4 |
| Floor emissive / rendering profiles / ground target scope | 각각 7 / 3 / 6 tests PASS |
| 변경 tracked JSON/XML parse | PASS |

빌드 로그는 통합 폴더의 Git 제외 `.codex_tmp/integration-*`에, Action 최종 로그는
`.codex_tmp/client-party-action-debug-final.log`에 남긴다. 로그와 중간 산출물은 커밋하지 않는다.

### 병합 전 실패의 실제 원인과 조치

1. Client HP cache의 unsigned snapshot 값이 signed aggregate에 축소 변환되어 빌드가
   실패했다. 원래 wire type과 같은 unsigned 저장으로 수정했다.
2. Action harness가 실제 Effect parser 구현을 링크하지 못했다. 기존 CPU parser의
   실제 dependency closure를 project/filter에 등록했으며 대체 stub은 만들지 않았다.
3. 마지막 sound 보강이 retired NONE/빈 clip의 stage를 먼저 검증하여 정상 저장 데이터를
   막았다. 명시적 suppression을 먼저 격리하고 활성 stage/occurrence와 NONE+비어 있지
   않은 clip은 계속 거부한다. 최종 519개 입력 중 활성 501개, 미래 14개와 억제 4개로 검증했다.
4. Server malformed-payload fixture는 비어 있는 Valtan entity 목록에 오류를 주입하지
   않아 정상 이동했고, 뒤 busy-queue 검사도 연쇄 실패했다. 실제 invalid entity를 항상
   주입·복원하도록 수정했다. 정상·실패 파티 계약은 수정 후 Debug/Release 모두 PASS다.
5. Valtan topology의 bootstrap scalar action은 24개지만 runtime catalog에는 volley
   action 1개가 추가된다. 25 runtime actions와 정확히 1 volley를 검사하도록 교정했다.

Debug Server의 durable-owner reset 검사는 제품과 같은 전역 mutex를 사용한다.
실행 중인 사용자 Debug Server PID 11248과 충돌한 환경 실패가 관찰됐다. 사용자 Server를
종료하거나 제품의 단일 owner 보호를 해제하지 않았다. 이를 전체 Debug suite PASS로 세지 않는다.

## 4. 병합 시점의 수동·리소스·후속 경계

아래는 PR #250 병합 시점의 상태다. 후속 실행과 해소 여부는 6절을 따른다.

- Client/UI를 자동 실행·조작·캡처하지 않았다. 카메라 구도, 벽 잔해, Bern clipping,
  Effect fidelity, 실제 음악과 파티 UI 화면은 사용자의 직접 확인 대상이다.
- 현재 물리 Resources에는 Bern BGM 1개와 Valtan BGM 7개 WAV가 없다.
  JSON/선택 경로가 유효해도 실제 소리 재생 완료를 의미하지 않는다.
- EffectRender headless 검사는 clean worktree의 물리 resource root 누락 경계가 남았다.
  해당 검사를 PASS로 기록하거나 전체 pack을 Git에 추가하지 않았다.
- 병합 후 정본 Debug/Release 전체 회귀, Action Release, 격리된 2/4인 실제 socket 이동,
  Valtan four-player harness를 실행하고 코드 결함은 후속 기능 브랜치/PR에서 수정한다.
- 원본 폴더는 보존된 dirty worktree다. 자동 pull/reset/stash를 수행하지 않았으므로,
  원격 main 병합과 원본 폴더의 최신 빌드 적용을 같은 상태로 취급하지 않는다.

## 5. 원격 병합 완료

- 통합 PR: https://github.com/tnestyle70/LostArk/pull/250
- main merge commit: `cd120501da95204219a696efad1aad2034e1808b`
- 통합 head `3c66df74`와 PR #247/#249 및 최종 사용자 스냅샷이 main의 ancestor임을 확인했다.
- GitHub에서 #247, #249, #250 세 PR 모두 `MERGED` 상태를 확인했다.

## 6. 병합 후 추가 회귀

후속 브랜치: `codex/post-merge-valtan-party-regression`.

### 실행에서 찾은 검증 문제와 수정

1. **Debug owner fixture 격리:** 같은 전역 이름 때문에 사용자 Server와 충돌하던 테스트를
   private named mutex helper로 분리했다. 제품 wrapper의 전역 이름·Win32 보호는 그대로다.
   fixture만 PID별 이름을 사용해 정상 획득, 다른 thread의 동시 owner 거부, release 뒤
   재획득과 invalid 이름 거부를 검사한다. Debug/Release Server contract 모두 failures 0이다.
2. **Live 이동 검사의 누적 시간:** 실제 guide 접근이 약 18초이므로 solo/2인/4인을 한
   process에 누적하면 기존 55초 watchdog을 넘었다. runner는 Core/Party2/Party4를 각각
   새 owned Server에서 실행한다. 제품 Server의 60초 상한과 각 process의 기존 timeout을
   늘리지 않았다. native invalid CLI 6가지도 각 configuration에서 거부됨을 확인했다.
3. **Effect 외부 Resources:** headless harness가 호출자의 resource root를 checkout의
   경로로 덮어쓰던 문제를 수정했다. 기존 `CRuntimeAssetRoot` 우선순위를 재사용하며
   잘못된 명시 경로는 fallback 없이 실패한다. 사용자 Client/Engine 동작은 바꾸지 않았다.
   PowerShell 7에서 빈 환경 변수를 명시 경로로 오해하던 검증 문제도 수정했다. 값이 빈
   경우의 정책을 제품 resolver와 일치시켰고 PowerShell 5/7에서 같은 결과를 확인했다.
4. **Editable Effect의 오래된 assertion:** 사용자가 floor-wipe Effect에서 decal을 제거한
   저장본에 과거 고정 owner를 강제하던 한 테스트를 교정했다. 다른 exact owner와 예상치
   못한 owner 거부는 유지했다. 저장 데이터·리소스는 수정하지 않았다.
5. **Native 실패 코드 전파:** 호출자의 지역 `LASTEXITCODE=0`이 하위 native 실행의
   실패를 가리던 문제를 재현했다. 정본 runner, Action/Effect/PointLight wrapper와 Valtan
   projector/test wrapper가 전역 자동 변수를 읽도록 교정했다. 여섯 script의 AST 검사와
   실제 잘못된 resource root를 지역 success sentinel 아래 실행하는 거부 검사를 정본
   회귀에 추가했다. 명시적으로 실패해야 하는 입력이 성공으로 집계되지 않음을 확인했다.

### 후속 자동 검증

| 범위 | 실제 결과 |
|---|---|
| 정본 Release 자동 회귀 | 최종 PASS; 앞선 개별 빌드 완료 후 `-SkipBuild`로 전체 gate 재실행 |
| 정본 Debug 자동 회귀 | V2/data/Network/Server PASS 뒤 사용자 Server owner 충돌로 live 단계 중단; 전체 PASS 아님 |
| Server Debug/Release build + contract | 모두 PASS, failures 0 |
| ActionPresentationTimelineHarness Debug/Release | 최종 실제 parser/input/audio/HP/notice 회귀 PASS |
| Release Valtan four-player socket harness | 4/4·5번째 거부·재접속·빈 방 재입장 PASS |
| Release Character Select/Bern core socket harness | private isolation·공유 Bern·solo 이동 후 입력 PASS |
| Release 2/4인 party socket harness | 정확한 roster/leader와 이동 후 gameplay command PASS |
| Bounded party CLI malformed/duplicate/conflicting options | Debug/Release 각 6개 거부 PASS |
| Effect headless renderer | Debug/Release 각각 26 frames PASS; visual fidelity 판정 아님 |
| Effect resource-root native gate | PowerShell 5/7 × Debug/Release, 각 8 cases PASS |
| Native exit-code propagation gate | Debug/PowerShell 7, Release/PowerShell 5·7 PASS; 여섯 AST guard + 실제 native 실패 전파 |
| PointLightFalloffContractHarness Debug/Release | 최종 실행 PASS |
| 최종 Valtan V2 | 51 tests PASS; 정본 Release 재실행 335.674초 |
| 최종 Effect/Boss/Flow focused | 142 tests OK, 기존 경로 7 skipped |
| Camera/Bern frustum/world/source/gameplay validators | PASS |
| World floor / gap filler / floor emissive | 각각 8 / 6 / 7 tests PASS |
| 후속 변경 PowerShell/XML parse 및 diff check | 9 scripts / 2 XML PASS, `git diff --check` PASS |

최종 정본 실행은 `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release
-SkipBuild -ResourceRoot C:/Users/user/Desktop/LostArk/Client/Bin/Resources`다. 앞서 완료한
Engine/Shared/Network/Server/Client와 native harness의 Debug/Release 개별 빌드 결과를
사용했다. 최종 로그는 통합 폴더의 Git 제외
`.codex_tmp/post-merge-canonical-Release-final.txt`에 있으며 `Regression completed: Release`를
확인했다. Debug 개별 최종 로그는 `.codex_tmp/post-merge-effect-debug-final.txt`와
`.codex_tmp/postmerge-owner-isolation-Debug.txt` 등에 보존한다.

독립 읽기 전용 교차 검토에서 입력·HP·음악 수명, bounded process cleanup, 카메라·97-wall·
106-event·Bern 정책의 새 P1/P2는 찾지 못했다. 테스트 통과를 모든 시각 결과나 무회귀의
절대적 보증으로 해석하지 않는다.

### 기존 실패와 남은 경계

확장 `test_valtan_requested_effect_elements.py`의 21 tests에서 52개의 subtest failure가
관찰됐다. 이 검사의 대상 17개 Authored 문서와 test/generator는 병합 전 main `0a08b084`,
사용자 스냅샷 `6ba598a6`, 병합본 `cd120501`에서 전부 동일한 Git blob이다.
현재 editable 저장본에 과거 generator의 element 수/ID/timing을 강제하는 기존 검사이며,
이번 병합 회귀로 분류하거나 사용자 데이터를 자동 복원하지 않았다. 이 suite는 PASS가 아니다.

제품 Debug live Server는 계속 실제 전역 owner 보호를 사용한다. 사용자 Server PID11248가
실행 중인 동안 새 Debug live Server를 띄우는 검사는 막히며, 테스트 전용 namespace를 제품
실행의 우회 옵션으로 노출하지 않았다. 실제 UI/audio 수동 확인과 누락 BGM 파일의 경계도 남는다.

정본 Debug runner도 V2·data·Network·Server 계약까지 통과한 뒤 live Server 시작에서
위 단일 owner 보호로 중단됐다. 전체 Debug 정본 회귀는 PASS가 아니다. 그 뒤 단계의
Action·Effect·PointLight Debug는 별도로 직접 실행해 PASS를 확인했다. 사용자 Client/UI와
실행 중인 Server는 조작하거나 종료하지 않았다.

누락 BGM은 코드가 참조하는 다음 Resources-relative asset ID다. 원본 물리 폴더
`C:/Users/user/Desktop/LostArk/Client/Bin/Resources/` 아래에 없고, 이 8개 WAV는 Git
dependency closure에도 포함되어 있지 않다. 임의 대체 음원이나 전체 resource pack을
추가하지 않았다. 실제 재생 확인 전 리소스 담당자의 파일 배치가 필요하다.

- `Sound/BGM/BernCastle/bgm_berntown_mscene01_thecapital.wav`
- `Sound/BGM/Valtan/M01_KeepGoing__992459057.wav`
- `Sound/BGM/Valtan/M04_KeepGoing2__106505321.wav`
- `Sound/BGM/Valtan/EventMixes/bgm_heartrb_ed_m05_mscene_valtanrevive.wav`
- `Sound/BGM/Valtan/EventMixes/bgm_heartrb_ed_m06_battle_valtan_1stphase_out.wav`
- `Sound/BGM/Valtan/M07_FakeDead__477456395.wav`
- `Sound/BGM/Valtan/M08_ValtanPhase2__575767475.wav`
- `Sound/BGM/Valtan/EventMixes/bgm_heartrb_ed_m09_mscene_valtandead.wav`
