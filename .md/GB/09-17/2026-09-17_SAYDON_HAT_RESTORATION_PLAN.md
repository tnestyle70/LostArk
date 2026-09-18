# 세이튼 모자 부착·재질 복구 계획

## G00. 목표와 기준

사용자가 저장한 `KAKULSAYDON_G1_PATTERN_83.logic.1`은 DURATION
`kakulsaydon.g1.logic.81`을 1573ms부터 5930ms 동안 사용한다. 종료는 7503ms다.
이 구간만 오른손 모자를 보여 주고, 평소와 종료 후에는 머리 모자를 유지한다.
일반·앙콜 세이튼, Character Preview와 시퀀스의 세이튼에 같은 부착 경로를 연결한다.
원본 action4219806에는 B_WP_2의 별도 모자 particle과 HidePawn notify가 있다.
HidePawn의 세부 payload를 머리 모자 visibility 계약으로 단정하지 않는다.
사용자는 저장된 오른손 구간을 요청했으므로 현재 DURATION의 시각과 손 선택을 우선한다.

## G01. 준비·렌더 계약

`NpcPresentationAssetService.h/.cpp`가 설치된 head 전용
`Character/KoukuSaton/WP_MN_RPCT_08/wp_mn_rpct_08_1_sk.wmodel`의 native catalog
재질과 기존 CModel Clone을 준비한다. 원본 head socket wp_3_1의 bone은
`bip001-head`, socket local TRS는 identity다. 실제 body의 source material identity와
head/hand bone을 대조하여 해당 세이튼 몸체에만 적용한다.

`Npc`, `Part_Body`, `WorldSequenceObject`의 기존 모델 수명과 렌더에 모자 모델을
함께 둔다. 모자 준비 실패 시 기존 몸체를 유지하고, 실제 draw 실패는 기존 Render의
HRESULT 실패 경계를 따른다. 본의 combined basis와 모델
preScale을 한 번씩 소비하고 기존 몸체·망치 Transform은 변경하지 않는다.
새 C++ 파일과 project/filter 등록은 필요하지 않다.

## G02. 오른손 WORLD와 머리 복귀

기존 WorldSequencePlayer의 BOSS anchor와 실제 `b_wp_1` 오른손 자식 본을 재사용한다.
hand 전용 `wp_mn_rpct_08_sk.wmodel`을 사용하며 native material source를 연결한다.
현재 저장된 Logic 시간에 일치하는 WORLD occurrence를 후보에 추가한다.
현재 Logic81은 judgementKind가 비어 있어 PRODUCT 게시가 거부된다. 기존
`ATTACHMENT_HOLD` kind로 시간 구간을 완성하고 새 판정이나 결과선을 추가하지 않는다.
definition이 해당 P83 occurrence에서만 소비되는지 검사하고, kind가 이미 다르면
덮어쓰지 않는다. 사용자 이름·시각은 유지하고 exact before/after를 patch에 남긴다.

기존 Showtime gun replacement와 같은 weak registration으로 실제 body instance와
살아 있는 WORLD object만 연결한다. 오른손 객체가 보일 때만 같은 body의 head hat을
숨긴다. 종료·Stop·seek·재사용·실패 시 객체 visibility/lease 수명에 따라 head가 복귀한다.
다른 actor instance의 모자는 숨기지 않는다. live data의 최종 stable-ID 병합과 publish는
상위 작업에서 최신 저장본을 다시 읽고 진행한다.

## G03. 검증과 남는 화면 경계

원본 PSK와 설치 WModel의 head/hand geometry, bone hierarchy와 model preScale을
대조한다. native 재질의 실제 slot·texture 존재를 확인한다. 변경 C++ 최소 컴파일,
JSON parse와 publisher, `git diff --check`를 실행한다. Client/UI 실행·조작과 최종
모자 위치·색·시각 판정은 사용자가 수행하며 실행 파일은 현재 사용 중인 채로 둔다.
