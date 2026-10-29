#include "elf_loader.hpp"
#include <fstream>
#include <cstring>
#include <elf.h>

namespace asm64 {
bool ElfLoader::load(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;
    f.seekg(0, std::ios::end);
    size_t size = (size_t)f.tellg();
    f.seekg(0);
    data_.resize(size);
    f.read(reinterpret_cast<char*>(data_.data()), (std::streamsize)size);

    if (size < sizeof(Elf64_Ehdr)) return false;
    auto* eh = reinterpret_cast<const Elf64_Ehdr*>(data_.data());
    if (std::memcmp(eh->e_ident, ELFMAG, SELFMAG) != 0) return false;
    if (eh->e_ident[EI_CLASS] != ELFCLASS64) return false;
    machine_ = eh->e_machine;
    entry_ = eh->e_entry;

    auto* sh = reinterpret_cast<const Elf64_Shdr*>(data_.data() + eh->e_shoff);
    auto* shstr = reinterpret_cast<const Elf64_Shdr*>(
        data_.data() + eh->e_shoff + (size_t)eh->e_shstrndx * eh->e_shentsize);
    const char* strtab = reinterpret_cast<const char*>(data_.data() + shstr->sh_offset);

    for (int i = 0; i < eh->e_shnum; ++i) {
        Section s;
        s.name = strtab + sh[i].sh_name;
        s.addr = sh[i].sh_addr;
        s.offset = sh[i].sh_offset;
        s.size = sh[i].sh_size;
        s.type = sh[i].sh_type;
        s.flags = sh[i].sh_flags;
        sections_.push_back(std::move(s));
    }
    for (int i = 0; i < eh->e_shnum; ++i) {
        if (sh[i].sh_type != SHT_SYMTAB) continue;
        auto* syms = reinterpret_cast<const Elf64_Sym*>(data_.data() + sh[i].sh_offset);
        size_t count = sh[i].sh_size / sizeof(Elf64_Sym);
        const char* sym_strtab = reinterpret_cast<const char*>(
            data_.data() + sh[sh[i].sh_link].sh_offset);
        for (size_t k = 0; k < count; ++k) {
            if (syms[k].st_name == 0) continue;
            Symbol sym;
            sym.name = sym_strtab + syms[k].st_name;
            sym.addr = syms[k].st_value;
            sym.size = syms[k].st_size;
            sym.type = ELF64_ST_TYPE(syms[k].st_info);
            symbols_.push_back(std::move(sym));
        }
    }
    return true;
}
const Section* ElfLoader::find_section(const std::string& name) const {
    for (const auto& s : sections_) if (s.name == name) return &s;
    return nullptr;
}
}
