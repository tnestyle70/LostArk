# 2026-08-21 플레이어 카메라 셰이크 원본 데이터 확정·추출 RESULT

요청: 스킬 클립의 SHAKE notify에 맞춰 카메라 흔들림을 주고 싶다 — 원작에 강도·
방향·줌 데이터가 있는지 확정하고 추출까지 (1단계). 런타임 연출은 다음 슬라이스.

## 0. 결론

원작 SHAKE(`CEFActionNotify_ViewShake`)는 notify마다 인라인
`CEFCameraViewShake` 블록을 갖고, 강도·주파수·블렌드·FOV(줌)까지 전부 저작돼
있다. 4직업(창술사 179 / 워로드 92 / 도화가 59 / 차원술사 66 = 396건)의 SHAKE
행에 스펙을 추출해 실었다. 판정·서버 무관, 데이터만 변경.

## 1. 필드 시맨틱 (LANCEMASTER 333건 전수 실측, mark 정렬)

`CEFCameraViewShake` 블록 기준 오프셋:

```text
+12 duration  +16 blendIn  +20 blendOut     (초; notify d=0인 스킬 다수 — 셰이크 길이는 여기가 정본)
+24/+28 x(진폭,주파수)  +36/+40 y  +48/+52 z (주파수 Hz 2~200; 진폭 0이면 그 축 비활성 — amp0+freq만 있는 행 존재)
+60/+64 FOV(진폭,주파수)                     (도 단위, 음수 = 줌인 펀치; 창술사 유성강타 15, 평타 0.3)
```

- (진폭,주파수) 쌍 검증: 전 레코드에서 진폭 0인데 주파수만 있는 행은 있어도
  그 역은 0건. x==y가 대부분이지만 z는 다른 스킬 다수 → 3축 분리 방출.
- UE3 CameraShake(OscillationDuration/BlendIn/BlendOut + 축별 Amp/Freq + FOV)
  관례와 정확히 일치.
- 줌인아웃: 일반 스킬은 이 FOV 성분이 전부이고, 궁극기 전용 대형 카메라 연출은
  별도 notify `CEFActionNotify_UltimateSkillCameraControl`(창술사 4건) — 미해석,
  후속.

## 2. 변경

### buildScript (Git 밖)

- `extract_action_loa.py`: ViewShake notify에서 위 블록을 파싱해
  `dur=..;in=..;out=..;x=a,f;y=a,f;z=a,f;fov=a,f` 스펙 문자열을 SHAKE 행의
  asset(→ animevents payload)으로 방출. 포맷 버전 불변(파서는 미지 키 무시,
  payload는 불투명 문자열).
- `merge_shake_payloads.py` 신규: Authored `.animevents`는 합성 HIT 셰이프가 든
  런타임 정본이라 재생성하지 않고, `SHAKE payload=""` 행만 (clip, startms)로
  매칭해 외과 병합. 동일 시각 중복 스펙(워로드 1, 차원술사 4)은 소스 순서 첫
  스펙 채택을 로그로 남김.
- 차원술사 추출 인자 확정(기존 미재현 문제 해소): loa `DIMENSIONMASTER.loa`,
  wmodel `DimensionMaster_Character.wmodel`, prefix **`pc_sp_m_00_sk_`**
  (`_sk`는 메시 이름 유래 — `pc_sp_m_00_`만 주면 0클립). 워로드는
  `GUNLANCER.loa` / `wgl_`.

### 저장소

- `Data/Animation/Reference/<4클래스>/<C>.animnotify` — 재추출 교체. 회귀:
  기준본 대비 diff가 SHAKE 행 외 0줄(창술사 442=221×2 등 전 클래스 확인).
- `Data/Animation/Authored/<4클래스>/<C>.animevents` — SHAKE payload만 병합
  (numstat 59/66/179/92 전부 1:1 교체, 누락 0).

## 3. 실행한 검증

```text
animnotify 재추출 회귀        4클래스 non-SHAKE diff 0
merge                        396/396 patched, missing 0
fill_animevents_hit_shapes --check  4클래스 전부 unchanged (HIT 무결) exit 0
git diff --numstat           변경 8파일 전부 동수 교체
```

## 4. 다음 슬라이스 (미구현)

- Client 카메라 셰이크 표현: 스킬 재생 시 authored SHAKE payload 소비 →
  감쇠 오실레이터(3축 + FOV, blendIn/out)로 카메라 오프셋. 표현 전용.
- 축 의미(x/y가 화면 기준인지 월드 기준인지)와 진폭 단위는 런타임에서 원작
  화면과 육안 대조로 캘리브레이션.
- `UltimateSkillCameraControl`(궁극기 줌 연출) 해석은 별도 후속.
