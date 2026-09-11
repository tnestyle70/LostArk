# 쿠크세이튼 1관문 두 짤 패턴 V1 연결 결과

## 구현 범위

2026-09-11. 기존 Effect Tool V1, source recipe, CModel/CMaterial, Action Workbench의 Composition Append 경로에 연결한다. 새로운 이펙트 런타임이나 보스 AI를 추가하지 않는다. Client/UI 자율 실행과 화면 캡처는 하지 않았다.

| 패턴 | 원본 애니메이션 | V1 문서 | 재생 창 |
|---|---|---|---|
| 4219877 내려치기 C | RPCT05 `Att_Battle_1_01`, 2000 ms | `effect.kouku.gate1.4219877.full.restore` | 6582 ms |
| 4219801 불뿜기 | RPCT05 `Att_Battle_14_01` 1667 ms + `Att_Battle_14_02` 3167 ms | `effect.kouku.gate1.4219801.full.restore` | 11075 ms |

Composition revision 323의 pattern 29·30 / presentation resource 53·54다. 원본 공격 뒤 남는 이펙트 수명은 실제 `Idle_Battle_1`의 LOOP_TO_WINDOW로 연결한다. 수동 AUDITION_ONLY, selectionWeight 0이고 기존 PRODUCT의 MECHANIC 분류 계약을 유지한다. 원본의 피해·투사체 서버 스킬 XML을 확보한 것은 아니므로 원작 서버 판정 복구나 자동 AI 편입으로 기록하지 않는다.

## 실제 소비자

- All Effects의 KoukuSaydon 분류에서 두 DIRECT_AUTHORED_DOCUMENT를 열 수 있다. `.restore`가 캐릭터 skill 복원으로 오인되지 않도록 기존 Sequencer의 typed Kouku 선택을 사용한다.
- Action Workbench의 Saydon/1관문 두 패턴은 기존 V1 Append resource binding을 소비한다. V1 Play/Solo도 Composition의 유일 패턴과 실제 CNpc/CModel preview를 선택한다.
- 같은 문서의 Play All을 다시 누르면 모델과 FX를 함께 0ms부터 재시작한다. 문서를 연 직후 첫 Element Solo도 Kouku 모델을 준비한 뒤 root를 조회한다. 잘못된 문서나 모호한 패턴 연결은 기존 선택을 보존한다.
- 원본 53 first-LOD emitter occurrence(내려치기16, 불뿜기37)를 유지했다. FX_Prj_01/FX_Prj_02 복수 소켓 발생을 분리하면 runtime 73 elements(16+57)다. 광원·메시·데칼·baked AnimationTrail을 포함한다.
- 원본 소켓32개를 읽고 `b_wp_1`, `bip001-head` 등 실제 모델 본을 전달한다. 원본 cm→m 변환과 BossCatalog modelPreScale 0.017의 비율을 보존한다. first socket을 잃던 UModel wrapper 파서를 교정했다.
- 파생 patternbindings의 optional sourceAnchorAnimations를 projector와 C++ reader에 함께 연결했다. 60 Hz 과거 샘플은 원본 animation timeline과 기존 CModel pose sampler를 사용한다. 제자리 수동 패턴 두 개는 frozen actor root와 움직이는 원본 bone timeline을 사용한다. 이동하는 모든 보스의 과거 root motion을 해결한 것은 아니다.

## 원본 재질과 모듈

37 material path에 대해 실제 VF별 38 source color program을 회수했다. 기존 native carrier에 2304..2341 범위만 추가했고 원래 character program은 유지한다. 텍스처 없는 analytic glow와 Disto05의 source binding도 원본 shader bytecode에서 확인했다. Disto05의 별도 accumulation PS `56fd4335bcbc2843a7577f32fab7249e`를 추가해 양/음 XYZW 출력을 기존 signed UV distortion target으로 변환한다.

V1 기존 파티클 모듈 evaluator를 재사용하며, 실제 원본에 있는 AccelerationOverLifetime의 `acceloverlife` 분포만 기존 갱신 경로에 추가한다. 궤적은 원본 baked edge22개와 원본 color/alpha/dynamic 및 point lifetime을 사용한다. Root에 직접 연결되는 두 궤적의 local scale은 BossCatalog의 0.017/0.01=1.7을 적용한다. 광원의 초기 색과 밝기, 원본 burst 1개와 particle lifetime 0.5초도 실제 typed light에 전달한다.

외부 패키지 7개에서 빠져 있던 35개 module export를 추가로 해석했다. 53개 base emitter의 raw 첫 LOD 참조 571개를 수와 순서까지 대조했고, 두 소켓 발생을 전개한 문서는 73요소·794모듈·1,168분포다. C++ 검사에서 발견한 외부 참조 누락, baked history 상대 종료 시각, typed light template, decal 전용 literal 계약을 교정했다. 원본 tick/editor 메타데이터는 추출 기록에 보존하며 portable 문서에는 실제 소비되는 속성을 전달한다. 데칼 roll은 실제 Playback 소비자와 함께 정확한 Kouku native profile에 한해 finite NUMBER와 3,600도 경계를 검사한다.

내려치기 문서는 기존 authored v15의 document-owned runtime projection을 사용한다. 두 baked trail은 이 경로의 SourcePresentation 활성 상태·profile·source identity를 연결한다. 일반 Stage_Document만 실행하면 확인되지 않는 이 계약까지 실제 Catalog와 같은 factory 및 Playback stage로 검증한다.

원본 픽셀 계산과 프로젝트의 렌더 입력은 구분한다. lit decal의 UE skylight는 프로젝트 scene owner가 없어 기존 native model과 같은 committed scene ambient를 사용한다. 지면 receiver는 기존 depth projector다. fog/scene grading은 명시적인 프로젝트 adapter이며 원작 전체 광원·후처리를 재현했다는 뜻이 아니다. 원본 비활성 FireWave notify를 켜서 패턴 후보를 만들지 않았다.

두 문서가 실제 참조하는 Resources는 texture 67개와 mesh 3개, 총70개다. 기존 원본 매핑을 재사용하고 누락된 소수 texture/mesh를 `Effect/KoukuSaydon/FullRestore/`에 설치했다. 모든 참조 파일의 존재를 확인했다. 바이너리는 Git 추적하지 않는다. 실행에 필요한 실제 파일은 native material patch/geometry installation의 Resources-relative ID를 따른다.

## 입장 병목

별도 [입장 병목 결과](2026-09-11_KOUKU_ENTRY_STUTTER_IMPLEMENTATION_RESULT.md)에 기록했다. 선택 class의 Product effect cue 준비와 약190.8 MB의 Deploy prototype 준비를 Loader로 이동하고 Level activation의 중복 준비를 제거했다. 사용자 PC의 입장 뒤 10초 프레임 저하가 얼마나 개선됐는지는 아직 계측하지 않았다.

## 검증 상태

- 공식 Kouku publisher revision323: product, map.kakulsaydon, world.gameplay, gameplay.balance 4 domain PASS. 저장30 / 실행26 / 실행 stage200.
- source socket parser 2 tests, decal receiver normal contract 4 tests PASS.
- native shader extraction의 textureless 분기 포함49 tests PASS. 비어 있지 않은 기존 shader 검사와 실제 selectioncolor binding 검증을 유지했다.
- source module/distribution 및 material/attachment Python 구조 검증은 두 문서 PASS.
- mesh/particle/decal/trail FXC syntax compile PASS. 마지막 codec 및 반복 Play/첫 Solo 수정까지 포함한 Debug Product compile/link/deploy PASS. `out/BuildPipeline/runs/20260910T215447751Z-debug-product.json`: Engine/Shared/Server/Client 모두 PASS, 누락 runtime input 없음.
- 실제 Product C++ obj를 사용한 CPU 검사 PASS. 두 원본 문서 Parse, v15 document-owned projection 생성, immutable 준비, Playback Stage, 총103시점 Seek, 되감기, 필수 source slot 누락 거절, 잘못된 Parse에서 이전 출력 보존을 확인했다. 내려치기45시점은 파티클 최대44·원본 잔상2개·edge pair 최대38, 불뿜기58시점은 파티클 최대103·조명2개가 생성됐다. 모든 검사 transform은 유한했다. 사용한 root/source-slot matrix는 명시적인 합성 입력이며 실제 CModel 본이나 화면 fidelity 증거는 아니다.
- CPU 검사 정본은 `out/KoukuSourceAnchors20260911/cpu_probe_result.json`이다. 최종 두 문서 SHA256은 각각 `d6c2d56ccb4647bf25a925ba4bc879257c6b7b3e34d71f3daa00fa3ef2c51e3d`, `d2e861c79c1417082f4df556734fe03db77f77a5ce765f9998243f953ba59ca6`이며 검사 입력과 현재 파일이 일치한다.
- 변경 JSON/XML parse, 두 FxCompile과 project reference 구조, 전체 `git diff --check` PASS. 입장 및 projector 기존 테스트의 baseline 실패 경계는 각 RESULT에 분리해 기록했다.
- 사용자 Client 화면, 최종 색·크기·타이밍과 FPS 확인은 미실행이다. visual PASS가 아니다.

## 사용자 확인 경로

마지막 상태 조회에서 Server/Client 프로세스는 모두 실행 중이 아니었다. Visual Studio의 `Server + Client` profile을 Ctrl+F5로 실행한다. Lobby에서 KoukuSaydon에 입장한다. F1 → Effect Tool V1 → All Effects → KoukuSaydon에서 두 문서를 열고 Play All/Solo를 확인한다. 모델 공급자는 해당 아레나이므로 Play/Solo는 입장 후 사용한다. F1 → Action Workbench → Boss `Saydon` → `1관문`에서 `세이튼_내려치기C_FullRestore` 또는 `세이튼_불뿜기(원본0-1)_FullRestore`를 선택하고 `Complete Play (Server)`로 실제 연결을 확인한다. 기존 패턴이 선택된 상태와 구분한다.

마지막 확인 항목은 반복 Play All, 문서를 연 직후 Element Solo, 원본 공격 애니메이션에서 Idle로 넘어가는 동안의 잔여 이펙트, 입장 직후10초 프레임 상태다. Client를 에이전트가 실행하거나 화면을 캡처하지 않았다.
