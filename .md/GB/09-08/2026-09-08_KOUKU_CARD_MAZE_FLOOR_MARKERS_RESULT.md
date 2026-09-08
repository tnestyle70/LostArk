# 카드미로 발밑·병사·탈출구 문양 적용 결과

후속 사용자 요청으로 Debug 1인 테스트를 별도 허용했다. 현재 1인 절차는 `2026-09-08_KOUKU_CARD_MAZE_SOLO_TEST_RESULT.md`를 따른다. 아래 최소 2명/solo 금지는 이 단계 당시 상태이며 다인 역할 분리는 그대로 유지한다.

2026-09-08. 현재 작업 브랜치와 기존 미커밋 작업을 보존했다. 자동 stage/commit/push는 하지 않았다.

## 구현 상태

- 문양이 배정된 HUNTER 플레이어 발밑과 해당 문양 병사 발밑에 같은 문양·색의 Decal을 표시한다. 플레이어가 병사를 찾아가는 규칙은 변경하지 않는다. Client는 몬스터 이동/생성/피격 권위를 갖지 않는다.
- 3스택 개인 출구는 같은 Decal을 서버 출구 위치에 고정한다. 플레이어·병사는 표현 객체의 위치만 따라가며 모델 scale/yaw는 상속하지 않는다.
- 망원경 담당은 문양 표시와 목표를 받지 않는다. 기존 Debug solo 겸임 예외를 제거했으므로 시작에는 담당 외에 최소 한 명의 생존 플레이어가 필요하다. 별도 1인 사냥꾼 테스트 모드는 추가하지 않았다.
- 사망/탈출/이동 중 플레이어 표시, despawn 병사 표시, 취소된 출구, 미로 종료/Level Reset의 표시를 정리한다. 기존 룰렛 머리 위 표시는 유지하고 카드미로의 머리 위 fallback만 제거했다.
- 전용 leaf 4개, mark/exit group 8개와 Independent 목록·프로젝트 None/96.DataFiles 등록. 기존 춤 연출은 수정하지 않았다.
- group 기본 미리보기는 1000ms, 제품은 기존 occurrence duration=0 계약으로 상태가 유지되는 동안 지속한다. 로드/재생 실패는 해당 표시를 격리하며 gameplay를 바꾸지 않는다.

## 자동 검증

- Product Debug Engine → Shared → Server → Client 컴파일·링크·정상 배포 성공. `out/BuildPipeline/runs/20260908T100241947Z-debug-product.json`, missingRuntimeInputs 없음. 기존 C4819/C4828/LNK4099 경고는 남아 있다.
- `Server/Bin/Debug/Server.exe --card-maze-contract-test`: failures 0. 단독 담당에게 문양을 주지 않는 조건, 네 명의 세 문양 배정, 자기 문양 한 방 처치, 접촉 초기화, 관전/완료 규칙 포함.
- 기존 EffectV2 binding pipeline의 `_resolve_group`로 새 8 group/4 leaf, schema·참조·실제 DDS 검증 성공. Independent JSON 및 project/filter XML parse 성공.
- 초기 group duration=0은 기존 검증기의 무한 자연 길이 금지에 걸렸으므로 group 기본 길이를 1000ms로 수정하고 재검사했다. 실제 Play_Group의 Occurrence_ChildStopSeconds가 제품 duration=0을 적용하므로 1초 후 종료되지 않는다.
- 독립 read-only 검토에서 snapshot consumer, pivot, 상태 수명, owner 규칙과 위 duration 재정의 확인. 최종 actionable defect 없음.
- `git diff --check` 성공. 변경한 C++ 인코딩은 기존 UTF-8을 유지했다.

## 사용자 실행 확인 — 아직 미검증

에이전트는 Client/UI를 실행·조작하거나 캡처하지 않았다. 화면의 실제 크기, 시인성, 바닥 투영 높이, 4인 동시 시각 결과는 PASS 처리하지 않는다.

1. 팀 서버 PC에서 이번 Server 빌드를 재시작한다. 이 PC는 LAN client로 판정됐으며 `192.168.0.4:7777` 설정을 유지했다. 마지막 점검 때 서버는 not-listening이었다.
2. 이 PC는 Visual Studio Client 프로젝트를 Ctrl+F5로 실행하고 Lobby → KoukuSaydon → F1의 카드미로 진입 경로를 사용한다. 최소 2명, 정상 파티 검증은 4명으로 진행한다.
3. 한 명이 상자를 망치로 친다. 담당에게 문양이 없고 나머지에게 문양이 배정되는지 본다. 같은 문양 병사 바닥을 확인하고 자기 문양 병사 3마리를 잡아 같은 문양 출구가 생기는지 확인한다.
4. 이동 시 발밑 부착, 병사 처치 즉시 제거, 접촉 초기화 시 출구 제거, 탈출·종료 후 잔상 정리를 확인한다.
5. F1 → Open Effect Tool v2에서 `cardmaze.mark`/`cardmaze.exit` 그룹과 `cardmaze.symbol` leaf를 찾는다. 네 문양 공통 기본 투영 크기는 2m이며 각 symbol leaf의 Decal 크기·색을 조정한다. 실제 runtime 수정 반영은 Client 재시작으로 확인한다.

## 팀 리소스 인계

새 binary payload는 없다. 기존 Resources-relative 입력은 `Effect/KoukuSaydon/Textures/FX_TEX_NOMIPMAP_00/` 아래 `fx_l_symbol_47.dds`, `fx_l_symbol_47_1.dds`, `fx_l_symbol_47_2.dds`, `fx_l_symbol_47_3.dds`다. 하트·스페이드·클럽·다이아몬드 순서이며 base/mask가 같은 텍스처를 소비한다.

물리 폴더: `Client/Bin/Resources/Effect/KoukuSaydon/Textures/FX_TEX_NOMIPMAP_00`. 이 PC에서는 실물 확인 완료. 팀 Drive 업로드/다른 PC 보유 여부는 확인하지 않았다. 이 네 파일이 이미 배포돼 있다면 추가 binary 전달 없이 소스 Data/Effects 변경으로 연결된다.
