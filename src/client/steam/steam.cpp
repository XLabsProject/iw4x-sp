#include <std_include.hpp>
#include "steam/steam.hpp"

namespace steam {
::utils::nt::library overlay(nullptr);

std::uint64_t callbacks::call_id_ = 0;
std::recursive_mutex callbacks::mutex_;
std::map<std::uint64_t, bool> callbacks::calls_;
std::map<std::uint64_t, callbacks::base*> callbacks::result_handlers_;
std::vector<callbacks::result> callbacks::results_;
std::vector<callbacks::base*> callbacks::callback_list_;

uint64_t callbacks::register_call() {
  std::lock_guard _(mutex_);
  calls_[++call_id_] = false;
  return call_id_;
}

void callbacks::register_callback(base* handler, const int callback) {
  std::lock_guard _(mutex_);
  handler->set_i_callback(callback);
  callback_list_.push_back(handler);
}

void callbacks::unregister_callback(base* handler) {
  std::lock_guard _(mutex_);
  for (auto i = callback_list_.begin(); i != callback_list_.end();) {
    if (*i == handler) {
      i = callback_list_.erase(i);
    } else {
      ++i;
    }
  }
}

void callbacks::register_call_result(const uint64_t call, base* result) {
  std::lock_guard _(mutex_);
  result_handlers_[call] = result;
}

void callbacks::unregister_call_result(const uint64_t call,
                                       [[maybe_unused]] base* result) {
  std::lock_guard _(mutex_);
  const auto i = result_handlers_.find(call);
  if (i != result_handlers_.end()) {
    result_handlers_.erase(i);
  }
}

void callbacks::return_call(void* data, const int size, const int type,
                            const uint64_t call) {
  std::lock_guard _(mutex_);

  result result;
  result.call = call;
  result.data = data;
  result.size = size;
  result.type = type;

  calls_[call] = true;

  results_.push_back(result);
}

void callbacks::run_callbacks() {
  std::lock_guard _(mutex_);

  for (const auto& result : results_) {
    if (const auto itr = result_handlers_.find(result.call);
        itr != result_handlers_.end()) {
      itr->second->run(result.data, false, result.call);
    }

    for (const auto& callback : callback_list_) {
      if (callback && callback->get_i_callback() == result.type) {
        callback->run(result.data, false, 0);
      }
    }

    if (result.data) {
      std::free(result.data);
    }
  }

  results_.clear();
}

std::filesystem::path get_steam_install_directory() {
  static std::string install_path{};
  if (!install_path.empty()) {
    return {install_path};
  }

  HKEY reg_key;
  if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software\\Valve\\Steam", 0,
                    KEY_QUERY_VALUE, &reg_key) == ERROR_SUCCESS) {
    char path[MAX_PATH] = {0};
    DWORD length = sizeof(path);
    RegQueryValueExA(reg_key, "InstallPath", nullptr, nullptr,
                     reinterpret_cast<BYTE*>(path), &length);
    RegCloseKey(reg_key);

    install_path = path;
  }

  return {install_path};
}

extern "C" {
bool SteamAPI_RestartAppIfNecessary() { return false; }

bool SteamAPI_Init() {
  overlay = ::utils::nt::library("gameoverlayrenderer.dll");

  if (!overlay) {
    const auto steam_path = get_steam_install_directory();
    if (!steam_path.empty()) {
      overlay =
          ::utils::nt::library::load(steam_path / "gameoverlayrenderer.dll");
    }
  }

  return true;
}

void SteamAPI_RegisterCallResult(callbacks::base* result, uint64_t call) {
  callbacks::register_call_result(call, result);
}

void SteamAPI_RegisterCallback(callbacks::base* handler, int callback) {
  callbacks::register_callback(handler, callback);
}

void SteamAPI_RunCallbacks() { callbacks::run_callbacks(); }

void SteamAPI_Shutdown() {}

void SteamAPI_UnregisterCallResult(callbacks::base* result,
                                   const std::uint64_t call) {
  callbacks::unregister_call_result(call, result);
}

void SteamAPI_UnregisterCallback(callbacks::base* handler) {
  callbacks::unregister_callback(handler);
}

bool SteamGameServer_Init() { return true; }

void SteamGameServer_RunCallbacks() {}

void SteamGameServer_Shutdown() {}

friends* SteamFriends() {
  static friends friends;
  return &friends;
}

matchmaking* SteamMatchmaking() {
  static matchmaking matchmaking;
  return &matchmaking;
}

matchmaking_servers* SteamMatchmakingServers() {
  static matchmaking_servers matchmaking_servers;
  return &matchmaking_servers;
}

game_server* SteamGameServer() {
  static game_server game_server;
  return &game_server;
}

master_server_updater* SteamMasterServerUpdater() {
  static master_server_updater master_server_updater;
  return &master_server_updater;
}

networking* SteamNetworking() {
  static networking networking;
  return &networking;
}

remote_storage* SteamRemoteStorage() {
  static remote_storage remote_storage;
  return &remote_storage;
}

user* SteamUser() {
  static user user;
  return &user;
}

utils* SteamUtils() {
  static utils utils;
  return &utils;
}

apps* SteamApps() {
  static apps apps;
  return &apps;
}

user_stats* SteamUserStats() {
  static user_stats user_stats;
  return &user_stats;
}
}
} // namespace steam
