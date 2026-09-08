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

## 남은 단계

- 검증한 #343 커밋을 PR에 갱신하고 병합한 뒤 #342에 통합한다.
- #342 V1/V2 독립 창, live boss attach, 이펙트 데이터를 검증하고 병합한다.
- 최종 화면 확인은 사용자가 Server+Client를 같은 최신 소스로 빌드·재시작한 뒤 수행한다. 에이전트의 시각 PASS는 기록하지 않았다.
