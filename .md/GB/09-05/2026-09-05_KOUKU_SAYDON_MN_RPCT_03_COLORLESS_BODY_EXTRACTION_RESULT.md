# 2026-09-05 쿠크세이튼 "변하는 광대" 후보 MN_RPCT_03 무채색 본체 추출 결과

브랜치 `codex/kouku-scale-1p7`. 같은 worktree에 다른 세션의 미커밋 변경(Arena gate spawn / Boss HUD)이
함께 있어 이 작업은 commit하지 않았다. 아래 "변경 파일"만 이 작업의 diff다.

## 1. 결론

| 항목 | 상태 |
|---|---|
| 사용자가 말한 "변하는 광대" 정체 | `MN_RPCT_03`으로 특정. 최종 확인은 사용자가 Workbench Resources / Animation Tool에서 실제 모델을 보고 판정한다 |
| `.wmodel` cook | 완료. `MN_RPCT_03.wmodel` 189,096,328 bytes, 249 clip, skeleton yes, SHA-256 `761b872a7ddac1784651f6cd071a3479fd00a746baae11c20b38efad7c31c9f7` |
| Resources 설치 | 완료. `Client/Bin/Resources/Character/KoukuSaton/MN_RPCT_03/` (wmodel 1 + textures 6). Git 비추적, 팀장 Drive 전달 필요 |
| Client 등록 | 완료. Animation preview asset, Composition animation target, Animation Tool clip donor, F1 Resource Files profile |
| Client compile | `ClCompile` Debug x64 성공(오류 0). 실행 중인 `Client.exe`가 출력물을 점유해 최종 link는 사용자가 종료 후 수행해야 한다 |
| 화면 확인 | 미실행. 사용자 전용 |

## 2. 탐색 근거

data3.lpk의 Action `.loa` 4,411개를 전부 디코딩해 UTF-16LE 이름을 스캔했다.

- "광대" 이름 캐릭터는 서커스 잡몹이다: 나팔부는 광대 `MN_REUP_01`, 춤추는 광대 `MN_REUP_02`,
  뿅망치 작은 광대 `MN_REUP_05`, 뿅망치 큰 광대 `MN_RHKP_01`, 대포 광대 `MN_RHKP_02`,
  외발자전거 광대 `MN_CMDUP_00`. 모두 카오스게이트·비탄의 섬용이며 `LV_LUT_MIDNIGHTC_ED`의
  package dependency closure(Character 11개)에 없다.
- 쿠크세이튼 레벨이 실제로 참조하는데 아직 cook되지 않은 캐릭터는 셋이다.

| package | 정체 | rig | 근거 |
|---|---|---|---|
| `MN_RPCT_01` | 광기의 사념체 (점프 잠식 잡몹) | 60본, 자체 PSA 47클립 | `MN_RPCT_01.loa` "광기의 사념체_전방으로 점프하여 잠식" 등 13 action. umodel 추출만 있고 cook 없음 |
| `MN_RPCT_03` | 무채색(흑백) 세이튼 본체 | 165본, RPCT_00/05 PSA와 이름·순서 완전 일치 | 자체 AnimSet 없음. `FX_MN_RPCT_03`이 레벨 직접 Effect 의존성. diffuse 4장(`03`, `03a`, `03_1`, `03_1a`) 모두 grayscale. `MN_RPCT_00.loa`에 "광기를 잃은 쿠크세이튼_*" 67 action(`_무채색` 포함) |
| `MN_RPCT_04` | 정적 소품 | 본 1개(`object001`) | RPCT_00/01 텍스처 재사용. 애니메이션 없음 |

`MN_RPCC_00`, `MN_RPCD_00/01`, `MN_RPCY_00`은 각각 크랙크릭, 핏빛 검사 케이든, 핏빛 창기사 제딘,
인큐버스 차원관문이며 쿠크세이튼과 무관하다.

## 3. 추출·cook 절차 (실행한 명령)

외부 staging root: `C:/LostArkExtract/LV_LUT_MIDNIGHTC_ED_20260829/RemainingCharacterExtraction-20260905/`.
저장소 안에는 cook 산출물을 두지 않았다.

1. umodel export (08-29와 같은 flag)
   `umodel_lostark_v7.exe -export -game=lostark -kr -nameresolve -dds -uncook -groups -path=<EFGame/ReleasePC> -out=<root>/UModelExports/MN_RPCT_03/Export MN_RPCT_03`
   source: `EFGame/ReleasePC/Packages/9G1M8UBM1BZKE7JT4964SVP.upk` 6,775,128 bytes,
   SHA-256 `0e52af83cf78f0742769e44269bcc1da03a27f261a31b50bdd7f7acef80fc257`. 45 object export.
2. rig 동일성: `mn_rpct_03_sk_loc_int.psk` REFSKELT 165 == `mn_rpct_00_ani.psa` / `mn_rpct_00_evt2_ani.psa` BONENAMES 165 (이름·순서 exact).
3. cook: `Tools/ActorXAssetCooker/Cook-ActorXWModel.ps1 -PskPath <psk> -PsaPath <RPCT_00 main psa>,<RPCT_00 evt2 psa> -StagingDirectory <root> -AssetName MN_RPCT_03 -TextureRoot <export tex> -ArmatureExportName rpct00 -BakeFrameRate 30 -ExpectedAnimationCount 249`
   결과 `[Model] sections=252 animations=249 skeleton=yes`, material 3, textures 6.
4. retime: `Tools/ModelAssetConverter/retime_wmodel_from_psa.py --runtime-prefix rpct00_` → 249 clip 중 37개 rate 복원(RPCT_05 cook과 동일 수치), `--check` receiptVerified true.
5. 검증: `ModelAssetConverter.exe info` clip 249개가 설치된 `MN_RPCT_05.wmodel`의 clip 집합과 동일.
6. 설치: `Client/Bin/Resources/Character/KoukuSaton/MN_RPCT_03/MN_RPCT_03.wmodel` + `textures/` 6 DDS, 복사 후 SHA-256 일치.
7. receipt: `<root>/mn-rpct-03-extraction.receipt.json` (source/psk/psa/wmodel/retime/install hash).

## 4. 변경 파일 (이 작업)

| 파일 | 변경 |
|---|---|
| `Client/Public/AnimationPreviewAssets.h` | `kakulsaydon.mn-rpct-03` preview asset 추가 (scale 0.017f, yaw -90, playback-only, boss archetype 없음) |
| `Client/Public/CompositionAnimationResource.h` | `COMPOSITION_ANIMATION_TARGET_ASSET_NAMES` 6 → 7, `MN_RPCT_03` 추가 |
| `Client/Private/Animation_Tool.cpp` | clip-only donor 조건을 `MN_RPCT_00 || MN_RPCT_03`로 확장 (Open_KoukuSaydonProfile, Adopt 경로, Clip Donor 패널, Kouku preview scale) |
| `Client/Private/MainApp.cpp` | F1 Resource Files → KoukuSaydon profile 매핑 목록에 `MN_RPCT_03` 추가 (1줄) |
| `Client/Private/SequencerTool.cpp` | "6 bodies" 고정 문자열을 배열 크기로 표시 |
| `Client/Private/CharacterPreviewPanel.cpp` | 상태 문구의 "six" 고정 표현 제거 |
| `Data/ResourceIntake/LV_LUT_MIDNIGHTC_ED.resource-intake.json` | `canonicalModels`, `packages.MN_RPCT_03`, `physicalPayload`(fileCount 70, bytes 712,824,176, animation section 889, aggregate `70175ceb…`), `characterPackageRoots` 갱신. aggregate 공식은 기존 값 `b4cfaf6f…`을 먼저 재현한 뒤 적용 |
| `CLAUDE.md` | 공용 Animation Resources 목록에 `MN_RPCT_03` 반영 |

Loader는 preview asset 파일이 없으면 해당 항목만 건너뛴다(`Loader.cpp` `is_regular_file` 검사). 아직 Drive
전달을 받지 않은 팀원 PC에서 Level 진입이 실패하지는 않는다.

## 5. 검증

| 검사 | 결과 |
|---|---|
| `msbuild Client.vcxproj /t:ClCompile Debug x64` | 성공, 오류 0 (재컴파일 10 TU). link 미실행: `Client.exe`/`Server.exe` 실행 중 |
| `test_kouku_saydon_client_product_level_contract.py` | 17 tests OK |
| `test_kouku_saydon_naming_boundary.py` | 1 실패. `MainApp.h:183`, `Level_KakulSaydonArena.cpp:31/1114/1310`의 `KakulSaydon` 잔류이며 이 작업이 만지지 않은 다른 세션 변경 |
| intake JSON parse / CRLF round-trip | 통과 |
| `git diff --check` | 통과 |
| 화면 확인 | 미실행 (사용자 전용) |

## 6. 사용자가 직접 확인할 경로

1. 실행 중인 `Client.exe`를 종료하고 Client를 다시 link/build한다.
2. Debug F1 → Action Workbench → Boss KoukuSaydon → Resources 목록에 `MN_RPCT_03` (249 clip) 항목이 보이는지 확인한다.
3. 또는 F1 Resource Files에서 `Resources/Character/KoukuSaton/MN_RPCT_03/MN_RPCT_03.wmodel`을 선택하면 Animation Tool이 clip donor preview로 연다.
4. 흑백 세이튼 본체가 RPCT_00 clip(`rpct00_att_battle_*`, `dmg_critical_*` 등)으로 재생되면 "변하는 광대"가 맞는지 판정한다.

## 7. 남은 경계

- `MN_RPCT_03`은 reference-only preview body다. Server profile, BossCatalog archetype, action reference 문서는 만들지 않았다.
  "광기를 잃은 쿠크세이튼" 67 action의 reference 문서가 필요하면 `MN_RPCT_00.action-effects.json`을
  `extract_action_effect_notifies.py`로 먼저 생성해야 한다. 디코딩한 `MN_RPCT_00.loa`(현재 data3.lpk 기준)는
  `<root>/ActionSources/`에 보존했다.
- `MN_RPCT_03`의 `a` 변형 material(`mn_rpct_03a`, `mn_rpct_03_1a`)과 `_dead` material은 wmodel의 기본 material 3종에
  포함되지 않았다. 색 변형 전환이 필요하면 별도 material 계약이 필요하다.
- data3.lpk가 2026-09-02에 패치되어 08-29의 `data3.index.hashed.json`(12,307 entry)은 stale이다. 현재 archive는 12,319 entry이며
  `C:/LostArkExtract/Tooling/LpkReader-20260905/lpk_reader.py`가 live index를 읽는다.
- `MN_RPCT_01`(광기의 사념체)과 `MN_RPCT_04`(소품)는 umodel export까지만 있고 cook하지 않았다.
- Resources 물리 파일은 Git에 넣지 않았다. 팀장 Drive 폴더에 `Character/KoukuSaton/MN_RPCT_03/` 7개 파일을 같은 상대 경로로 올려야 한다.
