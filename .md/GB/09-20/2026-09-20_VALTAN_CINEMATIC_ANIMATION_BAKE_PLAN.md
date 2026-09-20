# 발탄 원본 연출 애니메이션 병합 계획

SCENE06A Matinee 53의 입장과 54의 버러지들, SCENE04A Matinee 24의 사망 연출을 원본 Anim:A/B 및 UP_A/B 시간표로 샘플링한다. 설치된 발탄 AnimSet의 146개 애니메이션과 mesh/material/skeleton 섹션은 바이트 그대로 보존하고, 별도 Cinematics 경로의 파생 AnimSet에 세 연출 클립을 추가한다. 원본 파일은 덮어쓰지 않는다.

입장 walk_normal_1의 역재생, 구간별 재생 속도와 반복, A-over-B 곡선을 반영한다. 사망 UP_A는 원본 animblendingmat.animblending_mix의 bip001-spine1 하위 마스크를 사용한다. 원본 H_Up의 부모 본 공간 가산 회전도 동일한 트리와 강도 곡선을 사용한다. 본의 위치·회전·배율은 기존 WModel 좌표계에서 한 번만 샘플링하고 월드 이동은 기존 WorldSequence가 소유한다.

신규 파일은 Python 변환 도구 하나이며 C++ 프로젝트 등록은 필요 없다. 기존 BossCatalog와 WorldSequence의 animationSetId 및 animationTracks 변경은 발탄 기능 담당자가 최신 디스크 필드 병합으로 수행한다. 모든 기존 애니메이션 섹션의 hash 보존, 역재생과 상체 마스크 수치 대조, 실제 CModel의 본체/파생 AnimSet Attach 및 세 연출 전체 프레임의 본 행렬 검증을 수행한다. Client와 화면을 자동 실행하지 않는다.
