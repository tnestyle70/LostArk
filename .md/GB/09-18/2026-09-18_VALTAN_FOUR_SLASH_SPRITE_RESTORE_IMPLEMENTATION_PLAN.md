# 발탄 4연속 공격 사전 생성 Sprite 검격 복원

## G00. 목표와 현재 실측

제품 `VALTAN_FOUR_SLASH`의 SLASHES/SPIN은 carrier-v1 `active.clip-01/02`를 소비한다.
Full Restore stage008/009를 고쳐도 제품 두 문서는 바뀌지 않는다. clip01에는 원본
`Par_O_RPBF_Atk_02_08` 5 emitter가 없고, clip02에는 재질·본 연결이 꺼지고 emissive가
0.05인 emitter27만 남아 있다. 원본 action420609의 stage008 notify003은
0.494731992초/3.005268097초, stage009 notify003은 0초/2.896986008초다.

이번 범위는 이 다섯 SpriteParticle의 두 제품 occurrence 연결이다. 기존 사용자 저작
weapon-slash, mesh trail, hit-spark, fragments, screenPost는 보존한다. 원본 Trails4개 및
선택 항목인 Atk_01_01 전체 폭발은 이번 사전 생성 검격 수정과 분리한다.

## G01. 데이터 변경과 소비자

- `Data/Effects/Authored/effect.valtan.carrier-v1.attack.four-slash.active.clip-01.effect.json`:
  기존7요소 뒤에 stage008 notify003의5개를 추가한다.
- 같은 접두사의 `clip-02.effect.json`: 기존 `source.04edcf16319413095ac9` ID를 유지하여
  stage009의 emitter27 source payload로 복원하고 나머지4개를 추가한다. 다른 기존 요소는 보존한다.
- stage008/009 Full Restore의 sourceRecipe, native material2426/2458/3005/2326/2995,
  Resources 상대 경로, notify TRS와 StartControl/b_wp_r_01 socket을 재사용한다.
  groupId는 제품 문서 내부의 별도 `source.atk-02-08` 그룹으로 둔다.
- 두 제품 cue의 기존 GAMEPLAY_FOOTPRINT/worldScale1.5는 바꾸지 않는다.
  stage009의 원본 notify scale1.3과 별도 값이다.

## G02. 본 단위 보정

`Client/Private/Effect_PresentationService.cpp`의
`Requires_SourceBoneImportScaleNormalization`에 두 정확한 제품 ID의 StartControl만 추가한다.
`Collect_SourceAnchorRequests`에서도 같은 ID/slot에 기존 measured-unit source basis 분기를
허용해 정상 발탄의0.01 basis와 유령 발탄의1 basis를 기존 Full Restore와 같이 소비한다.
Tool은 같은 public helper를 호출한다. 새 public 타입·C++파일·project/filter 등록은 없다.
기존 C++의 인코딩과 CRLF를 유지한 ASCII 부분 편집만 수행한다.

## G03. 적용·검증

먼저 out/ValtanFourSlashSprite20260918 후보와 백업을 만든다. stable ID로 변경 요소를
병합하고 무관 요소의 byte span은 보존한다. 설치 직전 입력 hash를 재확인하고 임시파일을
원자적으로 교체한다. 실패 시 자기 변경만 rollback한다.

변경 두 문서의 JSON/material/resource 검사, 현재 codec Load/Drawable/Roundtrip/Stage,
실제 설치 WModel/animation의 b_wp_r_01·socket·preScale 수치와 particle lifetime을 확인한다.
필요한 Product Debug 증분 빌드와 presentation generation publisher를 실행하며, 실행 중인
EXE/DLL 잠금이 실제 발생하면 해당 링크·배포를 미완료로 보고한다. Client/UI는 실행하지 않는다.
화면 검증은 F1 → Effect Tool → Core Server Patterns → 4연속 공격 → SLASHES/SPIN의
제품 문서를 사용자가 Reload/Open 후 재생해 판정한다.

## G06. Effect Tool Full Restore의 도끼 주위 Sprite 투명도 복구

사용자가 실제 확인한 대상은 carrier-v1 clip이 아니라420609 stage008/009 Full Restore다.
현재 제품 EXE와 원본5종 Sprite의 존재를 재확인하고 실제 본 기반 CPU 재생을 조사했다.
후속 저작된 axe worms27+9요소는 sourceTransformTrack.alphaScaleKeys=[]을 갖는다.
Codec은 존재하는 빈 배열을 기본0인 optional 분포로 읽고 Playback Evaluate_Color가 이를
곱하여 모든 입자의 alpha를0으로 만든다. 원본 Atk_02_08의5요소는 이 결함 대상이 아니다.

build_valtan_four_slash_axe_ribbon.py의 orbit_track은 위치 곡선만 생성하고 alphaScaleKeys를
생략하도록 바꾼다. 실제 두 Authored 문서는 stable valtan.worms ID인36요소의 빈 알파
필드만 제거한다. 본·위치궤적·재질·시간·spawn과 원본 Sprite 및 리본은 보존한다.
최신 저장 bytes의 백업/교체 직전 일치 검사/원자 교체 후 실제 설치본을 다시 검사한다.
현재 Codec/Playback 의미를 바꾸거나 관련 없는 빈 curve의 전역 default를 바꾸지 않는다.

실제 설치 WModel의 본361샘플×2와 현재 source-bone helper로 before/after의 생성 수,
최종 quad, alpha를 대조한다. alpha만 회복되고 타이밍·개수·위치가 같아야 한다.
JSON/Python syntax/diff-check를 확인한다. C++ 변경이 없으므로 이 수정의 재빌드는 필요없다.
실행 중 Tool의 Load Saved와 최종 화면 확인은 사용자가 수행한다.
