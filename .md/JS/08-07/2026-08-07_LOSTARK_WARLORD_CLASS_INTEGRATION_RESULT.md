# 워로드 클래스 통합 — 구현 결과

작성자: JS · 2026-08-07 · 브랜치 `feature/player-hold-skill`

여섯 번째 playable class를 추출부터 인게임 렌더까지 연결했다. 앞선 다섯 클래스와 같은
파이프라인을 쓰되 세 군데가 다르고, 그 차이가 이 문서의 핵심이다.

## 1. 클래스 좌표

```text
EFTable_PC  PK 104  Name='Gunlancer'  →  tip.name.enum_playerclass_warlord = 워로드
  LookInfo   = EFDLChar_PC_WGL.PC_WGL       → 에셋 코드 wgl
  BaseClass  = 101 Warrior (PC_WR)           → 베이스 바디 pc_wr_00_sk
  GenderType = 2 (male)
  TownDefaultWeapon = 1013002 → EFTable_Item.Model = WP_WWGL_04
스킬 블록   LearnClass=104 → 17xxx
Action      XmlData/Action/GUNLANCER.loa
```

슬레이어가 `Berserker_Female`이었던 것과 같은 패턴이다. **파일명과 PC.Name은 한글 클래스명과
다르므로 매번 DB로 잡는다.**

## 2. 앞 다섯 클래스와 다른 점 셋

### 2.1 바디가 머리카락을 안 그린다

판정은 README대로 베이스 바디 psk의 `MATT0000` 슬롯으로 했다.

```text
pc_wr_00_sk (7슬롯)  lower / upper / base_upper / arm / base_upper / face / eye
```

`face`와 `eye`는 있는데 `hair`가 없다. 건슬링어·도화가·슬레이어는 바디가 셋 다 그려서
face/hair 타깃이 없었고, 창술사는 둘 다 따로였다. **워로드는 face 타깃 없음 + hair 타깃 있음**이라
`pc_wr_00_hair_sk`가 별도 파츠로 들어간다. `PC_WR_00_FACE`의 `pc_wr_00_face_sk`는 재질이
`pc_wr_face_mi_high`인 고품질 변형이라 슬레이어 `high_sk`와 같은 이유로 제외했다.

그 결과 `CHARACTER_PROTOTYPE_TAGS::Equipment`가 `std::array<..., 5>`로는 모자라 6으로 늘었다.

### 2.2 무기가 둘이다

`WP_WWGL_04` 패키지에 메시가 둘 있다.

| 메시 | 본 | 재질 |
|---|---|---|
| `wp_wwgl_04_sk` 총창 | 3 (`b_weapon_lance`, `_01`, `_02`) | `wp_wwgl_04_mi` |
| `wp_wwgl_sd_04_sk` 방패 | 1 (`b_weapon_shield`) | `wp_wwgl_04_mi` |

창술사처럼 교체가 아니라 **동시 장착**이다. 소켓은 바디 리그에서 읽는다 — `wr` 리그는
창술사·슬레이어 계열(`b_weapon_rhand` / `b_weapon_lhand`)이고 `b_wp_*`가 없다. 앞 네 클래스에
없던 `b_weapon_hold_01`이 하나 더 있지만 쓰지 않았다.

### 2.3 스탠스가 창술사에 이어 두 번째다

Z가 방어 태세 진입(17800)과 해제(17810)로 나뉜다. 인게임에서 누르면 모션이 바뀌고 다시 누르면
돌아온다는 것을 확인해 창술사와 같은 스탠스 구조로 넣었다. publisher의 슬롯 충돌 규칙이 같은
슬롯에 두 스킬을 허용하려면 **양쪽 다 non-NONE 스탠스**여야 하므로 `WARLORD_NORMAL`과
`WARLORD_DEFENSE` 둘을 추가했다. 기본 스탠스는 `WARLORD_NORMAL`이다.

## 3. 슬롯과 수치

| 키 | skillId | 이름 | 종류 | 쿨다운 | 마나 | 배율 |
|---|---|---|---|---|---|---|
| LMB | 17000 | 기본 평타 | COMBO 3단 | — | — | 100 |
| Q | 17030 | 날카로운 창 | ACTIVE | 5s | 211 | 862 |
| W | 17060 | 파이어 불릿 | ACTIVE | 7s | 256 | 1255 |
| E | 17080 | 대쉬 어퍼 파이어 | ACTIVE | 16s | 412 | 353 |
| R | 17110 | 리프 어택 | ACTIVE | 16s | 412 | 1580 |
| A | 17090 | 갈고리 사슬 | ACTIVE | 24s | 521 | 397 |
| S | 17040 | 배쉬 | ACTIVE | 10s | 315 | 587 |
| D | 17100 | 방패 격동 | ACTIVE | 12s | 349 | 725 |
| F | 17140 | 가디언의 낙뢰 | ACTIVE | 20s | 469 | 2077 |
| T | 17240 | 풀배럴 캐넌 | **HOLD** | 75s | 907 | **784** |
| X | 17820 | 전장의 방패 | ACTIVE | 90s | 0 | — |
| V | 17170 | 가디언의 수호 | ACTIVE | 300s | 0 | — |
| ALT+V | 17250 | 수호의 맹세 | ACTIVE | 300s | 0 | — |
| Z | 17800/17810 | 방어 태세 진입/해제 | ACTIVE | 2s | 0 | — |
| SPACE | 17020 | 돌격 | ACTIVE | 6s | 0 | — |

바인딩은 전부 `.clipseq`의 트라이포드 없는 기준 체인(스킬별 최저 seq)이다. 배쉬(17040)와
수호의 맹세(17250)는 `seq=0`이 없어 seq=1을 쓴다.

### 3.1 X 슬롯을 새로 열었다

전장의 방패가 X에 오면서 `Publish-GameplayBalance.ps1`의 슬롯 화이트리스트와
`CPlayerController`의 `SlotKeys`에 `X`/`DIK_X`를 추가했다. 다른 클래스에 전례가 없는 슬롯이다.

### 3.2 풀배럴 캐넌 데미지는 다른 대역에 있었다

`skillId × 10 + 변형` 규약으로 `172400~172409`를 뒤지면 행이 하나도 안 나온다. `.skilltiming`에도
`.animnotify` HIT에도 없다. 답은 게임 툴팁 원문에 있었다.

```text
tip.desc.skill_17240
  차지 실패 시  <$MACRO physic_ch @1:172702 @2:104 />
  오버차지 시  <$MACRO physic_ch @1:172701 @2:104 />
```

**`1724xx`가 아니라 `1727xx`를 참조한다.** 두 행 모두 `ValueA=392`이고, 인벤 툴팁의 실제 수치로
검증하면 상수가 맞아떨어진다.

```text
23,386 / 392 = 59.658          차지 실패
46,771 / 59.658 = 784 = 392 × 2   오버차지
```

오버차지 2배는 테이블 값이 아니라 차지 단계가 런타임에 곱하는 구조다(버스트 캐넌도
`172000` 2325 / `172010` 4649로 정확히 2배). 우리 HOLD 모델은 완주 시 END 단계에서 데미지를 한 번
적용하므로 완주 = 오버차지로 보고 **784**를 `OFFICIAL_SCALED`(transform `ValueA x overcharge 2`)로
넣었다.

**`skillId × 10` 규약이 항상 성립하지는 않는다. 효과 행이 비어 있으면 툴팁 매크로를 먼저 본다.**

### 3.3 mode= 는 추론값이라 인게임이 우선한다

`.clipseq`의 `mode=`는 원본에 없고 클립 이름 패턴으로 분류한 값이다(07-31 추출 문서 §10.2).
사용자가 인게임에서 확인한 결과 T 풀배럴 캐넌이 차지, V 가디언의 수호가 일반이었고, 이는
clipseq의 COMBO/HOLD 표기와 반대다. **추론값보다 실제 게임 동작을 채택했다.**

## 4. 걸린 것

**클래스 파서가 여섯 군데에 흩어져 있다.** `ActorCatalog`, `AnimationSkillBindingDocument`,
`CombatHUDViewModel`, `PlayerSkillCatalog`, `GameplayCatalog`, `BalanceTool`. 하나씩 하네스가
잡아줘서 조용히 넘어간 것은 없지만, 빌드를 네 번 돌려야 전부 나왔다.

**앞선 창술사 커밋 `237d21c`가 Server contract test를 깨뜨린 채였다.** 그때 `Validate`와
`ProjectAudit`만 돌리고 전체 빌드를 돌리지 않아 못 잡았다. 제거한 7개 스킬이 `QUICK_SKILLS`에
남아 있었고 슬롯이 바뀐 4개도 옛 키를 가리켰다. 근접 사거리 테스트가 쓰던 34080도 사라져
34090으로 바꾸고 기대 데미지 261 → 1050을 함께 고쳤다.
**데이터만 바뀌어도 전체 회귀를 돌려야 한다.**

**`NoDefaultCurrentDirectoryInExePath=1`이 정본 자동화를 막는다.** `Invoke-BuildAndRegression.ps1`이
`& cmd /c "UpdateLib.bat $Configuration"`을 접두사 없이 호출하는데, 이 환경 변수 때문에 cmd가
현재 디렉터리를 탐색하지 않아 `'UpdateLib.bat' is not recognized`가 난다. PATH 문제가 아니다.

```powershell
powershell -ExecutionPolicy Bypass -Command "$env:NoDefaultCurrentDirectoryInExePath=''; & 'Tools\Build\Invoke-BuildAndRegression.ps1' -Configuration Debug"
```

## 5. 산출물

```text
Client/Bin/Resources/Character/Warlord/          126 MB  바디 + 파츠 6 + textures 34
Client/Bin/Resources/Character/WP_WWGL_04/       1.5 MB  총창
Client/Bin/Resources/Character/WP_WWGL_SD_04/    1.5 MB  방패
Data/Animation/Reference/Warlord/   clipmap 114 / clipseq 91체인 / animnotify 2,056행 / skilltiming 64
Data/Animation/Authored/Warlord/    animevents 1,696 / skillbindings 16
```

바디 서브메시 순서는 `0 lower / 1 upper / 2 base_upper / 3 arm / 4 face / 5 eye`이고 갑옷 마스크는
`0|1|2|3`이다. **클래스마다 다르므로 복사하지 않고 매번 쿠킹된 모델에서 읽는다.**

## 6. 검증

```text
umodel 추출         5개 패키지 exit 0
psa 임포트          193 시퀀스, 경고 0, 클립 이름 충돌 0
쿠킹                9개 exit 0, 재질 경고 0
validate_wmodel     런타임 폴더에서 9개 전부 OK
팔레트 공유         6파츠 bindDiff=0 maxDelta=0.00e+000
텍스처              얼굴 99.8%→0%, 눈 91.4%→0%, 나머지 이미 24bpp 0%
바인딩              클립 33개 전부 모델에 존재, COMBO/HOLD 클립 수 계약 일치
```

```text
Engine / Shared / NetworkProtocolHarness / Server / Client   빌드 성공
NetworkProtocolHarness        failures : 0
ClientFrontendHarness         failures : 0
Server.exe --contract-test    failures : 0
Effect Tool bundle            PASS
Gameplay balance Publish      6 profiles / 113 skills / 73 damage profiles
git diff --check              통과
```

`ProjectAudit`의 `projects.data-source-visibility`는 224 vs 222로 실패한다. 워로드 `Data` 6개는
등록했고, 남은 2개는 기존 미등록분이다 — `Data/UI/SkillWindow/LanceMaster.skillroster.json`,
`Data/UI/SkillWindow/SkillWindow_Layout.json`. 내 작업과 무관해 건드리지 않았다.

**인게임 확인(사용자): Lobby → Character Select에서 워로드가 정상 렌더된다.**

## 7. 남은 것

- 무기 소켓은 총창 `b_weapon_rhand` / 방패 `b_weapon_lhand`로 잡았다. `b_weapon_hold_01`은
  쓰지 않았다.
- HOLD 3클립을 `eternalcyclone_01 / _02 / _07`로 잡았다(01 개시, 02 연사 루프, 07 마무리).
  실제 차지 연출과 맞는지 확인이 남았다.
- 루트 모션은 아직 굽지 않았다. `Data/Animation/RootMotion/Warlord.rootmotion.json`이 없어
  모든 스킬이 제자리에서 재생된다. 돌격(17020)과 방패 돌진 계열은 이동이 필요하다.
- `clipseq`의 `mode=` COMBO 표기가 실제와 다른 스킬이 더 있다(사용자 확인). 디테일 단계에서
  같이 정리한다.
- 전장의 방패(17820)는 8클립 15.4초짜리 단일 SEQUENCE다. 실제 지속 시간과 맞는지 봐야 한다.
