# 2026-09-18 베른 NPC 머리 누락 결과

## 증상

베른 NPC 19명이 화면에 보이지만 목 위(얼굴·머리카락)가 비어 있었다. 옷 텍스처, 키, 서 있는 자세는 정상이었다.

## 확정 원인

몸통 PSK 하나만 구웠다. 원본 NPC는 머리가 별도 파츠다.

- `lookinfo_resolved.json` 에서 19명 중 17명이 `tier: B_multipart` 이고, `otherMeshes` 에 머리 파츠 `MN_Head_*` 를 하나씩 지정한다.
  예: 25072 집배원 레노엘 → body `MN_CNCM_00.Mesh.MN_CNCM_00_SK`, head `MN_Head_MA02_075.Mesh.MN_Head_MA02_075_SK`.
- 나머지 2명은 `A_single` 이다. 몸통 PSK 안에 머리가 들어 있다.
  - MN_CNGG_00: 점 13,409개 중 5,416개가 머리 본(`bip001-head`, `b_fc_*`)에 붙음
  - MN_PETAT_00(아카테냥): 점 6,764개 중 2,168개가 머리 본에 붙음
- 17개 몸통 PSK는 머리 본에 붙은 점이 29~123개뿐이다(목 이음새 정도). 반면 머리 PSK는 점 대부분이 머리 본에 붙는다(예 MA02_075: 3,831개 중 3,309개).
- 머리 패키지는 `out/.../raw/` 에 없었다. 1차 추출 때 저장소 밖 `C:\LostArkExtract\BernNpcSource20260917\umodel\MN_Head_*` (102개)에 뽑혀 있었다. 이후 쿠킹 스크립트는 `raw/` 만 보았다.
- 쿠킹 스크립트(`bern_cook_run.py`, `cook_textured.py`, `cook_cm.py`)는 PSK를 하나만 골랐다. 파일명에 `_fc/head/hair` 가 들어간 것은 뒤로 밀었다.
- 설치본 `.wmodel` 은 재질이 1개(몸통)뿐이었다. 예: `Npc_MN_CNCM_00` `count=1 first=mn_cncm_00_mi`.

배제한 후보:
- 스키닝 붕괴: 머리 메시 자체가 wmodel 에 없었다.
- 투명 재질: 머리 재질이 없었다.
- LOD·슬롯 스킵: 입력 PSK에 머리가 없었다.

## 팀 NPC가 머리를 붙이는 방식

한 `.wmodel` 에 몸통과 머리를 합쳐 굽는다. 카탈로그나 런타임에는 파츠를 따로 붙이는 경로가 없다.

- `NpcCatalog.json` 필드는 `archetypeId, clientPresentationId, modelAssetId, animationSetId, idleClip, actionClips, runtimeStatus, shaderProfile, cutinMovie` 가 전부다. 파츠 필드는 없다.
- `NpcPresentationAssetService.cpp` 는 body 모델 하나에 `Attach_AnimationSet` 만 한다.
- `ModelAssetConverter info` 결과
  - `Npc_25008`: `count=2 first=mn_head_ma02_041_mi`
  - `Npc_25016`: `count=2 first=mn_head_fe03_012-1_mi`
  - `Npc_Forman`: `count=2 first=mn_head_ma01_010_mi`
  - 세 개 모두 한 wmodel 에 재질이 2개다.
- `.md/TEAM/NPC_OWNER_HANDOFF.md:107` 에 `build_npc.py 아키타입 psk -> 병합 메시 .wmodel` 로 적혀 있다. 이 스크립트는 JS 개인 PC에 있어 저장소에는 없다.

## 고친 내용

저장소 쿠커(`Cook-ActorXWModel.ps1`)는 `-PskPath` 를 하나만 받는다. 그래서 입력 단계에서 몸통+머리 PSK를 하나로 합쳤다.

1. `tmp/psk_head_merge.py` — PSK 청크 병합기
   - 몸통 골격(REFSKELT 124본)을 그대로 둔다. 머리 본은 이름으로 몸통 본 인덱스에 매핑한다. 17개 모두 누락 본 0이다.
   - PNTS/VTXW/FACE/MATT/RAWWEIGHTS 에 오프셋을 붙여 이어 붙인다. 16비트 인덱스 한계를 검사한다.
   - VERTEXCOLOR, EXTRAUVS0/1 은 wedge 단위로 합친다. 없는 쪽은 흰색·0으로 채운다.
   - 머리 PSK의 기준 자세가 몸통과 다른 경우(NP_LRJP_00 목·머리 본 약 1.4cm, MN_CNCN_00 수염 본 0.2cm)가 있다. 이때는 머리 점을 `몸통 rest × 머리 rest⁻¹` 로 옮긴다. 원작은 머리를 자기 기준 자세로 그리고 몸통 본으로 움직인다(master pose).
   - 변환 규약은 목 이음새 간격으로 검증했다. ActorX 자식 회전을 켤레로 두면 NP_LRJP_00 이음새 평균 최근접 거리가 1.850 → 1.453cm 로 줄었다. 켤레를 두지 않으면 2.113cm 로 벌어졌다.
2. `tmp/head_pairs.py` — LookInfo 로 (패키지, 몸통 PSK, 머리 PSK) 짝을 찾는다.
3. `tmp/cook_head.py` — 병합 PSK로 쿠커를 호출한다.
   - 직전 규칙을 유지했다: `-NoScaleDown`, `-TextureRoot` + `-MaterialRemap/-NormalRemap/-SpecularRemap/-EmissiveRemap`, `*_nor_ani.psa` + `-RequiredActionName idle_normal_1`, `-BakeFrameRate 30`, `-AllowBoneOrderRemap`, 스테이징은 저장소 밖, 패키지 폴더를 미리 만들지 않음.
   - 병합 PSK 파일명은 몸통 PSK와 같게 두었다. 그래서 클립 이름(`<몸통psk>.ao_*`)이 기존 카탈로그 `idleClip` 과 그대로 맞는다. `NP_SHBR_00` 에서 기존 설치본과 클립 이름이 동일함을 diff 로 확인했다.
   - 머리 재질 텍스처는 머리 패키지의 `.mat` 사이드카에서 찾는다. MN_CNCN_00 머리 `mn_head_ma02_079_mi` 는 원본 사이드카가 `mn_head_ma02_078_d/n/s` 를 가리킨다. 원본이 원래 그렇다.
4. 17개 쿠킹 결과: 성공 17, 실패 0. 워커 3개를 병렬로 돌렸다.
5. 설치
   - 17개를 `Client/Bin/Resources/Character/NPC/Npc_<PKG>/` 에 덮어썼다. 대상은 wmodel, info.txt, textures 이다.
   - 이전 설치본 17개는 `tmp/npc_head_fix_backup/` 에 백업했다.
   - MN_CNGG_00, MN_PETAT_00 은 원래 머리가 몸통에 있어서 그대로 두었다.
6. 배포 목록 갱신
   - `Resource_Distribution_2026-09-18.txt` / `Copy_ResourceDistribution_2026-09-18.ps1` 을 147파일, 331MB 로 갱신했다.
   - BOM 없는 UTF-8, CRLF 전 줄(153/153, 165/165), ps1 구문 오류 0을 확인했다.

`Data/Actors/NpcCatalog.json`, `Gameplay.world.json`, C++, 조명, 네비 파일은 이번에 수정하지 않았다. 수정 시각은 각각 06:42, 10:53 이고 이번 작업 전이다.

## 19/19 설치본 검증 (바이트)

| 패키지 | 재질 | 텍스처 참조/누락 | idle | 머리 점 | 몸통 최고점 → 병합 최고점(cm) |
|---|---|---|---|---|---|
| MN_CNAB_00 | 2 | 6/0 | O | 1,600 | 97.8 → 110.4 |
| MN_CNBF_00 | 2 | 6/0 | O | 4,632 | 120.4 → 147.9 |
| MN_CNCM_00 | 2 | 6/0 | O | 3,831 | 121.0 → 149.1 |
| MN_CNCN_00 | 2 | 6/0 | O | 2,914 | 123.1 → 134.6 |
| MN_CNDM_00 | 2 | 6/0 | O | 2,828 | 121.0 → 149.1 |
| MN_CNGG_00 | 2 | 6/0 | O | 몸통에 포함 | — |
| MN_CNLF_00 | 2 | 7/0 | O | 3,546 | 118.5 → 130.7 |
| MN_CNLP_00 | 2 | 6/0 | O | 2,182 | 123.3 → 132.3 |
| MN_CNMF_00 | 2 | 6/0 | O | 2,212 | 120.5 → 139.5 |
| MN_CNTW_00 | 2 | 6/0 | O | 2,512 | 120.4 → 142.3 |
| MN_CNWM_00 | 2 | 6/0 | O | 1,605 | 122.5 → 150.3 |
| MN_ISRY_00 | 2 | 6/0 | O | 3,013 | 120.4 → 142.3 |
| MN_PETAT_00 | 1 | 4/0 | O | 몸통에 포함 | — |
| NP_0000_00 | 2 | 4/0 | O | 917 | 126.2 → 144.7 |
| NP_LRJP_00 | 2 | 5/0 | O | 1,314 | 96.5 → 106.9 |
| NP_REOD_00 | 2 | 4/0 | O | 1,022 | 118.4 → 132.5 |
| NP_SHBR_00 | 2 | 6/0 | O | 1,232 | 121.0 → 133.0 |
| NP_SHHS_00 | 2 | 6/0 | O | 987 | 128.0 → 139.4 |
| NP_SJWD_00 | 2 | 5/0 | O | 1,276 | 114.2 → 127.8 |

- 17개 모두 팀 NPC와 같은 구성(한 wmodel, 재질 2개, 두 번째가 머리)이 됐다. 병합 후 최고점이 몸통보다 10~28cm 높다. 머리가 목 위에 붙었다는 뜻이다.
- 표의 높이는 PSK 원 좌표(cm)의 z 범위다. 런타임 렌더 키는 이 wmodel 에 NPC 공통 0.0001 배율과 골격 변환을 적용한 값이다. 이번에는 따로 재지 않았다.

## 머리를 붙이지 못한 패키지

없다. MN_CNGG_00 과 MN_PETAT_00 은 원본부터 머리가 몸통 PSK 안에 있다.

## 사용자가 할 일

- 빌드는 필요 없다. C++를 바꾸지 않았다.
- Server 재시작도 필요 없다. 배치와 카탈로그는 그대로다. Client 만 다시 실행해 베른에 들어가면 된다. Client 가 켜져 있었다면 모델은 로드할 때 읽히므로 재진입이 필요하다.
- 확인 위치는 직전 안내와 같다(원본 좌표, F6 자유 카메라).
  - 동쪽 카흐로스 (236.7, 43.5, −44.4), 벨몬드 (237.4, 43.5, −47.6)
  - 서쪽 바빌루 (62.1, 42.3, −93.1), 옥타니아 (58.7, 42.3, −93.7)
  - 남쪽 윈리 (195.0, 49.1, −146.6), 에단 (189.8, 49.1, −146.7)
- 화면으로만 판정할 수 있는 것
  - 얼굴이 목에 맞게 붙었는지
  - 얼굴·머리카락 텍스처
  - 대기 동작 중 머리가 몸과 같이 움직이는지

## 확인하지 못한 것

- 실제 화면 렌더(사용자 몫)
- 원작 머리 재질의 추가 입력(`_cm` 색 마스크, `_m` 마스크)은 연결하지 않았다. 쿠커 슬롯에 없다. 머리카락·피부 색조가 원본과 다를 수 있다. 팀 NPC도 같은 조건이다(`Npc_25008` specular 빈칸).
- 머리 기준 자세를 옮긴 NP_LRJP_00(최대 1.466cm)과 MN_CNCN_00(0.176cm)이 화면에서 목 이음새가 자연스러운지

## 실수

- 첫 조사에서 `raw/` 만 보고 머리 패키지가 없다고 판단할 뻔했다. 1차 추출이 저장소 밖 umodel 폴더에 있었다.
- 검증 스크립트가 변환기에 한글 절대경로를 넘겨 `failed to read file` 로 멈췄다. 설치는 끝난 뒤였다. 저장소 루트 기준 상대경로로 고쳐 다시 검증했다.
- export_log 조회에 `grep -P` 를 써서 로케일 오류가 났다. 필요한 정보는 다른 경로로 확인했다.
