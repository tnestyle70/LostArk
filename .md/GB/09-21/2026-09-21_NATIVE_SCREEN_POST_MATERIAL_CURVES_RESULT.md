# Native Screen Post 재질 곡선 연결 결과

## G01. 구현 완료

`Client/Public/Effect_ArtistMaterial.h`의 `Build_ArtistMaterialTrackBindings`에 기존 native SCREEN_POST carrier의 named parameter track admission을 추가했다. 기존 registry의 rendererShape, Element kind, ScreenPost 활성, SourceRecipe shape를 요구하며 parameter 이름·형·lane·curve·source clock 검증을 유지한다. 기존 Mesh/LocalDecal과 track 없는 기본값은 보존한다.

`Client/Private/Effect_DocumentRenderer_Rendering.cpp`의 `Build_NativeScreenPost`는 정적 Artist parameter packet을 복사한 뒤 준비된 binding이 있으면 기존 `Apply_ArtistMaterialTrackSamples`를 호출한다. 입력은 문서 시간인 `Frame.fSampleTimeSeconds`이며 helper가 `fSourceTimeOriginSeconds`를 한 번 더한다. `Evaluated.fSampleTimeSeconds`는 기존 element-local native shader 시간으로 유지한다. 잘못된 sampling은 material snapshot 생성 전에 실패하며 원인을 반환한다. capture 전용 분기는 변경하지 않았다.

## G02. 유리 후보

`out/KoukuBingoGlass20260920/source-scene.json`의 row 12 `EFInterpTrackPostRenderMaterial`은 원본 활성 12.500000953674316..15.433334350585938초를 소유한다. row 13 opacity와 row 14 type의 constant key와 source time을 그대로 후보에 옮겼다. opacity는 활성 시 1, 종료 시 0이고 type은 13.533333778381348초에 0→1이다.

후보는 `out/RaidRepair20260921/glass/validation/effect.kouku.bingo.encore.glass.screen.effect.json`이다. root가 생성·설치한 원본 `bfx_mi_bg_00.fx_mi.fx_d_brokenglass_01_tr` native program 2627 재질을 사용한다. source clock origin은 0이고 director post track의 identity node를 material curve carrier로 사용한다. sourceRecipe는 screenPost와 빈 module 목록이며 다른 zoomblur의 Cascade module을 복사하지 않았다. 요소의 source metadata는 실제 row 12를 가리킨다. 기존 detail ScreenPost profile enum은 native 준비 재질 경로의 기존 유효 descriptor이며 실제 실행 재질은 2627이다.

Effect JSON은 별도 문서 duration 필드가 없다. 원본 bool 구간대로 effect 최대 시간은 15.433334초이고 부모의 World Sequence occurrence를 23.333초로 연결하면 이후에는 화면 효과를 제출하지 않는다. 시간을 늘리기 위해 dummy element나 원본 bool 연장을 넣지 않았다. 부모 작업자가 등록과 P97/P10 연결을 소유한다.

## G03. 실행 검증

- 현재 Effect codec/playback/material 35 TU와 변경 `Effect_DocumentRenderer_Rendering.cpp` 전체 Debug scratch compile PASS. native probe link PASS. 외부 DirectXTK PDB 경고는 있었고 컴파일·링크 오류는 없었다.
- 실제 codec Load/Validate_Drawable, native 2627 material contract, playback, 실제 `Build_NativeScreenPost`의 parameter snapshot 블록과 실제 inline admission/sample helper 79 checks PASS.
- 원본 bool window 직전/경계/직후, type step 직전/경계/직후, 23.333초 비활성, element-local clock 보존, nonzero source origin 1회 적용, unknown parameter 거부와 기존 binding 보존, 잘못된 carrier, 누락 track, nonfinite clock 실패와 parameter 부분 변경 방지, track 없는 static 값 보존을 확인했다.
- `Shader_VtxEffectNativeScreenPost.hlsl` 실제 FXC `/T fx_5_0` PASS. 기존 공통 sample 경로 X4000 경고는 있었고 오류는 없었다.
- `git diff --check` PASS. 두 기존 파일의 UTF-8 BOM 없음·CRLF와 무관한 변경을 보존했다. source 기준본은 `out/RaidRepair20260921/glass/validation/backup`, 변경 SHA는 `code-install.json`, 원본/candidate 시간·SHA는 `candidate-receipt.json`, native 실행은 `run.log`, shader는 `fxc.log`, 컴파일은 `build.log`다.

## G04. 남은 경계

native 검증은 실제 CPU 코덱·재생·parameter snapshot 생성 블록과 shader 컴파일까지다. GPU 입력 텍스처/화면 합성이나 사용자 화면을 직접 실행하지 않았다. 후보 이외 LiveData/Resources는 이 하위 작업에서 변경하지 않았고 전체 Product build/publish는 부모 작업자가 통합한다.
