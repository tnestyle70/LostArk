# Artist D/T/V/Alt V 원본 복원 구현 계획

2026-09-09 사용자 승인 범위. 현재 `codex/dimensionmaster-tool-round3`, HEAD `591012dbebf7eeab0b660baec42852b9396e77d4`의 기존 dirty 변경을 보존한다. 본 작업은 Artist 신규 산출물만 소유하며 공용 Codec/Renderer/Playback/Tool/프로젝트 연결은 root가 담당한다.

## G00. 실제 입력과 보존 경계

`PlayerSkills.json → Artist.skillbindings.json → Artist.animevents → Authored effect → CEffectDocumentRenderer`가 실제 소비 경로다. 현재 D31490은68개 element/modelCue0, T31950은23개/modelCue0, V31910은46개, Alt V31930은2문서97개다. 기존 unified와 사용자 손튜닝은 비교본으로 보존하고 별도 `.full.restore`를 만든다. 원본 설치판 Action/UPK를 읽고 raw enabled notify, 첫 LOD, 실제 MIC/PS/VS/texture 입력을 합친다. 동일 asset/time의 활성·비활성 notify는 순서로1:1 join하며 단순 이름 dict 덮어쓰기를 금지한다.

## G01. 묵호와 미르 본체

현재 Reference `Artist.projectiles:244`는 D314900의 `SK_SDM_TIG_00.Mesh.SK_SDM_TIG_00_SK`, :286은 T319500의 `SK_SDM_DRA_00.Mesh.SK_SDM_DRA_00_SK`를 명시한다. 실제 설치 UPK에 TIG skeletalmesh1/animation7/texture4, DRA skeletalmesh1/animation2/texture3을 확인했다. 두 모델은 현재 Resources에 없고 D/T modelCues도 비어 있다.

원본 ActorX mesh/animation/UV를 쿠킹하고 기존 E 두루미의 CModel modelCue 재생 경로로 연결한다. TIG는 원본1.1466초 출생·6.5m/s·11m 거리와 clip, T는0.6초 출생·FIXAREA·용 clip을 검증한다. mesh UV/normal/tangent 및 source MIC의 D/N/S/E, 색/투명도/발광 입력을 보존한다. 원본 shader를 회수해 기존 native 실행 계열로 연결한다. 단순 diffuse/tint를 원본 재질 완료로 기록하지 않는다.

## G02. T helix와 V/Alt V 전체 발생

T MakeFlow03 helix3개와 ribbon을 본체와 별도로 복원한다. V/Alt V는 활성 world/camera particle graph를 모두 조사하고, 사용자 핵심인 노랑·빨강 원형 회전 집결의 MIC/UV/time/color/flow를 우선 연결한다. 원본 활성 occurrence가 현재 후보에 없으면 first LOD에서 추가한다. 공통 입력은 차원술사의 cm→m/source basis/CDO·상속 분포/dynamic 값·fixed tick을 재사용한다.

새 Artist native 번호는460~559와820~939를 사용한다. 새 header/HLSLI만 본 작업이 편집하며 Renderer/Codec/프로젝트 hook은 root가 연결한다. 같은 부모 이름만으로 원본 PS 동일성을 추정하지 않는다. 원본 CameraPPE는 일반 시각 이펙트로 포함하되 사용자가 제외한 camera movement/FOV/shake와 Sequencer 확장은 수정하지 않는다. 별도 Sky_Mirror StaticMesh 두 호출은 기존 ordinary MESH와 원본 native material로 연결한다.

## G03. 단독 재생 가능한 full restore

각 행의 material/resource/geometry/attachment/motion을 닫고 전체 수명 생성·finite와 shader 컴파일을 확인한다. 끝내 단독 재생이 불가능한 요소는 원인과 source ID를 RESULT에 기록하고 새 full restore에서 제외한다. 핵심 호랑이·용을 구현하지 않고 제거해 완료하는 것은 허용하지 않는다. 원본 unified, 비교용 source, Resources를 삭제하지 않는다.

## G04. 검증과 인계

변경 JSON parse/stable ID·resource 실물, native shader FXC, 필요한 CPU 검사, root의 제품 빌드와 `git diff --check`를 기록한다. Agent는 Client/UI를 실행·조작·캡처하지 않는다. 사용자에게 server-host `Server + Client` profile의 Ctrl+F5, Character Select Artist → F1 Effect Tool V1 → 해당 full restore → Solo/Play All 경로를 인계한다. 실제 화면 일치와 사용자 visual PASS는 사용자 확인 전으로 남긴다.

## G05. 구현 산출물과 재생 경계

`Tools/EffectPipeline/generate_artist_native_runtime_shader.py`, `generate_artist_native_runtime_contract.py`, `generate_artist_full_restore_documents.py`가 회수한 source receipt와 candidate를 읽어 각각 HLSL, exact material contract, full.restore 문서를 생성한다. 생성 도구는 원본 PS 산술과 uniform 입력을 유지하며 runtime interpreter를 추가하지 않는다. 두 CModel modelCue는 optional Material을 기존 CModel/숨김 pose provider에 전달한다. TIG 수명은11m/6.5m/s, DRA는 원본 projectile5초와 clip2초를 분리해 holdLastFrame을 사용한다.

원본 Random SubUV38행은 source Required의 RandomImageChanges/RandomImageTime과 mesh bScaleUV를 그대로 소비하는 공용 확장 뒤에 최종 Load/Save/Solo를 확인한다. source emitterDuration500인4행은 source metadata 상한과 actual element lifetime을 분리한다. 이 두 공용 변경과 실제 animevents의 unified/full.restore 선택은 root 담당이며 Artist 자료 생성만으로 제품 입력 반영을 완료 처리하지 않는다.

## G06. 09-10 전체 슬롯과 소환 애니메이션 후속

사용자 요청을 LMB4단계, Q/W/E/R/A/S/D/F/T/V/Z/Alt V의 full.restore 선택으로 확장한다. 기존 unified와 이전 full.restore의 사용자 튜닝은 보존한다. 일반9개 스킬(LMB/Q/W/E/R/A/S/F/Z)은 현재 설치 Action의 활성 notify와 첫 LOD를 다시 읽어230개 occurrence를 확정했으며, 원본 MIC/map/VF/texture와 module CDO를 재조인한 새 native1600번대 프로그램을 연결한다. 기존 D/T/V/Alt V174개 프로그램은 원래 번호를 유지한다. 실제 source shader 입력을 닫지 못한 요소만 stable source ID와 이유를 결과에 분리한다.

TIG의 실제 sk_cloudtiger clip은0.733333초이며 원본 bone animation clock을 정지 pose로 대체하지 않는다. DRA는 원본부터 XZ 바닥에 놓이는 얇은 skin plane이고2초 clip의 vertex 위치가 시간별로 변한다. 두 자산을 다른 임의 모델로 바꾸지 않고 원본 skin pose와 제품 CModel clip sampling을 먼저 대조한다. 원본의1000tick/s와 CAnimation의 고정30tick/s 차이를 source cook의 모든 key time·duration 정규화로 교정한다. DRA 원본 projectile은 FIXAREA/speed0이므로 이동을 임의로 추가하지 않고 바닥을 따라 변하는2초 skin motion과 원본5초 수명을 보존한다. Server damage/gameplay는 바꾸지 않는다. Alt V 꽃밭은 기존 연결을 유지하며 이번 일반 슬롯 뒤에만 추가 비용을 쓴다.

새 C++ 파일을 추가하지 않는다. Artist material header/HLSLI와 신규 Authored JSON의 프로젝트/카탈로그/ResourceTree 및 실제 animevents 연결은 공용 통합 담당이 수행한다. JSON parse, 상대 asset 실물, source pose 수치와 필요한 shader/제품 최소 컴파일을 분리한다. Client/UI 실행·캡처와 최종 시각 판정은 사용자만 수행한다.

## G07. 사용자 재검토의 미르새김 몸통 폭과 Alt V 단일 restore

미르새김31950의 설치 DRA는 어제 non-root 회전 부호를 교정한 모델과 byte exact이며 원본 projectile scale1/FIXAREA speed0과도 일치한다. 원본 GPU skin VS에 별도 폭 WPO는 없다. 이전 수정은 뒤집힌 면을 고쳤고 bind 폭0.190895m를 유지했다. 사용자가 다시 요청한 두꺼운 몸통은 원본 회수값과 구분한 PROJECT_AUTHORED 횡폭5배로 적용한다. 기존 DRA cooker의 옵션으로 정점의 bind 횡폭만0.954474m로 늘리고 골격·두 clip·가중치·중심 경로와 사용자 cue/별도 helix3개를 보존한다. 기존 폭5 후보의122pose 수치 검사를 재사용한다.

Alt V31930은 사용자 관찰에서 clip1의 카메라는 정상이고 clip2의 꽃밭은 invalid로 재생되지 않는다. 실제 `CEffectDocumentCodec::Validate_Drawable`의 Element 루프가 simulation-only provider에도 일반 Base texture를 요구하는 결함을 고친다. provider의 graph/순서 검증은 계속 수행하고 drawable binding 검사만 제외한다. 꽃밭 provider32행을 지우거나 일반 sprite로 위장하지 않는다.

clip1의52행과 clip2의202행을 기존 source clock에 따라 하나의 `effect.artist.skill.31930.full.restore`로 합친다. clip2는 실제 clip1 길이1733.333ms 뒤로 옮기고, Element 시간·provider 참조·카메라 clock을 함께 맞춘다. 정상인 컷신 카메라4행/1065key와254Element의 원래 배치·재질을 보존한다. animevent는 첫 clip에서 한 번 호출하며 뒤 clip에서는 중복 생성하지 않는다. Tool Catalog/ResourceTree와 project None 목록도 이 단일 정본을 사용한다. 기존 두 파일과 사용자 저장본은 검토 가능한 백업으로 보존한다.

새 C++ 파일은 없다. 기존 full codec/Playback과 원본 카메라 sampler를 재사용해 저장·재로드·full drawable·provider closure·시간 경계와 카메라 값 보존을 확인한다. Resources 설치와 Data 변경은 현재 bytes가 그대로인 경우에만 commit한다. Product 빌드 뒤 사용자 재실행에서 두꺼운 몸통과 Alt V의 카메라→꽃밭 전체 재생을 확인한다.

## G09. 도화가 전체 스킬 이펙트 미발생 긴급 수정

사용자가 실행 중인 EXE에서 도화가 스킬 이펙트가 전부 발생하지 않는다고 보고했다. Alt V의 두 EFFECT 행을 한 행으로 합친 현재 Artist.animevents는 실제 이벤트1047행인데 헤더는1048을 선언한다. 실제 소비자인 CAnimationEffectCueDocument::Load_FromText가 행 수 불일치에서 문서 전체를 거부하므로 Character::Load_EffectCues와 Product prewarm 모두 정상 cue를 등록하지 못한다.

Data/Animation/Authored/Artist/Artist.animevents의 헤더만1047로 교정한다. 기존18개 제품 EFFECT와 나머지 이벤트, 이펙트 문서 및 런타임의 행 수 검증을 유지한다. 실제 로더로18개 cue의 등록과 실패 전후를 확인하고, Catalog 대상과 저작 JSON을 검사한다. 데이터는 CProjectDataRoot의 Data 정본에서 읽으므로 제품 재빌드는 필요 없다. 이미 생성된 캐릭터는 도화가 외 class를 선택한 뒤 도화가로 돌아오거나 재입장하여 새로 로드한다. 실행 중인 Client/Server를 종료하거나 UI를 조작하지 않는다. 쿠크 후속 변경과 차원술사 BA 검토는 보존한 채 별도로 남긴다.

## G10. 기존 Product 선택과 Alt V GPU 발생 수 일치

사용자가 Q/W/R/A/S/F를 이전 Product로 되돌리도록 지정했다. 각 슬롯의 기존 unified를 재사용하며 A는 기존 linear-reveal.unified를 선택한다. R 두 clip의 ba1/ba4 연결을 유지하고 다른 슬롯의 full.restore를 보존한다. Alt V는 Playback이 simulation-only provider를 GPU 발생에서 제외하지만 Renderer의 Resolve_GpuRenderFamily가 sprite로 세어 전체 Render를 거부한다. 공통 family 분류에서 같은 provider를 END로 제외해 개수·순서·실제 draw 소비를 맞춘다. 기존 Codec·Playback·실제 Render 진입을 사용해254행 문서와 provider를 포함한 frame, 일반 문서와 잘못된 frame의 거부를 확인한다. 새 C++ 파일은 없으며 최소 Renderer 컴파일 후 최종 Product 빌드에 포함한다.
