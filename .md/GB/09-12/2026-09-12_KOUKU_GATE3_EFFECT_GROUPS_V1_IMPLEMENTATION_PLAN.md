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

## G08. 패턴별 독립 예고·폭발 그룹과 알비온 생성

2026-09-12 후속 요청으로 조사 전용 상태를 종료하고 확인한 원본을 반영한다. 사용자가
Sequencer에서 각 그룹의 시작 시간을 정하므로 예고와 폭발은 각각 0초부터 재생하는
독립 authored 문서로 만든다. 내부 입자 상대 시간과 같은 순간의 원본 방향 배치는 유지한다.
기존 패턴의 사용자 clip·시간·배치와 다른 세션의 렌더링·빌드 변경은 보존한다.

무지개 댄스는 RPCT05 Dance Aura의 cylinder/rainbow ring, Dance Light의 세 sprite와
기존 BOSS spotlight를 구분한다. 설치 모델에 없는 B_WP_3/4/5를 실제 본으로 위장하지
않으며 독립 저작 그룹은 명시적인 BOSS root 기준 배치를 사용한다. 소스 본 의존 문서와
root 저작 그룹의 역할을 이름·근거에서 구분한다.

칼날 댄스 계열은 RPCT05 4219804/4219805의 원형 1회·도넛 2회를 원본 크기로
각각 준비한다. 기존 쇼타임의 합성 warning.impact 문서는 별도 원형/안쪽 도넛/바깥
도넛의 예고·폭발 그룹으로 제공하며 두 액션 계열을 혼합하지 않는다. 알비온은
4219903의 파란 원형 예고와 ThunderStorm 폭발, 70도·11m 부채꼴의
0/90/180/270도 예고·폭발 그룹을 준비한다. 원본 notify의 int32 FRotator를 읽어
네 방향이 0도로 겹치지 않게 한다. 감전 buff의 붉은 반복 원형은 파란 장판으로 재사용하지 않는다.

십자 화염은 본체 화염파동과 투명 NPC 4222003의 실제 두 직각 효과를 사용한다.
원본 판정은 16m×2m 직사각형 두 개이며 폭발은 3.15초, 판정은 3.3초다.
광기의 불길 A/B/C는 각각의 평행·직각·대각 격자 배치와 예고/폭발을 구분한다.
기존 rainbow.grid 명칭의 격자와 무지개 댄스 원형 aura를 서로 다른 트리 부모로 정리한다.

변경은 기존 EffectPipeline 생성기 및 필요한 패턴별 생성기, Data/Effects/Authored,
EffectCatalog.json, EffectResourceTree.json, Composition의 presentationResources와
Client project/filter의 None 등록에서 이어진다. root 통합 단계만 공용 catalog/tree/
Composition/project 파일을 갱신하고 각 생성기는 담당 authored 문서만 만든다.

알비온의 플레이어 주변 생성은 기존 CombatObjectRuntime의 PER_ALIVE_PLAYER volley와
Server navigation 검증·원자적 commit, Shared spawn/snapshot/despawn을 재사용한다.
Kouku Composition의 typed logic 입력과 publisher, Server Kouku 실행, Client의 기존
CombatObjectProjectionRuntime/EffectPresentationService까지 연결한다. Client에서
플레이어 위치를 임의로 정하거나 판정을 실행하지 않는다. 예고와 폭발을 독립 저작한 뒤
실제 combat object의 원본 지연에 맞춘 재생 문서를 같은 정의가 소비하도록 구성한다.

새 C++ translation unit은 만들지 않고 기존 owner/consumer를 확장한다. 변경 CPP의
파일별 인코딩을 유지한다. 검증은 JSON/XML parse, 실제 참조 자산과 그룹 분리·방향·시간,
관련 publisher 및 Server 권위 생성·실패 보존 검사, 영향받는 CPP의 최소 컴파일을 수행한다.
공유 Client/Server와 다른 MSBuild는 중단하지 않고 제품 실행·화면 판정은 사용자에게 남긴다.

## G09. 백스탭 불뿜기·수직 화염링·분신 브레스

09-12 후속 첨부 이미지의 수직 주황색 링과 굵은 수평 화염포, 검붉은 분신 브레스를
세이튼 아래 별도 패턴 폴더로 연결한다. 이미지 안 자막은 작업 지시로 해석하지 않는다.
기존 원본 leaf와 설치 native 재질·DDS·WModel을 재사용하며 원본에서 확인한 배치와
사용자가 요청한 확대·공용 화염포 조합을 구분한다.

수직 링은 `Par_G_RPCT_05_FireRing_01_LOC_INT`의 실제 hoop mesh와 원본
Projectile 421982502를 기준으로 한다. 별도 `Sk_04_10/11` 화염링 소환은 이름만으로
같은 효과에 섞지 않는다. 백스탭/링의 화염포는 같은 원본·같은 확대값을 소비하게 하고
단독 화염포, 링, 링에서 분사하는 합성 그룹을 각각 제공한다. 분신의 붉은 브레스는
`Par_V_RPCT_Breath_01_LOC_INT`와 해당 Action notify를 대조해 크기·시간을 가져온다.
독립 그룹의 앞 방향을 +Z로 맞추고 위치·회전·크기는 기존 Append occurrence로 조절한다.
새 합성 본이나 새 renderer 경로를 만들지 않는다.

필요한 생성기는 `Tools/EffectPipeline`에서 source leaf를 조합하고 새 authored 문서만
new-or-equal로 설치한다. 기존 `install_kouku_effect_library.py --library-only`가
Catalog/EffectResourceTree/Composition presentationResources와 Client project/filter None을
등록한다. 사용자 Pattern 타임라인은 유지하며 생성 문서의 모든 element ID·provider 참조·
source material 연결을 보존한다. 신규 C++ 파일과 shader 프로그램은 필요하지 않다.

검증은 실제 Client codec/Playback의 독립 CPU 검사, 모델 geometry와 재생 transform의
수치 대조, Resources 존재, JSON/XML parse, 등록기 재실행과 변경 diff로 수행한다.
현재 다른 세션의 codec 변경과 공유 빌드는 별도로 구분하며 오래된 probe 통과를 최신
제품 검증으로 기록하지 않는다. 실제 Client 재생·크기·밀도·색·방향 판정은 사용자가 한다.

추가 UI 요청에 따라 Composition Resources → Effect의 Created Resources 목록 높이를
현재 값의 최소 3배로 늘린다. 실제 Workbench의 해당 child panel만 수정하고 기존 선택·Append·
스크롤 동작은 유지한다. 기존 C++ 인코딩을 보존하고 변경 translation unit을 격리 컴파일한다.
실행 중 Client/Server는 종료하지 않으며, 제품 바이너리 반영과 사용자 화면 확인을 구분한다.

## G10. V1 Effect의 MAP 배치와 Preview·Play·Save 일치

첨부 Timeline 오류는 `Requested root time has not been recorded`이며 저장 전 P37의
Effect occurrence에서 발생했다. 유효한 MAP·follow=false는 고정 pivot을 쓰지만 Workbench의
Anchor 선택은 edit에만 남고 기존 geometry staging은 위치·회전·크기만 복사한다. Save와
Preview/Play에서 이전 BOSS·follow=true가 남는 경로를 교정한다. Effect Append가 Resource의
defaultAnchorKind를 무시하는 누락도 같은 변경에서 연결한다.

Workbench의 기존 staged placement 경로에 Effect의 anchor·follow·bone·world 참조를 함께
보존한다. Collider의 미적용 Logic·anchor 편집과 timing·fade는 기존 명시 Apply 경계를 유지한다.
Resource 단독 Preview는 현재 선택한 보스 기준을 사용하고, MAP 기본값은 새 Timeline occurrence에
적용한다. MAP 좌표는 기존 PositionOffset의 절대 월드 미터 계약으로 저장하며 WORLD Object
참조와 혼동하지 않는다. 저장된 Pattern과 사용자의 미저장 draft를 외부에서 덮어쓰지 않는다.

MainApp은 단독 Effect Preview에 선택 Pattern의 actor·gate·stable boss placement를 전달한다.
원본 모델 metadata가 없는 오망성도 기존 single-member Bundle Preview의 CNpc/CModel 준비를
재사용해 선택한3관 세이튼 기준으로 보인다. 리소스 Preview actor를 플레이어 위치로 덮던 단계는
제거하고 기존 Level의 해당 boss 위치 resolve 결과를 유지한다. 새 애니메이션을 추측하지 않는다.

PresentationPlayer는 placement 변경으로 anchor가 바뀌면 이전 anchor로 재생을 계속하지 않고
해당 occurrence의 handle과 anchor cache만 갱신한다. V1/V2 모두 같은 배치 계약을 소비한다.
보스·bone-follow의 실제 과거 transform 누락을 고정값 fallback으로 숨기지 않는다. 고정 MAP과
정적으로 알려진 Preview root의 첫 비영시점 재생은 실제 소유 범위에서 검증한다.

기존 Composition save/projector 검사와 실제 codec/Playback·pivot history 검사를 재사용해
MAP Save/reload, Preview/Play 입력 일치, 중간 시점 시작·rewind·boss 이동 시 고정 위치를
확인한다. 필요한 C++ translation unit만 격리 컴파일하고 원본 인코딩·공유 변경을 보존한다.
Client/UI 자율 실행·캡처는 하지 않으며 제품 빌드와 사용자 최종 재생 확인을 별도로 기록한다.

복구된 P37 오망성은 MAP·follow=false가 맞더라도 PositionOffset [0,0,0]이면 3관 중앙이
아닌 월드 원점에서 재생된다. `Render_PresentationAnchor`의 MAP Effect 편집에
`Use Boss Spawn Position`을 추가한다. 현재 draft Area의 `Gameplay.world.json`을
`CWorldGameplayDocument`로 읽고 선택 Pattern의 stable boss placement를 정확히 찾아
BOSS 종류·actor profile·Gate의 기본 placement 일치를 검증한다. 성공한 월드 좌표만
edit.PositionOffset에 복사해 기존 placement staging·Preview·Save가 소비한다. 실패하면
기존 위치와 미저장 draft를 보존하고 오류 이유를 상태에 남긴다.

새 MAP Effect Append도 같은 조회를 재사용해 보스 스폰 위치를 초기값으로 정한다.
월드 source 누락·손상 또는 대상 불일치 시 원점으로 대신 추가하지 않고 오류를 보존하며
Append를 commit하지 않는다. 격리 검사는 실제 해당 BOSS의 World source를 fixture로 사용한다.
위치는 스폰을 한 번 복사한 절대 좌표이며 이후 boss 이동을 따라가지 않는다. World 문서·
실제 보스 Transform·Composition 저장 파일을 이 버튼에서 직접 변경하지 않는다.

## G11. 3관 KoukuSaydon 표시와 미저장 패턴 보존

추가 첨부 목록을 현재 Composition399와 대조했다. 화면의 마지막4개 신규 패턴은 파일에 없으므로
실행 중 Client의 draft를 보존해야 한다. 외부 Resource 등록으로 저장 기준본이 달라진 경우를
포함해 Save의 fresh/CAS 상태와 기존 복구 경로를 조사하며, Client 종료·Reload나 Composition
덮어쓰기를 하지 않는다. 저장 완료 여부는 실제 파일의 stable pattern ID·내용으로 판정한다.

재발 방지는 기존 Save_Atomic의 CAS를 유지하면서 외부 변경이 revision 증가와
presentationResources 끝의 신규 항목 추가뿐일 때만 수행한다. 기존 Resource·Pattern·기타
문서 내용과 순서가 기준본 그대로인지 비교하고, 신규 stable ID 충돌이 없거나 내용이 같을 때
현재 외부 Resource를 사용자 candidate에 합친 뒤 검증·원자 저장한다. 다른 외부 변경은 계속
거절하고 이전 파일과 draft를 보존한다. Save 버튼은 실패 후 재시도할 수 있게 하되 Publish와
Server Play의 freshness 요구는 유지한다. 현재 실행본의 미저장 복구와 새 코드 적용은 별개다.

3관의 현재 target은 `boss.kakulsaydon.g3.saydon`, actor는 `MN_RPCT_05`다. UI에서는 Gate3의
이 조합만 `KoukuSaydon`으로 표시해 Target Boss와 Pattern/Bundle 행을 맞춘다. 기존 저장 ID와
참고 Animation profile alias는 유지하며, 다른 관문의 Saydon 표시를 일괄 교체하지 않는다.

3관 catalog의 weaponModel은 null이다. 쿠크 머리 지팡이와 어깨 쿠크는 원본 mesh·NPC/Action
결합 근거와 설치 모델의 실제 본·크기를 확인한 뒤 기존 CModel·NPC 부착 경로로 연결한다.
사용자의 추가 확인에 따라 같은 지팡이를 드는 1관문 세이튼에도 WP05 장착을 적용한다.
1관문과3관문의 기존 body·게이트·배치 ID는 유지하고 어깨 파트의 관문별 표시 차이는 원본
LookInfo/몸체 입력을 확인한다. 서로 다른 역할의 WP08 head accessory를 어깨 대체로 추가하지 않는다.
리소스 추가는 원본과 설치물의 누락이 확인된 범위만 추출·변환하며, 원본3방향 화염의 실제
총구 socket과 같은 actor에서 소비되는지 함께 확인한다.

원본 `EFDLChar_MN_RPCT_07.MN_RPCT_07.loa`의 PartsMesh는
`WP_MN_RPCT_05_SK → WP_1_20`을 지정한다. body socket의 실제 본은 `b_wp_1`,
상대 위치·회전은 0, scale은 1이다. 기존 설치 지팡이와 `wp_mn_rpct_05_mi` 재질을 재사용하고
G1_SAYDON/G3_SAYDON catalog에 weapon preScale `0.01`, rotation `[-90,0,0]`을 연결한다. body의
preScale `0.017`과 본의 100배 basis를 포함한 최종 부착에서 idle·불뿜기14_01/14_02의
0초·0.5초 총6개 표본은 손 소켓과 무기 원점 오차0, basis 길이 `1.7 ± 0.000002`,
무기 원점 반경 `0.848671m → 1.442742m`다. 원점·반경만으로 방향을 판정하지 않는다.
원본 PSK와 설치 WModel의 전체15,172정점을 비교하면 rotation0의 Y/Z축은90도 어긋나며,
DirectX X축 pitch -90도에서 최대 정점 오차가 `0.287559m → 9.65e-7m`로 줄어든다.
이 geometry 변환을 기존 catalog preRotation으로 반영하고 같은 CModel·CNpc 부착 경로를 유지한다.

설치 몸체는5 submesh/168 bones이며 slot4의 `mn_rpcz_00_mi`가 어깨 쿠크다.
27,276개 정점은47개 `bip002` 본만 사용한다. idle0의 CPU skin과 body preScale을 적용한
bounds는 min `[-0.748430,2.300056,0.431305]`, max `[-0.236890,3.287906,1.030915]m`로
수축·누락되지 않는다. 어깨용 새 모델은 추가하지 않고 선택 actor context를 함께 교정한다.
이 수치 검사는 GPU 표시나 사용자의 시각 승인을 대신하지 않는다.

지팡이의 원본 애니메이션은 `wp_mn_rpct_05_sk.ao_att_battle_17_01` 한 개다.
기존 `Synchronize_SaydonHammerPose`의 일반/transition 양쪽에서 공통 clip lookup으로
`rpct00_att_battle_17_01`을 연결하고, 다른 clip은 실제 weapon rest pose를 사용한다.
RPCT06과 BINGO의 기존 모델·배율·회전 및 clip mapping은 유지한다. 기존 cpp 인코딩을
보존하며 변경 TU 격리 컴파일, lookup CPU 검사와 catalog JSON parse를 수행한다.

`WP_MN_RPCT_08_1_sk`는 `WP_3_1` 머리 소켓에 연결되는 별도2,614정점 부품이며
어깨의 `bip002`와 다르다. LookInfo 조건 필드의 표시 의미가 확정되기 전에는 어깨 복구를
위해 임의로 추가하지 않는다.

3방향 불뿜기는 원본 MN_RPCT_07 Action4219940의 첫 시전·발사 stage0/1을 독립 V1 group으로
등록한다. 기존 4219801의 두 방향 이펙트를 변경하지 않는다. 설치된 동일 emitter의 native 재질을
재사용하되 원본 notify를 다시 decode해 FX_Prj_01/02/03 각각의 부착, 어깨 발사 local offset,
parameter 기본값과 시간·크기를 투영한다. 원본 14_01/14_02 animation과 Gate3 preview metadata를
보존하고 패턴/세이튼 아래 등록한다. 기존 사용자 Timeline의 임의 자동 배치는 하지 않는다.

이 그룹의 source socket은 UE3 bone-local 좌표를 설치된 PSK/FBX 골격에 그대로 복사하지 않는다.
원본 UE→PSK mirror, PSK bind, 설치 WModel bind와 particle의 UE→Client 좌표 변환을 합성해
설치 골격의 socketLocalTransform으로 투영한다. 실제 정점·bind·애니메이션 수치로 확인한
MN_RPCT_05 범위에서만 생성기에 적용하고, 다른 모델의 전역 추출 규칙을 임의 변경하지 않는다.
총구 정렬은 socket origin뿐 아니라 원본 cue offset과 emitter의 최초 입자 위치·방향까지
같은 실제14_01/14_02 pose에서 비교한다. 어깨 입에 맞추려는 임의 본 대체나 offset은 넣지 않는다.

## G12. KoukuSaydon Arena 시작지점과 트리거 재시험

F1의 `1관문 - 세이튼` 위에 독립 `시작지점` 버튼을 추가한다. 기존 관문 목록과 index를 유지한다.
버튼은 같은 아레나 보스를 모두 제거하고 정본 `player.spawn.kakul.party01`로 요청 플레이어를
돌려보내며, 트리거와 시퀀스를 처음부터 재시험할 상태로 초기화한다.

기존 typed command sink와 `C2S_DEBUG_TELEPORT_TO_PLACEMENT` 요청·결과 계약을 확장한다.
Client가 위치를 하드코딩하거나 Transform을 직접 바꾸지 않는다. Server는 실제 spawn 정의,
navigation·높이·collision과 session/world를 검증한 후 이동·트리거 상태 및 같은 아레나 boss와
그 소유 연출을 정리한다. 검증 실패 시 기존 상태를 유지하며 오류를 기존 상태 UI에 표시한다.
기존 gate marker의 teleport 의미와 index, 다른 world의 오브젝트는 변경하지 않는다.

수정한 translation unit을 격리 컴파일하고 기존 요청·Server 결과 경로에서 정본 spawn 선택과
실패 보존을 확인한다. 실행 중 Client의 미저장 복구를 먼저 완료하며 Client/UI 실행·화면 판정은
사용자가 직접 한다.
## G14. 공통 불뿜기와 화염링 2배 구성 후보 (2026-09-14)

현재 지팡이 불뿜기 원본의 Fire_01/02 25요소를 공통 neutral 문서로 추출하는 기존 작업의 `copy_shared_firebreath_elements` 계약을 재사용한다. backstep.flame과 full, ring.flame의 기존 Sk_01 sprite11을 이 25요소로 교체하며 full의 ground decal3은 보존한다. 내부 본 attachment가 없는 shared yaw0/scale1.7을 그대로 사용하고 이전 화염의 yaw-90/scale2를 중복 적용하지 않는다.

ring과 ring.flame의 실제 FireRing_01 본체10요소만 바닥 원점 기준2배로 확대한다. 기존 중심1.1m와 source225cm 편심을 함께 확대하므로 Element position [0,1.1,1.575]는 [0,2.2,3.15], scale .7은1.4가 된다. 링 분출25의 origin은 [0,2.2,0]으로 함께 올리며 백스탭 분출은 기존 [0,1.1,0]이다. 별도 ring.end와 원본 추출 archive, 사용자 Composition/WorldSequence는 이 후보 생성기에서 수정하지 않는다.

기존 `build_kouku_backstep_flame_groups.py`에 `--shared-firebreath` current-authored 후보 경로를 추가한다. 기본 생성과 new-or-equal 설치 경로는 보존하며 새 경로는 out의 baseline/candidate/manifest만 작성한다. 기존 멤버/owner/transform이 기대와 다르면 현재 사용자 편집을 덮지 않고 거절한다. manifest는 실제 입력 hash와 asset별 destination/candidate/name/duration/default anchor를 전달하고 최종 guarded 설치는 상위 작업이 담당한다.

실제 Codec/Playback 저장·재로드·수명/finite·deterministic seek 및 설치된822개 hoop 정점의 전후 bounds/중심/바닥 접점을 대조한다. 그룹 구성과 공통25 payload, ground3/별도end/다른field 불변을 검사한다. 새 C++/프로젝트 항목, Client/UI 실행·캡처는 없다. 공굴리기 sourceaction4219866/4219910의 별도 FireBreath_01 library9는 사용자가 지정한 같은 stable asset ID로 공통25를 교체하고 `공굴리기_공통 불뿜기`로 표시한다. 기존 tree parent와 원본 archive/baseline은 보존한다. 이는 원본9의 재질 복원이 아닌 요청된 공통 외형 교체이며, 전체 공 탑승 동작이나24회 발사를 새로 구성하지 않는다.

## G15. 공통 불뿜기와 1·2·3·4 화염 파동 — 2026-09-14

현재 `staff.flame.full.restore`는 이름과 달리 action4219951의 백스텝 브레스 합성이다.
42요소 중 Fire_01/02 sprite25개가 사용자가 공통으로 쓰려는 화염이며, 나머지 지팡이·오라·
조명은 별개다. 현재 사용자 저장본은 보존하고 원본 표시명만 `불뿜기`로 바꾼다.
원본 lookupTable의 앞2개는 range header이며 XYZ 속도로 해석하지 않는다.
25개를 별도 root 이펙트 `effect.kouku.gate3.firebreath.shared`로 파생해 준비 지연을 제거한다.
실제 Playback이 이미 +Z로 변환한 분사 방향을 유지하고 head 기저의1.7배 크기를 별도 기록한다.
같은 파생 입력을 백스텝·화염링이 재사용하며 본체2배 확대는 G14의 링 중심 기준 변환이다.

화염 파동의 요청 입력은 Light `Par_M_Light_001`, 불기둥 `Par_L_RPCT_05_Sk_04_11_LOC_INT`,
폭발 바닥 `Par_G_RPCT_05_Wand_Decal_LOC_INT`다. 불기둥만 전체1.5배 확대한다.
원본 action4219820/4219948의 `rpct00_att_battle_10_02` 활성 WandDecal notify3104ms를
첫 폭발 기준으로 사용한다. 전조1904/2204/2504/2804ms 후 폭발3104/3404/3704/4004ms다.
삼각형10지점·행간300ms·전조1200ms는 원본 복원이 아니라 사용자 요청에 따른 저작 구성이다.
지점 간격은 확대 후 실제 입자/geometry 수치를 확인한 뒤 후보 manifest에 수치와 근거를 남긴다.

`Tools/EffectPipeline/build_kouku_flame_wave_groups.py`는 원점 전조·불기둥·바닥3종과
10지점 전체 미리보기를 out에 생성한다. 전체는 동일 BOSS 시작 pivot을 한 번 잡아 각 행의
전조와 폭발이 같은 바닥 지점을 공유한다. 편집용30box 후보는 별도 제공하되 서로 다른
시각의 BOSS pivot을 각자 고정하는 구성을 기본 패턴에 설치하지 않는다.

`Tools/KoukuSaydonPipeline/prepare_flame_unification.py`는 최신 저장본을 읽어 Catalog,
ResourceTree, Composition, Client project/filter 등록을 한 후보에 모은다. 기존P49의 이동·
애니메이션을 보존하고 새 화염 파동을 동작 시각에 맞춘다. P49 전체 수명은 바닥 잔광을 포함해11154ms로 늘린다. 같은 단일 동작을 세이튼·쿠크세이튼
각각 바로 재생할 수 있는 패턴을 추가한다. 기존P43은 백스텝 동작에 공통 불뿜기를 연결한다.
공굴리기는 정확한 원본/기존 소비자를 조사해 그 화염 부분만 교체한다. 이름이 유사하지만
다른 P40 십자화염폭발, 다른 세션의 화염 재질 수정과 사용자 타임라인은 보존한다.

후보 준비는 live Data를 쓰지 않는다. 저장·종료 확인 뒤 최신 입력 hash를 재검사하고,
대상 파일의 CAS가 모두 통과한 경우에만 백업을 남겨 적용한다. 실패하면 이미 교체한 파일을
원본으로 복구하고 오류를 보고한다. 변경한 Authoring을 정본 publisher로 검증·게시하며,
최종 Product 빌드에는 앞서 준비한 Shift 선택·앵커 그룹·Object preview 수정을 함께 포함한다.
JSON/XML parse, 실제 Codec/Playback, 회전·크기·동시 타이밍 수치와 보존 범위를 확인한다.
Client/UI는 실행하지 않는다. 사용자의 화면 확인과 원작 외형 승인은 RESULT에서 미검증으로
분리한다. 새 C++ 파일과 런타임 schema는 추가하지 않는다.


## G16. 십자 화염 폭발·3갈래 불뿜기의 공통 화염 적용

기존 공통 불뿜기25요소를 네 개의 현재 리소스에 파생해 적용한다. 십자 한 줄은 정·역방향50요소, 십자는 네 방향100요소, 전체는 여기에 기존 조명5요소를 유지한다. 3갈래는 기존 세 socket의 본 화염42요소를75요소로 교체하고 준비·예고·조명35요소를 보존한다. 각 요소의 시작 시각, 저장 박스, ID와 리소스 기본 수명은 유지한다. 빈 Pattern에 새로운 animation을 임의로 채우지 않는다.

`Tools/EffectPipeline/build_kouku_directional_shared_firebreath.py`는 stage만 수행하며, 현재 Authored와 공통 화염의 hash를 설치 manifest에 기록한다. 원본 `build_kouku_albion_cross_groups.py`의 선택된 십자3문서와 `build_kouku_threeway_breath_group.py`의 최종 출력에도 같은 교체 함수를 연결한다. 원본 복구 자료와 사용자가 요청한 화염 통합 정책을 구분하고 기존 저작 수정 보호 검사를 유지한다.

3갈래의 원본 main velocity는 socket-local +X이고 공통 화염은 +Z다. 복사한 세 그룹에만 Detail yaw90을 적용한다. 설치 모델의 본 basis1.7과 Detail scale1을 사용해1.7을 두 번 곱하지 않는다. 실제 모델·본과 현재 Codec/Playback으로 방향·크기·시각을 확인하고 기존 ID의 Authored 네 파일에 CAS 적용한 뒤 공식 publisher를 실행한다. 새 C++ 파일·shader·리소스 ID가 없어 프로젝트 등록 변경은 필요 없다. 화면 판정은 사용자가 한다.

### G16-01. 신규 파생 생성기 전체 코드

```python
"""Stage common firebreath in the existing cross and three-socket resources.

This is an appearance replacement, not a reconstruction of the source effects.
Existing resource identities, warning/preparation/light elements and authored
occurrence windows remain owned by the saved documents and their installer.
"""
import argparse
import collections
import copy
import hashlib
import json
from pathlib import Path

from build_kouku_shared_firebreath import SHARED_ASSET, copy_shared_firebreath_elements

ROOT = Path(__file__).resolve().parents[2]
AUTHORED = ROOT / 'Data/Effects/Authored'
CROSS_ASSETS = ('effect.kouku.firecross.impact.line',
                'effect.kouku.firecross.impact', 'effect.kouku.firecross.impact.full')
THREEWAY = 'effect.kouku.gate3.threeway.breath.full.restore'
CROSS_SOURCE = 'fx_mn_istm_00-4.par_d_istm_00-4_sk02_02'
THREEWAY_SOURCE = 'fx_mn_rpct_05_l.par_l_rpct_05_sk_01_loc_int'
REGENERATION_BUILDERS = ('build_kouku_albion_cross_groups.py', 'build_kouku_threeway_breath_group.py')
IDENTITY_PS = dict(uniformScaleMultiplier=1, yawOffsetDegrees=0,
                   directionYawDegrees=0, initialSpeedMultiplier=1)


def read(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + '\n', encoding='utf-8')


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def system(element):
    return element['sourceNode'].split('|')[-1].split('.particles')[0]


def strings(value):
    if isinstance(value, dict):
        for item in value.values():
            yield from strings(item)
    elif isinstance(value, list):
        for item in value:
            yield from strings(item)
    elif isinstance(value, str):
        yield value


def assert_independent(element):
    assert not element.get('runtimeCarrier') and not element.get('sourceTransformTrack')
    assert not element.get('transformInheritance', {}).get('enabled')


def replace_cross(document, shared):
    assert document['particleSystem'] == IDENTITY_PS
    selected = [e for e in document['elements'] if system(e) == CROSS_SOURCE]
    expected = 10 if document['effectAssetId'].endswith('.line') else 20
    assert len(selected) == expected
    groups = collections.defaultdict(list)
    for element in selected:
        assert_independent(element)
        assert not element['actionCueAttachment']['enabled']
        transform = element['detail']['transform']
        assert transform['rotationDegrees'][0] == transform['rotationDegrees'][2] == 0
        key = json.dumps([transform, element['detail']['timing']['startDelaySeconds']], sort_keys=True)
        groups[key].append(element)
    assert len(groups) == expected // 10 and all(len(g) == 10 for g in groups.values())
    result = copy.deepcopy(document)
    replaced_ids = {e['id'] for e in selected}
    result['elements'] = [copy.deepcopy(e) for e in document['elements'] if e['id'] not in replaced_ids]
    assert not replaced_ids.intersection(strings(result)), 'Retained element references a replaced impact'
    branches = []
    for ordinal, group in enumerate(groups.values()):
        source = group[0]
        transform = source['detail']['transform']
        start = source['detail']['timing']['startDelaySeconds']
        for opposite in (0, 180):
            key = document['effectAssetId'] + f'.shared.arm.{ordinal}.{opposite}'
            elements = copy_shared_firebreath_elements(shared, key, transform['position'])
            yaw = transform['rotationDegrees'][1] + opposite
            for element in elements:
                detail = element['detail']
                detail['transform']['rotationDegrees'] = [0, yaw, 0]
                detail['transform']['scale'] = [a * b for a, b in zip(
                    detail['transform']['scale'], transform['scale'])]
                detail['transform']['velocityPerSecond'] = copy.deepcopy(transform['velocityPerSecond'])
                detail['transform']['revolutionDegreesPerSecond'] = copy.deepcopy(transform['revolutionDegreesPerSecond'])
                detail['timing']['startDelaySeconds'] += start
            result['elements'].extend(elements)
            branches.append(dict(groupId=key, yawDegrees=yaw, startSeconds=start,
                                 position=transform['position'], elementIds=[e['id'] for e in elements]))
    return result, dict(replaced=len(selected), retained=len(document['elements']) - len(selected), branches=branches)


def replace_threeway(document, shared):
    assert document['particleSystem'] == IDENTITY_PS
    assert document['sourceModelPreview']['actorProfileId'] == 'MN_RPCT_05'
    selected = [e for e in document['elements'] if system(e) == THREEWAY_SOURCE]
    groups = collections.defaultdict(list)
    for element in selected:
        assert_independent(element)
        attachment = element['actionCueAttachment']
        assert attachment['enabled'] and attachment['follow']
        groups[attachment['runtimeAnchorSlotId']].append(element)
    assert set(groups) == {'FX_Prj_01', 'FX_Prj_02', 'FX_Prj_03'}
    assert all(len(g) == 14 for g in groups.values())
    result = copy.deepcopy(document)
    replaced_ids = {e['id'] for e in selected}
    result['elements'] = [copy.deepcopy(e) for e in document['elements'] if e['id'] not in replaced_ids]
    assert not replaced_ids.intersection(strings(result)), 'Retained element references a replaced breath'
    branches = []
    for slot, group in groups.items():
        source = group[0]
        transform, attachment = source['detail']['transform'], source['actionCueAttachment']
        start = source['detail']['timing']['startDelaySeconds']
        assert transform['rotationDegrees'] == [0, 0, 0]
        assert all(e['detail']['transform'] == transform and e['actionCueAttachment'] == attachment and
                   e['detail']['timing']['startDelaySeconds'] == start for e in group)
        key = document['effectAssetId'] + '.shared.' + slot.lower()
        elements = copy_shared_firebreath_elements(shared, key, transform['position'])
        for element in elements:
            element['actionCueAttachment'] = copy.deepcopy(attachment)
            detail = element['detail']
            # Old Sk_01's velocity payload is UE +X, hence Client +X. The
            # shared template is +Z: align it with that existing socket axis.
            detail['transform']['rotationDegrees'] = [0, 90, 0]
            # Actual installed RPCT05 bones already contribute scale1.7.
            # Keep the authored main scale instead of multiplying 1.7 twice.
            detail['transform']['scale'] = copy.deepcopy(transform['scale'])
            detail['transform']['velocityPerSecond'] = copy.deepcopy(transform['velocityPerSecond'])
            detail['transform']['revolutionDegreesPerSecond'] = copy.deepcopy(transform['revolutionDegreesPerSecond'])
            detail['timing']['startDelaySeconds'] += start
        result['elements'].extend(elements)
        branches.append(dict(groupId=key, sourceAnchorSlot=slot, bone=attachment['runtimeBoneName'],
                             attachment=attachment, sourceLocalForward=[1, 0, 0], sharedLocalYawDegrees=90,
                             startSeconds=start, position=transform['position'],
                             elementIds=[e['id'] for e in elements]))
    assert len(result['elements']) == 110
    return result, dict(replaced=42, retained=35, branches=branches)


def stage(output):
    output = output.resolve()
    assert output.is_relative_to(ROOT / 'out'), 'Stage output must remain under out/'
    shared_path = AUTHORED / (SHARED_ASSET + '.effect.json')
    shared = read(shared_path)
    composition_path = ROOT / 'Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json'
    tree_path = ROOT / 'Data/Effects/EffectResourceTree.json'
    composition, tree = read(composition_path), read(tree_path)
    resources = {r['assetId']: r for r in composition['presentationResources']
                 if r['kind'] == 'EFFECT' and r['resourceKind'] == 'V1_EFFECT'}
    parents = {r['assetId']: r['parentId'] for r in tree['references'] if r['kind'] == 'V1'}
    scripts = [Path(__file__), Path(__file__).with_name('build_kouku_shared_firebreath.py')]
    scripts.extend(Path(__file__).with_name(name) for name in REGENERATION_BUILDERS)
    inputs = {p.relative_to(ROOT).as_posix(): sha(p) for p in (shared_path, composition_path, tree_path, *scripts)}
    documents, diagnostics = [], []
    for asset in (*CROSS_ASSETS, THREEWAY):
        path = AUTHORED / (asset + '.effect.json')
        before = path.read_bytes()
        original = json.loads(before)
        candidate, info = replace_threeway(original, shared) if asset == THREEWAY else replace_cross(original, shared)
        assert len({e['id'] for e in candidate['elements']}) == len(candidate['elements'])
        candidate_path = output / 'candidate' / path.name
        write(candidate_path, candidate)
        backup = output / 'baseline' / path.name
        backup.parent.mkdir(parents=True, exist_ok=True)
        backup.write_bytes(before)
        resource = resources[asset]
        consumers = [dict(patternId=p['patternId'], occurrenceId=o['occurrenceId'])
                     for p in composition['patterns'] for o in p['presentationOccurrences']
                     if o['resourceId'] == resource['resourceId']]
        relative = path.relative_to(ROOT).as_posix()
        inputs[relative] = hashlib.sha256(before).hexdigest()
        documents.append(dict(effectAssetId=asset, candidatePath=candidate_path.relative_to(ROOT).as_posix(),
            path=relative, displayName=original['displayName'], durationMs=resource['durationMs'],
            defaultAnchorKind=resource['defaultAnchorKind'], parentId=parents[asset],
            resourceId=resource['resourceId'], beforeSha256=inputs[relative],
            candidateSha256=sha(candidate_path), changeKind='REPLACE_DIRECTIONAL_FLAME_APPEARANCE'))
        diagnostics.append(dict(effectAssetId=asset, beforeElements=len(original['elements']),
            afterElements=len(candidate['elements']), existingResourceDurationMs=resource['durationMs'],
            savedConsumers=consumers, **info))
    write(output / 'installation.json', dict(installed=False, stageOnly=True, documents=documents,
        inputHashes=inputs, diagnostics=diagnostics, sharedAssetId=SHARED_ASSET,
        sharedNativeDurationSeconds=5.5, authoredOccurrenceWindowsChanged=False,
        emptyPatternAnimationsCreated=False, compositionRevision=composition['revision'],
        manualVisualValidation='USER_PENDING'))
    print(json.dumps(dict(staged=len(documents), output=str(output), liveDataWritten=False)))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--output', type=Path, default=ROOT / 'out/KoukuDirectionalFlame20260914')
    stage(parser.parse_args().output)

```


## G17. 쇼타임 쿠크세이튼 사라지기 독립 Effect — 2026-09-14

사용자가 첨부한 P35 animation.8은 리허설4219985 stage007의 `rpct00_att_battle_24_03`이다.
실제 사라짐은 바로 앞 stage006 `rpct00_att_battle_28_09`의 HidePawn1.804677초 뒤,
Light notify016의1.811208초와 BallRead notify017의1.811831초다. 본편4219939 stage008과 같다.

새 `effect.kouku.gate3.showtime.saydon.disappear`를 `쇼타임 | 쿠크세이튼_사라지기`로 등록한다.
원본11개 BallRead emitter와 동시 Light1개를 0초 기준으로 묶고 상대 발생 간격을 보존한다.
FX_buff_01/b_root의 원본 위치0·배율1.2와 root snapshot Light 위치(0,1.5,0)를 유지한다.
현행 ball.red의 native material/resource를 재사용하고 sourceModelPreview는 해당 클립의 발생 직후 구간을 참조한다.
새 authored, Catalog, Tree와 Client project/filter None 등록만 변경한다. 실행 중 Composition과 기존 authored는 덮어쓰지 않는다.

검증은 선택 문서의 실제 codec/저장 재개방·CPU 재생, 기존 DDS/native shader 의존성,
JSON/XML parse와 diff 검사다. 사용자 EXE 빌드 보류를 유지하며 Client/UI를 실행하지 않는다.
목록 Refresh와 runtime catalog 재로드는 다르므로 Composition 실제 재생은 편집 저장 후 다음 Client 실행에서 확인한다.


## G18. 쇼타임 노란 사각형 예고와 3회 공습 폭발 — 2026-09-14

추가 요청은 원본 크기의 노란 직사각형 예고에 inner 차오름을 연결하고, 같은 장판 범위의
공습 폭발을 원본 3회 시각·위치로 묶는 것이다. 원본 Projectile421991210은 SkillDecal2113
`GR_Mon_Rectangle_cond_EX_01`과 `FX_O_De_CondSquare_02_01_Tr`를 사용한다.
확인한 세 AirStrike_Exp 발생은2.0/2.4/2.7초, 원본UE위치는(-600,0,0)/(0,0,0)/(600,0,0)이다.
선행 Missile은1.0/1.4/1.7초의 별도 발생이며 사용자가 요청한 폭발 묶음과 구분한다.

기존 GroundEffect LocalDecal·materialParameterTracks 경로로 사각형의 정식 재질을 연결한다.
예고의 확정 크기·타이밍은 SkillEffect421991224와 원본 FixArea footer의 실제 계약을 읽은 뒤 사용한다.
장판의 바깥 크기를 고정하고 원본 재질 inner 입력에 차오름 곡선을 넣는다.
폭발은 현행 showtime.airstrike.impact16개 요소를 원본 위치·회전·시각별 독립48개로 복제한다.
각 sourceRecipe와 native 재질, particle tail을 보존하며 emitter loop 횟수 변경으로 3회를 흉내내지 않는다.

등록 이름은 `쇼타임 / 사각형 장판 | 쇼타임_사각형_예고`와
`쇼타임 / 사각형 장판 | 쇼타임_사각형_폭발`이다. Tree의 반복되는 분류 접두사는 실제 UI 투영 규칙에 맞게 단축한다.
새 authored2개, Catalog·Tree·project/filter None과 필요한 native table/shader만 변경한다.
새 C++ runtime 경로는 만들지 않는다. 새 Python 생성기가 필요하면 실제 source 획득·검증·stage를 소유한다.

사용자는 후속 메시지로 EXE 빌드 보류를 해제하고 전체 반영 후 빌드까지 요청했다.
원본 수치, 실제 codec/playback·리소스 준비, 변경 shader/CPP와 JSON/XML/diff를 검증한 뒤 공식 Product 빌드를 수행한다.
실행 중 Client의 미저장 편집은 여전히 보존해야 하므로 외부 Composition 쓰기나 임의 프로세스 종료는 하지 않는다.

### G18-01. 원본 수치와 기존 소비자 연결

`421991210.loa`의 FixArea StartIndexDecal은 EFGAME의
`EFSummonsFixAreaStartIndexDecal` ScriptStruct 이름·타입·연결 순서와 대조한다.
확정 입력은 Time 0초, Duration 2초, DecalBlendInTime 1.5초, DecalScaleTime 0초,
DecalFillTime 1.5초, DecalBlendOutTime 0.5초다. `len-164`의 2.3초는 마지막 sound
Timer이므로 예고 수명으로 사용하지 않는다. `source_contract.json`의 hash와 byte offset으로
읽은 값을 다시 검사한 뒤 생성한다. alpha 0→1→0과 inner 선형 보간은 확인한 named timing을
기존 SourceTransformTrack에 투영하는 방식이며 원본 native 시간 보간 코드 전체를 복원했다는 뜻은 아니다.

SkillEffect 421991224의 AreaRange 1800·AreaAngle 300·AreaOffsetX -900을 사용한다.
`HitAreaWire`의 halfWidth 계산과 `ClientReplication`의 역변환에서 AreaAngle이 전체 폭임을
확인했으므로 예고는 폭 3m·길이 18m다. LocalDecal의 StartSize x/y는 projector의 두 평면 축이다.
따라서 source StartSize `[300,1800,300]`에서 Size.x=3·Size.y=18을 만들고 기존 renderer가
world X=3·Z=18로 소비한다. particle 크기에 UE world 좌표 변환을 다시 적용하지 않는다.
바깥 projector는 고정하고 native 3607의 `inner`만 0→1로 1.5초 동안 채운다.

폭발의 원본 UE X -600/0/+600cm는 공통 독립 전방 변환을 한 번 적용해 runtime Z -6/0/+6m로
놓는다. 원본 yaw 90도와 독립 전방 yaw -90도가 상쇄되어 최종 yaw는 0도다.
2.0/2.4/2.7초의 세 발생을 별도 ID·16요소씩 유지하고 독립 Effect의 시작만 0/0.4/0.7초로
옮긴다. 원본 sourceRecipe·native material·tail은 유지한다. 선행 Missile과 sound·shake,
원본 gameplay의 무작위 대상 선택은 이 두 독립 MAP Effect에 추가하지 않는다.

### G18-02. 신규 생성 파일과 루틴 책임

파일은 `Tools/EffectPipeline/build_kouku_showtime_rectangle_groups.py`다. CLI의 `stage()`가
직접 호출자이며 기존 warning builder의 `independent_document`·`constant_distribution`과
backstep builder의 `inspect_document`를 재사용한다. 출력은 `out/` 아래 candidate 2개와
`installation.json`이다. 실제 등록은 검증된 후보를 기존 Authored·Catalog·Tree에 연결하는 별도
설치 단계가 소유한다. 생성기는 실행 중 Composition이나 기존 Authored를 직접 쓰지 않는다.
새 C++ 선언·runtime·schema는 없으며 생성된 Authored 2개는 Client project/filter의
`96.DataFiles` None 항목으로 등록한다. Python 자체는 C++ compile 항목에 추가하지 않는다.

| 선언·입력 | 한 줄 책임 |
|---|---|
| `argparse`, `Path` | 명시 출력 경로를 받아 저장소 `out/` 경계 안으로 제한한다. |
| `copy` | 기존 template의 recipe·재질을 보존하면서 독립 문서를 만든다. |
| `hashlib`, `struct` | 계약서의 출처 hash와 원본 little-endian 필드 값을 다시 확인한다. |
| `json` | UTF-8 JSON을 읽고 NaN을 거부하는 후보·기록을 쓴다. |
| `warning`, `inspect_document` | 기존 독립 ID 변환·상수 분포·리소스 및 tail 검사 경로를 재사용한다. |
| `ROOT`, `DEFAULT_OUTPUT` | 저장소 경계와 이번 source receipt/candidate 작업 위치를 정한다. |
| `PREFIX`, `SOURCE_MATERIAL`, `NATIVE_ID` | 두 stable asset ID와 정확한 원본 재질·native 3607 결합을 검사한다. |
| `contract`, `timing`, `calls` | 원본 크기·named timing·서로 다른 세 폭발 occurrence의 입력을 전달한다. |
| `template`, `native_material` | 현재 설치된 particle recipe와 검증된 사각형 LocalDecal 재질을 제공한다. |
| `entries`, `inputHashes` | 생성 문서의 요소·수명·리소스 검사와 재현 입력 hash를 기록한다. |

`read()`는 파일 bytes를 JSON으로 해석하고 실패를 그대로 전달한다. `write()`는 후보의 부모
폴더를 만든 뒤 UTF-8·유효 숫자만 저장한다. `verify_source()`는 schema/version, Projectile
길이·hash, 이름으로 해독한 필드의 원본 byte 값, 세 timer·particle token, 위치·방향·scale,
area 크기와 GroundEffect 재질·색 hash를 순서대로 검사한다. 불일치하면 assertion으로 중단하며
다른 원본이나 generic 수치로 대체하지 않는다. 성공하면 named timing과 세 impact 호출을 반환한다.

`key()`는 기존 distribution 형식의 linear key와 0 tangent를 만든다. `rectangle_warning()`은
circle template의 독립 문서를 만든 뒤 1개 LocalDecal인지 검사하고 native 3607을 결합한다.
fixed TRS·2초 수명·3×18m 평면·active color를 설정하고 recipe의 lifetime/startsize를 같은
단위 계약으로 맞춘다. 기존 단일 source track에 alpha/inner 곡선을 넣고 material scalar를
설정한다. 내부 attachment·inheritance가 꺼졌음을 확인해 중복 source basis를 제거하고 반환한다.

`rectangle_impacts()`는 16요소 template을 세 번 기존 ID remap 경로로 복제한다. 각 발생마다
원본 시작 간격·공간 변환값·원본 source 시각을 설정한다. 각 복사본의 sourceRecipe와 재질은
그대로 두고 portable-copy의 `authored-copy:<원본 ID>`로 sourceNode를 연결해 RNG identity를
보존한다. attachment/inheritance가 꺼졌는지 검사하고 최종 48개 요소와 48개 고유 ID를
확인한 문서만 반환한다. emitter loop 증가는 사용하지 않는다.

`stage()`는 먼저 출력 경계를 검사하고 원본 계약 검증을 완료한다. 정확한 native 3607 재질과
기존 circle/airstrike template을 읽어 두 생성 함수를 호출한다. 각 candidate를 저장하고
`inspect_document()`로 resource 누락·particle tail을 확인한다. 예고 metadata는 정확히
2000ms, 폭발은 tail을 포함한 5801ms를 기록한다. 최종 manifest에는 입력 hash,
`installed=false`, `stageOnly=true`, `compositionWritten=false`, `USER_PENDING`을 남긴다.
도중 실패 시 live Data는 바뀌지 않으며 남은 out 후보는 설치 성공 증거로 사용하지 않는다.

이 manifest의 예고 2000ms는 시각 활성 구간이다. Composition의 신규 Preview/Append 길이는
G18-04에서 실제 Playback 종료 계산을 재사용해 4000ms로 결정한다. 보수적인 재생 시계와
시각 활성 창을 구분하며 예고 source timing·inner 곡선을 늘리지 않는다.

검사는 Python 문법·선택 JSON/XML와 원본 byte/ID/리소스 대조, 실제 Codec/Playback,
native 3607의 격리 FXC를 수행한다. 전체 Effect source 검사에서 발생한 기존 문서 오류는
선택 후보 검사와 구분하고, 공식 Product 빌드와 사용자 화면 확인 결과는 RESULT에 따로 적는다.
Composition에서는 같은 MAP 위치·회전으로 예고를 Append한 시각보다 2초 뒤에 폭발을 Append한다.
둘을 함께 배치할 때 기존 Set Group·그룹 위치 편집·복제와 Save를 사용한다.

### G18-03. 신규 생성기 전체 코드

아래는 이 G의 실제 신규 파일 전체 코드다. 원본 receipt와 native 재질 입력을 준비한 뒤
`python Tools/EffectPipeline/build_kouku_showtime_rectangle_groups.py`로 out 후보를 재생성한다.

```python
"""Stage the original 421991210 rectangle warning and three impact occurrences.

The decoded source receipt owns original field offsets, times and area dimensions.
This composes existing LocalDecal/particle contracts; it never writes live drafts.
"""
import argparse
import copy
import hashlib
import json
from pathlib import Path
import struct

import build_kouku_showtime_warning_groups as warning
from build_kouku_backstep_flame_groups import inspect_document

ROOT = warning.ROOT
PREFIX = 'effect.kouku.gate3.showtime.rectangle.'
SOURCE_MATERIAL = 'fx_m_mi_o_00.fx_mi.fx_o_de_condsquare_02_01_tr'
NATIVE_ID = 'effect.ue3.kouku-3607-native.v1'
DEFAULT_OUTPUT = ROOT / 'out/KoukuShowtimeRectangle20260914'


def read(path):
    return json.loads(path.read_bytes())


def write(path, document):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes((json.dumps(document, ensure_ascii=False, indent=2, allow_nan=False) + '\n').encode('utf8'))


def verify_source(contract):
    """Recheck the exact decoded records against their source bytes before stage."""
    assert contract['schema'] == 'lostark.readonly-source-contract.showtime-rectangle'
    assert contract['version'] == 1
    raw = Path(contract['sourceProjectile']['path']).read_bytes()
    assert hashlib.sha256(raw).hexdigest() == contract['sourceProjectile']['sha256']
    assert len(raw) == contract['sourceProjectile']['byteSize'] == 10105
    for field in contract['startIndexDecal']['fields']:
        fmt = '<f' if field['type'] == 'FloatProperty' else '<I'
        assert struct.unpack_from(fmt, raw, field['byteOffset'])[0] == field['value']
    names = contract['startIndexDecal']['sourceNamedTiming']
    assert names == dict(Time=0.0, Duration=2.0, DecalBlendInTime=1.5,
                         DecalScaleTime=0.0, DecalFillTime=1.5, DecalBlendOutTime=0.5)
    impacts = [c for c in contract['calls'] if c['role'] == 'impact']
    assert len(impacts) == 3
    for index, call in enumerate(impacts):
        assert struct.unpack_from('<f', raw, call['timerValueByteOffset'])[0] == call['timeSeconds']
        offset = call['particleTokenByteOffset']
        count = struct.unpack_from('<I', raw, offset)[0]
        token = raw[offset + 4:offset + 3 + count].decode('ascii')
        assert token.split("'")[1].lower() == call['particleSystem'].lower()
        assert call['runtimeIndependentPositionM'] == [0, 0, (index - 1) * 6]
        assert call['sourceScale'] == [1, 1, 1] and call['runtimeIndependentYawDegrees'] == 0
    area = contract['areaRows'][0]
    assert (area['AreaRange'], area['AreaAngle'], area['AreaOffsetX']) == (1800, 300, -900)
    assert contract['geometry']['fullLengthM'] == 18 and contract['geometry']['fullWidthM'] == 3
    ground = contract['groundEffect']
    payload = Path(ground['path']).read_bytes()
    assert hashlib.sha256(payload).hexdigest() == ground['sha256']
    assert ground['sourceMaterial'] == SOURCE_MATERIAL
    assert list(struct.unpack_from('<4f', payload, ground['fieldOffsets']['activeColor'])) == ground['activeColor']
    return names, impacts


def key(time, values):
    return dict(timeSeconds=time, value=values, arriveTangent=[0] * len(values),
                leaveTangent=[0] * len(values), interpolation='linear')


def rectangle_warning(contract, native_material, template):
    """Keep the full projector fixed while the native inner parameter fills it."""
    name = '쇼타임 / 사각형 장판 | 쇼타임_사각형_예고'
    doc = warning.independent_document(template, PREFIX + 'warning', name)
    assert len(doc['elements']) == 1 and not doc.get('modelCues')
    element = doc['elements'][0]
    identity = 'project.groundeffect.adapter.showtime.rectangle'
    element.update(id=PREFIX + 'warning.decal', groupId=PREFIX + 'warning',
                   displayName='쇼타임_사각형_예고', sourceNode=identity + '|' + SOURCE_MATERIAL,
                   resources=[], material=copy.deepcopy(native_material))
    assert element['material']['sourceMaterialPath'] == SOURCE_MATERIAL
    assert element['material']['sourceProfile']['runtimeShaderProfileId'] == NATIVE_ID
    detail, recipe = element['detail'], element['sourceRecipe']
    width, length = contract['geometry']['fullWidthM'], contract['geometry']['fullLengthM']
    timing = contract['startIndexDecal']['sourceNamedTiming']
    duration = timing['Duration']
    detail['transform'].update(position=[0, 0, 0], rotationDegrees=[0, 0, 0], scale=[1, 1, 1])
    assert not any(detail['linearLerp'][field] for field in ('position', 'rotation', 'scale'))
    detail['timing'].update(startDelaySeconds=0, lifeTimeSeconds=duration)
    detail['color']['multiply'] = contract['groundEffect']['activeColor']
    detail['decal'].update(size=[width, length], depth=6, receiverMode='upwardSurfaces', normalCutoff=.5)
    detail['particle'].update(lifeTimeSeconds=[duration] * 2, startSize=[width, length],
                              endSize=[width, length], localSpace=True)
    recipe.update(emitterDelaySeconds=0, emitterDurationSeconds=duration, emitterLoopCount=1)
    for module in recipe['modules']:
        for literal in module['literals']:
            if literal['propertyPath'] == 'emitterduration':
                literal['value'] = duration
            if literal['propertyPath'] == 'rotation.degrees.roll':
                literal['value'] = 0
        for distribution in module['distributions']:
            prop = distribution['propertyPath']
            if prop == 'lifetime':
                distribution.update(warning.constant_distribution(prop, [duration]))
            elif prop == 'startsize':
                # LocalDecal's particle Size.x/Size.y are the X/Z projector axes.
                distribution.update(warning.constant_distribution(prop, [width * 100, length * 100, width * 100]))
    track = element['sourceTransformTrack']
    track.update(sourceOccurrenceId=identity, sourceTimeOriginSeconds=0)
    assert len(track['nodes']) == 1 and track['nodes'][0]['scaleUE3'] == [1, 1, 1]
    track['nodes'][0]['sourceObjectPath'] = identity
    track['alphaScaleKeys'] = [key(t, [alpha] * 3) for t, alpha in
                              contract['startIndexDecal']['runtimeProjection']['alphaKeys']]
    track['materialParameterTracks'] = [dict(name='inner', kind='SCALAR',
        keys=[key(0, [0]), key(timing['DecalFillTime'], [1])])]
    overrides = dict(inner=0, decal_drawscale_x=width, decal_drawscale_y=length)
    profile = element['material']['sourceProfile']
    assert set(overrides) <= {p['name'] for p in profile['scalars']}
    for param in profile['scalars']:
        if param['name'] in overrides:
            param['value'] = overrides[param['name']]
    attachment = element['actionCueAttachment']
    assert not attachment['enabled'] and not element['transformInheritance']['enabled']
    attachment.pop('snapshotRootSourceBasisYawDegrees', None)
    return doc


def rectangle_impacts(contract, template):
    """Clone each original occurrence with its own IDs, offset and emission clock."""
    asset = PREFIX + 'impact'
    doc = copy.deepcopy(template)
    doc.update(effectAssetId=asset, displayName='쇼타임 / 사각형 장판 | 쇼타임_사각형_폭발', elements=[])
    assert len(template['elements']) == 16 and not template.get('sourceModelPreview')
    calls = [c for c in contract['calls'] if c['role'] == 'impact']
    first = calls[0]['timeSeconds']
    for index, call in enumerate(calls, 1):
        group = asset + '.burst' + str(index)
        own = warning.independent_document(template, group, '공습 폭발 ' + str(index))
        for element, original in zip(own['elements'], template['elements'], strict=True):
            # Preserve the original RNG identity through the portable-copy contract.
            origin = original['sourceNode']
            element['sourceNode'] = origin if origin.startswith('authored-copy:') else 'authored-copy:' + original['id']
            assert not element['actionCueAttachment']['enabled']
            assert not element['transformInheritance']['enabled']
            element['actionCueAttachment'].pop('snapshotRootSourceBasisYawDegrees', None)
            element['detail']['timing']['startDelaySeconds'] += call['timeSeconds'] - first
            element['detail']['transform'].update(position=call['runtimeIndependentPositionM'],
                rotationDegrees=[0, call['runtimeIndependentYawDegrees'], 0], scale=call['sourceScale'])
            element['sourcePresentation']['sourceTimeSeconds'] = call['timeSeconds']
            doc['elements'].append(element)
    assert len(doc['elements']) == 48
    assert len({e['id'] for e in doc['elements']}) == 48
    return doc


def stage(output):
    output = output.resolve()
    assert output.is_relative_to(ROOT / 'out'), 'Stage output must remain under out/'
    contract = read(output / 'source_contract.json')
    timing, calls = verify_source(contract)
    native_path = output / 'native/native_material_patch.json'
    material = next(p['material'] for p in read(native_path)['programs'] if p['program'] == 3607)
    authored = ROOT / 'Data/Effects/Authored'
    warning_path = authored / 'effect.kouku.gate3.showtime.circle.warning.effect.json'
    impact_path = authored / 'effect.kouku.gate3.showtime.airstrike.impact.effect.json'
    docs = [rectangle_warning(contract, material, read(warning_path)), rectangle_impacts(contract, read(impact_path))]
    entries = []
    for doc in docs:
        path = output / 'candidate' / (doc['effectAssetId'] + '.effect.json')
        write(path, doc)
        validation = inspect_document(doc)
        if doc['effectAssetId'].endswith('.warning'):
            validation['durationMs'] = round(timing['Duration'] * 1000)
        entries.append(dict(effectAssetId=doc['effectAssetId'], displayName=doc['displayName'],
            path=path.relative_to(ROOT).as_posix(), defaultAnchorKind='MAP', **validation))
    write(output / 'installation.json', dict(installed=False, stageOnly=True, documents=entries,
        sourceProjectileId=421991210, sourceSkillDecalId=2113, sourceWarningAreaSkillEffectId=421991224,
        geometry=contract['geometry'], sourceWarningTiming=timing,
        independentImpactStartSeconds=[c['timeSeconds'] - calls[0]['timeSeconds'] for c in calls],
        inputHashes={p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
                     for p in (warning_path, impact_path, native_path, output / 'source_contract.json')},
        compositionWritten=False, manualVisualValidation='USER_PENDING'))
    print(json.dumps(entries, ensure_ascii=False))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--output', type=Path, default=DEFAULT_OUTPUT)
    stage(parser.parse_args().output)
```

### G18-04. 신규 Effect Append에서 source particle tail 보존

Workbench의 기존 선택 길이 계산은 Detail의 startDelay+lifeTime+afterImage만 사용하므로
사라지기를 1001ms, 세 폭발을 2000ms로 잘랐다. 실제 Playback 종료는 각각 2201ms,
5801ms이며 source emitter/particle tail을 포함한다. 새 리소스의 Preview와 Append가 같은
끝 시각을 사용하도록 `CEffectPlayback::Calculate_ElementEndSeconds`와 기존
SourceParticleCarrier 판별을 재사용한다. 새 수명 계산식이나 별도 Playback은 만들지 않는다.

신규 V1_EFFECT/V1_ELEMENT 선택과 Preview/Append, Created resource를 재사용해 다시
Append할 때 현재 Effect 문서의 실제 종료를 계산한다. 요소 단위에서는 같은 source carrier
판별을 전달하고 전체 Effect는 각 요소 종료와 model cue 종료의 최대값을 사용한다.
값은 기존 editor 시간 범위에서 올림한 millisecond로 변환한다. 읽기·validation 실패는
기존 상태 메시지로 남기고 불완전한 리소스나 occurrence를 추가하지 않는다.

변경 대상은 새 Preview/Append의 기본 길이와 resource metadata다. 이미 존재하는 occurrence의
사용자 duration·start·fade·배치와 timeline 편집 결과를 자동 수정하지 않는다. 예고의 새 Append
길이는 기존 Playback의 보수적 4000ms이지만 실제 표시·채움은 여전히 2초/1.5초다.
폭발은 예고 box 끝이 아니라 예고 시작+2000ms에 놓는다. 기존 Pattern/World의 남은 창으로
clamp하는 배치 규칙은 유지하며 전체 tail을 쓰려면 그만큼의 timeline 길이가 필요하다.
실제 helper·caller는 다음 절과 같고 최소 컴파일 결과는 RESULT에 기록한다.

### G18-05. Workbench 종료 계산의 실제 helper·호출부

변경 파일은 `Client/Private/KoukuSaydonActionWorkbench.cpp` 하나다. 기존 public static
계산을 쓰기 위해 `Effect_Playback.h`를 include한다. 아래 두 함수는 파일 내부 helper이며
새 header 선언·C++ 파일·project/filter 항목은 없다. 원본 emitter·particle·trail 종료 규칙은
기존 `CEffectPlayback::Calculate_ElementEndSeconds`가 계속 소유한다.

| 선언·값 | 한 줄 책임 |
|---|---|
| `V1_ElementDurationMs(element)` | 실제 source carrier 판별로 공통 Playback 종료를 구하고 유효 millisecond를 반환한다. |
| `sourceParticleCarrier` | Effect Tool과 같은 enabled source recipe의 sprite/mesh/decal 판별을 전달한다. |
| `seconds` | 공통 종료 계산 결과이며 비유한 값·음수면 실패를 뜻하는 0을 반환한다. |
| `Refresh_V1ResourceDuration(resource, outStatus)` | 현행 Catalog 문서를 stable asset/element ID로 읽어 리소스 기본 길이만 갱신한다. |
| `durationMs`, `elementFound` | 검사 중인 최대 종료·선택 element 존재 여부를 보관하고 성공 뒤에만 입력 resource에 반영한다. |

`Refresh_V1ResourceDuration()`은 EFFECT의 V1_EFFECT/V1_ELEMENT만 대상으로 한다. Catalog가
없으면 구체 asset과 catalog status를 반환한다. V1_ELEMENT는 정확한 element ID를 찾고
V1_EFFECT는 visible 요소·visible ModelCue의 최대 종료를 구한다. 요소 누락·비유한 종료는
실패 이유를 보존하며 입력 duration은 그대로다. 정상 종료는 기존 1..600000ms 범위에서
올림해 저장한다. 아래는 두 helper의 실제 전체 코드다.

```cpp
	std::uint32_t V1_ElementDurationMs(const EFFECT_ELEMENT_DESC& element)
	{
		// Match Effect Tool's Element_PreviewEndSeconds carrier admission.
		const auto& recipe = element.SourceRecipe;
		const bool sourceParticleCarrier = recipe.bEnabled &&
			(recipe.strRendererShape == "sprite" || recipe.strRendererShape == "mesh" || recipe.strRendererShape == "decal");
		const auto seconds = CEffectPlayback::Calculate_ElementEndSeconds(element, sourceParticleCarrier);
		if (!std::isfinite(seconds) || seconds < 0.f) return 0u;
		return static_cast<std::uint32_t>(std::clamp(std::ceil(1000.0 * seconds),
			1.0, static_cast<double>(MAX_EDITOR_TIME_MS)));
	}

	bool Refresh_V1ResourceDuration(KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE& resource,
		std::string& outStatus)
	{
		if (resource.eKind != KOUKU_SAYDON_PRESENTATION_KIND::EFFECT ||
			(resource.strResourceKind != "V1_EFFECT" && resource.strResourceKind != "V1_ELEMENT")) return true;
		const auto document = CEffectCatalog::Find(resource.strAssetId);
		if (!document)
		{
			outStatus = "Effect duration unavailable: " + resource.strAssetId + "; " + CEffectCatalog::Get_Status();
			return false;
		}
		std::uint32_t durationMs = 1u;
		bool elementFound = resource.strResourceKind == "V1_EFFECT";
		for (const auto& element : document->Elements)
		{
			if (resource.strResourceKind == "V1_ELEMENT" ? element.strElementId != resource.strElementId : !element.bVisible) continue;
			elementFound = true;
			const auto elementMs = V1_ElementDurationMs(element);
			if (!elementMs)
			{
				outStatus = "Effect duration is not finite: " + element.strElementId;
				return false;
			}
			durationMs = (std::max)(durationMs, elementMs);
		}
		if (!elementFound)
		{
			outStatus = "Effect element duration unavailable: " + resource.strAssetId + " | " + resource.strElementId;
			return false;
		}
		if (resource.strResourceKind == "V1_EFFECT")
			for (const auto& cue : document->ModelCues)
				if (cue.bVisible)
					durationMs = (std::max)(durationMs, static_cast<std::uint32_t>(std::clamp(
						std::ceil(1000.0 * (cue.fStartDelaySeconds + cue.fDurationSeconds)),
						1.0, static_cast<double>(MAX_EDITOR_TIME_MS))));
		resource.iDurationMs = durationMs;
		return true;
	}
```

`Create_PresentationResource()`는 새 ID 발급 전에 복사한 source 길이를 갱신한다.
`Stage_PresentationSource()`는 검증한 currentSource를 사용해 새 resource를 만들거나 동일
asset/kind/element의 candidate resource metadata만 갱신한다. 이 staging의 실제 commit은
기존 caller가 계속 소유한다.

`Append_PresentationCandidate()`는 Created 항목의 저장된 기본 길이도 그대로 신뢰하지 않고
currentResource 복사본에서 다시 계산한다. 그 값으로 새 row의 길이를 정한 뒤 기존
Pattern 남은 창으로 clamp한다. Sequence 배치는 같은 currentResource를
`Configure_SequenceEffectOccurrence()`에 전달하고 기존 원자 candidate commit을 사용한다.
기존 occurrence를 순회해 수명을 바꾸지 않는다.

`Queue_PresentationPreview()`는 선택된 기존 Effect occurrence가 있으면 원래 box로
`Request_PatternPreview()`를 호출하고 먼저 반환한다. 새 standalone Preview만 currentResource
길이를 갱신한다. `Queue_SequenceEffectPreview()`는 위 Stage를 재사용하며 이미 선택한
occurrence는 복사한 duration·TRS를 유지한다. `Configure_SequenceEffectOccurrence()`는 새
row에만 호출한다. `Render_PresentationResources()`의 source 선택/확장도 같은 helper와
요소 계산을 사용해 목록과 V1_ELEMENT 기본 길이를 맞춘다.

## G19. V1 비동기 준비 결과를 대기 상태에서 보존

목표는 양손 발사 섬광과 장판 V1 준비 중 새 target enqueue가 발생해도 완료 결과를
FIFO 불일치로 폐기하지 않는 것이다. 저장된 Composition·Effect 문서는 변경하지 않는다.

변경 파일은 Client/Private/Effect_PresentationService.cpp의
Advance_LoadingProductCuePreparation이다. 새 H/include/enum/상태/API는 없다.
현재 Enqueue/Enqueue_Priority는 후속 target 추가 시 m_bYieldNextFrame을 켠다.
기존 소비자는 결과를 먼저 Pop한 뒤 Begin_LoadingFrame의 YIELDED를 structural
failure로 처리한다. 결과가 있을 때 먼저 Begin_LoadingFrame을 호출하고 YIELDED면
Pop 없이 다음 프레임으로 넘긴다. READY/IDLE의 결과는 이후 기존 epoch/revision/kind
검사, terminal complete 및 정확한 front ID 검증과 stage/commit/ACK를 그대로 거친다.
EPOCH_STAGE_COMPLETE는 owner의 마지막 target이 소진된 IDLE 상태에서도 처리한다.

기존 실제 queue/job을 사용해 추가 normal/priority enqueue 중 result 보존, 다음 frame
동일 target commit 가능, terminal complete, 잘못된 ID/epoch 거부를 검사한다.
대응 CPP 최소 Debug 컴파일과 diff 검사를 수행한다. 새 project/filter 등록은 없다.
사용자가 직접 수행 중인 Product 빌드·Client를 중지하거나 자동 재실행하지 않는다.
최종 재생 확인은 새 빌드의 쇼타임 양손 발사 섬광·장판 조준/폭발·화염장판이다.

## G20-Server. 쇼타임 duration의 플레이어별 고정·추적 그룹

이 절의 소유 범위는 Server이며 Client/Publisher/저장 데이터는 root가 연결한다. 현재 저장
revision816과 실행 중 미저장 duration은 외부에서 교체하지 않는다. 사용자 요청값인2초와
이동속도50%는 원작 복원값으로 기록하지 않는다.

기존 MechanicTrigger에 SHOWTIME_PLAYER_TARGETS와 두 visual ID, 고정 그룹 수명,
생성 간격, 추적 속도 비율을 추가한다. PATTERNSHOWTIMETARGETS의11필드가 encounter,
pattern, occurrence, start/duration, fixed/tracking visual ID, fixed lifetime, interval,
speed scale을 저장한다. 기존 bootstrap 행과 Shared packet 구조는 유지한다.

고정·추적은 각각 단독 또는 함께 쓸 수 있다. 비활성 visual ID의 TSV 값은 `-`이며 최소
한 ID가 필요하다. fixed가 없으면 lifetime은 0, 있으면 1..600000ms다. interval은
1..600000ms, speed scale은 0.01..10이며 두 활성 visual ID는 달라야 한다.

Logic ledger는 window 시작·종료·다음 고정 생성 tick과 플레이어별 tracking object ID를
소유한다. room은 시작tick과60tick 간격에 현재 살아 있는 player의 서버 좌표에서 고정 그룹을
stage/commit하고, tracking은 player당 하나만 만든다. 매 tick 실효 이동속도의0.5배만큼
대상 위치로 접근하며 같은 지면 높이를 조회한다. 죽음·퇴장·duration 종료에는 tracking만
정리하고 고정 그룹의 남은 저작 수명은 유지한다. Pattern abort/room reset은 기존 전체
owner 정리를 사용한다. Late join도 기존 CombatObject live spawn/snapshot 경로를 사용한다.

Client는 기존 iCombatObjectId로 독립 인스턴스를 구별하고 iSpawnTick으로 그룹 clock을
계산한다. 새 damage는 없으며 archetype는 combatobject.kouku.showtime.fixed와
combatobject.kouku.showtime.tracking이다. 그룹의 상대 시간·TRS는 Client visual mapping이
보존한다. Server는 Effect asset path나 Client 모델 좌표를 받지 않는다.

기존 Server 계약 검사에서1~4인 생성·60tick 반복·fixed 위치 보존·0.5속도·join/leave/revive·
종료/abort와 실패 rollback을 검증한다. 변경 TU와 struct를 사용하는 실제 Server 소비자를
out에 격리 컴파일하며 제품 빌드·publish·
실행 종료·reload는 수행하지 않는다. 새 C++ 파일과 project/filter 등록은 필요하지 않다.

## G20-Authoring. 쇼타임 플레이어별 생성과 추적 연결

기존 DURATION 정의에 SHOWTIME_PLAYER_TARGETS를 추가한다. fixedSelectionGroupId와
trackingPresentationOccurrenceId는 같은 Pattern의 실제 Effect 그룹/occurrence를 참조한다.
spawnIntervalMs=2000, followSpeedScale=0.5는 사용자 요청 튜닝값이며 원작 근거로 분류하지 않는다.
이 판정은 Collider와 Success/Fail/Timeout 결과를 소유하지 않는다. 새 값은 기존
Composition parse/validate/serialize와 Workbench Apply/Save를 통과한다. 정의 선택만 한
미완료 저작은 보존하고 실제 publish는 하나 이상의 유효한 template을 요구한다.

Python projector는 저작된 child의 상대 시간·TRS·수명을 보존한 targetedCombatVisuals를
기존 patternbindings 문서에 기록한다. 서버가 제어하는 source row는 static 재생에서 제외하며
source JSON은 보존한다. Server는 visual ID와 수명만 받아 기존 CombatObject spawn/snapshot/
despawn을 사용한다. MainApp이 소유하는 PresentationPlayer를 실제 arena replication에 연결한다.

현재 Save 충돌 복구용 revision816은 사용자가 Save할 때까지 변경하지 않는다. 새 Duration의
저장값을 확인한 뒤 바주카 Composition 등록을 합치고 변경된 authoring을 검증·publish한다.
변경 TU 컴파일, JSON 왕복·오류 보존과 기존 publisher/server 계약 검사를 수행한다. Client
실행·화면 확인은 사용자가 수행한다. 새 C++ 파일/project 등록은 없다.

## G21-Server. 쇼타임 XZ 순간이동과 서버 대상 방향

사용자 요청 좌표 X=2.57, Z=952.27은 기존 TRIGGER의 새 BOSS_TELEPORT_XZ 값으로
저장한다. teleportPosition의 Y=1.30은 저작 참고값이며 서버 캐릭터 높이에 대입하지 않는다.
기존 PATTERNMECHANICTRIGGER 행과 snapshot을 재사용한다. Client 문서 검증·저장,
Workbench Trigger 선택·XZ 입력, Python projection과 Gameplay publisher 및 Server parser를
같이 연결한다. 절대 위치 BossMotion과의 병용은 거부하고 animation root motion은 허용한다.

Server는 목적지 navigation과 실제 보스 크기의 destination overlap을 먼저 검사한다.
성공하면 보스 XZ와 이미 캡처한 stage root origin XZ를 같은 차이만큼 옮기며, 지면 기준도
새 위치로 보정한다. 캐릭터 Y, root origin Y, 애니메이션·stage·pattern 시각과 ID는 보존한다.
실패하면 위치와 루트 기준을 모두 유지한다. Client의 기존 snapshot 소비자는 같은 action
시퀀스의 위치·yaw만 반영하므로 새 Play_NetworkAction이나 vertical root reset을 만들지 않는다.

SHOWTIME_PLAYER_TARGETS 활성 duration에서는 기존 서버 pattern target을 유지하며 현재
좌표를 향해 보스 yaw를 갱신한다. 유효한 대상이 사라지면 기존 RETARGET_RANDOM_ALIVE의
살아 있는 전투 준비 player 선택 함수를 재사용한다. 여러 플레이어의 순회 순서가 몸 회전을
덮어쓰지 않으며, duration 밖에서는 새 회전을 하지 않는다. fixed/tracking 그룹의 map 축은
기존대로 보존한다. 새 Shared packet과 damage는 없다.

기존 Server 계약 검사에 Y·root clock 보존, 다음 root frame의 이동 유지, 잘못된 목적지
rollback, 대상 유지·사망 재선택·duration 종료를 추가한다. 변경 C++는 out에 격리 컴파일하고
Python/PowerShell의 실제 저장·projection 행을 검사한다. logic63 변경은 최신 source의 SHA와
함께 out 후보로만 준비하며 원본 JSON CAS 반영·제품 빌드·publish는 root가 담당한다.
새 C++ 파일과 project/filter 등록은 없다.

## G21-Bomb. 해골 폭탄의 사용자 수명과 원본 파란 폭발

현재 저장된 P35.presentation.638은 낙하 폭탄을 5290ms 동안 배치하지만 원본 복원
문서의 ModelCue와 세 심지 emitter의 배출 구간은 2초다. 같은 문서만 수정하여 ModelCue의
visible duration과 원본 EmitterLoops=0 요소의 유한 배출 구간을 5.29초로 맞춘다.
Bomb_respawn_1의 2초 source animation, holdLastFrame, scale3, b_body와 fx_01 부착,
개별 입자 수명·속도·재질은 보존한다. 낙하를 느리게 늘리거나 반복하지 않는다.

원본 Action4223102의 Par_X_RHCN_Exp_02 first LOD 18개 요소를 기존 source importer와
native material/VF 경로로 복원한다. source HidePawn5초와 폭발5.010초의 간격을 보존하여
사용자 폭탄 박스 종료 10ms 뒤에 별도 폭발 occurrence를 추가한다. source Exp01/02의
색·재질 비교와 NPC480712의 AI selector 미확정은 결과에서 구분한다. 사용자 5290ms를
폭발 tail 길이로 덮어쓰지 않고 별도 occurrence가 원본 입자 tail을 소유한다.

Authored Effect, Catalog/ResourceTree, Composition resource·occurrence와 프로젝트 None
등록은 최신 파일 hash를 포함한 out 후보로 준비한다. 원본 정본 merge/CAS와 publisher는
root가 소유한다. 현재 source shader/table의 기존 행을 보존하며 신규 native4개만 별도로
준비한다. 공유 C++의 duration 정책이나 기존 순수 심지·빙고 문서를 바꾸지 않는다.
실제 CModel의 착지 hold와 기존 CPU Playback의 2초 이후 심지 emission, 폭발의 source
material/geometry와 tail을 최소 수치 검사한다. Client/UI와 제품 빌드는 실행하지 않는다.

Exp02는 Action4223102 notify012의 Color=[1,1,1], source scale2와 UE Z=2.5cm를
정확히 소비한다. 후보의 모든 element에 runtime Y=0.025m와 scale2를 한 번만 적용한다.
native program3680~3683은 기존1359개 함수와1264개 table 행을 보존한 out 분할 shader와
table 후보로 검증하며 ParticleKouku3648의 실제 제품 carrier를 컴파일한다.
새 Effect JSON의 None/96.DataFiles와 새 native carrier의 프로젝트 등록을 함께 준비한다.

## G22-Sector. 기존 native Fan 경계의 노란 외곽선 element

현재11m/45도 warning과 같은 native3602의 방사 경계·원호식을 사용한다. 별도 source
ParticleSystem을 찾았다고 표시하지 않고, 사용자 요청에 따른 노란색 경계 강조 layer로
명시한다. `kouku.showtime.warning.sector.outline`을 기존 warning의 source recipe,
TRS·projection volume·angle·inner0·fade track과 같은 값으로 추가한다. 원본 warning은
보존하고 새 layer의 색만 yellow로 지정한다. 별도 geometry/shader/runtime는 추가하지 않는다.

generator가 새 layer를 한 번만 생성하도록 stable ID로 검증한다. 기존 사용자 outline이
있으면 덮어쓰지 않으며 warning-only 단계도 이 layer를 보존한다. 정본 Effect JSON은
외부 저장과 충돌하지 않도록 source hash와 element append patch가 있는 out 후보로 준비한다.
native 수식의 내부/원호/양쪽 경계 alpha와 실제 CPU Playback의 두 decal matrix·수명·방향을
검사한다. occurrence 전체시간 맞춤은 통합 담당의 기존 runtime 경로에서 처리한다.

## G23-Bomb. 수명 끝에 원본 ModelCue 애니메이션 1회 재생

**사용자 정정으로 철회 / 제품 미적용.** 사용자가 폭탄 복제 후 위치를 옮기지 않아 겹쳤으며 기존 폭탄 애니메이션과 대형 폭발 크기가 정상임을 확인했다. 아래는 취소된 후보의 준비·검증 기록이다. `endAlignedAnimation` 관련 이번 7개 파일의 field/codec/runtime/UI 변경은 지정 구문만 제거했고, 폭탄·폭발 정본 JSON은 변경하지 않았다. sector 외곽선과 generator 변경은 유지한다. 이 후보를 설치하거나 재활성화하지 않는다.

optional `endAlignedAnimation`은 기본 false다. true이면 CModel이 읽은 실제 clip 길이로 cue 종료에서 역산하여, 그 전에는 첫 pose를 유지하고 마지막 구간에서 원래 속도로 한 번 재생한다. 5.29초 cue와 2초 clip은 3.29초 대기 뒤 낙하한다. 기존 `Sample_ModelCuePose`를 쓰는 몸 렌더링과 심지 bone anchor 모두 같은 clock을 소비한다. 별도 심지 위치 보정은 없다.

Effect 문서 optional bool parse/serialize/validation, 준비된 실제 CModel clip과 cue 길이 검증, Model Cue detail 선택까지 연결한다. loop와 병용하거나 cue가 clip보다 짧으면 명시 거부한다. 기존 옵션 없는 cue와 hold/loop 동작은 유지한다. 새 C++ 파일이나 프로젝트 항목은 없다. 정본 drop JSON은 쓰지 않고 최신 hash의 out 후보를 root에 전달한다.

최소 검증은 실제 codec 왕복·이전 문서 기본값·잘못된 조합 거부, 현재 CModel의 첫 pose hold/끝의 원본 2초 재생·심지 anchor 동등성이다. 변경된 TU는 out 격리 컴파일한다. Client/UI 및 Product 빌드는 실행하지 않는다.

G23 설치 후보는 root의 전체 박스 source-clock fit와 맞추기 위해 ModelCue visible duration을 기존 전체 Effect 수명 5.645초로 맞춘다. 심지의 5.29초 배출과 0.355초 tail은 그대로다. 실제 source pose는 3.645초 대기 후 원본 2초를 재생한다. 박스를 6.355초로 fit하면 source clock 전체가 같은 비율로 늘어나므로 애니메이션의 표시 시간은 약 2.25155초다. 이것은 원본 clip 길이 변경과 구별한다.
