# 베른 지형 복구 · 배 바다 확장 · 마하라카 섬 배치 RESULT (2026-09-29)

## 구현 상태 (빌드·Client 실행 없음, 화면 확인은 사용자)

### A. 베른성 Landscape 지형
- 저작 `LV_BER_BERNCASTLE.mapplacements`의 `:landscape:` 42행 visible 0→1.
- Validate/Publish `-Scope Placements` 성공. 이후 섬 설치 뒤 재게시, 최종 50,034 placement / 24 shard.
- 바다 수면 공백: x 103..198, z -180..435 구간에 원본 WATER02_512 배치가 없어 같은 자산·같은 높이(10.714)의 수면 1장 추가
  (`LV_BER_BERNCASTLE_ISLAND:water:gap-plane`). 원본 바다 구멍(약 48%)은 떠 있는 바닥으로 덮지 않았다.
- 4% 동일 높이 구멍 메우기는 확신이 없어 하지 않았다.

### B. 배 바다 확장
- BernSea 영역: x 140..600, z -650..-156 (460x494 m), 0.5m 셀 920x988, walkable 825,667(약 206,417 m²), 레벨 10.95.
- 한계: publisher 셀 상한 1,000,000에 맞춘 창 크기. 창 밖으로는 항해 불가.
- 외곽은 blocker 링, 베른 지형·정적 keep-out·섬 지형은 막힘 처리, 최대 4연결 성분만 채택.
- 기존 항구/배 스폰 위치(300,-235)는 유지. 다른 영역(Bern/Bern2/Bern3/기본) walkable 수치는 Validate에서 동일.
- 재베이크: `Publish-ServerNavigation.ps1` Validate/Publish 성공.
- 위험: Client `CNavigation::Find_Path` 기본 expanded node 상한 16384 — 넓은 바다의 원거리 클릭 이동 예측이 잘릴 수 있음(서버 A*는 상한 없음).

### C. 마하라카 섬 (외형 전용)
- `LV_OCN_EVENTIS_MHP` Landscape 16타일을 새 shard `ISLAND00`(assets 16 / placements 0)으로 카탈로그화하고
  저작 placement 16행(editor)로 배치. 재질 행 불필요(텍스처 baked).
- 중심 (439.78, 10.8, -479.87), 세로 1.5배, 치마는 수면 7m 아래로 가라앉힘. 삼각형 16x7,688 = 123,008.
- 접안 지점 (414.25, 10.95, -423.75), 섬 육지까지 약 11m. 포탈 트리거용 값은 `island_dock.json`(fork 2 전달).
- 소품(나무·건물 등)은 재질 행이 필요해 이번엔 제외.

## 변경 파일
- Data/Maps/Authoring/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE.mapplacements, Data/Maps/Imported/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE.mapset, ..._ISLAND00.mapassets/.mapplacements(신규)
- Client/Bin/DataFiles/Map/LV_BER_BERNCASTLE*(publisher 생성), ..._ISLAND00.*(신규)
- Data/Navigation/LV_BER_BERNCASTLE.BernSea.navsource, Client/Server Bin/DataFiles/Navigation BernSea navgrid/navblockers(/navsurface)
- Tools/ShipPipeline/build_sea_nav.py(재작성), bern_terrain.py, bern_island_layout.py, install_bern_island.py(신규)
- 백업: C:\Users\USER\.claude\jobs\46aea322\tmp\bern_backup

## 사용자가 눈으로 볼 것
- 베른성 바닥 구멍이 메워졌는지, Landscape가 기존 지형과 겹쳐 튀지 않는지.
- 바다 수면 이음매(x 103..198)와 섬 주변에 사각 윤곽이 보이지 않는지.
- 배로 (414,-424) 부근까지 항해되고 섬 앞에서 멈추는지.

## 불확실
- 삼각형/프레임 비용 실측 없음(섬 +123k 삼각형은 계산값). 리소스 `Map/LV_OCN_EVENTIS_MHP_LAND/...`는 팀원에게 전달 필요(마하라카 리소스 보유 시 이미 존재).
- 세 publish 결과 외 Server 실제 시작 확인 안 함. Gameplay.world.json/BERN.worldbootstrap 변경은 fork 2 소유.

## REVERT — 지형 42개 다시 숨김 + 마하라카 섬 제거 (2026-09-29, 사용자 요청)

사용자 요청으로 위 A(지형 켜기)와 C(마하라카 섬)를 되돌렸다. 바다 확장(B)은 유지한다.

### 지형 42개 다시 숨김
- HEAD(LFS smudge)와 대조: 원본 행 중 달라진 것은 정확히 42행(LAND01 22 + LAND02 20)이고 마지막 필드(visible)만 1이었다.
- 그 42행만 visible 1→0으로 바이트 단위 수정(LF·크기 유지, 수정 전 sha 확인). 백업 `C:\Users\USER\.claude\jobs\46aea322\tmp\bern_revert_backup`.
- 게시본 `LV_BER_BERNCASTLE_LANDSCAPE.mapplacements`는 Git 추적본과 같아져 더 이상 변경 목록에 나오지 않는다.
- 설치된 Landscape Resources는 건드리지 않았다.

### 섬 제거
- 제거: 저작 배치 `LV_BER_BERNCASTLE_ISLAND:landscape:mhp:*` 16행, mapset의 `ISLAND00` 샤드 행(헤더 24→23),
  `Data/Maps/Imported/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE_ISLAND00.mapassets/.mapplacements`, 게시본의 같은 이름 2개(게시자가 지우지 않아 직접 삭제).
- 유지: 바다 수면 공백 메우기 1행 `LV_BER_BERNCASTLE_ISLAND:water:gap-plane`(섬이 아니라 바다 확장; ID 접두사만 ISLAND).
- 로드 범위/`CLevelRegistry`: 섬 때문에 바꾼 코드는 없었다(코드·Data에 ISLAND 참조 0). 샤드 추가만으로 로드됐다.
- `Map/LV_OCN_EVENTIS_MHP_LAND` 리소스: 마하라카 자체 리소스(09-19~09-27 생성)라 그대로 둠. `CY_Resources`에는 이 폴더가 원래 없어 삭제할 파일이 없다.
- `Tools/ShipPipeline/bern_island_layout.py`, `install_bern_island.py`는 그대로 두었고 다시 실행하지 않았다.

### 게시·검증
- `Publish-MapAuthoring.ps1 -AreaId LV_BER_BERNCASTLE -Scope Placements` Validate → Publish → Check 모두 exit 0. 최종 50,018 placement / 23 shard / 47 file (이전 50,034 / 24 / 49).
- 게시본과 저작본에 `LV_OCN_EVENTIS_MHP_LAND`·`:landscape:mhp:` 참조 0개. 저작 castle landscape 42행 visible=1 0개.
- 네비 Validate: Bern 80862 / Bern2 46616 / Bern3 4764 / BernSea 825667 — 변경 없음(재베이크 안 함).
- 참고: 지형을 켤 때 네비를 다시 굽지 않았으므로 되돌려도 네비 작업은 필요 없다.

### 남은 영향
- fork 2의 `island.dock.to.maharaka`(414.25, 10.95, -423.75)는 이제 섬이 없는 열린 바다에 서 있다. `Data/Worlds/**`는 이번에 수정하지 않았다.
- 미니맵 `BernSea.png`는 섬 윤곽을 그려 넣은 생성 이미지라 섬이 없어진 뒤에도 그 윤곽이 남는다(별도 처리 필요).
- 게시 파일이 아직 커밋되지 않았다.
