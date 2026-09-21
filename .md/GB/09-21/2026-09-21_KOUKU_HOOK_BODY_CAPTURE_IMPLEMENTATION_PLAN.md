# 3관문 갈고리 몸체 접촉 포획 구현 계획

## G00. 현재 실측과 범위

P18/P19 각 18개, P33 30개의 Object Collider 창에 GRAB_TO_WORLD_OBJECT와
bone 기반 grip world track이 게시되어 있다. 실제 바닥을 훑는 훅 끝은 Y 약
2.13~2.24m, collider halfY 약 0.4025m인데 서버는 플레이어의 발점 Y와 XZ
중심점만 검사한다. 몸통이나 옆구리가 닿아도 발점이 작은 훅 BOX 밖이면 잡히지 않는다.
Object Tool의 b_hook_01 본, preScale, 정규화한 본 축과 source bone bake를 대조한다.

## G01. 구현

grip을 가진 훅 BOX만 기존 Shared 플레이어 몸체(반경 0.45m, 중심 높이 0.9m,
halfY 0.9m, contact margin)로 접촉한다. 기존 일반 logic 영역의 중심점 판정은 유지한다.
고속 훅이 한 fixed tick 사이에 지나가면 같은 보이는 key 구간의 continuous overlap을
검사한다. 원형 몸체를 단순 확장 사각형으로 대신하지 않고 BOX 모서리 거리도 검사한다.
기존 포획·grip 추적·release·snapshot·Client DOWN_LOOP 표현 경로를 사용한다.
저작 collider를 임의로 키우거나 위치를 옮기지 않는다.

## G02. 검증과 반영

실제 설치 WModel 골격·clip과 저장된 collider를 표본 계산하고, 게시된 P33 track의
지면 높이와 몸체 교차를 확인한다. 몸통 접촉·측면 접촉·모서리 비접촉·높이 비접촉,
빠른 swept 접촉과 visibility gap, 기존 일반 point 영역을 native로 검증한다.
포획 후 GRABBED/WORLD_HOOK_TIP과 다음 grip 추적·해제도 확인한다.
Data 실물 편집과 제품 실행은 부모 작업이 소유한다. 새 제품 C++ 파일은 없다.
