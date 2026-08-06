# 플레이어 루트 모션 — 구워진 이동량으로 `movementDistance` 대체

작성자: JS · 2026-08-06 · 브랜치 `feature/player-anim-framerate-fix`

`movementDistance` 등속 직선 이동을 애니메이션에 구워진 `b_root` 변위로 대체했다.
선행 작업은 `2026-08-06_LOSTARK_PLAYER_ANIM_FRAMERATE_AND_CLIP_TIMING_RESULT.md`다.

## 1. 대체가 필요했던 이유

`b_root`를 처리하는 코드가 없어서 구워진 이동이 평범한 본 트랜스폼으로 적용되고 있었다.
서버는 `movementDistance`로 트랜스폼을 밀고 메시는 따로 미끄러져 **이중 이동**이었고,
두 값이 맞지도 않았다(탄영: 구워진 3.5유닛 vs `movementDistance` 6.0).

`movementDistance=0`인 스킬에도 구워진 이동이 있었다. 일섬각 3.5, 회선창 1.99 —
메시만 앞으로 가고 서버 위치는 제자리였다. 창술사 223개 클립 중 181개가 루트 이동을 갖는다.

## 2. 축 매핑 — 두 번 틀렸다

아마추어 노드 `flm`이 local (x, y, z)를 (100x, -100z, 100y)로 보내고 PreTransform이
`Scaling(0.0001) * RotationY(-90)`를 건다. 합치면 이렇다.

```text
forward (world +Z) =  0.01 * raw.x
up      (world +Y) = -0.01 * raw.z
lateral (world +X) = -0.01 * raw.y      전 클립에서 0
```

**raw.z는 측면이 아니라 수직이다.** 처음엔 이걸 측면으로 읽어 `lateral` 채널에 수직값을
넣었고, 이어서 "수직 이동이 애니메이션에 없다"고 잘못 결론냈다. 사용자가 일섬각이
"창을 지렛대 삼아 몸을 띄우는 동작"이라고 알려준 뒤 아마추어 노드 행렬을 직접 읽어 바로잡았다.
`flm_sk_assault`의 raw.z 105.4는 1.05유닛 상승이고 그 도약이 맞다.

검증: 탄영 총 변위가 예측한 3.5유닛과 정확히 일치, `lateral`이 전 스킬에서 0,
`endUp`이 34620을 뺀 전부에서 0(모두 지면 착지).

## 3. 구현

| 파일 | 내용 |
|---|---|
| `buildScript/extract_rootmotion.py` | `b_root` 키 + skillbindings를 스킬별 곡선으로 합성 (저장소 밖) |
| `Data/Animation/RootMotion/LanceMaster.rootmotion.json` | 19개 스킬, `forward`/`lateral`/`up` 누적 곡선 |
| `Engine/Public/Model.h`, `Model.cpp` | `Enable_RootMotionSuppression(bone, verticalAxis)` |
| `Client/Private/Character.cpp` | body 모델에만 `b_root` 수평 억제, local Z(수직)는 유지 |
| `Tools/GameplayPipeline/Publish-GameplayBalance.ps1` | `SKILLROOTMOTION` 행 발행과 검증 |
| `Server/Public/GameplayCatalog.h`, `Server/Private/GameplayCatalog.cpp` | 곡선 파싱 |
| `Server/Private/PlayerSkillSystem.cpp` | 곡선 차분을 조준 방향으로 회전해 적용 |

서버는 클립을 모른다. publisher가 `skillbindings`의 `playMs`/`playRate`로 체인을 하나의
곡선으로 합성해 `skillId -> 곡선`만 내려보내므로 서버 권위 경계가 유지된다.

수직은 bootstrap에 싣지 않는다. 포즈가 이미 도약을 그리므로 서버 Y까지 움직이면 이중이 된다.

곡선이 있으면 곡선이 이기고 없으면 기존 `movementDistance`가 남는다. 죽은 필드를 만들지
않으면서 대체된다.

## 4. 작업 중 걸린 것 넷

전부 publisher/하네스가 잡았고 조용히 넘어간 것은 없다.

1. `Format-InvariantFloat`가 음수를 거부 — 루트 모션은 음수가 나온다. 부호 있는 값용을 분리.
2. **34050 스테이지 트림 소실** — 전 스킬 일괄 적용 스크립트가 `playMs`를 CANCEL 윈도우에서만
   뽑아 `hurricaneswing_02`의 800ms 트림을 날렸다. CANCEL 규칙은 체인의 마지막 클립을
   제외하는데 그게 마지막이었다. **`playMs`는 스테이지 재생 길이와 CANCEL 전환점 중
   작은 값이며 스테이지 길이는 마지막 클립에도 적용된다.**
3. **행 정렬 순서** — bootstrap 행이 정렬되는데 `ROOTMOTION`이 `SKILL`보다 앞서 파싱돼
   로드 실패, 서버 계약 테스트 209건 연쇄 실패. `SKILLROOTMOTION`으로 개명.
4. **`Project_Point`의 셀 중심 스냅** — 이 함수는 셀 중심을 돌려주는데 그 좌표를 위치로
   그대로 썼다. `cellSize=1` grid에서 tick당 0.1유닛 이동이 매번 중심으로 되돌려져 이동량이
   버려졌다. 스킬 이동 경로에서 걷기 판정과 높이만 쓰도록 고쳤다.
   **기존 `movementDistance` 경로도 같은 스냅을 지나고 있었다.**

## 5. 검증

- `Invoke-BuildAndRegression.ps1 -Configuration Debug`: 전 프로젝트 빌드 성공,
  protocol harness `failures : 0`, `Server.exe --contract-test` `failures : 0`,
  balance Validate/Publish 성공.
- `ProjectAudit`: `projects.data-source-visibility` 실패(기존, SkillWindow 2개 미등록).
  새로 만든 `LanceMaster.rootmotion.json`은 `Client.vcxproj`/`.filters`에 등록해 차이를 늘리지 않았다.
- 인게임(사용자): 메시 슬라이드 제거 확인, 캐릭터 회전/기울어짐 없음, 일섬각 도약 정상,
  전진 정상, 이동량 정상.

## 6. 남은 것

- `movementDistance` 값을 0으로 정리하고 receipt 동기화. 현재는 곡선이 이기고 있어 동작상
  이미 대체됐다.
- `34620` 각성기의 `endUp -7.992`. 수직을 서버로 보내지 않아 당장 영향은 없다.
- `Project_Point` 스냅은 보스 추적과 `movePlayer` 트리거도 지난다. 이번엔 스킬 이동만 고쳤다.
- 나머지 네 클래스 루트 모션 추출.
- COMBO(34010/34510)는 스테이지별 `actionDurationMs`를 쓰므로 평평한 곡선으로 주소를 매길 수
  없어 제외했다. 필요해지면 스테이지별 곡선 계약이 따로 있어야 한다.
