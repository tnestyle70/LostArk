#pragma once

namespace LostArk::Server
{
	// Server의 명시적 계약 검사 진입점이다. 파괴 bootstrap의 읽기·검증 결과를 확인하고 검사 종료 코드를 반환한다.
	int Run_WorldDestructionBootstrapContractTests();
}
