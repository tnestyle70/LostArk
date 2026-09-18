# 세이튼 무력화 별·외곽 경계 V1, 공_튀기기 사라지기 교체, 발탄 도끼 리본 결과

## G00. 반영 범위와 실행 경계

세 요청을 코드 3파일, 저작 데이터 1건, 신규 V1 4문서, 발탄 Full Restore 2문서 확장, 빌더 2개로 연결했다.
Client.exe(21368)·Server.exe(17884)·Visual Studio가 실행 중이라 제품 EXE/DLL/CSO는 교체하지 않았다.
C++ 두 TU와 Trail FX는 격리 컴파일로 검증했고, 데이터는 codec/playback/기하 probe와 문서 validator로 검증했다.
화면의 형태·색·타이밍 판정은 사용자가 Client에서 직접 한다. 증거 폴더는 `out/StaggerBallValtan20260917/`다.

## G01. 공_튀기기 `b_root` 거절 수정과 사라지기 교체

원인: `WorldSequencePlayer_Objects.cpp`의 `Prepare_ObjectResources`(483행 근처)와 `Sample_ObjectEffectBone`(66행)이
V1 follow attachment의 본을 Object 모델에서 찾고, 정적 `fm_d_rhcn_00.wmodel`에는 `b_root`가 없어 실패했다.
owner 경로(`Effect_PresentationService::Resolve_SourceAnchors`)는 없는 본을 건너뛰지만, transform-history 재생
(`Collect_TransformHistorySample`)은 follow slot이 `SourceAnchorWorlds`에 없으면 오류이므로 slot을 비울 수 없었다.

수정: `Sample_ObjectEffectAttachments`가 모델에 없는 본을 identity bone으로 채워 slot을 object pivot(effect local TRS 포함)에
붙이고, preflight는 거절 대신 `OutputDebugStringA` 한 줄을 남긴다. 명시 `effect.bone`, collider `attachmentBone`,
boss/character owner 경로는 그대로 엄격하다.

데이터: `LV_LUT_MIDNIGHTC_ED.worldsequences.json` revision 2075→2076. `ball_bounce` effectTracks를
`effect.ball_bounce.smoke`(GROUP, MOTION_END)·`effect.1/2`(follow) 세 행에서
`effect.ball_bounce.disappear_start`(TIME 0, followObject false)와
`effect.ball_bounce.disappear_end`(MOTION_END, followObject false, offset `[0,-1.5333,-0.9]` = 이전 연기 오프셋
`[0,-2.3,-1.35]`를 V1의 1.5배 기저로 환산) 두 행으로 바꿨다. 다른 바이트는 그대로다(byte splice, 백업
`ball/worldsequences.before.rev2075.json`). `Publish-MapAuthoring.ps1 -Scope WorldSequences -Mode Publish`가
runtime `Client/Bin/DataFiles/Map/…worldsequences.json`을 같은 SHA `b1c66baf…`로 교체했다.

확인 경로: Client 재빌드 후 F1 → Action Workbench → Object → `월드오브젝트_공` / `공_튀기기` → Preview.
`World Object: N visible…`이 표시되고 공마다 생성 위치와 착지(1700ms) 위치에서 사라지기 이펙트가 한 번씩 재생돼야 한다.
오프셋이 다르면 Object Tool에서 `disappear_end`의 positionOffset을 조정한다.

## G02. 무력화 V1 4문서

`Tools/EffectPipeline/build_saydon_stagger_star_boundary.py`가 설치된 source 문서를 `append_group`으로 합친다.
source 문서의 emitter delay와 burst는 recipe에 보존돼 있어 시스템 시작 오프셋만 더했다.

| 문서 | 표시명 | 요소 | 내용 |
| --- | --- | --- | --- |
| `effect.kouku.gate1.stagger.star.group` | 세이튼 / 무력화 \| 별 그리기·별 폭발 | 101 | 카드 10, 별 선 10, 별 완성 4(4.1s), 별 섬광 4(6.116s), 폭발 38(8.1s), 꼭짓점 폭발 5×7(8.1s+0.12s 간격) |
| `effect.kouku.gate1.stagger.boundary.group` | 세이튼 / 무력화 \| 외곽 경계 | 37 | Sk_12_6 링·충전 13, Sk_12_7 반짝임 4, Sk_12_8 바닥 decal·광선 20 |
| `effect.kouku.gate1.stagger.star.boundary.group` | 세이튼 / 무력화 \| 무력화 별과 외곽 경계 | 138 | 위 둘의 합본 |
| `effect.kouku.gate1.stagger.laser.group` | 세이튼 / 무력화 \| 레이저 생성·변화 | 43 | Sk_12 0.609s, Sk_12_1 0.624s, Sk_01_2 2.037s (Att_Battle_6_04 한 회) |

별 선은 `sk_12_9` 후보의 smoke_tail·ninjaflow follower 10개를 tracer 곡선(UE3 cm 11표본, 0.625s)의
`sourceTransformTrack`으로 옮긴 것이다. smoke_tail 원본 재질 turbpa_06(native 3008)은 ribbon-shape 등록이라
sprite에 admission되지 않아 strike의 `fx_m_pa_smoke_01_8_tr`(2992)를 대체로 썼다(표시명 `smoke*`).
꼭짓점 폭발은 원본에 없는 PROJECT_AUTHORED이며 sub-group `explosion.vertex.v1~v5`로 분리했다.
문서의 `sourceModelPreview`는 PATTERN_1의 6_02×4·6_03(레이저는 6_04) stage를 0 기준으로 담는다.

등록: `EffectCatalog.json` 4행, `EffectResourceTree.json` `세이튼_무력화 시작` 카테고리 4참조,
`Client.vcxproj/.filters` `None` 4항목(백업 `stagger/before/`). Composition presentationResources는 사용자가
Workbench에서 저장 중이라 자동 추가하지 않았다. Workbench Effect 자원 목록(Reload 필요할 수 있음)에서 네 문서를
골라 Append하면 된다. 합본을 `세이튼_무력화 시작`에 붙일 때 startMs는 STAGE_51 시작(5263ms)이다.

## G03. 발탄 stage008/009 리본·worms

`Tools/EffectPipeline/build_valtan_four_slash_axe_ribbon.py`가 기존 요소(4 baked AnimationTrail과 history 포함)를 그대로 두고
요소를 추가했다(LF·BOM 없음, 재실행 시 같은 id를 교체).

| 문서 | 추가 | 리본 시작·창 |
| --- | --- | --- |
| stage009 (40→50) | 리본 1 + worms 9 | 1.640s, 0.533s |
| stage008 (32→62) | 리본 3 + worms 27 | 1.667/0.267s, 2.221/0.2s, 3.008/0.2s(point life 0.292s로 절단) |

리본은 emitter_21 복제에 SpawnPerUnit(unitscalar 50)·rate 0·`bspawninitialparticle true`를 넣은 저주의식 방식이고
재질은 원본 native 2410이다. worms는 420612 stage004의 sprite 9종을 StartControl/b_wp_r_01 socket 변환으로 옮기고
rate 60/s(1 tick 1개, burst 유지)·40cm 반지름 1.5회전/s 원 궤도(`sourceTransformTrack`, `localSpace=false`)로 공전시킨다.

코드: `Effect_DocumentRenderer_Particles.cpp` coverage profile 목록과 `Shader_VtxEffectTrail.hlsl::Resolve_NativeTrailUV`에
2410을 추가해 `(coverageUV, distanceUV)`로 packing한다(2410은 `v2.x`를 0..1 gradient, `v2.zw`를 tiled panner로 읽음).

확인 경로: Client 재빌드 후 F1 → Effect Tool V1 → Valtan Full Restore → `Valtan 420609 / Main [9]`(또는 [8]) Open → Play.
도끼 socket에서 리본과 공전하는 worms sprite가 같은 창에 나온다. 요소 목록의 `axe ribbon …`, `axe worms …`를
끄고 켜서 비교한다.

## G04. 실행한 검증

| 검사 | 결과 | 증거 |
| --- | --- | --- |
| `WorldSequencePlayer_Objects.cpp`, `Effect_DocumentRenderer_Particles.cpp` 격리 컴파일 | exit 0 (기존 C4828 경고만) | `compile-objects.log`, `compile-renderer.log` |
| `Shader_VtxEffectTrail.hlsl` FXC fx_5_0 | exit 0, CSO 188,667B (기존 X4000 경고만) | `compile-shader.log` |
| 발탄 2문서 codec probe (Load/Drawable/Roundtrip/Stage) | 8/8 PASS | `valtan/…`, `codec_probe.exe` + `LOSTARK_RESOURCE_ROOT` |
| 발탄 2문서 15초 playback | failures=0, finished=1 | `valtan/duration-probe.log` |
| 무력화 4문서 codec probe | 16/16 PASS | `stagger/codec-probe.log` |
| 무력화 4문서 15초 playback | failures=0, finished=1 (star 16s, boundary 30.4s, laser 12.04s) | `stagger/duration-probe.log` |
| 별 선 기하 probe (track) | 10요소 모두 입자 생성, 다섯 변 coverage 0.01→1.00, 측방 ≤0.10m, 0/1/2/3/4s 순서 | `probe/track-star-analysis.json` |
| 발탄 anchor probe (합성 StartControl 7.5m/s) | stage009 리본 최대 8점·worms 9요소 80입자, stage008 리본 4/3/3점·worms 27요소 | `probe/anchor-stage00{8,9}.log` |
| 변경 6문서 validator 함수 + resource closure | 통과, 137 파일 9.44MB 존재 | `validate-changed-documents.json` |
| WorldSequences publish | exit 0, runtime SHA 일치 | `ball/publish-worldsequences.log` |
| `git diff --check`, `py_compile` | 통과 | — |

미실행·기존 실패: `Validate-EffectSources.ps1` 전체는 `effect.kouku.gate1.blade-dance.circle.impact`(2026-09-13 HEAD, carrier 없음)에서
먼저 멈춘다. Python v15 baked history 규칙은 발탄 stage008/009의 HEAD 문서도 거절하며(C++ codec은 통과) 이번 변경과 무관하다.
2410 packing의 WARP pixel parity, 제품 EXE/CSO 교체, Client/UI 실행, 화면 판정은 하지 않았다.

## G05. 남은 단계

1. Client를 닫고 Product Debug Build를 실행해 `WorldSequencePlayer_Objects`, 렌더러, Trail CSO를 반영한다.
2. Workbench에서 `공_튀기기` Preview와 무력화 4문서 Append, Effect Tool V1에서 발탄 stage008/009 재생을 확인한다.
3. 화면 판정 뒤 대체 smoke 재질(2992→3008 sprite row), 꼭짓점 폭발 sub-group 유지 여부, worms 공전 반경·속도를 조정한다.
