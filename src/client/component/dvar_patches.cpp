#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>

#include "scheduler.hpp"
#include "dvar.hpp"

namespace dvar_patches {
namespace {
const game::dvar_t* dvar_register_name(const char* dvar_name, const char* value,
                                       unsigned __int16 flags,
                                       const char* description) {
  return game::Dvar_RegisterString(
      dvar_name, value,
      game::DVAR_ARCHIVE | game::DVAR_USERINFO | game::DVAR_UNK1, description);
}
} // namespace

class component final : public component_interface {
public:
  void post_start() override {
    dvar::override::dvar_register_bool("intro", true, game::DVAR_NONE);
    dvar::override::dvar_register_float("cg_fov", 65.0f, 65.0f, 160.0f,
                                        game::DVAR_ARCHIVE);
    dvar::override::dvar_register_string("fs_basegame", BASEGAME,
                                         game::DVAR_INIT);

#ifdef _DEBUG
    dvar::override::dvar_register_bool("sv_cheats", true, game::DVAR_NONE);
#endif
  }

  void post_load() override {
    utils::hook(0x475156, dvar_register_name, HOOK_CALL).install()->quick();
    patch_sp();
  }

private:
  static void patch_sp() {
    utils::hook::set<std::uint8_t>(0x635841, 0xEB); // Read only
    utils::hook::set<std::uint8_t>(0x635913, 0xEB); // Cheat protected
    utils::hook::set<std::uint8_t>(0x6358A5, 0xEB); // Write protected
    utils::hook::set<std::uint8_t>(0x635974, 0xEB); // Latched

#ifdef _DEBUG
    // Nop Dvar_RegisterVariant for sv_cheats in Dvar_Init
    utils::hook::nop(0x471522, 5);
#endif

    // Skip dvar output
    utils::hook::set<std::uint8_t>(0x4CD2B7, 0xEB);

    // Ignore server dvar change for clients
    utils::hook::set<std::uint8_t>(0x63580B, 0xEB);

    scheduler::once(
        [] {
          game::Dvar_RegisterBool("scr_damageFeedback", true, game::DVAR_NONE,
                                  "Show marker when hitting enemies");
          game::Dvar_RegisterString("connect_ip", "127.0.0.1:28960",
                                    game::DVAR_NONE,
                                    "Temporary dvar used to connect");
        },
        scheduler::pipeline::main);
  }
};
} // namespace dvar_patches

REGISTER_COMPONENT(dvar_patches::component)
