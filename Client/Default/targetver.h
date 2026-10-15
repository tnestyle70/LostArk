// Windows SDK가 어떤 버전의 API 선언을 노출할지 정하는 컴파일 정책의 입구다.
// 헤더를 읽는 행위가 Windows를 설치·업데이트하거나 실행 중 OS 버전을 검사하지는 않는다.
#pragma once

// // SDKDDKVer.h를 포함하면 최고 수준의 가용성을 가진 Windows 플랫폼이 정의됩니다.
// 이전 Windows 플랫폼용 애플리케이션을 빌드하려는 경우에는 SDKDDKVer.h를 포함하기 전에
// WinSDKVer.h를 포함하고 _WIN32_WINNT를 지원하려는 플랫폼으로 설정합니다.
// SDKDDKVer.h는 설치된 SDK의 Windows 버전 상수와 미지정 버전 매크로의 기본값을 제공한다.
// 현재 이 파일은 _WIN32_WINNT를 직접 지정하지 않아 앞서 주어진 빌드 정의와 SDK 기본값을 따른다.
// 아래 include만으로 모든 구형 Windows에서 실행 가능하다는 보장은 없다. 사용하는 API와 배포 조건도 맞아야 한다.
// 하한 버전을 지정할 때는 Windows 헤더를 처음 읽기 전에 정책을 정해야 한다.
// 따라서 이 파일만 수정하면 충분하다고 가정하지 않고 /FI 및 실제 include 순서를 함께 확인한다.
#include <SDKDDKVer.h>
