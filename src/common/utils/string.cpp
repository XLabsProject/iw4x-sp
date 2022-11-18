#include "string.hpp"
#include <sstream>
#include <algorithm>

#include "nt.hpp"

namespace utils::string {
const char* va_format(std::string_view fmt, std::format_args&& args) {
  static thread_local va_provider<8, 256> provider;
  const auto str = std::vformat(fmt, args);
  return provider.copy(str);
}

std::vector<std::string> split(const std::string& s, const char delim) {
  std::stringstream ss(s);
  std::string item;
  std::vector<std::string> elems;

  while (std::getline(ss, item, delim)) {
    elems.push_back(item); // elems.push_back(std::move(item)); // if C++11
                           // (based on comment from @mchiasson)
  }

  return elems;
}

std::string to_lower(std::string text) {
  std::transform(text.begin(), text.end(), text.begin(),
                 [](const unsigned char input) {
                   return static_cast<char>(std::tolower(input));
                 });

  return text;
}

std::string to_upper(std::string text) {
  std::transform(text.begin(), text.end(), text.begin(),
                 [](const unsigned char input) {
                   return static_cast<char>(std::toupper(input));
                 });

  return text;
}

bool starts_with(const std::string& text, const std::string& substring) {
  return text.find(substring) == 0;
}

bool ends_with(const std::string& text, const std::string& substring) {
  if (substring.size() > text.size())
    return false;
  return std::equal(substring.rbegin(), substring.rend(), text.rbegin());
}

bool compare(const std::string& lhs, const std::string& rhs) {
  return std::ranges::equal(lhs, rhs,
                            [](const unsigned char a, const unsigned char b) {
                              return std::tolower(a) == std::tolower(b);
                            });
}

std::string dump_hex(const std::string& data, const std::string& separator) {
  std::string result;

  for (unsigned int i = 0; i < data.size(); ++i) {
    if (i > 0) {
      result.append(separator);
    }

    result.append(va("%02X", data[i] & 0xFF));
  }

  return result;
}

std::string get_clipboard_data() {
  if (OpenClipboard(nullptr)) {
    std::string data;

    auto* const clipboard_data = GetClipboardData(1u);
    if (clipboard_data) {
      auto* const cliptext = static_cast<char*>(GlobalLock(clipboard_data));
      if (cliptext) {
        data.append(cliptext);
        GlobalUnlock(clipboard_data);
      }
    }
    CloseClipboard();

    return data;
  }
  return {};
}

std::string convert(const std::wstring& wstr) {
  std::string result;
  result.reserve(wstr.size());

  for (const auto& chr : wstr) {
    result.push_back(static_cast<char>(chr));
  }

  return result;
}

std::wstring convert(const std::string& str) {
  std::wstring result;
  result.reserve(str.size());

  for (const auto& chr : str) {
    result.push_back(static_cast<wchar_t>(chr));
  }

  return result;
}

std::string replace(std::string str, const std::string& from,
                    const std::string& to) {
  if (from.empty()) {
    return str;
  }

  std::size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length();
  }

  return str;
}
} // namespace utils::string
