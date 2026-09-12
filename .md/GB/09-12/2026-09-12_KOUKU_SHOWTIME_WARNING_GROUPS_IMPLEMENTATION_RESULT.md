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

