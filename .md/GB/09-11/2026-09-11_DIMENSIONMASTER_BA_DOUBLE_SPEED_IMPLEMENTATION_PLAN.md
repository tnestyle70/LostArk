# 차원술사 BA 1~4 표현 배율 2배 구현 계획

## G00. 요청과 현재 소비자

사용자의 `배율만, 빠르게` 요청에 따라 skill 2050010의 네 clip에 기존 playRate 2.0을 설정한다. 현재 네 clip은 기본 1.0이다. 첫 clip의 source playMs 1400과 나머지 자연 재생 범위는 보존한다.

정본은 Data/Animation/Authored/DimensionMaster/DimensionMaster.skillbindings.json이다. Character::Start_Clip은 model animation speed에 이 값을 전달하고 Update_EffectCues는 같은 값을 EFFECT_SPAWN_DESC::fPlaybackRate로 전달한다. 각 clip의 Product cue는 ba0~ba3.full.restore를 순서대로 참조한다. spawn 수량 배율을 추가하면 중복 적용되므로 별도 변경하지 않는다.

## G01. 변경과 종료 조건

기존 네 clip 항목에 playRate 2.0만 추가한다. Server action/피해/콤보 입력창과 root motion 수치는 변경하지 않는 presentation 배율 조정이다. 새로운 C++/프로젝트 등록이나 데이터 publisher가 필요하지 않다.

변경 JSON parse, 네 clip과 Product cue 연결, 다른 binding의 의미 보존, git diff --check를 확인한다. C++는 변경하지 않으므로 재컴파일은 필요하지 않다. Client를 실행하지 않으며 실제 화면은 사용자 확인으로 남긴다.
