#include <string>

#include <utils/nt.hpp>
#include <utils/string.hpp>

#include "flags.hpp"

#include <cassert>
#include <shellapi.h>

namespace utils::flags {
void parse_flags(std::vector<std::string>& flags) {
  int num_args;
  auto* const argv = CommandLineToArgvW(GetCommandLineW(), &num_args);

  assert(flags.empty());

  if (argv == nullptr) {
    return;
  }

  for (auto i = 0; i < num_args; ++i) {
    std::wstring wide_flag(argv[i]);
    if (wide_flag[0] == L'-') {
      wide_flag.erase(wide_flag.begin());
      const auto converted_string = string::convert(wide_flag);
      flags.emplace_back(string::to_lower(converted_string));
    }
  }

  LocalFree(argv);
}

bool has_flag(const std::string& flag) {
  static auto parsed = false;
  static std::vector<std::string> enabled_flags;

  if (!parsed) {
    parse_flags(enabled_flags);
    parsed = true;
  }

  return std::ranges::any_of(
      enabled_flags.cbegin(), enabled_flags.cend(),
      [flag](const auto& elem) { return elem == string::to_lower(flag); });
}
} // namespace utils::flags
