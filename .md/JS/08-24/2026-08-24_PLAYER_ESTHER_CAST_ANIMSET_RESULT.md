# 2026-08-24 플레이어 4직업 에스더 호출 캐스팅 애니메이션 애니셋 부착 RESULT

작성자: JS · 2026-08-24

## 1. 목표

원작의 에스더 스킬 사용 시 손을 들어 호출하는 캐스팅 애니메이션(`Act_EstherSkill_1`,
`COMMONACTION.loa` 공용 액션)을 4직업(창술사·워로드·도화가·차원술사) body 모델에서
재생 가능하게 한다. body 재쿠킹 없이 기존 `CModel::Attach_AnimationSet` 경로
(NPC/발탄 애니셋과 동일 계약)로 클립 1개짜리 애니셋 `.wmodel`을 부착한다.

## 2. 원본 실측

- 클립은 직업 psa가 아니라 **체형(가족) 공용 애니셋**에 있다. 직업 자체 psa
  (`pc_flm_00_ani` 223개 등)에는 없음을 스캔으로 확정.

| 직업 | 가족 베이스 패키지 | 클립 소스 psa | 클립명(원본) |
|---|---|---|---|
| 창술사 | `PC_FT_00` (무도가 여) | `pc_ft_00_ani.psa` | `act_estherskill_1` |
| 워로드 | `PC_WR_00` (전사 남) | `pc_wr_00_ani.psa` | `act_estherskill_1` |
| 도화가 | `PC_SP_00` (스페셜리스트 여) | `pc_sp_00_ani.psa` | `act_estherskill_1` |
| 차원술사 | `PC_SP_M_00` (스페셜리스트 남) | `pc_sp_m_00_ani.psa` | `act_estherskill_1` |

- 클립 길이 45프레임 @30fps (1.5초). 원작 액션은 `FX_R_Hand` 소켓 파티클
  (`fx_cm_01.Etc.Par_D_Esther_call_11`, 시작 후 ~0.35초)과 사운드
  (`PC_COMMON_ESTHER.PC_Common_FX_Active1/2`)를 함께 건다 — 이펙트/사운드는 이번
  범위 밖(이펙트 담당 인계 참고 정보).

## 3. 쿠킹 (buildScript, 저장소 밖)

`build_npc_animset.py --clips act_estherskill_1`로 rig psk + 가족 psa에서 단일 클립
애니셋 FBX를 굽고 `ModelAssetConverter --no-auto-textures`로 변환했다.
rig·armature명·캐리어 메시명은 각 body wmodel의 본 테이블을 Dump-Bones로 실측해
동일 해시가 나오도록 맞췄다.

| 직업 | rig psk | --armature-name | --carrier-name | 쿠킹 클립명 |
|---|---|---|---|---|
| 창술사 | `pc_flm_00_upper_sk_loc_int.psk` (221본) | `flm` | `pc_ft_00_sk.001` | `flm_act_estherskill_1` |
| 워로드 | `pc_wr_00_sk.psk` (215본) | `wgl` | `pc_wr_00_sk.001` | `wgl_act_estherskill_1` |
| 도화가 | `pc_sp_00_sk.psk` (236본) | `sdm` | `pc_sp_00_sk.001` | `sdm_act_estherskill_1` |
| 차원술사 | `pc_sp_m_00_sk.psk` (222본) | `pc_sp_m_00_sk` | `pc_sp_m_00_sk.001` | `pc_sp_m_00_sk_act_estherskill_1` |

함정 2건:

- 창술사 body의 armature 본은 가족 베이스 `pc_ft_00_sk`(207본)가 아니라
  `pc_flm_00_upper_sk_loc_int`(221본)이다. 처음 207본 rig로 구워 본 수 불일치 →
  221본 master로 재쿠킹.
- 차원술사 body는 ActorX intake 쿠킹이라 armature 노드 scale=1.0(다른 3직업은 100),
  admission scale 0.01(다른 3직업 0.0001)로 상쇄된다. **본-로컬** rest/키 값 단위는
  4직업 동일(cm)이라 애니셋 키는 그대로 호환된다.

## 4. 배치·데이터·코드

- 리소스(로컬 배치, 각 ~0.6MB): `Character/<Class>/AnimSets/<Class>_EstherAnimSet.wmodel`
  (LanceMaster / Warlord / Artist / DimensionMaster)
- `Data/Actors/CharacterCatalog.json` formatVersion 2 → 3: entry에
  `animationSetModel`(null 또는 `Character/...wmodel` ResourceId) 필수 추가.
  4직업은 경로, Gunslinger/Slayer는 null.
- `CActorCatalog::ParseCharacters`: v3 강제, `animationSetModel` nullable 파싱
  (NPC `animationSetId` 패턴과 동일, IsResourceId 검증).
- `CPlayableCharacterAssetService::Ensure_Prototypes`: body `CModel` 생성 직후
  `animationSetModel`이 있으면 애니셋 로드 + `Attach_AnimationSet` 후 stage.
  실패는 해당 class admission 전체 fail-closed (기존 staging 계약 그대로).
  장비 파츠는 body 본 팔레트를 바인딩하므로 body 부착만으로 충분하다.

## 5. 자동 검증 (실행함)

- 본 테이블 대조(compare_attach.py, 스크래치패드): 4쌍 모두 이름·순서·parent 완전
  일치, ANIM trailer skeletonHash 일치 → ATTACHABLE.
  (flm `636dcae030842ac1`, wgl `e2118af94d7dcfed`, sdm `934b182ab1ae68b1`,
  sp_m `43588e0806b2ee7f`)
- `validate_wmodel` 4개 전부 RESULT: OK (본 수 224/218/239/225 = body와 동일).
- `CharacterCatalog.json` parse OK.
- Client x64 Debug 빌드(Engine·Shared 참조 포함) exit 0, 오류 0 —
  `Client\Bin\Debug\Client.exe` 링크 성공. `git diff --check` 통과.

## 6. 수동 검증 (사용자)

- 게임 내 클립 재생 육안 확인은 사용자 몫. 에스더 스킬 바인딩 전이라도 F1
  Animation/Effect 툴의 클립 목록에서 `*_act_estherskill_1` 표출 여부로 확인 가능.

## 7. 남은 것

- 에스더 스킬 skillId ↔ 이 클립의 `.skillbindings.json` 연결(스킬 슬라이스에서).
- 손 이펙트·사운드 복원(§2 참조 정보, 이펙트 담당).
- `Client/Bin/Resources` 물리 폴더는 팀장 배포 채널로 공유 필요(애니셋 4개는 현재
  이 PC에만 있음).

원본은 Smilegate 저작물 — psa/psk 스테이징과 FBX 중간 산출물은 Git에 올리지 않는다.
