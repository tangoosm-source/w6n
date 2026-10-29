#include "disasm/engine.hpp"
#include "loader/elf_loader.hpp"
#include "asm/builder.hpp"
#include "analysis/roundtrip.hpp"
#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>
#include <elf.h>

using namespace asm64;

static std::vector<uint8_t> read_raw(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    return std::vector<uint8_t>((std::istreambuf_iterator<char>(f)),
                                 std::istreambuf_iterator<char>());
}

static size_t vaddr_to_offset(const ElfLoader& elf, uint64_t vaddr) {
    for (auto& s : elf.sections()) {
        if (s.type == SHT_NOBITS) continue;
        if (vaddr >= s.addr && vaddr < s.addr + s.size)
            return (size_t)(s.offset + (vaddr - s.addr));
    }
    return (size_t)-1;
}

static bool is_code_symbol(const Symbol& s, const Section* text) {
    if (s.name.empty()) return false;
    if (s.name[0] == '$' || s.name[0] == '.') return false;
    if (s.addr == 0) return false;
    if (!text) return true;
    return s.addr >= text->addr && s.addr < text->addr + text->size;
}

static void print_usage() {
    std::cout <<
        "asmanalyzer - ARM64 analiz araci\n"
        "Kullanim:\n"
        "  asmanalyzer raw          <file.bin> [base_hex] [count]\n"
        "  asmanalyzer elf          <file.elf> [--sections] [--functions]\n"
        "                                       [--disasm <sym>] [--disasm-all]\n"
        "  asmanalyzer bytes        <hex1> <hex2> ...\n"
        "  asmanalyzer roundtrip    <file.bin> [base_hex]\n"
        "  asmanalyzer elf-roundtrip <file.elf> [symbol]\n";
}

int main(int argc, char** argv) {
    if (argc < 3) { print_usage(); return 1; }
    std::string mode = argv[1];
    std::string path = argv[2];
    Disassembler dis;

    if (mode == "raw") {
        auto data = read_raw(path);
        if (data.empty()) { std::cerr << "Dosya bos veya yok\n"; return 1; }
        uint64_t base  = argc > 3 ? std::stoull(argv[3], nullptr, 16) : 0x1000;
        size_t   count = argc > 4 ? (size_t)std::stoull(argv[4]) : 0;
        for (auto& i : dis.disassemble(data.data(), data.size(), base, count))
            std::cout << i.full_text() << "\n";
        return 0;
    }

    if (mode == "elf") {
        ElfLoader elf;
        if (!elf.load(path)) { std::cerr << "ELF yuklenemedi\n"; return 1; }
        std::cout << "Entry   : 0x" << std::hex << elf.entry_point() << "\n";
        std::cout << "Machine : " << std::dec << elf.machine() << " (183=AArch64)\n";
        auto* text = elf.find_section(".text");
        auto& d = elf.data();
        for (int i = 3; i < argc; ++i) {
            std::string opt = argv[i];
            if (opt == "--sections") {
                for (auto& s : elf.sections())
                    std::cout << "  [0x" << std::hex << s.addr << std::dec
                              << "] " << s.name << "  size=0x" << std::hex << s.size
                              << std::dec << "\n";
            } else if (opt == "--functions") {
                for (auto& s : elf.symbols()) {
                    if (!is_code_symbol(s, text)) continue;
                    if (s.type != STT_FUNC && s.type != STT_NOTYPE) continue;
                    std::cout << "  0x" << std::hex << s.addr << std::dec
                              << " " << s.name << " (size=" << s.size << ")\n";
                }
            } else if (opt == "--disasm" && i + 1 < argc) {
                std::string sym = argv[++i];
                for (auto& s : elf.symbols()) {
                    if (s.name != sym) continue;
                    if (s.type != STT_FUNC && s.type != STT_NOTYPE) continue;
                    size_t off = vaddr_to_offset(elf, s.addr);
                    if (off == (size_t)-1) {
                        std::cerr << "Sembol icin section bulunamadi: " << sym << "\n";
                        break;
                    }
                    size_t sz = s.size;
                    if (sz == 0 && text && s.addr >= text->addr)
                        sz = (size_t)(text->size - (s.addr - text->addr));
                    for (auto& in : dis.disassemble(d.data() + off, sz, s.addr))
                        std::cout << in.full_text() << "\n";
                    break;
                }
            } else if (opt == "--disasm-all") {
                if (text)
                    for (auto& in : dis.disassemble(
                             d.data() + text->offset, text->size, text->addr))
                        std::cout << in.full_text() << "\n";
            }
        }
        return 0;
    }

    if (mode == "elf-roundtrip") {
        ElfLoader elf;
        if (!elf.load(path)) { std::cerr << "ELF yuklenemedi\n"; return 1; }
        auto* text = elf.find_section(".text");
        if (!text) { std::cerr << ".text section bulunamadi\n"; return 1; }
        auto& d = elf.data();

        const uint8_t* data_ptr = nullptr;
        size_t   data_size = 0;
        uint64_t base      = 0;

        if (argc > 3) {
            std::string sym = argv[3];
            bool found = false;
            for (auto& s : elf.symbols()) {
                if (s.name != sym) continue;
                if (s.type != STT_FUNC && s.type != STT_NOTYPE) continue;
                size_t off = vaddr_to_offset(elf, s.addr);
                if (off == (size_t)-1) {
                    std::cerr << "Sembol icin section bulunamadi: " << sym << "\n";
                    return 1;
                }
                size_t sz = s.size;
                if (sz == 0 && s.addr >= text->addr)
                    sz = (size_t)(text->size - (s.addr - text->addr));
                data_ptr  = d.data() + off;
                data_size = sz;
                base      = s.addr;
                found     = true;
                break;
            }
            if (!found) {
                std::cerr << "Sembol bulunamadi: " << sym << "\n";
                return 1;
            }
        } else {
            data_ptr  = d.data() + text->offset;
            data_size = text->size;
            base      = text->addr;
        }

        Builder* b = make_opcode_byte_builder();
        auto rep = run_roundtrip(data_ptr, data_size, base, dis, *b);
        print_roundtrip_report(rep);
        delete b;
        return rep.mismatch == 0 ? 0 : 2;
    }

    if (mode == "roundtrip") {
        auto data = read_raw(path);
        if (data.empty()) { std::cerr << "Dosya bos veya yok\n"; return 1; }
        uint64_t base = argc > 3 ? std::stoull(argv[3], nullptr, 16) : 0x0;
        Builder* b = make_opcode_byte_builder();
        auto rep = run_roundtrip(data.data(), data.size(), base, dis, *b);
        print_roundtrip_report(rep);
        delete b;
        return rep.mismatch == 0 ? 0 : 2;
    }

    if (mode == "bytes") {
        std::vector<uint8_t> bytes;
        for (int i = 2; i < argc; ++i)
            bytes.push_back((uint8_t)std::stoul(argv[i], nullptr, 16));
        for (auto& in : dis.disassemble(bytes.data(), bytes.size(), 0, 0))
            std::cout << in.full_text() << "\n";
        return 0;
    }

    print_usage();
    return 1;
}
