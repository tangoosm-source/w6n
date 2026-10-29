#pragma once
#include <cstdint>
#include <string>

namespace asm64 {
class Builder {
public:
    virtual ~Builder() = default;
    virtual bool build(const std::string& mnemonic,
                       const std::string& operands,
                       uint64_t address,
                       uint32_t& out_opcode,
                       std::string& err) = 0;
    virtual const char* name() const = 0;
};
Builder* make_opcode_byte_builder();
}
