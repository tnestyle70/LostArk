# 플레이어 애니메이션 프레임레이트와 클립 타이밍 정정

작성자: JS · 2026-08-06 · 브랜치 `feature/player-anim-framerate-fix`

어제(`.md/JS/08-05/`) 연환섬이 3타까지 안 나가는 문제를 CANCEL 기반 "조기 전환"으로
설명하려 했는데, 실측해보니 원인이 다른 곳에 있었다. 이 문서는 그 정정과 실제로 고친
내용을 남긴다.

## 1. 어제 문서의 정정

`2026-08-05_LOSTARK_LANCEMASTER_CLIP_ADVANCE_TIMING_PLAN.md`에서 결정 대기로 남긴
두 항목은 다음과 같이 해소됐다.

- **2.2 데이터 출처** — `EFTable_SkillEffect.db`는 볼 필요가 없었다. 전환 시각 자체가
  원본 어디에도 없다(`2026-07-31_..._SKILLTIMING_V2_EXTRACT_RESULT.md` §10.2). DB 시각은
  시전 전체 기준이라 클립 로컬인 Action 시계와 다르고, 159개 중 35개만 120ms 안에 든다.
- **2.1 저장 방식** — 스키마 확장이 필요한 건 맞았지만 이유가 달랐다. "CANCEL 기반 조기
  전환"이 아니라 **원본이 스테이지마다 저작해둔 재생 길이**다.

어제 PLAN의 CANCEL 판정 규칙("시간순 마지막 notify")도 부정확했다. 실제 규칙은
**레이블 없고 `win=NONE`이며 `t+d`가 클립 끝에 닿는 CANCEL 윈도우 중 가장 이른 것**이다.

## 2. 근본 원인 — 플레이어 애니메이션이 25% 느리게 재생되고 있었다

`.wmodel`의 `duration`은 프레임 수이고 `ticksPerSecond`에 **24**가 박혀 있다.
원본 PSA의 `ANIMINFO.AnimRate`는 **30**이다(221/223 시퀀스, 나머지 28.x는 export 반올림).

```text
창술사 130개 스테이지 중 128개가  .wmodel 프레임수 / 30 == Action len=  (소수점까지 일치)
```

24는 Blender 씬 기본 fps가 FBX로 샌 것이다. 다섯 플레이어 클래스 전부 24, 발탄/NPC는 30
(다른 경로로 쿠킹돼 클립별 rate가 보존돼 있다).

**두 번째 버그**: `CPart_Body::Update()`가 `Play_Animation()`을 직접 불러서
`CModel::Update_Animation()`이 곱하는 `m_fAnimationSpeed`를 건너뛰고 있었다. 즉
`Set_AnimationSpeed()`는 캐릭터 몸에 대해 한 번도 동작한 적이 없다.

## 3. `.animnotify`의 `len=`은 클립 길이가 아니다

`CEFActionNotify_Anim`의 duration, 즉 **그 스테이지가 그 클립을 재생하는 시간**이다.
같은 클립이 스테이지마다 다른 값을 갖는다.

```text
SK_PenetrationLunge_02   base 체인 stage01   dur=0.2500
                         트라이포드 stage05   dur=1.5000
```

굉열파가 "기 모으는 동작만 하고 찌르기가 안 나오던" 원인이다. `_02`는 1500ms 클립 중
250ms만 재생해야 하는데 런타임이 `Is_ClipFinished()`로 전체를 기다려, 0.7+1.5=2.2초가
지나 `_03`(HIT 전부 여기 있음)이 시작하자마자 서버가 2.25초에 액션을 끊었다.

## 4. 구현

| 파일 | 내용 |
|---|---|
| `Engine/Private/Animation.cpp` | `.wmodel` 경로의 `m_fTickPerSecond`를 `COOKED_TICK_RATE = 30.f`로 고정 |
| `Client/Private/Part_Body.cpp` | `Play_Animation` → `Update_Animation` (재생 배속 반영) |
| `Client/Public/AnimationSkillBindingDocument.h` | `ANIMATION_SKILL_CLIP { strClipName, iPlayMs, fPlayRate }` |
| `Client/Private/AnimationSkillBindingDocument.cpp` | `formatVersion` 2, clip 원소가 문자열 또는 `{clip, playMs?, playRate?}` |
| `Client/Public/Character.h`, `Character.cpp` | `CLIP_STEP`, `Is_ClipFinished()`가 `playMs` 지점 판정, `Start_Clip`이 배속 적용 |
| `Client/Private/Animation_Tool.cpp`, `Effect_Tool.cpp` | 원소 타입 변경에 따른 읽기 수정 |
| `Tools/ClientFrontendHarness/...` | v2 픽스처, `playMs: 0` 거부 케이스 추가 |
| `Tools/ProjectAudit/Invoke-ProjectAudit.ps1` | 스키마 v2 + clip 객체(`playMs`/`playRate`) 검증 |
| `Data/Animation/Authored/*/*.skillbindings.json` | 5개 클래스 `formatVersion` 2 |

`Clips`를 문자열 벡터에서 구조체 벡터로 승격한 이유는 `Animation_Tool`이
insert/swap/erase로 직접 편집하기 때문이다. 병렬 배열이면 다섯 군데를 미러링해야 하고
어긋나면 조용히 잘못된 길이가 붙는다.

### 저장소 밖 (buildScript, Git 미추적)

`build_flm_part.py` / `build_character_part.py` / `build_npc.py`에
`ctx.scene.render.fps = 30`을 넣었다. **`.wmodel` 파일 자체는 여전히 24이고 엔진이 그
필드를 안 읽는 방식이라 재쿠킹과 리소스 재배포는 필요 없다.** 재쿠킹은 오프라인 python
스크립트가 찍는 길이를 맞추기 위한 선택 사항이다.

## 5. 창술사 타이밍 데이터

`playMs`(전환 지점)와 `playRate`(재생 배속)를 16개 스킬에 적용했다. 근거 강도가 다르므로
구분해서 기록한다.

- **`playRate`** — `.loa` 스테이지 레코드의 clip duration 바로 뒤 float. 평타 1.3~1.5,
  트라이포드 변형 1.0~1.15로 갈리는 분포가 배속으로 읽힌다. **추정이다.**
- **굉열파·풍진격의 `playMs`** — `.animnotify`의 `len=`. 원본이 직접 저작한 값이다.
- **연환섬의 `playMs` 900/582** — 레이블 없는 `win=NONE` CANCEL 윈도우 **시작값**.
  원본이 "여기서 전환한다"고 적은 게 아니라 "여기부터 입력을 받는다"고 적은 것이라
  튜닝값이다. 사용자가 Animation Tool에서 27프레임/17프레임으로 독립 확인했다.

`actionDurationMs` 14건과 `hitTimeMs` 1건(34540)을 재계산하고
`2026-08-05.balance-provenance.receipt.json`의 대응 field를 동기화했다.
34540은 `hitTimeMs=1445`가 새 duration 1192를 넘어 `GameplayCatalog.cpp:210`의
`iHitTimeMs > iActionDurationMs` 하드 리젝트에 걸리므로 첫 HIT 노티파이 기준 304로 내렸다.

### 적용하지 않은 것

- **평타 34010 / 34510** — COMBO는 스테이지별 `actionDurationMs`·`inputOpenMs`가 서버
  소유다. 배속만 걸면 애니메이션이 서버 단계보다 먼저 끝나 포즈가 굳는다.
- **34590 적룡포** — 홀딩 스킬. 홀딩 입력 처리가 별도 작업으로 남아 있어 제외.
- **34170 / 34020 / 34570 / 34580** — 바인딩된 클립을 `.loa` 스테이지에서 해석하지 못해
  미적용.
- **나머지 네 클래스** — 같은 방식이 적용 가능하나 이번 범위에 넣지 않았다.

## 6. 검증

- `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug`
  - Engine / Shared / NetworkProtocolHarness / ClientFrontendHarness / Server / Client 빌드 성공
  - protocol harness `failures : 0`, Effect Tool bundle PASS
  - `Gameplay balance Validate/Publish succeeded` — receipt 동기화와 hitTime 제약 통과
  - Server navigation Validate 3 area 성공
- `ProjectAudit` — `gameplay.playable-skill-animation-authoring-contract` 통과.
  `projects.data-source-visibility` **실패(기존)**: `cdb8680`(SkillWindow, PR #56)이 추가한
  `Data/UI/SkillWindow/*.json` 2개가 `Client.vcxproj`에 등록되지 않았다. `main`에서도
  동일하게 실패하며 이번 변경과 무관하다. 등록 여부 미정.
- 인게임 확인(사용자): 굉열파 찌르기 복구, 연환섬 3타 재생. 은하유성탄·은하비섬창의
  마무리 동작 누락은 §2의 `Part_Body` 버그였고 수정 후 재빌드했다. **배속이 실제로
  적용된 상태의 인게임 확인은 아직 진행 중이다.**

## 7. 다음

- 루트 모션 — `movementDistance` 등속 이동을 구워진 `b_root` 변위로 완전 대체.
  서버가 클립을 모르므로 스킬별 합성 곡선을 발행해야 한다. 상세는 별도 문서.
- SkillWindow Data 파일 2개 등록 여부
- 34590 홀딩, 31210 도화가 Q 바인딩(트라이포드 클립이 묶인 것으로 보임)
- 나머지 네 클래스 타이밍
