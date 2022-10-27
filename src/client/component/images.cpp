#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>

namespace images {
class component final : public component_interface {
public:
  void post_load() override { utils::hook::set<std::uint8_t>(0x544746, 0xEB); }
};
} // namespace images

REGISTER_COMPONENT(images::component)
