# World 표시 이펙트와 Alt V profiler 분석 구현 계획

작성일: 2026-09-11. 기준 HEAD `2d7b96693fb93c397a26632c221d5d29bf4a2ae1`, 브랜치
`codex/kouku-gate1-sequence-playback`. 넓은 미커밋 변경을 보존하고 자동 stage/commit하지 않는다.

## G00. 실제 입력과 작업 경계

사용자 첨부 이미지는 청록 클릭 지점 수축 표시와 금빛 바닥 원·아래 방향 화살표다.
정지 이미지로 시간 곡선을 확정하지 않고 원본 particle graph·material·geometry를 조사한다.
현재 `CClickMoveEffect`의 cursoreffect.gfx 기반 링은 이번 3D 원본의 근거로 사용하지 않는다.
이번 구현 소비자는 F1 Effect Tool V1이다. 플레이어 command나 Server gameplay는 변경하지 않는다.

최신 profiler JSON은 같은 세션의 누적 캡처 두 개다. frame 246~269의 CPU 평균은
90.230ms이고 GPU.Frame은 CPU 공급 공백을 포함할 수 있다. scope의 부모·자식 중복,
프레임 ID, capture 구간, Debug/Release 차이를 구분해 분석 문서를 작성한다.

## G01. World Resources와 V1 문서

원본 source graph의 mesh/sprite, MIC, texture, 수명·크기·방출·회전을 기존 importer와
material/carrier에서 재사용한다. runtime 파일은 `Client/Bin/Resources/Effect/World/`에
필요한 것만 설치한다. binary는 Git에 추가하지 않는다. 원본과 다른 저작 보정은 명시한다.

독립 stable ID는 `effect.world.mouse_click`과 `effect.world.move_destination`을 사용한다.
`Data/Effects/Authored` 문서, 기존 `EffectCatalog.json`의 direct authored 행과
`EffectResourceTree.json`의 World 부모/두 참조를 연결한다. 기존 항목은 보존한다.
새 Data 문서는 Client project/filter의 `96.DataFiles`에 None으로 등록한다.

## G02. Effect Tool과 재생 소유자

`Effect_Tool.h/.cpp` All Effects의 기존 owner 선택에 World를 추가한다. 목록은 metadata만
소비하고 Open/Play에서 선택 문서를 validate한다. 기존 Kouku 독립 authored 목록과 공통
Open/Play 흐름을 공유하며 보스 ownership을 World에 강제하지 않는다.

World의 Play All은 기존 `CEffectAuthoringSequencer`의 scene-player root snapshot,
V1 occurrence factory와 `CEffectObject`를 재사용한다. mesh·sprite의 motion은 같은
document clock으로 재생하고 준비 직후 첫 delta 제외 정책을 유지한다. 별도 player skill
animation이나 보스 Composition을 추측해 연결하지 않는다. 문서 자체 ModelCue가 있으면
같은 occurrence clock에서 재생한다.

클릭 표시는 한 번, 이동 위치 표시는 반복한다. 반복 정책은 임시 preview가 소유하도록
코드에서 설정하며 다른 Effect나 저장 sequence로 새 정책이 새지 않게 한다. Stop·재시작·
문서 교체와 실패 시 이전 정상 preview 보존을 검토한다. 새 C++ runtime 경로는 만들지 않는다.

실측한 원본 클릭은 `par_b_picking_01`의 4 emitter, 이동 표시는 `par_i_movetrack_01`의
12 emitter를 사용한다. 후자는 아래 방향 화살표·위 점·금색 ring의 원본 구조로 선택했으며
1관문에서 이 variant를 직접 선택하는 level/script 참조는 확인되지 않았다.
원본의 생략된 Required emitterLoops는 무한 반복인 0으로 해석한다. 이동 9개 반복 emitter와
3개 초기 pulse를 구분하고, Tool의 임시 재생창은 클릭 1200ms·이동 7000ms를 사용한다.
Playback의 bounded tail까지 기다린 뒤 반복하면 원·화살표가 먼저 사라지는 빈 구간이 생긴다.
기존 `Sample`로 같은 occurrence를 되감아 매 반복의 자원 재준비와 `Seek`의 pause 부작용을 피한다.

exact source material 13종 중 4개 기존 native profile을 재사용하고 9개는 비어 있는
2351~2359에 추가한다. 새 `Shader_EffectWorldNative.hlsli`는 기존 carrier/dispatcher가 소비하며
Client project/filter의 `97.ShaderFiles` None 항목으로 등록한다. 원본 signed particle alpha가
UV 이동 입력인 피킹 재질에는 범용 opacity clamp/cull을 추가하지 않는다.

## G03. 분석과 검증

분석 문서는 현재 scope/counter 목록과 실제 writer 유무, 빠진 GPU pass/asset identity/
allocation/job 계측을 구분한다. DOD, worker pool, task dependency, Chase-Lev deque,
work stealing, animation pose 공유, map culling/batching과 GPU overdraw/upload/copy 최적화의
적용 순서를 현재 코드와 캡처 근거로 설명한다. 이번 요청의 성능 범위는 진단·해결책 제시이며
JobSystem 전면 도입이나 렌더러 교체를 완료로 기록하지 않는다.

변경 JSON/XML parse, 기존 Effect scoped validator/실제 codec·Playback 수치 검사,
필요한 Debug compile/link와 `git diff --check`를 수행한다. 다른 빌드와 겹치지 않는다.
Client/UI 실행·조작·화면 캡처는 하지 않는다. 최종 화면 확인은 사용자가
Server + Client profile → arena → F1 → Effect Tool V1 → World → 각 Open/Play All로 수행한다.
