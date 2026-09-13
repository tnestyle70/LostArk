# 스킬별 Full Restore Bloom Intensity 구현 계획

## G00. 목표와 현재 실행 경로

모든 Full Restore 스킬과 존재하는 1·2·3·4 단계, clip별 버전, 통합 버전에 각각 다른 bloom intensity를 저장한다. Alt+V와 W는 사용자 설명의 예시이며 구현 범위를 제한하지 않는다. Effect Detail에서 선택한 문서의 값을 바꾸면 그 문서 전체 재생에 즉시 반영한다. 값의 소유자는 stable effectAssetId로 식별하는 V1 Effect 문서 하나다. 동시에 재생하는 다른 문서는 자기 값을 유지한다.

Authored V1 문서 919개를 전수 조사했다. 명시 Full Restore 124개(Artist 18, DimensionMaster 14, LanceMaster 47, Warlord 27, Kouku 18)와 동일한 전체 element 구성을 가진 festival 파생본 1개, 총 125개에 독립된 root 행을 추가한다. catalog나 saved tree에 등록되지 않은 문서도 제외하지 않는다. 관련 imported/tuning 문서도 같은 편집 기능을 사용하며, 사용자 지정에 따라 root 행이 없는 기존 문서는 기본값 1.3을 사용한다. 없는 단계나 새로운 스킬을 임의 생성하지 않는다.

현재 `CRenderer::Draw`는 SceneHDR에 배경·모델·이펙트를 합치고 ScreenPosts 뒤에 `Render_Bloom`을 호출한다. `Shader_Deferred.hlsl::Extract_Bloom`은 전체 HDR 화면의 밝은 부분을 추출하고 `Resolve_FinalLDR`가 전역 intensity를 곱한다. V1 Emissive/V2 기존 bloom 명칭 값은 radiance 자체를 바꾸므로 이번 스킬별 번짐 조절의 대체 수단이 아니다.

기존 `codex/kouku-authored-finale-popup`의 다른 작업과 사용자 저작 변경을 보존한다. C++ 인코딩·개행을 유지하고 자동 stage/commit하지 않는다. Client·Server는 사용자가 실행 중이며 에이전트가 종료·실행·조작하지 않는다.

## G01. 스킬 문서·Effect Detail·즉시 적용

V1 authored root에 optional `bloomIntensity` 하나를 추가한다. 생략하면 1.3이며 유한한 0~16 범위를 검증한다. 문서 JSON의 값은 그 문서의 모든 요소와 모델 표현에 적용된다. 125개 문서의 신규 행은 사용자 지정 기본값 1.3으로 설정하고 기존 사용자가 조절한 값은 보존한다. 기존 element의 원본 색/Emissive/source material 입력을 변경하지 않는다.

Effect Detail에서 선택한 스킬의 단일 슬라이더로 편집한다. 편집 중 값은 현재 문서와 실행 중인 해당 preview에 즉시 전달하고 Save가 원자적 저장·재로드를 보장한다. 다른 열린/재생 중 문서의 값이나 Rendering Benchmark 전역 설정을 덮어쓰지 않는다. 값 변경 때문에 원본 shader를 다시 컴파일하거나 전체 Resources를 재준비하지 않는다.

`Effect_AuthoringDocument`, `Effect_DocumentCodec`, V1 Tool과 실제 Object/Playback/DocumentRenderer 소비자를 연결한다. draw마다 그 문서의 값이 바인딩되고 다음 draw의 값이 이전 스킬에서 누수되지 않아야 한다. 최신 실행 경로에서 sourceRecipe/native와 generic, mesh/particle/decal/trail 및 실제 사용되는 ModelCue/ScreenPost의 전달 범위를 확인한다.

## G02. 공용 bloom 출력과 합성

Engine Renderer와 기존 Effect carrier shader에 공용 effect bloom 출력을 연결한다. 모든 스킬이 각자의 배율을 적용한 기여를 공용 버퍼에 모으며 스킬별 별도 버퍼나 별도 renderer를 생성하지 않는다. 기본 SceneHDR 색 출력은 유지하고 기존 blur/final 후처리를 재사용한다.

전체 화면 bloom과 새 effect bloom이 중복 합산되지 않도록 기존 밝기 추출 입력을 분리한다. 원본 HDR 색과 alpha/additive blend, distortion/depth 계약을 유지한다. 기존 비Effect 출력과 화면 후처리도 새 MRT 슬롯에 잘못된 값을 쓰거나 이전 프레임을 읽지 않도록 명시적으로 처리한다. 여러 반투명 효과가 겹칠 때의 기여와 배율1의 기존 상태 호환 범위를 실제 수치로 확인한다.

draw별 색의 bright-pass 결과에 해당 문서 intensity를 곱한 값을 RT2에 출력한다. RT0 HDR 색과 RT1 distortion 출력은 유지한다. bloom을 합친 뒤 threshold를 다시 적용하면 intensity가 비선형적으로 변하므로 추출 완료된 기여만 기존 blur로 넘긴다. 배율1에서 단독 밝은 불투명 출력은 기존과 맞지만, 반투명·어두운 출력의 합성은 기존 전체 화면 추출과 달라질 수 있다. 이 차이는 원본 HDR 색을 바꾸지 않고 문서별 bloom 기여를 독립적으로 조절하기 위한 합성 경계다.

문서의1.3은 해당 스킬 자체의 intensity이며 전역 intensity의 추가 배율이 아니다. 비Effect의 기여에는 장면 전역 intensity를 먼저 적용하고, V1에는 문서 intensity를 적용한 뒤 같은 버퍼에서 합성한다. 최종 단계에서 전역 intensity를 다시 곱하지 않는다. 전역 Bloom enable·threshold·soft knee·scatter와 scene profile 데이터는 보존한다.

실제 소유 파일은 `Engine/Private/Renderer.cpp`, 대응 헤더·render-output 계약과 기존 Engine/Client HLSL carrier다. Client C++ 소비자와 shader의 이름·값 기본값·출력 계약을 함께 검증한다. 새 C++ 파일이 필요한 경우에만 프로젝트/filters의 기존 위치에 등록한다.

## G03. 검증과 완료 증거

기존 codec의 생략 기본값, 서로 다른 두 문서 값, 저장·재로드, invalid/NaN/범위 오류와 실패 시 기존 문서 보존을 확인한다. 실제 draw 상수 전달에서 Alt+V/W의 독립 값과 연속 draw 누수를 확인한다. 전체 스킬 preview의 시간·선택 상태를 유지하는 live edit 경로를 검증한다.

변경 CPP·shader의 최소 Debug 컴파일, 필요한 컴파일 shader 배포를 수행한다. 사용자가 실행 중인 EXE/DLL을 잠근 경우 독립 out 컴파일 결과와 제품 반영 미완료를 구분하며 임의 종료하지 않는다. JSON/XML parse와 `git diff --check`, source/target 경로를 확인한다.

GPU 검증은 창·화면 캡처 없이 필요한 작은 수치 검사로 한다. 같은 HDR 색을 유지하면서 서로 다른 스킬의 bloom 값만 바뀌는지, 0값과 기본값, 동시 효과가 독립적인지 확인한다. 화면의 번짐·색·사용성 최종 판정은 사용자가 한다. 실제 실행 증거와 남은 경계는 대응 RESULT에 기록한다.
