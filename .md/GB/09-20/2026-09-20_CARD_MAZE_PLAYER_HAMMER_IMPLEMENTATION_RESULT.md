# 카드미로 플레이어 뿅망치 결과

정상 class 몸체를 유지하는 MAZE에서 interaction binding이 광대 전용이라 애니메이션이 없었고, 기본 무기를 대체하는 part도 없었다. 기존 CModel/CharacterPart와 typed interaction 명령에 연결했다.

## 구현

- 여섯 class native `pr_it_gstfp_00_att_1_01`(LMB 1초), `pr_it_gstfp_00_att_2_01`(Q 2.5초)를 각각 동일 class PSA에서 추출했다. 설치 body의 전체 skeleton/hash를 보존한 별도 animation-set WModel을 CharacterCatalog에 추가했다. 기존 body 파일은 변경하지 않았다.
- 일반 class MAZE 진입 snapshot에서 기본 무기를 숨기고 `WorldObject WhirlwindHammer.wmodel`을 실제 `bip001-r-hand`에 장착한다. 이탈하면 원래 stance의 무기를 복구한다. IT_GSTFP_00 모델은 09-06 검증에서 프라이팬으로 확인되어 사용하지 않았다.
- 실제 hand combined basis는 여섯 class 모두 .01이다. 망치 원본 길이 .56016m에 part preScale2를 적용하여 기본 약1.1203m이며, 이후 character size를 따른다. 원본 손목 basis 위에 별도 F1 조절값을 덧씌운다.
- LMB는 기존 wire W→MAZE index1, Q는 index0이다. MAZE 중 키보드 W는 같은 액션을 중복 제출하지 않는다. 기존 capture/free-camera 경계를 지킨다. Server action 1000/2500ms, 타격 시점 LMB12tick(.4초, 프로젝트 튜닝)/Q30tick(1초, 기존)을 소비한다. 판정과 HP 권위는 Server다.
- All Effects의 각 class에 카드미로 LMB/Q 행, native clip Play와 공유 빈 Effect 문서 Open을 추가했다. 빈 두 문서는 등록된 direct-authored slot이며 사용자가 실제 FX를 붙여 Save하면 기존 제품 reload/preparation 경로가 갱신한다. 빈 상태에서는 FX를 준비/생성하지 않는다. Scene 캐릭터에 영향을 주지 않고 미리보기의 일반 스킬 복귀 시 임시 망치를 해제한다.
- F1 Camera의 Character Size 아래 Card Maze Player Hammer에서 Pos(cm), Rotation(deg), 축별 Size를 조절한다. 맵별 camera profile의 기존 Save/Reload/CAS/Reset을 재사용한다. 기본0/0/1이며 쿠크 휠윈드 screenshot 수치는 복제하지 않는다.
- 일반 광기 변신은 무기를 숨기고 Mario만 표시한다. Apply_NetworkStance가 다음 snapshot에서 광기광대의 무기를 다시 켜던 우회도 Set_PartVisible에서 막았다.

## 검증

- Client 7 TU(등록된 빈 slot 준비/실패 격리 포함)와 Server 5 TU 변경분 최종 컴파일 성공.
- 실제 CModel WARP에서 여섯 body에 설치후보 animation sets를 attach하고 12개 clip의 모든30fps hand frame을 샘플링: finite/길이/actualhand 확인 PASS. 창/Draw 호출없음.
- ArenaCameraProfile 실제 production parser/save 검증168개 PASS: 구형 profile optional default, TRS roundtrip, NaN/범위/zero axis 거부, stale write 시 기존 파일 보존 포함.
- Codec에서 두 empty document Load/Validate/Serialize-Parse roundtrip PASS. empty라 Drawable/particle 검사는 해당 없음. `out/BingoRepair20260920/cpu/maze-blank-probe.log`.
- 기존 CardMaze Server 계약 검사에 실제 Q/W admission 및 LMB 타격 전/시/후 단일 HP 감소 검증을 추가했다. 최신 Debug Server의 `--card-maze-contract-test` PASS(0 failures), `out/KoukuRowLifetime20260920/card-maze-v2.log`. 타격 후 검사에서 fixture가 같은 serverTick에 UpdatePlayers를 두 번 부르던 부분을 실제 fixed tick과 같은 증가 방식으로 바로잡았다.
- Resources 설치는 최신 hash확인/백업/원자교체 후 candidate와 byte 일치. `out/CardMaze20260920/installed-v2.json` 및 `candidates-v2/receipt.json`.

## 남은 화면 확인

Client/UI는 실행하지 않았다. 본에 실제로 붙은 상태와 오른쪽→왼쪽 휘두르기/점프내려치기의 화면 결과는 사용자 확인 대상이다. native PSA clip1/2와 실제 모델 골격 연결은 수치로 검증했다. LMB contact .4초는 원본 notify 확정값이 아닌 프로젝트 튜닝이다. 빈 FX 두 슬롯은 사용자 저작을 기다린다.

최종 JSON 15개와 Client project/filter XML 2개 parse PASS. 변경 C++/JSON/project 범위 git diff --check PASS. 기존 LF/CRLF 경고만 표시됐다.

## 2026-09-20 카드미로 HUD와 Q 피해 교정

- `KoukuMadnessGaugeView.cpp`의 Update는 MAZE HUD 또는 카드미로 참여 role 동안 캐릭터 광기 게이지를 숨긴다. 카드미로 이탈 후에는 기존 표시 조건으로 복구한다.
- `MainApp.cpp`는 카드미로에서 Q만 스킬 아이콘/쿨다운을 표시하고 W의 LMB 키라벨 치환을 제거했다. 기존 빈 W 슬롯으로 돌아가며 실제 LMB 망치 입력/애니메이션은 유지한다.
- `GameRoom_KoukuMiniGames.cpp`는 카드미로 Q(index 0)의 raw damage만 500으로 선택한다. LMB와 Mario는 기존 100, 문양 병정의 한 번 처치·조각 표시 계약도 유지한다. 현재 clown box는 HP 500/방어 0이므로 Q 한 번에 파괴된다.
- 기존 `ServerGameplayContractTests_CardMaze.cpp`를 실제 typed Q 명령→contact 전/시/후 검증으로 강화했다. LMB 100 유지, Q damage event 500, full-health box 한 번 파괴, 중복 타격 방지, 1~4인 카드미로 탈출 검사가 모두 통과했다.

Client 2 TU(`MainApp`, `KoukuMadnessGaugeView`)와 Server 2 TU(`GameRoom_KoukuMiniGames`, `ServerGameplayContractTests_CardMaze`)를 별도 out 경로에서 컴파일했다. 기존 `Camera_Free.h`의 인코딩 경고가 있었으나 컴파일 오류는 없다. 변경 Server OBJ를 기존 제품 OBJ와 격리 링크한 `out/CardMazeHud20260920/card-maze.exe --card-maze-contract-test`는 `card maze failures: 0`, 종료 코드 0이다. 로그는 `out/CardMazeHud20260920/{client,server,link,card-maze}.log`에 있다. 실행 시 `LOSTARK_SERVER_DATA_ROOT`를 현재 `Server/Bin/DataFiles`로 명시했다. 최초 기본 경로 실행은 out/DataFiles의 bootstrap 부재로 시작 검사에서 실패했으며, 경로 지정 후 전체 통과했다.

Data/Resources/project 항목은 변경하지 않았고 별도 Publish는 필요하지 않다. C++/문서 변경 범위 `git diff --check` 통과. Product 통합 링크·배포와 실행 중 Server의 재시작은 이 독립 컴파일/계약 검사에 포함하지 않았다. Client/UI는 실행하지 않았으며, 카드미로 진입의 게이지 숨김·Q만 표시와 이탈 복구의 화면 확인은 사용자에게 남는다.
