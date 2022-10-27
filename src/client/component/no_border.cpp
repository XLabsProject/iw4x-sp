#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/dvars.hpp"

#include <utils/hook.hpp>

namespace no_border {
namespace {
void __declspec(naked) window_style_stub() {
  __asm {
    push eax;
    mov eax, dvars::r_noBorder;
    cmp byte ptr [eax + 0x10], 1;
    pop eax;

    je remove_border;

    mov ebp, WS_VISIBLE | WS_SYSMENU | WS_CAPTION;
    jmp safe_return;

   remove_border:
    mov ebp, WS_VISIBLE | WS_POPUP;

   safe_return:
    push 0x50C0B8;
    ret;
  }
}
} // namespace

class component final : public component_interface {
public:
  void post_load() override {
    utils::hook(0x50C0B3, window_style_stub, HOOK_JUMP).install()->quick();

    dvars::r_noBorder =
        game::Dvar_RegisterBool("r_noBorder", false, game::DVAR_ARCHIVE,
                                "Do not use a border in windowed mode");
  }
};
} // namespace no_border

REGISTER_COMPONENT(no_border::component)
