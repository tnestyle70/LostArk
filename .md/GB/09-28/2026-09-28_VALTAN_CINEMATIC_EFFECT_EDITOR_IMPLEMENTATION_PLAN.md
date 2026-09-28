# 발탄 등장 연출 Effect Editor 구현 계획

## G00. 현재 연결과 요청 범위

사용자가 지정한 대상은 루가루가 아닌 발탄 본체 등장이다. 현재 `CINEMATIC EFFECTS > 진입 / Entrance`는 공유 Effect body별 Open만 제공한다. 실제 `entrance`와 `entrance.colorless`의 WorldSequence가 24,708ms 동안 모델 동작과 14회 Effect(13개 공유 문서, 84개 고유 Element/88개 발생)를 소유한다. 미연결 actor64 트랙은 활성화하지 않는다.

## G01. 연출 전체 Open과 Element 목록

`Effect_Tool_ValtanWorld.cpp`의 Entrance 그룹에서 연출 Open Editor를 제공한다. 기존 Movie 편집 callback 계약을 `valtan.entrance`에도 연결한다. `Effect_Tool_Workspace.cpp`는 전체 Effect/Element 목록을 CPU 문서 캐시로 표시하고 선택한 Element의 원래 저작 body를 연다. 같은 파일의 반복 사용을 숨기지 않는다. 기존 Save Changes의 stable ID, freshness/atomic 저장과 미저장 전환 보호를 유지한다.

## G02. 실제 애니메이션과 Solo 시계

`Level_ValtanArena`가 기존 WorldSequencePlayer를 사용하는 편집 세션을 소유하며 MainApp은 기존 Movie callbacks를 해당 Level로 전달한다. Play/Pause/Seek/Stop/속도와 Element Solo는 같은 source clock, 원래 모델·본·TRS·가시성을 사용한다. WorldSequencePlayer의 preview override는 기존 Effect renderer/service를 통해 stage/commit하고 실제 Product 데이터나 Server simulation을 변경하지 않는다. 다른 Effect의 draw만 제외하고 모델 애니메이션을 유지한다.

## G03. 피자 붉은 검기 Solo

피자/모아치기 붉은 검기의 실제 문서·element와 실패 경로를 확인해 같은 조건의 정상 Element를 막는 공통 원인을 교정한다. 미지원 shader/material 입장 검증을 해제하거나 임의 element 삭제로 우회하지 않는다.

## G04. 검증과 인계

현재 파일 인코딩과 기존 미커밋 변경을 보존한다. 기존 C++ 파일 확장이므로 신규 project/filter 등록은 없다. 실제 Codec/preview 소비 계약과 실패 시 이전 상태 보존을 검사하고, 변경 JSON/XML parse 및 git diff --check를 확인한다. 다른 build와 겹치지 않게 정상 Debug Product build를 실행한다. Client/UI 실행·화면 판정은 사용자가 수행하며 RESULT에서 자동 검증과 분리한다.
