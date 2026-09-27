# main PR #465·#467과 로컬 작업 통합 결과

로컬 `5fc2f940a`와 원격 `8c7ad98b0`를 기능 브랜치에서 병합했다. 원격에 이미 병합된
초상 디퍼드 경로와 베른 선박·마하라카 변경을 받으면서 로컬 Guide, 드래곤 비행,
캐릭터 재질 컴파일 분리와 쿠크 수정은 보존했다. Client/UI는 실행하지 않았다.

## 충돌 해결

- 16개 파일의 충돌을 기능별로 결합했다. NPC 소멸자의 두 사운드·이펙트 정리와
  Server의 드래곤·선박 회귀조건을 모두 유지했다.
- 서로 다른 재질이 program 1526을 사용했다. 로컬 Guardian 의상은 1526을 유지하고,
  incoming `source.character.maharaka-ismp-1.v1`은 1528로 옮겼다. Maharaka-ismp-2는
  1527이다. Base/Light 함수·dispatch·CPU packing·named-vector patch·registry를 함께
  연결했다. 기존 로컬 함수는 그대로이며 incoming 함수는 번호 외에 변경하지 않았다.
- 작은 public header와 단일 CPP/generated INL 구조, 그룹별 leaf dispatch를 유지했다.
  새 Maharaka CPU packing은 `SourceCharacterMaterialParameters_Generated.inl`에 넣었다.
  두 프로그램은 기존 1472 그룹 안이므로 새 프로젝트·shader wrapper 등록은 없다.
- Sound의 Mario와 Maharaka class, 기존 7개 vehicle 전체와 신규 선박 9개를 모두 보존했다.
- 자동 병합된 vehicle publisher에서 선박용 `tunedSpeed`가 드래곤 override를 소비하지
  않는 연결 오류를 수정했다. 공식 publisher로 16 vehicles/34 skills를 다시 게시했으며
  드래곤의 지상/비행/수직 속도는 10/16/8, 선박의 원격 튜닝 값은 그대로다.

## 실행한 검증

- `Publish-VehicleProfiles.ps1 -Mode Validate`, `-Mode Publish` 성공.
- SourceCharacter group roundtrip·변경 격리 7개, program registry 4개 테스트 성공.
- 변경 JSON 19개 중복 key 포함 parse와 프로젝트/filter XML 6개 parse 성공.
- 기존 로컬 shader 함수 전부와 incoming Maharaka 함수 4개를 본문 비교했다. 번호 변경
  외의 차이 0, Engine/Client shader mirror 차이 0, dispatch 중복 0이다.
- 두 부모의 Sound class 전체, 로컬 vehicle 7개와 incoming 선박 9개의 의미 일치 확인.
- conflict marker 0. `git diff --check`의 유일한 5건은 incoming MainApp의 인코딩 관련
  의도된 줄 끝 공백이다. 해당 PR RESULT/gotchas의 지시에 따라 유지했다.

비교 보고서는 Git 제외 `out/Pr465467Integration20260927/structural-checks.json`에 있다.
이 통합 단계에서는 C++/FXC Product 빌드를 실행하지 않았다. 후속 기능 작업의 최종
Product 빌드에 포함해 검증하며, 현재 빌드 완료로 기록하지 않는다.

## 리소스와 실행 경계

이 변경은 Resources를 설치하지 않았다. 검토 당시 신규 선박 모델 9개, 항구 NPC 모델
2개와 Maharaka 모델·텍스처·사운드가 로컬에 누락되어 있었다. 각 작업의 Drive pack을
설치해야 해당 표시를 사용할 수 있다. Server bootstrap 게시와 실행 중 Server 재시작은
별도이며, 초상·선박·마하라카 최종 화면은 사용자가 확인한다.

## PR #469 추가 통합

`5d0ca1f98`의 에스더 판정과 플레이어 반복 타격을 위 병합 기준선에 결합했다. C++는
자동 병합되었으며 local Guide·피격 경로의 기존 변경을 유지했다. 유일한 명시적 충돌은
`Gameplay.bootstrap`의 Valtan presentation generation으로, 양쪽 hash 중 하나를 선택하지
않고 공식 `Publish-GameplayBalance.ps1 -Mode Publish`로 현재 결합된 정본에서 재생성했다.
146개 artifact의 generation은 `0a7c328f0e8a257119a9715035a01238f31b406dd6308a261b52e71ff1e398da`다.

- Kouku 정본 검증 114 patterns/577 stages, Valtan projection·template parity·alignment PASS.
- 공식 Gameplay Publish 성공: 109686 rows, hit-shape coverage 95/95.
- Artist·DimensionMaster·GuardianKnight·LanceMaster·Warlord animevents/hitshapes
  두 생성기의 `--check` 모두 unchanged.
- incoming 변경 JSON 8개, Shared project/filter XML 2개 parse 성공.
- Kouku authoring·published 문서와 effect V2 정본은 local `5fc2f940a`와 변경 0이다.
  이번 publish가 변경한 파일은 Gameplay.bootstrap과 새 Valtan generation manifest뿐이다.
- C++/FXC Product 빌드는 후속 기능 통합 검증에 남겨 두었다. Client/UI/Server는 실행하지 않았다.
