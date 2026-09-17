# 셰이더 컴파일 단위 분리 결과

## G00. 적용

기존41개 SourceCharacter program(1..32,80..88)을6묶음으로 나눴다. AnimMeshBinary·MeshBinary·Deferred가 각 묶음의 독립 CSO를 사용하며 CShader는 실제 g_SourceCharacterProgram 값으로 선택한다. 기본 CSO의 map forward33..65와 기존 material/alpha/Light pass를 유지한다. 별도 CModel 런타임은 만들지 않았다.

Effect scene color/depth 선언을 Cube/Slice 계산 include에서 분리했다. source generator는 원본 전체 텍스트 재구성과 무변경 write0를 지원하고 project/filter/SDK배포가 새 CSO를 포함한다. CShader binding revision을 공유해 clone의 상태·상수·SRV·bone 배열이 실제 선택 CSO에 전달되며 무변경 Begin의 전체 동기화를 건너뛴다.

## G01. 검증

Engine/Client 정규 FX 컴파일과 Engine Build/link PASS. headless WARP에서123program,6clone,9실패·reload보존,492Light pass 바인딩 검사 PASS(window0/draw0). const/texture/512bone 배열을 실제 GPU binding으로 대조했다. 생성기 roundtrip/no-op/단일leaf변경/미등록ID거절 테스트3개 PASS.

| timestamp를 갱신한 입력 | 재생성 CSO | Engine+Client FX target 경과 |
|---|---:|---:|
| 무변경 | 0 |0.583초|
| Base group009 |2|39.578초|
| Light group009 |2|87.121초|
| 공유 source 입력 |21|413.411초|
| 마지막 무변경 |0|0.604초|

위 시간은 파일 내용 변경 없이 해당 include의 수정 시각을 갱신한 실제 증분 무효화 검사다. 재질 수식 변경이 자기 leaf만 바꾸는지는 별도 생성기 테스트로 검증했다.

Light009의 두 출력은 Deferred와 투명머리카락 Light18을 실제 소비하는 AnimMesh이다. 캐릭터 공용입력 변경은 모든group에영향을주므로 전체재생성이정상이다. 동일입력의분리전수정빌드A/B는없으며, 이시간을기존전체빌드대비단축률로표현하지않는다.

CPU microbench bind+Begin은같은group무변경0.250→0.270µs, group전환무변경0.335→1.262µs,512bone+상수갱신5.063→6.279µs,갱신+group전환5.110→6.447µs다. 프레임FPS나화면동일성검증이아니다. 근거는out/ShaderProgramIsolation20260914/{incremental-measurements.json,warp/probe.log}.

## G02. 보류 범위

Character Select 추가 source material의 steady emission·metallic diffuse mask·masked/바람은조사중이었으며사용자중간마무리요청으로제품미적용이다. 현재변경은컴파일분리와실제기존소비자연결이다. 최종통합Product결과는캐릭터/아레나통합RESULT G06에기록한다.

## G03. 추가 컴파일 최적화 구현 — 2026-09-17

main `bddacace2` 동기화와 Loading 누락 선언 보완 후 Debug Product는 2,290,594ms에 성공했다.
Client 단계는 2,249,173ms, OBJ 220개·CSO 109개 갱신이었다. 이 성공본의 CSO 141개
(986,782,990 bytes)를 `out/ShaderBuildOptimization20260917/baseline/Client`에 보존했다.
이는 이번 셰이더 최적화 전 기준이며, 캐시 없는 동일 입력 A/B 빌드 시간은 아니다.

`codex/shader-build-isolation-20260917`에서 다음 소스를 반영했다.

- Native ModelCue pass 7/8/12/13은 `ProgramVariantPass` annotation으로 기본 FX가 소유한다.
  CShader가 정책을 검증하고 stale SourceCharacter selector가 있어도 기본 native PS를 선택한다.
  SourceGroup 6개는 같은 uniform/resource/default를 유지하고 native 함수·PS 컴파일을 제외한다.
  해당 그룹에서 native pass를 직접 호출하면 실패한다. pass 14개와 input signature·변수 타입
  검사를 유지하며 표면 85~89, 광원·반투명 pass도 보존한다.
- Artist facade를 Inputs/Common/Programs로 분리했다. 기존 27개 native 그룹과 53개
  Mesh/Particle wrapper는 자기 SelectedGroup만 포함한다. 공통 carrier의 가변 최대 ID 대신
  실제 guarded case 목록을 사용하며, 여러 그룹을 소비하는 Decal/Trail/ScreenPost는 전체 목록을
  유지한다. Kouku·Vehicle installer와 기존 전체 교체 generator도 같은 expand/write 경로를 쓴다.
- 일반 shader owner 11개와 V2 common/소비자 6개의 같은 compile 식을 공유했다. 기본 owner와
  V2 확장 기준 170개, Deferred/MeshBinary의 SourceGroup 6개씩을 포함하면 344개를 줄인다.
  AnimMeshBinary의 반투명 pass 9/10 공유는 이 344개와 별도다. 함수·pass 상태·순서는 유지한다.
- 새 HLSLI 30개를 기존 Client project/filter의 None 항목으로 등록했다. 신규 FX와 C++ 파일은 없다.
  Resources 구조·원본 리소스와 제품 재질 수식은 바꾸지 않았다.

## G04. 소스 검증과 통합 빌드 대기

Artist 전체 authoring 텍스트의 펼친 결과는 기존 bytes와 같다
(`057db8686ad15ac2ea713f894b77260bc98c985f1dd090cef5b04a166ba5e6a4`).
일반/V2 공유 변경 18개 파일은 공유 선언을 원래 compile 식으로 되돌리면 기준 원본 bytes와 같다.
근거는 `out/ShaderBuildOptimization20260917/pass-sharing/reversible-proof.json`과
`variant-structural-check.json`이다. 원본 보존 검사는 실제 컴파일·실행 검증과 구분한다.

`test_artist_shader_input_isolation.py` 6개와 기존 `test_source_character_program_groups.py` 3개가
PASS다. roundtrip, 내용·수정시각 no-op, 그룹 추가·case 수정의 제한된 변경 범위, carrier guard,
legacy full-reset, 잘못된 selector의 저장 전 거부를 검사했다. 변경 generator 4개 Python 문법,
Client project/filter XML parse 및 diff check도 통과했다. 기존 headless CShader probe에는 native
기본 FX 선택·clone 공유·상수/texture/bones 144건과 직접 그룹 호출 거부 8건을 추가했다.

00:36 KST 최적화 후 Product를 시도했으나 실행 중인 Debug Client PID 2756과 Server PID 18308을
`ProductOutputGuard.psm1`이 발견해 컴파일 전에 중단했다.
근거는 `out/BuildPipeline/runs/20260916T153644949Z-debug-product.json`이다.
현재 실행 EXE와 CSO는 G03의 최적화 전 성공본이다. 에이전트는 프로세스를 종료하거나 실행하지
않았고 사용자에게 저장·종료 확인을 요청했다. 최적화 후 FXC/Product, CShader probe, 컴파일된
shader closure·bytecode 비교와 실제 증분 시간 측정은 아직 실행하지 않았다.

## G09. 09-17 Effect V1 우선 요청에 따른 후속 분할 보류

후속 조사에서 기존 MSBuild 변경 감지는 131개 중 변경된7개만 선택했고, 느린 지점은 base animated FX와 큰 native family compile임을 확인했다. PLAN G09~G11의 granular composite runtime과 family별 추가 분할은 사용자 요청으로 중단했다. 시험했던 Shader.h/.cpp 추가분만 사전 byte snapshot으로 복원했으며 이전 shader isolation 변경은 보존했다. 백업은 out/ShaderCompositeDesign20260917/paused-composite에 있다. 이번 후속 작업에서 새 generator/MSBuild 변경이나 정상 Product 빌드는 수행하지 않았다. 작은 metadata-only FX 시험만 out에서 수행했다. 이 후속 분할 최적화는 구현 완료가 아니다. 현재 우선 작업은 세이튼 Effect V1 재생·편집 오류 수정이다.