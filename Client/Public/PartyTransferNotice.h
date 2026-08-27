#pragma once

#include "Network/PacketMessages.h"

namespace Client
{

inline const wchar_t* Get_PartyTransferFailureText(
	LostArk::Shared::PARTY_TRANSFER_RESULT result)
{
	using LostArk::Shared::PARTY_TRANSFER_RESULT;
	switch (result)
	{
	case PARTY_TRANSFER_RESULT::REJECTED_NOT_LEADER:
		return L"\ud30c\ud2f0\uc7a5\ub9cc \uc785\uc7a5\uc744 \uc694\uccad\ud560 \uc218 \uc788\uc2b5\ub2c8\ub2e4. \ud604\uc7ac \uc9c0\uc5ed\uc5d0 \uba38\ubb34\ub985\ub2c8\ub2e4.";
	case PARTY_TRANSFER_RESULT::REJECTED_ROOM_FULL:
		return L"\uc785\uc7a5 \uc9c0\uc5ed\uc758 \uc778\uc6d0\uc774 \uac00\ub4dd \ucc3c\uc2b5\ub2c8\ub2e4. \ud30c\ud2f0\ub294 \ud604\uc7ac \uc9c0\uc5ed\uc5d0 \uba38\ubb34\ub985\ub2c8\ub2e4.";
	case PARTY_TRANSFER_RESULT::REJECTED_MEMBER_UNAVAILABLE:
		return L"\uc785\uc7a5\ud560 \uc218 \uc5c6\ub294 \ud30c\ud2f0\uc6d0\uc774 \uc788\uc2b5\ub2c8\ub2e4. \ud30c\ud2f0 \uc0c1\ud0dc\ub97c \ud655\uc778\ud574 \uc8fc\uc138\uc694.";
	case PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED:
		return L"\uc11c\ubc84\uac00 \uc785\uc7a5\uc744 \uc2b9\uc778\ud558\uc9c0 \ubabb\ud588\uc2b5\ub2c8\ub2e4. \ud30c\ud2f0\ub294 \ud604\uc7ac \uc9c0\uc5ed\uc5d0 \uba38\ubb34\ub985\ub2c8\ub2e4.";
	case PARTY_TRANSFER_RESULT::REJECTED_OUTBOUND_BUSY:
		return L"\uc785\uc7a5 \uc751\ub2f5\uc744 \uc900\ube44\ud558\uc9c0 \ubabb\ud588\uc2b5\ub2c8\ub2e4. \uc7a0\uc2dc \ud6c4 \ub2e4\uc2dc \uc2dc\ub3c4\ud574 \uc8fc\uc138\uc694.";
	default:
		return nullptr;
	}
}

}
