#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace gsc {
namespace {
std::vector<int> main_handles;
std::vector<int> init_handles;

// Do not use C++ objects because Scr_LoadScript may longjmp
void g_scr_load_scripts_stub() {
  // Clear handles (from previous GSC loading session)
  main_handles.clear();
  init_handles.clear();

  char path[MAX_PATH]{};

  auto num_files = 0;
  const auto** files =
      game::FS_ListFiles("scripts/", "gsc", game::FS_LIST_ALL, &num_files, 10);

  for (auto i = 0; i < num_files; ++i) {
    const auto* script_file = files[i];
    game::Com_Printf(game::CON_CHANNEL_SERVER, "Loading script %s...\n",
                     script_file);

    sprintf_s(path, "%s/%s", "scripts", script_file);

    // Scr_LoadScriptInternal will add the '.gsc' suffix so we remove it
    path[std::strlen(path) - 4] = '\0';

    if (!game::Scr_LoadScript(path)) {
      game::Com_Printf(game::CON_CHANNEL_SERVER,
                       "Script %s encountered an error while loading\n", path);
      continue;
    }

    game::Com_Printf(game::CON_CHANNEL_SERVER,
                     "Script %s.gsc loaded successfully\n", path);

    const auto main_handle = game::Scr_GetFunctionHandle(path, "main");
    if (main_handle) {
      main_handles.push_back(main_handle);
    }

    const auto init_handle = game::Scr_GetFunctionHandle(path, "init");
    if (init_handle) {
      init_handles.push_back(init_handle);
    }
    // Allow scripts with no handles
  }

  game::FS_FreeFileList(files, 10);
}

void scr_load_level_stub() {
  for (const auto& handle : main_handles) {
    const auto id = game::Scr_ExecThread(handle, 0);
    game::Scr_FreeThread(static_cast<std::uint16_t>(id));
  }

  utils::hook::invoke<void>(0x470860); // Scr_LoadLevel

  for (const auto& handle : init_handles) {
    const auto id = game::Scr_ExecThread(handle, 0);
    game::Scr_FreeThread(static_cast<std::uint16_t>(id));
  }
}
} // namespace

class loading final : public component_interface {
public:
  void post_load() override {
    utils::hook(0x4054C2, g_scr_load_scripts_stub, HOOK_JUMP)
        .install()
        ->quick();

    utils::hook(0x4C1E34, scr_load_level_stub, HOOK_CALL).install()->quick();
  }
};
} // namespace gsc

REGISTER_COMPONENT(gsc::loading)
