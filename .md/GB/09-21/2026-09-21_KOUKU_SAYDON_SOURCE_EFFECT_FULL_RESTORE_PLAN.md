# 쿠크·세이튼 원본 액션 이펙트 full restore 계획

## 범위

첨부 이미지의 대형 세이튼 원본 공격을 원본 Action/ParticleSystem 후보로 준비한다. 새 이미지를 만들지 않고
`out/KoukuActionEffects20260912/candidate`의 source notify, CDO/archetype 모듈,
재질·리소스 연결을 사용한다.

## 원본 정본과 대상

| 구간 | 원본 profile/action | 후보 | 연결 목적 |
|---|---|---|---|
| 2관문 대형 세이튼 바람 | `MN_RPCT_06/4221804` | stage 000–005 | 불어날리기 바람과 airborne 시각 |
| 2관문 대형 세이튼 노란 시선 | `MN_RPCT_06/4221813` | stage 000–001 | 노란 시선, 바닥 aura, eye 02_03/04 폭발 |

바람 문서는 4221804의 stage 000–002를 원본 시계 순서로 묶고, 노란 시선은 4221813 stage 000을
독립 resource로 둔다. 조건 분기를 임의로 한 선형 동작으로
합치지 않으며, 실제 pattern occurrence는 현재 저장본을 읽은 뒤 stable ID로 append한다.

사용자 재지정으로 1관문 무력화(4219816/17/18)와 파1빨2 광선(4221838)은 이번 설치 범위에서
제외한다. 후보 추출 조사만 했고 해당 정본·패턴은 변경하지 않는다.

## 구현 절차

1. 원본 organization을 대상 action만 남긴 임시 projection으로 제한한다.
2. 기존 Kouku native material patch를 사용해 action 문서를 재생성하고 `Data/Effects/Authored`
   에 설치한다. 사용자 authored 문서가 있으면 CAS 검사로 덮어쓰지 않는다.
3. EffectCatalog/EffectResourceTree와 Kouku Composition의 presentation resource만
   stable asset ID로 append한다. 현재 dirty Composition/patternbindings의 다른 field는 보존한다.
4. root 통합 담당과 pattern occurrence 시각·카드 제거 범위를 협의한다. 원본 action stage가
   없는 슬롯은 임의 effect를 추가하지 않고 원본 누락으로 기록한다.
5. 대상 JSON/EffectCodec 구조, Resources-relative path, source material enabled,
   `git diff --check`를 검증한다. Client 실행·UI 캡처·최종 visual PASS는 수행하지 않는다.

## 보류 경계

원본 action 후보 설치와 pattern 연결은 분리한다. 실제 화면의 색·밝기·크기, airborne
물리와 낙사 판정, nav/dead-zone은 root 및 Server 담당 변경에서 검증한다.
