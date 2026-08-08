# 로그인 인트로 플립북 — 보류

## 상태
기능 보류. 코드 인프라와 에셋은 남겨두고 `Lobby_Layout.json`에서 슬롯만 뺐다. 다시 켜려면 아래 슬롯 JSON을
`Data/UI/Lobby/Lobby_Layout.json`의 `TitleBackground` 슬롯 뒤에(같은 `slots` 배열 안, 마지막 항목으로)
다시 넣으면 된다.

## 구현 완료된 부분 (그대로 유지)
- `Client/Public/HUDRuntimeView.h` — `HUD_SLOT`에 `bAnimationLoop`(기본 true), `dAnimationStartSeconds`
  (슬롯이 처음 `Render()`되는 시점을 기준 시각으로 잡는 sentinel, 기본 -1.0) 필드 추가.
- `Client/Private/HUDRuntimeView.cpp` — `animation.loop` JSON 파싱, `Render()`에서 `loop:false`면
  마지막 프레임 재생 후 그 슬롯만 그리기를 멈추는 분기. 재생 기준 시각은 앱 전역 `ImGui::GetTime()`이
  아니라 슬롯이 실제로 처음 그려진 시점부터 잰다 (엔진/에셋 로딩 지연이 non-loop 재생 구간을 먹어버리는
  문제를 막기 위함 — 처음 구현 때 이 버그로 인트로가 아예 안 보이는 문제가 있었고, 이 수정으로 해결됨).
- `Client/Bin/Resources/UI/Lobby/LoginIntro/Login_i01.png` ~ `Login_i25.png` — `D:\로아\EFUI_LOBBY\login_i1~25_nopack.dds`
  (960x540, 25프레임)를 변환해서 이미 넣어둠. 삭제하지 않음.

## 재활성화 시 다시 넣을 슬롯 JSON

```json
    ,
    {
      "id": "IntroLogo",
      "ownerClass": null,
      "type": 0,
      "rect": { "x": 0, "y": 0, "width": 1280, "height": 720 },
      "rotation": 0,
      "stages": { "baseFrom": 0, "shineFrom": 1 },
      "layers": [],
      "shine": { "texture": null, "additive": false },
      "animation": {
        "fps": 12,
        "scale": 1,
        "offset": { "x": 0, "y": 0 },
        "loop": false,
        "frames": [
        "UI/Lobby/LoginIntro/Login_i01.png",
        "UI/Lobby/LoginIntro/Login_i02.png",
        "UI/Lobby/LoginIntro/Login_i03.png",
        "UI/Lobby/LoginIntro/Login_i04.png",
        "UI/Lobby/LoginIntro/Login_i05.png",
        "UI/Lobby/LoginIntro/Login_i06.png",
        "UI/Lobby/LoginIntro/Login_i07.png",
        "UI/Lobby/LoginIntro/Login_i08.png",
        "UI/Lobby/LoginIntro/Login_i09.png",
        "UI/Lobby/LoginIntro/Login_i10.png",
        "UI/Lobby/LoginIntro/Login_i11.png",
        "UI/Lobby/LoginIntro/Login_i12.png",
        "UI/Lobby/LoginIntro/Login_i13.png",
        "UI/Lobby/LoginIntro/Login_i14.png",
        "UI/Lobby/LoginIntro/Login_i15.png",
        "UI/Lobby/LoginIntro/Login_i16.png",
        "UI/Lobby/LoginIntro/Login_i17.png",
        "UI/Lobby/LoginIntro/Login_i18.png",
        "UI/Lobby/LoginIntro/Login_i19.png",
        "UI/Lobby/LoginIntro/Login_i20.png",
        "UI/Lobby/LoginIntro/Login_i21.png",
        "UI/Lobby/LoginIntro/Login_i22.png",
        "UI/Lobby/LoginIntro/Login_i23.png",
        "UI/Lobby/LoginIntro/Login_i24.png",
        "UI/Lobby/LoginIntro/Login_i25.png"
        ]
      }
    }
```

## 남은 미해결 항목 (보류 전 상태)
- 첫 빌드에서는 재생이 아예 안 보이는 버그가 있었음(위 시각 기준 문제) → 코드 수정 완료했으나, 수정 후
  버전으로 재생 자체가 매끄러운지는 사용자가 아직 재확인 전에 보류 결정함. 다시 켤 때 실제 재생 결과를
  한 번 더 확인할 것.
- `login_i26_nopack.dds` (660x216, 다른 비율)는 시퀀스에 포함하지 않음 — 애니메이션 끝난 뒤 별도로 잠깐
  보여주는 정지 카드(로고/타이틀 각인)일 가능성이 있었으나 스코프에 넣지 않았음.
