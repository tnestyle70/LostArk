# 2026-09-25 에스더 5종 사운드 원작 복원 RESULT

작성자: JS · 브랜치 `feature/esther-bahuntur-wei-source-effects`

목표: 에스더 5종 strike에 원본 소환 시퀀스의 AkEvent를 원래 시각으로 붙인다.

**현재 상태: 완료. 효과음·대사 모두 사용자 청취 확인(09-25 "잘 나와").**

## 1. 원본과 시각

`Tools/SoundPipeline/build_esther_sound_cues.py`가 이펙트와 같은 시퀀스 파서(`build_esther_bahuntur_source_effects`)로
AkEvent 액션과 Timer 지연을 읽는다. cue 시각 = 이벤트 시각 − 카메오 등장 시각(바훈투르만 1.6s, 나머지 0).
실리안은 이펙트 문서가 Action 542600이지만 `533000` 시퀀스의 FX 시각(0.8/2.9/3.1/4.2s)과 같아 같은 시계다.

| 에스더 | 시퀀스 | cue(ms:event) |
|---|---|---|
| 실리안 `NPC_59030` | 533000 | 700 Attack1_1, 1850 Attack1_2·1_3, 3000 Attack1_4, 4050 Attack1_5 |
| 웨이 `NPC_58700` | 532100 | 0 Waye1_1, 244 Waye1_2, 1500 Waye1_4, 1683 Dochul1_1, 2030 Waye1_2, 2800 Waye1_4, 2900 Dochul1_2, 3280 Waye1_3, 4683 Waye1_5, 4700 Dochul1_2, 5634 Dochul1_3, 5716 Waye1_6 |
| 바훈투르 `NPC_59060` | 532200 | 0 Attack1_6(원본 −1.6s, 클립 시작으로 당김), 300 1_1, 775 1_2, 1034 1_3, 1700 1_4, 2200 1_5 |
| 니나브 `NPC_59504` | 532300 | 0 1_1, 630 1_2, 1100 1_3, 1685 1_4, 3300 1_5 |
| 이난나 `NPC_59620` | 532400 | 0 1_1, 0 1_3, 250 1_2 |

## 2. 변경

- wav 62개(효과음 이벤트 28 → 31개, 대사 5 → 31개) → `Client/Bin/Resources/Sound/Asther/<event 소문자>__<mediaId>.wav`. **Drive 전달 필요.**
- `CharacterSoundCatalog.json` Esther 항목 +33(기존 5개 유지).
- `EstherActionSoundCues.json` 2 → 37 cue. 실리안의 기존 추정 cue `Silian1_Attack9_Cast1`(연합군 Action 소리, 0ms)은 시퀀스 cue로 교체. PLAYER cast cue는 유지.
- `EstherActionSoundCueDocument.cpp`: `timingBasis`에 `SOURCE_SUMMONS_SEQUENCE` 허용.
- `test_esther_action_sound_contract.py` 새 구성으로 갱신.

## 3. 검증 / 남은 것

- 실행함: dry-run 28/28 해석, 계약 테스트 3/3 OK.
- Debug Product 빌드 PASS — `out/BuildPipeline/runs/20260925T134156914Z-debug-product.json`(Client OBJ 1).
- 하지 않음: 청취 확인(사용자).
- 첫 청취(09-25): 효과음은 나오지만 에스더 대사가 없음. 시퀀스의 `Attack1_*`는 전부 효과음이고, 대사는
  `EFTable_EpicSkill.SkillVoice`(`Esther_<이름>1_Attack1_Vox1_1_2d`, 변형 5~7개)다. EpicSkill이 SkillVoice를 컷인
  `SkillMC`와 짝으로 두므로 대사 cue는 NpcCatalog `cutinMovie.delayMs`(실리안·니나브·이난나 0, 웨이 2600,
  바훈투르 500)에 둔다. 시퀀스 시각이 아니므로 `PROJECT_TUNED_EDGE`.
- 대사 추가는 데이터만 바뀌어 빌드 없이 Client 재시작으로 반영된다.
