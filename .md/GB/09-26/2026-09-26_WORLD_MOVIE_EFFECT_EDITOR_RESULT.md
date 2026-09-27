# World Movie의 V1 Element 편집과 Camera live 편집 결과

## G01. 구현 상태

`All Effects → World → Character Selection Movies`에서 `Open Editor`가 선택 Movie의 Effect를
V1 Current Effect에 연다. Model View의 Intro/Loop Effect 목록에서 원본 asset ID를 선택하고
Element와 Detail을 편집한다. `View in Movie`는 선택 Effect의 첫 occurrence 시각으로 이동한다.
`Play All`, Pause/Resume, Stop과 시간 탐색은 기존 Level Movie owner를 사용한다.

V1에서 Movie 배우를 별도 Character preview로 대체하지 않는다. 기존 WORLD actor animation과
Effect·camera clock을 유지하며 Solo/Play Group도 Movie 안의 해당 Effect element를 격리한다.
Play All은 격리를 해제한다. Timeline / Camera와 World Effect 박스의 Open Effect Editor는 같은
세션을 왕복한다. 첫 V1 생성 시 callback을 연결하고 World의 playback 소유권을 전달한다.

Element draft는 catalog와 파일을 바꾸지 않는 불변 prepared target으로 준비한다. 기존 level-owned
active/pending handle을 현재 transform history와 source clock에서 검증한 뒤 교체한다. 실패한
후보는 정리하고 기존 instance와 Movie 상태를 유지한다. 편집본은 Play/Stop/Seek·Intro/Loop에서
유지되며 End Movie Editing과 Level 종료에서 정리한다. V1 Save는 기존 Effect 원본 저장과
prepared target 갱신을 사용한다. Load Saved/Discard는 같은 Movie에도 저장본을 반영한다.

문서 전환 코드에는 `A Solo → B 선택 → Play All`에서 A의 격리가 남지 않도록 전체 A draft를
먼저 복원하는 경로를 추가했다. 새 B의 검증이 끝난 뒤 복원하며 실패하면 isolation과 기존 문서를
유지한다. 같은 asset의 Load Saved/Discard도 전체 저장본을 적용하여 `A Solo → Load Saved A →
B 선택`에 격리가 남지 않게 한다. New/Save As는 End Movie Editing을 먼저 요구하며,
End는 임시 target 복원 후 isolation ID와 filter를 초기화한다. 이 단락은 소스 반영 범위이며
실행 검증 결과는 G03, 실제 클릭과 표시 확인은 G04와 구분한다.

## G02. 카메라와 저장

World Camera Box Detail의 선택 key에 Eye position, Look at, Up direction과 축별 FOV 입력을
추가했다. 기본 `Apply camera when edit ends`는 입력을 놓을 때 반영하며 명시적
`Apply camera live`도 제공한다. key ID와 원본 source/movie 시간 변환을 유지한다.

동일 box 시작·길이 안의 key 변경은 현재 camera만 다시 평가한다. 재생 token, 시각, pause와
배우·Effect를 유지한다. 미래 key 수정은 현재 시점을 그 key로 강제 이동시키지 않는다.
camera box 시작·길이 변경과 다른 row는 기존 전체 admission·Stop 후 Apply를 사용한다.
`Save movie`는 기존 stable ID/field 병합·freshness·원자 저장을 사용하고 Effect 내부 파일은
V1에서 따로 저장한다. 실제 저작 원본과 rendering 옵션은 작업 중 직접 교체하지 않았다.

## G03. 자동 검증

정상 Debug Product Build는 PASS다. 명령은
`powershell -NoProfile -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug`다.
첫 전체 변경 빌드는 `out/BuildPipeline/runs/20260926T024700420Z-debug-product.json`으로
94.6초, Client OBJ68개와 EXE를 갱신했다. 후속 보완 증분은
`20260926T024942343Z-debug-product.json`과 `20260926T030209814Z-debug-product.json`이다.
최종 prepared identity 수정 빌드는 `out/BuildPipeline/runs/20260926T030655739Z-debug-product.json`으로
11.7초, Client OBJ1개와 EXE를 갱신했다. 네 실행 모두 PASS, CSO/PCH 변경0이며 기존
C4819·외부 라이브러리 PDB 경고는 남는다. 설치 실행 파일은 `Client/Bin/Debug/Client.exe`다.

`python out/WorldMovieEffectEditor20260926/build_probe.py --run`의 compile/link/run 모두 exit0이다.
현재 Product Client OBJ와 Engine DLL/import library, 별도 복사한 현재 CSO로 연결한
창 없는 D3D11 WARP 검사다. 이전 Client 구현을 복사·재컴파일한 대체 런타임은 사용하지 않았다.
실제 Loader의 primary+5 Movie stage, 460 World occurrence, 60 Movie Effect resource를 준비했다.
최종 기록은 `out/WorldMovieEffectEditor20260926/probe-run.log`, 빌드 입력과 해시는
같은 폴더의 `product-inputs.json`, `shader-inputs.json`에 있다.

| 검사 | 실제 결과 |
|---|---|
| Effect pending/active 교체 | PASS. bloom·Element transform을 바꾼 draft 교체와 bloom 값 소비를 확인하고 배우 instance·camera·token·clock·pause와 지연 layer commit을 보존 |
| 실패 rollback | PASS. 잘못된 bloom 및 두 번째 replacement의 잘못된 handle을 거절하고 원래 object·handle·preview를 보존 |
| Movie 재생 지속 | PASS. Stop 중 draft 준비는 재생을 시작하지 않으며 Play/Seek·Intro/Loop에서도 같은 draft 유지, Clear는 최신 저장본으로 복원 |
| catalog 격리 | PASS. 임시 draft가 Product catalog의 원본 문서와 identity를 변경하지 않음 |
| Camera live | PASS. Eye/FOV 즉시 sample, paused/running 상태 보존, 잘못된 Eye=LookAt 거절, 미래 key identity와 source/movie 시간 변환 보존 |
| Camera Save/Reload | PASS. sandbox에 camera-only 저장, World publish 없이 Reload 후 Eye 유지와 재적용 |
| V1 실제 명령 | PASS. public Open/Play가 첫 호출부터 동일 Movie callback·timeline 사용, Solo A→B 전환 시 A 전체 복원, 미저장 전환 거절, End 정리 |

초기 native 실행은 prepared resource와 Document identity 불일치로 실패했다.
`Build_ResourceSignature`가 주소+asset ID를 사용하므로, prepare 뒤 문서 복사본을 만들던 순서를
불변 shared Document 선생성 → 동일 객체 prepare/attach로 고쳤다. 검증을 제거하지 않았다.
실패 기록은 같은 폴더의 `probe-run-first-failure.log`, `probe-run-staging-failure.log`,
`probe-run-resource-identity-failure.log`에 보존했다. 최종 재실행은 위 모든 항목을 통과했다.

실제 `Data/Camera/ClassSelection.cinematics.json`과 SL00 World 원본의 전후 SHA-256은 동일하다.
Save는 sandbox만 사용했고 V1 검사는 실제 원본을 읽기만 했다. 변경한 C++ 파일 20개는 원래 인코딩을
유지했으며 `git diff --check`를 통과했다. 소스 JSON/XML과 프로젝트 등록 변경은 없다.

## G04. 사용자 확인 경계

Client/UI를 실행·조작하거나 화면을 캡처하지 않았다. 화면·소리와 실제 ImGui 클릭은 사용자
확인이 남아 있다. 새 Debug Client에서 Character Select 입장 후 다음 경로로 확인한다.

1. F1 → Effect Tool V1 → All Effects → World → Character Selection Movies → Open Editor.
2. Model View에서 Intro/Loop의 Effect 선택 → Current Effect의 Element 선택 → Detail 수정.
3. Play All로 배우 애니메이션과 Effect를 함께 재생하고 Pause/View in Movie/시간 탐색을 확인.
4. Timeline / Camera → Camera box 선택 → Seek to camera key → Eye/Look at/FOV 수정.
5. Effect는 V1 Save, 카메라는 Save movie로 각각 저장.

이번 완료 범위는 구현·빌드·자동 계약 검증이다. 실제 화면의 시각적 일치 판정은 포함하지 않는다.
