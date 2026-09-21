# Complete Play WORLD 및 빙고 컷씬 재생 복구

## G05. 실제 원인과 수정

사용자 첫 오류는 ending.kouku1이 없는 이전 WORLD snapshot을 사용한 상태였다. Client PID79804의 아레나 활성11:20:00/Product2029 로드11:20:01 뒤 WORLD2145가11:21:47 게시됐다. 현재 게시본의 ending5개 및 전체 WORLD 참조106개(고유68개)는 정상 resolve된다. 새 준비에서 idle WORLD를 공식 parse/validate/commit 경로로 읽는다. 실행 중 base sequence는 명시적으로 거부하고 이전 문서를 보존한다. 같은 Area의 Loader AREA_LEAF snapshot은 유지한다. 완료된 base hold는 기존 Reload 계약에 따라 정리되며 별도 child cue/draft는 교체하지 않는다.

BINGO handoff whitelist 누락과 P10 MAP SOUND3개 거부가 행을 격리해0ms/0stage를 만들었다. source codec·Product occurrence reader·Python publisher를 현재 저장본과 연결했고 unknown gate, 음향의 follow/bone/world/emission 오용은 계속 거부한다. 연결된 Sequence Complete Play는 Server plan을 사용하며, Bingo Loop와 일반 컷씬 Play의 의미를 버튼/tooltip에 표시한다.

## 자동 검증 및 빌드

- 실제 WorldSequence native reader: published2145, objects466/templates282/instances339; ending5개, arrow8개, bomb scale 참조 PASS(exit0).
- 실제 최신 Composition reader 원본 Sequence156:47/47. P9 52042ms/1stage/5WORLD/20presentation, P10 26322ms/1stage/1WORLD/9presentation, 격리 없음. expansion·roundtrip·unknown gate 및 잘못된 MAP SOUND5종 거부 PASS.
- 실제 Product Read_Occurrence 함수 원문과 실제 DataJson/types:34/34(원본29행+거부5종). Python 실제 consumer8검사 PASS.
- WorldSequencePlayer/KoukuSaydonPresentationPlayer 전체 TU scratch 컴파일 PASS.
- 사용자가 빌드한 Debug Client11:35:54, 변경 Codec OBJ11:35:47/Player OBJ11:35:52/WorldPlayer OBJ11:35:46을 확인했다. 모두 최종 source 수정 이후다. root Product runner 재시도는 사용자가 이미 새 Client/Server를 실행해 output guard에서 중단됐으며 중복 빌드·프로세스 종료를 하지 않았다.
- Source/data hash, 로그는 out/KoukuBingoPlaybackFix20260921/reader 및 playback-result.json. 데이터/Resources 설치나 재게시 없이 재생 admission 코드를 수정했다.

## G06. 최종 엔딩 얼굴 본 단위와 설치

Character 모델 경로는 정상이다. SCENE01B ending baker가 원본 SkelControl의 cm 이동값에 BASIS만 적용해, scale100 armature 아래의 metre 단위 본 translation을 100배 크게 구웠다. 사용자가 제공한 자막 구간은31.208~36.458초의 saydon2다. Saydon 턱 최대 delta0.5를0.005, Kouku 입2.8을0.028로 교정했다. 가방과 눈 등 같은 원본 translation control도 함께 교정했다.

`build_bingo_ending_actors.py`는 실제 rpct00/rpcz00 armature scale100을 검증한 뒤 translation에0.01을 한 번 적용한다. 기존 Character 두 WModel에서 ending4클립의 해당 position key bytes만 교체했다. geometry/material/skeleton과 다른350개 animation, 회전·scale·key 시간은 그대로 보존했다. 원본 control와 기존 bake 전 구간 대조 및 단위/거부8검사 PASS다.

실제 현재 Engine CModel과 WARP device로 새/기존 Character를 로드해 ending4클립의5,896개 시점,12,782,528개 본 행렬 성분을 샘플했다. 모두 finite이며 바뀐 pose가 실제 소비된다. 창 생성과 draw는 없고 화면 모양을 승인한 검사가 아니다. 근거는 `out/KoukuBingoMouthFix20260921/candidate-receipt.json`, `generator-checks.json`, `native/model-probe.log`다.

현재 설치본과 `C:/Users/user/Desktop/GBResources`의 두 WModel을 각각 최신 hash 확인·백업·원자 교체했다. 양쪽 결과 SHA는 Saydon `d4d10a7334e1965085a5559173968bafc110ca22e85373c9e03563fa3af3c0f0`, Kouku `2b58981a38498b6fb627bc46d8e841572bab2f69e28bfb1b02ba572f84220fd8`이다.

## G07. 앵콜 유리 로드와 두 씬 전체 의존성

실제 EffectFailure 로그 PID75004의 spark0 실패는 `Effect authored-v15 root is invalid`다. spark0·spark 두 파일만 version15로 표기됐지만 v15 필수 runtimeExtensions가 없었다. 통합 staging이11:06:22의 잘못된 후보를 복사한 뒤 원래 후보가11:06:59에 v13으로 수정·검증됐고,11:08:46 설치는 갱신되지 않은 staging hash를 사용했다. publish 누락이나 shader 실패가 아니다. 현재 generator는 정상 v13을 상속하므로 runtime admission을 완화하지 않았다.

두 파일의 version만13으로 복구하고 나머지 byte를 보존했다. 해당 변경은 native 검사에 통과한 후보와 설치 파일의 SHA를 일치시켰다. Kouku 문서722개에서 같은 root 결함은 이 두 개뿐이다. 근거는 `out/KoukuEncoreGlassFix20260921/{field-patch,inventory,staging-root-cause}.json`이다.

P9/P10의 V1 이펙트6개와 V2 암전, 설치 모델의 필요한 named clip5개를 현재 실제 codec와 CPU stage로 검사했다. 원본은 두 spark만 실패했고 수정 후보32/32 PASS다. 참조 resource81개, camera13개, sound event5개와 WAV 참조도 모두 resolve된다. 최종 설치 후 후보 override 없이 live 경로로 다시 실행한32/32 검사도 PASS이며 Sequence156/World2145와81파일 누락0을 재확인했다. GPU 렌더링·최종 음향 청취와 구분한다. 근거는 `out/KoukuBingoPlaybackFix20260921/dependencies`다.

## 최종 빌드와 반영 경계

최종 Debug Product Build는 Engine/Shared/Server/Client 전체 PASS다. 이미 사용자 빌드가 최신이어서 OBJ/PCH/CSO/binary 재작성은0개였고 필요한 deploy와 runtime 기본 검사까지 완료했다. 정본 receipt는 `out/BuildPipeline/runs/20260921T024811395Z-debug-product.json`이다. 이전 output guard 실패와 구분한다. 이번 수정은 authored effect와 Character Resources를 직접 소비하므로 추가 publish가 필요하지 않다.

설치6파일(실행 Character2, 전달 Character2, effect JSON2)의 백업·입력/출력 SHA와 transaction 결과는 `out/KoukuBingoPlaybackFix20260921/install/receipt.json`에 있다. Sequence/Pattern 저작 저장본·배치·시간·카메라를 변경하거나 사용자의 메모리 draft를 Reload하지 않았다.

## 사용자 실행과 남은 항목

사용자가 이전 최종엔딩 재생 화면을 제공해 Sequence 연결·재생을 확인했고, 그때 관찰된 턱 변형의 교정 파일을 이번에 설치했다. 최종 앵콜/엔딩 화면·음향, 전체 Server Complete Play와 실제 프레임 시간은 사용자 판정 전이다. Client/UI를 자율 실행하지 않았다. 다음 새 Client 실행은 설치된 교정 모델과 effect를 읽으며 다시 빌드할 필요는 없다.

화살표8track의 resource·transform·shader 검증과 빙고 뒤집기 준비 재사용은 기존 해당 RESULT를 따른다. 뒤집기 모델은 animated=false이고 animationTracks가 없어 이번 얼굴 bake 단위 오류와 원인이 다르다. CPU 재사용 검사를 실제 무끊김 화면 성공으로 확대하지 않는다.
