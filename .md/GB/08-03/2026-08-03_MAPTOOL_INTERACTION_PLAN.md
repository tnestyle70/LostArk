# 2026-08-03 MapTool 상호작용 확장 계획 (참조 영상 04~08 패리티 + 발전형)

참조 이미지: `C:\Users\user\Desktop\툴\04~08_MapTool_*.png`
— 배치/트랜스폼 메뉴(04), 애니메이션 메뉴(05), Convenience(복제·사각 인스턴싱·
AlphaBlend 전환)(06), Collider Tool(구 부모 + OBB 자식)(07), QuadTree Tool(08).

작성 모드: STRUCTURE_FIRST. 이 문서는 내부적으로 슬라이스 A~D로 나뉜다.
**한 번에 한 슬라이스만 구현한다.** 착수 순서는 A → B → C → D.

## 0. 현재 체크포인트 (공통 실측)

우리 CMapTool이 이미 갖춘 것: 카탈로그 기반 메시 목록/armed 클릭 배치, Hierarchy
목록 선택 + Inspector 수치 편집(배치·회전·스케일 DragFloat), 저장(authoring 전용)
/재로드, 네비 bake/paint/blocker, 카메라 패널, `ConsumesWorldLeftMouse` 입력 중재.

참조 영상 대비 없는 것(전부 grep/실측 검증):

- 월드 클릭으로 **기존 배치물 선택** 불가(픽킹은 위치만 반환, 오브젝트 식별 없음)
- 트랜스폼 **기즈모** 없음(ImGuizmo 등 라이브러리 부재)
- **Clone/사각 범위 인스턴싱** 없음(armed 연속 배치만 존재)
- **콜라이더 어소링** 없음 — Engine `CCollider`(SPHERE/AABB/OBB + 디버그 렌더)는
  완비돼 있으나 Client 참조 0건(휴면), 저장 계약에도 콜라이더 필드 없음
- **QuadTree 툴** 없음(현재 컬링은 frustum + static batch)
- 배치물 **애니메이션 미리보기** 없음(맵 모델은 전부 MODEL::NONANIM admission)
- Undo/Redo, 멀티 선택, 인월드 gameplay 마커 없음
- **Grid Show**(뷰포트 참조 그리드 토글), **CameraSpeed 런타임 조절**(현재
  fSpeedPerSec는 레벨 init 고정, 툴 UI 없음), **PickingCell Index 표시**(커서
  아래 네비 셀 인덱스, 셀 밖 -1) 없음 — 영상 04·07 Basic Information 구성 요소

공통 저장 계약(불변): 시각 배치는 `Data/Maps/Authoring/<AreaId>/<AreaId>.mapplacements`
(LOSTARK_MAP_PLACEMENTS 2, uint64 placementId 도메인 분할), 런타임 교체는
`Publish-MapAuthoring.ps1`만. 신규 저장 필드/문서는 같은 원칙을 따른다.

---

## 슬라이스 A — 월드 클릭 선택 + 선택 시각화

참조 영상 04의 "Picking Object: BernCastle_Bridge01" 기능.

### A-1. 완료 조건

1. MAP_ASSETS 모드에서 armed 상태가 아닐 때, 월드 좌클릭으로 커서 아래 배치물이
   선택되고 Hierarchy/Inspector가 그 배치로 동기화된다.
2. 선택된 배치물의 로컬 바운즈 와이어 박스가 표시된다.
3. Basic Information 상당(픽 좌표, 선택 오브젝트, 카메라 pos/look, 커서 아래
   네비 셀 PickingCell Index — 기존 Try_PickNavigationCell 경로 재사용, 셀 밖
   -1)이 패널에 보인다.
4. Grid Show(뷰포트 참조 그리드 토글 — Engine DebugDraw의 DrawGrid 헬퍼 +
   기존 PrimitiveBatch 패턴 재사용)와 CameraSpeed 런타임 조절(CCamera_Free에
   이동 속도 setter 추가 — 현재 fSpeedPerSec는 레벨 init 고정)이 같은 패널에서
   동작한다. 셋 다 슬라이스 A의 픽킹/패널 경로와 같은 저비용 항목이라 A 범위다.

### A-2. 접근 결정 — 오브젝트 ID 렌더 타겟 (권장안)

두 가지 안을 비교한 뒤 권장안을 명시한다. 최종 선택은 사용자.

```text
안 1) ID 렌더 타겟(권장): Target_PickPos와 동일한 레시피로 Target_PickId를
  MRT_GameObject에 추가하고, NONBLEND 메시 셰이더 4종이 SV_TARGET에 placementId를
  기록. CPicking과 동일한 staging readback으로 커서 픽셀의 ID를 읽는다.
  근거: 기존 픽킹 시스템의 확장이라 "두 번째 픽킹 시스템 금지" 제약과 정합.
  비용: 셰이더 4종 + 상수 1개 + 엔진 타겟 등록 + readback 경로.
  주의: uint64 placementId를 그대로 담을 포맷이 없다 -> 런타임 전용 세션 인덱스
  (엔트리 배열 index+1)를 R32_UINT에 기록하고, CPU에서 다시 placementId로
  역매핑한다. 저장 ID로는 절대 쓰지 않는다(세션 한정 값).
안 2) CPU 레이 vs 배치 바운즈: Try_PickPlacementPosition의 unproject 레이를
  전 배치 AABB와 교차. 코드가 Client에 갇히는 장점, 겹친 오브젝트/정밀도 한계와
  매 클릭 전 배치 순회 비용이 단점. 배치 수천 개 지역에서 프레임 스파이크 위험.
```

### A-3. 파일 책임 지도 (안 1 기준)

```text
수정 Engine/Private/Renderer.cpp: Target_PickId(R32_UINT) 등록 + MRT_GameObject에
  6번째 타겟 추가. 클리어 색 0(=선택 없음).
수정 Client/Bin/ShaderFiles/Shader_VtxMeshBinary.hlsl 외 3종(VtxAnimMesh,
  VtxAnimMeshBinary, VtxMeshMapInstance): 출력 구조체에 uint SV_TARGET 추가,
  상수 g_iPickId(인스턴싱 셰이더는 인스턴스 스트림 확장 대신 배치 단위 상수 +
  배치 내 인스턴스 식별은 2단계 판정으로 위임 — 아래 A-4 흐름 참조).
수정 Engine/Private/Picking.cpp 또는 신설 Client 판독기: PickPos와 같은
  Copy_RT_Resource + staging Map. 권장은 CPicking에 Picking_Id(uint32_t&) 추가
  (같은 파일이 같은 기법을 두 타겟에 적용 — 응집).
수정 Client/Private/MapTool.cpp: Update_WorldInteraction에 "armed 아님 + 좌클릭"
  분기 -> 세션 인덱스 -> placementId -> m_iSelectedPlacementId 갱신.
수정 Client/Private/MapPlacementRuntime.cpp(경계 주의): 커밋 시 배치 엔트리마다
  세션 인덱스를 부여/보관하는 조회 테이블. 정적 배치(인스턴싱) 엔트리는
  "배치 오브젝트 세션 인덱스 + 인스턴스 로컬 판정"의 2단계.
```

### A-4. CPP 흐름 (클릭 → 선택)

```text
Update_WorldInteraction 진입
-> ImGui WantCaptureMouse면 반환 (기존과 동일)
-> armed 상태면 기존 배치 로직 (변경 없음)
-> 좌클릭 에지 && armed 아님
   -> CGameInstance::Picking_Id(id) — 커서 픽셀 ID readback
   -> id == 0 -> 선택 해제
   -> 표준 오브젝트: 세션 인덱스 -> MAP_RUNTIME_PLACED_ENTRY -> placementId
   -> 정적 배치 오브젝트: 배치가 반환되면, 배치 내부 인스턴스들의 월드 바운즈에
      CPU 레이 교차로 최근접 인스턴스 확정(배치 인스턴스 수는 유한하고 프러스텀
      가시 목록이 이미 있음) -> FMapStaticInstance.PlacementId
-> m_iSelectedPlacementId 갱신 -> Inspector 자동 동기화(기존 멤버 재사용)
선택 시각화: 기존 NAVIGATION_RENDER_RESOURCES PrimitiveBatch 패턴으로 선택
  배치의 월드 바운즈 와이어 박스를 매 프레임 드로우.
```

### A-5. 검증

```text
브레이크포인트: Picking_Id 반환값 / 역매핑된 placementId / Inspector 표시 ID 일치.
수동: 겹친 두 오브젝트에서 앞 오브젝트가 선택되는가, 빈 공간 클릭 시 해제되는가,
  armed 중에는 선택이 발동하지 않는가.
엔진 헤더 변경 시 UpdateLib 후 Client. 실측: UpdateLib는 Engine/Bin/ShaderFiles
  (Shader_Cell/Shader_Deferred + Engine_Shader_Defines.hlsli)만 EngineSDK/hlsl을
  거쳐 Client/Bin/ShaderFiles로 복사한다. 수정 대상 메시 셰이더 4종
  (VtxMeshBinary/VtxAnimMesh/VtxAnimMeshBinary/VtxMeshMapInstance)은
  Client/Bin/ShaderFiles 정본이라 UpdateLib 복사 대상이 아니며 덮어써지지 않는다.
  이 슬라이스에서 UpdateLib가 필요한 변경은 Engine 공개 헤더(Target_PickId
  등록/Picking_Id 판독 API)뿐이다. 단 Shader_Cell/Shader_Deferred는 UpdateLib가
  덮어쓰므로 "셰이더 전체가 Client/Bin 소유"라는 일반화는 하지 않는다.
```

---

## 슬라이스 B — 트랜스폼 기즈모

참조 영상 04·05의 축 화살표/평면 핸들 드래그.

### B-1. 결정 사항 (구현 전 사용자 확정 필요)

```text
외부 라이브러리 ImGuizmo 도입 여부.
  도입 안: 검증된 이동/회전/스케일 + 스냅. Engine/External에 서드파티 추가는
    팀 합의 필요(빌드/라이선스/vcxproj 등록 — MIT, 소스 2파일 수준).
  자체 구현 안: 이동 3축+3평면 핸들만 우선(회전/스케일은 Inspector 수치 유지).
    A 슬라이스의 unproject 레이 + 축/평면 교차 수학으로 구현 가능. 범위는 작게.
권장: 1차는 자체 이동 기즈모(범위 절제), 회전/스케일 수요가 검증되면 ImGuizmo
  도입을 팀 안건으로 올린다. 이유: 서드파티 추가는 merge/빌드 영향이 커서
  단독 결정 부적합(AGENTS 팀 경계).
```

### B-2. 흐름과 경계 (자체 이동 기즈모 기준)

```text
전제: 슬라이스 A 완료(선택 존재).
드래그 시작: 기즈모 핸들 히트 판정(레이 vs 축 원기둥/평면 사각) -> 드래그 축 고정
  -> ConsumesWorldLeftMouse가 드래그 중 true를 유지하도록 조건 확장
드래그 중: 마우스 레이와 (축 직선|평면)의 최근접점 -> 시작점 대비 델타
  -> MAP_PLACEMENT_RECORD.position 갱신
  -> 표준 오브젝트: CMapAssetObject::Set_PlacementTransform
  -> 배치 인스턴스: CMapStaticBatchObject::Update_Instance (미러 패리티 이동
     마이그레이션은 기존 Inspector 코드 경로 재사용)
드래그 끝: 문서 dirty 마크. 저장은 기존 Save 버튼 경로 그대로.
그리기: PrimitiveBatch 와이어(축 3색). ImGui 오버레이가 아닌 월드 공간 드로우.
```

### B-3. 검증

드래그 전후 record.position 델타 == 화면 델타의 월드 환산치(로그), 저장→재로드
후 위치 보존, 배치 인스턴스 미러 케이스 마이그레이션 유지.

---

## 슬라이스 C — Clone + 사각 범위 인스턴싱 (Convenience 패리티)

참조 영상 06: Object_Clone / Rect Instance Mode(개수·범위·회전) / AlphaBlend 전환.

### C-1. 완료 조건

1. 선택 배치 Clone 버튼: 동일 assetId·transform 복제 + 새 editor placementId 발급
   (`Allocate_EditorPlacementId` 재사용) + 소폭 오프셋.
2. 사각 인스턴싱: 중심/범위/개수/회전 입력 → N개 배치를 생성해 문서에 추가
   (참조 영상의 랜덤 산포. 시드 고정 옵션으로 재현성 확보).
3. AlphaBlend 전환 패리티는 **저장 계약 확장이 필요**: 현재 renderMode는
   `.mapassets` 카탈로그(에셋 단위) 소유라 배치 단위 전환이 없다. 이번 슬라이스는
   에셋 단위 전환(카탈로그 편집)을 범위 밖으로 두고, 필요 시 별도 계획으로
   `MAP_PLACEMENT_RECORD`에 renderMode override 필드(v3 승격)를 다룬다.
   근거: 배치 문서 v2 포맷 변경은 publisher/파서/오디트 픽스처 동시 수정이
   필요한 계약 변경이라 상호작용 슬라이스에 섞지 않는다.

### C-2. 흐름

```text
Clone: 선택 record 복사 -> 새 placementId/sourcePlacementId 발급 -> 문서 Add
  -> Stage_PlacementRuntime(단건) -> 실패 시 문서 롤백(기존 단건 배치 경로 재사용)
Rect Instancing: 입력 검증(개수 상한 — 문서 MAX와 카탈로그 배치 상한 이내)
  -> 루프: 위치 = 중심 + 시드 난수(범위 내), yaw = 지정 회전 or 난수
  -> 각 반복은 Clone과 동일 경로 -> 중간 실패 시 이번 작업 생성분 전체 롤백
     (부분 커밋 금지 — 생성한 placementId 목록을 기록해 두고 역순 제거)
```

### C-3. 검증

생성 N개 == 문서 count 증가, 실패 주입(카탈로그에 없는 assetId) 시 전체 롤백,
저장 후 publish 스크립트 Validate 통과.

---

## 슬라이스 D — 콜라이더 어소링 (Collider Tool 패리티)

참조 영상 07: 부모 구 콜라이더 + 자식 OBB, 오프셋/반지름 편집, 와이어 표시.

### D-1. 소유권/저장 결정 (구현 전 확정)

```text
용도 확정이 먼저다. 현재 제품 충돌 판정은 서버 권위(이동은 navgrid, 전투 판정은
  Server)라 클라 콜라이더는 툴/프리뷰·연출 용도다. 무엇에 쓸지(카메라 충돌?
  이펙트 차폐? 향후 서버 export?)를 먼저 한 줄로 확정하고 시작한다.
저장 위치: 카탈로그-배치 분리 원칙에 따라
  에셋 단위 콜라이더 정의 -> 신규 Data/Maps/Colliders/<AreaId>.json
  (schema lostark.map-colliders, assetId 키, 부모 SPHERE + 자식 OBB 배열,
  각 항목 stable colliderId) — 배치 단위 오버라이드는 후속.
런타임 소비자가 아직 없으므로 publisher는 만들지 않는다(소비자 없는 인터페이스
  금지 — AI 하네스 규칙). 툴 표시용 로드만 구현하고, 서버/게임플레이 소비가
  생기는 시점에 publisher+오디트를 같은 슬라이스로 추가한다.
```

### D-2. 파일 책임 지도

```text
신설 Client/Public·Private/MapColliderDocument.{h,cpp}: 위 JSON 파스/검증/직렬화.
  CameraCutDocument와 동일 계층·동일 실패 경로.
수정 Client/Private/MapTool.cpp: Inspector에 Collider 섹션(선택 배치의 assetId
  기준): Add_Collider(부모 구), Add_Child_Collider(OBB), 오프셋/크기/회전 편집.
콜라이더 인스턴스: Engine CCollider를 LEVEL::STATIC에 프로토타입 등록
  (CCollider::Create(device, context, COLLIDER::SPHERE/OBB) — 최초의 Client
  소비자가 된다). 툴이 편집 중인 에셋의 콜라이더만 Clone해 들고, 선택 배치의
  CTransform 월드로 매 프레임 CCollider::Update.
표시: CGameInstance::Add_DebugComponent(콜라이더) — Render_Debug가 매 프레임
  리스트를 비우므로 매 프레임 재등록(기존 Character 네비 디버그와 동일 사용법).
```

주의(실측 근거): `CBounding_AABB::Update`는 회전을 의도적으로 버린다.
회전 박스는 반드시 OBB(vDegree)로 저장/편집한다 — AABB에 회전 UI를 붙이면
조용히 무시된다.

### D-3. 검증

편집값 저장→재로드 왕복 일치, 와이어가 배치 이동(슬라이스 B)에 추종, Debug 전용
컴파일 확인, `96.DataFiles` 등록 + ProjectAudit 통과.

---

## 후속 단계 (별도 계획으로 분리, 이번 문서 범위 밖)

1. **애니메이션 배치물 미리보기(영상 05)**: 맵 admission이 전부 NONANIM이므로
   "애니메이션 있는 모델" 배치 계약 자체(카탈로그 확장 + ANIM admission +
   스킨드 바운즈 부재 문제)가 선행 설계 대상이다. CModel의 재생 API
   (Start/Stop/Set_AnimTrackPosition/blend)는 이미 완비 — 갭은 데이터 계약 쪽.
2. **QuadTree 컬링(영상 08)**: 도입 전 측정 게이트 — 현 frustum+batch에서 Bern
   전체 씬 프로파일(기존 Profiler 오버레이)로 컬링 병목이 증명될 때만 설계한다.
   참조 영상의 기능을 이식하는 것이 목적이 아니라 성능이 목적이므로, 병목이
   없으면 만들지 않는다.
3. **Undo/Redo**: 문서 편집 명령의 커맨드 스택. 슬라이스 A~C의 편집 경로가
   전부 "record 변경 + 런타임 반영" 형태로 수렴한 뒤에 얹는다.
4. **배치 단위 renderMode override**(C-1의 v3 계약 변경).

## 프로젝트 설정·등록 (슬라이스 공통)

- MapTool은 기존 툴이므로 허브/시나리오 등록 변경 없음(dev.map.active 재사용).
- 신설 파일만 vcxproj/filters 등록. 신규 Data JSON은 96.DataFiles 등록.
- 슬라이스 D의 `Data/Maps/Colliders/<AreaId>.json`은 새 Area별 optional layer이고
  콜라이더 어소링은 MapTool 지원 범위 확장이므로,
  `.md/TEAM/AREA_DATA_LAYER_GUIDE.md`의 레이어 표와 "MapTool이 지금 편집하는 것"
  절 갱신을 같은 변경 단위에 포함한다(선택적 레이어, 툴 표시 전용 로드, 런타임
  소비자가 생기기 전에는 publisher 없음).
- Engine 수정(A의 타겟 추가, D의 CCollider 프로토타입은 Client 등록이라 무관)이
  있는 슬라이스는 UpdateLib → Client 순서.
- 각 슬라이스 완료 시 Server+Client 실행 → Lobby Test → F1 MapTool 수동 검증 + ProjectAudit +
  `Publish-MapAuthoring.ps1 -Mode Validate`(저장 계약 접촉 시).

## 사용자가 먼저 작성할 범위

슬라이스 A의 판독 경로(CPicking 확장)와 MapTool 선택 분기부터. 셰이더 4종의
SV_TARGET 추가는 구조 확정 후 진행(렌더링 담당 영역이므로 직접 작성 권장).
