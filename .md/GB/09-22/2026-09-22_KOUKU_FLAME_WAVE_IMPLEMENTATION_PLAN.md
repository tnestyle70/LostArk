# 쿠크 화염파동 불바닥·14개 편집 그룹 구현 계획

## G00. 현재 기준과 변경 경계

기준 브랜치는 `GB/collider-pattern-bug-fix`, HEAD는 `d05a56c1d`다. 기존 화염파동의
설치 경위와 10지점 구성은 [09-12 결과 G15](../09-12/2026-09-12_KOUKU_GATE3_EFFECT_GROUPS_V1_IMPLEMENTATION_RESULT.md#g15-공통-불뿜기와-1234-화염-파동--2026-09-14)를 따른다.
이번 범위는 사용자가 요청한 불바닥 추가, 간격 축소와 14개 위치의 그룹 편집이다.

기존 full 두 문서는 4행의 1/2/3/4개, 총 10지점·260요소다. 전조는 Light,
기둥은 Sk_04_11의 12요소, 바닥은 WandDecal의 13요소다. WandDecal의 핵심 decal은
native2311과 `fx_a_decal_014`를 사용하며, FireWave 원본의 바닥 화염 native2873·2874는 없다.
화면의 바닥 불꽃을 전조 Point Light의 밝기만으로 복원할 수 없다.

Composition에는 사용자 변경이 있다. 현재 저작 파일과 실행 중 도구의 메모리 draft를
구분하며, 후보를 `out/KoukuFlameWave20260922`에 먼저 준비한다. 최종 승인 전에는
라이브 Data 교체나 publish를 수행하지 않는다. Client/UI는 사용자가 직접 확인한다.

## G01. prepare_kouku_flame_wave_refinement.py의 원본 연결

`Tools/EffectPipeline/prepare_kouku_flame_wave_refinement.py`는 설치된 source FireWave에서
`particlespriteemitter_16`과 `particlespriteemitter_23`을 선택한다. 각각 native2874의
ground ring과 native2873의 화염 sprite이며, 두 요소의 원본 EPAL_Z 축 잠금·크기·재질·
texture·색·alpha·수명을 유지한다.

독립 저작 위치 하나에서 재사용하도록 고정 StartLocation 분포만 원점으로 바꾸고
`authoredModuleOverrides`를 기록한다. 바닥 높이는 0.04m다. 원본 source 문서와 disabled
FireWave notify는 변경하지 않는다. 이는 요청한 독립 연출의 조합이며 원본 action 발사
계약이 복구됐다는 의미가 아니다.

기존 바닥 문서에 두 요소를 추가하고, full의 각 지점에서도 같은 source 조합을 사용한다.
기존 element ID·전조·기둥·WandDecal의 재질과 레시피는 보존한다.

## G02. 14개 manual 그룹과 배치

4행을 2/3/4/5개로 변경한다. 좌우 중심 간격은 5m에서 3.5m로 줄이고, 행 전진은
4.330m에서 3.031m로 줄인다. 첫 행은 전방 3.5m다. 이 배치는 사용자 요청에 따른
저작값이며 원작의 좌표를 회수한 값으로 설명하지 않는다.

기존 `flame-wave.rN.cN`은 Tool의 `manual.` 판별에 포함되지 않아 같은 Unattached
anchor로 묶인다. `manual.flame-wave.rN.cN`으로 그룹을 식별하고 기존
`Build_AttachmentElementGroups → Translate_AttachmentElementGroup → Save Changes`
경로를 재사용한다. 지점마다 전조·기둥·바닥을 포함한 28요소가 함께 이동한다.

새 C++ 파일과 runtime schema는 없다. 기존 세 Effect asset ID를 유지하므로
`.vcxproj`와 `.vcxproj.filters`의 신규 None 등록도 필요하지 않다.

## G03. Composition의 수명과 최신 저장본 병합

`build_kouku_flame_wave_groups.py::duration_ms`는 현행 Codec이 생략한 optional
`sourceScale.lifeTime`을 기본값 1로 읽는다. conservative active+tail 계약으로 새 바닥
수명과 full 수명을 계산하고 실제 Playback의 duration과 대조한다.

`prepare_composition_candidate`는 최신 디스크 저장본에서 resourceId, patternId,
occurrenceId로 대상을 찾고 기존 duration 값이 예상값과 같을 때만 교체한다. JSON의
해당 scalar token과 revision만 바꿔 무관한 내용·공백·BOM을 보존한다. 같은 필드의 변경은
충돌로 보고하며 파일 전체를 덮어쓰지 않는다.

resource 기본 수명과 같은 occurrence만 새 수명으로 확장한다. 기존 기본값과 다른
P58의 5,352ms 창과 사용자 위치는 유지한다. P49/P59의 전체 재생 끝이 부족하면
해당 pattern의 durationMs만 늘린다. revision은 최신 값에서 1 증가시킨다.

## G04. 검증과 최종 적용

후보 JSON의 유일 ID, Resources 상대 경로와 실제 파일을 검사한다. 실제 Codec의
Product staging과 Playback 전체 시간 sweep에서 finite·capacity·root 회전/이동을
검사한다. 그룹 함수 원문으로 14개 구분, 선택 그룹만 이동, Codec Save/Load 보존을 확인한다.

Composition은 기존 projector의 `--mode validate`를 격리 후보 repository에서 실행한다.
구조·projection 생성 실패와 기존 게시본의 `projected Product is stale`를 구분한다.
validate를 통과시키기 위해 라이브 projected output을 임의로 덮어쓰지 않는다.

최종 승인 후에는 최신 hash를 재확인하고 stable-ID field patch로 병합·백업·원자 교체한다.
필요한 domain publish, 실행 중 도구 Reload와 사용자의 최종 외형 판정은 별도 완료 항목이다.
