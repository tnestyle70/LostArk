# 쿠크 탄도 밀치기 Client 문서·편집 결과

## G01. 반영한 계약

`KoukuSaydonCompositionDocument.h/.cpp`에 기본false인 optional `pushBallistic`의 parse·validate·serialize를 연결했다. 일반 밀치기의0..20m/기존 최대시간과 거리·시간 paired 규칙을 보존한다. 탄도는 양수100m 이하,100..5000ms,`pushCanLeaveArena=true`를 요구한다. 방향은 기존 두 값과 `AWAY_FROM_CONTACT`를 받으며 yaw는 양수 `BOSS_FORWARD`에서만 허용한다. JSON에 명시한 false도 damage Result 외에서는 거절한다. false는 직렬화에서 생략된다.

`KoukuSaydonActionWorkbench.h/.cpp`의 ColliderDamageSettings가 forcePush/canLeave/ballistic/yaw를 모두 읽고 보존한다. 재사용은 ID·표시명 외 전체 Result 값이 같은 경우에만 허용하므로 정책이 다른 기존 Result로 조용히 연결되지 않는다. 신규 Result도 네 정책을 복사한다. Collider 빠른 편집과 Result 편집 모두 세 번째 방향, 탄도별 거리·시간범위, 강제밀침/낙사허용을 표시한다. 탄도활성화는 낙사허용을 켜며 비활성화까지 해당 checkbox를 잠근다. 밀치기해제는 종속정책과 yaw를 함께 해제한다.

기존 네 파일의 UTF-8 BOM 없음과 CRLF를 보존했다. ActionWorkbench.h는 Visual C++ Language Service PID73980의 공유잠금으로 직접쓰기·교체가 거부되어 후보를 보존했다. RestartManager는 읽기조회만 수행했다. 사용자가 Visual Studio를 닫은 뒤 부모작업자가 기준hash일치를 확인하고 동일 후보를 설치했다. 현재header와 컴파일에 사용한 후보의 SHA256은 `48df8e79910d4d27c1d7e4df37f2e37db792009d70b348eabdb469106b0a7326`으로 같다.

## G02. 실제 검증

`out/KoukuBallisticClient20260921`에 기준본·scope 후보·native source/commands/log를 보존했다.

- 현재 codec와 의존4TU의 fresh compile/link 및 실제 Reload/Validate/Serialize/Save_Atomic 검사102건 PASS. 탄도 경계18조합,세 방향,일반 최대값,기본생략,정책보존,잘못된저장15종의 disk/LastGood/generation rollback,잘못된Boolean타입3종의 Reload rollback을 확인했다.
- ActionWorkbench 전체 TU scratch compile PASS. 당시 잠겨있던header와 동일한 후보 include로 컴파일했고 최종 설치본 hash가 일치한다.
- 실제 `Set_ColliderTriggerDamage`의 검증·Result재사용/생성 코드와 실제 UI→settings 로드 블록을 추출한 native adapter16검사 PASS. 다른정책의Result비재사용,공유원본보존,신규생성/다시읽기의네정책보존,동일값재사용,잘못된탄도/yaw실패의기존draft보존을 확인했다.
- 변경파일 `git diff --check` 통과.

`run.log`, `quick-run.log`, `quick-source-receipt.json`, `compile.log`, `workbench.log`, `files.json`이 증거다. native fixture는 out의 격리 Data만 저장했다. Product빌드·publisher·Client UI·Server실행·실제 Data변경은 이 하위작업에서 수행하지 않았다. 실제 Server탄도/접촉중심 소비와 projector/publisher는 부모·Server 담당 변경과 함께 최종 검증한다.
