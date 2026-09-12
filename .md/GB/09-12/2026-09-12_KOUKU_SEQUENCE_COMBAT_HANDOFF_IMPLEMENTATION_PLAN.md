# 쿠크 입장 시퀀스와 같은 서버 보스의 전투 연결 구현 계획

## G00 사용자와 확정한 실행 구조

Complete Play — Sequence + Pattern Flow는 책 펼침 및 입장 연출을 재생하면서 실제 서버 권위 세이튼의 같은 NetEntityId·HP·모델을 사용한다. 연출 종료 시 서버가 플레이어를 열린 전투 아레나 시작 위치에 배치하고 플레이어 카메라와 HUD를 복구한 뒤 저장된 전투 Pattern Flow로 이어진다. 마지막에 연출 프록시를 없애고 다른 전투 보스를 생성하는 구조는 사용자 요구와 다르므로 완료 목표에서 제외한다.

F1의 1관문/3관문은 기존 열린 전투 아레나에서 즉시 보스 생성과 패턴 테스트를 하는 경로로 유지한다. Complete Play는 이 테스트 진입과 분리한 통합 실행이다. 플레이어 배치는 Client Transform 직접 변경이 아니라 기존 서버 명령/승인/snapshot을 사용한다.

## G01 현재 구현과 먼저 보존할 경계

현재 MainApp의 PrepareKoukuGateCompletePlay는 기존 보스를 despawn하고, Finish에서 Debug_ActivateGate로 서버 보스 생성과 플레이어 이동을 함께 요청한다. 2026-09-12 13:25:09 Client Product 빌드는 이 이전 구조다. 신규 같은 개체 구조의 구현·빌드 완료로 취급하지 않는다.

기존 Stop/실패/15초 timeout/world generation/저장 revision 검사, 늦은 spawn 및 move 응답의 process-local token retire, Camera/HUD 보류, 종료 승인 대기의 fade는 재사용한다. Sequence의 enterCombatOnFinish는 G1/G2/G3 입장 선택에만 사용하며 clear/maze를 자동 재생 목록에 넣지 않는다.

## G02 실제 보스 애니메이션과 원본 위치

CModel::Attach_AnimationSet은 이미 동일 rig의 클립 추가를 지원한다. KoukuSaydonPresentationAssetService의 body prototype stage 단계에서 기존 animationSetId를 소비하고 동일 골격·이름 충돌을 검사한 뒤 commit하는 방식으로 실제 Npc에 연출 클립을 연결한다. 별도 Engine 모델 런타임이나 Client 로컬 보스 프록시를 추가하지 않는다. 조사 근거는 out/KoukuSourceSequenceRestore20260912/live_boss_animation_admission_review.json이다.

원본 G1은 SaydonBook/Stage/Finale 세 actor와 동시 A/B weight를 사용한다. 기존 baked 동일 rig 클립은 이 혼합을 보존하지만 실제 보스 한 개로 합치려면 역할 전환 시각과 서버 위치 곡선을 명시해야 한다. Book/배경/Effect/Camera는 기존 시간축을 유지하며 보스 World proxy occurrence만 실제 Npc Animation으로 대체한다. 원본 stage/finale z 약737.5와 전투 아레나 z 약942.3의 차이를 함께 해결한다.

현재 서버 BossMotion은 고정 Y/yaw의 단일 직선뿐이다. 기존 BossMotion 계약에 제한된 position/yaw key track을 확장하고 Server tick에서 샘플해야 한다. 통상 전투 이동의 navigation 검증과 연출의 공중 이동을 명시적으로 구분하며, 종료 위치는 전투 시작 위치와 navigation을 검증한다. Client의 임의 Transform 이동으로 우회하지 않는다.

## G03 서버 입장 Pattern과 종료 소비자

입장 연출을 실제 보스의 첫 Product Pattern으로 실행한다. 기존 Kouku Brain Begin_Pattern/Finish_Pattern은 entity와 HP를 유지하며, 자동 AI는 실행하지 않는다. 종료는 Client preview elapsed가 아니라 서버 audition의 최종 COMPLETED를 소비한다. 실제 모델 애니메이션과 Camera/World/Effect는 기존 commonStartTick/iActionStartTick을 사용한다.

저장 문서→publisher→Server Pattern에 명시적 연출 역할을 연결한다. 연출 중 참가자 이동/스킬/버퍼/발사체와 보스 피해를 서버에서도 통제하고, 기존 bPatternInvulnerable와 player combat-ready의 이전 상태를 run이 소유해 완료/Stop/disconnect마다 원복한다. isCombatReady=false만으로는 이동·스킬 입력 경로가 값을 다시 켜므로 충분하지 않다. 필요한 typed lifecycle은 기존 packet을 재사용하되 실제 소비자를 확인한다.

실행 단계는 서버 보스 준비 승인 → 같은 보스의 입장 Pattern 및 연출 시간축 → 서버 연출 완료 → 플레이어 시작 위치 배치 승인 → 카메라/HUD 복구 및 저장 Flow 시작이다. 마지막 단계에서 Debug_ActivateGate를 다시 호출해 보스를 재생성하지 않는다. pending 상태와 실패 때 기존 정상 상태 보존을 명확히 하고 늦은 응답이 새 실행을 시작하지 않게 한다.

## G04 사용자가 만든 입장 Pattern

사용자가 이전 EXE 작업 사본에 만든 세이튼_1관문연출은 KAKULSAYDON_G1_PATTERN_35/MN_RPCT_05/GATE1/boss.kakulsaydon.g1.saydon이다. 해당 작업 사본에는 10종 native clip을 13개 구간, 41,488ms로 연결한 DRAFT를 넣는다. 이는 원본 키 발생 순서 편집 초안이며 원본 동시 A/B 혼합, 위치, 카메라, HUD/전투 handoff의 완료 증거가 아니다. 사용자의 수정 및 새 Pattern ID를 보존하여 통합한다.

이전 EXE는 out/PreviousAnimationSession20260912/Data를 별도로 저장한다. 원본 Data와의 반영은 data_snapshot_manifest.json 및 저장 전후 근거로 세 방향을 비교한다. 활성 사용자 편집과 다른 미커밋 변경을 덮어쓰지 않고, 공유 Resources의 기존 파일 교체는 해당 EXE 종료 후 수행한다.

## G05 검증과 완료 경계

같은 entity/HP 보존, 서버 시작·완료 tick, 중간 Stop/실패/연결 해제/지연 응답, 첫 단계와 후속 Flow의 저장 revision 일치, 서버 위치 key 경계와 최종 navigation, JSON 왕복을 필요한 기존 검사로 확인한다. 변경 C++ 최소 컴파일과 정본 Product 빌드, domain publisher의 최소 Publish/Check 및 git diff --check를 수행한다. 새 파일이 생기면 해당 project/filter를 함께 등록한다.

제품 빌드, 설치된 리소스/데이터, 실행 준비, 사용자 화면 확인을 각각 RESULT에 기록한다. Client/UI를 자율 조작하거나 캡처하지 않는다. 원본 A/B, GlobalSlomo, 파티원 교체/착지 및 미해결 재질의 미완료를 화면 복원 완료로 표현하지 않는다.

## G06 현재 Complete Play의 게시·Flow 누락 수정

같은 서버 개체 전환 구현과 별도로, 현재 연출 재생 경로를 막는 source/published revision 불일치를 정식 KoukuSaydon owner 게시로 해소한다. 원본 사용자 초안은 보존하고 publisher의 unavailable 격리를 사용한다. 기존 G1/G2 Flow는 유지한다. 저장 Flow가 없는 G3은 이미 정상 게시 가능한 P18 외곽불·갈고리 시각테스트 한 행을 기본 Flow로 연결한다. 새 공격 패턴을 만들거나 F1에 없던 자동 Flow가 원래 존재했다고 설명하지 않는다.

이전 EXE의 P33/P34 편집은 기준본과 현재본의 동일성을 확인해 합친다. 양쪽 신규 P35의 충돌은 현재 쇼타임 P35를 보존하고 이전 1관문연출을 새 P36으로 재배정해 해결하며 clip/시간/loop 값은 보존한다. 최종 source revision으로 Product·World·Balance를 함께 게시한 뒤 세 관문 Complete 선택과 Flow target의 가용성을 확인한다.
