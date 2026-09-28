# 2026-09-28 선풍참혼(34140) 트라이포드 없는 판정 복원 RESULT

## 증상

- A(선풍참혼) 1타·2타마다 투사체 원 콜라이더가 하나씩 나갔다.
- 투사체를 뺀 뒤 2타 뒤쪽 세 찌르기에 콜라이더가 없었다.

## 원인

- `fill_projectiles.py`의 "효과 PK가 기본 스킬을 가리키면(pk // 10 == skillId) 그룹 무관하게 승격" 규칙이 `seq=3` 트라이포드 그룹의 MISSILE 4개(341405/341409/341411/341413)를 base 체인에 넣었다. 기준 `seq=0` 그룹에는 스폰이 없다. 08-18 RESULT가 이미 "선풍참혼 seq 3의 창 던지기는 base가 아니다"로 판정한 항목을 09-23 PK 합집합 규칙이 되살렸다.
- `_04` 클립의 0.55/0.75/1.10초 타격은 PK 없는 `ParticleHit` notify다(`.loa`의 `CEFActionNotify_ParticleHit` 레코드는 시각·count·interval·"FX" 라벨만 갖는다). `fill_animevents_hit_shapes.py`는 Effect 판정이 있는 클립에서 ParticleHit를 전부 버려 세 판정이 사라졸다. 원본 툴팁 `tip.desc.skill_34140`은 2타를 `341404×2, 341401, 341402, 341402, 341403`으로 명시한다.

## 구현

- `fill_projectiles.py`: 스폰 승격을 스킬의 최저 clipseq 그룹으로만 제한. 상위 그룹 스폰은 효과 PK가 기본 스킬을 가리켜도 트라이포드 표현으로 본다. base 그룹의 unbound 클립 재배치 규칙은 유지.
- `build_base_hit_rows.py`: `.basehits`에 `particle=` 행 추가. base 체인에서 Effect 판정이 있는 클립의, 60ms 안에 뒤따르는 Effect 판정이 없는 ParticleHit를 모아 툴팁 damage 목록 중 아직 판정되지 않은 행(PK 일치 → ValueA/B/F/Key 일치 → 형상 일치 순으로 소거, hitrepeats 반복 수 반영)과 맞춘다. 개수가 다르면 마지막 클립의 마지막 Effect 판정 뒤 ParticleHit만 툴팁 꼬리와 맞추고 나머지는 기존 skilltiming 위치 추정을 유지한다. Effect 판정이 없는 클립은 대상이 아니다. 행은 DB의 AreaType/AreaRange/AreaAngle/AreaHeight/AreaOffsetX/AreaRemoveRange/MaxAmount/PushMinTime/PushMinRange/Freeze* 를 notify 필드 이름으로 싣는다(기존 notify 값과 필드 대응 실측 일치).
- `fill_animevents_hit_shapes.py`: `particle=` 행을 읽어 해당 ParticleHit에 자기 형상을 주고, Effect 판정 클립에서도 그 행을 남긴다.
- 재생성: 창술사 `projectiles.json`(0개), `basehits`(+3), `animevents`(+3 HIT), `hitshapes.json`. 34140 2타 판정: 199ms×2 박스, 438ms 박스, 650/850ms 부채꼴 2.2m·240°, 1200ms 부채꼴(넉백 110/50, freeze 600).

## 검증

- `fill_projectiles.py`, `fill_animevents_hit_shapes.py`, `build_hitshapes.py` `--check`: Warlord/Artist/DimensionMaster unchanged. `build_base_hit_rows.py`는 타 클래스 particle 행 0개(Artist 31420은 개수 불일치·꼬리 없음으로 진단만 출력).
- `Publish-GameplayBalance.ps1 -Mode Publish` 성공, `SKILLSTAGEPROJ 34140` 0행, `SKILLSTAGEHIT 34140 1` 15행(5타격×3 result).
- 사용자 로컬 Server+Client(Debug) 확인: 투사체 없음, 2타 콜라이더 정상(사용자 서면 판정).

## 남은 항목

- 1타 후퇴 클립 `_02` 50ms 박스 판정은 툴팁에 대응 행이 없다(현재 skilltiming 위치 추정). 사용자 결정 대기.
- 34120 연환섬 `_01` 400ms는 Effect 판정 없는 클립이라 이번 규칙 대상이 아니다. 툴팁 정렬로 보면 341200 박스가 맞지만 변경하지 않았다.
- 쿠크 projector byte-parity 실패(CRLF + 부동소수 끝자리)는 09-27과 동일. 로컬 재생성한 `KoukuSaydonEncounter.json`, `KoukuSaydon.patternbindings.json`은 커밋하지 않는다.
