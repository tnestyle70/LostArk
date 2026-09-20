# 마리오 NPC 광대 망치 마무리 결과

원본 `MN_REUP_05` action4221701(서커스_뿅망치 휘두르기)을 확인했다. NPC REUP와 플레이어 변신광대, RHKP 대포를 구분했다.

- `Object/마리오_광대` parent, 대기/뿅망치_휘두르기_마무리 두motion을 신규stableID `world.object.kouku.mario_clown`으로 추가했다. 기존 REUP WModel의 몸체와 내장 망치를 재사용한다.
- attack길이1367ms의 MOTION_END에 `effect.kouku.mario.clown.hammer.end`를1회 재생하고 원본FX tail1401ms를 유지한다. 기존WorldSequencePlayer가모델/FX/도구미리보기를소비한다.
- 원본 source `fx_mn_reup_05.Par_Y_REUP_Atk01_02_LOC_INT` 두mesh emitter를 native material2936 및 기존 fm_h_swing_03 mesh로 복원했다. 마지막 source notify .724025초의TRS/9parameters는보존했다. clip끝으로이동한timing은사용자요청기반프로젝트튜닝이다. 원본첫swipe .426153초를중복발생시키지않는다.
- MonsterCatalog optional attack.endEffectAssetId→ClientReplication→CNpc 연결로 실제Server-spawn NPC도 동일FX를실제clip끝에서1회생성한다. 다른action으로변경시예약이취소된다. 전체NPC효과가아니라해당REUP공격만참조한다.
- Object모델의-90°기본몸체방향을world transform에서적용하고 cue추가rotation+90°로상쇄한다. effect원본SNAPSHOT_ROOT sourcebasis-90°는한번만소비한다.

검증: 실제설치REUP CModel WARP load(2mesh/망치포함, prop1socket), actual attack1.36667초 PASS. 실제WorldSequenceDocument 전체AuthoringLoad/Validate29checks PASS. 실제Codec Load/Validate_Drawable/roundtrip 및CEffectPlayback 재생/rewind PASS:1.4초,2emitter,96samples/1640finitechecks. 로그 `out/BingoRepair20260920/cpu/mario-probe.log`. Npc/ActorCatalog/ClientReplication/Level_Loading 4 TU 최종 컴파일 PASS. JSON/project XML parse와 변경 범위 git diff --check PASS.

소스와저작파일설치완료. RuntimeWorldSeq publish와최종Client/Server빌드는root통합작업에서실시한다. Client/UI/화면확인안함. 최종FX의시각적크기/방향은사용자확인대상이다.
