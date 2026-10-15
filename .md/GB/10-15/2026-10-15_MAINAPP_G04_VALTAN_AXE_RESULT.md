# G04 발탄 손 무기 소켓 편집 분리 결과

작성일: 2026-10-15. 사용자 작성본의 G04를 검토·완성했다.
**제품 소스 반영과 Debug x64 Product 빌드·Client 링크 완료.** Client 실행과 화면·저장·게시 검증은 수행하지 않았다.
추가·교체 위치와 반영 후 H/CPP 6개 전문은 [G04 PLAN](2026-10-15_MAINAPP_G04_VALTAN_AXE_PLAN.md)에 있다.

## 실제 반영

- F1 Axe Transform의 손 무기 소켓 편집을 기존 CValtanBossTool로 이동했다.
- 사용자가 작성 중이던 ValtanBossTool_Axe.cpp를 Load·Save·Render 전체 구현으로 완성했다.
- ValtanBossTool.cpp의 미완성 중복 Render_AxeEditor는 제거하고 TreeNode·Combo·attempted 질문의 답을 새 구현 주석에 남겼다.
- 본체·유령 draft와 선택값은 BossTool 멤버로 유지한다. CPP 전역 draft와 기존 함수 static draft를 남기지 않았다.
- MainApp의 기존 편집 선언을 제거하고 준비된 BossTool의 Render_AxeEditor를 호출한다.
- MainApp_ValtanAxe.cpp와 옛 프로젝트·필터 항목을 제거했다. 새 항목은 사용자가 추가한 위치를 유지하고 `/utf-8`, PCH `NotUsing`, 빈 ForcedIncludeFiles를 보완했다.
- ValtanBossTool.h/.cpp와 ActorCatalog.h의 영어 설명 주석을 한국어로 정리했다. private 함수·상태를 목적별로 구분하고 소유·차용·수명을 설명했다.

실제 CValtan/Part_Equipment, Pattern duration, Server 생성·피해 판정, 데이터·publisher는 변경하지 않았다.
G05·G06-a와 개발 도구 공통 소유자 분리는 적용하지 않았다. 기존 Release 노출 조건도 유지했다.

## 손 무기와 패턴의 구분

| 대상 | 편집·입력 | 실제 소비 |
|---|---|---|
| 손에 드는 도끼 | Axe Transform, BossCatalog.weaponSocketTransform | ActorCatalog → CValtan::Update → CPart_Equipment → b_wp_r_01 |
| 플레이어 위치·주변 낙하 도끼 | Pattern/Stage의 combat object event와 schedule | Server의 target/nav 검사 → 전투 객체 생성·판정 → Client 복제 표현 |
| All Effects | effect 연결·재생 가능한 inventory | 기존 연출·Server combat object 경로를 연결하는 목록 |

AXE_DRAFT는 저장 전 손 무기 편집 상태다. attempted는 최초 로드 시도, loaded는 유효 값 확보,
saved는 마지막 성공한 로드·저장의 비교 기준, value는 현재 편집값, status는 결과 문자열이다.
최초 Load는 값을 채우며 setter를 호출하지 않는다. Save·Reload는 기존처럼 preview setter 결과를 무시하므로
저장 성공이나 value 보유만으로 실제 모델의 반영 성공을 단정하지 않는다.

BOSS_WEAPON_SOCKET_TRANSFORM에는 positionMeters와 rotationDegrees만 있다. scale은 없다.
위치는 정규화한 소켓 본의 축을 따르는 m 단위 보정, 회전은 추가 pitch/yaw/roll의 도 단위 값이다.
canonical은 저작 기준 원본인 정본을 뜻한다. 현재 ActorCatalog는 ProjectDataRoot로 Data/Actors 원본을 직접 읽는다.
편집 정본·publisher가 만든 DataFiles·메모리 캐시는 별개이며 setter는 캐시만 변경한다.

## 도구 수명과 live 상태

| 단계 | 현재 책임 |
|---|---|
| 생성 | MainApp이 필요할 때 BalanceTool과 BossTool을 생성. BossTool은 BalanceTool을 raw pointer로 차용 |
| 프레임 갱신 | MainApp이 창 표시와 별도로 Save/Publish·재생 준비를 갱신. BossTool의 graph/live 갱신은 표시 조건을 따름 |
| 그리기 | 표시 조건에 맞는 ImGui 패널을 구성. 실제 모델은 GameObject 경로가 그림 |
| 숨김 | 창·입력·관련 preview를 닫아도 도구 객체와 axe draft는 유지 |
| 종료 | MainApp이 도구를 해제. axe draft 소멸은 자동 Save·Publish·preview 복원을 실행하지 않음 |

m_strSelectedPatternId/StageId는 사용자가 Inspector에서 고른 항목이다.
m_strLivePatternId는 HUD의 Server Pattern ID를 현재 audition inventory에서 확인한 관찰값이고,
m_strLiveStageId는 그 Pattern의 Stage 중 HUD actionId와 대응한 stable ID다.
이 값들은 LIVE 강조·상태 표시·Follow Live에 쓰며 게임 모델·이펙트를 직접 생성하거나 Server 진행을 결정하지 않는다.

## 실행한 검증

| 검증 | 결과·근거 |
|---|---|
| 실제 소스 Debug x64 문법 검사 | Axe, BossTool, MainApp, ActorCatalog 4/4 PASS. 실제 include, 순차 `/Zs /Y-`. [결과](C:/Users/tnest/Desktop/LostArk/out/MainAppRefactor/g04-review/syntax-debug/results.json) |
| 정식 Debug Product | PASS, 총 199110ms. Engine·Shared·Server·Client 및 배포 완료. Client 104 OBJ와 바이너리 2개 갱신, 링크 성공. [빌드 결과](C:/Users/tnest/Desktop/LostArk/out/BuildPipeline/runs/20261014T173345934Z-debug-product.json) |
| XML·등록·심볼 | 두 XML parse, 새 TU 각 1개·옛 TU 0개, 새 Render 정의 1개·옛 선언/호출 0개 |
| 변경분 공백·문서 | 이번 root delta의 git diff --no-index --check 통과(CRLF 인지 설정), 전체 H/CPP 6개와 문서 일치. [기록](C:/Users/tnest/Desktop/LostArk/out/MainAppRefactor/g04-review/source-structure-validation.json) |
| 독립 코드 검토 | Load/Save/helper/lock의 기존 동작, static serial, 선행 도구 생성, BalanceTool 차용, 기존 UI 문자열 유지 확인 |
| 주석 전용 검토 | ActorCatalog.h 토큰 동일. BossTool.cpp는 미완성 Render 제거 외 동작·문자열 유지. value의 preview 보장 표현은 검토 지적에 따라 수정 |

실행 명령:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File 'Tools/Build/Invoke-BuildAndRegression.ps1' -Configuration Debug -Profile Product -MaxCompilerProcesses 4 -BuildLogDirectory 'out/MainAppRefactor/g04-review/build-debug'
```

Product 경로의 런타임 파일 존재·Navigation 참조·Item/Valtan reward catalog 검사는 통과했다.
데이터를 게시하거나 Client/Server를 실행한 결과는 아니다.

작업 전에 시작되어 있던 다른 Client 빌드는 변경 전 프로젝트 항목을 유지해 이동한 MainApp_ValtanAxe.cpp에서
C1083으로 종료됐다. 그 프로세스 종료 후 최신 프로젝트를 읽는 위 Product 빌드를 실행하여 통과했다.
옛 CPP를 복원해 두 경로를 함께 컴파일하지 않았다.

문자 집합 C4819/C4828과 기존 DirectXTK PDB의 LNK4099 경고는 남아 있다.
ActorCatalog.h의 새 한국어 주석도 BOM 없는 UTF-8을 CP949 TU가 읽을 때 C4819를 발생시킨다.
파일 인코딩이나 프로젝트 전체 문자 집합을 일괄 변경하지 않았으며 경고 0으로 기록하지 않는다.

## 남은 사용자 확인

- Valtan F1 Axe Transform의 본체/유령 선택, 축 변경, Reset, Reload와 실제 손 무기 부착.
- 창 숨김·다시 열기·Level 왕복 후 각 draft 유지.
- 최신 저장본 기준 Save, 같은 축 충돌 거부와 다른 축 변경 보존, Save 성공/Publish 시작 실패 상태.
- 명시적인 Publish 성공 뒤 기존 Server 재진입 절차에서의 표현 확인.

위 화면·저장·게시·Server 재진입은 미검증이다. 구현·컴파일 완료와 수동 확인을 구분한다.
무관한 사용자 변경은 유지했으며 commit/push는 수행하지 않았다.
