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

## 3. 자동 검증 증거

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

## 4. 수동·리소스·후속 경계

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
