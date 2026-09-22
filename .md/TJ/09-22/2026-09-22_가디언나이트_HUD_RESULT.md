# 가디언나이트 전투 HUD — 원작 아트 이식 결과 (2026-09-22)

브랜치 `feature/guardianknight-hud` (origin/main `15ad4b33`에서 분기).

사용자 요청: 가디언나이트 HUD를 만든다. 작업 중 범위 조정 — **아이덴티티 로직은 JS가 작업 중이라
이번 작업은 표시(UI)만 깐다.**

## 1. 원작 조사

가디언나이트는 원작 내부 이름이 **DragonKnight**, 에셋 접두사 `ddk`, `EFTable_PC` 702행이다.
HUD는 두 문서로 나뉘며 **우리가 가진 두 태세에서 둘 다 화면에 있다.**

| 문서 | 패키지 | 내용 |
|---|---|---|
| `identitydragonknight.gfx` | `EFUI_IDENTITYDRAGONKNIGHT` | `DragonKnightSkinFrame` — 뿔 프레임, 오브 게이지, Z 키 판, 스킬상태 8점, 아크패시브 슬롯 2칸 |
| `identitydragonknightemberethgauge.gfx` | `EFUI_IDENTITYDRAGONKNIGHTEMBERETHGAUGE` | `IdentityDragonKnightEmberethGauge` — 다른 직업이 마나 바를 그리는 자리의 10소켓 바 |

AS에서 읽어낸 동작(추측 아님):

- `orbGauge`는 `ark.controls.Progress`. `target.width = percent * trackLength`이므로 **가로 방향
  왼→오른쪽 노출**이지 늘이기가 아니다. `track` 스프라이트는 태세당 한 프레임(1 인간 / 2 화신 /
  3 리베라티오)이고 이게 `inMarkGaugeState()`가 바꾸는 것이다. 우리에겐 1·2만 해당된다.
- `inMark`는 `Progress.updateMark`: `x = target.x + target.width`, y는 트랙 중앙, `useAutoHideMark`가
  정확히 0일 때와 정확히 최대일 때 숨긴다.
- `skillKey_lb` 색은 `DragonKnightSkinFrame.draw()`와 `set defaultGaugeValue`: **게이지가 꽉 찼거나
  화신 태세를 유지 중이면 흰색(0xFFFFFF), 아니면 0x686C20.**
- `stanceMc` 프레임 라벨 `normal`(1) / `dragon`(21) / `dragon_off`(73) / `liberatio`(93).
- `bloodGauge`는 `socket_0..socket_9`, x 오프셋 `-3,19,41,63,85,110,132,154,176,198`(y `-4`).
  4번과 5번 사이만 25px이라 5+5로 묶인다. `invokeDragonKnightBloodGauge(채워진수, 해금된수)`로
  해금 수를 넘어가는 소켓은 자물쇠(`lock`)를 쓴다.
- `slotAni_0`/`slotAni_1`은 아크패시브 슬롯이며 `DRAGON_AP12` / `NORMAL_AP13` 태세에서만 켜진다.
  **원작 스스로 우리가 가진 두 태세에서는 숨기므로 설치하지 않았다.**

좌표 변환은 전투 HUD가 이미 쓰는 것과 같다 — 엠블렘 중심(원작 960 ↔ HUD 673.5), Q줄 윗변
(원작 974 ↔ HUD 644.7), 2/3 배율. 기존 워로드 슬롯으로 검증했다(원작 846에서 시작하는 아이덴티티
프레임이 597.0에 저작돼 있고, 같은 식으로 계산하면 597.5가 나온다).

## 2. 구현

### `Tools/LpkPipeline/build_guardianknight_hud_ui.py` (신규)

원작 아틀라스에서 잘라 설치하고 레이아웃을 append 한다. 재실행해도 같은 결과다(idempotent).

- 아이덴티티 아트 10장 — 프레임 3겹(`Frame_Glow`/`Frame_Body`/`Frame_Inner`), 오브 3장
  (`Orb_Empty` 어두운 오브, `Orb_Fill_Human` 주황 불꽃, `Orb_Fill_Dragon` 붉은색), `Orb_Mark`,
  `SkillKey_Bg`, `SkillStatus_Plate`, `SkillStatus_Dot`.
- 엠베레스 아트 4장 — `Frame`, `Socket_Empty`, `Socket_Full`, `Socket_Lock`.
- 퀵슬롯 아이콘 18장 — `EFTable_Skill Icon/IconIndex → IconInfo.loa → ddk_skill_1` 아틀라스.
  평타 49000/49001은 **원작 테이블에도 아이콘이 없고** HUD에 LMB 슬롯도 없어서 일부러 뺐다.
- `Data/UI/HUD/HUD_Layout.json`에 슬롯 47개 append + `classes`에 `GuardianKnight` 추가.
  기존 문서의 CRLF와 서식을 보존했고 기존 슬롯은 한 줄도 건드리지 않았다.
- `Data/UI/HUD/GuardianKnightIdentity.json` — 각 슬롯이 무엇이고 원작 근거가 무엇인지 기록.

### `CMainApp`

- `Update_GuardianKnightIdentity()` 신규. 로컬 플레이어가 가디언나이트일 때만 돈다.
  - 마나 바 3슬롯을 숨긴다(원작이 그 자리를 엠베레스 바로 대체한다).
  - 오브: `Set_SlotFillRatio(identity/maximum)`, 태세에 따라 채움 텍스처 교체,
    `inMark`를 채움 경계로 `Set_SlotPosition` 하고 0/최대에서 숨김.
  - 스킬상태 8점과 판: 표시만. **스냅샷에 스킬별 사용 상태가 없어서 값을 지어내지 않고
    미사용 모습으로 둔다.**
  - 엠베레스: `invokeDragonKnightBloodGauge(채워진수, 해금된수)` 그대로 —
    `iEmberOrbs`가 점등 수, `iEmberMaximumSockets - iEmberLockedSockets`가 해금 수이고
    해금 수를 넘어가는 소켓은 자물쇠를 쓴다. 해금 수 계산식은 Server `CPlayerSkillSystem`이
    실제로 차감에 쓰는 capacity 식과 같은 것을 읽는다.
- Z 키 라벨을 텍스트 패스에 추가. `GK_Id_SkillKeyBg`는 keyframe이 아닌 실제 rect를 가진 슬롯이라
  기존 Artist/Warlord Z/X가 못 했던 중앙 정렬이 된다. 색은 위 원작 규칙 그대로.
- `SKILL_ICON_TABLE`에 GK 18행 추가(퀵슬롯 16 + Space 2).
- 기운 풀이 있는 클래스에서는 마나 바 아래 숫자 표시를 그리지 않는다. 그 자리는 이제
  오브와 소켓 아트가 차지하므로, 아트가 생기기 전까지 서 있던 숫자 자리표시자
  (`오브 N% 기운 X / Y`)를 걷어냈다.

## 3. 이번 작업에 넣지 않은 것

- **아이덴티티 게이지 충전·감소.** JS 담당. PR #437 기준 `GUARDIANKNIGHT`는
  `maximumIdentity: 100`이지만 `identityRegenPerSecond`/`identityDrainPerSecond`가 0이라
  오브는 0%로 그려진다. 값이 들어오면 코드 변경 없이 채워진다.
  기운 풀(`iEmberOrbs` 등)은 이미 Server 권위로 복제되고 있어 소켓은 바로 동작한다.
  (작업 도중 만들었던 서버 계약 초안 — 프로필 100/4/12 + 49040에 `requiresFullIdentity` 게이트 +
  bootstrap 35 — 은 `git stash` 항목으로 남겨 뒀다. 필요하면 그대로 꺼내 쓸 수 있다.)
- **태세 전환 연출.** `stanceMc`의 `dragon` 52프레임 / `dragon_off` 20프레임은 굽지 않았다.
  정지된 `normal` 포즈만 설치했다. 런타임(`CUILayoutRuntime`의 `keyframeAnimationPath` +
  `Play_KeyframeAnimation(슬롯, 라벨)`)은 이미 라벨→다음 라벨 재생을 지원하므로,
  구우면 코드 변경 없이 붙는다.
- **아크패시브 슬롯 2칸** — 위에 적은 대로 원작도 우리 태세에서 숨긴다.
- 게이지 만땅 이펙트(`normalFull_mc`/`apFull_mc`), `skillEffect`, `skillActive_mc`.

## 4. 검증

실행한 것:

- `cl /Zs` `MainApp.cpp` (utf-8, UNICODE 정의 포함) 오류 0.
- `HUD_Layout.json`, `GuardianKnightIdentity.json` JSON parse OK.
  레이아웃 diff는 append 전용(기존 9272줄 뒤에 추가 + `classes` 1줄), CRLF 유지, LF 혼입 0줄.
- `git diff --check` 통과. 추가한 C++ 라인 비ASCII 0자.
- 리소스 무결성: 레이아웃이 참조하는 GK 자산 37슬롯 전부 디스크에 존재(누락 0).
- 오프라인 레이아웃 합성으로 배치 확인 — 프레임·오브(62% 채움 + 경계 불꽃)·8점·엠베레스
  6/10 점등이 의도한 자리에 나온다. **이것은 클라이언트 실행 화면이 아니라 저작 좌표 합성이다.**

- (1차, `1b125a50` 기준) Debug 제품 빌드 **PASS**
  (`out/BuildPipeline/runs/20260921T184011866Z-debug-product.json`).
  `Client/Bin/Debug/Client.exe` 링크 성공, `OBJ=189 / PCH=0 / CSO=0 / binaries=1`.
  CSO 0은 셰이더를 안 건드려 기존 출력을 재사용한 정상 결과이고, 빈(0바이트) CSO도 없다.
  OBJ 189와 `tracking identity changed=True`는 방금 `origin/main` `1b125a50` 위로 리베이스한
  결과다(충돌 없음). `missingRuntimeInputs` / `invalidRuntimeInputs` 비어 있고
  Items·Valtan ClearRewards `CheckPublished` 모두 PASS.

하지 않은 것:

- 실제 클라이언트 실행과 화면 확인. **화면 최종 판정은 사용자가 한다.**
- 게시(publish) 없음 — 이번 변경에 실행 데이터 생성물이 없다.

## 5. 리소스 전달

`Client/Bin/Resources`는 Git 비추적이다. 이번에 추가된 폴더는 다음과 같다.

| 폴더 | 파일 수 | 크기 |
|---|---|---|
| `UI/HUD/GuardianKnight` | 10 | 131 KB |
| `UI/HUD/GuardianKnight/Embereth` | 4 | 24 KB |
| `UI/Skill/GuardianKnight` | 18 | 152 KB |

세 폴더를 팀에 전달해야 가디언나이트 HUD가 보인다. 없으면 해당 슬롯만 로드에서 빠지고
나머지 HUD는 그대로 동작한다.

## 6. 중간 산출물 (Git 비추적)

`D:/ClaudeWork/Extracted/HudGfx_dragonknight/`

| 항목 | 경로 |
|---|---|
| ffdec XML | `dragonknight_identity.xml`, `dragonknight_embereth.xml` |
| AS3 스크립트 | `dk_scripts/`, `dk_embereth_scripts/` |
| umodel 텍스처 | `tex/EFUI_IDENTITYDRAGONKNIGHT/` (20장), `tex_eb/…/` (1장) |
| 심볼 합성 미리보기 | `preview/DragonKnightSkinFrame.png`, `preview/DragonKnightStanceMc.png` |

작업 중 D: 외장 드라이브가 한 번 빠져서 중단됐다가 재연결 후 이어갔다. 산출물은 그대로 남아 있었다.

## 7. 같은 날 확인한 기존 문제 (이번 범위 밖)

`Tools/GameplayPipeline/Publish-GameplayBalance.ps1`이 첫 단계에서 실패한다.

```
KoukuSaydon composition validate failed: projected Product is stale:
  Data\Encounters\KoukuSaydon\KoukuSaydonEncounter.json
```

깨끗한 main에서도 동일하게 실패하므로 **이번 브랜치와 무관한 기존 문제**다. 쿠크 저작 원본
(`Data/KoukuSaydon/`)과 투영 산출물이 마지막 쿠크 커밋 `f291f886`에서 어긋난 것으로 보인다.
`project_kouku_saydon_composition.py --mode publish`로 재투영하면 풀리겠지만 다른 담당자의
생성물이라 건드리지 않았다. 이번 작업은 게시가 필요 없어서 진행에는 영향이 없었다.

## 8. 최신 main(`0ebd23cd`, PR #437) 위로 리베이스하며 맞춘 것

작업 중 main이 두 번 움직였다. 두 번째가 가디언나이트 브랜치를 다시 머지한 PR #437이고,
거기서 protocol이 102로 통일됐다. 그 머지가 남긴 것과 이번 HUD가 맞물리는 지점은 다음과 같다.

- **기운(ember) 풀이 이미 Server 권위로 복제되고 있었다.** `HUD_PLAYER_STATE`의
  `iEmberOrbs` / `iEmberLockedSockets` / `iEmberMaximumSockets`, Server의
  `GUARDIAN_EMBER_PROFILE`과 `CPlayerSkillSystem`의 차감·회복까지 들어와 있다.
  처음에 자원 비율로 소켓 수를 추측해 뒀던 것을 이 실제 값으로 바꿨고, 그 덕에 설치만 해 두고
  쓰지 않던 자물쇠 아트도 제자리를 찾았다.
- **숫자 자리표시자를 걷어냈다.** 머지된 코드가 마나 자리에 `오브 N% 기운 X / Y`를 그리면서
  주석에 `in place of mana until the class HUD art exists`라고 적어 두었다. 그 아트가 이번에
  생겼으므로 기운 풀이 있는 클래스에서는 이 숫자를 그리지 않는다.
- `SkillSlotMarks.json`의 GK 표식은 그쪽에서 이미 채웠다. 이번 작업은 건드리지 않았다.

## 9. 베른 내비게이션 게시 누락 (해결됨, 이번 범위 밖이었음)

작업 중 서버가 베른 월드 초기화에서 죽었다.

```
World simulation failed to initialize. World=1,
Status=Missing or truncated server navigation:
  ...\Server\Bin\DataFiles\Navigation\LV_BER_BERNCASTLE.Bern3.navgrid
```

커밋 `667886ff`(`feat(navigation): support stacked detail regions`)가 저작 원본
(`Bern3.navsource`/`.navpaint`)과 `navregions` 매니페스트, publisher·Server 코드까지는 넣고
게시 산출물을 빠뜨린 것이 원인이었다. 그 시점 `origin/main`에도 없었다.

임시로 `Tools/NavigationPipeline/Publish-ServerNavigation.ps1 -Mode Publish`로 로컬 복구했고,
그 뒤 `codex/bern-navigation-runtime-fix` 경로로 정식 게시본이 main(`3d7f7292`)에 들어왔다.
**로컬에서 구운 15개와 origin에 올라온 15개가 바이트 단위로 전부 동일했다** — publisher가
결정적으로 재생성된다는 증거다. 로컬 사본은 지우고 origin 것을 쓴다.

교훈: 게시물이 없어 보이면 **로컬 재게시나 원인 지목보다 `git fetch`로 origin을 먼저 확인한다.**
이 작업 중 main이 세 번 움직였다(`1b125a50` → `0ebd23cd` → `3d7f7292`, 그 사이 protocol 102 통일).
낡은 베이스에서 빌드하거나 게시하면 그대로 버려진다.

## 10. 첫 커밋 이후 사용자 화면 확인으로 잡은 것

빌드 후 사용자가 실제 화면에서 확인하며 보고한 순서대로 잡았다. 모두 원작 소스를 다시 읽어
근거를 찾은 뒤 고쳤고, 그 과정에서 내가 처음에 잘못 읽은 것도 함께 적는다.

### 오브가 가로로 닳던 것 (`800b4605`)

`ark.controls.Progress`가 `target.width`로 채우는 것은 맞지만, `orbGauge`의 배치 행렬이
`scaleX/Y ~0, rotateSkew0 -1, rotateSkew1 1`로 **-90도 돌아 있다**. 안쪽 `track`이 반대
회전을 들고 있어 그림만 똑바로 선다. 로컬 +X가 화면 위쪽이므로 원작 오브는 아래에서
차오르고 위에서 아래로 닳는다. 처음에 배치 행렬의 회전 성분을 읽지 않고 가로로 구현한 것이
원인이었다.

### 경계선이 구체 위로 벗어나던 것 (`4ddbcca7`)

비율이 float 나눗셈이라 최대값에서도 1에 정확히 못 미쳐 `useAutoHideMark` 조건을 통과했고,
수면에 중앙을 맞춘 마커가 꼭대기에서 절반을 밖으로 내밀었다. 양 끝 판정에 허용오차를 두고
마커 위치를 오브 안으로 제한했다.

### 소켓이 뚝 나타나고 사라지던 것 (`4ddbcca7`)

`DragonKnightBloodSocket`에 57프레임 타임라인이 있는데 정지 이미지만 깔았었다. 세 구간을
라벨 그대로 키프레임 문서 하나로 굽고(`build_guardianknight_socket_animation.py`), 소켓마다
키프레임 슬롯이 재생한다.

| 라벨 | 프레임 | 연출 |
|---|---|---|
| `show` | f2~21 | 보석이 드러나며 섬광 네 겹이 퍼졌다 사라진다 |
| `hide` | f23~38 | 불꽃이 `y -25 -> -41`로 떠오르며 `scale 0.97 -> 0.40`으로 사라진다 |
| `lock` | f43~55 | 자물쇠가 `scale 0.91 -> 1.00`으로 자리잡는다 |

**여기서 같은 실수를 반복했다.** 처음엔 행렬만 읽어 구웠더니 섬광이 안 사라지고 `hide`가
끝나도 보석이 남았다. `colorTransform`의 `alphaMultTerm`(256=1.0)이 실제 페이드를 담고 있다.
`gotchas`에 이미 "translate만 읽어서 틀렸다"고 적어 둔 것과 같은 종류다.

점등·잠금 수는 서버가 복제하는 기운 풀(`iEmberOrbs`,
`iEmberMaximumSockets - iEmberLockedSockets`)을 읽는다.

### 스킬 슬롯 점 (`4ddbcca7`)

여덟 개는 스킬 슬롯이고 빨간 점이 발현 스킬 자리다. 원작은
`_useList[i].visible = isUse`로 켜고 끌 뿐이고 판이 어두운 판이라 꺼진 자리는 검게 남는다.
전부 켜 두던 것을 아랫줄 1·2·4번만 켠다. 어느 자리인지는 프로젝트 데이터에 근거가 없어
사용자 지정이며 주석에 명시했다.

### 태세 밖 T 아이콘 (`4ddbcca7` -> `1104452c`)

가디언나이트 T는 화신 전용이라 인간형에서 칸이 비었다. 처음엔 태세를 쓰는 모든 직업에
적용했는데 사용자 결정으로 가디언나이트 T 하나로 좁혔다.

어두운 아이콘은 별도 자산이 아니다. IconInfo에 DDK 스킬 페이지는 한 벌이고 gray 변형이
없으며, 이름에 gray가 붙은 1063개는 전부 미니맵 심볼이다. `ARKNewSlot`이 `contentCanvas`에
활성일 때 `ColorTransform(1,1,1,1)`, 비활성일 때 `ColorTransform(0.3,0.3,0.3,1)`을 주므로
짐작했던 0.32/0.32/0.36 대신 그 0.3배 곱을 쓴다.

### 캐릭터 선택 목록 썸네일 (`70ac0788`)

건슬링어·차원술사·가디언나이트는 IconInfo에 136폭 썸네일이 없고 `ClassSelectImg_once_<PK>`
318폭 배너만 있다. 두 자산은 기울기가 78px 높이에 걸쳐 가로 15px로 **폭에 비례하지 않는
절대값**이라 같은 사다리꼴이다(버서커 14/1·1/14, 세 배너 14~15/1·1/14~15 실측). 슬롯 폭으로
자르되 배너 자신의 경사면 알파를 양끝에 실으면 원작 모양이 유지된다. 슬롯 크기는 상수로
적지 않고 `ClassSelectImg_0` 첫 칸에서 잰다. 높이만 넘치던 슬레이어(134x80)도 같이 잡혔다.

### 아이덴티티 문양 (`9dfb8e2f`, `fa6a3c0e`)

기존 네 직업이 원본 배율이 아니었다 — 워로드 0.81배, 창술사 0.83배, 도화가 0.71배로 축소,
차원술사는 1.23배로 확대. 가장 많이 줄인 도화가와 유일하게 늘린 차원술사가 뿌옇던 이유다.
`componentsv2`가 `class_<n>`마다 묶어 둔 export를 읽어 일곱 개를 원본 크기로 다시 잘랐다.

문양 구성이 직업마다 다른 것(워로드 3개, 창술사·가디언나이트 2개, 나머지 1개)은 원작
그대로이므로 크기를 서로 맞추지 않는다. 표시 높이만 기존 창술사 기준 71로 제한해 예전 크기
감각을 유지하고 설명 첫 줄(568.6) 앞에 12.6px 여백을 남긴다. 아트는 원본이고 슬롯에서만
줄이므로 확대는 없다.

가디언나이트 설명은 GameMsg `sys.classchange.identitydesc_dk`에서 가져왔다. 5줄이라
뒤로가기 아이콘(675) 위에 들어간다. 슬레이어·건슬링어는 사용자 결정으로 두지 않는다.

## 11. 최종 리소스 전달 목록

`Client/Bin/Resources`는 Git 비추적이다. 2026-09-22 사용자 공유 완료.

| 폴더 | 파일 수 | 비고 |
|---|---|---|
| `UI/HUD/GuardianKnight` | 10 | 신규 |
| `UI/HUD/GuardianKnight/Embereth` | 4 | 신규 |
| `UI/HUD/GuardianKnight/Embereth/Socket` | 8 | 신규(소켓 연출 조각) |
| `UI/Skill/GuardianKnight` | 18 | 신규 |
| `UI/ClassSelect/{Gunslinger,DimensionMaster,GuardianKnight}/IllustrationSmall.png` | 3 | 교체 |
| `UI/ClassSelect/*/NameID.png` | 7 | 교체 |

## 12. 최종 검증과 남은 것

실행한 것:

- `cl /Zs` `MainApp.cpp`, `Level_CharacterSelect.cpp` 오류 0
- JSON parse 3개(`HUD_Layout`, `EmberSocket`, `GuardianKnightIdentity`), 레이아웃·애니메이션
  참조 자산 누락 0, `git diff --check` 통과, 추가 C++ 라인 비ASCII 0자, CRLF 유지
- `Tools/Build` 게이트 6개 중 3개 실패 — 최신 `origin/main`에서 실패 수까지 동일한 기존 문제로,
  detached checkout으로 대조해 확인했다
- Debug 제품 빌드 PASS는 protocol 101 시점 기준이고, 이후 리베이스분(main `76a7ea8e`)은
  사용자가 직접 빌드한다

하지 않은 것:

- 최신 베이스에서의 링크와 실제 클라이언트 실행. **화면 최종 판정은 사용자.**
- 아이덴티티 게이지 충전·감소와 화신화 조건은 JS 담당이라 이번 범위 밖. 원작 설명 문구가
  "게이지가 가득 찼을 때 Z키 사용 시, 화신 상태로 전환"으로 조건을 명시하며, 작업 중 만들어
  둔 `requiresFullIdentity` 게이트 초안이 이 규칙과 같아 로컬 stash에 보관했다.
