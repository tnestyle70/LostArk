# 쿠크 관문별 Pattern Flow와 Complete Play 결과

## G00. 반영 범위

기존 F1/Composition Preview와 Server Pattern Audition 경로를 확장했다. 별도 Server 실행 경로나
새 Shared packet은 추가하지 않았다. 기존 대규모 미커밋 작업을 보존했고 자동 stage/commit/push하지 않았다.

- Boss Tool의 Pattern Flow 탭에서 관문 선택, Add From All Patterns의 Pattern/Bundle 선택,
  순서 이동·삭제·대기 시간 편집, Save/Load, Publish Saved Patterns, 재생·중단을 연결했다.
- F1 KoukuSaydon 목록은 Saved Pattern Flow와 All Patterns를 나눴다. Complete Play는 선택 관문의
  Sequence를 순서대로 마친 뒤 플레이어 follow camera로 돌아와 저장된 Flow를 시작한다.
  개별 Pattern/Bundle Complete Play도 유지했다. Composition Play All은 선택 관문에 한정하고
  실행 가능한 Bundle 멤버를 동시 실행하며 개별 row로 다시 중복 실행하지 않는다.
- Sequence 중단·실패, 관문 활성화 실패, 월드 또는 source revision 변경은 전투 시작으로 처리하지 않는다.
  Flow는 정확한 request/scope/epoch/revision의 전체 run COMPLETED 뒤에만 다음 row를 제출한다.
  Bundle 멤버 한 개 완료나 서버 follow-up PENDING은 다음 row의 시작 조건이 아니다.
- Action Workbench는 전체 Gate→Parent→Bundle→Pattern 트리를 표시한다. Action의 Gate/Model View
  필터를 제거하고 Sequence의 관문 선택은 유지했다. 로드 실패 이유와 Reload Patterns를 제공한다.
  imported World ID가 기존 ordinal-only 검증에 거부되어 트리가 비던 원인도 수정했다.

## G01. 저장과 2관문 Bundle

Composition revision 330의 optional patternFlows를 사용한다. 관문당 하나, 최대 256행이며
stable entryId, PATTERN/BUNDLE targetId, waitAfterMs(0–600000)를 저장한다.
문서의 기존 parse/validate/stage/commit과 CAS Save_Atomic을 재사용한다.
Save 후 Publish가 필요하며 게시 revision이 다르면 실행을 거부하고 이전 저장 파일을 보존한다.

초기 G1 Flow는 Pattern 1→2→6→7→29→30이다. 무력화 실패3·성공4·가짜세이튼5는 전체 목록에
유지하고 독립 Flow row에서는 제외했다. 기존 서버 follow-up/clone 연결은 유지한다.
G2 Flow는 Bundle 1→2→3→6→7→Pattern25→Pattern21→Bundle9→Bundle10→Bundle4→Pattern27이다.
마지막을 제외한 각 row 뒤에 1000ms를 기다린다.

거미카운터 Bundle6는 쿠크 P15 하나만 실행한다. 비어 있던 큰 세이튼 P16의 member 연결만 제거하고
P16 패턴 정의는 보존했다. 다른 actor는 기존 서버 idle 상태로 대기하므로 빈 idle 패턴이 필요 없다.
주사위 Bundle5와 공 Bundle8은 여전히 빈 멤버 때문에 Unavailable이며, 초기 Flow는 각각 준비된
Pattern25와 Pattern21을 사용한다. 실행 가능 목록은 26 Patterns와 8 Bundles다.

## G02. Sequence와 게시 입력 보정

G1 팝업북 37800ms와 피날레 21010ms는 기존 독립 Sequence Composition 순서를 따른다.
G2 인트로 27000ms에 필요한 World 정의 17개와 camera 5개·fade·lights의 Presentation 정의 7개를
독립 Sequence 문서에 연결하고 revision 3→4로 저장했다. 기존 타임라인과 기존 정의는 보존했다.
생성 스크립트도 Pattern과 두 종류의 정의를 함께 검증하고 atomic 저장하도록 보완했다. Action 문서의 같은 Pattern ID와 Sequence owner를 혼동하지 않는다.

Imported World는 world.kouku.gate2.intro.<suffix>와
world.sequence.instance.kouku.gate2.intro.<같은 suffix> 조합만 추가 인정하며 기존 generated ordinal
검증은 유지한다. non-LIGHT presentation의 WORLD anchor를 publisher가 잘못 거부하던 조건도
C++ 계약에 맞췄다. LIGHT는 MAP/PLAYER/BOSS 제한을 유지한다.
인트로 camera 5개는 MANUAL 대신 지원되는 PATTERN_ONLY로 바꾸고 camera source revision을 79로 올렸다.

## G03. 카드 기준점과 바닥 이펙트

새 Kouku pattern sequence가 시작될 때만 CNpc의 위치·회전과 보간 이력을 authoritative snapshot으로
즉시 갱신한다. 중앙 이동 뒤 startMs=0/followBoss=false 카드 장판이 이전 보간 pose를 고정하던 문제를
수정했다. 일반 이동과 같은 패턴의 stage 진행은 기존 보간을 유지한다.

내려찍기 P29와 거미카운터 P15 stage2는 v15 supplemental-only trail/ribbon 문서다.
Effect_DocumentRenderer가 LocalDecal 수를 무조건 양수로 요구해 전체 문서 준비가 실패했다.
유효한 supplemental이 있을 때 zero-LocalDecal을 허용하고 실제 typed adapter 개수 일치 검증은 유지한다.
임의 위치·크기 배율 수정은 하지 않았다.

## G04. 수행한 검증

| 검사 | 결과와 증거 |
|---|---|
| 실제 Composition Document C++ Reload/roundtrip/Save/CAS | PASS. Composition330, Sequence 로드; 잘못된 ID·관문·대기·stale Save가 이전 파일을 보존. out/KoukuPatternFlowService20260911/document_probe.run.log |
| 실제 Audition Service C++ + mock network | PASS. 혼합 순서, Bundle 전체 완료, follow-up PENDING, stale receipt, revision pin, wait, Stop와 승인 전 Stop, 거절·disconnect·gate/world 변경·송신 실패. service_probe.run.log |
| Python 관련 검사 | 3 tests PASS. out/KoukuPatternFlowService20260911/python-tests.log |
| NPC 실제 inline interpolator C++ | PASS. 이전 pose 재현, 새 snapshot(-0.319,737.531,237°) 즉시 반영, 이후 smooth sample 유지, NaN 거부. out/KoukuDecalAnchor20260911/interpolator_probe_result.json |
| 실제 D3D11 WARP effect 준비/renderer attach/clone | 두 문서 모두 이전 FAIL에서 PASS. out/KoukuDecalAnchor20260911/renderer_stage_probe_fixed.log |
| 공식 KoukuSaydon owner publish | source/runtime330, 26 Patterns/8 Bundles, 초기 G1 6행/G2 11행 모두 실행 가능 target 확인. out/KoukuPatternFlow20260911/publish.log |

공식 publisher는 다음 명령을 사용했다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision 330
```

추가 최종 검사:

- Debug x64 Client 최신 C++ 컴파일 exit0: out/KoukuPatternFlow20260911/client-build-final.log.
- Debug x64 Client 링크 exit0: out/KoukuPatternFlow20260911/client-link-only-final.log.
  실행 중인 사용자 Client.exe를 교체하지 않고 out/KoukuPatternFlow20260911/Linked/Client.exe에
  링크했다(53,243,392 bytes). 기존 Shared Debug library를 같은 임시 출력으로 복사해 사용했다.
  기본 실행 경로의 EXE는 아직 이전 버전이다. 기존 인코딩/DirectXTK PDB 경고가 남았으며 오류는 없다.
- 기존 --kouku-sequence-document-contract PASS: G1 두 Sequence, G2 explicit gate 시작·정확한 마지막
  Gate 전달, duplicate completion·preflight failure·취소·저장/CAS 검증. sequence-contract-review.log.
- 기존 --kouku-preview-transport-contract PASS: preview-transport-contract-review.log.
  synthetic fixture가 패턴 목록을 대체할 때 PatternFlows도 제거하도록 수정했다.
- Sequence importer focused test PASS: 누락된 World와 commit 실패가 원파일을 보존하며 정상 저장은
  기존 항목·모든 참조·revision을 유지. out/KoukuPatternFlowService20260911/sequence-import-test.log.
- 변경된 Composition330, Sequence4, Camera79 JSON과 관련 project XML parse PASS.
  변경 대상 git diff --check PASS(기존 줄바꿈 안내 제외).
- Sequence4는 Workbench 직접 authoring 입력이다. Product publisher의 별도 Arena.sequencer 입력과
  다르므로 Sequence 정의 수리에 추가 gameplay 배포가 필요하지 않다. 4개 owner receipt hash 정합 유지.

최소 링크 확인 전에 임시 OutDir의 일반 Build가 불필요한 전체 shader 재컴파일을 시작하여,
이 작업이 시작한 build 프로세스 트리만 중단하고 _BuildLinkAction으로 링크를 완료했다.
해당 중단을 Build PASS로 기록하지 않는다. 사용자 Client/Server 프로세스는 건드리지 않았다.

## G05. 실행 적용과 사용자 화면 확인

이 PC는 server-host이며 LAN 192.168.0.14:7777 설정과 방화벽 규칙이 준비돼 있다.
사용자가 실행 중인 Server/Client는 에이전트가 종료하거나 실행하지 않는다. 현재 실행 프로세스는
수정 전 EXE이므로 종료 후 Debug x64 빌드와 Server + Client profile의 Ctrl+F5가 필요하다.

확인 경로는 F1 → KoukuSaydon Complete Play → Reload KoukuSaydon Inventory → Gate 선택 →
Complete Play - Sequences + Pattern Flow다. 순서 편집은 KoukuSaydon Boss Tool → Pattern Flow →
Add From All Patterns → Save Pattern Flow → Publish Saved Patterns다.
Action Workbench의 Composition Patterns에서 전체 관문 트리를 확인한다.

사용자가 직접 확인할 화면은 G1 두 Sequence 종료 후 플레이어 시점과 저장 Flow 시작,
G2 인트로 및 두 actor Bundle 동시 재생과 단일 actor Bundle의 상대 idle,
진짜세이튼찾기 뒤 중앙 카드 장판의 위치·방향, 내려찍기·거미카운터 바닥 효과다.
D3D11 WARP 준비와 clone 성공은 아레나의 GPU 표시·최종 visual fidelity PASS를 뜻하지 않는다.
Client/UI 실행·조작·화면 캡처 및 사용자 대신 visual PASS 판정은 수행하지 않았다.
