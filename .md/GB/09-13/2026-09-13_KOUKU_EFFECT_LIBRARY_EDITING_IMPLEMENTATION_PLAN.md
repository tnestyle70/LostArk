# 쿠크·세이튼 전체 이펙트 탐색과 보스 기준 편집 구현 계획

## G00. 현재 요청과 확인한 원인

사용자는 트럼펫 장판과 선행 방사 레이저 두 그룹, 기존 `1관문 세이튼_불뿜기` 탐색·편집, 보스 앵커 미리보기와 지팡이 표시를 요청했다. 추가로 `세이튼_빙글빙글돌며카드던지기`의 실제 카드 모양·회전·비행을 패턴 이펙트에 넣도록 요청했다. 트럼펫의 폭발은 사용자가 알비온 네 방향 이펙트로 추가했으므로 새 그룹에 포함하지 않는다. 사용자는 현재 편집 중이며 애니메이션을 직접 추가한다. 실행 중 Composition의 기존 패턴·발생 항목을 덮어쓰지 않고 외부 리소스 등록은 기존 저장 계약의 append-only 확장만 사용한다.

현재 쿠크 V1 저작 문서는 635개, 제품 Catalog 항목은 113개, 별도 Tree 참조는 79개다. 불뿜기의 `effect.kouku.gate1.4219801.full.restore`는 Catalog와 P30에 연결됐지만 Tree에 없어서 숫자 ID 분류와 표시명으로 노출된다. 한글 이름 검색은 Tree 표시명만 조회한다. Catalog에 없는 저작 문서는 목록에서 제외된다.

불뿜기 문서에는 GATE1/MN_RPCT_05/boss.kakulsaydon.g1.saydon과 실제 14_01/14_02 애니메이션을 지정한 sourceModelPreview가 있다. 이 데이터를 재생 준비가 소비하는 경로와 실제 무기 조립을 확인한다. 목록 표시에서 전체 모델·이펙트 admission을 실행하지 않는다.

## G01. 하나의 분류 정본과 전체 저작 목록

`Data/Effects/EffectResourceTree.json`을 분류 정본으로 유지한다. 현재 Composition의 실제 패턴·리소스 연결, 기존 분류, 원본 Action·Sequence 사용 근거를 이용해 빠진 쿠크 문서를 관문/연출·패턴 아래 등록한다. 직접 연결이 없는 문서는 명시적인 라이브러리 패턴 분류를 사용하며 Server gameplay 패턴을 합성하지 않는다. 기존 사용자 분류와 다른 직업·World·Valtan 항목을 보존한다.

기존 Effect Tool의 Kouku 목록은 `Read_V1Inventory`의 작은 헤더 인벤토리를 사용해 Catalog 미등록 문서도 표시한다. 표시 이름과 분류 경로 모두 검색하며 누락·손상 문서는 오류를 해당 행에 남긴다. 새로고침 때만 인벤토리를 갱신한다. Open/Play는 선택한 정본 경로만 실제 codec으로 읽고 미저장 문서 전환과 실패 보존을 기존 경로에 맡긴다.

## G02. 실제 보스·앵커를 사용하는 Play All과 튜닝

전체/요소 재생은 기존 EffectAuthoringSequencer와 KoukuSaydonPresentationPlayer를 사용한다. 실제 sourceModelPreview 또는 명시된 Composition 사용 문맥에서 actor·gate·animation·anchor를 연결한다. 카메라/보스/월드 고정 입력을 구분하고 알 수 없는 본을 합성하거나 플레이어 위치로 조용히 대체하지 않는다. 소스 문서 전체의 구조와 source attachment를 유지하고 기존 detail/particleSystem 편집·저장 경로를 연결한다.

## G03. 세이튼·쿠크세이튼 지팡이

애니메이션·이펙트 미리보기가 실제로 조회하는 actor model description과 BossCatalog의 무기 설정을 대조한다. 이미 설치된 WP_MN_RPCT_05와 native material, body 손 본 연결을 기존 CModel/CNpc 경로에서 재사용한다. 파일별 인코딩과 진행 중 변경을 보존하고 다른 관문의 기존 망치·무기를 덮어쓰지 않는다.

## G04. 트럼펫 레이저·장판과 회전 카드

트럼펫은 원본 Action 4219807의 Sk_06_8 레이저와 Sk_06_9 카드 문양 장판을 각각 독립 V1 그룹으로 만든다. 레이저 원본 notify의 0도·45도 두 호출과 내부 네 방향 방사를 구분하며 단위·기본 높이의 출처를 기록한다. Sk_06_2 폭발은 포함하지 않는다. 새 그룹은 현재 `세이튼_트럼펫장판소환` 트리 분류와 전역 presentationResources에 등록한다.

회전 카드는 Action 4219819의 직접 particle 호출뿐 아니라 Effect notify가 연결한 Projectile 문서도 확인한다. 실제 카드 carrier·문양 재질과 발사 시간·방향·이동·회전의 원본 값을 기존 sourceTransformTrack/particle 경로에 연결한다. 원본에 없는 추적·명중·damage는 이 저작용 시각 그룹에 합성하지 않는다. `세이튼_빙글빙글돌며카드던지기` 분류 아래 등록하고 원본 애니메이션은 sourceModelPreview의 재생 문맥으로만 사용한다.

각 생성기는 새 후보를 out에 작성하고 기존 Resources-relative mesh/texture를 재사용한다. 현재 편집 중인 정본을 재확인한 뒤 새 문서만 설치하고, Catalog·Tree·Composition 리소스·프로젝트 None 등록을 연결한다. 기존 사용자 애니메이션과 폭발 항목의 내용은 byte/구조 비교로 보존을 확인한다.

## G05. Resources와 앵커 UI의 일관성

사용자가 첨부한 Composition Resources의 Effect V1 목록은 stable ID를 평면으로 나열하고 있다. 이 화면도 All Effects의 owner category(DimensionMaster 등 직업, World, KoukuSaydon)와 EffectResourceTree의 동일한 이름·분류를 소비하도록 변경한다. Pattern/Sequence Workbench가 공통 렌더링을 사용하며 Preview Source/Create/Append와 V1 Element 선택 계약은 유지한다. 별도 분류 JSON을 만들지 않는다.

현재 저장 `WORLD`는 World 트랙의 오브젝트 연결이며 `worldId`가 필수다. 저장 `MAP`은 고정 월드 좌표다. 화면은 Boss/World로 통일하고 World 안에서 고정 위치 또는 연출 오브젝트 연결을 고른다. 기존 저장값과 제품 재생 경로는 유지하며, 잘못된 WORLD 빈 ID를 고정 위치처럼 표시하지 않는다. 고정 위치는 기존 Use Player Position/Use Mouse Position과 Focus를 사용한다. 최초 위치는 현재 플레이어의 유효한 위치로 준비하고 명시 선택으로 바꿀 수 있게 한다. 기존 문서나 실행 중 초안 전체를 변환하지 않는다.

Created Resources의 이름은 Composition 별칭이며 `assetId`가 원본과 연결한다. 새 리소스는 동일한 원본 표시 이름을 기본으로 사용하고, 기존 별칭은 보존한다. 선택 시 원본 이름·분류·ID를 표시하며 `Use Source Name`으로 현재 Composition의 이름만 명시적으로 맞출 수 있게 한다. 기존 모든 별칭이나 원본 파일을 일괄 변경하지 않는다.

## G06. 검증과 전달

### G05 후속. Sequence Append와 직접 월드 피킹

2026-09-14 첨부 화면의 `KAKULSAYDON_G1_PATTERN_4.presentation.35`는 빈 `worldId`를 가진 WORLD다. 신규 Sequence Effect의 기본 배치는 MAP으로 준비하고, 명시적으로 선택한 World box 연결은 유지한다. 기존 WORLD 박스에도 Player/Mouse 위치 버튼을 표시한다. 피킹 요청 자체는 기존 박스를 바꾸지 않고 유효한 hit에서만 MAP, 절대 위치, follow/bone/world 참조 해제를 함께 stage한다. 취소와 stale 요청은 기존 편집을 보존한다. CPP/H의 기존 placement 경로만 수정하며 새 파일·프로젝트 등록·데이터 일괄 변환은 없다. 수정 TU 컴파일과 요청/취소/완료의 집중 검증으로 확인한다.

분류 재생성의 중복·유실·동일성, JSON/XML parse, 선택한 실제 문서의 codec/CPU 재생, 수정 C++의 컴파일과 정상 증분 Product Build, `git diff --check`를 확인한다. 실행 중인 Client에는 사용자 편집이 있을 수 있으므로 종료·Reload·UI 조작·캡처를 하지 않는다. 출력 점유나 미저장 상태로 제품 바이너리를 교체할 수 없으면 현재 컴파일·데이터 반영 상태와 필요한 사용자 동작을 RESULT에 분리한다.

화면에서의 최종 위치·크기·본 부착·재생 판단은 사용자가 한다. 최종 안내는 Effect Tool V1의 KoukuSaydon 분류에서 불뿜기 Open Editor/Play All과 Current Effect의 요소 Transform 편집으로 이어지도록 구체적으로 작성한다.
