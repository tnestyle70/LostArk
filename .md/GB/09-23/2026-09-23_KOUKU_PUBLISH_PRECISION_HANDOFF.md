# 밸런스 작업자 전달 내용

## G01. 공용 게시 오류 수정

쿠크 본 콜라이더와 오브젝트 갈고리의 생성 위치를 소수점 9자리로 통일했다.
같은 입력을 계산한 뒤 실수 끝자리 차이로 `projected Product is stale`가 발생하는 문제를
수정한 것이다. 실제 입력 변경의 stale 검사와 finite/bounds 검증은 유지한다.
원본 Composition, 무력화 수치, bootstrap schema를 변경한 작업은 아니다.

전달할 코드는 `Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py`와
`world_object_collider.py`이며, 대응 animation blend/object collider 테스트도 함께 포함한다.
파일 전체 복사보다 통합 커밋 또는 diff 병합을 사용해 다른 세션의 쿠크 기능 변경을 보존한다.
이 작업에서는 사용자 지시에 따라 커밋·푸시·PR을 만들지 않았다.

## G02. 통합 뒤 게시 순서

다른 세션의 코드와 최신 저장 JSON을 합친 뒤 그 저장본에서 다음 명령을 실행한다.
새 JSON 필드를 추가했다면 Python publisher와 실제 Server/Client 소비 코드도 함께 합쳐야 한다.

```powershell
python -B Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py --repository-root . --mode publish
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Publish
```

생성된 `Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json`,
`Data/Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json`과 필요한
`Server/Bin/DataFiles/Gameplay` 게시 파일을 통합 코드·저작 JSON과 함께 전달한다.
일반 밸런스 변경은 게시 뒤 Server 재시작과 실제 무력화 적용 확인이 필요하다.

격리 worktree에도 실제 `Client/Bin/Resources`가 있어야 한다. 이 폴더는 Git으로 전달되지 않는다.
모델이 없으면 94개 패턴이 탈락해 2개만 남는 현상이 재현된다. 이는 원본이 2개만 선언했다는
뜻이 아니다. 후보의 `patternInventory.patterns[].unavailableReason`과 모델 경로를 먼저 확인한다.
의도하지 않은 패턴 감소가 있으면 그 후보로 기존 게시본을 교체하지 않는다.

## G03. 무력화 변경에서 따로 확인할 부분

후속 요청에 따라 Gameplay publisher의 `staggerDamage=10`, `partDamage=100` 고정 검사를
0..1,000,000 정수 범위 검사로 수정했다. 비공격 스킬은 두 값을 0으로 유지하며 counter의
기존 슬롯·0/1 capability 정책은 바꾸지 않았다. 튜닝 값은 Server가 승인한 적중마다 적용된다.
작업자는 수치를 바꾼 뒤 `Update-BalanceProvenanceReceipt.ps1`로 receipt를 동기화하고 게시한다.
`Tools/GameplayPipeline/Publish-GameplayBalance.ps1`와 대응
`Test-GameplayWorldTrackNumbers.ps1`도 통합 커밋에 포함해야 한다.

기존 `staggerDamage` 숫자만 바꾸면 bootstrap 버전을 올릴 필요가 없다. 실제 새 열이나 의미를
추가했다면 publisher·Shared 버전·Server reader·해당 Client admission과 생성물을 같은 세대로
맞춰야 한다. v36 reader에 v34 게시본을 억지로 읽히거나 header 숫자만 고치지 않는다.

## G04. 이번 작업의 적용 경계

Desktop에는 수정 코드와 테스트를 기존 미커밋 변경 위에 병합했다. 사용자의 후속 요청에 따라
최신 Composition revision2222의 저작119개·실행113개를 그대로 유지한 생성 JSON을 재게시했다.
main 기준96개 패턴 및 임시 튜닝27/350을 사용한 격리 게시본은 검증용이므로 Desktop에 복사하지
않는다. 이후 다른 세션에서 원본을 더 바꾸면 그 최종 저장본으로 G02 순서를 다시 실행한다.
실제 게시·바이너리 호환 검사 결과는 같은 주제 RESULT를 기준으로 한다. 이번 세션에서 빌드,
셰이더 컴파일, Client/UI 실행, 커밋·푸시는 하지 않았다.
