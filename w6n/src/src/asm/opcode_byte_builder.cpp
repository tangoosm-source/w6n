#include "builder.hpp"

#if __has_include("opcode_byte.hpp")
#  include "opcode_byte.hpp"
#  define HAS_OPCODEBYTE 1
#else
#  define HAS_OPCODEBYTE 0
#endif

namespace asm64 {

class OpcodeByteBuilder : public Builder {
public:
    bool build(const std::string& mnemonic,
               const std::string& operands,
               uint64_t address,
               uint32_t& out_opcode,
               std::string& err) override
    {
#if HAS_OPCODEBYTE
        auto r = opcodebyte::encode(mnemonic, operands, address,
                                     opcodebyte::Mode::A);
        if (!r.ok) { err = r.error; return false; }
        out_opcode = r.value;
        return true;
#else
        (void)mnemonic; (void)operands; (void)address;
        (void)out_opcode;
        err = "OpcodeByte kutuphanesi baglanmadi";
        return false;
#endif
    }
    const char* name() const override { return "OpcodeByte"; }
};

Builder* make_opcode_byte_builder() { return new OpcodeByteBuilder(); }

} // namespace asm64
