# 2026-09-30 origin/main 병합 — PR #487 충돌 해결 RESULT

브랜치 `codex/maharaka-sea-island-waterpang-0930` (병합 전 HEAD `73400f59`, 백업 브랜치 `backup/pre-merge-0930`)에
`origin/main` (`50dcd986`, base `23008e0e`보다 32커밋 앞)을 `git merge`로 합쳤다. 원칙은 main의 작업과 이 브랜치의 작업이
둘 다 남는 것이다. 파일 전체를 한쪽으로 채택한 곳은 없고, 생성물만 병합된 소스로 다시 만들었다.

빌드와 Client/Server 실행은 하지 않았다. 구문 검사(`cl /Zs`)와 데이터 검증만 실행했다.

## 1. 충돌 파일 11개와 해결 방식

| 파일 | 충돌 원인 | 해결 |
|---|---|---|
| `Shared/Public/Network/PacketType.h` | 프로토콜 번호 main 124 대 이 브랜치 122 | 두 쪽 주석을 모두 남기고 `NETWORK_PROTOCOL_VERSION = 125` |
| `Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp` | 번호 고정 문구 6곳 (양쪽 모두 stale) | 충돌 6곳을 정리하고 번호 고정 23곳을 125로 통일. 양쪽 테스트는 모두 유지 |
| `Client/Private/Effect_ArtistMaterial_Tables.inl` | 양쪽이 서로 다른 프로그램 표를 같은 자리에 추가 (11구간) | 블록 단위 병합 (아래 3절) |
| `Server/Bin/DataFiles/World/BERN.worldbootstrap`, `MAHARAKA.worldbootstrap` | main이 형식을 v11에서 v12로 변경, 이 브랜치는 행 추가 | 손으로 합치지 않고 병합된 `Publish-WorldGameplay.ps1`로 재생성 |
| `Data/Effects/EffectCatalog.json` | 양쪽이 끝에 항목 추가 | 합집합 (1,493개, 중복 0) |
| `Tools/WorldPipeline/Publish-WorldGameplay.ps1` | `interactAction` 허용 목록 | 이 브랜치의 `dock:<이름>` 허용과 main의 `move`, `lever` 추가를 함께 유지 |
| `Client/Public/InteractKeyPromptView.h`, `Client/Private/InteractKeyPromptView.cpp` | `ACTION` enum과 switch case 번호 | main 번호 유지 (`MOVE=4`, `LEVER=5`), 이 브랜치의 `DOCK`은 6번. 문구, 아이콘, 파싱 세 곳에 모두 반영 |
| `Client/Private/Level_Loading.cpp` | 로딩 분기 else-if 체인 | 마하라카와 바다 분기 뒤에 main의 로비 분기를 이어 붙임 (중괄호 균형 확인) |
| `.md/GB/gotchas.md` | 양쪽이 끝에 항목 추가 | 두 쪽 모두 유지 |

추가로 `CLAUDE.md`의 낡은 문장 "현재 Shared protocol v119"를 v125로 고쳤다.

## 2. 프로토콜 125와 wire 병합

- main의 wire 변경: 124 (저장 캐릭터 복원, 레이드 EXIT 투표, 지점 지정 전투 아이템), 새 패킷 종류 3개.
- 이 브랜치의 wire 변경: 122 (`PLAYER_SNAPSHOT`에 `iWaterGunSkillId`, `iWaterGunCastTick` 추가).
- 두 변경은 서로 다른 위치라 자동 병합됐다. 쓰기와 읽기가 `isWaterpangArmed` 다음, 부착 필드 앞에서 대칭인 것을 확인했다.
  새 패킷 종류는 이 브랜치가 추가하지 않았으므로 번호 재배정은 없다.
- 새 번호는 main 최댓값 다음인 125다. Server와 Client를 함께 빌드하고 재시작해야 한다.

## 3. native program 번호 (충돌 없음)

- 이 브랜치: 4961~4979 (19개, 4928 버킷). main: 5312~5362 (51개, 새 5312 버킷 파일과 `Effect_ShaderFamily.h` 등록).
- 번호와 블록 이름이 겹치지 않아 재할당은 필요 없었다. 4928 버킷 파일 3개는 이 브랜치만, 5312 버킷 파일은 main만 바꿨다.
- `Effect_ArtistMaterial_Tables.inl` 병합 방식: 파일을 최상위 선언 블록 단위로 나눠, main 파일에 이 브랜치의 블록 57개를
  같은 앵커(`ARTIST_SWITCHES_3655` 뒤, main 블록 앞)에 넣고, `ARTIST_PROGRAM_STORAGE` 항목 19개를 항목 3655 뒤에 넣었다.
  선언 개수는 2635 (기존 2565 + 19 + 51). 검증: 블록 7,906개가 합집합과 일치, 모든 블록 텍스트가 원래 쪽과 동일,
  항목 2,635개 중복 0. BOM과 CRLF 유지.

## 4. 다시 만든 산출물

- `Publish-WorldGameplay.ps1 -Mode Validate/Publish -WorldId BERN` (배치 69), `MAHARAKA` (배치 42) 성공.
  `BERN.worldbootstrap`, `MAHARAKA.worldbootstrap`은 v12 형식이고, main 대비 차이는 이 브랜치의 행뿐이다
  (BERN은 `island.dock.to.maharaka`, `island.return.sea.landing`, MAHARAKA는 `island.exit.to.bern`).
- Bern/Maharaka 뷰어 월드 문서와 NPC presentation 문서도 함께 재생성됐다.

## 5. 검증 (실행한 것만)

- 충돌 마커 0, unmerged 경로 0.
- `cl /Zs` 구문 검사 75개 파일 통과 (Client 47, Server 24, Shared 1, Engine 2, NetworkProtocolHarness 1). 병합에서 양쪽이 바꾼
  모든 `.cpp`가 대상이다. Engine 헤더는 병합된 `GameInstance.h`, `Sound/Sound_Manager.h`를 덮어쓴 SDK 복사본을 썼다.
- 양쪽이 바꾼 파일 31개에서 각 쪽이 추가한 줄이 병합 결과에 남았는지 비교했다. 사라진 줄은 모두 의도한 합침이다
  (저장 배열 개수 선언, `ACTION` enum, case 번호, 프로토콜 번호 고정 문구, 게시 스크립트 조건문). 그 외 0줄.
- 양쪽 핵심 심볼 표 (모두 존재): 물총 스킬 표, 아레나 창 판정, 건너가기 무적, 물벼락 반지름, 복귀 착지 표식,
  배 HUD 슬롯, 바다 미니맵, 바다·마하라카 로딩 분기, jump 마커, 섬 모델 `ISL_00072`, 닻 마커·배 물살 이펙트,
  배 기억 처리, WaterpangEntry 확장, native program 4979와 main의 5362, 5312 셰이더 버킷 등록, 폭탄·발탄 이펙트,
  `ACTION::LEVER`, 로비 로딩 분기, worldbootstrap v12. main의 새 패킷 종류 3개도 모두 있다.
- JSON 65개 (양쪽이 건드린 파일) 파싱 실패 0.
- 병합 결과 대 `origin/main`은 142개 파일 (이 브랜치의 작업만), 대 병합 전 HEAD는 224개 파일 (main의 작업만).
  범위 밖 파일은 없다.

## 6. 남은 위험

- 빌드를 하지 않았다. `/Zs`는 구문과 타입 검사이고 링크, 셰이더 CSO 컴파일, 실행은 확인하지 못했다.
  양쪽이 바꾸지 않았지만 바뀐 헤더를 쓰는 다른 `.cpp`는 검사 대상이 아니다.
- 하네스는 컴파일만 확인했고 실행하지 않았다 (번호 고정은 125로 맞췄다).
- 새 셰이더 버킷(5312)과 4928 버킷이 함께 있는 CSO 변형 그룹 경계는 빌드 없이 확인하지 못했다.
- 재생성한 worldbootstrap을 실제 Server가 읽는지는 실행 전까지 미확인이다 (구문 검사와 게시 검증은 통과).
- 프로토콜이 125로 바뀌었으므로 예전 빌드의 peer와는 접속되지 않는다.

## 7. 다음 순서

VS를 닫고 Debug/x64로 빌드한 뒤 Server와 Client를 함께 재시작한다. 이후 마하라카 입항, 워터팡 물총 스킬, 바다 미니맵,
배 HUD, 로딩 화면을 화면에서 확인한다.
