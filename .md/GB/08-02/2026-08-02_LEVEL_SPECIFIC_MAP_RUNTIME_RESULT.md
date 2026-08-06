# 레벨별 맵 런타임 분리 결과

## 결과

맵 선택 책임을 실제 게임 레벨과 MapTool에서 분리했다.

| 레벨 | 고정 맵 데이터 | 내비게이션 | 검증 결과 |
|---|---|---|---|
| `LEVEL::BAREN` | `LV_BER_BERNCASTLE` | `.navgrid` 미완성, 맵 로드는 허용 | `Bern Castle Map` 렌더 진입 후 10초 유지 |
| `LEVEL::VALTAN_ARENA` | `LV_LUT_HEARTRB_ED` 275 assets / 13,109 placements (base + Landscape 6) | `ValtanArena.navgrid` 프로토타입 등록 | `Valtan Arena Map` 렌더 진입 및 Landscape 표시 확인 |
| `LEVEL::ASSET_TEST` | `ACTIVE.maparea`가 가리키는 편집 대상 | 활성 area 계약 사용 | 기존 MapTool 동작 유지 |

따라서 베른 레벨은 베른성, 발탄 레벨은 발탄 base와 원본 Landscape가 합쳐진 단일 맵을 소유한다. `ASSET_TEST`는 계속 맵 제작과 확인을 위한 편집 레벨이다.

## 구현 내용

1. `Client_Defines.h`에 `LEVEL::VALTAN_ARENA`를 기존 레벨 번호를 바꾸지 않는 위치에 추가했다.
2. `Level_Loading`과 `Loader`에 베른/발탄 전용 분기를 연결했다.
3. `CMapAssetCatalog::Load_Area()`를 추가해 게임 레벨은 `ACTIVE.maparea`와 무관하게 고정 area를 읽도록 했다.
4. 배치 문서 읽기, 검증, 정적 배치 생성, 단독 오브젝트 fallback, 실패 rollback을 `CMapPlacementRuntime`으로 공통화했다.
5. `MapTool`도 같은 배치 계산과 생성 경로를 사용하게 해 편집 화면과 실제 레벨의 Transform 계산이 갈라지지 않도록 했다.
6. `CMapAssetObject`와 `CMapStaticBatchObject`가 대상 레벨의 prototype index를 명시적으로 받도록 수정했다.
7. 베른과 발탄 레벨에는 맵 확인에 필요한 자유 카메라만 생성했다. 캐릭터, 보스, 전투, 이펙트, deploy prop은 추가하지 않았다.
8. 직접 확인용 실행 인자를 추가했다. 해당 인자는 로딩 완료 후 자동으로 대상 맵 레벨에 진입한다.

```text
Client.exe --map-level=bern
Client.exe --map-level=valtan
```

상대 리소스 경로 때문에 실행 작업 폴더는 Visual Studio와 같은 `Client/Default`여야 한다.

## 실패 처리 계약

- 고정 레벨용 placement 문서가 없거나 비어 있으면 성공으로 처리하지 않는다.
- placement는 `parse -> validate -> stage -> commit` 순서로 적용한다.
- 중간 생성이 실패하면 이번에 stage한 객체를 제거하고 기존 런타임 상태를 보존한다.
- shard 사이 placement ID 중복도 commit 전에 거부한다.

## 검증

- Engine x64 Debug/Release 빌드 성공
- `UpdateLib.bat Debug`, `UpdateLib.bat Release` 성공
- Client x64 Debug/Release 빌드 성공
- `test_build_maptool_scene`, `test_build_bern_castle_shards`: 21 tests 통과
- `Client.vcxproj`와 `.filters` XML 파싱 성공
- `git diff --check` 오류 없음
- 발탄 Debug 숨김 실행: 실제 `Valtan Arena Map` 렌더 진입, 실패 대화상자 없이 10초 유지
- 발탄 Landscape 통합 후 Debug 화면 캡처: 원본 지형 6장 렌더 확인
- 발탄 Landscape 병합 재실행: 추가 0건, catalog/placement 해시 동일
- 베른 Debug 숨김 실행: 실제 `Bern Castle Map` 렌더 진입, 실패 대화상자 없이 10초 유지

기존 C4819 코드페이지 경고와 외부 라이브러리 PDB 경고는 남지만 빌드 오류는 아니다.

## 범위 밖으로 보존한 작업

작업 전부터 수정돼 있던 `Body_Valtan`, `Valtan`, `Level_AssetTest`, `ACTIVE.maparea` 및 에셋 추출 도구/산출물은 되돌리거나 정리하지 않았다. `Loader.cpp`에 있던 ASSET_TEST Sphere/OBB collider prototype 등록도 그대로 보존했다.

## 아직 남은 맵·내비게이션 작업

1. `LV_LUT_HEARTRB_ED`에는 base 269 assets / 13,103 placements와 Landscape 6 assets / 6 placements가 단일 문서로 통합됐다. deploy prop 85 placements는 별도 런타임 계층이므로 이 수치에 포함되지 않는다.
2. 원래 base 생성 입력 일부가 현재 PC에 남아 있지 않아 전체 재생성 대신 검증·중복 방지·원자 교체를 수행하는 `merge_maptool_landscape.py`를 정본 병합 단계로 추가했다. 원본 입력을 복구하면 base 생성 직후 같은 병합 단계를 실행해야 한다.
3. 베른은 맵 런타임은 정상이나 `LV_BER_BERNCASTLE.navgrid`가 아직 없다. 몬스터 이동을 연결하기 전에 베른용 navsource/paint/grid를 생성해야 한다.
4. 발탄 내비게이션 prototype은 등록되지만 몬스터 consumer는 이번 범위에서 생성하지 않았다. 기존 `CValtan`의 `ASSET_TEST` 하드코딩 제거와 `VALTAN_ARENA` 연결은 몬스터 작업 단계에서 별도로 진행해야 한다.
5. 발탄 게임 내 입장 버튼/포털은 아직 없다. 현재는 실행 인자만 진입 호출자이며, 이후 로비나 레이드 선택 흐름에서 `CLevel_Loading::Create(..., LEVEL::VALTAN_ARENA)`를 호출하면 된다.
6. Bern/Valtan `.wmodel`, `.wmat`, texture는 `Client/Bin/Resources`의 Git 미추적 공유 리소스다. 다른 팀원이 보려면 같은 공유 리소스 팩을 배치하거나 `LOSTARK_SHARED_ASSET_ROOT`를 설정해야 한다.
