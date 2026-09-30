# 워터팡 입장 선행 준비와 NPC 물총

## G00. 실제 원인과 반영

Maharaka Loader는 입장 class 하나만 준비했다. Server는 경기 예약 뒤 `Update_MaharakaWaterpangAI`에서
20명 roster를 생성하며 Client 첫 spawn/snapshot에서 나머지 Guard/Lance class, 모코코12종의
head/outfit24개 모델, NPC8종의 모델·재질을 읽었다. 정확히6초에 고정된 Client timer는 없으며,
사용자가 관찰한 대기 중 정지와 연결되는 이 cold-load 경로를 입장 Loader로 옮겼다.

NPC8종은 `npcAppearance` 분기에서 Character 물총 arm을 건너뛰었고 표시되는 CNpc에는 총을
만드는 코드가 없었다. 기존 CPart_Equipment를 실제 NPC 오른손에 연결하고 현재 body pose 이후
갱신한다. snapshot armed=false와 사망/숨김은 표시를 중단하며 NPC 제거 시 part도 해제된다.
Server nickname은 기존 replicated player view를 통해 보존된다.

## G01. 사용한 경로

- Loader: owner thread가 item→visual-set ID를 캡처하고 worker가 class/의상/NPC/물총 prototype을 준비한다. 각 준비 사이 취소를 확인하고 실패는 기존 Level rollback을 따른다.
- EquipmentPresentationService: preload와 실제 착용이 같은 Admit_Models를 사용한다. 기존 level prototype이 있으면 model/material 파일을 다시 열지 않는다.
- MaharakaWaterpangPresentation: 기존 물총 prototype 준비 함수를 공용화했다. Character와 NPC는 같은 model prototype을 사용한다.
- NPC: 실제 `bip001-r-hand`가 필요하며 없으면 부착을 거절한다. 원본 주민 clip을 유지한다. 신규 C++ 파일·프로젝트 등록·리소스 수정은 없다.

## G02. 실제 리소스 검증

`python Tools/ActorXAssetCooker/verify_waterpang_contestants.py` PASS.
증거는 `out/WaterpangPreloadReview/result.json`이다.

- Shared NPC roster8종의 실제 WModel·idle clip·오른손 본 존재 확인. RootNode 있는 NPC4종은 .0001,
  주민4종은 .01 preScale을 거쳐 최종 손 basis가 모두 .01이다.
- Guard/Lance 물총 idle/run 실물 animation set과 모코코24개 모델 파일 존재 확인.
- 실제 물총 4,126 vertex·2mesh, 전체 외곽 크기 .770406×.286949×.234045m.
- Guardian watergun_idle의 `prop3 × inverse(right hand)` 실측으로 offset
  (.112383735, .040912395, -.019941085)m, pitch/yaw/roll (-20.648280,41.948665,11.908303)도를
  도출했다. 생산 NPC 코드 값과 최대오차 offset1e-6m·각도1e-5도 미만을 확인했다.
- 생산 소스에서 preload→Level commit, warm prototype 재사용, NPC visible→현재 pose→장비 갱신,
  snapshot armed 연결과 Server nickname 보존을 확인했다. `git diff --check` PASS.

이 수치는 원본 NPC 물총 자세 복원이나 실제 화면 PASS를 뜻하지 않는다. 빌드는 통합 세션이
최종 source로 수행한다. Client/UI는 실행하지 않았으며 대기 구간 frame 체감과 최종 장착 모양은
사용자가 새 Client/Server로 확인한다. 기존 물총 탱크의 미복원 반투명 submesh 숨김은 유지했다.
