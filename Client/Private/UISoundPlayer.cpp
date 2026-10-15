#include "UISoundPlayer.h"

#include "RuntimeAssetRoot.h"
#include "GameInstance.h"

#include <array>
#include <filesystem>
#include <string>

NS_BEGIN(Client)

namespace
{
	//고정 UI cue를 기존 Resources 상대 경로에 연결한다.
	//literal을 차용하며 파일을 적재하지 않는다.
	//이 구조를 보니까, 이런 식으로 전체적인 사운드 자체를 관리하는 거는 어때?
	//이거 사운드 전체가 어디에서 관리가 되고 있는 거지?
	//장치·캐시·채널은 Engine Sound_Manager가 관리하고, 음원 선택은 각 Client 기능이 맡는다.
	//이 파일은 공통 UI 네 cue만 선택하며 캐릭터·보스·Sequence의 기존 선택 경로는 유지한다.
    const wchar_t* Resolve_UISoundAssetId(UI_SOUND_CUE eCue)
    {
		switch (eCue)
        {
        case UI_SOUND_CUE::BUTTON_CLICK:
            return L"Sound/UI/Select/ui_default_button_click2__59426200.wav";
        case UI_SOUND_CUE::WINDOW_OPEN:
            return L"Sound/UI/Select/ui_inventory_show1__669750910.wav";
        case UI_SOUND_CUE::WINDOW_CLOSE:
            return L"Sound/UI/Select/ui_inventory_hide1__7273537.wav";
        case UI_SOUND_CUE::POPUP_REQUEST:
            return L"Sound/UI/System/sys_party_request1__654410772.wav";
        default:
            return nullptr;
        }
	}
}

//mainapp에서 해당 API를 호출해서 매개변수로 sound cue를 넘겨주고, 해당하는 sound cue를
//Resolve_UISoundAssetId의 매개변수로 넘겨줘서, 해당 cue의 enum에 맞는 sound 경로를 반환한다.
void Play_UISound(UI_SOUND_CUE eCue)
{
    const wchar_t* const pAssetId = Resolve_UISoundAssetId(eCue);

    if (nullptr == pAssetId)
    {
        return;
    }

    //Resources 배치를 해석하고, 경로 해석이 빈 결과를 반환한 요청은 건너뛴다.
    const std::filesystem::path SoundPath = CRuntimeAssetRoot::Resolve(pAssetId);

    if (SoundPath.empty())
    {
        return;
    }

    //기존 1.f와 one-shot 재생을 유지. 반환된 성공, 실패는 UI 동작의 성공 조건으로 쓰지 않는다.
    (void)Engine::CGameInstance::Get().Play_Sound(
        SoundPath.wstring(), 1.f, Engine::SOUND_CATEGORY::INTERFACE);
}
//기존 4개 매핑을 사용해 하나씩 cache의 준비를 요청한다. 개별 경로/적재 실패는 기록하고,
//다음 cue를 진행한다.
void Prepare_UISounds()
{
    const std::array<UI_SOUND_CUE, 4> cues = 
    {
        UI_SOUND_CUE::BUTTON_CLICK,
        UI_SOUND_CUE::WINDOW_OPEN,
        UI_SOUND_CUE::WINDOW_CLOSE,
        UI_SOUND_CUE::POPUP_REQUEST
    };

    for (const UI_SOUND_CUE cue : cues)
    {
        //resolve sound ui asset 함수를 호출해서, 실제 sound 물리 폴더 경로를 받아온다.
        //이 함수는 Resources 상대 asset ID를 반환하고, 아래 Resolve가 실제 물리 경로를 해석한다.
        const wchar_t* pAssetId = Resolve_UISoundAssetId(cue);
        
        if (nullptr == pAssetId)
        {
            OutputDebugStringW(L"[UISound] Prepare failed : invalid UI cue. \n");
            continue;
        }

        try
        {
            const std::filesystem::path soundPath = CRuntimeAssetRoot::Resolve(pAssetId);
            
            if (!soundPath.empty() &&
                SUCCEEDED(Engine::CGameInstance::Get().Prepare_Sound(soundPath.wstring(), false)))
            {
                continue;
            }
        }
        catch (const std::filesystem::filesystem_error& error)
        {
            OutputDebugStringA(error.what());
            OutputDebugStringA("\n");
        }

        const std::wstring message = std::wstring(L"[UISound] Prepare failed: ") + pAssetId + L"\n";
        OutputDebugStringW(message.c_str());
    }
}

NS_END
