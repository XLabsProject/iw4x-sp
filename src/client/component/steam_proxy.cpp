#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/nt.hpp>
#include <utils/string.hpp>
#include <utils/binary_resource.hpp>
#include <utils/flags.hpp>

#include <resource.hpp>

#include "steam/steam.hpp"
#include "steam/interface.hpp"

#include "scheduler.hpp"

namespace {
utils::binary_resource runner_file(RUNNER, "iw4xsp-runner.exe");

bool is_disabled() {
  static const auto disabled = utils::flags::has_flag("nosteam");
  return disabled;
}
} // namespace

class steam_proxy final : public component_interface {
public:
  void post_load() override {
    if (is_disabled()) {
      return;
    }

    this->load_client();
    this->clean_up_on_error();

#ifndef _DEBUG
    try {
      this->start_mod("IW4x-SP Singleplayer", 10180);
    } catch (const std::exception& ex) {
      printf("Steam: %s\n", ex.what());
    }
#endif
  }

  void pre_destroy() override {
    if (this->steam_client_module_) {
      if (this->steam_pipe_) {
        if (this->global_user_) {
          this->steam_client_module_.invoke<void>(
              "Steam_ReleaseUser", this->steam_pipe_, this->global_user_);
        }

        this->steam_client_module_.invoke<bool>("Steam_BReleaseSteamPipe",
                                                this->steam_pipe_);
      }
    }
  }

private:
  utils::nt::library steam_client_module_{};

  steam::interface client_engine_ {};
  steam::interface client_user_ {};
  steam::interface client_utils_ {};

  void* steam_pipe_ = nullptr;
  void* global_user_ = nullptr;

  [[nodiscard]] void* load_client_engine() const {
    if (!this->steam_client_module_)
      return nullptr;

    for (auto i = 1; i < 1000; ++i) {
      const auto* name =
          utils::string::va("CLIENTENGINE_INTERFACE_VERSION{:03}", i);

      auto* const client_engine = this->steam_client_module_.invoke<void*>(
          "CreateInterface", name, nullptr);
      if (client_engine)
        return client_engine;
    }

    return nullptr;
  }

  void load_client() {
    const auto steam_path = steam::get_steam_install_directory();
    if (steam_path.empty())
      return;

    utils::nt::library::load(steam_path / "tier0_s.dll");
    utils::nt::library::load(steam_path / "vstdlib_s.dll");
    this->steam_client_module_ =
        utils::nt::library::load(steam_path / "steamclient.dll");
    if (!this->steam_client_module_)
      return;

    this->client_engine_ = load_client_engine();
    if (!this->client_engine_)
      return;

    this->steam_pipe_ =
        this->steam_client_module_.invoke<void*>("Steam_CreateSteamPipe");
    this->global_user_ = this->steam_client_module_.invoke<void*>(
        "Steam_ConnectToGlobalUser", this->steam_pipe_);
    this->client_user_ = this->client_engine_.invoke<void*>(
        8, this->steam_pipe_, this->global_user_); // GetIClientUser
    this->client_utils_ = this->client_engine_.invoke<void*>(
        14, this->steam_pipe_); // GetIClientUtils
  }

  void start_mod(const std::string& title, const std::size_t app_id) {
    __try {
      this->start_mod_unsafe(title, app_id);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
      this->do_cleanup();
    }
  }

  void start_mod_unsafe(const std::string& title, std::size_t app_id) {
    if (!this->client_utils_ || !this->client_user_)
      return;

    if (!this->client_user_.invoke<bool>("BIsSubscribedApp", app_id)) {
      app_id = 480; // Spacewar
    }

    this->client_utils_.invoke<void>("SetAppIDForCurrentPipe", app_id, false);

    char our_directory[MAX_PATH] = {0};
    GetCurrentDirectoryA(sizeof(our_directory), our_directory);

    const auto path = runner_file.get_extracted_file();
    const auto* cmdline =
        utils::string::va("\"{}\" -proc {}", path, GetCurrentProcessId());

    game_id game_id;
    game_id.raw.type = 1; // k_EGameIDTypeGameMod
    game_id.raw.app_id = app_id & 0xFFFFFF;

    const auto* mod_id = "IW4x";
    game_id.raw.mod_id =
        *reinterpret_cast<const unsigned int*>(mod_id) | 0x80000000;

    this->client_user_.invoke<bool>("SpawnProcess", path.data(), cmdline,
                                    our_directory, game_id.bits, title.data(),
                                    app_id, 0, 0, 0);
  }

  void do_cleanup() {
    this->client_engine_ = nullptr;
    this->client_user_ = nullptr;
    this->client_utils_ = nullptr;

    this->steam_pipe_ = nullptr;
    this->global_user_ = nullptr;

    this->steam_client_module_ = utils::nt::library{nullptr};
  }

  void clean_up_on_error() {
    scheduler::schedule([this] {
      if (this->steam_client_module_ && this->steam_pipe_ &&
          this->global_user_ &&
          this->steam_client_module_.invoke<bool>(
              "Steam_BConnected", this->global_user_, this->steam_pipe_) &&
          this->steam_client_module_.invoke<bool>(
              "Steam_BLoggedOn", this->global_user_, this->steam_pipe_)) {
        return scheduler::cond_continue;
      }

      this->do_cleanup();
      return scheduler::cond_end;
    });
  }
};

REGISTER_COMPONENT(steam_proxy)
