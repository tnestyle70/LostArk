# 앵콜 유리 파편·spark World 연결 구현 계획

## G00. 원본과 현재 소비자

SCENE07A의 break 12개와 spark 2개는 모두 actor 24에 붙는다. actor 24는 camera 3에,
camera 3은 dummy 4에 붙는다. 이펙트의 Move는 각 한 개의 parent-local 상수 키다.
기존 앵콜 World Sequence의 actor slot이 원본 부모 위치·회전을 이미 재생하므로
같은 slot의 V1 Effect track을 쓴다. 새 원점이나 카메라 경로를 만들지 않는다.

## G01. 후보 작성

원본 Move 위치와 회전을 클라이언트 basis로 한 번 변환한다. 기존 몸체 preScale
0.017/native 0.01 비율인 1.7을 local offset과 원본 drawScale에 각각 한 번 적용한다.
현재 카메라와 배우 track은 읽기만 한다. 파편은 기존 V1 2종의 유한 emitter를 재사용하고,
spark 2종은 원본 세 번의 Trigger/OFF 구간을 기존 instantiate 도구로 파생한다.
꺼진 후 살아 있는 입자의 꼬리는 보존한다. World Sequence는 14개의 stable effectTrackId,
EffectCatalog는 두 개의 파생 문서 행만 추가할 수 있는 field patch로 인계한다.

## G02. 검증과 경계

source parent chain, lookup, Move/Toggle, 기존 material/recipe를 대조한다. 실제 저장된
배우 quaternion·위치와 1.7배 카메라로 활성 시간의 중심/축을 계산한다. 네이티브 codec으로
문서·World Sequence 후보를 검증하고 explicit OFF 이후 재발생과 파편의 유한 수명을 확인한다.
기존 P97과 Sequence 10의 동일 instance 참조를 확인한다. 라이브 Data, Resources,
카메라·애니메이션, 제품 C++와 UI는 수정하지 않는다. 화면 판정은 사용자에게 남긴다.
