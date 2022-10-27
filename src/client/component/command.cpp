#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/string.hpp>
#include <utils/memory.hpp>
#include <utils/hook.hpp>

#include "command.hpp"

namespace command {
namespace {
utils::hook::detour client_command_hook;

std::unordered_map<std::string, std::function<void(params&)>> handlers;
std::unordered_map<std::string,
                   std::function<void(game::gentity_s*, params_sv&)>>
    handlers_sv;

void main_handler() {
  params params;

  const auto command = utils::string::to_lower(params[0]);
  if (handlers.contains(command)) {
    handlers[command](params);
  }
}

void client_command_stub(int client_num, const char* s) {

  auto* entity = &game::g_entities[client_num];

  params_sv params;

  const auto command = utils::string::to_lower(params[0]);
  const auto got = handlers_sv.find(command);

  if (got != handlers_sv.end()) {
    got->second(entity, params);
    return;
  }

  client_command_hook.invoke<void>(client_num, s);
}
} // namespace

params::params() : nesting_(game::cmd_args->nesting) {
  assert(game::cmd_args->nesting < game::CMD_MAX_NESTING);
}

int params::size() const { return game::cmd_args->argc[this->nesting_]; }

const char* params::get(const int index) const {
  if (index >= this->size()) {
    return "";
  }

  return game::cmd_args->argv[this->nesting_][index];
}

std::string params::join(const int index) const {
  std::string result;

  for (auto i = index; i < this->size(); i++) {
    if (i > index)
      result.append(" ");
    result.append(this->get(i));
  }

  return result;
}

params_sv::params_sv() : nesting_(game::sv_cmd_args->nesting) {
  assert(game::sv_cmd_args->nesting < game::CMD_MAX_NESTING);
}

int params_sv::size() const { return game::sv_cmd_args->argc[this->nesting_]; }

const char* params_sv::get(const int index) const {
  if (index >= this->size()) {
    return "";
  }

  return game::sv_cmd_args->argv[this->nesting_][index];
}

std::string params_sv::join(const int index) const {
  std::string result;

  for (auto i = index; i < this->size(); i++) {
    if (i > index)
      result.append(" ");
    result.append(this->get(i));
  }

  return result;
}

void add_raw(const char* name, void (*callback)(), const int is_key) {
  assert(is_key == 0 || is_key == 1);

  game::Cmd_AddCommand(
      name, callback,
      utils::memory::get_allocator()->allocate<game::cmd_function_s>(), is_key);
}

void add(const char* name, const std::function<void(const params&)>& callback) {
  const auto command = utils::string::to_lower(name);

  if (!handlers.contains(command)) {
    add_raw(name, main_handler);
  }

  handlers[command] = callback;
}

void add(const char* name, const std::function<void()>& callback) {
  add(name, [callback](const params&) { callback(); });
}

void add_sv(const char* name,
            std::function<void(game::gentity_s*, const params_sv&)> callback) {
  const auto command = utils::string::to_lower(name);

  if (!handlers_sv.contains(command)) {
    handlers_sv[command] = callback;
  }
}

void execute(std::string command, const bool sync) {
  command += "\n";

  if (sync) {
    game::Cmd_ExecuteSingleCommand(0, 0, command.data());
  } else {
    game::Cbuf_AddText(0, command.data());
  }
}

class component final : public component_interface {
public:
  static_assert(sizeof(game::cmd_function_s) == 0x18);
  static_assert(offsetof(game::gentity_s, client) == 0x108);

  void post_load() override {
    client_command_hook.create(0x4DF4B0, client_command_stub);
  }

  void pre_destroy() override { client_command_hook.clear(); }
};
} // namespace command

REGISTER_COMPONENT(command::component)
