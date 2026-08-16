# 2026-08-16 Warlord visual evidence manifest

이 문서는 사용자가 제공한 Warlord 원본 화면을 스킬 커맨드별 시각 근거로 고정한다. 이미지는
`워로드원본/`과 `C:/Users/user/Desktop/로스트아크이펙트이미지/워로드/`에 같은 파일명과 같은
바이트로 보존한다. 이 화면은 source occurrence 후보를 좁히는 참고 근거이며, 자동 visual PASS나
다른 스킬의 material/resource 근거로 승격하지 않는다.

| 파일 | SHA-256 | 관찰 역할 |
|---|---|---|
| `Warlord_Q00.png` | `94377e6860475854f5832e0f61b54e3c3e3ca09c9cf9ddfe0eccbb46aaaef59e` | 무기 발광, 전방 창끝형 검광, 머리 위 백색 발광, 검 주변 연기 |
| `Warlord_W00.png` | `d791691b0ae7bd0bd542347a4126ff36f08eb2f5cd7c0ab17ccb342d14d35727` | 붉은 발광·연기·불티와 탄환 흔적 |
| `Warlord_E00.png` | `454c958a2a624148be5e8c015bc4aa0a931f124f25d2be6758aeca042b52a6bd` | 돌진 중 붉은/검은 검격과 후속 폭발 |
| `Warlord_R00.png` | `6c77dd55a7682b86485ca45bc6a3e777ebf7ed5c49b3a35581322efa373478f2` | 지면 충돌, 백색 기둥, 파편, 연무와 바닥 흔적 |
| `Warlord_A00.png` | `deac9d5c5f7b306f6f08ea52fcd2e73b633114b7d195527080383f134ad8caa6` | 네 갈래 창끝 사슬의 부채꼴 발사·회수 |
| `Warlord_Z00.png` | `deac9d5c5f7b306f6f08ea52fcd2e73b633114b7d195527080383f134ad8caa6` | 캐릭터 주변 반투명 구형 방어막 |
| `Warlord_Z01.png` | `d08970cf7d7058455238ad7f9ba0a9d356e06d02c53750f26ca66ce1d37ce1b1` | 워로드를 앵커로 둘러싼 여섯 개의 방패와 노란색 mesh particle |
| `Warlord_S00.png` | `1c1f5844027591e4b53201da95117026f2498c1d90dc8b8fc664b6374eb70b1c` | 전방 파란 방패/부채꼴 형상과 잔류 입자 |
| `Warlord_D00.png` | `f3bb35b0a085c3903d422000a2eff6e3a7fef78377f10b1797eec7c65fe26031` | 백색 반투명 기둥, 전기, 지면 발광과 검은 파편 |
| `Warlord_F00.png` | `33667bd4e3a64c7cc9d86424af4c94a82ff142831858bcdb31bcf716e79a029e` | 검을 감싸는 청색/보라색 emissive와 근거리 바닥 원형 |
| `Warlord_F01.png` | `c98784f842d858885b3873c4b8b3cb968a12978da355a6c4eb55da5313965880` | 넓게 퍼지는 번개, 백색 발광점과 옅은 청색 바닥 영역 |
| `Warlord_V00.png` | `c00db0b0545a1365ca1debaa0662a301f416a0b518abd27bc5002a118408e442` | 거대한 전방 화염, 검은/붉은 연기와 불티 |
| `Warlord_V01.png` | `3f224298dce0929ab84553097c28c2ef2237c19b18c5d7dca69c7c9e730da956` | 검게 탄 대형 바닥 흔적과 붉은 잔불 |

## 합성 화면과 누락 경계

- `A00`과 `Z00`은 같은 원본 프레임이다. 한 화면에 A 사슬과 Z 방어막이 함께 존재하므로 각
  커맨드의 관찰 근거로 같은 SHA를 별칭 보존했다. `Z01`은 이후 추가된 Z 단독 근거이며,
  워로드 앵커 기준 여섯 방패와 노란색 mesh particle의 후보 선별에는 `Z01`을 우선한다.
- 사용자 문장에는 D가 6·7번째 화면이라고 적혀 있으나, 화면 자체는 6번째가 S 방패 역할,
  7번째가 D 기둥·전기 역할에 명확히 대응한다. 후보 혼입을 피하기 위해 `S00=6`, `D00=7`로
  분리했다.
- BA/BA3의 백색 sword-tip mesh 설명에 대응하는 첨부 화면은 없다. 다른 화면을 BA 증거로
  재사용하지 않는다. BA는 exact source occurrence와 사용자 서술로만 후보화하고 최초 육안 판정은
  계속 미완료로 둔다.

## 적용 경계

- `PlayerSkills -> Warlord.skillbindings -> 기존 clip` 연결을 유지한다.
- 해당 clip의 exact source system 안에서 위 역할과 일치하는 stable occurrence만 Visible 후보로
  선택한다. 원본 element는 삭제하지 않고 비선택 행을 `visible:false`로 보존한다.
- Full, `AUTHORING_APPROXIMATE`, Hard/fail-closed 분류는 변경하지 않는다. Approximate는 표시를
  유지한 채 사용자 조정 후보가 될 수 있지만 Hard는 계속 실행하지 않는다.
- Warlord A 17090의 chain06/07은 `SOURCE_MASKED_WPO_ARITHMETIC_UNAVAILABLE` 경계를 유지한다.
  `fx_d_grid_016.dds + effect.standard`를 exact 입력으로 되살리거나 다른 스킬에 일반화하지 않는다.
- Z는 현재 101개 unified 문서 범위 밖의 stance presentation이다. `wgl_sk_defence_start`의 exact
  source occurrence를 별도 intake한 뒤 `Z01` 역할과 대조하며, material/DDS 근거가 닫히기 전에는
  다른 스킬 fallback, gameplay cue 연결 또는 Product 승격을 하지 않는다.
- 사용자 직접 Effect Tool 육안 확인 전에는 visual PASS로 기록하지 않는다.
