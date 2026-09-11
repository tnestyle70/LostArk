# 차원술사 BA 1~4 표현 배율 2배 결과

## G00. 실제 변경

DimensionMaster.skillbindings.json의 skill 2050010 네 clip에 playRate 2.0을 설정했다. 기존 기본값은 모두1.0이었다. BA1의 playMs 1400과 BA2~4의 원본 clip 재생 범위는 보존했다. 다른 skill binding과 Server action/판정/입력창, root motion 수치는 변경하지 않았다.

BA1~4 Product cue는 각각 effect.dimensionmaster.skill.2050010.ba0~ba3.full.restore다. Character::Start_Clip의 model speed와 Update_EffectCues의 EFFECT_SPAWN_DESC가 같은 clip 배율을 소비한다. EffectPresentationService는 배율을 EffectObject에 전달하고 EffectObject는 delta*rate로 Playback을 진행한다. 이로써 스폰 시점·burst·파티클 진행과 수명이 같은2배 시계로 동작한다. 발생량이나 source spawnRate를 별도로 곱하지 않았다.

## G01. 검증과 적용

- 변경 JSON parse PASS.
- HEAD와 비교해 네 clip의 playRate 2.0 설정 이외의 의미 변경 없음 PASS.
- BA 네 clip과 실제 Product animevents/이펙트 파일의 1:1 연결 PASS.
- git diff --check PASS. 병행 작업의 쿠크 변경은 수정·stage하지 않았다.
- C++/XML 변경이 없어 재컴파일과 publisher 실행은 필요하지 않으며 수행하지 않았다.
- Client/UI 실행 및 화면 검증 미실행. Client 재실행 후 차원술사 LMB로 확인한다. Server의 콤보 진행·판정 시간은 기존 그대로인 presentation 배율 변경이다.
