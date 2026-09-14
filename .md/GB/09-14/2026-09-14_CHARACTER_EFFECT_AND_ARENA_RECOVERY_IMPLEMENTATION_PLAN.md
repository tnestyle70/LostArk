# 캐릭터 이펙트와 아레나 후속 구현 계획

## G00. 현재 기준과 작업 경계

현재 브랜치는 `pattern3-rendering-restore`다. 시작 시 Character/MainApp/Loader/SequenceViewer, 쿠크 Composition·Encounter·맵 연출과 창술사 34090 문서에 미커밋 변경이 있다. 기존 편집을 보존하고 요청과 연결되는 부분만 추가한다. Client/UI는 실행·조작하지 않는다.

셰이더 분리, 쿠크 편집기·재생, Server 마리오 진행은 각각 별도 PLAN/RESULT에서 구현과 검증을 소유한다. 이 문서는 캐릭터 local space, 차원술사 시작 장면, 쿠크·베른 맵 및 Character Select 재질 확대를 연결한다.

## G01. 캐릭터 이펙트의 출생 좌표

실제 skillbinding·effect cue·EffectCatalog·Authored 참조를 조사한다. 여섯 playable class의 현재 저작 문서에서 켜진 particle localSpace를 끄고, 실제 playback의 SpawnRootWorld 경로를 대조한다. sourceRecipe와 actionCueAttachment가 localSpace 값을 다시 생성하거나 별도 owner 추종을 적용하는지 확인한다. 원본 Reference/Imported 근거를 임의 변경하지 않는다. 변경 목록은 stable effect/element ID로 기록한다.

## G02. 차원술사 Alt V 시작 장면

`CEffectDocumentRenderer::Stage_PreparedInternal`은 이미 완료된 HDR/Bloom을 occurrence 시작에 저장한다. 그러나 `Build_NativeScreenPost`의 cube 프로필은 별도 비어 있는 capture 상태를 생성하고 첫 Render에서 다시 캡처한다. 이 프로필은 최초 pair를 소비하도록 연결한다. 화면 수축 중심은 화면 중앙으로 고정하여 카메라·world target 투영의 아래 방향 이동이 수축에 섞이지 않게 한다. 포탈의 전환 시점 캡처와 실패·준비 프레임 계약은 유지한다.

수정 대상은 기존 Renderer staging/rendering helper와 필요한 실제 capture 소비자다. 새 C++ translation unit 없이 기존 프로젝트 등록을 사용한다. 실제 capture/binder의 synthetic HDR/Bloom 수치 검증과 Product Build로 확인한다.

## G03. 맵 렌더링 및 베른 네비게이션

기존 맵 공통 최적화는 이미 Bern·Kouku에도 연결되어 있다. Area별 배치·재질·visibility·shader 선택과 최근 구조화 profiler를 확인해 남은 반복 비용을 줄인다. 시작 위치만의 숫자·특수 분기를 다른 위치에 복사하지 않는다.

Bern navigation은 기존 source/paint와 runtime policy, 실제 지형을 대조한다. 현재의 좁은 범위, 잘못된 상층 선택, 끊긴 통행 성분을 조사하고 실제 바닥·계단에 근거한 연결만 복구한다. 벽·낭떠러지를 전부 WALKABLE로 만드는 방식은 사용하지 않는다. publisher Validate/Publish와 Server 경로 검증을 수행한다.

## G04. Character Select 전체 맵 재질

현재 803배치·87 사용 material slot 중 source binding은 9개다. 기존 source MIC·geometry·UV1·RNM·환경 자료와 현재 지원 family를 연결하여 나머지를 확대한다. material identity가 같은 모델을 재사용하고 placement별 조명은 구분한다. 실제 WModel/texture 경로와 publisher parse·validate·stage·commit을 확인한다. source-exact 근거가 없는 family는 임의 PBR 대체로 완료 처리하지 않는다.

추출된 현재 source는55 meshes/62 ordered-material variants/92 사용 slot이며, 재질이 없는 helper를
제외한78 MIC의14 terminal family와 실제 static switch를 대조한다. 기존9행의 연결과 사용자 Transform은
보존한다. 일반 family의 source 값·texture·pass·steady emissive·metallic diffuse mask·wind 소비까지
연결하고, 미지원 masked/투명 분기는 원본 shader cache로 확인한 뒤 기존 CModel/CMaterial에 확장한다.
원본 UV1의 반복 추출 일치와 RNM799배치/58 texture,29 environment override를 따로 검증한다.
반복 추출에서 달라지는 tangent.w/COLOR0를 원본 채널로 주장하지 않는다.

## G05. 검증과 인계

변경 JSON/XML parse, 해당 publisher/실제 CPU·D3D 수치 계약, 정상 증분 Debug Product Build, `git diff --check`를 실행한다. 실행한 검사와 아직 사용자 확인 전인 외형·FPS·이동·타임라인 재생을 RESULT에 분리한다. 새로운 Resources는 상대 경로와 실제 설치 상태를 기록하고 Git에는 넣지 않는다. 대규모 기존 dirty 파일은 자동 stage/commit하지 않는다.
