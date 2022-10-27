#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>

namespace ceg {
class component final : public component_interface {
public:
  void post_load() override {
    // Some unnecessary ceg functions.
    // Important functions are patched in the exe already

    utils::hook::set<std::uint32_t>(0x44AD80, 0xC301B0);
    utils::hook::set<std::uint32_t>(0x476A20, 0xC301B0);
    utils::hook::set<std::uint32_t>(0x4E3B90, 0xC301B0);
    utils::hook::set<std::uint32_t>(0x4E3B90, 0xC301B0);
    utils::hook::set<std::uint32_t>(0x411160, 0xC301B0);
    utils::hook::set<std::uint32_t>(0x41E390, 0xC301B0);
    utils::hook::set<std::uint32_t>(0x47C2E0, 0xC301B0);
    utils::hook::set<std::uint32_t>(0x4EEA90, 0xC301B0);

    // Killer caller
    utils::hook::set<std::uint8_t>(0x43F320, 0xC3);
    utils::hook::set<std::uint8_t>(0x458ED0, 0xC3);
    utils::hook::set<std::uint8_t>(0x47A140, 0xC3);

    utils::hook::nop(0x411166, 9);
  }
};
} // namespace ceg

REGISTER_COMPONENT(ceg::component)
