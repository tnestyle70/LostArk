# 세이튼 카드·비둘기·넉백 연결 수정 결과

## G00. 원인과 소스 수정

기존 Preview는 연출 전용이라 해당 패턴의 Server 피격·넉백을 실행하지 않았다.
양수 push 결과 또는 PURSUIT_PROJECTILES를 가진 패턴은 기존 Server audition을
사용하도록 바꿨다. Parent와 Bundle도 포함된 패턴을 확인한다.

publisher는 OBJECT_CONTACT에만 본 track을 구워 ENTER_AREA 피해 collider의
WEAPON 본을 무시했다. 같은 기존 bake를 ENTER_AREA/OBJECT_OVERLAP에도 연결했다.
P13 첫 망치 타격의 실제 b_rpct_01 위치는 boss-local (9.770,-0.259,0.397)m인 반면
자동 생성 원은 대부분 (1.35,0,0)m에 있었다. 사용자의 첫 원 x9.6 보정은 보존 기준으로
확인한 뒤, 후보에서는 실제 본에 붙는 BOX로 대체했다.

카드 Trigger 정의125~128에 typed PURSUIT_PROJECTILES를 연결했다. Client codec/UI,
projector, 기존 Server 생성 경로와 preview sampler가 같은 값을 소비한다. 1ms의 짧은
박스도 생성 tick을 보장한다. Server와 Preview 생성 각도는270→90도로180도 반전했다.
무한 수명 카드의 전체 수명 CONTACT는10분 staging 상한 뒤에도 피해를 유지하며,
명시한 짧은 CONTACT 구간은 그대로 만료한다.

카드는1ms Trigger 종료와17582ms 자연 Pattern 완료 후에도 room-owned update와
독립 Client targeted visual loop를 유지한다. 자연 완료 뒤 Client가 Stop을 막던 조건을
`Snapshot::Can_Stop()`로 통일해 같은 epoch의 명시 Stop은 sequencer/F1에서도 제출한다.
기존 Server completed-owner STOP 검증과 Cancel_Source를 재사용한다.

## G01. 검토한 데이터 후보

- P78: 기존 사용자 trigger 시작7643/9848/11997/14252ms 보존, 각각 하트/스페이드/클로버/다이아 한 장, lifetime0, 간격0. 이전 Duration occurrence만 비활성화.
- Sound: `g_satan1_attack08_shot1`, full event `s_mob_g_koukusatan1.g_satan1_attack08_shot1`. 원본4219840 stage67~70에서 카드 출력0초와 AKEvent0.001초 확인. 설치된2.6초 WAV4개를 재사용하고 각 trigger+1ms에 Sound Box 추가.
- P13: 망치 피해 BOX4개, full5×7m XZ. 기존 카드 뒤집기 boxes와 World/MAP 배치를 보존하며 중복 피해 행 `.presentation.54`만 제거.
- P86/P87: 같은 망치 본 BOX, 각각1700/3533ms와1566/2200/2866ms에서100ms 판정. 실제 near-floor 본 pose에 맞췄다.
- P99: 저장 MAP 경고의 중심(1.927,10.560,325.496), yaw−225.9도, full6.6×18m BOX. 바람2396~3997ms, 전방 밀림 전용 result129.
- P85: 기존6m 거리는 유지하고 전용 result117의 비행242→1200ms. 중력9.8 기준 정점0.0717→1.764m. 공유 laser result104는 변경하지 않았다.
- 비둘기: 원본 native2893의 diffuse/normal/specular 출력과 현재 RT0-only 경로의 차이를 확인했다. 저작 밝기1은 유지하고 기존 mesh carrier에 scene directional 조명을 연결했다. 원본 o3/o5 및 양면 normal을 기존 recovered source-map Lambert/Blinn 식으로 소비하며 다른 material profile은 그대로다. texture·alpha·비행·폭발 데이터는 변경하지 않았다. 4배 RGB 보정 후보는 최종 설치 목록에서 제외했다.

## G02. 검증

- 기존 실제 Server overlap contract:134 assertions, failures0.
- 수정 후 실제 Server support-surface contract:288 assertions, failures0. 격리된 전체 Server DataFiles 복사본을 사용했고1ms 생성·전방·601초 이후 접촉 피해/despawn을 포함한다. 로그는 `out/SaydonPatternRepair20260922/pursuit-support-final.log`.
- 첫 실행의 기존4개 실패는 birth pulse/terminal event와 Showtime 중앙 생성 변경을 반영하지 않은 테스트 기대값을 교정했다. 추가1개는 요구된 격리 데이터 환경변수를 설정해 해결했다. 최종288개에는 모두 포함된다.
- 카드 focused Python4 tests, 새 codec parse/validate/exact roundtrip15 checks 통과.
- 대상6개 패턴 통합 prepare/publication projection 통과. 전체 Product89개에 모두 포함.
- 피해 BOX12개와 실제 installed WModel bone key111개 유한값 확인.
- 후보의 다른 필드 보존, 같은 필드 충돌 거부, 재적용 동일성 확인.
- 자연 완료 후 Stop의9개 상태 검사와 관련 Client3개 TU 최소 컴파일 통과.
- 비둘기 native2893 단일 generator 재생성 일치, MeshKouku2880 fx_5_0와 MaterialBinding C++ TU 컴파일 통과. 양면/음수 handedness를 포함한400개 basis 대조 최대 오차2.22e-16, 퇴화 basis finite와 lightCount0의 기존 RT0 유지 확인.
- 비둘기 최종16-light packet 검증 로그는 `out/KoukuCardDoveSound20260922/native-lighting/`의 shader-compile.log, cpp-compile.log, numeric-verification.json에 있다.
- 정본 Debug Product 증분 compile/link/deploy 통과. 실행 기록은 `out/BuildPipeline/runs/20260921T175920785Z-debug-product.json`.
- 테스트 fixture 교정 뒤 Product 증분 빌드도 통과했다 (`out/BuildPipeline/runs/20260921T180635839Z-debug-product.json`). 비둘기 셰이더와 자연 종료 후 Stop까지 포함한 최종 Product Debug compile/link/deploy도 통과했다 (`out/BuildPipeline/runs/20260921T183223657Z-debug-product.json`). Client OBJ7, CSO68, binary1 갱신이며 공용 입력 의존 셰이더도 함께 컴파일됐다. 기존 C4819/X4000/Effects deprecated/DirectXTK PDB 경고는 남아 있고 오류는0이다.
- C++ 변경 범위 최소 컴파일 통과. 신규 제품 C++·project/filter 항목 없음.
- `git diff --check` 통과.
- 변경 JSON/XML23개와 최종 설치 후보 JSON1개 parse 통과. 별도 Gameplay Validate는 현재 저작본의 generated Product가 stale이라는 선행 검사에서 중단됐다. 데이터 교체 뒤 정식 Composition/Gameplay publish·validate는 아직 남아 있다.

## G03. 반영 상태와 남은 검증

현재 데이터는 `out/SaydonPatternRepair20260922/candidate`에 준비된 후보이며 실데이터
교체와 publish는 사용자 저장 기준 확인 뒤 진행한다. Client와 Server EXE 종료 뒤 카드·추적·
비둘기·완료 후 Stop을 포함한 최종 Product 링크·배포는 완료했다. 데이터 적용 뒤 runtime
revision 및 bootstrap은 추가 확인 대상이다.

Client/UI를 실행하거나 화면을 판정하지 않았다. 최종 색·청음·타격 위치·상승/낙하 화면은
사용자 확인 대상이다. shared LAN server의 현재 메모리가 로컬 publish만으로 바뀌지는 않는다.

비둘기는 원본 재질 출력에 scene directional 직접광을 연결한 범위다. 원본 deferred shadow,
전체 SH lighting, point/spot까지 완전히 재현했다거나 GPU 화면이 원본과 일치한다고 기록하지 않는다.
