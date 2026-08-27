# 발탄 등장 컷신 카메라 구현 결과

## 1. 완료 상태

`발탄 등장신 .mp4`의 실제 컷신 구간을 19.867초로 실측하고, 현재 Camera Tool 정본과 Valtan split authoring 데이터에 등장 카메라를 구현했다.

- 카메라 위치, LookAt, FOV, 보간, 구간 전환만 저작했다.
- 신규 자막, Effect, BGM, 컷신 전용 캐릭터 애니메이션은 추가하지 않았다.
- 발탄 조우를 활성화하면 Server scripted sequence의 첫 pattern으로 한 번 재생된다.
- 컷신 중 발탄은 무적이며 세 stage 모두 공격 판정과 Server 이동이 없다.
- 마지막 1초 동안 제품 follow camera로 복귀한 뒤 기존 레이드 자동 순서로 이어진다.
- 일반 랜덤 pattern pool에는 들어가지 않으므로 전투 도중 다시 선택되지 않는다.
- 세 cue는 `CATMULL_ROM + LINEAR`로 재생해 중간 키프레임에서 감속·정지하지 않는다.
- Server snapshot은 cue와 시작 시점을 동기화하되, 같은 cue 안에서는 render delta로 계속 전진하고 최대 25% 속도 보정만 적용해 새 snapshot마다 위치가 점프하지 않는다.

## 2. 구현된 타임라인

| 순서 | Server stage | Camera cue | 시간 | 카메라 의도 |
|---|---|---|---:|---|
| 1 | `ESTABLISH` | `camera.valtan.entrance.establish` | 8,600ms | 낮은 원경에서 발탄을 향해 접근하고 회전 |
| 2 | `ARENA_REVEAL` | `camera.valtan.entrance.arena-reveal` | 5,800ms | 하드 컷 뒤 상공에서 아레나를 공개하며 원호 이동 |
| 3 | `HERO_HANDOFF` | `camera.valtan.entrance.hero-handoff` | 5,467ms | 낮은 영웅 구도로 마무리하고 마지막 1,000ms에 follow camera 복귀 |

총 Server gate 시간은 `8,600 + 5,800 + 5,467 = 19,867ms`다. Camera Tool keyframe 재생은 마지막 stage의 4,467ms까지이며 `transitionOutMs: 1000`을 합쳐 Server stage와 정확히 맞춘다.

## 3. 런타임 연결

```text
boss arena activateEncounter trigger
  -> Server: VALTAN_ENTRANCE_CINEMATIC 시작
  -> snapshot: pattern / stage / actionStartTick 복제
  -> 모든 Client: 같은 authoritative age로 camera cue 샘플
  -> Level: cinematic owner가 활성화된 동안 gameplay input 차단
  -> HERO_HANDOFF와 follow-camera 복귀 완료
  -> 1초 scripted-sequence pursuit handoff
  -> 기존 VALTAN_WHIRLWIND부터 레이드 진행
```

초기 Valtan Level 진입 직후가 아니라 Stage 1과 Lugaru 이후 실제 보스 아레나의 `activateEncounter` 지점에서 시작한다. 별도 신규 network message는 만들지 않았으며 기존 Server-authoritative boss snapshot 계약을 재사용한다.

## 4. 데이터와 코드

- `Data/Valtan/Valtan.gameplay.json`: entry-only Server pattern과 3개 stage
- `Data/Valtan/Valtan.presentation.json`: stage별 clip과 camera invocation
- `Data/Encounters/Valtan/ValtanCinematicCamera.json`: Camera Tool cue 3개와 keyframe
- `Data/Encounters/Valtan/ValtanEncounter.json`: publisher 생성 gameplay product
- `Data/Encounters/Valtan/ValtanPatternRotations.json`: scripted sequence product
- `Data/Animation/Authored/Valtan/Valtan.patternbindings.json`: presentation product
- `Tools/ValtanPipeline/valtan_tuning_pipeline.py`: 첫 scripted pattern에만 entry-only ownership 허용
- 관련 Python contract와 `ServerGameplayContractTests.cpp`: 정확한 순서, 시간, 무적, 무피격 계약 검증

## 5. 자동 검증

### 통과

- Valtan split pipeline validate/project: PASS (`managedPatterns=30`, `worldMembers=97`)
- Gameplay balance Validate/Publish: PASS (`54 patterns`, `230 stages`)
- Camera Tool exact cue/pattern contract: PASS
- ActionPresentationTimelineHarness의 연속 render clock 및 0.5초 지연 snapshot 비후퇴 계약: PASS
- split pattern master와 animation-tool 관련 focused test 14개: PASS
- JSON parse: PASS
- Debug 핵심 Engine, Shared, Server, Client build/link: PASS
- Release 핵심 Engine, Shared, Server, Client build/link: PASS
- Debug/Release Server: `Load the exact invulnerable 19.867-second Valtan entrance camera gate` PASS
- Debug/Release Server: 첫 scripted sequence가 entrance gate인 계약 PASS

### 저장소 기존 실패로 분리

전체 회귀는 이번 변경과 무관한 현재 dirty worktree의 기존 기대값 불일치 때문에 녹색이 아니다.

- Valtan V2 migration test: migration fixture는 world member 30개를 기대하지만 현재 제품은 97개다. focused live-data validation은 PASS다.
- Release Server contract의 기존 실패 2개: 광범위 Valtan topology count/deep condition, Bern NPC crowd/patrol fixture.
- Debug/Release NetworkProtocolHarness의 기존 실패 1개: `Valtan Pattern Flow Contract Uses Protocol 39` 기대값 불일치. 이번 기능은 Shared/protocol을 변경하지 않는다.
- Artist 31470 oracle: 이 PC에 `Windows Kits/10.0.22621.0/x64/d3dcompiler_47.dll`이 없어 실패한다.

이 기존 실패를 컷신 데이터에 맞추기 위해 우회하거나 관련 없는 팀 데이터를 되돌리지는 않았다.

## 6. 사용자 화면 확인

에이전트는 Client 화면을 대신 실행하거나 영상 유사도를 판정하지 않았다. 최종 확인은 다음 순서다.

1. 실행 중인 Server와 Client를 모두 종료한다.
2. Visual Studio에서 `Server + Client` 프로필로 실행한다.
3. `Lobby -> Valtan`으로 진입한다.
4. Stage 1 몬스터와 Lugaru를 진행한다.
5. 보스 아레나 진입 trigger를 밟는다.
6. 약 19.867초 동안 세 구간의 카메라가 전원에게 같은 타이밍으로 재생되는지 확인한다.
7. 카메라가 자연스럽게 follow view로 돌아오고 기존 발탄 첫 패턴으로 이어지는지 확인한다.

카메라의 위치나 속도 미세 조정은 이 수직 슬라이스의 수동 visual fidelity 확인 뒤 Camera Tool keyframe만 수정하면 된다.
