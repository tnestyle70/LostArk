#pragma once

//Client 소유 namespace를 여는 공용 매크로다.
#include "Engine_Macro.h"
//설치 재질의 고정 폭 program ID를 비교한다.
#include <cstdint>
NS_BEGIN(Client::SourceEquipmentMaterial)
//Part_Body·Part_Equipment·WorldSequenceObject가 반투명 렌더 pass를 선택할 때 사용한다.
inline bool Is_Translucent(uint32_t program)
{
    return program == 160u || program == 166u || program == 168u || program == 169u || program == 170u || program == 171u || program == 172u || program == 174u || program == 182u || program == 187u || program == 190u || program == 192u || program == 195u || program == 212u || program == 213u || program == 600u || program == 601u ||
        program == 700u || program == 701u || program == 702u || program == 801u || program == 903u;
}

//장비의 masked 프로그램 중 양면 처리가 필요한 원본 재질을 구분한다.
inline bool Is_TwoSidedMasked(uint32_t program)
{
    return program == 161u || program == 167u || program == 179u;
}
NS_END
