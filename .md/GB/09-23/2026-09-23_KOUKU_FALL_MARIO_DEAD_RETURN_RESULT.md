# 쿠크 도박장 낙사와 마리오 사망 복귀

## 요청과 적용

- 도박장(2관문 SL03)은 실제 지면을 벗어나면 기존 Server FALLING 중력·사망 타이머를 사용한다. 낙사 후 기존 부활 명령을 누르면 stage.kakul.sl03의 네비게이션 검증된 중앙으로 부활한다. 자동 부활을 추가한 것은 아니다.
- 마리오 1~4의 걷기와 일반 밀림은 물리 지면이 없는 곳에서 이동 네비게이션의 경계 정지보다 낙사를 먼저 처리한다. 물리 지면이 남아 있는 장애물을 구멍으로 취급하지 않고, body sweep 충돌도 유지한다.
- 마리오 사망자는 입장 때 고정한 복귀점(없으면 stage.kakul.sl05)으로 HP 0 / DEAD를 유지한 채 복귀한다. 낙사뿐 아니라 피해에 의한 사망도 해당한다. 정상 완료 콜백을 호출하지 않는다.
- 낙하 중에는 stage/복귀 pin을 보존한다. 사망 복귀 commit에서 마리오 rail·폼·HUD·입력·이동·attachment를 해제한다. 기존 Shared snapshot으로 Client에 전달하며 별도 Client 로컬 순간이동을 만들지 않았다.
- 복귀점 검증 실패 시 죽은 마리오 상태와 pin을 유지하여 다음 tick에 재시도한다. 그동안 부활 명령으로 마리오 내부에서 살아나는 우회는 거절한다.
- authored TriggerMove 점프/입출장은 낙사 진입 검사에서 보호한다. 기존 강제 arena ejection과 ballistic flight의 별도 이동 경로는 유지한다.

## 원인

일반 걷기는 Resolve_TraversalStep 실패 시 정지했다. 또한 기존 Update_MarioControlState는 FALLING 또는 DEAD에서 Clear_MarioControl을 먼저 호출하여 stage와 복귀 좌표를 지웠다. 도박장의 기존 부활은 떨어진 사망 위치를 다시 투영했다.

추가 검사에서 Gate2Fine의 25×25.5m 정밀 bake 사각형 밖에도 기본 grid의 도박장 바닥이 이어짐을 확인했다. 정밀 grid 경계를 도박장 경계로 사용한 첫 구현은 도박장 검사 3개가 실패하여 폐기했다. 현행 코드는 분리된 SL01~SL05 playerSpawn 중 가장 가까운 정본 스테이지가 SL03인지로 도박장 적용 범위를 구분하고, 실제 낙사는 별도의 물리 지면/높이/충돌 검사로 결정한다. 스테이지 거리로 바닥을 만들거나 충돌을 없애지 않는다. Mario 및 Maze 상태는 이 도박장 분류에서 제외한다.

## 검증

- Debug Product Engine/Shared/Server/Client 빌드 성공: out/BuildPipeline/runs/20260923T024640379Z-debug-product.json.
- 1차 실제 Server --debug-teleport-contract-test 실패 0: out/kouku-fall-contract-20260923.log. 네 stage 각각 실제 nav 지면 끝, 낙하 중 pin 보존, DEAD 복귀, 피해 사망, 점프 보호 및 기존 회귀 포함.
- Navigation publisher Validate 성공: LV_LUT_MIDNIGHTC_ED와 8개 refinement region. 저작 데이터나 게시 산출물을 이번 기능 때문에 변경하지 않았다.
- 도박장 범위 교정 전 검사: 실패 3건, out/kouku-fall-contract-final-20260923.log. 교정 후 최종 검사 실패 0 / exit 0: out/kouku-fall-contract-final2-20260923.log. 도박장 지면 끝 낙하·사망·부활 명령의 중앙 도착과 마리오 네 stage의 사망 복귀를 실제 게시 데이터로 검증했다.
- 변경 C++의 git diff --check 성공. 새 파일/프로젝트 등록 및 runtime JSON 변경 없음.

## 사용자 화면 확인

Server + Client Debug를 새로 실행한다. 마리오 각 stage에서 점프 없이 틈으로 걸어 낙하하는지, 사망 후 팀원이 있는 3관문 아레나로 돌아오되 죽어 있는지 확인한다. 정상 점프로 건너는 경우와 도박장 낙사 후 부활 버튼의 중앙 복귀도 확인한다. Client를 에이전트가 실행하거나 화면을 판정하지 않았다. Release 빌드와 사용자 시각 검증은 아직 하지 않았다.
