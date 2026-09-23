# 쿠크 Release 입장과 GBResources 전달 누락 확인

## G01. 확인된 문제와 적용

2026-09-23 배포 점검에서 `Desktop/GBResources`에 있는4,065개 파일은 모두 현재 실행 Resources와
SHA256이 같았다. 기존 파일의 구버전 문제는 없었다. 그러나9월19일 배포 이후 로컬에서
추가·갱신된 쿠크 참조 리소스 중31개(407,790,972bytes)가 GBResources에 없었다.
이는9월23일에 새로 생성한31개라는 뜻이 아니다. 오늘 ZIP에 반영된19~22일 작업의 전달 누락이다.

사용자의 최종 지정 경로 `C:/Users/user/Desktop/GBResources2`에31개를 복사하고 원본과 SHA256
일치를 확인했다. Character4개, Effect18개, Sound9개다. 기존 GBResources와 실제 게임의
Resources는 수정하지 않았다. 앞서 지정했던 GRResources2에도 같은31개가 남아 있다.
전달에는 최종 지정한 GBResources2를 사용한다.

상대 PC의 기존 `LostArk/Client/Bin/Resources`에 GBResources2 안의 Character, Effect, Sound를
같은 상대 경로로 합친 뒤 Client를 다시 실행한다. GBResources2 자체를 Resources 아래 넣지 않는다.
이 폴더는 보완 파일 모음이므로 기존 Resources를 대체하거나 삭제하지 않는다.

## G02. 입장 실패와 publisher 수정의 구분

Release 쿠크는 전체 Pattern/Sequence 이펙트와 enabled World 인스턴스를 입장 전에 준비한다.
필수 이펙트 준비 실패는 Level_Loading의 복구 경로에서 연결을 닫고 Lobby로 돌아가며,
Level_Lobby는 실제 사유와 관계없이 공통 문구 `Server entry failed.`를 표시한다.
베른은 해당 쿠크 전용 리소스를 준비하지 않아 베른 성공과 쿠크 실패가 함께 발생할 수 있다.

로컬에서는05:03경 V1 214개 실패0, V2 33개, World339개 준비와 Arena.Ready가 기록됐다.
서버의 원격3개 PC 기록은 쿠크 월드에서 peer의 정상 TCP 종료이며 Server 강제 종료가 아니다.
상대 PC의 상세 실패 로그는 아직 받지 않아 첫 실패 파일과 원격 입장 복구는 확정하지 않았다.

ZIP과 현재 실행본의 파일 비교에서 다른 것은 Encounter JSON과 Gameplay.bootstrap 두 개다.
Client/Server Release EXE, Action revision2222 원본, patternbindings와 게시 Composition은 같다.
Encounter의 차이는189,054개 수치의 소수점 정규화뿐이며 최대 차이는5.000000413701855e-10m다.
문자열·키·배열 길이 차이는0이다. publisher 수정 자체가 이번 Lobby 복귀를 일으켰다는 근거는 없다.

## G03. 오늘 구현·배포에 반영한 작업

| 작업 | 반영 내용 | 파일 전달 |
|---|---|---|
| 3관문 순서 | 요청한 시작·반복 순서,1초 추적과 일반 추적,4개 Mario Parent의 순차 자식 | Data와 Server/Client 코드 |
| 빙고 반복 | 최초24개 뒤14개 반복, 같은 Parent의 보드 유지 | Data와 Server scheduler |
| 블랙홀 | 원본4219927 클립, 보스24개·중앙40개 element, 지정 위치로 이동 후 앵콜 시작점 바라보기 | 새 Effect JSON2개, 기존 모델·텍스처 재사용 |
| 메두사·공포 | 현재42198102 클립에19개 element,3167~3667ms gaze 실패를 기존 FEAR3000에 연결 | 새 Effect JSON1개와 판정 데이터 |
| 사운드 | 블랙홀·메두사 원본 음원 연결과 다른 패턴 누락27개 연결 | Data 참조 변경; 기존 물리 음원 전달도 필요 |
| 관문 좌표 | 공먹기·백스텝·알비온의 해당 관문 위치, 바람방구 방향과 추적 중복 이동 보정 | Data와 Client/Server 코드 |
| 게시 정밀도 | 콜라이더 좌표9자리 정규화, 무력화·부위파괴 고정값 검사를 허용 범위로 교정 | 다른 세션의 publisher 코드와 생성물 |
| Release 배포 | 네 제품 빌드,1~4인 서버 계약904개,빙고45개,codec22개,이펙트 수치검증 | 기존 폴더 선택형 ZIP; 상대PC 실접속 증거와 구분 |

이번 새 Effect3개가 로컬 리소스162개를 재사용한다는 검사만으로 전체 배포의 추가 리소스가
0개라고 안내한 것은 잘못이었다. 포탈 파생 모델·빙고 리소스 등 이전 작업을 포함한 전체 전달을
확인해야 한다. 광대 MN_PPPP_00과 Bomb.wmodel은 기존 GBResources와 이미 일치해 추가하지 않았다.

## G04. GBResources2에 보완한 전체 목록

아래는 Resources 상대 경로다. GBResources에서31개 모두 경로가 없었으며 최신 로컬 실물을 복사했다.
크기와 SHA256, 소비 문서는 `out/KoukuEntryFallback20260923/kouku-resource-gb-post0919-delta.json`,
복사 결과는 같은 폴더의 `resource-staging-result.json`을 따른다.

| Resources 상대 경로 | bytes |
|---|---:|
| `Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel` | 283,680,928 |
| `Character/KoukuSaton/MN_RPCT_06/MN_RPCT_06.wmodel` | 22,195,032 |
| `Character/KoukuSaton/MN_RPCZ_00/MN_RPCZ_00.wmodel` | 56,437,780 |
| `Character/KoukuSaton/WP_MN_RPCT_05/WP_MN_RPCT_05.wmodel` | 1,267,012 |
| `Effect/Esther/Inanna/Textures/FX_TEX_02/fx_d_atypical_094_ycl.dds` | 65,664 |
| `Effect/Esther/Inanna/Textures/FX_TEX_02/fx_d_atypical_095.dds` | 32,896 |
| `Effect/Esther/Inanna/Textures/FX_TEX_04/fx_g_leaf_08.dds` | 32,896 |
| `Effect/Esther/Ninave/Textures/ENGINE_MI_SHADERS/t_cubemap_01_tex.dds` | 32,896 |
| `Effect/Esther/Ninave/Textures/FX_TEX_00/fx_a_atypical_026.dds` | 640 |
| `Effect/Esther/Ninave/Textures/FX_TEX_00/fx_a_atypical_046.dds` | 8,320 |
| `Effect/Esther/Ninave/Textures/FX_TEX_02/fx_d_atypical_041_ycl.dds` | 384 |
| `Effect/Esther/Ninave/Textures/FX_TEX_04/fx_f_feather_001.dds` | 8,320 |
| `Effect/Esther/Ninave/Textures/FX_TEX_04/fx_i_thunder_03.dds` | 131,200 |
| `Effect/Esther/Ninave/Textures/FX_TEX_05/fx_k_auraline_13.dds` | 65,664 |
| `Effect/Esther/Ninave/Textures/FX_TEX_05/fx_l_spiderline_01.dds` | 1,048,704 |
| `Effect/KoukuSaydon/FullRestore/Meshes/fm_b_cylinder_002_gate2_clear_inward.wmodel` | 11,592 |
| `Effect/KoukuSaydon/FullRestore/Meshes/fm_d_chamferbox_01.wmodel` | 12,456 |
| `Effect/KoukuSaydon/FullRestore/Textures/fx_tex_02/fx_d_normal_097.dds` | 262,272 |
| `Effect/KoukuSaydon/FullRestore/Textures/fx_tex_02/fx_d_symbol_105_cl_loc_int.dds` | 65,664 |
| `Effect/KoukuSaydon/FullRestore/Textures/fx_tex_02/fx_d_typical_009.dds` | 131,200 |
| `Effect/KoukuSaydon/FullRestore/Textures/fx_tex_05/fx_g_sy_arrow_03.dds` | 32,896 |
| `Effect/KoukuSaydon/FullRestore/Textures/fx_tex_nomipmap_00/fx_d_environ_073_nomipmap.dds` | 1,048,704 |
| `Sound/KoukuSaton/Events/bgm_midnightc_ed_m18_scene_fakeclear.source-stop.wav` | 9,286,340 |
| `Sound/KoukuSaton/Events/bgm_midnightc_ed_m20_scene_finish.source-stop.wav` | 19,570,260 |
| `Sound/KoukuSaton/Events/scene_midnightc_ed_koukuskill.source.wav` | 2,608,884 |
| `Sound/KoukuSaton/Events/scene_midnightc_ed_koukustopclearingdungeon.source.wav` | 8,393,868 |
| `Sound/KoukuSaton/Events/scene_midnightc_ed_popup1.source.1.wav` | 364,604 |
| `Sound/KoukuSaton/S_MOB_G_KOUKU1/g_kouku1_attack04_proj1__3705202.wav` | 244,064 |
| `Sound/KoukuSaton/S_MOB_G_KOUKU1/g_kouku1_attack04_projexp1__89092800.wav` | 120,584 |
| `Sound/KoukuSaton/S_MOB_G_KOUKU1/g_kouku1_attack07_proj1__201491086.wav` | 323,444 |
| `Sound/KoukuSaton/S_MOB_G_KOUKU1/g_kouku1_attack07_projexp1__84606297.wav` | 305,804 |

## G05. 감사 범위와 남은 확인

쿠크 관련 JSON937개와 WModel 재질의 하위 의존성을 따라2,184개 참조를 조사했다.
설치된 로컬 파일 누락과 WModel 재질 파싱 오류는0이다. GBResources는 추가팩이며, GB에 없는
오래된 기본 리소스 전부를 신규 누락으로 세지 않았다. 위31개는9월19일 이후 로컬 갱신과 GB부재를
함께 충족하는 보완 목록이다. 파일 생성·수정 시각만으로 실제 작업 의미나 상대 설치 상태를 증명하지 않는다.

원격 PC에 전달·설치하거나 Client/UI를 실행하지 않았다. 기존 ZIP은 변경하지 않았다.
재입장 실패가 남으면 해당 PC의 Client/Bin/Release/Diagnostics 최신 client-session JSONL의
lobby.recovery.presented 상세와 Client/Default/EffectFailure.user.log의 Kouku.Loading.Failed를 확인한다.
