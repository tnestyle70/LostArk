# 2026-08-18 발탄 1~67 source-sequence 패턴 프리뷰 결과

branch: `codex/valtan-wall-contact-destruction-and-pillars`

기존에 clip을 임의 조합하고 짧은 시간으로 잘라 재생하던 1~67 프리뷰를 폐기했다.
새 프리뷰는 실제 `Valtan.clipseq`의 action·sequence를 strict join해 source 순서와 native
재생 길이를 그대로 사용한다.

## G00. 구현 결과

- `Valtan.patternpreview.json`: formatVersion 2, 정확히 67 patterns
- source selector: 60개, direct `clips`·`previewMs`·`quick`·`SOURCE_EXACT`: 0개
- confidence: `SOURCE_FAMILY_DIRECT` 19, `USER_CONFIRMED_FAMILY` 14,
  `CANDIDATE` 11, `COMPOSITE` 15, `UNRESOLVED` 5, `NO_ANIMATION` 3
- marker 행: 7, 9, 12, 16, 28, 35, 41, 46
- 핵심 교정: 1=`400422:0` 연속돌기 1회전, 2·5·10·18·24=`400424:0`
  단발 대쉬, 4=`420633:4`, 21=`420629:1`,
  32=`420616:4` candidate, 48=`420658:4 + 420659:1` candidate
- source sequence의 ordered clip과 whole-sequence repeat를 보존한다.
- 모든 실제 clip은 non-loop로 시작하고 native 전체 길이로 재생한다.
- scheduler를 Tool `Render()`에서 분리해 창을 접어도 진행하며, Tool 비활성화·target 교체 시
  안전하게 Stop한다.
- 프리뷰가 model playback을 소유하는 동안 일반 clip·chain 조작을 잠그고 외부 clip 교체를 감지한다.
- `CBody_Valtan`도 model animation clock을 사용해 0.5~3.0배속에서 scheduler와 동기화한다.
- 완료/Stop은 속도 1.0과 `mesh_idle_battle_1` loop로 복구한다.

제품 Server 패턴, `Valtan.patternbindings.json`, damage, Effect, 월드 환경 연출은 변경하지 않았다.

## G01. 자동 검증

```text
ClientFrontendHarness Debug/Release build
  PASS

ClientFrontendHarness --valtan-pattern-preview-fast
  Debug  12 PASS / failures: 0
  Release 12 PASS / failures: 0

Client Debug/Release build
  PASS

Valtan.patternpreview.json
  JSON parse PASS / patterns 67 / selectors 60 / markers 8
  legacy direct fields 0 / source selector join PASS

Client/ClientFrontendHarness project + filters
  XML parse PASS

UI playback 소유권 P0/P1 재감사
  남은 재현 가능한 P0/P1 0

scoped git diff --check
  PASS (기존 LF -> CRLF 경고만 존재)
```

Release 빌드에는 기존 C4819 codepage 경고와 DirectXTK PDB LNK4099 경고가 있었지만
컴파일·링크 오류는 없었다.

## G02. 사용자 화면 확인과 남은 경계

Client 화면은 자동 실행하지 않았다. 사용자가 Animation Tool에서 `[Boss] Valtan`을 선택하고
`Valtan 1-67 Pattern Preview -> Play All 1-67`을 눌러 육안 일치도를 판정한다. 빠른 순서 검토는
`Speed 3.0x`, 개별 동작 확인은 목록 선택 후 `Play Selected`를 사용한다.

자동 검증은 source 문서 join·순서·재생 계약을 보장한다. 원본 영상과의 프레임 단위 일치는
자동 확정하지 않았다. `CANDIDATE` 11개와 `UNRESOLVED` 5개는 사용자 영상 대조 후 확정해야 하며,
근거가 없는 후보를 완성값으로 표시하지 않는다.
