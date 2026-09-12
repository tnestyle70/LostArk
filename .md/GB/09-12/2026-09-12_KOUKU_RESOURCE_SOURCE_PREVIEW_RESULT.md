# 쿠크 독립 Effect Resource의 원본 모델 시계 연결 결과

## G00. 실제 원인과 수정 범위

사용자 첨부 오류의 `preview.resource.occurrence`는 단일 Resource Preview였다.
MainApp이 Animation 없는 Stage를 만들었고 PresentationPlayer가 그 빈 Stage에서 source
bone pose를 찾으므로 첫 외부 fixed-step sample부터 Effect handle이 제거됐다.
쇼타임 총구발사 등 여섯 문서는 이미 원본 `sourceModelPreview`를 갖고 있었다.
shader나 DDS 누락으로 발생한 오류가 아니다.

PresentationPlayer는 단일 V1 Effect/Element Resource의 sourceModelPreview를 기존
`CEffectCompositionModelPreview::Select_SourceEffect`로 읽고, 기존 `Begin_BundlePreview`의
단일 member에 actor·Animation·Effect를 함께 준비한다. 실제 CNpc/CModel과 플레이어 위치를
사용하며 공개 preview ID와 0ms 시계는 유지한다. MainApp은 이 actor 소유 Preview에
World Preview를 중복 시작하지 않는다. 새로운 모델 런타임이나 Server 경로는 없다.

Animation 원본의 sourceStart·rate·play 구간은 유지하고, Effect occurrence의 수명 안에서
마지막 pose를 입자 tail까지 유지한다. 저장된 모델 metadata가 없는 독립 Resource는
현재 선택 모델의 실제 bone pose만 한 번 캡처한다. 이 동작은 `preview.kouku.resource`에
한정하며 Product/일반 Pattern의 Animation history 누락은 계속 명시적 오류다.

## G01. 원본 metadata 누락 네 문서 보완

| Effect | 원본 Action/Stage | 원본 모델 클립 |
|---|---|---|
| G1 내려치기C 4219877 | stage0 | rpct00_att_battle_1_01, 2000ms |
| G1 불뿜기 4219801 | stage0~1 | rpct00_att_battle_14_01/02, 1667+3167ms |
| G2 거미카운터 stage1 | 4219776 stage1 | rpcz00_att_battle_6_02, 1000ms |
| G2 거미카운터 stage2 | 4219776 stage2 | rpcz00_att_battle_6_03, 1167ms |

G1은 원본 MN_RPCT_05 action-effects, G2는 MN_RPCZ_00 action-effects의 선택 Stage와 clip
길이에서 생성했다. 여러 authored 거미카운터 occurrence도 같은 source Stage/sourceStart0/rate1
임을 확인했다. 임의로 첫 Pattern을 정답으로 선택하지 않는다.
`build_kouku_gate1_full_restore.py`와 `build_kouku_spider_counter_restore.py`도 같은 metadata를
만들도록 갱신했다. 네 JSON은 sourceModelPreview만 추가했고 기존 field를 전부 보존했다.

## G02. 실행한 검증

현재 authored Kouku 문서에서 외부 FOLLOW bone을 사용하는 항목은 11문서/138요소다.
보완 후 모두 sourceModelPreview를 가진다. 해당 문서의 실제 Codec과 설치 WModel을 읽고
원본 CModel bone sample 코드를 사용한 수치 검사에서 11/11 통과, 실패 0이다.

- 합계 4,632 fixed-step source sample과 실제 CPU 입자 재생을 검사했다.
- 0ms, 첫 1/60초, 각 clip 끝 직전·정확끝·끝 직후·1초 뒤, Effect 마지막 sample과 0ms rewind를 검사했다.
- rewind 전후 source matrix가 일치했고 모든 bone/world matrix가 finite였다.
- 총구발사는 MN_RPCT_05.wmodel, preScale0.017, Source In1097ms, 11요소, 660 sample, peak144 CPU rows로 통과했다.
- 실제 PresentationPlayer/MainApp CPP와 기존 EffectCompositionModelPreview CPP를 out으로 컴파일했다.
- 두 Python generator AST parse, 네 JSON의 기존 field 보존, 변경 범위 git diff --check를 통과했다.

처음 이전 out의 Codec/Playback obj를 재사용한 임시 probe는 0xc0000005로 실패했다.
현재 public header에 맞춰 모든 CPU 의존 객체를 out에 다시 컴파일한 뒤 동일 실제 입력 검사가
exit0으로 완료됐다. 제품 실패와 구분한다. 기존 SDK header 인코딩 경고와 DirectXTK PDB 경고는
이 작업에서 SDK를 수정하지 않고 보존했다.

증거는 `out/KoukuResourceSourcePreview20260912/`의 `result.json`, `source_inventory.json`,
`metadata_installation.json`, `actual_model_probe.cpp`, `production_source_anchors.cpp`, compile/link 로그다.
source anchor 함수와 TargetPivot 함수는 현재 제품 소스에서 기계적으로 추출해 검사했으며 실제
PresentationPlayer 전체 CPP 컴파일도 별도로 수행했다. 임시 probe는 Client 진입점을 호출하지 않는다.

## G03. 사용자 화면 확인과 적용 경계

Client/UI 실행·조작·캡처와 GPU visual fidelity 판정은 수행하지 않았다.
최종 Client 링크와 배포 상태는 상위 통합 작업이 기록한다. 새 Client에서 Sequencer의
Effect Resource → 쇼타임 → 총구발사를 Preview하고 Play/Seek/Restart 및 잔상 종료를
사용자가 확인해야 한다. 실행 중이던 구버전 exe나 이미 읽은 Effect snapshot에 대한 적용 완료로
기록하지 않는다. 이번 변경은 Git 관리 코드와 JSON이며 새로운 Resources 바이너리는 없다.

## 최종 통합 확인

루트의 최종 Client 컴파일/링크·Server 빌드와 revision 352 공식 Kouku 4-domain 게시를 완료했다. 노란 3종은 Composition과 Client project에 등록했고 검증한 Decal CSO를 기본 Debug 경로에 반영했다. 상세 로그·중단 범위·사용자 실행 경로는 [통합 결과](2026-09-12_KOUKU_PARENT_PATTERN_TIMELINE_IMPLEMENTATION_RESULT.md#G03-배포와-사용자-확인)를 따른다. 사용자 화면 확인은 대기다.

