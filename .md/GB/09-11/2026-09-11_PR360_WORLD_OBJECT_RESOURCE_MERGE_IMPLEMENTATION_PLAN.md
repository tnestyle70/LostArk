# PR360 World Object·리소스 통합 구현 계획

## G00. 기준선과 목표

사용자는 PR #360의 충돌을 해결하고 오늘 오후 2시 이후의 작업과 CY 추가 리소스를 비교해 main 및 로컬 Resources를 동기화하도록 요청했다. 갈고리·이동 불꽃은 World Object의 기존 저작·런타임 경로에서 관리하고 카드 병정, MIDNIGHT 맵과 쿠크 리소스의 유효한 추가분을 반영한다.

시작 worktree는 `codex/kouku-gate1-sequence-playback`의 `7795a68d867a8df5a840aa3d1f544cef8d825d48`, clean이다. origin/main은 `1b07290f`이며 GitHub 조회상 오늘 작업 PR #361은 아직 Open이다. PR #360 head는 `35ca86d615c2b40eedce908d25c8e3e82a6da259`, merge base는 `397aab6c`이다. 최신 7795의 모델·Effect worker, Profiler, 애니메이션, 컷신과 WorldSequence 변경을 보존한 통합 결과를 만든다.

별도 `codex/pr360-main-resource-sync`에서 먼저 origin/main을 합친 뒤 PR #360을 no-commit merge했다. 전체 ours/theirs 선택 대신 기능별 충돌을 해결한다. 검증한 결과는 PR #360 head branch에 fast-forward push하고 정상 PR merge로 main에 반영한다. force push와 main 직접 소스 편집은 하지 않는다. 원격 head가 바뀌면 새 변경을 먼저 확인한다.

## G01. 코드와 wire 계약

PR #360의 `objectMotion.emissions`는 기존 World Object/WorldSequence 안에서 positionOffset, yawDegrees, startDelayMs를 소유한다. Object Detail에서 편집하고 기존 Composition WORLD collider의 worldEmissionIndex가 실제 배치 하나를 선택한다. 갈고리/불의 여러 개별 World box를 이 계약으로 모으며 기존 Server authority와 renderer owner를 유지한다.

ClientReplication, Level_KakulSaydonArena, WorldObjectTool, WorldSequenceDocument/Player_Objects와 Map publisher 충돌에서 양쪽 동작을 보존한다. 자동 병합된 Character, WorldSequencePlayer, Shared/Server도 실제 소비자 연결과 기존 async load/프로파일러 계측을 대조한다. 새 별도 모델·World 런타임을 만들지 않는다.

양쪽 branch가 다른 wire 변경에 protocol 79를 사용했다. 오늘 branch의 CardMaze ENTRY_HIDDEN/VALID_FLAGS31과 PR의 MarioPoppedBallMask/CurseReleasedMask를 함께 유지하고 새 protocol 80으로 통합한다. snapshot read/write 순서, validation과 NetworkProtocolHarness 검사를 같은 변경으로 연결한다. 모르는 값의 silent fallback이나 기존 gameplay 권위 우회는 금지한다.

## G02. 저작 데이터와 게시 출력

WorldSequence와 Composition은 stable resource/placement/template/pattern/occurrence ID를 기준으로 세 버전을 비교한다. 오늘의 컷신·관문 Flow·카메라·재질·이펙트와 World import를 보존하고 PR의 PATTERN18/19 fire/hook emission, 칼날, Mario timing, Bingo hammer 변경을 추가한다. 배열 전체나 revision 번호가 큰 쪽을 무조건 채택하지 않는다.

`Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED`와 `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json`이 정본이다. mapplacements의 LFS pointer와 실제 binary를 구분한다. Client/Bin/DataFiles/Map, patternbindings, encounter 등 생성 출력은 통합된 정본과 해당 publisher로 다시 생성·Check한다. 신규 C++ 파일이 필요한 범위가 아니며 project/filter 등록은 실제 추가 데이터가 있을 때만 필요한 항목을 추가한다.

## G03. CY와 오늘 리소스 통합

실제 CY 입력은 `C:/Users/user/Downloads/CY_Resources`이며 Monster, KoukuSaton, LV_LUT_MIDNIGHTC_ED가 있다. 각각 기존 runtime의 Character/Monster, Character/KoukuSaton, Map/LV_LUT_MIDNIGHTC_ED 상대 경로와 비교한다. 사용자가 확정한 출력은 `C:/Users/user/Desktop/GB_Resources`다. 기존 `C:/Users/user/Desktop/GB/GB_Resources`와 CY 원본은 보존한다. 출력은 전체 Resources 복사본이 아니라 CY 실행 입력과 오늘 오후 2시 이후 로컬 변경분을 합친 팩이다.

선택 계획은 4,033개, 1,755,169,993 B다. CY 실행 입력 중 3,665개를 채택하고, 겹치는 맵 305개는 오늘 로컬 교정본을 우선하며, CY에 없는 오늘 로컬 63개를 추가한다. 비어 있던 출력에는 로컬과 CY의 bytes가 같아도 파일을 한 번 넣는다. 수정시각 후보 43개 외에 생성시각·기존 GB 비교로 확인한 신규 Gate2Intro DDS 20개도 포함한다. 복사 도구가 과거 수정시각을 보존한 파일을 누락하지 않으며, 시각만으로 최신판을 판단하지 않는다.

맵 305개는 로컬과 기존 GB의 SHA가 같고 WMSH/일부 material 교정을 포함하므로 CY 구형을 덮지 않는다. Character 4개는 로컬 WANM의 duration/tick rate/모든 key time을 기존 retimer로 30 Hz 정규화한 전체 bytes가 CY와 같은 것을 확인한 뒤 CY를 설치한다. quaternion과 geometry를 재선택하지 않는다. 신규 칼날 static WModel 1개와 맵 WModel/DDS/TGA 697개는 실행 자산이므로 현재 직접 참조가 없어도 보존·설치한다.

PNG/TGA는 기존 CMaterial이 소비하는 실행 텍스처이며 유지한다. glTF/PSA/PSK, backup, 추출 receipt 등 원본 40개는 팩에서 제외하되 CY에서 삭제하지 않는다. 기존 Character 4개를 교체할 때만 이전 bytes를 보관하고 source/destination SHA를 고정한 뒤 임시 파일에서 원자 교체한다. 새 출력과 신규 맵 설치는 최종 절대 경로의 루트 포함 관계를 확인하고 덮어쓰지 않는다. 같은 SHA는 재개 시 건너뛰고 알 수 없는 목적 파일이나 변경된 source는 보존·보고한다. binary Resources와 out의 조사 자료는 Git index에 넣지 않는다.

## G04. 검증과 동기화

충돌 marker, JSON/XML parse, 해당 WorldSequence/Composition publisher 검증과 output Check, 변경 wire/Server/World 계약의 기존 focused 검사를 실행한다. 실제 runtime Resources 참조의 존재 및 선택한 파일의 복사 후 hash를 확인한다. 필요한 Debug 제품 빌드로 Engine→Shared→Server→Client와 SDK/runtime 배포를 검증한다.

최신 사용자 추가 요청에 따라 빌드한 버그 수정본을 기존 PR #360에 게시한다. 사용자가 EXE를 직접 검증하고 문제가 없으면 직접 merge한다. 에이전트는 PR merge, main 동기화와 리소스 외부 공유를 보류한다. PR 게시 전에 원격 head를 다시 확인하고 force push 없이 통합 후속 commit을 올린다. 사용자 실행 중 process를 종료하거나 Client/UI를 실행·캡처하지 않는다. 자동 검증·PR 게시·로컬 리소스 복사와 사용자의 최종 시각 확인은 RESULT에 분리한다.
