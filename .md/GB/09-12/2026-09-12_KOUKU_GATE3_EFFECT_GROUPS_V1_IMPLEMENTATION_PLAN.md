# 쿠크·세이튼 전체 패턴·연출 이펙트와 Effect Tool V1 구현 계획

## G00. 현재 입력과 작업 경계

사용자의 후속 요청으로 범위를 쿠크·세이튼 1~3관문 전체 패턴과 입장·카드미로·기타
시퀀스 연출로 확장했다. 아래 쇼타임·마리오·화염은 첫 연결 단위이며 전체 작업의
종료 범위가 아니다. 기존 저장 ID는 유지하고 표시 이름을 `1관문_세이튼_패턴_이펙트`
형식으로 정리한다. 시퀀스는 실제 연출 부모 아래에 묶는다.

사용자가 제공한 여섯 이미지를 내려찍기 화염, 마리오 오망성·진입 포탈, 화염 인형,
쇼타임 경고 장판·폭발·기관총·탄체·공 착탄으로 나누어 원본 Action/Projectile/ParticleSystem과
대조한다. 원본 사진을 새 텍스처로 그리지 않고 설치된 게임의 추출 경로와 기존 native
material importer를 사용한다. 기존 무지개 복원의 source module, 실제 모델 앵커,
독립 V1 occurrence 경로를 확장한다.

작업 시작 브랜치는 `codex/character-select-200fps`이며 렌더링·성능 관련 미커밋 변경과
사용자가 만든 Composition의 3관문 `세이튼_쇼타임` 폴더/빈 패턴32가 존재한다.
다른 세션의 변경을 보존하고 자동 stage/commit하지 않는다. Client와 Server가 실행 중이므로
독립 out 경로에서 최소 컴파일하며 실행 중인 Client의 UI나 화면을 조작하지 않는다.

## G01. 원본 그룹과 실제 앵커

`Tools/EffectPipeline/build_kouku_showtime_restore.py`는 쇼타임 원본 PS의 경고, 폭발,
총 생성·발사, 조준 표시, 낙하·착탄을 독립 authored 문서로 구성한다.
`build_kouku_gate3_slam_mario_restore.py`는 화염분출, 마리오와 인형의 실제 원본
Action/stage/notify를 소비한다. 실제 매칭은 원본의 clip·source object·timer를 기준으로
확정하고 사용자 관찰과 원본 추론을 RESULT에서 구분한다.

새 문서는 `Data/Effects/Authored/effect.kouku.gate3.*.effect.json`에 놓는다.
원본 모듈의 relative timing, lifetime, local/world space와 source attachment를 보존한다.
그룹의 transform은 Append occurrence가 소유하고, 각 element는 그 안의 상대 transform을
소유한다. 총구·인형 입처럼 본을 요구하는 그룹은 실제 설치 모델의 bone/clip을 샘플링한다.
synthetic anchor가 finite라는 사실을 실제 부착 성공으로 기록하지 않는다.

## G02. V1 목록과 공용 재생 경로

`Effect_Tool.cpp/.h`, `Effect_Tool_Workspace.cpp`, `EffectAuthoringSequencer`의 기존 구현을
확장한다. All Effects의 KoukuSaydon을 관문·패턴·이펙트 그룹 트리로 표시하고 3관문 쇼타임
아래에서 전체 그룹 또는 요소를 선택해 재생·Append한다. V1의 Box Detail과 Composition
Resources 외부 창을 제거하되 다른 툴의 동일 창은 유지한다. 필요한 transform/anchor 편집은
선택한 그룹의 편집 흐름에 남긴다.

독립 그룹 편집·재생에 전체 보스 graph의 publish 또는 별도 Validate 버튼을 요구하지 않는다.
문서 parse, 잘못된 경로·ID 거절, source resource 준비와 실패 시 기존 문서/재생 보존은
기존 stage/commit 경로에서 수행한다. source bone이 있는 새 문서도 저장 Composition의
유일 패턴 존재 여부에 묶이지 않도록 기존 모델 공급자를 통해 실제 모델을 준비한다.

## G03. Catalog와 쇼타임 패턴 연결

`Data/Effects/EffectCatalog.json`에는 direct authored ID/상대 경로만 추가한다.
`Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json`은 전 관문 공용 정본이다.
기존 GATE3 쇼타임 폴더와 패턴의 사용자 변경을 보존하면서 신규 Effect resource를 등록하고
원본 clip 및 occurrence를 연결한다. 원본 전체 branch를 임의의 순차 timeline으로 만들지 않고
독립 Append·튜닝을 위한 각 그룹의 실제 범위를 유지한다.

새 native program은 기존 Kouku carrier의 미사용 범위에 추가하며 기존 프로그램을 삭제하거나
재번호하지 않는다. shader 설치는 기존 모든 source group을 함께 전달하는 append merge를
사용한다. `Client/Bin/Resources/Effect/KoukuSaydon`의 필요한 DDS/WModel만 설치하며 Git에
추적하지 않는다. 새 JSON은 Client project/filter의 `96.DataFiles` 아래 None으로 등록한다.
기존 C++ 파일을 확장하고 신규 C++ translation unit은 만들지 않는다.

## G04. 확인과 결과 기록

신규 JSON과 수정 XML parse, catalog ID/path 대응, 실제 Resources 존재, source clip/bone 및
element 시간·좌표를 확인한다. 수정 CPP와 shader의 최소 Debug 컴파일을 실행한다.
현재 실행 중인 Client와 다른 세션의 빌드를 간섭하지 않으며 제품 링크 여부는 RESULT에
명시한다. `git diff --check`와 대상 diff를 검토한다.

사용자는 빌드된 Client에서 F1 → Effect Tool V1 → All Effects → KoukuSaydon → 3관문 →
쇼타임으로 들어가 그룹을 선택하고 Effect Action Benchmark에 Append하여 위치·앵커·시간을
튜닝한다. 실제 버튼 입력, 장판→폭발의 순서와 색·크기·부착의 최종 판정은 사용자 확인으로
남긴다. 자동 검증과 사용자 화면 확인을 RESULT에서 분리한다.

## G05. 전 관문 원본과 트리 확장

기존 원본 목록의 349 Action, 직접 참조 ParticleSystem 327개와 간접 Projectile/NPC,
Scene/Matinee 참조를 조사 시작점으로 사용한다. 1,008개 패키지 후보를 전부 실제 패턴
사용으로 간주하지 않는다. 전체 직접·간접 참조를 따라 원본 발생·클립·부착을 읽고
현재 실행 carrier와 native material에 연결한다. 기존 손튜닝 문서는 덮어쓰지 않는다.

목록 구조는 기존 `Data/Effects/EffectResourceTree.json`의 nodes/references 정본을
사용한다. 관문 → 패턴 또는 연출 → 쿠크/세이튼 또는 연출 부모 → 기획자 패턴/구간명 →
Effect로 연결하며 source ID와 표시 이름을 분리한다. 트리는 organization만 읽고
목록 표시 때 전체 authored 문서나 보스 graph의 resource prepare를 실행하지 않는다.
새 Effect는 실제 작성된 문서만 참조하며 빈 placeholder나 미복원 항목을 복원 완료로
표시하지 않는다.

기존에 source graph만 있는 항목은 동일 importer를 통해 first LOD/module·상속값과
실제 texture/mesh/material을 연결한다. 필요한 새 native 프로그램은 실제 material/VF
동일성을 대조해 재사용하고 미사용 ID를 추가한다. 반복 패턴의 분기와 대체 stage는
임의로 한 연속 재생으로 이어붙이지 않는다. 연출은 원본 actor/Matinee toggle/transform과
부모 시간을 소비하며 원본 전체 logic/Server 패턴 구현과 이펙트 저작 복원을 구분한다.

## G06. 사용자 빌드 X3004 교정

사용자 빌드의 `g_EffectSceneDepthTexture` 미선언은 실제 `Shader_VtxAnimMeshBinary`의
MODEL_ONLY include에서 재현했다. native installer의 왜곡 companion에도 기본 함수와 같은
`!defined(ARTIST_NATIVE_MODEL_ONLY)` 조건을 유지한다. 모델용 texture 선언 순서를 바꿔
불필요한 일반 이펙트 함수를 모델 shader로 끌어들이지 않는다.

전체 cohort의 원본 CB2 선언이 5 또는 7행인 경우에도 기존 4행 passValues 배열을 생성한
범위 오류를 함께 교정한다. 원본으로 확인한 diffuse/specular override와 실제 render target
크기를 제공하고, 미확인 shader ID의 추가 pass 상수는 생성 단계에서 거절한다.
기존 generated shader와 생성 도구를 함께 수정하고 모델·이펙트 carrier를 실제 FXC로 확인한다.

## G07. 사용자 빌드의 왜곡 projection 미선언 교정

후속 `Client/Default/x64/Debug/Client.log`의 첫 오류는 `Shader_EffectKoukuNativeGroup2304.hlsli`의 `ArtistNative2697Distortion`에서 `projection`이 선언되지 않은 X3004다. 현재 group에서 같은 참조 누락을 가진 왜곡 함수12개를 확인했다. `Tools/EffectPipeline/generate_artist_native_runtime_shader.py`와 해당 generated group의 연결된 함수만 수정한다.

원본 PS의 CB1 투영 참조와 VS의 TEXCOORD5 의미를 대조해 입력 world position 및 projection을 함께 공급한다. 선언만 추가하고 clip position을 다시 투영하는 경로를 만들지 않는다. 이전 MODEL_ONLY guard와 원본 passValues 길이 교정, 다른 세션의 native 설치 변경은 보존한다. 실제 영향받는 carrier 및 모델 shader를 독립 out 경로의 FXC로 확인하고, 추가 컴파일 오류가 있으면 최초 오류와 구분해 같은 원인 범위에서 처리한다. 실행 중인 Visual Studio build·Client·Server를 종료하거나 제품 output을 동시에 덮지 않는다. 생성기 Python 구문, 변경 범위 diff와 실제 FXC 결과를 RESULT에 기록한다.

추가 검토에서 `ArtistNative3338`의 실제 열 번째 텍스처가 기존 source SRV 9개 상한과 맞지 않음을 확인했다. 기존 renderer와 screen-post snapshot의 배열·검증 mask·bind, 공통 및 모델 shader 선언, native helper 생성기를 함께 10개로 연결한다. 텍스처를 생략하거나 다른 슬롯으로 대신하지 않는다. 리본 왜곡의 실제 소비자인 Trail에는 world position 보간과 source projection 전달·바인딩을 추가하여 0 행렬 입력을 방지한다. 새 C++ 파일이나 프로젝트 등록은 필요하지 않으며, 수정 CPP의 최소 컴파일과 영향을 받는 FXC 소비자를 확인한다.

같은 변경 대상 `Effect_NativeScreenPostMaterial.cpp`의 단독 컴파일에서 Windows/Winsock include 순서 충돌이 재현되면 기존 `Client_Defines.h`를 먼저 포함해 프로젝트 공통 헤더 순서를 따른다. 이 수정은 해당 translation unit의 컴파일 차단 해소로 제한한다.
