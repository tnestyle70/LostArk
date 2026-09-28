# 발탄 돌의 Product 생성 연결 구현 계획

## G04. 2026-09-28 — 붉은 돌 폭발과 중앙 피자 원본 연결

사용자 제공 화면의 돌은 청록 대기, 흰색·붉은색 충전, 원형 예고와 방사형 폭발의 순서다.
현재 `six-pizza.rock.telegraph/explode/explosion.full`은 기존 저작 요소의 분리·조합이며
원본 ParticleSystem 전체 복원이 아니다. 기존 본문은 보존하고 원본 호출 자료·첫 LOD
module·material·mesh가 확인된 별도 복원 문서를 만든다. armed/hit event는 기존
combat-object authority와 시각을 유지하고 해당 복원 문서를 참조하게 한다.

중앙 피자는 STEP_07의 `mesh_att_battle_12_07`와 STEP_11의 `mesh_att_battle_12_11`에
effect cue가 없다. 원본420620의 stage001·005 Particle notify를 기존 full-restore builder로
복구하고 해당 clip occurrence에 연결한다. 노랑·빨강 sector는 원본 PlayDecalEffect의
재질·부채꼴 범위·시간 입력을 확인해 기존 decal/runtime 경로로 복구한다. 기존 hand-tuned
본문은 지우지 않고 대체된 Pattern 연결만 정확한 cue ID 기준으로 조정한다.

돌의 원본 `ITR_02326`은 Spawn/On/Off 모델 애니메이션과 두 재질을 사용한다. Off 시작에
청록→붉은 발광색·강도 변화와 Off02가 시작하고 Off01 폭발은 1.82초 뒤 시작한다.
이 전체 Off 묶음을 독립 복원 문서로 제공한다. BossCatalog의 optional
`stopActiveOnArmed`, `armedEffectOwnsTerminal`은 기본 false이며 명시한 돌만 기존 idle을
종료하고 하나의 자연 수명 Off를 시작한다. 후속 hit는 이미 시작한 combat-object ID의
Off를 중복 생성하지 않고 Sound는 기존 event를 소비한다. 직접 hit만 받은 돌은 같은 Off를
한 번 시작한다. Preview도 같은 Armed 시작 시계로 끝까지 샘플링하고 실제 Stop/역방향
seek와 boss reset 시 소유 상태를 정리한다. 현재 저장된 Server 피해·연쇄 지연은 보존한다.

Preview의 forward 재생과 pause/seek가 다른 sector 상태를 만드는 경로를 실제 local
boss/Effect service에서 대조한다. 명시 seek의 history 재구성과 정상 전진의 시계·stage
경계가 같은 효과를 내도록 직접 원인만 수정한다. 이전 Sound Save·Preview 잠금·한국어
검색 수정은 유지한다. 새 C++ 파일이 필요하면 기존 project/filter에 등록한다.

후보는 `out/ValtanStonePizzaRestore20260928`에 만들고 native codec/playback의 시각별
생성·수명·유한 transform 및 원본 resource 연결을 확인한다. 사용자 종료·전체 반영 승인에
따라 설치 직전 최신 디스크를 다시 읽고 stable ID/변경 field 병합, canonical writer lock,
hash 재검사·백업·원자 교체·실패 rollback을 적용한다. 필요한 발탄 publisher와 정상 Debug
Product 빌드를 실행한다. 한글 검색명을 실제 All Effects 입력까지 연결해 전달하며 Client/UI
실행·최종 화면 판정은 사용자에게 남긴다. 원본 자료 일치와 화면 동일성을 구분한다.

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
