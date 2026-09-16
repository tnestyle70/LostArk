# 세이튼 노란 예고 장판과 공격 그룹 구현 결과

## G00. 반영 상태

노란 예고 장판을 기존 공격 앞에 연결한 V1 그룹 세 종류를 Authored, EffectCatalog, EffectResourceTree에 반영했다. 기존 공격 leaf와 작은 오망성 변경은 보존했다. Composition resource와 Client project의 신규 Data `None` 항목은 Parent 작업자가 최신 문서에 연결한다. 제품 publish와 최종 Client 실행 여부도 Parent 작업 결과를 따른다.

| 표시 이름 | asset ID | 구성 | CPU 문서 길이 |
|---|---|---|---:|
| 원형 예고·폭발 | `effect.kouku.gate3.showtime.circle.warning.impact` | 반경 4 m 예고 + 기존 `fire.impact` 11요소 | 5.8초 |
| 도넛 예고·폭발 | `effect.kouku.gate3.showtime.donut.warning.impact` | 내반경 4 m·외반경 8 m 예고 + 기존 `circle.impact01` 13요소 | 6.5초 |
| 부채꼴 예고·사격 (저작 조합) | `effect.kouku.gate3.showtime.sector.warning.shot` | 반경 11 m·45° 예고 + 기존 `gun.ground` 원본 독립 leaf 3요소 | 4.0초 |

모두 예고가 0초에 시작하고 1.5초에 원본 공격이 시작한다. 0.2초 fade in과 마지막 0.2초 fade out을 적용했다. 예고는 한 번만 생성하며 원본 공격의 입자 수명과 방출 구성을 보존한다. 부채꼴 그룹은 본 부착 미리보기 대신 동일 원본 ParticleSystem의 독립 root leaf를 소비한다.

## G01. 원본 근거와 투영 경계

원형은 FixedArea 421991207 → SkillEffect 421991218 → SkillDecal 2112, 도넛은 421991208 → 421991220 → 2116으로 연결된다. 원본 FixedArea 후미에서 1.5초 timer, 0.2초 fade 값, SkillDecal ID와 damage SkillEffect ID를 확인했다. `EFTable_SkillDecal.db`의 GroundEffect archetype을 실제 data3.lpk ParticleSoundNew 문서로 연결해 다음 원본 material을 복원했다.

- 3600: `fx_m_mi_o_00.fx_mi.fx_o_de_condcircle_02_01_tr`
- 3601: `fx_m_mi_o_00.fx_mi.fx_o_de_condmondonut_02_01_tr`
- 3602: `fx_m_mi_o_00.fx_mi.fx_o_de_condmonfan_02_01_tr`

GroundEffect의 실제 구조체 필드 순서로 Width/Height 100 cm, Near/Far -300/+300 cm, Thickness 10 cm, ActiveColor `[1, 0.5, 0, 3]`을 확인했다. 기존 LocalDecal projector에 한 번 발생하는 `project.groundeffect.adapter.*` recipe를 사용한다. 원본 Cascade emitter가 있었다는 의미가 아니다. source PS 수식에서 `thickness`는 도넛의 내외 반경 비율, `angle`은 전체 회전 비율이므로 각각 0.5/1 및 부채꼴 0.125를 전달했다. 부채꼴은 LocalDecal UV의 방향을 root +Z에 맞춘다.

부채꼴의 원본 FanShape는 SkillDecal 2111이고 실제 사격 범위는 SkillEffect 421991212이다. Showtime action에는 2111 PlayDecalEffect 호출이 없어 이 둘의 조합과 1.5초 예고는 프로젝트 저작이다. 원본 Showtime의 동일 occurrence를 완전히 복원했다고 주장하지 않는다.

세 원본 PS 모두 `EngineResources.DefaultTexture`를 샘플하지만 실제 물리 입력은 설치 원본에서 확인되지 않았다. 이 샘플의 유한 차분은 caustic UV 흔들림에 사용된다. 기존 원본 `fx_tex_00.fx_a_blankwhite_01`을 명시적으로 바인딩해 차분을 0으로 만들었다. 실제 `fx_tex_05.fx_m_caustic_001`과 Fan의 `fx_tex_02.fx_d_line_003_ycl`, 원본 도형 경계·색·caustic 시간 수식은 유지한다. **원본 엔진 노이즈에 의한 UV 흔들림은 미복원이다.** 이 제한은 `engine_noise_adapter.json`에도 기록했다.

## G02. 변경 파일과 검증

전용 `Tools/EffectPipeline/build_kouku_showtime_warning_groups.py`가 원본 연결 추출, native 후보 생성, 최소 설치, V1 문서 생성과 Catalog/Tree 등록을 담당한다. `--install-native`는 현재 설치된 corpus를 보존하고 세 ID만 추가한다. 64개 단위의 기존 3584 bucket을 사용한다.

`generate_artist_native_runtime_shader.py`에는 세 source PS와 실제 VS `51afa7d015c2db45bc7c7faf7300c9c3`, CB0 미소유 행 `[0,1,2,3]`을 함께 검증하는 GroundEffect prefix를 추가했다. 기존 Cascade prefix와 분리하며 TEXCOORD5에 source world position을 제공한다. `Shader_VtxEffectDecal.hlsl`에는 기존 다른 carrier와 같은 X/-Z/Y, cm 변환 입력 한 줄을 추가했다. 설치 산출물은 `Effect_ArtistMaterial.h`, `Shader_EffectKoukuNativeGroup3584.hlsli`, `Shader_EffectArtistNative.hlsli`다. 새 carrier 또는 거대 단일 group은 만들지 않았다.

실행한 검증은 다음과 같다.

- 실제 native lowering 3개 성공, deferred 0개.
- 현재 `Effect_DocumentCodec.cpp`, `Effect_Playback.cpp`로 out probe를 컴파일하고 세 문서를 Load/Stage했다. 60 Hz로 각 421프레임을 평가했으며 모든 평가 행렬은 finite였다.
- 예고 peak는 그룹마다 정확히 1개, 평가 폭은 8/16/22 m, 0.5초 alpha는 원본값 3, 공격 최초 발생은 세 그룹 모두 1.5초다. fade 종료 뒤 예고가 사라지고 7초까지 재발생하지 않았다. fixed-step 입자의 제거 시점은 1.5초 경계 뒤 최대 두 샘플까지 유지될 수 있으나 fade 값은 1.5초에 0이다.
- 세 문서의 stable element ID 중복 없음, 참조 Resource 52개 모두 존재, Catalog/Tree 항목 각각 정확히 1개. 생성·등록 재실행 결과 byte 동일.
- `fxc /T fx_5_0 /O1`의 실제 Decal carrier out compile 성공. `out/KoukuShowtimeWarnings20260912/Shader_VtxEffectDecal.cso`는 793,187 bytes. 기존 공통 sample helper의 X4000 warning은 로그에 남아 있다.
- Python py_compile, 변경 범위 `git diff --check` 통과.

증거는 `out/KoukuShowtimeWarnings20260912/source_warning_chains.json`, `native/native_runtime_contract.json`, `engine_noise_adapter.json`, `authored_groups.json`, `warning_probe.json`, `asset_validation.json`, `fxc.log`에 있다. out과 Resources binary는 Git에 추가하지 않았다.

Client/UI를 실행하거나 캡처하지 않았다. 첨부 이미지와의 최종 GPU 표시, 세 장판과 실제 공격의 육안 정합성은 사용자 확인 대기이며 visual PASS로 기록하지 않는다. 사용자의 마무리 지시에 따라 추가 후보 조사와 범위 확장은 중단했다.

## 최종 통합 확인

루트의 최종 Client 컴파일/링크·Server 빌드와 revision 352 공식 Kouku 4-domain 게시를 완료했다. 노란 3종은 Composition과 Client project에 등록했고 검증한 Decal CSO를 기본 Debug 경로에 반영했다. 상세 로그·중단 범위·사용자 실행 경로는 [통합 결과](2026-09-12_KOUKU_PARENT_PATTERN_TIMELINE_IMPLEMENTATION_RESULT.md#G03-배포와-사용자-확인)를 따른다. 사용자 화면 확인은 대기다.

## G03. 노란 예고의 쿠크 표면 수신 제외 — 2026-09-15

사용자가 보스가 노란 장판과 겹치면 몸이 노랗게 그려진다고 보고했다. 현재 쿠크 Decal 131요소 중 GroundEffect native3600/3601/3602/3607을 쓰는 26요소는 projector 깊이6m, upward cutoff0.5다. 원본 Near/Far -300/+300cm를 보존한 이 투영은 높이1m·위쪽 법선인 보스 표면도 통과시킨다. 쇼타임 warning 9문서에는 Light 요소가 없다. 이 검사는 현재 장판 수신 경로의 결함을 확인하며 사용자의 화면을 자동 판정한 것은 아니다.

`Shader_SourceCharacterMaterial.hlsli`의 marker5 PickPos.W에 실제 source program ID를 low8 mantissa로 기록했다. depth.z의 프레임 row는 변경하지 않았다. `Effect_DocumentRenderer_Geometry.cpp`가 Target_PickPos를 Decal SRV로 연결하고, `Shader_VtxEffectDecal.hlsl`은 정확한 Load로 읽어 위 네 GroundEffect에 대해서만 program21/26을 거부한다. 본체·무기12개 catalog binding이 두 program에 해당한다. program21은 다른 masked monster도 공유하므로 같은 재질군을 쓰는 다른 actor도 이 네 장판은 받지 않는다. 범용 object 종류 마스크나 원본 엔진의 모든 decal receiver 정책 복원이라고 주장하지 않는다.

전체 Map authoring의 source 계열48행은 program25/30/80..83이며21/26은 없다. 이들 marker5 map과 legacy/map PBR·source BG 등 다른 marker는 유지한다. 원본 색·재질 수식·크기·깊이·채움 시간, 사용자 Effect·Composition 저작 JSON, 다른 Decal과 전역 조명은 변경하지 않았다. 따라서 별도 데이터 publish는 없다.

### G03-01. packed W와 실행 검증

- 실제 Target_PickPos와 Picking staging은 모두 RGBA32_FLOAT다. Picking은 W가0인지 검사한 뒤 XYZ를 사용하고 public W를1로 복구한다. Deferred의 source marker5 light는 PickPos.XYZ를 읽으며, Map normal/RNM decoder는 다른 marker 계약이다. Static shadow encoder는 low23 mantissa를 보존하고 exponent의 채널만 바꾼다.
- 실제 WARP RGBA32_FLOAT texture에 program0..255와 shadow channel0..15 조합4096개를 저장·readback하여 program, exponent, XYZ, bit22가 보존됨을 확인했다. source encoding과 receiver filter는 수정된 제품 파일에서 원문을 추출했다. 이는 GPU texture 정밀도 검증이며 전체 native material draw 검사는 아니다.
- 제품 receiver 함수의28,672개 조합에서 네 GroundEffect·marker5·program21/26만 거부했다. 현재 native Map program6종×4장판24조합은 보존했다. 이전 깊이·법선 검사에서 통과하는 actor overlap fixture가 새 정책에서 거부됨도 확인했다.
- 실제 Decal FX, source21/26 skinned FX, source26 static FX의 `fxc /T fx_5_0 /O1` out 컴파일과 Geometry Debug TU 컴파일이 통과했다. 기존 native X4000과 SDK C4819 경고는 로그에 보존했다. 세 소스 파일의 기존 CRLF·UTF-8을 유지했고 변경 범위 diff 검사도 통과했다.

근거는 `out/KoukuGroundReceiver20260915/validation.receipt.json`, `receiver_probe_result.txt`, `material-audit.txt`, `*_compile.log`와 변경 전 byte 사본이다. shared include를 Engine 정본에서 candidate 폴더로 복사해 검사했으며 실행 중인 제품의 SDK/CSO를 교체하지 않았다. 새 상설 테스트 프로젝트나 제품 빌드, Client/UI 실행·조작·캡처는 하지 않았다. 사용자 Product Build 이후 같은 장판의 몸 색과 바닥 유지 여부 확인이 남아 있다.


## G04. 캐릭터·폭탄 수신 제외와 부채꼴 고정 — 2026-09-15

사용자가 폭탄 본체와 심지가 실제 화면에 나타난 것을 확인한 뒤, 노란 예고가 폭탄과 플레이어도 물들이는 문제를 보고했다. 폭탄의 native program30은 정적 Map과 공유하므로 재질 번호 전체를 제외할 수 없다. 실제 skinned material writer에서 PickPos.W의 mantissa bit8을 기록하고, marker0/5의 이 표식과 native skin/equipment program1..24/26..29만 네 GroundEffect3600/3601/3602/3607에서 제외하도록 소스를 수정했다. Animated Map BG는 표식 전에 반환하고 다른 Map marker의 packed normal/RNM은 해석하지 않는다. program25/30/80..83의 정적 Map 수신은 유지한다.

부채꼴 `kouku.showtime.warning.sector`는 기존 inner track의 첫 값0으로 고정한 JSON 후보를 만들었다. 시간에 따라0→1이던 inner track만 제거하며 반경11m, 각도45°, 원본 caustic/경계, 위치, 색과 fade는 보존한다. 생성 도구의 신규 부채꼴에도 같은 고정 정책을 적용했다. 과거의 G03 채움 보존 설명은 당시 상태이며 현재 요청은 이 정적 정책이다.

### G04-01. 수치·컴파일 확인

- 실제 RGBA32_FLOAT WARP texture의8192 program/skinned/shadow-channel 조합에서XYZ, exponent, bit22, source program과 bit8 roundtrip이 통과했다.
- 제품 HLSL 원문을 추출한57,344 receiver 검사와 현재 정적 Map6개 program×4장판24조합이 통과했다. 기존 depth6m/upward 검사에서 통과하던 actor overlap은 새 필터에서 제외된다.
- SourceGroup001/025 skinned FX와 Decal FX의 분리 `fxc /T fx_5_0 /O1` 컴파일이 모두 통과했다. 기존 native X4000 경고는 로그에 남아 있다. 이는 전체 장면의 GPU 표시나 사용자 visual PASS가 아니다.
- 생성기 Python parse와 fan 후보의 inner0/track 제거·재적용 idempotence를 확인했다. 후보 등록 및 최종 codec/Owner 발행 결과는 아래 통합 설치 기록에 추가한다.

근거는 `out/KoukuShowtimePolish20260915/{root-validation.receipt.json,fan-stage.json,receiver_result.log,native-map-audit.json,Shader_Vtx*.log}`다. 제품 빌드·EXE 교체·Client/UI 실행이나 캡처는 하지 않았다. 소스 반영 뒤 사용자 빌드와 같은 장판의 실제 화면 확인이 필요하다.

### G04-02. 최종 설치

최신 저장844의 다른 사용자 저작과 Stage52/57을 보존한 채 root가14파일을 byte CAS·Composition writer lock으로 설치해845가 됐다. Fan 후보는 실제 Effect codec Load를 통과했고 inner0/inner track 없음이 정본에서 확인됐다. 쇼타임 Owner의 product/map/world/gameplay 네 domain 발행은 모두 PASS다. shader와 C++ 소스·Data 등록까지 완료했으며 제품 빌드는 사용자가 수행한다. 실제 새 화면은 아직 확인하지 않았다. 설치 hash와 runtime 행은 `out/KoukuShowtimePolish20260915/installed-verification.json`에 있다.
