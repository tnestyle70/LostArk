# 중단된 커밋·푸시·PR 작업 이어받기

## 범위와 기존 진행 상태

마지막 pull 기준은 `bddacace`다. Claude가 `40c7b400`부터 `2661d4c0`까지 기능별 커밋 7개를 만들고, 쇼타임 파일 13개를 stage한 상태에서 중단됐다. 해당 브랜치의 이전 PR #392는 이미 병합되어 이번 변경은 별도 PR로 전달한다.

기존 브랜치 `feature/kouku-cutscene-camera-map-0915`와 7개 커밋을 보존했다. 쇼타임, 로딩 변수·루가루 이동, 작업 기록을 추가 커밋했다. 병합 전 상태는 로컬 태그 `backup/pre-merge-20260918` (`55031365`)에 남겼다. 원격 main에 직접 push하거나 PR을 merge하지 않는다.

## 최신 main 통합

`origin/main`의 `a198c940`까지 9개 커밋을 병합했다. 양쪽 전체 파일 중 하나를 택하지 않고 아래 충돌을 함께 반영했다.

- Loader: Character Select 등록을 중복하지 않으면서 Valtan TriggerBox 등록을 유지.
- CNpc: 쇼타임 원본 배우 표시 억제와 main의 잔상 기능을 유지. 숨겨진 배우의 잔상 이력을 비우고 BLEND/일반 Render 모두 동일 표시 조건 사용.
- PresentationPlayer: 미리보기 종료 시 표시 억제 해제와 잔상 정리를 모두 실행.
- WorldSequenceObject: 발탄 반투명 메시 경로와 main의 세이튼 모자 준비를 모두 유지.
- Composition: main의 다른 패턴을 그대로 두고 P76의 기존 미커밋 연기·카메라 설정을 반영. revision 1497.
- Character Select LFS 배치: 통합한 저작본을 정본 publisher로 재게시. 조명 파일 SHA256 불변. 게시 Check 통과.

발탄 runtime camerashots는 실제로 파일이 없었다. source 13샷을 CameraShots scope로 게시하고 Check 후 추적에 추가했다. 이는 기존 데이터 전달 누락의 보완이며 전투 사망 카메라를 바꾸지 않는다.

## 검사 도구 전달 보완

쇼타임 카메라 검사가 Git 제외 `out/` 백업에 의존하던 부분을 추적 fixture로 바꿨다. 원본 4샷을 보존하며 현재 설치된 4샷과 수치 비교한다. 일회성 retarget 도구는 백업이 없는 상태에서 revision 88 이외의 문서를 재배율하지 않도록 거절한다.

## 검증

- 쇼타임 카메라 검사 3개, NPC 미리보기 표시 검사 2개, 카메라 한정 게시 검사 4개, 발탄 외곽 벽 검사 11개 통과.
- 변경 JSON 18개 parse 및 PR diff whitespace 검사 통과. 추가 발탄 카메라는 publisher가 parse/validate/Check.
- World Gameplay Validate: Bern 55, Valtan 153, Kouku 112, Training 4, Character Select 5 placements 통과.
- Valtan Area Validate: 13,184 placements / 24 files 통과. World Destruction Validate: 102 groups, 218 bindings, outer109 27 groups / 54 placements 통과.
- Bern 및 Character Select Navigation Validate 통과. 이는 바닥 메시 복구 또는 실제 보행 성공 판정이 아니다.
- 통합 Debug Product 빌드와 추가 Server/Composition 검사의 최종 결과는 아래에 기록한다.

## 완료와 구분해야 할 경계

- 쇼타임 P76은 DRAFT다. 미리보기용 2개 animation occurrence를 임의 삭제하거나 PRODUCT로 승격하지 않았다.
- 베른 LV_MODULE 272개 가시성 변경은 누락 통로 복구 완료가 아니다. 사용자 제보 통로의 메시·보행 문제는 미해결이다.
- NPC 19명 및 발탄 컷신 모델은 팀 Drive Resources를 별도로 받아야 한다. 모델 바이너리와 빌드 출력은 이 PR에 넣지 않는다. 상대 경로·배포 위치는 기존 NPC/발탄 RESULT를 따른다.
- 루가루 입장 이동은 09-16 결과의 '카메라 철회, 위치 이동 유지' 결정에 따라 보존했다. 관련 Server Debug 우회 제거가 포함된다.
- Client/UI를 실행하거나 화면 PASS로 기록하지 않았다. Release 빌드도 이번 Git 작업에서 실행하지 않았다.
- Framework.sln의 로컬 VS 버전/빈 폴더, 개인 설정·캐시·임시파일·백업·로컬 배포 목록은 보존하되 커밋하지 않았다.
