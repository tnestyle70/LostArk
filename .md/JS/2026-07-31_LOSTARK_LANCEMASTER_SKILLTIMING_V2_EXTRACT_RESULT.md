# LostArk 창술사 스킬 타이밍 v2 추출 — 판정범위·히트스톱 포함

작성자: JS · 2026-07-31

애니메이션 툴의 이벤트 저작에 쓸 원작 전투 타이밍을 언팩 LPK에서 다시 뽑았다.
기존 v1(`hits/freeze/push/multi/interval` 5개 스칼라)은 스킬당 요약값 하나뿐이라
**히트마다 다른 판정범위와 히트스톱 블렌드를 표현하지 못했다.** v2는 히트 단위
레코드를 추가하면서 v1 필드는 한 글자도 바꾸지 않는다.

선행 조사는
[2026-07-30_LOSTARK_LANCEMASTER_ANIM_EVENTS_FROM_UNPACKED_LPK_RESULT.md](2026-07-30_LOSTARK_LANCEMASTER_ANIM_EVENTS_FROM_UNPACKED_LPK_RESULT.md)에 있다.

## 1. 결과

```text
Client/Bin/DataFiles/Anim/LanceMaster.skilltiming   v1 → v2 (UTF-8, CRLF)
  변형 행      50개  (v1과 동일한 50개 ID)
  히트 레코드 170개  (타이밍 명시 105 / 미명시 65)
  실제 스킬    23개
```

`.md` 산출물 외 기계 판독본은 scratchpad의 `lancemaster_timing.json`이며 원본
DB와 함께 Git에 올리지 않는다(Smilegate 저작물).

## 2. v2 포맷

```text
LOSTARK_SKILL_TIMING 2 "LanceMaster" 50
<변형ID> "<이름>" hits="a-b,c-d" freeze=<s> push=<s> multi=<n> interval=<s> \
                 sa=<n> move=<n> cd=<ms> base=<스킬ID>
  hit g=<n> key=<n> timed=<0|1> t=<ms> w=<ms> rep=<n> repms=<ms> \
      fz=<ms> fzin=<ms> fzout=<ms> fzcancel=<0|1> push=<ms> pushr=<cm> \
      area=<t> ar=<cm> aa=<deg> ah=<cm> ax=<cm> arem=<cm> maxt=<n> \
      hittype=<n> back=<0|1> counter=<0|1> hitset="<키>" sndset="<키>"
```

- **스킬 행은 v1과 완전 호환.** `sa/move/cd/base`는 뒤에 붙였고 v1 파서는 모르는
  키를 무시한다.
- **`hit` 연속 행은 v1 파서가 자동으로 버린다.** `Load_SkillReference`는 행마다
  `Read_Token` 다음 `Read_Quoted`를 요구하는데, `hit` 뒤에 따옴표가 없어
  `Read_Quoted`가 실패하고 `continue`로 넘어간다 (`Animation_Tool.cpp:713` 부근).
  **즉 지금 빌드로도 그대로 로드되고 화면은 이전과 동일하다.**
- 필드 대응: `t/w` = `HitTypeTimeMin` / (`Max`-`Min`), `rep/repms` =
  `MultiHitCount`/`MultiHitTime`, `fz*` = `FreezeTime`/`FreezeBlendIn`/`Out`,
  `area/ar/aa/ah/ax/arem` = `AreaType`/`Range`/`Angle`/`Height`/`OffsetX`/`RemoveRange`.
- **시간 단위는 전부 ms**(v1 스칼라만 초). 프레임 반올림 없이 원본 정밀도를 유지한다.

## 3. 방법

```text
DB   SourceData/LPK/data2/EFGame_Extra/ClientData/TableData/EFTable_*.db
실행 Blender 5.0 번들 파이썬 (표준 파이썬 없음)
```

1. `EFTable_Skill`에서 `PrimaryKey 34000~34999, SecondaryKey=1` → 창술사 43개 스킬.
2. `EFTable_GameMsg`의 `tip.name.skill_*` → 한글 스킬명.
3. `EFTable_SkillEffect`에서 `PrimaryKey 340000~349999, SecondaryKey=1` → 404개 효과.
4. 효과를 `PrimaryKey/10` = 변형ID로 묶고, 변형ID를 10 단위로 내려 스킬에 귀속.

## 4. 실측으로 확인한 것

- **`SecondaryKey`는 스킬 레벨이고 타이밍은 레벨 무관 동일**하다. `SecondaryKey=1`만
  보면 되고, 이걸로 211,948행이 404행으로 줄었다.
- **효과 PK = 변형ID×10 + n, 변형ID는 스킬ID+트라이포드 인덱스.** 34040(이연격)의
  변형이 34041·34042·34044·34047·34048이다. 이 산술 규칙을 `EFTable_SkillFeature`의
  `Type=43` 행(스킬이 소유한 효과 PK 목록)과 대조했더니 **12개 스킬 전부 일치**했다.
  Type=43은 12개 스킬만 덮어서 정본으로는 못 쓰지만, 독립적인 교차검증은 된다.
- **히트 윈도우는 판정 구간이 아니라 판정 시각 + 오차**다. 전체 테이블 기준 폭
  평균 **21.8ms**(최소 5, 최대 1500). 창술사는 16~100ms로 넓은 편인데도 30fps
  기준 1~3프레임이다.
- **다단히트는 윈도우 분할이 아니라 고정 간격 반복**이다. 34047은 `t=0` 인데
  `rep=3, repms=300` 이고, 34118처럼 첫 타 시각과 반복이 함께 오는 경우도 있다.
  → 현재 툴의 `iHitCount`("윈도우에 균등 분배") 모델은 원작과 다르다.
- **히트스톱은 단순 정지가 아니다.** `fz=450, fzin=150, fzout=250` 처럼 인/아웃
  블렌드가 따로 있다. 전체 최빈값은 `fz=325`(118,344행 중 66,358행).
- **판정범위는 히트마다 바뀐다.** 34130은 한 스킬 안에서
  `area=3 ar=270 aa=360 ax=180` → `area=2 ar=530 aa=270 ax=-10`으로 달라진다.
  v1은 이걸 담을 자리가 없었다.
- `AreaType` 의미(전체 분포 역산): `0`=범위없음, `1`=박스/직선(각도 0),
  `2`=부채꼴(각도 평균 325), `3`=원/도넛(`arem`이 안쪽 구멍, 2,583행이 360도).
- 일부 스킬은 `push`가 **음수**다(34072·34567·34611·34660). 밀림이 아니라 끌어당김
  으로 보인다. v1 스칼라는 0으로 클램프했고 그 동작을 유지했다. 부호는 `hit` 행에 남아 있다.

## 5. 한계 — 정직하게

- **타이밍이 명시된 히트는 105/170뿐이다.** 나머지 65개는 `HitTypeTime`과
  `MultiHitCount`가 모두 0이고 범위·경직만 있다. 이 히트들은 엔진 기본 시점에
  터지는 것으로 보이며, **우리 쪽에서 시각을 저작해야 한다.**
- 23개 스킬만 타이밍을 갖는다. 나머지 20개(태세·이동·기상 등)는 판정이 없거나
  버프성이라 히트 행이 없다.
- **이펙트·사운드 스폰 시각은 여전히 없다.** 792개 DB 전수 검색에서 애니 클립명
  (`gabolg`, `penetrationlunge`, `sk_riseup`)과 파티클명(`par_*_flm_*`)을 참조하는
  컬럼이 **0개**였다. `hitset`/`sndset`은 피격자 쪽 FX/사운드 **세트 이름**이지
  시전 이펙트 타임라인이 아니다. 궁극기 3종만 SkillCam UPK에 실제 시각이 있다.
- `ar/ax` 등 거리 값은 UE3 단위(≈cm)다. **우리 모델 스케일과 1:1이 아닐 수 있으니
  환산 계수를 먼저 정해야 한다.**

## 6. 검증

- v1 파서를 파이썬으로 재현해 구/신 파일을 파싱하고 v1 필드 6종(name·hits·freeze·
  push·multi·interval)을 50행 전부 비교 → **차이 0**. ID 집합도 동일.
- 연속 행이 v1 파서에서 버려지는지 `Read_Token`/`Read_Quoted` 실제 구현으로 확인.
- 인코딩·개행: 기존 파일과 동일하게 UTF-8 / CRLF / BOM 없음으로 맞춰 diff가
  추가분만 나오게 했다. ImGui 폰트는 `GetGlyphRangesKorean()`이 걸려 있어 한글이 나온다.
- 코드 변경이 없으므로 빌드는 돌리지 않았다. 툴에서 육안 확인은 다음 단계에서 한다.

## 7. 툴 반영 (2단계)

뽑은 값을 실제로 쓸 수 있게 `CAnimation_Tool`을 고쳤다.

### 7.1 이벤트 종류

구간형과 점형 두 축으로 나눴다. 개념마다 종류를 늘리지 않는다.

```text
구간  HIT / CANCEL / SUPERARMOR / INVULN / MOVE
점    SOUND / EFFECT
```

`SOUND`와 `EFFECT`를 payload 하나로 합치지 않은 건 서로 다른 서브시스템이 발사하기
때문이다. 판정은 `Is_Window()` 하나로 한다.

### 7.2 시간 단위 — 프레임에서 ms로

`ANIM_EVENT`가 `iStartMs`/`iEndMs`를 들고, **프레임은 표시 단위로만** 쓴다.
원작 히트 윈도우 평균이 21.8ms라 30fps 프레임(33.3ms)으로 반올림하면 정밀도가
날아가기 때문이다.

환산은 추측한 fps가 아니라 **클립 자신의 틱 레이트**로 한다. 이를 위해
공개 헤더를 두 줄 넓혔다.

```text
Engine/Public/Animation.h   CAnimation::Get_TickPerSecond()
Engine/Public/Model.h       CModel::Get_AnimationTickPerSecond(iAnimIndex)
Engine/Private/Model.cpp    구현 (범위 밖이면 0 반환 → 호출부가 기본값 판단)
```

**공개 헤더 변경이므로 `UpdateLib.bat` 후 Client까지 다시 빌드해야 한다.**
덕분에 기존 `m_fFps` 필드는 필요가 없어져 제거했고, `cast offset`도 프레임에서
ms로 바뀌었다.

### 7.3 다단히트 모델 수정

원작과 어긋나 있던 부분이다.

```text
이전  윈도우 start..end 에 iHitCount 개를 균등 분배
이후  hit N = iStartMs + N * iRepeatMs, 윈도우 폭은 매 히트의 오차 허용치
```

`Get_TickFrame` → `Get_TickMs`로 바뀌었고 `Get_ActiveTick`은 반복 히트마다
윈도우를 검사한다.

### 7.4 HIT_PARAMS

`HIT_PARAMS`를 만들어 `ANIM_EVENT`와 `SKILL_HIT`가 같은 구조체를 쓴다. 스탬프가
필드별 번역이 아니라 **구조체 복사** 한 줄이 된다.

```text
반복   iRepeatCount / iRepeatMs
경직   iFreezeMs / iFreezeInMs / iFreezeOutMs
넉백   iPushMs / iPushRange          (음수 iPushMs = 끌어당김)
범위   iAreaType / iAreaRange / iAreaAngle / iAreaHeight / iAreaOffsetX
       iAreaInner / iMaxTargets
```

거리는 **원본 게임 단위 그대로** 둔다. 우리 월드 스케일 환산은 소비하는 쪽이 갖는다.

### 7.5 파일 포맷 v3

```text
"clip" HIT startms=1657 endms=1744 rep=3 repms=300 fz=450 fzin=150 fzout=250 \
       push=200 pushr=30 area=3 ar=250 aa=120 ah=150 ax=-30 arem=0 maxt=0
"clip" CANCEL startms=800 endms=1200
"clip" SOUND  startms=500 payload="..."
```

v1(위치 컬럼)·v2(프레임 key=value)도 계속 읽고, 로드 시 클립 틱 레이트로 ms 변환한다.

### 7.6 검증

- Engine x64 Debug → `UpdateLib.bat Debug` → Client x64 Debug **오류 0**
- v2→v3 변환을 파이썬으로 재현해 커밋된 `.animevents` 22개 HIT 행을 전수 대조:
  **5개 히트가 1프레임 이동**한다. 전부 v2가 정수 나눗셈으로 내림하던 **반프레임
  경계**값이고(예: 2·12.5·23 프레임 → v2는 12, v3는 13), v3는 반올림한다.
  숨기지 않고 **로드 시 상태줄에 변환 경고를 띄우도록** 했다.
  `flm_sk_strongrotational_02`, `flm_sk_talonstrike`, `flm_sk_squalllance_02` 3개
  클립이 해당되니 저장 전에 눈으로 확인할 것.
- `.animevents`는 아직 v2인 채로 두었다. 툴에서 확인한 뒤 저장하면 v3로 바뀐다.
- **실행 확인은 아직 안 했다.** TEST_LEVEL2에서 F1로 열어 클립 선택 → 스탬프 →
  저장 → 재로드까지 육안 확인이 남아 있다.

## 8. 정정 — 노티파이는 존재한다 (3단계)

**5장의 "이펙트·사운드 스폰 시각은 없다, 우리가 저작해야 한다"는 결론은 틀렸다.**
792개 SQLite만 뒤지고 `XmlData/Action`을 안 봤다. 원작 노티파이는 전부 있다.

### 8.1 어디에 있었나

```text
data3/EFGame_Extra/ClientData/XmlData/Action/LANCEMASTER.loa   3.5MB
CEFActionObject → CEFActionStage → CEFActionNotify_*
```

애니 에셋(`UAnimSequence`)에 노티파이가 없다는 07-30 조사는 맞았다. 다만 **없는 게
아니라 Action 상태머신이 소유**한다. 합성은 3층이다.

| 층 | 소유 | 내용 |
|---|---|---|
| 애니 에셋 | `PC_FLM_00.upk` | 포즈 키만 |
| **Action** | `XmlData/Action/LANCEMASTER.loa` | **파티클·사운드·셰이크·캔슬·슈퍼아머 시각** |
| SkillEffect | `EFTable_SkillEffect.db` | 전투 판정(서버 권위) |

창술사 하나에 노티파이 **7,817개**:

```text
PlayParticleEffect 2962   InputTiming 1290   AKEvent 836   PawnMaterialParam 678
Effect 375   ViewShake 333   Anim 292   ParticleHit 282   TrailGhostEffect 221
SuperArmor 142   SuperArmorPVP 81   Trails 65   UltimateSkillCameraControl 4
```

### 8.2 레코드 레이아웃 (실측 복원)

```text
"CEFActionObject\0"   int32 reserved   int32 skillId   <len-prefixed 표시명>
"CEFActionNotify_X\0" +0..+15 flags   +16 float start   +20 float end   +24 float duration
```

문자열 길이 프리픽스는 **양수면 ASCII(널 포함), 음수면 UTF-16LE 코드유닛 수**다.
한글 스킬명이 음수 쪽에 들어 있다. `start`와 `end`는 7,817개 중 4개만 다르므로
실질 구간은 `start .. start+duration`이다.

### 8.3 시계가 다르다 — 중요

- **Action 노티파이 시각은 클립 로컬**이다. 검증: 노티파이 시작의 **98.9%
  (7410/7495)** 가 자기 스테이지 `Anim` 노티파이의 duration(= 클립 길이) 안에 든다.
  넘어가는 85개는 클립보다 오래 사는 파티클이다.
- **SkillEffect DB 시각은 시전 전체 기준**이다(1445·1657ms 등).
- 즉 **툴이 저작하는 시계는 Action 쪽과 같다.** cast offset이 필요 없다.
- DB의 `1445`가 여러 스킬에 반복 등장하는 걸 보면 그쪽은 공용 기본값에 가깝고,
  클립별 실제 타이밍은 Action이 정본이다.

### 8.4 산출물

```text
Client/Bin/DataFiles/Anim/LanceMaster.clipmap     클립 142개 → 스킬ID + 한글명
Client/Bin/DataFiles/Anim/LanceMaster.animnotify  클립 142개 / 노티파이 4,289행
```

```text
"flm_sk_sharpswing_01" skill=34040 len=0.6000 name="긴 창_이연격"
  n t=0.1981 d=0.4019 kind=CANCEL src=InputTiming asset=""
  n t=0.3000 d=0.3000 kind=HIT    src=ParticleHit asset=""
  n t=0.3024 d=0.0000 kind=EFFECT src=PlayParticleEffect asset="FX_PC_FLM_01.Par_M_FLM_TwoAttTrail_02"
  n t=0.0000 d=0.6000 kind=SOUND  src=AKEvent asset="PC_LANCEMASTER_F.PC_LanceMaster_F_SK_SharpSwing1_FX000_1"
```

종류별: EFFECT 2506, CANCEL 725, SOUND 522, SHAKE 219, HIT 171, SUPERARMOR 146.
에셋명은 SOUND 522/522, EFFECT 2091/2506에서 나온다. HIT·CANCEL·SHAKE·SUPERARMOR는
순수 타이밍이라 에셋이 없는 게 정상이다.

### 8.5 클립 ↔ 한글 스킬명

`CEFActionObject`의 skillId + 한글명, `CEFActionNotify_Anim`의 클립명으로 매핑했다.
클립명 규칙은 `flm_` + 소문자. 200개 참조 중 173개가 모델 클립으로 해석되고,
나머지 27개는 공용 애니(`Idle_Normal_1`, `Run_Normal_1`, `Att_Normal_1_*`)라
다른 패키지 소속이다. **스킬 클립은 전부 해석된다.**

DB에 없는 **긴 창/짧은 창 구분**이 여기서 나온다(`긴 창_이연격`, `짧은 창_나선창`).

교차검증: `SkillFeature Type=43`(스킬이 소유한 효과 PK 목록)이 12개 스킬에 대해
4장의 산술 그룹핑과 전부 일치했다.

**정정**: 07-30 문서가 궁극기를 "Squall Lance(스퀄 랜스)", "Gae Bolg(게볼그)"로
적었는데 그건 내부 코드명이다. `SK_Super_SquallLance_*`를 쓰는 건
**34630 초각성기_연가창식: 마룡합일섬**이다.

### 8.6 툴 배선 — 원작 노티파이 임포트

`EVENT_KIND`에 `SHAKE`를 추가하고(점 이벤트), 이벤트 패널에
**`Import original (N of M)`** 버튼과 종류별 체크박스를 붙였다.

- 임포트한 이벤트는 `bImported` 플래그를 갖고 파일에 `src=orig`로 저장된다.
- **재임포트는 이 플래그가 붙은 행만 지우고 다시 넣는다.** 손으로 만든 이벤트는
  살아남고, 버튼을 여러 번 눌러도 결과가 같다.
- 리스트에 `*orig` 표시가 붙어 손작업분과 구분된다.
- 같은 파티클을 여러 부착점에서 쏘는 중복행은 `(kind, start, end, payload)`가
  같으면 한 줄로 합친다.
- 기본 체크는 **HIT / CANCEL / SUPERARMOR / SHAKE**다. EFFECT 2506·SOUND 522는
  양이 많아 필요할 때 켜서 받는다.
- 시각 변환은 `초 × 1000` → ms. Action 시계가 클립 로컬이라 offset이 없다.

### 8.7 검증

- Client x64 Debug **오류 0, 경고 0**
- C++ 파서(`Load_ClipNotify` + `Import_Notifies`)를 파이썬으로 재현해 실제 파일에
  적용: **클립 142/142, 4,289행 전부 파싱, 건너뛴 줄 0.**
- 기본 체크 기준 임포트량: 후보 1,261 → 중복 제거 후 **1,057건**. 한 클립 최대
  23건(`flm_sk_riseup_01`)이라 편집 가능한 규모다.
- 파일 최대 줄 길이 123바이트 < `fgets` 버퍼 2048.
- **실행 확인은 아직 안 했다.** F1 → 클립 선택 → Import → 스크럽으로 히트가 포즈와
  맞는지, 저장/재로드가 왕복하는지 육안 확인이 남았다.

## 9. 클립 재생 순서 — 시퀀스 그룹

"클립이 1·2·3 순서로 가는 것도 있고 뒤죽박죽인 것도 있다"의 답. 노드 구조가 있다.

```text
CEFActionObject (스킬)
  └ 그룹 = 재생 체인 하나        ← 트라이포드 변형마다 하나
      └ CEFActionStage (노드)    ← 클립 1개 + 그 클립의 노티파이
```

그룹 헤더는 그룹 첫 스테이지 **앞 16바이트**에
`[flag, flag, 그룹번호, 스테이지수]`로 들어 있다.

읽을 때 걸린 함정 세 개:

- 그룹번호가 **0에서 시작하지 않는다** (34620은 2부터)
- **연속이 아니다** (34060은 0,1,2,**4**,5,6 — 3이 빈다)
- 첫 필드가 0일 때도 1일 때도 있다

세 제약을 다 풀고 나서야 전부 맞았다.

### 9.1 검증

```text
sum(스테이지수) == 실제 스테이지 개수 : 55 / 55 액션
클립 슬롯 292개 == Anim 노티파이 292개 : 정확히 일치
```

두 번째가 강한 근거다. 모든 `Anim` 노티파이가 정확히 한 슬롯에 배정되고 남거나
모자라지 않는다. 34630 마룡합일섬의 체인
(`super_squalllance_01→02→03→04`)은 SkillCam UPK에서 뽑은 타임라인
(0.000/1.787/3.547/4.347s)과 클립도 순서도 같다.

### 9.2 산출물

```text
Client/Bin/DataFiles/Anim/LanceMaster.clipseq   체인 69개 / 스킬 26개 / 슬롯 204개
```

클립 리스트가 에셋 순서라 뒤섞여 보였을 뿐이고, 실제 순서는 이 체인이다.
반월섬(34110)은 클립 16개가 체인 6개로 갈린다.

```text
seq2  custom_7_01 → custom_7_02 → custom_7_03 → custom_5
seq3  custom_7_01 → custom_7_02 → custom_37   → custom_5
```

`_start → _loop → _end`(적룡포), `01→02→03→04`(평타 콤보) 같은 패턴이 그대로 보인다.
체인이 1개짜리인 그룹은 순서 정보가 없어 뺐다.

### 9.3 툴 배선

Playback 아래 **Chain** 섹션. 현재 클립이 속한 체인을 전부 보여주고, 단계 버튼을
누르면 그 클립으로 이동한다. 현재 단계는 초록색, 호버하면 클립명과 시전 시작
기준 시각이 뜬다. `use offset N ms` 버튼이 `m_iCastOffsetMs`를 그 값으로 채운다.

체인 오프셋은 `.animnotify` 헤더의 `len=`(게임이 가진 클립 길이)을 앞에서부터
누적해 구한다.

### 9.4 검증과 한계

- Client x64 Debug **오류 0**, 내 파일발 경고 0
  (`C4819`는 `GameInstance.h` 기존 경고)
- `.clipseq` 파싱 재현: **체인 69/69, 파싱 실패 0, 체인 클립 204개 전부 길이 보유**
- **체인 오프셋 + 클립 로컬 히트 시각으로 DB의 시전 전체 기준 시각을 재구성해보면
  159개 중 35개만 120ms 안에 든다.** 가까운 건 아주 가깝다(굉열파 1250 vs DB 1245,
  반월섬 1600 vs DB 1590). 안 맞는 이유는 DB 효과 변형과 Action 트라이포드 체인을
  서로 짝지어 비교한 게 아니라 전체를 교차 비교했기 때문으로 보인다.
  **즉 오프셋 계산 자체는 맞지만 두 소스가 1:1로 정렬되지는 않는다.**

## 10. 재생 방식(mode)과 캔슬 레이블

### 10.1 InputTiming 안에 있던 것

`CEFActionNotify_InputTiming` 1,290개 중 **863개가 한글 레이블**을 갖는다.
음수 길이 프리픽스 UTF-16 문자열이고, 정수 필드에 타입 코드는 없다
(타입인 줄 알았던 `ints[3]`은 그 문자열의 길이 프리픽스였다).

```text
연가심공 243   [선스] 185   [선이] 182   회피기캔슬(테스트) 92
회피기 및 절룡세 캔슬 68   [선콤] 22   입력캔슬 9   이동캔슬 9   평타 캔슬 6
스탠스변경 3   회전동기화 4   ...  총 23종
```

`(테스트)`가 섞인 걸 보면 **정식 enum이 아니라 기획자 주석**이다. 그래도 캔슬
종류 분류로는 그대로 쓸 수 있다. 427개는 레이블 없는 순수 타이밍 윈도우다.

윈도우 3개는 **다음 스킬을 이름으로 가리킨다**(열공참 t=0.800 → 긴 창_회선창,
연환섬 t=1.950 → 긴 창_선풍참혼, 스탠스변경 → 긴 창_평타). 열공참은 실제로
`34061 열공참 체인` 액션도 따로 있다. 다만 3개뿐이라 일반 입력→스킬 라우팅은
이 데이터에 없다.

### 10.2 재생 방식은 원본에 없다 — 추론으로 채웠다

콤보/차징/원샷 구분은 어느 필드에도 없다. `[선콤]`이 8개 스킬에 붙지만
"이 스킬이 콤보"인지 "여기서 다음 입력을 버퍼링"인지 확정이 안 된다.

그래서 **클립 이름 패턴으로 분류해 `.clipseq`에 `mode=`로 넣었다.**

```text
HOLD     _start/_loop/_end 삼종      6    적룡포, 청룡출수
ONESHOT  각성기/초각성 또는 super_*    7    연가창식 4종, 초각성 2종
COMBO    _01.._0N 균일 넘버링        14   평타, 연환섬, 굉열파
SEQUENCE 나머지(대부분 2단)          42   이연격 01→02
```

각성기는 클립이 `squalllance_01→02→03`처럼 균일 넘버링이라 COMBO로 잘못 잡혔다.
게임 자체 명명(`연가창식`/`초각성` 접두)으로 ONESHOT을 강제해 고쳤다.
**추론값이므로 애매한 건 손으로 고치는 걸 전제로 한다.**

### 10.3 산출물과 툴 반영

```text
.clipseq   v2  — mode= 추가 (체인 69개)
.animnotify    — CANCEL 행에 label= 추가 (레이블 보유 461행)
.animevents    — 원본 재생성, 2,988 → 3,050건
```

- 윈도우형 이벤트도 `payload`를 저장하게 바꿔 캔슬 레이블이 왕복한다.
- Chain 섹션이 `[COMBO]` 같은 mode를 같이 보여준다.
- 이벤트 리스트의 윈도우 행에 레이블이 붙고, 선택하면 편집도 된다.

**앞서 "1ms 차이 중복"이라고 적었던 CANCEL 198/199ms는 중복이 아니었다.**
`[선스]`와 `[선이]`로 서로 다른 윈도우다. 레이블이 붙으면서 드러났고,
그래서 이벤트 수가 62건 늘었다.

### 10.4 검증

- Client x64 Debug **오류 0**
- `.animevents` 왕복: **3,050 / 3,050 파싱, 건너뛴 줄 0, 전부 `src=orig`**
- 재임포트 멱등성: **추가 0 / 삭제 0**
- 네 파일 모두 UTF-8 / CRLF, 최대 줄 270바이트 < 버퍼 2048
- **실행 확인은 아직 안 했다.**

## 11. 런타임 — 키 입력으로 체인 재생

### 11.1 구조

기존 분리를 그대로 따랐다. 새 파일은 만들지 않았다.

```text
CCharacter (공용)          체인 로드 + 재생   Play_Skill / Update_Chain
CLogic_LanceMaster (JS)    키 -> 스킬ID       Binds[]
```

- `CHARACTER_SPEC`에 `pAssetName`("LanceMaster") 한 필드 추가.
  `../Bin/DataFiles/Anim/<name>.clipseq`를 찾는 데 쓴다. 스펙 구현체가
  `Spec_LanceMaster` 하나뿐이라 다른 팀원 코드에 영향 없다.
- `CCharacter::Update`가 로직보다 **먼저** `Update_Chain()`을 돌린다. 로직이
  `Is_PlayingSkill()`을 읽을 때 이미 최신이다.
- 시전 중에는 로직이 입력을 무시한다. 캔슬 윈도우는 추출해뒀지만 아직 미배선.

### 11.2 두 개의 함정

**1. `Set_Animation`은 트랙을 되감지 않는다.** `CModel::Set_Animation`은 인덱스만
바꾸고 `m_fCurrentTrackPosition`은 그대로 둔다. 끝까지 재생된 클립을 다시 걸면
**즉시 "끝남" 상태**가 된다. 체인이 같은 클립을 반복하는 경우가 실제로 있어
(적룡포 `_loop→_end→_loop→_end`, 격돌 `_loop→_loop`) 그대로 두면 체인이
한 프레임에 끝까지 흘러버린다.
→ `Start_Clip()`이 `Set_AnimTrackPosition(idx, 0)`으로 항상 되감는다. 공개
`Set_Animation`의 기존 의미는 건드리지 않았다.

**2. 시퀀스 번호가 0부터가 아니다.** 9장에서 확인한 그대로 런타임에도 걸렸다.
바인딩 11개 중 3개(열공참·회선창·맹룡열파)가 `seq=0`이 없어 재생 실패했다.
→ `Play_Skill`이 정확한 인덱스가 없으면 **그 스킬의 가장 낮은 체인**으로 폴백한다.

그리고 열공참은 체인이 아예 없었다. `.clipseq` 생성 때 클립 1개짜리 그룹을
버렸기 때문이다. 순서 정보가 없다는 이유였는데, **재생에는 필요하다.**
→ 단일 클립 그룹도 포함하도록 재생성. 체인 69 → **107개**, 스킬 26 → **35개**.

### 11.3 어떻게 붙였나

`Play_Animation`의 `isFinished`를 꺼내려고 `CPart_Body`에 접근자를 넣었다가
되돌렸다. `CContainerObject`에 파트 객체 접근자가 없어 `CCharacter`가 `CPart_Body`를
못 잡기 때문이다. Engine 공개 헤더를 늘리는 대신 `CCharacter`가 이미 들고 있는
`m_pBodyModel`로 `Get_AnimationProgress`를 읽어 `position >= duration`으로 판정한다.
`CAnimation`이 완료를 반환하는 조건과 **같은 식**이다(트랙이 끝을 넘어선 채로
남는다). 결과적으로 Engine 변경 0, 죽은 코드 0.

### 11.4 키 바인딩

```text
Q 이연격   W 열공참   E 회선창   R 반월섬
A 연환섬   S 질풍참   D 선풍참혼  F 맹룡열파
Z 청룡출수  X 평타     1 마룡합일섬(각성기)
```

DirectInput은 레벨만 주므로 눌림 edge는 로직에서 직접 추적한다
(`Camera_Free`와 같은 방식).

### 11.5 검증

- Client x64 Debug **오류 0** / 내 파일발 경고 0 (`C4819`는 `GameInstance.h` 기존)
- `Load_ClipChains`의 `atoi`+`strstr` 파싱을 재현해 실제 파일에 적용:
  **체인 107개 로드, 스킬 35개, 체인 클립 242개 전부 모델에 존재(누락 0)**
- **바인딩 11/11이 전부 `seq=0`으로 해석된다**
- 건드린 파일 전부 ASCII 유지. `Part_Body`는 되돌려 diff 없음
- **실행 확인은 아직 안 했다.** F1 툴로 클립은 확인했지만, 실제로 키를 눌러
  체인이 이어 붙는지는 봐야 한다.

### 11.6 아직 아닌 것

- `mode`는 읽어서 들고만 있고 분기하지 않는다. COMBO도 한 번 누르면 끝까지 간다.
- 캔슬·선입력 미배선. 시전 중 입력은 버려진다.
- 이동·회전 없음. 제자리에서 클립만 재생한다.
- 실무적으로는 DB 시각이 이제 거의 필요 없다. Action 노티파이가 클립 로컬
  히트 시각을 직접 주기 때문이다. cast offset은 `.skilltiming` 행을 굳이 참조할
  때만 쓴다.
- **실행 확인은 아직 안 했다.**
