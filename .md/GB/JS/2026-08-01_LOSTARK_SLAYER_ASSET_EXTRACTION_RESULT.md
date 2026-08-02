# LostArk 슬레이어(Slayer) 리소스 추출 결과

작성자: JS · 2026-08-01

건슬링어·도화가와 같은 파이프라인으로 뽑은 세 번째 클래스다. 통합된
`build_character_part.py` / `cook_character.py`에 클래스 항목 하나씩 추가한 것이 전부이고,
**앞 두 클래스와 다른 점이 세 가지** 있다.

## 1. 클래스 좌표 — DB로 확정

```text
EFTable_PC  PK 112  Name='Berserker_Female'
  LookInfo   = EFDLChar_PC_WBK_F.PC_WBK_F   → 에셋 코드 wbk_f
  BaseClass  = 111 Warrior_Female (PC_WR_F)  → 베이스 바디 pc_wr_f_00_sk
  GenderType = 1 (female)
  TownDefaultWeapon = 1016002
```

근거는 `EFTable_GameMsg`의 `tip.name.enum_playerclass_berserker_female` = **슬레이어**.
전사 계열에 여성 행이 셋(`Warrior_Female` / `Berserker_Female` / `HolyKnight_Female`,
후자는 발키리)이라 확인이 필요했다.

스킬 블록은 `Skill.LearnClass`로 잡는다. **PK와 블록 번호는 다르다.**

| 클래스 | PK | 스킬 블록 |
|---|---:|---:|
| LanceMaster | 305 | 34xxx |
| Devilhunter_Female | 512 | 38xxx |
| YinYangShi | 602 | 31xxx |
| **Berserker_Female** | **112** | **45xxx** (38개) |

Action 파일은 `XmlData/Action/BERSERKER_FEMALE.loa` (4.4MB, 액션 51개).

## 2. 리그가 균일하지 않다 — 첫 사례이자 이번의 핵심

건슬링어·도화가는 바디와 방어구가 전부 같은 본 수였다. 슬레이어는 아니다.

| 대상 | 본 |
|---|---:|
| `pc_wr_f_00_sk` (바디), 헤어, 그림자 | **242** |
| 방어구 6파츠 전부 | **238** |

다만 창술사처럼 **어긋난 게 아니라 완전 부분집합**이다(`notInRef=0`). 238개 이름이
모두 바디 리그에 있으므로 이름 기반 재바인딩으로 풀린다. **가정하지 않고 실측했다:**

```text
Compare-InverseBind   6파츠 전부  bindDiff=0  maxDelta=0.00e+000
```

즉 쿠킹 후 모든 파츠가 동일한 245본 테이블(242 + FBX 노드 3)을 갖는다. 팔레트 공유 성립.
`nameDiff=1`은 FBX 메시 노드명으로 전례와 같다.

> 교훈: 본 수가 다르다고 바로 창술사식 재바인딩 계보로 갈 필요는 없다. **부분집합이면
> 이름 매칭으로 풀리고, 확인은 `Compare-InverseBind`로 한다.**

## 3. 무기 소켓이 창술사 계열이다

```text
wr_f 리그   b_weapon_rhand [156]   b_weapon_lhand [129]
gn_f / sp   b_wp_1 / b_wp_2 / b_wp_3
```

건슬링어·도화가에서 쓰던 `b_wp_*`가 **아예 없다.** 클래스마다 리그에서 읽어야 한다는
규칙이 그대로 증명된 사례다. 없는 이름을 지정하면 `Get_BoneMatrix`가 조용히
항등행렬을 반환해 무기가 수천 배로 렌더된다.

## 4. 얼굴·머리는 바디가 그린다 (도화가와 같은 결론)

`PC_WR_F_00`에 `pc_wr_f_22_hair`, `PC_WR_F_00_HAIR`에 `pc_wr_f_00_hair_sk`가 따로 있어
헷갈릴 수 있지만, 판정은 **베이스 바디 psk의 재질 슬롯**이다.

```text
pc_wr_f_00_sk.psk   슬롯 9개
  pc_wr_f_face_mi  pc_wr_f_eyeao_mi  pc_wr_f_eye_mi  pc_wr_f_01_hair_mi
  pc_wbk_f_00_upper/arm/lower_mi + pc_wr_f_basebody_upper_mi
```

바디가 머리를 이미 그리므로 face/hair 타깃 없음. 덤으로 **기본 복장이 이미 슬레이어
방어구**(`pc_wbk_f_00_*`)다.

`PC_WR_F_00_FACE`는 스켈레탈 메시가 0개다(빈 패키지).

## 5. 제외한 것들

- **`pc_wbk_f_00_high_sk`** — `-list`에는 뜨지만 `.pskx`로 나온다(정점 65535 초과 형식).
  **80,741정점** 고품질 전신 모델이고 재질이 `-6` 변형 + 헤어다. 컷신·캐릭터 선택용으로
  보여 쓰지 않았다. `find -name "*.psk"`로는 안 잡히니 주의.
- **`_chn` 컷** (`pc_wbk_f_00_upper_sk_chn`, `high_sk_chn`) — 지역 컷. 일반 컷 사용.
- **`wp_wwbk_03_sk_bk`** — 무기 패키지에 메시가 둘이다. 기본 `wp_wwbk_03_sk`는 본 1개
  (`b_weapon_r`)인데, `_bk`는 **11본 + 자체 애님셋 `wp_wwbk_03_ani.psa`(4시퀀스)**를 갖는다.
  아이템 테이블이 가리키는 건 기본 쪽이라 지금까지처럼 정적으로 구웠다.
  각성·변신 연출용으로 보이며 필요해지면 그때 붙인다.

## 6. 애니메이션

```text
pc_wbk_f_00_ani.psa      208 시퀀스  242본   96.6MB   ← 사용
pc_wbk_f_00_ani_bk.psa   197 시퀀스  242본   90.7MB   부분집합
pc_wr_f_00_ani.psa     1,052 시퀀스  242본 1003.1MB   공용, 제외
```

`.loa` 클립 참조 192개 중 169개 해석. 미해석 23개 중 clipseq에 걸린 4개는 전부 공용
로코모션(`Idle_Normal_1`, `Att_Normal_1_*`, `Act_Jump_Lope_1`)이다.
**클립 이름 39자 충돌 0** — 이번엔 개명이 필요 없었다.

## 7. 텍스처 — 세 클래스 중 가장 깨끗

| 텍스처 | discard | 조치 |
|---|---:|---|
| `pc_wr_f_00_face_d` | **99.6%** | `Strip-TgaAlpha.ps1` → 24bpp → 0% |
| `pc_dl_eyebace_d` | 0.0% | 이미 24bpp 불투명 (도화가는 100%였다) |
| `pc_dl_00_eyeiris_d` | 90.8% | 홍채가 알파 마스크. base와 합성 → 0% |
| `pc_dl_eye_ao` | 68.3% | 건드리지 않음 |
| `pc_dl_01_hair_d` | 47.6% | 건드리지 않음 (머리카락 알파는 진짜) |
| 방어구 5종 | 0.6~3.3% | 정상 |
| `wp_wwbk_03_d` | 0.0% | 24bpp |

눈 텍스처가 건슬링어와 **같은 공용 `pc_dl_*` 세트**다. umodel이 `-uncook`으로
`PC_DL_00_FACE`를 자동으로 끌어와 별도 추출이 필요 없었다.

## 8. 산출물

```text
Client/Bin/Resources/LostArk/Character/Slayer/
├─ Slayer.wmodel           103.41 MB   (바디+머리, 애니메이션 208)
├─ Slayer_Arm.wmodel         1.04 MB
├─ Slayer_Upper.wmodel       0.69 MB
├─ Slayer_Lower.wmodel       0.79 MB
├─ Slayer_Shoulder.wmodel    0.51 MB
├─ Slayer_Helmet.wmodel      0.37 MB
├─ Slayer_Helmet1.wmodel     0.73 MB
└─ textures/                32개, 53 MB

Client/Bin/Resources/LostArk/Character/WP_WWBK_03/   684 KB
```

```text
Client/Bin/DataFiles/Anim/
├─ Slayer.skilltiming   54 스킬 / 113 히트행         (DB)
├─ Slayer.clipmap      148 클립                       (.loa)
├─ Slayer.clipseq       99 체인 / 35 스킬 / 451 슬롯   (.loa)
├─ Slayer.animnotify   148 클립 / 2,913행             (.loa)
└─ Slayer.animevents   2,439 이벤트                   (.loa)
```

## 9. 런타임 배선

바디 서브메시 순서가 **도화가와 다르다.** 헤어가 0번이고 얼굴이 1번이다.

```text
0 hair / 1 face / 2 eye_ao / 3 eye / 4 upper / 5 base_body / 6 arm / 7 lower
마스크 = 4|5|6|7
```

도화가는 `0 face / 1 eye_ao / 2 eye / 3 hair / ...`였다. **마스크는 매번 쿠킹된 모델에서
읽어야 하고 다른 클래스에서 복사하면 안 된다.** 스펙 주석에 그렇게 적었다.

무기는 스케일 없이(`XMMatrixIdentity`) 붙인다. 다른 클래스와 같은 판단 기준(바디와 같은
단위)이지만 **지금까지 중 가장 긴 무기다 — 대검 209.1 대 바디 높이 129.6.** 비율이
1.61이라 화면에서 어색하면 여기부터 볼 것.

TEST_LEVEL2는 한 번에 한 클래스만 등록한다. 현재 슬레이어가 활성이고 나머지 셋은
주석 처리돼 있다. `CLoader`의 호출과 `CLevel_Test2`의 스펙을 **같이** 바꿔야 한다.

## 10. 검증

```text
umodel 추출        4개 패키지 전부 exit 0 (psk 24 / psa 6 / tga 117)
본 대조            바디 242 / 방어구 238, 부분집합 관계 (notInRef=0)
인버스 바인드      6파츠 전부 bindDiff=0 maxDelta=0.00e+000 — 팔레트 공유 성립
psa 임포트         208 시퀀스 / 액션 208 / 경고 0 / 클립 이름 충돌 0
쿠킹               8개 전부 exit 0, 재질 경고 0
validate_wmodel    8개 전부 OK
C++ 리더 재현      4파일 선언 개수 == 파싱 개수, 건너뛴 줄 0
                   체인 클립 451슬롯 전부 모델에 존재
최대 줄 길이       512바이트 < fgets 버퍼 1024·2048
인코딩             데이터 UTF-8/CRLF/BOM 없음, 소스는 형제 파일과 같은 ASCII
Client x64 Debug   오류 0 / 내 파일발 경고 0
```

## 11. 렌더 확인 — 통과

TEST_LEVEL2에서 실제로 띄워 정상 렌더를 확인했다(작성자 육안).

**9장에서 걱정한 대검 길이 비율 1.61은 문제가 아니었다.** 스케일 없이
(`XMMatrixIdentity`) 붙이는 게 맞고, 209.1이라는 수치는 슬레이어 대검이 원래 그만큼
크기 때문이다. 무기 스케일 판정 기준은 **비율이 아니라 바디와 같은 단위인지**라는 것이
세 클래스로 확인됐다(권총 0.24 / 붓 0.98 / 대검 1.61, 전부 스케일 없이 정상).

창술사 창만 x100이 필요한 건 그것 하나가 다른 소스 단위(2.3)에서 나왔기 때문이다.

## 12. 남은 것

- `CLogic_Slayer::Update`가 비어 있다. 체인 99개는 준비됐고 키 바인딩만 정하면 된다.
- `wp_wwbk_03_sk_bk`(애니메이션 있는 무기 변형)는 넣지 않았다.
- `high_sk` 고품질 모델, `_chn` 컷, 복장 `PC_WBK_F_01~`은 조립에 넣지 않았다.
- 원본은 Smilegate 저작물이다. 스테이징과 결과물 모두 Git에 올리지 않는다.
