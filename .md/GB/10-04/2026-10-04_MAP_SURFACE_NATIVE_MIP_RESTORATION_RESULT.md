# 베른·발탄 표면 텍스처의 원본 mip 복원 결과

## G00. 실제 재질 소비자부터 확인

현재 runtime mapmaterials의 비조명 DDS 참조를 집계했다. 최초 베른1,580개 중 full chain은
1,527개, 단일 mip은11개, 부분 chain은42개였다. 발탄434개 중 full10개, 단일424개였다.
이는 파일 header 상태이며 원본 복원 완료 여부나 FPS 병목 비율을 뜻하지 않는다.
조명 RNM/shadow와 일반 표면·height·lookup을 분리했다.

WModel에 남은 grass24 단일 mip 복사본은 현재 typed BG/Landscape의 실제 texture가 아니다.
현재 BG581행이 참조하는 SourceMaterials 입력은11단이며 mip0만 원본 압축 blocks와 같고
하위10단은 다르다. Landscape42행의 NativeLayers 입력은11단 모두 원본과 같았다.
따라서 베른 잔디 전체에 mip이 없다는 결론을 내리지 않는다.

원본은 현재 설치 게임의 `EFGame/ReleasePC/Packages`에서 실제 논리 package와 object로
resolve했다. 이전 PC의 `Resource_LostArk`나 사라진 out manifest를 근거로 쓰지 않았다.
현재 MIC 상속과 texture parameter를 다시 파싱해 실제 consumer와 연결했다. 후보는 원본
native compressed blocks를 재사용하며 mip0, format, sRGB metadata를 보존한다.

## G01. 베른 단일 mip11개와 grass24의 원본 하위 단계 설치

단일11개는 섬 D/N8개와 공용 normal/ORM/ambient-reflection3개다. 원본은 각각 D/N11단,
normal2단, ORM7단, reflection10단을 갖는다. reflection은 원본 Texture2D이며 cube로
변환하지 않았다. grass24는 기존 DX10 header148B를 그대로 두고 원본11단 BC payload로
교체했다. 다른11개는 mip count/flags/caps만 chain에 맞추고 format/mask는 유지했다.

2026-10-04 17:15 KST에12개 DDS를 같은 Resources-relative 경로에 설치했다.
합계8,348,564→10,665,056B이며 native mip118개다. 모든 최고 단계는 이전 설치 bytes와
같다. 일부 공용 texture는 다른 Area 문서도 동일 Resources 경로를 참조하며 동일 원본 입력의
하위 단계만 복원했다. 재질 연결·색 공간·SRV dimension은 바꾸지 않았다.

원본 package·decoder·candidate·target·consumer freshness 확인 후 대상별 백업과 원자
교체를 수행했다. 실제 설치 파일로 제품과 같은 DirectXTK WARP DDS loader를 사용해
linear/sRGB24개 texture/SRV 로드와236개 GPU mip readback이 모두 file bytes와 일치했다.
동일 경로 소비자로 확인된 Bern/SL12의 runtime·authoring4문서는 변경하지 않았다.
이름이 같은 texture를 다른 경로로 가진 Area까지 이 분모에 포함하지 않는다.
Client/UI 실행과 자동 Reload도 없었다.

원본 join·단계별 hash·상대 경로와 설치 전후 SHA는
`out/BernSurfaceMipAudit20261004/{missing-source-join,install-manifest,install-receipt}.json`,
GPU 근거는 `gpu-validation-receipt.json`, `installed-loader-validation.log`에 보관했다.
백업은 `out/BernSurfaceMipAudit20261004/backup/20261004T081521Z-e0e2db59`다.

## G02. 검증 경계

이 결과는 실제 DDS의 native mip 복구와 로더 검증이다. sampler의 최소 mip 품질 선택,
메시 삼각형 감소, draw 병합과는 별도다. 추가 하위 mip만큼 texture 저장/상주 용량은 증가할
수 있으며 VRAM streaming을 구현한 것이 아니다. 원작 전체 장면 일치나60FPS는 확인하지 않았다.
현재 실행 중 GPU texture가 바뀐 것은 아니며 다음 로드에서 교체본을 읽는다.

코드의 CPU LOD 누락·작은 draw 병합과 Debug 하/Release 최상 기본값은
[Bern draw 결과](2026-10-04_BERN_DRAW_SUBMISSION_OPTIMIZATION_RESULT.md)의G16~18이며,
Debug/Release Product와 수치 검증 후 PR526으로 main에 병합됐다. 이번 Resources 교체 자체는
재컴파일이나 Map 재게시를 요구하지 않는다. Resources payload와 전체 복구 ZIP은 Git에 넣지 않는다.

## G03. 발탄424개 원본 하위 mip 설치

현재 단일 mip424개는 실제 MIC233개·Texture2D417개·원본 package98개에 연결됐으며
모두 원본 하위 단계가 있었다. DXT1 201개, ATI2 165개, DXT5 58개다. 원본 native 범위는
7~12단이며 단일 원본·미지원 carrier·mip0 불일치 대상은0개였다. 기존 공용 batch decoder의
한 package24개 객체 묶음이 UModel allocator 오류로 실패해 최초 로그를 보존하고 chunk_size1로 격리 재시도해
모두 통과했다. 실패를 생략하거나 다른 텍스처로 대신하지 않았다.

검증된424개를 동일 Resources 경로에 설치했다. 합계276,478,976→368,629,240B이며
원본 native mip4,513개를 보존한다. 최고 단계·format·색 공간은 유지했고 DDS header는
mip count/flags/caps만 수정했다. 설치본의 DirectXTK WARP848개 texture/SRV 로드와
9,026개 mip GPU readback이 모두 설치 file bytes와 일치했다. rollback은0이다.

최종 runtime 비조명 참조434개는 기존 full10개와 복원424개를 합해 모두 full logical chain이다.
기존 full10개는 이번에 다시 원본 전체와 대조했다고 주장하지 않는다. runtime/authoring 재질
문서와 UV·품질 정책은 보존했다. 현재 실행 중 Client의 texture를 Reload한 것은 아니다.

근거는 `out/ValtanSurfaceMipRestoration20261004/{source-join,consumer-receipt,
install-manifest,gpu-validation-receipt,install-receipt,installed-current-inventory,result-summary}.json`이다.
백업은 같은 out의 `backup/20261004T082350Z-4fe38f83`에 보존했다.

## G04. 원작 환경설정 표와 프로젝트 실행 정책

현재 설치 원작의 data2.lpk에서 EFTable_SystemOption787행과 GameMsg를 새로 읽어
텍스처 품질 row 전체를 현재 JSON과 비교했다. saveTag `TextureQuality`, 최상/상/중/하,
원본 default0는 정확히 같다. 따라서 UI 항목·등급명·저장 키의 원본 근거는 있다.
최소 mip0/1/2/3의 sampler 연결과 사용자 요청 Debug 기본3은 프로젝트 정책이다.
원작 실행 코드의 동일 등급별 샘플링/streaming 정책을 확보한 것으로 설명하지 않는다.
원본 archive/entry hash와 row 비교 근거는
`out/BernSurfaceMipAudit20261004/original-options/source-row-comparison.json`이다.

## G05. Landscape native 범위와 압축 해제 누락1개 교정

42개 height Texture2D는 원본부터64/32/16/8/4의5개 native mip만 갖는다.2×2/1×1은
원본에 없으므로 생성하지 않았다. 그중41개는210개 전체 대조 중 해당 모든 단계가 같았지만
LAND02 LC767의 texture export11054 mip4는 설치된64B가 native decoded bytes가 아닌
packed bytes와 같았다. 원본 component의 positive ref11055로 실제 texture identity를 확인했다.

해당 mip의 bulk flags는128(LZ4)이며 packed/decoded 길이가 모두64B였다. 길이가 같다는
이유로 압축 해제를 건너뛰었던 기존 오류의 잔여 데이터다. 현재 flags 기반 decoder로 재회수해
header·mip0~3을 보존하고 최종4×4 단계만 교정했다. 전체21,952B 중56B가 바뀌었다.

동일 경로에 백업·원자 설치 후 실제 loader의2개 texture/SRV와10개 GPU mip readback이
모두 일치했다. 최종 height42개·native210단계는 전부 원본 byte와 같다. 지형 mesh/height
placement·UV·원본 단계 범위를 바꾸지 않았다. 이 오류를 앞선 절벽 형상의 원인으로 연결하지 않는다.
근거는 `out/BernSurfaceMipAudit20261004/height-fix/install-receipt.json`과
`height-post-install-summary.json`이며 백업은 `height-fix/backup/20261004T082746Z-42503955`다.

## G06. 베른 생성 mip1,454개와 특수 texture13개 설치

SourceMaterials DX10 DDS1,477개를 실제 소비자 기준으로 원본에 연결했다. 먼저 일반
재질의1,454개에서 native 하위 단계를 회수해 설치했다. 합계1,010,215,960B는 교체 전후
같고 native mip15,062개다. DX10 header148B, 최고 단계, format/sRGB/dimension을 모두
보존했으며 생성된 하위 bytes만 원본으로 바꿨다. 이 묶음은 mip 수나 상주 용량을 줄인 작업이 아니다.

일반 묶음은 BC1 715개, BC5 499개, BC3 234개, RGBA6개다. RGBA는 원본 bulk의 BGRA와
현재 DDS의 RGBA 순서를 무손실로 대응시켰고 mip0 동일성을 확인했다. 원본 전체가 이미 같은
RGBA4개는 재작성하지 않았다. 동명 statue의 앞 ObjectRedirector와 뒤 Texture2D가 공존하는
2개는 실제 MIC positive reference가 가리키는 export192/194를 package hash와 함께 고정했다.
임의 첫 번째 object 선택으로 공용 resolver를 완화하지 않았다.

실재 원본 이름 `wp_fbm_av_002-1_d/n`을 기존 정규식이 거부해 공용 split_object에 component
첫 문자 뒤 하이픈 허용을 추가했다. 경로·quote·공백·빈 component·선행 dash 거부33검사,
기존6개 테스트, Python 문법과 실제 두 원본의 재회수·mip0 비교를 통과했다.

일반 묶음은 독립 검토에서1,454개 target/candidate/header/mip0/nativeOutput 전체와
RGBA·동명 export·하이픈 예외의 원본 근거를 대조했다. 실제 MIC 상속 package170개를 포함한
source receipt dependency도 검증했다. 설치본의2,908개 texture/SRV와30,124개 GPU mip
readback이 모두 일치했으며 rollback은0이다. 재질 consumer 문서는 변경하지 않았다.
근거는 `out/BernSurfaceMipAudit20261004/full-chain/{install-manifest,install-receipt,
gpu-validation-receipt,final-summary}.json`과
`out/BernSurfaceMipCrossReview20261004/bulk-independent-review.json`이다.
백업은 `full-chain/backup/20261004T083715Z-a9f523d7`이다.

일반 parameter join으로 결정되지 않았던 물·파도·하늘13개는 원본 material의 고정 texture
table, MIC override와 native uniform-expression slot을 현재 compiled expression에 연결했다.
이름에 cube가 있는 파일도 실제 원본은 Texture2D였다. 원본 shader 정책을 변경하지 않고
1,476,724B·native119단계를 같은 경로에 설치했다. 설치본26개 texture/SRV·238개 mip
GPU readback을 통과했다. 일반1454·앞선12·height1개와 target 교집합은0이다.
근거는 `out/BernSurfaceMipAudit20261004/special14/{identity-result,install-manifest,install-receipt}.json`,
백업은 `special14/backup/20261004T083702Z-da8ec5dc`다.

## G07. 명시적인 NoMipmaps5개 복원

별도로 남은5개는 fresh Texture2D 속성에 `MipGenSettings=TMGS_NoMipmaps`가 명시돼
있으며 원본은1단뿐이다. 현재 생성된 하위 mip을 유지하는 것도 원본 정책과 달라 동일 BC
mip0를 보존한 native1단 DDS로 복원했다. foliage/normal/noise/sky/state 입력이며 state는
실제 native uniform slot에서도 `efmaster_material_prologue.tex.statefx_default`로 일치한다.
4개 MIC의 정확한 전역 shader-map key 동치는 미확정으로 남겼다. 해당 프로그램과 expression
slot에 연결되는 원본 map들의 texture identity가 모두 같고 mip0도 byte exact인 근거로
texture만 복원했다. 원본 state AddressX=Clamp와 현재 sampler UClamp도 같다.

기존12,146,932→9,110,244B로 설치했다. DX10 format/sRGB/dimension과 최고 압축 bytes는
유지하고 count/flags/caps만1단에 맞췄다. 설치본10개 texture/SRV·10개 mip readback을
통과했고 앞선 네 Bern 설치 범위와 교집합은0이다. sampler 코드와 재질 JSON은 변경하지 않았다.
원본에 없는 하위 단계를 만드는 것을 native 복원으로 처리하지 않았다.

근거는 `out/BernSurfaceMipAudit20261004/native-one-level-policy/{policy-audit,
scope-disjoint-check,install-manifest,install-receipt}.json`이다.
백업은 같은 디렉터리의 `backup/20261004T084127Z-719d3196`이다.

## G08. 최종 완료 범위

변경한 파일은 Bern 소비 범위1,485개(최초12+height1+일반1,454+특수13+NoMipmaps5),
Valtan424개이며 서로 다른 Resources 경로 총1,909개다. 모두 원본 최고 단계와 현재 색 공간·
format·연결 ID를 보존했다. 이전부터 native와 일치하는 파일, 원본 조명 복원본과 무관한
리소스는 재작성하지 않았다. 변경 파일 전체의 native mip은19,822개이며 설치 후 양색공간
texture/SRV3,818회 및 GPU mip readback39,644회를 통과했다.

공유 코드 변경은 source-object의 실제 하이픈 이름을 허용하는 Python 검증과 README,
프로파일의 현재 실효 `Texture.minimumMip` 기록이다. Profiler Debug Product는G20의
증분 빌드를 통과했으며 추출기 경계33개·기존6개 테스트·Python 문법 검사도 통과했다.
Resources 파일은 현재 PC에 설치한 결과이며 Git에는 포함하지 않는다. 전체 리소스 ZIP이나
UI 이외 복구 묶음은 만들지 않았다. 다음 Client 로드에서 새 DDS를 사용하며 실행 중
texture/미저장 draft/Server가 자동 갱신됐다고 설명하지 않는다.

최종 화면·베른 컷신 재캡처와 FPS는 사용자가 확인한다. mip 복구, draw 병합과 LOD 적용
확대가 실제60FPS를 달성했다는 측정은 아직 없다. 현재 개인 텍스처 품질 저장값은 최상이며
Debug 하 기본값을 비교할 때는 해당 옵션을 직접 하로 선택한다.

마지막 독립 파일 검사에서도1,909개 현재 설치 SHA와 백업 SHA가 모두 receipt와 같고,
설치 묶음 간 target 중복은0이며 실제 소비자 runtime/authoring6문서 hash가 유지됐다.
전체 파일 크기는1,308,689,108→1,400,119,176B다. 최종 집계와 각 설치/백업 경로는
`out/BernSurfaceMipAudit20261004/final-install-audit.json`에 보관했다.
