#include "roundtrip.hpp"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <utility>

namespace asm64 {

RoundtripReport run_roundtrip(const uint8_t* data, size_t size,
                              uint64_t base_addr,
                              Disassembler& dis, Builder& builder) {
    RoundtripReport r;
    auto insns = dis.disassemble(data, size, base_addr);
    for (auto& ins : insns) {
        if (ins.bytes.size() != 4) continue;
        RoundtripCase c;
        c.address  = ins.address;
        c.mnemonic = ins.mnemonic;
        c.operands = ins.operands;
        c.original = (uint32_t)ins.bytes[0]
                   | ((uint32_t)ins.bytes[1] << 8)
                   | ((uint32_t)ins.bytes[2] << 16)
                   | ((uint32_t)ins.bytes[3] << 24);
        c.built = builder.build(c.mnemonic, c.operands, c.address,
                                 c.produced, c.error);
        c.ok    = c.built && (c.produced == c.original);

        // Esdegerlik: farkli byte, ayni anlam
        if (c.built && !c.ok) {
            uint8_t alt[4] = {
                (uint8_t)(c.produced & 0xFF),
                (uint8_t)((c.produced >> 8) & 0xFF),
                (uint8_t)((c.produced >> 16) & 0xFF),
                (uint8_t)((c.produced >> 24) & 0xFF)
            };
            auto alt_dis = dis.disassemble(alt, 4, c.address);
            if (!alt_dis.empty()
                && alt_dis[0].mnemonic == c.mnemonic
                && alt_dis[0].operands == c.operands) {
                c.ok = true;
                c.equivalence = true;
            }
        }
        r.total++;
        if (c.ok) {
            r.ok++;
        } else if (!c.built) {
            r.unsupported++;
            r.unsupported_by_mnemonic[c.mnemonic]++;
            if (r.unsupported_examples.size() < 100)
                r.unsupported_examples.push_back(c);
        } else {
            r.mismatch++;
            r.failures.push_back(c);
        }
    }
    return r;
}

void print_roundtrip_report(const RoundtripReport& r) {
    std::cout << "\n===== Roundtrip Raporu (" << r.total << " komut) =====\n";
    std::cout << "OK          : " << r.ok          << "\n";
    std::cout << "FARK        : " << r.mismatch    << "\n";
    std::cout << "DESTEKLENMEZ: " << r.unsupported << "  (OpcodeByte tanimadi)\n";

    if (!r.failures.empty()) {
        std::cout << "\n--- Ilk 30 fark ---\n";
        size_t n = std::min<size_t>(r.failures.size(), 30);
        for (size_t i = 0; i < n; ++i) {
            auto& c = r.failures[i];
            std::cout << "  0x" << std::hex << std::setw(8) << std::setfill('0') << c.address
                      << std::dec << ": " << c.mnemonic;
            if (!c.operands.empty()) std::cout << " " << c.operands;
            std::cout << "  beklendi=0x" << std::hex << std::setw(8)
                      << std::setfill('0') << c.original
                      << "  bulundu=0x" << std::setw(8) << std::setfill('0') << c.produced
                      << std::dec << "\n";
        }
        if (r.failures.size() > n)
            std::cout << "  ... ve " << (r.failures.size() - n) << " fark daha\n";
    }

    if (!r.unsupported_examples.empty()) {
        std::cout << "\n--- Desteklenmeyen ilk 30 ornek ---\n";
        size_t ne = std::min<size_t>(r.unsupported_examples.size(), 30);
        for (size_t i = 0; i < ne; ++i) {
            auto& c = r.unsupported_examples[i];
            std::cout << "  0x" << std::hex << std::setw(8) << std::setfill('0') << c.address
                      << std::dec << ": " << c.mnemonic;
            if (!c.operands.empty()) std::cout << " " << c.operands;
            std::cout << "\n";
        }
    }

    if (!r.unsupported_by_mnemonic.empty()) {
        std::vector<std::pair<std::string, size_t>> sorted(
            r.unsupported_by_mnemonic.begin(), r.unsupported_by_mnemonic.end());
        std::sort(sorted.begin(), sorted.end(),
                  [](const auto& a, const auto& b){ return a.second > b.second; });

        std::cout << "\n--- En sik desteklenmeyen 20 komut ---\n";
        size_t n = std::min<size_t>(sorted.size(), 20);
        for (size_t i = 0; i < n; ++i) {
            std::cout << "  " << std::setw(7) << sorted[i].second
                      << "  " << sorted[i].first << "\n";
        }
        if (sorted.size() > n)
            std::cout << "  ... ve " << (sorted.size() - n) << " farkli komut daha\n";
    }
}

} // namespace asm64
