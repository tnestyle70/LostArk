# 보스 자막 표시 통일

## 기준과 반영

팀장 기준은 Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json의
KAKULSAYDON_G1_PATTERN_5 자막6개: scale [2,2,2], positionOffset [0,90,0].
기존 결과 09-22_KOUKU_SUBTITLE_LAYOUT_RESULT와 실제 저장본이 일치했다.

Sequence 자막20개와 Boss 정본 자막18개를 이 값으로 통일했다. 이미 기준값이던6개는
유지하고32개를 변경했다. revision은 Sequence168/Boss2223이다.
MainApp::RenderCinematicSubtitles의 WorldSequence 화면 자막(발탄 포함)도 같은 값을
사용한다. 폰트 YoonGasiIIM, 흰색, 검은 그림자, 화면 높이 비례 배율은 기존 공통값이다.
BALLOON 월드 말풍선은 유지했다. occurrence 위치·크기 편집 기능은 제거하지 않았다.

## 검증

- 두 원본 JSON의 자막38개 크기/위치 일치 검사 PASS.
- revision 및 해당 subtitle scale/positionOffset을 제외한 구조가 HEAD와 동일함을 검증.
  문구, 타이밍, 카메라, 애니메이션, 효과, 사운드 변경 없음.
- Debug Product 빌드 PASS: out/BuildPipeline/runs/20260923T010353399Z-debug-product.json.
- Kouku projector Publish PASS: revision2223, 저장119/실행113 패턴 유지.
- Gameplay Publish 및 Composition Publish PASS.
- scene subtitle 회귀7개 PASS, git diff --check PASS.

SequenceComposition은 기존 CProjectDataRoot의 authoring 문서 로드 경로를 사용한다.
Composition Publish 성공을 별도 SequenceComposition 복사본 생성으로 해석하지 않는다.
새 리소스는 없으며 바뀐 Client를 실행하고 저장된 문서를 다시 읽어 확인해야 한다.
Client/UI는 실행하지 않았다. 최종 자막 크기·위치의 화면 판단은 사용자 확인 대상이다.
