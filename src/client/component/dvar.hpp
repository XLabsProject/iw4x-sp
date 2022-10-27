#pragma once

namespace dvar {
namespace override {
void dvar_register_bool(const std::string& name, bool value,
                        std::uint16_t flags);
void dvar_register_float(const std::string& name, float value, float min,
                         float max, std::uint16_t flags);
void dvar_register_int(const std::string& name, int value, int min, int max,
                       std::uint16_t flags);
void dvar_register_string(const std::string& name, const std::string& value,
                          std::uint16_t flags);
} // namespace override
} // namespace dvar
