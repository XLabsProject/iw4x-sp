#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>

namespace steam_patches {
namespace {
void __declspec(naked) steam_init() {
  const static DWORD func = 0x4293F0;

  __asm {
    call func;
    test al, al;
    jz return_safe;

    push 0x43FAF9;
    ret;

  return_safe:
    mov al, 1;
    ret;
  }
}
} // namespace

class component final : public component_interface {
public:
  void post_load() override {
    patch_sp();
    utils::hook(0x43FAF0, steam_init, HOOK_JUMP).install()->quick();
  }

  static void patch_sp() {
    // Prevent stat loading from steam
    utils::hook::set<std::uint8_t>(0x43FB33, 0xC3);

    // Steam must be running
    utils::hook::nop(0x6040A3, 30);

    // No-Steam
    utils::hook::nop(0x4E9458, 7);
    utils::hook::nop(0x4E9470, 7);
  }
};
} // namespace steam_patches

REGISTER_COMPONENT(steam_patches::component)
