# 발탄 돌의 Product 생성 연결 구현 계획

## G00. 현재 연결과 변경 경계

십자 돌은 `effect.valtan.sequence.cross`의 네 particle이 0.5초 동안 각 축으로 움직이며
고정된 월드 거리마다 돌을 생성한다. 원본 복원 source 재질은 이 문서에 연결되지 않았다.
땅구르기·피자·발악의 돌은 BossCatalog의 서로 다른 V1 active/explode를 Server combat-object
spawn/hit가 호출한다. 이미 native2391 재질을 사용하므로 외형 교정은 같은 material·돌에만
적용하고 Server 위치, 1.5m cover, 5초/19.5초 폭발과 기존 companion은 보존한다.

## G01. 기존 고정 간격 생성기와 native carrier 연결

`Client/Private/Effect_Playback.cpp`의 Step에서 portable authored SourceRecipe에 명시된
fixedCenterSpacingWorldUnits를 기존 Spawn_FixedCenterSpacingParticles로 전달한다.
동일 Spawn_Particles의 source module 평가가 lifetime/size/native dynamic parameter를 적용한다.
source burst/rate와 함께 생성하지 않으며 spacing 없는 기존 source는 현재 분기를 유지한다.

`Client/Private/Effect_DocumentCodec_Validation.cpp`는 direct-authored world-space mesh
SourceRecipe의 고정 간격 사용을 허용한다. 위치·속도·가속도·추종 module이 없는 required,
spawn, lifetime, TypeDataMesh, size, dynamic parameter만 허용해 중심점 계약을 유지한다.
기존 양수 간격·zero rate/burst·world space 검사는 그대로 유지한다. 새 C++ 파일과
public struct가 없으므로 기존 vcxproj/filters 등록은 변경하지 않는다.

## G02. 최신 저장본 기반 후보

십자 기존 stable element ID 네 개의 material/resources를 수정한 native 돌로 교체하고
기존 네 축의 position/velocity·0.5초 방출·간격·particle life·연기 네 요소를 보존한다.
sourceRecipe는 같은 돌의 검증된 persistent recipe를 복사해 직접 저작 carrier로 만들고
spawn 권위는 기존 fixed spacing에 둔다. 소스 돌의 notify anchor와 다른 emitter는 복제하지 않는다.
1.2배 시각 배율 및 dissolve 교정은 통합 담당과 동일 기준을 사용한다.

데이터 후보와 이전 SHA/백업은 out/ValtanStoneProduct20260922에 둔다. 최종 저장본 반영,
관련 domain publish 및 Product 빌드는 통합 담당이 수행한다. 다른 사용자 편집을 덮지 않는다.

## G03. 검증

후보의 실제 CEffectDocumentCodec load/validate와 CEffectPlayback 60Hz 샘플에서 네 축 돌의
개수·birth center·수명·native dynamic 값과 scale을 기준본과 대조한다. spacing 없는 세
persistent 돌은 1개와 Server 폭발 시각을 유지하고 편집 문서 roundtrip도 검사한다.
실패 입력의 기존 문서 보존과 targeted source compile, JSON parse, git diff --check를 확인한다.
Client/UI를 실행하지 않으며 최종 화면 판단은 사용자가 수행한다.
