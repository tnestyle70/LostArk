# 쿠크 연출 Character 통합 결과

## 원인과 수정 범위

G2 쿠크의 BossCatalog animationSetId만 Map 카드미로 전용 모델을 가리켰고, 실제 Client ActorCatalog는 Character 경로만 허용했다. 전체 카탈로그 초기화가 실패해 Character Select 등 다른 Level 입장에도 영향을 줬다. 폰트 초기화 실패가 아니었다. Loading 생성 도중 등록한 배경이 남고 기존 Lobby 글자가 계속 그려져 화면이 섞였다.

별도 Map 보스 연출 11개를 기존 Character 본체 3개에 WANM section으로 병합했다. 기존 모든 section bytes를 보존했고 donor와 skeleton/rest basis 일치 및 같은 clip payload를 확인했다. 이름 충돌은 거부하고 같은 입력 재실행은 무변경이다.

| 본체 | 기존 클립 | 추가 | 최종 |
|---|---:|---:|---:|
| MN_RPCZ_00 쿠크 | 91 | 3 | 94 |
| MN_RPCT_05 세이튼 | 249 | 7 | 256 |
| MN_RPCT_06 큰 세이튼 | 34 | 1 | 35 |

전수 목록과 파일 크기·SHA는 [리소스 목록](2026-09-21_KOUKU_CHARACTER_CINEMATICS_RESOURCES.md)에 둔다.

## 저작·재생 참조

WorldSequence object 11개의 modelAssetId만 Character로 변경하고 revision2143→2144를 게시했다. stable ID, occurrence, 시간, visibility, modelPreScale, 재질 source, camera/effect/audio/transform을 보존했다. G2 쿠크 animationSetId는 bodyModel과 같은 Character ID로 복구했다. P77 기존 보스 재사용·bossMotion·연출 clip 이름은 유지했다.

현재 1관문 P36/Sequence P8과 쇼타임은 이미 Character 원래 클립을 사용하므로 그대로 두었다. 별도 gate1.full3종도 Character에 합쳤지만 현재 사용하지 않는 occurrence를 다시 연결하지 않았다. Map donor는 오프라인 원본 비교 입력으로 보존하며 소품을 보스 모델과 함께 이관하지 않았다.

source sequence/2관문 intro/effect attachment/encore 생성 도구는 Character에 animation만 합치고 canonical ID를 반환한다. 본 부착 샘플은 animation index0 대신 명시 clip name을 사용한다. 게시기도 body/weapon/animationSet에 Character 경로와 정확한 .wmodel 확장자를 요구해 Client와 동일한 경계를 검사한다.

## UI 실패 정리와 실행 중 발견된 assertion

Loading은 자기 Chrome와 Recovery sprite만 hide/remove한다. 등록된 다른 owner나 공유 texture prototype을 지우지 않는다. Loader 초기화는 실제 실패 단계와 status를 최초 recovery로 남긴다. 종료 순서와 renderer queued shared_ptr 수명도 검토했다.

앞선 UI 오진 때 추가됐던 MainApp의 LOADING 강제 CancelFrame/return 블록을 제거해 정상 ImGui 제출 경로로 복귀했다. 사용자가 Debug 실행 중 보고한 imgui.cpp11572 assertion은 CancelFrame이 EndFrame만 호출하고 UpdatePlatformWindows를 생략한 것이 직접 원인이다. 공통 CancelFrame에 ViewportsEnable 조건 플랫폼 프레임 마무리를 추가했다. vendor assertion 제거와 viewport 기능 비활성화로 우회하지 않았다.

## 확인한 검증

- 실제 Client ActorCatalog reader: 수정 전 `Actor catalog contract mismatch.` / 수정 후 `Actor catalogs ready.`.
- 실제 CModel: 11개 clip 전프레임 10,171시점 / 23,587,616개 행렬 수치 / 최대 오차0. 창0, draw0.
- 기존 Character mesh/material/skeleton/animation section 보존, 추가 key4,422,678개의 finite/timing/quaternion 검사, 재실행 byte 동일성 통과.
- 실제 CWorldSequenceDocument Load/Validate: authoring와 published 모두 revision2144 PASS. 맵3369/deploy7의 실제 catalog와 WModel clip context 사용, object461/template277/instance334, 변경11개 경로 일치.
- 통합 설치는 후보·현재 파일 hash 확인, 원본 백업, 파일별 원자 교체와 자기 변경 rollback으로 처리했다. 설치본3개와 배포본3개 SHA 일치.
- Character bake 검사4개 및 publisher Map donor 거부·잘못된 경로 포함 검사9개 통과.

빌드와 게시의 최종 실행 근거는 아래 로그에 기록한다. Client GPU 화면·음향의 최종 확인은 사용자 실행으로 남기며 native reader/pose 검사와 구분한다.

## 배포와 근거 위치

`C:/Users/user/Desktop/GBResources/Character/KoukuSaton/`에 갱신한 WModel3개를 넣었다. 동일 폴더 루트의 README, 리소스 상세 Markdown, JSON manifest로 설치 경로·클립·크기·SHA를 확인할 수 있다. 이 PC의 Client/Bin/Resources에는 이미 설치했다. 다른 PC는 코드·게시 DataFiles와 함께 이 Character 폴더를 Client/Bin/Resources에 병합한 뒤 빌드한다. Resources 자체는 Git에 추가하지 않았다.

- `out/KoukuCharacterCinematics20260921/installation.json`: 실제 설치6파일 before/after hash
- `out/KoukuCharacterCinematicBake20260921/candidates/receipt.json`: 모델3개와 clip11개 원본·후보
- `out/KoukuCharacterCinematicBake20260921/model_probe.log`: 실제 엔진 pose
- `out/KoukuCharacterCinematics20260921/catalog-before.log`, `catalog-after.log`: 실제 Client catalog
- `out/KoukuCharacterCinematics20260921/world-reader/receipt.json`: authoring/published 실제 reader
- `out/KoukuCharacterCinematics20260921/publish.log`, `publish-final.log`: 명시 domain 게시
- `out/KoukuCharacterCinematics20260921/product-debug.log`: 첫 Debug Product compile/deploy
- [Loading cleanup 결과](2026-09-21_LOADING_INITIALIZATION_ROLLBACK_RESULT.md)
- [Character bake 결과](2026-09-21_KOUKU_CHARACTER_CINEMATIC_BAKE_RESULT.md)

## 최종 사용자 빌드 확인과 추가 회귀

사용자가 직접 Debug 빌드를 완료했다. ImGuiLayer.cpp/MainApp.cpp 수정 시각09:27:55 이후 ImGuiLayer.obj09:28:56, Engine.dll09:28:56, MainApp.obj09:29:07, Client.exe09:29:10을 확인했다. Engine/Bin/Debug와 Client/Bin/Debug의 Engine.dll SHA-256은 `5408299fcce6c240b84454098b42e2479203c72066ece94565fa4f222dba1e02`로 같다. 사용자의 Client/Server는09:29:11에 시작했다. 에이전트의 중복 빌드는 실행 중 출력 보호로 차단됐으며 사용자 프로세스를 종료하지 않았다.

바로 이 최종 Engine.dll을 별도 headless probe로 검사했다. 수정 전 DLL의 CancelFrame은 FrameCount1 / FrameCountPlatformEnded-1로 assertion 조건을 만들었다. 수정 DLL은 viewport on/off 각각12프레임, 중복 Cancel과 미초기화 호출 포함86개 검사를 통과했다. 창·draw·platform create callback은0회다. 근거는 `out/KoukuCharacterCinematicBake20260921/imgui-cancel-probe/receipt.json`이다.

source generator 실물 검사15개도 통과했다. 9개 source clip의 append·재실행 보존, 2관문 배우2종의 실제811프레임 재베이크 byte 동일성, Encore 독립 후보의 전체 clip 보존, 본부착 이펙트 전체 WORLD 궤적 동일성과 live 리소스 무변경을 확인했다. 근거는 `out/CinematicCharacterAudit20260921/generator-check/result.json`이다. 전체 `git diff --check`도 통과했다.

최종 KoukuSaydon owner 게시도 exit0으로 완료했다. sourceRevision2027을 확인했고 product PASS, map/world 재사용 검증 PASS, gameplay.balance PASS로 전체317090ms다. source publisher 경로 계약 수정까지 반영된 실행이며 코드 빌드와 리소스 설치/배포 및 사용자 화면 판정은 별도로 구분한다.
