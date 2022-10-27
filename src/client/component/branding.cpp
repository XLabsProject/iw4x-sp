#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/dvars.hpp"

#include <utils/string.hpp>
#include <utils/hook.hpp>

#include <version.h>

namespace branding {
namespace {
#ifdef _DEBUG
constexpr auto* BUILD_TYPE = "IW4x_DEV SP";
#else
constexpr auto* BUILD_TYPE = "IW4x SP";
#endif

const char* get_build_number() {
  return SHORTVERSION " latest " __DATE__ " " __TIME__;
}

const char* get_version_string() {
  const auto* result = utils::string::va(
      "%s %s build %s %s", BUILD_TYPE, "(Alpha)", get_build_number(),
      reinterpret_cast<const char*>(0x6A1574));

  return result;
}

void dvar_set_version_string(const game::dvar_t* dvar, const char* /*value*/) {
  const auto* result = get_version_string();
  utils::hook::invoke<void>(0x480E70, dvar, result);
}

void cg_draw_version() {
  assert(game::ScrPlace_IsFullScreenActive());

  // Default values
  constexpr auto font_scale = 0.25f;
  constexpr auto max_chars = std::numeric_limits<int>::max();
  // Default colours
  constexpr float shadow_color[] = {0.0f, 0.0f, 0.0f, 0.69f};
  constexpr float color[] = {0.4f, 0.69f, 1.0f, 0.69f};

  auto* const placement = game::ScrPlace_GetUnsafeFullPlacement();
  auto* const font = game::UI_GetFontHandle(placement, 0, 0.583f);

  const auto width = game::UI_TextWidth((*dvars::version)->current.string, 0,
                                        font, font_scale);
  const auto height = game::UI_TextHeight(font, font_scale);

  // clang-format off
  game::UI_DrawText(placement, (*dvars::version)->current.string, max_chars,
                    font,
                    1.0f - (dvars::cg_drawVersionX->current.value +
                        static_cast<float>(width)),
                    1.0f - (dvars::cg_drawVersionY->current.value +
                        static_cast<float>(height)),
                    3, 3, font_scale, shadow_color, 0);
  game::UI_DrawText(placement, (*dvars::version)->current.string, max_chars,
                    font,
                    (0.0f - static_cast<float>(width)) -
                        dvars::cg_drawVersionX->current.value,
                    (0.0f - static_cast<float>(height)) -
                        dvars::cg_drawVersionY->current.value,
                    3, 3, font_scale, color, 0);
  // clang-format on
}

void cg_draw_full_screen_debug_overlays_stub(int local_client_num) {
  assert(game::ScrPlace_IsFullScreenActive());

  if (dvars::cg_drawVersion->current.enabled) {
    cg_draw_version();
  }

  utils::hook::invoke<void>(0x44BD00, local_client_num);
}
} // namespace

class component final : public component_interface {
public:
  void post_load() override {
    utils::hook(0x46F570, get_build_number, HOOK_JUMP).install()->quick();
    utils::hook(0x60429A, dvar_set_version_string, HOOK_CALL)
        .install()
        ->quick();

    utils::hook::set<const char*>(0x446A48, "IW4x-SP: Console");
    utils::hook::set<const char*>(0x50C110, "IW4x-SP: Game");

    utils::hook::set<const char*>(0x579364, "IW4x-SP: " SHORTVERSION "> ");

    // Com_Init_Try_Block_Function
    utils::hook::set<const char*>(0x604004, BUILD_TYPE);
    utils::hook::set<const char*>(0x603FFF, SHORTVERSION);
    utils::hook::set<const char*>(0x603FF5, __DATE__);

    register_branding_dvars();
    utils::hook(0x57DAFF, cg_draw_full_screen_debug_overlays_stub, HOOK_CALL)
        .install()
        ->quick();
  }

  static void register_branding_dvars() {
#ifdef _DEBUG
    constexpr auto value = true;
#else
    constexpr auto value = false;
#endif
    dvars::cg_drawVersion = game::Dvar_RegisterBool(
        "cg_drawVersion", value, game::DVAR_NONE, "Draw the game version");
    dvars::cg_drawVersionX = game::Dvar_RegisterFloat(
        "cg_drawVersionX", 50.0f, 0.0f, 512.0f, game::DVAR_NONE,
        "X offset for the version string");
    dvars::cg_drawVersionY = game::Dvar_RegisterFloat(
        "cg_drawVersionY", 18.0f, 0.0f, 512.0f, game::DVAR_NONE,
        "Y offset for the version string");
  }
};
} // namespace branding

REGISTER_COMPONENT(branding::component)
