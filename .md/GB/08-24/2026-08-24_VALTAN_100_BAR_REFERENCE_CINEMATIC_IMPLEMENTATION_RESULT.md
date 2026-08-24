# 발탄 100줄 원작 컷신 구현 결과

## 1. 완료 결과

- 참고 영상 `tSpXa0v9TXw` 19:32~19:43의 흐름을 현재 `VALTAN_FOUR_PILLARS_105`에 연결했다.
- 화면 표기와 무관하게 제품 발동 기준은 사용자가 정한 `100줄`을 그대로 유지했다.
- 기존 109줄의 중앙 고정 도약·카메라·아레나 붕괴 계약은 변경하지 않았다.
- 새 맵 리소스 추출은 필요하지 않았다. 저장소에 있던 발탄 action 420610 애니메이션, 붉은 하늘 asset, exact carrier Effect를 재사용했다.

## 2. 제품 시간축

| 순서 | Server stage | 시간 | 제품 동작 |
|---|---|---:|---|
| 1 | `TAKEOFF` | 1933ms | 잠글 생존 플레이어를 한 번 선택하고 방향을 고정한 뒤 46m 수직 상승, 근접 컷에서 상공 컷으로 전환 |
| 2 | `YELLOW_ZONE` | 5500ms | 시작 때 저장한 대상 위치로 이동·가속 낙하, 붉은 구름과 검은 aperture 전개, 외곽 링 1회 판정 |
| 3 | `TARGET_CONE` | 3200ms | gameplay camera 복귀 후 230ms에 잠긴 방향 즉사 원뿔 판정 |
| 4 | `RECOVERY` | 1300ms | 붉은 하늘 종료와 4비석 생성 |

- 카메라는 `TAKEOFF` 전체와 `YELLOW_ZONE`의 앞 5200ms만 소유한다. 즉사 판정 전에 300ms 먼저 gameplay camera를 돌려준다.
- 카메라와 붉은 하늘은 `BOSS_XZ` 방식으로 발탄의 수평 이동을 따라간다. 109줄 cue는 기존 `WORLD` 고정 방식이다.
- 서버 도약이 좌표를 쓰는 동안 clip root motion을 중복 적용하지 않아 수직 상승과 잠긴 착지점이 흐트러지지 않는다.

## 3. 애니메이션과 Effect

- `mesh_att_battle_8_01_start/loop/end`를 `non-loop / loop / non-loop`로 정리하고 stage 시간을 원본 clip 길이에 맞췄다.
- action `420610`, sequence `2`, stage `19~22`를 reviewed source로 선택했다.
- 신규 Effect owner는 두 개다.
  - TAKEOFF: 3개 요소
  - TARGET_CONE: 23개 요소
- 총 26개 요소는 Sprite 16, Mesh 8, Decal 2이며 리소스 참조 60개가 모두 존재한다.
- 기존 carrier receipt의 역사 증거는 그대로 보존하고, 신규 2 owner·2 catalog row·2 cue·26 요소만 additive proof로 봉인했다. 이미 적용된 proof가 달라지면 write 모드도 실패한다.

## 4. 자동 검증

| 검증 | 결과 |
|---|---|
| Gameplay Balance Validate | PASS, 34 patterns / 131 stages |
| Balance Runtime Set Validate | PASS |
| Valtan root motion check | PASS, 28 patterns / 49 stages / 2323 samples |
| ActionPresentationTimelineHarness Debug/Release | PASS / PASS |
| 100줄 Server 제품 경로 contract | PASS: 100줄 1회 발동, 46m 상승, 잠긴 대상 착지, 4비석 |
| Server Debug/Release build | PASS / PASS |
| Server contract-test Debug/Release | PASS / PASS, failures 0 |
| Client Debug/Release build | PASS / PASS |
| carrier materializer check | PASS, `state=APPLIED`, `changed=0`, append 26 / owners 2 |
| carrier materializer unit | PASS, 11/11 |
| 신규 Effect resource closure | PASS, missing 0 |
| `git diff --check` | PASS |

전체 `Publish-Effects -Mode Validate`는 이번 Valtan 범위를 통과한 뒤 기존 무관한 DimensionMaster source seal 불일치에서 중단한다.

`effect.dimensionmaster.skill.2050180.v1.unified/effect.dimensionmaster.skill.2050180.unified`

해당 DimensionMaster 파일과 동시에 진행 중인 다른 Effect 작업은 수정하지 않았다. 전체 Effect 프로젝트 sync도 저장소의 기존 광범위한 미등록 corpus를 감지하지만, 이번 신규 Valtan 문서 두 개는 `Client.vcxproj`와 `.filters`에 직접 등록했고 두 구성의 Client build가 통과했다.

## 5. 수동 화면 검증 경계

- 에이전트는 Client를 실행하거나 화면 결과를 대신 PASS 처리하지 않았다.
- 사용자가 `Server + Client`로 들어가 F1 `Valtan Pattern Audition`의 `Reset + Play Sky + Pillar Cycle`을 눌러 최종 화면을 확인해야 한다.
- 확인 항목은 근접 충전 컷, 상공 상승 컷, 붉은 vortex, 카메라 복귀, 지목 유지, 착지 원뿔, 4비석 순서다.

## 6. 1~67 순서 재생 조작부

- Server가 이미 소유하던 `PLAY_ORDERED_1_67` 제품 계약과 67개 occurrence ledger를 그대로 사용한다.
- 1280x720 화면에서 긴 진단 정보 아래로 밀리던 조작부를 패널 안내문 바로 아래로 옮겼다.
- `Play / Restart Ordered 1-67`은 앞 occurrence가 끝난 뒤 다음 occurrence를 시작하며 marker와 repeat도 저작 순서대로 처리한다.
- `Stop Ordered Playback`으로 즉시 중지할 수 있다.
- 이 목록은 원본 영상 기준 1~67 occurrence이며, Encounter catalog의 34개 고유 패턴을 한 번씩 전부 재생하는 별도 목록은 아니다.
- Client Debug/Release build와 focused `git diff --check`를 통과했다.
