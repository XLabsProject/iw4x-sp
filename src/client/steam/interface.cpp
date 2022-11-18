#include <std_include.hpp>
#include "interface.hpp"
#include <utils/memory.hpp>
#include <utils/nt.hpp>

namespace steam {
interface::interface() : interface(nullptr) {}

interface::interface(void* interface_ptr)
    : interface_ptr_(static_cast<void***>(interface_ptr)) {}

interface::operator bool() const { return this->interface_ptr_ != nullptr; }

interface::method interface::find_method(const std::string& name) {
  const auto method_entry = this->methods_.find(name);
  if (method_entry != this->methods_.end()) {
    return method_entry->second;
  }

  return this->search_method(name);
}

interface::method interface::search_method(const std::string& name) {
  if (!utils::memory::is_bad_read_ptr(this->interface_ptr_)) {
    auto vftbl = *this->interface_ptr_;

    while (!utils::memory::is_bad_read_ptr(vftbl) &&
           !utils::memory::is_bad_code_ptr(*vftbl)) {
      const auto result = this->analyze_method(*vftbl);
      if (result.param_size_found && result.name_found) {
        const method method_result{*vftbl, result.param_size};
        this->methods_[result.name] = method_result;

        if (result.name == name) {
          return method_result;
        }
      }

      ++vftbl;
    }
  }

  return {};
}

interface::method_result interface::analyze_method(const void* method_ptr) {
  method_result result;
  if (utils::memory::is_bad_code_ptr(method_ptr))
    return result;

  ud_t ud;
  ud_init(&ud);
  ud_set_mode(&ud, 32);
  ud_set_pc(&ud, reinterpret_cast<std::uint64_t>(method_ptr));
  ud_set_input_buffer(&ud, static_cast<const std::uint8_t*>(method_ptr),
                      INT32_MAX);

  while (true) {
    ud_disassemble(&ud);

    if (ud_insn_mnemonic(&ud) == UD_Iret && !result.param_size_found) {
      const ud_operand* operand = ud_insn_opr(&ud, 0);
      if (operand && operand->type == UD_OP_IMM && operand->size == 16) {
        result.param_size = operand->lval.uword;
      } else {
        result.param_size = 0;
      }

      result.param_size_found = true;
    }

    if (ud_insn_mnemonic(&ud) == UD_Ipush && !result.name_found) {
      const auto operand = ud_insn_opr(&ud, 0);
      if (operand->type == UD_OP_IMM && operand->size == 32) {
        auto* operand_ptr = reinterpret_cast<char*>(operand->lval.udword);
        if (!utils::memory::is_bad_read_ptr(operand_ptr) &&
            this->is_rdata(operand_ptr)) {
          result.name = operand_ptr;
          result.name_found = true;
        }
      }
    }

    if (*reinterpret_cast<unsigned char*>(ud.pc) == 0xCC)
      break; // int 3
    if (result.param_size_found && result.name_found)
      break;
  }

  return result;
}

bool interface::is_rdata(void* pointer) {
  const auto pointer_lib = utils::nt::library::get_by_address(pointer);

  for (const auto& section : pointer_lib.get_section_headers()) {
    const auto size = sizeof(section->Name);
    char name[size + 1];
    name[size] = 0;
    std::memcpy(name, section->Name, size);

    if (name == ".rdata"s) {
      const auto target = reinterpret_cast<std::size_t>(pointer);
      const size_t source_start =
          size_t(pointer_lib.get_ptr()) + section->PointerToRawData;
      const size_t source_end = source_start + section->SizeOfRawData;

      return target >= source_start && target <= source_end;
    }
  }

  return false;
}
} // namespace steam
