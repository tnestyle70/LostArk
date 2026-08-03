# LostArk 도화가(Artist) 리소스 추출 결과

작성자: JS · 2026-08-01

건슬링어와 같은 경로로 도화가를 뽑았다. 선행 문서
(`2026-08-01_LOSTARK_GUNSLINGER_ASSET_EXTRACTION_RESULT.md`)의 파이프라인이 거의 그대로
돌았고, **새로 배운 것은 두 가지**다 — 클래스 좌표를 DB로 찾는 순서와, 얼굴·머리 판정
기준이 "패키지 존재 여부"가 아니라는 것.

## 1. 클래스 좌표 — 전부 DB로 확정

이름으로 추측하면 안 된다. Specialist 계열이 여섯이라 특히 그렇다.

```text
EFTable_PC  PK 602  Name='YinYangShi'
  LookInfo   = EFDLChar_PC_SDM.PC_SDM     → 에셋 코드 sdm
  BaseClass  = 601 Specialist (PC_SP)     → 베이스 바디 pc_sp_00_sk
  GenderType = 1 (female)
  TownDefaultWeapon = 1061002
```

`YinYangShi`가 도화가라는 근거는 `EFTable_GameMsg`다.

```text
tip.name.enum_playerclass_yinyangshi     도화가
tip.name.enum_playerclass_weather_artist 기상술사   (PK 603)
tip.name.enum_playerclass_alchemist      환수사     (PK 604)
```

스킬 ID 블록은 `Skill.LearnClass`로 확정한다. **PK와 블록 번호는 다르다.**

| 클래스 | PK | 스킬 블록 |
|---|---:|---:|
| LanceMaster | 305 | 34xxx |
| Devilhunter_Female | 512 | 38xxx |
| **YinYangShi** | **602** | **31xxx** (78개) |

스킬명이 "필법 : 흘리기", "묵법 : 파죽"처럼 먹·붓 계열이라 눈으로도 맞다.

Action 파일은 `XmlData/Action/YINYANGSHI.loa`. 건슬링어 때와 달리 이번엔 파일명이
`Name`과 그대로 일치했다(건슬링어는 `Devilhunter_Female` → `HUNTER_FEMALE.loa`였다).
**규칙이 아니라 우연이므로 매번 실제 파일 목록을 확인할 것.**

## 2. 얼굴·머리 판정 — 패키지 존재 여부는 기준이 아니다

**이번에 하마터면 틀릴 뻔한 지점이다.**

`PC_SP_00_FACE`(`pc_sp_00_face_sk`)와 `PC_SP_00_HAIR`(`pc_sp_00_hair_sk`)가 **둘 다
실재한다.** 건슬링어는 `PC_GN_F_00_FACE`가 아예 없어서 자연히 걸러졌으므로, 패키지가
있으면 창술사처럼 따로 합쳐야 한다고 판단하기 쉽다. **틀렸다.**

베이스 바디의 재질 슬롯이 답이다.

```text
pc_sp_00_sk.psk   슬롯 10개
  pc_sp_face_mi_high  pc_sp_eyeao_mi  pc_sp_eye_mi   ← 얼굴 · 눈
  pc_sp_06_hair_mi                                   ← 머리
  pc_sp_01_upper/arm/lower_mi + pc_sp_av_base_body_mi
```

바디가 이미 머리를 그린다. face/hair를 합치면 창술사가 겪은 중복 렌더가 그대로 난다.

> **판정 기준: 베이스 바디 psk의 MATT0000 슬롯에 face/eye/hair 재질이 있는가.**
> 패키지가 있느냐 없느냐가 아니다. `build_character_part.py` docstring에 박아뒀다.

## 3. 리그 — 재바인딩 불필요

| 대상 | 본 | 마스터(236) 대비 |
|---|---:|---|
| `pc_sp_00_sk` (바디+머리) | 236 | 마스터 |
| `pc_sp_00_face_sk` / `_hair_sk` | 236 | 100% 일치 (쓰지 않음) |
| `pc_sdm_00_*` 방어구 7파츠 | 236 | 100% 일치, 누락 0 |
| `wp_wsdm_09_sk` (무기) | 6 | 별개 (`b_root` + `b_body_01~05`) |

무기 소켓은 리그에서 읽었다. `b_weapon_*`은 **없고** gn_f와 같은 계열이다.

```text
b_wp_1  parent=bip001-r-hand    ← 붓을 잡는 손
b_wp_2  parent=bip001-l-hand
b_wp_3  parent=bip001-spine2
```

## 4. 애니메이션 — psa가 셋인데 하나만 쓴다

```text
pc_sdm_00_ani.psa      99 시퀀스  236본   52.4MB   ← 사용
pc_sdm_00_ani_bk.psa   86 시퀀스  236본   43.7MB   완전 부분집합
pc_sdm_00_ani_bk_2.psa 77 시퀀스   74본   12.0MB   로코모션 3개만 추가
pc_sp_00_ani.psa    1,056 시퀀스  236본  1002MB   공용, 제외
```

`_bk_2`가 `sk_inkpaddle`·`sk_skykongkong_*`처럼 메인에 없는 스킬 클립을 갖고 있어
빠뜨리는 줄 알았는데, **`YINYANGSHI.loa`가 그것들을 참조하지 않는다.** 죽은 클립이다.

`.loa` 클립 참조 112개 중 87개가 모델에서 해석된다. 나머지 25개는 **전부 공용
로코모션**(Idle / Run / Walk / Jump / Fall / AirBorne / Drown 등)이고 19개는 제외한
`pc_sp_00_ani.psa`에 있다. **스킬 클립은 하나도 빠지지 않았다.**

## 5. 텍스처 — 얼굴과 눈은 같은 함정

셰이더가 `discard if a < 0.3`이라 알파를 불투명도로 쓰지 않는 디퓨즈는 보정해야 한다.

| 텍스처 | discard | 조치 |
|---|---:|---|
| `pc_sp_00_face_d` | **99.5%** | `Strip-TgaAlpha.ps1` → 24bpp → 0% |
| `pc_sp_eyebace_00_d` | **100.0%** | 단독으론 전부 사라짐. `Bake-EyeTexture.ps1`로 iris와 합성 → 0% |
| `pc_sp_eyeiris_00_d` | 84.8% | 위 합성의 재료 |
| `pc_sp_eye_ao` | 73.5% | 건드리지 않음 (AO 오버레이) |
| `pc_sp_06_hair_d` | 49.4% | 건드리지 않음 (머리카락은 알파가 진짜 불투명도) |
| `pc_sdm_00_lower_d_loc_int` | **27.0%** | 건드리지 않음 — **육안 확인 필요** |
| `pc_sdm_00_helmet_d` | 18.2% | 건드리지 않음 |
| 나머지 방어구·무기 | 0~8% | 정상 |

하의 27%는 전례(방어구 대부분 한 자릿수)보다 높다. 치맛단 시스루 표현이면 정상이고
아니면 보정 대상이다. **렌더를 봐야 판단할 수 있어 그대로 뒀다.**

## 6. 스크립트 — 세 번째 복사본을 만들지 않았다

`build_gdh_f_part.py`는 `build_flm_part.py`의 거의 복사본이었다. 도화가용으로 세 번째를
만드는 대신 클래스 설정만 테이블로 뽑아 합쳤다.

```text
buildScript/build_character_part.py   CLASSES = { gdh_f, sdm }   ← 신규, 통합
buildScript/cook_character.py         CLASSES = { gdh_f, sdm }   ← 신규, 통합
buildScript/build_weapon.py           클래스 무관 (구 build_gdh_f_weapon.py)
buildScript/build_flm_part.py         창술사 전용으로 유지
```

창술사만 남긴 이유는 리그가 실제로 다르기 때문이다(221/207 혼재 + 마스터 재바인딩 +
스트레이 `b_root` 보정). 억지로 합치면 설정이 아니라 분기가 늘어난다.

**통합 검증**: 같은 타깃(`gdh_f helmet`)을 구/신 스크립트로 각각 빌드해 로그 전 항목
일치, FBX **464,060바이트로 크기 동일**. 확인 후 `build_gdh_f_part.py`·
`cook_gdh_f.py`를 삭제했다.

```powershell
blender --background --factory-startup --python build_character_part.py -- `
  sdm <PSK_ROOT> <target> <out.fbx> [--anim]
# target: body | upper | lower | lower1 | arm | shoulder | helmet | helmet1

<blender-python> cook_character.py sdm <PSK_ROOT> <FBX_DIR> <OUT_DIR> <CONVERTER_EXE>
```

## 7. 산출물

```text
Client/Bin/Resources/LostArk/Character/Artist/
├─ Artist.wmodel            56.5 MB   (바디+머리, 애니메이션 99)
├─ Artist_Upper.wmodel       0.59 MB
├─ Artist_Lower.wmodel       0.48 MB
├─ Artist_Lower1.wmodel      0.41 MB   (변형, 아래 주의)
├─ Artist_Arm.wmodel         0.46 MB
├─ Artist_Shoulder.wmodel    0.55 MB
├─ Artist_Helmet.wmodel      0.49 MB
├─ Artist_Helmet1.wmodel     0.21 MB   (변형)
└─ textures/                41개, 81 MB

Client/Bin/Resources/LostArk/Character/WP_WSDM_09/   5.7 MB
```

```text
Client/Bin/DataFiles/Anim/
├─ Artist.skilltiming   27 스킬 / 94 히트행        (DB)
├─ Artist.clipmap       67 클립                     (.loa)
├─ Artist.clipseq       69 체인 / 42 스킬 / 127 슬롯 (.loa)
├─ Artist.animnotify    67 클립 / 1,356행           (.loa)
└─ Artist.animevents    1,016 이벤트                (.loa)
```

**`Artist_Lower1`에 이름 없는 재질 슬롯이 하나 있다.** `pc_sdm_00_lower1_sk_loc_int`의
두 번째 슬롯이 `material_1`(`.mat` 없음)이고, `_usa` 컷에서는 같은 자리가
`pc_sdm_00-4_lower_mi_usa`다. 즉 국제판 컷이 그 조각을 비워둔 것이고, 해당 서브메시는
**22정점**뿐이다. `CMaterial`의 1×1 회색 폴백으로 렌더된다. 조립에는 `Lower`를 쓰므로
현재 영향은 없지만 `Lower1`로 갈아끼우면 보인다.

## 8. 무기

```text
EFTable_PC   PK 602      TownDefaultWeapon = 1061002
EFTable_Item PK 1061002  Model = EFDLItem_WP_WSDM_09-1.WP_WSDM_09-1
```

**`WP_WSDM_09-1`은 패키지로 존재하지 않는다**(umodel: `unable to find package`).
실제 메시는 `WP_WSDM_09`의 `wp_wsdm_09_sk` 하나이고 `-1`~`-9`는 머티리얼 변형이다.
전 변형이 `wp_wsdm_09_d/_n/_s`로 **같은 텍스처**를 가리키므로 psk가 들고 있는 `-3`
슬롯을 그대로 써도 결과가 같다.

스케일은 주지 않았다(건슬링어와 같은 판단). 붓 길이 100.4 대 바디 높이 102.6으로 이미
바디 단위이고, 소켓이 바디의 preTransform을 실어준다. 창술사 창이 x100을 받는 건 그것만
다른 소스 단위(2.3)에서 나왔기 때문이다.

## 9. 런타임 배선

```text
Client/Public/Logic_Artist.h     Spec_Artist 선언
Client/Private/Logic_Artist.cpp  스펙 상수 + 빈 로직
Client/Private/Loader.cpp        Ready_Artist_Prototypes
Client/Private/Level_Test2.cpp   Spec_Artist 스폰
```

한 번에 한 클래스만 등록한다(디버그 힙 문제). `CLoader`의 호출과 `CLevel_Test2`의 스펙을
**같이** 바꿔야 한다. 지금은 창술사·건슬링어가 주석 처리돼 있고 도화가가 활성이다.

바디 서브메시 순서는 `0 face / 1 eye_ao / 2 eye / 3 hair / 4 upper / 5 base_body /
6 arm / 7 lower`이고 마스크는 `4|5|6|7`이다. **건슬링어와 다른 점**: 이 바디는 기본 복장
외에 맨살 서브메시(`pc_sp_av_base_body_mi`)를 따로 갖고 있고, 방어구 파츠도 각자 같은
재질로 노출 피부를 그린다. 둘 다 두면 z-fighting이라 5도 같이 숨겼다. **방어구가 못 덮는
틈이 생기면 되돌릴 후보는 5번이다.**

## 10. 검증

```text
umodel 추출        5개 패키지 전부 exit 0 (psk 20 / psa 7 / tga 325)
본 대조            바디·방어구 7파츠 전부 236본, 불일치 0
psa 임포트         99 시퀀스 / 액션 99 / 경고 0 / 클립 이름 충돌 0
쿠킹               9개 전부 exit 0
validate_wmodel    9개 전부 OK (디코더 거부 조건 재현)
인버스 바인드      bindDiff=0 maxDelta=0.00e+000 — 팔레트 공유 성립
                   (nameDiff=1은 FBX 메시 노드명, 전례와 동일)
C++ 리더 재현      Artist 4파일 선언 개수 == 파싱 개수, 건너뛴 줄 0
                   체인 클립 127슬롯 전부 모델에 존재
최대 줄 길이       258바이트 < fgets 버퍼 1024·2048
인코딩             데이터 UTF-8/CRLF/BOM 없음, 소스는 형제 파일과 같은 ASCII
Client x64 Debug   오류 0 / 내 파일발 경고 0
```

`Loader.cpp`의 C4819는 HEAD에도 있던 기존 경고다(비ASCII 408바이트로 동일).

## 11. 렌더 확인 — 통과

TEST_LEVEL2에서 실제로 띄워 정상 렌더를 확인했다(작성자 육안). 이걸로 열어두었던
판단 두 개가 닫힌다.

- **5장의 하의 알파 27%는 그대로 두는 게 맞다.** 보정 대상이 아니었다.
- **9장의 서브메시 마스크 `4|5|6|7`이 맞다.** 맨살(5번)까지 숨겨도 방어구가 다 덮고,
  되돌릴 필요가 없다.

건슬링어·창술사와 달리 이 클래스는 쿠킹 검증에서 끝나지 않고 화면까지 확인된 상태다.

## 12. 남은 것

- `CLogic_Artist::Update`가 비어 있다. 체인 69개는 준비됐고 키 바인딩만 정하면 된다.
- 무기 소켓(`b_wp_1`)은 스펙에 적혀 있고 렌더도 확인됐지만, 스킬 모션 중 붓이 손을
  정확히 따라가는지는 애니메이션을 돌려봐야 안다.
- 소환수 액션 `SK_SDM_CAR_00.loa` / `SK_SDM_TIG_00.loa`가 따로 있다. 도화가 소환체용
  이며 클립은 다른 패키지 소속이라 이번 조립에 넣지 않았다.
- 복장 `PC_SDM_01~`, 헤어 `PC_SP_*_HAIR`는 스테이징에만 있고 조립에 넣지 않았다.
- 원본은 Smilegate 저작물이다. 스테이징과 결과물 모두 Git에 올리지 않는다.
