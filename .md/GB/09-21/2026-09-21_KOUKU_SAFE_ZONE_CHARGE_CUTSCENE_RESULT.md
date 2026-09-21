# 쿠크 안전존·휠윈드·컷씬 모델 결과

## G00. 적용 상태와 사용자 기준

사용자 후속 답변에 따라 안전존 내부에서는 즉사·공포·최대 체력 50% 피해를 모두 막는다.
밖에서 시선 판정에 실패하면 기존 같은 Fail 슬롯의 공포와 50% 피해를 함께 적용한다.
Publish는 사용자가 직접 수행한다는 후속 지시를 받았으며 에이전트는 live Publish를 실행하지 않았다.

현재 상태는 **소스 구현 및 격리 검증 완료, 승인된 데이터·모델 설치 완료, 미게시·제품 빌드 미완료**다.
사용자가 Composition을 계속 편집하므로 현재 디스크의 무관 변경을 보존한다.
후보 준비 뒤 사용자가 전부 저장 완료를 확인해 실제 최신 source1985→1986으로 병합했다.
실제 반영 때 최신 파일에 stable ID/필드 기준 merge, 동일 필드 충돌 거부, hash 재확인,
백업과 원자 교체를 사용한다. 오래된 candidate 문서 전체를 덮어쓰지 않는다.

## G01. 안전존과 상태 문구 소스

Composition·Workbench·projector·Gameplay publisher·Server catalog에 `INVULNERABILITY_ZONE`을 연결했다.
고정 MAP Collider 1~64개를 같은 시간 창에 연결하며 Result 슬롯은 없다.
Server LogicRuntime은 각 Pattern 실행의 현재 보호 대상을 결과 적용 전에 모아
그 실행의 `INSTANT_DEATH`, `MAX_HP_PERCENT_DAMAGE`, `FEAR`만 차단한다.
다른 보스·다른 Pattern·외부 페널티에 보호가 누수하지 않으며 이미 걸린 공포를 정화하지 않는다.
기존 contact/judgement와 같이 종료 tick까지 보호하고 다음 tick부터 해제한다.

Server는 입장 즉시 및 60tick(2초)마다 pulse를 갱신한다.
Shared protocol99 `SNAPSHOT_PLAYER.iInvulnerabilityZonePulseTick`은 현재 tick의 활성 접촉일 때만
복제하고 퇴장·종료·사망 등 비활성에서는 0이다. Client Level은 서버 pulse에 맞춰
기존 `CStatusEffectTextView`로 파란 `무적`(PROJECT_TUNED RGB3399FF)을 그린다.
중복 제거 키를 owner+word로 바꿔 공포와 보호 문구가 서로 재생을 반복시키지 않는다.
기존 YG760 SpriteFont에서 무적·공포 글리프가 모두 존재하는 것도 확인했다.

## G02. 준비한 데이터

- P11 Logic101에 새 kind를 넣고 파랑/빨강 원 4개 각각의 표시 시각에 독립된 안전존을 연결한다.
  데칼 leaf의 S*R*T·그룹 child·MAP occurrence를 대조했으며 현재 local Z+2.5m와 4×4m 크기를
  반영한 반경2m 원이다. 판정은 기존 XZ 플레이어 중심 포함 계약이며 body radius 가산이나 Y 검사는 없다.
- P11 광역 타격 3행의 Success를 기존 즉사 Result3에 연결한다. 공용 Result98(10% 피해)은 변경하지 않는다.
  gaze Result6(50% 피해)+Result37(공포)의 순서와 연결도 보존한다.
- P24 Logic38의 기존 player-target charge를 10m, yaw−90도로 복구한다.
  설치 MN_RPCZ_00 본 방향과 24방향을 대조한 최소 내적은 .9993645다.
  기존 네비게이션/충돌 clamp, 수평 root 배율0, P21/P23 및 occurrence 시간은 보존한다.
- 기존 거미 공포 Result35에 SOUND `sound.kouku.018b3ad1ae968ba781cc`를 연결하는 후보도 통합했다.
  상세 source 보이스 근거와 원래 Play 상태 전달 변경은 같은 날짜의
  KOUKU_SPIDER_PLAY_AND_FEAR_SOUND_RESULT를 따른다.

P11은 최신 사용자 게시본1971에서 빠져 있었다. before publication은 Logic7의 judgementKind
누락으로 거부됐고, 후보는 정규 validate_document·validate_publishable·projected_outputs를 통과했다.
단순 전체 Publish 성공과 특정 Pattern의 게시 허용을 구분한다.

## G03. 컷씬 모델 후보

worldsequences의 별도 파생 WModel은 복구된 Character 모델을 material source로 사용하면서도
기존 WINT1.0/stride76의 face normal을 유지하고 있었다.
요청 구간 5개는 아래와 같으며 총191,456 triangles다.

| 구간 | Resources 상대 모델 |
|---|---|
| 2관문 입장 Saydon | Map/KakulSaydon/Gate2Intro/Saydon/Saydon.wmodel |
| 2관문 입장 Kouku | Map/KakulSaydon/Gate2Intro/Kouku/Kouku.wmodel |
| 2관문 미로 Kouku | Map/KakulSaydon/SourceSequences/kouku.gate2.maze/Kouku/Kouku.wmodel |
| 3관문 입장 Saydon | Map/KakulSaydon/SourceSequences/kouku.gate3.intro/SaydonArrival/SaydonArrival.wmodel |
| 빙고 앵콜 Saydon | Map/KakulSaydon/SourceSequences/kouku.bingo.encore/Saydon/Saydon.wmodel |

기존 converter로 원본 glTF indexed corners에 대응해 smooth normal/tangent/handedness만 복구했다.
위치·UV·skin·index와 재질·골격·전용 animation section bytes는 보존한다. Character WModel 전체를
복사하지 않는다. 추가로 같은 결함을 확인한 6개 모델은 후보만 생성했으며 요청 설치 목록에는 제외했다.
11개 전체 457,486 triangles/1,361,821 vertices의 basis 오차는 최대2.98e−8이며 새 정점 추가는 없다.
설치 전/후 SHA와 경로는 `out/KoukuSequenceBasis20260921/candidate-install.receipt.json`에 있다.

## G04. 실행한 검증

- Client Level/StatusEffectTextView/Workbench: 실제 프로젝트 옵션 Debug·Release 6TU 컴파일 PASS.
- 실제 Composition codec: fresh5TU, 56 checks/0 failures. 새 kind roundtrip·Save/Reload2회 및
  잘못된 outcome/geometry 등의 Save 실패에서 기존 파일·LastGood·generation 보존 PASS.
- Python 안전존 regression3개, 기존 charge/수평 root focused2개 PASS.
- 최신 ABI Server47TU+Shared8TU 격리 컴파일/링크 PASS. 실제 LogicRuntime+GameRoom snapshot
  52 checks/0 failures, 영구 안전존 회귀12개 포함. 이전 게시1957 P11에 현재50% 결과와 새 zone을
  연결한 검사와 현재 후보의 정규 projection 검증을 구분한다.
- NetworkProtocolHarness `--mario-controls-only` 219 checks/0 failures,
  `--move-prediction-only` 101 checks/0 failures. Protocol84에 머물렀던 기대값을99로 갱신했고,
  payload size 산식에는 새 zone pulse와 기존 HonorTitle/retained entity presentation 필드를 명시했다.
- source1984 P11을 실제 Gameplay publisher의 원문 helper/Logic 구간으로 평가해 Logic10·Outcome9·Region7의
  26행을 생성했다. 다른 독립 추출 결과와 행 문자열까지 일치한다. 이 행을 이전1957 bootstrap의 P11에만
  넣은 격리 fixture에서 실제 catalog parser·Brain validation·runtime52 checks/0 failures를 확인했다.
  이 fixture는 전체 최신 게시본이 아니라 현재 P11행 검증용이다. full publisher를 실행한 것으로 기록하지 않는다.
- 같은 fixture에서 현재 BigSaydon spawn과 후보4개 원의 실제 geometry/timing으로 2·3차 gaze와
  즉사3회를 추가 확인한 최종 결과는73 checks/0 failures다. 보호 대상은 HP100·FEAR없음·pulse발행을 유지했다.
- 파생 WModel11개의 Release native `--skinned-basis-candidate` decoder PASS.
- 변경 JSON3개 parse, project/filter XML6개 parse, C++21개 UTF-8 BOM 정책과 기존 CRLF 유지 확인.
- 독립 검토에서 종료 tick 보호 누락1건을 수정하고 추가 판정 결함 없음 확인.
- `git diff --check` PASS. Client/UI 자율 실행·화면 캡처·음성 청취는 하지 않았다.

정식 Debug Product는 컴파일 전에 **출력 점유 preflight FAIL**이다.
Client PID13240과 Server PID14988이 실행 중이어서 새 실행 파일을 링크하지 않았다.
증거: `out/BuildPipeline/runs/20260920T202341229Z-debug-product.json`.
격리 컴파일 성공을 정식 제품 빌드 또는 실행 중 프로그램의 업데이트로 기록하지 않는다.

## G05. 남은 반영과 사용자 확인

사용자의 저장 완료 답변을 현재 저장본 반영 승인으로 받아 추가 확인 없이 설치했다. 후보 상세는
`out/KoukuSafeZoneRepair20260921/REVIEW.md`, 원본 변경 후보는 `combined-field-patch.json`이다.
live Composition은20개 필드/추가 항목 패치를 최신1985에 병합해1986으로 원자 교체했다.
요청 컷씬 모델5개도 before/candidate SHA 재확인, 백업, 원자 교체 후 최종 SHA를 검증했다.
`out/KoukuSafeZoneRepair20260921/applied.receipt.json` 및
`out/KoukuSequenceBasis20260921/installed.receipt.json`이 설치 증거다. Publish는 하지 않았다.
Protocol99 Client와 Server의 제품 빌드·재시작이 필요하다. 실행 파일 점유 해소는 사용자가 수행한다.
사용자가 Publish한 후 P11/휠윈드/거미 Play와 컷씬 모델의 실제 화면·소리는 사용자가 최종 확인한다.

현재 저작 시간상 첫 gaze의 판정은2206ms, 안전 원 생성은2480~2510ms다. 따라서 첫 시선 판정은
보호 영역이 생기기 전이며, 이후2·3차 시선은 보호된다. 사용자 occurrence 시각을 임의 이동하지 않았다.

같은 저장 완료 답변에서 중앙 이동 좌표와 레이저·바주카·대형 세이튼의 밀려나기 튜닝이 추가됐다.
이 후속 범위는 아래에 별도로 검증·반영 결과를 기록하며, 위1986 설치 완료와 혼동하지 않는다.

## G06. 후속 중앙 이동·밀치기 반영

사용자의 같은 저장 완료 승인으로 최신 Composition1986→1987을 추가 병합했다.
`out/KoukuKnockback20260921/applied.receipt.json`에 before/after SHA와 원본 백업 경로를 기록했다.
최종 문서는 검증한 후보와 JSON 의미가 완전히 일치한다. Publish는 실행하지 않았다.

- 중앙 이동 Logic103은 `BOSS_TELEPORT_GROUNDED`, 위치(4.66,10.56,322.94)다.
  기존 P23.logic12/P25.logic14의 연결을 그대로 소비한다. 실제 nav 높이는10.5599994659다.
- 휠윈드 Result39의2m/242ms를 기준으로 레이저 P21·바주카 P85의10개 접촉은6m/242ms다.
  중앙 발사 Result104는 yaw+90도, 사선 P21.logic4 Result106은+135.9000015도,
  P21.logic6 Result107은+45도로 실제 collider 축과 일치한다.
- 대형 세이튼 P13/P17/P27/P86/P87의12개 접촉은 Result105의16m/242ms,
  `forcePush=true`와 `AWAY_FROM_BOSS`를 사용한다. 다른 공유 Result39/42/98과 팡파레는 보존했다.
- 위22개 접촉만 `pushCanLeaveArena=true`다. 밀림 도중 실제 외곽을 넘으면 기존 FALLING→DEAD를
  소비한다. 6m/16m가 경계에 도달하지 않거나 지형·충돌에 막히면 사망을 강제하지 않는다.
  단순 nav 높이 변화는 낙사 근거로 쓰지 않는다. 현재 중앙의 높이 경계는 기존 clamp를 유지한다.
- 강제 밀림은 FEAR·다운·기상 grace·기존 밀림을 교체하며 사망·낙하·잡힘·부착·맵 이동은 제외한다.
  같은 Pattern 안전존이 차단한 피해 Result는 강제 밀림도 실행하지 않는다.

Client codec/Workbench, Python projector, Gameplay publisher, Server catalog/LogicRuntime/hit reaction과
GameRoom 소비자를 함께 연결했다. 새 optional field의 기본값은 false/false/0이어서 기존 일반 밀림을 보존한다.
외곽 판정은 explicit void 또는 같은 navigation grid의 진행 방향에 다시 원본 바닥이 없는 비보행 지점이다.
Runtime blocker에는 원본 바닥이 있으므로 그대로 막힌다. 외곽 후보라도 body collision 뒤 최종 위치를 다시
검사한다. 안쪽 slide는 exact walkable·line of sight·traversal을 통과한 때만 commit한다.
외곽까지 이어지는 authoring paint 차단과 원래 바닥 부재는 기존 runtime grid가 원인을 구분하지 않는다.
현재 Gate2 주변에는 paint 차단·runtime blocker가 없음을 조사했으며 새 외곽 paint 변경 때 재검토한다.

실행한 후속 검증:

- Client Workbench Debug/Release 실제 옵션2TU, C++ codec fresh5TU 컴파일·링크 PASS.
  codec48 checks/0 failures: Save/Reload3회, invalid Save8건과 invalid Reload3건의 디스크/LastGood 보존.
- Python typed push focused2 tests PASS. 완성 후보의 full document 검증과22개 접촉 projection PASS.
  stable merge의 재실행·무관 저장 보존·동일 필드 충돌 거부를 검사했다.
- 실제 Gameplay publisher의 원문 helper/Logic 구간을 out에서 평가했다. closure11개 Pattern의
  push22행(11컬럼12개,12컬럼10개)과 각도45/90/135.9000015를 검증했다. 전체 live publisher 실행은 아니다.
- 현재 ABI의 Server47TU+Shared8TU 및 기존 계약 test TU2개를 격리 컴파일·링크했다.
  실제 생성22행의 Server catalog 읽기, force 상태 교체/제외, 거리, 안전존, 실제 navigation 외곽 낙사와
  정적 충돌/body slide 검증의 최종 결과는43 checks/0 failures다.
  근거: `out/Knockback20260921/final.log`, `out/KoukuKnockback20260921/publisher-rows/evaluation.receipt.json`.
- 독립 리뷰에서 높이 차이의 낙사 오인과 collision 후 최종 바닥 재검증 누락을 수정하고 재검증했다.
- 최종 변경 C++28개의 기존 UTF-8 BOM 유무/CRLF 유지와 설치 문서의 검증 후보 일치를 확인했다.
  `out/KoukuKnockback20260921/final-structure-validation.json`에 기록했다.

실행 중 Client/Server의 제품 EXE 링크·재시작은 아직 하지 않았다. 실행 파일 점유는 마지막 제품 빌드를
막는 조건이며 데이터 병합이나 격리 검증을 막지 않는다. 사용자는 새 Client/Server 빌드·재시작 후 최신
저장본을 열어 직접 Publish하고 실제 Play·밀림·낙사·사운드·모델 화면을 확인한다. 이전 Client 메모리
문서를 새 디스크 파일 위에 다시 저장하거나 자동 Reload하는 동작은 에이전트가 수행하지 않았다.
