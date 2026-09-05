# 2026-09-05 쿠크 모델별 패턴·Append·Sequence 편집 구현 계획

작업 브랜치는 사용자 지정 `koukysaydon-pattern3`다. 이전 PR #317 위에서 작업하고,
검증 후 PR 병합과 원본 `C:/Users/user/Desktop/LostArk`의 main pull까지 수행한다.

## G00. 실패 원인과 모델 소유권

사용자 화면의 `MN_RPCT_05` Action `4219811`은 세이튼 동작이다. 기존 Resources는
모든 프로필을 보여 주지만 Append를 `MN_RPCZ_00`만 허용해서 해당 버튼이 비활성화된다.
또한 실제 reference Action 0을 미선택/RAW로 오인하는 UI와 validator 오류가 있다.

상위 발탄/쿠크세이튼 구분은 유지하고 쿠크세이튼 패턴을 실제 모델별로 나눈다.
쿠크는 RPCZ_00, 세이튼은 RPCT_05, 대형 세이튼은 RPCT_06이며 RPCT_07은 RPCT_05의
동작 프로필 별칭이다. 패턴의 `actorProfileId`가 해당 물리 모델을 저장한다.

Composition v2는 모델 owner를 명시한다. v1은 homogeneous occurrence에서 owner를 유도하고
빈 기존 Gate1 패턴은 쿠크로 읽는다. 서로 다른 모델이 섞인 패턴은 오류 항목으로 보존한다.
Append는 선택한 모델의 패턴에 연결하고, 해당 모델에 패턴이 없으면 같은 candidate에서 새 패턴을 만든다.
실패한 Append는 원래 draft를 보존한다. PRODUCT 배포의 실제 Gate1 boss 제한은 유지한다.

## G01. Sequence 추가·선택·삭제·Save

`KoukuSaydonActionWorkbench`의 기존 candidate/validate/commit과 Composition `Save_Atomic`을 사용한다.
추가한 Action의 Stage와 Animation은 즉시 Sequence에 표시하고 화면 맞춤을 요청한다.
순차 clip은 같은 Animation 행에 놓고 겹친 구간만 추가 행을 사용한다.
빈 공간의 사각 드래그와 Ctrl 선택으로 stable Stage/occurrence ID를 선택하고 Delete 키 또는
Delete Selected로 하나의 candidate에서 삭제한다. Stage를 지우면 포함된 clip도 같이 지운다.
Sequencer에도 Save와 dirty/status를 표시한다. 저장·재로드와 CAS 실패 보존을 실제 native 메서드로 확인한다.

## G02. 대형 동작 미리보기 배율

추출된 Actor/Component scale 근거가 있으면 해당 원본값을 우선한다. 근거를 찾지 못한 대형 동작은
사용자가 요청한 100배를 미리보기 표시 배율로 사용하며 원본 Unreal 값으로 표기하지 않는다.
기존 CModel/preview actor의 Transform을 사용하고 일반 동작 전환 시 기본 배율로 되돌린다.
실제 적용할 배율과 근거·검증은 RESULT에 기록한다.

## G03. 검증과 반영

기존 native harness에 focused entry만 추가해 screenshot Action, Action 0, alias, 모델별 패턴,
batch delete, Save/reload, 잘못된 owner·선택·CAS 실패 보존을 검사한다. 새 harness는 만들지 않는다.
기존 테스트 프로젝트에 필요한 실제 product CPP 등록과 XML parse를 포함한다.
변경한 C++ 최소 컴파일, 해당 publisher 검사, JSON/XML parse와 `git diff --check`를 수행한다.
Client/UI의 실행·화면 확인은 사용자가 한다. 에이전트는 PR/merge/pull과 실행 준비를 완료하고
현재 프로세스 상태와 사용자가 누를 경로를 보고한다.
