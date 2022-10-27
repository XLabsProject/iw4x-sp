#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <utils/nt.hpp>

#include "command.hpp"
#include "filesystem.hpp"

namespace patches {
namespace {
void sys_init_stub() {
  game::Cmd_SetAutoComplete("exec", "", "cfg");
  utils::hook::invoke<void>(0x4EEB20);
}

void cl_play_unskippable_cinematic_f() {
  // CL_PlayCinematic_f
  utils::hook::invoke<void>(0x4CC950);
  *game::cin_skippable = true;
}

void cl_start_multiplayer_f() {
  utils::nt::update_dll_search_path(filesystem::get_binary_directory());
  utils::nt::launch_process("iw4x.exe", "-multiplayer");
  command::execute("quit", false);
}

const char* live_get_local_client_name_stub() {
  return game::Dvar_FindVar("name")->current.string;
}
} // namespace

class component final : public component_interface {
public:
  void post_load() override {
    utils::hook(0x6042A2, sys_init_stub, HOOK_CALL).install()->quick();

    utils::hook(0x492EF0, live_get_local_client_name_stub, HOOK_JUMP)
        .install()
        ->quick();

    patch_sp();
  }

private:
  static void patch_sp() {
    // Force external console
    utils::hook::nop(0x604071, 21);

    // Remove limit on IWD file loading
    utils::hook::set<std::uint8_t>(0x630FF3, 0xEB);

    // Ignore XUID match
    utils::hook::set<std::uint8_t>(0x65D1AF, 0xEB);

    // Remove this particular string from the log file
    utils::hook::nop(0x6030A6, 5);

    // Config related
    utils::hook::set<std::uint8_t>(0x4D3FD3, 0xEB);

    // Improper quit popup
    utils::hook::nop(0x4F5B3A, 2);

    // Remove fs_game check
    utils::hook::nop(0x612932, 2);

    // Build os path stuff
    utils::hook::set<std::uint8_t>(0x6300BF, 0xEB);

    // Show intro (or not)
    utils::hook::set<std::uint8_t>(0x6035BD, 0x0);

    // raw -> main
    utils::hook::set<std::uint32_t>(0x50A0B2, 0x723390);

    // Disable 'replay' output
    utils::hook::nop(0x65F14A, 5);

    // Disable heartbeat output
    utils::hook::nop(0x57EF87, 5);

    // Allow intro to be skipped
    utils::hook::set<void (*)()>(0x47529F, cl_play_unskippable_cinematic_f);

    // Start IW4x
    utils::hook::set<void (*)()>(0x475327, cl_start_multiplayer_f);

    // Enable commandline arguments
    utils::hook::set<std::uint8_t>(0x453B24, 0xEB);

    // Rename config
    utils::hook::set<const char*>(0x6040F2, CLIENT_CONFIG);
    utils::hook::set<const char*>(0x602D38, CLIENT_CONFIG);
    utils::hook::set<const char*>(0x6037C9, CLIENT_CONFIG);
    utils::hook::set<const char*>(0x4D3FDB, CLIENT_CONFIG);
    utils::hook::set<const char*>(0x469DB6, CLIENT_CONFIG);
  }
};
} // namespace patches

REGISTER_COMPONENT(patches::component)
