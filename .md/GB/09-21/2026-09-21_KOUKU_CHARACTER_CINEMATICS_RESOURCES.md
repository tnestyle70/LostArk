# 쿠크 연출 Character 리소스 배포 목록

2026-09-21. 새 파일 경로를 추가한 작업이 아니라, 기존 Character WModel 3개에 연출 클립 11개를 추가한 작업이다. 텍스처·재질·메시·골격과 기존 클립은 보존했다.

## 배포 파일

아래 경로는 `Client/Bin/Resources` 기준이다. Resources는 Git 관리 대상이 아니므로 팀 리소스 배포에 아래 WModel 3개를 포함한다. 로컬 설치는 완료했으며 외부 저장소 업로드는 수행하지 않았다.

| 모델 | Resources 상대 경로 | 전체 클립 | 최종 크기 | 증가량 |
|---|---|---:|---:|---:|
| 쿠크 | `Character/KoukuSaton/MN_RPCZ_00/MN_RPCZ_00.wmodel` | 91 → 94 | 38.76 MiB | +11.42 MiB |
| 세이튼 | `Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel` | 249 → 256 | 245.97 MiB | +57.32 MiB |
| 큰 세이튼 | `Character/KoukuSaton/MN_RPCT_06/MN_RPCT_06.wmodel` | 34 → 35 | 21.17 MiB | +4.43 MiB |

## 추가 클립 전수 목록

길이는 WModel 자체의 샘플 길이다. 30 Hz 마지막 샘플 때문에 카드미로 11.95초 타임라인의 클립은 11.9667초 등으로 끝날 수 있다. 기존 패턴의 start/play/source trim과 transform은 변경하지 않았다.

| 모델 | 클립 이름 | 길이(초) |
|---|---|---:|
| 쿠크 | `gate2_intro_27s` | 27.0000 |
| 쿠크 | `kouku.gate2.clear.kouku` | 35.4000 |
| 쿠크 | `kouku.gate2.maze.kouku` | 11.9667 |
| 세이튼 | `gate2_intro_27s` | 27.0000 |
| 세이튼 | `kouku.gate1.full.saydonbook` | 41.5000 |
| 세이튼 | `kouku.gate1.full.saydonstage` | 41.5000 |
| 세이튼 | `kouku.gate1.full.saydonfinale` | 41.5000 |
| 세이튼 | `kouku.gate2.clear.saydonarrival` | 35.4000 |
| 세이튼 | `kouku.gate3.intro.saydonarrival` | 18.6667 |
| 세이튼 | `kouku.bingo.encore.saydon` | 23.3333 |
| 큰 세이튼 | `kouku.gate2.clear.largesaydon` | 35.4000 |

## 기존 구성 유지와 참조 변경

- 1관문 현재 P36과 Sequence P8은 이미 Character MN_RPCT_05의 원래 클립을 사용한다. 사용자가 편집한 타이밍·이벤트를 보존했다. 별도 gate1.full 파생 클립 3개도 Character로 합쳤지만 현재 사용하지 않는 occurrence를 다시 연결하지 않았다.
- 쇼타임은 `rpct00_evt2_rpct_showtime_01`, `rpct00_att_battle_28_05_start` 등 기존 Character 클립을 유지했다.
- WorldSequence object 11개는 stable ID를 유지하고 `modelAssetId`만 Character로 변경했다. 월드 문서 revision은 2143 → 2144다.
- G2 쿠크 BossCatalog의 `animationSetId`는 자신의 Character bodyModel로 복구했다.
- 예전 Map donor 11개는 오프라인 원본 비교/베이킹 입력으로 보존한다. 제품 BossCatalog/WorldSequence의 해당 보스 참조는 더 이상 이 파일을 사용하지 않는다. 책·무대 등 Map 소품은 이 보스 모델 이관 범위가 아니다.

## 파일 SHA-256

- `Character/KoukuSaton/MN_RPCZ_00/MN_RPCZ_00.wmodel`: `2a84934ee08086906856c46cc79f164facb282581085b06302271c46bd4baa57`
- `Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel`: `975c50aac1ea8fef4b4a9032642dce0ca7bf950541cb56f7e1420d1de070241d`
- `Character/KoukuSaton/MN_RPCT_06/MN_RPCT_06.wmodel`: `f86d3e4e14bdb9b268a66c47d9b31db155befc8f33036bef3e56290d132b9e66`

## 검증 근거

- 원본 Character의 기존 section 전체 byte 보존, 추가 WANM payload 원본 일치, 재실행 byte 동일성 검사 통과.
- 실제 CModel에서 11개 클립의 10,171개 시점, 23,587,616개 행렬 수치를 대조했으며 최대 오차는 0이다. 창 생성과 draw는 0회다.
- 후보와 로컬 설치본 SHA-256 일치. 실제 Client ActorCatalog는 수정 전 실패, 수정 후 정상 초기화를 확인했다.
- 세부 로그: `out/KoukuCharacterCinematicBake20260921/model_probe.log`, `out/KoukuCharacterCinematics20260921/installation.json`.
- GPU 화면·카메라·음향의 최종 감상 검증은 사용자 실행으로 확인한다. 수치 검증을 화면 확인으로 기재하지 않는다.
