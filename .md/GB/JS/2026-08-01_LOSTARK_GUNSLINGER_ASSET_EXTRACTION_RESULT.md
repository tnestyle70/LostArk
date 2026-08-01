# LostArk 건슬링어(Gunslinger) 리소스 추출 결과

작성자: JS · 2026-08-01

창술사와 같은 경로로 건슬링어 캐릭터 메시·스켈레톤·애니메이션·텍스처를 추출해
`.wmodel`까지 만들었다. 창술사 문서
(`2026-07-29_LOSTARK_LANCEMASTER_ASSET_EXTRACTION_RESULT.md`)와 **다른 점이 세 가지** 있고,
그게 이 문서의 핵심이다.

## 1. 클래스 코드는 `gdh_f`다 — DB로 확정

창술사 문서의 헌터 코드표(`gdh` `gbs` `gam` `gst`)에 건슬링어가 없다. 데빌헌터의
female 변종으로 들어가 있기 때문이다. `EFTable_PC.db` 실측:

```text
PK 512  Name='Devilhunter_Female'
  LookInfo   = EFDLChar_PC_GDH_F.PC_GDH_F
  BaseClass  = 511 Gunner_Female (EFDLChar_PC_GN_F)
  OriginalActionObjectGroupName = 'Gunslinger'
```

같은 조회로 나머지 헌터도 확정했다. `gam`=호크아이, `gst`=스카우터, `gbs`=블래스터.

DB 위치:
`C:\Users\95jus\Downloads\SourceData\SourceData\LPK\data2\EFGame_Extra\ClientData\TableData\`

시스템에 실제 Python이 없다(Store 스텁만 있음). SQLite 조회는 Blender 번들 Python을 쓴다.

```text
C:\Program Files\Blender Foundation\Blender 5.0\5.0\python\bin\python.exe
```

## 2. 베이스 바디가 얼굴·눈·머리를 이미 포함한다

`pc_gn_f_00_sk`의 재질 슬롯이 답이다.

```text
pc_dl_face_mi_high  pc_dl_eyeao_mi  pc_dl_eye_mi   ← 얼굴 · 눈
pc_ft_15_hair_mi                                   ← 머리
pc_gn_f_00_upper/arm/lower_mi                      ← 기본 복장
```

그래서 `PC_GN_F_00_FACE` / `PC_GN_F_00_HAIR` 패키지는 **존재하지 않는다**(직접 probe 확인).
`PC_GN_00_FACE` / `PC_GN_00_HAIR`는 남성 건너용이다(같은 네임스페이스에 남성 바디
`pc_gn_00_sk`가 있다). 별도 face/hair 메시를 합치면 안 된다.

결과적으로 `build_flm.py`가 겪었던 **얼굴·머리 중복 렌더 문제를 구조적으로 회피**한다.

### `PC_FT_*`는 여성 공용 템플릿이 아니다

창술사 문서의 "`ft`는 여성 캐릭터 기본 바디/얼굴 템플릿" 서술은 건슬링어에 적용되지
않는다. `ft`는 여성 무도가(Fighter, PK 301) 베이스이고 건너 리그와 **56본이 다른 별개
계열**이다. 실측:

| 대상 | 본 | `pc_gn_f_00_sk`(236)와 공유 | 미포함 |
|---|---:|---:|---:|
| `pc_ft_00_face_sk` | 207 | 151 | 56 |
| `pc_ft_00_hair_sk` | 221 | 155 | 66 |

## 3. 리그가 창술사보다 깨끗하다 — 재바인딩 불필요

창술사는 221본/207본이 섞여 마스터 아마추어 재바인딩이 필요했다. 건슬링어는 그렇지 않다.

| 대상 | 본 | 마스터(236) 대비 |
|---|---:|---|
| `pc_gn_f_00_sk` (바디+머리) | 236 | 마스터 |
| `pc_gdh_f_00_*` 방어구 7파츠 | 236 | 100% 일치, 누락 0 |
| `pc_gdh_f_00_ani.psa` | 232 | 완전 부분집합 |
| `pc_gn_f_00_ani.psa` | 232 | 완전 부분집합 |

스트레이 `b_root` 웨이트도 없다(전 파츠 실측). 창술사의 `repair_root_weights` 보정은
넣지 않았다.

## 4. 도구와 경로

```text
추출기   C:\Users\95jus\Downloads\umodel_win32\umodel_lostark_v7.exe
게임     C:\ProgramData\Smilegate\Games\LOSTARK\EFGame
스테이징 C:\Users\95jus\Downloads\umodel_win32\_export_gdh_f_psk
보정텍스처 C:\Users\95jus\Downloads\umodel_win32\_fixed_tex_gdh_f
FBX      C:\Users\95jus\Downloads\umodel_win32\_fbx_gdh_f
```

추출 커맨드는 창술사와 동일하다(`-game=lostark -kr -nameresolve` 두 플래그 필수).

```powershell
umodel_lostark_v7.exe `
  -path="C:\ProgramData\Smilegate\Games\LOSTARK\EFGame" `
  -game=lostark -kr -nameresolve `
  -export -psk -uncook -out="<staging>" `
  PC_GDH_F_00
```

추출한 패키지: `PC_GDH_F_00`, `PC_GN_F_00`, `PC_GN_00_FACE`, `PC_GN_00_HAIR`, `PC_GDH_F*`.
결과 psk 102 / psa 7 / tga 708.

### 애니메이션 범위

| psa | 시퀀스 | 크기 |
|---|---:|---:|
| `pc_gdh_f_00_ani` (클래스) | 187 | 84.8MB |
| `pc_gn_f_00_ani` (공용) | 1,068 | 931.8MB |
| `pc_gn_f_00_vehicle_ani` | 44 | 52.4MB |

창술사 선례대로 **클래스 187개만** 넣었다. 공용 1,068개는 결과물 크기가 감당되지 않는다.
로코모션이 필요하면 이름으로 골라 추가한다.

## 5. 텍스처 — 자동 매칭을 쓰면 안 된다

`--texture-root` 자동 매칭이 이 에셋에서 **세 종류로 틀린다.**

```text
pc_gdh_f_00_upper_mi   base=..._upper-1_cm.tga   ← _cm(컬러마스크)을 디퓨즈로 잡음
pc_gdh_f_00-3_helmet_mi base=textures/diffuse.tga ← 재질의 파라미터 이름을 파일로 잡음
전 파츠                 emissive=pc_gdh_f_03_helmet_e.tga ← 무관한 emissive 유출
```

그래서 `--no-auto-textures`로 끄고 Material Instance `.mat`의 명시 슬롯
(`Diffuse` / `Normal` / `Specular`)만 remap한다. `cook_gdh_f.py`가 자동화한다.

`.mat`의 `Normal=normal` 같은 값은 실제 텍스처가 아니라 셰이더 파라미터 이름이다.
단, `EFMASTER_MATERIAL_PROLOGUE`에 공용 `normal.tga`가 실재하므로 해석은 정상이다.

### 알파 컷아웃 — 창술사와 동일한 함정

셰이더가 `discard if a < 0.3`이므로 디퓨즈 알파를 불투명도로 쓰지 않는 텍스처는 보정해야 한다.

| 텍스처 | discard | 조치 |
|---|---:|---|
| `pc_dl_00_face_d.tga` | 99.6% | `Strip-TgaAlpha.ps1`로 24bpp화 → 0% |
| `pc_dl_00_eyeiris_d` + `pc_dl_eyebace_d` | 90.8% | `Bake-EyeTexture.ps1`로 합성 → 0% |
| `pc_ft_15_hair_d`, `pc_mg_av_030_hair1_d` | 45~54% | **건드리지 않음** (머리카락은 알파가 진짜 불투명도) |
| `pc_dl_eye_ao` | 68.3% | 건드리지 않음 (눈 AO 오버레이) |
| `*_s.tga` 전반 | 1~87% | 무관 (specular는 화면에 반영되지 않음) |

## 6. 빌드 파이프라인

```text
umodel (-psk)  ->  psk 102 + psa 7
      -> Blender 5.0 headless (io_scene_psk_psa)
         · build_gdh_f_part.py — FBX당 메시 1개, 마스터 아마추어 236본 고정
      -> FBX 8개
      -> cook_gdh_f.py (ModelAssetConverter, --no-auto-textures + 명시 remap)
      -> GunSlinger*.wmodel + textures/
```

스크립트는 `C:\Users\95jus\Desktop\buildScript\`에 있다(저장소 밖, Git 미추적).

```powershell
blender --background --factory-startup --python build_gdh_f_part.py -- `
  <PSK_ROOT> <target> <out.fbx> [--anim]
# target: body | upper | upper1 | lower | arm | shoulder | helmet | helmet1

<blender-python> cook_gdh_f.py <PSK_ROOT> <FBX_DIR> <OUT_DIR> <CONVERTER_EXE>
```

`--anim`은 `body`에만 준다. 방어구에 애니메이션을 넣으면 조각당 수십 MB가 된다.

## 7. 검증

| 항목 | 결과 |
|---|---|
| umodel 추출 | 5개 패키지 전부 exit 0 |
| psa 임포트 | 187 시퀀스 / 액션 187 / **경고 0** |
| 바디 FBX | 339.8MB, 메시 1개, 정점 10,762, 본 236 |
| 쿠킹 | 8개 전부 exit 0, 미해결 디퓨즈 슬롯 0 |
| `info` (바디) | `sections=190 animations=187 skeleton=yes`, `material-version=2 count=7` |
| 스켈레톤 대조 | 본 239, `[0..237]` 전부 일치 (`[238]`은 FBX 메시 노드명, 정상) |
| 인버스 바인드 | `bindDiff=0 maxDelta=0.00e+000` — 팔레트 공유 성립 |
| 텍스처 바인딩 | 전 파츠 `_d`/`_n`/`_s` 정상, 잘못된 emissive 제거됨 |

최종 산출물:

```text
Client/Bin/Resources/LostArk/Character/GunSlinger/
├─ GunSlinger.wmodel            96.66 MB   (바디+머리, 애니메이션 187)
├─ GunSlinger_Upper.wmodel       0.39 MB
├─ GunSlinger_Upper1.wmodel      0.49 MB   (-5 변형)
├─ GunSlinger_Lower.wmodel       0.35 MB
├─ GunSlinger_Arm.wmodel         0.45 MB
├─ GunSlinger_Shoulder.wmodel    0.55 MB
├─ GunSlinger_Helmet.wmodel      0.10 MB
├─ GunSlinger_Helmet1.wmodel     0.34 MB   (-3 변형, 헤어 포함)
└─ textures/                    39개, 60.3 MB
```

`Client/Bin/Resources/`는 `.gitignore` 대상이라 `git status`에 나타나지 않는다. 정상이며
팀 공유는 Drive 팩 경로 규칙을 따른다.

## 8. 무기

건슬링어는 권총·저격총·산탄총 3정을 쓴다. 어느 세트인지는 이름으로 고르지 않고 DB로 정했다.

```text
EFTable_PC     PK 512  TownDefaultWeapon = 1056002
EFTable_Item   PK 1056002  Model = EFDLItem_WP_WGDH_02.WP_WGDH_02
```

즉 기본 무기 패키지는 `WP_WGDH_00`이 아니라 **`WP_WGDH_02`**다. 접미사가 종류를 가른다.

| 오브젝트 | 무기 | 정점 |
|---|---|---:|
| `wp_wgdh_02h_sk` | 권총 (handgun) | 393 |
| `wp_wgdh_02l_sk` | 저격총 (long) | 1,114 |
| `wp_wgdh_02s_sk` | 산탄총 (shotgun) | 572 |

세 정 모두 재질이 `wp_wgdh_02_mi` 하나로 공유된다.

### 데빌헌터와 같은 메시를 쓴다

아이템 1052002(데빌헌터)와 1056002(건슬링어)가 **같은 `WP_WGDH_02`를 가리킨다.**
`wp_wgdh_f_*` 여성 전용 메시가 따로 존재하지만(`WP_WGDH_F_00` 등) 기본 무기는 그쪽이
아니다. 이름에 `_f`가 있다고 자동으로 건슬링어 것이라고 보면 안 된다.
`WP_WGDH_F_00`은 스테이징에 추출만 해두었다.

### 스켈레톤 없이 굽는다

무기 psk는 본이 `b_weapon_rhand` 하나뿐이다. 부착점 이름일 뿐 스키닝 의미가 없고,
창술사 `WP_WFLM_00L`도 `skeleton=no`로 들어가 있다. 그래서
`build_gdh_f_weapon.py`가 아마추어를 버리고 메시만 내보낸다.

```powershell
blender --background --factory-startup --python build_gdh_f_weapon.py -- `
  <psk> <out.fbx>
```

검증:

```text
WP_WGDH_02H/L/S   sections=2 animations=0 skeleton=no
  wp_wgdh_02_mi  base=wp_wgdh_02_d.tga  normal=_n  specular=_s  emissive=(없음)
  디퓨즈·노멀 24bpp, discard 0% — 알파 보정 불필요
```

산출물(창술사와 같은 무기별 폴더 규칙):

```text
Client/Bin/Resources/LostArk/Character/
├─ WP_WGDH_02H/   1.07 MB
├─ WP_WGDH_02L/   1.12 MB
└─ WP_WGDH_02S/   1.08 MB
```

## 9. 런타임 배선과 애니메이션 데이터

### 9.1 클래스 배선에서 걸린 두 가지

**클립 이름이 39자에서 잘려 충돌하면 모델 전체가 거부된다.** `MODEL_SECTION_DESC::name`이
40바이트라 `gdh_sk_super_largecaliberexplosivebullet_01`과 `_02`(각 43자)가 같은 이름이 되고,
`WModelDecoder`의 중복 클립 검사에 걸려 `CModel::Create`가 nullptr을 반환한다. 187개 중 2개
때문에 전체가 죽는다. `build_gdh_f_part.py`가 Blender 단계에서 충돌만 골라 개명한다.

**무기 소켓 본 이름이 창술사와 다르다.** gn_f 리그에는 `b_weapon_*`이 **없고**
`b_wp_1`(bip001-r-hand) / `b_wp_2`(bip001-l-hand)를 쓴다. 없는 본을 지정하면
`Get_BoneMatrix`가 조용히 항등행렬을 반환해 바디의 `0.0001` preTransform이 무기에 전달되지
않고 수천 배로 렌더된다. 에러가 없으므로 눈으로만 발견된다.

쌍권총이라 `CHARACTER_SPEC`의 단일 무기 필드를 `WEAPON_PART_SPEC` 배열로 바꿨다.
공용 헤더 변경이므로 창술사 스펙도 함께 갱신했다.

### 9.2 애니메이션 데이터 파일

```text
Client/Bin/DataFiles/Anim/
├─ GunSlinger.skilltiming   79 스킬 / 201 히트행       (DB)
├─ GunSlinger.clipmap      122 클립                    (.loa)
├─ GunSlinger.clipseq      115 체인 / 39 스킬 / 331 슬롯 (.loa)
├─ GunSlinger.animnotify   122 클립 / 3,445 노티파이행  (.loa)
└─ GunSlinger.animevents   2,322 이벤트                (.loa)
```

전부 UTF-8 · CRLF · BOM 없음.

> 처음에는 `.clipmap`/`.clipseq`를 손으로 짝지어 만들었다(94 클립, 25개 `[추정]`).
> 10장에서 `.loa` 파서가 생기면서 **전부 원본값으로 교체**했다. 손 매핑의 추정
> 5건 중 2건이 실제로 틀려 있었다.

`.skilltiming`은 `EFTable_Skill` + `EFTable_SkillEffect` + `EFTable_GameMsg`만으로도 기계적으로
나온다. **검증은 같은 스크립트로 `LanceMaster.skilltiming`을 재생성해 커밋본과 DIFF 0**을
확인하는 방식으로 했다. 그 과정에서 역산한 규칙:

| 규칙 | 내용 |
|---|---|
| 변형 선별 | timed 히트가 없는 변형 제외 (97 → 50) |
| 행 중복 | 데미지 티어만 다른 동일 행은 첫 것만 |
| 마커 행 | 시간·경직·넉백·범위가 모두 0인 행 제외 |
| 음수 시간 | `t=-100`도 유효, `w`는 0으로 클램프 |
| 집계 대상 | `freeze/push/multi/interval`은 timed 행만 |
| `maxt` | `ChainCombatEffectMaxTarget`이 아니라 **`MaxAmount`** |

클립 이름은 792개 DB 어디에도 없다(대소문자 무시 확인). 그래서 `.clipmap` / `.clipseq`를
처음에는 영문 클립 토큰과 한글 스킬명을 사람이 짝지어 만들었고, 확신이 낮은 5건에
`[추정]`을 붙였다. **10장에서 `.loa` 파서로 전부 교체했으니 이 서술은 과거형이다.**

### 9.3 스크립트

```text
buildScript/extract_skilltiming.py   DB → skilltiming. 클래스 무관
buildScript/extract_action_loa.py    .loa → clipmap/clipseq/animnotify/animevents (10장)
buildScript/validate_wmodel.py       엔진 디코더 검증 재현. cook_gdh_f.py가 자동 호출
```

`validate_wmodel.py`는 쿠킹 직후 `.wmodel`을 엔진과 같은 규칙으로 검사한다. 위 클립 이름
충돌이 런타임에서 원인 불명으로 죽던 것을 1초에 잡아준다.

### 9.4 `.animevents` / `.animnotify`의 정본은 `.loa` Action 파일이다

**이걸 못 찾아 상당 시간을 버렸다. 다음 사람은 여기부터 보면 된다.** 파서는 10장에 있다.

```text
C:\Users\95jus\Downloads\SourceData\SourceData\LPK
  data3\EFGame_Extra\ClientData\XmlData\Action\
    LANCEMASTER.loa      3.5 MB   창술사
    HUNTER_FEMALE.loa    4.9 MB   건슬링어
    HUNTER.loa           4.2 MB   데빌헌터
    (클래스별 non-NPC 914개)
```

**파일명은 에셋 코드가 아니라 `EFTable_PC.Name` 계열의 클래스 영문명이다.** 건슬링어를
`GDH`/`GUNSLINGER`로 찾으면 안 나온다. PK 512의 `'Hunter_Female'`이 파일명이다.

내부 구조가 `.animevents`의 `src=` 값과 그대로 대응한다.

```text
CEFActionObject
└─ CEFActionStage (StageName)
   ├─ CEFActionNotify_Anim                Anim="SK_TerminatingShot_01"  ← 클립 참조
   ├─ CEFActionNotify_InputTiming         → src=InputTiming        (CANCEL)
   ├─ CEFActionNotify_PlayParticleEffect  → src=PlayParticleEffect (EFFECT)
   ├─ CEFActionNotify_AKEvent             → SOUND
   ├─ CEFActionNotify_SuperArmor / _PVP   → SUPERARMOR
   └─ CEFActionNotify_Effect
```

HUNTER_FEMALE 실측 건수: PlayParticleEffect 4,575 / InputTiming 1,422 / AKEvent 822 /
Effect 619. 클립 토큰은 CamelCase다(`TerminatingShot` 57, `PerfectShot` 74, `GunKata` 55,
`Hallucination` 55, `TripleShot` 141).

**따라서 `GunKata`·`Hallucination`·`TripleShot`은 실재하는 스킬 액션이다.** 직렬화는
`int32 길이 + ASCII 문자열` 프리픽스 스트림이다.

`.db` 792개에는 클립 이름이 하나도 없다(대소문자 무시 확인). Action 데이터는 `.loa`에만
있으므로 DB 쪽을 아무리 뒤져도 안 나온다.

## 10. `.loa` 파서 — `buildScript/extract_action_loa.py`

9.4의 파일을 실제로 읽는 스크립트를 만들었다. 클래스 무관이고 네 파일을 한 번에 낸다.

```powershell
<blender-python> extract_action_loa.py <CLASS.loa> <wmodel> <AssetName> <clipPrefix> <outDir>
```

창술사 쪽 일회성 스크립트 다섯 개(`notify_extract` `loa_map2` `seq_probe` `make_notify`
`make_events` `make_clipseq`)가 하던 일을 하나로 합쳤다. 그것들은 **클립 토큰을 서로 다른
세 가지 방법으로 뽑고 있었고**, 그래서 결과가 파일마다 어긋나 있었다(10.3 참고).

### 10.1 검증 — DIFF 0

`.skilltiming` 때와 같은 방식. `LANCEMASTER.loa`로 커밋본을 재현했다.

```text
LanceMaster.animevents   3,050행   DIFF 0
LanceMaster.animnotify   4,432행   DIFF 0
LanceMaster.clipmap        143행   DIFF 0
```

`.animevents`가 `.animnotify`에서 파생되므로 위 셋이 맞으면 노티파이 레코드 해석·스테이지
귀속·에셋명 추출·한글 레이블·중복 제거·반올림이 전부 원본과 같다는 뜻이다.

### 10.2 실측으로 고친 것 셋

**클립 참조는 순수 식별자가 아니다.** `SK_Death'sEye_01`, `SK_Rolling_02-H`처럼 `'`와 `-`가
들어간다. 기존 스크립트의 `[A-Za-z][A-Za-z0-9_]*`는 여기서 토큰을 끊어 조용히 버렸다.
두 글자를 **영숫자 사이에서만** 허용하도록 넓혔다(플로트 바이트가 토큰을 늘리지 못하게).

```text
창술사   클립 142 → 144   (talonstrike_custom_4_03-1/-2)
건슬링어 클립 117 → 122   (death'seye 3개 = 초각성 불스 아이, rolling 2개 = 덤블링)
```

**쿠킹된 클립 이름은 39자에서 잘린다.** `MODEL_SECTION_DESC::name`이 40바이트라
`SK_Super_LargeCaliberExplosiveBullet_01/_02`가 같은 이름이 되고, `build_gdh_f_part.py`가
뒤엣것에 `~1`을 붙여 피한다. 파서가 `.loa`의 원본 이름을 같은 규칙으로 잘라 되맞춘다.

**그룹 번호는 0에서 시작하지도 연속이지도 않는다.** 헤더 16바이트를 값이 아니라 모양으로만
판정한다(비헤더는 count 자리에 플로트가 들어와 범위 검사에서 떨어진다).

### 10.3 옛 `.clipseq`에 빠져 있던 것

창술사 `.clipseq`는 `.clipmap`과 다른 방법으로 클립을 뽑고 있었다. 통합하니 세 스킬이
드러났다 — **탄영(34020) · 기상기(34030) · 돌파(34520).** 셋 다 `.clipmap`과
`.animnotify`에는 원래 있었는데 `.clipseq`에만 없었다. 이동기·기상기라 런타임에서
`Play_Skill`이 계속 거부하고 있었을 것이다.

```text
LanceMaster.clipseq   107 → 110 체인 (+3 스킬), 슬롯 245 → 247
```

`.clipmap` / `.animnotify` / `.animevents`도 10.2의 `-`·`'` 수정만큼 늘었다.
**전부 추가이고 삭제·변경 행은 0이다.**

### 10.4 건슬링어 결과

```text
GunSlinger.clipmap     122 클립
GunSlinger.clipseq     115 체인 / 39 스킬 / 331 슬롯
GunSlinger.animnotify  122 클립 / 3,445행
  EFFECT 2,184  CANCEL 546  SUPERARMOR 184  SOUND 372  SHAKE 119  HIT 40
GunSlinger.animevents  2,322 이벤트 (중복 1,190건 제거)
```

손 매핑이 실제로 틀렸던 곳:

| 클립 토큰 | 손 매핑 | `.loa` 정답 |
|---|---|---|
| `deadhard` | 38320 `[추정]` | **38190** 핸드건 - 레인오브불릿 |
| `explosivebuckshot` | 38190 `[추정]` | **38280** 각성 - 폭발 샷건 |
| `tripleshot` | 미매핑 | **38170** 샷건 - 샷건연사 |
| `hallucination` | 미매핑 | **38020** 핸드건 - 퀵스텝 |
| `gunkata` | 38260 `[추정]` | 38260 (맞음) 피스키퍼 |
| `death'seye` | 38310 `[추정]` | 38310 (맞음) 초각성 - 불스 아이 |

건슬링어 스킬명은 `핸드건 -` / `라이플 -` / `샷건 -` 접두로 **무기 스탠스를 구분**한다.
창술사의 `긴 창_` / `짧은 창_`과 같은 자리다. DB에는 없는 정보다.

`gdh_sk_quickshot`, `gdh_sk_deadhard_bw/fw/lw/rw`, `gdh_sk_rolling_0*-h/-l/-s`는
`HUNTER_FEMALE.loa`가 참조하지 않는다. 모델에는 있으나 액션이 없는 클립이다.
반대로 `Idle_Normal_1` / `Att_Normal_1_*` / `Act_Jump_Lope_1` 5개는 공용 애니 패키지
소속이라 모델에 없다 — 양쪽 클래스 모두 같은 5개이고 정상이다.

## 11. 툴·스크립트 정리

### 11.1 Animation_Tool이 캐릭터를 따라간다

`m_AssetName`이 `"LanceMaster"`로 하드코딩돼 있었다. TEST_LEVEL2는 이미
`Spec_GunSlinger`를 띄우고 있었으므로 **툴이 창술사 파일을 건슬링어 모델에 얹고 있었다.**

`Resolve_Model()`과 같은 패턴으로 `Resolve_Character()`를 넣고, `Sync_AssetName()`이
`CHARACTER_SPEC::pAssetName`을 읽어 경로를 정한다. 에셋이 바뀌면 로드된 이벤트·스킬참조·
clipmap·notify·clipseq를 전부 버리고 다시 읽는다(두 클래스 클립이 한 리스트에 섞이지
않게). 기본값은 빈 문자열이고, 스펙이 이름을 안 주면 패널이 그 사실을 표시한다.

### 11.2 `validate_wmodel.py` 승격

scratchpad에 있던 것을 `buildScript/`로 옮기고 `check(path, verbose=False)`를 임포트할 수
있게 했다. `cook_gdh_f.py`가 **파트마다 쿠킹 직후 호출**하고, 디코더가 거부할 패키지면
실패로 처리한다. 컨버터는 exit 0을 내고도 디코더가 거부하는 출력을 만들 수 있는데
(클립 이름 충돌이 그 경우다) 런타임에서는 `CModel::Create`가 이유 없이 nullptr을 낼 뿐이다.

### 11.3 지운 것

- `buildScript/extract_clipmap_gdh_f.py` — 손 매핑표가 파일 안에 박혀 있던 스크립트.
  `extract_action_loa.py`가 완전히 대체했고, 남겨두면 틀린 매핑을 다시 쓰게 된다.
- `dev/null/` — LFS 훅 4개가 복사돼 있던 흔적. `core.hookspath`는 unset이고
  `.git/hooks/`에 같은 파일이 이미 있다(`cmp` 동일 확인). 추적되지 않는 사고 산물이다.

## 12. 검증

```text
Client x64 Debug              오류 0 / 내 파일발 경고 0
                              (C4819는 MapAssetObject.h 등 기존 것, LNK4099는 DirectXTK)
LanceMaster 재현              animevents / animnotify / clipmap  DIFF 0
C++ 리더 재현 (파이썬)         두 클래스 8파일 전부 선언 개수 == 파싱 개수, 건너뛴 줄 0
  animevents  3,086 / 2,322   Load_Events
  animnotify  144 / 122 클립   Load_ClipNotify
  clipmap     144 / 122        Load_ClipMap
  clipseq     110 / 115 체인   Load_ClipSeq + CCharacter::Load_ClipChains
체인 클립 존재 여부            247 / 331 슬롯 전부 모델에 있음 (누락 0)
최대 줄 길이                   269바이트 < fgets 버퍼 1024·2048
인코딩                        UTF-8 / CRLF / BOM 없음
validate_wmodel               GunSlinger 본체·방어구·무기 전부 OK
```

**실행 확인은 아직 안 했다.** F1로 툴을 열어 Asset이 `GunSlinger`로 뜨는지, 클립을
고르면 한글 스킬명과 Chain이 붙는지, Import original이 도는지는 봐야 한다.

## 13. 남은 것

- **AssetTest 레벨 실제 렌더 확인을 아직 하지 않았다.** 쿠킹 검증까지만 끝난 상태다.
- 무기를 손에 붙이는 소켓 연결(`CPartObject` / `CWeapon` 경로)은 하지 않았다.
- `CLogic_GunSlinger::Update`가 아직 비어 있다. 체인은 준비됐으니 키 바인딩만 정하면
  된다(창술사는 `Logic_LanceMaster.cpp`의 `Binds[]`가 예시다).
- 애니메이션 이름은 FBX take 규칙상 `gdh_<액션명>` 형태다(아마추어명 `gdh`).
- 복장 `PC_GDH_F_01~`, 아바타 `PC_GDH_F_AV_*`, 헤어 `PC_GDH_F_HR_*`는 스테이징에만 있고
  조립에 넣지 않았다.
- 원본은 Smilegate 저작물이다. 스테이징과 결과물 모두 Git에 올리지 않는다.
