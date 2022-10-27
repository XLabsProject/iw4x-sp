#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

#include "extension.hpp"

namespace gsc {
namespace {
struct script_function_def {
  game::BuiltinFunction actionFunc;
  bool type;
};

struct script_method_def {
  game::BuiltinMethod actionFunc;
  bool type;
};

std::unordered_map<std::string, script_function_def> custom_scr_funcs;
std::unordered_map<std::string, script_method_def> custom_scr_meths;

game::BuiltinFunction built_in_get_function_stub(const char** p_name,
                                                 int* type) {
  if (p_name) {
    const auto got = custom_scr_funcs.find(utils::string::to_lower(*p_name));
    if (got != custom_scr_funcs.end()) {
      *type = got->second.type;
      return got->second.actionFunc;
    }
  } else {
    for (const auto& [name, func] : custom_scr_funcs) {
      game::Scr_RegisterFunction(reinterpret_cast<int>(func.actionFunc),
                                 name.data());
    }
  }

  // If no function was found let's call BuiltIn_GetFunction
  return utils::hook::invoke<game::BuiltinFunction>(0x4DD160, p_name, type);
}

game::BuiltinMethod built_in_get_method_stub(const char** p_name, int* type) {
  if (p_name) {
    const auto got = custom_scr_meths.find(utils::string::to_lower(*p_name));
    if (got != custom_scr_meths.end()) {
      *type = got->second.type;
      return got->second.actionFunc;
    }
  } else {
    for (const auto& [name, meth] : custom_scr_meths) {
      game::Scr_RegisterFunction(reinterpret_cast<int>(meth.actionFunc),
                                 name.data());
    }
  }

  // If no method was found let's call BuiltIn_GetMethod
  return utils::hook::invoke<game::BuiltinMethod>(0x5DB850, p_name, type);
}
} // namespace

void add_function(const char* name, game::BuiltinFunction func, bool type) {
  script_function_def def;
  def.actionFunc = func;
  def.type = type;

  custom_scr_funcs.emplace(utils::string::to_lower(name), def);
}

void add_method(const char* name, game::BuiltinMethod func, bool type) {
  script_method_def def;
  def.actionFunc = func;
  def.type = type;

  custom_scr_meths.emplace(utils::string::to_lower(name), def);
}

class extension final : public component_interface {
public:
  void post_load() override {
    // Fetch custom functions
    utils::hook(0x4ADF9C, built_in_get_function_stub, HOOK_CALL)
        .install()
        ->quick(); // Scr_GetFunction

    utils::hook(0x444827, built_in_get_method_stub, HOOK_CALL)
        .install()
        ->quick(); // Scr_GetMethod

    add_functions();
#ifdef _DEBUG
    add_debug_functions();
#endif
  }

  static void add_functions() {
    add_function("StrICmp", [] {
      const auto result = game::I_stricmp(
          game::SL_ConvertToString(game::Scr_GetConstString(0)),
          game::SL_ConvertToString(game::Scr_GetConstString(1)));

      game::Scr_AddInt(result);
    });

    add_function("Float", [] {
      switch (game::Scr_GetType(0)) {
      case 2:
        game::Scr_AddFloat(
            static_cast<float>(std::atof(game::Scr_GetString(0))));
        break;
      case 5:
        game::Scr_AddFloat(game::Scr_GetFloat(0));
        break;
      case 6:
        game::Scr_AddFloat(static_cast<float>(game::Scr_GetInt(0)));
        break;
      default:
        game::Scr_ParamError(0, utils::string::va("cannot cast %s to float",
                                                  game::Scr_GetTypeName(0)));
        break;
      }
    });
  }

  static void add_debug_functions() {
    add_function("AddDebugCommand",
                 [] { game::Cbuf_AddText(0, game::Scr_GetString(0)); });
  }
};
} // namespace gsc

REGISTER_COMPONENT(gsc::extension)
