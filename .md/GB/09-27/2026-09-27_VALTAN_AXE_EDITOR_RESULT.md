# 발탄 F1 손도끼 변환 결과

기준일: 2026-09-27. 브랜치 `codex/bern-dragon-camera-performance`의 작업 중 변경이다.

## G01. 현재 구현

F1 → Valtan Arena의 Start Position / Before Entrance / Arena Start 묶음 아래에
`Axe Transform`을 추가했다. 정상/유령 발탄을 선택하고 XYZ position / rotation을 조절한다.
편집된 값은 기존 `CValtan -> CPart_Equipment -> b_wp_r_01`에 다음 객체 update부터 반영된다.
숨기거나 저장한 뒤에도 같은 프로세스의 해당 presentation archetype에 유지된다.

위치는 정규화한 손본 축 기준 미터이며 boss presentationScale 적용 전이다. 본 행렬에 이미
포함된 model preScale로 translation을 다시 축소하지 않는다. 회전은 pitch/yaw/roll 도 단위다.
위치 각 축 ±10m, 회전 ±360도 이내의 유한값만 허용한다. 모든 보정이 0이면 기존 부착
행렬을 그대로 사용한다. 다른 캐릭터 장비에는 zero default가 유지된다.

`BossCatalog.json` v8에 optional `weaponSocketTransform`을 추가했다. 적용 가능한 owner는
`BOSS_VALTAN`, `BOSS_VALTAN_GHOST`이며 필드는 `positionMeters`, `rotationDegrees`다.
필드가 없는 기존 JSON은 zero default로 호환된다. 이번 작업은 사용자가 요청한 편집 기능을
추가했으며 현재 저장된 실제 도끼의 위치·회전 수치는 변경하지 않았다.

## G02. Save와 Publish

Save는 최신 `Data/Actors/BossCatalog.json`을 읽고 stable archetype ID와 변경한 축으로
병합한다. 다른 축·다른 boss·재질 등 무관한 최신 필드를 보존하며 동일 축의 실제 충돌은
draft와 디스크를 보존한 채 거부한다. 기존 Valtan canonical byte-range lock과 recovery
journal 경계를 사용한다. 임시 JSON 재parse, 백업, 교체 직전 최신 bytes 확인 후 원자 교체한다.
백업은 같은 폴더의 `BossCatalog.json.axe.previous.bak`이며 Git 제외 `*.bak`에 해당한다.

`Save + Publish`는 저장 뒤 기존 Balance Tool의 dirty 보존 Reload, exact source revision,
비동기 `Publish_ServerRuntimeSet`을 사용한다. 기존 `COMBAT_VISUAL` generation artifact가
BossCatalog 전체를 Server의 pinned presentation generation에 보존한다. 새로운 publisher나
두 번째 모델 런타임을 만들지 않았다. 현재 프로세스의 시각 보정, Data 저장, Server 디스크
게시, 실행 중 Server 재시작과 Client 재진입은 별도 단계다. 도끼 시각 보정은 encounter의
Server 피해·충돌 판정을 변경하지 않는다.

최초 Load 실패는 한 번만 읽고 사용자의 Reload saved를 기다리므로 오류 상태에서 매 프레임
전체 catalog를 재parse하지 않는다. Reload saved와 Reset axes는 명시적 사용자 버튼이다.

## G03. 실행한 검증

- `test_boss_weapon_socket_transform.py`: PASS. 실제 publisher 함수와 현재 shared helper를
  PowerShell AST에서 읽어 14개 사례를 실행했다. legacy omission, 정상/유령 owner, 범위 끝값,
  owner 오류, weapon 부재, null/누락/추가 field, 벡터 길이, bool/string, 범위 초과를 검사한다.
- `test_boss_weapon_socket_save.py`: PASS. 실제 신규 editor의 Save/Load helper와 실제
  DataJson codec, 실제 transform validator를 추출해 MSVC C++20로 컴파일하고 Client/UI 없이
  격리 fixture에서 실행했다. optional 부재, 다른 축+다른 boss 변경 병합, 같은 축 충돌 거부,
  canonical lock 점유, 실제 파일 점유로 최종 교체 실패 시 파일/draft 보존, 점유 해제 후 재시도,
  nonfinite 거부와 임시 파일 정리를 검사했다. 출력은 `out/ValtanAxeEditorTests/`다.
- Client `.vcxproj` / `.filters` XML과 기존 BossCatalog JSON parse: PASS.
- 변경 파일 `git diff --check`: PASS.
- 기존 `test_boss_default_particles_contract.py`: 실행했으나 `Missing actual publisher function`
  오류로 실패. 해당 테스트가 `Assert-ExactProperties` 등의 현재
  `KoukuBootstrapRows.ps1` 위치를 읽지 못하는 기존 fixture 문제다. 이번 새 테스트는 실제
  publisher와 실제 shared helper를 함께 읽는다. 기존 테스트의 기대값은 바꾸지 않았다.

신규 CPP는 `Client/Default/Client.vcxproj`와 `.filters`의 MainApp 그룹에 등록했다. 기존
C++의 UTF-8/BOM 없음·CRLF 인코딩을 보존했고 신규 CPP는 UTF-8/BOM 없음이다.
전체 Debug/Release Product 빌드는 통합 작업자가 진행하며 그 결과는 통합 RESULT가 소유한다.
Client/UI 자동 실행과 화면 캡처는 하지 않았다. 실제 시각 결과와 버튼 조작은 사용자 확인 전이다.

## G04. 사용자 확인 경로와 리소스

발탄 아레나에서 F1 → Valtan Arena → Axe Transform을 연다. `Valtan`을 선택해 Position X와
Rotation Z를 조절한 뒤 손도끼의 이동·방향을 확인한다. `Ghost Valtan`은 유령 표현이 활성일 때
확인한다. Save 후 재진입과 Save + Publish 완료/실패 메시지를 구분해 확인한다. 게시 성공 뒤
Server를 재시작하고 재진입해야 새 pinned generation을 사용하는 상태까지 확인할 수 있다.

이 변경은 추가 리소스가 없으므로 GBResources에 추가할 파일도 없다.

## G05. 통합 Debug 컴파일에서 확인한 include 순서 수정

통합 `out/BernValtan20260927/debug-build.log`에서 신규 TU의 ImGui placement `operator new`
선언이 Engine Debug `new` macro와 충돌해 C2473/C2365가 발생했다. `imgui.h`를 첫 include로
옮겨 기존 `MainApp_SequenceViewer.cpp`와 같은 순서로 수정했다. 다른 CP949 헤더 경고를
회피하려고 기존 파일 인코딩을 바꾸지 않았다. 변경 후 실제 Save/Load 추출 fixture 재컴파일과
실행, diff check는 통과했다. 전체 Product 재빌드 결과는 통합 RESULT에 기록한다.
