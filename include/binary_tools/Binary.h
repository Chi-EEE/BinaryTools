#pragma once

#include <string>
#include "Span.h"

namespace binary_tools
{
    Span<char> ReadAllBytes(const std::string &filePath);
}
