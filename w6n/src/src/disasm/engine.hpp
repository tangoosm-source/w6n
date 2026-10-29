#pragma once
#include <capstone/capstone.h>
#include <cstdint>
#include <string>
#include <vector>

namespace asm64 {
struct Instruction {
    uint64_t address;
    std::vector<uint8_t> bytes;
    std::string mnemonic;
    std::string operands;
    std::string full_text() const;
};

class Disassembler {
public:
    Disassembler();
    ~Disassembler();
    Disassembler(const Disassembler&) = delete;
    Disassembler& operator=(const Disassembler&) = delete;

    std::vector<Instruction> disassemble(const uint8_t* data, size_t size,
                                         uint64_t base_addr, size_t count = 0);
    bool disassemble_one(const uint8_t* data, size_t size,
                         uint64_t addr, Instruction& out);
private:
    csh handle_ = 0;
};
}
