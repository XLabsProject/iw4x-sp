#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/dvars.hpp"
#include "game/engine/scoped_critical_section.hpp"
#include "game/engine/large_local.hpp"

#include <utils/hook.hpp>

#include "command.hpp"
#include "scheduler.hpp"

namespace debug {
namespace {
void com_assert_f() { assert("a" && false); }

void com_bug_f(const command::params& params) {
  char new_file_name[0x105]{};
  char to_ospath[MAX_PATH]{};
  char from_ospath[MAX_PATH]{};
  const char* bug;

  if (!*game::logfile) {
    game::Com_PrintError(game::CON_CHANNEL_ERROR,
                         "CopyFile failed: logfile wasn't opened\n");
  }

  if (params.size() == 2) {
    bug = params.get(1);
  } else {
    assert(dvars::bug_name);
    bug = dvars::bug_name->current.string;
  }

  _snprintf_s(new_file_name, _TRUNCATE, "%s_%s.log", bug,
              game::Live_GetLocalClientName(0));

  game::engine::scoped_critical_section _(game::CRITSECT_CONSOLE,
                                          game::SCOPED_CRITSECT_NORMAL);

  if (*game::logfile) {
    game::FS_FCloseFile(*game::logfile);
    *game::logfile = 0;
  }

  game::FS_BuildOSPath(std::filesystem::current_path().string().data(), "",
                       "console.log", from_ospath);
  game::FS_BuildOSPath(std::filesystem::current_path().string().data(), "",
                       new_file_name, to_ospath);
  const auto result = CopyFileA(from_ospath, to_ospath, 0);
  game::Com_OpenLogFile();

  if (!result) {
    game::Com_PrintError(game::CON_CHANNEL_ERROR, "CopyFile failed(%d) %s %s\n",
                         GetLastError(), "console.log", new_file_name);
  }
}

void g_print_fast_file_errors(const char* fastfile) {
  assert(fastfile);

  game::engine::large_local rawfile_buf_large_local(0x18000);
  auto* rawfile_buf = static_cast<char*>(rawfile_buf_large_local.get_buf());

  auto* text = game::DB_ReadRawFile(fastfile, rawfile_buf, 0x18000);

  assert(text);

  if (*text) {
    game::Com_PrintError(game::CON_CHANNEL_ERROR,
                         "There were errors when building fast file '%s'\n",
                         fastfile);
    game::Com_PrintError(game::CON_CHANNEL_ERROR, "%s", text);
  }
}

void g_init_game_stub() {
  utils::hook::invoke<void>(0x4D6410);

  g_print_fast_file_errors("code_post_gfx");
  g_print_fast_file_errors("common");
  g_print_fast_file_errors((*dvars::sv_mapname)->current.string);
}
} // namespace

class component final : public component_interface {
public:
  static_assert(sizeof(RTL_CRITICAL_SECTION) == 0x18);

  void post_load() override {
    utils::hook::set<void (*)()>(0x604203, com_assert_f);

    scheduler::once(
        [] {
          dvars::bug_name = game::Dvar_RegisterString(
              "bug_name", "bug0", game::DVAR_CHEAT | game::DVAR_CODINFO,
              "Name appended to the copied console log");
        },
        scheduler::pipeline::main);

    command::add("bug", com_bug_f);

#ifdef _DEBUG
    utils::hook(0x4C79DF, g_init_game_stub, HOOK_CALL)
        .install()
        ->quick(); // Scr_FreeEntityList
#endif
  }
};
} // namespace debug

REGISTER_COMPONENT(debug::component)
