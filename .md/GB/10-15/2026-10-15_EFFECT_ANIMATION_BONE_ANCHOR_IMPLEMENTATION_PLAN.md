# Effect Animation Library와 Bone Anchor 구현 계획

기준: 2026-10-15, `703294d542dedb932dc440863628ba037ffbafa2`,
`codex/effect-animation-bone-anchor-20261015`. 사용자 요청에 따라 main을 fast-forward pull했고 시작 작업 트리는 clean이었다.

## G00. 현재 호출 경로와 변경 목표

Action Workbench의 Composition Animation Library는 이미 설치된 28개 target의 clip을 읽는다.
반면 Effect Tool의 `CEffectAuthoringSequencer` resource 목록은 현재 로드된 모델의 clip만 읽는다.
V1 `Render_ModelViewWindow`는 sequencer 분기에서 반환하므로 이전 Element Follow UI에 도달하지 않는다.
Kouku Collider는 bone/boneTarget/boneRotation 저장 및 게시된 bone track을 소비하지만,
현재 본 제공자가 없는 상태에서 대상 모델을 로드할 수 있는 직접 진입점이 없다.

기존 Character Preview, CModel, AnimationTargetService와 각 owner의 저장 경로를 확장한다.
캐릭터·보스·World Object를 분류하여 애니메이션을 고르고, 실제 모델의 본을 선택한 뒤
이펙트 또는 Collider를 부착하는 편집 흐름을 완성한다.

## G01. CompositionAnimationResource와 CharacterPreviewPanel

`CompositionAnimationResource.h`, `Animation_Tool_DocumentIo.cpp`의 설치 WModel inventory를 공용으로
사용하고 `CharacterPreviewPanel.h/.cpp`가 같은 stable target의 preview를 준비한다.
World Object는 저장된 World Sequence의 object 이름, stable ID, 실제 model asset ID를 사용한다.
괴기스러운 인형과 공놀이시간의 분열 공을 포함하며 Character/Boss/World 분류를 보존한다.
목록을 탐색하는 것만으로 runtime target이나 저작 문서를 변경하지 않는다.
명시적인 모델 로드·클립 추가에서 호환성과 현재 draft를 검사하고 실패하면 기존 상태를 보존한다.

## G02. EffectAuthoringSequencer와 Effect Tool

V1/V2가 공유하는 `Render_ModelView` 및 Composition Resources에 같은 animation library를 연결한다.
로드한 모델의 clip을 기존 animation row에 넣어 effect row와 동일한 clock으로 재생·seek한다.
`Render_AnchorChoice`는 실제 모델의 bone/socket을 검색·선택하고 기존 row anchor 저장 계약을 사용한다.
V1의 selected-element follow 편집은 sequencer 분기에서도 노출한다. 문서 내부 부착과
독립 sequence occurrence의 부착은 기존 소유권을 유지하며 각 저장 버튼의 범위를 명시한다.
미적용 Detail draft, 미저장 source, 잘못된 bone은 기존 stage/validate/commit 경계로 거절한다.

## G03. Boss Collider Box Detail

`KoukuSaydonActionWorkbench`의 Collider 상세와 기존 MainApp preview route를 연결해 선택한
Pattern actor 모델을 직접 준비한다. BODY/WEAPON의 실제 본 목록과 검색을 제공하고
bone target·회전·부모 변경도 preview 갱신에 포함한다.
대형세이튼 잡기는 실제 설치 모델의 왼손 bone을 기준으로 저장·publisher sampling·Server 소비를
확인한다. Valtan의 별도 hit-anchor 계약도 대조하되, 이번 대형세이튼 편집을 위해 별도 Valtan gameplay schema를 신설하지 않는다.
보스 root로의 조용한 대체, Client 단독 피해 판정, 전체 패턴 데이터 덮어쓰기는 추가하지 않는다.

## G04. 검증과 전달

신규 C++ 파일이 필요하면 물리 위치에 맞는 `.vcxproj`와 `.vcxproj.filters` 항목만 추가한다.
기존 C++ 인코딩과 줄바꿈을 보존한다. Product Debug 증분 Build로 변경 코드와 소비자 링크를 확인한다.
목록의 모델·clip·stable ID, 실제 WModel 본, 저장/로드와 잘못된 입력 보존을 해당 기존 검사로 확인한다.
변경 XML/JSON parse와 `git diff --check`를 실행하고 RESULT에는 실행한 검증만 기록한다.
Client/UI는 자율 실행하지 않는다. 사용자에게 모델 선택→클립 추가→본 선택→저장→재생 경로를 전달하고
실제 화면 판정은 미확인으로 구분한다. 네트워크 endpoint와 팀장 렌더링 옵션은 이번 변경 대상이 아니다.
