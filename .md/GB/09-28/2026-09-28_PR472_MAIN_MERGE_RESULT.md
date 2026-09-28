# G01 — PR472 main 통합과 보존 검증

병합 부모는 PR `82455a5375fb5a12126478f116466b049cecddc0`과 main
`8bae8fcf9c4f062dafa6f7a1f18f0bbb73268b33`이다. 18개 충돌 파일을 한쪽 선택으로
덮지 않고 각 변경의 소비자를 대조했다. main 직접 수정이나 history 재작성은 하지 않는다.

## G02 — 해결한 실제 충돌

- main의 작은 public header, 단일 CPP/generated INL 소유와 leaf shader dispatch 구조를 유지했다.
  PR의 주민 2개·원본 받침 4개 family packing/Base/Light 함수를 해당 구조 안에 추가했다.
- main Guardian 의상 1526, 모코모코1 1528, 모코모코2 1527은 유지했다.
  PR의 받침01만 1528에서 미사용 1532로 옮겼다. 받침02/03/04=1529/1530/1531과
  주민1474/1475는 유지했다. family 이름이나 배치 ID는 바꾸지 않았다.
- CPU/HLSL static 분류와 Base/Light 입력 packing을 1529..1532로 맞췄다.
  main의 모코모코1=1528은 skinned NPC이므로 static 분기에 들어가지 않아야 한다.
  registry·named-vector patch·probe도 같은 매핑이다. 총 추가 프로그램 registry는 128개다.
- 프로젝트/filter에는 양쪽 항목을 함께 보존했다. main 대비 차이는 필요한 None 항목 4개다.
  공통 gotchas·렌더링 문서는 양쪽 독립 추가 내용을 유지했다.
- 자동 병합된 주민 Light 입력, ocean translated-world 입력과 main의 sourcePosition 전달,
  map-forward compile guard, 렌더링 최적화·가디언·Guide·전투 코드는 함께 유지했다.

## G03 — 실행한 검사

- 마하라카 받침 계약 6, 받침/기둥 6, 워터팡 8, NPC 6, 누락 재질 5, 원본 coverage 5,
  SourceCharacter group 7, registry transaction 4: 총 47개 테스트 PASS.
  받침 계약에는 Guardian/NPC/stand/resident의 9개 family와 leaf dispatch 중복 방지 검사를 추가했다.
- Map publisher Check: LV_OCN_EVENTIS_MHP 4671 placements/7 files PASS.
  World publisher Validate: MAHARAKA 34 placements PASS. 이 통합에서 재게시하지 않았다.
- 부모 내용 대조: PR만 변경한 54파일과 main만 변경한 455파일의 내용 보존.
  Windows 줄바꿈은 정규화해 비교하고, mapassets/mapplacements 4파일은 LFS 원본 SHA256로 확인했다.
  변경 JSON parse 및 Client project/filter XML parse PASS.
- 충돌 leaf의 main Base/Light 함수 각29개는 본문 그대로 보존.
  PR의 추가6개와 모코모코1 각7개 함수는 필요한 함수 번호 외 본문 일치.
  main CPU family258개, PR 추가6개/모코모코1 packing, main named-vector133행과
  PR 대상6행도 번호 대응 후 일치. main의 기존 함수 개선을 옛 본문으로 되돌리지 않았다.
- 읽기 전용 독립 재검토에서 추가 통합 누락은 발견하지 못했다.
- 새 Debug Server의 localhost listening, TCP 연결과 10초 bounded smoke exit0 확인.
  최초 고정 2초 접속 시도는 초기화 전이라 거부됐고, 준비를 기다리는 bounded 재검사에서는
  약11.2초 후 TCP 연결에 성공했다. 서버 런타임 접속 실패로 은폐하지 않는다.
- `git diff --check MERGE_HEAD`와 이번 해결 파일의 diff 검사 PASS.
  HEAD 대비 incoming main 전체에는 기존 계획서 2개의 공백 경고8건이 있다.
  unrelated main 문서를 정리하는 변경은 추가하지 않았다.

- `Test-SourceCharacterShaderVariants.ps1 -Configuration Debug` PASS.
  새로 컴파일된 모델/Deferred CSO와 Engine을 격리 복사해 실행했다. 프로그램149,
  clone14, 실패경로9, light564, native144, afterimage4, unavailable10, combat12,
  numeric draw46, windowsCreated0이다. Guardian1526/NPC1527·1528을 기존 주민·받침
  검사에 추가해 양쪽 프로그램의 실제 binding/clone/bone 경로를 검사했다.
  이는 자동 계약 검사이며 사용자 visual fidelity 승인이 아니다.

- 정본 `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product
  -MaxCompilerProcesses 4` PASS. Engine/Shared/Server/Client 전부 성공했고 정상 post-build 배포 완료.
  Clean/Rebuild/컴파일 생략은 하지 않았다. 총1,897,654ms이며 최종 증거는
  `out/BuildPipeline/runs/20260927T155532902Z-debug-product.json`이다.
  missingRuntimeInputs/invalidRuntimeInputs 모두0. 기존 FXC/인코딩/외부 PDB 경고는 남아 있다.
- 최종 `Test-CompiledShaderClosure.ps1 -Configuration Debug -Modules Product` PASS:
  producer248/Client consumer163/Effect consumer142/resource-root8, WARP V1/V2 각1352.
  Client 빌드 도중 먼저 실행한 검사는 post-build 복사 전 Deferred hash 차이1건으로 실패했다.
  완료 후 Engine/Client Deferred SHA256 일치와 전체 재검사 PASS를 확인했다.

보존 검사/빌드/smoke/probe 근거는 Git 제외 `out/Pr472Merge/`에 있다.

## G04 — 배포와 남은 경계

마하라카 저작/게시 데이터와 Resources는 이번 병합에서 변경하지 않았다.
앞서 만든 CY_Resources２의 재질은 semantic family로 연결되므로 이번 native 번호 변경 때문에
리소스 pack을 다시 만들 필요가 없다. 기존 Drive 배포 필요성은 그대로다.

Client/UI 자율 실행·캡처·visual PASS 판정은 하지 않았다. 사용자는 새 Debug 빌드로
Server + Client를 직접 시작해 마하라카 주민·원본 받침·MapTool Camera를 확인한다.
Release/광역 FullDiagnostic 및 이전 쿠크 Server contract 실패의 재판정은 수행하지 않았다.
기존 미완료 워터팡 기능을 이번 충돌 해결로 완료 처리하지 않는다.
