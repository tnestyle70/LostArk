# 히트 영역 와이어 오버레이 — 애니메이션 툴 콜라이더 연결

작성자: JS · 2026-08-10 밤 · 브랜치 `feature/warlord-cloth-bone-chain`

팀장 요청 "콜라이더를 툴에서 연결해서 보고 저장하게"의 구현. PR #79(server-authoritative
collider slice)가 범위 밖으로 남긴 "animation hit volume authoring"을 채운다.

## 1. 새 문서를 만들지 않은 이유

조사 결과 히트 판정 데이터는 애니메이션 툴에 **이미 전부 있었다.** `.animevents`의
HIT 이벤트가 시간창·반복 히트에 더해 `HIT_PARAMS`로 형태까지 소유한다:
`iAreaType`(1 box / 2 fan / 3 circle·ring), `iAreaRange/Angle/Height/OffsetX/Inner`.
원작 skilltiming의 수치가 Reference 스탬프로 들어오고 `hit detail` UI가 편집하며
Save가 저장한다. 별도 hitvolumes 문서를 만들면 같은 역할의 두 번째 경로가 된다.
빠져 있던 것은 **씬에서 보는 것** 하나였고, 그것만 붙였다.

한 스킬의 다중 히트 구간은 원래 구조로 지원된다 — HIT 이벤트를 여러 개 두면 되고,
각각 자기 시간창과 형태를 갖는다.

## 2. 구현 — `Render_HitAreaWires` (Animation_Tool)

현재 클립의 HIT 이벤트 중 **선택된 것(노랑)**과 **재생 헤드가 시간창 안인 것(빨강,
반복 히트 포함)**을 씬 캐릭터 위 XZ 와이어로 그린다. `hit detail` 드래그에 실시간
반영. Events 헤더의 `show areas` 체크박스로 토글.

- 형태 매핑: box = 전방 길이(ar)×전체 폭(ah) — 12m×3m 창 찌르기 비율로 판정.
  fan/circle/ring/각도제한 링 섹터는 호 경로 공유(aa=sweep, arem→inner).
  ax는 전방 오프셋.
- **단위: 원작 100유닛 = 1m.** 스킬 34040 `ar=250` ↔ 밸런스 `maximumRange 3.0`으로
  검증했다.
- 투영: `Get_Transform(D3DTS::VIEW/PROJ)` → NDC → **ImGui 메인 뷰포트 rect**.
  이 프로젝트의 ImGui 스크린 좌표는 뷰포트 원점(`Pos`)을 포함하므로 (0,0) 기준으로
  매핑하면 창 위치만큼 어긋나 아무것도 안 보인다(첫 구현이 이 함정에 걸렸다.
  Character Select 오버레이의 `WorkPos + ...` 관례가 정본). 드로우는
  `GetBackgroundDrawList(viewport)` — 툴 창이 와이어 위에 유지된다.
- 캐릭터 transform은 `CGameObject::Get_Component` 베이스 한정 호출로 얻는다
  (`CContainerObject`가 두 인자 오버로드로 가리고 있다).

저작 워크플로: 클립 선택 → `Import original`(타이밍) 또는 Skill Timing Reference
창의 `stamp`(형태 포함 원작 수치) → 와이어 보면서 `hit detail` 조절 → Save.
notify 임포트는 area=0이라 와이어가 없는 게 정상이며, 형태는 stamp나 수동 입력으로
넣는다.

## 3. 같이 고친 것 — animevents 문서 불일치 3건

창술사 문서가 `Load rejected: effectref is only valid for v4/v5`로 통째로 거부되고
있었다. 통합 커밋 `e8dc2eb`가 창술사(41행)·도화가(14행)·워로드(27행)에 v4/v5 전용
`effectref` 행을 넣으면서 헤더를 v3로 남긴 것이다. effectref 행들이 v5 전용
anchor/follow/transform 토큰까지 쓰므로 세 파일 헤더를 **v5로 승격**했고, 창술사의
payload·effectref 둘 다 없는 빈 스텁 1행(`flm_abn_fear_1` EFFECT)을 제거하며 행 수를
3132→3131로 보정했다. v5 로더는 `src=orig` 행을 SOURCE_REFERENCE로 정상 해석한다.
차원술사(v5)·건슬링어·슬레이어(effectref 없음)는 원래 정상이었다.

## 4. 검증

```text
Client Debug 빌드                  통과
실행 확인 (사용자 육안)             창술사 문서 로드 복구, HIT 행 선택 시 와이어,
                                  area/range 드래그 실시간 추종, 창 위치 무관
정본 회귀                          커밋 전 실행
```

## 5. 남은 것

- **서버 소비 슬라이스.** 저작된 HIT area를 `Publish-*`로 Server 판정에 내리는 것.
  현재 서버 스킬 판정은 `maximumRange` 원 하나다(`PlayerSkillSystem.cpp:380`).
  Shared엔 box/fan/ring/cross 판정이 이미 있으므로 publisher + 스킬별 shape 소비만
  연결하면 된다. balance/provenance 계약과 엮이므로 별도 작업.
- 본 앵커(특정 본 추적 판정)는 현 데이터 계약(ax 전방 오프셋뿐)에 없다. 투사체형
  이펙트 콜라이더가 실제로 필요해질 때 계약 확장으로 다룬다.
