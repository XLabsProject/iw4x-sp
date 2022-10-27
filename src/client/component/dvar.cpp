
#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>

#include "dvar.hpp"

namespace dvar {
struct dvar_base {
  std::uint16_t flags{};
};

struct dvar_bool : dvar_base {
  bool value{};
};

struct dvar_float : dvar_base {
  float value{};
  float min{};
  float max{};
};

struct dvar_int : dvar_base {
  int value{};
  int min{};
  int max{};
};

struct dvar_string : dvar_base {
  std::string value{};
};

namespace {
template <typename T>
T* find_dvar(std::unordered_map<std::string, T>& map, const std::string& name) {
  auto i = map.find(name);
  if (i != map.end()) {
    return &i->second;
  }

  return nullptr;
}
} // namespace

namespace override {
static std::unordered_map<std::string, dvar_bool> register_bool_overrides;
static std::unordered_map<std::string, dvar_float> register_float_overrides;
static std::unordered_map<std::string, dvar_int> register_int_overrides;
static std::unordered_map<std::string, dvar_string> register_string_overrides;

void dvar_register_bool(const std::string& name, const bool value,
                        const std::uint16_t flags) {
  dvar_bool values;
  values.value = value;
  values.flags = flags;
  register_bool_overrides[name] = values;
}

void dvar_register_float(const std::string& name, const float value,
                         const float min, const float max,
                         const std::uint16_t flags) {
  dvar_float values;
  values.value = value;
  values.min = min;
  values.max = max;
  values.flags = flags;
  register_float_overrides[name] = values;
}

void dvar_register_int(const std::string& name, const int value, const int min,
                       const int max, const std::uint16_t flags) {
  dvar_int values;
  values.value = value;
  values.min = min;
  values.max = max;
  values.flags = flags;
  register_int_overrides[name] = values;
}

void dvar_register_string(const std::string& name, const std::string& value,
                          const std::uint16_t flags) {
  dvar_string values;
  values.value = value;
  values.flags = flags;
  register_string_overrides[name] = values;
}
} // namespace override

namespace {
utils::hook::detour dvar_register_bool_hook;
utils::hook::detour dvar_register_float_hook;
utils::hook::detour dvar_register_int_hook;
utils::hook::detour dvar_register_string_hook;

const game::dvar_t* dvar_register_bool_stub(const char* name, bool value,
                                            std::uint16_t flags,
                                            const char* description) {
  auto* var = find_dvar(override::register_bool_overrides, name);
  if (var) {
    value = var->value;
    flags = var->flags;
  }

  return dvar_register_bool_hook.invoke<const game::dvar_t*>(name, value, flags,
                                                             description);
}

const game::dvar_t* dvar_register_float_stub(const char* name, float value,
                                             float min, float max,
                                             std::uint16_t flags,
                                             const char* description) {
  auto* var = find_dvar(override::register_float_overrides, name);
  if (var) {
    value = var->value;
    min = var->min;
    max = var->max;
    flags = var->flags;
  }

  return dvar_register_float_hook.invoke<const game::dvar_t*>(
      name, value, min, max, flags, description);
}

const game::dvar_t* dvar_register_int_stub(const char* name, int value, int min,
                                           int max, std::uint16_t flags,
                                           const char* description) {
  auto* var = find_dvar(override::register_int_overrides, name);
  if (var) {
    value = var->value;
    min = var->min;
    max = var->max;
    flags = var->flags;
  }

  return dvar_register_int_hook.invoke<const game::dvar_t*>(
      name, value, min, max, flags, description);
}

const game::dvar_t* dvar_register_string_stub(const char* name,
                                              const char* value,
                                              std::uint16_t flags,
                                              const char* description) {
  auto* var = find_dvar(override::register_string_overrides, name);
  if (var) {
    value = var->value.data();
    flags = var->flags;
  }

  return dvar_register_string_hook.invoke<const game::dvar_t*>(
      name, value, flags, description);
}
} // namespace

class component final : public component_interface {
public:
  void post_load() override {
    dvar_register_bool_hook.create(0x429390, &dvar_register_bool_stub);
    dvar_register_float_hook.create(0x4051D0, &dvar_register_float_stub);
    dvar_register_int_hook.create(0x4E9490, &dvar_register_int_stub);
    dvar_register_string_hook.create(0x49E0B0, &dvar_register_string_stub);
  }

  void pre_destroy() override {
    dvar_register_bool_hook.clear();
    dvar_register_float_hook.clear();
    dvar_register_int_hook.clear();
    dvar_register_string_hook.clear();
  }
};
} // namespace dvar

REGISTER_COMPONENT(dvar::component)
