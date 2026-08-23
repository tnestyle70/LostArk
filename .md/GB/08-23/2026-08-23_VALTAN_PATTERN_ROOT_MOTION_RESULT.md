# 2026-08-23 발탄 패턴 루트 모션 — 등속 슬라이드를 구워진 이동으로 대체

담당: 맵/발탄. 최종 화면 판정자: 사용자.

## 1. 확인 결과 — 발탄은 루트 모션으로 움직이지 않았다

플레이어는 2026-08-06에 이미 전환했다(`.md/JS/08-06/2026-08-06_LOSTARK_PLAYER_ROOT_MOTION_RESULT.md`).
서버가 `Data/Animation/RootMotion/<Class>.rootmotion.json`의 곡선을 액션 시계로 샘플링해
차분만큼 밀고, 클라이언트는 `Enable_RootMotionSuppression("b_root", 2)`로 애니메이션 자체
이동을 억제한다.

발탄에는 그 계약이 하나도 없었다.

| | 플레이어 | 발탄(수정 전) |
|---|---|---|
| 서버 이동 | 저작 곡선 차분 | `fDistance / durationSeconds` 등속 |
| 클라 루트 억제 | `CCharacter`에서 호출 | 없음 |
| 곡선 문서 | 4개 class | 없음 |

`MN_RPBF_01_AnimSet.wmodel`을 직접 파싱해 `b_root` 채널을 실측했다. 146개 클립이 채널을
가지고 그중 **102개가 실제로 이동**한다. 즉 클립에 이동이 구워져 있는데 아무도 소비하지
않았고, 서버가 미는 트랜스폼과 메시가 따로 놀았다. 플레이어가 겪었던 이중 이동과 같은 결함이다.

축 매핑과 배율은 플레이어와 동일하다. 발탄 메시의 최대 span이 296 unit이고 ×0.01에서
2.96 m라 보스 키와 맞는다.

```text
forward =  0.01 * raw.x
lateral = -0.01 * raw.y
up      = -0.01 * raw.z   (서버로 보내지 않는다)
```

스테이지별 실측은 다음과 같다. 130개 스테이지 중 저작 이동을 선언한 것은 2개뿐이고,
**35개가 "애니메이션은 움직이는데 서버는 안 미는"** 상태였다.

| 패턴/스테이지 | 서버가 밀던 거리 | 클립에 구워진 거리 |
|---|---:|---:|
| `VALTAN_ARMOR_BREAK_OPENING` / `WALL_CHARGE` | 100.0 m | −1.00 m |
| `VALTAN_DASH_CHARGE` / `CHARGE` | 20.0 m | 0.19 m |
| `VALTAN_PORTAL_RUSH` / `RUSHES` | 0 | 7.27 m |
| `VALTAN_DASH_CHARGE` / `RECOVERY` | 0 | 4.90 m |
| `VALTAN_JUMP_SPIN` / `JUMP` | 0 | 3.41 m |

## 2. 구현한 계약

### 2.1 추출기

`Tools/ValtanActionExtractor/build_valtan_rootmotion.py`를 추가했다. 플레이어 쪽 추출기는
저장소 밖에 있어 재현이 불가능했는데, 이번 것은 저장소 안에 두고 `--check`로 결정성을
검사한다.

이미 저작된 체인만 사용한다.

```text
ValtanEncounter.json         pattern -> stage -> actionId, durationMs
Valtan.patternbindings.json  actionId -> clip, sourceStartMs, playRate
MN_RPBF_01_AnimSet.wmodel    clip -> b_root position keys
```

산출물은 `Data/Animation/RootMotion/Valtan.rootmotion.json`
(`lostark.valtan-pattern-root-motion` v1, 27 pattern / 48 stage / 2,291 sample)이다.

### 2.2 저작 거리를 선언한 스테이지는 건드리지 않는다

`WALL_CHARGE`와 `CHARGE`는 곡선 문서에서 의도적으로 제외했고 추출기가 그 사실을 출력한다.
두 스테이지가 묶인 클립은 각각 −1.00 m와 0.19 m만 움직이는데 저작 거리는 100 m와 20 m다.
곡선으로 바꾸면 돌진이 벽에 닿지 못해 방어구 파괴 기믹이 성립하지 않는다. 클립 바인딩 자체가
어긋나 있다고 보는 것이 맞고, 그 판단은 사용자 몫으로 남긴다.

### 2.3 퍼블리셔

`Publish-GameplayBalance.ps1`이 `PATTERNSTAGEROOTMOTION` 행 48개를 발행한다. 행 이름은
`PATTERNSTAGE`보다 뒤에 정렬되므로 스테이지가 먼저 파싱된다. 플레이어 때 `ROOTMOTION`이
`SKILL`보다 앞서 정렬돼 209건이 연쇄 실패했던 함정을 피했다.

기존 class 루프는 `Data/Animation/RootMotion/*.rootmotion.json`을 glob하므로, 보스 schema를
만나면 명시적으로 건너뛰게 했다. 다른 알 수 없는 문서는 계속 fail-closed다.

### 2.4 서버

- `PLAYER_ROOT_MOTION_SAMPLE`을 `ROOT_MOTION_SAMPLE`로 개명했다. 플레이어와 보스가 함께
  쓰는 타입이라 이름에 소유자를 남기지 않는다. 10곳 기계 치환이다.
- `BOSS_PATTERN_STAGE_MOTION`에 `RootMotion` 곡선을 추가했다.
- `GameplayCatalog`이 `PATTERNSTAGEROOTMOTION`을 파싱한다. 알 수 없는 encounter/pattern,
  범위를 넘는 stage index, 한 스테이지에 두 번째 곡선은 전부 fail-closed다.
- `SERVER_WORLD_ENTITY`가 스테이지 진입 시 곡선을 복사한다. 카탈로그 메모리를 가리키지 않아
  수명 결합이 없다.
- `CValtanBrain::Try_BuildStageMotion`이 곡선이 있으면 이전 틱과 현재 틱을 샘플링해 차분만큼
  전진하고, 없으면 기존 등속 경로를 그대로 쓴다.
- `GAMEPLAY_BOOTSTRAP_VERSION` 16 → 17로 올려 퍼블리셔와 서버가 함께 움직이게 했다.

### 2.5 임팩트 스윕을 돌진으로 한정

`GameRoom`은 스테이지 이동이 성립하면 **무조건** 임팩트 리시버 스윕을 하고, 맞으면 벽을 부순
뒤 임팩트 스테이지로 넘겼다. 이동하는 스테이지가 2개에서 48개로 늘자 0.2 m짜리 회복 발놀림이
벽을 부수고, 돌진이 아닌 스테이지에서 `Complete_ImpactStage`가 false를 반환해 room이
하드 실패했다. Ordered 1-67 계약 8건이 그렇게 깨졌다.

스윕을 `entity.bPatternChargeImpact`로 게이트했다. 돌진 동작은 그대로이고 일반 스테이지는
자기 걸음만 걷는다.

### 2.6 클라이언트

`CValtan`이 body 모델에 `Enable_RootMotionSuppression(ROOT_MOTION_BONE, ROOT_MOTION_VERTICAL_AXIS)`를
호출한다. 무기는 body 스켈레톤을 따르므로 body 하나면 충분하고, local Z는 포즈가 도약을
그리므로 열어 둔다. `CCharacter`와 같은 계약이다.

## 3. 자동 검증

| 검증 | 결과 |
|---|---|
| `build_valtan_rootmotion.py` 실행 | 27 pattern / 48 stage / 2,291 sample |
| `build_valtan_rootmotion.py --check` | 결정적, `check OK` |
| 문서 불변식(첫 샘플 0, 마지막 = durationMs, 시간 단조 증가) | 48/48 통과 |
| `Publish-GameplayBalance.ps1 -Mode Validate` | PASS, 33 boss patterns / 130 pattern stages |
| `Publish-GameplayBalance.ps1 -Mode Publish` 2회 sha256 | 동일, 결정적 PASS |
| bootstrap `PATTERNSTAGEROOTMOTION` 행 | 48행, `PATTERNSTAGE` 뒤 정렬 확인 |
| Server x64 Debug 빌드 | PASS |
| Client x64 Debug 빌드 | PASS |
| `Server.exe --contract-test` | 707 PASS / failures 0 |
| 신규 계약 테스트 | PASS |
| `NetworkProtocolHarness` | failures 0 |
| `git diff --check` | 오류 없음 |

신규 계약 테스트는 `Walk a Valtan stage along the travel its clip baked and leave the
authored charge on its own distance`이며 세 가지를 함께 검사한다. `VALTAN_PORTAL_RUSH`
`RUSHES`를 60틱 걸어 누적 이동이 구워진 7.27 m와 0.01 m 안에서 일치하는지, `VALTAN_DASH_CHARGE`
`CHARGE`가 곡선 없이 `FORWARD` 저작 거리를 유지하는지, 곡선도 저작 거리도 없는 스테이지가
아무 걸음도 제안하지 않는지.

## 4. 남은 경계

- `WALL_CHARGE` 100 m와 `CHARGE` 20 m는 그대로다. 클립 바인딩이 각각
  `mesh_dmg_parts_start_1`, `mesh_att_battle_4_01`인데 둘 다 돌진 클립이 아니다. 바인딩을
  실제 돌진 클립으로 바꿀지, 저작 거리를 유지할지는 사용자 결정이다.
- 수직(`up`)은 서버로 보내지 않는다. 포즈가 도약을 그리므로 이중이 된다.
- `lateral`은 곡선에 실려 있으나 서버 적용은 전진 축만 쓴다. 발탄 클립의 `raw.y`는 전 구간
  0이라 현재 차이가 없다.
- 82개 스테이지는 구워진 이동이 0.05 m 미만이라 곡선을 만들지 않았다.
- 화면 판정은 사용자 전용이다.

## 5. 사용자 수동 확인

Server와 Client를 모두 재빌드했으므로 실행만 하면 된다.

1. 발탄에 진입해 일반 패턴에서 발이 미끄러지지 않는지 본다. 특히 `VALTAN_PORTAL_RUSH`,
   `VALTAN_JUMP_SPIN`, `VALTAN_DASH_CHARGE` 회복 동작.
2. 애니메이션이 앞으로 나가는 만큼 실제로 보스가 이동하는지, 끝나고 제자리로 튀지 않는지 본다.
3. 돌진(`VALTAN_DASH_CHARGE`, `VALTAN_ARMOR_BREAK_OPENING`)이 예전처럼 벽까지 도달해
   방어구 파괴로 이어지는지 본다. 이 두 개는 의도적으로 바꾸지 않았다.
4. 무기와 파츠가 body를 따라오는지, 몸만 남고 무기가 뒤처지지 않는지 본다.
