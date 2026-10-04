# Bern 원본 RNM mip·Landscape shadow 복원 구현 계획서

## G00. 목표와 현재 실측

2026-10-04 전수 조사에서 Bern 기존 RNM DDS 4,618개 중 4,616개가 원본 하위 mip을 잃었다. 나머지 2개는 앞선 세션에서 이미 복원되어 10개 mip을 보존한다. 원본 15개 cooked package를 읽어 native mip 표를 대조했다. 이 중 4,518개는 Crunch carrier이며, 임의 BC 재압축이나 이미지 축소로 대신하지 않는다.

새 Landscape shadow 42개 중 LAND02 `shadowmaptexture2d_199`의 mip4(16×16)는 압축 컨테이너 길이와 decoded 길이가 모두 256으로 같아 기존 BulkData decoder가 raw로 오판했다. 기존 shadow 2,194개의 설치 데이터는 원본 전체 mip과 일치한다. 기존 설치 데이터의 27개 같은 길이 사례는 재생성 위험이며 설치 결함으로 세지 않는다.

사용자가 원본 기반 전체 복원을 직접 승인했다. 최신 디스크 기준 대상 DDS만 백업·원자 교체한다. 사용자가 수동 OFF한 Bloom/Fog/FXAA 및 다른 설정, mapmaterials, 배치/TRS, 다른 세션 변경은 보존한다. Client/UI 실행과 자동 Reload는 수행하지 않는다.

## G01. BulkData 소비 경로

`Tools/LandscapeExtractor/extract_ue3_landscape.py`의 `decompress_texture_bulk`는 native BulkData flags를 명시적으로 받는다. raw(0)는 길이를 검사해 반환하고 LZ4(0x80)는 길이가 같더라도 UE 압축 헤더·block 표를 해석한다. 지원하지 않는 flags는 거부한다. 높이·weight texture 호출자와 `build_source_landscape_lighting_candidate.py`의 G8 shadow 호출자가 flags를 전달한다.

실제 원본 같은 길이 compressed payload와 기존 전체 shadow 증거를 사용해 회귀를 확인한다. LAND02 새 shadow는 원본 모든 mip에서 디코드한 bytes로 교정한다.

## G02. 원본 cooked BC mip 추출

`Tools/LevelPlacementExtractor/extract_ue3_texture_mips.py`의 기존 원본 scratch mip 회전 경로를 재사용한다. 패키지별 여러 texture를 같은 native mip 단계로 회전하고 UModel LostArk KR decoder로 한 번에 추출한다. 패키지 원본은 읽기 전용이며 scratch는 repository `out/BernNativeLightingMipRestoration20261004` 아래에만 둔다.

각 texture의 native export serial, class, Format, UsesCrunch, native mip dimensions와 inline offsets를 확인한다. 원본으로 디코드한 mip0 BC bytes는 현재 설치 DDS의 mip0와 모두 일치해야 한다. 원본 lower mip BC blocks를 그대로 연결하고, 기존 DDS header의 mip count/caps만 수정한다. 모든 단계에서 원본 package/decoder/입력 SHA를 다시 확인한다. 기존 full-chain 2개도 원본 전체 mip과 대조하되 동일하면 재작성하지 않는다.

## G03. 검증과 설치

후보 전체의 per-mip native packed hash·decoded BC hash를 기록한다. 현재 DirectXTK loader를 연결한 격리 WARP probe로 SRV/texture mip 수와 모든 GPU mip readback bytes를 대조한다. 이 probe는 `out`에만 두고 제품 project/filter를 변경하지 않는다. 이것은 자동 데이터/GPU 검증이며 사용자 화면 확인으로 기록하지 않는다.

후보가 완성되면 대상 상대 Resources path, before/after SHA, 원본 근거와 검증 결과를 receipt에 기록한다. 교체 직전 현재 SHA가 후보 준비 시 값과 같은 파일만 백업 후 같은 폴더의 temp에서 `os.replace`한다. 실제 동일 필드 충돌은 보존하고 다시 검토한다. 설치 후 모든 대상 bytes를 후보와 재대조한다. C++/HLSL 변경이 없으므로 이 변경만의 Product Build는 필요하지 않으며 root의 별도 picker Product Build와 중복 실행하지 않는다.

## G04. 완료 산출물

동일 topic RESULT에 구현, 원본 전체 대조, DirectXTK 검증, 설치 및 백업 경로를 구분한다. 사용자 deliverable은 현재 작업 `outputs/bern-native-lighting-mip-restoration.json`과 요약 txt이다. Resources payload는 Git에 추가하지 않는다. 화면의 실제 차이는 사용자가 Bern 재진입/자기 Reload 후 확인한다.
