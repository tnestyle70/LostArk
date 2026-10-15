#pragma once

namespace Client
{
    // wWinMain이 엔진 생성 전에 한 번 호출한다. 로그 준비 실패는 게임 시작을 막지 않는다.
    void PrepareCrashReport() noexcept;
}
