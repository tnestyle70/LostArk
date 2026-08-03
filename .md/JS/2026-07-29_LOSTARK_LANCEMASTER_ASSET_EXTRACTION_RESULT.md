# LostArk 창술사(LanceMaster) 리소스 추출 결과

작성자: JS · 2026-07-29

원본 게임 클라이언트에서 창술사 캐릭터 메시·스켈레톤·애니메이션·텍스처를 추출해
`ModelAssetConverter`가 먹을 수 있는 형태까지 만드는 경로를 확정했다.

## 1. 도구와 경로

```text
추출기   C:\Users\95jus\Downloads\umodel_win32\umodel_lostark_v7.exe
게임     C:\ProgramData\Smilegate\Games\LOSTARK\EFGame
스테이징 C:\Users\95jus\Downloads\umodel_win32\_export_flm      (gltf + tga)
         C:\Users\95jus\Downloads\umodel_win32\_export_flm_psk  (psk + psa)
```

추출 원본은 저장소 밖 스테이징 폴더에 둔다. 최종 산출물만
`Client/Bin/Resources/LostArk/Character/LanceMaster/`로 들어가며 이 경로는
`.gitignore:64`로 이미 제외되어 있다(Drive 팩 대상).

## 2. umodel 플래그 조합 — 이게 핵심이다

`ReleasePC/Packages/`의 파일명은 전부 난독화되어 있다(`UB1MW291QZZEZ7DQ61SSPSC.upk`).
이름으로 패키지를 지정하려면 **두 플래그가 같이** 있어야 한다.

```text
-game=lostark -nameresolve
```

- 둘 다 있으면 패키지가 역난독화된 이름(`PC_FLM_00`)으로 등록되고 와일드카드가 먹는다.
  로그의 `Game:` 값이 `80004E`로 나온다.
- `-game=lostark`가 빠지면 `Game: 800000`(일반 UE3)으로 잡혀 LZO 복호화가 깨진다.
  이때는 난독화된 **파일명으로만** 지정할 수 있다.
- 한국 클라이언트는 `-kr`을 같이 준다.

확정 커맨드:

```powershell
umodel_lostark_v7.exe `
  -path="C:\ProgramData\Smilegate\Games\LOSTARK\EFGame" `
  -game=lostark -kr -nameresolve `
  -export -psk -uncook -out="<staging>" `
  PC_FLM_00
```

### `-notex`를 쓰면 안 되는 이유

텍스처를 이미 뽑았다고 `-notex`로 메시만 다시 뽑으면, psk의 `MATT0000` 청크에
실제 재질명 대신 `material_0`, `material_1` 플레이스홀더가 기록된다.
그러면 `ModelAssetConverter`의 재질명↔텍스처명 자동 매칭이 전부 어긋난다.
**메시를 뽑을 때는 항상 재질을 같이 로드시킨다.**

## 3. 클래스 코드 표

패키지·오브젝트는 클래스 영문명이 아니라 3글자 코드를 쓴다. 창술사는 `flm`이다.
`-list`로 전체 덤프한 뒤 `pc_<code>_<nn>` 패턴을 집계해 얻은 목록이다.

| 그룹 | 코드 |
|---|---|
| 전사(w) | `wbk` `wgl` `wdr` `whk` / 공용 `wr` |
| 무도가(f) | `fbm` `ffm` `fif` **`flm`(창술사)** |
| 헌터(g) | `gdh` `gbs` `gam` `gst` / 공용 `gn` |
| 마법사(m) | `mbd` `msm` `mar` `mem` / 공용 `mg` |
| 암살자(d) | `dbl` `ddk` `ddm` `drp` `dse` / 공용 `dl` `dk` |
| 스페셜리스트(s) | `sdm` `smw` `sru` / 공용 `sp` |

`ft`는 클래스가 아니라 여성 캐릭터 기본 바디/얼굴 템플릿이다. 창술사는 여성 전용
클래스라 얼굴·머리를 `PC_FT_*`에서 가져와야 한다.

## 4. 구성 패키지

`PC_FLM*` 와일드카드로 18개 패키지, 오브젝트 1213개가 나온다.
이번에 조립에 쓴 것은 다음과 같다.

```text
PC_FLM_00        pc_flm_00_upper / lower / arm / shoulder / helmet  (기본 복장 5파츠)
PC_FT_00         pc_ft_00_sk                                        (기본 바디)
PC_FT_00_FACE    pc_ft_00_face_sk                                   (얼굴)
PC_FT_00_HAIR    pc_ft_00_hair_sk                                   (머리)
PC_FLM_00        pc_flm_00_ani.psa                                  (애니메이션 223개)
```

그 외 `PC_FLM_01~08`(복장), `PC_FLM_AV_003/011/012/023/137`(아바타),
`PC_FLM_HR_*`(헤어), `WP_WFLM_*`(무기)는 스테이징에 추출만 해두고 이번 조립에는
넣지 않았다.

### 스켈레톤이 두 종류다

| 본 수 | 대상 |
|---|---|
| 221 | `pc_flm_00_upper`, `pc_ft_00_hair` |
| 207 | 나머지 전부, 그리고 `.psa` |

207은 221의 **부분집합**이다(누락 없음을 이름 대조로 확인). 추가 14개는
`b_add_hair01_b_*`, `b_add_tail_1_*`, `b_upper_skirt_*` 같은 머리·치마 물리 본이며
psa가 건드리지 않으므로 rest pose로 남는다.
따라서 **221본 아마추어를 마스터로 삼아** 나머지를 이름 기준으로 재바인딩하면 된다.

## 5. 조립 파이프라인

`umodel`의 glTF export는 애니메이션을 못 넣는다
(`ERROR: glTF animation could be exported from mesh viewer only` — GUI 뷰어 전용).
애니메이션이 필요하면 psk/psa로 뽑아 Blender를 경유해야 한다.

```text
umodel (-psk)  ->  .psk × 8 + .psa
      -> Blender 5.0 headless (io_scene_psk_psa)
         · 221본 psk를 먼저 임포트해 마스터 아마추어 확보
         · 나머지 psk의 메시를 마스터로 재부모화하고 여분 아마추어 삭제
         · psa 223개를 액션으로 임포트
      -> .fbx (액션당 1 take)
      -> ModelAssetConverter
      -> LanceMaster.wmodel + textures/
```

빌드 스크립트: `build_flm.py` (스크래치패드, 재사용하려면 저장소로 옮길 것)

### Blender에서 걸린 두 가지

- **애드온을 명시적으로 활성화해야 한다.** `--factory-startup`으로 띄우면 모듈은
  import되지만 등록이 안 돼 `Material.psk` 커스텀 프로퍼티가 없어서 psk 임포트가
  `AttributeError`로 죽는다. `addon_utils.enable('bl_ext.blender_org.io_scene_psk_psa')`를
  먼저 호출한다.
- **FBX 익스포터의 `bake_anim_use_all_actions`는 `animation_data`가 있는 오브젝트만
  훑는다.** psa 임포터는 액션만 만들고 아마추어에 할당하지 않으므로,
  `armature.animation_data_create()` 후 아무 액션이나 하나 할당해두지 않으면
  액션이 223개여도 FBX에 take가 0개로 나간다.

## 6. 검증

| 항목 | 결과 |
|---|---|
| umodel 추출 | `Exported 1213/1213 objects` (PC_FLM*), 경고는 미지원 머티리얼 노드 클래스뿐 |
| psa 시퀀스 수 | `pc_flm_00_ani` 223 + `_ani_bk` 206 = 429 → `-list`의 animsequence 429와 일치 |
| psa 본 수 | 207 → 221본 아마추어에 전부 매핑, 임포트 경고 0 |
| 병합 결과 | 메시 8개 / 정점 19,251 / 본 221 |
| FBX | 384.7 MB, 액션 223개 (psa 임포트 경고 0) |
| 재질 매칭 | `pc_flm_00_arm_mi` → `arm_d.tga` / `arm_n.tga` / `arm_s.tga` 정상 |
| 컨버터 | `meshes=17 animations=223`, 경고 없음 |

최종 산출물:

```text
Client/Bin/Resources/LostArk/Character/LanceMaster/
├─ LanceMaster.wmodel   109.6 MB
└─ textures/            37개, 38.8 MB
```

`info` 결과는 `sections=226 animations=223 skeleton=yes`,
`material-version=2 count=15`, 애니메이션 section이 `flm_abn_bug_1` 처럼
액션 이름대로 223개 생성됐다. `git status`상 산출물은 전부 ignore되어 추적되지 않는다.

## 7. 남은 것 / 주의

- 렌더러가 실제로 쓰는 것은 diffuse와 normal이다. `_s`는 보존만 되고 화면에는
  반영되지 않는다(컨버터 README 기준).
- 애니메이션 이름은 FBX take 규칙상 `<아마추어명>_<액션명>`이 된다. 아마추어를
  `flm`으로 두었으므로 `flm_att_identity1_1_02` 형태다.
- `AssetTest` 레벨에서 실제 렌더 확인은 아직 하지 않았다.
- 원본 리소스는 Smilegate 저작물이다. 스테이징 폴더와 결과물 모두 Git에 올리지
  말고 팀 Drive 팩 경로 규칙을 따른다.
