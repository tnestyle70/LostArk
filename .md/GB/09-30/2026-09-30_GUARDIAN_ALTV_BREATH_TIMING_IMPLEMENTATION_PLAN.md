# Guardian ALT V 브레스 타격 시점 구현 계획

## G00. 실측과 목표

GuardianKnight ALT_V는 skill49420과 ddk_sk_super_sanctumofembereth clip이다. PlayerSkills.hitTimeMs 및 GuardianKnight.hitshapes의 DAMAGE/COUNTER/STAGGER 세 행이300ms다. reference skilltiming의 hits는 빈 문자열이며300ms는 원본 action 타격 notify를 직접 추출한 값이 아니다.

현재 animevents는 projectile494200을3790ms에 시작한다. 실제 effect.guardianknight.skill.49420.clip.0.full.restore의 fx-03 FireBreath01 12개 및 fx-04 LocalFireBreath01 2개가3.78999996초부터 재생한다. guardian.altv.source.effect는 sequence0ms 시작이며 camera zoomout의1500+2300ms=3800ms에 follow camera로 돌아간다. 두 시점은30Hz의114번째 tick으로 양자화된다.

## G01. 데이터 변경

피해를 카메라 복귀/브레스 최초 표시 구간인3800ms에 맞춘다. Data/Balance/PlayerSkills.json의skill49420.hitTimeMs, Data/Animation/HitShapes/GuardianKnight.hitshapes.json의해당 stable collider3개의timeMs, GuardianKnight.animevents의동일clip HIT start/end, 공식receipt의skill49420.hitTimeMs PROJECT_TUNED 근거만 변경한다. 피해량·범위·반복·4833ms action lifetime·이펙트·카메라 시간과 사용자 튜닝은 보존한다.

최신 저장 bytes에서 대상 필드만 병합하고 후보의 의미상 diff를 검증한다. 교체 직전 모든 source hash와 카메라·브레스 evidence hash를 재확인하며 ReplaceFileW 백업·원자교체와 실패 시 자기 변경 rollback을 사용한다. 승인된 현재 저장본 반영 계약을 적용하며 실행 중 Tool draft를 Reload하지 않는다.

## G02. 소비자와 검증

기존 Publish-GameplayBalance는 HitShapes.timeMs를 SKILLHIT에 저장하고 PlayerSkillSystem.Update는 해당 clock이 되기 전 collider를 적용하지 않는다. 단순 PlayerSkills.hitTimeMs만 바꾸면 shape 소비자는300ms로 남으므로 함께 수정한다. Retail profile은49420의timing을 override하지 않는다.

기존 ServerGameplayContractTests_SkillStages.cpp의Guardian fixture에3.799초 피해0,3.801초 최초 피해1회, 후속 중복없음을 추가한다. 새 C++ 파일과project/filter등록은 없다. JSON parse, data/source consistency, scoped Python 검사, Gameplay Validate/Publish, 기존 --skill-stages-contract-test를 사용한다. 실제 Client/UI는 실행하지 않고 최종 화면 확인은 사용자에게 남긴다.
