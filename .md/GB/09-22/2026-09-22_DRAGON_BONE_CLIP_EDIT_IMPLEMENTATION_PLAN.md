# 용 본 조정과 활강·상승 클립 구현 계획

## G00. 현재 모델과 편집 경계

고대의 바다 Vehicle 9523의 실제 AncientSea.wmodel은 91본과 30Hz native 9클립을 가진다.
기존 BoneAnimationDocument/Workbench와 CModel 채널 설치를 확장한다. 기존
authored.example 세 클립은 지상 idle 기반 예시이므로 보존하고 새 stable ID로 연구용
활강·상승 클립을 만든다. Client/UI 조작과 화면 판정은 사용자가 수행한다.

## G01. source 구간과 실제 pose에서 새 클립

BoneAnimationDocument.h/.cpp에 검증된 source 시작·종료 구간을 새 clip으로 만드는
명령과 실제 pose를 rest-relative key로 굽는 명령을 둔다. 후보 문서 전체 Compile이
성공한 뒤에만 draft를 교체한다. 원본 native 채널과 저장 파일은 이 단계에서 바꾸지 않는다.

활강은 npc_sk_look의 2500ms 날개를 펼친 자세를 2초 동안 유지한다. 상승은 1800ms부터
2766.6667ms까지 실제 날갯짓을 사용하고 끝 120ms를 시작 자세로 연결한다. 모델의
bip001 수직 변위는 rest 기준으로 제거하여 Server 고도를 이중 적용하지 않는다.
생성은 명시적 버튼으로 draft에 두 stable clip을 추가하며 기존 동명 clip을 덮지 않는다.

## G02. 본을 보면서 조정하는 작업 흐름

BoneAnimationWorkbench.h/.cpp에 source in/out 생성 입력, 실제 본 계층 선택, 선택 본의
현재 키 값 읽기, 저장 전 live TRS preview, Set key/Cancel pose, 시작·끝 key 복사와
draft Undo/Redo를 연결한다. 선택 본·클립·시각 변경 시 이전 본의 값이 새 본에 남지 않는다.
현재 scene preview의 실제 root와 view/projection을 쓰는 골격 overlay를 제공하고 본은
패널의 계층 목록에서 선택한다. scene의 본 클릭과 3D 회전 gizmo는 이번 범위에 넣지 않는다.
pose draft는 기존 document에 Set key할 때만 반영하며 미확정 상태에서는 다른 본·클립·시각으로
이동하지 않는다. baked clip 길이 변경은 전체 key를 같은 시간 비율로 재샘플링한다.

CharacterActionWorkbench의 기존 typed preview target과 재생 소유권을 사용한다.
본 preview와 action sequencer가 동시에 같은 모델을 재생하지 않도록 전환을 연결한다.

## G03. 저장과 검증

기존 Save의 skeleton/DAG/finite 검증, CAS, backup, atomic replace, 설치 rollback을
유지한다. schema는 유지한다. 새 BoneAnimationWorkbench_Viewport.cpp는 Client.vcxproj의
ClCompile과 .filters의 기존 03. Tools/05. Sequencer에 등록한다.
실제 설치 Sea CModel로 source 범위, pose, 반복 seam, native 보존, invalid 입력의 draft
보존과 JSON roundtrip을 확인한다. 변경 TU를 Debug 설정으로 컴파일하고 diff를 검사한다.
다른 Product 빌드가 실행 중이면 같은 출력에 병렬 링크하지 않는다. UI 동작과 최종 애니메이션
형태는 자동 검증과 분리해서 RESULT에 남긴다.
