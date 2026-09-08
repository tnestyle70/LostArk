# 카드미로 진입 수정 결과

## 구현과 원인

- 기본 카메라의 eyeOffset [0,14,-10]을 같은 각도의 [0,8.4,-6]으로 변경했다. 거리 60%로 가까워졌고 FOV 50과 lookAt, 별도 망원경 시점은 보존했다. 요청 이미지에 맞춘 초기 조정이며 최종 구도 확인은 사용자 몫이다.
- F1 MAZE 진입의 Apply_DebugKoukuHudMode가 NONE 외 모든 모드를 CLOWN으로 바꾸던 코드를 수정했다. MAZE는 NORMAL 몸체와 MAZE HUD/Q 명령을 함께 사용한다. Mario/Polymorph/Dance 디버그 변신은 그대로다.
- cardmaze.telescope는 실제 published triggerBox이지만 시각 모델은 없다. 기존 판정은 상자 안에서도 중심을 바라보지 않으면 망치 전방 검사에 실패할 수 있었다. 해당 OBB 내부는 방향과 무관하게 허용하고 enabled/높이 검사, 외부 거리/전방 검사는 유지했다.
- 미로 시작 전 HUD에 중앙 Q 안내를 표시한다. G로 획득하는 경로는 추가하지 않는다. 이전의 망치 획득 계약을 유지한다.

## 자동 검증

- Product Debug Engine/Shared/Server/Client 컴파일·링크·정상 배포 성공: `out/BuildPipeline/runs/20260908T103517881Z-debug-product.json`. 기존 인코딩·셰이더·PDB 경고는 남아 있다. PLAN의 C++ 네 파일 전문과 현재 UTF-8 코드 일치 확인.
- Server 카드미로 기존 contract test에 실제 CGameRoom 생성/프로파일 로드와 Debug MAZE 진입을 추가했다. NORMAL body + MAZE Q 슬롯, 실제 Handle_InteractionSlot → Update_Players의 12tick 판정 → 중앙 반대 방향에서 1인 문양 병사 spawn + 관전 flag를 확인했다. `--card-maze-contract-test`: failures 0.
- 정식 Publish-MapAuthoring Area publish 성공: 3231 placements, 8 runtime documents. authoring/runtime cardmaze.follow 값 동일, JSON parse 성공. 처음 일반 sandbox Python 검색이 실패했으나 PATH 지정·승인된 publisher 실행으로 성공했으며 runtime 파일을 직접 편집하지 않았다.
- 독립 read-only 검토에서 NORMAL interaction consumer와 Q/OBB/camera 경로 확인, 추가 actionable defect 없음.
- `git diff --check` 성공. 기존 dirty 작업 보존, 자동 stage/commit/push 없음. 변경 C++는 기존 UTF-8 유지, 신규 C++/project registration 없음.

## 남은 화면 확인과 명시적 제한

- 에이전트는 Client/UI 실행·조작·캡처를 하지 않았다. 원본 스크린샷으로 증상을 확인했으며 수정 후 화면은 아직 확인하지 않았다.
- 중앙 상자 시각 모델과 일반 플레이어의 망치 애니메이션 바인딩은 추가하지 않았다. 현재 애니메이션 바인딩은 광대 리그 전용이므로 일반 플레이어에게 임의로 붙이지 않는다. 빈 모션은 Client 승인 action을 실패시키지 않으며 서버 Q 판정과 시작 전환은 위 테스트로 확인했다.
- 따라서 Q를 눌렀을 때 일반 플레이어의 망치 휘두르기 모션이 없더라도 약 0.4초 뒤 관전·문양·병사 생성 상태를 확인해야 한다.
- 실제 사용자 실행 파일/접속한 서버의 빌드 버전은 이 스크린샷만으로 확정하지 못했다. 수정한 Debug Server와 Client를 모두 사용해야 한다. 공유 endpoint는 192.168.0.4:7777이며 이번 점검에서 not-listening이었다.

## 재확인 순서

수정된 Debug Server 재시작 → Client 재시작 → Lobby / KoukuSaydon → F1 / 카드미로 → 일반 캐릭터와 가까워진 시점 확인 → F1 닫기, F6 free camera가 아닌 플레이어 카메라 상태 → 진입 위치인 중앙에서 Q 한 번 → 약 0.4초 후 망원경 시점·본인 문양·병사 생성 확인. 1인 테스트는 해당 서버 방에 실제 한 명일 때만 허용한다. Q 반응이 없으면 F1 상태와 현재 서버 빌드를 함께 확인한다.
