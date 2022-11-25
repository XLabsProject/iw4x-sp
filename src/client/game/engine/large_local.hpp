#pragma once

namespace game::engine {
class large_local {
public:
  explicit large_local(int size_param);
  ~large_local();

  large_local(large_local&&) = delete;
  large_local(const large_local&) = delete;
  large_local& operator=(large_local&&) = delete;
  large_local& operator=(const large_local&) = delete;

  [[nodiscard]] void* get_buf() const;

private:
  void pop_buf();

  int start_pos_;
  int size_;
};
} // namespace game::engine
