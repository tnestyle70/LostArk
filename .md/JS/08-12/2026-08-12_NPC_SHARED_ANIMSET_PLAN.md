# NPC 체형별 공유 애니메이션 셋 — PLAN (설계 확정, 구현 전)

작성자: JS · 2026-08-12 · branch `feature/npc-placement-tool` 후속

목표: NPC `.wmodel`은 스켈레톤+메시만 갖고, 애니메이션은 체형(아키타입)별 공유
리소스 1벌로 분리한다. 툴에서 NPC를 선택하면 그 체형의 전체 클립이 열람되고
아무 클립이나 골라 재생/사용할 수 있다. NPC 가짓수가 수십 종으로 늘어도
애니 메모리는 "체형 수 × 1벌"로 고정된다.

## 0. 실측 근거 (2026-08-12 확인)

- `MODEL_ANIMATION_CHANNEL_DATA`가 `boneNameHash`(u64)와 `resolvedBoneIndex`를
  둘 다 보존한다 (`ModelAssetData.h:147-154`).
- 클립마다 `skeletonHash`(u64)가 있다 (`ModelAssetData.h:169`). 값은 본 테이블
  순서대로 `nameHash`를 FNV-1a로 접은 것 (`WSkeletonReader.cpp:176-182`).
- `WAnimationReader.cpp:256`이 **이미** 클립 skeletonHash ≠ 모델 skeletonHash면
  디코드를 거부한다. 즉 "skeletonHash가 같다 = 본 테이블(이름·순서)이 같다 =
  `resolvedBoneIndex`가 그대로 유효하다"가 성립한다. **이름 재결속 없이 인덱스
  그대로 외부 클립을 붙일 수 있는 호환성 게이트가 포맷에 이미 있다.**
- `CAnimation` 복사는 `CChannel`을 `shared_ptr`로 공유한다 (`Animation.h:36`,
  암시적 복사 생성자). 키프레임 메모리는 붙인 모델 수와 무관하게 1벌.
- 같은 체형의 신본 바디·머리·아키타입 rig는 rest가 비트 단위로 동일하고 124본이다
  (`.md/JS/2026-08-02_LOSTARK_NPC_GROUP_RESOLUTION_RESULT.md` 2장). 같은 소스
  rig에서 같은 순서로 구우면 skeletonHash가 일치한다.
- 현재 4종 클립 실측: Beda 4 / Schmidt 4 / Aylara ~34 / Forman ~46. 4종 교집합은
  `npc_idle_normal_1`(+`_1_1`)뿐. 공유 셋 도입 전까지는 "카탈로그 idleClip이 그
  모델에 실존하는가"가 데이터 계약의 전부다.
- 쿠킹 스크립트는 저장소 밖 `C:\Users\95jus\Desktop\buildScript\`의
  `npc_lookinfo.py / build_npc.py / cook_npc.py` (Blender 5.0 번들 python).

## 1. 설계

```text
Resources/Character/NPC/AnimSets/HM_MA01/HM_MA01.wmodel   ← 스켈레톤 + 클립 전부, 메시 0
Resources/Character/NPC/AnimSets/HM_FE04/HM_FE04.wmodel
Resources/Character/NPC/Npc_Forman/Npc_Forman.wmodel      ← 스켈레톤 + 메시, 클립 0
...

NpcCatalog.json (formatVersion 2)
  entry += "animationSetId": "Character/NPC/AnimSets/HM_MA01/HM_MA01.wmodel" | null
  (null이면 베다처럼 자기 클립 내장 모델)

런타임:
  CNpcPresentationAssetService
    ├─ body 모델 프로토타입 (archetype당 1)
    └─ animset 모델 프로토타입 (체형당 1, 여러 NPC가 공유)
  CNpc 초기화 시 body CModel에 animset의 CAnimation을 attach
    - 게이트: body.skeletonHash == animset.skeletonHash, 다르면 fail-closed
    - CAnimation은 clone(재생 상태 인스턴스별), CChannel 키프레임은 공유
```

툴은 추가 작업이 거의 없다 — 이미 구현한 Preview Clip 콤보가 모델의
`Get_NumAnimations()`를 열거하므로, attach가 끝난 모델에서는 체형 전체 클립이
자동으로 나온다.

## 2. G 분해

| G | 내용 | 검증 |
|---|---|---|
| G1 | Engine: `CModel`이 자기 skeletonHash 보존 + `Attach_AnimationSet(const CModel& animSet)` (hash 불일치 fail-closed, 클립 이름 충돌 거부). 메시 0개 wmodel 로드 허용 확인 | Engine+UpdateLib+Client 빌드, 기존 레벨 회귀 |
| G2 | cook: `cook_npc.py`에 `--animset`(스켈레톤+클립, 메시 제외) / `--mesh-only`(클립 제외) 모드. 같은 rig 소스에서 skeletonHash 일치 보장, 불일치 시 쿠킹 오류 | 산출물 skeletonHash 대조 스크립트 |
| G3 | HM_MA01·HM_FE04 animset 쿠킹 + 4종 바디 mesh-only 재쿠킹 (Beda는 자기 클립 유지 가능) | validate_wmodel, 해시 대조 |
| G4 | NpcCatalog v2(`animationSetId`) + 파서 + `CNpcPresentationAssetService`가 animset 프로토타입 로드·attach | Client 빌드, Bern/에디터에서 4종 표시 |
| G5 | 툴/제품 확인: 에디터 미리보기 콤보에 체형 전체 클립 표출, 제품 Bern snapshot 경로 회귀 | 실행 확인, ProjectAudit |

순서 근거: G1이 팀 공유 Engine 공개 헤더를 건드리므로 가장 먼저 확정하고
UpdateLib 공지가 필요하다. G2·G3는 저장소 밖 스크립트와 리소스 물리 폴더 작업이라
코드 리뷰 대상이 아니고, G4·G5가 저장소 계약을 닫는다.

## 3. 이번에 결정해 둔 것

- 새 `.wanim` 포맷을 만들지 않는다. animset도 `.wmodel`(메시 0개)로 구워
  `CModel::Create` 단일 입구를 유지한다 (AGENTS: 두 번째 모델 런타임 금지).
- attach는 additive다. 클립 내장 모델(현 4종)은 attach 없이도 그대로 동작하므로
  G3 재쿠킹 전까지 제품이 깨지지 않는다.
- per-placement 클립 저장은 여전히 별도 슬라이스다. 이 작업은 "고를 수 있는
  클립의 공급"을 해결하고, "고른 클립의 저장·복제"는 placement schema 확장에서 한다.

## 4. 미확정 (구현 중 실측 필요)

- `CModel::Initialize`가 메시 0개를 실제로 허용하는지 (거부 시 G1에서 허용 분기).
- animset attach 시 `Get_AnimationName` 인덱스 공간이 내장 클립과 합쳐지는 방식
  (내장 뒤에 이어 붙이기 + 이름 충돌 거부 예정).
- HM_MA01 전체 클립 수·용량 (psa 원본 기준 수십 개, 쿠킹 후 크기 실측 필요).
