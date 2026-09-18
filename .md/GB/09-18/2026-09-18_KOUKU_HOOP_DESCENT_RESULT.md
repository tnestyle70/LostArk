# 2026-09-18 쿠크 훌라후프 상단 시작 후 하강 착지 RESULT

## 요청

`쿠크_훌라후프`(KAKULSAYDON_G1_PATTERN_84)는 `쿠크_훌라후프_상단시작` TRIGGER로 위에서 시작하지만
Server play에서 내려오지 않는다. 애니메이션의 원본 하강을 따라 아래로 내려오게 한다.

## 원인

- logic.77은 `ALBION_AIRBORNE` JUMP, `airborneDurationMs 0`이다. `CKoukuSaydonBrain::Sample_AlbionAirborneHeight`는
  JUMP 동안 높이를 고정값으로 반환하므로 Stage native root motion의 하강을 덮어쓴다.
- 원본 하강을 소비하는 기존 계약은 SLAM phase다. 09-17 G10에서 JUMP 0 ms와 SLAM(logic.2)을 같은 clock에
  두었으나, 사용자가 P84를 action A(`rpcz00_att_battle_12_*`) 9 Stage로 재저작하면서 SLAM box가 빠졌다.
- JUMP 높이 11.191078 m는 action B `13_start` 하강량이다. 현재 `12_start`의 실제 하강량은 10.890836 m다.

C++ 변경은 없다. 기존 Server·Workbench 계약을 그대로 사용하는 데이터 수정이다.

## 변경

`Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json` (revision 1380 → 1381, 바이트 splice·CAS 설치)

| 항목 | 값 |
|---|---|
| logic.77 `쿠크_훌라후프_상단시작` airborneHeightM | 11.191078264019293 → 10.890836241085884 |
| 새 logic.83 `쿠크_훌라후프_하강착지` | TRIGGER / ALBION_AIRBORNE / SLAM |
| P84 logic.3 | logic.83, startMs 0, durationMs 1 (첫 `12_start` Stage) |
| P84 logic.4 | logic.83, startMs 15333, durationMs 1 (두 번째 `12_start` Stage) |
| nextLogicOrdinal / P84 nextLogicOccurrenceOrdinal | 83→84 / 3→5 |

사용자의 이후 Workbench Save(revision 1391, 1395)가 3-way merge로 위 변경을 그대로 유지한 것을 확인했다.

## 검증

| 검증 | 결과 |
|---|---|
| 설치 WModel 기반 projector root-motion bake로 `12_start` 하강량 측정 | 10.890836 m, 최저점 1134 ms |
| P84 closure의 `validate_document`/`validate_publishable`/`projected_outputs` (revision 1381, 1395) | PASS, mechanicTriggers JUMP 0 / SLAM 0 / SLAM 15333 |
| Server 높이 규칙 재현 시뮬레이션(30 Hz) | 0 ms 10.89 m → 약1167 ms 착지, loop 동안 지면, `12_end` 상승 약15.8 m → 15333 ms SLAM → 약16500 ms 착지, 종료 높이 0 |
| Server tick 순서 코드 추적 | Prepare(Begin_Pattern·Enter_Stage) → Apply_StageRootMotion → Logic cue → Brain Update → Commit_KoukuMechanicTriggers 순서라 첫 tick에 Stage root motion이 있고 JUMP 행이 SLAM 행보다 먼저 커밋된다 |
| 두 번째 SLAM Stage 경계 | P84는 fixed timeline이 아니다. Stage별 clock으로 `12_start`(Stage 8) 진입은 T0+459, SLAM cue는 ceil(15333×30/1000)=T0+460이라 하강 곡선을 사용한다 |
| 원본 저장본 전체 `validate_document` | 기존 P32 세이튼_쇼타임(stage 없음) presentation lifetime 오류로 실패. 이번 변경과 무관하며 publisher는 Pattern 단위로 제외한다 |
| `.md` 문서 `git diff --check` | PASS |

적대적 검증 workflow 3개(Server tick 순서, Client 표시, Workbench preview)는 모델 사용량 한도로 결과 없이 종료됐다.
Server tick 순서와 Stage 경계는 위와 같이 직접 추적했다. Client 표시와 Workbench preview는 09-17 G10의
JUMP 0 ms + SLAM 검증(Client codec/native model 438 checks)과 같은 경로이며 이번에 재실행하지 않았다.

## 게시 상태

- revision 1381과 1395 게시는 사용자가 게시 도중 Save해 입력 변경 CAS로 중단됐고, publisher가 이전
  Product/Server 데이터와 receipt를 복원했다. 로그는 `out/KoukuHoopDescent20260918/publish.log`,
  `publish-1395.log`다.
- 저장이 2분 동안 없을 때만 게시하는 `out/KoukuHoopDescent20260918/publish_when_quiet.sh`로 다시 실행했다.
  1차(revision 1406)는 00:35 Save로 롤백됐고, 2차에서 **revision 1409를 네 domain 모두 PASS로 게시**했다
  (`publish-quiet.log`, 00:45:35 완료).
- 게시된 `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap`의 P84 행은 JUMP 10.890836 m @0 ms,
  SLAM @0 ms, SLAM @15333 ms이고 Stage root motion 9행이다. F1 inventory에서 P84와 P11의
  unavailableReason은 비어 있다.
- 저장 84개 중 69개가 게시됐고 15개는 기존 미완성 draft(stage 없음, 연출 Stage 다중 animation,
  judgementKind 없는 DURATION box 등)로 F1에 재생 불가 사유와 함께 남는다. 이번 작업과 무관하다.
- Server 재시작은 필요 없다. 다음 Complete Play / Play Isolated가 revision 1409를 승인한다.

## 대형 세이튼 노랑시선 이름 변경

사용자가 첨부한 화면의 대형 세이튼 눈에서 나오는 노란 ray는 V2 그룹 `boss.kouku.medusa.laser`다.
`Data/Effects/V2/Bindings/MN_RPCT_06.effectv2bindings.json`이 `mn_rpct_06_sk.ao_att_battle_7_01` 1333 ms에
`bip001-l_eye`로 붙이고, P11 `대형세이튼_파1빨2`가 같은 그룹을 머리 본 box 3개로 사용한다.
원본 근거는 action 4221813 `대형 세이튼_번뜩이는 공포`의 `Par_X_RPCT_Eye_02_02/03/04`다.

사용자가 준 문자열 `패턴 | 대형 세이튼 / 대형세이튼 | 노랑시선`을 표시 이름으로만 적용했고 stable ID는 그대로다.

| 위치 | 변경 |
|---|---|
| Composition `kakulsaydon.g1.presentation.38` displayName | `boss.kouku.medusa.laser` → 위 이름 (revision 1408→1409) |
| `Data/Effects/V2/Groups/boss.kouku.medusa.laser.effectv2group.json` | 선택 필드 `displayName` 추가 |

V2 그룹은 `EffectResourceTree.json`의 V1 폴더 `2관문 > 패턴 > 대형 세이튼` 아래에 둘 수 없다
(V2 참조 부모는 `root.v2`만 허용). 그래서 트리 문서는 바꾸지 않았다. V2 pipeline의 group root 필드 검사는
PASS다. 스크립트와 이전 bytes는 `out/SaydonYellowGazeName20260918/`에 있다.

## 사용자 확인 경로

`Lobby → KoukuSaydon → F1 → Action Workbench → Boss → 쿠크피자 폴더 → 쿠크_훌라후프` → (revision 1409 게시 완료)
→ 2관문 쿠크 대상으로 `Play Isolated`. 시작 직후 위에서 약1.1초 동안 내려와 착지하는지,
`12_end` 뒤 두 번째 `12_start`에서 다시 내려오는지 확인한다. 화면 판정은 사용자 몫이며 이 문서는 PASS로 기록하지 않는다.

## 산출물

- `out/KoukuHoopDescent20260918/apply_kouku_hoop_descent.py`: 측정·splice·admission·높이 시뮬레이션·CAS 설치
- `out/KoukuHoopDescent20260918/receipt.json`, `before/`, `candidate/`(projected P84, 높이 샘플)
- 공용 문서: `.md/GB/gotchas.md` 새 항목, `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md` ALBION_AIRBORNE 절 3줄
