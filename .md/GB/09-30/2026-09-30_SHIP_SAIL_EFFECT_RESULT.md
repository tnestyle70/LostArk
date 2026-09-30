# 2026-09-30 "돛 이펙트가 안 뜬다" — 진단과 조치 RESULT

빌드와 Client/Server 실행은 하지 않았다. 화면 판정은 사용자 몫이다.

## 0. 결론

- 사용자가 말한 "돛"은 사진으로 보아 **닻(anchor) 표식 이펙트**일 가능성이 가장 높다(추론). 원본 사진(141859, 141921)에는 입항 지점에 주황빛 광채 원반과 금빛 닻이 있고, 같은 위치의 우리 화면(030807)에는 `[G]` 프롬프트 아이콘만 있다.
- 이 표식은 세션 로그상 **정상 준비, 정상 배치**됐다(오류 없음). 그런데 화면에 안 보이는 원인은 로그로 확정할 수 없다. 에이전트는 Client를 실행하거나 화면을 캡처하지 않는다.
- 화면 크기 불일치라는 **근거 있는 원인 후보 하나**를 데이터로 고쳤다: 마커를 섬 스케일 2.0에 맞춰 2배로 키웠다. 시각 확인은 사용자 몫이다.
- 진짜 "돛(sail)" 이펙트일 가능성도 배제하지 못했다. 아래 5절에 후보를 적었다.

## 1. 사진 판정 (직접 열어 확인)

| 사진 | 무엇인가 | 관찰 |
|---|---|---|
| 스크린샷 2026-09-30 030807 | 우리 프레임워크 (Bern Castle Network Player Test, 배 8204 유령선) | 정지한 검은 돛 배, 섬은 우상단, `마하라카 썸머 캠프 [G]` 프롬프트(체크 아이콘). 프롬프트 아래 바다에 광채가 없다. 배가 정지라 물살도 없다. |
| 스크린샷 2026-09-28 141859 | 원본 클라이언트 | 정박 중인 배 옆 입항 지점에 주황빛 광채 원반과 야자수 아이콘 프롬프트. 섬 좌측에 흰 안개 기둥. |
| 스크린샷 2026-09-28 141921 | 원본 클라이언트 | 부두 앞 바다 위 금빛 닻 + 원형 광채. |
| 스크린샷 2026-09-28 141528 | 원본 클라이언트 (항구) | 흰/베이지 돛을 편 3돛 배. 일반 배 돛은 밝은 천이다. |

## 2. 로그 근거 (Client/Default/EffectFailure.user.log, 세션 pid 38660, 03:04 이후)

- 03:04:22 `AnchorMarker.Bern spawn waiting: ... not prepared` → 03:04:30~31 `V1.prepare.document/renderer` 성공 → 03:04:37 `AnchorMarker.Bern placed ... pos=(430.10, 10.95, -476.64) yaw=111.9 attempts=10`.
- 이후 이 마커에 대한 실패 줄이 없다. 셰이더 CSO(`Shader_VtxEffectMeshKouku4928.cso`, `Shader_VtxEffectParticleKouku4928.cso`)는 00:01에 컴파일되어 HLSLI(23:56)보다 새롭다.
- 배 물살은 `ShipWake.Bern ... state=on` 줄이 정상으로 남는다(항해 중 speed 3.6~11 m/s). 준비 실패 없음.

## 3. 배제한 원인

- 리소스 누락: 문서가 참조하는 8개 리소스(모델 1, 텍스처 7) 모두 `Client/Bin/Resources`에 존재.
- 셰이더 program 미설치/미컴파일: CSO 타임스탬프가 HLSLI보다 새롭고 native 표(4973~4979)에 행이 있다.
- 배치 경로 결함: 같은 `Spawn_LevelPlacement` + `bOwnerSustainedSourceLoops` + `Commit` + `Update_WorldRoot` 순서를 Kouku 3관문 오라(`Submit_Gate3Auras`)가 그대로 쓴다.
- 부착 설정 결함: 마커 요소는 `follow=false`라 `Collect_SourceAnchorRequests`가 앵커 요청을 만들지 않는다. Kouku 오라 문서(부착 disabled)와 결과가 같다.
- 루프 종료: 모든 이미터가 `emitterLoopCount 0`(무한)이고 owner-sustained 루프다. 14초 뒤 사라지지 않는다.
- 원본 프롭 스케일: DeployData 레코드에서 Prop 1040158의 스케일 필드는 100 %.

## 4. 조치 (적용)

- 원인 후보: 우리 섬은 원본의 2.0배(`Tools/ShipPipeline/bern_isl72.py ISLAND_SCALE`)로 놓았지만 닻 마커는 원본 크기(요소 크기 1.0~2.5 m)로 남아 있었다. 원본 사진에서 광채/섬 폭 비율을 우리 섬에 적용하면 마커가 절반 크기로 작아 보이고, 배 정박점이 마커에서 2.7 m라 배에 가려질 수 있다.
- 변경: 문서 `particleSystem.uniformScaleMultiplier` 1 → 2.0. 이 값은 파티클 시뮬레이션 요소의 상위 변환(`Effect_Playback.cpp` 8299행)으로 적용되어 크기와 위치 오프셋이 함께 커진다. Kouku 문서 3개가 이미 같은 필드를 쓴다.
- 빌더 `build_island_anchor_symbol_source_effects.py`에 `MARKER_SCALE = 2.0`을 추가하고 `project()`가 같은 값을 문서에 쓰게 했다. 후보 모드 재생성 결과 설치본과 **바이트 동일(변경 0)**.
- 바뀐 파일:
  - `Data/Effects/Authored/effect.bern.anchor.marker.marker.full.restore.effect.json` (LF 유지)
  - `Tools/EffectPipeline/build_island_anchor_symbol_source_effects.py` (CRLF 유지)

## 5. 진짜 "돛(sail)" 이펙트 후보 (사진으로 확정하지 못함)

- 유령선(8204)의 검은 돛은 재질 문제가 아니라 원래 유령선 디자인일 수 있다. 플레이어용 `EFDLShip_GHOST.GHOST_01/02.loa`의 파티클 블록은 물살 2개(`FX_CM_05.Par_S_WaterTrail_N_01`, `_F_01`)뿐이다. 돛 전용 파티클은 몬스터용 `EFDLShip_MN_GHOST.*`에만 있다(`FX_CM_05.SHIP.Ghost.Par_D_GhostShip_01~03`, `Par_D_GhostShipMove_01`, `Par_L_GhostShip_01/02`, 본 FX_Point_02~13). 플레이어 배에는 없다.
- 사용자가 다른 배(8200~8203, 8205~8208)의 돛을 말한 것이면 그 배들의 LookInfo도 확인해야 한다. 이번에는 조사하지 않았다.
- 사용자가 "돛"을 실제로 돛(천)으로 말한 것이면 이 조치는 해당 없음이다. 알려주면 해당 배 종류를 지정해 LookInfo/재질을 추적한다.

## 6. 실행한 검증

- 사진 4장 직접 열람, 로그 grep, 리소스 8개 존재 확인, DeployData 레코드 스케일 필드 확인.
- 문서 검증 함수 4종(재질 색공간, native 스프라이트 옵션, 모듈 override, 부착 방향): 실패 0. 문서 버전 13이라 v15 확장 검사는 해당 없음.
- `py_compile` 통과, 빌더 후보 재생성 변경 0, JSON parse, `git diff --check` 통과.
- 실행하지 않은 것: 전체 `Validate-EffectSources.ps1`(쿠크 기존 파일로 실패), 빌드, Client 실행, 화면 확인.

## 7. 사용자 확인

- 재빌드는 필요 없다. 데이터 문서만 바뀌었으므로 Client 재시작으로 반영된다.
- 베른에서 배로 섬 앞에 가서, 닻 표식이 프롬프트 아래 바다에 보이는지, 배와 겹치지 않는지 확인한다.
- 안 보이면 `Client/Default/EffectFailure.user.log`의 `AnchorMarker.Bern` 줄과 스크린샷을 준다. 다음 후보는 (1) 배 정박점과 마커의 거리 2.7 m(배가 덮음), (2) 마커 높이(물 표면 대비 0.05 m), (3) LookInfo 블록 1, 2 미반영이다.
