# Kouku Parent 타임라인과 Pattern 행 구현 결과

## G04. Parent 선택·Append 버튼 진입 보완

최종 Product Debug 빌드는 2026-09-12 18:00:06에 PASS했다. Engine·Shared·Server·Client 모두 성공했고 새 Client.exe에서 상단 Append/Sequencer 생성 버튼 문자열과 변경 source/object 최신성도 확인했다. 현재 켜진 out 실행 복사본은 이전 빌드이며 새 UI의 사용자 확인은 재실행 뒤 진행한다. 기록은 `out/KoukuPopupFinale20260912/implementation/final-build-verification.json`이다.

2026-09-12 후속 사용자가 Parent 선택 뒤 sequencer가 비어 보이고 `Publish All Patterns` 오른쪽 Append 버튼이 없다고 보고했다. 소스에서 미생성 Parent는 FOLDER 선택으로 남아 Render_Timeline이 바로 반환했으며, `Create Parent Timeline`과 `Append Pattern at Cursor`는 Details 안에만 존재했다. 기본 Debug Client.exe에도 이 기존 문자열이 있었으므로 기능 누락을 구버전 EXE 문제로만 설명하지 않는다.

`KoukuSaydonActionWorkbench.cpp/.h`에 다음을 반영했다.

- Publish 바로 오른쪽에 `Append Pattern` 버튼과 자식 선택 팝업을 연결했다. 유효한 Parent 또는 그 backing Pattern을 선택하면 활성화한다.
- 미생성 Parent에서 자식을 고르면 같은 actor의 15000ms backing timeline과 첫 Pattern row를 한 candidate에 준비하고 기존 전개·검증 후 한 번에 commit한다. 기존 Append_PatternBox가 Parent folder ID도 소비하며 실패한 시도는 folder·Pattern·ordinal·기존 draft를 보존한다.
- Parent만 선택한 상태에는 sequencer 안에 생성 안내와 `Create Parent Timeline`을 표시한다. 선택 자체는 저장 문서를 변경하지 않는다.
- 생성·Append 성공 뒤 backing Pattern과 새 박스를 선택한다. 기존 Pattern lane의 최소 한 행, 공통 Animation/Logic/World/Effect 등의 행, 원본 Pattern 편집과 Parent 복귀를 유지한다.

기존 Workbench CPU probe를 `out/KoukuPopupFinale20260912/implementation/parent-entry-probe/`에서 확장하여 실행했다. 현재 CPP/H로 Workbench와 CompositionDocument를 직접 컴파일하고 현재 Client의 기존 link closure로 out 전용 console probe를 연결했다. 게임 main과 ImGui/UI는 실행하지 않았다.

검증 결과는 다음과 같다.

- 잘못된 관문 자식, Parent 수명을 넘는 첫 배치, 없는 자식 ID의 첫 Append 모두 거부하고 전체 draft 비교가 동일했다. backing timeline과 ordinal도 남지 않았다.
- 첫 정상 Append는 backing Pattern 하나와 첫 occurrence를 생성했고, 기존 timeline 재열기는 문서를 변경하지 않았다. 자식 원본 선택 후 Parent 재열기, 기존 중복·겹침·삭제·길이 거부, 반복·Duplicate/Delete·Preview·scratch Save/Reload·Bundle 전개가 통과했다.
- 현재 `KoukuSaydonSequenceComposition.json`을 실제 `CKoukuSaydonCompositionDocument::Reload`로 읽어 revision7, Pattern7개, 항목 quarantine0을 확인했다.
- 변경 소스·PLAN/RESULT의 `git diff --check`가 통과했다. 기존 Engine 헤더의 C4828 인코딩 경고가 있었고 새 컴파일·링크 오류는 없었다.

최초 probe는 다른 작업이 추가한 CompositionDocument 헤더 필드와 과거 object를 혼용하여 비교 실패가 있었다. 해당 두 object를 현재 소스로 다시 컴파일한 뒤 모든 검사가 통과했다. 이를 제품 rollback 실패나 화면 PASS로 기록하지 않는다. 로그는 `KoukuSaydonActionWorkbench.compile.log`, `KoukuSaydonCompositionDocument.compile.log`, `link.log`, `run.log`다. 최종 Product 빌드는 위 기록과 같이 완료됐으며 사용자 버튼 조작·최종 화면 확인은 별도다.

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

## G04. Publish 중 타임라인 입력 제한 해제

2026-09-12 사용자 보고 직후 실제 publisher를 조사했다. Client 70792에서 시작한 PowerShell 26648은 revision 377을 대상으로 20:03부터 Kouku Product, Map, World Gameplay, Gameplay Balance를 순차 처리했고 20:09:17에 마지막 domain이 PASS로 끝났다. `out/KoukuSaydon/KoukuSaydonComposition.70792.379696390.publish.log`가 증거다. 사용자는 그 뒤 기존 실행 화면에서 입력이 다시 동작한다고 확인했다. 따라서 이번 원인은 완료 후 handle 누수가 아니라, 긴 게시 시간 동안 `Render_Timeline`이 `!publishing`으로 선택·scrub·초안 편집까지 막던 구조다. 실행 중 EXE가 즉시 교체되어 복구된 것은 아니다.

`KoukuSaydonActionWorkbench.cpp`에서 해당 입력 제한과 lifetime·선택 조작·Parent 생성/Append의 publish 제한을 해제했다. 기존 text input/active item/marquee 조건과 candidate 검증을 유지한다. Save/Reload, 중복 Publish, Server Complete Play의 보호는 유지한다. 게시 진행 중에는 저장 revision·경과 시간·Save 대기 안내를 타임라인에 표시한다. 성공 후 dirty가 남아 있으면 해당 편집은 미저장 상태이며 Save/Publish가 필요하다고 안내한다. 정상·실패 완료는 초안 Reload나 선택 초기화를 하지 않는다. 기존 UTF-8 BOM 없음/CRLF 인코딩과 다른 세션의 Parent·Albion 변경을 보존했다.

검증은 `out/KoukuPublishTimeline20260912/`에 보존했다.

- 현재 Workbench/Composition 단독 컴파일과 공식 Client Debug x64 `/t:ClCompile` PASS (`client-clcompile.log`).
- 기존 실제 Workbench CPU probe에 UI 없는 자식 프로세스 상태를 연결했다. 실행 중 box 선택·900ms scrub·window 편집, Save/중복 publish 거부와 격리 원본 bytes 보존을 확인했다. exit 0/17 양쪽에서 완료 후 draft/dirty/선택/커서 보존과 Save 재개 PASS. 성공 시 inventory refresh가 한 번만 전달됐다 (`run.log`, `validation_summary.json`). UI pointer hit test나 화면 입력을 실행한 검사는 아니다.
- 공식 intermediate의 실제 전체 입력을 사용한 Windows Client 링크 PASS. 수정본은 `out/KoukuPublishTimeline20260912/ClientBuild/Client.exe`이며 `ClientBuild/link.rsp`, `ClientBuild/link.log`에 명령과 결과를 보존했다. 기존 과거 tlog에는 이미 없어진 out Shared.lib 경로와 여러 command record가 남아 있어 최초 응답 파일 링크가 실패했고, 검증된 현재 Client 입력 목록과 `Shared/Bin/Debug/Shared.lib`로 최종 링크했다.
- 변경 범위 `git diff --check` PASS. 제품 JSON/schema 변경과 publisher 재실행은 하지 않았다. 테스트 Save는 out의 격리 Data 사본에서만 수행했다. 실제 Composition은 작업 중 사용자의 저장으로 revision 377→381이 되어 원본 전체 hash 불변이라고 기록하지 않는다.

사용자는 현재 Client를 계속 사용하겠다고 선택했다. 따라서 기본 `Client/Bin/Debug/Client.exe`와 실행 중 Client 70792/Server 37236은 교체·종료·재실행하지 않았다. 기본 EXE는 18:11:36 빌드 그대로이고 이번 수정은 실행 중 앱에 아직 적용되지 않았다. 사용자가 Client를 종료한 뒤 다음 Debug x64 제품 빌드를 수행하면 기본 EXE에 반영할 수 있다. 별도 ClientBuild는 링크 결과 보존용이며 런타임 DLL/Resources를 복제한 실행 패키지가 아니다. 수정본의 실제 화면 입력 확인은 사용자 확인 대기다.

## G05. 6분 게시 시간 조사 — 최적화 미반영

추가 질문에 따라 코드 변경 없이 게시 비용을 조사했다. rev377 당시 receipt를 후속 rev382 transaction의 rollback 백업에서 읽었다. Kouku product 165,253ms, world gameplay 25,617ms, gameplay balance 181,598ms로 action 합계 372,468ms다. 맵은 REUSED였다. 최초 프로세스 시작 20:03:00.7456부터 마지막 receipt 20:09:17.1795까지 약 376,434ms이며, action 외 비용은 약 4초다. 후속 게시 성공 뒤 해당 임시 백업은 정상 정리됐다. 읽은 수치는 `out/KoukuPublishPerformance20260912/phase-timings.json`에 기록했고 최신 rev382 receipt와 구분했다. 4개 domain의 fingerprint에는 Resources 전체 팩 전수 hash 경로가 없으므로 이를 6분의 주원인으로 분류하지 않는다.

`project_kouku_saydon_composition.py::prepare_publication`은 패턴 후보와 번들 후보마다 `validate_document → validate_publishable → projected_outputs`를 호출한다. `validate_publishable`은 내부에서 `validate_document`를 다시 호출하고, 번들 후보는 ready 전체 패턴을 포함한다. 각 검증·생성 호출 사이에 공용 입력 재사용이 없다. 현재 World Sequence 원본은 14,353,773bytes이며 같은 Object 참조 검증에서 반복 parse한다.

모델 cache도 `project_encounter`의 `bone_cache`, `project_presentation`의 `native_actor_cache` 안에서 매 호출 새로 만들어진다. 조사 시 Pattern 35/36의 source trim은 MN_RPCT_05 WModel 로드를 요구한다. 실제 모델은 197,135,136bytes(약 188MiB)이며 `verify_dimensionmaster_summon_bind_pose.py::read_wmodel`은 전체 파일과 정점·모든 애니메이션 키를 Python 객체로 해석한다. 후보별 전체 변환이 이 작업을 반복한다. 이 함수의 개별 CPU 비중은 계측하지 않았으며 165초 전체를 모델 읽기 시간으로 기록하지 않는다.

마지막 `Publish-GameplayBalance.ps1:56`은 같은 projector를 `--mode validate`로 다시 호출한다. 그 mode도 출력 비교 전에 전체 `prepare_publication/projected_outputs`를 다시 수행한다. 이 후단은 쿠크만이 아니라 현재 player 6종, skill 230개, damage profile 109개 등 공용 Gameplay bootstrap 검증·생성도 수행한다. 저장 JSON 한 번 쓰기와 다른 범위다.

개선 대상은 게시 1회 안에서 공용 모델/JSON을 재사용하고, 개별 오류 격리를 보존하면서 번들마다 전체 출력을 재생성하는 중복을 제거하며, 같은 transaction의 확정된 입력·출력 검증 결과를 후단에서 재사용하는 것이다. stale 원본 감지와 atomic rollback은 유지해야 한다. 이 성능 변경은 아직 구현·게시하지 않았다. G04에서 완료한 것은 편집 잠금 해제이며 게시 처리 시간 감소로 보고하지 않는다. 사용자의 진행 중 게시와 경합할 수 있는 추가 전체 측정은 시작 직후 중단했고, 사용자 게시 프로세스는 건드리지 않았다.

## G06. Publish 공통 입력 재사용 반영과 전체 절차 검증

사용자의 수정 요청에 따라 G05의 게시 병목을 기존 Python projector와 공통 WModel reader, World publisher에서 수정했다. 이번 변경은 게시 도구의 입력 처리이며 게임의 모델 로딩·렌더링 런타임 변경은 아니다. 한국어 표시명과 특정 모델 ID에 특례를 추가하지 않았다.

- projector는 한 호출 안에서 JSON/text, catalog join, 모델, 본 pose와 동일 후보의 변환 결과를 재사용한다. ContextVar 세션은 호출 종료 시 폐기하고 다른 root·다음 게시로 넘기지 않는다. 후보별 오류 격리·dependency closure·원본 및 Product 검증은 유지한다. Gameplay Balance는 여전히 같은 projector를 다시 검증하며 이 호출에도 최적화가 적용된다.
- 기존 `read_wmodel`의 기본 호출은 전체 vertex·animation key를 그대로 decode한다. publisher의 timing 검사는 geometry 없이 clip metadata만, 본 sampling은 필요한 clip key만 읽는다. 생략한 배열은 `None`으로 구별하고 누락 payload를 요구하는 검사는 거부한다. section/span·bone reference·중복 검사는 생략하지 않는다.
- World publisher도 반복 참조한 JSON을 한 번 parse한다. Python JSON/text는 원문 bytes, 모델은 SHA256을 교체 전후 다시 확인한다. World JSON은 파일 version과 `Ordinal` 원문 비교를 사용한다. 원본 변경이나 부분 교체 실패는 기존 제품을 복구한다. 파일 size·mtime만 유지한 변경도 거부한다.
- `BuildDomains.json`에 실제 import 도구인 공통 WModel reader와 Gameplay Balance의 light validator를 포함했다. 이 도구 변경 시 기존 receipt를 그대로 재사용하지 않는다. 별도 지속 cache와 새 검증 생략 옵션은 추가하지 않았다.

같은 revision 382 입력을 고정한 전후 비교는 `out/KoukuPublishPerformance20260912/comparison.json`에 보존했다. 직접 `prepare_publication + projected_outputs + 최종 입력 검사`는 176.023초에서 2.407초로 줄었다. JSON decode는 567회에서 7회, 모델 decode는 48회에서 6회다. 모델 읽기는 4.950GB에서 0.968GB로 줄었으며 후자는 최종 SHA256 검사 읽기까지 포함한다. 관측한 최대 working set은 이전 5.36GB 이상, 수정 후 278.5MB다. 생성 Product 두 개 및 product/inventory가 bytes까지 동일하며 snapshot Data hash도 바뀌지 않았다. 중간 수정본의 1.877초 수치는 최종 결과로 사용하지 않는다.

World publisher 전후 비교는 별도 동일 fixture에서 43.596초에서 1.859초로 줄었고 출력 6개가 모두 동일했다. 길이와 mtime을 유지한 U+200B→U+200C 원문 변경 거부, 출력 3개 교체 후 의도적 실패와 기존 출력 전체 복구도 PASS다. 기록은 `world/before.json`, `world/after.json`, `world/snapshot-tests.log`다. 이는 과거 실행에서 관측한 World 25.617초와 다른 측정이므로 같은 기준으로 합산하지 않는다.

실제 `Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision 382`도 `out/KoukuPublishPerformance20260912/pipeline`의 독립 Data·출력 사본에서 실행했다. Resources만 읽기용 junction으로 참조했고 정본 데이터나 실제 실행 중 앱의 제품 파일을 교체하지 않았다. 결과는 Product PASS, Map REUSED, World PASS, Gameplay Balance PASS이며 전체 80.475초였다. receipt의 action 시간은 Product 5.832초, World 3.943초, Gameplay Balance 43.694초이며 나머지는 owner의 입력/receipt/transaction 처리 시간이다. 기존 fixture 출력 103개가 모두 SHA256까지 동일했다. `pipeline/owner-publish.log`, `owner-publish-result.json`, `output-comparison.json`이 증거다. 전체 게시를 2.407초로 기록하지 않는다.

관련 projector 검사 14개, reader 검사 13개가 PASS다. 실제 WModel 4개에서 기존 full reader의 모든 vertex/bone/clip/key 값과 기본 호출 결과가 일치했다. 전체 test suite PASS는 아니다. 기존 `test_bone_contact_bakes_real_hammer_and_body_with_target_yaw`는 현재 Parent Pattern32를 남기는 오래된 축소 fixture의 lifetime 오류로 before/after 모두 실패했고, 기존 DimensionMaster exact receipt 검사는 원래 없던 receipt 파일 때문에 실행할 수 없었다. 해당 원본과 검사를 완화하지 않았으며 세부 범위는 `projector-validation.json`, `reader-validation.json`을 따른다.

후속 격리 Gameplay `Validate` 한 번은 12.675초에 PASS했다. 같은 domain 인자의 `-SkipValtanSplitProjection`을 유지했으므로 최초 Valtan Project는 이 경로에서 실행되지 않는다. Kouku Python 검증 2.804초, PS balance/boss 0.484초, Valtan pattern 0.727초, Kouku pattern 1.628초, 기타 PS 검증 5.982초, presentation Python 0.293초, row sort 0.640초였다. 이 별도 Validate 수치를 전체 owner의 Publish action 43.694초와 같다고 간주하지 않는다. 남은 Publish 전용 처리 및 owner 실행 환경의 차이는 분리 실측하지 않았으므로 전체 80초를 완전히 제거했다고 보고하지 않는다. 변경 JSON/PowerShell parse와 대상 source·문서의 `git diff --check` PASS이며 현재 실행한 검증 요약은 `validation-summary.json`이다.

사용자가 out 구버전 실행을 명시 요청하여 20:31에 `out/InteractiveRuntime/20260912_174839`의 Client와 보이는 Server CMD를 실행했다. 당시 Client 10484, Server 63264와 7777 listener를 확인했다. 저작 Data와 Resources는 정본을 사용했으며 실행 기록은 해당 폴더의 `last-launch.json`이다. 이후 두 프로세스가 종료된 것을 관측했고 에이전트는 종료·재실행하지 않았다.

후속 사용자가 다른 최적화 빌드를 직접 실행했다고 알렸다. 현재 확인한 Client 15004는 기본 `Client/Bin/Debug/Client.exe`의 20:50:54 빌드이며, Workbench object는 20:49:20에 갱신됐다. 실행 바이너리에 G04의 publish 중 편집 안내 두 문자열이 포함돼 있고 호출 소스는 정본 Data root의 `Tools/Build/Invoke-BuildDomainOwner.ps1`를 사용한다. 따라서 현재 Client에는 입력 제한 수정이 포함되며 다음 Publish는 수정된 도구를 소비한다. Server 80532도 기본 Debug 경로에서 실행 중이다. 다른 최적화의 내용·성능이나 사용자의 실제 UI 입력 결과를 이 검사로 대신 PASS 처리하지 않는다. Client/UI 조작·화면 캡처는 하지 않았다.

## G07. Pattern Resources에서 추가할 소스 선택 분리

`3관문_쿠크세이튼_마리오`는 실제 folder.15 → Pattern37, duration15000ms인 Parent였다. Composition Patterns에서 소스를 고르면 `Select_PatternById`가 편집 대상을 바꿔 기존 Parent 전용 Append 버튼이 비활성화되는 구조를 확인했다. Workbench H/CPP에서 기존 계층 렌더러를 `Render_PatternTree(resourcePicker)`로 공유하고 Composition Resources에 `Pattern` 탭을 추가했다. Gate/Parent/Bundle 탐색과 Pattern leaf 선택은 resource 모드에서 편집 대상, cursor, preview, 생성 목적지를 바꾸지 않는다. source는 기존 session `m_strAppendPatternId`만 사용하며 JSON이나 이중 Pattern 정의를 만들지 않는다.

상단 및 Details의 `Append Pattern`은 동일 Resources 탭으로 연결한다. 기존 shell의 view request로 닫힌 Resources도 표시하고 timeline 최대화를 해제한다. 실제 `Append Pattern at Cursor`는 현재 Parent/source의 관문, 모델·target boss, 정상 문서와 수명, Parent 끝을 다시 확인한다. 미생성 Parent는 기존 첫 append transaction을 유지한다. source 전체 수명, 실제 start/window/end와 잘림 안내를 표시한다. 마리오 2페이즈의 26672ms를 현재 15000ms Parent에 넣으면 남은 창만 배치되므로 전체 재생이 필요하면 Parent lifetime을 먼저 늘린다. 겹침·중첩·전개 실패는 기존 Append transaction이 거부하며 초안을 보존한다.

`out/KoukuPatternResources20260912/validation-summary.json`에 실행 결과를 보존했다.

- 공식 Client Debug x64 `/t:ClCompile /p:BuildProjectReferences=false` exit0. 기존 헤더의 C4819/C4828 경고가 있으며 오류는 없다. 변경 H를 소비하는 실제 호출자도 다시 컴파일했다.
- 현재 Client 프로젝트의 실제 226개 object와 기존 링크 설정으로 `ClientBuild/Client.exe` 링크 exit0. 기존 DirectXTK PDB 누락 등의 경고는 남는다. 현재 default Client15004/Server80532는 종료·재실행하지 않았고 기본 EXE는 변경하지 않았다. 별도 EXE는 링크 증거이며 DLL/Resources 실행 패키지가 아니다.
- 기존 실제 Workbench CPU probe를 재사용해 격리 Data 사본에서 실행했다. Parent37의1000ms에서 source34→33→34 전환 후 대상·커서·초안 유지, source34의 실제 append와 새 occurrence 선택, 원본 source 보존, Save/Reload의 stable ID 보존이 통과했다. missing source, Parent source, 다른 관문, 일반 Pattern 대상, cursor=Parent 끝 거부 및 source33의 실제 overlap append rollback도 통과했다. source 선택은 session 상태 주입 뒤 실제 resolver를 호출했으며 UI 클릭·레이아웃 검증은 아니다. 게임 main, renderer, Client/UI, publisher는 실행하지 않았다.
- UTF-8 BOM 없음/CRLF 보존, 기존 project/filter 등록 확인, 정본 Composition 파일 SHA256 보존, `git diff --check` exit0. 제품 JSON/schema/XML 변경은 없어 새 parse 대상이 없다. 테스트 Save는 out Data 사본에만 기록했다.

사용자 경로는 수정본 빌드로 시작한 뒤 `Composition Patterns → 3관문 → 3관문_쿠크세이튼_마리오 [Parent] → Append Pattern → Composition Resources / Pattern → Pattern Tree에서 자식 패턴 선택 → Append Pattern at Cursor → Save`다. 기존 dirty 작업을 저장하고 Client를 종료한 뒤 기본 Debug Client를 다시 빌드해야 현재 실행에 반영된다. 이 PC는 LAN server-host이며 사용자는 `Server + Client` profile을 사용한다. 사용자 화면 확인은 대기 상태이며 자동 stage/commit/push는 수행하지 않았다.
