# NPC 체형별 공유 애니메이션 셋 — RESULT

작성자: JS · 2026-08-12 · branch `feature/npc-placement-tool`

PLAN: `2026-08-12_NPC_SHARED_ANIMSET_PLAN.md`

## 1. 구현 완료

- Engine `CModel`: 로드 시 `skeletonHash` 보관(`Get_SkeletonHash`),
  `Attach_AnimationSet(const CModel&)` — ANIM 타입·본 개수·0이 아닌 동일 해시를 전부
  검증하고, 클립 이름 충돌은 전체 선검증 후 통째 거부(부분 커밋 없음). 붙은 클립은
  `CAnimation::Clone`이라 키프레임(`CChannel`)은 애니셋 1벌을 공유한다.
  PLAN의 `allowMeshlessAnimationSet`은 캐리어 메시 방식 채택으로 불필요해져 넣지 않았다.
- 쿠킹(저장소 밖 `C:\Users\95jus\Desktop\buildScript\`):
  - `build_npc_animset.py` 신규 — 아키타입 rig psk + psa들 → 애니셋 FBX.
    컨버터가 메시 0개 FBX를 거부해("assimp failed") 루트 본에 스키닝된 삼각형 1개
    캐리어 메시(`npc_body`)를 포함한다.
  - `build_npc.py` — 병합 메시 이름을 `npc_<ID>`에서 상수 `npc_body`로 변경.
    컨버터 본 테이블이 메시 노드 이름을 포함해서, 이름이 다르면 같은 rig여도
    skeletonHash가 어긋난다. 이것이 이번 작업의 핵심 함정이었다.
- 리소스: `Character/NPC/AnimSets/HM_MA01/HM_MA01.wmodel`(45클립, 25.6MB) 신규.
  포먼(0.39MB)·슈미트(0.65MB) mesh-only 재쿠킹 배포. skeletonHash 3개 일치
  (`0xa1531f5b29e7a6ac`), `validate_wmodel` 전부 OK.
- `NpcCatalog.json` formatVersion 2 — entry에 `animationSetId`(nullable) 추가.
  포먼·슈미트 = HM_MA01 참조, 베다(자체 rig)·아일라라(옷깃 graft 본 결정 대기) = null.
- `CActorCatalog::ParseNpcs` v2 파싱(6필드 정확 일치, null 또는 ResourceId).
- `CNpcPresentationAssetService`: 애니셋을 레벨당 1회
  `Prototype_Component_Model_AnimSet_<stem>`으로 등록하고 body 프로토타입 생성 시
  attach. 같은 애니셋의 두 번째 archetype은 등록본을 재사용한다. attach 실패는
  해당 archetype 전체 fail-closed.

## 2. 자동 검증 (실행함)

- Engine → UpdateLib → Client Debug 빌드 오류 0.
- `Publish-WorldGameplay.ps1` 전 Area 통과 (v2 카탈로그).
- skeletonHash 대조 스크립트(scratchpad `compare_skeleton.py`) 3개 일치.
- `validate_wmodel` 애니셋·포먼·슈미트 OK. `git diff --check` 통과.

## 3. 수동 검증 (사용자 확인 완료)

- MapTool에서 슈미트 배치 → Preview Clip 콤보에 45개 클립 표출·재생 확인 (2026-08-12).

## 4. 남은 것

- 아일라라: 옷깃 graft 본 1개 포기 여부 결정 후 HM_FE04 애니셋 + mesh-only 전환.
- HM_MA01의 다른 `ani_*` 그룹(스테이징에 다수) 추가 시 클립 이름 충돌 dedupe 필요.
- Bern 제품 경로(Server snapshot) 육안 확인은 아직 안 함 — 코드 경로는 에디터
  미리보기와 동일한 서비스 attach를 사용한다.
- 팀 공유: Engine public header 변경이라 머지 후 각자 UpdateLib 재실행 필요.
