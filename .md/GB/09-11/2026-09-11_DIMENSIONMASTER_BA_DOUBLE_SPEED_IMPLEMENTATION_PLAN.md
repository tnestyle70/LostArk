# 차원술사 BA 표현 배율 2배와 전체 애니메이션 연결 복구 계획

## G00. 요청과 현재 소비자

사용자의 `배율만, 빠르게` 요청과 후속 전체 애니메이션 미재생 신고를 함께 처리한다. PR #357이 병합된 main은 skill 2050010의 중복 `_02` 단계를 제거한 3단계 정본이다. 이전 main에서 만든 PR #358의 4개 clip 수정과 충돌하므로 최신 `_01`, `_03`, `_04`만 유지하고 각 playRate를 2.0으로 설정한다. 첫 clip의 source playMs 1400과 나머지 자연 재생 범위는 보존한다.

정본은 Data/Animation/Authored/DimensionMaster/DimensionMaster.skillbindings.json이다. AnimationSkillBindingDocument::Validate는 COMBO clip 수가 Server 소유 comboStages와 다르면 문서 전체를 거절한다. Character::Load_ClipChains는 그 실패에서 chain commit 전에 반환하므로 신규 캐릭터의 모든 스킬 chain이 비게 된다. 정본 3단계를 맞춰 이 실패 조건을 제거하고 검증과 transaction 경계를 유지한다.

Character::Start_Clip은 model animation speed에 playRate를 전달하고 Update_EffectCues는 같은 값을 EFFECT_SPAWN_DESC::fPlaybackRate로 전달한다. 선택된 clip의 Product cue는 각각 ba0, ba2, ba3.full.restore다. `_02`의 ba1 이펙트 저작 자료는 유지한다. spawn 수량 배율을 추가하면 중복 적용되므로 별도 변경하지 않는다.

## G01. 변경과 종료 조건

최신 main의 세 clip 항목에 playRate 2.0만 추가한다. Server action/피해/콤보 입력창과 root motion 수치는 유지한다. 기존 stage-split 회귀 검사의 binding 기대값만 3단계 2배속으로 갱신한다. 새로운 C++/프로젝트 등록은 필요하지 않다.

변경 JSON parse, 전체 binding과 PlayerSkills의 단계 수, 실제 clip과 Product cue 연결, 다른 binding의 의미 보존, 기존 stage-split 회귀 검사, git diff --check를 확인한다. 같은 PR의 Kouku 버튼 C++ 변경은 Product Debug로 컴파일하고 최신 main 계약의 실행 데이터는 공식 publisher로 게시한 뒤 새 Server로 확인한다. Client를 실행하지 않으며 실제 화면은 사용자 확인으로 남긴다.
