# 2026-08-16 Artist visual evidence

사용자가 직접 실행한 Debug Client/Effect Tool 화면을 보존한 수동 시각 증거다. `current`는 현재 복원 결과, `original_reference`는 원작 비교 화면이다. 자동 visual PASS로 사용하지 않는다.

| No. | 파일 | 구분 | 관찰 대상 | SHA-256 |
|---:|---|---|---|---|
| 11 | `11_artist_q_current_broken.png` | current | Q Sprite/Mesh Particle 사각 carrier와 색/alpha 결함 | `4D23721CBBC93603F6A34B21E050E160D2C6FA19873421877B86AA5ACD22F727` |
| 12 | `12_artist_q_original_reference.png` | original_reference | Q 먹물 stroke/droplet 원작 | `BCF741895CBAEEEE46223DDFD25C868C1413C1B73205560878D0E4D87D208B01` |
| 13 | `13_artist_lmb_ba_editor_missing_families.png` | editor_blocker | LMB BA1~BA4 Product row의 편집 family 누락 표시 | `4F6960F3235A673A282256F523257B5BA3725E7FB7365DF8EF482B393D4E7CAA` |
| 14 | `14_artist_a_current_white_billboard.png` | current | A 흰 Sprite billboard | `16BA70FCCBA4A92FE3EE0644BE04FD0A85EEDBA91814872BAC56EB4CB7E2CDB8` |
| 15 | `15_artist_a_current_noise_carrier.png` | current | A noise DDS가 최종 carrier처럼 노출 | `F45FDF3C6788FCA6F05FDAD83B9ECD1E1E0DE555761A0C46FDF4104A1BD22C19` |
| 16 | `16_artist_e_original_reference.png` | original_reference | E 먹물/백색 두루미 원작 | `032F9CDDDB96ACACA97A13F7E8FE0D3381FB98A71020E5092E032A9486E5510B` |
| 17 | `17_artist_e_current_missing_crane.png` | current | E 사각 carrier와 두루미 미출력 | `798EACD7F78FDC7410E57A2EE1029E12A50A14132D6EC1A82309FF6B18A4C4ED` |
| 18 | `18_artist_r_current_broken.png` | current | R 검정/보라 사각 carrier | `E5BF82D823F3DE2D579531000890D2A8D836DA84C280505E11D05EB8BC37FB8E` |
| 19 | `19_artist_r_original_reference.png` | original_reference | R 먹물/보라 impact 원작 | `51E95C0740C41459B9730AC8FF3FDC6FE46CE35A0ACD2703902081EA1C1E1D6F` |
| 20 | `20_artist_w_original_reference.png` | original_reference | W 넓은 먹물 splatter 원작 | `EE6CDE89D984B830922A932FE6EB8B4359CF581A6CE096EB8A72F622100B8733` |
| 21 | `21_artist_w_current_broken.png` | current | W 주황/검정 사각 sheet | `42E1B9244E11FC8B53051B6367658C26B7CE4C2D42387FD9553495ECABB70A1C` |
| 22 | `22_artist_v_current_mesh_particle.png` | current | V Mesh Particle 과대 water ring/과포화 | `C65F17CAD622AEC0E22ABECFA27C575A2A3D464F28D698159D03A1844C683CBC` |
| 23 | `23_artist_v_current_sprite_particle.png` | current | V Sprite Particle 흰 ray/사각 sheet | `16BDA8CF37EA5B855DFD331BF450EE9DDE69171DA9C9E20AF305A0DC94180012` |
| 24 | `24_artist_s_current_broken.png` | current | S 검정 사각 Sprite sheet | `2D9F397D87C78FA795EBBB580F97DEECA82E8334C009F130CAB357DECB927A7D` |
| 25 | `25_artist_q_current_user_reduced_two_mesh_plus_decal.png` | current_user_reduced | 사용자가 나머지를 `visible=false`로 저장한 뒤 남긴 Q 검격 Mesh 2개와 먹물 Decal. Mesh의 색·형태·전방 끝점 배치 비교 입력 | `2237C3D29C33B3D39D253007A0B30EB48264C827A6EF8160DCEAE2DA46F58573` |
| 26 | `26_artist_r_current_sprite_family_failure.png` | current_failure | R Sprite가 검정 사각 carrier, 백색 flow, 주황 sheet로 노출되는 공용 family 결함 비교 입력 | `4A8DF1A3CE69579E952A7308174B33506EC204451F2E814F7145C6B6DEFD9BDF` |
| 27 | `27_artist_w_current_sprite_family_failure.png` | current_failure | W Sprite가 거대한 보라·검정 사각 sheet와 파란 Approximate streak로 노출되는 공용 family 결함 비교 입력 | `0F77CB2C96164CDE26022AD5E2DF8B154FFE37A2BF0568D63D779F6EC71644BB` |

Artist F 31470은 사용자가 위치를 조정해 승인한 golden control이며 이 묶음에서 재생성하거나 수정하지 않는다.

## 2026-08-16 role-first 원본 참고 화면

아래 파일은 현재 출력이 아니라 사용자가 제공한 원작 구성 참고 화면이다. 자동 visual PASS나 source
asset identity의 단독 근거로 사용하지 않고, source occurrence를 역할별로 좁히는 수동 기준으로만 사용한다.

| 파일 | 구분 | 관찰 역할 |
|---|---|---|
| `artist_role_reference_2026-08-16/01_artist_common_ink_burst_reference_a.png` | original_reference | 검정·보라 먹물 Sprite가 중심에서 생성되어 퍼지는 초기 구간 |
| `artist_role_reference_2026-08-16/02_artist_common_ink_burst_reference_b.png` | original_reference | 먹물 Sprite가 바깥으로 퍼지고 dissolve되는 후속 구간 |
| `artist_role_reference_2026-08-16/03_artist_z_portal_yinyang_reference.png` | original_reference | Z 바닥 음양 Decal, 흐린 원형 Portal, 밝은 중심과 작은 검정 Particle |
| `artist_role_reference_2026-08-16/04_artist_t_v_burst_reference.png` | original_reference | T/V의 원형 경계, 먹물·밝은 Particle과 화면 확장형 burst 참고 |
| `artist_role_reference_2026-08-16/05_artist_t_dragon_circle_reference.png` | original_reference | T의 용 문양 바닥 Decal과 원형 먹물 경계 참고 |

### 사용자 역할 정본

- Q: 바닥 먹물 Decal, 전방 검정 반원형 검격 Mesh, 약 5개 보라 Particle 묶음, 퍼지고 dissolve되는 먹물 Sprite.
- W: 붓 기준 전방 부채꼴로 퍼지는 연보라·검정·보라 Sprite와 바닥 먹물 Decal.
- E: 붓 Trail, 흰 원형 Portal과 중심 Glow, 검정 먹물 Particle, 전방으로 날아가는 두루미와 주변 먹물.
- R: 연한 원형 Decal, 검정·보라 Sprite, 작은 흰색·보라색 Particle, 중심에서 액체 먹물처럼 퍼진 뒤 dissolve.
- Z: 바닥 Decal과 흐린 Portal, 검정·밝은 작은 Particle, 중앙으로 줄어들며 소멸하는 음양 Decal.
- D: 연한 흰색·검정 바닥 Decal, 붓 휘두름, 검정 호랑이, 먹물 Sprite, 작은 검정 검격 Mesh.
- T: 바닥을 헤엄치는 용 Mesh, 흰 경계 원형 Decal과 먹물 Sprite, 용 문양 바닥 Decal.
- V: 도화가 주변 연노랑·붉은 Mesh, 화면 전체 불꽃형 소용돌이와 회전 소멸.
- S: 바닥 먹물 Decal, 전방 작은 흰 Particle, 길이·끝광이 반복 변화하는 난초 형상, 검정 Sprite 종료.
- A: S와 유사한 바닥 Decal이 생성되고 나비 Particle이 날아다니는 구성.
- LMB BA1~BA3: 전방 검정 검격. LMB BA4: 작은 검격 2개와 큰 검격 1개이며 DimensionMaster 검격과 유사한 구성.

### 2026-08-16 공용 Sprite family 재현 범위

- Q의 25번 화면은 자동 선별 결과가 아니라 사용자가 직접 나머지 element를 숨긴 현재 저장 상태다. 이후 복원은 이 `visible=false` 상태를 보존하고 Mesh 2개와 Decal만 비교한다.
- R의 26번, W의 27번, 기존 E 17번과 S 24번은 각각 다른 스킬의 현재 화면이지만 공통적으로 source의 보조 noise/emissive lane이 최종 coverage sheet처럼 노출되거나, coverage/alpha 합성 없이 billboard 사각 경계가 남는 증거다.
- S는 사용자의 최신 관찰에서 난초/먹물 Sprite 대신 주황 사각형만 출력된다. 별도 새 화면이 첨부되지 않았으므로 기존 24번을 그대로 현재 재현 증거로 사용하며 새 이미지를 만들거나 중복 복사하지 않는다.
