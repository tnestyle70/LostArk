# 고대의 바다 비행 구현 계획

## G00. 실제 모델과 권위 경계

고대의 바다(9523)의 E(98523)는 `npc_sk_look` 단일 160 tick/30 Hz 원본 클립이다. 실제 WModel의 bip001 높이와 양쪽 upperarm/forearm/neck/pelvis 회전을 읽었다. 54→83 frame의 날개 왕복 구간을 선택하고 110 frame 이후 하강·착지 구간을 사용한다. 이 구간 선택은 저작값이며 원작 비행 gameplay라고 주장하지 않는다.

## G01. Server 입력과 상태

기존 C2S_MOVE에 typed GROUND_GOAL/VEHICLE_FLIGHT intent를 추가한다. 비행 입력은 목표 좌표 대신 정규화된 world XZ 방향과 수직 축이며, Client transform을 제출하지 않는다. protocol 104 snapshot은 GROUNDED/TAKEOFF/FLYING/LANDING phase와 phase 시작 tick을 전달한다. E skill은 Server가 이륙·착륙을 전환한다. timeout이면 입력을 0으로 만들고 navigation XZ 경계와 collision은 Server가 유지한다. 고도는 지면 기준 제한하며 강제 하차 시 마지막 검증 지면에 복귀한다. 발탄·쿠크는 9523에만 예외를 적용한다.

## G02. 실제 Client 소비자

Controller는 비행 중 WASD/Space/Ctrl을 typed command sink로 보낸다. E 이외 탈것 skill은 비행 중 막는다. `Character`와 `Part_Vehicle`은 기존 catalog E 클립의 동일 시간창을 rider/용에 적용한다. 날개 loop 끝은 시작 pose와 짧게 보간하고 실제 목·머리 chain을 진행 방향 목표에 맞춰 제한적으로 회전한다. Camera_Free orbit은 통합 담당이 같은 탑승 상태와 실제 follow transform을 읽는다.

## G03. 검증

기존 Server vehicle contract에 재입력, timeout, 제한 고도, 지면 복귀, 아레나 예외를 추가한다. Shared packet roundtrip/변조 거부와 변경 JSON parse, publisher 검증, diff check를 실행한다. 정상 증분 Product 빌드는 통합 담당이 한 번 수행한다. 신규 C++ 파일은 없으므로 project/filter 항목 변경은 없다. Client와 UI 실행 및 시각 판정은 사용자가 직접 한다.

## G04. 비행 창 편집과 카메라

`CharacterModelWorkbench`의 고대의 바다 E Details에서 원본 클립의 loop start/end와 landing start를 초 단위로 편집한다. 실제 설치 모델의 클립 길이와 0 < loop start < loop end <= landing start < clip end를 검사하고, 기존 stable action의 최신 subtree와 비교해 원자 저장한다. 별도 Save Flight Logic은 애니메이션/Effect/Sound draft를 변경하지 않는다. 입력 중 action 전환을 막고 저장 또는 명시 Discard 이후에만 전환한다. 재생성 도구도 flightWindow를 보존한다. Server 시간은 domain publish에서 저장된 구간으로 파생하며 실행 중 Server를 자동 갱신하지 않는다.

`Camera_Free`는 승인된 고대의 바다 탑승 상태와 실제 follow target 방향을 소비한다. 좌클릭 press가 UI·포커스·연출 경계를 통과한 경우에만 드래그를 시작하고 이동량을 rad/pixel로 누적한다. 용의 최단 방향 전환을 공전 yaw에 더하고 기존 지수 보간으로 위치와 시선을 이동한다. F6·대상 교체·하차는 공전 session 상태를 해제하며 저작 camera profile은 보존한다.
