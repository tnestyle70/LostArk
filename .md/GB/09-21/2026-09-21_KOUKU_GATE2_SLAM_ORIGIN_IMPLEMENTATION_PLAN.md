# 쿠크 2관문 대형 세이튼 내려치기 기준점 구현 계획

## G00. 현재 실측과 이번 범위

현재 설치된 revision 1987 Gameplay와 실제 CGameRoom의 Gate2 Pattern Flow 순서를
CPU에서 재생했다. bundle.3의 P13 조커 찾기가 대형 세이튼을 XZ 약 3.1325m 뒤로
이동시킨 채 끝난다. 잘린 애니메이션의 root delta를 반복한 저작 결과이며 이후 P21 레이저,
P23 팡파레, P24와 P85 동안 대형 세이튼 좌표는 매 tick 그대로다. entity 배열 순서를
반대로 바꿔도 같은 결과다. P86/P87은 resetBossToSpawn=false여서 앞 패턴의 끝 위치를
그대로 다음 root motion 기준점으로 사용한다.

이번 변경은 P86 두 번 내려치기와 P87 세 번 내려치기의 시작 기준점을 기존 spawn으로
복원한다. P13의 애니메이션 곡선, 다른 패턴, 대형 세이튼 배치와 Server 이동 코드는 유지한다.
사용자가 편집 중인 최신 source와 설치된 revision 1987 검증은 구분한다.

## G01. 파일과 기존 소비자

`Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json`의 P86/P87에만
`resetBossToSpawn=true`를 설정한다. 본 요청의 팡파레/레이저 결과 수정과 같은 최신 저장본
병합 후보에 통합하며, 최종 병합은 주 작업자가 소유한다. resetBossYawDegrees는 추가하지 않는다.

기존 projector와 publisher가 flag를 BOSS_PATTERN_DEFINITION으로 전달한다.
`CGameRoom::Prepare_KoukuAuditionTick`은 각 member의 자체 iBossEntityId로 boss를 찾고
Begin_Pattern한 복사본의 XYZ를 그 boss의 fSpawnPositionXYZ로 설정한다. 별도 yaw 값이
없으면 현재 yaw를 유지한다. root origin은 이후 첫 `Apply_StageRootMotion`에서 캡처되므로
stage별 원래 곡선과 손/이펙트의 boss 기준 계산이 복원된 몸체 위치를 사용한다.
동적 WORLD support가 존재할 때만 기존 max(spawnY, supportGroundY) 정책을 그대로 쓴다.

새 public interface, C++ 파일, project/filter 등록은 없다.

## G02. 검증과 완료 경계

out/KoukuBigSaydonMotion20260921에서 설치된 catalog의 독립 메모리 복사에 두 flag만
바꾸어 같은 전체 Gate2 entry 순서를 재생한다. P13 뒤의 기존 이동은 유지하고 P86/P87
첫 tick의 XYZ가 자체 spawn으로 돌아오는지 확인한다. yaw 유지, 다른 boss의 불변,
각 slam의 stage별 원본 RootMotion 표본 불변과 패턴 중 실제 이동을 확인한다.
원본과 후보 모두 정상/역순 entity 배열로 실행한다. 현재 source P13/P86/P87의 animation,
duration, source range와 사용자 최신 변경은 별도로 기록한다.

Server listener와 Client를 실행하지 않는다. 실제 source 교체와 publish는 주 작업자의
최신 저장 병합 절차를 사용하고, 제품 화면 판정은 사용자가 직접 한다.
