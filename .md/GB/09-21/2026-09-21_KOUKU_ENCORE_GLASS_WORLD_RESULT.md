# 앵콜 유리 파편·spark World 연결 결과

## G00. 완료 상태와 인계

`prepare_encore_glass_effects.py`가 SCENE07A의 파편 12개와 spark 2개를 기존
`sequence.kouku.bingo.encore.saydon`의 `actor` slot에 연결할 후보를 만든다.
라이브 Data·Resources·C++는 이 작업에서 수정하지 않았다. 부모 작업이
`out/KoukuEncoreGlass20260921/field-patch.json`의 stable ID 행을 최신 저장본에 병합한다.
전체 World 후보는 검증용이다. 최신 ending·arrow 등 다른 변경을 덮어쓰는 용도로 사용하지 않는다.

| 인계 항목 | 내용 |
|---|---|
| World Sequence | 기존 template에 `effect.kouku.bingo.encore.glass.*` effectTracks 14개 |
| 기존 파편 asset | `effect.kouku.source.fx_cm_02.ice.par_e_shot_01`, `effect.kouku.source.fx_cm_02.ice.par_g_icebomb_01_pr` 재사용 |
| 새 spark 문서 | `effect.kouku.bingo.encore.glass.spark0`, `.spark` 두 개; `candidate/Data/Effects/Authored` |
| EffectCatalog | 위 두 문서의 DIRECT_AUTHORED_DOCUMENT 행 추가 |
| 기존 소비자 | Action P97, Sequence P10 모두 `world.sequence.instance.kouku.bingo.encore.saydon` 한 개를 참조 |

후보 생성 명령은 `python -B Tools/KoukuSaydonPipeline/prepare_encore_glass_effects.py`다.
후보 출력은 저장소 `out/` 안으로 제한한다. World 반영 후 부모 작업은
`Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Scope WorldSequences -Mode Publish`
또는 해당 도메인 owner 게시를 실행한다. 효과 문서는 기존 직접 저작 문서 catalog를 소비한다.

## G01. 원본 위치·시간과 1.7배 보정

14개의 원본 emitter는 모두 actor 24 → camera 3 → dummy 4 부모 체인이다.
각 Move는 한 개의 상수 parent-local 키이며 bone 부착이 아니다. source UPK의
InterpLookupTrack도 다시 해독하여 동적 lookup이 없음을 확인했다. 상대 위치·Euler,
원본 drawScale을 소비하고 UE cm→Client m/basis를 한 번 적용했다.

현재 배우 body preScale 0.017과 원본 0.01의 비율 1.7을 local offset과 크기에 각각
한 번 적용한다. 이미 있는 배우 motion/quaternion과 1.7배 거리의 카메라는 보존한다.
3,710시점을 원본 부모 체인과 대조한 최대 위치 오차는 **0.00084058m**, 카메라 투영
NDC 최대 오차는 **0.00109992**다. 원본의 마지막 돌진 때 부모가 카메라 뒤로 지나가는
구간도 같은 부호로 재현된다. 실제 모든 Trigger 시점의 emitter는 카메라 앞에 있다.

파편 12개는 15.433334초에 한 번 발생한다. spark는 12.500001, 13.533334,
15.433334초에 발생하며 각각 원본 OFF 12.833334, 13.866668, 16.000000초로
emission을 닫는다. 파편에 임의 Matinee-end emission 구간을 만들지 않았다.
원본 입자 수명과 OFF 뒤 살아 있는 입자 꼬리는 유지한다.

## G02. 검증 증거

| 검사 | 실행 결과 |
|---|---|
| 실제 현재 native codec/CPU playback | **102 PASS / 0 FAIL**, 14개 그룹의 drawable·roundtrip·finite duration·비어 있지 않은 입자·OFF 이후 새 birth 없음·잔여 수명 종료 |
| spark 반복 | 두 문서 모두 실제 입자 emitter clock 3개가 각각 활성화됨 |
| Python 계약 | **5 PASS**, Trigger/OFF 해석·가짜 Matinee 종료 금지·움직이는 local 키 거부·live Data 출력 거부 |
| 재실행 | effectTracks 14개와 spark 문서 2개의 바이트가 동일 |
| 재질 보존 | 파생 spark 18개 Element의 material이 원본과 완전히 동일 |
| 제품 실행 | 실행하지 않음; 사용자 화면 검증은 미완료 |

증거는 `out/KoukuEncoreGlass20260921/native.log`, `verification.json`,
`source-transform-evidence.json`, `receipt.json`이다. 네이티브 World 검사는 실제
Encore resource/template/instance만 분리한 문서를 사용했다. 무관한 Deploy/Map placement는
정상 level catalog가 필요하므로 전체 World 검증·게시는 부모 owner 단계에서 수행한다.
Effect TU는 현재 소스를 scratch 컴파일했고, 앵콜 ending 담당자의 최신 Orbit 지원
`Effect_Playback.obj`, `Effect_DocumentCodec_PortableRuntime.obj`도 받아 다시 링크해 통과했다.
화면 균열 native shader는 부모 작업과 별도 담당자의 구현이며 이 결과의 파편 복원과 구분한다.

최종 두 후보 SHA-256:

- spark0: `d863e01ca44ef594a87192c3c0a4e1ee5e68d8cb71022ce3134dc2a2fa2fa87a`
- spark: `29f4969be9c4a262a00c95b5d5879e7f7ab5276eccec84f2a13b097c858712bc`
