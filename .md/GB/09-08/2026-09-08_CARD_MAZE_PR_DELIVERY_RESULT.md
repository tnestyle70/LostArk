# 카드미로 PR 인계 결과

## 기준과 포함 범위

2026-09-08 10:28:29 KST pull/fast-forward `f92178f0` 이후 변경.
작업 브랜치는 `codex/kouku-card-maze-0908`, PR 병합 대상은 `main`이다.
main 직접 push나 PR 자동 병합은 하지 않는다.

오늘 카드미로의 Client/Shared/Server 수직 슬라이스, 네비게이션 저작 자료,
MapTool/Camera/WorldSequence, 문양 이펙트 정의, 몬스터 catalog/profile,
publisher와 정상 publish 결과 및 단계별 PLAN/RESULT를 함께 전달한다.
ClassSelect MapCatalog 변경은 앞쪽의 중복 sourceLights/lights 키 제거이며
동일 Area 뒤쪽의 정식 경로는 유지된다. 조명 기능 삭제가 아니다.

앞선 단계 RESULT는 이력이다. 현재 상태는 ENTRY_FIX_RESULT, SOLO_TEST_RESULT,
FLOOR_MARKERS_RESULT와 실제 코드가 우선한다. 초반의 protocol69, G 획득,
문양당 3마리 동시 생성, 출구 효과 미등록 설명은 현재 동작이 아니다.
현재 protocol70, Q 중앙 타격, 활성 문양당 1마리 보충 생성, 등록된 바닥 문양을 사용한다.
다인에서는 최초 망원경 담당에게 문양을 주지 않으며 Debug 방에 실제 1명일 때만 겸임한다.

## PR 준비 중 실제 수정

Composition Validate가 `Camera Shot source.shots[24] ... unknown=['displayName']`로
실패했다. 기존 Map camera owner가 지원하는 선택 displayName을 Composition에서도
1~128 UTF-8 byte 표시 문자열로 검사하도록 수정했다. 새 회귀 검사에서 한국어 수용,
null/숫자/빈 문자열/128byte 초과/제어 문자 거부를 확인했다.
Composition을 다시 publish하여 쿠크 camera/worldsequence 변경으로 오래된 receipt도 갱신했다.

## 검증 상태

- 기존 Product Debug 빌드 증거: `out/BuildPipeline/runs/20260908T103517881Z-debug-product.json`.
  Engine/Shared/Server/Client PASS. 이번 PR 준비에서 C++ 추가 변경은 하지 않았다.
- Server Debug `--card-maze-contract-test` 재실행: exit 0, failures 0.
- NetworkProtocolHarness Debug `--mario-controls-only` 재실행: exit 0.
- `python -B -m unittest Tools.CompositionPipeline.test_composition_pipeline.CompositionPipelineTests.test_camera_display_name_accepts_korean_and_rejects_invalid_text`: OK.
- `Tools/CompositionPipeline/Publish-Compositions.ps1 -Mode Publish`: 성공.
- 변경 JSON 30개 및 Client 프로젝트 XML/filter parse 성공. `git diff --check` 성공.
- 독립 읽기 전용 검토에서 ClassSelect 중복 키 제거 확인 및 stale receipt 지적 확인 후 재publish.
- Client/UI 실행·조작·캡처와 실제 4인 플레이는 수행하지 않았다. 마지막 프로세스 확인 시 Client/Server 없음.

## 별도 전달과 미완료

물리 Resources 및 Drive 안내 정본은 [리소스 인계](2026-09-08_CARD_MAZE_RESOURCE_HANDOFF.md).
신규 카드 병사 4폴더 28파일, 오늘 신규 참조한 기존 세토 12파일,
기존 문양 DDS 4개를 분리 안내했다. Drive 업로드와 팀원 수령은 확인하지 않았다.
Resources, 개인 `.vcxproj.user`, `.claude`, `.codex`, 오래된 복사 스크립트/리스트,
개인 dev/temp와 navpaint 백업, 빌드·중간 산출물은 PR에서 제외한다.

일반 캐릭터 망치 애니메이션과 망원경 상자의 시각 모델은 추가하지 않았다.
사용자 화면 확인 및 다인 네트워크 확인은 남아 있다. 최종 복귀 좌표는 기존 2관문이며
`cardmaze.return`에서 추후 튜닝한다. 팀 서버와 Client는 같은 protocol70 빌드가 필요하다.
팀 LAN endpoint는 변경하지 않았다. 이 PC의 개인 F5 로컬 서버 전환은 사용자 결정 대기이며 PR 범위가 아니다.
