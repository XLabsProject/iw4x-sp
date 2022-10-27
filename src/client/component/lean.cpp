#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>

#include "command.hpp"

namespace lean {
namespace {
game::kbutton_t in_lean_left;
game::kbutton_t in_lean_right;

void in_lean_left_down() { game::IN_KeyDown(&in_lean_left); }

void in_lean_left_up() { game::IN_KeyUp(&in_lean_left); }

void in_lean_right_down() { game::IN_KeyDown(&in_lean_right); }

void in_lean_right_up() { game::IN_KeyUp(&in_lean_right); }

void set_lean_flags(game::usercmd_s* cmd) {
  if (in_lean_left.active || in_lean_left.wasPressed) {
    cmd->buttons |= game::CMD_BUTTON_LEAN_LEFT;
  }

  if (in_lean_right.active || in_lean_right.wasPressed) {
    cmd->buttons |= game::CMD_BUTTON_LEAN_RIGHT;
  }

  in_lean_left.wasPressed = false;
  in_lean_right.wasPressed = false;
}

void __declspec(naked) cl_cmd_buttons_stub() {
  __asm {
    pushad;

    push esi;
    call set_lean_flags;
    add esp, 0x4;

    popad;

    // code skipped by our hook
    mov eax, ecx;
    imul eax, eax, 0x21C;

    // CL_CmdButton
    push 0x57B2B8;
    ret;
  }
}
} // namespace

class component final : public component_interface {
public:
  static_assert(sizeof(game::kbutton_t) == 0x14);
  static_assert(sizeof(game::usercmd_s) == 0x40);

  void post_load() override {
    command::add_raw("+leanleft", in_lean_left_down, TRUE);
    command::add_raw("-leanleft", in_lean_left_up, TRUE);

    command::add_raw("+leanright", in_lean_right_down, TRUE);
    command::add_raw("-leanright", in_lean_right_up, TRUE);

    utils::hook(0x57B2B0, cl_cmd_buttons_stub, HOOK_JUMP).install()->quick();
  }
};
} // namespace lean

REGISTER_COMPONENT(lean::component)
