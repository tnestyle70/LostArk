# Kouku Parent 타임라인과 Pattern 행 구현 결과

## G00. 구현한 저장·편집 계약

기존 Parent 폴더에 optional `timelinePatternId`를 연결했다. `Create Parent Timeline`은 같은 폴더의 실행 Pattern을 만들고 기본 duration 15000 ms를 둔다. 기존 Animation/Logic/Summon/World/Scene Profile/Effect/Collider 등의 행은 그대로이며 Parent에만 Pattern 행을 표시한다.

`Append Pattern at Cursor`는 독립 Pattern을 stable ID로 참조한다. occurrence는 시작, 시간창, 반복을 소유하며 상세 원본을 열어 편집한 뒤 Parent로 돌아올 수 있다. 원본을 바꾸면 이를 참조하는 Parent와 Bundle도 DRAFT가 된다. 참조 중인 원본 삭제는 거부하고 Parent 이름과 backing Pattern 이름은 함께 바꾼다. 기존 분류 전용 Parent와 다중 보스 Bundle은 그대로 읽힌다.

박스 선택·이동·trim·Duplicate·Delete와 명시 duration을 연결했다. Stage를 포함하는 복제·삭제는 Parent 시간창도 함께 늘리거나 압축한다. 단순 lane 복제는 선택 끝에 배치하며 필요한 경우에만 전체 duration을 늘린다. 실패한 수정은 기존 draft를 보존한다.

## G01. Preview·publisher·Server 연결

Preview와 Python publisher는 각 child의 Stage/Animation/Logic/World/Scene/Profile/presentation과 내부 참조를 occurrence별로 전개한다. 저작 파일은 참조를 저장하고 파생 실행 데이터만 기존 단일 Pattern으로 바꾼다. 자식 원본과 Parent의 공통 행은 별개의 ID·수명으로 남는다. 두 Parent를 다른 보스의 Bundle 멤버로 사용하면 각각 원본 문서에서 독립 전개한 뒤 Pattern과 추가 Logic 정의를 합친 Preview snapshot을 쓴다.

진행 중 자식의 시간이 창 끝에서 잘리면 `cancelAtEnd`가 Gameplay `PATTERNLOGICCANCEL`을 거쳐 실제 Logic runtime에 전달된다. 자식 timeout/fail을 실행하지 않고 해당 판정을 취소하며 부모 판정은 유지한다. 자식 POSE_INPUT의 HUD도 자신의 구간에서만 활성화된다. 분신의 종료 시간도 자식 창을 넘지 않는다.

Parent Product의 `fixedTimeline`은 `PATTERNFIXEDTIMELINE`으로 전달된다. Server는 Parent 시작에서 누적 millisecond 경계를 계산하므로 각 Stage를 별도로 tick 올림할 때 생기는 누적 지연을 피한다. 같은 30Hz tick으로 압축되는 두 Stage 경계는 전개 단계와 Server admission에서 거부한다.

동일 보스의 겹치는 Pattern 창, 중첩·순환 Parent, 서로 다른 관문/모델/대상, 자식의 전체 패턴 조기 종료·동적 follow-up·강제 BossMotion·reset/전투전환 등 현재 고정 배치에서 의미를 유지할 수 없는 조합은 명시적으로 거부한다. 이는 저작 시간표와 반복을 실행하는 기능이며 상황에 따라 공격을 무작위 선택하는 새 AI나 마리오 1~4 진입 카운터는 추가하지 않았다. 마리오 전체 패턴의 세부 row 조립은 사용자가 개별 패턴을 준비한 뒤 진행한다.

## G02. 수행한 검증

- 실제 Composition C++ probe 61 checks PASS: legacy parse/roundtrip, 참조와 반복, 잘린 마지막 주기, 모든 지원 lane 및 내부 ID 재연결, 부모 공통 판정 보존, 경계 취소, 공용 상태 충돌, CAS Save/reopen와 외부 변경 시 기존 파일 보존, 두 Parent Bundle의 독립 전개·합성.
- Python Parent focused 4 tests PASS. C++/Python 전개를 같은 입력으로 교차 대조했고 실제 실행 field가 일치했다. default/counter 직렬화 차이는 실행 차이로 취급하지 않았다.
- 실제 Client project Debug ClCompile와 기존 227개 link input을 사용한 out 전용 Client 링크 PASS. 기존 인코딩 경고가 있었으며 새 compile/link 오류는 없었다.
- 같은 Client object closure에 CPU 전용 main을 연결한 Workbench probe PASS: Parent 생성, Append, 겹침/잘못된 duration/참조 원본 삭제 거부와 이전 draft 보존, 반복, Duplicate/Delete, 확장 Preview request, source와 독립된 scratch Save/reload, 실제 Bundle에 Parent를 저장한 뒤 재열기·scrub·확장 snapshot 생성까지 통과했다. 게임 main과 UI는 실행하지 않았다.
- 실제 Server Debug build/link PASS. out 전용 Server의 `--kouku-object-overlap-contract-test`는 failures 0. 새 자식 deadline 취소와 Parent 판정 유지, 자식 댄스 HUD 구간 검사를 포함했다. 실제 `CKoukuSaydonBrain`과 로드한 `CGameplayCatalog`를 연결한 fixedTimeline 검사도 PASS: 34/34/14932 ms Stage가 누적 tick 2/3에 전환하고 정확히 450 tick(15초)에 끝났다. 시작 tick 100과 uint32 wrap 경계 모두 동일했다. fixture는 bootstrap을 쓰지 않았다.
- Gameplay publisher PowerShell AST, 변경 파일의 `git diff --check` PASS.

검증 산출물은 `out/KoukuParentTimeline20260912/`에 둔다. `client-clcompile.log`, `link-client.log`, `link-probe.log`, `workbench-probe/run.log`, `server-build.log`, `fixed_timeline_run.log`, `composition_crosscheck_*`를 사용한다. Document 61개 검사의 소스와 로그는 `out/KoukuParentPattern20260912/parent_probe.cpp`, `parent_probe.run.log`에 있다. 초기 standalone Workbench 링크는 UI 의존 심벌이 빠져 실패했고, 최종 검사는 실제 프로젝트의 전체 link closure를 사용했다.

## G03. 배포와 사용자 확인

최종 Client Debug ClCompile와 실제 기본 경로의 Client Link/Server Build가 통과했다. `Client/Bin/Debug/Client.exe`, `Server/Bin/Debug/Server.exe`를 갱신했다. 검증한 `/O1` Decal CSO 793,187 bytes를 기본 Debug 실행 폴더에 복사하고 hash 일치를 확인했다. 로그는 `client-clcompile.log`, `final-client-product-link.log`, `final-server-build.log`다. `MSBuild /t:Link` 단독 호출은 오류 없이 반환했지만 실제 기본 EXE를 갱신하지 않아 최종 증거에서 제외했다. 현재 프로젝트의 227개 실제 입력으로 기본 Debug 경로에 다시 링크했으며 Client.exe는 15:41:20에 갱신되어 모든 입력 object보다 최신임을 확인했다.

Composition revision 352에서 공식 `Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision 352`를 실행했다. Kouku product, 해당 Area map, world gameplay, gameplay balance 네 domain이 성공했다. Product는 31 patterns/246 stages/8 bundles이며 저장 저작 목록은 36 patterns/10 bundles다. 사용자 DRAFT를 임의로 게시 승인하지 않았다. 로그는 `final-publish.log`다. 최종 JSON/XML parse와 변경 범위 `git diff --check`도 통과했다.

이번 종료 범위에는 Parent 편집·실행 계약, 작은 오망성, 노란 예고 3종의 Composition/Client project 등록, Showtime 총 두 개와 map 소품 네 개의 원본 재질 연결·공통 추출/적용 경로, source attachment 오류 11개 검사가 포함된다. 세부 결과는 [작은 오망성](2026-09-12_KOUKU_SMALL_PENTAGRAM_FLOOR_IMPLEMENTATION_RESULT.md), [노란 예고](2026-09-12_KOUKU_SHOWTIME_WARNING_GROUPS_IMPLEMENTATION_RESULT.md), [재질 적용](2026-09-12_WORLD_OBJECT_SOURCE_MATERIAL_IMPLEMENTATION_RESULT.md), [source attachment](2026-09-12_KOUKU_RESOURCE_SOURCE_PREVIEW_RESULT.md)를 따른다.

사용자의 마무리 요청에 따라 CuttingBlade/HornClown/Trumpet/LaserCannon 네 소품의 신규 재질 셰이더 작업은 중단했다. 해당 후보는 제품에 설치하지 않았고 기존 재질은 보존했다. 노란 부채꼴은 저작 조합이며 원본 엔진 노이즈의 UV 흔들림은 미복원이다. 마리오 전체 시간표 조립과 1~4 진입 카운터도 이번에 추가하지 않았다.

이번에 확인·갱신한 구성은 Debug다. 후속 ZIP 갱신 요청에 따라 2026-09-12 15:47:34에 바탕화면 `C:/Users/user/Desktop/GB_Resources.zip`을 갱신했다(335,339,910 bytes). 새 공식 runtime ZIP은 현재 EXE/DLL/CSO 및 양쪽 Bin/DataFiles 363개를 담으며 113개 CSO와 793,187-byte Decal을 포함한다. 모든 manifest hash와 현재 실행 폴더의 일치를 확인했다. 외부 ZIP 751개 엔트리 중 runtime/README 두 개만 교체했고 나머지 749개는 SHA-256으로 보존을 확인했다. 바탕화면 추출 폴더와 out 포장 사본도 갱신했다. 기존 폴더 선택 설치기와 바로가기는 유지한다. 검증 증거는 `out/RuntimeDelivery20260912/session-delivery-result.json`이며 이전 ZIP은 같은 out 폴더의 `GB_Resources-before-session-update.zip`으로 보존했다. 선택하는 LostArk 정본 폴더의 Data도 이번 변경과 일치해야 한다. runtime ZIP은 Data 저작 정본과 Resources를 담지 않는다. Git commit/push는 수행하지 않았다.

최종 확인 시 Client/Server는 실행 중이지 않았다. 에이전트가 종료·실행하지 않았다. 이 PC는 LAN `server-host`이므로 Visual Studio의 `Server + Client` profile을 사용자가 직접 시작한다.

사용 경로는 Action Workbench의 KoukuSaydon → Parent 선택 → `Create Parent Timeline` → Pattern 행의 `Append Pattern at Cursor`다. Parent duration과 자식 창을 편집한 뒤 Save → Publish All Patterns → Server 재시작으로 실제 재생한다. 작은 오망성 상세 수치·입력과 사용자 확인 경로는 [작은 오망성 결과](2026-09-12_KOUKU_SMALL_PENTAGRAM_FLOOR_IMPLEMENTATION_RESULT.md)를 따른다.

Client/UI 조작·화면 캡처·최종 시각 판정은 수행하지 않았다. 최종 화면과 입력 동작은 사용자 확인 대기다. 대규모 다른 작업의 미커밋 변경을 보존했으며 자동 stage/commit/push하지 않았다.
