# 로스트아크 원형 맵 재추출 결과

## 결론

기존 `dev.training.ground`에 들어간 맵은 사용자가 지목한 캐릭터 선택 맵이나 원본 수련장 맵이 아니었다. `BG_SHS_RCARENA_*` 10개를 골라 18개 인스턴스로 수동 배치한 `LV_DEV_TRAINING_GROUND` 큐레이션 키트였다.

이번 작업에서는 다음 두 원본 씬을 섞지 않고 별도 Area 문서와 리소스 루트로 다시 추출했다.

| Area ID | 범위 | 에셋 | 배치 |
|---|---|---:|---:|
| `LV_LOBBY_CLASSSELECT_SL00` | 스크린샷의 캐릭터 선택 원형 홀 정적 지오메트리 | 55 | 803 |
| `LV_SHS_RCARENA_D` | 원본 수련장 `SL00~SL03` 가시 지오메트리 | 302 | 7,856 |

`LV_SHS_RCARENA_D_PS`의 100개 배치는 모두 `lv_navimesh.mesh.lv_common_mesh_cul_box_*` 내비게이션 컬링 박스였으므로 원본 조사 증거에는 보존하되 가시 맵 배치에서는 제외했다. 이를 포함한 원본 패키지 배치 총계는 7,956개다.

## 왜 기존 파이프라인 결과가 달랐는가

1. 입력 패키지가 달랐다. 기존 데이터는 원본 Level 패키지의 ExportTable 배치를 복원한 것이 아니라 수련장 테마 메시 10개를 선정한 개발용 조립 맵이었다.
2. ImportTable만으로는 씬이 복원되지 않는다. ImportTable은 StaticMesh·Material·Texture 의존성을 식별하는 근거이고, 실제 위치·회전·부호 스케일·가시성은 Level ExportTable의 Actor/Component에서 읽어야 한다.
3. 에셋 생성과 Area 표시가 분리돼 있다. 현재 MapTool은 `Data/Maps/Editor/ACTIVE.maparea`의 한 Area만 읽으며, 그 값은 아직 `LV_LUT_HEARTRB_ED`다. 새 에셋과 배치 파일을 만들었다고 화면이 자동 전환되지는 않는다.
4. 작업 트리의 미완성 `CHARACTER_SELECT` 레벨은 맵 Area를 로드하지 않는다. 고정 레벨 계약에도 없는 별도 레벨을 보강하지 않았으며, 두 맵은 `DEVELOPMENT`에서 선택해 쓰는 Area로 등록했다.
5. 이번 결과는 정적 월드 지오메트리다. 원본 Scene/FX/light/cinematic 레이어까지 동일 화면으로 재현하려면 optional layer를 별도 추출·변환해야 한다.

스크린샷과 직접 대응하는 `LV_LOBBY_CLASSSELECT_SL00`에는 `bg_gdogods_magicfloor03d_sm` 및 Aryanorb/Kayangel/Gate of Gods 계열의 백색·금색 원형/방사형 메시가 포함된다. 기존 10개 수련장 키트와 이 55개 에셋의 교집합은 없었다.

## 생성물

Git 관리 대상 데이터:

- `Client/Bin/DataFiles/Map/LV_LOBBY_CLASSSELECT_SL00.mapassets`
- `Client/Bin/DataFiles/Map/LV_LOBBY_CLASSSELECT_SL00.mapplacements`
- `Data/Maps/Authoring/LV_LOBBY_CLASSSELECT_SL00/LV_LOBBY_CLASSSELECT_SL00.mapplacements`
- `Client/Bin/DataFiles/Map/LV_SHS_RCARENA_D.mapassets`
- `Client/Bin/DataFiles/Map/LV_SHS_RCARENA_D.mapplacements`
- `Data/Maps/Authoring/LV_SHS_RCARENA_D/LV_SHS_RCARENA_D.mapplacements`
- `Data/Maps/MapCatalog.json`

Git 제외 런타임 리소스:

- `Client/Bin/Resources/Map/LV_LOBBY_CLASSSELECT_SL00`: 55/55 `.wmodel`, 158개 복구 텍스처
- `Client/Bin/Resources/Map/LV_SHS_RCARENA_D`: 302/302 `.wmodel`, 739개 복구 텍스처

외부 조사·추출 증거:

- `C:/Users/user/Desktop/Resource_LostArk/05_Reports/MapExtraction/LV_LOBBY_CLASSSELECT`
- `C:/Users/user/Desktop/Resource_LostArk/05_Reports/MapExtraction/LV_SHS_RCARENA_D`

## MapTool 검토

현재 MapTool에는 이미 다음 기능이 있다.

- 카탈로그 팔레트 선택과 월드 클릭 배치
- 배치 계층 선택
- Position, rotation quaternion, signed scale 편집
- `Data/Maps/Authoring/<AreaId>` 저장과 publish 분리

따라서 다음 구현의 핵심은 배치·크기 조절을 새로 만드는 것이 아니라 `MapCatalog.json`의 development Area를 선택하고 안전하게 다시 로드하는 Area selector다. 선택 변경은 `parse -> validate -> stage -> commit`으로 처리하고 실패하면 기존 Area를 유지해야 한다. 제품 레벨을 우회하거나 새 `LEVEL` enum을 추가하지 않고 `DEVELOPMENT`에서 두 Area를 전환하는 것이 고정 계약에 맞다.

## 검증

- 캐릭터 선택: 카탈로그 55행, 배치 803행, authoring/runtime 정규화 본문 동일
- 원본 수련장: 카탈로그 302행, 배치 7,856행, authoring/runtime 정규화 본문 동일
- 캐릭터 선택 재질 55/55, 모델 변환 55/55
- 수련장 재질 302/302, 모델 변환 302/302
- `Client/Bin/Resources` 최상위 폴더가 `Character, Deploy, Effect, Fonts, Map, UI` 여섯 개임을 확인
- UTF-16 UE3 FName 종단 처리 회귀 테스트 추가

## 남은 수동 확인

MapTool Area selector가 아직 없으므로 이번 변경만으로 두 맵을 UI에서 선택해 띄우지는 못한다. 다음 단계는 selector를 추가한 뒤 각 Area의 카메라 시작점, 원형 중심, 스케일, backface/cull 상태를 실제 렌더링으로 확인하는 것이다.
