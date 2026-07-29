# Lost Ark BG visual review

`render_bg_asset_review.py`는 전체 환경 인벤토리의 glTF를 Blender에서 회색 형상 썸네일로 렌더하고, 검색·체크·선택 JSON 내보내기가 가능한 정적 HTML 갤러리를 만든다.

기본 발탄 수동 검수 범위는 `bg_rad_` 2,376개다. 렌더 도중 종료해도 이미 생성된 정상 PNG는 건너뛰므로 같은 명령으로 재개할 수 있다.

```powershell
& 'C:\Program Files\Blender Foundation\Blender 5.0\blender.exe' `
  --background `
  --python 'C:\Users\user\Desktop\LostArk\Tools\AssetReview\render_bg_asset_review.py' `
  -- `
  --inventory 'C:\Users\user\Desktop\Resource_LostArk\05_Reports\EnvironmentLibrary_20260729\environment_inventory.csv' `
  --output 'C:\Users\user\Desktop\Resource_LostArk\05_Reports\BGRadVisualReview_20260729' `
  --prefix bg_rad_ `
  --size 256
```

완료 후 `index.html`을 열고 이름, 패키지, category, asset ID로 검색한다. 체크한 항목은 브라우저 local storage에 유지되며 `Export picks`가 `bg_rad_selected_assets.json`을 만든다.

전체 BG 18,568개까지 넓힐 때는 새 output 폴더와 `--prefix bg_`를 사용한다. `--start`, `--limit`으로 일부 구간만 시험할 수 있고, `--force`는 기존 썸네일도 다시 만든다.

이미 별도로 추린 CSV의 모든 행을 렌더할 때는 `--prefix '*'`를 사용한다.

이 갤러리는 형상 전수 검수용이다. 실제 MapTool 배치 대상은 선택 JSON을 기존 cook/install 파이프라인으로 변환하여 `.wmodel`, 머티리얼, 텍스처를 검증한 뒤 카탈로그에 추가한다. 2,376개를 현재 eager Prototype 카탈로그에 직접 넣지 않는다.
