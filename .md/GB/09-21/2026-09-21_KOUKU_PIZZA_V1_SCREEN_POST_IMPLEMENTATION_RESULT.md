# 쿠크 피자 V1 ScreenPost 구현 결과

## G00: 완료한 코드

기존 V1 enum 번호를 유지하며 `CHROMATIC_ABERRATION_RECONSTRUCTED_V1`과 `screen.chromatic-aberration.reconstructed.v1`을 추가했다. 선택형 `intensityLerp=false`, `intensityEnd=0`의 parse/serialize 및 finite/nonnegative 검증을 연결했다. 기존 Timing lifetime을 사용한 시작→종료 선형 보간을 적용하고 source dynamic override와 alpha-over-life 처리 순서는 유지했다. 색수차는 기존 Engine 프로필에 제출한다. Effect Tool의 기존 Apply/Save 상세 UI에서 프로필, 보간 여부, 종료 강도를 편집한다. 8개 기존 C++ 파일만 수정했으며 새 제품 파일·프로젝트 등록은 없다. UTF-8/BOM 상태와 CRLF를 보존했다.

## G01: 데이터 후보

`out/KoukuPizzaScreenPost20260921/effect.kouku.pizza.explosion.group.effect.json`은 원래 14개 요소와 문서값을 유지하고 2개 ScreenPost만 추가한다. stable ID는 `authored.pizza.explosion.screen-post.zoom-blur`, `authored.pizza.explosion.screen-post.chromatic-aberration`이다. 시작 강도는 3/5, 종료 강도 0, 보간 true, 지연 0, lifetime 0.5초다. sourceNode는 V2 donor 출처만 나타내며 sourceRecipe/sourcePresentation은 false다. 기존 bloom 1.29999995, TRS·시간·모델·재질 및 P25 occurrence는 변경하지 않았다. 후보는 부모 작업이 최신 저장본에 stable ID 기준으로 병합·반영한다. V2 정본 group과 leaf는 변경하지 않았다.

## G02: 실행한 검증

- scratch native 빌드에서 실제 codec/playback와 모든 참여 소비자를 새 Document 헤더로 재컴파일했다. Effect_Object와 UI 2개 TU를 포함한 38개 TU가 exit 0으로 컴파일됐다. 기존 EngineSDK 문자 인코딩 경고와 DirectXTK PDB 경고가 있으며 새 C++ error는 없다. 제품 전체 빌드/링크를 수행한 결과는 아니다.
- 실제 `CEffectDocumentCodec`, `CEffectPlayback`을 링크한 out 전용 probe가 34개 검사 전부 PASS, exit 0이었다. 후보 parse/drawable/roundtrip, 기존 enum 번호, 원래 14개와 문서값 직렬화 일치, 기존 frame의 요소 transform·입자/Trail/Light 수 유지가 통과했다.
- 0, 0.125, 0.25, 0.499초에서 V1 강도를 실제 V2 `SCREEN_POST_PARAMS::Evaluate_Intensity` 함수와 비교했다. 두 효과 모두 일치했고 0.5/0.75초에는 종료됐다.
- 선택 필드가 없는 기존 문서는 새 필드를 쓰지 않고 상수 강도를 유지했다. source dynamic=8과 alpha=0.25 fixture는 보간 유무 모두 기존 우선순위로 2를 출력했다. 종료값 음수/NaN/Inf를 거부하고 잘못된 bool parse 실패 시 이전 문서를 보존했다.
- 다른 에이전트가 8개 코드 diff, 기존 enum/token 순서, 실제 Engine 제출, UI Apply/Save 연결을 독립 검토하여 수정 필요사항 없음으로 확인했다. 변경 코드 및 문서의 `git diff --check`를 확인했다.

재현 산출물은 `out/KoukuPizzaScreenPost20260921/native/`의 `compile.rsp`, `build.cmd`, `probe.cpp`, `probe-build.cmd`, `run.ps1`, `run.log`, `receipt.json`이다. scratch 실행은 현재 `Client/Bin/Debug` DLL과 `LOSTARK_RESOURCE_ROOT=Client/Bin/Resources`를 사용한다. 실행 파일 위치에 따른 기본 Resources 추론에 의존하지 않는다.

## G03: 남은 경계

구현·CPU 검증 뒤 사용자가 저장 완료·현재 저장본 반영을 승인했다. 부모 작업은 최신 `Data/Effects/Authored/effect.kouku.pizza.explosion.group.effect.json`에 위 두 stable ID만 추가했다. 백업·교체 직전 hash 확인·원자 교체를 사용했고 원래 14개와 나머지 문서값이 그대로인지 반영 후 확인했다. 적용 해시는 후보와 같으며 `out/KoukuPizzaScreenPost20260921/applied.receipt.json`에 기록했다.

Client/UI는 실행하지 않았다. 이후 부모 작업에서 정본 Product Debug 빌드·링크·DLL 배치를 완료했으며 결과는 `out/BuildPipeline/runs/20260920T224103825Z-debug-product.json`의 PASS다. 새 Client.exe로 사용자가 재실행하고 화면을 확인하는 단계는 남아 있다. CPU 수치 일치는 GPU 표시 완료를 뜻하지 않는다. 별도의 Effect publisher는 없고 기존 `Data/Effects/Authored` 소비 경로를 유지한다.
