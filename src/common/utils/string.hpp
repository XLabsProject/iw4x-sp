#pragma once
#include "memory.hpp"
#include <iterator>
#include <string>
#include <string_view>
#include <format>

#ifndef ARRAYSIZE
template <class Type, std::size_t n> std::size_t ARRAYSIZE(Type (&)[n]) {
  return n;
}
#endif

namespace utils::string {
template <std::size_t Buffers, std::size_t MinBufferSize>
class va_provider final {
public:
  static_assert(Buffers != 0 && MinBufferSize != 0,
                "Buffers and MinBufferSize mustn't be 0");

  va_provider() : current_buffer_(0) {}

  const char* copy(const std::string& str) {
    ++this->current_buffer_ %= ARRAYSIZE(this->string_pool_);
    auto entry = &this->string_pool_[this->current_buffer_];

    if (!entry->size || !entry->buffer) {
      throw std::runtime_error("String pool not initialized");
    }

    while (str.size() > entry->size) {
      entry->double_size();
    }

    std::memcpy(entry->buffer, str.data(), str.size());

    return entry->buffer;
  }

private:
  class entry final {
  public:
    explicit entry(const std::size_t _size = MinBufferSize)
        : size(_size), buffer(nullptr) {
      if (this->size < MinBufferSize)
        this->size = MinBufferSize;
      this->allocate();
    }

    ~entry() {
      if (this->buffer)
        memory::get_allocator()->free(this->buffer);
      this->size = 0;
      this->buffer = nullptr;
    }

    void allocate() {
      if (this->buffer)
        memory::get_allocator()->free(this->buffer);
      this->buffer =
          memory::get_allocator()->allocate_array<char>(this->size + 1);
    }

    void double_size() {
      this->size *= 2;
      this->allocate();
    }

    std::size_t size;
    char* buffer;
  };

  std::size_t current_buffer_;
  entry string_pool_[Buffers];
};

const char* va_format(std::string_view fmt, std::format_args&& args);

template <typename Arg> // This should display a nice "null" instead of a number
static void sanitize_format_args(Arg& arg) {
  if constexpr (std::is_same_v<Arg, char*> ||
                std::is_same_v<Arg, const char*>) {
    if (arg == nullptr) {
      arg = const_cast<char*>("null");
    }
  }
}

template <typename... Args>
const char* va(std::string_view fmt, Args&&... args) {
  (sanitize_format_args(args), ...);
  return va_format(fmt, std::make_format_args(args...));
}

std::vector<std::string> split(const std::string& s, char delim);

std::string to_lower(const std::string& text);
std::string to_upper(const std::string& text);

bool starts_with(const std::string& text, const std::string& substring);
bool ends_with(const std::string& text, const std::string& substring);
bool compare(const std::string& lhs, const std::string& rhs);

std::string dump_hex(const std::string& data,
                     const std::string& separator = " ");

std::string get_clipboard_data();

std::string convert(const std::wstring& wstr);
std::wstring convert(const std::string& str);

std::string replace(std::string str, const std::string& from,
                    const std::string& to);
} // namespace utils::string
