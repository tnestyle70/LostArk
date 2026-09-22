# 발탄 복원 돌의 유지 구간 마스크 교정 결과

## G00. 현재 상태

사용자가 최종 저장본 반영 질문에 **아직 편집 중이니 반영 대기**라고 응답했다.
돌 JSON 7개는 후보로만 준비했으며 원본 데이터 교체, GameplayBalance 게시,
새 Client 실행 파일 링크는 수행하지 않았다. 미저장 Tool draft와 디스크 후보를 구분한다.

관련 유령 발탄과 가디언의 C++ 수정은 소스에 반영됐고 Client 전체 Debug x64
`MSBuild /t:ClCompile`이 exit 0으로 완료됐다. 실행 중 Client는 기존 실행 파일이다.
전체 컴파일 로그는 `out/ValtanStoneMask20260922/client-clcompile.log`에 있다.

## G01. 실제 원인과 교정 후보

native2391의 Dynamic.X는 최대 1로 제한된다. 유지값을 1.1로 올려도 UV radial + 실제
noise + 0.1 + gap 0.175의 최솟값 0.275가 clip threshold 0.3333보다 작아 일부가 잘린다.
해당 기둥 돌의 `09.gap_offset`을 0.25로 올려 유지 구간 최솟값을 0.35로 만든다.
유효 scalar와 authoringOverrides의 value를 맞추고 원본 compilerValue를 보존했다.
native shader, 다른 mesh를 사용하는 파편, Source Resources는 변경하지 않았다.

후보 범위는 Product 십자 1문서, 땅구르기·피자·발악 active 3문서와 복원 source 돌
3문서다. 각 기존 돌 mesh transform에 1.2를 곱한다. 복원 source에서는 실제 기둥
`fm_d_stoneparts_003` 6개만 변경하고 같은 shader의 다른 파편 2개는 보존했다.
버러지(TRASH)와 발악(STRUGGLING)은 서로 다른 패턴이므로 동일 대상으로 완료 처리하지 않는다.
Product 생성 연결은 별도 `2026-09-22_VALTAN_STONE_PRODUCT_RESULT.md`가 소유한다.

## G02. 실행한 검증

현재 `Shader_EffectKoukuNativeGroup2368.hlsli`의 실제 ArtistNative2391와 sampler wrapper를
추출해 ps_5_0으로 컴파일하고 WARP에 제출했다. 설치 DDS 5개의 sRGB/linear·wrap 설정과
설치 WModel 삼각형의 barycentric UV 18,018개를 사용했다. 총 90 draw에서 nonfinite는 0이다.

| 실제 mesh UV, t=0.25, X=1, alpha=1 | 기존 gap 0.175 | 후보 gap 0.25 |
|---|---:|---:|
| 전체 잘린 샘플 | 1,096 / 18,018 | 0 / 18,018 |
| 하단 25% 잘린 샘플 | 776 / 4,284 | 0 / 4,284 |
| 상단 25% 잘린 샘플 | 0 / 4,445 | 0 / 4,445 |

후보 X=0에는 1,772개, X=-0.25에는 656개의 샘플이 남는다. 기존 재질도 같은 조건에서
잔여가 있으며 source Dynamic.X와 수명 종료 정리를 유지한다. alpha=0은 전부 잘린다.
이는 mip0 UV별 재질 수치 검증이며 camera raster, Client 화면, 완전히 부드러운 소멸의
검증으로 기록하지 않는다. 실제 Product playback 수명 종료의 돌 draw 0은 별도 검사했다.

후보 7문서의 실제 codec load/Validate_Drawable, source/active Serialize→Parse roundtrip,
Product의 기존 birth center·개수·수명과 1.2배 mesh scale 검사가 통과했다.
`out/ValtanStoneProduct20260922/product_probe.log`에 소비자 검증을 기록했다.

## G03. 후보와 다음 반영 경계

- source 후보: `out/ValtanStoneMask20260922/source_candidates/manifest.json`
- Product 후보: `out/ValtanStoneProduct20260922/manifest.json`
- GPU 요약/원시값/입력 hash: `out/ValtanStoneMask20260922/summary.json`, `gpu_result.json`, `inputs.json`
- 최신 디스크와 stable ID/필드 기준 병합 준비: `out/ValtanStoneMask20260922/transaction/prepared.json`

병합 dry-run은 7문서에 성공했다. 최종 승인을 받으면 최신 저장본을 다시 읽고 같은 필드의
실제 충돌만 보고한다. 교체 직전 hash 재확인·원자적 교체·실패 시 자기 변경 rollback을
준비했다. cross는 Valtan presentation generation hash에 포함되므로 반영 후 공식
GameplayBalance publisher로 bootstrap/generation을 갱신하고 실제 사용하는 Server의
재시작을 구분해 안내해야 한다. 사용자 화면 확인은 미실행이다.
