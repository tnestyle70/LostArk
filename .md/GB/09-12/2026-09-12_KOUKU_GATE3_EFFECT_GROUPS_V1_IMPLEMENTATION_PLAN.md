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
