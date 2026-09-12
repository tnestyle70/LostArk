# Character Select 잔여 맵 재질 복원 비용 조사

## G00. 현재 범위

2026-09-12 사용자는 Character Select의 핵심 바닥 외 재질 복원 비용 파악을 요청했다. 피날레·팝업북은 다른 세션 종료 후 사용자가 다시 요청할 때 적용한다. 이번 작업은 읽기 전용 조사이며 제품 코드·Data·Resources·runtime·ZIP을 수정하거나 빌드·publisher·Client/UI를 실행하지 않았다. 다른 작업의 렌더링 성능 diff는 보존했다.

조사 기준은 `codex/map-character-render-performance`, HEAD `aac4fbdd`와 당시 작업공간의 실제 입력이다. authoring/runtime `LV_LOBBY_CLASSSELECT_SL00.mapmaterials.json`은 바이트가 같고 모두9행이므로, 이번 잔여 범위를 게시 누락으로 판단하지 않는다.

## G01. 현재 연결 범위와 누락의 의미

정본은 `Data/Maps/Authoring/LV_LOBBY_CLASSSELECT_SL00/`의 mapplacements/mapmaterials/maplights, Imported의 같은 Area mapassets다. catalog의 실제 WModel을 읽어 WMSH submesh가 참조하는 material index와 WMAT 이름을 대조했다. 런타임과 같은 `Resource/` prefix 보정을 거쳐 Resources 내부 texture 존재를 확인했다.

| 항목 | 현재 실측 |
|---|---:|
| 전체 배치 | 803 |
| catalog 모델 / 실제 배치가 참조하는 모델 | 63 / 60 |
| 사용 중인 모델별 material slot | 87 |
| 원본 재질 계산이 연결된 slot | 9 |
| 기본 WModel 재질 경로인 slot | 78 |
| 원본 재질·배치별 조명을 연결한 배치 | 29 |
| 기본 재질 경로의 배치 | 774 |
| 사용 모델 중 UV1 보유 / 미보유 | 8 / 52 |
| 모델 실물 누락 / 사용 texture 경로 누락 / 사용 slot의 diffuse 공란 | 0 / 0 / 0 |

기본 재질 경로는 모델이나 diffuse가 없다는 뜻이 아니다. 원본 MIC의 색·반사·발광·분기와 배치별 간접광을 아직 모두 전달하지 않은 상태다. source binding의 수는 화면 복원율이나 보이는 픽셀 비율이 아니다. 현재9행은 PBR6행+seamless PBR1행+source specular2행이다. 원본 과거 조사75MIC와 현재70개 고유 사용 material name은 component override 및 모델별 slot 중복 때문에 서로 다른 분모다.

반복 배치가 많다. PILLAR01B/C 두 모델이350배치, DECO04가40배치, WALLCRYSTAL 계열이44배치, foliage/tree 계열이62배치다.774배치를 손으로 각각 수정할 작업으로 계산하지 않는다. 같은 source mesh/MIC를 재사용하고, placement별 atlas 좌표와 lighting 계수는 별도로 유지한다.

## G02. 복원 작업량의 조건부 견적

다음 수치는 현재 범위와 기존 파이프라인을 바탕으로 한 **집중 개발 작업량 추정**이다. 실제 생산성 측정이나 Codex 완료까지의 대기시간 약속이 아니며, 사용자 화면 비교·수정 반복 시간은 별도다.

| 범위 | 예상 작업량 | 포함 범위와 조건 |
|---|---|---|
| 기존 승인 입력을 그대로 재사용할 수 있는 좁은 묶음만 | 2~4시간 | 동일 mesh/MIC/입력 확인 후 기존 variant 확대. 전체78slot 완료 견적이 아니며 아래 전체 작업의 별도 가산 항목도 아님 |
| 기둥·벽·장식의 대표 불투명군 | 8~16시간 | 실제 배치 MIC 확인, 기존 family/texture/상수 연결, 대표 배치 적용·검증 |
| 잔여 일반 배경의 UV1·RNM·배치별 환경 확대 | 추가8~24시간 | 최대52개 미보유 모델의 원본 UV1 필요 여부 판정·필요 채널 재쿠킹, material variant, 배치별 atlas/반사, publisher/컴파일 검증 |
| 수정·식생·투명·하늘 특수군 | 추가8~24시간 | 각 실제 source program과 geometry/pass 입력을 대조하고 기존 지원 경로로 연결. 새 native/vertex 변형이 필요하면 재산정 |

전체의 거친 예산은24~64 집중 작업시간, 하루8시간 기준 약3~8개발일이다. 기존 family·변환 도구 재사용과 원본 자료 접근 가능을 전제로 하며, 원작과 모든 픽셀이 같은 결과를 이 기간 안에 보장하는 견적이 아니다. 주요 일반 배경부터 처리하면 약1~2개발일의 대표군 작업 후 확대 여부와 실제 성능을 판단할 수 있다.

52개 모델에 UV1이 없다는 사실은52개 모두에게 임의 UV를 생성하라는 의미가 아니다. 원본이 RNM을 사용하는 모델부터 기존 cooker로 원본 채널을 전달하고 배치별 조명 연결을 검증한다. UV0 복사로 UV1 복원을 대신하지 않는다. 원본 COLOR0·tangent·ordered MIC가 필요한 계열도 함께 확인한다.

09-08의 원본 조사에는55mesh/75MIC/14 terminal family,178texture/58lightmap/2cube 확보 기록이 있으나 현재 제품 전체에 적용된 상태는 아니다. 당시 out 경로의 자료가 현재 모두 재사용 가능하다고 확인하지 않았으므로 오래된 후보를 곧바로 설치하는 시간을 견적으로 잡지 않는다. 그 이후 쿠크·발탄 작업으로 기존 runtime 지원 범위가 넓어져14family를 모두 새 shader 구현 수로 계산하는 것도 잘못이다.

## G03. 렌더링 비용

현재 맵에는 PBR3/4, source specular5, overlay7, BG8, foliage9/grass10, snowice11/vertexblend12/wet13과 selected native 경로가 이미 존재한다. 일반 불투명 표면은 실제 원본 식이 일치하는 기존 family를 사용하면 기존 geometry/G-buffer/light 경로 안에서 복원할 수 있다. 현재8MRT를 재질마다 새로 추가하는 작업이 아니다. 단, texture sampling·표면 계산·UV1 vertex bandwidth·lightmap texture 메모리가 늘 수 있고, 서로 다른 atlas pair와 material variant를 위해 batch가 분리될 수 있다. 배치 수가 그대로라고 draw 비용도 무조건 같다고 보장하지 않는다.

특히 `MapAssetCatalog.cpp`의 `source.*`는 `MODEL_SURFACE_FAMILY::SOURCE_CHARACTER`로 연결된다. `CMaterial::Bind_SourceCharacter`에서 program33~65의 forward/sky/helper는 row0 경로라 추가 deferred material row가 없지만, 다른 native program은 실제 material별 frame row를 만들 수 있다. Renderer는 이 row별로 광원 계산을 반복한다. 그러므로 native program 전체 이식을 기본 방안으로 삼지 않고, 일반 맵 family로 같은 식을 표현할 수 있는 표면을 우선한다. 추가 native row가 필요한 표면은 직접광 pass 수와 GPU 시간을 따로 비교한다.

환경 반사는 기존 cube/2D reflection과 BRDF 입력을 우선 재사용하고, 원본 lightmap으로 이미 구운 광원을 dynamic point로 중복 추가하지 않는다. 실시간 cube capture나 전체 화면 굴절 등 새 pass가 필요한 경우는 별도 비용으로 구분한다. 특수 투명 표면은 넓은 화면 점유와 겹침에 따른 overdraw도 별도로 확인한다.

현재 재질 복원 전후의 동일 카메라 A/B GPU 캡처는 없다. 성능 작업 RESULT에 기록된 사용자130fps 관찰을 이번 미적용 재질의 FPS 보장으로 쓰지 않는다. 따라서 추가 GPU ms·예상 FPS 하락률·VRAM 증가 MB는 아직 확정하지 않았다. 판단에 필요한 값은 source material 수, batch/draw 수, 실제 source row별 광원 pass, GPU elapsed, texture/vertex 메모리다.

## G04. 권장 우선순위와 다음 적용 경계

기존 바닥은 보존하고 반복이 많은 기둥·벽·석재부터 대표 모델의 실제 source MIC를 연결한다. 그다음 같은 계열 전체 배치에 확대하고 UV1·RNM·환경 반사까지 연결한다. 마지막으로 결정·식생·투명·시간 발광 등 특수 표면을 처리한다. UI에서 밝기만 올려 빠진 재질식을 대신하지 않는다.

현재 texture 실물 누락이 없어 Resources 전체 재업로드를 먼저 할 근거는 없다. 이후 UV1 복구 모델이나 추가된 원본 texture가 생기면 실제 사용하는 Resources 상대 경로만 배포 대상에 기록한다. 기존 family만으로 끝나는 항목과 shader/runtime 수정이 필요한 항목을 구분하고, 다른 작업이 끝나기 전에 공유 제품 파일을 변경하지 않는다.

수치 근거는 `out/CharacterSelectMaterialCost20260912/material-inventory.json`, 분류는 `material-inventory-notes.md`, 비용 근거는 `runtime-cost-notes.md`다. 기존 복원 이력은 `09-07/2026-09-07_FLOOR_MATERIAL_RECOVERY_RESULT.md` G08~G13, `09-08/2026-09-08_CHARACTER_MATERIAL_MINIFICATION_RESULT.md`, 현재 경계는 `.md/GB/렌더링이펙트복원V2.md`의 선택 맵 재질 항목을 대조했다. 이번 조사에는 사용자 화면 복원 PASS가 없다.
