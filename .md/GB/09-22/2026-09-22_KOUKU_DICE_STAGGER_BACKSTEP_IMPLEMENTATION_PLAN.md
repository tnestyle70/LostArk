# 주사위 속박·무력화·백스텝 원본 표현 복원 계획

## G00. 사용자 화면과 변경 경계

주사위 패턴의 속박된 플레이어 발밑에 원형 문양과 버블을 표시하고, 원본 카드 출력과 터짐을 복원한다. 무력화는 보라 방패, 별 선, 원본 무지개 폭발을 사용한다. 백스텝의 일곱 화염 링은 1관문 중심으로 이동하며 이전 위치에 실제 세이튼 모델의 흰 잔상을 남긴다.

Composition, Catalog, Resource Tree와 프로젝트 등록은 통합 담당이 최신 저장본에 stable ID와 필드별 before guard로 병합한다. 이 작업은 후보와 manifest만 작성한다. Client/UI 실행과 화면 판정은 사용자 몫이다.

## G01. 원본과 연결

- `KoukuSaton_Star_Jail` 버프는 `Spotlight_01_LOC_INT`와 해제 `Spotlight_02_LOC_INT`를 참조한다. 이미 설치된 native source leaf를 파생하고, 서버 `isPatternBound`/`iPatternBindEndTick` 상태를 기존 Presentation Player `Sample` 세션으로 표현한다. 사망·퇴장·Reset은 즉시 해제하며 정상 속박 해제는 원본 해제 이펙트를 재생한다.
- 카드 출력은 Action 4219840 stage 2의 Sk08, Sk08_2, Eye01 19개 요소를 복원한다. 기존 6개 출력은 첫 시스템만 남아 있었다. 기존 카드 투사체의 사용자 위치·크기와 원본 충돌 폭발 경로를 보존한다.
- 무력화는 원본 Shield01, Sk12_2/9/4/3 별 그리기, Sk12_5 폭발을 사용한다. 기존 별의 임시 vertex burst는 새 문서에 포함하지 않는다. Sk12_9의 follower 위치 곡선은 기존 해석을 재사용하고 smoke 재질의 2992 대체를 원본 native3008로 복원한다. 기존 외곽 경계와 레이저를 보존한다.
- Backstep 링 위치는 설치 Gameplay 문서의 G1/G3 보스 중심 차이 xyz만 더한다. 각 링의 상대 간격, 타이밍, 회전과 크기를 유지한다.
- Action 4219951 stage 4의 TrailGhost 원본은 50ms 방출, 5ms 샘플, 500ms 잔상, source color intensity 1이다. 실제 CNpc/CModel과 CSkeletalAfterimage를 재사용한다. 흰색 반투명 appearance는 사용자 요청에 따른 PROJECT_AUTHORED이며 원본 rim/fade shader 복원과 구분한다.

## G02. 파일과 소비자

`KoukuSaydonPresentationPlayer.h/.cpp`는 속박 세션의 소유·샘플링·Reset과 백스텝 notify 시간 판정을 담당한다. `Npc.h/.cpp`는 현재 모델과 뼈 자세의 공통 잔상 설정을 담당한다. 신규 C++ 파일은 없어 프로젝트 등록이 필요 없다. Python 후보 작성기는 `out/KoukuPatternRestore20260922`에 후보 JSON, source receipt, stable-ID manifest를 생성한다.

## G03. 검증과 완료 경계

원본 LOA SHA·source notify·설치 리소스 존재, JSON parse, 요소 native profile 및 native Codec/Playback 수치 sweep, stable-ID 필드 guard와 7개 위치 delta를 검사한다. 컴파일은 통합 담당이 제품 빌드에서 수행한다. native sweep은 렌더 가능 carrier와 유한 geometry 검사이며 실제 GPU 색·방패 방향·본 부착 화면 확인의 대체가 아니다. 최종 RESULT에 후보/적용/컴파일/사용자 화면 확인을 분리한다.
