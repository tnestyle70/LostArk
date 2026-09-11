# 쿠크 1관문 두 패턴 원본 V1 이펙트 생성

## 구현 전 계획

요청 범위에 맞춰 선정한 두 짤 패턴을 연결한다. 내려치기C는 MN_RPCT_05 action 4219877 stage 0(2초), 불뿜기는 action 4219801 stage 0→1(1.666667초+3.166667초)이다. 최초 조사한 화염파동은 FireWave notify 3개의 원본 enabled 값이 모두 false여서 활성 불뿜기로 교체했다. action의 분기 stage 전체를 순차 재생으로 바꾸지 않는다.

1. 원본 action notify와 ParticleSystem의 첫 실제 LOD를 정확한 object path로 조인한다. 원본 불뿜기 7개 Particle notify와 내려치기의 Trails를 각각 보존한다.
2. 기존 SourceIndex, emitter_detail, sourceRecipe, action attachment 변환을 재사용한다. 소스 CDO와 분포·파라미터·TRS를 해석하고, 필요한 새 Resources 자산만 cook한다.
3. 두 새 full.restore V1 문서와 재생 수명 근거를 만든다. native material shader 연결은 root 담당, Composition Append는 pattern_inventory 담당이다.
4. JSON 구조, 원본 occurrence 분모와 생성 행, Resources 경로를 검증한다. Client/UI는 실행하지 않으며 화면 판정은 사용자에게 남긴다.

추가 배정된 native 연결은 `install_kouku_gate1_native_materials.py`에서 기존 `Effect_ArtistMaterial.h`의 2304~2341 table 행만 갱신하고, 재질 patch를 full.restore 생성기에 전달한다. `Effect_Playback.cpp`의 baked edge trail은 쿠크 native profile에 한해 보존한 원본 color/alpha/dynamic 모듈을 샘플한다. 기존 발탄의 bounded 표현은 유지한다.

## 진행 상태

선택한 원본 action 단계의 V1 저작 문서 생성, native 재질 연결, Catalog와 프로젝트 등록을 완료했다. 최종 Product Debug 빌드는 root의 통합 작업에서 PASS를 확인했다. Client/UI 실행과 화면 복원 판정은 수행하지 않았다.

| 원본 action / stage | 원본 clip 길이 | 원본 PS 호출 | 원본 emitter 호출 | V1 요소 | 문서 전체 구간 |
|---|---:|---:|---:|---:|---:|
| 4219877 / 0 내려치기C | 2,000ms | Particle 5 + Trails 1 | 16 | 16 | 6,582ms |
| 4219801 / 0→1 불뿜기 | 1,666.667 + 3,166.667ms | Particle 7 | 37 | 57 | 11,075ms |

불뿜기의 20개 emitter는 원본 한 notify가 `FX_Prj_01`, `FX_Prj_02` 두 소켓을 지정하므로 각 소켓의 독립 스트림으로 20개가 추가된다. 행을 늘려 서로 다른 원본 패턴으로 세지 않는다. 전체 73요소는 sprite 58, mesh 3, decal 8, AnimationTrail 2, point light 2이다. native material 38프로그램이 71개 렌더 요소를 연결하고 나머지 2개는 기존 typed point light다. 37개 서로 다른 원본 Material을 renderer family별로 38프로그램에 연결했다.

원본 source root는 `C:/LostArkExtract/LV_LUT_MIDNIGHTC_ED_20260829`이다. 원본 action JSON, Particle graph와 UPK tagged properties를 조인하고 Engine/EFGame/Core CDO 및 archetype을 병합했다. 소스에 없던 Spawn 값이나 불활성 FireWave notify를 활성 원본으로 만들어 넣지 않았다. 원본 첫 실제 LOD의 모듈 순서, raw distribution, burst, emitter duration/delay/loop, notify TRS와 파라미터를 유지한다. ParticleParameter는 각 notify의 실제 scalar/vector override를 대입한다.

생성 문서는 `Data/Effects/Authored/effect.kouku.gate1.4219877.full.restore.effect.json`과 `effect.kouku.gate1.4219801.full.restore.effect.json`이다. 두 문서를 `Data/Effects/EffectCatalog.json`의 DIRECT_AUTHORED_DOCUMENT와 `Client.vcxproj/.filters`의 96.DataFiles에 등록했다. native installer는 기존 `Effect_ArtistMaterial.h`의 2304~2341 프로그램 table만 변경한다. 기존 프로그램과 소비자 코드는 보존하며 재실행 시 변경이 없음을 확인했다.

## 부착·수명·특수 표현

소켓은 원본 UModel의 32개 socket property를 추출한 `out/KoukuGate1FullRestore20260911/source_socket_contract.json`을 사용한다. MidControl은 `b_wp_1`+(0.375,0,0)m, FX_Prj_01은 `b_wp_1`+(0.32,0,-0.15)m, FX_Prj_02는 `bip001-head`+(-0.03,-0.07,0)m이다. 소켓의 원본 회전도 유지한다. source root snapshot은 기존 -90° source basis 계약, follow는 원본 bone anchor 경로를 사용한다. actor CModel bone 소비자는 별도 통합 변경이다.

문서 전체 구간은 마지막 notify 시각 + source emitter active 구간 + 최대 particle lifetime으로 산출했다. 애니메이션 종료에서 남은 파티클을 절단하지 않는다. 불뿜기 두 번째 단계의 소스 offset 1.666667초는 보존했고, Composition의 정수 millisecond 1667과 0.333ms 차이가 있다.

내려치기 Trails는 원본 AnimNotify_Trails_303의 22개 baked edge sample을 사용한다. notify 재생 clamp는 0.2초이며 두 emitter의 원본 point lifetime은 0.7/0.5초다. 기존 baked-edge 경로에 native profile의 particle color/alpha/ColorScaleOverLife와 dynamic parameter 샘플을 연결했다. clamp 이후 새 점은 만들지 않고 이미 나온 점의 tail만 처리한다. 기존 발탄 분기는 유지한다. Root에 직접 연결되는 두 Trails에는 BossCatalog G1_SAYDON의 bodyModelPreScale 0.017을 source cm→m 0.01과 맞추기 위해 local scale 1.7을 적용했다. 원본 baked 좌표는 수정하지 않았으며 bone attachment의 1.7이 중복 적용되지 않는다.

불뿜기 AccelerationOverLifetime 1개 source module은 두 소켓 요소가 소비한다. `acceloverlife`를 normalized particle age로 기존 acceleration updater에서 평가하고 `balwaysinworldspace=true`를 유지한다. 새 실행기를 만들지 않았다.

Decal은 원본 TypeData의 near/far, DefaultSize, roll, yaw-only를 기존 ground projector에 투영한다. 깊이는 `(far-near)*0.01m`, 중심 이동은 `-(near+far)*0.005m`이고 기존 projector의 ray는 -Y다. 원본 roll은 투영 평면 회전, bone root의 yaw만 orientation에 적용하되 원점은 전체 bone transform으로 계산한다. 셰이더의 정규화 localY를 cm depth로 복원하는 식은 `(near+far)/2 - localY*(far-near)`이다. 이 ground projector 좌표 변환은 재구성 adapter이며 원본 EF native 실행 oracle에 의한 source-exact 증명은 아니다.

내려치기 Decal02의 Spawn/Size는 외부 패키지 참조다. BFX_HIGH_00의 Spawn11/Rotation15, FX_BS_00의 Size11, FX_BS_04의 Location16을 정확한 object path로 가져왔다. 처음에는 선택한 PS package만 읽어 이 참조를 놓쳤고 no-Spawn으로 오판했으나, 실제 C++ codec probe에서 다른 emitter의 cardinality 오류를 계기로 원본 참조를 전수 확인하여 바로잡았다. 잘못 추가한 단일 projector 생성 adapter는 제거했다. 현재 문서는 원본 Spawn/Burst/Size 모듈을 그대로 사용한다.

외부 모듈 closure는 FX_BS_00/02/04/05/07, BFX_LOW_01, BFX_HIGH_00의 7개 패키지와 35개 추가 export를 읽는다. 이 보강은 9개 runtime 요소에 영향을 준다. 53개 base emitter의 첫 LOD에서 raw non-null Required/Modules/TypeData/Spawn 참조 수와 순서를 모두 생성 moduleOrder와 비교하여 누락 0건을 확인했다. 모듈·분포 참조를 해소하지 못하면 생성기가 즉시 거절한다. 추가 Dynamic 모듈은 native 2309/2310이 원래 선택한 ParticleDynamicParameter VF와 일치하며 renderer shape와 전체 window는 바뀌지 않았다.

Point light는 source PointLightComponent의 radius 200cm, brightness 10, white base color와 source initial color (3,1.5,0.5)를 기존 typed light에 연결한다. 광원은 파티클 초기 크기를 반경으로 임의 대체하지 않는다. 원본 light는 Burst 1개, continuous rate 0, particle lifetime 0.5초이므로 typed light의 표시 구간도 0.5초로 투영하고 추가된 ColorScaleOverLife를 같은 normalized lifetime으로 평가한다. Source Size=400cm를 PointLightComponent radius=200cm로 변환하는 EF native light update oracle은 없는 상태여서 해당 반경은 원본 component의 2m를 유지했다.

## Resources와 검증

`Effect/KoukuSaydon/FullRestore/Textures`에 원본 DDS 2개와 원본 RGBA TGA를 무압축 DDS로 변환한 1개를 설치했다. 나머지는 같은 sourceObjectPath에 대응하는 기존 Resources의 DDS를 재사용했다. TGA 변환은 base mip RGBA byte parity를 확인했다. `Meshes`에는 원본 mesh 3개를 등록했다. cylinder 2개는 기존 cook를 geometry identity 확인 후 재사용, plane은 기존 ModelAssetConverter geometry cooker를 사용했다. Plane의 UV1/vertex color는 UModel glTF에 존재하지 않으며 raw UPK vertex-buffer별 독립 oracle 확인은 하지 않았다.

실행한 검증:

- 두 생성 Python의 py_compile PASS.
- V1 attachment, native sprite option, material color space, module override validator 두 문서 PASS. v15 baked-edge extension validator는 해당하는 내려치기 문서 PASS.
- Codec의 71개 distribution capability 쌍과 실제 particle/decal 모듈의 분포집합을 비교하여 누락/추가/중복 property 0건. 금지된 source-only parameter binding 필드가 없다.
- 두 문서의 모든 참조 Resources 존재 및 안전한 상대 경로 검사 PASS. Catalog ID 중복 0, 변경 project/filter XML parse PASS.
- native table installer 재실행 changed=False. 관련 git diff --check PASS.
- 최종 Product Debug build PASS. Publisher 결과는 root 통합 RESULT에 기록한다. Client/UI와 사용자의 시각 검증은 USER_PENDING이다.

원본 occurrence와 생성 결과의 상세 정본은 `out/KoukuGate1FullRestore20260911`의 source_notifies/source_occurrences/source_module_inputs/runtime_occurrences/projection/installed_validation JSON이다. 원본 값 전체는 기존 추출본을 정본으로 유지한다. Resources 바이너리는 Git에 추가하지 않았고 기존 대규모 변경을 stage/commit하지 않았다.


실제 product C++ codec probe의 최초 검사에서 Python 구조 검사가 잡지 못한 두 저장 오류를 확인하여 수정했다. Light는 source material이 없으므로 기존 typed light와 동일한 `effect.standard`를 쓰며 profile은 비활성이다. Baked history의 `sourceEndTimeSeconds`는 clip의 절대 EndTime 1.616498초가 아니라 마지막 sample의 상대 시간 0.2666664초여야 한다. 원본 절대 시간은 추출 evidence에 보존하고 relative interval을 로더 계약에 맞췄다. C++ 허용 조건은 완화하지 않았다. 동일 product CPU probe 재검증은 통합 작업에서 수행한다.

Decal TypeData의 editor/tick/CDO 설정은 raw source evidence에 보존하고 portable 문서에는 기존 projector가 소비하는 near/far, LOD validity, always-update와 실제 회전에 필요한 roll 숫자만 전달한다. `rotation.degrees.roll`은 이번 Playback 소비자와 대응되는 저장 계약이며 별도 범위 검증 대상으로 연결한다.

최종 trail 검증은 단순 Stage_Document가 아니라 Catalog의 `Create_DocumentOwnedRuntimeProjection` → immutable resource preparation → `Stage_PrevalidatedVisualProgramDocument` 경로를 포함한다. 두 Trails의 SourcePresentation은 기존 v15 계약에 맞게 enabled/reconstructed와 안정적인 profile·source identity를 지정한다. 이를 거치지 않은 CPU probe에서 나온 일반 trail point는 baked-edge 복원 증거로 사용하지 않는다.

최종 product CPU 검증 PASS: 내려치기C는 실제 v15 document-owned projection=true, native 16개와 baked Trails 2개 각각 EdgePairs≥2를 확인했고, 45개 시점의 Seek/finite 검사가 통과했다. 불뿜기는 native 55개와 point light 2개, 58개 시점 검사가 통과했다. 원본 source slot/root의 수치 입력을 사용한 CPU 검증이며 CModel 화면 재현·GPU 픽셀·사용자 visual fidelity의 PASS를 의미하지 않는다. 검증 정본은 `out/KoukuSourceAnchors20260911/cpu_probe_result.json`이다.
