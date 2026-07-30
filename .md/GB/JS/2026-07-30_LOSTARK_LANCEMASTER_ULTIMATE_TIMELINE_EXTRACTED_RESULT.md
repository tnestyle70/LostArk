# LostArk 창술사(LanceMaster) 궁극기 타임라인 추출 성공

작성자: JS · 2026-07-30

팀원(tnestyle70)의 UPK 추출 파이프라인(`Tools/LevelPlacementExtractor/
extract_ue3_placements.py`)을 사용해 **창술사 SkillCam UPK를 복호화하고, umodel이
못 뿜던 궁극기 시네마틱 타임라인(애니 클립 시작 시각 · 이벤트/셰이크 시각)을
실제 값으로 추출**했다.

## 1. 핵심 결론

1. **UPK 경로는 base16 없이 바로 복호화된다.** `.lpk`(데이터 테이블)와 달리
   SkillCam은 `.upk`라서, 저장소에 이미 있는 UPK AES 키
   (`V1ZEG1PL34V77SQW39A9I4VUW34T6L15`)로 AES-256 ECB + LZ4 복호화가 그대로 된다.
2. `STANDARD_SKILLCAM_LANCEMASTER`(물리 `NU5OQ5GQ9JN3PAAJ5H9UA5OJXH5NUXG.upk`)를
   복호화해 **ver 868 / names 338 / imports 83 / exports 214**를 정상 파싱했다.
3. umodel은 이 Kismet/Matinee 클래스의 프로퍼티 값을 안 뿜지만, 이 파이프라인은
   **복호화된 logical 스트림에서 tagged property를 직접 파싱**하므로 InterpData
   길이, InterpTrackAnimControl 애니 키, InterpTrackEvent 이벤트 키의 **시간 값까지**
   나온다.
4. 즉 **수작업 타임라인 저작 없이 궁극기 3종의 타이밍을 데이터로 확보**했다.
   (일반 전투 스킬은 여전히 `.lpk`/base16 영역 — 별개.)

## 2. 추출한 궁극기 타임라인 (실측)

`interpdata`의 `InterpLength`가 각 시네마틱 길이, `InterpTrackAnimControl`이 애니
클립·플레이레이트·시작 시각, `InterpTrackEvent`가 이벤트/카메라셰이크 시각이다.

### 궁극기 A — Squall Lance (스퀄 랜스) · InterpLength 5.355s

| time | 애니 클립 | playRate | 비고 |
|---:|---|---:|---|
| 0.000s | sk_super_squalllance_01 | ×1.2 | |
| 1.787s | sk_super_squalllance_02 | ×1.0 | |
| 3.547s | sk_super_squalllance_03 | ×1.0 | rootMotion |
| 4.347s | sk_super_squalllance_04 | ×1.0 | |

| time | 이벤트 |
|---:|---|
| 0.000s | playrate_0 |
| 0.899s | shake01 |
| 1.218s | shake02 |
| 1.753s | shake03 |
| 2.230s | playrate_1 |
| 2.864s | shake04 |
| 4.430s | playrate_2 |

### 궁극기 B — Gae Bolg (게볼그) · InterpLength 3.841s

| time | 애니 클립 | playRate |
|---:|---|---:|
| 0.000s | sk_super_gabolg_01 | ×1.0 |
| 1.567s | sk_super_gabolg_02 | ×1.0 |

이벤트: playrate_0 @0.0s, playrate_1 @1.57s, playrate_2 @3.84s.
리커버리 변형 `interpdata_5`(sk_super_gabolg_01_re/02_re)는 동일 타이밍.

전체 기계 판독본: `scratchpad/lancemaster_ultimate_timeline.json`.

## 3. 방법

```python
import extract_ue3_placements as ex          # 팀원 도구를 모듈로 임포트
physical = open(NU5OQ...upk,'rb').read()
summary  = ex.parse_summary(physical)
logical  = ex.decompress_package(physical, summary, ex.LOSTARK_KR_AES_KEY)  # AES-256 ECB + LZ4
names    = ex.parse_name_table(logical, summary)
exports  = ex.parse_export_table(logical, summary, names)
# InterpTrackEvent / InterpTrackAnimControl export의 serial을 tagged-property로 파싱
```

- 도구의 `decode_property_value`는 struct 배열(EventTrack, AnimSeqs)을 `{count,size}`로만
  주므로, **struct 배열을 직접 파싱**하는 드라이버를 따로 짰다
  (`scratchpad/parse_timeline.py`). 각 키는 `[raw Time float][tagged props: AnimSeqName/
  PlayRate/EventName ...][None]` 구조이고, **Time은 tagged 블록 직전 float**다.
- 시간 값이 각 스킬의 `InterpLength` 안에서 단조 증가하고 애니 길이와 정합해
  판독이 맞음을 교차 검증했다.
- 표준 파이썬이 없어 Blender 5.0 번들 파이썬으로 실행. capstone 등 불필요.

## 4. 남은 것 / 경계

- 여기서 얻은 것은 **궁극기/각성 3종**(super 계열)의 시네마틱 타이밍이다.
  일반 전투 스킬(~8종)의 이펙트/사운드/프레임 결선은 `.lpk` 데이터 테이블에 있고
  base16(WinLicense 보호) 없이는 못 읽는다(→ [LPK 디코더 문서](2026-07-30_LOSTARK_LPK_CONTAINER_DECODER_RESULT.md)).
- `shake01~04`는 이벤트 이름이며 실제 흔들림 크기는 이 패키지의 `camerashake`
  export(2개) 파라미터에 있다. 필요하면 같은 방식으로 파싱 가능.
- 이 파이프라인은 **다른 클래스의 SkillCam도 동일하게 복호화**한다
  (`STANDARD_SKILLCAM_<CLASS>`). 창술사 외 캐릭터도 같은 드라이버로 뽑을 수 있다.

## 5. 산출물

```text
scratchpad/decrypt_skillcam.py            SkillCam UPK 복호화 + export 덤프
scratchpad/parse_timeline.py              InterpTrackEvent/AnimControl struct 배열 파서
scratchpad/skillcam_logical.bin           복호화된 logical 패키지(재사용)
scratchpad/skillcam_timeline.json         파싱 원본
scratchpad/lancemaster_ultimate_timeline.json   정리된 최종 타임라인
```

원본 데이터는 Smilegate 저작물이다. 복호화 산출물은 Git에 올리지 않는다.
