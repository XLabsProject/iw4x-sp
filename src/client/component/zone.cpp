#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>

namespace zone {
class component final : public component_interface {
public:
  static_assert(sizeof(game::XZoneInfo) == 0xC);

  void post_load() override { patch_sp(); }

private:
  static void patch_sp() {
    // Ignore zone version mismatch
    utils::hook::set<std::uint8_t>(0x4256D8, 0xEB);

    // Disc read error
    utils::hook::nop(0x4B7335, 2);
    utils::hook::set<std::uint8_t>(0x4256B9, 0xEB);
  }
};
} // namespace zone

REGISTER_COMPONENT(zone::component)
