#pragma once

// 안정적인 표준 라이브러리 헤더만 캐시한다. 게임·저작 헤더는 각 소스가 직접 포함한다.
// 기능별 헤더 변경이 모든 프로젝트의 PCH를 무효화하지 않게 경계를 유지한다.
#include <algorithm>
#include <array>
#include <cstdint>
#include <ctime>
#include <filesystem>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>
