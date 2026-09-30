# 쿠크 인형 화염·즉사 칼날·카드 미로 수정 결과

## G00. 반영 범위

사용자가 승인한 인형 화염 Collider 수정과 회귀에 즉사 칼날 속도 절반, 카드 미로 탈출
문양 랜덤 배치를 함께 반영했다. 기존 미커밋 변경을 보존했으며 자동 stage/commit하지 않았다.
Client/UI를 실행하지 않았다. LAN 자동 설정은2026-09-30 만료 뒤라 실행·우회하지 않았다.

## G01. 인형 화염과 판정의 부모 좌표계

원인은 생성 시점이나 역회전이 아니라 Effect root Y90도와 본 Collider 부모 좌표계의 차이다.
앞뒤 입 Collider20개는 현재 화염 track `effect.doll.flame`을 optional `worldEffectTrackId`로
명시한다. publisher와 Client pivot은 같은 `Bone × EffectTrackTRS × ObjectWorld`를 적용한다.
저장/로드·Product reader·editor 복사/비교/초기화·preview와 선택 UI를 함께 연결했다.
빈 값은 기존 경로를 유지하며 불명확하거나 지원하지 않는 연결은 실패 처리한다.

큰 인형 WORLD10개가 있는 P34/88/89/91/92/93만 연결했다. 불 이펙트·모델·원래 Collider
offset/크기·광기 증가량·반복 간격100ms는 유지했다. 공10개 게시 region은 byte 동일하다.

실제 설치 WModel의 앞뒤 입20개에 대해2/4/8/12/16초와 반복 이후19.314/36.628초를
검사했다. 독립 Client 행렬과 중심 오차0.015m 이내·전방 dot>.99999이며 불 방향5m는
접촉하고 종전 오방향5m는 접촉하지 않는다. 기존 구현을 모의해 연결 필드만 무시하면
Effect yaw90→37의 기대 변화-53도 대비 실제0도로 실패하므로 회귀 검출도 확인했다.
균일 scale2·offset[.25,.5,-.75]·Effect yaw37·Object yaw23/scale1.25 조합도 통과했다.

최종 Server bootstrap의 인형20개 WORLDKEY36776개를 Encounter와 전수 비교했다.
시간·visible·수명은 동일하고 수치 차이는 최대1.11e-16이다. P88 양쪽 입2/8/19.314초의
실제 Server key를 보간해 설치 모델과 비교한 중심 오차는 최대2.535mm,
전방 dot는 최소.9999999465이며 불 중심축5m가 모두 접촉한다. 최종 공10개도 이전 원본
projection과 동일하다. 증거는 `final-published-worldkey-receipt.json`이다.

## G02. 즉사 칼날 속도와 거리

사용 중인 world.38/39 template과 P95.world.1/P33.world.5 수명을11000→22000ms로
늘렸다. 위치 key 시각은2배, world.38 velocity는2→1m/s로 변경했다. world.39의 경로는
원래 위치값을 보존했다. Effect·즉사 Collider·지속 sound 끝 시각도22초이며 sound 시작,
발사1개·배치·STOP/LOOP·자전1440도/초는 유지한다. P95 duration도22초로 늘려 자르지 않는다.
P33의27.83초 안에는 새 칼날 끝이 들어간다. 미사용world.48과 일반 피해world.19는 동일하다.

변경 전 t와 변경 후2t를 실제 sample_object로 각82회 비교한 위치 오차는0m다.
world.38은2→1m/s, world.39는2.000001→1.00000055m/s이며 종점은 동일하다.
전체 원본 diff를 허용 필드와 비교해 승인 범위 밖 데이터 보존을 확인했다.

## G03. 카드 미로의 탈출 문양

일치 병정 처치는 완료 진행도만 확정한다. 다음 Server tick에서 최초 배치와 같은
Sample_Corridor를 사용해 탈출 위치를 고르고 좌표와 표시 flag를 함께 확정한다.
중앙5m·플레이어4m·생존 병정 및 활성 탈출 문양3m 이격, walkable/중앙 연결 경로를 검사한다.
탐색 실패는 완료 진행도를 유지하고 다음 tick에 재시도하며 확정 좌표는 재추첨하지 않는다.
Client의 기존 exit snapshot·표시·중앙 이동·최종 귀환 계약을 유지한다.

순차 player 루프가 같은 tick의 앞선 출구를 즉시 반영한다. 죽은 병정은 앞선 gameplay update에서
제거되고 퇴장자·사망자·문양 없는 망원경 담당은 출구 생성 대상이 아니다. 기존 최초 배치와
병정 재생성 호출은 exit 전용 추가 조건 없이 보존했다. 탐색은 기존 호출당256회 상한이며,
병적으로 연결되지 않은 navigation의 반복 탐색 비용은 별도 실측하지 않았다.

## G04. 저장·게시와 검증 증거

원본은 최신 디스크 bytes를 비교하고 백업 후 원자 교체했다. Composition revision2498,
WORLD revision2289이며 백업·필드별 비교·수치 검증 로그는
`out/KoukuDollCollider20261001/`에 있다. 새 C++ 파일·프로젝트 등록·protocol 변경은 없다.

검증 완료:

- 실제 모델 인형 회귀5개 PASS(75.188초), generic Effect TRS 추가 검사 PASS(1.849초).
- 저장 칼날 회귀3개 PASS(.809초), before/after 전체 허용 필드 비교와 sampler 증거 PASS.
- 기존 Python ResultTuning·WorldBoneCache와 저장 칼날 묶음18개 PASS(142.134초).
- Debug/Release Product 정상 증분 Build PASS. Client 오류0.
- KoukuSaydon owner publisher의 product·map·world·gameplay4개 domain PASS(402.748초).
  설치 WORLD는 저작본과 동일하며 Client Product의 연결20개, P95의 fixedTimeline22000ms를 확인했다.
- 게시 후 변경 tracked JSON/XML26개 parse PASS, 정상 저장소 설정의 요청 범위 git diff --check PASS.

native WORLD Effect fixture는 optional/named roundtrip·Apply/Save·preview copy·clear·invalid
anchor rollback을 통과했다. 추가 검사 초기의 revision 증가와 staged preview dirty 기대값은
기존 정상 계약에 맞춰 교정했으며 Production 코드는 추가 변경하지 않았다. 기존 전체
`--kouku-composition-editor-contract`는 그 뒤 Sequence seed가 현재5개인데3개를 기대하는
기존 항목에서 실패했다. scratch Composition/Sequence가 원본과 byte 동일함을 확인하여
새 fixture의 오염을 배제했다. 이 범위 밖 기대값이나 현재 저장 데이터를 임의 변경하지 않았다.

기존 native harness의 `--kouku-world-effect-frame-contract` 기능별 검사도 Debug Build 및
실행 exit0/PASS다. 기존2개 test cpp에만 진입을 추가했으며 새 프로젝트는 만들지 않았다.
Product reader/pivot의 native 실행은 이 fixture 대상이 아니므로 Product 컴파일과 실제
설치 모델의 독립 행렬·게시 데이터 검증 범위로 구분한다.

Debug/Release Server의 다음4개 entry를 각각 실행해 총8개 모두 exit0/PASS로 완료했다.
전체 실행 receipt는 `server-contract-results.json`, 각 로그는 `server-<configuration>-<case>.log`다.

| Server entry | Debug | Release |
|---|---|---|
| `--card-maze-contract-test` | PASS:1~4인 | PASS:제품 정책2~4인 |
| `--kouku-object-overlap-contract-test` | PASS | PASS |
| `--kouku-product-contract-test` | PASS | PASS |
| `--world-playback-contract-test` | PASS | PASS |

Debug product는 기존 draft/catalog·Mario 입장·완료·퇴장·Product 승인 시나리오를 포함하여
약9분 걸렸고 최종 failures0이다. 구현이나 게시 실패로 대기한 상태가 아니었다.
검증용 Server 프로세스는 모두 종료했다. 남은 미통과 항목은 위 기존 편집기 전체 검사의
Sequence5/3 기대값 불일치이며, 요청 기능의 단독 검사와 제품 Server 회귀는 통과했다.

### 2026-10-01 통합 중 발견한 Release 1인 카드 미로 진입 회귀

사용자가 Release Client/Server에서 혼자 시험한 상황과 실제 `CKoukuCardMazeRuntime::Plan`의
빌드 분기를 대조했다. 상자를 파괴한 뒤 Q 명령이 망원경 시작까지 도달해도 1인 HUNTER·망원경
겸직을 `_DEBUG`에서만 허용하여 Release는 다른 생존 플레이어가 없다는 이유로 거부했다.
그 결과 역할·카드 병정·카메라 flag가 모두 생성되지 않았다. 랜덤 출구 수정은 이미 시작한
미로의 일치 병정 처치 뒤에 실행되므로 이 진입 거부와 별개다. 기존 편집기 Sequence 기대값
3/현재 5 불일치 역시 이 Server 인원 판정과 연결되지 않는다.

정확히 인간 한 명인 방의 기존 겸직을 Debug/Release 공통으로 연결했다. 다인 방에서 다른
참가자가 죽어 혼자 남았을 때는 겸직으로 전환하지 않는다. Client Q typed command와
`CardMaze.flags & 1` 카메라 소비, snapshot 문양 소비는 기존 경로를 유지한다.

`--card-maze-contract-test`에 1/2/4인 실제 Q codec→handler→fixed tick 판정→상자 500+500
파괴→다음 Q→역할/병정 생성→world snapshot 카메라·문양→Q 관전 토글 검사를 추가했다.
기존 1~4인 병정 처치·랜덤 출구 실패 재시도·중앙 복귀 검사도 Release에서 1인을 포함한다.
VS18 Insiders의 정상 Release Server 증분 Build가 성공했고, 최신 EXE의
`--card-maze-contract-test`가 `card maze failures: 0`으로 종료했다. 1/2/4인 실제 Q와
world snapshot 검사를 포함한 결과이며 로그는 `out/IntegrationValidation/release-card-maze.log`다.
새 회귀의 첫 실행에서는 마지막 시계 교정 전 OBJ와 기본 class END인 fixture가 확인되어,
올바른 최신 source 및 실제 Lance Master class로 재컴파일한 뒤 통과했다. Production 판정이나
snapshot 검증을 완화하지 않았다. 같은 VS18 Insiders의 정상 Debug Shared/Server Build 뒤
`debug-card-maze.log`도 201개 검사, `card maze failures: 0`으로 종료했다.
Debug의 `--kouku-product-contract-test` 역시 `debug-kouku-product.log`에서 `failures : 0`으로
종료했다. 이 검사는 실제 화면·시점 전환을 사용자가 확인했다는 의미는 아니다.

## G05. 사용자 최종 확인 경계

수치·저장·빌드·headless 검증과 실제 화면 판정은 구분한다. 사용자는 갱신된 Client/Server로
인형 불 안/밖 광기, 느려진 칼날의 끝 위치, 카드 미로의 원거리 랜덤 문양→중앙→최종 귀환을
확인한다. GPU 입자별 외곽 폭을 Collider 전체와 일치시켰다는 의미는 아니다.
게시 파일 교체를 실행 중 프로세스 메모리나 미저장 authoring draft 갱신으로 설명하지 않는다.

## G06. 다른 세션 통합용 변경 지도

현재 작업 브랜치는 `codex/colosseum-material-restore-20261001`이며 다른 기능의 미커밋 변경이
많이 공존한다. 이 작업은 commit/stage하지 않았다. 파일 전체를 다른 checkout으로 덮어쓰지
않고 아래 의미 변경만 최신 코드에 병합한다. 검증은 완료됐고 검사 프로세스도 종료됐다.

| 범위 | 파일과 병합할 변경 |
|---|---|
| Client authoring | `Client/Public/KoukuSaydonCompositionDocument.h`, `Client/Private/KoukuSaydonCompositionDocument.cpp`: optional `strWorldEffectTrackId`·parse/save·exact WORLD/BODY/BONE 검증 |
| Client editor | `Client/Private/KoukuSaydonActionWorkbench.cpp`: 같은 필드의 placement copy/equality/reset·preview·Effect frame 선택 |
| Client consumer | `Client/Private/KoukuSaydonPresentationPlayer.cpp`, `Client/Public/Level_KakulSaydonArena.h`, `Client/Private/Level_KakulSaydonArena.cpp`: Product/preview가 ID를 pivot에 전달 |
| World pivot | `Client/Public/WorldSequencePlayer.h`, `Client/Private/WorldSequencePlayer_Objects.cpp`: named Effect root resolve 및 Bone×Effect×Object 합성. 같은 파일의 다른 변경과 구분 |
| Server 카드 미로 | `Server/Public/KoukuSaydonLogicRuntime.h`, `Server/Private/KoukuSaydonLogicRuntime.cpp`: kill 완료와 출구 좌표 commit 분리, Sample_Corridor의 optional exit 이격 |
| Server 카드 미로 tick | `Server/Private/GameRoom_KoukuMiniGames.cpp`: 목표 완료 후 추첨·실패 재시도·확정 위치 유지 |
| publisher | `Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py`, `world_object_collider.py`: optional frame 검증·본 부모·uniform scale bake |
| 저작 데이터 | `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json` revision2498와 `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json` revision2289. 인형20개 field와 두 칼날 시간/속도 및 P33/P95 창만 변경 |
| 생성물 | `Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json`, `Data/Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json`, 설치 map worldsequences, `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap` 및 그 Kouku presentation generation. 수작업 병합보다 최신 저작본+코드로 같은 publisher를 실행 |
| 회귀 | `Server/Private/ServerGameplayContractTests_CardMaze.cpp`, `Tools/KoukuSaydonPipeline/test_result_tuning_contract.py`, `test_prepare_mario_world_blades.py`, 기존 ValtanPatternAuditionServiceHarness의 `BossCompositionDocumentContractTests.cpp`·`ValtanPatternAuditionServiceHarness.cpp` |
| 문서 | 이 PLAN/RESULT 및 `.md/GB/gotchas.md`, `.md/GB/렌더링이펙트복원V2.md`, `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`의 WORLD Effect frame 추가 부분만 |

통합 뒤 필요한 명령은 `Invoke-BuildAndRegression.ps1 -Profile Product`의 Debug/Release,
`Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision <최신 revision>`이다.
revision2498을 현재보다 새 저장본에 강제로 맞추지 않는다. Rendering profile·FXAA·일반 칼날·
미사용 world.48은 이 작업 변경 대상이 아니다. 실행 EXE/DLL/PDB/EngineSDK와 `out/` 로그는
소스 커밋에 포함하지 않는다.


## G07. 통합 후 기존 8개 칼날 마이그레이션 테스트 입력 고정

`test_prepare_mario_world_blades.py`의 기존 두 검사가 현재 저작 파일을 과거 8-emission 변환기에
다시 넣어 P33.world.4 탐색에서 실패했다. 운영 변환기와 최신 단일 칼날·22초 데이터는 변경하지
않고, 도구 도입 commit `34ec277efc9f84585d66d6f049a664a512b91ee8`의 필요한 P33/P91
WORLD occurrence와 resource/instance/template를 37,489-byte fixture로 추렸다. 이미 생성된
WORLD 39와 P33.world.5 출력만 제외해 최초 변환 경로를 검사하며, 남긴 원본 행과 ordinal/revision은
그대로 보존했다. 출처·선택 범위는 fixture 내부에 기록했다.

`python -m unittest discover -s Tools/KoukuSaydonPipeline -p test_prepare_mario_world_blades.py -v`:
기존 변환 두 검사와 실제 최신 저장 데이터의 22초 칼날 세 검사, 총 5개 PASS(3.039초).
새 fixture JSON parse와 `git diff --check`도 통과했다. 제품 코드·운영 authoring 파일 수정은 없다.
