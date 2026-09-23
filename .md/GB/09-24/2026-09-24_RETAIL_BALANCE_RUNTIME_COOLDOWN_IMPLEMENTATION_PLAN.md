# Retail 수치와 실행 중 쿨타임 정책 구현 계획

## G00. 현재 상태와 목표

PR #454의 Retail 프로필은 Server 게시 시 player/skill/damage/boss와 world monster 수치를 덮는다. 공용 Balance Test는 아직 base JSON만 읽고 통합 publisher는 profile 인자를 전달하지 않는다. 따라서 기존 UI에서 편집한 값이 Retail override에 가려지거나 게시 버튼이 Retail 수치를 되돌릴 수 있다. HUD도 Server의 종료 tick과 Client base cooldown 분모를 섞고 있다.

Retail 전투 수치를 유지한 상태에서 현재 room의 쿨타임 정책을 실행 중 선택한다. 정책은 빌드 구성과 독립적이고 room 생성 시 빠른 테스트 모드가 기본이다. `Debug (3s)`와 `Release (Retail)` 버튼은 파일 저장이나 게시가 아니라 typed Server 명령이다. 기존 cooldown 0인 평타/스킬은 0초를 유지해 연속 공격 구조를 보존한다.

## G01. room 정책과 typed 네트워크 경계

Shared의 cooldown mode enum, 요청/결과, player snapshot mode와 cooldown duration ticks를 추가한다. `IPlayerCommandSink -> NetworkPlayerCommandSink -> NetworkManager -> Shared -> ServerApp -> RoomCommand -> CGameRoom` 경로를 재사용한다. 현재 session/world와 sequence를 검증하고 room policy를 한 번 변경한다. 같은 방의 다른 플레이어와 늦게 입장한 플레이어는 snapshot으로 동일 모드를 읽는다. 재접속한 다른 room의 정책이나 global catalog를 변경하지 않는다.

`PlayerSkillSystem`은 승인된 시작과 pending 시작 양쪽에서 room policy를 받는다. 기존 cooldown의 시작 시점과 서버가 적용한 duration을 보존하며 모드 변경 시 아직 진행 중인 cooldown만 새 길이로 계산한다. 이미 끝난 cooldown을 다시 열거나 현재 action을 재시작하지 않는다. 기믹/차량 전용 cooldown은 playable skill 정책과 분리한다. protocol은 root가 병합한 109 계약에 통일한다.

## G02. 표시와 수치 저장

`CombatHUDViewModel`은 진행 중 cooldown의 전체 길이를 Server snapshot으로 읽고 MainApp의 원형 표시도 같은 값을 사용한다. 공용 panel은 base row와 Retail override를 합쳐 실제 수치를 보여 준다. field마다 소유 문서를 기록해 override가 있으면 Retail profile, 없으면 base JSON의 stable ID/field에 저장한다. coefficient/addend/spread와 critical 필드도 실제 Retail field를 사용한다.

`Save-BalanceTestDraft.ps1`은 최신 base/profile 저장본을 같은 source snapshot 집합으로 묶고 field 충돌을 검사한다. profile 수정은 다른 배열이나 미지원 field를 보존한다. candidate Retail 검증 뒤 기존 원자 replace/자기 변경 rollback을 사용한다. `Publish-BalanceRuntimeSet.ps1`은 Gameplay와 World의 Validate/Publish 모두 같은 Retail profile을 전달한다. 새 C++ 파일은 추가하지 않는다. Retail profile 원본만 Client 프로젝트/filters의 기존 96.DataFiles/Balance None 항목에 등록한다.

## G03. Release 처치와 검증

Kill Current Gate Boss의 Release 전용 거부를 제거하되 현재 session/world/gate, primary boss, stale HUD와 sequence 검증 및 정상 death 소비자는 유지한다. Debug/Release가 같은 계약을 검증한다.

focused packet round-trip/잘못된 enum, room 기본 정책/전원 전환/기존 cooldown 보존, active HUD duration, profile field save/CAS/rollback을 검사한다. 제품 빌드·게시·commit은 root가 수행한다. Client/UI와 4인 화면 검증은 사용자가 직접 한다. 실제 실행 증거는 대응 RESULT에 기록한다.
