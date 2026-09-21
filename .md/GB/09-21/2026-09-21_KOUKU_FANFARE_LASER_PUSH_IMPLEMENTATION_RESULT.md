# 쿠크 팡파레·레이저 밀림 변경 결과

## G00. 현재 완료 범위

후보 데이터와 실제 Server 소비자 검증을 완료하고 사용자의 저장 완료·현재 저장본 반영 승인을 받았다.
최신 revision 2023에 요청 필드만 병합해 revision 2024를 저장하고 Pattern publish를 완료했다.
실행 중 Client/Server와 화면은 조작하지 않았다.

## G01. 요청한 동작과 기존 소비자

P23 팡파레의 네 접촉은 기존 Result42(2m/242ms)를 복제한 전용 6m/242ms 결과를 사용한다.
피해, BOSS_FORWARD 방향, forcePush=false와 외곽 진입 정책은 유지한다.
P21 레이저의 일곱 접촉은 Result104/106/107을 각각 복제하고 forcePush=true를 사용한다.
거리 6m, 시간 242ms, 방향 90/135.9/45도와 기존 경계 이탈 정책을 유지한다.
Result104를 공유하는 P85 바주카 등 다른 결과와 패턴은 바꾸지 않는다.

기존 `Can_ArmPlayerHitReaction`과 `Arm_PlayerHitReaction`이 forcePush 입력을 소비한다.
두 번째 collider는 첫 밀림 또는 피격 유예 중에도 즉시 새로운 밀림 방향·시간을 적용하며,
같은 collider window의 연속 tick에는 기존 재접촉 제한이 유지된다.
사망·낙하·잡힘·이동 트리거의 제외 조건은 유지된다. 새로운 Server 코드나 protocol은 없다.

같은 stable-field 후보에는 P86/P87의 resetBossToSpawn=true도 포함한다.
위치 원인과 전체 Flow 실측은 `2026-09-21_KOUKU_GATE2_SLAM_ORIGIN_IMPLEMENTATION_RESULT.md`가
소유한다. 개별 Complete Play 재진입 코드 결과는 `2026-09-21_KOUKU_COMPLETE_PLAY_REENTRY_RESULT.md`에 있다.

## G02. 실제 검증

`out/KoukuFanfareLaser20260921`에 후보, stable ID patch, 실제 projector 출력과 receipt를 저장했다.
전체 JSON 검증과 P21/P23/P86/P87의 dependency closure projection을 통과했다.
원본 결과·다른 패턴·발동 시간·transform 보존, 중복 적용, 무관한 최신 저장 보존,
동일 필드 충돌 거부를 확인했다. 초기 후보 revision 2005→2006과 별개로 최신 저장본
revision 2018에 대한 읽기 전용 병합 검증도 통과했다. 최종 반영은 다시 최신 저장본을 읽는다.

`out/KoukuFineNavigation20260921/push-followup/run.log`의 실제 C++ 소비자 검증은 **34개 PASS, exit 0**이다.
LogicRuntime, PlayerSkillSystem, ServerCombatHitRuntime, catalog/navigation/geometry를 현재 소스로
컴파일한 격리 실행이며 제품 실행 파일을 링크하거나 Client를 실행하지 않았다.

- 서로 다른 ENTER_AREA window에서 진행 중 밀림과 피격 유예를 force=true가 즉시 교체했다.
- force=false 대조군은 기존 밀림·유예를 유지하고 두 번째 window를 먼저 판정하지 않았다.
- 같은 window의 동일 tick·다음 tick은 중복 적용되지 않았다.
- 실제 projector의 레이저 7개·팡파레 4개 결과 모두 6m/242ms와 원래 방향·경계 정책을 소비했다.

소스 해시와 명령은 같은 디렉터리의 `receipt.json`에 있다. 위 검증은 typed 결과 입력과
Server 소비자의 검증이며, 실제 화면 거리와 최종 느낌은 사용자 확인 전이다.

## G03. 저장본 반영과 남은 경계

사용자가 Composition과 Effect Tool의 변경을 저장한 뒤 최신 디스크를 stable ID/필드 기준으로
병합했다. 백업·교체 직전 hash 재확인·원자 교체·자기 변경 rollback 절차를 사용했다.
`out/KoukuFanfareLaser20260921/applied.receipt.json`에 2023→2024 해시와 백업을 기록했다.
반영 직후 백업과 비교해 무관한 최신 편집, P25의 occurrence 시간/TRS, 기존 결과를 모두 보존한 것을 확인했다.
새 Result ID는 108/109/110/111이다. 사용자의 재실행과 화면 확인은 남아 있다.

## G04. Pattern publish

`Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision 2024`가
exit 0으로 완료됐다. product/world/gameplay.balance는 PASS, map은 기존 receipt를 재사용했다.
Encounter와 Pattern bindings의 sourceRevision은 모두 2024이며 게시 데이터에서 레이저 7개,
팡파레 4개 결과와 P86/P87 시작 위치 복원 두 값을 확인했다.
`published.receipt.json`과 `publish-retry.log`에 검증 및 publisher 결과를 기록했다.

첫 시도는 Encounter JSON의 원자 교체가 Windows 파일 잠금에 막혔다. publisher가 남긴 백업
27개와 현재 실행 데이터를 비교해 모두 동일한 것을 확인했다. 후속 Restart Manager 조회와
비변경 DELETE 접근 검사는 잠금 없음이었으며 실패 당시 점유 프로세스는 특정하지 못했다.
사용자의 실행 파일 종료 이후 재시도는 정상 완료됐다. 잠금 때문에 파일을 비원자 덮어쓰거나
freshness 검사를 제거하지 않았다. 실패 로그와 보존 확인은 `publish.log`, `publish-failure-audit.json`에 있다.

## G05. Debug 제품 빌드

사용자가 Client/Server를 종료한 뒤 정본 `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`를
기존과 같은 VS 18 Insiders amd64 MSBuild로 실행했다. Engine·Shared·Server·Client 모두 PASS, 전체 exit 0이다.
Client는 변경 의존성을 포함한 OBJ 157개와 Client.exe를 생성했고 PCH/CSO 쓰기는 0이었다.
DLL 배치와 필수 runtime 파일, Items/ClearRewards 내용 검사도 PASS이며 누락·무효 항목은 없다.
실행 파일은 `Client/Bin/Debug/Client.exe`, `Server/Bin/Debug/Server.exe`다.

빌드 정본 결과는 `out/BuildPipeline/runs/20260920T224103825Z-debug-product.json`,
로그와 최종 파일 해시는 `out/KoukuFanfareLaser20260921/debug-product-build.log`, `final.receipt.json`에 있다.
Client/UI를 자동 실행하지 않았으며 최종 전투 화면·체감 거리는 사용자 확인 전이다.

## G06. 실제 콜라이더 연결과 즉시 밀림 후속 후보

사용자의 실제 재생 피드백으로 revision 2025를 다시 조사했다. 팡파레의 6m 수치 변경만으로는 실제 회전 부채꼴에 밀림이 연결되지 않았다. logic8~11은 모두 +X 고정 BOX54~57에 연결돼 있었고, 회전 Sector50~53의 Result98은 밀림이 없는 일반 피해였다. Result108은 force=false, 외곽 이탈 false, 방향 offset0이었다. 일반 피해98 자체는 grace를 만들지 않으며, 기존 다른 밀림·FEAR·down·grace가 있을 때 non-force가 거절되는 것은 별도 원인이다.

후보는 실제 네 Sector의 한 번 피해 결과를 각각 전용 6m/242ms 강제 밀림·외곽 이탈 결과113~116으로 연결한다. 각 대응 visual8/19/20/21의 위치·시작시간·방향에 맞춰 boss-local yaw90/0/-75.7/-180을 적용했다. 중복 BOX logic8~11은 enabled=false, collider54~57은 debugRender=false로 보존했다. 앞 세 원형 공격과 공용 결과는 유지한다. 바주카는 6m/242ms/yaw90/외곽 이탈을 유지한 Result104의 전용 복제117에서 forcePush=true만 추가하여 실제 두 빔 logic2/3에 연결했다. 첫 빔과 겹치는 source logic1은 비활성화하고 해당 debug collider21만 숨겼다. 동일 창의 반복 피해는 기존 소비자가 방지한다.

피자 P25의 기존 12개 Sector는 전부 BOSS yaw90으로 고정돼 있었으나 현재 사용자 폭발은 서로 다른 MAP TRS와 시각의 13개 행이다. 실제 현재 CEffectPlayback(.2초) 행렬과 설치 WModel 삼각형을 반경2m에서 0.02도 간격으로 측정했다. asset-local 안전 간격은 -73.18~11.98도, 중심 -30.6도이며 이에 반대인 피해 중심149.4도를 각 MAP 폭발 yaw에 더했다. 기존 gameplay 피해 halfAngle135도는 유지했고 원본 geometry의 약85.16도 안전 간격과 정확90도 gameplay 안전각을 동일하다고 기록하지 않는다. 각 폭발 위치/시작시각에 한 번 피해를 대응하고 마지막 13번째에는 logic15/collider118을 추가했다. 본체 시작 yaw와 MAP 폭발 TRS는 별개이며 visual은 변경하지 않았다.

`out/KoukuHitCollider20260921`에 field-patch/candidate/projected/receipt와 geometry 근거가 있다. 기존 shared Results·다른 Pattern·모든 visual occurrence를 보존하고 동일후보 재적용·무관 저장 보존·동일 필드 충돌 거부를 확인했다. 실제 projector validation/projection PASS다. 실제 Server 소비자를 현재 소스로 scratch 컴파일해 13개 피자, 4개 팡파레, 2개 바주카 창을 검증했고 `hit_run.log`는 208개 PASS/0 FAILURE/exit0이다. 보스 yaw0/226.5에서 피자 위험·안전 방향, 진행 중 밀림/FEAR/down/grace에서 강제 밀림의 방향·6m 거리 입력·242ms·외곽 이탈 정책, 같은 창 재타격 방지와 force=false 대조군을 확인했다. 이는 실제 물리 경로에서 6m를 끝까지 이동했다는 검사나 GPU 화면 승인과 다르다.

이 후속은 데이터 후보이며 live JSON 병합·publish·제품 실행을 수행하지 않았다. Server C++ 추가 수정도 없다. 부모 작업이 최신 저장본의 stable field를 병합하고 통합 후보의 새 ID 충돌을 확인한다.
