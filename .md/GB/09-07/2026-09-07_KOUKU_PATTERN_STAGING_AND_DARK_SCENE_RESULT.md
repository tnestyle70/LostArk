# 2026-09-07 쿠크 패턴 방향·암전·관문 위치 결과

## G00. 구현 상태

현재 상태는 **코드·정본 수정, Debug Product 빌드, 런타임 배포 및 Server 시작 검증 완료**다. 사용자가 마지막 카드·조커카드 저장 후 Client와 Server를 종료한 것을 확인하고 빌드·배포했다. 실제 Client 화면 확인은 사용자에게 남겼다.

| 요청 | 현재 구현 |
|---|---|
| 무력화 방패·반사 영역 | 두 방패와 두 반사 영역의 local 위치/yaw를 함께 +90도 회전. 사용자 변경 offset을 보존 |
| 진짜 세이튼 찾기 | 중심 (-0.07, 942.33) 주위 네 위치를 -60도 회전. 반지름 약 7.68194m 및 중심 바라보기 정책 보존 |
| 댄스타임·룰렛 | 기존 G1 Saydon spawn yaw 237도에서 -90도인 147도를 패턴 시작에 고정 |
| 암전 | Scene Profile에 optional mapLightIntensityMultiplier 추가. 암전 profile은 0.05 |
| 1·3관문 Move Player | (-2.45, 1.32, 945.17)로 변경. 다른 관문 좌표는 유지 |

Composition revision 84에 사용자가 저장한 변경을 보존하고 요청 필드만 적용해 85로 올렸다. 변경 목록과 네 actor 위치는 `out/KoukuCameraPatternTuning/numerical-evidence.json`에 기록했다. 새 resetBossYawDegrees는 기존 resetBossToSpawn의 optional 부속 값이며 -360~360의 유한수만 허용한다. F1 저장/재로드 → Product → 기존 PATTERNSPAWNRESET row → Server 시작 상태 → snapshot 경로로 전달한다. 필드가 없으면 기존 방향 유지이며 true/clone의 중심 바라보기 정책과 중첩되지 않는다.

카메라와 XZ 회전식 기준으로 사용자가 지정한 -90도 및 -60도 부호를 확인했다. 실제 화면 방향은 사용자 확인 전이다. 네 actor의 회전 후 위치는 모두 walkable이고 blocker가 없다. Move Player 목적지도 walkable이며 Server navigation 높이는 약 1.3176258m로 보정된다.

## G01. 암전 원리와 조작

기존 directional RGB를 낮춰도 enabled 맵 Spot은 별도로 합산된다. 새 mapLightIntensityMultiplier는 Scene Profile 활성화가 성공한 뒤 현재 scene 값으로 commit하고, 맵 조명을 실제 제출할 때 원래 brightness에 한 번 곱한다. 기본값은 1, 허용 범위는 0~4다. 사용자 Spotlight brightness 4에 암전 배율 0.05를 곱하면 0.2가 된다. 맵 조명 정본의 brightness는 바꾸지 않는다. 패턴의 PLAYER/BOSS LIGHT와 Effect 조명에는 적용하지 않으며 profile 복원 시 이전 배율로 돌아온다.

F1 → Rendering Workbench → Light Resources → Scene Profile 선택 → Light Detail의 `Map Light Intensity Multiplier`에서 수정하고 `Save Light` → `Publish Light`로 저장한다. Exposure Multiplier는 전체 scene 노출을 바꾸는 별도 값이다.

현재 Scene Profile 재생은 즉시 Activate_Profile하며 Blend ms 보간은 구현돼 있지 않다. 기존 UI의 오해를 부르는 설명과 label을 reserved/즉시 적용 설명으로 바로잡았다. 이번 변경에 보간 runtime을 추가하지 않았다.

## G02. 사용자 카드 저장 보존

15:30의 revision 163에서는 카드 3 clip을 확인했고, 사용자가 종료 직전 마지막으로 저장한 정본은 revision 366이었다. 마지막 저장본에는 카드와 조커카드가 각각 3 clip이며 clipName을 한글 별명으로 바꾼 상태였다. 실제 WModel lookup ID가 바뀌면 재생 실패하므로 원래 영어 ID 6개를 복구하고 한글 값 6개는 새 optional displayName으로 보존했다. Object Tool은 표시 이름만 편집하고 Native clip은 읽기 전용으로 표시한다.

카드는 총 5,135ms, clip 시작 0/2,000/4,635ms다. 조커카드는 총 3,167ms, 시작 0/2,000/2,667ms다. 마지막 clip은 두 모델 모두 `mn_rhoc_00_sk.ao_att_battle_2_01`의 500ms다. 한글 별명, 시작 시각, loop/rate/hold, Transform, objectMotion과 다른 저장 항목을 보존했다. Instance는 template ID를 참조하므로 별도 clip 복사본 갱신은 필요 없다.

공 모델/텍스처 교정까지 포함한 revision 367을 Area publish했다. 마지막 사용자 저장본은 `out/KoukuPatternStaging/WorldSequences.before-publish.json`, 정확히 15개 필드의 변경 근거는 `world-last-save-preservation.json`에 있다. 현재 Composition에는 카드 world occurrence가 연결돼 있지 않다. 카드 저장·World Object 재생 준비를 Complete Play 편입 완료로 기록하지 않는다.

## G03. 실행한 검증

- 기존 Composition projection focused tests 2개 PASS: spawn reset/world anchor, optional yaw 값의 허용·거부 및 무필드 보존.
- 기존 Rendering publisher 전체 8개 PASS: multiplier 기본값/round-trip, 잘못된 수치·자료형 거절과 이전 runtime bytes 보존 포함. `out/KoukuPatternStaging/rendering-tests.log`.
- 기존 WorldSequence authoring/publisher 검사 26개 PASS: 표시 이름의 한글/빈 값/무필드/UTF-8 byte 범위 및 거부 입력 포함. `out/KoukuPatternStaging/world-sequence-tests.log`.
- World Object 물리 WModel 8개의 110 clip을 읽어 연결된 animationTracks 22개 모두 exact name resolve PASS, 누락 0. `world-object-animation-track-validation.json`.
- Map Area Validate 및 Publish PASS: 3,231 placements / 7 files, worldsequences revision 367. `map-validate.log`, `map-publish.log`.
- Rendering profile revision 18 Publish PASS. 암전 profile mapLightIntensityMultiplier=0.05. `rendering-publish.log`.
- Kouku owner Product/Gameplay Publish 및 projector validate PASS: sourceRevision 85 / 6 Product patterns / 66 stages / 2 outputs. Server KOUKUSAYDONPRODUCTREVISION=85, 댄스/룰렛 PATTERNSPAWNRESET yaw=147 확인. `kouku-publish.log`.
- World Gameplay Validate PASS. 새 optional reset yaw의 기존 consumer 호환성을 확인했으며 무관한 사용자 boss placement source를 수정/배포하지 않았다. `world-gameplay-validate.log`.
- worldsequences/maplights/RenderingProfiles source-runtime JSON 의미 일치 3/3, Composition publisher 전후 source bytes 동일. `final-source-runtime-check.json`.
- 최종 Debug Product 컴파일·링크 및 SDK/shader/runtime DLL 배포 PASS. `product-debug-final.log`, receipt `out/BuildPipeline/runs/20260907T064933718Z-debug-product.json`. 첫 빌드 후 수정 구간의 CRLF 보존을 정리하고 증분 Product까지 재확인했다.
- 새 Server의 `127.0.0.1:17777 --headless --smoke-timeout-ms 1000` 시작·catalog 로드·정상 종료 PASS(exit 0). `server-startup.log`. Client/UI는 실행하지 않았다.
- 변경 JSON parse 및 publisher PowerShell 4개 parse, `git diff --check` PASS. XML 변경 없음.
- 새 C++/project 파일 없음. 기존 Client C++는 UTF-8 및 CRLF를 보존했다.

기존 EngineSDK 인코딩/DirectXTK PDB 경고 및 일부 class hit-shape coverage 경고는 남아 있으며 해당 기능의 컴파일·publisher는 성공했다. 위 상대 로그는 별도 명시가 없으면 `out/KoukuPatternStaging/` 아래다.

## G04. 사용자 화면 확인

1. Visual Studio `Server + Client` profile을 Ctrl+F5로 시작한다. 최종 프로세스 확인에서 두 프로그램은 종료 상태다.
2. Lobby → KoukuSaydon → F1 → KoukuSaydon Complete Play에서 inventory를 다시 읽고 Saved Patterns를 재생한다. 방향·암전·1/3관문 Move Player 위치를 확인한다.
3. F1 Tools → World Object Tool에서 카드·조커카드의 세 clip과 한글 이름을 확인한다.
4. 공 상태 선택 → Object Detail의 Lifetime과 Physics / Motion / Emission의 Arc Height → Apply Vertical Arc → Timeline 드래그/Play → Save → Publish Area로 튜닝한다.

실제 Client 재생·visual PASS는 사용자 확인 전이다. 기존 사용자 dirty 변경을 보존했으며 자동 stage/commit/push는 하지 않았다.

## G05. MN_RHCN_00 공과 물리 Timeline

MN_RHCN_00 패키지와 실제 모델에는 카드처럼 선택할 bone animation clip이 없다. 원본 particle `FX_MN_RPCZ_00_U.par_u_rpcz_ballshoot_ball_01/02`의 21개 위치 표본에서 -20→150→-20cm의 상승·하강이 확인됐다. 각 lifetime은 1.15초/1초다. 이 source가 사용자가 말한 세이튼 등장 occurrence에 정확히 연결됐다고 확정하지 않았다.

설치된 `Effect/KoukuSaydon/Meshes/wp_mn_rhcn_00/mesh/fm_d_rhcn_00.wmodel`은 716 vertices/1,320 triangles의 전체 공이며 원본 MN_RHCN_00_SK geometry와 위치 보정 후 오차 5.3e-8m 이내다. 파편이 아니며 새 cook이 필요 없다. 해당 모델과 `Effect/KoukuSaydon/Textures/MN_RHCN_00/tex/mn_rhcn_00_d.dds`를 world.object.kouku.ball에 연결했다. modelPreScale=.01과 사용자가 가진 Transform/Motion은 유지했다. Resources 파일 자체는 변경하지 않아 새 binary Drive 전달물은 없다. 이 두 경로는 기존 팀 Resource pack에 필요하다.

Object Tool은 기존 y=v*t+0.5*a*t² sampling을 그대로 사용한다. Arc Height H와 Lifetime T로 v=4H/T, a=-8H/T²를 기존 objectMotion에 저장하면 첫 생성은 T/2에서 H까지 올라갔다 T에서 시작 높이로 돌아온다. 생성 개수·간격은 보존하고, 늦게 생성된 공도 같은 전체 종료시각을 공유한다. Timeline의 Physics Y 곡선은 첫 생성 기준이며 드래그가 실제 기존 Seek를 호출한다. Transform 키와 회전은 보존한다. 별도 PhysX world, 새 저장 곡선, 임의 animation clip은 추가하지 않았다.

근거는 `ball-mn-rhcn-00-source-audit.json`이며 원본·Resources에 쓰기나 자동 화면 캡처를 수행하지 않았다.

## G06. Effect·Scene Profile 누락 수정과 최종 방향

사용자 후속 관찰의 공통 원인은 진짜 세이튼 찾기 Scene Profile의 `durationMs=24127`,
`blendMs=600000`이었다. 유효한 저작 Blend 값을 Product reader가 일반 Effect fade 수명으로 검사해
전체 staging을 취소했고 무력화 Effect까지 누락됐다. 플레이어 시작 위치 변경이 원인은 아니다.
SCENE_PROFILE만 canonical `0..600000` 범위를 허용하고, 잘못된 Scene Profile 행은 명시 진단과
함께 격리한다. 일반 Effect fade 길이 검증은 그대로 유지한다. 재발 원인을 `.md/GB/gotchas.md`의
`Kouku Scene Profile의 blendMs에 Effect 페이드 길이 검증을 적용하지 않는다` 항목에 기록했다.
Blend는 현재 예약 메타데이터이고 프로파일은 즉시 활성화된다. 어두움은 기존 multiplier가 조절한다.

사용자가 현재 3시 → 6시 방향이라고 확인했으므로 Server yaw → CNpc → CTransform과 저장 camera
투영을 확인했다. 현재 부호에서는 **+90도**가 시계 방향이다. 앞서 제시한 -90도/57도는 폐기했고
최종 댄스타임·룰렛은 **147 → 237도**다. Composition 85→86의 차이는 revision과 이 두 yaw뿐이며,
진짜 세이튼/분신 위치·scene blend와 다른 모든 사용자 키는 보존했다.

Composition·encounter Product·patternbindings·Server bootstrap은 86으로 명시 publish했다.
`PATTERNSPAWNRESET` 두 행도 237이다. 최종 Product 빌드는
`out/BuildPipeline/runs/20260907T073151905Z-debug-product.json`으로 PASS이며,
관련 projection 58/58·WorldSequence 26/26·Server Object 접촉 12/12·protocol 11/11이 통과했다.
격리 headless Server의 bootstrap 로드 및 1초 자동 종료도 PASS다. Native Object track 24개는
설치된 8개 모델의 110개 clip에서 모두 resolve했다. WorldSequence는 부모/자식 변경을 포함한
revision 368이며 source/runtime 의미가 일치한다.

근거는 `out/KoukuPatternStaging/presentation-blend-regression-audit.json` 및
`out/WorldObjectMotions/`에 있다. 실제 Client Complete Play, 암전 외형, 6시 방향의 최종 시각 확인은
사용자 대기다. Server + Client profile을 Ctrl+F5로 재시작한 뒤 Saved Patterns의 무력화 시작,
진짜 세이튼 찾기 v2, 댄스타임, 룰렛을 확인한다. UI 실행·조작·캡처나 자동 커밋은 하지 않았다.
