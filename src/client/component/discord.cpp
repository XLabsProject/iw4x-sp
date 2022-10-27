#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <discord_rpc.h>

#include "scheduler.hpp"

namespace discord {
class component final : public component_interface {
public:
  void post_load() override {
    DiscordEventHandlers handlers;
    ZeroMemory(&handlers, sizeof(handlers));
    handlers.ready = ready;
    handlers.errored = errored;
    handlers.disconnected = errored;
    handlers.joinGame = nullptr;
    handlers.spectateGame = nullptr;
    handlers.joinRequest = nullptr;

    Discord_Initialize("978049907585863710", &handlers, 1, nullptr);

    scheduler::loop(Discord_RunCallbacks, scheduler::pipeline::main);

    initialized_ = true;
  }

  void pre_destroy() override {
    if (initialized_) {
      Discord_Shutdown();
    }
  }

private:
  bool initialized_ = false;

  static void ready(const DiscordUser* request) {
    DiscordRichPresence discord_presence;
    ZeroMemory(&discord_presence, sizeof(discord_presence));

    discord_presence.state = "Singleplayer";
    discord_presence.instance = 1;
    Discord_UpdatePresence(&discord_presence);
  }

  static void errored(const int error_code, const char* message) {
    printf("Discord: (%i) %s", error_code, message);
  }
};
} // namespace discord

REGISTER_COMPONENT(discord::component)
