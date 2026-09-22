# Kouku Subtitle Box 화면 배치 연결 결과

기준: `GB/collider-pattern-bug-fix`, `583e4b5bbd2d`, 2026-09-22.
대응 계획: `2026-09-22_KOUKU_SUBTITLE_LAYOUT_IMPLEMENTATION_PLAN.md`.

## G01. 원인과 수정

Subtitle occurrence의 PositionOffset/Scale은 기존 JSON codec과 publisher에 이미 존재했다.
그러나 PresentationPlayer의 active row와 `Collect_Subtitles`는 문구/상하 위치만 전달했고,
`MainApp::RenderCinematicSubtitles`는 고정 글자 높이를 사용했다. 또한 Box Detail의
geometry live preview/Save 허용 종류에 Subtitle이 빠져 있었다. 따라서 범용 3D 숫자를
편집하거나 Preview를 눌러도 자막 위치와 크기의 최종 소비자가 없었다.

기존 occurrence의 PositionOffset X/Y를 1080 높이 기준 화면 px로 소비하며 +Y는 아래다.
Scale X는 균일 글자 배율이며 Box Detail의 Text scale 입력은 XYZ를 함께 저장한다.
자막에 사용하지 않는 3D rotation/Z 입력은 노출하지 않는다. 기존 offset 0/scale 1의
표시와 World/Valtan 자막·말풍선 기본값은 유지한다. 새 JSON field나 C++ 파일은 없다.

Box Detail은 기존 stable occurrence ID 기반 geometry overlay를 사용한다. 활성 Preview에서는
해당 자막의 화면 layout만 업데이트하여 재생 시계를 유지한다. owner가 없으면 자막 시작 시각에
멈춘 Preview를 요청한다. Preview 버튼의 edited occurrence, Apply, Save가 같은 layout을 보존한다.
Player는 active row와 typed `KOUKU_SUBTITLE_VIEW`에 offset/scale을 넘기며 최종 renderer는
글자 높이·행 간격·화면 위치에 이를 적용한다. 같은 문구라도 layout이 다른 행은 합치지 않는다.

## G02. 실제 데이터 반영

Sequence 정본 `Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json`의
`KAKULSAYDON_G1_PATTERN_5` / `2관문_클리어` 자막 `.presentation.28`~`.presentation.33`
여섯 행을 scale `[2,2,2]`, positionOffset `[0,90,0]`으로 설치했다. 실제 resource 순서는
`cin.37081_31_01, 03, 04, 05, 02, 06`이며 Boss 정본의 동일 pattern ID는 수정 대상이 아니다.
통합 담당이 최신 디스크 기준으로 좁은 병합·백업·교체했고 revision 167과 Composition publish
성공을 확인했다. 이 담당에서도 최신 source의 여섯 stable ID와 값·revision을 재확인했다.

처음 검토한 +60은 두 줄 자막의 확대된 행 간격 때문에 첫 줄이 기존보다 약 8.5px 위로
이동했다. 최종 +90은 모든 줄을 아래로 이동시키면서 1080 화면 안에 남긴다.

## G03. 실행한 검증

| 검증 | 결과와 경계 |
|---|---|
| 현재 Workbench, PresentationPlayer, MainApp 독립 x64 Debug TU 컴파일 | 3개 PASS. 기존 코드 페이지 C4819 경고가 있으며 컴파일 오류 없음 |
| 기존 scene subtitle 회귀 테스트 | 7/7 PASS. actual publisher validator/projector가 layout 보존, NaN/0 scale 거부, 입력 보존 |
| 현재 실제 Collect_Subtitles, RenderCinematicSubtitles, UILabelFont 함수 추출 probe | PASS. Preview/Product 선택, 같은 layout 중복 제거, 다른 layout 보존, half-resolution offset, 비유한 배율 격리 |
| 설치된 YoonGasiIIM SpriteFont + WARP 실측 | 6행/8줄 모두 최종 높이 52px. 최대 폭 742.765px로 허용 1612.8px 이내이며 폭 자동축소 없음 |
| 최종 Sequence JSON 재파싱 | revision 167, 여섯 행 모두 positionOffset `[0,90,0]`, scale `[2,2,2]` PASS |
| 변경 범위 `git diff --check` | PASS |

1920×1080 기준 한 줄 중심 Y는 994, 두 줄은 926/994이며 기존 938.5와 904.5/938.5보다
각각 아래다. 글자 하단은 1020으로 화면 내부다. 요청 높이는 기존 26×2=52px이며,
기존 기본 글자는 공용 baked font snapping 때문에 실제 25px였음도 별도로 기록한다.
공용 UILabelFont의 폭 맞춤과 snapping 규칙은 수정하지 않았다.

probe는 현재 실제 collector/renderer/font helper 본문을 추출하고 실제 설치 font를 WARP에서
로드해 `SpriteFont::MeasureString`을 호출했다. `Draw_Text` 호출은 수치로 기록했으며 실제
Client UI, GPU 글자 draw, 편집 화면 입력을 실행한 검증이 아니다. active row 입력은 headless
fixture이며 전체 제품 재생/저장 클릭을 대신했다고 기록하지 않는다.

## G04. 변경 파일과 남은 확인

- `Client/Private/KoukuSaydonActionWorkbench.cpp`: Subtitle screen UI, live overlay, Save/Revert 허용.
- `Client/Public/KoukuSaydonPresentationPlayer.h`, `Client/Private/KoukuSaydonPresentationPlayer.cpp`: typed layout 전달과 현재 row 업데이트.
- `Client/Private/MainApp.cpp`: 최종 화면 위치/글자 높이/행 간격 소비.
- `Tools/KoukuSaydonPipeline/test_scene_subtitle_candidates.py`: product projection layout 회귀 한 항목 추가.

수정 전 bytes/hash와 결과는 `out/KoukuSubtitleLayout20260922/`의 `before.json`, `before/`,
`compile.log`, `projector_tests.log`, `source_hashes.json`, `consumer_probe_result.json`에 있다.
`subtitle_data_candidate.json`은 revision 166→167의 정확한 여섯 행 필드 교체 근거다.
별도 산출물·하네스는 제품 source commit 대상이 아니다.

현재 소스·데이터 설치와 Composition publish를 완료했고 Engine/Shared/Server/Client Debug
Product 빌드와 배포도 exit 0으로 완료했다. receipt는
`out/BuildPipeline/runs/20260922T011037720Z-debug-product.json`이며 missing/invalid runtime
input은 0이다. Client/UI를 실행하거나 Reload하지 않았다. 최종 화면과 실제 Box Detail 입력
확인은 사용자가 수정 빌드에서 수행한다.
