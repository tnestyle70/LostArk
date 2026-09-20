# 2026-09-20 쿠크 빙고 폭탄·망치·메두사 복구 결과

## G1. 완료 범위

- 서버 `Update_KoukuBingo`는 MARKED 종료에 `Plant_Bomb`와 planted slot World Sequence cue를 같은 tick에 확정하며, 2,000 ms 뒤 폭발/십자 보드 변경을 판정한다. 기존 서버 권위 경로를 그대로 사용한다.
- `sequence.kouku.bingo.bomb.planted`에 원본 심지 0~2,000 ms와 `MOTION_END` 폭발 3,301 ms tail을 연결했다. 기존 `WorldSequenceDocument::PresentationSpanMs`가 tail까지 소비하고 본체는 2,000 ms에 숨긴다.
- 별도 `world.object.kouku.bingo_bomb.original`과 4개 planted slot binding으로 빙고 본체를 1배로 분리했다. 기존 Object 폭탄 2배와 쇼타임 Effect Model Cue 3배는 보존했다. 기존 검증된 `b_body` socket offset `[.2,0,-.521496]`, X -90도와 modelPreScale .01을 그대로 재사용했다.
- All Effects에 `effect.kouku.bingo.bomb.full.restore`(본체+심지+폭발 21 emitters), `effect.kouku.bingo.hammer.aura`(망치 선행 4 emitters), `effect.kouku.bingo.medusa.face.full.restore`(메두사 얼굴 19 emitters)를 Catalog/Tree/Composition resource와 프로젝트 None 항목에 등록했다.
- 망치 원본은 `fx_mn_rpct_07_v.par_v_rpct_07_hammer_aura_01_loc_int`의 실제 first LOD/CDO/archetype closure를 추출했다. 기존 native programs 2585/2860/2869/2304를 재사용하므로 새 shader table/runtime ABI 변경은 없다. 4개 방향 hammer template의 0~5,000 ms object-follow effect로 연결했다. 기존 망치 움직임과 Server 판정 범위는 변경하지 않았다.
- 저장된 P94 `메두사공포`는 기존에 빈 DRAFT였다. 이 항목에 원본 `rpct00_att_battle_32_01 → 32_02 → 32_03`을 연결하고 얼굴 생성 cue를 1,000 ms에 배치했다. 32_02의 원본 11.1초 창은 `HOLD_LAST_POSE`로 보존했다. DRAFT 상태와 제품 encounter 자동 실행 목록은 보존했다.

## G2. 메두사 원본과 요청 적용의 구분

`out/KoukuAllEffects20260912/source/Action/MN_RPCT_07.loa`에서 action 4219936 stage 3/6 notify 3을 다시 추출했다. `par_v_rpct_atk_face_spwn_01_loc_int`는 원본 `enabled=false`이므로 기존 자동 복원에서 제외된 것이었다. 사용자의 구체적인 얼굴 이미지 복구 요청에 따라 **빙고 전용 파생 occurrence만 활성화**했다. 공유 원본 leaf와 raw notify는 변경하지 않았다.

원본 notify 위치 `[0,.1,0]`, scale `[3,3,3]`, root snapshot basis yaw -90도, Color/Alpha parameter, 19개 native 재질/SizeOverLife/발생 시간은 기존 복원 consumer로 적용했다. 이 결과는 이미지의 얼굴 연출에 대한 source-backed 후보이며 GPU 표시와 화면 크기 판정은 사용자 확인이 남아 있다. P94에는 별도 신규 시선/공포 피해 로직을 만들지 않았다.

## G3. 저장 및 검증

- 수정 직전 최신 디스크 내용을 다시 읽고 stable ID/변경 field만 병합했다. root의 P77 별도 뿅망치와 P95 즉사 칼날 항목은 보존했다. hash 재확인, backup, 같은 디렉터리 임시 파일 원자 교체를 사용했다.
- 해당 설치 시 World Sequence revision 2134, Composition revision 1906. 이후 다른 에이전트의 독립 행 추가로 revision은 증가할 수 있다.
- 설치 파일과 SHA: `out/BingoRepair20260920/install.receipt.json`. 원본/후보/추출 근거는 같은 폴더의 `world.before.json`, `world.candidate.json`, `hammer`, `medusa`에 있다.
- 최신 C++ codec, validator, Playback TU를 직접 컴파일해 Load/Validate_Drawable/Serialize-Parse 안정 roundtrip/두 번 rewind 재생을 검사했다. 폭탄 21/21, 망치 4/4, 메두사 19/19 emitters가 실제 CPU playback particle을 생성했다. 폭발은 2초 이전에 나오지 않고 심지는 제한된 방출과 tail 뒤 종료되는 것도 검사했다. 총 620,354 checks PASS. 로그 `out/BingoRepair20260920/cpu/final-probe.log`.
- 실제 WorldSequence codec과 parent 이동/유한 이펙트 overlap 29 checks 재검증 PASS. `git diff --check` PASS.
- Composition validate는 source 구조와 projection을 통과한 뒤 기존 generated Product stale 검사에서 종료했다. `Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json`에 대해 명시 publish가 필요하며 root 세션이 전체 Product와 함께 취합한다. 로그 `composition-validation-final.log`.
- Client/UI 실행이나 사용자 편집 draft Reload는 하지 않았다. 서버 설치→폭발의 실제 다인 접속 재생 및 화면 확인은 미실행이다.

## G4. 일반 해골의 경계 유지와 생성 flip 후속 완료

- `MN_ISTM_00-4.loa` 전체 Action에서 정확한 보드 원본을 확인했다. 4222005 일반→해골은 `Par_D_ISTM_00-4_tile_NormaltoMark`, 4222006 일반 해골 유지는 `tile_Mark`, 4222007 붉은 빙고 해골 유지는 `tile_MarkBingo`다. 앞서 조사한 RPCT `skull_sign`은 action 4219919 투사체이므로 보드 경계와 다른 효과다.
- 3개 ParticleSystem의 first LOD/CDO/archetype closure 14 emitters와 원본 6개 재질을 복원했다. `effect.kouku.bingo.skull.flip` 6개, `.white` 4개, `.red` 4개로 Catalog/Tree/Composition/프로젝트에 등록했다. 새 native 프로그램 2620~2625는 color/distortion source binding을 함께 설치했으며 기존 모든 native 함수/ID를 보존했다.
- `fm_d_chamferbox_01` 42정점/120인덱스 메시를 실제 원본 UModel glTF→converter→geometry contract로 조리했다. 이 형상은 새로운 대체 도형이 아니다. receipt의 `SOURCE_GLTF_WMODEL_GEOMETRY_PARITY`와 UPK→glTF raw-channel 미검증 경계는 그대로 유지했다. 기존 `fm_c_square_001`은 재사용했다.
- 원본 `NormaltoMark`는 0~.5초의 회전 타일 메시와 .5초 burst로 나오는 해골/경계를 구분한다. 본체가 2 turns/s로 돌아가며, 후반 레이어는 1초까지 산다. 이 실제 원본 flip을 사용하므로 조커 스켈레톤을 다른 floor 모델에 이식하지 않았다. 생성 모션 1,000ms 뒤 `NEXT`로 유지 모션에 연결한다.
- 유지 효과의 5초 native emitter loop와 5초 particle 수명은 원본 값이다. World Sequence는 300초 유지 창을 LOOP하며 Server board에서 칸이 제거될 때까지 계속 살아 있다. white/red 각각 실제 native 경계와 해골을 함께 사용하므로 종전 V2 Decal과 이중 표시하지 않는다.
- Object에 `월드오브젝트_빙고일반해골` Parent와 `일반해골_유지`, `일반해골_생성_플립`, `빙고해골_유지` 3모션을 추가했다. 기존 floor CModel은 숨긴 carrier로만 사용하고 원본 source effect mesh/sprite를 그린다.
- `m_BingoMarks`는 Server white/red mask만 읽는다. 각 cell에서 동일한 저장 Object 문서의 작은 subset을 기존 `CWorldSequencePlayer`로 재생하고, 처음 흰 칸 생성은 flip→유지, 재색칠은 white/red 유지, 칸 제거/Reset은 효과와 clone 정리로 이어진다. 현재 맵의 3.04m cell 중심을 사용하고 원본 sprite 폭 3.03m·메시 크기 3m는 보존했다. 실패한 상태 교체는 기존 cell을 먼저 지우지 않으며 generation별 bounded retry를 사용한다.
- 마지막 World source revision 2138. 최신 디스크 field merge로 앞선 뿅망치/P77/P95/Mario 변경을 보존했다. `skull/install.receipt.json`에 변경 파일 SHA/백업과 설치값을 기록했다.

## G5. 후속 검증과 남은 화면 확인

- 최신 PresentationPlayer TU `/Zs` PASS. 실제 World Sequence codec/Validate 29 checks PASS.
- 3개 skull asset 실제 C++ Load/Validate_Drawable/안정 roundtrip/rewind playback: 14/14 emitter, 169,138 checks PASS.
- 18초 동안 white/red의 모든 4 emitter가 5초 경계마다 끊기지 않고 존재하며, 방출 제한 뒤 잔존이 정리되는 것을 확인했다. 원본 flip의 실제 회전/크기 행렬 변화 110 samples, native sprite 폭 1.515~3.03m, mesh scale 3을 확인했다. 총 168,565 checks PASS. `skull-particle-probe.log`.
- 2560/2624 native Mesh/Particle shader carrier 4개 `fxc /T fx_5_0` PASS. 최신 Artist material table 포함 CPU probe 재컴파일 PASS. JSON/XML parse와 `git diff --check` PASS.
- 원본/설치/검증 증거는 `out/BingoRepair20260920/skull`, `skull-notifies.json`, `skull-cues.json`, `skull-codec.log`, `skull-world-probe.log`, `skull-runtime-compile.log`, `skull-fxc.log`에 있다.
- 실제 다인 접속에서 서버 설치→폭발, 흰/붉은 보드 전환, 반복 경계, 메두사 방향·크기, 망치 aura, 폭탄 socket 시각 일치는 사용자 화면 검증이 남는다. Client/UI 실행은 하지 않았다. 전체 Product build와 명시 publish는 root가 취합한다.

## G6. 통합 publish 확인

- root가 Composition revision 1909를 명시 publish했다. 결과는 Product 85 patterns / 473 stages / 8 bundles이며 `out/RaidRepair20260920/kouku-publish.log`에서 확인했다.
- MidnightC 전체 World Sequence revision 2139도 기존 `Publish-MapAuthoring.ps1 -Scope WorldSequences -Mode Publish` 경로로 검증·게시되었다. 런타임 SHA256은 `bc5c24d856c764b46c9e84b8db65d10c0fbf52721e19b7044a4af407ccd05898`이다. `out/RaidRepair20260920/kouku-worldsequences-publish.log`.
- 이 통합 publish와 별도로 수행한 C++ World codec 검사는 Object subset이다. 전체 문서의 Map/Deploy binding을 해당 subset probe로 검증했다고 기록하지 않는다. 게시된 디스크 데이터가 실행 중 메모리나 사용자 화면에 반영됐다는 뜻은 아니다.
