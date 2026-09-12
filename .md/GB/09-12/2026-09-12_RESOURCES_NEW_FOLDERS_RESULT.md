# Resources 신규 폴더·수정 모델 조사 결과

기준: **2026-09-11 23:00:00 KST 이후**부터 **2026-09-12 15:20:14 +09:00**까지.

조사 위치: `C:/Users/user/Desktop/LostArk/Client/Bin/Resources`

현재 디스크에 존재하는 모든 하위 폴더 **7,864개**를 숨김 항목까지 재귀 조회하고 Windows `CreationTime`으로 선별했다. 사용자가 확정한 23시 기준으로 기존 정리본을 수정했다. 조건에 맞는 폴더는 **60개**이며, 모두 9월 12일에 생성됐다.

## 경로별 요약

모든 경로는 Resources 기준 상대 경로다. 폴더 수는 신규 폴더만 세며 하위 폴더를 포함한다.

| 경로 | 내용 | 신규 폴더 수 | 생성 시각 KST |
|---|---|---:|---|
| `Effect/KoukuSaydon/FullRestore/Textures` 아래 | 쿠크 이펙트 텍스처 패키지별 폴더 | **37** | 9월 12일 01:56:02~02:54:36 |
| `Map/KakulSaydon/SourceSequences` 및 그 아래 | 쿠크 관문별 컷신 배우·텍스처 | **23** | 9월 12일 11:32:02~12:18:52 |
| 합계 | | **60** | |

`Effect/KoukuSaydon/FullRestore/Textures` 자체는 9월 11일 06:01:37에 생성된 기존 폴더다. 이번 집계에는 그 아래에서 23시 이후 새로 생긴 37개 폴더만 들어간다. `FullRestore/Meshes` 역시 기존 폴더이므로 신규 폴더 목록에 들어가지 않는다.

`Character`, `Deploy`, `Fonts`, `Sound`, `UI`에는 이 기준으로 신규 폴더가 없었다. 앞서 오전 기준에 포함했던 베른·발탄 재질, World 클릭 표시, Gate2Intro, MIDNIGHTC_ED Meshes/Textures 폴더도 모두 23시 이전 생성이므로 제외했다.

## SourceSequences 세부 구성

| 위치 | 신규 하위 구성 | 폴더 수 |
|---|---|---:|
| `SourceSequences` | 최상위 신규 폴더 자체 | 1 |
| `kouku.gate1.full` | Book, SaydonBook, SaydonFinale, SaydonStage와 각각의 textures | 9 |
| `kouku.gate2.clear` | Kouku, LargeSaydon, SaydonArrival과 각각의 textures | 7 |
| `kouku.gate2.maze` | Kouku와 textures | 3 |
| `kouku.gate3.intro` | SaydonArrival과 textures | 3 |
| 합계 | 시퀀스 4개, 배우 9개 및 각각의 textures, 루트 1개 | **23** |

## 신규 폴더 전체 목록

| 번호 | Resources 상대 폴더 경로 | 생성 시각 KST |
|---:|---|---|
| 1 | `Effect/KoukuSaydon/FullRestore/Textures/bg_rad_biackiss_b` | 2026-09-12 02:41:54 |
| 2 | `Effect/KoukuSaydon/FullRestore/Textures/bg_rad_koukusaton_b` | 2026-09-12 02:41:39 |
| 3 | `Effect/KoukuSaydon/FullRestore/Textures/bg_rad_koukusaton_g` | 2026-09-12 02:41:57 |
| 4 | `Effect/KoukuSaydon/FullRestore/Textures/cine_prob_02` | 2026-09-12 02:41:44 |
| 5 | `Effect/KoukuSaydon/FullRestore/Textures/efmaster_material` | 2026-09-12 02:41:39 |
| 6 | `Effect/KoukuSaydon/FullRestore/Textures/efmaster_material_prologue` | 2026-09-12 02:42:04 |
| 7 | `Effect/KoukuSaydon/FullRestore/Textures/enginematerials` | 2026-09-12 02:54:36 |
| 8 | `Effect/KoukuSaydon/FullRestore/Textures/fx_d_w_01` | 2026-09-12 02:41:43 |
| 9 | `Effect/KoukuSaydon/FullRestore/Textures/fx_mastermaterial` | 2026-09-12 02:41:43 |
| 10 | `Effect/KoukuSaydon/FullRestore/Textures/fx_tex_02` | 2026-09-12 01:56:02 |
| 11 | `Effect/KoukuSaydon/FullRestore/Textures/fx_tex_03` | 2026-09-12 02:41:42 |
| 12 | `Effect/KoukuSaydon/FullRestore/Textures/fx_tex_04` | 2026-09-12 01:56:02 |
| 13 | `Effect/KoukuSaydon/FullRestore/Textures/fx_tex_06` | 2026-09-12 02:41:51 |
| 14 | `Effect/KoukuSaydon/FullRestore/Textures/fx_tex_high_00` | 2026-09-12 02:41:50 |
| 15 | `Effect/KoukuSaydon/FullRestore/Textures/fx_tex_high_01` | 2026-09-12 02:41:47 |
| 16 | `Effect/KoukuSaydon/FullRestore/Textures/fx_tex_high_03` | 2026-09-12 01:56:03 |
| 17 | `Effect/KoukuSaydon/FullRestore/Textures/fx_tex_nomipmap_00` | 2026-09-12 02:41:41 |
| 18 | `Effect/KoukuSaydon/FullRestore/Textures/mn_boim_00` | 2026-09-12 02:41:59 |
| 19 | `Effect/KoukuSaydon/FullRestore/Textures/mn_cngn_00` | 2026-09-12 02:42:00 |
| 20 | `Effect/KoukuSaydon/FullRestore/Textures/mn_iskk_01` | 2026-09-12 02:42:02 |
| 21 | `Effect/KoukuSaydon/FullRestore/Textures/mn_rhcn_00` | 2026-09-12 02:21:53 |
| 22 | `Effect/KoukuSaydon/FullRestore/Textures/mn_rhoc_00` | 2026-09-12 02:42:02 |
| 23 | `Effect/KoukuSaydon/FullRestore/Textures/mn_rhoc_00-1` | 2026-09-12 02:42:01 |
| 24 | `Effect/KoukuSaydon/FullRestore/Textures/mn_rpct_05` | 2026-09-12 02:41:51 |
| 25 | `Effect/KoukuSaydon/FullRestore/Textures/mn_rpcz_00` | 2026-09-12 02:42:02 |
| 26 | `Effect/KoukuSaydon/FullRestore/Textures/mn_rpdi_00` | 2026-09-12 02:41:56 |
| 27 | `Effect/KoukuSaydon/FullRestore/Textures/sk_mn_ppct_05` | 2026-09-12 02:42:03 |
| 28 | `Effect/KoukuSaydon/FullRestore/Textures/wp_mn_adcs_03` | 2026-09-12 01:56:04 |
| 29 | `Effect/KoukuSaydon/FullRestore/Textures/wp_mn_ppct_00` | 2026-09-12 02:42:03 |
| 30 | `Effect/KoukuSaydon/FullRestore/Textures/wp_mn_reup_01` | 2026-09-12 02:42:04 |
| 31 | `Effect/KoukuSaydon/FullRestore/Textures/wp_mn_rhkp_06` | 2026-09-12 01:56:05 |
| 32 | `Effect/KoukuSaydon/FullRestore/Textures/wp_mn_rhkp_07` | 2026-09-12 02:42:04 |
| 33 | `Effect/KoukuSaydon/FullRestore/Textures/wp_mn_rpct_05` | 2026-09-12 02:42:04 |
| 34 | `Effect/KoukuSaydon/FullRestore/Textures/wp_mn_rpct_07` | 2026-09-12 02:21:53 |
| 35 | `Effect/KoukuSaydon/FullRestore/Textures/wp_mn_tslr_00` | 2026-09-12 02:41:53 |
| 36 | `Effect/KoukuSaydon/FullRestore/Textures/wp_mn_ttbb_02` | 2026-09-12 02:42:05 |
| 37 | `Effect/KoukuSaydon/FullRestore/Textures/wp_wdbl_00` | 2026-09-12 02:41:53 |
| 38 | `Map/KakulSaydon/SourceSequences` | 2026-09-12 11:32:02 |
| 39 | `Map/KakulSaydon/SourceSequences/kouku.gate1.full` | 2026-09-12 11:32:02 |
| 40 | `Map/KakulSaydon/SourceSequences/kouku.gate1.full/Book` | 2026-09-12 11:32:02 |
| 41 | `Map/KakulSaydon/SourceSequences/kouku.gate1.full/Book/textures` | 2026-09-12 11:32:02 |
| 42 | `Map/KakulSaydon/SourceSequences/kouku.gate1.full/SaydonBook` | 2026-09-12 11:32:12 |
| 43 | `Map/KakulSaydon/SourceSequences/kouku.gate1.full/SaydonBook/textures` | 2026-09-12 11:32:12 |
| 44 | `Map/KakulSaydon/SourceSequences/kouku.gate1.full/SaydonFinale` | 2026-09-12 12:18:52 |
| 45 | `Map/KakulSaydon/SourceSequences/kouku.gate1.full/SaydonFinale/textures` | 2026-09-12 12:18:52 |
| 46 | `Map/KakulSaydon/SourceSequences/kouku.gate1.full/SaydonStage` | 2026-09-12 11:32:24 |
| 47 | `Map/KakulSaydon/SourceSequences/kouku.gate1.full/SaydonStage/textures` | 2026-09-12 11:32:24 |
| 48 | `Map/KakulSaydon/SourceSequences/kouku.gate2.clear` | 2026-09-12 11:39:08 |
| 49 | `Map/KakulSaydon/SourceSequences/kouku.gate2.clear/Kouku` | 2026-09-12 11:39:13 |
| 50 | `Map/KakulSaydon/SourceSequences/kouku.gate2.clear/Kouku/textures` | 2026-09-12 11:39:13 |
| 51 | `Map/KakulSaydon/SourceSequences/kouku.gate2.clear/LargeSaydon` | 2026-09-12 11:39:08 |
| 52 | `Map/KakulSaydon/SourceSequences/kouku.gate2.clear/LargeSaydon/textures` | 2026-09-12 11:39:08 |
| 53 | `Map/KakulSaydon/SourceSequences/kouku.gate2.clear/SaydonArrival` | 2026-09-12 11:39:25 |
| 54 | `Map/KakulSaydon/SourceSequences/kouku.gate2.clear/SaydonArrival/textures` | 2026-09-12 11:39:25 |
| 55 | `Map/KakulSaydon/SourceSequences/kouku.gate2.maze` | 2026-09-12 11:32:30 |
| 56 | `Map/KakulSaydon/SourceSequences/kouku.gate2.maze/Kouku` | 2026-09-12 11:32:30 |
| 57 | `Map/KakulSaydon/SourceSequences/kouku.gate2.maze/Kouku/textures` | 2026-09-12 11:32:30 |
| 58 | `Map/KakulSaydon/SourceSequences/kouku.gate3.intro` | 2026-09-12 11:39:37 |
| 59 | `Map/KakulSaydon/SourceSequences/kouku.gate3.intro/SaydonArrival` | 2026-09-12 11:39:37 |
| 60 | `Map/KakulSaydon/SourceSequences/kouku.gate3.intro/SaydonArrival/textures` | 2026-09-12 11:39:37 |

## 판정 범위와 검증

과거 스냅샷과의 비교가 아니라 이 PC의 현재 생성 시각 기준이다. 복사·재설치로 생성된 폴더도 포함될 수 있으며, 수정 시각은 신규 판정에 사용하지 않았다. 기준 시각 이전부터 존재한 폴더 안에 새로 추가되거나 교체된 파일은 이 신규 폴더 목록에 포함하지 않는다.

전체 7,864개 폴더를 재조회해 새 기준의 60개 경로를 확인했다. 요약의 37+23과 전체 목록 60행이 일치하며 SourceSequences는 1+9+7+3+3=23개다. 기존 조사에서 reparse point는 0개였고 이번 재조회도 오류 없이 완료됐다.

리소스는 조회만 했으며 이동·삭제·재배치하지 않았다. 이번 변경은 이 결과 문서의 기준 시각과 목록 수정이다. 빌드·publisher·stage·commit·push는 실행하지 않았다.

## 추가 확인: Drive로 전달할 수정·신규 연출 모델

사용자가 지정한 다섯 범주의 실물과 설치 기록을 2026-09-12에 대조했다. **모두 현재 Resources에 존재한다.** 수정 모델 7개는 실제 SHA-256이 설치 receipt의 candidate SHA와 일치하고, 반사 모델 26개도 생성 mapping의 candidate SHA와 모두 일치했다. SourceSequences의 신규 배우 모델 9개와 텍스처도 존재한다. 지정 범위의 WModel은 합계 **42개(수정 7 + 반사 신규 26 + 시퀀스 신규 9)**다.

앞선 신규 폴더 60개는 폴더 생성 시각만으로 선별한 목록이다. 아래 수정 모델 7개와 반사 모델 26개는 기존 폴더 안에 교체·추가됐으므로, 기존 Drive 배포에는 이 항목들도 함께 포함해야 한다.

| 전달 위치: Resources 상대 경로 | 실물 확인 | 확인한 교정/신규 내용 |
|---|---|---|
| `Map/KakulSaydon/Gate2Intro/Table/` | 모델 1개 + 텍스처 4개 | 468,064-byte 교정본. 잘못된 bounds tail 80 bytes 제거 |
| `Map/KakulSaydon/Gate2Intro/CardEruption/` | 모델 1개 + 텍스처 3개 | 30Hz / 14초 교정본 |
| `Map/KakulSaydon/SourceSequences/` | 모델 9개 + DDS 105개 | 관문별 신규 컷신 배우. 전체 폴더 전달 |
| `Character/KoukuSaton/MN_CDMD_00/` | 모델 1개 + 텍스처 4개 | 인형 자식 본 회전 교정본 |
| `Map/LV_LUT_MIDNIGHTC_ED/` 안의 반사 모델 포함 폴더 24개 | 신규 모델 26개 | `__reflect_npp`, `__reflect_ppn`, `__reflect_npn` 변형 |
| `Map/LV_LUT_MIDNIGHTC_ED/` 안의 DECO24D/E/F 폴더 3개 | 수정 모델 3개 | 잘못 연결된 발광 입력 교정본 |
| `Effect/DimensionMaster/Models/SK_SWP_CUB_00/` | 모델 1개 + 텍스처 1개 | `sk_swp_cub_00_sk.wmodel`의 30Hz / 약 3.333초 교정본 |

수정 모델 7개는 모두 **2026-09-12 15:06:44 KST**에 설치된 실물이며, 반사 모델 26개는 **12:20:00 KST**에 생성됐다. 마지막 큐브 폴더 이름은 `SK_SWP_CUB_00`이며 사용자 문장의 Markdown 이스케이프 역슬래시는 실제 경로에 포함하지 않는다.

### 교체된 모델 7개 정확한 경로

| Resources 상대 파일 경로 | 크기 bytes | 설치 후보 SHA 일치 |
|---|---:|---|
| `Character/KoukuSaton/MN_CDMD_00/MN_CDMD_00.wmodel` | 6,242,500 | 일치 |
| `Map/LV_LUT_MIDNIGHTC_ED/MAP_CFEDE8067300_BG_RAD_KOUKUSATON_DECO24D_SM_KHB/MAP_CFEDE8067300_BG_RAD_KOUKUSATON_DECO24D_SM_KHB.wmodel` | 79,064 | 일치 |
| `Map/LV_LUT_MIDNIGHTC_ED/MAP_B71A2EC9D778_BG_RAD_KOUKUSATON_DECO24E_SM_KHB/MAP_B71A2EC9D778_BG_RAD_KOUKUSATON_DECO24E_SM_KHB.wmodel` | 69,408 | 일치 |
| `Map/LV_LUT_MIDNIGHTC_ED/MAP_7AC8BB3D2FEE_BG_RAD_KOUKUSATON_DECO24F_SM_KHB/MAP_7AC8BB3D2FEE_BG_RAD_KOUKUSATON_DECO24F_SM_KHB.wmodel` | 105,144 | 일치 |
| `Map/KakulSaydon/Gate2Intro/CardEruption/CardEruption.wmodel` | 2,527,340 | 일치 |
| `Effect/DimensionMaster/Models/SK_SWP_CUB_00/sk_swp_cub_00_sk.wmodel` | 347,472 | 일치 |
| `Map/KakulSaydon/Gate2Intro/Table/Table.wmodel` | 468,064 | 일치 |

### 반사 변형 모델 26개 정확한 경로

아래 파일이 들어 있는 원본 asset 폴더 24개를 기존 텍스처와 함께 전달한다. 외곽불 3개는 이 목록과 별도다.

| Resources 상대 파일 경로 | 크기 bytes | 생성 후보 SHA 일치 |
|---|---:|---|
| `Map/LV_LUT_MIDNIGHTC_ED/MAP_0FF6DCE3C734_BG_RAD_KOUKUSATON_DECO06A_SM/MAP_0FF6DCE3C734_BG_RAD_KOUKUSATON_DECO06A_SM__reflect_npp.wmodel` | 10,746 | 일치 |
| `Map/LV_LUT_MIDNIGHTC_ED/MAP_3A0FD723820D_BG_RAD_KOUKUSATON_DECO02B_SM/MAP_3A0FD723820D_BG_RAD_KOUKUSATON_DECO02B_SM__reflect_npp.wmodel` | 11,280 | 일치 |
| `Map/LV_LUT_MIDNIGHTC_ED/MAP_05DF2F9D2EC3_BG_RAD_KOUKUSATON_DECO06_SM/MAP_05DF2F9D2EC3_BG_RAD_KOUKUSATON_DECO06_SM__reflect_npp.wmodel` | 10,684 | 일치 |
| `Map/LV_LUT_MIDNIGHTC_ED/MAP_0552B41F2862_BG_RAD_KOUKUSATON_DECO06B_SM/MAP_0552B41F2862_BG_RAD_KOUKUSATON_DECO06B_SM__reflect_ppn.wmodel` | 10,560 | 일치 |
| `Map/LV_LUT_MIDNIGHTC_ED/MAP_7B8C86EC0AD0_BG_RAD_KOUKUSATON_GATE03_SM/MAP_7B8C86EC0AD0_BG_RAD_KOUKUSATON_GATE03_SM__reflect_npp.wmodel` | 10,622 | 일치 |
| `Map/LV_LUT_MIDNIGHTC_ED/MAP_2D1E7CFA3E1E_BG_RAD_KOUKUSATON_GATE03D_SM/MAP_2D1E7CFA3E1E_BG_RAD_KOUKUSATON_GATE03D_SM__reflect_ppn.wmodel` | 10,752 | 일치 |
| `Map/LV_LUT_MIDNIGHTC_ED/MAP_99E4CAC6EFC8_BG_LUT_PEARHABIT_CIRCUSTENT02C_SM_KHG_OVR_F0A440184632/MAP_99E4CAC6EFC8_BG_LUT_PEARHABIT_CIRCUSTENT02C_SM_KHG_OVR_F0A440184632__reflect_npp.wmodel` | 49,204 | 일치 |
| `Map/LV_LUT_MIDNIGHTC_ED/MAP_055345A3D79E_BG_RAD_KOUKUSATON_GATE03B_SM/MAP_055345A3D79E_BG_RAD_KOUKUSATON_GATE03B_SM__reflect_npp.wmodel` | 10,808 | 일치 |
| `Map/LV_LUT_MIDNIGHTC_ED/MAP_8F406FBFD1A2_BG_RAD_KOUKUSATON_GATE01C_SM/MAP_8F406FBFD1A2_BG_RAD_KOUKUSATON_GATE01C_SM__reflect_ppn.wmodel` | 10,622 | 일치 |
| `Map/LV_LUT_MIDNIGHTC_ED/MAP_4B7DA198D942_BG_RAD_KOUKUSATON_GATE03E_SM/MAP_4B7DA198D942_BG_RAD_KOUKUSATON_GATE03E_SM__reflect_ppn.wmodel` | 10,684 | 일치 |
| `Map/LV_LUT_MIDNIGHTC_ED/MAP_DB703C0BAACF_BG_RAD_KOUKUSATON_FENCE01_SM/MAP_DB703C0BAACF_BG_RAD_KOUKUSATON_FENCE01_SM__reflect_npp.wmodel` | 10,560 | 일치 |
| `Map/LV_LUT_MIDNIGHTC_ED/MAP_44B79CCEABB0_BG_RAD_KOUKUSATON_GATE03C_SM/MAP_44B79CCEABB0_BG_RAD_KOUKUSATON_GATE03C_SM__reflect_npp.wmodel` | 10,752 | 일치 |
| `Map/LV_LUT_MIDNIGHTC_ED/MAP_BFBFF5B96E23_BG_RAD_KOUKUSATON_DECO03E_SM/MAP_BFBFF5B96E23_BG_RAD_KOUKUSATON_DECO03E_SM__reflect_ppn.wmodel` | 10,932 | 일치 |
| `Map/LV_LUT_MIDNIGHTC_ED/MAP_87946967F123_BG_RAD_KOUKUSATON_DECO25_SM/MAP_87946967F123_BG_RAD_KOUKUSATON_DECO25_SM__reflect_npp.wmodel` | 10,436 | 일치 |
| `Map/LV_LUT_MIDNIGHTC_ED/MAP_82A521F7E878_BG_RAD_KOUKUSATON_DECO27_SM/MAP_82A521F7E878_BG_RAD_KOUKUSATON_DECO27_SM__reflect_npp.wmodel` | 10,436 | 일치 |
| `Map/LV_LUT_MIDNIGHTC_ED/MAP_ORIG_BG_RAD_KOUKUSATON_DECO26_SM/MAP_ORIG_BG_RAD_KOUKUSATON_DECO26_SM__reflect_npp.wmodel` | 10,436 | 일치 |
| `Map/LV_LUT_MIDNIGHTC_ED/MAP_ORIG_BG_RAD_KOUKUSATON_DECO28_SM/MAP_ORIG_BG_RAD_KOUKUSATON_DECO28_SM__reflect_npp.wmodel` | 10,436 | 일치 |
| `Map/LV_LUT_MIDNIGHTC_ED/MAP_E48115105A38_BG_RAD_KOUKUSATON_DECO05B_SM/MAP_E48115105A38_BG_RAD_KOUKUSATON_DECO05B_SM__reflect_ppn.wmodel` | 10,622 | 일치 |
| `Map/LV_LUT_MIDNIGHTC_ED/MAP_38822DA233AA_BG_EVT_CHRISTMAS_LIGHTING01B_SM_OVR_F0A440184632/MAP_38822DA233AA_BG_EVT_CHRISTMAS_LIGHTING01B_SM_OVR_F0A440184632__reflect_ppn.wmodel` | 35,188 | 일치 |
| `Map/LV_LUT_MIDNIGHTC_ED/MAP_7D3C56D9305A_BG_RAD_KOUKUSATON_GATE04_SM/MAP_7D3C56D9305A_BG_RAD_KOUKUSATON_GATE04_SM__reflect_npp.wmodel` | 10,684 | 일치 |
| `Map/LV_LUT_MIDNIGHTC_ED/MAP_38822DA233AA_BG_EVT_CHRISTMAS_LIGHTING01B_SM_OVR_F0A440184632/MAP_38822DA233AA_BG_EVT_CHRISTMAS_LIGHTING01B_SM_OVR_F0A440184632__reflect_npp.wmodel` | 35,188 | 일치 |
| `Map/LV_LUT_MIDNIGHTC_ED/MAP_38822DA233AA_BG_EVT_CHRISTMAS_LIGHTING01B_SM_OVR_F0A440184632/MAP_38822DA233AA_BG_EVT_CHRISTMAS_LIGHTING01B_SM_OVR_F0A440184632__reflect_npn.wmodel` | 35,188 | 일치 |
| `Map/LV_LUT_MIDNIGHTC_ED/MAP_0EE955A1CF06_BG_EVT_CHRISTMAS_LIGHTING01A_SM_OVR_F0A440184632/MAP_0EE955A1CF06_BG_EVT_CHRISTMAS_LIGHTING01A_SM_OVR_F0A440184632__reflect_npp.wmodel` | 25,116 | 일치 |
| `Map/LV_LUT_MIDNIGHTC_ED/MAP_7AC977CB9E7A_BG_EVT_CHRISTMAS_LIGHTING01_SM_OVR_F0A440184632/MAP_7AC977CB9E7A_BG_EVT_CHRISTMAS_LIGHTING01_SM_OVR_F0A440184632__reflect_npp.wmodel` | 20,316 | 일치 |
| `Map/LV_LUT_MIDNIGHTC_ED/MAP_87FF423F2C76_BG_RAD_KOUKUSATON_DECO05E_SM_RSH/MAP_87FF423F2C76_BG_RAD_KOUKUSATON_DECO05E_SM_RSH__reflect_npp.wmodel` | 16,980 | 일치 |
| `Map/LV_LUT_MIDNIGHTC_ED/MAP_B69796096221_BG_RAD_KOUKUSATON_GATE02_SM/MAP_B69796096221_BG_RAD_KOUKUSATON_GATE02_SM__reflect_npp.wmodel` | 11,056 | 일치 |

Map의 대상은 `Map/LV_LUT_MIDNIGHTC_ED/<asset>/...` 직접 자식 경로다. 별도로 존재하는 `Map/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED/...` 중첩 사본과 혼동하지 않는다.

### 근거와 완료 범위

- [기존 모델 설치 기록](C:/Users/user/Desktop/LostArk/out/KoukuSequenceCombat20260912/resource_install_receipt.json): `installed=true`; 인형·외곽불 3개·CardEruption·큐브의 현재 SHA가 후보와 일치.
- [Table 설치 기록](C:/Users/user/Desktop/LostArk/out/KoukuSequenceAdmission20260912/table_install_receipt.json): `installed=true`; 현재 SHA가 후보와 일치.
- [반사 모델 mapping](C:/Users/user/Desktop/LostArk/out/Reflect/mapping.json): `variants` 26개 모두 실물 존재 및 candidate SHA 일치, 부모 폴더 24개.
- [실제 설치 리소스의 기존 검증 기록](C:/Users/user/Desktop/LostArk/out/KoukuSequenceAdmission20260912/model_probe_seven/verification_receipt-installed.json): 기존 세션에서 모델/재질 준비 146개, Clone 146개 성공. 이번 확인에서 해당 프로그램을 다시 실행한 것은 아니다.

인형·큐브 개별 RESULT의 초기 ‘후보/미설치’ 문장은 후속 설치 이전 상태다. 이번 판정에는 부모 [최신 설치 결과 G08](C:/Users/user/Desktop/LostArk/.md/GB/09-12/2026-09-12_KOUKU_SEQUENCE_COMBAT_HANDOFF_RESULT.md)과 위 설치 receipt 및 현재 파일을 사용했다.

이번 요청에서는 로컬 실물·설치 일치 여부만 확인하고 이 문서에 반영했다. Drive 업로드·외부 전달은 수행하지 않았다. Resources/제품 코드는 변경하지 않았고 Client 실행·화면 판정도 하지 않았다.
