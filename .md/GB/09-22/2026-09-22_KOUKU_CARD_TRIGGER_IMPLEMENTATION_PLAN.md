# 세이튼 카드 Trigger 구현 계획

## G00. 현재 입력과 범위

`KAKULSAYDON_G1_PATTERN_78.logic.9~12`는 사용자가 만든 Trigger이며 startMs는 7643, 9848, 11997, 14252다. 대응 definition 125~128에는 triggerKind가 없어 아직 이름뿐이다. 기존 logic.1의 DURATION definition73이 4초 간격 생성을 계속 소유한다.

## G01. Trigger 소비자 연결

기존 PURSUIT_PROJECTILES를 TRIGGER에서도 허용하고 spawnIntervalMs=0 한 번 생성만 받는다. Client codec/save/authoring UI, projector, Preview가 같은 정의를 소비한다. 기존 Server pursuit projection과 CombatObject를 사용하며 새 런타임은 만들지 않는다. 네 definition에는 각 문양 하나와 lifetimeMs=0을 연결하고 기존 duration occurrence만 비활성화하는 field patch 후보를 만든다. live authoring 파일은 통합 담당자가 최신 저장본과 병합한다.

## G02. 방향과 수명

Server와 Preview의 기존 homing spawn bearing270도를90도로 바꾸어 정확히180도 반전한다. 비추적 회전카드는 그대로 둔다. 생성 이후 접촉/대상 소멸/명시적 중지 전까지 추적한다. Trigger box 길이는 카드 lifetime이 아니다.

## G03. 검증

기존 projector/attack tests와 focused Client codec roundtrip, 변경 TU 최소 컴파일을 사용한다. 새 C++ 파일과 프로젝트 등록은 없다. 후보의 네 원본 occurrence ID와 timing 보존, 단발 생성, 무기한 수명과 old duration 비활성화를 확인한다. Client/UI는 실행하지 않으며 실제 화면은 사용자가 확인한다.
