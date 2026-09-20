#pragma once
#include "Engine_Defines.h"
#include "Sound/Sound_Manager.h"
namespace Engine { class CGameInstance { public: static CGameInstance& Get() { static CGameInstance instance; return instance; } void Apply_SoundCategoryVolume(SOUND_CATEGORY,float) {} void Set_SoundMuteOnFocusLoss(bool) {} }; }
