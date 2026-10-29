#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace asm64 {
struct Section {
    std::string name;
    uint64_t addr = 0, offset = 0, size = 0;
    uint32_t type = 0;
    uint64_t flags = 0;
};
struct Symbol {
    std::string name;
    uint64_t addr = 0, size = 0;
    uint8_t type = 0;
};
class ElfLoader {
public:
    bool load(const std::string& path);
    const std::vector<uint8_t>& data() const { return data_; }
    const std::vector<Section>& sections() const { return sections_; }
    const std::vector<Symbol>& symbols() const { return symbols_; }
    uint64_t entry_point() const { return entry_; }
    uint16_t machine() const { return machine_; }
    const Section* find_section(const std::string& name) const;
private:
    std::vector<uint8_t> data_;
    std::vector<Section> sections_;
    std::vector<Symbol> symbols_;
    uint64_t entry_ = 0;
    uint16_t machine_ = 0;
};
}
