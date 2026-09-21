# 3관문 갈고리 몸체 접촉 포획 결과

## G00. 확인한 원인

서버 갈고리 판정이 플레이어의 발점 Y와 XZ 중심점만 검사했다. 실제 훅 끝 BOX는
지면 이동 구간에서 Y 약 2.13~2.24m, halfY 약 0.4025m다. 따라서 플레이어 몸통이나
옆구리가 훅에 닿아도 발점이 BOX 밖이면 포획이 빠졌다. 표시용 collider의 크기를 키워서
보정할 문제가 아니었다.

P18/P19의 18개씩과 P33의 30개, 총 66개 HOOK_CAPTURE window가 이미 GRAB_TO_WORLD_OBJECT와
grip track으로 연결되어 있었다. 설치 WModel `Character/KoukuSaton/MN_UMAX_00/MN_UMAX_00.wmodel`의
`b_hook_01`은 bone index 17, parent 16이며 modelPreScale은 0.00999999978, resource scale은
1.14999998이다. 이를 실제 저장된 모션·collider로 다시 bake한 30,180개 key는 게시된
region track과 모두 정확히 일치했다. 설치 모델과 diffuse/normal/specular texture도 존재한다.

Client `WorldSequenceObject::Try_GetAttachmentWorld`의 본 축 정규화·preScale 적용과
Python sampler가 같은 순서를 사용한다. Object Tool의 훅 끝·grip 표시를 바꾸거나,
Data·Resources에 누락된 포획 연결을 추가할 필요는 없었다. 이번 확인은 현재 훅의
포획 데이터와 모델에 관한 것이며 다른 원본 이펙트·음향 복원 완료를 뜻하지 않는다.

## G01. 변경

`KoukuSaydonLogicRuntime.cpp`에서 grip을 가진 훅 BOX만 기존 Shared 플레이어 몸체
반경 0.45m, 중심 높이 0.9m, halfY 0.9m와 contact margin으로 검사한다.
고속 훅은 기존 visible key 구간 안에서 수직 교차 시간을 먼저 구하고, 그 구간에서
원형 몸체와 이동 BOX 사이 최소 거리를 검사한다. 사각 확장으로 인한 모서리 오포획은
허용하지 않는다. 일반 logic 영역의 기존 point 판정과 visibility gap은 유지한다.

기존 포획 후 GRABBED/WORLD_HOOK_TIP, 저장된 grip XYZ 추적, 숨김 key의 release deadline을
그대로 쓴다. ClientReplication은 WORLD_HOOK_TIP에 보스 손 socket을 적용하지 않고
서버 XYZ를 사용하며, Character는 기존 DOWN_LOOP와 해제 표현을 소비한다.
Shared packet·Client 코드·projector·저작 데이터·Resources 수정은 없다.

## G02. 검증

- 수정 전 native 79개 검사 중 69개 실패를 재현했다. 실제 게시 P18 18개, P19 18개,
  P33 29개의 지면상 몸체 접촉 65개가 모두 발점 판정 때문에 실패했고 합성 4개도 실패했다.
- 수정 후 native 82개 전부 성공했다. 위 실제 65개, 몸통·측면·접선·고속 sweep,
  높이·모서리·분리·visibility gap의 거부, 기존 point 판정과 grip 추적·해제 인계를 포함한다.
- P33 나머지 한 창은 검사 중 지면 위 플레이어와 교차하는 표본이 없어 접촉 성공으로
  집계하지 않았다. 이 창의 source bake와 게시 track 일치는 별도로 확인했다.
- 영속 `Run_KoukuPushContracts`에 11개 회귀를 추가했다. 기존 넉백 검증과 합친 영속 35개,
  추가 포물선 native 57개도 모두 성공했다.
- `test_world_object_collider` 16개 성공. 변경 C++ 전체 TU 컴파일과 `git diff --check` 성공.

증거는 `out/KoukuHookCapture20260921`의 `baseline_full.log`, `after.log`,
`combined_push_hook_regression.log`, `source-bake-receipt.json`, `receipt.json`에 있다.
수정 전 두 파일은 `before/`에 보존했다. 제품 Client/Server 실행·화면 검증은 하지 않았다.

## G03. 최종 통합

변경 제품 파일은 `Server/Private/KoukuSaydonLogicRuntime.cpp`와
`Server/Private/ServerGameplayContractTests_KoukuOverlap.cpp` 두 개다.
부모 작업이 최종 제품 빌드를 소유하며 이 수정만을 위한 데이터 publish는 필요 없다.
실제 화면의 잡힘·끌림·해제 모습은 사용자가 직접 확인한다.
