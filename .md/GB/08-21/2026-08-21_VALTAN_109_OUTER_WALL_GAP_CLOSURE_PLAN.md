# 2026-08-21 발탄 109줄 외곽 벽 이음새 보강 계획

## 1. 목표

발탄 전투장에 109줄 패턴 전까지 유지되는 외곽 벽은 현재 `DEPLOY_ITR_02306` 30개가
반지름 16.1m에 12도 간격으로 배치되어 있다. 배치 누락은 없지만 모델의 하단 몸체 폭이
약 1.97m이고 이웃 pivot chord는 약 3.366m라, 사용자 화면에서 외곽 벽 사이에 반복되는
큰 시각 공백이 보인다.

이번 작업은 기존 30개 벽의 위치, 109줄 전용 Server binding, collision, navigation을
바꾸지 않고 각 이웃의 정확한 중간 6도 위치에 짧은 동일 계열 연결벽 30개를 추가한다.
연결벽은 기존 30 destruction group에 하나씩 소속되어 109줄 IMPACT에서 원래 벽과 같은
tick에 제거된다.

## 2. 실측 기준

| 항목 | 현재 값 | 변경 후 값 |
|---|---:|---:|
| 109 외곽 destruction group / binding | 30 / 30 | 30 / 30 |
| 원본 `DEPLOY_ITR_02306` | 30 | 30, transform 불변 |
| 연결벽 `DEPLOY_ITR_02307` | 0 | 30 |
| 외곽 group member | 30 | 60 |
| debris source emitter | 30 | 30 |
| suppression alias | 0 | 30 |
| 동시 파편 actor | 360 | 360 |
| collision / nav region | 30 / 30 | 불변 |

`DEPLOY_ITR_02307`은 intact/fractured WModel과 12-piece recipe가 모두 존재하지만, 이번
연결벽은 별도 emitter가 아니라 같은 group의 bound visual alias로 둔다. 109줄에서 원본
30개가 360개 파편으로 붕괴하는 동안 연결벽도 함께 숨긴다. 720개 actor를 한 frame에
생성해 FPS와 PhysX 부하를 두 배로 만드는 경로는 사용하지 않는다.

## 3. 배치 계약

아레나 중심은 `C=(156.03, -122.06)`, 반지름은 `R=16.1m`, 높이는 `Y=23.04m`다.
기존 원본 ID suffix는 각도순이 아니므로 suffix를 각도 index로 해석하지 않는다. 원본 벽
`10900000000000NN`의 실제 transform에서 중심 기준 각도 `thetaSource`를 구하고, 같은 suffix의
연결벽 `10910000000000NN`을 `theta=thetaSource+6`도에 둔다. 다음을 사용한다.

```text
x = 156.03 + 16.1 * cos(theta)
z = -122.06 + 16.1 * sin(theta)
yaw = 90 - theta
q = (0, sin(yaw/2), 0, cos(yaw/2))
runtimePlacementId = 1091000000000000 + NN
sourcePlacementId = Authored:OuterRing109GapFiller:<runtimePlacementId>
assetId = DEPLOY_ITR_02307
uniformScale = 1.52
```

실제 triangle slice를 0.025m 간격으로 검사했을 때 scale 1.0~1.30은 하단 성벽에
0.235~0.490m의 반복 공백을 남겼다. scale 1.52는 약 0~5m 높이의 성벽 띠를 닫는다.
높은 첨탑 사이의 열린 공간은 원본 실루엣에도 존재하므로 인위적으로 막지 않는다.

원본 30개의 각도 집합은 정확히 `0,12,...,348`도이고 결과 filler 각도 집합은 정확히
`6,18,...,354`도여야 한다. 각 filler `10910000000000NN`은 원본
`10900000000000NN`과 같은
`destroyable.group.valtan.outerwall109.10900000000000NN`의 두 번째 member가 된다.
그 group의 기존 emitter는 filler ID를 suppression alias 한 개로 소유한다. filler를 위한
collisionBox/nav region/group/binding은 추가하지 않는다. 현재 collision tangential 폭
4.588m가 3.366m 간격을 이미 덮으므로 gameplay 장벽은 연속이다.

## 4. 변경 범위

- `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.deployplacements`
- `Data/Encounters/Valtan/ValtanWorldEvents.json`
- `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.destructionsimulation.json`
- `Tools/WorldPipeline/Publish-ValtanWorldDestruction.ps1`
- `Server/Private/WorldDestructionBootstrapContractTests.cpp`
- 재현 가능한 filler 동기화 도구와 단위 테스트
- publish된 Map/World runtime 문서

현재 별도 작업으로 수정 중인 `Gameplay.world.json`, 발탄 pattern/brain/catalog 파일은
덮어쓰지 않는다. 새 collision/nav를 만들지 않으므로 해당 dirty hunk와도 겹치지 않는다.

## 5. 실패 차단 계약

- filler는 정확히 30개이고 ID, 각도, 반지름, 높이, quaternion, scale이 canonical 식과
  일치해야 한다.
- 30 group은 각각 원본 한 개와 filler 한 개만 소유한다.
- 각 기존 debris source는 같은 번호 filler alias 하나만 소유한다.
- 외곽 graph는 30 groups / 60 members / 30 emitters / 30 aliases / 360 fragments다.
- 109 외곽 binding은 30개 모두 `VALTAN_ARENA_BREAK_109/IMPACT`이고 일반 contact binding은
  계속 0개다.
- floor 84/30, entrance sweep, 159 impact, ordinary contact wall 수는 바뀌지 않는다.
- parse/validate/stage 중 하나라도 실패하면 authoring/runtime 일부만 교체하지 않는다.

## 6. 검증

1. filler 동기화 단위 테스트와 `--check-only`를 통과한다.
2. `Publish-MapAuthoring.ps1 -AreaId LV_LUT_HEARTRB_ED`를 통과하고 authoring/runtime deploy
   placement가 byte-identical인지 확인한다.
3. `Publish-ValtanWorldDestruction.ps1`의 Validate, ContractTest, Publish를 통과한다.
4. WorldGameplay와 Navigation Validate로 collision/nav 불변 계약을 회귀 확인한다.
5. Server Debug/Release build와 `Server.exe --contract-test`, ClientFrontendHarness
   Debug/Release를 실행한다.
6. Client Debug/Release build와 `git diff --check`를 통과한다.
7. 사용자가 Lobby에서 Valtan으로 진입해 초기 외곽 하단 띠의 30개 반복 공백이 닫혔는지,
   일반 공격에 선파괴되지 않는지, 109줄에서 60개 visual placement가 모두 사라지고 바닥은
   유지되는지 직접 판정한다. 에이전트는 visual PASS를 대신 선언하지 않는다.
