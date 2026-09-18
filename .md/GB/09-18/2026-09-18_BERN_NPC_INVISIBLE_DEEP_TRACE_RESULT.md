# 베른 19명 NPC 미표시 — 크기·렌더링·지오메트리 심층 추적 RESULT

작성일: 2026-09-18
대상: `Data/Worlds/LV_BER_BERNCASTLE/Gameplay.world.json`의 `npc.bern.src.*` 19개,
`Client/Bin/Resources/Character/NPC/Npc_<PKG>/` 19개 패키지

## 0. 사용자 관찰

F6 자유 카메라로 베른 맵 전체를 돌아다녀도 NPC가 하나도 보이지 않았다. 앞선 두 진단
("네비 격자 밖이라 걸어갈 수 없었다", "데이터는 전부 정상")은 이 관찰을 설명하지 못했다.

## 1. 중단된 fork의 부분 적용 상태

직전 fork는 "코드 3곳과 카탈로그를 바이트 단위로 패치합니다" 직후 중단됐다.

- 09:02:18(`ClientReplication.cpp` 진단 12줄) 이후 저장소 C++/데이터 파일 수정 0건
  (`Client/Private`, `Client/Public`, `Data/Actors`, `Data/Worlds`, `Server/Private`, `Shared`,
  `Engine` 전수 mtime 확인).
- `Data/Actors/NpcCatalog.json` JSON parse 성공, 126항목, HEAD 77항목 변경 0건.
- 결론: 중단된 fork는 아무 파일도 쓰지 않았다. 되돌리거나 완성할 반쪽 편집 없음.

## 2. 확정 원인 — 모델 단위가 100배 작다

### 2.1 코드 근거

`Client/Private/NpcPresentationAssetService.cpp:206-208`이 모든 NPC 몸체에 archetype과 무관한
고정 preTransform을 건다.

```cpp
const matrix_t preTransform =
    XMMatrixScaling(0.0001f, 0.0001f, 0.0001f) *
    XMMatrixRotationY(XMConvertToRadians(-90.f));
```

`Engine/Private/Model.cpp:649-660`에서 이 행렬은 루트 본 combined에 곱해진다. NPC 경로에는
카탈로그 스케일 필드가 없다(`NpcCatalog.json` 항목 키는 `archetypeId, clientPresentationId,
modelAssetId, animationSetId, idleClip, runtimeStatus` 여섯 개뿐이고 `CNpc`의 스케일은 Debug
전용 `Set_DebugPresentationScale`뿐이다). 따라서 모든 NPC wmodel은 같은 단위여야 한다.

### 2.2 측정값 (wmodel 스켈레톤 rest 변환을 합성한 본 월드 높이)

| 모델 | armature 노드 스케일 | 본 월드 높이(단위) | × 0.0001 = 렌더 키 |
|---|---|---|---|
| 팀 `Npc_25001` | npc ×100 | 16889.4 | 1.689 m |
| 팀 `Npc_25184` | npc ×100 | 15181.3 | 1.518 m |
| 팀 `Npc_Forman` | npc ×100 | 15978.2 | 1.598 m |
| 팀 `Npc_Beda` | npc ×100 | 14305.2 | 1.431 m |
| 팀 `Npc_11592` | npc ×100 | 14613.8 | 1.461 m |
| 신규 `Npc_MN_CNAB_00` | ×100 | 127.5 | **0.013 m** |
| 신규 `Npc_MN_CNBF_00` | ×100 | 149.9 | **0.015 m** |
| 신규 `Npc_NP_REOD_00` | ×100 | 151.5 | **0.015 m** |
| 신규 `Npc_MN_PETAT_00` | ×100 | 92.1 | **0.009 m** |
| 신규 `Npc_NP_SJWD_00` | ×100 | 146.1 | **0.015 m** |

inverse-bind 이동량도 같은 비율이다. 팀 `Npc_25184` 중앙값 72.0 / 최대 151.8, 신규
`Npc_MN_CNBF_00` 중앙값 0.74 / 최대 1.50. 두 쪽 모두 127본 리그로 구조는 같고 단위만 정확히
100배 다르다. 설치된 19명은 1~1.5 cm 크기로 그려졌으므로 자유 카메라로도 보이지 않았다.

### 2.3 왜 100배인가

`Tools/ActorXAssetCooker/Cook-ActorXWModel.ps1:87-91`:

```
# Disable the ActorX importer scale-down. Use it when the target body
# model keeps source (centimetre) translation units ...
[switch]$NoScaleDown
```

팀 NPC는 cm 단위로 구워져 있다. 이번 19개는 `-NoScaleDown` 없이 구워져 ActorX importer가
PSK를 m 단위로 줄였다(`*.actorx.json`의 `settings.scale_down = True`).

## 3. 배제한 후보 (측정 근거)

| 영역 | 확인 | 결과 |
|---|---|---|
| B 프로토타입 등록 | `NpcPresentationAssetService.cpp:172-292` | archetype별 지연 등록, 카탈로그 조회. 고정 목록 없음 |
| B 개수 상한 | `ActorCatalog.cpp` | NPC 배열 상한 없음. 128 제한은 `modelMaterialOverrides` 배열용 |
| B 가시성 | `Npc.h:394`, `Npc.cpp:686-700` | `m_bPresentationVisible` 기본 true, NONBLEND 등록. 숨기는 호출자는 쿠크 컷신 2곳뿐 |
| B idle 클립 | `Npc.cpp:67-76` | 이름이 틀려도 첫 클립으로 폴백. 신규 19개 모두 카탈로그 idle 이름이 wmodel 안에 존재 |
| B 최근 커밋 | `git show --stat c3f3360b` | NPC·Replication·Level_Bern 파일 변경 없음 |
| C 맵 범위 | 베른 shard `.mapplacements` 50,017행 | x −1337.6~1017.1, z −946.5~476.3. 원본 좌표 범위를 덮는다 |
| C 원본 좌표 지오메트리 | 19명 원본 좌표 반경 5 m | 배치 50~247개, 발밑 이하 30~233개(도로·계단·바닥 메시). 허공 아님 |
| C 팀 기준선 | 팀 NPC 30명 반경 5 m | 배치 47~120개 |
| D Server 상한 | `Server/Private`, `Shared/Public` grep | 월드 엔티티 수·거리 컬링 상한 없음. NPC는 네비 투영 대상 아님(선행 RESULT) |

"저장소 베른 맵이 원본 맵과 크기가 다르다"는 가설은 C의 수치로 기각된다. 원본 좌표 주변에
실제 맵 지오메트리가 있다.

## 4. 조치

### 4.1 cm 단위 재쿠킹

- 스크립트: `C:\Users\USER\.claude\jobs\45c8ba77\tmp\cook_cm.py`
  (`cook_textured.py`와 입력·PSA 선택·텍스처 remap이 같고 `-NoScaleDown`만 추가)
- 스테이징: `C:\Users\USER\.claude\jobs\45c8ba77\tmp\bern_cooked_cm\Npc_<PKG>\`
- 시험 1개 `MN_CNAB_00`: 본 높이 12747.3 × 0.0001 = 1.27 m, 텍스처 참조 3개 전부 존재,
  `mn_cnab_00_sk.ao_idle_normal_1` 존재. 클립 이름이 이전과 같아 카탈로그 수정 불필요.

### 4.2 원본 좌표 복원

원인이 좌표가 아니었으므로 직전 fork가 네비 통로로 옮긴 19명(rev 551)을 원본 DeployData
좌표(`bern19.source-coordinates.json`)로 되돌렸다.

- 19개 `position` 줄과 `revision` 551 → 552만 바이트 편집. CRLF 957줄 유지.
- HEAD 36개 배치 변경 0건, 신규 19개 원본 좌표·yaw 불일치 0건, `git diff --check` 통과.
- `Publish-WorldGameplay.ps1 -Mode Validate` → `-Mode Publish` 통과.
  `BERN.worldbootstrap` 헤더 `LOSTARK_WORLD_BOOTSTRAP 11 BERN LV_BER_BERNCASTLE 552 55 0 0 0`,
  `npc.bern.src.*` 19행.
- 베른 navigation은 x 123.99~148.99 통로만 덮으므로 원본 좌표의 19명에게 걸어서는 갈 수 없다.
  F6 자유 카메라로 확인한다. 미니맵 위치 확인도 원본 좌표여야 의미가 있다.

### 4.3 19개 재쿠킹·설치 결과

`cook_cm.py`로 19개 전부 성공(실패 0). 스테이징과 설치본 모두 같은 검사를 통과했다.
검사: 스켈레톤 본 월드 높이 × 0.0001(렌더 키), UTF-16LE `textures/*.tga` 참조가 옆
`textures/`에 전부 존재, 배치가 실제로 쓰는 archetype의 `idleClip` 이름이 wmodel 안에 존재.

| 패키지 | 렌더 키 | 텍스처 참조 | 누락 | idle |
|---|---|---|---|---|
| MN_CNAB_00 | 1.27 m | 3 | 0 | 있음 |
| MN_CNBF_00 | 1.50 m | 3 | 0 | 있음 |
| MN_CNCM_00 | 1.51 m | 3 | 0 | 있음 |
| MN_CNCN_00 | 1.51 m | 3 | 0 | 있음 |
| MN_CNDM_00 | 1.51 m | 3 | 0 | 있음 |
| MN_CNGG_00 | 1.10 m | 6 | 0 | 있음 |
| MN_CNLF_00 | 1.50 m | 4 | 0 | 있음 |
| MN_CNLP_00 | 1.51 m | 3 | 0 | 있음 |
| MN_CNMF_00 | 1.50 m | 3 | 0 | 있음 |
| MN_CNTW_00 | 1.50 m | 3 | 0 | 있음 |
| MN_CNWM_00 | 1.51 m | 3 | 0 | 있음 |
| MN_ISRY_00 | 1.50 m | 3 | 0 | 있음 |
| MN_PETAT_00 | 0.92 m | 4 | 0 | 있음 |
| NP_0000_00 | 1.60 m | 2 | 0 | 있음 |
| NP_LRJP_00 | 1.27 m | 2 | 0 | 있음 |
| NP_REOD_00 | 1.52 m | 2 | 0 | 있음 |
| NP_SHBR_00 | 1.51 m | 3 | 0 | 있음 |
| NP_SHHS_00 | 1.60 m | 2 | 0 | 있음 |
| NP_SJWD_00 | 1.46 m | 2 | 0 | 있음 |

팀 NPC 기준선 1.43~1.69 m와 같은 범위다. MN_PETAT_00(아카테냥)과 MN_CNGG_00은 원래 작은
체형이다. 클립 이름이 이전 쿠킹과 같아 `NpcCatalog.json`은 수정하지 않았다.

- 설치 위치: `Client/Bin/Resources/Character/NPC/Npc_<PKG>/`(`.wmodel`, `.wmodel.info.txt`, `textures/`)
- 이전 m 단위 설치본 백업: `C:\Users\USER\.claude\jobs\45c8ba77\tmp\npc_m_unit_backup\`
- 배포 목록 갱신: `Resource_Distribution_2026-09-18.txt`(95파일 252 MB),
  `Copy_ResourceDistribution_2026-09-18.ps1`(구문 오류 0, CRLF 유지)

## 5. 변경 파일

| 파일 | 변경 |
|---|---|
| `Data/Worlds/LV_BER_BERNCASTLE/Gameplay.world.json` | 19개 `position`을 원본 좌표로, revision 551 → 552 |
| `Server/Bin/DataFiles/World/BERN.worldbootstrap` 외 publisher 출력 | 게시 산출물 |
| `Client/Bin/Resources/Character/NPC/Npc_<PKG>/` 19개 | cm 단위 wmodel·info·textures로 교체(Git 비추적) |
| `Resource_Distribution_2026-09-18.txt`, `Copy_ResourceDistribution_2026-09-18.ps1` | 머리말·합계 |
| C++ | 변경 없음 |

`git diff --check` 통과. `NpcCatalog.json` 무변경(126항목, HEAD 77항목 변경 0).

## 6. 사용자 확인 절차

1. Client 빌드는 필요 없다. 이미 빌드했다면 `[NpcPresentation]` 진단이 들어간 Client를 그대로 쓴다.
2. Server를 재시작한다(worldbootstrap revision 552).
3. Client 재실행 → Lobby → Bern 입장.
4. 19명은 원본 좌표에 있다. 베른 navigation이 x 124~149 통로만 덮어 걸어서는 못 간다.
   F6 자유 카메라로 다음 위치를 본다(클라이언트 좌표 x, y, z).
   - 동쪽 창고 구역: 카흐로스 (236.7, 43.5, −44.4), 벨몬드 (237.4, 43.5, −47.6), 젠킨스 (245.6, 44.4, −51.4)
   - 서쪽 거래소 구역: 바빌루 (62.1, 42.3, −93.1), 옥타니아 (58.7, 42.3, −93.7)
   - 남쪽 수리 구역: 윈리 (195.0, 49.1, −146.6), 마법학자 에단 (189.8, 49.1, −146.7), 일하는 아카테냥 (188.4, 49.1, −144.6)
5. 모델이 사람 크기로 서 있는지, 텍스처가 입혀졌는지, 미니맵 아이콘 위치가 맞는지 확인한다.
6. 안 보이면 VS 출력 창의 `[NpcPresentation]` 줄(19명 중 실패한 것의 placement·archetype·실패 단계)을 전달한다.

## 7. 못 배제한 것과 다음 측정

| 후보 | 배제 못 한 이유 | 갈리는 측정 |
|---|---|---|
| 런타임 모델 로드 실패(`CModel::Create`) | 실행해야만 나는 오류 | `[NpcPresentation] ... model prototype preparation failed` 출력 여부 |
| 텍스처 로드 실패로 회색 표시 | 파일은 전부 존재, TGA 디코드는 실행 판정 | 화면에서 회색 몸체인지 |
| 애니메이션 본 적용 오류로 형태 붕괴 | 정적 검사로는 skinning 결과를 못 봄 | 화면에서 자세가 무너지는지. 무너지면 PSA와 PSK 본 순서(`-AllowBoneOrderRemap`) 재확인 |

## 8. 이번 작업의 실수

1. 첫 cm 쿠킹 스크립트 생성에서 치환 앵커를 따옴표 포함 문자열로 잘못 잡아 assert로 멈췄다(Write 도구로 치환 스크립트를 다시 만듦).
2. 원본 좌표 판정 스크립트가 `bern19.source-coordinates.json` 구조(`placements` dict)를 확인하지 않고 list로 가정해 한 번 실패했다.
3. 설치 전 검증에서 카탈로그를 `modelAssetId`로 색인해, 같은 모델을 공유하는 미사용 옛 항목의 idle 이름을 읽고 8개를 NG로 오판했다. 배치 archetype 기준으로 고쳐 19/19 통과를 확인했다.
4. 설치 때 `.wmodel.info.txt`를 빠뜨려 배포 목록과 불일치할 뻔했다. 목록 갱신 단계에서 복사했다.
5. 배포 ps1을 다시 쓰면서 CRLF가 LF로 바뀌었다. 바이트로 되돌려 CRLF 113줄, lone LF 0.
