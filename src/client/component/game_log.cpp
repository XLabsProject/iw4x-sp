#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/dvars.hpp"

#include <utils/hook.hpp>

#include "gsc/extension.hpp"

#include "game_log.hpp"
#include "scheduler.hpp"

namespace game_log {
namespace {
int log_file = 0;

void g_scr_log_print() {
  char string[1024]{};
  std::size_t i_string_len = 0;

  const auto i_num_parms = game::Scr_GetNumParam();
  for (std::size_t i = 0; i < i_num_parms; ++i) {
    const auto* psz_token = game::Scr_GetString(i);
    const auto i_token_len = std::strlen(psz_token);

    i_string_len += i_token_len;
    if (i_string_len >= sizeof(string)) {
      // Do not overflow the buffer
      break;
    }

    strncat_s(string, psz_token, _TRUNCATE);
  }

  log_printf("%s", string);
}

void g_init_game_stub() {
  game::Com_Printf(game::CON_CHANNEL_SERVER,
                   "------- Game Initialization -------\n");
  game::Com_Printf(game::CON_CHANNEL_SERVER, "gamename: IW4x\n");
  game::Com_Printf(game::CON_CHANNEL_SERVER, "gamedate: %s\n", __DATE__);

  if (*dvars::g_log->current.string == '\0') {
    game::Com_Printf(game::CON_CHANNEL_SERVER, "Not logging to disk.\n");
  } else {
    game::FS_FOpenFileByMode(dvars::g_log->current.string, &log_file,
                             game::FS_APPEND_SYNC);

    if (!log_file) {
      game::Com_PrintWarning(game::CON_CHANNEL_SERVER,
                             "WARNING: Couldn't open logfile: %s\n");
    } else {
      log_printf(
          "------------------------------------------------------------\n");
      log_printf("InitGame\n");
    }
  }

  utils::hook::invoke<void>(0x4FA880); // Vehicle_ClearServerDefs
}

void g_shutdown_game_stub() {
  if (log_file) {
    log_printf("ShutdownGame:\n");
    log_printf(
        "------------------------------------------------------------\n");

    game::FS_FCloseFile(log_file);
    log_file = 0;
  }

  utils::hook::invoke<void>(0x455F90); // Actor_ClearThreatBiasGroups
}
} // namespace

void log_printf(const char* fmt, ...) {
  char string[1024]{};
  char string2[1024]{};
  va_list ap;

  if (!log_file) {
    return;
  }

  va_start(ap, fmt);
  vsnprintf_s(string2, _TRUNCATE, fmt, ap);
  va_end(ap);

  const auto time = game::level->time / 1000;
  const auto len = sprintf_s(string, "%3i:%i%i %s", time / 60, time % 60 / 10,
                             time % 60 % 10, string2);
  game::FS_Write(string, len, log_file);
}

class component final : public component_interface {
public:
  static_assert(offsetof(game::level_locals_t, time) == 0x34);

  void post_load() override {
    utils::hook(0x4C75E0, g_init_game_stub, HOOK_CALL).install()->quick();
    utils::hook(0x418B5A, g_shutdown_game_stub, HOOK_CALL).install()->quick();

    scheduler::once(
        [] {
          dvars::g_log = game::Dvar_RegisterString(
              "g_log", "games_sp.log", game::DVAR_NONE, "Log file name");
        },
        scheduler::pipeline::main);

    gsc::add_function("logprint", g_scr_log_print);
  }
};
} // namespace game_log

REGISTER_COMPONENT(game_log::component)
