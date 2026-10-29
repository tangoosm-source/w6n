#include "engine.hpp"
#include <stdexcept>
#include <sstream>
#include <iomanip>

namespace asm64 {
std::string Instruction::full_text() const {
    std::ostringstream ss;
    ss << "0x" << std::hex << std::setw(8) << std::setfill('0') << address << ": ";
    for (auto b : bytes) ss << std::setw(2) << (int)b << ' ';
    ss << "\t" << mnemonic;
    if (!operands.empty()) ss << " " << operands;
    return ss.str();
}
Disassembler::Disassembler() {
    if (cs_open(CS_ARCH_ARM64, CS_MODE_ARM, &handle_) != CS_ERR_OK)
        throw std::runtime_error("cs_open(ARM64) failed");
}
Disassembler::~Disassembler() { if (handle_) cs_close(&handle_); }
std::vector<Instruction> Disassembler::disassemble(const uint8_t* data, size_t size,
                                                    uint64_t base_addr, size_t count) {
    std::vector<Instruction> result;
    cs_insn* insn = nullptr;
    size_t n = cs_disasm(handle_, data, size, base_addr, count, &insn);
    if (n == 0) return result;
    result.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        Instruction inst;
        inst.address = insn[i].address;
        inst.bytes.assign(insn[i].bytes, insn[i].bytes + insn[i].size);
        inst.mnemonic = insn[i].mnemonic;
        inst.operands = insn[i].op_str;
        result.push_back(std::move(inst));
    }
    cs_free(insn, n);
    return result;
}
bool Disassembler::disassemble_one(const uint8_t* data, size_t size,
                                    uint64_t addr, Instruction& out) {
    auto vec = disassemble(data, size, addr, 1);
    if (vec.empty()) return false;
    out = std::move(vec[0]);
    return true;
}
}
