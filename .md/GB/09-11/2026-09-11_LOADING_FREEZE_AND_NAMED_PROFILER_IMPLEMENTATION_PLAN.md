# 로딩 정지 제거와 이름 있는 Profiler 캡처

## G00. 현재 실측과 목표

사용자는 Character Select 직업 변경의 약 20초 정지, 쿠크 진입 후 정지, 창술사·차원술사 ALT V 프레임 저하를 보고했다. 실제 20:09 캡처에는 frame15598 Engine.LevelUpdate 23.32초, frame15611 Effect.Prewarm.Advance 13.30초가 있다. 20:06 캡처의 activation 구간 MainApp.LevelAndEnvironment.Update는 3.36초다. 별도로 Profiler.Panel.Refresh가 37~173ms를 사용한다. 계측 부모·자식 시간을 합산하지 않는다.

현재 runtime은 Debug/Release 모두 CSO만 소비하며 HLSL source compile은 없다. 첫 class 선택의 Ensure_Prototypes는 main에서 body/animation/equipment/weapon을 동기 로드한다. Runtime Effect incremental queue는 target 수만 제한하고 target 하나의 문서·resource 준비 전체는 main에서 수행한다. 쿠크 worldsequence 문서는 약 9.8MB이며 activation에서 parse한다. 이 경계들을 기존 경로의 stage/commit으로 분리한다.

## G01. Profiler 캡처 이름·목록·삭제

ProfilerTool/ProfilerCaptureIO 기존 파일을 확장한다. UTF-8 이름, timestamp/frame/PID/sequence 기반 파일명과 no-replace commit으로 기존 파일을 보존한다. 최근 최대 1200프레임 snapshot 계약은 유지한다. Saved JSON 목록은 최초/Refresh/저장·삭제 완료 때만 갱신하고 매 프레임 파일 I/O를 하지 않는다. 선택 삭제는 capture root 안의 동일한 일반 JSON 파일인지 identity와 경로를 확인한 뒤 수행한다. 실패하면 목록·선택·기존 캡처를 유지한다. 사용자 파일을 에이전트가 대신 삭제하지 않는다.

## G02. 직업 asset 준비

PlayableCharacterAssetService의 기존 CModel staging을 Prepare/Commit으로 나눈다. worker는 디코딩·device-only resource 생성만 수행하고 main이 Add_Prototypes batch를 commit한다. ready set mutex는 디코딩 전체를 감싸지 않는다. Character Select는 필요한 class만 준비하고 최신 선택을 유지하며, 준비 뒤 typed Server class-change command를 제출한다. 기존 character는 실패·대기 중 보존한다. 기존 Loader Ensure 호출은 같은 prepare core를 재사용한다. 취소와 bounded 종료, 오래된 준비 결과의 폐기 및 worker resource 해제를 포함한다. Replication cold player spawn도 최신 entity snapshot을 보존하며 같은 준비 서비스를 사용한다. SkillBindings/EffectCues/FaceSliders/FaceMorph CPU 입력은 immutable prepared 상태로 모델과 함께 공개하고 인스턴스별 가변 상태를 분리한다.

## G03. Effect 준비 worker 재사용

Loader의 기존 EffectLoadPreparationJob worker 실행 루프를 공용 Product preparation 실행 함수로 옮겨 Loader와 runtime prewarm이 같은 stage/result/ACK/commit 계약을 사용하게 한다. Runtime은 단일 지속 worker와 bounded 작업 슬롯을 소유하고 main Advance는 준비 요청·결과 회수·짧은 commit만 수행한다. Find→parse→Prepare_ProductTarget를 프레임에서 직접 실행하지 않는다. catalog revision/level 전환/취소와 queue owner를 확인하고 실패 receipt를 유지한다. 기존 Effect renderer와 Prototype 경로를 그대로 사용한다.

## G04. 쿠크 activation 준비

WorldSequence의 기존 parser/validator를 Loader에서 호출해 typed document를 stage하고, activation은 실제 placement/deploy target 계약을 확인해 준비된 문서를 소비한다. authoring Load/Save와 제품 loading을 같은 parser로 유지하고 별도 runtime 모델을 만들지 않는다. 필요 없는 전체 Area/class 선로드를 하지 않는다. 쿠크 초기 visibility lookup의 반복 순회와 첫 Product presentation 문서 처리도 실제 호출 지점에 맞춰 정리·계측한다.

## G05. Profiler와 shader 반복 비용

Profiler 집계가 매 refresh마다 모든 raw scope를 정렬하고 map lookup하는 비용을 실제 데이터로 비교한다. 동일한 Inclusive/Self/Max/Calls와 worker nesting을 유지하면서 completion order·dense aggregate를 이용해 반복 처리를 줄인다. invalid/missing scope와 zero duration, window 분모를 검증한다. CShader의 같은 input signature에 대한 반복 input layout 생성은 exact signature 기준으로 공유하고 bytecode read/FX11 create/bindings/layout 비용을 계측한다.

## G06. 스킬 지속 비용과 검증

사용자가 제공한 최신 ALT V 캡처에서 load stall과 정상 재생 프레임, Profiler 자체 간섭을 분리한다. 새로운 수치가 지목하는 반복 연산·할당·복사·제출을 먼저 고친다. 확인된 전체 viewport PickPos readback은 실제 피킹 요청의 1픽셀 readback으로 바꾸고 좌표·resource·RowPitch·no-hit을 유지한다. Spawn 0개의 불필요한 행렬 계산과 immutable particle module 반복 검색은 기존 prepared cache로 줄이며 fixed-step/RNG 순서를 보존한다. work stealing/fiber/GPU particle을 이름만으로 도입하지 않고 실제 순수 CPU 작업과 dependency를 근거로 결정한다.

기존 인코딩/dirty 변경을 보존한다. 새 제품 CPP 파일을 추가하면 해당 project/filter에만 등록한다. 현재는 기존 파일 확장을 우선한다. 실제 임시 데이터의 save/list/delete, stale/cancel/rollback, 준비 worker와 main commit 경계, aggregate 결과 동일성, focused Debug compile을 검증한다. 전체 제품 빌드는 사용자 실행/빌드 프로세스와 충돌하지 않는 때 수행하며, Client/UI 조작과 화면 판정은 사용자가 한다. RESULT는 소스 반영·컴파일·수치 검사·사용자 화면 및 FPS를 분리한다.
