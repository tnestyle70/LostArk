# 2026-09-22 베른 Landscape 42개 미리보기 재복구 PLAN

## 확인된 원인

현재 `Client/Bin/Resources/Map/LV_BER_BERNCASTLE_T/Landscape`의 42개 Landscape 중 38개 WModel과 관련 텍스처가 2026-08-25의 절벽 분리 결과가 아니라 구형 결과물이다. 사용자가 선택한 `MAP_0268B030B539_LAND02_LC_00755`도 그 38개다.

구형 결과물은 상단과 절벽을 하나의 `LANDSCAPE_BAKED` 재질로 처리한다. 새 후보는 원본 Heightmap, Weightmap, 레이어 할당, 원본 재질 파라미터, `layercliff`를 보존하고 절벽을 `LANDSCAPE_CLIFF` 재질과 높이 기반 측면 투영 UV로 분리한다.

42개 배치의 `visible=0`, `Data/Maps/Authoring`, `Client/Bin/DataFiles`, publisher, Server 데이터는 이번 단계에서 변경하지 않는다. 그러므로 이 단계는 개별 Map Tool 미리보기 복구이며 베른의 검은 구멍을 아직 채우지 않는다.

## G01 — 원본 재추출 완료

원본 패키지 `LV_BER_BERNCASTLE_T_LAND01/LAND02`에서 다음 후보를 생성한다.

```powershell
python -B Tools/LandscapeExtractor/extract_ue3_landscape.py `
  --umodel "C:\Users\USER\Documents\Codex\2026-07-28\c-programdata-smilegate-games-lostark\outputs\UModel_LOSTARK\umodel_lostark_v7.exe" `
  --package-root "C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\ReleasePC\Packages" `
  --converter "Tools\ModelAssetConverter\Bin\ModelAssetConverter.exe" `
  --output "_work\BERN_CASTLE_LANDSCAPE_PREVIEW_RECOVERY_2026-09-22" `
  --bake-resolution 256 `
  --expect-components 42 `
  LV_BER_BERNCASTLE_T_LAND01 `
  LV_BER_BERNCASTLE_T_LAND02
```

완료 조건은 `Reports/extraction_report.json`의 `status=PASS`, component/heightmap/weightmap/WModel 42개, collision mismatch 0, seam mismatch 0이다.

## G02 — `Install-BernLandscapePreview.ps1`

새 파일: `Tools/LandscapeExtractor/Install-BernLandscapePreview.ps1`

이 설치기는 후보와 현재 설치본의 `Resources/Map/LV_BER_BERNCASTLE_T/Landscape`만 대상으로 한다.

- 후보 report가 검증된 42개 패키지가 아니면 중단한다.
- 후보와 기존 WModel의 `WINT`/`WMOD` 헤더를 검증한다.
- WModel·PNG·DDS만 비교한다. 후보에 새로 들어온 절벽 DDS는 설치본에 없어도 허용한다.
- `Check`는 읽기 전용이다.
- `Install`은 변경 파일을 먼저 `BackupRoot`에 복사하고, 임시 파일과 원자 교체로 Resources만 바꾼다.
- 설치 뒤 SHA-256을 다시 비교하고, 오류면 기존 파일을 백업으로 복원하며 새 파일은 제거한다.

실제 코드 정본은 위 새 파일이며, 이 문서의 G02 계약과 정확히 일치해야 한다.

## G03 — 설치와 확인

사전 검사는 다음 명령이다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/LandscapeExtractor/Install-BernLandscapePreview.ps1 `
  -CandidateRoot "_work\BERN_CASTLE_LANDSCAPE_PREVIEW_RECOVERY_2026-09-22" `
  -Mode Check
```

설치는 다음 명령이다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/LandscapeExtractor/Install-BernLandscapePreview.ps1 `
  -CandidateRoot "_work\BERN_CASTLE_LANDSCAPE_PREVIEW_RECOVERY_2026-09-22" `
  -Mode Install `
  -BackupRoot "_work\BERN_CASTLE_LANDSCAPE_RUNTIME_BACKUP_2026-09-22"
```

사용자는 Client를 다시 열어 F1 Map Tool에서 `Bern Castle Landscape (42)` → `Bern LAND02 Landscape 755`를 선택해 미리보기를 확인한다. 평지·길·잔디 레이어가 보이고, 절벽에서 평면 텍스처가 세로로 길게 늘어나지 않는지를 판정한다.

## 자동 검증

```text
LandscapeExtractor unit tests: 23 PASS
새 후보: 42 components / 0 collision-height mismatch / 0 seam mismatch
새 후보 WModel 42개: 2026-08-25 cliff-split 후보와 SHA-256 동일
설치 전 Check: runtime file 208개, 설치본과 다른 파일 195개
  - WModel 38개
  - 텍스처 157개
배치 데이터 변경: false
publisher 실행: false
```
