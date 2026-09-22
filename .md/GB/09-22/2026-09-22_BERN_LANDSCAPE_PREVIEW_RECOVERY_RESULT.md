# 2026-09-22 베른 Landscape 42개 미리보기 재복구 RESULT

## 완료된 반영

원본 `LV_BER_BERNCASTLE_T_LAND01/LAND02` 패키지에서 Landscape 42개를 새로 추출하고 `_work/BERN_CASTLE_LANDSCAPE_PREVIEW_RECOVERY_2026-09-22`에 조리했다.

후보에는 원본 Heightmap, Weightmap, 레이어 할당, 원본 재질 파라미터, `layercliff`가 포함됐다. 경사면은 `LANDSCAPE_CLIFF` 재질 슬롯과 높이 기반 측면 투영 UV를 사용한다.

후보와 현재 설치본을 대조한 뒤 `Tools/LandscapeExtractor/Install-BernLandscapePreview.ps1`로 Resources만 설치했다.

```text
설치 전 서로 다른 파일: 195
  WModel: 38
  PNG/DDS 텍스처: 157
설치 후 서로 다른 파일: 0
```

기존 설치본은 다음 두 경로에 보존했다.

```text
_work/BERN_CASTLE_LANDSCAPE_RUNTIME_BACKUP_2026-09-22
_work/BERN_CASTLE_LANDSCAPE_RUNTIME_BACKUP_2026-09-22_RETRY
```

첫 설치 시 Windows `File.Replace` 호출의 백업 인자에 null을 전달해 교체 전 오류가 발생했다. 대상 파일은 바뀌지 않았고, 읽기 전용 Check에서 여전히 195개 차이임을 확인한 뒤 백업 파일 경로를 사용하는 방식으로 설치기를 고쳐 재시도했다.

## 의도적으로 하지 않은 것

- Landscape 42개 placement의 `visible=0`은 유지했다.
- Map Tool authoring, `Client/Bin/DataFiles`, Server runtime, 네비게이션, 조명, publisher는 변경하거나 실행하지 않았다.
- 따라서 이 결과는 Map Tool 개별 Landscape 미리보기 리소스 복구다. 베른 맵의 검은 공간을 제품 런타임에서 채우는 반영은 아직 하지 않았다.

## 실행한 검증

```text
새 추출 report: PASS
component / heightmap / weightmap / WModel: 42 / 42 / 42 / 42
collision height mismatch: 0
subsection seam mismatch: 0
새 WModel 42개: 2026-08-25 절벽 분리 후보와 SHA-256 동일
Install Check (설치 후): runtime file 208개, 다른 파일 0개
LandscapeExtractor unit tests: 23 PASS
Install-BernLandscapePreview.ps1 PowerShell parse: PASS
git diff --check: PASS
```

## 사용자 화면 확인 필요

Client를 다시 실행한 뒤 F1 Map Tool에서 `Bern Castle Landscape (42)` → `Bern LAND02 Landscape 755`를 선택해 Preview를 본다.

확인할 것은 다음 두 가지다.

1. 평지·길·잔디 레이어가 원래 베른성 바닥과 자연스럽게 보이는지
2. 절벽 면에 평지 텍스처가 세로로 길게 늘어나는 현상이 사라졌는지

이 화면 판정이 통과되어야 다음 단계인 42개 배치 가시화와 placement 전용 publish 여부를 결정할 수 있다.
