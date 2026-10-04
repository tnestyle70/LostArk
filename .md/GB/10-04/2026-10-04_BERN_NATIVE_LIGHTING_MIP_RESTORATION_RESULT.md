# Bern 원본 RNM mip·Landscape shadow 복원 결과

## G00. 현재 완료 상태

2026-10-04 10:31 KST 최신 Resources에서 Bern RNM 4,618개 전체가 원본 native mip chain과 일치한다. 하위 mip이 없던 4,616개를 복원했고 기존에 복원된 2개는 전체 DDS bytes가 같아 재작성하지 않았다. native RNM 32,214개 mip(3~11단)을 실제 원본으로 디코드해 대조했다. 원본 mip0 BC bytes는 4,618개 모두 기존 설치 데이터와 동일했다.

LAND02 `Map/Lighting/Bern/lv_ber_berncastle_t_land02_shadowmaptexture2d_199.dds`도 원본 9개 mip으로 복원했다. LC00762의 mip4(16×16)는 압축 container 256bytes와 decoded 256bytes가 같아 기존 decoder가 container를 반환했고 229bytes가 틀렸다. 새 DDS SHA256은 `ec9b9696824c8cc0df62e4a7b1860e168872fa9655fbe6ed6dfd5c9ccf33f35d`다. LC00622의 화면 지점 문제와 이 다른 component의 오류를 동일 원인으로 기록하지 않는다.

실제 교체는 4,617 DDS, 동일 보존 2개, 충돌 0건이었다. 교체된 파일 bytes는 51,816,405에서 68,998,709로 증가했다. 전체 설치 후 파일 SHA는 검증한 후보와 일치한다. 배치/TRS/mapmaterials, 다른 Resources, Bloom/Fog/FXAA 등 사용자 설정을 변경하지 않았다.

## G01. 구현 변경

- `Tools/LevelPlacementExtractor/extract_ue3_texture_mips.py`: `extract_texture_mips_batch` 추가. 같은 package의 여러 native mip을 scratch에서 회전해 UModel `-obj` 목록으로 원본 cooked BC blocks를 회수한다. 원본 package/serial/packed payload를 변경하지 않고 단일 API와 같은 완전한 mip chain, format/dimensions, 현재 mip0 및 입력 freshness를 검사한다. object name 중복과 redirect를 batch에서 거부하며 그 경우 기존 single 경로를 사용한다.
- `Tools/LandscapeExtractor/extract_ue3_landscape.py`: `decompress_texture_bulk(payload, expected_size, bulk_flags)`로 명시 flags 판단. raw0은 길이 검사, LZ4 0x80은 길이 동일 여부와 관계없이 UE header/block table 디코드. 미지원 flags와 truncated compressed header는 거부한다. BGRA 소비자가 native flags를 전달한다.
- `Tools/LandscapeExtractor/build_source_landscape_lighting_candidate.py`: G8 shadow 소비자도 native flags를 전달한다. 이 파일은 다른 세션이 만든 candidate builder이며 해당 호출 한 줄만 수정했다.
- 두 extractor README에 batch 경로와 BulkData flags 계약을 설명했다. 제품 C++/HLSL/프로젝트 등록 변경은 없다.

RNM 4,518개는 원본 Crunch carrier, 100개는 일반 BC1 carrier다. 모두 LostArk KR UModel 전용 decoder로 원본 저장 mip을 회수했으며 일반 Crunch 변환·이미지 축소·BC 재압축·색공간 변환을 수행하지 않았다. source native storage의 4×4 minimum BC block과 DDS의 logical 2×2/1×1 크기를 각각 보존했다.

## G02. 실제 자동 검증

| 확인 | 결과 |
|---|---|
| 원본 package 15개 SHA 및 모든 대상 native serial/hash/inline table | 일치, 원본 변경 없음 |
| RNM 4,618개 원본 mip0 vs 기존 BC bytes | 모두 동일 |
| RNM native 32,214mip decoded block hash·DDS 크기/수 | 모두 검증 |
| Landscape Shadow199 native 9mip vs corrected 후보 | 모두 동일 |
| 현재 제품 DirectXTK loader, WARP 4,619 DDS | texture/SRV 전체 mip count 및 모든 GPU mip readback bytes 동일 |
| 기존 같은 길이 compressed 27개 shadow 실제 원본 fixture, 200mip | 수정 decoder vs 현재 정상 설치 bytes 모두 동일 |
| raw exact, unsupported flags, truncated compressed header | 정상 raw 보존, 오류 입력 거부 |
| 기존 `test_extract_ue3_texture_mips.py` | 6 tests PASS |
| 변경 Python 3개 AST parse, scoped `git diff --check` | PASS |
| 설치 전 전체 hash, 개별 교체 직전 hash, 백업, `os.replace`, 설치 후 SHA | 4,617개 설치·4,619개 검증 PASS |

기존 Bern shadow 2,194개의 설치된 모든 원본 mip은 조사에서 이미 exact였다. 그 중 압축 길이 충돌 27건은 기존 설치 결함이 아니라 옛 decoder로 재생성할 때의 위험이며 이번 수정으로 해소했다.

## G03. 증거와 백업

repository `out/BernNativeLightingMipRestoration20261004`에 `candidate-receipt.json`, 각 후보의 `.dds.receipt.json`, `bulk-flags-check.json`, `loader-validation.log`, `loader-input.tsv`, `install-receipt.json`을 보존한다. 파일별 native packed hash·decoded block hash·source serial/package/decoder hash, before/after SHA 및 freshness를 확인할 수 있다. loader log SHA256은 `3776020c392a0fd96defee9c51dee2436661784e46ba4a0c7763ad724e3c7fd5`다.

원본 설치 데이터 백업은 `out/BernNativeLightingMipRestoration20261004/backup/20261004-103026` 아래 Resources 상대 경로 구조를 따른다. 동일했던 2개는 백업·교체하지 않았다. 실패 rollback은 자기 afterSHA가 유지된 항목만 대상으로 구현해 concurrent save를 덮어쓰지 않는다.

사용자 산출물은 현재 Codex 작업 `outputs/bern-native-lighting-mip-restoration.json`과 `.txt`다. Resources payload와 격리 UModel scratch, loader probe EXE/OBJ는 Git에 추가하지 않는다. 팀 전달은 기존 Resources Drive 계약을 따른다.

## G04. 수동 확인과 남은 경계

Client/UI를 실행하거나 사용자 draft를 버리는 Reload를 수행하지 않았다. 제품 loader의 실제 D3D11 업로드 검증은 완료했지만 Bern 사용자 화면을 보았다고 기록하지 않는다. 실행 중 이미 로드된 SRV는 이번 파일 설치만으로 자동 재생성된다고 설명하지 않으며 사용자의 Bern 재진입/본인 Reload 뒤 확인한다.

이 작업은 native mip과 해당 shadow byte 복원이다. 원본 장면의 모든 연출, shader, 동적 조명 및 최종 외형이 완전히 같다는 증거로 확대하지 않는다. 사용자 Bloom/Fog/FXAA OFF는 보존한다. C++/HLSL 변경이 없어 이 데이터/추출 도구 변경만의 Product Build는 요구하지 않으며 root의 F1 picker 변경 빌드와 중복 실행하지 않았다.
