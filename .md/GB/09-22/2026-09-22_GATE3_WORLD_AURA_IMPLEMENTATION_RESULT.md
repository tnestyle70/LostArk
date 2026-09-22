# Gate3 World 오라 복원 결과

## 반영한 원본·데이터

Zone37081의 DeployData Prop300010→Prop DB EFDLProp_ITR_00279→LookInfo→BFX_Low_01.Etc.Par_G_ITR_T_EF_On/GoOff를 연결했다. On은 녹색 대기 4 emitter, GoOff는 파란 활성 2 emitter다. Off는 1회 종료 이펙트이므로 대기 상태로 사용하지 않는다. Prop300009→EFDLProp_ITR_10175→FX_ITR_10175.Par_G_Waiting_01의 황금색 리스폰 14 emitter도 복원했다.

World ResourceTree와 EffectCatalog에 `effect.world.entry_aura`(월드 | 진입오라), `effect.world.entry_aura.active`(월드 | 진입오라 · 활성), `effect.world.respawn_aura`(월드 | 리스폰오라)를 등록했다. 신규 authored 문서는 기존 sourceRecipe·native material·CEffectPlayback 경로를 사용한다. 기존 catalog row와 native group은 보존했다.

원본 14 material의 BasePass native4416..4429와 원본 distortion companion 4개를 복원했다. 전체14 programs의 deferred는0이다. exact source texture24개는 기존 Effect-relative 리소스 또는 원본 UModel 추출로 연결했고, mesh4개는 World/Meshes에 설치했다. 새로 추출한 fm_g_square_01과 fm_d_gear_01은 glTF→WModel geometry parity 경로를 사용한다. fm_d_chamferbox_01과 fm_g_sword_01은 기존 동일 원본 geometry를 재사용한다. sword TypeData pre-rotation[-90,0,90]은 원본 그대로 유지한다.

오라의 20개 emitter는 모두 source sprite/mesh, emitterLoopCount0이다. 저작 window는10초이며 runtime 소유자는 기존 Enable_OwnerSustainedSourceLoops로 지속 재생한다. 모듈 lifetime과 native shader 수식을 임의 대체하지 않았다.

## 원본 배치와 trigger 근거

진입오라 center는[-22.2061767578,25.59,954.594296875], yaw -67.5도다. fm_g_square_01 정점 bounds X±20.057693481, Z±20.188861847에 modelPreScale.01와 원본 StartSize18을 적용한 XZ 반폭은[3.610384827,3.633995132]m다. square TypeData와 MeshRotation에는 추가 회전이 없다. 지면 source offset5cm는 문서에서 보존한다.

리스폰오라는[-11.9999291992,25.59,964.54328125], yaw -22.5도다. 이 값은 원본 DeployData transform을 기존 cm→m·축 변환으로 변환한 것이며 스크린샷에서 위치를 추측한 값이 아니다. 통합 담당이 Shared `Is_KoukuGate3EntryAura`에 이 XZ 반폭과 yaw, 높이24.59..27.59m를 연결했다. Level은 replicated player pose와 `Get_LastServerTick()`의30Hz tick으로 연속10초를 센 뒤 leader의 기존 typed `ENTER_GATE3`를1회 제출하며 기존 파티 동의를 유지한다. 이탈·사망·entry 상태변경은 countdown을 취소한다. 기존 제품 font 경로로 안내문을 흰색, 남은 초를 노란색으로 표시한다.

ClickMoveEffect의 optional prewarm에3개 asset을 추가했고 Level owner가 sustained source loop를 소유한다. 새 aura stage/commit과 handle 생존 확인 성공 이후에만 이전 aura를 정리하므로 실패 시 기존 표시를 보존한다. WAIT_ENTRY의 Client 이동 입력을 열고 Server Handle_Move에서 현재·목표가 준비 terrace인지 검증한 후 기존 navigation/collision을 따른다. skill 차단과 Server 전투 권위는 유지한다.

## 실행한 검증

- 세 문서 JSON parse,20 stable emitter ID의 유일성, sprite/mesh carrier와 loop0, 모든 참조 resource 존재를 확인했다.
- 14 native material program과4 distortion companion의 원본 shader closure를 생성했고 deferred0을 확인했다.
- `append_reviewed`는 기존1659 programs를 유지하며 새4416 group을 추가했다. 설치 전 기존 native group의 bytes가 설치 후에도 동일함을 확인했다.
- Catalog/ResourceTree는 stable ID 단위로 추가했으며 기존 row를 보존했다. 교체 직전 bytes/hash를 확인하고 out의 백업을 남긴 뒤 원자적으로 교체했다.
- `.vcxproj`·filters XML parse 및 ProjectReference/Project childcount0, GUID parse를 확인했다. 처음 임시 Data 등록 스크립트가 nested Project closing까지 삽입한 오류는 해당2개 블록만 제거해 수정했고, 마지막 root closing에만 삽입하도록 스크립트를 교정했다.
- Python 복원 도구 `py_compile`과 관련 `git diff --check`를 통과했다.

정상 통합 Product Debug 빌드의 최신 분할 codec/material/playback object로 기존 World marker CPU 진단을 세 문서에 한정해 다시 링크했다. 녹색4·파랑2·리스폰14의 모든 원본20 emitter가 유한한 transform/color와 함께 출력됐다. 각727/727/787 fixed-step 샘플, serialize/parse roundtrip, Save_Atomic, corrupt parse·duplicate stage 실패 시 기존 상태 보존, rewind 및 world root translation 검증을 통과했다. 유한 preview는12/12/13초 후 정리되며 owner-sustained는20초에서도 각각4/3/39개 particle 출력을 유지했다. 로그는 out/Gate3Aura20260922/cpu_probe_run.log다.

통합 Product 빌드 로그는 out/GuardianPlaybackRepair20260922/build/20260922T042238189Z-Client-Debug.log를 따른다. Client/UI는 실행하지 않았으며 최종 원본 유사도·GPU 표시·트리거 화면 판정은 사용자 확인 대상이다.

## 재현 경로

`Tools/EffectPipeline/build_gate3_world_auras.py`의 source→materials→geometry→project 단계를 사용한다. `--game-release`, `--umodel`, `--evidence-root`로 실제 원본과 도구 위치를 지정할 수 있다. materials는 기존 최신 `build_kouku_pattern_native`를 호출하며 zero-CB0·engine-only texture와 distortion을 함께 복원한다. 후보는 out/Gate3Aura20260922/candidate에 생성하고 native 설치는 기존 `install_kouku_gate1_native_shaders.append_reviewed`를 사용한다. 제품 데이터의 최종 교체는 최신 stable ID·hash 병합 단계로 수행한다.

세부 source/geometry/native/설치 증거는 out/Gate3Aura20260922의 source_occurrences.json, source_module_inputs.json, geometry_installation.json, native/merged_native_runtime_contract.json, structural-verification.json에 남긴다. Resources 설치와 코드·데이터 반영을 실행 중 Client reload 또는 Server 자동 갱신으로 설명하지 않는다.
