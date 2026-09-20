# 쿠크 빙고 진입 유리 파손 추출 결과

계획은 `2026-09-20_RAID_PRESENTATION_REPAIR_IMPLEMENTATION_PLAN.md` G13을 따른다. 사용자가 다른 작업자의 카메라·애니메이션과 구분하여 요청한 이펙트 추출을 완료했다. 원본 자료는 `out/KoukuBingoGlass20260920/README.md`에서 시작한다.

`SCENE07A`의 `EFInterpTrackPostRenderMaterial` export12가 실제 화면 균열 재질 `bfx_mi_bg_00.fx_mi.fx_d_brokenglass_01_tr`을 참조한다. 12.500001~15.433334초에 표시하며, 15.433334초에 12개 break 그룹이 파편을 발생시킨다. 두 spark 그룹도 함께 보존했다. 카메라·애니메이션 원본을 수정하거나 다른 저작자의 Sequence에 행을 자동 삽입하지 않았다.

설치 게임의 정확한 물리 package와 MIC parent를 해석하고 ShaderMap static key를 대조했다. 원본 텍스처4개를 DDS로 추출하고 shader5개를 DXBC 및 disassembly로 보존했다. `extraction.receipt.json`에 원본 hash, 추출물 hash, 재질 식별자, 타이밍을 기록했다. 기존 V1 라이브러리의 실제 파편·spark4개 asset과 참조 리소스41개(중복 포함)의 존재를 확인했다. 단순 명칭 유사도에 따른 이펙트 교체는 하지 않았다.

추출 명령은 `out/KoukuBingoGlass20260920/extract_source.py`, 성공 로그는 `extraction-second.log`다. JSON parse와 DDS/DXBC signature, 추출 receipt hash 및 참조 리소스 검사를 수행했다. 원본 추출은 완료됐고 화면 균열의 runtime screen carrier 연결과 사용자 최종 화면 확인은 후속 연출 작업이다. 새 shader 렌더링이나 GPU 표시 성공으로 기록하지 않는다.
