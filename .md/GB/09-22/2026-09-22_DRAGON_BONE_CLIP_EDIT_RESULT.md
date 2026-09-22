# 용 본 조정과 활강·상승 클립 결과

## G00. 반영한 편집과 정본

고대의 바다의 실제 91본/9 native clip 모델을 사용해 아래 두 clip을
`Data/Animation/Authored/Vehicle_9523/Vehicle_9523.boneclips.json`에 저장했다.

| clip | 길이 | 자세와 재생 |
|---|---:|---|
| authored.dragon.glide | 2000ms | npc_sk_look 2500ms의 펼친 날개 자세 유지 |
| authored.dragon.ascent | 967ms | npc_sk_look 1800~2766.6667ms 날갯짓, 마지막120ms 시작 자세 연결 |

두 clip은 bip001의 local Z를 rest로 고정한 제자리 시각 애니메이션이다. 기존3개
authored.example clip은 보존했고 VehicleCatalog의 E flightWindow와 Server 비행 binding은
바꾸지 않았다. 실제 상승 고도는 기존 Server가 소유한다. 원본 native clip을 덮지 않았다.

적용 직전에 Client/Server가 실행 중이지 않고 대상 JSON이 Git-clean임을 확인했다.
fresh Load → Create_AncientSeaFlightStudies → 기존 Save의 CAS/backup/atomic replace →
fresh Load/Compile/Install 순서로 저장했다. `.previous`, `.writer.lock`은 Save가 만든
sidecar이며 소스 커밋 대상이 아니다.

이전 SHA256은 `f3c87e239aa2e63570857444a4b339f71a5d571bb6021e57c4eac1ed21d5f9a7`,
저장 후는 `710c94d61e6e6108f9c491019bab059ed788e8d756fdcb7391cea6d243e25cfe`다.

## G01. 본 편집 작업 흐름

기존 BoneAnimationWorkbench에 아래 조작을 연결했다.

- Source in/out과 속도로 새 authored clip 생성, 지정 원본 시각의 held pose 생성.
- 본 계층·검색, 실제 모델의 골격/선택 본 표시, local TRS drag의 즉시 pose preview.
- Set key로 확정, Cancel pose로 복귀. 미확정 pose가 있으면 다른 본·클립·시간·문서
  편집을 막아 선택 변경으로 자세를 잃지 않는다. explicit Discard는 저장본으로 복귀한다.
- Undo/Redo, 시작·끝 이동, 선택 본의 시작 key를 끝으로 복사.
- baked/rest clip의 전체 key retime. source segment는 기존 trim 편집을 사용한다.
- Save 이후 기존 Animation Resources 새로고침. 새 bone 명령과 새 sequence Play가
  같은 모델의 재생을 동시에 소유하지 않도록 CharacterActionWorkbench에 연결했다.

본 overlay는 실제 combined * BoneRoot * view * projection을 사용한다. combined의
preScale을 중복 적용하지 않는다. 본 선택은 패널에서 하며 scene 클릭 picking, IK,
3D rotation gizmo는 이번 구현에 없다. 기존 ImGui render가 gameplay 입력 처리 이후여서
동일 프레임 click 차단을 보장하지 못하므로 overlay는 수동적인 표시/hover만 한다.

새 `Client/Private/BoneAnimationWorkbench_Viewport.cpp`를 Client project/filter의 기존
`03. Tools/05. Sequencer`에 등록했다. schema와 Engine public API는 변경하지 않았다.

## G02. 실행한 검증

| 검증 | 결과 |
|---|---|
| 실제 WModel 기반 생성·Compile·Install·native 보존·실패 rollback·JSON pose roundtrip | PASS |
| 91본 retime 축소500/400ms 및2배 확대, 시간순서·양끝·실패 보존 포함 | 30,625 assertion PASS. 비율 대응 pose 최대 local matrix 오차 2.98023e-07 |
| 정본 Save 후 fresh Load/Compile/Install | 392 assertion PASS. 기존3예시 pose 오차0, 설치된 반복 끝점 최대 차이2.38419e-07 |
| BoneAnimationDocument 실제 TU compile/link | PASS |
| BoneAnimationWorkbench, Viewport, CharacterActionWorkbench Debug 개별 /c compile | PASS. 기존 SDK C4819 경고만 관찰 |
| 수정 JSON parse, project/filter XML parse 및 변경 범위 git diff --check | PASS |

문서·정본 저장 증거는 `out/DragonBoneEdit20260922/document-result.log`,
`document-compile.log`, `install-result.log`, `install-receipt.json`이다.
Core/Viewport/Integration TU compile 증거는 `out/DragonBoneClip20260922/` 아래에 있다.
수치 검사는 실제 설치 skeleton과 animation을 사용했고 Client·UI·GPU 화면은 실행하지 않았다.

## G03. 제품 빌드와 사용자 확인

공유 폴더의 재질 복원 작업이 Product Debug 빌드를 소유한다. 해당 작업이 실행 중이어서
별도의 Product 빌드를 동시에 실행하지 않았고, 신규 Viewport TU가 project load 이후
추가됐을 수 있으므로 현재 등록을 포함한 Client 정상 증분 Build가 필요함을 전달했다.
이 문서 작성 시점에는 새 실행 파일의 최종 링크 성공을 아직 확인하지 않았다.

새 Debug Client 빌드 후 F1의 Action Workbench → Character → Mounts /
VEHICLE_ANCIENT_SEA의 action을 선택한다. Details → Bone & Animation Edit에서
Authored clip을 authored.dragon.glide 또는 authored.dragon.ascent로 선택하고
Play bone preview를 누른다. Show skeleton in scene과 Bone hierarchy에서 날개
`bip001-l-upperarm`, `bip001-r-upperarm`, 목/꼬리 본을 선택해 조정한다.
키를 확정한 뒤 Save Bone Clips로 저장한다.

두 clip은 준비된 튜닝 출발점이다. 실제 날개의 형태·속도·반복 인상과 overlay 위치의
화면 판정은 사용자 확인으로 남는다. code compile, 정본 저장, 수치 재생과 화면 완성을
구분한다. 기존 대규모 dirty 변경을 보존했으며 본 작업만의 commit/push/PR은 하지 않았다.
