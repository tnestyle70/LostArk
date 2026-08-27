# 발탄 Cinematic Camera Tool 구현 결과

## 0. 결론

`ValtanCinematicCamera.json` camera-only format 5를 편집·검증·저장하고 기존
`CValtanCinematicCameraController` sampler로 미리 보는 전용 Camera Tool 수직 슬라이스를 구현했다.
`CameraCuts.json`, Catmull-Rom 또는 Client-local cinematic runtime은 추가하지 않았다.

Camera cue/death cue와 keyframe의 draft 편집, 현재 pipeline camera Capture, Play/Pause/Stop/Scrub,
Boss Tool typed deep-link, strict whole-document validation, exact-byte CAS와 sibling temporary atomic save,
Tool 종료·F1 전환·Level 전환·Server cinematic 선점 시 camera 복구까지 연결했다.

구현과 camera-only source/compile 검증은 완료했다. 최종 공유 worktree Client Debug/Release canonical
build는 통합 세션에서 진행 중이며, camera focused harness와 publisher는 Debug/Release 모두 통과했다.
Client를 실행하거나 화면을 자동 조작하지 않았으므로 visual PASS는 아니다.

## 1. 구현 상태

### 1.1 camera-only 문서와 제품 sampler

- `CValtanCinematicCameraDocument::Stage_CameraDraft/Serialize_Text`를 추가했다.
- format 5의 `cues`, `deathCue`, `provenance`를 strict parse 뒤 의미 손실 없이 roundtrip한다.
- 기존 `skyCues`, sky controller policy/state와 Level의 six-proxy Apply/Reset/Clear 경로를 제거했다.
  old formatVersion 4 또는 `skyCues` 재주입은 strict parse가 거부하고 기존 ready document를 보존한다.
- Product sky는 Map Effect 단일 권위다. 기존 ChaosGate proxy 3장은 catalog/profile/placement에서
  제거했고, reference camera가 실제 소비하는 SpaceHole 3장은 `DEBUG_REFERENCE_*` identity와
  debug-only evidence로 분리했다.
- camera/death cue ID는 `[A-Za-z0-9_.-]`, 최대 128자의 stable ID 계약으로 검사한다.
- 제품 Controller와 Tool이 같은 `CValtanCinematicCameraController::Sample_Cue`를 호출한다.
- BOSS_XZ/BOSS_FACING/PLAYER_BOSS_FRAME Tool preview는 로컬 boss GameObject를 직접 찾지 않는다.
  Level이 제품 runtime과 같은 현재 replicated boss pose/yaw 및 local replicated Character pose를 typed
  Debug context로 공급하며, Tool과 제품 Controller는 같은 `Apply_CueTracking`을 호출한다. 현재 level의
  actor frame이 없거나 PLAYER_BOSS_FRAME의 player sample이 없으면 base pose로 오인시키지 않고 거부한다.

### 1.2 Camera Tool

- F1 Developer Tools에 Camera Tool을 lazy-create하고 Boss Tool camera lane의 stable cue ID를
  `CAMERA_TOOL_OPEN_REQUEST`로 전달한다.
- cue duration/easing/tracking/origin/shake와 key time/eye/lookAt/FOV를 draft에서 편집한다.
- interior key insert/delete와 현재 VIEW/PROJ pipeline pose/FOV Capture를 제공한다.
- Play/Pause, Stop/Restore, scrub은 strict staged draft와 제품 sampler만 사용한다.
- 저장 성공은 live encounter/controller를 hot swap하지 않는다. 다음 level reload가 새 정본을 소비한다.

### 1.3 저장 transaction

Save는 다음 순서다.

```text
whole-v5 camera-only draft serialize -> encounter strict reparse
-> Reload에서 잡은 exact source bytes와 1차 비교
-> sibling temporary write/flush + FlushFileBuffers
-> temporary bytes 동일성 + encounter strict reparse
-> destination exact bytes 2차 비교
-> MoveFileExW(REPLACE_EXISTING | WRITE_THROUGH)
-> loaded baseline/draft commit
```

validation, CAS, temporary parse 또는 replace 실패 시 destination, draft, baseline과 기존 preview를 유지한다.

### 1.4 camera owner와 복구

- Engine presentation priority는 Authoring 10, Valtan Reference Audition 20, Server Cinematic 100이다.
- Tool은 Reference/Server owner를 선점하지 않는다. Server만 낮은 owner를 선점한다.
- 선점 chain의 최초 world/FOV를 한 번만 저장해 최종 Server End가 Tool 시작 전 pose를 복구한다.
- Tool은 매 update owner를 확인한다. Server가 선점하면 Server owner를 End하지 않고 Tool 상태만 비운다.
- 창 닫기, F1 숨김, 다른 Tool 전환, level change와 destructor에서 Tool이 아직 owner일 때만 즉시 End한다.
- 예상 밖 sampler/pose 적용 실패도 Tool owner를 반환하고 이전 camera pose를 복구한다.

## 2. 자동 검증

| 검증 | 결과 |
|---|---|
| `test_valtan_camera_tool_contract.py` | PASS |
| `test_build_maptool_scene.py` | 16/16 PASS |
| `test_sync_valtan_tower_phase_registration.py` | 7/7 PASS |
| GameplayBalance Validate `-SkipValtanSplitProjection` | PASS |
| GameplayBalance canonical Validate | PASS |
| ActionPresentationTimelineHarness Debug build | PASS |
| ActionPresentationTimelineHarness Release build | PASS |
| format 5 roundtrip 및 legacy v4 / retired `skyCues` rejection | Debug/Release 실행 PASS |
| Tool actor-tracking sampler와 제품 BOSS_XZ/PLAYER_BOSS_FRAME 동치 case | Debug/Release 실행 PASS |
| CameraTool/Document/Controller/Level `/Zs` Debug | PASS |
| CameraTool/Document/Controller/Level `/Zs` Release | PASS |
| Engine x64 Debug | PASS |
| Engine x64 Release | PASS, 기존 C4819/LNK4099 warning만 존재 |
| `UpdateLib.bat Debug` | PASS |
| `UpdateLib.bat Release` | PASS |
| Client project/filter XML parse | PASS |
| 비평 재감사 | P2 actor-tracking base-only preview를 typed replicated context + shared product sampler로 해소 |
| 최종 Client Debug/Release | 통합 세션 full build 진행 중, 이 범위의 D/R `/Zs` 및 focused link PASS |

동시 편집 중 잠시 관찰된 encounter stage drift와 controller 미완성 link 상태는 담당 세션의 갱신 뒤
해소됐다. 범위 밖 공유 데이터를 우회 수정하지 않았고, 최신 source에서 GameplayBalance canonical
Validate, CameraTool/Document/Controller/Level Debug/Release `/Zs`, ActionPresentationTimelineHarness
Debug/Release build와 실행이 모두 통과했다.

## 3. 사용자 육안 검증 순서

최신 Server + Debug Client의 Server-approved Valtan Arena에서 사용자가 수행한다.

1. `F1 -> Boss Tool -> Camera` lane에서 cue의 `Open Camera Tool`을 누른다.
2. 같은 stable cue가 Camera Tool에서 선택되는지 확인한다.
3. Free camera pose를 잡고 key의 `Capture Current Camera`를 누른 뒤 Eye/LookAt/FOV가 바뀌는지 확인한다.
4. Scrub, Play/Pause, Stop/Restore와 창 닫기 때 원래 camera pose로 돌아오는지 확인한다.
5. 살아 있는 replicated Valtan/로컬 player가 있는 상태에서 BOSS_XZ, BOSS_FACING, PLAYER_BOSS_FRAME preview가 현재 actor 위치·방향을 따라가는지 확인한다.
6. Reference Camera가 활성인 동안 Tool preview가 거부되고 reference input/proxy 상태가 유지되는지 확인한다.
7. Tool preview 중 Server cinematic을 재생해 Server shot이 우선하고 종료 뒤 Tool 시작 전 pose가 복구되는지 확인한다.
8. 외부에서 JSON bytes를 바꾼 뒤 Save가 충돌을 보고 draft를 보존하는지 확인한다.
9. 정상 Save/Reload 뒤 camera cue/key와 death cue가 유지되고 Map Effect 재생과 독립적인지 확인한다.

사용자 관찰 전에는 framing, shake, tracking fidelity와 camera restore를 visual PASS로 기록하지 않는다.

## 4. 남은 경계

- 최종 공유 tree Client Debug/Release build는 world-set closure 통합 뒤 재실행해야 한다.
- Camera Tool은 기존 cue/key 조정 범위다. Boss presentation invocation의 신규 연결/삭제는 여러 정본을 함께
  commit해야 하므로 이번 단일 camera-document 저장 transaction에 넣지 않았다.
- PR/commit/push와 Client 실행은 수행하지 않았다.
