# 쿠크 접촉 재타격과 대형 세이튼 포물선 넉백 구현 결과

## G00. 완료한 서버 연결

`pushBallistic`와 `AWAY_FROM_CONTACT`를 기존 typed result → world hit → player reaction →
Room 이동 경로에 연결했다. 실제 판정한 장판의 현재 세계 중심을 전달하고 보스 위치로
대체하지 않는다. 지면 밀림 기본 동작과 20m 제한은 유지했다. 포물선은 100m 이하,
100~5000ms, 외곽 이탈 허용, MAX_HP_PERCENT_DAMAGE 결과만 받는다.
PATTERNLOGICPUSH의 기존 8/9/11/12열 입력과 새 13열 bool 입력을 함께 읽는다.

30m/1500ms의 초기 수평 속도는 20m/s, 수직 속도는 7.35m/s이며 750ms 정점 높이는
2.75625m다. 비행 중 네비게이션과 collision 이동 제약을 건너뛰고 하강 중 실제 바닥에
착지한다. 낮은 바닥은 끝점에서 중력 하강을 이어가며 높이를 순간 변경하지 않는다.
바닥이 없으면 기존 FALLING에 하강 속도를 인계하고 기존 사망 deadline을 사용한다.
KNOCKDOWN과 기존 snapshot XYZ를 사용하므로 Shared packet이나 Client 이동 우회는 없다.

레이저는 기존 `rearmOnExit`와 `forcePush`를 연결하면 231ms 창 안에서도 두 번째 실제
재진입이 기존 242ms 밀림을 즉시 교체한다. 연속으로 안에 머무르는 tick은 다시 때리지
않는다. 저작 데이터·UI·projector의 연결은 부모 작업이 별도로 소유한다.

## G01. 실제 바닥 정보

Navigation publisher가 Server 전용 `.navsurface`를 같은 Area transaction에 포함한다.
기존 `.navgrid` 파일과 Client 입력 형식은 그대로다. NSRF v1에는 grid 전체의 FNV-1a
64-bit hash, cell 수, source/paint resolved surface mask가 있다. 독립 mask 덕분에
높이 0인 BLOCKED 바닥도 바닥 부재와 구분한다. runtime blocker는 바닥을 제거하지 않고
명시 void condition만 제거한다. 잘못된 sidecar는 부분 navigation을 남기지 않고 실패한다.
구버전 sidecar 없는 grid는 기존 walkable 바닥 집합을 유지한다.

같은 navigation 객체를 다시 Load할 때 이전 void condition 집합이 남던 문제도
초기화했다. 그렇지 않으면 이전 void와 같은 이름의 새 obstacle이 바닥을 지운다.
BuildDomains navigation outputs에 Server `*.navsurface`를 등록했다.
public 저장 계약은 AREA_DATA_LAYER_GUIDE에 반영했다.

## G02. 실행한 검증

- 변경한 전체 C++ TU의 scratch 컴파일 성공. 제품 빌드나 프로세스 실행은 하지 않았다.
- 실제 `Run_KoukuPushContracts` 24개 성공: 기존 force/ground/edge 동작과 새 장판 중심,
  포물선 정점·착지, 중심 누락 거부, 짧은 레이저 창의 실제 재진입을 포함한다.
- 추가 native 57개, 실패 0: y=0 blocked floor, collision 통과, 30Hz 적분, 낮은 deck,
  gap·void 낙하→실제 사망, runtime obstacle 착지, sidecar 손상 네 종류, native bootstrap
  13열·조건부 범위 제한·Brain contact 검증을 실행했다.
- 실제 Gate2 중심 `(4.66, 10.56, 322.94)`에서 8방향 30m 비행은 모두 1500ms 뒤
  발판을 벗어나 FALLING으로 전환했다.
- Navigation publisher ContractTest 성공. out 후보의 실제 Kouku base+detail 9개
  navgrid 바이트는 설치된 기존 파일과 모두 동일하다. Gate2Fine는 physical surface
  2306개와 walkable 2301개를 구분한다.
- `git diff --check`와 BuildDomains JSON parse 성공. 수정 C++의 UTF-8/CRLF 유지.

native 실행은 네트워크가 없는 기본 subsystem room fixture에서 실제 서버 함수 본문을
그대로 추출해 링크했다. 원본 전체 TU도 컴파일했으며 추출 본문 hash를 기록했다.
제품 room의 입장·네트워크·UI 실행 성공으로 확대 해석하지 않는다.

증거: `out/KoukuBallistic20260921/receipt.json`, `run.log`, `extraction.json`,
`navigation-receipt.json`, `before.json`. 원본 백업은 같은 폴더의 `before/`다.

## G03. 부모 통합 경계

후보는 `out/KoukuBallistic20260921/navigation-candidate/Navigation`에만 생성했다.
실데이터 교체, gameplay publish, 최종 navigation publish와 제품 빌드는 부모 작업이
소유한다. 이 문서는 source·후보·native 검증 완료 상태이며 설치 완료를 뜻하지 않는다.

최종 navigation 명령은 아래와 같다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/NavigationPipeline/Publish-ServerNavigation.ps1 -Mode Publish -AreaId LV_LUT_MIDNIGHTC_ED
```

최종 장면의 이동 거리·연출 속도·낙하 모습은 사용자가 직접 확인한다.
