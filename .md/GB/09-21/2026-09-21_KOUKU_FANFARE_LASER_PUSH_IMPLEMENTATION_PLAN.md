# 쿠크 연속 재생 위치와 팡파레·레이저 밀림 구현 계획

## G00. 범위와 현재 실측

사용자는 Flow 후 개별 재생 정지와 대형 세이튼 위치 어긋남 수정에 더해 팡파레의
밀림 거리 확대, 레이저의 두 번째 collider 접촉에서 즉시 다시 밀림을 요청했다.
이 문서는 플레이어 밀림과 P86/P87 시작 위치의 저작 데이터 변경을 소유한다.
개별 Complete Play 재진입의 코드 수정은 대응 결과에서 다룬다.

현재 P23 팡파레 logic.8~11은 공용 Result42의 2m/242ms를 사용한다.
P21 레이저 logic.1~7은 Result104/106/107의 6m/242ms를 사용하지만 forcePush가 없다.
104는 P85 바주카도 공유한다. 사용자 저작본은 작업 중 1995→1997로 저장되고 있다.
전체 이전 문서로 덮지 않고 최종 저장본에 요청 필드만 stable ID로 병합한다.

실제 Server Room의 전체 GATE2 Flow에서 bundle.3/P13의 잘린 원본 root 구간들이
대형 세이튼에 약 3.1325m의 끝 위치를 남겼다. 쿠크 P21/P23/P24/P85는 대형 세이튼의
위치를 바꾸지 않는다. P86/P87은 resetBossToSpawn=false여서 앞 위치를 이어받고 있었다.

## G01. 변경과 소비자

팡파레 네 접촉에만 Result42를 복제한 6m/242ms 결과를 연결한다. 기존 방향과
외곽 진입 정책, 피해량과 타이밍은 보존한다. 레이저는 세 결과를 각각 복제해 forcePush=true로
만들고 P21의 일곱 접촉에만 연결한다. 바주카와 나머지 공용 결과는 바꾸지 않는다.

기존 `CKoukuSaydonLogicRuntime`은 접촉 window별 재타격 시각을 보존한다.
`CPlayerSkillSystem::Can_ArmPlayerHitReaction`과 `Arm_PlayerHitReaction`의 forcePush 경로가
기존 밀림·피격 유예를 교체하므로 새 protocol이나 runtime 우회는 필요 없다.
두 번째 window는 즉시 새 6m 밀림을 시작하고 같은 window의 연속 tick은 기존 debounce를 유지한다.
사망·낙하·잡힘·이동 트리거와 같은 제외 상태는 유지한다.

P86/P87의 기존 resetBossToSpawn을 true로 바꿔 각각 저작된 대형 세이튼 spawn에서
시작한다. 패턴 안의 source root와 stage/occurrence offset, P13의 동작은 보존한다.
두 보스의 이동 권위를 합치거나 Server에서 대형 세이튼의 수평 이동을 전역 금지하지 않는다.
실제 전체 Flow와 단독 시작을 비교하고 entity 저장 순서를 뒤집어 같은 결과를 확인한다.

## G02. 검증과 반영

out 후보에서 전체 문서 검증과 실제 projector의 P21/P23 결과를 확인한다.
원본 결과·다른 Pattern·occurrence 시간/transform 불변, 중복 적용, 무관한 최신 저장 보존,
같은 필드 충돌 거부를 확인한다. 기존 C++ 소비자를 사용해 첫 밀림 진행 중 두 번째
콜라이더, 피격 유예, 같은 window 중복 방지, 기존 force=false 동작을 검증한다.

검토 가능한 후보와 코드 검증을 끝낸 뒤 사용자 저장 완료를 한 번 확인한다.
최신 디스크를 다시 읽어 필드 병합·백업·hash 재검사·원자 교체하며 실패하면 자기 변경만
원복한다. 실행 중 Client의 draft를 자동 Reload하거나 종료하지 않는다. 현재 소스 반영,
Pattern publish, 새 실행 파일, 실제 화면 확인을 분리한다. 새 제품 C++ 파일/프로젝트 등록은 없다.

## G06. 실제 접촉 연결 후속 수정

저장본 revision 2025에서 팡파레의 밀림 네 창은 실제 회전 Sector와 별개인 +X 고정 BOX에 연결돼 있다. 실제 네 Sector의 한 번 피해 결과에 전용 forcePush=true/6m/242ms/외곽 이탈 가능 결과를 연결하고, 중복 BOX 피해창은 비활성화한다. 앞 세 원형 공격과 공용 Result는 보존한다. 방향은 각 Sector의 실제 boss-local yaw를 사용한다. 바주카는 기존 +X 방향을 유지하며 전용 강제 밀림 결과를 연결하여 기존 밀림·FEAR·down·grace 중에도 요청 접촉을 반영한다. 일반 피해 자체가 grace를 만든다고 가정하지 않는다.

피자 P25는 현재 12개 피해 Sector가 모두 BOSS yaw+90으로 고정돼 있지만 사용자 폭발은 13개의 MAP 위치·방향·시간을 갖는다. 각 폭발 stable ID에 피해창을 대응하고 동일 MAP 위치와 실제 mesh footprint에서 측정한 방향으로 판정을 옮긴다. 사용자 visual TRS와 시작시간은 보존한다. 실제 geometry의 안전 간격과 gameplay Sector의 기존 90도 안전각은 구분하며 임의 90도 회전 부호를 적용하지 않는다. 관련 데이터는 out 후보·stable-field patch만 만들고 최신 실데이터 병합은 부모 작업이 담당한다.

실제 projector 출력과 기존 native LogicRuntime/PlayerSkillSystem probe로 각 방향의 hit/miss·즉시 밀림·중복 접촉·다른 패턴 보존을 확인한다. Server/Client 제품 코드와 전체 빌드·UI 조작은 추가하지 않는다.
