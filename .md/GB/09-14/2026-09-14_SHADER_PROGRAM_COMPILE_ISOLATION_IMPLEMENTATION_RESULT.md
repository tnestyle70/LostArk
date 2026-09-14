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
