#pragma once
#include "../asm/builder.hpp"
#include "../disasm/engine.hpp"
#include <cstdint>
#include <string>
#include <vector>
#include <map>

namespace asm64 {
struct RoundtripCase {
    uint64_t address;
    std::string mnemonic;
    std::string operands;
    uint32_t original;
    uint32_t produced;
    bool     ok;
    bool     built;
    bool     equivalence = false;
    std::string error;
};
struct RoundtripReport {
    size_t total    = 0;
    size_t ok       = 0;
    size_t mismatch = 0;
    size_t unsupported = 0;
    std::vector<RoundtripCase> failures;
    std::vector<RoundtripCase> unsupported_examples;
    std::map<std::string, size_t> unsupported_by_mnemonic;
};
RoundtripReport run_roundtrip(const uint8_t* data, size_t size,
                              uint64_t base_addr,
                              Disassembler& dis, Builder& builder);
void print_roundtrip_report(const RoundtripReport& r);
}
