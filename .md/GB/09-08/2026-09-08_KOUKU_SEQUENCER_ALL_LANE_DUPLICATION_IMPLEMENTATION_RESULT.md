# 쿠크 Sequencer 전체 lane 선택·구간 복제 결과

## G00. 구현 상태

상세 Pattern Sequencer의 Stage/Animation/Logic/Summon/World/Scene Profile과
Effect/Collider/Sound/Camera/Light를 동일 선택 집합으로 연결했다. 모든 lane의 Ctrl+click,
빈 영역 드래그, Ctrl+drag 추가 선택, 선택 표시, Detail focus를 처리한다.
드래그는 시간폭 전체를 감싼 박스를 선택하여 한 타격 구간에 긴 master/World가 섞이지 않게 한다.
Duplicate 버튼과 Ctrl+D, Delete는 같은 batch 명령을 호출한다. Stage reorder는 Stage/Animation
선택만 허용한다. 새 Sound 정의나 두 번째 타임라인 실행 경로는 만들지 않았다.

혼합 구간은 상대 시간을 보존해 뒤 Stage 경계에 삽입하고, 이후 원본 lane은 같은 길이만큼 민다.
삽입점을 가로지르는 기존 Logic/World 등의 수명도 연장한다. lane-only 복제는 선택 끝에 붙이며
필요하면 마지막 Stage를 늘린다. Stage/Animation만 복제하는 기존 packing/trim은 유지한다.
BossMotion 도중 삽입처럼 기존 두 점 이동 계약으로 표현할 수 없는 요청은 원본을 보존하고 거절한다.

Collider와 소유 Logic의 전체 Collider 집합, World와 동반 Effect를 중복 없이 복제한다.
새 Stage/action/occurrence/region ID를 만들고 복제본끼리 참조를 바꾼다. 공유 Logic 정의의 typed
내부 참조를 바꿔야 하는 경우에만 정의와 결과를 따로 복사한다. 기존 카드/검색 master와 외부
motion/후속 Pattern 참조는 유지한다. World 일괄 삭제는 기존 단일 삭제처럼 해당 World에 붙은
모든 presentation을 함께 제거한다. 모든 변경은 한 candidate로 검증·commit하며 실패 시 원본을 유지한다.

수정 파일은 기존 Workbench H/CPP, 기존 native contract test CPP, Server GameRoom.cpp다.
신규 C++ 파일·schema·project/filter 등록은 없다. 사용자의 실제 패턴에 타격 횟수를 추가하지 않았다.

## G01. REJECTED 진단과 수정

사용자 첨부 이미지의 `expected Product source revision is not active`는 좌표 값 검사가 아니라
요청 Product source revision과 Server의 시작 시 로드한 catalog revision 불일치다.
Composition revision 166, Encounter sourceRevision 166, Gameplay bootstrap의
KOUKUSAYDONPRODUCTREVISION 166과 16:29:15 publish 성공을 확인했다.
당시 Server/Client는 16:20:42부터 실행 중이었다. 활성 번호 160은 직접 로그로 입증하지 않았으므로
확정값으로 기록하지 않는다. 새 데이터는 Server 재시작 후 Client 재접속으로 소비한다.

GameRoom의 기존 reject 경계에서 요청 번호·Server 활성 번호·Server 재시작과 Client 재접속
안내를 포함하도록 문구만 바꿨다. packet/schema와 revision 검사는 유지한다. runtime hot reload를
추가하거나 활성 버전 검사를 우회하지 않는다. 이미지 아래 Waiting 문구는 이전 제출 상태 캐시이며
현재 대기 상태의 증거가 아니다. 그 별도 UI 캐시는 이번 변경 범위에 포함하지 않았다.

## G02. 검증

- 최신 reverse Logic→Collider 소유권 포함 native `--kouku-preview-transport-contract` PASS.
  로그: `out/KoukuJoker20260908/harness-all-lane-preview-transport-20260908-164119.log`.
  혼합 lane 복제, 원본 시각 이동, master 수명 연장, COW 참조, stable ID, 위치/회전/크기 보존,
  Logic-only 전체 Collider, lane-only/끝 시간 확장, Save/Reload 및 invalid/overflow 원본 보존을 확인했다.
- 같은 기존 harness의 최소 refs=false 컴파일·링크 PASS.
- Composition/Product JSON parse 및 revision 166 일치, 관련 project XML 4개 parse PASS.
- 변경 경로의 scoped `git diff --check` PASS.
- Debug Product `Engine → Shared → Server → Client` 컴파일·링크·SDK/shader/runtime DLL 배포 PASS.
  16:46:17 완료, 249028ms, missingRuntimeInputs 없음.
  receipt: `out/BuildPipeline/runs/20260908T074617840Z-debug-product.json`.
  log: `out/KoukuJoker20260908/all-lane-product166-build.log`.
- 빌드 전후 Composition/Encounter/Gameplay.bootstrap byte 동일성을 확인했다. 최신 사용자 revision
  166과 저장 좌표를 수정하거나 재publish하지 않았다. Client/Server를 에이전트가 실행하지 않았다.
- 기존 전체 editor mode는 60초 제한에서 종료됐으며 PASS로 기록하지 않는다. 추가로 기존
  TimelineControls만 재사용한 focused 실행도 90초 제한으로 종료됐다. 임시 fixture에서는
  mixed 복제·ID 중복 제거·이동 왕복·stale/foreign ID·overflow 검사를 지나 noncontiguous fixture
  저장까지 진행했지만 전체 완료 전이므로 해당 추가 실행은 PASS가 아니다. 추가 재사용 호출은
  제거하여 기존 editor mode에 유지하고, 위 16:41 전용 all-lane/Logic-only PASS 범위를 구분한다.
  로그: `out/KoukuJoker20260908/harness-all-lane-preview-transport-20260908-164551.log`.
  에이전트는 이 추가 검사 시간을 늘려 반복하지 않았다. Product 실행 준비는 완료했다.

## G03. 사용자 확인

Client/UI를 에이전트가 실행하거나 조작하지 않았다. 사용자 화면에서 한 타격 구간의 Stage와 필요한
lane을 감싸 드래그한 뒤 선택 표시를 확인하고 Duplicate 또는 Ctrl+D를 누른다. 긴 카드 World와
검색 master는 한 타격 복제에서 직접 선택하지 않는다. 복제 결과는 DRAFT이며 Save → Set Pattern
to PRODUCT → 배포 성공 확인 → Server/Client 재시작 → F1 Complete Play로 실제 타격·카드 반응을 확인한다.
화면 선택감과 아레나 재생의 최종 시각 판정은 사용자 확인이 남아 있다.
