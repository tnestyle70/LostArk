# 2026-09-18 베른 성 텍스처 완료 NPC 19명 배치 결과

## 목적
텍스처·재질까지 붙은 19개 패키지만 베른 성에 패키지당 1명씩 배치해 모델 형상, 텍스처 연결,
미니맵 표시를 사용자가 실제 화면에서 확인할 수 있게 한다. 베른 성 부하 때문에 19명을 넘기지 않는다.
나머지 패키지는 이번 범위에서 버린다.

## 1. 설치 (Client/Bin/Resources/Character/NPC)
스테이징 `bern_cooked_tex` 의 19개를 `.wmodel` 과 `textures/` 만 복사해 설치했다.
`.fbx`, `.actorx.json`, `.actorx-work-*` 는 제외했다. 설치 후 각 wmodel 을 바이트로 열어 검증했다.

- 텍스처 참조는 UTF-16LE 로 저장된다. `textures/<name>.tga` 패턴으로 셌다.
- 19개 전부: 참조 수 = `textures/` 파일 수, 누락 0, `<psk_stem>.ao_idle_normal_1` 클립 존재. **19/19 통과**
- 참조 수 분포: 2개(NP_0000_00, NP_LRJP_00, NP_REOD_00, NP_SHHS_00, NP_SJWD_00),
  3개(대부분), 4개(MN_CNLF_00, MN_PETAT_00), 6개(MN_CNGG_00)

### 이번 19개에 포함되지 않은 구버전 설치본 13개
`Npc_MN_ANNF_01, MN_CNSX_00, MN_CNTF_00, MN_CNTM_00, MN_LDNW_00, MN_MKEN_01, MN_REMB_00,
MN_SLZM_00, MN_SLZM_04, MN_UMFT_00, MN_UMWF_00, NP_0018_00, NP_LRCP_00`
텍스처가 없는 1차 쿠킹본이다. 폴더는 그대로 두었고 이번 배치에서는 참조하지 않는다.
팀 기존 자산(`Npc_11592` 등 숫자 폴더, `Npc_Beda`, `Npc_Forman`, `Npc_25008`, `Npc_25016`)은 건드리지 않았다.

## 2. 선정된 19명
패키지별 후보 중 `EFTable_Npc.MapSymbolIndex` 가 있는 항목을 우선하고, 동률이면 이름 있는 쪽,
그래도 동률이면 NpcId 가 작은 쪽을 골랐다. 좌표와 yaw 는 원본 DeployData 변환값 그대로이고
yaw 는 0~360 으로 정규화했다.

| 패키지 | 이름 | NpcId | 기능 | archetypeId | 좌표(x, y, z) | yaw | idleClip |
|---|---|---|---|---|---|---|---|
| MN_CNAB_00 | 그레텔 | 25255 | [지도 교환] | NPC_25255 | 75.201, 34.610, -36.040 | 231.86 | `mn_cnab_00_sk.ao_idle_normal_1` |
| MN_CNBF_00 | 스이에 | 25007 | [연금술사] | NPC_25007 | 243.157, 44.290, -66.308 | 178.88 | `mn_cnbf_00_sk.ao_idle_normal_1` |
| MN_CNCM_00 | 집배원 레노엘 | 25072 | [우편] | NPC_25072 | 70.882, 42.260, -73.277 | 185.80 | `mn_cncm_00_sk.ao_idle_normal_1` |
| MN_CNCN_00 | 스트라벨 | 25030 | [창고지기] | NPC_25030 | 64.182, 42.282, -75.476 | 355.43 | `mn_cncn_00_00_sk.ao_idle_normal_1` |
| MN_CNDM_00 | 마법학자 에단 | 25025 | [물약 상인] | NPC_25025 | 189.816, 49.110, -146.669 | 276.86 | `mn_cndm_00_sk.ao_idle_normal_1` |
| MN_CNGG_00 | 바빌루 | 25044 | [거래소 중개인] | NPC_25044 | 62.119, 42.262, -93.138 | 265.43 | `mn_cngg_00_sk.ao_idle_normal_1` |
| MN_CNLF_00 | 윈리 | 25012 | [수리공] | NPC_25012 | 195.033, 49.110, -146.608 | 303.75 | `mn_cnlf_00_sk.ao_idle_normal_1` |
| MN_CNLP_00 | 타르코 | 25057 | [창고지기] | NPC_25057 | 195.066, 38.460, -40.897 | 46.54 | `mn_cnlp_00_sk.ao_idle_normal_1` |
| MN_CNMF_00 | 마레인 | 25019 | [잡화 상인] | NPC_25019 | 194.530, 47.619, -118.836 | 343.50 | `mn_cnmf_00_sk.ao_idle_normal_1` |
| MN_CNTW_00 | 옥타니아 | 25306 | [화폐 거래소] | NPC_25306 | 58.682, 42.260, -93.661 | 234.67 | `mn_cntw_00_sk.ao_idle_normal_1` |
| MN_CNWM_00 | 바스커빌 | 25045 | [창고지기] | NPC_25045 | 61.428, 45.462, -119.082 | 243.28 | `mn_cnwm_00_sk.ao_idle_normal_1` |
| MN_ISRY_00 | 로넬리 | 25024 | [창고지기] | NPC_25024 | 178.519, 47.621, -147.132 | 226.05 | `mn_isry_00_sk.ao_idle_normal_1` |
| MN_PETAT_00 | 일하는 아카테냥 | 80025 | [펫 관리] | NPC_80025 | 188.425, 49.117, -144.637 | 289.09 | `mn_petat_00_sk.ao_idle_normal_1` |
| NP_0000_00 | 카흐로스 | 25006 | [창고지기] | NPC_25006 | 236.697, 43.510, -44.421 | 181.76 | `np_0000_00_sk.ao_idle_normal_1` |
| NP_LRJP_00 | 마놀린 | 25182 | [수리공] | NPC_25182 | 227.150, 43.510, -70.168 | 223.07 | `np_lrjp_00_sk.ao_idle_normal_1` |
| NP_REOD_00 | 벨몬드 | 80023 | [골드 상점] | NPC_80023 | 237.425, 43.515, -47.568 | 161.02 | `np_reod_00_sk.ao_idle_normal_1` |
| NP_SHBR_00 | 케실리 | 25008 | [생활의 재료 교환] | NPC_SRC_25008 | 212.860, 39.580, -43.274 | 189.32 | `np_shbr_00_sk.ao_idle_normal_1` |
| NP_SHHS_00 | 젠킨스 | 25005 | [거래소 중개인] | NPC_25005 | 245.638, 44.385, -51.388 | 185.27 | `np_shhs_00_sk.ao_idle_normal_1` |
| NP_SJWD_00 | 스텔리아 | 25016 | [요리사] | NPC_SRC_25016 | 239.741, 37.144, 275.647 | 182.64 | `np_sjwd_00_sk.ao_idle_normal_1` |

### archetypeId 를 새로 만든 2명
`NPC_25008`(케실리), `NPC_25016`(스텔리아) 는 HEAD 에 이미 있는 팀 자산이고
`Npc_25008` / `Npc_25016` 모델과 `AnimSets/HM_MA02` / `HM_FE03` 를 쓴다. 이번에 구운 모델로
덮으면 팀 자산이 바뀌므로 건드리지 않고 `NPC_SRC_25008`, `NPC_SRC_25016` 을 새로 만들어
이번 모델을 가리키게 했다. 두 팀 항목은 바이트 그대로 남아 있다.

## 3. Gameplay.world.json
이전 작업이 남긴 `npc.bern.src.*` 47개와, 원본 30개 배치의 배열 서식까지 펼쳐 놓은 변경이
함께 있었다. `git checkout` 으로 HEAD(revision 548, 36배치)로 되돌린 뒤 19개만 원본과 같은
compact 서식으로 덧붙였다.

- revision 548 -> 550 (549 는 이전에 발행된 83배치본이라 건너뛰었다)
- 총 배치 36 -> 55 (collisionBox 1, npc 30+19, playerSpawn 4, triggerBox 1)
- `git diff` : 추가 210줄 / 삭제 1줄. 삭제된 1줄은 `"revision": 548,` 뿐이다.
  즉 기존 30개 배치와 playerSpawn/triggerBox/collisionBox 는 **한 줄도 바뀌지 않았다**.
- 줄끝은 작업 폴더 규약(CRLF, `core.autocrlf=true`)을 유지했다. 좌표는 원본 소수 3자리를
  그대로 썼고 float32 재직렬화를 하지 않았다. Map Tool Save 는 쓰지 않았다.

## 4. NpcCatalog.json
설치된 wmodel 바이트에서 실제 클립 이름을 읽어 그 값만 넣었다.

- idleClip 교정 12개: `ao_hm_fe02_anim`, `ao_evt2_facial_normal_1`, `ao_act_special_1`,
  `ao_act_sitdown_l1_loop_1` 등 잘못된 클립을 각 모델의 `ao_idle_normal_1` 로 바꿨다.
- 이미 정상이라 무변경 2개: `NPC_25044`, `NPC_80025`
- 신규 5개: `NPC_25030`, `NPC_25182`, `NPC_25005`, `NPC_SRC_25008`, `NPC_SRC_25016`
- 항목 수 121 -> 126. HEAD 의 77개 항목은 내용 변경 0건.
- `animationSetId` 는 전부 `null` 로 두었다. 쿠킹한 wmodel 이 클립을 자체 보유하므로
  별도 AnimSet 이 필요 없다. (팀의 `Npc_25008` 계열은 AnimSet 분리형이라 계약이 다르다.)

### 이번 19명이 쓰지 않는 나머지 카탈로그 항목
이전 작업이 추가한 44개 중 32개는 이번 배치가 참조하지 않는다. 대부분 텍스처 없는 구버전
모델(`Npc_MN_CNTF_00` 등)을 가리키며 배치에서 빠졌으므로 런타임에서 로드되지 않는다.
정리 여부는 사용자 판단 대기.

## 5. 게시
`Tools/WorldPipeline/Publish-WorldGameplay.ps1` 만 실행했다. 제품 C++ 빌드와
`Invoke-BuildAndRegression.ps1` 은 실행하지 않았다(Visual Studio 실행 중).

- Validate: `Validated BERN: 55 placements` 외 4개 world, spawn group 3종 전부 통과
- Publish: `Published BERN: 55 placements -> Server/Bin/DataFiles/World/BERN.worldbootstrap`
- bootstrap 헤더: `LOSTARK_WORLD_BOOTSTRAP 11 BERN LV_BER_BERNCASTLE 550 55 0 0 0`
- `npc.bern.src.*` 행 19개 확인. archetypeId 19종 모두 정확히 1회씩.
- `BERN.npcpresentation.json` 에는 이번 19명이 들어가지 않는다. 이 문서는 placement 별
  `idleClip`/`behavior` override 만 담으며 두 값이 모두 null 이면 제외된다(publisher:1020).
  19명의 idle 은 카탈로그 archetype 이 소유한다. 정상 동작이다.

## 6. 검증
- `git diff --check` : 경고 없음 (종료코드 0)
- 두 JSON `json.load` parse 통과
- 배포 목록 `Resource_Distribution_2026-09-18.txt` / `Copy_ResourceDistribution_2026-09-18.ps1`
  신규 생성. 파일 95개 / 264 MB(wmodel 19 + info 19 + tga 57). ps1 은 ScriptBlock 구문 검사 통과.

## 7. 사용자가 할 일
**빌드 불필요.** C++ 은 한 줄도 바꾸지 않았다. 바뀐 것은 JSON 원본과 publisher 생성물뿐이다.

1. **Server 재시작 필요.** `BERN.worldbootstrap` 이 revision 550 으로 바뀌었다.
2. Client 실행 -> Lobby -> Bern 입장.
3. 19명 위치는 크게 세 덩어리다.
   - **서쪽 (x 58~75)** : 옥타니아, 바빌루, 스트라벨, 집배원 레노엘, 그레텔, 바스커빌
   - **동쪽 (x 189~246)** : 스이에, 마놀린, 카흐로스, 케실리, 마레인, 로넬리, 윈리,
     타르코, 젠킨스, 벨몬드, 마법학자 에단, 일하는 아카테냥
   - **멀리 떨어진 1명** : 스텔리아 (x 239.7, z **+275.6**)
4. 주의: 팀이 손으로 놓은 기존 30명은 x 130~145, z -168~-20 의 좁은 구역에 있다.
   이번 19명은 원본 zone 좌표라 그 바깥에 흩어진다. 특히 스텔리아는 z 가 +275 로 다른
   18명과 400 이상 떨어져 있다. 원본 zone 11102 의 z 범위는 -173 ~ +374 이고 전체 1220배치
   중 520개가 z>0 이다. 즉 원본 데이터상 맞는 위치지만, 현재 로드되는 베른 성 맵이 그
   구역까지 덮는지는 **화면에서 확인해야 한다**. 안 보이면 맵 범위 밖인 것이고 좌표 오류가 아니다.
5. 확인할 것: 모델 형상, 텍스처(회색 덩어리가 아닌지), 미니맵 아이콘.

## 8. 이번 작업에서 내가 한 실수
1. 설치본 검증 스크립트를 처음에 ASCII 패턴으로 짜서 텍스처 참조를 0으로 잘못 셌다.
   wmodel 은 경로를 UTF-16LE 로 저장한다. 바이트 덤프로 확인한 뒤 고쳤다.
2. print 문에 em dash 를 써서 cp949 콘솔에서 `UnicodeEncodeError` 로 한 번 죽었다.
   이후 `sys.stdout.reconfigure(encoding="utf-8")` 를 먼저 호출했다.
3. `NpcCatalog.json` 꼬리 패턴을 파일 끝 개행 없이 가정해 assert 로 한 번 실패했다.
   백업에서 되돌린 뒤 개행을 포함해 다시 처리했다.
4. 배포 ps1 배열 마지막에 후행 쉼표를 남겨 PowerShell 파싱이 실패했다. 제거했다.

## 9. 확인한 것과 확인하지 않은 것
**확인함**: 설치 파일의 바이트 수준 텍스처 참조·클립 존재, 배치 diff 의 순수성(삭제 1줄),
팀 카탈로그 항목 무변경, publisher Validate/Publish 성공, bootstrap 행 19개, JSON parse,
`git diff --check`, ps1 구문.
**확인하지 않음**: 실제 화면의 모델·텍스처·미니맵 표시(사용자 전용), 스텔리아 좌표가 현재
로드되는 맵 범위 안인지, Server 재시작 후 실제 spawn 여부.
