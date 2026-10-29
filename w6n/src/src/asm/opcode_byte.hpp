#pragma once
#include <cstdint>
#include <string>

namespace opcodebyte {
enum class Mode { A, B };

struct Result {
    bool        ok = false;
    uint32_t    value = 0;
    std::string error;
};

Result encode(const std::string& mnemonic,
              const std::string& operands,
              uint64_t address,
              Mode mode = Mode::A);
}
