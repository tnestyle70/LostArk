# Guardian 그랜드 피날레 용 문양 정렬 결과

## G00. 원인과 보존한 편집

용 문양 DDS는 `fx_j_symbol_ddk_01_cl`이며 차징 clip1에서 emitter52/3/6/2가 함께 사용한다. 사용자가 저장한 fill3만 `followEmitterAxisRotation=true`와 수동 TRS를 사용했고 다른 겹은 기존 방향과 배치에 남아 있었다. 공용 renderer의 회전 실패가 아니라 겹마다 다른 저작 설정이 원인이었다.

설치본은 수동 뿔03(`kouku.49150.e3c543e9ec6c981056b0`), 복제04(`authored.copy.kouku.49150.e3c543e9ec6c981056b0.1`), fill3를 JSON element 전체의 원문 bytes까지 보존한다. element 추가·삭제·재정렬은 없다. Grand Finale 외에는 승인된 Revenge Spear와 Blaze Step Local Space 변경만 함께 설치했다.

## G01. 설치본에 반영한 값

clip1 완성·전이 문양52/6/2와 clip2 시작 문양30/15를 저장된 fill3의 TRS에 맞췄다. 같은 source location, pivot(.5,.9), StartRotation -.25turn, StartSize185×280cm 및 활성 SizeLife(1.2,1,1)를 유지하며 회전 추종과 localSpace를 사용한다. fill3의 회전값은 기존(-90,0,0)을 유지한다. 사용자가 조정한 기준을 바꾸는 추가 전역 yaw나 shader 회전은 넣지 않았다.

fill3의 ColorScaleOverLife R은 native4138의 main UV 이동에 사용된다. 원본 delay .25초, lifetime1.5초, normalized curve scale1.05263162에서 완성 시점은1.674999944초다. 이 곡선은 보존했고 완성·전이52/6/2의 emitter delay를1.675초로 맞췄다. 완성52 alpha scale만 원본 최고값.600000024 상수로 변경하여 최초 particle alpha .72를 만든다. 이 source 곡선 변경은 `authoredModuleOverrides=true`로 표시했다. 6/2의 원본 flash 곡선과 source 수명은 유지한다. 지연 변경은 입자 잔존 시간도 옮기므로 원본 타이밍 복원으로 기록하지 않는다.

clip2의 완성 문양30/15는 생성0초·수명.4초·공격 시각을 보존한다. fx_j_normal_03 계열54/24는 뿔03,53/25는 복제04의 배치에 맞췄다. 양의 Z 쪽 source를 음의 Z 쪽 뿔에서 복제한 위치에 맞추므로53/25에 local(0,0,-2)의 회전·scale 결과를 위치 보정했다. source 위치 분포와 색·크기·방출 곡선은 보존한다.

## G02. 검증 증거

`out/GuardianGrandFinale20260930/candidate-review.json`, `candidate-review.txt`와 `codec-probe.log`에 범위 검사와 실제 제품 Codec 실행 결과를 기록했다. 두 후보 Load와 resource boundary12개는 통과했다. 변경은 clip1의7요소,clip2의2요소뿐이며 모든 다른 필드는 strict allowlist 검사로 보존을 확인했다. 공식 common per-document validator5개는 두 문서에서 통과했고 참조 Resources 누락은0이다.

공식 v15 추가 검사는 clip2의 기존 baked-edge history에서 `end=clamp=lastSample=.20000000298023224`를 거부한다. 같은 원본에도 동일하게 실패하고 후보는 runtimeExtensions를 변경하지 않았다. 실제 native Codec은 이 endpoint를 허용한다. 따라서 공식 전체 검증 성공으로 기록하지 않는다.

실제 설치 GuardianKnight.wmodel의285개 bone, ddk_sk_grandfinale_02의52ticks/30Hz와 제품 source bone import-scale normalization을 사용한 Playback/최종 sprite geometry 검사를 실행했다. raw bone의0.01을 제품 크기로 오인하지 않도록 정규화 전후를 분리했다. clip1의1.7초 시점에서 full52/fill3/full6/transition2의 중심·법선·크기·정점이 일치했고 full52 최초 alpha .72를 확인했다. 카메라는 고정 수치 fixture이며 GPU draw나 사용자 화면 확인을 수행한 것은 아니다.

추가 기하 검증은 `geometry-candidate-summary.json`에 기록했다. 원본·clip1후보·clip2후보 세 실행은 모두 exit0이며 기록 행은1680/1680/88이다. fill3와 뿔03/04의 보호 기록66개는 동일했다. 네 용 겹36회 비교와 clip2 두 겹의 identity/실제 본2회 비교에서 중심·크기·법선·right/up·정점 오차는0이었다. clip2는 grandfinale_03(71ticks/30Hz)의 실제 본을 사용했다. 두 glyph만 남긴 메모리 fixture의 미참조 baked history는 제거했으며 제품 후보 파일은 변경하지 않았다.

60Hz fixed step에서1.68초까지는 완성 겹이 없고1.69초 표본에서 fill의 정규화 나이.955555와 완성 겹 최초 alpha .720000029가 함께 확인됐다. 이는1.675초 목표 이후 첫 simulation step의 양자화이며 별도 조기 표시를 추가하지 않았다. normal4는 원본 camera billboard와 source 곡선을 유지하고 원본/복제 측 local offset 보정식을 별도로 대조했다.

## G03. 설치·빌드 경계

사용자가 최신 저장본 기준 네 문서 전체 병합을 명시적으로 승인한 뒤 Grand Finale clip1/2, Revenge Spear clip1, Blaze Step clip2 Local Space 변경을 Data 원본에 설치했다. 최신 디스크 재확인, stable ID와 변경 필드 병합, SHA 확인, 백업과 원자 교체를 사용했다. 설치 receipt는 `out/GuardianGrandFinale20260930/installed-20260930T090036807332Z/receipt.json`이다. 기본 공격49000의 조사 대상은 설치하지 않았다.

설치 후 네 실제 Data 경로를 다시 읽어 JSON parse, asset ID, 검증 후보와의 전체 bytes/SHA 일치, 대상 네 파일의 `git diff --check`를 확인했다. 실제 제품 Codec은 네 문서 failures0/exit0, resource boundary12개 PASS였다. 증거는 같은 설치 디렉터리의 `installed-validation.json`, `installed-codec.log`, `installed-diff-check.log`다. 추가로 현재 Git diff에 나타나는 Guardian 이펙트8문서 전체도 JSON parse와 실제 Codec Load failures0/exit0를 확인했고 읽기 전후 SHA가 모두 같았다. 이 추가 검증은 `all-modified-guardian-validation.json`과 `all-modified-guardian-codec.log`에 기록했다. 검증 과정에서 사용자 수동 편집 파일은 변경하지 않았다.

EffectCatalog/Authored가 직접 제품 입력이므로 별도 Effect publish는 필요 없다. 이번 변경에는 C++·shader·Resources 신규 파일이 없다. 실행 중 도구의 Reload나 memory draft 갱신을 자동 수행하지 않았으며 설치 검증을 GPU 화면 확인으로 기록하지 않는다. 전체 Server/Client owner publish와 Debug/Release Product 빌드는 완료했다. 빌드 receipt는 `out/BuildPipeline/runs/20260930T091228557Z-debug-product.json`, `20260930T091359049Z-release-product.json`이며 둘 다 PASS다. 게시 후에도 설치4문서 및 수동 편집8문서 SHA가 그대로임을 `out/FinalRaidGuardian20260930/post-publish-effects.json`으로 확인했다.

## G04. 후속 확인과 Local Space 설치

기본 공격49000 clip2의 선택 Sprite Particle03은 .6~.8초에 존재하고 screenshot의1.2초에는 원본과 저장본 모두 수명이 끝났다. 별개로 저장본의 source-axis 회전 추종 false→true 때문에 실제 입자 법선이 수평에서 수직으로 변했고, 위치가(-.5,.7,.3)에서(0,0,0)으로 바뀌었다. Mesh Particle02의 TRS 변경과 보조 wind mesh 한 개 삭제도 확인했다. 실제 UI 편집 순서를 추측하거나 GPU 가림 여부를 단정하지 않았다. `out/GuardianMissingSlash20260930/investigation-result.json`에 actual Codec/Playback/설치 CModel 근거를 기록했고 source는 변경하지 않았다. 사용자는 이후 검격을 찾았고 보인다고 확인했으므로 복구를 적용하지 않는다.

이어 사용자가 현재 열린 블레이즈 스텝49260 clip2의 모든 입자 Local Space를 끄도록 범위를 확정했다. 해당 문서49요소 중42개 particle, true인39개 필드만 false로 바꾼 후보를 `candidate-localspace-49260-clip2.effect.json`에 준비한 뒤 전체 병합 승인에 따라 설치했다. 위치·회전·소켓 등 다른 필드는 보존했다. 설치 후42개 particle 모두 false이며39개 bool 외의 필드가 원본과 같음을 확인했다. 설치 manifest는 Grand Finale2개, Revenge1개, Local Space1개 총4개다.

Local Space 후보는 실제 native Codec1문서 failures0/exit0와 공식 common5개 검사를 통과했다. 39개 bool 외의 모든49요소 및 sourceRecipe·anchor·TRS·runtimeExtensions 보존도 확인했다. 이 direct-authored 경로는 Codec이 읽은 `Detail.Particle.bLocalSpace`를 Playback에서 직접 사용하므로 Required source literal의 buselocalspace를 추가로 변경하지 않는다. 별도의 v15 Python 검사는 기존 baked-history의 clamp=lastSample=.183333486 때문에 원본과 후보 모두 같은 실패를 보였다. 이를 신규 오류나 공식 전체 성공으로 기록하지 않는다.

## G05. 용 문양 3개 중심 기준90도 회전 설치

후속 범위 질문에 사용자는 용 문양3개만으로 확정했다. 대상은 clip1의 완성52·flash6·transition2이며 차오르는3과 clip2는 제외한다. 현재 저장본의27요소 중 대상3개는 같은 중심을 사용하지만 fill3는 사용자가 별도로1.37m 이동한 상태다. 이 차이를 유지한다.

현재 세 요소 모두 axis-follow가 켜져 있어 소비 누락은 확인되지 않았다. Element 원점에서 source StartLocation1m와 pivot(.5,.9)의1.12m만큼 실제 quad 중심이 떨어진 것이 회전 시 이동 원인이다. 후보는 rotationDegrees를(-90,0,0)에서(0,90,-90)으로, position을(-1.60000026,-1.20637143,-.0740000159)에서(.51999974,.91362857,-.0740000159)로 바꾼다. 원본 pivot·source 회전·재질·크기·생성 시각은 보존한다.

`out/GuardianGrandFinale20260930/rotation-3/validation.json`의 actual Codec/Playback/final sprite 검사에서 회전 표본24개, 세 겹 일치8회, 보호된 다른 재생 표본774개를 확인했다. 실제 설치 GuardianKnight.wmodel의 b_effectworldzero와 ddk_sk_grandfinale_02를 사용했고 중심·법선·크기 보존 및 right→-up/up→right90도·정점 변환의 최대 오차는5.960001e-7이다. synthetic camera로 수치 geometry를 검사한 것이며 Client/UI·GPU 화면 확인은 아니다. 기존 probe의 내부 `aligned` 분기는4개를 fill3로 바꾸므로 이번 판정에는 `saved` 분기만 사용했다.

세 요소의 position/rotationDegrees6필드 외 원문 bytes가 같고 다른24개 element 전체가 보존된다. 공식 common5개, 참조 Resources 누락0, actual native Codec1문서 failures0 및 resource boundary12개도 통과했다. 제품 코드·shader 변경이 없어 재컴파일은 수행하지 않았다.

사용자가 `반영해줘`로 승인한 뒤 최신 디스크 SHA가 검증 기준본과 같음을 다시 확인하여 Data 원본1파일을 백업·원자 교체했다. 설치 receipt는 `out/GuardianGrandFinale20260930/rotation-3/installed-20260930T094352487714Z/receipt.json`이다. 설치본 SHA256은 `58e0a03c977ed2230875c96b8ef8ae4d2d175178638be53eb7cd0c6037084387`이며 검증 후보와 일치한다. 실제 설치 경로의 JSON parse·native Codec failures0·resource boundary12개·`git diff --check`를 통과했다. 같은 설치 폴더의 `validation.json`과 `codec.log`에 기록했다.

이 direct authored Effect는 Data 원본을 직접 소비하므로 별도 publish·C++/shader 재빌드는 필요 없다. 실행 중 도구의 메모리는 자동 변경하지 않았다. 사용자가 해당 문서에서 `Load Saved` 후 `Restart Preview`로 확인한다. 파일 설치·수치 검증은 완료했으며 Client/UI·GPU 화면과 최종 사용자 시각 판정은 별개다.

## G06. 추가 저장 보존 요청 뒤 최신 디스크 재검증

사용자가 추가 저장한 편집의 보존을 요청한 뒤, 파일 저장 시각이 2026-09-30 18:43:52 KST인 Grand Finale49150 clip1의 현재 원문 SHA256 `58e0a03c977ed2230875c96b8ef8ae4d2d175178638be53eb7cd0c6037084387`을 다시 확인했다. 이 값이 현재 최신 입력이다. G03 최초 설치 후보의 `0af991f8a63244c6997d9c36bc39ba767181d027f652f30c6327ffca0a9656d3`과 당시 설치·게시 후 검증은 역사 기록으로 유지하며, 이전 후보를 현재 파일 위에 재적용하지 않았다. G05의 세 용 문양 회전과 그 이전 사용자 편집을 함께 보존한다.

최초 설치 후보와 비교하면 복제3개 추가·요소1개 삭제로25개에서27개가 되었고, 기존6개 요소의 위치·회전10필드가 실질적으로 달라졌다. 정확한 JSON 비교에서는9개 요소16필드 차이가 있으나 나머지6필드는1e-6 이내의 float 저장 정밀도 차이다. 완성·전이3개 delay의1.675→1.67499995도 이 정밀도 차이에 해당한다. 이 비교는 현재 변경을 분류한 기록이며 사용자 동작의 순서를 추정하거나 현재 튜닝을 되돌리는 근거로 사용하지 않는다.

현재 Data 실제 경로로 native Codec을 실행하여1문서 failures0/exit0와 resource boundary12개 PASS를 확인했고 JSON parse도 통과했다. 읽기 전후 SHA는 모두 최신 `58e0a03c…4387`로 동일했다. 증거는 `out/FinalRaidGuardian20260930/guardian-latest-user-save.json`, `guardian-latest-edit-validation.json`, `guardian-latest-codec.log`다. 검증 과정에서 제품 원본을 수정하지 않았으며 최신 튜닝의 GPU 화면 판정은 수행하지 않았다.

팀 ZIP에는 이 최신 저장본을 포함하는 것을 기준으로 삼는다. 실제 포함 여부와 패키지 해시는 통합 담당의 최종 ZIP 해시 receipt에서 별도로 확인하며, 이 재검증 기록 시점에는 ZIP 생성·검증 완료를 주장하지 않는다.

## G07. 화면 후속: 차오르는 문양·완성 고정·뿔 주변 높이

사용자가 차오르는 문양도 정렬하고, 완성·찍기 문양5개 모두 Local Space를 끄며,
뿔 주변 Sprite 전체를0.1m 낮추도록 확정했다. clip1의 emitter3는 이미 회전한52와
position `(0.51999974,0.91362857,-0.0740000159)`, rotationDegrees `(0,90,-90)`을
일치시켰다. 차오르는 UV 진행·시작0.25초·수명1.5초·크기와 Local Space ON은 유지했다.
이전 사용자 위치와의1.37m 차이를 유지하던 G05 범위를 이번 동일 위치 요청으로 보완했다.

clip1의52/6/2와 clip2의30/15는 detail.particle.localSpace만 false로 바꿨다.
본 attachment follow와 sprite axis-follow는 유지하며, 생성 시점의 본 basis를 캡처한
기존 world particle 경로가 이후 위치·방향을 고정한다. source Required provenance,
색·수명·재질·생성 시각은 보존했다. clip2의 position/rotation에는 추가 변경이 없다.

뿔 주변은 skintrail54/53/24/25의 원본4개·복제3개와 chargingcontrol69/70/66/5를
합친11개다. 검정뿐 아니라 청록 동반 면도 포함한다. 뿔 본체64·복제64 두 요소,
용 문양, notify022 무기, screenPost/light는 높이 조정 대상에서 제외했다.

### 본 좌표와 실제 높이 보정

최초 Element Y-0.1 후보는 설치 본을 사용한 최종 world 변위 검사에서 수평으로 움직였다.
제품의 PlayableCharacterAssetService는 CModel에 `Scale(.0001)*Ry(-90)`를 전달한다.
이전 수치 fixture의 Scale만 사용한 입력도 이 제품 yaw를 포함하도록 교정했다.
Y yaw 자체가 높이축을 바꾸지는 않으므로 최초 Y-0.1이 높이 조정에 실패한 결론은 동일하다.

실제 정규화된 b_effectworldzero의 local Z는 world-Y를 향한다.11개 요소의 Y를 원래
저장값으로 복구하고 local Z를0.1 증가시켰다. 실제 화면 높이를0.1m 내리기 위한 좌표
환산이며 X와 Y, 회전·분포·속도·크기·시각은 원래 값이다. 같은 작업의 최초 Y 후보와
그 패키지는 out 감사 폴더에 보존했으며 최종 전달본으로 사용하지 않는다.

### 검증과 최종 설치

| 검사 | 실제 결과 |
|---|---|
| 최종 허용 변경 | fill3 위치·회전2필드, Local Space5개, 주변11개 높이; 나머지 보존 |
| 후보/설치본 JSON·공통 validator5개·참조 Resources | PASS, 누락0 |
| 실제 native Codec Load/Stage |2문서 failures0, resource boundary12개 PASS |
| 고정 부모에서 fill3와 완성3개 중심·면 일치 |12표본 PASS |
| 생성 후 owner `(3,-2,5)` 이동에도 완성·찍기5개 위치·방향·정점 고정 |34표본 PASS |
| fill3의 owner 이동 추종 |4표본 PASS |
| 기존 뿔2개 불변 |60표본 PASS |
| 주변11개 실제 world `(0,-0.1,0)` 이동·색/크기/수명 유지 | PASS |
| 제품 preTransform와 clip1/2 본 전체 구간, owner yaw0/90/180/270 |420+572검사 PASS, 축 변환 최대오차2.5332e-8 |
| 정렬·추종 비교 최대오차 |2.4e-7 |
| Client/UI 실행·실제 GPU 화면 | 미실행, 사용자 최종 화면 확인 영역 |

clip1 27요소, clip2 89요소의 stable ID와 순서를 보존했다. 최초18필드 후보에 독립 검토를
수행했고, 실제 높이 보완은11개의 Y 복구·Z 변경 외 원문 bytes가 같음을 확인했다.
설치는 매번 최신 SHA 검사·백업·원자 교체를 사용했다.
최종 clip1은 `50f4b80b60e4c10904701cfc53ceb30a435a0b5265db66a2f99af4976b699e29`,
clip2는 `644f39d6a8d0735e41a51dc3ea138e081fdc0916514788f5722d3316829d2665`다.

증거는 `out/GuardianGrandFinale20260930/fill-impact-height`의
`candidate-validation.json`, `height-install-manifest.json`, `product-geometry-validation.json`,
`clip*-product-geometry.jsonl/log`, `installed-20260930T102835394370Z/receipt.json`,
`installed-20260930T103423282861Z/receipt.json`이다. G05의 상대 회전 검사와 이번 제품
preTransform·world 높이 검사를 구분하며, 최종 절대 축 판단에는 이번 보완 검사를 사용한다.

direct-authored Effect2문서만 변경했다. C++·shader·Resources·publisher·게임 EXE 재빌드는
필요 없다. 실행 중 도구의 메모리와 Product 캐시는 자동 갱신하지 않았다.

공식 build_portable.py로 기존 Release Product 성공 영수증
`out/BuildPipeline/runs/20260930T100434924Z-release-product.json`과 동일 게임 바이너리를
재사용해 `C:/Users/user/Desktop/LostArk-Verification-20260930-GuardianFixed.zip`을 생성했다.
게임 재컴파일·Client/Server 실행은 하지 않았고 launcher만 다시 컴파일했다.
최종 ZIP은168005673 bytes, SHA256
`d21bb63d83a2b24d48377f48e1e46f39a232194ededc394e63943aad746880a2`다.
launcher preflight·ZIP CRC·전 파일 manifest hash를 통과했으며 최종 Guardian2문서의
ZIP/현재 source bytes 일치와 기존 발탄 검격 수정 hash 보존을 별도 확인했다.
Resources는 기존처럼 외부 입력이다. 과거 Verification ZIP을 덮어쓰지 않았다.
증거는 같은 out 폴더의 `package-final-build.log`, `final-package-verification.json`,
`final-package-builder-receipt.json`, `final-installed-validation.json`이다.
