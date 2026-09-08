# PR #343·#342 통합 결과

## #343 충돌 해결

- 기준: 사용자 PR #343 `c1e7b50c`와 main `d191f839`(카드미로 PR #341). 원래 Desktop checkout과 실행 중인 Client/Server는 유지했다.
- 카메라 revision 78, World Sequence revision 412로 병합했다. 양쪽 stable ID를 모두 유지했다: objectResources 11, templates 95, instances 131. 숫자 출력 형식 차이는 JSON 값으로 비교했다.
- Kouku encounter와 patternbindings는 정본 Composition revision 176에서 재생성했다. 두 출력의 충돌은 revision뿐이었으며 정본 projection과 나머지 필드는 동일했다.
- protocol 72는 main의 카드미로 snapshot·조각 피드백과 #343의 FINISH_OWNER를 함께 지원한다. 기존 packet 번호와 STOP_OWNER/FINISH_OWNER 구분을 유지했다. 다른 protocol 실행 파일과 혼용하지 않는다.
- Client 프로젝트의 차원술사 S 복원 문서와 카드미로 V2 문서 등록을 모두 유지했다.
- Composition 배포 검증에 기존 Map v3 effectTracks와 Camera activation/defaultHoldMs/transitionEasing 계약을 연결했다. 허용 필드·ID·시간·TRS·binding 검증을 유지하고 원본 track을 보존한다.

## 수행한 검증

- 별도 worktree Shared·Server·Client Debug 컴파일 및 링크 성공. Engine 소스는 #343과 차이가 없어 동일 Desktop 빌드의 Engine DLL/lib/PDB/CSO를 별도 경로에 복사하여 사용했다. Client 셰이더는 별도 경로에서 컴파일했다. 기존 인코딩·셰이더·third-party PDB 경고는 남아 있다.
- NetworkProtocolHarness `--mario-controls-only`, `--kouku-bundle-only` 성공. 카드미로 snapshot, 소유 연출 종료·잘린 payload의 기존 상태 보존을 확인했다.
- `Server.exe --card-maze-contract-test`: failures 0. 최초에는 새 worktree의 Navigation/Gameplay/보상 bootstrap이 없어 실패했고, 기존 publisher로 준비한 뒤 재실행하여 통과했다. 서버 listener나 Client/UI를 실행한 검사는 아니다.
- MapAuthoring, WorldGameplay, Navigation, GameplayBalance, ItemCatalog, ValtanClearRewards, Kouku projection, Composition publisher 성공. Resources는 원본 폴더를 입력으로만 사용했다.
- 변경 JSON/XML parse, 병합 데이터 stable ID 보존, `git diff --check` 확인. 상세 로컬 로그: `out/pr-integration/`.

## #342 통합 구현

- #343은 main에 `b76d4a9b`로 병합됐다. 그 main을 #342에 통합했다.
- F1에 `Effect Tool V1`, `Effect Tool V2` 버튼을 각각 제공한다. visibility·focus·typed resource open·닫기 경로를 분리하고 V1을 열 때 V2를 생성하지 않는다.
- V2의 CPU pane·Resource tree·Sequencer를 V2 owner로 옮겼다. 각 창·기본 sequence ID는 별개이며, 공용 모델의 animation clock은 마지막으로 재생한 sequencer 하나만 소유한다.
- V1의 metadata 지연 로드·clipper·선택 항목 저장/refresh 최적화를 유지했다. V2도 CPU draft를 닫을 때 지우지 않고 native preview 편집값은 capture 후 다시 열 때 복구한다. 저장 파일의 CAS 검사는 유지했다.
- PR #342의 live arena boss attachment와 등장·레이저·망치 이펙트/clip bindings를 유지했다. 빌린 boss는 runtime ignore, clip 변경·pause·scrub·직접 transform 이동 대상에서 제외했다. 에디터가 만든 preview target은 기존 조작을 유지한다.
- 레벨 변경 두 경계에서 이전 frame presentation을 정리한다. V2 thumbnail budget은 CPU/기존 native 패널을 그리기 전에 한 번 초기화한다.
- Data 문서 12개(leaf 8, group 3, binding 1)를 기존 `96.DataFiles` None 항목에 등록했다. 신규 C++ 파일은 없다.
- Gameplay의 기존 105 placement를 유지하고 Big Saydon의 PR #342 위치 `(10.24, 10, 317.75)`를 반영했다(revision 8471). 기존 World publisher로 viewer/bootstrap을 재생성했다. Resources payload를 Git에 추가하지 않았다.

## #342 검증

- 최종 Client Debug 컴파일·링크·DLL/CSO 배포 성공. `client342-build.log`와 최신 Effect_Tool_V2.obj timestamp로 마지막 thumbnail reset/미사용 bridge 제거까지 컴파일에 포함됐음을 확인했다.

- V2 binding/group 검사 22/22 PASS. main의 카드미로 group 8개에 대한 개수·수명 기대값을 신규 쿠크 group들과 함께 유지했다.
- 독립 도구·resource facade 계약 25/25 PASS. CPU draft 저장은 GPU preview나 전체 보스 admission을 요구하지 않는 기존 계약을 유지한다.
- Composition 전체 검사 33/33 PASS(140.433초). 앞선 전체 검사는 중간 branch 전환과 겹쳐 28 PASS/5 ERROR였으며 완료 근거로 사용하지 않았다. 데이터 고정 후 전체를 다시 실행해 성공했다.
- Map/World와 Composition publish, 변경 JSON/XML parse 및 diff 공백 검사를 확인했다. MN_RPCT_06의 clip-keyed formatVersion 1은 현재 native codec이 지원하는 NPC/Kouku 계약이다. BOSS_VALTAN 전용 v2 CLI를 해당 문서의 validator로 사용하지 않는다.

## 사용자 확인

- 최종 UI·아레나 화면은 사용자가 직접 확인한다. F1 → `Effect Tool V1` / `Effect Tool V2`를 각각 열고 복원 파일·V2 leaf/group을 확인한다. 양쪽 도구를 닫았다 다시 열어 draft와 선택을 확인한다.
- protocol 72 Server와 Client를 함께 빌드·재시작한다. 에이전트는 Client/UI를 실행·조작·캡처하지 않았다.
