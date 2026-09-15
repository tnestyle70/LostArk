# 맵·발탄 원본 입력과 표현 복원 결과

## G00. 현재 상태

진행 중인 결과다. 사용자가 카메라 복구 성공을 확인했고, 실제
`C:/Users/user/Desktop/로스트아크_렌더링`의 비교 PNG 13장을 전부 열람했다.
이번 변경의 Client 실행·화면 캡처·visual PASS는 수행하지 않았다. 추가 재질·렌더링·오라의
통합 Product build와 사용자 화면 확인은 아래 검증 상태와 구분한다.

우선순위는 발탄 전투 공간/본체, Character Select, 베른과 발탄 이펙트, 쿠크다.
같은 `pattern-3` 작업 디렉터리의 기존 Kouku/Effect 미커밋 변경을 보존한다.
카메라·렌더링의 앞 단계는 09-14 대응 PLAN/RESULT를 따른다.

사용자의 "잠깐 끊고 갈까" 요청으로 현재 상태에서 작업을 일시 중단했다.
추가 조사·제품 수정·빌드를 시작하지 않는다. 재개 시 첫 작업은 아래 후속 증분 build와
휠윈드 실제 OBJ 검증이며, Character Select 중앙8개 MIC의 native shader 해석과 발탄 하늘525의
custom lighting 연결,528의 engine-owned 환경광 입력은 미해결 조사 지점으로 보존했다.

## G01. 카메라와 캐릭터 크기

| 맵 | 저장 수평 FOV @16:9 | 거리 | 구분 |
|---|---:|---:|---|
| Character Select | 50° | 16m | 원본 공통 isometric CDO |
| Bern | 55° | 16m | 사용자 비교값. Source baseline은50° |
| Valtan | 55° | 18m | 원본 PS 카메라 영역 |
| Kouku | 50° | 19m | 원본1관문 영역. 2·3관문은 시험 적용 |

FOV 아래 Character size와 Reset size를 구현했다. 기존 catalog presentation scale에
0.25~4배를 곱하며 body/equipment/socket의 같은 presentation root에 적용한다.
네 Level의 생성·class 교체·camera profile 적용이 이를 소비한다. Server Transform,
충돌·공격 반경을 수정하지 않는다. source/before camera preset은 크기 입력을 보존한다.

기존 v1 JSON의 optional characterSizeMultiplier는 생략 시1이며 새 저장은 값을 포함한다.
숫자 범위·외부 파일 변경·잘못된 문서에서 기존 draft/파일을 보존한다. 1은 기존 catalog 기준이며
원작 게임이 추가로 적용하는 최종 actor scale을 확정한 값이 아니다.

현재 ArenaCameraProfile.cpp를 별도 컴파일해 실제 저장·로드 경로96검사 PASS.
근거: `out/FullMapRestoration20260915/camera-profile-check.log`.

## G02. 발탄 전투 공간

원본 StaticMeshComponent effective MIC와 native suffix를 대조해 바닥·바위443배치의
재질 입력을 연결했다. 기존7개에서436개를 추가했다. source MIC rock02 252,
rock04 27, rock05 164이며 source static set과 vertex COLOR/UV1/RNM을 구분했다.
동일 이름의 서로 다른 package texture는 별도 Resources 상대 경로로 보존한다.

원본 shadow234개는 DOM GUID `4ba587b9fa985e4b91c324a665a0ef33`에 연결된다.
SDF G8의 전체 native mip payload와 component 좌표를 운반한다. 원본으로 확정되지 않은
shadow transfer width0.05는 PROJECT_ADAPTER 경계다. 기존22 point light의 baked GUID와
해당 stone 입력은 교집합이 없으므로 광원 전체를 임의 비활성화하지 않았다.

중앙 Deploy A/B는 원본 geometry에서 각각25,819/34,306정점과2/3 UV 채널 및 정점 색을
회수했다. 두 모델의 바닥/균열 material slot을 BossCatalog의 named surface에 연결했다.
source crack은 원본 static option에 따라 sourceFlags133이며 별도 MIC emissive는 없다.
기존 중복 emissive overlay를 native surface에 다시 더하지 않는다.

ActorCatalog는 해당 정적 surface row를 CMapAssetCatalog의 동일 parser로 검증하고,
DeployPropObject는 기존 MapAssetRenderUtils/CModel/CMaterial을 소비한다. placement에
속하는 bakedLighting과 비지원 draw/cull은 actor row에서 거부한다. 별도 모델 경로를 추가하지 않았다.

새 geometry의 접선 교차 검증에서 UE→glTF의 반사 부호 누락을 찾아 설치 전에 수습했다.
glTF W=−native, 최종 runtime W=native가 맞다. 새75 geometry의152,609정점 W를 교정했고
나머지 position/N/UV/T.xyz/COLOR/index는 전량 보존했다. 기존7 geometry는 원본과 일치하여 유지했다.
근거는 `Valtan/native-tangent-basis-audit.json`, `existing-seven-tangent-audit.json`,
`tangent-repair-install.json`이며 모두 현재 out/FullMapRestoration20260915 아래에 있다.

MapAssetCatalog.cpp 최소 컴파일과 실제 새 Deploy4개 row/거절 시 기존값 보존 등20검사 PASS.
맵 Area Validate/Publish/Check PASS. 난간은 원본20,440정점과4개 material slot을 연결했고,
PS export527 구름은 source forward program59와 별도 MIC variant를 연결했다.
현재 map catalog717, material446, placement lighting444개다. PS export525/528 하늘의
원본 shader와 lighting channel은 추가 조사 중이며 전체 맵 완료가 아니다.

## G03. 발탄 본체·크기·이펙트

정상·유령·갑옷·도끼는 실제 설치 WModel 정점, mesh offset, skeleton hierarchy,
CModel preScale과 socket basis를 계산했다. 정상 bind 높이는 약3.000266m,
유령은2.863072m다. 서로 다른 파일의 raw 단위와 preScale을 임의 통일하지 않는다.
원본 NPC ModelSize140의 최종 소비자는 미확정이다. 이후 사용자가1.4배가 맞다고 명시해
정상/유령 BossCatalog presentationScale을1.4로 적용했다. native CPU 소비자 입증과 사용자
확정 배율을 구분한다. ClientReplication의 정상/유령 desc가 이를 읽으며 body/armor/weapon과
기본 오라의 동일 owner root에 적용한다.
캐릭터도 bind pose와 실제 서 있는 animation/actor scale을 구분한다.

유령 program84의 source master는 BLEND_Translucent다. NONBLEND ordered coverage에서
제외하고 기존 native character forward pass10에 연결했다. source varying/광원/안개/bloom을
사용한다. 원본 post-render depth와 approximate sort까지 동일 구현한 것은 아니다.

LookInfo 기본 particle은 normal default07, ghost default08이다. 정상 척추1부착,
유령 척추·양손·양팔5부착을 ActorCatalog.defaultParticles와 실제 본 이름/TRS로 연결했다.
새2 asset의8 emitter에서 원본 loops0을 보존했다. 기존 Effect_Playback의 owner-sustained
옵션은 이 boss 기본 오라에만 사용한다. 매프레임 전체 Seek/주기 reset을 제거하고 기존 고정 step
Update로 재생하며 죽음·숨김·pool 반환·normal/ghost 교체·release에서 handle을 정리한다.
새 native material2372만 추가했고 기존 공유 material/함수 변경을 보존했다.

실제 Product OBJ39개를 링크한30초 probe에서 움직이고 회전하는 root를 사용했다.
정상5 emitter와 유령3 emitter가20초 뒤에도 모두 발생하며 peak/capacity는 각각25/32,
12/19다. 동일 문서의 유한 재생 종료, loop1 입력의 지속 옵션 거절, 재설정 시 옵션 초기화도 PASS.
이 검사는 catalog/codec/playback CPU 경로이며 실제 CValtan lifecycle과 GPU 화면 판정은 아니다.
근거: `ValtanActor/aura_product_obj_probe.receipt.json`.

갑옷 색상 조사는 LookInfo의 body/armor/axe8개 slot과 MIC override를 대조했고, 원본 UPK에서
다시 추출한26개 texture의 base mip RGBA가 설치 DDS와 전부 일치했다. 원본 텍스처 자체에
황갈색 성분이 있으므로 색을 임의 제거하지 않았다. 이후 사용자가 원작도 금색이 맞다고 정정했고
비교 근거를 `발탄아레나_비교2_FOV55.png`라고 지정했다. 해당 이미지를 다시 열람했다.
금색 자체는 결함 조사에서 제외하며 갑옷의 원본 texture/MIC 색상 값을 제거하거나 무채색으로
변경한 작업은 없다. 최종 반사 밝기·청록 조명·안개·오라의 일치는 사용자 화면 비교가 남아 있다.
source action의 PawnMaterialParam/PawnMaterialChange/부위 분리는 full restore의 별도 범위다.
회오리420633 stage2/4의 활성 TransColor notify는 이번 재질 연결만으로 복원되지 않는다.

휠윈드420633의 기존9개 element 중8개 material에 source native 연결을 반영했다.
신규5개 프로그램과3개 distortion companion을 추가했고 기존1,188개 native 함수와
1,143개 material row 및 해당 effect의 material 외 필드가 보존됨을 대조했다.
이 추가 변경은 아래 첫 Product build 이후이므로 후속 빌드와 실제 OBJ 검증이 남아 있다.
high-jump는 아직 원본 native material 연결이 빠진 부분이 있다.
진입, 2페이즈 점프/붉은 하늘, 휠윈드, 도끼 변화, 에테르, 모든 패턴 occurrence는 후속 복원 중이다.
인벤토리 수집만으로 제품 연결 또는 화면 복원 완료로 표시하지 않는다.

## G04. 맵별 렌더링과 Benchmark

네 맵 WorldInfo/CDO/chain/volume의 활성 override를 resolve했다. 이름에 epic이 있는 chain도
실제 tone enum은 UE3 customizable이며 기존 Hable과 같은 함수가 아니다. native CPU packing과
원본 DXBC로 ToneScale/Range/Toe, highlights/midtones/shadows/colorize/desaturation 및 LUT 순서를 확인했다.

optional quality/region sourcePostProcess를 기존 RenderingProfileService와 Engine quality에
연결했다. source profile에서 이전 display desaturation을 중복 적용하지 않는다.
활성 LUT가 없는 경우 neutral 입력을 사용하고 override=false의 serialized LUT 이름을 켜지 않는다.
발탄 LUT는 원본 PF_A8R8G8B8, sRGB=false,256×16,16³의 실제 pixels다.

RenderingProfiles는22개/revision43으로 게시했다. 네 기존 base ID에 원본 입력을 반영하고
각 맵 before/source 별도 profile을 보존했다. Bern5, Valtan2 convex part, Kouku11 환경 영역을
원본 범위·priority·활성 값으로 연결했다. Valtan은 원본 안개/DOM 입력과 LUT를 사용한다.
runtime SH로 입증되지 않은 Lightmass 환경색을 runtime ambient로 임의 대체하지 않았다.

Benchmark는 네 맵 Before / Restored source profile / Return to entry를 제공한다.
source tone과 LUT도 비교 조건 fingerprint에 포함하며 기존 소유권·외부 변경 보존을 유지한다.
이 비교가 DOF, light shaft, 환경 particle, 물·폭포까지 자동 복원하는 것은 아니다.

LUTBlend의 원본 intermediate format은 A8R8G8B8이다. float CPU atlas 비교와8bit GPU
출력을 구분한다. 원본 DXBC immediate 0x322bcc77은1e-8이며 disassembly의 표시0을
literal0으로 옮기면 어두운 영역의 LUT quantization이 달라진다. 현재 기존 Deferred 경로에
GPU LUT bake를 기존 Deferred에 연결했다. 기존 pass0~27을 보존하고 원본 FLUT<1>/<2>에
대응하는28/29를 추가했다. 최종100조건/409,600 RGBpixels에서 LUT와 tone/lookup 최대오차0,
실패0, 비유한0이다. 실제20고유 map/region grading,16 stress,4 LUT crossfade,60 tone조건을
포함한다. 실제 ScopedSourceLutState 본문의 성공/early-return 실패 시 GPU state 복구와
전체 fx_5_0 effect 최소 컴파일도 PASS. Client 화면 비교는 아니다.

## G05. Character Select와 쿠크·베른의 남은 범위

사용자는 비교 화면이 현재 로스트아크에서 직접 촬영한 화면이라고 확인했다.
Character Select의 설치 원본 LV_LOBBY_CLASSSELECT_SL00을 새로 읽어803개 component와
55개 unique mesh를 다시 추출했다. placement property error와 unresolved placement는0이며
현재 배치 ID의 누락·추가도0이다. 전체 object 중 별도 native class2개는 generic tagged parser
밖이므로 전체 object 해석 성공으로 확대하지 않는다. package SHA256은
`55191fbeb0ebf2228c030a7807e7db287144ead3182a7921809f84f529c2d33c`다.

새55개 geometry와 현재63개 catalog variant의 모든 material submesh에서 방향 있는
삼각형 Position/Normal/UV0가 일치했다.6개 variant의 raw index 차이는 Assimp 정점 병합이며
semantic triangle mismatch는0이다. 중앙 FLOOR12/BRIDGE01E도 동일하다. UV1의 누락과
material/lightmap의 최종 출력은 이 기본 geometry 일치와 별개의 미해결 범위다.

중앙 원판2개와 다리8개의 원본/현재 world triangle을301×301 grid로 대조했다.
겹침28,448 sample에서 기존 Y 보정 때문에 위아래 관계가 바뀐 sample은0이다.
이는 CPU geometry 비교이며 alpha/culling/동적 표시를 고려한 화면 판정은 아니다.
반면 두 EFMotionStaticMeshActor의 현재 transform이 원점/항등인 확정 결함을 발견했다.
원본 위치 약(-785.61,-142.859,183.92),(-758.376,-142.859,211.255)m와 원본 회전·배율로
authoring을 수정하고 기존 publisher의 Publish/Check를 통과했다. 변경2행 외801행과 기존
Y9건은 byte 보존했다. 근거: `CharacterSelect/motion-placement-restoration.receipt.json`.
장식의 회전 운동은 정적 위치 복구와 별도 범위다.

중앙 문양 차이의 최종 원인은 미확정이다. 관련14 package와 별도 SL01 조사에서 교체할
중앙 asset의 확정 근거는 아직 없다. SL00에 독립 Decal class는 없지만 ParticleSystemComponent
10개와 동적 actor가 있으며 별도 표시 조건까지 조사해야 한다. UE4 혼합·리뉴얼·추출 불가능을
현재 원인으로 단정하지 않는다. 근거: `CharacterSelect/fresh-summary.json`,
`fresh-geometry-comparison.json`, `fresh-placement-comparison.json`, `central-layer-geometry.json`.

쿠크의 원본 PS/SL source placement2,951개는 위치/배율과 일치했다. 최대 위치 차이는
0.00008m, scale차이는4.055e-7이다. 추가 SCENE01A79개는 지원 Actor resolver 밖이므로
이 비교에 포함하지 않았다. 원형 바닥 FLOOR08/FLOOR08A의 실제1026정점·3186index는 새
원본 추출과 vertex/index payload가 전부 일치했고 교체하지 않았다. 원본이없는 COLOR0를
새로 만들어 넣지 않았다. 최종 preScale 후 최대 좌표 오차는9.03e-7m다.
근거: `Kouku/source-placement-scale.json`, `floor-native-parity.json`.

따라서 비교한 쿠크 원형 바닥을 일괄 확대할 근거는 없다. 2·3관문 카메라, 부착 actor 크기,
원본 광원/RNM과 실제 viewport 조건을 별도 대조한다. 베른 도서관·물·나무·폭포·창문 빛,
쿠크 전체 map material은 원본 후처리 연결과 별도로 남아 있다.

## G06. 빌드와 사용자 확인

- 카메라 실제 parser/save96검사: PASS.
- 공유 static surface parser 실제20검사와 최소 컴파일: PASS.
- Valtan Area/RenderingProfiles publisher: PASS.
- Debug Product build: PASS,421,324ms. 근거 `out/BuildPipeline/runs/20260914T171400350Z-debug-product.json`.
- 위 build 이후 휠윈드 native material 추가: 소스 반영, 후속 증분 build 대기.
- GPU LUT 원본 DXBC 대조100조건/409,600 RGBpixels 및 state 복구: PASS.
- 기본 오라의 실제 Product OBJ30초 지속 재생 probe: PASS. GPU/Client 화면 검사는 아니다.
- 이번 변경의 사용자 화면 판정: 대기. 카메라 복구 성공만 사용자가 확인했다.

build 완료 뒤 사용자가 Server/Client를 갱신해 F1 Player Follow Camera와 Rendering Workbench의
Benchmark를 확인한다. Client나 UI를 에이전트가 실행하거나 visual PASS를 대신 기록하지 않는다.
Git 제외 Resources의 추가 경로는 위 범위와 대응 설치 기록을 함께 공유해야 한다.
