# 발탄 등장 컷신 카메라 구현 계획

## 1. 목표

`발탄 등장신 .mp4`의 실제 재생 구간을 기준으로 발탄 조우 시작 시 모든 Client가 같은 Server 패턴 틱으로 카메라 컷신을 재생하고, 카메라 복귀가 끝난 뒤에만 기존 레이드 자동 패턴 순서를 시작한다.

이번 범위는 카메라 위치·LookAt·FOV·구간 전환뿐이다. 신규 자막, Effect, BGM, 컷신 전용 캐릭터 애니메이션은 만들지 않는다.

## 2. 영상 실측과 타임라인

- 파일 전체: 24.63초 / 30fps
- 플레이어 영상 UI를 제외한 실제 컷신: 1.900초부터 21.767초까지, 총 19.867초
- `ESTABLISH`: 8,600ms, 낮은 원경에서 발탄을 향해 접근하고 회전한다.
- `ARENA_REVEAL`: 5,800ms, 플래시 컷 뒤 상공에서 아레나를 공개하며 원호 이동한다.
- `HERO_HANDOFF`: Server stage 5,467ms. 카메라 keyframe은 4,467ms이며 마지막 1,000ms는 제품 follow camera로 복귀한다.

## 3. 데이터 계약

### 3.1 Server gameplay

`Data/Valtan/Valtan.gameplay.json`에 공격 판정이 없는 `VALTAN_ENTRANCE_CINEMATIC`을 추가한다.

- `invulnerableWhileRunning: true`
- 세 stage 모두 `hit.shape.kind: NONE`
- `serverMotion: null`
- 자동 scripted sequence의 첫 pattern
- 일반 랜덤 selection set에는 등록하지 않는다. pipeline은 scripted sequence의 첫 pattern 하나만 entry-only gate로 허용하며, 그 밖의 unowned pattern은 계속 거부한다.

### 3.2 Client presentation

`Data/Valtan/Valtan.presentation.json`에 같은 pattern/stage/action tuple을 추가한다. 신규 애니메이션은 만들지 않고 현재 검증된 발탄 등장 clip을 stage 끝까지 반복한다. 각 stage의 `cameraInvocations`는 전용 camera cue를 `ENTER`에서 한 번 시작한다.

### 3.3 Camera Tool 정본

`Data/Encounters/Valtan/ValtanCinematicCamera.json`에 다음 cue를 추가한다.

- `camera.valtan.entrance.establish`
- `camera.valtan.entrance.arena-reveal`
- `camera.valtan.entrance.hero-handoff`

세 cue는 `BOSS_FACING`, `CATMULL_ROM`, `LINEAR`를 사용한다. Catmull-Rom이 키프레임 사이의 곡선을 만들고 `LINEAR` 시간 진행이 중간 키프레임마다 속도가 0으로 떨어지는 현상을 막는다. 발탄 아레나 중심 `[156.03, 22.99751, -122.06]`을 tracking origin으로 쓰며 마지막 cue만 `transitionOutMs: 1000`을 사용한다.

## 4. 런타임 흐름

```text
activateEncounter
  -> Server가 VALTAN_ENTRANCE_CINEMATIC 시작
  -> snapshot의 pattern/stage/actionStartTick을 모든 Client가 수신
  -> CValtanCinematicCameraController가 같은 authoritative age를 샘플
  -> CLevel_ValtanArena가 gameplay input을 차단한 채 camera pose 적용
  -> HERO_HANDOFF keyframe 종료
  -> 1초 follow-camera 복귀
  -> Server stage 종료와 함께 기존 VALTAN_WHIRLWIND부터 레이드 순서 시작
```

## 5. 검증

1. JSON parse와 split Valtan pipeline validate/project/publish
2. Camera Tool 계약 test에 exact stage/cue/duration/무피격/첫 순서 검증 추가
3. Server gameplay 계약에서 live pattern/stage 수와 첫 자동 pattern 갱신
4. Debug/Release publisher, Server contract, Client build
5. `git diff --check`

Client 화면의 최종 카메라 미세 조정과 영상 유사도 판정은 사용자가 `Lobby -> Valtan -> boss arena trigger`에서 직접 확인한다.
