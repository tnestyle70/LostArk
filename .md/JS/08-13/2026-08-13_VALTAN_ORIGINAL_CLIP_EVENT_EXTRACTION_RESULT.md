# 2026-08-13 발탄 원작 클립·이벤트 전량 추출 RESULT

작성자: JS · 2026-08-13 · branch `feature/lancemaster-charge-aim`

PLAN: `2026-08-13_VALTAN_ORIGINAL_CLIP_EVENT_EXTRACTION_PLAN.md`
인계 배경: `.md/GB/08-10/2026-08-10_COMBAT_COLLIDER_SPAWN_VERTICAL_SLICE_RESULT.md` §5,
`.md/GB/08-06/2026-08-06_VALTAN_PATTERN_BALANCE_HUD_IMPLEMENTATION_RESULT.md` §5.

## 1. 구현 완료

- 발탄 신원 확정: `EFTable_Npc` PK 480007 "마수군단장 발탄" = `MN_RPBF_01`.
  원작 Action은 `XmlData/Action/MN_RPBF_00.loa`에 있으며 `ValtanEncounter.json`의
  `sourceActionIds` 63개가 전부 이 파일에서 해석된다(총 109 액션).
- 원작 애니셋 전량 쿠킹: `Character/Valtan/AnimSets/MN_RPBF_01_AnimSet.wmodel`
  (146클립 = `MN_RPBF_00_Ani` 132 + `Evt2_Ani` 14, 선언=임포트, 중복 0).
  기존 body 27클립은 146의 진부분집합임을 대조로 확인(`body - animset = []`).
- attach 방식: 팀장이 쿠킹한 `MN_RPBF_01.wmodel`은 그대로 두고
  `CModel::Attach_AnimationSet`으로 붙인다. skeletonHash는 본 테이블 이름 순서 FNV이므로
  body의 노드 구조(`RootNode/mesh(아마추어)/…/mesh.001`)를 애니셋 쿠킹에서 재현했다
  (`build_npc_animset.py`에 `--armature-name/--carrier-name` 옵션 추가).
  `Compare-Skeletons` 87/87 ALIGNED, `validate_wmodel` OK.
- 클립 이름 규약: attach 충돌 방지를 위해 애니셋 클립은 `mesh_` 접두사를 유지한다.
  기존 presentation 6클립은 body 내장 이름 유지, 신규 패턴 바인딩은 `mesh_*` 참조.
- 클라 배선:
  - `BossCatalog.json` `animationSetId`를 placeholder에서 실제 ResourceId로 교체.
  - `CActorCatalog` boss 파싱에 `IsResourceId(animationSetId)` 검증 추가.
  - `CValtanPresentationAssetService::Ensure_Prototypes` — body 생성 후 애니셋 로드·attach,
    실패 시 fail-closed.
  - `CLoader` 저작 프리뷰 경로(Animation Tool의 보스 프리뷰가 사용) — 애니셋 파일이
    존재하면 attach, 파일이 있는데 attach 실패면 fail-closed, 파일이 없으면 기존
    body-only 프리뷰 유지(리소스 미배포 PC 배려).
- Reference 문서: `Data/Animation/Reference/Valtan/`
  - `.clipmap` 108클립 ↔ 액션, `.clipseq` 체인 265(슬롯 2001),
    `.animnotify` 1,138행(EFFECT 817/SOUND 258/SHAKE 63), `.animevents` 878건(툴 v3).
  - 추출은 기존 `extract_action_loa.py` 무수정 재사용(PREFIX `mesh_`).
    loa 토큰 158 중 146 밖 37개는 같은 파일의 다른 변형 모델(02 계열 등) 참조.

## 2. 검증

- 자동: 애니셋 `validate_wmodel` OK, `Compare-Skeletons` 87/87 ALIGNED, Client Debug 빌드
  오류 0, `BossCatalog.json` parse 경로는 기존 로더 검증 통과(실행 확인).
- 수동(사용자, 2026-08-13): Animation Tool 발탄 프리뷰에서 173클립(27+146) 표출·재생 확인.

## 3. 남은 것 (후속)

- **HIT 타이밍 원본 좌표 미해결** — 몬스터는 clip notify에 hit이 없고(`ParticleHit` 0건),
  `EFTable_Skill`에 동일 키(420601~) row가 있으나 range뿐. 플레이어의
  `SkillEffect PK = variant*10+n` 규약은 몬스터 대역에 적용되지 않았다(빈 skilltiming은
  배포하지 않음). 다음 후보: loa `ToggleCollision`(161)·`EmitTriggerSignal`(30) notify,
  7자리 서브스킬(4206724~)과 SkillEffect의 다른 조인 경로.
- 패턴별 클립 바인딩(`ValtanEncounter` stage ↔ `mesh_*` 클립)과 stage duration 교정 —
  이번에 확보한 `.clipseq`/`.animnotify`가 재료.
- 무기 자체 애니셋(WP_MN_RPBF_00_Ani), 2페이즈 모델(MN_RPBF_02)은 범위 밖.
- 리소스 배포: `AnimSets/MN_RPBF_01_AnimSet.wmodel`은 현재 이 PC에만 있다. 머지 시
  팀장 물리 폴더 배포 필요(없는 PC는 제품 발탄 fail-closed, 프리뷰는 27클립 폴백).

## 4. 산출물 위치

- 스테이징: `_export_valtan_g` (MN_RPBF_00/01, WP_MN_RPBF_00/01, -groups)
- 애니셋: `Client/Bin/Resources/Character/Valtan/AnimSets/MN_RPBF_01_AnimSet.wmodel` (110MB FBX → 쿠킹본)
- 참조 문서: `Data/Animation/Reference/Valtan/Valtan.{clipmap,clipseq,animnotify,animevents}`
