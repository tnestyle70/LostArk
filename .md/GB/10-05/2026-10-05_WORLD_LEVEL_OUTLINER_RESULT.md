# 2026-10-05 World Level Tree·Object Details·편집 이력 결과

## 완료한 연결

기존 동기화 PR #523을 main `18d9bf92`에 합친 뒤 기능 브랜치에서 구현했다.
Interview는 main `0f898e3`과 일치하며 `기술소개서/자막.txt`를 확인했다. 자막 본문은 변경하지 않았다.

- World Level Tool의 Area inventory를 Level → category → source level/asset 또는
  pattern → occurrence 트리로 투영한다. 검색, 종류 필터, Ctrl/Shift 선택, 현재 선택 reveal과
  보이는 행만 그리는 clipper를 연결했다. 같은 Scene 선택의 반복 동기화가 다중 선택을 지우지 않는다.
- Parent 생성·이름 변경·이동·drag/drop·삭제·Undo/Redo와 당시 stable 선택을 복원한다.
  조직은 `Data/Maps/Authoring/WorldHierarchy/<AreaId>.json`에 저장한다. Parent 삭제는 자식을
  상위 Parent로 옮기며 TRS·Visible 상속은 없다. 미로드 assignment도 보존한다.
- 현재 Bern/Character Select/Valtan/Kouku의 트리 선택, Pick in world, MapTool의
  Open Object Details가 같은 live Object Details로 연결된다. Focus/F는 현재 pose와 모델
  bounds를 사용한다. 목록 선택과 실제 triangle hit 정보는 구분한다.
- map Pos/Rot/signed Scale/Visible은 기존 placement edit session과 Placements publisher로
  저장한다. 먼저 Enable map placement editing을 명시한다. 부분 로드에서도 원본 전체 draft를
  유지한다. MapTool과 Details의 양방향 인계는 미저장·게시·preview 복원 실패를 보존한다.
- 실제 CModel mesh/CMaterial, 정확한 material override의 Resources 상대 texture ID,
  식생 wind와 map water 입력을 읽기 전용으로 표시한다. 상속 WModel의 미노출 texture는 unknown,
  물의 auxiliary texture 선언은 실제 binder 연결과 구분한다. 새 렌더링 경로는 만들지 않았다.
- Effect Tool V1, Saydon, Valtan, World Object, Object Sequencer의 재생 toolbar 오른쪽에
  Undo/Redo를 추가했다. 값과 당시 선택·편집 cursor를 복원하고 drag를 한 단계로 묶는다.
  Ctrl+Z, Ctrl+Y/Ctrl+Shift+Z는 해당 창 focus에서 처리하며 텍스트 입력 중 가로채지 않는다.
- Map placement는 64단계의 stable-ID delta로 TRS·session duplicate 생성/삭제·선택을
  복원한다. 큰 map 전체를 프레임마다 복사하지 않는다. hierarchy, 각 편집 owner와 공통 이력은
  실패한 복원에서 현재 draft와 Undo/Redo stack을 보존한다.
- 일반적인 같은 문서 Save는 최신 저장 기준을 유지하면서 이력을 보존한다. 외부 WORLD animation
  편집은 이력 항목의 반대쪽 snapshot을 expected 상태로 검사한다. 다른 World 편집이 끼면
  오래된 Undo를 거부하며, World가 바뀌지 않은 로컬 편집의 Undo는 World를 덮지 않는다.

## 실제 검증

| 검증 | 결과 |
|---|---|
| 최종 Debug x64 Product | PASS, 70328 ms, `out/BuildPipeline/runs/20261005T041509517Z-debug-product.json` |
| 최종 Release x64 Product | PASS, 519778 ms, `out/BuildPipeline/runs/20261005T042416943Z-release-product.json` |
| 실제 공통 CEditorUndoHistory | Debug/Release 각각 15,048 checks, 0 failures |
| 실제 hierarchy writer + DataJson | Debug/Release 각각 243 checks, 0 failures |
| 실제 tree 메서드 추출 fixture | Debug/Release 각각 100,093 checks, 0 failures |
| 실제 map history 메서드 + 실패 주입 host/runtime 대역 | Debug/Release 각각 28 checks, 0 failures |
| 실제 World emission provenance 메서드 | 8 checks, 0 failures |
| Client project/filter XML parse, staged diff whitespace | PASS |

Product는 Engine/Shared/Server/Client 컴파일·링크·배포 및 실행 파일/Navigation/Item·Valtan reward
입력 검사를 통과했다. 두 최종 보고서 모두 missingRuntimeInputs/invalidRuntimeInputs는 비어 있다.
기존 C4819/C4828, 일부 형변환 및 외부 PDB 경고는 남아 있다. 경고 0건을 뜻하지 않는다.
Client/Server/UI는 실행하지 않았으며 데이터 publish도 수행하지 않았다.

native tree fixture는 50,021행을 보존하고 기본 collapsed visible 3행을 확인했다. 초기 구성은
Debug 533.558ms, Release 183.946ms였다. 이는 CPU fixture 수치이며 게임 FPS나 GPU 검증이 아니다.
공통 이력 검사는 삭제/선택 복원, saved baseline, 실패·재시도, drag 묶음, 분기, 제한,
5,000회 독립 cursor model 비교와 외부 World 편집 충돌의 expected 상태를 검사한다.

코드 검증 근거는 `out/WorldLevelOutliner20261005`, `out/WorldLevelHierarchy`,
`out/EditorUndoHistory`, `out/MapPlacementHistoryContracts`에 남겼다. 전체 반영 코드는 대응 PLAN에
기록했다. 기존 C++의 인코딩/BOM/개행과 기존 비ASCII 문자를 유지했고 새 파일은 UTF-8이다.
프로젝트/filter는 새 hierarchy H/CPP와 공통 history header, 조직 JSON None만 추가했다.
컴파일 산출물, EngineSDK, 개인 설정은 소스 commit에서 제외했다.

기존 main checkout을 통과하며 동일 입력의 시간이 바뀐 부분은 이전 성공 빌드에서 기록했던
701개 shader/project 입력에 한해서 hash와 기록 시각을 확인해 원래 시각을 복구했다.
`out/MainSync20261005/recorded-input-times-verification.json`에서 불일치 0을 확인했다.
기록 없는 C++ 입력은 정상 Build로 다시 컴파일했고 출력·tracking 파일은 수정하지 않았다.
Clean/Rebuild나 전체 CSO touch는 실행하지 않았다. 마지막 보완 뒤 Debug는 OBJ 24개,
CSO 0개를 갱신하며 정상 증분 빌드를 통과했다.

## 사용자 화면 확인과 남은 경계

1. Debug F1 → World Level Tool에서 현재 Area의 식생·물·폭포 배치를 트리로 선택하거나
   Pick in world로 고른다. Object Details의 mesh/material과 Focus/F를 확인한다.
2. map 편집을 활성화해 Pos/Rot/Scale/Visible을 바꾸고 Undo/Redo, Save를 확인한다.
   원본 배치는 Visible로 관리하며 Delete는 session duplicate에 한정한다.
3. Parent 생성·이동·다중 선택·이름 변경·삭제 후 Undo/Redo와 Save/재열기를 확인한다.
4. 각 도구의 삭제·복제·Box 수정·timeline drag 뒤 재생 toolbar의 Undo/Redo를 확인한다.
   Saydon/Valtan 단축키 focus는 재생·시퀀서 창 기준이다.

실제 화면·피킹·GPU 재질·서버 재생 결과는 사용자가 확인해야 한다. 자동 빌드나 synthetic fixture를
화면 검증으로 기록하지 않는다. Release의 기존 Debug 전용 저작 도구 경계는 유지한다.
Deploy SOURCE_EXACT 영구 배치는 보존하고 기존 가역 presentation preview만 사용한다.
Deploy preview 및 Server 재생 명령은 문서 Undo의 범위가 아니며 팀장 렌더링 옵션도 바꾸지 않았다.

명시 Reload/문서 교체, 외부 병합, Valtan의 다른 owner 변경 감지는 이전 이력의 경계다.
기존 New Pattern은 파일을 생성·저장하는 transaction이므로 완료 시 이력 경계를 유지한다.
이미 emission 삭제를 저장하여 연결 Collider/Logic 원본 행까지 지운 경우, Undo는 World draft를
복원하지만 제거된 외부 provenance를 임의로 재생성하지 않는다. 그 상태의 재저장은 안전하게
거부하고 복원 draft를 보존한다. 연결 Collider가 없는 emission의 복원 저장은 허용한다.
