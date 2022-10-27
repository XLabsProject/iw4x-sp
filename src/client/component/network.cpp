#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "network.hpp"

#include <utils/hook.hpp>

namespace network {
class component final : public component_interface {
public:
  void post_load() override { patch_sp(); }

private:
  static void patch_sp() {
    // Ignore protocol mismatch
    utils::hook::set<std::uint8_t>(0x65D0C2, 0xEB);
    utils::hook::set<std::uint8_t>(0x65D0ED, 0xEB);

    // Disable matchmaking stuff
    utils::hook::set<std::uint8_t>(0x43BAE0, 0xEB);

    // Ignore 'MAX_PACKET_USERCMDS'
    utils::hook::set<std::uint8_t>(0x4B1436, 0xEB);

    // Disable IWNet stuff
    utils::hook::set<std::uint8_t>(0x44E824, 0xEB);
    utils::hook::set<std::uint8_t>(0x44E808, 0xEB);

    utils::hook::set<const char*>(0x475417, "connect_coop");

    // Force Win socket initialization
    utils::hook::nop(0x42B649, 2);

    // Kill LSP
    utils::hook::set<std::uint8_t>(0x4553F0, 0xC3); // Hello
    utils::hook::set<std::uint8_t>(0x428D00, 0xC3); // LSP_SendLogRequest
    utils::hook::set<std::uint8_t>(0x4D13C0, 0xC3); // LSP_ParsePacket
    utils::hook::set<std::uint8_t>(0x66D440, 0xC3); // LSP_AddKeepAlive
    utils::hook::nop(0x494E68, 5);                  // Don't create LSP socket
  }
};
} // namespace network

REGISTER_COMPONENT(network::component)
