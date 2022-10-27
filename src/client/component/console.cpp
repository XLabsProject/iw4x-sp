#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>

#include "dvar.hpp"

namespace console {
namespace {
void hide_console() {
  auto* const con_window = GetConsoleWindow();

  DWORD process;
  GetWindowThreadProcessId(con_window, &process);

  if (process == GetCurrentProcessId() || IsDebuggerPresent()) {
    ShowWindow(con_window, SW_HIDE);
  }
}

void con_toggle_console() {
  game::Field_Clear(game::g_consoleField);

  if (game::conDrawInputGlob->matchIndex >= 0 &&
      game::conDrawInputGlob->autoCompleteChoice[0] != '\0') {
    game::conDrawInputGlob->matchIndex = -1;
    game::conDrawInputGlob->autoCompleteChoice[0] = '\0';
  }

  game::g_consoleField->widthInPixels = *game::g_console_field_width;
  game::g_consoleField->charHeight = *game::g_console_char_height;
  game::g_consoleField->fixedSize = 1;
  game::con->outputVisible = false;

  // clientUIActives[0].keyCatchers
  utils::hook::set<std::uint32_t>(0x929140,
                                  *reinterpret_cast<std::uint32_t*>(0x929140) ^
                                      game::KEYCATCH_CONSOLE);
}
} // namespace

class component final : public component_interface {
public:
  static_assert(sizeof(game::field_t) == 0x118);
  static_assert(sizeof(game::ConDrawInputGlob) == 0x64);

  component() { hide_console(); }

  void post_start() override {
    // Prevents console from opening
    dvar::override::dvar_register_bool("monkeytoy", false, game::DVAR_NONE);
  }

  void post_load() override {
    utils::hook(0x44317E, con_toggle_console, HOOK_CALL)
        .install()
        ->quick(); // CL_KeyEvent
    utils::hook(0x442E8E, con_toggle_console, HOOK_JUMP)
        .install()
        ->quick(); // CL_KeyEvent
  }
};
} // namespace console

REGISTER_COMPONENT(console::component)
