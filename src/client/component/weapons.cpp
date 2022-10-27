#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>

namespace weapons {
namespace {
game::WeaponCompleteDef*
bg_load_weapon_variant_def_fast_file_stub(const char* name) {
  if (auto* raw_weapon_file =
          game::BG_LoadWeaponVariantDefInternal("sp", name)) {
    return raw_weapon_file;
  }

  auto* zone_weapon_file =
      game::DB_FindXAssetHeader(game::ASSET_TYPE_WEAPON, name).weapon;
  return game::DB_IsXAssetDefault(game::ASSET_TYPE_WEAPON, name)
             ? nullptr
             : zone_weapon_file;
}
} // namespace

class component final : public component_interface {
public:
  void post_load() override { patch_sp(); }

private:
  static void patch_sp() {
    // Weapon asset existence check
    utils::hook::nop(0x43E1D8, 5);
    // Is asset default
    utils::hook::nop(0x43E1E0, 5);
    // Do not jump
    utils::hook::nop(0x43E1EA, 2);

    // Ignore missing default weapon
    utils::hook::set<std::uint8_t>(0x659DBE, 0xEB);

    utils::hook(0x659E00, bg_load_weapon_variant_def_fast_file_stub, HOOK_JUMP)
        .install()
        ->quick();
    // Disable warning if raw weapon file cannot be found
    utils::hook::nop(0x659730, 5);
  }
};
} // namespace weapons

REGISTER_COMPONENT(weapons::component)
