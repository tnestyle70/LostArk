# 쿠크 앵콜 가짜 클리어 UI와 앞부분 5초 단축

## 반영 내용과 원인

기존 클리어 표시는 Level의 서버 관문 완료 상태로만 시작했으므로 Action Workbench의
Sequence 단독 Play에는 표시되지 않았다. 일반 UI는 화면 유리 후처리보다 뒤에 그려져,
표시만 연결해도 유리 재질 입력에는 포함되지 않는 구조였다.

KoukuSaydonPresentationPlayer의 공통 Sample 경로에서 기존 앵콜 camera asset ID를 가진
occurrence의 시간을 소비한다. 기존 RaidClear_Kouku Layout/키프레임과 던전 클리어 캡션을
재사용하며 SceneHDR의 opt-in SCENE_UI 그룹으로 유리 후처리 전에 그린다. 일반 HUD/UI의
기존 그룹과 순서는 유지한다. 보상, 서버 관문 완료, MVP를 발생시키지 않는 표현 전용 UI다.
Stop/종료에서는 숨기고 역방향 Seek에서는 같은 시간으로 다시 샘플한다. UI 준비 실패는
해당 UI를 격리하고 진단을 보존한다. 새로운 C++ 파일은 없고 큰 기존 TU의 /bigobj만 등록했다.

사용자가 Save했다고 확인한 디스크 저장본을 바탕으로 다음을 함께 5000ms 앞당겼다.

- Sequence P10과 Boss P97: 전체 26322 → 21322ms, 배우/카메라 구간 23333 → 18333ms.
- WorldSequence: 배우 motion key, 원본 animation sourceStartMs=5000, spark/파편14개 시간.
- 카메라 key와 끝 암전 곡선. 다른 카메라/World template는 보존했다.
- 유리 재질 시작 12.5 → 7.5초. 원본 material curve는 보존하고 source-time origin을5초 이동했다.
- 5초 전에 끝난 팝업 효과음 occurrence만 제거. 이어지는 음성 source offset은2900ms,
  BGM offset은5000ms로 이동하여 잘린 시간만 건너뛴다. 음원 파일은 수정하지 않았다.
- 자막 시작 10233/12367/16700 → 5233/7367/11700ms. 기존 문구·크기·위치는 유지했다.
- 가짜 UI는 단축된 첫 화면부터 표시하고10433ms에 숨긴다. 첫 균열7500ms,
  두 번째 균열8533ms, 파편10433ms와 같은 원본 시간 기준을 사용한다.

`trim_encore_lead_in.py`는 대상 stable ID만 교체하고 다른 JSON 행의 바이트를 보존한다.
기존 저장본은 out/KoukuEncoreTrim20260923/before에 백업했으며, 동시 저장 freshness 검사와
원자 교체·자기 변경 rollback을 사용했다. 이미 단축한 문서에 재실행하면 사전 조건에서 거절한다.

## 검증과 게시

- Debug Product Engine/Shared/Server/Client 빌드 성공:
  out/BuildPipeline/runs/20260923T041024321Z-debug-product.json.
- 첫 빌드는 기존 대형 PresentationPlayer TU의 object section 한도 오류가 발생했다.
  해당 파일의 /bigobj를 추가한 뒤 재빌드에 성공했다.
- 단축/자막/음성 source offset/배우/카메라/유리 source clock/리소스 검사7개 PASS.
- WorldSequences 및 CameraShots 런타임 Publish 성공.
- Kouku projector Publish 성공: sourceRevision2224, 저작119/실행113 패턴 유지.
- 최종 Gameplay 및 Composition 게시 결과와 서버 재검사는 동시 작성한
  RAID_UPDATES_PR_RESULT의 통합 검증에 기록한다.
- 사용자는 "지금 나오는거까진 확인"했다고 답했다. 이는 표시 확인이며 전체 구간의
  원본 일치나 최종 화면 품질 PASS로 확대하지 않았다. 에이전트는 Client/UI를 실행하지 않았다.

## 배포와 남은 경계

이번 수정으로 생성/변경한 Resources 바이너리는0개다. 기존 배우, 음원, 유리 텍스처,
UI/RaidClear와 Font_YoonGasiIIM을 재사용한다. Data 저작본과 Client/Server 게시본은 Git으로
전달하며 수신자는 변경된 Engine/Server/Client를 빌드하고 실행 중 Server/Client를 다시 시작한다.
Release 빌드와 전체 연출의 사용자 육안 확인은 별도다.
